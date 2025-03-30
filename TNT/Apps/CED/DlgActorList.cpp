// DlgActorList.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "DlgActorList.h"
#include "Actor.h"
#include "MovieView.h"
#include "mfcutils.h"
#include "DlgSelectMesh.h"
#include "DlgActorNew.h"
#include "Mesh.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CDlgActorList dialog


CDlgActorList::CDlgActorList(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgActorList::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgActorList)
	//}}AFX_DATA_INIT
	m_BuildList = false ;
}


void CDlgActorList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgActorList)
	DDX_Control(pDX, IDC_ACTORLIST_LAYER, m_Layer);
	DDX_Control(pDX, IDC_EDIT, m_Edit);
	DDX_Control(pDX, IDC_DISPLAYPATH, m_DisplayPath);
	DDX_Control(pDX, IDC_CHARACTER, m_CharacterName);
	DDX_Control(pDX, IDC_LIST, m_List);
	//}}AFX_DATA_MAP
}


void CDlgActorList::Reset( )
{
	if( ::IsWindow( GetSafeHwnd() ) )
	{
		m_List.DeleteAllItems( ) ;
		RebuildLayerDropList() ;
	}
}

void CDlgActorList::AddActor( CActor *pActor )
{
	bool OldBuildList = m_BuildList ;
	m_BuildList = true ;
	if( ::IsWindow( GetSafeHwnd() ) )
	{
		int iItem = m_List.GetItemCount( ) ;
		int Index = m_List.InsertItem( iItem, pActor->GetName() ) ;
		if( Index != -1 )
		{
			VERIFY( m_List.SetItemData( Index, (DWORD)pActor ) ) ;
		}
	}
	m_BuildList = OldBuildList ;
}

void CDlgActorList::SelectActor( CActor *pActor )
{
	bool OldBuildList = m_BuildList ;
	m_BuildList = true ;
	if( ::IsWindow( GetSafeHwnd() ) )
	{
		LV_FINDINFO	fi ;

		fi.flags = LVFI_PARAM ;
		fi.lParam = (DWORD)pActor ;

		int Index = m_List.FindItem( &fi, -1 ) ;
		if( Index != -1 )
		{
			m_List.SetItemState( Index, LVIS_SELECTED, LVIS_SELECTED ) ;
			m_List.EnsureVisible( Index, false ) ;
			m_List.RedrawWindow( ) ;
		}
		UpdateDetails( pActor ) ;
	}
	m_BuildList = OldBuildList ;
}

void CDlgActorList::UpdateDetails( CActor *pActor )
{
	if( ::IsWindow( GetSafeHwnd() ) )
	{
		if( pActor && pActor->m_pCharacter )
			m_CharacterName.SetWindowText( pActor->m_pCharacter->GetName() ) ;
		else
			m_CharacterName.SetWindowText( "<no character>" ) ;

        m_Edit.EnableWindow( pActor && ((pActor->GetType() == ACTOR_SCENERY) || (pActor->GetType() == ACTOR_CHARACTER)) ) ;

		if( pActor)
		{
			m_DisplayPath.SetCheck( pActor->m_DisplayPath ) ;
			m_Layer.SetCurSel( ((pActor->GetLayer()+1) < m_Layer.GetCount()) ? (pActor->GetLayer()+1) : -1 );
			m_Layer.EnableWindow( TRUE );
		}
		else
			m_Layer.EnableWindow( FALSE );

		m_DisplayPath.EnableWindow( pActor != NULL ) ;
		m_DisplayPath.RedrawWindow() ;
	}
}




BEGIN_MESSAGE_MAP(CDlgActorList, CDialog)
	//{{AFX_MSG_MAP(CDlgActorList)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST, OnEndlabeleditList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemchangedList)
	ON_BN_CLICKED(IDC_DISPLAYPATH, OnDisplaypath)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDblclkList)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	ON_CBN_SELCHANGE(IDC_ACTORLIST_LAYER, OnSelchangeActorlistLayer)
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP

	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDCANCEL, OnCancel)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgActorList message handlers

void CDlgActorList::OnOk()
{
}

void CDlgActorList::OnCancel()
{
}

void CDlgActorList::OnAdd() 
{
	// TODO: Add your control notification handler code here
	if( m_pView )
		m_pView->ActorNew( ) ;
}

void CDlgActorList::OnDelete() 
{
	// TODO: Add your control notification handler code here
	ASSERT( m_pView ) ;

	if( ::IsWindow( GetSafeHwnd() ) )
	{
		if( m_pView )
		{
			CActor *pActor = NULL ;
			for( int i = 0 ; i < m_List.GetItemCount() ; i++ )
			{
				if( m_List.GetItemState( i, LVIS_SELECTED ) )
				{
					pActor = (CActor*)m_List.GetItemData( i ) ;
					break ;
				}
			}
			if( pActor )
			{
				CActor *pNewActor = NULL ;
				if( (i+1) < m_List.GetItemCount() )
					pNewActor = (CActor*)m_List.GetItemData( i+1 ) ;
				else if( (i-1) >= 0 )
					pNewActor = (CActor*)m_List.GetItemData( i-1 ) ;

				m_pView->ActorDel( pActor, pNewActor ) ;
			}
		}
	}
}

BOOL CDlgActorList::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	if( ::IsWindow( GetSafeHwnd() ) )
	{
		CRect r ;
		m_List.GetClientRect( &r ) ;
		m_List.InsertColumn( 0, "Actor Name", LVCFMT_LEFT, r.Width()-16, -1 ) ;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgActorList::RebuildLayerDropList( void )
{
	int CurSelect = m_Layer.GetCurSel();

	m_Layer.ResetContent();
	m_Layer.AddString( "<none>" );
	int NumLayers = m_pView->m_pMovie->GetNumLayers();
	for( int i=0; i<NumLayers; i++ )
		m_Layer.AddString( m_pView->m_pMovie->GetLayer(i) );

	if( CurSelect >= m_Layer.GetCount() )
		CurSelect = m_Layer.GetCount()-1;
	else if( CurSelect == -1 )
		CurSelect = 0;

	m_Layer.SetCurSel( CurSelect );
}

void CDlgActorList::OnEndlabeleditList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	// TODO: Add your control notification handler code here

	*pResult = 0 ;

	if( ::IsWindow( GetSafeHwnd() ) )
	{
		if( m_pView )
		{
			CActor *pActor = (CActor*)pDispInfo->item.lParam ;
			ASSERT(pActor) ;
			if( pDispInfo->item.pszText )
			{
				m_List.SetItemText( pDispInfo->item.iItem, 0, pDispInfo->item.pszText ) ;
				pActor->SetName( pDispInfo->item.pszText ) ;
				m_pView->GetDocument()->SetModifiedFlag( ) ;
				*pResult = 1 ;
			}
		}
	}
}

void CDlgActorList::OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here

	if( !m_BuildList )
	{
		if( ::IsWindow( GetSafeHwnd() ) )
		{
			if( m_pView )
			{
				if( (pNMListView->uNewState & LVIS_SELECTED) )
				{
					CActor *pActor = (CActor*)pNMListView->lParam ;
					if( pActor )
					{
						m_pView->ActorSetCurrent( pActor ) ;
						UpdateDetails( pActor ) ;
					}
				}
			}
		}
	}
	
	*pResult = 0;
}

void CDlgActorList::OnDisplaypath() 
{
	// TODO: Add your control notification handler code here

	if( ::IsWindow( GetSafeHwnd() ) )
	{
		if( m_pView )
		{
			CActor *pActor = m_pView->GetCurActor( ) ;
			if( pActor )
			{
				pActor->m_DisplayPath = !!m_DisplayPath.GetCheck( ) ;
				m_pView->Invalidate( ) ;
			}
		}
	}
}

void CDlgActorList::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here

	if( m_pView )
		m_pView->ActorLookAt( ) ;

	*pResult = 0;
}


void CDlgActorList::OnEdit() 
{
	// TODO: Add your control notification handler code here
	CActor *pActor = NULL ;
	for( int i = 0 ; i < m_List.GetItemCount() ; i++ )
	{
		if( m_List.GetItemState( i, LVIS_SELECTED ) )
		{
			pActor = (CActor*)m_List.GetItemData( i ) ;
			break ;
		}
	}

	if( !pActor )
		return;

//	if( pActor->GetType() == ACTOR_SCENERY )
//	{
//    	CDlgSelectMesh	Dlg ;
//        Dlg.m_Name = pActor->m_Name ;
//	    Dlg.m_pMeshList = &m_pView->GetDocument()->m_MeshList ;
//        Dlg.m_pMesh = pActor->m_pMesh ;
//
//        if( Dlg.DoModal() )
//        {
//            pActor->m_pMesh = Dlg.m_pMesh ;
//            m_pView->GetDocument()->UpdateAllViews( NULL ) ;
//        }
//    }

	ASSERT( m_pView && m_pView->m_pMovie ) ;
	CDlgActorNew	Dlg ;
	Dlg.m_Name				= pActor->GetName();
	Dlg.m_pCharacterList	= &m_pView->GetDocument()->m_CharacterList ;
	Dlg.m_pCharacter		= pActor->GetCharacter();
	Dlg.m_Skin				= pActor->GetSkin();
	Dlg.m_Type				= pActor->GetType();
	Dlg.m_pMeshList			= &m_pView->GetDocument()->m_MeshList ;
	Dlg.m_pMesh				= pActor->m_pMesh;
	Dlg.m_LockType			= TRUE;

	if( Dlg.DoModal() == IDOK )
	{
		pActor->SetName( Dlg.m_Name ) ;
		if( Dlg.m_Type == ACTOR_CHARACTER )
		{
			pActor->SetCharacter( Dlg.m_pCharacter ) ;
			pActor->SetSkin( Dlg.m_Skin ) ;
		}
		if( Dlg.m_Type == ACTOR_SCENERY )
		{
			pActor->SetMesh( Dlg.m_pMesh ) ;
			pActor->SetName( Dlg.m_pMesh->GetFileName() ) ;
		}
		pActor->SetType( Dlg.m_Type ) ;
		m_pView->GetDocument()->UpdateAllViews( NULL ) ;
		m_pView->GetDocument()->SetModifiedFlag( ) ;
	}
}

void CDlgActorList::OnSelchangeActorlistLayer() 
{
	int Layer = m_Layer.GetCurSel();
	if( Layer == -1 )
		return;

	CActor *pActor = NULL ;
	for( int i = 0 ; i < m_List.GetItemCount() ; i++ )
	{
		if( m_List.GetItemState( i, LVIS_SELECTED ) )
		{
			pActor = (CActor*)m_List.GetItemData( i ) ;
			if( pActor )
				pActor->SetLayer( Layer-1 );
		}
	}
}

void CDlgActorList::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnLButtonUp(nFlags, point);
}
