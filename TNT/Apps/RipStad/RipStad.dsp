# Microsoft Developer Studio Project File - Name="RipStad" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=RipStad - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "RipStad.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "RipStad.mak" CFG="RipStad - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "RipStad - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "RipStad - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/TnT/Apps/RipStad", VSDAAAAA"
# PROP Scc_LocalPath "."
CPP=cwcl.exe
RSC=rc.exe

!IF  "$(CFG)" == "RipStad - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /I "$(TNT)/3rdParty/Dolphin/include" /I "$(TNT)/3rdParty/PS2/sce/ee/include" /I "$(TNT)/3rdparty/PC/include" /I "$(TNT)/x_files" /I "$(TNT)/x_std" /I "$(TNT)/Auxiliary/Bitmap" /I "$(TNT)/Quagmire/Engine" /I "$(TNT)\Apps\RipStad\Mesh" /I "$(TNT)\Apps\RipStad\XBOX" /I "$(TNT)\Apps\RipStad\PS2" /I "$(TNT)\Apps\RipStad\GameCube" /I "$(TNT)\Apps\RipStad\PC" /I "$(TNT)\Apps\RipStad" /I "$(TNT)/Auxiliary/Mesh" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "TARGET_PC_WINCON" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=cwlink.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib $(TNT)\3rdParty\XBOX\Lib\xgraphics.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "RipStad - Win32 Debug"

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
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(TNT)/3rdParty/Dolphin/include" /I "$(TNT)/3rdParty/PS2/sce/ee/include" /I "$(TNT)/3rdparty/PC/include" /I "$(TNT)/x_files" /I "$(TNT)/x_std" /I "$(TNT)/Auxiliary/Bitmap" /I "$(TNT)/Quagmire/Engine" /I "$(TNT)\Apps\RipStad\Mesh" /I "$(TNT)\Apps\RipStad\XBOX" /I "$(TNT)\Apps\RipStad\PS2" /I "$(TNT)\Apps\RipStad\GameCube" /I "$(TNT)\Apps\RipStad\PC" /I "$(TNT)\Apps\RipStad" /I "$(TNT)/Auxiliary/Mesh" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "TARGET_PC_WINCON" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=cwlink.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib $(TNT)\3rdParty\XBOX\Lib\xgraphics.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "RipStad - Win32 Release"
# Name "RipStad - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "XBOX"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\XBOX\XBOXRipper.cpp
# End Source File
# Begin Source File

SOURCE=.\XBOX\XBOXRipper.h
# End Source File
# End Group
# Begin Group "PS2"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PS2\PS2Help.cpp
# End Source File
# Begin Source File

SOURCE=.\PS2\PS2Help.h
# End Source File
# Begin Source File

SOURCE=.\PS2\PS2Ripper.cpp
# End Source File
# Begin Source File

SOURCE=.\PS2\PS2Ripper.h
# End Source File
# End Group
# Begin Group "GameCube"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\GameCube\GCRipper.cpp
# End Source File
# Begin Source File

SOURCE=.\GameCube\GCRipper.h
# End Source File
# End Group
# Begin Group "PC"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PC\PCRipper.cpp
# End Source File
# Begin Source File

SOURCE=.\PC\PCRipper.h
# End Source File
# End Group
# Begin Group "Mesh"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Mesh\AsciiMesh.cpp
# End Source File
# Begin Source File

SOURCE=.\Mesh\AsciiMesh.h
# End Source File
# Begin Source File

SOURCE=.\Mesh\CTextFile.cpp
# End Source File
# Begin Source File

SOURCE=.\Mesh\CTextFile.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\GeomMerge.cpp
# End Source File
# Begin Source File

SOURCE=.\GeomMerge.h
# End Source File
# Begin Source File

SOURCE=.\Readme.txt
# End Source File
# Begin Source File

SOURCE=.\RipStad.cpp
# End Source File
# Begin Source File

SOURCE=.\RipStad.h
# End Source File
# Begin Source File

SOURCE=.\Strip.cpp
# End Source File
# Begin Source File

SOURCE=.\Strip.h
# End Source File
# End Group
# Begin Group "AUX_Mesh"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Auxiliary\Mesh\Basics.h
# End Source File
# Begin Source File

SOURCE=..\..\Auxiliary\Mesh\cstring.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Auxiliary\Mesh\cstring.h
# End Source File
# End Group
# End Target
# End Project
