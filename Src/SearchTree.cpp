#include <windows.h>
#include <tchar.h>
#include "Tombo.h"
#include "resource.h"
#include "UniConv.h"
#include "SearchEngine.h"
#include "SearchTree.h"
#include "DirList.h"
#include "MemoNote.h"
#include "TString.h"
#include "TomboURI.h"
#include "DialogTemplate.h"
#include "Message.h"

static DWORD FindList(DirList *pDl, LPCTSTR pString);

////////////////////////////////
// ctor & dtor
////////////////////////////////

BOOL SearchTree::Init(SearchEngineA *p, LPCTSTR path, DWORD offset, BOOL bDirection, BOOL skip, BOOL skipEncrypt)
{
	pStartPath = path;
	pRegex = p;
	nBaseOffset = offset;
	bSearchDirectionForward = bDirection;
	bSkipOne = skip;
	bSearchEncryptedMemo = !skipEncrypt;
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
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SEARCHING), hParent, (DLGPROC)SearchTreeDlgProc, (LONG)this);
}

/////////////////////////////////////////
// Initialize
/////////////////////////////////////////

static DlgMsgRes aDlgMsgRes[] = {
	{ IDCANCEL, MSG_ID_DLG_CMN_CANCEL}, 
};

void SearchTree::InitDialog(HWND hDlg)
{
	OverrideDlgMsg(hDlg, -1, aDlgMsgRes, sizeof(aDlgMsgRes)/sizeof(DlgMsgRes));

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

SearchResult SearchTree::Search()
{
	_tcscpy(aPath, pStartPath);
	LPTSTR pBase = aPath + nBaseOffset + 1;
	*pBase= TEXT('\0');

	srResult = SearchTreeRec(pStartPath + nBaseOffset + 1, pBase);
	return srResult;
}

SearchResult SearchTree::SearchTreeRec(LPCTSTR pNextParse, LPTSTR pBase)
{
	TomboURI sBaseURI;
	if (!sBaseURI.InitByNotePath(aPath + nBaseOffset)) return SR_FAILED;

	// expand directory list
	DirList dl;
//	if (!dl.Init(DIRLIST_OPT_NONE, NULL)) return SR_FAILED;
	if (!dl.Init(DIRLIST_OPT_ALLOCURI | DIRLIST_OPT_ALLOCHEADLINE, sBaseURI.GetFullURI())) return SR_FAILED;
	_tcscpy(pBase, TEXT("*.*"));
	if (!dl.GetList(TEXT(""), aPath, !bSearchEncryptedMemo)) return SR_FAILED;

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
		LPCTSTR pFileName = dl.GetFileName(pItem->nFileNamePos);

		_tcscpy(pBase, pFileName);
		if (pItem->bFolder) {
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

SearchResult SearchTree::SearchOneItem()
{
	if (bSkipOne) {
		bSkipOne = FALSE;
		return SR_NOTFOUND;
	}

	LPCTSTR p = aPath + nBaseOffset + 1;
	MemoNote *pNote = NULL;
	if (!MemoNote::MemoNoteFactory(TEXT(""), p, &pNote)) return SR_FAILED;
	if (pNote == NULL) return SR_NOTFOUND;

	SearchResult result = pRegex->Search(pNote);
	delete pNote;
	return result;
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
		LPCTSTR q = pDl->GetFileName(p->nFileNamePos);

		if (_tcscmp(q, pString) == 0) return i;
	}
	return 0xFFFFFFFF;
}
