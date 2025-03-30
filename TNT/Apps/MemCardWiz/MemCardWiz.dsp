# Microsoft Developer Studio Project File - Name="MemCardWiz" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=MemCardWiz - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MemCardWiz.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MemCardWiz.mak" CFG="MemCardWiz - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MemCardWiz - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "MemCardWiz - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/TNT/Apps/MemCardWiz", XQKAAAAA"
# PROP Scc_LocalPath "."
CPP=cwcl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MemCardWiz - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "$(TNT)\x_files" /I "$(TNT)\Auxiliary\Bitmap" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=cwlink.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 xgraphics.lib /nologo /subsystem:windows /machine:I386 /libpath:"$(TNT)\3rdParty\XBOX\Lib"

!ELSEIF  "$(CFG)" == "MemCardWiz - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(TNT)\x_files" /I "$(TNT)\Auxiliary\Bitmap" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=cwlink.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 xgraphics.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"$(TNT)\3rdParty\XBOX\Lib"

!ENDIF 

# Begin Target

# Name "MemCardWiz - Win32 Release"
# Name "MemCardWiz - Win32 Debug"
# Begin Group "APP SYSTEM"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MemCardWiz.cpp
# End Source File
# Begin Source File

SOURCE=.\MemCardWiz.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "APP DIALOG"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\GCIconPropDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\GCIconPropDlg.h
# End Source File
# Begin Source File

SOURCE=.\MemCardWizDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MemCardWizDlg.h
# End Source File
# Begin Source File

SOURCE=.\PS2IconPropDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PS2IconPropDlg.h
# End Source File
# Begin Source File

SOURCE=.\TextEntryDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\TextEntryDlg.h
# End Source File
# End Group
# Begin Group "OTHER"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MemCardProject.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\MemCardProject.h
# End Source File
# End Group
# Begin Group "RESOURCE"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\res\MemCardWiz.ico
# End Source File
# Begin Source File

SOURCE=.\MemCardWiz.rc
# End Source File
# Begin Source File

SOURCE=.\res\MemCardWiz.rc2
# End Source File
# Begin Source File

SOURCE=.\res\MemCardWizSmall.ico
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# End Group
# End Target
# End Project
