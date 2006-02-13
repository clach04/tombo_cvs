#include <windows.h>
#include <tchar.h>
#include "Region.h"
#include "VarBuffer.h"
#include "YAEditDoc.h"
#include "LineWrapper.h"
#include "LineWrapperImpl.h"
#include "YAEdit.h"
#include "MemManager.h"
#include "TomboLib/UniConv.h"
#include "LineManager.h"
#include "PhysicalLineManager.h"
#include "StringSplitter.h"

/////////////////////////////////////////////////////////////////////////////
// ctor & dtor
/////////////////////////////////////////////////////////////////////////////

YAEditDoc::YAEditDoc() : pView(NULL), pPhLineMgr(NULL), pCallback(NULL)
{
}

YAEditDoc::~YAEditDoc()
{
	if (pPhLineMgr) delete pPhLineMgr;
}

BOOL YAEditDoc::Init(const char *pStr, YAEdit *pV, YAEditCallback*pCb)
{
	pCallback = pCb;
	pView = pV;
	if (!ReleaseDoc()) return FALSE;
	return LoadDoc(pStr);
}

/////////////////////////////////////////////////////////////////////////////
// load document
/////////////////////////////////////////////////////////////////////////////

BOOL YAEditDoc::LoadDoc(const char *pStr)
{
	ReleaseDoc();
	LPTSTR pStrT;
#ifdef _WIN32_WCE
	pStrT = ConvSJIS2Unicode(pStr);
#else
	pStrT = (LPTSTR)pStr;
#endif

	if (!pPhLineMgr->LoadDoc(pStrT)) return FALSE;
#ifdef _WIN32_WCE
	delete [] pStrT;
#endif

	SetModify(FALSE);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// save document
/////////////////////////////////////////////////////////////////////////////

char *YAEditDoc::GetDocumentData(LPDWORD pLen)
{
	return pPhLineMgr->GetDocumentData(pLen);
}

/////////////////////////////////////////////////////////////////////////////
// release document memory
/////////////////////////////////////////////////////////////////////////////

BOOL YAEditDoc::ReleaseDoc()
{
	// LineManager depends on MarkerAllocator, delete/new order is important.
	if (pPhLineMgr) delete pPhLineMgr;

	pPhLineMgr = new PhysicalLineManager();
	if (pPhLineMgr == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}
	if (!pPhLineMgr->Init()) return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Replace string
/////////////////////////////////////////////////////////////////////////////

BOOL YAEditDoc::ReplaceString(const Region *pDelRegion, LPCTSTR pString)
{
	DWORD nPhLinesBefore = pPhLineMgr->MaxLine();

	// delete region and insert string
	Region rNewRegion;
	DWORD nAffLines;
	if (!pPhLineMgr->ReplaceRegion(pDelRegion, pString, &nAffLines, &rNewRegion)) return FALSE;

	DWORD nPhLinesAfter = pPhLineMgr->MaxLine();

	if (!pView->UpdateNotify(pPhLineMgr, pDelRegion, &rNewRegion, nPhLinesBefore, nPhLinesAfter, nAffLines)) return FALSE;
	SetModify(TRUE);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// update modify status
/////////////////////////////////////////////////////////////////////////////

void YAEditDoc::SetModify(BOOL b)
{
	if (bModified == b) return;
	BOOL bOld = bModified;
	bModified = b;
	if (pCallback) pCallback->ChangeModifyStatusNotify(bModified);
}

/////////////////////////////////////////////////////////////////////////////
// update modify status
/////////////////////////////////////////////////////////////////////////////

DWORD YAEditDoc::GetDataBytes(const Region *pRegion)
{
	if (pRegion->posStart.row == pRegion->posEnd.row) {
		return pRegion->posEnd.col - pRegion->posStart.col;
	} else {
		DWORD nBytes = 0;
		LineInfo *pInfo;
		pInfo= pPhLineMgr->GetLineInfo(pRegion->posStart.row);

		// first line
		nBytes = pInfo->pLine->nUsed - pRegion->posStart.col + 1; // +1 means LF

		for (DWORD i = pRegion->posStart.row + 1; i < pRegion->posEnd.row; i++) {
			pInfo = pPhLineMgr->GetLineInfo(i);
			nBytes += pInfo->pLine->nUsed + 1;
		}

		// last line
		nBytes += pRegion->posEnd.col;

		return nBytes;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Convert data bytes to physical position
/////////////////////////////////////////////////////////////////////////////

void YAEditDoc::ConvertBytesToCoordinate(DWORD nPos, Coordinate *pPos)
{
	DWORD nBytes = 0;
	LineInfo *p = NULL;
	for (DWORD i = 0; i < pPhLineMgr->MaxLine(); i++) {
		p = pPhLineMgr->GetLineInfo(i);
		if (nBytes + p->pLine->nUsed + 1 > nPos) {
			break;
		}
		nBytes += p->pLine->nUsed + 1;
	}
	pPos->row = i;
	pPos->col = nPos - nBytes;
}
