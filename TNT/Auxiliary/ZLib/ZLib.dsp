# Microsoft Developer Studio Project File - Name="ZLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104
# TARGTYPE "Xbox Static Library" 0x0b04

CFG=ZLib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ZLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ZLib.mak" CFG="ZLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ZLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "ZLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ZLib - Win32 PS2 DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "ZLib - Win32 PS2 DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ZLib - Win32 PS2 DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "ZLib - Win32 PS2 DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ZLib - Win32 Dolphin DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "ZLib - Win32 Dolphin DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ZLib - Win32 XBox DevKit Debug" (based on "Xbox Static Library")
!MESSAGE "ZLib - Win32 XBox DevKit Release" (based on "Xbox Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "ZLib - Win32 Debug"

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
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "X_DEBUG" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "X_DEBUG" /YX /FD /GZ /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ZLib - Win32 Release"

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
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "NDEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Debug"

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
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Zi /Od /D "TARGET_PS2_DEV" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /Od /D "TARGET_PS2_DEV" /D "X_DEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DEV
# ADD LIB32 /nologo /D:TARGET_PS2_DEV

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Release"

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
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /O2 /D "TARGET_PS2_DEV" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /D "TARGET_PS2_DEV" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DEV
# ADD LIB32 /nologo /D:TARGET_PS2_DEV

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Debug"

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
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Zi /Od /D "TARGET_PS2_DVD" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /Od /D "TARGET_PS2_DVD" /D "X_DEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DVD
# ADD LIB32 /nologo /D:TARGET_PS2_DVD

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Release"

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
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /O2 /D "TARGET_PS2_DVD" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /D "TARGET_PS2_DVD" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_PS2_DVD
# ADD LIB32 /nologo /D:TARGET_PS2_DVD

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Debug"

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
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Zi /O1 /D "TARGET_DOLPHIN_DEV" /D "X_DEBUG" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /O1 /D "TARGET_DOLPHIN_DEV" /D "X_DEBUG" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DEV
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DEV

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Release"

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
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /O2 /D "TARGET_DOLPHIN_DEV" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /D "TARGET_DOLPHIN_DEV" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /D:TARGET_DOLPHIN_DEV
# ADD LIB32 /nologo /D:TARGET_DOLPHIN_DEV

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "TARGET_XBOX_DEV" /D "X_DEBUG" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "TARGET_XBOX_DEV" /D "X_DEBUG" /YX /FD /G6 /Zvc6 /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "TARGET_XBOX_DEV" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /GX /O2 /D "TARGET_XBOX_DEV" /YX /FD /G6 /Zvc6 /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "ZLib - Win32 Debug"
# Name "ZLib - Win32 Release"
# Name "ZLib - Win32 PS2 DevKit Debug"
# Name "ZLib - Win32 PS2 DevKit Release"
# Name "ZLib - Win32 PS2 DVD Debug"
# Name "ZLib - Win32 PS2 DVD Release"
# Name "ZLib - Win32 Dolphin DevKit Debug"
# Name "ZLib - Win32 Dolphin DevKit Release"
# Name "ZLib - Win32 XBox DevKit Debug"
# Name "ZLib - Win32 XBox DevKit Release"
# Begin Group "Source"

# PROP Default_Filter ""
# Begin Group "Include"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\deflate.h
# End Source File
# Begin Source File

SOURCE=.\infblock.h
# End Source File
# Begin Source File

SOURCE=.\infcodes.h
# End Source File
# Begin Source File

SOURCE=.\inffast.h
# End Source File
# Begin Source File

SOURCE=.\inffixed.h
# End Source File
# Begin Source File

SOURCE=.\inftrees.h
# End Source File
# Begin Source File

SOURCE=.\infutil.h
# End Source File
# Begin Source File

SOURCE=.\trees.h
# End Source File
# Begin Source File

SOURCE=.\zutil.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\adler32.c

!IF  "$(CFG)" == "ZLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\compress.c

!IF  "$(CFG)" == "ZLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crc32.c

!IF  "$(CFG)" == "ZLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\deflate.c

!IF  "$(CFG)" == "ZLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gzio.c

!IF  "$(CFG)" == "ZLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\infblock.c

!IF  "$(CFG)" == "ZLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\infcodes.c

!IF  "$(CFG)" == "ZLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\inffast.c

!IF  "$(CFG)" == "ZLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\inflate.c

!IF  "$(CFG)" == "ZLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\inftrees.c

!IF  "$(CFG)" == "ZLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\infutil.c

!IF  "$(CFG)" == "ZLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\trees.c

!IF  "$(CFG)" == "ZLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\uncompr.c

!IF  "$(CFG)" == "ZLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\zutil.c

!IF  "$(CFG)" == "ZLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 Dolphin DevKit Release"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Debug"

!ELSEIF  "$(CFG)" == "ZLib - Win32 XBox DevKit Release"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\zconf.h
# End Source File
# Begin Source File

SOURCE=.\zlib.h
# End Source File
# End Target
# End Project
