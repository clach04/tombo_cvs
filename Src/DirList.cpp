#include <windows.h>
#include <tchar.h>
#include "DirList.h"
#include "MemoNote.h"
#include "TString.h"
#include "TomboURI.h"
#include "Repository.h"

static StringBufferT *pSortSB;
extern "C" int SortItems(const void *e1, const void *e2)
{
	const DirListItem *p1 = (const DirListItem*)e1;
	const DirListItem *p2 = (const DirListItem*)e2;
	LPCTSTR q1 = pSortSB->Get(p1->nFileNamePos);
	LPCTSTR q2 = pSortSB->Get(p2->nFileNamePos);
	if (p1->bFolder == p2->bFolder) {
		return _tcsicmp(q1, q2);
	} else {
		if (p1->bFolder) {
			return -1;
		} else {
			return 1;
		}
	}
}

BOOL DirList::Init(DWORD nOption, LPCTSTR pUB)
{
	bAllocURI = nOption & DIRLIST_OPT_ALLOCURI;
	bAllocHeadLine = nOption & DIRLIST_OPT_ALLOCHEADLINE;

	pURIBase = pUB;

	if (!vDirList.Init(50, 10)) return FALSE;
	if (!sbDirList.Init(400, 20)) return FALSE;
	return TRUE;
}

DirList::~DirList()
{
}

BOOL DirList::GetList(LPCTSTR pPrefix, LPCTSTR pMatchPath, BOOL bSkipEncrypt)
{
	// make folder/file list
	WIN32_FIND_DATA wfd;
	HANDLE hHandle = FindFirstFile(pMatchPath, &wfd);
	if (hHandle != INVALID_HANDLE_VALUE) {
		struct DirListItem di;

		do {
			if (_tcscmp(wfd.cFileName, TEXT(".")) == 0 || _tcscmp(wfd.cFileName, TEXT("..")) == 0) continue;

			DWORD l = _tcslen(wfd.cFileName);
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// folder 
				di.bFolder = TRUE;
			} else {
				// file
				DWORD n = MemoNote::IsNote(wfd.cFileName);
				if (n == NOTE_TYPE_NO || n == NOTE_TYPE_TDT) continue;
				
				di.bFolder = FALSE;
			}

			if (bAllocURI) {
				if (!sbDirList.Add(pURIBase, _tcslen(pURIBase), &(di.nURIPos))) return FALSE;
				DWORD d;
				if (!sbDirList.Add(wfd.cFileName, l, &d)) return FALSE;
				if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					if (!sbDirList.Add(TEXT("/"), 1, &d)) return FALSE;
				}
				if (!sbDirList.Add(TEXT(""), 1, &d)) return FALSE;
			}

			TomboURI sURI;
			TString sHeadLine;
			if (!sURI.Init(GetFileName(di.nURIPos))) return FALSE;

			URIOption opt(NOTE_OPTIONMASK_ENCRYPTED);
			g_Repository.GetOption(&sURI, &opt);

			if (bAllocHeadLine) {
				if (!opt.bEncrypt || !bSkipEncrypt) {
					if (!g_Repository.GetHeadLine(&sURI, &sHeadLine)) return FALSE;
					if (!sbDirList.Add(sHeadLine.Get(), _tcslen(sHeadLine.Get()) + 1, &(di.nHeadLinePos))) return FALSE;
				}
			}
	
			// Add file name to buffer
			if (!opt.bEncrypt || !bSkipEncrypt) {
				if (!sbDirList.Add(wfd.cFileName, l + 1, &(di.nFileNamePos))) return FALSE;
				if (!vDirList.Add(&di)) return FALSE;
			}
		} while(FindNextFile(hHandle, &wfd));
		FindClose(hHandle);
	}

	DWORD n = vDirList.NumItems();

	// sort 
	pSortSB = &sbDirList;
	qsort((LPBYTE)vDirList.GetBuf(), n, sizeof(struct DirListItem), SortItems);

	return TRUE;
}
