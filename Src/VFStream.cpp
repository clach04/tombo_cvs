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

extern "C" {
void* Regex_Compile(const char *pPattern, BOOL bIgnoreCase, const char **ppReason);
void Regex_Free(void *p);
int Regex_Search(void *p, int iStart, int iRange, const char *pTarget, int *pStart, int *pEnd);
}

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

////////////////////////////////////
// VFDirectoryGenerator implimentation
////////////////////////////////////
/*
class TestScan : public DirectoryScanner {
	MemoSelectView *pView;
	HTREEITEM hParent;
	void *pRegex;
	int x, numfile;
	DWORD nTopDirLen;
public:
	void Init(MemoSelectView *p, HTREEITEM hItem, const char *pStr);
	void InitialScan();	// スキャン開始前
	void AfterScan();  // スキャン処理後
	void PreDirectory(LPCTSTR p){} // ディレクトリ走査前
	void PostDirectory(LPCTSTR p){} // ディレクトリ走査後
	void File(LPCTSTR p); // ファイル
};

void TestScan::Init(MemoSelectView *pv, HTREEITEM hItem, const char *pStr)
{
	const char *p;
	pView = pv;
	hParent = hItem;
	pRegex = Regex_Compile(pStr, TRUE, &p);
	x = 0; numfile = 0;
	LPCTSTR pDir = g_Property.TopDir();
	nTopDirLen = _tcslen(pDir);
	DirectoryScanner::Init(pDir, 0);
}

void TestScan::InitialScan()
{
}

void TestScan::AfterScan()
{
	Regex_Free(pRegex);
}

void TestScan::File(LPCTSTR p)
{
	int n = _tcslen(p);
	int s, e, r;
	numfile++;
	if (n < 4 || _tcscmp(p + n - 4, TEXT(".txt"))) return;

	class File inf;
	if (!inf.Open(CurrentPath(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING)) return;

	char *pText = new char[inf.FileSize() + 1];
	if (pText == NULL) return;

	DWORD nSize = inf.FileSize();
	if (!inf.Read((LPBYTE)pText, &nSize)) return;
	pText[nSize] = TEXT('\0');

	r = Regex_Search(pRegex, 0, nSize, pText, &s, &e);
	delete [] pText;

	if (r > 0) {
		x++;
		DWORD nl = _tcslen(CurrentPath() + nTopDirLen + 1) - _tcslen(p);
		LPTSTR pPrefix = new TCHAR[nl + 1];
		_tcsncpy(pPrefix, CurrentPath() + nTopDirLen + 1, nl);
		pPrefix[nl] = TEXT('\0');
		pView->InsertFile(hParent, pPrefix, p);
		delete [] pPrefix;
	}
}
*/

// =============================================================
class VirtualStreamFolderScanner : public DirectoryScanner {
	DWORD nTopDirLen;
	VFStream *pNext;

	DWORD nError;
public:
	void Init(LPCTSTR pPath, VFStream *pStream);
	void InitialScan() { nError = ERROR_SUCCESS; }
	void AfterScan() {}
	void PreDirectory(LPCTSTR p){}
	void PostDirectory(LPCTSTR p){}
	void File(LPCTSTR p);

	DWORD GetError() { return nError; }
};

void VirtualStreamFolderScanner::Init(LPCTSTR pPath, VFStream *p)
{
	TCHAR buf[MAX_PATH];

	pNext = p;

	LPCTSTR pDir = g_Property.TopDir();
	wsprintf(buf, TEXT("%s%s"), pDir, pPath);

	nTopDirLen = _tcslen(pDir);

	DirectoryScanner::Init(buf, 0);
}

void VirtualStreamFolderScanner::File(LPCTSTR p)
{
	int n = _tcslen(p);
	if (n < 4 || _tcscmp(p + n - 4, TEXT(".txt"))) return;

	// Create & init MemoNote object.
	PlainMemoNote *pNote = new PlainMemoNote();
	if (pNote == NULL) {
		nError = ERROR_NOT_ENOUGH_MEMORY;
		StopScan();
		return;
	}
	if (!pNote->Init(CurrentPath() + nTopDirLen)) {
		nError = GetLastError();
		StopScan();
		delete pNote;
	}
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

// =============================================================

VFDirectoryGenerator::VFDirectoryGenerator() : pDirPath(NULL)
{
}

VFDirectoryGenerator::~VFDirectoryGenerator()
{
	delete [] pDirPath;
}

void VFDirectoryGenerator::FreeObject()
{
	if (pNext) pNext->FreeObject();
	delete pNext;
	pNext = NULL;
}

BOOL VFDirectoryGenerator::Init(LPTSTR p)
{
	pDirPath = p;
	return TRUE;
}

BOOL VFDirectoryGenerator::Activate()
{
	if (!pNext) return FALSE;

	VirtualStreamFolderScanner vfs;
	vfs.Init(pDirPath, pNext);
	return vfs.Scan();
}

BOOL VFDirectoryGenerator::Prepare()
{
	if (pNext) return pNext->Prepare();
	return TRUE;
}

BOOL VFDirectoryGenerator::Store(VFNote *p)
{
	// usually, this member is not called.
	return FALSE;
}

BOOL VFDirectoryGenerator::PostActivate()
{
	if (pNext) return pNext->PostActivate();
	return TRUE;
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