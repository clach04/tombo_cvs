#include <windows.h>
#include <tchar.h>
#include "resource.h"
#include "SearchEngine.h"
#include "SearchTree.h"
#include "DirList.h"
#include "MemoNote.h"
#include "TString.h"
#include "Message.h"

static LPCTSTR GetNextDirSeparator(LPCTSTR pStart);
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

//	pItr = new SelectViewIterator();
//	if (!pItr) {
//		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
//		return FALSE;
//	}
//	if (!pItr->Init(path, offset, FALSE)) return FALSE;

	return TRUE;
}

SearchTree::~SearchTree()
{
//	if (pItr) delete pItr;
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
	xxDebug = 0;

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

	xxDebug++;
	return bMatch ? SR_FOUND : SR_NOTFOUND;
}

/////////////////////////////////////////
// sub functions
/////////////////////////////////////////

static LPCTSTR GetNextDirSeparator(LPCTSTR pStart)
{
	LPCTSTR p = pStart;
	while(*p) {
		if (IsDBCSLeadByte((BYTE)*p)) {
			p++;
			if (*p) p++;
			continue;
		}
		if (*p == TEXT('\\')) return p;
		p++;
	}
	return NULL;
}

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

#ifdef COMMENT
/////////////////////////////////////////
/////////////////////////////////////////
// SelectViewIterator implimentation
/////////////////////////////////////////
/////////////////////////////////////////

/////////////////////////////////////////
// Initialize
/////////////////////////////////////////

BOOL SelectViewIterator::Init(LPCTSTR pPath, DWORD base, BOOL bb)
{
	nStackPointer = 0;
	_tcscpy(aPath, pPath);
	nBaseOffset = base;
	pBase = aPath + nBaseOffset;
	bBack = bb;
	if (!LayoutInitialStack(pPath)) return FALSE;
	return GetFirstFile();

}

/////////////////////////////////////////
// dtor
/////////////////////////////////////////

SelectViewIterator::~SelectViewIterator()
{
	// release all stack items
	while(Top()) {
		Pop();
	}
}

/////////////////////////////////////////
// set initial status to the stack
/////////////////////////////////////////

BOOL SelectViewIterator::LayoutInitialStack(LPCTSTR pPath)
{
	LPCTSTR pParsing = pPath + nBaseOffset + 1;
	LPTSTR p = pBase + 1;

	// push root folder to stack
	_tcscpy(p, TEXT("*.*"));
	if (!Push(0)) return FALSE;

	// pickup next path element
	LPCTSTR pNextSep = GetNextDirSeparator(pParsing);

	// if next element is file or empty, break loop
	while(pNextSep != NULL) {
		// append new folder name to parse buffer
		// p indicates elemental folder name
		DWORD n = pNextSep - pParsing;
		_tcsncpy(p, pParsing, n);
		*(p + n) = TEXT('\0');

		// backpatch upper level DirStackItem's current index
		if (!Backpatch(p)) return FALSE;

		// append dir separator and wildcards for enum directory.
		_tcscat(p + n, TEXT("\\*.*"));
		// alloc new dirlist and enum directory
//		if (!Push(dsStack[nStackPointer - 1].nPathTail + n + 1)) return FALSE;
		if (!Push(Top()->nPathTail + n + 1)) return FALSE;

		// chop wildcards
		*(p + n + 1) = TEXT('\0');

		// for next iteration
		pParsing = pNextSep + 1;
		p += n + 1;
		pNextSep = GetNextDirSeparator(pParsing);
	}

	// if last element is file, backpatch at the top of the stack.
	if (*pParsing) {
		// element is file
		// backpatch upper level DirStackItem's current index
		if (!Backpatch(pParsing)) return FALSE;
		_tcscpy(pBase + Top()->nPathTail + 1, pParsing);
	}

	return TRUE;
}

/////////////////////////////////////////
// Push to stack
/////////////////////////////////////////

BOOL SelectViewIterator::Push(DWORD npt)
{
	DirList *pDl = new DirList();
	if (!pDl) { SetLastError(ERROR_NOT_ENOUGH_MEMORY); return FALSE; }
	if (!pDl->Init(FALSE, FALSE)) return FALSE;
	if (!pDl->GetList(TEXT(""), aPath)) return FALSE;

	dsStack[nStackPointer].pDirList = pDl;
	dsStack[nStackPointer].nCurrentItem = 0xFFFFFFFF;
	dsStack[nStackPointer].nPathTail = npt;
	nStackPointer++;

	return TRUE;
}

/////////////////////////////////////////
// Pop from stack
/////////////////////////////////////////
// stack item is released.

void SelectViewIterator::Pop()
{
	// if at the top of the stack, nothing to do
	if (!Top()) return;

	nStackPointer--;
	delete dsStack[nStackPointer].pDirList;
}


/////////////////////////////////////////
// Backpatch 
/////////////////////////////////////////

BOOL SelectViewIterator::Backpatch(LPCTSTR pString)
{
	DirStackItem *pTop = Top();
	if (!pTop) return FALSE;
	pTop->nCurrentItem = FindList(pTop->pDirList, pString);
	if (pTop->nCurrentItem == 0xFFFFFFF) {
		// not found.
		SetLastError(ERROR_INVALID_DATA);
		return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////
// Get first file item
/////////////////////////////////////////
BOOL SelectViewIterator::PushOneItem(DWORD k)
{
	DirStackItem *pCurrent = Top();		

	DirList *pDl = pCurrent->pDirList;
	if (pDl->NumItems() == 0) return TRUE; // empty folder

	pCurrent->nCurrentItem = k; // choose kth item;
	DirListItem *pItem = pDl->GetItem(k);
	LPCTSTR pPathItem = pDl->GetFileName(pItem->nNamePos);

	DWORD n = _tcslen(pPathItem);
	DWORD t = pCurrent->nPathTail + 1;
	_tcscpy(pBase + t, pPathItem);

	// if item is file, return
	if (!(pItem->bFlg)) return TRUE;

	_tcscpy(pBase + t + n, TEXT("\\*.*"));
	if (!Push(pCurrent->nPathTail + n + 1)) return FALSE;

	*(pBase + t + n + 1) = TEXT('\0');

	return TRUE;
}

BOOL SelectViewIterator::StackTopItem()
{
	// if selecting item is folder, go recursively.
	while(TRUE) {
		if (!PushOneItem(0)) return FALSE;
		if (Top()->pDirList->NumItems() == 0) break;
		if (!Top()->pDirList->GetItem(0)->bFlg) break;
	}
	return TRUE;
}

BOOL SelectViewIterator::GetFirstFile()
{
	while(TRUE) {
		// push to top of the item
		if (Top()->nCurrentItem == 0xFFFFFFFF ||
			Top()->pDirList->GetItem(Top()->nCurrentItem)->bFlg) {
			if (!StackTopItem()) return FALSE;
		}
		// pop till next item exists
		while (
			Top()->pDirList->NumItems() == 0 ||
			Top()->nCurrentItem + 1 == Top()->pDirList->NumItems()) {
			Pop();
			if (!Top()) {
				// all items are retrieved.
				return TRUE;
			}
			*(pBase + Top()->nPathTail + 1) = TEXT('\0');
		}
		(Top()->nCurrentItem)++;
		if (!Top()->pDirList->GetItem(Top()->nCurrentItem)->bFlg) {
			DirList *pDl = Top()->pDirList;
			DirListItem *pItem = pDl->GetItem(Top()->nCurrentItem);
			LPCTSTR pPathItem = pDl->GetFileName(pItem->nNamePos);
			_tcscpy(pBase + Top()->nPathTail + 1, pPathItem);
			break;
		}
		if (!PushOneItem(Top()->nCurrentItem)) return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////
// retrieve next item
/////////////////////////////////////////

BOOL SelectViewIterator::Next()
{
#ifdef COMMENT
	if (Top()->nCurrentItem == 0xFFFFFFFF) {
		// current selecting is directory. Stack first item.
	}

	DWORD n = Top()->pDirLst->NumItems();
	if (Top()->nCurrentItem < n) {
		(Top()->nCurrentItem)++;
	} else {
		// Scan finished in this directory. Stack next one.
		Sleep(1);
	}
#endif
	return TRUE;
}
#endif
