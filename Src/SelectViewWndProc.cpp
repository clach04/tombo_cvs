#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

#if defined(_WIN32_WCE) && defined(PLATFORM_PKTPC)
#include <aygshell.h>
#endif
#include "resource.h"
#include "Property.h"
#include "MemoSelectView.h"

extern "C" {
typedef LRESULT (CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
}

static WNDPROC gSuperProc;
static HINSTANCE hInst;
static HWND hParentWnd;
static MemoSelectView *pView;

void SelectViewSetWndProc(WNDPROC wp, HWND hParent, HINSTANCE h, MemoSelectView *p)
{
	gSuperProc = wp;
	hParentWnd = hParent;
	hInst = h;
	pView = p;
}

#if !defined(PLATFORM_PSPC) && !defined(PLATFORM_BE500)
LRESULT CALLBACK NewSelectViewProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	// Focus状態で一般のキーをたたいた際に音が出ないようにイベントを無視する
	case WM_CHAR:
		return 0;
//	case WM_KEYDOWN:
//		if (wParam == VK_DELETE) {
//			SendMessage(hParentWnd, WM_COMMAND, IDM_DELETEITEM, 0);
//			return 0;
//		}
//		break;
	case WM_SETFOCUS:
		{
			if (g_Property.IsUseTwoPane() && pView) {
				pView->OnGetFocus();
			}
			break;
		}
	}
    return CallWindowProc((WNDPROC)gSuperProc, hwnd, msg, wParam, lParam);
}
#endif