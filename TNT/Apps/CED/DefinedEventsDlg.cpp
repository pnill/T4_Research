// DefinedEventsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "DefinedEventsDlg.h"
#include "MFCutils.h"
#include "MotionEditView.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDefinedEventsDlg dialog


CDefinedEventsDlg::CDefinedEventsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDefinedEventsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDefinedEventsDlg)
	//}}AFX_DATA_INIT
	m_pDoc							= NULL;
	m_bAttachedEventListAvailable	= FALSE;
	m_bAttachedEventListSelected	= FALSE;
	m_pSkeleton						= NULL;
	m_EditEventActive				= FALSE;
}


void CDefinedEventsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDefinedEventsDlg)
	DDX_Control(pDX, IDC_DEFINED_EVENTS_DLG_BUTTON_LINKED, m_Linked);
	DDX_Control(pDX, IDC_STATIC_DEFAULT_RADIUS, m_StaticEditRadius);
	DDX_Control(pDX, IDC_DEFINED_EVENTS_DLG_BUTTON_ATTACHED, m_Attached);
	DDX_Control(pDX, IDC_DEFINED_EVENTS_DLG_BUTTON_RELATIVETOJOINT, m_AttachedForcedRelativeToJoint);
	DDX_Control(pDX, IDC_DEFINED_EVENTS_DLG_STATIC_ROTZ, m_StaticAttachedRotZ);
	DDX_Control(pDX, IDC_DEFINED_EVENTS_DLG_STATIC_ROTY, m_StaticAttachedRotY);
	DDX_Control(pDX, IDC_DEFINED_EVENTS_DLG_STATIC_ROTX, m_StaticAttachedRotX);
	DDX_Control(pDX, IDC_DEFINED_EVENTS_DLG_STATIC_DISTANCE, m_StaticAttachedDistance);
	DDX_Control(pDX, IDC_DEFINED_EVENTS_DLG_FORCEPOSZ, m_AttachedForcePosZ);
	DDX_Control(pDX, IDC_DEFINED_EVENTS_DLG_FORCEPOSY, m_AttachedForcePosY);
	DDX_Control(pDX, IDC_DEFINED_EVENTS_DLG_FORCEPOSX, m_AttachedForcePosX);
	DDX_Control(pDX, IDC_DEFINED_EVENTS_DLG_BUTTON_FORCEPOSZ, m_AttachedForcePosZBut);
	DDX_Control(pDX, IDC_DEFINED_EVENTS_DLG_BUTTON_FORCEPOSY, m_AttachedForcePosYBut);
	DDX_Control(pDX, IDC_DEFINED_EVENTS_DLG_BUTTON_FORCEPOSX, m_AttachedForcePosXBut);
	DDX_Control(pDX, IDC_DEFINED_EVENTS_DLG_ROTZ, m_AttachedRotZ);
	DDX_Control(pDX, IDC_DEFINED_EVENTS_DLG_ROTY, m_AttachedRotY);
	DDX_Control(pDX, IDC_DEFINED_EVENTS_DLG_ROTX, m_AttachedRotX);
	DDX_Control(pDX, IDC_DEFINED_EVENTS_DLG_DISTANCE, m_AttachedDistance);
	DDX_Control(pDX, IDC_BUTTON_PROJECT_EVENT_DELETE, m_ButtonEventDelete);
	DDX_Control(pDX, IDC_BUTTON_PROJECT_EVENT_EXPORT_LIST, m_ButtonExportList);
	DDX_Control(pDX, IDC_BUTTON_PROJECT_EVENT_IMPORT_LIST, m_ButtonImportList);
	DDX_Control(pDX, IDC_BUTTON_PROJECT_INSERT_EVENT, m_ButtonInsertEvent);
	DDX_Control(pDX, IDC_DEFINED_EVENTS_DLG_COMBO_ATTACHED_BONE, m_ComboAttachedBone);
	DDX_Control(pDX, IDC_LIST_PROJECT_EVENTS, m_EventListCtrl);
	DDX_Control(pDX, IDC_EDIT_PROJECT_EVENT_NAME, m_EditEventName);
	DDX_Control(pDX, IDC_EDIT_PROJECT_EVENT_DEFAULT_RADIUS, m_EditRadius);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDefinedEventsDlg, CDialog)
	//{{AFX_MSG_MAP(CDefinedEventsDlg)
	ON_BN_CLICKED(IDMYOK, OnMyok)
	ON_EN_KILLFOCUS(IDC_EDIT_PROJECT_EVENT_DEFAULT_RADIUS, OnKillfocusEditProjectEventDefaultRadius)
	ON_BN_CLICKED(IDC_BUTTON_PROJECT_INSERT_EVENT, OnButtonInsertEvent)
	ON_BN_CLICKED(IDC_BUTTON_PROJECT_EVENT_DELETE, OnButtonEventDelete)
	ON_BN_CLICKED(IDC_BUTTON_PROJECT_EVENT_IMPORT_LIST, OnButtonProjectEventImportList)
	ON_BN_CLICKED(IDC_BUTTON_PROJECT_EVENT_EXPORT_LIST, OnButtonProjectEventExportList)
	ON_CBN_SELCHANGE(IDC_COMBO_SELECT_ACTIVE_EVENT_LIST, OnSelchangeComboSelectActiveEventList)
	ON_NOTIFY_EX( TTN_NEEDTEXT, 0, OnToolTipNotify )
	ON_EN_CHANGE(IDC_EDIT_PROJECT_EVENT_NAME, OnChangeDefinedEventsDlgName)
	ON_EN_CHANGE(IDC_DEFINED_EVENTS_DLG_ROTX, OnChangeDefinedEventsDlgRotx)
	ON_EN_CHANGE(IDC_DEFINED_EVENTS_DLG_ROTY, OnChangeDefinedEventsDlgRoty)
	ON_EN_CHANGE(IDC_DEFINED_EVENTS_DLG_ROTZ, OnChangeDefinedEventsDlgRotz)
	ON_EN_CHANGE(IDC_DEFINED_EVENTS_DLG_DISTANCE, OnChangeDefinedEventsDlgDistance)
	ON_EN_CHANGE(IDC_DEFINED_EVENTS_DLG_FORCEPOSX, OnChangeDefinedEventsDlgForceposx)
	ON_EN_CHANGE(IDC_DEFINED_EVENTS_DLG_FORCEPOSY, OnChangeDefinedEventsDlgForceposy)
	ON_EN_CHANGE(IDC_DEFINED_EVENTS_DLG_FORCEPOSZ, OnChangeDefinedEventsDlgForceposz)
	ON_BN_CLICKED(IDC_DEFINED_EVENTS_DLG_BUTTON_FORCEPOSX, OnDefinedEventsDlgButtonForceposx)
	ON_BN_CLICKED(IDC_DEFINED_EVENTS_DLG_BUTTON_FORCEPOSY, OnDefinedEventsDlgButtonForceposy)
	ON_BN_CLICKED(IDC_DEFINED_EVENTS_DLG_BUTTON_FORCEPOSZ, OnDefinedEventsDlgButtonForceposz)
	ON_BN_CLICKED(IDC_DEFINED_EVENTS_DLG_BUTTON_RELATIVETOJOINT, OnDefinedEventsDlgButtonRelativetojoint)
	ON_BN_CLICKED(IDC_DEFINED_EVENTS_DLG_BUTTON_ATTACHED, OnButtonAttached)
	ON_BN_CLICKED(IDC_DEFINED_EVENTS_DLG_BUTTON_LINKED, OnButtonLinked)
	ON_CBN_SELCHANGE(IDC_DEFINED_EVENTS_DLG_COMBO_ATTACHED_BONE, OnSelchangeComboSelectAttachedBone)
	ON_LBN_SELCHANGE(IDC_LIST_PROJECT_EVENTS, OnSelchangeLinstProjectEvents)
	ON_MESSAGE(MSM_MOUSEMOVE, OnMouseStaticMove)
	ON_BN_CLICKED(IDC_EDITEVENT_ACTIVATE, OnEditeventActivate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDefinedEventsDlg message handlers

BOOL CDefinedEventsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_StaticEventR.SubclassDlgItem( IDC_STATIC_RADIUS, this ) ;
	m_StaticEventR.SetNotifyWindow( this ) ;

	m_ButtonCancel.AutoLoad( IDCANCEL, this );
	m_ButtonOK.AutoLoad( IDMYOK, this );

	UpdateProjectEventList();
	UpdateAttachedEventList();
	InitActiveEventListCombo();

	SetInitialActiveEventListSelection();

	InitEventListCtrl();
	UpdateEventListCtrl();
	if( m_EventListCtrl.GetCount() > 0 )
		m_EventListCtrl.SetCurSel( 0 );

	InitUI();

	EnableToolTips( TRUE );

	m_ToolTipCtrl.Create( this );
	m_ToolTipCtrl.AddTool( &m_EditEventName, IDC_EDIT_PROJECT_EVENT_NAME );
	m_ToolTipCtrl.AddTool( &m_EditRadius, IDC_EDIT_PROJECT_EVENT_DEFAULT_RADIUS );
	m_ToolTipCtrl.AddTool( &m_EventListCtrl, IDC_LIST_PROJECT_EVENTS );
	m_ToolTipCtrl.AddTool( &m_ComboAttachedBone, IDC_DEFINED_EVENTS_DLG_COMBO_ATTACHED_BONE ); // TODO:  Not working
	m_ToolTipCtrl.AddTool( &m_ButtonInsertEvent, IDC_BUTTON_PROJECT_INSERT_EVENT );
	m_ToolTipCtrl.AddTool( &m_ButtonImportList, IDC_BUTTON_PROJECT_EVENT_IMPORT_LIST );
	m_ToolTipCtrl.AddTool( &m_ButtonExportList, IDC_BUTTON_PROJECT_EVENT_EXPORT_LIST );
	m_ToolTipCtrl.AddTool( &m_ButtonEventDelete, IDC_BUTTON_PROJECT_EVENT_DELETE );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDefinedEventsDlg::OnOK() 
{
	// Do NOT call baseclass OnOk function.  We don't want the user to be able
	// to close the dialog by pressing return.
}

void CDefinedEventsDlg::OnMyok() 
{
	OnokProjectEventList();
	OnokAttachedEventList();

	CDialog::OnOK();
}

LONG CDefinedEventsDlg::OnMouseStaticMove( UINT wParam, LONG lParam )
{
	float dx;
	float dy;

	dx = (float)( (SHORT)( ( lParam >> 16 ) & 0xffff ) );
	dy = (float)( -(SHORT)( lParam & 0xffff ) ) ;
	dx /= 10 ;
	dy /= 10 ;

	switch( wParam )
	{
		case IDC_STATIC_RADIUS:
			{
				CString RadiusString;

				m_Radius += ( dx + dy );
				RadiusString.Format( "%0.2f", m_Radius );
				MFCU_UpdateEditControl( m_EditRadius, RadiusString );
				break ;
			}
	}

	return 0 ;
}

void CDefinedEventsDlg::OnKillfocusEditProjectEventDefaultRadius() 
{
	float Radius;
	CString RadiusString;

	CEvent* pEvent = (CEvent*)m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() );
	if( !pEvent )
		return;

	m_EditRadius.GetWindowText( RadiusString );
    Radius = (float)(f32)atof( RadiusString );
	if( !((Radius == 0) && (RadiusString != "0")) )
    {
		m_Radius = Radius;
		pEvent->SetRadius( Radius );
    }
	RadiusString.Format( "%0.2f", m_Radius );
	MFCU_UpdateEditControl( m_EditRadius, RadiusString );
    UpdateData( FALSE );
}

void CDefinedEventsDlg::InitInsertEventUI()
{
	CString RadiusString;

	m_Radius = 5.0f;

	RadiusString.Format( "%0.2f", m_Radius );
	MFCU_UpdateEditControl( m_EditRadius, RadiusString );
	m_EditEventName.SetWindowText( "" );
}

void CDefinedEventsDlg::OnButtonInsertEvent() 
{
	if( m_bAttachedEventListSelected )
	{
		OnButtonAttachedInsertEvent();
	}
	else
	{
		OnButtonProjectInsertEvent();
	}

	OnEditeventActivate();
	CEdit* pEdit = (CEdit*)GetDlgItem( IDC_EDIT_PROJECT_EVENT_NAME );
	pEdit->SetFocus();
	pEdit->SetSel( 0, 1000 );
}

void CDefinedEventsDlg::SetDocument( CCeDDoc* pDoc )
{
	m_pDoc = pDoc;
}

void CDefinedEventsDlg::UpdateProjectEventList()
{
	CEventList* pProjectEventList;

	ASSERT( m_pDoc );

	pProjectEventList = m_pDoc->GetProjectEventList();

	ASSERT( pProjectEventList );

	m_ProjectEventList = *pProjectEventList;

	POSITION pos = m_ProjectEventList.GetHeadPosition();
	while( pos )
	{
		CEvent* pEvent = m_ProjectEventList.GetNext( pos );
		pEvent->SetModified( FALSE );
		pEvent->SetOldName( pEvent->GetName() );
	}
}

void CDefinedEventsDlg::UpdateAttachedEventList()
{
	CMDIFrameWnd *pFrame;

	pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
	if( !pFrame )
		return;
	
	CMDIChildWnd *pChild;

	pChild = pFrame->MDIGetActive();
	if( !pChild )
		return;

	if( (pChild->GetActiveView())->IsKindOf( RUNTIME_CLASS( CMotionEditView ) ) )
	{
		// Get the active view attached to the active MDI child// window.
		CMotionEditView *pView;
		CEventList* pAttachedEventList;

		pView = (CMotionEditView *)pChild->GetActiveView();
		pAttachedEventList = pView->GetAttachedEvents();
		m_AttachedEventList = *pAttachedEventList;
		m_pSkeleton = pView->GetSkeleton();
		m_bAttachedEventListAvailable = TRUE;

		POSITION pos = m_AttachedEventList.GetHeadPosition();
		while( pos )
		{
			CEvent* pEvent = m_AttachedEventList.GetNext( pos );
			pEvent->SetModified( FALSE );
			pEvent->SetOldName( pEvent->GetName() );
		}
	}
}

void CDefinedEventsDlg::OnButtonEventDelete() 
{
	if( m_bAttachedEventListSelected )
	{
		AttachedEventDelete();		
	}
	else
	{
		ProjectEventDelete();
	}
}

void CDefinedEventsDlg::OnButtonProjectEventImportList() 
{
	if( m_bAttachedEventListSelected || m_bAttachedEventListAvailable)
	{
		AttachedImportLoadFileDialog();		
	}
	else
	{
		ProjectImportLoadFileDialog();
	}
}

void CDefinedEventsDlg::OnButtonProjectEventExportList() 
{
	if( m_bAttachedEventListSelected || m_bAttachedEventListAvailable)
	{
		AttachedExportSaveFileDialog();
	}
	else
	{
		ProjectExportSaveFileDialog();
	}
}

BOOL CDefinedEventsDlg::ImportProjectEventList(CString FileName)
{
	CFile ImportFile;
	BOOL ReturnValue = FALSE;

	if( ImportFile.Open( FileName, CFile::modeRead, NULL ) )
	{
		CArchive ar( &ImportFile, CArchive::load );
		s32 Length = ar.GetFile()->GetLength();
		u8 *Buffer = new u8[Length];
		ASSERT (Buffer);
		ar.Read( Buffer, Length );
		CMemFile mf( Buffer, Length );
		CieArchive a( &mf, FALSE, NULL );
		BOOL done = FALSE;

		if( Length <= 0 )
		{
			done = TRUE;
		}

		while( !done )
		{
			CString String;
			a.ReadTag();
			switch( a.m_rTag )
			{
				///////////////////////////////////////////////////////////////////////////
				//	SYSTEM ENTRIES
				case IE_TAG_ARCHIVE_NAME:
					{
						a.ReadString( String );
						if( String != "Project Event List" )
						{
							done = TRUE;
						}
						break ;
					}
				case IE_TAG_END:
					{
						done = TRUE;
						break;
					}

				///////////////////////////////////////////////////////////////////////////
				//	DOCUMENT ENTRIES
				case IE_TAG_EVENTLIST:
					{
						m_ProjectEventList.Serialize( a );
						ReturnValue = TRUE;
						break;
					}

				default:
					{
						a.ReadSkip();
						break;
					}
			}
		}

		//---	Close Files & Buffers, etc.
		mf.Detach ();
		delete [] Buffer;
	}

	return ReturnValue;
}

BOOL CDefinedEventsDlg::ExportProjectEventList(CString FileName)
{
	CFile ExportFile;

	if( ExportFile.Open( FileName, CFile::modeCreate | CFile::modeWrite, NULL ) )
	{
		CArchive ar( &ExportFile, CArchive::store );
		CMemFile mf;
		CieArchive a( &mf, TRUE, NULL );

		//---	Write Header Tags
		a.WriteTaggedString( IE_TAG_ARCHIVE_TYPE, CString( "Iguana Character Editor Project Event List" ) );
		a.WriteTaggedString( IE_TAG_ARCHIVE_NAME, CString( "Project Event List" ) );
		a.WriteTaggedu32( IE_TAG_ARCHIVE_VERSION, 0x00010000 );

		{
			SYSTEMTIME Date;
			GetLocalTime( &Date );
			CString DateString;
			int DateLen = GetDateFormat( LOCALE_USER_DEFAULT, DATE_LONGDATE, &Date, NULL, DateString.GetBuffer(256), 256 );
			DateString.ReleaseBuffer( DateLen );
			a.WriteTaggedString( IE_TAG_ARCHIVE_DATE, DateString );
		}

		m_ProjectEventList.Serialize( a );

		//---	Close Archive
		a.WriteTag( IE_TYPE_END,IE_TAG_END );

		//---	Write to Disk File
		s32 Length = mf.GetLength();
		u8 *Buffer = mf.Detach();
		ar.Write( Buffer, Length );
		x_free( Buffer );

		return TRUE;
	}

	return FALSE;
}

void CDefinedEventsDlg::OnSelchangeComboSelectAttachedBone()
{
	CEvent* pEvent = (CEvent*)m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() );
	ASSERT( pEvent );

	if( m_ComboAttachedBone.GetCurSel() == -1 )
	{
		pEvent->SetBone( -1 );
		pEvent->SetBoneName( CString("<no name>") );
		return;
	}

	CString BoneName;
	m_ComboAttachedBone.GetLBText( m_ComboAttachedBone.GetCurSel(), BoneName );
	pEvent->SetBoneName( BoneName );
}

void CDefinedEventsDlg::OnSelchangeComboSelectActiveEventList() 
{
	CComboBox* pEventListCombo;
	
	pEventListCombo = (CComboBox *)GetDlgItem( IDC_COMBO_SELECT_ACTIVE_EVENT_LIST );
	ASSERT( pEventListCombo );
	m_bAttachedEventListSelected = pEventListCombo->GetCurSel();
	ReInitDialog();
}

void CDefinedEventsDlg::SetInitialActiveEventListSelection()
{
	CComboBox* pEventListCombo;
	
	pEventListCombo = (CComboBox *)GetDlgItem( IDC_COMBO_SELECT_ACTIVE_EVENT_LIST );
	ASSERT( pEventListCombo );
	pEventListCombo->SetCurSel( 0 );
}

void CDefinedEventsDlg::InitActiveEventListCombo()
{
	CComboBox* pEventListCombo;
	
	pEventListCombo = (CComboBox *)GetDlgItem( IDC_COMBO_SELECT_ACTIVE_EVENT_LIST );
	ASSERT( pEventListCombo );
	pEventListCombo->InsertString( 0, "Project Global Event List - Unattached to Joints" );
	pEventListCombo->InsertString( 1, "Character Specific Events - Attached to Joints" );
}

void CDefinedEventsDlg::ReInitDialog()
{
	InitEventListCtrl();
	UpdateEventListCtrl();
	if( m_EventListCtrl.GetCount() > 0 )
		m_EventListCtrl.SetCurSel( 0 );
	InitUI();
//	UpdateBoneUI();
}

void CDefinedEventsDlg::UpdateEventListCtrl()
{
	if( m_bAttachedEventListSelected )
	{
		UpdateAttachedEventListCtrl();
	}
	else
	{
		UpdateProjectEventListCtrl();
	}
}

void CDefinedEventsDlg::InitEventListCtrl()
{
	if( m_bAttachedEventListSelected )
	{
		InitAttachedEventListCtrl();
	}
	else
	{
		InitProjectEventListCtrl();
	}
}

void CDefinedEventsDlg::InitProjectEventListCtrl()
{
}

void CDefinedEventsDlg::InitAttachedEventListCtrl()
{
}


void CDefinedEventsDlg::UpdateProjectEventListCtrl()
{
	POSITION pos;
	pos = m_ProjectEventList.GetHeadPosition();

	m_EventListCtrl.ResetContent();

	while( pos )
	{
		CEvent* pEvent = m_ProjectEventList.GetNext( pos );
		s32 Index = m_EventListCtrl.AddString( pEvent->GetName() );
		m_EventListCtrl.SetItemData( Index, (u32)pEvent );
	}

    UpdateData( FALSE );	
}

void CDefinedEventsDlg::UpdateAttachedEventListCtrl()
{
	POSITION pos;
	pos = m_AttachedEventList.GetHeadPosition();

	m_EventListCtrl.ResetContent();

	while( pos )
	{
		CEvent* pEvent = m_AttachedEventList.GetNext( pos );
		s32 Index = m_EventListCtrl.AddString( pEvent->GetName() );
		m_EventListCtrl.SetItemData( Index, (u32)pEvent );
	}

    UpdateData( FALSE );	
}

void CDefinedEventsDlg::InitUI()
{
	InitBoneCombo();

	if( m_bAttachedEventListSelected )
	{
		InitAttachedUI();
	}
	else
	{
		InitProjectUI();
	}
}

void CDefinedEventsDlg::InitBoneCombo()
{
	INT nBones;
	INT iBone;

	m_ComboAttachedBone.ResetContent();

	if( m_bAttachedEventListSelected && m_bAttachedEventListAvailable && ( m_pSkeleton != NULL ) )
	{
		nBones = m_pSkeleton->GetNumBones();
		for( iBone = 0; iBone < nBones; iBone++ )
		{
			INT nBoneParents;
			INT iBoneParent;
			CString BoneName;
			int Pos;

			nBoneParents = m_pSkeleton->GetBoneNumParents( m_pSkeleton->GetBoneFromIndex( iBone ) );
			for( iBoneParent = 0; iBoneParent < nBoneParents; iBoneParent++ )
			{
				BoneName += " ";
			}
			BoneName += m_pSkeleton->GetBoneFromIndex( iBone )->BoneName;
			Pos = m_ComboAttachedBone.AddString( BoneName );
			ASSERT( Pos != CB_ERR ) ;
			ASSERT( Pos != CB_ERRSPACE ) ;
		}
	}
	else if( !m_bAttachedEventListSelected )
	{
		POSITION pos=m_pDoc->m_CharacterList.GetHeadPosition();
		while( pos )
		{
			CCharacter* pCharacter = (CCharacter*) m_pDoc->m_CharacterList.GetNext( pos );
			CSkeleton* pSkeleton = (CSkeleton*) pCharacter->GetSkeleton();
			s32 nBones = pSkeleton->GetNumBones();
			for( iBone=0; iBone<nBones; iBone++ )
			{
				CString BoneName = pSkeleton->GetBoneFromIndex( iBone )->BoneName;
				if( m_ComboAttachedBone.FindStringExact( 0, BoneName ) == -1 )
				{
					s32 Index = m_ComboAttachedBone.AddString( BoneName );
					ASSERT( Index != CB_ERR ) ;
					ASSERT( Index != CB_ERRSPACE ) ;
				}
			}
		}
	}

	m_ComboAttachedBone.SetCurSel( 0 );
}

void CDefinedEventsDlg::InitAttachedUI()
{
	GetDlgItem( IDC_STATIC_INSERT_NEW_EVENT )->EnableWindow( m_bAttachedEventListAvailable );
	GetDlgItem( IDC_STATIC_EVENT_NAME )->EnableWindow( m_bAttachedEventListAvailable );
	GetDlgItem( IDC_STATIC_DEFAULT_RADIUS )->EnableWindow( m_bAttachedEventListAvailable );
	m_ButtonInsertEvent.EnableWindow( m_bAttachedEventListAvailable );
	GetDlgItem( IDC_EDIT_PROJECT_EVENT_DEFAULT_RADIUS )->EnableWindow( m_bAttachedEventListAvailable );
	GetDlgItem( IDC_STATIC_IMPORT_EXPORT )->EnableWindow( m_bAttachedEventListAvailable );
	m_ButtonImportList.EnableWindow( m_bAttachedEventListAvailable );
	m_ButtonExportList.EnableWindow( m_bAttachedEventListAvailable );
	GetDlgItem( IDC_STATIC_EVENT_LIST )->EnableWindow( m_bAttachedEventListAvailable );
	m_ButtonEventDelete.EnableWindow( m_bAttachedEventListAvailable );
	m_ComboAttachedBone.EnableWindow( m_bAttachedEventListAvailable );
	m_EventListCtrl.EnableWindow( m_bAttachedEventListAvailable );
	m_EditEventName.EnableWindow( m_bAttachedEventListAvailable );
	m_EditRadius.EnableWindow( m_bAttachedEventListAvailable );

//	InitBoneUI( m_bAttachedEventListAvailable?true:false );
	UpdateUI();
}

void CDefinedEventsDlg::InitProjectUI()
{
	GetDlgItem( IDC_STATIC_INSERT_NEW_EVENT )->EnableWindow( TRUE );
	GetDlgItem( IDC_STATIC_EVENT_NAME )->EnableWindow( TRUE );
	GetDlgItem( IDC_STATIC_DEFAULT_RADIUS )->EnableWindow( TRUE );
	m_ButtonInsertEvent.EnableWindow( TRUE );
	GetDlgItem( IDC_EDIT_PROJECT_EVENT_DEFAULT_RADIUS )->EnableWindow( TRUE );
	GetDlgItem( IDC_STATIC_IMPORT_EXPORT )->EnableWindow( TRUE );
	m_ButtonImportList.EnableWindow( TRUE );
	m_ButtonExportList.EnableWindow( TRUE );
	GetDlgItem( IDC_STATIC_EVENT_LIST )->EnableWindow( TRUE );
	m_ButtonEventDelete.EnableWindow( TRUE );
	m_ComboAttachedBone.EnableWindow( FALSE );
	m_EventListCtrl.EnableWindow( TRUE );
	m_EditEventName.EnableWindow( TRUE );
	m_EditRadius.EnableWindow( TRUE );

//	InitBoneUI( FALSE );
	UpdateUI();
}

void CDefinedEventsDlg::InitBoneUI( bool Active )
{
	m_ComboAttachedBone.EnableWindow( Active );
	GetDlgItem( IDC_STATIC_ATTACHED_BONE )->EnableWindow( Active );
	m_StaticAttachedRotX.EnableWindow( Active );
	m_StaticAttachedRotY.EnableWindow( Active );
	m_StaticAttachedRotZ.EnableWindow( Active );
	m_StaticAttachedDistance.EnableWindow( Active );
	m_AttachedForcePosXBut.EnableWindow( Active );
	m_AttachedForcePosYBut.EnableWindow( Active );
	m_AttachedForcePosZBut.EnableWindow( Active );
	m_AttachedForcePosX.EnableWindow( m_AttachedForcePosXBut.GetCheck() ? Active : FALSE );
	m_AttachedForcePosY.EnableWindow( m_AttachedForcePosYBut.GetCheck() ? Active : FALSE );
	m_AttachedForcePosZ.EnableWindow( m_AttachedForcePosZBut.GetCheck() ? Active : FALSE );
	m_AttachedRotX.EnableWindow( Active );
	m_AttachedRotY.EnableWindow( Active );
	m_AttachedRotZ.EnableWindow( Active );
	m_AttachedDistance.EnableWindow( Active );
	m_Linked.EnableWindow( Active );

	if( m_AttachedForcePosXBut.GetCheck() || m_AttachedForcePosYBut.GetCheck() || m_AttachedForcePosZBut.GetCheck() )
		m_AttachedForcedRelativeToJoint.EnableWindow( Active );
	else
		m_AttachedForcedRelativeToJoint.EnableWindow( FALSE );
}

void CDefinedEventsDlg::UpdateUI( void )
{
	//---	get the selected event
	if ( (m_EventListCtrl.GetCurSel() == -1) ||
		  !m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() ) ||
		  !m_EditEventActive )
	{
		m_EditEventName.EnableWindow( FALSE );
		m_EditRadius.EnableWindow( FALSE );
		m_Attached.EnableWindow( FALSE );
		m_StaticEditRadius.EnableWindow( FALSE );
	}
	else
	{
		CEvent* pEvent = (CEvent*)m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() );
		ASSERT( pEvent );

		m_EditEventName.SetWindowText( pEvent->GetName() );
		MFCU_UpdateEditControl( m_EditRadius, pEvent->GetRadius() );
		m_Attached.SetCheck( pEvent->GetAttached() );

		m_EditEventName.EnableWindow( TRUE );
		m_EditRadius.EnableWindow( TRUE );
		m_Attached.EnableWindow( TRUE );

		m_ComboAttachedBone.EnableWindow( TRUE );
		m_StaticEditRadius.EnableWindow( TRUE );
	}

	UpdateBoneUI();
}

void CDefinedEventsDlg::UpdateBoneUI( void )
{
	//---	get the selected event
	if ( (m_EventListCtrl.GetCurSel() == -1) ||
		 !m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() ) ||
		 !m_EditEventActive )
	{
		InitBoneUI( FALSE );
		return;
	}

	CEvent* pEvent = (CEvent*)m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() );
	ASSERT( pEvent );

	//---	set the dialog values
	m_ComboAttachedBone.SetCurSel( m_bAttachedEventListSelected ? pEvent->GetBone() : m_ComboAttachedBone.FindStringExact( 0, pEvent->GetBoneName() ) );
	MFCU_UpdateEditControl( m_AttachedRotX, pEvent->GetAttRot().X );
	MFCU_UpdateEditControl( m_AttachedRotY, pEvent->GetAttRot().Y );
	MFCU_UpdateEditControl( m_AttachedRotZ, pEvent->GetAttRot().Z );
	MFCU_UpdateEditControl( m_AttachedDistance, pEvent->GetAttDist() );
	MFCU_UpdateEditControl( m_AttachedForcePosX, pEvent->GetAttForcePos().X );
	MFCU_UpdateEditControl( m_AttachedForcePosY, pEvent->GetAttForcePos().Y );
	MFCU_UpdateEditControl( m_AttachedForcePosZ, pEvent->GetAttForcePos().Z );
	m_AttachedForcePosXBut.SetCheck( pEvent->GetAttForceX() ? TRUE : FALSE );
	m_AttachedForcePosYBut.SetCheck( pEvent->GetAttForceY() ? TRUE : FALSE );
	m_AttachedForcePosZBut.SetCheck( pEvent->GetAttForceZ() ? TRUE : FALSE );
	m_AttachedForcedRelativeToJoint.SetCheck( pEvent->GetAttForceRelToJoint() ? TRUE : FALSE );
	m_Linked.SetCheck( pEvent->GetLink() == 1 ? TRUE : FALSE );

	//---	activate the dialog
	InitBoneUI( pEvent->GetAttached() );
}

void CDefinedEventsDlg::OnSelchangeLinstProjectEvents()
{
	UpdateUI();
}

void CDefinedEventsDlg::OnButtonProjectInsertEvent()
{
	CEvent* pEvent;
	pEvent = new CEvent;
	if( pEvent )
		m_ProjectEventList.AddTail( pEvent );

	s32 Index = m_EventListCtrl.AddString( pEvent->GetName() );
	m_EventListCtrl.SetItemData( Index, (u32)pEvent );
	m_EventListCtrl.SetCurSel( Index );

	UpdateUI();
}

void CDefinedEventsDlg::OnButtonAttachedInsertEvent()
{
	CEvent* pEvent;
	pEvent = new CEvent;
	if( pEvent )
		m_AttachedEventList.AddTail( pEvent );

	s32 Index = m_EventListCtrl.AddString( pEvent->GetName() );
	m_EventListCtrl.SetItemData( Index, (u32)pEvent );
	m_EventListCtrl.SetCurSel( Index );

	UpdateUI();
}

void CDefinedEventsDlg::ProjectEventDelete() 
{
	if( m_EventListCtrl.GetCurSel() == -1 )
		return;

	CEvent* pEvent = (CEvent*)m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() );
	ASSERT(pEvent);
	if( !pEvent ) return;

	s32 CurSel = m_EventListCtrl.GetCurSel();

	m_ProjectEventList.Remove( pEvent );
	m_EventListCtrl.DeleteString( CurSel );
	if( m_EventListCtrl.GetCount() > CurSel )
		m_EventListCtrl.SetCurSel( CurSel );
	else
		m_EventListCtrl.SetCurSel( CurSel-1 );

	UpdateProjectEventListCtrl();
}

void CDefinedEventsDlg::AttachedEventDelete() 
{
	if( m_EventListCtrl.GetCurSel() == -1 )
		return;

	CEvent* pEvent = (CEvent*)m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() );
	ASSERT(pEvent);
	if( !pEvent ) return;

	s32 CurSel = m_EventListCtrl.GetCurSel();

	m_AttachedEventList.Remove( pEvent );
	m_EventListCtrl.DeleteString( CurSel );
	if( m_EventListCtrl.GetCount() > CurSel )
		m_EventListCtrl.SetCurSel( CurSel );
	else
		m_EventListCtrl.SetCurSel( CurSel-1 );

	UpdateAttachedEventListCtrl();
}

void CDefinedEventsDlg::SearchForAndChangeModifiedEvents( CEventList* pEventList, CCharacter* pCharacter )
{
	CEvent* pEvent;
	CMotion* pMotion;
	CEvent* pMotionEvent;
	POSITION epos;
	POSITION mpos;
	POSITION mepos;
	CString EventName;


	epos = pEventList->GetHeadPosition();
	while( epos )
	{
		pEvent = pEventList->GetNext( epos );
		if( !pEvent->GetModified() )
			continue;

		EventName = pEvent->GetOldName();
		mpos = pCharacter->m_MotionList.GetHeadPosition();
		while( mpos )
		{
			pMotion = pCharacter->m_MotionList.GetNext( mpos );
			mepos = pMotion->m_EventList.GetHeadPosition();
			while( mepos )
			{
				pMotionEvent = pMotion->m_EventList.GetNext( mepos );
				if( pMotionEvent->GetName() == EventName )
				{
					*pMotionEvent <= *pEvent;
					pMotionEvent->UpdateEventPosition();
					//pMotionEvent->ProjectEventPosition( v, pMotion->m_pCharacter->m_pSkeleton, CMotion *pMotion );
				}
			}
		}
	}
}

void CDefinedEventsDlg::OnokProjectEventList()
{
	CEventList* pProjectEventList;

	ASSERT( m_pDoc );

	pProjectEventList = m_pDoc->GetProjectEventList();

	ASSERT( pProjectEventList );

	*pProjectEventList = m_ProjectEventList;

	//---	now, for each modified event in this list, find the events used in the document and change them to match
	POSITION pos = m_pDoc->m_CharacterList.GetHeadPosition();
	while( pos )
		SearchForAndChangeModifiedEvents( &m_ProjectEventList, m_pDoc->m_CharacterList.GetNext( pos ) );
}

void CDefinedEventsDlg::OnokAttachedEventList()
{
	if( !m_bAttachedEventListAvailable )
		return;

	CMDIFrameWnd *pFrame;
	pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
	if( !pFrame )
		return;

	CMDIChildWnd *pChild;

	pChild = pFrame->MDIGetActive();
	if( !pChild )
		return;

	if( (pChild->GetActiveView())->IsKindOf( RUNTIME_CLASS( CMotionEditView ) ) )
	{
		// Get the active view attached to the active MDI child// window.
		CMotionEditView *pView;
		CEventList* pAttachedEventList;

		pView = (CMotionEditView *)pChild->GetActiveView();
		pAttachedEventList = pView->GetAttachedEvents();
		*pAttachedEventList = m_AttachedEventList;

		//---	now, for each modified event in this list, find the events used in the current character and change them to match
		SearchForAndChangeModifiedEvents( &m_AttachedEventList, pView->m_pCharacter );

		//---	update the event display for the active view
		pView->UpdateEventProjectEvents();

		//---	get the character in the active view to use the correct bone rotations
		pView->m_pCharacter->SetPose();

		pView->Invalidate();
	}
}

void CDefinedEventsDlg::ProjectExportSaveFileDialog()
{
	char            FileName[256];
    OPENFILENAME    FileDialog;

    memset( &FileDialog, 0, sizeof(OPENFILENAME) );
    memset( &FileName, 0, 256 );

    FileDialog.lStructSize       = sizeof(OPENFILENAME);
    FileDialog.hwndOwner         = NULL;
    FileDialog.lpstrFile         = FileName;
    FileDialog.nMaxFile          = 256;
    FileDialog.Flags             = OFN_HIDEREADONLY; 
    FileDialog.lpstrFilter       = "Project Event List File (.pel)\0*.pel\00\0";
    FileDialog.lpstrTitle        = "Export Project Event List File";

    if( GetSaveFileName( &FileDialog ) )
    {
		CString FileNameExt; 
		BeginWaitCursor();

		FileNameExt = FileName;
		if( FileNameExt.Find( '.' ) < 0 )
		{
			FileNameExt += ".pel";
		}
        if( !ExportProjectEventList( FileNameExt ) )
        {
            MessageBox( "An error occured exporting the project event list.", "Event List Export Error", MB_OK | MB_ICONERROR );
        }
    }
	EndWaitCursor();
}

void CDefinedEventsDlg::AttachedExportSaveFileDialog()
{
	char            FileName[256];
    OPENFILENAME    FileDialog;

    memset( &FileDialog, 0, sizeof(OPENFILENAME) );
    memset( &FileName, 0, 256 );

    FileDialog.lStructSize       = sizeof(OPENFILENAME);
    FileDialog.hwndOwner         = NULL;
    FileDialog.lpstrFile         = FileName;
    FileDialog.nMaxFile          = 256;
    FileDialog.Flags             = OFN_HIDEREADONLY; 
    FileDialog.lpstrFilter       = "Attached Event List File (.ael)\0*.ael\00\0";
    FileDialog.lpstrTitle        = "Export Attached Event List File";

    if( GetSaveFileName( &FileDialog ) )
    {
		CString FileNameExt; 
		BeginWaitCursor();

		FileNameExt = FileName;
		if( FileNameExt.Find( '.' ) < 0 )
		{
			FileNameExt += ".ael";
		}
        if( !ExportAttachedEventList( FileNameExt ) )
        {
            MessageBox( "An error occured exporting the attached event list.", "Event List Export Error", MB_OK | MB_ICONERROR );
        }
    }
	EndWaitCursor();
}

BOOL CDefinedEventsDlg::ExportAttachedEventList(CString FileName)
{
	CFile ExportFile;

	if( ExportFile.Open( FileName, CFile::modeCreate | CFile::modeWrite, NULL ) )
	{
		CArchive ar( &ExportFile, CArchive::store );
		CMemFile mf;
		CieArchive a( &mf, TRUE, NULL );

		//---	Write Header Tags
		a.WriteTaggedString( IE_TAG_ARCHIVE_TYPE, CString( "Iguana Character Editor Attached Event List" ) );
		a.WriteTaggedString( IE_TAG_ARCHIVE_NAME, CString( "Attached Event List" ) );
		a.WriteTaggedu32( IE_TAG_ARCHIVE_VERSION, 0x00010000 );

		{
			SYSTEMTIME Date;
			GetLocalTime( &Date );
			CString DateString;
			int DateLen = GetDateFormat( LOCALE_USER_DEFAULT, DATE_LONGDATE, &Date, NULL, DateString.GetBuffer(256), 256 );
			DateString.ReleaseBuffer( DateLen );
			a.WriteTaggedString( IE_TAG_ARCHIVE_DATE, DateString );
		}

		m_AttachedEventList.Serialize( a );

		//---	Close Archive
		a.WriteTag( IE_TYPE_END,IE_TAG_END );

		//---	Write to Disk File
		s32 Length = mf.GetLength();
		u8 *Buffer = mf.Detach();
		ar.Write( Buffer, Length );
		x_free( Buffer );

		return TRUE;
	}

	return FALSE;
}

void CDefinedEventsDlg::ProjectImportLoadFileDialog()
{
	char            FileName[256];
    OPENFILENAME    FileDialog;

    memset( &FileDialog, 0, sizeof(OPENFILENAME) );
    memset( &FileName, 0, 256 );

    FileDialog.lStructSize       = sizeof(OPENFILENAME);
    FileDialog.hwndOwner         = NULL;
    FileDialog.lpstrFile         = FileName;
    FileDialog.nMaxFile          = 256;
    FileDialog.Flags             = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY; 
    FileDialog.lpstrFilter       = "Project Event List File (.pel)\0*.pel\0All Files (*.*)\0*.*\00\0";
    FileDialog.lpstrTitle        = "Import Project Event List File";

    if( GetOpenFileName( &FileDialog ) )
    {
		BeginWaitCursor();
        if( !ImportProjectEventList( FileName ) )
        {
            MessageBox( "An error occured importing the project event list.", "Event List Import Error", MB_OK | MB_ICONERROR );
        }
		else
		{
			UpdateProjectEventListCtrl();
		}
    }
	EndWaitCursor();
}

void CDefinedEventsDlg::AttachedImportLoadFileDialog()
{
	char            FileName[256];
    OPENFILENAME    FileDialog;

    memset( &FileDialog, 0, sizeof(OPENFILENAME) );
    memset( &FileName, 0, 256 );

    FileDialog.lStructSize       = sizeof(OPENFILENAME);
    FileDialog.hwndOwner         = NULL;
    FileDialog.lpstrFile         = FileName;
    FileDialog.nMaxFile          = 256;
    FileDialog.Flags             = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY; 
    FileDialog.lpstrFilter       = "Attached Event List File (.ael)\0*.ael\0All Files (*.*)\0*.*\00\0";
    FileDialog.lpstrTitle        = "Import Attached Event List File";

    if( GetOpenFileName( &FileDialog ) )
    {
		BeginWaitCursor();
        if( !ImportAttachedEventList( FileName ) )
        {
            MessageBox( "An error occured importing the attached event list.", "Event List Import Error", MB_OK | MB_ICONERROR );
        }
		else
		{
			UpdateAttachedEventListCtrl();
		}
    }
	EndWaitCursor();
}

BOOL CDefinedEventsDlg::ImportAttachedEventList(CString FileName)
{
	CFile ImportFile;
	BOOL ReturnValue = FALSE;

	if( ImportFile.Open( FileName, CFile::modeRead, NULL ) )
	{
		CArchive ar( &ImportFile, CArchive::load );
		s32 Length = ar.GetFile()->GetLength();
		u8 *Buffer = new u8[Length];
		ASSERT (Buffer);
		ar.Read( Buffer, Length );
		CMemFile mf( Buffer, Length );
		CieArchive a( &mf, FALSE, NULL );
		BOOL done = FALSE;

		if( Length <= 0 )
		{
			done = TRUE;
		}

		while( !done )
		{
			CString String;
			a.ReadTag();
			switch( a.m_rTag )
			{
				///////////////////////////////////////////////////////////////////////////
				//	SYSTEM ENTRIES
				case IE_TAG_ARCHIVE_NAME:
					{
						a.ReadString( String );
						if( String != "Attached Event List" )
						{
							done = TRUE;
						}
						break ;
					}
				case IE_TAG_END:
					{
						done = TRUE;
						break;
					}

				///////////////////////////////////////////////////////////////////////////
				//	DOCUMENT ENTRIES
				case IE_TAG_EVENTLIST:
					{
						m_AttachedEventList.Serialize( a );
						ReturnValue = TRUE;
						break;
					}

				default:
					{
						a.ReadSkip();
						break;
					}
			}
		}

		//---	Close Files & Buffers, etc.
		mf.Detach ();
		delete [] Buffer;
	}

	return ReturnValue;
}

BOOL CDefinedEventsDlg::OnToolTipNotify( UINT id, NMHDR * pTTTStruct, LRESULT * pResult )
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

BOOL CDefinedEventsDlg::PreTranslateMessage(MSG* pMsg) 
{
	m_ToolTipCtrl.RelayEvent( pMsg );
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CDefinedEventsDlg::OnChangeDefinedEventsDlgRotx()
{
	//---	get the selected event
	if ( (m_EventListCtrl.GetCurSel() == -1) || !m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() ) )
		return;
	CEvent* pEvent = (CEvent*)m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() );
	ASSERT( pEvent );
	if( !pEvent )	return;

	f32 val;
	vector3  rot;
	CString str;

	m_AttachedRotX.GetWindowText( str );
	val = (f32)atof( str );
	rot = pEvent->GetAttRot();
	rot.X = val;
	pEvent->SetAttRot( rot );
}

void CDefinedEventsDlg::OnChangeDefinedEventsDlgRoty()
{
	//---	get the selected event
	if ( (m_EventListCtrl.GetCurSel() == -1) || !m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() ) )
		return;
	CEvent* pEvent = (CEvent*)m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() );
	ASSERT( pEvent );
	if( !pEvent )	return;

	f32 val;
	vector3  rot;
	CString str;

	m_AttachedRotY.GetWindowText( str );
	val = (f32)atof( str );
	rot = pEvent->GetAttRot();
	rot.Y = val;
	pEvent->SetAttRot( rot );
}

void CDefinedEventsDlg::OnChangeDefinedEventsDlgRotz()
{
	//---	get the selected event
	if ( (m_EventListCtrl.GetCurSel() == -1) || !m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() ) )
		return;
	CEvent* pEvent = (CEvent*)m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() );
	ASSERT( pEvent );
	if( !pEvent )	return;

	f32 val;
	vector3  rot;
	CString str;

	m_AttachedRotZ.GetWindowText( str );
	val = (f32)atof( str );
	rot = pEvent->GetAttRot();
	rot.Z = val;
	pEvent->SetAttRot( rot );
}

void CDefinedEventsDlg::OnChangeDefinedEventsDlgDistance()
{
	//---	get the selected event
	if ( (m_EventListCtrl.GetCurSel() == -1) || !m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() ) )
		return;
	CEvent* pEvent = (CEvent*)m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() );
	ASSERT( pEvent );
	if( !pEvent )	return;

	f32 val;
	CString str;

	m_AttachedDistance.GetWindowText( str );
	val = (f32)atof( str );
	pEvent->SetAttDist( val );
}

void CDefinedEventsDlg::OnChangeDefinedEventsDlgForceposx()
{
	//---	get the selected event
	if ( (m_EventListCtrl.GetCurSel() == -1) || !m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() ) )
		return;
	CEvent* pEvent = (CEvent*)m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() );
	ASSERT( pEvent );
	if( !pEvent )	return;

	f32 val;
	vector3  trans;
	CString str;

	m_AttachedForcePosX.GetWindowText( str );
	val = (f32)atof( str );
	trans = pEvent->GetAttForcePos();
	trans.X = val;
	pEvent->SetAttForcePos( trans );
}

void CDefinedEventsDlg::OnChangeDefinedEventsDlgForceposy()
{
	//---	get the selected event
	if ( (m_EventListCtrl.GetCurSel() == -1) || !m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() ) )
		return;
	CEvent* pEvent = (CEvent*)m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() );
	ASSERT( pEvent );
	if( !pEvent )	return;

	f32 val;
	vector3  trans;
	CString str;

	m_AttachedForcePosY.GetWindowText( str );
	val = (f32)atof( str );
	trans = pEvent->GetAttForcePos();
	trans.Y = val;
	pEvent->SetAttForcePos( trans );
}

void CDefinedEventsDlg::OnChangeDefinedEventsDlgForceposz()
{
	//---	get the selected event
	if ( (m_EventListCtrl.GetCurSel() == -1) || !m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() ) )
		return;
	CEvent* pEvent = (CEvent*)m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() );
	ASSERT( pEvent );
	if( !pEvent )	return;

	f32 val;
	vector3  trans;
	CString str;

	m_AttachedForcePosZ.GetWindowText( str );
	val = (f32)atof( str );
	trans = pEvent->GetAttForcePos();
	trans.Z = val;
	pEvent->SetAttForcePos( trans );
}

void CDefinedEventsDlg::OnDefinedEventsDlgButtonForceposx()
{
	//---	get the selected event
	if ( (m_EventListCtrl.GetCurSel() == -1) || !m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() ) )
		return;
	CEvent* pEvent = (CEvent*)m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() );
	ASSERT( pEvent );
	if( !pEvent )	return;

	pEvent->SetAttForceX( m_AttachedForcePosXBut.GetCheck() ? TRUE : FALSE );
	m_AttachedForcePosX.EnableWindow( m_AttachedForcePosXBut.GetCheck() );

	if( pEvent->GetAttForceX() || pEvent->GetAttForceY() || pEvent->GetAttForceZ() )
		m_AttachedForcedRelativeToJoint.EnableWindow( TRUE );
	else
		m_AttachedForcedRelativeToJoint.EnableWindow( FALSE );
}

void CDefinedEventsDlg::OnDefinedEventsDlgButtonForceposy()
{
	//---	get the selected event
	if ( (m_EventListCtrl.GetCurSel() == -1) || !m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() ) )
		return;
	CEvent* pEvent = (CEvent*)m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() );
	ASSERT( pEvent );
	if( !pEvent )	return;

	pEvent->SetAttForceY( m_AttachedForcePosYBut.GetCheck() ? TRUE : FALSE );
	m_AttachedForcePosY.EnableWindow( m_AttachedForcePosYBut.GetCheck() );

	if( pEvent->GetAttForceX() || pEvent->GetAttForceY() || pEvent->GetAttForceZ() )
		m_AttachedForcedRelativeToJoint.EnableWindow( TRUE );
	else
		m_AttachedForcedRelativeToJoint.EnableWindow( FALSE );
}

void CDefinedEventsDlg::OnDefinedEventsDlgButtonForceposz()
{
	//---	get the selected event
	if ( (m_EventListCtrl.GetCurSel() == -1) || !m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() ) )
		return;
	CEvent* pEvent = (CEvent*)m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() );
	ASSERT( pEvent );
	if( !pEvent )	return;

	pEvent->SetAttForceZ( m_AttachedForcePosZBut.GetCheck() ? TRUE : FALSE );
	m_AttachedForcePosZ.EnableWindow( m_AttachedForcePosZBut.GetCheck() );

	if( pEvent->GetAttForceX() || pEvent->GetAttForceY() || pEvent->GetAttForceZ() )
		m_AttachedForcedRelativeToJoint.EnableWindow( TRUE );
	else
		m_AttachedForcedRelativeToJoint.EnableWindow( FALSE );
}

void CDefinedEventsDlg::OnDefinedEventsDlgButtonRelativetojoint()
{
	//---	get the selected event
	if ( (m_EventListCtrl.GetCurSel() == -1) || !m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() ) )
		return;
	CEvent* pEvent = (CEvent*)m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() );
	ASSERT( pEvent );
	if( !pEvent )	return;

	pEvent->SetAttForceRelToJoint( m_AttachedForcedRelativeToJoint.GetCheck() ? TRUE : FALSE );
}

void CDefinedEventsDlg::OnEditeventActivate()
{
	m_EditEventActive = !m_EditEventActive;
	((CButton*)GetDlgItem( IDC_EDITEVENT_ACTIVATE ))->SetCheck( m_EditEventActive );
	UpdateUI();
}

void CDefinedEventsDlg::OnButtonAttached()
{
	//---	get the selected event
	if ( (m_EventListCtrl.GetCurSel() == -1) || !m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() ) )
		return;
	CEvent* pEvent = (CEvent*)m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() );
	ASSERT( pEvent );
	if( !pEvent )	return;

	pEvent->SetAttached( m_Attached.GetCheck() ? TRUE : FALSE );
	UpdateBoneUI();
}

void CDefinedEventsDlg::OnButtonLinked()
{
	//---	get the selected event
	if ( (m_EventListCtrl.GetCurSel() == -1) || !m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() ) )
		return;

	CEvent* pEvent = (CEvent*)m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() );
	ASSERT( pEvent );
	if( !pEvent )	return;

	pEvent->SetLink( m_Linked.GetCheck() ? 1 : 0 );
	UpdateBoneUI();
}

void CDefinedEventsDlg::OnChangeDefinedEventsDlgName()
{
	//---	get the selected event
	if ( (m_EventListCtrl.GetCurSel() == -1) || !m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() ) )
		return;
	CEvent* pEvent = (CEvent*)m_EventListCtrl.GetItemData( m_EventListCtrl.GetCurSel() );
	ASSERT( pEvent );
	if( !pEvent )	return;

	CString Name;
	m_EditEventName.GetWindowText( Name );
	pEvent->SetName( Name );

	//---	find the given event in the list and change its text
	s32 NumEvents = m_EventListCtrl.GetCount();
	s32 i;
	for( i=0; i<NumEvents; i++ )
	{
		if( m_EventListCtrl.GetItemData( i ) == (u32)pEvent )
			break;
	}

	ASSERT(i < NumEvents);
	m_EventListCtrl.DeleteString( i );
	i = m_EventListCtrl.AddString( Name );
	m_EventListCtrl.SetItemData( i, (u32)pEvent );
	m_EventListCtrl.SetCurSel( i );
}
