#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#if defined(PLATFORM_PKTPC)
#include <aygshell.h>
#endif

#include "Tombo.h"
#include "MemoSelectView.h"
#include "resource.h"

#include "UniConv.h"
#include "MemoManager.h"
#include "MainFrame.h"
#include "MemoNote.h"
#include "Property.h"
#include "TString.h"
#include "TreeViewItem.h"
#include "PasswordManager.h"
#include "SearchEngine.h"
#include "Message.h"

// IDB_MEMOSELECT_IMAGES�Ɋi�[����Ă���C���[�W1�̃T�C�Y
#define IMAGE_CX 16
#define IMAGE_CY 16

// IDB_MEMOSELECT_IMAGES�Ɋi�[����Ă���C���[�W��
#define NUM_BITMAPS 10

void SelectViewSetWndProc(WNDPROC wp, HWND hParent, HINSTANCE h, MemoSelectView *p);
LRESULT CALLBACK NewSelectViewProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

#define ROOT_NODE_NAME MSG_MEMO

/////////////////////////////////////////
// 
/////////////////////////////////////////

static HTREEITEM InsertFolder(HWND hTree, HTREEITEM hParent, LPCTSTR pName);
static BOOL InsertFile(HWND hTree, HTREEITEM hParent, LPCTSTR pPrefix, LPCTSTR pName);
static void InsertDummyNode(HWND hTree, HTREEITEM hItem);
static LPTSTR GeneratePath(HWND hTree, HTREEITEM hItem, LPTSTR pBuf, DWORD nSiz);

/////////////////////////////////////////
// �E�B���h�E����
/////////////////////////////////////////

BOOL MemoSelectView::Create(LPCTSTR pName, RECT &r, HWND hParent, DWORD nID, HINSTANCE hInst, HFONT hFont)
{
	DWORD nWndStyle;
	nWndStyle = WS_CHILD | WS_VSCROLL | WS_HSCROLL | TVS_HASLINES | TVS_HASBUTTONS | TVS_SHOWSELALWAYS | TVS_EDITLABELS;

#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	hViewWnd = CreateWindowEx(WS_EX_CLIENTEDGE, WC_TREEVIEW, pName, nWndStyle,
								r.left, r.top, r.right, r.bottom,
								hParent, (HMENU)nID, hInst, this);
#else
	hViewWnd = CreateWindow(WC_TREEVIEW, pName, nWndStyle,
							r.left, r.top, r.right, r.bottom, 
							hParent, (HMENU)nID, hInst, this);
#endif
	if (hViewWnd == NULL) return FALSE;

#if !defined(PLATFORM_PSPC) && !defined(PLATFORM_BE500)
	WNDPROC wp = (WNDPROC)GetWindowLong(hViewWnd, GWL_WNDPROC);
	SelectViewSetWndProc(wp, hParent, g_hInstance, this);
	SetWindowLong(hViewWnd, GWL_WNDPROC, (LONG)NewSelectViewProc);
#endif

	// Create Imagelist.
	if ((hImageList = ImageList_Create(IMAGE_CX, IMAGE_CY, ILC_MASK, NUM_BITMAPS, 0)) == NULL) return FALSE;
	HBITMAP hBmp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_MEMOSELECT_IMAGES));

	// Transparent color is GREEN
	COLORREF rgbTransparent = RGB(0,255,0);
	ImageList_AddMasked(hImageList, hBmp, rgbTransparent);
	DeleteObject(hBmp);
	TreeView_SetImageList(hViewWnd, hImageList, TVSIL_NORMAL);

	// �t�H���g�ݒ�
	if (hFont != NULL) {
		SendMessage(hViewWnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	}
	return TRUE;
}

/////////////////////////////////////////
// �m�[�h�̑}��
/////////////////////////////////////////
//
// �m�[�h���\�[�g���đ}������B
// hInsertAfter�ɂ��Ă͐ݒ肷��K�v�͂Ȃ��B
static BOOL IsExpand(HWND hTree, HTREEITEM hItem);

static HTREEITEM InsertNode(HWND hTree, TV_INSERTSTRUCT *ti)
{
	LPCTSTR pInsertStr = ti->item.pszText;
	DWORD n = _tcslen(pInsertStr);
	TString compBuf;

	TreeViewItem *pInsItem = (TreeViewItem*)ti->item.lParam;
	DWORD nInsOrder = pInsItem->ItemOrder();

	TreeViewItem *pCompareItem;
	DWORD nCompOrder;

	if (!compBuf.Alloc(n + 1)) {
		return NULL;
	}

	if (ti->hParent != TVI_ROOT) {
		HTREEITEM h;
		HTREEITEM hIns;

		TV_ITEM t;
		t.mask = TVIF_TEXT | TVIF_PARAM;
		t.cchTextMax = n;
		t.pszText = compBuf.Get();

		hIns = TVI_FIRST;

		h = TreeView_GetChild(hTree, ti->hParent);
		while(h) {
			t.hItem = h;
			TreeView_GetItem(hTree, &t);
			pCompareItem = (TreeViewItem*)t.lParam;

			nCompOrder = 0;
			if (pCompareItem) {
				nCompOrder = pCompareItem->ItemOrder();
			} else {
				nCompOrder = -1;
			}

			if (nInsOrder < nCompOrder) break;
			if (nInsOrder == nCompOrder && _tcsicmp(t.pszText, pInsertStr) > 0) break;

			hIns = h;
			h = TreeView_GetNextSibling(hTree, h);
		}
		ti->hInsertAfter = hIns;
	} else {
		ti->hInsertAfter = TVI_LAST;
	}
	HTREEITEM hInserted = TreeView_InsertItem(hTree, ti);
	return hInserted;
}

/////////////////////////////////////////
// �t�@�C���m�[�h�̍쐬
/////////////////////////////////////////

static BOOL InsertFile(HWND hTree, HTREEITEM hParent, LPCTSTR pPrefix, LPCTSTR pFile)
{
	TCHAR path[MAX_PATH];
	TCHAR disp[MAX_PATH];

	_tcscpy(disp, pFile);

	TV_INSERTSTRUCT ti;
	ti.hParent = hParent;
	ti.hInsertAfter = TVI_LAST;
	ti.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;

	DWORD len = _tcslen(pFile);
	if (len <= 4) return TRUE;

	MemoNote *pNote;

	if (_tcsicmp(pFile + len - 4, TEXT(".txt")) == 0) {
		pNote = new PlainMemoNote();

		ti.item.iImage = ti.item.iSelectedImage = IMG_ARTICLE;
	} else if (_tcsicmp(pFile + len - 4, TEXT(".chi")) == 0) {
		pNote = new CryptedMemoNote();

		ti.item.iImage = ti.item.iSelectedImage = IMG_ARTICLE_ENCRYPTED;
	} else {
		// *.txt, *.chi�ȊO�͂Ȃɂ�����
		return TRUE;
	}

	if (pNote == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}

	wsprintf(path, TEXT("%s%s"), pPrefix, pFile);
	if (!pNote->Init(path)) return FALSE;

	TreeViewFileItem *ptvi = new TreeViewFileItem();
	if (!ptvi) {
		delete pNote;
		return FALSE;
	}
	ptvi->SetNote(pNote);
	ti.item.lParam = (LPARAM)ptvi;

	*(disp + len - 4) = TEXT('\0');
	ti.item.pszText = disp;
	pNote->SetViewItem(InsertNode(hTree, &ti));
	return TRUE;
}

// pMatchPath�͖�����"\\*.*"�`���ƂȂ��Ă��邱��
static BOOL LoadNodeUnderItem(HWND hTree, HTREEITEM hParent, LPCTSTR pMatchPath, LPCTSTR pPrefix)
{
	WIN32_FIND_DATA wfd;

	HANDLE hHandle = FindFirstFile(pMatchPath, &wfd);
	if (hHandle != INVALID_HANDLE_VALUE) {
		do {
			if (_tcscmp(wfd.cFileName, TEXT(".")) == 0 || _tcscmp(wfd.cFileName, TEXT("..")) == 0) continue;

			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// �t�H���_
				InsertFolder(hTree, hParent, wfd.cFileName);
			} else {
				if (!InsertFile(hTree, hParent, pPrefix, wfd.cFileName)) {
					FindClose(hHandle);
					return FALSE;
				}
			}
		} while(FindNextFile(hHandle, &wfd));
		FindClose(hHandle);
	}
	return TRUE;
}

BOOL MemoSelectView::LoadItems()
{
	DeleteAllItem();
	HTREEITEM hRoot = InsertFolder(hViewWnd, TVI_ROOT, ROOT_NODE_NAME);

	// ���[�g�m�[�h�̓W�J
	TreeView_Expand(hViewWnd, hRoot, TVE_EXPAND);
	return TRUE;
}

/////////////////////////////////////////
// ���ݑ��݂���S�A�C�e�����폜����
/////////////////////////////////////////
// �֘A�t�����Ă���TreeViewItem��delete����

void MemoSelectView::DeleteItemsRec(HTREEITEM hFirst)
{
	HTREEITEM hItem = hFirst;
	HTREEITEM hNext, hChild;
	TV_ITEM ti;
	MemoNote *p;
	HWND hTree = hViewWnd;

	ti.mask = TVIF_TEXT | TVIF_PARAM;
	TCHAR buf[1024];

	while(hItem) {

		ti.hItem = hItem;
		ti.pszText = buf;
		ti.cchTextMax = 1024;
		TreeView_GetItem(hTree, &ti);

		TreeViewItem *tvi;
		tvi = (TreeViewItem *)ti.lParam;
		p = NULL;
		if (tvi && !tvi->HasMultiItem()) {
			p = ((TreeViewFileItem *)tvi)->GetNote();
		}

		hChild = TreeView_GetChild(hTree, hItem);
		if (hChild) {
			DeleteItemsRec(hChild);
		}

		hNext = TreeView_GetNextSibling(hTree, hItem);

		if (pClipItem == tvi) pClipItem = NULL;
		// �폜����
		delete p;
		delete tvi;
		TreeView_DeleteItem(hTree, hItem);

		hItem = hNext;
	}
}

BOOL MemoSelectView::DeleteAllItem()
{
	DeleteItemsRec(TreeView_GetRoot(hViewWnd));
	pClipItem = NULL;
	return TRUE;
}

/////////////////////////////////////////
// �c���[���J���Ă��邩�ǂ����H
/////////////////////////////////////////

static BOOL IsExpand(HWND hTree, HTREEITEM hItem)
{
	TV_ITEM ti;
	ti.hItem = hItem;
	ti.mask = TVIF_STATE;
	ti.state = TVIS_EXPANDED;
	ti.stateMask = TVIS_EXPANDED;
	TreeView_GetItem(hTree, &ti);
	return ti.state & TVIS_EXPANDED;
}

/////////////////////////////////////////
// ���ݑI������Ă���A�C�e���̎擾
/////////////////////////////////////////

TreeViewItem* MemoSelectView::GetCurrentItem(HTREEITEM *pItem)
{
	HTREEITEM hItem = TreeView_GetSelection(hViewWnd);
	if (hItem == NULL) return NULL;

	TV_ITEM it;
	it.mask = TVIF_PARAM;
	it.hItem = hItem;
	if (TreeView_GetItem(hViewWnd, &it)) {
		if (pItem) *pItem = hItem;
		TreeViewItem *tvi = (TreeViewItem*)it.lParam;
		return tvi;
	}
	return NULL;
}


BOOL MemoSelectView::Show(int nCmdShow)
{
	ShowWindow(hViewWnd, nCmdShow);
#if defined(PLATFORM_PKTPC)
	// �^�b�v&�z�[���h���j���[���o���܂܉�ʂ��؂�ւ�����ۂɃ��j���[���������
	if (nCmdShow == SW_HIDE) {
		ReleaseCapture();
	}
#endif
	return UpdateWindow(hViewWnd);
}

void MemoSelectView::SetFocus()
{
	::SetFocus(hViewWnd);
}

///////////////////////////////////////////
// OnNotify�n���h��
///////////////////////////////////////////
//
// ���ʂƂ���TRUE, FALSE�̈ӎv�\�����������ꍇ�ɂ͂��̒l��Ԃ��B
// �f�t�H���g�̓���ɏ]�������ꍇ�ɂ�0xFFFFFFFF��Ԃ��B


LRESULT MemoSelectView::OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	LPNM_TREEVIEW pHdr = (LPNM_TREEVIEW)lParam;

#if defined(PLATFORM_PKTPC)
	NMRGINFO *pnmrginfo = (PNMRGINFO)lParam;
	if (pnmrginfo->hdr.code == GN_CONTEXTMENU) {
		// �^�b�v&�z�[���h����

		TV_HITTESTINFO hti;
		hti.pt = pnmrginfo->ptAction;
		TreeView_HitTest(hWnd, &hti);
		TreeView_SelectItem(hWnd, hti.hItem);

		HMENU hX;
		HMENU hPopup;
		hX = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_MENUBAR1));
		hPopup = GetSubMenu(hX, 1);
		TrackPopupMenuEx(hPopup, 0, pnmrginfo->ptAction.x, pnmrginfo->ptAction.x, hWnd, NULL);
		DestroyMenu(hX);
		return 0;
	}
#endif

	switch(pHdr->hdr.code) {
	case NM_DBLCLK:
		{
			HTREEITEM hItem = TreeView_GetSelection(hViewWnd);
			TV_ITEM it;
			it.mask = TVIF_PARAM;
			it.hItem = hItem;

			TreeView_GetItem(hViewWnd, &it);
			TreeViewItem *tvi = (TreeViewItem*)it.lParam;
			if (!tvi) break;

			// �V���O���N���b�N���[�h�E�����Ǐ]���[�h�̏ꍇ�A2�񃍁[�h����邱�ƂɂȂ邪�A�Ƃ肠�������̂܂�
			// View�̐؂�ւ������ł�SelectView�Ƀt�H�[�J�X���������Ă��܂��B�Ȃ��H
			if (!tvi->HasMultiItem()) {
				PostMessage(hWnd, MWM_OPEN_REQUEST, (WPARAM)OPEN_REQUEST_MSVIEW_ACTIVE, (LPARAM)(TreeViewFileItem*)tvi);
			}
			// �Öق�Expand/Collapse������
		}
		break;
	case NM_RETURN:
		{
			// DBLCLK�ƈႢ�A�[���I�Ƀc���[�̊J�����𔭐�������
			HTREEITEM hItem = TreeView_GetSelection(hViewWnd);
			TV_ITEM it;
			it.mask = TVIF_PARAM | TVIF_STATE;
			it.hItem = hItem;
			it.stateMask = TVIS_EXPANDED;
			TreeView_GetItem(hViewWnd, &it);

			TreeViewItem *tvi = (TreeViewItem*)it.lParam;
			if (!tvi) break;

			if (tvi->HasMultiItem()) {
				ToggleExpandFolder(hItem, it.state & TVIS_EXPANDED);
			} else {
				SendMessage(hWnd, MWM_OPEN_REQUEST, (WPARAM)OPEN_REQUEST_MSVIEW_ACTIVE, (LPARAM)(TreeViewFileItem*)tvi);
			}
		}
		break;
	case TVN_SELCHANGING:
		{
			NMTREEVIEW *p = (LPNMTREEVIEW)lParam;
			TreeViewItem *tvi = (TreeViewItem*)((p->itemOld).lParam);

			MemoNote *pNote;
			pNote = NULL;
			if (tvi && !tvi->HasMultiItem()) {
				pNote = ((TreeViewFileItem*)tvi)->GetNote();
			}
			if (pNote == NULL) return FALSE;

#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
			// 2-Pane View�̏ꍇ�ɂ� �����̐؂�ւ����������A����
			// �ۑ��m�F�ŃL�����Z�����������ꍇ�A�A�C�e���̐؂�ւ���
			// �F�߂Ă͂Ȃ�Ȃ�
			if (g_Property.IsUseTwoPane()) {			
				// �����̐؂�ւ�������
				DWORD nYNC;
				if (pMemoMgr && 
					(pNote == pMemoMgr->CurrentNote() || pMemoMgr->CurrentNote() == NULL)) {
						// ���if���̑�2�����͈ꋓ�ɕ�����SELCHANGING�����邽��
					if (!pMemoMgr->SaveIfModify(&nYNC, FALSE)) {
						// �Z�[�u�Ɏ��s...
	
						// �؂�ւ�����ƍ���̂őj�~
						return TRUE;
					}
					switch(nYNC) {
					case IDCANCEL:
						// �؂�ւ��Ăق����Ȃ��ƃ��[�U�������Ă���̂őj�~
						return TRUE;
					case IDOK:
						/* fall through */
					case IDYES:
						return FALSE;
					case IDNO:
						// �f�[�^�̔j��
						pMemoMgr->ClearMemo();
						return FALSE;
					}
				}
			}
#endif
			return FALSE;
		}

	case TVN_SELCHANGED:
		{
			NMTREEVIEW *p = (LPNMTREEVIEW)lParam;
			TreeViewItem *tvi = (TreeViewItem*)((p->itemNew).lParam);
			if (tvi == NULL) {
				break;
			}

			MemoNote *pNote;
			if (tvi->HasMultiItem()) {
				pNote = NULL;
			} else {
				pNote = ((TreeViewFileItem*)tvi)->GetNote();
			}

			// ���j���[�̐���
			pMemoMgr->SelectNote(pNote);

			if (g_Property.IsUseTwoPane() && (p->action == TVC_BYMOUSE || p->action == TVC_BYKEYBOARD)) {
				// ���[�U����̏ꍇ�A�����̐؂�ւ��𔭐�������
				// �v���O�����ɂ����̂̏ꍇ�A�؂�ւ��͔��������Ȃ�
				pMemoMgr->SetMSSearchFlg(TRUE);
				if (!tvi->HasMultiItem()) {
					PostMessage(hWnd, MWM_OPEN_REQUEST, (WPARAM)OPEN_REQUEST_MDVIEW_ACTIVE, (LPARAM)(TreeViewFileItem*)tvi);
				}
			}
		}
		break;
	case TVN_ITEMEXPANDING:
		if (pHdr->action == TVE_EXPAND) {
			TreeExpand(pHdr->itemNew.hItem);
		} else {
			TreeCollapse(pHdr->itemNew.hItem);
		}
		return FALSE;

	case TVN_ENDLABELEDIT:
		return EditLabel(&(((LPNMTVDISPINFO)lParam)->item));
	}
	// WM_NOTIFY�̕ԓ��ɂȂ邽�߁A�K�v�Ȃ��ꍇ�ȊO�͊eNotify��return���邱��
	return 0xFFFFFFFF;
}

///////////////////////////////////////////
// OnCommand�n���h��
///////////////////////////////////////////

BOOL MemoSelectView::OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HTREEITEM  hItem;
	TreeViewItem *pItem = GetCurrentItem(&hItem);

	switch(LOWORD(wParam)) {
	case IDM_ENCRYPT:
		if (!pItem->Encrypt(pMemoMgr, this)) {
			TomboMessageBox(hWnd, MSG_ENCRYPTION_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
		}
		return TRUE;
	case IDM_DECRYPT:
		if (!pItem->Decrypt(pMemoMgr, this)) {
			TomboMessageBox(hWnd, MSG_DECRYPTION_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
		}
		return TRUE;
	case IDM_DELETEITEM:
		DeleteNode(hWnd, hItem, pItem);
		return TRUE;
	case IDM_ACTIONBUTTON:
		OnActionButton(hWnd);
		return TRUE;
	case IDM_CUT:
		OnCut(hWnd);
		return TRUE;
	case IDM_COPY:
		OnCopy(hWnd);
		return TRUE;
	case IDM_PASTE:
		OnPaste(hWnd);
		return TRUE;
	case IDM_RENAME:
		OnEditLabel();
		return TRUE;
	}
	return FALSE;
}

///////////////////////////////////////////
// HotKey�̏���
///////////////////////////////////////////

BOOL MemoSelectView::OnHotKey(HWND hWnd, WPARAM wParam)
{
	switch(wParam) {
	case APP_BUTTON1:
		/* fall through */
	case APP_BUTTON2:
		/* fall through */
	case APP_BUTTON3:
		/* fall through */
	case APP_BUTTON4:
		/* fall through */
	case APP_BUTTON5:
		OnActionButton(hWnd);
		return TRUE;
	default:
		return FALSE;
	}
}

///////////////////////////////////////////
// �E�B���h�E�T�C�Y�擾
///////////////////////////////////////////

void MemoSelectView::GetSize(LPWORD pWidth, LPWORD pHeight)
{
	RECT r;
	GetWindowRect(hViewWnd, &r);
	*pWidth = (WORD)(r.right - r.left);
	*pHeight = (WORD)(r.bottom - r.top);
}

///////////////////////////////////////////
// �����̊K�w�̎擾
///////////////////////////////////////////

LPTSTR MemoSelectView::GeneratePath(TreeViewItem *p, LPTSTR pBuf, DWORD nSiz)
{
	return ::GeneratePath(hViewWnd, p->GetViewItem(), pBuf, nSiz);
}

static LPTSTR GeneratePath(HWND hTree, HTREEITEM hItem, LPTSTR pBuf, DWORD nSiz)
{
	LPTSTR p = pBuf + nSiz - 2;
	*(p+1) = TEXT('\0');

	LPTSTR pPrev;

	TV_ITEM it;
	TCHAR buf[MAX_PATH];

	HTREEITEM h = hItem;
	it.mask = TVIF_HANDLE | TVIF_TEXT;
	it.pszText = buf;

	while(h) {
		it.hItem = h;
		it.cchTextMax = MAX_PATH;
		TreeView_GetItem(hTree, &it);

		DWORD l = _tcslen(buf);
		pPrev = p + 1;
		*p-- = TEXT('\\');
		p -= l - 1;
		_tcsncpy(p, buf, l);
		p--;
		h = TreeView_GetParent(hTree, h);
	}
	return pPrev;

}

void MemoSelectView::TreeExpand(HTREEITEM hItem)
{
	HWND hTree = hViewWnd;

	// �_�~�[�̎q�m�[�h���폜
	HTREEITEM di = TreeView_GetChild(hTree, hItem);
	DeleteItemsRec(di);

	// �C���[�W��W�J���Ă�����̂ɕύX
	TreeViewItem *pItem = GetTVItem(hItem);
	SetIcon(pItem, MEMO_VIEW_STATE_OPEN_SET);

	// �t���p�X���擾
	TCHAR buf[MAX_PATH];	// �����̃��[�g����̑��΃p�X
	TCHAR buf2[MAX_PATH];
	LPTSTR pPath = ::GeneratePath(hTree, hItem, buf, MAX_PATH);
	wsprintf(buf2, TEXT("%s\\%s*.*"), g_Property.TopDir(), pPath);

	LoadNodeUnderItem(hTree, hItem, buf2, pPath);

	// ���ʂ̃m�[�h�����݂��Ȃ��ꍇ�ɂ͍ēx����
	if (TreeView_GetChild(hTree, hItem) == NULL) {
		TreeCollapse(hItem);
	}
}

void MemoSelectView::TreeCollapse(HTREEITEM hItem)
{
	HWND hTree = hViewWnd;
	HTREEITEM h = TreeView_GetChild(hTree, hItem);

	// �m�[�h�̍폜
	DeleteItemsRec(h);

	// �_�~�[�̃m�[�h��}��
	InsertDummyNode(hTree, hItem);

	// �C���[�W����Ă�����̂ɕύX
	TreeViewItem *pItem = GetTVItem(hItem);
	SetIcon(pItem, MEMO_VIEW_STATE_OPEN_CLEAR); 

	TreeView_Expand(hViewWnd, hItem, TVE_COLLAPSE | TVE_COLLAPSERESET);
}

///////////////////////////////////////////////////////
// �V�K�m�[�h�̍쐬
///////////////////////////////////////////////////////
// �����̐V�K�쐬�ɑΉ����ăm�[�h��ǉ�����B

TreeViewFileItem *MemoSelectView::NewMemoCreated(MemoNote *pNote, LPCTSTR pHeadLine, HTREEITEM hParent)
{
	TV_INSERTSTRUCT ti;
	ti.hParent = hParent ? hParent : TVI_ROOT;
	ti.hInsertAfter = TVI_LAST;
	ti.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	
	ti.item.pszText = (LPTSTR)pHeadLine;
	ti.item.iImage = ti.item.iSelectedImage = pNote->GetMemoIcon();

	TreeViewFileItem *tvi = new TreeViewFileItem();
	if (tvi == NULL) return FALSE;
	tvi->SetNote(pNote);
	ti.item.lParam = (LPARAM)tvi;

	HTREEITEM hItem = ::InsertNode(hViewWnd, &ti);

	pNote->SetViewItem(hItem);
	return tvi;
}

///////////////////////////////////////////
// �E�B���h�E�T�C�Y�̈ړ�
///////////////////////////////////////////

void MemoSelectView::MoveWindow(DWORD x, DWORD y, DWORD nWidth, DWORD nHeight)
{
	::MoveWindow(hViewWnd, x, y, nWidth, nHeight, TRUE);
}

/////////////////////////////////////////////////////////////
// TreeViewItem�̏�Ԃ��ς�������Ƃɂ��r���[�ւ̕ύX�˗�
/////////////////////////////////////////////////////////////
// �A�C�R����ԁE�w�b�h���C�������񂪕ύX�ɂȂ����ꍇ��
// TreeViewItem����R�[�������B����ɉ����ăA�C�R����Ԃ����
// �}������Ă���A�C�e���̈ʒu���X�V����B
// �V����Node��pItem�Ɗ֘A�Â�����

BOOL MemoSelectView::UpdateItemStatus(TreeViewItem *pItem, LPCTSTR pNewHeadLine)
{
	// ����HTREEITEM���擾
	HTREEITEM hOrigNode = pItem->GetViewItem();

	// �e�m�[�h�̎擾
	HTREEITEM hParent = TreeView_GetParent(hViewWnd, hOrigNode);
	if (hParent == NULL) {
		hParent = TVI_ROOT;
	}

	// ��U�폜
	TreeView_DeleteItem(hViewWnd, hOrigNode);

	// �A�C�e���̑}��
	TV_INSERTSTRUCT ti;
	ti.hParent = hParent;
	ti.hInsertAfter = TVI_LAST;
	ti.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	ti.item.iImage = ti.item.iSelectedImage = pItem->GetIcon(this, 0); // ����̃A�C�R�����擾
	ti.item.pszText = (LPTSTR)pNewHeadLine;
	ti.item.lParam = (LPARAM)pItem;

	HTREEITEM hItem = ::InsertNode(hViewWnd, &ti);
	pItem->SetViewItem(hItem);
	return TRUE;
}

///////////////////////////////////////////
// �����̃r���[����̍폜
///////////////////////////////////////////
// p�͑Ή�����HTREEITEM�������Ȃ��Ȃ邽�߁Ap��ViewItem��NULL�ɐݒ肳���B

BOOL MemoSelectView::DeleteItem(TreeViewItem *p)
{
	HTREEITEM hItem = p->GetViewItem();

	HTREEITEM hParent = TreeView_GetParent(hViewWnd, hItem);

	// �q����������m�[�h�̍폜& TreeViewItem, MemoNote ��delete���s��
	HTREEITEM hChild = TreeView_GetChild(hViewWnd, hItem);
	DeleteItemsRec(hChild);

	TreeView_DeleteItem(hViewWnd, hItem);
	p->SetViewItem(NULL);

	// �e�m�[�h�Ɍ��ʂƂ��ăm�[�h�����݂��Ȃ��Ȃ����ꍇ�ɂ�Collapse���Ă���
	if (TreeView_GetChild(hViewWnd, hParent) == NULL) {
		TreeCollapse(hParent);
	}
	return TRUE;
}

///////////////////////////////////////////
// �A�N�V�����{�^���ɑ΂��鏈��
///////////////////////////////////////////

void MemoSelectView::OnActionButton(HWND hWnd)
{
	HTREEITEM hItem;
	TreeViewItem *p = GetCurrentItem(&hItem);
	if (p && !p->HasMultiItem()) {
		// �����̏ꍇ�ɂ�MainFrame�ɑ΂��ăI�[�v���v�����o��
		SendMessage(hWnd, MWM_OPEN_REQUEST, (WPARAM)OPEN_REQUEST_MSVIEW_ACTIVE, (LPARAM)(TreeViewFileItem*)p);
		return;
	}
	
	RECT r;
	TreeView_GetItemRect(hViewWnd, hItem, &r, TRUE);
	PostMessage(hViewWnd, WM_LBUTTONDBLCLK, MK_LBUTTON, MAKELPARAM(r.left, r.top));
}

///////////////////////////////////////////
// �I�����Ă���A�C�e���̃p�X�̎擾
///////////////////////////////////////////

HTREEITEM MemoSelectView::GetPathForNewItem(TString *pPath)
{
	TCHAR buf[MAX_PATH];
	HTREEITEM hItem = NULL;

	TreeViewItem *pItem = GetCurrentItem(&hItem);
	MemoNote *pNote = NULL;
	if (pItem && !pItem->HasMultiItem()) {
		pNote = ((TreeViewFileItem*)pItem)->GetNote();
	}
	if (hItem == NULL) {
		if (!pPath->Alloc(1)) return NULL;
		_tcscpy(pPath->Get(), TEXT(""));

		return TreeView_GetRoot(hViewWnd);
	}

	if (pNote != NULL) {
		hItem = TreeView_GetParent(hViewWnd, hItem);
	}

	LPTSTR pPathTop = ::GeneratePath(hViewWnd, hItem, buf, MAX_PATH);
	if (!pPath->Set(pPathTop)) return NULL;
	return hItem;
}

///////////////////////////////////////////
// �����E�t�H���_�̍폜
///////////////////////////////////////////

void MemoSelectView::DeleteNode(HWND hWnd, HTREEITEM hItem, TreeViewItem *pItem)
{
	if (hItem == NULL) return;

	if (pItem->Delete(pMemoMgr, this)) {
		delete pItem;
	}
}

///////////////////////////////////////////
// �w�肳�ꂽ�A�C�e����I����ԂƂ���
///////////////////////////////////////////

void MemoSelectView::SelectNote(MemoNote *pNote)
{
	if (!pNote) return;
	HTREEITEM hItem = pNote->GetViewItem();
	TreeView_SelectItem(hViewWnd, hItem);
}

///////////////////////////////////////////
// �t�H���_�̑}��
///////////////////////////////////////////

BOOL MemoSelectView::CreateNewFolder(HTREEITEM hItem, LPCTSTR pFolder)
{
	// �c���[�����Ă���ꍇ�ɂ͊J��
	if (!IsExpand(hViewWnd, hItem) && (TreeView_GetChild(hViewWnd, hItem) != NULL)) {
		// �J�����Ƃō쐬�����t�H���_�͎����I�ɓǂݍ��܂��
		TreeView_Expand(hViewWnd, hItem, TVE_EXPAND);
	} else {
		// �J���Ă���ꍇ�ɂ͑}��
		InsertFolder(hViewWnd, hItem, pFolder);
		if (TreeView_GetChild(hViewWnd, hItem) != NULL) {
			TreeView_Expand(hViewWnd, hItem, TVE_EXPAND);
		}
	}
	return TRUE;
}

/////////////////////////////////////////
// �t�H���_�̑}��
/////////////////////////////////////////

static HTREEITEM InsertFolder(HWND hTree, HTREEITEM hParent, LPCTSTR pName)
{
	TV_INSERTSTRUCT ti;
	ti.hParent = hParent;
	ti.hInsertAfter = TVI_LAST;
	ti.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	ti.item.pszText = (LPTSTR)pName;
	ti.item.iImage = IMG_FOLDER;
	ti.item.iSelectedImage = IMG_FOLDER;
	TreeViewFolderItem *tvi = new TreeViewFolderItem();
	ti.item.lParam = (LPARAM)tvi;

	HTREEITEM hItem = InsertNode(hTree, &ti);
	tvi->SetViewItem(hItem);

	// �_�~�[�m�[�h
	InsertDummyNode(hTree, hItem);
	return hItem;
}

/////////////////////////////////////////
// �w�b�h���C��������̕ύX
/////////////////////////////////////////
// �ă\�[�g���K�v�ƂȂ邽�߁A��U�폜���čđ}���E�\�[�g���Ă���

BOOL MemoSelectView::UpdateHeadLine(MemoNote *pNote, LPCTSTR pHeadLine)
{
	HTREEITEM hItem = pNote->GetViewItem();
	if (hItem == NULL) return FALSE;

#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	HTREEITEM hCurrentSel;
	if (g_Property.IsUseTwoPane()) {
		hCurrentSel = TreeView_GetSelection(hViewWnd);
	}
#endif

	// �e�m�[�h�̎擾
	HTREEITEM hParent = TreeView_GetParent(hViewWnd, hItem);
	// ���݂̃m�[�h�̍폜
	TreeView_DeleteItem(hViewWnd, hItem);

	// �V�����m�[�h�̑}���EMemoNote�Ƃ̊֘A�t��
	TV_INSERTSTRUCT ti;
	ti.hParent = hParent;
	ti.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	ti.item.pszText = (LPTSTR)pHeadLine;
	ti.item.iImage = ti.item.iSelectedImage = pNote->GetMemoIcon();

	TreeViewFileItem *tvi = new TreeViewFileItem();
	tvi->SetNote(pNote);
	ti.item.lParam = (LPARAM)tvi;

	if (IsExpand(hViewWnd, hParent)) {
		pNote->SetViewItem(::InsertNode(hViewWnd, &ti));
		TreeView_SelectItem(hViewWnd, pNote->GetViewItem());
	} else {
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
		if (g_Property.IsUseTwoPane()) {
			// �폜�E�}���ɂ��I������Ă���A�C�e�����ς��̂Ŗ߂�
			TreeView_SelectItem(hViewWnd, hCurrentSel);
		}
#endif
	}
	return TRUE;
}


BOOL MemoSelectView::GetHeadLine(MemoNote *pNote, LPTSTR pHeadLine, DWORD nLen)
{
	HTREEITEM hItem = pNote->GetViewItem();
	if (hItem == NULL) return FALSE;

	TCHAR buf[MAX_PATH];

	TV_ITEM ti;
	ti.mask = TVIF_TEXT;
	ti.hItem = hItem;
	ti.pszText = buf;
	ti.cchTextMax = MAX_PATH;
	TreeView_GetItem(hViewWnd, &ti);

	CopyKanjiString(pHeadLine, buf, nLen);

	return TRUE;
}

/////////////////////////////////////////
// �A�C�R���̐ݒ�
/////////////////////////////////////////

void MemoSelectView::SetIcon(TreeViewItem *ptv, DWORD nStatus)
{
	DWORD nImage;

	nImage = ptv->GetIcon(this, nStatus);

	TV_ITEM ti;
	ti.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	ti.hItem = ptv->GetViewItem();
	ti.iImage = ti.iSelectedImage = nImage;
	TreeView_SetItem(hViewWnd, &ti);
}

/////////////////////////////////////////
// �[���N���b�v�{�[�h�ւ̐ݒ�
/////////////////////////////////////////

void MemoSelectView::SetShareArea()
{
	HTREEITEM hItem;
	TreeViewItem *pItem;
	if ((pItem = GetCurrentItem(&hItem)) == NULL) return;

	if (pClipItem) {
		SetIcon(pClipItem, MEMO_VIEW_STATE_CLIPED_CLEAR);
	}
	pClipItem = pItem;
	if (pClipItem) {
		SetIcon(pClipItem, MEMO_VIEW_STATE_CLIPED_SET);
	}
}

/////////////////////////////////////////
// �ړ����郁���̑I��
/////////////////////////////////////////

void MemoSelectView::OnCut(HWND hWnd)
{
	SetShareArea();
	bCut = TRUE;
}

/////////////////////////////////////////
// �R�s�[���郁���̑I��
/////////////////////////////////////////

void MemoSelectView::OnCopy(HWND hWnd)
{
	SetShareArea();
	bCut = FALSE;
}

/////////////////////////////////////////
// �ړ��E�R�s�[�̎��s
/////////////////////////////////////////

void MemoSelectView::OnPaste(HWND hWnd)
{
	if (pClipItem == NULL) return;

	if (bCut) {
		// �����̈ړ�
		if (!pClipItem->Move(pMemoMgr, this)) {
			TomboMessageBox(hViewWnd, MSG_MOVE_MEMO_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
			return;
		} else {
			delete pClipItem;
			pClipItem = NULL;
		}
	} else {
		// �����̃R�s�[
		if (!pClipItem->Copy(pMemoMgr, this)) {
			TomboMessageBox(hViewWnd, MSG_COPY_MEMO_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
		}
	}
}

/////////////////////////////////////////
// �t�H�[�J�X�̎擾
/////////////////////////////////////////

void MemoSelectView::OnGetFocus()
{
	MainFrame *pMf = pMemoMgr->GetMainFrame();
	if (pMf) {
		pMf->ActivateView(TRUE);
	}
}

/////////////////////////////////////////
// �t�H���g�̐ݒ�
/////////////////////////////////////////

void MemoSelectView::SetFont(HFONT hFont)
{
	SendMessage(hViewWnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
}

///////////////////////////////////////////
// �c���[�̊J��
///////////////////////////////////////////

void MemoSelectView::ToggleExpandFolder(HTREEITEM hItem, UINT status)
{
	if (status & TVIS_EXPANDED) {
		TreeCollapse(hItem);
	} else {
		TreeView_Expand(hViewWnd, hItem, TVE_EXPAND);
		TreeExpand(hItem);
	}
}

///////////////////////////////////////////
// 
///////////////////////////////////////////
#ifdef COMMENT
MemoNote *MemoSelectView::GetNote(TreeViewItem *p) 
{ 
	return p ? p->pNote : NULL; 
}
#endif
/////////////////////////////////////////////
//  HTREEITEM����TreeViewItem*�̎擾
/////////////////////////////////////////////

TreeViewItem *MemoSelectView::GetTVItem(HTREEITEM h)
{
	TV_ITEM ti;
	ti.mask = TVIF_PARAM;
	ti.hItem = h;
	TreeView_GetItem(hViewWnd, &ti);
	return (TreeViewItem*)ti.lParam;
}

/////////////////////////////////////////////
// HTREEITEM��TreeViewItem*��ݒ�
/////////////////////////////////////////////

BOOL MemoSelectView::SetTVItem(HTREEITEM h, TreeViewItem *p)
{
	TV_ITEM ti;
	ti.mask = TVIF_PARAM;
	ti.hItem = h;
	ti.lParam = (LPARAM)p;
	TreeView_SetItem(hViewWnd, &ti);
	return TRUE;
}

/////////////////////////////////////////////
//
/////////////////////////////////////////////

static void InsertDummyNode(HWND hTree, HTREEITEM hItem)
{
	// �_�~�[�m�[�h
	TV_INSERTSTRUCT tisub;
	tisub.hParent = hItem;
	tisub.hInsertAfter = TVI_LAST;
	tisub.item.mask = TVIF_PARAM;
	tisub.item.lParam = NULL;
	TreeView_InsertItem(hTree, &tisub);
}

/////////////////////////////////////////////
// �t�@�C�����̕ύX
/////////////////////////////////////////////
//
// �߂�l�F TRUE = ���x���ύX��accept
//			FALSE = ���x���ύX��reject

LRESULT MemoSelectView::EditLabel(TVITEM *pItem)
{
	if (pItem->pszText == NULL) return FALSE;

	TreeViewItem *pti = (TreeViewItem*)(pItem->lParam);
	if (pti == NULL) return FALSE;
	return pti->Rename(pMemoMgr, this, pItem->pszText);
}

/////////////////////////////////////////////
// ���̃m�[�h���擾
/////////////////////////////////////////////

static HTREEITEM GetNextItem(HWND hWnd, HTREEITEM hCurrentItem)
{
	HTREEITEM hPrev;
	HTREEITEM h = hCurrentItem;

	hPrev = h;
	h = TreeView_GetNextSibling(hWnd, h);

	// Sibling��NULL�̏ꍇ�A�e�̒��T���B�e�̒킪���Ȃ���ΐe�̐e�ł���ɒ���B
	while (h == NULL) {
		HTREEITEM hParent = TreeView_GetParent(hWnd, hPrev);
		if (hParent == NULL) break; // ���[�g�܂ŗ���

		hPrev = hParent;
		h = TreeView_GetNextSibling(hWnd, hParent);
	}
	return h;
}

static HTREEITEM GetPrevItem(HWND hWnd, HTREEITEM hCurrentItem)
{
	HTREEITEM hPrev;
	HTREEITEM h = hCurrentItem;

	hPrev = h;
	h = TreeView_GetPrevSibling(hWnd, h);

	while(h == NULL) {
		HTREEITEM hParent= TreeView_GetParent(hWnd, hPrev);
		if (hParent == NULL) break;

		hPrev = hParent;
		h = TreeView_GetPrevSibling(hWnd, hParent);
	}
	return h;
}

// �����q���擾
static  HTREEITEM GetLastChild(HWND hWnd, HTREEITEM hCurrentItem)
{
	HTREEITEM h = TreeView_GetChild(hWnd, hCurrentItem);
	HTREEITEM hPrev;

	hPrev = h;
	while(h) {
		hPrev = h;
		h = TreeView_GetNextSibling(hWnd, h);
	}
	return hPrev;
}

/////////////////////////////////////////////
// �y�[�W�Ԍ���
/////////////////////////////////////////////

BOOL MemoSelectView::Search(BOOL bFirstSearch, BOOL bForward)
{
	HTREEITEM  hItem, hOrigItem;

	SearchEngineA *pSE = pMemoMgr->GetSearchEngine();
	if (pSE == NULL) return TRUE;


	TreeViewItem *pOrigItem = GetCurrentItem(&hOrigItem);

	BOOL bSearchEncryptMemo = pSE->IsSearchEncryptMemo();
	BOOL bFileNameOnly = pSE->IsFileNameOnly();

	hItem = hOrigItem;

	// ���[�g�t�H���_�������Ă�����J���Ă���
	if (TreeView_GetParent(hViewWnd, hItem) == NULL && !IsExpand(hViewWnd, hItem)) {
		TreeView_Expand(hViewWnd, hItem, TVE_EXPAND);
		TreeExpand(hItem);
	}

	if (bSearchEncryptMemo && !bFileNameOnly) {
		// �p�X���[�h�`�F�b�N
		// �Ώۂ��t�@�C�����݂̂̏ꍇ�A�p�X���[�h�͕s�v�Ȃ̂ŏ�������͂���
		PasswordManager *pPM = pMemoMgr->GetPasswordManager();
		BOOL bCancel;
		const char *pPasswd;
		pPasswd = pPM->Password(&bCancel, FALSE);
		if (pPasswd == NULL && bCancel == TRUE) {
			return TRUE;
		}
	}

	HCURSOR hOrigCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

	if (!bFirstSearch) {
		if (bForward) {
			hItem = GetNextItem(hViewWnd, hItem);
		} else {
			hItem = GetPrevItem(hViewWnd, hItem);
		}
		if (hItem == NULL) {
			SetCursor(hOrigCursor);
			MessageBox(NULL, MSG_STRING_NOT_FOUND, TOMBO_APP_NAME, MB_OK | MB_ICONINFORMATION);
			return TRUE;
		}
	}

	BOOL bResult = SearchItems(hItem, bSearchEncryptMemo, bFileNameOnly, bForward);
	if (bResult) {
		TreeView_SelectItem(hViewWnd, hOrigItem);
		if (pOrigItem && !pOrigItem->HasMultiItem()) {
			pMemoMgr->GetMainFrame()->SendRequestOpen((TreeViewFileItem*)pOrigItem, OPEN_REQUEST_NO_ACTIVATE_VIEW);
		}
		MessageBox(NULL, MSG_STRING_NOT_FOUND, TOMBO_APP_NAME, MB_OK | MB_ICONINFORMATION);
	}

	SetCursor(hOrigCursor);
	return bResult;
}

/////////////////////////////////////////////
// �y�[�W�Ԍ�����1�t�@�C���ɑ΂��錟��

BOOL MemoSelectView::SearchOneItem(HTREEITEM hItem, BOOL bSearchEncryptedMemo, BOOL bFileNameOnly)
{
	TreeView_SelectItem(hViewWnd, hItem);

	TreeViewItem *pItem;
	pItem = GetTVItem(hItem);

	if (pItem) {
		BOOL bMatch;
		if (bFileNameOnly) {
			TCHAR buf[MAX_PATH];
			TV_ITEM ti;
			ti.mask = TVIF_TEXT;
			ti.hItem = hItem;
			ti.pszText = buf;
			ti.cchTextMax = MAX_PATH - 1;
			TreeView_GetItem(hViewWnd, &ti);

#ifdef _WIN32_WCE
			char bufA[MAX_PATH * 2];
			WideCharToMultiByte(CP_ACP, 0, buf, -1, bufA, MAX_PATH * 2 - 1, NULL, NULL);
			bMatch = pMemoMgr->GetSearchEngine()->SearchForward(bufA, 0, FALSE);
#else
			bMatch = pMemoMgr->GetSearchEngine()->SearchForward(buf, 0, FALSE);
#endif
		} else {
			// �Í����������ΏۊO�̏ꍇ�A�������Ȃ�
			MemoNote *pNote = NULL;
			if (pItem && !pItem->HasMultiItem()) {
				pNote = ((TreeViewFileItem*)pItem)->GetNote();
			}
			if (!bSearchEncryptedMemo && pNote->IsEncrypted()) return TRUE;

			char *pMemo = pNote->GetMemoBodyA(pMemoMgr->GetPasswordManager());
			if (pMemo == NULL) return TRUE;

			bMatch = pMemoMgr->GetSearchEngine()->SearchForward(pMemo, 0, FALSE);
			MemoNote::WipeOutAndDelete(pMemo);
		}

		// �q�b�g�����猟����ł��؂�
		if (bMatch) {
			if (pItem && !pItem->HasMultiItem()) {
				pMemoMgr->GetMainFrame()->SendRequestOpen((TreeViewFileItem*)pItem, OPEN_REQUEST_MSVIEW_ACTIVE);
			}
			if (!bFileNameOnly) pMemoMgr->SearchDetailsView(TRUE, TRUE, TRUE, TRUE);
			return FALSE;
		}
	}
	return TRUE;

}
BOOL MemoSelectView::SearchItems(HTREEITEM hTreeItem, BOOL bSearchEncryptedMemo, BOOL bFileNameOnly, BOOL bForward)
{
	HTREEITEM h = hTreeItem;
	BOOL bResult = TRUE;
	TreeViewItem *pItem;

	while(h && bResult) {
		pItem = GetTVItem(h);

		if (pItem) {
			if (pItem->HasMultiItem()) {
				// �T�u�A�C�e�������� = �t�H���_
				if (!IsExpand(hViewWnd, h)) {
					// �c���[�̓W�J
					TreeView_Expand(hViewWnd, h, TVE_EXPAND);
					TreeExpand(h);
				}
				// �ċA
				// GetChild����蒼���Ă���̂�Expand����邽��
				if (IsExpand(hViewWnd, h)) {
					if (bForward) {
						h = TreeView_GetChild(hViewWnd, h);
					} else {
						h = GetLastChild(hViewWnd, h);
					}
					continue;
				}
			} else {
				bResult = SearchOneItem(h, bSearchEncryptedMemo, bFileNameOnly);
			}
		}

		if (bForward) {
			h = GetNextItem(hViewWnd, h);
		} else {
			h = GetPrevItem(hViewWnd, h);
		}
	}
	return bResult;
}

/////////////////////////////////////////////
// ���x���̕ҏW
/////////////////////////////////////////////

BOOL MemoSelectView::OnEditLabel()
{
	HTREEITEM hItem = TreeView_GetSelection(hViewWnd);
	if (hItem == NULL) return FALSE;

	TreeView_EditLabel(hViewWnd, hItem);

	return TRUE;
}