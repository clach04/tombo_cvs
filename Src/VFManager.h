#ifndef VFMANAGER_H
#define VFMANAGER_H

class VFDirectoryGenerator;
class VFStore;
class File;

/////////////////////////////////////////////
//  Virtual folder definition
/////////////////////////////////////////////

class VFInfo {
public:
	VFInfo() : pName(NULL){}

	void Release();
	BOOL WriteXML(File *p);

	LPTSTR pName;
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

	// pGen and pStore is controled under callee. you must release stream.
	virtual BOOL ProcessStream(LPCTSTR pName, BOOL bPersist, VFDirectoryGenerator *pGen, VFStore *pStore) = 0;
};

/////////////////////////////////////////////
//  Virtual folder manager
/////////////////////////////////////////////

class VFManager {
	DWORD nGrepCount;
	TVector<VFInfo> vbInfo;
protected:
	void ClearInfo();

public:
	/////////////////////////////////
	// ctor & dtor
	VFManager();
	~VFManager();
	BOOL Init();

	/////////////////////////////////
	// factory methods
	BOOL StreamObjectsFactory(const VFInfo *pInfo, VFDirectoryGenerator **ppGen, VFStore **ppStore);
	const VFInfo *GetGrepVFInfo(LPCTSTR pPath, LPCTSTR pRegex,
							BOOL bIsCaseSensitive, BOOL bCheckCrypt, BOOL bCheckFileName, BOOL bNegate);

	/////////////////////////////////
	// enumerators
	BOOL Enum(VirtualFolderEnumListener *pListener);
	BOOL RetrieveInfo(const VFInfo *pInfo, VirtualFolderEnumListener *pListener);

	/////////////////////////////////
	// Update folders
	BOOL UpdateVirtualFolders(VFInfo **ppInfo, DWORD nNumFolders);

};

#endif
