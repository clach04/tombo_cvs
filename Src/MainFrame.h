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
// メインフレームウィンドウ
///////////////////////////////////////

class MainFrame {
	static LPCTSTR pClassName;

	HWND hMainWnd;
	HINSTANCE hInstance;

	HWND hMSCmdBar;			// コマンドバー
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

	RECT rWindowRect;	// 画面サイズ

	// ペインサイズ変更中フラグ
	BOOL bResizePane;

	BOOL bSearchStartFromTreeView;

protected:
	// アプリケーションボタンをアプリからハンドリングできるようにする
	BOOL EnableApplicationButton(HWND hWnd);

	// ペイン配分の変更
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

	// ウィンドウクラスの登録
	static BOOL RegisterClass(HINSTANCE hInst);

	BOOL Create(LPCTSTR pWndName, HINSTANCE hInst, int nCmdShow);

	int MainLoop();

	// イベントハンドラ
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

	void About();	// Aboutダイアログ
	void NewMemo();	// 新規メモの作成
	void NewFolder(TreeViewItem *pItem); // 新規フォルダの作成
	void SetWrapText(BOOL bWrap); // 折り返し表示の切り替え
	void TogglePane(); // ペインの切り替え

	void SetTopMost(); // keep top of the window

	//////////////////////////
	// open notes

	void RequestOpenMemo(MemoLocator *pLoc, DWORD nSwitchView);
	void SendRequestOpen(MemoLocator *pLoc, DWORD nSwitchFlg) { SendMessage(hMainWnd, MWM_OPEN_REQUEST, (WPARAM)nSwitchFlg, (LPARAM)pLoc); }
	void PostRequestOpen(MemoLocator *pLoc, DWORD nSwitchFlg) { PostMessage(hMainWnd, MWM_OPEN_REQUEST, (WPARAM)nSwitchFlg, (LPARAM)pLoc); }


	void OnList(BOOL bAskSave);

	// ビューのアクティブ化
	// TRUEの場合一覧ビューが、FALSEの場合詳細ビューがアクティブ化される。
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

	// ウィンドウサイズの保存・復元
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

	// タイトルの変更
	void SetTitle(LPCTSTR pTitle);

	////////////////////////////////
	// misc funcs

	int MessageBox(LPCTSTR pText, LPCTSTR pCaption, UINT uType); 
};


#endif