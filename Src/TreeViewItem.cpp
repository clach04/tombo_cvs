#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include "Tombo.h"
#include "TreeViewItem.h"
#include "MemoSelectView.h"
#include "MemoNote.h"
#include "TString.h"
#include "UniConv.h"
#include "Property.h"
#include "MemoManager.h"
#include "PasswordManager.h"
#include "MemoFolder.h"
#include "DirectoryScanner.h"
#include "Message.h"

#define ITEM_ORDER_FILE		1
#define ITEM_ORDER_FOLDER	0

/////////////////////////////////////////////
//  ctor & dtor
/////////////////////////////////////////////

TreeViewItem::TreeViewItem(BOOL bItem) : bHasMultiItem(bItem)
{
}

TreeViewItem::~TreeViewItem()
{
}

/////////////////////////////////////////////
/////////////////////////////////////////////
//  File
/////////////////////////////////////////////
/////////////////////////////////////////////

TreeViewFileItem::TreeViewFileItem() : TreeViewItem(FALSE)
{
}

BOOL TreeViewFileItem::Move(MemoManager *pMgr, MemoSelectView *pView)
{
	if (!Copy(pMgr, pView)) {
		return FALSE;
	}
	return DeleteWithoutAsk(pMgr, pView);
}

BOOL TreeViewFileItem::Copy(MemoManager *pMgr, MemoSelectView *pView)
{
	TString sPath;
	TString sHeadLine;

	HTREEITEM hParent = pView->GetPathForNewItem(&sPath);
	if (hParent == NULL) return FALSE;

	MemoNote *pNewNote = MemoNote::CopyMemo(pNote, sPath.Get(), &sHeadLine);
	if (pNewNote == NULL) return FALSE;
	pView->NewMemoCreated(pNewNote, sHeadLine.Get(), hParent);
	return TRUE;
}

BOOL TreeViewFileItem::Delete(MemoManager *pMgr, MemoSelectView *pView)
{
	// ユーザへの意思確認
	if (TomboMessageBox(NULL, MSG_CONFIRM_DELETE, MSG_DELETE_TTL, MB_ICONQUESTION | MB_OKCANCEL) != IDOK) return FALSE;

	BOOL bResult = DeleteWithoutAsk(pMgr, pView);

	if (!bResult && pNote == NULL) {
		TomboMessageBox(NULL, MSG_DELETE_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
	}
	return bResult;
}

BOOL TreeViewFileItem::DeleteWithoutAsk(MemoManager *pMgr,MemoSelectView *pView)
{
	// クリップボードに登録されていたらリセットする
	pView->CheckResetClipboard(this);

	if (g_Property.IsUseTwoPane() && pMgr->IsNoteDisplayed(pNote)) {
		// 現在表示されているメモの表示をキャンセルする
		pMgr->NewMemo();
	}
	BOOL bResult = TRUE;

	// ビュー上からアイテムを削除
	if (!pView->DeleteItem(this)) {
		bResult = FALSE;
	}

	// ファイルの削除
	if (!pNote->DeleteMemoData()) return FALSE;
	delete pNote;
	pNote = NULL;
	return TRUE;
}

BOOL TreeViewFileItem::Encrypt(MemoManager *pMgr, MemoSelectView *pView)
{
	// 既に暗号化されていたら無視する
	if (pNote->IsEncrypted()) return TRUE;

	// 詳細ビューに表示されているメモを暗号化しようとしているのであれば、
	// 保存しておく
	if (g_Property.IsUseTwoPane() && pMgr->IsNoteDisplayed(pNote)) {
		pMgr->InactiveDetailsView();
	}

	// 暗号化
	BOOL b;
	TString sHeadLine;
	MemoNote *p = pNote->Encrypt(pMgr->GetPasswordManager(), &sHeadLine, &b);
	if (p == NULL) return FALSE;

	// 暗号化前のメモを削除
	if (!pNote->DeleteMemoData()) {
		MessageBox(NULL, MSG_DELETE_PREV_CRYPT_MEMO_FAILED, TEXT("TOMBO"), MB_ICONWARNING | MB_OK);
	}

	// TreeViewItemの保持するMemoNoteを暗号化されたものに置き換える
	delete pNote;
	pNote = p;

	// 暗号化に伴いアイコン・ヘッドラインが変更になる可能性があるので更新依頼
	if (!pView->UpdateItemStatus(this, sHeadLine.Get())) {
		return FALSE;
	}
	return TRUE;
}

BOOL TreeViewFileItem::Decrypt(MemoManager *pMgr, MemoSelectView *pView)
{
	// 平文だったら無視する
	if (!pNote->IsEncrypted()) return TRUE;

	// 詳細ビューに表示されているメモを復号化しようとしているのであれば、
	// 保存しておく
	if (g_Property.IsUseTwoPane() && pMgr->IsNoteDisplayed(pNote)) {
		pMgr->InactiveDetailsView();
	}

	// 復号化
	BOOL b;
	TString sHeadLine;
	MemoNote *p = pNote->Decrypt(pMgr->GetPasswordManager(), &sHeadLine, &b);
	if (p == NULL) return FALSE;

	// 復号化前のメモを削除
	if (!pNote->DeleteMemoData()) {
		MessageBox(NULL, MSG_DEL_PREV_DECRYPT_MEMO_FAILED, TEXT("TOMBO"), MB_ICONWARNING | MB_OK);
	}

	// TreeViewItemの保持するMemoNoteを復号化されたものに置き換える
	delete pNote;
	pNote = p;

	// 暗号化に伴いアイコン・ヘッドラインが変更になる可能性があるので更新依頼
	if (!pView->UpdateItemStatus(this, sHeadLine.Get())) {
		return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////
//  名称変更
/////////////////////////////////////////////

BOOL TreeViewFileItem::Rename(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR pNewName)
{
	BOOL bResult = pNote->Rename(pNewName);
	if (!bResult) {
		DWORD nErr = GetLastError();
		if (nErr == ERROR_NO_DATA) {
			TomboMessageBox(NULL, MSG_NO_FILENAME, TOMBO_APP_NAME, MB_ICONWARNING | MB_OK);
		} else if (nErr == ERROR_ALREADY_EXISTS) {
			TomboMessageBox(NULL, MSG_SAME_FILE, TOMBO_APP_NAME, MB_ICONWARNING | MB_OK);
		} else {
			TCHAR buf[MAX_PATH];
			wsprintf(buf, MSG_RENAME_FAILED, nErr);
			TomboMessageBox(NULL, buf, TOMBO_APP_NAME, MB_ICONERROR | MB_OK);
		}
	}
	return bResult;
}

/////////////////////////////////////////////
// 
/////////////////////////////////////////////

DWORD TreeViewFileItem::GetIcon(MemoSelectView *, DWORD nStatus)
{
	if (nStatus & MEMO_VIEW_STATE_CLIPED_SET) {
		if (pNote->IsEncrypted()) {
			return IMG_ARTICLE_ENC_MASKED;
		} else {
			return IMG_ARTICLE_MASKED;
		}
	} else {
		if (pNote->IsEncrypted()) {
			return IMG_ARTICLE_ENCRYPTED;
		} else {
			return IMG_ARTICLE;
		}
	}
}

HTREEITEM TreeViewFileItem::GetViewItem()
{
	return pNote ? pNote->GetViewItem() : NULL;
}

void TreeViewFileItem::SetViewItem(HTREEITEM h)
{
	if (pNote) pNote->SetViewItem(h);
}

DWORD TreeViewFileItem::ItemOrder()
{
	return ITEM_ORDER_FILE;
}

/////////////////////////////////////////////
//  Folder
/////////////////////////////////////////////

TreeViewFolderItem::TreeViewFolderItem() : hItem(NULL), TreeViewItem(TRUE)
{
}

BOOL TreeViewFolderItem::Move(MemoManager *pMgr, MemoSelectView *pView)
{
	// Srcパスの取得
	TCHAR buf[MAX_PATH];
	TString sCurrentPath;
	LPTSTR pCurrentPath = pView->GeneratePath(this, buf, MAX_PATH);
	if (!sCurrentPath.AllocFullPath(pCurrentPath)) return FALSE;

	// Dstパスの取得
	TString sDstPath, sDstFullPath;
	HTREEITEM hParent = pView->GetPathForNewItem(&sDstPath);
	if (!sDstFullPath.AllocFullPath(sDstPath.Get())) return FALSE;

	// 移動しようとしているフォルダに詳細ビューでActiveになっているメモが
	// 存在するかもしれないので詳細ビューを一旦InActiveにする
	pMgr->InactiveDetailsView();

	// 表示の削除
	pView->DeleteItem(this);

	// 移動処理
	MemoFolder mfFolder;
	if (!mfFolder.Init(sCurrentPath.Get())) return FALSE;
	if (!mfFolder.Move(sDstFullPath.Get())) return FALSE;	

	// 表示の追加
	TString sItemName;
	ChopFileSeparator(pCurrentPath);
	sItemName.GetPathTail(pCurrentPath);
	pView->CreateNewFolder(hParent, sItemName.Get());
	return TRUE;
}

BOOL TreeViewFolderItem::Copy(MemoManager *pMgr, MemoSelectView *pView)
{
	// Srcパスの取得
	TCHAR buf[MAX_PATH];
	TString sCurrentPath;
	LPTSTR pCurrentPath = pView->GeneratePath(this, buf, MAX_PATH);
	if (!sCurrentPath.AllocFullPath(pCurrentPath)) return FALSE;

	// Dstパスの取得
	TString sDstPath, sDstFullPath;
	HTREEITEM hParent = pView->GetPathForNewItem(&sDstPath);
	if (!sDstFullPath.AllocFullPath(sDstPath.Get())) return FALSE;

	// 移動しようとしているフォルダに詳細ビューでActiveになっているメモが
	// 存在するかもしれないので詳細ビューを一旦InActiveにする
	pMgr->InactiveDetailsView();

	// コピー先が同名のアイテム名になるよう補正
	TString sItemName;
	ChopFileSeparator(pCurrentPath);
	sItemName.GetPathTail(pCurrentPath);
	sDstFullPath.StrCat(sItemName.Get());
	sDstFullPath.StrCat(TEXT("\\"));

	// ファイルのコピー
	MemoFolder mf;
	mf.Init(sCurrentPath.Get());
	BOOL bResult = mf.Copy(sDstFullPath.Get());

	// フォルダの修正
	if (bResult) {
		pView->CreateNewFolder(hParent, sItemName.Get());
		return TRUE;
	} else {
		LPCTSTR pErr = mf.GetErrorReason();
		MessageBox(NULL, pErr ? pErr : MSG_NOT_ENOUGH_MEMORY, TOMBO_APP_NAME, MB_ICONERROR | MB_OK);
		return FALSE;
	}
}

BOOL TreeViewFolderItem::Delete(MemoManager *pMgr, MemoSelectView *pView)
{
	TCHAR buf[MAX_PATH];
	TString sCurrentPath;
	LPTSTR pCurrentPath = pView->GeneratePath(this, buf, MAX_PATH);
	if (!sCurrentPath.AllocFullPath(pCurrentPath)) return FALSE;

	if (_tcslen(pCurrentPath) == 0 ||
		TomboMessageBox(NULL, MSG_CONFIRM_DEL_FOLDER, MSG_DEL_FOLDER_TTL, MB_ICONQUESTION | MB_OKCANCEL) != IDOK) return FALSE;

	// 移動しようとしているフォルダに詳細ビューでActiveになっているメモが
	// 存在するかもしれないので詳細ビューを一旦InActiveにする
	pMgr->InactiveDetailsView();

	// ツリーのCollapse
	pView->TreeCollapse(GetViewItem());

	// ファイルの削除
	MemoFolder mf;
	mf.Init(sCurrentPath.Get());	
	// 表示の削除
	if (mf.Delete()) {
		pView->DeleteItem(this);
		return TRUE;
	} else {
		LPCTSTR pErr = mf.GetErrorReason();
		MessageBox(NULL, pErr ? pErr : MSG_NOT_ENOUGH_MEMORY, TOMBO_APP_NAME, MB_ICONERROR | MB_OK);
		return FALSE;
	}
}

BOOL TreeViewFolderItem::Encrypt(MemoManager *pMgr, MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewFolderItem::Decrypt(MemoManager *pMgr, MemoSelectView *pView)
{
	return FALSE;
}

DWORD TreeViewFolderItem::GetIcon(MemoSelectView *pView, DWORD nStatus)
{
	// ステータスの取得
	TV_ITEM ti;
	ti.mask = TVIF_STATE | TVIF_IMAGE;
	ti.hItem = GetViewItem();
	ti.stateMask = TVIS_EXPANDED;
	pView->GetItem(&ti);

	BOOL bExpanded = ti.state & TVIS_EXPANDED;
	BOOL bCliped = pView->IsCliped(this);

	if (nStatus & MEMO_VIEW_STATE_CLIPED_SET) {
		bCliped = TRUE;
	}
	if (nStatus & MEMO_VIEW_STATE_CLIPED_CLEAR) {
		bCliped = FALSE;
	}
	if (nStatus & MEMO_VIEW_STATE_OPEN_SET) {
		bExpanded = TRUE;
	}
	if (nStatus & MEMO_VIEW_STATE_OPEN_CLEAR) {
		bExpanded = FALSE;
	}

	if (bCliped) {
		if (bExpanded) {
			return IMG_FOLDER_SEL_MASKED;
		} else {
			return IMG_FOLDER_MASKED;
		}
	} else {
		if (bExpanded) {
			return IMG_FOLDER_SEL;
		} else {
			return IMG_FOLDER;
		}
	}
}

/////////////////////////////////////////////
//  名称変更
/////////////////////////////////////////////

BOOL TreeViewFolderItem::Rename(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR pNewName)
{
	TCHAR buf[MAX_PATH];
	TString sCurrentPath;
	LPTSTR pCurrentPath = pView->GeneratePath(this, buf, MAX_PATH);

	// If root node, disable changing.
	if (_tcslen(pCurrentPath) == 0) return FALSE;

	if (!sCurrentPath.AllocFullPath(pCurrentPath)) return FALSE;

	MemoFolder mf;
	mf.Init(sCurrentPath.Get());	

	BOOL bResult = mf.Rename(pNewName);
	if (!bResult) {
		DWORD nErr = GetLastError();
		if (nErr == ERROR_NO_DATA) {
			TomboMessageBox(NULL, MSG_NO_FOLDERNAME, TOMBO_APP_NAME, MB_ICONWARNING | MB_OK);
		} else if (nErr == ERROR_ALREADY_EXISTS) {
			TomboMessageBox(NULL, MSG_SAME_FOLDER, TOMBO_APP_NAME, MB_ICONWARNING | MB_OK);
		} else {
			TCHAR buf[MAX_PATH];
			wsprintf(buf, MSG_REN_FOLDER_FAILED, nErr);
			TomboMessageBox(NULL, buf, TOMBO_APP_NAME, MB_ICONERROR | MB_OK);
		}
	}
	return bResult;
}

/////////////////////////////////////////////
//
/////////////////////////////////////////////

HTREEITEM TreeViewFolderItem::GetViewItem()
{
	return hItem;
}

void TreeViewFolderItem::SetViewItem(HTREEITEM h)
{
	hItem = h;
}

DWORD TreeViewFolderItem::ItemOrder()
{
	return ITEM_ORDER_FOLDER;
}
