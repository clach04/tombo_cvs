#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include "Tombo.h"
#include "TString.h"
#include "MemoManager.h"
#include "MemoDetailsView.h"
#include "MemoSelectView.h"
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

/////////////////////////////////////////////
// ctor & dtor
/////////////////////////////////////////////

MemoManager::MemoManager() : pSearchEngineA(NULL), bMSSearchFlg(FALSE), bMDSearchFlg(FALSE), pCurrentNote(NULL)
{
}

MemoManager::~MemoManager()
{
	if(pSearchEngineA) {
		delete pSearchEngineA;
	}
}

/////////////////////////////////////////////
// オブジェクト初期化
/////////////////////////////////////////////

BOOL MemoManager::Init(MainFrame *mf, MemoDetailsView *md, MemoSelectView *ms) 
{
	pMainFrame = mf;
	pMemoDetailsView = md;
	pMemoSelectView = ms;
	pCurrentNote = NULL;
	return TRUE; 
}

////////////////////////////////////////////////////////
// create & hold memonote
////////////////////////////////////////////////////////

void MemoManager::SetCurrentNote(MemoLocator *pLoc)
{
	if (pLoc == NULL) {
		pCurrentNote = NULL;
		hCurrentItem = NULL;
		return;
	}

	MemoNote *pNote = pLoc->GetNote();

	if (pNote == NULL) {
		pCurrentNote = NULL;
		hCurrentItem = NULL;
	} else {
		delete pCurrentNote;
		pCurrentNote = pNote->Clone();
		hCurrentItem = pLoc->GetHITEM();
	}
}

////////////////////////////////////////////////////////
// 新規メモの割り当て
////////////////////////////////////////////////////////

MemoLocator MemoManager::AllocNewMemo(LPCTSTR pText, MemoNote *pTemplate)
{
	TString sHeadLine;
	TString sMemoPath;

	// パスの取得
	HTREEITEM hParent = pMemoSelectView->GetPathForNewItem(&sMemoPath);	
	if (hParent == NULL) return MemoLocator(NULL, NULL);

	MemoNote *pNote;
	if (pTemplate) {
		pNote = pTemplate->GetNewInstance();
	} else {
		pNote = new PlainMemoNote();
	}
	if (pNote == NULL || !(pNote->InitNewMemo(sMemoPath.Get(), pText, &sHeadLine))) {
		delete pNote;
		return MemoLocator(NULL, NULL);
	}
	HTREEITEM hNewItem = pMemoSelectView->NewMemoCreated(pNote, sHeadLine.Get(), hParent);
	return MemoLocator(pNote, hNewItem);
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
// メモが選択された場合の処理
////////////////////////////////////////////////////////

void MemoManager::SelectNote(MemoNote *pNote)
{
	if (pNote == NULL) {
		pMainFrame->EnableEncrypt(FALSE);
		pMainFrame->EnableDecrypt(FALSE);
	} else if (pNote->IsEncrypted()) {
		pMainFrame->EnableEncrypt(FALSE);
		pMainFrame->EnableDecrypt(TRUE);
	} else {
		pMainFrame->EnableEncrypt(TRUE);
		pMainFrame->EnableDecrypt(FALSE);
	}
}

////////////////////////////////////////////////////////
// フォルダの新規作成
////////////////////////////////////////////////////////

BOOL MemoManager::MakeNewFolder(HWND hWnd)
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

		HTREEITEM hItem = pMemoSelectView->GetPathForNewItem(&sPartPath);
		if (hItem == NULL) return FALSE;

		if (!sPath.AllocFullPath(sPartPath.Get())) return FALSE;
		if (!sPath.StrCat(pFolder)) return FALSE;
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
// メモ内容のセーブ
////////////////////////////////////////////////////////
// 修正されていなければ保存しない
// 修正されている場合には確認した後に保存

BOOL MemoManager::SaveIfModify(LPDWORD pYNC, BOOL bDupMode)
{
	if (pYNC) {
		*pYNC = IDOK;
	}

	// 修正されていなければセーブしない
	if (!pMemoDetailsView->IsModify()) {
		StoreCursorPos();
		return TRUE;
	}

	if (pYNC) {
		*pYNC = TomboMessageBox(NULL, MSG_MEMO_EDITED, MSG_CONFIRM_SAVE, MB_ICONQUESTION | MB_YESNOCANCEL);
		if (*pYNC == IDNO || *pYNC == IDCANCEL) return TRUE;
	}

	// メモの取得
	LPTSTR p = pMemoDetailsView->GetMemo();
	if (p == NULL) {
		return FALSE;
	}

	if (bDupMode) {
		MemoLocator loc(NULL, NULL);
		if (pCurrentNote == NULL) {
			loc = AllocNewMemo(p);
		} else {
			loc = AllocNewMemo(p, pCurrentNote);
		}
		if (loc.GetNote() == NULL) return FALSE;
		SetCurrentNote(&loc);
	}

	// 新規メモの場合、ノードの作成
	if (pCurrentNote == NULL) {
		MemoLocator loc(NULL, NULL);
		loc = AllocNewMemo(p);
		if (loc.GetNote() == NULL) return FALSE;
		SetCurrentNote(&loc);

		// この時点で新規メモではなくなるのでステータスを変える
		pMainFrame->SetNewMemoStatus(FALSE);
	}

	// ヘッドライン文字列の取得
	TString sHeadLine;

	//　メモの保存
	if (!pCurrentNote->Save(pPassMgr, p, &sHeadLine)) {
		MemoNote::WipeOutAndDelete(p);
		return FALSE;
	}

	// 変更ステータスの解除
	pMemoDetailsView->ResetModify();
	pMainFrame->SetModifyStatus(FALSE);

	// ヘッドライン文字列の変更
	if (hCurrentItem) {
		MemoLocator loc(pCurrentNote, hCurrentItem);
		pMemoSelectView->UpdateHeadLine(&loc, sHeadLine.Get());
	}

	MemoNote::WipeOutAndDelete(p);

	// カーソル位置の保存
	StoreCursorPos();

	return TRUE;
}

////////////////////////////////////////////////////////
// メモ内容の表示
////////////////////////////////////////////////////////

BOOL MemoManager::SetMemo(MemoLocator *pLoc)
{
	MemoNote *pNote = pLoc->GetNote();

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
				pMemoDetailsView->SetMemo(MSG_CANT_OPEN_MEMO, 0);
				return TRUE;
			}
		}
	} while (bLoop);

	MemoInfo mi;
	DWORD nPos = 0;
	if (pNote && pNote->MemoPath()) {
		if (!mi.ReadInfo(pNote->MemoPath(), &nPos)) nPos = 0;
	}

	pMemoDetailsView->SetMemo(p, nPos);
	MemoNote::WipeOutAndDelete(p);
	SetCurrentNote(pLoc);
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
	pMemoDetailsView->SetMemo(TEXT(""), 0);
	MemoLocator loc(NULL, NULL);
	SetCurrentNote(&loc);
	return TRUE;
}

////////////////////////////////////////////////////////
// カーソル位置の保存
////////////////////////////////////////////////////////

BOOL MemoManager::StoreCursorPos()
{
	if (g_Property.KeepCaret()) {
		// オープン位置の保存
		DWORD nPos = pMemoDetailsView->GetCursorPos();

		MemoInfo mi;
		if (pCurrentNote) {
			mi.WriteInfo(pCurrentNote->MemoPath(), nPos);
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

BOOL MemoManager::IsNoteDisplayed(LPCTSTR pFile)
{
	if (pCurrentNote == NULL) return FALSE;
	return _tcsicmp(pFile, pCurrentNote->MemoPath()) == 0;
}

////////////////////////////////////////////////////////
// Release select view items notify
////////////////////////////////////////////////////////
//
// unlink items - detailsview mappings.

void MemoManager::ReleaseItemNotify(MemoLocator *pLoc)
{
	if (pLoc->GetHITEM() == NULL && pLoc->GetNote() == NULL) return;

	if (pLoc->GetHITEM() == hCurrentItem) {
		hCurrentItem = NULL;
	}
}

////////////////////////////////////////////////////////
// Release select view items notify
////////////////////////////////////////////////////////
//
// link items - detailsview mappings.

void MemoManager::InsertItemNotify(MemoLocator *pLoc)
{
	MemoNote *pNote = pLoc->GetNote();
	HTREEITEM hItem = pLoc->GetHITEM();
	if (hItem == NULL && pNote == NULL) return;
	
	if (pCurrentNote && pNote->Equal(pCurrentNote)) {
			hCurrentItem = hItem;
	}
}