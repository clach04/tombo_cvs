#include <windows.h>
#include <commctrl.h>
#include "Uniconv.h"
#include "MemoSelectView.h"
#include "TreeViewItem.h"
#include "File.h"
#include "VFStream.h"
#include "TSParser.h"


#if defined(PLATFORM_HPC) || defined(PLATFORM_PSPC)
extern "C" {
#include "ctypeutil.h"
};
int _strnicmp(const char *p1, const char *p2, int n);
#endif

///////////////////////////////////////////////
///////////////////////////////////////////////
// functions
///////////////////////////////////////////////
///////////////////////////////////////////////

///////////////////////////////////////////////
// TSParser
///////////////////////////////////////////////

TSParser::TSParser() : hParent(NULL), pView(NULL), pScriptStr(NULL)
{
}

TSParser::~TSParser()
{
	delete [] pScriptStr;
}

BOOL TSParser::Init(LPCTSTR pFileName, MemoSelectView *p, HTREEITEM hItem)
{
	hParent = hItem;
	pView = p;

	File inf;
	if (!inf.Open(pFileName, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING)) return FALSE;
	char *pText = new char[inf.FileSize() + 1];
	if (pText == NULL) return FALSE;

	DWORD nSize = inf.FileSize();
	if (!inf.Read((LPBYTE)pText, &nSize)) {
		delete [] pText;
		return FALSE;
	}
	pText[nSize] = TEXT('\0');

	pScriptStr = pText;

	return TRUE;
}

BOOL TSParser::Compile()
{
	token_ptr = pScriptStr;
	prev_token = "";

	gettoken();
	// TODO: Error Handling
	BOOL bRes = ExprList();
	if (!bRes) {
		MessageBox(NULL, TEXT("Compile error"), TEXT("DEBUG"), MB_OK);
	}
	return bRes;
}

///////////////////////////////////////////////
// lexer
///////////////////////////////////////////////

void TSParser::gettoken()
{
	const char *in;
	const char *prev;

gettoken_func_top:

	// skip space
	while(*token_ptr && (*token_ptr == ' ' || *token_ptr == '\t' ||
			     *token_ptr == '\r' || *token_ptr == '\n')) {
		token_ptr++;
	}

	// skip comment line
	if (*token_ptr == '#') {
		while(*token_ptr && *token_ptr != '\r' && *token_ptr != '\n') {
			if (IsDBCSLeadByte(*token_ptr)) {
				token_ptr++;
			}
			token_ptr++;
		}
		goto gettoken_func_top;
	}

	prev = prev_token;
	prev_token = in = token_ptr;

	if (_strnicmp(token_ptr, "vfolder", 7) == 0 && !isalnum(*(token_ptr+7))) {
		token_ptr += 7;
		nexttoken = TOKEN_KW_VFOLDER;
	} else if (_strnicmp(token_ptr, "dir", 3) == 0 && !isalnum(*(token_ptr+3))) {
		token_ptr += 3;
		nexttoken = TOKEN_KW_DIR;
	} else if (_strnicmp(token_ptr, "store", 5) == 0 && !isalnum(*(token_ptr+5))) {
		token_ptr += 5;
		nexttoken = TOKEN_KW_STORE;
	} else if (_strnicmp(token_ptr, "TITLE", 5) == 0 && !isalnum(*(token_ptr+5))) {
		token_ptr += 5;
		nexttoken = TOKEN_KW_TITLE;
	} else if (_strnicmp(token_ptr, "LAST_UPDATE", 11) == 0 && !isalnum(*(token_ptr+11))) {
		token_ptr += 11;
		nexttoken = TOKEN_KW_LAST_UPDATE;

	} else if (*token_ptr == '"' || *token_ptr == '\'') {
		char sep = *token_ptr++;

		val_start = token_ptr;
		while(*token_ptr) {
			if (sep == '"' && *token_ptr == '\\' && *(token_ptr+1) == '"') {
				// escape '\x'
				token_ptr += 2;
				continue;
			}
			if (IsDBCSLeadByte(*token_ptr)) {
				token_ptr++;
				if (*token_ptr) {
					token_ptr++;
					continue;
				} else {
					break;
				}
			}
			if (*token_ptr == sep) {
				token_ptr++;
				val_end = token_ptr;

				nexttoken = TOKEN_STR;
				return;
			}
			token_ptr++;
		}
		prev_token = prev;
		token_ptr = in;
		nexttoken = TOKEN_OTHER;
	} else if (*token_ptr == '\0') {
		nexttoken = TOKEN_EOF;
	} else if (*token_ptr == ';') {
		token_ptr++;
		nexttoken = TOKEN_SEMICOLON;
	} else if (*token_ptr == '[') {
		token_ptr++;
		nexttoken = TOKEN_LBRA;
	} else if (*token_ptr == '(') {
		token_ptr++;
		nexttoken = TOKEN_LPAR;
	} else if (*token_ptr == ')') {
		token_ptr++;
		nexttoken = TOKEN_RPAR;
	} else if (*token_ptr == ']') {
		token_ptr++;
		nexttoken = TOKEN_RBRA;
	} else if (*token_ptr == '|') {
		token_ptr++;
		nexttoken = TOKEN_PIPE;
	} else {
		nexttoken = TOKEN_OTHER;
	}
}

///////////////////////////////////////////////
// parser body
///////////////////////////////////////////////

// ExprList -> {Expr} $
BOOL TSParser::ExprList()
{
	while(nexttoken == TOKEN_KW_VFOLDER) {
		if (!Expr()) return FALSE;
	}
	if (nexttoken != TOKEN_EOF) return FALSE;
	return TRUE;
}

// Expr -> "vfolder" STR '[' StreamDef ']' ';'
BOOL TSParser::Expr()
{
	if (nexttoken != TOKEN_KW_VFOLDER) return FALSE;
	gettoken();

	if (nexttoken != TOKEN_STR) return FALSE;
	LPTSTR pNodeName = ConvSJIS2UnicodeWithByte(val_start, val_end - val_start - 1);
	gettoken();

	TreeViewVirtualFolder *pVf = new TreeViewVirtualFolder();

	if (nexttoken != TOKEN_LBRA) {
		delete pVf;
		return FALSE;
	}
	gettoken();

	if (!StreamDef(pVf)) {
		delete pVf;
		return FALSE;
	}

	if (nexttoken != TOKEN_RBRA) {
		delete pVf;
		return FALSE;
	}
	gettoken();

	if (nexttoken != TOKEN_SEMICOLON) {
		delete pVf;
		return FALSE;
	}
	gettoken();

	// Insert virtual folder to MemoSelectView
	pView->InsertFolderToLast(hParent, pNodeName, pVf);

	delete [] pNodeName;

	return TRUE;
}

// StreamDef -> DirStreamItem '|' StoreStreamItem
// TODO: Generalize expressions.
BOOL TSParser::StreamDef(TreeViewVirtualFolder *pVf)
{
	if (!DirStreamItem(pVf)) return FALSE;

	if (nexttoken != TOKEN_PIPE) return FALSE;
	gettoken();

	if (!StoreStreamItem(pVf)) return FALSE;
	return TRUE;
}

// StreamItem -> "dir" '(' DirList ')'
BOOL TSParser::DirStreamItem(TreeViewVirtualFolder *pVf)
{
	if (nexttoken != TOKEN_KW_DIR) return FALSE;
	gettoken();

	if (nexttoken != TOKEN_LPAR) return FALSE;
	gettoken();

	if (!DirList(pVf)) return FALSE;

	if (nexttoken != TOKEN_RPAR) return FALSE;
	gettoken();

	return TRUE;
}

// DirList -> STR
// TODO: accept multi STR's 
BOOL TSParser::DirList(TreeViewVirtualFolder *pVf)
{
	if (nexttoken != TOKEN_STR) return FALSE;
	LPTSTR pDirPath = ConvSJIS2UnicodeWithByte(val_start, val_end - val_start - 1);
	gettoken();

	VFDirectoryGenerator *pDirGen = new VFDirectoryGenerator();
	if (pDirGen == NULL) {
		delete [] pDirPath;
		return FALSE;
	}
	if (!pDirGen->Init(pDirPath)) {
		delete pDirGen;
		delete [] pDirPath;
		return FALSE;
	}

	pVf->SetGenerator(pDirGen);

	return TRUE;
}

// StoreSearchItem -> 'store' '(' ('TITLE' | 'LAST_UPDATE') ')'
BOOL TSParser::StoreStreamItem(TreeViewVirtualFolder *pVf)
{
	if (nexttoken != TOKEN_KW_STORE) return FALSE;
	gettoken();
	if (nexttoken != TOKEN_LPAR) return FALSE;
	gettoken();

	if (nexttoken != TOKEN_KW_TITLE && nexttoken != TOKEN_KW_LAST_UPDATE) return FALSE;

	VFStore *pStore = new VFStore((nexttoken == TOKEN_KW_TITLE) ? VFStore::ORDER_TITLE : VFStore::ORDER_LAST_UPD);
	if (pStore == NULL) return FALSE;

	gettoken();

	if (nexttoken != TOKEN_RPAR) {
		delete pStore;
		return FALSE;
	}
	gettoken();

	pVf->SetStore(pStore);

	return TRUE;
}
