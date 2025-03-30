# Microsoft Developer Studio Project File - Name="Skin" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104
# TARGTYPE "Xbox Static Library" 0x0b04

CFG=Skin - Win32 PS2 DevKit Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Skin.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Skin.mak" CFG="Skin - Win32 PS2 DevKit Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Skin - Win32 PS2 DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Skin - Win32 PS2 DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Skin - Win32 PS2 DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Skin - Win32 PS2 DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Skin - Win32 XBox DevKit Debug" (based on "Xbox Static Library")
!MESSAGE "Skin - Win32 XBox DevKit Release" (based on "Xbox Static Library")
!MESSAGE "Skin - Win32 PC Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Skin - Win32 PC Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Skin - Win32 PS2 USB Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Skin - Win32 PS2 USB Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Skin - Win32 Dolphin DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Skin - Win32 Dolphin DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Skin - Win32 Dolphin DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Skin - Win32 Dolphin DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/TNT/Auxiliary/Skin", WSGAAAAA"
# PROP Scc_LocalPath "."

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)\Quagmire\Engine\PS2" /I "$(TNT)\Quagmire\Engine\PS2\Helpers" /I "$(TNT)\Auxiliary\AnimPlay" /I "$(TNT)/Auxiliary/Skin" /D "TARGET_PS2_DEV" /D "X_DEBUG" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)\Quagmire\Engine\PS2" /I "$(TNT)\Quagmire\Engine\PS2\Helpers" /I "$(TNT)\Auxiliary\AnimPlay" /I "$(TNT)/Auxiliary/Skin" /D "TARGET_PS2_DEV" /D "X_DEBUG" /YX /FD /GZ /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DEV
# ADD LIB32 /nologo /D:TARGET_PS2_DEV

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "RPS2DEV"
# PROP BASE Intermediate_Dir "RPS2DEV"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "RPS2DEV"
# PROP Intermediate_Dir "RPS2DEV"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
CPP=cwcl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)\Quagmire\Engine\PS2" /I "$(TNT)\Quagmire\Engine\PS2\Helpers" /I "$(TNT)\Auxiliary\AnimPlay" /I "$(TNT)/Auxiliary/Skin" /D "TARGET_PS2_DEV" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /GX /O2 /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)\Quagmire\Engine\PS2" /I "$(TNT)\Quagmire\Engine\PS2\Helpers" /I "$(TNT)\Auxiliary\AnimPlay" /I "$(TNT)/Auxiliary/Skin" /D "TARGET_PS2_DEV" /YX /FD /GZ /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DEV
# ADD LIB32 /nologo /D:TARGET_PS2_DEV

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)\Quagmire\Engine\PS2" /I "$(TNT)\Quagmire\Engine\PS2\Helpers" /I "$(TNT)\Auxiliary\AnimPlay" /I "$(TNT)/Auxiliary/Skin" /D "TARGET_PS2_DVD" /D "X_DEBUG" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)\Quagmire\Engine\PS2" /I "$(TNT)\Quagmire\Engine\PS2\Helpers" /I "$(TNT)\Auxiliary\AnimPlay" /I "$(TNT)/Auxiliary/Skin" /D "TARGET_PS2_DVD" /D "X_DEBUG" /YX /FD /GZ /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DEV
# ADD LIB32 /nologo /D:TARGET_PS2_DEV

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "RPS2DVD"
# PROP BASE Intermediate_Dir "RPS2DVD"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "RPS2DVD"
# PROP Intermediate_Dir "RPS2DVD"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
CPP=cwcl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)\Quagmire\Engine\PS2" /I "$(TNT)\Quagmire\Engine\PS2\Helpers" /I "$(TNT)\Auxiliary\AnimPlay" /I "$(TNT)/Auxiliary/Skin" /D "TARGET_PS2_DVD" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX- /ZI /Od /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)\Quagmire\Engine\PS2" /I "$(TNT)\Quagmire\Engine\PS2\Helpers" /I "$(TNT)\Auxiliary\AnimPlay" /I "$(TNT)/Auxiliary/Skin" /D "TARGET_PS2_DVD" /YX /FD /GZ /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DEV
# ADD LIB32 /nologo /D:TARGET_PS2_DEV

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /I "$(TNT)\Auxiliary\Skin" /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)/Auxiliary/Skin" /I "$(TNT)\Auxiliary\AnimPlay" /D "TARGET_XBOX_DEV" /D "X_DEBUG" /D "_XBOX" /YX /FD /GZ /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /I "$(TNT)\Auxiliary\Skin" /I "$(TNT)\Auxiliary\Skin\XBOX" /I "$(TNT)\Auxiliary\Skin\XBOX\Shaders" /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)\Auxiliary\AnimPlay" /D "TARGET_XBOX_DEV" /D "X_DEBUG" /D "_XBOX" /YX /FD /GZ /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_XBOX_DEV
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "RXBOXDEV"
# PROP BASE Intermediate_Dir "RXBOXDEV"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "RXBOXDEV"
# PROP Intermediate_Dir "RXBOXDEV"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /Od /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)/Auxiliary/Skin" /I "$(TNT)\Auxiliary\AnimPlay" /D "TARGET_XBOX_DEV" /D "X_DEBUG" /D "_XBOX" /YX /FD /GZ /G6 /Zvc6 /c
# ADD CPP /nologo /MT /W3 /O2 /I "$(TNT)\Auxiliary\Skin" /I "$(TNT)\Auxiliary\Skin\XBOX" /I "$(TNT)\Auxiliary\Skin\XBOX\Shaders" /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)\Auxiliary\AnimPlay" /D "TARGET_XBOX_DEV" /D "_XBOX" /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_XBOX_DEV
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

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
# ADD BASE CPP /nologo /G5 /W3 /Gm /GX /ZI /Od /I "$(TNT)\Auxiliary\Skin" /I "$(TNT)\Quagmire\Engine\PC\Helpers" /I "$(TNT)\Quagmire\Engine\PC" /I "$(TNT)\Auxiliary\Tokenizer" /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)\Auxiliary\AnimPlay" /I "$(TNT)\3rdParty\DX8\Include" /I "$(TNT)\3rdParty\DX8\Lib" /I "$(TNT)/Auxiliary/Skin" /D "TARGET_PC" /D "X_DEBUG" /YX /FD /GZ /c
# ADD CPP /nologo /G5 /W3 /Gm /GX /ZI /Od /I "$(TNT)\Auxiliary\Skin" /I "$(TNT)\Quagmire\Engine\PC\Helpers" /I "$(TNT)\Quagmire\Engine\PC" /I "$(TNT)\Auxiliary\Tokenizer" /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)\Auxiliary\AnimPlay" /I "$(TNT)\3rdParty\DX8\Include" /I "$(TNT)\3rdParty\DX8\Lib" /I "$(TNT)/Auxiliary/Skin" /I "$(TNT)/Auxiliary/Skin/PC/Shaders" /D "TARGET_PC" /D "X_DEBUG" /YX /FD /GZ /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

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
# ADD BASE CPP /nologo /G5 /W3 /O2 /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)\3rdParty\DX8\Include" /I "$(TNT)\3rdParty\DX8\Lib" /I "$(TNT)/Auxiliary/Skin" /I "$(TNT)\Auxiliary\AnimPlay" /D "NDEBUG" /YX /FD /c
# ADD CPP /nologo /G5 /W3 /O2 /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)\3rdParty\DX8\Include" /I "$(TNT)\3rdParty\DX8\Lib" /I "$(TNT)/Auxiliary/Skin" /I "$(TNT)\Auxiliary\AnimPlay" /D "NDEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)\Quagmire\Engine\PS2" /I "$(TNT)\Quagmire\Engine\PS2\Helpers" /I "$(TNT)\Auxiliary\AnimPlay" /I "$(TNT)/Auxiliary/Skin" /D "TARGET_PS2_USB" /D "X_DEBUG" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)\Quagmire\Engine\PS2" /I "$(TNT)\Quagmire\Engine\PS2\Helpers" /I "$(TNT)\Auxiliary\AnimPlay" /I "$(TNT)/Auxiliary/Skin" /D "TARGET_PS2_USB" /D "X_DEBUG" /YX /FD /GZ /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DEV
# ADD LIB32 /nologo /D:TARGET_PS2_DEV

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "RPS2USB"
# PROP BASE Intermediate_Dir "RPS2USB"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "RPS2USB"
# PROP Intermediate_Dir "RPS2USB"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
CPP=cwcl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)\Quagmire\Engine\PS2" /I "$(TNT)\Quagmire\Engine\PS2\Helpers" /I "$(TNT)\Auxiliary\AnimPlay" /I "$(TNT)/Auxiliary/Skin" /D "TARGET_PS2_USB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)\Quagmire\Engine\PS2" /I "$(TNT)\Quagmire\Engine\PS2\Helpers" /I "$(TNT)\Auxiliary\AnimPlay" /I "$(TNT)/Auxiliary/Skin" /D "TARGET_PS2_USB" /YX /FD /GZ /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DEV
# ADD LIB32 /nologo /D:TARGET_PS2_DEV

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

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
# ADD BASE CPP /nologo /W3 /Zi /Od /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)/Auxiliary/Skin" /I "$(TNT)\Auxiliary\AnimPlay" /D "TARGET_DOLPHIN_DEV" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /Od /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)/Auxiliary/Skin" /I "$(TNT)\Auxiliary\AnimPlay" /D "TARGET_DOLPHIN_DEV" /D "X_DEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DEV
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DEV

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

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
# ADD BASE CPP /nologo /W3 /O2 /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)/Auxiliary/Skin" /I "$(TNT)\Auxiliary\AnimPlay" /D "TARGET_DOLPHIN_DEV" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)/Auxiliary/Skin" /I "$(TNT)\Auxiliary\AnimPlay" /D "TARGET_DOLPHIN_DEV" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DEV
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DEV

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

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
# ADD BASE CPP /nologo /W3 /Zi /Od /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)/Auxiliary/Skin" /I "$(TNT)\Auxiliary\AnimPlay" /D "TARGET_DOLPHIN_DVD" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /Od /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)/Auxiliary/Skin" /I "$(TNT)\Auxiliary\AnimPlay" /D "TARGET_DOLPHIN_DVD" /D "X_DEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DVD
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DVD

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

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
# ADD BASE CPP /nologo /W3 /O2 /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)/Auxiliary/Skin" /I "$(TNT)\Auxiliary\AnimPlay" /D "TARGET_DOLPHIN_DVD" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /I "$(TNT)/Auxiliary/Skin" /I "$(TNT)\Auxiliary\AnimPlay" /D "TARGET_DOLPHIN_DVD" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DVD
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DVD

!ENDIF 

# Begin Target

# Name "Skin - Win32 PS2 DevKit Debug"
# Name "Skin - Win32 PS2 DevKit Release"
# Name "Skin - Win32 PS2 DVD Debug"
# Name "Skin - Win32 PS2 DVD Release"
# Name "Skin - Win32 XBox DevKit Debug"
# Name "Skin - Win32 XBox DevKit Release"
# Name "Skin - Win32 PC Debug"
# Name "Skin - Win32 PC Release"
# Name "Skin - Win32 PS2 USB Debug"
# Name "Skin - Win32 PS2 USB Release"
# Name "Skin - Win32 Dolphin DevKit Debug"
# Name "Skin - Win32 Dolphin DevKit Release"
# Name "Skin - Win32 Dolphin DVD Debug"
# Name "Skin - Win32 Dolphin DVD Release"
# Begin Group "Implementation"

# PROP Default_Filter ""
# Begin Group "PS2"

# PROP Default_Filter ""
# Begin Group "microcode"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ps2\SKIN.dsm

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - [DSM] $(InputPath)
IntDir=.\DPS2DEV
InputPath=.\ps2\SKIN.dsm
InputName=SKIN

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(PS2)\sce\ee\gcc\bin\ps2dvpas -g -o $(IntDir)\$(InputName).obj -I$(PS2)\sce\ee\include -I$(TNT)\Auxiliary\Skin\PS2 $(InputPath) > $(IntDir)\$(InputName).lst

# End Custom Build

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - [DSM] $(InputPath)
IntDir=.\RPS2DEV
InputPath=.\ps2\SKIN.dsm
InputName=SKIN

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(PS2)\sce\ee\gcc\bin\ps2dvpas -g -o $(IntDir)\$(InputName).obj -I$(PS2)\sce\ee\include -I$(TNT)\Auxiliary\Skin\PS2 $(InputPath) > $(IntDir)\$(InputName).lst

# End Custom Build

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - [DSM] $(InputPath)
IntDir=.\DPS2DVD
InputPath=.\ps2\SKIN.dsm
InputName=SKIN

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(PS2)\sce\ee\gcc\bin\ps2dvpas -g -o $(IntDir)\$(InputName).obj -I$(PS2)\sce\ee\include -I$(TNT)\Auxiliary\Skin\PS2 $(InputPath) > $(IntDir)\$(InputName).lst

# End Custom Build

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - [DSM] $(InputPath)
IntDir=.\RPS2DVD
InputPath=.\ps2\SKIN.dsm
InputName=SKIN

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(PS2)\sce\ee\gcc\bin\ps2dvpas -g -o $(IntDir)\$(InputName).obj -I$(PS2)\sce\ee\include -I$(TNT)\Auxiliary\Skin\PS2 $(InputPath) > $(IntDir)\$(InputName).lst

# End Custom Build

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - [DSM] $(InputPath)
IntDir=.\DPS2USB
InputPath=.\ps2\SKIN.dsm
InputName=SKIN

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(PS2)\sce\ee\gcc\bin\ps2dvpas -g -o $(IntDir)\$(InputName).obj -I$(PS2)\sce\ee\include -I$(TNT)\Auxiliary\Skin\PS2 $(InputPath) > $(IntDir)\$(InputName).lst

# End Custom Build

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1
# PROP Ignore_Default_Tool 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ps2\SKIN_boot.vsm

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ps2\SKIN_DoubleMatrix.vsm

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ps2\SKIN_EnvMapped.vsm

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ps2\SKIN_flush.vsm

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\SKIN_FlushShadow.vsm

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ps2\SKIN_kernel.vsm

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ps2\SKIN_matrix.vsm

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ps2\SKIN_mcode0.vsm

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ps2\SKIN_memlayout.vsm

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ps2\SKIN_SingleMatrix.vsm

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ps2\SKIN_transform.vsm

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\SKIN_TransMorph.vsm
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\PS2\SKIN_TransShadow.vsm

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ps2\SKIN_TripleMatrix.vsm

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\ps2\PS2_Skin.cpp

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2\PS2_Skin.hpp

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "XBOX"

# PROP Default_Filter ""
# Begin Group "Shaders"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\XBOX\Shaders\XBOX_PShader.h

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XBOX\Shaders\XBOX_PShader.psh

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# Begin Custom Build
InputDir=.\XBOX\Shaders
OutDir=.\DXBOXDEV
InputPath=.\XBOX\Shaders\XBOX_PShader.psh
InputName=XBOX_PShader

"$(OutDir)\$(InputName).bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -h -hname s_SKIN_PS_ShadowBuffer $(InputPath) 
	copy $(InputDir)\$(InputName).h $(OutDir)\$(InputName).bin 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

# Begin Custom Build
InputDir=.\XBOX\Shaders
OutDir=.\RXBOXDEV
InputPath=.\XBOX\Shaders\XBOX_PShader.psh
InputName=XBOX_PShader

"$(OutDir)\$(InputName).bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -h -hname s_SKIN_PS_ShadowBuffer $(InputPath) 
	copy $(InputDir)\$(InputName).h $(OutDir)\$(InputName).bin 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XBOX\Shaders\XBOX_VBShader.h

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XBOX\Shaders\XBOX_VBShader.vsh

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# Begin Custom Build - Generating XBOX Vertex Shader code for $(InputPath)
InputDir=.\XBOX\Shaders
OutDir=.\DXBOXDEV
InputPath=.\XBOX\Shaders\XBOX_VBShader.vsh
InputName=XBOX_VBShader

"$(OutDir)\$(InputName).bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -h -hname s_SKIN_ObjShader $(InputPath) 
	copy $(InputDir)\$(InputName).h $(OutDir)\$(InputName).bin 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

# Begin Custom Build - Generating XBOX Vertex Shader code for $(InputPath)
InputDir=.\XBOX\Shaders
OutDir=.\RXBOXDEV
InputPath=.\XBOX\Shaders\XBOX_VBShader.vsh
InputName=XBOX_VBShader

"$(OutDir)\$(InputName).bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -h -hname s_SKIN_ObjShader $(InputPath) 
	copy $(InputDir)\$(InputName).h $(OutDir)\$(InputName).bin 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XBOX\Shaders\XBOX_VBShaderDefines.h

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XBOX\Shaders\XBOX_VBShadowShader.h

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XBOX\Shaders\XBOX_VBShadowShader.vsh

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# Begin Custom Build - Generating XBOX Vertex Shader code for $(InputPath)
InputDir=.\XBOX\Shaders
OutDir=.\DXBOXDEV
InputPath=.\XBOX\Shaders\XBOX_VBShadowShader.vsh
InputName=XBOX_VBShadowShader

"$(OutDir)\$(InputName).bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -h -hname s_SKIN_ShadowShader $(InputPath) 
	copy $(InputDir)\$(InputName).h $(OutDir)\$(InputName).bin 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

# Begin Custom Build - Generating XBOX Vertex Shader code for $(InputPath)
InputDir=.\XBOX\Shaders
OutDir=.\RXBOXDEV
InputPath=.\XBOX\Shaders\XBOX_VBShadowShader.vsh
InputName=XBOX_VBShadowShader

"$(OutDir)\$(InputName).bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -h -hname s_SKIN_ShadowShader $(InputPath) 
	copy $(InputDir)\$(InputName).h $(OutDir)\$(InputName).bin 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\XBOX\XBOX_Skin.cpp

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XBOX\XBOX_Skin.hpp

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XBOX\XBOX_SkinBuffer.cpp

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XBOX\XBOX_SkinBuffer.hpp

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "PC"

# PROP Default_Filter ""
# Begin Group "PC Shaders"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PC\Shaders\PC_VBShader.h
# End Source File
# Begin Source File

SOURCE=.\PC\Shaders\PC_VBShader.vsh
# End Source File
# Begin Source File

SOURCE=.\PC\Shaders\PC_VBShaderDefines.h
# End Source File
# Begin Source File

SOURCE=.\PC\Shaders\PC_VBShadowShader.h
# End Source File
# Begin Source File

SOURCE=.\PC\Shaders\PC_VBShadowShader.vsh
# End Source File
# End Group
# Begin Source File

SOURCE=.\PC\PC_Skin.cpp

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PC\PC_Skin.hpp

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PC\PC_SkinBuffer.cpp

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PC\PC_SkinBuffer.hpp

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "GameCube"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\GameCube\GC_Skin.cpp

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameCube\GC_Skin.hpp

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameCube\GC_SkinHelp.hpp

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameCube\GC_SkinRender.cpp

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\Skin_Common.cpp

!IF  "$(CFG)" == "Skin - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Skin - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\CommonSkinDefines.hpp
# End Source File
# Begin Source File

SOURCE=.\Skin.hpp
# End Source File
# End Target
# End Project
