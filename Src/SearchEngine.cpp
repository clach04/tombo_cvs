#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#include <string.h>

#include "SearchEngine.h"
#include "UniConv.h"

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
	pPattern = ConvUnicode2SJIS(pPat);
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
		if (IsDBCSLeadByte((BYTE)*p)) {
			p++;
		}
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
		if ((p-1) > pText && IsDBCSLeadByte((BYTE)*(p-1))) {
			p--;
		}
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
