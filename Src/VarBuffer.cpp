#include <windows.h>
#include "VarBuffer.h"

VarBufferImpl::~VarBufferImpl()
{
	if (pBuf) {
		LocalFree(pBuf);
	}
}

BOOL VarBufferImpl::Init(DWORD ni, DWORD delta)
{
	pBuf = (LPBYTE)LocalAlloc(LMEM_FIXED, ni);
	if (pBuf == NULL) return FALSE;

	nMax = nInitBytes = ni;
	nDeltaBytes = delta;

	nCurrentUse = 0;

	return TRUE;
}

BOOL VarBufferImpl::Add(LPBYTE pData, DWORD nBytes, LPDWORD pOffset)
{
	if (nCurrentUse + nBytes >= nMax) {
		// ReAllocate extra memory
		DWORD nNumAdd = (((nBytes - (nMax - nCurrentUse)) / nDeltaBytes)  + 1) * nDeltaBytes;
		pBuf = (LPBYTE)LocalReAlloc(pBuf, nMax + nNumAdd, LMEM_MOVEABLE);
		if (pBuf == NULL) return FALSE;

		nMax += nNumAdd;
	}

	memcpy(pBuf + nCurrentUse, pData, nBytes);
	if (pOffset) {
		*pOffset = nCurrentUse;
	}
	nCurrentUse += nBytes;
	return TRUE;
}
