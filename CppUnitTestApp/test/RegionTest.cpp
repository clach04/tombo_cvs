#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>

#include <windows.h>
#include "Region.h"

class RegionTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(RegionTest);
	CPPUNIT_TEST(coordEqualTest);
	CPPUNIT_TEST(coordCompTest);
	CPPUNIT_TEST_SUITE_END();

public:
	RegionTest() {}
	~RegionTest() {}

	virtual void setUp() {}
	virtual void tearDown() {}


	void coordEqualTest();
	void coordCompTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(RegionTest);

void RegionTest::coordEqualTest()
{
	Coordinate c1(2,2);

	Coordinate c2;

	c2.Set(2,2);
	CPPUNIT_ASSERT(c1 == c2);

	Coordinate c3(2,3);
	CPPUNIT_ASSERT(!(c1 == c3));
	Coordinate c4(3,2);
	CPPUNIT_ASSERT(!(c1 == c4));
}

void RegionTest::coordCompTest()
{
	Coordinate c1(2,2);
	Coordinate c3(2,3);
	Coordinate c4(3,2);
	Coordinate c2;

	c2.Set(2,2);

	CPPUNIT_ASSERT(!(c1 < c2));
	CPPUNIT_ASSERT(c1 < c3);
	CPPUNIT_ASSERT(c2 < c4);
}