#include <windows.h>
#include <tchar.h>
#include "TString.h"
#include "TomboURI.h"

/////////////////////////////////////////////
/////////////////////////////////////////////
// URI implimentation
/////////////////////////////////////////////
/////////////////////////////////////////////

BOOL TomboURI::Init(LPCTSTR pURI)
{
	// check header string
	if (_tcsnicmp(pURI, TEXT("tombo://"), 8) != 0) {
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

	return sURI.Set(pURI);
}

LPCTSTR TomboURI::GetNextSep(LPCTSTR pPartPath)
{
	LPCTSTR p = pPartPath;
	while (*p) {
		if (*p == TEXT('/')) return p;
		p = CharNext(p);
	}
	return NULL;
}

BOOL TomboURI::GetRepository(TString *pRepo)
{
	LPCTSTR p = sURI.Get() + 8;
	LPCTSTR q = GetNextSep(p);

	if (!pRepo->Alloc(q - p + 1)) return FALSE;
	_tcsncpy(pRepo->Get(), p, q - p);
	*(pRepo->Get() + (q - p)) = TEXT('\0');

	return TRUE;
}

/////////////////////////////////////////////
// 
/////////////////////////////////////////////
BOOL TomboURI::GetHeadLine(TString *pHeadLine)
{
	TomboURIItemIterator itr(this);
	if (!itr.Init()) return FALSE;

	if (!pHeadLine->Alloc(GetMaxPathItem() + 1)) return FALSE;
	_tcscpy(pHeadLine->Get(), TEXT("[root]"));

	LPCTSTR p;
	for (itr.First(); p = itr.Current(); itr.Next()) {
		_tcscpy(pHeadLine->Get(), p);
		if (itr.IsLeaf()) {
			DWORD n = _tcslen(pHeadLine->Get());
			if (n >= 4) {
				*(pHeadLine->Get() + n - 4) = TEXT('\0');
			}
		}
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
	LPCTSTR p = TomboURI::GetNextSep(pURI->GetFull() + 8);
	p++;
	nPos = p - pURI->GetFull();

	Next();
}

LPCTSTR TomboURIItemIterator::Current()
{
	if (*pBuf == TEXT('\0')) return NULL;
	return pBuf;
}

void TomboURIItemIterator::Next()
{
	LPCTSTR p = pURI->GetFull() + nPos;

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

	nPos = p - pURI->GetFull();
}

BOOL TomboURIItemIterator::IsLeaf()
{
	LPCTSTR p = pURI->GetFull() + nPos;
	return (*p == TEXT('\0')) && (*(p-1) != TEXT('/'));
}