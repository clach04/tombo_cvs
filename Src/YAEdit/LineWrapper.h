#ifndef LINEWRAPPER_H
#define LINEWRAPPER_H

////////////////////////////////////////////////////////////////////////////
// Wrap line text
/////////////////////////////////////////////////////////////////////////////

class LineWrapper {
public:
	enum WrapResult {
		WRAPRESULT_FAIL,
		WRAPRESULT_CONT,
		WRAPRESULT_FINISH
	};

	LineWrapper() {}
	virtual ~LineWrapper() {}

	/////////////////////////////////////////////////////////////////
	// wrapping implimentaiton

	// IN:
	//		nCurrentPos
	//		pBase
	//		nLineLimit
	// OUT:
	//		pSepPos	
	// RESULT:
	//		WRAPRESULT_FINISH
	//		WRAPRESULT_CONT

	virtual WrapResult Wrap(DWORD nCurrentPos, LPCTSTR pBase, DWORD nLineLimit, LPDWORD pSepPos) = 0;

	BOOL GetNewMarkerList(TVector<DWORD> *pLines, DWORD nCalcStartPos, LPCTSTR pData, DWORD nLen);
};

////////////////////////////////////////////////////////////////////////////
// Simple line wrapper
/////////////////////////////////////////////////////////////////////////////
// SimpleLineWrapper associates 1 physical line to 1 logical line(no wrapping).

class SimpleLineWrapper : public LineWrapper {
public:
	SimpleLineWrapper();
	virtual ~SimpleLineWrapper();

	WrapResult Wrap(DWORD nCurrentPos, LPCTSTR pBase, DWORD nLineLimit, LPDWORD pSepPos);
};

////////////////////////////////////////////////////////////////////////////
// wrap by specified letters 
/////////////////////////////////////////////////////////////////////////////

class FixedLetterWrapper : public LineWrapper {
	DWORD nWidth;
public:
	FixedLetterWrapper(DWORD n);
	~FixedLetterWrapper();

	WrapResult Wrap(DWORD nCurrentPos, LPCTSTR pBase, DWORD nLineLimit, LPDWORD pSepPos);
};
#endif
