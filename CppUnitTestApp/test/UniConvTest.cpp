#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>

#include <windows.h>
#include "UniConv.h"

#define TEST_CLASS_NAME UniConvTest

class TEST_CLASS_NAME : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(TEST_CLASS_NAME);
	CPPUNIT_TEST(Base64EncodeTest1);
	CPPUNIT_TEST(Base64EncodeTest2);
	CPPUNIT_TEST(Base64EncodeTest3);
	CPPUNIT_TEST(Base64EncodeTest4_1);
	CPPUNIT_TEST(Base64EncodeTest4_2);
	CPPUNIT_TEST(Base64EncodeTest4_3);
	CPPUNIT_TEST(Base64EncodeTest4_4);
	CPPUNIT_TEST(Base64EncodeTest4_5);
	CPPUNIT_TEST(Base64EncodeTest4_6);
	CPPUNIT_TEST(Base64EncodeTest4_7);

	CPPUNIT_TEST(Base64DecodeTest1);
	CPPUNIT_TEST(Base64DecodeTest2);

	CPPUNIT_TEST_SUITE_END();

public:
	TEST_CLASS_NAME() {}
	~TEST_CLASS_NAME() {}

	virtual void setUp() {}
	virtual void tearDown() {}

	void Base64EncodeTest1();
	void Base64EncodeTest2();
	void Base64EncodeTest3();
	void Base64EncodeTest4_1();
	void Base64EncodeTest4_2();
	void Base64EncodeTest4_3();
	void Base64EncodeTest4_4();
	void Base64EncodeTest4_5();
	void Base64EncodeTest4_6();
	void Base64EncodeTest4_7();

	void Base64DecodeTest1();
	void Base64DecodeTest2();

};

CPPUNIT_TEST_SUITE_REGISTRATION(TEST_CLASS_NAME);

// test probe
extern DWORD g_Base64EncodeAllocSize;

////////////////////////////////////////////////

void TEST_CLASS_NAME::Base64EncodeTest1() {
	char *pOut = Base64Encode((LPBYTE)"Hello", 5);

	CPPUNIT_ASSERT(g_Base64EncodeAllocSize == 9);
	CPPUNIT_ASSERT(strcmp("SGVsbG8=", pOut) == 0);
	delete [] pOut;
}

void TEST_CLASS_NAME::Base64EncodeTest2() {
	char *pOut = Base64Encode(NULL, 5);
	CPPUNIT_ASSERT(pOut == NULL);
}

void TEST_CLASS_NAME::Base64EncodeTest3() {
	char *pOut = Base64Encode((LPBYTE)"Hello", 0);
	CPPUNIT_ASSERT(pOut == NULL);
}

void TEST_CLASS_NAME::Base64EncodeTest4_1()
{
	char *pOut = Base64Encode((LPBYTE)"Hello ", 1);

	CPPUNIT_ASSERT(g_Base64EncodeAllocSize == 5);
	CPPUNIT_ASSERT(strcmp("SA==", pOut) == 0);
	delete [] pOut;
}

void TEST_CLASS_NAME::Base64EncodeTest4_2()
{
	char *pOut = Base64Encode((LPBYTE)"Hello ", 2);

	CPPUNIT_ASSERT(g_Base64EncodeAllocSize == 5);
	CPPUNIT_ASSERT(strcmp("SGU=", pOut) == 0);
	delete [] pOut;
}

void TEST_CLASS_NAME::Base64EncodeTest4_3()
{
	char *pOut = Base64Encode((LPBYTE)"Hello ", 3);

	CPPUNIT_ASSERT(g_Base64EncodeAllocSize == 5);
	CPPUNIT_ASSERT(strcmp("SGVs", pOut) == 0);
	delete [] pOut;
}

void TEST_CLASS_NAME::Base64EncodeTest4_4()
{
	char *pOut = Base64Encode((LPBYTE)"Hello ", 4);

	CPPUNIT_ASSERT(g_Base64EncodeAllocSize == 9);
	CPPUNIT_ASSERT(strcmp("SGVsbA==", pOut) == 0);
	delete [] pOut;
}

void TEST_CLASS_NAME::Base64EncodeTest4_5()
{
	char *pOut = Base64Encode((LPBYTE)"Hello ", 5);

	CPPUNIT_ASSERT(g_Base64EncodeAllocSize == 9);
	CPPUNIT_ASSERT(strcmp("SGVsbG8=", pOut) == 0);
	delete [] pOut;
}

void TEST_CLASS_NAME::Base64EncodeTest4_6()
{
	char *pOut = Base64Encode((LPBYTE)"Hello ", 6);

	CPPUNIT_ASSERT(g_Base64EncodeAllocSize == 9);
	CPPUNIT_ASSERT(strcmp("SGVsbG8g", pOut) == 0);
	delete [] pOut;
}

void TEST_CLASS_NAME::Base64EncodeTest4_7()
{
	char *pOut = Base64Encode((LPBYTE)"Hello w", 7);

	CPPUNIT_ASSERT(g_Base64EncodeAllocSize == 13);
	CPPUNIT_ASSERT(strcmp("SGVsbG8gdw==", pOut) == 0);
	delete [] pOut;
}

void TEST_CLASS_NAME::Base64DecodeTest1()
{
	DWORD n;
	LPBYTE p = Base64Decode("SGVsbG8=", &n);
	CPPUNIT_ASSERT(n == 5);
	CPPUNIT_ASSERT(strncmp((char*)p, "Hello", 5) == 0);
	delete [] p;
}

void TEST_CLASS_NAME::Base64DecodeTest2()
{
	DWORD n;
	LPBYTE p = Base64Decode("SGVsbG8", &n);
	CPPUNIT_ASSERT(p == NULL);
}