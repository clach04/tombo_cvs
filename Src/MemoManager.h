#ifndef MEMOMANAGER_H
#define MEMOMANAGER_H

class MemoDetailsView;
class MemoSelectView;
class MainFrame;
class MemoNote;
class PasswordManager;
class TreeViewItem;
class TreeViewFileItem;

class SearchEngineA;

class MemoManager {
protected:
	MemoDetailsView *pMemoDetailsView;
	MemoSelectView *pMemoSelectView;
	MainFrame *pMainFrame;
	PasswordManager *pPassMgr;

	TreeViewFileItem *pCurrentItem;

	TreeViewFileItem *AllocNewMemo(LPCTSTR pText, MemoNote *pTemplate = NULL);

	SearchEngineA *pSearchEngineA;

	BOOL bMSSearchFlg;
	BOOL bMDSearchFlg;
public:
	/////////////////////////////////////
	// ctor & dtor

	MemoManager();
	~MemoManager();

	/////////////////////////////////////
	// �������֘A

	BOOL Init(MainFrame *p, MemoDetailsView *md, MemoSelectView *ms);
	void SetPasswordManager(PasswordManager *p) { pPassMgr = p; }

	/////////////////////////////////////

	// �t�@�C�����폜����B�폜���Ɍ��̓��e��0�N���A����
	static BOOL WipeOutAndDeleteFile(LPCTSTR pFile);

	// �V�K�����̍쐬
	BOOL NewMemo();

	// �������X�V����Ă�����Z�[�u����
	//	pYNC�ɂ�IDOK,IDYES,IDNO,IDCANCEL�̂����ꂩ������B
	//		�Z�[�u�̕K�v���Ȃ������ꍇ�ɂ�IDOK���ݒ肳���B
	//		IDYES���ݒ肳��Ă���ꍇ�ɂ͕ۑ������s���ꂽ�B
	//  pYNC��NULL��ݒ肵���ꍇ�A�₢���킹�̃��b�Z�[�W�{�b�N�X�͕\������Ȃ�
	//		���̏ꍇ�AbDupMode��TRUE�̏ꍇ�ɂ̓����͏�ɕʃt�@�C���Ƃ��ĕۑ������B
	//				  bDupMode��FALSE�̏ꍇ�ɂ͒ʏ퓮��
	BOOL SaveIfModify(LPDWORD pYNC, BOOL bDupMode);

	// �����̃��[�h
	BOOL SetMemo(TreeViewFileItem *pItem);

	// �����̃N���A
	BOOL ClearMemo();

	void SelectNote(MemoNote *pNote);

	BOOL MakeNewFolder(HWND hWnd);	// �t�H���_�̐V�K�쐬

	BOOL StoreCursorPos();

	void ActivateView(BOOL bSelectViewActive);

	void SelectAll();	// �S�I��(�ڍ׃r���[)

	// �w�肵�����������ݏڍ׃r���[�ŕ\������Ă��邩
	BOOL IsNoteDisplayed(MemoNote *pNote);

	// �ڍ׃r���[�ɕ\������Ă���ꍇ�ɕK�v�Ȃ�ۑ����A�ꗗ�r���[�Ƀt�H�[�J�X���ڂ�
	// TOOD: �ۑ��˗��ɒu����������C������
	void InactiveDetailsView();

	/////////////////////////////////////
	// �����֘A

	void SetSearchEngine(SearchEngineA *p);
	SearchEngineA *GetSearchEngine() { return pSearchEngineA; }

	// �ڍ׃r���[�ɑ΂��錟��
	BOOL SearchDetailsView(BOOL bFirstSearch, BOOL bForward, BOOL bNFMsg, BOOL bSearchFromTop);

	/////////////////////////////////////
	// �f�[�^�A�N�Z�T

	PasswordManager *GetPasswordManager() { return pPassMgr; }
	MemoNote *CurrentNote();
	TreeViewFileItem *CurrentItem();
	MainFrame *GetMainFrame() { return pMainFrame; }

	/////////////////////////////////////
	// �����t���O

	// �u���������v�Ō��ݑI������Ă��鍀�ڂ������ΏۂɊ܂߂邩?

	// �ꗗ�r���[�̏ꍇ�A���[�U���I����ύX������܂߂�
	BOOL MSSearchFlg() { return bMSSearchFlg; }
	void SetMSSearchFlg(BOOL b) { bMSSearchFlg = b; }


	BOOL MDSearchFlg() { return bMDSearchFlg; }
	void SetMDSearchFlg(BOOL b) { bMDSearchFlg = b; }

};

#endif