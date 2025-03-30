# Microsoft Developer Studio Project File - Name="TBlockTool" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=TBlockTool - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TBlockTool.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TBlockTool.mak" CFG="TBlockTool - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TBlockTool - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "TBlockTool - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/TNT/Apps/TBlockTool", XOKAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TBlockTool - Win32 Release"

# PROP BASE Use_MFC 5
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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "$(TNT)\x_files" /I "$(TNT)\Auxiliary\Bitmap" /I "$(TNT)\Auxiliary\BMPColor" /I "$(TNT)\Auxiliary\ZLib" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 $(TNT)\3rdParty\XBOX\LIB\xgraphics.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"LIBC"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "TBlockTool - Win32 Debug"

# PROP BASE Use_MFC 5
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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /ZI /Od /I "$(TNT)\x_files" /I "$(TNT)\Auxiliary\Bitmap" /I "$(TNT)\Auxiliary\BMPColor" /I "$(TNT)\Auxiliary\ZLib" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 $(TNT)\3rdParty\XBOX\LIB\xgraphics.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBCD" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /map

!ENDIF 

# Begin Target

# Name "TBlockTool - Win32 Release"
# Name "TBlockTool - Win32 Debug"
# Begin Group "APP SYSTEM"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TBlockTool.cpp
# End Source File
# Begin Source File

SOURCE=.\TBlockTool.h
# End Source File
# End Group
# Begin Group "APP DIALOG"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SwapIndexDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SwapIndexDlg.h
# End Source File
# Begin Source File

SOURCE=.\TBlockCmdLineDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\TBlockCmdLineDlg.h
# End Source File
# Begin Source File

SOURCE=.\TBlockToolDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\TBlockToolDlg.h
# End Source File
# End Group
# Begin Group "RESOURCE"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\res\TBlockFile.ico
# End Source File
# Begin Source File

SOURCE=.\res\TBlockSmall.ico
# End Source File
# Begin Source File

SOURCE=.\res\TBlockTool.ico
# End Source File
# Begin Source File

SOURCE=.\TBlockTool.rc
# End Source File
# Begin Source File

SOURCE=.\res\TBlockTool.rc2
# End Source File
# End Group
# Begin Group "UTILITY"

# PROP Default_Filter ""
# Begin Source File

SOURCE="$(TNT)\Auxiliary\BMPColor\BMPColor.cpp"
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE="$(TNT)\Auxiliary\BMPColor\BMPColor.h"
# End Source File
# End Group
# Begin Group "TEXTURE"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\StatusOutput.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\StatusOutput.h
# End Source File
# Begin Source File

SOURCE=.\TBlockCmdLineApp.cpp
# End Source File
# Begin Source File

SOURCE=.\TBlockCmdLineApp.h
# End Source File
# Begin Source File

SOURCE=.\TBlockCmdLineTxt.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\TBlockCmdLineTxt.h
# End Source File
# Begin Source File

SOURCE=.\TBlockIMEXObj.cpp
# End Source File
# Begin Source File

SOURCE=.\TBlockIMEXObj.h
# End Source File
# Begin Source File

SOURCE=.\TBlockTexture.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\TBlockTexture.h
# End Source File
# Begin Source File

SOURCE=.\TBlockWriteBMP.cpp
# End Source File
# Begin Source File

SOURCE=.\TBlockWriteBMP.h
# End Source File
# End Group
# End Target
# End Project
