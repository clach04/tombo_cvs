#include <windows.h>
#include <tchar.h>
#include "Tombo.h"
#include "Logger.h"
#include "UniConv.h"

Logger::Logger() : bLogActive(FALSE)
{
}

BOOL Logger::Init(LPCTSTR pFile)
{
	BOOL bResult = fLogFile.Open(pFile, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, CREATE_ALWAYS);
	if (bResult) bLogActive = TRUE;
	return bResult;
}

BOOL Logger::WriteLog(LPCTSTR pStr)
{
	if (!bLogActive) return TRUE;
	char *p = ConvUnicode2SJIS(pStr);
	fLogFile.Write((LPBYTE)p, strlen(p));
	delete [] p;

	return TRUE;
}

void Logger::Close()
{
	if (!bLogActive) return;
	fLogFile.Close();
}