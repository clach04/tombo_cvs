#ifndef REPOSITORY_H
#define REPOSITORY_H

class TString;
class TomboURI;

/////////////////////////////////////////
// Notes repository
/////////////////////////////////////////

class Repository {
public:
	Repository();
	virtual ~Repository();

//	virtual BOOL Create(LPCTSTR pRequestHeadLine, TomboURI *pTemplate, TString *pRealHeadLine, TomboURI *pRealURI) = 0;

	virtual BOOL Update(TomboURI *pCurrentURI, LPCTSTR pData, TomboURI *pNewURI, TString *pNewHeadLine) = 0;
};

////////////////////////////////////////////////
// Repository which stores notes to local file
////////////////////////////////////////////////

class LocalFileRepository : public Repository {
	LPTSTR pTopDir;
public:
	LocalFileRepository();
	virtual ~LocalFileRepository();

	BOOL Init(LPCTSTR pRoot);


//	BOOL Create(LPCTSTR pRequestHeadLine, TomboURI *pTemplate, TString *pRealHeadLine, TomboURI *pRealURI);
	BOOL Update(TomboURI *pCurrentURI, LPCTSTR pData, TomboURI *pNewURI, TString *pNewHeadLine);

};

#endif