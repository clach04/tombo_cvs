#ifndef REPOSITORY_H
#define REPOSITORY_H

/////////////////////////////////////////
// Notes repository
/////////////////////////////////////////

class Repository {
public:
	Repository();
	virtual ~Repository();
};

////////////////////////////////////////////////
// Repository which stores notes to local file
////////////////////////////////////////////////

class LocalFileRepository : public Repository {
	LPTSTR pTopDir;
public:
	LocalFileRepository();
	virtual ~LocalFileRepository();

	BOOL Init(LPCTSTR pRoot);
};

#endif