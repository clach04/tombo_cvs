#ifndef MEMOSELECTVIEW_H
#define MEMOSELECTVIEW_H

class MemoManager;
class MemoNote;
class TString;
class TreeViewItem;
class MemoLocator;

class MemoSelectView {
	HWND hViewWnd;
	HIMAGELIST hImageList;

	MemoManager *pMemoMgr;

	// Cut/Copy/Paste用
	TreeViewItem *pClipItem; // Cut/Copy元Dispatcher
	BOOL bCut;

	// 自動切換えモードフラグ
	BOOL bAutoLoadMode;
	BOOL bSingleClickMode;

	void SetShareArea(TreeViewItem *pItem);

	void DeleteOneItem(HTREEITEM hItem);
	void DeleteItemsRec(HTREEITEM hFirst);

	// ツリーの開閉
	void ToggleExpandFolder(HTREEITEM hItem, UINT stat);

	///////////////////////////////////

	void SetIcon(TreeViewItem* p, DWORD nStatus);

	///////////////////////////////////
	LRESULT EditLabel(TVITEM *pItem);


public:
	/////////////////////////////
	// Initialize functions

	MemoSelectView() : hViewWnd(NULL), pMemoMgr(NULL), bAutoLoadMode(FALSE), bSingleClickMode(FALSE) {}
	BOOL Init(MemoManager *p) { pMemoMgr = p; bCut = FALSE; pClipItem = NULL; return TRUE; }
	BOOL Create(LPCTSTR pName, RECT &r, HWND hParent, DWORD nID, HINSTANCE hInst, HFONT hFont);

	/////////////////////////////
	// 表示・画面関連

	BOOL Show(int nCmdShow);
	void SetFocus();
	void SetFont(HFONT hFont);

	LRESULT OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);

	void OnActionButton(HWND hWnd);
	BOOL OnHotKey(HWND hWnd, WPARAM wParam);
	void OnGetFocus();
	void OnDelete(HTREEITEM hItem, TreeViewItem *pItem);
	void OnEncrypt(TreeViewItem *pItem);
	void OnDecrypt(TreeViewItem *pItem);
	void OnEditLabel(HTREEITEM hItem);

	// クリップボード(もどき)操作用
	void OnCut(TreeViewItem *pItem);
	void OnCopy(TreeViewItem *pItem);
	void OnPaste();

	// ウィンドウサイズ取得
	void GetSize(LPWORD pWidth, LPWORD pHeight);

	void MoveWindow(DWORD x, DWORD y, DWORD nWidth, DWORD nHeight);

	/////////////////////////////
	// プロパティ操作関連

	// 自動切換えモード
	void SetAutoLoadMode(BOOL bMode) { bAutoLoadMode = bMode; }
	BOOL IsAutoLoadMode() { return bAutoLoadMode; }

	void SetSingleClickMode(BOOL bMode) { bSingleClickMode = bMode; }
	BOOL IsSingleClickMode() { return bSingleClickMode; }

	/////////////////////////////
	// 検索
//	BOOL Search(BOOL bFirstSearch, BOOL bForward);

	/////////////////////////////
	// ビューアイテム操作関連

	BOOL InitTree();
	BOOL DeleteAllItem();

	void TreeExpand(HTREEITEM hItem);
	void TreeCollapse(HTREEITEM hItem);

	// 新規メモ生成時処理
	HTREEITEM NewMemoCreated(MemoNote *pNote, LPCTSTR pHeadLine, HTREEITEM hItem);

	BOOL InsertFile(HTREEITEM hParent, LPCTSTR pPrefix, LPCTSTR pFile);
	BOOL InsertFile(HTREEITEM hParent, MemoNote *pNote, LPCTSTR pTitle, BOOL bInsertLast);

	// if bInsertLast is TRUE, Insert folder without sorting.
	HTREEITEM InsertFolder(HTREEITEM hParent, LPCTSTR pName, TreeViewItem *tvi, BOOL bInsertLast);

	// TreeViewItemの状態が変わったことによるビューへの変更依頼
	BOOL UpdateItemStatusNotify(TreeViewItem *pItem, LPCTSTR pNewHeadLine);

	// 現在選択しているアイテムのパスを取得する。
	// 選択しているアイテムがフォルダの場合、自身までのパスを含む
	HTREEITEM GetPathForNewItem(TString *pPath, TreeViewItem *pItem = NULL);

	HTREEITEM GetTreeItemFromPath(LPCTSTR pPath);

	// ノードの選択
	void SelectNote(MemoNote *pNote);

	BOOL CreateNewFolder(HTREEITEM hItem, LPCTSTR pFolder);
	BOOL GetHeadLine(MemoNote *pNote, LPTSTR pHeadLine, DWORD nLen);

	// ヘッドライン文字列の書き換え
	BOOL UpdateHeadLine(MemoLocator *pLoc, LPCTSTR pHeadLine);

	// 現在選択されているアイテムと関連付けられているMemoNoteを返す。
	// pItemが指定されている場合にはHTREEITEMも返す。
	// 選択されていない場合には戻り値としてNULLを返す。
	TreeViewItem *GetCurrentItem(HTREEITEM *pItem = NULL);

	TreeViewItem *GetTVItem(HTREEITEM h);

	BOOL IsCliped(TreeViewItem* p) { return p == pClipItem; }
	LONG GetItem(TV_ITEM *p) { return TreeView_GetItem(hViewWnd, p); }
	BOOL SetTVItem(HTREEITEM h, TreeViewItem *p);

	LPTSTR GeneratePath(HTREEITEM hItem, LPTSTR pBuf, DWORD nSiz);

	// Choose specified notes. if tree is collapsed, expand one.
	BOOL ShowItem(LPCTSTR pPath);

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