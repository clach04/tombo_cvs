#ifndef PASSWORDMANAGER_H
#define PASSWORDMANAGER_H

// �^�C���A�E�g�̃`�F�b�N
//
// �d��ON �t�H�A�O���E���h�̏�Ԃŕ��u
// �d��OFF�ɂ��ĕ��u
// �d��ON ���̃A�v�����N�����ĕ��u


//////////////////////////////////////////////
// �p�X���[�h�}�l�[�W��
//////////////////////////////////////////////
// �p�X���[�h���Ǘ�����B
// �A�v���N����p�X���[�h�����͂̏ꍇ�A�_�C�A���O��\�����ăp�X���[�h�̓��͂𑣂��B

class PasswordManager {
	char *pPassword;

	HWND hParent;
	HINSTANCE hInstance;
	FILETIME ftLastAccess;

public:
	PasswordManager() : pPassword(NULL), hParent(NULL), hInstance(NULL) {}
	~PasswordManager();

	BOOL Init(HWND hParent, HINSTANCE hInstance);

	// �߂�l��NULL��bCancel��TRUE�̏ꍇ�A���[�U���L�����Z������
	// �Í����̏ꍇ�ɂ͍ē��͂��K�v�ƂȂ�BbEncrypt = TRUE�Ƃ��邱�ƁB
	const char *Password(BOOL *bCancel, BOOL bEncrypt);

	// �p�X���[�h���L�����Ă��邩
	BOOL IsRememberPassword() { return pPassword != NULL; }

	// �p�X���[�h�̏���
	void ForgetPassword();

	// �^�C�}�ɂ��p�X���[�h�����p�֐�
	void UpdateAccess();
	void ForgetPasswordIfNotAccessed();
};

//////////////////////////////////////////////
// �p�X���[�hFingerPrint�̎擾�E�m�F
//////////////////////////////////////////////
// pFp��32�o�C�g�̃o�b�t�@

// FingerPrint��1�o�C�g�̎��ʎq,16�o�C�g��seed��16�o�C�g�̌��ʁA�v33�o�C�g����Ȃ�B
// pFP[ 0]          : ���ʎq(�o�[�W�������)
// pFp[ 1]�`pFp[16] : seed
// pFp[17]�`pFp[32] : ����

BOOL GetFingerPrint(LPBYTE pFp, const char *pPassword);
BOOL CheckFingerPrint(LPBYTE pFp, const char *pPassword);

//////////////////////////////////////////////
// global definitions
//////////////////////////////////////////////

extern PasswordManager *g_pPassManager;

#endif
