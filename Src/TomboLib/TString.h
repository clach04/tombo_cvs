#ifndef TOMBO_STRING_H
#define TOMBO_STRING_H

////////////////////////////////////
// TCHAR string
////////////////////////////////////

class TString {
	LPTSTR pString;
public:
	TString() : pString(NULL) {}
	~TString() { if (pString) delete [] pString; }

	LPTSTR Get() { return pString; }

	BOOL Alloc(DWORD nSize);
	BOOL Set(LPCTSTR p);
	BOOL StrCat(LPCTSTR p);

	BOOL Join(LPCTSTR p1, LPCTSTR p2);
	BOOL Join(LPCTSTR p1, LPCTSTR p2, LPCTSTR p3);
	BOOL Join(LPCTSTR p1, LPCTSTR p2, LPCTSTR p3, LPCTSTR p4);


	// TOMBOROOT\\pPath なる文字列を生成
//	BOOL AllocFullPath(LPCTSTR pPath);

	// pFullPathからディレクトリ部のみを取得
	// XX\YY\ZZ => XX\YY
	BOOL GetDirectoryPath(LPCTSTR pFullPath);

	// pFullPathから末尾を取得
	// XX\YY\ZZ => ZZ
	BOOL GetPathTail(LPCTSTR pFullPath);

	void ChopExtension();
	void ChopFileNumber();
	void ChopFileSeparator();
};

////////////////////////////////////
// WCHAR string
////////////////////////////////////

class WString {
	LPWSTR pString;
public:
	WString() : pString(NULL) {}
	~WString() { if (pString) delete [] pString; }

	LPWSTR Get() { return pString; }
	BOOL Alloc(DWORD nLetters);

	BOOL Set(TString *pSrc);
};

////////////////////////////////////
//  File path operation funcs.
////////////////////////////////////

// Eliminate letters "\\/:,;*?<>\"\t" from pSrc.
// pDst has at least same size of pSrc
void DropInvalidFileChar(LPTSTR pDst, LPCTSTR pSrc);

// Get base file name (except path and extensions)
BOOL GetBaseName(TString *pBase, LPCTSTR pFull);

LPCTSTR GetNextDirSeparator(LPCTSTR pStart);

// chop file name and leave path.
// ex. C:\foo\bar\baz.txt -> C:\foo\bar\ 
void GetFilePath(LPTSTR pPath, LPCTSTR pFullPath);

#endif