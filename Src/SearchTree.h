#ifndef SEARCHTREE_H
#define SEARCHTREE_H

#include "VarBuffer.h"

enum SearchResult;
class SearchEngineA;
class DirList;

///////////////////////////////////////////////////////////
// Popup "searching" dialog and do search another thread
///////////////////////////////////////////////////////////

class SearchTree {
protected:
	///////////////////////////////
	// dialog

	HWND hDlgWnd;

	///////////////////////////////
	// Thread and thread control vars.

	HANDLE hSearchThread;
	BOOL bStopFlag;

	///////////////////////////////
	// for searching variables

	SearchEngineA *pRegex;
	BOOL bSearchDirectionForward;
	BOOL bSearchEncryptedMemo;
	BOOL bSkipOne;

	LPCTSTR pStartPath;
	TCHAR aPath[MAX_PATH*2];
	DWORD nBaseOffset;

	SearchResult srResult;

protected:
	SearchResult SearchTreeRec(LPCTSTR pNextParse, LPTSTR pBase);

	SearchResult SearchOneItem();

public:
	////////////////////////
	// ctor & dtor

	SearchTree() : pRegex(NULL), hSearchThread(NULL), hDlgWnd(NULL){}
	~SearchTree();
	BOOL Init(SearchEngineA *p, LPCTSTR pFullPath, DWORD nInitialOffset, BOOL bDirectionForward, BOOL bSkipOne);

	////////////////////////
	// Dialog callback

	void InitDialog(HWND hDlg);
	void OnClose(HWND hDlg, WORD nId);

	HWND GetWnd() { return hDlgWnd; }

	////////////////////////
	// other funcs

	void Popup(HINSTANCE hInst, HWND hParent);
	SearchResult Search();

	void CancelRequest();
	BOOL IsCancelRequest() { return bStopFlag; }

	void SetResult(SearchResult sr) { srResult = sr; }
	SearchResult GetResult() { return srResult; }

	LPCTSTR GetFullPath() { return aPath; }
	LPCTSTR GetPartPath() { return aPath + nBaseOffset + 1; }
};

#endif