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
static BOOL GetHeadLineFromFilePath(LPCTSTR pFilePath, TString *pHeadLine);


static BOOL GetHeadLinePath(LPCTSTR pTopDir, LPCTSTR pMemoPath, LPCTSTR pHeadLine, LPCTSTR pExt,
							TString *pFullPath, LPTSTR *ppNotePath, TString *pNewHeadLine);


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

	TString sOrigFile;
	if (!GetPhysicalPath(pCurrentURI, &sOrigFile)) return FALSE;

	// prepare text to write
	char *pText = ConvUnicode2SJIS(pMemo);
	if (pText == NULL) return FALSE;
	SecureBufferA sText(pText);

	BOOL bResult;

	// get current headline from path
	if (!GetHeadLine(pCurrentURI, pNewHeadLine)) return FALSE;

	URIOption opt(NOTE_OPTIONMASK_SAFEFILE);
	if (!GetOption(pCurrentURI, &opt)) return FALSE;

	if (pOpt->bKeepTitle || opt.bSafeFileName) {
		if (!SaveIfHeadLineIsNotChanged(pNote, pText, sOrigFile.Get())) return FALSE;

		// URI is not changed.
		return pNewURI->Init(*pCurrentURI);

	} else {
		// Get new headline from memo text
		TString sHeadLine;
		if (!GetHeadLineFromMemoText(pMemo, &sHeadLine)) return FALSE;

		DWORD nH = ChopFileNumberLen(pNewHeadLine->Get());
		DWORD nH2 = ChopFileNumberLen(sHeadLine.Get());

		// check headline has changed.
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
		if (!GetHeadLinePath(pTopDir, sMemoDir.Get(), pHeadLine, pNote->GetExtension(),
							 &sNewFile, &pNotePath, pNewHeadLine)) return FALSE;

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

	if (_tcslen(pHeadLine->Get()) == 0) {
		if (!pHeadLine->Set(DEFAULT_HEADLINE)) return FALSE;
	}

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
	BOOL bIsLeaf = pURI->IsLeaf();

	// check if the uri is safename
	if (bIsLeaf) {
		LPCTSTR pURIstr = pURI->GetFullURI();
		DWORD n = _tcslen(pURIstr);
		if (n > 4 && _tcscmp(pURIstr + n - 4, TEXT(".chs")) == 0) {
			TString sPath;
			if (!pURI->GetFilePath(&sPath)) return FALSE;
			CryptedMemoNote cn;
			cn.Init(sPath.Get());
			LPTSTR p = cn.GetMemoBody(g_pPassManager);
			if (p == NULL) return FALSE;
			SecureBufferT sbt(p);
			if (!GetHeadLineFromMemoText(p, pHeadLine)) return FALSE;
			return TRUE;
		}
	}

	if (!pURI->GetBaseName(pHeadLine)) return FALSE;

	if (_tcslen(pHeadLine->Get()) == 0) {
		// root
		return pHeadLine->Set(TEXT("[root]"));
	}

	if (bIsLeaf) {
		LPTSTR p = pHeadLine->Get();
		DWORD n = _tcslen(p);
		if (n > 4) {
			*(p + n - 4) = TEXT('\0');
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

	if (pOption->nFlg & NOTE_OPTIONMASK_SAFEFILE) {
		LPCTSTR p = pURI->GetFullURI();
		if (_tcslen(p) > 4 && _tcscmp(p + _tcslen(p) - 4, TEXT(".chs")) == 0) {
			pOption->bSafeFileName = TRUE;
		} else {
			pOption->bSafeFileName = FALSE;
		}
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

static BOOL GetHeadLineFromFilePath(LPCTSTR pFilePath, TString *pHeadLine)
{
	LPCTSTR p = pFilePath;
	LPCTSTR q = NULL;
#ifdef _WIN32_WCE
	while (*p) {
		if (*p == TEXT('\\')) q = p;
		p++;
	}
#else
	while (*p) {
		if (*p == TEXT('\\')) q = p;
		if (IsDBCSLeadByte(*p)) {
			p++;
		}
		p++;
	}
#endif
	if (q == NULL) {
		if (!pHeadLine->Set(pFilePath)) return FALSE;
	} else {
		if (!pHeadLine->Set(q + 1)) return FALSE;
	}

	pHeadLine->ChopExtension();
	pHeadLine->ChopFileNumber();
	return TRUE;
}

static BOOL GetRandomName(LPCTSTR pBaseDir, TString *pFileName)
{
	int n = rand() % 10000;

	return TRUE;
}

/////////////////////////////////////////////
// 与えられた文字列からファイル名を生成する
/////////////////////////////////////////////

// IN:	pTopDir		: Top directory
//		pMemoPath	: メモのパス(TOPDIRからの相対パス,ファイル名は含まず)
//		pHeadLine	: ヘッドライン文字列
//		pExt		: 付与する拡張子
// OUT:	pFullPath	: メモのフルパス
//		ppNotePath	: メモのパス(TOPDIRからの相対パス,ファイル名を含み、
//					  必要なら"(n)"でディレクトリで一意となるように調整されている
//		pNewHeadLine: 一覧表示用新ヘッドライン(必要に応じて"(n)"が付与されている)

static BOOL GetHeadLinePath(LPCTSTR pTopDir, LPCTSTR pMemoPath, LPCTSTR pHeadLine, LPCTSTR pExt,
							TString *pFullPath, LPTSTR *ppNotePath, TString *pNewHeadLine)
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

/////////////////////////////////////////
// Decide new allocated filename
/////////////////////////////////////////

BOOL LocalFileRepository::NegotiateNewName(LPCTSTR pMemoPath, LPCTSTR pText, LPCTSTR pMemoDir,
							 TString *pFullPath, LPTSTR *ppNotePath, TString *pNewHeadLine)
{
	TString sHeadLine;

	if (pOpt->bSafeFileName) {
		// decide file name
		if (!pFullPath->Join(pTopDir, TEXT("\\"), pMemoDir, TEXT("0000000000000000.chs"))) return FALSE;
		LPTSTR pFileNamePart = pFullPath->Get() + _tcslen(pTopDir) + 1 + _tcslen(pMemoDir);
		int nw;
		do {
			// generate 10digit random number
			nw = rand() % 10000;
			wsprintf(pFileNamePart, TEXT("%04d"), nw);
			nw = rand() % 10000;
			wsprintf(pFileNamePart + 4, TEXT("%04d"), nw);
			nw = rand() % 100;
			wsprintf(pFileNamePart + 8, TEXT("%04d"), nw);
			nw = rand() % 100;
			wsprintf(pFileNamePart + 12, TEXT("%04d"), nw);
			_tcscpy(pFileNamePart + 16, TEXT(".chs"));
		} while(IsFileExist(pFullPath->Get())); // if same name exists, retry it

		*ppNotePath = pFullPath->Get() + _tcslen(pTopDir) + 1;
		if (!GetHeadLineFromMemoText(pText, pNewHeadLine)) return FALSE;
	} else {
		if (pOpt->bKeepTitle) {
			if (!GetHeadLineFromFilePath(pMemoPath, &sHeadLine)) return FALSE;
		} else {
			if (!GetHeadLineFromMemoText(pText, &sHeadLine)) return FALSE;
		}
	
		if (!GetHeadLinePath(pTopDir, pMemoDir, sHeadLine.Get(), TEXT(".chi"), 
								pFullPath, ppNotePath, pNewHeadLine)) {
			return FALSE;
		}
	}
	return TRUE;
}

//////////////////////////////
// Encrypt file
//
// assume pCurURI is Leaf.

TomboURI *LocalFileRepository::DoEncryptFile(MemoNote *pNote, TString *pHeadLine)
{
	TString sMemoDir;
	if (!sMemoDir.GetDirectoryPath(pNote->MemoPath())) return NULL;

	// Get plain memo data from file
	LPTSTR pText = pNote->GetMemoBody(g_pPassManager);
	if (pText == NULL) return NULL;
	SecureBufferT sbt(pText);

	TString sFullPath;
	LPTSTR pNotePath;

	// Decide new name
	if (!NegotiateNewName(pNote->MemoPath(), pText, sMemoDir.Get(), 
						&sFullPath, &pNotePath, pHeadLine)) return NULL;

	// Create new CyrptedMemoNote instance
	CryptedMemoNote *p = new CryptedMemoNote();

	if (!p->Init(pNotePath)) return NULL;
	AutoPointer<CryptedMemoNote> ap(p);

	// convert date as write
	char *pTextA = ConvUnicode2SJIS(pText);
	if (pTextA == NULL) return NULL;
	SecureBufferA sba(pTextA);

	// Save memo
	if (!p->SaveData(g_pPassManager, pTextA, sFullPath.Get())) return NULL;

	// generate new URI
	TomboURI *pURI = new TomboURI();
	if (pURI == NULL || !p->GetURI(pURI)) return NULL;

	return pURI;
}

BOOL LocalFileRepository::EncryptLeaf(const TomboURI *pPlainURI, URIOption *pOption)
{
	MemoNote *pPlain = MemoNote::MemoNoteFactory(pPlainURI);
	AutoPointer<MemoNote> ap(pPlain);

	pOption->pNewHeadLine = new TString();
	if (pOption->pNewHeadLine == NULL) return FALSE;

	pOption->pNewURI = DoEncryptFile(pPlain, pOption->pNewHeadLine);
	if (pOption->pNewURI == NULL) return FALSE;

	if (!pPlain->DeleteMemoData()) {
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
	AutoPointer<MemoNote> ap2(p);

	pOption->pNewURI = new TomboURI();
	if (pOption->pNewURI == NULL) return FALSE;
	if (!p->GetURI(pOption->pNewURI)) return FALSE;

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
	AutoPointer<MemoNote> ap(pNote);

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

	TomboURI *p = new TomboURI();
	if (p == NULL || !pNote->GetURI(p)) {
		delete p;
		return FALSE;
	}
	pOption->pNewURI = p;

	return TRUE;
}