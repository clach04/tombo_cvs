#include <windows.h>
#include <tchar.h>
#include "TString.h"
#include "TomboURI.h"

static LPCTSTR pURIPrefix = TEXT("tombo://");

/////////////////////////////////////////////
/////////////////////////////////////////////
// URI implimentation
/////////////////////////////////////////////
/////////////////////////////////////////////

/////////////////////////////////////////////
// initialize
/////////////////////////////////////////////

BOOL TomboURI::Init(LPCTSTR pURI)
{
	// check header string
	if (_tcsnicmp(pURI, pURIPrefix, 8) != 0) {
		SetLastError(ERROR_INVALID_DATA);
		return FALSE;
	}

	nMaxPathItem = 0;

	// count repository string length
	LPCTSTR p = pURI + 8;
	LPCTSTR q;
	DWORD nLv = 0;
	while (1) {
		q = GetNextSep(p);
		if (q == NULL) {
			int n = _tcslen(p);
			if (nMaxPathItem < n) nMaxPathItem = n;
			break;
		}
		if (nMaxPathItem < (q - p)) nMaxPathItem = q - p;
		p = q + 1;
		nLv++;
	}

	// check like  "tombo:///"
	if (nLv == 0) {
		SetLastError(ERROR_INVALID_DATA);
		return FALSE;
	}

	return Set(pURI);
}

/////////////////////////////////////////////
// Yet another initializer
/////////////////////////////////////////////
// This function is obsoleted and will be removed in the future.

BOOL TomboURI::InitByNotePath(LPCTSTR pNotePath)
{
	TString s;
	LPCTSTR pPrefix = TEXT("tombo://default/");
	if (!s.Alloc(_tcslen(pPrefix) + _tcslen(pNotePath) + 1)) return FALSE;

	_tcscpy(s.Get(), pPrefix);
	LPTSTR p = s.Get() + _tcslen(pPrefix);
	LPCTSTR q = pNotePath;
	if (*q == TEXT('\\')) q++;

	while(*q) {
		if (*q == TEXT('\\')) {
			*p++ = TEXT('/');
			q++;
			continue;
		}
#if defined(PLATFORM_WIN32)
		if (IsDBCSLeadByte(*q)) {
			*p++ = *q++;
		}
#endif
		*p++ = *q++;
	}
	*p = TEXT('\0');
	return Init(s.Get());
}

/////////////////////////////////////////////
// skip to next separator
/////////////////////////////////////////////

LPCTSTR TomboURI::GetNextSep(LPCTSTR pPartPath)
{
	LPCTSTR p = pPartPath;
	while (*p) {
		if (*p == TEXT('/')) return p;
		p = CharNext(p);
	}
	return NULL;
}

/////////////////////////////////////////////
// get repository name
/////////////////////////////////////////////

BOOL TomboURI::GetRepositoryName(TString *pRepo)
{
	LPCTSTR p = Get() + 8;
	LPCTSTR q = GetNextSep(p);

	if (!pRepo->Alloc(q - p + 1)) return FALSE;
	_tcsncpy(pRepo->Get(), p, q - p);
	*(pRepo->Get() + (q - p)) = TEXT('\0');

	return TRUE;
}

/////////////////////////////////////////////
// get path part
/////////////////////////////////////////////

LPCTSTR TomboURI::GetPath() const
{
	LPCTSTR p = Get() + _tcslen(pURIPrefix);

	// skip repository part
	p = _tcschr(p, TEXT('/'));
	return p;
}

/////////////////////////////////////////////
// Is the URI encrypted?
/////////////////////////////////////////////

BOOL TomboURI::IsEncrypted()
{
	DWORD n = _tcslen(Get());
	if (n > 4 && _tcscmp(Get() + n - 4, TEXT(".chi")) == 0) return TRUE;
	else return FALSE;		
}

/////////////////////////////////////////////
// Is the URI point to leaf node?
/////////////////////////////////////////////

BOOL TomboURI::IsLeaf()
{
	if (_tcslen(GetPath() + 1) == 0) return FALSE; // root
	LPCTSTR p = Get();
	return *(p + _tcslen(p) - 1) != TEXT('/');
}

/////////////////////////////////////////////
// get parent path
/////////////////////////////////////////////

BOOL TomboURI::GetParent(TomboURI *pParent)
{
	LPCTSTR p = GetPath();
	LPCTSTR pBase = p;

	if (*p) p++;

	LPCTSTR q = NULL;
	while (*p) {
		if (*p == TEXT('/')) {
			q = p;
		}
		p = CharNext(p);
	}
	if (q == NULL) {
		// result is root node.
		q = pBase + 1;
	}
	if (!pParent->Alloc(q - Get() + 1)) return FALSE;
	_tcsncpy(pParent->Get(), Get(), q - Get());
	*(pParent->Get() + (q - Get())) = TEXT('\0');
	return TRUE;
}

/////////////////////////////////////////////
// get parent path
/////////////////////////////////////////////

BOOL TomboURI::GetFilePath(TString *pPath)
{
	LPCTSTR p = GetPath();
	if (*p) *p++;

	if (!pPath->Alloc(_tcslen(p) + 1)) return FALSE;
	_tcscpy(pPath->Get(), p);

	LPTSTR q = pPath->Get();
	while(q = _tcschr(q, TEXT('/'))) {
		*q = TEXT('\\');
	}
	return TRUE;
}

/////////////////////////////////////////////
/////////////////////////////////////////////
// iterator implimentation
/////////////////////////////////////////////
/////////////////////////////////////////////

BOOL TomboURIItemIterator::Init()
{
	// reallocate buffer
	if (pBuf) delete[] pBuf;
	pBuf = new TCHAR[pURI->GetMaxPathItem() + 1];
	if (pBuf == NULL) return FALSE;

	return TRUE;
}

void TomboURIItemIterator::First()
{
	// seek head of path
	LPCTSTR p = TomboURI::GetNextSep(pURI->GetFullURI() + 8);
	p++;
	nPos = p - pURI->GetFullURI();

	Next();
}

LPCTSTR TomboURIItemIterator::Current()
{
	if (*pBuf == TEXT('\0')) return NULL;
	return pBuf;
}

void TomboURIItemIterator::Next()
{
	LPCTSTR p = pURI->GetFullURI() + nPos;

	LPTSTR q = pBuf;
	while (*p && *p != TEXT('/')) {
#if defined(PLATFORM_WIN32)
		if (IsDBCSLeadByte(*p)) {
			*q++ = *p++;
		}
#endif
		*q++ = *p++;
	}
	*q = TEXT('\0');
	if (*p == TEXT('/')) p++;

	nPos = p - pURI->GetFullURI();
}

BOOL TomboURIItemIterator::IsLeaf()
{
	LPCTSTR p = pURI->GetFullURI() + nPos;
	return (*p == TEXT('\0')) && (*(p-1) != TEXT('/'));
}