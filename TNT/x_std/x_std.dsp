# Microsoft Developer Studio Project File - Name="x_std" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104
# TARGTYPE "Xbox Static Library" 0x0b04

CFG=x_std - Win32 PC Server Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "x_std.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "x_std.mak" CFG="x_std - Win32 PC Server Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "x_std - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "x_std - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "x_std - Win32 PC Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "x_std - Win32 PC Release" (based on "Win32 (x86) Static Library")
!MESSAGE "x_std - Win32 PS2 DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "x_std - Win32 PS2 DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "x_std - Win32 PS2 USB Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "x_std - Win32 PS2 USB Release" (based on "Win32 (x86) Static Library")
!MESSAGE "x_std - Win32 PS2 DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "x_std - Win32 PS2 DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "x_std - Win32 Dolphin DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "x_std - Win32 Dolphin DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "x_std - Win32 Dolphin DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "x_std - Win32 Dolphin DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "x_std - Win32 XBox DevKit Debug" (based on "Xbox Static Library")
!MESSAGE "x_std - Win32 XBox DevKit Release" (based on "Xbox Static Library")
!MESSAGE "x_std - Win32 XBox DVD Debug" (based on "Xbox Static Library")
!MESSAGE "x_std - Win32 XBox DVD Release" (based on "Xbox Static Library")
!MESSAGE "x_std - Win32 PC Server Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "x_std - Win32 PC Server Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/NewTech/TnT/x_std", COTAAAAA"
# PROP Scc_LocalPath "."

!IF  "$(CFG)" == "x_std - Win32 Debug"

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
XBCP=xbecopy.exe
XBE=imagebld.exe
LINK32=link.exe
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "X_DEBUG" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "..\x_files" /D "WIN32" /D "_DEBUG" /D "X_DEBUG" /YX /FD /GZ /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "x_std - Win32 Release"

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
XBCP=xbecopy.exe
XBE=imagebld.exe
LINK32=link.exe
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\x_files" /D "WIN32" /D "NDEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "x_std - Win32 PC Debug"

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
XBCP=xbecopy.exe
XBE=imagebld.exe
LINK32=link.exe
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /Zi /Od /D "TARGET_PC" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /GX /Zi /Od /I "..\x_files" /D "TARGET_PC" /D "X_DEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "x_std - Win32 PC Release"

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
XBCP=xbecopy.exe
XBE=imagebld.exe
LINK32=link.exe
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "TARGET_PC" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\x_files" /D "TARGET_PC" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 DevKit Debug"

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
XBCP=xbecopy.exe
XBE=imagebld.exe
LINK32=link.exe
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Od /D "TARGET_PS2_DEV" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Od /I "..\x_files" /D "TARGET_PS2_DEV" /D "X_DEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DEV
# ADD LIB32 /nologo /D:TARGET_PS2_DEV

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 DevKit Release"

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
XBCP=xbecopy.exe
XBE=imagebld.exe
LINK32=link.exe
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /O2 /D "TARGET_PS2_DEV" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /I "..\x_files" /D "TARGET_PS2_DEV" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DEV
# ADD LIB32 /nologo /D:TARGET_PS2_DEV

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 USB Debug"

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
XBCP=xbecopy.exe
XBE=imagebld.exe
LINK32=link.exe
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Od /D "TARGET_PS2_USB" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Od /I "..\x_files" /D "TARGET_PS2_USB" /D "X_DEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_USB
# ADD LIB32 /nologo /D:TARGET_PS2_USB

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 USB Release"

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
XBCP=xbecopy.exe
XBE=imagebld.exe
LINK32=link.exe
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /O2 /D "TARGET_PS2_USB" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /I "..\x_files" /D "TARGET_PS2_USB" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_USB
# ADD LIB32 /nologo /D:TARGET_PS2_USB

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 DVD Debug"

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
XBCP=xbecopy.exe
XBE=imagebld.exe
LINK32=link.exe
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Od /D "TARGET_PS2_DVD" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Od /I "..\x_files" /D "TARGET_PS2_DVD" /D "X_DEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DVD
# ADD LIB32 /nologo /D:TARGET_PS2_DVD

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 DVD Release"

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
XBCP=xbecopy.exe
XBE=imagebld.exe
LINK32=link.exe
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /O2 /D "TARGET_PS2_DVD" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /I "..\x_files" /D "TARGET_PS2_DVD" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DVD
# ADD LIB32 /nologo /D:TARGET_PS2_DVD

!ELSEIF  "$(CFG)" == "x_std - Win32 Dolphin DevKit Debug"

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
XBCP=xbecopy.exe
XBE=imagebld.exe
LINK32=link.exe
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Zi /Od /D "TARGET_DOLPHIN_DEV" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /Od /I "..\x_files" /D "TARGET_DOLPHIN_DEV" /D "X_DEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DEV
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DEV

!ELSEIF  "$(CFG)" == "x_std - Win32 Dolphin DevKit Release"

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
XBCP=xbecopy.exe
XBE=imagebld.exe
LINK32=link.exe
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /O2 /D "TARGET_DOLPHIN_DEV" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /I "..\x_files" /D "TARGET_DOLPHIN_DEV" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DEV
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DEV

!ELSEIF  "$(CFG)" == "x_std - Win32 Dolphin DVD Debug"

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
XBCP=xbecopy.exe
XBE=imagebld.exe
LINK32=link.exe
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Zi /Od /D "TARGET_DOLPHIN_DVD" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /Od /I "..\x_files" /D "TARGET_DOLPHIN_DVD" /D "X_DEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DVD
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DVD

!ELSEIF  "$(CFG)" == "x_std - Win32 Dolphin DVD Release"

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
XBCP=xbecopy.exe
XBE=imagebld.exe
LINK32=link.exe
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /O2 /D "TARGET_DOLPHIN_DVD" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /I "..\x_files" /D "TARGET_DOLPHIN_DVD" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DVD
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DVD

!ELSEIF  "$(CFG)" == "x_std - Win32 XBox DevKit Debug"

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
XBE=imagebld.exe
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "_XBOX" /D "TARGET_XBOX_DEV" /D "X_DEBUG" /YX /FD /G6 /c
# ADD CPP /nologo /W4 /Gm /GX /Zi /Od /I "$(tnt)\x_files" /D "_XBOX" /D "TARGET_XBOX_DEV" /D "X_DEBUG" /YX /FD /G6 /c

!ELSEIF  "$(CFG)" == "x_std - Win32 XBox DevKit Release"

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
XBE=imagebld.exe
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "_XBOX" /D "TARGET_XBOX_DEV" /YX /FD /F0x6 /c
# ADD CPP /nologo /W4 /GX /O2 /I "$(tnt)\x_files" /D "_XBOX" /D "TARGET_XBOX_DEV" /YX /FD /G6 /c

!ELSEIF  "$(CFG)" == "x_std - Win32 XBox DVD Debug"

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
XBE=imagebld.exe
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "_XBOX" /D "TARGET_XBOX_DVD" /D "X_DEBUG" /YX /FD /G6 /c
# ADD CPP /nologo /W4 /Gm /GX /Zi /Od /I "$(tnt)\x_files" /D "_XBOX" /D "TARGET_XBOX_DVD" /D "X_DEBUG" /YX /FD /G6 /c

!ELSEIF  "$(CFG)" == "x_std - Win32 XBox DVD Release"

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
XBE=imagebld.exe
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "_XBOX" /D "TARGET_XBOX_DVD" /YX /FD /G6 /c
# ADD CPP /nologo /W4 /GX /O2 /I "$(tnt)\x_files" /D "_XBOX" /D "TARGET_XBOX_DVD" /YX /FD /G6 /c

!ELSEIF  "$(CFG)" == "x_std - Win32 PC Server Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "x_std___Win32_PC_Server_Debug"
# PROP BASE Intermediate_Dir "x_std___Win32_PC_Server_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "SRVDPC"
# PROP Intermediate_Dir "SRVDPC"
# PROP Target_Dir ""
XBCP=xbecopy.exe
XBE=imagebld.exe
LINK32=link.exe
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /Zi /Od /I "..\x_files" /D "TARGET_PC" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /GX /Zi /Od /I "..\x_files" /D "TARGET_PC" /D "X_DEBUG" /D "_SERVER" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "x_std - Win32 PC Server Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x_std___Win32_PC_Server_Release"
# PROP BASE Intermediate_Dir "x_std___Win32_PC_Server_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "SRVRPC"
# PROP Intermediate_Dir "SRVRPC"
# PROP Target_Dir ""
XBCP=xbecopy.exe
XBE=imagebld.exe
LINK32=link.exe
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\x_files" /D "TARGET_PC" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\x_files" /D "TARGET_PC" /D "_SERVER" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "x_std - Win32 Debug"
# Name "x_std - Win32 Release"
# Name "x_std - Win32 PC Debug"
# Name "x_std - Win32 PC Release"
# Name "x_std - Win32 PS2 DevKit Debug"
# Name "x_std - Win32 PS2 DevKit Release"
# Name "x_std - Win32 PS2 USB Debug"
# Name "x_std - Win32 PS2 USB Release"
# Name "x_std - Win32 PS2 DVD Debug"
# Name "x_std - Win32 PS2 DVD Release"
# Name "x_std - Win32 Dolphin DevKit Debug"
# Name "x_std - Win32 Dolphin DevKit Release"
# Name "x_std - Win32 Dolphin DVD Debug"
# Name "x_std - Win32 Dolphin DVD Release"
# Name "x_std - Win32 XBox DevKit Debug"
# Name "x_std - Win32 XBox DevKit Release"
# Name "x_std - Win32 XBox DVD Debug"
# Name "x_std - Win32 XBox DVD Release"
# Name "x_std - Win32 PC Server Debug"
# Name "x_std - Win32 PC Server Release"
# Begin Source File

SOURCE=.\x_algorithm.h
# End Source File
# Begin Source File

SOURCE=.\x_algorithm2.h
# End Source File
# Begin Source File

SOURCE=.\x_bitset.h
# End Source File
# Begin Source File

SOURCE=.\x_complex.h
# End Source File
# Begin Source File

SOURCE=.\x_deque.h
# End Source File
# Begin Source File

SOURCE=.\x_functional.h
# End Source File
# Begin Source File

SOURCE=.\x_iterator.h
# End Source File
# Begin Source File

SOURCE=.\x_list.h
# End Source File
# Begin Source File

SOURCE=.\x_list2.h
# End Source File
# Begin Source File

SOURCE=.\x_map.h
# End Source File
# Begin Source File

SOURCE=.\x_memory.h
# End Source File
# Begin Source File

SOURCE=.\x_memory2.h
# End Source File
# Begin Source File

SOURCE=.\x_private_tree.h
# End Source File
# Begin Source File

SOURCE=.\x_queue.h
# End Source File
# Begin Source File

SOURCE=.\x_set.h
# End Source File
# Begin Source File

SOURCE=.\x_stack.h
# End Source File
# Begin Source File

SOURCE=.\x_std.cpp

!IF  "$(CFG)" == "x_std - Win32 Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 PC Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 Dolphin DVD Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 PC Server Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 PC Server Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\x_std.h
# End Source File
# Begin Source File

SOURCE=.\x_std_statistics.cpp

!IF  "$(CFG)" == "x_std - Win32 Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 PC Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 Dolphin DVD Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 PC Server Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 PC Server Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\x_string.cpp

!IF  "$(CFG)" == "x_std - Win32 Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 PC Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 Dolphin DVD Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 PC Server Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 PC Server Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\x_string.h
# End Source File
# Begin Source File

SOURCE=.\x_string2.cpp

!IF  "$(CFG)" == "x_std - Win32 Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 PC Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 Dolphin DVD Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 PC Server Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 PC Server Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\x_string2.h
# End Source File
# Begin Source File

SOURCE=.\x_utility.h
# End Source File
# Begin Source File

SOURCE=.\x_valarray.h
# End Source File
# Begin Source File

SOURCE=.\x_vector.cpp

!IF  "$(CFG)" == "x_std - Win32 Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 PC Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 PC Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 USB Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 USB Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 Dolphin DVD Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 Dolphin DVD Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 XBox DevKit Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 XBox DVD Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 XBox DVD Release"

!ELSEIF  "$(CFG)" == "x_std - Win32 PC Server Debug"

!ELSEIF  "$(CFG)" == "x_std - Win32 PC Server Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\x_vector.h
# End Source File
# Begin Source File

SOURCE=.\x_vector2.h
# End Source File
# End Target
# End Project
