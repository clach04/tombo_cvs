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
	delete pFileName;
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

VFStore::VFStore(enum OrderInfo odr) : oiOrder(odr), ppArray(NULL)
{
}

VFStore::~VFStore()
{
}

void VFStore::FreeObject()
{
	/* NOP */
}

BOOL VFStore::Prepare()
{
	// free if already allocated.
	if (ppArray) {
		LocalFree(ppArray);
	}

	nCapacity = STORE_INIT_SIZE;
	// allocate array
	ppArray = (VFNote **)LocalAlloc(LMEM_FIXED, sizeof(VFNote*) * STORE_INIT_SIZE);
	if (ppArray == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}

	nPos = 0;
	return TRUE;
}

BOOL VFStore::Store(VFNote *p)
{
	if (nPos >= nCapacity) {
		// extend array.
		// array's address is changed by LocalReAlloc.
		nCapacity += STORE_EXTEND_DELTA;
		ppArray = (VFNote**)LocalReAlloc(ppArray, sizeof(VFNote*) * nCapacity, LMEM_MOVEABLE);
		if (ppArray == NULL) return FALSE;
	}

	ppArray[nPos++] = p;
	return TRUE;
}

BOOL VFStore::PostActivate()
{
	// TODO: sorting.
	return TRUE;
}

void VFStore::FreeArray()
{
	// release VFNote;
	for (DWORD i = 0; i < nPos; i++) {
		delete ppArray[i];
	}

	if (ppArray) {
		LocalFree(ppArray);
		ppArray = NULL;
		nCapacity = nPos = 0;
	}
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

BOOL VFRegexFilter::Init(LPCTSTR pPattern, BOOL bCase, BOOL bEnc, BOOL bFileName, PasswordManager *pPassMgr)
{
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
	char *pText = pNote->GetMemoBodyA(pRegex->GetPasswordManager());
	if (pText == NULL) return FALSE;

	if (pRegex->SearchForward(pText, 0, 0)) {
		BOOL bResult = pNext->Store(p);
		delete [] pText;
		return bResult;
	} else {
		return TRUE;
	}
}
