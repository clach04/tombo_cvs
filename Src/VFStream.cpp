#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include "Tombo.h"
#include "UniConv.h"
#include "File.h"
#include "Property.h"
#include "MemoSelectView.h"
#include "DirectoryScanner.h"
#include "MemoNote.h"
#include "VFStream.h"
#include "SearchEngine.h"

#define STORE_INIT_SIZE 100
#define STORE_EXTEND_DELTA 50

////////////////////////////////////
// VFNote
////////////////////////////////////

VFNote::~VFNote()
{
	delete [] pFileName;
	delete pNote;
}

BOOL VFNote::Init(MemoNote *p, LPCTSTR pFile)
{
	pNote = p;
	pFileName = StringDup(pFile);
	*(pFileName + _tcslen(pFileName) - 4) = TEXT('\0');
	return (pFileName != NULL);
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

////////////////////////////////////
//  VFTimestampFilter
////////////////////////////////////

VFTimestampFilter::VFTimestampFilter()
{
}

VFTimestampFilter::~VFTimestampFilter()
{
}

BOOL VFTimestampFilter::Init(DWORD nBase, BOOL bNew)
{
	nBaseTime = nBase;
	bNewer = bNew;
	return TRUE;
}

BOOL VFTimestampFilter::Store(VFNote *pNote)
{
	return TRUE;
}
