#ifndef DIRECTORYSCANNER_H
#define DIRECTORYSCANNER_H

//////////////////////////////////////////////////
// �t���O��`
//////////////////////////////////////////////////

// �f�B���N�g���̍ċA���s��Ȃ�
#define DIRECTORY_SCAN_NOREC	1
// �t�@�C���ɂ��Ă͌Ăяo���Ȃ�
#define DIRECTORY_SCAN_NOFILE	2

//////////////////////////////////////////////////
// �f�B���N�g���K�w�𑖍�����
//////////////////////////////////////////////////
//
// �X�L��������(DirectoryScanner::Scan())��DirectoryScanner::StopScan()�ŃX�L������ł��؂邱�Ƃ��ł���B
// �������AAfterScan(), PostDirectory()�ɂ��Ă�StopScan()�ɂ��X�L������ł��؂�ꂽ�ꍇ�ł�
// ���s�����B(�������J�����������s����\�������邽��)
// �K�v�ł����DirectoryScanner::IsStopScan()�Ŕ��肵�A�������X�L�b�v�����邱��

#define SCANPATH_LEN (MAX_PATH * 2)

class DirectoryScanner {
	TCHAR aScanPath[SCANPATH_LEN]; // �T�C�Y�ɗ]�T�����邽��*2���Ă���

	BOOL bContinue;
	DWORD nScanFlag;

	void ScanDirectory();

protected:
	////////////////////////////////
	// Sub class ���������o

	// ������
	BOOL Init(LPCTSTR pTopPath, DWORD nFlg);

	// ���݃X�L�������̃p�X
	LPCTSTR CurrentPath() { return aScanPath; }

	// �X�L�������f�w��
	void StopScan() { bContinue = FALSE; }
	BOOL IsStopScan() { return !bContinue; }

	////////////////////////////////
	// Template Method

	virtual void InitialScan() = 0;	// �X�L�����J�n�O
	virtual void AfterScan() = 0; // �X�L����������
	virtual void PreDirectory(LPCTSTR p) = 0; // �f�B���N�g�������O
	virtual void PostDirectory(LPCTSTR p) = 0; // �f�B���N�g��������
	virtual void File(LPCTSTR p) = 0; // �t�@�C��

public:
	// �X�L�����J�n
	BOOL Scan();
};

#endif
