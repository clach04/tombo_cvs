#ifndef TOMBOURI_H
#define TOMBOURI_H

/////////////////////////////////////////////
// Notes path information
/////////////////////////////////////////////

class TomboURI {
	TString sURI;
	int nMaxPathItem;

	friend class TomboURITest;
public:
	///////////////////////////////
	// ctor, dtor and initializer
	TomboURI() {}
	~TomboURI() { }
	BOOL Init(LPCTSTR pURI);

	///////////////////////////////
	// accessor
	BOOL GetRepository(TString *pRepo);
	BOOL GetHeadLine(TString *pHeadLine);

	LPCTSTR GetFull() { return sURI.Get(); }
	LPCTSTR GetPath();

	DWORD GetMaxPathItem() { return nMaxPathItem; }

	///////////////////////////////
	// helper functions
	static LPCTSTR GetNextSep(LPCTSTR p);
};

/////////////////////////////////////////////
// path item iterator
/////////////////////////////////////////////

class TomboURIItemIterator {
	TomboURI *pURI;
	LPTSTR pBuf;
	DWORD nPos;
public:

	///////////////////////////////
	// ctor, dtor and initializer
	TomboURIItemIterator(TomboURI *p) : pURI(p), pBuf(NULL) {}
	~TomboURIItemIterator() { if (pBuf) delete[] pBuf; }
	BOOL Init();

	///////////////////////////////
	// iteration methods
	void First();
	LPCTSTR Current();
	void Next();

	///////////////////////////////
	// May current item have child?
	BOOL IsLeaf();
};

#endif