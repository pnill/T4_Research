# Microsoft Developer Studio Project File - Name="CeD" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=CeD - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CeD.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CeD.mak" CFG="CeD - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CeD - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "CeD - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/TnT/Apps/CeD", HPZBAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CeD - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "y_files" /I "x_files" /I "anim" /D "NDEBUG" /D "TARGET_PC_MFC" /D "WIN32" /D "_WINDOWS" /D "TARGET_PC_D3D_NO_ENTRY" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 winmm.lib /nologo /subsystem:windows /profile /machine:I386

!ELSEIF  "$(CFG)" == "CeD - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "y_files" /I "x_files" /I "anim" /D "_DEBUG" /D "TARGET_PC_D3D_NO_ENTRY" /D "WIN32" /D "_WINDOWS" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib /nologo /subsystem:windows /profile /debug /machine:I386

!ENDIF 

# Begin Target

# Name "CeD - Win32 Release"
# Name "CeD - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Actor.cpp
# End Source File
# Begin Source File

SOURCE=.\ActorList.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimBar.cpp
# End Source File
# Begin Source File

SOURCE=.\AttachJointDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Camera.cpp
# End Source File
# Begin Source File

SOURCE=.\CeD.cpp
# End Source File
# Begin Source File

SOURCE=.\CeD.rc
# End Source File
# Begin Source File

SOURCE=.\CeDDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\CeDView.cpp
# End Source File
# Begin Source File

SOURCE=.\Character.cpp
# End Source File
# Begin Source File

SOURCE=.\CharacterExport.cpp

!IF  "$(CFG)" == "CeD - Win32 Release"

!ELSEIF  "$(CFG)" == "CeD - Win32 Debug"

# ADD CPP /Od /Ob0

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CharacterList.cpp
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\DefinedEventListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\DefinedEventsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgActorList.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgActorNew.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgCamera1.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgCharacter.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgEventList.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgFilePaths.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgKey.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgLayer.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgMotion.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgMotionList.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgMovieMerge.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgMoviePreview.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgSelectMesh.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgShotList.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgSkelPointList.cpp
# End Source File
# Begin Source File

SOURCE=.\eNode.cpp
# End Source File
# Begin Source File

SOURCE=.\Event.cpp
# End Source File
# Begin Source File

SOURCE=.\EventList.cpp
# End Source File
# Begin Source File

SOURCE=.\ExportFile.cpp
# End Source File
# Begin Source File

SOURCE=.\ExportParams.cpp
# End Source File
# Begin Source File

SOURCE=.\ExportProgress.cpp
# End Source File
# Begin Source File

SOURCE=.\ieArchive.cpp
# End Source File
# Begin Source File

SOURCE=.\ieList.cpp
# End Source File
# Begin Source File

SOURCE=.\Key.cpp
# End Source File
# Begin Source File

SOURCE=.\KeyList.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\Mesh.cpp
# End Source File
# Begin Source File

SOURCE=.\MeshList.cpp
# End Source File
# Begin Source File

SOURCE=.\mfcutils.cpp
# End Source File
# Begin Source File

SOURCE=.\Motion.cpp
# End Source File
# Begin Source File

SOURCE=.\MotionEditFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\MotionEditView.cpp
# End Source File
# Begin Source File

SOURCE=.\MotionFlags.cpp
# End Source File
# Begin Source File

SOURCE=.\MotionList.cpp
# End Source File
# Begin Source File

SOURCE=.\MouseStatic.cpp
# End Source File
# Begin Source File

SOURCE=.\Movie.cpp
# End Source File
# Begin Source File

SOURCE=.\MovieBar.cpp
# End Source File
# Begin Source File

SOURCE=.\MovieExport.cpp
# End Source File
# Begin Source File

SOURCE=.\MovieFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\MovieList.cpp
# End Source File
# Begin Source File

SOURCE=.\MovieView.cpp
# End Source File
# Begin Source File

SOURCE=.\MultiBar.cpp
# End Source File
# Begin Source File

SOURCE=.\NumFrames.cpp
# End Source File
# Begin Source File

SOURCE=.\ProjectFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\ProjectView.cpp
# End Source File
# Begin Source File

SOURCE=.\RenderContext.cpp
# End Source File
# Begin Source File

SOURCE=.\RollupBar.cpp
# End Source File
# Begin Source File

SOURCE=.\RollupCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\RollupPane.cpp
# End Source File
# Begin Source File

SOURCE=.\Shot.cpp
# End Source File
# Begin Source File

SOURCE=.\ShotList.cpp
# End Source File
# Begin Source File

SOURCE=.\Skeleton.cpp
# End Source File
# Begin Source File

SOURCE=.\SkelPoint.cpp
# End Source File
# Begin Source File

SOURCE=.\SkelPointList.cpp
# End Source File
# Begin Source File

SOURCE=.\SortedHeaderCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\SortedListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Actor.h
# End Source File
# Begin Source File

SOURCE=.\ActorList.h
# End Source File
# Begin Source File

SOURCE=.\AnimBar.h
# End Source File
# Begin Source File

SOURCE=.\AttachJointDlg.h
# End Source File
# Begin Source File

SOURCE=.\Camera.h
# End Source File
# Begin Source File

SOURCE=.\CeD.h
# End Source File
# Begin Source File

SOURCE=.\CeDDoc.h
# End Source File
# Begin Source File

SOURCE=.\CeDView.h
# End Source File
# Begin Source File

SOURCE=.\Character.h
# End Source File
# Begin Source File

SOURCE=.\CharacterExport.h
# End Source File
# Begin Source File

SOURCE=.\CharacterList.h
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.h
# End Source File
# Begin Source File

SOURCE=.\DefinedEventListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\DefinedEventsDlg.h
# End Source File
# Begin Source File

SOURCE=.\DlgActorList.h
# End Source File
# Begin Source File

SOURCE=.\DlgActorNew.h
# End Source File
# Begin Source File

SOURCE=.\DlgCamera1.h
# End Source File
# Begin Source File

SOURCE=.\DlgCharacter.h
# End Source File
# Begin Source File

SOURCE=.\DlgEventList.h
# End Source File
# Begin Source File

SOURCE=.\DlgFilePaths.h
# End Source File
# Begin Source File

SOURCE=.\DlgKey.h
# End Source File
# Begin Source File

SOURCE=.\DlgLayer.h
# End Source File
# Begin Source File

SOURCE=.\DlgMotion.h
# End Source File
# Begin Source File

SOURCE=.\DlgMotionList.h
# End Source File
# Begin Source File

SOURCE=.\DlgMovieMerge.h
# End Source File
# Begin Source File

SOURCE=.\DlgMoviePreview.h
# End Source File
# Begin Source File

SOURCE=.\DlgSelectMesh.h
# End Source File
# Begin Source File

SOURCE=.\DlgShotList.h
# End Source File
# Begin Source File

SOURCE=.\DlgSkelPointList.h
# End Source File
# Begin Source File

SOURCE=.\eNode.h
# End Source File
# Begin Source File

SOURCE=.\Event.h
# End Source File
# Begin Source File

SOURCE=.\EventList.h
# End Source File
# Begin Source File

SOURCE=.\ExportFile.h
# End Source File
# Begin Source File

SOURCE=.\ExportParams.h
# End Source File
# Begin Source File

SOURCE=.\ExportProgress.h
# End Source File
# Begin Source File

SOURCE=.\FileTags.h
# End Source File
# Begin Source File

SOURCE=.\ieArchive.h
# End Source File
# Begin Source File

SOURCE=.\ieList.h
# End Source File
# Begin Source File

SOURCE=.\ItemInfo.h
# End Source File
# Begin Source File

SOURCE=.\Key.h
# End Source File
# Begin Source File

SOURCE=.\KeyList.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\Mesh.h
# End Source File
# Begin Source File

SOURCE=.\MeshList.h
# End Source File
# Begin Source File

SOURCE=.\mfcutils.h
# End Source File
# Begin Source File

SOURCE=.\Motion.h
# End Source File
# Begin Source File

SOURCE=.\MotionEditFrame.h
# End Source File
# Begin Source File

SOURCE=.\MotionEditView.h
# End Source File
# Begin Source File

SOURCE=.\MotionFlags.h
# End Source File
# Begin Source File

SOURCE=.\MotionList.h
# End Source File
# Begin Source File

SOURCE=.\MouseStatic.h
# End Source File
# Begin Source File

SOURCE=.\Movie.h
# End Source File
# Begin Source File

SOURCE=.\MovieBar.h
# End Source File
# Begin Source File

SOURCE=.\MovieExport.h
# End Source File
# Begin Source File

SOURCE=.\MovieFrame.h
# End Source File
# Begin Source File

SOURCE=.\MovieList.h
# End Source File
# Begin Source File

SOURCE=.\MovieView.h
# End Source File
# Begin Source File

SOURCE=.\MultiBar.h
# End Source File
# Begin Source File

SOURCE=.\NumFrames.h
# End Source File
# Begin Source File

SOURCE=.\ProjectFrame.h
# End Source File
# Begin Source File

SOURCE=.\ProjectView.h
# End Source File
# Begin Source File

SOURCE=.\RenderContext.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\RollupBar.h
# End Source File
# Begin Source File

SOURCE=.\RollupCtrl.h
# End Source File
# Begin Source File

SOURCE=.\RollupPane.h
# End Source File
# Begin Source File

SOURCE=.\Shot.h
# End Source File
# Begin Source File

SOURCE=.\ShotList.h
# End Source File
# Begin Source File

SOURCE=.\Skeleton.h
# End Source File
# Begin Source File

SOURCE=.\SkelPoint.h
# End Source File
# Begin Source File

SOURCE=.\SkelPointList.h
# End Source File
# Begin Source File

SOURCE=.\SortedHeaderCtrl.h
# End Source File
# Begin Source File

SOURCE=.\SortedListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\animsels.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bbPlay.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00002.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00003.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00004.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00005.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00006.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00007.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00008.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00009.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00010.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00011.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00012.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00013.bmp
# End Source File
# Begin Source File

SOURCE=.\res\CeD.ico
# End Source File
# Begin Source File

SOURCE=.\res\CeD.rc2
# End Source File
# Begin Source File

SOURCE=.\res\CeDDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\Cur00001.cur
# End Source File
# Begin Source File

SOURCE=.\res\cur00002.cur
# End Source File
# Begin Source File

SOURCE=.\res\cur00003.cur
# End Source File
# Begin Source File

SOURCE=.\res\cur00004.cur
# End Source File
# Begin Source File

SOURCE=.\res\cur00005.cur
# End Source File
# Begin Source File

SOURCE=.\res\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\res\Cursor2.cur
# End Source File
# Begin Source File

SOURCE=.\res\facedir.cur
# End Source File
# Begin Source File

SOURCE=.\res\framesta.bmp
# End Source File
# Begin Source File

SOURCE=.\res\goto_end.bmp
# End Source File
# Begin Source File

SOURCE=.\res\goto_sta.bmp
# End Source File
# Begin Source File

SOURCE=.\res\movedir.cur
# End Source File
# Begin Source File

SOURCE=.\res\next_fra.bmp
# End Source File
# Begin Source File

SOURCE=.\res\null_d.bmp
# End Source File
# Begin Source File

SOURCE=.\res\null_f.bmp
# End Source File
# Begin Source File

SOURCE=.\res\null_u.bmp
# End Source File
# Begin Source File

SOURCE=.\res\null_x.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Operatio.cur
# End Source File
# Begin Source File

SOURCE=.\res\playd.bmp
# End Source File
# Begin Source File

SOURCE=.\res\playu.bmp
# End Source File
# Begin Source File

SOURCE=.\res\prev_fra.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Prevd.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Prevf.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Prevu.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Prevx.bmp
# End Source File
# Begin Source File

SOURCE=.\res\projectt.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tab.bmp
# End Source File
# Begin Source File

SOURCE=.\res\testbutt.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar1.bmp
# End Source File
# End Group
# Begin Group "Skel & Anim"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Anim\acccmp.h
# End Source File
# Begin Source File

SOURCE=.\Anim\acccmp_c.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\Anim\acccmp_d.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\Anim\Anim.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\Anim\Anim.h
# End Source File
# Begin Source File

SOURCE=.\Anim\Skel.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\Anim\Skel.h
# End Source File
# End Group
# Begin Group "y_files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\y_files\y_Arg.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\y_files\y_Arg.h
# End Source File
# Begin Source File

SOURCE=.\y_files\y_ChkSum.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\y_files\y_ChkSum.h
# End Source File
# Begin Source File

SOURCE=.\y_files\y_Token.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\y_files\y_Token.h
# End Source File
# End Group
# Begin Group "x_files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\x_files\x_bitmap.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\x_files\x_bitmap.h
# End Source File
# Begin Source File

SOURCE=.\x_files\x_color.h
# End Source File
# Begin Source File

SOURCE=.\x_files\x_debug.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\x_files\x_debug.h
# End Source File
# Begin Source File

SOURCE=.\x_files\x_files.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\x_files\x_files.h
# End Source File
# Begin Source File

SOURCE=.\x_files\x_fixed.h
# End Source File
# Begin Source File

SOURCE=.\x_files\x_mat3.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\x_files\x_math.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\x_files\x_math.h
# End Source File
# Begin Source File

SOURCE=.\x_files\x_memory.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\x_files\x_memory.h
# End Source File
# Begin Source File

SOURCE=.\x_files\x_plus.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\x_files\x_plus.h
# End Source File
# Begin Source File

SOURCE=.\x_files\x_quatrn.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\x_files\x_stats.h
# End Source File
# Begin Source File

SOURCE=.\x_files\x_stdio.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\x_files\x_stdio.h
# End Source File
# Begin Source File

SOURCE=.\x_files\x_target.h
# End Source File
# Begin Source File

SOURCE=.\x_files\x_time.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\x_files\x_time.h
# End Source File
# Begin Source File

SOURCE=.\x_files\x_tri.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\x_files\x_types.h
# End Source File
# Begin Source File

SOURCE=.\x_files\x_vsprtf.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Group "QGeom"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\QRaw.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\QRaw.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
