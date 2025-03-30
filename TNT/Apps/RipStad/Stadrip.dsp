# Microsoft Developer Studio Project File - Name="Stadrip" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=Stadrip - Win32 Debug QBC
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Stadrip.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Stadrip.mak" CFG="Stadrip - Win32 Debug QBC"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Stadrip - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Stadrip - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "Stadrip - Win32 Debug QBC" (based on "Win32 (x86) Console Application")
!MESSAGE "Stadrip - Win32 Release QBC" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/TnT/Apps/Stadrip", LGDAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Stadrip - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /I "$(TNT)/x_files" /I "$(TNT)/Auxiliary/Bitmap" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "ASB_VERSION" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib xgraphics.lib /nologo /subsystem:console /incremental:yes /machine:I386 /out:".\Stadium\Stadrip.exe"

!ELSEIF  "$(CFG)" == "Stadrip - Win32 Debug"

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
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(TNT)/Quagmire/Engine" /I "$(TNT)/Auxiliary/Bitmap" /I "$(TNT)/x_files" /I "$(TNT)/Auxiliary/Arglist" /I "$(TNT)/Auxiliary/QRaw" /I "$(TNT)/Auxiliary/Tokenizer" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "TARGET_PC_WINCON" /D "ASB_VERSION" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:".\Stadium\Stadrip.exe" /pdbtype:sept

!ELSEIF  "$(CFG)" == "Stadrip - Win32 Debug QBC"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Stadrip___Win32_Debug_QBC"
# PROP BASE Intermediate_Dir "Stadrip___Win32_Debug_QBC"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_QBC"
# PROP Intermediate_Dir "Debug_QBC"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(TNT)/Auxiliary/Bitmap" /I "$(TNT)/x_files" /I "$(TNT)/Auxiliary/Arglist" /I "$(TNT)/Auxiliary/QRaw" /I "$(TNT)/Auxiliary/Tokenizer" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "TARGET_PC_WINCON" /D "ASB_VERSION" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(TNT)/Auxiliary/Bitmap" /I "$(TNT)/x_files" /I "$(TNT)/Auxiliary/Arglist" /I "$(TNT)/Auxiliary/QRaw" /I "$(TNT)/Auxiliary/Tokenizer" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "TARGET_PC_WINCON" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:".\Stadium\Stadrip.exe" /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib $(TNT)\x_files\Debug\x_files.lib $(TnT)\Auxiliary\Bitmap\Debug\AUX_Bitmap.lib /nologo /subsystem:console /debug /machine:I386 /out:".\Stadium\Stadrip.exe" /pdbtype:sept

!ELSEIF  "$(CFG)" == "Stadrip - Win32 Release QBC"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Stadrip___Win32_Release_QBC"
# PROP BASE Intermediate_Dir "Stadrip___Win32_Release_QBC"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Stadrip___Win32_Release_QBC"
# PROP Intermediate_Dir "Stadrip___Win32_Release_QBC"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(TNT)/x_files" /I "$(TNT)/Auxiliary/Bitmap" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "$(TNT)/x_files" /I "$(TNT)/Auxiliary/Bitmap" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /machine:I386 /out:".\Stadrip.exe"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib $(TnT)\x_files\Release\x_files.lib $(TnT)\Auxiliary\Bitmap\Release\AUX_Bitmap.lib /nologo /subsystem:console /incremental:yes /machine:I386 /out:".\Stadium\Stadrip.exe"

!ENDIF 

# Begin Target

# Name "Stadrip - Win32 Release"
# Name "Stadrip - Win32 Debug"
# Name "Stadrip - Win32 Debug QBC"
# Name "Stadrip - Win32 Release QBC"
# Begin Group "New"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ASB_StadRip.cpp

!IF  "$(CFG)" == "Stadrip - Win32 Release"

!ELSEIF  "$(CFG)" == "Stadrip - Win32 Debug"

!ELSEIF  "$(CFG)" == "Stadrip - Win32 Debug QBC"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Stadrip - Win32 Release QBC"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ASB_StadRip.h

!IF  "$(CFG)" == "Stadrip - Win32 Release"

!ELSEIF  "$(CFG)" == "Stadrip - Win32 Debug"

!ELSEIF  "$(CFG)" == "Stadrip - Win32 Debug QBC"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Stadrip - Win32 Release QBC"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\collision.cpp

!IF  "$(CFG)" == "Stadrip - Win32 Release"

!ELSEIF  "$(CFG)" == "Stadrip - Win32 Debug"

!ELSEIF  "$(CFG)" == "Stadrip - Win32 Debug QBC"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Stadrip - Win32 Release QBC"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\collision.h

!IF  "$(CFG)" == "Stadrip - Win32 Release"

!ELSEIF  "$(CFG)" == "Stadrip - Win32 Debug"

!ELSEIF  "$(CFG)" == "Stadrip - Win32 Debug QBC"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Stadrip - Win32 Release QBC"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\collisionData.cpp

!IF  "$(CFG)" == "Stadrip - Win32 Release"

!ELSEIF  "$(CFG)" == "Stadrip - Win32 Debug"

!ELSEIF  "$(CFG)" == "Stadrip - Win32 Debug QBC"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Stadrip - Win32 Release QBC"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\QBC_StadRip.cpp
# End Source File
# Begin Source File

SOURCE=.\QBC_StadRip.h
# End Source File
# Begin Source File

SOURCE=.\stadPointData.cpp
# End Source File
# Begin Source File

SOURCE=.\stadPointData.h
# End Source File
# Begin Source File

SOURCE=.\StadRip.cpp
# End Source File
# Begin Source File

SOURCE=.\StadRip.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ArgList.cpp
# End Source File
# Begin Source File

SOURCE=.\QRaw.cpp
# End Source File
# Begin Source File

SOURCE=.\QRaw.hpp
# End Source File
# Begin Source File

SOURCE=.\rip.cpp

!IF  "$(CFG)" == "Stadrip - Win32 Release"

!ELSEIF  "$(CFG)" == "Stadrip - Win32 Debug"

# ADD CPP /G6 /Ot /Ow /Oi /Op /Oy
# SUBTRACT CPP /Ox /Oa /Og /Os

!ELSEIF  "$(CFG)" == "Stadrip - Win32 Debug QBC"

# ADD BASE CPP /G6 /Ot /Ow /Oi /Op /Oy
# SUBTRACT BASE CPP /Ox /Oa /Og /Os
# ADD CPP /G6 /Ot /Ow /Oi /Op /Oy
# SUBTRACT CPP /Ox /Oa /Og /Os

!ELSEIF  "$(CFG)" == "Stadrip - Win32 Release QBC"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\rip.hpp
# End Source File
# Begin Source File

SOURCE=.\StadiumEffects.txt
# End Source File
# Begin Source File

SOURCE=.\StadripMain.cpp
# End Source File
# Begin Source File

SOURCE=.\Tokenizer.cpp
# End Source File
# End Target
# End Project
