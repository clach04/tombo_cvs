#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#include "Tombo.h"
#include "PasswordManager.h"
#include "PasswordDialog.h"
#include "CryptManager.h"
#include "Property.h"
#include "resource.h"
#include "Message.h"

#define FINGERPRINT_VERSION 1
#define FINGERPRINT_NUM_ENCRYPT 64

//////////////////////////////////////////
// global vars
//////////////////////////////////////////

PasswordManager *g_pPassManager = NULL;

//////////////////////////////////////////
// �f�X�g���N�^
//////////////////////////////////////////

PasswordManager::~PasswordManager()
{
	ForgetPassword();
}

//////////////////////////////////////////
// ������
//////////////////////////////////////////

BOOL PasswordManager::Init(HWND hWnd, HINSTANCE hInst)
{
	hParent = hWnd;
	hInstance = hInst;
	UpdateAccess();
	return TRUE;
}


//////////////////////////////////////////
// �p�X���[�h�̎擾
//////////////////////////////////////////

const char *PasswordManager::Password(BOOL *pCancel, BOOL bEncrypt)
{
	if (pPassword) return pPassword;

	PasswordDialog dlg;

	BOOL bPrev = bDisableHotKey;
	bDisableHotKey = TRUE;
	DWORD nResult = dlg.Popup(hInstance, hParent, bEncrypt);
	bDisableHotKey = bPrev;
	if ( nResult == IDOK) {
		if (dlg.Password() == NULL) {
			TomboMessageBox(hParent, MSG_CANT_GET_PASS,
						TEXT("ERROR"), MB_ICONSTOP | MB_OK);
			return NULL;
		} else {
			// �^�C�}�[�̃Z�b�g

			if (!SetTimer(hParent, ID_PASSWORDTIMER, 60*1000, NULL)) {
				TomboMessageBox(hParent, MSG_TIMER_SET_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
				return NULL;
			}

			// �p�X���[�h�̕ێ�
			pPassword = new char[strlen(dlg.Password()) + 1];
			strcpy(pPassword, dlg.Password());
			return pPassword;
		}
	} else {
		*pCancel = TRUE;
		return NULL;
	}
}


//////////////////////////////////////////
// �p�X���[�h�̖�����
//////////////////////////////////////////

void PasswordManager::ForgetPassword()
{
	if (pPassword) {
		char *p = pPassword;
		while (*p) {
			*p++ = '\0';
		}
		delete [] pPassword;
		pPassword = NULL;
		KillTimer(hParent, ID_PASSWORDTIMER);
	}
}


//////////////////////////////////////////
// FingerPrint�̌v�Z
//////////////////////////////////////////
// GetFingerPrint, SetFingerPrint�̉�����


static BOOL GetFingerPrintFromSeed(LPBYTE pResult, const LPBYTE pSeed, const char *pPass)
{
	CryptManager cMgr;
	if (!cMgr.Init(pPass)) return FALSE;

	BYTE workbuf1[16], workbuf2[16];
	DWORD i, count;

	for (i = 0; i < 16; i++) workbuf1[i] = pSeed[i];

	for (count = 0; count < FINGERPRINT_NUM_ENCRYPT; count++) {
		// �Í���
		if (!cMgr.Encrypt(workbuf1, 16)) return FALSE;

		// MD5�ɂ��n�b�V����
		getMD5Sum(workbuf2, workbuf1, 16);

		for (i = 0; i < 16; i++) workbuf1[i] = workbuf2[i];
	}
	for (i = 0; i < 16; i++) pResult[i] = workbuf1[i];

	return TRUE;
}

//////////////////////////////////////////
// �t�B���K�[�v�����g�̎擾
//////////////////////////////////////////

BOOL GetFingerPrint(LPBYTE pFp, const char *pPassword)
{
	DWORD i;

	// FingerPrint��seed�擾�̂��߂̗����n�񏉊���
	SYSTEMTIME st;
	GetSystemTime(&st);
	FILETIME ft;
	SystemTimeToFileTime(&st, &ft);
	srand(ft.dwLowDateTime);

	// FingerPrint��seed�̎擾
	for (i = 0; i < 16; i++) {
		pFp[i] = (BYTE)(rand() & 0xFF);
	}

	// FingerPrint�̎擾
	pFp[0] = FINGERPRINT_VERSION;
	BOOL bResult = GetFingerPrintFromSeed(pFp + 17, pFp + 1, pPassword);

	return bResult;
}

//////////////////////////////////////////
// �t�B���K�[�v�����g�̃`�F�b�N
//////////////////////////////////////////

BOOL CheckFingerPrint(LPBYTE pFp, const char *pPassword)
{
	BYTE result[16];

	if (pFp[0] != FINGERPRINT_VERSION) {
		SetLastError(ERROR_INVALID_DATA);
		return FALSE;
	}

	if (!GetFingerPrintFromSeed(result, pFp + 1, pPassword)) return FALSE;
	for (DWORD i = 0; i < 16; i++) {
		if (result[i] != pFp[17 + i]) return FALSE;
	}
	return TRUE;
}

//////////////////////////////////////////
// �A�N�Z�X�̑��s
//////////////////////////////////////////
void PasswordManager::UpdateAccess()
{
	SYSTEMTIME st;
	GetSystemTime(&st);
	SystemTimeToFileTime(&st, &ftLastAccess);
}

void PasswordManager::ForgetPasswordIfNotAccessed()
{
	if (pPassword == NULL) return;

	SYSTEMTIME st;
	FILETIME ftNow;
	GetSystemTime(&st);
	SystemTimeToFileTime(&st, &ftNow);

	// ftNow - ftLastAccess;
	DWORD nDiffHigh, nDiffLow;

	nDiffLow = ftNow.dwLowDateTime - ftLastAccess.dwLowDateTime;
	if (ftNow.dwLowDateTime < ftLastAccess.dwLowDateTime) {
		nDiffLow += 0xFFFFFFFF;
		nDiffLow += 1;
		ftNow.dwHighDateTime--;
	}
	nDiffHigh = ftNow.dwHighDateTime - ftLastAccess.dwHighDateTime;

	if (nDiffLow > g_Property.PassTimeout() * 60 * 10000000) {
		SendMessage(hParent, WM_TIMER, 0, 0);
	}
}
