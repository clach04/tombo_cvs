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

	LPTSTR Get() const { return pString; }

	BOOL Alloc(DWORD nSize);
	BOOL Set(LPCTSTR p);
	BOOL StrCat(LPCTSTR p);

	BOOL Join(LPCTSTR p1, LPCTSTR p2);
	BOOL Join(LPCTSTR p1, LPCTSTR p2, LPCTSTR p3);
	BOOL Join(LPCTSTR p1, LPCTSTR p2, LPCTSTR p3, LPCTSTR p4);

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
// String class with reference count
////////////////////////////////////

class SharedString {
	struct SharedBuf {
		LPTSTR pStr;
		WORD nRefCount;
	};

	SharedBuf *pBuf;

	friend class SharedStringTest;
protected:
	void ReleaseBuf();
	void Ref(const SharedString& s);

	BOOL Alloc(DWORD nSize);
public:
	SharedString();
	SharedString(const SharedString& s);

	~SharedString();

	BOOL Init(LPCSTR p);
	BOOL Init(const SharedString& s);

	BOOL Set(LPCTSTR p);
	BOOL Set(const SharedString& s);
	LPCTSTR Get() const;
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
//  Secure buffer
////////////////////////////////////
// secure auto ptr string buffer

// SecureBuffer decides erase range by INITIAL string length. So be careful if you want to update buffer.

class SecureBufferT {
	LPTSTR pBuf;
	DWORD nBufLen;
public:
	SecureBufferT(LPTSTR p) { pBuf = p; nBufLen = _tcslen(p); }
	~SecureBufferT();

	LPTSTR Get() { return pBuf; }
};

class SecureBufferA {
	char *pBuf;
	DWORD nBufLen;
public:
	SecureBufferA(char *p) { pBuf = p; nBufLen = strlen(p); }
	~SecureBufferA();

	char *Get() { return pBuf; }
};

#endif