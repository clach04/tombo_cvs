#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "Message.h"
#include "MemoSelectView.h"
#include "MemoDetailsView.h"
#include "PasswordManager.h"
#include "MemoManager.h"

class MemoNote;
class TreeViewItem;
class VFManager;
class BookMark;
struct BookMarkItem;

///////////////////////////////////////
// Main frame window
///////////////////////////////////////

class MainFrame {
	static LPCTSTR pClassName;

	HWND hMainWnd;
	HINSTANCE hInstance;

	HWND hMSCmdBar;			// command bar handle
	HWND hMDCmdBar;

	HIMAGELIST hSelectViewImgList;

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
	HMENU hMSToolMenu;
	HMENU hMDEditMenu;
#endif

	MemoSelectView msView;
	MemoDetailsView mdView;

	MemoManager mmMemoManager;

	VFManager *pVFManager;

	PasswordManager pmPasswordMgr;

	BOOL bSelectViewActive;

	RECT rWindowRect;	// window size with menu/title

	// pane size is changing
	BOOL bResizePane;

	BOOL bSearchStartFromTreeView;

	// bookmarks
	BookMark *pBookMark;

protected:
	// hook application button for handling from TOMBO
	BOOL EnableApplicationButton(HWND hWnd);

	// move pane splitter
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

	HMENU GetMSBookMarkMenu();

#if defined(PLATFORM_BE500)
	HMENU GetMSToolMenu();
#endif

	void EnableMenu(UINT uId, BOOL bEnable);

public:
	MainFrame(); // ctor
	~MainFrame(); // dtor

	// register window class and create functions
	static BOOL RegisterClass(HINSTANCE hInst);

	BOOL Create(LPCTSTR pWndName, HINSTANCE hInst, int nCmdShow);

	int MainLoop();

	// Event handler
	void OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL OnExit();
	BOOL OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void OnSettingChange(WPARAM wParam);
	void OnSIPResize(BOOL bImeOn, RECT *pSipRect);
	void OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void OnProperty();
	void OnForgetPass();
	void OnTimer(WPARAM nTimerID);
	void OnMutualExecute();
	BOOL OnHotKey(WPARAM wParam, LPARAM lParam);
	void OnResize(WPARAM wParam, LPARAM lParam);
	void OnTooltip(WPARAM wParam, LPARAM lParam);
	void OnVFolderDef();

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

	void About();	// About dialog
	void NewMemo();	// create new notes
	void NewFolder(TreeViewItem *pItem); // create new folder
	void SetWrapText(BOOL bWrap); // Toggle wrapping on/off
	void TogglePane(); // switch 1pange/2panes

	void SetTopMost(); // keep top of the window

	//////////////////////////
	// open notes

	void RequestOpenMemo(MemoLocator *pLoc, DWORD nSwitchView);
	void SendRequestOpen(MemoLocator *pLoc, DWORD nSwitchFlg) { SendMessage(hMainWnd, MWM_OPEN_REQUEST, (WPARAM)nSwitchFlg, (LPARAM)pLoc); }
	void PostRequestOpen(MemoLocator *pLoc, DWORD nSwitchFlg) { PostMessage(hMainWnd, MWM_OPEN_REQUEST, (WPARAM)nSwitchFlg, (LPARAM)pLoc); }

	void OnList(BOOL bAskSave);

	// Activate view
	// tree view is activated when bList is TRUE, otherwise edit view is activated.
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

	// Save/restore window size
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

	// change window title
	void SetTitle(LPCTSTR pTitle);

	////////////////////////////////
	// bookmark related members
	void OnBookMarkAdd(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void OnBookMarkConfig(HWND hWnd, WPARAM wParam, LPARAM lParam);

	void OnBookMark(HWND hWnd, WPARAM wParam, LPARAM lParam);

	void AppendBookMark(HMENU hMenu, const BookMarkItem *pItem);
	void LoadBookMark(LPCTSTR pBookMarks);

	////////////////////////////////
	// misc funcs

	int MessageBox(LPCTSTR pText, LPCTSTR pCaption, UINT uType); 
};


#endif