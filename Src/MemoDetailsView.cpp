#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#if defined(_WIN32_WCE) && defined(PLATFORM_PKTPC)
#include <aygshell.h>
#endif
#include "Tombo.h"
#include "MemoManager.h"
#include "MainFrame.h"
#include "MemoDetailsView.h"
#include "resource.h"
#include "TString.h"
#include "UniConv.h"
#include "Property.h"
#include "SearchEngine.h"
#include "Message.h"


static BOOL GetDateText(TString *pInsStr, LPCTSTR pFormat);

void SetWndProc(SUPER_WND_PROC wp, HWND hParent, HINSTANCE h, MemoDetailsView *p, MemoManager *pMgr);
LRESULT CALLBACK NewDetailsViewProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

LPCTSTR pMonth[12] = {
	TEXT("Jan"), TEXT("Feb"), TEXT("Mar"), TEXT("Apr"),
	TEXT("May"), TEXT("Jun"), TEXT("Jul"), TEXT("Aug"),
	TEXT("Sep"), TEXT("Oct"), TEXT("Nov"), TEXT("Dec")
};

LPCTSTR pWeekJ[7] = {
	TEXT("日"), TEXT("月"), TEXT("火"), TEXT("水"), TEXT("木"), TEXT("金"), TEXT("土")
};

LPCTSTR pWeekE[7] = {
	TEXT("Sun"), TEXT("Mon"), TEXT("Tue"), TEXT("Wed"), TEXT("Thr"), TEXT("Fri"), TEXT("Sat")
};

///////////////////////////////////////////
// ウィンドウ生成
///////////////////////////////////////////
extern HINSTANCE g_hInstance;

BOOL MemoDetailsView::Create(LPCTSTR pName, RECT &r, HWND hParent, DWORD nID, DWORD nID_nf, HINSTANCE hInst, HFONT hFont)
{
	DWORD nWndStyle;

	nWndStyle = WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_WANTRETURN;

#if defined(PLATFORM_PKTPC) || defined(PLATFORM_PSPC) || defined(PLATFORM_BE500)
	nLeftOffset = 3;
#else
	nLeftOffset = 0;
#endif
	
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	hViewWnd_fd = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), pName, nWndStyle, 
							r.left + nLeftOffset, r.top, r.right - nLeftOffset, r.bottom, 
							hParent, (HMENU)nID, hInst, this);
	hViewWnd_nf = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), pName, nWndStyle | ES_AUTOHSCROLL | WS_HSCROLL, 
							r.left + nLeftOffset, r.top, r.right - nLeftOffset, r.bottom, 
							hParent, (HMENU)nID_nf, hInst, this);
#else
	hViewWnd_fd = CreateWindow(TEXT("EDIT"), pName, nWndStyle, 
							r.left + nLeftOffset, r.top, r.right - nLeftOffset, r.bottom, 
							hParent, (HMENU)nID, hInst, NULL);
	hViewWnd_nf = CreateWindow(TEXT("EDIT"), pName, nWndStyle | ES_AUTOHSCROLL | WS_HSCROLL, 
							r.left + nLeftOffset, r.top, r.right - nLeftOffset, r.bottom, 
							hParent, (HMENU)nID_nf, hInst, NULL);
#endif
	if (hViewWnd_fd == NULL || hViewWnd_nf == NULL) return FALSE;
	hViewWnd = hViewWnd_fd;

//#if !defined(PLATFORM_PSPC) && !defined(PLATFORM_BE500)
	// EDIT Controlのサブクラス化
	SUPER_WND_PROC wp = (SUPER_WND_PROC)GetWindowLong(hViewWnd, GWL_WNDPROC);
	SetWndProc(wp, hParent, g_hInstance, this, pMemoMgr);
	SetWindowLong(hViewWnd_nf, GWL_WNDPROC, (LONG)NewDetailsViewProc);
	SetWindowLong(hViewWnd_fd, GWL_WNDPROC, (LONG)NewDetailsViewProc);
//#endif

	if (hFont != NULL) {
		SetFont(hFont);
	}
	SetTabstop();
	return TRUE;
}

///////////////////////////////////////////
// 表示/非表示切り替え
///////////////////////////////////////////

BOOL MemoDetailsView::Show(int nCmdShow)
{
	ShowWindow(hViewWnd, nCmdShow);
	if (nCmdShow == SW_SHOW) {
		bShowStatus = TRUE;
	} else if (nCmdShow == SW_HIDE) {
		bShowStatus = FALSE;
	}

#if defined(PLATFORM_PKTPC)
	// タップ&ホールドメニューが出たまま画面が切り替わった際にメニューを閉じさせる
	if (nCmdShow == SW_HIDE) {
		ReleaseCapture();
	}
#endif
	return UpdateWindow(hViewWnd);
}

///////////////////////////////////////////
// ウィンドウサイズの移動
///////////////////////////////////////////

void MemoDetailsView::MoveWindow(DWORD x, DWORD y, DWORD nWidth, DWORD nHeight)
{
	::MoveWindow(hViewWnd_nf, x + nLeftOffset, y, nWidth - nLeftOffset, nHeight, TRUE);
	::MoveWindow(hViewWnd_fd, x + nLeftOffset, y, nWidth - nLeftOffset, nHeight, TRUE);
}

///////////////////////////////////////////
// OnCommandの処理
///////////////////////////////////////////

BOOL MemoDetailsView::OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam)) {
	case IDM_CUT:
		SendMessage(hViewWnd, WM_CUT, 0, 0);
		return TRUE;
	case IDM_COPY:
		SendMessage(hViewWnd, WM_COPY, 0, 0);
		return TRUE;
	case IDM_PASTE:
		SendMessage(hViewWnd, WM_PASTE, 0, 0);
		return TRUE;
	case IDM_UNDO:
		SendMessage(hViewWnd, WM_UNDO, 0, 0);
		return TRUE;
	case IDM_ACTIONBUTTON:
		PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_RETURNLIST, 0), 0);
		return TRUE;
	case IDM_INSDATE1:
		{
			TString sDate;
			if (!GetDateText(&sDate, g_Property.DateFormat1())) {
				TomboMessageBox(NULL, MSG_GET_DATE_FAILED, TEXT("ERROR"), MB_ICONERROR | MB_OK);
				return TRUE;
			}
			SendMessage(hViewWnd, EM_REPLACESEL, 0, (LPARAM)sDate.Get());
			return TRUE;
		}
	case IDM_INSDATE2:
		{
			TString sDate;
			if (!GetDateText(&sDate, g_Property.DateFormat2())) {
				TomboMessageBox(NULL, MSG_GET_DATE_FAILED, TEXT("ERROR"), MB_ICONERROR | MB_OK);
				return TRUE;
			}
			SendMessage(hViewWnd, EM_REPLACESEL, 0, (LPARAM)sDate.Get());
			return TRUE;
		}
	case IDM_TOGGLEREADONLY:
		{
			SetReadOnly(!IsReadOnly());
			return FALSE;
		}
	}

	return FALSE;
}

///////////////////////////////////////////
// ホットキーの処理
///////////////////////////////////////////

BOOL MemoDetailsView::OnHotKey(HWND hWnd, WPARAM wParam)
{
	switch(wParam) {
	case APP_BUTTON1:
		/* fall through */
	case APP_BUTTON2:
		/* fall through */
	case APP_BUTTON3:
		/* fall through */
	case APP_BUTTON4:
		/* fall through */
	case APP_BUTTON5:
		PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_RETURNLIST, 0), 0);
		return TRUE;
	default:
		return FALSE;
	}
}

///////////////////////////////////////////
// フォーカスの取得
///////////////////////////////////////////

void MemoDetailsView::OnGetFocus()
{
	if (!g_Property.IsUseTwoPane()) return;

	MainFrame *pMf = pMemoMgr->GetMainFrame();
	if (pMf) {
		// switch view
		pMf->ActivateView(FALSE);

		// menu control
		pMf->EnableDelete(FALSE);
		pMf->EnableRename(FALSE);
		pMf->EnableEncrypt(FALSE);
		pMf->EnableDecrypt(FALSE);

		pMf->EnableCut(TRUE);
		pMf->EnableCopy(TRUE);
		pMf->EnablePaste(TRUE);

	}
	SetModifyStatus();
}

///////////////////////////////////////////
// メモの設定
///////////////////////////////////////////

BOOL MemoDetailsView::SetMemo(LPCTSTR pMemo, DWORD nPos, BOOL bReadOnly)
{
	SetReadOnly(bReadOnly);

	SetWindowText(hViewWnd, pMemo);
	if (g_Property.KeepCaret()) {
		SendMessage(hViewWnd, EM_SETSEL, nPos, nPos);
		PostMessage(hViewWnd, EM_SCROLLCARET, 0, 0);
	}
	SetModifyStatus();
	return TRUE;
}	

///////////////////////////////////////////
// メモの取得
///////////////////////////////////////////

LPTSTR MemoDetailsView::GetMemo()
{
	DWORD n = GetWindowTextLength(hViewWnd);

	LPTSTR p = new TCHAR[n + 1];
	if (p == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	GetWindowText(hViewWnd, p,n+1);
	return p;
}

/////////////////////////////////////////
// フォントの設定
/////////////////////////////////////////

void MemoDetailsView::SetFont(HFONT hFont)
{
	SendMessage(hViewWnd_fd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SendMessage(hViewWnd_nf, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
}

/////////////////////////////////////////
// カーソル位置の取得
/////////////////////////////////////////

DWORD MemoDetailsView::GetCursorPos()
{
	DWORD nPos;
	SendMessage(hViewWnd, EM_GETSEL, (WPARAM)&nPos, (LPARAM)NULL);
	return nPos;
}

/////////////////////////////////////////
// 日付文字列の取得
/////////////////////////////////////////

static BOOL GetDateText(TString *pInsStr, LPCTSTR pFormat)
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	// 文字バッファサイズのカウント
	DWORD nLen = 0;
	LPCTSTR p = pFormat;
	while(*p) {
		if (iskanji(*p)) {
			p += 2;
			nLen += 2;
			continue;
		}
		if (*p == TEXT('%')) {
			p++;
			switch(*p) {
			case TEXT('y'):
				nLen += 4;
				break;
			case TEXT('b'):
				nLen += 3;
				break;
			case TEXT('w'):
			case TEXT('W'):
				nLen += 3;
				break;
			default:
				nLen += 2;
				break;
			}
			if (*p) p++;
		} else {
			p++;
			nLen++;
		}
	}
	nLen++;
	if (!pInsStr->Alloc(nLen)) return FALSE;

	LPTSTR q = pInsStr->Get();
	p = pFormat;
	while(*p) {
		if (iskanji(*p)) {
			*q++ = *p++;
			*q++ = *p++;
			continue;
		}
		if (*p == TEXT('%')) {
			p++;
			switch(*p) {
			case TEXT('y'):
				wsprintf(q, TEXT("%4d"), st.wYear);
				q += 4;
				break;
			case TEXT('Y'):
				wsprintf(q, TEXT("%02d"), st.wYear % 100);
				q += 2;
				break;
			case TEXT('M'):
				wsprintf(q, TEXT("%02d"), st.wMonth);
				q += 2;
				break;
			case TEXT('b'):
				_tcscpy(q, pMonth[st.wMonth - 1]);
				q += 3;
				break;
			case TEXT('d'):
				wsprintf(q, TEXT("%02d"), st.wDay);
				q += 2;
				break;
			case TEXT('D'):
				wsprintf(q, TEXT("%d"), st.wDay);
				q++;
				if (st.wDay >= 10) q++;
				break;
#if !defined(TOMBO_LANG_ENGLISH)
			case TEXT('w'):
				_tcscpy(q, pWeekJ[st.wDayOfWeek]);
				q += _tcslen(pWeekJ[st.wDayOfWeek]);
				break;
#endif
			case TEXT('W'):
				_tcscpy(q, pWeekE[st.wDayOfWeek]);
				q += _tcslen(pWeekE[st.wDayOfWeek]);
				break;
			case TEXT('h'):
				wsprintf(q, TEXT("%02d"), st.wHour);
				q += 2;
				break;
			case TEXT('H'):
				wsprintf(q, TEXT("%02d"), st.wHour % 12);
				q += 2;
				break;
			case TEXT('a'):
				if (st.wHour >= 12) {
					_tcscpy(q, TEXT("PM"));
				} else {
					_tcscpy(q, TEXT("AM"));
				}
				q += 2;
				break;
			case TEXT('m'):
				wsprintf(q, TEXT("%02d"), st.wMinute);
				q += 2;
				break;
			case TEXT('s'):
				wsprintf(q, TEXT("%02d"), st.wSecond);
				q += 2;
				break;
			default:
				*q++ = TEXT('%');
				if (*p) *q++ = *p++;
			}
			p++;
		} else {
			*q++ = *p++;
		}
	}
	*q = TEXT('\0');
	return TRUE;
}

/////////////////////////////////////////
// ステータス表示
/////////////////////////////////////////

void MemoDetailsView::SetModifyStatus()
{
	pMemoMgr->GetMainFrame()->SetModifyStatus(IsModify());
}

/////////////////////////////////////////
// ステータス表示
/////////////////////////////////////////

void MemoDetailsView::SelectAll() 
{	
	SetFocus();
	SendMessage(hViewWnd, EM_SETSEL, 0, -1); 
}

/////////////////////////////////////////
// 折り返し表示の切り替え
/////////////////////////////////////////

BOOL MemoDetailsView::SetFolding(BOOL bFold)
{
	HWND hPrev;
	HWND hAfter;

	// 現在使用されているウィンドウの切り替え
	if (hViewWnd == hViewWnd_fd) {
		hPrev = hViewWnd_fd;
		hAfter = hViewWnd_nf;
	} else {
		hPrev = hViewWnd_nf;
		hAfter = hViewWnd_fd;
	}
	hViewWnd = hAfter;

	// テキスト本文の引継ぎ
	DWORD nLen = GetWindowTextLength(hPrev) + 1;
	LPTSTR p = new TCHAR[nLen];
	if (p == NULL) {
		hViewWnd = hPrev;
		return FALSE;
	}
	p[0] = TEXT('\0');
	GetWindowText(hPrev, p, nLen);
	SetWindowText(hAfter, p);
	delete [] p; p = NULL;

	// 更新状態の引継ぎ
	SendMessage(hAfter, EM_SETMODIFY, (WPARAM)SendMessage(hPrev, EM_GETMODIFY, 0, 0), 0);

	// カーソル位置の引継ぎ

	// 現在表示されている場合、表示状態を切り替え
	if (!bShowStatus) return TRUE;
	::ShowWindow(hPrev, SW_HIDE);
	::ShowWindow(hAfter, SW_SHOW);

	return TRUE;
}

/////////////////////////////////////////
// タブストップの設定
/////////////////////////////////////////

void MemoDetailsView::SetTabstop() {
	DWORD n = g_Property.Tabstop() * 4;
	SendMessage(hViewWnd_fd, EM_SETTABSTOPS, 1, (LPARAM)&n);
	SendMessage(hViewWnd_nf, EM_SETTABSTOPS, 1, (LPARAM)&n);
}

/////////////////////////////////////////
// 検索
/////////////////////////////////////////
//
// RESULT: return TRUE if matched.
//
// bFirstSearch : 検索開始位置: TRUE = メモ先頭 FALSE = 現在のカーソル位置 + 1
// bForward : 検索の向き: TRUE = 順方向 FALSE = 逆方向
// bNFMsg: 見つからなかった場合にメッセージを出すか
// pFound: if string is found, set TRUE otherwise set FALSE.


BOOL MemoDetailsView::Search(BOOL bFirstSearch, BOOL bForward, BOOL bNFMsg, BOOL bSearchFromTop)
{
	SearchEngineA *pSE;
	pSE = pMemoMgr->GetSearchEngine();
	if (pSE == NULL) return FALSE;

	LPTSTR pT = GetMemo();

	char *p;

	DWORD nSearchStart;
	BOOL bShift = FALSE;

	if (bFirstSearch) {
		nSearchStart = 0;
	} else {
		nSearchStart = GetCursorPos();
		bShift = TRUE;
	}

#ifdef _WIN32_WCE
	// Wide char(Unicode)->MBCS(SJIS)
	p = ConvUnicode2SJIS(pT);
	// エディットコントロールから取得したカーソル位置はWide Charでの文字数のため、
	// SJIS上でのバイト数に変換
	nSearchStart = CountWCBytes(pT, nSearchStart);
#else
	p = pT;
#endif

	BOOL bMatch;
	if (bForward) {
		bMatch = pSE->SearchForward(p, nSearchStart, bShift);
	} else {
		bMatch = pSE->SearchBackward(p, nSearchStart, bShift);
	}

#ifdef _WIN32_WCE
	delete [] pT;
#endif

	if (bMatch) {
		DWORD nStart = pSE->MatchStart();
		DWORD nEnd = pSE->MatchEnd();

#ifdef _WIN32_WCE
		DWORD nStart2;
		DWORD nEnd2;

		// マッチングした文字位置はMultiByteのバイト数のため、
		// EDITBOXが食えるようにWideChar換算での文字数に変換する
		nStart2 = CountMBStrings(p, nStart);
		nEnd2 = nStart2 + CountMBStrings(p + nStart, nEnd - nStart);

		// for debug code
//		TCHAR buf[1024];
//		wsprintf(buf, TEXT("%d - %d => %d - %d"), nStart, nEnd, nStart2, nEnd2);
//		MessageBox(NULL, buf, TEXT("DEBUG"), MB_OK);

		nStart = nStart2;
		nEnd = nEnd2;
#endif

		SendMessage(hViewWnd, EM_SETSEL, (WPARAM)nStart, (LPARAM)nEnd);
		SendMessage(hViewWnd, EM_SCROLLCARET, 0, 0);

	} else {
		if (bNFMsg) MessageBox(NULL, MSG_STRING_NOT_FOUND, TOMBO_APP_NAME, MB_OK | MB_ICONINFORMATION);
	}

	delete [] p;
	return bMatch;
}

/////////////////////////////////////////
// change read only mode
/////////////////////////////////////////

void MemoDetailsView::SetReadOnly(BOOL bro)
{
	bReadOnly = bro;
//	SendMessage(hViewWnd, EM_SETREADONLY, (WPARAM)bReadOnly, 0);
	if (pMemoMgr) pMemoMgr->GetMainFrame()->SetReadOnlyStatus(bReadOnly);
}