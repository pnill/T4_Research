// GCIconPropDlg.cpp : implementation file
//

#include "stdafx.h"
#include "memcardwiz.h"
#include "GCIconPropDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGCIconPropDlg dialog


CGCIconPropDlg::CGCIconPropDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGCIconPropDlg::IDD, pParent)
{

	m_IconInfo.Init();

	//{{AFX_DATA_INIT(CGCIconPropDlg)
	//}}AFX_DATA_INIT
}


void CGCIconPropDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGCIconPropDlg)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGCIconPropDlg, CDialog)
	//{{AFX_MSG_MAP(CGCIconPropDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGCIconPropDlg message handlers

BOOL CGCIconPropDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	

	((CButton*)GetDlgItem( IDC_RADIO_ICON_ANIMFAST ))->SetCheck( FALSE );
	((CButton*)GetDlgItem( IDC_RADIO_ICON_ANIMNORM ))->SetCheck( FALSE );
	((CButton*)GetDlgItem( IDC_RADIO_ICON_ANIMSLOW ))->SetCheck( FALSE );

	if( m_IconInfo.m_AnimSpeed == GC_ICON_ANIMSPEED_FAST )
	{
		((CButton*)GetDlgItem( IDC_RADIO_ICON_ANIMFAST ))->SetCheck( TRUE );
	}
	else if( m_IconInfo.m_AnimSpeed == GC_ICON_ANIMSPEED_NORMAL )
	{
		((CButton*)GetDlgItem( IDC_RADIO_ICON_ANIMNORM ))->SetCheck( TRUE );
	}
	else if( m_IconInfo.m_AnimSpeed == GC_ICON_ANIMSPEED_SLOW )
	{
		((CButton*)GetDlgItem( IDC_RADIO_ICON_ANIMSLOW ))->SetCheck( TRUE );
	}

	((CSpinButtonCtrl*)GetDlgItem( IDC_SPIN_ICONNFRAMES ))->SetRange32( GC_ICON_NFRAMES_MIN, GC_ICON_NFRAMES_MAX );
	((CSpinButtonCtrl*)GetDlgItem( IDC_SPIN_ICONNFRAMES ))->SetPos( m_IconInfo.m_NFrames );


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGCIconPropDlg::OnOK() 
{
	if( ((CButton*)GetDlgItem( IDC_RADIO_ICON_ANIMFAST ))->GetCheck() )
	{
		m_IconInfo.m_AnimSpeed = GC_ICON_ANIMSPEED_FAST;
	}
	else if( ((CButton*)GetDlgItem( IDC_RADIO_ICON_ANIMNORM ))->GetCheck() )
	{
		m_IconInfo.m_AnimSpeed = GC_ICON_ANIMSPEED_NORMAL;
	}
	else if( ((CButton*)GetDlgItem( IDC_RADIO_ICON_ANIMSLOW ))->GetCheck() )
	{
		m_IconInfo.m_AnimSpeed = GC_ICON_ANIMSPEED_SLOW;
	}

	m_IconInfo.m_NFrames = ((CSpinButtonCtrl*)GetDlgItem( IDC_SPIN_ICONNFRAMES ))->GetPos();


	CDialog::OnOK();
}

////////////////////////////////////////////////////////////////////////////

void CGCIconPropDlg::SetIconInfo( const MC_GCIconInfo& rIconInfo )
{
	m_IconInfo = rIconInfo;
}

//==========================================================================

void CGCIconPropDlg::GetIconInfo( MC_GCIconInfo& rIconInfo ) const
{
	rIconInfo = m_IconInfo;
}

//==========================================================================
