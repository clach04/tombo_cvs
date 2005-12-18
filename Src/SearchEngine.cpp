#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#include <string.h>

#include "Tombo.h"
#include "SearchEngine.h"
#include "TString.h"
#include "UniConv.h"
#include "Property.h"
#include "TomboURI.h"
#include "Repository.h"

extern "C" {
void* Regex_Compile(const char *pPattern, BOOL bIgnoreCase, const char **ppReason);
void Regex_Free(void *p);
int Regex_Search(void *p, int iStart, int iRange, const char *pTarget, int *pStart, int *pEnd);
int Count_Char(const char *pStr, int iEnd);
};


#if defined(PLATFORM_HPC) || defined(PLATFORM_PSPC)
int _strnicmp(const char *p1, const char *p2, int n)
{
	const char *p, *q;
	p = p1;
	q = p2;
	char c1 = '\0', c2 = '\0';
	for (int i = 0; i < n; i++) {
		if (!*p || !*q) break;
		c1 = ('A' <= *p && *p <= 'Z') ? 'a' + (*p - 'A') : *p;
		c2 = ('A' <= *q && *q <= 'Z') ? 'a' + (*q - 'A') : *q;
		if (c1 != c2) break;
		*p++;
		*q++;
	}
	return c2 - c1;
}
#endif

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
// テキスト検索 (SJIS版)
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// ctor & dtor
////////////////////////////////////////////////////////

SearchEngineA::SearchEngineA() : pPattern(NULL), pCompiledPattern(NULL)
{
}

SearchEngineA::~SearchEngineA()
{
	if (pPattern) {
		delete [] pPattern;
	}
	if (pCompiledPattern) {
		Regex_Free(pCompiledPattern);
	}
}

BOOL SearchEngineA::Init(BOOL bSE, BOOL bFo, PasswordManager *pPMgr)
{
	bSearchEncrypt = bSE;
	bFileNameOnly = bFo;
	pPassMgr = pPMgr;
	return TRUE;
}

////////////////////////////////////////////////////////
// パターンの設定
////////////////////////////////////////////////////////

BOOL SearchEngineA::Prepare(LPCTSTR pPat, BOOL bCS, const char **ppReason)
{
#if defined(_WIN32_WCE)
	pPattern = ConvUCS2ToUTF8(pPat);
#else
	pPattern = ConvUnicode2SJIS(pPat);
#endif
	if (pPattern == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		*ppReason = "Not enough memory";
		return FALSE;
	}

	if (pCompiledPattern) {
		Regex_Free(pCompiledPattern);
	}

	pCompiledPattern = Regex_Compile(pPattern, !bCS, ppReason);
	if (!pCompiledPattern) {
		return FALSE;
	}

	bCaseSensitive = bCS;
	return TRUE;
}

////////////////////////////////////////////////////////
// 前方検索
////////////////////////////////////////////////////////

BOOL SearchEngineA::SearchForward(const char *pText, DWORD nStartPos, BOOL bShift)
{
	const char *p = pText + nStartPos;
	DWORD nPatLen = strlen(pPattern);

	if (bShift) {
#if defined(PLATFORM_WIN32)
		if (IsDBCSLeadByte((BYTE)*p)) {
			p++;
		}
#endif
		if (*p) p++;
	}

	if (!pCompiledPattern) return TRUE;

	int s, e;
	int res = Regex_Search(pCompiledPattern, (p - pText), strlen(pText), pText, &s, &e);
	if (res >= 0) {
		nMatchStart = s;
		nMatchEnd = e;
		return TRUE;
	} else {
		return FALSE;
	}
}

////////////////////////////////////////////////////////
// 後方検索
////////////////////////////////////////////////////////

BOOL SearchEngineA::SearchBackward(const char *pText, DWORD nStartPos, BOOL bShift)
{
	const char *p = pText + nStartPos;
	DWORD nPatLen = strlen(pPattern);

	if (bShift) {
		p--;
#if defined(PLATFORM_WIN32)
		if ((p-1) > pText && IsDBCSLeadByte((BYTE)*(p-1))) {
			p--;
		}
#endif
	}

	if (!pCompiledPattern) return TRUE;

	int s, e;
	int res = Regex_Search(pCompiledPattern, (p - pText), -(p - pText), pText, &s, &e);
	if (res >= 0) {
		nMatchStart = s;
		nMatchEnd = e;

		return TRUE;
	} else {
		return FALSE;
	}
	return FALSE;
}

////////////////////////////////////////////////////////
// Matching
////////////////////////////////////////////////////////

SearchResult SearchEngineA::Search(const TomboURI *pURI)
{
	if (bFileNameOnly) {
		TString sPartName;
		if (!g_Repository.GetFileName(pURI, &sPartName)) return SR_FAILED;

		BOOL bMatch;
#ifdef _WIN32_WCE
		char *bufA = ConvUCS2ToUTF8(sPartName.Get());
		bMatch = SearchForward(bufA, 0, FALSE);
		delete [] bufA;
#else
		bMatch = SearchForward(sPartName.Get(), 0, FALSE);
#endif
		return bMatch ? SR_FOUND : SR_NOTFOUND;
	} else {
		URIOption opt(NOTE_OPTIONMASK_ENCRYPTED);
		if (!g_Repository.GetOption(pURI, &opt)) return SR_FAILED;

		// skip crypted note if it is not search target.
		if (!IsSearchEncryptMemo() && opt.bEncrypt) return SR_NOTFOUND;

		char *pMemo = g_Repository.GetNoteDataUTF8(pURI);
		if (pMemo == NULL) return SR_FAILED;

		BOOL bMatch = SearchForward(pMemo, 0, FALSE);
		WipeOutAndDelete(pMemo);
		return bMatch ? SR_FOUND : SR_NOTFOUND;
	}
}

////////////////////////////////////////////////////////
// Duplicate object
////////////////////////////////////////////////////////

SearchEngineA *SearchEngineA::Clone()
{
	const char *pReason;
	SearchEngineA *p = new SearchEngineA();
	LPTSTR pPat = ConvSJIS2Unicode(pPattern);
	if (!p) return NULL;
	if (!p->Init(bSearchEncrypt, bFileNameOnly, pPassMgr) ||
		!p->Prepare(pPat, bCaseSensitive, &pReason)) {
		delete p;
		delete [] pPat;
		return NULL;
	}
	delete [] pPat;
	return p;
}


