// DlgCamera1.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "DlgCamera1.h"

#include "MotionEditView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCamera1 dialog


CDlgCamera1::CDlgCamera1(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgCamera1::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCamera1)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pView = NULL ;
	m_EditsLocked = false ;
}


void CDlgCamera1::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCamera1)
	DDX_Control(pDX, IDC_TIMEACCURATE, m_TimeAccurate);
	DDX_Control(pDX, IDC_CAMERA_TARGETZ, m_Z);
	DDX_Control(pDX, IDC_CAMERA_TARGETY, m_Y);
	DDX_Control(pDX, IDC_CAMERA_TARGETX, m_X);
	DDX_Control(pDX, IDC_CAMERA_ROTATION, m_Rotation);
	DDX_Control(pDX, IDC_CAMERA_ELEVATION, m_Elevation);
	DDX_Control(pDX, IDC_CAMERA_BONE, m_Bone);
	//}}AFX_DATA_MAP
}


void CDlgCamera1::SetTarget( vector3d *pVector )
{
	m_EditsLocked = true ;
	MFCU_UpdateEditControl( m_X, pVector->X ) ;
	MFCU_UpdateEditControl( m_Y, pVector->Y ) ;
	MFCU_UpdateEditControl( m_Z, pVector->Z ) ;
	m_EditsLocked = false ;
}

void CDlgCamera1::SetRotation( f32 Rotation )
{
	m_EditsLocked = true ;
	MFCU_UpdateEditControl( m_Rotation, RAD_TO_DEG(Rotation) ) ;
	m_EditsLocked = false ;
}

void CDlgCamera1::SetElevation( f32 Elevation )
{
	m_EditsLocked = true ;
	MFCU_UpdateEditControl( m_Elevation, RAD_TO_DEG(Elevation) ) ;
	m_EditsLocked = false ;
}

void CDlgCamera1::Reset( )
{
	m_Bone.ResetContent() ;
	m_Bone.AddString( "<no bone>" ) ;
}

void CDlgCamera1::EnableBoneList( bool Enable )
{
	m_Bone.EnableWindow( Enable ) ;
}

void CDlgCamera1::AddBone( CString Name )
{
	int Pos = m_Bone.AddString( Name ) ;
	ASSERT( Pos != CB_ERR ) ;
	ASSERT( Pos != CB_ERRSPACE ) ;
}

void CDlgCamera1::SelectBone( int iBone )
{
	m_Bone.SetCurSel( iBone ) ;
}



BEGIN_MESSAGE_MAP(CDlgCamera1, CDialog)
	//{{AFX_MSG_MAP(CDlgCamera1)
	ON_EN_CHANGE(IDC_CAMERA_TARGETX, OnChangeCameraTargetx)
	ON_EN_CHANGE(IDC_CAMERA_TARGETY, OnChangeCameraTargety)
	ON_EN_CHANGE(IDC_CAMERA_TARGETZ, OnChangeCameraTargetz)
	ON_EN_CHANGE(IDC_CAMERA_ROTATION, OnChangeCameraRotation)
	ON_EN_CHANGE(IDC_CAMERA_ELEVATION, OnChangeCameraElevation)
	ON_CBN_SELCHANGE(IDC_CAMERA_BONE, OnSelchangeCameraBone)
	ON_BN_CLICKED(IDC_TIMEACCURATE, OnTimeaccurate)
	//}}AFX_MSG_MAP

	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDCANCEL, OnCancel)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCamera1 message handlers

void CDlgCamera1::OnOk() 
{
}

void CDlgCamera1::OnCancel() 
{
}

void CDlgCamera1::OnChangeCameraTargetx() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here

	if( !m_EditsLocked )
	{
		vector3d	Target ;
		m_pView->m_Camera.GetTarget( &Target ) ;

		CString		String ;
		m_X.GetWindowText( String ) ;

		Target.X = (f32)atof( String ) ;
		m_pView->m_Camera.SetTarget( &Target ) ;
		m_pView->m_Camera.ComputeMatrix() ;
		m_pView->Invalidate() ;
	}
}

void CDlgCamera1::OnChangeCameraTargety() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_EditsLocked )
	{
		vector3d	Target ;
		m_pView->m_Camera.GetTarget( &Target ) ;

		CString		String ;
		m_Y.GetWindowText( String ) ;

		Target.Y = (f32)atof( String ) ;
		m_pView->m_Camera.SetTarget( &Target ) ;
		m_pView->m_Camera.ComputeMatrix() ;
		m_pView->Invalidate() ;
	}
}

void CDlgCamera1::OnChangeCameraTargetz() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_EditsLocked )
	{
		vector3d	Target ;
		m_pView->m_Camera.GetTarget( &Target ) ;

		CString		String ;
		m_Z.GetWindowText( String ) ;

		Target.Z = (f32)atof( String ) ;
		m_pView->m_Camera.SetTarget( &Target ) ;
		m_pView->m_Camera.ComputeMatrix() ;
		m_pView->Invalidate() ;
	}
}

void CDlgCamera1::OnChangeCameraRotation() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_EditsLocked )
	{
		CString		String ;
		m_Rotation.GetWindowText( String ) ;

		radian Rotation = DEG_TO_RAD((f32)atof( String )) ;
		m_pView->m_Camera.SetRotation( Rotation ) ;
		m_pView->m_Camera.ComputeMatrix() ;
		m_pView->Invalidate() ;
	}
}

void CDlgCamera1::OnChangeCameraElevation() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
	if( !m_EditsLocked )
	{
		CString		String ;
		m_Elevation.GetWindowText( String ) ;

		radian Elevation = DEG_TO_RAD((f32)atof( String )) ;
		m_pView->m_Camera.SetElevation( Elevation ) ;
		m_pView->m_Camera.ComputeMatrix() ;
		m_pView->Invalidate() ;
	}
}

void CDlgCamera1::OnSelchangeCameraBone() 
{
	// TODO: Add your control notification handler code here
	ASSERT( m_pView ) ;
	m_pView->SetCameraBone( m_Bone.GetCurSel() ) ;
}

void CDlgCamera1::OnTimeaccurate() 
{
	// TODO: Add your control notification handler code here
	ASSERT( m_pView ) ;
	m_pView->SetTimeAccuratePlayback( m_TimeAccurate.GetState( ) & 0x03 ) ;
}
