# Microsoft Developer Studio Project File - Name="Netspine" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104
# TARGTYPE "Xbox Static Library" 0x0b04

CFG=Netspine - Win32 PC Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "NetSpine.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "NetSpine.mak" CFG="Netspine - Win32 PC Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Netspine - Win32 PC Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Netspine - Win32 PC Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Netspine - Win32 PS2 DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Netspine - Win32 PS2 DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Netspine - Win32 PS2 USB Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Netspine - Win32 PS2 USB Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Netspine - Win32 PS2 DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Netspine - Win32 PS2 DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Netspine - Win32 XBox DevKit Debug" (based on "Xbox Static Library")
!MESSAGE "Netspine - Win32 XBox DevKit Release" (based on "Xbox Static Library")
!MESSAGE "Netspine - Win32 XBox DVD Debug" (based on "Xbox Static Library")
!MESSAGE "Netspine - Win32 XBox DVD Release" (based on "Xbox Static Library")
!MESSAGE "Netspine - Win32 Dolphin DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Netspine - Win32 Dolphin DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Netspine - Win32 Dolphin DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Netspine - Win32 Dolphin DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ASB/TNT/NetSpine", WSMAAAAA"
# PROP Scc_LocalPath "."

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

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
# ADD BASE CPP /nologo /G5 /W3 /GX /Zi /Od /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_PC" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /G5 /MTd /W3 /GX /Zi /Od /I "CPC2\Include" /I "Netspine\Include" /I "Netspine\Src" /I "NetGameSDK2\Include" /I "NetworkCore\Include" /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_PC" /D "X_DEBUG" /D "WIN32" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

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
# ADD BASE CPP /nologo /G5 /W3 /GX /O2 /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_PC" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "CPC2\Include" /I "Netspine\Include" /I "Netspine\Src" /I "NetGameSDK2\Include" /I "NetworkCore\Include" /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_PC" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

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
# ADD BASE CPP /nologo /W3 /Od /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_PS2_DEV" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /Od /I "CPC2\Include" /I "Netspine\Include" /I "Netspine\Src" /I "NetGameSDK2\Include" /I "NetworkCore\Include" /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_PS2_DEV" /D "X_DEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DEV
# ADD LIB32 /nologo /D:TARGET_PS2_DEV

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

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
# ADD BASE CPP /nologo /W3 /O2 /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_PS2_DEV" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /I "CPC2\Include" /I "Netspine\Include" /I "Netspine\Src" /I "NetGameSDK2\Include" /I "NetworkCore\Include" /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_PS2_DEV" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DEV
# ADD LIB32 /nologo /D:TARGET_PS2_DEV

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

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
# ADD BASE CPP /nologo /W3 /Od /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_PS2_USB" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Od /I "CPC2\Include" /I "Netspine\Include" /I "Netspine\Src" /I "NetGameSDK2\Include" /I "NetworkCore\Include" /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_PS2_USB" /D "X_DEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_USB
# ADD LIB32 /nologo /D:TARGET_PS2_USB

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

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
# ADD BASE CPP /nologo /W3 /O2 /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_PS2_USB" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /I "CPC2\Include" /I "Netspine\Include" /I "Netspine\Src" /I "NetGameSDK2\Include" /I "NetworkCore\Include" /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_PS2_USB" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_USB
# ADD LIB32 /nologo /D:TARGET_PS2_USB

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

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
# ADD BASE CPP /nologo /W3 /Od /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_PS2_DVD" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /ZI /Od /I "CPC2\Include" /I "Netspine\Include" /I "Netspine\Src" /I "NetGameSDK2\Include" /I "NetworkCore\Include" /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_PS2_DVD" /D "X_DEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DVD
# ADD LIB32 /nologo /D:TARGET_PS2_DVD

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

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
# ADD BASE CPP /nologo /W3 /O2 /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_PS2_DVD" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /I "CPC2\Include" /I "Netspine\Include" /I "Netspine\Src" /I "NetGameSDK2\Include" /I "NetworkCore\Include" /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_PS2_DVD" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DVD
# ADD LIB32 /nologo /D:TARGET_PS2_DVD

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

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
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_XBOX_DEV" /D "X_DEBUG" /D "_XBOX" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "CPC2\Include" /I "Netspine\Include" /I "Netspine\Src" /I "NetGameSDK2\Include" /I "NetworkCore\Include" /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_XBOX_DEV" /D "X_DEBUG" /D "_XBOX" /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

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
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_XBOX_DEV" /D "_XBOX" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /GX /O2 /I "CPC2\Include" /I "Netspine\Include" /I "Netspine\Src" /I "NetGameSDK2\Include" /I "NetworkCore\Include" /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_XBOX_DEV" /D "_XBOX" /YX /FD /G6 /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DXBOXDVD"
# PROP BASE Intermediate_Dir "DXBOXDVD"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DXBOXDVD"
# PROP Intermediate_Dir "DXBOXDVD"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
RSC=rc.exe
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_XBOX_DVD" /D "X_DEBUG" /D "_XBOX" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "CPC2\Include" /I "Netspine\Include" /I "Netspine\Src" /I "NetGameSDK2\Include" /I "NetworkCore\Include" /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_XBOX_DVD" /D "X_DEBUG" /D "_XBOX" /YX /FD /G6 /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "RXBOXDVD"
# PROP BASE Intermediate_Dir "RXBOXDVD"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "RXBOXDVD"
# PROP Intermediate_Dir "RXBOXDVD"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
RSC=rc.exe
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_XBOX_DVD" /D "_XBOX" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /GX /O2 /I "CPC2\Include" /I "Netspine\Include" /I "Netspine\Src" /I "NetGameSDK2\Include" /I "NetworkCore\Include" /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_XBOX_DVD" /D "_XBOX" /YX /FD /G6 /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

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
# ADD BASE CPP /nologo /W3 /Zi /Od /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_DOLPHIN_DEV" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /O1 /I "CPC2\Include" /I "Netspine\Include" /I "Netspine\Src" /I "NetGameSDK2\Include" /I "NetworkCore\Include" /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_DOLPHIN_DEV" /D "X_DEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DEV
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DEV

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

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
# ADD BASE CPP /nologo /W3 /O2 /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_DOLPHIN_DEV" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /I "CPC2\Include" /I "Netspine\Include" /I "Netspine\Src" /I "NetGameSDK2\Include" /I "NetworkCore\Include" /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_DOLPHIN_DEV" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DEV
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DEV

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

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
# ADD BASE CPP /nologo /W3 /Zi /Od /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_DOLPHIN_DVD" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /Od /I "CPC2\Include" /I "Netspine\Include" /I "Netspine\Src" /I "NetGameSDK2\Include" /I "NetworkCore\Include" /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_DOLPHIN_DVD" /D "X_DEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DVD
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DVD

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

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
# ADD BASE CPP /nologo /W3 /O2 /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_DOLPHIN_DVD" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /I "CPC2\Include" /I "Netspine\Include" /I "Netspine\Src" /I "NetGameSDK2\Include" /I "NetworkCore\Include" /I "$(TNT)\x_files" /I "$(TNT)\Quagmire\Engine" /D "TARGET_DOLPHIN_DVD" /YX /FD /c
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

# Name "Netspine - Win32 PC Debug"
# Name "Netspine - Win32 PC Release"
# Name "Netspine - Win32 PS2 DevKit Debug"
# Name "Netspine - Win32 PS2 DevKit Release"
# Name "Netspine - Win32 PS2 USB Debug"
# Name "Netspine - Win32 PS2 USB Release"
# Name "Netspine - Win32 PS2 DVD Debug"
# Name "Netspine - Win32 PS2 DVD Release"
# Name "Netspine - Win32 XBox DevKit Debug"
# Name "Netspine - Win32 XBox DevKit Release"
# Name "Netspine - Win32 XBox DVD Debug"
# Name "Netspine - Win32 XBox DVD Release"
# Name "Netspine - Win32 Dolphin DevKit Debug"
# Name "Netspine - Win32 Dolphin DevKit Release"
# Name "Netspine - Win32 Dolphin DVD Debug"
# Name "Netspine - Win32 Dolphin DVD Release"
# Begin Group "Netspine Source"

# PROP Default_Filter "*.c, *.cpp"
# Begin Group "Private Headers"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=.\NetSpine\Src\AsymCphrReaderWriter.hpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\BufferedReaderWriter.hpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\ByteReaderWriter.hpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\ClientConnection.hpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\Cstr.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\EndianReaderWriter.hpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\FileTransferImp.hpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\ISFMsgSerializer.hpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\ISFPrivateDefs.hpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\MemoryReaderWriter.hpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\NSConnection.hpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\NSHeader.hpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\NSMsgHelpers.hpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\NSReaderWriter.hpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\NSTokenPublic.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Include\rdas_defs.hpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\rdas_query.hpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\rdas_serializer.hpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\ReaderWriter.hpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\RemoteFileList.hpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Include\ResourceProperty.hpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\Serializable.hpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\SocketReaderWriter.hpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\UserRegMsgSerializer.hpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\NetSpine\Src\AsymCphrReaderWriter.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\BufferedReaderWriter.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\ChallengeClient.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\ChallengeServerMsgSerializer.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\ClientConnection.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\CStr.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\FileTransferImp.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\ISFClient.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\ISFMsgSerializer.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\MemoryReaderWriter.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\NetspineClient.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\NSClientService.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\NSConnection.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\NSHeader.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\NSMsgHelpers.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\NSReaderWriter.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\NSTokenPublic.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\rdas_query.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\rdas_serializer.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\RemoteFileList.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\ResourceClient.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\ResourceProperty.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\SocketReaderWriter.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\UserRegClient.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\UserRegMsgSerializer.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# End Group
# Begin Group "NetSpine Include"

# PROP Default_Filter "*.h, *.hpp"
# Begin Source File

SOURCE=.\NetSpine\Include\ChallengeClient.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Include\ChallengeServerErrors.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Include\ChallengeServerMsgDefs.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Include\FileTransferErrors.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Include\FileTransferMsgDefs.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Include\ISFClient.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Include\ISFErrors.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Include\ISFMsgDefs.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Include\N2KMessageDefs.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Include\NetspineClient.hpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Src\NSClientTick.hpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Include\NSError.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Include\ResourceClient.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Include\ResourceMsgDefs.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Include\UserRegClient.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Include\UserRegDefs.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetSpine\Include\UserRegErrors.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# End Group
# Begin Group "NetGameSDK Source"

# PROP Default_Filter "*.c, *.cpp"
# Begin Source File

SOURCE=.\NetGameSDK2\Src\GuaranteeAlgorithm.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetGameSDK2\Src\GuaranteeProtocol.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetGameSDK2\Src\NetSimProtocol.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetGameSDK2\Src\ng2.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# End Group
# Begin Group "NetGameSDK Include"

# PROP Default_Filter "*.h, *.hpp"
# Begin Source File

SOURCE=.\NetGameSDK2\Include\GuaranteeAlgorithm.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetGameSDK2\Include\GuaranteeProtocol.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetGameSDK2\Include\NetSimProtocol.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetGameSDK2\Include\ng2.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# End Group
# Begin Group "CPC2 Source"

# PROP Default_Filter "*.c"
# Begin Group "CPC XBox"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE=.\CPC2\Src\Xbox\c_fileio.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Src\Xbox\c_memory.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Src\Xbox\c_system.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Src\Xbox\c_time.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "CPC PS2"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE=.\CPC2\Src\PS2\c_fileio.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Src\PS2\c_memory.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Src\PS2\c_system.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Src\PS2\c_time.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "CPC Gamecube"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE=.\CPC2\Src\GCUBE\c_fileio.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Src\GCUBE\c_memory.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Src\GCUBE\c_system.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Src\GCUBE\c_time.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# End Group
# Begin Group "CPC Win32"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE=.\CPC2\Src\Win32\c_conio.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Src\Win32\c_fileio.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Src\Win32\c_memory.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Src\Win32\c_system.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Src\Win32\c_time.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\CPC2\Src\c_dbgsys.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Src\c_files.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Src\c_math.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Src\c_null_conio.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Src\c_null_time.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\CPC2\Src\c_stdlib.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Src\c_string.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Src\c_vararg.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Src\c_vsprtf.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# End Group
# Begin Group "CPC2 Include"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=.\CPC2\Include\c_assert.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Include\c_console.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Include\c_ctype.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Include\c_debug.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Include\c_defs.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Include\c_generic.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Include\c_interface.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Include\c_math.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Include\c_memory.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Include\c_stdio.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Include\c_stdlib.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Include\c_string.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Include\c_system.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Include\c_targets.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Include\c_threads.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Include\c_time.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Include\c_types.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Include\c_vararg.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Include\cpc.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Include\cpc_adv.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Include\cpc_ansi.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CPC2\Include\cpc_defs.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# End Group
# Begin Group "Network Core Source"

# PROP Default_Filter "*.c, *.cpp"
# Begin Group "PS2"

# PROP Default_Filter "*.c, *.cpp"
# Begin Source File

SOURCE=.\NetworkCore\Src\PS2\eznetcnf.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Src\PS2\eznetcnf_internal.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Src\PS2\libezcnf.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Src\PS2\libezctl.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Src\PS2\netcnfif_common.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Src\PS2\ps2_ThreadSystem.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Src\PS2\PS2NS_DeviceManager.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Src\PS2\x_sock_PS2.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "XBox"

# PROP Default_Filter "*.*"
# Begin Source File

SOURCE=.\NetworkCore\Src\XBox\XBoxNS_DeviceManager.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "Win32"

# PROP Default_Filter "*.*"
# Begin Source File

SOURCE=.\NetworkCore\Src\Win32\w32_ThreadSystem.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1
# ADD CPP /D "TARGET_DOLPHIN"
# SUBTRACT CPP /D "TARGET_DOLPHIN_DEV"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Src\Win32\Win32NS_DeviceManager.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1
# ADD CPP /D "TARGET_DOLPHIN"
# SUBTRACT CPP /D "TARGET_DOLPHIN_DEV"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "Gamecube"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\NetworkCore\Src\GameCube\Dol_SockImp.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Src\GameCube\Dol_SockImp.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Src\GameCube\Dolphin_DeviceManager.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Src\GameCube\x_sock_GC.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\NetworkCore\Src\CircularBuffer.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Src\crc.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Src\CXSocket.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Src\des.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Src\EndianHelper.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Src\GenLnkLst.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Src\MemoryStream.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Src\null_ThreadSystem.cpp

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Src\x_sock.c

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# End Group
# Begin Group "Network Core Include"

# PROP Default_Filter "*.h, *.hpp"
# Begin Source File

SOURCE=.\NetworkCore\Include\ciphertype.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Include\CircularBuffer.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Include\crc.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Include\CXSocket.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Include\des.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Include\EndianHelper.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Include\GenLnkLst.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Include\MemoryStream.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Include\misc_aid.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Include\NS_DeviceManager.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Include\ps2NS_DeviceManager.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Include\ThreadSystem.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Include\x_sock.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkCore\Include\x_sockdefs.h

!IF  "$(CFG)" == "Netspine - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PC Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "Netspine - Win32 Dolphin DVD Release"

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
