#ifndef MEMODETAILSVIEW_H
#define MEMODETAILSVIEW_H

class MemoManager;

#ifdef STRICT 
typedef WNDPROC SUPER_WND_PROC;
#else 
typedef FARPROC SUPER_WND_PROC;
#endif 


//////////////////////////////////////////
// メモ内容表示ビュー
//////////////////////////////////////////

class MemoDetailsView {
	HWND hViewWnd;		// 現在使用されているウィンドウ(hViewWnd_fd or hViewWnd_nf)
	HWND hViewWnd_fd;	// 折り返しを行うウィンドウ
	HWND hViewWnd_nf;	// 折り返しを行わないウィンドウ

	MemoManager *pMemoMgr;
	DWORD nLeftOffset;

	BOOL bShowStatus;	// DetailsViewの表示・非表示状態

	BOOL bReadOnly;		// is read only mode?

	DWORD nInitialPos;	// cursor position when open this note.

public:
	///////////////////////
	// 初期化関連
	MemoDetailsView() : hViewWnd(NULL) {}
	BOOL Init(MemoManager *p) { pMemoMgr = p; return TRUE; }
	BOOL Create(LPCTSTR pName, RECT &r, HWND hParent, DWORD nID, DWORD nID_nf, HINSTANCE hInst, HFONT hFont);

	void SetTabstop();				// タブストップ変更
	BOOL SetFolding(BOOL bFold);	// 折り返し表示切替

	void SetReadOnly(BOOL bReadOnly);
	BOOL IsReadOnly() { return bReadOnly; }

	////////////////////////
	// Windowメッセージ関連
	BOOL Show(int nCmdShow);
	void SetFocus() { ::SetFocus(hViewWnd); }
	void SetFont(HFONT hFont);
	void MoveWindow(DWORD x, DWORD y, DWORD nWidth, DWORD nHeight);

	BOOL OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL OnHotKey(HWND hWnd, WPARAM wParam);
	void OnGetFocus();

	////////////////////////////
	// データアクセス関連
	BOOL SetMemo(LPCTSTR pMemo, DWORD nPos, BOOL bReadOnly);
	LPTSTR GetMemo();
	BOOL IsModify() { if (hViewWnd) return SendMessage(hViewWnd, EM_GETMODIFY, 0, 0); else return FALSE; }
	void ResetModify() { SendMessage(hViewWnd, EM_SETMODIFY, (WPARAM)(UINT)FALSE, 0); }

	DWORD GetCursorPos();
	DWORD GetInitialPos() { return nInitialPos; }

	void SelectAll();		// 全選択
	void SetModifyStatus();

	////////////////////////////
	// search
	BOOL Search(BOOL bFirstSearch, BOOL bForward, BOOL bNFMsg, BOOL bSearchFromTop);
};

#endif
