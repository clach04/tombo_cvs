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
#include "MainFrame.h"
#include "PasswordManager.h"
#include "MemoFolder.h"
#include "DirectoryScanner.h"
#include "Message.h"
#include "VFStream.h"
#include "TSParser.h"
#include "VarBuffer.h"
#include "DirList.h"
#include "VFManager.h"

#if defined(PLATFORM_BE500)
#include "COShellAPI.h"
#endif

#define ITEM_ORDER_FILE		1
#define ITEM_ORDER_FOLDER	0

/////////////////////////////////////////////
/////////////////////////////////////////////
//  TreeViewItem
/////////////////////////////////////////////
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

BOOL TreeViewItem::IsOperationEnabled(MemoSelectView *pView, OpType op)
{
	DWORD nOpMatrix = OpNewMemo | OpNewFolder | OpCut | OpCopy | OpPaste;
	return (nOpMatrix & op) != 0;
}

BOOL TreeViewItem::IsUseDetailsView()
{
	return FALSE;
}

BOOL TreeViewItem::OpenMemo(MemoSelectView *pView, DWORD nOption)
{
	return TRUE;
}

BOOL TreeViewItem::LoadMemo(MemoSelectView *pView, BOOL bAskPass)
{
	return TRUE;
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

void TreeViewFileItem::SetNote(MemoNote *p)
{ 
	pNote = p;
	bIsEncrypted = pNote->IsEncrypted();
}

BOOL TreeViewFileItem::Move(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR *ppErr)
{
	if (!Copy(pMgr, pView, ppErr)) {
		return FALSE;
	}
	TString uri;
	if (!pView->GetURI(&uri, GetViewItem())) return FALSE;

	if (g_Property.IsUseTwoPane() && pMgr->IsNoteDisplayed(uri.Get())) {
		// 現在表示されているメモの表示をキャンセルする
		pMgr->NewMemo();
	}
	// ファイルの削除
	if (!pNote->DeleteMemoData()) return FALSE;
	return TRUE;
}

BOOL TreeViewFileItem::Copy(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR *ppErr)
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

	TString uri;
	if (!pView->GetURI(&uri, GetViewItem())) return FALSE;

	if (g_Property.IsUseTwoPane() && pMgr->IsNoteDisplayed(uri.Get())) {
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
	TString uri;
	if (!pView->GetURI(&uri, GetViewItem())) return FALSE;

	if (g_Property.IsUseTwoPane() && pMgr->IsNoteDisplayed(uri.Get())) {
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
	bIsEncrypted = TRUE;

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
	TString uri;
	if (!pView->GetURI(&uri, GetViewItem())) return FALSE;

	if (g_Property.IsUseTwoPane() && pMgr->IsNoteDisplayed(uri.Get())) {
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
	bIsEncrypted = FALSE;

	// 暗号化に伴いアイコン・ヘッドラインが変更になる可能性があるので更新依頼
	if (!pView->UpdateItemStatusNotify(this, sHeadLine.Get())) {
		return FALSE;
	}
	return TRUE;
}

BOOL TreeViewFileItem::IsOperationEnabled(MemoSelectView *pView, OpType op)
{
	if (op == OpEncrypt) {
		return !bIsEncrypted;
	} else if (op == OpDecrypt) {
		return bIsEncrypted;
	} else {
		DWORD nOpMatrix = OpDelete | OpRename | OpNewMemo | OpNewFolder | OpCut | OpCopy | OpPaste;
		return (nOpMatrix & op) != 0;
	}
}

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

DWORD TreeViewFileItem::GetIcon(MemoSelectView *, DWORD nStatus)
{
	if (nStatus & MEMO_VIEW_STATE_INIT) {
		if (bIsEncrypted) {
			return IMG_ARTICLE_ENCRYPTED;
		} else {
			return IMG_ARTICLE;
		}
	}

	if (nStatus & MEMO_VIEW_STATE_CLIPED_SET) {
		if (bIsEncrypted) {
			return IMG_ARTICLE_ENC_MASKED;
		} else {
			return IMG_ARTICLE_MASKED;
		}
	} else {
		if (bIsEncrypted) {
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

BOOL TreeViewFileItem::GetFolderPath(MemoSelectView *pView, TString *pPath)
{
	LPCTSTR pNotePath = pNote->MemoPath();
	if (!pPath->Alloc(_tcslen(pNotePath))) return FALSE;
	GetFilePath(pPath->Get(), pNotePath);	
	return TRUE;
}

BOOL TreeViewFileItem::GetLocationPath(MemoSelectView *pView, TString *pPath)
{
	if (!pPath->Set(pNote->MemoPath())) return FALSE;
	return TRUE;
}

BOOL TreeViewFileItem::GetURIItem(MemoSelectView *pView, TString *pItem)
{
	LPCTSTR p = pNote->MemoPath();
	LPCTSTR q = NULL;
	if (*p != TEXT('\\')) {
		q = p;
	}

	while (*p) {
#if defined(PLATFORM_WIN32)
		if (IsDBCSLeadByte(*p)) {
			p+=2;
			continue;
		}
#endif
		if (*p == TEXT('\\')) {
			q = p;
		}
		p++;
	}
	if (q) {
		if (*q == TEXT('\\')) {
			return pItem->Set(q + 1);
		} else {
			return pItem->Set(q);
		}
	} else {
		return FALSE;
	}
}

TreeViewFileItem::IsUseDetailsView()
{
	return TRUE;
}

BOOL TreeViewFileItem::OpenMemo(MemoSelectView *pView, DWORD nOption)
{
	TString sURI;
	if (!pView->GetURI(&sURI, GetViewItem())) return FALSE;
	pView->GetManager()->GetMainFrame()->OpenDetailsView(sURI.Get(), nOption);
	return TRUE;
}

BOOL TreeViewFileItem::LoadMemo(MemoSelectView *pView, BOOL bAskPass)
{
	TString sURI;
	if (!pView->GetURI(&sURI, GetViewItem())) return FALSE;
	pView->GetManager()->GetMainFrame()->LoadMemo(sURI.Get(), bAskPass);
	return TRUE;
}

BOOL TreeViewFileItem::ExecApp(MemoManager *pMgr, MemoSelectView *pView, ExeAppType nType)
{
	LPCTSTR p = pNote->MemoPath();
	TString sFullPath;
	if (!sFullPath.Join(g_Property.TopDir(), TEXT("\\"), p)) return FALSE;

	if (nType == ExecType_Assoc) {
		SHELLEXECUTEINFO se;
		memset(&se, 0, sizeof(se));
		se.cbSize = sizeof(se);
		se.hwnd = pView->GetHWnd();
		se.lpVerb = TEXT("open");
		se.lpFile = sFullPath.Get();
		se.lpParameters = NULL;
		se.lpDirectory = NULL;
		se.nShow = SW_SHOWNORMAL;
		ShellExecuteEx(&se);
		if ((int)se.hInstApp < 32) return FALSE;
		return TRUE;
	}
	if (nType == ExecType_ExtApp1 || nType == ExecType_ExtApp2) {
		LPCTSTR pExeFile = nType == ExecType_ExtApp1 ? g_Property.GetExtApp1() : g_Property.GetExtApp2();
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		memset(&si, 0, sizeof(si));
		memset(&pi, 0, sizeof(pi));
		si.cb = sizeof(si);

		TString sExe;
		TString sCmdLine;
#if defined(PLATFORM_WIN32)
		if (!sCmdLine.Join(TEXT("\""), pExeFile, TEXT("\" "))) return FALSE;
		if (!sCmdLine.StrCat(TEXT("\""))) return FALSE;
		if (!sCmdLine.StrCat(sFullPath.Get())) return FALSE;
		if (!sCmdLine.StrCat(TEXT("\""))) return FALSE;
		if (!CreateProcess(NULL, sCmdLine.Get(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) return FALSE;
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
#endif
#if defined(PLATFORM_HPC)
		if (!sExe.Set(pExeFile)) return FALSE;
		if (!sCmdLine.Join(TEXT("\""), sFullPath.Get(), TEXT("\""))) return FALSE;
		if (!CreateProcess(sExe.Get(), sCmdLine.Get(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) return FALSE;
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
#endif
#if defined(PLATFORM_PKTPC)
		if (!sExe.Set(pExeFile)) return FALSE;
		if (!sCmdLine.Set(sFullPath.Get())) return FALSE;
		if (!CreateProcess(sExe.Get(), sCmdLine.Get(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) return FALSE;
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
#endif
#if defined(PLATFORM_BE500)
		return CoshExecute(pView->GetHWnd(), pExeFile, sFullPath.Get());
#endif
		return TRUE;
	}

	return TRUE;
}

/////////////////////////////////////////////
/////////////////////////////////////////////
//  Folder
/////////////////////////////////////////////
/////////////////////////////////////////////

TreeViewFolderItem::TreeViewFolderItem() : TreeViewItem(TRUE)
{
}

static BOOL IsSubFolder(LPCTSTR pSrc, LPCTSTR pDst)
{
	DWORD n = _tcslen(pSrc);
	if (_tcsncmp(pSrc, pDst, n) == 0) return TRUE;
	return FALSE;
}

BOOL TreeViewFolderItem::Move(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR *ppErr)
{
	// Srcパスの取得
	TCHAR buf[MAX_PATH];
	TString sCurrentPath;
	HTREEITEM hItem = GetViewItem();
	LPTSTR pCurrentPath = pView->GeneratePath(hItem, buf, MAX_PATH);
	if (!sCurrentPath.Join(g_Property.TopDir(), TEXT("\\"), pCurrentPath)) return FALSE;

	// Dstパスの取得
	TString sDstPath, sDstFullPath;
	HTREEITEM hParent = pView->GetPathForNewItem(&sDstPath);
	if (!sDstFullPath.Join(g_Property.TopDir(), TEXT("\\"), sDstPath.Get())) return FALSE;

	if (IsSubFolder(pCurrentPath, sDstPath.Get())) {
		*ppErr = MSG_DST_FOLDER_IS_SRC_SUBFOLDER;
		return FALSE;
	}

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

BOOL TreeViewFolderItem::Copy(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR *ppErr)
{
	// Srcパスの取得
	TCHAR buf[MAX_PATH];
	TString sCurrentPath;
	HTREEITEM hItem = GetViewItem();
	LPTSTR pCurrentPath = pView->GeneratePath(hItem, buf, MAX_PATH);
	if (!sCurrentPath.Join(g_Property.TopDir(), TEXT("\\"), pCurrentPath)) return FALSE;

	// Dstパスの取得
	TString sDstPath, sDstFullPath;
	HTREEITEM hParent = pView->GetPathForNewItem(&sDstPath);
	if (!sDstFullPath.Join(g_Property.TopDir(), TEXT("\\"), sDstPath.Get())) return FALSE;

	if (IsSubFolder(pCurrentPath, sDstPath.Get())) {
		*ppErr = MSG_DST_FOLDER_IS_SRC_SUBFOLDER;
		return FALSE;
	}

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
	if (!sCurrentPath.Join(g_Property.TopDir(), TEXT("\\"), pCurrentPath)) return FALSE;

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
	if (!sCurrentPath.Join(g_Property.TopDir(), TEXT("\\"), pCurrentPath)) return FALSE;

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
	if (!sCurrentPath.Join(g_Property.TopDir(), TEXT("\\"), pCurrentPath)) return FALSE;

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

BOOL TreeViewFolderItem::IsOperationEnabled(MemoSelectView *pView, OpType op)
{
	if (op == OpDelete || op == OpRename || op == OpCut || op == OpCopy) {
		HTREEITEM hParent = pView->GetParentItem(GetViewItem());
		if (hParent == NULL) return FALSE;
		return TRUE;
	} else {
		DWORD nOpMatrix = OpEncrypt | OpDecrypt | OpNewMemo | OpNewFolder | OpPaste | OpGrep;
		return (nOpMatrix & op) != 0;
	}
}

BOOL TreeViewFolderItem::GetFolderPath(MemoSelectView *pView, TString *pPath)
{
	return pView->GetPathForNewItem(pPath, this) != NULL;
}

BOOL TreeViewFolderItem::GetLocationPath(MemoSelectView *pView, TString *pPath)
{
	return GetFolderPath(pView, pPath);
}

BOOL TreeViewFolderItem::GetURIItem(MemoSelectView *pView, TString *pItem)
{
	TCHAR buf[MAX_PATH];
	if (!pView->GetURINodeName(GetViewItem(), buf, MAX_PATH)) return FALSE;
	return pItem->Set(buf);
}

BOOL TreeViewFolderItem::Rename(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR pNewName)
{
	TCHAR buf[MAX_PATH];
	TString sCurrentPath;
	HTREEITEM hItem = GetViewItem();
	LPTSTR pCurrentPath = pView->GeneratePath(hItem, buf, MAX_PATH);

	// If root node, disable changing.
	if (_tcslen(pCurrentPath) == 0) return FALSE;

	if (!sCurrentPath.Join(g_Property.TopDir(), TEXT("\\"), pCurrentPath)) return FALSE;

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

BOOL TreeViewFolderItem::ExecApp(MemoManager *pMgr, MemoSelectView *pView, ExeAppType nType)
{
	if (nType != ExecType_Assoc) return FALSE;

	TCHAR buf[MAX_PATH];
	TString sCurrentPath;
	HTREEITEM hItem = GetViewItem();
	LPTSTR pCurrentPath = pView->GeneratePath(hItem, buf, MAX_PATH);
	if (!sCurrentPath.Join(g_Property.TopDir(), TEXT("\\"), pCurrentPath)) return FALSE;

#if defined(PLATFORM_PKTPC)
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));
	si.cb = sizeof(si);

	if (!CreateProcess(TEXT("\\windows\\iexplore.exe"), sCurrentPath.Get(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) return FALSE;
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return TRUE;
#else

	SHELLEXECUTEINFO se;
	memset(&se, 0, sizeof(se));
	se.cbSize = sizeof(se);
	se.hwnd = pView->GetHWnd();
	se.lpVerb = TEXT("explore");
	se.lpFile = sCurrentPath.Get();
	se.lpParameters = NULL;
	se.lpDirectory = NULL;
	se.nShow = SW_SHOWNORMAL;
	ShellExecuteEx(&se);
	return TRUE;
#endif
}

/////////////////////////////////////////////
/////////////////////////////////////////////
//  File link
/////////////////////////////////////////////
/////////////////////////////////////////////

BOOL TreeViewFileLink::IsOperationEnabled(MemoSelectView *pView, OpType op)
{
	DWORD nOpMatrix = OpNewMemo | OpLink;
	return (nOpMatrix & op) != 0;
}


BOOL TreeViewFileLink::GetFolderPath(MemoSelectView *pView, TString *pPath)
{
	LPCTSTR pNotePath = pNote->MemoPath();
	if (!pPath->Alloc(_tcslen(pNotePath))) return FALSE;
	GetFilePath(pPath->Get(), pNotePath);	
	return TRUE;
}

BOOL TreeViewFileLink::GetLocationPath(MemoSelectView *pView, TString *pPath)
{
	return FALSE;
}

BOOL TreeViewFileLink::OpenMemo(MemoSelectView *pView, DWORD nOption)
{
	TString sURI;
	pNote->GetURI(&sURI);
	pView->GetManager()->GetMainFrame()->OpenDetailsView(sURI.Get(), nOption);
	return TRUE;
}

BOOL TreeViewFileLink::LoadMemo(MemoSelectView *pView, BOOL bAskPass)
{
	TString sURI;
	pNote->GetURI(&sURI);
	pView->GetManager()->GetMainFrame()->LoadMemo(sURI.Get(), bAskPass);
	return TRUE;
}

/////////////////////////////////////////////
/////////////////////////////////////////////
//  Virtual folder(Root)
/////////////////////////////////////////////
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

	BOOL ProcessStream(LPCTSTR pName, BOOL bPersit, VFDirectoryGenerator *pGen, VFStore *pStore);
};

BOOL VFExpandListener::ProcessStream(LPCTSTR pName, BOOL bPersit, VFDirectoryGenerator *pGen, VFStore *pStore)
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


BOOL TreeViewVirtualFolderRoot::IsOperationEnabled(MemoSelectView *pView, OpType op)
{
	DWORD nOpMatrix = OpNewMemo;
	return (nOpMatrix & op) != 0;
}


BOOL TreeViewVirtualFolderRoot::GetFolderPath(MemoSelectView *pView, TString *pPath)
{
	return pPath->Set(TEXT(""));
}

BOOL TreeViewVirtualFolderRoot::GetLocationPath(MemoSelectView *pView, TString *pPath)
{
	return FALSE;
}

BOOL TreeViewVirtualFolderRoot::GetURIItem(MemoSelectView *pView, TString *pItem)
{
	return pItem->Set(TEXT("@vfolder"));
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

BOOL TreeViewVirtualFolder::IsOperationEnabled(MemoSelectView *pView, OpType op)
{
	DWORD nOpMatrix = OpNewMemo;
	return (nOpMatrix & op) != 0;
}

BOOL TreeViewVirtualFolder::GetFolderPath(MemoSelectView *pView, TString *pPath)
{
	return pPath->Set(TEXT(""));
}

BOOL TreeViewVirtualFolder::GetLocationPath(MemoSelectView *pView, TString *pPath)
{
	return FALSE;
}