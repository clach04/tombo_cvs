#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#if defined(PLATFORM_WIN32)
#include <stdio.h>
#endif
#if defined(PLATFORM_BE500)
#include <GetDisk.h>
#endif
#include "Tombo.h"

#include "Property.h"
#include "PropertyPage.h"
#include "resource.h"
#include "FileSelector.h"
#include "UniConv.h"
#include "MemoNote.h"
#include "PasswordManager.h"
#include "Message.h"

//////////////////////////////////////////
// 定義
//////////////////////////////////////////
#define RESMSG(x) (GetString(x))

#define TOMBO_MAIN_KEY TEXT("Software\\flatfish\\Tombo")
#define TOPDIR_ATTR_NAME TEXT("TopDir")
#define FINGERPRINT_ATTR_NAME TEXT("FingerPrint")
#define PASSTIMEOUT_ATTR_NAME TEXT("PassTimeOut")
#define SELECTVIEW_FONTNAME_ATTR_NAME TEXT("SelectViewFontName")
#define SELECTVIEW_FONTSIZE_ATTR_NAME TEXT("SelectViewFontSize")
#define DETAILSVIEW_FONTNAME_ATTR_NAME TEXT("DetailsViewFontName")
#define DETAILSVIEW_FONTSIZE_ATTR_NAME TEXT("DetailsViewFontSize")
#define DETAILSVIEW_DATEFORMAT1_ATTR_NAME TEXT("DateFormat1")
#define DETAILSVIEW_DATEFORMAT2_ATTR_NAME TEXT("DateFormat2")
#define DETAILSVIEW_KEEPCARET_ATTR_NAME TEXT("KeepCaret")
#define DETAILSVIEW_TABSTOP_ATTR_NAME TEXT("Tabstop")
#define AUTOSELECT_MODE_ATTR_NAME TEXT("AutoSelectMode")
#define SINGLECLICK_MODE_ATTR_NAME TEXT("SingleClickMode")
#define APP_BUTTON1_ATTR_NAME TEXT("AppButton1")
#define APP_BUTTON2_ATTR_NAME TEXT("AppButton2")
#define APP_BUTTON3_ATTR_NAME TEXT("AppButton3")
#define APP_BUTTON4_ATTR_NAME TEXT("AppButton4")
#define APP_BUTTON5_ATTR_NAME TEXT("AppButton5")
#define USE_TWO_PANE_ATTR_NAME TEXT("UseTwoPane")
#define SWITCH_WINDOW_TITLE_ATTR_NAME TEXT("SwitchWindowTitle")
#define KEEP_TITLE_ATTR_NAME TEXT("KeepTitle")
#define PROP_FOLDER_ATTR_NAME TEXT("PropertyDir")
#define CODEPAGE_ATTR_NAME TEXT("CodePage")
#define DISABLEEXTRAACTIONBUTTON_ATTR_NAME TEXT("DisableExtraActionButton")

// saved each exit time.
#define HIDESTATUSBAR_ATTR_NAME TEXT("HideStatusBar")
#define STAYTOPMOST_ATTR_NAME TEXT("StayTopMost")
#define TOMBO_WINSIZE_ATTR_NAME TEXT("WinSize")
#define TOMBO_REBARHIST_ATTR_NAME TEXT("RebarPos")

#if defined(PLATFORM_PKTPC) || (defined(PLATFORM_BE500) && defined(TOMBO_LANG_ENGLISH))
#define PROPTAB_PAGES 6
#else
#define PROPTAB_PAGES 5
#endif

#define MAX_PASSWORD_LEN 1024

#define FONTSIZE_MIN 8
#define FONTSIZE_MAX 18
#define DEFAULT_FONTNAME TEXT("Tahoma")
#define DEFAULT_FONTSIZE 9

// デフォルトのディレクトリ
#if defined(PLATFORM_WIN32)
#define MEMO_TOP_DIR TEXT("c:\\My Documents\\Pocket_PC My Documents\\TomboRoot")
#else
#define MEMO_TOP_DIR TEXT("\\My Documents\\TomboRoot")
#endif

#if defined(PLATFORM_BE500)
#define TOMBO_ROOT_SUFFIX TEXT("\\TomboRoot")
#endif

#define MEMO_TOP_DIR_NUM_HISTORY 8

#define DEFAULTDATEFORMAT1 TEXT("%y/%M/%d")
#define DEFAULTDATEFORMAT2 TEXT("%h:%m:%s")

static BOOL CreateDirectories(LPCTSTR pDir);
static BOOL MakeFont(HFONT *phFont, LPCTSTR pName, DWORD nSize);


//////////////////////////////////////////
// レジストリ操作


static HKEY GetTomboRootKey();

// 値の設定 : 失敗したらhKeyはクローズされる

static BOOL SetSZToReg(HKEY hKey, LPCTSTR pAttr, LPCTSTR pValue);
static BOOL SetDWORDToReg(HKEY hKey, LPCTSTR pAttr, DWORD nValue);

// 値の取得 : 失敗したらhKeyはクローズされる

static LPTSTR GetMultiSZFromReg(HKEY hKey, LPCTSTR pAttr);

// Get Value : Don't close hKey when failed.
//		if function is failed, return nDefault;
static DWORD GetDWORDFromReg(HKEY hKey, LPCTSTR pAttr, DWORD nDefault);

//////////////////////////////////////////
// メッセージリソースの取得
//////////////////////////////////////////

LPCTSTR GetString(UINT nID)
{
	static TCHAR buf[MESSAGE_MAX_SIZE];
	LoadString(g_hInstance, nID, buf, MESSAGE_MAX_SIZE);
	return buf;
}

//////////////////////////////////////////
// ctor
//////////////////////////////////////////

Property::Property()
{
	_tcscpy(aTopDir, TEXT(""));
}

//////////////////////////////////////////
// フォント
//////////////////////////////////////////

HFONT Property::SelectViewFont()
{
	HFONT hFont = NULL;
	MakeFont(&hFont, aSelectViewFontName, nSelectViewFontSize);
	return hFont;
}

HFONT Property::DetailsViewFont()
{
	HFONT hFont = NULL;
	MakeFont(&hFont, aDetailsViewFontName, nDetailsViewFontSize);
	return hFont;
}

//////////////////////////////////////////
// TOMBO general property tab
//////////////////////////////////////////

class TomboPropertyTab : public PropertyTab {
protected:
	Property *pProperty;
public:
	TomboPropertyTab(Property *prop, DWORD id, DLGPROC proc, DWORD nTitleResID) : PropertyTab(id, nTitleResID, proc), pProperty(prop) {}
};

//////////////////////////////////////////
// フォルダ選択タブ
//////////////////////////////////////////

class FolderTab : public TomboPropertyTab {
public:
	FolderTab(Property *p) : 
	  TomboPropertyTab(p, IDD_PROPTAB_FOLDER,(DLGPROC)DefaultPageProc, IDS_PROPTAB_FOLDER) {}
	~FolderTab() {}
	void Init(HWND hDlg);
	BOOL Apply(HWND hDlg);
	BOOL OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
};

void FolderTab::Init(HWND hDlg)
{
	HWND hFolder = GetDlgItem(hDlg, IDC_TOPFOLDER);
	LoadHistory(hFolder, TOMBO_TOPDIRHIST_ATTR_NAME);
	if (GetWindowTextLength(hFolder) == 0) {
		if (pProperty->TopDir()) {
			SetWindowText(hFolder, pProperty->TopDir());
		}
	}

	HWND hKeepTitle = GetDlgItem(hDlg, IDC_KEEPTITLE);
	if (pProperty->KeepTitle()) {
		SendMessage(hKeepTitle, BM_SETCHECK, BST_UNCHECKED, 0);
	} else {
		SendMessage(hKeepTitle, BM_SETCHECK, BST_CHECKED, 0);
	}
}

BOOL FolderTab::Apply(HWND hDlg)
{
	HWND hKeepTitle = GetDlgItem(hDlg, IDC_KEEPTITLE);
	if (SendMessage(hKeepTitle, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pProperty->nKeepTitle = FALSE;
	} else {
		pProperty->nKeepTitle = TRUE;
	}

	TCHAR aTopPath[MAX_PATH];
	HWND hTopPath = GetDlgItem(hDlg, IDC_TOPFOLDER);
	GetWindowText(hTopPath, aTopPath, MAX_PATH);

	// 空白の除去
	LPTSTR p = aTopPath;
	while(*p == TEXT(' ') || *p == TEXT('\t')) p++;
	if (_tcslen(p) == 0) {
		SetFocus(hTopPath);
		SendMessage(hTopPath, EM_SETSEL, 0, -1);
		return FALSE;
	}

#if defined(PLATFORM_WIN32)
	if (_tcslen(p) == 3 && _istalpha(p[0]) && p[1] == TEXT(':') && p[2] == TEXT('\\')) {
		_tcscpy(pProperty->aTopDir, p);
		return TRUE;
	} else if (_tcslen(p) == 2 && _istalpha(p[0]) && p[1] == TEXT(':')) {
		p[2] = TEXT('\\');
		p[3] = TEXT('\0');
		_tcscpy(pProperty->aTopDir, p);
		return TRUE;
	} else {
		ChopFileSeparator(p);
	}
#endif
#if defined(_WIN32_WCE)
	if (_tcscmp(p, TEXT("\\")) == 0) {
		MessageBox(hDlg, MSG_ROOT_NOT_ALLOWED, TOMBO_APP_NAME, MB_ICONERROR | MB_OK);
		SetFocus(hTopPath);
		return FALSE;
	}
	ChopFileSeparator(p);
#endif
	// フォルダの存在チェック
	WIN32_FIND_DATA fd;
	HANDLE h = FindFirstFile(p, &fd);
	if (h == INVALID_HANDLE_VALUE) {
		// 存在せず
		if (MessageBox(hDlg, MSG_IS_CREATE_DIR, MSG_CREATE_DIR_TTL, MB_ICONQUESTION | MB_OKCANCEL) == IDOK) {
			if (!CreateDirectories(p)) {
				MessageBox(hDlg, MSG_MKDIR_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
				SetFocus(hTopPath);
				SendMessage(hTopPath, EM_SETSEL, 0, -1);
				return FALSE;
			}
		} else {
			SetFocus(hTopPath);
			SendMessage(hTopPath, EM_SETSEL, 0, -1);
			return FALSE;
		}
	} else {
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY == 0) {
			// ディレクトリではない
			SetFocus(hTopPath);
			SendMessage(hTopPath, EM_SETSEL, 0, -1);
			return FALSE;
		}
		FindClose(h);
	}

	// 履歴の保存
	RetrieveAndSaveHistory(hTopPath, TOMBO_TOPDIRHIST_ATTR_NAME, p, MEMO_TOP_DIR_NUM_HISTORY);

	_tcscpy(pProperty->aTopDir, p);
	return TRUE;
}

BOOL FolderTab::OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam)) {
	case IDC_FOLDERSEL:
		{
			FileSelector sel;
			if (sel.Popup(g_hInstance, hDlg, MSG_CHOOSE_FOLDER, NULL) == IDOK) {
				HWND hEdit = GetDlgItem(hDlg, IDC_TOPFOLDER);
				SetWindowText(hEdit, sel.SelectedPath());
			}
		}
		break;
	}
	return TRUE;
}

//////////////////////////////////////////
// パスワード設定タブ
//////////////////////////////////////////

class PasswordTab : public TomboPropertyTab {
public:
	PasswordTab(Property *p) : 
	  TomboPropertyTab(p, IDD_PROPTAB_PASSWORD,(DLGPROC)DefaultPageProc, IDS_PROPTAB_PASSWORD) {}
	~PasswordTab() {}
	void Init(HWND hDlg);
	BOOL Apply(HWND hDlg);
	BOOL OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
};

void PasswordTab::Init(HWND hDlg)
{
	HWND hPass1 = GetDlgItem(hDlg, IDC_PASS1);
	HWND hPass2 = GetDlgItem(hDlg, IDC_PASS2);
	HWND hDesc = GetDlgItem(hDlg, IDC_PROP_PASSWORD_DESC);
	SetWindowText(hPass1, TEXT(""));
	SetWindowText(hPass2, TEXT(""));
	SetWindowText(hDesc, RESMSG(IDS_PROPERTY_PASSWORD_DESC));
}

BOOL PasswordTab::Apply(HWND hDlg)
{
	HWND hPass1 = GetDlgItem(hDlg, IDC_PASS1);
	HWND hPass2 = GetDlgItem(hDlg, IDC_PASS2);

	if (!pProperty->bValidSum) {
		SetFocus(hPass1);
		return FALSE;
	}

	SetWindowText(hPass1, TEXT(""));
	SetWindowText(hPass2, TEXT(""));
	return TRUE;
}

static BOOL SaveFingerPrint(LPBYTE pPasswordSum, DWORD nLen)
{
	DWORD sam, res;
	HKEY hTomboRoot;

#ifdef _WIN32_WCE
	res = RegCreateKeyEx(HKEY_CURRENT_USER, TOMBO_MAIN_KEY, 0, NULL, 0,
				0, NULL, &hTomboRoot, &sam);
#else
	res = RegCreateKeyEx(HKEY_CURRENT_USER, TOMBO_MAIN_KEY, 0, NULL, REG_OPTION_NON_VOLATILE,
				KEY_ALL_ACCESS, NULL, &hTomboRoot, &sam);
#endif
	if (res != ERROR_SUCCESS) {
		SetLastError(res);
		return FALSE;
	}

	// パスワードFingerPrint
	res = RegSetValueEx(hTomboRoot, FINGERPRINT_ATTR_NAME, 0, REG_BINARY, pPasswordSum, nLen);
	if (res != ERROR_SUCCESS) {
		SetLastError(res);
		RegCloseKey(hTomboRoot);
		return FALSE;
	}

	RegCloseKey(hTomboRoot);
	return TRUE;
}

BOOL PasswordTab::OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam)) {
	case IDC_PROP_SETPASS:
		{
			if (pProperty->bValidSum) {
				if (MessageBox(hDlg, MSG_REG_PASSWD, MSG_REG_PASSWD_TTL,  MB_YESNO) == IDNO) {
					return TRUE;
				}
			}
			TCHAR aPass1[MAX_PASSWORD_LEN];
			TCHAR aPass2[MAX_PASSWORD_LEN];

			// パスワード取得
			HWND hPass1 = GetDlgItem(hDlg, IDC_PASS1);
			HWND hPass2 = GetDlgItem(hDlg, IDC_PASS2);
			GetWindowText(hPass1, aPass1, MAX_PASSWORD_LEN);
			GetWindowText(hPass2, aPass2, MAX_PASSWORD_LEN);

			// 有効性チェック
			if (_tcslen(aPass1) == 0) {
				MessageBox(NULL, MSG_PLEASE_SET_PASS, TEXT("Warning"), MB_ICONEXCLAMATION | MB_OK);
				return TRUE;
			}
			if (_tcscmp(aPass1, aPass2) != 0) {
				MessageBox(NULL, MSG_PASS_NOT_MATCH, TEXT("Warning"), MB_ICONEXCLAMATION | MB_OK);
				return TRUE;
			}

			char *pPassA = ConvUnicode2SJIS(aPass1);
			SetWindowText(hPass1, TEXT(""));
			SetWindowText(hPass2, TEXT(""));
			for (DWORD i = 0; i < MAX_PASSWORD_LEN; i++) {
				aPass1[i] = aPass2[i] = TEXT('\0');
			}

			if (pPassA == NULL) {
				MessageBox(NULL, MSG_GET_PASS_FAILED, TEXT("Error"), MB_ICONERROR | MB_OK);
				return TRUE;
			}

			if (!GetFingerPrint(pProperty->aPasswordSum, pPassA)) {
				MessageBox(NULL, MSG_GET_FP_FAILED, TEXT("Error"), MB_ICONERROR | MB_OK);
				MemoNote::WipeOutAndDelete(pPassA);
				return TRUE;
			}

			if (pProperty->bValidSum) {
				if (SaveFingerPrint(pProperty->aPasswordSum, sizeof(pProperty->aPasswordSum))) {
					// パスワードの変更反映
					MessageBox(hDlg, MSG_PW_REGED, MSG_REG_PASSWD_TTL, MB_ICONINFORMATION | MB_OK);
					pProperty->bValidSum = TRUE;
				} else {
					MessageBox(hDlg, MSG_PW_REG_FAILED, TEXT("Error"), MB_ICONERROR | MB_OK);
				}
			}
			MemoNote::WipeOutAndDelete(pPassA);
			return TRUE;
		}
		break;
	}
	return TRUE;
}

//////////////////////////////////////////
// パスワードタイムアウトタブ
//////////////////////////////////////////

class PassTimeoutTab : public TomboPropertyTab {
public:
	PassTimeoutTab(Property *p) : 
	  TomboPropertyTab(p, IDD_PROPTAB_PASS_TIMEOUT,(DLGPROC)DefaultPageProc, IDS_PROPTAB_PASS_TIMEOUT) {}
	~PassTimeoutTab() {}
	void Init(HWND hDlg);
	BOOL Apply(HWND hDlg);
};

void PassTimeoutTab::Init(HWND hDlg)
{
	HWND hTimeout = GetDlgItem(hDlg, IDC_PASS_TIMEOUT);
	TCHAR buf[64];
	wsprintf(buf, TEXT("%d"), pProperty->nPassTimeOut);
	SetWindowText(hTimeout, buf);
}

BOOL PassTimeoutTab::Apply(HWND hDlg)
{
	HWND hTimeout = GetDlgItem(hDlg, IDC_PASS_TIMEOUT);
	TCHAR buf[64];
	DWORD n;

	GetWindowText(hTimeout, buf, 64);

	n = _ttol(buf);
	if (n < 1 || n > 60) {
		SetFocus(hTimeout);
		return FALSE;
	}
	pProperty->nPassTimeOut = n;
	return TRUE;
}

//////////////////////////////////////////
// フォントタブ
//////////////////////////////////////////

class FontTab : public TomboPropertyTab {
public:
	FontTab(Property *p) :
	  TomboPropertyTab(p, IDD_PROPTAB_FONT, (DLGPROC)DefaultPageProc, IDS_PROPTAB_FONT) {}
	~FontTab() {}
	void Init(HWND hDlg);
	BOOL Apply(HWND hDlg);
	BOOL OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
};

extern "C" static int CALLBACK PropEnumFonts(ENUMLOGFONT FAR *pFont, NEWTEXTMETRIC FAR *pMetric, int iFontType, LPARAM lParam);

static void SetFontName(HDC hDC, HWND hFontName, LPCTSTR pDefaultFontName)
{
	EnumFontFamilies(hDC, NULL, (FONTENUMPROC)PropEnumFonts, (LPARAM)hFontName);
	DWORD n = SendMessage(hFontName, CB_FINDSTRINGEXACT, -1, (LPARAM)pDefaultFontName);
	if (n != CB_ERR) {
		SendMessage(hFontName, CB_SETCURSEL, n, 0);
	} else {
		SendMessage(hFontName, CB_SETCURSEL, 0, 0);
	}
}

static void SetFontSize(HWND hFontSize, DWORD nDefaultSize)
{
	TCHAR buf[5];
	BOOL bSet = FALSE;

	for (DWORD i = FONTSIZE_MIN; i <= FONTSIZE_MAX; i++) {
		wsprintf(buf, TEXT("%d"), i);
		SendMessage(hFontSize, CB_ADDSTRING, 0, (LPARAM)buf);
		if (i == nDefaultSize) {
			bSet = TRUE;
			SendMessage(hFontSize, CB_SETCURSEL, i - FONTSIZE_MIN, 0);
		}
	}
	if (!bSet) {
		SendMessage(hFontSize, CB_SETCURSEL, 0, 0);
	}
}

static void InitFontControls(HDC hDC, HWND hName, HWND hSize, HWND hDefault, LPCTSTR pPropName, DWORD nPropSize)
{
	LPCTSTR pFont;
	DWORD nSize;

	if (nPropSize == 0xFFFFFFFF) {
		pFont = DEFAULT_FONTNAME;
		nSize = DEFAULT_FONTSIZE;
		SendMessage(hDefault, BM_SETCHECK, BST_CHECKED, 0);
		EnableWindow(hName, FALSE);
		EnableWindow(hSize, FALSE);
	} else {
		SendMessage(hDefault, BM_SETCHECK, BST_UNCHECKED, 0);
		EnableWindow(hName, TRUE);
		EnableWindow(hSize, TRUE);
		pFont = pPropName;
		nSize = nPropSize;
	}
	SetFontName(hDC, hName, pFont);
	SetFontSize(hSize, nSize);
}

void FontTab::Init(HWND hDlg)
{
	HWND hSelectUseDefault = GetDlgItem(hDlg, IDC_FONT_SELECT_USEDEFAULT);
	HWND hSelectName = GetDlgItem(hDlg, IDC_FONT_SELECT_NAME);
	HWND hSelectSize = GetDlgItem(hDlg, IDC_FONT_SELECT_SIZE);

	HWND hDetailsUseDefault = GetDlgItem(hDlg, IDC_FONT_DETAILS_USEDEFAULT);
	HWND hDetailsName = GetDlgItem(hDlg, IDC_FONT_DETAILS_NAME);
	HWND hDetailsSize = GetDlgItem(hDlg, IDC_FONT_DETAILS_SIZE);

	HDC hDC = GetDC(hDlg);

	InitFontControls(hDC, hSelectName, hSelectSize, hSelectUseDefault, pProperty->aSelectViewFontName, pProperty->nSelectViewFontSize);
	InitFontControls(hDC, hDetailsName, hDetailsSize, hDetailsUseDefault, pProperty->aDetailsViewFontName, pProperty->nDetailsViewFontSize);

	ReleaseDC(hDlg, hDC);
}

static void GetFontStat(HWND hName, HWND hSize, HWND hDefault, LPTSTR pName, LPDWORD pSize)
{
	DWORD nStat = SendMessage(hDefault, BM_GETCHECK, 0, 0);
	if (nStat & BST_CHECKED) {
		_tcscpy(pName, TEXT(""));
		*pSize = 0xFFFFFFFF;
	} else {
		DWORD n;
		n = SendMessage(hName, CB_GETCURSEL, 0, 0);
		SendMessage(hName, CB_GETLBTEXT, n, (LPARAM)pName);
		*pSize = SendMessage(hSize, CB_GETCURSEL, 0, 0) + FONTSIZE_MIN;
	}
}

BOOL FontTab::Apply(HWND hDlg)
{
	HWND hSelectUseDefault = GetDlgItem(hDlg, IDC_FONT_SELECT_USEDEFAULT);
	HWND hSelectName = GetDlgItem(hDlg, IDC_FONT_SELECT_NAME);
	HWND hSelectSize = GetDlgItem(hDlg, IDC_FONT_SELECT_SIZE);
	HWND hDetailsUseDefault = GetDlgItem(hDlg, IDC_FONT_DETAILS_USEDEFAULT);
	HWND hDetailsName = GetDlgItem(hDlg, IDC_FONT_DETAILS_NAME);
	HWND hDetailsSize = GetDlgItem(hDlg, IDC_FONT_DETAILS_SIZE);

	GetFontStat(hSelectName, hSelectSize, hSelectUseDefault, pProperty->aSelectViewFontName, &(pProperty->nSelectViewFontSize));
	GetFontStat(hDetailsName, hDetailsSize, hDetailsUseDefault, pProperty->aDetailsViewFontName, &(pProperty->nDetailsViewFontSize));

	return TRUE;
}

static void ToggleDefault(HWND hName, HWND hSize, HWND hDefault)
{
	if (SendMessage(hDefault, BM_GETCHECK, 0, 0) & BST_CHECKED) {
		// デフォルトフォント
		EnableWindow(hName, FALSE);
		EnableWindow(hSize, FALSE);
	} else {
		// ユーザ定義フォント
		EnableWindow(hName, TRUE);
		EnableWindow(hSize, TRUE);
	}
}

BOOL FontTab::OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	HWND hSelectUseDefault = GetDlgItem(hDlg, IDC_FONT_SELECT_USEDEFAULT);
	HWND hSelectName = GetDlgItem(hDlg, IDC_FONT_SELECT_NAME);
	HWND hSelectSize = GetDlgItem(hDlg, IDC_FONT_SELECT_SIZE);
	HWND hDetailsUseDefault = GetDlgItem(hDlg, IDC_FONT_DETAILS_USEDEFAULT);
	HWND hDetailsName = GetDlgItem(hDlg, IDC_FONT_DETAILS_NAME);
	HWND hDetailsSize = GetDlgItem(hDlg, IDC_FONT_DETAILS_SIZE);

	switch (wParam) {
	case IDC_FONT_SELECT_USEDEFAULT:
		ToggleDefault(hSelectName, hSelectSize, hSelectUseDefault);
		break;
	case IDC_FONT_DETAILS_USEDEFAULT:
		ToggleDefault(hDetailsName, hDetailsSize, hDetailsUseDefault);
		break;
	}
	return TRUE;
}

//////////////////////////////////////////
// コンボボックスへのフォント名の挿入
//
// EnumFontFamiliesから呼び出されるコールバックハンドラ
// EnumFontFamiliesのLPARAMにはコンボボックスへのウィンドウハンドルを指定すること

int CALLBACK PropEnumFonts(ENUMLOGFONT FAR *pFont, NEWTEXTMETRIC FAR *pMetric, int iFontType, LPARAM lParam)
{
	HWND hWnd = (HWND)lParam;

	LPTSTR pFace = pFont->elfLogFont.lfFaceName;
	if (*pFace == TEXT('@')) return TRUE; // 縦書きフォントには未対応
	// if (pMetric->tmPitchAndFamily & 0x1) return TRUE;

	SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)pFace);
	return TRUE;
}

//////////////////////////////////////////
// 日付フォーマットタブ
//////////////////////////////////////////

class DateFormatTab : public TomboPropertyTab {
public:
	DateFormatTab(Property *p) :
	  TomboPropertyTab(p, IDD_PROPTAB_INSDATE, (DLGPROC)DefaultPageProc, IDS_PROPTAB_DATE) {}
	~DateFormatTab() {}
	void Init(HWND hDlg);
	BOOL Apply(HWND hDlg);
};

void DateFormatTab::Init(HWND hDlg)
{
	HWND hFormat1 = GetDlgItem(hDlg, IDC_PROP_DATEFORMAT1);
	HWND hFormat2 = GetDlgItem(hDlg, IDC_PROP_DATEFORMAT2);
	HWND hDesc = GetDlgItem(hDlg, IDC_DATEFORMAT_DESC);
	SetWindowText(hFormat1, pProperty->aDateFormat1);
	SetWindowText(hFormat2, pProperty->aDateFormat2);
	DWORD nTS = 4*4;
	SendMessage(hDesc, EM_SETTABSTOPS, 1, (LPARAM)&nTS);
	SetWindowText(hDesc, MSG_DATEFORMAT_DESC);
}

BOOL DateFormatTab::Apply(HWND hDlg)
{
	HWND hFormat1 = GetDlgItem(hDlg, IDC_PROP_DATEFORMAT1);
	HWND hFormat2 = GetDlgItem(hDlg, IDC_PROP_DATEFORMAT2);

	GetWindowText(hFormat1, pProperty->aDateFormat1, MAX_DATEFORMAT_LEN - 1);
	GetWindowText(hFormat2, pProperty->aDateFormat2, MAX_DATEFORMAT_LEN - 1);
	return TRUE;
}

//////////////////////////////////////////
// カーソル位置設定タブ
//////////////////////////////////////////

class KeepCaretTab : public TomboPropertyTab {
public:
	KeepCaretTab(Property *p) :
	  TomboPropertyTab(p, IDD_PROPTAB_KEEPCARET, (DLGPROC)DefaultPageProc, IDS_PROPTAB_KEEPCARET) {}
	~KeepCaretTab() {}
	void Init(HWND hDlg);
	BOOL Apply(HWND hDlg);
};

void KeepCaretTab::Init(HWND hDlg)
{
	HWND hWnd = GetDlgItem(hDlg, IDC_PROP_KEEPCARET);
	if (pProperty->KeepCaret()) {
		SendMessage(hWnd, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
	}

	// タブストップ
	HWND hTabWnd = GetDlgItem(hDlg, IDC_PROP_TABSTOP);
	TCHAR buf[30];
	wsprintf(buf, TEXT("%d"), pProperty->Tabstop());
	SetWindowText(hTabWnd, buf);

#if defined(PLATFORM_WIN32) || defined(PLATFORM_PKTPC)
	HWND hSwitchTitleWnd = GetDlgItem(hDlg, IDC_PROP_SWITCHTITLE);
	if (pProperty->SwitchWindowTitle()) {
		SendMessage(hSwitchTitleWnd, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(hSwitchTitleWnd, BM_SETCHECK, BST_UNCHECKED, 0);
	}
#endif
}

BOOL KeepCaretTab::Apply(HWND hDlg)
{
	HWND hWnd = GetDlgItem(hDlg, IDC_PROP_KEEPCARET);
	if (SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pProperty->nKeepCaret = TRUE;
	} else {
		pProperty->nKeepCaret = FALSE;
	}

	// タブストップ
	HWND hTabWnd = GetDlgItem(hDlg, IDC_PROP_TABSTOP);
	TCHAR buf[30];
	int n;
	GetWindowText(hTabWnd, buf, 30);

	n = _ttol(buf);
	if (n < 1) {
		SetFocus(hTabWnd);
		return FALSE;
	}
	pProperty->nTabstop = n;

#if defined(PLATFORM_WIN32) || defined(PLATFORM_PKTPC)
	HWND hSwitchWnd = GetDlgItem(hDlg, IDC_PROP_SWITCHTITLE);
	if (SendMessage(hSwitchWnd, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pProperty->nSwitchWindowTitle = TRUE;
	} else {
		pProperty->nSwitchWindowTitle = FALSE;
	}
#endif

	return TRUE;
}

//////////////////////////////////////////
// 一覧ビュー設定
//////////////////////////////////////////

#ifdef COMMENT
#if defined(PLATFORM_WIN32)
class SelectMemoTab : public TomboPropertyTab {
public:
	SelectMemoTab(Property *p) :
	  TomboPropertyTab(p, IDD_PROPTAB_SELECTMEMO, (DLGPROC)DefaultPageProc, IDS_PROPTAB_SELECTMEMO) {}
	~SelectMemoTab() {}
	void Init(HWND hDlg);
	BOOL Apply(HWND hDlg);
};

void SelectMemoTab::Init(HWND hDlg)
{
	HWND hAutoSelect = GetDlgItem(hDlg, IDC_PROP_AUTOLOAD);
	HWND hSingleClick = GetDlgItem(hDlg, IDC_PROP_SINGLECLICK);

	if (pProperty->AutoSelectMemo()) {
		SendMessage(hAutoSelect, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(hAutoSelect, BM_SETCHECK, BST_UNCHECKED, 0);
	}

	if (pProperty->SingleClickOpenMemo()) {
		SendMessage(hSingleClick, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(hSingleClick, BM_SETCHECK, BST_UNCHECKED, 0);
	}
}

BOOL SelectMemoTab::Apply(HWND hDlg)
{
	HWND hAutoSelect = GetDlgItem(hDlg, IDC_PROP_AUTOLOAD);
	HWND hSingleClick = GetDlgItem(hDlg, IDC_PROP_SINGLECLICK);

	if (SendMessage(hAutoSelect, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pProperty->nAutoSelectMemo = TRUE;
	} else {
		pProperty->nAutoSelectMemo = FALSE;
	}

	if (SendMessage(hSingleClick, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pProperty->nSingleClick = TRUE;
	} else {
		pProperty->nSingleClick = FALSE;
	}

	return TRUE;
}

#endif
#endif

//////////////////////////////////////////
// アクションボタン設定
//////////////////////////////////////////

#if defined(PLATFORM_PKTPC)
class AppButtonTab : public TomboPropertyTab {
public:
	AppButtonTab(Property *p) :
	  TomboPropertyTab(p, IDD_PROPTAB_APPBUTTON, (DLGPROC)DefaultPageProc, IDS_PROPTAB_APPBUTTON) {}
	~AppButtonTab() {}
	void Init(HWND hDlg);
	BOOL Apply(HWND hDlg);
};

void AppButtonTab::Init(HWND hDlg)
{
	HWND hAppButton1 = GetDlgItem(hDlg, IDC_PROP_APPBUTTON1);
	HWND hAppButton2 = GetDlgItem(hDlg, IDC_PROP_APPBUTTON2);
	HWND hAppButton3 = GetDlgItem(hDlg, IDC_PROP_APPBUTTON3);
	HWND hAppButton4 = GetDlgItem(hDlg, IDC_PROP_APPBUTTON4);
	HWND hAppButton5 = GetDlgItem(hDlg, IDC_PROP_APPBUTTON5);

	HWND hDisableAppButton = GetDlgItem(hDlg, IDC_PROPTAB_DISABLEACTION);

	if (pProperty->AppButton1()) {
		SendMessage(hAppButton1, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(hAppButton1, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	if (pProperty->AppButton2()) {
		SendMessage(hAppButton2, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(hAppButton2, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	if (pProperty->AppButton3()) {
		SendMessage(hAppButton3, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(hAppButton3, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	if (pProperty->AppButton4()) {
		SendMessage(hAppButton4, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(hAppButton4, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	if (pProperty->AppButton5()) {
		SendMessage(hAppButton5, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(hAppButton5, BM_SETCHECK, BST_UNCHECKED, 0);
	}

	if (pProperty->DisableExtraActionButton()) {
		SendMessage(hDisableAppButton, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(hDisableAppButton, BM_SETCHECK, BST_UNCHECKED, 0);
	}
}

BOOL AppButtonTab::Apply(HWND hDlg)
{
	HWND hAppButton1 = GetDlgItem(hDlg, IDC_PROP_APPBUTTON1);
	HWND hAppButton2 = GetDlgItem(hDlg, IDC_PROP_APPBUTTON2);
	HWND hAppButton3 = GetDlgItem(hDlg, IDC_PROP_APPBUTTON3);
	HWND hAppButton4 = GetDlgItem(hDlg, IDC_PROP_APPBUTTON4);
	HWND hAppButton5 = GetDlgItem(hDlg, IDC_PROP_APPBUTTON5);
	HWND hDisableAppButton = GetDlgItem(hDlg, IDC_PROPTAB_DISABLEACTION);

	if (SendMessage(hAppButton1, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pProperty->nAppButton1 = APPBUTTON_ACTION_ENABLE;
	} else {
		pProperty->nAppButton1 = APPBUTTON_ACTION_DISABLE;
	}
	if (SendMessage(hAppButton2, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pProperty->nAppButton2 = APPBUTTON_ACTION_ENABLE;
	} else {
		pProperty->nAppButton2 = APPBUTTON_ACTION_DISABLE;
	}
	if (SendMessage(hAppButton3, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pProperty->nAppButton3 = APPBUTTON_ACTION_ENABLE;
	} else {
		pProperty->nAppButton3 = APPBUTTON_ACTION_DISABLE;
	}
	if (SendMessage(hAppButton4, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pProperty->nAppButton4 = APPBUTTON_ACTION_ENABLE;
	} else {
		pProperty->nAppButton4 = APPBUTTON_ACTION_DISABLE;
	}
	if (SendMessage(hAppButton5, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pProperty->nAppButton5 = APPBUTTON_ACTION_ENABLE;
	} else {
		pProperty->nAppButton5 = APPBUTTON_ACTION_DISABLE;
	}

	if (SendMessage(hDisableAppButton, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pProperty->nDisableExtraActionButton = TRUE;
	} else {
		pProperty->nDisableExtraActionButton = FALSE;
	}

	return TRUE;
}

#endif

//////////////////////////////////////////
// Codepage tab
//////////////////////////////////////////
#if defined(PLATFORM_BE500) && defined(TOMBO_LANG_ENGLISH)

class CodepageTab : public TomboPropertyTab {
public:
	CodepageTab(Property *p) :
	  TomboPropertyTab(p, IDD_PROPTAB_CODEPAGE, (DLGPROC)DefaultPageProc, IDS_PROPTAB_CODEPAGE) {}
	~CodepageTab() {}
	void Init(HWND hDlg);
	BOOL Apply(HWND hDlg);
};

void CodepageTab::Init(HWND hDlg)
{
	HWND hWnd = GetDlgItem(hDlg, IDC_CODEPAGE);
	SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Default(English)"));
	SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Greek"));
	if (pProperty->nCodePage == 1253) {
		SendMessage(hWnd, CB_SETCURSEL, 1, 0);
	} else {
		SendMessage(hWnd, CB_SETCURSEL, 0, 0);
	}
}

BOOL CodepageTab::Apply(HWND hDlg)
{
	HWND hWnd = GetDlgItem(hDlg, IDC_CODEPAGE);
	if (SendMessage(hWnd, CB_GETCURSEL, 0, 0) == 1) {
		pProperty->nCodePage = 1253; // Greek
	} else {
		pProperty->nCodePage = 0; // default
	}
	return TRUE;
}
#endif

//////////////////////////////////////////
// プロパティダイアログの表示
//////////////////////////////////////////

DWORD Property::Popup(HINSTANCE hInst, HWND hWnd)
{
	PropertyTab *pages[PROPTAB_PAGES];
	FolderTab pgFolder(this);
	PassTimeoutTab pgTimeout(this);
	FontTab pgFont(this);
	DateFormatTab pgDate(this);
//	PasswordTab pgPass(this);
	KeepCaretTab pgKeepCaret(this);
#if defined(PLATFORM_WIN32)
//	SelectMemoTab pgSelectMemo(this);
#endif
#if defined(PLATFORM_PKTPC)
	AppButtonTab pgAppButton(this);
#endif
#if defined(PLATFORM_BE500) && defined(TOMBO_LANG_ENGLISH)
	CodepageTab pgCodepage(this);
#endif
	pages[0] = &pgFolder;
	pages[1] = &pgTimeout;
	pages[2] = &pgFont;
	pages[3] = &pgDate;
	pages[4] = &pgKeepCaret;
#if defined(PLATFORM_WIN32)
//	pages[5] = &pgSelectMemo;
#endif
#if defined(PLATFORM_PKTPC)
	pages[5] = &pgAppButton;
#endif
#if defined(PLATFORM_BE500) && defined(TOMBO_LANG_ENGLISH)
	pages[5] = &pgCodepage;
#endif
	PropertyPage pp;
	if (pp.Popup(hInst, hWnd, pages, PROPTAB_PAGES, RESMSG(IDS_PROPTAB_TITLE), MAKEINTRESOURCE(IDI_TOMBO)) == IDOK) {
		if (!Save()) {
			MessageBox(NULL, MSG_SAVE_DATA_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
		}
		return IDOK;
	}
	return IDCANCEL;
}

//////////////////////////////////////////
// パラメータのロード
//////////////////////////////////////////

BOOL Property::Load(BOOL *pStrict)
{
	DWORD res, sam, typ, siz;
	HKEY hTomboRoot;

#ifdef _WIN32_WCE
	res = RegCreateKeyEx(HKEY_CURRENT_USER, TOMBO_MAIN_KEY, 0, NULL, 0,
				0, NULL, &hTomboRoot, &sam);
#else
	res = RegCreateKeyEx(HKEY_CURRENT_USER, TOMBO_MAIN_KEY, 0, NULL, REG_OPTION_NON_VOLATILE,
				KEY_ALL_ACCESS, NULL, &hTomboRoot, &sam);
#endif
	if (res != ERROR_SUCCESS) {
		SetLastError(res);
		return FALSE;
	}
	*pStrict = TRUE;

	// メモトップフォルダパス
	siz = sizeof(aTopDir);
	res = RegQueryValueEx(hTomboRoot, TOPDIR_ATTR_NAME, NULL, &typ, (LPBYTE)aTopDir, &siz);
	if (res != ERROR_SUCCESS) {
#if defined(PLATFORM_BE500)
		GetUserDiskName(g_hInstance, aTopDir, MAX_PATH);
		if (_tcslen(aTopDir) + _tcslen(TOMBO_ROOT_SUFFIX) < MAX_PATH - 1) {
			_tcscat(aTopDir, TOMBO_ROOT_SUFFIX);
		}
#else
		_tcscpy(aTopDir, MEMO_TOP_DIR);
#endif
		*pStrict = FALSE;
	}

	// パスワードFingerPrint
	bValidSum = TRUE;
	siz = sizeof(aPasswordSum);
	res = RegQueryValueEx(hTomboRoot, FINGERPRINT_ATTR_NAME, NULL, &typ, aPasswordSum, &siz);
	if (res != ERROR_SUCCESS) {
		memset(aPasswordSum, 0, sizeof(aPasswordSum));
		bValidSum = FALSE;
//		*pStrict = FALSE;
	}

	// パスワードタイムアウト
	siz = sizeof(nPassTimeOut);
	res = RegQueryValueEx(hTomboRoot, PASSTIMEOUT_ATTR_NAME, NULL, &typ, (LPBYTE)&nPassTimeOut, &siz);
	if (res != ERROR_SUCCESS) {
		nPassTimeOut = 5;
		*pStrict = FALSE;
	}

	// フォント
	siz = sizeof(aSelectViewFontName);
	res = RegQueryValueEx(hTomboRoot, SELECTVIEW_FONTNAME_ATTR_NAME, NULL, &typ, (LPBYTE)aSelectViewFontName, &siz);
	if (res != ERROR_SUCCESS) {
		_tcscpy(aSelectViewFontName, TEXT(""));
	}
	siz = sizeof(nSelectViewFontSize);
	res = RegQueryValueEx(hTomboRoot, SELECTVIEW_FONTSIZE_ATTR_NAME, NULL, &typ, (LPBYTE)&nSelectViewFontSize, &siz);
	if (res != ERROR_SUCCESS) {
		nSelectViewFontSize = 0xFFFFFFFF;
	}

	siz = sizeof(aDetailsViewFontName);
	res = RegQueryValueEx(hTomboRoot, DETAILSVIEW_FONTNAME_ATTR_NAME, NULL, &typ, (LPBYTE)aDetailsViewFontName, &siz);
	if (res != ERROR_SUCCESS) {
		_tcscpy(aDetailsViewFontName, TEXT(""));
	}
	siz = sizeof(nDetailsViewFontSize);
	res = RegQueryValueEx(hTomboRoot, DETAILSVIEW_FONTSIZE_ATTR_NAME, NULL, &typ, (LPBYTE)&nDetailsViewFontSize, &siz);
	if (res != ERROR_SUCCESS) {
		nDetailsViewFontSize = 0xFFFFFFFF;
	}

	// 日付フォーマット
	siz = sizeof(aDateFormat1);
	res = RegQueryValueEx(hTomboRoot, DETAILSVIEW_DATEFORMAT1_ATTR_NAME, NULL, &typ, (LPBYTE)aDateFormat1, &siz);
	if (res != ERROR_SUCCESS) {
		_tcscpy(aDateFormat1, DEFAULTDATEFORMAT1);
	}
	siz = sizeof(aDateFormat2);
	res = RegQueryValueEx(hTomboRoot, DETAILSVIEW_DATEFORMAT2_ATTR_NAME, NULL, &typ, (LPBYTE)aDateFormat2, &siz);
	if (res != ERROR_SUCCESS) {
		_tcscpy(aDateFormat2, DEFAULTDATEFORMAT2);
	}

	// カーソル位置の保存
	siz = sizeof(nKeepCaret);
	res = RegQueryValueEx(hTomboRoot, DETAILSVIEW_KEEPCARET_ATTR_NAME, NULL, &typ, (LPBYTE)&nKeepCaret, &siz);
	if (res != ERROR_SUCCESS) {
		nKeepCaret = FALSE;
	}

	// 選択モード
	siz = sizeof(nAutoSelectMemo);
	res = RegQueryValueEx(hTomboRoot, AUTOSELECT_MODE_ATTR_NAME, NULL, &typ, (LPBYTE)&nAutoSelectMemo, &siz);
	if (res != ERROR_SUCCESS) {
		nAutoSelectMemo = TRUE;
	}
	siz = sizeof(nAutoSelectMemo);
	res = RegQueryValueEx(hTomboRoot, SINGLECLICK_MODE_ATTR_NAME, NULL, &typ, (LPBYTE)&nSingleClick, &siz);
	if (res != ERROR_SUCCESS) {
		nSingleClick = TRUE;
	}
#if defined(PLATFORM_PKTPC)
	// アプリケーションボタン
	siz = sizeof(nAppButton1);
	res = RegQueryValueEx(hTomboRoot, APP_BUTTON1_ATTR_NAME, NULL, &typ, (LPBYTE)&nAppButton1, &siz);
	if (res != ERROR_SUCCESS) {
		nAppButton1 = APPBUTTON_ACTION_DISABLE;
	}
	siz = sizeof(nAppButton2);
	res = RegQueryValueEx(hTomboRoot, APP_BUTTON2_ATTR_NAME, NULL, &typ, (LPBYTE)&nAppButton2, &siz);
	if (res != ERROR_SUCCESS) {
		nAppButton2 = APPBUTTON_ACTION_DISABLE;
	}
	siz = sizeof(nAppButton3);
	res = RegQueryValueEx(hTomboRoot, APP_BUTTON3_ATTR_NAME, NULL, &typ, (LPBYTE)&nAppButton3, &siz);
	if (res != ERROR_SUCCESS) {
		nAppButton3 = APPBUTTON_ACTION_DISABLE;
	}
	siz = sizeof(nAppButton4);
	res = RegQueryValueEx(hTomboRoot, APP_BUTTON4_ATTR_NAME, NULL, &typ, (LPBYTE)&nAppButton4, &siz);
	if (res != ERROR_SUCCESS) {
		nAppButton4 = APPBUTTON_ACTION_DISABLE;
	}
	siz = sizeof(nAppButton5);
	res = RegQueryValueEx(hTomboRoot, APP_BUTTON5_ATTR_NAME, NULL, &typ, (LPBYTE)&nAppButton5, &siz);
	if (res != ERROR_SUCCESS) {
		nAppButton5 = APPBUTTON_ACTION_DISABLE;
	}
#endif

	// タブストップ
	siz = sizeof(nTabstop);
	res = RegQueryValueEx(hTomboRoot, DETAILSVIEW_TABSTOP_ATTR_NAME, NULL, &typ, (LPBYTE)&nTabstop, &siz);
	if (res != ERROR_SUCCESS) {
		nTabstop = 8;
	}

	// ペイン切り替え
	siz = sizeof(nUseTwoPane);
	res = RegQueryValueEx(hTomboRoot, USE_TWO_PANE_ATTR_NAME, NULL, &typ, (LPBYTE)&nUseTwoPane, &siz);
	if (res != ERROR_SUCCESS) {
#if defined(PLATFORM_HPC) || defined(PLATFORM_WIN32)
		nUseTwoPane = TRUE;
#else
		nUseTwoPane = FALSE;
#endif
	}

#if defined(PLATFORM_WIN32) || defined(PLATFORM_PKTPC)
	siz = sizeof(nSwitchWindowTitle);
	res = RegQueryValueEx(hTomboRoot, SWITCH_WINDOW_TITLE_ATTR_NAME, NULL, &typ, (LPBYTE)&nSwitchWindowTitle, &siz);
	if (res != ERROR_SUCCESS) {
		nSwitchWindowTitle = TRUE;
	}
#else
	nSwitchWindowTitle = FALSE;
#endif

	siz = sizeof(nKeepTitle);
	res = RegQueryValueEx(hTomboRoot, KEEP_TITLE_ATTR_NAME, NULL, &typ, (LPBYTE)&nKeepTitle, &siz);
	if (res != ERROR_SUCCESS) {
		nKeepTitle = FALSE;
	}

	// Property folder
	siz = sizeof(aPropDir);
	res = RegQueryValueEx(hTomboRoot, PROP_FOLDER_ATTR_NAME, NULL, &typ, (LPBYTE)aPropDir, &siz);
	if (res != ERROR_SUCCESS) {
		aPropDir[0] = TEXT('\0');
	}

#if defined(PLATFORM_BE500)
	// Code page
	siz = sizeof(nCodePage);
	res = RegQueryValueEx(hTomboRoot, CODEPAGE_ATTR_NAME, NULL, &typ, (LPBYTE)&nCodePage, &siz);
	if (res != ERROR_SUCCESS) {
		nCodePage = 0;
	}
#endif

#if defined(PLATFORM_PKTPC)
	// Disable open/close notes when action button pushed
	siz = sizeof(nDisableExtraActionButton);
	res = RegQueryValueEx(hTomboRoot, DISABLEEXTRAACTIONBUTTON_ATTR_NAME, NULL, &typ, (LPBYTE)&nDisableExtraActionButton, &siz);
	if (res != ERROR_SUCCESS) {
		nDisableExtraActionButton = 0;
	}
#endif

#if defined(PLATFORM_HPC) || defined(PLATFORM_WIN32)
	siz = sizeof(nHideStatusBar);
	res = RegQueryValueEx(hTomboRoot, HIDESTATUSBAR_ATTR_NAME, NULL, &typ, (LPBYTE)&nHideStatusBar, &siz);
	if (res != ERROR_SUCCESS) {
		nHideStatusBar = 0;
	}
#endif

#if defined(PLATFORM_WIN32)
	nTopMost = GetDWORDFromReg(hTomboRoot, STAYTOPMOST_ATTR_NAME, 0);
#endif
	
	RegCloseKey(hTomboRoot);
	return TRUE;
}

//////////////////////////////////////////
// パラメータのストア
//////////////////////////////////////////

BOOL Property::Save()
{
	DWORD sam, res;
	HKEY hTomboRoot;

#ifdef _WIN32_WCE
	res = RegCreateKeyEx(HKEY_CURRENT_USER, TOMBO_MAIN_KEY, 0, NULL, 0,
				0, NULL, &hTomboRoot, &sam);
#else
	res = RegCreateKeyEx(HKEY_CURRENT_USER, TOMBO_MAIN_KEY, 0, NULL, REG_OPTION_NON_VOLATILE,
				KEY_ALL_ACCESS, NULL, &hTomboRoot, &sam);
#endif
	if (res != ERROR_SUCCESS) {
		SetLastError(res);
		return FALSE;
	}

	// メモトップフォルダパス
	if (!SetSZToReg(hTomboRoot, TOPDIR_ATTR_NAME, aTopDir)) return FALSE;

	// パスワードFingerPrint
	if (bValidSum) {
		res = RegSetValueEx(hTomboRoot, FINGERPRINT_ATTR_NAME, 0, REG_BINARY, aPasswordSum, sizeof(aPasswordSum));
		if (res != ERROR_SUCCESS) {
			SetLastError(res);
			RegCloseKey(hTomboRoot);
			return FALSE;
		}
	}

	// パスワードタイムアウト
	if (!SetDWORDToReg(hTomboRoot, PASSTIMEOUT_ATTR_NAME, nPassTimeOut)) return FALSE;

	// フォント
	if (!SetSZToReg(hTomboRoot, SELECTVIEW_FONTNAME_ATTR_NAME, aSelectViewFontName)) return FALSE;
	if (!SetSZToReg(hTomboRoot, DETAILSVIEW_FONTNAME_ATTR_NAME, aDetailsViewFontName)) return FALSE;
	if (!SetDWORDToReg(hTomboRoot, SELECTVIEW_FONTSIZE_ATTR_NAME, nSelectViewFontSize)) return FALSE;
	if (!SetDWORDToReg(hTomboRoot, DETAILSVIEW_FONTSIZE_ATTR_NAME, nDetailsViewFontSize)) return FALSE;

	// 日付フォーマット
	if (!SetSZToReg(hTomboRoot, DETAILSVIEW_DATEFORMAT1_ATTR_NAME, aDateFormat1)) return FALSE;
	if (!SetSZToReg(hTomboRoot, DETAILSVIEW_DATEFORMAT2_ATTR_NAME, aDateFormat2)) return FALSE;

	// カーソル位置の保持
	if (!SetDWORDToReg(hTomboRoot, DETAILSVIEW_KEEPCARET_ATTR_NAME, nKeepCaret)) return FALSE;

	// 選択モード
	if (!SetDWORDToReg(hTomboRoot, AUTOSELECT_MODE_ATTR_NAME, nAutoSelectMemo)) return FALSE;
	if (!SetDWORDToReg(hTomboRoot, SINGLECLICK_MODE_ATTR_NAME, nSingleClick)) return FALSE;

#if defined(PLATFORM_PKTPC)
	// アプリケーションボタン
	if (!SetDWORDToReg(hTomboRoot, APP_BUTTON1_ATTR_NAME, nAppButton1)) return FALSE;
	if (!SetDWORDToReg(hTomboRoot, APP_BUTTON2_ATTR_NAME, nAppButton2)) return FALSE;
	if (!SetDWORDToReg(hTomboRoot, APP_BUTTON3_ATTR_NAME, nAppButton3)) return FALSE;
	if (!SetDWORDToReg(hTomboRoot, APP_BUTTON4_ATTR_NAME, nAppButton4)) return FALSE;
	if (!SetDWORDToReg(hTomboRoot, APP_BUTTON5_ATTR_NAME, nAppButton5)) return FALSE;
#endif

	if (!SetDWORDToReg(hTomboRoot, DETAILSVIEW_TABSTOP_ATTR_NAME, nTabstop)) return FALSE;
	if (!SetDWORDToReg(hTomboRoot, USE_TWO_PANE_ATTR_NAME, nUseTwoPane)) return FALSE;

#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WIN32)
	if (!SetDWORDToReg(hTomboRoot, SWITCH_WINDOW_TITLE_ATTR_NAME, nSwitchWindowTitle)) return FALSE;
#endif

	if (!SetDWORDToReg(hTomboRoot, KEEP_TITLE_ATTR_NAME, nKeepTitle)) return FALSE;

	// Property dir
	if (_tcslen(aPropDir) > 0) {
		if (!SetSZToReg(hTomboRoot, PROP_FOLDER_ATTR_NAME, aPropDir)) return FALSE;
	}

#if defined(PLATFORM_BE500)
	if (!SetDWORDToReg(hTomboRoot, CODEPAGE_ATTR_NAME, nCodePage)) return FALSE;
#endif

#if defined(PLATFORM_PKTPC)
	if (!SetDWORDToReg(hTomboRoot, DISABLEEXTRAACTIONBUTTON_ATTR_NAME, nDisableExtraActionButton)) return FALSE;
#endif
#if defined(PLATFORM_HPC) || defined(PLATFORM_WIN32)
	if (!SetDWORDToReg(hTomboRoot, HIDESTATUSBAR_ATTR_NAME, nHideStatusBar)) return FALSE;
#endif


#if defined(PLATFORM_BE500)
	CGDFlushRegistry();
#endif

	return TRUE;
}

//////////////////////////////////////////
// ディレクトリ関連
//////////////////////////////////////////

// ディレクトリがなかったら掘る
static BOOL CheckCreateDir(LPCTSTR pDir)
{
	WIN32_FIND_DATA wfd;
	HANDLE h = FindFirstFile(pDir, &wfd);
	if (h == INVALID_HANDLE_VALUE) {
		return CreateDirectory(pDir, NULL);
	}
	FindClose(h);
	if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		return TRUE;
	} else {
		SetLastError(ERROR_FILE_EXISTS);
		return FALSE;
	}
}

// 階層的にディレクトリを掘ってゆく
static BOOL CreateDirectories(LPCTSTR pDir)
{
	LPCTSTR p = pDir;
	TCHAR aPath[MAX_PATH];
	LPTSTR q = aPath;

#if defined(PLATFORM_WIN32)
	if (isalpha(*p) && *(p+1) == ':' && *(p+2) == '\\') {
		// D:\Path
		*q++ = *p++;
		*q++ = *p++;
		*q++ = *p++;
	} else if (*p == '\\' && *(p+1) == '\\') {
		// \\server\share\Path
		*q++ = *p++;
		*q++ = *p++;

		// server\ 
		while(*p) {
			if (*p == '\\') {
				*q++ = *p++;
				break;
			}
			if (iskanji(*p)) {
				*q++ = *p++;
			}
			*q++ = *p++;
		}
		// share\ 
		while(*p) {
			if (*p == '\\') {
				*q++ = *p++;
				break;
			}
			if (iskanji(*p)) {
				*q++ = *p++;
			}
			*q++ = *p++;
		}
	} else {
		return FALSE;
	}
#else
	// \\ 
	if (*p != TEXT('\\')) {
		return FALSE;
	}
	*q++ = *p++;
#endif

	// ディレクトリを掘り進む
	while(*p) {
		if (*p == TEXT('\\')) {
			*q = TEXT('\0');
			if (!CheckCreateDir(aPath)) return FALSE;
		}
		*q++ = *p++;
	}
	*q = TEXT('\0');
	return CheckCreateDir(aPath);
}

//////////////////////////////////////////
// レジストリ関連
//////////////////////////////////////////

static BOOL SetSZToReg(HKEY hKey, LPCTSTR pAttr, LPCTSTR pValue)
{
	DWORD res;
	res = RegSetValueEx(hKey, pAttr, 0, REG_SZ, (LPBYTE)pValue, (_tcslen(pValue) + 1)*sizeof(TCHAR));
	if (res != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		SetLastError(res);
		return FALSE;
	}
	return TRUE;
}

static BOOL SetDWORDToReg(HKEY hKey, LPCTSTR pAttr, DWORD nValue)
{
	DWORD res;
	res = RegSetValueEx(hKey, pAttr, 0, REG_DWORD, (LPBYTE)&nValue, sizeof(DWORD));
	if (res != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		SetLastError(res);
		return FALSE;
	}
	return TRUE;
}

static DWORD GetDWORDFromReg(HKEY hKey, LPCTSTR pAttr, DWORD nDefault)
{
	DWORD siz = sizeof(DWORD);
	DWORD typ;
	DWORD nValue;
	DWORD res = RegQueryValueEx(hKey, pAttr, NULL, &typ, (LPBYTE)&nValue, &siz);
	if (res != ERROR_SUCCESS) {
		return nDefault;
	}
	return nValue;
}



static LPTSTR GetMultiSZFromReg(HKEY hKey, LPCTSTR pAttr, LPDWORD pSize)
{
	DWORD res, siz, typ;
	res = RegQueryValueEx(hKey, pAttr, NULL, &typ, NULL, &siz);
	if (res != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		SetLastError(res);
		return NULL;
	}
	*pSize = siz;

	LPTSTR pBuf;
	pBuf = new TCHAR[siz / sizeof(TCHAR)];
	if (pBuf == NULL) {
		RegCloseKey(hKey);
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	res = RegQueryValueEx(hKey, pAttr, NULL, &typ, (LPBYTE)pBuf, &siz);
	if (res != ERROR_SUCCESS) {
		delete[] pBuf;
		RegCloseKey(hKey);
		SetLastError(res);
		return NULL;
	}
	return pBuf;
}

static BOOL SetMultiSZToReg(HKEY hKey, LPCTSTR pAttr, LPCTSTR pValue, DWORD nSize)
{
	DWORD res;
	DWORD typ;
#if defined(PLATFORM_WIN32)
	typ = REG_MULTI_SZ;
#else
	typ = REG_BINARY;
#endif
	res = RegSetValueEx(hKey, pAttr, 0, typ, (LPBYTE)pValue, nSize);
	if (res != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		SetLastError(res);
		return FALSE;
	}
	return TRUE;
}

///////////////////////////////////////////////////
// フォントの取得
///////////////////////////////////////////////////

static BOOL MakeFont(HFONT *phFont, LPCTSTR pName, DWORD nSize)
{
	if (nSize == 0xFFFFFFFF) {
		*phFont = NULL;
		return TRUE;
	}
	LOGFONT lf;
	lf.lfHeight = nSize;
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = FW_NORMAL;
	lf.lfItalic = FALSE;
	lf.lfUnderline = FALSE;
	lf.lfStrikeOut = FALSE;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH;
	_tcscpy(lf.lfFaceName, pName);
	*phFont = CreateFontIndirect(&lf);
	return *phFont != NULL;
}

///////////////////////////////////////////////////
// ウィンドウサイズ保存
///////////////////////////////////////////////////

BOOL Property::SaveWinSize(LPRECT pWinRect, WORD nSelectViewWidth)
{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	DWORD sam, res;
	HKEY hTomboRoot;

	TCHAR buf[1024];

	res = RegCreateKeyEx(HKEY_CURRENT_USER, TOMBO_MAIN_KEY, 0, NULL, REG_OPTION_NON_VOLATILE,
				KEY_ALL_ACCESS, NULL, &hTomboRoot, &sam);
	if (res != ERROR_SUCCESS) {
		SetLastError(res);
		return FALSE;
	}

	wsprintf(buf, TEXT("%d,%d,%d,%d,%d"), 
		pWinRect->left, pWinRect->top, 
		pWinRect->right, pWinRect->bottom, 
		nSelectViewWidth);

	if (!SetSZToReg(hTomboRoot, TOMBO_WINSIZE_ATTR_NAME, buf)) {
		return FALSE;
	}
	RegCloseKey(hTomboRoot);
#endif
	return TRUE;
}

///////////////////////////////////////////////////
// ウィンドウサイズ取得
///////////////////////////////////////////////////

BOOL Property::GetWinSize(LPRECT pWinRect, LPWORD pSelectViewWidth)
{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	HKEY hTomboRoot = GetTomboRootKey();
	if (!hTomboRoot) return FALSE;

	DWORD res, typ, siz;
	TCHAR buf[1024];

	siz = 1024;
	res = RegQueryValueEx(hTomboRoot, TOMBO_WINSIZE_ATTR_NAME, NULL, &typ, (LPBYTE)buf, &siz);
	if (res != ERROR_SUCCESS || typ != REG_SZ) {
		SetLastError(res);
		RegCloseKey(hTomboRoot);
		return FALSE;
	}
#if defined(PLATFORM_WIN32)
	if (sscanf(buf, TEXT("%d,%d,%d,%d,%d"),
#endif
#if defined(PLATFORM_HPC)
	if (swscanf(buf, TEXT("%d,%d,%d,%d,%d"),
#endif
			&(pWinRect->left), &(pWinRect->top), &(pWinRect->right), &(pWinRect->bottom),
			pSelectViewWidth) != 5) {
		SetLastError(ERROR_INVALID_DATA);
		RegCloseKey(hTomboRoot);
		return FALSE;
	}
	RegCloseKey(hTomboRoot);
#endif
	return TRUE;
}

///////////////////////////////////////////////////
// ペイン切り替え
///////////////////////////////////////////////////

void Property::SetUseTwoPane(BOOL bPane) 
{ 
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	nUseTwoPane = bPane;
	
	HKEY hTomboRoot = GetTomboRootKey();
	if (!hTomboRoot) return;

	if (!SetDWORDToReg(hTomboRoot, USE_TWO_PANE_ATTR_NAME, nUseTwoPane)) return;
	RegCloseKey(hTomboRoot);
#endif
}

///////////////////////////////////////////////////
// ルートキーの取得
///////////////////////////////////////////////////

static HKEY GetTomboRootKey()
{
	DWORD res, sam;
	HKEY hTomboRoot;

#ifdef _WIN32_WCE
	res = RegCreateKeyEx(HKEY_CURRENT_USER, TOMBO_MAIN_KEY, 0, NULL, 0,
				0, NULL, &hTomboRoot, &sam);
#else
	res = RegCreateKeyEx(HKEY_CURRENT_USER, TOMBO_MAIN_KEY, 0, NULL, REG_OPTION_NON_VOLATILE,
				KEY_ALL_ACCESS, NULL, &hTomboRoot, &sam);
#endif
	if (res != ERROR_SUCCESS) {
		SetLastError(res);
		return NULL;
	}
	return hTomboRoot;
}

///////////////////////////////////////////////////
// 検索履歴
///////////////////////////////////////////////////

LPTSTR LoadStringHistory(LPCTSTR pAttrName)
{
	HKEY hTomboRoot = GetTomboRootKey();
	if (!hTomboRoot) return NULL;
	DWORD nSize;
	LPTSTR p = GetMultiSZFromReg(hTomboRoot, pAttrName, &nSize);
	if (p) RegCloseKey(hTomboRoot);
	return p;
}

BOOL StoreStringHistory(LPCTSTR pAttrName, LPCTSTR pHistString, DWORD nSize)
{
	HKEY hTomboRoot = GetTomboRootKey();
	if (!hTomboRoot) return FALSE;
	if (SetMultiSZToReg(hTomboRoot, pAttrName, pHistString, nSize)) {
		RegCloseKey(hTomboRoot);
		return TRUE;
	} else {
		return FALSE;
	}
}

////////////////////////////////////////////////////////////////
// 履歴の呼び出し
////////////////////////////////////////////////////////////////

BOOL LoadHistory(HWND hCombo, LPCTSTR pAttrName)
{
	LPTSTR p = LoadStringHistory(pAttrName);
	if (p) {
		LPTSTR q = p;
		while(*q) {
			SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)q);
			q += _tcslen(q) + 1;
		}
		delete [] p;
	}
	SendMessage(hCombo, CB_SETCURSEL, 0, 0);
	return TRUE;
}


////////////////////////////////////////////////////////////////
// 履歴の保存
////////////////////////////////////////////////////////////////

BOOL RetrieveAndSaveHistory(HWND hCombo, LPCTSTR pAttrName, LPCTSTR pSelValue, DWORD nSave)
{
	DWORD nItems = SendMessage(hCombo, CB_GETCOUNT, 0, 0);
	if (nItems > nSave) nItems = nSave;

	DWORD i;
	DWORD nMatch = SendMessage(hCombo, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)pSelValue);

	// 必要領域のカウント
	DWORD nBufLen = _tcslen(pSelValue) + 1;
	DWORD nLen;
	for (i = 0; i < nItems; i++) {
		if (i == nMatch) continue;

		nLen = SendMessage(hCombo, CB_GETLBTEXTLEN, i, 0);
		if (nLen == CB_ERR) return FALSE;
		nBufLen += nLen + 1;
	}
	nBufLen++;

	// 領域の確保
	LPTSTR p = new TCHAR[nBufLen];
	if (!p) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}

	// 保存用バッファ生成
	LPTSTR q = p;
	_tcscpy(q, pSelValue);
	q += _tcslen(pSelValue) + 1;
	for (i = 0; i < nItems; i++) {
		if (i == nMatch) continue;
		SendMessage(hCombo, CB_GETLBTEXT, i, (LPARAM)q);
		q += _tcslen(q) + 1;
	}
	*q = TEXT('\0');

	BOOL bResult = StoreStringHistory(pAttrName, p, nBufLen * sizeof(TCHAR));
	delete [] p; 

	return bResult;
}

///////////////////////////////////////////////////
// Save statusbar info
///////////////////////////////////////////////////

BOOL Property::SaveStatusBarStat()
{
#if defined(PLATFORM_HPC) || defined(PLATFORM_WIN32)
	HKEY hTomboRoot = GetTomboRootKey();
	if (!hTomboRoot) return FALSE;

	if (!SetDWORDToReg(hTomboRoot, HIDESTATUSBAR_ATTR_NAME, nHideStatusBar)) return FALSE;

	RegCloseKey(hTomboRoot);
#endif
	return TRUE;
}

///////////////////////////////////////////////////
// Save topmost stat
///////////////////////////////////////////////////

BOOL Property::SaveTopMostStat()
{
#if defined(PLATFORM_WIN32)
	HKEY hTomboRoot = GetTomboRootKey();
	if (!hTomboRoot) return FALSE;

	if (!SetDWORDToReg(hTomboRoot, STAYTOPMOST_ATTR_NAME, nTopMost)) return FALSE;

	RegCloseKey(hTomboRoot);
#endif
	return TRUE;
}

///////////////////////////////////////////////////
// Rebar位置保持
///////////////////////////////////////////////////
#if defined(PLATFORM_HPC)

BOOL SetCommandbarInfo(LPCOMMANDBANDSRESTOREINFO p, DWORD n)
{
	HKEY hTomboRoot = GetTomboRootKey();
	if (!hTomboRoot) return FALSE;

	DWORD res = RegSetValueEx(hTomboRoot, TOMBO_REBARHIST_ATTR_NAME, 0, 
					REG_BINARY, (LPBYTE)p, sizeof(COMMANDBANDSRESTOREINFO)*n);
	RegCloseKey(hTomboRoot);
	if (res != ERROR_SUCCESS) {
		SetLastError(res);
		return FALSE;
	} else {
		return TRUE;
	}
}

BOOL GetCommandbarInfo(LPCOMMANDBANDSRESTOREINFO p, DWORD n)
{
	HKEY hTomboRoot = GetTomboRootKey();
	if (!hTomboRoot) return FALSE;

	DWORD typ;
	DWORD siz = n * sizeof(COMMANDBANDSRESTOREINFO);

	DWORD res = RegQueryValueEx(hTomboRoot, TOMBO_REBARHIST_ATTR_NAME, 0, 
						&typ, (LPBYTE)p, &siz);

	if (siz != n * sizeof(COMMANDBANDSRESTOREINFO)) {
		res = ERROR_INVALID_DATA;
	}
	RegCloseKey(hTomboRoot);
	if (res != ERROR_SUCCESS) {
		SetLastError(res);
		return FALSE;
	} else {
		return TRUE;
	}
}


#endif