#ifndef REPOSITORYIMPL_H
#define REPOSITORYIMPL_H

class TomboURI;
class TString;
class Repository;
class URIOption;

class MemoNote;



/////////////////////////////////////////
// Repository implimentation 
/////////////////////////////////////////
// The base class of implimentation

class RepositoryImpl {
public:
public:
	RepositoryImpl();
	virtual ~RepositoryImpl();

	////////////////////////////
	// Interface definitions

	virtual BOOL Create(const TomboURI *pTemplate, LPCTSTR pData, TString *pRealHeadLine, TomboURI *pAllocedURI) = 0;

	virtual BOOL Update(TomboURI *pCurrentURI, LPCTSTR pData, TomboURI *pNewURI, TString *pNewHeadLine) = 0;
	virtual BOOL Delete(const TomboURI *pURI, URIOption *pOption) = 0;
	virtual BOOL Copy(const TomboURI *pCopyFrom, const TomboURI *pCopyTo, URIOption *pOption) = 0;
	virtual BOOL ChangeHeadLine(const TomboURI *pURI, LPCTSTR pReqNewHeadLine, URIOption *pOption) = 0;

	virtual BOOL GetHeadLine(const TomboURI *pURI, TString *pHeadLine) = 0;

	virtual BOOL GetOption(const TomboURI *pURI, URIOption *pOption) const = 0;
	virtual BOOL SetOption(const TomboURI *pCurrentURI, URIOption *pOption) = 0;

	virtual BOOL GetPhysicalPath(const TomboURI *pURI, TString *pFullPath) = 0;

	////////////////////////////
	// helper function

	BOOL IsEncrypted(const TomboURI *pURI);
};

///////////////////////////////////////////////////////////////////
// Repository implimentation which stores notes to local file
///////////////////////////////////////////////////////////////////

class LocalFileRepository : public RepositoryImpl {
	LPTSTR pTopDir;
	const RepositoryOption *pOpt;

protected:
	//////////////////////////////////////////
	// Save notes and its subroutines

	BOOL Save(const TomboURI *pCurrentURI, LPCTSTR pMemo, TomboURI *pNewURI, TString *pHeadLine);

	BOOL SaveIfHeadLineIsNotChanged(MemoNote *pNote, const char *pText, LPCTSTR pOrigFile);
	BOOL SaveIfHeadLineIsChanged(MemoNote *pNote, const char *pText,
								 LPCTSTR pOrigFile, LPCTSTR pHeadLine, 
								 TString *pNewHeadLine);

	////////////////////////////////////////
	// Encryption/Decryption subroutines
	BOOL EncryptLeaf(const TomboURI *pCurrentURI, URIOption *pOption);
	BOOL DecryptLeaf(const TomboURI *pCurrentURI, URIOption *pOption);

	TomboURI *DoEncryptFile(MemoNote *pNote, TString *pHeadLine);
	BOOL NegotiateNewName(LPCTSTR pMemoPath, LPCTSTR pText, 
							 LPCTSTR pMemoDir,
							 TString *pFullPath, LPTSTR *ppNotePath, TString *pHeadLine);
public:
	LocalFileRepository();
	virtual ~LocalFileRepository();

	BOOL Init(LPCTSTR pRoot, const RepositoryOption *pOpt);

	///////////////////////////////
	// impliment RepositoryImpl methods

	BOOL Create(const TomboURI *pTemplate, LPCTSTR pData, TString *pRealHeadLine, TomboURI *pAllocedURI);
	BOOL Update(TomboURI *pCurrentURI, LPCTSTR pData, TomboURI *pNewURI, TString *pNewHeadLine);
	BOOL Delete(const TomboURI *pURI, URIOption *pOption);
	BOOL Copy(const TomboURI *pCopyFrom, const TomboURI *pCopyTo, URIOption *pOption);
	BOOL ChangeHeadLine(const TomboURI *pURI, LPCTSTR pReqNewHeadLine, URIOption *pOption);

	BOOL GetHeadLine(const TomboURI *pURI, TString *pHeadLine);
	BOOL GetPhysicalPath(const TomboURI *pURI, TString *pFullPath);

	BOOL GetOption(const TomboURI *pURI, URIOption *pOption) const;
	BOOL SetOption(const TomboURI *pCurrentURI, URIOption *pOption);
};

#endif