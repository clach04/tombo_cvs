#ifndef SEARCHDLG_H
#define SEARCHDLG_H

#include "Property.h"

////////////////////////////////////////////////////////////////
// �����_�C�A���O
////////////////////////////////////////////////////////////////

class SearchDialog {
	HWND hDialog;
	HINSTANCE hInstance;

	LPTSTR pSearchStr;
	BOOL bCaseSensitive;
	BOOL bCheckEncrypt;
	BOOL bFileNameOnly;
	BOOL bSearchDirectionUp;
public:

	////////////////////////////////
	// ctor & dtor

	SearchDialog() : hDialog(NULL), hInstance(NULL), pSearchStr(NULL), bCaseSensitive(FALSE), bFileNameOnly(FALSE), bSearchDirectionUp(FALSE) {}
	~SearchDialog();

	////////////////////////////////
	// �C�x���g�n���h��

	void InitDialog(HWND hDlg);

	BOOL OnOK();

	////////////////////////////////
	// �_�C�A���O�N��

	DWORD Popup(HINSTANCE hInst, HWND hParent, BOOL bCheckEncrypt);

	////////////////////////////////
	// ���ʎ擾

	LPCTSTR SearchString() { return pSearchStr; }
	BOOL IsCaseSensitive() { return bCaseSensitive; }
	BOOL IsSearchEncryptMemo() { return bCheckEncrypt; }
	BOOL IsFileNameOnly() { return bFileNameOnly; }
	BOOL IsSearchDirectionUp() { return bSearchDirectionUp; }
};


#endif
