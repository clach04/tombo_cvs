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
#include "PasswordManager.h"

//////////////////////////////////////
// Global variables
//////////////////////////////////////

Property g_Property;
HINSTANCE g_hInstance;
Logger g_Logger;
Logger *g_pLogger;

BOOL bDisableHotKey;

PasswordManager *g_pPasswordManager = NULL;

//////////////////////////////////////
// Declarations
//////////////////////////////////////

BOOL CheckAndRaiseAnotherTombo();
BOOL ParseCmdLine(LPTSTR pCmdLine);

extern "C" {
	const char *CheckBlowFish();
};

//////////////////////////////////////
// WinMain
//////////////////////////////////////
#ifndef UNIT_TEST
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR pCmdLine, int nCmdShow)
{
	// Check other Tombo.exe is executed
	if (CheckAndRaiseAnotherTombo()) {
		return 0;
	}

	// Check BLOWFISH library
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

	// Check command line strings
	ParseCmdLine(pCmdLine);

	// initialize logger
	g_pLogger = &g_Logger;
	// if write debug log, comment out two lines:
//	g_Logger.Init(TEXT("\\My Documents\\Tombo.log"));
//	TomboMessageBox(NULL, TEXT("Log mode is ON"), TEXT("DEBUG"), MB_OK);

	bDisableHotKey = FALSE;

#if defined(PLATFORM_PSPC) || defined(PLATFORM_PKTPC) || defined(PLATFORM_BE500)
	InitCommonControls();
#endif
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	// initialize rebar control
   INITCOMMONCONTROLSEX icex;
   icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
   icex.dwICC   = ICC_COOL_CLASSES|ICC_BAR_CLASSES;
   InitCommonControlsEx(&icex);
#endif

#if defined(PLATFORM_PKTPC)
	SHInitExtraControls();
#endif

	// create MainFrame instance
	MainFrame frmMain;
	MainFrame::RegisterClass(hInst);

	g_hInstance = hInst;
	frmMain.Create(TOMBO_APP_NAME, hInst, nCmdShow);

	// go message loop
	int res = frmMain.MainLoop();

	g_Logger.Close();
	return res;
}
#endif

//////////////////////////////////////
// Mutial execute check function
//////////////////////////////////////

static BOOL CALLBACK EnumProc(HWND hWnd, LPARAM lParam)
{
	TCHAR buf[1024];
	GetClassName(hWnd, buf, 1024);
	if (_tcscmp(buf, TOMBO_MAIN_FRAME_WINDOW_CLSS) == 0) {

		// send message to elder instanse
		SendMessage(hWnd, MWM_RAISE_MAINFRAME, 0, 0);
		*(BOOL*)lParam = TRUE;
	}
	return TRUE;
}

//////////////////////////////////////
// ��d�N���`�F�b�N
//////////////////////////////////////
//
// �E�B���h�E��񋓂���Tomobo�̓�d�N�����`�F�b�N�A
// ��d�N�����Ă���悤�ł���΂����Raise���ďI������B
static BOOL CheckAndRaiseAnotherTombo()
{
	BOOL bExist = FALSE;
	EnumWindows((WNDENUMPROC)EnumProc, (WPARAM)&bExist);
	return bExist;
}

//////////////////////////////////////
// ���b�Z�[�W�{�b�N�X
//////////////////////////////////////
//
// HotKey��Disable�����

int TomboMessageBox(HWND hWnd, LPCTSTR pText, LPCTSTR pCaption, UINT uType) 
{
	BOOL bPrev = bDisableHotKey;
	bDisableHotKey = TRUE;
	int nResult = MessageBox(hWnd, pText, pCaption, uType);
	bDisableHotKey = bPrev;
	return nResult;
}

//////////////////////////////////////
// Parse command line string
//////////////////////////////////////
BOOL GetDefaultFolder(LPTSTR pCmdLine, LPDWORD pStart, LPDWORD pEnd, LPTSTR *ppNext);

BOOL ParseCmdLine(LPTSTR pCmdLine)
{
	LPTSTR p = pCmdLine;
	while (*p) {
		if (*p == TEXT('-')) {
			if (_tcsnicmp(p+1, TEXT("root="), 5) == 0) {
				DWORD nStart, nEnd;
				LPTSTR pNext;
				if (GetDefaultFolder(p + 6, &nStart, &nEnd, &pNext)) {
					g_Property.SetDefaultTomboRoot(p + 6 + nStart, nEnd - nStart);
				}
				p = pNext;
//			} else if (_tcsnicmp(p+1, TEXT("ro"), 2) == 0) {
//				g_Property.SetDefaultROMode(TRUE);
			}
		}
		p = CharNext(p);
	}
	return TRUE;
}

BOOL GetDefaultFolder(LPTSTR pCmdLine, LPDWORD pStart, LPDWORD pEnd, LPTSTR *ppNext)
{
	LPTSTR p = pCmdLine;
	BOOL bQuoted = FALSE;

	// Is quoted?
	if (*p == TEXT('"')) {
		bQuoted = TRUE;
		p++;
	}
	*pStart = p - pCmdLine;

	BOOL bBreak = FALSE;
	while(*p) {
		if (!bQuoted && *p == TEXT(' ')) {
			bBreak = TRUE;
			break;
		}
		if (bQuoted && *p == TEXT('"')) {
			bBreak = TRUE;
			break;
		}
		p = CharNext(p);
	}
	if (bQuoted) {
		if (*p != TEXT('"')) {
			*ppNext = p; // may be EOL
			return FALSE;
		}
		*ppNext = p + 1;
		*pEnd = p - pCmdLine;
		return TRUE;
	} else {
		*ppNext = p;
		*pEnd = p - pCmdLine;
		return TRUE;
	}


	return TRUE;
}