#include <windows.h>
#include "FileInputStream.h"

// �X�g���[������1�����ǂݍ��ށB
// �ǂݍ��߂Ȃ��ꍇ�ɂ�0��Ԃ��B���̏ꍇ�AGetLastError()�ɃG���[�R�[�h�B
// EOF�̏ꍇ�ɂ�GetLastError()��ERROR_HANDLE_EOF��Ԃ�

char FileInputStream::getNext()
{

	DWORD nr;

	while(1) {
		// �o�b�t�@�Ɏc���Ă���΂����Ԃ�
		if (pCur < pData) {
			return *pCur++;
		}

		// ���ł�EOF�Ȃ�EOF��Ԃ��B
		if (bEof == TRUE) {
			SetLastError(ERROR_HANDLE_EOF);
			return 0;
		}

		// �c���Ă��Ȃ���΃t�@�C������ǂݍ���
		if (!ReadFile(hHandle, buf, sizeof(buf), &nr, NULL)) {
			return 0;
		}
		if (sizeof(buf) != nr) {
			bEof = TRUE;
		}
		pCur = buf;
		pData = buf + nr;
	}
	return 0; // dummy
}

BOOL FileInputStream::GetLine(char *buf, DWORD s, BOOL bNoCR)
{
	char *p = buf;
	char *max = buf + s;
	BOOL bCR = FALSE;

	while(TRUE) {
		if (p >= max) {
			SetLastError(ERROR_MORE_DATA);
			return FALSE;
		}
		
		*p = getNext();
		if (*p == 0) {
			if (GetLastError() == ERROR_HANDLE_EOF && p != buf) {
				*p++ = '\0';
				return TRUE;
			}
			return FALSE;
		} else if (*p == '\r') {
			bCR = TRUE;
			p++;
			continue;
		} else if (*p == '\n' && bCR) {
			if (bNoCR) {
				*(p-1) = '\0';
			} else {
				p++;
			}
			break;
		}

		p++;
	}
	*p++ = '\0';
	return TRUE;

}

