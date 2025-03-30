// DlgSkelPointList.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "DlgSkelPointList.h"
#include "x_types.h"
#include "MotionEditView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgSkelPointList dialog


CDlgSkelPointList::CDlgSkelPointList(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSkelPointList::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgSkelPointList)
	//}}AFX_DATA_INIT
	m_CheckLocked = false ;
}


void CDlgSkelPointList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSkelPointList)
	DDX_Control(pDX, IDC_DELETE, m_Delete);
	DDX_Control(pDX, IDC_ADD, m_Add);
	DDX_Control(pDX, IDC_LIST, m_List);
	DDX_Control(pDX, IDC_Z, m_Z);
	DDX_Control(pDX, IDC_Y, m_Y);
	DDX_Control(pDX, IDC_X, m_X);
	DDX_Control(pDX, IDC_RADIUS, m_R);
	DDX_Control(pDX, IDC_BONE, m_Bone);
	//}}AFX_DATA_MAP
}


int CDlgSkelPointList::GetSelectedMotionsSkelPointCheck( int Index )
{
	int State			= 2 ;
	CListBox *pList	= NULL ;

	//---	Get Pointer to Selection List
	if( m_pView )
	{
		CMotionEditFrame *pFrame = m_pView->GetFrame() ;
		pList = &pFrame->m_DlgMotionList.m_List ;
	}

	//---	Loop through all Selected Items
	if( pList )
	{
		for( int i = 0 ; i < pList->GetCount() ; i++ )
		{
			if( pList->GetSel( i ) )
			{
				CMotion *pMotion = (CMotion*)pList->GetItemData( i ) ;
				if( pMotion )
				{
					if( pMotion->SkelPointGetCheck( Index ) )
					{
						if( State == 0 )
						{
							State = 2 ;
							break ;
						}
						State = 1 ;
					}
					else
					{
						if( State == 1 )
						{
							State = 2 ;
							break ;
						}
						State = 0 ;
					}
				}
			}
		}
	}

	return State ;
}

void CDlgSkelPointList::SetSelectedMotionsSkelPointCheck( int Index, BOOL State )
{
	CListBox *pList	= NULL ;

	//---	Get Pointer to Selection List
	if( m_pView )
	{
		CMotionEditFrame *pFrame = m_pView->GetFrame() ;
		pList = &pFrame->m_DlgMotionList.m_List ;
	}

	//---	Loop through all Selected Items
	if( pList )
	{
		for( int i = 0 ; i < pList->GetCount() ; i++ )
		{
			if( pList->GetSel( i ) )
			{
				CMotion *pMotion = (CMotion*)pList->GetItemData( i ) ;
				if( pMotion )
					pMotion->SkelPointSetCheck( Index, State ) ;
			}
		}
	}
}

void CDlgSkelPointList::UpdateList( CCharacter *pCharacter )
{
	CMotion *pMotion = NULL ;
	if( pCharacter )
		pMotion = pCharacter->GetCurMotion() ;

	//---	Reset the List
	m_List.DeleteAllItems() ;
//	m_List.InsertItem( 0, "<no skeleton point>" ) ;
//	m_List.SetItemData( 0, NULL ) ;
	
	m_List.EnableWindow( (pCharacter != NULL) ) ;

	//---	Set Checkboxes in List
	if( pMotion )
		ListView_SetExtendedListViewStyleEx( m_List.m_hWnd, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES ) ;
	else
		ListView_SetExtendedListViewStyleEx( m_List.m_hWnd, LVS_EX_CHECKBOXES, 0 ) ;

	//---	Add All Skel Points
	if( pCharacter )
	{
		CSkelPointList *pSPList = &pCharacter->m_SkelPointList ;
		POSITION Pos = pSPList->GetHeadPosition( ) ;
		while( Pos )
		{
			CSkelPoint *pSkelPoint = pSPList->GetNext( Pos ) ;
			ASSERT( pSkelPoint ) ;
			CString Name ;
			Name.Format( "%s", pSkelPoint->GetName() ) ;
			int Index = m_List.InsertItem( m_List.GetItemCount(), Name ) ;
			m_List.SetItemData( Index, (DWORD)pSkelPoint ) ;

			if( pMotion )
			{
				int		State = GetSelectedMotionsSkelPointCheck( Index ) ;

				if( State < 2 )
				{
					m_CheckLocked = true ;
					ListView_SetItemState (m_List.m_hWnd, Index, UINT((State + 1) << 12), LVIS_STATEIMAGEMASK ) ;
					m_CheckLocked = false ;
				}
				else
				{
					m_CheckLocked = true ;
					ListView_SetItemState (m_List.m_hWnd, Index, UINT((3) << 12), LVIS_STATEIMAGEMASK ) ;
					m_CheckLocked = false ;
				}
			}
		}

		//---	Set Current Selection
		int CurrentSel = 0 ;
		CurrentSel = MFCU_ListCtrlFind( m_List, (DWORD)pCharacter->GetCurSkelPoint() ) ;
		if( CurrentSel == -1 ) CurrentSel = 0 ;
		m_List.SetItemState( CurrentSel, LVIS_SELECTED, LVIS_SELECTED ) ;
		m_List.EnsureVisible( CurrentSel, false ) ;
		m_List.RedrawWindow( ) ;
	}

	//---	Redraw Control
	m_List.RedrawWindow() ;
}

void CDlgSkelPointList::UpdateControls( CCharacter *pCharacter )
{
	vector3d	v ;
	f32			Radius = 0 ;
	CString		Name ;

	V3_Set( &v, 0, 0, 0 ) ;

	bool Enable = false ;

	if( pCharacter && (pCharacter->GetCurMotion() == NULL) )
	{
		CSkelPoint *pSkelPoint = pCharacter->GetCurSkelPoint() ;
		if( pSkelPoint )
		{
			pSkelPoint->GetPosition( &v ) ;
			Radius = pSkelPoint->GetRadius() ;
			Name = pSkelPoint->GetName() ;
			Enable = true ;
		}
	}

	//---	Enable / Disable Controls
	m_X.EnableWindow( Enable ) ;
	m_Y.EnableWindow( Enable ) ;
	m_Z.EnableWindow( Enable ) ;
	m_R.EnableWindow( Enable ) ;
	m_Bone.EnableWindow( Enable && pCharacter && pCharacter->GetSkeleton() ) ;
//	m_Add.EnableWindow( Enable ) ;
//	m_Delete.EnableWindow( Enable ) ;

	//---	Set Strings into Controls
	m_EditsLocked = true ;
	MFCU_UpdateEditControl( m_X, v.X ) ;
	MFCU_UpdateEditControl( m_Y, v.Y ) ;
	MFCU_UpdateEditControl( m_Z, v.Z ) ;
	MFCU_UpdateEditControl( m_R, Radius ) ;
	m_EditsLocked = false ;

	//---	Reset the Combo & Add all Bones
	m_Bone.ResetContent( ) ;
	m_Bone.AddString( "<no bone>" ) ;
	if( pCharacter != NULL )
	{
		CSkeleton *pSkel = pCharacter->GetSkeleton() ;
		if( pSkel )
		{
			for( int i = 0 ; i < pSkel->GetNumBones() ; i++ )
			{
				CString	s ;
				for( int j = 0 ; j < pSkel->GetBoneNumParents( pSkel->GetBoneFromIndex(i) ) ; j++ )
				{
					s += "  " ;
				}
				s += pSkel->GetBoneFromIndex(i)->BoneName ;
				m_Bone.AddString( s ) ;
			}
		}
		CSkelPoint *pSkelPoint = pCharacter->GetCurSkelPoint() ;
		if( pSkelPoint )
		{
			m_Bone.SetCurSel( pSkelPoint->GetBone() ) ;
		}
	}

	//---	Force Redraw
	m_X.RedrawWindow( ) ;
	m_Y.RedrawWindow( ) ;
	m_Z.RedrawWindow( ) ;
	m_R.RedrawWindow( ) ;
	m_Bone.RedrawWindow( ) ;
	m_Add.RedrawWindow( ) ;
	m_Delete.RedrawWindow( ) ;
}






BEGIN_MESSAGE_MAP(CDlgSkelPointList, CDialog)
	//{{AFX_MSG_MAP(CDlgSkelPointList)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_CBN_SELCHANGE(IDC_BONE, OnSelchangeBone)
	ON_EN_CHANGE(IDC_RADIUS, OnChangeRadius)
	ON_EN_CHANGE(IDC_X, OnChangeX)
	ON_EN_CHANGE(IDC_Y, OnChangeY)
	ON_EN_CHANGE(IDC_Z, OnChangeZ)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemchangedList)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST, OnEndlabeleditList)
	//}}AFX_MSG_MAP

	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDCANCEL, OnCancel)

	ON_MESSAGE( MSM_MOUSEMOVE, OnMouseStaticMove )

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSkelPointList message handlers

void CDlgSkelPointList::OnOk( )
{
}

void CDlgSkelPointList::OnCancel( )
{
}

LONG CDlgSkelPointList::OnMouseStaticMove( UINT wParam, LONG lParam )
{
	f32		dx = (f32)( (s16)((lParam>>16)&0xffff)) ;
	f32		dy = (f32)(-(s16)(lParam&0xffff)) ;

	dx /= 10 ;
	dy /= 10 ;

	ASSERT( m_pView ) ;

	switch( wParam )
	{
		case IDC_STATIC_X:
			m_pView->ModifySkelPointPosition( dx+dy, 0, 0 ) ;
			break ;
		case IDC_STATIC_Y:
			m_pView->ModifySkelPointPosition( 0, dx+dy, 0 ) ;
			break ;
		case IDC_STATIC_Z:
			m_pView->ModifySkelPointPosition( 0, 0, dx+dy ) ;
			break ;
		case IDC_STATIC_R:
			m_pView->ModifySkelPointRadius( dx+dy ) ;
			break ;
	}

	return 0 ;
}

BOOL CDlgSkelPointList::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	m_StaticX.SubclassDlgItem( IDC_STATIC_X, this ) ;
	m_StaticX.SetNotifyWindow( this ) ;

	m_StaticY.SubclassDlgItem( IDC_STATIC_Y, this ) ;
	m_StaticY.SetNotifyWindow( this ) ;
	
	m_StaticZ.SubclassDlgItem( IDC_STATIC_Z, this ) ;
	m_StaticZ.SetNotifyWindow( this ) ;
	
	m_StaticR.SubclassDlgItem( IDC_STATIC_R, this ) ;
	m_StaticR.SetNotifyWindow( this ) ;

	m_List.InsertColumn( 0, "Points", LVCFMT_LEFT, 1024 ) ;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgSkelPointList::OnAdd() 
{
	// TODO: Add your control notification handler code here
	
	ASSERT( m_pView ) ;
	m_pView->SkelPointAdd( ) ;
}

void CDlgSkelPointList::OnDelete() 
{
	// TODO: Add your control notification handler code here
	ASSERT( m_pView ) ;
	
	CSkelPoint *pSkelPoint = (CSkelPoint *)m_List.GetItemData( MFCU_ListCtrlGetSelectedIndex( m_List ) ) ;
	if( pSkelPoint )
	{
		CSkelPoint *pNewCurrentSkelPoint = (CSkelPoint*)m_List.GetItemData( min(m_List.GetItemCount()-1, MFCU_ListCtrlGetSelectedIndex( m_List )+1) ) ;
		if( pNewCurrentSkelPoint == pSkelPoint )
			pNewCurrentSkelPoint = (CSkelPoint*)m_List.GetItemData( max(0, MFCU_ListCtrlGetSelectedIndex( m_List )-1) ) ;
		m_pView->SkelPointDelete( pSkelPoint, pNewCurrentSkelPoint ) ;
	}
}

void CDlgSkelPointList::OnSelchangeBone() 
{
	// TODO: Add your control notification handler code here
	
	CSkelPoint *pSkelPoint = (CSkelPoint *)m_List.GetItemData( MFCU_ListCtrlGetSelectedIndex( m_List ) ) ;
	if( pSkelPoint )
	{
		pSkelPoint->SetBone( m_Bone.GetCurSel() ) ;
	}
}

void CDlgSkelPointList::OnChangeRadius() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_EditsLocked )
	{
		CSkelPoint *pSkelPoint = (CSkelPoint *)MFCU_ListCtrlGetSelected( m_List ) ;
		if( pSkelPoint )
		{
			CString		String ;
			m_R.GetWindowText( String ) ;
			pSkelPoint->SetRadius( (f32)atof( String ) ) ;
			m_pView->GetDocument()->SetModifiedFlag( TRUE ) ;
			m_pView->Invalidate() ;
		}
	}
}

void CDlgSkelPointList::OnChangeX() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_EditsLocked )
	{
		CSkelPoint *pSkelPoint = (CSkelPoint *)MFCU_ListCtrlGetSelected( m_List ) ;
		if( pSkelPoint )
		{
			vector3d	Position ;
			pSkelPoint->GetPosition( &Position ) ;

			CString		String ;
			m_X.GetWindowText( String ) ;

			Position.X = (f32)atof( String ) ;
			pSkelPoint->SetPosition( &Position ) ;
			m_pView->GetDocument()->SetModifiedFlag( TRUE ) ;
			m_pView->Invalidate() ;
		}
	}
}

void CDlgSkelPointList::OnChangeY() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_EditsLocked )
	{
		CSkelPoint *pSkelPoint = (CSkelPoint *)MFCU_ListCtrlGetSelected( m_List ) ;
		if( pSkelPoint )
		{
			vector3d	Position ;
			pSkelPoint->GetPosition( &Position ) ;

			CString		String ;
			m_Y.GetWindowText( String ) ;

			Position.Y = (f32)atof( String ) ;
			pSkelPoint->SetPosition( &Position ) ;
			m_pView->GetDocument()->SetModifiedFlag( TRUE ) ;
			m_pView->Invalidate() ;
		}
	}
}

void CDlgSkelPointList::OnChangeZ() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_EditsLocked )
	{
		CSkelPoint *pSkelPoint = (CSkelPoint *)MFCU_ListCtrlGetSelected( m_List ) ;
		if( pSkelPoint )
		{
			vector3d	Position ;
			pSkelPoint->GetPosition( &Position ) ;

			CString		String ;
			m_Z.GetWindowText( String ) ;

			Position.Z = (f32)atof( String ) ;
			pSkelPoint->SetPosition( &Position ) ;
			m_pView->GetDocument()->SetModifiedFlag( TRUE ) ;
			m_pView->Invalidate() ;
		}
	}
}

void CDlgSkelPointList::OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	*pResult = 0;
	
	int Index = pNMListView->iItem ;
	if( Index != -1 )
	{
		CSkelPoint *pSkelPoint = (CSkelPoint*)MFCU_ListCtrlGetSelected( m_List ) ;
		m_pView->SkelPointSetCurrent( pSkelPoint ) ;
	}

	if( !m_CheckLocked )
	{
		if (pNMListView->uOldState == 0 && pNMListView->uNewState == 0)
			return;	// No change
		BOOL bPrevState = (BOOL)(((pNMListView->uOldState & LVIS_STATEIMAGEMASK)>>12)-1) ;				// Old check box state
		if( bPrevState < 0 )																			// On startup there's no previous state 
			bPrevState = 0 ;																			// so assign as false (unchecked)
		BOOL bChecked=(BOOL)(((pNMListView->uNewState & LVIS_STATEIMAGEMASK)>>12)-1) ;					// New check box state
		if( bChecked < 0 )																				// On non-checkbox notifications assume false
			bChecked = 0 ;
		if (bPrevState == bChecked)																		// No change in check box
			return ;

		//---	Update Check State
		if( m_pView && m_pView->m_pCharacter && m_pView->GetCurMotion() )
		{
			SetSelectedMotionsSkelPointCheck( Index, bChecked ) ;
		}
	}
}

void CDlgSkelPointList::OnEndlabeleditList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0 ;

	CSkelPoint *pSkelPoint = (CSkelPoint*)m_List.GetItemData( pDispInfo->item.iItem ) ;
	if( pSkelPoint )
	{
		if( pDispInfo->item.pszText )
		{
			pSkelPoint->SetName( pDispInfo->item.pszText ) ;
			CString String ;
			m_List.SetItemText( pDispInfo->item.iItem, 0, pDispInfo->item.pszText ) ;

			m_pView->GetDocument()->SetModifiedFlag( ) ;
			*pResult = 0 ;
		}
	}
}
