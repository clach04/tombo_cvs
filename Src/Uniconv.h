#ifndef UNICONV_H
#define UNICONV_H

// SJIS<->Unicode変換

// 領域はnewで動的に確保する。できなかったらNULLでSetLastError()する。
// 領域の開放は呼んだ側が行う。

LPTSTR ConvSJIS2Unicode(const char *p);
char *ConvUnicode2SJIS(LPCTSTR p);

LPTSTR ConvSJIS2UnicodeWithByte(const char *p, DWORD nByte);

// strdupのUNICODE版。領域はnew [] で確保するため、delete[] で開放すること。
LPTSTR StringDup(LPCTSTR pStr);

//////////////////////////////////
// JIS -> SJIS
//////////////////////////////////
// pOutは最低でもpInと同じ大きさを持つ必要がある。

void ConvJIS2SJIS(const char *pIn, char *pOut);

//////////////////////////////////
// SJIS->JIS
//////////////////////////////////
// 領域は動的に確保されるため、不要になった時点で delete[] する必要がある。

char *ConvSJIS2JIS(char *pStr);

//////////////////////////////////
// 漢字文字列のstrncp
//////////////////////////////////
// nLenが漢字の2バイト目を切る場合、1バイト目もコピーしない。
// そのため、SJISの文字列として常に有効な文字列となる。

void CopyKanjiString(LPTSTR pDst, LPCTSTR pSrc, DWORD nLen);

//////////////////////////////////
// BASE64 Decode
//////////////////////////////////
// 

BOOL MimeDec(char *pDst, const char *pSrc);

//////////////////////////////////////////////////
// BASE64 Encode
//////////////////////////////////////////////////

// 各エンコード単位は75文字に制限されるため、エンコード後の文字列は複数行に渡る可能性がある。
// これを受けて、エンコード結果は文字列のリストとして実現している。
// そのため、領域管理上、エンコーダはクラスとして実装する。

// 確保した領域はBase64Encoderが管理するため、開放の必要はない。
// 
// エンコードは今のところISO-2022-JP(JIS)のみ。他のコードセットについてはわからないので(^^;考慮していない。
// 2byte文字であれば、多少の修正で何とかなるとは思うのだが…

// また、Base64Encoderに食わせた文字列はASCIIであってもエンコードする。
// エンコードした文字列が全部ASCIIであってもコードセットは2022-JPとなる。(ちょっとださい)

// エンコードするためには、Base64Encoder::Encode()を呼ぶ。
// エンコードされた文字列を取得する際には、 Base64Encoder::Item *p;を宣言しておいて、
// p->Next()でループさせる。実際の行情報はp->Value()で取得できる。

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

	// pStrをエンコードしてpBufに格納。
	// pSizeは呼び出し時はpBufのサイズ。戻り時には未使用領域の位置
	BOOL EncodeBuf(char *pBuf, DWORD *pSize, char *pStr);

	DWORD GetToken(char *p, BOOL *pKanji, DWORD *pType);

	// pStrをpLineへエンコードする。
	// 入りきらなかった場合、 戻り値としてエンコード未了領域の先頭を返す。
	char *EncodeLine(char *pStr, char *pLine, BOOL *pKanji);

public:

	Base64Encoder() : head(NULL), tail(NULL) {}
	~Base64Encoder();

	BOOL Encode(char *str);

	Item *First() { return head; }
};

//////////////////////////////////
// 文字種判別用マクロ
//////////////////////////////////

#if defined(TOMBO_LANG_ENGLISH)
#define iskanji(c) (0)
#define iskanji2(c) (0)
#else
#define iskanji(c) (((unsigned char)c) >= 0x81 && ((unsigned char)c) <= 0x9F || ((unsigned char)c)>=0xE0 && ((unsigned char)c) <= 0xFC)
#define iskanji2(c) (((unsigned char)c) >= 0x40 && ((unsigned char)c) <= 0xFC && ((unsigned char)c) != 0x7F)
#endif

//////////////////////////////////
// 末尾のファイルセパレータを取る
//////////////////////////////////
// aa\xx\ -> aa\xx

void ChopFileSeparator(LPTSTR pBuf);

#endif
