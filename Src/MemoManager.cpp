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

#include "Repository.h"

#include "AutoPtr.h"

/////////////////////////////////////////////
// ctor & dtor
/////////////////////////////////////////////

MemoManager::MemoManager() : pSearchEngineA(NULL), bMSSearchFlg(FALSE), bMDSearchFlg(FALSE)
{
}

MemoManager::~MemoManager()
{
	if(pSearchEngineA) {
		delete pSearchEngineA;
	}
}

/////////////////////////////////////////////
// initialize
/////////////////////////////////////////////

BOOL MemoManager::Init(MainFrame *mf, MemoDetailsView *md, MemoSelectView *ms) 
{
	pMainFrame = mf;
	pMemoDetailsView = md;
	pMemoSelectView = ms;
	pMemoDetailsView->pCurrentURI = NULL;
	return TRUE; 
}

/////////////////////////////////////////////
// get current selected path
/////////////////////////////////////////////

BOOL MemoManager::GetCurrentSelectedPath(TString *pPath)
{
	LPCTSTR pURI;
	TString sURIstr;

	if (pMemoDetailsView->pCurrentURI) {
		pURI = pMemoDetailsView->pCurrentURI;
	} else {
		if (!pMemoSelectView->GetURI(&sURIstr)) return FALSE;
		pURI = sURIstr.Get();
	}

	TomboURI sURI;
	if (!sURI.Init(pURI)) return FALSE;

	if (sURI.IsLeaf()) {
		TomboURI sParent;
		if (!sURI.GetParent(&sParent)) return FALSE;
		if (!sParent.GetFilePath(pPath)) return FALSE;
	} else {
		if (!sURI.GetFilePath(pPath)) return FALSE;
	}
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
		if (!g_Property.DisableSaveDlg()) {
			*pYNC = pMainFrame->MessageBox(MSG_MEMO_EDITED, MSG_CONFIRM_SAVE, MB_ICONQUESTION | MB_YESNOCANCEL | MB_APPLMODAL);
			if (*pYNC == IDNO || *pYNC == IDCANCEL) return TRUE;
		} else {
			*pYNC = IDOK;
		}
	}

	////////////////////////////////////////////

	// get memo data
	LPTSTR pText = pMemoDetailsView->GetMemo();
	SecureBufferT sbData(pText);
	if (pText == NULL) return FALSE;

	////////////////////////////////////////////

	if (bDupMode) {
		if (!AllocNewMemo(pText, pMemoDetailsView->pCurrentURI)) return FALSE;
		// duplicate mode notes are treated as update because pCurrentURI has set.
	}

	// Create node if the note is new
	if (pMemoDetailsView->pCurrentURI == NULL) {
		if (!AllocNewMemo(pText)) return FALSE;
		// change status because the note is not new note at this point.
		pMainFrame->SetNewMemoStatus(FALSE);
	}

	///////////////////////////////////////
	// save notes and update treeview

	TomboURI sCurrentURI;
	if (!sCurrentURI.Init(pMemoDetailsView->pCurrentURI)) return FALSE;

	TomboURI sNewURI;
	TString sNewHeadLine;

	if (!g_Repository.Update(&sCurrentURI, pText, &sNewURI, &sNewHeadLine)) return FALSE;

	pMemoDetailsView->ResetModify();
	// UpdateHeadLine causes TVN_SELCHANGING and call SaveIfModify.
	// So if not ResetModify is called, infinite calling causes GPF.

	// update headline string
	pMemoSelectView->UpdateHeadLine(pMemoDetailsView->pCurrentURI, &sNewURI, sNewHeadLine.Get());

	pMainFrame->SetModifyStatus(FALSE);
	pMemoDetailsView->SetCurrentNote(sNewURI.GetFullURI());

	pMainFrame->SetWindowTitle(&sNewURI);
	// save caret position
	StoreCursorPos();

	return TRUE;
}

////////////////////////////////////////////////////////
// allocate new memo
////////////////////////////////////////////////////////

BOOL MemoManager::AllocNewMemo(LPCTSTR pText, LPCTSTR pTemplateURI)
{
	TString sMemoPath;
	TomboURI sParentURI;

	// get note path
	{
		TString sURIstr;
		if (!pMemoSelectView->GetURI(&sURIstr)) return FALSE;
		if (!sParentURI.Init(sURIstr.Get())) return FALSE;

		if (sParentURI.IsLeaf()) {
			TomboURI sParent;
			if (!sParentURI.GetParent(&sParent)) return FALSE;
			if (!sParent.GetFilePath(&sMemoPath)) return FALSE;
		} else {
			if (!sParentURI.GetFilePath(&sMemoPath)) return FALSE;
		}
	}

	// get parent node
	HTREEITEM hParent;
	hParent = pMemoSelectView->ShowItem(sMemoPath.Get(), FALSE);
	if (hParent == NULL) return FALSE;

	// allocate new MemoNote instance and associate to tree view
	TString sHeadLine;
	TomboURI sNewURI;

	TomboURI *pTmpl;
	TomboURI sTemplateURI;
	if (pTemplateURI) {
		if (!sTemplateURI.Init(pTemplateURI)) return FALSE;
		pTmpl = &sTemplateURI;
	} else {
		pTmpl = NULL;
	}

	if (!g_Repository.RequestAllocateURI(sMemoPath.Get(), pText, &sHeadLine, &sNewURI, pTmpl)) return FALSE;

	HTREEITEM hNewItem = pMemoSelectView->InsertFile(hParent, &sNewURI, sHeadLine.Get(), FALSE, FALSE);
	pMemoDetailsView->SetCurrentNote(sNewURI.GetFullURI());

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

	MemoInfo mi(g_Property.TopDir());
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
	pMemoDetailsView->SetCurrentNote(pURI->GetFullURI());

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
	pMainFrame->SetWindowTitle(NULL);
	return TRUE;
}

////////////////////////////////////////////////////////
// メモのクリア
////////////////////////////////////////////////////////

BOOL MemoManager::ClearMemo()
{
	return pMemoDetailsView->ClearMemo();
}

////////////////////////////////////////////////////////
// Save cursor position
////////////////////////////////////////////////////////

BOOL MemoManager::StoreCursorPos()
{
	if (g_Property.KeepCaret()) {

		DWORD nPos = pMemoDetailsView->GetCursorPos();
		DWORD nInitPos = pMemoDetailsView->GetInitialPos();

		MemoInfo mi(g_Property.TopDir());
		if (pMemoDetailsView->pCurrentURI && nPos != nInitPos) {
			MemoNote *pNote = MemoNote::MemoNoteFactory(pMemoDetailsView->pCurrentURI);
			if (pNote == NULL) return FALSE;
			mi.WriteInfo(pNote->MemoPath(), nPos);
			delete pNote;
		}
	}
	return TRUE;
}

////////////////////////////////////////////////////////
// 
////////////////////////////////////////////////////////

void MemoManager::InactiveDetailsView()
{
	pMainFrame->LeaveDetailsView(TRUE);
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
// Is this note are displayed in detailsview?
////////////////////////////////////////////////////////

BOOL MemoManager::IsNoteDisplayed(LPCTSTR pURI)
{
	if (pMemoDetailsView->pCurrentURI == NULL) return FALSE;
	return _tcsicmp(pURI, pMemoDetailsView->pCurrentURI) == 0;
}
