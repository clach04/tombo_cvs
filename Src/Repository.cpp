#include <windows.h>
#include <tchar.h>

#include "UniConv.h"
#include "Repository.h"
#include "TString.h"
#include "TomboURI.h"
#include "MemoNote.h" // for implimentation
#include "PasswordManager.h"

/////////////////////////////////////////
// Repository ctor & dtor, initializer
/////////////////////////////////////////

Repository::Repository() {}
Repository::~Repository() {}

/////////////////////////////////////////
// ctor & dtor, initializer
/////////////////////////////////////////

LocalFileRepository::LocalFileRepository() : pTopDir(NULL)
{
}

LocalFileRepository::~LocalFileRepository()
{
	delete[] pTopDir;
}

BOOL LocalFileRepository::Init(LPCTSTR pRoot)
{
	pTopDir = StringDup(pRoot);
	return TRUE;
}

#ifdef COMMENT
BOOL LocalFileRepository::Create(LPCTSTR pRequestHeadLine, TomboURI *pTemplate, TString *pRealHeadLine, TomboURI *pRealURI)
{
	return TRUE;
}
#endif


BOOL LocalFileRepository::Update(TomboURI *pCurrentURI, LPCTSTR pData, 
								 TomboURI *pNewURI, TString *pNewHeadLine)
{

	MemoNote *pNote = MemoNote::MemoNoteFactory(pCurrentURI);
	if (pNote == NULL) return FALSE;

	TString sHeadLine;
	if (!pNote->Save(pData, pNewHeadLine)) {
		delete pNote;
		return FALSE;
	}
	if (!pNote->GetURI(pNewURI)) {
		delete pNote;
		return FALSE;
	}
	return TRUE;
}

BOOL LocalFileRepository::GetHeadLine(const TomboURI *pURI, TString *pHeadLine)
{
	TomboURI *pThis = (TomboURI*)pURI;
	TomboURIItemIterator itr(pThis);
	if (!itr.Init()) return FALSE;

	if (!pHeadLine->Alloc(pURI->GetMaxPathItem() + 1)) return FALSE;
	_tcscpy(pHeadLine->Get(), TEXT("[root]"));

	LPCTSTR p;
	for (itr.First(); p = itr.Current(); itr.Next()) {
		_tcscpy(pHeadLine->Get(), p);
		if (itr.IsLeaf()) {
			DWORD n = _tcslen(pHeadLine->Get());
			if (n >= 4) {
				*(pHeadLine->Get() + n - 4) = TEXT('\0');
			}
		}
	}
	return TRUE;
}
