# Microsoft Developer Studio Project File - Name="CppUnitTestApp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** �ҏW���Ȃ��ł������� **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=CppUnitTestApp - Win32 Debug
!MESSAGE ����͗L����Ҳ�̧�قł͂���܂���B ������ۼު�Ă�����ނ��邽�߂ɂ� NMAKE ���g�p���Ă��������B
!MESSAGE [Ҳ�̧�ق̴���߰�] ����ނ��g�p���Ď��s���Ă�������
!MESSAGE 
!MESSAGE NMAKE /f "CppUnitTestApp.mak".
!MESSAGE 
!MESSAGE NMAKE �̎��s���ɍ\�����w��ł��܂�
!MESSAGE ����� ײݏ��ϸۂ̐ݒ���`���܂��B��:
!MESSAGE 
!MESSAGE NMAKE /f "CppUnitTestApp.mak" CFG="CppUnitTestApp - Win32 Debug"
!MESSAGE 
!MESSAGE �I���\������� Ӱ��:
!MESSAGE 
!MESSAGE "CppUnitTestApp - Win32 Release" ("Win32 (x86) Application" �p)
!MESSAGE "CppUnitTestApp - Win32 Debug" ("Win32 (x86) Application" �p)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CppUnitTestApp - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "../../../include" /I "../.." /I "../Src" /I "../Src/TomboLib" /I "." /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "CPPUNIT_USE_TYPEINFO" /D "PLATFORM_WIN32" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x40c /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 ../../../Lib/cppunit.lib ../../../Lib/testrunner.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "CppUnitTestApp - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "../Src" /I "../Src/TomboLib" /I "../Src/YAEdit" /I "." /I "../cppunit/include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "CPPUNIT_USE_TYPEINFO" /D "PLATFORM_WIN32" /D "USE_CPPUNIT" /D "TOMBO" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x40c /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ../cppunit/lib/cppunitd.lib ../cppunit/lib/testrunnerd.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "CppUnitTestApp - Win32 Release"
# Name "CppUnitTestApp - Win32 Debug"
# Begin Group "GUI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CppUnitTestApp.cpp
# End Source File
# Begin Source File

SOURCE=.\CppUnitTestApp.h
# End Source File
# Begin Source File

SOURCE=.\CppUnitTestApp.rc
# End Source File
# Begin Source File

SOURCE=.\CppUnitTestAppDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CppUnitTestAppDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\CppUnitTestApp.ico
# End Source File
# Begin Source File

SOURCE=.\res\CppUnitTestApp.rc2
# End Source File
# End Group
# Begin Group "Tests"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\test\MemoInfoTest.cpp
# End Source File
# Begin Source File

SOURCE=.\test\RegexTest.cpp
# End Source File
# Begin Source File

SOURCE=.\test\SharedStringTest.cpp
# End Source File
# Begin Source File

SOURCE=.\test\TomboURITest.cpp
# End Source File
# Begin Source File

SOURCE=.\test\UniConvTest.cpp
# End Source File
# Begin Source File

SOURCE=.\test\URIScannerTest.cpp
# End Source File
# Begin Source File

SOURCE=.\test\VarBufferTest.cpp
# End Source File
# Begin Source File

SOURCE=.\test\XXXTest.cpp
# End Source File
# End Group
# Begin Group "Target"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Src\GNUPG\bithelp.h
# End Source File
# Begin Source File

SOURCE=..\Src\GNUPG\blowfish.c
# End Source File
# Begin Source File

SOURCE=..\Src\Chipher.h
# End Source File
# Begin Source File

SOURCE=..\Src\regex\config.h
# End Source File
# Begin Source File

SOURCE=..\Src\Crypt.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\Crypt.h
# End Source File
# Begin Source File

SOURCE=..\Src\CryptManager.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\CryptManager.h
# End Source File
# Begin Source File

SOURCE=..\Src\TomboLib\File.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\TomboLib\File.h
# End Source File
# Begin Source File

SOURCE=..\Src\GNUPG\md5.c
# End Source File
# Begin Source File

SOURCE=..\Src\MemoInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\MemoInfo.h
# End Source File
# Begin Source File

SOURCE=..\Src\Message.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\Message.h
# End Source File
# Begin Source File

SOURCE=..\Src\regex\regex.c
# End Source File
# Begin Source File

SOURCE=..\Src\regex\regex.h
# End Source File
# Begin Source File

SOURCE=..\Src\TomboURI.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\TomboURI.h
# End Source File
# Begin Source File

SOURCE=..\Src\TomboLib\TString.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\TomboLib\TString.h
# End Source File
# Begin Source File

SOURCE=..\Src\TomboLib\Uniconv.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\TomboLib\Uniconv.h
# End Source File
# Begin Source File

SOURCE=..\Src\URIScanner.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\URIScanner.h
# End Source File
# Begin Source File

SOURCE=..\Src\regex\util.c
# End Source File
# Begin Source File

SOURCE=..\Src\TomboLib\VarBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\TomboLib\VarBuffer.h
# End Source File
# End Group
# End Target
# End Project
