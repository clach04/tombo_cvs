#ifndef FILTERCTLDLG_H
#define FILTERCTLDLG_H

class MemoSelectView;
class VFManager;

class FilterCtlDlg {
	HIMAGELIST hImageList;
	MemoSelectView *pView;
	VFManager *pManager;

	DWORD nNotKeepPos;

public:
	BOOL Init(MemoSelectView *pView, VFManager *pManager);

	DWORD Popup(HINSTANCE hInst, HWND hParent, HIMAGELIST hImgList); 

	void InitDialog(HWND hDlg);
	void DestroyDialog(HWND hDlg, int iResult);

	BOOL OnOK(HWND hDlg);
	BOOL OnNotify(HWND hDlg, WPARAM wParam, LPARAM lParam);
	void OnToggleKeep(HWND hDlg);

	BOOL OnUp(HWND hDlg);
	BOOL OnDown(HWND hDlg);

	BOOL Notify_ItemChanged(HWND hDlg, LPARAM lParam);
	BOOL Notify_EndLabelEdit(HWND hDlg, LPARAM lParam);
	BOOL Notify_Keydown(HWND hDlg, LPARAM lParam);
};

#endif