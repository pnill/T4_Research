// AttachJointDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "AttachJointDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


s32		CAttachJointDlg::s_LastBone = 0;
s32		CAttachJointDlg::s_LastLinkedChar = 0;
xbool	CAttachJointDlg::s_LastMoveToJoint = TRUE;

/////////////////////////////////////////////////////////////////////////////
// CAttachJointDlg dialog


CAttachJointDlg::CAttachJointDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAttachJointDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAttachJointDlg)
	m_StringSelectedEvent = _T("");
	//}}AFX_DATA_INIT
	m_pView = NULL;
	m_pSelectedEvent = NULL;
	m_Immediate = FALSE;
}


void CAttachJointDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAttachJointDlg)
	DDX_Control(pDX, IDC_LINKEDCHARACTER, m_LinkedCharacter);
	DDX_Text(pDX, IDC_STATIC_EVENT_NAME, m_StringSelectedEvent);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAttachJointDlg, CDialog)
	//{{AFX_MSG_MAP(CAttachJointDlg)
	ON_BN_CLICKED(IDC_LINKEDCHARACTER, OnButtonAttachToLinkedChar)
	ON_BN_CLICKED(IDC_BUTTON_MOVE_TO_JOINT, OnButtonMoveToJoint)
	ON_BN_CLICKED(IDC_ATTACH, OnButtonAttached)
	ON_NOTIFY_EX( TTN_NEEDTEXT, 0, OnToolTipNotify )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAttachJointDlg message handlers

BOOL CAttachJointDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	ASSERT( m_pView );
	ASSERT( m_pSelectedEvent );

	//---	if this is an event which is already attached, don't allow it to be moved
	if( m_pSelectedEvent->GetAttached() && ((m_pSelectedEvent->GetBone() != -1) || (m_pSelectedEvent->GetBoneName() != "<no bone>")) )
	{
		CDialog::OnOK();
		return FALSE;
	}

	if( m_Immediate )
	{
		EventToJoint();
		return FALSE;
	}

	CButton* pButton = (CButton*) GetDlgItem( IDC_ATTACH );
	pButton->SetCheck( s_LastMoveToJoint ? FALSE : TRUE );

	m_LinkedCharacter.SetCheck( s_LastLinkedChar );

	InitSelectedEvent();
	InitBoneCombo();

	EnableToolTips( TRUE );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAttachJointDlg::SetView(CMotionEditView *pView)
{
	m_pView = pView;
}

void CAttachJointDlg::SetEvent(CEvent *pEvent)
{
	m_pSelectedEvent = pEvent;
}

void CAttachJointDlg::InitSelectedEvent()
{
	ASSERT( m_pView );

	m_pSelectedEvent = m_pView->EventGetCurrent();

	if( m_pSelectedEvent )
	{
		m_StringSelectedEvent = m_pSelectedEvent->GetName();
	}
	else
	{
		m_StringSelectedEvent = "<no event>";
	}
	UpdateData( FALSE );
}

void CAttachJointDlg::InitBoneCombo()
{
	CComboBox* pBoneListCombo;
	CSkeleton* pSkeleton;
	s32			NewIndex;
	
	pBoneListCombo = (CComboBox *)GetDlgItem( IDC_COMBO_ATTACHED_BONE );
	ASSERT( pBoneListCombo );

	s32 Index = pBoneListCombo->GetCurSel();
	CString IndexName("");
	if( Index != -1 )
		pBoneListCombo->GetLBText( Index, IndexName );

	pBoneListCombo->ResetContent();

	//---	get the skeleton to be put in the combo box
	if( m_LinkedCharacter.GetCheck() )
	{
		if( !m_pView->m_pCharacter->GetPropDef(0).GetLinkedCharacter() )
			return;

		pSkeleton = m_pView->m_pCharacter->GetPropDef(0).GetLinkedCharacter()->m_pSkeleton;
	}
	else
		pSkeleton = m_pView->GetSkeleton();

	//---	set the combo box
	NewIndex = -1;
	if( pSkeleton )
	{
		INT nBones;
		INT iBone;

		nBones = pSkeleton->GetNumBones();
		for( iBone = 0; iBone < nBones; iBone++ )
		{
			INT nBoneParents;
			INT iBoneParent;
			CString BoneName;

			nBoneParents = pSkeleton->GetBoneNumParents( pSkeleton->GetBoneFromIndex( iBone ) );
			for( iBoneParent = 0; iBoneParent < nBoneParents; iBoneParent++ )
			{
				BoneName += " ";
			}
			BoneName += pSkeleton->GetBoneFromIndex( iBone )->BoneName;
			int Pos = pBoneListCombo->AddString( BoneName );

			//---	continue the search for the new bone index
			if( (NewIndex==-1) && (IndexName == BoneName) )
				NewIndex = iBone;

			ASSERT( Pos != CB_ERR ) ;
			ASSERT( Pos != CB_ERRSPACE ) ;
		}
	}

	if( !pSkeleton )
		pBoneListCombo->SetCurSel( -1 );
	else if( NewIndex != -1 )
		pBoneListCombo->SetCurSel( NewIndex );
	else if(( s_LastBone < pSkeleton->GetNumBones() ) && ((s_LastLinkedChar ? TRUE : FALSE) == (m_LinkedCharacter.GetCheck() ? TRUE : FALSE)))
		pBoneListCombo->SetCurSel( s_LastBone );
	else
		pBoneListCombo->SetCurSel( 0 );
}

void CAttachJointDlg::OnButtonAttachToLinkedChar()
{
	InitBoneCombo();
}

void CAttachJointDlg::EventToJoint ( void )
{
	CComboBox* pBoneListCombo;
	INT iBone;
	s32 LinkedChar;
	CString Message;
	
	ASSERT( m_pView );

	pBoneListCombo = (CComboBox *)GetDlgItem( IDC_COMBO_ATTACHED_BONE );
	ASSERT( pBoneListCombo );

	if( m_Immediate )
	{
		iBone = s_LastBone;
		LinkedChar = s_LastLinkedChar;
	}
	else
	{
		iBone = ( pBoneListCombo->GetCurSel() );
		LinkedChar = m_LinkedCharacter.GetCheck();
	}

	if( s_LastMoveToJoint )
	{
		//---	if the event is supposed to be attached to the linked character,
		//		move the event from the main character over.
		if( LinkedChar )
		{
			m_pSelectedEvent->SetAttached( TRUE );
			m_pSelectedEvent->SetLink( 1 );
			m_pSelectedEvent->SetBone( iBone );
			m_pSelectedEvent->UpdateEventPosition();
			m_pSelectedEvent->SetAttached( FALSE );
			m_pSelectedEvent->SetLink( 0 );
		}
		else
		{
			//---	new functionality, move the event to the selected bone
			m_pSelectedEvent->SetAttached( TRUE );
			m_pSelectedEvent->SetLink( 0 );
			m_pSelectedEvent->SetBone( iBone );
			m_pSelectedEvent->UpdateEventPosition();
			m_pSelectedEvent->SetBone( -1 );
			m_pSelectedEvent->SetAttached( FALSE );
		}
	}
	else
	{
		if( LinkedChar )
		{
			m_pSelectedEvent->SetAttached( TRUE );
			m_pSelectedEvent->SetLink( 1 );
			m_pSelectedEvent->SetBone( iBone );
			m_pSelectedEvent->UpdateEventPosition();
		}
		else
		{
			//---	new functionality, move the event to the selected bone
			m_pSelectedEvent->SetAttached( TRUE );
			m_pSelectedEvent->SetLink( 0 );
			m_pSelectedEvent->SetBone( iBone );
			m_pSelectedEvent->UpdateEventPosition();
		}
	}

	if( !m_Immediate )
	{
		//---	store the static dialog information
		s_LastBone = iBone;
		s_LastLinkedChar = m_LinkedCharacter.GetCheck();
	}

	//---	update the document/view
	m_pView->GetDocument()->SetModifiedFlag();
	m_pView->RedrawWindow();
	m_pView->UpdateEventList();
	m_pView->UpdateEventControls();

	CDialog::OnOK();
/*
	{
		CSkeleton* pSkeleton;

		pSkeleton = m_pView->GetSkeleton();
		ASSERT( pSkeleton );
		ASSERT( m_pSelectedEvent );

		Message.Format( "WARNING!  Performing this action will attach all instances of the \nevent %s to the joint %s for all motions of this character.\n\nDo you wish to proceed?", 
			m_pSelectedEvent->GetName(), pSkeleton->GetBoneFromIndex( iBone )->BoneName );
	}

	if( MessageBox( Message, "Verify Joint Attachment", MB_YESNO | MB_ICONEXCLAMATION ) == IDYES )
	{
		AddEventtoAttachedEventList( iBone );
		m_pView->AttachBoneCurrentEvent( iBone );
		m_pView->GetDocument()->SetModifiedFlag();
	}
*/
}

void CAttachJointDlg::OnButtonMoveToJoint() 
{
	EventToJoint();
}

void CAttachJointDlg::OnButtonAttached()
{
	CButton* pButton = (CButton*) GetDlgItem( IDC_ATTACH );
	s_LastMoveToJoint = pButton->GetCheck() ? FALSE : TRUE;
}

void CAttachJointDlg::AddEventtoAttachedEventList( INT iBone )
{
/*
	CEventList* pAttachEvents;
	CEvent* pEvent;
	POSITION pos;

	ASSERT( m_pSelectedEvent );
	ASSERT( m_pView );

	pAttachEvents = m_pView->GetAttachedEvents();
	ASSERT( pAttachEvents );

	pos = pAttachEvents->GetPositionbyName( m_pSelectedEvent->GetName() );
	if( pos )
	{
		pEvent = (CEvent *)pAttachEvents->GetAt( pos );
		ASSERT( pEvent );
		pEvent->SetBone( iBone );
	}
	else
	{
		pEvent = new CEvent;
		ASSERT( pEvent );
		pEvent->SetBone( iBone );
		pEvent->SetName( m_pSelectedEvent->GetName() );
		pEvent->SetMotion( m_pSelectedEvent->GetMotion() );
		pEvent->SetRadius( m_pSelectedEvent->GetRadius() );
		pAttachEvents->AddTail( pEvent );
	}

	{
		CDocument* pDocument;

		pDocument = m_pView->GetDocument();
		ASSERT( pDocument );
		pDocument->UpdateAllViews( NULL );
	}
*/
}

BOOL CAttachJointDlg::OnToolTipNotify( UINT id, NMHDR * pTTTStruct, LRESULT * pResult )
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
