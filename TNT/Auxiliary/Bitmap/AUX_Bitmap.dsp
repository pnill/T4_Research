# Microsoft Developer Studio Project File - Name="AUX_Bitmap" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104
# TARGTYPE "Xbox Static Library" 0x0b04

CFG=AUX_BITMAP - WIN32 XBOX DEVKIT DEBUG
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "AUX_Bitmap.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AUX_Bitmap.mak" CFG="AUX_BITMAP - WIN32 XBOX DEVKIT DEBUG"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AUX_Bitmap - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "AUX_Bitmap - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "AUX_Bitmap - Win32 PC Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "AUX_Bitmap - Win32 PC Release" (based on "Win32 (x86) Static Library")
!MESSAGE "AUX_Bitmap - Win32 PS2 DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "AUX_Bitmap - Win32 PS2 DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "AUX_Bitmap - Win32 PS2 USB Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "AUX_Bitmap - Win32 PS2 USB Release" (based on "Win32 (x86) Static Library")
!MESSAGE "AUX_Bitmap - Win32 PS2 DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "AUX_Bitmap - Win32 PS2 DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "AUX_Bitmap - Win32 Dolphin DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "AUX_Bitmap - Win32 Dolphin DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "AUX_Bitmap - Win32 Dolphin DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "AUX_Bitmap - Win32 Dolphin DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "AUX_Bitmap - Win32 XBox DevKit Debug" (based on "Xbox Static Library")
!MESSAGE "AUX_Bitmap - Win32 XBox DevKit Release" (based on "Xbox Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/TnT/Auxiliary/Bitmap", WSCAAAAA"
# PROP Scc_LocalPath "."

!IF  "$(CFG)" == "AUX_Bitmap - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
CPP=cwcl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "_DEBUG" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "_DEBUG" /YX /FD /GZ /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
CPP=cwcl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "NDEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "NDEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PC Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DPC"
# PROP BASE Intermediate_Dir "DPC"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DPC"
# PROP Intermediate_Dir "DPC"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
CPP=cwcl.exe
# ADD BASE CPP /nologo /G5 /ML /W3 /Zi /Od /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "TARGET_PC" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /G5 /ML /W3 /Zi /Od /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "TARGET_PC" /D "X_DEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PC Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "RPC"
# PROP BASE Intermediate_Dir "RPC"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "RPC"
# PROP Intermediate_Dir "RPC"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
CPP=cwcl.exe
# ADD BASE CPP /nologo /G5 /W3 /O2 /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "TARGET_PC" /YX /FD /c
# ADD CPP /nologo /G5 /W3 /O2 /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "TARGET_PC" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DevKit Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DPS2DEV"
# PROP BASE Intermediate_Dir "DPS2DEV"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DPS2DEV"
# PROP Intermediate_Dir "DPS2DEV"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
CPP=cwcl.exe
# ADD BASE CPP /nologo /W3 /Zi /Od /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "TARGET_PS2_DEV" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /Od /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "TARGET_PS2_DEV" /D "X_DEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DEV
# ADD LIB32 /nologo /D:TARGET_PS2_DEV

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DevKit Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "RPS2DEV"
# PROP BASE Intermediate_Dir "RPS2DEV"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "RPS2DEV"
# PROP Intermediate_Dir "RPS2DEV"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
CPP=cwcl.exe
# ADD BASE CPP /nologo /W3 /O2 /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "TARGET_PS2_DEV" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "TARGET_PS2_DEV" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DEV
# ADD LIB32 /nologo /D:TARGET_PS2_DEV

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 USB Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DPS2USB"
# PROP BASE Intermediate_Dir "DPS2USB"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DPS2USB"
# PROP Intermediate_Dir "DPS2USB"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
CPP=cwcl.exe
# ADD BASE CPP /nologo /W3 /Zi /Od /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "TARGET_PS2_USB" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /Od /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "TARGET_PS2_USB" /D "X_DEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_USB
# ADD LIB32 /nologo /D:TARGET_PS2_USB

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 USB Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "RPS2USB"
# PROP BASE Intermediate_Dir "RPS2USB"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "RPS2USB"
# PROP Intermediate_Dir "RPS2USB"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
CPP=cwcl.exe
# ADD BASE CPP /nologo /W3 /O2 /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "TARGET_PS2_USB" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "TARGET_PS2_USB" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_USB
# ADD LIB32 /nologo /D:TARGET_PS2_USB

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DVD Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DPS2DVD"
# PROP BASE Intermediate_Dir "DPS2DVD"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DPS2DVD"
# PROP Intermediate_Dir "DPS2DVD"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
CPP=cwcl.exe
# ADD BASE CPP /nologo /W3 /Zi /Od /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "TARGET_PS2_DVD" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /Od /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "TARGET_PS2_DVD" /D "X_DEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DVD
# ADD LIB32 /nologo /D:TARGET_PS2_DVD

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DVD Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "RPS2DVD"
# PROP BASE Intermediate_Dir "RPS2DVD"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "RPS2DVD"
# PROP Intermediate_Dir "RPS2DVD"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
CPP=cwcl.exe
# ADD BASE CPP /nologo /W3 /O2 /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "TARGET_PS2_DVD" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "TARGET_PS2_DVD" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DVD
# ADD LIB32 /nologo /D:TARGET_PS2_DVD

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DevKit Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DDOLPHINDEV"
# PROP BASE Intermediate_Dir "DDOLPHINDEV"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DDOLPHINDEV"
# PROP Intermediate_Dir "DDOLPHINDEV"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
CPP=cwcl.exe
# ADD BASE CPP /nologo /W3 /Zi /Od /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "TARGET_DOLPHIN_DEV" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /Od /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "TARGET_DOLPHIN_DEV" /D "X_DEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DEV
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DEV

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DevKit Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "RDOLPHINDEV"
# PROP BASE Intermediate_Dir "RDOLPHINDEV"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "RDOLPHINDEV"
# PROP Intermediate_Dir "RDOLPHINDEV"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
CPP=cwcl.exe
# ADD BASE CPP /nologo /W3 /Od /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "TARGET_DOLPHIN_DEV" /YX /FD /c
# ADD CPP /nologo /W3 /Od /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "TARGET_DOLPHIN_DEV" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DEV
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DEV

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DVD Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DDOLPHINDVD"
# PROP BASE Intermediate_Dir "DDOLPHINDVD"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DDOLPHINDVD"
# PROP Intermediate_Dir "DDOLPHINDVD"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
CPP=cwcl.exe
# ADD BASE CPP /nologo /W3 /Zi /Od /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "TARGET_DOLPHIN_DVD" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /Od /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "TARGET_DOLPHIN_DVD" /D "X_DEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DVD
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DVD

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DVD Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "RDOLPHINDVD"
# PROP BASE Intermediate_Dir "RDOLPHINDVD"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "RDOLPHINDVD"
# PROP Intermediate_Dir "RDOLPHINDVD"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
CPP=cwcl.exe
# ADD BASE CPP /nologo /W3 /Od /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "TARGET_DOLPHIN_DVD" /YX /FD /c
# ADD CPP /nologo /W3 /Od /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "TARGET_DOLPHIN_DVD" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DVD
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DVD

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 XBox DevKit Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DXBOXDEV"
# PROP BASE Intermediate_Dir "DXBOXDEV"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DXBOXDEV"
# PROP Intermediate_Dir "DXBOXDEV"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "_XBOX" /D "TARGET_XBOX_DEV" /D "X_DEBUG" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "_XBOX" /D "TARGET_XBOX_DEV" /D "X_DEBUG" /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 XBox DevKit Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "RXBOXDEV"
# PROP BASE Intermediate_Dir "RXBOXDEV"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "RXBOXDEV"
# PROP Intermediate_Dir "RXBOXDEV"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /Od /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "_XBOX" /D "TARGET_XBOX_DEV" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /GX- /Od /I "$(TnT)\x_files" /I "$(TnT)\Auxiliary\Bitmap" /D "_XBOX" /D "TARGET_XBOX_DEV" /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "AUX_Bitmap - Win32 Debug"
# Name "AUX_Bitmap - Win32 Release"
# Name "AUX_Bitmap - Win32 PC Debug"
# Name "AUX_Bitmap - Win32 PC Release"
# Name "AUX_Bitmap - Win32 PS2 DevKit Debug"
# Name "AUX_Bitmap - Win32 PS2 DevKit Release"
# Name "AUX_Bitmap - Win32 PS2 USB Debug"
# Name "AUX_Bitmap - Win32 PS2 USB Release"
# Name "AUX_Bitmap - Win32 PS2 DVD Debug"
# Name "AUX_Bitmap - Win32 PS2 DVD Release"
# Name "AUX_Bitmap - Win32 Dolphin DevKit Debug"
# Name "AUX_Bitmap - Win32 Dolphin DevKit Release"
# Name "AUX_Bitmap - Win32 Dolphin DVD Debug"
# Name "AUX_Bitmap - Win32 Dolphin DVD Release"
# Name "AUX_Bitmap - Win32 XBox DevKit Debug"
# Name "AUX_Bitmap - Win32 XBox DevKit Release"
# Begin Group "Implementation"

# PROP Default_Filter ""
# Begin Group "Format Handlers"

# PROP Default_Filter ""
# Begin Group "BMP"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BmpLd.cpp

!IF  "$(CFG)" == "AUX_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PC Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DVD Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 XBox DevKit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\BmpLd.hpp
# End Source File
# End Group
# Begin Group "PCX"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PcxLd.cpp

!IF  "$(CFG)" == "AUX_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PC Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DVD Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 XBox DevKit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PcxLd.hpp
# End Source File
# End Group
# Begin Group "TGA"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\TgaLd.cpp

!IF  "$(CFG)" == "AUX_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PC Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DVD Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 XBox DevKit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TgaLd.hpp
# End Source File
# End Group
# Begin Group "GIF"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\GifLd.cpp

!IF  "$(CFG)" == "AUX_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PC Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DVD Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 XBox DevKit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GifLd.hpp
# End Source File
# End Group
# End Group
# Begin Group "External"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\TnT\PS2_in_VC.h
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_bitmap.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_bitmap_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_color.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_color_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_debug.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_memory.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_plus.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_stdio.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_target.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_types.hpp
# End Source File
# End Group
# Begin Group "Util"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AUX_Compress.cpp

!IF  "$(CFG)" == "AUX_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PC Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DVD Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 XBox DevKit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\AUX_SizzleXBOX.cpp

!IF  "$(CFG)" == "AUX_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PC Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DVD Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 XBox DevKit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\AUX_Swizzle.cpp

!IF  "$(CFG)" == "AUX_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PC Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DVD Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 XBox DevKit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\AUX_SwizzleGC.cpp

!IF  "$(CFG)" == "AUX_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PC Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DVD Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 XBox DevKit Release"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\AUX_Bitmap.cpp

!IF  "$(CFG)" == "AUX_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PC Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DVD Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 XBox DevKit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\AUX_BitmapConv.cpp

!IF  "$(CFG)" == "AUX_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PC Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 Dolphin DVD Release"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "AUX_Bitmap - Win32 XBox DevKit Release"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\AUX_Bitmap.hpp
# End Source File
# End Target
# End Project
