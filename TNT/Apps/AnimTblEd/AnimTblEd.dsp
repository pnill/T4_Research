# Microsoft Developer Studio Project File - Name="AnimTblEd" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=AnimTblEd - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "AnimTblEd.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AnimTblEd.mak" CFG="AnimTblEd - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AnimTblEd - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "AnimTblEd - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/AnimTblEd", HVEAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "AnimTblEd - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "AnimTblEd - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "AnimTblEd - Win32 Release"
# Name "AnimTblEd - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AnimationEditDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimID1.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimIDSelectionDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimNode.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimTblEd.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimTblEd.rc
# End Source File
# Begin Source File

SOURCE=.\AnimTblEdDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimTblEdGlobals.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimTblEdView.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimTrans.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimTransEditDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimTransList.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimTreeNode.cpp
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\EditStringDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\EquivDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\EquivNode.cpp
# End Source File
# Begin Source File

SOURCE=.\ExportConfigDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ExportDef.cpp
# End Source File
# Begin Source File

SOURCE=.\ExportDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ExportList.cpp
# End Source File
# Begin Source File

SOURCE=.\Flag.cpp
# End Source File
# Begin Source File

SOURCE=.\FlagList.cpp
# End Source File
# Begin Source File

SOURCE=.\FlagsEditDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\GroupNode.cpp
# End Source File
# Begin Source File

SOURCE=.\GroupNodeEditDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\IDList.cpp
# End Source File
# Begin Source File

SOURCE=.\LocoAnimNSR.cpp
# End Source File
# Begin Source File

SOURCE=.\LocoAnimTable.cpp
# End Source File
# Begin Source File

SOURCE=.\LocoArcData.cpp
# End Source File
# Begin Source File

SOURCE=.\LocomotionTables.cpp
# End Source File
# Begin Source File

SOURCE=.\LocomotionTablesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\Method.cpp
# End Source File
# Begin Source File

SOURCE=.\MethodEditDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MetricsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NodeSearchMap.cpp
# End Source File
# Begin Source File

SOURCE=.\Priorities.cpp
# End Source File
# Begin Source File

SOURCE=.\PrioritiesEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\ProjectSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\ProjFile.cpp
# End Source File
# Begin Source File

SOURCE=.\QuestionDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ScanSourceCodeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SourceFilesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SourceIDDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SourceParser.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\SourceParser.lex

!IF  "$(CFG)" == "AnimTblEd - Win32 Release"

# Begin Custom Build - Running Flex on $(InputPath) -- Generating SourceParser.c
InputDir=.
WkspDir=.
InputPath=.\SourceParser.lex

"$(InputDir)/SourceParser.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(WkspDir)/flex.exe -f -oSourceParser.c SourceParser.lex

# End Custom Build

!ELSEIF  "$(CFG)" == "AnimTblEd - Win32 Debug"

# Begin Custom Build - Running Flex on $(InputPath) -- Generating SourceParser.c
InputDir=.
WkspDir=.
InputPath=.\SourceParser.lex

"$(InputDir)/SourceParser.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(WkspDir)/flex.exe -f -oSourceParser.c SourceParser.lex

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SourceScanResults.cpp
# End Source File
# Begin Source File

SOURCE=.\State.cpp
# End Source File
# Begin Source File

SOURCE=.\StateBitsEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\StateClass.cpp
# End Source File
# Begin Source File

SOURCE=.\StateClassList.cpp
# End Source File
# Begin Source File

SOURCE=.\StateName.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Trans.cpp
# End Source File
# Begin Source File

SOURCE=.\UnAnimDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\UpdateView.cpp
# End Source File
# Begin Source File

SOURCE=.\ViewPropertiesDlg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AnimationEditDlg.h
# End Source File
# Begin Source File

SOURCE=.\AnimID1.h
# End Source File
# Begin Source File

SOURCE=.\AnimIDSelectionDlg.h
# End Source File
# Begin Source File

SOURCE=.\AnimNode.h
# End Source File
# Begin Source File

SOURCE=.\AnimTblEd.h
# End Source File
# Begin Source File

SOURCE=.\AnimTblEdDefs.h
# End Source File
# Begin Source File

SOURCE=.\AnimTblEdDoc.h
# End Source File
# Begin Source File

SOURCE=.\AnimTblEdGlobals.h
# End Source File
# Begin Source File

SOURCE=.\AnimTblEdView.h
# End Source File
# Begin Source File

SOURCE=.\AnimTrans.h
# End Source File
# Begin Source File

SOURCE=.\AnimTransEditDlg.h
# End Source File
# Begin Source File

SOURCE=.\AnimTransList.h
# End Source File
# Begin Source File

SOURCE=.\AnimTreeNode.h
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.h
# End Source File
# Begin Source File

SOURCE=.\DataType.h
# End Source File
# Begin Source File

SOURCE=.\EditStringDlg.h
# End Source File
# Begin Source File

SOURCE=.\EquivDlg.h
# End Source File
# Begin Source File

SOURCE=.\EquivNode.h
# End Source File
# Begin Source File

SOURCE=.\ExportConfigDlg.h
# End Source File
# Begin Source File

SOURCE=.\ExportDef.h
# End Source File
# Begin Source File

SOURCE=.\ExportDlg.h
# End Source File
# Begin Source File

SOURCE=.\ExportList.h
# End Source File
# Begin Source File

SOURCE=.\Flag.h
# End Source File
# Begin Source File

SOURCE=.\FlagList.h
# End Source File
# Begin Source File

SOURCE=.\FlagsEditDlg.h
# End Source File
# Begin Source File

SOURCE=.\GroupNode.h
# End Source File
# Begin Source File

SOURCE=.\GroupNodeEditDlg.h
# End Source File
# Begin Source File

SOURCE=.\IDList.h
# End Source File
# Begin Source File

SOURCE=.\LocoAnimNSR.h
# End Source File
# Begin Source File

SOURCE=.\LocoAnimTable.h
# End Source File
# Begin Source File

SOURCE=.\LocoArcData.h
# End Source File
# Begin Source File

SOURCE=.\LocomotionTables.h
# End Source File
# Begin Source File

SOURCE=.\LocomotionTablesDlg.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\Method.h
# End Source File
# Begin Source File

SOURCE=.\MethodEditDlg.h
# End Source File
# Begin Source File

SOURCE=.\MetricsDlg.h
# End Source File
# Begin Source File

SOURCE=.\NodeSearchMap.h
# End Source File
# Begin Source File

SOURCE=.\Priorities.h
# End Source File
# Begin Source File

SOURCE=.\PrioritiesEdit.h
# End Source File
# Begin Source File

SOURCE=.\ProjectSettings.h
# End Source File
# Begin Source File

SOURCE=.\ProjFile.h
# End Source File
# Begin Source File

SOURCE=.\QuestionDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\ScanSourceCodeDlg.h
# End Source File
# Begin Source File

SOURCE=.\SourceFilesDlg.h
# End Source File
# Begin Source File

SOURCE=.\SourceIDDlg.h
# End Source File
# Begin Source File

SOURCE=.\SourceParser.h
# End Source File
# Begin Source File

SOURCE=.\SourceScanResults.h
# End Source File
# Begin Source File

SOURCE=.\State.h
# End Source File
# Begin Source File

SOURCE=.\StateBitsEdit.h
# End Source File
# Begin Source File

SOURCE=.\StateClass.h
# End Source File
# Begin Source File

SOURCE=.\StateClassList.h
# End Source File
# Begin Source File

SOURCE=.\StateName.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Trans.h
# End Source File
# Begin Source File

SOURCE=.\UnAnimDlg.h
# End Source File
# Begin Source File

SOURCE=.\UpdateView.h
# End Source File
# Begin Source File

SOURCE=.\ViewPropertiesDlg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\AnimTblEd.ico
# End Source File
# Begin Source File

SOURCE=.\res\AnimTblEd.rc2
# End Source File
# Begin Source File

SOURCE=.\res\AnimTblEdDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\res\dragcurs.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\res\qbc99.ICO
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar1.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\AnimTblEd.reg
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
