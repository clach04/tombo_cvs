#ifndef VFSTREAM_H
#define VFSTREAM_H

class MemoNote;

////////////////////////////////////
////////////////////////////////////
// Virtual Folder Node Stream
////////////////////////////////////
////////////////////////////////////
// implimentation of TOMBO script.


////////////////////////////////////
// stream item
////////////////////////////////////

class VFNote {
	MemoNote *pNote;
	LPTSTR pFileName;
public:
	VFNote() : pNote(NULL), pFileName(NULL) {}
	~VFNote();
	BOOL Init(MemoNote *p, LPCTSTR pFileName);

	MemoNote *GetNote() { return pNote; }
	LPCTSTR GetFileName() { return pFileName; }
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

	virtual BOOL Prepare() = 0;
	virtual BOOL Store(VFNote *p) = 0;
	virtual BOOL PostActivate() = 0;

	BOOL SetNext(VFStream *p);

	virtual void FreeObject() = 0;
};

////////////////////////////////////
// Directory generator
////////////////////////////////////
// In current version, VFDirectoryGenerator is only generator.

class VFDirectoryGenerator : public VFStream {
	LPTSTR pDirPath;
public:
	VFDirectoryGenerator();
	~VFDirectoryGenerator();
	void FreeObject();

	// pDir's memory is controled under VFDirectoryGenerator.
	// do not delete pDir after calling Init.
	BOOL Init(LPTSTR pDir);

	BOOL Prepare();
	BOOL Store(VFNote *p);
	BOOL PostActivate();

	BOOL Activate();

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

	VFNote **ppArray;
	DWORD nPos;
	DWORD nCapacity;

public:
	VFStore(enum OrderInfo odr);
	~VFStore();
	void FreeObject();

	BOOL Prepare();
	BOOL Store(VFNote *p);
	BOOL PostActivate();

	DWORD NumItem() { return nPos; }
	VFNote **GetNotes() { return ppArray; }

	// free VFNote array. 
	// VFNote in array is deleted, but MemoNote in each VFNote is not deleted.
	void FreeArray();
};


#endif