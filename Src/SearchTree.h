#ifndef SEARCHTREE_H
#define SEARCHTREE_H

#include "VarBuffer.h"

class SearchEngineA;
class DirList;

#ifdef COMMENT
///////////////////////////////////////////////////////////
// SelectView iterator
///////////////////////////////////////////////////////////
// for support partial traversing on the tree, 
// current position information are implimented by stack.

struct DirStackItem {
	DWORD nPathTail;
	DirList *pDirList;
	DWORD nCurrentItem; // if not selected, 0xFFFFFFFF
};

class SelectViewIterator {
private:
	// path information
	TCHAR aPath[MAX_PATH*2];
	LPTSTR pBase;
	DWORD nBaseOffset;

	// directory stack
	DirStackItem dsStack[32]; // TODO: change vector stack
	DWORD nStackPointer;

	// scan direction
	BOOL bBack;

protected:
	BOOL Push(DWORD nPathTail);
	void Pop();
	DirStackItem *Top() { return nStackPointer ? dsStack + (nStackPointer - 1) : NULL; }
	BOOL Backpatch(LPCTSTR pString);
	BOOL LayoutInitialStack(LPCTSTR pPath);

	BOOL GetFirstFile();

	BOOL StackTopItem();
	BOOL PushOneItem(DWORD n);

public:
	~SelectViewIterator();
	BOOL Init(LPCTSTR pPath, DWORD nBaseOffset, BOOL bBack);

	BOOL Next();
	BOOL IsFinish();
	LPCTSTR CurrentPath() { return aPath; }
};
#endif

///////////////////////////////////////////////////////////
// Popup "searching" dialog and do search another thread
///////////////////////////////////////////////////////////

class SearchTree {
public:
	enum SearchResult {
		SR_FOUND,
		SR_NOTFOUND,
		SR_FAILED,
		SR_CANCELED
	};
private:
	///////////////////////////////
	// dialog

	HWND hDlgWnd;

	///////////////////////////////
	// Thread and thread control vars.

	HANDLE hSearchThread;
	BOOL bStopFlag;

	///////////////////////////////
	// for searching variables

//	SelectViewIterator *pItr;
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

	DWORD xxDebug;
};


#endif