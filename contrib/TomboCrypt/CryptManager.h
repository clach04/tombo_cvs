#ifndef CRYPTMANAGER_H
#define CRYPTMANAGER_H

#include "Crypt.h"

class CryptManager {
	Crypt crypt;
	BYTE md5key[16];

public:
	~CryptManager();

	// �p�X���[�h���w�肷��B
	BOOL Init(const char *pKey);

	// pBuf�ɕ�����ݒ肵�ČĂԂ��Ƃ�pBuf�ɈÍ�����Ԃ��B
	BOOL Encrypt(LPBYTE pBuf, int len);

	// pBuf�ɈÍ�����ݒ肵�ČĂԂ��Ƃ�pBuf�ɕ�����Ԃ��B
	// len��8�̔{���łȂ���΂Ȃ�Ȃ��B
	BOOL Decrypt(LPBYTE pBuf, int len);

	// �e�L�X�g���󂯎��A�Í������ăt�@�C���ɕۑ�����
	BOOL EncryptAndStore(const LPBYTE pData, int nSize, LPCTSTR pFileName);

	// �Í������ꂽ�t�@�C���𕜍�������B*pSize�ɂ͖{���̃o�C�g��������B
	// �܂��A�߂�l�̃f�[�^��̍Ō�ɂ�NULL���t������Ă���B
	// �߂�l�̃o�b�t�@��new����Ă��邽�߁A����Ȃ��Ȃ����i�K�ŌĂяo������delete[]����K�v������B
	LPBYTE LoadAndDecrypt(LPDWORD pSize, LPCTSTR pFileName);

	// read byte stream from std input
	LPBYTE LoadStdIn(LPDWORD pSize);

//	BOOL EncryptFile(LPCTSTR pSrc, LPCTSTR pDst);
//	BOOL DecryptFile(LPCTSTR pSrc, LPCTSTR pDst);
};

extern "C" {
	void getMD5Sum(byte *md5sum, byte *in, int len);
};
#endif

