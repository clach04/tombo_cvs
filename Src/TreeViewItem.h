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
	// �A�C�e���ɑ΂��鑀��

	// ���̃A�C�e���Ɠ����f�[�^�����ݑI�����Ă���ʒu�Ɠ���ɕۑ�����
	virtual BOOL Move(MemoManager *pMgr, MemoSelectView *pView) = 0;

	// ���̃A�C�e���Ɠ����f�[�^�����ݑI�����Ă���ʒu�Ɠ���ɃR�s�[����
	virtual BOOL Copy(MemoManager *pMgr, MemoSelectView *pView) = 0;

	// �f�[�^���̂��폜����B�ێ����Ă���MemoNote�ɂ��Ă�delete�����
	// this�ɂ��Ă�delete����Ȃ����߁A�߂�l��TRUE�̏ꍇ�̂݁Adelete���s���K�v������
	virtual BOOL Delete(MemoManager *pMgr, MemoSelectView *pView) = 0;


	// �������̖̂���(�t�@�C����/�f�B���N�g����)��ύX����
	virtual BOOL Rename(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR pNewName) = 0;

	virtual BOOL Encrypt(MemoManager *pMgr, MemoSelectView *pView) = 0;
	virtual BOOL Decrypt(MemoManager *pMgr, MemoSelectView *pView) = 0;

	///////////////////////////////////////////////////////

	// �A�C�R�����̎擾

	virtual DWORD GetIcon(MemoSelectView *pView, DWORD nStatus) = 0;

	///////////////////////////////////////////////////////
	// �A�C�e���Ԃ̔�r
	virtual DWORD ItemOrder() = 0;

	BOOL HasMultiItem() { return bHasMultiItem; }

	virtual MemoLocator ToLocator();
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

	DWORD GetIcon(MemoSelectView *pView, DWORD nStatus);

	DWORD ItemOrder();

	void SetNote(MemoNote *p) { pNote = p; }
	MemoNote *GetNote() { return pNote; }

	MemoLocator ToLocator();
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

	DWORD GetIcon(MemoSelectView *pView, DWORD nStatus);
	DWORD ItemOrder();

	virtual BOOL Expand(MemoSelectView *pView);
};

/////////////////////////////////////////////
//  Virtual folder definition
/////////////////////////////////////////////
#define VFINFO_FLG_CASESENSITIVE	1
#define VFINFO_FLG_CHECKCRYPTED		2
#define VFINFO_FLG_FILENAMEONLY		4
#define VFINFO_FLG_PERSISTENT		8
#define VFINFO_FLG_NEGATE			16

class VFInfo {
public:
	VFInfo() : pName(NULL), pPath(NULL), pRegex(NULL) {}

	void Release() { delete[] pName; delete[] pPath; delete[] pRegex; }

	LPTSTR pName;
	LPTSTR pPath;
	LPTSTR pRegex;
	DWORD nFlag;
};

/////////////////////////////////////////////
//  Virtual Folder (Root)
/////////////////////////////////////////////

class TreeViewVirtualFolderRoot : public TreeViewFolderItem {
	TVector<VFInfo> vbInfo;
public:
	TreeViewVirtualFolderRoot();
	~TreeViewVirtualFolderRoot();
	BOOL Init();

	DWORD GetIcon(MemoSelectView *pView, DWORD nStatus);

	BOOL Expand(MemoSelectView *pView);

	////////////////////////////
	// Virtual folder operator 
	BOOL AddSearchResult(MemoSelectView *pView, VFInfo *p);

	BOOL StreamObjectsFactory(VFInfo *pInfo, TreeViewVirtualFolder **ppVf, VFDirectoryGenerator **ppGen, VFStore **ppStore);
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

	// pGen's life scope is control under TreeViewVirtualFolder.
	// don't delete pGen after calling SetGenerator.
	BOOL SetGenerator(VFDirectoryGenerator *pGen);

	// pStore's  life scope is control under TreeViewVirtualFolder.
	// don't delete pStore after calling SetStore.
	BOOL SetStore(VFStore *pStore);
};


#endif
