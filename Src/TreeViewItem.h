#ifndef TREEVIEWITEM_H
#define TREEVIEWITEM_H

class MemoNote;
class MemoSelectView;
class MemoManager;

/////////////////////////////////////////////
//  TreeViewのアイテムの抽象化
/////////////////////////////////////////////

class TreeViewItem {
	BOOL bHasMultiItem;
public:
	TreeViewItem(BOOL bMultiItem);
	virtual ~TreeViewItem();

	MemoNote *pNote;

	virtual HTREEITEM GetViewItem() = 0;
	virtual void SetViewItem(HTREEITEM h) = 0;

	///////////////////////////////////////////////////////
	// アイテムに対する操作

	// このアイテムと同じデータを現在選択している位置と同列に保存する
	virtual BOOL Move(MemoManager *pMgr, MemoSelectView *pView) = 0;

	// このアイテムと同じデータを現在選択している位置と同列にコピーする
	virtual BOOL Copy(MemoManager *pMgr, MemoSelectView *pView) = 0;

	// データ実体を削除する。保持しているMemoNoteについてもdeleteされる
	// thisについてはdeleteされないため、戻り値がTRUEの場合のみ、deleteを行う必要がある
	virtual BOOL Delete(MemoManager *pMgr, MemoSelectView *pView) = 0;


	// メモ実体の名称(ファイル名/ディレクトリ名)を変更する
	virtual BOOL Rename(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR pNewName) = 0;

	virtual BOOL Encrypt(MemoManager *pMgr, MemoSelectView *pView) = 0;
	virtual BOOL Decrypt(MemoManager *pMgr, MemoSelectView *pView) = 0;

	///////////////////////////////////////////////////////

	// アイコン情報の取得

	virtual DWORD GetIcon(MemoSelectView *pView, DWORD nStatus) = 0;

	///////////////////////////////////////////////////////
	// アイテム間の比較
	virtual DWORD ItemOrder() = 0;

	BOOL HasMultiItem() { return bHasMultiItem; }
};

/////////////////////////////////////////////
//  ファイル
/////////////////////////////////////////////
class TreeViewFileItem : public TreeViewItem {
protected:
	BOOL DeleteWithoutAsk(MemoManager *pMgr, MemoSelectView *pView);
public:
	TreeViewFileItem();

	HTREEITEM GetViewItem();
	void SetViewItem(HTREEITEM h);

	BOOL Move(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Copy(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Delete(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Encrypt(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Decrypt(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Rename(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR pNewName);

	DWORD GetIcon(MemoSelectView *pView, DWORD nStatus);

	DWORD ItemOrder();

};

/////////////////////////////////////////////
//  フォルダ
/////////////////////////////////////////////

class TreeViewFolderItem : public TreeViewItem {
	HTREEITEM hItem;
public:
	TreeViewFolderItem();

	HTREEITEM GetViewItem();
	void SetViewItem(HTREEITEM h);

	BOOL Move(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Copy(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Delete(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Encrypt(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Decrypt(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Rename(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR pNewName);

	DWORD GetIcon(MemoSelectView *pView, DWORD nStatus);

	DWORD ItemOrder();
};

#endif
