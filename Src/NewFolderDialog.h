#ifndef NEWFOLDERDIALOG_H
#define NEWFOLDERDIALOG_H

class NewFolderDialog {
	HWND hDialog;
	HINSTANCE hInstance;
	TCHAR aFolder[MAX_PATH];
public:
	
	NewFolderDialog() : hDialog(NULL), hInstance(NULL) { aFolder[0] = TEXT('\0'); }
	void InitDialog(HWND hWnd);
	BOOL OnOK(HWND hWnd);

	DWORD Popup(HINSTANCE hInst, HWND hParent);
	LPCTSTR FolderName() { return aFolder; }
};

#endif