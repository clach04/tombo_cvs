#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#if defined(PLATFORM_BE500)
#include <CSO.h>
#endif

#include "Tombo.h"
#include "Message.h"
#include "resource.h"

#include "PlatformLayer.h"

///////////////////////////////////////////////////
// ctor & dtor
///////////////////////////////////////////////////

PlatformLayer::PlatformLayer() : hMainWnd(NULL)
{
}

PlatformLayer::~PlatformLayer()
{
}

BOOL PlatformLayer::Init(HWND h)
{
	hMainWnd = h;
	return TRUE;
}

///////////////////////////////////////////////////
///////////////////////////////////////////////////
// helper functions
///////////////////////////////////////////////////
///////////////////////////////////////////////////

void SetAppIcon(HINSTANCE hInst, HWND hWnd)
{
#if defined(PLATFORM_WIN32)
	// set app icon
	HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_TOMBO));
	SendMessage(hWnd, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hIcon);
#endif
#if defined(PLATFORM_HPC)
	HICON hIcon =  (HICON)LoadImage(hInst,
                                MAKEINTRESOURCE(IDI_TOMBO),
                                IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	SendMessage(hWnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIcon);
#endif
}

///////////////////////////////////////////////////
// menu helper
///////////////////////////////////////////////////

void OverrideMenuTitle(HMENU hMenu, MenuMsgRes *pRes, int nNumRes)
{
	for (int i = 0; i < nNumRes; i++) {
		if (pRes[i].iMenuID == -1) {
			InsertMenu(hMenu,  pRes[i].iPos, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
		} else {
			InsertMenu(hMenu,  pRes[i].iPos, MF_BYPOSITION | MF_STRING | pRes[i].iExtOpt, pRes[i].iMenuID, g_mMsgRes.GetMsg(pRes[i].iMsgID));
		}
	}
}

