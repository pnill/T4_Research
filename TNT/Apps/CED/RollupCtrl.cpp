// RollupCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "RollupCtrl.h"

#include "RollupPane.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	ROLL_BORDERW	1
#define	ROLL_SLIDERW	8
#define ROLL_SPLITTERW  5

#define	ROLL_PANEBORDER	2
#define	ROLL_PANEGAP	4

/////////////////////////////////////////////////////////////////////////////
// CRollupCtrl

CRollupCtrl::CRollupCtrl()
{
	m_TotalChildHeight = 0 ;
	m_ScrollPos = 0 ;
}

CRollupCtrl::~CRollupCtrl()
{
	for( int i = 0 ; i < m_Panes.GetSize() ; i++ )
	{
		CRollupPane *pPane = (CRollupPane*)m_Panes[i] ;
		pPane->DestroyWindow( ) ;
		delete pPane ;
	}
}


/////////////////////////////////////////////////////////////////////////////
// Pane Management

CRollupPane *CRollupCtrl::AddPaneDialog( CString Name, UINT nTemplateID, CDialog *pDialog )
{
	//---	Get Rect which is Valid on X for Pane
	CRect r ;
	GetClientRect( &r ) ;
    r.left  += ROLL_SPLITTERW ;
	r.right -= ROLL_SLIDERW ;
	r.DeflateRect( ROLL_PANEBORDER, 0 ) ;
	r.OffsetRect( 0, GetTotalPaneHeight()-1 ) ;

	//---	Create the Pane as a Child of the Control
	CRollupPane *pPane = new CRollupPane ;
	ASSERT( pPane ) ;
	pPane->m_pRollupCtrl = this ;
	pPane->Create( NULL, Name, WS_CHILD, r, this, m_Panes.GetSize() ) ;

	//---	Create the Dialog as a Child of the Pane
	pDialog->Create( nTemplateID, pPane ) ;
	pPane->m_pDialog = pDialog ;

	//---	Add Pane to List
	m_Panes.Add( pPane ) ;

	//---	Offset Pane to Correct Position
	pPane->SetOpen( true ) ;

	//---	Ensure All panes are in correct position and make new pane visible
	PaneChanged( ) ;
	pPane->ShowWindow( SW_SHOW ) ;
//	pPane->RedrawWindow( ) ;

	//---	Return Pointer to Pane
	return pPane ;
}

void CRollupCtrl::DeleteAllPanes( )
{
//	SetRedraw( FALSE ) ;
	for( int i = m_Panes.GetSize()-1 ; i >= 0 ; i-- )
	{
		CRollupPane *pPane = (CRollupPane*)m_Panes[i] ;
		delete pPane ;
	}
	m_Panes.SetSize( 0 ) ;
//	SetRedraw( TRUE ) ;
	PaneChanged( ) ;
}

void CRollupCtrl::PaneChanged( )
{
	//---	Reposition All Panes correctly
	RepositionPanes( ) ;
}

void CRollupCtrl::RepositionPanes( )
{
	int		PaneY = -m_ScrollPos ;

	//---	Loop through each Pane
	for( int i = 0 ; i < m_Panes.GetSize() ; i++ )
	{
		CRect rp ;
		
		//---	Get Pane Rectangle
		CRollupPane *pPane = (CRollupPane*)m_Panes[i] ;
		pPane->GetWindowRect( &rp ) ;
		pPane->SetWindowPos( NULL, ROLL_PANEBORDER, PaneY, 0, 0, SWP_NOSIZE | SWP_NOZORDER ) ;
		pPane->RedrawWindow( ) ;
		PaneY += rp.Height() + ROLL_PANEGAP ;
	}
	Invalidate( ) ;
}

int CRollupCtrl::GetTotalPaneHeight( )
{
	int		TotalHeight = 0 ;

	for( int i = 0 ; i < m_Panes.GetSize() ; i++ )
	{
		CRollupPane *pPane = (CRollupPane*)m_Panes[i] ;
		TotalHeight += pPane->GetHeight() ;
		if( i > 0 ) TotalHeight += ROLL_PANEGAP ;
	}

	return TotalHeight ;
}

int CRollupCtrl::GetVisiblePaneHeight( )
{
	CRect r ;
	GetClientRect( &r ) ;
	return r.Height() ;
}

int CRollupCtrl::SetScrollPos( int ScrollPos )
{
	int	MaxScrollPos = GetTotalPaneHeight() - GetVisiblePaneHeight() ;
	ScrollPos = min( ScrollPos, MaxScrollPos ) ;
	ScrollPos = max( ScrollPos, 0 ) ;
	m_ScrollPos = ScrollPos ;

	//---	Set Panes after Scrolling
	RepositionPanes( ) ;

	//---	Redraw Ctrl
	Invalidate() ;

	return m_ScrollPos ;
}

int CRollupCtrl::GetScrollPos( )
{
	return m_ScrollPos ;
}

void CRollupCtrl::LimitScrollPos( )
{
	SetScrollPos( GetScrollPos() ) ;
}


BEGIN_MESSAGE_MAP(CRollupCtrl, CWnd)
	//{{AFX_MSG_MAP(CRollupCtrl)
	ON_WM_NCPAINT()
	ON_WM_SETCURSOR()
	ON_WM_NCCALCSIZE()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CRollupCtrl message handlers

void CRollupCtrl::DrawSlider( CDC *pDC )
{
	//---	Get Container Rect for Slider
	CRect r ;
	pDC->GetWindow()->GetClientRect( &r ) ;
	r.left		 = r.right - ROLL_SLIDERW ;

	//---	Calculate Height of Visible Area
	double VisiblePercent = (double)GetVisiblePaneHeight() / (double)GetTotalPaneHeight() ;

	//---	If There is more data than can be displayed then draw slider
	if( VisiblePercent < 1.0 )
	{
		//---	Draw Top Part of Slider Container
		if( m_ScrollPos > 0 )
		{
			CRect r2 = r ;
			r2.bottom = r.top + (int)(m_ScrollPos * VisiblePercent) ;
			pDC->FillSolidRect( &r2, RGB(64,64,64) ) ;
		}

		//---	Draw Bottom Part of Slider Container
		if( ((m_ScrollPos + r.Height()) * VisiblePercent) < r.Height() )
		{
			CRect r2 = r ;
			r2.top = r.top + (int)((m_ScrollPos + r.Height()) * VisiblePercent) ;
			pDC->FillSolidRect( &r2, RGB(64,64,64) ) ;
		}

		//---	Draw Slider Handle
		CRect r2 = r ;
		r2.top = r.top + (int)(m_ScrollPos * VisiblePercent) ;
		r2.bottom = r.top + (int)((m_ScrollPos + r.Height()) * VisiblePercent) ;
		pDC->FillSolidRect( &r2, RGB(128,128,128) ) ;
	}
	else
	{
		//---	Fill slot where slider goes
		pDC->FillSolidRect( &r, ::GetSysColor( COLOR_BTNFACE ) ) ;
	}
}

void CRollupCtrl::OnNcPaint() 
{
	// TODO: Add your message handler code here
	
	// Do not call CWnd::OnNcPaint() for painting messages

	//---	Get DC for Entire Window
	CDC *pDC = GetWindowDC( ) ;
	ASSERT( pDC ) ;

	//---	Get Rectangle for Size of Everything
	CRect r ;
	pDC->GetWindow()->GetWindowRect( &r ) ;
	r.OffsetRect( -r.TopLeft() ) ;

	//---	Draw Frame
	for( int i = 0 ; i < ROLL_BORDERW ; i++ )
	{
		pDC->Draw3dRect( r, ::GetSysColor( COLOR_3DSHADOW ), ::GetSysColor( COLOR_3DLIGHT ) ) ;
		r.DeflateRect( 1, 1 ) ;
	}

	//---	Release DC
	ReleaseDC( pDC ) ;
}

BOOL CRollupCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// TODO: Add your message handler code here and/or call default

	//---	Get Slider Bar Rectangle
	CRect r, r2 ;
	GetClientRect( &r ) ;
    r2 = r ;
	r.left = r.right - ROLL_SLIDERW ;
    r2.right = r2.left + ROLL_SPLITTERW ;
	CPoint p ;
	GetCursorPos( &p ) ;
	ScreenToClient( &p ) ;

    //---   Check if Cursor within Splitter
    if( r2.PtInRect( p ) )
    {
		SetCursor (AfxGetApp()->LoadStandardCursor( IDC_SIZEWE )) ;
		return 0 ;
    }

	//---	Check if Cursor within Slider Bar
	if( r.PtInRect( p ) && ( GetTotalPaneHeight() > GetVisiblePaneHeight() ) )
	{
		SetCursor (AfxGetApp()->LoadCursor( IDC_PAN )) ;
		return 0 ;
	}
	
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CRollupCtrl::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	// TODO: Add your message handler code here and/or call default
	
	lpncsp->rgrc[0].top		+= ROLL_BORDERW ;
	lpncsp->rgrc[0].bottom	-= ROLL_BORDERW ;
	lpncsp->rgrc[0].left	+= ROLL_BORDERW ;
	lpncsp->rgrc[0].right	-= ROLL_BORDERW ;

//	CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);
}

void CRollupCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	CRect r ;
	GetClientRect( &r ) ;
	r.right -= ROLL_SLIDERW ;
	dc.FillSolidRect( r, ::GetSysColor( COLOR_BTNFACE ) ) ;
	DrawSlider( &dc ) ;

/*
	for( int i = 0 ; i < m_Panes.GetSize() ; i++ )
	{
		CRollupPane *pPane = (CRollupPane*)m_Panes[i] ;
		pPane->Invalidate( ) ;
		pPane->RedrawWindow( NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN ); 
	}
*/

	// Do not call CWnd::OnPaint() for painting messages
}

BOOL CRollupCtrl::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	// TODO: Add your specialized code here and/or call the base class

	//---	Create Control Window
	BOOL Success = CWnd::Create(lpszClassName, lpszWindowName, dwStyle|WS_CLIPCHILDREN, rect, pParentWnd, nID, pContext);

	//---	Return Success
	return Success ;
}

void CRollupCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	m_MouseClick = point ;
	m_MouseLast = point ;
	SetCapture( ) ;

	CWnd::OnLButtonDown(nFlags, point);
}

void CRollupCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	ReleaseCapture( ) ;

	CWnd::OnLButtonUp(nFlags, point);
}

void CRollupCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	if( GetCapture( ) == this )
	{
		m_MouseDelta = point - m_MouseLast ;
		m_MouseLast = point ;
		SetScrollPos( GetScrollPos( ) + m_MouseDelta.y ) ;
	}

	CWnd::OnMouseMove(nFlags, point);
}

void CRollupCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here

	//---	Limit ScrollPos if window got larger
	if( ::IsWindow( GetSafeHwnd() ) )
	{
		LimitScrollPos( ) ;
	}
}
