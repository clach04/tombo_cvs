#ifndef MEMONOTE_H
#define MEMONOTE_H

class PasswordManager;
class TString;
class TomboURI;
class Repository;

#include "TreeViewItem.h"

////////////////////////////////////////
// Note type definition
////////////////////////////////////////

#define NOTE_TYPE_NO      0
#define NOTE_TYPE_PLAIN   1
#define NOTE_TYPE_CRYPTED 2
#define NOTE_TYPE_TDT     3

////////////////////////////////////////
// represents notes
////////////////////////////////////////

class MemoNote {
protected:
	LPTSTR pPath;
public:

	///////////////////////////////////////////
	// initialize

	MemoNote();
	virtual ~MemoNote();

	BOOL Init(LPCTSTR p);

	///////////////////////////////////////////

	virtual MemoNote *GetNewInstance() = 0;
	virtual LPCTSTR GetExtension() = 0;
	virtual DWORD GetMemoIcon() = 0;

	MemoNote *Clone();
	BOOL Equal(MemoNote *pTarget);

	//////////////////////////////////
	// memo data operation members

	// create new memo
	BOOL InitNewMemo(LPCTSTR pMemoPath, LPCTSTR pText, TString *pHeadLine);

	// get memo data
	virtual LPTSTR GetMemoBody(PasswordManager *pMgr);
	virtual char *GetMemoBodyA(PasswordManager *pMgr);

	// save memo
	virtual BOOL SaveData(PasswordManager *pMgr, const char *pMemo, LPCTSTR pWriteFile);

	//////////////////////////////////
	// Encryption/Decryption : 
	//     returns MemoNote instance after encryption/decryption.
	//     set buffer to pHeadLine
	//     if headline string is changed, *pIsModified is set to TURE and
	//     new headline string is set to buffer.

	virtual MemoNote *Encrypt(PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified);
	virtual MemoNote *Decrypt(PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified);

	//////////////////////////////////
	// File operation

	virtual BOOL DeleteMemoData();
	BOOL Rename(LPCTSTR pNewName);
	static MemoNote *CopyMemo(MemoNote *pOrig, LPCTSTR pMemoPath, TString *pHeadLine);

	//////////////////////////////////
	// path related functions

	LPCTSTR MemoPath() { return pPath; }
	BOOL GetURI(TomboURI *pURI);

	//////////////////////////////////
	// notes attributes

	BOOL IsReadOnly(BOOL *pReadOnly);
	virtual BOOL IsEncrypted() { return FALSE; }

	//////////////////////////////////
	// Release buffer

	// Release buffer that is allocated by GetMemoBody()
	// This function clears buffer to zero before release
	// so we should use this func for security reasons.
	static void WipeOutAndDelete(LPTSTR pMemo);
#ifdef _WIN32_WCE
	static void WipeOutAndDelete(char *pMemo);
#endif

	static DWORD IsNote(LPCTSTR pFile);

	//////////////////////////////////
	// Factory method for MemoNote.
	// create and initialize MemoNote and return by ppNote.
	//
	// if creation failed, return FALSE.
	// if pFile is not memo, return TRUE and *ppNote sets to NULL.
	static BOOL MemoNoteFactory(LPCTSTR pPrefix, LPCTSTR pFile, MemoNote **ppNote);
	static MemoNote *MemoNoteFactory(const TomboURI *pURI);
	static MemoNote *MemoNoteFactory(LPCTSTR pURI);

	friend class LocalFileRepository;
};

////////////////////////////////////////
// Plain notes
////////////////////////////////////////

class PlainMemoNote : public MemoNote {
public:
	MemoNote *GetNewInstance();
	LPCTSTR GetExtension();
	DWORD GetMemoIcon();

	LPTSTR GetMemoBody(PasswordManager *pMgr);
	char *GetMemoBodyA(PasswordManager *pMgr);

	BOOL SaveData(PasswordManager *pMgr, const char *pMemo, LPCTSTR pWriteFile);

	MemoNote *Encrypt(PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified);
};

////////////////////////////////////////
// Encrypted notes
////////////////////////////////////////

class CryptedMemoNote : public MemoNote {
protected:
	LPBYTE GetMemoBodySub(PasswordManager *pMgr, LPDWORD pSize);
public:
	MemoNote *GetNewInstance();
	LPCTSTR GetExtension();
	DWORD GetMemoIcon();

	LPTSTR GetMemoBody(PasswordManager *pMgr);
	char *GetMemoBodyA(PasswordManager *pMgr);

	BOOL SaveData(PasswordManager *pMgr, const char *pMemo, LPCTSTR pWriteFile);

	BOOL IsEncrypted() { return TRUE; }

	MemoNote *Decrypt(PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified);
};

#endif