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
	TEXT("��"), TEXT("��"), TEXT("��"), TEXT("��"), TEXT("��"), TEXT("��"), TEXT("�y")
};

LPCTSTR pWeekE[7] = {
	TEXT("Sun"), TEXT("Mon"), TEXT("Tue"), TEXT("Wed"), TEXT("Thr"), TEXT("Fri"), TEXT("Sat")
};

///////////////////////////////////////////
// �E�B���h�E����
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
	// EDIT Control�̃T�u�N���X��
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
// �\��/��\���؂�ւ�
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
	// �^�b�v&�z�[���h���j���[���o���܂܉�ʂ��؂�ւ�����ۂɃ��j���[���������
	if (nCmdShow == SW_HIDE) {
		ReleaseCapture();
	}
#endif
	return UpdateWindow(hViewWnd);
}

///////////////////////////////////////////
// �E�B���h�E�T�C�Y�̈ړ�
///////////////////////////////////////////

void MemoDetailsView::MoveWindow(DWORD x, DWORD y, DWORD nWidth, DWORD nHeight)
{
	::MoveWindow(hViewWnd_nf, x + nLeftOffset, y, nWidth - nLeftOffset, nHeight, TRUE);
	::MoveWindow(hViewWnd_fd, x + nLeftOffset, y, nWidth - nLeftOffset, nHeight, TRUE);
}

///////////////////////////////////////////
// OnCommand�̏���
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
// �z�b�g�L�[�̏���
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
// �t�H�[�J�X�̎擾
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
// �����̐ݒ�
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
// �����̎擾
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
// �t�H���g�̐ݒ�
/////////////////////////////////////////

void MemoDetailsView::SetFont(HFONT hFont)
{
	SendMessage(hViewWnd_fd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SendMessage(hViewWnd_nf, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
}

/////////////////////////////////////////
// �J�[�\���ʒu�̎擾
/////////////////////////////////////////

DWORD MemoDetailsView::GetCursorPos()
{
	DWORD nPos;
	SendMessage(hViewWnd, EM_GETSEL, (WPARAM)&nPos, (LPARAM)NULL);
	return nPos;
}

/////////////////////////////////////////
// ���t������̎擾
/////////////////////////////////////////

static BOOL GetDateText(TString *pInsStr, LPCTSTR pFormat)
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	// �����o�b�t�@�T�C�Y�̃J�E���g
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
// �X�e�[�^�X�\��
/////////////////////////////////////////

void MemoDetailsView::SetModifyStatus()
{
	pMemoMgr->GetMainFrame()->SetModifyStatus(IsModify());
}

/////////////////////////////////////////
// �X�e�[�^�X�\��
/////////////////////////////////////////

void MemoDetailsView::SelectAll() 
{	
	SetFocus();
	SendMessage(hViewWnd, EM_SETSEL, 0, -1); 
}

/////////////////////////////////////////
// �܂�Ԃ��\���̐؂�ւ�
/////////////////////////////////////////

BOOL MemoDetailsView::SetFolding(BOOL bFold)
{
	HWND hPrev;
	HWND hAfter;

	// ���ݎg�p����Ă���E�B���h�E�̐؂�ւ�
	if (hViewWnd == hViewWnd_fd) {
		hPrev = hViewWnd_fd;
		hAfter = hViewWnd_nf;
	} else {
		hPrev = hViewWnd_nf;
		hAfter = hViewWnd_fd;
	}
	hViewWnd = hAfter;

	// �e�L�X�g�{���̈��p��
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

	// �X�V��Ԃ̈��p��
	SendMessage(hAfter, EM_SETMODIFY, (WPARAM)SendMessage(hPrev, EM_GETMODIFY, 0, 0), 0);

	// �J�[�\���ʒu�̈��p��

	// ���ݕ\������Ă���ꍇ�A�\����Ԃ�؂�ւ�
	if (!bShowStatus) return TRUE;
	::ShowWindow(hPrev, SW_HIDE);
	::ShowWindow(hAfter, SW_SHOW);

	return TRUE;
}

/////////////////////////////////////////
// �^�u�X�g�b�v�̐ݒ�
/////////////////////////////////////////

void MemoDetailsView::SetTabstop() {
	DWORD n = g_Property.Tabstop() * 4;
	SendMessage(hViewWnd_fd, EM_SETTABSTOPS, 1, (LPARAM)&n);
	SendMessage(hViewWnd_nf, EM_SETTABSTOPS, 1, (LPARAM)&n);
}

/////////////////////////////////////////
// ����
/////////////////////////////////////////
//
// RESULT: return TRUE if matched.
//
// bFirstSearch : �����J�n�ʒu: TRUE = �����擪 FALSE = ���݂̃J�[�\���ʒu + 1
// bForward : �����̌���: TRUE = ������ FALSE = �t����
// bNFMsg: ������Ȃ������ꍇ�Ƀ��b�Z�[�W���o����
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
	// �G�f�B�b�g�R���g���[������擾�����J�[�\���ʒu��Wide Char�ł̕������̂��߁A
	// SJIS��ł̃o�C�g���ɕϊ�
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

		// �}�b�`���O���������ʒu��MultiByte�̃o�C�g���̂��߁A
		// EDITBOX���H����悤��WideChar���Z�ł̕������ɕϊ�����
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