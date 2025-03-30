// DlgProps.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "DlgProps.h"
#include "MotionEditView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static bool s_bInUpdate = FALSE;

/////////////////////////////////////////////////////////////////////////////
// CDlgProps dialog


CDlgProps::CDlgProps(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgProps::IDD, pParent)
{
	m_pView	= NULL;

	//{{AFX_DATA_INIT(CDlgProps)
	//}}AFX_DATA_INIT
}


void CDlgProps::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgProps)
	DDX_Control(pDX, IDC_LINKMOTIONCOMBOTITLE, m_LinkMotionComboTitle);
	DDX_Control(pDX, IDC_PROPSELECT, m_PropSelectCombo);
	DDX_Control(pDX, IDC_LINKMOTION, m_LinkMotionCombo);
	DDX_Control(pDX, IDC_EXPORT, m_Export);
	DDX_Control(pDX, IDC_ACTIVE, m_Active);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgProps, CDialog)
	//{{AFX_MSG_MAP(CDlgProps)
	ON_BN_CLICKED(IDC_ACTIVE, OnActive)
	ON_BN_CLICKED(IDC_EXPORT, OnExport)
	ON_CBN_SELCHANGE(IDC_LINKMOTION, OnSelchangeLinkmotion)
	ON_CBN_SELCHANGE(IDC_PROPSELECT, OnSelchangePropselect)
	ON_BN_CLICKED(IDC_EDITDEF, OnEditdef)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgProps message handlers


BOOL CDlgProps::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


//==========================================================================================
void CDlgProps::UpdatePropList( bool bNewMotionSelected )
{
	s32			Index;
	s32			i;
	CPropDef*	pSelectedProp;

	//---	if there is no selected character, disable the dialog
	if( !m_pView || !m_pView->m_pCharacter )
	{
		m_PropSelectCombo.EnableWindow( FALSE );
		UpdatePropData();
		return;
	}

	//---	hack to let the dialog controls know that it is the update functions setting them
	//		and not the user.
	s_bInUpdate = TRUE;

	//---	enable tihe combo box window
	m_PropSelectCombo.EnableWindow( TRUE );

	//---	save the currently selected prop, and reset
	//		the prop select combo box's content.
	Index			= m_PropSelectCombo.GetCurSel();
	if( Index == -1 )
		pSelectedProp	= NULL;
	else
		pSelectedProp	= (CPropDef*) m_PropSelectCombo.GetItemData( Index );
	m_PropSelectCombo.ResetContent();

	//---	add any active props to the list.  Set the list item's data value
	//		to the props true index.
	for( i=0; i<MAX_NUM_PROPS; i++ )
	{
		CPropDef&	rPropDef = m_pView->m_pCharacter->GetPropDef(i);

		if( rPropDef.IsActive() )
		{
			Index = m_PropSelectCombo.AddString( rPropDef.GetPropName() );
			m_PropSelectCombo.SetItemData( Index, (u32)i );
		}
	}

	//---	if no prop has yet been set, or a new motion is selected, find
	//		the first active prop for the current motion and select it.
	if( ((pSelectedProp == NULL) || bNewMotionSelected) && m_pView->m_pCharacter->m_pCurMotion )
	{
		//---	find the first active prop in the prop list
		for( i=0; i<MAX_NUM_PROPS; i++ )
			if( (((s32)m_PropSelectCombo.GetItemData(i)) != -1) &&
			    ( m_pView->m_pCharacter->m_pCurMotion->GetProp( (s32)m_PropSelectCombo.GetItemData(i) ).IsFullyActive() ))
				break;

		if( i==MAX_NUM_PROPS )
			Index = 0;
		else
			Index = i;

		pSelectedProp	= (CPropDef*) m_PropSelectCombo.GetItemData( Index );
	}

	//---	look for and set the previously selected prop.  This should automatically call
	//		OnSelchangePropselectcombo() which will call UpdatePropData()
	for( i=0; i<MAX_NUM_PROPS; i++ )
		if( m_PropSelectCombo.GetItemData( i ) == (u32)pSelectedProp )
			m_PropSelectCombo.SetCurSel( i );

	//---	hack to let the dialog controls know that it is the update functions setting them
	//		and not the user.
	s_bInUpdate = FALSE;
}


//==========================================================================================
void CDlgProps::UpdateLinkMotionList( void )
{
	//---	get the prop information
	s32			SelectedProp	= m_PropSelectCombo.GetCurSel();
	s32			PropIndex		= m_PropSelectCombo.GetItemData( SelectedProp );
	CPropDef&	rPropDef		= m_pView->m_pCharacter->GetPropDef( PropIndex );
	CPropData&	rPropData		= m_pView->m_pCharacter->m_pCurMotion->GetProp( PropIndex );

	CCharacter*	pCharacter;
	s32			NumMotions;
	CMotion*	pMotion;
	s32			MotionSelect;
	s32			Index;
	s32			i;

	//---	hack to let the dialog controls know that it is the update functions setting them
	//		and not the user.
	s_bInUpdate = TRUE;

	//---	save the currently selected prop, and reset
	//		the prop select combo box's content.
	m_LinkMotionCombo.ResetContent();

	//---	make the first one the deselection one
	MotionSelect	= 0;
	Index			= m_LinkMotionCombo.AddString( "<no linked motion>" );
	m_LinkMotionCombo.SetItemData( Index, NULL );

	//---	if there is a linked character, refill the list
	pCharacter = rPropData.GetLinkedCharacter();
	if( pCharacter )
	{
		//---	enable the window
		m_LinkMotionCombo.EnableWindow( rPropDef.IsActive() && rPropData.IsFullyActive() );
		m_LinkMotionComboTitle.EnableWindow( rPropDef.IsActive() && rPropData.IsFullyActive() );

		//---	add any active props to the list.  Set the list item's data value
		//		to the props true index.
		NumMotions = pCharacter->GetNumMotions();
		for( i=0; i<NumMotions; i++ )
		{
			pMotion = pCharacter->GetMotion( i );

			//---	add the motion to the list, set the list item's data to a pointer
			//		to the motion.
			Index = m_LinkMotionCombo.AddString( pMotion->GetExportName() );
			m_LinkMotionCombo.SetItemData( Index, (u32)pMotion );
		}

		//---	look for the currently selected motion
		for( i=0; i<NumMotions; i++ )
			if( m_LinkMotionCombo.GetItemData( i ) == (u32)rPropData.GetLinkedMotion() )
			{
				MotionSelect = i;
				break;
			}

		//---	Select the selected motion
		m_LinkMotionCombo.SetCurSel( MotionSelect );
	}
	else
	{
		//---	enable the window
		m_LinkMotionCombo.EnableWindow( FALSE );
		m_LinkMotionComboTitle.EnableWindow( FALSE );
	}

	//---	hack to let the dialog controls know that it is the update functions setting them
	//		and not the user.
	s_bInUpdate = FALSE;
}

//==========================================================================================
void CDlgProps::UpdatePropData( void )
{
	//---	if nothing is selected, disable the entire dialog and go on.
	if( !m_pView ||
		!m_pView->m_pCharacter ||
		!m_pView->m_pCharacter->m_pCurMotion ||
		(m_PropSelectCombo.GetCurSel() == -1) )
	{
		m_LinkMotionCombo.EnableWindow( FALSE );
		m_Export.EnableWindow( FALSE );
		m_Active.EnableWindow( FALSE );
		return;
	}

	//---	get the prop information
	s32			SelectedProp	= m_PropSelectCombo.GetCurSel();
	s32			PropIndex		= m_PropSelectCombo.GetItemData( SelectedProp );
	CPropDef&	rPropDef		= m_pView->m_pCharacter->GetPropDef( PropIndex );
	CPropData&	rPropData		= m_pView->m_pCharacter->m_pCurMotion->GetProp( PropIndex );

	//---	hack to let the dialog controls know that it is the update functions setting them
	//		and not the user.
	s_bInUpdate = TRUE;

	m_Active.SetCheck( rPropData.IsActive() ? TRUE : FALSE );
	m_Active.EnableWindow( rPropDef.IsActive() );
	m_Export.SetCheck( rPropData.IsExported() ? TRUE : FALSE );
	m_Export.EnableWindow( rPropDef.IsActive() && rPropDef.IsExported() && rPropData.IsActive() );
	UpdateLinkMotionList();

	//---	hack to let the dialog controls know that it is the update functions setting them
	//		and not the user.
	s_bInUpdate = FALSE;
}

//==========================================================================================
void CDlgProps::UpdateDetails( bool bNewMotionSelected )
{
	if( !GetSafeHwnd() )
		return;

	UpdatePropList( bNewMotionSelected );
	UpdatePropData();
}


//=====================================================================================================
void CDlgProps::OnActive() 
{
	if( s_bInUpdate )
		return;

	s32			SelectedProp	= m_PropSelectCombo.GetCurSel();
	s32			PropIndex		= m_PropSelectCombo.GetItemData( SelectedProp );
	CPropData&	rProp			= m_pView->m_pCharacter->m_pCurMotion->GetProp( PropIndex );

	rProp.SetActive( m_Active.GetCheck() ? TRUE : FALSE );
	UpdatePropData();

	//---	update the views to show the prop or not
	m_pView->GetDocument()->UpdateAllViews( NULL );
}

//=====================================================================================================
void CDlgProps::OnExport() 
{
	if( s_bInUpdate )
		return;

	s32			SelectedProp	= m_PropSelectCombo.GetCurSel();
	s32			PropIndex		= m_PropSelectCombo.GetItemData( SelectedProp );
	CPropData&	rProp			= m_pView->m_pCharacter->m_pCurMotion->GetProp( PropIndex );

	rProp.SetExported( m_Export.GetCheck() ? TRUE : FALSE );
	UpdatePropData();
}

//=====================================================================================================
void CDlgProps::OnSelchangeLinkmotion() 
{
	if( s_bInUpdate )
		return;

	s32			SelectedProp	= m_PropSelectCombo.GetCurSel();
	s32			PropIndex		= m_PropSelectCombo.GetItemData( SelectedProp );
	CPropData&	rProp			= m_pView->m_pCharacter->m_pCurMotion->GetProp( PropIndex );

	s32			MotionIndex		= m_LinkMotionCombo.GetCurSel();
	ASSERT( MotionIndex != -1 );
	rProp.SetLinkedMotion( (CMotion*)m_LinkMotionCombo.GetItemData( MotionIndex ) );

	//---	update the views to show the prop or not
	m_pView->GetDocument()->UpdateAllViews( NULL );
}

//=====================================================================================================
void CDlgProps::OnSelchangePropselect() 
{
	UpdatePropData();
}

//=====================================================================================================
void CDlgProps::OnEditdef() 
{
	m_pView->DisplayPropDefs( TRUE, m_PropSelectCombo.GetItemData( m_PropSelectCombo.GetCurSel() ) );
}
