#ifndef FILTERCTLDLG_H
#define FILTERCTLDLG_H

class MemoSelectView;

class FilterCtlDlg {
	HIMAGELIST hImageList;
	MemoSelectView *pView;
public:
	BOOL Init(MemoSelectView *pView);

	DWORD Popup(HINSTANCE hInst, HWND hParent, HIMAGELIST hImgList); 

	void InitDialog(HWND hDlg);
	BOOL OnOK(HWND hDlg);
};

#endif