#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

#if defined(PLATFORM_BE500)
#include <CSO.h>
#endif

#include "Tombo.h"
#include "MainFrame.h"
#include "resource.h"
#include "Message.h"
#include "Property.h"
#include "TString.h"
#include "SipControl.h"
#include "TreeViewItem.h"
#include "GrepDialog.h"

#ifdef _WIN32_WCE
#if defined(PLATFORM_PKTPC)
#include <Aygshell.h>
#include <Imm.h>
#endif
#if defined(PLATFORM_PSPC)
#include <Aygshell.h>
extern "C" {
	// ?? Imm.h消しちゃったのかなぁ??
UINT WINAPI ImmGetVirtualKey(HWND);
};
#endif
#endif

#include "MemoNote.h"
#include "AboutDialog.h"
#include "SearchDlg.h"
#include "SearchEngine.h"
#include "SearchTree.h"

LPCTSTR MainFrame::pClassName = TOMBO_MAIN_FRAME_WINDOW_CLSS;

static LRESULT CALLBACK MainFrameWndProc(HWND, UINT, WPARAM, LPARAM);

#define SHGetMenu(hWndMB)  (HMENU)SendMessage((hWndMB), SHCMBM_GETMENU, (WPARAM)0, (LPARAM)0);
#define SHGetSubMenu(hWndMB,ID_MENU) (HMENU)SendMessage((hWndMB), SHCMBM_GETSUBMENU, (WPARAM)0, (LPARAM)ID_MENU);
#define SHSetSubMenu(hWndMB,ID_MENU) (HMENU)SendMessage((hWndMB), SHCMBM_SETSUBMENU, (WPARAM)0, (LPARAM)ID_MENU);

// 左右ペイン間の柱の幅
#if defined(PLATFORM_WIN32)
#define BORDER_WIDTH 2
#endif
#if defined(PLATFORM_HPC)
#define BORDER_WIDTH 5
#endif

///////////////////////////////////////
// コマンドバー関連
///////////////////////////////////////
///////////////////////////////////////
// Pocket PC

#if defined(PLATFORM_PKTPC)
#define NUM_TOOLBAR_BMP 10

#define NUM_MS_TOOLTIP 1
LPTSTR pMSToolTip[] = {
	MSG_TOOLTIPS_NEWMEMO,
};

#define NUM_MD_TOOLTIP 6
LPTSTR pMDToolTip[] = {
	MSG_TOOLTIPS_RETURNLIST,
	MSG_TOOLTIPS_SAVE,
	TEXT(""),
	TEXT(""),
	MSG_TOOLTIPS_INSDATE1,
	MSG_TOOLTIPS_INSDATE2,
};
#endif

///////////////////////////////////////
// PSPC 版

#if defined(PLATFORM_PSPC)
#define NUM_CMDBAR_BUTTONS 6
#define NUM_IMG_BUTTONS 11

#define NUM_MD_CMDBAR_BUTTONS 7
#define NUM_MD_IMG_BUTTONS 1

static TBBUTTON aCmdBarButtons[NUM_CMDBAR_BUTTONS] = {
	{0,  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{17, IDM_NEWMEMO   , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
	{0,  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{22, IDM_SEARCH_PREV,              0, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{23, IDM_SEARCH_NEXT,              0, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0,  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
};

static TBBUTTON aMDCmdBarButtons[NUM_MD_CMDBAR_BUTTONS] = {
	{0,            0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{15,           IDM_RETURNLIST, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{STD_FILESAVE, IDM_SAVE,       TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0,            0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{22,           IDM_SEARCH_PREV,              0, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{23,           IDM_SEARCH_NEXT,              0, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0,            0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},

};
#endif

///////////////////////////////////////
// H/PC 2Pane版

#if defined(PLATFORM_HPC)
static void ControlMenu(HMENU hMenu, BOOL bSelectViewActive);
static void ControlToolbar(HWND hToolbar, BOOL bSelectViewActive);
static HWND GetCommandBar(HWND hBand, UINT uBandID);

#define NUM_MY_TOOLBAR_BMPS 0
#define NUM_CMDBAR_BUTTONS 19
#define NUM_IMG_BUTTONS 10

static TBBUTTON aCmdBarButtons[NUM_CMDBAR_BUTTONS] = {
	{0,                                  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{STD_FILENEW + NUM_MY_TOOLBAR_BMPS,  IDM_NEWMEMO,    TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{STD_FILESAVE + NUM_MY_TOOLBAR_BMPS, IDM_SAVE,       TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0,                                  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{STD_CUT + NUM_MY_TOOLBAR_BMPS,      IDM_CUT,        TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{STD_COPY + NUM_MY_TOOLBAR_BMPS,     IDM_COPY,       TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{STD_PASTE + NUM_MY_TOOLBAR_BMPS,    IDM_PASTE,      TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0,                                  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{STD_DELETE + NUM_MY_TOOLBAR_BMPS,   IDM_DELETEITEM, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0,                                  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{18,                                 IDM_INSDATE1,   TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{19,                                 IDM_INSDATE2,   TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0,                                  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{21,                                 IDM_TOGGLEPANE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0,                                  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{24,                                 IDM_SEARCH,     TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{22,                                 IDM_SEARCH_PREV,              0, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{23,                                 IDM_SEARCH_NEXT,              0, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0,                                  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
};

#define NUM_MD_CMDBAR_BUTTONS 2

static TBBUTTON aMDCmdBarButtons[NUM_MD_CMDBAR_BUTTONS] = {
	{0,  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{15, IDM_RETURNLIST, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
};
#endif

///////////////////////////////////////
// Win32版

#if defined(PLATFORM_WIN32)
static void ControlMenu(HMENU hMenu, BOOL bSelectViewActive);
static void ControlToolbar(HWND hToolbar, BOOL bSelectViewActive);

#define NUM_MY_TOOLBAR_BMPS 11
#define NUM_TOOLBAR_BUTTONS 19

static TBBUTTON aToolbarButtons[NUM_TOOLBAR_BUTTONS] = {
	{STD_FILENEW + NUM_MY_TOOLBAR_BMPS,  IDM_NEWMEMO,    TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{STD_FILESAVE + NUM_MY_TOOLBAR_BMPS, IDM_SAVE,       TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0,                                  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{STD_CUT + NUM_MY_TOOLBAR_BMPS,      IDM_CUT,        TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{STD_COPY + NUM_MY_TOOLBAR_BMPS,     IDM_COPY,       TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{STD_PASTE + NUM_MY_TOOLBAR_BMPS,    IDM_PASTE,      TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0,                                  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{STD_DELETE + NUM_MY_TOOLBAR_BMPS,   IDM_DELETEITEM, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0,                                  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{3,                                  IDM_INSDATE1,   TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{4,                                  IDM_INSDATE2,   TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0,                                  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{9,                                  IDM_SEARCH,     TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{7,                                  IDM_SEARCH_PREV,              0, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{8,                                  IDM_SEARCH_NEXT,              0, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0,                                  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{6,                                  IDM_TOGGLEPANE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{10,                                 IDM_TOPMOST,    TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0,                                  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
};
#endif

///////////////////////////////////////
// l'agenda版
#if defined(PLATFORM_BE500)

#define NUM_IMG_BUTTONS 0
#define NUM_MD_IMG_BUTTONS 0

#define NUM_SV_CMDBAR_BUTTONS 8
CSOBAR_BUTTONINFO	aSVCSOBarButtons[NUM_SV_CMDBAR_BUTTONS] = 
{
	IDM_SV_MENU_1,  CSOBAR_BUTTON_SUBMENU_DOWN,  CSO_BUTTON_DISP, (-1),        NULL, MSG_MEMO, NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	IDM_SV_MENU_2,  CSOBAR_BUTTON_SUBMENU_DOWN,  CSO_BUTTON_DISP, (-1),        NULL, MSG_TOOL, NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	0,              CSOBAR_SEP,                  CSO_BUTTON_DISP, (-1),        NULL, NULL,     NULL,   0,                     1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	IDM_NEWMEMO,    CSOBAR_BUTTON_NORM,          CSO_BUTTON_DISP, IDB_NEWMEMO, NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	0,              CSOBAR_SEP,                  CSO_BUTTON_DISP, (-1),        NULL, NULL,     NULL,   0,                     1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	IDM_SEARCH,     CSOBAR_BUTTON_NORM,         CSO_BUTTON_DISP, IDB_FIND,     NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	IDM_SEARCH_PREV,CSOBAR_BUTTON_NORM,         CSO_BUTTON_DISP, IDB_FINDPREV, NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	IDM_SEARCH_NEXT,CSOBAR_BUTTON_NORM,         CSO_BUTTON_DISP, IDB_FINDNEXT, NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
};

#define NUM_DV_CMDBAR_BUTTONS 12

// ビットマップを張る場合にはOnCreateでInstanceを設定すること
CSOBAR_BUTTONINFO	aDVCSOBarButtons[NUM_DV_CMDBAR_BUTTONS] = 
{
	IDM_RETURNLIST, CSOBAR_COMMON_BUTTON,       CSO_BUTTON_DISP, (-1),            NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), CSO_ID_BACK, CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	0,              CSOBAR_SEP,                 CSO_BUTTON_DISP, (-1),            NULL, NULL,     NULL,   0,                     1, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	IDM_DV_MENU_1,  CSOBAR_BUTTON_SUBMENU_DOWN, CSO_BUTTON_DISP, (-1),            NULL, MSG_EDIT, NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	0,              CSOBAR_SEP,                 CSO_BUTTON_DISP, (-1),            NULL, NULL,     NULL,   0,                     1, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	IDM_SAVE,       CSOBAR_BUTTON_NORM,         CSO_BUTTON_DISP, IDB_SAVE,        NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	0,              CSOBAR_SEP,                 CSO_BUTTON_DISP, (-1),            NULL, NULL,     NULL,   0,                     1, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	IDM_CUT,        CSOBAR_BUTTON_NORM,         CSO_BUTTON_DISP, IDB_CUT,         NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	IDM_COPY,       CSOBAR_BUTTON_NORM,         CSO_BUTTON_DISP, IDB_COPY,        NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	IDM_PASTE,      CSOBAR_BUTTON_NORM,         CSO_BUTTON_DISP, IDB_PASTE,       NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	0,              CSOBAR_SEP,                 CSO_BUTTON_DISP, (-1),            NULL, NULL,     NULL,   0,                     1, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	IDM_INSDATE1,   CSOBAR_BUTTON_NORM,         CSO_BUTTON_DISP, IDB_INSDATE1,    NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	IDM_INSDATE2,   CSOBAR_BUTTON_NORM,         CSO_BUTTON_DISP, IDB_INSDATE2,    NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,

//	IDM_SEARCH_PREV,CSOBAR_BUTTON_NORM,         CSO_BUTTON_DISP, IDB_FINDPREV,    NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 2, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
//	IDM_SEARCH_NEXT,CSOBAR_BUTTON_NORM,         CSO_BUTTON_DISP, IDB_FINDNEXT,    NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 2, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
};

#endif

///////////////////////////////////////
// ctor
///////////////////////////////////////

MainFrame::MainFrame() : bResizePane(FALSE), bSelectViewActive(FALSE)
{
}

///////////////////////////////////////
// ウィンドウクラスの登録
///////////////////////////////////////

BOOL MainFrame::RegisterClass(HINSTANCE hInst)
{
	WNDCLASS wc;

	wc.style = 0;
	wc.lpfnWndProc = (WNDPROC)MainFrameWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(LONG);
	wc.hInstance = hInst;
	wc.hIcon = NULL;
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_PSPC) || defined(PLATFORM_BE500)
	wc.hCursor = NULL;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
#else
	wc.hCursor = NULL;
	wc.hbrBackground = (HBRUSH)COLOR_BTNSHADOW;
#endif
#if defined(PLATFORM_WIN32)
	wc.hCursor = LoadCursor(NULL, IDC_SIZEWE);
	wc.hbrBackground = (HBRUSH)COLOR_BTNSHADOW;
#endif
	wc.lpszMenuName = NULL;
	wc.lpszClassName = pClassName;

	::RegisterClass(&wc);
	return TRUE;
}


///////////////////////////////////////////////////
// Event Handler
///////////////////////////////////////////////////

static LRESULT CALLBACK MainFrameWndProc(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	if (nMessage == WM_CREATE) {
		LPCREATESTRUCT pCS = (LPCREATESTRUCT)lParam;
		MainFrame *frm = (MainFrame*)pCS->lpCreateParams;
		SetWindowLong(hWnd, 0, (LONG)frm);
		frm->OnCreate(hWnd, wParam, lParam);
		return 0;
	}

	MainFrame *frm = (MainFrame*)GetWindowLong(hWnd, 0);
	if (frm == NULL) {
		return DefWindowProc(hWnd, nMessage, wParam, lParam);
	}

	BOOL bRes;
	switch(nMessage) {
	case WM_CLOSE:
		return frm->OnExit();
	case WM_COMMAND:
		frm->OnCommand(hWnd, wParam, lParam);
		return 0;
	case WM_NOTIFY:
		bRes = frm->OnNotify(hWnd, wParam, lParam);
		if (bRes != 0xFFFFFFFF) return bRes;
		break;
	case MWM_OPEN_REQUEST:
		frm->RequestOpenMemo((MemoLocator*)lParam, (BOOL)wParam);
		return 0;
	case WM_SETFOCUS:
		frm->SetFocus();
		return 0;
	case WM_SETTINGCHANGE:
		frm->OnSettingChange(wParam);
		return 0;
	case WM_TIMER:
		frm->OnTimer(wParam);
		return 0;
	case WM_SIZE:
		frm->OnResize(wParam, lParam);
		return 0;
	case WM_HOTKEY:
		// ハンドルできるものについてのみハンドル
		if (frm->OnHotKey(wParam, lParam)) return 0;
		break;
	case WM_LBUTTONDOWN:
		if (g_Property.IsUseTwoPane()) {
			// ペイン配分の変更開始
			frm->OnLButtonDown(wParam, lParam);
			return 0;
		}
		break;
	case WM_MOUSEMOVE:
		if (g_Property.IsUseTwoPane()) {
			// ペイン配分の変更中
			frm->OnMouseMove(wParam, lParam);
			return 0;
		}
		break;
	case WM_LBUTTONUP:
		if (g_Property.IsUseTwoPane()) {
			// ペイン配分の変更終了
			frm->OnLButtonUp(wParam, lParam);
			return 0;
		}
	case MWM_RAISE_MAINFRAME:
		frm->OnMutualExecute();
		return 0;
	}
	return DefWindowProc(hWnd, nMessage, wParam, lParam);
}

//////////////////////////////////////
// message loop
//////////////////////////////////////
// 細工をしてActionボタンを押した際にVK_RETURNが発生しないようにしている

//通常のACTIONシーケンス

//KD	VK_F23 1
//KU	VK_F23 1
//KD	VK_RETURN  1
//KU	VK_RETRN  1

//KD	VK_F23(86)	 1
//KU	VK_F23(86)	 c0000001
//KD	VK_RETURN(d) 1
//KU	VK_RETURN(d) c0000001

//KD	VK_PROCESSKEY(e5)	1
//KU	VK_F23(86)			c0000001
//KD	VK_RETURN(d) 1
//KU	VK_RETURN(d) c0000001

#include "File.h"
#include "Uniconv.h"

int MainFrame::MainLoop() {
	MSG msg;

	HACCEL hAccelSv = LoadAccelerators(g_hInstance, MAKEINTRESOURCE(IDR_ACCEL_SELECT));
	HACCEL hAccelDv = LoadAccelerators(g_hInstance, MAKEINTRESOURCE(IDR_ACCEL_DETAIL));

#if defined(PLATFORM_PKTPC) || defined(PLATFORM_PSPC) || defined(PLATFORM_BE500)
	BOOL bIgnoreReturnKeyDown = FALSE;
	BOOL bIgnoreReturnKeyUp = FALSE;
	BOOL bIgnoreEscKeyDown = FALSE;
	BOOL bIgnoreEscKeyUp = FALSE;
#endif

	while(GetMessage(&msg, NULL, 0, 0)) {
		// パスワードタイムアウト処理
		pmPasswordMgr.ForgetPasswordIfNotAccessed();
		if (msg.message == WM_KEYDOWN || msg.message == WM_LBUTTONDOWN) {
			pmPasswordMgr.UpdateAccess();
		}
	
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_PSPC) || defined(PLATFORM_BE500)
		// アクションキー押下に伴うVK_RETURNの無視

#if defined(PLATFORM_PKTPC)
		// On PocketPC devices, you can select enable/disable about this feature.
		if (!g_Property.DisableExtraActionButton()) {
		//disable logic begin
#endif

		if (msg.message == WM_KEYDOWN) {
			WPARAM w = msg.wParam;
			if (w == VK_PROCESSKEY) {
				w = ImmGetVirtualKey(msg.hwnd);
			}
			if (w == VK_F23) {
				bIgnoreReturnKeyDown = bIgnoreReturnKeyUp = TRUE;
				continue;
			}
			if (w == VK_F24) {
				bIgnoreEscKeyDown = bIgnoreEscKeyUp = TRUE;
				continue;
			}
			if (bIgnoreReturnKeyDown && w == VK_RETURN) {
				bIgnoreReturnKeyDown = FALSE;
				continue;
			}
			if (bIgnoreEscKeyDown && w == VK_ESCAPE) {
				bIgnoreEscKeyDown = FALSE;
				continue;
			}
		}
		if (msg.message == WM_KEYUP) {
			if (msg.wParam == VK_F23) {
				continue;
			}
			if (bIgnoreReturnKeyUp && msg.wParam == VK_RETURN) {
				bIgnoreReturnKeyUp = FALSE;
				PostMessage(hMainWnd, WM_COMMAND, MAKEWPARAM(IDM_ACTIONBUTTON, 0), 0);
				continue;
			}
			if (msg.wParam == VK_F24) {
				continue;
			}
			if (bIgnoreEscKeyUp && msg.wParam == VK_ESCAPE) {
				bIgnoreEscKeyUp = FALSE;
				PostMessage(hMainWnd, WM_COMMAND, MAKEWPARAM(IDM_RETURNLIST, 0), 0);
				continue;
			}
		}
#if defined(PLATFORM_PKTPC)
		} // disable logic end
#endif

#endif
		// 本来の処理
		if (!TranslateAccelerator(hMainWnd, bSelectViewActive ? hAccelSv : hAccelDv, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return msg.wParam;
}

///////////////////////////////////////////////////
// ウィンドウ生成
///////////////////////////////////////////////////

BOOL MainFrame::Create(LPCTSTR pWndName, HINSTANCE hInst, int nCmdShow)
{
	hInstance = hInst;

	mmMemoManager.Init(this, &mdView, &msView);
	msView.Init(&mmMemoManager);
	mdView.Init(&mmMemoManager);

#ifdef _WIN32_WCE
	hMainWnd = CreateWindow(pClassName, pWndName,
						WS_VISIBLE,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						NULL,
						NULL, 
						hInst,
						this);
#else
#if defined(PLATFORM_WIN32)
	hMainWnd = CreateWindow(pClassName, pWndName,
						WS_OVERLAPPEDWINDOW,
						0,
						0,
						640,
						320,
						NULL,
						LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU_MAIN)), 
						hInst,
						this);
#else 
	// CE版デバッグモード
	hMainWnd = CreateWindow(pClassName, pWndName,
						WS_SYSMENU | WS_THICKFRAME,
						0,
						0,
						240,
						320,
						NULL,
						LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU_MAIN)), 
						hInst,
						this);
#endif
#endif

#if defined(PLATFORM_WIN32)
	// アイコンの設定
	HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TOMBO));
	SendMessage(hMainWnd, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hIcon);
#endif
#if defined(PLATFORM_HPC)
	HICON hIcon =  (HICON)LoadImage(hInstance,
                                MAKEINTRESOURCE(IDI_TOMBO),
                                IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	SendMessage(hMainWnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIcon);
#endif

	ShowWindow(hMainWnd, nCmdShow);
	UpdateWindow(hMainWnd);

	// パスワードマネージャ初期化
	pmPasswordMgr.Init(hMainWnd, hInstance);
	mmMemoManager.SetPasswordManager(&pmPasswordMgr);
	g_pPasswordManager = &pmPasswordMgr;

	return TRUE;
}

///////////////////////////////////////////////////
// コマンドバー生成
///////////////////////////////////////////////////

#if defined(PLATFORM_PSPC)
static HWND MakeCommandBar(HINSTANCE hInst, HWND hWnd, 
						   DWORD nCtlID, WORD nMenuID, 
						   TBBUTTON *pButtons,
						   DWORD nButton, DWORD nImages, DWORD nBitmap)
{
	HWND h;
	h = CommandBar_Create(hInst, hWnd, nCtlID);
	int boffset;
	CommandBar_AddBitmap(h, HINST_COMMCTRL,IDB_STD_SMALL_COLOR, 15, 0, 0);
	boffset = CommandBar_AddBitmap(h, hInst, nBitmap, nImages, 0, 0);

	CommandBar_InsertMenubar(h, hInst, nMenuID, 0);
	CommandBar_AddButtons(h, nButton, pButtons);
	CommandBar_AddAdornments(h, 0, 0);
	return h;
}
#endif

///////////////////////////////////////////////////
// CSOBar生成
///////////////////////////////////////////////////
#if defined(PLATFORM_BE500)
static HWND MakeCSOBar(HINSTANCE hInst, HWND hWnd, 
						   DWORD nCtlID)
{
	CSOBAR_BASEINFO cb;
	cb.x = cb.y = cb.width = cb.height = -1;
	cb.line = 1;
	cb.backColor = CSOBAR_DEFAULT_BACKCOLOR;
	cb.titleColor = CSOBAR_DEFAULT_CODECOLOR;
	cb.titleText = NULL;
	cb.titleBmpResId = NULL;
	cb.titleBmpResIns = NULL;

	HWND h;
	h = CSOBar_Create(hInst, hWnd, nCtlID, cb);
	CSOBar_AddAdornments(h, hInst, 1, CSOBAR_ADORNMENT_CLOSE, 0);
	return h;
}
#endif

HWND CreateToolBar(HWND hParent, HINSTANCE hInst);

///////////////////////////////////////////////////
// ウィンドウ初期化
///////////////////////////////////////////////////

void MainFrame::OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	hMainWnd = hWnd;
	LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;

	DWORD nHOffset;

	RECT r;
	GetClientRect(hWnd, &r);

	// プロパティのロード
	BOOL bResult, bStrict;
	bResult = g_Property.Load(&bStrict);
	if (!(bResult && bStrict)) {
		BOOL bPrev = bDisableHotKey;
		bDisableHotKey = TRUE;
		DWORD nResult = g_Property.Popup(pcs->hInstance, hWnd);
		bDisableHotKey = bPrev;
		if (nResult == IDCANCEL) {
			PostQuitMessage(1);
			return;
		}
	}

	nHOffset = 0;
	// コマンドバー
#if defined(PLATFORM_PKTPC)
	SHMENUBARINFO mbi;
	memset(&mbi, 0, sizeof(SHMENUBARINFO));
	mbi.cbSize = sizeof(SHMENUBARINFO);
	mbi.hwndParent = hWnd;
	mbi.nToolBarId = IDM_MAIN_MENU;
	mbi.hInstRes =pcs->hInstance;
	mbi.nBmpId = IDB_TOOLBAR;
	mbi.cBmpImages = NUM_TOOLBAR_BMP;
	if (!SHCreateMenuBar(&mbi)) {
		TomboMessageBox(hWnd, TEXT("SHCreateMenuBar failed."), TEXT("DEBUG"), MB_OK);
	}
	hMSCmdBar = mbi.hwndMB;

	SendMessage(hMSCmdBar, TB_SETTOOLTIPS, (WPARAM)NUM_MS_TOOLTIP, (LPARAM)pMSToolTip);

	memset(&mbi, 0, sizeof(SHMENUBARINFO));
	mbi.cbSize = sizeof(SHMENUBARINFO);
	mbi.hwndParent = hWnd;
	mbi.nToolBarId = IDM_DETAILS_MENU;
	mbi.hInstRes =pcs->hInstance;
	mbi.nBmpId = IDB_TOOLBAR;
	mbi.cBmpImages = NUM_TOOLBAR_BMP;
	if (!SHCreateMenuBar(&mbi)) {
		TomboMessageBox(hWnd, TEXT("SHCreateMenuBar failed."), TEXT("DEBUG"), MB_OK);
	}
	hMDCmdBar = mbi.hwndMB;

	SendMessage(hMDCmdBar, TB_SETTOOLTIPS, (WPARAM)NUM_MD_TOOLTIP, (LPARAM)pMDToolTip);

	ShowWindow(hMDCmdBar, SW_HIDE);

	RECT rMenuRect;
	GetWindowRect(hMSCmdBar, &rMenuRect);
	nHOffset = rMenuRect.bottom - rMenuRect.top;

	r.bottom -= nHOffset - 1;
#endif
#if defined(PLATFORM_PSPC)
	hMSCmdBar = MakeCommandBar(pcs->hInstance, hWnd, ID_CMDBAR_MAIN,
								IDR_MENU_MAIN, aCmdBarButtons, 
								NUM_CMDBAR_BUTTONS, NUM_IMG_BUTTONS, 
								IDB_TOOLBAR);
	hMDCmdBar = MakeCommandBar(pcs->hInstance, hWnd, ID_CMDBAR_DETAILS,
								IDR_MENU_DETAILS, aMDCmdBarButtons,
								NUM_MD_CMDBAR_BUTTONS, NUM_IMG_BUTTONS,
								IDB_TOOLBAR);
    CommandBar_Show(hMSCmdBar, TRUE);
	nHOffset = CommandBar_Height(hMSCmdBar);
	r.top += nHOffset;
	r.bottom -= nHOffset;
#endif
#if defined(PLATFORM_HPC)
	HWND hBand, hwnd;
	REBARBANDINFO arbbi[2];

	// CommandBand生成
	HIMAGELIST himl = ImageList_Create(16,16,ILC_COLOR, 0, 1);
	HBITMAP hBmp = LoadBitmap(pcs->hInstance, MAKEINTRESOURCE(IDB_REBAR));
	ImageList_Add(himl, hBmp, (HBITMAP)NULL);
	DeleteObject(hBmp);

	hBand = CommandBands_Create(pcs->hInstance, hWnd, IDC_CMDBAND,
								RBS_AUTOSIZE | RBS_VARHEIGHT | RBS_BANDBORDERS | RBS_SMARTLABELS, 
								himl);
	arbbi[0].cbSize = sizeof(REBARBANDINFO);
	arbbi[0].fMask = RBBIM_ID | RBBIM_STYLE | RBBIM_SIZE | RBBIM_IMAGE;
	arbbi[0].fStyle = RBBS_NOGRIPPER;
	arbbi[0].wID = ID_CMDBAR_MAIN;
	arbbi[0].cx = 280;
	arbbi[0].iImage = 0;

	arbbi[1].cbSize = sizeof(REBARBANDINFO);
	arbbi[1].fMask = RBBIM_ID | RBBIM_STYLE | RBBIM_SIZE;
	arbbi[1].fStyle = 0;
	arbbi[1].wID = ID_BUTTONBAND;
	arbbi[1].cx = 360;

	// restore commandbar info
	COMMANDBANDSRESTOREINFO cbri[2];
	BOOL bRestoreFlg = GetCommandbarInfo(cbri, 2);
	if (bRestoreFlg) {
		arbbi[0].fStyle = cbri[0].fStyle;
		arbbi[0].cx = cbri[0].cxRestored;

		arbbi[1].fMask |= RBBIM_STYLE;
		arbbi[1].fStyle = cbri[1].fStyle;
		arbbi[1].cx = cbri[1].cxRestored;
	}

	CommandBands_AddBands(hBand, pcs->hInstance, 2, arbbi);
	// 0番目のバンド(メニュー)の設定
	hwnd = GetCommandBar(hBand, ID_CMDBAR_MAIN);
	CommandBar_InsertMenubar(hwnd, pcs->hInstance, IDR_MENU_MAIN, 0);

	// 1番目のバンド(ボタン)の設定
	hwnd = GetCommandBar(hBand, ID_BUTTONBAND);

	CommandBar_AddBitmap(hwnd, HINST_COMMCTRL, IDB_STD_SMALL_COLOR, 15, 0, 0);
	CommandBar_AddBitmap(hwnd, pcs->hInstance, IDB_TOOLBAR, NUM_IMG_BUTTONS, 0, 0);

	CommandBar_AddButtons(hwnd, sizeof(aCmdBarButtons)/sizeof(TBBUTTON), aCmdBarButtons);

	hMSCmdBar = hBand;
	CommandBands_AddAdornments(hBand, pcs->hInstance, 0, NULL);
	nHOffset = CommandBands_Height(hBand);
	r.top += nHOffset;
	r.bottom -= nHOffset;

	if (bRestoreFlg) {
		if (cbri[0].fMaximized) {
			SendMessage(hBand, RB_MAXIMIZEBAND, 0, (LPARAM)0);
		}
		if (cbri[1].fMaximized) {
			SendMessage(hBand, RB_MAXIMIZEBAND, 1, (LPARAM)0);
		}
	}
	
#endif
#if defined(PLATFORM_BE500)
	// 一覧ビュー
	hMSCmdBar = MakeCSOBar(pcs->hInstance, hWnd, ID_CMDBAR_MAIN);
	HMENU hMSMenu = LoadMenu(pcs->hInstance, MAKEINTRESOURCE(IDR_MENU_MAIN));
	hMSMemoMenu = aSVCSOBarButtons[0].SubMenu	= GetSubMenu(hMSMenu, 0);
	hMSToolMenu = aSVCSOBarButtons[1].SubMenu	= GetSubMenu(hMSMenu, 1);
	for (int i = 0; i < NUM_SV_CMDBAR_BUTTONS; i++) {
		aSVCSOBarButtons[i].reshInst = pcs->hInstance;
	}
	CSOBar_AddButtons(hMSCmdBar, pcs->hInstance, NUM_SV_CMDBAR_BUTTONS, &aSVCSOBarButtons[0]);

	// 詳細ビュー
	hMDCmdBar = MakeCSOBar(pcs->hInstance, hWnd, ID_CMDBAR_DETAILS);
	HMENU hMDMenu = LoadMenu(pcs->hInstance, MAKEINTRESOURCE(IDR_MENU_DETAILS));
	hMDEditMenu = aDVCSOBarButtons[2].SubMenu	= GetSubMenu(hMDMenu, 0);
	for (i = 0; i < NUM_DV_CMDBAR_BUTTONS; i++) {
		aDVCSOBarButtons[i].reshInst = pcs->hInstance;
	}
	CSOBar_AddButtons(hMDCmdBar, pcs->hInstance, NUM_DV_CMDBAR_BUTTONS, &aDVCSOBarButtons[0]);

    CSOBar_Show(hMSCmdBar, TRUE);
	nHOffset = CSOBar_Height(hMSCmdBar);
	r.top += nHOffset;
	r.bottom -= nHOffset;
#endif
#if defined(PLATFORM_WIN32)
	// Rebar
	 hRebar = CreateWindowEx(WS_EX_TOOLWINDOW, REBARCLASSNAME, NULL,
								WS_BORDER | RBS_BANDBORDERS | RBS_AUTOSIZE | 
								WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS| RBS_TOOLTIPS | 
								WS_CLIPCHILDREN|RBS_VARHEIGHT,
								0, 0, 0, 0,
								hWnd, NULL, pcs->hInstance, NULL);
	REBARINFO rbi;
	rbi.cbSize = sizeof(rbi);
	rbi.fMask = 0;
	rbi.himl = NULL;
	SendMessage(hRebar, RB_SETBARINFO, 0, (LPARAM)&rbi);

	hToolBar = CreateToolBar(hRebar, pcs->hInstance);

	REBARBANDINFO rbband;
	rbband.cbSize = sizeof(rbband);
	rbband.fMask = RBBIM_SIZE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE ;
	rbband.fStyle = RBBS_CHILDEDGE | RBBS_GRIPPERALWAYS;

	DWORD dwBtnSize = SendMessage(hToolBar, TB_GETBUTTONSIZE, 0, 0);

 	rbband.cbSize = sizeof(rbband);
	rbband.hwndChild  = hToolBar;
	rbband.cxMinChild = 0;
	rbband.cyMinChild = HIWORD(dwBtnSize);
	rbband.cx         = 250;

	SendMessage(hRebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbband);
	SendMessage(hRebar, RB_MAXIMIZEBAND, 0, 0);
#endif

	// Status Bar
#if defined(PLATFORM_HPC)
	hStatusBar = CreateStatusWindow(WS_CHILD , TEXT(""), 
									hWnd, IDC_STATUS);
#endif
#if defined(PLATFORM_WIN32)
	hStatusBar = CreateStatusWindow(WS_CHILD | SBARS_SIZEGRIP, "", 
									hWnd, IDC_STATUS);
#endif
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	ResizeStatusBar();
	SendMessage(hStatusBar, SB_SETTEXT, 0 | SBT_NOBORDERS , (LPARAM)"");
	SetNewMemoStatus(g_Property.IsUseTwoPane());
	SetModifyStatus(FALSE);

	// control show/hide status bar
	HMENU hMenu = GetMainMenu();
	if (g_Property.HideStatusBar()) {
		CheckMenuItem(hMenu, IDM_SHOWSTATUSBAR, MF_BYCOMMAND | MF_UNCHECKED);
	} else {
		CheckMenuItem(hMenu, IDM_SHOWSTATUSBAR, MF_BYCOMMAND | MF_CHECKED);
		ShowWindow(hStatusBar, SW_SHOW);
	}	


#endif

#if defined(PLATFORM_WIN32)
	// 1-Pane状態で起動された場合、ツールバーの画面切り替えボタンを押下状態とする
	if (!g_Property.IsUseTwoPane()) {
		SendMessage(hToolBar, TB_PRESSBUTTON, IDM_TOGGLEPANE, MAKELONG(TRUE, 0));
		HMENU hMenu = GetMenu(hWnd);
		BOOL b = CheckMenuItem(hMenu, IDM_TOGGLEPANE, MF_BYCOMMAND | MF_UNCHECKED);
	}
#endif
#if defined(PLATFORM_HPC)
	if (!g_Property.IsUseTwoPane()) {
		HMENU hMenu = CommandBar_GetMenu(GetCommandBar(hMSCmdBar, ID_CMDBAR_MAIN), 0);
		SendMessage(GetCommandBar(hMSCmdBar, ID_BUTTONBAND), TB_PRESSBUTTON, IDM_TOGGLEPANE, MAKELONG(TRUE, 0));
		CheckMenuItem(hMenu, IDM_TOGGLEPANE, MF_BYCOMMAND | MF_UNCHECKED);
	}
#endif

	// メモ詳細ビュー生成
	mdView.Create(TEXT("MemoDetails"), r, hWnd, IDC_MEMODETAILSVIEW, IDC_MEMODETAILSVIEW_NF, hInstance, g_Property.DetailsViewFont());

	if (!g_Property.WrapText()) {
		SetWrapText(g_Property.WrapText());
	}

	// メモ選択ビュー生成
	msView.Create(TEXT("MemoSelect"), r, hWnd, IDC_MEMOSELECTVIEW, hInstance, g_Property.SelectViewFont());
	msView.InitTree();

	if (g_Property.IsUseTwoPane()) {
		// 自動切換えモードに設定
		msView.SetAutoLoadMode(g_Property.AutoSelectMemo());
		msView.SetSingleClickMode(g_Property.SingleClickOpenMemo());

		// マルチペインに伴うウィンドウの再レイアウト
	}
	LoadWinSize(hWnd);
	mdView.SetMemo(TEXT(""), 0, FALSE);

	if (!EnableApplicationButton(hWnd)) {
		TomboMessageBox(hMainWnd, MSG_INITAPPBTN_FAIL, TEXT("Warning"), MB_ICONEXCLAMATION | MB_OK);
	}

	if (g_Property.IsUseTwoPane()) {
		// 初期表示時の詳細ビューは新規メモと同様の扱いとする
		mmMemoManager.NewMemo();
	}

#if defined(PLATFORM_WIN32)
	SetTopMost();
#endif
	ActivateView(TRUE);
}

#if defined(PLATFORM_WIN32)
///////////////////////////////////////////////////
// ツールバーの生成

HWND CreateToolBar(HWND hParent, HINSTANCE hInst)
{
	HWND hwndTB;
	TBADDBITMAP tbab;

	hwndTB = CreateWindowEx(WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, (LPSTR)NULL, 
							WS_CHILD | 
							WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CCS_NODIVIDER | CCS_NORESIZE |
							TBSTYLE_FLAT | TBSTYLE_ALTDRAG |
							TBSTYLE_TOOLTIPS| CCS_ADJUSTABLE ,
							0, 0, 0, 0, 
							hParent, (HMENU)IDC_TOOLBAR, hInst, NULL);
	SendMessage(hwndTB, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

	tbab.hInst = hInst;
	tbab.nID = IDB_TOOLBAR;
    SendMessage(hwndTB, TB_ADDBITMAP, (WPARAM) 3, (LPARAM) &tbab);  

	tbab.hInst = HINST_COMMCTRL;
	tbab.nID = IDB_STD_SMALL_COLOR;
    SendMessage(hwndTB, TB_ADDBITMAP, (WPARAM) NUM_TOOLBAR_BUTTONS, (LPARAM) &tbab);  

    
	SendMessage(hwndTB, TB_ADDBUTTONS, (WPARAM) NUM_TOOLBAR_BUTTONS, 
        (LPARAM) (LPTBBUTTON) &aToolbarButtons); 
     return hwndTB; 
}
#endif

#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
///////////////////////////////////////////////////
// ステータスバーのリサイズ
///////////////////////////////////////////////////
void MainFrame::ResizeStatusBar()
{
	RECT r;
	GetClientRect(hStatusBar, &r);
	DWORD nHeight = r.bottom - r.top;
	DWORD nWidth = r.right - r.left;

	int nSep[4];

	DWORD nWndSize = nHeight * 2;
	nSep[0] = nWidth - nWndSize*3 - nHeight;
	nSep[1] = nSep[0] + nWndSize;
	nSep[2] = nSep[1] + nWndSize;
	nSep[3] = nSep[2] + nWndSize;
	SendMessage(hStatusBar, SB_SETPARTS, (WPARAM)4, (LPARAM)nSep);
}
#endif

///////////////////////////////////////////////////
// set status indicator on statusbar
///////////////////////////////////////////////////

void MainFrame::SetStatusIndicator(DWORD nPos, LPCTSTR pText, BOOL bDisp)
{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	LPCTSTR p;
	if (bDisp) {
		p = pText;
	} else {
		p = TEXT("");
	}
	SendMessage(hStatusBar, SB_SETTEXT, nPos, (LPARAM)p);
#endif
}

void MainFrame::SetModifyStatus(BOOL bModify)
{
	EnableSaveButton(bModify);
	SetStatusIndicator(3, MSG_UPDATE, bModify);
}

///////////////////////////////////////////////////
// 終了
///////////////////////////////////////////////////

BOOL MainFrame::OnExit()
{
	DWORD nYNC;
	if (!mmMemoManager.SaveIfModify(&nYNC, FALSE)) {
		TCHAR buf[1024];
		wsprintf(buf, MSG_SAVE_FAILED, GetLastError());
		TomboMessageBox(hMainWnd, buf, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
		ActivateView(FALSE);
		return FALSE;
	}
	if (nYNC == IDCANCEL) return FALSE;
	pmPasswordMgr.ForgetPassword();
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	SaveWinSize();
	g_Property.SaveStatusBarStat();
#endif

	g_Property.SaveWrapTextStat();

#if defined(PLATFORM_WIN32)
	g_Property.SaveTopMostStat();
#endif
#if defined(PLATFORM_HPC)
	// save rebar info
	COMMANDBANDSRESTOREINFO cbri[2];
	cbri[0].cbSize = cbri[1].cbSize = sizeof(COMMANDBANDSRESTOREINFO);
	CommandBands_GetRestoreInformation(hMSCmdBar, SendMessage(hMSCmdBar, RB_IDTOINDEX, ID_CMDBAR_MAIN, 0), &cbri[0]);
	CommandBands_GetRestoreInformation(hMSCmdBar, SendMessage(hMSCmdBar, RB_IDTOINDEX, ID_BUTTONBAND, 0), &cbri[1]);
	SetCommandbarInfo(cbri, 2);
#endif
	PostQuitMessage(0);
	return TRUE;
}

///////////////////////////////////////////////////
// WM_COMMANDの処理
///////////////////////////////////////////////////

void MainFrame::OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// WM_COMMANDはまず現在アクティブになっているビューに処理させる
	if (bSelectViewActive) {
		if (msView.OnCommand(hWnd, wParam, lParam)) return;
	} else {
		if (mdView.OnCommand(hWnd, wParam, lParam)) return;
	}

	// 処理されなかったコマンドの場合、メインウィンドウでの処理を試みる
	switch(LOWORD(wParam)) {
#if defined(PLATFORM_BE500)
	case CSOBAR_ADORNMENTID_CLOSE:
		/* fall through */
#endif
	case IDM_EXIT:
		SendMessage(hWnd, WM_CLOSE, 0, 0);
		break;
	case IDM_NEWMEMO:
		NewMemo();
		break;
	case IDM_NEWFOLDER:
		NewFolder(NULL);
		break;
	case IDM_ABOUT:
		About();
		break;
	case IDM_RETURNLIST:
		OnList(TRUE);
		break;
	case IDM_PROPERTY:
		OnProperty();
		break;
	case IDM_FORGETPASS:
		OnForgetPass();
		break;
	case IDM_SELALL:
		mmMemoManager.SelectAll();
		break;
	case IDM_SAVE:
		if (!mmMemoManager.SaveIfModify(NULL, FALSE)) {
			TCHAR buf[1024];
			wsprintf(buf, MSG_SAVE_FAILED, GetLastError());
			TomboMessageBox(NULL, buf, TEXT("ERROR"), MB_ICONERROR | MB_OK);
		}
		break;
	case IDM_DETAILS_HSCROLL:
		g_Property.SetWrapText(!g_Property.WrapText());
		SetWrapText(g_Property.WrapText());
		break;
#if defined(PLATFORM_HPC) || defined(PLATFORM_WIN32)
	case IDM_TOGGLEPANE:
		TogglePane();
		break;
#endif
#if defined(PLATFORM_WIN32)
	case IDM_TOPMOST:
		g_Property.ToggleStayTopMost();
		SetTopMost();
		break;
#endif
	case IDM_SEARCH:
		OnSearch();
		break;
	case IDM_SEARCH_NEXT:
		OnSearchNext(TRUE);
		break;
	case IDM_SEARCH_PREV:
		OnSearchNext(FALSE);
		break;
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	case IDM_SHOWSTATUSBAR:
		ToggleShowStatusBar();
		break;
#endif
	case IDM_GREP:
		OnGrep();
		break;
	}
	return;
}

///////////////////////////////////////////////////
// WM_NOTIFYの処理
///////////////////////////////////////////////////
// 基本は各ViewにDispatchする。
// Dispatchに失敗したらFALSEを返す。

BOOL MainFrame::OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (wParam == IDC_MEMOSELECTVIEW) {
		return msView.OnNotify(hWnd, wParam, lParam);
	}
#if defined(PLATFORM_HPC)
	if (wParam == IDC_CMDBAND) {
		// when move commandbar, Realign MS/MD view.
		NMREBAR *pnm = (NMREBAR*)lParam;
		if (pnm->hdr.code == RBN_HEIGHTCHANGE) {
			RECT r;
			GetClientRect(hMainWnd, &r);
			OnResize(0, MAKELPARAM(r.right - r.left, r.bottom - r.top));
		}
	}
#endif
	return FALSE;
}

///////////////////////////////////////////////////
// システム設定の変更
///////////////////////////////////////////////////
// 現在はIMEのON/OFFのチェックのみ

void MainFrame::OnSettingChange(WPARAM wParam)
{
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_PSPC) || defined(PLATFORM_BE500)
	BOOL bStat;
	SipControl sc;
	if (!sc.Init()) return;
	if (!sc.GetSipStat(&bStat)) return;

	RECT r = sc.GetRect();
	OnSIPResize(bStat, r.bottom - r.top);
#endif
#if defined(PLATFORM_HPC) || defined(PLATFORM_PSPC)
	if (wParam == SPI_SETWORKAREA) {
		// Change taskbar size
		RECT r;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
		MoveWindow(hMainWnd, r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE);
	}
#endif
}

///////////////////////////////////////////////////
// IMのON/OFFに伴うリサイズ
///////////////////////////////////////////////////

void MainFrame::OnSIPResize(BOOL bImeOn, DWORD nSipHeight)
{
#ifdef _WIN32_WCE
#ifdef PLATFORM_PKTPC
	DWORD nBottom;
	RECT r, rWinRect;

	GetClientRect(hMainWnd, &rWinRect);
	GetWindowRect(hMSCmdBar, &r);
	
	nBottom = rWinRect.bottom - (r.bottom - r.top); 

	if (bImeOn) {
		msView.MoveWindow(rWinRect.left, rWinRect.top, rWinRect.right, nBottom - nSipHeight);
		mdView.MoveWindow(rWinRect.left, rWinRect.top, rWinRect.right, nBottom - nSipHeight);
	} else {
		msView.MoveWindow(rWinRect.left, rWinRect.top, rWinRect.right, nBottom);
		mdView.MoveWindow(rWinRect.left, rWinRect.top, rWinRect.right, nBottom);
	}
#endif
#if defined(PLATFORM_PSPC) || defined(PLATFORM_BE500)
	DWORD nTop, nBottom;

#if defined(PLATFORM_PSPC)
	DWORD nHOffset = CommandBar_Height(hMDCmdBar);
#else
	DWORD nHOffset = CSOBar_Height(hMDCmdBar);
#endif
	nTop = nHOffset;
	nBottom = 320 - nHOffset * 2;
	if (bImeOn) {
		msView.MoveWindow(0, nTop, 240, nBottom - nSipHeight);
		mdView.MoveWindow(0, nTop, 240, nBottom - nSipHeight);
	} else {
		msView.MoveWindow(0, nTop, 240, nBottom);
		mdView.MoveWindow(0, nTop, 240, nBottom);
	}
#endif
#endif
}

///////////////////////////////////////////////////
// KeyUpイベントのハンドリング
///////////////////////////////////////////////////

BOOL MainFrame::OnHotKey(WPARAM wParam, LPARAM lParam)
{
	if (::bDisableHotKey) return FALSE;
	if (bSelectViewActive) {
		return msView.OnHotKey(hMainWnd, wParam);
	} else {
		return mdView.OnHotKey(hMainWnd, wParam);
	}
}

///////////////////////////////////////////////////
//  リサイズ
///////////////////////////////////////////////////

void MainFrame::OnResize(WPARAM wParam, LPARAM lParam)
{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	WORD fwSizeType = wParam;
	WORD nWidth = LOWORD(lParam);
	WORD nHeight = HIWORD(lParam);

#if defined(PLATFORM_WIN32)
	RECT r;
	GetWindowRect(hRebar, &r);
	WORD nRebarH = (WORD)(r.bottom - r.top);
#endif
#if defined(PLATFORM_HPC)
	WORD nRebarH = CommandBands_Height(hMSCmdBar);
#endif

	WORD wLeftWidth, wHeight;
	msView.GetSize(&wLeftWidth, &wHeight);

	WORD nStatusHeight;
	if (g_Property.HideStatusBar()) {
		nStatusHeight = 0;
		ShowWindow(hStatusBar, SW_HIDE);
	} else {
		RECT rStatus;
		GetWindowRect(hStatusBar, &rStatus);
		nStatusHeight = (WORD)(rStatus.bottom - rStatus.top);
		ShowWindow(hStatusBar, SW_SHOW);
	}

	if (g_Property.IsUseTwoPane()) {
		msView.MoveWindow(0, nRebarH , wLeftWidth, nHeight-nRebarH - nStatusHeight);
		mdView.MoveWindow(wLeftWidth + BORDER_WIDTH, nRebarH, nWidth - wLeftWidth - BORDER_WIDTH, nHeight-nRebarH - nStatusHeight);
	} else {
		RECT rc;
		GetClientRect(hMainWnd, &rc);

		msView.MoveWindow(0, nRebarH, rc.right, rc.bottom - nRebarH - nStatusHeight);
		mdView.MoveWindow(0, nRebarH, rc.right, rc.bottom - nRebarH - nStatusHeight);
	}

#endif // PLATFORM_WIN32 || PLATFORM_HPC

#if defined(PLATFORM_WIN32)
	// Rebar
	SendMessage(hRebar, WM_SIZE, wParam, lParam);
#endif
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	// Staus bar
	SendMessage(hStatusBar, WM_SIZE, wParam, lParam);
	ResizeStatusBar();
#endif
}

///////////////////////////////////////////////////
//  ツールチップ
///////////////////////////////////////////////////

void MainFrame::OnTooltip(WPARAM wParam, LPARAM lParam)
{
}

///////////////////////////////////////////////////
//  左ボタン押下
///////////////////////////////////////////////////
//
// ペイン配分の変更開始

void MainFrame::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	WORD fwKeys = wParam;
	WORD xPos = LOWORD(lParam);

	if (!(fwKeys & MK_LBUTTON)) return;
	bResizePane = TRUE;
	SetCapture(hMainWnd);
#endif
}

///////////////////////////////////////////////////
//  マウスドラッグ
///////////////////////////////////////////////////
//
// ペイン配分の変更中

void MainFrame::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	WORD fwKeys = wParam;
	WORD xPos = LOWORD(lParam);

	if (!(fwKeys & MK_LBUTTON) && !bResizePane) return;
	MovePane(xPos);
#endif
}

///////////////////////////////////////////////////
//  左ボタン開放
///////////////////////////////////////////////////
//
// ペイン配分の変更終了

void MainFrame::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	WORD fwKeys = wParam;
	WORD xPos = LOWORD(lParam);

	if (!(fwKeys & MK_LBUTTON) && !bResizePane) return;
	bResizePane = FALSE;
	ReleaseCapture();

	RECT r;
	GetClientRect(hMainWnd, &r);
	WORD wTotalWidth = (WORD)(r.right - r.left);
	if (xPos < 20) {
		xPos = 20;
	}
	if (xPos > wTotalWidth - 20) {
		xPos = wTotalWidth - 20;
	}
	MovePane(xPos);
#endif 
}

///////////////////////////////////////////////////
//  ペイン配分の変更処理
///////////////////////////////////////////////////
void MainFrame::MovePane(WORD width)
{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	if (!g_Property.IsUseTwoPane()) return;

	WORD wLeftWidth, wHeight;
	msView.GetSize(&wLeftWidth, &wHeight);
	RECT r;
	GetClientRect(hMainWnd, &r);
	WORD wTotalWidth = (WORD)(r.right - r.left);

#if defined(PLATFORM_WIN32)
	RECT r2;
	GetWindowRect(hRebar, &r2);
	WORD nRebarH = (WORD)(r2.bottom - r2.top);
#endif
#if defined(PLATFORM_HPC)
	WORD nRebarH = CommandBands_Height(hMSCmdBar);
#endif
	msView.MoveWindow(0, nRebarH, width, wHeight);
	mdView.MoveWindow(width + BORDER_WIDTH , nRebarH, 
						wTotalWidth - width - BORDER_WIDTH, wHeight);
#endif
}

///////////////////////////////////////////////////
// 
///////////////////////////////////////////////////

void MainFrame::SetFocus()
{
	if (bSelectViewActive) {
		msView.SetFocus();
	} else {
		mdView.SetFocus();
	}
}

///////////////////////////////////////////////////
// 新規メモ
///////////////////////////////////////////////////

void MainFrame::NewMemo()
{
	SipControl sc;
	if (!sc.Init() || !sc.SetSipStat(TRUE)) {
		TomboMessageBox(hMainWnd, MSG_GETSIPSTAT_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
	}

	// メモ編集中に新規メモを作成した場合、保存確認して新規メモに移る
	if (g_Property.IsUseTwoPane()) {
		OnList(TRUE);
	}
	SetNewMemoStatus(TRUE);
	mmMemoManager.NewMemo();

	ActivateView(FALSE);
}

///////////////////////////////////////////////////
// 新規フォルダ
///////////////////////////////////////////////////

void MainFrame::NewFolder(TreeViewItem *pItem)
{
	if (!mmMemoManager.MakeNewFolder(hMainWnd, pItem)) {
		TomboMessageBox(hMainWnd, MSG_CREATEFOLDER_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
	}
}

///////////////////////////////////////////////////
// メモ一覧に戻る
///////////////////////////////////////////////////

void MainFrame::OnList(BOOL bAskSave)
{
	SipControl sc;
	if (!sc.Init() || !sc.SetSipStat(FALSE)) {
		TomboMessageBox(hMainWnd, MSG_GETSIPSTAT_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
	}

	DWORD nYNC;
	BOOL bResult;
	if (bAskSave) {
		bResult = mmMemoManager.SaveIfModify(&nYNC, FALSE);
	} else {
		nYNC = IDYES;
		bResult = mmMemoManager.SaveIfModify(NULL, TRUE);
	}
	if (!bResult) {
		TCHAR buf[1024];
		wsprintf(buf, MSG_SAVE_FAILED, GetLastError());
		TomboMessageBox(hMainWnd, buf, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
		ActivateView(FALSE);
		return;
	}
	if (nYNC == IDCANCEL) return;

	ActivateView(TRUE);

	if (g_Property.IsUseTwoPane()) {
		// 2Paneの場合、暗号化されたメモのみクリアする
		if (nYNC == IDNO) {
			// メモを破棄し、旧メモをリロード
			RequestOpenMemo(&(mmMemoManager.CurrentLoc()), OPEN_REQUEST_MDVIEW_ACTIVE);
		} else {
			MemoNote *pCurrent = mmMemoManager.CurrentNote();
			if (pCurrent && pCurrent->IsEncrypted()) {
				mmMemoManager.NewMemo();
			} else {
#if defined(PLATFORM_HPC)
				RequestOpenMemo(&(mmMemoManager.CurrentLoc()), OPEN_REQUEST_MDVIEW_ACTIVE);
#endif
			}
		}
	} else {
		mmMemoManager.NewMemo();
		SetNewMemoStatus(FALSE);
	}

#if defined(PLATFORM_PKTPC)
	SetTitle(TOMBO_APP_NAME);
#endif
	SetFocus();
}

///////////////////////////////////////////////////
// バージョン情報表示
///////////////////////////////////////////////////

void MainFrame::About()
{
	AboutDialog dlg;
	BOOL bPrev = bDisableHotKey;
	bDisableHotKey = TRUE;
	dlg.Popup(hInstance, hMainWnd);
	bDisableHotKey = bPrev;
}

///////////////////////////////////////////////////
// メモオープン要求
///////////////////////////////////////////////////
// bSwitchViewがTRUEの場合には詳細ビューに切り替える

void MainFrame::RequestOpenMemo(MemoLocator *pLoc, DWORD nSwitchView)
{
	MemoNote *pNote = pLoc->GetNote();
	if (pNote == NULL) {
		if (pLoc->IsDeleteReceived()) delete pLoc;
		return; // フォルダの場合
	}
	if (((nSwitchView & OPEN_REQUEST_MSVIEW_ACTIVE) == 0) && (pNote->IsEncrypted() && !pmPasswordMgr.IsRememberPassword())) {
		// bSwitchViewがFALSEで、メモを開くためにパスワードを問い合わせる必要がある場合には
		// メモは開かない
		if (pLoc->IsDeleteReceived()) delete pLoc;
		return;
	}
	mmMemoManager.SetMemo(pLoc);

#if defined(PLATFORM_WIN32) || defined(PLATFORM_PKTPC)
	LPCTSTR pPath = pNote->MemoPath();
	LPCTSTR pPrefix = TEXT("Tombo - ");
	if (pPath) {
		LPCTSTR pBase = _tcsrchr(pPath, TEXT('\\'));
		if (pBase) {
			pBase++;
		} else {
			pBase = pPath;
		}
		LPTSTR p = new TCHAR[_tcslen(pBase) + 1 + _tcslen(pPrefix)];
		if (p) {
			// PocketPCの場合画面が狭いのでアプリ名は表示させない
#if defined(PLATFORM_WIN32)
			wsprintf(p, TEXT("%s%s"), pPrefix, pBase);
#endif
#if defined(PLATFORM_PKTPC)
			wsprintf(p, TEXT("%s"), pBase);
#endif
			DWORD l = _tcslen(p);
			if (l > 4) {
				*(p + l - 4) = TEXT('\0');
			}
			if (g_Property.SwitchWindowTitle()) {
				SetWindowText(hMainWnd, p);
			}
			delete [] p;
		}
	}	
#endif

	SetNewMemoStatus(FALSE);
	if (!(nSwitchView & OPEN_REQUEST_NO_ACTIVATE_VIEW)) {
		if (g_Property.IsUseTwoPane()) {
			if (nSwitchView & OPEN_REQUEST_MSVIEW_ACTIVE) {
				ActivateView(FALSE);
			}
		} else {
			ActivateView(FALSE);
		}
	}
	if (pLoc->IsDeleteReceived()) delete pLoc;
}

///////////////////////////////////////////////////
// ビューの切り替え処理
///////////////////////////////////////////////////

void MainFrame::ActivateView(BOOL bList)
{
	if (bSelectViewActive == bList) return;

	bSelectViewActive = bList;

	// メニューのコントロール
#if defined(PLATFORM_WIN32)
	HMENU hMenu = GetMenu(hMainWnd);
	ControlMenu(hMenu, bSelectViewActive);
	ControlToolbar(hToolBar, bSelectViewActive);
#endif
#if defined(PLATFORM_HPC)
	HMENU hMenu = CommandBar_GetMenu(GetCommandBar(hMSCmdBar, ID_CMDBAR_MAIN), 0);
	ControlMenu(hMenu, bSelectViewActive);
	ControlToolbar(GetCommandBar(hMSCmdBar, ID_BUTTONBAND), bSelectViewActive);
#endif

	if (g_Property.IsUseTwoPane()) {
		// 2-Pane版では両ビューを同時表示
		mdView.Show(SW_SHOW);
		msView.Show(SW_SHOW);
	} else {
		// CE版(& CEデバグ版 on Win32) ではビューの切り替えを行う
		// ビューの表示・非表示の切り替え
		// コマンドバーの切り替え
		// フォーカスの切り替え

		if (bSelectViewActive) {
			// 一覧ビュー
			mdView.Show(SW_HIDE);
			msView.Show(SW_SHOW);
#if defined(PLATFORM_PKTPC)
			ShowWindow(hMDCmdBar, SW_HIDE);
			ShowWindow(hMSCmdBar, SW_SHOW);
#endif
#if defined(PLATFORM_PSPC) 
			CommandBar_Show(hMDCmdBar, SW_HIDE);
			CommandBar_Show(hMSCmdBar, SW_SHOW);
#endif
#if defined(PLATFORM_BE500)
			CSOBar_Show(hMDCmdBar, SW_HIDE);
			CSOBar_Show(hMSCmdBar, SW_SHOW);
#endif
		} else {
			// 詳細ビュー
			msView.Show(SW_HIDE);
			mdView.Show(SW_SHOW);

#if defined(PLATFORM_PKTPC)
			ShowWindow(hMSCmdBar, SW_HIDE);
			ShowWindow(hMDCmdBar, SW_SHOW);
#endif
#if defined(PLATFORM_PSPC)
			CommandBar_Show(hMSCmdBar, SW_HIDE);
			CommandBar_Show(hMDCmdBar, SW_SHOW);
#endif
#if defined(PLATFORM_BE500)
			CSOBar_Show(hMSCmdBar, SW_HIDE);
			CSOBar_Show(hMDCmdBar, SW_SHOW);
#endif
		}
	}
	SetFocus();
}

///////////////////////////////////////////////////
// メニューのコントロール

#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
static void ControlMenu(HMENU hMenu, BOOL bSelectViewActive)
{
	UINT uDisableFlg = MF_BYCOMMAND | MF_GRAYED;
#if defined(PLATFORM_WIN32)
	uDisableFlg |= MF_DISABLED;
#endif

	UINT uFlg1, uFlg2;
	if (bSelectViewActive) {
		uFlg1 = MF_BYCOMMAND | MF_ENABLED;
		uFlg2 = uDisableFlg;
	} else {
		uFlg1 = uDisableFlg;
		uFlg2 = MF_BYCOMMAND | MF_ENABLED;
	}

	EnableMenuItem(hMenu, IDM_FORGETPASS, uFlg1);
	EnableMenuItem(hMenu, IDM_PROPERTY, uFlg1);
	EnableMenuItem(hMenu, IDM_TOGGLEPANE, uFlg1);

	EnableMenuItem(hMenu, IDM_INSDATE1, uFlg2);
	EnableMenuItem(hMenu, IDM_INSDATE2, uFlg2);
	EnableMenuItem(hMenu, IDM_DETAILS_HSCROLL, uFlg2);
}
#endif

///////////////////////////////////////////////////
// ツールバーのボタン状態の更新
///////////////////////////////////////////////////
void MainFrame::EnableMenu(UINT uId, BOOL bEnable)
{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	HMENU hMenu = GetMainMenu();
	UINT uDisableFlg = MF_BYCOMMAND | MF_GRAYED;
#if defined(PLATFORM_WIN32)
	uDisableFlg |= MF_DISABLED;
#endif

	UINT uFlg1;
	if (bEnable) {
		uFlg1 = MF_BYCOMMAND | MF_ENABLED;
	} else {
		uFlg1 = uDisableFlg;
	}
	EnableMenuItem(hMenu, uId, uFlg1);
#endif
}


///////////////////////////////////////////////////
// ツールバーのボタン状態の更新
///////////////////////////////////////////////////

#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
static void ControlToolbar(HWND hToolbar, BOOL bSelectViewActive)
{
	SendMessage(hToolbar, TB_ENABLEBUTTON, IDM_INSDATE1, MAKELONG(!bSelectViewActive, 0));
	SendMessage(hToolbar, TB_ENABLEBUTTON, IDM_INSDATE2, MAKELONG(!bSelectViewActive, 0));

	SendMessage(hToolbar, TB_ENABLEBUTTON, IDM_TOGGLEPANE, MAKELONG(bSelectViewActive, 0));
}
#endif

///////////////////////////////////////////////////
// Menu control
///////////////////////////////////////////////////

void MainFrame::EnableEncrypt(BOOL bEnable)
{
	HMENU hMenu = GetMSEditMenu();
	if (bEnable) {
		EnableMenuItem(hMenu, IDM_ENCRYPT, MF_BYCOMMAND | MF_ENABLED);
	} else {
		EnableMenuItem(hMenu, IDM_ENCRYPT, MF_BYCOMMAND | MF_GRAYED);
	}
}

void MainFrame::EnableDecrypt(BOOL bEnable)
{
	HMENU hMenu = GetMSEditMenu();
	if (bEnable) {
		EnableMenuItem(hMenu, IDM_DECRYPT, MF_BYCOMMAND | MF_ENABLED);
	} else {
		EnableMenuItem(hMenu, IDM_DECRYPT, MF_BYCOMMAND | MF_GRAYED);
	}
}

void MainFrame::EnableDelete(BOOL bEnable)
{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	EnableMenu(IDM_DELETEITEM, bEnable);
	SendMessage(GetMainToolBar(), TB_ENABLEBUTTON, IDM_DELETEITEM, MAKELONG(bEnable, 0));
#endif
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_BE500) || defined(PLATFORM_PSPC)
	HMENU hMenu = GetMSEditMenu();
	if (bEnable) {
		EnableMenuItem(hMenu, IDM_DELETEITEM, MF_BYCOMMAND | MF_ENABLED);
	} else {
		EnableMenuItem(hMenu, IDM_DELETEITEM, MF_BYCOMMAND | MF_GRAYED);
	}
#endif
}

void MainFrame::EnableRename(BOOL bEnable)
{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	EnableMenu(IDM_RENAME, bEnable);
#endif
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_BE500) || defined(PLATFORM_PSPC)
	HMENU hMenu = GetMSEditMenu();
	if (bEnable) {
		EnableMenuItem(hMenu, IDM_RENAME, MF_BYCOMMAND | MF_ENABLED);
	} else {
		EnableMenuItem(hMenu, IDM_RENAME, MF_BYCOMMAND | MF_GRAYED);
	}
#endif
}

void MainFrame::EnableNew(BOOL bEnable)
{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	EnableMenu(IDM_NEWMEMO, bEnable);
	SendMessage(GetMainToolBar(), TB_ENABLEBUTTON, IDM_NEWMEMO, MAKELONG(bEnable, 0));
#endif
#if defined(PLATFORM_PKTPC)
	HMENU hMenu = GetMSEditMenu();
	if (bEnable) {
		EnableMenuItem(hMenu, IDM_NEWMEMO, MF_BYCOMMAND | MF_ENABLED);
	} else {
		EnableMenuItem(hMenu, IDM_NEWMEMO, MF_BYCOMMAND | MF_GRAYED);
	}
	SendMessage(hMSCmdBar, TB_ENABLEBUTTON, IDM_NEWMEMO, MAKELONG(bEnable, 0));
#endif
#if defined(PLATFORM_BE500) || defined(PLATFORM_PSPC)
	HMENU hMenu = GetMSEditMenu();
	if (bEnable) {
		EnableMenuItem(hMenu, IDM_NEWMEMO, MF_BYCOMMAND | MF_ENABLED);
	} else {
		EnableMenuItem(hMenu, IDM_NEWMEMO, MF_BYCOMMAND | MF_GRAYED);
	}
#endif
}

void MainFrame::EnableCut(BOOL bEnable)
{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	EnableMenu(IDM_CUT, bEnable);
	SendMessage(GetMainToolBar(), TB_ENABLEBUTTON, IDM_CUT, MAKELONG(bEnable, 0));
#endif
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_BE500) || defined(PLATFORM_PSPC)
	HMENU hMenu = GetMSEditMenu();
	if (bEnable) {
		EnableMenuItem(hMenu, IDM_CUT, MF_BYCOMMAND | MF_ENABLED);
	} else {
		EnableMenuItem(hMenu, IDM_CUT, MF_BYCOMMAND | MF_GRAYED);
	}
#endif
}

void MainFrame::EnableCopy(BOOL bEnable)
{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	EnableMenu(IDM_COPY, bEnable);
	SendMessage(GetMainToolBar(), TB_ENABLEBUTTON, IDM_COPY, MAKELONG(bEnable, 0));
#endif
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_BE500) || defined(PLATFORM_PSPC)
	HMENU hMenu = GetMSEditMenu();
	if (bEnable) {
		EnableMenuItem(hMenu, IDM_COPY, MF_BYCOMMAND | MF_ENABLED);
	} else {
		EnableMenuItem(hMenu, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);
	}
#endif
}

void MainFrame::EnablePaste(BOOL bEnable)
{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	EnableMenu(IDM_PASTE, bEnable);
	SendMessage(GetMainToolBar(), TB_ENABLEBUTTON, IDM_PASTE, MAKELONG(bEnable, 0));
#endif
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_BE500) || defined(PLATFORM_PSPC)
	HMENU hMenu = GetMSEditMenu();
	if (bEnable) {
		EnableMenuItem(hMenu, IDM_PASTE, MF_BYCOMMAND | MF_ENABLED);
	} else {
		EnableMenuItem(hMenu, IDM_PASTE, MF_BYCOMMAND | MF_GRAYED);
	}
#endif
}

void MainFrame::EnableNewFolder(BOOL bEnable)
{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	EnableMenu(IDM_NEWFOLDER, bEnable);
#endif
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_PSPC)
	HMENU hMenu = GetMSEditMenu();
	if (bEnable) {
		EnableMenuItem(hMenu, IDM_NEWFOLDER, MF_BYCOMMAND | MF_ENABLED);
	} else {
		EnableMenuItem(hMenu, IDM_NEWFOLDER, MF_BYCOMMAND | MF_GRAYED);
	}
#endif
#if defined(PLATFORM_BE500)
	HMENU hMenu = GetMSToolMenu();
	if (bEnable) {
		EnableMenuItem(hMenu, IDM_NEWFOLDER, MF_BYCOMMAND | MF_ENABLED);
	} else {
		EnableMenuItem(hMenu, IDM_NEWFOLDER, MF_BYCOMMAND | MF_GRAYED);
	}
#endif
}

void MainFrame::EnableGrep(BOOL bEnable)
{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	EnableMenu(IDM_GREP, bEnable);
#endif
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_BE500) || defined(PLATFORM_PSPC)
	HMENU hMenu = GetMSEditMenu();
	if (bEnable) {
		EnableMenuItem(hMenu, IDM_GREP, MF_BYCOMMAND | MF_ENABLED);
	} else {
		EnableMenuItem(hMenu, IDM_GREP, MF_BYCOMMAND | MF_GRAYED);
	}
#endif
}

///////////////////////////////////////////////////
// パスワード消去
///////////////////////////////////////////////////

void MainFrame::OnForgetPass()
{
	DWORD nYNC;
	if (!mmMemoManager.SaveIfModify(&nYNC, FALSE)) {
		TCHAR buf[1024];
		wsprintf(buf, MSG_SAVE_FAILED, GetLastError());
		TomboMessageBox(hMainWnd, buf, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
		ActivateView(FALSE);
		return;
	}
	if (nYNC == IDCANCEL) return;

	pmPasswordMgr.ForgetPassword();
	TomboMessageBox(hMainWnd, MSG_ERASE_PW, MSG_ERASE_PW_TITLE, MB_ICONINFORMATION | MB_OK);
	mmMemoManager.NewMemo();
}

///////////////////////////////////////////////////
// プロパティの変更
///////////////////////////////////////////////////

void MainFrame::OnProperty()
{
	BOOL bPrev = bDisableHotKey;
	bDisableHotKey = TRUE;

	mmMemoManager.NewMemo();

	int nResult = g_Property.Popup(hInstance, hMainWnd);
	bDisableHotKey = bPrev;
	if (nResult != IDOK) return;

	// フォント設定
	msView.SetFont(g_Property.SelectViewFont());
	mdView.SetFont(g_Property.DetailsViewFont());

	// タブストップの設定
	mdView.SetTabstop();

	// メモフォルダの再構成
	msView.DeleteAllItem();
	msView.InitTree();
#if defined(PLATFORM_WIN32) || defined(PLATFORM_PKTPC)
	if (!g_Property.SwitchWindowTitle()) {
		SetWindowText(hMainWnd, TOMBO_APP_NAME);
	}
#endif
}

///////////////////////////////////////////////////
// タイマー
///////////////////////////////////////////////////

void MainFrame::OnTimer(WPARAM nTimerID)
{
	if (nTimerID == 0) {
		if (!bSelectViewActive) {
			MemoNote *pCurrent = mmMemoManager.CurrentNote();
			if (pCurrent && pCurrent->IsEncrypted()) {
				OnList(FALSE);
			}
		}
		pmPasswordMgr.ForgetPassword();
	} else if (nTimerID == ID_PASSWORDTIMER) {
		pmPasswordMgr.ForgetPasswordIfNotAccessed();
	}
}

///////////////////////////////////////////////////
// 二重起動
///////////////////////////////////////////////////
void MainFrame::OnMutualExecute()
{
	SetForegroundWindow(hMainWnd);
#if defined(PLATFORM_WIN32)
	BringWindowToTop(hMainWnd);
#endif
	OnSettingChange(NULL);
}

///////////////////////////////////////////////////
// アプリケーションボタンの有効化
///////////////////////////////////////////////////
// http://www.pocketpcdn.com/qa/handle_hardware_keys.html

typedef BOOL (__stdcall *UnregisterFunc1Proc)(UINT, UINT);

BOOL MainFrame::EnableApplicationButton(HWND hWnd)
{
#if defined(PLATFORM_PKTPC)
	HINSTANCE hCoreDll;
	UnregisterFunc1Proc procUnregisterFunc;
	hCoreDll = LoadLibrary(TEXT("coredll.dll"));
	if (!hCoreDll) return FALSE;
	procUnregisterFunc = (UnregisterFunc1Proc)GetProcAddress(hCoreDll, TEXT("UnregisterFunc1"));
	if (!procUnregisterFunc) {
		FreeLibrary(hCoreDll);
		return FALSE;
	}
	if (g_Property.AppButton1()) {
		procUnregisterFunc(MOD_WIN, APP_BUTTON1);
		RegisterHotKey(hWnd, APP_BUTTON1, MOD_WIN, APP_BUTTON1);
	}
	if (g_Property.AppButton2()) {
		procUnregisterFunc(MOD_WIN, APP_BUTTON2);
		RegisterHotKey(hWnd, APP_BUTTON2, MOD_WIN, APP_BUTTON2);
	}
	if (g_Property.AppButton3()) {
		procUnregisterFunc(MOD_WIN, APP_BUTTON3);
		RegisterHotKey(hWnd, APP_BUTTON3, MOD_WIN, APP_BUTTON3);
	}
	if (g_Property.AppButton4()) {
		procUnregisterFunc(MOD_WIN, APP_BUTTON4);
		RegisterHotKey(hWnd, APP_BUTTON4, MOD_WIN, APP_BUTTON4);
	}
	if (g_Property.AppButton5()) {
		procUnregisterFunc(MOD_WIN, APP_BUTTON5);
		RegisterHotKey(hWnd, APP_BUTTON5, MOD_WIN, APP_BUTTON5);
	}

	FreeLibrary(hCoreDll);
	return TRUE;
#else
	return TRUE;
#endif
}

///////////////////////////////////////////////////
// ウィンドウサイズの保存
///////////////////////////////////////////////////

void MainFrame::SaveWinSize()
{
#if defined(PLATFORM_HPC) || defined(PLATFORM_WIN32)
	RECT r;
	GetWindowRect(hMainWnd,&r);
	WORD nWidth;
	if (g_Property.IsUseTwoPane()) {
		WORD nHeight;
		msView.GetSize(&nWidth, &nHeight);
	} else {
		RECT r2;
		if (!Property::GetWinSize(&r2, &nWidth)) {
			nWidth = (r.right - r.left) / 3;	
		}
	}
	r.bottom = r.bottom - r.top;
	r.right = r.right - r.left;
	Property::SaveWinSize(&r, nWidth);
#endif
}

///////////////////////////////////////////////////
// ウィンドウサイズの復元
///////////////////////////////////////////////////

void MainFrame::LoadWinSize(HWND hWnd)
{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	RECT rMainFrame;
	WORD nSelectViewWidth;
	RECT rClientRect;
	GetClientRect(hWnd, &rClientRect);

	if (!Property::GetWinSize(&rMainFrame, &nSelectViewWidth)) {
		nSelectViewWidth = (rClientRect.right - rClientRect.left) / 3;	
	} else {
		MoveWindow(hWnd, rMainFrame.left, rMainFrame.top, rMainFrame.right, rMainFrame.bottom, TRUE);
	}

	msView.MoveWindow(0, 0, nSelectViewWidth, rClientRect.bottom);
	// 詳細ビューは一覧ビューのサイズを元に計算されるためここでは変更しない
#endif
}

///////////////////////////////////////////////////
// Editview - Tools menu
///////////////////////////////////////////////////

HMENU MainFrame::GetMDToolMenu()
{
#if defined(PLATFORM_WIN32)
	return GetMenu(hMainWnd);
#endif
#if defined(PLATFORM_PKTPC)
	return SHGetSubMenu(hMDCmdBar, IDM_DETAILS_TOOL);
#endif
#if defined(PLATFORM_PSPC)
	return CommandBar_GetMenu(hMDCmdBar, 0);
#endif
#if defined(PLATFORM_BE500)
	return hMDEditMenu;
#endif
#if defined(PLATFORM_HPC)
	return CommandBar_GetMenu(GetCommandBar(hMSCmdBar, ID_CMDBAR_MAIN), 0);
#endif

}

HMENU MainFrame::GetMSEditMenu()
{
#if defined(PLATFORM_WIN32)
	return GetMenu(hMainWnd);
#endif
#if defined(PLATFORM_PKTPC)
	return SHGetSubMenu(hMSCmdBar, IDM_EDIT_MEMO);
#endif
#if defined(PLATFORM_PSPC)
	return CommandBar_GetMenu(hMSCmdBar, 0);
#endif
#if defined(PLATFORM_HPC)
	return CommandBar_GetMenu(GetCommandBar(hMSCmdBar, ID_CMDBAR_MAIN), 0);
#endif
#if defined(PLATFORM_BE500)
	return hMSMemoMenu;
#endif
}

#if defined(PLATFORM_BE500)
HMENU MainFrame::GetMSToolMenu()
{
	return hMSToolMenu;
}
#endif

///////////////////////////////////////////////////
// 詳細ビューの折り返し表示の制御
///////////////////////////////////////////////////

void MainFrame::SetWrapText(BOOL bWrap)
{
	UINT uCheckFlg;

	HMENU hMenu = GetMDToolMenu();

	if (bWrap) {
		uCheckFlg = MF_CHECKED;
	} else {
		uCheckFlg = MF_UNCHECKED;
	}

	// Change edit view status
	if (!mdView.SetFolding(bWrap)) {
		TomboMessageBox(NULL, MSG_FOLDING_FAILED, TOMBO_APP_NAME, MB_ICONERROR | MB_OK);
		return;
	}

	// CheckMenuItem is superseded, but CE don't have SetMenuItemInfo.
	CheckMenuItem(hMenu, IDM_DETAILS_HSCROLL, MF_BYCOMMAND | uCheckFlg);
}

///////////////////////////////////////////////////
// ペインの切り替え
///////////////////////////////////////////////////
// 2-Pane
//		メニュー               : チェックあり
//		ツールバー             : 押されていない
//		Property::IsUseTwoPane : TRUE
// 1-Pane
//		メニュー               : チェックなし
//		ツールバー             : 押下状態
//		Property::IsUseTwoPane : FALSE

void MainFrame::TogglePane()
{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	MENUITEMINFO mii;
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_STATE;

	HMENU hMenu;
	BOOL bFolding;
	UINT uCheckFlg;

#if defined(PLATFORM_WIN32)
	hMenu = GetMenu(hMainWnd);
#endif
#if defined(PLATFORM_HPC)
	hMenu = CommandBar_GetMenu(GetCommandBar(hMSCmdBar, ID_CMDBAR_MAIN), 0);
#endif

	// メニュー状態の取得
	if (!GetMenuItemInfo(hMenu, IDM_TOGGLEPANE, FALSE, &mii)) return;
	// メニュー状態の反転
	if (mii.fState & MFS_CHECKED) {
		bFolding = TRUE;
		uCheckFlg = MF_UNCHECKED;
	} else {
		bFolding = FALSE;
		uCheckFlg = MF_CHECKED;
	}
	// superseded な関数だが、CEだとSetMenuItemInfoは値の設定ができないのでCheckMenuItemを使用
	CheckMenuItem(hMenu, IDM_TOGGLEPANE, MF_BYCOMMAND | uCheckFlg);

	// ツールバーの制御
#if defined(PLATFORM_WIN32)
	SendMessage(hToolBar, TB_PRESSBUTTON, IDM_TOGGLEPANE, MAKELONG(!uCheckFlg, 0));
#endif
#if defined(PLATFORM_HPC)
	SendMessage(GetCommandBar(hMSCmdBar, ID_BUTTONBAND), TB_PRESSBUTTON, IDM_TOGGLEPANE, MAKELONG(!uCheckFlg, 0));
#endif
	if (g_Property.IsUseTwoPane()) {
		SaveWinSize();
	}

	g_Property.SetUseTwoPane(uCheckFlg);
	RECT r;
	GetClientRect(hMainWnd, &r);

	if (g_Property.IsUseTwoPane()) {
		// 1->2Pane
		RECT rr;
		WORD nWidth;
		g_Property.GetWinSize(&rr, &nWidth);
		msView.MoveWindow(0, 0, nWidth, rr.bottom - rr.top);
		OnResize(0, MAKELPARAM(r.right - r.left, r.bottom - r.top));
		msView.Show(SW_SHOW);
		mdView.Show(SW_SHOW);
	} else {
		// 2->1Pane
		OnResize(0, MAKELPARAM(r.right - r.left, r.bottom - r.top));
		mdView.Show(SW_HIDE);
		msView.Show(SW_SHOW);
	}
#endif
}

///////////////////////////////////////////////////
// ウィンドウタイトルの切り替え
///////////////////////////////////////////////////

void MainFrame::SetTitle(LPCTSTR pTitle) {
	if (!g_Property.SwitchWindowTitle()) return;
	SetWindowText(hMainWnd, pTitle);
}

///////////////////////////////////////////////////
// 検索
///////////////////////////////////////////////////

void MainFrame::OnSearch()
{
	SearchDialog sd;
	if (sd.Popup(g_hInstance, hMainWnd, bSelectViewActive) != IDOK) return;

	SearchEngineA *pSE = new SearchEngineA();
	if(!pSE->Init(sd.IsSearchEncryptMemo(), sd.IsFileNameOnly(), &pmPasswordMgr)) {
		delete pSE;
		return;
	}
	const char *pReason;
	if (!pSE->Prepare(sd.SearchString(), sd.IsCaseSensitive(), &pReason)) {
		LPTSTR p = ConvSJIS2Unicode(pReason);
		if (p) {
			MessageBox(p, TOMBO_APP_NAME, MB_OK | MB_ICONEXCLAMATION);
			delete [] p;
		} else {
			MessageBox(MSG_NOT_ENOUGH_MEMORY, TOMBO_APP_NAME, MB_OK | MB_ICONEXCLAMATION);
		}
		delete pSE;
		return;
	}
	mmMemoManager.SetSearchEngine(pSE);

	// Enable FindNext/Prev button
#if defined(PLATFORM_WIN32)
	SendMessage(hToolBar, TB_SETSTATE, IDM_SEARCH_PREV, MAKELONG(TBSTATE_ENABLED, 0)); 
	SendMessage(hToolBar, TB_SETSTATE, IDM_SEARCH_NEXT, MAKELONG(TBSTATE_ENABLED, 0)); 
#endif
#if defined(PLATFORM_HPC)
	SendMessage(GetCommandBar(hMSCmdBar, ID_BUTTONBAND), TB_SETSTATE, IDM_SEARCH_PREV, MAKELONG(TBSTATE_ENABLED, 0)); 
	SendMessage(GetCommandBar(hMSCmdBar, ID_BUTTONBAND), TB_SETSTATE, IDM_SEARCH_NEXT, MAKELONG(TBSTATE_ENABLED, 0)); 
#endif
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_PSPC)
	SendMessage(hMSCmdBar, TB_SETSTATE, IDM_SEARCH_PREV, MAKELONG(TBSTATE_ENABLED, 0)); 
	SendMessage(hMSCmdBar, TB_SETSTATE, IDM_SEARCH_NEXT, MAKELONG(TBSTATE_ENABLED, 0)); 
	SendMessage(hMDCmdBar, TB_SETSTATE, IDM_SEARCH_PREV, MAKELONG(TBSTATE_ENABLED, 0)); 
	SendMessage(hMDCmdBar, TB_SETSTATE, IDM_SEARCH_NEXT, MAKELONG(TBSTATE_ENABLED, 0)); 
#endif

	bSearchStartFromTreeView = bSelectViewActive;

	// 検索実行
	if (bSelectViewActive) {
		DoSearchTree(TRUE, !sd.IsSearchDirectionUp());
		mmMemoManager.SetMSSearchFlg(FALSE);
	} else {
		mdView.Search(TRUE, TRUE, TRUE, FALSE);
		mmMemoManager.SetMDSearchFlg(FALSE);
	}
}

void MainFrame::DoSearchTree(BOOL bFirst, BOOL bForward)
{

	SearchEngineA *pSE = mmMemoManager.GetSearchEngine();

	TString sPath, sFullPath;
	// Get path of selecting on treeview.
	TreeViewItem *pItem = msView.GetCurrentItem();
	if (pItem->HasMultiItem()) {
		// folder
		if (!msView.GetPathForNewItem(&sPath)) return;
	} else {
		// file
		if (!sPath.Set(((TreeViewFileItem *)pItem)->GetNote()->MemoPath())) return;
	}
	if (!sFullPath.Join(g_Property.TopDir(), TEXT("\\"), sPath.Get())) return;

	// Create dialog and do search.
	SearchTree st;
	st.Init(pSE, sFullPath.Get(), _tcslen(g_Property.TopDir()), bForward, !bFirst);
	st.Popup(g_hInstance, hMainWnd);

	TCHAR buf[1024];

	switch(st.GetResult()) {
	case SR_FOUND:
		msView.ShowItem(st.GetPartPath());
		mmMemoManager.SearchDetailsView(TRUE, TRUE, TRUE, TRUE);
		break;
	case SR_NOTFOUND:
		MessageBox(MSG_STRING_NOT_FOUND, TOMBO_APP_NAME, MB_OK | MB_ICONINFORMATION);
		break;
	case SR_CANCELED:
		msView.ShowItem(st.GetPartPath());
		MessageBox(MSG_STRING_SEARCH_CANCELED, TOMBO_APP_NAME, MB_OK | MB_ICONINFORMATION);
		break;
	case SR_FAILED:
		wsprintf(buf, MSG_SEARCH_FAILED, GetLastError());
		msView.ShowItem(st.GetPartPath());
		MessageBox(buf, TOMBO_APP_NAME, MB_OK | MB_ICONERROR);
		break;
	}
}

///////////////////////////////////////////////////
// 次の一致項目を検索
///////////////////////////////////////////////////

void MainFrame::OnSearchNext(BOOL bForward)
{
	if (mmMemoManager.GetSearchEngine() == NULL) return;

	if (bSelectViewActive) {
		DoSearchTree(mmMemoManager.MSSearchFlg(), bForward);
		mmMemoManager.SetMSSearchFlg(FALSE);
	} else {
		// if search starts at edit view, show message when match failed.
		// if starts at tree view, search next item.
		BOOL bMatched = mdView.Search(mmMemoManager.MDSearchFlg(), bForward, !bSearchStartFromTreeView, FALSE);
		mmMemoManager.SetMDSearchFlg(FALSE);
		if (bSearchStartFromTreeView && !bMatched) {
			ActivateView(TRUE);
			DoSearchTree(mmMemoManager.MSSearchFlg(), bForward);
			mmMemoManager.SetMSSearchFlg(FALSE);
		}
	}
}

///////////////////////////////////////////////////
// show/hide status bar
///////////////////////////////////////////////////

#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
void MainFrame::ToggleShowStatusBar()
{
	g_Property.ToggleShowStatusBar();

#if defined(PLATFORM_WIN32)
	HMENU hMenu = GetMenu(hMainWnd);
#else
	HMENU hMenu = CommandBar_GetMenu(GetCommandBar(hMSCmdBar, ID_CMDBAR_MAIN), 0);
#endif

	if (g_Property.HideStatusBar()) {
		CheckMenuItem(hMenu, IDM_SHOWSTATUSBAR, MF_BYCOMMAND | MF_UNCHECKED);
	} else {
		CheckMenuItem(hMenu, IDM_SHOWSTATUSBAR, MF_BYCOMMAND | MF_CHECKED);
	}

	RECT r;
	GetClientRect(hMainWnd, &r);
	OnResize(0, MAKELPARAM(r.right - r.left, r.bottom - r.top));
}
#endif

///////////////////////////////////////////////////
// コマンドバンドからIDでコマンドバーを取得
///////////////////////////////////////////////////

void MainFrame::EnableSaveButton(BOOL bEnable)
{
	WORD nStat;
	if (bEnable) {
		nStat = TBSTATE_ENABLED;
	} else {
		nStat = 0;
	}
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	EnableMenu(IDM_SAVE, bEnable);
	SendMessage(GetMainToolBar(), TB_SETSTATE, IDM_SAVE, MAKELONG(nStat, 0)); 
#endif

#if defined(PLATFORM_PKTPC) || defined(PLATFORM_PSPC)
	SendMessage(hMDCmdBar, TB_ENABLEBUTTON, IDM_SAVE, MAKELONG(nStat, 0)); 
#endif
#if defined(PLATFORM_BE500)
	if (bEnable) {
		CSOBar_SetButtonState(hMDCmdBar, TRUE, IDM_SAVE, 1, CSO_BUTTON_DISP);
	} else {
		CSOBar_SetButtonState(hMDCmdBar, TRUE, IDM_SAVE, 1, CSO_BUTTON_GRAYED);
	}
#endif
}

///////////////////////////////////////////////////
// コマンドバンドからIDでコマンドバーを取得
///////////////////////////////////////////////////
#if defined(PLATFORM_HPC)
static HWND GetCommandBar(HWND hBand, UINT uBandID)
{
	UINT idx = SendMessage(hBand, RB_IDTOINDEX, uBandID, 0);
	if (idx == -1) return NULL;
	HWND hwnd = CommandBands_GetCommandBar(hBand, idx);
	return hwnd;
}
HMENU MainFrame::GetMainMenu() 
{ 
	return CommandBar_GetMenu(GetCommandBar(hMSCmdBar, ID_CMDBAR_MAIN), 0); 
}
HWND MainFrame::GetMainToolBar() 
{ 
	return GetCommandBar(hMSCmdBar, ID_BUTTONBAND); 
}
#endif

int MainFrame::MessageBox(LPCTSTR pText, LPCTSTR pCaption, UINT uType)
{
	return TomboMessageBox(hMainWnd, pText, pCaption, uType);
}

///////////////////////////////////////////////////
// Grep
///////////////////////////////////////////////////

void MainFrame::OnGrep()
{
	HTREEITEM hItem;
	TString sPath;
	hItem = msView.GetPathForNewItem(&sPath);
	if (hItem == NULL) return;

	GrepDialog gd;
	if (!gd.Init(sPath.Get())) return;
	if (gd.Popup(hInstance, hMainWnd) == IDOK) {
		if (!msView.InsertVirtualFolder(&gd)) {
			MessageBox(MSG_INSERTVFOLDER_FAIL, TOMBO_APP_NAME, MB_OK | MB_ICONERROR);
		}
	}
}

///////////////////////////////////////////////////
// stay topmost of the screen
///////////////////////////////////////////////////

void MainFrame::SetTopMost()
{
#if defined(PLATFORM_WIN32)
	HMENU hMenu = GetMenu(hMainWnd);

	if (g_Property.StayTopMost()) {
		CheckMenuItem(hMenu, IDM_TOPMOST, MF_BYCOMMAND | MF_CHECKED);
		SendMessage(hToolBar, TB_SETSTATE, IDM_TOPMOST, MAKELONG(TBSTATE_ENABLED |TBSTATE_PRESSED, 0)); 

		SetWindowPos(hMainWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	} else {
		CheckMenuItem(hMenu, IDM_TOPMOST, MF_BYCOMMAND | MF_UNCHECKED);
		SendMessage(hToolBar, TB_SETSTATE, IDM_TOPMOST, MAKELONG(TBSTATE_ENABLED, 0)); 

		SetWindowPos(hMainWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
#endif
}