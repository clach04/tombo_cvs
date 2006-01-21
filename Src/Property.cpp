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
#include "resource.h"
#include "FileSelector.h"
#include "UniConv.h"
#include "PasswordManager.h"
#include "Message.h"
#include "DialogTemplate.h"
#include "TString.h"
#include "PropertyPage.h"
#include "TomboPropertyTab.h"

//////////////////////////////////////////
// Attribute definitions
//////////////////////////////////////////
#define RESMSG(x) (GetString(x))

#define TOMBO_MAIN_KEY TEXT("Software\\flatfish\\Tombo")
#define TOPDIR_ATTR_NAME TEXT("TopDir")
#define FINGERPRINT_ATTR_NAME TEXT("FingerPrint")
#define PASSTIMEOUT_ATTR_NAME TEXT("PassTimeOut")
#define SELECTVIEW_FONTNAME_ATTR_NAME TEXT("SelectViewFontName")
#define SELECTVIEW_FONTSIZE_ATTR_NAME TEXT("SelectViewFontSize")
#define SELECTVIEW_FONTQUALITY_ATTR_NAME TEXT("SelectViewFontQuality")
#define DETAILSVIEW_FONTNAME_ATTR_NAME TEXT("DetailsViewFontName")
#define DETAILSVIEW_FONTSIZE_ATTR_NAME TEXT("DetailsViewFontSize")
#define DETAILSVIEW_FONTQUALITY_ATTR_NAME TEXT("DetailsViewFontQuality")
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
#define SIPSIZE_DELTA_ATTR_NAME TEXT("SipSizeDelta")
#define USE_TWO_PANE_ATTR_NAME TEXT("UseTwoPane")
#define SWITCH_WINDOW_TITLE_ATTR_NAME TEXT("SwitchWindowTitle")
#define KEEP_TITLE_ATTR_NAME TEXT("KeepTitle")
#define PROP_FOLDER_ATTR_NAME TEXT("PropertyDir")
#define CODEPAGE_ATTR_NAME TEXT("CodePage")
#define DISABLEEXTRAACTIONBUTTON_ATTR_NAME TEXT("DisableExtraActionButton")
#define WRAPTEXT_ATTR_NAME TEXT("WrapText")
#define OPENREADONLY_ATTR_NAME TEXT("OpenReadOnly")
#define DEFAULTNOTE_ATTR_NAME TEXT("DefaultNote")
#define DISABLESAVEDLG_ATTR_NAME TEXT("DisableSaveDlg")
#define USEASSOC_ATTR_NAME TEXT("UseSoftwareAssoc")
#define EXTAPP1_ATTR_NAME TEXT("ExtApp1")
#define EXTAPP2_ATTR_NAME TEXT("ExtApp2")
#define SAFEFILENAME_ATTR_NAME TEXT("UseSafeFileName")
#define USEYAE_ATTR_NAME TEXT("UseYAEdit")

// saved each exit time.
#define HIDESTATUSBAR_ATTR_NAME TEXT("HideStatusBar")
#define STAYTOPMOST_ATTR_NAME TEXT("StayTopMost")
#define TOMBO_WINSIZE_ATTR_NAME TEXT("WinSize")
#define TOMBO_WINSIZE_ATTR_NAME2 TEXT("WinSize2")
#define TOMBO_WINSIZE_ATTR_NAME3 TEXT("WinSize3")
#define TOMBO_REBARHIST_ATTR_NAME TEXT("RebarPos")
#define HIDEREBAR_ATTR_NAME TEXT("HideRebar")

#define BOOKMARK_ATTR_NAME TEXT("BookMark")

#if defined(PLATFORM_PKTPC) || (defined(PLATFORM_BE500) && defined(TOMBO_LANG_ENGLISH))
#if defined(PLATFORM_PKTPC)
#define PROPTAB_PAGES 9
#else
#define PROPTAB_PAGES 8
#endif
#else
#define PROPTAB_PAGES 7
#endif

#define MAX_PASSWORD_LEN 1024

// Default dirs
#if defined(PLATFORM_WIN32)
#define MEMO_TOP_DIR TEXT("c:\\My Documents\\Pocket_PC My Documents\\TomboRoot")
#else
#define MEMO_TOP_DIR TEXT("\\My Documents\\TomboRoot")
#endif

#if defined(PLATFORM_BE500)
#define TOMBO_ROOT_SUFFIX TEXT("\\TomboRoot")
#endif

#define DEFAULTDATEFORMAT1 TEXT("%y/%M/%d")
#define DEFAULTDATEFORMAT2 TEXT("%h:%m:%s")

static BOOL MakeFont(HFONT *phFont, LPCTSTR pName, DWORD nSize, BYTE bQuality);

//////////////////////////////////////////
// レジストリ操作

static HKEY GetTomboRootKey();

// 値の設定 : 失敗したらhKeyはクローズされる

static BOOL SetSZToReg(HKEY hKey, LPCTSTR pAttr, LPCTSTR pValue);
static BOOL SetDWORDToReg(HKEY hKey, LPCTSTR pAttr, DWORD nValue);
static BOOL SetMultiSZToReg(HKEY hKey, LPCTSTR pAttr, LPCTSTR pValue, DWORD nSize);

// Get Value : Don't close hKey when failed.
//		if function is failed, return nDefault;
static DWORD GetDWORDFromReg(HKEY hKey, LPCTSTR pAttr, DWORD nDefault);
static LPTSTR GetMultiSZFromReg(HKEY hKey, LPCTSTR pAttr);

static DWORD CountMultiSZLen(LPCTSTR pData);

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

Property::Property() : pDefaultTopDir(NULL), pBookMark(NULL), pSearchHistory(NULL), pTopDirHistory(NULL), pWinSize(NULL)
#if defined(PLATFORM_HPC)
	,pCmdBarInfo(NULL)
#endif
{
	_tcscpy(aTopDir, TEXT(""));
	_tcscpy(aDefaultNote, TEXT(""));
}

Property::~Property()
{
	delete [] pDefaultTopDir;
	delete [] pBookMark;
	delete [] pSearchHistory;
	delete [] pTopDirHistory;
	delete [] pWinSize;
#if defined(PLATFORM_HPC)
	delete [] pCmdBarInfo;
#endif
}

//////////////////////////////////////////
// フォント
//////////////////////////////////////////

HFONT Property::SelectViewFont()
{
	HFONT hFont = NULL;
	MakeFont(&hFont, aSelectViewFontName, nSelectViewFontSize, bSelectViewFontQuality);
	return hFont;
}

HFONT Property::DetailsViewFont()
{
	HFONT hFont = NULL;
	MakeFont(&hFont, aDetailsViewFontName, nDetailsViewFontSize, bDetailsViewFontQuality);
	return hFont;
}

//////////////////////////////////////////
// Popup property dialog
//////////////////////////////////////////

DWORD Property::Popup(HINSTANCE hInst, HWND hWnd, LPCTSTR pSelPath)
{
	PropertyTab *pages[PROPTAB_PAGES];
	FolderTab pgFolder(this);
	DefaultNoteTab pgDefNote(this, pSelPath);
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
	SipTab pgSip(this);
#endif
#if defined(PLATFORM_BE500) && defined(TOMBO_LANG_ENGLISH)
	CodepageTab pgCodepage(this);
#endif
#if !defined(PLATFORM_PSPC)
	ExtAppTab pgExtApp(this);
#endif

	DWORD n = 0;

	pages[n++] = &pgFolder;
	pages[n++] = &pgDefNote;
	pages[n++] = &pgTimeout;
	pages[n++] = &pgFont;
	pages[n++] = &pgDate;
	pages[n++] = &pgKeepCaret;
#if !defined(PLATFORM_PSPC)
	pages[n++] = &pgExtApp;
#endif
#if defined(PLATFORM_PKTPC)
	pages[n++] = &pgAppButton;
	pages[n++] = &pgSip;
#endif
#if defined(PLATFORM_BE500) && defined(TOMBO_LANG_ENGLISH)
	pages[n++] = &pgCodepage;
#endif

	PropertyPage pp;
	if (pp.Popup(hInst, hWnd, pages, n, MSG_PROPTAB_TITLE, MAKEINTRESOURCE(IDI_TOMBO)) == IDOK) {
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
	DWORD res, typ, siz;
	HKEY hTomboRoot;

	hTomboRoot = GetTomboRootKey();
	if (hTomboRoot == NULL) return FALSE;

	*pStrict = TRUE;

	// メモトップフォルダパス
	if (pDefaultTopDir) {
		_tcscpy(aTopDir, pDefaultTopDir);
	} else {
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
	}

	// パスワードFingerPrint
	bValidSum = TRUE;
	siz = sizeof(aPasswordSum);
	res = RegQueryValueEx(hTomboRoot, FINGERPRINT_ATTR_NAME, NULL, &typ, aPasswordSum, &siz);
	if (res != ERROR_SUCCESS) {
		memset(aPasswordSum, 0, sizeof(aPasswordSum));
		bValidSum = FALSE;
	}

	// パスワードタイムアウト
	siz = sizeof(nPassTimeOut);
	res = RegQueryValueEx(hTomboRoot, PASSTIMEOUT_ATTR_NAME, NULL, &typ, (LPBYTE)&nPassTimeOut, &siz);
	if (res != ERROR_SUCCESS) {
		nPassTimeOut = 5;
		*pStrict = FALSE;
	}

	// fonts
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
	bSelectViewFontQuality = (BYTE)GetDWORDFromReg(hTomboRoot, SELECTVIEW_FONTQUALITY_ATTR_NAME, DEFAULT_QUALITY);

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
	bDetailsViewFontQuality = (BYTE)GetDWORDFromReg(hTomboRoot, DETAILSVIEW_FONTQUALITY_ATTR_NAME, DEFAULT_QUALITY);

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

	// SIP size delta
	nSIPSizeDelta = GetDWORDFromReg(hTomboRoot, SIPSIZE_DELTA_ATTR_NAME, 0);
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
		nUseTwoPane = TRUE;
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

	TCHAR buf[1024];
	siz = 1024;
	res = RegQueryValueEx(hTomboRoot, TOMBO_WINSIZE_ATTR_NAME2, NULL, &typ, (LPBYTE)buf, &siz);
	if (res != ERROR_SUCCESS) {
		pWinSize = NULL;
	}
	pWinSize = StringDup(buf);

#if defined(PLATFORM_WIN32)
	nTopMost = GetDWORDFromReg(hTomboRoot, STAYTOPMOST_ATTR_NAME, 0);
	nHideRebar = GetDWORDFromReg(hTomboRoot, HIDEREBAR_ATTR_NAME, 0);
#endif

	nWrapText = GetDWORDFromReg(hTomboRoot, WRAPTEXT_ATTR_NAME, 1);
	bOpenReadOnly = GetDWORDFromReg(hTomboRoot, OPENREADONLY_ATTR_NAME, FALSE);

	siz = sizeof(aDefaultNote);
	res = RegQueryValueEx(hTomboRoot, DEFAULTNOTE_ATTR_NAME, NULL, &typ, (LPBYTE)aDefaultNote, &siz);
	if (res != ERROR_SUCCESS) {
		aDefaultNote[0] = TEXT('\0');
	}

	nDisableSaveDlg = GetDWORDFromReg(hTomboRoot, DISABLESAVEDLG_ATTR_NAME, FALSE);

	// external applications
	nUseAssoc = GetDWORDFromReg(hTomboRoot, USEASSOC_ATTR_NAME, FALSE);
	siz = sizeof(aExtApp1);
	res = RegQueryValueEx(hTomboRoot, EXTAPP1_ATTR_NAME, NULL, &typ, (LPBYTE)aExtApp1, &siz);
	if (res != ERROR_SUCCESS) {
		aExtApp1[0] = TEXT('\0');
	}
	siz = sizeof(aExtApp2);
	res = RegQueryValueEx(hTomboRoot, EXTAPP2_ATTR_NAME, NULL, &typ, (LPBYTE)aExtApp2, &siz);
	if (res != ERROR_SUCCESS) {
		aExtApp2[0] = TEXT('\0');
	}

	// Safe filename options
	nSafeFileName = GetDWORDFromReg(hTomboRoot, SAFEFILENAME_ATTR_NAME, FALSE);

	nUseYAEdit = GetDWORDFromReg(hTomboRoot, USEYAE_ATTR_NAME, FALSE);

	// load bookmark
	delete[] pBookMark;
	pBookMark = GetMultiSZFromReg(hTomboRoot, BOOKMARK_ATTR_NAME);

	delete [] pSearchHistory;
	pSearchHistory = GetMultiSZFromReg(hTomboRoot, TOMBO_SEARCHHIST_ATTR_NAME); 

	delete [] pTopDirHistory;
	pTopDirHistory = GetMultiSZFromReg(hTomboRoot, TOMBO_TOPDIRHIST_ATTR_NAME);

#if defined(PLATFORM_PKTPC) && defined(FOR_VGA)
	nWinSize2 = GetDWORDFromReg(hTomboRoot, TOMBO_WINSIZE_ATTR_NAME3, 0xFFFF);
#endif

#if defined(PLATFORM_HPC)
	LPCOMMANDBANDSRESTOREINFO pcbi = new COMMANDBANDSRESTOREINFO[NUM_COMMANDBAR];
	siz = sizeof(COMMANDBANDSRESTOREINFO) * NUM_COMMANDBAR;
	res = RegQueryValueEx(hTomboRoot, TOMBO_REBARHIST_ATTR_NAME, 0, 
						&typ, (LPBYTE)pcbi, &siz);

	if (siz == NUM_COMMANDBAR * sizeof(COMMANDBANDSRESTOREINFO)) {
		delete []pCmdBarInfo;
		pCmdBarInfo = pcbi;
	}
#endif

	RegCloseKey(hTomboRoot);
	return TRUE;
}

//////////////////////////////////////////
// パラメータのストア
//////////////////////////////////////////

BOOL Property::Save()
{
	HKEY hTomboRoot;
	DWORD res;

	hTomboRoot = GetTomboRootKey();
	if (hTomboRoot == NULL) return FALSE;

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

	if (!SetDWORDToReg(hTomboRoot, SELECTVIEW_FONTQUALITY_ATTR_NAME, bSelectViewFontQuality)) return FALSE;
	if (!SetDWORDToReg(hTomboRoot, DETAILSVIEW_FONTQUALITY_ATTR_NAME, bDetailsViewFontQuality)) return FALSE;

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
	if (!SetDWORDToReg(hTomboRoot, SIPSIZE_DELTA_ATTR_NAME, nSIPSizeDelta)) return FALSE;
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
#if defined(PLATFORM_WIN32)
	if (!SetDWORDToReg(hTomboRoot, HIDEREBAR_ATTR_NAME, nHideRebar)) return FALSE;
#endif

	if (!SetDWORDToReg(hTomboRoot, OPENREADONLY_ATTR_NAME, bOpenReadOnly)) return FALSE;

	if (!SetSZToReg(hTomboRoot, DEFAULTNOTE_ATTR_NAME, aDefaultNote)) return FALSE;
	if (!SetDWORDToReg(hTomboRoot, DISABLESAVEDLG_ATTR_NAME, nDisableSaveDlg)) return FALSE;

	if (!SetDWORDToReg(hTomboRoot, USEASSOC_ATTR_NAME, nUseAssoc)) return FALSE;
	if (!SetSZToReg(hTomboRoot, EXTAPP1_ATTR_NAME, aExtApp1)) return FALSE;
	if (!SetSZToReg(hTomboRoot, EXTAPP2_ATTR_NAME, aExtApp2)) return FALSE;
	
	if (!SetDWORDToReg(hTomboRoot, SAFEFILENAME_ATTR_NAME, nSafeFileName)) return FALSE;

	if (!SetDWORDToReg(hTomboRoot, WRAPTEXT_ATTR_NAME, nWrapText)) return FALSE;

#if defined(PLATFORM_WIN32)
	if (!SetDWORDToReg(hTomboRoot, STAYTOPMOST_ATTR_NAME, nTopMost)) return FALSE;
#endif
#if defined(PLATFORM_HPC) || defined(PLATFORM_WIN32)
	if (!SetDWORDToReg(hTomboRoot, HIDESTATUSBAR_ATTR_NAME, nHideStatusBar)) return FALSE;
#endif

	if (!SetMultiSZToReg(hTomboRoot, BOOKMARK_ATTR_NAME, pBookMark, CountMultiSZLen(pBookMark) * sizeof(TCHAR))) return FALSE;
	if (!SetMultiSZToReg(hTomboRoot, TOMBO_SEARCHHIST_ATTR_NAME, pSearchHistory, CountMultiSZLen(pSearchHistory) * sizeof(TCHAR))) return FALSE;
	if (!SetMultiSZToReg(hTomboRoot, TOMBO_TOPDIRHIST_ATTR_NAME, pTopDirHistory, CountMultiSZLen(pTopDirHistory) * sizeof(TCHAR))) return FALSE;

	if (!SetSZToReg(hTomboRoot, TOMBO_WINSIZE_ATTR_NAME2, pWinSize)) return FALSE;

#if defined(PLATFORM_PKTPC) && defined(FOR_VGA)
	if (!SetDWORDToReg(hTomboRoot, TOMBO_WINSIZE_ATTR_NAME3, nWinSize2)) return FALSE;
#endif

#if defined(PLATFORM_HPC)
	if (RegSetValueEx(hTomboRoot, TOMBO_REBARHIST_ATTR_NAME, 0, 
					REG_BINARY, (LPBYTE)pCmdBarInfo, sizeof(COMMANDBANDSRESTOREINFO) * NUM_COMMANDBAR) != ERROR_SUCCESS) return FALSE;
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

static LPTSTR GetMultiSZFromReg(HKEY hKey, LPCTSTR pAttr)
{
	DWORD res, siz, typ;
	res = RegQueryValueEx(hKey, pAttr, NULL, &typ, NULL, &siz);
	if (res != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		SetLastError(res);
		return NULL;
	}

	LPTSTR pBuf;
	DWORD n = siz / sizeof(TCHAR) + 1;
	pBuf = new TCHAR[n];
	if (pBuf == NULL) {
		RegCloseKey(hKey);
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	memset(pBuf, 0, n * sizeof(TCHAR));
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
	if (pValue == NULL) return TRUE;

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
// get font
///////////////////////////////////////////////////

static BOOL MakeFont(HFONT *phFont, LPCTSTR pName, DWORD nSize, BYTE bQuality)
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
	lf.lfQuality = bQuality;
	lf.lfPitchAndFamily = DEFAULT_PITCH;
	_tcscpy(lf.lfFaceName, pName);
	*phFont = CreateFontIndirect(&lf);
	return *phFont != NULL;
}

///////////////////////////////////////////////////
// save window size
///////////////////////////////////////////////////

BOOL Property::SaveWinSize(UINT flags, UINT showCmd, LPRECT pWinRect, WORD nSelectViewWidth)
{
	TCHAR buf[1024];

	wsprintf(buf, TEXT("%d,%d,%d,%d,%d,%d,%d"), 
		flags, showCmd,
		pWinRect->left, pWinRect->top,
		pWinRect->right, pWinRect->bottom,
		nSelectViewWidth);

	delete [] pWinSize;
	pWinSize = StringDup(buf);
	return TRUE;
}

///////////////////////////////////////////////////
// get window size
///////////////////////////////////////////////////

BOOL Property::GetWinSize(UINT *pFlags, UINT *pShowCmd, LPRECT pWinRect, LPWORD pSelectViewWidth)
{
	if (pWinSize == NULL) {
		SetLastError(ERROR_INVALID_DATA);
		return FALSE;
	}

	if (_stscanf(pWinSize, TEXT("%d,%d,%d,%d,%d,%d,%d"),
		pFlags, pShowCmd,
		&(pWinRect->left), &(pWinRect->top),
		&(pWinRect->right), &(pWinRect->bottom),
		pSelectViewWidth) != 7) {

		SetLastError(ERROR_INVALID_DATA);
		return FALSE;
	}
	// check and modify window position
	if (pWinRect->left < 0) pWinRect->left = 0;
	if (pWinRect->top < 0) pWinRect->top = 0;

	return TRUE;
}

#if defined(PLATFORM_PKTPC) && defined(FOR_VGA)
WORD Property::GetWinSize2()
{
	return (WORD)nWinSize2;
}

BOOL Property::SaveWinSize2(WORD nSelectViewWidth)
{
	nWinSize2 = nSelectViewWidth;
	return TRUE;
}

#endif

void Property::SetUseTwoPane(BOOL bPane) 
{
	nUseTwoPane = bPane;
}

LPCTSTR Property::GetSearchHist() 
{
	return pSearchHistory;
}

LPCTSTR Property::GetTopDirHist()
{
	return pTopDirHistory;
}

void Property::SetSearchHist(LPTSTR pHist)
{
	delete [] pSearchHistory;
	pSearchHistory = pHist;
}

void Property::SetTopDirHist(LPTSTR pHist)
{
	delete [] pTopDirHistory;
	pTopDirHistory = pHist;
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

////////////////////////////////////////////////////////////////
// Get/Set History from/to ComboBox
////////////////////////////////////////////////////////////////

BOOL SetHistoryToComboBox(HWND hCombo, LPCTSTR pHistoryStr)
{
	LPCTSTR p = pHistoryStr;
	LPCTSTR q = p;
	while(*q) {
		SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)q);
		q += _tcslen(q) + 1;
	}
	SendMessage(hCombo, CB_SETCURSEL, 0, 0);
	return TRUE;
}

LPTSTR GetHistoryFromComboBox(HWND hCombo, LPCTSTR pSelValue, DWORD nSave)
{
	DWORD nItems = SendMessage(hCombo, CB_GETCOUNT, 0, 0);
	if (nItems > nSave) nItems = nSave;

	DWORD i;
	DWORD nMatch = SendMessage(hCombo, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)pSelValue);

	DWORD nBufLen = _tcslen(pSelValue) + 1;
	DWORD nLen;
	for (i = 0; i < nItems; i++) {
		if (i == nMatch) continue;

		nLen = SendMessage(hCombo, CB_GETLBTEXTLEN, i, 0);
		if (nLen == CB_ERR) return FALSE;
		nBufLen += nLen + 1;
	}
	nBufLen++;

	LPTSTR p = new TCHAR[nBufLen];
	if (!p) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}

	LPTSTR q = p;
	_tcscpy(q, pSelValue);
	q += _tcslen(pSelValue) + 1;
	for (i = 0; i < nItems; i++) {
		if (i == nMatch) continue;
		SendMessage(hCombo, CB_GETLBTEXT, i, (LPARAM)q);
		q += _tcslen(q) + 1;
	}
	*q = TEXT('\0');
	return p;
}

///////////////////////////////////////////////////
// Rebar
///////////////////////////////////////////////////
#if defined(PLATFORM_HPC)

BOOL Property::SetCommandbarInfo(LPCOMMANDBANDSRESTOREINFO p, DWORD n)
{
	delete[] pCmdBarInfo;
	pCmdBarInfo = new COMMANDBANDSRESTOREINFO[NUM_COMMANDBAR];
	memcpy(pCmdBarInfo, p, sizeof(COMMANDBANDSRESTOREINFO) * NUM_COMMANDBAR);
	return TRUE;
}

BOOL Property::GetCommandbarInfo(LPCOMMANDBANDSRESTOREINFO p, DWORD n)
{
	if (pCmdBarInfo == NULL) return FALSE;
	memcpy(p, pCmdBarInfo, sizeof(COMMANDBANDSRESTOREINFO) * NUM_COMMANDBAR);
	return TRUE;
}

#endif

BOOL Property::SetDefaultTomboRoot(LPCTSTR p, DWORD nLen)
{
	pDefaultTopDir = new TCHAR[nLen + 1];
	if (!pDefaultTopDir) return FALSE;
	_tcsncpy(pDefaultTopDir, p, nLen);
	pDefaultTopDir[nLen] = TEXT('\0');
	ChopFileSeparator(pDefaultTopDir);
	return TRUE;
}

static DWORD CountMultiSZLen(LPCTSTR pData)
{
	LPCTSTR p = pData;
	DWORD n = 0;
	while(*p) {
		DWORD i = _tcslen(p) + 1;
		p += i;
		n += i;
	}
	n++;
	return n;
}

///////////////////////////////////////////////////
// BookMark
///////////////////////////////////////////////////


LPCTSTR Property::GetBookMark()
{
	return pBookMark;
}

BOOL Property::SetBookMark(LPCTSTR pBM)
{
	DWORD nSize = CountMultiSZLen(pBM);

	LPTSTR pBuf = new TCHAR[nSize];
	if (pBuf == NULL) return FALSE;
	for (DWORD i = 0; i < nSize; i++) {
		pBuf[i] = pBM[i];
	}
	delete[] pBookMark;

	pBookMark = pBuf;
	return TRUE;
}
