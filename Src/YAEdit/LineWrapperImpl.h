#ifndef LINEWRAPPERIMPL_H
#define LINEWRAPPERIMPL_H

/////////////////////////////////////////////////////////////////////////////
// wrapped by window width
/////////////////////////////////////////////////////////////////////////////

class TomboEdit;

class FixedPixelLineWrapper : public LineWrapper {
protected:
	TomboEdit *pView;
	DWORD nViewWidth;
public:

	
	FixedPixelLineWrapper() {}
	virtual ~FixedPixelLineWrapper() {}

	BOOL Init(TomboEdit *p) { pView = p; return TRUE; }

	WrapResult Wrap(DWORD nCurrentPos, LPCTSTR pBase, DWORD nLineLimit, LPDWORD pSepPos);

	void SetViewWidth(DWORD nWidth) { nViewWidth = nWidth; }
	DWORD GetViewWidth() { return nViewWidth; }
};


#endif