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

// IDB_MEMOSELECT_IMAGESに格納されているイメージ1個のサイズ
#define IMAGE_CX 16
#define IMAGE_CY 16

// IDB_MEMOSELECT_IMAGESに格納されているイメージ数
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
// ウィンドウ生成
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

	// フォント設定
	if (hFont != NULL) {
		SendMessage(hViewWnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	}
	return TRUE;
}

/////////////////////////////////////////
// ノードの挿入
/////////////////////////////////////////
//
// ノードをソートして挿入する。
// hInsertAfterについては設定する必要はない。
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
// ファイルノードの作成
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
		// *.txt, *.chi以外はなにもせず
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

// pMatchPathは末尾が"\\*.*"形式となっていること
static BOOL LoadNodeUnderItem(HWND hTree, HTREEITEM hParent, LPCTSTR pMatchPath, LPCTSTR pPrefix)
{
	WIN32_FIND_DATA wfd;

	HANDLE hHandle = FindFirstFile(pMatchPath, &wfd);
	if (hHandle != INVALID_HANDLE_VALUE) {
		do {
			if (_tcscmp(wfd.cFileName, TEXT(".")) == 0 || _tcscmp(wfd.cFileName, TEXT("..")) == 0) continue;

			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// フォルダ
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

	// ルートノードの展開
	TreeView_Expand(hViewWnd, hRoot, TVE_EXPAND);
	return TRUE;
}

/////////////////////////////////////////
// 現在存在する全アイテムを削除する
/////////////////////////////////////////
// 関連付けられているTreeViewItemもdeleteする

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
		// 削除処理
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
// ツリーを開いているかどうか？
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
// OnNotifyハンドラ
///////////////////////////////////////////
//
// 結果としてTRUE, FALSEの意思表示をしたい場合にはその値を返す。
// デフォルトの動作に従いたい場合には0xFFFFFFFFを返す。


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

			// シングルクリックモード・自動追従モードの場合、2回ロードされることになるが、とりあえずこのまま
			// Viewの切り替えだけではSelectViewにフォーカスがあたってしまう。なぜ？
			if (!tvi->HasMultiItem()) {
				PostMessage(hWnd, MWM_OPEN_REQUEST, (WPARAM)OPEN_REQUEST_MSVIEW_ACTIVE, (LPARAM)(TreeViewFileItem*)tvi);
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
			// 2-Pane Viewの場合には メモの切り替えが発生し、かつ
			// 保存確認でキャンセルを押した場合、アイテムの切り替えを
			// 認めてはならない
			if (g_Property.IsUseTwoPane()) {			
				// メモの切り替えが発生
				DWORD nYNC;
				if (pMemoMgr && 
					(pNote == pMemoMgr->CurrentNote() || pMemoMgr->CurrentNote() == NULL)) {
						// 上のif文の第2条件は一挙に複数のSELCHANGINGが来るため
					if (!pMemoMgr->SaveIfModify(&nYNC, FALSE)) {
						// セーブに失敗...
	
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

			// メニューの制御
			pMemoMgr->SelectNote(pNote);

			if (g_Property.IsUseTwoPane() && (p->action == TVC_BYMOUSE || p->action == TVC_BYKEYBOARD)) {
				// ユーザ操作の場合、メモの切り替えを発生させる
				// プログラムによるものの場合、切り替えは発生させない
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
	// WM_NOTIFYの返答になるため、必要ない場合以外は各Notifyでreturnすること
	return 0xFFFFFFFF;
}

///////////////////////////////////////////
// OnCommandハンドラ
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

	// ダミーの子ノードを削除
	HTREEITEM di = TreeView_GetChild(hTree, hItem);
	DeleteItemsRec(di);

	// イメージを展開しているものに変更
	TreeViewItem *pItem = GetTVItem(hItem);
	SetIcon(pItem, MEMO_VIEW_STATE_OPEN_SET);

	// フルパスを取得
	TCHAR buf[MAX_PATH];	// メモのルートからの相対パス
	TCHAR buf2[MAX_PATH];
	LPTSTR pPath = ::GeneratePath(hTree, hItem, buf, MAX_PATH);
	wsprintf(buf2, TEXT("%s\\%s*.*"), g_Property.TopDir(), pPath);

	LoadNodeUnderItem(hTree, hItem, buf2, pPath);

	// 下位のノードが存在しない場合には再度閉じる
	if (TreeView_GetChild(hTree, hItem) == NULL) {
		TreeCollapse(hItem);
	}
}

void MemoSelectView::TreeCollapse(HTREEITEM hItem)
{
	HWND hTree = hViewWnd;
	HTREEITEM h = TreeView_GetChild(hTree, hItem);

	// ノードの削除
	DeleteItemsRec(h);

	// ダミーのノードを挿入
	InsertDummyNode(hTree, hItem);

	// イメージを閉じているものに変更
	TreeViewItem *pItem = GetTVItem(hItem);
	SetIcon(pItem, MEMO_VIEW_STATE_OPEN_CLEAR); 

	TreeView_Expand(hViewWnd, hItem, TVE_COLLAPSE | TVE_COLLAPSERESET);
}

///////////////////////////////////////////////////////
// 新規ノードの作成
///////////////////////////////////////////////////////
// メモの新規作成に対応してノードを追加する。

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
// ウィンドウサイズの移動
///////////////////////////////////////////

void MemoSelectView::MoveWindow(DWORD x, DWORD y, DWORD nWidth, DWORD nHeight)
{
	::MoveWindow(hViewWnd, x, y, nWidth, nHeight, TRUE);
}

/////////////////////////////////////////////////////////////
// TreeViewItemの状態が変わったことによるビューへの変更依頼
/////////////////////////////////////////////////////////////
// アイコン状態・ヘッドライン文字列が変更になった場合に
// TreeViewItemからコールされる。これに応じてアイコン状態および
// 挿入されているアイテムの位置を更新する。
// 新しいNodeはpItemと関連づけられる

BOOL MemoSelectView::UpdateItemStatus(TreeViewItem *pItem, LPCTSTR pNewHeadLine)
{
	// 旧のHTREEITEMを取得
	HTREEITEM hOrigNode = pItem->GetViewItem();

	// 親ノードの取得
	HTREEITEM hParent = TreeView_GetParent(hViewWnd, hOrigNode);
	if (hParent == NULL) {
		hParent = TVI_ROOT;
	}

	// 一旦削除
	TreeView_DeleteItem(hViewWnd, hOrigNode);

	// アイテムの挿入
	TV_INSERTSTRUCT ti;
	ti.hParent = hParent;
	ti.hInsertAfter = TVI_LAST;
	ti.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	ti.item.iImage = ti.item.iSelectedImage = pItem->GetIcon(this, 0); // 現状のアイコンを取得
	ti.item.pszText = (LPTSTR)pNewHeadLine;
	ti.item.lParam = (LPARAM)pItem;

	HTREEITEM hItem = ::InsertNode(hViewWnd, &ti);
	pItem->SetViewItem(hItem);
	return TRUE;
}

///////////////////////////////////////////
// メモのビューからの削除
///////////////////////////////////////////
// pは対応するHTREEITEMを持たなくなるため、pのViewItemはNULLに設定される。

BOOL MemoSelectView::DeleteItem(TreeViewItem *p)
{
	HTREEITEM hItem = p->GetViewItem();

	HTREEITEM hParent = TreeView_GetParent(hViewWnd, hItem);

	// 子供がいたらノードの削除& TreeViewItem, MemoNote のdeleteを行う
	HTREEITEM hChild = TreeView_GetChild(hViewWnd, hItem);
	DeleteItemsRec(hChild);

	TreeView_DeleteItem(hViewWnd, hItem);
	p->SetViewItem(NULL);

	// 親ノードに結果としてノードが存在しなくなった場合にはCollapseしておく
	if (TreeView_GetChild(hViewWnd, hParent) == NULL) {
		TreeCollapse(hParent);
	}
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
		SendMessage(hWnd, MWM_OPEN_REQUEST, (WPARAM)OPEN_REQUEST_MSVIEW_ACTIVE, (LPARAM)(TreeViewFileItem*)p);
		return;
	}
	
	RECT r;
	TreeView_GetItemRect(hViewWnd, hItem, &r, TRUE);
	PostMessage(hViewWnd, WM_LBUTTONDBLCLK, MK_LBUTTON, MAKELPARAM(r.left, r.top));
}

///////////////////////////////////////////
// 選択しているアイテムのパスの取得
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
// メモ・フォルダの削除
///////////////////////////////////////////

void MemoSelectView::DeleteNode(HWND hWnd, HTREEITEM hItem, TreeViewItem *pItem)
{
	if (hItem == NULL) return;

	if (pItem->Delete(pMemoMgr, this)) {
		delete pItem;
	}
}

///////////////////////////////////////////
// 指定されたアイテムを選択状態とする
///////////////////////////////////////////

void MemoSelectView::SelectNote(MemoNote *pNote)
{
	if (!pNote) return;
	HTREEITEM hItem = pNote->GetViewItem();
	TreeView_SelectItem(hViewWnd, hItem);
}

///////////////////////////////////////////
// フォルダの挿入
///////////////////////////////////////////

BOOL MemoSelectView::CreateNewFolder(HTREEITEM hItem, LPCTSTR pFolder)
{
	// ツリーが閉じている場合には開く
	if (!IsExpand(hViewWnd, hItem) && (TreeView_GetChild(hViewWnd, hItem) != NULL)) {
		// 開くことで作成したフォルダは自動的に読み込まれる
		TreeView_Expand(hViewWnd, hItem, TVE_EXPAND);
	} else {
		// 開いている場合には挿入
		InsertFolder(hViewWnd, hItem, pFolder);
		if (TreeView_GetChild(hViewWnd, hItem) != NULL) {
			TreeView_Expand(hViewWnd, hItem, TVE_EXPAND);
		}
	}
	return TRUE;
}

/////////////////////////////////////////
// フォルダの挿入
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

	// ダミーノード
	InsertDummyNode(hTree, hItem);
	return hItem;
}

/////////////////////////////////////////
// ヘッドライン文字列の変更
/////////////////////////////////////////
// 再ソートが必要となるため、一旦削除して再挿入・ソートしている

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

	// 親ノードの取得
	HTREEITEM hParent = TreeView_GetParent(hViewWnd, hItem);
	// 現在のノードの削除
	TreeView_DeleteItem(hViewWnd, hItem);

	// 新しいノードの挿入・MemoNoteとの関連付け
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
			// 削除・挿入により選択されているアイテムが変わるので戻す
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
// 移動するメモの選択
/////////////////////////////////////////

void MemoSelectView::OnCut(HWND hWnd)
{
	SetShareArea();
	bCut = TRUE;
}

/////////////////////////////////////////
// コピーするメモの選択
/////////////////////////////////////////

void MemoSelectView::OnCopy(HWND hWnd)
{
	SetShareArea();
	bCut = FALSE;
}

/////////////////////////////////////////
// 移動・コピーの実行
/////////////////////////////////////////

void MemoSelectView::OnPaste(HWND hWnd)
{
	if (pClipItem == NULL) return;

	if (bCut) {
		// メモの移動
		if (!pClipItem->Move(pMemoMgr, this)) {
			TomboMessageBox(hViewWnd, MSG_MOVE_MEMO_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
			return;
		} else {
			delete pClipItem;
			pClipItem = NULL;
		}
	} else {
		// メモのコピー
		if (!pClipItem->Copy(pMemoMgr, this)) {
			TomboMessageBox(hViewWnd, MSG_COPY_MEMO_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
		}
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
// 次のノードを取得
/////////////////////////////////////////////

static HTREEITEM GetNextItem(HWND hWnd, HTREEITEM hCurrentItem)
{
	HTREEITEM hPrev;
	HTREEITEM h = hCurrentItem;

	hPrev = h;
	h = TreeView_GetNextSibling(hWnd, h);

	// SiblingがNULLの場合、親の弟を探す。親の弟がいなければ親の親でさらに挑戦。
	while (h == NULL) {
		HTREEITEM hParent = TreeView_GetParent(hWnd, hPrev);
		if (hParent == NULL) break; // ルートまで来た

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

// 末っ子を取得
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
// ページ間検索
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

	// ルートフォルダが閉じられていたら開いておく
	if (TreeView_GetParent(hViewWnd, hItem) == NULL && !IsExpand(hViewWnd, hItem)) {
		TreeView_Expand(hViewWnd, hItem, TVE_EXPAND);
		TreeExpand(hItem);
	}

	if (bSearchEncryptMemo && !bFileNameOnly) {
		// パスワードチェック
		// 対象がファイル名のみの場合、パスワードは不要なので条件からはずす
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
// ページ間検索時1ファイルに対する検索

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
			// 暗号化メモが対象外の場合、検索しない
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

		// ヒットしたら検索を打ち切る
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
				// サブアイテムが存在 = フォルダ
				if (!IsExpand(hViewWnd, h)) {
					// ツリーの展開
					TreeView_Expand(hViewWnd, h, TVE_EXPAND);
					TreeExpand(h);
				}
				// 再帰
				// GetChildを取り直しているのはExpandされるため
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
// ラベルの編集
/////////////////////////////////////////////

BOOL MemoSelectView::OnEditLabel()
{
	HTREEITEM hItem = TreeView_GetSelection(hViewWnd);
	if (hItem == NULL) return FALSE;

	TreeView_EditLabel(hViewWnd, hItem);

	return TRUE;
}