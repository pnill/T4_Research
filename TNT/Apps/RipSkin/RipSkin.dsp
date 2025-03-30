# Microsoft Developer Studio Project File - Name="RipSkin" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=RipSkin - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "RipSkin.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "RipSkin.mak" CFG="RipSkin - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "RipSkin - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "RipSkin - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/TNT/Apps/RipSkin", KLHAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "RipSkin - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "$(TNT)\Apps\Ripgeom" /I "$(TNT)\Auxiliary\Skin" /I "$(TNT)\Auxiliary\Skin\PC" /I "$(TNT)\Auxiliary\AnimPlay" /I "$(TNT)\3rdparty\DX8\Include" /I "$(TNT)\x_files" /I "$(TNT)\Auxiliary\Tokenizer" /I "$(TNT)/Auxiliary/Mesh" /I "$(TNT)/3rdParty/Dolphin/include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "RipSkin - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /GX /Zi /Od /I "$(TNT)\Apps\Ripgeom" /I "$(TNT)\Auxiliary\Skin" /I "$(TNT)\Auxiliary\Skin\PC" /I "$(TNT)\Auxiliary\AnimPlay" /I "$(TNT)\3rdparty\DX8\Include" /I "$(TNT)\x_files" /I "$(TNT)\Auxiliary\Tokenizer" /I "$(TNT)/Auxiliary/Mesh" /I "$(TNT)/3rdParty/Dolphin/include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "RipSkin - Win32 Release"
# Name "RipSkin - Win32 Debug"
# Begin Group "Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Auxiliary\mesh\BasicMorph.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Auxiliary\mesh\BasicMorph.h
# End Source File
# Begin Source File

SOURCE=..\..\Auxiliary\mesh\Basics.h
# End Source File
# Begin Source File

SOURCE=..\..\Auxiliary\Skin\CommonSkinDefines.hpp
# End Source File
# Begin Source File

SOURCE=..\..\Auxiliary\mesh\cstring.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Auxiliary\mesh\cstring.h
# End Source File
# Begin Source File

SOURCE=..\..\Auxiliary\mesh\CTextFile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Auxiliary\mesh\CTextFile.h
# End Source File
# Begin Source File

SOURCE=..\RipGeom\GeomMerge.cpp
# End Source File
# Begin Source File

SOURCE=..\RipGeom\GeomMerge.h
# End Source File
# Begin Source File

SOURCE=..\..\Auxiliary\mesh\mesh.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Auxiliary\mesh\mesh.h
# End Source File
# Begin Source File

SOURCE=.\rip_helper.cpp
# End Source File
# Begin Source File

SOURCE=.\rip_helper.h
# End Source File
# Begin Source File

SOURCE=.\RipMorph.cpp
# End Source File
# Begin Source File

SOURCE=.\RipMorph.h
# End Source File
# Begin Source File

SOURCE=.\RipSkin.cpp
# End Source File
# Begin Source File

SOURCE=.\RipSkin.h
# End Source File
# Begin Source File

SOURCE=.\RipSkinMain.cpp
# End Source File
# Begin Source File

SOURCE=.\Skel.cpp
# End Source File
# Begin Source File

SOURCE=.\skel.h
# End Source File
# Begin Source File

SOURCE=..\..\Auxiliary\Skin\Skin.hpp
# End Source File
# Begin Source File

SOURCE=.\Strip.cpp
# End Source File
# Begin Source File

SOURCE=.\Strip.h
# End Source File
# Begin Source File

SOURCE=..\..\Auxiliary\Tokenizer\Tokenizer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Auxiliary\Tokenizer\Tokenizer.hpp
# End Source File
# End Group
# Begin Group "PS2"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PS2_RipSkin.cpp
# End Source File
# Begin Source File

SOURCE=.\PS2_RipSkin.h
# End Source File
# Begin Source File

SOURCE=..\RipGeom\PS2Help.cpp
# End Source File
# Begin Source File

SOURCE=..\RipGeom\PS2Help.h
# End Source File
# Begin Source File

SOURCE=.\strippack.cpp
# End Source File
# Begin Source File

SOURCE=.\strippack.h
# End Source File
# End Group
# Begin Group "GameCube"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\GameCube_RipSkin.cpp
# End Source File
# Begin Source File

SOURCE=.\GameCube_RipSkin.h
# End Source File
# Begin Source File

SOURCE=.\GC_TriStrip.cpp
# End Source File
# Begin Source File

SOURCE=.\GC_TriStrip.h
# End Source File
# End Group
# Begin Group "XBox"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\XBOX_RipSkin.cpp
# End Source File
# Begin Source File

SOURCE=.\XBOX_RipSkin.h
# End Source File
# End Group
# Begin Group "PC"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PC_RipSkin.cpp
# End Source File
# Begin Source File

SOURCE=.\PC_RipSkin.h
# End Source File
# End Group
# End Target
# End Project
