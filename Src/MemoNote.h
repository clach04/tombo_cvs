#ifndef MEMONOTE_H
#define MEMONOTE_H

class PasswordManager;
class TString;
class TomboURI;
class Repository;

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

	virtual MemoNote *GetNewInstance() const = 0;
	virtual LPCTSTR GetExtension() = 0;
	virtual DWORD GetMemoIcon() = 0;

	MemoNote *Clone() const;
	BOOL Equal(MemoNote *pTarget);

	//////////////////////////////////
	// memo data operation members

	// get memo data
	virtual LPTSTR GetMemoBody(LPCTSTR pTopDir, PasswordManager *pMgr) const;
	virtual char *GetMemoBodyA(LPCTSTR pTopDir, PasswordManager *pMgr) const;

	// save memo
	virtual BOOL SaveData(PasswordManager *pMgr, const char *pMemo, LPCTSTR pWriteFile);

	//////////////////////////////////
	// Encryption/Decryption : 
	//     returns MemoNote instance after encryption/decryption.
	//     set buffer to pHeadLine
	//     if headline string is changed, *pIsModified is set to TURE and
	//     new headline string is set to buffer.

	virtual MemoNote *Decrypt(LPCTSTR pTopDir, PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified) const;

	//////////////////////////////////
	// File operation

	virtual BOOL DeleteMemoData(LPCTSTR pTopDir) const ;
	BOOL Rename(LPCTSTR pTopDir, LPCTSTR pNewName);
	static MemoNote *CopyMemo(LPCTSTR pTopDir, const MemoNote *pOrig, LPCTSTR pMemoPath, TString *pHeadLine);

	//////////////////////////////////
	// path related functions

	LPCTSTR MemoPath() const { return pPath; }
	BOOL GetURI(TomboURI *pURI) const;

	//////////////////////////////////
	// notes attributes

	virtual BOOL IsEncrypted() const { return FALSE; }

	//////////////////////////////////
	// Release buffer

	// Release buffer that is allocated by GetMemoBody()
	// This function clears buffer to zero before release
	// so we should use this func for security reasons.
	static void WipeOutAndDelete(LPTSTR pMemo);
#ifdef _WIN32_WCE
	static void WipeOutAndDelete(char *pMemo);
#endif
	static BOOL WipeOutAndDeleteFile(LPCTSTR pFile);

	static DWORD IsNote(LPCTSTR pFile);

	//////////////////////////////////
	// Factory method for MemoNote.
	// create and initialize MemoNote and return by ppNote.
	//
	// if creation failed, return FALSE.
	// if pFile is not memo, return TRUE and *ppNote sets to NULL.
	static BOOL MemoNoteFactory(LPCTSTR pFile, MemoNote **ppNote);
	static MemoNote *MemoNoteFactory(const TomboURI *pURI);

	//////////////////////////////////
	// Headline related funcs

	static BOOL GetHeadLinePath(LPCTSTR pTopDir, LPCTSTR pMemoPath, LPCTSTR pHeadLine, LPCTSTR pExt, 
								TString *pFullPath, LPTSTR *ppNotePath, TString *pNewHeadLine);
	static BOOL GetHeadLineFromMemoText(LPCTSTR pMemo, TString *pHeadLine);
	static BOOL GetHeadLineFromFilePath(LPCTSTR pFilePath, TString *pHeadLine);

	friend class LocalFileRepository;
};

////////////////////////////////////////
// Plain notes
////////////////////////////////////////

class PlainMemoNote : public MemoNote {
public:
	MemoNote *GetNewInstance() const ;
	LPCTSTR GetExtension();
	DWORD GetMemoIcon();

	LPTSTR GetMemoBody(LPCTSTR pTopDir, PasswordManager *pMgr) const;
	char *GetMemoBodyA(LPCTSTR pTopDir, PasswordManager *pMgr) const;

	BOOL SaveData(PasswordManager *pMgr, const char *pMemo, LPCTSTR pWriteFile);
};

////////////////////////////////////////
// Encrypted notes
////////////////////////////////////////

class CryptedMemoNote : public MemoNote {
protected:
	LPBYTE GetMemoBodySub(LPCTSTR pTopDir, PasswordManager *pMgr, LPDWORD pSize) const;
public:
	MemoNote *GetNewInstance() const ;
	LPCTSTR GetExtension();
	DWORD GetMemoIcon();

	LPTSTR GetMemoBody(LPCTSTR pTopDir, PasswordManager *pMgr) const;
	char *GetMemoBodyA(LPCTSTR pTopDIr, PasswordManager *pMgr) const;

	BOOL SaveData(PasswordManager *pMgr, const char *pMemo, LPCTSTR pWriteFile);

	BOOL IsEncrypted() const { return TRUE; }

	MemoNote *Decrypt(LPCTSTR pTopDir, PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified) const;
};

#endif