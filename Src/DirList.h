#ifndef DIRLIST_H
#define DIRLIST_H

#include "VarBuffer.h"
class MemoNote;

//////////////////////////////////////////////////////
// option definition
//////////////////////////////////////////////////////

#define DIRLIST_OPT_NONE          0
#define DIRLIST_OPT_CHOPEXTENSION 1
#define DIRLIST_OPT_ALLOCMEMONOTE 2
#define DIRLIST_OPT_ALLOCURI      4

//////////////////////////////////////////////////////
// Directory item information
//////////////////////////////////////////////////////

struct DirListItem {
	DWORD nFileNamePos;
	DWORD nURIPos;
	BOOL bFolder;
	MemoNote *pNote;	
		// if bAllocMemoNotes is FALSE, this var is always invalid.
		// if TRUE, caller must delete this object manually.
};

//////////////////////////////////////////////////////
// Directory list scanner
//////////////////////////////////////////////////////

class DirList {
	BOOL bChopExtension;
	BOOL bAllocMemoNotes;
	LPCTSTR pURIBase;
public:
	TVector<DirListItem> vDirList;
	StringBufferT sbDirList;

	BOOL Init(DWORD nOption, LPCTSTR pURIBase);
	~DirList();

	BOOL GetList(LPCTSTR pPrefix, LPCTSTR pMatchPath);

	DWORD NumItems() { return vDirList.NumItems(); }
	DirListItem *GetItem(DWORD n) { return vDirList.GetUnit(n); }
	LPCTSTR GetFileName(DWORD n) { return sbDirList.Get(n); }
};

#endif
