#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include "Tombo.h"
#include "UniConv.h"
#include "TString.h"
#include "File.h"
#include "Property.h"
#include "MemoSelectView.h"
#include "DirectoryScanner.h"
#include "MemoNote.h"
#include "VFStream.h"
#include "SearchEngine.h"
#include "VarBuffer.h"

#define STORE_INIT_SIZE 100
#define STORE_EXTEND_DELTA 50

////////////////////////////////////
// VFNote
////////////////////////////////////

VFNote::~VFNote()
{
	delete [] pFileName;
//	delete pNote;
}

BOOL VFNote::Init(MemoNote *p, LPCTSTR pFile)
{
	pNote = p;
	pFileName = StringDup(pFile);
	if (pFileName == NULL) return FALSE;
	*(pFileName + _tcslen(pFileName) - 4) = TEXT('\0');

	// Get file update time
	TString aFullPath;
	if (!aFullPath.AllocFullPath(pNote->MemoPath())) return FALSE;

	WIN32_FIND_DATA wfd;
	HANDLE h = FindFirstFile(aFullPath.Get(), &wfd);
	if (h != INVALID_HANDLE_VALUE) {
		uLastUpdate = ((UINT64)wfd.ftLastWriteTime.dwHighDateTime << 32) | (UINT64)wfd.ftLastWriteTime.dwLowDateTime ;
		FindClose(h);
		return TRUE;
	} else {
		return FALSE;
	}
}

////////////////////////////////////
////////////////////////////////////

VFStream::VFStream() : pNext(NULL)
{
}

VFStream::~VFStream()
{
}

BOOL VFStream::SetNext(VFStream *p)
{
	pNext = p;
	return TRUE;
}

BOOL VFStream::Prepare()
{
	if (pNext) return pNext->Prepare();
	return TRUE;
}

void VFStream::FreeObject()
{
	if (pNext) pNext->FreeObject();
	delete pNext;
	pNext = NULL;
}

BOOL VFStream::PostActivate()
{
	if (pNext) return pNext->PostActivate();
	return TRUE;
}

////////////////////////////////////
// Traverse directory
////////////////////////////////////
// VFDirectoryGenerator's helper class

class VirtualStreamFolderScanner : public DirectoryScanner {
	DWORD nTopDirLen;
	VFStream *pNext;
	BOOL bCheckEncrypt;

	DWORD nError;
public:
	void Init(LPCTSTR pPath, VFStream *pStream, BOOL bCe);
	void InitialScan() { nError = ERROR_SUCCESS; }
	void AfterScan() {}
	void PreDirectory(LPCTSTR p){}
	void PostDirectory(LPCTSTR p){}
	void File(LPCTSTR p);

	DWORD GetError() { return nError; }
};

void VirtualStreamFolderScanner::Init(LPCTSTR pPath, VFStream *p, BOOL bCe)
{
	TCHAR buf[MAX_PATH];

	pNext = p;
	bCheckEncrypt = bCe;

	LPCTSTR pDir = g_Property.TopDir();
	wsprintf(buf, TEXT("%s%s"), pDir, pPath);

	nTopDirLen = _tcslen(pDir);

	DirectoryScanner::Init(buf, 0);
}

void VirtualStreamFolderScanner::File(LPCTSTR p)
{
	// if encrypted memo is not search target and the note is encrypted, ignore it
	if (!bCheckEncrypt && MemoNote::IsNote(p) == NOTE_TYPE_CRYPTED) return;

	// create MemoNote object
	MemoNote *pNote = NULL;
	if (!MemoNote::MemoNoteFactory(TEXT(""), CurrentPath() + nTopDirLen, &pNote)) {
		StopScan();
		return;
	}
	if (pNote == NULL) return;

	VFNote *pVF = new VFNote();
	if (pVF == NULL) {
		nError = ERROR_NOT_ENOUGH_MEMORY;
		StopScan();
		delete pNote;
		return;
	}

	if (!pVF->Init(pNote, p)) {
		nError = GetLastError();
		StopScan();
		delete pNote;
		delete pVF;
		return;
	}

	// pass to the filter.
	if (!pNext->Store(pVF)) {
		nError = GetLastError();
		StopScan();
		delete pNote;
		delete pVF;
	}
}

////////////////////////////////////
// VFDirectoryGenerator
////////////////////////////////////

VFDirectoryGenerator::VFDirectoryGenerator() : pDirPath(NULL)
{
}

VFDirectoryGenerator::~VFDirectoryGenerator()
{
	delete [] pDirPath;
}

BOOL VFDirectoryGenerator::Init(LPTSTR p, BOOL bCe)
{
	bCheckEncrypt = bCe;
	pDirPath = p;
	return TRUE;
}

BOOL VFDirectoryGenerator::Activate()
{
	if (!pNext) return FALSE;

	VirtualStreamFolderScanner vfs;
	vfs.Init(pDirPath, pNext, bCheckEncrypt);
	return vfs.Scan();
}

BOOL VFDirectoryGenerator::Store(VFNote *p)
{
	// usually, this member is not called.
	return FALSE;
}

VFStream *VFDirectoryGenerator::Clone(VFStore **ppTail)
{
	VFDirectoryGenerator *p = new VFDirectoryGenerator();
	if (p == NULL) return NULL;
	
	p->bCheckEncrypt = bCheckEncrypt;
	p->pDirPath = StringDup(pDirPath);
	p->pNext = pNext->Clone(ppTail);
	if (!p || p->pDirPath == NULL ||
		p->pNext == NULL) {
		delete p;
		return NULL;
	}
	return p;
}
////////////////////////////////////
// VFStore implimentation
////////////////////////////////////

VFStore::VFStore(enum OrderInfo odr) : oiOrder(odr)
{
}

VFStore::~VFStore()
{
}

BOOL VFStore::Init()
{
	return vNotes.Init(STORE_INIT_SIZE, STORE_EXTEND_DELTA);
}

void VFStore::FreeObject()
{
	/* NOP */
}

BOOL VFStore::Prepare()
{
	if (!vNotes.Clear(TRUE)) return FALSE;
	return TRUE;
}

BOOL VFStore::Store(VFNote *p)
{
	return vNotes.Add(&p);
}

BOOL VFStore::PostActivate()
{
	// TODO: sorting.
	return TRUE;
}

void VFStore::FreeArray()
{
	DWORD n = vNotes.NumItems();
	for (DWORD i = 0; i < n; i++) {
		delete (*vNotes.GetUnit(i));
	}
	vNotes.Clear(FALSE);
}

VFStream *VFStore::Clone(VFStore **ppTail)
{
	VFStore *p = new VFStore(oiOrder);
	if (!p || !p->Init()) return NULL;
	*ppTail = p;
	return p;
}

////////////////////////////////////
//  VFRegexFilter
////////////////////////////////////

VFRegexFilter::VFRegexFilter() : pRegex(NULL)
{
}

VFRegexFilter::~VFRegexFilter()
{
	delete pRegex;
}

BOOL VFRegexFilter::Init(LPCTSTR pPattern, BOOL bCase, BOOL bEnc, BOOL bFileName, BOOL bNeg, PasswordManager *pPassMgr)
{
	bNegate = bNeg;
	pRegex = new SearchEngineA();
	const char *pReason;
	if (!pRegex || !pRegex->Init(bEnc, bFileName, pPassMgr)) return FALSE;
	if (!pRegex->Prepare(pPattern, bCase, &pReason)) return FALSE;
	return TRUE;
}

BOOL VFRegexFilter::Store(VFNote *p)
{
	MemoNote *pNote = p->GetNote();
	if (pNote == NULL) return FALSE;
	switch(pRegex->Search(pNote)) {
	case SR_NOTFOUND:
		if (bNegate) {
			return pNext->Store(p);
		} else {
			delete p; // when discarding, Store() must delete object p.
			return TRUE;
		}
	case SR_FOUND:
		if (bNegate) {
			delete p; // when discarding, Store() must delete object p.
			return TRUE;
		} else {
			return pNext->Store(p);
		}
	case SR_CANCELED:
	case SR_FAILED:
	default:
		return FALSE;
	}
}

VFStream *VFRegexFilter::Clone(VFStore **ppTail)
{
	VFRegexFilter *p = new VFRegexFilter();
	if (p == NULL) return NULL;

	p->pRegex = pRegex->Clone();
	p->pNext = pNext->Clone(ppTail);
	p->bNegate = bNegate;
	if (!p || p->pRegex == NULL || 
		p->pNext == NULL) {
		delete p;
		return NULL;
	}
	return p;
}
////////////////////////////////////
//  VFLimitFilter
////////////////////////////////////

VFLimitFilter::VFLimitFilter() : nLimit(0)
{
}

VFLimitFilter::~VFLimitFilter()
{
}

BOOL VFLimitFilter::Init(DWORD n)
{
	nLimit = n;
	return TRUE;
}

BOOL VFLimitFilter::Prepare()
{
	nCount = 0;
	return VFStream::Prepare();
}

BOOL VFLimitFilter::Store(VFNote *p)
{
	if (nCount >= nLimit) {
		delete p;
		return TRUE;
	}
	nCount++;
	return pNext->Store(p);
}

VFStream *VFLimitFilter::Clone(VFStore **ppTail)
{
	VFLimitFilter *p = new VFLimitFilter();
	if (!p || !p->Init(nLimit)) {
		delete p;
		return NULL;
	}
	p->pNext = pNext->Clone(ppTail);
	if (!p || p->pNext == NULL) {
		delete p;
		return NULL;
	}
	return p;
}

////////////////////////////////////
//  VFTimestampFilter
////////////////////////////////////

VFTimestampFilter::VFTimestampFilter()
{
}

VFTimestampFilter::~VFTimestampFilter()
{
}

BOOL VFTimestampFilter::Init(DWORD nDelta, BOOL bNew)
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	FILETIME ft;
	SystemTimeToFileTime(&st, &ft);

	uBase = ((UINT64)ft.dwHighDateTime << 32) | (UINT64)ft.dwLowDateTime;
	UINT64 d = 0xc92a69c000;	// 1 day
	uBase -= d * nDelta;
	bNewer = bNew;

	return TRUE;
}

BOOL VFTimestampFilter::Store(VFNote *pNote)
{
	MemoNote *pMemo = pNote->GetNote();
	if (bNewer && pNote->GetLastUpdate() > uBase || (!bNewer && pNote->GetLastUpdate() < uBase)) {
		return pNext->Store(pNote);
	} else {
		delete pNote;
		return TRUE;
	}
}

VFStream *VFTimestampFilter::Clone(VFStore **ppTail)
{
	VFTimestampFilter *p = new VFTimestampFilter();
	if (!p) return NULL;

	p->bNewer = bNewer;
	p->uBase = uBase;
	p->pNext = pNext->Clone(ppTail);
	if (!p || p->pNext == NULL) {
		delete p;
		return NULL;
	}
	return p;
}

////////////////////////////////////
//  VFSortFilter
////////////////////////////////////

VFSortFilter::VFSortFilter()
{
}

VFSortFilter::~VFSortFilter()
{
}

BOOL VFSortFilter::Init(SortFuncType sf)
{
	sfType = sf;
	return vNotes.Init(STORE_INIT_SIZE, STORE_EXTEND_DELTA);
}

BOOL VFSortFilter::Prepare()
{
	vNotes.Clear(TRUE);
	return VFStream::Prepare();
}

BOOL VFSortFilter::Store(VFNote *p)
{
	if (!vNotes.Add(&p)) return FALSE;
	return TRUE;
}

extern "C" {
typedef int SortFunc(const void *e1, const void *e2);
};

extern "C" static int SortNotes_FileNameAsc(const void *e1, const void *e2)
{
	VFNote *p1 = *(VFNote**)e1;
	VFNote *p2 = *(VFNote**)e2;
	return _tcsicmp(p1->GetFileName(), p2->GetFileName());
}

extern "C" static int SortNotes_FileNameDesc(const void *e1, const void *e2)
{
	VFNote *p1 = *(VFNote**)e1;
	VFNote *p2 = *(VFNote**)e2;
	return _tcsicmp(p2->GetFileName(), p1->GetFileName());
}

extern "C" static int SortNotes_LastUpdateOlder(const void *e1, const void *e2)
{
	VFNote *p1 = *(VFNote**)e1;
	VFNote *p2 = *(VFNote**)e2;
	if (p1->GetLastUpdate() == p2->GetLastUpdate()) return 0;
	if (p1->GetLastUpdate() > p2->GetLastUpdate()) return 1;
	else return -1;
}

extern "C" static int SortNotes_LastUpdateNewer(const void *e1, const void *e2)
{
	VFNote *p1 = *(VFNote**)e1;
	VFNote *p2 = *(VFNote**)e2;
	if (p2->GetLastUpdate() == p1->GetLastUpdate()) return 0;
	if (p2->GetLastUpdate() > p1->GetLastUpdate()) return 1;
	else return -1;
}

BOOL VFSortFilter::PostActivate()
{
	VFNote *p;
	DWORD n = vNotes.NumItems();

	SortFunc *pFunc;
	switch(sfType) {
	case SortFunc_FileNameAsc:
		pFunc = SortNotes_FileNameAsc;
		break;
	case SortFunc_FileNameDsc:
		pFunc = SortNotes_FileNameDesc;
		break;
	case SortFunc_LastUpdateAsc:
		pFunc = SortNotes_LastUpdateOlder;
		break;
	case SortFunc_LastUpdateDsc:
		pFunc = SortNotes_LastUpdateNewer;
		break;
	default:
		return FALSE;
	}
	qsort((LPBYTE)vNotes.GetBuf(), n, sizeof(VFNote*), pFunc);
	for (DWORD i = 0; i < n; i++) {
		p = *vNotes.GetUnit(i);
		if (!pNext->Store(p)) return FALSE;
	}
	return VFStream::PostActivate();
}

VFStream *VFSortFilter::Clone(VFStore **ppTail)
{
	VFSortFilter *p = new VFSortFilter();
	if (!p || !p->Init(sfType)) return NULL;

	p->pNext = pNext->Clone(ppTail);
	if (!p || p->pNext == NULL) {
		delete p;
		return NULL;
	}
	return p;
}
