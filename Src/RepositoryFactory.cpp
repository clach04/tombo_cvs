#include <windows.h>
#include <tchar.h>
#include "Repository.h"
#include "RepositoryFactory.h"
#include "Property.h"
#include "TString.h"
#include "TomboURI.h"

/////////////////////////////////////////
// global variable
/////////////////////////////////////////

RepositoryFactory g_RepositoryFactory;

/////////////////////////////////////////
// ctor & dtor, initializer
/////////////////////////////////////////

RepositoryFactory::RepositoryFactory() : pDefaultRepo(NULL)
{
}

RepositoryFactory::~RepositoryFactory()
{
	delete pDefaultRepo;
}

BOOL RepositoryFactory::Init(Property *pProp)
{
	LocalFileRepository *p = new LocalFileRepository();
	if (p == NULL) return FALSE;
	
	if (!p->Init(pProp->TopDir())) return FALSE;

	pDefaultRepo = p;
	return TRUE;
}

/////////////////////////////////////////
// factory method
/////////////////////////////////////////

Repository *RepositoryFactory::GetRepository(TomboURI *pURI)
{
	return pDefaultRepo;
}

// for convinient funcs
Repository *RepositoryFactory::GetRepository(LPCTSTR pURI)
{
	TomboURI uri;
	if (!uri.Init(pURI)) return NULL;
	return GetRepository(&uri);
}
