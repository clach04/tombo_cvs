#include <windows.h>
#include <tchar.h>
#include "Tombo.h"
#include "resource.h"
#include "Message.h"
#include "GrepDialog.h"
#include "Property.h"
#include "PropertyPage.h"

/////////////////////////////////////////
// ctor & dtor
/////////////////////////////////////////

BOOL GrepDialog::Init(LPCTSTR pPath)
{
	if (!sPath.Set(pPath)) return FALSE;
	return TRUE;
}

/////////////////////////////////////////
// Main Tab
/////////////////////////////////////////

class GrepMainTab : public PropertyTab {
	GrepDialog *pDialog;
public:
	GrepMainTab(GrepDialog *pDlg) : pDialog(pDlg), PropertyTab(IDD_GREPTAB_MAIN, IDS_GREPTAB_MAIN, (DLGPROC)DefaultPageProc) {}
	~GrepMainTab() {}

	void Init(HWND hDlg);
	BOOL Apply(HWND hDlg);
	BOOL OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
};

void GrepMainTab::Init(HWND hDlg)
{
	HWND hCombo = GetDlgItem(hDlg, IDC_GREPTAB_MAIN_SEARCHSTRING);
	HWND hPath = GetDlgItem(hDlg, IDC_GREPTAB_MAIN_STARTDIR);

	TString sDispPath;
	if (!sDispPath.Join(TEXT("folder:\\"), pDialog->GetPath())) return;
	SetWindowText(hPath, sDispPath.Get());
	LoadHistory(hCombo, TOMBO_SEARCHHIST_ATTR_NAME);
}

BOOL GrepMainTab::Apply(HWND hDlg)
{
	HWND hCaseSensitive = GetDlgItem(hDlg, IDC_GREPTAB_MAIN_CASESENSITIVE);
	HWND hCheckCrypted = GetDlgItem(hDlg, IDC_GREPTAB_MAIN_ENCRYPTNOTE);
	HWND hCheckFileName = GetDlgItem(hDlg, IDC_GREPTAB_MAIN_FILENAME);
	HWND hMatchString = GetDlgItem(hDlg, IDC_GREPTAB_MAIN_SEARCHSTRING);

	TString *pMatchString = pDialog->GetMatchStringPtr();
	DWORD nLen = GetWindowTextLength(hMatchString);
	if (!pMatchString->Alloc(nLen + 1)) return FALSE;
	GetWindowText(hMatchString, pMatchString->Get(), nLen + 1);
	RetrieveAndSaveHistory(hMatchString, TOMBO_SEARCHHIST_ATTR_NAME, pMatchString->Get(), NUM_SEARCH_HISTORY);

	pDialog->SetCaseSensitive(SendMessage(hCaseSensitive, BM_GETCHECK, 0, 0) == BST_CHECKED);
	pDialog->SetCheckCryptedMemo(SendMessage(hCheckCrypted, BM_GETCHECK, 0, 0) == BST_CHECKED);
	pDialog->SetCheckFileName(SendMessage(hCheckFileName, BM_GETCHECK, 0, 0) == BST_CHECKED);

	return TRUE;
}

BOOL GrepMainTab::OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

/////////////////////////////////////////
// Persistent Tab
/////////////////////////////////////////

class GrepPersistTab : public PropertyTab {
public:
	GrepPersistTab() : PropertyTab(IDD_GREPTAB_PERSIST, IDS_GREPTAB_PERSIST, (DLGPROC)DefaultPageProc) {}
	~GrepPersistTab() {}

	void Init(HWND hDlg);
	BOOL Apply(HWND hDlg);
	BOOL OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
};

void GrepPersistTab::Init(HWND hDlg)
{
}

BOOL GrepPersistTab::Apply(HWND hDlg)
{
	return TRUE;
}

BOOL GrepPersistTab::OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

/////////////////////////////////////////
// Dialog popup
/////////////////////////////////////////

DWORD GrepDialog::Popup(HINSTANCE hInst, HWND hParent)
{
	PropertyTab *pages[2];
	GrepMainTab pgMain(this);
	GrepPersistTab pgPersist;

	pages[0] = &pgMain;
	pages[1] = &pgPersist;

	PropertyPage pp;

	return pp.Popup(hInst, hParent, pages, 2, MSG_GREPTAB_MAIN_TTL, MAKEINTRESOURCE(IDI_TOMBO));
}
