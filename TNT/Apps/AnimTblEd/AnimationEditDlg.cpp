// AnimationEditDlg.cpp : implementation file


#include "stdafx.h"
#include "AnimTblEd.h"
#include "AnimationEditDlg.h"
#include "AnimTreeNode.h"
#include "AnimNode.h"
#include "SourceIDDlg.h"
#include "Method.h"
#include "FlagList.h"
#include "Flag.h"
#include "StateClassList.h"
#include "StateClass.h"
#include "State.h"
#include "AnimTblEdGlobals.h"

extern void UpdateScrollBar(CScrollBar *pScrollBar, int nSBCode);

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	FLAG_ID_COUNT	12

#define NOSEARCH_STRING "-no search-"

int FlagIDs[FLAG_ID_COUNT] = {IDC_FLAG0, IDC_FLAG1, IDC_FLAG2, IDC_FLAG3, IDC_FLAG4, IDC_FLAG5, IDC_FLAG6, IDC_FLAG7, IDC_FLAG8, IDC_FLAG9, IDC_FLAG10, IDC_FLAG11};
int StateIDs[4] = {IDC_STATECOMBO0, IDC_STATECOMBO1, IDC_STATECOMBO2, IDC_STATECOMBO3};
int StateTransIDs[4] = {IDC_TRANSCOMBO0, IDC_TRANSCOMBO1, IDC_TRANSCOMBO2, IDC_TRANSCOMBO3};
int StateNameIDs[4] = {IDC_STATECLASSNAME0, IDC_STATECLASSNAME1, IDC_STATECLASSNAME2, IDC_STATECLASSNAME3};


CString	CAnimationEditDlg::s_LastName;
CString	CAnimationEditDlg::s_LastLeftID;
CString	CAnimationEditDlg::s_LastRightID;

/////////////////////////////////////////////////////////////////////////////
// CAnimationEditDlg dialog

CAnimationEditDlg::CAnimationEditDlg(CAnimNode* pAnim, CWnd* pParent /*=NULL*/)
	: CDialog(CAnimationEditDlg::IDD, pParent)
{
	m_AnimName = _T("");

	m_pAnimNode		= pAnim;
	m_pGroupNode	= NULL;
	m_pDoc			= m_pAnimNode->GetDocument();
	m_bSearchMode	= FALSE;
}

CAnimationEditDlg::CAnimationEditDlg(CGroupNode* pGroup, CWnd* pParent /*=NULL*/)
	: CDialog(CAnimationEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAnimationEditDlg)
	m_AnimName = _T("");
	//}}AFX_DATA_INIT

	m_pAnimNode		= NULL;
	m_pGroupNode	= pGroup;
	m_pDoc			= m_pGroupNode->GetDocument();
	m_bSearchMode	= FALSE;
}


void CAnimationEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAnimationEditDlg)
	DDX_Control(pDX, IDC_LIST_DEPENDANCIES, m_DependancyList);
	DDX_Text(pDX, IDC_ANIMNAME, m_AnimName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAnimationEditDlg, CDialog)
	//{{AFX_MSG_MAP(CAnimationEditDlg)
	ON_LBN_DBLCLK(IDC_LIST_DEPENDANCIES, OnDblclkListDependancies)
	ON_BN_CLICKED(IDC_BUTTON_NEWDEPENDANCY, OnButtonNewdependancy)
	ON_BN_CLICKED(IDC_BUTTON_EDITDEPENDANCY, OnButtonEditdependancy)
	ON_BN_CLICKED(IDC_BUTTON_DELETEDEPENDANCY, OnButtonDeletedependancy)
	ON_BN_CLICKED(IDC_BUTTON_DEFAULTPREREQ, OnButtonDefaultprereq)
	ON_LBN_SELCHANGE(IDC_LIST_DEPENDANCIES, OnSelchangeListDependancies)
	ON_LBN_SELCANCEL(IDC_LIST_DEPENDANCIES, OnSelcancelListDependancies)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_FLAG0, OnFlag0)
	ON_BN_CLICKED(IDC_FLAG1, OnFlag1)
	ON_BN_CLICKED(IDC_FLAG2, OnFlag2)
	ON_BN_CLICKED(IDC_FLAG3, OnFlag3)
	ON_BN_CLICKED(IDC_FLAG4, OnFlag4)
	ON_BN_CLICKED(IDC_FLAG5, OnFlag5)
	ON_BN_CLICKED(IDC_FLAG6, OnFlag6)
	ON_BN_CLICKED(IDC_FLAG7, OnFlag7)
	ON_CBN_SELCHANGE(IDC_STATECOMBO0, OnSelchangeStatecombo0)
	ON_CBN_SELCHANGE(IDC_STATECOMBO1, OnSelchangeStatecombo1)
	ON_CBN_SELCHANGE(IDC_STATECOMBO2, OnSelchangeStatecombo2)
	ON_CBN_SELCHANGE(IDC_STATECOMBO3, OnSelchangeStatecombo3)
	ON_BN_CLICKED(IDC_TRANSITIONANIM, OnTransitionanim)
	ON_CBN_SELCHANGE(IDC_TRANSCOMBO0, OnSelchangeTranscombo0)
	ON_CBN_SELCHANGE(IDC_TRANSCOMBO1, OnSelchangeTranscombo1)
	ON_CBN_SELCHANGE(IDC_TRANSCOMBO2, OnSelchangeTranscombo2)
	ON_CBN_SELCHANGE(IDC_TRANSCOMBO3, OnSelchangeTranscombo3)
	ON_BN_CLICKED(IDC_LINKTOANIM, OnLinktoanim)
	ON_EN_KILLFOCUS(IDC_FRAMERATE, OnKillfocusFramerate)
	ON_CBN_SELCHANGE(IDC_LINKRATELIST, OnSelchangeLinkratelist)
	ON_BN_CLICKED(IDC_BUTTON_LEFTCHANGEID, OnButtonLeftchangeid)
	ON_BN_CLICKED(IDC_BUTTON_RIGHTCHANGEID, OnButtonRightchangeid)
	ON_BN_CLICKED(IDC_BUTTON_SWAP, OnButtonSwap)
	ON_CBN_KILLFOCUS(IDC_CALLBACK, OnKillfocusCallback)
	ON_CBN_SETFOCUS(IDC_CALLBACK, OnSetfocusCallback)
	ON_CBN_SELCHANGE(IDC_CALLBACK, OnSelchangeCallback)
	ON_CBN_SELCHANGE(IDC_SELECTCALLBACK, OnSelchangeSelectcallback)
	ON_CBN_SETFOCUS(IDC_SELECTCALLBACK, OnSetfocusSelectcallback)
	ON_CBN_KILLFOCUS(IDC_SELECTCALLBACK, OnKillfocusSelectcallback)
	ON_BN_CLICKED(IDC_CHANGEBOTH, OnChangeboth)
	ON_EN_CHANGE(IDC_LINKENTERFRAME, OnChangeLinkenterframe)
	ON_BN_CLICKED(IDC_LINKMETHOD, OnLinkmethod)
	ON_BN_CLICKED(IDC_LINKPRIORITY, OnLinkpriority)
	ON_BN_CLICKED(IDC_LINKRATE, OnLinkrate)
	ON_BN_CLICKED(IDC_LINKTRANSITIONS, OnLinktransitions)
	ON_BN_CLICKED(IDC_FLAG8, OnFlag8)
	ON_BN_CLICKED(IDC_FLAG9, OnFlag9)
	ON_BN_CLICKED(IDC_FLAG10, OnFlag10)
	ON_BN_CLICKED(IDC_FLAG11, OnFlag11)
	ON_BN_CLICKED(IDC_LINKFLAGS, OnLinkflags)
	ON_BN_CLICKED(IDC_LINKSTATES, OnLinkstates)
	ON_EN_KILLFOCUS(IDC_LINKENTERFRAME, OnKillfocusLinkenterframe)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnimationEditDlg message handlers

BOOL CAnimationEditDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if (m_bSearchMode)
		InitDialogForSearch();
	else if (m_pAnimNode)
		InitDialogForAnimNode();
	else
		InitDialogForGroupNode();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CAnimationEditDlg::InitDialogForSearch()
{
	m_pIDList	= &m_pDoc->m_IDList;
	m_pAnimList	= &m_pDoc->m_AnimList;

	//---	Default Search is AND.
	((CButton*)GetDlgItem( IDC_RADIO_ANDSEARCH ))->SetCheck( TRUE );
	((CButton*)GetDlgItem( IDC_CHECK_SEARCHGROUPS ))->SetCheck( TRUE );

	//---	Init the Right/Left Source ID data.
	InitAnimIDs( TRUE );

	//---	get cut tables
	m_pCutTable			= m_pAnimNode->GetCutTable();
	m_pParentCutTable	= m_pAnimNode->GetParentCutTable();

	//---	get the callbacks
	m_Callback			= m_pAnimNode->GetCallback();
	m_ParentCallback	= m_pAnimNode->GetParentCallback();

	//---	get the node bitset data
	m_Method			= m_pAnimNode->BS_GetMethod();
	m_Flags				= m_pAnimNode->BS_GetFlags();
	m_States			= m_pAnimNode->BS_GetStates();
	m_EndStates			= m_pAnimNode->BS_GetEndStates();
	m_Priority			= m_pAnimNode->BS_GetPriority();
	m_GroupMethod		= m_pAnimNode->BS_GetGroupMethod();
	m_GroupFlags		= m_pAnimNode->BS_GetGroupFlags();
	m_GroupStates		= m_pAnimNode->BS_GetGroupStates();
	m_GroupPriority		= m_pAnimNode->BS_GetGroupPriority();

	//---	link anim data
	m_bLinkCheck		= m_pAnimNode->m_LinkAnim.GetLength() ? TRUE : FALSE;
	m_bLinkMethod		= m_pAnimNode->m_bLinkMethod;
	m_bLinkPriority		= m_pAnimNode->m_bLinkPriority;
	m_bLinkFrameRate	= m_pAnimNode->m_bLinkFrameRate;
	m_bLinkFlags		= m_pAnimNode->m_bLinkFlags;
	m_bLinkStates		= m_pAnimNode->m_bLinkStates;
	m_bLinkTransitions	= m_pAnimNode->m_bLinkTransitions;
	m_LinkEnterFrame	= m_pAnimNode->m_LinkEnterFrame;

	InitLinkAnimDialog( TRUE );
	UpdateMethodDialog( TRUE );
	InitFlagsDialog( TRUE );
	InitStatesDialog( TRUE );
	UpdatePriorityDialog( TRUE );
	InitTransitionDialog( TRUE );
	InitFrameRateDialog( TRUE );
	InitCallbackDialog( TRUE );
	InitSelectCallbackDialog( TRUE );
	InitCutTableDialog( TRUE );

	//---	These Items are not searchable.
	// Dependancies.
	GetDlgItem( IDC_PREREQTRANSTITLE )->EnableWindow( FALSE );
	GetDlgItem( IDC_LIST_DEPENDANCIES )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_NEWDEPENDANCY )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_EDITDEPENDANCY )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_DELETEDEPENDANCY )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_DEFAULTPREREQ )->EnableWindow( FALSE );
	// Comment
	GetDlgItem( IDC_STATIC_COMMENT )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_COMMENT )->EnableWindow( FALSE );
}


void CAnimationEditDlg::InitDialogForAnimNode( void )
{
	m_pIDList	= &m_pDoc->m_IDList;
	m_pAnimList	= &m_pDoc->m_AnimList;

	//---	Hide Search-Specific Controls
	GetDlgItem( IDC_STATIC_SEARCH )->ShowWindow( SW_HIDE );
	GetDlgItem( IDC_RADIO_ANDSEARCH )->ShowWindow( SW_HIDE );
	GetDlgItem( IDC_RADIO_ORSEARCH )->ShowWindow( SW_HIDE );
	GetDlgItem( IDC_CHECK_SEARCHGROUPS )->ShowWindow( SW_HIDE );

    //---   retrieve current name
    m_Name = m_pAnimNode->GetName();

	//---	if no name yet, default to the last name
	if((GetLastName() != "") && (( m_Name == "" ) || ( m_Name == "ANIMATION" )))
		m_pAnimNode->SetName( GetLastName() );

	//---	if no Left ID yet, default to the last Left ID
	if( m_pAnimNode->m_RLIDs.GetLeftID() == "" )
		m_pAnimNode->m_RLIDs.SetLeftID( GetLastLeftID(), m_pIDList );

	//---	if no Right ID yet, default to the last Right ID
	if( m_pAnimNode->m_RLIDs.GetRightID() == "" )
		m_pAnimNode->m_RLIDs.SetRightID( GetLastRightID(), m_pIDList );

	SetDlgItemText( IDC_PREFIX, "ANIM_" );
	m_Name = m_pAnimNode->GetName();

	SetDlgItemText( IDC_ANIMNAME, m_Name );
	GetDlgItem( IDC_ANIMNAME )->SetFocus();

	//---	Init the Right/Left Source ID data.
	InitAnimIDs();

	//---	Fill the comment edit box.
	SetDlgItemText( IDC_EDIT_COMMENT, m_pAnimNode->m_Comment );

	//---	get cut tables
	m_pCutTable			= m_pAnimNode->GetCutTable();
	m_pParentCutTable	= m_pAnimNode->GetParentCutTable();

	//---	get the callbacks
	m_Callback			= m_pAnimNode->GetCallback();
	m_ParentCallback	= m_pAnimNode->GetParentCallback();

	//---	get the node bitset data
	m_Method			= m_pAnimNode->BS_GetMethod();
	m_Flags				= m_pAnimNode->BS_GetFlags();
	m_States			= m_pAnimNode->BS_GetStates();
	m_EndStates			= m_pAnimNode->BS_GetEndStates();
	m_Priority			= m_pAnimNode->BS_GetPriority();
	m_GroupMethod		= m_pAnimNode->BS_GetGroupMethod();
	m_GroupFlags		= m_pAnimNode->BS_GetGroupFlags();
	m_GroupStates		= m_pAnimNode->BS_GetGroupStates();
	m_GroupPriority		= m_pAnimNode->BS_GetGroupPriority();

	//---	link anim data
	m_bLinkCheck		= m_pAnimNode->m_LinkAnim.GetLength() ? TRUE : FALSE;
	m_bLinkMethod		= m_pAnimNode->m_bLinkMethod;
	m_bLinkPriority		= m_pAnimNode->m_bLinkPriority;
	m_bLinkFrameRate	= m_pAnimNode->m_bLinkFrameRate;
	m_bLinkFlags		= m_pAnimNode->m_bLinkFlags;
	m_bLinkStates		= m_pAnimNode->m_bLinkStates;
	m_bLinkTransitions	= m_pAnimNode->m_bLinkTransitions;
	m_LinkEnterFrame	= m_pAnimNode->m_LinkEnterFrame;

	InitLinkAnimDialog();
	UpdateMethodDialog();
	InitFlagsDialog();
	InitStatesDialog();
	UpdatePriorityDialog();
	InitTransitionDialog();
	InitFrameRateDialog();
	InitCallbackDialog();
	InitSelectCallbackDialog();
	InitCutTableDialog();

	//---	load the dependancies from the anim tree
	m_pAnimNode->GetDependancies( m_AnimTransList );

	FillDependancyList();
}


void CAnimationEditDlg::InitDialogForGroupNode( void )
{
	m_pIDList	= &m_pDoc->m_IDList;
	m_pAnimList	= &m_pDoc->m_AnimList;

	//---	Hide Search-Specific Controls
	GetDlgItem( IDC_STATIC_SEARCH )->ShowWindow( SW_HIDE );
	GetDlgItem( IDC_RADIO_ANDSEARCH )->ShowWindow( SW_HIDE );
	GetDlgItem( IDC_RADIO_ORSEARCH )->ShowWindow( SW_HIDE );
	GetDlgItem( IDC_CHECK_SEARCHGROUPS )->ShowWindow( SW_HIDE );

    //---   retrieve current name
    m_Name = m_pGroupNode->GetName();

	//---	if no name yet, default to the last name
	if((GetLastName() != "") && (( m_Name == "" ) || ( m_Name == "GROUP" )))
		m_pGroupNode->SetName( GetLastName() );

	SetDlgItemText( IDC_PREFIX, "ANIMGROUP_" );
	m_Name = m_pGroupNode->GetName();

	SetDlgItemText( IDC_ANIMNAME, m_Name );
	GetDlgItem( IDC_ANIMNAME )->SetFocus();

	//---	Init the Right/Left Source ID data.
	InitAnimIDs();

	//---	Fill the comment edit box.
	SetDlgItemText( IDC_EDIT_COMMENT, m_pGroupNode->m_Comment );

	//---	get cut tables
	m_pCutTable			= m_pGroupNode->GetCutTable();
	m_pParentCutTable	= m_pGroupNode->GetParentCutTable();

	//---	get the callbacks
	m_Callback			= m_pGroupNode->GetCallback();
	m_ParentCallback	= m_pGroupNode->GetParentCallback();

	//---	get the callbacks
	m_SelectCallback		= m_pGroupNode->GetSelectCallback();
	m_ParentSelectCallback	= m_pGroupNode->GetParentSelectCallback();

	//---	get the node bitset data
	m_Method			= m_pGroupNode->BS_GetMethod();
	m_Flags				= m_pGroupNode->BS_GetFlags();
	m_States			= m_pGroupNode->BS_GetStates();
	m_EndStates			= 0;
	m_Priority			= m_pGroupNode->BS_GetPriority();
	m_GroupMethod		= m_pGroupNode->BS_GetGroupMethod();
	m_GroupFlags		= m_pGroupNode->BS_GetGroupFlags();
	m_GroupStates		= m_pGroupNode->BS_GetGroupStates();
	m_GroupPriority		= m_pGroupNode->BS_GetGroupPriority();

	InitLinkAnimDialog();
	UpdateMethodDialog();
	InitFlagsDialog();
	InitStatesDialog();
	UpdatePriorityDialog();
	InitTransitionDialog();
	InitFrameRateDialog();
	InitCallbackDialog();
	InitSelectCallbackDialog();
	InitCutTableDialog();

	//---	load the dependancies from the anim tree
	m_pGroupNode->GetDependancies( m_AnimTransList );

	FillDependancyList();
}


void CAnimationEditDlg::OnOK() 
{
	OnKillfocusCallback();
	OnKillfocusSelectcallback();

	if (m_bSearchMode)
		OnOKForSearchMode();
	else if (m_pAnimNode != NULL)
		OnOKForAnimNode();
	else
		OnOKForGroupNode();
}


void CAnimationEditDlg::OnOKForSearchMode()
{
	CString Name, LeftID, RightID, String;
	int		nSel;

	ASSERT(m_pAnimNode);

	//---	Get the search mode.
	if (((CButton*)GetDlgItem( IDC_RADIO_ANDSEARCH ))->GetCheck())
		m_NodeSearchMap.SetANDSearch();
	else
		m_NodeSearchMap.SetORSearch();

	//---	Will we include groups in the search?
	m_NodeSearchMap.SetSearchGroups(((CButton*)GetDlgItem( IDC_CHECK_SEARCHGROUPS ))->GetCheck() ? TRUE : FALSE);

	//---	Get the entered Name
	this->GetDlgItemText( IDC_ANIMNAME, Name );
	Name.TrimLeft();
	Name.TrimRight();

	//---	If the Name search field was entered then initialize a Name search.
	if (Name.GetLength() != 0)
		m_NodeSearchMap.SetSearchName( Name );

	//---	Get the entered SearchIDs
	GetDlgItemText( IDC_EDIT_LEFTID, LeftID );
	GetDlgItemText( IDC_EDIT_RIGHTID, RightID );

	//---	If LeftID Search was entered then initialize a LeftID search.
	if (LeftID.GetLength() != 0)
		m_NodeSearchMap.SetSearchLeftID( LeftID );

	//---	If RightID Search was entered then initialize a RightID search.
	if (RightID.GetLength() != 0)
		m_NodeSearchMap.SetSearchRightID( RightID );

	//---	Set Search for Flags
	if (m_Flags)
		m_NodeSearchMap.SetSearchFlags ( m_Flags );

	//---	Set Search for Method
	nSel = ((CComboBox*) GetDlgItem(IDC_METHOD))->GetCurSel();
	if (nSel != 0)
		m_NodeSearchMap.SetSearchMethod( nSel - 1 );

	//---	Set Search for Priority
	nSel = ((CComboBox*) GetDlgItem(IDC_PRIORITY))->GetCurSel();
	if (nSel != 0)
		m_NodeSearchMap.SetSearchPriority( nSel - 1 );

	//---	Set Search for Callback
	nSel = ((CComboBox*) GetDlgItem(IDC_CALLBACK))->GetCurSel();
	if (nSel != 0)
	{
		GetDlgItemText( IDC_CALLBACK, String );
		m_NodeSearchMap.SetSearchCallback( String );
	}

	//---	SetSearch for SelectCallback
	nSel = ((CComboBox*) GetDlgItem(IDC_SELECTCALLBACK))->GetCurSel();
	if (nSel != 0)
	{
		GetDlgItemText( IDC_SELECTCALLBACK, String );
		m_NodeSearchMap.SetSearchSelectCallback( String );
	}

	//---	Set Search for CutTable
	nSel = ((CComboBox*) GetDlgItem(IDC_CUTTABLE))->GetCurSel();
	if (nSel != 0)
	{
		GetDlgItemText( IDC_CUTTABLE, String );
		m_NodeSearchMap.SetSearchCutTable( String );
	}

	CDialog::OnOK();
}


void CAnimationEditDlg::OnOKForAnimNode() 
{
	CString	Name, LeftID, RightID;
	CButton	*pCheckBox;
	CComboBox* pComboBox;

	ASSERT(m_pAnimNode);

	//---	set the frame rate box value now
	OnKillfocusFramerate();

	//---	Get the new name from the dialog.
	this->GetDlgItemText( IDC_ANIMNAME, Name );
	Name.TrimLeft();
	Name.TrimRight();
	if (Name.GetLength() == 0)
	{
		this->MessageBox( "The animation must have a name.", "Error", MB_ICONERROR );
		return;
	}

	//---	get the IDs from the dialog
	GetDlgItemText( IDC_EDIT_RIGHTID, RightID );
	GetDlgItemText( IDC_EDIT_LEFTID, LeftID );
	if ((LeftID.GetLength() == 0) || (RightID.GetLength() == 0))
	{
		this->MessageBox( "The equivalency must have a left and right ID.", "Error", MB_ICONERROR );
		return;
	}

	//---	Remove the animation from the list of existing animations.
	if( !m_pAnimNode->GetIgnore() )
		m_pAnimList->RemoveDec( m_pAnimNode->GetName() );

	//---	Set the new name.
	m_pAnimNode->SetName( Name );

	//---	Add the new animation to the list of existing animations.
	if( !m_pAnimNode->GetIgnore() )
		m_pAnimList->AddInc( m_pAnimNode->GetName() );

	//---	Fill the comment edit box.
	this->GetDlgItemText( IDC_EDIT_COMMENT, m_pAnimNode->m_Comment );
	m_pAnimNode->m_Comment.TrimLeft();
	m_pAnimNode->m_Comment.TrimRight();

	//---	set the cut table
	pComboBox = (CComboBox*) GetDlgItem( IDC_CUTTABLE );
	m_pAnimNode->SetCutTable( (CLocoAnimTable*)pComboBox->GetItemData(pComboBox->GetCurSel()) );

	//---	set the callback function
	CString NewCallback;
	pComboBox = (CComboBox*) GetDlgItemText( IDC_CALLBACK, NewCallback );
	CString Prefix = NewCallback.Left( 6 );
	if (Prefix == "<def> ")
		NewCallback = NewCallback.Right( NewCallback.GetLength() - 6 );
	NewCallback.TrimLeft();
	NewCallback.TrimRight();
	m_pAnimNode->SetCallback( NewCallback );

	//---	Set the node bitset data
	m_pAnimNode->BS_SetMethod(((CComboBox*) GetDlgItem(IDC_METHOD))->GetCurSel());
	m_pAnimNode->BS_SetFlags(m_Flags);
	m_pAnimNode->BS_SetPriority(((CComboBox*) GetDlgItem(IDC_PRIORITY))->GetCurSel());
	m_pAnimNode->BS_SetStates(m_States);
	m_pAnimNode->BS_SetEndStates(m_EndStates);
	m_pAnimNode->m_FrameRate = m_FrameRate;

	//---	set the anim IDs
	m_pAnimNode->m_RLIDs.SetLeftID( LeftID, m_pIDList );
	m_pAnimNode->m_RLIDs.SetRightID( RightID, m_pIDList );

	//---	set the Left and Right IDs flag settings.
	pCheckBox = (CButton*)GetDlgItem( IDC_CHECK_LEFTMIRRORED );
	m_pAnimNode->m_RLIDs.SetLeftMirrored( pCheckBox->GetCheck() ? TRUE : FALSE );

	pCheckBox = (CButton*)GetDlgItem( IDC_CHECK_LEFTREVERSED );
	m_pAnimNode->m_RLIDs.SetLeftReversed( pCheckBox->GetCheck() ? TRUE : FALSE );

	pCheckBox = (CButton*)GetDlgItem( IDC_CHECK_RIGHTMIRRORED );
	m_pAnimNode->m_RLIDs.SetRightMirrored( pCheckBox->GetCheck() ? TRUE : FALSE );

	pCheckBox = (CButton*)GetDlgItem( IDC_CHECK_RIGHTREVERSED );
	m_pAnimNode->m_RLIDs.SetRightReversed( pCheckBox->GetCheck() ? TRUE : FALSE );

	//---	Set the dependancies back into the tree
	m_pAnimNode->SetDependancies( m_AnimTransList );

	//---	Mark the document as having changed.
	GBL_SetModifiedFlag( TRUE );

	//---	Get the new default name
	SetLastName( Name );

	//---	get the new default Left anim ID
	SetLastLeftID( m_pAnimNode->m_RLIDs.GetLeftID() );

	//---	get the new default right anim ID
	SetLastRightID( m_pAnimNode->m_RLIDs.GetRightID() );

	//---	set link information
	if( m_bLinkCheck )
		m_pAnimNode->m_LinkAnim		= m_LinkAnim;
	else
		m_pAnimNode->m_LinkAnim		= "";
	m_pAnimNode->m_bLinkMethod		= m_bLinkMethod;
	m_pAnimNode->m_bLinkPriority	= m_bLinkPriority;
	m_pAnimNode->m_bLinkFrameRate	= m_bLinkFrameRate;
	m_pAnimNode->m_bLinkStates		= m_bLinkStates;
	m_pAnimNode->m_bLinkFlags		= m_bLinkFlags;
	m_pAnimNode->m_LinkEnterFrame	= m_LinkEnterFrame;

	CDialog::OnOK();
}


void CAnimationEditDlg::OnOKForGroupNode() 
{
	CString	Name, LeftID, RightID;
	CComboBox* pComboBox;

	ASSERT(m_pGroupNode);

	//---	Get the new name from the dialog.
	this->GetDlgItemText( IDC_ANIMNAME, Name );
	Name.TrimLeft();
	Name.TrimRight();
	if (Name.GetLength() == 0)
	{
		this->MessageBox( "The group must have a name.", "Error", MB_ICONERROR );
		return;
	}

	//---	Remove the animation from the list of existing animations.
	if( !m_pGroupNode->GetIgnore() )
		m_pAnimList->RemoveDec( m_pGroupNode->GetName() );

	//---	Set the new name.
	m_pGroupNode->SetName( Name );

	//---	Add the new animation to the list of existing animations.
	if( !m_pGroupNode->GetIgnore() )
		m_pAnimList->AddInc( m_pGroupNode->GetName() );

	//---	Fill the comment edit box.
	this->GetDlgItemText( IDC_EDIT_COMMENT, m_pGroupNode->m_Comment );
	m_pGroupNode->m_Comment.TrimLeft();
	m_pGroupNode->m_Comment.TrimRight();

	//---	set the cut table
	pComboBox = (CComboBox*) GetDlgItem( IDC_CUTTABLE );
	m_pGroupNode->SetCutTable( (CLocoAnimTable*)pComboBox->GetItemData(pComboBox->GetCurSel()) );

	//---	set the callback function
	CString NewCallback;
	pComboBox = (CComboBox*) GetDlgItemText( IDC_CALLBACK, NewCallback );
	CString Prefix = NewCallback.Left( 6 );
	if (Prefix == "<def> ")
		NewCallback = NewCallback.Right( NewCallback.GetLength() - 6 );
	NewCallback.TrimLeft();
	NewCallback.TrimRight();
	m_pGroupNode->SetCallback( NewCallback );

	//---	set the select callback function
	pComboBox = (CComboBox*) GetDlgItemText( IDC_SELECTCALLBACK, NewCallback );
	Prefix = NewCallback.Left( 6 );
	if (Prefix == "<def> ")
		NewCallback = NewCallback.Right( NewCallback.GetLength() - 6 );
	NewCallback.TrimLeft();
	NewCallback.TrimRight();
	m_pGroupNode->SetSelectCallback( NewCallback );

	//---	Set the node bitset data
	m_pGroupNode->BS_SetMethod(((CComboBox*) GetDlgItem(IDC_METHOD))->GetCurSel());
	m_pGroupNode->BS_SetFlags(m_Flags);
	m_pGroupNode->BS_SetPriority(((CComboBox*) GetDlgItem(IDC_PRIORITY))->GetCurSel());
	m_pGroupNode->BS_SetStates(m_States);

	//---	Set the dependancies back into the tree
	m_pGroupNode->SetDependancies( m_AnimTransList );

	//---	Mark the document as having changed.
	GBL_SetModifiedFlag( TRUE );

	//---	Get the new default name
	SetLastName( Name );

	CDialog::OnOK();
}

void CAnimationEditDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void CAnimationEditDlg::InitAnimIDs(bool SearchMode)
{
	CButton		*pCheckBox;

	if (m_pAnimNode == NULL)
	{
		GetDlgItem( IDC_EDIT_LEFTID )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_RIGHTID )->EnableWindow( FALSE );
		GetDlgItem( IDC_CHANGEBOTH )->EnableWindow( FALSE );
		GetDlgItem( IDC_CHECK_LEFTMIRRORED )->EnableWindow( FALSE );
		GetDlgItem( IDC_CHECK_LEFTREVERSED )->EnableWindow( FALSE );
		GetDlgItem( IDC_CHECK_RIGHTMIRRORED )->EnableWindow( FALSE );
		GetDlgItem( IDC_CHECK_RIGHTREVERSED )->EnableWindow( FALSE );
		GetDlgItem( IDC_BUTTON_RIGHTCHANGEID )->EnableWindow( FALSE );
		GetDlgItem( IDC_BUTTON_LEFTCHANGEID )->EnableWindow( FALSE );
		GetDlgItem( IDC_LEFTANIMIDAREA )->EnableWindow( FALSE );
		GetDlgItem( IDC_RIGHTANIMIDAREA )->EnableWindow( FALSE );
		GetDlgItem( IDC_BUTTON_SWAP )->EnableWindow( FALSE );
		return;
	}

	if (SearchMode)
	{
		//---	Allow direct text editing when searching.
		((CEdit*)GetDlgItem( IDC_EDIT_LEFTID ))->SetReadOnly( FALSE );
		((CEdit*)GetDlgItem( IDC_EDIT_RIGHTID ))->SetReadOnly( FALSE );

		GetDlgItem( IDC_CHECK_LEFTMIRRORED )->EnableWindow( FALSE );
		GetDlgItem( IDC_CHECK_LEFTREVERSED )->EnableWindow( FALSE );
		GetDlgItem( IDC_CHECK_RIGHTMIRRORED )->EnableWindow( FALSE );
		GetDlgItem( IDC_CHECK_RIGHTREVERSED )->EnableWindow( FALSE );
		return;
	}
	
	//---	Assign the left ID name.
	SetDlgItemText( IDC_EDIT_LEFTID, m_pAnimNode->m_RLIDs.GetLeftID() );

	//---	Mark the left ID as mirrored.
	pCheckBox = (CButton*)GetDlgItem( IDC_CHECK_LEFTMIRRORED );
	pCheckBox->SetCheck( m_pAnimNode->m_RLIDs.GetLeftMirrored() );

	//---	Mark the left ID as reversed.
	pCheckBox = (CButton*)GetDlgItem( IDC_CHECK_LEFTREVERSED );
	pCheckBox->SetCheck( m_pAnimNode->m_RLIDs.GetLeftReversed() );

	//---	Assign the right ID name.
	SetDlgItemText( IDC_EDIT_RIGHTID, m_pAnimNode->m_RLIDs.GetRightID() );

	//---	Mark the right ID as mirrored.
	pCheckBox = (CButton*)GetDlgItem( IDC_CHECK_RIGHTMIRRORED );
	pCheckBox->SetCheck( m_pAnimNode->m_RLIDs.GetRightMirrored() );

	//---	Mark the right ID as reversed.
	pCheckBox = (CButton*)GetDlgItem( IDC_CHECK_RIGHTREVERSED );
	pCheckBox->SetCheck( m_pAnimNode->m_RLIDs.GetRightReversed() );
}


void CAnimationEditDlg::InitCallbackDialog(bool SearchMode)
{
	CComboBox*		pComboBox = (CComboBox*) GetDlgItem(IDC_CALLBACK);
	CStringArray	Callbacks;
	CString			ParentString;
	CString			String;
	int				i;

	m_pDoc->m_AnimTree.FillCallbackStringArray( Callbacks );

	if (SearchMode)
		pComboBox->AddString( NOSEARCH_STRING );
	
	//---	load the strings into the combo box, mark the default, and select the current one
	for( i=0; i<Callbacks.GetSize(); i++ )
	{
		//---	set the parent item
		if (SearchMode)
			String = Callbacks[i];
		else if (m_ParentCallback == Callbacks[i])
			String = "<def> " + Callbacks[i];
		else
			String = "          " + Callbacks[i];

		pComboBox->AddString( String );

		//---	if this is the selected item, select it now
		if (m_Callback == Callbacks[i])
			pComboBox->SetCurSel( pComboBox->FindString( -1, String ) );
	}

	if (SearchMode)
		pComboBox->SetCurSel( pComboBox->FindStringExact( -1, NOSEARCH_STRING ) );
	else if ((m_Callback == "") && (m_ParentCallback == ""))
		SetDlgItemText( IDC_CALLBACK, "<def> " );
}


void CAnimationEditDlg::InitSelectCallbackDialog(bool SearchMode)
{
	CComboBox*		pComboBox = (CComboBox*) GetDlgItem(IDC_SELECTCALLBACK);
	CStringArray	Callbacks;
	CString			ParentString;
	CString			String;
	int				i;

	if (!m_pGroupNode && !SearchMode)
	{
		GetDlgItem( IDC_SELECTCALLBACK )->EnableWindow( FALSE );
		GetDlgItem( IDC_SELECTCALLBACKTITLE )->EnableWindow( FALSE );
		return;
	}

	m_pDoc->m_AnimTree.FillSelectCallbackStringArray( Callbacks );
	
	if (SearchMode)
		pComboBox->AddString( NOSEARCH_STRING );

	//---	load the strings into the combo box, mark the default, and select the current one
	for( i=0; i<Callbacks.GetSize(); i++ )
	{
		//---	set the parent item
//---	NO NEED TO SHOW DEFAULT, NO DEFAULT IMPLEMENTED
//		if (m_ParentSelectCallback == Callbacks[i])
//			String = "<def> " + Callbacks[i];
//		else
//			String = "          " + Callbacks[i];
		String = Callbacks[i];

		pComboBox->AddString( String );

		//---	if this is the selected item, select it now
		if (m_SelectCallback == Callbacks[i])
			pComboBox->SetCurSel( pComboBox->FindString( -1, String ) );
	}

//---	NO NEED TO SHOW DEFAULT, NO DEFAULT IMPLEMENTED
//	if ((m_SelectCallback == "") && (m_ParentSelectCallback == ""))
//		SetDlgItemText( IDC_SELECTCALLBACK, "<def> " );

	if (SearchMode)
		pComboBox->SetCurSel( pComboBox->FindStringExact( -1, NOSEARCH_STRING ) );
}

void CAnimationEditDlg::InitCutTableDialog(bool SearchMode)
{
	CLocoAnimTable* pTable;
	CComboBox*		pComboBox = (CComboBox*) GetDlgItem(IDC_CUTTABLE);
	s32				index, i;
	CString			Table;
	CString			NoneString;

	//---	put a 'none' selection at the top
	pComboBox->ResetContent();

	if (SearchMode)
		NoneString = NOSEARCH_STRING;
	else if (m_pParentCutTable == NULL)
		NoneString = "<def> <none>";
	else
		NoneString = "      <none>";

	pComboBox->AddString( NoneString );
	pComboBox->SetItemData( pComboBox->FindStringExact( -1, NoneString ), NULL );

	//---	loop through each of the locomotion tables and place ones set as cut tables into the list of cut tables.
	for( i=0; i<m_pDoc->m_LocomotionTables.GetCount(); i++ )
	{
		pTable = m_pDoc->m_LocomotionTables.GetTable( i );

		//---	if this table is a cut table, add it to the list box
		if (pTable->m_Type == TABLETYPE_CUT)
		{
			if (SearchMode)
				Table = pTable->m_Name;
			else if ((m_pParentCutTable != NULL) && (pTable->m_Name == m_pParentCutTable->m_Name))
				Table = "<def> " + pTable->m_Name;
			else
				Table = "      " + pTable->m_Name;

			pComboBox->AddString( Table );
			index = pComboBox->FindStringExact( -1, Table );
			pComboBox->SetItemData( index, (u32)pTable );

			//---	if this is the currently selected table, select this item
			if (m_pCutTable == pTable)
				pComboBox->SetCurSel( index );
		}
	}

	if (SearchMode)
		pComboBox->SetCurSel( pComboBox->FindStringExact( -1, NOSEARCH_STRING ) );
	else if (m_pCutTable == NULL)
		pComboBox->SetCurSel( pComboBox->FindStringExact( -1, NoneString ) );
}

void CAnimationEditDlg::InitFlagsDialog(bool SearchMode)
{
	int c;

	//---	set the flag scroll bar range
	m_FlagsScrollIndex = 0;
	c = m_pDoc->m_FlagList.GetNumFlags();
	if (c > FLAG_ID_COUNT)
	{
		((CScrollBar*) GetDlgItem(IDC_FLAGSCROLL))->SetScrollRange(0, c - FLAG_ID_COUNT);
		((CScrollBar*) GetDlgItem(IDC_FLAGSCROLL))->ShowScrollBar(TRUE);
	}
	else
		((CScrollBar*) GetDlgItem(IDC_FLAGSCROLL))->ShowScrollBar(FALSE);

	UpdateFlagsDialog();
}

void CAnimationEditDlg::UpdateFlagsDialog()
{
	int i;
	CFlag *pFlag;
	CButton *pButton;
	int Flags;
	BOOL DisableFlags;
	
	if( m_bLinkCheck && m_bLinkFlags && m_pAnimNode )
	{
		DisableFlags = TRUE;
		if( m_pLinkAnim )
			Flags = m_pLinkAnim->BS_GetFlags()|m_pLinkAnim->BS_GetGroupFlags();
	}
	else
	{
		DisableFlags = FALSE;
		Flags = m_Flags;
	}

	for (i=0; i<FLAG_ID_COUNT; i++)
	{
		pButton = (CButton*) GetDlgItem(FlagIDs[i]);
		pFlag = m_pDoc->m_FlagList.GetFlag(i + m_FlagsScrollIndex);

		if (pFlag)
		{
			pButton->SetWindowText(pFlag->GetName());
			if (m_GroupFlags & pFlag->GetMask())
				pButton->SetCheck(2);
			else
				pButton->SetCheck((Flags & pFlag->GetMask()) ? TRUE : FALSE);

			pButton->EnableWindow( DisableFlags ? FALSE : TRUE );
		}
		else
			pButton->ModifyStyle(WS_VISIBLE, 0);
	}

	GetDlgItem(IDC_FLAGSBOX)->Invalidate();
}

void CAnimationEditDlg::InitStatesDialog(bool SearchMode)
{
	int c;

	if (SearchMode)
	{
		GetDlgItem(IDC_STATESBOX)->EnableWindow( FALSE );
		GetDlgItem(IDC_STATESCROLL)->EnableWindow( FALSE );
		GetDlgItem(IDC_STATIC_BEGINNINGSTATE)->EnableWindow( FALSE );
		GetDlgItem(IDC_TRANSITIONANIM)->EnableWindow( FALSE );
	}

	//---	set the flag scroll bar range
	m_StatesScrollIndex = 0;
	c = m_pDoc->m_StateClassList.GetCount();
	if (c > 4)
	{
		((CScrollBar*) GetDlgItem(IDC_STATESCROLL))->SetScrollRange(0, c - 4);
		((CScrollBar*) GetDlgItem(IDC_STATESCROLL))->ShowScrollBar(TRUE);
	}
	else
		((CScrollBar*) GetDlgItem(IDC_STATESCROLL))->ShowScrollBar(FALSE);

	UpdateStatesDialog();
}

void CAnimationEditDlg::UpdateStatesDialog()
{
	int i, j, Default;
	CStateClass *pClass;
	CComboBox* pCombo;
	CButton* pName;

	for (i=0; i<4; i++)
	{
		pCombo = (CComboBox*)GetDlgItem(StateIDs[i]);
		pName = (CButton*) GetDlgItem(StateNameIDs[i]);

		pClass = m_pDoc->m_StateClassList.GetStateClass(i + m_StatesScrollIndex);
		if (pClass)
		{
			pName->SetWindowText(pClass->GetName());
			pCombo->ResetContent();
			Default = pClass->GetSetting(m_GroupStates);
			for (j = 0; j<pClass->GetNumStates(); j++)
			{
				if (j == Default)
					pCombo->AddString("<def> " + pClass->GetState(j)->GetName());
				else
					pCombo->AddString("          " + pClass->GetState(j)->GetName());
			}

			if( m_bLinkStates && m_bLinkStates)
			{
				if( m_pLinkAnim )
					pCombo->SetCurSel( pClass->GetSetting( m_pLinkAnim->BS_GetStates() ));
				GetDlgItem(IDC_PRIORITY)->EnableWindow( FALSE );
			}
			else
			{
				pCombo->SetCurSel(pClass->GetSetting(m_States));
				pCombo->EnableWindow( TRUE );
			}
		}
		else
		{
			pCombo->ModifyStyle(WS_VISIBLE, 0);
			pName->ModifyStyle(WS_VISIBLE, 0);
		}
	}

	GetDlgItem(IDC_STATESBOX)->Invalidate();
}


void CAnimationEditDlg::UpdatePriorityDialog(bool SearchMode)
{
	CPriorities *pPriorities;
	int c;

	((CComboBox*) GetDlgItem(IDC_PRIORITY))->ResetContent();

	if (SearchMode)
		((CComboBox*) GetDlgItem(IDC_PRIORITY))->AddString( NOSEARCH_STRING );

	//---	initialize the priority selection
	pPriorities = &m_pDoc->m_Priorities;
	for (c=0; c<pPriorities->GetCount(); c++)
	{
		if (SearchMode)
			((CComboBox*) GetDlgItem(IDC_PRIORITY))->AddString( pPriorities->GetPriority(c) );
		else if (c == m_GroupPriority)
			((CComboBox*) GetDlgItem(IDC_PRIORITY))->AddString("<def> " + pPriorities->GetPriority(c));
		else
			((CComboBox*) GetDlgItem(IDC_PRIORITY))->AddString("          " + pPriorities->GetPriority(c));
	}

	if( m_bLinkCheck && m_bLinkPriority && m_pAnimNode )
	{
		if( m_pLinkAnim )
			((CComboBox*)GetDlgItem(IDC_PRIORITY))->SetCurSel( m_pLinkAnim->BS_GetPriority() );
		GetDlgItem(IDC_PRIORITY)->EnableWindow( FALSE );
	}
	else
	{
		((CComboBox*) GetDlgItem(IDC_PRIORITY))->SetCurSel(m_Priority);
		GetDlgItem(IDC_PRIORITY)->EnableWindow( TRUE );
	}
}

void CAnimationEditDlg::UpdateMethodDialog(bool SearchMode)
{
	CMethods *pMethods;
	int c;

	((CComboBox*) GetDlgItem(IDC_METHOD))->ResetContent();

	if (SearchMode)
		((CComboBox*) GetDlgItem(IDC_METHOD))->AddString( NOSEARCH_STRING );

	//---	initialize the priority selection
	pMethods = &m_pDoc->m_Methods;
	for (c=0; c<pMethods->GetCount(); c++)
	{
		if (SearchMode)
			((CComboBox*) GetDlgItem(IDC_METHOD))->AddString( pMethods->GetMethod(c) );
		else if (c == m_GroupMethod)
			((CComboBox*) GetDlgItem(IDC_METHOD))->AddString("<def> " + pMethods->GetMethod(c));
		else
			((CComboBox*) GetDlgItem(IDC_METHOD))->AddString("          " + pMethods->GetMethod(c));
	}

	if( m_bLinkCheck && m_bLinkMethod && m_pAnimNode )
	{
		if( m_pLinkAnim )
			((CComboBox*)GetDlgItem(IDC_METHOD))->SetCurSel( m_pLinkAnim->BS_GetMethod() );
		GetDlgItem(IDC_METHOD)->EnableWindow( FALSE );
		return;
	}
	else
	{
		((CComboBox*) GetDlgItem(IDC_METHOD))->SetCurSel(m_Method);
		GetDlgItem(IDC_METHOD)->EnableWindow( TRUE );
	}
}

void CAnimationEditDlg::InitTransitionDialog(bool SearchMode)
{
	if ((m_States != m_EndStates) && (m_pAnimNode != NULL))
	{
		((CButton*) GetDlgItem(IDC_TRANSITIONANIM))->SetCheck(TRUE);
		GetDlgItem(IDC_TRANSCOMBO0)->EnableWindow(TRUE);
		GetDlgItem(IDC_TRANSCOMBO1)->EnableWindow(TRUE);
		GetDlgItem(IDC_TRANSCOMBO2)->EnableWindow(TRUE);
		GetDlgItem(IDC_TRANSCOMBO3)->EnableWindow(TRUE);
	}
	else
	{
		((CButton*) GetDlgItem(IDC_TRANSITIONANIM))->SetCheck(FALSE);
		GetDlgItem(IDC_TRANSCOMBO0)->EnableWindow(FALSE);
		GetDlgItem(IDC_TRANSCOMBO1)->EnableWindow(FALSE);
		GetDlgItem(IDC_TRANSCOMBO2)->EnableWindow(FALSE);
		GetDlgItem(IDC_TRANSCOMBO3)->EnableWindow(FALSE);
		if (m_pAnimNode == NULL)
			((CButton*) GetDlgItem(IDC_TRANSITIONANIM))->EnableWindow(FALSE);
	}

	UpdateTransitionDialog();
}

void CAnimationEditDlg::UpdateTransitionDialog()
{
	int i, j, Default;
	CStateClass *pClass;
	CComboBox* pCombo;

	for (i=0; i<4; i++)
	{
		pCombo = (CComboBox*)GetDlgItem(StateTransIDs[i]);

		pClass = m_pDoc->m_StateClassList.GetStateClass(i + m_StatesScrollIndex);
		if (pClass)
		{
			pCombo->ResetContent();
			Default = pClass->GetSetting(m_States);
			for (j = 0; j<pClass->GetNumStates(); j++)
			{
				if (j == Default)
					pCombo->AddString("      <no change>");
				else
					pCombo->AddString("      " + pClass->GetState(j)->GetName());
			}

			pCombo->SetCurSel(pClass->GetSetting(m_EndStates));
		}
		else
		{
			pCombo->ModifyStyle(WS_VISIBLE, 0);
		}
	}

	GetDlgItem(IDC_STATESBOX)->Invalidate();
}

void CAnimationEditDlg::OnFlag(int Index)
{
	CButton *pCheck = (CButton*)GetDlgItem(FlagIDs[Index]);
	CFlag* pFlag = m_pDoc->m_FlagList.GetFlag(m_FlagsScrollIndex + Index);

	//---	set the flags appropriately
	if (pFlag->GetMask() & m_GroupFlags)
	{
		pCheck->SetCheck( 2 );
	}
	else
	{
		if (pCheck->GetCheck() == 1)
			m_Flags |= pFlag->GetMask();
		else
			m_Flags &= ~pFlag->GetMask();

		//---	update the flags dialog
		UpdateFlagsDialog();
	}
}

void CAnimationEditDlg::OnState(int Index)
{
	CStateClass *pClass = m_pDoc->m_StateClassList.GetStateClass(Index + m_StatesScrollIndex);
	CComboBox *pCombo = (CComboBox*)GetDlgItem(StateIDs[Index]);

	//---	if 
	if (pClass->GetSetting(m_States) == pClass->GetSetting(m_EndStates))
		pClass->SetSetting(m_EndStates, pCombo->GetCurSel());

	//---	set the state appropriately
	pClass->SetSetting(m_States, pCombo->GetCurSel());

	//---	update the states dialog
	UpdateStatesDialog();
	UpdateTransitionDialog();
}

void CAnimationEditDlg::OnTrans(int Index)
{
	CStateClass *pClass = m_pDoc->m_StateClassList.GetStateClass(Index + m_StatesScrollIndex);
	CComboBox *pCombo = (CComboBox*)GetDlgItem(StateTransIDs[Index]);

	//---	set the state appropriately
	pClass->SetSetting(m_EndStates, pCombo->GetCurSel());

	//---	update the states dialog
	UpdateTransitionDialog();
}

//DEL void CAnimationEditDlg::OnAddsourceid() 
//DEL {
//DEL 	CSourceIDDlg	SourceIDDlg;
//DEL 
//DEL 	SourceIDDlg.m_pIDList	= m_pIDList;
//DEL 	SourceIDDlg.m_pAnimNode = m_pAnimNode;
//DEL 
//DEL 	SourceIDDlg.DoModal();
//DEL 	
//DEL 	if (SourceIDDlg.m_Modified)
//DEL 	{
//DEL 		//---	If the name field is not set to anything then give it the default name.
//DEL 		if ((strlen( m_pAnimNode->GetName() ) == 0) || (strcmp( m_pAnimNode->GetName(), "(null)" ) == 0) )
//DEL 		{
//DEL 			SourceIDDlg.m_pAnimNode->SetDefaultName();
//DEL 				
//DEL 			SetDlgItemText( IDC_ANIMNAME, m_pAnimNode->GetName() );
//DEL 		}
//DEL 			
//DEL 		//---	Refill the list box with the new list of animations.
//DEL 		RefillListBox();
//DEL 
//DEL //		SetDlgItemText( IDC_SOURCEID, m_pAnimNode->GetID() ) ;
//DEL 	}
//DEL 
//DEL 	EnableDeleteButton();
//DEL }

//DEL void CAnimationEditDlg::OnDeletesourceid() 
//DEL {
//DEL 	CString	IDToDelete;
//DEL 
//DEL 	// TODO: Add your control notification handler code here
//DEL 	if (m_SourceIDList.GetCurSel() >= 0)
//DEL 	{
//DEL 		m_SourceIDList.GetText( m_SourceIDList.GetCurSel(), IDToDelete);
//DEL 		m_pAnimNode->RemoveID( IDToDelete );
//DEL 		m_pIDList->DecrementUses( IDToDelete );
//DEL 	}
//DEL 
//DEL 	RefillListBox();
//DEL 	EnableDeleteButton();
//DEL }

//DEL void CAnimationEditDlg::EnableDeleteButton()
//DEL {
//DEL 	CButton *pButton = (CButton*)GetDlgItem( IDC_DELETESOURCEID );
//DEL 
//DEL 	if (m_SourceIDList.GetCount() < 2)
//DEL 		pButton->EnableWindow( FALSE );
//DEL 	else
//DEL 		pButton->EnableWindow( TRUE );
//DEL }


//DEL void CAnimationEditDlg::RefillListBox()
//DEL {
//DEL 	CListBox		*pListBox = (CListBox*)GetDlgItem( IDC_ANIMLIST );
//DEL 
//DEL 	pListBox->ResetContent();
//DEL 	pListBox->AddString( m_pAnimNode->GetFirstAnim() );
//DEL 	for (int i = 1; i < m_pAnimNode->GetNumAnims(); i++)
//DEL 		pListBox->AddString( m_pAnimNode->GetNextAnim() );
//DEL }


void CAnimationEditDlg::OnDblclkListDependancies() 
{
	this->OnButtonEditdependancy();	
}

void CAnimationEditDlg::FillDependancyList()
{
	CAnimTrans	*pAnimTrans;
	CString		Description;

 	m_DependancyList.ResetContent();
	pAnimTrans = m_AnimTransList.GetFirst();
	while (pAnimTrans)
	{
		Description = pAnimTrans->GetTagName();
//		pAnimTrans->GetNLDescription( Description );
	 	m_DependancyList.InsertString( -1, Description );
		

		pAnimTrans = m_AnimTransList.GetNext();
	}
}

void CAnimationEditDlg::OnButtonNewdependancy() 
{
	CAnimTreeNode*	pNode		= (CAnimTreeNode*)(m_pAnimNode != NULL) ? (CAnimTreeNode*)m_pAnimNode : (CAnimTreeNode*)m_pGroupNode;
	CAnimTrans*		pAnimTrans	= new CAnimTrans( m_pDoc, pNode );
	
	if (!pAnimTrans->Edit( pNode->GetName() ))
	{
		delete pAnimTrans;
		return;
	}

	m_AnimTransList.Add( pAnimTrans );

	//---	Redraw our list box with the updated dependancy list.
	FillDependancyList();
}

void CAnimationEditDlg::OnButtonEditdependancy() 
{
	CAnimTrans	*pAnimTrans;
	CListBox	*pListBox;
	int			CurSel;

	pListBox = (CListBox*)this->GetDlgItem( IDC_LIST_DEPENDANCIES );;
	CurSel = pListBox->GetCurSel();

	if (CurSel < 0)
		return;

	pAnimTrans = m_AnimTransList.GetNth( CurSel );
	if (pAnimTrans)
	{
		//---	if this animation is a default one from parents, do not allow it to be edited
		if (pAnimTrans->GetTag() == "<def> ")
			return;

		pAnimTrans->Edit( m_pAnimNode->GetName() );
	}

	//---	Redraw our list box with the updated dependancy list.
	FillDependancyList();
}

void CAnimationEditDlg::OnButtonDeletedependancy() 
{
	CAnimTrans	*pAnimTrans;
	CListBox	*pListBox;
	int			CurSel;

	pListBox = (CListBox*)this->GetDlgItem( IDC_LIST_DEPENDANCIES );;
	CurSel = pListBox->GetCurSel();

	if (CurSel < 0)
		return;

	pAnimTrans = m_AnimTransList.GetNth( CurSel );
	if (pAnimTrans)
	{
		//---	if this animation is a default one from parents, do not allow it to be edited
		if (pAnimTrans->GetTag() == "<def> ")
			return;

		pAnimTrans = m_AnimTransList.Remove( pAnimTrans );
		delete pAnimTrans;
	}

	//---	Redraw our list box with the updated dependancy list.
	FillDependancyList();
}


void CAnimationEditDlg::OnButtonDefaultprereq() 
{
	CAnimTrans	*pAnimTrans;
	CListBox	*pListBox;
	int			CurSel;

	//---	Moves the current selection to the top of the list where it will act as the default.
	pListBox = (CListBox*)this->GetDlgItem( IDC_LIST_DEPENDANCIES );
	CurSel = pListBox->GetCurSel();

	if (CurSel >= 0)
	{
		pAnimTrans = m_AnimTransList.GetNth( CurSel );
		if (pAnimTrans && pAnimTrans->IsPrerequisite())
			m_AnimTransList.SetDefaultPrerequisite( pAnimTrans );
	}

	//---	Redraw our list box with the updated dependancy list.
	FillDependancyList();
}

void CAnimationEditDlg::OnSelchangeListDependancies() 
{
	CAnimTrans	*pAnimTrans;
	CListBox	*pListBox;
	int			CurSel;

	//---	Moves the current selection to the top of the list where it will act as the default.
	pListBox = (CListBox*)this->GetDlgItem( IDC_LIST_DEPENDANCIES );;
	CurSel = pListBox->GetCurSel();

	if (CurSel >= 0)
	{
		((CButton*)this->GetDlgItem( IDC_BUTTON_EDITDEPENDANCY ))->EnableWindow( TRUE );
		((CButton*)this->GetDlgItem( IDC_BUTTON_DELETEDEPENDANCY ))->EnableWindow( TRUE );

		pAnimTrans = m_AnimTransList.GetNth( CurSel );
		if (pAnimTrans && pAnimTrans->IsPrerequisite() && (strcmp( pAnimTrans->GetFromAnim(), "<any>" ) != 0))
		{
			//---	If it is a prerequisite that is not a catch-all then it can be made the default.
			((CButton*)this->GetDlgItem( IDC_BUTTON_DEFAULTPREREQ ))->EnableWindow( TRUE );
			return;
		}
	}
	else
	{
		((CButton*)this->GetDlgItem( IDC_BUTTON_EDITDEPENDANCY ))->EnableWindow( TRUE );
		((CButton*)this->GetDlgItem( IDC_BUTTON_DELETEDEPENDANCY ))->EnableWindow( TRUE );
	}

	((CButton*)this->GetDlgItem( IDC_BUTTON_DEFAULTPREREQ ))->EnableWindow( FALSE );
}

void CAnimationEditDlg::OnSelcancelListDependancies() 
{
	((CButton*)this->GetDlgItem( IDC_BUTTON_EDITDEPENDANCY ))->EnableWindow( FALSE );
	((CButton*)this->GetDlgItem( IDC_BUTTON_DELETEDEPENDANCY ))->EnableWindow( FALSE );
	((CButton*)this->GetDlgItem( IDC_BUTTON_DEFAULTPREREQ ))->EnableWindow( FALSE );
}

/*
typedef struct tagSCROLLINFO {  // si
	UINT cbSize;
	UINT fMask; 
    int  nMin;
	int  nMax;
	UINT nPage;
	int  nPos; 
    int  nTrackPos;
}   SCROLLINFO; 
*/	
void CAnimationEditDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);

	//---	update the scroll bar
	UpdateScrollBar(pScrollBar, nSBCode);

	//---	update the dialog
	if (pScrollBar == (CScrollBar*) GetDlgItem(IDC_FLAGSCROLL))
	{
		if (pScrollBar->GetScrollPos() != m_FlagsScrollIndex)
		{
			m_FlagsScrollIndex = pScrollBar->GetScrollPos();
			UpdateFlagsDialog();
		}
	}
	else if (pScrollBar == (CScrollBar*) GetDlgItem(IDC_STATESCROLL))
	{
		if (pScrollBar->GetScrollPos() != m_StatesScrollIndex)
		{
			m_StatesScrollIndex = pScrollBar->GetScrollPos();
			UpdateStatesDialog();
			UpdateTransitionDialog();
		}
	}
}

void CAnimationEditDlg::OnFlag0() 
{
	OnFlag(0);
}

void CAnimationEditDlg::OnFlag1() 
{
	OnFlag(1);
}

void CAnimationEditDlg::OnFlag2() 
{
	OnFlag(2);
}

void CAnimationEditDlg::OnFlag3() 
{
	OnFlag(3);
}

void CAnimationEditDlg::OnFlag4() 
{
	OnFlag(4);
}

void CAnimationEditDlg::OnFlag5() 
{
	OnFlag(5);
}

void CAnimationEditDlg::OnFlag6() 
{
	OnFlag(6);
}

void CAnimationEditDlg::OnFlag7() 
{
	OnFlag(7);
}

void CAnimationEditDlg::OnFlag8() 
{
	OnFlag(8);
}

void CAnimationEditDlg::OnFlag9() 
{
	OnFlag(9);
}

void CAnimationEditDlg::OnFlag10() 
{
	OnFlag(10);
}

void CAnimationEditDlg::OnFlag11() 
{
	OnFlag(11);
}

void CAnimationEditDlg::OnSelchangeStatecombo0() 
{
	OnState(0);
}

void CAnimationEditDlg::OnSelchangeStatecombo1() 
{
	OnState(1);
}

void CAnimationEditDlg::OnSelchangeStatecombo2() 
{
	OnState(2);
}

void CAnimationEditDlg::OnSelchangeStatecombo3() 
{
	OnState(3);
}

void CAnimationEditDlg::OnTransitionanim() 
{
	if (((CButton*) GetDlgItem(IDC_TRANSITIONANIM))->GetCheck() == TRUE)
	{
		GetDlgItem(IDC_TRANSCOMBO0)->EnableWindow(TRUE);
		GetDlgItem(IDC_TRANSCOMBO1)->EnableWindow(TRUE);
		GetDlgItem(IDC_TRANSCOMBO2)->EnableWindow(TRUE);
		GetDlgItem(IDC_TRANSCOMBO3)->EnableWindow(TRUE);
	}
	else
	{
		m_EndStates = m_States;
		GetDlgItem(IDC_TRANSCOMBO0)->EnableWindow(FALSE);
		GetDlgItem(IDC_TRANSCOMBO1)->EnableWindow(FALSE);
		GetDlgItem(IDC_TRANSCOMBO2)->EnableWindow(FALSE);
		GetDlgItem(IDC_TRANSCOMBO3)->EnableWindow(FALSE);
	}

	UpdateTransitionDialog();
}

void CAnimationEditDlg::OnSelchangeTranscombo0() 
{
	OnTrans(0);
}

void CAnimationEditDlg::OnSelchangeTranscombo1() 
{
	OnTrans(1);
}

void CAnimationEditDlg::OnSelchangeTranscombo2() 
{
	OnTrans(2);
}

void CAnimationEditDlg::OnSelchangeTranscombo3() 
{
	OnTrans(3);
}

void CAnimationEditDlg::OnKillfocusFramerate() 
{
	CString AnimRate;
	int find, find2, Len;

	//---	get the string from the edit box
	((CEdit*) GetDlgItem(IDC_FRAMERATE))->GetWindowText(AnimRate.GetBufferSetLength(20), 19);

	//---	search the string for unwanted characters
	do
	{
		find = AnimRate.FindOneOf("`~!@#$%^&*()-_+=\\|}][{PpOoIiUuYyTtRrEeWwQqAaSsDdFfGgHhJjKkLl:;""'ZzXxCcVvBbNnMm<,>?/");
		if (find != -1)
			AnimRate = AnimRate.Left(find) + AnimRate.Right(AnimRate.GetLength() - find - 1);
	} while (find != -1);

	Len = AnimRate.SpanIncluding("0123456789.").GetLength();
	AnimRate = AnimRate.Left(Len);

	find = AnimRate.FindOneOf(".");
	if (find == -1)
	{
		CString point0 = ".0";
		AnimRate = AnimRate + point0;
	}
	else
	{
		CString Right = AnimRate.Right(AnimRate.GetLength() - find);
		do
		{
			find2 = Right.FindOneOf(".");
			if (find2 != -1)
			{
				Right = Right.Left(find2) + Right.Right(Right.GetLength() - find2 - 1);
				AnimRate = AnimRate.Left(find + 1) + Right;
			}
		} while (find2 != -1);
	}


	m_FrameRate = AnimRate;
	((CEdit*) GetDlgItem(IDC_FRAMERATE))->SetWindowText(AnimRate);
}

void CAnimationEditDlg::InitFrameRateDialog(bool SearchMode)
{
	if ((m_pAnimNode == NULL) || SearchMode)
	{
		GetDlgItem( IDC_FRAMERATEBOX )->EnableWindow( FALSE );
		GetDlgItem( IDC_FRAMERATE )->EnableWindow( FALSE );
		return;
	}

	m_FrameRate = m_pAnimNode->m_FrameRate;
	
	UpdateFrameRateDialog();
}

void CAnimationEditDlg::UpdateFrameRateDialog()
{
	CEdit *pEdit = (CEdit*) GetDlgItem(IDC_FRAMERATE);

	//---	if the rate is linked to another animation, display the combo box and select the linked animation
	if (m_bLinkCheck && m_bLinkFrameRate && m_pAnimNode )
	{
		pEdit->EnableWindow( FALSE );
	}
	else
	{
		//---	otherwise, display the frame rate
		pEdit->EnableWindow( TRUE );

		if (m_FrameRate.IsEmpty() || (m_FrameRate.FindOneOf("`~!@#$%^&*()_-=+|\\}][{PpOoIiUuYyTtRrEeWqQwAaSsDdFfGgHhJjKkLl:;""'?/><,MmNnBbVvCcXxZz") != -1))
			m_FrameRate = "1.0";

		pEdit->SetWindowText(m_FrameRate);
	}
}

void CAnimationEditDlg::InitLinkAnimDialog(bool SearchMode)
{
	if ((m_pAnimNode == NULL) || SearchMode)
	{
		GetDlgItem( IDC_LINKBOX )->EnableWindow( FALSE );
		GetDlgItem( IDC_LINKTOANIM )->EnableWindow( FALSE );
		GetDlgItem( IDC_LINKRATELIST )->EnableWindow( FALSE );
		GetDlgItem( IDC_LINKMETHOD )->EnableWindow( FALSE );
		GetDlgItem( IDC_LINKPRIORITY )->EnableWindow( FALSE );
		GetDlgItem( IDC_LINKRATE )->EnableWindow( FALSE );
		GetDlgItem( IDC_LINKFLAGS )->EnableWindow( FALSE );
		GetDlgItem( IDC_LINKSTATES )->EnableWindow( FALSE );
		GetDlgItem( IDC_LINKTRANSITIONS )->EnableWindow( FALSE );
		GetDlgItem( IDC_LINKENTERFRAME )->EnableWindow( FALSE );
		GetDlgItem( IDC_LINKENTERFRAMETITLE )->EnableWindow( FALSE );
		return;
	}

	m_LinkAnim	= m_pAnimNode->m_LinkAnim;
	m_pLinkAnim = m_pDoc->FindAnimByName( m_LinkAnim );

	//---	determine if this animation's rate is linked to another animation or given directly
	if ( m_LinkAnim.IsEmpty() )
		m_bLinkCheck = FALSE;
	else
		m_bLinkCheck = TRUE;

	((CButton*) GetDlgItem(IDC_LINKTOANIM))->SetCheck(m_bLinkCheck);

	//---	add the default link animrate settings to the link rate list
	((CComboBox*) GetDlgItem(IDC_LINKRATELIST))->AddString("<previous anim>");
	((CComboBox*) GetDlgItem(IDC_LINKRATELIST))->AddString("<destination anim>");

	//---	fill the animation combo box
	m_pAnimNode->m_pDoc->FillAnimComboBox(((CComboBox*) GetDlgItem(IDC_LINKRATELIST)));

	UpdateLinkAnimDialog();
}

void CAnimationEditDlg::UpdateLinkAnimDialog()
{
	CComboBox *pList = ((CComboBox*) GetDlgItem(IDC_LINKRATELIST));
	int CurSel = pList->FindStringExact( 0, m_LinkAnim );
	pList->SetCurSel( CurSel );

	//---	if the rate is linked to another animation, display the combo box and select the linked animation
	((CButton*)GetDlgItem( IDC_LINKMETHOD ))->SetCheck( m_bLinkMethod );
	((CButton*)GetDlgItem( IDC_LINKPRIORITY ))->SetCheck( m_bLinkPriority );
	((CButton*)GetDlgItem( IDC_LINKRATE ))->SetCheck( m_bLinkFrameRate );
	((CButton*)GetDlgItem( IDC_LINKFLAGS ))->SetCheck( m_bLinkFlags );
	((CButton*)GetDlgItem( IDC_LINKSTATES ))->SetCheck( m_bLinkStates );
	((CButton*)GetDlgItem( IDC_LINKTRANSITIONS ))->SetCheck( m_bLinkTransitions );
	CString EnterFrame;
	EnterFrame.Format( "%d", m_LinkEnterFrame );
	((CEdit*)GetDlgItem( IDC_LINKENTERFRAME ))->SetWindowText( EnterFrame );

	//---	enable/disable based on the link check box
	GetDlgItem( IDC_LINKRATELIST )->EnableWindow( m_bLinkCheck );
	GetDlgItem( IDC_LINKMETHOD )->EnableWindow( m_bLinkCheck );
	GetDlgItem( IDC_LINKPRIORITY )->EnableWindow( m_bLinkCheck );
	GetDlgItem( IDC_LINKRATE )->EnableWindow( m_bLinkCheck );
	GetDlgItem( IDC_LINKFLAGS )->EnableWindow( m_bLinkCheck );
	GetDlgItem( IDC_LINKSTATES )->EnableWindow( m_bLinkCheck );
	GetDlgItem( IDC_LINKTRANSITIONS )->EnableWindow( m_bLinkCheck );
	GetDlgItem( IDC_LINKENTERFRAME )->EnableWindow( m_bLinkCheck );
	GetDlgItem( IDC_LINKENTERFRAMETITLE )->EnableWindow( m_bLinkCheck );
}

void CAnimationEditDlg::OnLinktoanim()
{
	// TODO: Add your control notification handler code here
	m_bLinkCheck = ((CButton*) GetDlgItem(IDC_LINKTOANIM))->GetCheck();

	UpdateLinkAnimDialog();
	UpdateMethodDialog();
	UpdatePriorityDialog();
	UpdateFrameRateDialog();
	UpdateFlagsDialog();
	UpdateStatesDialog();
	UpdateFrameRateDialog();
}

void CAnimationEditDlg::OnSelchangeLinkratelist() 
{
	CComboBox *pList = ((CComboBox*) GetDlgItem(IDC_LINKRATELIST));

	if (pList->GetCurSel() == -1)
	{
		m_bLinkCheck = FALSE;
		m_LinkAnim = "";
	}
	else
		pList->GetLBText(pList->GetCurSel(), m_LinkAnim );

	m_pLinkAnim = m_pDoc->FindAnimByName( m_LinkAnim );

	UpdateLinkAnimDialog();
	UpdateMethodDialog();
	UpdatePriorityDialog();
	UpdateFrameRateDialog();
	UpdateFlagsDialog();
	UpdateStatesDialog();
	UpdateFrameRateDialog();
}

void CAnimationEditDlg::OnButtonLeftchangeid() 
{
	CSourceIDDlg	SourceIDDlg;
	CString			AnimID;

	GetDlgItemText( IDC_EDIT_LEFTID, AnimID );
	SourceIDDlg.m_AnimID = AnimID;
	SourceIDDlg.m_pIDList = m_pIDList;

	if (SourceIDDlg.DoModal() == IDOK)
	{
		if (AnimID != SourceIDDlg.m_AnimID)
		{
			AnimID = SourceIDDlg.m_AnimID;
			SetDlgItemText( IDC_EDIT_LEFTID, AnimID );

			// If we are setting search variables then do not do smart member setting.
			if (!m_bSearchMode)
			{
				CString OldAnimID;

				GetDlgItemText( IDC_EDIT_RIGHTID, OldAnimID );
				if( OldAnimID.GetLength() == 0 )
					SetDlgItemText( IDC_EDIT_RIGHTID, AnimID );

				CString OldName;
				GetDlgItemText( IDC_ANIMNAME, OldName );
				if( (OldName.GetLength() == 0) || (OldName == "ANIMATION") )
				{
					m_Name = AnimID;
					m_Name.MakeUpper();
					SetDlgItemText( IDC_ANIMNAME, m_Name );
					GetDlgItem( IDC_ANIMNAME )->SetFocus();
				}
				else
				this->GetDlgItem( IDOK )->SetFocus();
			}
		}
	}
}

void CAnimationEditDlg::OnButtonRightchangeid() 
{
	CSourceIDDlg	SourceIDDlg;
	CString			AnimID;

	GetDlgItemText( IDC_EDIT_RIGHTID, AnimID );
	SourceIDDlg.m_AnimID = AnimID;
	SourceIDDlg.m_pIDList = m_pIDList;

	if (SourceIDDlg.DoModal() == IDOK)
	{
		if (AnimID != SourceIDDlg.m_AnimID)
		{
			AnimID = SourceIDDlg.m_AnimID;
			SetDlgItemText( IDC_EDIT_RIGHTID, AnimID );

			CString OldAnimID;
			
			// If we are setting search variables then do not do smart member setting.
			if (!m_bSearchMode)
			{
				GetDlgItemText( IDC_EDIT_LEFTID, OldAnimID );
				if( OldAnimID.GetLength() == 0 )
					SetDlgItemText( IDC_EDIT_LEFTID, AnimID );

				CString OldName;
				GetDlgItemText( IDC_ANIMNAME, OldName );
				if( (OldName.GetLength() == 0) || (OldName == "ANIMATION") )
				{
					m_Name = AnimID;
					m_Name.MakeUpper();
					SetDlgItemText( IDC_ANIMNAME, m_Name );
					GetDlgItem( IDC_ANIMNAME )->SetFocus();
				}
				else
					this->GetDlgItem( IDOK )->SetFocus();
			}
		}
	}
}


void CAnimationEditDlg::OnChangeboth() 
{
	CSourceIDDlg	SourceIDDlg;
	CString			AnimID;

	GetDlgItemText( IDC_EDIT_LEFTID, AnimID );
	SourceIDDlg.m_AnimID = AnimID;
	SourceIDDlg.m_pIDList = m_pIDList;

	if (SourceIDDlg.DoModal() == IDOK)
	{
		if (AnimID != SourceIDDlg.m_AnimID)
		{
			AnimID = SourceIDDlg.m_AnimID;
			SetDlgItemText( IDC_EDIT_LEFTID, AnimID );
			SetDlgItemText( IDC_EDIT_RIGHTID, AnimID );

			CString OldName;
			GetDlgItemText( IDC_ANIMNAME, OldName );
			if( (OldName.GetLength() == 0) || (OldName == "ANIMATION") )
			{
				m_Name = AnimID;
				m_Name.MakeUpper();
				SetDlgItemText( IDC_ANIMNAME, m_Name );
				GetDlgItem( IDC_ANIMNAME )->SetFocus();
			}
			else
			this->GetDlgItem( IDOK )->SetFocus();
		}
	}
}


void CAnimationEditDlg::OnButtonSwap() 
{
	// TODO: Add your control notification handler code here
	CString	RightName;
	CString	LeftName;
	int		RightMirrored;
	int		RightReversed;
	int		LeftMirrored;
	int		LeftReversed;

	//---	Swap the identifiers and the flags.	
	GetDlgItemText( IDC_EDIT_RIGHTID, RightName );
	GetDlgItemText( IDC_EDIT_LEFTID, LeftName );

	RightMirrored = ((CButton*)GetDlgItem( IDC_CHECK_RIGHTMIRRORED ))->GetCheck();
	RightReversed = ((CButton*)GetDlgItem( IDC_CHECK_RIGHTREVERSED ))->GetCheck();
	LeftMirrored = ((CButton*)GetDlgItem( IDC_CHECK_LEFTMIRRORED ))->GetCheck();
	LeftReversed = ((CButton*)GetDlgItem( IDC_CHECK_LEFTREVERSED ))->GetCheck();

	SetDlgItemText( IDC_EDIT_RIGHTID, LeftName );
	SetDlgItemText( IDC_EDIT_LEFTID, RightName );

	((CButton*)GetDlgItem( IDC_CHECK_RIGHTMIRRORED ))->SetCheck( LeftMirrored );
	((CButton*)GetDlgItem( IDC_CHECK_RIGHTREVERSED ))->SetCheck( LeftReversed );
	((CButton*)GetDlgItem( IDC_CHECK_LEFTMIRRORED ))->SetCheck( RightMirrored );
	((CButton*)GetDlgItem( IDC_CHECK_LEFTREVERSED ))->SetCheck( RightReversed );
}

void CAnimationEditDlg::OnKillfocusCallback() 
{
	CString Callback;
	CString	Prefix;
	CString Build;

	GetDlgItemText( IDC_CALLBACK, Callback );	

	//---	remove the prefix
	Prefix = Callback.Left(6);
	if ((Prefix == "      ") || (Prefix == "<def> "))
		Callback = Callback.Right( Callback.GetLength() - 6 );

	//---	remove any extra spaces from the callback
	Callback.TrimLeft();
	Callback.TrimRight();

	//---	remove illegal characters
	Build = Callback.SpanIncluding( "_0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" );
	while (Build != Callback)
	{
		if (Callback[ Build.GetLength() ] == ' ')
		{
			char* pStr = Callback.GetBuffer( Callback.GetLength() );
			pStr[ Build.GetLength() ] = '_';
			Callback.ReleaseBuffer();
		}
		else
			Callback = Callback.Left( Build.GetLength() ) + Callback.Right( Callback.GetLength() - Build.GetLength() - 1 );

		Build = Callback.SpanIncluding( "_0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" );
	}
/*
	//---	decide if this callback function currently exists
	CComboBox* pComboBox = (CComboBox*) GetDlgItem( IDC_CALLBACK );
	s32 Index = pComboBox->FindString( -1, Callback );
	CString String;
	while (Index != -1)
	{
		pComboBox->GetLBText( Index, String );
		String = String.Right( String.GetLength() - 6 );
		if (Callback == String)
		{
		}
	}*/

	//---	add back on the appropriate prefix
	m_Callback = Callback;
	if (Callback == m_ParentCallback)
		Callback = "<def> " + Callback;
	else
		Callback = "      " + Callback;

	//---	set the string
	SetDlgItemText( IDC_CALLBACK, Callback );	
}

void CAnimationEditDlg::OnSetfocusCallback() 
{
	CString Callback;

	GetDlgItemText( IDC_CALLBACK, Callback );	

	if (Callback.GetLength() > 6)
	{
		Callback = Callback.Right( Callback.GetLength() - 6 );
		SetDlgItemText( IDC_CALLBACK, Callback );	
	}
}

void CAnimationEditDlg::OnSelchangeCallback() 
{
}

void CAnimationEditDlg::OnSelchangeSelectcallback() 
{
	// TODO: Add your control notification handler code here
	
}

void CAnimationEditDlg::OnSetfocusSelectcallback() 
{
	CString Callback;

	GetDlgItemText( IDC_SELECTCALLBACK, Callback );	

	if (Callback.GetLength() > 6)
	{
		Callback = Callback.Right( Callback.GetLength() - 6 );
		SetDlgItemText( IDC_SELECTCALLBACK, Callback );	
	}
}

void CAnimationEditDlg::OnKillfocusSelectcallback() 
{
	CString Callback;
	CString	Prefix;
	CString Build;

	GetDlgItemText( IDC_SELECTCALLBACK, Callback );	

	//---	remove the prefix
	Prefix = Callback.Left(6);
	if ((Prefix == "      ") || (Prefix == "<def> "))
		Callback = Callback.Right( Callback.GetLength() - 6 );

	//---	remove any extra spaces from the callback
	Callback.TrimLeft();
	Callback.TrimRight();

	//---	remove illegal characters
	Build = Callback.SpanIncluding( "_0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" );
	while (Build != Callback)
	{
		if (Callback[ Build.GetLength() ] == ' ')
		{
			char* pStr = Callback.GetBuffer( Callback.GetLength() );
			pStr[ Build.GetLength() ] = '_';
			Callback.ReleaseBuffer();
		}
		else
			Callback = Callback.Left( Build.GetLength() ) + Callback.Right( Callback.GetLength() - Build.GetLength() - 1 );

		Build = Callback.SpanIncluding( "_0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" );
	}
/*
	//---	decide if this callback function currently exists
	CComboBox* pComboBox = (CComboBox*) GetDlgItem( IDC_CALLBACK );
	s32 Index = pComboBox->FindString( -1, Callback );
	CString String;
	while (Index != -1)
	{
		pComboBox->GetLBText( Index, String );
		String = String.Right( String.GetLength() - 6 );
		if (Callback == String)
		{
		}
	}*/

	//---	add back on the appropriate prefix
	m_Callback = Callback;
	if (Callback == m_ParentCallback)
		Callback = "<def> " + Callback;
	else
		Callback = "      " + Callback;

	//---	set the string
	SetDlgItemText( IDC_SELECTCALLBACK, Callback );	
}

void CAnimationEditDlg::OnChangeLinkenterframe() 
{
	CString Frame;

	((CEdit*)GetDlgItem(IDC_LINKENTERFRAME))->GetWindowText( Frame );
	m_LinkEnterFrame = atoi( Frame );
}

void CAnimationEditDlg::OnKillfocusLinkenterframe() 
{
	UpdateLinkAnimDialog();
}

void CAnimationEditDlg::OnLinkmethod() 
{
	m_bLinkMethod = ((CButton*)GetDlgItem(IDC_LINKMETHOD))->GetCheck() ? TRUE : FALSE;
	UpdateLinkAnimDialog();
	UpdateMethodDialog();
}

void CAnimationEditDlg::OnLinkpriority() 
{
	m_bLinkPriority = ((CButton*)GetDlgItem(IDC_LINKPRIORITY))->GetCheck() ? TRUE : FALSE;
	UpdateLinkAnimDialog();
	UpdatePriorityDialog();
}

void CAnimationEditDlg::OnLinkrate() 
{
	m_bLinkFrameRate = ((CButton*)GetDlgItem(IDC_LINKRATE))->GetCheck() ? TRUE : FALSE;
	UpdateLinkAnimDialog();
	UpdateFrameRateDialog();
}

void CAnimationEditDlg::OnLinkflags() 
{
	m_bLinkFlags = ((CButton*)GetDlgItem(IDC_LINKFLAGS))->GetCheck() ? TRUE : FALSE;
	UpdateLinkAnimDialog();
	UpdateFlagsDialog();
}

void CAnimationEditDlg::OnLinkstates() 
{
	m_bLinkStates	= ((CButton*)GetDlgItem(IDC_LINKSTATES))->GetCheck() ? TRUE : FALSE;
	UpdateLinkAnimDialog();
	UpdateStatesDialog();
}

void CAnimationEditDlg::OnLinktransitions() 
{
	m_bLinkTransitions = ((CButton*)GetDlgItem(IDC_LINKTRANSITIONS))->GetCheck() ? TRUE : FALSE;
	UpdateLinkAnimDialog();
}




