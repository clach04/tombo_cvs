#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include "Tombo.h"
#include "TreeViewItem.h"
#include "MemoSelectView.h"
#include "MemoNote.h"
#include "TString.h"
#include "TomboURI.h"
#include "UniConv.h"
#include "Property.h"
#include "MemoManager.h"
#include "MainFrame.h"
#include "DirectoryScanner.h"
#include "MemoFolder.h"
#include "Message.h"
#include "VFStream.h"
#include "TSParser.h"
#include "VarBuffer.h"
#include "DirList.h"
#include "VFManager.h"

#include "Repository.h"

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
TreeViewFileItem::Locator::~Locator()
{
	delete pURI;
}

void TreeViewFileItem::Locator::set(const TomboURI *p)
{
	delete pURI;
	pURI = new TomboURI();
	pURI->Init(p->GetFullURI());
}

TreeViewFileItem::TreeViewFileItem() : TreeViewItem(FALSE)
{
}

TreeViewFileItem::~TreeViewFileItem()
{
}

void TreeViewFileItem::SetNote(const TomboURI *p)
{
	loc.set(p);
	bIsEncrypted = g_Repository.IsEncrypted(loc.getURI());
}

BOOL TreeViewFileItem::Move(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR *ppErr)
{
	if (!Copy(pMgr, pView, ppErr)) {
		return FALSE;
	}
	if (g_Property.IsUseTwoPane() && pMgr->IsNoteDisplayed(loc.getURI()->GetFullURI())) {
		// close current note
		pMgr->NewMemo();
	}

	URIOption opt;
	if (!g_Repository.Delete(loc.getURI(), &opt)) return FALSE;
	return TRUE;
}

BOOL TreeViewFileItem::Copy(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR *ppErr)
{
	TomboURI sCopyToURI;
	TomboURI sCurURI;

	if (!pView->GetURI(&sCurURI)) return FALSE;
	if (!g_Repository.GetAttachURI(&sCurURI, &sCopyToURI)) return FALSE;

	HTREEITEM hParent = pView->ShowItemByURI(&sCopyToURI, FALSE, FALSE);
	URIOption opt;
	if (!g_Repository.Copy(loc.getURI(), &sCopyToURI, &opt)) return FALSE;

	pView->InsertFile(hParent, opt.pNewURI, opt.pNewHeadLine->Get(), FALSE, FALSE);
	return TRUE;
}

BOOL TreeViewFileItem::Delete(MemoManager *pMgr, MemoSelectView *pView)
{
	// Confirm
	if (TomboMessageBox(NULL, MSG_CONFIRM_DELETE, MSG_DELETE_TTL, MB_ICONQUESTION | MB_OKCANCEL) != IDOK) return FALSE;

	if (g_Property.IsUseTwoPane() && pMgr->IsNoteDisplayed(loc.getURI()->GetFullURI())) {
		// close current note
		pMgr->NewMemo();
	}

	URIOption opt;
	if (!g_Repository.Delete(loc.getURI(), &opt)) {
		TomboMessageBox(NULL, MSG_DELETE_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
		return FALSE;
	}
	return TRUE;
}

BOOL TreeViewFileItem::Encrypt(MemoManager *pMgr, MemoSelectView *pView)
{
	// if this note is encrypted, ignore. 
	if (g_Repository.IsEncrypted(loc.getURI())) return TRUE;

	// if the note is opened, close it.
	if (g_Property.IsUseTwoPane() && pMgr->IsNoteDisplayed(loc.getURI()->GetFullURI())) {
		pMgr->InactiveDetailsView();
	}

	URIOption opt(NOTE_OPTIONMASK_ENCRYPTED);
	opt.bEncrypt = TRUE;
	if (!g_Repository.SetOption(loc.getURI(), &opt)) {
		switch(GetLastError()) {
		case ERROR_TOMBO_W_DELETEOLD_FAILED:
			MessageBox(NULL, MSG_DELETE_PREV_CRYPT_MEMO_FAILED, TOMBO_APP_NAME, MB_ICONWARNING | MB_OK);
			break;
		default:
			MessageBox(NULL, MSG_ENCRYPT_FAILED, TOMBO_APP_NAME, MB_ICONERROR | MB_OK);
			break;
		}
		return FALSE;
	}
	// replace MemoNote that TreeViewItem have
	loc.set(opt.pNewURI);
	bIsEncrypted = TRUE;

	// update icon and headline string
	if (!pView->UpdateItemStatusNotify(this, opt.pNewHeadLine->Get())) {
		MessageBox(NULL, TEXT("UpdateItemStatusNotify failed"), TEXT("DEBUG"), MB_OK);
		return FALSE;
	}
	return TRUE;
}

BOOL TreeViewFileItem::Decrypt(MemoManager *pMgr, MemoSelectView *pView)
{
	// if the note is plain text, ignore it
	if (!g_Repository.IsEncrypted(loc.getURI())) return TRUE;

	// if the note is opened, close it.
	if (g_Property.IsUseTwoPane() && pMgr->IsNoteDisplayed(loc.getURI()->GetFullURI())) {
		pMgr->InactiveDetailsView();
	}

	// decrypt
	URIOption opt(NOTE_OPTIONMASK_ENCRYPTED);
	opt.bEncrypt = FALSE;
	if (!g_Repository.SetOption(loc.getURI(), &opt)) {
		switch(GetLastError()) {
		case ERROR_TOMBO_W_DELETEOLD_FAILED:
			MessageBox(NULL, MSG_DEL_PREV_DECRYPT_MEMO_FAILED, TOMBO_APP_NAME, MB_ICONWARNING | MB_OK);
			break;
		default:
			MessageBox(NULL, MSG_DECRYPT_FAILED, TOMBO_APP_NAME, MB_ICONERROR | MB_OK);
			break;
		}
		return FALSE;
	}

	// replace MemoNote that TreeViewItem have
	loc.set(opt.pNewURI);
	bIsEncrypted = FALSE;

	// update icon and headline string
	if (!pView->UpdateItemStatusNotify(this, opt.pNewHeadLine->Get())) {
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

BOOL TreeViewFileItem::Rename(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR pNewHeadLine)
{
	URIOption opt;
	if (!g_Repository.ChangeHeadLine(loc.getURI(), pNewHeadLine, &opt)) {
		switch(GetLastError()) {
		case ERROR_NO_DATA:
			TomboMessageBox(NULL, MSG_NO_FILENAME, TOMBO_APP_NAME, MB_ICONWARNING | MB_OK);
			break;
		case ERROR_ALREADY_EXISTS:
			TomboMessageBox(NULL, MSG_SAME_FILE, TOMBO_APP_NAME, MB_ICONWARNING | MB_OK);
			break;
		case ERROR_TOMBO_I_OPERATION_NOT_PERFORMED:
			return TRUE;
		default:
			{
				TCHAR buf[MAX_PATH];
				wsprintf(buf, MSG_RENAME_FAILED, GetLastError());
				TomboMessageBox(NULL, buf, TOMBO_APP_NAME, MB_ICONERROR | MB_OK);
			}
		}
		return FALSE;
	}

	loc.set(opt.pNewURI);
	return TRUE;
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

BOOL TreeViewFileItem::GetURIItem(MemoSelectView *pView, TString *pItem)
{
	return FALSE;
}

BOOL TreeViewFileItem::GetURI(TString *pURI)
{
	if (!pURI->Set(loc.getURI()->GetFullURI())) return FALSE;
	return TRUE;
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
	TString sFullPath;
	if (!g_Repository.GetPhysicalPath(loc.getURI(), &sFullPath)) return FALSE;

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
	// Inactivate edit view
	pMgr->InactiveDetailsView();
	pView->TreeCollapse(GetViewItem());

	// convert to URI
	TomboURI sSrcURI, sDstURI;
	HTREEITEM hSrcItem = GetViewItem();
	if (!pView->GetURI(&sSrcURI, hSrcItem)) return FALSE;

	HTREEITEM hDstItem;
	TomboURI sSelURI;
	if (!pView->GetCurrentItem(&hDstItem)) return FALSE;
	if (!pView->GetURI(&sSelURI, hDstItem)) return FALSE;
	if (!g_Repository.GetAttachURI(&sSelURI, &sDstURI)) return FALSE;
	HTREEITEM hParentX = pView->ShowItemByURI(&sDstURI, FALSE, FALSE);

	URIOption opt;
	if (!g_Repository.Move(&sSrcURI, &sDstURI, &opt)) {
		if (GetLastError() == ERROR_TOMBO_W_OPERATION_NOT_PERMITTED) {
			*ppErr = MSG_DST_FOLDER_IS_SRC_SUBFOLDER;
		}
		return FALSE;
	}

	TString sHL;
	if (!g_Repository.GetHeadLine(&sSrcURI, &sHL)) return FALSE;
	pView->CreateNewFolder(hParentX, sHL.Get());
	return TRUE;
}

BOOL TreeViewFolderItem::Copy(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR *ppErr)
{
	// Inactivate edit view
	pMgr->InactiveDetailsView();

	// convert to URI
	TomboURI sSrcURI, sDstURI;
	HTREEITEM hSrcItem = GetViewItem();
	if (!pView->GetURI(&sSrcURI, hSrcItem)) return FALSE;

	HTREEITEM hDstItem;
	TomboURI sSelURI;
	if (!pView->GetCurrentItem(&hDstItem)) return FALSE;
	if (!pView->GetURI(&sSelURI, hDstItem)) return FALSE;
	if (!g_Repository.GetAttachURI(&sSelURI, &sDstURI)) return FALSE;
	HTREEITEM hParentX = pView->ShowItemByURI(&sDstURI, FALSE, FALSE);

	URIOption opt;
	if (!g_Repository.Copy(&sSrcURI, &sDstURI, &opt)) {
		if (GetLastError() == ERROR_TOMBO_W_OPERATION_NOT_PERMITTED) {
			*ppErr = MSG_DST_FOLDER_IS_SRC_SUBFOLDER;
		}
		return FALSE;
	}

	TString sHL;
	if (!g_Repository.GetHeadLine(&sSrcURI, &sHL)) return FALSE;

	pView->CreateNewFolder(hParentX, sHL.Get());
	return TRUE;
}

BOOL TreeViewFolderItem::Delete(MemoManager *pMgr, MemoSelectView *pView)
{
	TomboURI sURI;
	if (!pView->GetURI(&sURI)) return FALSE;
	if (sURI.IsRoot()) return TRUE;
	if (TomboMessageBox(NULL, MSG_CONFIRM_DEL_FOLDER, MSG_DEL_FOLDER_TTL, MB_ICONQUESTION | MB_OKCANCEL) != IDOK) return FALSE;

	// inactivate notes
	pMgr->InactiveDetailsView();

	// Collapse tree
	pView->TreeCollapse(GetViewItem());

	URIOption opt;
	if (!g_Repository.Delete(&sURI, &opt)) {
		switch(GetLastError()) {
		case ERROR_TOMBO_W_OTHERFILE_EXISTS:
			MessageBox(NULL, MSG_OTHER_FILE_EXISTS, TOMBO_APP_NAME, MB_ICONWARNING | MB_OK);
			break;
		case ERROR_TOMBO_E_RMFILE_FAILED:
			MessageBox(NULL, MSG_RMFILE_FAILED, TOMBO_APP_NAME, MB_ICONERROR | MB_OK);
			break;
		case ERROR_TOMBO_E_RMDIR_FAILED:
			MessageBox(NULL, MSG_RMDIR_FAILED, TOMBO_APP_NAME, MB_ICONERROR | MB_OK);
			break;
		default:
			MessageBox(NULL, MSG_DELETE_FAILED, TOMBO_APP_NAME, MB_ICONERROR | MB_OK);
			break;
		}
		return FALSE;
	}
	return TRUE;
}

BOOL TreeViewFolderItem::Encrypt(MemoManager *pMgr, MemoSelectView *pView)
{
	TomboURI sURI;
	if (!pView->GetURI(&sURI)) return FALSE;

	if (sURI.IsRoot()) return TRUE;
	if (TomboMessageBox(NULL, MSG_CONFIRM_ENCRYPT_FOLDER, MSG_CONFIRM_ENCRYPT_FOLDER_TTL, MB_ICONQUESTION | MB_OKCANCEL) != IDOK) return TRUE;

	pMgr->InactiveDetailsView();
	pView->TreeCollapse(GetViewItem());

	// encrypt request to repository
	URIOption opt(NOTE_OPTIONMASK_ENCRYPTED);
	opt.bEncrypt = TRUE;
	if (!g_Repository.SetOption(&sURI, &opt)) {
		switch(GetLastError()) {
		case ERROR_TOMBO_I_GET_PASSWORD_CANCELED:
			MessageBox(NULL, MSG_GET_PASS_FAILED, TOMBO_APP_NAME, MB_ICONINFORMATION | MB_OK);
			return TRUE;
		default:
			MessageBox(NULL, MSG_ENCRYPT_FAILED, TOMBO_APP_NAME, MB_ICONERROR | MB_OK);
			return FALSE;
		}
	}
	return TRUE;
}

BOOL TreeViewFolderItem::Decrypt(MemoManager *pMgr, MemoSelectView *pView)
{
	TomboURI sURI;
	if (!pView->GetURI(&sURI)) return FALSE;

	if (sURI.IsRoot()) return TRUE;
	if (TomboMessageBox(NULL, MSG_CONFIRM_DECRYPT_FOLDER, MSG_CONFIRM_DECRYPT_FOLDER_TTL, MB_ICONQUESTION | MB_OKCANCEL) != IDOK) return TRUE;

	pMgr->InactiveDetailsView();
	pView->TreeCollapse(GetViewItem());

	// decrypt request to repository
	URIOption opt(NOTE_OPTIONMASK_ENCRYPTED);
	opt.bEncrypt = FALSE;
	if (!g_Repository.SetOption(&sURI, &opt)) {
		switch(GetLastError()) {
		case ERROR_TOMBO_I_GET_PASSWORD_CANCELED:
			MessageBox(NULL, MSG_GET_PASS_FAILED, TOMBO_APP_NAME, MB_ICONINFORMATION | MB_OK);
			return TRUE;
		default:
			MessageBox(NULL, MSG_DECRYPT_FAILED, TOMBO_APP_NAME, MB_ICONERROR | MB_OK);
			return FALSE;
		}	
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

BOOL TreeViewFolderItem::GetURIItem(MemoSelectView *pView, TString *pItem)
{
	TCHAR buf[MAX_PATH];
	if (!pView->GetURINodeName(GetViewItem(), buf, MAX_PATH)) return FALSE;
	return pItem->Set(buf);
}

BOOL TreeViewFolderItem::Rename(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR pNewName)
{
	TomboURI sCurrentURI;
	if (!pView->GetURI(&sCurrentURI)) return FALSE;
	
	if (sCurrentURI.IsRoot()) return FALSE;

	pMgr->InactiveDetailsView();
	pView->TreeCollapse(GetViewItem());

	URIOption opt;
	if (!g_Repository.ChangeHeadLine(&sCurrentURI, pNewName, &opt)) {
		DWORD nErr = GetLastError();
		switch (nErr) {
		case ERROR_NO_DATA:
			TomboMessageBox(NULL, MSG_NO_FOLDERNAME, TOMBO_APP_NAME, MB_ICONWARNING | MB_OK);
			break;
		case ERROR_ALREADY_EXISTS:
			TomboMessageBox(NULL, MSG_SAME_FOLDER, TOMBO_APP_NAME, MB_ICONWARNING | MB_OK);
			break;
		default:
			{
				TCHAR buf[MAX_PATH];
				wsprintf(buf, MSG_REN_FOLDER_FAILED, nErr);
				TomboMessageBox(NULL, buf, TOMBO_APP_NAME, MB_ICONERROR | MB_OK);
			}
		}
		return FALSE;
	}
	return TRUE;
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

	TomboURI sURI;
	if (!pView->GetURI(&sURI, hParent)) return FALSE;

	DirList dlDirList;
	if (!dlDirList.Init(DIRLIST_OPT_ALLOCURI | DIRLIST_OPT_ALLOCHEADLINE,
						sURI.GetFullURI())) return FALSE;
	if (!dlDirList.GetList(pPrefix, pMatchPath)) return FALSE;

	// Insert to folder
	DWORD n = dlDirList.NumItems();
	for (DWORD i = 0; i < n; i++) {
		struct DirListItem *p = dlDirList.GetItem(i);
		LPCTSTR q = dlDirList.GetFileName(p->nHeadLinePos);
		if (p->bFolder) {
			// folder
			TreeViewFolderItem *pItem = new TreeViewFolderItem();
			pView->InsertFolder(hParent, q, pItem, TRUE);
		} else {
			// note
			LPCTSTR pURI = dlDirList.GetFileName(p->nURIPos);

			TomboURI sURI;
			if (!sURI.Init(pURI)) return FALSE;
			if (!pView->InsertFile(hParent, &sURI, q, TRUE, FALSE)) return FALSE;
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

BOOL TreeViewFileLink::OpenMemo(MemoSelectView *pView, DWORD nOption)
{
	pView->GetManager()->GetMainFrame()->OpenDetailsView(GetRealURI()->GetFullURI(), nOption);
	return TRUE;
}

BOOL TreeViewFileLink::LoadMemo(MemoSelectView *pView, BOOL bAskPass)
{
	pView->GetManager()->GetMainFrame()->LoadMemo(GetRealURI()->GetFullURI(), bAskPass);
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

		TomboURI sURI;
		if (!p->GetURI(&sURI)) return FALSE;
		pView->InsertFile(hItem, &sURI, pTitle, TRUE, TRUE);
	}
	pStore->FreeArray();
	return TRUE;
}

BOOL TreeViewVirtualFolder::IsOperationEnabled(MemoSelectView *pView, OpType op)
{
	DWORD nOpMatrix = OpNewMemo;
	return (nOpMatrix & op) != 0;
}
