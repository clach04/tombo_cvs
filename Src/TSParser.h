#ifndef TSCOMPILER_H
#define TSCOMPILER_H

class MemoSelectView;
class ParseInfo;
class VFStream;
class TreeViewVirtualFolder;

////////////////////////////////////
// TOMBO script parser
////////////////////////////////////

class TSParser {
public:
	TSParser();
	~TSParser();

	BOOL Parse(LPCTSTR pFileName, MemoSelectView *pView, HTREEITEM hItem);
};

///////////////////////////////////////
// XML TAG abstruction
///////////////////////////////////////

class TSParseTagItem {
protected:
	DWORD nTagID;
	TSParseTagItem *pNext;

public:
	VFStream *pHead;
	VFStream *pTail;

	////////////////////////
	// ctor & dtor

	TSParseTagItem(DWORD nID) : nTagID(nID), pHead(NULL), pTail(NULL) {}
	virtual ~TSParseTagItem(); 

	////////////////////////
	// accessor & mutator

	DWORD GetTagID() { return nTagID; }

	////////////////////////
	// tag chain operation

	void SetNext(TSParseTagItem *p) { pNext = p; }
	TSParseTagItem *GetNext() { return pNext; }

	////////////////////////
	// Start element
	virtual BOOL StartElement(ParseInfo *p, const unsigned short **atts);

	////////////////////////
	// End element
	virtual BOOL EndElement(ParseInfo *p);
};

///////////////////////////////////////
// XML Parser helper
///////////////////////////////////////

class ParseInfo {
	MemoSelectView *pView;
	HTREEITEM hItem;

	BOOL bError;
	TSParseTagItem *pTop;
public:
	////////////////////////
	// ctor & dtor

	ParseInfo() : bError(FALSE), pTop(NULL) {}
	~ParseInfo();
	BOOL Init(MemoSelectView *p, HTREEITEM h);

	////////////////////////
	// Error info

	BOOL IsError() { return bError; }
	void SetError() { bError = TRUE; }

	////////////////////////
	// check & get tag Object

	DWORD GetTagID(const WCHAR *pTagName);
	TSParseTagItem *GetTagObjectFactory(DWORD nTagID);

	////////////////////////
	// Tag info operation

	void Push(TSParseTagItem *p);
	TSParseTagItem *Top() { return pTop; }
	void Pop();

	////////////////////////
	// Tag validation
	BOOL IsValidParent(DWORD nTag);

	////////////////////////
	// insert tree
	BOOL InsertTree(LPCTSTR pName, TreeViewVirtualFolder *pVF);
};


#endif
