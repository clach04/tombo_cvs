#include <windows.h>
#include <tchar.h>
#if defined(_WIN32_WCE) && defined(PLATFORM_PKTPC)
#include <aygshell.h>
#endif
#include "resource.h"
#include "MemoDetailsView.h"
#include "MemoManager.h"

#include "Property.h"

static MemoManager *pManager; 
static MemoDetailsView *pView;
static WNDPROC gSuperProc;
static HINSTANCE hInst;
static HWND hParentWnd;

#define KEY_ESC 0x1B
#define KEY_CTRL_A 1
#define KEY_CTRL_B 2

void SetWndProc(WNDPROC wp, HWND hParent, HINSTANCE h, MemoDetailsView *p, MemoManager *pMgr)
{
	gSuperProc = wp;
	hParentWnd = hParent;
	hInst = h;
	pView = p;
	pManager = pMgr;
}

/////////////////////////////////////////
// サブクラス化用Window Procedure
/////////////////////////////////////////
//
// タップ&ホールド/ESCによる一覧ビューへのリターンに対応するため、
// エディットコントロールに対して
// ウィンドウプロシージャの乗っ取りを行っている。

#if !defined(PLATFORM_PSPC) && !defined(PLATFORM_BE500)
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
//		if (wParam == 'B' && GetKeyState(VK_CONTROL)) {
//			// toggle browsing mode
//			pView->SetReadOnly(!pView->IsReadOnly());
//			return 0;
//		}
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
			// 検索状態フラグクリア
			pManager->SetMDSearchFlg(TRUE);

			// タップ&ホールド処理
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
		// 検索状態フラグクリア
		pManager->SetMDSearchFlg(TRUE);
		break;
#endif
	case WM_SETFOCUS:
		{
			if (g_Property.IsUseTwoPane() && pView) {
				pView->OnGetFocus();
			}
			break;
		}
	}

	LRESULT lResult = CallWindowProc((WNDPROC)gSuperProc, hwnd, msg, wParam, lParam);
	if (pView && (msg == WM_CHAR || msg == WM_KEYDOWN && wParam == VK_DELETE)) {
		pView->SetModifyStatus();
	}
    return lResult;
}
#endif