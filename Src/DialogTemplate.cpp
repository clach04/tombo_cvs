#include <windows.h>
#include <tchar.h>
#include "DialogTemplate.h"

namespace Tombo_Lib {

////////////////////////////////////////////////////////////////
// Dialog Procedure(Template)
////////////////////////////////////////////////////////////////

static BOOL APIENTRY DlgProcTemplate(HWND hDlg, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	DialogTemplate *pDlg;
	if (nMessage == WM_INITDIALOG) {
		SetWindowLong(hDlg, DWL_USER, lParam);
		pDlg = (DialogTemplate*)lParam;

		pDlg->SetDialog(hDlg);
		pDlg->InitDialog(hDlg);
		return TRUE;
	}

	pDlg = (DialogTemplate*)GetWindowLong(hDlg, DWL_USER);
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
		default:
			return pDlg->OnCommand(hDlg, wParam, lParam);
		}
		return TRUE;
	case WM_NOTIFY:
		return pDlg->OnNotify(hDlg, wParam, lParam);
	}
	return FALSE;

}

////////////////////////////////////////////////////////////////
// Default handler
////////////////////////////////////////////////////////////////

void DialogTemplate::InitDialog(HWND hDlg)
{
}

BOOL DialogTemplate::OnOK()
{
	return TRUE;
}

BOOL DialogTemplate::OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

BOOL DialogTemplate::OnNotify(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

void DialogTemplate::SetDialog(HWND hDlg)
{
	hDialog = hDlg;
}

////////////////////////////////////////////////////////////////
// Popup entry
////////////////////////////////////////////////////////////////

DWORD DialogTemplate::Popup(HINSTANCE hInst, WORD nID, HWND hParent)
{
	hInstance = hInst;
	
	DWORD result;
	result = DialogBoxParam(hInst, MAKEINTRESOURCE(nID), 
								hParent, (DLGPROC)DlgProcTemplate, (LONG)this);

	return result;
}

} // end namespace