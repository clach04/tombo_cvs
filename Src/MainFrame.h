#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "Message.h"
#include "MemoSelectView.h"
#include "MemoDetailsView.h"
#include "PasswordManager.h"
#include "MemoManager.h"

class MemoNote;
class TreeViewItem;

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

#if defined(PLATFORM_BE500)
	HMENU hMSMemoMenu;
	HMENU hMDEditMenu;
#endif

	MemoSelectView msView;
	MemoDetailsView mdView;

	MemoManager mmMemoManager;

	PasswordManager pmPasswordMgr;

	BOOL bSelectViewActive;

	RECT rWindowRect;	// ��ʃT�C�Y

	// �y�C���T�C�Y�ύX���t���O
	BOOL bResizePane;

	BOOL bSearchStartFromTreeView;

protected:
	// �A�v���P�[�V�����{�^�����A�v������n���h�����O�ł���悤�ɂ���
	BOOL EnableApplicationButton(HWND hWnd);

	// �y�C���z���̕ύX
	void MovePane(WORD width);

	void SetStatusIndicator(DWORD nPos, LPCTSTR pText, BOOL bDisp);

#if defined(PLATFORM_WIN32)
	HMENU GetMainMenu() { return GetMenu(hMainWnd); }
	HWND GetMainToolBar() { return hToolBar; }
#endif
#if defined(PLATFORM_HPC)
	HMENU GetMainMenu();
	HWND GetMainToolBar();
#endif
	HMENU GetMDToolMenu();
	HMENU GetMSEditMenu();

	void EnableMenu(UINT uId, BOOL bEnable);

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
	void OnSettingChange(WPARAM wParam);
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
	// Search/grep

	void OnSearch();
	void OnSearchNext(BOOL bForward);
	void DoSearchTree(BOOL bFirst, BOOL bForward);

	void OnGrep();

	///////////////////
	// for move panes

	void OnLButtonDown(WPARAM wParam, LPARAM lParam);
	void OnMouseMove(WPARAM wParam, LPARAM lParam);
	void OnLButtonUp(WPARAM wParam, LPARAM lParam);


	////////////////////
	// menu handler

	void About();	// About�_�C�A���O
	void NewMemo();	// �V�K�����̍쐬
	void NewFolder(TreeViewItem *pItem); // �V�K�t�H���_�̍쐬
	void SetWrapText(BOOL bWrap); // �܂�Ԃ��\���̐؂�ւ�
	void TogglePane(); // �y�C���̐؂�ւ�

	void SetTopMost(); // keep top of the window

	//////////////////////////
	// open notes

	void RequestOpenMemo(MemoLocator *pLoc, DWORD nSwitchView);
	void SendRequestOpen(MemoLocator *pLoc, DWORD nSwitchFlg) { SendMessage(hMainWnd, MWM_OPEN_REQUEST, (WPARAM)nSwitchFlg, (LPARAM)pLoc); }
	void PostRequestOpen(MemoLocator *pLoc, DWORD nSwitchFlg) { PostMessage(hMainWnd, MWM_OPEN_REQUEST, (WPARAM)nSwitchFlg, (LPARAM)pLoc); }


	void OnList(BOOL bAskSave);

	// �r���[�̃A�N�e�B�u��
	// TRUE�̏ꍇ�ꗗ�r���[���AFALSE�̏ꍇ�ڍ׃r���[���A�N�e�B�u�������B
	void ActivateView(BOOL bList);
	BOOL SelectViewActive() { return bSelectViewActive; }

	void SetFocus();

	////////////////////////////////
	// Control menus & toolbars

	void EnableEncrypt(BOOL bEnable);
	void EnableDecrypt(BOOL bEnable);
	void EnableDelete(BOOL bEnable);
	void EnableSaveButton(BOOL bEnable);
	void EnableRename(BOOL bEnable);
	void EnableNew(BOOL bEnable);
	void EnableCut(BOOL bEnable);
	void EnableCopy(BOOL bEnable);
	void EnablePaste(BOOL bEnable);
	void EnableNewFolder(BOOL bEnable);
	void EnableGrep(BOOL bEnable);

	// �E�B���h�E�T�C�Y�̕ۑ��E����
	void LoadWinSize(HWND hWnd);
	void SaveWinSize();

	
	////////////////////////////////
	// Control status indicator

	void SetReadOnlyStatus(BOOL bReadOnly) { SetStatusIndicator(1, MSG_RONLY, bReadOnly); }
	void SetNewMemoStatus(BOOL bNew) { SetStatusIndicator(2, MSG_NEW, bNew); }
	void SetModifyStatus(BOOL bModify);

#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	void ToggleShowStatusBar();
#endif

	// �^�C�g���̕ύX
	void SetTitle(LPCTSTR pTitle);

	////////////////////////////////
	// misc funcs

	int MessageBox(LPCTSTR pText, LPCTSTR pCaption, UINT uType); 
};


#endif