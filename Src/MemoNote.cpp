#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include "Tombo.h"
#include "MemoNote.h"
#include "File.h"
#include "CryptManager.h"
#include "UniConv.h"
#include "PasswordManager.h"
#include "MemoManager.h"
#include "Property.h"
#include "TString.h"
#include "MemoInfo.h"
#include "Message.h"


#define DEFAULT_HEADLINE MSG_DEFAULT_HEADLINE

/////////////////////////////////////////////
//
/////////////////////////////////////////////

// IN:
//		pHeadLine   : ヘッドライン文字列 or ラベル文字列
//		pMemoPath   : メモのパス
//		pExt	    : 拡張子(".txt" or ".chi")
// OUT:
//		pFullPath	: フルパス
//		pNewHeadLine: 修正後ヘッドライン文字列(=ラベル文字列)
//		*ppNodePath	: メモのパス(フルパスからTOMBOROOTPATHを除いたもの)

static BOOL GetHeadLinePath(LPCTSTR pMemoPath, LPCTSTR pHeadLine, LPCTSTR pExt, 
							TString *pFullPath, LPTSTR *ppNotePath, TString *pNewHeadLine);

// ヘッドライン文字列の取得
static BOOL GetHeadLineFromMemoText(LPCTSTR pMemo, TString *pHeadLine);
static BOOL GetHeadLineFromFilePath(LPCTSTR pFilePath, TString *pHeadLine);

// ヘッドライン文字列から"(n)"部分を取り除いた長さを返す
static int ChopFileNumberLen(LPTSTR pHeadLine);

/////////////////////////////////////////////
// セキュアなバッファ
/////////////////////////////////////////////
// 領域開放時に0クリアする

class SecureBufferT {
	LPTSTR pBuf;
	DWORD nBufLen;
public:
	SecureBufferT(LPTSTR p) { pBuf = p; nBufLen = _tcslen(p); }
	~SecureBufferT();

	LPTSTR Get() { return pBuf; }
};

SecureBufferT::~SecureBufferT()
{
	if (pBuf) {
		LPTSTR p = pBuf;
		for (DWORD i = 0; i < nBufLen; i++) *p++ = TEXT('\0');
		delete [] pBuf;
	}
}

class SecureBufferA {
	char *pBuf;
	DWORD nBufLen;
public:
	SecureBufferA(char *p) { pBuf = p; nBufLen = strlen(p); }
	~SecureBufferA();

	char *Get() { return pBuf; }
};

SecureBufferA::~SecureBufferA()
{
	if (pBuf) {
		char *p = pBuf;
		for (DWORD i = 0; i < nBufLen; i++) *p++ = '\0';
		delete [] pBuf;
	}
}

/////////////////////////////////////////////
//
/////////////////////////////////////////////

MemoNote::MemoNote() : pPath(NULL)
{
}

MemoNote::~MemoNote()
{
	if (pPath) {
		delete [] pPath;
	}
	
}

BOOL MemoNote::Init(LPCTSTR p)
{
	if (pPath) delete [] pPath;
	pPath = new TCHAR[_tcslen(p) + 1];
	if (pPath == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}
	_tcscpy(pPath, p);

	return TRUE;
}

/////////////////////////////////////////////
// Copy Instance
/////////////////////////////////////////////
// if subclass of MemoNote has class-oriented member variables,
// create this method for each classes.

MemoNote *MemoNote::Clone()
{
	MemoNote *p = GetNewInstance();
	if (p == NULL || !p->Init(pPath)) return NULL;
	return p;
}

BOOL MemoNote::Equal(MemoNote *pTarget)
{
	if (pTarget == NULL) return FALSE;
	return (_tcsicmp(pPath, pTarget->MemoPath()) == 0);
}

/////////////////////////////////////////////
// メモ内容の取得(MemoNote)
/////////////////////////////////////////////

LPTSTR MemoNote::GetMemoBody(PasswordManager *pMgr) 
{
	SetLastError(ERROR_INVALID_FUNCTION);
	return NULL;
}

char *MemoNote::GetMemoBodyA(PasswordManager *pMgr)
{
	SetLastError(ERROR_INVALID_FUNCTION);
	return NULL;
}

/////////////////////////////////////////////
// メモ本体用領域の開放
/////////////////////////////////////////////

void MemoNote::WipeOutAndDelete(LPTSTR p)
{
	if (p == NULL) return;

	LPTSTR q = p;
	while (*q) {
		*q++ = TEXT('\0');
	}
	delete [] p;
}

#ifdef _WIN32_WCE
void MemoNote::WipeOutAndDelete(char *p)
{
	if (p == NULL) return;

	char *q = p;
	while (*q) {
		*q++ = TEXT('\0');
	}
	delete [] p;
}
#endif

/////////////////////////////////////////////
// 新規メモ
/////////////////////////////////////////////
BOOL MemoNote::InitNewMemo(LPCTSTR pMemoPath, LPCTSTR pText, TString *pHeadLine)
{
	TString sFullPath;
	TString sHeadLine;
	LPTSTR pNotePath;

	if (!GetHeadLineFromMemoText(pText, &sHeadLine)) return FALSE;
	if (!GetHeadLinePath(pMemoPath, sHeadLine.Get(), GetExtension(), &sFullPath, &pNotePath, pHeadLine)) return FALSE;

	return Init(pNotePath);
}

/////////////////////////////////////////////
// メモ内容の取得(PlainMemoNote)
/////////////////////////////////////////////

char *PlainMemoNote::GetMemoBodyA(PasswordManager*)
{
	TString sFileName;
//	if (!sFileName.AllocFullPath(pPath)) return NULL;
	if (!sFileName.Join(g_Property.TopDir(), TEXT("\\"), pPath)) return NULL;

	File inf;
	if (!inf.Open(sFileName.Get(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING)) return NULL;

	char *pText = new char[inf.FileSize() + 1];
	if (pText == NULL) return NULL;

	DWORD nSize = inf.FileSize();
	if (!inf.Read((LPBYTE)pText, &nSize)) return NULL;
	pText[nSize] = TEXT('\0');

	return pText;
}

LPTSTR PlainMemoNote::GetMemoBody(PasswordManager *p)
{
	char *pText = GetMemoBodyA(p);
	if (!pText) return NULL;

	LPTSTR pMemo = ConvSJIS2Unicode(pText);
	delete [] pText;
	if (!pMemo) return NULL;

	return pMemo;
}

/////////////////////////////////////////////
// メモ内容の取得(CryptedMemoNote)
/////////////////////////////////////////////
LPBYTE CryptedMemoNote::GetMemoBodySub(PasswordManager *pMgr, LPDWORD pSize)
{
	CryptManager cMgr;
	BOOL bRegistedPassword = TRUE;

	TString sFileName;
//	if (!sFileName.AllocFullPath(pPath)) return NULL;
	if (!sFileName.Join(g_Property.TopDir(), TEXT("\\"), pPath)) return NULL;

	BOOL bCancel;
	const char *pPassword = pMgr->Password(&bCancel, FALSE);
	if (pPassword == NULL) return NULL;

	if (g_Property.FingerPrint()) {
		if (!CheckFingerPrint(g_Property.FingerPrint(), pPassword)) {
			if (TomboMessageBox(NULL, MSG_PASS_NOT_MATCH2, MSG_PASS_MISMATCH_TTL, MB_ICONQUESTION | MB_YESNO) == IDNO) {
				pMgr->ForgetPassword();
				return NULL;
			}
			bRegistedPassword = FALSE;
		}
	}

	if (!cMgr.Init(pPassword)) return NULL;

	LPBYTE pPlain = cMgr.LoadAndDecrypt(pSize, sFileName.Get());
	if (pPlain == NULL) {
		pMgr->ForgetPassword();
		return NULL;
	}
	if (!bRegistedPassword) pMgr->ForgetPassword();

	return pPlain;
}

char *CryptedMemoNote::GetMemoBodyA(PasswordManager *pMgr)
{
	DWORD nSize;
	return (char*)GetMemoBodySub(pMgr, &nSize);
}

LPTSTR CryptedMemoNote::GetMemoBody(PasswordManager *pMgr)
{
	DWORD nSize;
	LPBYTE pPlain = GetMemoBodySub(pMgr, &nSize);
	if (!pPlain) return NULL;

	LPTSTR pMemo = ConvSJIS2Unicode((const char*)pPlain);

	// plain textバッファのゼロクリア
	for (DWORD i = 0; i < nSize; i++) {
		pPlain[i] = 0;
	}

	return pMemo;
}

////////////////////////////////////////////////////
// Save memo data and return new headline string
////////////////////////////////////////////////////
//
// IN : pMgr, pMemo
// OUT: pHeadLine
BOOL MemoNote::Save(PasswordManager *pMgr, LPCTSTR pMemo, TString *pHeadLine)
{
	TString sOrigFile;
//	if (!sOrigFile.AllocFullPath(pPath)) return FALSE;
	if (!sOrigFile.Join(g_Property.TopDir(), TEXT("\\"), pPath)) return FALSE;

	BOOL bResult;
	TString sHeadLine;

	// get old headline from path
	if (!GetBaseName(pHeadLine, pPath)) return FALSE;

	if (g_Property.KeepTitle()) {
		// clone original headline
		sHeadLine.Set(pHeadLine->Get());
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


	DWORD nH = ChopFileNumberLen(pHeadLine->Get());
	DWORD nH2 = ChopFileNumberLen(sHeadLine.Get());

	LPTSTR pNotePath = pPath;

	// check headline has changed.
	if (nH == nH2 && _tcsncmp(pHeadLine->Get(), sHeadLine.Get(), nH) == 0) {
		// not changed.
		// Generate backup file name
		TString sBackupFile;
		if (!sBackupFile.Join(sOrigFile.Get(), TEXT(".tmp"))) return FALSE;
		// Backup(copy) original file
		//
		// Because ActiveSync can't treat Move&Write, backup operation uses not move but copy
		if (!CopyFile(sOrigFile.Get(), sBackupFile.Get(), FALSE)) {
			// if new file, copy are failed but it is OK.
			if (GetLastError() != ERROR_FILE_NOT_FOUND) return FALSE;
		}
		// Save to file
		if (!SaveData(pMgr, pText, sOrigFile.Get())) {
			// When save failed, try to rollback original file.
			DeleteFile(sOrigFile.Get());
			MoveFile(sBackupFile.Get(), sOrigFile.Get());
			return FALSE;
		}
		// remove backup file
		DeleteFile(sBackupFile.Get());
		bResult = TRUE;
	} else {
		// changed.
		TString sMemoDir;
		TString sNewFile;

		if (!sMemoDir.GetDirectoryPath(pPath)) return FALSE;
		if (!GetHeadLinePath(sMemoDir.Get(), sHeadLine.Get(), GetExtension() , &sNewFile, &pNotePath, pHeadLine)) return FALSE;

		bResult = SaveData(pMgr, pText, sNewFile.Get());
		if (bResult) {
			// delete original file
			DeleteFile(sOrigFile.Get());

			// Update note's file path information. 
			LPTSTR pNewPath = StringDup(pNotePath);
			if (pNewPath == NULL) return FALSE;
			delete [] pPath;
			pPath = pNewPath;

		} else {
			// delete writing file
			DeleteFile(sNewFile.Get());
		}

		// Additionally, rename memo info(*.tdt) file.
		if (g_Property.KeepCaret()) {
			MemoInfo mi;
			mi.RenameInfo(sOrigFile.Get(), sNewFile.Get());
		}
	}
	return bResult;
}

/////////////////////////////////////////////
// メモ内容の保存
/////////////////////////////////////////////
BOOL MemoNote::SaveData(PasswordManager *pMgr, const char *pText, LPCTSTR pWriteWile)
{
	return FALSE;
}

BOOL PlainMemoNote::SaveData(PasswordManager *pMgr, const char *pText, LPCTSTR pWriteFile)
{
	File outf;
	if (!outf.Open(pWriteFile, GENERIC_WRITE, 0, OPEN_ALWAYS)) return FALSE;
	if (!outf.Write((LPBYTE)pText, strlen(pText))) return FALSE;
	if (!outf.SetEOF()) return FALSE;
	outf.Close();
	return TRUE;
}

BOOL CryptedMemoNote::SaveData(PasswordManager *pMgr, const char *pText, LPCTSTR pWriteFile)
{
	CryptManager cMgr;
	BOOL bCancel;
	const char *pPassword = pMgr->Password(&bCancel, TRUE);
	if (pPassword == NULL) return FALSE;

	if (g_Property.FingerPrint()) {
		if (!CheckFingerPrint(g_Property.FingerPrint(), pPassword)) {
			TomboMessageBox(NULL, MSG_PASS_NOT_MATCH3, MSG_PASS_MISMATCH_TTL, MB_ICONWARNING | MB_OK);
			pMgr->ForgetPassword();
			return NULL;
		}
	}
	if (!cMgr.Init(pPassword)) {
		MessageBox(NULL, TEXT("In CryptedMemoNote::SaveData,CryptManager::Init failed"), TEXT("DEBUG"), MB_OK);
		return FALSE;
	}
	return cMgr.EncryptAndStore((LPBYTE)pText, strlen(pText), pWriteFile);
}

/////////////////////////////////////////////
// 暗号化
/////////////////////////////////////////////

MemoNote *MemoNote::Encrypt(PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified)
{
	MessageBox(NULL, TEXT("MemoNote::Encrypt called"), TEXT("DEBUG"), MB_OK);
	return NULL;
}

MemoNote *PlainMemoNote::Encrypt(PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified)
{
	TString sMemoDir;
	if (!sMemoDir.GetDirectoryPath(pPath)) {
		MessageBox(NULL, TEXT("GetDirectoryPath failed"), TEXT("DEBUG"), MB_OK);
		return FALSE;
	}

	// メモ本文取得
	LPTSTR pText = GetMemoBody(pMgr);
	if (pText == NULL) {
		MessageBox(NULL, TEXT("GetMemoBody failed"), TEXT("DEBUG"), MB_OK);
		return FALSE;
	}

	// メモファイル名の確定
	TString sFullPath;
	LPTSTR pNotePath;
	TString sHeadLine;

	// ファイル名のベースネームを決定
	if (g_Property.KeepTitle()) {
		// チェックOFF時
		if (!GetHeadLineFromFilePath(pPath, &sHeadLine)) {
			MemoNote::WipeOutAndDelete(pText);
			MessageBox(NULL, TEXT("GetHeadLineFromFilePath failed"), TEXT("DEBUG"), MB_OK);
			return FALSE;
		}
	} else {
		if (!GetHeadLineFromMemoText(pText, &sHeadLine)) {
			MemoNote::WipeOutAndDelete(pText);
			MessageBox(NULL, TEXT("GetHeadLineFromMemoText failed"), TEXT("DEBUG"), MB_OK);
			return FALSE;
		}
	}

	// 新ファイル名を決定
	if (!GetHeadLinePath(sMemoDir.Get(), sHeadLine.Get(), TEXT(".chi"), &sFullPath, &pNotePath, pHeadLine)) {
		MessageBox(NULL, TEXT("GetHeadLinePath failed"), TEXT("DEBUG"), MB_OK);
		MemoNote::WipeOutAndDelete(pText);
		return FALSE;
	}

	// 新しいMemoNoteインスタンスを生成
	CryptedMemoNote *p = new CryptedMemoNote();
	if (!p->Init(pNotePath)) {
		MemoManager::WipeOutAndDeleteFile(sFullPath.Get());
		MemoNote::WipeOutAndDelete(pText);
		MessageBox(NULL, TEXT("new CryptedMemoNote failed"), TEXT("DEBUG"), MB_OK);
		return FALSE;
	}

	char *pTextA = ConvUnicode2SJIS(pText);
	MemoNote::WipeOutAndDelete(pText);
	if (pTextA == NULL) {
		MessageBox(NULL, TEXT("ConvUnicode2SJIS failed"), TEXT("DEBUG"), MB_OK);
		return FALSE;
	}
	// メモ保存
	if (!p->SaveData(pMgr, pTextA, sFullPath.Get())) {
		MemoNote::WipeOutAndDelete(pTextA);
		delete p;
		MessageBox(NULL, TEXT("SaveData failed"), TEXT("DEBUG"), MB_OK);
		return FALSE;
	}
	MemoNote::WipeOutAndDelete(pTextA);
	return p;
}

/////////////////////////////////////////////
// 復号化
/////////////////////////////////////////////

MemoNote *MemoNote::Decrypt(PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified)
{
	return NULL;
}

MemoNote *CryptedMemoNote::Decrypt(PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified)
{
	// メモ本文取得
	LPTSTR pText = GetMemoBody(pMgr);
	if (pText == NULL) return FALSE;
	SecureBufferT sTextT(pText);

	// ヘッドライン取得
	TString sMemoDir;
	if (!sMemoDir.GetDirectoryPath(pPath)) return FALSE;

	TString sFullPath;
	LPTSTR pNotePath;
	TString sHeadLine;
	if (g_Property.KeepTitle()) {
		if (!GetHeadLineFromFilePath(pPath, &sHeadLine)) return FALSE;
	} else {
		if (!GetHeadLineFromMemoText(pText, &sHeadLine)) return FALSE;
	}

	if (!GetHeadLinePath(sMemoDir.Get(), sHeadLine.Get(), TEXT(".txt"), &sFullPath, &pNotePath, pHeadLine)) {
		return FALSE;
	}

	// 新しいMemoNoteインスタンスを生成
	PlainMemoNote *p = new PlainMemoNote();
	if (!p->Init(pNotePath)) {
		MemoManager::WipeOutAndDeleteFile(sFullPath.Get());
		return NULL;
	}

	// メモ保存
	char *pTextA = ConvUnicode2SJIS(pText);
	if (pTextA == NULL) return FALSE;
	SecureBufferA sTextA(pTextA);

	if (!p->SaveData(pMgr, pTextA, sFullPath.Get())) {
		delete p;
		return NULL;
	}
	return p;
}

/////////////////////////////////////////////
// データの削除
/////////////////////////////////////////////

BOOL MemoNote::DeleteMemoData()
{
	TString sFileName;
//	if (!sFileName.AllocFullPath(pPath)) return FALSE;
	if (!sFileName.Join(g_Property.TopDir(), TEXT("\\"), pPath)) return FALSE;

	// 付加情報を保持していた場合にはその情報も削除
	if (MemoPath()) {
		MemoInfo mi;
		mi.DeleteInfo(MemoPath());
	}

	return MemoManager::WipeOutAndDeleteFile(sFileName.Get());
}

/////////////////////////////////////////////
// インスタンスの生成
/////////////////////////////////////////////

MemoNote *PlainMemoNote::GetNewInstance()
{
	return new PlainMemoNote();
}

MemoNote *CryptedMemoNote::GetNewInstance()
{
	return new CryptedMemoNote();
}

/////////////////////////////////////////////
// 拡張子の取得
/////////////////////////////////////////////

LPCTSTR PlainMemoNote::GetExtension()
{
	return TEXT(".txt");
}

LPCTSTR CryptedMemoNote::GetExtension()
{
	return TEXT(".chi");
}

/////////////////////////////////////////////
// アイコン番号の取得
/////////////////////////////////////////////

DWORD PlainMemoNote::GetMemoIcon()
{
	return IMG_ARTICLE;
}

DWORD CryptedMemoNote::GetMemoIcon()
{
	return IMG_ARTICLE_ENCRYPTED;
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

// 与えられた文字列からファイル名を生成する

// IN:	pMemoPath	: メモのパス(TOPDIRからの相対パス,ファイル名は含まず)
//		pHeadLine	: ヘッドライン文字列
//		pExt		: 付与する拡張子
// OUT:	pFullPath	: メモのフルパス
//		ppNotePath	: メモのパス(TOPDIRからの相対パス,ファイル名を含み、
//					  必要なら"(n)"でディレクトリで一意となるように調整されている
//		pNewHeadLine: 一覧表示用新ヘッドライン(必要に応じて"(n)"が付与されている)

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

////////////////////////////////////////////////////////
// ファイルパスからヘッドラインを取得
////////////////////////////////////////////////////////
// 本文は参照しないため、実際のヘッドライン文字列と必ずしも一致しないことに注意。

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

////////////////////////////////////////////////////////
// メモファイルをコピーしてインスタンスを生成
////////////////////////////////////////////////////////

MemoNote *MemoNote::CopyMemo(MemoNote *pOrig, LPCTSTR pMemoPath, TString *pHeadLine)
{
	MemoNote *pNote;
	pNote = pOrig->GetNewInstance();
	if (pNote == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}

	TString sNewFullPath;
	LPTSTR pNotePath;
	TString sHeadLine;

	if (!GetHeadLineFromFilePath(pOrig->MemoPath(), &sHeadLine)) {
		delete pNote;
		return NULL;
	}
	if (!GetHeadLinePath(pMemoPath, sHeadLine.Get(), pNote->GetExtension(), &sNewFullPath, &pNotePath, pHeadLine)) {
		delete pNote;
		return NULL;
	}

	TString sOrigPath;
//	if (!sOrigPath.AllocFullPath(pOrig->MemoPath())) {
	if (!sOrigPath.Join(g_Property.TopDir(), TEXT("\\"), pOrig->MemoPath())) {
		delete pNote;
		return NULL;
	}

	if (!CopyFile(sOrigPath.Get(), sNewFullPath.Get(), TRUE) || !pNote->Init(pNotePath)) {
		delete pNote;
		return NULL;
	}
	return pNote;
}

/////////////////////////////////////////////
// ファイル名変更
/////////////////////////////////////////////

BOOL MemoNote::Rename(LPCTSTR pNewName)
{
	TString sPath;
	if (!sPath.GetDirectoryPath(pPath)) return FALSE;

	if (_tcslen(pNewName) == 0) {
		SetLastError(ERROR_NO_DATA);
		return FALSE;
	}

	// 新しいpPathの領域確保
	DWORD nBaseLen = _tcslen(sPath.Get());
	LPTSTR pNewPath = new TCHAR[nBaseLen + _tcslen(pNewName) + _tcslen(GetExtension()) + 6 + 1];
	if (pNewPath == NULL) return FALSE;

	// 新pPath生成
	_tcscpy(pNewPath, sPath.Get());
	DropInvalidFileChar(pNewPath + nBaseLen, pNewName);
	_tcscat(pNewPath + nBaseLen, GetExtension());

	// ファイル名リネーム用パス生成
	TString sOldFullPath;
	TString sNewFullPath;
//	if (!sOldFullPath.AllocFullPath(pPath) ||
//		!sNewFullPath.AllocFullPath(pNewPath)) {
	if (!sOldFullPath.Join(g_Property.TopDir(), TEXT("\\"), pPath) ||
		!sNewFullPath.Join(g_Property.TopDir(), TEXT("\\"), pNewPath)) {
		delete [] pNewPath;
		return FALSE;
	}

	// ファイル名リネーム実行
	if (!MoveFile(sOldFullPath.Get(), sNewFullPath.Get())) {
		delete [] pNewPath;
		return FALSE;
	}

	// *.tdtのリネームの実行
	MemoInfo mi;
	mi.RenameInfo(sOldFullPath.Get(), sNewFullPath.Get());

	delete [] pPath;
	pPath = pNewPath;

	return TRUE;
}

/////////////////////////////////////////////
// Check file is readonly
/////////////////////////////////////////////

BOOL MemoNote::IsReadOnly(BOOL *pReadOnly)
{
	if (!pReadOnly) return FALSE;

	TString sFullPath;
	if (!sFullPath.Join(g_Property.TopDir(), TEXT("\\"), pPath)) return FALSE;

	WIN32_FIND_DATA wfd;
	HANDLE h = FindFirstFile(sFullPath.Get(), &wfd);
	if (h == INVALID_HANDLE_VALUE) return FALSE;
	FindClose(h);

	*pReadOnly = (wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) == FILE_ATTRIBUTE_READONLY;
	return TRUE;
}

/////////////////////////////////////////////
// Check is this file memo?
/////////////////////////////////////////////

DWORD MemoNote::IsNote(LPCTSTR pFile)
{
	DWORD len = _tcslen(pFile);
	if (len <= 4) return NOTE_TYPE_NO;

	LPCTSTR p = pFile + len - 4;

	DWORD nType;
	if (_tcsicmp(p, TEXT(".txt")) == 0) {
		nType = NOTE_TYPE_PLAIN;
	} else if (_tcsicmp(p, TEXT(".chi")) == 0) {
		nType = NOTE_TYPE_CRYPTED;
	} else {
		nType = NOTE_TYPE_NO;
	}
	return nType;
}


/////////////////////////////////////////////
// MemoNote object factory
/////////////////////////////////////////////

BOOL MemoNote::MemoNoteFactory(LPCTSTR pPrefix, LPCTSTR pFile, MemoNote **ppNote)
{
	*ppNote = NULL;

	DWORD nType = IsNote(pFile);
	if (nType == NOTE_TYPE_NO) return TRUE;
	if (nType == NOTE_TYPE_PLAIN) {
		*ppNote = new PlainMemoNote();
	} else {
		*ppNote = new CryptedMemoNote();
	}

	if (*ppNote == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}

	TCHAR path[MAX_PATH];
	wsprintf(path, TEXT("%s%s"), pPrefix, pFile);
	if (!(*ppNote)->Init(path)) {
		delete (*ppNote);
		*ppNote = NULL;
		return FALSE;
	}
	return TRUE;
}

