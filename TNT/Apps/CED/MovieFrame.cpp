// MovieFrame.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "MovieFrame.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMovieFrame

IMPLEMENT_DYNCREATE(CMovieFrame, CChildFrame)

CMovieFrame::CMovieFrame()
{
	m_pMovie = NULL ;
}

CMovieFrame::~CMovieFrame()
{
}


BEGIN_MESSAGE_MAP(CMovieFrame, CChildFrame)
	//{{AFX_MSG_MAP(CMovieFrame)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMovieFrame message handlers

int CMovieFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CChildFrame::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	
	//---	Setup Tool Bar
	if( !m_ToolBar.Create( this ) ||
		!m_ToolBar.LoadToolBar( IDR_MOVIE ) )
	{
		TRACE0( "Failed to create toolbar\n" ) ;
		return -1 ;      // fail to create
	}

	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_ToolBar.SetBarStyle( m_ToolBar.GetBarStyle() |
		CBRS_LEFT | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ;

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_ToolBar.EnableDocking( CBRS_ALIGN_ANY ) ;
	EnableDocking( CBRS_ALIGN_ANY ) ;
	DockControlBar( &m_ToolBar, AFX_IDW_DOCKBAR_LEFT ) ;

	//---	Setup Rollup Bar
	m_RollupBar.SetWidth( (int)(140*1.65) ) ;
	m_RollupBar.Create( this, CBRS_RIGHT | CBRS_TOOLTIPS | CBRS_FLYBY, 4 ) ;
	m_RollupBar.SetBarStyle( m_RollupBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ;
	
	//---	Setup Movie Bar
	m_MovieBar.Create( this, IDD_MOVIECTRL, CBRS_BOTTOM | CBRS_TOOLTIPS | CBRS_FLYBY, 3 ) ;
	m_MovieBar.SetBarStyle( m_MovieBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ;

	return 0;
}

void CMovieFrame::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	
	CChildFrame::OnClose();
}

void CMovieFrame::OnDestroy() 
{
	// TODO: Add your message handler code here
	m_RollupBar.m_DlgActorList.m_pView = NULL ;
	m_RollupBar.m_DlgKey.m_pView = NULL ;

	CChildFrame::OnDestroy();
}
