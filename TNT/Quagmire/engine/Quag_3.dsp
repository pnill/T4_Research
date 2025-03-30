# Microsoft Developer Studio Project File - Name="Quag_3" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104
# TARGTYPE "Xbox Static Library" 0x0b04

CFG=QUAG_3 - WIN32 PS2 DEVKIT DEBUG
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Quag_3.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Quag_3.mak" CFG="QUAG_3 - WIN32 PS2 DEVKIT DEBUG"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Quag_3 - Win32 PS2 DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Quag_3 - Win32 PS2 DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Quag_3 - Win32 PS2 DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Quag_3 - Win32 PS2 DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Quag_3 - Win32 XBox DevKit Debug" (based on "Xbox Static Library")
!MESSAGE "Quag_3 - Win32 XBox DevKit Release" (based on "Xbox Static Library")
!MESSAGE "Quag_3 - Win32 PC Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Quag_3 - Win32 PC Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Quag_3 - Win32 PS2 USB Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Quag_3 - Win32 PS2 USB Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Quag_3 - Win32 Dolphin Devkit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Quag_3 - Win32 Dolphin Devkit Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/TnT/Quagmire/Engine", CQCAAAAA"
# PROP Scc_LocalPath "."

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(TNT)/x_files" /I "$(TNT)/Quagmire/Engine" /I "$(TNT)/Quagmire/Engine/Common" /I "$(TNT)/Quagmire/Engine/PS2" /I "$(TNT)/Quagmire/Engine/PS2/Helpers" /I "$(TNT)\Auxiliary\Bitmap" /D "TARGET_PS2_DEV" /D "X_DEBUG" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(TNT)/x_files" /I "$(TNT)/Quagmire/Engine" /I "$(TNT)/Quagmire/Engine/Common" /I "$(TNT)/Quagmire/Engine/PS2" /I "$(TNT)/Quagmire/Engine/PS2/Helpers" /I "$(TNT)\Auxiliary\Bitmap" /D "TARGET_PS2_DEV" /D "X_DEBUG" /YX /FD /GZ /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DEV
# ADD LIB32 /nologo /D:TARGET_PS2_DEV

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(TNT)/x_files" /I "$(TNT)/Quagmire/Engine" /I "$(TNT)/Quagmire/Engine/Common" /I "$(TNT)/Quagmire/Engine/PS2" /I "$(TNT)/Quagmire/Engine/PS2/Helpers" /I "$(TNT)\Auxiliary\Bitmap" /D "TARGET_PS2_DEV" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /GX /O2 /I "$(TNT)/x_files" /I "$(TNT)/Quagmire/Engine" /I "$(TNT)/Quagmire/Engine/Common" /I "$(TNT)/Quagmire/Engine/PS2" /I "$(TNT)/Quagmire/Engine/PS2/Helpers" /I "$(TNT)\Auxiliary\Bitmap" /D "TARGET_PS2_DEV" /YX /FD /GZ /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DEV
# ADD LIB32 /nologo /D:TARGET_PS2_DEV

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(TNT)/x_files" /I "$(TNT)/Quagmire/Engine" /I "$(TNT)/Quagmire/Engine/Common" /I "$(TNT)/Quagmire/Engine/PS2" /I "$(TNT)/Quagmire/Engine/PS2/Helpers" /I "$(TNT)\Auxiliary\Bitmap" /D "TARGET_PS2_DVD" /D "X_DEBUG" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(TNT)/x_files" /I "$(TNT)/Quagmire/Engine" /I "$(TNT)/Quagmire/Engine/Common" /I "$(TNT)/Quagmire/Engine/PS2" /I "$(TNT)/Quagmire/Engine/PS2/Helpers" /I "$(TNT)\Auxiliary\Bitmap" /D "TARGET_PS2_DVD" /D "X_DEBUG" /YX /FD /GZ /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DVD
# ADD LIB32 /nologo /D:TARGET_PS2_DVD

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(TNT)/x_files" /I "$(TNT)/Quagmire/Engine" /I "$(TNT)/Quagmire/Engine/Common" /I "$(TNT)/Quagmire/Engine/PS2" /I "$(TNT)/Quagmire/Engine/PS2/Helpers" /I "$(TNT)\Auxiliary\Bitmap" /D "TARGET_PS2_DVD" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /GX- /O2 /I "$(TNT)/x_files" /I "$(TNT)/Quagmire/Engine" /I "$(TNT)/Quagmire/Engine/Common" /I "$(TNT)/Quagmire/Engine/PS2" /I "$(TNT)/Quagmire/Engine/PS2/Helpers" /I "$(TNT)\Auxiliary\Bitmap" /D "TARGET_PS2_DVD" /YX /FD /GZ /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DVD
# ADD LIB32 /nologo /D:TARGET_PS2_DVD

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

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
MTL=midl.exe
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /I "$(TNT)/x_files" /I "$(TNT)/Quagmire/Engine" /I "$(TNT)/Quagmire/Engine/Common" /I "$(TNT)/Quagmire/Engine/XBOX" /I "$(TNT)\Auxiliary\Bitmap" /I "$(TNT)/3rdparty/XBox/include" /D "TARGET_XBOX_DEV" /D "X_DEBUG" /D "_XBOX" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "$(TNT)/x_files" /I "$(TNT)/Quagmire/Engine" /I "$(TNT)/Quagmire/Engine/Common" /I "$(TNT)/Quagmire/Engine/XBOX" /I "$(TNT)\Auxiliary\Bitmap" /I "$(TNT)\Quagmire\Engine\Xbox\Shaders" /D "TARGET_XBOX_DEV" /D "X_DEBUG" /D "_XBOX" /D "ENABLE_ASB_NETWORKING" /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_XBOX_DEV
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

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
MTL=midl.exe
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(TNT)/x_files" /I "$(TNT)/Quagmire/Engine" /I "$(TNT)/Quagmire/Engine/Common" /I "$(TNT)/Quagmire/Engine/XBOX" /I "$(TNT)\Auxiliary\Bitmap" /D "TARGET_XBOX_DEV" /D "_XBOX" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /O2 /I "$(TNT)/x_files" /I "$(TNT)/Quagmire/Engine" /I "$(TNT)/Quagmire/Engine/Common" /I "$(TNT)/Quagmire/Engine/XBOX" /I "$(TNT)\Auxiliary\Bitmap" /I "$(TNT)\Quagmire\Engine\Xbox\Shaders" /D "TARGET_XBOX_DEV" /D "_XBOX" /D "ENABLE_ASB_NETWORKING" /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_XBOX_DEV
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

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
# ADD BASE CPP /nologo /G5 /ML /W3 /Zi /Od /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)\Quagmire\Engine\Common" /I "$(TNT)\Quagmire\Engine\PC" /I "$(TNT)\Auxiliary\Bitmap" /D "TARGET_PC" /D "X_DEBUG" /YX /FD /GZ /c
# ADD CPP /nologo /G5 /ML /W3 /Zi /Od /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)\Quagmire\Engine\Common" /I "$(TNT)\Quagmire\Engine\PC" /I "$(TNT)\Auxiliary\Bitmap" /I "$(TNT)\Quagmire\Engine\PC\Shaders" /D "TARGET_PC" /D "X_DEBUG" /YX /FD /GZ /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

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
# ADD BASE CPP /nologo /G5 /W3 /O2 /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)\Quagmire\Engine\Common" /I "$(TNT)\Quagmire\Engine\PC" /I "$(TNT)\Auxiliary\Bitmap" /D "TARGET_PC" /YX /FD /c
# ADD CPP /nologo /G5 /W3 /O2 /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)\Quagmire\Engine\Common" /I "$(TNT)\Quagmire\Engine\PC" /I "$(TNT)\Auxiliary\Bitmap" /D "TARGET_PC" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(TNT)/x_files" /I "$(TNT)/Quagmire/Engine" /I "$(TNT)/Quagmire/Engine/Common" /I "$(TNT)/Quagmire/Engine/PS2" /I "$(TNT)/Quagmire/Engine/PS2/Helpers" /I "$(TNT)\Auxiliary\Bitmap" /D "TARGET_PS2_USB" /D "X_DEBUG" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(TNT)/x_files" /I "$(TNT)/Quagmire/Engine" /I "$(TNT)/Quagmire/Engine/Common" /I "$(TNT)/Quagmire/Engine/PS2" /I "$(TNT)/Quagmire/Engine/PS2/Helpers" /I "$(TNT)\Auxiliary\Bitmap" /D "TARGET_PS2_USB" /D "X_DEBUG" /YX /FD /GZ /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DEV
# ADD LIB32 /nologo /D:TARGET_PS2_DEV

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(TNT)/x_files" /I "$(TNT)/Quagmire/Engine" /I "$(TNT)/Quagmire/Engine/Common" /I "$(TNT)/Quagmire/Engine/PS2" /I "$(TNT)/Quagmire/Engine/PS2/Helpers" /I "$(TNT)\Auxiliary\Bitmap" /D "TARGET_PS2_USB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /GX /O2 /I "$(TNT)/x_files" /I "$(TNT)/Quagmire/Engine" /I "$(TNT)/Quagmire/Engine/Common" /I "$(TNT)/Quagmire/Engine/PS2" /I "$(TNT)/Quagmire/Engine/PS2/Helpers" /I "$(TNT)\Auxiliary\Bitmap" /D "TARGET_PS2_USB" /YX /FD /GZ /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DEV
# ADD LIB32 /nologo /D:TARGET_PS2_DEV

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DGCDev"
# PROP BASE Intermediate_Dir "DGCDev"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DGCDev"
# PROP Intermediate_Dir "DGCDev"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
CPP=cwcl.exe
# ADD BASE CPP /nologo /G6 /W3 /Zi /Od /I "$(TNT)/x_files" /I "$(TNT)/Quagmire/Engine" /I "$(TNT)/Quagmire/Engine/Common" /I "$(TNT)/Quagmire/Engine/Gamecube" /I "$(TNT)\Auxiliary\Bitmap" /I "$(TNT)\3rdparty\dolphin\include" /D "TARGET_DOLPHIN_DEV" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /G6 /W3 /Zi /Od /I "$(TNT)/x_files" /I "$(TNT)/Quagmire/Engine" /I "$(TNT)/Quagmire/Engine/Common" /I "$(TNT)/Quagmire/Engine/Gamecube" /I "$(TNT)\Auxiliary\Bitmap" /I "$(TNT)\3rdparty\dolphin\include" /D "TARGET_DOLPHIN_DEV" /D "X_DEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DEV
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DEV

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "RGCDev"
# PROP BASE Intermediate_Dir "RGCDev"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "RGCDev"
# PROP Intermediate_Dir "RGCDev"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
CPP=cwcl.exe
# ADD BASE CPP /nologo /G6 /W3 /O2 /I "$(TNT)/x_files" /I "$(TNT)/Quagmire/Engine" /I "$(TNT)/Quagmire/Engine/Common" /I "$(TNT)/Quagmire/Engine/Gamecube" /I "$(TNT)\Auxiliary\Bitmap" /I "$(TNT)\3rdparty\gamecube\include" /D "TARGET_DOLPHIN_DEV" /YX /FD /c
# ADD CPP /nologo /G6 /W3 /O2 /I "$(TNT)/x_files" /I "$(TNT)/Quagmire/Engine" /I "$(TNT)/Quagmire/Engine/Common" /I "$(TNT)/Quagmire/Engine/Gamecube" /I "$(TNT)\Auxiliary\Bitmap" /I "$(TNT)\3rdparty\gamecube\include" /D "TARGET_DOLPHIN_DEV" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DEV
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DEV

!ENDIF 

# Begin Target

# Name "Quag_3 - Win32 PS2 DevKit Debug"
# Name "Quag_3 - Win32 PS2 DevKit Release"
# Name "Quag_3 - Win32 PS2 DVD Debug"
# Name "Quag_3 - Win32 PS2 DVD Release"
# Name "Quag_3 - Win32 XBox DevKit Debug"
# Name "Quag_3 - Win32 XBox DevKit Release"
# Name "Quag_3 - Win32 PC Debug"
# Name "Quag_3 - Win32 PC Release"
# Name "Quag_3 - Win32 PS2 USB Debug"
# Name "Quag_3 - Win32 PS2 USB Release"
# Name "Quag_3 - Win32 Dolphin Devkit Debug"
# Name "Quag_3 - Win32 Dolphin Devkit Release"
# Begin Group "Implementation"

# PROP Default_Filter ""
# Begin Group "Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\E_AccumTimer.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Common\E_AccumTimer.hpp
# End Source File
# Begin Source File

SOURCE=.\Common\E_Geom.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Common\E_InputQ.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Common\E_InputQ.hpp
# End Source File
# Begin Source File

SOURCE=.\Common\E_SMem.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Common\E_SMem.hpp
# End Source File
# Begin Source File

SOURCE=.\Common\E_Text.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Common\E_Text.hpp
# End Source File
# Begin Source File

SOURCE=.\Common\E_View.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Common\Q_FileReadQueue.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Common\Q_FileReadQueue.h
# End Source File
# Begin Source File

SOURCE=.\Q_Geom_inline.hpp
# End Source File
# Begin Source File

SOURCE=.\Common\Q_MemoryUnit.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Common\Q_MemoryUnit.hpp
# End Source File
# Begin Source File

SOURCE=.\Common\Q_MemoryUnitResource.hpp
# End Source File
# End Group
# Begin Group "GameCube"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\GameCube\GC_Debug.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameCube\GC_Draw.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameCube\GC_Draw.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameCube\GC_DVD.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameCube\GC_DVD.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameCube\GC_Engine.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameCube\GC_Font.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameCube\GC_Font.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameCube\GC_FontData.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameCube\GC_Geom.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameCube\GC_Geom.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameCube\GC_Geom_inline.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameCube\GC_Input.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameCube\GC_Input.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameCube\GC_MemoryUnit.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameCube\GC_MemoryUnit.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameCube\GC_MemoryUnitResource.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameCube\GC_Video.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameCube\GC_Video.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameCube\GC_VRAM.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# End Group
# Begin Group "PC"

# PROP Default_Filter ""
# Begin Group "PC Shaders"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PC\Shaders\PC_GeomMultiTexShader.h
# End Source File
# Begin Source File

SOURCE=.\PC\Shaders\PC_GeomMultiTexShader.vsh
# End Source File
# Begin Source File

SOURCE=.\PC\Shaders\PC_GeomObjShader.h
# End Source File
# Begin Source File

SOURCE=.\PC\Shaders\PC_GeomObjShader.vsh
# End Source File
# Begin Source File

SOURCE=.\PC\Shaders\PC_GeomShaderDefines.h
# End Source File
# Begin Source File

SOURCE=.\PC\Shaders\PC_GeomShadowShader.h
# End Source File
# Begin Source File

SOURCE=.\PC\Shaders\PC_GeomShadowShader.vsh
# End Source File
# Begin Source File

SOURCE=.\PC\Shaders\PC_PSMultiTex.h
# End Source File
# Begin Source File

SOURCE=.\PC\Shaders\PC_PSMultiTex.psh
# End Source File
# End Group
# Begin Source File

SOURCE=.\Pc\PC_Debug.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PC\PC_Draw.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PC\PC_Draw.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PC\PC_Engine.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PC\PC_Font.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PC\PC_Font.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PC\PC_FontData.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Pc\PC_Geom.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PC\PC_Geom.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PC\PC_Geom_inline.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PC\PC_Input.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PC\PC_Input.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PC\PC_MemoryUnit.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PC\PC_MemoryUnit.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PC\PC_MemoryUnitResource.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PC\PC_Shaders.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PC\PC_Video.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PC\PC_Video.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PC\PC_VRAM.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "PS2"

# PROP Default_Filter ""
# Begin Group "Helpers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Ps2\Helpers\dmahelp.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ps2\Helpers\dmahelp.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ps2\Helpers\dmaman.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ps2\Helpers\dmaman.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ps2\Helpers\dvtrace.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ps2\Helpers\dvtrace.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\Helpers\SPRhelp.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\Helpers\SPRhelp.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ps2\Helpers\vifhelp.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ps2\Helpers\vifhelp.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "DRAW"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Ps2\PS2_Draw.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ps2\PS2_DrawLines.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ps2\PS2_DrawPoints.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\PS2_DrawRects.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ps2\PS2_DrawSprites.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ps2\PS2_DrawTrans.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ps2\PS2_DrawTris.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ps2\PS2_DrawTrisClip.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "Geom"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PS2\geom\boot.vsm

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\geom\clipper.vsm

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\geom\geom.dsm

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - [DSM] $(InputPath)
IntDir=.\DPS2DEV
InputPath=.\PS2\geom\geom.dsm
InputName=geom

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(PS2)\sce\ee\gcc\bin\ps2dvpas -g -o $(IntDir)\$(InputName).obj -I$(PS2)\sce\ee\include -I$(TNT)\Quagmire\Engine\PS2\Geom $(InputPath) > $(IntDir)\$(InputName).lst

# End Custom Build

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - [DSM] $(InputPath)
IntDir=.\RPS2DEV
InputPath=.\PS2\geom\geom.dsm
InputName=geom

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(PS2)\sce\ee\gcc\bin\ps2dvpas -g -o $(IntDir)\$(InputName).obj -I$(PS2)\sce\ee\include -I$(TNT)\Quagmire\Engine\PS2\Geom $(InputPath) > $(IntDir)\$(InputName).lst

# End Custom Build

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - [DSM] $(InputPath)
IntDir=.\DPS2DVD
InputPath=.\PS2\geom\geom.dsm
InputName=geom

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(PS2)\sce\ee\gcc\bin\ps2dvpas -g -o $(IntDir)\$(InputName).obj -I$(PS2)\sce\ee\include -I$(TNT)\Quagmire\Engine\PS2\Geom $(InputPath) > $(IntDir)\$(InputName).lst

# End Custom Build

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - [DSM] $(InputPath)
IntDir=.\RPS2DVD
InputPath=.\PS2\geom\geom.dsm
InputName=geom

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(PS2)\sce\ee\gcc\bin\ps2dvpas -g -o $(IntDir)\$(InputName).obj -I$(PS2)\sce\ee\include -I$(TNT)\Quagmire\Engine\PS2\Geom $(InputPath) > $(IntDir)\$(InputName).lst

# End Custom Build

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - [DSM] $(InputPath)
IntDir=.\DPS2USB
InputPath=.\PS2\geom\geom.dsm
InputName=geom

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(PS2)\sce\ee\gcc\bin\ps2dvpas -g -o $(IntDir)\$(InputName).obj -I$(PS2)\sce\ee\include -I$(TNT)\Quagmire\Engine\PS2\Geom $(InputPath) > $(IntDir)\$(InputName).lst

# End Custom Build

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - [DSM] $(InputPath)
IntDir=.\RPS2USB
InputPath=.\PS2\geom\geom.dsm
InputName=geom

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(PS2)\sce\ee\gcc\bin\ps2dvpas -g -o $(IntDir)\$(InputName).obj -I$(PS2)\sce\ee\include -I$(TNT)\Quagmire\Engine\PS2\Geom $(InputPath) > $(IntDir)\$(InputName).lst

# End Custom Build

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\geom\kernel.vsm

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\geom\lights.vsm

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\geom\matrix.vsm

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\geom\mcode0.vsm

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\geom\memlayout.vsm

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\geom\quad_clipper.vsm

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\geom\quads_clip.vsm

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\geom\quads_noclip.vsm

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\geom\tris_clip.vsm

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\geom\tris_clip_lit.vsm

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\geom\tris_envmapped.vsm

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\geom\tris_noclip.vsm

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\geom\tris_noclip_lit.vsm

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\geom\tris_shadow.vsm

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\PS2\Framebuf.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\Framebuf.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ps2\PS2_CD.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ps2\PS2_CD.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ps2\PS2_CDUtil.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ps2\PS2_CDUtil.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ps2\PS2_Debug.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ps2\PS2_Debug.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\PS2_Dev.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\PS2_Dev.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ps2\PS2_Engine.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\PS2_Font.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\PS2_Font.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\PS2_FontData.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\PS2_Geom.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\PS2_Geom.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ps2\PS2_Geom_inline.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ps2\PS2_Input.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ps2\PS2_Input.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\PS2_MemoryUnit.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\PS2_MemoryUnit.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\PS2_MemoryUnitResource.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ps2\PS2_Microcode.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\PS2_Primitive.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\ps2_usb.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\ps2_usb.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\PS2_VRam.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "XBox"

# PROP Default_Filter ""
# Begin Group "Shaders"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\XBOX\Shaders\XBOX_GeomMultiTexShader.h
# End Source File
# Begin Source File

SOURCE=.\XBOX\Shaders\XBOX_GeomMultiTexShader.vsh

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# Begin Custom Build - Generating XBOX Vertex Shader code for $(InputPath)
InputDir=.\XBOX\Shaders
OutDir=.\DXBOXDEV
InputPath=.\XBOX\Shaders\XBOX_GeomMultiTexShader.vsh
InputName=XBOX_GeomMultiTexShader

"$(OutDir)\$(InputName).bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -h -hname s_GEOM_MultiTexShader $(InputPath) 
	copy $(InputDir)\$(InputName).h $(OutDir)\$(InputName).bin 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# Begin Custom Build - Generating XBOX Vertex Shader code for $(InputPath)
InputDir=.\XBOX\Shaders
OutDir=.\RXBOXDEV
InputPath=.\XBOX\Shaders\XBOX_GeomMultiTexShader.vsh
InputName=XBOX_GeomMultiTexShader

"$(OutDir)\$(InputName).bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -h -hname s_GEOM_MultiTexShader $(InputPath) 
	copy $(InputDir)\$(InputName).h $(OutDir)\$(InputName).bin 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XBOX\Shaders\XBOX_GeomObjShader.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XBOX\Shaders\XBOX_GeomObjShader.vsh

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# Begin Custom Build - Generating XBOX Vertex Shader code for $(InputPath)
InputDir=.\XBOX\Shaders
OutDir=.\DXBOXDEV
InputPath=.\XBOX\Shaders\XBOX_GeomObjShader.vsh
InputName=XBOX_GeomObjShader

"$(OutDir)\$(InputName).bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -h -hname s_GEOM_ObjShader $(InputPath) 
	copy $(InputDir)\$(InputName).h $(OutDir)\$(InputName).bin 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# Begin Custom Build - Generating XBOX Vertex Shader code for $(InputPath)
InputDir=.\XBOX\Shaders
OutDir=.\RXBOXDEV
InputPath=.\XBOX\Shaders\XBOX_GeomObjShader.vsh
InputName=XBOX_GeomObjShader

"$(OutDir)\$(InputName).bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -h -hname s_GEOM_ObjShader $(InputPath) 
	copy $(InputDir)\$(InputName).h $(OutDir)\$(InputName).bin 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XBOX\Shaders\XBOX_GeomShaderDefines.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XBOX\Shaders\XBOX_GeomShadowShader.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XBOX\Shaders\XBOX_GeomShadowShader.vsh

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# Begin Custom Build - Generating XBOX Vertex Shader code for $(InputPath)
InputDir=.\XBOX\Shaders
OutDir=.\DXBOXDEV
InputPath=.\XBOX\Shaders\XBOX_GeomShadowShader.vsh
InputName=XBOX_GeomShadowShader

"$(OutDir)\$(InputName).bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -h -hname s_GEOM_ShadowShader $(InputPath) 
	copy $(InputDir)\$(InputName).h $(OutDir)\$(InputName).bin 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# Begin Custom Build - Generating XBOX Vertex Shader code for $(InputPath)
InputDir=.\XBOX\Shaders
OutDir=.\RXBOXDEV
InputPath=.\XBOX\Shaders\XBOX_GeomShadowShader.vsh
InputName=XBOX_GeomShadowShader

"$(OutDir)\$(InputName).bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -h -hname s_GEOM_ShadowShader $(InputPath) 
	copy $(InputDir)\$(InputName).h $(OutDir)\$(InputName).bin 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XBOX\Shaders\XBOX_PSMultiTex.h
# End Source File
# Begin Source File

SOURCE=.\XBOX\Shaders\XBOX_PSMultiTex.psh

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# Begin Custom Build - Generating XBOX Pixel Shader code for $(InputPath)
InputDir=.\XBOX\Shaders
OutDir=.\DXBOXDEV
InputPath=.\XBOX\Shaders\XBOX_PSMultiTex.psh
InputName=XBOX_PSMultiTex

"$(OutDir)\$(InputName).bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /O1 /O2 -nologo -h -hname s_GEOM_PS_MultiTexShader $(InputPath) 
	copy $(InputDir)\$(InputName).h $(OutDir)\$(InputName).bin 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# Begin Custom Build - Generating XBOX Pixel Shader code for $(InputPath)
InputDir=.\XBOX\Shaders
OutDir=.\RXBOXDEV
InputPath=.\XBOX\Shaders\XBOX_PSMultiTex.psh
InputName=XBOX_PSMultiTex

"$(OutDir)\$(InputName).bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -h -hname s_GEOM_PS_MultiTexShader $(InputPath) 
	copy $(InputDir)\$(InputName).h $(OutDir)\$(InputName).bin 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\Xbox\XBOX_Debug.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XBOX\XBOX_Dev.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XBOX\XBOX_Dev.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Xbox\XBOX_Draw.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XBOX\XBOX_Draw.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Xbox\XBOX_Engine.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Xbox\XBOX_Font.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Xbox\XBOX_Font.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Xbox\XBOX_FontData.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Xbox\XBOX_Geom.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Xbox\XBOX_Geom.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Xbox\XBOX_Geom_inline.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Xbox\XBOX_Input.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Xbox\XBOX_Input.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XBOX\XBOX_MemoryUnit.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XBOX\XBOX_MemoryUnit.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XBOX\XBOX_MemoryUnitResource.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Xbox\XBOX_Primitive.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XBOX\XBOX_Shaders.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Xbox\XBOX_Video.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Xbox\XBOX_Video.hpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Xbox\XBOX_VRAM.cpp

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "LIBS"

# PROP Default_Filter ""
# Begin Group "DX8-Libs"

# PROP Default_Filter ""
# End Group
# Begin Group "PS2-Libs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\lib\libvu0.lib

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\lib\libdev.lib

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\lib\libdma.lib

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\lib\libgraph.lib

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\lib\libipu.lib

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\lib\libkernl.lib

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\lib\liblout.lib

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\lib\libmc.lib

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\lib\libmpeg.lib

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\lib\libmsin.lib

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\lib\libmtap.lib

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\lib\libpad.lib

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\lib\libpc.lib

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\lib\libpkt.lib

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\lib\librspu2.lib

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\lib\libsdr.lib

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\lib\libssyn.lib

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\lib\libcdvd.lib

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "GameCube-Libs"

# PROP Default_Filter ""
# End Group
# Begin Group "XBOX-Libs"

# PROP Default_Filter ""
# End Group
# End Group
# Begin Group "External"

# PROP Default_Filter ""
# Begin Group "XFiles-Ext"

# PROP Default_Filter ""
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

SOURCE=..\..\x_files\x_files.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_math.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_math_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_math_m4_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_math_q_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_math_r3_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_math_v2_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_math_v3_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_math_v4_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_memory.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_plus.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_stats.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_stdio.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_target.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_time.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_types.hpp
# End Source File
# End Group
# Begin Group "DX8-Ext"

# PROP Default_Filter ""
# End Group
# Begin Group "PS2-Ext"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\gcc\ee\include\_ansi.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\gcc\ee\include\sys\config.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\include\devfont.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\include\devgif.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\include\devvif0.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\include\devvif1.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\include\devvu0.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\include\devvu1.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\include\eekernel.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\include\eeregs.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\include\eestruct.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\include\eetypes.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\gcc\ee\include\machine\ieeefp.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\include\libdev.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\include\libdma.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\include\libgifpk.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\include\libgraph.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\include\libpad.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\include\libvu0.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\TnT\PS2_in_VC.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\gcc\ee\include\sys\reent.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\include\sifdev.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\sce\ee\include\sifrpc.h

!IF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Quag_3 - Win32 Dolphin Devkit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "GameCube-Ext"

# PROP Default_Filter ""
# End Group
# End Group
# End Group
# Begin Source File

SOURCE=.\Q_Draw.hpp
# End Source File
# Begin Source File

SOURCE=.\Q_Engine.hpp
# End Source File
# Begin Source File

SOURCE=.\Q_GC.hpp
# End Source File
# Begin Source File

SOURCE=.\Q_Geom.hpp
# End Source File
# Begin Source File

SOURCE=.\Q_Input.hpp
# End Source File
# Begin Source File

SOURCE=.\Q_Light.hpp
# End Source File
# Begin Source File

SOURCE=.\Q_PC.hpp
# End Source File
# Begin Source File

SOURCE=.\Q_Primitive.hpp
# End Source File
# Begin Source File

SOURCE=.\Q_PS2.hpp
# End Source File
# Begin Source File

SOURCE=.\Q_SMem.hpp
# End Source File
# Begin Source File

SOURCE=.\Q_Text.hpp
# End Source File
# Begin Source File

SOURCE=.\Q_View.hpp
# End Source File
# Begin Source File

SOURCE=.\Q_VRAM.hpp
# End Source File
# Begin Source File

SOURCE=.\Q_XBOX.hpp
# End Source File
# End Target
# End Project
