#ifndef TSCOMPILER_H
#define TSCOMPILER_H

class VFDirectoryGenerator;
class MemoSelectView;

enum TokenType {
	TOKEN_EOF,
	TOKEN_OTHER,
	TOKEN_STR,
	TOKEN_SEMICOLON,
	TOKEN_LBRA,
	TOKEN_RBRA,
	TOKEN_LPAR,
	TOKEN_RPAR,
	TOKEN_PIPE,
	TOKEN_KW_VFOLDER,
	TOKEN_KW_DIR,
	TOKEN_KW_STORE,
	TOKEN_KW_TITLE,
	TOKEN_KW_LAST_UPDATE
};

////////////////////////////////////
// TOMBO script parser
////////////////////////////////////

class TSParser {
	char *pScriptStr;	// target script

	// lexer working variable
	const char *token_ptr;
	const char *prev_token;
	enum TokenType nexttoken;

	// token value
	const char *val_start;
	const char *val_end;

	HTREEITEM hParent;
	MemoSelectView *pView;

	// lexer
	void gettoken();

	// parse functions
	BOOL ExprList();
	BOOL Expr();
	BOOL StreamDef(TreeViewVirtualFolder *p);
	BOOL DirStreamItem(TreeViewVirtualFolder *p);
	BOOL StoreStreamItem(TreeViewVirtualFolder *p);
	BOOL DirList(TreeViewVirtualFolder *p);
public:
	TSParser();
	~TSParser();

	BOOL Init(LPCTSTR pFileName, MemoSelectView *pView, HTREEITEM hItem);

	BOOL Compile();
};

#endif
