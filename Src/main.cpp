#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#if defined(PLATFORM_PKTPC)
#include <aygshell.h>
#endif

#include "Tombo.h"
#include "Message.h"
#include "UniConv.h"
#include "MainFrame.h"
#include "Property.h"
#include "Logger.h"

//////////////////////////////////////
// グローバル変数
//////////////////////////////////////

Property g_Property;
HINSTANCE g_hInstance;
Logger g_Logger;
Logger *g_pLogger;

BOOL bDisableHotKey;

//////////////////////////////////////
// プロトタイプ
//////////////////////////////////////

BOOL CheckAndRaiseAnotherTombo();

extern "C" {
	const char *CheckBlowFish();
};

//////////////////////////////////////
// WinMain
//////////////////////////////////////

#ifdef _WIN32_WCE
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR pCmdLine, int nCmdShow)
#else
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR pCmdLine, int nCmdShow)
#endif
{
	// 二重起動チェック
	if (CheckAndRaiseAnotherTombo()) {
		return 0;
	}

	const char *p = CheckBlowFish();
	if (p != NULL) {
		TCHAR buf[1024];
		LPTSTR pMsg = ConvSJIS2Unicode(p);
		if (pMsg) {
			wsprintf(buf, MSG_CHECKBF_FAILED, pMsg);
		} else {
			wsprintf(buf, MSG_CHECKBF_FAILED, TEXT("unknown"));
		}
		MessageBox(NULL, buf, MSG_CHECKBF_TTL, MB_ICONWARNING | MB_OK);
		delete [] pMsg;
	}

	// Loggerの初期化
	g_pLogger = &g_Logger;
	// ログ出力時には以下の2行をコメントアウトすること
//	g_Logger.Init(TEXT("\\My Documents\\Tombo.log"));
//	TomboMessageBox(NULL, TEXT("Log mode is ON"), TEXT("DEBUG"), MB_OK);

	bDisableHotKey = FALSE;

#if defined(PLATFORM_PSPC) || defined(PLATFORM_PKTPC) || defined(PLATFORM_BE500)
	InitCommonControls();
#endif
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	// Rebarの初期化
   INITCOMMONCONTROLSEX icex;
   icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
   icex.dwICC   = ICC_COOL_CLASSES|ICC_BAR_CLASSES;
   InitCommonControlsEx(&icex);
#endif

#if defined(PLATFORM_PKTPC)
	SHInitExtraControls();
#endif

	MainFrame frmMain;
	MainFrame::RegisterClass(hInst);


	g_hInstance = hInst;
	frmMain.Create(TOMBO_APP_NAME, hInst, nCmdShow);


	int res = frmMain.MainLoop();

	g_Logger.Close();
	return res;
}

//////////////////////////////////////
// 二重起動チェック列挙関数
//////////////////////////////////////
static BOOL CALLBACK EnumProc(HWND hWnd, LPARAM lParam)
{
	TCHAR buf[1024];
	GetClassName(hWnd, buf, 1024);
	if (_tcscmp(buf, TOMBO_MAIN_FRAME_WINDOW_CLSS) == 0) {
//		SetForegroundWindow(hWnd);
		SendMessage(hWnd, MWM_RAISE_MAINFRAME, 0, 0);
		*(BOOL*)lParam = TRUE;
	}
	return TRUE;
}

//////////////////////////////////////
// 二重起動チェック
//////////////////////////////////////
//
// ウィンドウを列挙してTomoboの二重起動をチェック、
// 二重起動しているようであればそれをRaiseして終了する。
static BOOL CheckAndRaiseAnotherTombo()
{
	BOOL bExist = FALSE;
	EnumWindows((WNDENUMPROC)EnumProc, (WPARAM)&bExist);
	return bExist;
}

//////////////////////////////////////
// メッセージボックス
//////////////////////////////////////
//
// HotKeyをDisableする版

int TomboMessageBox(HWND hWnd, LPCTSTR pText, LPCTSTR pCaption, UINT uType) 
{
	BOOL bPrev = bDisableHotKey;
	bDisableHotKey = TRUE;
	int nResult = MessageBox(hWnd, pText, pCaption, uType);
	bDisableHotKey = bPrev;
	return nResult;
}