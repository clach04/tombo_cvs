#ifndef MEMODETAILSVIEW_H
#define MEMODETAILSVIEW_H

class MemoManager;

#ifdef STRICT 
typedef WNDPROC SUPER_WND_PROC;
#else 
typedef FARPROC SUPER_WND_PROC;
#endif 


//////////////////////////////////////////
// Edit view
//////////////////////////////////////////

class MemoDetailsView {
	HWND hViewWnd;		// The window handle used now(hViewWnd_fd or hViewWnd_nf)
	HWND hViewWnd_fd;	// The window created by wrapping options
	HWND hViewWnd_nf;	// The window created by no wrapping options

	MemoManager *pMemoMgr;
	DWORD nLeftOffset;

	BOOL bShowStatus;	// Is view displayed?

	BOOL bReadOnly;		// is read only mode?

	DWORD nInitialPos;	// cursor position when open this note.

public:

	///////////////////////
	// Initialize

	MemoDetailsView() : hViewWnd(NULL) {}
	BOOL Init(MemoManager *p) { pMemoMgr = p; return TRUE; }
	BOOL Create(LPCTSTR pName, RECT &r, HWND hParent, DWORD nID, DWORD nID_nf, HINSTANCE hInst, HFONT hFont);


	///////////////////////
	// Properties

	void SetTabstop();				// Tab stop
	BOOL SetFolding(BOOL bFold);	// Change wrapping

	void SetReadOnly(BOOL bReadOnly);
	BOOL IsReadOnly() { return bReadOnly; }

	void SetModifyStatus();

	////////////////////////
	// Message handler

	BOOL Show(int nCmdShow);
	void SetFocus() { ::SetFocus(hViewWnd); }
	void SetFont(HFONT hFont);
	void MoveWindow(DWORD x, DWORD y, DWORD nWidth, DWORD nHeight);

	BOOL OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL OnHotKey(HWND hWnd, WPARAM wParam);
	void OnGetFocus();

	////////////////////////////
	// Data access

	BOOL SetMemo(LPCTSTR pMemo, DWORD nPos, BOOL bReadOnly);
	LPTSTR GetMemo();
	BOOL IsModify() { if (hViewWnd) return SendMessage(hViewWnd, EM_GETMODIFY, 0, 0); else return FALSE; }
	void ResetModify() { SendMessage(hViewWnd, EM_SETMODIFY, (WPARAM)(UINT)FALSE, 0); }

	DWORD GetCursorPos();
	DWORD GetInitialPos() { return nInitialPos; }

	void SelectAll();

	void InsertDate1();
	void InsertDate2();

	////////////////////////////
	// search

	BOOL Search(BOOL bFirstSearch, BOOL bForward, BOOL bNFMsg, BOOL bSearchFromTop);
};

#endif
