#include <windows.h>
#include <tchar.h>
#include <commctrl.h>

#include "Tombo.h"
#include "Property.h"
#include "resource.h"
#include "SearchDlg.h"
#include "UniConv.h"
#include "SipControl.h"

////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////

#define NUM_SEARCH_HISTORY 10

////////////////////////////////////////////////////////////////
// ctor & dtor
////////////////////////////////////////////////////////////////

SearchDialog::~SearchDialog()
{
	if (pSearchStr) delete [] pSearchStr;
}


////////////////////////////////////////////////////////////////
// ダイアログプロシージャ
////////////////////////////////////////////////////////////////

static BOOL APIENTRY DlgProc(HWND hDlg, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	SearchDialog *pDlg;
	if (nMessage == WM_INITDIALOG) {
		SetWindowLong(hDlg, DWL_USER, lParam);
		pDlg = (SearchDialog*)lParam;

		pDlg->InitDialog(hDlg);
		return TRUE;
	}

	pDlg = (SearchDialog*)GetWindowLong(hDlg, DWL_USER);
	if (pDlg == NULL) return FALSE;

	switch (nMessage) {
	case WM_COMMAND:
		switch (wParam) {
		case IDOK:
			if (pDlg->OnOK()) {
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

////////////////////////////////////////////////////////////////
// ダイアログ初期化
////////////////////////////////////////////////////////////////

void SearchDialog::InitDialog(HWND hDlg)
{
	hDialog = hDlg;

	HWND hCombo = GetDlgItem(hDlg, IDC_SEARCH_STRING);
	LoadHistory(hCombo, TOMBO_SEARCHHIST_ATTR_NAME);

	HWND hCheckEncrypt = GetDlgItem(hDlg, IDC_SEARCH_ENCRYPTMEMO);
	EnableWindow(hCheckEncrypt, bCheckEncrypt);
	HWND hFileNameOnly = GetDlgItem(hDlg, IDC_FILENAMEONLY);
	EnableWindow(hFileNameOnly, bFileNameOnly);
	
	CheckRadioButton(hDlg, IDC_SEARCH_DIRECTION_UP, IDC_SEARCH_DIRECTION_DOWN, IDC_SEARCH_DIRECTION_DOWN);
}

////////////////////////////////////////////////////////////////
// ポップアップ
////////////////////////////////////////////////////////////////

DWORD SearchDialog::Popup(HINSTANCE hInst, HWND hParent, BOOL bCE)
{
	hInstance = hInst;

	bCheckEncrypt = bFileNameOnly = bCE;

	SipControl sc;
	BOOL bSipStat, bResult;
		bResult = sc.Init() && sc.GetSipStat(&bSipStat);
	if (bResult) sc.SetSipStat(TRUE);

	DWORD result;
	result = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SEARCH), 
								hParent, (DLGPROC)DlgProc, (LONG)this);

	if (bResult) sc.SetSipStat(bSipStat);
	return result;
}

////////////////////////////////////////////////////////////////
// OK
////////////////////////////////////////////////////////////////

BOOL SearchDialog::OnOK()
{
	// 暗号化メモを対象とする
	HWND hCheckEncrypt = GetDlgItem(hDialog, IDC_SEARCH_ENCRYPTMEMO);
	bCheckEncrypt = (SendMessage(hCheckEncrypt, BM_GETCHECK, 0, 0) == BST_CHECKED);
	
	// 大文字小文字を区別
	HWND hCaseSensitive = GetDlgItem(hDialog, IDC_SEARCH_CASESENSITIVE);
	bCaseSensitive = (SendMessage(hCaseSensitive, BM_GETCHECK, 0, 0) == BST_CHECKED);

	HWND hFileNameOnly = GetDlgItem(hDialog, IDC_FILENAMEONLY);
	bFileNameOnly = (SendMessage(hFileNameOnly, BM_GETCHECK, 0, 0) == BST_CHECKED);

	// Search direction
	HWND hSearchDirectionUp = GetDlgItem(hDialog, IDC_SEARCH_DIRECTION_UP);
	bSearchDirectionUp = (SendMessage(hSearchDirectionUp, BM_GETCHECK, 0, 0) == BST_CHECKED);

	// 検索文字列
	HWND hSearchWord = GetDlgItem(hDialog, IDC_SEARCH_STRING);
	int n = GetWindowTextLength(hSearchWord);

	// 何も入れていなければ無視
	if (n == 0) return FALSE;

	LPTSTR pSearchTextW = new TCHAR[n + 2];
	if (pSearchTextW == NULL) return FALSE;
	GetWindowText(hSearchWord, pSearchTextW, n + 1);
	
	RetrieveAndSaveHistory(hSearchWord, TOMBO_SEARCHHIST_ATTR_NAME, pSearchTextW, NUM_SEARCH_HISTORY);
	pSearchStr = pSearchTextW;
	return TRUE;
}