#ifndef TOMBOEDIT_H
#define TOMBOEDIT_H

class TomboEditDoc;
class FixedPixelLineWrapper;
class LineChunk;
class FontWidthCache;
class TomboEditView;
class LineManager;
class PhysicalLineManager;
class YAECallbackHandler;
class YAEDocCallbackHandler;

//////////////////////////////////////////////////
// Controller class for YAE
//////////////////////////////////////////////////

class TomboEdit {
protected:

	///////////////////////////////////////
	// window related members
	HDC hCommonDC;
	HINSTANCE hInstance;

	///////////////////////////////////////
	// VMC related members
	TomboEditDoc *pDoc;
	TomboEditView *pView;

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

	// value is by char, 
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

	////////////////////////////////////////////////////
	// line operation helper
	BOOL JoinLine(DWORD nLgLineNo);
	BOOL InsertLine(LPCTSTR pText);

public:

	///////////////////////////////////////
	// ctor & initialize
	TomboEdit();
	~TomboEdit();
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
	TomboEditDoc *SetDoc(TomboEditDoc *pNewDoc);
	TomboEditDoc *GetDoc() { return pDoc; }
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
	// TomboEditDoc callback

	void RequestRedraw(DWORD nLineNo, WORD nLeftPos, BOOL bToBottom);
	void RequestRedrawRegion(const Region *pRegion);

	void ChangeLinesNotify(int nDelta);

	/////////////////////////////////
	// Line wrapping 
	FixedPixelLineWrapper *GetWrapper() { return pWrapper; }
	DWORD GetLineWidth(DWORD nOffset, LPCTSTR pStr, DWORD nLen);

	/////////////////////////////////
	// Clipboard
	BOOL CopyToClipboard();
	BOOL InsertFromClipboard();

	/////////////////////////////////
	// region
	BOOL DeleteRegion();
	void ClearRegion();

	/////////////////////////////////
	// forcus window
	void OnSetFocus();
	void OnKillFocus();

	////////////////////////////////////////////////////
	// Region related members

	BOOL IsRegionSelected() { return rSelRegion.posStart != rSelRegion.posEnd; }
	BOOL IsSelRegionOneLine() { return rSelRegion.posStart.row == rSelRegion.posEnd.row; }
	const Region& SelectedRegion() { return rSelRegion; }
	void ClearSelectedRegion();

	BOOL GetRegionString(LPTSTR pBuf);
	DWORD GetRegionSize();

	void SetMark(const Coordinate& nStart);
	void SelectRegion(const Coordinate &nCurrent, Coordinate *pPrev);

	////////////////////////////////////////////////////
	// coordinate conversion

	void LogicalCursorPosToPhysicalCursorPos(DWORD nLgLineNo, DWORD nLgCursorPosX, LPDWORD pPhLineNo, LPDWORD pPhCursorPos);
	void PhysicalCursorPosToLogicalCursorPos(DWORD nPhLineNo, DWORD nPhCursorPosX, LPDWORD pLgLineNo, LPDWORD pLgCursorPos);
	void LogicalPosToPhysicalPos(const Coordinate *pLgPos, Coordinate *pPhPos);
	void PhysicalPosToLogicalPos(const Coordinate *pPhPos, Coordinate *pLgPos);

	void GetEndPhysicalPos(DWORD nLgLineNo, Coordinate *pPos);

	////////////////////////////////////////////////////
	// callback from Document
	BOOL UpdateNotify(PhysicalLineManager *pPhMgr, const Region *pOldRegion, DWORD nBefPhLines, DWORD nAftPhLines, DWORD nAffeLines);
};

//////////////////////////////////////////////////
// YAE callbacks
//////////////////////////////////////////////////

class YAECallbackHandler {
public:
};

#endif