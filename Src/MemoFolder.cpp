#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

#include "Tombo.h"
#include "Property.h"
#include "UniConv.h"
#include "TString.h"
#include "MemoFolder.h"
#include "DirectoryScanner.h"
#include "PasswordManager.h"
#include "MemoNote.h"
#include "Message.h"

///////////////////////////////////////////////
// ctor & dtor
///////////////////////////////////////////////

MemoFolder::MemoFolder() : pFullPath(NULL)
{
}

MemoFolder::~MemoFolder()
{
	if (pFullPath) delete [] pFullPath;
}

BOOL MemoFolder::Init(LPCTSTR p)
{
	if ((pFullPath = StringDup(p)) == NULL) return FALSE;
	return TRUE;
}

///////////////////////////////////////////////
// ディレクトリの移動
///////////////////////////////////////////////

BOOL MemoFolder::Move(LPCTSTR pDst)
{
	// 末尾のディレクトリを取得
	TCHAR aDst[MAX_PATH];
	_tcscpy(aDst, pFullPath);
	ChopFileSeparator(aDst);
	TString sDir;
	sDir.GetDirectoryPath(aDst);
	LPTSTR p = aDst + _tcslen(sDir.Get());

	TCHAR aSrc[MAX_PATH];
	_tcscpy(aSrc, pFullPath);
	ChopFileSeparator(aSrc);

	// 移動先が移動元のフォルダ名と同じになるようにパラメータを調整
	TString sDst;
	if (!sDst.Join(pDst, p)) return FALSE;

	// 移動処理
	if (!MoveFile(aSrc, sDst.Get())) return FALSE;

	return TRUE;
}

///////////////////////////////////////////////
// ディレクトリ削除
///////////////////////////////////////////////

class DSFileDelete: public DirectoryScanner {
public:
	TCHAR aFailPath[MAX_PATH * 2];
	TCHAR aErrorMsg[1024];


	DSFileDelete();

	BOOL Init(LPCTSTR pPath); 
	void InitialScan();
	void AfterScan();
	void PreDirectory(LPCTSTR);
	void PostDirectory(LPCTSTR);
	void File(LPCTSTR);
};

DSFileDelete::DSFileDelete(){ /* NOP */}

BOOL DSFileDelete::Init(LPCTSTR pPath)
{
	return DirectoryScanner::Init(pPath, 0);
}

void DSFileDelete::InitialScan() {/* NOP */}
void DSFileDelete::PreDirectory(LPCTSTR) {/* NOP */}

void DSFileDelete::PostDirectory(LPCTSTR)
{
	if (!IsStopScan()) {
		TCHAR buf[MAX_PATH*2];
		_tcscpy(buf, CurrentPath());
		ChopFileSeparator(buf);
		if (!RemoveDirectory(buf)) {
			_tcscpy(aFailPath, buf);
			_tcscpy(aErrorMsg, MSG_RMDIR_FAILED);
			StopScan();
		}
	}
}

void DSFileDelete::AfterScan() 
{
	PostDirectory(NULL);
}

void DSFileDelete::File(LPCTSTR p)
{
	LPCTSTR pExt;
	DWORD nLen = _tcslen(CurrentPath());

	if (nLen >= 4) {
		pExt = CurrentPath() + nLen - 4;
	} else {
		pExt = NULL;
	}
	// TOMBO管轄以外のファイルがあった場合に誤って消去しないようにチェック
	if (_tcsicmp(pExt, TEXT(".txt")) != 0 &&
		_tcsicmp(pExt, TEXT(".chi")) != 0 &&
		_tcsicmp(pExt, TEXT(".tdt")) != 0) {

		_tcscpy(aFailPath, CurrentPath());
		_tcscpy(aErrorMsg, MSG_OTHER_FILE_EXISTS);
		StopScan();
		return;
	}

	// 削除処理
	if (!DeleteFile(CurrentPath())) {
		_tcscpy(aFailPath, CurrentPath());
		_tcscpy(aErrorMsg, MSG_RMFILE_FAILED);
		StopScan();
	}
}

BOOL MemoFolder::Delete()
{
	DSFileDelete fd;
	fd.Init(pFullPath);
	BOOL bResult = fd.Scan();
	if (!bResult) {
		sErrorReason.Join(fd.aErrorMsg, TEXT(" : "), fd.aFailPath);
	}
	return bResult;
}

///////////////////////////////////////////////
// ディレクトリコピー
///////////////////////////////////////////////

class DSFileCopy: public DirectoryScanner {
	TCHAR aDstPath[MAX_PATH * 2];
public:
	TCHAR aFailPath[MAX_PATH * 2];
	TCHAR aErrorMsg[1024];

	BOOL Init(LPCTSTR pSrc, LPCTSTR pDst);
	void InitialScan();
	void AfterScan();
	void PreDirectory(LPCTSTR);
	void PostDirectory(LPCTSTR);
	void File(LPCTSTR);
};

BOOL DSFileCopy::Init(LPCTSTR pSrc, LPCTSTR pDst)
{
	_tcscpy(aDstPath, pDst);
	return DirectoryScanner::Init(pSrc, 0);
}

void DSFileCopy::InitialScan()
{
	TCHAR buf[MAX_PATH * 2];
	_tcscpy(buf, aDstPath);
	ChopFileSeparator(buf);
	if (!CreateDirectory(buf, NULL)) {
		_tcscpy(aFailPath, buf);
		_tcscpy(aErrorMsg, MSG_MKDIR_FAILED);
		StopScan();
	}
}

void DSFileCopy::AfterScan() {/* NOP */}

void DSFileCopy::File(LPCTSTR p)
{
	_tcscat(aDstPath, p);

	if (!CopyFile(CurrentPath(), aDstPath, FALSE)) {
		_tcscpy(aFailPath, aDstPath);
		_tcscpy(aErrorMsg, MSG_FILECOPY_FAILED);
		StopScan();
	}

	*(aDstPath + (_tcslen(aDstPath) - _tcslen(p))) = TEXT('\0');
}

void DSFileCopy::PreDirectory(LPCTSTR p)
{
	_tcscat(aDstPath, p);
	if (!CreateDirectory(aDstPath, NULL)) {
		_tcscpy(aFailPath, aDstPath);
		_tcscpy(aErrorMsg, MSG_MKDIR_FAILED);
		StopScan();
	}
	_tcscat(aDstPath, TEXT("\\"));
}

void DSFileCopy::PostDirectory(LPCTSTR p)
{
	*(aDstPath + (_tcslen(aDstPath) - _tcslen(p) - 1)) = TEXT('\0');
}

BOOL MemoFolder::Copy(LPCTSTR pDst)
{
	DSFileCopy fc;
	fc.Init(pFullPath, pDst);

	if (!fc.Scan()) {
		sErrorReason.Join(fc.aErrorMsg, TEXT(" : "), fc.aFailPath);
		return FALSE;
	} else {
		return TRUE;
	}
}

///////////////////////////////////////////////
// ディレクトリ名の変更
///////////////////////////////////////////////

BOOL MemoFolder::Rename(LPCTSTR pNewName)
{
	if (_tcslen(pNewName) == 0) {
		SetLastError(ERROR_NO_DATA);
		return FALSE;
	}

	TString sCurrent;
	if (!sCurrent.Set(pFullPath)) return FALSE;
	sCurrent.ChopFileSeparator();
	
	TString sNew;
	sNew.GetDirectoryPath(sCurrent.Get());
	sNew.StrCat(pNewName);

	return MoveFile(sCurrent.Get(), sNew.Get());
}

///////////////////////////////////////////////
// Encrypt/Decrypt
///////////////////////////////////////////////

class DSEncrypt: public DirectoryScanner {
	DWORD nBaseLen;
	PasswordManager *pPassMgr;
	BOOL bEncrypt;
public:
	DWORD nNotEncrypted;
	LPCTSTR pErrorReason;

	// If bEncrypt is FALSE, decrypt files.
	BOOL Init(LPCTSTR pPath, PasswordManager *pMgr, BOOL bEncrypt);

	void InitialScan() {}
	void AfterScan() {}
	void PreDirectory(LPCTSTR) {}
	void PostDirectory(LPCTSTR) {}
	void File(LPCTSTR);
};

BOOL DSEncrypt::Init(LPCTSTR pPath, PasswordManager *pMgr, BOOL bEnc) {
	nBaseLen = _tcslen(g_Property.TopDir());
	nNotEncrypted = 0;
	pPassMgr = pMgr;
	bEncrypt = bEnc;
	return DirectoryScanner::Init(pPath, 0);
}

void DSEncrypt::File(LPCTSTR pFile)
{
	LPCTSTR pPath = CurrentPath() + nBaseLen;
	MemoNote *pNote;
	if (!MemoNote::MemoNoteFactory(TEXT(""), pPath, &pNote)) {
		nNotEncrypted++;
		return;
	}
	if (pNote == NULL) return;
	TString s;
	BOOL b;

	// skip already encrypted/decrypted
	if (bEncrypt && pNote->IsEncrypted() ||
		!bEncrypt && !pNote->IsEncrypted()) return;

	MemoNote *pNewNote;
	if (bEncrypt) {
		pNewNote = pNote->Encrypt(pPassMgr, &s, &b);
	} else {
		pNewNote = pNote->Decrypt(pPassMgr, &s, &b);
	}

	if (pNewNote) {
		if (!pNote->DeleteMemoData()) {
			// delete failed. plain memo exists.
			if (bEncrypt) {
				pErrorReason = MSG_PLAIN_TEXT_DEL_FAILED;
			} else {
				pErrorReason = MSG_CRYPT_FILE_DEL_FAILED;
			}
			nNotEncrypted++;
		}
	} else {
		// encrypt failed.
		nNotEncrypted++;
		if (bEncrypt) {
			pErrorReason = MSG_ENCRYPT_FAILED;
		} else {
			pErrorReason = MSG_DECRYPT_FAILED;
		}
	}

	delete pNote;
	delete pNewNote;
}

BOOL MemoFolder::EnDeCrypt(PasswordManager *pMgr, BOOL bEncrypt)
{
	DSEncrypt fc;
	fc.Init(pFullPath, pMgr, bEncrypt);

	// for cancel, get password before encryption.
	BOOL bCancel;
	const char *pPass = pMgr->Password(&bCancel, bEncrypt);
	if (!pPass) {
		sErrorReason.Set(MSG_GET_PASS_FAILED);
		return FALSE;
	}

	// Scan directory and encrypt files
	if (!fc.Scan() || fc.nNotEncrypted != 0) {
		sErrorReason.Set(fc.pErrorReason);
		return FALSE;
	}

	return TRUE;
}

