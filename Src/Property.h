#ifndef PROPERTY_H
#define PROPERTY_H

#include <commctrl.h>

#define MAX_DATEFORMAT_LEN 256

class Property {
	TCHAR aTopDir[MAX_PATH];

	LPTSTR pDefaultTopDir;

	BOOL bValidSum;
	BYTE aPasswordSum[33];
	DWORD nPassTimeOut;

	TCHAR aSelectViewFontName[LF_FACESIZE];
	DWORD nSelectViewFontSize;
	BYTE  bSelectViewFontQuality;

	TCHAR aDetailsViewFontName[LF_FACESIZE];
	DWORD nDetailsViewFontSize;
	BYTE bDetailsViewFontQuality;

	TCHAR aDateFormat1[MAX_DATEFORMAT_LEN];
	TCHAR aDateFormat2[MAX_DATEFORMAT_LEN];

	DWORD nKeepCaret;
	DWORD nTabstop;

	DWORD nAutoSelectMemo;
	DWORD nSingleClick;

	DWORD nAppButton1, nAppButton2, nAppButton3, nAppButton4, nAppButton5;

	DWORD nUseTwoPane;
	DWORD nSwitchWindowTitle;

	DWORD nKeepTitle;

	TCHAR aPropDir[MAX_PATH];

#if defined(PLATFORM_BE500)
	DWORD nCodePage;
#endif
#if defined(PLATFORM_PKTPC)
	DWORD nDisableExtraActionButton;
	DWORD nSIPSizeDelta;
#endif
#if defined(PLATFORM_HPC) || defined(PLATFORM_WIN32)
	DWORD nHideStatusBar;
#endif
#if defined(PLATFORM_WIN32)
	DWORD nTopMost;
#endif
	DWORD nWrapText;
	BOOL bOpenReadOnly;

	TCHAR aDefaultNote[MAX_PATH];

public:
	Property();
	~Property();

	/////////////////////////////////
	// set default props

	BOOL SetDefaultROMode(BOOL) {return TRUE; }
	BOOL SetDefaultTomboRoot(LPCTSTR p, DWORD nLen);

	/////////////////////////////////
	// accessor

	LPCTSTR TopDir() { return aTopDir; }
	const LPBYTE FingerPrint() { return bValidSum ? aPasswordSum : NULL; }

	DWORD PassTimeout() { return nPassTimeOut; }
	BOOL ScrollPage() { return TRUE; }

	// font
	HFONT SelectViewFont();
	HFONT DetailsViewFont();

	// date format
	LPCTSTR DateFormat1() { return aDateFormat1; }
	LPCTSTR DateFormat2() { return aDateFormat2; }

	// tab stop
	DWORD Tabstop() { return nTabstop; }

	// whether keep caret position or not
	BOOL KeepCaret() { return nKeepCaret; }

	// �I���r���[�̋���
	BOOL AutoSelectMemo() { return nAutoSelectMemo; }
	BOOL SingleClickOpenMemo() { return nSingleClick; }

	// �A�v���P�[�V�����{�^��
	DWORD AppButton1() { return nAppButton1; }
	DWORD AppButton2() { return nAppButton2; }
	DWORD AppButton3() { return nAppButton3; }
	DWORD AppButton4() { return nAppButton4; }
	DWORD AppButton5() { return nAppButton5; }

#if defined(PLATFORM_PKTPC)
	DWORD SipSizeDelta() { return nSIPSizeDelta; }
#endif

	// �E�B���h�E���������ɘA�������邩?
	BOOL SwitchWindowTitle() { return nSwitchWindowTitle; }

	// �v���p�e�B�_�C�A���O�̕\��
	DWORD Popup(HINSTANCE hInst, HWND hWnd, LPCTSTR pSelPath);

	// �v���p�e�B�l�̃��[�h
	BOOL Load(BOOL *pStrict);

	// �v���p�e�B�l�̃Z�[�u
	BOOL Save();

	// 2Pane���[�h���g�����ǂ���
	BOOL IsUseTwoPane() { return nUseTwoPane; }
	void SetUseTwoPane(BOOL bPane);

	// �t�@�C�����������̃^�C�g���ɘA�������Ȃ�
	BOOL KeepTitle() { return nKeepTitle; }

	// Property folder path
	LPCTSTR PropertyDir() { return aPropDir; }

#if defined(PLATFORM_BE500)
	// Codepage selection
	DWORD CodePage() { return nCodePage; }
#endif

#if defined(PLATFORM_PKTPC)
	// Disable open/close notes when action button pushed
	DWORD DisableExtraActionButton() { return nDisableExtraActionButton; }
#endif
#if defined(PLATFORM_HPC) || defined(PLATFORM_WIN32)
	// Hide status bar
	DWORD HideStatusBar() { return nHideStatusBar; }
	void ToggleShowStatusBar() { nHideStatusBar = !nHideStatusBar; }
#endif
#if defined(PLATFORM_WIN32)
	BOOL StayTopMost() { return nTopMost; }
	void ToggleStayTopMost() { nTopMost = !nTopMost; }
#endif
	DWORD WrapText() { return nWrapText; }
	void SetWrapText(DWORD n) { nWrapText = n; }

	BOOL OpenReadOnly() { return bOpenReadOnly; }


	// save restore main window size
	static BOOL SaveWinSize(UINT flags, UINT showCmd, LPRECT pWinRect, WORD nSelectViewWidth);
	static BOOL GetWinSize(UINT *pFlags, UINT *pShowCmd, LPRECT pWinRect, LPWORD pSelectViewWidth);

	BOOL SaveStatusBarStat();
	BOOL SaveTopMostStat();
	BOOL SaveWrapTextStat();

	LPCTSTR GetDefaultNote() { return aDefaultNote; }

	friend class FolderTab;
	friend class PasswordTab;
	friend class PassTimeoutTab;
	friend class FontTab;
	friend class DateFormatTab;
	friend class KeepCaretTab;
	friend class SelectMemoTab;
	friend class AppButtonTab;
	friend class CodepageTab;
	friend class SipTab;
	friend class DefaultNoteTab;
};

////////////////////////////////////
// ��������
////////////////////////////////////

LPTSTR LoadStringHistory(LPCTSTR pAttrName);
BOOL StoreStringHistory(LPCTSTR pAttrName, LPCTSTR pHistString, DWORD nSize);
BOOL RetrieveAndSaveHistory(HWND hCombo, LPCTSTR pAttrName, LPCTSTR pSelValue, DWORD nSave);
BOOL LoadHistory(HWND hCombo, LPCTSTR pAttrName);

////////////////////////////////////
// �R�}���h�o�[�ʒu���ێ�
////////////////////////////////////

#if defined(PLATFORM_HPC)
BOOL SetCommandbarInfo(LPCOMMANDBANDSRESTOREINFO p, DWORD n);
BOOL GetCommandbarInfo(LPCOMMANDBANDSRESTOREINFO p, DWORD n);
#endif

////////////////////////////////////
// �O���[�o���ϐ��錾
////////////////////////////////////
extern Property g_Property;

/////////////////////////////////////////////
// ��������`(�t�@�C���Ԃ��܂�������̂̂�)
/////////////////////////////////////////////

#define TOMBO_SEARCHHIST_ATTR_NAME TEXT("SearchHistory")
#define TOMBO_TOPDIRHIST_ATTR_NAME TEXT("TopDirHistory")

#endif
