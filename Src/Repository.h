#ifndef REPOSITORY_H
#define REPOSITORY_H

class TString;
class TomboURI;
class MemoNote;
class RepositoryImpl;
class URIOption;
class RepositoryOption;

/////////////////////////////////////////
// Option flag definitions
/////////////////////////////////////////

#define NOTE_OPTIONMASK_ENCRYPTED 1
//#define NOTE_OPTIONMASK_POSITION  2

/////////////////////////////////////////
// Repository options
/////////////////////////////////////////

class RepositoryOption {
public:
	BOOL bKeepTitle;
	BOOL bKeepCaret;
};

//////////////////////////////////////////////////////////////
// Repository
//////////////////////////////////////////////////////////////
// Repository is an abstraction of TOMBO's notes/folder tree.

class Repository {

protected:
	RepositoryImpl *pDefaultImpl;
	RepositoryOption roOption;

public:
	Repository();
	~Repository();

	BOOL Init(LPCTSTR pTopDir, const RepositoryOption *pOpt);

	BOOL Create(const TomboURI *pTemplate, LPCTSTR pData, TString *pRealHeadLine, TomboURI *pAllocedURI);

	BOOL Update(TomboURI *pCurrentURI, LPCTSTR pData, TomboURI *pNewURI, TString *pNewHeadLine);
	BOOL GetHeadLine(const TomboURI *pURI, TString *pHeadLine);

	BOOL Delete(const TomboURI *pURI, URIOption *pOption);
	BOOL Copy(const TomboURI *pCopyFrom, const TomboURI *pCopyTo, URIOption *pOption);
	BOOL ChangeHeadLine(const TomboURI *pURI, LPCTSTR pReqNewHeadLine, URIOption *pOption);

	BOOL GetOption(const TomboURI *pURI, URIOption *pOption);
	BOOL SetOption(const TomboURI *pCurrentURI, URIOption *pOption);

	// Get real physical path from URI.
	//
	// This method may be not supported by some RepositoryImpl type.
	BOOL GetPhysicalPath(const TomboURI *pURI, TString *pFullPath);

	/////////////////////////////
	// helper function

	// Check the URI data is encrypted
	BOOL IsEncrypted(const TomboURI *pURI);

	// Get URI the a item attached to.
	// ex. :  tombo://repo/a/b/c/ -> tombo://repo/a/b/c/
	//     :  tombo://repo/a/b/c/d.txt -> tombo://repo/a/b/c/
	BOOL GetAttachURI(const TomboURI *pBase, TomboURI *pAttached);
};

//////////////////////////////////////////////////////////////
// URI option
//////////////////////////////////////////////////////////////

class URIOption {
public:
	URIOption(DWORD flg = 0) : nFlg(flg), pNewNote(NULL), pNewURI(NULL), pNewHeadLine(NULL) {}
	~URIOption();

	// request section
	DWORD nFlg;
	BOOL bEncrypt;

	// result info section
	// if these value is not set by NULL, delete when NoteOption is deleted.
	TString *pNewHeadLine;
	TomboURI *pNewURI;
	MemoNote *pNewNote;

	// error info section
	DWORD nErrorCode; // if success, value is 0
	UINT iLevel; // MB_ICONERROR or MB_ICONWARNING
};


//////////////////////////////////////////////////////////////
// Global definitions
//////////////////////////////////////////////////////////////

extern Repository g_Repository;

#endif