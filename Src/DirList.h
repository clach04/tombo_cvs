#ifndef DIRLIST_H
#define DIRLIST_H

#include "VarBuffer.h"
class TomboURI;

//////////////////////////////////////////////////////
// option definition
//////////////////////////////////////////////////////

#define DIRLIST_OPT_NONE          0
//#define DIRLIST_OPT_ALLOCURI      1
//#define DIRLIST_OPT_ALLOCHEADLINE 2
#define DIRLIST_OPT_ALLOCFILENAME 4

//////////////////////////////////////////////////////
// Directory item information
//////////////////////////////////////////////////////

struct DirListItem {
	DWORD nFileNamePos;
	DWORD nURIPos;
	DWORD nHeadLinePos;
	BOOL bFolder;
};

//////////////////////////////////////////////////////
// Directory list scanner
//////////////////////////////////////////////////////

class DirList {
	DWORD nOption;

	LPCTSTR pURIBase;
public:
	TVector<DirListItem> vDirList;
	StringBufferT sbDirList;

	BOOL Init(LPCTSTR pURIBase);
	~DirList();

	BOOL GetList(LPCTSTR pMatchPath, BOOL bSkipEncrypt);

	DWORD NumItems() { return vDirList.NumItems(); }
	DirListItem *GetItem(DWORD n) { return vDirList.GetUnit(n); }
	LPCTSTR GetFileName(DWORD n) { return sbDirList.Get(n); }
};

#endif
