#include <windows.h>
#include <tchar.h>

#include "Message.h"
#include "UniConv.h"
#include "TString.h"
#include "TomboURI.h"
#include "Property.h"
#include "Repository.h"
#include "RepositoryImpl.h"

#include "MemoNote.h"
#include "AutoPtr.h"
#include "PasswordManager.h"
#include "MemoInfo.h"

#include "DirList.h"
#include "DirectoryScanner.h"
#include "MemoFolder.h"

/////////////////////////////////////////
// static funcs
/////////////////////////////////////////

#define DEFAULT_HEADLINE MSG_DEFAULT_HEADLINE

static int ChopFileNumberLen(LPTSTR pHeadLine);
static BOOL IsFileExist(LPCTSTR pFileName);

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
// get attribute file path from notes URI 
/////////////////////////////////////////

BOOL LocalFileRepository::GetTDTFullPath(const TomboURI *pURI, TString *pTdtName)
{
	TString sPath;
	if (!pURI->GetFilePath(&sPath)) return FALSE;
	return pTdtName->Join(pTopDir, TEXT("\\"), sPath.Get(), TEXT(".tdt"));
}

/////////////////////////////////////////
// Update note
/////////////////////////////////////////

BOOL LocalFileRepository::Update(const TomboURI *pCurrentURI, LPCTSTR pData,
								 TomboURI *pNewURI, TString *pNewHeadLine)
{
	// Save note data
	if (!Save(pCurrentURI, pData, pNewURI, pNewHeadLine)) {
		return FALSE;
	}
	
	// if tdt exists, move one
	TString sCurrentTdtPath, sNewTdtPath;
	if (!GetTDTFullPath(pCurrentURI, &sCurrentTdtPath)) return FALSE;
	if (!GetTDTFullPath(pNewURI, &sNewTdtPath)) return FALSE;

	MemoInfo mi(pTopDir);
	mi.RenameInfo(sCurrentTdtPath.Get(), sNewTdtPath.Get());
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
		if (!MemoNote::GetHeadLineFromMemoText(pMemo, &sHeadLine)) return FALSE;

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
		if (!MemoNote::GetHeadLinePath(sMemoDir.Get(), pHeadLine, pNote->GetExtension(),
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
			if (!MemoNote::GetHeadLineFromMemoText(p, pHeadLine)) return FALSE;
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
	if (pOption->nFlg & NOTE_OPTIONMASK_VALID) {
		LPCTSTR p = pURI->GetFullURI();
		DWORD len = _tcslen(p);
		if (_tcscmp(p + len - 1, TEXT("/")) == 0) {
			// folder
			pOption->bValid = TRUE;
			pOption->bFolder = TRUE;
		} else {
			// file
			p = p + len - 4;
			if (_tcsicmp(p, TEXT(".txt")) == 0 ||
				_tcsicmp(p, TEXT(".chi")) == 0 || 
				_tcsicmp(p, TEXT(".chs")) == 0) {
				pOption->bValid = TRUE;
				pOption->bFolder = FALSE;
			} else {
				pOption->bValid = FALSE;
				return TRUE;
			}
		}
	}
	if (pOption->nFlg & NOTE_OPTIONMASK_ENCRYPTED) {
		LPCTSTR p = pURI->GetFullURI();
		DWORD n = _tcslen(p);
		if (n > 4) {
			if (_tcscmp(p + n - 4, TEXT(".chi")) == 0 ||
				_tcscmp(p + n - 4, TEXT(".chs")) == 0) {
				pOption->bEncrypt = TRUE;
			} else {
				pOption->bEncrypt = FALSE;
			}
		}
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
			return EnDecryptFolder(pCurrentURI, pOption);
		}
	}
	return TRUE;
}

/////////////////////////////////////////
// Get safe file name
/////////////////////////////////////////

BOOL LocalFileRepository::GetSafeFileName(const TString *pBasePath, TString *pNewName)
{
	TString s;

	if (!s.Alloc(_tcslen(pBasePath->Get()) + 20 + 1)) return FALSE;
	_tcscpy(s.Get(), pBasePath->Get());
	_tcscat(s.Get(), TEXT("0000000000000000.chs"));

	LPTSTR pFileNamePart = s.Get() + _tcslen(pBasePath->Get());
	int nw;
	do {
		// generate 10digit random number
		nw = rand() % 10000;
		wsprintf(pFileNamePart, TEXT("%04d"), nw);
		nw = rand() % 10000;
		wsprintf(pFileNamePart + 4, TEXT("%04d"), nw);
		nw = rand() % 10000;
		wsprintf(pFileNamePart + 8, TEXT("%04d"), nw);
		nw = rand() % 10000;
		wsprintf(pFileNamePart + 12, TEXT("%04d"), nw);
		_tcscpy(pFileNamePart + 16, TEXT(".chs"));
	} while(IsFileExist(s.Get())); // if same name exists, retry it

	if (!pNewName->Set(pFileNamePart)) return FALSE;

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
		TString sBase;
		TString sNewName;
		if (!sBase.Join(pTopDir, TEXT("\\"), pMemoDir)) return FALSE;
		if (!GetSafeFileName(&sBase, &sNewName)) return FALSE;

		if (!pFullPath->Join(sBase.Get(), sNewName.Get())) return FALSE;

		*ppNotePath = pFullPath->Get() + _tcslen(pTopDir) + 1;
		if (!MemoNote::GetHeadLineFromMemoText(pText, pNewHeadLine)) return FALSE;
	} else {
		if (pOpt->bKeepTitle) {
			if (!MemoNote::GetHeadLineFromFilePath(pMemoPath, &sHeadLine)) return FALSE;
		} else {
			if (!MemoNote::GetHeadLineFromMemoText(pText, &sHeadLine)) return FALSE;
		}
	
		if (!MemoNote::GetHeadLinePath(pMemoDir, sHeadLine.Get(), TEXT(".chi"), 
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
		SetLastError(ERROR_TOMBO_W_DELETEOLD_FAILED);
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
		SetLastError(ERROR_TOMBO_W_DELETEOLD_FAILED);
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////
// Encrypt/Decrypt folder
/////////////////////////////////////////

BOOL LocalFileRepository::EnDecryptFolder(const TomboURI *pCurrentURI, URIOption *pOption)
{
	TString sPath;
	if (!GetPhysicalPath(pCurrentURI, &sPath)) return FALSE;

	DSEncrypt fc;
	if (!fc.Init(pTopDir, sPath.Get(), pCurrentURI->GetFullURI(), pOption->bEncrypt)) return FALSE;

	// ask password
	BOOL bCancel;
	const char *pPass = g_pPassManager->Password(&bCancel, pOption->bEncrypt);
	if (pPass == NULL) {
		SetLastError(ERROR_TOMBO_I_GET_PASSWORD_CANCELED);
		return FALSE;
	}

	// scan and encrypt/decrypt
	if (!fc.Scan() || fc.nNotEncrypted != 0) {
		SetLastError(ERROR_TOMBO_E_SOME_ERROR_OCCURED);
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
		TString sFullPath;
		if (!GetPhysicalPath(pURI, &sFullPath)) return FALSE;
		
		DSFileDelete fd;
		fd.Init(sFullPath.Get());
		if (!fd.Scan()) {
			return FALSE;
		}
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

static BOOL IsSubFolder(LPCTSTR pSrc, LPCTSTR pDst)
{
	DWORD n = _tcslen(pSrc);
	if (_tcsncmp(pSrc, pDst, n) == 0) return TRUE;
	return FALSE;
}

/////////////////////////////////////////
// Copy note/folder
/////////////////////////////////////////

BOOL LocalFileRepository::Copy(const TomboURI *pCopyFrom, const TomboURI *pCopyTo, URIOption *pOption)
{
	URIOption opt1(NOTE_OPTIONMASK_ENCRYPTED | NOTE_OPTIONMASK_SAFEFILE | NOTE_OPTIONMASK_VALID);
	if (!GetOption(pCopyFrom, &opt1)) return FALSE;

	URIOption opt2(NOTE_OPTIONMASK_VALID);
	if (!GetOption(pCopyTo, &opt2)) return FALSE;

	if (!opt1.bValid || !opt2.bValid || !opt2.bFolder) {
		SetLastError(ERROR_TOMBO_E_INVALIDURI);
		return FALSE;
	}

	if (opt1.bFolder) {
		TString sSrcFull, sDstFull;
		if (!GetPhysicalPath(pCopyFrom, &sSrcFull)) return FALSE;
		if (!GetPhysicalPath(pCopyTo, &sDstFull)) return FALSE;

		if (IsSubFolder(sSrcFull.Get(), sDstFull.Get())) {
			SetLastError(ERROR_TOMBO_W_OPERATION_NOT_PERMITTED);
			return FALSE;
		}
		
		// Adjust Path
		TString sHL;
		if (!GetHeadLine(pCopyFrom, &sHL)) return FALSE;
		if (!sDstFull.StrCat(sHL.Get()) || !sDstFull.StrCat(TEXT("\\"))) return FALSE;

		MemoFolder mf;
		if (!mf.Init(pTopDir, sSrcFull.Get())) return FALSE;
		return mf.Copy(sDstFull.Get());
	} else {
		if ((pOption->pNewHeadLine = new TString()) == NULL) { SetLastError(ERROR_NOT_ENOUGH_MEMORY); return FALSE; }
		if ((pOption->pNewURI = new TomboURI()) == NULL) { SetLastError(ERROR_NOT_ENOUGH_MEMORY); return FALSE; }

		TString sToPath;
		if (!GetPhysicalPath(pCopyTo, &sToPath)) return FALSE;

		if (opt1.bSafeFileName) {
			TString sOrigFull;
			if (!GetPhysicalPath(pCopyFrom, &sOrigFull)) return FALSE;

			TString sBase;
			if (!pCopyFrom->GetBaseName(&sBase)) return FALSE;
			TString sNewPath;
			if (!sNewPath.Join(sToPath.Get(), sBase.Get())) return FALSE;
			if (IsFileExist(sNewPath.Get())) {
				TString sNewBase;
				if (!GetSafeFileName(&sToPath, &sNewBase)) return FALSE;
				if (!sNewPath.Join(sToPath.Get(), sNewBase.Get())) return FALSE;
				if (!sBase.Set(sNewBase.Get())) return FALSE;
			}
			if (!CopyFile(sOrigFull.Get(), sNewPath.Get(), TRUE)) return FALSE;

			if (!GetHeadLine(pCopyFrom, pOption->pNewHeadLine)) return FALSE;

			TString sNewURI;
			if (!sNewURI.Join(pCopyTo->GetFullURI(), sBase.Get())) return FALSE;
			if (!pOption->pNewURI->Init(sNewURI.Get())) return FALSE;

			return TRUE;
		} else {
			MemoNote *pNote = MemoNote::MemoNoteFactory(pCopyFrom);
			if (pNote == NULL) return FALSE;
			AutoPointer<MemoNote> ap(pNote);

			LPCTSTR pMemoPath = sToPath.Get() + _tcslen(pTopDir) + 1;

			MemoNote *pNewNote = MemoNote::CopyMemo(pNote, pMemoPath, pOption->pNewHeadLine);
			if (pNewNote == NULL) return FALSE;
			AutoPointer<MemoNote> ap2(pNewNote);

			pNewNote->GetURI(pOption->pNewURI);
			return TRUE;
		}
	}
}

/////////////////////////////////////////
// Move(Rename)
/////////////////////////////////////////

BOOL LocalFileRepository::Move(const TomboURI *pMoveFrom, const TomboURI *pMoveTo, URIOption *pOption)
{
	URIOption opt1(NOTE_OPTIONMASK_ENCRYPTED | NOTE_OPTIONMASK_SAFEFILE | NOTE_OPTIONMASK_VALID);
	if (!GetOption(pMoveFrom, &opt1)) return FALSE;

	URIOption opt2(NOTE_OPTIONMASK_VALID);
	if (!GetOption(pMoveTo, &opt2)) return FALSE;

	if (!opt1.bValid || !opt2.bValid) {
		SetLastError(ERROR_TOMBO_E_INVALIDURI);
		return FALSE;
	}

	if (opt1.bFolder) {
		TString sFullSrc;
		if (!GetPhysicalPath(pMoveFrom, &sFullSrc)) return FALSE;
		TString sFullDst;
		if (!GetPhysicalPath(pMoveTo, &sFullDst)) return FALSE;

		if (IsSubFolder(sFullSrc.Get(), sFullDst.Get())) {
			SetLastError(ERROR_TOMBO_W_OPERATION_NOT_PERMITTED);
			return FALSE;
		}

		MemoFolder mfFolder;
		if (!mfFolder.Init(pTopDir, sFullSrc.Get())) return FALSE;
		if (!mfFolder.Move(sFullDst.Get())) return FALSE;	

	} else {
		// not merged yet
	}
	return TRUE;
}

/////////////////////////////////////////
// Rename headline
/////////////////////////////////////////

BOOL LocalFileRepository::ChangeHeadLine(const TomboURI *pURI, LPCTSTR pReqNewHeadLine, URIOption *pOption)
{
	URIOption opt(NOTE_OPTIONMASK_ENCRYPTED | NOTE_OPTIONMASK_SAFEFILE | NOTE_OPTIONMASK_VALID);
	if (!GetOption(pURI, &opt)) return FALSE;
	if (opt.bValid == FALSE) {
		SetLastError(ERROR_TOMBO_E_INVALIDURI);
		return FALSE;
	}

	if (opt.bFolder) {
		TString sFullPath;
		if (!GetPhysicalPath(pURI, &sFullPath)) return FALSE;

		MemoFolder mf;
		if (!mf.Init(pTopDir, sFullPath.Get())) return FALSE;

		return mf.Rename(pReqNewHeadLine);
	} else {
		if (opt.bEncrypt && opt.bSafeFileName) {
			SetLastError(ERROR_TOMBO_I_OPERATION_NOT_PERFORMED);
			return FALSE;
		}

		MemoNote *pNote = MemoNote::MemoNoteFactory(pURI);
		if (pNote == NULL) return FALSE;
		AutoPointer<MemoNote> ap(pNote);

		if (!pNote->Rename(pReqNewHeadLine)) return FALSE;

		TomboURI *p = new TomboURI();
		if (p == NULL || !pNote->GetURI(p)) {
			delete p;
			return FALSE;
		}
		pOption->pNewURI = p;

		return TRUE;
	}
}

/////////////////////////////////////////
// GetList
/////////////////////////////////////////

BOOL LocalFileRepository::GetList(const TomboURI *pFolder, DirList *pList, BOOL bSkipEncrypt)
{
	TString sPartPath;
	if (!pFolder->GetFilePath(&sPartPath)) return FALSE;

	TString sFullPath;
	if (_tcslen(sPartPath.Get()) > 0) {
		if (!sFullPath.Join(pTopDir, TEXT("\\"), sPartPath.Get(), TEXT("*.*"))) return FALSE;
	} else {
		if (!sFullPath.Join(pTopDir, TEXT("\\*.*"))) return FALSE;
	}

	if (!pList->Init(pFolder->GetFullURI())) return FALSE;
	if (!pList->GetList(sFullPath.Get(), FALSE)) return FALSE;

	return TRUE;
}

/////////////////////////////////////////
// 
/////////////////////////////////////////
BOOL LocalFileRepository::RequestAllocateURI(const TomboURI *pBaseURI, LPCTSTR pText, TString *pHeadLine, TomboURI *pURI, const TomboURI *pTemplateURI)
{	
	LPCTSTR pMemoPath;
	TString sMemoPath;
	if (!pBaseURI->GetFilePath(&sMemoPath)) return FALSE;
	pMemoPath = sMemoPath.Get();

	MemoNote *pNote;
	if (pTemplateURI) {
		MemoNote *pCurrent = MemoNote::MemoNoteFactory(pTemplateURI);
		if (pCurrent == NULL) return FALSE;
		AutoPointer<MemoNote> apNote(pCurrent);

		pNote = pCurrent->GetNewInstance();
	} else {
		pNote = new PlainMemoNote();
	}

	if (pNote == NULL) return FALSE;

	AutoPointer<MemoNote> ap(pNote);
	
	TString sFullPath;
	TString sHeadLine;
	LPTSTR pNotePath;

	if (!MemoNote::GetHeadLineFromMemoText(pText, &sHeadLine)) return FALSE;
	if (!MemoNote::GetHeadLinePath(pMemoPath, sHeadLine.Get(), pNote->GetExtension(), &sFullPath, &pNotePath, pHeadLine)) return FALSE;
	if (!pNote->Init(pNotePath)) return FALSE;

	if (!pNote->GetURI(pURI)) return FALSE;

	return TRUE;
}

BOOL LocalFileRepository::GetAttribute(const TomboURI *pURI, NoteAttribute *pAttribute)
{
	MemoNote *pNote = MemoNote::MemoNoteFactory(pURI);
	if (pNote == NULL) return FALSE;
	AutoPointer<MemoNote> ap(pNote);

	MemoInfo mi(pTopDir);
	DWORD nPos = 0;
	if (pNote->MemoPath()) {
		if (!mi.ReadInfo(pNote->MemoPath(), &nPos)) nPos = 0;
	}
	pAttribute->nCursorPos = nPos;

	BOOL bReadOnly;
	if (!g_Property.OpenReadOnly()) {
		if (!pNote->IsReadOnly(&bReadOnly)) {
			return FALSE;
		}
	} else {
		bReadOnly = TRUE;
	}
	pAttribute->bReadOnly = bReadOnly;

	return TRUE;
}

BOOL LocalFileRepository::SetAttribute(const TomboURI *pURI, const NoteAttribute *pAttribute)
{
	MemoNote *pNote = MemoNote::MemoNoteFactory(pURI);
	if (pNote == NULL) return FALSE;
	AutoPointer<MemoNote> ap(pNote);

	MemoInfo mi(pTopDir);

	if (pNote == NULL) return FALSE;
	mi.WriteInfo(pNote->MemoPath(), pAttribute->nCursorPos);

	return TRUE;
}

LPTSTR LocalFileRepository::GetNoteData(const TomboURI *pURI)
{
	MemoNote *pNote = MemoNote::MemoNoteFactory(pURI);
	if (pNote == NULL) return FALSE;
	AutoPointer<MemoNote> ap(pNote);

	BOOL bLoop = FALSE;
	LPTSTR p;

	do {
		bLoop = FALSE;
		p = pNote->GetMemoBody(g_pPassManager);
		if (p == NULL) {
			DWORD nError = GetLastError();
			if (nError == ERROR_INVALID_PASSWORD) {
				bLoop = TRUE;
			} else {
				return NULL;
			}
		}
	} while (bLoop);
	return p;
}