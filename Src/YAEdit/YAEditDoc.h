#ifndef YAEDITDOC_H
#define YAEDITDOC_H

class TomboEdit;
class LineChunk;
class Region;
class PhysicalLineManager;
class YAEDocCallbackHandler;

////////////////////////////////////////////////////
// Document container for YAE
////////////////////////////////////////////////////

class TomboEditDoc {
protected:
	PhysicalLineManager *pPhLineMgr;
	TomboEdit *pView;

	YAEDocCallbackHandler *pHandler;

	// this member should not to edit directry. use SetModify().
	BOOL bModified;

	BOOL ReleaseDoc();
public:
	////////////////////////////////////////////////////
	// ctor, dtor & initialize

	TomboEditDoc();
	~TomboEditDoc();

	BOOL Init(const char *pStr, TomboEdit *pView, YAEDocCallbackHandler *pCb);

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

};

////////////////////////////////////////////////////
// Document callback for YAE
////////////////////////////////////////////////////

class YAEDocCallbackHandler {
public:
	virtual void OnModifyStatusChanged(TomboEditDoc *pDoc, BOOL bOld, BOOL bNew) = 0;
};

#endif