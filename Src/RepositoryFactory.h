#ifndef REPOSITORYFACTORY_H
#define REPOSITORYFACTORY_H

class Repository;
class Property;

/////////////////////////////////////////
// Repository factory
/////////////////////////////////////////

class RepositoryFactory {
	Repository *pDefaultRepo;

public:
	RepositoryFactory();
	~RepositoryFactory();

	BOOL Init(Property *pProp);

	Repository *GetRepository(LPCTSTR pRepoName);
};

/////////////////////////////////////////
// Global definition
/////////////////////////////////////////

extern RepositoryFactory g_RepositoryFactory;


#endif