#ifndef DIRLIST_H
#define DIRLIST_H

#include "VarBuffer.h"
class MemoNote;

struct DirListItem {
	DWORD nNamePos;
	char bFlg;
	MemoNote *pNote;	
		// if bAllocMemoNotes is FALSE, this var is always invalid.
		// if TRUE, caller must delete this object manually.
};

class DirList {
	BOOL bChopExtension;
	BOOL bAllocMemoNotes;
public:
	TVector<DirListItem> vDirList;
	StringBufferT sbDirList;

	BOOL Init(BOOL bChopExtension, BOOL bAllocMemoNotes);
	~DirList();

	BOOL GetList(LPCTSTR pPrefix, LPCTSTR pMatchPath);

	DWORD NumItems() { return vDirList.NumItems(); }
	DirListItem *GetItem(DWORD n) { return vDirList.GetUnit(n); }
	LPCTSTR GetFileName(DWORD n) { return sbDirList.Get(n); }
};

#endif
