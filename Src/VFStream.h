#ifndef VFSTREAM_H
#define VFSTREAM_H

class MemoNote;
class SearchEngineA;
class PasswordManager;
class VFStore;
class TString;
class File;

#include "VarBuffer.h"

////////////////////////////////////
////////////////////////////////////
// Virtual Folder Node Stream
////////////////////////////////////
////////////////////////////////////

////////////////////////////////////
// stream item
////////////////////////////////////

class VFNote {
	MemoNote *pNote;
	LPTSTR pFileName;
	UINT64 uLastUpdate;
	UINT64 uCreateDate;
	UINT64 uFileSize;

public:
	VFNote() : pNote(NULL), pFileName(NULL) {}
	~VFNote();
	BOOL Init(MemoNote *p, LPCTSTR pFileName);

	MemoNote *GetNote() { return pNote; }

	// If ClearNote() is not called and VFNote is deleted, 
	// pNote is deleted, too. To prevent deleting, you should call ClearNote().
	// In this case, deleting pNote is due to caller.
	void ClearNote() { pNote = NULL; }
	LPCTSTR GetFileName() { return pFileName; }

	UINT64 GetLastUpdate() { return uLastUpdate; }
	UINT64 GetCreateDate() { return uCreateDate; }
	UINT64 GetFileSize() { return uFileSize; }
};

////////////////////////////////////
// VFStream interface
////////////////////////////////////

class VFStream {
protected:
	VFStream *pNext;
public:

	VFStream();
	virtual ~VFStream();

	///////////////////////////
	// Create/delete filter chains.

	BOOL SetNext(VFStream *p);
	virtual void FreeObject();
	VFStream *GetNext() { return pNext; }

	///////////////////////////
	// Notes filtering.

	// Initialize classes before sequence of Store().
	// by default, Prepare() calls pNext->Prepare().
	virtual BOOL Prepare();

	// Try to store VFNote. If success, retrun TRUE. If errors, return FALSE.
	// if a class VFStream's subclass decide discard p, the class should delete p.
	virtual BOOL Store(VFNote *p) = 0;

	// Finalize classes after calling sequence of Store().
	// by default PostActive() calls pNext->PostActive().
	virtual BOOL PostActivate();

	///////////////////////////
	// copy method
	virtual VFStream *Clone(VFStore **ppTail) = 0;

	///////////////////////////
	// generate XML
	virtual BOOL GenerateXMLOpenTag(File *pFile) = 0;
	virtual BOOL GenerateXMLCloseTag(File *pFile) = 0;

	///////////////////////////
	// for FilterDefDlg
	virtual LPCTSTR GetFilterType() = 0;
	virtual BOOL ToString(TString *p) = 0;

	///////////////////////////
	// for Setting parameter
	virtual BOOL UpdateParamWithDialog(HINSTANCE hInst, HWND hParent) = 0;
		// return TRUE if updated.
		// FALSE if not updated(CANCELED) or failed.
};

////////////////////////////////////
// Directory generator
////////////////////////////////////
// In current version, VFDirectoryGenerator is only generator.

class VFDirectoryGenerator : public VFStream {
	LPTSTR pDirPath;
	BOOL bCheckEncrypt;
public:
	VFDirectoryGenerator();
	~VFDirectoryGenerator();

	////////////////////////////
	// VFStream implimentation

	// BOOL Prepare(); inherit 
	BOOL Store(VFNote *p);
	// BOOL PostActivate(); inherit
	BOOL Activate();
	// void FreeObject(); inherit

	BOOL Init(LPTSTR pDir, BOOL bCheckEncrypt);
		// pDir's memory is controled under VFDirectoryGenerator.
		// do not delete pDir after calling Init.

	VFStream *Clone(VFStore **ppTail);

	BOOL GenerateXMLOpenTag(File *pFile);
	BOOL GenerateXMLCloseTag(File *pFile);

	LPCTSTR GetDirPath() { return pDirPath; }
	BOOL SetDirPath(LPCTSTR pPath);

	LPCTSTR GetFilterType();
	BOOL ToString(TString *p);

	BOOL UpdateParamWithDialog(HINSTANCE hInst, HWND hParent);
};

////////////////////////////////////
// Directory generator
////////////////////////////////////

class VFStore : public VFStream {
public:
	enum OrderInfo {
		ORDER_TITLE,
		ORDER_LAST_UPD,
	};

protected:
	enum OrderInfo oiOrder;
	TVector <VFNote*> vNotes;

public:
//	VFStore(enum OrderInfo odr);
	VFStore();
	~VFStore();
	BOOL Init();

	void FreeObject();

	BOOL Prepare();
	BOOL Store(VFNote *p);
	BOOL PostActivate();
	VFStream *Clone(VFStore **ppTail);
	BOOL GenerateXMLOpenTag(File *pFile);
	BOOL GenerateXMLCloseTag(File *pFile);

	DWORD NumItem() { return vNotes.NumItems(); }
	VFNote *GetNote(DWORD n) { return *vNotes.GetUnit(n); }

	// free VFNote array. 
	void FreeArray();
		// VFNote in array is deleted, but MemoNote in each VFNote is not deleted.

	LPCTSTR GetFilterType();
	BOOL ToString(TString *p);

	BOOL UpdateParamWithDialog(HINSTANCE hInst, HWND hParent);
};

////////////////////////////////////
// Filter by regular expression
////////////////////////////////////

class VFRegexFilter : public VFStream {
#ifdef UNIT_TEST
public:
#endif
	SearchEngineA *pRegex;
	TString *pPattern;
	BOOL bCaseSensitive;
	BOOL bFileNameOnly;
	BOOL bNegate;

	PasswordManager *pPassMgr;

public:
	VFRegexFilter();
	~VFRegexFilter();
	BOOL Init(LPCTSTR pPattern, BOOL bCase, BOOL bEnc, BOOL bFileName, BOOL bNeg, PasswordManager *pPassMgr);
	BOOL Reset(LPCTSTR pPattern, BOOL bCase, BOOL bEnc, BOOL bFileName, BOOL bNeg);

	////////////////////////////
	// VFStream implimentation

	// BOOL Prepare();		inherit to VFStream
	virtual BOOL Store(VFNote *p);
	// BOOL PostActivate();	inherit to VFStream
	// void FreeObject();	inherit to VFStream

	VFStream *Clone(VFStore **ppTail);
	BOOL GenerateXMLOpenTag(File *pFile);
	BOOL GenerateXMLCloseTag(File *pFile);

	LPCTSTR GetFilterType();
	BOOL ToString(TString *p);

	BOOL UpdateParamWithDialog(HINSTANCE hInst, HWND hParent);
};

////////////////////////////////////
// Limit number of notes
////////////////////////////////////

class VFLimitFilter : public VFStream {
#ifdef UNIT_TEST
public:
#endif
	DWORD nLimit;
	DWORD nCount;
public:
	VFLimitFilter();
	~VFLimitFilter();
	BOOL Init(DWORD nLimit);

	BOOL Prepare();
	BOOL Store(VFNote *p);

	VFStream *Clone(VFStore **ppTail);
	BOOL GenerateXMLOpenTag(File *pFile);
	BOOL GenerateXMLCloseTag(File *pFile);

	LPCTSTR GetFilterType();
	BOOL ToString(TString *p);

	BOOL UpdateParamWithDialog(HINSTANCE hInst, HWND hParent);
};

////////////////////////////////////
// Check timestamp
////////////////////////////////////

class VFTimestampFilter : public VFStream {
#ifdef UNIT_TEST
public:
#endif
	BOOL bNewer;
	DWORD nDeltaDays;
	UINT64 uBase;
public:
	VFTimestampFilter();
	~VFTimestampFilter();
	BOOL Reset(DWORD nDeltaDays, BOOL bNewer);
	BOOL Init(DWORD nDelta = 0, BOOL bNew = FALSE) { return Reset(nDelta, bNew); }

	// BOOL Prepare();
	BOOL Store(VFNote *p);

	VFStream *Clone(VFStore **ppTail);
	BOOL GenerateXMLOpenTag(File *pFile);
	BOOL GenerateXMLCloseTag(File *pFile);

	LPCTSTR GetFilterType();
	BOOL ToString(TString *p);

	BOOL UpdateParamWithDialog(HINSTANCE hInst, HWND hParent);
};

////////////////////////////////////
// Sort 
////////////////////////////////////

class VFSortFilter : public VFStream {
public:
	enum SortFuncType {
		SortFunc_Unknown,
		SortFunc_FileNameAsc,
		SortFunc_FileNameDsc,
		SortFunc_LastUpdateAsc,
		SortFunc_LastUpdateDsc,
		SortFunc_CreateDateAsc,
		SortFunc_CreateDateDsc,
		SortFunc_FileSizeAsc,
		SortFunc_FileSizeDsc
	};
#ifndef UNIT_TEST
protected:
#endif
	SortFuncType sfType;
	TVector<VFNote*> vNotes;
public:
	VFSortFilter();
	~VFSortFilter();
	BOOL Init(SortFuncType sf);

	BOOL Prepare();
	BOOL Store(VFNote *p);
	BOOL PostActivate();

	VFStream *Clone(VFStore **ppTail);
	BOOL GenerateXMLOpenTag(File *pFile);
	BOOL GenerateXMLCloseTag(File *pFile);

	LPCTSTR GetFilterType();
	BOOL ToString(TString *p);

	BOOL UpdateParamWithDialog(HINSTANCE hInst, HWND hParent);
};

#endif
