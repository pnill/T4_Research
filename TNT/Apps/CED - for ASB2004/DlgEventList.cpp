// DlgEventList.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "DlgEventList.h"
#include "Motion.h"
#include "Event.h"
#include "MFCutils.h"
#include "MotionEditView.h"
#include "AttachJointDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgEventList dialog


CDlgEventList::CDlgEventList(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgEventList::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgEventList)
	//}}AFX_DATA_INIT

	m_pView = NULL ;
	m_EditsLocked = false ;
}


void CDlgEventList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgEventList)
	DDX_Control(pDX, IDC_EVENT_DISTLOCAL, m_DistLocal);
	DDX_Control(pDX, IDC_EVENT_DIST, m_Dist);
	DDX_Control(pDX, IDC_EVENT_DIRLOCAL, m_DirLocal);
	DDX_Control(pDX, IDC_EVENT_DIR, m_Dir);
	DDX_Control(pDX, IDC_EVENT_ZLOCAL, m_Zlocal);
	DDX_Control(pDX, IDC_EVENT_YLOCAL, m_Ylocal);
	DDX_Control(pDX, IDC_EVENT_XLOCAL, m_Xlocal);
	DDX_Control(pDX, IDC_STATIC_ATTACHED_JOINT_DISPLAY_EVENTLIST, m_Bone);
	DDX_Control(pDX, IDC_BUTTON_ATTACH_BONE, m_ButtonAttachBone);
	DDX_Control(pDX, IDC_BUTTON_INSERT_EVENT, m_ButtonInsertEvent);
	DDX_Control(pDX, IDC_COMBO_PROJECT_EVENTS, m_ComboBoxProjectEvents);
	DDX_Control(pDX, IDC_EVENT_RADIUS, m_R);
	DDX_Control(pDX, IDC_EVENT_FRAME, m_Frame);
	DDX_Control(pDX, IDC_EVENT_Z, m_Z);
	DDX_Control(pDX, IDC_EVENT_Y, m_Y);
	DDX_Control(pDX, IDC_EVENT_X, m_X);
	DDX_Control(pDX, IDC_EVENT_LIST, m_List);
	//}}AFX_DATA_MAP
}


void CDlgEventList::UpdateEventList( CMotion *pMotion )
{
	//---	Reset the List
	m_List.ResetContent() ;
	m_List.AddString( "<no event>" ) ;

	m_List.EnableWindow( pMotion != NULL ) ;
	m_ComboBoxProjectEvents.EnableWindow( pMotion != NULL );
	m_ButtonInsertEvent.EnableWindow( pMotion != NULL );

	//---	Add All Events
	if( pMotion )
	{
		for( int i = 0 ; i < pMotion->GetNumEvents() ; i++ )
		{
			CEvent *pEvent = pMotion->GetEvent(i) ;
			CString Name ;
			Name.Format( "%03d - %s", pEvent->GetFrame(), pEvent->GetName() ) ;
			int Index = m_List.AddString( Name ) ;
			m_List.SetItemData( Index, (DWORD)pEvent ) ;
		}

		//---	Set Current Selection
		int CurrentSel = 0 ;
		CurrentSel = MFCU_FindListBoxItemByData( &m_List, (DWORD)pMotion->GetCurEvent() ) ;
		if( CurrentSel == -1 ) CurrentSel = 0 ;
		m_List.SetCurSel( CurrentSel ) ;
	}

	//---	Redraw Control
	m_List.RedrawWindow() ;
}

void CDlgEventList::UpdateEventControls( CMotion *pMotion )
{
//	vector3d	v, vlocal, vframe ;
	f32			Radius = 0 ;
	int			Frame = 0 ;
	CString		Name ;
	bool		Bone = FALSE;
	CString		BoneName;
	CEvent*		pEvent = NULL;

//	V3_Set( &v, 0, 0, 0 ) ;
//	V3_Set( &vlocal, 0, 0, 0 ) ;

	bool Enable = false ;

	if( pMotion )
	{
		pEvent = pMotion->GetCurEvent() ;
		if( pEvent )
		{
//			pEvent->GetPosition( &v ) ;

//			pMotion->GetTranslation( &vframe, pEvent->GetFrame() );
//			V3_Sub( &vlocal, &v, &vframe );

			Radius = pEvent->GetRadius() ;
			Frame = pEvent->GetFrame() ;
			Name = pEvent->GetName() ;
			if(( pEvent->GetBone() != -1 ) || ( pEvent->GetBoneName() != "<no bone>" ))
			{
				Bone = TRUE;

				CSkeleton* pSkeleton;
				pSkeleton = m_pView->GetSkeleton();
				ASSERT( pSkeleton );

				if( pEvent->GetBone() != -1 )
					BoneName = pSkeleton->GetBoneFromIndex( pEvent->GetBone() )->BoneName;
				else
					BoneName = pEvent->GetBoneName();
			}

			Enable = true ;
		}
	}

	//---	Set Strings into Controls
	m_EditsLocked = 1;
	UpdateEventPositionDisplay( 0, pMotion, pEvent ) ;
	MFCU_UpdateEditControl( m_R, Radius ) ;
	MFCU_UpdateEditControl( m_Frame, Frame ) ;
//	MFCU_UpdateEditControl( m_Name, Name ) ;
	m_EditsLocked = 0;

	//---	Enable / Disable Controls
	m_X.EnableWindow( Enable && !pEvent->GetAttached() ) ;
	m_Y.EnableWindow( Enable && !pEvent->GetAttached() ) ;
	m_Z.EnableWindow( Enable && !pEvent->GetAttached() ) ;
	m_Dir.EnableWindow( Enable && !pEvent->GetAttached() ) ;
	m_Dist.EnableWindow( Enable && !pEvent->GetAttached() ) ;
	m_Xlocal.EnableWindow( Enable && !pEvent->GetAttached() ) ;
	m_Ylocal.EnableWindow( Enable && !pEvent->GetAttached() ) ;
	m_Zlocal.EnableWindow( Enable && !pEvent->GetAttached() ) ;
	m_DirLocal.EnableWindow( Enable && !pEvent->GetAttached() ) ;
	m_DistLocal.EnableWindow( Enable && !pEvent->GetAttached() ) ;
	m_R.EnableWindow( Enable ) ;
	m_Frame.EnableWindow( Enable ) ;
//	m_Name.EnableWindow( Enable ) ;
	m_Bone.EnableWindow( Enable ) ;
	m_Bone.ModifyStyleEx( ( Enable && Bone ) ? 0 : WS_EX_STATICEDGE, ( Enable && Bone ) ? WS_EX_STATICEDGE : 0, 0 );
	m_Bone.SetWindowText( BoneName );
	GetDlgItem( IDC_STATIC_ATTACHED_JOINT_TEXT_EVENTLIST )->EnableWindow( ( Enable && Bone ) ? TRUE : FALSE );
//	m_ButtonAddCurrentEvent.EnableWindow( Enable );
	m_ButtonAttachBone.EnableWindow( Enable && !pEvent->GetAttached() );

	m_R.RedrawWindow( ) ;
	m_Frame.RedrawWindow( ) ;
//	m_Name.RedrawWindow( ) ;
	m_Bone.UpdateWindow();
	m_Bone.RedrawWindow( ) ;

	{
		CRect Rect;

		m_Bone.GetWindowRect( Rect );
		ScreenToClient( Rect );
		InvalidateRect( Rect, TRUE );
		UpdateWindow();
	}
}

BEGIN_MESSAGE_MAP(CDlgEventList, CDialog)
	//{{AFX_MSG_MAP(CDlgEventList)
	ON_LBN_SELCHANGE(IDC_EVENT_LIST, OnSelchangeEventList)
	ON_BN_CLICKED(IDC_EVENT_ADD, OnEventAdd)
	ON_BN_CLICKED(IDC_EVENT_DELETE, OnEventDelete)
	ON_EN_CHANGE(IDC_EVENT_X, OnChangeEventX)
	ON_EN_CHANGE(IDC_EVENT_XLOCAL, OnChangeEventXlocal)
	ON_EN_CHANGE(IDC_EVENT_Y, OnChangeEventY)
	ON_EN_CHANGE(IDC_EVENT_YLOCAL, OnChangeEventYlocal)
	ON_EN_CHANGE(IDC_EVENT_Z, OnChangeEventZ)
	ON_EN_CHANGE(IDC_EVENT_ZLOCAL, OnChangeEventZlocal)
	ON_EN_CHANGE(IDC_EVENT_DIR, OnChangeEventDir)
	ON_EN_CHANGE(IDC_EVENT_DIRLOCAL, OnChangeEventDirlocal)
	ON_EN_CHANGE(IDC_EVENT_DIST, OnChangeEventDist)
	ON_EN_CHANGE(IDC_EVENT_DISTLOCAL, OnChangeEventDistlocal)
	ON_EN_CHANGE(IDC_EVENT_RADIUS, OnChangeEventRadius)
	ON_EN_CHANGE(IDC_EVENT_FRAME, OnChangeEventFrame)
	ON_EN_CHANGE(IDC_EVENT_NAME, OnChangeEventName)
	ON_BN_CLICKED(IDC_BUTTON_NEW_PROJECT_EVENT, OnButtonNewProjectEvent)
	ON_BN_CLICKED(IDC_BUTTON_ADD_CURRENT_EVENT, OnButtonAddCurrentEvent)
	ON_BN_CLICKED(IDC_BUTTON_INSERT_EVENT, OnButtonInsertEvent)
	ON_BN_CLICKED(IDC_BUTTON_ATTACH_BONE, OnButtonAttachBone)
	ON_BN_CLICKED(IDC_BUTTON_ATTACH_BONENOW, OnButtonAttachBoneNow)
	ON_NOTIFY_EX( TTN_NEEDTEXT, 0, OnToolTipNotify )
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_MESSAGE( MSM_MOUSEMOVE, OnMouseStaticMove )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgEventList message handlers

void CDlgEventList::OnOk( )
{
}

void CDlgEventList::OnCancel( )
{
}

BOOL CDlgEventList::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	m_StaticEventX.SubclassDlgItem( IDC_STATIC_EVENT_X, this ) ;
	m_StaticEventX.SetNotifyWindow( this ) ;

	m_StaticEventY.SubclassDlgItem( IDC_STATIC_EVENT_Y, this ) ;
	m_StaticEventY.SetNotifyWindow( this ) ;
	
	m_StaticEventZ.SubclassDlgItem( IDC_STATIC_EVENT_Z, this ) ;
	m_StaticEventZ.SetNotifyWindow( this ) ;
	
	m_StaticEventR.SubclassDlgItem( IDC_STATIC_EVENT_R, this ) ;
	m_StaticEventR.SetNotifyWindow( this ) ;

	m_ComboBoxProjectEvents.SetCurSel( 0 );

	EnableToolTips( TRUE );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgEventList::OnSelchangeEventList() 
{
	// TODO: Add your control notification handler code here

	ASSERT( m_pView ) ;

	CEvent *pEvent = (CEvent *)m_List.GetItemData( m_List.GetCurSel() ) ;
	m_pView->EventSetCurrent( pEvent ) ;
}

void CDlgEventList::OnEventAdd() 
{
	// TODO: Add your control notification handler code here
	ASSERT( m_pView ) ;

	m_pView->EventAdd( ) ;
}

void CDlgEventList::OnEventDelete() 
{
	// TODO: Add your control notification handler code here
	ASSERT( m_pView ) ;
	
	CEvent *pEvent = (CEvent *)m_List.GetItemData( m_List.GetCurSel() ) ;
	if( pEvent )
	{
		CEvent *pNewCurrentEvent = (CEvent*)m_List.GetItemData( min(m_List.GetCount()-1, m_List.GetCurSel()+1) ) ;
		if( pNewCurrentEvent == pEvent )
			pNewCurrentEvent = (CEvent*)m_List.GetItemData( max(0, m_List.GetCurSel()-1) ) ;
		m_pView->EventDelete( pEvent, pNewCurrentEvent ) ;
	}
}

void CDlgEventList::OnChangeEventX() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	if( !m_EditsLocked )
	{
		CEvent *pEvent = (CEvent *)m_List.GetItemData( m_List.GetCurSel() ) ;
		if( pEvent && !pEvent->GetAttached() )
		{
			vector3d	Position ;
			pEvent->GetPosition( &Position ) ;

			CString		String ;
			m_X.GetWindowText( String ) ;

			Position.X = (f32)atof( String ) ;
			pEvent->SetPosition( &Position ) ;
			UpdateEventPositionDisplay( IDC_EVENT_X, m_pView->GetCurMotion(), pEvent );
			m_pView->GetDocument()->SetModifiedFlag( TRUE ) ;
			m_pView->Invalidate() ;
		}
	}
}

void CDlgEventList::OnChangeEventXlocal() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	if( !m_EditsLocked )
	{
		CEvent *pEvent = (CEvent *)m_List.GetItemData( m_List.GetCurSel() ) ;
		if( pEvent && !pEvent->GetAttached() )
		{
			vector3d v;
			CMotion* pMotion = m_pView->GetCurMotion();
			pMotion->GetTranslation( &v, pEvent->GetFrame() );

			vector3d	Position ;
			pEvent->GetPosition( &Position ) ;

			CString		String ;
			m_Xlocal.GetWindowText( String ) ;

			Position.X = (f32)atof( String ) ;
			Position.X += v.X;
			pEvent->SetPosition( &Position ) ;
			UpdateEventPositionDisplay( IDC_EVENT_XLOCAL, pMotion, pEvent );
			m_pView->GetDocument()->SetModifiedFlag( TRUE ) ;
			m_pView->Invalidate() ;
		}
	}
}

void CDlgEventList::OnChangeEventY() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_EditsLocked )
	{
		CEvent *pEvent = (CEvent *)m_List.GetItemData( m_List.GetCurSel() ) ;
		if( pEvent && !pEvent->GetAttached() )
		{
			vector3d	Position ;
			pEvent->GetPosition( &Position ) ;

			CString		String ;
			m_Y.GetWindowText( String ) ;

			Position.Y = (f32)atof( String ) ;
			pEvent->SetPosition( &Position ) ;
			UpdateEventPositionDisplay( IDC_EVENT_Y, m_pView->GetCurMotion(), pEvent );
			m_pView->GetDocument()->SetModifiedFlag( TRUE ) ;
			m_pView->Invalidate() ;
		}
	}
}

void CDlgEventList::OnChangeEventYlocal() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_EditsLocked )
	{
		CEvent *pEvent = (CEvent *)m_List.GetItemData( m_List.GetCurSel() ) ;
		if( pEvent && !pEvent->GetAttached() )
		{
			vector3d v;
			CMotion* pMotion = m_pView->GetCurMotion();
			pMotion->GetTranslation( &v, pEvent->GetFrame() );

			vector3d	Position ;
			pEvent->GetPosition( &Position ) ;

			CString		String ;
			m_Ylocal.GetWindowText( String ) ;

			Position.Y = (f32)atof( String ) ;
			Position.Y += v.Y;
			pEvent->SetPosition( &Position ) ;
			UpdateEventPositionDisplay( IDC_EVENT_YLOCAL, pMotion, pEvent );
			m_pView->GetDocument()->SetModifiedFlag( TRUE ) ;
			m_pView->Invalidate() ;
		}
	}
}

void CDlgEventList::OnChangeEventZ() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_EditsLocked )
	{
		CEvent *pEvent = (CEvent *)m_List.GetItemData( m_List.GetCurSel() ) ;
		if( pEvent && !pEvent->GetAttached() )
		{
			vector3d	Position ;
			pEvent->GetPosition( &Position ) ;

			CString		String ;
			m_Z.GetWindowText( String ) ;

			Position.Z = (f32)atof( String ) ;
			pEvent->SetPosition( &Position ) ;
			UpdateEventPositionDisplay( IDC_EVENT_Z, m_pView->GetCurMotion(), pEvent );
			m_pView->GetDocument()->SetModifiedFlag( TRUE ) ;
			m_pView->Invalidate() ;
		}
	}
}

void CDlgEventList::OnChangeEventZlocal() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_EditsLocked )
	{
		CEvent *pEvent = (CEvent *)m_List.GetItemData( m_List.GetCurSel() ) ;
		if( pEvent && !pEvent->GetAttached() )
		{
			vector3d v;
			CMotion* pMotion = m_pView->GetCurMotion();
			pMotion->GetTranslation( &v, pEvent->GetFrame() );

			vector3d	Position ;
			pEvent->GetPosition( &Position ) ;

			CString		String ;
			m_Zlocal.GetWindowText( String ) ;

			Position.Z = (f32)atof( String ) ;
			Position.Z += v.Z;
			pEvent->SetPosition( &Position ) ;
			UpdateEventPositionDisplay( IDC_EVENT_ZLOCAL, pMotion, pEvent );
			m_pView->GetDocument()->SetModifiedFlag( TRUE ) ;
			m_pView->Invalidate() ;
		}
	}
}

void CDlgEventList::OnChangeEventDir() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_EditsLocked )
	{
		CEvent *pEvent = (CEvent *)m_List.GetItemData( m_List.GetCurSel() ) ;
		if( pEvent && !pEvent->GetAttached() )
		{
			vector3d	Position ;
			pEvent->GetPosition( &Position ) ;

			f32 Dist = x_sqrt( Position.X*Position.X + Position.Z*Position.Z ) ;

			CString		String ;
			m_Dir.GetWindowText( String ) ;

			f32 Dir = atoi(String)*R_360/(f32)(1<<pEvent->GetMotion()->GetExportBits()) ;

			Position.X = Dist*x_sin( Dir ) ;
			Position.Z = Dist*x_cos( Dir ) ;

			pEvent->SetPosition( &Position ) ;
			UpdateEventPositionDisplay( IDC_EVENT_DIR, m_pView->GetCurMotion(), pEvent );
			m_pView->GetDocument()->SetModifiedFlag( TRUE ) ;
			m_pView->Invalidate() ;
		}
	}
}

void CDlgEventList::OnChangeEventDirlocal() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_EditsLocked )
	{
		CEvent *pEvent = (CEvent *)m_List.GetItemData( m_List.GetCurSel() ) ;
		if( pEvent && !pEvent->GetAttached() )
		{
			vector3d	Pos ;
			pEvent->GetPosition( &Pos ) ;

			CMotion* pMotion = m_pView->GetCurMotion();
			if( !pMotion )
				return;

			vector3d PosLocal, PosFrame;
			pMotion->GetTranslation( &PosFrame, pEvent->GetFrame() );
			V3_Sub( &PosLocal, &Pos, &PosFrame );

			f32 Dist = x_sqrt( PosLocal.X*PosLocal.X + PosLocal.Z*PosLocal.Z ) ;

			CString		String ;
			m_DirLocal.GetWindowText( String ) ;

			f32 Dir = atoi(String)*R_360/(f32)(1<<pMotion->GetExportBits()) ;

			PosLocal.X = Dist*x_sin( Dir ) ;
			PosLocal.Z = Dist*x_cos( Dir ) ;

			V3_Add( &Pos, &PosLocal, &PosFrame );

			pEvent->SetPosition( &Pos ) ;
			UpdateEventPositionDisplay( IDC_EVENT_DIRLOCAL, pMotion, pEvent );
			m_pView->GetDocument()->SetModifiedFlag( TRUE ) ;
			m_pView->Invalidate() ;
		}
	}
}

void CDlgEventList::OnChangeEventDist() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_EditsLocked )
	{
		CEvent *pEvent = (CEvent *)m_List.GetItemData( m_List.GetCurSel() ) ;
		if( pEvent && !pEvent->GetAttached() )
		{
			vector3d	Position ;
			pEvent->GetPosition( &Position ) ;

			f32 Dir = x_atan2( Position.X, Position.Z ) ;

			CString		String ;
			m_Dist.GetWindowText( String ) ;

			f32 Dist = (f32)atof(String);

			Position.X = Dist*x_sin( Dir ) ;
			Position.Z = Dist*x_cos( Dir ) ;

			pEvent->SetPosition( &Position ) ;
			UpdateEventPositionDisplay( IDC_EVENT_DIST, m_pView->GetCurMotion(), pEvent );
			m_pView->GetDocument()->SetModifiedFlag( TRUE ) ;
			m_pView->Invalidate() ;
		}
	}
}

void CDlgEventList::OnChangeEventDistlocal() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_EditsLocked )
	{
		CEvent *pEvent = (CEvent *)m_List.GetItemData( m_List.GetCurSel() ) ;
		if( pEvent && !pEvent->GetAttached() )
		{
			vector3d	Pos ;
			pEvent->GetPosition( &Pos ) ;

			CMotion* pMotion = m_pView->GetCurMotion();
			if( !pMotion )
				return;

			vector3d PosLocal, PosFrame;
			pMotion->GetTranslation( &PosFrame, pEvent->GetFrame() );
			V3_Sub( &PosLocal, &Pos, &PosFrame );

			f32 Dir = x_atan2( PosLocal.X, PosLocal.Z ) ;

			CString		String ;
			m_DistLocal.GetWindowText( String ) ;

			f32 Dist = (f32)atof(String);

			PosLocal.X = Dist*x_sin( Dir ) ;
			PosLocal.Z = Dist*x_cos( Dir ) ;

			V3_Add( &Pos, &PosLocal, &PosFrame );

			pEvent->SetPosition( &Pos ) ;
			UpdateEventPositionDisplay( IDC_EVENT_DISTLOCAL, m_pView->GetCurMotion(), pEvent );
			m_pView->GetDocument()->SetModifiedFlag( TRUE ) ;
			m_pView->Invalidate() ;
		}
	}
}

void CDlgEventList::UpdateEventPositionDisplay( u32 ControlChanged, CMotion* pMotion, CEvent* pEvent )
{
	vector3d	Pos;
	f32			Dist;
	s32			Dir;
	vector3d	PosLocal;
	f32			DistLocal;
	s32			DirLocal;

	V3_Set( &Pos, 0, 0, 0 );
	V3_Set( &PosLocal, 0, 0, 0 );
	Dist = DistLocal = 0.0f;
	Dir = DirLocal = 0;

	//---	if there is no selected event return now
//	CEvent *pEvent = pMotion->GetCurEvent() ;
//	CEvent *pEvent = (CEvent *)m_List.GetItemData( m_List.GetCurSel() ) ;
	if( pEvent )
	{
		s32	ExportAngle360 = (1<<pMotion->GetExportBits());

		//---	get the position information for the event in world space
		pEvent->GetPosition( &Pos );
		Dist = x_sqrt( Pos.X*Pos.X + Pos.Z*Pos.Z );
		Dir = (s32) (x_atan2( Pos.X, Pos.Z) * (f32)ExportAngle360 / R_360);
		while( Dir >= ExportAngle360 )	Dir -= ExportAngle360;
		while( Dir < 0 )				Dir += ExportAngle360;
		
		//---	if a motion was provided, get the local coordinates
		if( pMotion )
		{
			vector3d PosFrame;
			pMotion->GetTranslation( &PosFrame, pEvent->GetFrame() );

			V3_Sub( &PosLocal, &Pos, &PosFrame );
			DistLocal = x_sqrt( PosLocal.X*PosLocal.X + PosLocal.Z*PosLocal.Z );
			DirLocal = (s32) (x_atan2( PosLocal.X, PosLocal.Z ) * (f32)ExportAngle360 / R_360);
			while( DirLocal >= ExportAngle360 )	DirLocal -= ExportAngle360;
			while( DirLocal < 0 )				DirLocal += ExportAngle360;
		}
	}

	//---	update only the display information which must be updated
	m_EditsLocked++;
	if( ControlChanged != 1 )
	{
		if( ControlChanged != IDC_EVENT_X )			MFCU_UpdateEditControl( m_X, Pos.X );
		if( ControlChanged != IDC_EVENT_Y )			MFCU_UpdateEditControl( m_Y, Pos.Y );
		if( ControlChanged != IDC_EVENT_Z )			MFCU_UpdateEditControl( m_Z, Pos.Z );
		if( ControlChanged != IDC_EVENT_DIST )		MFCU_UpdateEditControl( m_Dist, Dist );
		if( ControlChanged != IDC_EVENT_DIR )		MFCU_UpdateEditControl( m_Dir, Dir );
	}
	if( ControlChanged != IDC_EVENT_XLOCAL )	MFCU_UpdateEditControl( m_Xlocal, PosLocal.X );
	if( ControlChanged != IDC_EVENT_YLOCAL )	MFCU_UpdateEditControl( m_Ylocal, PosLocal.Y );
	if( ControlChanged != IDC_EVENT_ZLOCAL )	MFCU_UpdateEditControl( m_Zlocal, PosLocal.Z );
	if( ControlChanged != IDC_EVENT_DISTLOCAL )	MFCU_UpdateEditControl( m_DistLocal, DistLocal );
	if( ControlChanged != IDC_EVENT_DIRLOCAL )	MFCU_UpdateEditControl( m_DirLocal, DirLocal );
	m_EditsLocked--;
}

void CDlgEventList::OnChangeEventRadius() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_EditsLocked )
	{
		CEvent *pEvent = (CEvent *)m_List.GetItemData( m_List.GetCurSel() ) ;
		if( pEvent )
		{
			CString		String ;
			m_R.GetWindowText( String ) ;

			pEvent->SetRadius( (f32)atof( String ) ) ;
			m_pView->GetDocument()->SetModifiedFlag( TRUE ) ;
			m_pView->Invalidate() ;
		}
	}
}

void CDlgEventList::OnChangeEventFrame() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_EditsLocked )
	{
		CEvent *pEvent = (CEvent *)m_List.GetItemData( m_List.GetCurSel() ) ;
		if( pEvent )
		{
			CString		String ;
			m_Frame.GetWindowText( String ) ;

			pEvent->SetFrame( atoi( String ) ) ;
			UpdateEventPosition( pEvent );
			UpdateEventPositionDisplay( 1, m_pView->GetCurMotion(), pEvent );
			m_pView->UpdateEventList( ) ;
			m_pView->GetDocument()->SetModifiedFlag( TRUE ) ;
			m_pView->Invalidate() ;
		}
	}
}

void CDlgEventList::OnChangeEventName() 
{
/*	
	if( !m_EditsLocked )
	{
		CEvent *pEvent = (CEvent *)m_List.GetItemData( m_List.GetCurSel() ) ;
		if( pEvent )
		{
			CString		String ;
			m_Name.GetWindowText( String ) ;

			pEvent->SetName( String ) ;
			m_pView->UpdateEventList() ;
			m_pView->GetDocument()->SetModifiedFlag( TRUE ) ;
			m_pView->Invalidate() ;
		}
	}
*/
}

LONG CDlgEventList::OnMouseStaticMove( UINT wParam, LONG lParam )
{
	f32		dx = (f32)( (s16)((lParam>>16)&0xffff)) ;
	f32		dy = (f32)(-(s16)(lParam&0xffff)) ;

	dx /= 10 ;
	dy /= 10 ;

	ASSERT( m_pView ) ;

	switch( wParam )
	{
		case IDC_STATIC_EVENT_X:
			m_pView->ModifyEventPosition( dx+dy, 0, 0 ) ;
			break ;
		case IDC_STATIC_EVENT_Y:
			m_pView->ModifyEventPosition( 0, dx+dy, 0 ) ;
			break ;
		case IDC_STATIC_EVENT_Z:
			m_pView->ModifyEventPosition( 0, 0, dx+dy ) ;
			break ;
		case IDC_STATIC_EVENT_R:
			m_pView->ModifyEventRadius( dx+dy ) ;
			break ;
	}

	return 0 ;
}
void CDlgEventList::UpdateEventDefinedEvents()
{
	INT CurSel;
	
	CurSel = m_ComboBoxProjectEvents.GetCurSel();
	m_ComboBoxProjectEvents.ResetContent();
	UpdateEventAttachedEvents();
	UpdateEventProjectEvents();
	m_ComboBoxProjectEvents.SetCurSel( ( CurSel == CB_ERR ) ? 0 : CurSel );
}

void CDlgEventList::UpdateEventAttachedEvents()
{
	POSITION pos;
	CEventList* pAttachedEvents;

	pAttachedEvents = m_pView->GetAttachedEvents();
	if( pAttachedEvents )
	{
		pos = pAttachedEvents->GetHeadPosition();
		while( pos )
		{
			CEvent* pEvent;
			s32		Index;

			pEvent = pAttachedEvents->GetNext( pos );
			if( pEvent )
			{
				if( !pEvent->GetAttached() )
				{
					Index = m_ComboBoxProjectEvents.AddString( pEvent->GetName() );
					m_ComboBoxProjectEvents.SetItemData( Index, (u32)pEvent );
				}
				else
				{
					CString Text;
					CSkeleton* pSkeleton;
					skel_bone* pBone;
					
					pSkeleton = m_pView->GetSkeleton();
					ASSERT( pSkeleton );

					if( pEvent->GetBone() == -1 )
						pBone = pSkeleton->GetBoneFromName( pEvent->GetBoneName() );
					else
						pBone = pSkeleton->GetBoneFromIndex( pEvent->GetBone() );

					if( pBone )
					{				
						Text = pEvent->GetName();
						Text += " - ";
						Text += pBone->BoneName;
						Index = m_ComboBoxProjectEvents.AddString( Text );
						m_ComboBoxProjectEvents.SetItemData( Index, (u32)pEvent );
					}
				}
			}
		}
	}
}

void CDlgEventList::UpdateEventProjectEvents()
{
	POSITION pos;
	CEventList* pProjectEvents = NULL;
	CEventList* pAttachedEvents = NULL;

	pProjectEvents = m_pView->GetProjectEvents();
	ASSERT( pProjectEvents );
	pAttachedEvents = m_pView->GetAttachedEvents();

	pos = pProjectEvents->GetHeadPosition();
	while( pos )
	{
		CEvent* pEvent;
		CString EventName;
		s32		Index;

		pEvent = pProjectEvents->GetNext( pos );
		EventName = pEvent->GetName();

    	ASSERT( pAttachedEvents );
		if( pAttachedEvents->GetPositionbyName( EventName ) == NULL )
		{
			if( !pEvent->GetAttached() )
			{
				Index = m_ComboBoxProjectEvents.AddString( EventName );
				m_ComboBoxProjectEvents.SetItemData( Index, (u32)pEvent );
			}
			else
			{
				CString Text;
				CSkeleton* pSkeleton;
				skel_bone* pBone;
				
				pSkeleton = m_pView->GetSkeleton();
				ASSERT( pSkeleton );

				if( pEvent->GetBone() == -1 )
					pBone = pSkeleton->GetBoneFromName( pEvent->GetBoneName() );
				else
					pBone = pSkeleton->GetBoneFromIndex( pEvent->GetBone() );

				if( pBone )
				{				
					Text = EventName;
					Text += " - ";
					Text += pBone->BoneName;
					m_ComboBoxProjectEvents.AddString( Text );
					m_ComboBoxProjectEvents.SetItemData( Index, (u32)pEvent );
				}
			}
		}
	}
}

void CDlgEventList::OnButtonNewProjectEvent() 
{
	ASSERT( m_pView );
	
	m_pView->GetDocument()->ShowDefinedEventsDlg();
}

void CDlgEventList::OnButtonInsertEvent() 
{
	CEvent* pInsertEvent = NULL;
	CString EventName;
	s32 CurSel;

	ASSERT( m_pView );

	CurSel = m_ComboBoxProjectEvents.GetCurSel();
	if( CurSel == -1 )
	{
		m_ComboBoxProjectEvents.GetWindowText( EventName );
		if( EventName.GetLength() == 0)
			return;
	}
	else
		m_ComboBoxProjectEvents.GetLBText( m_ComboBoxProjectEvents.GetCurSel(), EventName );

	//---	remove any bone name part of the string
	s32 Index = EventName.Find( " - " );
	if( Index != -1 )
		EventName = EventName.Left( Index );

	//---	find the event in the event lists
	pInsertEvent = GetProjectEventbyName( EventName );
	if( !pInsertEvent )
		pInsertEvent = GetAttachedEventbyName( EventName );

	//---	if the event wans't found, attempt to add it
	if( !pInsertEvent )
	{
		CString msg;
		msg.Format( "The event '%s' is not defined.  Do you want to create a new event?", EventName );
		s32 Answer = MessageBox( msg, "Create new event?", MB_ICONWARNING | MB_YESNO );
		if( Answer != IDYES )
			return;

		//---	add the new event
		pInsertEvent = new CEvent;
		pInsertEvent->SetName( EventName );
		m_pView->GetDocument()->GetProjectEventList()->AddTail( pInsertEvent );
	}

	AddDefinedEvent( pInsertEvent );
	m_pView->GetDocument()->SetModifiedFlag( TRUE ) ;
	m_pView->GetDocument()->UpdateAllViews( NULL, HINT_EVENT_NEW ) ;
}

CEvent* CDlgEventList::GetAttachedEventbyName(CString EventName)
{
	POSITION pos;
	CEvent* pEvent;

	pEvent = NULL;
	pos = m_pView->GetAttachedEvents()->GetPositionbyName( EventName );
	if( pos )
	{
		pEvent = (CEvent *)m_pView->GetAttachedEvents()->GetAt( pos );
	}
	return pEvent;
}

CEvent* CDlgEventList::GetProjectEventbyName(CString EventName)
{
	POSITION pos;
	CEvent* pEvent;

	pEvent = NULL;
	pos = m_pView->GetProjectEvents()->GetPositionbyName( EventName );
	if( pos )
	{
		pEvent = (CEvent *)m_pView->GetProjectEvents()->GetAt( pos );
	}
	return pEvent;
}

void CDlgEventList::AddDefinedEvent(CEvent *pAddEvent)
{
	ASSERT( m_pView );
	ASSERT( pAddEvent );

	if( !pAddEvent )
		return;

	if( !m_pView->GetCurMotion() )
		return;

	bool Attached = FALSE;

	CEvent *pEvent = m_pView->GetCurMotion()->AddEvent() ;

	ASSERT( pEvent );

	if( pEvent )
	{
		*pEvent <= *pAddEvent;

		pEvent->SetFrame( (s32)(m_pView->m_pCharacter->GetCurFrame()+0.5f) );
		UpdateEventPosition( pEvent );
	}
}


void CDlgEventList::UpdateEventPosition( CEvent* pEvent )
{
	vector3d v;
	s32 CurFrame = (s32)(m_pView->m_pCharacter->GetCurFrame()+0.5f);

	if( !pEvent )
		return;

	if( !pEvent->UpdateEventPosition() )// v, m_pView->GetSkeleton(), m_pView->GetCurMotion() ) )
	{
		m_pView->m_Camera.GetTarget( &v );
		pEvent->SetPosition( &v );
	}

	m_pView->GetSkeleton()->SetPose( m_pView->GetCurMotion(), (f32)CurFrame );
}

void CDlgEventList::OnButtonAddCurrentEvent() 
{
/*
	CEvent* pCurEvent = NULL;
	ASSERT( m_pView );

	pCurEvent = m_pView->EventGetCurrent();
	if( pCurEvent )
	{
		if( pCurEvent->GetBone() == -1 )
		{
			AddCurrentEventProjectList( pCurEvent );
		}
		else
		{
			AddCurrentEventAttachedList( pCurEvent );
		}
		m_pView->GetDocument()->UpdateAllViews( NULL );
		m_pView->GetDocument()->SetModifiedFlag();
	}
*/
}

void CDlgEventList::AddCurrentEventProjectList( CEvent* pAddEvent )
{
	if( pAddEvent )
	{
		POSITION pos;

		pos = m_pView->GetProjectEvents()->GetPositionbyName( pAddEvent->GetName() );
		if( !pos )
		{
			CEvent* pEvent;

			pEvent = new CEvent;
			if( pEvent )
			{
				pEvent->SetName( pAddEvent->GetName() );
				pEvent->SetRadius( pAddEvent->GetRadius() );
				pEvent->SetBone( pAddEvent->GetBone() );
				m_pView->GetProjectEvents()->AddTail( pEvent );
			}
		}
		else
		{
			float Radius;
			CString Message;
			
			{
				CString RadiusString;

				UpdateData( TRUE );
				m_R.GetWindowText( RadiusString );
				Radius = (float)atof( RadiusString );
			}
			Message.Format( "Event already exists in the list.  Do you want to update the default event radius to %.3f?", Radius );
			if( MessageBox( Message, "Event Insertion", 
				MB_YESNO | MB_ICONQUESTION ) == IDYES )
			{
				CEvent* pEvent = NULL;

				pEvent = (CEvent *)m_pView->GetProjectEvents()->GetAt( pos );
				ASSERT( pEvent );
				pEvent->SetRadius( Radius );
			}
		}
	}
}

void CDlgEventList::AddCurrentEventAttachedList(CEvent *pAddEvent)
{
	if( pAddEvent )
	{
		POSITION pos;

		pos = m_pView->GetAttachedEvents()->GetPositionbyName( pAddEvent->GetName() );
		if( !pos )
		{
			CEvent* pEvent;

			pEvent = new CEvent;
			if( pEvent )
			{
				pEvent->SetName( pAddEvent->GetName() );
				pEvent->SetRadius( pAddEvent->GetRadius() );
				pEvent->SetBone( pAddEvent->GetBone() );
				m_pView->GetAttachedEvents()->AddTail( pEvent );
			}
		}
		else
		{
			float Radius;
			CString Message;
			
			{
				CString RadiusString;

				UpdateData( TRUE );
				m_R.GetWindowText( RadiusString );
				Radius = (float)atof( RadiusString );
			}
			Message.Format( "Event already exists in the list.  Do you want to update the default event radius to %.3f?", Radius );
			if( MessageBox( Message, "Event Insertion", 
				MB_YESNO | MB_ICONQUESTION ) == IDYES )
			{
				CEvent* pEvent = NULL;

				pEvent = (CEvent *)m_pView->GetAttachedEvents()->GetAt( pos );
				ASSERT( pEvent );
				pEvent->SetRadius( Radius );
			}
		}
	}
}

void CDlgEventList::OnButtonAttachBone() 
{
	CAttachJointDlg AttachJointDlg;
	
	ASSERT( m_pView );
	AttachJointDlg.SetView( m_pView );
	AttachJointDlg.SetEvent( m_pView->m_pCharacter->m_pCurMotion->m_pCurEvent );
	m_pView->Invalidate();

	AttachJointDlg.DoModal();
}

void CDlgEventList::OnButtonAttachBoneNow() 
{
	CAttachJointDlg AttachJointDlg;
	
	ASSERT( m_pView );
	AttachJointDlg.SetView( m_pView );
	AttachJointDlg.SetEvent( m_pView->m_pCharacter->m_pCurMotion->m_pCurEvent );
	AttachJointDlg.SetImmediate();
	m_pView->Invalidate();

	AttachJointDlg.DoModal();
}

BOOL CDlgEventList::OnToolTipNotify( UINT id, NMHDR * pTTTStruct, LRESULT * pResult )
{
    TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pTTTStruct;    
    UINT nID = pTTTStruct->idFrom;

    if (pTTT->uFlags & TTF_IDISHWND)    
    {
        // idFrom is actually the HWND of the tool
        nID = ::GetDlgCtrlID((HWND)nID);        
        if(nID)        
        {
            pTTT->lpszText = MAKEINTRESOURCE(nID);
            pTTT->hinst = AfxGetResourceHandle();            
            return TRUE;
        }    
    }    return FALSE;
}
