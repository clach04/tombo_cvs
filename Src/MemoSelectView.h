#ifndef MEMOSELECTVIEW_H
#define MEMOSELECTVIEW_H

class MemoManager;
class MemoNote;
class TString;
class TreeViewItem;
class TreeViewFileItem;

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

	void SetShareArea();
	void DeleteItemsRec(HTREEITEM hFirst);

	// �c���[�̊J��
	void ToggleExpandFolder(HTREEITEM hItem, UINT stat);

	///////////////////////////////////

	void SetIcon(TreeViewItem* p, DWORD nStatus);

	///////////////////////////////////
	LRESULT EditLabel(TVITEM *pItem);

	/////////////////////////////
	// �����p�����֐�

	BOOL SearchItems(HTREEITEM hItem, BOOL bSearchEncryptedMemo, BOOL bFileNameOnly, BOOL bForward);
	BOOL SearchOneItem(HTREEITEM hItem, BOOL bSearchEncryptedMemo, BOOL bFileNameOnly);

public:
	/////////////////////////////
	// �������֘A�֐�

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

	// �N���b�v�{�[�h(���ǂ�)����p
	void OnCut(HWND hWnd);
	void OnCopy(HWND hWnd);
	void OnPaste(HWND hWnd);
	void CheckResetClipboard(TreeViewItem *pItem) { if (pItem == pClipItem) pClipItem = NULL; }

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
	BOOL Search(BOOL bFirstSearch, BOOL bForward);

	/////////////////////////////
	// �r���[�A�C�e������֘A

	BOOL LoadItems();
	BOOL DeleteAllItem();

	void TreeExpand(HTREEITEM hItem);
	void TreeCollapse(HTREEITEM hItem);

	// �V�K��������������
	TreeViewFileItem *NewMemoCreated(MemoNote *pNote, LPCTSTR pHeadLine, HTREEITEM hItem);

	// TreeViewItem�̏�Ԃ��ς�������Ƃɂ��r���[�ւ̕ύX�˗�
	BOOL UpdateItemStatus(TreeViewItem *pItem, LPCTSTR pNewHeadLine);

	// pNote����ʂ���폜
	BOOL DeleteItem(TreeViewItem *pItem);

	// ���ݑI�����Ă���A�C�e���̃p�X���擾����B
	// �I�����Ă���A�C�e�����t�H���_�̏ꍇ�A���g�܂ł̃p�X���܂�
	HTREEITEM GetPathForNewItem(TString *pPath);

	// ����/�t�H���_�̍폜
	void DeleteNode(HWND hWnd, HTREEITEM hItem, TreeViewItem *pItem);

	BOOL CreateNewFolder(HTREEITEM hItem, LPCTSTR pFolder);

	// �w�b�h���C��������̏�������
	BOOL UpdateHeadLine(TreeViewFileItem *pItem, LPCTSTR pHeadLine);

	// ���ݑI������Ă���A�C�e���Ɗ֘A�t�����Ă���MemoNote��Ԃ��B
	// pItem���w�肳��Ă���ꍇ�ɂ�HTREEITEM���Ԃ��B
	// �I������Ă��Ȃ��ꍇ�ɂ͖߂�l�Ƃ���NULL��Ԃ��B
	//MemoNote *GetCurrentItem(HTREEITEM *pItem = NULL);
	TreeViewItem *GetCurrentItem(HTREEITEM *pItem = NULL);

	TreeViewItem *GetTVItem(HTREEITEM h);

	BOOL IsCliped(TreeViewItem* p) { return p == pClipItem; }
	LONG GetItem(TV_ITEM *p) { return TreeView_GetItem(hViewWnd, p); }
	BOOL SetTVItem(HTREEITEM h, TreeViewItem *p);

	LPTSTR GeneratePath(TreeViewItem *p, LPTSTR pBuf, DWORD nSiz);

	BOOL OnEditLabel();
};

/////////////////////////////////////////
// ������(�A�C�R��)��Ԓ�`
/////////////////////////////////////////

// �N���b�v�{�[�h�ɓ����Ă���
#define MEMO_VIEW_STATE_CLIPED_SET		(1)
#define MEMO_VIEW_STATE_CLIPED_CLEAR	(1 << 1)
// �J����Ă���
#define MEMO_VIEW_STATE_OPEN_SET		(1 << 2)
#define MEMO_VIEW_STATE_OPEN_CLEAR		(1 << 3)


#endif