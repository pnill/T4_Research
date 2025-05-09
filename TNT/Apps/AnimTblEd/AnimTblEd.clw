; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CAnimationEditDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "AnimTblEd.h"
LastPage=0

ClassCount=31
Class1=CAnimTblEdApp
Class2=CAnimTblEdDoc
Class3=CAnimTblEdView
Class4=CMainFrame

ResourceCount=29
Resource1=IDD_UNASSIGNED
Resource2=IDD_SOURCEFILES
Resource3=IDD_EXPORT
Class5=CAboutDlg
Class6=CChildFrame
Class7=CSourceFilesDlg
Resource4=IDD_LOCOMOTIONTALES
Class8=CUnAnimDlg
Resource5=IDR_TREEICONS
Resource6=IDD_ANIMID
Resource7=IDD_QUESTION
Class9=CGroupNodeEditDlg
Resource8=IDD_STATEBITSEDIT
Resource9=IDD_PRIORITYEDIT
Class10=CAnimationEditDlg
Class11=CSourceIDDlg
Resource10=IDD_DIALOG_EDITSTRING
Class12=CExportDlg
Resource11=IDD_TRANSITIONEDIT
Class13=CEquivDlg
Resource12=IDD_VIEWPROPERTIES
Class14=CAnimTransEditDlg
Resource13=IDR_MOUSEMENU
Class15=CAnimIDSelectionDlg
Resource14=IDR_MAINFRAME
Resource15=IDD_METRICS
Class16=CStateBitsEdit
Class17=CStateName
Resource16=IDR_ANIMTBTYPE
Class18=FlagsEditDlg
Resource17=IDD_SOURCEID
Class19=CPrioritiesEdit
Resource18=IDD_EQUIVALENCYEDIT
Class20=CQuestionDlg
Resource19=IDD_EDITSTATENAME
Class21=CStringEdit
Class22=CEditStringDlg
Resource20=IDD_EXPORTCONFIG
Class23=CProjectSettings
Resource21=IDD_PROJECTSETTINGS
Class24=CMetricsDlg
Resource22=IDD_SCANSOURCECODE
Class25=CExportConfigDlg
Resource23=IDD_GROUPEDIT
Class26=CViewPropertiesDlg
Resource24=IDD_FLAGEDIT
Class27=CMethodEditDlg
Resource25=IDD_ANIMATIONEDIT
Class28=CLocomotionTablesDlg
Class29=CAnimKeyListBox
Resource26=IDD_METHODEDIT
Resource27=IDD_ABOUTBOX
Class30=CScanSourceCodeDlg
Resource28=IDR_STATUSICONS
Class31=CSourceScanResults
Resource29=IDD_SCANRESULTS

[CLS:CAnimTblEdApp]
Type=0
HeaderFile=AnimTblEd.h
ImplementationFile=AnimTblEd.cpp
Filter=N
BaseClass=CWinApp
VirtualFilter=AC

[CLS:CAnimTblEdDoc]
Type=0
HeaderFile=AnimTblEdDoc.h
ImplementationFile=AnimTblEdDoc.cpp
Filter=N
LastObject=ID_TREEITEM_IGNORE
BaseClass=CDocument
VirtualFilter=DC

[CLS:CAnimTblEdView]
Type=0
HeaderFile=AnimTblEdView.h
ImplementationFile=AnimTblEdView.cpp
Filter=C
LastObject=ID_GROUP_SWAPEQUIV
BaseClass=CTreeView
VirtualFilter=VWC

[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T


[CLS:CChildFrame]
Type=0
HeaderFile=ChildFrm.h
ImplementationFile=ChildFrm.cpp
Filter=M

[CLS:CAboutDlg]
Type=0
HeaderFile=AnimTblEd.cpp
ImplementationFile=AnimTblEd.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_PRINT_SETUP
Command4=ID_FILE_MRU_FILE1
Command5=ID_APP_EXIT
Command6=ID_VIEW_TOOLBAR
Command7=ID_VIEW_STATUS_BAR
Command8=ID_APP_ABOUT
CommandCount=8

[TB:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_CUT
Command5=ID_EDIT_COPY
Command6=ID_EDIT_PASTE
Command7=ID_FILE_PRINT
Command8=ID_APP_ABOUT
Command9=ID_PROJECT_EXPORT
CommandCount=9

[MNU:IDR_ANIMTBTYPE]
Type=1
Class=CAnimTblEdView
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_CLOSE
Command4=ID_FILE_SAVE
Command5=ID_FILE_SAVE_AS
Command6=ID_FILE_PRINT
Command7=ID_FILE_PRINT_PREVIEW
Command8=ID_FILE_PRINT_SETUP
Command9=ID_FILE_MRU_FILE1
Command10=ID_APP_EXIT
Command11=ID_EDIT_UNDO
Command12=ID_EDIT_CUT
Command13=ID_EDIT_COPY
Command14=ID_EDIT_PASTE
Command15=ID_EDIT_SEARCH
Command16=ID_VIEW_TOOLBAR
Command17=ID_VIEW_STATUS_BAR
Command18=ID_VIEW_PROPERTIES
Command19=ID_PROJECT_EXPORT
Command20=ID_PROJECT_EXPORTPROPERTIES
Command21=ID_PROJECT_SOURCEFILES
Command22=ID_PROJECT_METHODS
Command23=ID_PROJECT_FLAGS
Command24=ID_PROJECT_PRIORITIES
Command25=ID_PROJECT_EDITSTATES
Command26=ID_PROJECT_SETTINGS
Command27=ID_PROJECT_METRICS
Command28=ID_PROJECT_LOCOMOTIONTABLES
Command29=ID_WINDOW_NEW
Command30=ID_WINDOW_CASCADE
Command31=ID_WINDOW_TILE_HORZ
Command32=ID_WINDOW_ARRANGE
Command33=ID_APP_ABOUT
CommandCount=33

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_EDIT_COPY
Command2=ID_EDIT_SEARCH
Command3=ID_TREEITEM_IGNORE
Command4=ID_FILE_NEW
Command5=ID_FILE_OPEN
Command6=ID_FILE_PRINT
Command7=ID_FILE_SAVE
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_UNDO
Command10=ID_EDIT_CUT
Command11=ID_NEXT_PANE
Command12=ID_PREV_PANE
Command13=ID_EDIT_COPY
Command14=ID_EDIT_PASTE
Command15=ID_TREEITEM_EDIT
Command16=ID_EDIT_CUT
Command17=ID_EDIT_UNDO
CommandCount=17

[DLG:IDD_SOURCEFILES]
Type=1
Class=CSourceFilesDlg
ControlCount=5
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_ADD,button,1342242816
Control4=IDC_DELETE,button,1342242816
Control5=IDC_LIST,listbox,1353777419

[CLS:CSourceFilesDlg]
Type=0
HeaderFile=SourceFilesDlg.h
ImplementationFile=SourceFilesDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CSourceFilesDlg

[DLG:IDD_UNASSIGNED]
Type=1
Class=CUnAnimDlg
ControlCount=1
Control1=IDC_LIST,listbox,1353777411

[CLS:CUnAnimDlg]
Type=0
HeaderFile=UnAnimDlg.h
ImplementationFile=UnAnimDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CUnAnimDlg

[TB:IDR_TREEICONS]
Type=1
Class=?
Command1=ID_BUTTON32778
Command2=ID_BUTTON32779
Command3=ID_BUTTON32786
Command4=ID_BUTTON32787
Command5=ID_BUTTON32792
Command6=ID_BUTTON32793
Command7=ID_BUTTON32788
Command8=ID_BUTTON32794
Command9=ID_BUTTON32789
Command10=ID_BUTTON32805
Command11=ID_BUTTON32795
Command12=ID_BUTTON32790
Command13=ID_BUTTON32806
Command14=ID_BUTTON32791
Command15=ID_BUTTON32796
Command16=ID_BUTTON32797
Command17=ID_BUTTON32820
Command18=ID_BUTTON32821
Command19=ID_BUTTON32822
Command20=ID_BUTTON32823
CommandCount=20

[MNU:IDR_MOUSEMENU]
Type=1
Class=CAnimTblEdView
Command1=ID_GROUP_ADDGROUP
Command2=ID_GROUP_ADDANIMATION
Command3=ID_GROUP_ADDANIMATIONS
Command4=ID_GROUP_ADDTRANS
Command5=ID_GROUP_SWAPEQUIV
Command6=ID_EDIT_SEARCH
Command7=ID_GROUP_SCANSOURCECODE
Command8=ID_TREEITEM_IGNORE
Command9=ID_GROUP_EDIT
Command10=ID_GROUP_DELETE
Command11=ID_ANIM_ADDTRANS
Command12=ID_ANIM_SWAPEQUIV
Command13=ID_ANIM_SCANSOURCECODE
Command14=ID_TREEITEM_IGNORE
Command15=ID_ANIM_EDIT
Command16=ID_ANIM_DELETE
Command17=ID_EQUIV_EDIT
Command18=ID_DEPENDANCY_DISABLE
Command19=ID_EQUIV_DELETE
CommandCount=19

[DLG:IDD_GROUPEDIT]
Type=1
Class=CGroupNodeEditDlg
ControlCount=38
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_EDITNAME,edit,1350631552
Control4=IDC_STATIC,static,1342308354
Control5=IDC_STATIC,button,1342177287
Control6=IDC_FLAGSBOX,button,1342177287
Control7=IDC_FLAG0,button,1342242822
Control8=IDC_FLAG2,button,1342242822
Control9=IDC_FLAG3,button,1342242822
Control10=IDC_FLAG4,button,1342242822
Control11=IDC_FLAG5,button,1342242822
Control12=IDC_FLAG6,button,1342242822
Control13=IDC_FLAG7,button,1342242822
Control14=IDC_STATESBOX,button,1342177287
Control15=IDC_FLAGSCROLL,scrollbar,1342177281
Control16=IDC_FLAG1,button,1342242822
Control17=IDC_STATECLASSNAME0,static,1342308352
Control18=IDC_STATECOMBO0,combobox,1344339971
Control19=IDC_STATESCROLL,scrollbar,1342177281
Control20=IDC_STATECLASSNAME1,static,1342308352
Control21=IDC_STATECOMBO1,combobox,1344339971
Control22=IDC_STATECLASSNAME2,static,1342308352
Control23=IDC_STATECOMBO2,combobox,1344339971
Control24=IDC_STATECLASSNAME3,static,1342308352
Control25=IDC_STATECOMBO3,combobox,1344339971
Control26=IDC_STATIC,button,1342177287
Control27=IDC_PRIORITY,combobox,1344339971
Control28=IDC_STATIC_COMMENT,button,1342177287
Control29=IDC_EDIT_COMMENT,edit,1350631620
Control30=IDC_STATIC_NCHILDREN,static,1342308352
Control31=IDC_FLAG8,button,1342242822
Control32=IDC_FLAG9,button,1342242822
Control33=IDC_METHODTITLE,button,1342177287
Control34=IDC_METHOD,combobox,1344339970
Control35=IDC_CUTTABLETITLE,button,1342177287
Control36=IDC_CUTTABLE,combobox,1344339971
Control37=IDC_CALLBACKTITLE,button,1342177287
Control38=IDC_CALLBACK,combobox,1344340226

[CLS:CGroupNodeEditDlg]
Type=0
HeaderFile=GroupNodeEditDlg.h
ImplementationFile=GroupNodeEditDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CGroupNodeEditDlg
VirtualFilter=dWC

[DLG:IDD_ANIMATIONEDIT]
Type=1
Class=CAnimationEditDlg
ControlCount=82
Control1=IDC_ANIMNAME,edit,1350631552
Control2=IDC_CHECK_RIGHTMIRRORED,button,1342242819
Control3=IDC_CHECK_RIGHTREVERSED,button,1342242819
Control4=IDC_BUTTON_RIGHTCHANGEID,button,1342242816
Control5=IDC_BUTTON_SWAP,button,1342251008
Control6=IDC_CHECK_LEFTMIRRORED,button,1342242819
Control7=IDC_CHECK_LEFTREVERSED,button,1342242819
Control8=IDC_BUTTON_LEFTCHANGEID,button,1342242816
Control9=IDC_METHOD,combobox,1344339971
Control10=IDC_PRIORITY,combobox,1344339971
Control11=IDC_FRAMERATE,edit,1350631554
Control12=IDC_LINKTOANIM,button,1342242819
Control13=IDC_LINKRATELIST,combobox,1344340227
Control14=IDC_FLAG0,button,1342242822
Control15=IDC_FLAG1,button,1342242822
Control16=IDC_FLAG2,button,1342242822
Control17=IDC_FLAG3,button,1342242822
Control18=IDC_FLAG4,button,1342242822
Control19=IDC_FLAG5,button,1342242822
Control20=IDC_FLAG6,button,1342242822
Control21=IDC_FLAG7,button,1342242822
Control22=IDC_FLAG8,button,1342242822
Control23=IDC_FLAG9,button,1342242822
Control24=IDC_STATECOMBO0,combobox,1344339971
Control25=IDC_STATECOMBO1,combobox,1344339971
Control26=IDC_STATECOMBO2,combobox,1344339971
Control27=IDC_STATECOMBO3,combobox,1344339971
Control28=IDC_TRANSITIONANIM,button,1342242819
Control29=IDC_TRANSCOMBO0,combobox,1344339971
Control30=IDC_TRANSCOMBO1,combobox,1344339971
Control31=IDC_TRANSCOMBO2,combobox,1344339971
Control32=IDC_TRANSCOMBO3,combobox,1344339971
Control33=IDC_LIST_DEPENDANCIES,listbox,1352728835
Control34=IDC_BUTTON_NEWDEPENDANCY,button,1342242816
Control35=IDC_BUTTON_EDITDEPENDANCY,button,1342242816
Control36=IDC_BUTTON_DELETEDEPENDANCY,button,1342242816
Control37=IDC_BUTTON_DEFAULTPREREQ,button,1342251008
Control38=IDC_CALLBACK,combobox,1344340226
Control39=IDC_SELECTCALLBACK,combobox,1344340226
Control40=IDC_CUTTABLE,combobox,1344339971
Control41=IDC_EDIT_COMMENT,edit,1350631620
Control42=IDOK,button,1342242817
Control43=IDCANCEL,button,1342242816
Control44=IDC_STATIC_BEGINNINGSTATE,static,1342308352
Control45=IDC_PREFIX,static,1342308354
Control46=IDC_STATIC,button,1342177287
Control47=IDC_FLAGSBOX,button,1342177287
Control48=IDC_PREREQTRANSTITLE,button,1342177287
Control49=IDC_STATESBOX,button,1342177287
Control50=IDC_FLAGSCROLL,scrollbar,1342177281
Control51=IDC_STATECLASSNAME0,static,1342308352
Control52=IDC_STATESCROLL,scrollbar,1342177281
Control53=IDC_STATECLASSNAME1,static,1342308352
Control54=IDC_STATECLASSNAME2,static,1342308352
Control55=IDC_STATECLASSNAME3,static,1342308352
Control56=IDC_STATIC,button,1342177287
Control57=IDC_STATIC_COMMENT,button,1342177287
Control58=IDC_FRAMERATEBOX,button,1342177287
Control59=IDC_RIGHTANIMIDAREA,button,1342177287
Control60=IDC_LEFTANIMIDAREA,button,1342177287
Control61=IDC_EDIT_LEFTID,edit,1350633600
Control62=IDC_EDIT_RIGHTID,edit,1350633600
Control63=IDC_METHODTITLE,button,1342177287
Control64=IDC_CUTTABLETITLE,button,1342177287
Control65=IDC_CALLBACKTITLE,button,1342177287
Control66=IDC_SELECTCALLBACKTITLE,button,1342177287
Control67=IDC_CHANGEBOTH,button,1342242816
Control68=IDC_FLAG10,button,1342242819
Control69=IDC_FLAG11,button,1342242819
Control70=IDC_LINKBOX,button,1342177287
Control71=IDC_LINKENTERFRAMETITLE,static,1342308352
Control72=IDC_LINKENTERFRAME,edit,1350639746
Control73=IDC_LINKMETHOD,button,1342242819
Control74=IDC_LINKPRIORITY,button,1342242819
Control75=IDC_LINKRATE,button,1342242819
Control76=IDC_LINKFLAGS,button,1342242819
Control77=IDC_LINKTRANSITIONS,button,1342242819
Control78=IDC_LINKSTATES,button,1342242819
Control79=IDC_RADIO_ANDSEARCH,button,1342177289
Control80=IDC_RADIO_ORSEARCH,button,1342177289
Control81=IDC_STATIC_SEARCH,button,1342177287
Control82=IDC_CHECK_SEARCHGROUPS,button,1342242819

[DLG:IDD_SOURCEID]
Type=1
Class=CSourceIDDlg
ControlCount=10
Control1=IDC_SOURCEIDLIST,listbox,1352728579
Control2=IDC_SOURCEIDLISTMULT,listbox,1352728587
Control3=IDOK,button,1342242817
Control4=IDCANCEL,button,1342242816
Control5=IDC_STATIC,button,1342177287
Control6=IDC_CHECK_ENABLEFILTERS,button,1342254083
Control7=IDC_LIST_FILTER,listbox,1352728835
Control8=IDC_BUTTON_NEWFILTER,button,1342242816
Control9=IDC_BUTTON_DELETEFILTER,button,1476460544
Control10=IDC_DISPLAYTAB,SysTabControl32,1342177280

[CLS:CAnimationEditDlg]
Type=0
HeaderFile=AnimationEditDlg.h
ImplementationFile=AnimationEditDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_FLAG11

[CLS:CSourceIDDlg]
Type=0
HeaderFile=SourceIDDlg.h
ImplementationFile=SourceIDDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CSourceIDDlg

[DLG:IDD_EXPORT]
Type=1
Class=CExportDlg
ControlCount=7
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_BUTTON_NEWEXPORT,button,1342242816
Control4=IDC_LIST_EXPORT,listbox,1352728833
Control5=IDC_BUTTON_EXPORT,button,1342242816
Control6=IDC_BUTTON_EDIT,button,1342242816
Control7=IDC_BUTTON_DELETE,button,1342242816

[CLS:CExportDlg]
Type=0
HeaderFile=ExportDlg.h
ImplementationFile=ExportDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CExportDlg

[DLG:IDD_EQUIVALENCYEDIT]
Type=1
Class=CEquivDlg
ControlCount=13
Control1=IDC_BUTTON_LEFTCHANGEID,button,1342242816
Control2=IDC_BUTTON_RIGHTCHANGEID,button,1342242816
Control3=IDOK,button,1342242817
Control4=IDCANCEL,button,1342242816
Control5=IDC_STATIC,button,1342177287
Control6=IDC_STATIC,button,1342177287
Control7=IDC_CHECK_LEFTMIRRORED,button,1342242819
Control8=IDC_CHECK_LEFTREVERSED,button,1342242819
Control9=IDC_CHECK_RIGHTMIRRORED,button,1342242819
Control10=IDC_CHECK_RIGHTREVERSED,button,1342242819
Control11=IDC_EDIT_LEFTID,edit,1350633600
Control12=IDC_EDIT_RIGHTID,edit,1350633600
Control13=IDC_BUTTON_SWAP,button,1342242816

[CLS:CEquivDlg]
Type=0
HeaderFile=EquivDlg.h
ImplementationFile=EquivDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CEquivDlg

[DLG:IDD_TRANSITIONEDIT]
Type=1
Class=CAnimTransEditDlg
ControlCount=19
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_BUTTON_CHANGEFROM,button,1342242816
Control4=IDC_BUTTON_CHANGETO,button,1342242816
Control5=IDC_BUTTON_CHANGEPLAY,button,1342242816
Control6=IDC_EDIT_FROMANIM,edit,1350633600
Control7=IDC_EDIT_TOANIM,edit,1350633600
Control8=IDC_EDIT_PLAYANIM,edit,1350633600
Control9=IDC_STATIC_PLAY,button,1342177287
Control10=IDC_STATIC,button,1342177287
Control11=IDC_STATIC,button,1342177287
Control12=IDC_STATIC,button,1342177287
Control13=IDC_EDIT_BLENDFRAMES,edit,1350641792
Control14=IDC_SPIN_BLENDFRAMES,msctls_updown32,1342177462
Control15=IDC_STATIC,static,1342308352
Control16=IDC_COMBO_PREREQTRANS,combobox,1344340226
Control17=IDC_STATIC_DESCRIPTION,edit,1351682180
Control18=IDC_STATIC,static,1342308352
Control19=IDC_CHECK_DISABLE,button,1342242819

[CLS:CAnimTransEditDlg]
Type=0
HeaderFile=AnimTransEditDlg.h
ImplementationFile=AnimTransEditDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CAnimTransEditDlg
VirtualFilter=dWC

[DLG:IDD_ANIMID]
Type=1
Class=CAnimIDSelectionDlg
ControlCount=4
Control1=IDC_LIST_ANIMATIONS,listbox,1352728835
Control2=IDOK,button,1342242817
Control3=IDCANCEL,button,1342242816
Control4=IDC_DISPLAYTAB,SysTabControl32,1342177280

[CLS:CAnimIDSelectionDlg]
Type=0
HeaderFile=AnimIDSelectionDlg.h
ImplementationFile=AnimIDSelectionDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_DISPLAYTAB

[DLG:IDD_STATEBITSEDIT]
Type=1
Class=CStateBitsEdit
ControlCount=30
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_NEWSTATECLASS,button,1342242816
Control4=IDC_STATECLASSTAB,SysTabControl32,1342242816
Control5=IDC_DELETESTATECLASS,button,1342242816
Control6=IDC_STATECLASSNAME,edit,1350631560
Control7=IDC_STATELIST,listbox,1352728832
Control8=IDC_ADDSTATE,button,1342242816
Control9=IDC_MODIFYSTATE,button,1342242816
Control10=IDC_DELETESTATE,button,1342242816
Control11=IDC_STATICNAME,static,1342308352
Control12=IDC_STATEDEFINEPREFIX,static,1342308352
Control13=IDC_STATICSTATES,static,1342308352
Control14=IDC_STATICNUMSTATES,static,1342308352
Control15=IDC_STATICSTARTBIT,static,1342308352
Control16=IDC_STATICNUMBITS,static,1342308352
Control17=IDC_NUMSTATES,edit,1350576257
Control18=IDC_STARTBIT,edit,1350576257
Control19=IDC_NUMBITS,edit,1350576257
Control20=IDC_STATIC_,static,1342308352
Control21=IDC_STATIC,static,1342308352
Control22=IDC_REMAININGBITS,edit,1350576257
Control23=IDC_STATIC,static,1342308352
Control24=IDC_TOTALSTATEBITS,edit,1350576257
Control25=IDC_TOTALFLAGBITS,edit,1350576257
Control26=IDC_STATIC,static,1342308352
Control27=IDC_STATIC,static,1342308352
Control28=IDC_TOTALPRIORITYBITS,edit,1350576257
Control29=IDC_TOTALMETHODBITS2,edit,1350576257
Control30=IDC_STATIC,static,1342308352

[DLG:IDD_EDITSTATENAME]
Type=1
Class=CStateName
ControlCount=2
Control1=IDC_STATEDEFINEPREFIX,static,1342308354
Control2=IDC_STATENAME,edit,1350631552

[CLS:CStateBitsEdit]
Type=0
HeaderFile=StateBitsEdit.h
ImplementationFile=StateBitsEdit.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_STATECLASSNAME

[CLS:CStateName]
Type=0
HeaderFile=StateName.h
ImplementationFile=StateName.cpp
BaseClass=CDialog
Filter=D
LastObject=IDOK
VirtualFilter=dWC

[DLG:IDD_FLAGEDIT]
Type=1
Class=FlagsEditDlg
ControlCount=16
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_FLAGLIST,listbox,1352728832
Control4=IDC_ADDFLAG,button,1342242816
Control5=IDC_DELETEFLAG,button,1342242816
Control6=IDC_MODIFYFLAG,button,1342242816
Control7=IDC_STATIC,static,1342308352
Control8=IDC_REMAININGBITS,edit,1350576257
Control9=IDC_STATIC,static,1342308352
Control10=IDC_TOTALSTATEBITS,edit,1350576257
Control11=IDC_TOTALFLAGBITS,edit,1350576257
Control12=IDC_STATIC,static,1342308352
Control13=IDC_STATIC,static,1342308352
Control14=IDC_TOTALPRIORITYBITS,edit,1350576257
Control15=IDC_TOTALMETHODBITS,edit,1350576257
Control16=IDC_STATIC,static,1342308352

[DLG:IDD_PRIORITYEDIT]
Type=1
Class=CPrioritiesEdit
ControlCount=16
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_LIST,listbox,1352728832
Control4=IDC_STATIC,static,1342308352
Control5=IDC_REMAININGBITS,edit,1350576257
Control6=IDC_ADD,button,1342242816
Control7=IDC_DELETE,button,1342242816
Control8=IDC_STATIC,static,1342308352
Control9=IDC_TOTALSTATEBITS,edit,1350576257
Control10=IDC_TOTALFLAGBITS,edit,1350576257
Control11=IDC_STATIC,static,1342308352
Control12=IDC_MODIFY,button,1342242816
Control13=IDC_STATIC,static,1342308352
Control14=IDC_TOTALPRIORITYBITS,edit,1350576257
Control15=IDC_TOTALMETHODBITS,edit,1350576257
Control16=IDC_STATIC,static,1342308352

[CLS:CPrioritiesEdit]
Type=0
HeaderFile=PrioritiesEdit.h
ImplementationFile=PrioritiesEdit.cpp
BaseClass=CDialog
Filter=D
LastObject=CPrioritiesEdit
VirtualFilter=dWC

[DLG:IDD_QUESTION]
Type=1
Class=CQuestionDlg
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_QUESTION,edit,1342179333

[CLS:CQuestionDlg]
Type=0
HeaderFile=QuestionDlg.h
ImplementationFile=QuestionDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CQuestionDlg
VirtualFilter=dWC

[CLS:CStringEdit]
Type=0
HeaderFile=StringEdit.h
ImplementationFile=StringEdit.cpp
BaseClass=CDialog
Filter=D
LastObject=CStringEdit

[DLG:IDD_DIALOG_EDITSTRING]
Type=1
Class=CEditStringDlg
ControlCount=4
Control1=IDC_EDIT_STRING,edit,1350631552
Control2=IDOK,button,1342242817
Control3=IDCANCEL,button,1342242816
Control4=IDC_STATIC_DESCRIPTION,static,1342308865

[CLS:CEditStringDlg]
Type=0
HeaderFile=EditStringDlg.h
ImplementationFile=EditStringDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC

[DLG:IDD_PROJECTSETTINGS]
Type=1
Class=CProjectSettings
ControlCount=6
Control1=IDOK,button,1342242817
Control2=IDC_BASEANIMRATE,edit,1350631552
Control3=IDCANCEL,button,1342242816
Control4=IDC_STATIC,static,1342308352
Control5=IDC_BITFIELDBITS,edit,1350631552
Control6=IDC_STATIC,static,1342308352

[CLS:CProjectSettings]
Type=0
HeaderFile=ProjectSettings.h
ImplementationFile=ProjectSettings.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_BASEANIMRATE

[DLG:IDD_METRICS]
Type=1
Class=CMetricsDlg
ControlCount=17
Control1=IDOK,button,1342242817
Control2=IDC_STATIC,static,1342308352
Control3=IDC_STATIC,static,1342308352
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_NUMANIMS,static,1342308354
Control8=IDC_NUMGROUPS,static,1342308354
Control9=IDC_NUMTRANSITIONS,static,1342308354
Control10=IDC_NUMANIMSWTRANS,static,1342308354
Control11=IDC_SIZEINBYTES,static,1342308354
Control12=IDC_STATIC,static,1342308352
Control13=IDC_STATIC,static,1342308352
Control14=IDC_STATIC,static,1342308352
Control15=IDC_SIZEPERANIM,static,1342308354
Control16=IDC_SIZEPERGROUP,static,1342308354
Control17=IDC_SIZEPERTRANSITION,static,1342308354

[CLS:CMetricsDlg]
Type=0
HeaderFile=MetricsDlg.h
ImplementationFile=MetricsDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CMetricsDlg
VirtualFilter=dWC

[DLG:IDD_EXPORTCONFIG]
Type=1
Class=CExportConfigDlg
ControlCount=10
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_EDIT_PATHFILE,edit,1350633600
Control4=IDC_BUTTON_CHANGEPATH,button,1342242816
Control5=IDC_CHECK_ONOFF,button,1342242819
Control6=IDC_STATIC,static,1342308352
Control7=IDC_EDIT_NAME,edit,1350631552
Control8=IDC_STATIC,static,1342308352
Control9=IDC_CHECK_ALLCAPS,button,1476460547
Control10=IDC_DATASEPARATE,button,1342242819

[CLS:CExportConfigDlg]
Type=0
HeaderFile=ExportConfigDlg.h
ImplementationFile=ExportConfigDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CExportConfigDlg
VirtualFilter=dWC

[DLG:IDD_VIEWPROPERTIES]
Type=1
Class=CViewPropertiesDlg
ControlCount=9
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_VP_OPTIONS,listbox,1352728833
Control4=IDC_VP_DISPLAYED,listbox,1352728833
Control5=IDC_VP_ADD,button,1342242816
Control6=IDC_VP_REMOVE,button,1342242816
Control7=IDC_STATIC,button,1342177287
Control8=IDC_STATIC,static,1342308352
Control9=IDC_STATIC,static,1342308352

[CLS:CViewPropertiesDlg]
Type=0
HeaderFile=ViewPropertiesDlg.h
ImplementationFile=ViewPropertiesDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_VP_DISPLAYED
VirtualFilter=dWC

[DLG:IDD_METHODEDIT]
Type=1
Class=CMethodEditDlg
ControlCount=16
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_LIST,listbox,1352728832
Control4=IDC_STATIC,static,1342308352
Control5=IDC_REMAININGBITS,edit,1350576257
Control6=IDC_ADD,button,1342242816
Control7=IDC_DELETE,button,1342242816
Control8=IDC_STATIC,static,1342308352
Control9=IDC_TOTALSTATEBITS,edit,1350576257
Control10=IDC_TOTALFLAGBITS,edit,1350576257
Control11=IDC_STATIC,static,1342308352
Control12=IDC_MODIFY,button,1342242816
Control13=IDC_STATIC,static,1342308352
Control14=IDC_TOTALPRIORITYBITS,edit,1350576257
Control15=IDC_TOTALMETHODBITS,edit,1350576257
Control16=IDC_STATIC,static,1342308352

[CLS:CMethodEditDlg]
Type=0
HeaderFile=MethodEditDlg.h
ImplementationFile=MethodEditDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_MODIFY
VirtualFilter=dWC

[DLG:IDD_LOCOMOTIONTALES]
Type=1
Class=CLocomotionTablesDlg
ControlCount=63
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_LOCOMOTIONTABLES,listbox,1352728835
Control4=IDC_ADD,button,1342242816
Control5=IDC_COPY,button,1342242816
Control6=IDC_REMOVE,button,1342242816
Control7=IDC_NAME,edit,1350631552
Control8=IDC_PLUS,button,1342242816
Control9=IDC_EQUAL,button,1342242816
Control10=IDC_MINUS,button,1342242816
Control11=IDC_POINTFOREWARD,button,1342177289
Control12=IDC_SIDEFOREWARD,button,1342177289
Control13=IDC_PHASE,button,1342177289
Control14=IDC_PHASESHIFT,edit,1350631552
Control15=IDC_PHASESHIFTSPIN,msctls_updown32,1342177460
Control16=IDC_USEDANIMSTITLE,button,1342177287
Control17=IDC_USEDANIMS,listbox,1350631681
Control18=IDC_MAXNSRTITLE,static,1342308352
Control19=IDC_MAXNSR,edit,1350631552
Control20=IDC_MAXNSRSPIN,msctls_updown32,1342177332
Control21=IDC_VALUE,edit,1350631552
Control22=IDC_VALUESPIN,msctls_updown32,1342177332
Control23=IDC_ORIENTATION,button,1342177287
Control24=IDC_TABLERECT,button,1073741831
Control25=IDC_SELECTEDARCTITLE,static,1342308354
Control26=IDC_SELECTEDARC,static,1342308353
Control27=IDC_SELECTEDZONETITLE,static,1342308354
Control28=IDC_SELECTEDZONE,static,1342308353
Control29=IDC_NUMARCSTITLE,static,1342308353
Control30=IDC_NUMARCS,static,1342308353
Control31=IDC_NUMZONESTITLE,static,1342308353
Control32=IDC_NUMZONES,static,1342308353
Control33=IDC_VALUETITLE,static,1342308352
Control34=IDC_ADDANIM,button,1342242816
Control35=IDC_REMOVEANIM,button,1342242816
Control36=IDC_LINEONLY,static,1342308352
Control37=IDC_AREAONLY,static,1342308352
Control38=IDC_USEDANIMSBAR,scrollbar,1342177281
Control39=IDC_RADIAN,button,1342246915
Control40=IDC_OVERLAP,edit,1350631552
Control41=IDC_OVERLAPSPIN,msctls_updown32,1342177332
Control42=IDC_OVERLAPTITLE,static,1342308352
Control43=IDC_CHANGEANIM,button,1342242816
Control44=IDC_NSRINFO,button,1342177287
Control45=IDC_FIXEDONFIELD,button,1073807363
Control46=IDC_REORIENTANIM,edit,1350633600
Control47=IDC_REORIENTANIMTITLE,button,1342177287
Control48=IDC_REORIENTANIMCHANGE,button,1342242816
Control49=IDC_TABLETYPETITLE,button,1342177287
Control50=IDC_TABLETYPE,combobox,1344339971
Control51=IDC_FACEDIR,button,1073807363
Control52=IDC_LOCOCALLBACKTITLE,button,1342177287
Control53=IDC_CALLBACK,combobox,1344340226
Control54=IDC_OVERLAPARCTITLE,static,1342308352
Control55=IDC_OVERLAPARC,edit,1350631552
Control56=IDC_OVERLAPARCSPIN,msctls_updown32,1342177332
Control57=IDC_LIMITEDNSR,button,1342242819
Control58=IDC_DONOTEXPORT,button,1342242819
Control59=IDC_STARTANGLE,combobox,1344340226
Control60=IDC_STARTANGLETITLE,static,1342308352
Control61=IDC_ENDANGLETITLE,static,1342308352
Control62=IDC_ENDANGLE,combobox,1344340226
Control63=IDC_NAMETITLE,static,1342308352

[CLS:CLocomotionTablesDlg]
Type=0
HeaderFile=LocomotionTablesDlg.h
ImplementationFile=LocomotionTablesDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_ENDANGLE
VirtualFilter=dWC

[CLS:CAnimKeyListBox]
Type=0
HeaderFile=AnimKeyListBox.h
ImplementationFila=AnimKeyListBox.cpp
BaseClass=CListBox
Filter=W

[TB:IDR_STATUSICONS]
Type=1
Class=?
Command1=ID_BUTTON32817
Command2=ID_BUTTON32814
Command3=ID_BUTTON32815
Command4=ID_BUTTON32816
Command5=ID_BUTTON32830
Command6=ID_BUTTON32831
Command7=ID_BUTTON32832
Command8=ID_BUTTON32833
CommandCount=8

[CLS:CScanSourceCodeDlg]
Type=0
HeaderFile=ScanSourceCodeDlg.h
ImplementationFile=ScanSourceCodeDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CScanSourceCodeDlg
VirtualFilter=dWC

[DLG:IDD_SCANSOURCECODE]
Type=1
Class=CScanSourceCodeDlg
ControlCount=17
Control1=IDC_SCAN_FOLDERS,listbox,1353777411
Control2=IDC_SCAN_ADDFOLDER,button,1342242816
Control3=IDC_SCAN_REMOVEFOLDER,button,1342242816
Control4=IDC_SCAN_RECURSIVE,button,1342242819
Control5=IDC_SCAN_COMMENTS,button,1342242819
Control6=IDC_SCAN_NODERECURSE,button,1342242819
Control7=IDOK,button,1342242817
Control8=IDCANCEL,button,1342242816
Control9=IDC_STATIC,button,1342177287
Control10=IDC_ANIM_NAME,edit,1342179456
Control11=IDC_STATIC,static,1342308352
Control12=IDC_NODE_IS_GROUP,button,1476427779
Control13=IDC_STATIC,static,1342308352
Control14=IDC_STATIC,button,1342177287
Control15=IDC_STATIC,button,1342177287
Control16=IDC_NODE_IS_ANIM,button,1476427779
Control17=IDC_STATIC,static,1342308352

[DLG:IDD_SCANRESULTS]
Type=1
Class=CSourceScanResults
ControlCount=14
Control1=IDC_NUMKEYWORDS,edit,1342179456
Control2=IDC_NUMKEYWORDSFOUND,edit,1342179456
Control3=IDC_NUMFILES,edit,1342179456
Control4=IDC_NUMLINES,edit,1342179456
Control5=IDC_SCANRESULTS_LB,listbox,1353777411
Control6=IDC_NOMATCH,listbox,1353777411
Control7=IDOK,button,1342242817
Control8=IDC_STATIC,button,1342177287
Control9=IDC_STATIC,static,1342308352
Control10=IDC_STATIC,static,1342308352
Control11=IDC_STATIC,static,1342308352
Control12=IDC_STATIC,button,1342177287
Control13=IDC_STATIC,static,1342308352
Control14=IDC_STATIC,button,1342177287

[CLS:CSourceScanResults]
Type=0
HeaderFile=SourceScanResults.h
ImplementationFile=SourceScanResults.cpp
BaseClass=CDialog
Filter=D
LastObject=ID_GROUP_SCANSOURCECODE
VirtualFilter=dWC

