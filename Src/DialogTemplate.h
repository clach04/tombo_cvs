#ifndef DIALOGTEMPLATE_H
#define DIALOGTEMPLATE_H

namespace Tombo_Lib {

////////////////////////////////////////////////////////////////
// Dialog template
////////////////////////////////////////////////////////////////

class DialogTemplate {
protected:
	HINSTANCE hInstance;
	HWND hDialog;

	DWORD Popup(HINSTANCE hInst, WORD nID, HWND hParent);

public:
	virtual void InitDialog(HWND hDlg);
	virtual BOOL OnOK();

	virtual BOOL OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(HWND hDlg, WPARAM wParam, LPARAM lParam);


	void SetDialog(HWND hDlg);
};

} // end namespace

#endif
