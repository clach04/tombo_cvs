#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "MemoSelectView.h"
#include "MemoDetailsView.h"
#include "PasswordManager.h"
#include "MemoManager.h"

class MemoNote;

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

	HMENU hMenuEncrypt;		//「暗号化」メニュー
	HMENU hFoldMenu;

	MemoSelectView msView;
	MemoDetailsView mdView;

	MemoManager mmMemoManager;

	PasswordManager pmPasswordMgr;

	BOOL bSelectViewActive;

	RECT rWindowRect;	// 画面サイズ

	// ペインサイズ変更中フラグ
	BOOL bResizePane;

protected:
	// アプリケーションボタンをアプリからハンドリングできるようにする
	BOOL EnableApplicationButton(HWND hWnd);

	// ペイン配分の変更
	void MovePane(WORD width);

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
	// 検索
	void OnSearch();
	void OnSearchNext(BOOL bForward);
	void DoSearchTree(BOOL bFirst, BOOL bForward);

	///////////////////
	// ペイン配分変更
	void OnLButtonDown(WPARAM wParam, LPARAM lParam);
	void OnMouseMove(WPARAM wParam, LPARAM lParam);
	void OnLButtonUp(WPARAM wParam, LPARAM lParam);


	////////////////////
	// メニュー

	void About();	// Aboutダイアログ
	void NewMemo();	// 新規メモの作成
	void NewFolder(); // 新規フォルダの作成
	void ToggleFolding(); // 折り返し表示の切り替え
	void TogglePane(); // ペインの切り替え

	// メモオープンの要求
	void RequestOpenMemo(MemoLocator *pLoc, DWORD nSwitchView);

	void OnList(BOOL bAskSave);

	// ビューのアクティブ化
	// TRUEの場合一覧ビューが、FALSEの場合詳細ビューがアクティブ化される。
	void ActivateView(BOOL bList);
	BOOL SelectViewActive() { return bSelectViewActive; }

	void SetFocus();

	void EnableEncrypt(BOOL bEnable);
	void EnableDecrypt(BOOL bEnable);

	// ウィンドウサイズの保存・復元
	void LoadWinSize(HWND hWnd);
	void SaveWinSize();

	// ステータス表示制御
	void SetNewMemoStatus(BOOL bNew);
	void SetModifyStatus(BOOL bModify);

	// タイトルの変更
	void SetTitle(LPCTSTR pTitle);

	void SendRequestOpen(MemoLocator *pLoc, DWORD nSwitchFlg) { SendMessage(hMainWnd, MWM_OPEN_REQUEST, (WPARAM)nSwitchFlg, (LPARAM)pLoc); }
	void PostRequestOpen(MemoLocator *pLoc, DWORD nSwitchFlg) { PostMessage(hMainWnd, MWM_OPEN_REQUEST, (WPARAM)nSwitchFlg, (LPARAM)pLoc); }


	int MessageBox(LPCTSTR pText, LPCTSTR pCaption, UINT uType); 
};


#endif