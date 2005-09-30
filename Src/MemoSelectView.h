#ifndef MEMOSELECTVIEW_H
#define MEMOSELECTVIEW_H

class MemoManager;
class TString;
class TreeViewItem;
class MemoLocator;
class TreeViewVirtualFolderRoot;
class VFManager;
class VFInfo;
class TomboURI;

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

#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC) || defined(PLATFORM_PKTPC)
	void OnNotify_RClick(POINT pt);
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

	// 自動切換えモード
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
	// ビューアイテム操作関連

	BOOL InitTree(VFManager *pManager);
	BOOL DeleteAllItem();

	/////////////////////////////
	// Expand/collapse tree
	void ToggleExpandFolder(HTREEITEM hItem, UINT stat);
	void TreeExpand(HTREEITEM hItem);
	void TreeCollapse(HTREEITEM hItem);
	BOOL IsExpand(HTREEITEM hItem);

	HIMAGELIST GetImageList() { return hSelectViewImgList; }

	HTREEITEM InsertFile(HTREEITEM hParent, const TomboURI *pURI, LPCTSTR pTitle, BOOL bInsertLast, BOOL bLink);

	// update headline string
	BOOL UpdateHeadLine(LPCTSTR pOldURI, TomboURI *pNewURI, LPCTSTR pNewHeadLine);

	// if bInsertLast is TRUE, Insert folder without sorting.
	HTREEITEM InsertFolder(HTREEITEM hParent, LPCTSTR pName, TreeViewItem *tvi, BOOL bInsertLast);

	// TreeViewItemの状態が変わったことによるビューへの変更依頼
	BOOL UpdateItemStatusNotify(TreeViewItem *pItem, LPCTSTR pNewHeadLine);

	// 現在選択しているアイテムのパスを取得する。
	// 選択しているアイテムがフォルダの場合、自身までのパスを含む
	HTREEITEM GetPathForNewItem(TString *pPath, TreeViewItem *pItem = NULL);

	HTREEITEM GetParentItem(HTREEITEM hItem) { return TreeView_GetParent(hViewWnd, hItem); }

	BOOL CreateNewFolder(HTREEITEM hItem, LPCTSTR pFolder);


	// Search tree and get HTREEITEM
	HTREEITEM GetItemFromURI(LPCTSTR pURI);

	// 現在選択されているアイテムと関連付けられているTreeViewItemを返す。
	// pItemが指定されている場合にはHTREEITEMも返す。
	// 選択されていない場合には戻り値としてNULLを返す。
	TreeViewItem *GetCurrentItem(HTREEITEM *pItem = NULL);

	TreeViewItem *GetTVItem(HTREEITEM h);

	BOOL IsCliped(TreeViewItem* p) { return p == pClipItem; }
	LONG GetItem(TV_ITEM *p) { return TreeView_GetItem(hViewWnd, p); }
	BOOL SetTVItem(HTREEITEM h, TreeViewItem *p);

	LPTSTR GeneratePath(HTREEITEM hItem, LPTSTR pBuf, DWORD nSiz);

	BOOL GetURI(TString *pURI, HTREEITEM hTarget = NULL);
	BOOL GetURI(TomboURI *pURI, HTREEITEM hTarget = NULL);

	BOOL GetURINodeName(HTREEITEM h, LPTSTR pBuf, DWORD nBufLen);

	// Choose specified notes. if tree is collapsed, expand one.
	HTREEITEM ShowItemByURI(const TomboURI *pURI, BOOL bSelChange = TRUE, BOOL bOpenNotes = TRUE);

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
// メモの(アイコン)状態定義
/////////////////////////////////////////

// 初期化時
#define MEMO_VIEW_STATE_INIT			1

// クリップボードに入っている
#define MEMO_VIEW_STATE_CLIPED_SET		(1 << 1)
#define MEMO_VIEW_STATE_CLIPED_CLEAR	(1 << 2)

// 開かれている
#define MEMO_VIEW_STATE_OPEN_SET		(1 << 3)
#define MEMO_VIEW_STATE_OPEN_CLEAR		(1 << 4)


#endif