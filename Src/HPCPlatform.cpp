#if defined(PLATFORM_HPC)
#include <windows.h>
#include <commctrl.h>

#include "resource.h"
#include "Property.h"

#include "PlatformLayer.h"
#include "HPCPlatform.h"

#define NUM_MY_TOOLBAR_BMPS 0
#define NUM_IMG_BUTTONS 10

#define NUM_CMDBAR_BUTTONS 19
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

static HWND GetCommandBar(HWND hBand, UINT uBandID)
{
	UINT idx = SendMessage(hBand, RB_IDTOINDEX, uBandID, 0);
	if (idx == -1) return NULL;
	HWND hwnd = CommandBands_GetCommandBar(hBand, idx);
	return hwnd;
}

void HPCPlatform::Create(HWND hWnd, HINSTANCE hInst)
{
	HWND hBand, hwnd;
	REBARBANDINFO arbbi[2];

	// CommandBand生成
	HIMAGELIST himl = ImageList_Create(16,16,ILC_COLOR, 0, 1);
	HBITMAP hBmp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_REBAR));
	ImageList_Add(himl, hBmp, (HBITMAP)NULL);
	DeleteObject(hBmp);

	hBand = CommandBands_Create(hInst, hWnd, IDC_CMDBAND,
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

	CommandBands_AddBands(hBand, hInst, 2, arbbi);
	// 0番目のバンド(メニュー)の設定
	hwnd = GetCommandBar(hBand, ID_CMDBAR_MAIN);
	CommandBar_InsertMenubar(hwnd, hInst, IDR_MENU_MAIN, 0);

	// 1番目のバンド(ボタン)の設定
	hwnd = GetCommandBar(hBand, ID_BUTTONBAND);

	CommandBar_AddBitmap(hwnd, HINST_COMMCTRL, IDB_STD_SMALL_COLOR, 15, 0, 0);
	CommandBar_AddBitmap(hwnd, hInst, IDB_TOOLBAR, NUM_IMG_BUTTONS, 0, 0);

	CommandBar_AddButtons(hwnd, sizeof(aCmdBarButtons)/sizeof(TBBUTTON), aCmdBarButtons);

	hMSCmdBar = hBand;
	CommandBands_AddAdornments(hBand, hInst, 0, NULL);

	if (bRestoreFlg) {
		if (cbri[0].fMaximized) {
			SendMessage(hBand, RB_MAXIMIZEBAND, 0, (LPARAM)0);
		}
		if (cbri[1].fMaximized) {
			SendMessage(hBand, RB_MAXIMIZEBAND, 1, (LPARAM)0);
		}
	}

}

HWND HPCPlatform::GetMainToolBar()
{
	return GetCommandBar(hMSCmdBar, ID_BUTTONBAND);
}

HMENU HPCPlatform::GetMainMenu() 
{ 
	return CommandBar_GetMenu(GetCommandBar(hMSCmdBar, ID_CMDBAR_MAIN), 0); 
}

HMENU HPCPlatform::GetMDToolMenu()
{
	return CommandBar_GetMenu(GetCommandBar(hMSCmdBar, ID_CMDBAR_MAIN), 0);

}

HMENU HPCPlatform::GetMSEditMenu()
{
	return CommandBar_GetMenu(GetCommandBar(hMSCmdBar, ID_CMDBAR_MAIN), 0);
}

HMENU HPCPlatform::GetMSBookMarkMenu()
{
	HMENU hMenu = GetMainMenu();
	return GetSubMenu(hMenu, BOOKMARK_MENU_POS);
}

void HPCPlatform::EnableMenu(UINT uId, BOOL bEnable)
{
	BOOL bTb = FALSE;
	HWND hTbWin;
	switch(uId) {
	case IDM_DELETEITEM:
	case IDM_NEWMEMO:
	case IDM_CUT:
	case IDM_COPY:
	case IDM_PASTE:
	case IDM_SAVE:
		bTb = TRUE;
		hTbWin = GetMainToolBar();
	default:
		break;
	}


	HMENU hMenu = GetMainMenu();
	UINT uFlg1;
	if (bEnable) {
		uFlg1 = MF_BYCOMMAND | MF_ENABLED;
	} else {
		uFlg1 = MF_BYCOMMAND | MF_GRAYED;
	}
	EnableMenuItem(hMenu, uId, uFlg1);

	if (bTb) {
		SendMessage(hTbWin, TB_ENABLEBUTTON, uId, MAKELONG(bEnable, 0));
	}
}

void HPCPlatform::ControlMenu(BOOL bTreeActive)
{
	HMENU hMenu = CommandBar_GetMenu(GetCommandBar(hMSCmdBar, ID_CMDBAR_MAIN), 0);

	UINT uFlg1, uFlg2;
	if (bTreeActive) {
		uFlg1 = MF_BYCOMMAND | MF_ENABLED;
		uFlg2 = MF_BYCOMMAND | MF_GRAYED;
	} else {
		uFlg1 = MF_BYCOMMAND | MF_GRAYED;
		uFlg2 = MF_BYCOMMAND | MF_ENABLED;
	}

	EnableMenuItem(hMenu, IDM_FORGETPASS, uFlg1);
	EnableMenuItem(hMenu, IDM_PROPERTY, uFlg1);

	EnableMenuItem(hMenu, IDM_INSDATE1, uFlg2);
	EnableMenuItem(hMenu, IDM_INSDATE2, uFlg2);
	EnableMenuItem(hMenu, IDM_DETAILS_HSCROLL, uFlg2);
}

void HPCPlatform::ControlToolbar(BOOL bTreeActive)
{
	HWND hToolBar = GetCommandBar(hMSCmdBar, ID_BUTTONBAND);
	SendMessage(hToolBar, TB_ENABLEBUTTON, IDM_INSDATE1, MAKELONG(!bTreeActive, 0));
	SendMessage(hToolBar, TB_ENABLEBUTTON, IDM_INSDATE2, MAKELONG(!bTreeActive, 0));
}

void HPCPlatform::OpenDetailsView()
{
	ControlMenu(FALSE);
	ControlToolbar(FALSE);
}

void HPCPlatform::CloseDetailsView()
{
	ControlMenu(TRUE);
	ControlToolbar(TRUE);
}

void HPCPlatform::EnableSearchNext()
{
	SendMessage(GetCommandBar(hMSCmdBar, ID_BUTTONBAND), TB_SETSTATE, IDM_SEARCH_PREV, MAKELONG(TBSTATE_ENABLED, 0)); 
	SendMessage(GetCommandBar(hMSCmdBar, ID_BUTTONBAND), TB_SETSTATE, IDM_SEARCH_NEXT, MAKELONG(TBSTATE_ENABLED, 0)); 
}

void HPCPlatform::AdjustUserRect(RECT *r)
{
	DWORD nHOffset = CommandBands_Height(hMSCmdBar);
	r->top += nHOffset;
	r->bottom -= nHOffset;
}

void HPCPlatform::CheckMenu(UINT uid, BOOL bCheck)
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
	SendMessage(GetCommandBar(hMSCmdBar, ID_BUTTONBAND), TB_PRESSBUTTON, uid, MAKELONG(bButton, 0));
}

#endif // PLATFORM_HPC
