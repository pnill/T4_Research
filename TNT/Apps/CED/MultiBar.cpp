// MultiBar.cpp: implementation of the CMultiBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ced.h"
#include "MultiBar.h"
#include "Actor.h"
#include "MovieView.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// cribbed from AFXIMPL.H

extern const TCHAR _afxWndControlBar[]; // controls with grey backgrounds

//#define AfxDeferRegisterClass(fClass) \
//	((afxRegisteredClasses & fClass) ? TRUE : AfxEndDeferRegisterClass(fClass))

//extern BOOL AFXAPI AfxEndDeferRegisterClass(short fClass);

//#define AFX_WNDCOMMCTLS_REG     (0x0010)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMultiBar::CMultiBar()
{
	m_pView = NULL ;
}

CMultiBar::~CMultiBar()
{

}

/////////////////////////////////////////////////////////////////////////////
// Create

bool CMultiBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	ASSERT(pParentWnd != NULL);

	//---	Initialize common controls
//	VERIFY(AfxDeferRegisterClass(AFX_WNDCOMMCTLS_REG));
	m_LastSelection = -1 ;

	//---	Allow chance to modify styles
	dwStyle |= CBRS_BORDER_3D ;
	m_dwStyle = dwStyle ;
	CREATESTRUCT cs ;
	memset( &cs, 0, sizeof( cs ) ) ;
	cs.lpszClass	= _afxWndControlBar;
	cs.style		= (DWORD)dwStyle | WS_CLIPCHILDREN|WS_VISIBLE ;
	cs.dwExStyle	= WS_EX_CLIENTEDGE ;
	cs.hMenu		= (HMENU)nID ;
	cs.hInstance	= AfxGetInstanceHandle( ) ;
	cs.hwndParent	= pParentWnd->GetSafeHwnd( ) ;
	if( !PreCreateWindow( cs ) )
		return FALSE ;

	//---	Create the Bar Window & the Rollup Control
	CRect r( 0, 0, m_Width, 0 ) ;
	CWnd::Create( cs.lpszClass, NULL, cs.style, r, pParentWnd, nID, NULL ) ;

	//---	Load Image List for TAB
	BOOL Success = m_TabImages.Create( IDB_TAB, 22, 256, RGB(255,255,255) ) ;
	ASSERT( Success ) ;

	//---	Create Rollup
	r.DeflateRect( 2, 0 ) ;
	m_RollupCtrl.Create( NULL, NULL, WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_VISIBLE, r, this, 1 ) ;
//	SetupCharacter( ) ;

	//---	Create Tab
	CRect r2(0, 0, m_Width, 0 ) ;
	m_Tab.Create( WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_VISIBLE|TCS_FIXEDWIDTH, r2, this, 0 ) ;
	m_Tab.SetImageList( &m_TabImages ) ;
	m_Tab.SetItemSize( CSize(24,24) ) ;
	m_Tab.SetPadding( CSize(0,0) ) ;
	TC_ITEM Item = {0} ;
	Item.mask =  TCIF_IMAGE ;
	Item.iImage = 0 ;
	m_Tab.InsertItem( 0, &Item ) ;
	Item.iImage = 1 ;
	m_Tab.InsertItem( 1, &Item ) ;
	Item.iImage = 2 ;
	m_Tab.InsertItem( 2, &Item ) ;
	Item.iImage = 3 ;
	m_Tab.InsertItem( 3, &Item ) ;
	Item.iImage = 4 ;
	m_Tab.InsertItem( 4, &Item ) ;
	Item.iImage = 5 ;
	m_Tab.InsertItem( 5, &Item ) ;
	m_Tab.SetCurSel( 0 );
	TabSelchange( NULL, NULL );

	//---	Return Success
	return TRUE ;
}

void CMultiBar::SetupCharacter( )
{
	m_RollupCtrl.DeleteAllPanes( ) ;
	AddPaneDialog( "View/Edit Layer", IDD_EDIT_LAYER, &m_DlgLayer ) ;
	AddPaneDialog( "Actor List", IDD_ACTORLIST, &m_DlgActorList ) ;
	AddPaneDialog( "KeyFrames", IDD_KEY, &m_DlgKey ) ;
	m_RollupCtrl.PaneChanged( ) ;
	if( m_pView  )
	{
        m_pView->SetUseShotList( false ) ;
		m_pView->SetActorType ( ACTOR_CHARACTER ) ;
		m_pView->ActorSetCurrent( NULL ) ;
		m_pView->RedrawWindow( ) ;
	}
}

void CMultiBar::SetupCamera( )
{
	m_RollupCtrl.DeleteAllPanes( ) ;
	AddPaneDialog( "View/Edit Layer", IDD_EDIT_LAYER, &m_DlgLayer ) ;
	AddPaneDialog( "Camera List", IDD_ACTORLIST, &m_DlgActorList ) ;
	AddPaneDialog( "KeyFrames", IDD_KEY, &m_DlgKey ) ;
	m_RollupCtrl.PaneChanged( ) ;
	if( m_pView )
	{
        m_pView->SetUseShotList( false ) ;
		m_pView->SetActorType ( ACTOR_CAMERA ) ;
		m_pView->ActorSetCurrent( NULL ) ;
		m_pView->ModifyViewFlags( 0, MVF_VIEWCAMERAS|MVF_VIEWCAMERATARGETS );
		m_pView->RedrawWindow( ) ;
	}
}

void CMultiBar::SetupLight( )
{
	m_RollupCtrl.DeleteAllPanes( ) ;
//	AddPaneDialog( "View/Edit Layer", IDD_EDIT_LAYER, &m_DlgLayer ) ;
//	AddPaneDialog( "Light List", IDD_ACTORLIST, &m_DlgActorList ) ;
//	AddPaneDialog( "KeyFrames", IDD_KEY, &m_DlgKey ) ;
	m_RollupCtrl.PaneChanged( ) ;
	if( m_pView )
	{
        m_pView->SetUseShotList( false ) ;
		m_pView->SetActorType ( ACTOR_LIGHT ) ;
		m_pView->ActorSetCurrent( NULL ) ;
		m_pView->ModifyViewFlags( 0, MVF_VIEWLIGHTS );
		m_pView->RedrawWindow( ) ;
	}
}

void CMultiBar::SetupScenery( )
{
	m_RollupCtrl.DeleteAllPanes( ) ;
	AddPaneDialog( "View/Edit Layer", IDD_EDIT_LAYER, &m_DlgLayer ) ;
	AddPaneDialog( "Scenery List", IDD_ACTORLIST, &m_DlgActorList ) ;
	AddPaneDialog( "KeyFrames", IDD_KEY, &m_DlgKey ) ;
	m_RollupCtrl.PaneChanged( ) ;
	if( m_pView )
	{
        m_pView->SetUseShotList( false ) ;
		m_pView->SetActorType ( ACTOR_SCENERY ) ;
		m_pView->ActorSetCurrent( NULL ) ;
		m_pView->RedrawWindow( ) ;
	}
}

void CMultiBar::SetupShotList( )
{
	m_RollupCtrl.DeleteAllPanes( ) ;
	AddPaneDialog( "View/Edit Layer", IDD_EDIT_LAYER, &m_DlgLayer ) ;
	AddPaneDialog( "Movie Shot List", IDD_SHOTLIST, &m_DlgShotList ) ;
	m_RollupCtrl.PaneChanged( ) ;
	if( m_pView )
	{
        m_pView->SetUseShotList( true ) ;
		m_pView->ShotListInitialize( ) ;
		m_pView->ModifyViewFlags( 0, MVF_VIEWCAMERAS|MVF_VIEWCAMERATARGETS|MVF_VIEWLIGHTS );
		m_pView->RedrawWindow( ) ;
	}
}

void CMultiBar::SetupPreview( )
{
	m_RollupCtrl.DeleteAllPanes( ) ;
	AddPaneDialog( "View/Edit Layer", IDD_EDIT_LAYER, &m_DlgLayer ) ;
	AddPaneDialog( "Movie Preview", IDD_MOVIEPREVIEW, &m_DlgMoviePreview ) ;
	m_RollupCtrl.PaneChanged( ) ;
	if( m_pView )
	{
        m_pView->SetUseShotList( true ) ;
		m_pView->RedrawWindow( ) ;
	}
}


void CMultiBar::DoneCharacter( )
{
	if( m_pView )
	{
	}
}

void CMultiBar::DoneCamera( )
{
	if( m_pView )
	{
		m_pView->ModifyViewFlags( MVF_VIEWCAMERAS|MVF_VIEWCAMERATARGETS, 0 );
	}
}

void CMultiBar::DoneLight( )
{
	if( m_pView )
	{
		m_pView->ModifyViewFlags( MVF_VIEWLIGHTS, 0 );
	}
}

void CMultiBar::DoneScenery( )
{
	if( m_pView )
	{
	}
}

void CMultiBar::DoneShotList( )
{
	if( m_pView )
	{
		m_pView->ModifyViewFlags( MVF_VIEWCAMERAS|MVF_VIEWCAMERATARGETS|MVF_VIEWLIGHTS, 0 );
	}
}

void CMultiBar::DonePreview( )
{
	if( m_pView )
	{
	}
}

CDlgLayer *CMultiBar::GetDlgLayer( )
{
	CDlgLayer	*pDlg = NULL ;
	if( ::IsWindow(m_DlgLayer.GetSafeHwnd()) )
	{
		pDlg = &m_DlgLayer ;
	}
	return pDlg ;
}

CDlgActorList *CMultiBar::GetDlgActorList( )
{
	CDlgActorList	*pDlg = NULL ;
	if( ::IsWindow(m_DlgActorList.GetSafeHwnd()) )
	{
		pDlg = &m_DlgActorList ;
	}
	return pDlg ;
}

CDlgKey *CMultiBar::GetDlgKey( )
{
	CDlgKey	*pDlg = NULL ;
	if( ::IsWindow(m_DlgKey.GetSafeHwnd()) )
	{
		pDlg = &m_DlgKey ;
	}
	return pDlg ;
}


BEGIN_MESSAGE_MAP(CMultiBar, CControlBar)
	//{{AFX_MSG_MAP(CMultiBar)
	ON_WM_SIZE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP

	ON_NOTIFY( TCN_SELCHANGE, 0, TabSelchange )

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMultiBar message handlers


void CMultiBar::OnSize(UINT nType, int cx, int cy) 
{
	CControlBar::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here

	//---	Make CRollupCtrl track size of CRollupBar
	if( ::IsWindow(m_RollupCtrl.GetSafeHwnd()) )
	{
		//---	Set TabCtrl
		m_Tab.SetWindowPos( NULL, 0, 0, cx, cy, SWP_NOZORDER ) ;
		m_RollupCtrl.SetWindowPos( NULL, 2, 28, cx-4, cy-30, SWP_NOZORDER ) ;

//		int x = 0 ;
//		int y = 32 ;
//		cy -= 32 ;
//		m_RollupCtrl.SetWindowPos( NULL, x, y, cx, cy, SWP_NOZORDER ) ;
	}
}

void CMultiBar::TabSelchange(NMHDR * pNotifyStruct, LRESULT* pResult)
{
//idTabCtl = (int) LOWORD(wParam); lpnmhdr = (LPNMHDR) lParam; 
	if( m_Tab.GetCurSel() == m_LastSelection )
		return;

	//---	deinit the last selection
	switch( m_LastSelection )
	{
	case 0:	DoneCharacter( ) ;	break ;
	case 1:	DoneCamera( ) ;	break ;
	case 2:	DoneLight( ) ;		break ;
	case 3:	DoneScenery( ) ;	break ;
	case 4:	DoneShotList( ) ;	break ;
	case 5:	DonePreview( ) ;	break ;
	}

	//---	initialize the new selection
	switch( m_Tab.GetCurSel() )
	{
	case 0:	SetupCharacter( ) ;	break ;
	case 1:	SetupCamera( ) ;	break ;
	case 2:	SetupLight( ) ;		break ;
	case 3:	SetupScenery( ) ;	break ;
	case 4:	SetupShotList( ) ;	break ;
	case 5:	SetupPreview( ) ;	break ;
	}

	//---	store the last selection
	m_LastSelection = m_Tab.GetCurSel();
}

void CMultiBar::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
//	m_RollupCtrl.RedrawWindow( ) ;

	// Do not call CControlBar::OnPaint() for painting messages
}
