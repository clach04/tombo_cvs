#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

///////////////////////////////////////////////////////////
// �e�L�X�g�������W�b�N(SJIS��)
///////////////////////////////////////////////////////////

class SearchEngineA {
	DWORD nMatchStart;
	DWORD nMatchEnd;

	BOOL bCaseSensitive;
	BOOL bFileNameOnly;
	char *pPattern;

	BOOL bSearchEncrypt;

	void *pCompiledPattern;
public:

	//////////////////////////////
	// ctor & dtor, ������
	SearchEngineA();
	~SearchEngineA();
	BOOL Init(BOOL bSearchEncrypt, BOOL bFileNameOnly);

	//////////////////////////////
	// �p�^�[������

	BOOL Prepare(LPCTSTR pPattern, BOOL bCs, const char **ppReason);

	//////////////////////////////
	// �����̎��s

	BOOL SearchForward(const char *pText, DWORD nStartPos, BOOL bShift);
	BOOL SearchBackward(const char *pText, DWORD nStartPos, BOOL bShift);

	//////////////////////////////
	// ���ʎ擾
	DWORD MatchStart() { return nMatchStart; }
	DWORD MatchEnd() { return nMatchEnd; }

	BOOL IsSearchEncryptMemo() { return bSearchEncrypt; }
	BOOL IsFileNameOnly() { return bFileNameOnly; }
};

#endif