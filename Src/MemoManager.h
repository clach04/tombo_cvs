#ifndef MEMOMANAGER_H
#define MEMOMANAGER_H

class MemoDetailsView;
class MemoSelectView;
class MainFrame;
class PasswordManager;
class SearchEngineA;
class TreeViewItem;
class TString;
class TomboURI;

/////////////////////////////////////
// Control other view
/////////////////////////////////////
// control select - detail view relation and mainframe items(menu, toolbar, etc.)

class MemoManager {
protected:
	MemoDetailsView *pMemoDetailsView;
	MemoSelectView *pMemoSelectView;
	MainFrame *pMainFrame;

	BOOL AllocNewMemo(LPCTSTR pText, BOOL bCopy);

	SearchEngineA *pSearchEngineA;

	BOOL bMSSearchFlg;
	BOOL bMDSearchFlg;

public:
	/////////////////////////////////////
	// ctor & dtor

	MemoManager();
	~MemoManager();

	BOOL Init(MainFrame *p, MemoDetailsView *md, MemoSelectView *ms);

	/////////////////////////////////////

	// �������X�V����Ă�����Z�[�u����
	//	pYNC�ɂ�IDOK,IDYES,IDNO,IDCANCEL�̂����ꂩ������B
	//		�Z�[�u�̕K�v���Ȃ������ꍇ�ɂ�IDOK���ݒ肳���B
	//		IDYES���ݒ肳��Ă���ꍇ�ɂ͕ۑ������s���ꂽ�B
	//  pYNC��NULL��ݒ肵���ꍇ�A�₢���킹�̃��b�Z�[�W�{�b�N�X�͕\������Ȃ�
	//		���̏ꍇ�AbDupMode��TRUE�̏ꍇ�ɂ̓����͏�ɕʃt�@�C���Ƃ��ĕۑ������B
	//				  bDupMode��FALSE�̏ꍇ�ɂ͒ʏ퓮��
	BOOL SaveIfModify(LPDWORD pYNC, BOOL bDupMode);

	/////////////////////////////////////
	// �����֘A

	void SetSearchEngine(SearchEngineA *p);
	SearchEngineA *GetSearchEngine() { return pSearchEngineA; }

	/////////////////////////////////////
	// data accessor

	MainFrame *GetMainFrame() { return pMainFrame; }
	MemoSelectView *GetSelectView() { return pMemoSelectView; }
	MemoDetailsView *GetDetailsView() { return pMemoDetailsView; }

	/////////////////////////////////////
	// searching

	// �u���������v�Ō��ݑI������Ă��鍀�ڂ������ΏۂɊ܂߂邩?
	// �ꗗ�r���[�̏ꍇ�A���[�U���I����ύX������܂߂�
	BOOL MSSearchFlg() { return bMSSearchFlg; }
	void SetMSSearchFlg(BOOL b) { bMSSearchFlg = b; }

	BOOL MDSearchFlg() { return bMDSearchFlg; }
	void SetMDSearchFlg(BOOL b) { bMDSearchFlg = b; }

	/////////////////////////////////////
	// 
	void ChangeURINotify(const TomboURI *pNewURI);

	// �ڍ׃r���[�ɕ\������Ă���ꍇ�ɕK�v�Ȃ�ۑ����A�ꗗ�r���[�Ƀt�H�[�J�X���ڂ�
	// TODO: �ۑ��˗��ɒu����������C������
	void InactiveDetailsView();

	BOOL GetCurrentSelectedPath(TString *pPath);

};

#endif