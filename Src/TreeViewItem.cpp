#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include "Tombo.h"
#include "TreeViewItem.h"
#include "MemoSelectView.h"
#include "TString.h"
#include "TomboURI.h"
#include "UniConv.h"
#include "Property.h"
#include "MemoManager.h"
#include "MainFrame.h"
#include "DirectoryScanner.h"
#include "Message.h"
#include "VFStream.h"
#include "TSParser.h"
#include "VarBuffer.h"
#include "VFManager.h"
#include "AutoPtr.h"
#include "URIScanner.h"

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
	pURI = new TomboURI(*p);	// share string buffer
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

BOOL TreeViewFileItem::CopyMove(BOOL bCopy, MemoManager *pMgr, MemoSelectView *pView)
{
	TomboURI sCopyToURI;
	TomboURI sCurURI;

	if (!pView->GetURI(&sCurURI)) return FALSE;
	if (!g_Repository.GetAttachURI(&sCurURI, &sCopyToURI)) return FALSE;

	HTREEITEM hParent = pView->ShowItemByURI(&sCopyToURI, FALSE, FALSE);
	URIOption opt;
	if (bCopy) {
		if (!g_Repository.Copy(loc.getURI(), &sCopyToURI, &opt)) return FALSE;
	} else {
		if (!g_Repository.Move(loc.getURI(), &sCopyToURI, &opt)) return FALSE;
	}

	pView->InsertFile(hParent, opt.pNewURI, opt.pNewHeadLine->Get(), FALSE, FALSE);
	return TRUE;
}

BOOL TreeViewFileItem::Move(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR *ppErr)
{
	return CopyMove(FALSE, pMgr, pView);
}

BOOL TreeViewFileItem::Copy(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR *ppErr)
{
	return CopyMove(TRUE, pMgr, pView);
}

BOOL TreeViewFileItem::Delete(MemoManager *pMgr, MemoSelectView *pView)
{
	// Confirm
	if (TomboMessageBox(NULL, MSG_CONFIRM_DELETE, MSG_DELETE_TTL, MB_ICONQUESTION | MB_OKCANCEL) != IDOK) return FALSE;

	if (g_Property.GetUseTwoPane() && pMgr->GetDetailsView()->IsNoteDisplayed(loc.getURI())) {
		// close current note
		pMgr->GetDetailsView()->DiscardMemo();
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
	if (g_Property.GetUseTwoPane() && pMgr->GetDetailsView()->IsNoteDisplayed(loc.getURI())) {
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
	// replace URI that TreeViewItem have
	loc.set(opt.pNewURI);
	bIsEncrypted = TRUE;
	pView->GetManager()->ChangeURINotify(opt.pNewURI);

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
	if (g_Property.GetUseTwoPane() && pMgr->GetDetailsView()->IsNoteDisplayed(loc.getURI())) {
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

	// replace URI that TreeViewItem have
	loc.set(opt.pNewURI);
	bIsEncrypted = FALSE;
	pView->GetManager()->ChangeURINotify(opt.pNewURI);

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
	pView->GetManager()->ChangeURINotify(opt.pNewURI);
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
	pView->GetManager()->GetMainFrame()->OpenDetailsView(loc.getURI(), nOption);
	return TRUE;
}

BOOL TreeViewFileItem::LoadMemo(MemoSelectView *pView, BOOL bAskPass)
{
	pView->GetManager()->GetMainFrame()->LoadMemo(loc.getURI(), bAskPass);
	return TRUE;
}

BOOL TreeViewFileItem::ExecApp(MemoManager *pMgr, MemoSelectView *pView, ExeAppType nType)
{
	if(!g_Repository.ExecuteAssoc(loc.getURI(), nType)) return FALSE;
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

BOOL TreeViewFolderItem::CopyMove(BOOL bCopy, MemoManager *pMgr, MemoSelectView *pView, LPCTSTR *ppErr)
{
	// Inactivate edit view
	pMgr->InactiveDetailsView();
	if (!bCopy) {
		pView->TreeCollapse(GetViewItem());
	}

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
	BOOL bResult;
	if (bCopy) {
		bResult = g_Repository.Copy(&sSrcURI, &sDstURI, &opt);
	} else {
		bResult = g_Repository.Move(&sSrcURI, &sDstURI, &opt);
	}
	if (!bResult) {
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

BOOL TreeViewFolderItem::Move(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR *ppErr)
{
	return CopyMove(FALSE, pMgr, pView, ppErr);
}

BOOL TreeViewFolderItem::Copy(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR *ppErr)
{
	return CopyMove(TRUE, pMgr, pView, ppErr);
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
	HTREEITEM hParent = GetViewItem();

	TomboURI sURI;
	if (!pView->GetURI(&sURI, hParent)) return FALSE;

	BOOL bLoose;
	URIList *pURIList = g_Repository.GetChild(&sURI, FALSE, TRUE, &bLoose);
	if (pURIList == NULL) return FALSE;
	AutoPointer<URIList> ap(pURIList);

	if (bLoose) {
		MessageBox(NULL, MSG_DECRYPT_IS_PARTIAL, TEXT("Warning"), MB_ICONWARNING | MB_OK);
	}

	DWORD n = pURIList->GetSize();
	for (DWORD i = 0; i < n; i++) {
		URIOption opt(NOTE_OPTIONMASK_VALID);
		if (!g_Repository.GetOption(pURIList->GetURI(i), &opt)) return FALSE;
		if (opt.bFolder) {
			TreeViewFolderItem *pItem = new TreeViewFolderItem();
			pView->InsertFolder(hParent, pURIList->GetTitle(i), pItem, TRUE);
		} else {
			if (!pView->InsertFile(hParent, pURIList->GetURI(i), pURIList->GetTitle(i), TRUE, FALSE)) return FALSE;
		}
	}

	return TRUE;
}

BOOL TreeViewFolderItem::ExecApp(MemoManager *pMgr, MemoSelectView *pView, ExeAppType nType)
{
	TomboURI sURI;
	if (!pView->GetURI(&sURI)) return FALSE;
	g_Repository.ExecuteAssoc(&sURI, nType);
	return TRUE;
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
		LPCTSTR pTitle = pNote->GetTitle();
		pView->InsertFile(hItem, pNote->GetURI(), pTitle, TRUE, TRUE);
	}
	pStore->FreeArray();
	return TRUE;
}

BOOL TreeViewVirtualFolder::IsOperationEnabled(MemoSelectView *pView, OpType op)
{
	DWORD nOpMatrix = OpNewMemo;
	return (nOpMatrix & op) != 0;
}
