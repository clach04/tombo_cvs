#include <windows.h>
#include <tchar.h>
#include "resource.h"
#include "UniConv.h"
#include "SearchEngine.h"
#include "SearchTree.h"
#include "DirList.h"
#include "MemoNote.h"
#include "TString.h"
#include "Message.h"

static DWORD FindList(DirList *pDl, LPCTSTR pString);

////////////////////////////////
// ctor & dtor
////////////////////////////////

BOOL SearchTree::Init(SearchEngineA *p, LPCTSTR path, DWORD offset, BOOL bDirection, BOOL skip)
{
	pStartPath = path;
	pRegex = p;
	nBaseOffset = offset;
	bSearchDirectionForward = bDirection;
	bSkipOne = skip;
	return TRUE;
}

SearchTree::~SearchTree()
{
}

////////////////////////////////
// Dialog procedure
////////////////////////////////

static LRESULT CALLBACK SearchTreeDlgProc(HWND hDlg, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	SearchTree *pDlg;
	if (nMessage == WM_INITDIALOG) {
		SetWindowLong(hDlg, DWL_USER, lParam);
		pDlg = (SearchTree*)lParam;

		pDlg->InitDialog(hDlg);
		return TRUE;
	}

	pDlg = (SearchTree*)GetWindowLong(hDlg, DWL_USER);
	if (pDlg == NULL) return FALSE;

	switch(nMessage) {
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			pDlg->OnClose(hDlg, LOWORD(wParam));
			return TRUE;
		} else if (LOWORD(wParam) == IDCANCEL) {
			pDlg->CancelRequest();
			pDlg->OnClose(hDlg, LOWORD(wParam));
			return TRUE;
		}
	}
	return FALSE;
}

////////////////////////////////
// Thread function
////////////////////////////////

extern "C" static DWORD WINAPI SearchThreadFunc(LPVOID p)
{
	SearchTree *pSt = (SearchTree*)p;

	// Do search work
	pSt->Search();

	PostMessage(pSt->GetWnd(), WM_COMMAND, MAKEWPARAM(IDOK, 0), NULL);
	return 0;
}

/////////////////////////////////////////
// Popup dialog and start searching
/////////////////////////////////////////

void SearchTree::Popup(HINSTANCE hInst, HWND hParent)
{
	// TODO: layout at the center of the main window.
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SEARCHING), hParent, (DLGPROC)SearchTreeDlgProc, (LONG)this);
}

/////////////////////////////////////////
// Initialize
/////////////////////////////////////////

void SearchTree::InitDialog(HWND hDlg)
{
	DWORD nThreadId;

	hDlgWnd = hDlg;
	bStopFlag = FALSE;
	srResult = SR_NOTFOUND;
	hSearchThread = CreateThread(NULL, 0, SearchThreadFunc, (LPVOID)this, 0, &nThreadId);
}

/////////////////////////////////////////
// Close dialog
/////////////////////////////////////////

void SearchTree::OnClose(HWND hDlg, WORD nId)
{
	if (hSearchThread) {
		WaitForSingleObject(hSearchThread, INFINITE);
		CloseHandle(hSearchThread);
		hSearchThread = NULL;
	}
	EndDialog(hDlg, nId);
}

/////////////////////////////////////////
// request cancel
/////////////////////////////////////////

void SearchTree::CancelRequest()
{
	HWND hWnd = GetDlgItem(hDlgWnd, IDC_SEARCHMSG);
	SetWindowText(hWnd, MSG_SEARCH_CANCELING);
	bStopFlag = TRUE;
}

/////////////////////////////////////////
// search main
/////////////////////////////////////////

SearchTree::SearchResult SearchTree::Search()
{
	_tcscpy(aPath, pStartPath);
	LPTSTR pBase = aPath + nBaseOffset + 1;
	*pBase= TEXT('\0');

	srResult = SearchTreeRec(pStartPath + nBaseOffset + 1, pBase);
	return srResult;
}

SearchTree::SearchResult SearchTree::SearchTreeRec(LPCTSTR pNextParse, LPTSTR pBase)
{
	// expand directory list
	DirList dl;
	if (!dl.Init(FALSE, FALSE)) return SR_FAILED;
	_tcscpy(pBase, TEXT("*.*"));
	if (!dl.GetList(TEXT(""), aPath)) return SR_FAILED;

	// check current selecting path
	DWORD nCurrentSelP;
	if (pNextParse && *pNextParse) {
		LPCTSTR p = GetNextDirSeparator(pNextParse);
		if (p) {
			// folder
			DWORD n = p - pNextParse;
			_tcsncpy(pBase, pNextParse, n);
			*(pBase + n) = TEXT('\0');
			nCurrentSelP = FindList(&dl, pBase);
			if (nCurrentSelP == 0xFFFFFFFF) return SR_FAILED;
			_tcscpy(pBase + n, TEXT("\\\0"));
			SearchResult sr = SearchTreeRec(p + 1, pBase + n + 1);
			if (sr != SR_NOTFOUND) return sr;
			if (bSearchDirectionForward) {
				nCurrentSelP++;
			} else {
				nCurrentSelP--;
			}
		} else {
			// file
			_tcscpy(pBase, pNextParse);
			nCurrentSelP = FindList(&dl, pNextParse);
			if (nCurrentSelP == 0xFFFFFFFF) return SR_FAILED;
		}
	} else {
		// expire folder
		if (bSearchDirectionForward) {
			nCurrentSelP = 0;
		} else {
			nCurrentSelP = dl.NumItems() - 1;
		}
	}

	// iteration
	DWORD n = dl.NumItems();
	DWORD i = nCurrentSelP;
	while(i >= 0 && i < n) {
		if (bStopFlag) {
			return SR_CANCELED;
		}

		DirListItem *pItem = dl.GetItem(i);
		LPCTSTR pFileName = dl.GetFileName(pItem->nNamePos);

		_tcscpy(pBase, pFileName);
		if (pItem->bFlg) {
			// directory
			DWORD l = _tcslen(pFileName);
			_tcscpy(pBase + l, TEXT("\\\0"));
			SearchResult sr;
			sr = SearchTreeRec(NULL, pBase + _tcslen(pFileName) + 1);
			if (sr != SR_NOTFOUND) return sr;
		} else {
			// file
			SearchResult sr = SearchOneItem();
			if (sr != SR_NOTFOUND) return sr;
		}
		if (bSearchDirectionForward) {
			i++;
		} else {
			i--;
		}
	}
	return SR_NOTFOUND;
}

/////////////////////////////////////////
// search one file
/////////////////////////////////////////

SearchTree::SearchResult SearchTree::SearchOneItem()
{
	if (bSkipOne) {
		bSkipOne = FALSE;
		return SR_NOTFOUND;
	}

	LPCTSTR p = aPath + nBaseOffset + 1;

	// if target is filename
	if (pRegex->IsFileNameOnly()) {
		TString sPartName;
		if (!GetBaseName(&sPartName, p)) return SR_FAILED;

		BOOL bMatch;
#ifdef _WIN32_WCE
		char *bufA = ConvUnicode2SJIS(sPartName.Get());
		bMatch = pRegex->SearchForward(bufA, 0, FALSE);
		delete [] bufA;
#else
		bMatch = pRegex->SearchForward(sPartName.Get(), 0, FALSE);
#endif
		return bMatch ? SR_FOUND : SR_NOTFOUND;
	}

	// skip crypted note if it is not search target.
	if (!pRegex->IsSearchEncryptMemo() && MemoNote::IsNote(p) == NOTE_TYPE_CRYPTED) return SR_NOTFOUND;

	MemoNote *pNote;
	if (!MemoNote::MemoNoteFactory(TEXT(""), p, &pNote)) return SR_FAILED;

	char *pMemo = pNote->GetMemoBodyA(pRegex->GetPasswordManager());
	if (pMemo == NULL) {
		delete pNote;
		return SR_FAILED;
	}

	BOOL bMatch = pRegex->SearchForward(pMemo, 0, FALSE);
	MemoNote::WipeOutAndDelete(pMemo);
	delete pNote;
	return bMatch ? SR_FOUND : SR_NOTFOUND;
}

/////////////////////////////////////////
// sub functions
/////////////////////////////////////////

// TODO: use binary search
static DWORD FindList(DirList *pDl, LPCTSTR pString)
{
	DWORD n = pDl->NumItems();
	for (DWORD i = 0; i < n; i++) {
		DirListItem *p = pDl->GetItem(i);
		LPCTSTR q = pDl->GetFileName(p->nNamePos);

		if (_tcscmp(q, pString) == 0) return i;
	}
	return 0xFFFFFFFF;
}
