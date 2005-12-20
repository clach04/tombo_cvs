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

#include "AutoPtr.h"

extern "C" {
void* Regex_Compile(const char *pPattern, BOOL bIgnoreCase, const char **ppReason);
void Regex_Free(void *p);
int Regex_Search(void *p, int iStart, const char *pTarget, BOOL bForward, int *pStart, int *pEnd);

DWORD UnicodePosToMBCSPos(const char *pStr, DWORD n);
DWORD MBCSPosToUnicodePos(const char *pStr, DWORD n);
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
// regex search (MBCS version)
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// ctor & dtor
////////////////////////////////////////////////////////

SearchEngineA::SearchEngineA() : pCompiledPattern(NULL), pPattern(NULL)
{
}

SearchEngineA::~SearchEngineA()
{
	delete [] pPattern;
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
// prepare regex pattern
////////////////////////////////////////////////////////

BOOL SearchEngineA::Prepare(LPCTSTR pPat, BOOL bCS, const char **ppReason)
{
	delete[] pPattern;	// release previous Prepare data
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
// search from text
////////////////////////////////////////////////////////

BOOL SearchEngineA::SearchTextA(const char *pText, DWORD nStartPos, BOOL bForward, BOOL bShift)
{
	const char *p = pText + nStartPos;

	if (!pCompiledPattern) return TRUE;

	// if bShift is TRUE, shift start position 1 letter
	if (bShift) {
		if (bForward) {
#if defined(PLATFORM_WIN32)
			if (IsDBCSLeadByte((BYTE)*p)) {
				p++;
			}
			if (*p) p++;
#else
			p += 2;
#endif
		} else {
#if defined(PLATFORM_WIN32)
			p--;
			if ((p-1) > pText && IsDBCSLeadByte((BYTE)*(p-1))) {
				p--;
			}
#else
			p -= 2;
#endif
		}
	}

	// execute searching
	int s, e;
	int res = Regex_Search(pCompiledPattern, (p - pText), pText, bForward, &s, &e);
	if (res >= 0) {
		nMatchStart = s;
		nMatchEnd = e;
		return TRUE;
	} else {
		return FALSE;
	}
}

BOOL SearchEngineA::SearchTextT(LPCTSTR pText, DWORD nStartPos, BOOL bForward, BOOL bShift)
{
#if defined(PLATFORM_WIN32)
	return SearchTextA(pText, nStartPos, bForward, bShift);
#else
	char *pTextA = ConvUnicode2SJIS(pText);
	SecureBufferAutoPointerA sa(pTextA);

	// convert unicode pos to MBCS pos
	DWORD nStartPosA = UnicodePosToMBCSPos(pTextA, nStartPos);

	// exec searching
	BOOL bResult = SearchTextA(pTextA, nStartPosA, bForward, bShift);

	// if matched, convert MBCS pos to unicode pos
	if (bResult) {
		DWORD nMatchStartA = nMatchStart;
		DWORD nMatchEndA = nMatchEnd;

		nMatchStart = MBCSPosToUnicodePos(pTextA, nMatchStartA);
		nMatchEnd = MBCSPosToUnicodePos(pTextA, nMatchEndA);
	}
	return bResult;
#endif
}

////////////////////////////////////////////////////////
// Matching
////////////////////////////////////////////////////////

SearchResult SearchEngineA::SearchFromURI(const TomboURI *pURI)
{
	if (bFileNameOnly) {
		TString sPartName;
		if (!g_Repository.GetFileName(pURI, &sPartName)) return SR_FAILED;

		BOOL bMatch;

		char *pText = ConvUnicode2SJIS(sPartName.Get());
		SecureBufferAutoPointerA sb(pText);
		bMatch = (Regex_Search(pCompiledPattern, 0, pText, TRUE, NULL, NULL) >= 0);

		return bMatch ? SR_FOUND : SR_NOTFOUND;
	} else {
		URIOption opt(NOTE_OPTIONMASK_ENCRYPTED);
		if (!g_Repository.GetOption(pURI, &opt)) return SR_FAILED;

		// skip crypted note if it is not search target.
		if (!IsSearchEncryptMemo() && opt.bEncrypt) return SR_NOTFOUND;

		char *pMemo = g_Repository.GetNoteDataA(pURI);
		if (pMemo == NULL) return SR_FAILED;

		BOOL bMatch = (Regex_Search(pCompiledPattern, 0, pMemo, TRUE, NULL, NULL) >= 0);
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


