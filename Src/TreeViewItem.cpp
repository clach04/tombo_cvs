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
		// ���ݕ\������Ă��郁���̕\�����L�����Z������
		pMgr->NewMemo();
	}
	// �t�@�C���̍폜
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
	// ���[�U�ւ̈ӎv�m�F
	if (TomboMessageBox(NULL, MSG_CONFIRM_DELETE, MSG_DELETE_TTL, MB_ICONQUESTION | MB_OKCANCEL) != IDOK) return FALSE;

	if (g_Property.IsUseTwoPane() && pMgr->IsNoteDisplayed(pNote->MemoPath())) {
		// ���ݕ\������Ă��郁���̕\�����L�����Z������
		pMgr->NewMemo();
	}
	// �t�@�C���̍폜
	if (!pNote->DeleteMemoData()) {
		TomboMessageBox(NULL, MSG_DELETE_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
		return FALSE;
	}
	return TRUE;
}

BOOL TreeViewFileItem::Encrypt(MemoManager *pMgr, MemoSelectView *pView)
{
	// ���ɈÍ�������Ă����疳������
	if (pNote->IsEncrypted()) return TRUE;

	// �ڍ׃r���[�ɕ\������Ă��郁�����Í������悤�Ƃ��Ă���̂ł���΁A
	// �ۑ����Ă���
	if (g_Property.IsUseTwoPane() && pMgr->IsNoteDisplayed(pNote->MemoPath())) {
		pMgr->InactiveDetailsView();
	}

	// �Í���
	BOOL b;
	TString sHeadLine;
	MemoNote *p = pNote->Encrypt(pMgr->GetPasswordManager(), &sHeadLine, &b);
	if (p == NULL) return FALSE;

	// �Í����O�̃������폜
	if (!pNote->DeleteMemoData()) {
		MessageBox(NULL, MSG_DELETE_PREV_CRYPT_MEMO_FAILED, TEXT("TOMBO"), MB_ICONWARNING | MB_OK);
	}

	// TreeViewItem�̕ێ�����MemoNote���Í������ꂽ���̂ɒu��������
	delete pNote;
	pNote = p;

	// �Í����ɔ����A�C�R���E�w�b�h���C�����ύX�ɂȂ�\��������̂ōX�V�˗�
	if (!pView->UpdateItemStatusNotify(this, sHeadLine.Get())) {
		MessageBox(NULL, TEXT("UpdateItemStatusNotify failed"), TEXT("DEBUG"), MB_OK);// XXXX_DEBUG
		return FALSE;
	}
	return TRUE;
}

BOOL TreeViewFileItem::Decrypt(MemoManager *pMgr, MemoSelectView *pView)
{
	// �����������疳������
	if (!pNote->IsEncrypted()) return TRUE;

	// �ڍ׃r���[�ɕ\������Ă��郁���𕜍������悤�Ƃ��Ă���̂ł���΁A
	// �ۑ����Ă���
	if (g_Property.IsUseTwoPane() && pMgr->IsNoteDisplayed(pNote->MemoPath())) {
		pMgr->InactiveDetailsView();
	}

	// ������
	BOOL b;
	TString sHeadLine;
	MemoNote *p = pNote->Decrypt(pMgr->GetPasswordManager(), &sHeadLine, &b);
	if (p == NULL) return FALSE;

	// �������O�̃������폜
	if (!pNote->DeleteMemoData()) {
		MessageBox(NULL, MSG_DEL_PREV_DECRYPT_MEMO_FAILED, TEXT("TOMBO"), MB_ICONWARNING | MB_OK);
	}

	// TreeViewItem�̕ێ�����MemoNote�𕜍������ꂽ���̂ɒu��������
	delete pNote;
	pNote = p;

	// �Í����ɔ����A�C�R���E�w�b�h���C�����ύX�ɂȂ�\��������̂ōX�V�˗�
	if (!pView->UpdateItemStatusNotify(this, sHeadLine.Get())) {
		return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////
//  ���̕ύX
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

/////////////////////////////////////////////
//  Get MemoLocator

MemoLocator TreeViewFileItem::ToLocator()
{
	return MemoLocator(pNote, GetViewItem());
}


/////////////////////////////////////////////
//  Folder
/////////////////////////////////////////////

TreeViewFolderItem::TreeViewFolderItem() : TreeViewItem(TRUE)
{
}

BOOL TreeViewFolderItem::Move(MemoManager *pMgr, MemoSelectView *pView)
{
	// Src�p�X�̎擾
	TCHAR buf[MAX_PATH];
	TString sCurrentPath;
	HTREEITEM hItem = GetViewItem();
	LPTSTR pCurrentPath = pView->GeneratePath(hItem, buf, MAX_PATH);
	if (!sCurrentPath.AllocFullPath(pCurrentPath)) return FALSE;

	// Dst�p�X�̎擾
	TString sDstPath, sDstFullPath;
	HTREEITEM hParent = pView->GetPathForNewItem(&sDstPath);
	if (!sDstFullPath.AllocFullPath(sDstPath.Get())) return FALSE;

	// �ړ����悤�Ƃ��Ă���t�H���_�ɏڍ׃r���[��Active�ɂȂ��Ă��郁����
	// ���݂��邩������Ȃ��̂ŏڍ׃r���[����UInActive�ɂ���
	pMgr->InactiveDetailsView();

	// �c���[��Collapse
	pView->TreeCollapse(GetViewItem());

	// �ړ�����
	MemoFolder mfFolder;
	if (!mfFolder.Init(sCurrentPath.Get())) return FALSE;
	if (!mfFolder.Move(sDstFullPath.Get())) return FALSE;	

	// �\���̒ǉ�
	TString sItemName;
	ChopFileSeparator(pCurrentPath);
	sItemName.GetPathTail(pCurrentPath);
	pView->CreateNewFolder(hParent, sItemName.Get());
	return TRUE;
}

BOOL TreeViewFolderItem::Copy(MemoManager *pMgr, MemoSelectView *pView)
{
	// Src�p�X�̎擾
	TCHAR buf[MAX_PATH];
	TString sCurrentPath;
	HTREEITEM hItem = GetViewItem();
	LPTSTR pCurrentPath = pView->GeneratePath(hItem, buf, MAX_PATH);
	if (!sCurrentPath.AllocFullPath(pCurrentPath)) return FALSE;

	// Dst�p�X�̎擾
	TString sDstPath, sDstFullPath;
	HTREEITEM hParent = pView->GetPathForNewItem(&sDstPath);
	if (!sDstFullPath.AllocFullPath(sDstPath.Get())) return FALSE;

	// �ړ����悤�Ƃ��Ă���t�H���_�ɏڍ׃r���[��Active�ɂȂ��Ă��郁����
	// ���݂��邩������Ȃ��̂ŏڍ׃r���[����UInActive�ɂ���
	pMgr->InactiveDetailsView();

	// �R�s�[�悪�����̃A�C�e�����ɂȂ�悤�␳
	TString sItemName;
	ChopFileSeparator(pCurrentPath);
	sItemName.GetPathTail(pCurrentPath);
	sDstFullPath.StrCat(sItemName.Get());
	sDstFullPath.StrCat(TEXT("\\"));

	// �t�@�C���̃R�s�[
	MemoFolder mf;
	mf.Init(sCurrentPath.Get());
	BOOL bResult = mf.Copy(sDstFullPath.Get());

	// �t�H���_�̏C��
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

	// �ړ����悤�Ƃ��Ă���t�H���_�ɏڍ׃r���[��Active�ɂȂ��Ă��郁����
	// ���݂��邩������Ȃ��̂ŏڍ׃r���[����UInActive�ɂ���
	pMgr->InactiveDetailsView();

	// �c���[��Collapse
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

	// �X�e�[�^�X�̎擾
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
//  ���̕ύX
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
			if (!pView->InsertFile(hParent, p->pNote, q, TRUE)) return FALSE;
		}
	}
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

BOOL TreeViewVirtualFolderRoot::Expand(MemoSelectView *pView)
{
	HTREEITEM hParent = GetViewItem();

	TString sVFpath;
	if (!sVFpath.Join(g_Property.PropertyDir(), TEXT("\\"), TOMBO_VFOLDER_DEF_FILE)) return FALSE;

	TSParser tp;
	if (!tp.Init(sVFpath.Get(), pView, hParent)) return FALSE;
	if (!tp.Compile()) return FALSE;

	return TRUE;
}

/////////////////////////////////////////////
/////////////////////////////////////////////
//  Virtual folder (non-root)
/////////////////////////////////////////////
/////////////////////////////////////////////

TreeViewVirtualFolder::TreeViewVirtualFolder() : pGenerator(NULL), pStore(NULL), pTail(NULL)
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
	pTail = p;
	return TRUE;
}

BOOL TreeViewVirtualFolder::SetStore(VFStore *p)
{
	pStore = p;

	if (!pTail) return FALSE;
	pTail->SetNext(p);
	pTail = NULL;

	return TRUE;
}

BOOL TreeViewVirtualFolder::Expand(MemoSelectView *pView)
{
	HTREEITEM hItem = GetViewItem();

	// set waiting cursor
	HCURSOR hOrigCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

	// scanning.
	if (!pGenerator || !pStore || 
		!pGenerator->Prepare() || 
		!pGenerator->Activate() ||
		!pGenerator->PostActivate()) {

		SetCursor(hOrigCursor);
		return FALSE;
	}

	// insert to tree
	DWORD n = pStore->NumItem();
	VFNote **ppNotes = pStore->GetNotes();
	for (DWORD i = 0; i < n; i++) {
		MemoNote *p = ppNotes[i]->GetNote();
		LPCTSTR pTitle = ppNotes[i]->GetFileName();
		pView->InsertFile(hItem, p, pTitle, TRUE);
	}
	pStore->FreeArray(); ppNotes = NULL;

	// set normal cursor
	SetCursor(hOrigCursor);
	return TRUE;
}
