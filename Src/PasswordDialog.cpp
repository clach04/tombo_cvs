#include <windows.h>
#include <tchar.h>
#include <commctrl.h>

#include "Tombo.h"
#include "PasswordDialog.h"
#include "resource.h"
#include "UniConv.h"
#include "MemoNote.h"
#include "SipControl.h"
#include "Message.h"

//////////////////////////////////////////////////////////
// デストラクタ
//////////////////////////////////////////////////////////

PasswordDialog::~PasswordDialog()
{
	if (pPassword != NULL) {
		char *p = pPassword;
		// パスワードエリアの初期化
		while(*p) {
			*p++ = '\0';
		}
	}
}

//////////////////////////////////////////////////////////
// ダイアログプロシージャ
//////////////////////////////////////////////////////////

static BOOL APIENTRY DlgProc(HWND hDlg, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	PasswordDialog *pDlg;
	if (nMessage == WM_INITDIALOG) {
		SetWindowLong(hDlg, DWL_USER, lParam);
		pDlg = (PasswordDialog*)lParam;

		pDlg->InitDialog(hDlg);
		return TRUE;
	}

	pDlg = (PasswordDialog*)GetWindowLong(hDlg, DWL_USER);
	if (pDlg == NULL) return FALSE;

	switch (nMessage) {
	case WM_COMMAND:
		switch (wParam) {
		case IDOK:
			if (pDlg->OnOK(hDlg)) {
				pDlg->ClearPassword(hDlg);
				EndDialog(hDlg, IDOK);
			}
			break;
		case IDCANCEL:
			pDlg->ClearPassword(hDlg);
			EndDialog(hDlg, IDCANCEL);
			break;
		}
		return TRUE;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////
// ポップアップ
//////////////////////////////////////////////////////////

DWORD PasswordDialog::Popup(HINSTANCE hInst, HWND hParent, BOOL b)
{
	hInstance = hInst;
	bVerify = b;

	SipControl sc;
	BOOL bSipStat, bResult;

	bResult = sc.Init() && sc.GetSipStat(&bSipStat);
	if (bResult) sc.SetSipStat(TRUE);

	DWORD result;
	result = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PASSWORD), 
								hParent, (DLGPROC)DlgProc, (LONG)this);

	if (bResult) sc.SetSipStat(bSipStat);
	return result;
}

//////////////////////////////////////////////////////////
// ダイアログ初期化
//////////////////////////////////////////////////////////

void PasswordDialog::InitDialog(HWND hDlg)
{
	ClearPassword(hDlg);
	HWND hEdit2 = GetDlgItem(hDlg, IDC_PASS2);
	EnableWindow(hEdit2, bVerify);
	SendMessage(hEdit2, EM_SETREADONLY, (WPARAM)!bVerify, 0);
}

//////////////////////////////////////////////////////////
// パスワード入力EDITBOXのクリア
//////////////////////////////////////////////////////////
// GetWindowText()でのパスワード横取りを阻止する
// これで本当にメモリから情報が消去されるかは不明。

void PasswordDialog::ClearPassword(HWND hDlg)
{
	HWND hEdit = GetDlgItem(hDlg, IDC_PASS);
	HWND hEdit2 = GetDlgItem(hDlg, IDC_PASS2);
	SetWindowText(hEdit, TEXT(""));
	SetWindowText(hEdit2, TEXT(""));
}

//////////////////////////////////////////////////////////
// OKボタン
//////////////////////////////////////////////////////////

static LPTSTR GetPass(HWND hEdit)
{
	DWORD n = GetWindowTextLength(hEdit);
	LPTSTR p = new TCHAR[n + 1];
	if (p == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	GetWindowText(hEdit, p, n + 1);
	p[n] = TEXT('\0');
	return p;
}

BOOL PasswordDialog::OnOK(HWND hDlg)
{
	HWND hEdit = GetDlgItem(hDlg, IDC_PASS);
	HWND hEdit2 = GetDlgItem(hDlg, IDC_PASS2);
	LPTSTR pPass1 = GetPass(hEdit);
	if (!pPass1) return FALSE;
	if (_tcslen(pPass1) == 0) {
		MemoNote::WipeOutAndDelete(pPass1);
		return FALSE;
	}

	LPTSTR pPass2;

	if (bVerify) {
		pPass2 = GetPass(hEdit2);
		if (!pPass2) {
			MemoNote::WipeOutAndDelete(pPass1);
			return FALSE;
		}
		if (_tcscmp(pPass1, pPass2) != 0) {
			TomboMessageBox(hDlg, MSG_PASS_NOT_MATCH, TEXT("Warning"), MB_ICONEXCLAMATION | MB_OK);
			MemoNote::WipeOutAndDelete(pPass1);
			MemoNote::WipeOutAndDelete(pPass2);
			return FALSE;
		}
		MemoNote::WipeOutAndDelete(pPass2);
	}
	pPassword = ConvUnicode2SJIS(pPass1);

	MemoNote::WipeOutAndDelete(pPass1);
	return (pPassword != NULL);
}