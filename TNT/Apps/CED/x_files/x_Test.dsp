# Microsoft Developer Studio Project File - Name="x_Test" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=x_Test - Win32 PS2 DevKit Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "x_Test.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "x_Test.mak" CFG="x_Test - Win32 PS2 DevKit Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "x_Test - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "x_Test - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "x_Test - Win32 PC Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "x_Test - Win32 PC Release" (based on "Win32 (x86) Console Application")
!MESSAGE "x_Test - Win32 PS2 DevKit Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "x_Test - Win32 PS2 DevKit Release" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/TnT/x_files", GFGAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "x_Test - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(TnT)\x_files" /D "WIN32" /D "X_DEBUG" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(TnT)\x_files" /D "WIN32" /D "X_DEBUG" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "x_Test - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(TnT)\x_files" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "$(TnT)\x_files" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "x_Test - Win32 PC Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DPC"
# PROP BASE Intermediate_Dir "DPC"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DPC"
# PROP Intermediate_Dir "DPC"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(TnT)\x_files" /D "TARGET_PC" /D "WIN32" /D "X_DEBUG" /D "_DEBUG" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(TnT)\x_files" /D "TARGET_PC" /D "WIN32" /D "X_DEBUG" /D "_DEBUG" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "x_Test - Win32 PC Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "RPC"
# PROP BASE Intermediate_Dir "RPC"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "RPC"
# PROP Intermediate_Dir "RPC"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(TnT)\x_files" /D "TARGET_PC" /D "WIN32" /D "NDEBUG" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "$(TnT)\x_files" /D "TARGET_PC" /D "WIN32" /D "NDEBUG" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "x_Test - Win32 PS2 DevKit Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DPS2Dev"
# PROP BASE Intermediate_Dir "DPS2Dev"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DPS2Dev"
# PROP Intermediate_Dir "DPS2Dev"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(TnT)\x_files" /D "X_DEBUG" /D "TARGET_PS2_DEV" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(TnT)\x_files" /D "X_DEBUG" /D "TARGET_PS2_DEV" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:console /debug /machine:I386 /out:"DPS2Dev/x_Test.elf" /pdbtype:sept /D:TARGET_PS2_DEV
# ADD LINK32 /nologo /subsystem:console /debug /machine:I386 /out:"DPS2Dev/x_Test.elf" /pdbtype:sept /D:TARGET_PS2_DEV
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "x_Test - Win32 PS2 DevKit Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "RPS2Dev"
# PROP BASE Intermediate_Dir "RPS2Dev"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "RPS2Dev"
# PROP Intermediate_Dir "RPS2Dev"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Ot /I "$(TnT)\x_files" /D "TARGET_PS2_DEV" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Ot /I "$(TnT)\x_files" /D "TARGET_PS2_DEV" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:console /debug /machine:I386 /out:"DPS2Dev/x_Test.elf" /pdbtype:sept /D:TARGET_PS2_DEV
# ADD LINK32 /nologo /subsystem:console /debug /machine:I386 /out:"DPS2Dev/x_Test.elf" /pdbtype:sept /D:TARGET_PS2_DEV
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "x_Test - Win32 Debug"
# Name "x_Test - Win32 Release"
# Name "x_Test - Win32 PC Debug"
# Name "x_Test - Win32 PC Release"
# Name "x_Test - Win32 PS2 DevKit Debug"
# Name "x_Test - Win32 PS2 DevKit Release"
# Begin Source File

SOURCE=.\x_Test.cpp
# End Source File
# Begin Source File

SOURCE=..\3rdParty\PS2\sce\ee\lib\PS2.lib

!IF  "$(CFG)" == "x_Test - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "x_Test - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "x_Test - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "x_Test - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "x_Test - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "x_Test - Win32 PS2 DevKit Release"

!ENDIF 

# End Source File
# End Target
# End Project
