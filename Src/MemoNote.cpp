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
//		pHeadLine   : �w�b�h���C�������� or ���x��������
//		pMemoPath   : �����̃p�X
//		pExt	    : �g���q(".txt" or ".chi")
// OUT:
//		pFullPath	: �t���p�X
//		pNewHeadLine: �C����w�b�h���C��������(=���x��������)
//		*ppNodePath	: �����̃p�X(�t���p�X����TOMBOROOTPATH������������)

static BOOL GetHeadLinePath(LPCTSTR pMemoPath, LPCTSTR pHeadLine, LPCTSTR pExt, 
							TString *pFullPath, LPTSTR *ppNotePath, TString *pNewHeadLine);

// �w�b�h���C��������̎擾
static BOOL GetHeadLineFromMemoText(LPCTSTR pMemo, TString *pHeadLine);
static BOOL GetHeadLineFromFilePath(LPCTSTR pFilePath, TString *pHeadLine);

// �w�b�h���C�������񂩂�"(n)"��������菜����������Ԃ�
static int ChopFileNumberLen(LPTSTR pHeadLine);

/////////////////////////////////////////////
// �Z�L���A�ȃo�b�t�@
/////////////////////////////////////////////
// �̈�J������0�N���A����

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

MemoNote::MemoNote() : pPath(NULL), hItem(NULL)
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
// �������e�̎擾(MemoNote)
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
// �����{�̗p�̈�̊J��
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
// �V�K����
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
// �������e�̎擾(PlainMemoNote)
/////////////////////////////////////////////

char *PlainMemoNote::GetMemoBodyA(PasswordManager*)
{
	TString sFileName;
	if (!sFileName.AllocFullPath(pPath)) return NULL;

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
// �������e�̎擾(CryptedMemoNote)
/////////////////////////////////////////////
LPBYTE CryptedMemoNote::GetMemoBodySub(PasswordManager *pMgr, LPDWORD pSize)
{
	CryptManager cMgr;
	BOOL bRegistedPassword = TRUE;

	TString sFileName;
	if (!sFileName.AllocFullPath(pPath)) return NULL;

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

	// plain text�o�b�t�@�̃[���N���A
	for (DWORD i = 0; i < nSize; i++) {
		pPlain[i] = 0;
	}

	return pMemo;
}

/////////////////////////////////////////////
// �������e�̕ۑ�
/////////////////////////////////////////////
BOOL MemoNote::XX(PasswordManager *pMgr, LPCTSTR pMemo, LPCTSTR aOrigFile, LPCTSTR aWriteFile)
{
	// �������ރ������e�̐���
	char *pText = ConvUnicode2SJIS(pMemo);
	if (pText == NULL) return FALSE;
	SecureBufferA sText(pText);

	BOOL bResult = SaveData(pMgr, pText, aWriteFile);

	if (_tcscmp(aOrigFile, aWriteFile) != 0) {
		if (bResult) {
			MemoManager::WipeOutAndDeleteFile(aOrigFile);
		} else {
			MemoManager::WipeOutAndDeleteFile(aWriteFile);
		}
	}
	return bResult;
}

// �p�X�t�@�C��������x�[�X��(�p�X�Ɗg���q������������)���擾
// ...\..\AA.txt -> AA
static BOOL GetBaseName(TString *pBase, LPCTSTR pFull)
{
	LPCTSTR p = pFull;
	LPCTSTR pLastDot = NULL;
	LPCTSTR pLastYen = NULL;
	while (*p) {
#ifndef _WIN32_WCE
		if (iskanji(*p)) {
			p += 2;
			continue;
		}
#endif
		if (*p == TEXT('.')) pLastDot = p;
		if (*p == TEXT('\\')) pLastYen = p;
		p++;
	}
	if (pLastDot == NULL) pLastDot = p;
	if (pLastYen == NULL) pLastYen = pFull - 1;

	DWORD n = pLastDot - pLastYen - 1;
	if (!pBase->Alloc(n + 1)) return FALSE;
	_tcsncpy(pBase->Get(), pLastYen + 1, n);
	*(pBase->Get() + n) = TEXT('\0');
	return TRUE;
}

BOOL MemoNote::Save(PasswordManager *pMgr, LPCTSTR pMemo, TString *pHeadLine)
{
	TString sOrigFile;
	if (!sOrigFile.AllocFullPath(pPath)) return FALSE;

	BOOL bResult;
	TString sHeadLine;

	// ���w�b�h���C�����p�X����擾
	if (!GetBaseName(pHeadLine, pPath)) return FALSE;

	if (g_Property.KeepTitle()) {
		sHeadLine.Set(pHeadLine->Get());
	} else {
		// �V�w�b�h���C������������擾
		if (!GetHeadLineFromMemoText(pMemo, &sHeadLine)) return FALSE;
	}

	DWORD nH = ChopFileNumberLen(pHeadLine->Get());
	DWORD nH2 = ChopFileNumberLen(sHeadLine.Get());

	TString sWriteFile;
	LPTSTR pNotePath = pPath;

	if (nH == nH2 && _tcsncmp(pHeadLine->Get(), sHeadLine.Get(), nH) == 0) {

		if (!sWriteFile.Alloc(_tcslen(sOrigFile.Get()) + _tcslen(TEXT(".tmp")) + 1)) return FALSE;
		wsprintf(sWriteFile.Get(), TEXT("%s.tmp"), sOrigFile.Get());

		bResult = XX(pMgr, pMemo, sOrigFile.Get(), sWriteFile.Get());
		if (bResult) {
			MoveFile(sWriteFile.Get(), sOrigFile.Get());
		}
	} else {
		TString sMemoDir;

		if (!sMemoDir.GetDirectoryPath(pPath)) return FALSE;
		// �w�b�h���C�����ύX���ꂽ
		if (!GetHeadLinePath(sMemoDir.Get(), sHeadLine.Get(), GetExtension() , &sWriteFile, &pNotePath, pHeadLine)) {
			return FALSE;
		}

		bResult = XX(pMgr, pMemo, sOrigFile.Get(), sWriteFile.Get());

		if (g_Property.KeepCaret()) {
			MemoInfo mi;
			mi.RenameInfo(sOrigFile.Get(), sWriteFile.Get());
		}
	}

	if (bResult && pNotePath != pPath) {
		LPTSTR pNewPath = StringDup(pNotePath);
		if (pNewPath == NULL) return FALSE;
		delete [] pPath;
		pPath = pNewPath;
	}

	return bResult;
}

/////////////////////////////////////////////
// �������e�̕ۑ�
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
	if (!cMgr.Init(pPassword)) return FALSE;
	return cMgr.EncryptAndStore((LPBYTE)pText, strlen(pText), pWriteFile);
}

/////////////////////////////////////////////
// �Í���
/////////////////////////////////////////////

MemoNote *MemoNote::Encrypt(PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified)
{
	return NULL;
}

MemoNote *PlainMemoNote::Encrypt(PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified)
{
	TString sMemoDir;
	if (!sMemoDir.GetDirectoryPath(pPath)) return FALSE;

	// �����{���擾
	LPTSTR pText = GetMemoBody(pMgr);
	if (pText == NULL) return FALSE;

	// �����t�@�C�����̊m��
	TString sFullPath;
	LPTSTR pNotePath;
	TString sHeadLine;

	// �t�@�C�����̃x�[�X�l�[��������
	if (g_Property.KeepTitle()) {
		// �`�F�b�NOFF��
		if (!GetHeadLineFromFilePath(pPath, &sHeadLine)) {
			MemoNote::WipeOutAndDelete(pText);
			return FALSE;
		}
	} else {
		if (!GetHeadLineFromMemoText(pText, &sHeadLine)) {
			MemoNote::WipeOutAndDelete(pText);
			return FALSE;
		}
	}

	// �V�t�@�C����������
	if (!GetHeadLinePath(sMemoDir.Get(), sHeadLine.Get(), TEXT(".chi"), &sFullPath, &pNotePath, pHeadLine)) {
		MemoNote::WipeOutAndDelete(pText);
		return FALSE;
	}


	// �V����MemoNote�C���X�^���X�𐶐�
	CryptedMemoNote *p = new CryptedMemoNote();
	if (!p->Init(pNotePath)) {
		MemoManager::WipeOutAndDeleteFile(sFullPath.Get());
		MemoNote::WipeOutAndDelete(pText);
		return FALSE;
	}
	p->SetViewItem(hItem);

	char *pTextA = ConvUnicode2SJIS(pText);
	MemoNote::WipeOutAndDelete(pText);
	if (pTextA == NULL) return FALSE;
	// �����ۑ�
	if (!p->SaveData(pMgr, pTextA, sFullPath.Get())) {
		MemoNote::WipeOutAndDelete(pTextA);
		delete p;
		return FALSE;
	}
	MemoNote::WipeOutAndDelete(pTextA);
	return p;
}

/////////////////////////////////////////////
// ������
/////////////////////////////////////////////

MemoNote *MemoNote::Decrypt(PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified)
{
	return NULL;
}

MemoNote *CryptedMemoNote::Decrypt(PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified)
{
	// �����{���擾
	LPTSTR pText = GetMemoBody(pMgr);
	if (pText == NULL) return FALSE;
	SecureBufferT sTextT(pText);

	// �w�b�h���C���擾
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

	// �V����MemoNote�C���X�^���X�𐶐�
	PlainMemoNote *p = new PlainMemoNote();
	if (!p->Init(pNotePath)) {
		MemoManager::WipeOutAndDeleteFile(sFullPath.Get());
		return NULL;
	}
	p->SetViewItem(hItem);

	// �����ۑ�
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
// �f�[�^�̍폜
/////////////////////////////////////////////

BOOL MemoNote::DeleteMemoData()
{
	TString sFileName;
	if (!sFileName.AllocFullPath(pPath)) return FALSE;

	// �t������ێ����Ă����ꍇ�ɂ͂��̏����폜
	if (MemoPath()) {
		MemoInfo mi;
		mi.DeleteInfo(MemoPath());
	}

	return MemoManager::WipeOutAndDeleteFile(sFileName.Get());
}

/////////////////////////////////////////////
// �C���X�^���X�̐���
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
// �g���q�̎擾
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
// �A�C�R���ԍ��̎擾
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
		if (iskanji(*p)) {
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

	return TRUE;
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

// �^����ꂽ�����񂩂�t�@�C�����𐶐�����

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

////////////////////////////////////////////////////////
// �t�@�C���p�X����w�b�h���C�����擾
////////////////////////////////////////////////////////
// �{���͎Q�Ƃ��Ȃ����߁A���ۂ̃w�b�h���C��������ƕK��������v���Ȃ����Ƃɒ��ӁB

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
		if (iskanji(*p)) {
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
// �����t�@�C�����R�s�[���ăC���X�^���X�𐶐�
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
	if (!sOrigPath.AllocFullPath(pOrig->MemoPath())) {
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
// �t�@�C�����ύX
/////////////////////////////////////////////

BOOL MemoNote::Rename(LPCTSTR pNewName)
{
	TString sPath;
	if (!sPath.GetDirectoryPath(pPath)) return FALSE;

	if (_tcslen(pNewName) == 0) {
		SetLastError(ERROR_NO_DATA);
		return FALSE;
	}

	// �V����pPath�̗̈�m��
	DWORD nBaseLen = _tcslen(sPath.Get());
	LPTSTR pNewPath = new TCHAR[nBaseLen + _tcslen(pNewName) + _tcslen(GetExtension()) + 6 + 1];
	if (pNewPath == NULL) return FALSE;

	// �VpPath����
	_tcscpy(pNewPath, sPath.Get());
	DropInvalidFileChar(pNewPath + nBaseLen, pNewName);
	_tcscat(pNewPath + nBaseLen, GetExtension());

	// �t�@�C�������l�[���p�p�X����
	TString sOldFullPath;
	TString sNewFullPath;
	if (!sOldFullPath.AllocFullPath(pPath) ||
		!sNewFullPath.AllocFullPath(pNewPath)) {
		delete [] pNewPath;
		return FALSE;
	}

	// �t�@�C�������l�[�����s
	if (!MoveFile(sOldFullPath.Get(), sNewFullPath.Get())) {
		delete [] pNewPath;
		return FALSE;
	}

	// *.tdt�̃��l�[���̎��s
	MemoInfo mi;
	mi.RenameInfo(sOldFullPath.Get(), sNewFullPath.Get());

	delete [] pPath;
	pPath = pNewPath;

	return TRUE;
}