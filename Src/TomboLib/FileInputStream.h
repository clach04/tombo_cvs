#ifndef FILEINPUTSTREAM_H
#define FILEINPUTSTREAM_H

class FileInputStream {
	HANDLE hHandle;
	char buf[4096];
	char *pCur, *pData;
	BOOL bEof;

public:
	FileInputStream(HANDLE h = INVALID_HANDLE_VALUE) : hHandle(h), pCur(buf), pData(buf), bEof(FALSE) {}

	void Assoc(HANDLE h) { hHandle = h; pCur = pData = buf; bEof = FALSE; }
	char getNext();

	// bNoCR = TRUE�̏ꍇ�ApBuf�ɐݒ肳���l�̖�����\r\n�͗��Ƃ��Đݒ肷��B
	// bNoCR = FALSE�̏ꍇ�A�����ɂ�\r\n���c��
	BOOL GetLine(char *pBuf, DWORD size, BOOL bNoCR = FALSE);
};

#endif
