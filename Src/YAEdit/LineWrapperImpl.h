#ifndef LINEWRAPPERIMPL_H
#define LINEWRAPPERIMPL_H

/////////////////////////////////////////////////////////////////////////////
// wrapped by window width
/////////////////////////////////////////////////////////////////////////////

class YAEdit;

class FixedPixelLineWrapper : public LineWrapper {
protected:
	YAEdit *pView;
	DWORD nViewWidth;
public:

	
	FixedPixelLineWrapper() {}
	virtual ~FixedPixelLineWrapper() {}

	BOOL Init(YAEdit *p) { pView = p; return TRUE; }

	WrapResult Wrap(DWORD nCurrentPos, LPCTSTR pBase, DWORD nLineLimit, LPDWORD pSepPos);

	void SetViewWidth(DWORD nWidth) { nViewWidth = nWidth; }
	DWORD GetViewWidth() { return nViewWidth; }
};


#endif