#include <windows.h>
#include <tchar.h>
#include "DirList.h"
#include "MemoNote.h"

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
	bChopExtension = nOption & DIRLIST_OPT_CHOPEXTENSION;
	bAllocMemoNotes = nOption & DIRLIST_OPT_ALLOCMEMONOTE;

	pURIBase = pUB;

	if (!vDirList.Init(50, 10)) return FALSE;
	if (!sbDirList.Init(400, 20)) return FALSE;
	return TRUE;
}

DirList::~DirList()
{
}

BOOL DirList::GetList(LPCTSTR pPrefix, LPCTSTR pMatchPath)
{
	// make folder/file list
	WIN32_FIND_DATA wfd;
	HANDLE hHandle = FindFirstFile(pMatchPath, &wfd);
	if (hHandle != INVALID_HANDLE_VALUE) {
		struct DirListItem di;
		MemoNote *pNote;

		do {
			if (_tcscmp(wfd.cFileName, TEXT(".")) == 0 || _tcscmp(wfd.cFileName, TEXT("..")) == 0) continue;

			DWORD l = _tcslen(wfd.cFileName);
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// folder 
				di.bFolder = TRUE;
			} else {
				// file
				if (bAllocMemoNotes) {
					if (!MemoNote::MemoNoteFactory(pPrefix, wfd.cFileName, &pNote) || pNote == NULL) continue;
				} else {
					DWORD n = MemoNote::IsNote(wfd.cFileName);
					if (n == NOTE_TYPE_NO || n == NOTE_TYPE_TDT) continue;
				}
				
				di.bFolder = FALSE;
				di.pNote = pNote;
				if (bChopExtension) {
					l -= 4;
					*(wfd.cFileName + l) = TEXT('\0');
				}
			}

			// Add file name to buffer
			if (!sbDirList.Add(wfd.cFileName, l + 1, &(di.nFileNamePos))) return FALSE;
			if (!vDirList.Add(&di)) return FALSE;

		} while(FindNextFile(hHandle, &wfd));
		FindClose(hHandle);
	}

	DWORD n = vDirList.NumItems();

	// sort 
	pSortSB = &sbDirList;
	qsort((LPBYTE)vDirList.GetBuf(), n, sizeof(struct DirListItem), SortItems);

	return TRUE;
}
