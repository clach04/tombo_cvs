#if defined(PLATFORM_WIN32)
#include <windows.h>
#include <commctrl.h>

#include "resource.h"
#include "PlatformLayer.h"
#include "Win32Platform.h"
#include "StatusBar.h"
#include "Property.h"

#define NUM_MY_TOOLBAR_BMPS 12

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

Win32Platform::Win32Platform() : pStatusBar(NULL)
{
}

Win32Platform::~Win32Platform()
{
	delete pStatusBar;
}

static HWND CreateToolBar(HWND hParent, HINSTANCE hInst)
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

void Win32Platform::Create(HWND hWnd, HINSTANCE hInst)
{
	pStatusBar = new StatusBar();
	pStatusBar->Create(hWnd, g_Property.IsUseTwoPane());

	 hRebar = CreateWindowEx(WS_EX_TOOLWINDOW, REBARCLASSNAME, NULL,
								WS_BORDER | RBS_BANDBORDERS | RBS_AUTOSIZE | 
								WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS| RBS_TOOLTIPS | 
								WS_CLIPCHILDREN|RBS_VARHEIGHT,
								0, 0, 0, 0,
								hWnd, NULL, hInst, NULL);
	REBARINFO rbi;
	rbi.cbSize = sizeof(rbi);
	rbi.fMask = 0;
	rbi.himl = NULL;
	SendMessage(hRebar, RB_SETBARINFO, 0, (LPARAM)&rbi);

	hToolBar = CreateToolBar(hRebar, hInst);

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
}

void Win32Platform::EnableMenu(UINT uId, BOOL bEnable)
{
	BOOL bTB = FALSE;
	HWND hTB;

	switch (uId) {
	case IDM_DELETEITEM:
		Sleep(1);
	case IDM_NEWMEMO:
	case IDM_CUT:
	case IDM_COPY:
	case IDM_PASTE:
	case IDM_SAVE:
		bTB = TRUE;
		hTB = GetMainToolBar();
	default:
		break;
	}

	HMENU hMenu = GetMainMenu();

	UINT uFlg1;
	if (bEnable) {
		uFlg1 = MF_BYCOMMAND | MF_ENABLED;
	} else {
		uFlg1 = MF_BYCOMMAND | MF_GRAYED | MF_DISABLED;
	}
	EnableMenuItem(hMenu, uId, uFlg1);

	if (bTB) {
		int x = SendMessage(hTB, TB_ENABLEBUTTON, uId, MAKELONG(bEnable, 0));
	}
}

void Win32Platform::ControlMenu(BOOL bTreeActive)
{
	HMENU hMenu = GetMainMenu();

	UINT uDisableFlg = MF_BYCOMMAND | MF_GRAYED;
#if defined(PLATFORM_WIN32)
	uDisableFlg |= MF_DISABLED;
#endif

	UINT uFlg1, uFlg2;
	if (bTreeActive) {
		uFlg1 = MF_BYCOMMAND | MF_ENABLED;
		uFlg2 = uDisableFlg;
	} else {
		uFlg1 = uDisableFlg;
		uFlg2 = MF_BYCOMMAND | MF_ENABLED;
	}

	EnableMenuItem(hMenu, IDM_FORGETPASS, uFlg1);
	EnableMenuItem(hMenu, IDM_PROPERTY, uFlg1);

	EnableMenuItem(hMenu, IDM_INSDATE1, uFlg2);
	EnableMenuItem(hMenu, IDM_INSDATE2, uFlg2);
	EnableMenuItem(hMenu, IDM_DETAILS_HSCROLL, uFlg2);
}

void Win32Platform::ControlToolbar(BOOL bTreeActive)
{
	SendMessage(hToolBar, TB_ENABLEBUTTON, IDM_INSDATE1, MAKELONG(!bTreeActive, 0));
	SendMessage(hToolBar, TB_ENABLEBUTTON, IDM_INSDATE2, MAKELONG(!bTreeActive, 0));
}

void Win32Platform::OpenDetailsView()
{
	ControlMenu(FALSE);
	ControlToolbar(FALSE);
}

void Win32Platform::CloseDetailsView()
{
	ControlMenu(TRUE);
	ControlToolbar(TRUE);
}

void Win32Platform::EnableSearchNext()
{
	SendMessage(hToolBar, TB_SETSTATE, IDM_SEARCH_PREV, MAKELONG(TBSTATE_ENABLED, 0)); 
	SendMessage(hToolBar, TB_SETSTATE, IDM_SEARCH_NEXT, MAKELONG(TBSTATE_ENABLED, 0)); 
}

void Win32Platform::AdjustUserRect(RECT *pRect)
{
	// get rebar height
	RECT r;
	GetWindowRect(hRebar, &r);
	WORD nRebarH = (WORD)(r.bottom - r.top);

	// get statusbar height
	WORD nStatusHeight;
	if (g_Property.HideStatusBar()) {
		nStatusHeight = 0;
	} else {
		nStatusHeight = GetStatusBarHeight();
	}

	pRect->top = nRebarH;
	pRect->bottom -= (nRebarH + nStatusHeight);

}

void Win32Platform::CheckMenu(UINT uid, BOOL bCheck)
{
	BOOL bNegButton = FALSE;
	switch(uid) {
	case IDM_TOGGLEPANE:
		bNegButton = TRUE;
		break;
	default:
		break;
	}

	BOOL bButton = bNegButton ? !bCheck : bCheck;

	HMENU hMenu = GetMainMenu();
	// CheckMenuItem is superseeded funcs, but in CE, SetMenuItemInfo can't set values, so use it.
	CheckMenuItem(hMenu, uid, MF_BYCOMMAND | (bCheck ? MF_CHECKED : MF_UNCHECKED));
	SendMessage(hToolBar, TB_PRESSBUTTON, uid, MAKELONG(bButton, 0));
}

void Win32Platform::ShowStatusBar(BOOL bShow)
{
	pStatusBar->Show(bShow);
}

void Win32Platform::SetStatusIndicator(DWORD nPos, LPCTSTR pText, BOOL bDisp)
{
	pStatusBar->SetStatusIndicator(nPos, pText, bDisp);
}

WORD Win32Platform::GetStatusBarHeight()
{
	return pStatusBar->GetHeight();
}

void Win32Platform::ResizeStatusBar(WPARAM wParam, LPARAM lParam)
{
	SendMessage(hRebar, WM_SIZE, wParam, lParam);

	pStatusBar->SendSize(wParam, lParam);
	pStatusBar->ResizeStatusBar();
}

void Win32Platform::GetStatusWindowRect(RECT *pRect)
{
	pStatusBar->GetWindowRect(pRect);
}

#endif // PLATFORM_WIN32