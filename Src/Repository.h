#ifndef REPOSITORY_H
#define REPOSITORY_H

class TString;
class TomboURI;
class MemoNote;
class RepositoryImpl;
class URIOption;
class RepositoryOption;
class DirList;

/////////////////////////////////////////
// Option flag definitions
/////////////////////////////////////////

#define NOTE_OPTIONMASK_ENCRYPTED  1
#define NOTE_OPTIONMASK_SAFEFILE   2
#define NOTE_OPTIONMASK_VALID      4

/////////////////////////////////////////
// Custom error code
/////////////////////////////////////////

#define ERROR_TOMBO_REP_CODE_BASE_ERROR (0x24000000)
#define ERROR_TOMBO_REP_CODE_BASE_WARN  (0x22000000)
#define ERROR_TOMBO_REP_CODE_BASE_INFO  (0x21000000)

#define ERROR_TOMBO_E_INVALIDURI         (ERROR_TOMBO_REP_CODE_BASE_ERROR + 1)
#define ERROR_TOMBO_E_SOME_ERROR_OCCURED (ERROR_TOMBO_REP_CODE_BASE_ERROR + 2)
#define ERROR_TOMBO_E_RMFILE_FAILED      (ERROR_TOMBO_REP_CODE_BASE_ERROR + 3)
#define ERROR_TOMBO_E_RMDIR_FAILED       (ERROR_TOMBO_REP_CODE_BASE_ERROR + 4)

#define ERROR_TOMBO_W_DELETEOLD_FAILED          (ERROR_TOMBO_REP_CODE_BASE_WARN + 1)
#define ERROR_TOMBO_W_OTHERFILE_EXISTS          (ERROR_TOMBO_REP_CODE_BASE_ERROR + 2)
#define ERROR_TOMBO_W_OPERATION_NOT_PERMITTED   (ERROR_TOMBO_REP_CODE_BASE_ERROR + 3)

#define ERROR_TOMBO_I_OPERATION_NOT_PERFORMED (ERROR_TOMBO_REP_CODE_BASE_INFO + 1)
#define ERROR_TOMBO_I_GET_PASSWORD_CANCELED   (ERROR_TOMBO_REP_CODE_BASE_INFO + 2)

/////////////////////////////////////////
// Repository options
/////////////////////////////////////////

class RepositoryOption {
public:
	BOOL bKeepTitle;
	BOOL bKeepCaret;
	BOOL bSafeFileName; // scramble filename
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
	BOOL Move(const TomboURI *pMoveFrom, const TomboURI *pMoveTo, URIOption *pOption);

	// Rename headline
	// Repository data is updated if necessary.
	BOOL ChangeHeadLine(const TomboURI *pURI, LPCTSTR pReqNewHeadLine, URIOption *pOption);

	// get option from URI
	//
	// if NOTE_OPTIONMASK_VALID is set by pOption, bValid and bFolder is effective.
	// if NOTE_OPTIONMASK_ENCRYPTED is set, bEncrypt is effective.
	// if NOTE_OPTIONMASK_SAFEFILE is set, bSafeFile is effective.
	BOOL GetOption(const TomboURI *pURI, URIOption *pOption);

	// set option from URI
	//
	// if NOTE_OPTIONMASK_ENCRYPTED is set and bEncrypt is TRUE and bSafeFile is FALSE, encrypt to the URI by normal mode.
	// if NOTE_OPTIONMASK_ENCRYPTED is set and bEncrypt is TRUE and bSafeFile is TRUE, encrypt to the URI by safefile mode.
	// if NOTE_OPTIONMASK_ENCRYPTED is set and bEncrypt is FALSE, decrypt to the URI.
	//
	// In current implimentation, change normal mode <-> safe file mode is not supported.
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

	// Get notes under the url.
	BOOL GetList(const TomboURI *pFolder, DirList *pList, BOOL bSkipEncrypt);

	// 
	BOOL RequestAllocateURI(LPCTSTR pMemoPath, LPCTSTR pText, TString *pHeadLine, TomboURI *pURI, const TomboURI *pTemplateURI);
};

//////////////////////////////////////////////////////////////
// URI option
//////////////////////////////////////////////////////////////

class URIOption {
public:
	URIOption(DWORD flg = 0) : nFlg(flg), pNewURI(NULL), pNewHeadLine(NULL) {}
	~URIOption();

	// request section
	DWORD nFlg;
	BOOL bEncrypt;
	BOOL bSafeFileName;
	BOOL bValid;
	BOOL bFolder;

	// result info section
	// if these value is not set by NULL, delete when NoteOption is deleted.
	TString *pNewHeadLine;
	TomboURI *pNewURI;
};


//////////////////////////////////////////////////////////////
// Global definitions
//////////////////////////////////////////////////////////////

extern Repository g_Repository;

#endif