#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include "Tombo.h"
#include "TString.h"
#include "MemoManager.h"
#include "MemoDetailsView.h"
#include "MemoSelectView.h"
#include "VarBuffer.h"
#include "MainFrame.h"
#include "MemoNote.h"
#include "UniConv.h"
#include "File.h"
#include "Property.h"
#include "NewFolderDialog.h"
#include "MemoInfo.h"
#include "TreeViewItem.h"
#include "SearchEngine.h"
#include "Message.h"
#include "TomboURI.h"

/////////////////////////////////////////////
// ctor & dtor
/////////////////////////////////////////////

MemoManager::MemoManager() : pSearchEngineA(NULL), bMSSearchFlg(FALSE), bMDSearchFlg(FALSE), pCurrentURI(NULL)
{
}

MemoManager::~MemoManager()
{
	if(pSearchEngineA) {
		delete pSearchEngineA;
	}
	delete [] pCurrentURI;
}

/////////////////////////////////////////////
// initialize
/////////////////////////////////////////////

BOOL MemoManager::Init(MainFrame *mf, MemoDetailsView *md, MemoSelectView *ms) 
{
	pMainFrame = mf;
	pMemoDetailsView = md;
	pMemoSelectView = ms;
	pCurrentURI = NULL;
	return TRUE; 
}

////////////////////////////////////////////////////////
// create & hold memonote
////////////////////////////////////////////////////////

void MemoManager::SetCurrentNote(LPCTSTR pURI)
{
	delete [] pCurrentURI;
	pCurrentURI = NULL;
	if (pURI) {
		pCurrentURI = StringDup(pURI);
	}
}

////////////////////////////////////////////////////////
// allocate new memo
////////////////////////////////////////////////////////

// TODO: CHECK
MemoNote *MemoManager::AllocNewMemo(LPCTSTR pText, MemoNote *pTemplate)
{
	TString sMemoPath;

	// get note path
	TreeViewItem *pItem = pMemoSelectView->GetCurrentItem();
	if (!pItem->GetFolderPath(pMemoSelectView, &sMemoPath)) return NULL;

	HTREEITEM hParent;
	if (_tcscmp(sMemoPath.Get(), TEXT("\0")) == 0) {
		// the path is root
		hParent = pMemoSelectView->ShowItem(sMemoPath.Get(), FALSE);
	} else {
		// the path is not root
		ChopFileSeparator(sMemoPath.Get());
		hParent = pMemoSelectView->ShowItem(sMemoPath.Get(), FALSE);
		if (!sMemoPath.StrCat(TEXT("\\"))) return NULL;
	}

	// assert
	// if current item is root, sMemoPath == ""
	// if current item is not root, sMemoPath == "...\"

	if (hParent == NULL) return NULL;

	// allocate new MemoNote instance and associate to tree view
	TString sHeadLine;
	MemoNote *pNote;
	if (pTemplate) {
		pNote = pTemplate->GetNewInstance();
	} else {
		pNote = new PlainMemoNote();
	}
	if (pNote == NULL || !(pNote->InitNewMemo(sMemoPath.Get(), pText, &sHeadLine))) {
		delete pNote;
		return NULL;
	}
	HTREEITEM hNewItem = pMemoSelectView->NewMemoCreated(pNote, sHeadLine.Get(), hParent);
	return pNote;
}

/////////////////////////////////////////////
// get current selected path
/////////////////////////////////////////////

BOOL MemoManager::GetCurrentSelectedPath(TString *pPath)
{
	if (pCurrentURI) {
		MemoNote *pNote = MemoNote::MemoNoteFactory(pCurrentURI);
		TString sMemoPath;
		if (!sMemoPath.Set(pNote->MemoPath())) {
			delete pNote;
			return FALSE;
		}
		delete pNote;
		if (!pPath->GetDirectoryPath(sMemoPath.Get())) return FALSE;
		ChopFileSeparator(pPath->Get());
		return TRUE;
	}

	TreeViewItem *pItem = pMemoSelectView->GetCurrentItem();
	if (!pItem->GetFolderPath(pMemoSelectView, pPath)) return FALSE;
	ChopFileSeparator(pPath->Get());
	return TRUE;
}

/////////////////////////////////////////////
// ファイルのWipeOutと削除
/////////////////////////////////////////////

BOOL MemoManager::WipeOutAndDeleteFile(LPCTSTR pFile)
{
	File delf;
	if (!delf.Open(pFile, GENERIC_WRITE, 0, OPEN_ALWAYS)) return FALSE;

	DWORD nSize = delf.FileSize() / 64 + 1;
	BYTE buf[64];
	for (DWORD i = 0; i < 64; i++) buf[i] = 0;

	for (i = 0; i < nSize; i++) {
		delf.Write(buf, 64);
	}
	delf.Close();
	return DeleteFile(pFile);
}

////////////////////////////////////////////////////////
// フォルダの新規作成
////////////////////////////////////////////////////////

BOOL MemoManager::MakeNewFolder(HWND hWnd, TreeViewItem *pItem)
{
	NewFolderDialog dlg;
	BOOL bPrev = bDisableHotKey;
	bDisableHotKey = TRUE;
	DWORD nResult = dlg.Popup(g_hInstance, hWnd);
	bDisableHotKey = bPrev;
	if (nResult == IDOK) {

		LPCTSTR pFolder = dlg.FolderName();

		TString sPartPath;
		TString sPath;

		HTREEITEM hItem;
		hItem = pMemoSelectView->GetPathForNewItem(&sPartPath, pItem);
		if (hItem == NULL) return FALSE;

		if (!sPath.Join(g_Property.TopDir(), TEXT("\\"), sPartPath.Get())) return FALSE;		

		if (!sPath.StrCat(pFolder)) return FALSE;
		TrimRight(sPath.Get());
		ChopFileSeparator(sPath.Get());

		if (CreateDirectory(sPath.Get(), NULL)) {
			// フォルダの挿入
			pMemoSelectView->CreateNewFolder(hItem, pFolder);
		} else {
			return FALSE;
		}
	}
	return TRUE;
}

////////////////////////////////////////////////////////
// Save notes
////////////////////////////////////////////////////////
// Notes is not save if not modified or read only.
// Otherwise save notes after confirm.

BOOL MemoManager::SaveIfModify(LPDWORD pYNC, BOOL bDupMode)
{
	if (pYNC) {
		*pYNC = IDOK;
	}

	// skip saving if not modified or read only
	if (!pMemoDetailsView->IsModify() || pMemoDetailsView->IsReadOnly()) {
		StoreCursorPos();
		return TRUE;
	}

	if (pYNC) {
		*pYNC = pMainFrame->MessageBox(MSG_MEMO_EDITED, MSG_CONFIRM_SAVE, MB_ICONQUESTION | MB_YESNOCANCEL | MB_APPLMODAL);
		if (*pYNC == IDNO || *pYNC == IDCANCEL) return TRUE;
	}

	// get memo data
	LPTSTR p = pMemoDetailsView->GetMemo();
	if (p == NULL) {
		return FALSE;
	}

	if (bDupMode) {
		MemoNote *pNote;
		if (pCurrentURI == NULL) {
			pNote = AllocNewMemo(p);
		} else {
			MemoNote *pCurrent = MemoNote::MemoNoteFactory(pCurrentURI);
			if (pCurrent == NULL) return FALSE;
			pNote = AllocNewMemo(p, pCurrent);
			delete pCurrent;
		}
		if (pNote == NULL) return FALSE;
		TString sURI;
		if (!pNote->GetURI(&sURI)) return FALSE;
		SetCurrentNote(sURI.Get());
	}

	// 新規メモの場合、ノードの作成
	if (pCurrentURI == NULL) {
		MemoNote *pNote = AllocNewMemo(p);
		if (pNote == NULL) return FALSE;

		TString sURI;
		if (!pNote->GetURI(&sURI)) return FALSE;
		SetCurrentNote(sURI.Get());

		// この時点で新規メモではなくなるのでステータスを変える
		pMainFrame->SetNewMemoStatus(FALSE);
	}

	///////////////////////////////////////
	// save notes and update treeview

	TString sHeadLine;
	TString sOldURI, sNewURI;
	if (!sOldURI.Set(pCurrentURI)) {
		MemoNote::WipeOutAndDelete(p);
		return FALSE;
	}
	MemoNote *pNote = MemoNote::MemoNoteFactory(pCurrentURI);
	if (!pNote->Save(pPassMgr, p, &sHeadLine)) {
		MemoNote::WipeOutAndDelete(p);
		delete pNote;
		return FALSE;
	}
	if (!pNote->GetURI(&sNewURI)) {
		MemoNote::WipeOutAndDelete(p);
		delete pNote;
		return FALSE;
	}

	// reset modify status
	pMemoDetailsView->ResetModify();
	pMainFrame->SetModifyStatus(FALSE);

	// update headline string
	pMemoSelectView->UpdateHeadLine(sOldURI.Get(), sNewURI.Get(), pNote);
	SetCurrentNote(sNewURI.Get());

	delete pNote;
	MemoNote::WipeOutAndDelete(p);

	// save caret position
	StoreCursorPos();

	return TRUE;
}

////////////////////////////////////////////////////////
// メモ内容の表示
////////////////////////////////////////////////////////

BOOL MemoManager::SetMemo(TomboURI *pURI)
{
	MemoNote *pNote = MemoNote::MemoNoteFactory(pURI);
	if (pNote == NULL) return FALSE;

	BOOL bLoop = FALSE;
	LPTSTR p;

	do {
		bLoop = FALSE;
		p = pNote->GetMemoBody(pPassMgr);
		if (p == NULL) {
			DWORD nError = GetLastError();
			if (nError == ERROR_INVALID_PASSWORD) {
				bLoop = TRUE;
			} else {
				pMemoDetailsView->SetMemo(MSG_CANT_OPEN_MEMO, 0, TRUE);
				delete pNote;
				return TRUE;
			}
		}
	} while (bLoop);

	MemoInfo mi;
	DWORD nPos = 0;
	if (pNote && pNote->MemoPath()) {
		if (!mi.ReadInfo(pNote->MemoPath(), &nPos)) nPos = 0;
	}
	BOOL bReadOnly;
	if (!g_Property.OpenReadOnly()) {
		if (!pNote->IsReadOnly(&bReadOnly)) {
			delete pNote;
			return FALSE;
		}
	} else {
		bReadOnly = TRUE;
	}
	pMemoDetailsView->SetMemo(p, nPos, bReadOnly);
	MemoNote::WipeOutAndDelete(p);
	SetCurrentNote(pURI->GetFull());

	delete pNote;
	return TRUE;
}

////////////////////////////////////////////////////////
// 新規メモの作成
////////////////////////////////////////////////////////

BOOL MemoManager::NewMemo()
{
	ClearMemo();
	pMainFrame->SetNewMemoStatus(TRUE);
#if defined(PLATFORM_WIN32)
	pMainFrame->SetTitle(TOMBO_APP_NAME);
#endif
	return TRUE;
}

////////////////////////////////////////////////////////
// メモのクリア
////////////////////////////////////////////////////////

BOOL MemoManager::ClearMemo()
{
	pMemoDetailsView->SetMemo(TEXT(""), 0, FALSE);
	SetCurrentNote(NULL);
	return TRUE;
}

////////////////////////////////////////////////////////
// Save cursor position
////////////////////////////////////////////////////////

BOOL MemoManager::StoreCursorPos()
{
	if (g_Property.KeepCaret()) {

		DWORD nPos = pMemoDetailsView->GetCursorPos();
		DWORD nInitPos = pMemoDetailsView->GetInitialPos();

		MemoInfo mi;
		if (pCurrentURI && nPos != nInitPos) {
			MemoNote *pNote = MemoNote::MemoNoteFactory(pCurrentURI);
			if (pNote == NULL) return FALSE;
			mi.WriteInfo(pNote->MemoPath(), nPos);
			delete pNote;
		}
	}
	return TRUE;
}

void MemoManager::ActivateView(BOOL bSVActive)
{
	pMainFrame->ActivateView(bSVActive);
}

////////////////////////////////////////////////////////
// 全選択
////////////////////////////////////////////////////////

void MemoManager::SelectAll()
{
	if (pMemoDetailsView) {
		pMemoDetailsView->SelectAll();
	}
}

////////////////////////////////////////////////////////
// 
////////////////////////////////////////////////////////

void MemoManager::InactiveDetailsView()
{
	pMainFrame->OnList(TRUE);
}

////////////////////////////////////////////////////////
// 
////////////////////////////////////////////////////////

void MemoManager::SetSearchEngine(SearchEngineA *p)
{
	if (pSearchEngineA) {
		delete pSearchEngineA;
	}
	pSearchEngineA = p;
}

////////////////////////////////////////////////////////
// 詳細ビューに対する検索
////////////////////////////////////////////////////////

BOOL MemoManager::SearchDetailsView(BOOL bFirstSearch, BOOL bForward, BOOL bNFMsg, BOOL bSearchFromTop)
{
	return pMemoDetailsView->Search(bFirstSearch, bForward, bNFMsg, bSearchFromTop); 
}

////////////////////////////////////////////////////////
// Is this note are displayed in detailsview?
////////////////////////////////////////////////////////

BOOL MemoManager::IsNoteDisplayed(LPCTSTR pURI)
{
	if (pCurrentURI == NULL) return FALSE;
	return _tcsicmp(pURI, pCurrentURI) == 0;
}
