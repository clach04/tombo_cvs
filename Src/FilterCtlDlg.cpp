#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

#include "resource.h"
#include "Message.h"
#include "TString.h"
#include "TreeViewItem.h"
#include "MemoSelectView.h"
#include "FilterCtlDlg.h"
#include "VFManager.h"

static LRESULT CALLBACK FilterCtlDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////
// initialize
/////////////////////////////////////////


BOOL FilterCtlDlg::Init(MemoSelectView *p)
{
	pView = p;
	return TRUE;
}

/////////////////////////////////////////
// entry point
/////////////////////////////////////////

DWORD FilterCtlDlg::Popup(HINSTANCE hInst, HWND hParent, HIMAGELIST hImgList)
{
	hImageList = hImgList;
	return DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_FILTERCTL), hParent, (DLGPROC)FilterCtlDlgProc, (LONG)this);
}

/////////////////////////////////////////
// callbacks
/////////////////////////////////////////

void FilterCtlDlg::InitDialog(HWND hDlg)
{
	HWND hList = GetDlgItem(hDlg, IDC_FILTERCTL_LIST);

	ListView_SetImageList(hList, hImageList, LVSIL_SMALL);

	/////////////////////////////////////////
	// Insert column headers

	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 200;
	lvc.pszText = MSG_VFOLDER_NAME_TTL;
	ListView_InsertColumn(hList, 0, &lvc);

	/////////////////////////////////////////
	// Insert current items

	LVITEM li;
	li.mask = LVIF_TEXT | LVIF_IMAGE;
	li.iSubItem = 0;
	li.iImage = 8;
	DWORD i = 0;

	TreeViewVirtualFolderRoot *pVRoot = pView->GetVirtualFolderRoot();
	TreeViewVirtualFolderRoot::ItemIterator *pItr;
	pItr = pVRoot->GetIterator(pView);
	TString sLabel;
	if (pItr->First()) {
		do {
			if (!pItr->Get(&sLabel)) return;
			Sleep(1);
			li.iItem = i++;
			li.cchTextMax = _tcslen(sLabel.Get());
			li.pszText = sLabel.Get();
			ListView_InsertItem(hList, &li);
		} while (pItr->Next());
	}
}

BOOL FilterCtlDlg::OnOK(HWND hDlg)
{
	return TRUE;
}

/////////////////////////////////////////
// Dialog procedure
/////////////////////////////////////////

static LRESULT CALLBACK FilterCtlDlgProc(HWND hDlg, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	FilterCtlDlg *pDlg;
	if (nMessage == WM_INITDIALOG) {
		SetWindowLong(hDlg, DWL_USER, lParam);
		pDlg = (FilterCtlDlg*)lParam;

		pDlg->InitDialog(hDlg);
		return TRUE;
	}
	pDlg = (FilterCtlDlg*)GetWindowLong(hDlg, DWL_USER);
	if (pDlg == NULL) return FALSE;

	switch(nMessage) {
	case WM_COMMAND:
		switch (wParam) {
		case IDOK:
			if (pDlg->OnOK(hDlg)) {
				// prevent removing imagelist
				HWND hList = GetDlgItem(hDlg, IDC_FILTERCTL_LIST);
				ListView_SetImageList(hList, NULL, LVSIL_SMALL);

				EndDialog(hDlg, IDOK);
			}
			break;
		case IDCANCEL:
			{
				// prevent removing imagelist
				HWND hList = GetDlgItem(hDlg, IDC_FILTERCTL_LIST);
				ListView_SetImageList(hList, NULL, LVSIL_SMALL);

				EndDialog(hDlg, IDCANCEL);
				break;
			}
		}
		return TRUE;
	}
	return FALSE;
}
