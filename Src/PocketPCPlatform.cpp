#if defined(PLATFORM_PKTPC)

#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include <aygshell.h>

#include "resource.h"
#include "Message.h"
#include "PlatformLayer.h"
#include "PocketPCPlatform.h"

#define NUM_TOOLBAR_BMP 12

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

void PocketPCPlatform::Create(HWND hWnd, HINSTANCE hInst)
{
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

	SendMessage(hMSCmdBar, TB_SETTOOLTIPS, (WPARAM)NUM_MS_TOOLTIP, (LPARAM)pMSToolTip);

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

void PocketPCPlatform::OpenDetailsView()
{
	ShowWindow(hMDCmdBar, SW_SHOW);
	ShowWindow(hMSCmdBar, SW_HIDE);
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
	RECT rMenuRect;
	GetWindowRect(hMSCmdBar, &rMenuRect);
	DWORD nHOffset = rMenuRect.bottom - rMenuRect.top;

	r->bottom -= nHOffset - 1;
}

#endif // PLATFORM_PKTPC