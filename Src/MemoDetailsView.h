#ifndef MEMODETAILSVIEW_H
#define MEMODETAILSVIEW_H

class MemoManager;

//////////////////////////////////////////
// �������e�\���r���[
//////////////////////////////////////////

class MemoDetailsView {
	HWND hViewWnd;		// ���ݎg�p����Ă���E�B���h�E(hViewWnd_fd or hViewWnd_nf)
	HWND hViewWnd_fd;	// �܂�Ԃ����s���E�B���h�E
	HWND hViewWnd_nf;	// �܂�Ԃ����s��Ȃ��E�B���h�E

	MemoManager *pMemoMgr;
	DWORD nLeftOffset;

	BOOL bShowStatus;	// DetailsView�̕\���E��\�����

public:
	///////////////////////
	// �������֘A
	MemoDetailsView() : hViewWnd(NULL) {}
	BOOL Init(MemoManager *p) { pMemoMgr = p; return TRUE; }
	BOOL Create(LPCTSTR pName, RECT &r, HWND hParent, DWORD nID, DWORD nID_nf, HINSTANCE hInst, HFONT hFont);

	void SetTabstop();				// �^�u�X�g�b�v�ύX
	BOOL SetFolding(BOOL bFold);	// �܂�Ԃ��\���ؑ�

	////////////////////////
	// Window���b�Z�[�W�֘A
	BOOL Show(int nCmdShow);
	void SetFocus() { ::SetFocus(hViewWnd); }
	void SetFont(HFONT hFont);
	void MoveWindow(DWORD x, DWORD y, DWORD nWidth, DWORD nHeight);

	BOOL OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL OnHotKey(HWND hWnd, WPARAM wParam);
	void OnGetFocus();

	////////////////////////////
	// �f�[�^�A�N�Z�X�֘A
	BOOL SetMemo(LPCTSTR pMemo, DWORD nPos);
	LPTSTR GetMemo();
	BOOL IsModify() { return SendMessage(hViewWnd, EM_GETMODIFY, 0, 0); }
	void ResetModify() { SendMessage(hViewWnd, EM_SETMODIFY, (WPARAM)(UINT)FALSE, 0); }

	DWORD GetCursorPos();

	void SelectAll();		// �S�I��
	void SetModifyStatus();

	////////////////////////////
	// ����
	BOOL Search(BOOL bFirstSearch, BOOL bForward, BOOL bNFMsg, BOOL bSearchFromTop);
};

#endif
