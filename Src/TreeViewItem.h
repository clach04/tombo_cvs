#ifndef TREEVIEWITEM_H
#define TREEVIEWITEM_H

class MemoNote;
class MemoSelectView;
class MemoManager;
class VFDirectoryGenerator;
class VFStore;
class VFStream;
class MemoLoctor;
class TreeViewVirtualFolder;
class TString;
class VFManager;
class VFInfo;

#include "MemoManager.h"
#include "VarBuffer.h"

/////////////////////////////////////////////
//  TreeView�̃A�C�e���̒��ۉ�
/////////////////////////////////////////////

class TreeViewItem {
	BOOL bHasMultiItem;
	HTREEITEM hItem;

public:
	TreeViewItem(BOOL bMultiItem);
	virtual ~TreeViewItem();

	HTREEITEM GetViewItem();
	void SetViewItem(HTREEITEM h);

	///////////////////////////////////////////////////////
	// Item operator

	// ���̃A�C�e���Ɠ����f�[�^�����ݑI�����Ă���ʒu�Ɠ���ɕۑ�����
	virtual BOOL Move(MemoManager *pMgr, MemoSelectView *pView) = 0;

	// ���̃A�C�e���Ɠ����f�[�^�����ݑI�����Ă���ʒu�Ɠ���ɃR�s�[����
	virtual BOOL Copy(MemoManager *pMgr, MemoSelectView *pView) = 0;

	// delete node. 
	// MemoNote object that has TreeViewItem is also deleted.
	// "this" object is not deleted, so delete manually if return value is TRUE.

	virtual BOOL Delete(MemoManager *pMgr, MemoSelectView *pView) = 0;
	virtual BOOL CanDelete(MemoSelectView *pView);

	// rename node
	virtual BOOL Rename(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR pNewName) = 0;
	virtual BOOL CanRename(MemoSelectView *pView);


	virtual BOOL Encrypt(MemoManager *pMgr, MemoSelectView *pView) = 0;
	virtual BOOL Decrypt(MemoManager *pMgr, MemoSelectView *pView) = 0;
	virtual BOOL CanEncrypt(MemoSelectView *pView);
	virtual BOOL CanDecrypt(MemoSelectView *pView);
	virtual BOOL CanNewMemo(MemoSelectView *pView);
	virtual BOOL CanNewFolder(MemoSelectView *pView);
	virtual BOOL CanCut(MemoSelectView *pView);
	virtual BOOL CanCopy(MemoSelectView *pView);
	virtual BOOL CanPaste(MemoSelectView *pView);
	virtual BOOL CanGrep(MemoSelectView *pVIew);

	///////////////////////////////////////////////////////
	// �A�C�R�����̎擾

	virtual DWORD GetIcon(MemoSelectView *pView, DWORD nStatus) = 0;

	///////////////////////////////////////////////////////
	// �A�C�e���Ԃ̔�r
	virtual DWORD ItemOrder() = 0;

	BOOL HasMultiItem() { return bHasMultiItem; }

	virtual MemoLocator ToLocator();

	///////////////////////////////////////////////////////
	// Get path information
	virtual BOOL GetFolderPath(MemoSelectView *pView, TString *pPath) = 0;

};

/////////////////////////////////////////////
//  File
/////////////////////////////////////////////
class TreeViewFileItem : public TreeViewItem {
protected:
	MemoNote *pNote;
public:
	TreeViewFileItem();
	~TreeViewFileItem();

	BOOL Move(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Copy(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Delete(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Encrypt(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Decrypt(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Rename(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR pNewName);
	virtual BOOL CanRename(MemoSelectView *pView);

	DWORD GetIcon(MemoSelectView *pView, DWORD nStatus);
	virtual BOOL CanDelete(MemoSelectView *pView);

	virtual BOOL CanEncrypt(MemoSelectView *pView);
	virtual BOOL CanDecrypt(MemoSelectView *pView);
	virtual BOOL CanNewMemo(MemoSelectView *pView);
	virtual BOOL CanNewFolder(MemoSelectView *pView);
	virtual BOOL CanCut(MemoSelectView *pView);
	virtual BOOL CanCopy(MemoSelectView *pView);
	virtual BOOL CanPaste(MemoSelectView *pView);
	virtual BOOL CanGrep(MemoSelectView *pVIew);

	DWORD ItemOrder();

	void SetNote(MemoNote *p) { pNote = p; }
	MemoNote *GetNote() { return pNote; }

	MemoLocator ToLocator();
	BOOL GetFolderPath(MemoSelectView *pView, TString *pPath);

};

/////////////////////////////////////////////
//  Real folder
/////////////////////////////////////////////

class TreeViewFolderItem : public TreeViewItem {
public:
	TreeViewFolderItem();

	BOOL Move(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Copy(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Delete(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Encrypt(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Decrypt(MemoManager *pMgr, MemoSelectView *pView);

	BOOL Rename(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR pNewName);
	virtual BOOL CanRename(MemoSelectView *pView);

	virtual BOOL CanEncrypt(MemoSelectView *pView);
	virtual BOOL CanDecrypt(MemoSelectView *pView);
	virtual BOOL CanNewMemo(MemoSelectView *pView);
	virtual BOOL CanNewFolder(MemoSelectView *pView);
	virtual BOOL CanCut(MemoSelectView *pView);
	virtual BOOL CanCopy(MemoSelectView *pView);
	virtual BOOL CanPaste(MemoSelectView *pView);
	virtual BOOL CanGrep(MemoSelectView *pVIew);

	DWORD GetIcon(MemoSelectView *pView, DWORD nStatus);
	DWORD ItemOrder();

	virtual BOOL CanDelete(MemoSelectView *pView);
	virtual BOOL Expand(MemoSelectView *pView);
	BOOL GetFolderPath(MemoSelectView *pView, TString *pPath);

};


/////////////////////////////////////////////
//  File link
/////////////////////////////////////////////
class TreeViewFileLink : public TreeViewFileItem {
public:
	virtual BOOL CanDelete(MemoSelectView *pView);
	virtual BOOL CanRename(MemoSelectView *pView);
	virtual BOOL CanEncrypt(MemoSelectView *pView);
	virtual BOOL CanDecrypt(MemoSelectView *pView);
	virtual BOOL CanNewMemo(MemoSelectView *pView);
	virtual BOOL CanNewFolder(MemoSelectView *pView);
	virtual BOOL CanCut(MemoSelectView *pView);
	virtual BOOL CanCopy(MemoSelectView *pView);
	virtual BOOL CanPaste(MemoSelectView *pView);
	virtual BOOL CanGrep(MemoSelectView *pVIew);

	virtual BOOL GetFolderPath(MemoSelectView *pView, TString *pPath);
};

/////////////////////////////////////////////
//  Virtual Folder (Root)
/////////////////////////////////////////////

class TreeViewVirtualFolderRoot : public TreeViewFolderItem {
	VFManager *pManager;
public:
	TreeViewVirtualFolderRoot();
	~TreeViewVirtualFolderRoot();
	BOOL Init(VFManager *pManager);

	DWORD GetIcon(MemoSelectView *pView, DWORD nStatus);

	BOOL Expand(MemoSelectView *pView);

	////////////////////////////
	// Virtual folder operator 
	BOOL AddSearchResult(MemoSelectView *pView, const VFInfo *p);
	BOOL InsertVirtualFolder(MemoSelectView *pView, LPCTSTR pName, VFDirectoryGenerator *pGen, VFStore *pStore);

	class ItemIterator {
		TreeViewVirtualFolderRoot *pRoot;
		MemoSelectView *pView;
		HWND hWnd;
		HTREEITEM hCurrentItem;
	public:
		ItemIterator(MemoSelectView *pView, TreeViewVirtualFolderRoot *p);
		BOOL First();
		BOOL Next();
		BOOL Get(TString *pLabel);

	};

	ItemIterator *GetIterator(MemoSelectView *pView);

	////////////////////////////
	// common methods

	BOOL CanDelete(MemoSelectView *pView); 
	BOOL CanRename(MemoSelectView *pView);
	virtual BOOL CanEncrypt(MemoSelectView *pView);
	virtual BOOL CanDecrypt(MemoSelectView *pView);
	virtual BOOL CanNewMemo(MemoSelectView *pView);
	virtual BOOL CanNewFolder(MemoSelectView *pView);
	virtual BOOL CanCut(MemoSelectView *pView);
	virtual BOOL CanCopy(MemoSelectView *pView);
	virtual BOOL CanPaste(MemoSelectView *pView);
	virtual BOOL CanGrep(MemoSelectView *pVIew);

	virtual BOOL GetFolderPath(MemoSelectView *pView, TString *pPath);
};


/////////////////////////////////////////////
//  Virtual Folder (Non-root)
/////////////////////////////////////////////

class TreeViewVirtualFolder : public TreeViewFolderItem {
protected:
	VFDirectoryGenerator *pGenerator;
	VFStore *pStore;
public:
	TreeViewVirtualFolder();
	~TreeViewVirtualFolder();

	DWORD GetIcon(MemoSelectView *pView, DWORD nStatus);

	BOOL Expand(MemoSelectView *pView);
	BOOL CanDelete(MemoSelectView *pView);
	BOOL CanRename(MemoSelectView *pView);
	virtual BOOL CanEncrypt(MemoSelectView *pView);
	virtual BOOL CanDecrypt(MemoSelectView *pView);
	virtual BOOL CanNewMemo(MemoSelectView *pView);
	virtual BOOL CanNewFolder(MemoSelectView *pView);
	virtual BOOL CanCut(MemoSelectView *pView);
	virtual BOOL CanCopy(MemoSelectView *pView);
	virtual BOOL CanPaste(MemoSelectView *pView);
	virtual BOOL CanGrep(MemoSelectView *pVIew);

	virtual BOOL GetFolderPath(MemoSelectView *pView, TString *pPath);

	// pGen's life scope is control under TreeViewVirtualFolder.
	// don't delete pGen after calling SetGenerator.
	BOOL SetGenerator(VFDirectoryGenerator *pGen);

	// pStore's  life scope is control under TreeViewVirtualFolder.
	// don't delete pStore after calling SetStore.
	BOOL SetStore(VFStore *pStore);
};


#endif
