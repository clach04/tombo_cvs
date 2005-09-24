#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

class PasswordManager;
enum SearchResult;
class TomboURI;

///////////////////////////////////////////////////////////
// Regular expression pattern matching (MBCS)
///////////////////////////////////////////////////////////

class SearchEngineA {
protected:
	DWORD nMatchStart;
	DWORD nMatchEnd;

	BOOL bCaseSensitive;
	BOOL bFileNameOnly;
	char *pPattern;

	BOOL bSearchEncrypt;

	void *pCompiledPattern;

	PasswordManager *pPassMgr;

public:

	//////////////////////////////
	// ctor & dtor
	SearchEngineA();
	~SearchEngineA();
	BOOL Init(BOOL bSearchEncrypt, BOOL bFileNameOnly, PasswordManager *pPassMgr);

	//////////////////////////////
	// prepare pattern

	BOOL Prepare(LPCTSTR pPattern, BOOL bCs, const char **ppReason);

	//////////////////////////////
	// exec pattern

	BOOL SearchForward(const char *pText, DWORD nStartPos, BOOL bShift);
	BOOL SearchBackward(const char *pText, DWORD nStartPos, BOOL bShift);

	SearchResult Search(const TomboURI *pURI);

	//////////////////////////////
	// get result

	DWORD MatchStart() { return nMatchStart; }
	DWORD MatchEnd() { return nMatchEnd; }

	//////////////////////////////
	// misc funcs

	BOOL IsSearchEncryptMemo() { return bSearchEncrypt; }
	BOOL IsFileNameOnly() { return bFileNameOnly; }
	PasswordManager *GetPasswordManager() { return pPassMgr; }

	SearchEngineA *Clone();
};

#endif
