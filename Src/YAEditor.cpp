#include <windows.h>
#include <tchar.h>
#include "YAEditor.h"

#include "Tombo.h"
#include "Region.h"
#include "YAEdit.h"
#include "YAEditDoc.h"
#include "YAEditView.h"
#include "MemoManager.h"
#include "MainFrame.h"

class TomboDocCallback : public YAEDocCallbackHandler {
	MemoManager *pMgr;
public:
	TomboDocCallback(MemoManager *pMgr);

	void OnModifyStatusChanged(TomboEditDoc *pDoc, BOOL bOld, BOOL bNew);
};

TomboDocCallback::TomboDocCallback(MemoManager *p) : pMgr(p)
{
}

void TomboDocCallback::OnModifyStatusChanged(TomboEditDoc *pDoc, BOOL bOld, BOOL bNew)
{
	pMgr->GetMainFrame()->SetModifyStatus(pDoc->IsModify());
}



YAEditor::YAEditor() : pEdit(NULL), pMemoMgr(NULL)
{
}

YAEditor::~YAEditor()
{
	delete pEdit;
}

BOOL YAEditor::Create(LPCTSTR pName, RECT &r, HWND hParent, HINSTANCE hInst, HFONT hFont)
{
	pEdit = new TomboEdit();
	pEdit->Create(hInst, hParent, nID, r, NULL, new TomboDocCallback(pMemoMgr));
//	pEdit->SetFocus();
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

LPTSTR YAEditor::GetMemo()
{
	DWORD nLen;
	char *pData = pEdit->GetDoc()->GetDocumentData(&nLen);
	return pData;
}

void YAEditor::MoveWindow(DWORD x, DWORD y, DWORD nWidth, DWORD nHeight)
{
	if (pEdit) pEdit->ResizeWindow(x, y, nWidth, nHeight);
}

BOOL YAEditor::SetMemo(LPCTSTR pMemo, DWORD nPos, BOOL bReadOnly)
{
	TomboEditDoc *pDoc = new TomboEditDoc();
	if (!pDoc->Init(pMemo, pEdit, new TomboDocCallback(pMemoMgr))) return FALSE;

	TomboEditDoc *pOldDoc = pEdit->SetDoc(pDoc);
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