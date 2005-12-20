#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>
#include <windows.h>

// These test cases assume default encoding as CP932(SJIS).

#include "UniConv.h"

extern "C" {
void* Regex_Compile(const char *pPattern, BOOL bIgnoreCase, const char **ppReason);
void Regex_Free(void *p);
int Regex_Search(void *p, int iStart, const char *pTarget, BOOL bForward, int *pStart, int *pEnd);

DWORD UnicodePosToMBCSPos(const char *pStr, DWORD n);
DWORD MBCSPosToUnicodePos(const char *pStr, DWORD n);
};

#define TEST_CLASS_NAME RegexTest

class TEST_CLASS_NAME : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(TEST_CLASS_NAME);

	CPPUNIT_TEST(RegexTest1);

	CPPUNIT_TEST(CaseIgnoreTest1);
	CPPUNIT_TEST(CaseIgnoreTest2);

	CPPUNIT_TEST(NoMatchTest1);
	
	CPPUNIT_TEST(KanjiTest1);
	CPPUNIT_TEST(KanjiTest2);
	CPPUNIT_TEST(KanjiTest3);
	
	CPPUNIT_TEST(ConvUTF8PosToUCSPosTest1);
	CPPUNIT_TEST(ConvUTF8PosToUCSPosTest2);
	CPPUNIT_TEST(ConvUTF8PosToUCSPosTest3);
	CPPUNIT_TEST(ConvUTF8PosToUCSPosTest4);

	CPPUNIT_TEST(ConvUCSPosToUTF8PosTest1);
	CPPUNIT_TEST(ConvUCSPosToUTF8PosTest2);
	CPPUNIT_TEST(ConvUCSPosToUTF8PosTest3);
	CPPUNIT_TEST(ConvUCSPosToUTF8PosTest4);

	CPPUNIT_TEST(UnicodePosToMBCSPosTest1);

	CPPUNIT_TEST(MBCSPosToUnicodePosTest1);

	CPPUNIT_TEST_SUITE_END();

public:
	TEST_CLASS_NAME() {}
	~TEST_CLASS_NAME() {}

	virtual void setUp() {}
	virtual void tearDown() {}

	void RegexTest1();

	void CaseIgnoreTest1();
	void CaseIgnoreTest2();

	void NoMatchTest1();

	void KanjiTest1();
	void KanjiTest2();
	void KanjiTest3();

	void ConvUTF8PosToUCSPosTest1();
	void ConvUTF8PosToUCSPosTest2();
	void ConvUTF8PosToUCSPosTest3();
	void ConvUTF8PosToUCSPosTest4();

	void ConvUCSPosToUTF8PosTest1();
	void ConvUCSPosToUTF8PosTest2();
	void ConvUCSPosToUTF8PosTest3();
	void ConvUCSPosToUTF8PosTest4();

	void UnicodePosToMBCSPosTest1();

	void MBCSPosToUnicodePosTest1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TEST_CLASS_NAME);

////////////////////////////////////////////////


void TEST_CLASS_NAME::RegexTest1() {
	const char *pReason;
	void *pPat = Regex_Compile("pat", FALSE, &pReason);
	CPPUNIT_ASSERT(pPat != NULL);

	const char *pString = "aaa bbb pap pat pas";

	int nStart, nEnd;

	int n = Regex_Search(pPat, 0, pString, TRUE, &nStart, &nEnd);

	CPPUNIT_ASSERT(n == 12);
	CPPUNIT_ASSERT(nStart == 12);
	CPPUNIT_ASSERT(nEnd == 15);
}

void TEST_CLASS_NAME::CaseIgnoreTest1() {
	const char *pReason;
	void *pPat = Regex_Compile("PAT", FALSE, &pReason);
	CPPUNIT_ASSERT(pPat != NULL);

	const char *pString = "aaa bbb pat PAT pas";

	int nStart, nEnd;

	int n = Regex_Search(pPat, 0, pString, TRUE, &nStart, &nEnd);

	CPPUNIT_ASSERT(n == 12);
	CPPUNIT_ASSERT(nStart == 12);
	CPPUNIT_ASSERT(nEnd == 15);
}


void TEST_CLASS_NAME::CaseIgnoreTest2() {
	const char *pReason;
	void *pPat = Regex_Compile("PAT", TRUE, &pReason);
	CPPUNIT_ASSERT(pPat != NULL);

	const char *pString = "aaa bbb pat PAT pas";

	int nStart, nEnd;

	int n = Regex_Search(pPat, 0, pString, TRUE, &nStart, &nEnd);

	CPPUNIT_ASSERT(n == 8);
	CPPUNIT_ASSERT(nStart == 8);
	CPPUNIT_ASSERT(nEnd == 11);
}

void TEST_CLASS_NAME::NoMatchTest1() {
	const char *pReason;
	void *pPat = Regex_Compile("pat", FALSE, &pReason);
	CPPUNIT_ASSERT(pPat != NULL);

	const char *pString = "aaa bbb pap pad pas";

	int nStart, nEnd;

	int n = Regex_Search(pPat, 0, pString, TRUE, &nStart, &nEnd);

	CPPUNIT_ASSERT(n == -1);
}

void TEST_CLASS_NAME::KanjiTest1() {

	const char *pReason;
	void *pPat = Regex_Compile("Š¿Žš", FALSE, &pReason);
	CPPUNIT_ASSERT(pPat != NULL);

	char *pString = "aaa Š¿Žš pap pad pas";

	int nStart, nEnd;

	int n = Regex_Search(pPat, 0, pString, TRUE, &nStart, &nEnd);

	CPPUNIT_ASSERT(n == 4);
	CPPUNIT_ASSERT(nStart == 4);
	CPPUNIT_ASSERT(nEnd == 8);
}

void TEST_CLASS_NAME::KanjiTest2() {

	const char *pReason;
	void *pPat = Regex_Compile("Š¿+Žš", FALSE, &pReason);
	CPPUNIT_ASSERT(pPat != NULL);

	char *pString = "aŠ¿Š¿Žš pap pad pas";

	int nStart, nEnd;

	int n = Regex_Search(pPat, 0, pString, TRUE, &nStart, &nEnd);

	CPPUNIT_ASSERT(n == 1);
	CPPUNIT_ASSERT(nStart == 1);
	CPPUNIT_ASSERT(nEnd==7);
}

void TEST_CLASS_NAME::KanjiTest3() {
	const char *pReason;
	void *pPat = Regex_Compile("[Š¿Žš]+", FALSE, &pReason);
	CPPUNIT_ASSERT(pPat != NULL);

	char *pString = "ƒeƒXƒgaŠ¿Š¿ŽšŽšŠ¿Žš pap pad pas";

	int nStart, nEnd;

	int n = Regex_Search(pPat, 0, pString, TRUE, &nStart, &nEnd);

	CPPUNIT_ASSERT(n == 7);
	CPPUNIT_ASSERT(nStart == 7);
	CPPUNIT_ASSERT(nEnd==19);
}

void TEST_CLASS_NAME::ConvUTF8PosToUCSPosTest1() {
	char *pUTF = ConvUCS2ToUTF8(_L("abcdefg"));
	DWORD n = ConvUTF8PosToUCSPos(pUTF, 3);
	CPPUNIT_ASSERT(n == 3);
}

void TEST_CLASS_NAME::ConvUTF8PosToUCSPosTest2() {
	char *pUTF = ConvUCS2ToUTF8(_L("abƒÎƒÓƒÆcdefg"));
	DWORD n = ConvUTF8PosToUCSPos(pUTF, 6);
	CPPUNIT_ASSERT(n == 4);
}

void TEST_CLASS_NAME::ConvUTF8PosToUCSPosTest3() {
	char *pUTF = ConvUCS2ToUTF8(_L("abƒÎƒÆŠ¿aŽšcdefg"));
	DWORD n = ConvUTF8PosToUCSPos(pUTF, 10);
	CPPUNIT_ASSERT(n == 6);
}

void TEST_CLASS_NAME::ConvUTF8PosToUCSPosTest4() {
	char *pUTF = ConvUCS2ToUTF8(_L("abƒÎƒÆŠ¿aŽšcdefg"));
	DWORD n = ConvUTF8PosToUCSPos(pUTF, 18);
	CPPUNIT_ASSERT(n == 12);
}

void TEST_CLASS_NAME::ConvUCSPosToUTF8PosTest1() {
	char *pUTF = ConvUCS2ToUTF8(_L("abcdefg"));
	DWORD n = ConvUCSPosToUTF8Pos(pUTF, 3);
	CPPUNIT_ASSERT(n == 3);
}

void TEST_CLASS_NAME::ConvUCSPosToUTF8PosTest2() {
	char *pUTF = ConvUCS2ToUTF8(_L("abƒÎƒÓƒÆcdefg"));
	DWORD n = ConvUCSPosToUTF8Pos(pUTF, 4);
	CPPUNIT_ASSERT(n == 6);
}

void TEST_CLASS_NAME::ConvUCSPosToUTF8PosTest3() {
	char *pUTF = ConvUCS2ToUTF8(_L("abƒÎƒÆŠ¿aŽšcdefg"));
	DWORD n = ConvUCSPosToUTF8Pos(pUTF, 6);
	CPPUNIT_ASSERT(n == 10);
}

void TEST_CLASS_NAME::ConvUCSPosToUTF8PosTest4() {
	char *pUTF = ConvUCS2ToUTF8(_L("abƒÎƒÆŠ¿aŽšcdefg"));
	DWORD n = ConvUCSPosToUTF8Pos(pUTF, 12);
	CPPUNIT_ASSERT(n == 18);
}

void TEST_CLASS_NAME::UnicodePosToMBCSPosTest1() {
	const char *p = "abcŠ¿Žšdef¶ghi";
	DWORD n = UnicodePosToMBCSPos(p, 9);
	CPPUNIT_ASSERT(n == 11);
}

void TEST_CLASS_NAME::MBCSPosToUnicodePosTest1() {
	const char *p = "abcŠ¿Žšdef¶ghi";
	DWORD n = MBCSPosToUnicodePos(p, 11);
	CPPUNIT_ASSERT(n == 9);
}
