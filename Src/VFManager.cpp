#include <windows.h>
#include <tchar.h>
#include "Message.h"
#include "VarBuffer.h"
#include "VFManager.h"
#include "VFStream.h"
#include "PasswordManager.h"
#include "Tombo.h"
#include "UniConv.h"
#include "TString.h"
#include "TSParser.h"

/////////////////////////////////////////////
//  ctor & dtor
/////////////////////////////////////////////

VFManager::VFManager() : nGrepCount(0)
{
}

VFManager::~VFManager()
{
	// delete vbInfo's items memory
	DWORD n = vbInfo.NumItems();
	VFInfo *p;
	for (DWORD i = 0; i < n; i++) {
		p = vbInfo.GetUnit(i);
		p->Release();
	}
}

BOOL VFManager::Init()
{
	return vbInfo.Init(5,5);
}

/////////////////////////////////////////////
//  Create stream from VFInfo
/////////////////////////////////////////////

BOOL VFManager::StreamObjectsFactory(const VFInfo *pInfo, VFDirectoryGenerator **ppGen, VFStore **ppStore)
{
	*ppGen = (VFDirectoryGenerator*)pInfo->pGenerator->Clone(ppStore);
	return *ppGen != NULL;
}
#ifdef COMMENT
BOOL VFManager::StreamObjectsFactory(const VFInfo *pInfo, VFDirectoryGenerator **ppGen, VFStore **ppStore)
{
	// Initialize regex filter
	VFRegexFilter *pRegex = new VFRegexFilter();
	if (!pRegex->Init(pInfo->pRegex,
					pInfo->nFlag & VFINFO_FLG_CASESENSITIVE,
					pInfo->nFlag & VFINFO_FLG_CHECKCRYPTED,
					pInfo->nFlag & VFINFO_FLG_FILENAMEONLY,
					pInfo->nFlag & VFINFO_FLG_NEGATE,
					g_pPasswordManager)) { // Password Manager
		return FALSE;
	}

	// Initialize other objects
	*ppGen = new VFDirectoryGenerator();
	*ppStore = new VFStore(VFStore::ORDER_TITLE);
	LPTSTR pPath = StringDup(pInfo->pPath);
	if (!pRegex || !*ppGen || !*ppStore || !pPath ||
		!(*ppGen)->Init(pPath, pInfo->nFlag & VFINFO_FLG_CHECKCRYPTED) ||
		!(*ppStore)->Init()) { // pPassMgr
		delete pRegex;
		delete *ppGen;
		delete *ppStore;
		delete [] pPath;
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}

	(*ppGen)->SetNext(pRegex);
	pRegex->SetNext(*ppStore);

	return TRUE;
}
#endif
/////////////////////////////////////////////
//  Get VFInfo
/////////////////////////////////////////////
const VFInfo *VFManager::GetGrepVFInfo(LPCTSTR pPath, LPCTSTR pRegex,
						BOOL bIsCaseSensitive, BOOL bCheckCrypt, BOOL bCheckFileName, BOOL bNegate)
{
	if (pPath == NULL || pRegex == NULL) {
		SetLastError(ERROR_INVALID_DATA);
		return NULL;
	}
	VFInfo vInfo;

	// Create template stream
	VFDirectoryGenerator *pGen = new VFDirectoryGenerator();
	VFStore *pStore = new VFStore(VFStore::ORDER_TITLE);
	VFRegexFilter *pFilter = new VFRegexFilter();
	if (!pGen || !pStore || !pFilter) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		delete pGen;
		delete pStore;
		delete pFilter;
		return NULL;
	}
	if (!pGen->Init(StringDup(pPath), bCheckCrypt) ||
		!pFilter->Init(pRegex, bIsCaseSensitive, bCheckCrypt, bCheckFileName, bNegate, g_pPasswordManager) ||
		!pStore->Init()) {
		delete pGen;
		delete pStore;
		delete pFilter;
		return NULL;
	}

	pGen->SetNext(pFilter); pFilter->SetNext(pStore); //link objects
	vInfo.bPersist = FALSE;
	vInfo.pGenerator = pGen;
	vInfo.pStore = pStore;

	// Name to this virtualfolder.
	vInfo.pName = new TCHAR[_tcslen(MSG_GREP_NONAME_LABEL) + 10];
	if (vInfo.pName == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	wsprintf(vInfo.pName, TEXT("%s%03d"), MSG_GREP_NONAME_LABEL, ++nGrepCount);

	// Regist data
	if (!vbInfo.Add(&vInfo)) return FALSE;
	DWORD n = vbInfo.NumItems();
	VFInfo *q = vbInfo.GetUnit(n - 1);

	return q;
}

#ifdef COMMENT
const VFInfo *VFManager::GetGrepVFInfo(LPCTSTR pPath, LPCTSTR pRegex,
						BOOL bIsCaseSensitive, BOOL bCheckCrypt, BOOL bCheckFileName, BOOL bNegate)
{
	if (pPath == NULL || pRegex == NULL) {
		SetLastError(ERROR_INVALID_DATA);
		return NULL;
	}

	VFInfo *p = new VFInfo();

	p->nFlag = 0;
	if (bIsCaseSensitive) { p->nFlag |= VFINFO_FLG_CASESENSITIVE; }
	if (bCheckCrypt) { p->nFlag |= VFINFO_FLG_CHECKCRYPTED; }
	if (bCheckFileName) { p->nFlag |= VFINFO_FLG_FILENAMEONLY; }
	if (bNegate) { p->nFlag |= VFINFO_FLG_NEGATE; }

	p->pPath = new TCHAR[_tcslen(pPath) + 2];
	if (p->pPath == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		delete p;
		return NULL;
	}
	*(p->pPath) = TEXT('\\');
	_tcscpy(p->pPath + 1, pPath);

	p->pRegex = StringDup(pRegex);
	if (p->pRegex == NULL) {
		delete p;
		return NULL;
	}

	p->pName = new TCHAR[_tcslen(MSG_GREP_NONAME_LABEL) + 10];
	if (p->pName == NULL) {
		delete p;
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	wsprintf(p->pName, TEXT("%s%03d"), MSG_GREP_NONAME_LABEL, ++nGrepCount);

	if (!vbInfo.Add(p)) return FALSE;
	DWORD n = vbInfo.NumItems();
	VFInfo *q = vbInfo.GetUnit(n - 1);
	delete p;
	return q;
}
#endif
/////////////////////////////////////////////
//  Enum vfolders
/////////////////////////////////////////////

BOOL VFManager::Enum(VirtualFolderEnumListener *pListener)
{
	VFInfo *pInfo;
	VFDirectoryGenerator *pGen;
	VFStore *pStore;

	/// Add virtual folders.
	TCHAR buf[MAX_PATH + 1];
	TCHAR buf2[MAX_PATH + 1];
	GetModuleFileName(NULL, buf, MAX_PATH);
	GetFilePath(buf2, buf);

	TString sVFpath;
	if (!sVFpath.Join(buf2, TOMBO_VFOLDER_DEF_FILE)) return FALSE;

	TSParser tp;
	tp.Parse(sVFpath.Get(), pListener);

	// Enum temporary vfolders
	DWORD n = vbInfo.NumItems();
	for (DWORD i = 0; i < n; i++) {
		pInfo = vbInfo.GetUnit(i);
		if (!StreamObjectsFactory(pInfo, &pGen, &pStore)) return FALSE;
		if (!pListener->ProcessStream(pInfo->pName, pGen, pStore)) break;
	}
	return TRUE;
}

BOOL VFManager::RetrieveInfo(const VFInfo *pInfo, VirtualFolderEnumListener *pListener)
{
	VFDirectoryGenerator *pGen;
	VFStore *pStore;
	if (!StreamObjectsFactory(pInfo, &pGen, &pStore)) return FALSE;
	pListener->ProcessStream(pInfo->pName, pGen, pStore);
	return TRUE;
}

/////////////////////////////////////////////
//  VirtualFolderEnumListener implimentation
/////////////////////////////////////////////

VirtualFolderEnumListener::~VirtualFolderEnumListener()
{
}

/////////////////////////////////////////////
//  VFInfo implimentation
/////////////////////////////////////////////

void VFInfo::Release()
{
	 delete[] pName;
	 delete[] pPath;
	 delete[] pRegex;
}

