#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>

#include <windows.h>
#include "VarBuffer.h"
#include "MemManager.h"
#include "Region.h"
#include "PhysicalLineManager.h"

class PhysicalLineManagerTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(PhysicalLineManagerTest);
	CPPUNIT_TEST(LoadEmptyStrTest);
	CPPUNIT_TEST(LoadSingleLineTest);
	CPPUNIT_TEST(LoadTwoLineTest);
	CPPUNIT_TEST(ReplaceRegion1to1Test);
	CPPUNIT_TEST(ReplaceRegion1to2Test);
	CPPUNIT_TEST(ReplaceRegion1to3Test);
	CPPUNIT_TEST(ReplaceRegion2to1Test);
	CPPUNIT_TEST(ReplaceRegion2to2Test);
	CPPUNIT_TEST(ReplaceRegion2to3Test);
	CPPUNIT_TEST(ReplaceRegion3to1Test);
	CPPUNIT_TEST(ReplaceRegion3to2Test);
	CPPUNIT_TEST(ReplaceRegion3to3Test);

	CPPUNIT_TEST(ReplaceRegionEmpRgnToStrTest);

	CPPUNIT_TEST(ReplaceRegionRgnToEmpStrTest);
	CPPUNIT_TEST(ReplaceRegionRgnToEmpStr2Test);
	CPPUNIT_TEST(ReplaceRegionRgnToEmpStr3Test);
	CPPUNIT_TEST(ReplaceRegionRgnToEmpStr4Test);
	CPPUNIT_TEST(ReplaceRegionRgnToEmpStr5Test);
	CPPUNIT_TEST(ReplaceRegionRgnToEmpStr6Test);

	CPPUNIT_TEST_SUITE_END();

public:
	PhysicalLineManagerTest() {}
	~PhysicalLineManagerTest() {}

	virtual void setUp() {}
	virtual void tearDown() {}

	void LoadEmptyStrTest();
	void LoadSingleLineTest();
	void LoadTwoLineTest();
	void ReplaceRegion1to1Test();
	void ReplaceRegion1to2Test();
	void ReplaceRegion1to3Test();
	void ReplaceRegion2to1Test();
	void ReplaceRegion2to2Test();
	void ReplaceRegion2to3Test();
	void ReplaceRegion3to1Test();
	void ReplaceRegion3to2Test();
	void ReplaceRegion3to3Test();

	void ReplaceRegionEmpRgnToStrTest();

	void ReplaceRegionRgnToEmpStrTest();
	void ReplaceRegionRgnToEmpStr2Test();
	void ReplaceRegionRgnToEmpStr3Test();
	void ReplaceRegionRgnToEmpStr4Test();
	void ReplaceRegionRgnToEmpStr5Test();
	void ReplaceRegionRgnToEmpStr6Test();
};

CPPUNIT_TEST_SUITE_REGISTRATION(PhysicalLineManagerTest);

void PhysicalLineManagerTest::LoadEmptyStrTest()
{
	PhysicalLineManager mgr;
	CPPUNIT_ASSERT(mgr.Init());
	BOOL b = mgr.LoadDoc("");
	CPPUNIT_ASSERT(b);

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 1);
	MemBlock *p = mgr.aliLine.GetUnit(0)->pLine;
	CPPUNIT_ASSERT(p->nUsed == 0);
}

void PhysicalLineManagerTest::LoadSingleLineTest()
{
	PhysicalLineManager mgr;
	CPPUNIT_ASSERT(mgr.Init());

	LPCTSTR pStr = "Hello world";
	BOOL b = mgr.LoadDoc(pStr);
	CPPUNIT_ASSERT(b);

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 1);
	MemBlock *p = mgr.aliLine.GetUnit(0)->pLine;
	CPPUNIT_ASSERT(p->nUsed == strlen(pStr));
	CPPUNIT_ASSERT(strncmp(p->GetDataArea(), pStr, strlen(pStr)) == 0);
}

void PhysicalLineManagerTest::LoadTwoLineTest()
{
	PhysicalLineManager mgr;
	CPPUNIT_ASSERT(mgr.Init());

	LPCTSTR pStr = "Hello\nworld!";
	BOOL b = mgr.LoadDoc(pStr);
	CPPUNIT_ASSERT(b);

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 2);
	MemBlock *p0 = mgr.aliLine.GetUnit(0)->pLine;
	MemBlock *p1 = mgr.aliLine.GetUnit(1)->pLine;
	CPPUNIT_ASSERT(p0->nUsed == 5);
	CPPUNIT_ASSERT(strncmp(p0->GetDataArea(), "Hello", 5) == 0);
	CPPUNIT_ASSERT(p1->nUsed == 6);
	CPPUNIT_ASSERT(strncmp(p1->GetDataArea(), "world!", 6) == 0);
}

void PhysicalLineManagerTest::ReplaceRegion1to1Test()
{
	PhysicalLineManager mgr;
	CPPUNIT_ASSERT(mgr.Init());
	LPCTSTR pStr = "0123456789";
	CPPUNIT_ASSERT(mgr.LoadDoc(pStr));

	Region r, nr;
	DWORD nEffeEnd;
	r.posStart.Set(3, 0);
	r.posEnd.Set(5, 0);
	BOOL b = mgr.ReplaceRegion(&r, "abc", &nEffeEnd, &nr);
	CPPUNIT_ASSERT(b);

	// RESULT:
	// 012abc56789

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 1);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(0)->pLine->GetDataArea(), "012abc56789", 11) == 0);
	CPPUNIT_ASSERT(nEffeEnd == 1);

	Region tr;
	tr.posStart.Set(3, 0);
	tr.posEnd.Set(6, 0);
	CPPUNIT_ASSERT(nr == tr);
}

void PhysicalLineManagerTest::ReplaceRegion1to2Test()
{
	PhysicalLineManager mgr;
	CPPUNIT_ASSERT(mgr.Init());
	LPCTSTR pStr = "0123456789";
	CPPUNIT_ASSERT(mgr.LoadDoc(pStr));

	Region r, nr;
	DWORD nEffeEnd;
	r.posStart.Set(3, 0);
	r.posEnd.Set(5, 0);
	BOOL b = mgr.ReplaceRegion(&r, "abc\ndef", &nEffeEnd, &nr);
	CPPUNIT_ASSERT(b);

	// RESULT:
	// 012abc
	// def56789

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 2);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(0)->pLine->GetDataArea(), "012abc", 6) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(1)->pLine->GetDataArea(), "def56789", 8) == 0);
	CPPUNIT_ASSERT(nEffeEnd == 2);

	Region tr;
	tr.posStart.Set(3, 0);
	tr.posEnd.Set(3, 1);
	CPPUNIT_ASSERT(nr == tr);
}

void PhysicalLineManagerTest::ReplaceRegion1to3Test()
{
	PhysicalLineManager mgr;
	CPPUNIT_ASSERT(mgr.Init());
	LPCTSTR pStr = "0123456789";
	CPPUNIT_ASSERT(mgr.LoadDoc(pStr));

	Region r, nr;
	DWORD nEffeEnd;
	r.posStart.Set(3, 0);
	r.posEnd.Set(5, 0);
	BOOL b = mgr.ReplaceRegion(&r, "abc\ndef\nghi", &nEffeEnd, &nr);
	CPPUNIT_ASSERT(b);

	// RESULT:
	// 012abc
	// def
	// ghi56789
	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 3);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(0)->pLine->GetDataArea(), "012abc", 6) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(1)->pLine->GetDataArea(), "def", 3) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(2)->pLine->GetDataArea(), "ghi56789", 8) == 0);
	CPPUNIT_ASSERT(nEffeEnd == 3);

	Region tr;
	tr.posStart.Set(3, 0);
	tr.posEnd.Set(3, 2);
	CPPUNIT_ASSERT(nr == tr);
}

void PhysicalLineManagerTest::ReplaceRegion2to1Test()
{
	PhysicalLineManager mgr;
	CPPUNIT_ASSERT(mgr.Init());
	LPCTSTR pStr = "01234\n56789";
	CPPUNIT_ASSERT(mgr.LoadDoc(pStr));

	Region r, nr;
	DWORD nEffeEnd;
	r.posStart.Set(3, 0);
	r.posEnd.Set(3, 1);
	BOOL b = mgr.ReplaceRegion(&r, "abc", &nEffeEnd, &nr);
	CPPUNIT_ASSERT(b);

	// RESULT:
	// 012abc89

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 1);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(0)->pLine->GetDataArea(), "012abc89", 8) == 0);
	CPPUNIT_ASSERT(nEffeEnd == 1);

	Region tr;
	tr.posStart.Set(3, 0);
	tr.posEnd.Set(6, 0);
	CPPUNIT_ASSERT(nr == tr);
}

void PhysicalLineManagerTest::ReplaceRegion2to2Test() 
{
	PhysicalLineManager mgr;
	CPPUNIT_ASSERT(mgr.Init());
	LPCTSTR pStr = "01234\n56789";
	CPPUNIT_ASSERT(mgr.LoadDoc(pStr));

	Region r, nr;
	DWORD nEffeEnd;
	r.posStart.Set(3, 0);
	r.posEnd.Set(3, 1);
	BOOL b = mgr.ReplaceRegion(&r, "abc\nde", &nEffeEnd, &nr);
	CPPUNIT_ASSERT(b);

	// RESULT:
	// 012abc
	// de89

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 2);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(0)->pLine->GetDataArea(), "012abc", 6) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(1)->pLine->GetDataArea(), "de89", 4) == 0);
	CPPUNIT_ASSERT(nEffeEnd == 2);

	Region tr;
	tr.posStart.Set(3, 0);
	tr.posEnd.Set(2, 1);
	CPPUNIT_ASSERT(nr == tr);
}

void PhysicalLineManagerTest::ReplaceRegion2to3Test()
{
	PhysicalLineManager mgr;
	CPPUNIT_ASSERT(mgr.Init());
	LPCTSTR pStr = "01234\n56789";
	CPPUNIT_ASSERT(mgr.LoadDoc(pStr));

	Region r, nr;
	DWORD nEffeEnd;
	r.posStart.Set(0, 0);
	r.posEnd.Set(2, 1);
	BOOL b = mgr.ReplaceRegion(&r, "abc\ndefg\nhij", &nEffeEnd, &nr);
	CPPUNIT_ASSERT(b);

	// RESULT:
	// abc
	// defg
	// hij789

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 3);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(0)->pLine->GetDataArea(), "abc", 3) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(1)->pLine->GetDataArea(), "defg", 4) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(2)->pLine->GetDataArea(), "hij789", 6) == 0);
	CPPUNIT_ASSERT(nEffeEnd == 3);

	Region tr;
	tr.posStart.Set(0, 0);
	tr.posEnd.Set(3, 2);
	CPPUNIT_ASSERT(nr == tr);

}


void PhysicalLineManagerTest::ReplaceRegion3to1Test()
{
	PhysicalLineManager mgr;
	CPPUNIT_ASSERT(mgr.Init());

	LPCTSTR pStr = "line1 ONE\nline2 TWO\nline3 THREE\nline4 FOUR\nline5 FIVE";
	BOOL b = mgr.LoadDoc(pStr);
	CPPUNIT_ASSERT(b);

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 5);

	Region r, nr;
	DWORD nEffeEnd;
	r.posStart.Set(6, 1);
	r.posEnd.Set(5, 3);
	b = mgr.ReplaceRegion(&r, "abc", &nEffeEnd, &nr);

	// RESULT:
	// line1 ONE
	// line2 abc FOUR
	// line5 FIVE

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 3);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(0)->pLine->GetDataArea(), "line1 ONE", 9) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(1)->pLine->GetDataArea(), "line2 abc FOUR", 14) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(2)->pLine->GetDataArea(), "line5 FIVE", 10) == 0);
	CPPUNIT_ASSERT(nEffeEnd == 1);

	Region tr;
	tr.posStart.Set(6, 1);
	tr.posEnd.Set(9, 1);
	CPPUNIT_ASSERT(nr == tr);
}

void PhysicalLineManagerTest::ReplaceRegion3to2Test()
{
	PhysicalLineManager mgr;
	CPPUNIT_ASSERT(mgr.Init());

	LPCTSTR pStr = "line1 ONE\nline2 TWO\nline3 THREE\nline4 FOUR\nline5 FIVE";
	BOOL b = mgr.LoadDoc(pStr);
	CPPUNIT_ASSERT(b);

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 5);

	Region r, nr;
	DWORD nEffeEnd;
	r.posStart.Set(6, 1);
	r.posEnd.Set(5, 3);
	b = mgr.ReplaceRegion(&r, "abc\ndef", &nEffeEnd, &nr);

	// RESULT:
	// line1 ONE
	// line2 abc
	// def FOUR
	// line5 FIVE

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 4);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(0)->pLine->GetDataArea(), "line1 ONE", 9) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(1)->pLine->GetDataArea(), "line2 abc", 9) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(2)->pLine->GetDataArea(), "def FOUR", 8) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(3)->pLine->GetDataArea(), "line5 FIVE", 10) == 0);
	CPPUNIT_ASSERT(nEffeEnd == 2);

	Region tr;
	tr.posStart.Set(6, 1);
	tr.posEnd.Set(3, 2);
	CPPUNIT_ASSERT(nr == tr);
}

void PhysicalLineManagerTest::ReplaceRegion3to3Test()
{
	PhysicalLineManager mgr;
	CPPUNIT_ASSERT(mgr.Init());

	LPCTSTR pStr = "line1 ONE\nline2 TWO\nline3 THREE\nline4 FOUR\nline5 FIVE";
	BOOL b = mgr.LoadDoc(pStr);
	CPPUNIT_ASSERT(b);

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 5);

	Region r, nr;
	DWORD nEffeEnd;
	r.posStart.Set(6, 1);
	r.posEnd.Set(5, 3);
	b = mgr.ReplaceRegion(&r, "abc\ndef\nghi", &nEffeEnd, &nr);

	// RESULT:
	// line1 ONE
	// line2 abc
	// def
	// ghi FOUR
	// line5 FIVE

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 5);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(0)->pLine->GetDataArea(), "line1 ONE", 9) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(1)->pLine->GetDataArea(), "line2 abc", 9) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(2)->pLine->GetDataArea(), "def", 3) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(3)->pLine->GetDataArea(), "ghi FOUR", 8) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(4)->pLine->GetDataArea(), "line5 FIVE", 10) == 0);
	CPPUNIT_ASSERT(nEffeEnd == 3);

	Region tr;
	tr.posStart.Set(6, 1);
	tr.posEnd.Set(3, 3);
	CPPUNIT_ASSERT(nr == tr);
}

void PhysicalLineManagerTest::ReplaceRegionEmpRgnToStrTest()
{
	PhysicalLineManager mgr;
	CPPUNIT_ASSERT(mgr.Init());

	LPCTSTR pStr = "line1 ONE\nline2 TWO\nline3 THREE\nline4 FOUR\nline5 FIVE";
	BOOL b = mgr.LoadDoc(pStr);
	CPPUNIT_ASSERT(b);

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 5);

	Region r, nr;
	DWORD nEffeEnd;
	r.posStart.Set(5, 1);
	r.posEnd.Set(5, 1);
	b = mgr.ReplaceRegion(&r, "string", &nEffeEnd, &nr);

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 5);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(0)->pLine->GetDataArea(), "line1 ONE", 9) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(1)->pLine->GetDataArea(), "line2string TWO", 14) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(2)->pLine->GetDataArea(), "line3 THREE", 11) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(3)->pLine->GetDataArea(), "line4 FOUR", 10) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(4)->pLine->GetDataArea(), "line5 FIVE", 10) == 0);
	CPPUNIT_ASSERT(nEffeEnd == 1);

	Region tr;
	tr.posStart.Set(5, 1);
	tr.posEnd.Set(11, 1);
	CPPUNIT_ASSERT(nr == tr);
}

void PhysicalLineManagerTest::ReplaceRegionRgnToEmpStrTest()
{
	PhysicalLineManager mgr;
	CPPUNIT_ASSERT(mgr.Init());

	LPCTSTR pStr = "line1 ONE\nline2 TWO\nline3 THREE\nline4 FOUR\nline5 FIVE";
	BOOL b = mgr.LoadDoc(pStr);
	CPPUNIT_ASSERT(b);

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 5);

	Region r, nr;
	DWORD nEffeEnd;
	r.posStart.Set(0, 1);
	r.posEnd.Set(0, 3);
	b = mgr.ReplaceRegion(&r, "", &nEffeEnd, &nr);

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 3);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(0)->pLine->GetDataArea(), "line1 ONE", 9) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(1)->pLine->GetDataArea(), "line4 FOUR", 10) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(2)->pLine->GetDataArea(), "line5 FIVE", 10) == 0);
	CPPUNIT_ASSERT(nEffeEnd == 1);

	Region tr;
	tr.posStart.Set(0, 1);
	tr.posEnd.Set(0, 1);
	CPPUNIT_ASSERT(nr == tr);
}

void PhysicalLineManagerTest::ReplaceRegionRgnToEmpStr2Test()
{
	PhysicalLineManager mgr;
	CPPUNIT_ASSERT(mgr.Init());

	LPCTSTR pStr = "line1 ONE\nline2 TWO\nline3 THREE\nline4 FOUR\nline5 FIVE";
	BOOL b = mgr.LoadDoc(pStr);
	CPPUNIT_ASSERT(b);


	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 5);

	// line1 ONE
	// line2[ TWO
	// line3 THREE]
	// line4 FOUR
	// line5 FIVE

	Region r, nr;
	DWORD nEffeEnd;
	r.posStart.Set(5, 1);
	r.posEnd.Set(11, 2);
	b = mgr.ReplaceRegion(&r, "", &nEffeEnd, &nr);

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 4);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(0)->pLine->GetDataArea(), "line1 ONE", 9) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(1)->pLine->GetDataArea(), "line2", 5) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(2)->pLine->GetDataArea(), "line4 FOUR", 10) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(3)->pLine->GetDataArea(), "line5 FIVE", 10) == 0);
	CPPUNIT_ASSERT(nEffeEnd == 1);

	// line1 ONE
	// line2
	// line4 FOUR
	// line5 FIVE

	Region tr;
	tr.posStart.Set(5, 1);
	tr.posEnd.Set(5, 1);
	CPPUNIT_ASSERT(nr == tr);
}

void PhysicalLineManagerTest::ReplaceRegionRgnToEmpStr3Test()
{
	PhysicalLineManager mgr;
	CPPUNIT_ASSERT(mgr.Init());

	LPCTSTR pStr = "line1 ONE\nline2 TWO\nline3 THREE\nline4 FOUR\nline5 FIVE";
	BOOL b = mgr.LoadDoc(pStr);
	CPPUNIT_ASSERT(b);

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 5);

	Region r, nr;
	DWORD nEffeEnd;
	r.posStart.Set(0, 2);
	r.posEnd.Set(5, 3);
	b = mgr.ReplaceRegion(&r, "", &nEffeEnd, &nr);

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 4);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(0)->pLine->GetDataArea(), "line1 ONE", 9) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(1)->pLine->GetDataArea(), "line2 TWO", 9) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(2)->pLine->GetDataArea(), " FOUR", 5) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(3)->pLine->GetDataArea(), "line5 FIVE", 10) == 0);
	CPPUNIT_ASSERT(nEffeEnd == 1);

	Region tr;
	tr.posStart.Set(0, 2);
	tr.posEnd.Set(0, 2);
	CPPUNIT_ASSERT(nr == tr);
}

void PhysicalLineManagerTest::ReplaceRegionRgnToEmpStr4Test()
{
	PhysicalLineManager mgr;
	CPPUNIT_ASSERT(mgr.Init());

	LPCTSTR pStr = "line1 ONE\nline2 TWO\nline3 THREE\nline4 FOUR\nline5 FIVE";
	BOOL b = mgr.LoadDoc(pStr);
	CPPUNIT_ASSERT(b);

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 5);

	// line1 ONE
	// line2 TWO
	// [line3 THREE]
	// line4 FOUR
	// line5 FIVE

	Region r, nr;
	DWORD nEffeEnd;
	r.posStart.Set(0, 2);
	r.posEnd.Set(11, 2);
	b = mgr.ReplaceRegion(&r, "", &nEffeEnd, &nr);

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 5);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(0)->pLine->GetDataArea(), "line1 ONE", 9) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(1)->pLine->GetDataArea(), "line2 TWO", 9) == 0);
	CPPUNIT_ASSERT(mgr.aliLine.GetUnit(2)->pLine->nUsed == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(3)->pLine->GetDataArea(), "line4 FOUR", 10) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(4)->pLine->GetDataArea(), "line5 FIVE", 10) == 0);

	CPPUNIT_ASSERT(nEffeEnd == 1);

	Region tr;
	tr.posStart.Set(0, 2);
	tr.posEnd.Set(0, 2);
	CPPUNIT_ASSERT(nr == tr);
}

void PhysicalLineManagerTest::ReplaceRegionRgnToEmpStr5Test()
{
	PhysicalLineManager mgr;
	CPPUNIT_ASSERT(mgr.Init());

	LPCTSTR pStr = "line1 ONE\nline2 TWO\nline3 THREE\nline4 FOUR\nline5 FIVE";
	BOOL b = mgr.LoadDoc(pStr);
	CPPUNIT_ASSERT(b);

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 5);

	// line1 ONE
	// line2 TWO
	// [line3 THREE
	// ]line4 FOUR
	// line5 FIVE

	Region r, nr;
	DWORD nEffeEnd;
	r.posStart.Set(0, 2);
	r.posEnd.Set(0, 3);
	b = mgr.ReplaceRegion(&r, "", &nEffeEnd, &nr);

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 4);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(0)->pLine->GetDataArea(), "line1 ONE", 9) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(1)->pLine->GetDataArea(), "line2 TWO", 9) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(2)->pLine->GetDataArea(), "line4 FOUR", 10) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(3)->pLine->GetDataArea(), "line5 FIVE", 10) == 0);

	CPPUNIT_ASSERT(nEffeEnd == 1);

	Region tr;
	tr.posStart.Set(0, 2);
	tr.posEnd.Set(0, 2);
	CPPUNIT_ASSERT(nr == tr);
}

void PhysicalLineManagerTest::ReplaceRegionRgnToEmpStr6Test()
{
	PhysicalLineManager mgr;
	CPPUNIT_ASSERT(mgr.Init());

	LPCTSTR pStr = "line1 ONE\nline2 TWO\nline3 THREE\nline4 FOUR\nline5 FIVE";
	BOOL b = mgr.LoadDoc(pStr);
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 5);

	// line1 ONE
	// line2 TWO[
	// line3 THREE
	// line4 FOUR]
	// line5 FIVE

	Region r, nr;
	DWORD nEffeEnd;
	r.posStart.Set(9, 1);
	r.posEnd.Set(10, 3);
	b = mgr.ReplaceRegion(&r, "", &nEffeEnd, &nr);

	CPPUNIT_ASSERT(mgr.aliLine.NumItems() == 3);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(0)->pLine->GetDataArea(), "line1 ONE", 9) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(1)->pLine->GetDataArea(), "line2 TWO", 9) == 0);
	CPPUNIT_ASSERT(strncmp(mgr.aliLine.GetUnit(2)->pLine->GetDataArea(), "line5 FIVE", 10) == 0);

	// line1 ONE
	// line2 TWO
	// line5 FIVE

	CPPUNIT_ASSERT(nEffeEnd == 1);

	Region tr;
	tr.posStart.Set(9, 1);
	tr.posEnd.Set(9, 1);
	CPPUNIT_ASSERT(nr == tr);
}