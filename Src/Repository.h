#ifndef REPOSITORY_H
#define REPOSITORY_H

class TString;
class TomboURI;

class MemoNote;

/////////////////////////////////////////
// Notes repository
/////////////////////////////////////////

class Repository {
public:
	Repository();
	virtual ~Repository();

//	virtual BOOL Create(LPCTSTR pRequestHeadLine, TomboURI *pTemplate, TString *pRealHeadLine, TomboURI *pRealURI) = 0;

	virtual BOOL Update(TomboURI *pCurrentURI, LPCTSTR pData, TomboURI *pNewURI, TString *pNewHeadLine) = 0;
	virtual BOOL GetHeadLine(const TomboURI *pURI, TString *pHeadLine) = 0;
};

////////////////////////////////////////////////
// Repository which stores notes to local file
////////////////////////////////////////////////

class LocalFileRepository : public Repository {
	LPTSTR pTopDir;
protected:
	BOOL Save(const TomboURI *pCurrentURI, LPCTSTR pMemo, TomboURI *pNewURI, TString *pHeadLine);

	BOOL SaveIfHeadLineIsNotChanged(MemoNote *pNote, const char *pText, LPCTSTR pOrigFile);
	BOOL SaveIfHeadLineIsChanged(MemoNote *pNote, const char *pText,
								 LPCTSTR pOrigFile, LPCTSTR pHeadLine, 
								 TString *pNewHeadLine);

public:
	LocalFileRepository();
	virtual ~LocalFileRepository();

	BOOL Init(LPCTSTR pRoot);


//	BOOL Create(LPCTSTR pRequestHeadLine, TomboURI *pTemplate, TString *pRealHeadLine, TomboURI *pRealURI);
	BOOL Update(TomboURI *pCurrentURI, LPCTSTR pData, TomboURI *pNewURI, TString *pNewHeadLine);

	BOOL GetHeadLine(const TomboURI *pURI, TString *pHeadLine);
};

#endif