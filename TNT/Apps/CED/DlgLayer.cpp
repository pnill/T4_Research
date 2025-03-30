// DlgLayer.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "DlgLayer.h"
#include "MovieView.h"
#include "Actor.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgLayer dialog


CDlgLayer::CDlgLayer(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgLayer::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgLayer)
	//}}AFX_DATA_INIT
	m_pView = NULL;
}


void CDlgLayer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgLayer)
	DDX_Control(pDX, IDC_VIEWALLLAYERS, m_ViewAllLayers);
	DDX_Control(pDX, IDC_LAYER, m_Layer);
	DDX_Control(pDX, IDC_DELETELAYER, m_DeleteLayer);
	DDX_Control(pDX, IDC_ADDLAYER, m_AddLayer);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgLayer, CDialog)
	//{{AFX_MSG_MAP(CDlgLayer)
	ON_CBN_EDITCHANGE(IDC_LAYER, OnEditchangeLayer)
	ON_BN_CLICKED(IDC_DELETELAYER, OnDeletelayer)
	ON_BN_CLICKED(IDC_ADDLAYER, OnAddlayer)
	ON_CBN_SELCHANGE(IDC_LAYER, OnSelchangeLayer)
	ON_BN_CLICKED(IDC_VIEWALLLAYERS, OnViewalllayers)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgLayer message handlers

BOOL CDlgLayer::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_SelectedLayer = 0;

	if( m_pView )
	{
		UpdateDetails();
	}

	m_ViewAllLayers.SetCheck( FALSE );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgLayer::UpdateDetails( )
{
	m_Layer.ResetContent();

	m_Layer.AddString( "<all layers>" );

	if( m_pView && m_pView->m_pMovie )
	{
		int NumLayers = m_pView->m_pMovie->GetNumLayers();
		int i;

		for( i=0; i<NumLayers; i++ )
			m_Layer.AddString( m_pView->m_pMovie->GetLayer( i ) );
	}

	if( m_SelectedLayer >= m_Layer.GetCount() )
		m_SelectedLayer = m_Layer.GetCount()-1;
	else if( m_SelectedLayer < 0 )
		m_SelectedLayer = 0;

	if( m_SelectedLayer <= 0 )
		m_DeleteLayer.EnableWindow( FALSE );
	else
		m_DeleteLayer.EnableWindow( TRUE );

	if( m_pView->m_pMovie->GetNumLayers() == MAX_MOVIE_LAYERS )
		m_AddLayer.EnableWindow( FALSE );
	else
		m_AddLayer.EnableWindow( TRUE );

	m_Layer.SetCurSel( m_SelectedLayer );
}


void CDlgLayer::EnsureViewable( int Layer )
{
	Layer++; // make room for the <all layers> option at the top of the list

	if( m_SelectedLayer <= 0 )
		return;

	if( m_SelectedLayer == Layer )
		return;

	m_SelectedLayer = Layer;
	m_Layer.SetCurSel( Layer );
}


void CDlgLayer::OnEditchangeLayer() 
{
	if( m_SelectedLayer <= 0 )
		return;

	ASSERT( m_SelectedLayer < (m_pView->m_pMovie->GetNumLayers()+1) );

	CString NewName;
	m_Layer.GetLBText( m_SelectedLayer, NewName );
	m_pView->m_pMovie->SetLayer( m_SelectedLayer, NewName );
}

void CDlgLayer::OnDeletelayer() 
{
	if( m_SelectedLayer <= 0 )
		return;

	ASSERT( m_SelectedLayer < (m_pView->m_pMovie->GetNumLayers()+1) );
	int DeletedLayer = GetEditLayer();

	if( !m_pView )
		return;

	CString msg;
	msg.Format( "Are you sure that you want to delete layer '%s'?", m_pView->m_pMovie->GetLayer( DeletedLayer ) );
	int answer = MessageBox( msg, "Delete Layer", MB_ICONWARNING | MB_YESNO );
	if( answer != IDYES )
		return;

	//---	look for an actor to have been part of this layer
	bool OneExists=FALSE;
	POSITION pos = m_pView->m_pMovie->m_ActorList.GetHeadPosition();
	while( pos )
		if( m_pView->m_pMovie->m_ActorList.GetNext( pos )->GetLayer() == DeletedLayer )
		{
			OneExists = TRUE;
			break;
		}

	//---	if one exists...
	if( OneExists )
	{
		//---	ask the user if he wants to delete all of the layered actors.
		int answer = MessageBox( "Some actor's exist on the deleted layer.  Should they be deleted?", "Deleted Layer actors", MB_ICONWARNING|MB_YESNO );
		CActor* pActor;
		POSITION pos = m_pView->m_pMovie->m_ActorList.GetHeadPosition();
		int NumActors = m_pView->m_pMovie->m_ActorList.GetCount();
		int i;
		for( i=0; i<NumActors; i++ )
		{
			pos = m_pView->m_pMovie->m_ActorList.FindIndex( i );
			pActor = (CActor*)m_pView->m_pMovie->m_ActorList.GetAt( pos );
			if( pActor->GetLayer() == DeletedLayer )
			{
				if( answer == IDYES )
				{
					m_pView->m_pMovie->m_ActorList.RemoveAt( pos );
					delete pActor;
					i--;
					NumActors--;
				}
				else
					pActor->SetLayer( -1 );
			}
			else if( pActor->GetLayer() > DeletedLayer )
			{
				pActor->SetLayer( pActor->GetLayer() - 1 );
			}
		}
	}

	//---	remove the layer and redraw the window
	m_pView->m_pMovie->RemoveLayer( DeletedLayer );

	UpdateDetails();

	m_pView->MovieLayerDeleted( DeletedLayer );
	m_pView->MovieLayerChanged();
	m_pView->MovieLayerSelected();
}

void CDlgLayer::OnAddlayer() 
{
	CString NewLayer;
	m_SelectedLayer = m_pView->m_pMovie->GetNumLayers()+1;
	NewLayer.Format( "New Layer %d", m_SelectedLayer );
	m_pView->m_pMovie->AddLayer( NewLayer );
	UpdateDetails();
	m_Layer.SetCurSel( m_SelectedLayer );

	if( m_pView )
	{
		m_pView->MovieLayerChanged();
		m_pView->MovieLayerSelected();
	}
}

void CDlgLayer::OnSelchangeLayer() 
{
	m_SelectedLayer = m_Layer.GetCurSel();

	if( m_SelectedLayer <= 0 )
		m_DeleteLayer.EnableWindow( FALSE );
	else
		m_DeleteLayer.EnableWindow( TRUE );

	if( m_pView )
		m_pView->MovieLayerSelected();
}

void CDlgLayer::OnViewalllayers() 
{
	if( m_pView )
		m_pView->RedrawWindow();
}

int CDlgLayer::GetViewLayer()
{
	if( !GetSafeHwnd() )
		return -1;

	if( m_SelectedLayer < 0 )
		return -1;

	if( m_ViewAllLayers.GetCheck() )
		return -1;

	return m_SelectedLayer-1;
}

int CDlgLayer::GetEditLayer()
{
	if( !GetSafeHwnd() )
		return -1;

	if( m_SelectedLayer < 0 )
		return -1;

	return m_SelectedLayer-1;
}
