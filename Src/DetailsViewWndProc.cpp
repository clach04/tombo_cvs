#include <windows.h>
#include <tchar.h>
#if defined(_WIN32_WCE) && defined(PLATFORM_PKTPC)
#include <aygshell.h>
#endif
#include "resource.h"
#include "MemoDetailsView.h"
#include "MemoManager.h"

//#define PLATFORM_SIG3

#include "Property.h"

static MemoManager *pManager; 
static MemoDetailsView *pView;
static HINSTANCE hInst;
static HWND hParentWnd;
static SUPER_WND_PROC gSuperProc;

static BOOL bCtrlKeyDown = FALSE;

#define KEY_ESC 0x1B
#define KEY_CTRL_A 1
#define KEY_CTRL_B 2

#define KEY_COLON       0xBB
#define KEY_SEMICOLON   0xBA

void SetWndProc(SUPER_WND_PROC wp, HWND hParent, HINSTANCE h, MemoDetailsView *p, MemoManager *pMgr)
{
	gSuperProc = wp;
	hParentWnd = hParent;
	hInst = h;
	pView = p;
	pManager = pMgr;
}

/////////////////////////////////////////
// Window procedure for sub classing editview
/////////////////////////////////////////

#if defined(PLATFORM_SIG3)
INT nSelBase = -1;
#endif

LRESULT CALLBACK NewDetailsViewProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_CLEAR:
		if (pView->IsReadOnly()) return 0;
		break;
	case WM_CUT:
		if (pView->IsReadOnly()) return 0;
		break;
	case WM_PASTE:
		if (pView->IsReadOnly()) return 0;
		break;
	case WM_KEYDOWN:
#if defined(PLATFORM_SIG3)
		{
			BOOL bShiftDown = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
			if (bShiftDown && wParam == VK_UP) {
				INT nPrevStart, nPrevEnd;
				INT nAftStart, nAftEnd;
				SendMessage(hwnd, EM_GETSEL, (WPARAM)&nPrevStart, (LPARAM)&nPrevEnd);
				LRESULT lResult = CallWindowProc(gSuperProc, hwnd, msg, wParam, lParam);
				SendMessage(hwnd, EM_GETSEL, (WPARAM)&nAftStart, (LPARAM)&nAftEnd);

				if (nAftStart < nSelBase) {
					SendMessage(hwnd, EM_SETSEL, (WPARAM)nSelBase, (LPARAM)nAftStart);
					return lResult;
				} else {
					return lResult;
				}
			}
			if (bShiftDown && wParam == VK_DOWN) {
				INT nPrevStart, nPrevEnd;
				INT nAftStart, nAftEnd;
				SendMessage(hwnd, EM_GETSEL, (WPARAM)&nPrevStart, (LPARAM)&nPrevEnd);
				LRESULT lResult = CallWindowProc(gSuperProc, hwnd, msg, wParam, lParam);
				SendMessage(hwnd, EM_GETSEL, (WPARAM)&nAftStart, (LPARAM)&nAftEnd);

				if (nAftStart < nSelBase) {
					SendMessage(hwnd, EM_SETSEL, (WPARAM)nSelBase, (LPARAM)nAftEnd);
					return lResult;
				} else {
					return lResult;
				}

			}

			if (!(bShiftDown && wParam == VK_LEFT) && 
				!(bShiftDown && wParam == VK_RIGHT)) {
				POINT pt;
				GetCaretPos(&pt);
				LPARAM l = MAKELPARAM(pt.x, pt.y);
				nSelBase = SendMessage(hwnd, EM_CHARFROMPOS, 0, l) & 0xFFFF;
			}
		}
#endif

		if (pView->IsReadOnly()) {
			if (wParam == VK_DELETE) return 0;
			if (wParam == VK_BACK || wParam == VK_CONVERT || wParam == VK_LEFT) {
				SendMessage(hwnd, WM_KEYDOWN, VK_PRIOR, lParam);
				return 0;
			}
			if (wParam == VK_SPACE || wParam == VK_RIGHT) {
				SendMessage(hwnd, WM_KEYDOWN, VK_NEXT, lParam); 
				return 0;
			}
		} else {
			if (wParam == KEY_COLON && bCtrlKeyDown) {	// :
				if (pView) pView->InsertDate1();
			}
			if (wParam == KEY_SEMICOLON && bCtrlKeyDown) {	// ;
				if (pView) pView->InsertDate2();
			}
			if (wParam == VK_CONTROL) {
				bCtrlKeyDown = TRUE;
			}
		}
		break;
	case WM_KEYUP:
		if (wParam == VK_CONTROL) {
			bCtrlKeyDown = FALSE;
		}
		break;
	case WM_CHAR:
		pManager->SetMDSearchFlg(TRUE); // clear search status flag

		// if read only mode, ignore key events
		if (pView->IsReadOnly()) return 0;
		// disable Ctrl-B(BELL)
		if (wParam == KEY_CTRL_B) return 0;
		break;

#if defined(PLATFORM_PKTPC)
	case WM_LBUTTONDOWN:
		{
			// clear search status
			pManager->SetMDSearchFlg(TRUE);

			// Tap&hold
			SHRGINFO rgi;
			rgi.cbSize = sizeof(SHRGINFO);
			rgi.hwndClient = hwnd;
			rgi.ptDown.x = LOWORD(lParam);
			rgi.ptDown.y = HIWORD(lParam);
			rgi.dwFlags = SHRG_RETURNCMD;

			if (SHRecognizeGesture(&rgi) == GN_CONTEXTMENU) {
				//IDR_MENUBAR1
				HMENU hX;
				HMENU hPopup;
				hX = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENUBAR1));
				hPopup = GetSubMenu(hX, 0);
				TrackPopupMenuEx(hPopup, 0, rgi.ptDown.x, rgi.ptDown.y, hParentWnd, NULL);
				DestroyMenu(hX);
				return 0;
			}
			break;
		}
#else
	case WM_LBUTTONDOWN:
		// clear search status
		pManager->SetMDSearchFlg(TRUE);
#if defined(PLATFORM_SIG3)
		{
			INT xPos, yPos;
			xPos = (INT)LOWORD(lParam);
			yPos = (INT)HIWORD(lParam);
			LPARAM l = MAKELPARAM(xPos, yPos);
			nSelBase = SendMessage(hwnd, EM_CHARFROMPOS, 0, l) & 0xFFFF;
		}
#endif
		break;
#endif
	case WM_SETFOCUS:
		{
			if (pView) pView->OnGetFocus();
			break;
		}
	}

	LRESULT lResult = CallWindowProc(gSuperProc, hwnd, msg, wParam, lParam);
	if (pView && msg != EM_GETMODIFY) {
		pView->SetModifyStatus();
	}
    return lResult;
}
