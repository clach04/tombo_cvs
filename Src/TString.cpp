#include <windows.h>
#include <tchar.h>
#include "UniConv.h"
#include "Tombo.h"
#include "Property.h"
#include "TString.h"

// �w�b�h���C�����O������
#define SKIPCHAR TEXT("\\/:,;*?<>\"\t")

/////////////////////////////////////////////////
//
/////////////////////////////////////////////////

BOOL MyString::Alloc(DWORD nSize)
{
	if (pString) delete [] pString;
	pString = new TCHAR[nSize];
	if (pString == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}
	return TRUE;
}

BOOL MyString::Set(LPCTSTR p)
{
	if (!Alloc(_tcslen(p) + 1)) return FALSE;
	_tcscpy(pString, p);
	return TRUE;
}

BOOL MyString::StrCat(LPCTSTR pCat)
{
	if (pString == NULL) return Set(pCat);

	LPTSTR p = pString;
	pString = new TCHAR[_tcslen(p) + _tcslen(pCat) + 1];
	if (pString == NULL) {
		pString = p;
		return FALSE;
	}
	wsprintf(pString, TEXT("%s%s"), p, pCat);
	delete p;
	return TRUE;
}

BOOL MyString::Join(LPCTSTR p1, LPCTSTR p2)
{
	DWORD n = _tcslen(p1) + _tcslen(p2) + 1;
	if (!Alloc(n)) return FALSE;
	wsprintf(pString, TEXT("%s%s"), p1, p2);
	return TRUE;
}

BOOL MyString::Join(LPCTSTR p1, LPCTSTR p2, LPCTSTR p3)
{
	DWORD n = _tcslen(p1) + _tcslen(p2) + _tcslen(p3) + 1;
	if (!Alloc(n)) return FALSE;
	wsprintf(pString, TEXT("%s%s%s"), p1, p2, p3);
	return TRUE;
}

BOOL MyString::Join(LPCTSTR p1, LPCTSTR p2, LPCTSTR p3, LPCTSTR p4)
{
	DWORD n = _tcslen(p1) + _tcslen(p2) + _tcslen(p3) + _tcslen(p4) + 1;
	if (!Alloc(n)) return FALSE;
	wsprintf(pString, TEXT("%s%s%s%s"), p1, p2, p3, p4);
	return TRUE;
}

BOOL TString::AllocFullPath(LPCTSTR pPath)
{
	DWORD n = _tcslen(pPath) + _tcslen(g_Property.TopDir()) + 1;
	if (!Alloc(n + 1)) return FALSE;
	wsprintf(pString, TEXT("%s\\%s"), g_Property.TopDir(), pPath);
	return TRUE;
}

BOOL TString::GetDirectoryPath(LPCTSTR pFullPath)
{
	//�Ō��'\'�̈ʒu�̎擾
	LPCTSTR p = pFullPath;
	LPCTSTR q = NULL;
#ifdef _WIN32_WCE
	while (*p) {
		if (*p == TEXT('\\')) q = p;
		p++;
	}
#else
	while (*p) {
		if (*p == TEXT('\\')) q = p;
		if (iskanji(*p)) {
			p++;
		}
		p++;
	}
#endif

	if (q == NULL) {
		// '\'�����݂��Ȃ����� = �f�B���N�g�����͑��݂��Ȃ�����
		if (!Alloc(1)) return FALSE;
		pString[0] = TEXT('\0');
	} else {
		DWORD nDirStringLen = q - pFullPath + 1;
		if (!Alloc(nDirStringLen + 1)) return FALSE;
		_tcsncpy(pString, pFullPath, nDirStringLen);
		pString[nDirStringLen] = TEXT('\0');
	}
	return TRUE;
}

void TString::ChopExtension()
{
	if (!pString) return;

	DWORD l = _tcslen(pString);
	if (l > 4) {
		pString[l - 4] = TEXT('\0');
	}
}

void TString::ChopFileNumber()
{
	if (pString == NULL || *pString == TEXT('\0')) return;

	DWORD n = _tcslen(pString);
	LPTSTR p = pString + n - 1;
	if (*p != TEXT(')')) return;

	p--;
	while(p >= pString) {
		if (*p == TEXT('(')) {
			*p = TEXT('\0');
			return;
		}
		if (*p < TEXT('0') || *p > TEXT('9')) break;
		p--;
	}
	return;
}

static LPTSTR GetTail(LPTSTR pBuf)
{
#ifdef _WIN32_WCE
	return pBuf + _tcslen(pBuf) - 1;
#else
	LPTSTR p = pBuf;
	LPTSTR pTail = pBuf;
	while(*p) {
		if (iskanji(*p)) {
			pTail = p++;
			if (*p) p++;
		} else {
			pTail = p++;
		}
	}
	return pTail;
#endif
}

void TString::ChopFileSeparator()
{
	LPTSTR p;
	while(TRUE) {
		p = GetTail(pString);
		if (*p == TEXT('\\')) {
			*p = TEXT('\0');
		} else {
			break;
		}
	}

}

BOOL TString::GetPathTail(LPCTSTR pFullPath)
{
	//�Ō��'\'�̈ʒu�̎擾
	LPCTSTR p = pFullPath;
	LPCTSTR q = NULL;
#ifdef _WIN32_WCE
	while (*p) {
		if (*p == TEXT('\\')) q = p;
		p++;
	}
#else
	while (*p) {
		if (*p == TEXT('\\')) q = p;
		if (iskanji(*p)) {
			p++;
		}
		p++;
	}
#endif

	if (q == NULL) {
		// '\'�����݂��Ȃ����� = �f�B���N�g�����͑��݂��Ȃ�����
		return Set(pFullPath);
	} else {
		return Set(q + 1);
	}
}

////////////////////////////////////////////////////////////////////
// �t�@�C�����Ƃ��Ďg�p�ł��Ȃ������𔲂����`�ŕ�������R�s�[
////////////////////////////////////////////////////////////////////

void DropInvalidFileChar(LPTSTR pDst, LPCTSTR pSrc)
{
	LPTSTR q = pDst;
	LPCTSTR p = pSrc;

	// �t�@�C�����Ƃ��Ďg�p�ł��Ȃ��������X�L�b�v���ăw�b�h���C�����R�s�[
	while(*p) {
#ifndef _WIN32_WCE
		if (iskanji(*p)) {
			*q++ = *p++;
			*q++ = *p++;
			continue;
		}
#endif
		if (_tcschr(SKIPCHAR, *p) != NULL) {
			p++;
			continue;
		}
		*q++ = *p++;
	}
	*q = TEXT('\0');
}

////////////////////////////////////////////////////////////////////
// �p�X�t�@�C��������x�[�X��(�p�X�Ɗg���q������������)���擾
////////////////////////////////////////////////////////////////////
// ...\..\AA.txt -> AA
BOOL GetBaseName(TString *pBase, LPCTSTR pFull)
{
	LPCTSTR p = pFull;
	LPCTSTR pLastDot = NULL;
	LPCTSTR pLastYen = NULL;
	while (*p) {
#ifndef _WIN32_WCE
		if (iskanji(*p)) {
			p += 2;
			continue;
		}
#endif
		if (*p == TEXT('.')) pLastDot = p;
		if (*p == TEXT('\\')) pLastYen = p;
		p++;
	}
	if (pLastDot == NULL) pLastDot = p;
	if (pLastYen == NULL) pLastYen = pFull - 1;

	DWORD n = pLastDot - pLastYen - 1;
	if (!pBase->Alloc(n + 1)) return FALSE;
	_tcsncpy(pBase->Get(), pLastYen + 1, n);
	*(pBase->Get() + n) = TEXT('\0');
	return TRUE;
}

////////////////////////////////////////////////////////////////////
// find next '\\'
////////////////////////////////////////////////////////////////////

LPCTSTR GetNextDirSeparator(LPCTSTR pStart)
{
	LPCTSTR p = pStart;
	while(*p) {
		if (IsDBCSLeadByte((BYTE)*p)) {
			p++;
			if (*p) p++;
			continue;
		}
		if (*p == TEXT('\\')) return p;
		p++;
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////
// Get file path
////////////////////////////////////////////////////////////////////

void GetFilePath(LPTSTR pFilePath, LPCTSTR pFileName)
{
	LPCTSTR p = pFileName;
	LPCTSTR q = NULL;

	// get last position of '\'
	while(*p) {
#ifdef PLATFORM_WIN32
		if (IsDBCSLeadByte((BYTE)*p)) {
			p+= 2;
			continue;
		}
#endif
		if (*p == TEXT('\\')) {
			q = p;
		}
		p++;
	}
	if (q == NULL) {
		*pFilePath = TEXT('\0');
		return;
	}
	_tcsncpy(pFilePath, pFileName, q - pFileName + 1);
	*(pFilePath + (q - pFileName + 1)) = TEXT('\0');
}