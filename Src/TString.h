#ifndef TOMBO_STRING_H
#define TOMBO_STRING_H

////////////////////////////////////
// String
////////////////////////////////////

class MyString {
protected:
	LPTSTR pString;
public:
	MyString() : pString(NULL) {}
	~MyString() { if (pString) delete [] pString; }

	LPTSTR Get() { return pString; }
	BOOL Alloc(DWORD nSize);
	BOOL Set(LPCTSTR p);
	BOOL StrCat(LPCTSTR p);

	BOOL Join(LPCTSTR p1, LPCTSTR p2);
	BOOL Join(LPCTSTR p1, LPCTSTR p2, LPCTSTR p3);
	BOOL Join(LPCTSTR p1, LPCTSTR p2, LPCTSTR p3, LPCTSTR p4);
};

////////////////////////////////////
// Tombo��String (^_^;
////////////////////////////////////
// ����TCHAR��T�ɂ��Ƃ���(^_^;

class TString : public MyString {
public:
	// TOMBOROOT\\pPath �Ȃ镶����𐶐�
	BOOL AllocFullPath(LPCTSTR pPath);

	// pFullPath����f�B���N�g�����݂̂��擾
	// XX\YY\ZZ => XX\YY
	BOOL GetDirectoryPath(LPCTSTR pFullPath);

	// pFullPath���疖�����擾
	// XX\YY\ZZ => ZZ
	BOOL GetPathTail(LPCTSTR pFullPath);

	void ChopExtension();
	void ChopFileNumber();
	void ChopFileSeparator();
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