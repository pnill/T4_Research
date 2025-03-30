# Microsoft Developer Studio Project File - Name="x_files" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=x_files - Win32 XBox PC Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "x_files.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "x_files.mak" CFG="x_files - Win32 XBox PC Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "x_files - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "x_files - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "x_files - Win32 PC Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "x_files - Win32 PC Release" (based on "Win32 (x86) Static Library")
!MESSAGE "x_files - Win32 PS2 DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "x_files - Win32 PS2 DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "x_files - Win32 PS2 USB Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "x_files - Win32 PS2 USB Release" (based on "Win32 (x86) Static Library")
!MESSAGE "x_files - Win32 PS2 DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "x_files - Win32 PS2 DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "x_files - Win32 Dolphin DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "x_files - Win32 Dolphin DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "x_files - Win32 Dolphin DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "x_files - Win32 Dolphin DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "x_files - Win32 XBox DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "x_files - Win32 XBox DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "x_files - Win32 XBox DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "x_files - Win32 XBox DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "x_files - Win32 XBox PC Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "x_files - Win32 XBox PC Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/TnT/x_files", GFGAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "x_files - Win32 Debug"

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
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "X_DEBUG" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "X_DEBUG" /D "_CONSOLE" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "x_files - Win32 Release"

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
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "x_files - Win32 PC Debug"

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
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Zi /Od /D "TARGET_PC" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /G5 /ML /W3 /Zi /Od /D "TARGET_PC" /D "X_DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "x_files - Win32 PC Release"

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
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /O2 /D "TARGET_PC" /YX /FD /c
# ADD CPP /nologo /G5 /W3 /O2 /D "TARGET_PC" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "x_files - Win32 PS2 DevKit Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DPS2Dev"
# PROP BASE Intermediate_Dir "DPS2Dev"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DPS2Dev"
# PROP Intermediate_Dir "DPS2Dev"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Zi /Od /D "TARGET_PS2_DEV" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /Od /D "TARGET_PS2_DEV" /D "X_DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DEV
# ADD LIB32 /nologo /D:TARGET_PS2_DEV

!ELSEIF  "$(CFG)" == "x_files - Win32 PS2 DevKit Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "RPS2Dev"
# PROP BASE Intermediate_Dir "RPS2Dev"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "RPS2Dev"
# PROP Intermediate_Dir "RPS2Dev"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /O2 /D "TARGET_PS2_DEV" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /D "TARGET_PS2_DEV" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DEV
# ADD LIB32 /nologo /D:TARGET_PS2_DEV

!ELSEIF  "$(CFG)" == "x_files - Win32 PS2 USB Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DPS2USB"
# PROP BASE Intermediate_Dir "DPS2USB"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DPS2USB"
# PROP Intermediate_Dir "DPS2USB"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Zi /Od /D "TARGET_PS2_USB" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /Od /D "TARGET_PS2_USB" /D "X_DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_USB
# ADD LIB32 /nologo /D:TARGET_PS2_USB

!ELSEIF  "$(CFG)" == "x_files - Win32 PS2 USB Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "RPS2USB"
# PROP BASE Intermediate_Dir "RPS2USB"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "RPS2USB"
# PROP Intermediate_Dir "RPS2USB"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /O2 /D "TARGET_PS2_USB" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /D "TARGET_PS2_USB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_USB
# ADD LIB32 /nologo /D:TARGET_PS2_USB

!ELSEIF  "$(CFG)" == "x_files - Win32 PS2 DVD Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DPS2DVD"
# PROP BASE Intermediate_Dir "DPS2DVD"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DPS2DVD"
# PROP Intermediate_Dir "DPS2DVD"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Zi /Od /D "TARGET_PS2_DVD" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /Od /D "TARGET_PS2_DVD" /D "X_DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DVD
# ADD LIB32 /nologo /D:TARGET_PS2_DVD

!ELSEIF  "$(CFG)" == "x_files - Win32 PS2 DVD Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "RPS2DVD"
# PROP BASE Intermediate_Dir "RPS2DVD"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "RPS2DVD"
# PROP Intermediate_Dir "RPS2DVD"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /O2 /D "TARGET_PS2_DVD" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /D "TARGET_PS2_DVD" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DVD
# ADD LIB32 /nologo /D:TARGET_PS2_DVD

!ELSEIF  "$(CFG)" == "x_files - Win32 Dolphin DevKit Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DDolphinDev"
# PROP BASE Intermediate_Dir "DDolphinDev"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DDolphinDev"
# PROP Intermediate_Dir "DDolphinDev"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Zi /Od /D "TARGET_DOLPHIN_DEV" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /Od /D "TARGET_DOLPHIN_DEV" /D "X_DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DEV
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DEV

!ELSEIF  "$(CFG)" == "x_files - Win32 Dolphin DevKit Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "RDolphinDev"
# PROP BASE Intermediate_Dir "RDolphinDev"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "RDolphinDev"
# PROP Intermediate_Dir "RDolphinDev"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /O2 /D "TARGET_DOLPHIN_DEV" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /D "TARGET_DOLPHIN_DEV" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DEV
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DEV

!ELSEIF  "$(CFG)" == "x_files - Win32 Dolphin DVD Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DDolphinDVD"
# PROP BASE Intermediate_Dir "DDolphinDVD"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DDolphinDVD"
# PROP Intermediate_Dir "DDolphinDVD"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Zi /Od /D "TARGET_DOLPHIN_DVD" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /Od /D "TARGET_DOLPHIN_DVD" /D "X_DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DVD
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DVD

!ELSEIF  "$(CFG)" == "x_files - Win32 Dolphin DVD Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "RDolphinDVD"
# PROP BASE Intermediate_Dir "RDolphinDVD"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "RDolphinDVD"
# PROP Intermediate_Dir "RDolphinDVD"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /O2 /D "TARGET_DOLPHIN_DVD" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /D "TARGET_DOLPHIN_DVD" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DVD
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DVD

!ELSEIF  "$(CFG)" == "x_files - Win32 XBox DevKit Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DXBoxDev"
# PROP BASE Intermediate_Dir "DXBoxDev"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DXBoxDev"
# PROP Intermediate_Dir "DXBoxDev"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Zi /Od /D "TARGET_XBOX_DEV" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /Od /D "TARGET_XBOX_DEV" /D "X_DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_XBOX_DEV
# ADD LIB32 /nologo /D:TARGET_XBOX_DEV

!ELSEIF  "$(CFG)" == "x_files - Win32 XBox DevKit Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "RXBoxDev"
# PROP BASE Intermediate_Dir "RXBoxDev"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "RXBoxDev"
# PROP Intermediate_Dir "RXBoxDev"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /O2 /D "TARGET_XBOX_DEV" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /D "TARGET_XBOX_DEV" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_XBOX_DEV
# ADD LIB32 /nologo /D:TARGET_XBOX_DEV

!ELSEIF  "$(CFG)" == "x_files - Win32 XBox DVD Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DXBoxDVD"
# PROP BASE Intermediate_Dir "DXBoxDVD"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DXBoxDVD"
# PROP Intermediate_Dir "DXBoxDVD"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Zi /Od /D "TARGET_XBOX_DVD" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /Od /D "TARGET_XBOX_DVD" /D "X_DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_XBOX_DVD
# ADD LIB32 /nologo /D:TARGET_XBOX_DVD

!ELSEIF  "$(CFG)" == "x_files - Win32 XBox DVD Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "RXBoxDVD"
# PROP BASE Intermediate_Dir "RXBoxDVD"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "RXBoxDVD"
# PROP Intermediate_Dir "RXBoxDVD"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /O2 /D "TARGET_XBOX_DVD" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /D "TARGET_XBOX_DVD" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_XBOX_DVD
# ADD LIB32 /nologo /D:TARGET_XBOX_DVD

!ELSEIF  "$(CFG)" == "x_files - Win32 XBox PC Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "x_files___Win32_XBox_PC_Debug"
# PROP BASE Intermediate_Dir "x_files___Win32_XBox_PC_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "x_files___Win32_XBox_PC_Debug"
# PROP Intermediate_Dir "x_files___Win32_XBox_PC_Debug"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Zi /Od /D "TARGET_XBOX_DEV" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /Od /D "X_DEBUG" /D "TARGET_XBOX_PC" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_XBOX_DEV
# ADD LIB32 /nologo /out:"DXBoxPC\x_files.lib" /D:TARGET_XBOX_DEV

!ELSEIF  "$(CFG)" == "x_files - Win32 XBox PC Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x_files___Win32_XBox_PC_Release"
# PROP BASE Intermediate_Dir "x_files___Win32_XBox_PC_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "x_files___Win32_XBox_PC_Release"
# PROP Intermediate_Dir "x_files___Win32_XBox_PC_Release"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /O2 /D "TARGET_XBOX_DEV" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /D "TARGET_XBOX_PC" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_XBOX_DEV
# ADD LIB32 /nologo /out:"RXBoxPC\x_files.lib" /D:TARGET_XBOX_DEV

!ENDIF 

# Begin Target

# Name "x_files - Win32 Debug"
# Name "x_files - Win32 Release"
# Name "x_files - Win32 PC Debug"
# Name "x_files - Win32 PC Release"
# Name "x_files - Win32 PS2 DevKit Debug"
# Name "x_files - Win32 PS2 DevKit Release"
# Name "x_files - Win32 PS2 USB Debug"
# Name "x_files - Win32 PS2 USB Release"
# Name "x_files - Win32 PS2 DVD Debug"
# Name "x_files - Win32 PS2 DVD Release"
# Name "x_files - Win32 Dolphin DevKit Debug"
# Name "x_files - Win32 Dolphin DevKit Release"
# Name "x_files - Win32 Dolphin DVD Debug"
# Name "x_files - Win32 Dolphin DVD Release"
# Name "x_files - Win32 XBox DevKit Debug"
# Name "x_files - Win32 XBox DevKit Release"
# Name "x_files - Win32 XBox DVD Debug"
# Name "x_files - Win32 XBox DVD Release"
# Name "x_files - Win32 XBox PC Debug"
# Name "x_files - Win32 XBox PC Release"
# Begin Group "Implementation"

# PROP Default_Filter ""
# Begin Group "Inline"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\x_bitmap_inline.hpp
# End Source File
# Begin Source File

SOURCE=.\x_color_inline.hpp
# End Source File
# Begin Source File

SOURCE=.\x_math_inline.hpp
# End Source File
# Begin Source File

SOURCE=.\x_math_m4_inline.hpp
# End Source File
# Begin Source File

SOURCE=.\x_math_q_inline.hpp
# End Source File
# Begin Source File

SOURCE=.\x_math_r3_inline.hpp
# End Source File
# Begin Source File

SOURCE=.\x_math_v2_inline.hpp
# End Source File
# Begin Source File

SOURCE=.\x_math_v3_inline.hpp
# End Source File
# Begin Source File

SOURCE=.\x_math_v4_inline.hpp
# End Source File
# End Group
# Begin Group "External"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\3rdParty\PS2\sce\ee\gcc\ee\include\_ansi.h
# End Source File
# Begin Source File

SOURCE="..\..\Program Files\Microsoft Visual Studio\VC98\Include\BASETSD.H"
# End Source File
# Begin Source File

SOURCE=..\3rdParty\PS2\sce\ee\gcc\ee\include\sys\config.h
# End Source File
# Begin Source File

SOURCE=..\3rdParty\PS2\TnT\PS2_in_VC.h
# End Source File
# Begin Source File

SOURCE=..\3rdParty\PS2\sce\ee\gcc\ee\include\sys\reent.h
# End Source File
# Begin Source File

SOURCE=..\3rdParty\PS2\sce\ee\include\sifdev.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\x_bitmap.cpp
# End Source File
# Begin Source File

SOURCE=.\x_debug.cpp
# End Source File
# Begin Source File

SOURCE=.\x_files.cpp
# End Source File
# Begin Source File

SOURCE=.\x_math.cpp
# End Source File
# Begin Source File

SOURCE=.\x_memory.cpp
# End Source File
# Begin Source File

SOURCE=.\x_plus.cpp
# End Source File
# Begin Source File

SOURCE=.\x_sintable.cpp
# End Source File
# Begin Source File

SOURCE=.\x_stdio.cpp
# End Source File
# Begin Source File

SOURCE=.\x_time.cpp
# End Source File
# Begin Source File

SOURCE=.\x_vsprtf.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\x_bitmap.hpp
# End Source File
# Begin Source File

SOURCE=.\x_color.hpp
# End Source File
# Begin Source File

SOURCE=.\x_debug.hpp
# End Source File
# Begin Source File

SOURCE=.\x_files.hpp
# End Source File
# Begin Source File

SOURCE=.\x_math.hpp
# End Source File
# Begin Source File

SOURCE=.\x_memory.hpp
# End Source File
# Begin Source File

SOURCE=.\x_plus.hpp
# End Source File
# Begin Source File

SOURCE=.\x_stats.hpp
# End Source File
# Begin Source File

SOURCE=.\x_stdio.hpp
# End Source File
# Begin Source File

SOURCE=.\x_target.hpp
# End Source File
# Begin Source File

SOURCE=.\x_time.hpp
# End Source File
# Begin Source File

SOURCE=.\x_types.hpp
# End Source File
# End Target
# End Project
