#include <windows.h>
#include <tchar.h>

#include "Message.h"
#include "UniConv.h"
#include "TString.h"
#include "TomboURI.h"
#include "Repository.h"
#include "RepositoryImpl.h"

#include "MemoNote.h"
#include "AutoPtr.h"
#include "PasswordManager.h"
#include "MemoInfo.h"

/////////////////////////////////////////
// static funcs
/////////////////////////////////////////

#define DEFAULT_HEADLINE MSG_DEFAULT_HEADLINE

static BOOL GetHeadLineFromMemoText(LPCTSTR pMemo, TString *pHeadLine);
static int ChopFileNumberLen(LPTSTR pHeadLine);
static BOOL IsFileExist(LPCTSTR pFileName);

// IN:
//		pHeadLine   : ヘッドライン文字列 or ラベル文字列
//		pMemoPath   : メモのパス
//		pExt	    : 拡張子(".txt" or ".chi")
// OUT:
//		pFullPath	: フルパス
//		pNewHeadLine: 修正後ヘッドライン文字列(=ラベル文字列)
//		*ppNodePath	: メモのパス(フルパスからTOMBOROOTPATHを除いたもの)

static BOOL GetHeadLinePath(LPCTSTR pMemoPath, LPCTSTR pHeadLine, LPCTSTR pExt, 
							TString *pFullPath, LPTSTR *ppNotePath, TString *pNewHeadLine,
							LPCTSTR pTopDir);

/////////////////////////////////////////
// Repository ctor & dtor, initializer
/////////////////////////////////////////

RepositoryImpl::RepositoryImpl() {}
RepositoryImpl::~RepositoryImpl() {}

/////////////////////////////////////////
// Is the note encrypted?
/////////////////////////////////////////

BOOL RepositoryImpl::IsEncrypted(const TomboURI *pURI)
{
	URIOption opt(NOTE_OPTIONMASK_ENCRYPTED);
	GetOption(pURI, &opt);
	return opt.bEncrypt;
}

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

BOOL LocalFileRepository::Init(LPCTSTR pRoot, const RepositoryOption *p)
{
	pOpt = p;
	pTopDir = StringDup(pRoot);
	return TRUE;
}

/////////////////////////////////////////
// Create note
/////////////////////////////////////////

BOOL LocalFileRepository::Create(const TomboURI *pTemplate, LPCTSTR pData, TString *pRealHeadLine, TomboURI *pAllocedURI)
{
	return TRUE;
}

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

	if (pOpt->bKeepTitle) {
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
							 &sNewFile, &pNotePath, pNewHeadLine, pTopDir)) return FALSE;

		BOOL bResult = pNote->SaveData(g_pPassManager, pText, sNewFile.Get());
		if (bResult) {
			// delete original file
			DeleteFile(pOrigFile);

			// Additionally, rename memo info(*.tdt) file.
			if (pOpt->bKeepCaret) {
				MemoInfo mi(pTopDir);
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
// 与えられた文字列からファイル名を生成する
/////////////////////////////////////////////

// IN:	pMemoPath	: メモのパス(TOPDIRからの相対パス,ファイル名は含まず)
//		pHeadLine	: ヘッドライン文字列
//		pExt		: 付与する拡張子
// OUT:	pFullPath	: メモのフルパス
//		ppNotePath	: メモのパス(TOPDIRからの相対パス,ファイル名を含み、
//					  必要なら"(n)"でディレクトリで一意となるように調整されている
//		pNewHeadLine: 一覧表示用新ヘッドライン(必要に応じて"(n)"が付与されている)

static BOOL GetHeadLinePath(LPCTSTR pMemoPath, LPCTSTR pHeadLine, LPCTSTR pExt, 
							TString *pFullPath, LPTSTR *ppNotePath, TString *pNewHeadLine,
							LPCTSTR pTopDir)
{
	DWORD n = _tcslen(pHeadLine);
	if (n < _tcslen(DEFAULT_HEADLINE)) n = _tcslen(DEFAULT_HEADLINE);

	DWORD nHeadLineLen = n + 20;
	DWORD nFullPathLen = _tcslen(pTopDir) + 1 + 
						 _tcslen(pMemoPath) + nHeadLineLen + _tcslen(pExt);
	if (!pNewHeadLine->Alloc(nHeadLineLen + 1)) return FALSE;
	if (!pFullPath->Alloc(nFullPathLen + 1)) return FALSE;

	DropInvalidFileChar(pNewHeadLine->Get(), pHeadLine);
	if (_tcslen(pNewHeadLine->Get()) == 0) _tcscpy(pNewHeadLine->Get(), DEFAULT_HEADLINE);
	wsprintf(pFullPath->Get(), TEXT("%s\\%s%s"), pTopDir, pMemoPath, pNewHeadLine->Get());

	LPTSTR p = pFullPath->Get();
	LPTSTR q = p + _tcslen(p);
	LPTSTR r = pNewHeadLine->Get() + _tcslen(pNewHeadLine->Get());

	*ppNotePath = pFullPath->Get() + _tcslen(pTopDir) + 1;

	// ファイル名の確定
	// 同名のファイルが存在した場合には"(n)"を付加する
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
// ヘッドラインの取得
/////////////////////////////////////////////
// メモ本文からヘッドラインとなる文字列を取得する。
//
// ヘッドラインは
// ・メモの1行目である
// ・1行目が一定以上の長さの場合、その先頭部分
// とする。

static BOOL GetHeadLineFromMemoText(LPCTSTR pMemo, TString *pHeadLine)
{
	// ヘッドライン長のカウント
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

	// 領域確保・コピー
	if (!pHeadLine->Alloc(n + 1)) return FALSE;
	DropInvalidFileChar(pHeadLine->Get(), sHeadLineCand.Get());
	TrimRight(pHeadLine->Get());

	return TRUE;
}

////////////////////////////////////////////////////////
// ヘッドライン文字列から"(n)"部分を取り除く
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
// ファイルの存在チェック
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

/////////////////////////////////////////
// Get option information
/////////////////////////////////////////

BOOL LocalFileRepository::GetOption(const TomboURI *pURI, URIOption *pOption) const
{
	if (pOption->nFlg & NOTE_OPTIONMASK_ENCRYPTED) {
		pOption->bEncrypt = pURI->IsEncrypted();
	}
	return TRUE;
}

/////////////////////////////////////////
// Set option information
/////////////////////////////////////////

BOOL LocalFileRepository::SetOption(const TomboURI *pCurrentURI, URIOption *pOption)
{
	if (pOption->nFlg & NOTE_OPTIONMASK_ENCRYPTED) {
		if (pCurrentURI->IsLeaf()) {
			// set option to file
			if (pOption->bEncrypt) {
				// encrypt
				return EncryptLeaf(pCurrentURI, pOption);
			} else {
				// decrypt
				return DecryptLeaf(pCurrentURI, pOption);
			}
		} else {
			// set option to folder
			// TODO: impliment
		}
	}
	return TRUE;
}

/////////////////////////////////////////
// Encrypt/decrypt subroutines
/////////////////////////////////////////

BOOL LocalFileRepository::EncryptLeaf(const TomboURI *pCurrentURI, URIOption *pOption)
{
	BOOL b;
	MemoNote *pCur = MemoNote::MemoNoteFactory(pCurrentURI);
	AutoPointer<MemoNote> ap(pCur);

	pOption->pNewHeadLine = new TString();
	if (pOption->pNewHeadLine == NULL) return FALSE;

	MemoNote *p = pCur->Encrypt(g_pPassManager, pOption->pNewHeadLine, &b);
	if (p == NULL) return FALSE;

	pOption->pNewNote = p;

	if (!pCur->DeleteMemoData()) {
		pOption->nErrorCode = MSG_ID_DELETE_PREV_CRYPT_MEMO_FAILED;
		pOption->iLevel = MB_ICONWARNING;
		return FALSE;
	}

	return TRUE;
}

BOOL LocalFileRepository::DecryptLeaf(const TomboURI *pCurrentURI, URIOption *pOption)
{
	BOOL b;
	MemoNote *pCur = MemoNote::MemoNoteFactory(pCurrentURI);
	AutoPointer<MemoNote> ap(pCur);

	if ((pOption->pNewHeadLine = new TString()) == NULL) return FALSE;

	MemoNote *p = pCur->Decrypt(g_pPassManager, pOption->pNewHeadLine, &b);
	if (p == NULL) return FALSE;

	pOption->pNewNote = p;
	if (!pCur->DeleteMemoData()) {
		pOption->nErrorCode = MSG_ID_DEL_PREV_DECRYPT_MEMO_FAILED;
		pOption->iLevel = MB_ICONWARNING;
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////
// Delete Note/folder
/////////////////////////////////////////

BOOL LocalFileRepository::Delete(const TomboURI *pURI, URIOption *pOption)
{
	if (pURI->IsLeaf()) {
		MemoNote *pNote = MemoNote::MemoNoteFactory(pURI);
		if (pNote == NULL) return FALSE;
		AutoPointer<MemoNote> ap(pNote);

		return pNote->DeleteMemoData();
	} else {
		// TODO : impliment
	}
	return TRUE;
}

/////////////////////////////////////////
// Get Physical file/folder path
/////////////////////////////////////////
BOOL LocalFileRepository::GetPhysicalPath(const TomboURI *pURI, TString *pFullPath)
{

	if (!pFullPath->Alloc(_tcslen(pTopDir) + _tcslen(pURI->GetPath()) + 1)) return FALSE;
	LPCTSTR p = pURI->GetPath();
	_tcscpy(pFullPath->Get(), pTopDir);
	LPTSTR q = pFullPath->Get() + _tcslen(pFullPath->Get());

	while (*p) {
#if defined(PLATFORM_WIN32)
		if (IsDBCSLeadByte(*p)) {
			*q++ = *p++;
		}
#endif
		if (*p == TEXT('/')) {
			*q++ = TEXT('\\');
			p++;
			continue;
		}
		*q++ = *p++;
	}
	*q = TEXT('\0');
	return TRUE;
}

/////////////////////////////////////////
// Copy note/folder
/////////////////////////////////////////

BOOL LocalFileRepository::Copy(const TomboURI *pCopyFrom, const TomboURI *pCopyTo, URIOption *pOption)
{
	MemoNote *pNote = MemoNote::MemoNoteFactory(pCopyFrom);
	if (pNote == NULL) return FALSE;
	AutoPointer<MemoNote> ap(pNote);

	TString sToPath;
	if (!GetPhysicalPath(pCopyTo, &sToPath)) return FALSE;

	LPCTSTR pMemoPath = sToPath.Get() + _tcslen(pTopDir) + 1;

	if ((pOption->pNewHeadLine = new TString()) == NULL) return FALSE;
	if ((pOption->pNewURI = new TomboURI()) == NULL) return FALSE;

	MemoNote *pNewNote = MemoNote::CopyMemo(pNote, pMemoPath, pOption->pNewHeadLine);
	if (pNewNote == NULL) return FALSE;
	AutoPointer<MemoNote> ap2(pNewNote);

	pNewNote->GetURI(pOption->pNewURI);
	return TRUE;
}

/////////////////////////////////////////
// Rename headline
/////////////////////////////////////////

BOOL LocalFileRepository::ChangeHeadLine(const TomboURI *pURI, LPCTSTR pReqNewHeadLine, URIOption *pOption)
{
	MemoNote *pNote = MemoNote::MemoNoteFactory(pURI);
	if (pNote == NULL) return FALSE;

	pOption->pNewNote = pNote;

	if (!pNote->Rename(pReqNewHeadLine)) {
		switch (GetLastError()) {
		case ERROR_NO_DATA:
			pOption->iLevel = MB_ICONWARNING;
			pOption->nErrorCode = MSG_ID_NO_FILENAME;
			break;
		case ERROR_ALREADY_EXISTS:
			pOption->iLevel = MB_ICONWARNING;
			pOption->nErrorCode = MSG_ID_SAME_FILE;
			break;
		default:
			pOption->iLevel = MB_ICONERROR;
			pOption->nErrorCode = -1;
		}
		return FALSE;
	}

	return TRUE;
}