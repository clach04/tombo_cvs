#ifndef MEMOMANAGER_H
#define MEMOMANAGER_H

class MemoDetailsView;
class MemoSelectView;
class MainFrame;
class MemoNote;
class PasswordManager;
class SearchEngineA;
class TreeViewItem;
class TString;
class TomboURI;

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

	BOOL AllocNewMemo(LPCTSTR pText, LPCTSTR pTemplateURI = NULL);

	SearchEngineA *pSearchEngineA;

	BOOL bMSSearchFlg;
	BOOL bMDSearchFlg;

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
	BOOL SetMemo(TomboURI *pURI);

	// メモのクリア
	BOOL ClearMemo();

	BOOL MakeNewFolder(HWND hWnd, TreeViewItem *pItem);	// フォルダの新規作成

	BOOL StoreCursorPos();

	void SelectAll();	// 全選択(詳細ビュー)

	// 指定したメモが現在詳細ビューで表示されているか
	BOOL IsNoteDisplayed(LPCTSTR pURI);

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
	// data accessor

	PasswordManager *GetPasswordManager() { return pPassMgr; }

	MainFrame *GetMainFrame() { return pMainFrame; }

	BOOL GetCurrentSelectedPath(TString *pPath);

	/////////////////////////////////////
	// searching

	// 「次を検索」で現在選択されている項目を検索対象に含めるか?
	// 一覧ビューの場合、ユーザが選択を変更したら含める
	BOOL MSSearchFlg() { return bMSSearchFlg; }
	void SetMSSearchFlg(BOOL b) { bMSSearchFlg = b; }

	BOOL MDSearchFlg() { return bMDSearchFlg; }
	void SetMDSearchFlg(BOOL b) { bMDSearchFlg = b; }

};

#endif