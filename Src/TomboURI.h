#ifndef TOMBOURI_H
#define TOMBOURI_H

/////////////////////////////////////////////
// Notes path information
/////////////////////////////////////////////

class TomboURI : TString {
	int nMaxPathItem;

	friend class TomboURITest;
public:
	///////////////////////////////
	// ctor, dtor and initializer
	TomboURI() : TString() {}
	~TomboURI() {}

	BOOL Init(LPCTSTR pURI);
	BOOL InitByNotePath(LPCTSTR pNotePath);

	///////////////////////////////
	// accessor

	BOOL GetRepositoryName(TString *pRepo);

	// get full path of URI.
	LPCTSTR GetFullURI() { return Get(); }

	// get path part of URI.
	// ex. tombo://default/aa/bb/cc.txt -> /aa/bb/cc.txt
	LPCTSTR GetPath();

	// get parent path of URI.
	// ex. tombo://default/aa/bb/cc.txt -> tombo://default/aa/bb/cc/
	//     tombo://default/aa/bb/cc/    -> tombo://default/aa/bb/
	// return empty if the URI is repository root.
	//     tombo://default/ -> ""
	BOOL GetParent(TomboURI *pParent);

	DWORD GetMaxPathItem() const { return nMaxPathItem; }

	// Is the URI point to crypted file?
	// Checking does only to URI string. Not confirm to repository.
	BOOL IsEncrypted();

	// Is the URI point to leaf node?
	// Checking does only to URI string. Not confirm to repository.
	BOOL IsLeaf();

	// Get path string
	// This method will be obsoleted in future version.
	// ex. tombo://default/aaa/bbb/ccc.txt -> aaa\bbb\ccc.txt
	BOOL GetFilePath(TString *pPath);

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