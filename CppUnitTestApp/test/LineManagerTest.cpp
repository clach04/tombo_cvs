#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>

#include <windows.h>
#include "VarBuffer.h"
#include "MemManager.h"
#include "Region.h"
#include "PhysicalLineManager.h"
#include "LineWrapper.h"
#include "LineManager.h"

struct LineVector {
	DWORD nPos;
	DWORD nPh;
};

class LineManagerTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(LineManagerTest);
	CPPUNIT_TEST(AssocTest);
	CPPUNIT_TEST(AssocTest2);
	CPPUNIT_TEST(GetDeletedLgLinesTest1);
	CPPUNIT_TEST(GetDeletedLgLinesTest2);

	CPPUNIT_TEST(AdjustLgLines1);
	CPPUNIT_TEST(AdjustLgLines2);
	CPPUNIT_TEST(AdjustLgLines3);
	CPPUNIT_TEST(AdjustLgLines4);
	CPPUNIT_TEST(AdjustLgLines5);
	CPPUNIT_TEST(AdjustLgLines6);
	CPPUNIT_TEST(AdjustLgLines7);
	CPPUNIT_TEST(AdjustLgLines8);
	CPPUNIT_TEST(AdjustLgLines9);
	CPPUNIT_TEST(AdjustLgLines10);
	CPPUNIT_TEST_SUITE_END();

	LineWrapper *pWrapper;
public:
	LineManagerTest() {}
	~LineManagerTest() {}

	virtual void setUp();
	virtual void tearDown();

	void AssocTest();
	void AssocTest2();
	void GetDeletedLgLinesTest1();
	void GetDeletedLgLinesTest2();
	void AdjustLgLines1();
	void AdjustLgLines2();
	void AdjustLgLines3();
	void AdjustLgLines4();
	void AdjustLgLines5();
	void AdjustLgLines6();
	void AdjustLgLines7();
	void AdjustLgLines8();
	void AdjustLgLines9();
	void AdjustLgLines10();

	void CheckLgLine(LineManager *pMgr, DWORD nLineNo, DWORD nPh, DWORD nPos);
	void CheckAdjustLgLines(
		LineWrapper *p, 
		LPCTSTR pSrc, LPCTSTR pRepl, Region *pRegion, 
		DWORD nBefLines, DWORD nAftLines, 
		Coordinate &cLgStartCorrect, DWORD nAffeLines,
		LineVector *v);
};

void LineManagerTest::setUp()
{
	pWrapper = new FixedLetterWrapper(8); 
}

void LineManagerTest::tearDown()
{
	delete pWrapper;
}

void LineManagerTest::CheckLgLine(LineManager *pMgr, DWORD nLineNo, DWORD nPh, DWORD nPos)
{
	char buf[1024];
	sprintf(buf, "line = %d phline = %d pos = %d", nLineNo, nPh, nPos);

	LgLineInfo *pli;
	pli = pMgr->aliLine.GetUnit(nLineNo);
	CPPUNIT_ASSERT_MESSAGE(buf, pli->nPhyLineNo == nPh);
	CPPUNIT_ASSERT_MESSAGE(buf, pli->nLinePos == nPos);
}

void LineManagerTest::CheckAdjustLgLines(
	LineWrapper *p, 
	LPCTSTR pSrc, LPCTSTR pRepl, Region *pRegion, 
	DWORD nBefLines, DWORD nAftLines, 
	Coordinate &cLgStartCorrect, DWORD nAffeLines,
	LineVector *v)
{
	PhysicalLineManager mgr;
	CPPUNIT_ASSERT(mgr.Init());
	LPCTSTR pStr = pSrc;

	BOOL b = mgr.LoadDoc(pStr);
	CPPUNIT_ASSERT(b);

	LineManager lmgr;
	CPPUNIT_ASSERT(lmgr.Init(NULL));

	b = lmgr.Assoc(&mgr, pWrapper);
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(lmgr.MaxLine() == nBefLines);

	DWORD nPhLinesBefore = mgr.MaxLine();

	DWORD nNewEnd;
	b = mgr.ReplaceRegion(pRegion, pRepl, &nNewEnd, NULL);
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT_MESSAGE("invalid nAffeLines", nNewEnd == nAffeLines);

	DWORD nPhLinesAfter = mgr.MaxLine();

	Coordinate cLgStart;
	DWORD nAffLgLines;
	b = lmgr.AdjustLgLines(&mgr, pWrapper, *pRegion, nPhLinesBefore, nPhLinesAfter, nNewEnd, &cLgStart, &nAffLgLines);
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(cLgStart == cLgStartCorrect);

	CPPUNIT_ASSERT_MESSAGE("invalid nAftLines", lmgr.MaxLine() == nAftLines);

	for (DWORD i = 0; i < nAftLines; i++) {
		CheckLgLine(&lmgr, i, v[i].nPh, v[i].nPos);
		i++;
	}
}


CPPUNIT_TEST_SUITE_REGISTRATION(LineManagerTest);

void LineManagerTest::AssocTest() {
	PhysicalLineManager mgr;
	CPPUNIT_ASSERT(mgr.Init());

	LPCTSTR pStr = "line1 ONE\nline2 TWO\nline3 THREE\nline4 FOUR\nline5 FIVE";
	BOOL b = mgr.LoadDoc(pStr);
	CPPUNIT_ASSERT(b);

	LineManager lmgr;
	CPPUNIT_ASSERT(lmgr.Init(NULL));

	LineWrapper *pWrapper = new SimpleLineWrapper();

	b = lmgr.Assoc(&mgr, pWrapper);
	CPPUNIT_ASSERT(b);

	CPPUNIT_ASSERT(lmgr.MaxLine() == 5);

	CheckLgLine(&lmgr, 0, 0, 0);
	CheckLgLine(&lmgr, 1, 1, 0);
	CheckLgLine(&lmgr, 2, 2, 0);
	CheckLgLine(&lmgr, 3, 3, 0);
	CheckLgLine(&lmgr, 4, 4, 0);
}

void LineManagerTest::AssocTest2()
{
	PhysicalLineManager mgr;
	CPPUNIT_ASSERT(mgr.Init());

	LPCTSTR pStr = "1abcdefghij\n2klmno\n3pqrstuvwxyz\n\n5AB";

	BOOL b = mgr.LoadDoc(pStr);
	CPPUNIT_ASSERT(b);

	LineManager lmgr;
	CPPUNIT_ASSERT(lmgr.Init(NULL));

	LineWrapper *pWrapper = new FixedLetterWrapper(4);

	b = lmgr.Assoc(&mgr, pWrapper);
	CPPUNIT_ASSERT(b);

	CPPUNIT_ASSERT(lmgr.MaxLine() == 10);

	CheckLgLine(&lmgr, 0, 0, 0);
	CheckLgLine(&lmgr, 1, 0, 4);
	CheckLgLine(&lmgr, 2, 0, 8);
	CheckLgLine(&lmgr, 3, 1, 0);
	CheckLgLine(&lmgr, 4, 1, 4);
	CheckLgLine(&lmgr, 5, 2, 0);
	CheckLgLine(&lmgr, 6, 2, 4);
	CheckLgLine(&lmgr, 7, 2, 8);
	CheckLgLine(&lmgr, 8, 3, 0);
	CheckLgLine(&lmgr, 9, 4, 0);
}

void LineManagerTest::GetDeletedLgLinesTest1()
{
	PhysicalLineManager mgr;
	CPPUNIT_ASSERT(mgr.Init());

	LPCTSTR pStr = "1abcdefghij\n2klmno\n3pqrstuvwxyz\n\n5AB";
	//  1abcdefghij
	//* 2klmno
	//  3pqrstuvwxyz
	//  
	//  5AB

	BOOL b = mgr.LoadDoc(pStr);
	CPPUNIT_ASSERT(b);

	LineManager lmgr;
	CPPUNIT_ASSERT(lmgr.Init(NULL));

	b = lmgr.Assoc(&mgr, pWrapper);
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(lmgr.MaxLine() == 7);
	// [Lg][Ph]
	// [ 0][ 0]1abcdefg
	// [ 1][ 0]hij
	// [ 2][ 1]2klmno
	// [ 3][ 2]3pqrstuv
	// [ 4][ 2]wxyz
	// [ 5][ 3]
	// [ 6][ 4]5AB

	// replace "k" to "abc\ndef"
	DWORD nNewEnd;
	Region r;
	r.posStart.Set(1, 1);
	r.posEnd.Set(2, 1);
	b = mgr.ReplaceRegion(&r, "abc\ndef", &nNewEnd, NULL);
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(nNewEnd == 2);


	DWORD nAfEnd;
	Coordinate cAfStart;
	lmgr.GetDeletedLgLines(r, &cAfStart, &nAfEnd);
	CPPUNIT_ASSERT(cAfStart.row == 2);
	CPPUNIT_ASSERT(nAfEnd == 2);

	// Ph
	//  1abcdefghabcj
	//* 2abc
	//* deflmno
	//  3pqrstuvwxyz
	//
	//  5AB
}

void LineManagerTest::GetDeletedLgLinesTest2()
{
	PhysicalLineManager mgr;
	CPPUNIT_ASSERT(mgr.Init());

	LPCTSTR pStr = "1abcdefghij\n2klmno\n3pqrstuvwxyz\n\n5ab";
	//  1abcdefghij
	//  2klmno
	//  3pqrstuvwxyz
	//  
	//  5ab

	BOOL b = mgr.LoadDoc(pStr);
	CPPUNIT_ASSERT(b);

	LineManager lmgr;
	CPPUNIT_ASSERT(lmgr.Init(NULL));


	b = lmgr.Assoc(&mgr, pWrapper);
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(lmgr.MaxLine() == 7);
	// [Lg][Ph]
	// [ 0][ 0]1abcdefg
	// [ 1][ 0]hij
	// [ 2][ 1]2klmno
	// [ 3][ 2]3pqrstuv
	// [ 4][ 2]wxyz
	// [ 5][ 3]
	// [ 6][ 4]5ab

	// replace "o\n3" to "ABC\nDEF"
	DWORD nNewEnd;
	Region r;
	r.posStart.Set(5, 1);
	r.posEnd.Set(1, 2);
	b = mgr.ReplaceRegion(&r, "ABC\nDEF", &nNewEnd, NULL);
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(nNewEnd == 2);

	DWORD nAfEnd;
	Coordinate cAfStart;
	lmgr.GetDeletedLgLines(r, &cAfStart, &nAfEnd);
	CPPUNIT_ASSERT(cAfStart.row == 2);
	CPPUNIT_ASSERT(nAfEnd == 4);

	// Ph
	//  1abcdefghabcj
	//* 2klmnABC
	//* DEFpqrstuvwxyz
	//
	//  5ab

}

void LineManagerTest::AdjustLgLines1()
{
	// same lg lines replace
	LPCTSTR pStr = "1abcdefghij\n2klmno\n3pqrstuvwxyz\n\n5ab";
	LPCTSTR pRepl = "ABC\nDEF";
	Region r;
	r.posStart.Set(5, 1);
	r.posEnd.Set(1, 2);
//	Coordinate cLgStart(5, 2);
	Coordinate cLgStart(0, 2);

	LineVector v[] = {
		{0, 0},
		{8, 0},
		{0, 1},
		{0, 2},
		{8, 2},
		{0, 3},
		{0, 4}
	};
	//  1abcdefghij
	//  2klmn[o
	//  3]pqrstuvwxyz
	//  
	//  5ab


	// [Lg][Ph]
	// [ 0][ 0]1abcdefg
	// [ 1][ 0]hij
	// [ 2][ 1]2klmno
	// [ 3][ 2]3pqrstuv
	// [ 4][ 2]wxyz
	// [ 5][ 3]
	// [ 6][ 4]5ab

	// Ph
	//  1abcdefghabcj
	//* 2klmnABC
	//* DEFpqrstuvwxyz
	//
	//  5ab

	// [Lg][Ph]
	// [ 0][ 0]1abcdefg
	// [ 1][ 0]habcj
	// [ 2][ 1]2klmnABC
	// [ 3][ 2]DEFpqrst
	// [ 4][ 2]uvwxyz
	// [ 5][ 3]
	// [ 6][ 4]5ab

	CheckAdjustLgLines(pWrapper, pStr, pRepl, &r, 7, 7, cLgStart, 2, v);
}

void LineManagerTest::AdjustLgLines2()
{
	// extend lg lines

	LPCTSTR pStr = "1abcdefghij\n2klmno\n3pqrstuvwxyz\n\n5ab";
	LPCTSTR pRepl = "ABCDEF\nGHI";
	Region r;
	r.posStart.Set(5, 1);
	r.posEnd.Set(1, 2);
//	Coordinate cLgStart(5, 2);
	Coordinate cLgStart(0, 2);

	//  1abcdefghij
	//  2klmno
	//  3pqrstuvwxyz
	//  
	//  5ab

	// [Lg][Ph]
	// [ 0][ 0]1abcdefg
	// [ 1][ 0]hij
	// [ 2][ 1]2klmno
	// [ 3][ 2]3pqrstuv
	// [ 4][ 2]wxyz
	// [ 5][ 3]
	// [ 6][ 4]5ab

	// Ph
	//  1abcdefghabcj
	//* 2klmnABCDEF
	//* GHIpqrstuvwxyz
	//
	//  5ab

	// [Lg][Ph]
	// [ 0][ 0]1abcdefg
	// [ 1][ 0]habcj
	// [ 2][ 1]2klmnABC
	// [ 3][ 1]DEF
	// [ 4][ 2]GHIpqrst
	// [ 5][ 2]uvwxyz
	// [ 6][ 3]
	// [ 7][ 4]5ab

	LineVector v[] = {
		{0, 0},
		{8, 0},
		{0, 1},
		{8, 1},
		{0, 2},
		{8, 2},
		{0, 3},
		{0, 4}
	};

	CheckAdjustLgLines(pWrapper, pStr, pRepl, &r, 7, 8, cLgStart, 2, v);

}

void LineManagerTest::AdjustLgLines3()
{
	// reduce physical lines
	LPCTSTR pStr = "1abcdefghij\n2klmno\n3pqrstuvwxyz\n\n5ab";
	LPCTSTR pRepl = "";
	Region r; r.posStart.Set(2, 0); r.posEnd.Set(6, 1);
//	Coordinate cLgStart(2, 0);
	Coordinate cLgStart(0, 0);

	// [Ph]
	// [ 0]1a[bcdefghij
	// [ 1]2klmno]
	// [ 2]3pqrstuvwxyz
	// [ 3] 
	// [ 4]5ab

	// [Lg][Ph]
	// [ 0][ 0]1abcdefg
	// [ 1][ 0]hij
	// [ 2][ 1]2klmno
	// [ 3][ 2]3pqrstuv
	// [ 4][ 2]wxyz
	// [ 5][ 3]
	// [ 6][ 4]5ab

	// [ 0]1a
	// [ 1]3pqrstuvwxyz
	// [ 2]
	// [ 3]5ab

	// [Lg][Ph]
	// [ 0][ 0]1a
	// [ 1][ 1]3pqrstuv
	// [ 2][ 1]vwxyz
	// [ 3][ 2]
	// [ 4][ 3]5ab

	LineVector v[] = {
		{0, 0},
		{0, 1},
		{8, 1},
		{0, 2},
		{0, 3},
	};

	CheckAdjustLgLines(pWrapper, pStr, pRepl, &r, 7, 5, cLgStart, 1, v);

}

void LineManagerTest::AdjustLgLines4()
{
	// reduce ph lines and start position of the delete region is folded logical line.
	LPCTSTR pStr = "1abcdefghij\n2klmno\n3pqrstuvwxyz\n\n5ab";
	LPCTSTR pRepl = "";
	Region r; r.posStart.Set(9, 0); r.posEnd.Set(6, 1);
//	Coordinate cLgStart(1, 1);
	Coordinate cLgStart(0, 0);

	// [Ph]
	// [ 0]1abcdefgh[ij
	// [ 1]2klmno]
	// [ 2]3pqrstuvwxyz
	// [ 3] 
	// [ 4]5ab

	// [Lg][Ph]
	// [ 0][ 0]1abcdefg
	// [ 1][ 0]h[ij
	// [ 2][ 1]2klmno]
	// [ 3][ 2]3pqrstuv
	// [ 4][ 2]wxyz
	// [ 5][ 3]
	// [ 6][ 4]5ab

	// [ 0]1abcdefgh
	// [ 1]3pqrstuvwxyz
	// [ 2]
	// [ 3]5ab

	// [Lg][Ph]
	// [ 0][ 0]1abcdefg
	// [ 1][ 0]h
	// [ 2][ 1]3pqrstuv
	// [ 3][ 1]vwxyz
	// [ 4][ 2]
	// [ 5][ 3]5ab

	LineVector v[] = {
		{0, 0},
		{8, 0},
		{0, 1},
		{8, 1},
		{0, 2},
		{0, 3},
	};

	CheckAdjustLgLines(pWrapper, pStr, pRepl, &r, 7, 6, cLgStart, 1, v);

}

void LineManagerTest::AdjustLgLines5()
{
	// extend ph lines 

	LPCTSTR pStr = "1abcdefghij\n2klmno\n3pqrstuvwxyz\n\n5ab";
	LPCTSTR pRepl = "ABC\nDEFGHIJKL\nMNO";
	Region r; r.posStart.Set(9, 0); r.posEnd.Set(1, 1);
//	Coordinate cLgStart(1, 1);
	Coordinate cLgStart(0, 0);

	// [Ph]
	// [ 0]1abcdefgh[ij
	// [ 1]2]klmno
	// [ 2]3pqrstuvwxyz
	// [ 3] 
	// [ 4]5ab

	// [Lg][Ph]
	// [ 0][ 0]1abcdefg
	// [ 1][ 0]h[ij
	// [ 2][ 1]2]klmno
	// [ 3][ 2]3pqrstuv
	// [ 4][ 2]wxyz
	// [ 5][ 3]
	// [ 6][ 4]5ab

	// [Ph]
	// [ 0]1abcdefghABC
	// [ 1]DEFGHIJKL
	// [ 2]MNOklmno
	// [ 3]3pqrstuvwxyz
	// [ 4] 
	// [ 5]5ab

	// [Lg][Ph]
	// [ 0][ 0]1abcdefg
	// [ 1][ 0]hABC
	// [ 2][ 1]DEFGHIJK
	// [ 3][ 1]L
	// [ 4][ 2]MNOklmno
	// [ 5][ 3]3pqrstuv
	// [ 6][ 3]wxyz
	// [ 7][ 4]
	// [ 8][ 5]5ab

	LineVector v[] = {
		{0, 0},
		{8, 0},
		{0, 1},
		{8, 1},
		{0, 2},
		{0, 3},
		{8, 3},
		{0, 4},
		{0, 5}
	};

	CheckAdjustLgLines(pWrapper, pStr, pRepl, &r, 7, 9, cLgStart, 3, v);
}

void LineManagerTest::AdjustLgLines6()
{
	// replace empty region to string
	LPCTSTR pStr = "1abcdefghij\n2klmno\n3pqrstuvwxyz\n\n5ab";
	LPCTSTR pRepl = "ABC\nDEFGHIJKLMNO\nPQR";
	Region r; r.posStart.Set(6, 1); r.posEnd.Set(6, 1);
//	Coordinate cLgStart(6, 2);
	Coordinate cLgStart(0, 2);

	// [Ph]
	// [ 0]1abcdefghij
	// [ 1]2klmno[]
	// [ 2]3pqrstuvwxyz
	// [ 3] 
	// [ 4]5ab

	// [Lg][Ph]
	// [ 0][ 0]1abcdefg
	// [ 1][ 0]hij
	// [ 2][ 1]2klmno[]
	// [ 3][ 2]3pqrstuv
	// [ 4][ 2]wxyz
	// [ 5][ 3]
	// [ 6][ 4]5ab

	// [Ph]
	// [ 0]1abcdefghij
	// [ 1]2klmnoABC
	// [ 2]DEFGHIJKLMNO
	// [ 3]PQR
	// [ 4]3pqrstuvwxyz
	// [ 5] 
	// [ 6]5ab

	// [Lg][Ph]
	// [ 0][ 0]1abcdefg
	// [ 1][ 0]hij
	// [ 2][ 1]2klmnoAB
	// [ 3][ 1]C
	// [ 4][ 2]DEFGHIJK
	// [ 5][ 2]LMNO
	// [ 6][ 3]PQR
	// [ 7][ 4]3pqrstuv
	// [ 8][ 4]wxyz
	// [ 9][ 5]
	// [10][ 6]5ab

	LineVector v[] = {
		{0, 0},
		{8, 0},
		{0, 1},
		{8, 1},
		{0, 2},
		{8, 2},
		{0, 3},
		{0, 4},
		{8, 4},
		{0, 5},
		{0, 6}
	};

	CheckAdjustLgLines(pWrapper, pStr, pRepl, &r, 7, 11, cLgStart, 3, v);
}

void LineManagerTest::AdjustLgLines7()
{
	// insert more logical line
	LPCTSTR pStr = "1abcdefghij\n2klmno\n3pqrstuvwxyz\n\n5ab";
	LPCTSTR pRepl = "ABCDEFGHIJKLMNOPQRSTU";
	Region r; r.posStart.Set(6, 1); r.posEnd.Set(6, 1);
//	Coordinate cLgStart(6, 2);
	Coordinate cLgStart(0, 2);

	// [Ph]
	// [ 0]1abcdefghij
	// [ 1]2klmno[]
	// [ 2]3pqrstuvwxyz
	// [ 3] 
	// [ 4]5ab

	// [Lg][Ph]
	// [ 0][ 0]1abcdefg
	// [ 1][ 0]hij
	// [ 2][ 1]2klmno[]
	// [ 3][ 2]3pqrstuv
	// [ 4][ 2]wxyz
	// [ 5][ 3]
	// [ 6][ 4]5ab

	// [Ph]
	// [ 0]1abcdefghij
	// [ 1]2klmnoABCDEFGHIJKLMNOPQRSTU
	// [ 2]3pqrstuvwxyz
	// [ 3] 
	// [ 4]5ab

	// [Lg][Ph]
	// [ 0][ 0]1abcdefg
	// [ 1][ 0]hij
	// [ 2][ 1]2klmnoAB
	// [ 3][ 1]CDEFGHIJ
	// [ 4][ 1]KLMNOPQR
	// [ 5][ 1]STU
	// [ 6][ 2]3pqrstuv
	// [ 7][ 2]wxyz
	// [ 8][ 3]
	// [ 9][ 4]5ab

	LineVector v[] = {
		{0, 0},
		{8, 0},
		{0, 1},
		{8, 1},
		{16,1},
		{24,1},
		{0, 2},
		{8, 2},
		{0, 3},
		{0, 4},
	};

	CheckAdjustLgLines(pWrapper, pStr, pRepl, &r, 7, 10, cLgStart, 1, v);
}

void LineManagerTest::AdjustLgLines8()
{
	// insert to empty buffer
	LPCTSTR pStr = "";
	LPCTSTR pRepl = "ABC\nDEF";
	Region r; r.posStart.Set(0, 0); r.posEnd.Set(0, 0);
	Coordinate cLgStart(0, 0);

	// [Ph]
	// [ 0]ABC
	// [ 1]DEF

	// [Lg][Ph]
	// [ 0][ 0]ABC
	// [ 1][ 0]DEF

	LineVector v[] = {
		{0, 0},
		{0, 1}
	};

	CheckAdjustLgLines(pWrapper, pStr, pRepl, &r, 1, 2, cLgStart, 2, v);
}

void LineManagerTest::AdjustLgLines9()
{
	// join line
	LPCTSTR pStr = "aaaa\nbbbb\n\ndddd";
	LPCTSTR pRepl = "";
	Region r; r.posStart.Set(4, 1); r.posEnd.Set(0, 2);
//	Coordinate cLgStart(4, 1);
	Coordinate cLgStart(0, 1);

	// [Ph]
	// [ 0]aaaa
	// [ 1]bbbb[
	// [ 2]]
	// [ 3]dddd


	// [Ph]
	// [ 0]aaaa
	// [ 1]bbbb
	// [ 2]dddd

	LineVector v[] = {
		{0, 0},
		{0, 1},
		{0, 2}
	};

	CheckAdjustLgLines(pWrapper, pStr, pRepl, &r, 4, 3, cLgStart, 1, v);
}

void LineManagerTest::AdjustLgLines10()
{
	// join line
	LPCTSTR pStr = "01234567a";
	LPCTSTR pRepl = "b";
	Region r; r.posStart.Set(9, 0); r.posEnd.Set(9, 0);
//	Coordinate cLgStart(4, 1);
	Coordinate cLgStart(0, 0);

	// [Ph]
	// [ 0]01234567a[]

	// [Lg][Ph]
	// [ 0][ 0]01234567
	// [ 1][ 0]a[]

	LineVector v[] = {
		{0, 0},
		{8, 0},
	};

	// [Lg][Ph]
	// [ 0][ 0]01234567
	// [ 1][ 0]ab

	CheckAdjustLgLines(pWrapper, pStr, pRepl, &r, 2, 2, cLgStart, 1, v);	
}
