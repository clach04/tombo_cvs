#ifndef LOGGER_H
#define LOGGER_H

#include "File.h"

class Logger {
	BOOL bLogActive;
	File fLogFile;
public:
	Logger();
	BOOL Init(LPCTSTR pFile);

	BOOL WriteLog(LPCTSTR pStr);
	void Close();
};


#endif