// MouseStatic.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "MouseStatic.h"
#include "x_types.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMouseStatic

CMouseStatic::CMouseStatic()
{
	m_pNotifyWindow = NULL ;
}

CMouseStatic::~CMouseStatic()
{
}

void CMouseStatic::SetNotifyWindow( CWnd *pWnd )
{
	m_pNotifyWindow = pWnd ;
}

BEGIN_MESSAGE_MAP(CMouseStatic, CStatic)
	//{{AFX_MSG_MAP(CMouseStatic)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMouseStatic message handlers

void CMouseStatic::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	SetCapture( ) ;
	::GetCursorPos( &m_MouseClick ) ;

	CStatic::OnLButtonDown(nFlags, point);
}

void CMouseStatic::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	if( GetCapture( ) == this )
	{
		ClientToScreen( &point ) ;
		CPoint Delta = point - m_MouseClick ;
		::SetCursorPos( m_MouseClick.x, m_MouseClick.y ) ;

		if( m_pNotifyWindow != NULL )
		{
			m_pNotifyWindow->SendMessage( MSM_MOUSEMOVE, GetDlgCtrlID(), ( (((u16)Delta.x)<<16) | ((u16)Delta.y) ) ) ;
		}
	}

	CStatic::OnMouseMove(nFlags, point);
}

void CMouseStatic::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	ReleaseCapture( ) ;

	CStatic::OnLButtonUp(nFlags, point);
}

BOOL CMouseStatic::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// TODO: Add your message handler code here and/or call default

	::SetCursor( ::LoadCursor( NULL, IDC_SIZEALL )) ;
	
//	return CStatic::OnSetCursor(pWnd, nHitTest, message);
	return 0 ;
}
