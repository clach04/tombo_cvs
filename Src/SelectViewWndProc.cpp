#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

#if defined(_WIN32_WCE) && defined(PLATFORM_PKTPC)
#include <aygshell.h>
#endif
#include "resource.h"
#include "Property.h"
#include "MemoSelectView.h"

#define KEY_CTRL_C 3
#define KEY_CTRL_V 22
#define KEY_CTRL_X 24

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
	case WM_CHAR:
		{
			// ignore events for disabling click beeps when focusing.
			if (!pView) return 0;
			HTREEITEM hItem;
			TreeViewItem *pItem = pView->GetCurrentItem(&hItem);
			switch(wParam) {
			case KEY_CTRL_C:
				pView->OnCopy(pItem);
				break;
			case KEY_CTRL_X:
				pView->OnCut(pItem);
				break;
			case KEY_CTRL_V:
				pView->OnPaste();
				break;
			}
		return 0;
		}
	case WM_KEYDOWN:
		{
			BOOL bShiftDown = (GetKeyState(VK_SHIFT) & 0x8000) != 0;

			if (wParam == VK_DELETE) {
				SendMessage(hParentWnd, WM_COMMAND, IDM_DELETEITEM, 0);
				return 0;
			}
			if (wParam == VK_LEFT && bShiftDown) {
				pView->SelUpFolderWithoutOpen();
				return 0;
			}
			if (wParam == VK_UP && bShiftDown) {
				pView->SelPrevBrother();
				return 0;
			}
			if (wParam == VK_DOWN && bShiftDown) {
				pView->SelNextBrother();
				return 0;
			}
		}
		break;
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