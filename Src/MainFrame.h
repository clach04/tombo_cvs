#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "MemoSelectView.h"
#include "MemoDetailsView.h"
#include "PasswordManager.h"
#include "MemoManager.h"

class MemoNote;

///////////////////////////////////////
// ���C���t���[���E�B���h�E
///////////////////////////////////////

class MainFrame {
	static LPCTSTR pClassName;

	HWND hMainWnd;
	HINSTANCE hInstance;

	HWND hMSCmdBar;			// �R�}���h�o�[
	HWND hMDCmdBar;

#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	HWND hRebar;
	HWND hToolBar;
#endif
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	HWND hStatusBar;
	void ResizeStatusBar();
#endif

	HMENU hMenuEncrypt;		//�u�Í����v���j���[
	HMENU hFoldMenu;

	MemoSelectView msView;
	MemoDetailsView mdView;

	MemoManager mmMemoManager;

	PasswordManager pmPasswordMgr;

	BOOL bSelectViewActive;

	RECT rWindowRect;	// ��ʃT�C�Y

	// �y�C���T�C�Y�ύX���t���O
	BOOL bResizePane;

protected:
	// �A�v���P�[�V�����{�^�����A�v������n���h�����O�ł���悤�ɂ���
	BOOL EnableApplicationButton(HWND hWnd);

	// �y�C���z���̕ύX
	void MovePane(WORD width);

public:
	MainFrame(); // ctor

	// �E�B���h�E�N���X�̓o�^
	static BOOL RegisterClass(HINSTANCE hInst);

	BOOL Create(LPCTSTR pWndName, HINSTANCE hInst, int nCmdShow);

	int MainLoop();

	// �C�x���g�n���h��
	void OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL OnExit();
	BOOL OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void OnSettingChange();
	void OnSIPResize(BOOL bImeOn, DWORD nSipHeight);
	void OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void OnProperty();
	void OnForgetPass();
	void OnTimer(WPARAM nTimerID);
	void OnMutualExecute();
	BOOL OnHotKey(WPARAM wParam, LPARAM lParam);
	void OnResize(WPARAM wParam, LPARAM lParam);
	void OnTooltip(WPARAM wParam, LPARAM lParam);

	///////////////////
	// ����
	void OnSearch();
	void OnSearchNext(BOOL bForward);
	void DoSearchTree(BOOL bFirst, BOOL bForward);

	///////////////////
	// �y�C���z���ύX
	void OnLButtonDown(WPARAM wParam, LPARAM lParam);
	void OnMouseMove(WPARAM wParam, LPARAM lParam);
	void OnLButtonUp(WPARAM wParam, LPARAM lParam);


	////////////////////
	// ���j���[

	void About();	// About�_�C�A���O
	void NewMemo();	// �V�K�����̍쐬
	void NewFolder(); // �V�K�t�H���_�̍쐬
	void ToggleFolding(); // �܂�Ԃ��\���̐؂�ւ�
	void TogglePane(); // �y�C���̐؂�ւ�

	// �����I�[�v���̗v��
	void RequestOpenMemo(MemoLocator *pLoc, DWORD nSwitchView);

	void OnList(BOOL bAskSave);

	// �r���[�̃A�N�e�B�u��
	// TRUE�̏ꍇ�ꗗ�r���[���AFALSE�̏ꍇ�ڍ׃r���[���A�N�e�B�u�������B
	void ActivateView(BOOL bList);
	BOOL SelectViewActive() { return bSelectViewActive; }

	void SetFocus();

	void EnableEncrypt(BOOL bEnable);
	void EnableDecrypt(BOOL bEnable);

	// �E�B���h�E�T�C�Y�̕ۑ��E����
	void LoadWinSize(HWND hWnd);
	void SaveWinSize();

	// �X�e�[�^�X�\������
	void SetNewMemoStatus(BOOL bNew);
	void SetModifyStatus(BOOL bModify);

	// �^�C�g���̕ύX
	void SetTitle(LPCTSTR pTitle);

	void SendRequestOpen(MemoLocator *pLoc, DWORD nSwitchFlg) { SendMessage(hMainWnd, MWM_OPEN_REQUEST, (WPARAM)nSwitchFlg, (LPARAM)pLoc); }
	void PostRequestOpen(MemoLocator *pLoc, DWORD nSwitchFlg) { PostMessage(hMainWnd, MWM_OPEN_REQUEST, (WPARAM)nSwitchFlg, (LPARAM)pLoc); }


	int MessageBox(LPCTSTR pText, LPCTSTR pCaption, UINT uType); 
};


#endif