#ifndef YAEDITDOC_H
#define YAEDITDOC_H

class YAEdit;
class LineChunk;
class Region;
class PhysicalLineManager;
class YAEDocCallbackHandler;

////////////////////////////////////////////////////
// Document container for YAE
////////////////////////////////////////////////////

class YAEditDoc {
protected:
	PhysicalLineManager *pPhLineMgr;
	YAEdit *pView;

	YAEDocCallbackHandler *pHandler;

	// this member should not to edit directry. use SetModify().
	BOOL bModified;

	BOOL ReleaseDoc();
public:
	////////////////////////////////////////////////////
	// ctor, dtor & initialize

	YAEditDoc();
	~YAEditDoc();

	BOOL Init(const char *pStr, YAEdit *pView, YAEDocCallbackHandler *pCb);

	PhysicalLineManager *GetPhMgr() { return pPhLineMgr; }

	////////////////////////////////////////////////////
	// set/get document

	BOOL LoadDoc(const char *pStr);
	char *GetDocumentData(LPDWORD pLen);

	////////////////////////////////////////////////////
	// retrieve & set line 

	BOOL GetLineChunk(DWORD nLineNo, LineChunk *pChunk);
	DWORD GetPrevOffset(DWORD nLineNo, DWORD nCurrentPos);
	BOOL ReplaceString(const Region *pRegion, LPCTSTR pString);

	BOOL IsModify() { return bModified; }
	void SetModify(BOOL b);

	////////////////////////////////////////////////////
	// Data size related functions
	DWORD GetDataBytes(const Region *pRegion);
	void ConvertBytesToCoordinate(DWORD nPos, Coordinate *pPos);
};

////////////////////////////////////////////////////
// Document callback for YAE
////////////////////////////////////////////////////

class YAEDocCallbackHandler {
public:
	virtual void OnModifyStatusChanged(YAEditDoc *pDoc, BOOL bOld, BOOL bNew) = 0;
};
#endif