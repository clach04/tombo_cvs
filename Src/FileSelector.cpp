#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include "FileSelector.h"
#include "Resource.h"

#if defined(PLATFORM_WIN32)
////////////////////////////////////////////////////
////////////////////////////////////////////////////
// Win32�p�t�@�C���I���_�C�A���O
////////////////////////////////////////////////////
////////////////////////////////////////////////////

#include <shlobj.h>

DWORD FileSelector::Popup(HINSTANCE hInst, HWND hWnd, LPCTSTR pTitle, LPCTSTR pExt)
{
	if (pExt != NULL) {
		MessageBox(hWnd, TEXT("Not Implimented Yet"), TEXT("ERROR"), MB_ICONERROR|MB_OK);
		return IDCANCEL;
	}

	LPITEMIDLIST pIdList;
	TCHAR buf[MAX_PATH];
	BROWSEINFO bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.hwndOwner = hWnd;
	bi.pszDisplayName = buf;
	bi.lpszTitle = pTitle;
	bi.ulFlags = BIF_RETURNONLYFSDIRS;

	pIdList = SHBrowseForFolder(&bi);
	if (pIdList == NULL) return IDCANCEL;

	SHGetPathFromIDList(pIdList, aPath); 
	CoTaskMemFree(pIdList);
	return IDOK;
}

#else // PLATFORM_WIN32
////////////////////////////////////////////////////
////////////////////////////////////////////////////
// CE�p�t�@�C���I���_�C�A���O
////////////////////////////////////////////////////
////////////////////////////////////////////////////


#define IMAGE_CX 16
#define IMAGE_CY 16
#define NUM_BITMAPS 4

#define IMG_FOLDER 0
#define IMG_FOLDER_SEL 1
#define IMG_ARTICLE 2
#define IMG_DRIVE 3

//////////////////////////////////////////////////
// dtor
//////////////////////////////////////////////////
FileSelector::~FileSelector()
{
	if (hImg) ImageList_Destroy(hImg);
}

//////////////////////////////////////////////////
// Dialog Procedure
//////////////////////////////////////////////////

static BOOL APIENTRY DlgProc(HWND hDlg, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	FileSelector *pDlg;

	if (nMessage == WM_INITDIALOG) {
		// �I�u�W�F�N�g���L�����Ă���
		SetWindowLong(hDlg, DWL_USER, lParam);
		pDlg = (FileSelector*)lParam;

		pDlg->InitDialog(hDlg);
		return TRUE;
	}

	pDlg = (FileSelector*)GetWindowLong(hDlg, DWL_USER);
	if (pDlg == NULL) return FALSE;

	switch(nMessage) {
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			pDlg->OnOK(hDlg);
			break;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			break;
		}
		return TRUE;
	case WM_NOTIFY:
		if (wParam == IDC_FILESELECT_TREE) {
			pDlg->OnTreeNotify(hDlg, wParam, lParam);
		}
		return TRUE;
	}
	return FALSE;
}

//////////////////////////////////////////////////
// Popup
//////////////////////////////////////////////////

DWORD FileSelector::Popup(HINSTANCE hInst, HWND hWnd, LPCTSTR pt, LPCTSTR pExt)
{
	// Create image list
	if (hImg == NULL) {
	    if ((hImg = ImageList_Create(IMAGE_CX, IMAGE_CY, ILC_MASK, NUM_BITMAPS, 0)) == NULL) return IDCANCEL;
	     HBITMAP hBmp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_ARTICLEBOX)); 
		COLORREF rgbTransparent = RGB(0, 255, 0);
		ImageList_AddMasked(hImg, hBmp, rgbTransparent); 
	    DeleteObject(hBmp); 
	}

	pTitle = pt;
	if (pExt != NULL) pExtension = pExt;

	DWORD result;
	result = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_FILESELECT), hWnd, (DLGPROC)DlgProc, (LONG)this);
	DWORD n = GetLastError();
	return result;
}

//////////////////////////////////////////////////
// Initialize dialog
//////////////////////////////////////////////////

static DlgMsgRes aMsgRes[] = {
	{ IDOK,     MSG_DLG_CMN_OK},
	{ IDCANCEL, MSG_DLG_CMN_CANCEL},
};

void FileSelector::InitDialog(HWND hWnd)
{
	OverrideDlgMsg(hWnd, MSG_DLG_FILESELECT_TITLE, aMsgRes, sizeof(aMsgRes)/sizeof(DlgMsgRes));

	HWND hTree = GetDlgItem(hWnd, IDC_FILESELECT_TREE);

    TreeView_SetImageList(hTree, hImg, TVSIL_NORMAL); 
	TreeView_DeleteAllItems(hTree);

	// �h���C�u�m�[�h���쐬
	TV_INSERTSTRUCT ti;
	ti.hParent = TVI_ROOT;
	ti.hInsertAfter = TVI_LAST;

	TCHAR buf[3];

	ti.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	ti.item.iImage = ti.item.iSelectedImage = IMG_DRIVE;
	ti.item.pszText = buf;
	ti.item.lParam = NULL;
	buf[1] = TEXT(':');
	buf[2] = TEXT('\0');

	TV_INSERTSTRUCT td;
	td.hInsertAfter = TVI_LAST;
	td.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	td.item.iImage = IMG_FOLDER;
	td.item.iSelectedImage = IMG_FOLDER_SEL;
	td.item.pszText = TEXT("DUMMY");

#ifdef _WIN32_WCE
	_tcscpy(buf, TEXT("\\"));
	td.hParent = TreeView_InsertItem(hTree, &ti);
	TreeView_InsertItem(hTree, &td);
#else
	UINT ut;
	for (buf[0] = TEXT('A'); buf[0] <= TEXT('Z'); buf[0]++) {
		ut = GetDriveType(buf);
		if (ut == DRIVE_FIXED || ut == DRIVE_REMOTE || ut == DRIVE_RAMDISK) {
			td.hParent = TreeView_InsertItem(hTree, &ti);
			if (td.hParent == NULL) {
				TCHAR buf[1024];
				wsprintf(buf, TEXT("Can't insert tree item %d"), GetLastError());
				MessageBox(NULL, buf, TEXT("WARNING"), MB_ICONEXCLAMATION | MB_OK);
			}
			TreeView_InsertItem(hTree, &td);
		}
	}
#endif

	// �^�C�g���̐ݒ�
	SetWindowText(hWnd, pTitle);

	// OK�{�^���͍ŏ���disable
	HWND hOk = GetDlgItem(hWnd, IDOK);
	EnableWindow(hOk, FALSE);
}

///////////////////////////////////////////////////////////////////
// �c���[�r���[�ƑI�����ꂽ�A�C�e������A�p�X�ƂȂ镶����𓾂�B
//
// �߂�l�͕�����̐擪�B(pBuf�̒����w��)
// pBuf�͌�둤����O�Ɍ������Ďg�p����B
static LPTSTR GeneratePath(HWND hTree, HTREEITEM hItem, LPTSTR pBuf, DWORD nSiz)
{
	LPTSTR p = pBuf + nSiz - 2;
	*(p+1) = TEXT('\0');

	TV_ITEM it;
	TCHAR buf[MAX_PATH];

	HTREEITEM h = hItem;
	it.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
	it.pszText = buf;

	while(h) {
		it.hItem = h;
		it.cchTextMax = MAX_PATH;
		TreeView_GetItem(hTree, &it);

		*p-- = TEXT('\\');
		p -= _tcslen(buf) - 1;
		_tcsncpy(p, buf, _tcslen(buf));
		p--;

		h = TreeView_GetParent(hTree, h);
	}
#ifdef _WIN32_WCE
	// CE�̏ꍇ�A�h���C�u���Ȃ����߁A���̂܂܂���'\\'�ƂȂ��Ă��܂��B������+1����'\'��1�����B
	return p + 2;
#else
	return p + 1;
#endif
}

//////////////////////////////////////////////////
// �c���[�ɑ΂���NOTIFY
//////////////////////////////////////////////////

// �t�H���_�m�[�h�̓r���[��'+'��\�������邽�߁A�_�~�[�m�[�h��1�����Ă���B
// �m�[�h�I�[�v�����ɂ̓_�~�[�m�[�h�������āA�t�H���_�̓��e��ǉ�����B
// ����Ƃ��ɂ͎q���̗v�f�������āA�_�~�[�m�[�h��ǉ�����B

void FileSelector::OnTreeNotify(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	LPNM_TREEVIEW pHdr = (LPNM_TREEVIEW)lParam;
	HWND hTree = GetDlgItem(hWnd, IDC_FILESELECT_TREE);

	switch(pHdr->hdr.code) {
	case TVN_ITEMEXPANDING:
		if(pHdr->action == TVE_EXPAND) {
			// �c���[���J�����Ƃ���
			TreeExpand(hTree, pHdr->itemNew.hItem);
		} else {
			// �c���[����悤�Ƃ���
			TreeCollapse(hTree, pHdr->itemNew.hItem);
		}
		break;
	case TVN_SELCHANGED:
		ItemSelect(hWnd, hTree, pHdr->itemNew.hItem);
		break;
	}
}

//////////////////////////////////////////////////
// OK
//////////////////////////////////////////////////

void FileSelector::OnOK(HWND hWnd)
{
	HWND hTree = GetDlgItem(hWnd, IDC_FILESELECT_TREE);
	HTREEITEM h = TreeView_GetSelection(hTree);
	pPath = GeneratePath(hTree, h, aPath, MAX_PATH);
	if (pExtension) {
		pPath[_tcslen(pPath) - 1] = TEXT('\0');
	}

	EndDialog(hWnd, IDOK);
}

//////////////////////////////////////////////////
//

static HTREEITEM GetInsertAfter(HWND hTree, HTREEITEM hItem, LPCTSTR pStr, BOOL bFolder)
{
	HTREEITEM h = TreeView_GetChild(hTree, hItem);

	if (h == NULL) return TVI_FIRST;

	HTREEITEM hd = TVI_FIRST;
	TV_ITEM ti;
	HTREEITEM res = TVI_LAST;

	TCHAR buf[MAX_PATH];

	ti.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_IMAGE;
	while (h) {
		ti.hItem = h;
		ti.pszText = buf;
		ti.cchTextMax = MAX_PATH;

		TreeView_GetItem(hTree, &ti);
		if (!(ti.iImage == IMG_FOLDER && bFolder == FALSE)) {
			if (_tcsicmp(buf, pStr) >= 0) {
				res = hd;
				break;
			}
		}
		hd = h;
		h = TreeView_GetNextSibling(hTree, h);
	}
	return res;
}

//////////////////////////////////////////////////
// �c���[�̓W�J
//////////////////////////////////////////////////

void FileSelector::TreeExpand(HWND hTree, HTREEITEM hItem)
{
	TCHAR buf[MAX_PATH], buf2[MAX_PATH];
	LPTSTR pPath = GeneratePath(hTree, hItem, buf, MAX_PATH);
		
	HTREEITEM di = TreeView_GetChild(hTree, hItem);
	TreeView_DeleteItem(hTree, di);
	
	// �f�B���N�g���ꗗ�̎擾�E�c���[�ւ̒ǉ�
	WIN32_FIND_DATA wfd;
	TV_INSERTSTRUCT ti;
	ti.hParent = hItem;
	ti.hInsertAfter = TVI_LAST;
	ti.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	ti.item.iImage = IMG_FOLDER;
	ti.item.iSelectedImage = IMG_FOLDER_SEL;
	ti.item.pszText = wfd.cFileName;
	ti.item.lParam = (LPARAM)hItem;
	
	// �W�J�p��"+"�{�^����\�������邽�߂̃_�~�[�m�[�h
	TV_INSERTSTRUCT td;
	td.hInsertAfter = TVI_LAST;
	td.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	td.item.iImage = IMG_FOLDER;
	td.item.iSelectedImage = IMG_FOLDER_SEL;
	td.item.pszText = TEXT("DUMMY");

	wsprintf(buf2, TEXT("%s*.*"), pPath);
	HANDLE hFile = FindFirstFile(buf2, &wfd);
	if (hFile != INVALID_HANDLE_VALUE) {
		do {
			if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
			if (_tcscmp(wfd.cFileName, TEXT(".")) == 0 ||
				_tcscmp(wfd.cFileName, TEXT("..")) == 0) continue;

			// �}��
			ti.hInsertAfter = GetInsertAfter(hTree, hItem, wfd.cFileName, TRUE);
			td.hParent = TreeView_InsertItem(hTree, &ti);

			// �_�~�[�m�[�h��}��
			TreeView_InsertItem(hTree, &td);

		} while(FindNextFile(hFile, &wfd));
		FindClose(hFile);
	}
	
	// �t�@�C���I���̏ꍇ�ɂ̓t�@�C�����\������
	if (pExtension) {
		ti.item.iImage = ti.item.iSelectedImage = IMG_ARTICLE;
		wsprintf(buf2, TEXT("%s%s"), pPath, pExtension);
		hFile = FindFirstFile(buf2, &wfd);
		if (hFile != INVALID_HANDLE_VALUE) {
			do {
				if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

				ti.hInsertAfter = GetInsertAfter(hTree, hItem, wfd.cFileName, FALSE);
				TreeView_InsertItem(hTree, &ti);
			} while(FindNextFile(hFile, &wfd));
			FindClose(hFile);
		}
	}
}

//////////////////////////////////////////////////
// �c���[�����
//////////////////////////////////////////////////

void FileSelector::TreeCollapse(HWND hTree, HTREEITEM hItem) 
{
	// ���ʃm�[�h�̍폜
	HTREEITEM h = TreeView_GetChild(hTree, hItem);
	HTREEITEM hd;
	while (h) {
		hd = h;
		h = TreeView_GetNextSibling(hTree, h);
		TreeView_DeleteItem(hTree, hd);
	}
	
	// �_�~�[�m�[�h�̑}��
	TV_INSERTSTRUCT td;
	td.hParent = hItem;
	td.hInsertAfter = TVI_LAST;
	td.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	td.item.iImage = IMG_FOLDER;
	td.item.iSelectedImage = IMG_FOLDER_SEL;
	td.item.pszText = TEXT("DUMMY");
	TreeView_InsertItem(hTree, &td);
}

//////////////////////////////////////////////////
// �m�[�h�̑I��
//////////////////////////////////////////////////
//�@�t�@�C���I���̏ꍇ�A�t�H���_��I�����Ă���ꍇ�ɂ�OK�{�^����disable�ɂ���B

void FileSelector::ItemSelect(HWND hDlg, HWND hTree, HTREEITEM hItem)
{
	HWND hOk = GetDlgItem(hDlg, IDOK);
	BOOL bEnable;

	if (pExtension) {
		TV_ITEM ti;
		ti.mask = TVIF_HANDLE | TVIF_IMAGE;
		ti.hItem = hItem;
		TreeView_GetItem(hTree, &ti);
		bEnable = (ti.iImage == IMG_ARTICLE);
	} else {
		bEnable = TRUE;
	}
	EnableWindow(hOk, bEnable);
}

#endif // PLATFORM_WIN32