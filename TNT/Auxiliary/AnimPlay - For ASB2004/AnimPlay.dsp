# Microsoft Developer Studio Project File - Name="AnimPlay" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=AnimPlay - Win32 PC Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "AnimPlay.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AnimPlay.mak" CFG="AnimPlay - Win32 PC Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AnimPlay - Win32 PC Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "AnimPlay - Win32 PC Release" (based on "Win32 (x86) Static Library")
!MESSAGE "AnimPlay - Win32 PS2 DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "AnimPlay - Win32 PS2 DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "AnimPlay - Win32 PS2 USB Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "AnimPlay - Win32 PS2 USB Release" (based on "Win32 (x86) Static Library")
!MESSAGE "AnimPlay - Win32 PS2 DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "AnimPlay - Win32 PS2 DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "AnimPlay - Win32 Dolphin DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "AnimPlay - Win32 Dolphin DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "AnimPlay - Win32 Dolphin DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "AnimPlay - Win32 Dolphin DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "AnimPlay - Win32 XBox DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "AnimPlay - Win32 XBox DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/TnT/Auxiliary/AnimPlay", YDDAAAAA"
# PROP Scc_LocalPath "."
CPP=cwcl.exe
RSC=rc.exe

!IF  "$(CFG)" == "AnimPlay - Win32 PC Debug"

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
# ADD CPP /nologo /G5 /ML /W3 /Zi /Od /I "$(TNT)\x_files" /D "TARGET_PC" /D "X_DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 PC Release"

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
# ADD CPP /nologo /G5 /W3 /Zi /O2 /I "$(TNT)\x_files" /D "TARGET_PC" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 PS2 DevKit Debug"

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
# ADD CPP /nologo /W3 /Zi /Od /I "$(TNT)\x_files" /D "TARGET_PS2_DEV" /D "X_DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DEV
# ADD LIB32 /nologo /D:TARGET_PS2_DEV

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 PS2 DevKit Release"

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
# ADD CPP /nologo /W3 /Zi /O2 /I "$(TNT)\x_files" /D "TARGET_PS2_DEV" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DEV
# ADD LIB32 /nologo /D:TARGET_PS2_DEV

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 PS2 USB Debug"

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
# ADD CPP /nologo /W3 /Zi /Od /I "$(TNT)\x_files" /D "TARGET_PS2_USB" /D "X_DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_USB
# ADD LIB32 /nologo /D:TARGET_PS2_USB

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 PS2 USB Release"

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
# ADD CPP /nologo /W3 /Zi /O2 /I "$(TNT)\x_files" /D "TARGET_PS2_USB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_USB
# ADD LIB32 /nologo /D:TARGET_PS2_USB

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 PS2 DVD Debug"

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
# ADD CPP /nologo /W3 /Zi /Od /I "$(TNT)\x_files" /D "TARGET_PS2_DVD" /D "X_DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DVD
# ADD LIB32 /nologo /D:TARGET_PS2_DVD

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 PS2 DVD Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "RPS2DVD"
# PROP BASE Intermediate_Dir "RPS2DVD"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "RPS2Dev"
# PROP Intermediate_Dir "RPS2DVD"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /O2 /D "TARGET_PS2_DVD" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /O2 /I "$(TNT)\x_files" /D "TARGET_PS2_DVD" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DVD
# ADD LIB32 /nologo /D:TARGET_PS2_DVD

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 Dolphin DevKit Debug"

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
# ADD CPP /nologo /W3 /Zi /Od /I "$(TNT)\x_files" /I "$(TNT)\Auxiliary\AuxRAM" /D "TARGET_DOLPHIN_DEV" /D "X_DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DEV
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DEV

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 Dolphin DevKit Release"

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
# ADD CPP /nologo /W3 /Zi /O2 /I "$(TNT)\x_files" /I "$(TNT)\Auxiliary\AuxRAM" /D "TARGET_DOLPHIN_DEV" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DEV
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DEV

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 Dolphin DVD Debug"

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
# ADD CPP /nologo /W3 /Zi /Od /I "$(TNT)\x_files" /I "$(TNT)\Auxiliary\AuxRAM" /D "TARGET_DOLPHIN_DVD" /D "X_DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DVD
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DVD

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 Dolphin DVD Release"

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
# ADD BASE CPP /nologo /W3 /Zi /O2 /D "TARGET_DOLPHIN_DVD" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /O2 /I "$(TNT)\x_files" /I "$(TNT)\Auxiliary\AuxRAM" /D "TARGET_DOLPHIN_DVD" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DVD
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DVD

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 XBox DevKit Debug"

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
# ADD BASE CPP /nologo /W3 /Od /D "TARGET_XBOX_DEV" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /Od /I "$(TNT)\x_files" /D "TARGET_XBOX_DEV" /D "X_DEBUG" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_XBOX_DEV
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 XBox DevKit Release"

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
# ADD BASE CPP /nologo /W3 /Zi /O2 /D "TARGET_XBOX_DEV" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /I "$(TNT)\x_files" /D "TARGET_XBOX_DEV" /FD /Zm200 /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_XBOX_DEV
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "AnimPlay - Win32 PC Debug"
# Name "AnimPlay - Win32 PC Release"
# Name "AnimPlay - Win32 PS2 DevKit Debug"
# Name "AnimPlay - Win32 PS2 DevKit Release"
# Name "AnimPlay - Win32 PS2 USB Debug"
# Name "AnimPlay - Win32 PS2 USB Release"
# Name "AnimPlay - Win32 PS2 DVD Debug"
# Name "AnimPlay - Win32 PS2 DVD Release"
# Name "AnimPlay - Win32 Dolphin DevKit Debug"
# Name "AnimPlay - Win32 Dolphin DevKit Release"
# Name "AnimPlay - Win32 Dolphin DVD Debug"
# Name "AnimPlay - Win32 Dolphin DVD Release"
# Name "AnimPlay - Win32 XBox DevKit Debug"
# Name "AnimPlay - Win32 XBox DevKit Release"
# Begin Group "Implementation"

# PROP Default_Filter ""
# Begin Group "External"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\3rdParty\Ps2\TnT\PS2_in_VC.h
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_debug.hpp
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

SOURCE=..\..\x_files\x_math_r3_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_math_v2_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_math_v3_inline.hpp
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

SOURCE=..\..\x_files\x_time.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_types.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\acccmp.hpp
# End Source File
# Begin Source File

SOURCE=.\acccmp_d.cpp
# End Source File
# Begin Source File

SOURCE=.\AM_Cache.cpp
# End Source File
# Begin Source File

SOURCE=.\AM_Group.cpp
# End Source File
# Begin Source File

SOURCE=.\AM_Group.hpp
# End Source File
# Begin Source File

SOURCE=.\AM_Play.cpp

!IF  "$(CFG)" == "AnimPlay - Win32 PC Debug"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 PC Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 PS2 DevKit Debug"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 PS2 DevKit Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 PS2 USB Debug"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 PS2 USB Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 PS2 DVD Debug"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 PS2 DVD Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 Dolphin DevKit Debug"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 Dolphin DevKit Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 Dolphin DVD Debug"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 Dolphin DVD Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 XBox DevKit Debug"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "AnimPlay - Win32 XBox DevKit Release"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\AM_Play.hpp
# End Source File
# Begin Source File

SOURCE=.\AM_SGet.cpp
# End Source File
# Begin Source File

SOURCE=.\AM_Skel.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\AnimPlay.hpp
# End Source File
# End Target
# End Project
