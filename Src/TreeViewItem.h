#ifndef TREEVIEWITEM_H
#define TREEVIEWITEM_H

#include <commctrl.h>

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
class TomboURI;

#include "MemoManager.h"
#include "VarBuffer.h"

/////////////////////////////////////////////
//  TreeView Node
/////////////////////////////////////////////

class TreeViewItem {
	BOOL bHasMultiItem;
	HTREEITEM hItem;

public:
	///////////////////////////////////////////////////////
	// constants

	enum OpType {
		OpDelete	= (1 << 0),
		OpRename	= (1 << 1),
		OpEncrypt	= (1 << 2),
		OpDecrypt	= (1 << 3),
		OpNewMemo	= (1 << 4),
		OpNewFolder = (1 << 5),
		OpCut		= (1 << 6),
		OpCopy		= (1 << 7),
		OpPaste		= (1 << 8),
		OpGrep		= (1 << 9),
		OpLink		= (1 << 10)
	};

	enum ExeAppType {
		ExecType_Assoc,
		ExecType_ExtApp1,
		ExecType_ExtApp2,
	};

	///////////////////////////////////////////////////////
	// TreeViewItem specific methods

	TreeViewItem(BOOL bMultiItem);
	virtual ~TreeViewItem();

	HTREEITEM GetViewItem();
	void SetViewItem(HTREEITEM h);

	BOOL HasMultiItem() { return bHasMultiItem; }

	///////////////////////////////////////////////////////

	// is operation enabled?
	virtual BOOL IsOperationEnabled(MemoSelectView *pView, OpType op);

	//  node operators
	virtual BOOL Move(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR *ppErr) = 0;
	virtual BOOL Copy(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR *ppErr) = 0;
	virtual BOOL Rename(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR pNewName) = 0;
	virtual BOOL Delete(MemoManager *pMgr, MemoSelectView *pView) = 0;
		// MemoNote object that has TreeViewItem is also deleted.
		// "this" object is not deleted, so delete manually if return value is TRUE.
	virtual BOOL Encrypt(MemoManager *pMgr, MemoSelectView *pView) = 0;
	virtual BOOL Decrypt(MemoManager *pMgr, MemoSelectView *pView) = 0;

	virtual BOOL ExecApp(MemoManager *pMgr, MemoSelectView *pView, ExeAppType nType) = 0;

	// Compare item order
	virtual DWORD ItemOrder() = 0;
	virtual DWORD GetIcon(MemoSelectView *pView, DWORD nStatus) = 0;

	// Get path information

	// if the URI is tombo://default/aaa/bbb/ccc/ddd.txt ,
	// result of the GetFolderPath is \aaa\bbb\ccc\ 
	virtual BOOL GetURIItem(MemoSelectView *pView, TString *pItem) = 0;

	virtual BOOL OpenMemo(MemoSelectView *pView, DWORD nOption);
	virtual BOOL LoadMemo(MemoSelectView *pView, BOOL bAskPass);
	virtual BOOL IsUseDetailsView();
};

/////////////////////////////////////////////
//  File
/////////////////////////////////////////////
class TreeViewFileItem : public TreeViewItem {
protected:
	class Locator {
		TomboURI *pURI;
	public:
		Locator() : pURI(NULL) {}
		~Locator();
		const TomboURI *getURI() const { return pURI; }
		void set(const TomboURI *pURI);
	};

	Locator loc;
	BOOL bIsEncrypted;
public:
	TreeViewFileItem();
	~TreeViewFileItem();

	////////////////////////////////
	// class specific methods

	void SetNote(const TomboURI *p);

	////////////////////////////////
	// inherited methods
	BOOL IsOperationEnabled(MemoSelectView *pView, OpType op);

	BOOL Move(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR *ppErr);
	BOOL Copy(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR *ppErr);
	BOOL Delete(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Encrypt(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Decrypt(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Rename(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR pNewName);
	BOOL ExecApp(MemoManager *pMgr, MemoSelectView *pView, ExeAppType nType);

	DWORD GetIcon(MemoSelectView *pView, DWORD nStatus);
	DWORD ItemOrder();

	BOOL GetURIItem(MemoSelectView *pView, TString *pItem);

	BOOL OpenMemo(MemoSelectView *pView, DWORD nOption);
	BOOL LoadMemo(MemoSelectView *pView, BOOL bAskPass);

	BOOL GetURI(TString *pURI); 

	BOOL IsUseDetailsView();
	BOOL IsEncrypted() { return bIsEncrypted; }
};

/////////////////////////////////////////////
//  Real folder
/////////////////////////////////////////////

class TreeViewFolderItem : public TreeViewItem {
public:
	TreeViewFolderItem();

	////////////////////////////////
	// multi node methods

	virtual BOOL Expand(MemoSelectView *pView);

	////////////////////////////////
	// inherited methods

	BOOL IsOperationEnabled(MemoSelectView *pView, OpType op);

	BOOL Move(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR *ppErr);
	BOOL Copy(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR *ppErr);
	BOOL Rename(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR pNewName);
	BOOL Delete(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Encrypt(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Decrypt(MemoManager *pMgr, MemoSelectView *pView);
	BOOL ExecApp(MemoManager *pMgr, MemoSelectView *pView, ExeAppType nType);

	DWORD GetIcon(MemoSelectView *pView, DWORD nStatus);
	DWORD ItemOrder();

	BOOL GetURIItem(MemoSelectView *pView, TString *pItem);
};


/////////////////////////////////////////////
//  File link
/////////////////////////////////////////////
class TreeViewFileLink : public TreeViewFileItem {
public:
	const TomboURI *GetRealURI() const { return loc.getURI(); }

	////////////////////////////////
	// inherited methods

	BOOL IsOperationEnabled(MemoSelectView *pView, OpType op);

	BOOL OpenMemo(MemoSelectView *pView, DWORD nOption);
	BOOL LoadMemo(MemoSelectView *pView, BOOL bAskPass);

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

	////////////////////////////
	// class specific methods

	BOOL AddSearchResult(MemoSelectView *pView, const VFInfo *p);
	BOOL InsertVirtualFolder(MemoSelectView *pView, LPCTSTR pName, VFDirectoryGenerator *pGen, VFStore *pStore);

	////////////////////////////////
	// multi node methods

	BOOL Expand(MemoSelectView *pView);

	////////////////////////////////
	// inherited methods

	BOOL IsOperationEnabled(MemoSelectView *pView, OpType op);

	DWORD GetIcon(MemoSelectView *pView, DWORD nStatus);
	BOOL GetURIItem(MemoSelectView *pView, TString *pItem);
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

	////////////////////////////
	// class specific methods

	// pGen's life scope is control under TreeViewVirtualFolder.
	// don't delete pGen after calling SetGenerator.
	BOOL SetGenerator(VFDirectoryGenerator *pGen);

	// pStore's  life scope is control under TreeViewVirtualFolder.
	// don't delete pStore after calling SetStore.
	BOOL SetStore(VFStore *pStore);

	////////////////////////////////
	// multi node methods

	BOOL Expand(MemoSelectView *pView);

	////////////////////////////////
	// inherited methods

	BOOL IsOperationEnabled(MemoSelectView *pView, OpType op);

	DWORD GetIcon(MemoSelectView *pView, DWORD nStatus);
};


#endif
