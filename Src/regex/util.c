//////////////////////////////////////////////////////////
// util.c - regexをC++から呼び出すためのwrapper + 下位ルーチン定義


// CリンケージとC++リンケージの整合性を取るため、regex主要関数を
// wrapしている。

#include <windows.h>
#include <string.h>

#include "regex.h"

//////////////////////////////////////////////////////////
// 外部プロトタイプ宣言
//////////////////////////////////////////////////////////

void re_free_pattern (struct re_pattern_buffer *);
void *xmalloc(size_t siz);

//////////////////////////////////////////////////////////
// 内部プロトタイプ宣言
//////////////////////////////////////////////////////////

static void Init_CaseFold();

//////////////////////////////////////////////////////////
// 内部変数
//////////////////////////////////////////////////////////

//static HANDLE hRegexHeap = NULL;
static char case_fold[256];


#ifdef COMMENT
BOOL Regex_InitHeap()
{
	if (hRegexHeap) return TRUE;
	hRegexHeap = HeapCreate(0, 2048, 0);
	if (hRegexHeap) return TRUE;
	return FALSE;
}

void Regex_FreeHeap()
{
	if (!hRegexHeap) return;
	HeapDestroy(hRegexHeap);
	hRegexHeap = NULL;
}
#endif

//////////////////////////////////////////////////////////
// パターンのコンパイル
//////////////////////////////////////////////////////////
// pPattern		: 正規表現
// bIgnoreCase	: TRUEの場合英字大文字小文字は区別しない
// ppReason 	: エラー時にはその原因が文字列へのポインタとして設定される
//				  成功時は不定
//				  NULLの場合には無視される
// 
// 戻り値 		: 成功した場合にはパターンバッファへのポインタを返す
//				  失敗時にはppReasonにエラー原因の文字列を設定して返す

void* Regex_Compile(const char *pPattern, BOOL bIgnoreCase, const char **ppReason)
{
	struct re_pattern_buffer *rp;
	const char *p;

#if defined(TOMBO_LANG_ENGLISH)
	re_mbcinit(MBCTYPE_ASCII);
#else
	re_mbcinit(MBCTYPE_SJIS);
#endif

	rp = (struct re_pattern_buffer*)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, sizeof(struct re_pattern_buffer));
	if (!rp) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		if (ppReason) *ppReason = "Not enough memory";
		return NULL;
	}

	rp->buffer = 0;
	rp->allocated = 0;
	if (bIgnoreCase) {
		Init_CaseFold();
		rp->options |= RE_OPTION_IGNORECASE;
		re_set_casetable(case_fold);
	}
	p = re_compile_pattern(pPattern, strlen(pPattern), rp);
	if (!p) {
		// success
		rp->fastmap = (char*)xmalloc(1 << BYTEWIDTH);
			// rp->fastmapについてはre_free_pattern()側が開放する
		re_compile_fastmap(rp);
	} else {
		// fail
		if (ppReason) *ppReason = p;
		re_free_pattern(rp);
		rp = NULL;
	}
	return rp;
}

//////////////////////////////////////////////////////////
// コンパイルしたパターンの開放
//////////////////////////////////////////////////////////

void Regex_Free(void *p)
{
	struct re_pattern_buffer *rp;

	if (!p) return;
	rp = (struct re_pattern_buffer*)p;
	re_free_pattern(rp);
}

//////////////////////////////////////////////////////////
// 検索の実行
//////////////////////////////////////////////////////////
// [IN]
// p		: Regex_Compileで取得したパターン
// iStart	: 開始位置(バイト)
// iRange	: 検索範囲
//				値が負の場合には逆方向検索
// pTarget	: 検索対象
//
// [OUT] 
// pStart	:  マッチ文字列先頭
// pEnd		:  マッチ文字列末尾
//
// [戻り値]
//	マッチしたら開始位置
//  -1 : マッチせず
//  -2 : 内部エラーが発生

int Regex_Search(void *p, int iStart, int iRange, const char *pTarget, int *pStart, int *pEnd)
{
	int res;
	struct re_pattern_buffer *rp;
	struct re_registers rr;

//	TCHAR buf[1024];

	if (!p || !pStart || !pEnd) return -2;
	rp = (struct re_pattern_buffer*)p;

	memset(&rr, 0, sizeof(rr));

	res = re_search(rp, pTarget, strlen(pTarget), iStart, iRange, &rr);
//	wsprintf(buf, TEXT("%d"), res);
//	MessageBox(NULL, buf, TEXT("DEBUG"), MB_OK);

	if (res >= 0) {
		*pStart = rr.beg[0];
		*pEnd = rr.end[0];
	}
	re_free_registers(&rr);
	return res;
}

//////////////////////////////////////////////////////////
// デバッグ用コード
//////////////////////////////////////////////////////////

/*
#define PAT "^aa"
#define TGT "baa\naa"

#include "regex.h"
void re_free_pattern (struct re_pattern_buffer *);

void Test()
{
	struct re_pattern_buffer *rp;

	const char *pattern = PAT;
	const char *target = TGT;

	TCHAR buf[1024];
	int res;

	re_mbcinit(MBCTYPE_SJIS);

	rp = (struct re_pattern_buffer*)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, sizeof(struct re_pattern_buffer));
	rp->buffer = 0;
	rp->allocated = 0;

	re_compile_pattern(pattern, strlen(pattern), rp);

	res = re_search(rp, target, strlen(target), 0, strlen(target), 0);
	wsprintf(buf, TEXT("%s -> %s Result is %d"), TEXT(PAT), TEXT(TGT), res);
	MessageBox(NULL, buf, TEXT("OK"), MB_OK);

	re_free_pattern(rp);
	LocalFree(rp);
}
*/


//////////////////////////////////////////////////////////
// 領域確保関連
//////////////////////////////////////////////////////////
// regex.cにてメモリ確保・開放に使用

void *xmalloc(size_t siz)
{
	void *p;
//	p = HeapAlloc(hRegexHeap, HEAP_NO_SERIALIZE, siz);
	p = LocalAlloc(LMEM_FIXED, siz);
	return p;
}

void *xrealloc(void *p, size_t siz)
{
	void *np;
//	np = HeapReAlloc(hRegexHeap, HEAP_NO_SERIALIZE, p, siz);
	np = LocalReAlloc(p, siz, LMEM_MOVEABLE);
	return np;
}
void xfree(void *p)
{
//	HeapFree(hRegexHeap, HEAP_NO_SERIALIZE, p);
	LocalFree(p);
}

//////////////////////////////////////////////////////////
// casefoldテーブル初期化
//////////////////////////////////////////////////////////
// 大文字小文字を無視してマッチングを行う場合に使用されるtranslateテーブルの初期化

static void Init_CaseFold()
{
	int i;
	for (i = 0; i < 256; i++)
	  case_fold[i] = i;
	for (i = 'a'; i <= 'z'; i++)
	  case_fold[i] = i - ('a' - 'A');
}

//////////////////////////////////////////////////////////
// Unicodeでの文字数のカウント
//////////////////////////////////////////////////////////

int Count_Char(const char *pStr, int iEnd)
{
	const char *p = pStr;
	const char *pLimit = pStr + iEnd;
	int len = 0;
	while(*p && p < pLimit) {
		int o = (unsigned char)(*p);
		int n = re_mbctab[(unsigned char)(*p)];
		len++;
		p += n + 1;
	}
	return len;
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// 以下2関数についてはRuby 1.6.7のutil.c から引用させていただきました。

/**********************************************************************

  util.c -

  $Author$
  $Date$
  created at: Fri Mar 10 17:22:34 JST 1995

  Copyright (C) 1993-2000 Yukihiro Matsumoto

**********************************************************************/

unsigned long
scan_oct(start, len, retlen)
const char *start;
int len;
int *retlen;
{
    register const char *s = start;
    register unsigned long retval = 0;

    while (len-- && *s >= '0' && *s <= '7') {
	retval <<= 3;
	retval |= *s++ - '0';
    }
    *retlen = s - start;
    return retval;
}

unsigned long
scan_hex(start, len, retlen)
const char *start;
int len;
int *retlen;
{
    static char hexdigit[] = "0123456789abcdef0123456789ABCDEF";
    register const char *s = start;
    register unsigned long retval = 0;
    char *tmp;

    while (len-- && *s && (tmp = strchr(hexdigit, *s))) {
	retval <<= 4;
	retval |= (tmp - hexdigit) & 15;
	s++;
    }
    *retlen = s - start;
    return retval;
}

