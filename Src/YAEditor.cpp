#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include "YAEditor.h"

#include "Tombo.h"
#include "UniConv.h"
#include "Region.h"
#include "YAEdit.h"
#include "YAEditDoc.h"
#include "YAEditView.h"
#include "MemoManager.h"
#include "MainFrame.h"

/////////////////////////////////////////////////////////////////////////////
// TomboDoc callback
/////////////////////////////////////////////////////////////////////////////

class TomboDocCallback : public YAEDocCallbackHandler {
	MemoManager *pMgr;
public:
	TomboDocCallback(MemoManager *pMgr);

	void OnModifyStatusChanged(YAEditDoc *pDoc, BOOL bOld, BOOL bNew);
};


TomboDocCallback::TomboDocCallback(MemoManager *p) : pMgr(p)
{
}

void TomboDocCallback::OnModifyStatusChanged(YAEditDoc *pDoc, BOOL bOld, BOOL bNew)
{
	pMgr->GetMainFrame()->SetModifyStatus(pDoc->IsModify());
}

/////////////////////////////////////////////////////////////////////////////
// TomboDoc callback
/////////////////////////////////////////////////////////////////////////////

class YAEDetailsViewCallback : public YAEditCallback {
	YAEditor *pEditor;
public:
	YAEDetailsViewCallback(YAEditor *pSelf) : pEditor(pSelf) {}

	void OnGetFocus();
};

void YAEDetailsViewCallback::OnGetFocus()
{
	pEditor->OnGetFocus();
}

/////////////////////////////////////////////////////////////////////////////
// YAEditor implimentation 
/////////////////////////////////////////////////////////////////////////////


YAEditor::YAEditor(MemoDetailsViewCallback *pCB) : MemoDetailsView(pCB), pEdit(NULL), pMemoMgr(NULL)
{
}

YAEditor::~YAEditor()
{
	delete pEdit;
}

BOOL YAEditor::Create(LPCTSTR pName, RECT &r, HWND hParent, HINSTANCE hInst, HFONT hFont)
{
	pEdit = new YAEdit(new YAEDetailsViewCallback(this));
	pEdit->Create(hInst, hParent, nID, r, NULL, new TomboDocCallback(pMemoMgr));
	pEdit->SetFont(hFont);
	return TRUE;
}

BOOL YAEditor::Init(MemoManager *pMgr, DWORD n)
{
	pMemoMgr = pMgr;
	nID = n;
	return TRUE;
}

void YAEditor::SetFocus()
{
	if (pEdit) pEdit->SetFocus();
}

void YAEditor::OnGetFocus()
{
	pCallback->GetFocusCallback(this);
}


LPTSTR YAEditor::GetMemo()
{
	DWORD nLen;
	char *pData = pEdit->GetDoc()->GetDocumentData(&nLen);
#if defined(PLATFORM_WIN32)
	return pData;
#else
	LPTSTR pDataW = ConvSJIS2Unicode(pData);
	delete [] pData;
	return pDataW;
#endif
}

void YAEditor::MoveWindow(DWORD x, DWORD y, DWORD nWidth, DWORD nHeight)
{
	if (pEdit) pEdit->ResizeWindow(x, y, nWidth, nHeight);
}

BOOL YAEditor::SetMemo(LPCTSTR pMemoW, DWORD nPos, BOOL bReadOnly)
{
	YAEditDoc *pDoc = new YAEditDoc();
#if defined(PLATFORM_WIN32)
	const char *pMemo = pMemoW;
#else
	char *pMemo = ConvUnicode2SJIS(pMemoW);
#endif
	if (!pDoc->Init(pMemo, pEdit, new TomboDocCallback(pMemoMgr))) return FALSE;

#if !defined(PLATFORM_WIN32)
	delete [] pMemo;
#endif

	YAEditDoc *pOldDoc = pEdit->SetDoc(pDoc);
	delete pOldDoc;

	return TRUE;
}

BOOL YAEditor::IsModify()
{
	return pEdit->GetDoc()->IsModify();
}

void YAEditor::ResetModify()
{
	pEdit->GetDoc()->SetModify(FALSE);
}

BOOL YAEditor::OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

void YAEditor::SetMDSearchFlg(BOOL bFlg)
{
	pCallback->SetSearchFlg(bFlg);
}

void YAEditor::SetFont(HFONT hFont)
{
	pEdit->SetFont(hFont);
}