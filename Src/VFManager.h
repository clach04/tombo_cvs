#ifndef VFMANAGER_H
#define VFMANAGER_H

class VFDirectoryGenerator;
class VFStore;

/////////////////////////////////////////////
//  Virtual folder definition
/////////////////////////////////////////////

#define VFINFO_FLG_CASESENSITIVE	1
#define VFINFO_FLG_CHECKCRYPTED		2
#define VFINFO_FLG_FILENAMEONLY		4
#define VFINFO_FLG_PERSISTENT		8
#define VFINFO_FLG_NEGATE			16

class VFInfo {
public:
	VFInfo() : pName(NULL), pPath(NULL), pRegex(NULL) {}

	void Release();

	LPTSTR pName;
	LPTSTR pPath;
	LPTSTR pRegex;
	DWORD nFlag;

	BOOL bPersist;
	VFDirectoryGenerator *pGenerator;
	VFStore *pStore;
};

/////////////////////////////////////////////
//  Enumerator
/////////////////////////////////////////////

class VirtualFolderEnumListener {
public:
	virtual ~VirtualFolderEnumListener();
	virtual BOOL ProcessStream(LPCTSTR pName, VFDirectoryGenerator *pGen, VFStore *pStore) = 0;
};

/////////////////////////////////////////////
//  Virtual folder manager
/////////////////////////////////////////////

class VFManager {
	DWORD nGrepCount;
	TVector<VFInfo> vbInfo;
public:
	/////////////////////////////////
	// ctor & dtor
	VFManager();
	~VFManager();
	BOOL Init();

	BOOL StreamObjectsFactory(const VFInfo *pInfo, VFDirectoryGenerator **ppGen, VFStore **ppStore);

	const VFInfo *GetGrepVFInfo(LPCTSTR pPath, LPCTSTR pRegex,
							BOOL bIsCaseSensitive, BOOL bCheckCrypt, BOOL bCheckFileName, BOOL bNegate);

	BOOL Enum(VirtualFolderEnumListener *pListener);
	BOOL RetrieveInfo(const VFInfo *pInfo, VirtualFolderEnumListener *pListener);
};

#endif
