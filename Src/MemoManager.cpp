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
	return TRUE; 
}

/////////////////////////////////////////////
// get current selected path
/////////////////////////////////////////////

BOOL MemoManager::GetCurrentSelectedPath(TString *pPath)
{
	LPCTSTR pURI;
	TString sURIstr;

	if (pMemoDetailsView->GetCurrentURI()) {
		pURI = pMemoDetailsView->GetCurrentURI()->GetFullURI();
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
		pMemoDetailsView->StoreCursorPos();
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
		if (!AllocNewMemo(pText, TRUE)) return FALSE;
		// duplicate mode notes are treated as update because pCurrentURI has set.
	} else {
		// Create node if the note is new
		if (pMemoDetailsView->GetCurrentURI() == NULL) {
			if (!AllocNewMemo(pText, FALSE)) return FALSE;
			// change status because the note is not new note at this point.
			pMainFrame->SetNewMemoStatus(FALSE);
		}
	}

	///////////////////////////////////////
	// save notes and update treeview

	TomboURI sCurrentURI(*(pMemoDetailsView->GetCurrentURI()));	// to preserve it because it changed by method Save.

	TomboURI sNewURI;
	TString sNewHeadLine;

	// save note contents
	if (!pMemoDetailsView->Save(&sCurrentURI, &sNewURI, &sNewHeadLine, pText)) return FALSE;

	// UpdateHeadLine causes TVN_SELCHANGING and call SaveIfModify.
	// So if not ResetModify is called, infinite calling causes GPF.
	// update treeview headline string
	pMemoSelectView->UpdateHeadLine(sCurrentURI.GetFullURI(), &sNewURI, sNewHeadLine.Get());

	// update window title
	pMainFrame->SetWindowTitle(&sNewURI);

	return TRUE;
}

////////////////////////////////////////////////////////
// allocate new memo
////////////////////////////////////////////////////////

BOOL MemoManager::AllocNewMemo(LPCTSTR pText, BOOL bCopy)
{
	const TomboURI *pTemplateURI = NULL;
	if (bCopy) {
		pTemplateURI = pMemoDetailsView->GetCurrentURI();
	}

	// get note path
	TomboURI sAttachFolder;

	TomboURI sSelected;
	if (!pMemoSelectView->GetURI(&sSelected)) return FALSE;
	if (!sSelected.GetAttachFolder(&sAttachFolder)) return FALSE;

	// allocate new MemoNote instance and associate to tree view
	TString sHeadLine;
	TomboURI sNewURI;

	// get URI
	if (!g_Repository.RequestAllocateURI(&sAttachFolder, pText, &sHeadLine, &sNewURI, pTemplateURI)) return FALSE;

	// Insert new node to select view
	HTREEITEM hParent;
	hParent = pMemoSelectView->ShowItemByURI(&sAttachFolder, FALSE);
	if (hParent == NULL) return FALSE;
	HTREEITEM hNewItem = pMemoSelectView->InsertFile(hParent, &sNewURI, sHeadLine.Get(), FALSE, FALSE);

	pMemoDetailsView->SetCurrentNote(&sNewURI);

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

BOOL MemoManager::IsNoteDisplayed(const TomboURI *pURI)
{
	if (pMemoDetailsView->GetCurrentURI() == NULL) return FALSE;
	return _tcsicmp(pURI->GetFullURI(), pMemoDetailsView->GetCurrentURI()->GetFullURI()) == 0;
}
