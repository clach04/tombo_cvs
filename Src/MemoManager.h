#ifndef MEMOMANAGER_H
#define MEMOMANAGER_H

#include <commctrl.h>

class MemoDetailsView;
class MemoSelectView;
class MainFrame;
class MemoNote;
class PasswordManager;
class SearchEngineA;
class TreeViewItem;
class TString;

/////////////////////////////////////
// Memo location info
/////////////////////////////////////
//
// This class is helper class. Object life-time is only in functions.
// If you want to have these info, you should not keep pointer but copy member variables.

class MemoLocator {
	MemoNote *pNote;
	HTREEITEM hItem;
	BOOL bDeleteReceived;
public:
	MemoLocator(MemoNote *p, HTREEITEM h, BOOL bDel = FALSE) : pNote(p), hItem(h), bDeleteReceived(bDel) {}

	MemoNote *GetNote() { return pNote; }
	HTREEITEM GetHITEM() { return hItem; }
	BOOL IsDeleteReceived() { return bDeleteReceived; }
};

/////////////////////////////////////
// Control other view
/////////////////////////////////////
// control select - detail view relation and mainframe items(menu, toolbar, etc.)

class MemoManager {
protected:
	MemoDetailsView *pMemoDetailsView;
	MemoSelectView *pMemoSelectView;
	MainFrame *pMainFrame;
	PasswordManager *pPassMgr;

	// Current edit-view displaying item info
	MemoNote *pCurrentNote;
	HTREEITEM hCurrentItem;

	MemoLocator AllocNewMemo(LPCTSTR pText, MemoNote *pTemplate = NULL);

	SearchEngineA *pSearchEngineA;

	BOOL bMSSearchFlg;
	BOOL bMDSearchFlg;

	/////////////////////////////////////
	// maintain pCurrentNote;
	void SetCurrentNote(MemoLocator *pLoc);

public:
	/////////////////////////////////////
	// ctor & dtor

	MemoManager();
	~MemoManager();

	/////////////////////////////////////
	// 初期化関連

	BOOL Init(MainFrame *p, MemoDetailsView *md, MemoSelectView *ms);
	void SetPasswordManager(PasswordManager *p) { pPassMgr = p; }

	/////////////////////////////////////

	// ファイルを削除する。削除時に元の内容を0クリアする
	static BOOL WipeOutAndDeleteFile(LPCTSTR pFile);

	// 新規メモの作成
	BOOL NewMemo();

	// メモが更新されていたらセーブする
	//	pYNCにはIDOK,IDYES,IDNO,IDCANCELのいずれかが入る。
	//		セーブの必要がなかった場合にはIDOKが設定される。
	//		IDYESが設定されている場合には保存が実行された。
	//  pYNCにNULLを設定した場合、問い合わせのメッセージボックスは表示されない
	//		この場合、bDupModeがTRUEの場合にはメモは常に別ファイルとして保存される。
	//				  bDupModeがFALSEの場合には通常動作
	BOOL SaveIfModify(LPDWORD pYNC, BOOL bDupMode);

	// メモのロード
	BOOL SetMemo(MemoLocator *pLoc);

	// メモのクリア
	BOOL ClearMemo();

//	void UpdateMenu(TreeViewItem *pItem);

	BOOL MakeNewFolder(HWND hWnd, TreeViewItem *pItem);	// フォルダの新規作成

	BOOL StoreCursorPos();

	void ActivateView(BOOL bSelectViewActive);

	void SelectAll();	// 全選択(詳細ビュー)

	// 指定したメモが現在詳細ビューで表示されているか
	BOOL IsNoteDisplayed(LPCTSTR pFile);

	// 詳細ビューに表示されている場合に必要なら保存し、一覧ビューにフォーカスを移す
	// TODO: 保存依頼に置き換えられる気がする
	void InactiveDetailsView();

	/////////////////////////////////////
	// 検索関連

	void SetSearchEngine(SearchEngineA *p);
	SearchEngineA *GetSearchEngine() { return pSearchEngineA; }

	// 詳細ビューに対する検索
	BOOL SearchDetailsView(BOOL bFirstSearch, BOOL bForward, BOOL bNFMsg, BOOL bSearchFromTop);

	/////////////////////////////////////
	// データアクセサ

	PasswordManager *GetPasswordManager() { return pPassMgr; }
	MemoNote *CurrentNote() { return pCurrentNote; }
	MainFrame *GetMainFrame() { return pMainFrame; }
	MemoLocator CurrentLoc();

	BOOL GetCurrentSelectedPath(TString *pPath);

	/////////////////////////////////////
	// Notify from MemoSelectView

	void ReleaseItemNotify(MemoLocator *pLoc);
	void InsertItemNotify(MemoLocator *pLoc);

	/////////////////////////////////////
	// 検索フラグ

	// 「次を検索」で現在選択されている項目を検索対象に含めるか?

	// 一覧ビューの場合、ユーザが選択を変更したら含める
	BOOL MSSearchFlg() { return bMSSearchFlg; }
	void SetMSSearchFlg(BOOL b) { bMSSearchFlg = b; }


	BOOL MDSearchFlg() { return bMDSearchFlg; }
	void SetMDSearchFlg(BOOL b) { bMDSearchFlg = b; }

};

#endif