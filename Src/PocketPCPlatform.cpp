#if defined(PLATFORM_PKTPC)

#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include <aygshell.h>

#include "resource.h"
#include "Message.h"
#include "PlatformLayer.h"
#include "PocketPCPlatform.h"
#include "SipControl.h"
#include "Property.h"

#define NUM_TOOLBAR_BMP 12

#define NUM_MS_TOOLTIP 1
LPTSTR pMSToolTip[] = {
	NULL, // MSG_TOOLTIPS_NEWMEMO,
};

#define NUM_MD_TOOLTIP 6
LPTSTR pMDToolTip[] = {
	NULL, // MSG_TOOLTIPS_RETURNLIST,
	NULL, //MSG_TOOLTIPS_SAVE,
	TEXT(""),
	TEXT(""),
	NULL, // MSG_TOOLTIPS_INSDATE1,
	NULL, // MSG_TOOLTIPS_INSDATE2,
};

void PocketPCPlatform::Create(HWND hWnd, HINSTANCE hInst)
{
	// menubar for Tree view
	SHMENUBARINFO mbi;
	memset(&mbi, 0, sizeof(SHMENUBARINFO));
	mbi.cbSize = sizeof(SHMENUBARINFO);
	mbi.hwndParent = hWnd;
	mbi.nToolBarId = IDM_MAIN_MENU;
	mbi.hInstRes =hInst;
	mbi.nBmpId = IDB_TOOLBAR;
	mbi.cBmpImages = NUM_TOOLBAR_BMP;
	if (!SHCreateMenuBar(&mbi)) {
		MessageBox(hWnd, TEXT("SHCreateMenuBar failed."), TEXT("DEBUG"), MB_OK);
	}
	hMSCmdBar = mbi.hwndMB;

	TBBUTTONINFO tbi;
	memset(&tbi, 0, sizeof(&tbi));
	tbi.cbSize = sizeof(tbi);
	tbi.dwMask = TBIF_TEXT;

	tbi.pszText = (LPTSTR)MSG_MENU_NOTE;
	tbi.cchText = _tcslen(MSG_MENU_NOTE);
	SendMessage(hMSCmdBar, TB_SETBUTTONINFO, (WPARAM)(INT)IDM_EDIT_MEMO, (LPARAM)&tbi);

	tbi.pszText = (LPTSTR)MSG_MENU_TOOL;
	tbi.cchText = _tcslen(MSG_MENU_TOOL);
	SendMessage(hMSCmdBar, TB_SETBUTTONINFO, (WPARAM)(INT)IDM_MENUITEM3, (LPARAM)&tbi);

	pMSToolTip[0] = (LPTSTR)MSG_TOOLTIPS_NEWMEMO;
	SendMessage(hMSCmdBar, TB_SETTOOLTIPS, (WPARAM)NUM_MS_TOOLTIP, (LPARAM)pMSToolTip);

	////////////////////--
//	HMENU hM = SHGetSubMenu(hMSCmdBar, IDM_EDIT_MEMO);

//	InsertMenu(hM, 0, MF_BYPOSITION | MF_STRING, IDM_EXIT, TEXT("EXIT"));

	////////////////////--

	// menubar for edit view
	memset(&mbi, 0, sizeof(SHMENUBARINFO));
	mbi.cbSize = sizeof(SHMENUBARINFO);
	mbi.hwndParent = hWnd;
	mbi.nToolBarId = IDM_DETAILS_MENU;
	mbi.hInstRes =hInst;
	mbi.nBmpId = IDB_TOOLBAR;
	mbi.cBmpImages = NUM_TOOLBAR_BMP;
	if (!SHCreateMenuBar(&mbi)) {
		MessageBox(hWnd, TEXT("SHCreateMenuBar failed."), TEXT("DEBUG"), MB_OK);
	}
	hMDCmdBar = mbi.hwndMB;

	pMDToolTip[0] = (LPTSTR)MSG_TOOLTIPS_RETURNLIST;
	pMDToolTip[1] = (LPTSTR)MSG_TOOLTIPS_SAVE;
	pMDToolTip[4] = (LPTSTR)MSG_TOOLTIPS_INSDATE1;
	pMDToolTip[5] = (LPTSTR)MSG_TOOLTIPS_INSDATE2;
	SendMessage(hMDCmdBar, TB_SETTOOLTIPS, (WPARAM)NUM_MD_TOOLTIP, (LPARAM)pMDToolTip);

	ShowWindow(hMDCmdBar, SW_HIDE);
}

HMENU PocketPCPlatform::GetMDToolMenu() { return SHGetSubMenu(hMDCmdBar, IDM_DETAILS_TOOL); }
HMENU PocketPCPlatform::GetMSEditMenu() { return SHGetSubMenu(hMSCmdBar, IDM_EDIT_MEMO); }
HMENU PocketPCPlatform::GetMSBookMarkMenu() { return SHGetSubMenu(hMSCmdBar, IDM_MS_BOOKMARK); }

void PocketPCPlatform::EnableMenu(UINT uid, BOOL bEnable)
{
	BOOL bSBS = FALSE;
	BOOL bMenu = TRUE;
	HWND hTB;

	HMENU hMenu;
	switch (uid) {
	case IDM_DECRYPT:
	case IDM_ENCRYPT:
	case IDM_DELETEITEM:
	case IDM_RENAME:
	case IDM_CUT:
	case IDM_COPY:
	case IDM_PASTE:
	case IDM_NEWFOLDER:
	case IDM_GREP:
		hMenu = GetMSEditMenu();
		break;
	case IDM_NEWMEMO:
		bSBS = TRUE;
		hTB = hMSCmdBar;
		hMenu = GetMSEditMenu();
		break;
	case IDM_SAVE:
		bMenu = FALSE;
		bSBS = TRUE;
		hTB = hMDCmdBar;
		break;
	default:
		return;
	}

	if (bMenu) {
		if (bEnable) {
			EnableMenuItem(hMenu, uid, MF_BYCOMMAND | MF_ENABLED);
		} else {
			EnableMenuItem(hMenu, uid, MF_BYCOMMAND | MF_GRAYED);
		}
	}

	if (bSBS) {
		SendMessage(hTB, TB_ENABLEBUTTON, uid, MAKELONG(bEnable, 0));
	}
}

void PocketPCPlatform::CheckMenu(UINT uid, BOOL bCheck)
{
	HWND hTB;
	BOOL bCBS = FALSE;
	switch(uid) {
	case IDM_TOGGLEPANE:
		bCBS = TRUE;
		hTB = hMSCmdBar;
		bCheck = !bCheck;
		break;
	default:
		return;
	}

	if (bCBS) {
		SendMessage(hTB, TB_CHECKBUTTON, uid, MAKELONG(bCheck, 0));
	}
}

void PocketPCPlatform::OpenDetailsView()
{
	ShowWindow(hMDCmdBar, SW_SHOW);
	ShowWindow(hMSCmdBar, SW_HIDE);

	TBBUTTONINFO tbi;
	memset(&tbi, 0, sizeof(&tbi));
	tbi.cbSize = sizeof(tbi);
	tbi.dwMask = TBIF_TEXT;

	tbi.pszText = (LPTSTR)MSG_MENU_TOOL;
	tbi.cchText = _tcslen(MSG_MENU_TOOL);
	SendMessage(hMDCmdBar, TB_SETBUTTONINFO, (WPARAM)(INT)IDM_DETAILS_TOOL, (LPARAM)&tbi);

	tbi.pszText = (LPTSTR)MSG_MENU_EDIT;
	tbi.cchText = _tcslen(MSG_MENU_EDIT);
	SendMessage(hMDCmdBar, TB_SETBUTTONINFO, (WPARAM)(INT)IDM_EDIT_MEMO, (LPARAM)&tbi);

}

void PocketPCPlatform::CloseDetailsView()
{
	ShowWindow(hMDCmdBar, SW_HIDE);
	ShowWindow(hMSCmdBar, SW_SHOW);
}

void PocketPCPlatform::EnableSearchNext()
{
	SendMessage(hMSCmdBar, TB_SETSTATE, IDM_SEARCH_PREV, MAKELONG(TBSTATE_ENABLED, 0)); 
	SendMessage(hMSCmdBar, TB_SETSTATE, IDM_SEARCH_NEXT, MAKELONG(TBSTATE_ENABLED, 0)); 
	SendMessage(hMDCmdBar, TB_SETSTATE, IDM_SEARCH_PREV, MAKELONG(TBSTATE_ENABLED, 0)); 
	SendMessage(hMDCmdBar, TB_SETSTATE, IDM_SEARCH_NEXT, MAKELONG(TBSTATE_ENABLED, 0)); 
}

void PocketPCPlatform::AdjustUserRect(RECT *r)
{
	// menubar
	RECT rMenuRect;
	GetWindowRect(hMSCmdBar, &rMenuRect);
	DWORD nHOffset = rMenuRect.bottom - rMenuRect.top;

	r->bottom -= nHOffset - 1;

	// SIP
	BOOL bStat;
	SipControl sc;
	if (!sc.Init()) return;
	if (!sc.GetSipStat(&bStat)) return;

	if (bStat) {
		RECT rSip = sc.GetRect();
		DWORD nDelta = g_Property.SipSizeDelta();
	
		r->bottom -= (rSip.bottom - rSip.top + nDelta);
	}
}

#endif // PLATFORM_PKTPC