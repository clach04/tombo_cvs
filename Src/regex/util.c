//////////////////////////////////////////////////////////
// util.c - regex��C++����Ăяo�����߂�wrapper + ���ʃ��[�`����`


// C�����P�[�W��C++�����P�[�W�̐���������邽�߁Aregex��v�֐���
// wrap���Ă���B

#include <windows.h>
#include <string.h>

#include "regex.h"

//////////////////////////////////////////////////////////
// �O���v���g�^�C�v�錾
//////////////////////////////////////////////////////////

void re_free_pattern (struct re_pattern_buffer *);
void *xmalloc(size_t siz);

//////////////////////////////////////////////////////////
// �����v���g�^�C�v�錾
//////////////////////////////////////////////////////////

static void Init_CaseFold();

//////////////////////////////////////////////////////////
// �����ϐ�
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
// �p�^�[���̃R���p�C��
//////////////////////////////////////////////////////////
// pPattern		: ���K�\��
// bIgnoreCase	: TRUE�̏ꍇ�p���啶���������͋�ʂ��Ȃ�
// ppReason 	: �G���[���ɂ͂��̌�����������ւ̃|�C���^�Ƃ��Đݒ肳���
//				  �������͕s��
//				  NULL�̏ꍇ�ɂ͖��������
// 
// �߂�l 		: ���������ꍇ�ɂ̓p�^�[���o�b�t�@�ւ̃|�C���^��Ԃ�
//				  ���s���ɂ�ppReason�ɃG���[�����̕������ݒ肵�ĕԂ�

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
			// rp->fastmap�ɂ��Ă�re_free_pattern()�����J������
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
// �R���p�C�������p�^�[���̊J��
//////////////////////////////////////////////////////////

void Regex_Free(void *p)
{
	struct re_pattern_buffer *rp;

	if (!p) return;
	rp = (struct re_pattern_buffer*)p;
	re_free_pattern(rp);
}

//////////////////////////////////////////////////////////
// �����̎��s
//////////////////////////////////////////////////////////
// [IN]
// p		: Regex_Compile�Ŏ擾�����p�^�[��
// iStart	: �J�n�ʒu(�o�C�g)
// iRange	: �����͈�
//				�l�����̏ꍇ�ɂ͋t��������
// pTarget	: �����Ώ�
//
// [OUT] 
// pStart	:  �}�b�`������擪
// pEnd		:  �}�b�`�����񖖔�
//
// [�߂�l]
//	�}�b�`������J�n�ʒu
//  -1 : �}�b�`����
//  -2 : �����G���[������

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
// �f�o�b�O�p�R�[�h
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
// �̈�m�ۊ֘A
//////////////////////////////////////////////////////////
// regex.c�ɂă������m�ہE�J���Ɏg�p

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
// casefold�e�[�u��������
//////////////////////////////////////////////////////////
// �啶���������𖳎����ă}�b�`���O���s���ꍇ�Ɏg�p�����translate�e�[�u���̏�����

static void Init_CaseFold()
{
	int i;
	for (i = 0; i < 256; i++)
	  case_fold[i] = i;
	for (i = 'a'; i <= 'z'; i++)
	  case_fold[i] = i - ('a' - 'A');
}

//////////////////////////////////////////////////////////
// Unicode�ł̕������̃J�E���g
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
// �ȉ�2�֐��ɂ��Ă�Ruby 1.6.7��util.c ������p�����Ă��������܂����B

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

