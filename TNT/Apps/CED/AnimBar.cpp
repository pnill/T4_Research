// AnimBar.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "AnimBar.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "MotionEditView.h"

/////////////////////////////////////////////////////////////////////////////
// CAnimBar

CAnimBar::CAnimBar()
{
	m_NumFrames = 0 ;
	m_nFrame = 0 ;
	m_pView = NULL ;
	m_bRangesActive	= FALSE;
}

CAnimBar::~CAnimBar()
{
}


BEGIN_MESSAGE_MAP(CAnimBar, CDialogBar)
	//{{AFX_MSG_MAP(CAnimBar)
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP

	ON_EN_CHANGE( IDC_FRAME, OnChangeFrame )

END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Implementation

void CAnimBar::SetView( CMotionEditView *pView )
{
	m_pView = pView ;
}

void CAnimBar::Update( )
{
}

void CAnimBar::SetRange( s32 Range )
{
	CSliderCtrl *pSlider = (CSliderCtrl*)GetDlgItem( IDC_ANIM_SLIDER ) ;
	CEdit *pNumFrames = (CEdit*)GetDlgItem( IDC_FRAME_TOTAL ) ;
	CSpinButtonCtrl *pSpin = (CSpinButtonCtrl*)GetDlgItem( IDC_FRAME_SPIN ) ;

	m_NumFrames = Range;
	pSlider->SetRange( 0, max(0, m_NumFrames-1), TRUE ) ;	
	pSlider->SetPageSize( 1 ) ;
	pSlider->SetLineSize( 1 ) ;

	CString String ;
	String.Format( "%d", max(0,m_NumFrames) ) ;
	pNumFrames->SetWindowText( String ) ;
	pNumFrames->RedrawWindow() ;
	pSpin->SetRange( 0, max(0,m_NumFrames-1) ) ;
	pSpin->RedrawWindow() ;
}

void CAnimBar::SetMotion( CMotion *pMotion )
{
	if( pMotion )
		SetRange( pMotion->GetNumFrames() );
}

void CAnimBar::SetFrame( int nFrame )
{
	CSliderCtrl *pSlider = (CSliderCtrl*)GetDlgItem( IDC_ANIM_SLIDER ) ;
	CSpinButtonCtrl *pSpin = (CSpinButtonCtrl*)GetDlgItem( IDC_FRAME_SPIN ) ;
	CEdit *pFrame = (CEdit*)GetDlgItem( IDC_FRAME ) ;

	m_nFrame = nFrame ;
	pSlider->SetPos( nFrame ) ;

	CString String ;
	pFrame->GetWindowText( String ) ;
	if( atoi(String) != nFrame )
	{
		pSpin->SetPos( nFrame ) ;
		pFrame->RedrawWindow() ;
	}
}

void CAnimBar::SetSelStart( )
{
	m_nStartFrame = m_nFrame ;
	CSliderCtrl *pSlider = (CSliderCtrl*)GetDlgItem( IDC_ANIM_SLIDER ) ;
	pSlider->SetSelection( m_nStartFrame, m_nEndFrame ) ;
	pSlider->Invalidate() ;
}

void CAnimBar::SetSelEnd( )
{
	m_nEndFrame = m_nFrame ;
	CSliderCtrl *pSlider = (CSliderCtrl*)GetDlgItem( IDC_ANIM_SLIDER ) ;
	pSlider->SetSelection( m_nStartFrame, m_nEndFrame ) ;
	pSlider->Invalidate() ;
}

void CAnimBar::GetSelection( int *pStart, int *pEnd )
{
	CSliderCtrl *pSlider = (CSliderCtrl*)GetDlgItem( IDC_ANIM_SLIDER ) ;
	pSlider->GetSelection( *pStart, *pEnd ) ;
}

/////////////////////////////////////////////////////////////////////////////
// CAnimBar message handlers

void CAnimBar::OnSize(UINT nType, int cx, int cy)
{
	CDialogBar::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

	CWnd	*pWnd ;

	cx = max( 128, cx ) ;

	pWnd = GetDlgItem( IDC_ANIM_SLIDER ) ;
	if( pWnd ) pWnd->SetWindowPos( NULL, 88, 2, cx-56-88, 32, SWP_NOZORDER ) ;

	pWnd = GetDlgItem( IDC_FRAME_START ) ;
	if( pWnd ) pWnd->SetWindowPos( NULL, cx-50, 4, 21, 21, SWP_NOZORDER ) ;

	pWnd = GetDlgItem( IDC_FRAME_END ) ;
	if( pWnd ) pWnd->SetWindowPos( NULL, cx-25, 4, 21, 21, SWP_NOZORDER ) ;
}

void CAnimBar::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default

	int nFrame = m_nFrame ;

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
		nFrame = max(0,m_NumFrames-1) ;
		break ;
	case SB_THUMBPOSITION:
		nFrame = nPos ;
		break ;
	case SB_THUMBTRACK:
		nFrame = nPos ;
		break ;
	}

	UpdateOnScrollChange( nFrame );

//	CDialogBar::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CAnimBar::OnChangeFrame( )
{
	CEdit *pFrame = (CEdit*)GetDlgItem( IDC_FRAME ) ;
	s32 NewFrame;

	CString String;
	pFrame->GetWindowText( String );
	NewFrame = atoi( String );
	UpdateOnScrollChange( NewFrame );
}

void CAnimBar::UpdateOnScrollChange( s32 NewFrame )
{
	if( !m_pView || !m_pView->m_pCharacter )
		return;

	m_pView->SetAnimScrollFrame( NewFrame );
}

BOOL CAnimBar::Create( CWnd* pParentWnd, UINT nIDTemplate, UINT nStyle, UINT nID )
{
	BOOL Success = CDialogBar::Create( pParentWnd, nIDTemplate, nStyle, nID ) ;

	if( Success )
	{
		m_bbFrameStart.AutoLoad( IDC_FRAME_START, this ) ;
		m_bbFrameEnd.AutoLoad( IDC_FRAME_END, this ) ;
	}

	return Success;
}
