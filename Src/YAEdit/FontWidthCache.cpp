#include <windows.h>
#include "FontWidthCache.h"

////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////

FontWidthCache::FontWidthCache() : hRefDC(NULL), hWndRef(NULL)
{
}

FontWidthCache::~FontWidthCache()
{
	if (hRefDC) {
		ReleaseDC(hWndRef, hRefDC);
	}
	Clear();
}


BOOL FontWidthCache::Init(HWND h)
{
	hWndRef = h;
	hRefDC = GetDC(hWndRef);
	Clear();
	return TRUE;
}

void FontWidthCache::Clear()
{
	memset(bWidthTable, 0, FONT_TABLE_SIZE);
}

#if defined(PLATFORM_WIN32)

BYTE FontWidthCache::GetOnebyteCharWidth(const char c)
{
	DWORD nIdx = c;

//	if (bWidthTable[nIdx]) return bWidthTable[nIdx];

	SIZE size;
	GetTextExtentPoint32(hRefDC, &c, 1, &size);
//	bWidthTable[nIdx] = (BYTE)size.cx;
	return (BYTE)size.cx;
}

BYTE FontWidthCache::GetTwobyteCharWidth(const char *p)
{
//	DWORD nIdx = ((const unsigned char)c2) << 8 | ((const unsigned char)c1);

//	if (bWidthTable[nIdx]) return bWidthTable[nIdx];

	SIZE size;
	GetTextExtentPoint32(hRefDC, p, 2, &size);
//	bWidthTable[nIdx] = (BYTE)size.cx;
	return (BYTE)size.cx;
}

#else

BYTE FontWidthCache::GetWideCharWidth(const WCHAR c)
{
	DWORD nIdx = c;

	if (bWidthTable[nIdx]) return bWidthTable[nIdx];

	SIZE size;
	GetTextExtentPoint32(hRefDC, &c, 1, &size);
	bWidthTable[nIdx] = (BYTE)size.cx;
	return (BYTE)size.cx;
}

#endif

