#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include "resource.h"
#include "Message.h"
#include "Tombo.h"
#include "TString.h"
#include "FilterDefDlg.h"
#include "PropertyPage.h"
#include "VarBuffer.h"
#include "VFManager.h"
#include "VFStream.h"
#include "DialogTemplate.h"
#include "FilterAddDlg.h"

#define NUM_TAB_FILTERCTL 2

BOOL FilterDefDlg::Init()
{
	return TRUE;
}

/////////////////////////////////////////
// Src tab
/////////////////////////////////////////

class FilterDlgSrcTab : public PropertyTab {
	FilterDefDlg *pDialog;
public:
	FilterDlgSrcTab(FilterDefDlg *pDlg) : pDialog(pDlg), 
		PropertyTab(IDD_FILTERDEF_SRC, MSG_FILTERDEFPROPTTL_SRC, 
		(DLGPROC)DefaultPageProc) {}

	void Init(HWND hDlg);
	BOOL Apply(HWND hDlg);
};

void FilterDlgSrcTab::Init(HWND hDlg)
{
	HWND hSrcPath = GetDlgItem(hDlg, IDC_FILTERDEF_SRC_PATH);
	SetWindowText(hSrcPath, pDialog->pInfo->pGenerator->GetDirPath());
}

BOOL FilterDlgSrcTab::Apply(HWND hDlg)
{
	HWND hEdit = GetDlgItem(hDlg, IDC_FILTERDEF_SRC_PATH);

	DWORD nLen = GetWindowTextLength(hEdit);
	TString sPath;

	if (!sPath.Alloc(nLen + 1)) return FALSE;
	GetWindowText(hEdit, sPath.Get(), nLen + 1);

	return pDialog->pInfo->pGenerator->SetDirPath(sPath.Get());
}

/////////////////////////////////////////
// Filter tab
/////////////////////////////////////////

class FilterDlgFilterTab : public PropertyTab {
	FilterDefDlg *pDialog;
protected:
	BOOL InsertItem(HWND hList, DWORD iPos, VFStream *pStream, BOOL bFocus);
public:
	FilterDlgFilterTab(FilterDefDlg *pDlg) : pDialog(pDlg), 
		PropertyTab(IDD_FILTERDEF_FILTER, MSG_FILTERDEFPROPTTL_FILTER, 
		(DLGPROC)DefaultPageProc) {}

	void Init(HWND hDlg);
	BOOL Apply(HWND hDlg);
	void Cancel(HWND hDlg, WPARAM wParam, LPARAM lParam);
	BOOL OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
	BOOL OnNotify(HWND hDlg, WPARAM wParam, LPARAM lParam);

	BOOL Notify_Keydown(HWND hDlg, LPARAM lParam);
	BOOL Notify_ItemChanged(HWND hDlg, LPARAM lParam);
	BOOL Notify_DblClick(HWND hDlg, LPARAM lParam);
	BOOL Command_UpDown(HWND hDlg, int nDelta);
	BOOL Command_Regex(HWND hDlg);
	BOOL Command_Limit(HWND hDlg);
	BOOL Command_Timestamp(HWND hDlg);
	BOOL Command_Sort(HWND hDlg);
};

void FilterDlgFilterTab::Init(HWND hDlg)
{
	HWND hList = GetDlgItem(hDlg, IDC_FILTERDEF_FILTER_LIST);
	/////////////////////////////////////////
	// Insert column headers
	
	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvc.fmt = LVCFMT_LEFT;

	lvc.cx = 100;
	lvc.pszText = MSG_FILTERDEFPROPTTL_FILTERTYPE_HDR;
	ListView_InsertColumn(hList, 0, &lvc);

	lvc.cx = 150;
	lvc.pszText = MSG_FILTERDEFPROPTTL_FILTERVAL_HDR;
	ListView_InsertColumn(hList, 1, &lvc);

	/////////////////////////////////////////
	// Create stream data

	DWORD i = 0;
	VFStream *p = pDialog->pInfo->pGenerator->GetNext();

	// for exclude VFStore, loop condition is not "p".
	while(p->GetNext()) {
		InsertItem(hList, i, p, FALSE);
		i++;
		p = p->GetNext();
	}

	/////////////////////////////////////////
	// Initialize buttons
	HWND hUp = GetDlgItem(hDlg, IDC_FILTERDEF_FILTER_UP);
	HWND hDown = GetDlgItem(hDlg, IDC_FILTERDEF_FILTER_DOWN);
	EnableWindow(hUp, FALSE);
	EnableWindow(hDown, FALSE);
}

BOOL FilterDlgFilterTab::Apply(HWND hDlg) 
{
	HWND hList = GetDlgItem(hDlg, IDC_FILTERDEF_FILTER_LIST);
	VFInfo *pInfo = pDialog->pInfo;
	VFDirectoryGenerator *pGen = pInfo->pGenerator;
	VFStore *pStore = pInfo->pStore;

	int n = ListView_GetItemCount(hList);
	int i;
	LVITEM li;

	li.mask = LVIF_PARAM;
	li.iSubItem = 0;

	VFStream *pPrev = pGen;

	for (i = 0; i < n; i++) {
		li.iItem = i;
		ListView_GetItem(hList, &li);

		VFStream *p = (VFStream*)li.lParam;

		pPrev->SetNext(p);
		pPrev = p;
	}
	
	pPrev->SetNext(pStore);

	return TRUE; 
}

void FilterDlgFilterTab::Cancel(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	Apply(hDlg);
}

BOOL FilterDlgFilterTab::InsertItem(HWND hList, DWORD iPos, VFStream *pStream, BOOL bFocus)
{
	LV_ITEM li;
	TString sValue;

	li.iItem = iPos;

	// type
	li.mask = LVIF_TEXT | LVIF_PARAM;
	li.pszText = (LPTSTR)pStream->GetFilterType();
	li.cchTextMax = _tcslen(li.pszText);
	li.iSubItem = 0;
	li.lParam = (LPARAM)pStream;
	if (bFocus) {
		li.mask |= LVIF_STATE;
		li.state = li.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
	}
	ListView_InsertItem(hList, &li);

	// value
	if (!pStream->ToString(&sValue)) return FALSE;

	li.mask = LVIF_TEXT;
	li.pszText = sValue.Get();
	li.cchTextMax = _tcslen(li.pszText);
	li.iSubItem = 1;
	ListView_SetItem(hList, &li);
	return TRUE;
}

/////////////////////////////////////////
// WM_COMMAND handler
/////////////////////////////////////////

BOOL FilterDlgFilterTab::OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam)) {
	case IDC_FILTERDEF_FILTER_UP:
		return Command_UpDown(hDlg, -1);
	case IDC_FILTERDEF_FILTER_DOWN:
		return Command_UpDown(hDlg, 1);
	case IDC_FILTERDEF_FILTER_REGEX:
		return Command_Regex(hDlg);
	case IDC_FILTERDEF_FILTER_LIMIT:
		return Command_Limit(hDlg);
	case IDC_FILTERDEF_FILTER_TIMESTAMP:
		return Command_Timestamp(hDlg);
	case IDC_FILTERDEF_FILTER_SORT:
		return Command_Sort(hDlg);
		break;

	}
	return TRUE;
}

BOOL FilterDlgFilterTab::Command_UpDown(HWND hDlg, int iDelta)
{
	HWND hList = GetDlgItem(hDlg, IDC_FILTERDEF_FILTER_LIST);
	int iSel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);

	if (iSel <= 0) return TRUE;

	LVITEM li;
	li.mask = LVIF_PARAM;
	li.iItem = iSel;
	li.iSubItem = 0;
	ListView_GetItem(hList, &li);

	VFStream *pStream = (VFStream*)li.lParam;

	ListView_DeleteItem(hList, iSel);

	InsertItem(hList, iSel + iDelta, pStream, TRUE);
	SetFocus(hList);
	return TRUE;
}

BOOL FilterDlgFilterTab::Command_Regex(HWND hDlg)
{
	RegexFilterAddDlg ad;
	if (!ad.Init()) {
		MessageBox(hDlg, MSG_DLG_INIT_FAIL, TOMBO_APP_NAME, MB_OK | MB_ICONERROR);
		return TRUE;
	}
	if (ad.Popup(g_hInstance, hDlg) == IDOK) {
		VFRegexFilter *pFilter = new VFRegexFilter();
		if (pFilter == NULL) {
			MessageBox(hDlg, MSG_NOT_ENOUGH_MEMORY, TOMBO_APP_NAME, MB_OK | MB_ICONERROR);
			return TRUE;
		}

		if (!pFilter->Init(ad.GetMatchString()->Get(), 
			ad.IsCaseSensitive(), ad.IsCheckEncrypt(), 
			ad.IsCheckFileName(), ad.IsNegate(), g_pPasswordManager)) {
			MessageBox(hDlg, MSG_INVALID_REGEXP, TOMBO_APP_NAME, MB_OK | MB_ICONWARNING);
			return TRUE;
		}
		HWND hList = GetDlgItem(hDlg, IDC_FILTERDEF_FILTER_LIST);
		DWORD iPos = ListView_GetItemCount(hList);
		InsertItem(hList, iPos, pFilter, TRUE);
	}

	return TRUE;
}

BOOL FilterDlgFilterTab::Command_Limit(HWND hDlg)
{
	LimitFilterAddDlg ad;
	if (!ad.Init()) {
		MessageBox(hDlg, MSG_DLG_INIT_FAIL, TOMBO_APP_NAME, MB_OK | MB_ICONERROR);
		return TRUE;
	}
	if (ad.Popup(g_hInstance, hDlg) == IDOK) {
		VFLimitFilter *pFilter = new VFLimitFilter();
		if (!pFilter || !pFilter->Init(ad.GetLimit())) {
			MessageBox(hDlg, MSG_NOT_ENOUGH_MEMORY, TOMBO_APP_NAME, MB_OK | MB_ICONERROR);
			return TRUE;
		}
		HWND hList = GetDlgItem(hDlg, IDC_FILTERDEF_FILTER_LIST);
		DWORD iPos = ListView_GetItemCount(hList);
		InsertItem(hList, iPos, pFilter, TRUE);
	}
	return TRUE;
}

BOOL FilterDlgFilterTab::Command_Timestamp(HWND hDlg)
{
	TimestampFilterAddDlg ad;
	if (!ad.Init()) {
		MessageBox(hDlg, MSG_DLG_INIT_FAIL, TOMBO_APP_NAME, MB_OK | MB_ICONERROR);
		return TRUE;
	}
	if (ad.Popup(g_hInstance, hDlg) == IDOK) {
		VFTimestampFilter *pFilter = new VFTimestampFilter();
		if (!pFilter || !pFilter->Init(ad.GetDeltaDay(), ad.IsNewer())) {
			MessageBox(hDlg, MSG_NOT_ENOUGH_MEMORY, TOMBO_APP_NAME, MB_OK | MB_ICONERROR);
			return TRUE;
		}
		HWND hList = GetDlgItem(hDlg, IDC_FILTERDEF_FILTER_LIST);
		DWORD iPos = ListView_GetItemCount(hList);
		InsertItem(hList, iPos, pFilter, TRUE);
	}
	return TRUE;
}

BOOL FilterDlgFilterTab::Command_Sort(HWND hDlg)
{
	SortFilterAddDlg ad;
	if (!ad.Init()) {
		MessageBox(hDlg, MSG_DLG_INIT_FAIL, TOMBO_APP_NAME, MB_OK | MB_ICONERROR);
		return TRUE;
	}
	if (ad.Popup(g_hInstance, hDlg) == IDOK) {
		VFSortFilter *pFilter = new VFSortFilter();
		if (!pFilter || !pFilter->Init(ad.GetType())) {
			MessageBox(hDlg, MSG_NOT_ENOUGH_MEMORY, TOMBO_APP_NAME, MB_OK | MB_ICONERROR);
			return TRUE;
		}
		HWND hList = GetDlgItem(hDlg, IDC_FILTERDEF_FILTER_LIST);
		DWORD iPos = ListView_GetItemCount(hList);
		InsertItem(hList, iPos, pFilter, TRUE);
	}
	return TRUE;
}

/////////////////////////////////////////
// WM_NOTIFY handler
/////////////////////////////////////////

BOOL FilterDlgFilterTab::OnNotify(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	NMHDR *pHdr = (NMHDR*)lParam;
	if (pHdr->code == LVN_KEYDOWN) {
		return Notify_Keydown(hDlg, lParam);
	}
	if (pHdr->code == LVN_ITEMCHANGED) {
		return Notify_ItemChanged(hDlg, lParam);
	}
	if (pHdr->code == NM_DBLCLK && pHdr->idFrom == IDC_FILTERDEF_FILTER_LIST) {
		return Notify_DblClick(hDlg, lParam);
	}
	return FALSE;
}

BOOL FilterDlgFilterTab::Notify_Keydown(HWND hDlg, LPARAM lParam)
{
	NMHDR *pHdr = (NMHDR*)lParam;

	if (pHdr->idFrom != IDC_FILTERDEF_FILTER_LIST) return FALSE;
	if (pHdr->code != LVN_KEYDOWN) return FALSE;
	HWND hList = pHdr->hwndFrom;

	NMLVKEYDOWN *pKd = (NMLVKEYDOWN*)lParam;
	if (pKd->wVKey != VK_DELETE) return TRUE;

	// DEL key handling
	int iSel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
	if (iSel < 0 ) return TRUE;

	// delete Item
	LV_ITEM li;
	li.mask = LVIF_PARAM;
	li.iItem = iSel;
	li.iSubItem = 0;
	ListView_GetItem(hList, &li);
	VFStream *p = (VFStream*)li.lParam;
	ListView_DeleteItem(hList, iSel);
	delete p;

	// select altanative item
	li.mask = LVIF_STATE;
	li.iItem = (iSel == 0) ? 0 : iSel - 1;
	li.state = li.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
	ListView_SetItem(hList, &li);

	return TRUE;
}

BOOL FilterDlgFilterTab::Notify_ItemChanged(HWND hDlg, LPARAM lParam)
{
	NMHDR *pHdr = (NMHDR*)lParam;
	NMLISTVIEW *pLv = (NMLISTVIEW*)lParam;
	if (pLv->uNewState & LVIS_FOCUSED) {

		HWND hUp = GetDlgItem(hDlg, IDC_FILTERDEF_FILTER_UP);
		HWND hDown = GetDlgItem(hDlg, IDC_FILTERDEF_FILTER_DOWN);

		EnableWindow(hUp, pLv->iItem > 0);
		EnableWindow(hDown, pLv->iItem < ListView_GetItemCount(pHdr->hwndFrom) - 1);
	}
	return TRUE;
}

BOOL FilterDlgFilterTab::Notify_DblClick(HWND hDlg, LPARAM lParam)
{
	NMLISTVIEW* pLv = (NMLISTVIEW*)lParam;
	HWND hList = GetDlgItem(hDlg, IDC_FILTERDEF_FILTER_LIST);

	int iSel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
	if (iSel < 0 ) return TRUE;

	LVITEM li;
	li.mask = LVIF_PARAM;
	li.iItem = iSel;
	li.iSubItem = 0;
	ListView_GetItem(hList, &li);

	VFStream *pStream = (VFStream*)li.lParam;
	if (pStream->UpdateParamWithDialog(g_hInstance, hDlg)) {
		// update value
		TString sValue;
		if (!pStream->ToString(&sValue)) return FALSE;
		
		li.mask = LVIF_TEXT;
		li.pszText= sValue.Get();
		li.cchTextMax = _tcslen(li.pszText);
		li.iItem = iSel;
		li.iSubItem = 1;
		ListView_SetItem(hList, &li);

	}
	return TRUE;
}

/////////////////////////////////////////
// Dialog popup
/////////////////////////////////////////

DWORD FilterDefDlg::Popup(HINSTANCE hInst, HWND hParent, VFInfo *pi, BOOL bNew)
{
	PropertyTab *pages[NUM_TAB_FILTERCTL];
	FilterDlgSrcTab pgSrc(this);
	FilterDlgFilterTab pgFilter(this);

	pages[0] = &pgSrc;
	pages[1] = &pgFilter;

	pInfo = pi;

	DWORD nStartPage = bNew ? 0 : 1;
	PropertyPage pp;
	DWORD res = pp.Popup(hInst, hParent, pages, NUM_TAB_FILTERCTL, MSG_FILTERDEFDLG_TTL, MAKEINTRESOURCE(IDI_TOMBO), nStartPage);
	return res;
}
