// DlgKey.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "DlgKey.h"
#include "Actor.h"
#include "MovieView.h"
#include "MFCutils.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgKey dialog


CDlgKey::CDlgKey(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgKey::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgKey)
	m_KeyMode = -1;
	m_BoolAutoLink = FALSE;
	m_BoolVisible = FALSE;
	m_BoolCameraLock = FALSE;
	m_BoolAutoLinkDir = FALSE;
	//}}AFX_DATA_INIT

	m_pView = NULL ;
	m_pActor = NULL ;
	m_Updating = false ;
}


void CDlgKey::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgKey)
	DDX_Control(pDX, IDC_AUTOLINKDIR, m_AutoLinkDir);
	DDX_Control(pDX, IDC_MIRROR, m_Mirror);
	DDX_Control(pDX, IDC_RATE, m_FrameRate);
	DDX_Control(pDX, IDC_BLEND, m_Blend);
	DDX_Control(pDX, IDC_EVENT, m_Event);
	DDX_Control(pDX, IDC_CAMERALOCK, m_CameraLock);
	DDX_Control(pDX, IDC_VISIBLE, m_Visible);
	DDX_Control(pDX, IDC_SCALE, m_Scale);
	DDX_Control(pDX, IDC_RZ, m_RZ);
	DDX_Control(pDX, IDC_RY, m_RY);
	DDX_Control(pDX, IDC_RX, m_RX);
	DDX_Control(pDX, IDC_AUTOLINK, m_AutoLink);
	DDX_Control(pDX, IDC_MOTION, m_Motion);
	DDX_Control(pDX, IDC_RADIO1, m_Radio1);
	DDX_Control(pDX, IDC_RADIO2, m_Radio2);
	DDX_Control(pDX, IDC_STARTFRAME, m_StartFrame);
	DDX_Control(pDX, IDC_NUMFRAMES, m_NumFrames);
	DDX_Control(pDX, IDC_FRAME, m_Frame);
	DDX_Control(pDX, IDC_VZ, m_VZ);
	DDX_Control(pDX, IDC_VY, m_VY);
	DDX_Control(pDX, IDC_VX, m_VX);
	DDX_Control(pDX, IDC_Z, m_Z);
	DDX_Control(pDX, IDC_Y, m_Y);
	DDX_Control(pDX, IDC_X, m_X);
	DDX_Control(pDX, IDC_LIST, m_List);
	DDX_Radio(pDX, IDC_RADIO1, m_KeyMode);
	DDX_Check(pDX, IDC_AUTOLINK, m_BoolAutoLink);
	DDX_Check(pDX, IDC_VISIBLE, m_BoolVisible);
	DDX_Check(pDX, IDC_CAMERALOCK, m_BoolCameraLock);
	DDX_Check(pDX, IDC_AUTOLINKDIR, m_BoolAutoLinkDir);
	//}}AFX_DATA_MAP
}




void CDlgKey::SetActor( CActor *pActor )
{
	m_pActor = pActor ;

	if( ::IsWindow( GetSafeHwnd() ) )
	{
		m_List.DeleteAllItems( ) ;
		if( pActor )
		{
			POSITION Pos = pActor->m_Keys.GetHeadPosition( ) ;
			while( Pos )
			{
				CKey *pKey = pActor->m_Keys.GetNext( Pos ) ;

				CString Name ;
				Name.Format( "%04d- %s", pKey->GetFrame(), pKey->GetName() ) ;
				int iItem = m_List.GetItemCount( ) ;
				int Index = m_List.InsertItem( iItem, Name ) ;
				if( Index != -1 )
				{
					VERIFY( m_List.SetItemData( Index, (DWORD)pKey ) ) ;
				}
			}

			SelectKey( pActor->GetCurKey() ) ;
		}
	}
}

void CDlgKey::SetupMotions( CCharacter *pCharacter )
{
	if( ::IsWindow( GetSafeHwnd() ) )
	{
		m_Motion.ResetContent( ) ;
		int Item = m_Motion.AddString( "<no motion>" ) ;
		m_Motion.SetItemData( Item, 0 ) ;

		if( pCharacter )
		{
			for( int i = 0 ; i < pCharacter->GetNumMotions() ; i++ )
			{
				CMotion *pMotion = pCharacter->GetMotion(i) ;
				char FileName[_MAX_FNAME] ;
				char Ext[_MAX_EXT] ;
				_splitpath( pMotion->GetPathName(), NULL, NULL, FileName, Ext ) ;
				CString Name = FileName ;
				Name += Ext ;
				int Index = m_Motion.AddString( Name ) ;
				m_Motion.SetItemData( Index, (DWORD)pMotion ) ;
			}
		}
	}
}

void CDlgKey::SelectMotion( CMotion *pMotion )
{
	for( int i = 0 ; i < m_Motion.GetCount() ; i++ )
	{
		if( m_Motion.GetItemData( i ) == (DWORD)pMotion )
		{
			m_Motion.SetCurSel( i ) ;
			break ;
		}
	}
}

void CDlgKey::SelectKey( CKey *pKey )
{
	if( ::IsWindow( GetSafeHwnd() ) )
	{
		LV_FINDINFO	fi ;

		fi.flags = LVFI_PARAM ;
		fi.lParam = (DWORD)pKey ;

		int Index = m_List.FindItem( &fi, -1 ) ;
		if( Index != -1 )
		{
			m_List.SetItemState( Index, LVIS_SELECTED, LVIS_SELECTED ) ;
			m_List.EnsureVisible( Index, false ) ;
			m_List.RedrawWindow( ) ;
		}
		UpdateDetails( pKey ) ;
	}
}

void CDlgKey::UpdateDetails( CKey *pKey )
{
	if( ::IsWindow( GetSafeHwnd() ) )
	{
		bool	Enable = (pKey != NULL) ;
		bool	Enable2 = Enable && (m_pActor != NULL) ? (m_pActor->GetType() == ACTOR_CHARACTER) : false ;

		m_X.EnableWindow( Enable ) ;
		m_Y.EnableWindow( Enable ) ;
		m_Z.EnableWindow( Enable ) ;
		m_VX.EnableWindow( Enable ) ;
		m_VY.EnableWindow( Enable ) ;
		m_VZ.EnableWindow( Enable ) ;
		m_RX.EnableWindow( Enable ) ;
		m_RY.EnableWindow( Enable ) ;
		m_RZ.EnableWindow( Enable ) ;
		m_Frame.EnableWindow( Enable ) ;
		m_Event.EnableWindow( Enable ) ;

		m_Radio1.EnableWindow( Enable2 ) ;
		m_Radio2.EnableWindow( Enable2 ) ;
		m_AutoLink.EnableWindow( Enable2 ) ;
		m_AutoLinkDir.EnableWindow( Enable2 && (pKey && pKey->m_AMCbased) ) ;
		m_Motion.EnableWindow( Enable2 ) ;
		m_StartFrame.EnableWindow( Enable2 ) ;
		m_FrameRate.EnableWindow( Enable2 ) ;
		m_Blend.EnableWindow( Enable2 ) ;
		m_Mirror.EnableWindow( Enable2 ) ;

		bool	OldUpdating = m_Updating ;
		m_Updating = true ;

		if( pKey )
		{
			CMotion *pMotion = pKey->GetMotion( ) ;
			int nFrames = ((pMotion != NULL) && (m_pActor->m_pCharacter != NULL)) ? (pMotion->GetNumFrames()-1) : 0 ;
			vector3  v, vv, rv ;
			pKey->GetPosition( &v ) ;
			pKey->GetVector( &vv ) ;
			pKey->GetRotation( &rv ) ;
			MFCU_UpdateEditControl( m_X, v.X ) ;
			MFCU_UpdateEditControl( m_Y, v.Y ) ;
			MFCU_UpdateEditControl( m_Z, v.Z ) ;
			MFCU_UpdateEditControl( m_VX, vv.X ) ;
			MFCU_UpdateEditControl( m_VY, vv.Y ) ;
			MFCU_UpdateEditControl( m_VZ, vv.Z ) ;
			MFCU_UpdateEditControl( m_RX, rv.X ) ;
			MFCU_UpdateEditControl( m_RY, rv.Y ) ;
			MFCU_UpdateEditControl( m_RZ, rv.Z ) ;
			MFCU_UpdateEditControl( m_Frame, pKey->GetFrame() ) ;
			MFCU_UpdateEditControl( m_StartFrame, pKey->GetStartFrame() ) ;
			MFCU_UpdateEditControl( m_FrameRate, pKey->GetFrameRate() ) ;
			MFCU_UpdateEditControl( m_Blend, pKey->GetBlendFrames() ) ;
			m_Mirror.SetCheck( pKey->GetMirror() ) ;
			MFCU_UpdateEditControl( m_NumFrames, nFrames ) ;
			SelectMotion( pKey->GetMotion() ) ;
			m_KeyMode = pKey->m_AMCbased ? 1 : 0 ;
			m_BoolAutoLink = pKey->m_AutoLinkPos ;
			m_BoolAutoLinkDir = pKey->m_AutoLinkDir ;
			m_BoolVisible = pKey->m_Visible;
			m_BoolCameraLock = pKey->m_CameraLock;
			m_Event.SetCurSel (pKey->m_Event);

			UpdateData( FALSE ) ;
			if (m_pActor->GetType() == ACTOR_SCENERY)
			{
				MFCU_UpdateEditControl( m_Scale, pKey->GetScale () );
				m_Scale.EnableWindow(true);
				m_Visible.EnableWindow(true);
				m_CameraLock.EnableWindow(true);
			}
			else
			{
				MFCU_UpdateEditControl( m_Scale, "N/A" );
				m_Scale.EnableWindow(false);
				m_Visible.EnableWindow(false);
				m_CameraLock.EnableWindow(false);
			}
		}
		else
		{
			MFCU_UpdateEditControl( m_X, 0 ) ;
			MFCU_UpdateEditControl( m_Y, 0 ) ;
			MFCU_UpdateEditControl( m_Z, 0 ) ;
			MFCU_UpdateEditControl( m_VX, 0 ) ;
			MFCU_UpdateEditControl( m_VY, 0 ) ;
			MFCU_UpdateEditControl( m_VZ, 0 ) ;
			MFCU_UpdateEditControl( m_RX, 0 ) ;
			MFCU_UpdateEditControl( m_RY, 0 ) ;
			MFCU_UpdateEditControl( m_RZ, 0 ) ;
			MFCU_UpdateEditControl( m_Frame, 0 ) ;
			MFCU_UpdateEditControl( m_StartFrame, 0 ) ;
			MFCU_UpdateEditControl( m_FrameRate, 0 ) ;
			MFCU_UpdateEditControl( m_Blend, 0 ) ;
			m_Mirror.SetCheck( FALSE ) ;
			MFCU_UpdateEditControl( m_NumFrames, 0 ) ;
			MFCU_UpdateEditControl( m_Scale, "n/a" );
			SelectMotion( NULL ) ;
			m_KeyMode = 0 ;
			m_BoolAutoLink = false ;
			m_BoolAutoLinkDir = false;
			m_BoolVisible = true;
			m_Scale.EnableWindow( false );
			m_Visible.EnableWindow( false );
			m_CameraLock.EnableWindow( false );
			m_Event.SetCurSel(0);
			UpdateData( FALSE ) ;
		}

		m_X.RedrawWindow( ) ;
		m_Y.RedrawWindow( ) ;
		m_Z.RedrawWindow( ) ;
		m_VX.RedrawWindow( ) ;
		m_VY.RedrawWindow( ) ;
		m_VZ.RedrawWindow( ) ;
		m_RX.RedrawWindow( ) ;
		m_RY.RedrawWindow( ) ;
		m_RZ.RedrawWindow( ) ;
		m_Frame.RedrawWindow( ) ;
		m_Event.RedrawWindow( ) ;
		m_StartFrame.RedrawWindow( ) ;
		m_FrameRate.RedrawWindow( ) ;
		m_Blend.RedrawWindow( ) ;
		m_Mirror.RedrawWindow( ) ;
		m_Motion.RedrawWindow( ) ;
		m_Radio1.RedrawWindow( ) ;
		m_Radio2.RedrawWindow( ) ;
		m_AutoLink.RedrawWindow( ) ;
		m_AutoLinkDir.RedrawWindow( ) ;
		m_Scale.RedrawWindow();
		m_Visible.RedrawWindow();
		m_CameraLock.RedrawWindow();

		m_Updating = OldUpdating ;
	}
}





BEGIN_MESSAGE_MAP(CDlgKey, CDialog)
	//{{AFX_MSG_MAP(CDlgKey)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DEL, OnDel)
	ON_BN_CLICKED(IDC_SELECT_ALL, OnSelectAll)
	ON_BN_CLICKED(IDC_SELECT_NONE, OnSelectNone)
	ON_BN_CLICKED(IDC_AUTOLINK, OnAutolink)
	ON_EN_CHANGE(IDC_FRAME, OnChangeFrame)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemchangedList)
	ON_CBN_SELCHANGE(IDC_MOTION, OnSelchangeMotion)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_EN_CHANGE(IDC_STARTFRAME, OnChangeStartframe)
	ON_EN_CHANGE(IDC_X, OnChangeX)
	ON_EN_CHANGE(IDC_Y, OnChangeY)
	ON_EN_CHANGE(IDC_Z, OnChangeZ)
	ON_EN_CHANGE(IDC_VX, OnChangeVx)
	ON_EN_CHANGE(IDC_VY, OnChangeVy)
	ON_EN_CHANGE(IDC_VZ, OnChangeVz)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDblclkList)
	ON_EN_CHANGE(IDC_RX, OnChangeRx)
	ON_EN_CHANGE(IDC_RY, OnChangeRy)
	ON_EN_CHANGE(IDC_RZ, OnChangeRz)
	ON_EN_CHANGE(IDC_SCALE, OnChangeScale)
	ON_BN_CLICKED(IDC_VISIBLE, OnVisible)
	ON_BN_CLICKED(IDC_CAMERALOCK, OnCameralock)
	ON_CBN_SELCHANGE(IDC_EVENT, OnSelchangeEvent)
	ON_EN_CHANGE(IDC_RATE, OnChangeRate)
	ON_EN_CHANGE(IDC_BLEND, OnChangeBlend)
	ON_BN_CLICKED(IDC_MIRROR, OnMirror)
	ON_BN_CLICKED(IDC_AUTOLINKDIR, OnAutolinkdir)
	//}}AFX_MSG_MAP

	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDCANCEL, OnCancel)

	ON_MESSAGE( MSM_MOUSEMOVE, OnMouseStaticMove )

	END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgKey message handlers

BOOL CDlgKey::OnInitDialog() 
{
	CDialog::OnInitDialog();

	//---	Add Columns to ListCtrl
	CRect r ;
	m_List.GetClientRect( &r ) ;
	m_List.InsertColumn( 0, "Key Name", LVCFMT_LEFT, r.Width()-16, -1 ) ;

	//---	SubClass all MouseStatic controls
	m_StaticX.SubclassDlgItem( IDC_MOVE_X, this ) ;
	m_StaticX.SetNotifyWindow( this ) ;
	m_StaticY.SubclassDlgItem( IDC_MOVE_Y, this ) ;
	m_StaticY.SetNotifyWindow( this ) ;
	m_StaticZ.SubclassDlgItem( IDC_MOVE_Z, this ) ;
	m_StaticZ.SetNotifyWindow( this ) ;
	m_StaticVX.SubclassDlgItem( IDC_MOVE_VX, this ) ;
	m_StaticVX.SetNotifyWindow( this ) ;
	m_StaticVY.SubclassDlgItem( IDC_MOVE_VY, this ) ;
	m_StaticVY.SetNotifyWindow( this ) ;
	m_StaticVZ.SubclassDlgItem( IDC_MOVE_VZ, this ) ;
	m_StaticVZ.SetNotifyWindow( this ) ;
	m_StaticRX.SubclassDlgItem( IDC_MOVE_RX, this ) ;
	m_StaticRX.SetNotifyWindow( this ) ;
	m_StaticRY.SubclassDlgItem( IDC_MOVE_RY, this ) ;
	m_StaticRY.SetNotifyWindow( this ) ;
	m_StaticRZ.SubclassDlgItem( IDC_MOVE_RZ, this ) ;
	m_StaticRZ.SetNotifyWindow( this ) ;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgKey::OnOk() 
{
}

void CDlgKey::OnCancel() 
{
}

void CDlgKey::OnAdd() 
{
	// TODO: Add your control notification handler code here
	if( m_pView )
		m_pView->KeyAdd( ) ;
}

void CDlgKey::OnDel() 
{
	// TODO: Add your control notification handler code here
	if( m_pView && ::IsWindow(GetSafeHwnd()) )
	{
		CKey *pKey = (CKey*)MFCU_ListCtrlGetSelected( m_List ) ;
		if( pKey )
		{
			CKey *pNewKey = (CKey*)MFCU_ListCtrlGetNext( m_List, pKey ) ;
			if( !pNewKey )
				pNewKey = (CKey*)MFCU_ListCtrlGetPrev( m_List, pKey ) ;

			m_pView->KeyDelete( pKey, pNewKey ) ;
		}
	}
}

void CDlgKey::OnSelectAll() 
{
	// TODO: Add your control notification handler code here
	
	if( m_pView )
		m_pView->KeySelectAll( true ) ;
}

void CDlgKey::OnSelectNone() 
{
	// TODO: Add your control notification handler code here
	
	if( m_pView )
		m_pView->KeySelectAll( false ) ;
}

void CDlgKey::OnAutolink() 
{
	// TODO: Add your control notification handler code here
	
	if( m_pView && ::IsWindow(GetSafeHwnd()) )
	{
		CKey *pKey = (CKey*)MFCU_ListCtrlGetSelected( m_List ) ;
		if( pKey )
		{
			pKey->m_AutoLinkPos = !!m_AutoLink.GetCheck() ;
			if( m_pActor ) m_pActor->CalculateSplineCache( ) ;
			m_pView->Invalidate() ;
		}
	}
}

void CDlgKey::OnAutolinkdir() 
{
	// TODO: Add your control notification handler code here
	
	if( m_pView && ::IsWindow(GetSafeHwnd()) )
	{
		CKey *pKey = (CKey*)MFCU_ListCtrlGetSelected( m_List ) ;
		if( pKey )
		{
			m_pView->KeySetAutoLinkDir( !!m_AutoLinkDir.GetCheck() );
			if( m_pActor ) m_pActor->CalculateSplineCache( ) ;
			m_pView->Invalidate() ;
		}
	}

}

void CDlgKey::OnChangeFrame() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	if( m_pView && ::IsWindow(GetSafeHwnd()) && ::IsWindow(m_Frame.GetSafeHwnd()) )
	{
		if( !m_Updating )
		{
			ASSERT( m_pView ) ;
			CString s ;
			m_Frame.GetWindowText( s ) ;
			m_pView->KeySetFrame( atoi(s) ) ;
		}
	}
}

void CDlgKey::OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	if( m_pView && ::IsWindow(GetSafeHwnd()) )
	{
		UpdateData( true ) ;
		CKey *pKey = (CKey*)MFCU_ListCtrlGetSelected( m_List ) ;
		if( pKey )
			m_pView->KeySetCurrent( pKey ) ;
	}

	*pResult = 0;
}

void CDlgKey::OnSelchangeMotion() 
{
	// TODO: Add your control notification handler code here
	
	if( m_pView && ::IsWindow(GetSafeHwnd()) )
	{
		UpdateData( true ) ;
		CKey *pKey = (CKey*)MFCU_ListCtrlGetSelected( m_List ) ;
		if( pKey )
		{
			CMotion *pMotion = (CMotion*)m_Motion.GetItemData(m_Motion.GetCurSel()) ;
			m_pView->KeySetMotion( pMotion ) ;
		}
	}
}

void CDlgKey::OnSelchangeEvent() 
{
	// TODO: Add your control notification handler code here
	
	if( m_pView && ::IsWindow(GetSafeHwnd()) )
	{
		CKey *pKey = (CKey*)MFCU_ListCtrlGetSelected( m_List ) ;
		if( pKey )
		{
			CString String ;
			pKey->SetEvent( m_Event.GetCurSel() );
		}
	}
}

void CDlgKey::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	
	if( m_pView && ::IsWindow(GetSafeHwnd()) )
	{
		UpdateData( true ) ;
		CKey *pKey = (CKey*)MFCU_ListCtrlGetSelected( m_List ) ;
		if( pKey )
		{
			pKey->m_AMCbased = false ;
			m_AutoLinkDir.EnableWindow( FALSE );
			if( m_pActor ) m_pActor->CalculateSplineCache( ) ;
			m_pView->Invalidate() ;
		}
	}
}

void CDlgKey::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	
	if( m_pView && ::IsWindow(GetSafeHwnd()) )
	{
		UpdateData( true ) ;
		CKey *pKey = (CKey*)MFCU_ListCtrlGetSelected( m_List ) ;
		if( pKey )
		{
			pKey->m_AMCbased = true ;
			m_AutoLinkDir.EnableWindow( TRUE );
			if( m_pActor ) m_pActor->CalculateSplineCache( ) ;
			m_pView->Invalidate() ;
		}
	}
}

void CDlgKey::OnChangeStartframe() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_Updating )
	{
		ASSERT( m_pView ) ;
		CString s ;
		m_StartFrame.GetWindowText( s ) ;
		m_pView->KeySetStartFrame( atoi(s) ) ;
	}
}

void CDlgKey::OnChangeRate() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	if( !m_Updating )
	{
		ASSERT( m_pView ) ;
		CString s ;
		m_FrameRate.GetWindowText( s ) ;
		m_pView->KeySetFrameRate( (f32)atof(s) ) ;
	}
}

void CDlgKey::OnChangeBlend() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	if( !m_Updating )
	{
		ASSERT( m_pView ) ;
		CString s ;
		m_Blend.GetWindowText( s ) ;
		m_pView->KeySetBlendFrames( (f32)atof(s) ) ;
	}
}

void CDlgKey::OnMirror() 
{
	// TODO: Add your control notification handler code here
	if( !m_Updating )
	{
		ASSERT( m_pView ) ;
		m_pView->KeySetMirror( !!m_Mirror.GetCheck() ) ;
	}
}

void CDlgKey::OnChangeX() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_Updating )
	{
		ASSERT( m_pView ) ;
		CString s ;
		m_X.GetWindowText( s ) ;
		m_pView->KeySetX( (f32)atof(s) ) ;
	}
}

void CDlgKey::OnChangeY() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_Updating )
	{
		ASSERT( m_pView ) ;
		CString s ;
		m_Y.GetWindowText( s ) ;
		m_pView->KeySetY( (f32)atof(s) ) ;
	}
}

void CDlgKey::OnChangeZ() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_Updating )
	{
		ASSERT( m_pView ) ;
		CString s ;
		m_Z.GetWindowText( s ) ;
		m_pView->KeySetZ( (f32)atof(s) ) ;
	}
}

void CDlgKey::OnChangeVx() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_Updating )
	{
		ASSERT( m_pView ) ;
		CString s ;
		m_VX.GetWindowText( s ) ;
		m_pView->KeySetVX( (f32)atof(s) ) ;
	}
}

void CDlgKey::OnChangeVy() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_Updating )
	{
		ASSERT( m_pView ) ;
		CString s ;
		m_VY.GetWindowText( s ) ;
		m_pView->KeySetVY( (f32)atof(s) ) ;
	}
}

void CDlgKey::OnChangeVz() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_Updating )
	{
		ASSERT( m_pView ) ;
		CString s ;
		m_VZ.GetWindowText( s ) ;
		m_pView->KeySetVZ( (f32)atof(s) ) ;
	}
}

LONG CDlgKey::OnMouseStaticMove( UINT wParam, LONG lParam )
{
	f32		dx = (f32)( (s16)((lParam>>16)&0xffff)) ;
	f32		dy = (f32)(-(s16)(lParam&0xffff)) ;

	dx /= 10 ;
	dy /= 10 ;

	ASSERT( m_pView ) ;

	switch( wParam )
	{
	case IDC_MOVE_X:
		m_pView->KeyMove( dx+dy, 0, 0 ) ;
		break ;
	case IDC_MOVE_Y:
		m_pView->KeyMove( 0, dx+dy, 0 ) ;
		break ;
	case IDC_MOVE_Z:
		m_pView->KeyMove( 0, 0, dx+dy ) ;
		break ;
	case IDC_MOVE_VX:
		m_pView->KeyMoveV( dx+dy, 0, 0 ) ;
		break ;
	case IDC_MOVE_VY:
		m_pView->KeyMoveV( 0, dx+dy, 0 ) ;
		break ;
	case IDC_MOVE_VZ:
		m_pView->KeyMoveV( 0, 0, dx+dy ) ;
		break ;
	case IDC_MOVE_RX:
		m_pView->KeyMoveR( dx+dy, 0, 0 ) ;
		break ;
	case IDC_MOVE_RY:
		m_pView->KeyMoveR( 0, dx+dy, 0 ) ;
		break ;
	case IDC_MOVE_RZ:
		m_pView->KeyMoveR( 0, 0, dx+dy ) ;
		break ;
	}

	return 0 ;
}

void CDlgKey::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	if( m_pView )
		m_pView->KeyLookAt( ) ;

	*pResult = 0;
}

void CDlgKey::OnChangeRx() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_Updating )
	{
		ASSERT( m_pView ) ;
		CString s ;
		m_RX.GetWindowText( s ) ;
		m_pView->KeySetRX( (f32)atof(s) ) ;
	}
}

void CDlgKey::OnChangeRy() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_Updating )
	{
		ASSERT( m_pView ) ;
		CString s ;
		m_RY.GetWindowText( s ) ;
		m_pView->KeySetRY( (f32)atof(s) ) ;
	}
}

void CDlgKey::OnChangeRz() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_Updating )
	{
		ASSERT( m_pView ) ;
		CString s ;
		m_RZ.GetWindowText( s ) ;
		m_pView->KeySetRZ( (f32)atof(s) ) ;
	}
}


void CDlgKey::OnChangeScale() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	if( m_pView && ::IsWindow(GetSafeHwnd()) )
	{
		CKey *pKey = (CKey*)MFCU_ListCtrlGetSelected( m_List ) ;
		if( pKey )
		{
			CString String ;
			m_Scale.GetWindowText( String ) ;
			pKey->SetScale( (f32)atof( String ) ) ;
			m_pView->Invalidate() ;
		}
	}
	
}

void CDlgKey::OnVisible() 
{
	// TODO: Add your control notification handler code here
	
	if( m_pView && ::IsWindow(GetSafeHwnd()) )
	{
		CKey *pKey = (CKey*)MFCU_ListCtrlGetSelected( m_List ) ;
		if( pKey )
		{
			pKey->m_Visible = !!m_Visible.GetCheck() ;
			m_pView->Invalidate() ;
		}
	}
}




void CDlgKey::OnCameralock() 
{
	// TODO: Add your control notification handler code here
	if( m_pView && ::IsWindow(GetSafeHwnd()) )
	{
		CKey *pKey = (CKey*)MFCU_ListCtrlGetSelected( m_List ) ;
		if( pKey )
		{
			pKey->m_CameraLock = !!m_CameraLock.GetCheck() ;
			m_pView->Invalidate() ;
		}
	}
	
}
