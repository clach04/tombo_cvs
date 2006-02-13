#ifndef LINEWRAPPERIMPL_H
#define LINEWRAPPERIMPL_H

/////////////////////////////////////////////////////////////////////////////
// wrapped by window width
/////////////////////////////////////////////////////////////////////////////

class YAEdit;

class FixedPixelLineWrapper : public LineWrapper {
protected:
	YAEdit *pView;
public:

	FixedPixelLineWrapper() {}
	virtual ~FixedPixelLineWrapper() {}

	BOOL Init(YAEdit *p) { pView = p; return TRUE; }

	WrapResult Wrap(DWORD nCurrentPos, LPCTSTR pBase, DWORD nLineLimit, LPDWORD pSepPos);

};


#endif