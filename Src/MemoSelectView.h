#ifndef MEMOSELECTVIEW_H
#define MEMOSELECTVIEW_H

#include <commctrl.h>

class MemoManager;
class MemoNote;
class TString;
class TreeViewItem;
class MemoLocator;
class TreeViewVirtualFolderRoot;
class VFManager;
class VFInfo;

class MemoSelectView {

	/////////////////////////////
	// Window related members

	HWND hViewWnd;

	// root node
	HTREEITEM hMemoRoot;
	HTREEITEM hSearchRoot;

	HIMAGELIST hSelectViewImgList;

	MemoManager *pMemoMgr;

	/////////////////////////////
	// for Cut/Copy/Paste
	TreeViewItem *pClipItem;
	BOOL bCut;

	// mode flags
	BOOL bAutoLoadMode;
	BOOL bSingleClickMode;

	void SetShareArea(TreeViewItem *pItem);

	void DeleteOneItem(HTREEITEM hItem);
	void DeleteItemsRec(HTREEITEM hFirst);

	///////////////////////////////////

	void SetIcon(TreeViewItem* p, DWORD nStatus);

	///////////////////////////////////
	LRESULT EditLabel(TVITEM *pItem);

protected:
	HTREEITEM GetRootItem(LPCTSTR pRep);

public:
	/////////////////////////////
	// Initialize functions

	MemoSelectView() : hViewWnd(NULL), pMemoMgr(NULL), bAutoLoadMode(FALSE), bSingleClickMode(FALSE), hMemoRoot(NULL), hSearchRoot(NULL) {}
	BOOL Init(MemoManager *p) { pMemoMgr = p; bCut = FALSE; pClipItem = NULL; return TRUE; }
	BOOL Create(LPCTSTR pName, RECT &r, HWND hParent, DWORD nID, HINSTANCE hInst, HFONT hFont);

	/////////////////////////////
	// display related funcs

	HWND GetHWnd() { return hViewWnd; }

	BOOL Show(int nCmdShow);
	void SetFocus();
	void SetFont(HFONT hFont);

	LRESULT OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam);

#if defined(PLATFORM_WIN32)
	void OnNotify_RClick();
#endif
	BOOL OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);

	void OnActionButton(HWND hWnd);
	BOOL OnHotKey(HWND hWnd, WPARAM wParam);
	void OnGetFocus();
	void OnDelete(HTREEITEM hItem, TreeViewItem *pItem);
	void OnEncrypt(TreeViewItem *pItem);
	void OnDecrypt(TreeViewItem *pItem);
	void OnEditLabel(HTREEITEM hItem);

	// psudo clipboard
	void OnCut(TreeViewItem *pItem);
	void OnCopy(TreeViewItem *pItem);
	void OnPaste();

	void GetSize(LPWORD pWidth, LPWORD pHeight);
	void GetSize(LPRECT pRect);
	void GetClientRect(LPRECT pRect);

	void MoveWindow(DWORD x, DWORD y, DWORD nWidth, DWORD nHeight);

	/////////////////////////////
	// accessor/mutator for flags

	MemoManager *GetManager() { return pMemoMgr; }

	// �����؊������[�h
	void SetAutoLoadMode(BOOL bMode) { bAutoLoadMode = bMode; }
	BOOL IsAutoLoadMode() { return bAutoLoadMode; }

	void SetSingleClickMode(BOOL bMode) { bSingleClickMode = bMode; }
	BOOL IsSingleClickMode() { return bSingleClickMode; }

	/////////////////////////////
	// Key
	void SelUpFolderWithoutOpen();
	void SelNextBrother();
	void SelPrevBrother();

	/////////////////////////////
	// �r���[�A�C�e������֘A

	BOOL InitTree(VFManager *pManager);
	BOOL DeleteAllItem();

	/////////////////////////////
	// Expand/collapse tree
	void ToggleExpandFolder(HTREEITEM hItem, UINT stat);
	void TreeExpand(HTREEITEM hItem);
	void TreeCollapse(HTREEITEM hItem);
	BOOL IsExpand(HTREEITEM hItem);

	HIMAGELIST GetImageList() { return hSelectViewImgList; }

	// �V�K��������������
	HTREEITEM NewMemoCreated(MemoNote *pNote, LPCTSTR pHeadLine, HTREEITEM hItem);

	BOOL InsertFile(HTREEITEM hParent, LPCTSTR pPrefix, LPCTSTR pFile);
	HTREEITEM InsertFile(HTREEITEM hParent, MemoNote *pNote, LPCTSTR pTitle, BOOL bInsertLast, BOOL bLink);

	// if bInsertLast is TRUE, Insert folder without sorting.
	HTREEITEM InsertFolder(HTREEITEM hParent, LPCTSTR pName, TreeViewItem *tvi, BOOL bInsertLast);

	// TreeViewItem�̏�Ԃ��ς�������Ƃɂ��r���[�ւ̕ύX�˗�
	BOOL UpdateItemStatusNotify(TreeViewItem *pItem, LPCTSTR pNewHeadLine);

	// ���ݑI�����Ă���A�C�e���̃p�X���擾����B
	// �I�����Ă���A�C�e�����t�H���_�̏ꍇ�A���g�܂ł̃p�X���܂�
	HTREEITEM GetPathForNewItem(TString *pPath, TreeViewItem *pItem = NULL);

	HTREEITEM GetParentItem(HTREEITEM hItem) { return TreeView_GetParent(hViewWnd, hItem); }

	// node selection
	void SelectNote(MemoNote *pNote);

	BOOL CreateNewFolder(HTREEITEM hItem, LPCTSTR pFolder);
	BOOL GetHeadLine(MemoNote *pNote, LPTSTR pHeadLine, DWORD nLen);

	// update headline string
	BOOL UpdateHeadLine(LPCTSTR pOldURI, LPCTSTR pNewURI, MemoNote *pNewNote);
	HTREEITEM GetItemFromURI(LPCTSTR pURI);

	// ���ݑI������Ă���A�C�e���Ɗ֘A�t�����Ă���TreeViewItem��Ԃ��B
	// pItem���w�肳��Ă���ꍇ�ɂ�HTREEITEM���Ԃ��B
	// �I������Ă��Ȃ��ꍇ�ɂ͖߂�l�Ƃ���NULL��Ԃ��B
	TreeViewItem *GetCurrentItem(HTREEITEM *pItem = NULL);

	TreeViewItem *GetTVItem(HTREEITEM h);

	BOOL IsCliped(TreeViewItem* p) { return p == pClipItem; }
	LONG GetItem(TV_ITEM *p) { return TreeView_GetItem(hViewWnd, p); }
	BOOL SetTVItem(HTREEITEM h, TreeViewItem *p);

	LPTSTR GeneratePath(HTREEITEM hItem, LPTSTR pBuf, DWORD nSiz);
	BOOL GetCurrentItemPath(TString *pPath);

	BOOL GetURI(TString *pURI, HTREEITEM hTarget = NULL);

	BOOL GetURINodeName(HTREEITEM h, LPTSTR pBuf, DWORD nBufLen);

	// Choose specified notes. if tree is collapsed, expand one.
	HTREEITEM ShowItem(LPCTSTR pPath, BOOL bSelChange = TRUE, BOOL bOpenNotes = TRUE);
	HTREEITEM ShowItemByURI(LPCTSTR pPath, BOOL bSelChange = TRUE, BOOL bOpenNotes = TRUE);

	/////////////////////////////
	// Virtual folder
	BOOL InsertVirtualFolder(const VFInfo *pInfo);

	TreeViewVirtualFolderRoot *GetVirtualFolderRoot();
	void CloseVFRoot();

	/////////////////////////////
	// Control menu
	void ControlMenu();
};

/////////////////////////////////////////
// ������(�A�C�R��)��Ԓ�`
/////////////////////////////////////////

// ��������
#define MEMO_VIEW_STATE_INIT			1

// �N���b�v�{�[�h�ɓ����Ă���
#define MEMO_VIEW_STATE_CLIPED_SET		(1 << 1)
#define MEMO_VIEW_STATE_CLIPED_CLEAR	(1 << 2)

// �J����Ă���
#define MEMO_VIEW_STATE_OPEN_SET		(1 << 3)
#define MEMO_VIEW_STATE_OPEN_CLEAR		(1 << 4)


#endif