#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>

#include <windows.h>
#include <tchar.h>
#include "TString.h"
#include "TomboURI.h"

#define TEST_CLASS_NAME TomboURITest

class TEST_CLASS_NAME : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(TEST_CLASS_NAME);
	CPPUNIT_TEST(InitTest1);
	CPPUNIT_TEST(InitTest2);
	CPPUNIT_TEST(InitTest3);
	CPPUNIT_TEST(InitTest4);
	CPPUNIT_TEST(InitTest5);
	CPPUNIT_TEST(InitTest6);
	CPPUNIT_TEST(GetRepoTest1);
	CPPUNIT_TEST(HeadLineTest1);
	CPPUNIT_TEST(HeadLineTest2);
	CPPUNIT_TEST(HeadLineTest3);
	CPPUNIT_TEST(IteratorTest1);
	CPPUNIT_TEST(IteratorTest2);
	CPPUNIT_TEST(IteratorTest3);
	CPPUNIT_TEST_SUITE_END();

public:
	TEST_CLASS_NAME() {}
	~TEST_CLASS_NAME() {}

	virtual void setUp() {}
	virtual void tearDown() {}

	void InitTest1();
	void InitTest2();
	void InitTest3();
	void InitTest4();
	void InitTest5();
	void InitTest6();
	void GetRepoTest1();
	void HeadLineTest1();
	void HeadLineTest2();
	void HeadLineTest3();
	void IteratorTest1();
	void IteratorTest2();
	void IteratorTest3();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TEST_CLASS_NAME);

////////////////////////////////////////////////

void TEST_CLASS_NAME::InitTest1() {
	// no header information. fail.

	TomboURI uri;
	BOOL b = uri.Init("test");
	CPPUNIT_ASSERT(!b);
	CPPUNIT_ASSERT(GetLastError() == ERROR_INVALID_DATA);
}

void TEST_CLASS_NAME::InitTest2() {
	// incomplete repository definition. fail.

	TomboURI uri;
	BOOL b = uri.Init("tombo://default");
	CPPUNIT_ASSERT(!b);
	CPPUNIT_ASSERT(GetLastError() == ERROR_INVALID_DATA);
}

void TEST_CLASS_NAME::InitTest3() {
	// directs root of default repository.

	TomboURI uri;
	BOOL b = uri.Init("tombo://default/");
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(uri.nMaxPathItem == 7);
}

void TEST_CLASS_NAME::InitTest4() {
	// exists first item (no path sep)

	TomboURI uri;
	BOOL b = uri.Init("tombo://default/hello");
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(uri.nMaxPathItem == 7);
}

void TEST_CLASS_NAME::InitTest5() {
	// exists first item (with path sep)

	TomboURI uri;
	BOOL b = uri.Init("tombo://default/hello/");
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(uri.nMaxPathItem == 7);
}

void TEST_CLASS_NAME::InitTest6() {

	TomboURI uri;
	BOOL b = uri.Init("tombo://default/hello world");
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(uri.nMaxPathItem == 11);
}

void TEST_CLASS_NAME::GetRepoTest1() {
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/"));
	
	TString repo;
	BOOL b = uri.GetRepository(&repo);
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(strcmp(repo.Get(), "default") == 0);
}

void TEST_CLASS_NAME::HeadLineTest1() {
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/"));

	TString hl;
	BOOL b = uri.GetHeadLine(&hl);
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(strcmp(hl.Get(), "[root]") == 0);
}

void TEST_CLASS_NAME::HeadLineTest2() {
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/abc/def/"));

	TString hl;
	BOOL b = uri.GetHeadLine(&hl);
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(strcmp(hl.Get(), "def") == 0);
}

void TEST_CLASS_NAME::HeadLineTest3() {
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/abc/def/ghi.txt"));

	TString hl;
	BOOL b = uri.GetHeadLine(&hl);
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(strcmp(hl.Get(), "ghi") == 0);
}

void TEST_CLASS_NAME::IteratorTest1() {
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/"));

	TomboURIItemIterator itr(&uri);
	CPPUNIT_ASSERT(itr.Init());
	LPCTSTR p;
	itr.First();
	p = itr.Current();
	CPPUNIT_ASSERT(p == NULL);
	CPPUNIT_ASSERT(itr.IsLeaf() == FALSE);
}

void TEST_CLASS_NAME::IteratorTest2() {
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/aaa/bbb/ccc.txt"));

	TomboURIItemIterator itr(&uri);
	CPPUNIT_ASSERT(itr.Init());
	LPCTSTR p;
	itr.First();

	p = itr.Current();
	CPPUNIT_ASSERT(strcmp(p, "aaa") == 0);
	CPPUNIT_ASSERT(itr.IsLeaf() == FALSE);
	itr.Next();

	p = itr.Current();
	CPPUNIT_ASSERT(strcmp(p, "bbb") == 0);
	CPPUNIT_ASSERT(itr.IsLeaf() == FALSE);
	itr.Next();

	p = itr.Current();
	CPPUNIT_ASSERT(strcmp(p, "ccc.txt") == 0);
	CPPUNIT_ASSERT(itr.IsLeaf() == TRUE);
	itr.Next();

	p = itr.Current();
	CPPUNIT_ASSERT(p == NULL);

	itr.Next();
	CPPUNIT_ASSERT(p == NULL);

}

void TEST_CLASS_NAME::IteratorTest3() {
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/aaa/bbb/"));

	TomboURIItemIterator itr(&uri);
	CPPUNIT_ASSERT(itr.Init());
	LPCTSTR p;
	itr.First();

	p = itr.Current();
	CPPUNIT_ASSERT(strcmp(p, "aaa") == 0);
	CPPUNIT_ASSERT(itr.IsLeaf() == FALSE);
	itr.Next();

	p = itr.Current();
	CPPUNIT_ASSERT(strcmp(p, "bbb") == 0);
	CPPUNIT_ASSERT(itr.IsLeaf() == FALSE);
	itr.Next();

	p = itr.Current();
	CPPUNIT_ASSERT(p == NULL);

}

