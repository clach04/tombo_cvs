#ifndef UNICONV_H
#define UNICONV_H

// most of the methods are allocate buffer by new[], so use delete[] for free memoy. 

//////////////////////////////////
// Native <-> Unicode Conversion
//////////////////////////////////

LPTSTR ConvSJIS2Unicode(const char *p);
char *ConvUnicode2SJIS(LPCTSTR p);

//////////////////////////////////
// TCHAR/WCHAR version of strdup
//////////////////////////////////
// not use malloc but use new[].

LPTSTR StringDup(LPCTSTR pStr);
LPWSTR StringDupW(LPCWSTR pStr);
char *StringDupA(const char *pStr);

//////////////////////////////////
// Length count 
//////////////////////////////////
#ifdef _WIN32_WCE
// count number of MBCS strings. return value are number of letters insted of bytes.
DWORD CountMBStrings(const char *pStr, DWORD nBytes);

// count number of UNICODE bytes. return value are number of bytes when pStr converts to MBCS.
DWORD CountWCBytes(LPCTSTR pStr, DWORD nChar);
#endif

//////////////////////////////////
// LPTSTR <-> LPWSTR conversion
//////////////////////////////////
// On CE platform, same as StringDup.
// On Windows platform, MBCS <-> WBCS conversion.

LPWSTR ConvTCharToWChar(LPCTSTR p);
LPTSTR ConvWCharToTChar(LPCWSTR p);

//////////////////////////////////
// TCHAR <-> UTF-8
//////////////////////////////////

char *ConvTCharToUTF8(LPCTSTR p);
LPTSTR ConvUTF8ToTChar(const char *p);

//////////////////////////////////
// UTF-8 <--> UCS2
//////////////////////////////////

LPWSTR ConvUTF8ToUCS2(const char *pUTFData);
char *ConvUCS2ToUTF8(LPCWSTR pStr);

DWORD ConvUTF8PosToUCSPos(const char *pUTF, DWORD nUTFPos);
DWORD ConvUCSPosToUTF8Pos(const char *pUTF, DWORD nUCSPos);

char *EscapeXMLStr(LPCTSTR pStr);

//////////////////////////////////
// JIS -> SJIS
//////////////////////////////////
// pOut�͍Œ�ł�pIn�Ɠ����傫�������K�v������B

void ConvJIS2SJIS(const char *pIn, char *pOut);

//////////////////////////////////
// SJIS->JIS
//////////////////////////////////
// �̈�͓��I�Ɋm�ۂ���邽�߁A�s�v�ɂȂ������_�� delete[] ����K�v������B

char *ConvSJIS2JIS(char *pStr);

//////////////////////////////////
// �����������strncp
//////////////////////////////////
// nLen��������2�o�C�g�ڂ�؂�ꍇ�A1�o�C�g�ڂ��R�s�[���Ȃ��B
// ���̂��߁ASJIS�̕�����Ƃ��ď�ɗL���ȕ�����ƂȂ�B

void CopyKanjiString(LPTSTR pDst, LPCTSTR pSrc, DWORD nLen);


//////////////////////////////////
// BASE64 Encode
//////////////////////////////////
//
// This encoder 

char *Base64Encode(const LPBYTE pBinary, DWORD nSrcLen);
LPBYTE Base64Decode(const char *pM64str, LPDWORD pDataSize);


#ifdef COMMENT
//////////////////////////////////
// BASE64 Decode
//////////////////////////////////

BOOL MimeDec(char *pDst, const char *pSrc);

//////////////////////////////////////////////////
// BASE64 Encode
//////////////////////////////////////////////////

// �e�G���R�[�h�P�ʂ�75�����ɐ�������邽�߁A�G���R�[�h��̕�����͕����s�ɓn��\��������B
// ������󂯂āA�G���R�[�h���ʂ͕�����̃��X�g�Ƃ��Ď������Ă���B
// ���̂��߁A�̈�Ǘ���A�G���R�[�_�̓N���X�Ƃ��Ď�������B

// �m�ۂ����̈��Base64Encoder���Ǘ����邽�߁A�J���̕K�v�͂Ȃ��B
// 
// �G���R�[�h�͍��̂Ƃ���ISO-2022-JP(JIS)�̂݁B���̃R�[�h�Z�b�g�ɂ��Ă͂킩��Ȃ��̂�(^^;�l�����Ă��Ȃ��B
// 2byte�����ł���΁A�����̏C���ŉ��Ƃ��Ȃ�Ƃ͎v���̂����c

// �܂��ABase64Encoder�ɐH�킹���������ASCII�ł����Ă��G���R�[�h����B
// �G���R�[�h���������񂪑S��ASCII�ł����Ă��R�[�h�Z�b�g��2022-JP�ƂȂ�B(������Ƃ�����)

// �G���R�[�h���邽�߂ɂ́ABase64Encoder::Encode()���ĂԁB
// �G���R�[�h���ꂽ��������擾����ۂɂ́A Base64Encoder::Item *p;��錾���Ă����āA
// p->Next()�Ń��[�v������B���ۂ̍s����p->Value()�Ŏ擾�ł���B

class Base64Encoder{
public:
	class Item {
		char *pLine;
		Item *pNext;

		friend class Base64Encoder;
	public:
		Item *Next() { return (pNext && *(pNext->pLine) == 0) ? NULL : pNext; }
		const char *Line() { return pLine;}
	};

protected:

	Item *head, *tail;

	char *AllocLine();

	// pStr���G���R�[�h����pBuf�Ɋi�[�B
	// pSize�͌Ăяo������pBuf�̃T�C�Y�B�߂莞�ɂ͖��g�p�̈�̈ʒu
	BOOL EncodeBuf(char *pBuf, DWORD *pSize, char *pStr);

	DWORD GetToken(char *p, BOOL *pKanji, DWORD *pType);

	// pStr��pLine�փG���R�[�h����B
	// ���肫��Ȃ������ꍇ�A �߂�l�Ƃ��ăG���R�[�h�����̈�̐擪��Ԃ��B
	char *EncodeLine(char *pStr, char *pLine, BOOL *pKanji);

public:

	Base64Encoder() : head(NULL), tail(NULL) {}
	~Base64Encoder();

	BOOL Encode(char *str);

	Item *First() { return head; }
};
#endif

//////////////////////////////////
// �����픻�ʗp�}�N��
//////////////////////////////////

#if defined(TOMBO_LANG_ENGLISH)
#define iskanji(c) (0)
#define iskanji2(c) (0)
#else
#define iskanji(c) (((unsigned char)c) >= 0x81 && ((unsigned char)c) <= 0x9F || ((unsigned char)c)>=0xE0 && ((unsigned char)c) <= 0xFC)
#define iskanji2(c) (((unsigned char)c) >= 0x40 && ((unsigned char)c) <= 0xFC && ((unsigned char)c) != 0x7F)
#endif

//////////////////////////////////
// helper functions
//////////////////////////////////

// remove '\' 
// ex. aa\xx\ -> aa\xx
void ChopFileSeparator(LPTSTR pBuf);

void TrimRight(LPTSTR pBuf);

// Eliminate letters "\\/:,;*?<>\"\t" from pSrc.
// pDst has at least same size of pSrc
void DropInvalidFileChar(LPTSTR pDst, LPCTSTR pSrc);

LPCTSTR GetNextDirSeparator(LPCTSTR pStart);

// chop file name and leave path.
// ex. C:\foo\bar\baz.txt -> C:\foo\bar\ 
void GetFilePath(LPTSTR pPath, LPCTSTR pFullPath);

//////////////////////////////////
// Release buffer

// These function clear buffer to zero before release memory.

void WipeOutAndDelete(LPTSTR pMemo);
#ifdef _WIN32_WCE
void WipeOutAndDelete(char *pMemo);
#endif

//////////////////////////////////
// Delete file
// before deleting, write contents to zero.

BOOL WipeOutAndDeleteFile(LPCTSTR pFile);

#endif
