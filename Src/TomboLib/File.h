#ifndef FF_FILE_H
#define FF_FILE_H

// WIN32 APIでは64bitのファイルサイズをサポートしているが、
// 通常のアプリケーションでは32bitで十分なのでプログラムの簡略化のため、
// 内部では32bitで処理する。

class File {
protected:
	HANDLE hFile;
	DWORD nSize;
public:

	// コンストラクタ&デストラクタ
	File() : hFile(INVALID_HANDLE_VALUE) {}
	~File();

	BOOL Open(LPCTSTR pFileName, DWORD nMode, DWORD nShareMode, DWORD nOpenMode);
	void Close();

	// ファイルサイズの取得
	DWORD FileSize() { return nSize; }

	// ファイルポインタのシーク
	BOOL Seek(DWORD nPos);
	// 現在のファイルポインタの取得
	DWORD CurrentPos();

	BOOL Read(LPBYTE pBuf, LPDWORD pSize);
	BOOL Write(const LPBYTE pBuf, DWORD nSize);

	BOOL WriteUnicodeString(LPCWSTR p);

	BOOL SetEOF();
};

#endif