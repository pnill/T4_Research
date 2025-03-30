// DlgShotList.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "DlgShotList.h"
#include "Movie.h"
#include "Shot.h"
#include "Actor.h"
#include "MovieView.h"

#include "MFCutils.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgShotList dialog


CDlgShotList::CDlgShotList(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgShotList::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgShotList)
	//}}AFX_DATA_INIT
	m_pView = NULL ;
	m_Updating = false ;
	m_BuildingShotList = false ;
}


void CDlgShotList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgShotList)
	DDX_Control(pDX, IDC_INTERP, m_Interpolate);
	DDX_Control(pDX, IDC_SPEED, m_Speed);
	DDX_Control(pDX, IDC_FRAME, m_Frame);
	DDX_Control(pDX, IDC_TARGET, m_Target);
	DDX_Control(pDX, IDC_EYE, m_Eye);
	DDX_Control(pDX, IDC_SHOTLIST, m_ShotList);
	DDX_Control(pDX, IDC_FOV, m_FOV);
	//}}AFX_DATA_MAP
}





void CDlgShotList::SetupShotList( CMovie *pMovie )
{
	m_pMovie = pMovie ;

	if( ::IsWindow( GetSafeHwnd() ) )
	{
		m_BuildingShotList = true ;

		//---	Setup ShotList
		m_ShotList.DeleteAllItems( ) ;
		if( pMovie )
		{
			POSITION Pos = pMovie->m_ShotList.GetHeadPosition( ) ;
			while( Pos )
			{
				CShot *pShot = pMovie->m_ShotList.GetNext( Pos ) ;

				CString Name ;
				Name.Format( "%04d- %s", pShot->GetFrame(), pShot->GetName() ) ;
				int iItem = m_ShotList.GetItemCount( ) ;
				int Index = m_ShotList.InsertItem( iItem, Name ) ;
				if( Index != -1 )
				{
					VERIFY( m_ShotList.SetItemData( Index, (DWORD)pShot ) ) ;
				}
			}
		}

		m_BuildingShotList = false ;
	}
}

void CDlgShotList::SetupActorLists( CMovie *pMovie )
{
	m_pMovie = pMovie ;

	if( ::IsWindow( GetSafeHwnd() ) )
	{
		//---	Setup the 2 Actor Lists
		m_Eye.ResetContent( ) ;
		m_Target.ResetContent( ) ;
		int Index = m_Eye.AddString( "<no actor>" ) ;
		if( Index != -1 )
			m_Eye.SetItemData( Index, NULL ) ;
		Index = m_Target.AddString( "<no actor>" ) ;
		if( Index != -1 )
			m_Target.SetItemData( Index, NULL ) ;
		m_Eye.SetCurSel( 0 ) ;
		m_Target.SetCurSel( 0 ) ;

		POSITION Pos = pMovie->m_ActorList.GetHeadPosition( ) ;
		while( Pos )
		{
			CActor *pActor = pMovie->m_ActorList.GetNext( Pos ) ;

			int Index = m_Eye.AddString( pActor->GetName() ) ;
			if( Index != -1 )
				m_Eye.SetItemData( Index, (DWORD)pActor ) ;
			Index = m_Target.AddString( pActor->GetName() ) ;
			if( Index != -1 )
				m_Target.SetItemData( Index, (DWORD)pActor ) ;
		}
	}
}

void CDlgShotList::SelectShot( CShot *pShot, bool Details )
{
	if( ::IsWindow( GetSafeHwnd() ) )
	{
		LV_FINDINFO	fi ;

		fi.flags = LVFI_PARAM ;
		fi.lParam = (DWORD)pShot ;

		int Index = m_ShotList.FindItem( &fi, -1 ) ;
		if( Index != -1 )
		{
			m_ShotList.SetItemState( Index, LVIS_SELECTED, LVIS_SELECTED ) ;
			m_ShotList.EnsureVisible( Index, false ) ;
			m_ShotList.RedrawWindow( ) ;
		}

		if( Details )
			UpdateDetails( ) ;
	}
}

void CDlgShotList::UpdateDetails( )
{
	CShot *pShot = (CShot*)MFCU_ListCtrlGetSelected( m_ShotList ) ;

	if( ::IsWindow( GetSafeHwnd() ) )
	{
		bool	Enable = (pShot != NULL) ;

		m_Eye.EnableWindow( Enable ) ;
		m_Target.EnableWindow( Enable ) ;
		m_Frame.EnableWindow( Enable ) ;
		m_FOV.EnableWindow( Enable ) ;
		m_Speed.EnableWindow( Enable ) ;
		m_Interpolate.EnableWindow( Enable ) ;

		bool	OldUpdating = m_Updating ;
		m_Updating = true ;

		if( pShot )
		{
			MFCU_UpdateEditControl( m_Frame, pShot->GetFrame() ) ;
			MFCU_UpdateEditControl( m_FOV, pShot->GetFOV() ) ;
			MFCU_UpdateEditControl( m_Speed, pShot->GetSpeed() ) ;
			m_Eye.SetCurSel( MFCU_FindComboBoxItemByData( &m_Eye, (DWORD)pShot->GetEye() ) ) ;
			m_Target.SetCurSel( MFCU_FindComboBoxItemByData( &m_Target, (DWORD)pShot->GetTarget() ) ) ;
			m_Interpolate.SetCheck( pShot->GetInterpolate() ? TRUE : FALSE );
		}
		else
		{
			MFCU_UpdateEditControl( m_Frame, 0 ) ;
			MFCU_UpdateEditControl( m_FOV, 60 );
			MFCU_UpdateEditControl( m_Speed, 100 );
			m_Eye.SetCurSel( MFCU_FindComboBoxItemByData( &m_Eye, NULL ) ) ;
			m_Target.SetCurSel( MFCU_FindComboBoxItemByData( &m_Target, NULL ) ) ;
			m_Interpolate.SetCheck( FALSE );
		}

		m_Eye.RedrawWindow( ) ;
		m_Target.RedrawWindow( ) ;
		m_Frame.RedrawWindow( ) ;
		m_FOV.RedrawWindow( ) ;
		m_Speed.RedrawWindow( ) ;
		m_Interpolate.RedrawWindow( ) ;

		m_Updating = OldUpdating ;
	}
}





BEGIN_MESSAGE_MAP(CDlgShotList, CDialog)
	//{{AFX_MSG_MAP(CDlgShotList)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SHOTLIST, OnItemchangedShotlist)
	ON_CBN_SELCHANGE(IDC_EYE, OnSelchangeEye)
	ON_CBN_SELCHANGE(IDC_TARGET, OnSelchangeTarget)
	ON_EN_CHANGE(IDC_FRAME, OnChangeFrame)
	ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_SHOTLIST, OnBeginlabeleditShotlist)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_SHOTLIST, OnEndlabeleditShotlist)
	ON_EN_CHANGE(IDC_FOV, OnChangeFov)
	ON_EN_CHANGE(IDC_SPEED, OnChangeSpeed)
	ON_BN_CLICKED(IDC_INTERP, OnInterp)
	ON_NOTIFY(NM_CLICK, IDC_SHOTLIST, OnClickShotlist)
	//}}AFX_MSG_MAP

	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDCANCEL, OnCancel)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgShotList message handlers

void CDlgShotList::OnOk( )
{
}

void CDlgShotList::OnCancel( )
{
}

void CDlgShotList::OnAdd() 
{
	// TODO: Add your control notification handler code here
	if( m_pView )
		m_pView->ShotAdd( ) ;
}

void CDlgShotList::OnDelete() 
{
	// TODO: Add your control notification handler code here
	CShot *pShot = (CShot*)MFCU_ListCtrlGetSelected( m_ShotList ) ;
	if( pShot )
	{
		if( m_pView )
			m_pView->ShotDelete( pShot ) ;
	}
}

void CDlgShotList::OnItemchangedShotlist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here

	if( !m_BuildingShotList )
		UpdateDetails( ) ;

	*pResult = 0;
}

void CDlgShotList::OnSelchangeEye() 
{
	// TODO: Add your control notification handler code here

	CShot *pShot = (CShot*)MFCU_ListCtrlGetSelected( m_ShotList ) ;
	if( pShot )
	{
		CActor *pActor = (CActor*)m_Eye.GetItemData( m_Eye.GetCurSel() ) ;
		pShot->SetEye( pActor ) ;
		m_pView->Invalidate( ) ;
		m_pView->GetDocument()->SetModifiedFlag( true ) ;
	}
}

void CDlgShotList::OnSelchangeTarget() 
{
	// TODO: Add your control notification handler code here
	
	CShot *pShot = (CShot*)MFCU_ListCtrlGetSelected( m_ShotList ) ;
	if( pShot )
	{
		CActor *pActor = (CActor*)m_Target.GetItemData( m_Target.GetCurSel() ) ;
		pShot->SetTarget( pActor ) ;
		m_pView->Invalidate( ) ;
		m_pView->GetDocument()->SetModifiedFlag( true ) ;
	}
}

void CDlgShotList::OnChangeFrame() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	if( !m_Updating )
	{
		CShot *pShot = (CShot*)MFCU_ListCtrlGetSelected( m_ShotList ) ;
		if( pShot )
		{
			CString s ;
			m_Frame.GetWindowText( s ) ;
			s32 NewFrame = atoi(s);
			m_pView->ShotSetFrame( pShot, NewFrame ) ;
			m_pView->SetFrame( NewFrame );
			m_pView->Invalidate();
		}
	}
}


BOOL CDlgShotList::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	if( ::IsWindow( GetSafeHwnd() ) )
	{
		CRect r ;
		m_ShotList.GetClientRect( &r ) ;
		m_ShotList.InsertColumn( 0, "Shot Name", LVCFMT_LEFT, r.Width()-16, -1 ) ;
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgShotList::OnBeginlabeleditShotlist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	// TODO: Add your control notification handler code here

	CShot *pShot = (CShot*)m_ShotList.GetItemData( pDispInfo->item.iItem ) ;
	if( pShot )
	{
		CEdit *pEdit = m_ShotList.GetEditControl( ) ;
		if( pEdit )
			pEdit->SetWindowText( pShot->GetName() ) ;
	}
	
	*pResult = 0;
}

void CDlgShotList::OnEndlabeleditShotlist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	// TODO: Add your control notification handler code here

	*pResult = 0;

	CShot *pShot = (CShot*)m_ShotList.GetItemData( pDispInfo->item.iItem ) ;
	if( pShot )
	{
		if( pDispInfo->item.pszText )
		{
			pShot->SetName( pDispInfo->item.pszText ) ;
			CString String ;
			String.Format( "%04d - %s", pShot->GetFrame(), pShot->GetName() ) ;
			m_ShotList.SetItemText( pDispInfo->item.iItem, 0, String ) ;

			m_pView->GetDocument()->SetModifiedFlag( ) ;
			*pResult = 0 ;
		}
	}
}

void CDlgShotList::OnChangeFov() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	if( !m_Updating )
	{
		CShot *pShot = (CShot*)MFCU_ListCtrlGetSelected( m_ShotList ) ;
		if( pShot )
		{
			CString s ;
			m_FOV.GetWindowText( s ) ;
			pShot->SetFOV( atoi(s) ) ;
			m_pView->Invalidate();
		}
	}
}

void CDlgShotList::OnChangeSpeed() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.

	// TODO: Add your control notification handler code here
	if( !m_Updating )
	{
		CShot *pShot = (CShot*)MFCU_ListCtrlGetSelected( m_ShotList ) ;
		if( pShot )
		{
			CString s ;
			m_Speed.GetWindowText( s ) ;
			pShot->SetSpeed( atoi(s) ) ;
			m_pView->Invalidate();
		}
	}
}

void CDlgShotList::OnInterp() 
{
	if( !m_Updating )
	{
		CShot *pShot = (CShot*)MFCU_ListCtrlGetSelected( m_ShotList ) ;
		if( pShot )
		{
			pShot->SetInterpolate( m_Interpolate.GetCheck() ? TRUE : FALSE );
			m_pView->Invalidate();
		}
	}
}

void CDlgShotList::OnClickShotlist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if( !m_Updating )
	{
		CShot *pShot = (CShot*)MFCU_ListCtrlGetSelected( m_ShotList ) ;
		if( pShot )
		{
			m_pView->SetFrame( pShot->GetFrame() );
			m_pView->Invalidate();
		}
	}
	
	*pResult = 0;
}
