#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>

#include <windows.h>
#include "VarBuffer.h"
#include "StringSplitter.h"

class StringSplitterTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(StringSplitterTest);
	CPPUNIT_TEST(EmptyTest);
	CPPUNIT_TEST(SingleLineTest);
	CPPUNIT_TEST(CrLfTest);
	CPPUNIT_TEST(CrTest);
	CPPUNIT_TEST(LfTest);
	CPPUNIT_TEST(ScrTest);
	CPPUNIT_TEST(CrSTest);
	CPPUNIT_TEST(MultiLineEOFTest);
	CPPUNIT_TEST(MultiLineCRTest);
	CPPUNIT_TEST_SUITE_END();

public:
	StringSplitterTest() {}
	~StringSplitterTest() {}

	virtual void setUp() {}
	virtual void tearDown() {}

	void EmptyTest();
	void SingleLineTest();
	void CrLfTest();
	void CrTest();
	void LfTest();
	void ScrTest();
	void CrSTest();
	void MultiLineEOFTest();
	void MultiLineCRTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(StringSplitterTest);

void StringSplitterTest::EmptyTest()
{
	StringSplitter ss;
	CPPUNIT_ASSERT(ss.Init());
	CPPUNIT_ASSERT(ss.Parse(""));
	CPPUNIT_ASSERT(ss.MaxLine() == 1);
	CPPUNIT_ASSERT(ss.GetBegin(0) == 0);
	CPPUNIT_ASSERT(ss.GetEnd(0) == 0);
}

void StringSplitterTest::SingleLineTest()
{
	StringSplitter ss;
	CPPUNIT_ASSERT(ss.Init());
	CPPUNIT_ASSERT(ss.Parse("0123456"));
	CPPUNIT_ASSERT(ss.MaxLine() == 1);
	CPPUNIT_ASSERT(ss.GetBegin(0) == 0);
	CPPUNIT_ASSERT(ss.GetEnd(0) == 7);
}

void StringSplitterTest::CrLfTest()
{
	StringSplitter ss;
	CPPUNIT_ASSERT(ss.Init());
	BOOL b = ss.Parse("\r\n");
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(ss.MaxLine() == 2);
	CPPUNIT_ASSERT(ss.GetBegin(0) == 0);
	CPPUNIT_ASSERT(ss.GetEnd(0) == 0);
	CPPUNIT_ASSERT(ss.GetBegin(1) == 2);
	CPPUNIT_ASSERT(ss.GetEnd(1) == 2);
}

void StringSplitterTest::CrTest()
{
	StringSplitter ss;
	CPPUNIT_ASSERT(ss.Init());
	BOOL b = ss.Parse("\r");
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(ss.MaxLine() == 2);
	CPPUNIT_ASSERT(ss.GetBegin(0) == 0);
	CPPUNIT_ASSERT(ss.GetEnd(0) == 0);
	CPPUNIT_ASSERT(ss.GetBegin(1) == 1);
	CPPUNIT_ASSERT(ss.GetEnd(1) == 1);
}

void StringSplitterTest::LfTest()
{
	StringSplitter ss;
	CPPUNIT_ASSERT(ss.Init());
	BOOL b = ss.Parse("\n");
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(ss.MaxLine() == 2);
	CPPUNIT_ASSERT(ss.GetBegin(0) == 0);
	CPPUNIT_ASSERT(ss.GetEnd(0) == 0);
	CPPUNIT_ASSERT(ss.GetBegin(1) == 1);
	CPPUNIT_ASSERT(ss.GetEnd(1) == 1);
}

void StringSplitterTest::ScrTest()
{
	StringSplitter ss;
	CPPUNIT_ASSERT(ss.Init());
	BOOL b = ss.Parse("abc\n");
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(ss.MaxLine() == 2);
	CPPUNIT_ASSERT(ss.GetBegin(0) == 0);
	CPPUNIT_ASSERT(ss.GetEnd(0) == 3);
	CPPUNIT_ASSERT(ss.GetBegin(1) == 4);
	CPPUNIT_ASSERT(ss.GetEnd(1) == 4);
}

void StringSplitterTest::CrSTest()
{
	StringSplitter ss;
	CPPUNIT_ASSERT(ss.Init());
	BOOL b = ss.Parse("\nabc");
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(ss.MaxLine() == 2);
	CPPUNIT_ASSERT(ss.GetBegin(0) == 0);
	CPPUNIT_ASSERT(ss.GetEnd(0) == 0);
	CPPUNIT_ASSERT(ss.GetBegin(1) == 1);
	CPPUNIT_ASSERT(ss.GetEnd(1) == 4);
}

void StringSplitterTest::MultiLineEOFTest()
{
	StringSplitter ss;
	CPPUNIT_ASSERT(ss.Init());
	BOOL b = ss.Parse("abc\ndefg\nghi");
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(ss.MaxLine() == 3);
	CPPUNIT_ASSERT(ss.GetBegin(0) == 0);
	CPPUNIT_ASSERT(ss.GetEnd(0) == 3);
	CPPUNIT_ASSERT(ss.GetBegin(1) == 4);
	CPPUNIT_ASSERT(ss.GetEnd(1) == 8);
	CPPUNIT_ASSERT(ss.GetBegin(2) == 9);
	CPPUNIT_ASSERT(ss.GetEnd(2) == 12);
}

void StringSplitterTest::MultiLineCRTest()
{
	StringSplitter ss;
	CPPUNIT_ASSERT(ss.Init());
	BOOL b = ss.Parse("abc\ndefg\nghi\n");
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(ss.MaxLine() == 4);
	CPPUNIT_ASSERT(ss.GetBegin(0) == 0);
	CPPUNIT_ASSERT(ss.GetEnd(0) == 3);
	CPPUNIT_ASSERT(ss.GetBegin(1) == 4);
	CPPUNIT_ASSERT(ss.GetEnd(1) == 8);
	CPPUNIT_ASSERT(ss.GetBegin(2) == 9);
	CPPUNIT_ASSERT(ss.GetEnd(2) == 12);
	CPPUNIT_ASSERT(ss.GetBegin(3) == 13);
	CPPUNIT_ASSERT(ss.GetEnd(3) == 13);
}