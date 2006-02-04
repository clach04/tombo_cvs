#ifndef PROPERTY_H
#define PROPERTY_H

#include <commctrl.h>

#define MAX_DATEFORMAT_LEN 256

// property index(number)
#define PROP_N_PASSTIMEOUT				 0
#define PROP_N_DETAILSVIEW_KEEPCARET	 1
#define PROP_NDETAILSVIEW_TABSTOP		 2
#define PROP_N_SELECTVIEW_FONTSIZE		 3
#define PROP_N_SELECTVIEW_FONTQUALITY	 4
#define PROP_N_DETAILSVIEW_FONTSIZE		 5
#define PROP_N_DETAILSVIEW_FONTQUALITY	 6
#define PROP_N_AUTOSELECT_MODE			 7
#define PROP_N_SINGLECLICK_MODE			 8
#define PROP_N_USE_TWO_PANE				 9
#define PROP_N_SWITCH_WINDOW_TITLE		10
#define PROP_N_KEEP_TITLE				11
#define PROP_N_APP_BUTTON1				12
#define PROP_N_APP_BUTTON2				13
#define PROP_N_APP_BUTTON3				14
#define PROP_N_APP_BUTTON4				15
#define PROP_N_APP_BUTTON5				16
#define PROP_N_CODEPAGE					17	// BE500
#define PROP_N_DISABLEEXTRAACTIONBUTTON	18	// PKTPC
#define PROP_N_SIPSIZE_DELTA			19	// PKTPC
#define PROP_N_HIDESTATUSBAR			20	// HPC, WIN32
#define PROP_N_STAYTOPMOST				21	// WIN32
#define PROP_N_HIDEREBAR				22	// WIN32
#define PROP_N_WRAPTEXT					23
#define PROP_N_OPENREADONLY				24
#define PROP_N_DISABLESAVEDLG			25
#define PROP_N_USEASSOC					26
#define PROP_N_SAFEFILENAME				27
#define PROP_N_TOMBO_WINSIZE3			28	// PKTPCVGA

#define NUM_PROPS_NUM 29

#define PROP_S_TOPDIR					 0
#define PROP_S_SELECTVIEW_FONTNAME		 1
#define PROP_S_DETAILSVIEW_FONTNAME		 2
#define PROP_S_DETAILSVIEW_DATEFORMAT1	 3
#define PROP_S_DETAILSVIEW_DATEFORMAT2	 4
#define PROP_S_DEFAULTNOTE				 5
#define PROP_S_EXTAPP1					 6
#define PROP_S_EXTAPP2					 7
#define PROP_S_WINSIZE					 8
#define NUM_PROPS_STR 9

class File;
////////////////////////////////////
// Property data
////////////////////////////////////

class Property {

	// persistent props
	DWORD nPropsNum[NUM_PROPS_NUM];
	LPTSTR pPropsStr[NUM_PROPS_STR];

	LPTSTR pBookMark;
	LPTSTR pSearchHistory;
	LPTSTR pTopDirHistory;

#if defined(PLATFORM_HPC)
	LPCOMMANDBANDSRESTOREINFO pCmdBarInfo;
#endif

	// not persistent props
	LPTSTR pDefaultTopDir;
	DWORD nUseYAEdit;

	// internal helper funcs 
	BOOL SetStringProperty(DWORD nPropId, LPCTSTR pValue);

	BOOL SaveToFile(File *pFile);
	BOOL LoadFromReg(BOOL *pStrict);

public:
	Property();
	~Property();

	/////////////////////////////////
	// set default props

	BOOL SetDefaultROMode(BOOL) {return TRUE; }
	BOOL SetDefaultTomboRoot(LPCTSTR p, DWORD nLen);

	/////////////////////////////////
	// accessor

	// tombo root directory
	LPCTSTR GetTopDir() { return pPropsStr[PROP_S_TOPDIR]; }
	BOOL SetTopDir(LPCTSTR pDir) { return SetStringProperty(PROP_S_TOPDIR, pDir); }

	// password timeout
	DWORD GetPassTimeout() { return nPropsNum[PROP_N_PASSTIMEOUT]; }
	void SetPassTimeout(DWORD n) { nPropsNum[PROP_N_PASSTIMEOUT] = n; }

	// font
	LPCTSTR GetSelectViewFontName() { return pPropsStr[PROP_S_SELECTVIEW_FONTNAME]; }
	BOOL SetSelectViewFontName(LPCTSTR pFontName) { return SetStringProperty(PROP_S_SELECTVIEW_FONTNAME, pFontName); }

	LPCTSTR GetDetailsViewFontName() { return pPropsStr[PROP_S_DETAILSVIEW_FONTNAME]; }
	BOOL SetDetailsViewFontName(LPCTSTR pFontName) { return SetStringProperty(PROP_S_DETAILSVIEW_FONTNAME, pFontName); }

	HFONT SelectViewFont();
	HFONT DetailsViewFont();

	// date format
	LPCTSTR GetDateFormat1() { return pPropsStr[PROP_S_DETAILSVIEW_DATEFORMAT1]; }
	BOOL SetDateFormat1(LPCTSTR pFormat) { return SetStringProperty(PROP_S_DETAILSVIEW_DATEFORMAT1, pFormat); }

	LPCTSTR GetDateFormat2() { return pPropsStr[PROP_S_DETAILSVIEW_DATEFORMAT2]; }
	BOOL SetDateFormat2(LPCTSTR pFormat) { return SetStringProperty(PROP_S_DETAILSVIEW_DATEFORMAT2, pFormat); }

	LPCTSTR GetDefaultNote() { return pPropsStr[PROP_S_DEFAULTNOTE]; }
	BOOL SetDefaultNote(LPCTSTR p) { return SetStringProperty(PROP_S_DEFAULTNOTE, p); }

	LPCTSTR GetExtApp1() { return pPropsStr[PROP_S_EXTAPP1]; }
	BOOL SetExtApp1(LPCTSTR p) { return SetStringProperty(PROP_S_EXTAPP1, p); }

	LPCTSTR GetExtApp2() { return pPropsStr[PROP_S_EXTAPP2]; }
	BOOL SetExtApp2(LPCTSTR p) { return SetStringProperty(PROP_S_EXTAPP2, p); }

	// tab stop
	DWORD GetTabstop() { return nPropsNum[PROP_NDETAILSVIEW_TABSTOP]; }
	void SetTabstop(DWORD n) { nPropsNum[PROP_NDETAILSVIEW_TABSTOP] = n; }

	// whether keep caret position or not
	DWORD GetKeepCaret() { return nPropsNum[PROP_N_DETAILSVIEW_KEEPCARET]; }
	void SetKeepCaret(DWORD n) { nPropsNum[PROP_N_DETAILSVIEW_KEEPCARET] = n; }

	DWORD GetSelectViewFontSize() { return nPropsNum[PROP_N_SELECTVIEW_FONTSIZE]; }
	void SetSelectViewFontSize(DWORD n) { nPropsNum[PROP_N_SELECTVIEW_FONTSIZE] = n; }

	DWORD GetSelectViewFontQuality() { return nPropsNum[PROP_N_SELECTVIEW_FONTQUALITY]; }
	void SetSelectViewFontQuality(DWORD n) { nPropsNum[PROP_N_SELECTVIEW_FONTQUALITY] = n; }

	DWORD GetDetailsViewFontSize() { return nPropsNum[PROP_N_DETAILSVIEW_FONTSIZE]; }
	void SetDetailsViewFontSize(DWORD n) { nPropsNum[PROP_N_DETAILSVIEW_FONTSIZE] = n; }

	DWORD GetDetailsViewFontQuality() { return nPropsNum[PROP_N_DETAILSVIEW_FONTQUALITY]; }
	void SetDetailsViewFontQuality(DWORD n) { nPropsNum[PROP_N_DETAILSVIEW_FONTQUALITY] = n; }

	DWORD GetAutoSelectMemo() { return nPropsNum[PROP_N_AUTOSELECT_MODE]; }
	void SetAutoSelectMemo(DWORD n) { nPropsNum[PROP_N_AUTOSELECT_MODE] = n; }

	DWORD GetSingleClick() { return nPropsNum[PROP_N_SINGLECLICK_MODE]; }
	void SetSingleClick(DWORD n) { nPropsNum[PROP_N_SINGLECLICK_MODE] = n; }

	// is sync window text to note's name?
	DWORD GetSwitchWindowTitle() { return nPropsNum[PROP_N_SWITCH_WINDOW_TITLE]; }
	void SetSwitchWindowTitle(DWORD n) { nPropsNum[PROP_N_SWITCH_WINDOW_TITLE] = n; }

	// keep file name even if headline has changed
	DWORD GetKeepTitle() { return nPropsNum[PROP_N_KEEP_TITLE]; }
	void SetKeepTitle(DWORD n) { nPropsNum[PROP_N_KEEP_TITLE] = n; }

	// is two pane mode?
	DWORD GetUseTwoPane() { return nPropsNum[PROP_N_USE_TWO_PANE]; }
	void SetUseTwoPane(DWORD n) { nPropsNum[PROP_N_USE_TWO_PANE] = n; }

	// application buttons
	DWORD GetAppButton1() { return nPropsNum[PROP_N_APP_BUTTON1]; }
	DWORD GetAppButton2() { return nPropsNum[PROP_N_APP_BUTTON2]; }
	DWORD GetAppButton3() { return nPropsNum[PROP_N_APP_BUTTON3]; }
	DWORD GetAppButton4() { return nPropsNum[PROP_N_APP_BUTTON4]; }
	DWORD GetAppButton5() { return nPropsNum[PROP_N_APP_BUTTON5]; }

	void SetAppButton1(DWORD n) { nPropsNum[PROP_N_APP_BUTTON1] = n; }
	void SetAppButton2(DWORD n) { nPropsNum[PROP_N_APP_BUTTON2] = n; }
	void SetAppButton3(DWORD n) { nPropsNum[PROP_N_APP_BUTTON3] = n; }
	void SetAppButton4(DWORD n) { nPropsNum[PROP_N_APP_BUTTON4] = n; }
	void SetAppButton5(DWORD n) { nPropsNum[PROP_N_APP_BUTTON5] = n; }

#if defined(PLATFORM_PKTPC)
	DWORD GetSipSizeDelta() { return nPropsNum[PROP_N_SIPSIZE_DELTA]; }
	void SetSipSizeDelta(DWORD n) { nPropsNum[PROP_N_SIPSIZE_DELTA] = n; }
	// Disable open/close notes when action button pushed
	DWORD GetDisableExtraActionButton() { return nPropsNum[PROP_N_DISABLEEXTRAACTIONBUTTON]; }
	void SetDisableExtraActionButton(DWORD n) { nPropsNum[PROP_N_DISABLEEXTRAACTIONBUTTON] = n; }
#endif

#if defined(PLATFORM_BE500)
	// Codepage selection
	DWORD GetCodePage() { return nPropsNum[PROP_N_CODEPAGE]; }
	void SetCodePage(DWORD n) { nPropsNum[PROP_N_CODEPAGE] = n; }
#endif

#if defined(PLATFORM_HPC) || defined(PLATFORM_WIN32)
	// Hide status bar
	DWORD GetHideStatusBar() { return nPropsNum[PROP_N_HIDESTATUSBAR]; }
	void SetHideStatusBar(DWORD n) { nPropsNum[PROP_N_HIDESTATUSBAR] = n; }
	void ToggleShowStatusBar() { nPropsNum[PROP_N_HIDESTATUSBAR] = !nPropsNum[PROP_N_HIDESTATUSBAR]; }
#else
	DWORD GetHideStatusBar() { return TRUE; }
	void ToggleShowStatusBar() { /* nop */ }
#endif
#if defined(PLATFORM_WIN32)
	DWORD GetStayTopMost() { return nPropsNum[PROP_N_STAYTOPMOST]; }
	void SetStayTopMost(DWORD n) { nPropsNum[PROP_N_STAYTOPMOST] = n; }
	void ToggleStayTopMost() { nPropsNum[PROP_N_STAYTOPMOST] = !nPropsNum[PROP_N_STAYTOPMOST]; }

	DWORD GetHideRebar() { return nPropsNum[PROP_N_HIDEREBAR]; }
	void SetHideRebar(DWORD n) { nPropsNum[PROP_N_HIDEREBAR] = n; }
	void ToggleShowRebar() { nPropsNum[PROP_N_HIDEREBAR] = !nPropsNum[PROP_N_HIDEREBAR]; }
#endif
	DWORD GetWrapText() { return nPropsNum[PROP_N_WRAPTEXT]; }
	void SetWrapText(DWORD n) { nPropsNum[PROP_N_WRAPTEXT] = n; }

	DWORD GetOpenReadOnly() { return nPropsNum[PROP_N_OPENREADONLY]; }
	void SetOpenReadOnly(DWORD n) { nPropsNum[PROP_N_OPENREADONLY] = n; }

	// save restore main window size
	BOOL SaveWinSize(UINT flags, UINT showCmd, LPRECT pWinRect, WORD nSelectViewWidth);
	BOOL GetWinSize(UINT *pFlags, UINT *pShowCmd, LPRECT pWinRect, LPWORD pSelectViewWidth);

#if defined(PLATFORM_PKTPC) && defined(FOR_VGA)
	DWORD GetWinSize2() { return nPropsNum[PROP_N_TOMBO_WINSIZE3]; }
	void SetWinSize2(DWORD n) { nPropsNum[PROP_N_TOMBO_WINSIZE3] = n; }
#endif

	DWORD GetDisableSaveDlg() { return nPropsNum[PROP_N_DISABLESAVEDLG]; }
	void SetDisableSaveDlg(DWORD n) { nPropsNum[PROP_N_DISABLESAVEDLG] = n; }

	DWORD GetUseAssociation() { return nPropsNum[PROP_N_USEASSOC]; }
	void SetUseAssociation(DWORD n) { nPropsNum[PROP_N_USEASSOC] = n; }

	DWORD GetUseSafeFileName() { return nPropsNum[PROP_N_SAFEFILENAME]; }
	void SetUseSafeFileName(DWORD n) { nPropsNum[PROP_N_SAFEFILENAME] = n; }

//	BOOL UseYAEdit() { return nUseYAEdit; }
	BOOL UseYAEdit() { return FALSE; }
//	BOOL UseYAEdit() { return TRUE; }


	LPCTSTR GetBookMark() { return pBookMark; }
	BOOL SetBookMark(LPCTSTR pBookMark);

	LPCTSTR GetSearchHist() { return pSearchHistory; } 
	void SetSearchHist(LPTSTR pHist) { delete [] pSearchHistory; pSearchHistory = pHist; }

	LPCTSTR GetTopDirHist() { return pTopDirHistory; }
	void SetTopDirHist(LPTSTR pHist) { delete [] pTopDirHistory; pTopDirHistory = pHist; }

#if defined(PLATFORM_HPC)
	// save commandbar position
	BOOL SetCommandbarInfo(LPCOMMANDBANDSRESTOREINFO p);
	void SetCommandbarInfoWithBuffer(LPCOMMANDBANDSRESTOREINFO p) { delete [] pCmdBarInfo; pCmdBarInfo = p; }
	BOOL GetCommandbarInfo(LPCOMMANDBANDSRESTOREINFO p, DWORD n);
#endif

	void SetNumberPropertyById(DWORD nId, DWORD nValue) { nPropsNum[nId] = nValue; }
	void SetStringPropertyWithBuffer(DWORD nId, LPTSTR pValue) { pPropsStr[nId] = pValue; }

	/////////////////////////////////
	// main op

	// popup property dialog
	DWORD Popup(HINSTANCE hInst, HWND hWnd, LPCTSTR pSelPath);

	// load properties
	BOOL Load(BOOL *pStrict);

	// save properties
	BOOL Save();

};

////////////////////////////////////
// Search history
////////////////////////////////////

BOOL SetHistoryToComboBox(HWND hCombo, LPCTSTR pHistoryStr);
LPTSTR GetHistoryFromComboBox(HWND hCombo, LPCTSTR pSelValue, DWORD nSave);

////////////////////////////////////
// global var declaration
////////////////////////////////////
extern Property g_Property;

/////////////////////////////////////////////
// attribute name definitions
/////////////////////////////////////////////

// some codes assumes this value is just '2'. check if this value is changed.
#define NUM_COMMANDBAR 2

#endif
