#ifndef MEMONOTE_H
#define MEMONOTE_H

class PasswordManager;
class TString;

#include "TreeViewItem.h"

////////////////////////////////////////
// �����𒊏ۂ���N���X
////////////////////////////////////////

class MemoNote {
protected:
	LPTSTR pPath;
public:

	///////////////////////////////////////////
	// �������֘A

	MemoNote();
	virtual ~MemoNote();

	BOOL Init(LPCTSTR p);

	///////////////////////////////////////////

	virtual MemoNote *GetNewInstance() = 0;
	virtual LPCTSTR GetExtension() = 0;
	virtual DWORD GetMemoIcon() = 0;

	//////////////////////////////////
	// �������e����֘A

	// �V�K�����̐���
	BOOL InitNewMemo(LPCTSTR pMemoPath, LPCTSTR pText, TString *pHeadLine);

	// �������e�̎擾
	virtual LPTSTR GetMemoBody(PasswordManager *pMgr);
	virtual char *GetMemoBodyA(PasswordManager *pMgr);

	BOOL XX(PasswordManager *pMgr, LPCTSTR pMemo, LPCTSTR aOrigFile, LPCTSTR aWriteFile);

	// �������e�̕ۑ�
	BOOL Save(PasswordManager *pMgr, LPCTSTR pMemo, TString *pHeadLine);
	virtual BOOL SaveData(PasswordManager *pMgr, const char *pMemo, LPCTSTR pWriteFile);


	//////////////////////////////////
	// �Í����֘A

	virtual BOOL IsEncrypted() { return FALSE; }

	// �Í���: �Í������MemoNote�C���X�^���X��Ԃ��B
	// pHeadLine�ɂ̓o�b�t�@��ݒ肵�ČĂяo���B
	// �w�b�h���C��������ɕύX���������ꍇ�ɂ́@*pIsModified��TRUE�ƂȂ�A
	// �V�����w�b�h���C�������񂪐ݒ肳���B
	virtual MemoNote *Encrypt(PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified);

	// ������: ���������MemoNote�C���X�^���X��Ԃ��B	
	// pHeadLine�ɂ̓o�b�t�@��ݒ肵�ČĂяo���B
	// �w�b�h���C��������ɕύX���������ꍇ�ɂ́@*pIsModified��TRUE�ƂȂ�A
	// �V�����w�b�h���C�������񂪐ݒ肳���B
	virtual MemoNote *Decrypt(PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified);

	//////////////////////////////////
	// �t�@�C������

	// �����̃f�[�^�����폜����
	virtual BOOL DeleteMemoData();

	// �t�@�C�����̕ύX
	BOOL Rename(LPCTSTR pNewName);

	static MemoNote *CopyMemo(MemoNote *pOrig, LPCTSTR pMemoPath, TString *pHeadLine);

	//////////////////////////////////
	// �r���[�Ƃ̊֘A���

	LPCTSTR MemoPath() { return pPath; }


	//////////////////////////////////
	// Utility�n


	// GetMemoBody()�ɂ���Ď擾�����o�b�t�@�̊J��
	// �J�����Ƀo�b�t�@�̓��e���[���N���A����B
	// �Z�L�����e�B�ォ��GetMemoBody()�Ŋm�ۂ����̈��delete�ł͂Ȃ����̊֐���
	// �J�����邱��
	static void WipeOutAndDelete(LPTSTR pMemo);
#ifdef _WIN32_WCE
	static void WipeOutAndDelete(char *pMemo);
#endif

};

////////////////////////////////////////
// �Í�������Ă��Ȃ��ʏ�̃���
////////////////////////////////////////

class PlainMemoNote : public MemoNote {
public:
	MemoNote *GetNewInstance();
	LPCTSTR GetExtension();
	DWORD GetMemoIcon();

	LPTSTR GetMemoBody(PasswordManager *pMgr);
	char *GetMemoBodyA(PasswordManager *pMgr);

	BOOL SaveData(PasswordManager *pMgr, const char *pMemo, LPCTSTR pWriteFile);

	MemoNote *Encrypt(PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified);
};

////////////////////////////////////////
// �Í������ꂽ����
////////////////////////////////////////

class CryptedMemoNote : public MemoNote {
protected:
	LPBYTE GetMemoBodySub(PasswordManager *pMgr, LPDWORD pSize);
public:
	MemoNote *GetNewInstance();
	LPCTSTR GetExtension();
	DWORD GetMemoIcon();

	LPTSTR GetMemoBody(PasswordManager *pMgr);
	char *GetMemoBodyA(PasswordManager *pMgr);

	BOOL SaveData(PasswordManager *pMgr, const char *pMemo, LPCTSTR pWriteFile);

	BOOL IsEncrypted() { return TRUE; }

	MemoNote *Decrypt(PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified);
};

#endif