// MovieBar.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "MovieBar.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "MovieView.h"

/////////////////////////////////////////////////////////////////////////////
// CMovieBar

CMovieBar::CMovieBar()
{
	m_pView = NULL ;
}

CMovieBar::~CMovieBar()
{
}


BEGIN_MESSAGE_MAP(CMovieBar, CDialogBar)
	//{{AFX_MSG_MAP(CMovieBar)
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP

	ON_EN_CHANGE( IDC_FRAME, OnChangeFrame )
	ON_EN_CHANGE( IDC_TOTAL, OnChangeNumFrames )

END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Implementation

void CMovieBar::SetView( CMovieView *pView )
{
	m_pView = pView ;
}

void CMovieBar::Update( )
{
}

void CMovieBar::SetNumFrames( int NumFrames )
{
	CSliderCtrl *pSlider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER ) ;
	CEdit *pNumFrames = (CEdit*)GetDlgItem( IDC_TOTAL ) ;
	CSpinButtonCtrl *pSpin = (CSpinButtonCtrl*)GetDlgItem( IDC_SPIN ) ;

	pSlider->SetRange( 0, max(0,NumFrames-1), TRUE ) ;
	pSlider->SetPageSize( 1 ) ;
	pSlider->SetLineSize( 1 ) ;

	CString String ;
	pNumFrames->GetWindowText( String ) ;
	if( (atoi(String) != NumFrames) || (String.GetLength() == 0) )
	{
		String.Format( "%d", max(0,NumFrames) ) ;
		pNumFrames->SetWindowText( String ) ;
		pNumFrames->RedrawWindow() ;
	}

	pSpin->SetRange( 0, max(0,NumFrames-1) ) ;
	pSpin->RedrawWindow() ;
}

void CMovieBar::SetFrame( int nFrame )
{
	CSliderCtrl *pSlider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER ) ;
	CSpinButtonCtrl *pSpin = (CSpinButtonCtrl*)GetDlgItem( IDC_SPIN ) ;
	CEdit *pFrame = (CEdit*)GetDlgItem( IDC_FRAME ) ;

	pSlider->SetPos( nFrame ) ;
//    pSlider->RedrawWindow( ) ;

	CString String ;
	pFrame->GetWindowText( String ) ;
	if( (atoi(String) != nFrame) || (String.GetLength() == 0) )
	{
		pSpin->SetPos( nFrame ) ;
		pFrame->RedrawWindow() ;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMovieBar message handlers

void CMovieBar::OnSize(UINT nType, int cx, int cy)
{
	CDialogBar::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

	CWnd	*pWnd ;

	cx = max( 128, cx ) ;

	pWnd = GetDlgItem( IDC_SLIDER ) ;
	if( pWnd ) pWnd->SetWindowPos( NULL, 100, 2, cx-100-16, 32, SWP_NOZORDER ) ;

}

void CMovieBar::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default

	if( m_pView )
	{
		int nFrame = m_pView->GetFrame( ) ;

		switch( nSBCode )
		{
		case SB_LEFT:
			nFrame = 0 ;
			break ;
		case SB_ENDSCROLL:
			break ;
		case SB_LINELEFT:
		case SB_PAGELEFT:
			nFrame = nFrame - 1 ;
			break ;
		case SB_LINERIGHT:
		case SB_PAGERIGHT:
			nFrame = nFrame + 1 ;
			break ;
		case SB_RIGHT:
			nFrame = max(0,m_pView->GetNumFrames()-1) ;
			break ;
		case SB_THUMBPOSITION:
			nFrame = nPos ;
			break ;
		case SB_THUMBTRACK:
			nFrame = nPos ;
			break ;
		}

		m_pView->SetFrame( nFrame ) ;
	}

//	CDialogBar::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CMovieBar::OnChangeFrame( )
{
	CEdit *pFrame = (CEdit*)GetDlgItem( IDC_FRAME ) ;

	if( m_pView )
	{
		CString String ;
		pFrame->GetWindowText( String ) ;
		if( atoi( String ) != m_pView->GetFrame() )
		{
//			bool OldEditChange = m_EditChange ;
//			m_EditChange = true
			m_pView->SetFrame( atoi(String) ) ;
//			m_EditChange = OldEditChange ;
		}
	}
}

void CMovieBar::OnChangeNumFrames( )
{
	CEdit *pFrame = (CEdit*)GetDlgItem( IDC_TOTAL ) ;

	if( m_pView )
	{
		CString String ;
		pFrame->GetWindowText( String ) ;
		if( atoi( String ) != m_pView->GetNumFrames() )
		{
//			bool OldEditChange = m_EditChange ;
//			m_EditChange = true
			m_pView->SetNumFrames( atoi(String) ) ;
//			m_EditChange = OldEditChange ;
		}
	}
}

BOOL CMovieBar::Create( CWnd* pParentWnd, UINT nIDTemplate, UINT nStyle, UINT nID )
{
	BOOL Success = CDialogBar::Create( pParentWnd, nIDTemplate, nStyle, nID ) ;

	return Success;
}
