#include <windows.h>
#include <commctrl.h>
#define XMLPARSEAPI(type) type __cdecl	// for expat
#define XML_UNICODE_WCHAR_T
#include <expat.h>
#include "Tombo.h"
#include "UniConv.h"
#include "TSParser.h"
#include "VFStream.h"
#include "TreeViewItem.h"
#include "MemoSelectView.h"

class ConvertWideToMultiByte {
	char *p;
public:
	ConvertWideToMultiByte() : p(NULL) {}
	~ConvertWideToMultiByte() { delete []p; }

	char *Convert(WCHAR *p);
	char *Get() { return p; }
};

char *ConvertWideToMultiByte::Convert(WCHAR *pSrc)
{
	if (pSrc == NULL) return NULL;

	DWORD n = (wcslen(pSrc) + 1) * 2;
	p = new char[n];
	if (p == NULL) return NULL;
	WideCharToMultiByte(CP_ACP, 0, pSrc, -1, p, n, NULL, NULL);
	return p;
}

///////////////////////////////////////
// TSParser ctor&dtor
///////////////////////////////////////

TSParser::TSParser()
{
}

TSParser::~TSParser()
{
}

///////////////////////////////////////
// XML tag info 
///////////////////////////////////////

#define TAGID_UNKNOWN	0
#define TAGID_INITIAL	1
#define TAGID_FOLDERS	2
#define TAGID_VFOLDER	3
#define TAGID_GREP		4
#define TAGID_SRC		5

static DWORD nAllowParent[] = {
	0,						// TAGID_UNKONWN
	0,						// TAGID_INITIAL
	(1 << TAGID_INITIAL),	// TAGID_FOLDERS
	(1 << TAGID_FOLDERS),	// TAGID_VFOLDER
	(1 << TAGID_VFOLDER) | (1 << TAGID_GREP),
							// TAGID_GREP
	(1 << TAGID_VFOLDER) | (1 << TAGID_GREP),
							// TAGID_SRC
};

///////////////////////////////////////
// TAG data
///////////////////////////////////////

TSParseTagItem::~TSParseTagItem()
{
}

BOOL TSParseTagItem::StartElement(ParseInfo *p, const XML_Char **atts)
{
	return TRUE;
}

BOOL TSParseTagItem::EndElement(ParseInfo *p)
{
	return TRUE;
}

///////////////////////////////////////
//  "src" tag implimentation
///////////////////////////////////////

class TSSrcTag : public TSParseTagItem {
	WCHAR *pSrc;
	BOOL bCheckEncrypt;
public:
	TSSrcTag() : TSParseTagItem(TAGID_SRC), pSrc(NULL), bCheckEncrypt(FALSE) {}
	~TSSrcTag();

	BOOL StartElement(ParseInfo *p, const XML_Char **atts);
	BOOL EndElement(ParseInfo *p);
};

TSSrcTag::~TSSrcTag()
{
	if (pSrc) delete [] pSrc;
}

BOOL TSSrcTag::StartElement(ParseInfo *p, const XML_Char **atts)
{
	bCheckEncrypt = FALSE;

	DWORD i = 0;
	while(atts[i] != NULL) {
		if (wcsicmp(atts[i], L"folder") == 0) {
			pSrc = new WCHAR[wcslen(atts[i + 1]) + 1];
			if (pSrc == NULL) {
				SetLastError(ERROR_NOT_ENOUGH_MEMORY);
				return FALSE;
			}
			wcscpy(pSrc, atts[i + 1]);
		} else if (wcsicmp(atts[i], L"checkencrypt") == 0) {
			bCheckEncrypt = TRUE;
		}

		i += 2;
	}

	if (!pSrc) {
		// necessary attribute is not found.
		return FALSE;
	}
	return TRUE;
}

BOOL TSSrcTag::EndElement(ParseInfo *p)
{
	if (pHead != NULL) {
		// "src" tag can't have sub items.
		return FALSE;
	}

	VFDirectoryGenerator *pGen = new VFDirectoryGenerator();
	if (pGen == NULL) return FALSE;
#ifdef _WIN32_WCE
	LPTSTR pConved = StringDup(pSrc);
#else
	ConvertWideToMultiByte conv;
	if (!conv.Convert(pSrc)) return FALSE;
	LPTSTR pConved = StringDup(conv.Get());
#endif
	if (!pGen->Init(pConved, bCheckEncrypt)) return FALSE;
	
	// Pass create object to parent item
	TSParseTagItem *pParent = pNext;
	pParent->pHead = pParent->pTail = pGen;
	return TRUE;
}

///////////////////////////////////////
// "grep" tag implimentation
///////////////////////////////////////

class TSGrepTag : public TSParseTagItem {
	WCHAR *pPattern;
	BOOL bCaseSensitive;
	BOOL bFileNameOnly;
	BOOL bNegate;
public:
	TSGrepTag() : TSParseTagItem(TAGID_GREP), pPattern(NULL) {}
	~TSGrepTag();

	BOOL StartElement(ParseInfo *p, const XML_Char **atts);
	BOOL EndElement(ParseInfo *p);
};

TSGrepTag::~TSGrepTag()
{
	if (pPattern) delete [] pPattern;
}

BOOL TSGrepTag::StartElement(ParseInfo *p, const XML_Char **atts)
{
	bCaseSensitive = bFileNameOnly = bNegate = FALSE;
	DWORD i = 0;
	while(atts[i] != NULL) {
		if (wcsicmp(atts[i], L"pattern") == 0) {
			pPattern = new WCHAR[wcslen(atts[i + 1]) + 1];
			if (pPattern == NULL) return FALSE;
			wcscpy(pPattern, atts[i + 1]);
		} else if (wcsicmp(atts[i], L"casesensitive") == 0) {
			bCaseSensitive = TRUE;
		} else if (wcsicmp(atts[i], L"filenameonly") == 0) {
			bFileNameOnly = TRUE;
		} else if (wcsicmp(atts[i], L"not") == 0) {
			bNegate = TRUE;
		}
		i += 2;
	}
	if (!pPattern) {
		return FALSE;
	}
	return TRUE;
}

BOOL TSGrepTag::EndElement(ParseInfo *p)
{
	if (pHead == NULL) {
		// Grep tag should have child tag.
		return FALSE;
	}
	VFRegexFilter *pFilter = new VFRegexFilter();

#ifdef _WIN32_WCE
	LPTSTR pConved = pPattern;
#else
	ConvertWideToMultiByte conv;
	if (!conv.Convert(pPattern)) return FALSE;
	LPTSTR pConved = conv.Get();
#endif
	if (!pFilter || !pFilter->Init(pConved, bCaseSensitive, TRUE, bFileNameOnly, bNegate, g_pPasswordManager)) return FALSE;

	// Pass create object to parent item
	TSParseTagItem *pParent = pNext;
	pTail->SetNext(pFilter);
	pParent->pHead = pHead;
	pParent->pTail = pFilter;
	return TRUE;
}

///////////////////////////////////////
// "vfolder" tag implimentation
///////////////////////////////////////

class TSVFolderTag : public TSParseTagItem {
	WCHAR *pName;
public:
	TSVFolderTag() : TSParseTagItem(TAGID_VFOLDER), pName(NULL){}
	~TSVFolderTag();

	BOOL StartElement(ParseInfo *p, const XML_Char **atts);
	BOOL EndElement(ParseInfo *p);
};

TSVFolderTag::~TSVFolderTag()
{
	if (pName) delete[] pName;
}

BOOL TSVFolderTag::StartElement(ParseInfo *p, const XML_Char **atts)
{
	DWORD i = 0;
	while(atts[i] != NULL) {
		if (wcsicmp(atts[i], L"name") == 0) {
			pName = new WCHAR[wcslen(atts[i + 1]) + 1];
			if (pName == NULL) return FALSE;
			wcscpy(pName, atts[i + 1]);
		}
		i += 2;
	}
	if (!pName) {
		return FALSE;
	}
	return TRUE;
}

BOOL TSVFolderTag::EndElement(ParseInfo *p)
{
	if (pHead == NULL || pTail == NULL) return FALSE;

	TreeViewVirtualFolder *pVF = new TreeViewVirtualFolder();
	if (!pVF) return FALSE;
	VFStore *pStore = new VFStore(VFStore::ORDER_TITLE);
	if (!pStore || !pStore->Init()) {
		delete pVF;
		delete pStore;
		return FALSE;
	}

	pTail->SetNext(pStore);

	pVF->SetGenerator((VFDirectoryGenerator*)pHead);
	pVF->SetStore(pStore);

#ifdef _WIN32_WCE
	LPTSTR pConved = pName;
#else
	ConvertWideToMultiByte conv;
	if (!conv.Convert(pName)) return FALSE;
	LPTSTR pConved = conv.Get();
#endif

	p->InsertTree(pConved, pVF);
	return TRUE;
}

///////////////////////////////////////
// ParseInfo implimentation
///////////////////////////////////////

ParseInfo::~ParseInfo()
{
	TSParseTagItem *p = pTop;
	TSParseTagItem *q;
	while(p) {
		q = p;
		p = p->GetNext();
		delete q;
	}
}

BOOL ParseInfo::Init(MemoSelectView *p, HTREEITEM h)
{
	pView =p;
	hItem = h;
	TSParseTagItem *pTag = new TSParseTagItem(TAGID_INITIAL);
	if (pTag == NULL) return FALSE;
	Push(pTag);
	return TRUE;
}

DWORD ParseInfo::GetTagID(const WCHAR *pTagName)
{
	if (wcsicmp(pTagName, L"folders") == 0) {
		return TAGID_FOLDERS;
	} else if (wcsicmp(pTagName, L"vfolder") == 0) {
		return TAGID_VFOLDER;
	} else if (wcsicmp(pTagName, L"grep") == 0) {
		return TAGID_GREP;
	} else if (wcsicmp(pTagName, L"src") == 0) {
		return TAGID_SRC;
	} else {
		return TAGID_UNKNOWN;
	}
}

TSParseTagItem *ParseInfo::GetTagObjectFactory(DWORD nTagID)
{
	switch (nTagID) {
	case TAGID_FOLDERS:
		return new TSParseTagItem(TAGID_FOLDERS);
	case TAGID_VFOLDER:
		return new TSVFolderTag();
	case TAGID_GREP:
		return new TSGrepTag();
	case TAGID_SRC:
		return new TSSrcTag();
	default:
		return NULL;
	}
}

void ParseInfo::Push(TSParseTagItem *p)
{
	p->SetNext(pTop); 
	pTop = p;
}

void ParseInfo::Pop()
{
	TSParseTagItem *p = pTop;
	pTop = p->GetNext();
	delete p;
}

BOOL ParseInfo::IsValidParent(DWORD nTag)
{
	return ((nAllowParent[nTag] & (1 << pTop->GetTagID())) != 0);
}

BOOL ParseInfo::InsertTree(LPCTSTR pName, TreeViewVirtualFolder *pVF)
{
	return pView->InsertFolder(hItem, pName, pVF, TRUE) != NULL;
}

///////////////////////////////////////
// expat callback funcs.
///////////////////////////////////////

static void StartElement(void *userData, const XML_Char *name, const XML_Char **atts)
{
	ParseInfo *pInfo = (ParseInfo*)userData;
	if (pInfo->IsError()) return;

	// Check tag
	DWORD nCurTag = pInfo->GetTagID(name);
	if (nCurTag == TAGID_UNKNOWN) {
		pInfo->SetError();
		return;
	}
	if (!pInfo->IsValidParent(nCurTag)) {
		pInfo->SetError();
		return;
	}
	TSParseTagItem *pTag = pInfo->GetTagObjectFactory(nCurTag);
	pInfo->Push(pTag);
	if (!pTag->StartElement(pInfo, atts)) {
		pInfo->SetError();
	}
}

static void EndElement(void *userData, const XML_Char *name)
{
	ParseInfo *pInfo = (ParseInfo*)userData;
	if (pInfo->IsError()) {
		pInfo->Pop();
		return;
	}
	pInfo->Top()->EndElement(pInfo);
	pInfo->Pop();
}

///////////////////////////////////////
// parser main
///////////////////////////////////////

BOOL TSParser::Parse(LPCTSTR pFileName, MemoSelectView *pView, HTREEITEM hItem)
{
	XML_Parser pParser;
	ParseInfo info;

	if (!info.Init(pView, hItem)) return FALSE;

	HANDLE hFile = CreateFile(pFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) return FALSE;

	DWORD nFileSize = GetFileSize(hFile, NULL);

	pParser = XML_ParserCreate(NULL);
	if (pParser == NULL) {
		CloseHandle(hFile);
		return FALSE;
	}

	XML_SetElementHandler(pParser, StartElement, EndElement);
	XML_SetUserData(pParser, &info);

	void *pBuf = XML_GetBuffer(pParser, nFileSize);
	if (pBuf == NULL) {
		CloseHandle(hFile);
		return FALSE;
	}
	DWORD nRead;
	if (!ReadFile(hFile, pBuf, nFileSize, &nRead, NULL)) {
		CloseHandle(hFile);
		return FALSE;
	}

	CloseHandle(hFile);

	if (!XML_ParseBuffer(pParser, nFileSize, TRUE)) {
		const WCHAR *p = XML_ErrorString(XML_GetErrorCode(pParser));
		int ln = XML_GetCurrentLineNumber(pParser);
		int col = XML_GetCurrentColumnNumber(pParser);
		return FALSE;
	}
	XML_ParserFree(pParser);
	return TRUE;
}
