// RollupPane.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "RollupPane.h"
#include "RollupCtrl.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define	PANE_BORDERW		2

#define	PANE_TOPBORDER		7
#define PANE_BUTTONH		16


/////////////////////////////////////////////////////////////////////////////
// CRollupPane

CRollupPane::CRollupPane()
{
	m_pDialog = NULL ;
	m_Open = false ;
	m_ButtonPressed = false ;
}

CRollupPane::~CRollupPane()
{
	if( ::IsWindow(m_pDialog->GetSafeHwnd()) )
	{
		m_pDialog->DestroyWindow( ) ;
	}
	m_pDialog = NULL ;
}



int CRollupPane::GetHeight( )
{
	CRect r ;
	GetWindowRect( &r ) ;
	return r.Height() ;
}

bool CRollupPane::IsOpen( )
{
	return m_Open ;
}

bool CRollupPane::IsClosed( )
{
	return !m_Open ;
}

bool CRollupPane::SetOpen( bool Open )
{
	if( m_Open != Open )
	{
		m_Open = Open ;
		BOOL	DialogExists = ::IsWindow(m_pDialog->GetSafeHwnd()) ;
		if( m_Open && DialogExists )
		{
			//---	Set Window Size when Open
			CRect rw, rd ;
			GetWindowRect( &rw ) ;
			m_pDialog->GetClientRect( &rd ) ;
			SetWindowPos( NULL, 0, 0, rw.Width(), PANE_BUTTONH+PANE_BORDERW+rd.Height(), SWP_NOMOVE|SWP_NOZORDER ) ;
		}
		else
		{
			//---	Set Window Size when Closed
			CRect rw ;
			GetWindowRect( &rw ) ;
			SetWindowPos( NULL, 0, 0, rw.Width(), PANE_BUTTONH, SWP_NOMOVE|SWP_NOZORDER ) ;
		}

		m_pRollupCtrl->PaneChanged( ) ;
		m_pRollupCtrl->LimitScrollPos( ) ;
		if( DialogExists )
		{
			m_pDialog->ShowWindow( m_Open ? SW_SHOW : SW_HIDE ) ;
			m_pDialog->RedrawWindow( ) ;
		}
	}

	return m_Open ;
}

BEGIN_MESSAGE_MAP(CRollupPane, CWnd)
	//{{AFX_MSG_MAP(CRollupPane)
	ON_WM_PAINT()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_NCHITTEST()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CRollupPane message handlers

void CRollupPane::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	//---	Get Client Rectangle
	CRect r ;
	GetClientRect( &r ) ;

	//---	Clear Background
	dc.FillSolidRect( &r, ::GetSysColor( COLOR_BTNFACE ) ) ;
}

BOOL CRollupPane::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: Add your specialized code here and/or call the base class
	
	BOOL Success = CWnd::Create(lpszClassName, lpszWindowName, dwStyle|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, rect, pParentWnd, nID, pContext) ;

	return Success ;
}

void CRollupPane::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	// TODO: Add your message handler code here and/or call default
	
	lpncsp->rgrc[0].top		+= PANE_BUTTONH ;
	if( IsOpen() )
		lpncsp->rgrc[0].bottom	-= PANE_BORDERW ;
	lpncsp->rgrc[0].left	+= PANE_BORDERW ;
	lpncsp->rgrc[0].right	-= PANE_BORDERW ;
}

void CRollupPane::OnNcPaint() 
{
	CDC *pDC = GetWindowDC( ) ;
	ASSERT( pDC ) ;

	//---	Get Window Rectangle
	CRect r ;
	GetWindowRect( &r ) ;
	r.OffsetRect( -r.TopLeft() ) ;

	//---	Fill TOPBORDER with space
	CRect r2 = r ;
	r2.bottom = r2.top + PANE_BUTTONH ;
	pDC->FillSolidRect( &r2, ::GetSysColor( COLOR_BTNFACE ) ) ;

	//---	Draw Outline
	if( IsOpen() )
	{
		r.top += PANE_TOPBORDER ;
		pDC->Draw3dRect( &r, ::GetSysColor( COLOR_3DSHADOW ), ::GetSysColor( COLOR_3DLIGHT ) ) ;
		r.DeflateRect( 1, 1 ) ;
		pDC->Draw3dRect( &r, ::GetSysColor( COLOR_3DLIGHT ), ::GetSysColor( COLOR_3DSHADOW ) ) ;
	}
	else
	{
		r.top		+= 4 ;
		r.bottom	-= 4 ;
		pDC->Draw3dRect( &r, ::GetSysColor( COLOR_3DSHADOW ), ::GetSysColor( COLOR_3DLIGHT ) ) ;
		r.DeflateRect( 1, 1 ) ;
		pDC->Draw3dRect( &r, ::GetSysColor( COLOR_3DLIGHT ), ::GetSysColor( COLOR_3DSHADOW ) ) ;
	}

	//---	Clear Button Area
	r2.DeflateRect( 6, 0 ) ;
//	pDC->FillSolidRect( &r2, ::GetSysColor( COLOR_BTNFACE ) ) ;
	pDC->FillSolidRect( &r2, RGB( 192-10, 192+20, 192-10 ) ) ;

	//---	Draw - / + for Open / Closed
	CRect r3 = r2 ;
	r3.DeflateRect( 2, 2 ) ;
	r3.right = r3.left + 15 ;
	r3.DeflateRect( 4, 3 ) ;
	pDC->MoveTo( r3.left, r3.CenterPoint().y-1 ) ;
	pDC->LineTo( r3.right, r3.CenterPoint().y-1 ) ;
	if( IsClosed() )
	{
		pDC->MoveTo( r3.CenterPoint().x, r3.top-1 ) ;
		pDC->LineTo( r3.CenterPoint().x, r3.bottom ) ;
	}

	//---	Draw Name of Pane
	CFont Font ;
	Font.CreatePointFont( 8, "MS Sans Serif", pDC ) ;
	CFont *pOldFont = pDC->SelectObject( &Font ) ;
	r3 = r2 ;
	r3.DeflateRect( 2, 2 ) ;
	r3.left += 15 ;
	r3.top -= 1 ;
	CString String ;
	GetWindowText( String ) ;
	pDC->DrawText( String, r3, DT_CENTER ) ;
	pDC->SelectObject( pOldFont ) ;

	//---	Draw Button Outline
	if( m_ButtonPressed )
	{
		pDC->Draw3dRect( &r2, ::GetSysColor( COLOR_3DSHADOW ), ::GetSysColor( COLOR_3DLIGHT ) ) ;
		r2.DeflateRect( 1, 1 ) ;
		pDC->Draw3dRect( &r2, ::GetSysColor( COLOR_3DSHADOW ), ::GetSysColor( COLOR_3DLIGHT ) ) ;
		r2.DeflateRect( 1, 1 ) ;
	}
	else
	{
		pDC->Draw3dRect( &r2, ::GetSysColor( COLOR_3DLIGHT ), ::GetSysColor( COLOR_3DSHADOW ) ) ;
		r2.DeflateRect( 1, 1 ) ;
		pDC->Draw3dRect( &r2, ::GetSysColor( COLOR_3DLIGHT ), ::GetSysColor( COLOR_3DSHADOW ) ) ;
		r2.DeflateRect( 1, 1 ) ;
	}

	ReleaseDC( pDC );
}

UINT CRollupPane::OnNcHitTest(CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	return HTCLIENT ;

//	return CWnd::OnNcHitTest(point);
}

void CRollupPane::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	CRect r ;
	GetWindowRect( &r ) ;
	CRect r2 = r ;
	r2.bottom = r2.top + PANE_BUTTONH ;
	r2.DeflateRect( 6, 0 ) ;
	ClientToScreen( &point ) ;

	if( r2.PtInRect( point ) )
	{
		m_ButtonPressed = true ;
		RedrawWindow( NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME ) ;
		SetCapture( ) ;
	}

	CWnd::OnLButtonDown(nFlags, point);
}

void CRollupPane::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	ReleaseCapture( ) ;

	if( m_ButtonPressed )
	{
		SetOpen( !IsOpen() ) ;
		m_ButtonPressed = false ;
		RedrawWindow( NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME ) ;
	}

	CWnd::OnLButtonUp(nFlags, point);
}

void CRollupPane::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	CRect r ;
	GetWindowRect( &r ) ;
	CRect r2 = r ;
	r2.bottom = r2.top + PANE_BUTTONH ;
	r2.DeflateRect( 6, 0 ) ;
	ClientToScreen( &point ) ;

	if( GetCapture( ) == this )
	{
		if( (r2.PtInRect( point ) == TRUE ) != m_ButtonPressed )
		{
			m_ButtonPressed = (r2.PtInRect( point ) == TRUE ) ;
			RedrawWindow( NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME ) ;
		}
	}

	CWnd::OnMouseMove(nFlags, point);
}

void CRollupPane::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	OnLButtonDown( nFlags, point ) ;

//	CWnd::OnLButtonDblClk(nFlags, point);
}
