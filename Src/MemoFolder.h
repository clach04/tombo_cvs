#ifndef MEMOFOLDER_H
#define MEMOFOLDER_H

#include "TString.h"
#include "MemoNote.h"

class PasswordManager;

class MemoFolder {
	LPTSTR pFullPath;
	TString sErrorReason;

	BOOL EnDeCrypt(PasswordManager *pMgr, BOOL bEncrypt);

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
	BOOL Encrypt(PasswordManager *pMgr) { return EnDeCrypt(pMgr, TRUE); }
	BOOL Decrypt(PasswordManager *pMgr) { return EnDeCrypt(pMgr, FALSE); }

	LPCTSTR GetErrorReason() { return sErrorReason.Get(); }
};

#endif
