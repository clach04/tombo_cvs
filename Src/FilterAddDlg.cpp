#include <windows.h>
#include <tchar.h>
#if defined(PLATFORM_WIN32)
#include <stdio.h>
#endif
#include "resource.h"
#include "Tombo.h"
#include "Message.h"
#include "DialogTemplate.h"
#include "VFStream.h"
#include "FilterAddDlg.h"
#include "Property.h"
#include "TString.h"

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// Regex filter
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
// ctor & dtor
////////////////////////////////////////////////////////////////

RegexFilterAddDlg::~RegexFilterAddDlg()
{
	delete pMatchStr;
}

BOOL RegexFilterAddDlg::Init(LPCTSTR pMatch, BOOL bCS, BOOL bCE, BOOL bFile, BOOL bNeg)
{
	pMatchStr = new TString();
	if (pMatchStr == NULL) return FALSE;

	if (pMatch) {
		if (!pMatchStr->Set(pMatch)) return FALSE;
		bCaseSensitive = bCS;
		bCheckEncrypt = bCE;
		bMatchFileName = bFile;
		bNegate = bNeg;
	}

	return TRUE;
}

void RegexFilterAddDlg::InitDialog(HWND hDlg)
{
	HWND hSearchStr = GetDlgItem(hDlg, IDC_FILTERDEF_REGEX_SEARCHSTR);
	LoadHistory(hSearchStr, TOMBO_SEARCHHIST_ATTR_NAME);

	if (pMatchStr->Get() != NULL) {
		SendMessage(hSearchStr, CB_INSERTSTRING, 0, (LPARAM)(pMatchStr->Get()));
		SendMessage(hSearchStr, CB_SETCURSEL, 0, 0);

		HWND hCS = GetDlgItem(hDlg, IDC_FILTERDEF_REGEX_CASESENSITIVE);
		HWND hCE = GetDlgItem(hDlg, IDC_FILTERDEF_REGEX_CRYPTMEMO);
		HWND hCF = GetDlgItem(hDlg, IDC_FILTERDEF_REGEX_FILENAME);
		HWND hNg = GetDlgItem(hDlg, IDC_FILTERDEF_REGEX_NEGATE);

		SendMessage(hCS, BM_SETCHECK, bCaseSensitive ? BST_CHECKED : BST_UNCHECKED, 0);
		SendMessage(hCE, BM_SETCHECK, bCheckEncrypt ? BST_CHECKED : BST_UNCHECKED, 0);
		SendMessage(hCF, BM_SETCHECK, bMatchFileName ? BST_CHECKED : BST_UNCHECKED, 0);
		SendMessage(hNg, BM_SETCHECK, bNegate ? BST_CHECKED : BST_UNCHECKED, 0);
	}
}

BOOL RegexFilterAddDlg::OnOK()
{
	HWND hSearchStr = GetDlgItem(hDialog, IDC_FILTERDEF_REGEX_SEARCHSTR);
	HWND hCase = GetDlgItem(hDialog, IDC_FILTERDEF_REGEX_CASESENSITIVE);
	HWND hEncrypt = GetDlgItem(hDialog, IDC_FILTERDEF_REGEX_CRYPTMEMO);
	HWND hFilteTarget = GetDlgItem(hDialog, IDC_FILTERDEF_REGEX_FILENAME);
	HWND hNegate = GetDlgItem(hDialog, IDC_FILTERDEF_REGEX_NEGATE);

	DWORD nLen = GetWindowTextLength(hSearchStr);
	if (!pMatchStr->Alloc(nLen + 1)) return FALSE;
	GetWindowText(hSearchStr, pMatchStr->Get(), nLen + 1);
	RetrieveAndSaveHistory(hSearchStr, TOMBO_SEARCHHIST_ATTR_NAME, pMatchStr->Get(), NUM_SEARCH_HISTORY);

	bCaseSensitive = SendMessage(hCase, BM_GETCHECK, 0, 0) == BST_CHECKED;
	bCheckEncrypt = SendMessage(hEncrypt, BM_GETCHECK, 0, 0) == BST_CHECKED;
	bMatchFileName = SendMessage(hFilteTarget, BM_GETCHECK, 0, 0) == BST_CHECKED;
	bNegate = SendMessage(hNegate, BM_GETCHECK, 0, 0) == BST_CHECKED;

	return TRUE;
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// Limit filter
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

void LimitFilterAddDlg::InitDialog(HWND hDlg)
{
	if (nLimit != 0) {
		TCHAR buf[32];
		HWND hNumLimit = GetDlgItem(hDlg, IDC_FILTERDEF_LIMIT_NUMLIMIT);
		_stprintf(buf, TEXT("%d"), nLimit);
		SetWindowText(hNumLimit, buf);
	}
}

BOOL LimitFilterAddDlg::OnOK()
{
	HWND hNumLimit = GetDlgItem(hDialog, IDC_FILTERDEF_LIMIT_NUMLIMIT);
	
	TCHAR buf[32];
	GetWindowText(hNumLimit, buf, 32);
	int iNum = _ttoi(buf);
	if (iNum <= 0) {
		MessageBox(hDialog, MSG_SET_POSITIVE, TOMBO_APP_NAME, MB_OK | MB_ICONWARNING);
		return FALSE;
	}
	nLimit = iNum;
	return TRUE;
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// Timestamp filter
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
BOOL TimestampFilterAddDlg::Init(DWORD nD, BOOL b)
{
	nDeltaDays = nD;
	bNewer = b;
	return TRUE;
}

void TimestampFilterAddDlg::InitDialog(HWND hDlg)
{
	HWND hNewer = GetDlgItem(hDlg, IDC_FILTERDEF_TS_NEWER);

	if (nDeltaDays == 0) {
		SendMessage(hNewer, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		HWND hDelta = GetDlgItem(hDialog, IDC_FILTERDEF_TS_DELTADAY);
		TCHAR buf[32];
		_stprintf(buf, TEXT("%d"), nDeltaDays);
		SetWindowText(hDelta, buf);
		if (bNewer) {
			SendMessage(hNewer, BM_SETCHECK, BST_CHECKED, 0);
		} else {
			HWND hOlder = GetDlgItem(hDlg, IDC_FILTERDEF_TS_OLDER);
			SendMessage(hOlder, BM_SETCHECK, BST_CHECKED, 0);
		}
	}
}

BOOL TimestampFilterAddDlg::OnOK()
{
	// get day
	HWND hDelta = GetDlgItem(hDialog, IDC_FILTERDEF_TS_DELTADAY);
	TCHAR buf[32];
	GetWindowText(hDelta, buf, 32);
	int iNum = _ttoi(buf);
	if (iNum <= 0) {
		MessageBox(hDialog, MSG_SET_POSITIVE, TOMBO_APP_NAME, MB_OK | MB_ICONWARNING);
		return FALSE;
	}
	nDeltaDays = iNum;

	// get mode
	HWND hNewer = GetDlgItem(hDialog, IDC_FILTERDEF_TS_NEWER);
	bNewer = SendMessage(hNewer, BM_GETCHECK, 0, 0) == BST_CHECKED;

	return TRUE;
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// Sort filter
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

void SortFilterAddDlg::InitDialog(HWND hDlg)
{
	HWND hSortType = GetDlgItem(hDlg, IDC_FILTERDEF_SORT_TYPE);
	SendMessage(hSortType, CB_ADDSTRING, 0, (LPARAM)MSG_STREAM_VALUE_SORT_FNAME_ASC);
	SendMessage(hSortType, CB_ADDSTRING, 0, (LPARAM)MSG_STREAM_VALUE_SORT_FNAME_DSC);
	SendMessage(hSortType, CB_ADDSTRING, 0, (LPARAM)MSG_STREAM_VALUE_SORT_LASTUPD_ASC);
	SendMessage(hSortType, CB_ADDSTRING, 0, (LPARAM)MSG_STREAM_VALUE_SORT_LASTUPD_DSC);
	SendMessage(hSortType, CB_ADDSTRING, 0, (LPARAM)MSG_STREAM_VALUE_SORT_CREATE_ASC);
	SendMessage(hSortType, CB_ADDSTRING, 0, (LPARAM)MSG_STREAM_VALUE_SORT_CREATE_DSC);

	DWORD n;
	switch (sfType) {
	case VFSortFilter::SortFunc_FileNameAsc:	n = 0; break;
	case VFSortFilter::SortFunc_FileNameDsc:	n = 1; break;
	case VFSortFilter::SortFunc_LastUpdateAsc:	n = 2; break;
	case VFSortFilter::SortFunc_LastUpdateDsc:	n = 3; break;
	case VFSortFilter::SortFunc_CreateDateAsc:	n = 4; break;
	case VFSortFilter::SortFunc_CreateDateDsc:	n = 5; break;
	default:
		n = 0;
	}

	SendMessage(hSortType, CB_SETCURSEL, n, 0);
}

#define NUM_SOFTFILTER_SORT_TYPE 6

static VFSortFilter::SortFuncType sfComboList[NUM_SOFTFILTER_SORT_TYPE] = {
	VFSortFilter::SortFunc_FileNameAsc,
	VFSortFilter::SortFunc_FileNameDsc,
	VFSortFilter::SortFunc_LastUpdateAsc,
	VFSortFilter::SortFunc_LastUpdateDsc,
	VFSortFilter::SortFunc_CreateDateAsc,
	VFSortFilter::SortFunc_CreateDateDsc
};

BOOL SortFilterAddDlg::OnOK()
{
	HWND hSortType = GetDlgItem(hDialog, IDC_FILTERDEF_SORT_TYPE);
	DWORD n = SendMessage(hSortType, CB_GETCURSEL, 0, 0);
	if (n >= NUM_SOFTFILTER_SORT_TYPE) {
		sfType = VFSortFilter::SortFunc_Unknown;
	} else {
		sfType = sfComboList[n];
	}
	return TRUE;
}