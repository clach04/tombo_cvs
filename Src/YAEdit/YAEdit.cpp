#include <windows.h>
#include <tchar.h>
#include <imm.h>
#if defined(PLATFORM_WIN32)
#include <zmouse.h>
#endif
#include "resource.h"
#include "Region.h"
#include "YAEdit.h"
#include "YAEditView.h"
#include "YAEditDoc.h"
#include "VarBuffer.h"
#include "LineManager.h"
#include "LineWrapper.h"
#include "LineWrapperImpl.h"
#include "Logger.h"
#include "FontWidthCache.h"
#include "TomboLib/UniConv.h"
#include "TomboLib/Clipboard.h"
#include "TString.h"

/////////////////////////////////////////////////////////////////////////////
// constatnt definitions
/////////////////////////////////////////////////////////////////////////////

#define TOMBOEDIT_CLASS_NAME TEXT("TomboEditCtl")

#define CHARA_CTRL_C 3
#define CHARA_CTRL_V 22
#define CHARA_CTRL_X 24
#define CHARA_BS 8
#define CHARA_ENTER 13

/////////////////////////////////////////////////////////////////////////////
// static funcs/vars declarations
/////////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK TomboEditWndProc(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////
// Regist window class
/////////////////////////////////////////////////////////////////////////////

BOOL TomboEdit::RegisterClass(HINSTANCE hInst)
{
	WNDCLASS wc;

	wc.style = 0;
	wc.lpfnWndProc = (WNDPROC)TomboEditWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(LONG);
	wc.hInstance = hInst;
	wc.hIcon = NULL;
#ifdef _WIN32_WCE
	wc.hCursor = NULL;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
#else
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
#endif
	wc.lpszMenuName = NULL;
	wc.lpszClassName = TOMBOEDIT_CLASS_NAME;
	::RegisterClass(&wc);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// ctor & dtor
/////////////////////////////////////////////////////////////////////////////

TomboEdit::TomboEdit() : pWrapper(NULL), bScrollTimerOn(FALSE), pView(NULL), bMouseDown(FALSE), pLineMgr(NULL), pHandler(NULL)
{
}

TomboEdit::~TomboEdit()
{
	if (pView) {
		if (pView->hViewWnd) DestroyWindow(pView->hViewWnd);
	}
	if (pLineMgr) delete pLineMgr;
}

/////////////////////////////////////////////////////////////////////////////
// Event handler
/////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK TomboEditWndProc(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	if (nMessage == WM_CREATE) {
		LPCREATESTRUCT pCS = (LPCREATESTRUCT)lParam;
		TomboEdit *frm = (TomboEdit*)pCS->lpCreateParams;
		SetWindowLong(hWnd, 0, (LONG)frm);
		frm->OnCreate(hWnd, wParam, lParam);
		return 0;
	}

	TomboEdit *frm = (TomboEdit*)GetWindowLong(hWnd, 0);
	if (frm == NULL) {
		return DefWindowProc(hWnd, nMessage, wParam, lParam);
	}

	switch(nMessage) {
	case WM_KEYDOWN:
		if (frm->OnKeyDown(hWnd, wParam, lParam)) {
			return 0;
		}
		break;
	case WM_CHAR:
		frm->OnChar(hWnd, wParam, lParam);
		return 0;
	case WM_DESTROY:
		return 0;
	case WM_PAINT:
		frm->OnPaint(hWnd, wParam, lParam);
		return 0;
	case WM_VSCROLL:
		frm->OnVScroll(hWnd, wParam, lParam);
		return 0;
	case WM_HSCROLL:
		frm->OnHScroll(hWnd, wParam, lParam);
		return 0;
	case WM_LBUTTONDOWN:
		frm->OnLButtonDown(hWnd, wParam, lParam);
		break;
	case WM_MOUSEMOVE:
		frm->OnMouseMove(hWnd, wParam, lParam);
		break;
	case WM_LBUTTONUP:
		frm->OnLButtonUp(hWnd, wParam, lParam);
		break;
	case WM_SETFOCUS:
		frm->OnSetFocus();
		return 0;
	case WM_KILLFOCUS:
		frm->OnKillFocus();
		return 0;
	case WM_TIMER:
		frm->OnTimer(hWnd, wParam, lParam);
		return 0;
#if defined(PLATFORM_WIN32)
	case WM_MOUSEWHEEL:
		frm->OnMouseWheel(hWnd, wParam, lParam);
		return 0;
#endif
//	case WM_IME_COMPOSITION:
//		frm->OnIMEComposition(hWnd, wParam, lParam);
//		return 0;
	}
	return DefWindowProc(hWnd, nMessage, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////
// Create window
/////////////////////////////////////////////////////////////////////////////

BOOL TomboEdit::Create(HINSTANCE hInst, HWND hParent, DWORD nId, RECT &r, YAECallbackHandler *pCb, YAEDocCallbackHandler *pDocCb)
{
	pHandler = pCb;
	hInstance = hInst;
	pDoc = NULL;

	pView = new TomboEditView(this);
	if (!pView->Init()) return FALSE;

	FixedPixelLineWrapper *pWw = new FixedPixelLineWrapper();
	if (pWw == NULL || !pWw->Init(this)) return FALSE;
	pWrapper = pWw;

	pDoc = new TomboEditDoc(); 
	if (!pDoc->Init("", this, pDocCb)) return FALSE;

	
#if defined(PLATFORM_WIN32)
	pView->hViewWnd = CreateWindowEx(WS_EX_CLIENTEDGE, TOMBOEDIT_CLASS_NAME, TEXT(""),
						WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL,
						r.left,
						r.top,
						r.right,
						r.bottom,
						hParent,
						(HMENU)nId,
						hInst,
						this);
#else
	pView->hViewWnd = CreateWindow(TOMBOEDIT_CLASS_NAME, TEXT(""),
						WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL,
						r.left,
						r.top,
						r.right,
						r.bottom,
						hParent,
						(HMENU)nId,
						hInst,
						this);
#endif

	if (pView->hViewWnd == NULL) return FALSE;

#if defined(PLATFORM_WIN32)
	// clear buffer
	aKeyBuffer[0] = aKeyBuffer[1] = '\0';
#endif

	rSelRegion.posStart.Set(0, 0);
	rSelRegion.posEnd.Set(0, 0);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// WM_CREATE handler
/////////////////////////////////////////////////////////////////////////////
// In this function, hViewWnd are not initialized yet.

void TomboEdit::OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	pView->OnCreate(hWnd, wParam, lParam);
	pWrapper->SetViewWidth(pView->rClientRect.right - pView->rClientRect.left - pView->nMaxCharWidth);

//	pDoc = new TomboEditDoc(); 
//	if (!pDoc->Init("", this)) return;

	pLineMgr = new LineManager();
	if (!pLineMgr->Init(this)) return;

	// associate with default(empty) document.
	// Since memory allocation check, object is created in Create(), and assoicated here.
	SetDoc(pDoc);
}

/////////////////////////////////////////////////////////////////////////////
// WM_VSCROLL handler
/////////////////////////////////////////////////////////////////////////////

void TomboEdit::OnVScroll(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int nScrollCode = LOWORD(wParam);
	switch(nScrollCode) {
	case SB_LINEDOWN:
		pView->Next1L();
		break;
	case SB_LINEUP:
		pView->Prev1L();
		break;
	case SB_PAGEDOWN:
		pView->NextPage();
		break;
	case SB_PAGEUP:
		pView->PrevPage();
		break;
	case SB_THUMBTRACK:
		pView->SetScrollVertPos(HIWORD(wParam), TRUE);
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
// WM_HSCROLL handler
/////////////////////////////////////////////////////////////////////////////

void TomboEdit::OnHScroll(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int nScrollCode = LOWORD(wParam);
	switch(nScrollCode) {
	case SB_LINELEFT:
		pView->ScrollLeft1L();
		break;
	case SB_LINERIGHT:
		pView->ScrollRight1L();
		break;
	case SB_PAGELEFT:
		pView->ScrollLeft1P();
		break;
	case SB_PAGERIGHT:
		pView->ScrollRight1P();
		break;
	case SB_THUMBTRACK:
		{
			int nPos = HIWORD(wParam); 
			pView->SetScrollHorizPos(nPos);
		}
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
// WM_MOUSEWHEEL
/////////////////////////////////////////////////////////////////////////////
#if defined(PLATFORM_WIN32)
void TomboEdit::OnMouseWheel(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	short delta = ((short) HIWORD(wParam))/WHEEL_DELTA;
	pView->SetScrollVertByOffset(-delta * 2);
}
#endif

/////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Line drawing related members
//////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

void TomboEdit::OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC = BeginPaint(hWnd, &ps);
	if (pDoc == NULL) { 
		// Usually, pDoc has some value. this check is for safety.
		EndPaint(hWnd, &ps);
		return;
	}
	pView->PaintRect(hDC, ps.rcPaint);
	EndPaint(hWnd, &ps);
}

void TomboEdit::RequestRedraw(DWORD nLineNo, WORD nLeftPos, BOOL bToBottom) { pView->RequestRedraw(nLineNo, nLeftPos, bToBottom); }
void TomboEdit::RequestRedrawRegion(const Region *pRegion) { pView->RequestRedrawRegion(pRegion); }

/////////////////////////////////////////////////////////////////////////////
// FOCUS
/////////////////////////////////////////////////////////////////////////////

void TomboEdit::OnSetFocus()  { if (pView) pView->OnSetFocus();  }
void TomboEdit::OnKillFocus() { if (pView) pView->OnKillFocus(); }

/////////////////////////////////////////////////////////////////////////////
// WM_KEYDOWN
/////////////////////////////////////////////////////////////////////////////
BOOL TomboEdit::OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int nVertKey = (int)wParam;
	BOOL bShiftDown = (GetKeyState(VK_SHIFT) & 0x8000) != 0;

	if (bShiftDown) {
		switch (nVertKey) {
		case VK_RIGHT:
			{
				pView->MoveRight();

				Coordinate nSelNow;
				nSelNow.Set(pView->nCursorCol, pView->nCursorRow);
				Coordinate nSelOld;

				Region r;
				SelectRegion(nSelNow, &nSelOld);
				if (nSelNow < nSelOld) {
					r.posStart = nSelNow; r.posEnd = nSelOld;
				} else {
					r.posStart = nSelOld; r.posEnd = nSelNow;
				}
				RequestRedrawRegion(&r);

			}
			break;
		case VK_LEFT:
			{
				pView->MoveLeft();

				Coordinate nSelNow;
				nSelNow.Set(pView->nCursorCol, pView->nCursorRow);
				Coordinate nSelOld;

				Region r;
				SelectRegion(nSelNow, &nSelOld);
				if (nSelNow < nSelOld) {
					r.posStart = nSelNow; r.posEnd = nSelOld;
				} else {
					r.posStart = nSelOld; r.posEnd = nSelNow;
				}
				RequestRedrawRegion(&r);
			}
			break;
		case VK_UP:
			{
				pView->MoveUp();

				Coordinate nSelNow;
				nSelNow.Set(pView->nCursorCol, pView->nCursorRow);
				Coordinate nSelOld;

				Region r;
				SelectRegion(nSelNow, &nSelOld);
				if (nSelNow < nSelOld) {
					r.posStart = nSelNow; r.posEnd = nSelOld;
				} else {
					r.posStart = nSelOld; r.posEnd = nSelNow;
				}
				RequestRedrawRegion(&r);
			}
			break;
		case VK_DOWN:
			{
				pView->MoveDown();

				Coordinate nSelNow;
				nSelNow.Set(pView->nCursorCol, pView->nCursorRow);
				Coordinate nSelOld;

				Region r;
				SelectRegion(nSelNow, &nSelOld);
				if (nSelNow < nSelOld) {
					r.posStart = nSelNow; r.posEnd = nSelOld;
				} else {
					r.posStart = nSelOld; r.posEnd = nSelNow;
				}
				RequestRedrawRegion(&r);
			}
			break;
		}
	} else {
		switch (nVertKey) {
		case VK_DELETE:
			DeleteKeyDown();
			break;
		case VK_RIGHT:
			MoveRight();
			break;
		case VK_LEFT:
			MoveLeft();
			break;
		case VK_UP:
			MoveUp();
			break;
		case VK_DOWN:
			MoveDown();
			break;
		case VK_PRIOR:
			pView->PrevPage();
			return TRUE;
		case VK_NEXT:
			pView->NextPage();
			return TRUE;
		case VK_HOME:
			pView->MoveTOL();
			break;
		case VK_END:
			pView->MoveEOL();
			break;
		default:
			return FALSE;
		}
	}

	pView->ScrollCaret();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// WM_CHAR
/////////////////////////////////////////////////////////////////////////////

void TomboEdit::OnChar(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	TCHAR ch = (TCHAR)wParam;
	if (ch == CHARA_BS) {
		KeyBS();
		return;
	}
	if (ch == CHARA_ENTER) {
		DeleteRegion();
		InsertLine(TEXT("\n"));

		// check and update vert lines.
		pView->ResetScrollbar();
		MoveRight();

		return;
	}
	if (ch == CHARA_CTRL_C) {
		if (!CopyToClipboard()) {
			MessageBox(hWnd, TEXT("Copy to clipboard failed."), TEXT("ERROR"), MB_ICONWARNING | MB_OK);
		}
		return;
	}
	if (ch == CHARA_CTRL_X) {
		if (!CopyToClipboard()) {
			MessageBox(hWnd, TEXT("Copy to clipboard failed."), TEXT("ERROR"), MB_ICONWARNING | MB_OK);
		}
		DeleteRegion();
		return;
	}
	if (ch == CHARA_CTRL_V) {
		if (!InsertFromClipboard()) {
			MessageBox(hWnd, TEXT("Paste from clipboard failed."), TEXT("ERROR"), MB_ICONWARNING | MB_OK);
		}
		return;
	}
#if defined(PLATFORM_WIN32)
	// if char is DBCS lead byte, buffering.
	if (aKeyBuffer[0] != '\0') {
		aKeyBuffer[1] = ch;
		aKeyBuffer[2] = TEXT('\0');
		DeleteRegion();
		InsertLine(aKeyBuffer);
		aKeyBuffer[0] = aKeyBuffer[1] = '\0';
		return;
	} else {
		if (IsDBCSLeadByte(ch)) {
			aKeyBuffer[0] = ch;
			return;
		}
	}
#endif
	DeleteRegion();
	TCHAR kbuf[2];
	kbuf[0] = ch; kbuf[1] = TEXT('\0');
	InsertLine(kbuf);
}

void TomboEdit::KeyBS()
{
	if (SelectedRegion().posStart != SelectedRegion().posEnd) {
		// selection area exists

		Region r;
		r.posStart = SelectedRegion().posStart;

		DWORD nPreLines = pLineMgr->MaxLine();

		DeleteRegion();

//		if (nPreLines == pLineMgr->MaxLine()) {
//			GetEndPhysicalPos(SelectedRegion().posEnd.row, &(r.posEnd));
//		} else {
//			GetEndPhysicalPos(pLineMgr->MaxLine(), &(r.posEnd));
//		}
//		pView->RequestRedrawRegion(&r);

//		pView->SetCaretPosition(Coordinate(r.posStart.col, r.posStart.row));

	} else if (pView->nCursorCol == 0) {
		// not selected and cursor is top of line

		if (pView->nCursorRow > 0) {
			LineChunk lc;
			if (!pDoc->GetLineChunk(pView->nCursorRow - 1, &lc)) return;
			if (lc.IsContLine()) {
				MoveLeft();
				MoveLeft();
				DeleteKeyDown();
			} else {
				// Join to previous line
				MoveLeft();
				JoinLine(pView->nCursorRow);
				pView->ResetScrollbar();
			}
		}
	} else {
		// other case

		// check if caret places at the end of the line 
		BOOL bAtTheEOL = FALSE;
		LineChunk lc;
		if (!pDoc->GetLineChunk(pView->nCursorRow, &lc)) return;
		if (pView->nCursorCol == lc.LineLen()) {
			bAtTheEOL = TRUE;
		}

		MoveLeft();
		DeleteKeyDown();

		if (bAtTheEOL && pView->nCursorCol == 0) {
			MoveLeft();
		}

	}
}

/////////////////////////////////////////////////////////////////////////////
// move cursor
/////////////////////////////////////////////////////////////////////////////

void TomboEdit::MoveRight() { pView->MoveRight(); ClearRegion(); }
void TomboEdit::MoveLeft()  { pView->MoveLeft();  ClearRegion(); }
void TomboEdit::MoveUp()    { pView->MoveUp();    ClearRegion(); }
void TomboEdit::MoveEOL()   { pView->MoveEOL();   ClearRegion(); }
void TomboEdit::MoveTOL()   { pView->MoveTOL();   ClearRegion(); }
void TomboEdit::MoveDown()  { pView->MoveDown();  ClearRegion(); }

/////////////////////////////////////////////////////////////////////////////
// WM_LBUTTONDOWN
/////////////////////////////////////////////////////////////////////////////

void TomboEdit::OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD nMouseDrgStartX, nMouseDrgStartY;	// LButton down point by WM_LBUTTONDOWN

	SetFocus();

	nMouseDrgStartX = LOWORD(lParam);
	nMouseDrgStartY = HIWORD(lParam);

	// move caret
	DWORD nNewRow = pView->DpLinePixelToLgLineNo(nMouseDrgStartY);
	if (nNewRow < pLineMgr->MaxLine()) {
		pView->SetNearCursorPos(nMouseDrgStartX, nNewRow);

		Region rOldRgn = SelectedRegion();
		// set new region;
		Coordinate nSelDown;
		nSelDown.Set(pView->nCursorCol, pView->nCursorRow);
		SetMark(nSelDown);

		// crear previously selected region
		RequestRedrawRegion(&rOldRgn);
	}

	// get mouse capture
	SetCapture(hWnd);
	bMouseDown = TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//  Mouse move
/////////////////////////////////////////////////////////////////////////////

#ifndef MAKEPOINTS
#define MAKEPOINTS(l)   (*((POINTS FAR *) & (l))) 
#endif

void TomboEdit::SetSelectionFromPoint(int xPos, int yPos)
{
	// when SetCaptured, cursor pos may be negative.
	if (xPos < 0) xPos = 0;
	if (yPos < 0) {
		pView->Prev1L();
		yPos = 0;
	}
	if (xPos > pView->rClientRect.right) xPos = pView->rClientRect.right - 1;
	if (yPos > pView->rClientRect.bottom) {
		yPos = pView->rClientRect.bottom - 1;
		pView->Next1L();
	}

	// move cursor
	DWORD nNewRow = pView->DpLinePixelToLgLineNo(yPos);
	if (nNewRow < pLineMgr->MaxLine()) {
		pView->SetNearCursorPos(xPos, nNewRow);

		Coordinate nSelNow;
		nSelNow.Set(pView->nCursorCol, pView->nCursorRow);
		Coordinate nSelOld;

		Region r;
		SelectRegion(nSelNow, &nSelOld);
		if (nSelNow < nSelOld) {
			r.posStart = nSelNow; r.posEnd = nSelOld;
		} else {
			r.posStart = nSelOld; r.posEnd = nSelNow;
		}
		RequestRedrawRegion(&r);
	}
}

void TomboEdit::OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (!bMouseDown) return;

	POINTS ptPos;
	ptPos = MAKEPOINTS(lParam);
	int xPos = ptPos.x;
	int yPos = ptPos.y;

	if (xPos < 0 || yPos < 0 || xPos > pView->rClientRect.right || yPos > pView->rClientRect.bottom) {
		if (!bScrollTimerOn) {
			SetTimer(hWnd, IDT_SELSCROLL, 50, NULL);
			bScrollTimerOn = TRUE;
			ptMousePos.x = ptPos.x;
			ptMousePos.y = ptPos.y;
		}
	} else {
		if (bScrollTimerOn) {
			KillTimer(hWnd, IDT_SELSCROLL);
			bScrollTimerOn = FALSE;
		}
	}
	SetSelectionFromPoint(xPos, yPos);
}

/////////////////////////////////////////////////////////////////////////////
// WM_LBUTTONUP
/////////////////////////////////////////////////////////////////////////////

void TomboEdit::OnLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (!bMouseDown) return;

	// kill scroll timer
	if (bScrollTimerOn) {
		KillTimer(hWnd, IDT_SELSCROLL);
		bScrollTimerOn = FALSE;
	}

	// release mouse capture
	if (GetCapture() == hWnd) ReleaseCapture();
	bMouseDown = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// WM_TIMER
/////////////////////////////////////////////////////////////////////////////

void TomboEdit::OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch(wParam) {
	case IDT_SELSCROLL:
		SetSelectionFromPoint(ptMousePos.x, ptMousePos.y);
		break;
	}
}


/////////////////////////////////////////////////////////////////////////////
// Set new document
/////////////////////////////////////////////////////////////////////////////

TomboEditDoc *TomboEdit::SetDoc(TomboEditDoc *pNewDoc)
{
	TomboEditDoc *pOldDoc = pDoc;
	pDoc = pNewDoc;

	pView->Init();

	bWrapText = TRUE;

	if (bWrapText) {
		pView->nMaxWidthPixel = pWrapper->GetViewWidth();
	} else {
		pView->GetMaxLineWidth();
		pWrapper->SetViewWidth(pView->nMaxWidthPixel);
	}

	pLineMgr->Reset();
	pLineMgr->RecalcWrap(pWrapper);

	pView->ResetScrollbar();
	pView->RedrawAllScreen();
	return pOldDoc;
}

/////////////////////////////////////////////////////////////////////////////
// get max line width for decide hscroll range
/////////////////////////////////////////////////////////////////////////////

DWORD TomboEdit::GetLineWidth(DWORD nOffset, LPCTSTR pStr, DWORD nLen) { return pView->GetLineWidth(nOffset, pStr, nLen); }

/////////////////////////////////////////////////////////////////////////////
// DEL key handler
/////////////////////////////////////////////////////////////////////////////

void TomboEdit::DeleteKeyDown()
{
	LineChunk lc;
	if (!pDoc->GetLineChunk(pView->nCursorRow, &lc)) return;
	DWORD nLineChar = lc.LineLen();

	if (pView->nCursorCol != nLineChar || lc.IsContLine()) {
		// delete 1 char
		DWORD nMax = pLineMgr->MaxLine();
		////////////////////////////////

		DWORD nDelChar = 1;
#ifdef PLATFORM_WIN32
		if (IsDBCSLeadByte(*(lc.GetLineData() + pView->nCursorCol))) {
			nDelChar = 2;
		}
#endif
		Coordinate c(pView->nCursorCol, pView->nCursorRow);
		Region rDel;
		LogicalPosToPhysicalPos(&c, &(rDel.posStart));
		rDel.posEnd = rDel.posStart;
		rDel.posEnd.col += nDelChar;	
		pDoc->ReplaceString(&rDel, TEXT(""));

		if (nMax != pLineMgr->MaxLine()) {
			pView->ResetScrollbar();
		}
	} else {
		JoinLine(pView->nCursorRow);
		pView->ResetScrollbar();
	}
}

/////////////////////////////////////////////////////////////////////////////
// Resize window
/////////////////////////////////////////////////////////////////////////////

void TomboEdit::ResizeWindow(int x, int y, int width, int height) 
{
	MoveWindow(pView->hViewWnd, x, y, width, height, FALSE);

	pView->ResetParam();

	pWrapper->SetViewWidth(pView->rClientRect.right - pView->rClientRect.left - pView->nMaxCharWidth);

	// Preserve absolute cursor position before rewrapping.
	DWORD nPhLineNo, nPhLinePos;
	LogicalCursorPosToPhysicalCursorPos(pView->nCursorRow, pView->nCursorCol, &nPhLineNo, &nPhLinePos);

	// Rewrapping. Logical line will be changed.
	pLineMgr->RecalcWrap(pWrapper);
	pView->ResetScrollbar();

	DWORD nNewCursorRow, nNewCursorCol;

	// get cursor position after rewrapping.
	PhysicalCursorPosToLogicalCursorPos(nPhLineNo, nPhLinePos, &nNewCursorRow, &nNewCursorCol);

	// set new cursor position
	pView->nCursorRow = nNewCursorRow;
	LineChunk lc;
	if (!pDoc->GetLineChunk(pView->nCursorRow, &lc)) return;
	DWORD n = pView->GetLineWidth(0, lc.GetLineData(), nNewCursorCol);
	pView->nCursorCol = nNewCursorCol;
	pView->nCursorColPos = n;
	
	pView->SetCaretPos();

	// redraw screen
	pView->RedrawAllScreen();
}

/////////////////////////////////////////////////////////////////////////////
// Region
/////////////////////////////////////////////////////////////////////////////
void TomboEdit::SetFocus()
{
	::SetFocus(pView->hViewWnd);
}

/////////////////////////////////////////////////////////////////////////////
// Region
/////////////////////////////////////////////////////////////////////////////

BOOL TomboEdit::DeleteRegion()
{
	if (!IsRegionSelected()) return TRUE;

	Region rRegion;
	rRegion.posStart = SelectedRegion().posStart;
	rRegion.posEnd = SelectedRegion().posEnd;

	Region rDelRgn;
	LogicalPosToPhysicalPos(&(SelectedRegion().posStart), &(rDelRgn.posStart));
	LogicalPosToPhysicalPos(&(SelectedRegion().posEnd), &(rDelRgn.posEnd));
	pDoc->ReplaceString(&rDelRgn, TEXT(""));


	ClearSelectedRegion();

	pView->nCursorRow = rRegion.posStart.row;
	pView->nCursorCol = rRegion.posStart.col;
	LineChunk lc;
	pDoc->GetLineChunk(pView->nCursorRow, &lc);
	pView->nCursorColPos = pView->GetLineWidth(pView->nCursorRow, lc.GetLineData(), pView->nCursorCol);
	pView->SetCaretPos();

	return TRUE;
}

void TomboEdit::ClearRegion()
{
	Region r = SelectedRegion();
	RequestRedrawRegion(&r);
	ClearSelectedRegion();
}

void TomboEdit::ClearSelectedRegion()
{
	rSelRegion.posEnd.Set(pView->nCursorCol, pView->nCursorRow); 
	rSelRegion.posStart = rSelRegion.posEnd;
}

/////////////////////////////////////////////////////////////////////////////
// Clipboard
/////////////////////////////////////////////////////////////////////////////

BOOL TomboEdit::CopyToClipboard()
{
	Tombo_Lib::Clipboard cb;

	// Get region data
	DWORD nRgnSize = GetRegionSize();
	if (nRgnSize == 0) return TRUE;
	TString sRgnStr;
	if (!sRgnStr.Alloc(nRgnSize + 1)) return FALSE;
	if (!GetRegionString(sRgnStr.Get())) return FALSE;

	// Set data to clipboard
	if (!cb.Open(pView->hViewWnd)) return FALSE;
	if (!cb.SetText(sRgnStr.Get())) return FALSE;
	cb.Close();

	return TRUE;
}

BOOL TomboEdit::InsertFromClipboard()
{
	// Get data from clipboard
	Tombo_Lib::Clipboard cb;

	if (!cb.Open(pView->hViewWnd)) return FALSE;
	LPTSTR pText = cb.GetText();
	cb.Close();
	if (pText == NULL) return TRUE;

	// Insert to buffer
	// allocation check has finished, so this case is clipboard is empty or unknown format, so return TRUE.
	if (!InsertLine(pText)) return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Insert String considering cursor move
/////////////////////////////////////////////////////////////////////////////

BOOL TomboEdit::InsertLine(LPCTSTR pText)
{
	DWORD nLen = _tcslen(pText);

	Region r;
	Coordinate cCursor(pView->nCursorCol, pView->nCursorRow);
	pLineMgr->LogicalPosToPhysicalPos(&cCursor, &(r.posStart));
	r.posEnd = r.posStart;
	
	if (!pDoc->ReplaceString(&r, pText)) return FALSE;

	// Adjust Cursor pos : TODO integrate this logic
	DWORD i;
	for (i = 0; i < nLen; i++) {
		if (pText[i] != TEXT('\r') && pText[i] != TEXT('\n')) {
			MoveRight();
		}
		if (pView->nCursorColPos == 0) {
			MoveRight();
		}

#if defined(PLATFORM_WIN32)
		// MoveRight should call by letters(not bytes), so skip non-lead byte
		if (IsDBCSLeadByte(*(pText + i))) {
			i++;
		}
#endif
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// region
/////////////////////////////////////////////////////////////////////////////

DWORD TomboEdit::GetRegionSize()
{
	if (rSelRegion.posStart.row == rSelRegion.posEnd.row) {
		return rSelRegion.posEnd.col - rSelRegion.posStart.col;
	} else {
		DWORD nSize = 0;
		DWORD i;
		for (i = rSelRegion.posStart.row; i <= rSelRegion.posEnd.row; i++) {
			LineChunk lc;
			if (!pDoc->GetLineChunk(i, &lc)) return 0;

			if (i == rSelRegion.posStart.row) {
				nSize += lc.LineLen() - rSelRegion.posStart.col;
				if (!lc.IsContLine()) nSize += 2;	// \r\n
			} else if (i == rSelRegion.posEnd.row) {
				nSize += rSelRegion.posEnd.col;
			} else {
				nSize += lc.LineLen();
				if (!lc.IsContLine()) nSize += 2;	// \r\n
			}
		}
		return nSize;
	}
}

BOOL TomboEdit::GetRegionString(LPTSTR pBuf)
{
	if (rSelRegion.posStart.row == rSelRegion.posEnd.row) {
		LineChunk lc;
		if (!pDoc->GetLineChunk(rSelRegion.posStart.row, &lc)) return FALSE;
		DWORD n = rSelRegion.posEnd.col - rSelRegion.posStart.col;
		_tcsncpy(pBuf, lc.GetLineData() + rSelRegion.posStart.col, n);
		*(pBuf + n) = TEXT('\0');
		return TRUE;
	} else {
		LPTSTR p = pBuf;
		DWORD n;

		DWORD i;
		for (i = rSelRegion.posStart.row; i <= rSelRegion.posEnd.row; i++) {
			LineChunk lc;
			if (!pDoc->GetLineChunk(i, &lc)) return FALSE;

			if (i == rSelRegion.posStart.row) {
				n = lc.LineLen() - rSelRegion.posStart.col;
				_tcsncpy(p, lc.GetLineData() + rSelRegion.posStart.col, n);
				p += n;

				if (!lc.IsContLine()) {
					// add eol mark
					*p++ = TEXT('\r');
					*p++ = TEXT('\n');
				}
			} else if (i == rSelRegion.posEnd.row) {
				n = rSelRegion.posEnd.col;
				_tcsncpy(p, lc.GetLineData(), n);
				p += n;
			} else {
				n = lc.LineLen();
				_tcsncpy(p, lc.GetLineData(), n);
				p += n;

				if (!lc.IsContLine()) {
					// add eol mark
					*p++ = TEXT('\r');
					*p++ = TEXT('\n');
				}
			}

		}
		*p = TEXT('\0');
		return TRUE;
	}
}

void TomboEdit::SetMark(const Coordinate &nStart)
{
	rSelRegion.posStart = rSelRegion.posEnd = nStart;
}

void TomboEdit::SelectRegion(const Coordinate &nCurrent, Coordinate *pPrev)
{
	if (rSelRegion.posStart == rSelRegion.posEnd) {
		*pPrev = rSelRegion.posStart;

		if (rSelRegion.posStart < nCurrent) {
			bForwardDrag = TRUE;
			rSelRegion.posEnd = nCurrent;
		} else {
			bForwardDrag = FALSE;
			rSelRegion.posStart = nCurrent;
		}
	} else {
		if (bForwardDrag) {
			if (nCurrent < rSelRegion.posStart) {
				// turn to backward drag
				bForwardDrag = FALSE;
				*pPrev = rSelRegion.posEnd;
				rSelRegion.posEnd = rSelRegion.posStart;
				rSelRegion.posStart = nCurrent;
			} else {
				*pPrev = rSelRegion.posEnd;
				rSelRegion.posEnd = nCurrent;
			}
		} else {
			if (nCurrent > rSelRegion.posEnd) {
				// turn to forward drag
				bForwardDrag = TRUE;
				*pPrev = rSelRegion.posStart;
				rSelRegion.posStart = rSelRegion.posEnd;
				rSelRegion.posEnd = nCurrent;
			} else {
				*pPrev = rSelRegion.posStart;
				rSelRegion.posStart = nCurrent;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Convert logical pos -> physical pos
/////////////////////////////////////////////////////////////////////////////

void TomboEdit::LogicalCursorPosToPhysicalCursorPos(DWORD nLgLineNo, DWORD nLgCursorPosX, LPDWORD pPhLineNo, LPDWORD pPhCursorPos)
{
	pLineMgr->LogicalCursorPosToPhysicalCursorPos(nLgLineNo, nLgCursorPosX, pPhLineNo, pPhCursorPos);
}

void TomboEdit::PhysicalCursorPosToLogicalCursorPos(DWORD nPhLineNo, DWORD nPhCursorPosX, LPDWORD pLgLineNo, LPDWORD pLgCursorPos)
{
	pLineMgr->PhysicalCursorPosToLogicalCursorPos(nPhLineNo, nPhCursorPosX, pLgLineNo, pLgCursorPos);
}

void TomboEdit::LogicalPosToPhysicalPos(const Coordinate *pLgPos, Coordinate *pPhPos)
{
	pLineMgr->LogicalPosToPhysicalPos(pLgPos, pPhPos);
}

void TomboEdit::PhysicalPosToLogicalPos(const Coordinate *pPhPos, Coordinate *pLgPos)
{
	pLineMgr->PhysicalPosToLogicalPos(pPhPos, pLgPos);
}

// get end of 
void TomboEdit::GetEndPhysicalPos(DWORD nLgLineNo, Coordinate *pPos)
{
	pLineMgr->GetEndPhysicalPos(nLgLineNo, pPos);
}

/////////////////////////////////////////////////////////////////////////////
// Update notify from TomboEditDoc
/////////////////////////////////////////////////////////////////////////////

BOOL TomboEdit::UpdateNotify(PhysicalLineManager *pPhMgr, const Region *pRegion, DWORD nBefPhLines, DWORD nAftPhLines, DWORD nAffeLines)
{
	DWORD nBefLgLines = pLineMgr->MaxLine();
	Coordinate cLgAfStart;
	DWORD nAffLgLines;
	if (!pLineMgr->AdjustLgLines(pPhMgr, pWrapper, *pRegion, nBefPhLines, nAftPhLines, nAffeLines, &cLgAfStart, &nAffLgLines)) return FALSE;
	DWORD nAftLgLines = pLineMgr->MaxLine();

	// update view
	// impliment for the present..
	RequestRedraw(cLgAfStart.row, 0, TRUE);
	ClearRegion();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Join line
/////////////////////////////////////////////////////////////////////////////

BOOL TomboEdit::JoinLine(DWORD nLgLineNo)
{
	LineChunk lc;
	if (!pDoc->GetLineChunk(nLgLineNo, &lc)) return FALSE;

	Coordinate cLgEnd(lc.LineLen(), nLgLineNo); // end of the line;
	Coordinate cPhEnd;
	pLineMgr->LogicalPosToPhysicalPos(&cLgEnd, &cPhEnd);

	Region r;
	r.posStart = cPhEnd;
	r.posEnd.Set(0, cPhEnd.row + 1);
	if (!pDoc->ReplaceString(&r, TEXT(""))) return FALSE;

	return TRUE;
}