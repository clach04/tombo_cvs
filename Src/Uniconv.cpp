#include <windows.h>
#include <tchar.h>
#include "Uniconv.h"

#ifndef ESC
#define ESC 0x1B
#endif

static void shift(BYTE *ph, BYTE *pl);

////////////////////////////////////////////////////////////
// SJIS -> Unicode変換
////////////////////////////////////////////////////////////

LPTSTR ConvSJIS2Unicode(const char *p)
{
	// 領域確保
	DWORD l = strlen(p) + 1;
	LPTSTR pUni = new TCHAR[l];
	if (pUni == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}

#ifdef _WIN32_WCE
//	sjis2unicode((LPBYTE)p, pUni, l * sizeof(TCHAR));
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)p, -1, pUni, l);
#else
	_tcscpy(pUni, p);
#endif
	return pUni;
}

////////////////////////////////////////////////////////////
// Unicode -> SJIS変換
////////////////////////////////////////////////////////////

char *ConvUnicode2SJIS(LPCTSTR p)
{
	DWORD l = (_tcslen(p) + 1)*sizeof(TCHAR);
	char *pS = new char[l];
	if (pS == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
#ifdef _WIN32_WCE
//	unicode2sjis(p, (LPBYTE)pS, l);
	WideCharToMultiByte(CP_ACP, 0, p, -1, pS, l, NULL, NULL);
#else
	strcpy(pS, p);
#endif
	return pS;
}

///////////////////////////////////////////////////
// JIS -> SJIS変換ルーチン
///////////////////////////////////////////////////
// 変換アルゴリズムは "C言語による最新アルゴリズム辞典" 奥村晴彦, 技術評論社 がベース。
#define CODEPAGE_ANSI    0
#define CODEPAGE_JIS     1
#define CODEPAGE_HANKANA 2

void ConvJIS2SJIS(const char *pIn, char *pOut)
{
	const BYTE *p = (const BYTE*)pIn;
	BYTE *q = (BYTE*)pOut;

	BYTE c, d, n;
	DWORD codepage = CODEPAGE_ANSI;

	while(*p) {
		// Is escape-sequence?

		if (*p == ESC) {
			if (*(p+1) == '$') {
				if (*(p+2) == 'B' || *(p+2) == '@') {
					p+= 3;
					codepage = CODEPAGE_JIS;
					continue;
				}
			}
			if (*(p+1) == '(') {
				n = *(p+2);
				if (n == 'H' || n == 'J' || n == 'B') {
					p+= 3;
					codepage = CODEPAGE_ANSI;
					continue;
				}
				if (n == 'I') {
					p+= 3;
					codepage = CODEPAGE_HANKANA;
					continue;
				}
			}
			if (*(p+1) == 'K') {
				codepage = CODEPAGE_JIS;
				p+= 2;
				continue;
			}
			if (*(p+1) == 'H') {
				codepage = CODEPAGE_ANSI;
				p += 2;
				continue;
			}
		}

		// for processing by codepage
		switch(codepage) {
		case CODEPAGE_ANSI:
			*q++ = *p++;
			break;
		case CODEPAGE_JIS:
			c = *p++;
			if (c >= 0x21 && c <= 0x7E) {
				if ((d = *p++) >= 0x21 && d <= 0x7E) {
					shift(&c, &d);
				}
				*q++ = c; if (d != '\0') *q++ = (char)d;
			} else if (c >= 0xA1 && c <= 0xFE) {
				if ((d = *p++) >= 0xA1 && d <= 0xFE) {
					d &= 0x7F; c &= 0x7F; shift(&c, &d);
				}
				*q++ = c; if (d != '\0') *q++ = (char)d;
			} else *q++ = c;
			break;
		case CODEPAGE_HANKANA:
			*q++ = (*p++) | 0x80;
			break;
		}
	}
	*q++ = '\0';
}

#ifdef COMMENT
void ConvJIS2SJIS(const char *pIn, char *pOut)
{
	const BYTE *p = (const BYTE*)pIn;
	BYTE *q = (BYTE*)pOut;

	int c, d;
	BOOL jiskanji = FALSE;

	while((c = *p++) != '\0') {
		if (c == ESC) {
			if ((c = *p++) == '$') {
				if ((c = *p++) == '@' || c == 'B') {
					jiskanji = TRUE;
				} else {
					*q++ = ESC; *q++ = '$';
					if (c != '\0') *q++ = (char)c;
				}
			} else if (c == '(') {
				if ((c = *p++) == 'H' || c == 'J' || c == 'B') {
					jiskanji = FALSE;
				} else {
					*q++ = ESC; *q++ = '(';
					if (c != '\0') *q++ = (char)c;
				}
			} else if (c == 'K') {
				jiskanji = TRUE;
			} else if (c == 'H') {
				jiskanji = FALSE;
			} else {
				*q++ = ESC; if (c != '\0') *q++ = (char)c;
			}
		} else if (jiskanji && c >= 0x21 && c <= 0x7E) {
			if ((d = *p++) >= 0x21 && d <= 0x7E)
				shift(&c, &d);
			*q++ = c; if (d != '\0') *q++ = (char)d;
		} else if (jiskanji && c >= 0xA1 && c <= 0xFE) {
			if ((d = *p++) >= 0xA1 && d <= 0xFE) {
				d &= 0x7F; c &= 0x7F; shift(&c, &d);
			}
			*q++ = c; if (d != '\0') *q++ = (char)d;
		} else *q++ = c;
	}
	*q++ = '\0';
}
#endif

///////////////////////////
// 変換下請け

//static void shift(int *ph, int *pl)
static void shift(BYTE *ph, BYTE *pl)
{
	if (*ph & 1) {
		if (*pl < 0x60) *pl += 0x1F;
		else *pl += 0x20;
	} else *pl += 0x7E;
	if (*ph < 0x5F) *ph = (*ph + 0xE1) >> 1;
	else *ph = (*ph + 0x161) >> 1;
}

///////////////////////////////////////////////////
// MimeDec下請け

// BASE64文字を0-63の値に変換

inline char dec64(char c)
{
	if ('A' <= c && c <= 'Z') {
		return c - 'A';
	}
	if ('a' <= c && c <= 'z') {
		return c - 'a' + 26;
	}
	if ('0' <= c && c <= '9') {
		return c - '0' + 52;
	}
	if (c == '+') return 62;
	if (c == '/') return 63;
	else return 64;
}

///////////////////////////////////////////////////
// MIME BASE64(ISO-2022-JP) デコード
///////////////////////////////////////////////////
// BASE64ヘッダ(ISO-2022-JPのみ)をデコードする

BOOL MimeDec(char *pDst, const char *pSrc)
{
	const char *p = pSrc;
	char *q = pDst;
	char b1, b2, b3;
	char c1, c2, c3, c4;

S1:
	while(*p) {
		if (*p == '=' && *(p+1) == '?') break;
		*q++ = *p++;
	}
	if (*p == '\0') {
		*q++ = '\0';
		return FALSE;
	}
	// ASSERT(*p == '=' && *(p+1) == '?')

	if (strncmp(p + 2, "iso-2022-jp?B?", 14) != 0 &&
		strncmp(p + 2, "ISO-2022-JP?B?", 14) != 0) {
		*q++ = *p++;
		*q++ = *p++;
		goto S1;
	}
	p += 16;

	while(*p) {
		if (*p == '?') break;
		c1 = dec64(*p++);
		c2 = dec64(*p++);
		c3 = dec64(*p++);
		c4 = dec64(*p++);

		b1 = (c1 << 2) | ((c2 & 0x30) >> 4);
		*q++ = b1;

		if (c3 != 64) {
			b2 = ((c2 & 0xF) << 4) | ((c3 & 0x3c) >> 2);
			*q++ = b2;
		}

		if (c3 != 64 && c4 != 64) {
			b3 = ((c3 & 0x3) << 6) | c4;
			*q++ = b3;
		}


	}
	if (*p == '\0') {
		*q++ = '\0';
		return FALSE;
	}
	if (*p == '?' && *(p+1) == '=') {
		p += 2;
		goto S1;
	}
	*q = '\0';
	return FALSE;
}


////////////////////////////////////////
// ConvSJIS2JIS下請け

// SJISの2バイトペアをJISの2バイトペアに変換

static void jis(BYTE *ph, BYTE *pl)
{
	if (*ph <= 0x9F) {
		if (*pl < 0x9F) *ph = (*ph << 1) - 0xE1;
		else			*ph = (*ph << 1) - 0xE0;
	} else {
		if (*pl < 0x9F) *ph = (*ph << 1) - 0x161;
		else			*ph = (*ph << 1) - 0x160;
	}
	if		(*pl < 0x7F) *pl -= 0x1F;
	else if (*pl < 0x9F) *pl -= 0x20;
	else				 *pl -= 0x7E;
}


//////////////////////////////////////////////
// top～*curの内容がコピーされたバッファを確保する。
// topの指す先は開放される。
// 新しいバッファはbufsizよりCONVSJIS2_JIS_EXTEND_SIZE拡張されている

#define CONVSJIS2JIS_INIT_SIZE 256
#define CONVSJIS2JIS_EXTEND_SIZE 64

static char *Resize(char *top, char **cur, int *bufsiz)
{
	char *p = new char[*bufsiz + CONVSJIS2JIS_EXTEND_SIZE];
	if (p == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	memcpy(p, top, *cur - top);
	*cur = (*cur - top) + p;
	*bufsiz += CONVSJIS2JIS_EXTEND_SIZE;
	delete [] top;
	return p;
}

//////////////////////////////////////////////////
// Shift JISから JISへの変換
//////////////////////////////////////////////////
// 変換アルゴリズムは "C言語による最新アルゴリズム辞典" 奥村晴彦, 技術評論社 がベース。
//
// 領域を確保しそこへのポインタが返される。

char *ConvSJIS2JIS(char *str)
{
	char *outbuf = new char[CONVSJIS2JIS_INIT_SIZE];
	int bufsiz = CONVSJIS2JIS_INIT_SIZE;

	if (outbuf == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}

	char *p = str;
	char *q = outbuf;

	BYTE c, d;
	BOOL bKanji = FALSE;
	while ((c = *p++) != '\0') {
		if (iskanji(c)) {
			if (bKanji == FALSE) {
				if (q - outbuf + 3 >= bufsiz) {
					outbuf = Resize(outbuf, &q, &bufsiz);
					if (q == NULL) return FALSE;
				}
				*q++ = ESC;
				*q++ = '$';
				*q++ = 'B';
				bKanji = TRUE;
			}
			d = *p++;
			if (iskanji2(d)) {
				jis(&c, &d);
				if (q - outbuf + 2 >= bufsiz) {
					outbuf = Resize(outbuf, &q, &bufsiz);
					if (q == NULL) return FALSE;
				}
				*q++ = c;
				*q++ = d;
			} else {
				if (q - outbuf >= bufsiz) {
					outbuf = Resize(outbuf, &q, &bufsiz);
					if (q == NULL) return FALSE;
				}
				*q++ = c;
				if (d != '\0') {
					if (q - outbuf >= bufsiz) {
						outbuf = Resize(outbuf, &q, &bufsiz);
						if (q == NULL) return FALSE;
					}
					*q++ = d;
				}
			}
		} else {
			if (bKanji == TRUE) {
				if (q - outbuf + 3 >= bufsiz) {
					outbuf = Resize(outbuf, &q, &bufsiz);
					if (q == NULL) return FALSE;
				}
				*q++ = ESC;
				*q++ = '(';
				*q++ = 'B';
				bKanji = FALSE;
			}
			if (q - outbuf >= bufsiz) {
				outbuf = Resize(outbuf, &q, &bufsiz);
				if (q == NULL) return FALSE;
			}
			*q++ = c;
		}
	}
	if (bKanji = TRUE) {
			if (bKanji == TRUE) {
				if (q - outbuf + 3 >= bufsiz) {
					outbuf = Resize(outbuf, &q, &bufsiz);
					if (q == NULL) return FALSE;
				}
				*q++ = ESC;
				*q++ = '(';
				*q++ = 'B';
				bKanji = FALSE;
			}
	}
	if (q - outbuf >= bufsiz) {
		outbuf = Resize(outbuf, &q, &bufsiz);
		if (q == NULL) return FALSE;
	}
	*q++ = '\0';
	return outbuf;
}

////////////////////////////////////////////////////
// Base64Encoder実装
////////////////////////////////////////////////////


// 変更するとうまく動かなくなるかも。
// BASE64_LINE_WIDTHはENCODE_BUF_SIZEより 最低でも4/3 + 18以上大きいこと。
#define BASE64_LINE_WIDTH 80
#define ENCODE_BUF_SIZ 26

#define ENC_HEADER_LEN 16
char *pEncHeader = "=?iso-2022-jp?B?";

#define ENC_FOOTER_LEN 2
char *pEncFooter = "?=";


Base64Encoder::~Base64Encoder()
{
	Item *p = head;
	Item *q;
	while(p) {
		delete [] (p->pLine);
		q = p;
		p = p->pNext;
		delete q;
	}
}

// 指定バイト以内に収める
// 漢字が含まれていたらASCIIもまとめてエンコードする
// あふれる場合には改行する
// 改行の際に文字コードがJISだったらASCIIに戻すためESC $ (を含めてエンコード
// 漢字の1byteめと2byteめの間では改行しない。


static void EncBuf(char *pIn, char *pOut)
{
	DWORD v;
	static char enctable[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	
	DWORD n = strlen(pIn);

	unsigned char *p = (unsigned char *)pIn;
	unsigned char *q = (unsigned char *)pOut;

	while(n >= 3) {
		v = *p >> 2;
		*q++ = enctable[v];
		v = ((*p & 3) << 4) + (*(p+1) >> 4);
		*q++ = enctable[v];
		v = ((*(p+1) & 0xF) << 2) + (*(p+2) >> 6);
		*q++ = enctable[v];
		v = *(p+2) & 0x3F;
		*q++ = enctable[v];

		p += 3;
		n -= 3;
	}

	if (n == 2) {
		v = *p >> 2;
		*q++ = enctable[v];
		v = ((*p & 3) << 4) + (*(p+1) >> 4);
		*q++ = enctable[v];
		v = ((*(p+1) & 0x0F) << 2);
		*q++ = enctable[v];
		*q++ = '=';
	} else if (n == 1) {
		v = *p >> 2;
		*q++ = enctable[v];
		v = (*p & 3) << 4;
		*q++ = enctable[v];
		*q++ = '=';
		*q++ = '=';
	}
	*q = '\0';
}

#define TOKEN_EOF   0
#define TOKEN_OTHER 1
#define TOKEN_KANJI 2
#define TOKEN_TO_JIS   3
#define TOKEN_TO_ASCII 4

BOOL Base64Encoder::Encode(char *str)
{
	BOOL bKanji = FALSE;
	char *pLine = AllocLine();
	if (pLine == NULL) return FALSE;
	
	char *p = str;

	while(TRUE) {
		p = EncodeLine(p, pLine, &bKanji);
		if (*p == '\0') break; 
		pLine = AllocLine();
		if (pLine == NULL) return FALSE;
	}

	return TRUE;
}

char *Base64Encoder::EncodeLine(char *pIn, char *pOut, BOOL *pKanji)
{
	char *p = pIn;

	char kbuf[ENCODE_BUF_SIZ];
	char *q = kbuf;

	BOOL bKanji = *pKanji;
	DWORD n, t;

	// 行頭の調整
	// KKKK..	=>	E$BKKKK		bKanji = T
	// aaaa..	=>  aaaa
	// E$BK..	=>  E$BK
	// E(Ba..	=>  a			bKanji = T
	if (bKanji) {
		BOOL bk = bKanji;
		n = GetToken(p, &bk, &t);
		if (t == TOKEN_KANJI) {
			*q++ = ESC;
			*q++ = '$';
			*q++ = 'B';
		} else {
			p += n;
			bKanji = FALSE;
		}
	}

	// kbufに文字をつめてゆく
	t = TOKEN_EOF;

	while(*p) {
		BOOL bk = bKanji;
		DWORD pt;
		n = GetToken(p, &bk, &pt);
		if (q - kbuf + 3 + n >= ENCODE_BUF_SIZ) break;

		bKanji = bk;
		t = pt;
		memcpy(q, p, n);
		q += n;
		p += n;
	}

	// 行末の調整
	// KKKKK	->  KKE(B	next: kanji
	// aaE$B	->	aa		next: kanji
	// aaE(B	->	aaE(B	next: ascii
	// aaaaa	->	aaaaa	next: ascii

	if (t == TOKEN_KANJI) {
		*q++ = ESC;
		*q++ = '(';
		*q++ = 'B';
	} else if (t == TOKEN_TO_JIS) {
		q -= 3;
	}
	*q++ = '\0';

	// コードの変換
	if (strlen(kbuf) > 0) {
		strcpy(pOut, pEncHeader);
		EncBuf(kbuf, pOut + ENC_HEADER_LEN);
		strcat(pOut, pEncFooter);
	} else {
		*pOut = '\0';
	}
	*pKanji = bKanji;
	return p;
}

DWORD Base64Encoder::GetToken(char *pCurrent, BOOL *pKanji, DWORD *pType)
{
	DWORD nBytes;

	if (*pCurrent == '\0') {
		*pType = TOKEN_EOF;
		nBytes = 1;
	} else if (*pCurrent == ESC && *(pCurrent+1) && *(pCurrent + 2) == 'B') {
		if (*(pCurrent + 1) == '$') {
			*pType = TOKEN_TO_JIS;
			*pKanji = TRUE;
		} else if (*(pCurrent + 1) == '(') {
			*pType = TOKEN_TO_ASCII;
			*pKanji = FALSE;
		}
		nBytes = 3;
		pCurrent += 3;
	} else if (*pKanji) {
		*pType = TOKEN_KANJI;
		nBytes = 2;
		pCurrent += 2;
	} else {
		*pType = TOKEN_OTHER;
		nBytes = 1;
	}
	return nBytes;
}

char *Base64Encoder::AllocLine()
{
	char *p = new char[BASE64_LINE_WIDTH + 1];
	if (p == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
		
	Item *pItem = new Item;
	if (pItem == NULL) {
		delete [] p;
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}

	pItem->pLine = p;
	pItem->pNext = NULL;

	if (tail) {
		tail->pNext = pItem;
		tail = pItem;
	} else {
		head = tail = pItem;
	}
	return p;
}

////////////////////////////////////////////////////
// Base64Encoder実装
////////////////////////////////////////////////////

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

void ChopFileSeparator(LPTSTR pBuf)
{
	LPTSTR p;
	while(TRUE) {
		p = GetTail(pBuf);
		if (*p == TEXT('\\')) {
			*p = TEXT('\0');
		} else {
			break;
		}
	}
}

////////////////////////////////////////////////////
// 領域を確保してコピー
////////////////////////////////////////////////////

LPTSTR StringDup(LPCTSTR pStr)
{
	DWORD l = _tcslen(pStr);
	LPTSTR p = new TCHAR[l + 1];
	if (p == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	_tcscpy(p, pStr);
	return p;
}

////////////////////////////////////////////////////
// 文字列を漢字としてコピー
////////////////////////////////////////////////////

void CopyKanjiString(LPTSTR pDst, LPCTSTR pSrc, DWORD nLen)
{
#ifdef WIN32_WCE
	LPCTSTR p = pSrc;
	LPTSTR q = pDst;
	DWORD n = 0;
	while(*p) {
		if (iskanji(*p)) {
			if (n < nLen - 1) {
				*q++ = *p++;
				*q++ = *p++;
				n+= 2;
				continue;
			} else {
				break;
			}
		} else {
			*q++ = *p++; 
			n++;
		}
	}
	*q = TEXT('\0');
#else
	_tcsncpy(pDst, pSrc, nLen);
#endif
}
