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

void DropInvalidFileChar(LPTSTR pDst, LPCTSTR pSrc);


#endif