#ifndef REPOSITORYFACTORY_H
#define REPOSITORYFACTORY_H

class Repository;
class Property;
class TomboURI;

/////////////////////////////////////////
// Repository factory
/////////////////////////////////////////

class RepositoryFactory {
	Repository *pDefaultRepo;

public:
	RepositoryFactory();
	~RepositoryFactory();

	BOOL Init(Property *pProp);

	Repository *GetRepository(TomboURI *pURI);
	Repository *GetRepository(LPCTSTR pURI);
};

/////////////////////////////////////////
// Global definition
/////////////////////////////////////////

extern RepositoryFactory g_RepositoryFactory;


#endif