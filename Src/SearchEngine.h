#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

class PasswordManager;
///////////////////////////////////////////////////////////
// テキスト検索ロジック(SJIS版)
///////////////////////////////////////////////////////////

class SearchEngineA {
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
	// ctor & dtor, 初期化
	SearchEngineA();
	~SearchEngineA();
	BOOL Init(BOOL bSearchEncrypt, BOOL bFileNameOnly, PasswordManager *pPassMgr);

	//////////////////////////////
	// パターン準備

	BOOL Prepare(LPCTSTR pPattern, BOOL bCs, const char **ppReason);

	//////////////////////////////
	// 検索の実行

	BOOL SearchForward(const char *pText, DWORD nStartPos, BOOL bShift);
	BOOL SearchBackward(const char *pText, DWORD nStartPos, BOOL bShift);

	//////////////////////////////
	// 結果取得
	DWORD MatchStart() { return nMatchStart; }
	DWORD MatchEnd() { return nMatchEnd; }

	BOOL IsSearchEncryptMemo() { return bSearchEncrypt; }
	BOOL IsFileNameOnly() { return bFileNameOnly; }

	PasswordManager *GetPasswordManager() { return pPassMgr; }
};

#endif