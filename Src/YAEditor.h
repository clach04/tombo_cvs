#ifndef YAEDITOR_H
#define YAEDITOR_H

#include "MemoDetailsView.h"

class YAEdit;
class MemoManager;
class YAEDetailsViewCallback;

class YAEditor : public MemoDetailsView {
	YAEdit *pEdit;
	DWORD nID;

	YAEDetailsViewCallback *pYAECallback;

	BOOL SetMemo(LPCTSTR pMemo, DWORD nPos, BOOL bReadOnly);

public:

	YAEditor(MemoManager *pMgr);
	virtual ~YAEditor();
	BOOL Init(DWORD nID);


	BOOL Create(LPCTSTR pName, RECT &r, HWND hParent, HINSTANCE hInst, HFONT hFont);
	void SetFocus();
	void MoveWindow(DWORD x, DWORD y, DWORD nWidth, DWORD nHeight);

	BOOL IsModify();
	void ResetModify();

	void SetMDSearchFlg(BOOL bFlg);

	//////////////////////////
	// implimenting

	LPTSTR GetMemo();

	//////////////////////////
	// not implimented yet

	BOOL Show(int nCmdShow) { return TRUE; }

	void SetTabstop() {}
	BOOL SetFolding(BOOL bFold) { return TRUE; }
	void SetReadOnly(BOOL bReadOnly) {}
	BOOL IsReadOnly() { return FALSE; }

	void SetModifyStatus() {}

	void SetFont(HFONT hFont);

	BOOL OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL OnHotKey(HWND hWnd, WPARAM wParam) { return TRUE; }
	void OnGetFocus();

	DWORD GetCursorPos();
	DWORD GetInitialPos() { return 0; }

	void SelectAll() {}

	void ChangeModifyStatusNotify(BOOL bStatus);
	void SetSelectRegion(DWORD nStart, DWORD nEnd);

	BOOL ReplaceText(LPCTSTR p);
};

#endif