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

	// Cut/Copy/Paste�p
	TreeViewItem *pClipItem; // Cut/Copy��Dispatcher
	BOOL bCut;

	// �����؊������[�h�t���O
	BOOL bAutoLoadMode;
	BOOL bSingleClickMode;

	void SetShareArea(TreeViewItem *pItem);

	void DeleteOneItem(HTREEITEM hItem);
	void DeleteItemsRec(HTREEITEM hFirst);

	// �c���[�̊J��
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
	// �\���E��ʊ֘A

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

	// �N���b�v�{�[�h(���ǂ�)����p
	void OnCut(TreeViewItem *pItem);
	void OnCopy(TreeViewItem *pItem);
	void OnPaste();

	// �E�B���h�E�T�C�Y�擾
	void GetSize(LPWORD pWidth, LPWORD pHeight);

	void MoveWindow(DWORD x, DWORD y, DWORD nWidth, DWORD nHeight);

	/////////////////////////////
	// �v���p�e�B����֘A

	// �����؊������[�h
	void SetAutoLoadMode(BOOL bMode) { bAutoLoadMode = bMode; }
	BOOL IsAutoLoadMode() { return bAutoLoadMode; }

	void SetSingleClickMode(BOOL bMode) { bSingleClickMode = bMode; }
	BOOL IsSingleClickMode() { return bSingleClickMode; }

	/////////////////////////////
	// ����
//	BOOL Search(BOOL bFirstSearch, BOOL bForward);

	/////////////////////////////
	// �r���[�A�C�e������֘A

	BOOL InitTree();
	BOOL DeleteAllItem();

	void TreeExpand(HTREEITEM hItem);
	void TreeCollapse(HTREEITEM hItem);

	// �V�K��������������
	HTREEITEM NewMemoCreated(MemoNote *pNote, LPCTSTR pHeadLine, HTREEITEM hItem);

	BOOL InsertFile(HTREEITEM hParent, LPCTSTR pPrefix, LPCTSTR pFile);
	BOOL InsertFile(HTREEITEM hParent, MemoNote *pNote, LPCTSTR pTitle, BOOL bInsertLast);

	// if bInsertLast is TRUE, Insert folder without sorting.
	HTREEITEM InsertFolder(HTREEITEM hParent, LPCTSTR pName, TreeViewItem *tvi, BOOL bInsertLast);

	// TreeViewItem�̏�Ԃ��ς�������Ƃɂ��r���[�ւ̕ύX�˗�
	BOOL UpdateItemStatusNotify(TreeViewItem *pItem, LPCTSTR pNewHeadLine);

	// ���ݑI�����Ă���A�C�e���̃p�X���擾����B
	// �I�����Ă���A�C�e�����t�H���_�̏ꍇ�A���g�܂ł̃p�X���܂�
	HTREEITEM GetPathForNewItem(TString *pPath, TreeViewItem *pItem = NULL);

	HTREEITEM GetTreeItemFromPath(LPCTSTR pPath);

	// �m�[�h�̑I��
	void SelectNote(MemoNote *pNote);

	BOOL CreateNewFolder(HTREEITEM hItem, LPCTSTR pFolder);
	BOOL GetHeadLine(MemoNote *pNote, LPTSTR pHeadLine, DWORD nLen);

	// �w�b�h���C��������̏�������
	BOOL UpdateHeadLine(MemoLocator *pLoc, LPCTSTR pHeadLine);

	// ���ݑI������Ă���A�C�e���Ɗ֘A�t�����Ă���MemoNote��Ԃ��B
	// pItem���w�肳��Ă���ꍇ�ɂ�HTREEITEM���Ԃ��B
	// �I������Ă��Ȃ��ꍇ�ɂ͖߂�l�Ƃ���NULL��Ԃ��B
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