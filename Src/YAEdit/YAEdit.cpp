#include <windows.h>
#include <tchar.h>
#if !defined(PLATFORM_PSPC)
#include <imm.h>
#endif
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

#define YAEDIT_CLASS_NAME TEXT("YAEditCtl")

#define CHARA_CTRL_C 3
#define CHARA_CTRL_S 19
#define CHARA_CTRL_V 22
#define CHARA_CTRL_X 24
#define CHARA_BS 8
#define CHARA_ENTER 13

/////////////////////////////////////////////////////////////////////////////
// static funcs/vars declarations
/////////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK YAEditWndProc(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////
// Regist window class
/////////////////////////////////////////////////////////////////////////////

BOOL YAEdit::RegisterClass(HINSTANCE hInst)
{
	WNDCLASS wc;

	wc.style = 0;
	wc.lpfnWndProc = (WNDPROC)YAEditWndProc;
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
	wc.lpszClassName = YAEDIT_CLASS_NAME;
	::RegisterClass(&wc);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// ctor & dtor
/////////////////////////////////////////////////////////////////////////////

YAEdit::YAEdit() : pWrapper(NULL), bScrollTimerOn(FALSE), pView(NULL), bMouseDown(FALSE), pLineMgr(NULL), pHandler(NULL)
{
}

YAEdit::~YAEdit()
{
	if (pView) {
		if (pView->hViewWnd) DestroyWindow(pView->hViewWnd);
	}
	if (pLineMgr) delete pLineMgr;
}

/////////////////////////////////////////////////////////////////////////////
// Event handler
/////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK YAEditWndProc(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	if (nMessage == WM_CREATE) {
		LPCREATESTRUCT pCS = (LPCREATESTRUCT)lParam;
		YAEdit *frm = (YAEdit*)pCS->lpCreateParams;
		SetWindowLong(hWnd, 0, (LONG)frm);
		frm->OnCreate(hWnd, wParam, lParam);
		return 0;
	}

	YAEdit *frm = (YAEdit*)GetWindowLong(hWnd, 0);
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

BOOL YAEdit::Create(HINSTANCE hInst, HWND hParent, DWORD nId, RECT &r, YAECallbackHandler *pCb, YAEDocCallbackHandler *pDocCb)
{
	pHandler = pCb;
	hInstance = hInst;
	pDoc = NULL;

	pLineMgr = new LineManager();
	if (!pLineMgr->Init(this)) return FALSE;

	pView = new YAEditView(this);
	if (!pView->Init()) return FALSE;

	FixedPixelLineWrapper *pWw = new FixedPixelLineWrapper();
	if (pWw == NULL || !pWw->Init(this)) return FALSE;
	pWrapper = pWw;

	pDoc = new YAEditDoc(); 
	if (!pDoc->Init("", this, pDocCb)) return FALSE;

	
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	pView->hViewWnd = CreateWindowEx(WS_EX_CLIENTEDGE, YAEDIT_CLASS_NAME, TEXT(""),
						WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL,
						r.left,
						r.top,
						r.right,
						r.bottom,
						hParent,
						(HMENU)nId,
						hInst,
						this);
	DWORD nx = GetLastError();
#else
	pView->hViewWnd = CreateWindow(YAEDIT_CLASS_NAME, TEXT(""),
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

void YAEdit::OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	pView->OnCreate(hWnd, wParam, lParam);
	pWrapper->SetViewWidth(pView->rClientRect.right - pView->rClientRect.left - pView->nMaxCharWidth);


	// associate with default(empty) document.
	// Since memory allocation check, object is created in Create(), and assoicated here.
	SetDoc(pDoc);
}

/////////////////////////////////////////////////////////////////////////////
// WM_VSCROLL handler
/////////////////////////////////////////////////////////////////////////////

void YAEdit::OnVScroll(HWND hWnd, WPARAM wParam, LPARAM lParam)
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

void YAEdit::OnHScroll(HWND hWnd, WPARAM wParam, LPARAM lParam)
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
void YAEdit::OnMouseWheel(HWND hWnd, WPARAM wParam, LPARAM lParam)
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

void YAEdit::OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam)
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

void YAEdit::RequestRedraw(DWORD nLineNo, WORD nLeftPos, BOOL bToBottom) { pView->RequestRedraw(nLineNo, nLeftPos, bToBottom); }
void YAEdit::RequestRedrawRegion(const Region *pRegion) { pView->RequestRedrawRegion(pRegion); }

/////////////////////////////////////////////////////////////////////////////
// FOCUS
/////////////////////////////////////////////////////////////////////////////

void YAEdit::OnSetFocus()  { if (pView) pView->OnSetFocus();  }
void YAEdit::OnKillFocus() { if (pView) pView->OnKillFocus(); }

/////////////////////////////////////////////////////////////////////////////
// WM_KEYDOWN
/////////////////////////////////////////////////////////////////////////////
BOOL YAEdit::OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int nVertKey = (int)wParam;
	BOOL bShiftDown = (GetKeyState(VK_SHIFT) & 0x8000) != 0;

	if (bShiftDown) {
		switch (nVertKey) {
		case VK_RIGHT:
			pView->MoveRight();
			UpdateSelRegion();
			break;
		case VK_LEFT:
			pView->MoveLeft();
			UpdateSelRegion();
			break;
		case VK_UP:
			pView->MoveUp();
			UpdateSelRegion();
			break;
		case VK_DOWN:
			pView->MoveDown();
			UpdateSelRegion();
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

void YAEdit::UpdateSelRegion()
{
	Coordinate nSelNow = pView->GetCaretPosition();
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

/////////////////////////////////////////////////////////////////////////////
// WM_CHAR
/////////////////////////////////////////////////////////////////////////////

void YAEdit::OnChar(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	TCHAR ch = (TCHAR)wParam;
	if (ch == CHARA_BS) {
		KeyBS();
		return;
	}
	if (ch == CHARA_ENTER) {
		ReplaceText(SelectedRegion(), TEXT("\n"));
		pView->ResetScrollbar();
		return;
	}
	if (ch == CHARA_CTRL_C) {
		if (!CopyToClipboard()) {
			MessageBox(hWnd, TEXT("Copy to clipboard failed."), TEXT("ERROR"), MB_ICONWARNING | MB_OK);
		}
		return;
	}
	if (ch == CHARA_CTRL_X) {
		if (IsRegionSelected()) {
			if (!CopyToClipboard()) {
				MessageBox(hWnd, TEXT("Copy to clipboard failed."), TEXT("ERROR"), MB_ICONWARNING | MB_OK);
			}
			ReplaceText(SelectedRegion(), TEXT(""));
		}
		return;
	}
	if (ch == CHARA_CTRL_V) {
		if (!InsertFromClipboard()) {
			MessageBox(hWnd, TEXT("Paste from clipboard failed."), TEXT("ERROR"), MB_ICONWARNING | MB_OK);
		}
		return;
	}
	if (ch == CHARA_CTRL_S) {
		// nop
		return;
	}

#if defined(PLATFORM_WIN32)
	// if char is DBCS lead byte, buffering.
	if (aKeyBuffer[0] != '\0') {
		aKeyBuffer[1] = ch;
		aKeyBuffer[2] = TEXT('\0');
		ReplaceText(SelectedRegion(), aKeyBuffer);
		aKeyBuffer[0] = aKeyBuffer[1] = '\0';
		return;
	} else {
		if (IsDBCSLeadByte(ch)) {
			aKeyBuffer[0] = ch;
			return;
		}
	}
#endif
	TCHAR kbuf[2];
	kbuf[0] = ch; kbuf[1] = TEXT('\0');
	ReplaceText(SelectedRegion(), kbuf);
}

void YAEdit::KeyBS()
{
	if (IsRegionSelected()) {
		ReplaceText(SelectedRegion(), TEXT(""));
	} else {
		Region r;
		r.posEnd = pView->GetCaretPosition();
		pView->MoveLeft();
		r.posStart = pView->GetCaretPosition();
		if (!r.IsEmptyRegion()) ReplaceText(r, TEXT(""));
	}
}

void YAEdit::DeleteKeyDown()
{
	if (IsRegionSelected()) {
		ReplaceText(SelectedRegion(), TEXT(""));
	} else {
		Region r;
		r.posStart = pView->GetCaretPosition();
		pView->MoveRight();
		r.posEnd = pView->GetCaretPosition();
		if (!r.IsEmptyRegion()) ReplaceText(r, TEXT(""));
	}
}

/////////////////////////////////////////////////////////////////////////////
// move cursor
/////////////////////////////////////////////////////////////////////////////

void YAEdit::MoveRight() { pView->ScrollCaret(); pView->MoveRight(); ClearRegion(); }
void YAEdit::MoveLeft()  { pView->ScrollCaret(); pView->MoveLeft();  ClearRegion(); }
void YAEdit::MoveUp()    { pView->ScrollCaret(); pView->MoveUp();    ClearRegion(); }
void YAEdit::MoveEOL()   { pView->ScrollCaret(); pView->MoveEOL();   ClearRegion(); }
void YAEdit::MoveTOL()   { pView->ScrollCaret(); pView->MoveTOL();   ClearRegion(); }
void YAEdit::MoveDown()  { pView->ScrollCaret(); pView->MoveDown();  ClearRegion(); }

/////////////////////////////////////////////////////////////////////////////
// WM_LBUTTONDOWN
/////////////////////////////////////////////////////////////////////////////

void YAEdit::OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
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
		rSelRegion.posStart = rSelRegion.posEnd = pView->GetCaretPosition();

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

void YAEdit::SetSelectionFromPoint(int xPos, int yPos)
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
		UpdateSelRegion();
	}
}

void YAEdit::OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam)
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

void YAEdit::OnLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
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

void YAEdit::OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam)
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

YAEditDoc *YAEdit::SetDoc(YAEditDoc *pNewDoc)
{
	YAEditDoc *pOldDoc = pDoc;
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

	pView->SetCaretPosition(Coordinate(0, 0));
	ClearSelectedRegion();	

	pView->ResetScrollbar();
	pView->RedrawAllScreen();
	return pOldDoc;
}

/////////////////////////////////////////////////////////////////////////////
// get max line width for decide hscroll range
/////////////////////////////////////////////////////////////////////////////

DWORD YAEdit::GetLineWidth(DWORD nOffset, LPCTSTR pStr, DWORD nLen) { return pView->GetLineWidth(nOffset, pStr, nLen); }

/////////////////////////////////////////////////////////////////////////////
// Resize window
/////////////////////////////////////////////////////////////////////////////

void YAEdit::ResizeWindow(int x, int y, int width, int height) 
{
	MoveWindow(pView->hViewWnd, x, y, width, height, FALSE);

	pView->ResetParam();

	pWrapper->SetViewWidth(pView->rClientRect.right - pView->rClientRect.left - pView->nMaxCharWidth);

	// Preserve absolute cursor position before rewrapping.
	Coordinate cPhCursorPos;
	pLineMgr->LogicalPosToPhysicalPos(&(pView->GetCaretPosition()), &cPhCursorPos);

	Region rPhRgn;
	pLineMgr->LogicalPosToPhysicalPos(&(rSelRegion.posStart), &(rPhRgn.posStart));
	pLineMgr->LogicalPosToPhysicalPos(&(rSelRegion.posEnd), &(rPhRgn.posEnd));

	// Rewrapping. Logical line will be changed.
	pLineMgr->RecalcWrap(pWrapper);
	pView->ResetScrollbar();

	// get cursor position after rewrapping.
	Coordinate cLgCursorPos;
	pLineMgr->PhysicalPosToLogicalPos(&cPhCursorPos, &cLgCursorPos);
	pView->SetCaretPosition(cLgCursorPos);

	pLineMgr->PhysicalPosToLogicalPos(&(rPhRgn.posStart), &(rSelRegion.posStart));
	pLineMgr->PhysicalPosToLogicalPos(&(rPhRgn.posEnd), &(rSelRegion.posEnd));

	// redraw screen
	pView->RedrawAllScreen();
}

/////////////////////////////////////////////////////////////////////////////
// Region
/////////////////////////////////////////////////////////////////////////////
void YAEdit::SetFocus()
{
	::SetFocus(pView->hViewWnd);
}

/////////////////////////////////////////////////////////////////////////////
// Region
/////////////////////////////////////////////////////////////////////////////

void YAEdit::ClearRegion()
{
	RequestRedrawRegion(&SelectedRegion());
	ClearSelectedRegion();
}

void YAEdit::ClearSelectedRegion()
{
	rSelRegion.posEnd = pView->GetCaretPosition();
	rSelRegion.posStart = rSelRegion.posEnd;
}

/////////////////////////////////////////////////////////////////////////////
// Insert String considering cursor move
/////////////////////////////////////////////////////////////////////////////

BOOL YAEdit::ReplaceText(const Region &rLg, LPCTSTR pText)
{
	Region r;
	pLineMgr->LogicalPosToPhysicalPos(&(rLg.posStart), &(r.posStart));
	pLineMgr->LogicalPosToPhysicalPos(&(rLg.posEnd), &(r.posEnd));
	return pDoc->ReplaceString(&r, pText);
}

/////////////////////////////////////////////////////////////////////////////
// Clipboard
/////////////////////////////////////////////////////////////////////////////

BOOL YAEdit::CopyToClipboard()
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

BOOL YAEdit::InsertFromClipboard()
{
	// Get data from clipboard
	Tombo_Lib::Clipboard cb;

	if (!cb.Open(pView->hViewWnd)) return FALSE;
	LPTSTR pText = cb.GetText();
	cb.Close();
	if (pText == NULL) return TRUE;

	// Insert to buffer
	// allocation check has finished, so this case is clipboard is empty or unknown format, so return TRUE.
	if (!ReplaceText(SelectedRegion(), pText)) return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// region
/////////////////////////////////////////////////////////////////////////////

DWORD YAEdit::GetRegionSize()
{
	if (IsSelRegionOneLine()) {
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

BOOL YAEdit::GetRegionString(LPTSTR pBuf)
{
	if (IsSelRegionOneLine()) {
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

void YAEdit::SelectRegion(const Coordinate &nCurrent, Coordinate *pPrev)
{
	if (!IsRegionSelected()) {
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
// Update notify from YAEditDoc
/////////////////////////////////////////////////////////////////////////////

BOOL YAEdit::UpdateNotify(PhysicalLineManager *pPhMgr, const Region *pOldRegion, const Region *pNewRegion, DWORD nBefPhLines, DWORD nAftPhLines, DWORD nAffeLines)
{
	DWORD nBefLgLines = pLineMgr->MaxLine();

	Coordinate cLgAfStart;
	DWORD nAffLgLines;
	if (!pLineMgr->AdjustLgLines(pPhMgr, pWrapper, *pOldRegion, nBefPhLines, nAftPhLines, nAffeLines, &cLgAfStart, &nAffLgLines)) return FALSE;

	DWORD nAftLgLines = pLineMgr->MaxLine();

	// adjust caret position
	Coordinate cCaretPos;
	pLineMgr->PhysicalPosToLogicalPos(&(pNewRegion->posEnd), &cCaretPos);
	pView->SetCaretPosition(cCaretPos);
	ClearSelectedRegion();

	// update view
	// what a inefficient logic!!
	RequestRedraw(cLgAfStart.row, 0, TRUE);

	pView->ScrollCaret();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Set font
/////////////////////////////////////////////////////////////////////////////

void YAEdit::SetFont(HFONT hFont)
{
	if (hFont == NULL) return;
	pView->hFont = hFont;

	pView->ResetFontInfo();
	pWrapper->SetViewWidth(pView->rClientRect.right - pView->rClientRect.left - pView->nMaxCharWidth);

	if (pLineMgr) delete pLineMgr;
	pLineMgr = new LineManager();
	if (!pLineMgr->Init(this)) return;
	SetDoc(pDoc);
}