#ifndef TREEVIEWITEM_H
#define TREEVIEWITEM_H

class MemoNote;
class MemoSelectView;
class MemoManager;

/////////////////////////////////////////////
//  TreeView�̃A�C�e���̒��ۉ�
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
};

/////////////////////////////////////////////
//  �t�@�C��
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
//  �t�H���_
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
