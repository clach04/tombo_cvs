#include <windows.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>

#include "VarBuffer.h"
#include "LineWrapper.h"

//////////////////////////////////////////////
// Test fixture
//////////////////////////////////////////////

class LineWrapperTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(LineWrapperTest);
	CPPUNIT_TEST(SimpleWrapperTest);
	CPPUNIT_TEST(FixedLetterWrapperTest1);
	CPPUNIT_TEST(FixedLetterWrapperTest2);
	CPPUNIT_TEST_SUITE_END();

public:
	LineWrapperTest() {}
	~LineWrapperTest() {}

	virtual void setUp() {}
	virtual void tearDown() {}

	void SimpleWrapperTest();
	void FixedLetterWrapperTest1();
	void FixedLetterWrapperTest2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(LineWrapperTest);

//////////////////////////////////////////////
// Test case
//////////////////////////////////////////////

void LineWrapperTest::SimpleWrapperTest() {
	LineWrapper *pWrap = new SimpleLineWrapper();

	TVector<DWORD> vSep;
	CPPUNIT_ASSERT(vSep.Init(10, 10));

	DWORD n = 0;
	vSep.Add(&n);

	LPCTSTR p = "Hello world";
	BOOL b = pWrap->GetNewMarkerList(&vSep, 0, p, strlen(p));
	CPPUNIT_ASSERT(b);

	CPPUNIT_ASSERT(vSep.NumItems() == 2);
	CPPUNIT_ASSERT(*(vSep.GetUnit(1)) == strlen(p));
}

void LineWrapperTest::FixedLetterWrapperTest1() {
	LineWrapper *pWrap = new FixedLetterWrapper(5);

	TVector<DWORD> vSep;
	CPPUNIT_ASSERT(vSep.Init(10, 10));

	DWORD n = 0;
	vSep.Add(&n);

	LPCTSTR p = "01234567890123456789";

	DWORD nStart = 0;
	BOOL b = pWrap->GetNewMarkerList(&vSep, 0, p, strlen(p));
	CPPUNIT_ASSERT(b);

	CPPUNIT_ASSERT(vSep.NumItems() == 5);
	CPPUNIT_ASSERT(*(vSep.GetUnit(1)) == 5);
	CPPUNIT_ASSERT(*(vSep.GetUnit(2)) == 10);
	CPPUNIT_ASSERT(*(vSep.GetUnit(3)) == 15);
	CPPUNIT_ASSERT(*(vSep.GetUnit(4)) == 20);
}

void LineWrapperTest::FixedLetterWrapperTest2() {
	LineWrapper *pWrap = new FixedLetterWrapper(6);

	TVector<DWORD> vSep;
	CPPUNIT_ASSERT(vSep.Init(10, 10));

	DWORD n = 0;
	vSep.Add(&n);

	LPCTSTR p = "01234567890123456789";

	DWORD nStart = 0;
	BOOL b = pWrap->GetNewMarkerList(&vSep, 0, p, strlen(p));
	CPPUNIT_ASSERT(b);

	CPPUNIT_ASSERT(vSep.NumItems() == 5);
	CPPUNIT_ASSERT(*(vSep.GetUnit(1)) == 6);
	CPPUNIT_ASSERT(*(vSep.GetUnit(2)) == 12);
	CPPUNIT_ASSERT(*(vSep.GetUnit(3)) == 18);
	CPPUNIT_ASSERT(*(vSep.GetUnit(4)) == 20);
}