#include <windows.h>
#include <tchar.h>

#include "UniConv.h"
#include "Repository.h"
#include "TString.h"
#include "TomboURI.h"
#include "MemoNote.h" // for implimentation
#include "PasswordManager.h"
#include "Message.h"
#include "AutoPtr.h"

#include "Property.h"
#include "MemoInfo.h"

/////////////////////////////////////////
// static funcs
/////////////////////////////////////////

#define DEFAULT_HEADLINE MSG_DEFAULT_HEADLINE

static BOOL GetHeadLineFromMemoText(LPCTSTR pMemo, TString *pHeadLine);
static int ChopFileNumberLen(LPTSTR pHeadLine);
static BOOL IsFileExist(LPCTSTR pFileName);

// IN:
//		pHeadLine   : �w�b�h���C�������� or ���x��������
//		pMemoPath   : �����̃p�X
//		pExt	    : �g���q(".txt" or ".chi")
// OUT:
//		pFullPath	: �t���p�X
//		pNewHeadLine: �C����w�b�h���C��������(=���x��������)
//		*ppNodePath	: �����̃p�X(�t���p�X����TOMBOROOTPATH������������)

static BOOL GetHeadLinePath(LPCTSTR pMemoPath, LPCTSTR pHeadLine, LPCTSTR pExt, 
							TString *pFullPath, LPTSTR *ppNotePath, TString *pNewHeadLine);

/////////////////////////////////////////
// Repository ctor & dtor, initializer
/////////////////////////////////////////

Repository::Repository() {}
Repository::~Repository() {}

/////////////////////////////////////////
// ctor & dtor, initializer
/////////////////////////////////////////

LocalFileRepository::LocalFileRepository() : pTopDir(NULL)
{
}

LocalFileRepository::~LocalFileRepository()
{
	delete[] pTopDir;
}

BOOL LocalFileRepository::Init(LPCTSTR pRoot)
{
	pTopDir = StringDup(pRoot);
	return TRUE;
}

/////////////////////////////////////////
// Create note
/////////////////////////////////////////

#ifdef COMMENT
BOOL LocalFileRepository::Create(LPCTSTR pRequestHeadLine, TomboURI *pTemplate, TString *pRealHeadLine, TomboURI *pRealURI)
{
	return TRUE;
}
#endif

/////////////////////////////////////////
// Update note
/////////////////////////////////////////

BOOL LocalFileRepository::Update(TomboURI *pCurrentURI, LPCTSTR pData, 
								 TomboURI *pNewURI, TString *pNewHeadLine)
{
	if (!Save(pCurrentURI, pData, pNewURI, pNewHeadLine)) {
		return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////
// Update's subfunction
/////////////////////////////////////////

BOOL LocalFileRepository::Save(const TomboURI *pCurrentURI, LPCTSTR pMemo, 
							   TomboURI *pNewURI, TString *pNewHeadLine)
{
	MemoNote *pNote = MemoNote::MemoNoteFactory(pCurrentURI);
	if (pNote == NULL) return FALSE;
	AutoPointer<MemoNote> apNote(pNote);

	TString sHeadLine;

	// get old headline from path
	if (!GetHeadLine(pCurrentURI, pNewHeadLine)) return FALSE;

	TString sOrigFile;
	if (!sOrigFile.Join(pTopDir, TEXT("\\"), pNote->pPath)) return FALSE;

	if (g_Property.KeepTitle()) {
		// clone original headline
		sHeadLine.Set(pNewHeadLine->Get());
	} else {
		// Get new headline from memo text
		if (!GetHeadLineFromMemoText(pMemo, &sHeadLine)) return FALSE;
		if (_tcslen(sHeadLine.Get()) == 0) {
			// headline is empty
			if (!sHeadLine.Set(MSG_DEFAULT_HEADLINE)) return FALSE;
		}
	}

	// Prepare write file.
	char *pText = ConvUnicode2SJIS(pMemo);
	if (pText == NULL) return FALSE;
	SecureBufferA sText(pText);

	DWORD nH = ChopFileNumberLen(pNewHeadLine->Get());
	DWORD nH2 = ChopFileNumberLen(sHeadLine.Get());

	// check headline has changed.
	BOOL bResult;
	if (nH == nH2 && _tcsncmp(pNewHeadLine->Get(), sHeadLine.Get(), nH) == 0) {
		bResult = SaveIfHeadLineIsNotChanged(pNote, pText, sOrigFile.Get());
	} else {
		bResult = SaveIfHeadLineIsChanged(pNote, pText, sOrigFile.Get(), 
									 sHeadLine.Get(), pNewHeadLine);
	}
	if (bResult) {
		bResult = pNote->GetURI(pNewURI);
	}
	return bResult;
}

BOOL LocalFileRepository::SaveIfHeadLineIsChanged(
	MemoNote *pNote, const char *pText, LPCTSTR pOrigFile, LPCTSTR pHeadLine, 
	TString *pNewHeadLine)
{
		LPTSTR pNotePath = pNote->pPath;

		// changed.
		TString sMemoDir;
		TString sNewFile;

		if (!sMemoDir.GetDirectoryPath(pNote->pPath)) return FALSE;
		if (!GetHeadLinePath(sMemoDir.Get(), pHeadLine, pNote->GetExtension(),
							 &sNewFile, &pNotePath, pNewHeadLine)) return FALSE;

		BOOL bResult = pNote->SaveData(g_pPassManager, pText, sNewFile.Get());
		if (bResult) {
			// delete original file
			DeleteFile(pOrigFile);

			// Additionally, rename memo info(*.tdt) file.
			if (g_Property.KeepCaret()) {
				MemoInfo mi;
				mi.RenameInfo(pOrigFile, sNewFile.Get());
			}

			// Update note's file path information. 
			LPTSTR pNewPath = StringDup(pNotePath);
			if (pNewPath == NULL) return FALSE;
			delete [] pNote->pPath;
			pNote->pPath = pNewPath;

			return TRUE;

		} else {
			// rollback (delete new writing file)
			DeleteFile(sNewFile.Get());
			return FALSE;
		}
}

BOOL LocalFileRepository::SaveIfHeadLineIsNotChanged(MemoNote *pNote, const char *pText, LPCTSTR pOrigFile)
{
	// Generate backup file name
	TString sBackupFile;
	if (!sBackupFile.Join(pOrigFile, TEXT(".tmp"))) return FALSE;
	// Backup(copy) original file
	//
	// Because ActiveSync can't treat Move&Write, backup operation uses not move but copy
	if (!CopyFile(pOrigFile, sBackupFile.Get(), FALSE)) {
		// if new file, copy are failed but it is OK.
		if (GetLastError() != ERROR_FILE_NOT_FOUND) return FALSE;
	}
	// Save to file
	if (!pNote->SaveData(g_pPassManager, pText, pOrigFile)) {
		// When save failed, try to rollback original file.
		DeleteFile(pOrigFile);
		MoveFile(sBackupFile.Get(), pOrigFile);
		return FALSE;
	}
	// remove backup file
	DeleteFile(sBackupFile.Get());
	return TRUE;
}

/////////////////////////////////////////////
// �^����ꂽ�����񂩂�t�@�C�����𐶐�����
/////////////////////////////////////////////

// IN:	pMemoPath	: �����̃p�X(TOPDIR����̑��΃p�X,�t�@�C�����͊܂܂�)
//		pHeadLine	: �w�b�h���C��������
//		pExt		: �t�^����g���q
// OUT:	pFullPath	: �����̃t���p�X
//		ppNotePath	: �����̃p�X(TOPDIR����̑��΃p�X,�t�@�C�������܂݁A
//					  �K�v�Ȃ�"(n)"�Ńf�B���N�g���ň�ӂƂȂ�悤�ɒ�������Ă���
//		pNewHeadLine: �ꗗ�\���p�V�w�b�h���C��(�K�v�ɉ�����"(n)"���t�^����Ă���)

static BOOL GetHeadLinePath(LPCTSTR pMemoPath, LPCTSTR pHeadLine, LPCTSTR pExt, 
							TString *pFullPath, LPTSTR *ppNotePath, TString *pNewHeadLine)
{
	DWORD n = _tcslen(pHeadLine);
	if (n < _tcslen(DEFAULT_HEADLINE)) n = _tcslen(DEFAULT_HEADLINE);

	DWORD nHeadLineLen = n + 20;
	DWORD nFullPathLen = _tcslen(g_Property.TopDir()) + 1 + 
						 _tcslen(pMemoPath) + nHeadLineLen + _tcslen(pExt);
	if (!pNewHeadLine->Alloc(nHeadLineLen + 1)) return FALSE;
	if (!pFullPath->Alloc(nFullPathLen + 1)) return FALSE;

	DropInvalidFileChar(pNewHeadLine->Get(), pHeadLine);
	if (_tcslen(pNewHeadLine->Get()) == 0) _tcscpy(pNewHeadLine->Get(), DEFAULT_HEADLINE);
	wsprintf(pFullPath->Get(), TEXT("%s\\%s%s"), g_Property.TopDir(), pMemoPath, pNewHeadLine->Get());

	LPTSTR p = pFullPath->Get();
	LPTSTR q = p + _tcslen(p);
	LPTSTR r = pNewHeadLine->Get() + _tcslen(pNewHeadLine->Get());

	*ppNotePath = pFullPath->Get() + _tcslen(g_Property.TopDir()) + 1;

	// �t�@�C�����̊m��
	// �����̃t�@�C�������݂����ꍇ�ɂ�"(n)"��t������
	_tcscpy(q, pExt);
	if (!IsFileExist(p)) return TRUE;

	DWORD i = 1;
	do {
		wsprintf(q, TEXT("(%d)%s"), i, pExt);
		wsprintf(r, TEXT("(%d)"), i);
		i++;
	} while(IsFileExist(p));
	return TRUE;
}

/////////////////////////////////////////////
// �w�b�h���C���̎擾
/////////////////////////////////////////////
// �����{������w�b�h���C���ƂȂ镶������擾����B
//
// �w�b�h���C����
// �E������1�s�ڂł���
// �E1�s�ڂ����ȏ�̒����̏ꍇ�A���̐擪����
// �Ƃ���B

static BOOL GetHeadLineFromMemoText(LPCTSTR pMemo, TString *pHeadLine)
{
	// �w�b�h���C�����̃J�E���g
	LPCTSTR p = pMemo;
	DWORD n = 0;
	while(*p) {
		if ((*p == TEXT('\r')) || (*p == TEXT('\n'))) break;
#ifndef _WIN32_WCE
		if (IsDBCSLeadByte(*p)) {
			p += 2;
			n += 2;
			continue;
		}
#endif
		n++;
		p++;
	}

	TString sHeadLineCand;
	if (!sHeadLineCand.Alloc(n + 1)) return FALSE;
	_tcsncpy(sHeadLineCand.Get(), pMemo, n);
	*(sHeadLineCand.Get() + n) = TEXT('\0');

	// �̈�m�ہE�R�s�[
	if (!pHeadLine->Alloc(n + 1)) return FALSE;
	DropInvalidFileChar(pHeadLine->Get(), sHeadLineCand.Get());
	TrimRight(pHeadLine->Get());

	return TRUE;
}

////////////////////////////////////////////////////////
// �w�b�h���C�������񂩂�"(n)"��������菜��
////////////////////////////////////////////////////////

static int ChopFileNumberLen(LPTSTR pHeadLine)
{
	if (*pHeadLine == TEXT('\0')) return 0;

	DWORD n = _tcslen(pHeadLine);
	LPTSTR p = pHeadLine + n - 1;
	if (*p != TEXT(')')) return n;
	p--;
	while(p >= pHeadLine) {
		if (*p == TEXT('(')) {
			return p - pHeadLine;
		}
		if (*p < TEXT('0') || *p > TEXT('9')) break;
		p--;
	}
	return n;
}

/////////////////////////////////////////////
// �t�@�C���̑��݃`�F�b�N
/////////////////////////////////////////////

static BOOL IsFileExist(LPCTSTR pFileName)
{
	HANDLE hFile = CreateFile(pFileName, GENERIC_READ, FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return FALSE;
	} else {
		CloseHandle(hFile);
		return TRUE;
	}
}

/////////////////////////////////////////
// Get Headline string
/////////////////////////////////////////

BOOL LocalFileRepository::GetHeadLine(const TomboURI *pURI, TString *pHeadLine)
{
	TomboURI *pThis = (TomboURI*)pURI;
	TomboURIItemIterator itr(pThis);
	if (!itr.Init()) return FALSE;

	if (!pHeadLine->Alloc(pURI->GetMaxPathItem() + 1)) return FALSE;
	_tcscpy(pHeadLine->Get(), TEXT("[root]"));

	LPCTSTR p;
	for (itr.First(); p = itr.Current(); itr.Next()) {
		_tcscpy(pHeadLine->Get(), p);
		if (itr.IsLeaf()) {
			DWORD n = _tcslen(pHeadLine->Get());
			if (n >= 4) {
				*(pHeadLine->Get() + n - 4) = TEXT('\0');
			}
		}
	}
	return TRUE;
}
