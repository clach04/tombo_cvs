#include <windows.h>
#include <tchar.h>
#include "Tombo.h"
#include "UniConv.h"
#include "TString.h"
#include "TomboURI.h"
#include "VarBuffer.h"
#include "URIScanner.h"
#include "Repository.h"

#define URILIST_INITSIZE 20
#define URILIST_DELTASIZE 10

/////////////////////////////////////////////////
//////////////////////////////////////////////////
// URIList implementation
//////////////////////////////////////////////////
//////////////////////////////////////////////////

URIList::URIList()
{
}

URIList::~URIList()
{
	for (DWORD i = 0; i < liList.NumItems(); i++) {
		URIListItem *pItem = liList.GetUnit(i);
		delete pItem->pURI;
		delete [] pItem->pTitle;
	}
}

BOOL URIList::Init()
{
	if (!liList.Init(URILIST_INITSIZE, URILIST_DELTASIZE)) return FALSE;
	return TRUE;
}

BOOL URIList::Add(const TomboURI *pURI, LPCTSTR pTitle)
{
	URIListItem item;
	if (pURI) {
		item.pURI = new TomboURI(*pURI);
		if (item.pURI == NULL) { SetLastError(ERROR_NOT_ENOUGH_MEMORY); return FALSE; }
	}

	if (pTitle) {
		item.pTitle = StringDup(pTitle); 
		if (item.pTitle == NULL) return FALSE;
	}

	if (!liList.Add(&item)) return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////
//////////////////////////////////////////////////
// URIScanner implementation
//////////////////////////////////////////////////
//////////////////////////////////////////////////

//////////////////////////////////////////////////
// ctor & Initialize
//////////////////////////////////////////////////

URIScanner::URIScanner() : pBaseURI(NULL), pCurrentURI(NULL), pTop(NULL), pBaseTitle(NULL)
{
}

BOOL URIScanner::Init(IEnumRepository *pRepo, const TomboURI *pURI, BOOL bSKE)
{
	pBaseURI = new TomboURI(*pURI);
	pRepository = pRepo;
	bSkipEncrypt = bSKE;
	return TRUE;
}

//////////////////////////////////////////////////
// dtor
//////////////////////////////////////////////////

URIScanner::~URIScanner()
{
	ClearStack();
	delete pBaseURI;
	delete pBaseTitle;
}

//////////////////////////////////////////////////
// Clear stack frame
//////////////////////////////////////////////////

void URIScanner::ClearStack()
{
	while(pTop) {
		LeaveFrame();
	}
}

void URIScanner::LeaveFrame()
{
	StackFrame *pSF = pTop;
	pTop = pSF->pNext;

	delete pSF->pList;
	delete pSF;
}
//////////////////////////////////////////////////
// cutomisable methods
//////////////////////////////////////////////////

void URIScanner::InitialScan() {}
void URIScanner::AfterScan() {}
void URIScanner::PreFolder() {}
void URIScanner::PostFolder() {}
void URIScanner::Node() {}

//////////////////////////////////////////////////
// Scan repository
//////////////////////////////////////////////////

BOOL URIScanner::Scan()
{
	bStopScan = FALSE;

	pBaseTitle = new TString();
	if (!pRepository->GetHeadLine(pBaseURI, pBaseTitle)) return FALSE;

	// set marker
	pCurrentURI = pBaseURI;
	pTitle = pBaseTitle->Get();

	// check URI
	URIOption opt(NOTE_OPTIONMASK_VALID);
	if (!pRepository->GetOption(pCurrentURI, &opt)) return FALSE;

	if (!opt.bFolder) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	// call initialize method
	InitialScan();

	if (!PushFrame(pCurrentURI)) return FALSE;
	if (bStopScan) {
		AfterScan();
		return TRUE;
	}
	PreFolder();

	while(pTop) {
		// enum current frame
		while (pTop->nPos < pTop->pList->GetSize()) {

			if (bStopScan) break;

			// update marker
			pCurrentURI = pTop->pList->GetURI(pTop->nPos);
			pTitle = pTop->pList->GetTitle(pTop->nPos);

			// check folder or not
			URIOption opt(NOTE_OPTIONMASK_VALID);
			if (!pRepository->GetOption(pCurrentURI, &opt)) return FALSE;

			if (opt.bFolder) {
				// create new frame
				if (!PushFrame(pCurrentURI)) return FALSE;
				if (bStopScan) break;
				PreFolder();
				continue;
			} else {
				Node();
			}
			pTop->nPos++;
		}

		// when enumeration finished, 
		LeaveFrame();
		if (pTop) {
			pCurrentURI = pTop->pList->GetURI(pTop->nPos);
			pTitle = pTop->pList->GetTitle(pTop->nPos);
		} else {
			pCurrentURI = pBaseURI;
			pTitle = pBaseTitle->Get();
		}
		PostFolder();
		if (pTop) {
			pTop->nPos++;
		}
	}

	// call finalize method
	AfterScan();

	return TRUE;
}

//////////////////////////////////////////////////
//
//////////////////////////////////////////////////

BOOL URIScanner::PushFrame(const TomboURI *pURI)
{
	StackFrame *pSF = new StackFrame();
	if (pSF == NULL) { SetLastError(ERROR_NOT_ENOUGH_MEMORY); return FALSE; }

	pSF->pList = pRepository->GetChild(pCurrentURI, bSkipEncrypt);
	if (pSF->pList == NULL) {
		DWORD n = GetLastError();
		if (GetLastError() == ERROR_CANCELLED) {
			StopScan();
			delete pSF;
			return TRUE;
		}
		return FALSE;
	}
	pSF->nPos = 0;

	pSF->pNext = pTop;
	pTop = pSF;

	return TRUE;
}

