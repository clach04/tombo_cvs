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

TomboEditDoc::TomboEditDoc() : pView(NULL), pPhLineMgr(NULL), pHandler(NULL)
{
}

TomboEditDoc::~TomboEditDoc()
{
	if (pPhLineMgr) delete pPhLineMgr;
}

BOOL TomboEditDoc::Init(const char *pStr, TomboEdit *pV, YAEDocCallbackHandler *pCb)
{
	pHandler = pCb;
	pView = pV;
	if (!ReleaseDoc()) return FALSE;
	return LoadDoc(pStr);
}

/////////////////////////////////////////////////////////////////////////////
// load document
/////////////////////////////////////////////////////////////////////////////

BOOL TomboEditDoc::LoadDoc(const char *pStr)
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

char *TomboEditDoc::GetDocumentData(LPDWORD pLen)
{
	return pPhLineMgr->GetDocumentData(pLen);
}

/////////////////////////////////////////////////////////////////////////////
// release document memory
/////////////////////////////////////////////////////////////////////////////

BOOL TomboEditDoc::ReleaseDoc()
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
// get line data
/////////////////////////////////////////////////////////////////////////////

BOOL TomboEditDoc::GetLineChunk(DWORD nLineNo, LineChunk *pChunk) 
{
	// retrieve line data info.
	if (!pView->GetLineMgr()->GetLineChunk(nLineNo, pChunk)) return FALSE;
	pChunk->SetSelRegion(&pView->SelectedRegion());
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// get previous position
/////////////////////////////////////////////////////////////////////////////
// In DBCS char set, it is difficult to detmine previous char. It may be -1, or may be -2.
// In Unicode(UCS-2), this is simply -1 letter(2 bytes).

DWORD TomboEditDoc::GetPrevOffset(DWORD n, DWORD nPos)
{
#if defined(PLATFORM_WIN32)
	LineChunk lc;
	if (!pView->GetLineMgr()->GetLineChunk(n, &lc)) return FALSE;

	const char *p = lc.GetLineData();
	if (!p) return 0;

	const char *r = p + nPos;
	const char *q = p;
	const char *pPrevChar = NULL;
	while(*q && r > q) {
		pPrevChar = q;
		if (IsDBCSLeadByte(*q)) {
			q++;
		}
		q++;
	}
	return pPrevChar ? nPos - (pPrevChar - p) : 0;
#else
	return 1;
#endif
}

/////////////////////////////////////////////////////////////////////////////
// Replace string
/////////////////////////////////////////////////////////////////////////////

BOOL TomboEditDoc::ReplaceString(const Region *pDelRegion, LPCTSTR pString)
{
	Region r = *pDelRegion;

	pView->ClearSelectedRegion();

	DWORD nAffLines;
	// delete region and insert string
	DWORD nPhLinesBefore = pPhLineMgr->MaxLine();
	if (!pPhLineMgr->ReplaceRegion(&r, pString, &nAffLines)) return FALSE;
	DWORD nPhLinesAfter = pPhLineMgr->MaxLine();

	if (!pView->UpdateNotify(pPhLineMgr, &r, nPhLinesBefore, nPhLinesAfter, nAffLines)) return FALSE;
	SetModify(TRUE);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// update modify status
/////////////////////////////////////////////////////////////////////////////

void TomboEditDoc::SetModify(BOOL b)
{
	if (bModified == b) return;
	BOOL bOld = bModified;
	bModified = b; 
	if (pHandler) pHandler->OnModifyStatusChanged(this, bOld, bModified);
}
