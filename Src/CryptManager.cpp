#include <windows.h>
#include <stdlib.h>
#include "CryptManager.h"
#include "File.h"


// CryptManager�ɂ��Í����t�@�C���̃t�H�[�}�b�g
// '*'�͈Í������ꂽ�f�[�^
// 
// 0-3  : BF01(4 bytes)
// 4-7  : �f�[�^��(�܂� rand + md5sum)(4 bytes)
// 8-15 :* random data(8 bytes)
//16-31 :* ������md5sum(16 bytes)
//32-   :* data


void WipeOutAndDelete(char *p, DWORD len);
#ifdef _WIN32_WCE
void WipeOutAndDelete(LPTSTR p, DWORD len);
#endif

BOOL CryptManager::Init(const char *pKey)
{
	SYSTEMTIME st;
	GetSystemTime(&st);
	FILETIME ft;
	SystemTimeToFileTime(&st, &ft);
	srand(ft.dwLowDateTime);

	getMD5Sum(md5key, (byte*)pKey, strlen(pKey));
	return TRUE;
}

//////////////////////////////////////////////////
// �f�X�g���N�^
//////////////////////////////////////////////////
// �Z�L�����e�B��A�ێ����Ă���md5key����������B

CryptManager::~CryptManager()
{
	for (DWORD i = 0; i < 16; i++) {
		md5key[i] = 0;
	}
}

//////////////////////////////////////////////////
// �f�[�^�̈Í���
//////////////////////////////////////////////////

BOOL CryptManager::Encrypt(LPBYTE pBuf, int len)
{
	if (len == 0) return FALSE;

	if (!crypt.ResetStream(md5key, 16)) return FALSE;
	BYTE buf[8];
	LPBYTE p = pBuf;
	int n = len;
	int i;
	while (n > 8) {
		for (i = 0; i < 8; i++) {
			buf[i] = p[i];
		}
		crypt.Encrypt(p, buf, 8);
		p += 8;
		n -= 8;
	}
	if (n > 0) {
		for (i = 0; i < n; i++) {
			buf[i] = p[i];
		}
		crypt.Encrypt(p, buf, n);
	}

	for (i = 0; i < 8; i++) buf[i] = 0;
	return TRUE;
}

//////////////////////////////////////////////////
// �f�[�^�̕���
//////////////////////////////////////////////////

BOOL CryptManager::Decrypt(LPBYTE pBuf, int len)
{
	if (len == 0) return FALSE;

	if (!crypt.ResetStream(md5key, 16)) return FALSE;

	BYTE buf[8];
	LPBYTE p = pBuf;
	int n = len;
	int i;
	while (n >= 8) {
		for (i = 0; i < 8; i++) {
			buf[i] = p[i];
		}
		crypt.Decrypt(p, buf);
		p += 8;
		n -= 8;
	}

	for (i = 0; i < 8; i++) buf[i] = 0;
	return TRUE;
}

//////////////////////////////////////////////////
// �f�[�^�̈Í����ƃt�@�C���ւ̕ۑ�
//////////////////////////////////////////////////

BOOL CryptManager::EncryptAndStore(const LPBYTE pData, int nSize, LPCTSTR pFileName)
{
	// �̈�m�ہE�Í���

	int len = ((nSize >> 3) + 1) * 8;
	len += 24;

	LPBYTE pBuf = new BYTE[len];
	if (pBuf == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}

	// �����̖��ߍ���
	for (int i = 0; i < 8; i++) {
		pBuf[i] = (BYTE)(rand() & 0xFF);
	}

	// ������MD5SUM���擾
	getMD5Sum(pBuf + 8, pData, nSize);

	// �����̃R�s�[
	LPBYTE p = pBuf + 24;
	const BYTE *q = pData;
	for (i = 0; i < nSize; i++) {
		*p++ = *q++;
	}

	// �Í���
	if (!Encrypt(pBuf, nSize + 24)) {
		for (i = 0; i < len; i++) pBuf[i] = 0;
		WipeOutAndDelete((char*)pBuf, len);
		return FALSE;
	}

	// �t�@�C���ւ̕ۑ�
	File outf;

	if (!outf.Open(pFileName, GENERIC_WRITE, 0, CREATE_ALWAYS)) {
		for (i = 0; i < len; i++) pBuf[i] = 0;
		WipeOutAndDelete((char*)pBuf, len);
		return FALSE;
	}
	outf.Write((LPBYTE)"BF01", 4);						// �o�[�W�����w�b�_
	outf.Write((const LPBYTE)&nSize, sizeof(nSize));	// �����f�[�^��
	outf.Write(pBuf, len);								// �f�[�^
	outf.Close();

	WipeOutAndDelete((char*)pBuf, len);
	return TRUE;
}

LPBYTE CryptManager::LoadAndDecrypt(LPDWORD pSize, LPCTSTR pFileName)
{
	File inf;
	if (!inf.Open(pFileName, GENERIC_READ, 0, OPEN_EXISTING)) {
		return NULL;
	}

	DWORD nFileSize = inf.FileSize();
	char version[5];
	DWORD n;
	DWORD nDataSize;

	// �o�[�W�����w�b�_
	n = 4;
	inf.Read((LPBYTE)version, &n);
	version[4] = '\0';
	if (strcmp(version, "BF01") != 0) {
		SetLastError(ERROR_INVALID_DATA);
		return NULL;
	}

	// �f�[�^��
	n = sizeof(nDataSize);
	inf.Read((LPBYTE)&nDataSize, &n);

	LPBYTE pBuf = new BYTE[nFileSize + 1];
	n = nFileSize - 4 - sizeof(nDataSize);
	inf.Read(pBuf, &n);
	if (!Decrypt(pBuf, n)) {
		WipeOutAndDelete((char*)pBuf, nFileSize + 1);
		return NULL;
	}

	// ��������MD5SUM�̎擾
	BYTE decriptsum[16];
	getMD5Sum(decriptsum, pBuf + 24, nDataSize);

	// �������������ł������̃`�F�b�N
	for (int i = 0; i < 16; i++) {
		if (pBuf[8 + i] != decriptsum[i]) {
			WipeOutAndDelete((char*)pBuf, nFileSize + 1);
			SetLastError(ERROR_INVALID_PASSWORD);
			return NULL;
		}
	}
	pBuf[nDataSize + 24] = '\0';
	*pSize = nDataSize;

	// �̈�Ċm��
	// �����f�[�^��MD5SUM���܂Ƃ߂ĕ��������邽�߂�1�̃o�b�t�@�Ŋm�ۂ������A
	// delete�𐳂����s����悤�ɗ̈���Ċm�ہA�R�s�[���ĕԂ�
	LPBYTE pData = new BYTE[nDataSize + 1];
	if (pData == NULL) {
		WipeOutAndDelete((char*)pBuf, nFileSize + 1);
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	memcpy(pData, pBuf + 24, nDataSize);
	pData[nDataSize] = '\0';
	WipeOutAndDelete((char*)pBuf, nFileSize + 1);
	return pData;
}

void WipeOutAndDelete(LPTSTR p, DWORD len)
{
	for (DWORD i = 0; i < len; i++) p[i] = TEXT('\0');
}


void WipeOutAndDelete(LPTSTR p)
{
	if (p == NULL) return;

	LPTSTR q = p;
	while (*q) {
		*q++ = TEXT('\0');
	}
	delete [] p;
}

#ifdef _WIN32_WCE
void WipeOutAndDelete(char *p, DWORD len)
{
	for (DWORD i = 0; i < len; i++) p[i] = TEXT('\0');
}

void WipeOutAndDelete(char *p)
{
	if (p == NULL) return;

	char *q = p;
	while (*q) {
		*q++ = TEXT('\0');
	}
	delete [] p;
}
#endif
