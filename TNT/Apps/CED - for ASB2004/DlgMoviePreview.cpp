// DlgMoviePreview.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "DlgMoviePreview.h"

#include "MovieView.h"
#include "Movie.h"
#include "mfcutils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgMoviePreview dialog


CDlgMoviePreview::CDlgMoviePreview(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgMoviePreview::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgMoviePreview)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgMoviePreview::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgMoviePreview)
	DDX_Control(pDX, IDC_PREVIEW_H, m_CtrlPreviewH);
	DDX_Control(pDX, IDC_PREVIEW_W, m_CtrlPreviewW);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgMoviePreview, CDialog)
	//{{AFX_MSG_MAP(CDlgMoviePreview)
	ON_BN_CLICKED(IDC_PLAY, OnPlay)
	ON_EN_CHANGE(IDC_PREVIEW_W, OnChangePreviewW)
	ON_EN_CHANGE(IDC_PREVIEW_H, OnChangePreviewH)
	//}}AFX_MSG_MAP

    ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDCANCEL, OnCancel)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Operaions

/////////////////////////////////////////////////////////////////////////////
// CDlgMoviePreview message handlers

void CDlgMoviePreview::OnOk()
{
}

void CDlgMoviePreview::OnCancel()
{
}


void CDlgMoviePreview::OnPlay() 
{
	// TODO: Add your control notification handler code here
    CMovie *pMovie = m_pView->m_pMovie ;
    if( pMovie )
    {
        m_pView->m_StartTiming = true ;
        m_pView->m_Playing = !m_pView->m_Playing ;
        m_pView->m_nTimerMessages = 0 ;
    }
}

BOOL CDlgMoviePreview::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
    if( m_pView )
    {
        CMovie *pMovie = m_pView->m_pMovie ;
        if( pMovie )
        {
            MFCU_UpdateEditControl( m_CtrlPreviewW, (s32)pMovie->GetPreviewWidth() ) ;
            MFCU_UpdateEditControl( m_CtrlPreviewH, (s32)pMovie->GetPreviewHeight() ) ;
        }
    }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgMoviePreview::OnChangePreviewW() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here

    if( m_pView && m_pView->m_pMovie )
    {
        s32 w = m_pView->m_pMovie->GetPreviewWidth() ;
        MFCU_UpdateFromEditControl( m_CtrlPreviewW, w ) ;
        m_pView->m_pMovie->SetPreviewWidth( w ) ;
        m_pView->Invalidate() ;
    }
}

void CDlgMoviePreview::OnChangePreviewH() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
    if( m_pView && m_pView->m_pMovie )
    {
        s32 h = m_pView->m_pMovie->GetPreviewHeight() ;
        MFCU_UpdateFromEditControl( m_CtrlPreviewH, h ) ;
        m_pView->m_pMovie->SetPreviewHeight( h ) ;
        m_pView->Invalidate() ;
    }
}
