#ifndef YAEDIT_H
#define YAEDIT_H

class YAEditDoc;
class FixedPixelLineWrapper;
class LineChunk;
class FontWidthCache;
class YAEditView;
class LineManager;
class PhysicalLineManager;
class YAECallbackHandler;
class YAEDocCallbackHandler;

//////////////////////////////////////////////////
// Controller class for YAE
//////////////////////////////////////////////////

class YAEdit {
protected:

	///////////////////////////////////////
	// window related members
	HDC hCommonDC;
	HINSTANCE hInstance;

	///////////////////////////////////////
	// VMC related members
	YAEditDoc *pDoc;
	YAEditView *pView;

	///////////////////////////////////////
	// key related members
#if defined(PLATFORM_WIN32)
	char aKeyBuffer[3];
#endif

	///////////////////////////////////////
	// line management members

	LineManager *pLineMgr;

	FixedPixelLineWrapper *pWrapper;
	BOOL bWrapText;		// TRUE if wrap text

	BOOL bScrollTimerOn;
	POINT ptMousePos;
	BOOL bMouseDown;

	// value is by logical coordinate, 
	// (nSelStartCol, nSelStartRow) < (nSelEndCol, nSelEndRow) is always TRUE.
	Region rSelRegion;
	BOOL bForwardDrag;

	///////////////////////////////////////
	// callback handler
	YAECallbackHandler *pHandler;

protected:
	///////////////////////////////////////
	// select region
	void SetSelectionFromPoint(int xPos, int yPos);
	void UpdateSelRegion();

	////////////////////////////////////////////////////
	// line operation helper
	BOOL ReplaceText(const Region &r, LPCTSTR pText);

	////////////////////////////////////////////////////
	// Region related members
	BOOL GetRegionString(LPTSTR pBuf);
	DWORD GetRegionSize();

	void ClearRegion();
	void ClearSelectedRegion();

public:

	///////////////////////////////////////
	// ctor & initialize
	YAEdit();
	~YAEdit();
	BOOL Create(HINSTANCE hInst, HWND hWnd, DWORD nId, RECT &r, YAECallbackHandler *pViewCB, YAEDocCallbackHandler* pDocCB);
	void SetFocus();

	///////////////////////////////////////
	// initialize & register window class
	static BOOL RegisterClass(HINSTANCE hInst);

	/////////////////////////////////
	// Event handler

	void OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);

	BOOL OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam); // if FALSE, call default proc
	void OnChar(HWND hWnd, WPARAM wParam, LPARAM lParam);

	void OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void OnLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam);

	void OnVScroll(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void OnHScroll(HWND hWnd, WPARAM wParam, LPARAM lParam);

#if defined(PLATFORM_WIN32)
	void OnMouseWheel(HWND hWnd, WPARAM wParam, LPARAM lParam);
#endif

	void OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);

	/////////////////////////////////
	// application funcs.
	YAEditDoc *SetDoc(YAEditDoc *pNewDoc);
	YAEditDoc *GetDoc() { return pDoc; }
	LineManager *GetLineMgr() { return pLineMgr; }

	///////////////////////////////////////
	// cursor moving funcs.

	void MoveRight();
	void MoveLeft();
	void MoveUp();
	void MoveDown();
	void MoveEOL();
	void MoveTOL();

	/////////////////////////////////
	// Key hander
	void KeyBS();
	void DeleteKeyDown();

	/////////////////////////////////
	// Move/Resize window
	void ResizeWindow(int x, int y, int width, int height);

	/////////////////////////////////
	// YAEditDoc callback

	void RequestRedraw(DWORD nLineNo, WORD nLeftPos, BOOL bToBottom);
	void RequestRedrawRegion(const Region *pRegion);

	/////////////////////////////////
	// Line wrapping 
	FixedPixelLineWrapper *GetWrapper() { return pWrapper; }
	DWORD GetLineWidth(DWORD nOffset, LPCTSTR pStr, DWORD nLen);

	/////////////////////////////////
	// Clipboard
	BOOL CopyToClipboard();
	BOOL InsertFromClipboard();

	/////////////////////////////////
	// forcus window
	void OnSetFocus();
	void OnKillFocus();

	////////////////////////////////////////////////////
	// Region related members

	BOOL IsRegionSelected() { return rSelRegion.posStart != rSelRegion.posEnd; }
	BOOL IsSelRegionOneLine() { return rSelRegion.posStart.row == rSelRegion.posEnd.row; }
	const Region& SelectedRegion() { return rSelRegion; }

	void SelectRegion(const Coordinate &nCurrent, Coordinate *pPrev);

	////////////////////////////////////////////////////
	// callback from Document
	BOOL UpdateNotify(PhysicalLineManager *pPhMgr, const Region *pOldRegion, const Region *pNewRegion, DWORD nBefPhLines, DWORD nAftPhLines, DWORD nAffeLines);

	////////////////////////////////////////////////////
	// font
	void SetFont(HFONT hFont);
};

//////////////////////////////////////////////////
// YAE callbacks
//////////////////////////////////////////////////

class YAECallbackHandler {
public:
};

#endif