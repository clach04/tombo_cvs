# Microsoft Developer Studio Project File - Name="Tombo" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Tombo - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "Tombo.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "Tombo.mak" CFG="Tombo - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "Tombo - Win32 Release" ("Win32 (x86) Application" 用)
!MESSAGE "Tombo - Win32 Debug" ("Win32 (x86) Application" 用)
!MESSAGE "Tombo - Win32 ReleaseE" ("Win32 (x86) Application" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Tombo/Win32/Tombo", OKAAAAAA"
# PROP Scc_LocalPath "Desktop"
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Tombo - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../../Src/regex" /I "./res" /I "." /D "NDEBUG" /D "TOMBO" /D "PLATFORM_WIN32" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "Tombo - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../../Src/regex" /I "./res" /I "." /D "_DEBUG" /D "TOMBO" /D "PLATFORM_WIN32" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "Tombo - Win32 ReleaseE"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Tombo___Win32_ReleaseE"
# PROP BASE Intermediate_Dir "Tombo___Win32_ReleaseE"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseE"
# PROP Intermediate_Dir "ReleaseE"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "../../Src/regex" /I "./res" /I "." /D "NDEBUG" /D "TOMBO" /D "PLATFORM_WIN32" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../../Src/regex" /I "./resE" /I "." /D "NDEBUG" /D "TOMBO" /D "PLATFORM_WIN32" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "TOMBO_LANG_ENGLISH" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /machine:I386

!ENDIF 

# Begin Target

# Name "Tombo - Win32 Release"
# Name "Tombo - Win32 Debug"
# Name "Tombo - Win32 ReleaseE"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\Src\AboutDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\gnupg\blowfish.c
# End Source File
# Begin Source File

SOURCE=..\..\Src\Crypt.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\CryptManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\DetailsViewWndProc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\DirectoryScanner.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\DirList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\File.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\FileInputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\FileSelector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\GrepDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\List.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\Logger.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\main.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\MainFrame.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\gnupg\md5.c
# End Source File
# Begin Source File

SOURCE=..\..\Src\MemoDetailsView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\MemoFolder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\MemoInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\MemoManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\MemoNote.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\MemoSelectView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\NewFolderDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\PasswordDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\PasswordManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\Property.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\PropertyPage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\regex\regex.c
# End Source File
# Begin Source File

SOURCE=..\..\Src\SearchDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\SearchEngine.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\SearchTree.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\SelectViewWndProc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\SipControl.cpp
# End Source File
# Begin Source File

SOURCE=.\res\Tombo.rc

!IF  "$(CFG)" == "Tombo - Win32 Release"

!ELSEIF  "$(CFG)" == "Tombo - Win32 Debug"

!ELSEIF  "$(CFG)" == "Tombo - Win32 ReleaseE"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\resE\TomboE.rc

!IF  "$(CFG)" == "Tombo - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Tombo - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Tombo - Win32 ReleaseE"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Src\TreeViewItem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\TSParser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\TString.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\Uniconv.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\regex\util.c
# End Source File
# Begin Source File

SOURCE=..\..\Src\VarBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\VFStream.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\Src\AboutDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\Chipher.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\regex\config.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\Crypt.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\CryptManager.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\DirectoryScanner.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\DirList.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\File.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\FileInputStream.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\FileSelector.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\GrepDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\List.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\Logger.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\MainFrame.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\MemoDetailsView.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\MemoFolder.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\MemoInfo.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\MemoManager.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\MemoNote.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\MemoSelectView.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\Message.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\NewFolderDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\PasswordDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\PasswordManager.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\Property.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\PropertyPage.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\regex\regex.h
# End Source File
# Begin Source File

SOURCE=.\res\resource.h

!IF  "$(CFG)" == "Tombo - Win32 Release"

!ELSEIF  "$(CFG)" == "Tombo - Win32 Debug"

!ELSEIF  "$(CFG)" == "Tombo - Win32 ReleaseE"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\resE\resource.h

!IF  "$(CFG)" == "Tombo - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Tombo - Win32 Debug"

!ELSEIF  "$(CFG)" == "Tombo - Win32 ReleaseE"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Src\SearchDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\SearchEngine.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\SearchTree.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\SipControl.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\Tombo.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\TreeViewItem.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\TSParser.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\TString.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\Uniconv.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\VarBuffer.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\VFStream.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\Src\image\draftimg.bmp
# End Source File
# Begin Source File

SOURCE=.\draftimg.bmp
# End Source File
# Begin Source File

SOURCE=.\res\draftimg.bmp
# End Source File
# Begin Source File

SOURCE=..\..\Src\image\memoimg.bmp
# End Source File
# Begin Source File

SOURCE=..\..\Src\memoimg.bmp
# End Source File
# Begin Source File

SOURCE=.\memoimg.bmp
# End Source File
# Begin Source File

SOURCE=.\res\memoimg.bmp
# End Source File
# Begin Source File

SOURCE=..\..\Src\image\Tombo.ICO
# End Source File
# Begin Source File

SOURCE=.\res\Tombo.ICO
# End Source File
# Begin Source File

SOURCE=.\resE\Tombo.ICO
# End Source File
# Begin Source File

SOURCE=.\Tombo.ICO
# End Source File
# Begin Source File

SOURCE=..\..\Src\image\toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\toolbar.bmp
# End Source File
# End Group
# End Target
# End Project
