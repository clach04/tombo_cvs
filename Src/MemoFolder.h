#ifndef MEMOFOLDER_H
#define MEMOFOLDER_H

#include "TString.h"
#include "MemoNote.h"

class MemoFolder {
	LPTSTR pFullPath;
	TString sErrorReason;
public:

	///////////////////////////////////////////////
	// ctor & dtor
	MemoFolder();
	~MemoFolder();
	BOOL Init(LPCTSTR pPath);

	BOOL Copy(LPCTSTR pDst);
	BOOL Move(LPCTSTR pDst);
	BOOL Delete();

	BOOL Rename(LPCTSTR pNewName);

	LPCTSTR GetErrorReason() { return sErrorReason.Get(); }
};

#endif
