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
#include "Message.h"

void SelectViewSetWndProc(WNDPROC wp, HWND hParent, HINSTANCE h, MemoSelectView *p);
LRESULT CALLBACK NewSelectViewProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

/////////////////////////////////////////
// 
/////////////////////////////////////////

static void InsertDummyNode(HWND hTree, HTREEITEM hItem);

/////////////////////////////////////////
// ウィンドウ生成
/////////////////////////////////////////

BOOL MemoSelectView::Create(LPCTSTR pName, RECT &r, HWND hParent, DWORD nID, HINSTANCE hInst, HFONT hFont, HIMAGELIST hImgList)
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

	TreeView_SetImageList(hViewWnd, hImgList, TVSIL_NORMAL);

	// フォント設定
	if (hFont != NULL) {
		SendMessage(hViewWnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	}
	return TRUE;
}

/////////////////////////////////////////
// Insert node
/////////////////////////////////////////
// Insert node with sorting.

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
// Insert file node
/////////////////////////////////////////

BOOL MemoSelectView::InsertFile(HTREEITEM hParent, LPCTSTR pPrefix, LPCTSTR pFile)
{
	MemoNote *pNote;
	if (!MemoNote::MemoNoteFactory(pPrefix, pFile, &pNote)) return FALSE;
	if (pNote == NULL) return TRUE;

	TCHAR disp[MAX_PATH];
	DWORD len = _tcslen(pFile);
	_tcscpy(disp, pFile);
	*(disp + len - 4) = TEXT('\0');

	return InsertFile(hParent, pNote, disp, FALSE, FALSE);
}

BOOL MemoSelectView::InsertFile(HTREEITEM hParent, MemoNote *pNote, LPCTSTR pTitle, BOOL bInsertToLast, BOOL bLink)
{
	TV_INSERTSTRUCT ti;
	ti.hParent = hParent;
	ti.hInsertAfter = TVI_LAST;
	ti.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	ti.item.iImage = ti.item.iSelectedImage = pNote->GetMemoIcon();

	TreeViewFileItem *ptvi;
	if (bLink) {
		ptvi = new TreeViewFileLink();
	} else {
		ptvi = new TreeViewFileItem();
	}

	if (!ptvi) {
		delete pNote;
		return FALSE;
	}
	ptvi->SetNote(pNote);
	ti.item.lParam = (LPARAM)ptvi;

	ti.item.pszText = (LPTSTR)pTitle;
	HTREEITEM hItem;
	if (bInsertToLast) {
		hItem = TreeView_InsertItem(hViewWnd, &ti);
	} else {
		hItem = InsertNode(hViewWnd, &ti);
	}
	ptvi->SetViewItem(hItem);

	// Notify to MemoManager
	MemoLocator loc(pNote, hItem);
	pMemoMgr->InsertItemNotify(&loc);
	return TRUE;
}

/////////////////////////////////////////
// Insert folder
/////////////////////////////////////////

HTREEITEM MemoSelectView::InsertFolder(HTREEITEM hParent, LPCTSTR pName, TreeViewItem *tvi, BOOL bInsertLast)
{
	HWND hTree = hViewWnd;

	TV_INSERTSTRUCT ti;
	ti.hParent = hParent;
	ti.hInsertAfter = TVI_LAST;
	ti.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	ti.item.pszText = (LPTSTR)pName;
	ti.item.iImage = ti.item.iSelectedImage = tvi->GetIcon(this, MEMO_VIEW_STATE_INIT);
	ti.item.lParam = (LPARAM)tvi;

	HTREEITEM hItem;
	if (bInsertLast) {
		hItem = TreeView_InsertItem(hViewWnd, &ti);
	} else {
		hItem = InsertNode(hTree, &ti);
	}
	tvi->SetViewItem(hItem);

	InsertDummyNode(hTree, hItem);
	return hItem;
}

/////////////////////////////////////////
// Initialize tree
/////////////////////////////////////////

BOOL MemoSelectView::InitTree(VFManager *pManager)
{
	DeleteAllItem();

	// Insert main tree
	TreeViewFolderItem *pItem = new TreeViewFolderItem();
	hMemoRoot = InsertFolder(TVI_ROOT, MSG_MEMO, pItem, TRUE);
	TreeView_Expand(hViewWnd, hMemoRoot, TVE_EXPAND);

#ifdef COMMENT
	// check vfolder def file
	if (_tcslen(g_Property.PropertyDir()) == 0) return TRUE;
	TString sVFpath;
	if (!sVFpath.Join(g_Property.PropertyDir(), TEXT("\\"), TOMBO_VFOLDER_DEF_FILE)) return TRUE;
	File f;
	if (!f.Open(sVFpath.Get(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING)) return FALSE;
	f.Close();
#endif

	// Insert virtual tree
	TreeViewVirtualFolderRoot *pVFRoot = new TreeViewVirtualFolderRoot();
	if (!pVFRoot || !pVFRoot->Init(pManager)) return FALSE;
	hSearchRoot = InsertFolder(TVI_ROOT, MSG_VIRTUAL_FOLDER, pVFRoot, TRUE);

	return TRUE;
}

/////////////////////////////////////////
// Delete one item
/////////////////////////////////////////
//
// Delete item from the treeview and release its memory.

void MemoSelectView::DeleteOneItem(HTREEITEM hItem)
{
	TV_ITEM ti;
	ti.mask = TVIF_PARAM;

	// get lParam(is TreeViewItem*);
	ti.hItem = hItem;
	TreeView_GetItem(hViewWnd, &ti);
	TreeViewItem *tvi;
	tvi = (TreeViewItem *)ti.lParam;

	// if deleting item is clipped, release clipping.
	if (pClipItem == tvi) pClipItem = NULL;

	// if node is file item, notify to MemoManager.
	if (tvi && !tvi->HasMultiItem()) {
		MemoNote *p = ((TreeViewFileItem *)tvi)->GetNote();

		if (p) {
			MemoLocator loc(p, hItem);
			pMemoMgr->ReleaseItemNotify(&loc);
		}
	}

	delete tvi;
	TreeView_DeleteItem(hViewWnd, hItem);
}

/////////////////////////////////////////
// Delete items recursive.
/////////////////////////////////////////
//
// Usually, hFirst is first child of a node named "A".
// DeleteItemsRec() are delete follow sibling and these children, 
// so parent node "A" is not deleted.
// It deletes TreeViewItem that is pointed by HTREEITEM, too.

void MemoSelectView::DeleteItemsRec(HTREEITEM hFirst)
{
	HTREEITEM hItem = hFirst;
	HTREEITEM hNext, hChild;

	while(hItem) {
		// Delete children.
		hChild = TreeView_GetChild(hViewWnd, hItem);
		if (hChild) {
			DeleteItemsRec(hChild);
		}

		// Get next item before hItem is deleted.
		hNext = TreeView_GetNextSibling(hViewWnd, hItem);

		// Delete hItem
		DeleteOneItem(hItem);

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
// Is the item expending?
/////////////////////////////////////////

BOOL MemoSelectView::IsExpand(HTREEITEM hItem)
{
	TV_ITEM ti;
	ti.hItem = hItem;
	ti.mask = TVIF_STATE;
	ti.state = TVIS_EXPANDED;
	ti.stateMask = TVIS_EXPANDED;
	TreeView_GetItem(hViewWnd, &ti);
	return ti.state & TVIS_EXPANDED;
}

/////////////////////////////////////////
// 現在選択されているアイテムの取得
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
	// タップ&ホールドメニューが出たまま画面が切り替わった際にメニューを閉じさせる
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
// menu control
///////////////////////////////////////////

static void ControlSubMenu(HMENU hMenu, UINT nID, BOOL bEnable)
{
	if (bEnable) {
		EnableMenuItem(hMenu, nID, MF_BYCOMMAND | MF_ENABLED);
	} else {
		EnableMenuItem(hMenu, nID, MF_BYCOMMAND | MF_GRAYED);
	}
}

///////////////////////////////////////////
// OnNotify Handler
///////////////////////////////////////////
// if you want to return TRUE/FALSE, return TRUE/FALSE.
// if you want to do default action, return 0xFFFFFFFF.

LRESULT MemoSelectView::OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	LPNM_TREEVIEW pHdr = (LPNM_TREEVIEW)lParam;

#if defined(PLATFORM_PKTPC)
	NMRGINFO *pnmrginfo = (PNMRGINFO)lParam;
	if (pnmrginfo->hdr.code == GN_CONTEXTMENU) {
		// タップ&ホールド処理

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
		return TRUE;
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


			// Viewの切り替えだけではSelectViewにフォーカスがあたってしまう。なぜ？
			if (!tvi->HasMultiItem()) {
				TreeViewFileItem *ptvfi = (TreeViewFileItem*)tvi;
				// If 3rd parameter is TRUE, object are deleted by MainFrame::RequestOpenMemo() 
				MemoLocator *ploc = new MemoLocator(ptvfi->GetNote(), ptvfi->GetViewItem(), TRUE);
				pMemoMgr->GetMainFrame()->PostRequestOpen(ploc, OPEN_REQUEST_MSVIEW_ACTIVE);
			}
			// 暗黙でExpand/Collapseが発生
		}
		break;
	case NM_RETURN:
		{
			// DBLCLKと違い、擬似的にツリーの開閉処理を発生させる
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
				// open request
				TreeViewFileItem *ptvfi = (TreeViewFileItem*)tvi;
				MemoLocator loc(ptvfi->GetNote(), ptvfi->GetViewItem()); 
				pMemoMgr->GetMainFrame()->SendRequestOpen(&loc, OPEN_REQUEST_MSVIEW_ACTIVE);
			}
		}
		break;
	case TVN_SELCHANGING:
		{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
			// 2-Pane Viewの場合には メモの切り替えが発生し、かつ
			// 保存確認でキャンセルを押した場合、アイテムの切り替えを
			// 認めてはならない
			if (!g_Property.IsUseTwoPane() || !pMemoMgr) return FALSE;

			DWORD nYNC;
			if (!pMemoMgr->SaveIfModify(&nYNC, FALSE)) {
				// セーブに失敗...
				TCHAR buf[1024];
				wsprintf(buf, MSG_SAVE_FAILED, GetLastError());
				pMemoMgr->GetMainFrame()->MessageBox(buf, TEXT("ERROR"), MB_ICONSTOP | MB_OK | MB_APPLMODAL);

				// 切り替えられると困るので阻止
				return TRUE;
			}
			switch(nYNC) {
			case IDCANCEL:
				// 切り替えてほしくないとユーザが言っているので阻止
				return TRUE;
			case IDOK:
				/* fall through */
			case IDYES:
				return FALSE;
			case IDNO:
				// データの破棄
				pMemoMgr->ClearMemo();
				return FALSE;
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
			// Control menu item
			ControlMenu();

			if (g_Property.IsUseTwoPane() && (p->action == TVC_BYMOUSE || p->action == TVC_BYKEYBOARD)) {
				// ユーザ操作の場合、メモの切り替えを発生させる
				// プログラムによるものの場合、切り替えは発生させない
				pMemoMgr->SetMSSearchFlg(TRUE);
				if (!tvi->HasMultiItem()) {
					TreeViewFileItem *ptvfi = (TreeViewFileItem*)tvi;
					MemoLocator loc(ptvfi->GetNote(), ptvfi->GetViewItem());
					pMemoMgr->GetMainFrame()->SendRequestOpen(&loc, OPEN_REQUEST_MDVIEW_ACTIVE);
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

	case TVN_BEGINLABELEDIT:
		{
			TreeViewItem *pItem = (TreeViewItem*)(((LPNMTVDISPINFO)lParam)->item.lParam);
			if (pItem && !pItem->CanRename(this)) return TRUE;
			return FALSE;
		}
	case TVN_ENDLABELEDIT:
		return EditLabel(&(((LPNMTVDISPINFO)lParam)->item));
#if defined(PLATFORM_WIN32)
	case NM_RCLICK:
		OnNotify_RClick();
		break;
#endif

	}
	// WM_NOTIFYの返答になるため、必要ない場合以外は各Notifyでreturnすること
	return 0xFFFFFFFF;
}

#if defined(PLATFORM_WIN32)
void MemoSelectView::OnNotify_RClick()
{
	POINT pt, pth;
	GetCursorPos(&pt);
	pth = pt;
	ScreenToClient(hViewWnd, &pth);
	TV_HITTESTINFO hti;
	hti.pt = pth;
	TreeView_HitTest(hViewWnd, &hti);

	if (hti.hItem == NULL) return;

	TreeViewItem *pItem = GetTVItem(hti.hItem);

	HMENU hContextMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_CONTEXTMENU));
	HMENU hMenu = GetSubMenu(hContextMenu, 0);
	ControlSubMenu(hMenu, IDM_ENCRYPT, pItem->CanEncrypt(this));
	ControlSubMenu(hMenu, IDM_DECRYPT, pItem->CanDecrypt(this));
	ControlSubMenu(hMenu, IDM_CUT, pItem->CanCut(this));
	ControlSubMenu(hMenu, IDM_COPY, pItem->CanCopy(this));
	ControlSubMenu(hMenu, IDM_PASTE, pItem->CanPaste(this));
	ControlSubMenu(hMenu, IDM_RENAME, pItem->CanRename(this));
	ControlSubMenu(hMenu, IDM_DELETEITEM, pItem->CanDelete(this));
	ControlSubMenu(hMenu, IDM_NEWFOLDER, pItem->CanNewMemo(this));
	ControlSubMenu(hMenu, IDM_SEARCH, pItem->CanGrep(this));
	ControlSubMenu(hMenu, IDM_TRACELINK, pItem->CanLink(this));

	DWORD id = TrackPopupMenuEx(hMenu, TPM_RETURNCMD, pt.x, pt.y, hViewWnd, NULL);
	DestroyMenu(hContextMenu);

	switch(id) {
	case IDM_CUT:
		OnCut(pItem);
		break;
	case IDM_COPY:
		OnCopy(pItem);
		break;
	case IDM_PASTE:
		OnPaste();
		break;
	case IDM_ENCRYPT:
		{
			MemoNote *pNote = ((TreeViewFileItem*)pItem)->GetNote();
			if (g_Property.IsUseTwoPane() && pMemoMgr->IsNoteDisplayed(pNote->MemoPath())) {
				pMemoMgr->InactiveDetailsView();
			}
			TV_HITTESTINFO hti;
			hti.pt = pth;
			TreeView_HitTest(hViewWnd, &hti);
			if (hti.hItem == NULL) return;

			TreeViewItem *pItem = GetTVItem(hti.hItem);

			OnEncrypt(pItem);
		}
		break;
	case IDM_DECRYPT:
		OnDecrypt(pItem);
		break;
	case IDM_RENAME:
		OnEditLabel(hti.hItem);
		break;
	case IDM_DELETEITEM:
		OnDelete(hti.hItem, pItem);
		break;
	case IDM_NEWFOLDER:
		pMemoMgr->GetMainFrame()->NewFolder(pItem);
		break;
	case IDM_SEARCH:
		TreeView_SelectItem(hViewWnd, hti.hItem);
		pMemoMgr->GetMainFrame()->OnSearch();
		break;
	case IDM_TRACELINK:
		TreeViewFileLink *p = (TreeViewFileLink*)pItem;
		ShowItem(p->GetNote()->MemoPath(), TRUE, FALSE);
		break;
	}
}
#endif
///////////////////////////////////////////
// OnCommandハンドラ
///////////////////////////////////////////

BOOL MemoSelectView::OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HTREEITEM  hItem;
	TreeViewItem *pItem = GetCurrentItem(&hItem);

	switch(LOWORD(wParam)) {
	case IDM_ENCRYPT:
		OnEncrypt(pItem);
		return TRUE;
	case IDM_DECRYPT:
		OnDecrypt(pItem);
		return TRUE;
	case IDM_DELETEITEM:
		// DEL key is handled by SelectView findows procedure, but leave this chunk for sending WM_COMMAND
		OnDelete(hItem, pItem);
		return TRUE;
	case IDM_ACTIONBUTTON:
		OnActionButton(hWnd);
		return TRUE;
	case IDM_CUT:
		// Cut/Copy/Paste key is handled by SelectView findows procedure, but leave this chunk for sending WM_COMMAND
		OnCut(pItem);
		return TRUE;
	case IDM_COPY:
		OnCopy(pItem);
		return TRUE;
	case IDM_PASTE:
		OnPaste();
		return TRUE;
	case IDM_RENAME:
		OnEditLabel(hItem);
		return TRUE;
	}
	return FALSE;
}

///////////////////////////////////////////
// HotKeyの処理
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
// Encrypt
///////////////////////////////////////////

void MemoSelectView::OnEncrypt(TreeViewItem *pItem)
{
	if (!pItem->Encrypt(pMemoMgr, this)) {
		TCHAR buf[1024];
		wsprintf(buf, MSG_ENCRYPTION_FAILED, GetLastError());
		TomboMessageBox(hViewWnd, buf, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
	}
}

///////////////////////////////////////////
// Decrypt
///////////////////////////////////////////

void MemoSelectView::OnDecrypt(TreeViewItem *pItem)
{
	if (!pItem->Decrypt(pMemoMgr, this)) {
		TCHAR buf[1024];
		wsprintf(buf, MSG_DECRYPTION_FAILED, GetLastError());
		TomboMessageBox(hViewWnd, buf, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
	}
}

/////////////////////////////////////////////
// Rename
/////////////////////////////////////////////

void MemoSelectView::OnEditLabel(HTREEITEM hItem)
{
	if (hItem == NULL) return;
	TreeView_EditLabel(hViewWnd, hItem);
	return;
}

///////////////////////////////////////////
// delete
///////////////////////////////////////////

void MemoSelectView::OnDelete(HTREEITEM hItem, TreeViewItem *pItem)
{
	if (hItem == NULL) return;
	if (pItem->Delete(pMemoMgr, this)) {
		DeleteOneItem(hItem);
	}
}

///////////////////////////////////////////
// ウィンドウサイズ取得
///////////////////////////////////////////

void MemoSelectView::GetSize(LPWORD pWidth, LPWORD pHeight)
{
	RECT r;
	GetWindowRect(hViewWnd, &r);
	*pWidth = (WORD)(r.right - r.left);
	*pHeight = (WORD)(r.bottom - r.top);
}

///////////////////////////////////////////
// メモの階層の取得
///////////////////////////////////////////

LPTSTR MemoSelectView::GeneratePath(HTREEITEM hItem, LPTSTR pBuf, DWORD nSiz)
{
	HWND hTree = hViewWnd;

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

BOOL MemoSelectView::GetCurrentItemPath(TString *pPath)
{
	HTREEITEM h;
	TreeViewItem *pItem = GetCurrentItem(&h);
	if (pItem == NULL) return FALSE;

	TString sPath;
	if (!pItem->GetLocationPath(this, pPath)) return FALSE;
	return TRUE;
}

void MemoSelectView::TreeExpand(HTREEITEM hItem)
{
	// delete dummy child nodes.
	HTREEITEM di = TreeView_GetChild(hViewWnd, hItem);
	DeleteItemsRec(di);

	// change icon to "EXPANDING".
	TreeViewItem *pItem = GetTVItem(hItem);
	SetIcon(pItem, MEMO_VIEW_STATE_OPEN_SET);

	if (!pItem->HasMultiItem()) {
		MessageBox(NULL, TEXT("This node does'nt have multi item"), TEXT("DEBUG"), MB_OK);
		return;
	}

	TreeViewFolderItem *pFolder = (TreeViewFolderItem*)pItem;

	// expand nodes.
	pFolder->Expand(this);

	// Close node if lower node is not exist.
	if (TreeView_GetChild(hViewWnd, hItem) == NULL) {
		TreeCollapse(hItem);
	}
}

void MemoSelectView::TreeCollapse(HTREEITEM hItem)
{
	HTREEITEM h = TreeView_GetChild(hViewWnd, hItem);

	// ノードの削除
	DeleteItemsRec(h);

	// ダミーのノードを挿入
	InsertDummyNode(hViewWnd, hItem);

	// イメージを閉じているものに変更
	TreeViewItem *pItem = GetTVItem(hItem);
	SetIcon(pItem, MEMO_VIEW_STATE_OPEN_CLEAR); 

	TreeView_Expand(hViewWnd, hItem, TVE_COLLAPSE | TVE_COLLAPSERESET);
}

///////////////////////////////////////////////////////
// 新規ノードの作成
///////////////////////////////////////////////////////
// メモの新規作成に対応してノードを追加する。

HTREEITEM MemoSelectView::NewMemoCreated(MemoNote *pNote, LPCTSTR pHeadLine, HTREEITEM hParent)
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
	tvi->SetViewItem(hItem);
	return hItem;
}

///////////////////////////////////////////
// ウィンドウサイズの移動
///////////////////////////////////////////

void MemoSelectView::MoveWindow(DWORD x, DWORD y, DWORD nWidth, DWORD nHeight)
{
	::MoveWindow(hViewWnd, x, y, nWidth, nHeight, TRUE);
}

/////////////////////////////////////////////////////////////
// TreeViewItem status change notify
/////////////////////////////////////////////////////////////
// Called from TreeViewItem when icon status/headline string changed.
// Update treeview status and link assosiation of HTREEITEM <-> TreeViewItem 
// and TreeViewItem <-> MemoManager.

BOOL MemoSelectView::UpdateItemStatusNotify(TreeViewItem *pItem, LPCTSTR pNewHeadLine)
{
	// Get old HTREEITEM
	HTREEITEM hOrigNode = pItem->GetViewItem();

	HTREEITEM hParent = TreeView_GetParent(hViewWnd, hOrigNode);
	if (hParent == NULL) {
		hParent = TVI_ROOT;
	}

	// New HTREEITEM may be change, once release mapping.
	// Notify to memomanager
	pMemoMgr->ReleaseItemNotify(&(pItem->ToLocator()));

	// Delete from Tree. (TreeViewItem is not deleted because it will deleted at caller.)
	TreeView_DeleteItem(hViewWnd, hOrigNode);

	// Insert to tree.
	TV_INSERTSTRUCT ti;
	ti.hParent = hParent;
	ti.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	ti.item.iImage = ti.item.iSelectedImage = pItem->GetIcon(this, 0); // Get current icon
	ti.item.pszText = (LPTSTR)pNewHeadLine;
	ti.item.lParam = (LPARAM)pItem;
	HTREEITEM hItem = ::InsertNode(hViewWnd, &ti);

	// link TreeViewItem <-> HTREEITEM assosiation
	pItem->SetViewItem(hItem);
	TreeView_SelectItem(hViewWnd, hItem);

	// notify to MemoManager.
	pMemoMgr->InsertItemNotify(&(pItem->ToLocator()));
	return TRUE;
}

///////////////////////////////////////////
// アクションボタンに対する処理
///////////////////////////////////////////

void MemoSelectView::OnActionButton(HWND hWnd)
{
	HTREEITEM hItem;
	TreeViewItem *p = GetCurrentItem(&hItem);
	if (p && !p->HasMultiItem()) {
		// メモの場合にはMainFrameに対してオープン要求を出す
		TreeViewFileItem *ptvfi = (TreeViewFileItem*)p;
		MemoLocator loc(ptvfi->GetNote(), ptvfi->GetViewItem());
		pMemoMgr->GetMainFrame()->SendRequestOpen(&loc, OPEN_REQUEST_MSVIEW_ACTIVE);
		return;
	}
	
	RECT r;
	TreeView_GetItemRect(hViewWnd, hItem, &r, TRUE);
	PostMessage(hViewWnd, WM_LBUTTONDBLCLK, MK_LBUTTON, MAKELPARAM(r.left, r.top));
}

///////////////////////////////////////////
// 選択しているアイテムのパスの取得
///////////////////////////////////////////

HTREEITEM MemoSelectView::GetPathForNewItem(TString *pPath, TreeViewItem *pItem)
{
	TCHAR buf[MAX_PATH];
	HTREEITEM hItem = NULL;

	if (pItem == NULL) {
		pItem = GetCurrentItem(&hItem);
	} else {
		hItem = pItem->GetViewItem();
	}
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

	LPTSTR pPathTop = GeneratePath(hItem, buf, MAX_PATH);
	if (!pPath->Set(pPathTop)) return NULL;
	return hItem;
}

///////////////////////////////////////////
// フォルダの挿入
///////////////////////////////////////////

BOOL MemoSelectView::CreateNewFolder(HTREEITEM hItem, LPCTSTR pFolder)
{
	// ツリーが閉じている場合には開く
	if (!IsExpand(hItem) && (TreeView_GetChild(hViewWnd, hItem) != NULL)) {
		// 開くことで作成したフォルダは自動的に読み込まれる
		TreeView_Expand(hViewWnd, hItem, TVE_EXPAND);
	} else {
		// 開いている場合には挿入
		TreeViewFolderItem *pItem = new TreeViewFolderItem();
		InsertFolder(hItem, pFolder, pItem, FALSE);
		if (TreeView_GetChild(hViewWnd, hItem) != NULL) {
			TreeView_Expand(hViewWnd, hItem, TVE_EXPAND);
		}
	}
	return TRUE;
}

/////////////////////////////////////////
// ヘッドライン文字列の変更
/////////////////////////////////////////
// 再ソートが必要となるため、一旦削除して再挿入・ソートしている

BOOL MemoSelectView::UpdateHeadLine(MemoLocator *pLoc, LPCTSTR pHeadLine)
{
	MemoNote *pNote = pLoc->GetNote();
	HTREEITEM hItem = pLoc->GetHITEM();

#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	HTREEITEM hCurrentSel;
	if (g_Property.IsUseTwoPane()) {
		hCurrentSel = TreeView_GetSelection(hViewWnd);
	}
#endif

	// Get parent item
	HTREEITEM hParent = TreeView_GetParent(hViewWnd, hItem);

	// 新しいノードの挿入・MemoNoteとの関連付け
	TV_INSERTSTRUCT ti;
	ti.hParent = hParent;
	ti.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	ti.item.pszText = (LPTSTR)pHeadLine;
	ti.item.iImage = ti.item.iSelectedImage = pNote->GetMemoIcon();

	TreeViewFileItem *tvi = new TreeViewFileItem();
	MemoNote *pNewNote = pNote->Clone();

	// Delete current item
	DeleteOneItem(hItem);

	if (pNewNote == NULL) return FALSE;
	tvi->SetNote(pNewNote);
	ti.item.lParam = (LPARAM)tvi;

	// TODO:
	// 親がcloseされているときにはhCurrentItemの関連が切り離されているので
	// そもそもUpdateHeadLineが呼び出されることはない
	if (IsExpand(hParent)) {
		HTREEITEM hItem = ::InsertNode(hViewWnd, &ti);
		tvi->SetViewItem(hItem);
		TreeView_SelectItem(hViewWnd, hItem);

		// Notify to MemoManager
		MemoLocator loc(pNewNote, hItem);
		pMemoMgr->InsertItemNotify(&loc);

	} else {
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
		if (g_Property.IsUseTwoPane()) {
			// 削除・挿入により選択されているアイテムが変わるので戻す
			TreeView_SelectItem(hViewWnd, hCurrentSel);
		}
#endif
	}
	return TRUE;
}

/////////////////////////////////////////
// アイコンの設定
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
// 擬似クリップボードへの設定
/////////////////////////////////////////

void MemoSelectView::SetShareArea(TreeViewItem *pItem)
{
	if (pClipItem) {
		SetIcon(pClipItem, MEMO_VIEW_STATE_CLIPED_CLEAR);
	}
	pClipItem = pItem;
	if (pClipItem) {
		SetIcon(pClipItem, MEMO_VIEW_STATE_CLIPED_SET);
	}
}

/////////////////////////////////////////
// 移動するメモの選択
/////////////////////////////////////////

void MemoSelectView::OnCut(TreeViewItem *pItem)
{
	SetShareArea(pItem);
	bCut = TRUE;
}

/////////////////////////////////////////
// コピーするメモの選択
/////////////////////////////////////////

void MemoSelectView::OnCopy(TreeViewItem *pItem)
{
	SetShareArea(pItem);
	bCut = FALSE;
}

/////////////////////////////////////////
// 移動・コピーの実行
/////////////////////////////////////////

void MemoSelectView::OnPaste()
{
	if (pClipItem == NULL) return;

	if (bCut) {
		// メモの移動
		HTREEITEM hItem = pClipItem->GetViewItem();
		LPCTSTR pErr = NULL;
		if (!pClipItem->Move(pMemoMgr, this, &pErr)) {
			if (pErr) {
				TomboMessageBox(hViewWnd, pErr, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
			} else {
				TomboMessageBox(hViewWnd, MSG_MOVE_MEMO_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
			}
			return;
		}
		DeleteOneItem(hItem);
	} else {
		// メモのコピー
		LPCTSTR pErr = NULL;
		if (!pClipItem->Copy(pMemoMgr, this, &pErr)) {
			if (pErr) {
				TomboMessageBox(hViewWnd, pErr, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
			} else {
				TomboMessageBox(hViewWnd, MSG_COPY_MEMO_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
			}
		}
	}
}

/////////////////////////////////////////
// Control menu
/////////////////////////////////////////

void MemoSelectView::ControlMenu()
{
	MainFrame *pMf = pMemoMgr->GetMainFrame();
	if (pMf == NULL) return;

	TreeViewItem *pItem = GetCurrentItem();
	if (pItem) {
		pMf->EnableDelete(pItem->CanDelete(this));
		pMf->EnableRename(pItem->CanRename(this));

		pMf->EnableEncrypt(pItem->CanEncrypt(this));
		pMf->EnableDecrypt(pItem->CanDecrypt(this));

		pMf->EnableNew(pItem->CanNewMemo(this));
		pMf->EnableNewFolder(pItem->CanNewFolder(this));

		pMf->EnableCut(pItem->CanCut(this));
		pMf->EnableCopy(pItem->CanCopy(this));
		pMf->EnablePaste(pItem->CanPaste(this));

		pMf->EnableGrep(pItem->CanGrep(this));
	} else {
		pMf->EnableDelete(FALSE);
		pMf->EnableRename(FALSE);
		pMf->EnableEncrypt(FALSE);
		pMf->EnableDecrypt(FALSE);

		pMf->EnableCut(FALSE);
		pMf->EnableCopy(FALSE);
		pMf->EnablePaste(FALSE);
	}
}

/////////////////////////////////////////
// フォーカスの取得
/////////////////////////////////////////

void MemoSelectView::OnGetFocus()
{
	MainFrame *pMf = pMemoMgr->GetMainFrame();
	if (pMf) {
		pMf->ActivateView(TRUE);
	}
	ControlMenu();
}

/////////////////////////////////////////
// フォントの設定
/////////////////////////////////////////

void MemoSelectView::SetFont(HFONT hFont)
{
	SendMessage(hViewWnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
}

///////////////////////////////////////////
// ツリーの開閉
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

/////////////////////////////////////////////
//  HTREEITEMからTreeViewItem*の取得
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
// HTREEITEMにTreeViewItem*を設定
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
	// ダミーノード
	TV_INSERTSTRUCT tisub;
	tisub.hParent = hItem;
	tisub.hInsertAfter = TVI_LAST;
	tisub.item.mask = TVIF_PARAM;
	tisub.item.lParam = NULL;
	TreeView_InsertItem(hTree, &tisub);
}

/////////////////////////////////////////////
// ファイル名の変更
/////////////////////////////////////////////
//
// 戻り値： TRUE = ラベル変更をaccept
//			FALSE = ラベル変更をreject

LRESULT MemoSelectView::EditLabel(TVITEM *pItem)
{
	if (pItem->pszText == NULL) return FALSE;

	TreeViewItem *pti = (TreeViewItem*)(pItem->lParam);
	if (pti == NULL) return FALSE;
	return pti->Rename(pMemoMgr, this, pItem->pszText);
}

/////////////////////////////////////////////
// Expand tree and show note
/////////////////////////////////////////////

static HTREEITEM FindItem(HWND hWnd, HTREEITEM hParent, LPCTSTR pStr, BOOL bNote)
{
	HTREEITEM hItem = TreeView_GetChild(hWnd, hParent);
	TV_ITEM ti;

	TCHAR buf[MAX_PATH + 1];
	ti.mask = TVIF_TEXT | TVIF_PARAM;
	ti.cchTextMax = MAX_PATH;
	ti.pszText = buf;

	while(hItem) {
		ti.hItem = hItem;
		TreeView_GetItem(hWnd, &ti);

		if (_tcsicmp(buf, pStr) == 0) {
			if (((TreeViewItem*)ti.lParam)->HasMultiItem() == !bNote) {
				return hItem;
			}
		}

		hItem = TreeView_GetNextSibling(hWnd, hItem);
	}
	return NULL;
}

// ex.
//	msView.ShowItem(TEXT("temp\\Hello.txt"));

HTREEITEM MemoSelectView::ShowItem(LPCTSTR pPath, BOOL bSelChange, BOOL bOpenNotes)
{
	while(*pPath == TEXT('\\')) pPath++;

	if (_tcslen(pPath) == 0) {
		if (bSelChange) {
			TreeView_SelectItem(hViewWnd, hMemoRoot);
		}
		return hMemoRoot;
	}

	TString sPath;
	HTREEITEM hTargetItem;

	if (!sPath.Join(g_Property.TopDir(), TEXT("\\"), pPath)) return NULL;
	LPTSTR p = sPath.Get();
	p += _tcslen(g_Property.TopDir());

	hTargetItem = TreeView_GetRoot(hViewWnd);

	LPTSTR q, pPartPath;
	BOOL bEndFlg = FALSE;
	BOOL bNote = FALSE;
	do {
		// get path item
		q = p + 1;
		pPartPath = q;
		while(*q) {
#if defined(PLATFORM_WIN32)
			if (IsDBCSLeadByte(*q)) {
				q++;
				if (*q) q++;
				continue;
			}
#endif
			if (*q == TEXT('\\')) break;
			q++;
		}
		if (*q == TEXT('\0')) {
			bEndFlg = TRUE;
		}
		*p = TEXT('\0');
		*q = TEXT('\0');

		// if tree is not expanding, expand it.
		if (!IsExpand(hTargetItem)) {
			TreeView_Expand(hViewWnd, hTargetItem, TVE_EXPAND);
		}

		if (bEndFlg && MemoNote::IsNote(pPartPath)) {
			*(pPartPath + _tcslen(pPartPath) - 4) = TEXT('\0');
			bNote = TRUE;
		}

		if (bSelChange) TreeView_SelectItem(hViewWnd, hTargetItem);
		// Get Target Item
		hTargetItem = FindItem(hViewWnd, hTargetItem, pPartPath, bNote);
		if (hTargetItem == NULL) {
			return NULL;
		}
		if (bSelChange) TreeView_SelectItem(hViewWnd, hTargetItem);

		// restore path
		*p = TEXT('\\');
		*q = TEXT('\\');
		p = q;
	} while(!bEndFlg);


	if (bSelChange) {
		TreeView_SelectItem(hViewWnd, hTargetItem);

		// if selected item is note, open it.
		if (bNote) {
			TreeViewItem *ptv = GetTVItem(hTargetItem);
			if (!ptv->HasMultiItem()) {
				TreeViewFileItem *p = (TreeViewFileItem*)ptv;
				MemoLocator loc(p->GetNote(), p->GetViewItem());
				if (bOpenNotes) {
					pMemoMgr->GetMainFrame()->SendRequestOpen(&loc, OPEN_REQUEST_MSVIEW_ACTIVE);
				} else {
					pMemoMgr->GetMainFrame()->SendRequestOpen(&loc, OPEN_REQUEST_MDVIEW_ACTIVE);
				}
			}
		}
	}
	return hTargetItem;
}

/////////////////////////////////////////////
// Insert grep result
/////////////////////////////////////////////

BOOL MemoSelectView::InsertVirtualFolder(const VFInfo *pInfo)
{
	TreeViewVirtualFolderRoot *pVFRoot = (TreeViewVirtualFolderRoot*)GetTVItem(hSearchRoot);
	return pVFRoot->AddSearchResult(this, pInfo);
}

/////////////////////////////////////////////
// Get virtual folder root instance
/////////////////////////////////////////////

TreeViewVirtualFolderRoot *MemoSelectView::GetVirtualFolderRoot()
{
	if (!IsExpand(hSearchRoot)) {
		TreeView_Expand(hViewWnd, hSearchRoot, TVE_EXPAND);
	}
	return (TreeViewVirtualFolderRoot*)GetTVItem(hSearchRoot);
}

void MemoSelectView::CloseVFRoot()
{
	if (IsExpand(hSearchRoot)) {
		TreeCollapse(hSearchRoot);
	}
}

/////////////////////////////////////////////
// select node
/////////////////////////////////////////////

void MemoSelectView::SelUpFolderWithoutOpen()
{
	HTREEITEM hItem;
	TreeViewItem *pItem = GetCurrentItem(&hItem);
	HTREEITEM hParent = TreeView_GetParent(hViewWnd, hItem);
	if (hParent) {
		TreeView_SelectItem(hViewWnd, hParent);
	}
}

void MemoSelectView::SelNextBrother()
{
	HTREEITEM hItem;
	TreeViewItem *pItem = GetCurrentItem(&hItem);

	HTREEITEM h = TreeView_GetNextSibling(hViewWnd, hItem);
	if (h) {
		TreeView_SelectItem(hViewWnd, h);
	}
}

void MemoSelectView::SelPrevBrother()
{
	HTREEITEM hItem;
	TreeViewItem *pItem = GetCurrentItem(&hItem);

	HTREEITEM h = TreeView_GetPrevSibling(hViewWnd, hItem);
	if (h) {
		TreeView_SelectItem(hViewWnd, h);
	}
}