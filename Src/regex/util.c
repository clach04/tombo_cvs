#include <windows.h>
#include <string.h>

#include "oniguruma.h"

// regex library wrapper

static OnigEncoding GetNativeEncoding()
{
	OnigEncoding enc;

	UINT acp = GetACP();
	switch(acp) {
	case 932:	// Japan
		enc = ONIG_ENCODING_SJIS;
		break;
//	case 936:	// Chinese (PRC, Singapore)
//		break;
//	case 949:	// Korean
//		break;
//	case 950:	// Chinese (Taiwan; Hong Kong SAR, PRC) 
//		break;
//	case 1250:	// Eastern European 
//		break;
	case 1251:	// Cyrillic
		enc = ONIG_ENCODING_ISO_8859_5;
		break;
	case 1252:	// Latin 1
		enc = ONIG_ENCODING_ISO_8859_1;
		break;
	case 1253:	// Greek
		enc = ONIG_ENCODING_ISO_8859_7;
		break;
	default:	// 
		enc = ONIG_ENCODING_UTF16_LE;
	}
	return enc;
}

//////////////////////////////////////////////////////////
// Prepare and compile regex pattern
//////////////////////////////////////////////////////////
// pPattern		: expression pattern
//                  In CE version, pPattern's encoding is UCS2. 
// bIgnoreCase	: is case insensitive?
// ppReason 	: In current, ignored.
// 
// result 		: if success, returns pointer to pattern which is compiled.
//				  if fail, return NULL

void* Regex_Compile(const char *pPattern, BOOL bIgnoreCase, const char **ppReason)
{
	regex_t *reg;
	OnigErrorInfo einfo;

	UChar *pattern = (UChar*)pPattern;
	OnigEncoding enc;
	OnigOptionType option;
	int len;
	int r;

	if (bIgnoreCase) {
		option = ONIG_OPTION_IGNORECASE;
	} else {
		option = ONIG_OPTION_NONE;
	}

	enc = GetNativeEncoding();
	len = strlen(pattern);

	r = onig_new(&reg, pattern, 
					 pattern + len,
					 option, enc, ONIG_SYNTAX_DEFAULT, &einfo);
	if (r != ONIG_NORMAL) {
		return NULL;
	}
	return reg;
}

//////////////////////////////////////////////////////////
// free regex pattern
//////////////////////////////////////////////////////////
//
// p  : the pointer that Regex_Compile returned.

void Regex_Free(void *p)
{
	if (p == NULL) return;
	onig_free((regex_t*)p);
}

//////////////////////////////////////////////////////////
// execute matching
//////////////////////////////////////////////////////////
// [IN]
// p		: the pointer that Regex_Compile returned.
// iStart	: start position(bytes) on pTarget
// pTarget	: target string
// bForward : TRUE if search to forward
//            FALSE if search to backword
//
// [OUT] 
// pStart	:  ptr to start char of region that is matched.
// pEnd		:  ptr to end char of region that is matched.
//
// [return]
//	>=0 : position to start char of region that is matched.
//  -1  : not matched.
//  -2  : error

int Regex_Search(void *p, int iStart, const char *pTarget, BOOL bForward, int *pStart, int *pEnd)
{
	regex_t *reg = (regex_t*)p;
	unsigned char *str;
	unsigned char *end;
	unsigned char *start, *range;
	int r;
	int result;

	OnigRegion *region;

	str = (unsigned char*)pTarget;
	end = str + onigenc_str_bytelen_null(GetNativeEncoding(), str);

	if (bForward) {
		start = str + iStart;
		range = end;
	} else {
		start = str + iStart;
		range = str;
	}
	region = onig_region_new();
	
	r = onig_search(reg, str, end, start, range, region, ONIG_OPTION_NONE);
	if (r >= 0) {
		// match
		if (region->num_regs == 0) return -2;
		if (pStart) *pStart = region->beg[0];
		if (pEnd) *pEnd = region->end[0];
		result = r;
	} else if (r == ONIG_MISMATCH) {
		// mismatch
		result = -1;
	} else {
		// error
		result = -2;
	}
	onig_region_free(region, 1);
	return result;
}

//////////////////////////////////////////////////////////
// convert Unicode position to multibyte code position
//////////////////////////////////////////////////////////
// pStr   : ptr to multibyte code string
// n      : unicode position (= number of letters)
//
// result : multibyte position

DWORD UnicodePosToMBCSPos(const char *pStr, DWORD n)
{
	UChar *pStrU = (UChar*)pStr;

	UChar *p = pStrU;
	DWORD i = 0;
	while(*p) {
		if (i >= n) break;
		p++;
		if (*p) {
			p = onigenc_get_right_adjust_char_head(GetNativeEncoding(), pStrU, p);
		}
		i++;
	}
	return (p - pStrU);
}

DWORD MBCSPosToUnicodePos(const char *pStr, DWORD n)
{
	UChar *pStrU = (UChar*)pStr;

	UChar *p = pStrU;
	DWORD i = 0;
	while(*p) {
		if ((DWORD)(p - pStrU) >= n) break;
		p++;
		if (*p) {
			p = onigenc_get_right_adjust_char_head(GetNativeEncoding(), pStrU, p);
		}
		i++;
	}
	return i;
}