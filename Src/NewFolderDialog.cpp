#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include "NewFolderDialog.h"
#include "resource.h"
#include "SipControl.h"

//////////////////////////////////////////////////////////
// ダイアログプロシージャ
//////////////////////////////////////////////////////////

static BOOL APIENTRY DlgProc(HWND hDlg, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	NewFolderDialog *pDlg;
	if (nMessage == WM_INITDIALOG) {
		SetWindowLong(hDlg, DWL_USER, lParam);
		pDlg = (NewFolderDialog*)lParam;

		pDlg->InitDialog(hDlg);
		return TRUE;
	}

	pDlg = (NewFolderDialog*)GetWindowLong(hDlg, DWL_USER);
	if (pDlg == NULL) return FALSE;

	switch (nMessage) {
	case WM_COMMAND:
		switch (wParam) {
		case IDOK:
			if (pDlg->OnOK(hDlg)) {
				EndDialog(hDlg, IDOK);
			}
			break;
		case IDCANCEL:
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

DWORD NewFolderDialog::Popup(HINSTANCE hInst, HWND hParent)
{
	hInstance = hInst;

	SipControl sc;
	BOOL bSipStat, bResult;

	bResult = sc.Init() && sc.GetSipStat(&bSipStat);
	if (bResult) sc.SetSipStat(TRUE);
	
	DWORD result;
	result = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_NEWFOLDER), 
								hParent, (DLGPROC)DlgProc, (LONG)this);

	if (bResult) sc.SetSipStat(bSipStat);

	return result;
}

//////////////////////////////////////////////////////////
// ダイアログ初期化
//////////////////////////////////////////////////////////

void NewFolderDialog::InitDialog(HWND hDlg)
{
	HWND hEdit = GetDlgItem(hDlg, IDC_NEWFOLDER_NAME);
	SetWindowText(hEdit, TEXT(""));
}

//////////////////////////////////////////////////////////
// OKボタン
//////////////////////////////////////////////////////////

BOOL NewFolderDialog::OnOK(HWND hDlg)
{
	HWND hEdit = GetDlgItem(hDlg, IDC_NEWFOLDER_NAME);
	DWORD n = GetWindowTextLength(hEdit);
	if (n == 0) return FALSE;

	GetWindowText(hEdit, aFolder, n + 1);
	aFolder[n] = TEXT('\0');
	return TRUE;
}