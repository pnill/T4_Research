// MotionEditFrame.cpp : implementation file
//

#include "stdafx.h"
#include "CeD.h"
#include "MotionEditFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "CeDDoc.h"
#include "Skeleton.h"
#include "MotionEditView.h"

/////////////////////////////////////////////////////////////////////////////
// CMotionEditFrame

IMPLEMENT_DYNCREATE(CMotionEditFrame, CChildFrame)

CMotionEditFrame::CMotionEditFrame()
{
	m_pView = NULL ;
}

CMotionEditFrame::~CMotionEditFrame()
{
}


BEGIN_MESSAGE_MAP(CMotionEditFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CMotionEditFrame)
	ON_WM_CREATE()
	ON_WM_CONTEXTMENU()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMotionEditFrame message handlers

int CMotionEditFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if( CMDIChildWnd::OnCreate( lpCreateStruct ) == -1 )
		return -1 ;

	//---	Setup Tool Bar
	if( !m_wndToolBar.Create( this ) ||
		!m_wndToolBar.LoadToolBar( IDR_MOTIONEDIT ) )
	{
		TRACE0( "Failed to create toolbar\n" ) ;
		return -1 ;      // fail to create
	}

	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_wndToolBar.SetBarStyle( m_wndToolBar.GetBarStyle() |
		CBRS_LEFT | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ;

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking( CBRS_ALIGN_ANY ) ;
	EnableDocking( CBRS_ALIGN_ANY ) ;
	DockControlBar( &m_wndToolBar, AFX_IDW_DOCKBAR_LEFT ) ;

	//---	Initialize ToolBar Buttons
	m_wndToolBar.SetButtonStyle( m_wndToolBar.CommandToIndex( ID_STATE_SELECT ), TBBS_CHECKBOX ) ;
	m_wndToolBar.SetButtonStyle( m_wndToolBar.CommandToIndex( ID_STATE_PAN ), TBBS_CHECKBOX ) ;
	m_wndToolBar.SetButtonStyle( m_wndToolBar.CommandToIndex( ID_STATE_ROTATE ), TBBS_CHECKBOX ) ;
	m_wndToolBar.SetButtonStyle( m_wndToolBar.CommandToIndex( ID_STATE_ZOOM ), TBBS_CHECKBOX ) ;
	m_wndToolBar.SetButtonStyle( m_wndToolBar.CommandToIndex( ID_STATE_MOVEPLANEY ), TBBS_CHECKBOX ) ;
	m_wndToolBar.SetButtonStyle( m_wndToolBar.CommandToIndex( ID_STATE_FACEDIR ), TBBS_CHECKBOX ) ;
	m_wndToolBar.SetButtonStyle( m_wndToolBar.CommandToIndex( ID_ANIM_PLAY ), TBBS_CHECKBOX ) ;

	//---	Setup Dialog Bar
//	m_wndDialogBar.Create( this, IDD_TEST, CBRS_RIGHT | CBRS_TOOLTIPS | CBRS_FLYBY, 2 ) ;
//	m_wndDialogBar.SetBarStyle( m_wndDialogBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ;
//	m_wndDialogBar.EnableDocking( CBRS_ALIGN_ANY ) ;
//	DockControlBar( &m_wndDialogBar ) ;

	//---	Setup Rollup Bar
	m_RollupBar.SetWidth( (int)(140*1.65) ) ;
	m_RollupBar.Create( this, CBRS_RIGHT | CBRS_TOOLTIPS | CBRS_FLYBY, 4 ) ;
	m_RollupBar.SetBarStyle( m_RollupBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ;

	m_RollupBar.AddPaneDialog( "Character Select", IDD_CHARACTER_SELECT, &m_DlgCharacter ) ;
	m_RollupBar.AddPaneDialog( "Camera Control", IDD_CAMERA1, &m_DlgCamera ) ;
	m_RollupBar.AddPaneDialog( "Motion List", IDD_MOTIONLIST, &m_DlgMotionList ) ;
	m_RollupBar.AddPaneDialog( "Event List", IDD_EVENTLIST, &m_DlgEventList ) ;
	m_RollupBar.AddPaneDialog( "Skeleton Point List", IDD_POINTLIST, &m_DlgSkelPointList ) ;

	//---	Setup Anim Bar
	m_wndAnimBar.Create( this, IDD_ANIMCTRL, CBRS_BOTTOM | CBRS_TOOLTIPS | CBRS_FLYBY, 3 ) ;
	m_wndAnimBar.SetBarStyle( m_wndAnimBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ;

	return 0 ;
}

void CMotionEditFrame::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// TODO: Add your message handler code here
	CMenu menu ;
	VERIFY(menu.LoadMenu(IDR_MED_POPUP)) ;
	CMenu* pPopup = menu.GetSubMenu(0) ;
	ASSERT(pPopup != NULL) ;

	//---	no menu is used if the active view is changing an event Y
	//		coordinate (this uses the right mouse button)
	CMotionEditView* pView = (CMotionEditView*)GetActiveView();
	if(( pView->GetViewState() == MEVS_SET_EVENT ) && pView->MouseMoved() )
		return;

	//---	Enable Menu Items Accordingly
	CCeDDoc	*pDoc = (CCeDDoc*)GetActiveDocument() ;
	if( pDoc )
	{
		pDoc->UpdateMenuItems( pPopup ) ;
	}

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this) ;
}

void CMotionEditFrame::OnClose() 
{
	// TODO: Add your message handler code here and/or call default

	if( m_pView )
		m_pView->m_Playing = false ;

	CMDIChildWnd::OnClose();
}
