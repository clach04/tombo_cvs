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

	BOOL SearchTextA(const char *pText, DWORD nStartPos, BOOL bForward, BOOL bShift);

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

	BOOL SearchTextT(LPCTSTR pText, DWORD nStartPos, BOOL bForward, BOOL bShift);

	SearchResult SearchFromURI(const TomboURI *pURI);

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
