#include <windows.h>
#include <tchar.h>

#include "UniConv.h"
#include "Repository.h"

/////////////////////////////////////////
// Repository ctor & dtor, initializer
/////////////////////////////////////////

Repository::Repository() {}
Repository::~Repository() {}

/////////////////////////////////////////
// ctor & dtor, initializer
/////////////////////////////////////////

LocalFileRepository::LocalFileRepository() : pTopDir(NULL)
{
}

LocalFileRepository::~LocalFileRepository()
{
	delete[] pTopDir;
}

BOOL LocalFileRepository::Init(LPCTSTR pRoot)
{
	pTopDir = StringDup(pRoot);
	return TRUE;
}