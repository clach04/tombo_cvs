#ifndef MEMOINFO_H
#define MEMOINFO_H

#include "TString.h"

////////////////////////////////////////
// ÉÅÉÇèÓïÒDB
////////////////////////////////////////

class MemoInfo {
public:
	MemoInfo() {}
	~MemoInfo() {}

	BOOL WriteInfo(LPCTSTR pMemoPath, DWORD nPos);
	BOOL ReadInfo(LPCTSTR pMemoPath, LPDWORD pPos);

	BOOL DeleteInfo(LPCTSTR pMemoPath);
	BOOL RenameInfo(LPCTSTR pOldFilePath, LPCTSTR pNewFilePath);
};

#endif