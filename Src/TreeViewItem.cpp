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
#include "VFStream.h"
#include "TSParser.h"
#include "VarBuffer.h"
#include "DirList.h"
#include "VFManager.h"

#define ITEM_ORDER_FILE		1
#define ITEM_ORDER_FOLDER	0

/////////////////////////////////////////////
//  ctor & dtor
/////////////////////////////////////////////

TreeViewItem::TreeViewItem(BOOL bItem) : bHasMultiItem(bItem), hItem(NULL)
{
}

TreeViewItem::~TreeViewItem()
{
}

HTREEITEM TreeViewItem::GetViewItem()
{
	return hItem;
}

void TreeViewItem::SetViewItem(HTREEITEM h)
{
	hItem = h;
}

MemoLocator TreeViewItem::ToLocator()
{
	return MemoLocator(NULL, NULL);
}

BOOL TreeViewItem::CanDelete(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewItem::CanRename(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewItem::CanEncrypt(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewItem::CanDecrypt(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewItem::CanNewMemo(MemoSelectView *pView)
{
	return TRUE;
}

BOOL TreeViewItem::CanNewFolder(MemoSelectView *pView)
{
	return TRUE;
}

BOOL TreeViewItem::CanCut(MemoSelectView *pView)
{
	return TRUE;
}

BOOL TreeViewItem::CanCopy(MemoSelectView *pView)
{
	return TRUE;
}

BOOL TreeViewItem::CanPaste(MemoSelectView *pView)
{
	return TRUE;
}

BOOL TreeViewItem::CanGrep(MemoSelectView *pView)
{
	return FALSE;
}

/////////////////////////////////////////////
/////////////////////////////////////////////
//  File
/////////////////////////////////////////////
/////////////////////////////////////////////

TreeViewFileItem::TreeViewFileItem() : TreeViewItem(FALSE)
{
}

TreeViewFileItem::~TreeViewFileItem()
{
	delete pNote;
}

BOOL TreeViewFileItem::Move(MemoManager *pMgr, MemoSelectView *pView)
{
	if (!Copy(pMgr, pView)) {
		return FALSE;
	}
	if (g_Property.IsUseTwoPane() && pMgr->IsNoteDisplayed(pNote->MemoPath())) {
		// 現在表示されているメモの表示をキャンセルする
		pMgr->NewMemo();
	}
	// ファイルの削除
	if (!pNote->DeleteMemoData()) return FALSE;
	return TRUE;
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

	if (g_Property.IsUseTwoPane() && pMgr->IsNoteDisplayed(pNote->MemoPath())) {
		// 現在表示されているメモの表示をキャンセルする
		pMgr->NewMemo();
	}
	// ファイルの削除
	if (!pNote->DeleteMemoData()) {
		TomboMessageBox(NULL, MSG_DELETE_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
		return FALSE;
	}
	return TRUE;
}

BOOL TreeViewFileItem::Encrypt(MemoManager *pMgr, MemoSelectView *pView)
{
	// 既に暗号化されていたら無視する
	if (pNote->IsEncrypted()) return TRUE;

	// 詳細ビューに表示されているメモを暗号化しようとしているのであれば、
	// 保存しておく
	if (g_Property.IsUseTwoPane() && pMgr->IsNoteDisplayed(pNote->MemoPath())) {
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
	if (!pView->UpdateItemStatusNotify(this, sHeadLine.Get())) {
		MessageBox(NULL, TEXT("UpdateItemStatusNotify failed"), TEXT("DEBUG"), MB_OK);
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
	if (g_Property.IsUseTwoPane() && pMgr->IsNoteDisplayed(pNote->MemoPath())) {
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
	if (!pView->UpdateItemStatusNotify(this, sHeadLine.Get())) {
		return FALSE;
	}
	return TRUE;
}

BOOL TreeViewFileItem::CanEncrypt(MemoSelectView *pView)
{
	MemoNote *p = GetNote();
	if (p == NULL) return FALSE;
	return !p->IsEncrypted();
}

BOOL TreeViewFileItem::CanDecrypt(MemoSelectView *pView)
{
	MemoNote *p = GetNote();
	if (p == NULL) return FALSE;
	return p->IsEncrypted();
}

BOOL TreeViewFileItem::CanNewMemo(MemoSelectView *pView)
{
	return TRUE;
}

BOOL TreeViewFileItem::CanNewFolder(MemoSelectView *pView)
{
	return TRUE;
}

BOOL TreeViewFileItem::CanCut(MemoSelectView *pView)
{
	return TRUE;
}

BOOL TreeViewFileItem::CanCopy(MemoSelectView *pView)
{
	return TRUE;
}

BOOL TreeViewFileItem::CanPaste(MemoSelectView *pView)
{
	return TRUE;
}

BOOL TreeViewFileItem::CanGrep(MemoSelectView *pView)
{
	return FALSE;
}

/////////////////////////////////////////////
//  Rename
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

BOOL TreeViewFileItem::CanRename(MemoSelectView *pView)
{
	return TRUE;
}

/////////////////////////////////////////////
// 
/////////////////////////////////////////////

DWORD TreeViewFileItem::GetIcon(MemoSelectView *, DWORD nStatus)
{
	if (nStatus & MEMO_VIEW_STATE_INIT) {
		if (pNote->IsEncrypted()) {
			return IMG_ARTICLE_ENCRYPTED;
		} else {
			return IMG_ARTICLE;
		}
	}

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


DWORD TreeViewFileItem::ItemOrder()
{
	return ITEM_ORDER_FILE;
}

BOOL TreeViewFileItem::CanDelete(MemoSelectView *pView)
{
	return TRUE;
}

/////////////////////////////////////////////
//  Get MemoLocator

MemoLocator TreeViewFileItem::ToLocator()
{
	return MemoLocator(pNote, GetViewItem());
}

BOOL TreeViewFileItem::GetFolderPath(MemoSelectView *pView, TString *pPath)
{
	LPCTSTR pNotePath = pNote->MemoPath();
	if (!pPath->Alloc(_tcslen(pNotePath))) return FALSE;
	GetFilePath(pPath->Get(), pNotePath);	
	return TRUE;
}

/////////////////////////////////////////////
//  Folder
/////////////////////////////////////////////

TreeViewFolderItem::TreeViewFolderItem() : TreeViewItem(TRUE)
{
}

BOOL TreeViewFolderItem::Move(MemoManager *pMgr, MemoSelectView *pView)
{
	// Srcパスの取得
	TCHAR buf[MAX_PATH];
	TString sCurrentPath;
	HTREEITEM hItem = GetViewItem();
	LPTSTR pCurrentPath = pView->GeneratePath(hItem, buf, MAX_PATH);
	if (!sCurrentPath.AllocFullPath(pCurrentPath)) return FALSE;

	// Dstパスの取得
	TString sDstPath, sDstFullPath;
	HTREEITEM hParent = pView->GetPathForNewItem(&sDstPath);
	if (!sDstFullPath.AllocFullPath(sDstPath.Get())) return FALSE;

	// 移動しようとしているフォルダに詳細ビューでActiveになっているメモが
	// 存在するかもしれないので詳細ビューを一旦InActiveにする
	pMgr->InactiveDetailsView();

	// ツリーのCollapse
	pView->TreeCollapse(GetViewItem());

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
	HTREEITEM hItem = GetViewItem();
	LPTSTR pCurrentPath = pView->GeneratePath(hItem, buf, MAX_PATH);
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
	HTREEITEM hItem = GetViewItem();
	LPTSTR pCurrentPath = pView->GeneratePath(hItem, buf, MAX_PATH);
	if (!sCurrentPath.AllocFullPath(pCurrentPath)) return FALSE;

	if (_tcslen(pCurrentPath) == 0 ||
		TomboMessageBox(NULL, MSG_CONFIRM_DEL_FOLDER, MSG_DEL_FOLDER_TTL, MB_ICONQUESTION | MB_OKCANCEL) != IDOK) return FALSE;

	// 移動しようとしているフォルダに詳細ビューでActiveになっているメモが
	// 存在するかもしれないので詳細ビューを一旦InActiveにする
	pMgr->InactiveDetailsView();

	// ツリーのCollapse
	pView->TreeCollapse(GetViewItem());

	MemoFolder mf;
	mf.Init(sCurrentPath.Get());	
	if (mf.Delete()) {
		return TRUE;
	} else {
		LPCTSTR pErr = mf.GetErrorReason();
		MessageBox(NULL, pErr ? pErr : MSG_NOT_ENOUGH_MEMORY, TOMBO_APP_NAME, MB_ICONERROR | MB_OK);
		return FALSE;
	}
}

BOOL TreeViewFolderItem::Encrypt(MemoManager *pMgr, MemoSelectView *pView)
{
	TCHAR buf[MAX_PATH];
	TString sCurrentPath;
	HTREEITEM hItem = GetViewItem();
	LPTSTR pCurrentPath = pView->GeneratePath(hItem, buf, MAX_PATH);
	if (!sCurrentPath.AllocFullPath(pCurrentPath)) return FALSE;

	if (_tcslen(pCurrentPath) == 0 ||
		TomboMessageBox(NULL, MSG_CONFIRM_ENCRYPT_FOLDER, MSG_CONFIRM_ENCRYPT_FOLDER_TTL, MB_ICONQUESTION | MB_OKCANCEL) != IDOK) return TRUE;

	pMgr->InactiveDetailsView();
	pView->TreeCollapse(GetViewItem());
	MemoFolder mf;
	mf.Init(sCurrentPath.Get());	

	if (!mf.Encrypt(pMgr->GetPasswordManager())) {
		LPCTSTR pErr = mf.GetErrorReason();
		MessageBox(NULL, pErr ? pErr : MSG_NOT_ENOUGH_MEMORY, TOMBO_APP_NAME, MB_ICONERROR | MB_OK);
		return TRUE;
	}
	return TRUE;
}

BOOL TreeViewFolderItem::Decrypt(MemoManager *pMgr, MemoSelectView *pView)
{
	TCHAR buf[MAX_PATH];
	TString sCurrentPath;
	HTREEITEM hItem = GetViewItem();
	LPTSTR pCurrentPath = pView->GeneratePath(hItem, buf, MAX_PATH);
	if (!sCurrentPath.AllocFullPath(pCurrentPath)) return FALSE;

	if (_tcslen(pCurrentPath) == 0 ||
		TomboMessageBox(NULL, MSG_CONFIRM_DECRYPT_FOLDER, MSG_CONFIRM_DECRYPT_FOLDER_TTL, MB_ICONQUESTION | MB_OKCANCEL) != IDOK) return TRUE;

	pMgr->InactiveDetailsView();
	pView->TreeCollapse(GetViewItem());
	MemoFolder mf;
	mf.Init(sCurrentPath.Get());	

	if (!mf.Decrypt(pMgr->GetPasswordManager())) {
		LPCTSTR pErr = mf.GetErrorReason();
		MessageBox(NULL, pErr ? pErr : MSG_NOT_ENOUGH_MEMORY, TOMBO_APP_NAME, MB_ICONERROR | MB_OK);
		return TRUE;
	}
	return TRUE;
}

DWORD TreeViewFolderItem::GetIcon(MemoSelectView *pView, DWORD nStatus)
{
	if (nStatus & MEMO_VIEW_STATE_INIT) {
		return IMG_FOLDER;
	}

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

BOOL TreeViewFolderItem::CanDecrypt(MemoSelectView *pView)
{
	return TRUE;
}

BOOL TreeViewFolderItem::CanEncrypt(MemoSelectView *pView)
{
	return TRUE;
}

BOOL TreeViewFolderItem::CanNewMemo(MemoSelectView *pView)
{
	return TRUE;
}

BOOL TreeViewFolderItem::CanNewFolder(MemoSelectView *pView)
{
	return TRUE;
}

BOOL TreeViewFolderItem::CanGrep(MemoSelectView *pView)
{
	return TRUE;
}

BOOL TreeViewFolderItem::GetFolderPath(MemoSelectView *pView, TString *pPath)
{
	return pView->GetPathForNewItem(pPath, this) != NULL;
}

/////////////////////////////////////////////
//  名称変更
/////////////////////////////////////////////

BOOL TreeViewFolderItem::Rename(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR pNewName)
{
	TCHAR buf[MAX_PATH];
	TString sCurrentPath;
	HTREEITEM hItem = GetViewItem();
	LPTSTR pCurrentPath = pView->GeneratePath(hItem, buf, MAX_PATH);

	// If root node, disable changing.
	if (_tcslen(pCurrentPath) == 0) return FALSE;

	if (!sCurrentPath.AllocFullPath(pCurrentPath)) return FALSE;

	pMgr->InactiveDetailsView();
	pView->TreeCollapse(GetViewItem());

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

DWORD TreeViewFolderItem::ItemOrder()
{
	return ITEM_ORDER_FOLDER;
}


BOOL TreeViewFolderItem::Expand(MemoSelectView *pView)
{
	TCHAR buf[MAX_PATH];
	HTREEITEM hParent = GetViewItem();

	LPTSTR pPrefix = pView->GeneratePath(hParent, buf, MAX_PATH);

	TCHAR buf2[MAX_PATH];
	wsprintf(buf2, TEXT("%s\\%s*.*"), g_Property.TopDir(), pPrefix);
	LPCTSTR pMatchPath = buf2;

	DirList dlDirList;
	if (!dlDirList.Init(TRUE, TRUE)) return FALSE;
	if (!dlDirList.GetList(pPrefix, pMatchPath)) return FALSE;

	// Insert to folder
	DWORD n = dlDirList.NumItems();
	for (DWORD i = 0; i < n; i++) {
		struct DirListItem *p = dlDirList.GetItem(i);
		LPCTSTR q = dlDirList.GetFileName(p->nNamePos);
		if (p->bFlg) {
			// folder
			TreeViewFolderItem *pItem = new TreeViewFolderItem();
			pView->InsertFolder(hParent, q, pItem, TRUE);
		} else {
			// note
			if (!pView->InsertFile(hParent, p->pNote, q, TRUE, FALSE)) return FALSE;
		}
	}
	return TRUE;
}

BOOL TreeViewFolderItem::CanDelete(MemoSelectView *pView)
{
	HTREEITEM hParent = pView->GetParentItem(GetViewItem());
	if (hParent == NULL) return FALSE;
	return TRUE;
}

BOOL TreeViewFolderItem::CanRename(MemoSelectView *pView)
{
	// In current version, act is same as CanDelete, so calls it.
	return CanDelete(pView);
}

BOOL TreeViewFolderItem::CanCut(MemoSelectView *pView)
{
	return CanDelete(pView);
}

BOOL TreeViewFolderItem::CanCopy(MemoSelectView *pView)
{
	return CanDelete(pView);
}

BOOL TreeViewFolderItem::CanPaste(MemoSelectView *pView)
{
	return TRUE;
}

/////////////////////////////////////////////
/////////////////////////////////////////////
//  File link
/////////////////////////////////////////////
/////////////////////////////////////////////

BOOL TreeViewFileLink::CanDelete(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewFileLink::CanRename(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewFileLink::CanDecrypt(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewFileLink::CanEncrypt(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewFileLink::CanNewMemo(MemoSelectView *pView)
{
	return TRUE;
}

BOOL TreeViewFileLink::CanNewFolder(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewFileLink::CanCut(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewFileLink::CanCopy(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewFileLink::CanPaste(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewFileLink::CanGrep(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewFileLink::GetFolderPath(MemoSelectView *pView, TString *pPath)
{
	LPCTSTR pNotePath = pNote->MemoPath();
	if (!pPath->Alloc(_tcslen(pNotePath))) return FALSE;
	GetFilePath(pPath->Get(), pNotePath);	
	return TRUE;
}


/////////////////////////////////////////////
/////////////////////////////////////////////
//  Virtual folder(Root)
/////////////////////////////////////////////
/////////////////////////////////////////////

/////////////////////////////////////////////
// ctor & dtor
/////////////////////////////////////////////

TreeViewVirtualFolderRoot::TreeViewVirtualFolderRoot()
{
}

TreeViewVirtualFolderRoot::~TreeViewVirtualFolderRoot()
{
}

BOOL TreeViewVirtualFolderRoot::Init(VFManager *p)
{
	pManager = p;
	return TRUE;
}

DWORD TreeViewVirtualFolderRoot::GetIcon(MemoSelectView *pView, DWORD nStatus)
{
	if (nStatus & MEMO_VIEW_STATE_INIT) {
		return IMG_VFOLDER;
	}

	// Get status
	TV_ITEM ti;
	ti.mask = TVIF_STATE | TVIF_IMAGE;
	ti.hItem = GetViewItem();
	ti.stateMask = TVIS_EXPANDED;
	pView->GetItem(&ti);

	BOOL bExpanded = ti.state & TVIS_EXPANDED;

	if (nStatus & MEMO_VIEW_STATE_OPEN_SET) {
		bExpanded = TRUE;
	}
	if (nStatus & MEMO_VIEW_STATE_OPEN_CLEAR) {
		bExpanded = FALSE;
	}

	if (bExpanded) {
		return IMG_VFOLDER_SEL;
	} else {
		return IMG_VFOLDER;
	}
}

BOOL TreeViewVirtualFolderRoot::InsertVirtualFolder(MemoSelectView *pView, LPCTSTR pName, VFDirectoryGenerator *pGen, VFStore *pStore)
{
	HTREEITEM hParent = GetViewItem();

	TreeViewVirtualFolder *pVf = new TreeViewVirtualFolder();
	if (pVf == NULL) return FALSE;

	pVf->SetGenerator(pGen);
	pVf->SetStore(pStore);

	if (!pView->InsertFolder(hParent, pName, pVf, TRUE)) return FALSE;
	return TRUE;
}

class VFExpandListener : public VirtualFolderEnumListener {
	TreeViewVirtualFolderRoot *pRoot;
	MemoSelectView *pView;
public:
	VFExpandListener(MemoSelectView *pv, TreeViewVirtualFolderRoot *pr) : pView(pv), pRoot(pr) {}

	BOOL ProcessStream(LPCTSTR pName, VFDirectoryGenerator *pGen, VFStore *pStore);
};

BOOL VFExpandListener::ProcessStream(LPCTSTR pName, VFDirectoryGenerator *pGen, VFStore *pStore)
{
	return pRoot->InsertVirtualFolder(pView, pName, pGen, pStore); 
}

BOOL TreeViewVirtualFolderRoot::Expand(MemoSelectView *pView)
{
	VFExpandListener vfel(pView, this);
	pManager->Enum(&vfel);

	return TRUE;
}


BOOL TreeViewVirtualFolderRoot::AddSearchResult(MemoSelectView *pView, const VFInfo *pInfo)
{
	HTREEITEM hParent = GetViewItem();
	if (!pView->IsExpand(hParent)) {
		pView->ToggleExpandFolder(hParent, 0);
	} else {
		// insert tree manually
		VFExpandListener vfel(pView, this);
		pManager->RetrieveInfo(pInfo, &vfel);
	}
	return TRUE;
}

BOOL TreeViewVirtualFolderRoot::CanDelete(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewVirtualFolderRoot::CanRename(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewVirtualFolderRoot::CanDecrypt(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewVirtualFolderRoot::CanEncrypt(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewVirtualFolderRoot::CanNewMemo(MemoSelectView *pView)
{
	return TRUE;
}

BOOL TreeViewVirtualFolderRoot::CanNewFolder(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewVirtualFolderRoot::CanCut(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewVirtualFolderRoot::CanCopy(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewVirtualFolderRoot::CanPaste(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewVirtualFolderRoot::CanGrep(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewVirtualFolderRoot::GetFolderPath(MemoSelectView *pView, TString *pPath)
{
	return pPath->Set(TEXT(""));
}

TreeViewVirtualFolderRoot::ItemIterator *TreeViewVirtualFolderRoot::GetIterator(MemoSelectView *pView)
{
	return new ItemIterator(pView, this);
}

TreeViewVirtualFolderRoot::ItemIterator::ItemIterator(MemoSelectView *pv, TreeViewVirtualFolderRoot *p)
{
	pView = pv;
	pRoot = p;
	hWnd = pView->GetHWnd();
}

BOOL TreeViewVirtualFolderRoot::ItemIterator::First()
{
	HTREEITEM hItem = pRoot->GetViewItem();
	hCurrentItem = TreeView_GetChild(hWnd, hItem);
	return hCurrentItem != NULL;
}

BOOL TreeViewVirtualFolderRoot::ItemIterator::Next()
{
	hCurrentItem = TreeView_GetNextSibling(hWnd, hCurrentItem);
	return hCurrentItem != NULL;
}

BOOL TreeViewVirtualFolderRoot::ItemIterator::Get(TString *pLabel)
{
	TCHAR buf[MAX_PATH * 2];
	TV_ITEM ti;
	ti.mask = TVIF_TEXT;
	ti.hItem = hCurrentItem;
	ti.pszText = buf;
	ti.cchTextMax = MAX_PATH * 2 - 1;
	TreeView_GetItem(hWnd, &ti);
	DWORD n = _tcslen(ti.pszText);
	if (!pLabel->Alloc(n + 1)) return FALSE;
	_tcscpy(pLabel->Get(), ti.pszText);
	return TRUE;
}

/////////////////////////////////////////////
/////////////////////////////////////////////
//  Virtual folder (non-root)
/////////////////////////////////////////////
/////////////////////////////////////////////

TreeViewVirtualFolder::TreeViewVirtualFolder() : pGenerator(NULL), pStore(NULL)
{
}

TreeViewVirtualFolder::~TreeViewVirtualFolder()
{
	if (pGenerator) {
		pGenerator->FreeObject();
		delete pGenerator;
	}
}

DWORD TreeViewVirtualFolder::GetIcon(MemoSelectView *pView, DWORD nStatus)
{
	if (nStatus & MEMO_VIEW_STATE_INIT) {
		return IMG_VFOLDER;
	}

	// Get status
	TV_ITEM ti;
	ti.mask = TVIF_STATE | TVIF_IMAGE;
	ti.hItem = GetViewItem();
	ti.stateMask = TVIS_EXPANDED;
	pView->GetItem(&ti);

	BOOL bExpanded = ti.state & TVIS_EXPANDED;

	if (nStatus & MEMO_VIEW_STATE_OPEN_SET) {
		bExpanded = TRUE;
	}
	if (nStatus & MEMO_VIEW_STATE_OPEN_CLEAR) {
		bExpanded = FALSE;
	}

	if (bExpanded) {
		return IMG_VFOLDER_SEL;
	} else {
		return IMG_VFOLDER;
	}
}

BOOL TreeViewVirtualFolder::SetGenerator(VFDirectoryGenerator *p)
{
	pGenerator = p;
	return TRUE;
}

BOOL TreeViewVirtualFolder::SetStore(VFStore *p)
{
	pStore = p;
	return TRUE;
}

BOOL TreeViewVirtualFolder::Expand(MemoSelectView *pView)
{
	HTREEITEM hItem = GetViewItem();

	// scanning.
	if (!pGenerator || !pStore || 
		!pGenerator->Prepare() || 
		!pGenerator->Activate() ||
		!pGenerator->PostActivate()) {
		return FALSE;
	}

	// Insert notes to tree
	DWORD n = pStore->NumItem();
	VFNote *pNote;
	for (DWORD i = 0; i < n; i++) {
		pNote = pStore->GetNote(i);
		MemoNote *p = pNote->GetNote();
		pNote->ClearNote(); // to prevent deleting p
		LPCTSTR pTitle = pNote->GetFileName();
		pView->InsertFile(hItem, p, pTitle, TRUE, TRUE);
	}
	pStore->FreeArray();
	return TRUE;
}

BOOL TreeViewVirtualFolder::CanDelete(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewVirtualFolder::CanRename(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewVirtualFolder::CanDecrypt(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewVirtualFolder::CanEncrypt(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewVirtualFolder::CanNewMemo(MemoSelectView *pView)
{
	return TRUE;
}

BOOL TreeViewVirtualFolder::CanNewFolder(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewVirtualFolder::CanCut(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewVirtualFolder::CanCopy(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewVirtualFolder::CanPaste(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewVirtualFolder::CanGrep(MemoSelectView *pView)
{
	return FALSE;
}

BOOL TreeViewVirtualFolder::GetFolderPath(MemoSelectView *pView, TString *pPath)
{
	return pPath->Set(TEXT(""));
}
