#include <windows.h>
#include <tchar.h>

#include "Uniconv.h"
#include "DirectoryScanner.h"

//////////////////////////////////////////////////
// �f�B���N�g���K�w�𑖍�����
//////////////////////////////////////////////////

BOOL DirectoryScanner::Init(LPCTSTR p, DWORD nFlg)
{
	_tcscpy(aScanPath, p);

	// ������\�ŏI��悤�ɒ���
	ChopFileSeparator(aScanPath);
	_tcscat(aScanPath, TEXT("\\"));

	nScanFlag = nFlg;
	return TRUE;
}


BOOL DirectoryScanner::Scan()
{
	bContinue = TRUE;

	InitialScan();
	if (bContinue) {
		ScanDirectory();
	}
	AfterScan();
	return bContinue;
}

//////////////////////////////////////////////////
// �X�L������������
//////////////////////////////////////////////////

void DirectoryScanner::ScanDirectory()
{
	LPTSTR pTail = aScanPath + _tcslen(aScanPath);

	_tcscat(pTail, TEXT("*.*"));

	WIN32_FIND_DATA wfd;
	HANDLE hHandle = FindFirstFile(aScanPath, &wfd);
	if (hHandle != INVALID_HANDLE_VALUE) {
		do {
			if (_tcscmp(wfd.cFileName, TEXT(".")) == 0 || _tcscmp(wfd.cFileName, TEXT("..")) == 0) continue;
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// �t�H���_
				if (!(nScanFlag & DIRECTORY_SCAN_NOREC)) {
					_tcscpy(pTail, wfd.cFileName);
					_tcscat(pTail, TEXT("\\"));

					PreDirectory(wfd.cFileName);
					ScanDirectory();
					PostDirectory(wfd.cFileName);
					if (!bContinue) break;
				}
			} else {
				// �t�@�C��
				if (!(nScanFlag & DIRECTORY_SCAN_NOFILE)) {
					_tcscpy(pTail, wfd.cFileName);
					File(wfd.cFileName);
				}
				if (!bContinue) break;
			}
		} while(FindNextFile(hHandle, &wfd));
		FindClose(hHandle);
	}
	*pTail = TEXT('\0');
}
