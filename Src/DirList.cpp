#include <windows.h>
#include <tchar.h>
#include "DirList.h"
#include "MemoNote.h"

static StringBufferT *pSortSB;
extern "C" int SortItems(const void *e1, const void *e2)
{
	const DirListItem *p1 = (const DirListItem*)e1;
	const DirListItem *p2 = (const DirListItem*)e2;
	LPCTSTR q1 = pSortSB->Get(p1->nNamePos);
	LPCTSTR q2 = pSortSB->Get(p2->nNamePos);
	if (p1->bFlg == p2->bFlg) {
		return _tcsicmp(q1, q2);
	} else {
		if (p1->bFlg == 1) {
			return -1;
		} else {
			return 1;
		}
	}
}

BOOL DirList::Init(BOOL bCE, BOOL bAMN)
{
	bChopExtension = bCE;
	bAllocMemoNotes = bAMN;

	if (!vDirList.Init(50, 10)) return FALSE;
	if (!sbDirList.Init(100, 50)) return FALSE;
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
				di.bFlg = 1;
			} else {
				// file
				if (bAllocMemoNotes) {
					if (!MemoNote::MemoNoteFactory(pPrefix, wfd.cFileName, &pNote) || pNote == NULL) continue;
				} else {
					DWORD n = MemoNote::IsNote(wfd.cFileName);
					if (n == NOTE_TYPE_NO || n == NOTE_TYPE_TDT) continue;
				}
				
				di.bFlg = 0;
				di.pNote = pNote;
				if (bChopExtension) {
					l -= 4;
					*(wfd.cFileName + l) = TEXT('\0');
				}
			}

			// Add file name to buffer
			if (!sbDirList.Add(wfd.cFileName, l + 1, &(di.nNamePos))) return FALSE;
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
