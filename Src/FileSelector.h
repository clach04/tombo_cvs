#ifndef FILESELECTOR_H
#define FILESELECTOR_H

///////////////////////////////////////////////////
// �t�@�C���������̓t�H���_��I��������Z���N�^
///////////////////////////////////////////////////

#if defined(PLATFORM_WIN32)
class FileSelector {
	TCHAR aPath[MAX_PATH];
public:
	FileSelector() {}
	~FileSelector(){}

	// pExt == NULL : �t�H���_�̑I��
	// pExt != NULL : �w�肵���g���q�����t�@�C���̑I��
	DWORD Popup(HINSTANCE hInst, HWND hWnd, LPCTSTR pTitle, LPCTSTR pExt);

	LPCTSTR SelectedPath() { return aPath; }
};

#else
class FileSelector {
	HWND hDialog;
	HIMAGELIST hImg;

	TCHAR aPath[MAX_PATH];
	LPTSTR pPath;

	LPCTSTR pTitle;
	LPCTSTR pExtension;
protected:
	void TreeExpand(HWND hTree, HTREEITEM hItem);
	void TreeCollapse(HWND hTree, HTREEITEM hItem);
	void ItemSelect(HWND hDlg, HWND hTree, HTREEITEM hItem);

public:
	FileSelector() : hDialog(NULL), hImg(NULL), pPath(NULL), pTitle(NULL), pExtension(NULL) {}
	~FileSelector();

	// pExt == NULL : �t�H���_�̑I��
	// pExt != NULL : �w�肵���g���q�����t�@�C���̑I��
	DWORD Popup(HINSTANCE hInst, HWND hWnd, LPCTSTR pTitle, LPCTSTR pExt);

	void InitDialog(HWND hDlg);
	void OnTreeNotify(HWND hDlg, WPARAM wParam, LPARAM lParam);
	void OnOK(HWND hDlg);

	LPCTSTR SelectedPath() { return pPath; }
};
#endif

#endif
