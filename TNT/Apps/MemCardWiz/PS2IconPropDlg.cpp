// PS2IconPropDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MemCardWiz.h"
#include "PS2IconPropDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPS2IconPropDlg dialog


CPS2IconPropDlg::CPS2IconPropDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPS2IconPropDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPS2IconPropDlg)
	m_AMB_B = 32;
	m_AMB_G = 32;
	m_AMB_R = 32;
	m_BK_Alpha = 128;
	m_BK_BL_B = 215;
	m_BK_BL_G = 215;
	m_BK_BL_R = 215;
	m_BK_BR_B = 215;
	m_BK_BR_G = 215;
	m_BK_BR_R = 215;
	m_BK_TL_B = 215;
	m_BK_TL_G = 215;
	m_BK_TL_R = 215;
	m_BK_TR_B = 215;
	m_BK_TR_G = 215;
	m_BK_TR_R = 215;
	m_L1_B = 255;
	m_L1_G = 255;
	m_L1_R = 255;
	m_L1_X = 1.0f;
	m_L1_Y = 0.0f;
	m_L1_Z = 0.0f;
	m_L2_B = 255;
	m_L2_G = 255;
	m_L2_R = 255;
	m_L2_X = 0.0f;
	m_L2_Y = 1.0f;
	m_L2_Z = 0.0f;
	m_L3_B = 255;
	m_L3_G = 255;
	m_L3_R = 255;
	m_L3_X = 0.0f;
	m_L3_Y = 0.0f;
	m_L3_Z = 1.0f;
	//}}AFX_DATA_INIT
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void CPS2IconPropDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPS2IconPropDlg)
	DDX_Text(pDX, IDC_EDIT_AMBIENT_B, m_AMB_B);
	DDV_MinMaxByte(pDX, m_AMB_B, 0, 255);
	DDX_Text(pDX, IDC_EDIT_AMBIENT_G, m_AMB_G);
	DDV_MinMaxByte(pDX, m_AMB_G, 0, 255);
	DDX_Text(pDX, IDC_EDIT_AMBIENT_R, m_AMB_R);
	DDV_MinMaxByte(pDX, m_AMB_R, 0, 255);
	DDX_Text(pDX, IDC_EDIT_BK_ALPHA, m_BK_Alpha);
	DDV_MinMaxByte(pDX, m_BK_Alpha, 0, 128);
	DDX_Text(pDX, IDC_EDIT_BK_BL_B, m_BK_BL_B);
	DDV_MinMaxByte(pDX, m_BK_BL_B, 0, 255);
	DDX_Text(pDX, IDC_EDIT_BK_BL_G, m_BK_BL_G);
	DDV_MinMaxByte(pDX, m_BK_BL_G, 0, 255);
	DDX_Text(pDX, IDC_EDIT_BK_BL_R, m_BK_BL_R);
	DDV_MinMaxByte(pDX, m_BK_BL_R, 0, 255);
	DDX_Text(pDX, IDC_EDIT_BK_BR_B, m_BK_BR_B);
	DDV_MinMaxByte(pDX, m_BK_BR_B, 0, 255);
	DDX_Text(pDX, IDC_EDIT_BK_BR_G, m_BK_BR_G);
	DDV_MinMaxByte(pDX, m_BK_BR_G, 0, 255);
	DDX_Text(pDX, IDC_EDIT_BK_BR_R, m_BK_BR_R);
	DDV_MinMaxByte(pDX, m_BK_BR_R, 0, 255);
	DDX_Text(pDX, IDC_EDIT_BK_TL_B, m_BK_TL_B);
	DDV_MinMaxByte(pDX, m_BK_TL_B, 0, 255);
	DDX_Text(pDX, IDC_EDIT_BK_TL_G, m_BK_TL_G);
	DDV_MinMaxByte(pDX, m_BK_TL_G, 0, 255);
	DDX_Text(pDX, IDC_EDIT_BK_TL_R, m_BK_TL_R);
	DDV_MinMaxByte(pDX, m_BK_TL_R, 0, 255);
	DDX_Text(pDX, IDC_EDIT_BK_TR_B, m_BK_TR_B);
	DDV_MinMaxByte(pDX, m_BK_TR_B, 0, 255);
	DDX_Text(pDX, IDC_EDIT_BK_TR_G, m_BK_TR_G);
	DDV_MinMaxByte(pDX, m_BK_TR_G, 0, 255);
	DDX_Text(pDX, IDC_EDIT_BK_TR_R, m_BK_TR_R);
	DDV_MinMaxByte(pDX, m_BK_TR_R, 0, 255);
	DDX_Text(pDX, IDC_EDIT_L1_B, m_L1_B);
	DDV_MinMaxByte(pDX, m_L1_B, 0, 255);
	DDX_Text(pDX, IDC_EDIT_L1_G, m_L1_G);
	DDV_MinMaxByte(pDX, m_L1_G, 0, 255);
	DDX_Text(pDX, IDC_EDIT_L1_R, m_L1_R);
	DDV_MinMaxByte(pDX, m_L1_R, 0, 255);
	DDX_Text(pDX, IDC_EDIT_L1_X, m_L1_X);
	DDV_MinMaxFloat(pDX, m_L1_X, -1.f, 1.f);
	DDX_Text(pDX, IDC_EDIT_L1_Y, m_L1_Y);
	DDV_MinMaxFloat(pDX, m_L1_Y, -1.f, 1.f);
	DDX_Text(pDX, IDC_EDIT_L1_Z, m_L1_Z);
	DDV_MinMaxFloat(pDX, m_L1_Z, -1.f, 1.f);
	DDX_Text(pDX, IDC_EDIT_L2_B, m_L2_B);
	DDV_MinMaxByte(pDX, m_L2_B, 0, 255);
	DDX_Text(pDX, IDC_EDIT_L2_G, m_L2_G);
	DDV_MinMaxByte(pDX, m_L2_G, 0, 255);
	DDX_Text(pDX, IDC_EDIT_L2_R, m_L2_R);
	DDV_MinMaxByte(pDX, m_L2_R, 0, 255);
	DDX_Text(pDX, IDC_EDIT_L2_X, m_L2_X);
	DDV_MinMaxFloat(pDX, m_L2_X, -1.f, 1.f);
	DDX_Text(pDX, IDC_EDIT_L2_Y, m_L2_Y);
	DDV_MinMaxFloat(pDX, m_L2_Y, -1.f, 1.f);
	DDX_Text(pDX, IDC_EDIT_L2_Z, m_L2_Z);
	DDV_MinMaxFloat(pDX, m_L2_Z, -1.f, 1.f);
	DDX_Text(pDX, IDC_EDIT_L3_B, m_L3_B);
	DDV_MinMaxByte(pDX, m_L3_B, 0, 255);
	DDX_Text(pDX, IDC_EDIT_L3_G, m_L3_G);
	DDV_MinMaxByte(pDX, m_L3_G, 0, 255);
	DDX_Text(pDX, IDC_EDIT_L3_R, m_L3_R);
	DDV_MinMaxByte(pDX, m_L3_R, 0, 255);
	DDX_Text(pDX, IDC_EDIT_L3_X, m_L3_X);
	DDV_MinMaxFloat(pDX, m_L3_X, -1.f, 1.f);
	DDX_Text(pDX, IDC_EDIT_L3_Y, m_L3_Y);
	DDV_MinMaxFloat(pDX, m_L3_Y, -1.f, 1.f);
	DDX_Text(pDX, IDC_EDIT_L3_Z, m_L3_Z);
	DDV_MinMaxFloat(pDX, m_L3_Z, -1.f, 1.f);
	//}}AFX_DATA_MAP
}

////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CPS2IconPropDlg, CDialog)
	//{{AFX_MSG_MAP(CPS2IconPropDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPS2IconPropDlg message handlers

BOOL CPS2IconPropDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CString WT;

	((CEdit*)GetDlgItem( IDC_EDIT_L1_X ))->SetLimitText( 5 );
	((CEdit*)GetDlgItem( IDC_EDIT_L1_Y ))->SetLimitText( 5 );
	((CEdit*)GetDlgItem( IDC_EDIT_L1_Z ))->SetLimitText( 5 );
	((CEdit*)GetDlgItem( IDC_EDIT_L2_X ))->SetLimitText( 5 );
	((CEdit*)GetDlgItem( IDC_EDIT_L2_Y ))->SetLimitText( 5 );
	((CEdit*)GetDlgItem( IDC_EDIT_L2_Z ))->SetLimitText( 5 );
	((CEdit*)GetDlgItem( IDC_EDIT_L3_X ))->SetLimitText( 5 );
	((CEdit*)GetDlgItem( IDC_EDIT_L3_Y ))->SetLimitText( 5 );
	((CEdit*)GetDlgItem( IDC_EDIT_L3_Z ))->SetLimitText( 5 );

	((CEdit*)GetDlgItem( IDC_EDIT_L1_R ))->SetLimitText( 3 );
	((CEdit*)GetDlgItem( IDC_EDIT_L1_G ))->SetLimitText( 3 );
	((CEdit*)GetDlgItem( IDC_EDIT_L1_B ))->SetLimitText( 3 );
	((CEdit*)GetDlgItem( IDC_EDIT_L2_R ))->SetLimitText( 3 );
	((CEdit*)GetDlgItem( IDC_EDIT_L2_G ))->SetLimitText( 3 );
	((CEdit*)GetDlgItem( IDC_EDIT_L2_B ))->SetLimitText( 3 );
	((CEdit*)GetDlgItem( IDC_EDIT_L3_R ))->SetLimitText( 3 );
	((CEdit*)GetDlgItem( IDC_EDIT_L3_G ))->SetLimitText( 3 );
	((CEdit*)GetDlgItem( IDC_EDIT_L3_B ))->SetLimitText( 3 );

	((CEdit*)GetDlgItem( IDC_EDIT_AMBIENT_R ))->SetLimitText( 3 );
	((CEdit*)GetDlgItem( IDC_EDIT_AMBIENT_G ))->SetLimitText( 3 );
	((CEdit*)GetDlgItem( IDC_EDIT_AMBIENT_B ))->SetLimitText( 3 );

	((CEdit*)GetDlgItem( IDC_EDIT_BK_ALPHA ))->SetLimitText( 3 );

	((CEdit*)GetDlgItem( IDC_EDIT_BK_TL_R ))->SetLimitText( 3 );
	((CEdit*)GetDlgItem( IDC_EDIT_BK_TL_G ))->SetLimitText( 3 );
	((CEdit*)GetDlgItem( IDC_EDIT_BK_TL_B ))->SetLimitText( 3 );
	((CEdit*)GetDlgItem( IDC_EDIT_BK_TR_R ))->SetLimitText( 3 );
	((CEdit*)GetDlgItem( IDC_EDIT_BK_TR_G ))->SetLimitText( 3 );
	((CEdit*)GetDlgItem( IDC_EDIT_BK_TR_B ))->SetLimitText( 3 );
	((CEdit*)GetDlgItem( IDC_EDIT_BK_BL_R ))->SetLimitText( 3 );
	((CEdit*)GetDlgItem( IDC_EDIT_BK_BL_G ))->SetLimitText( 3 );
	((CEdit*)GetDlgItem( IDC_EDIT_BK_BL_B ))->SetLimitText( 3 );
	((CEdit*)GetDlgItem( IDC_EDIT_BK_BR_R ))->SetLimitText( 3 );
	((CEdit*)GetDlgItem( IDC_EDIT_BK_BR_G ))->SetLimitText( 3 );
	((CEdit*)GetDlgItem( IDC_EDIT_BK_BR_B ))->SetLimitText( 3 );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//==========================================================================

void CPS2IconPropDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}

//==========================================================================

void CPS2IconPropDlg::SetIconInfo( const MC_PS2IconInfo& rIconInfo )
{
	m_AMB_B    = rIconInfo.m_AMB_B;
	m_AMB_G    = rIconInfo.m_AMB_G;
	m_AMB_R    = rIconInfo.m_AMB_R;
	m_BK_Alpha = (BYTE)rIconInfo.m_BK_Alpha;
	m_BK_BL_B  = rIconInfo.m_BK_BL_B;
	m_BK_BL_G  = rIconInfo.m_BK_BL_G;
	m_BK_BL_R  = rIconInfo.m_BK_BL_R;
	m_BK_BR_B  = rIconInfo.m_BK_BR_B;
	m_BK_BR_G  = rIconInfo.m_BK_BR_G;
	m_BK_BR_R  = rIconInfo.m_BK_BR_R;
	m_BK_TL_B  = rIconInfo.m_BK_TL_B;
	m_BK_TL_G  = rIconInfo.m_BK_TL_G;
	m_BK_TL_R  = rIconInfo.m_BK_TL_R;
	m_BK_TR_B  = rIconInfo.m_BK_TR_B;
	m_BK_TR_G  = rIconInfo.m_BK_TR_G;
	m_BK_TR_R  = rIconInfo.m_BK_TR_R;
	m_L1_B     = rIconInfo.m_L1_B;
	m_L1_G     = rIconInfo.m_L1_G;
	m_L1_R     = rIconInfo.m_L1_R;
	m_L1_X     = rIconInfo.m_L1_X;
	m_L1_Y     = rIconInfo.m_L1_Y;
	m_L1_Z     = rIconInfo.m_L1_Z;
	m_L2_B     = rIconInfo.m_L2_B;
	m_L2_G     = rIconInfo.m_L2_G;
	m_L2_R     = rIconInfo.m_L2_R;
	m_L2_X     = rIconInfo.m_L2_X;
	m_L2_Y     = rIconInfo.m_L2_Y;
	m_L2_Z     = rIconInfo.m_L2_Z;
	m_L3_B     = rIconInfo.m_L3_B;
	m_L3_G     = rIconInfo.m_L3_G;
	m_L3_R     = rIconInfo.m_L3_R;
	m_L3_X     = rIconInfo.m_L3_X;
	m_L3_Y     = rIconInfo.m_L3_Y;
	m_L3_Z     = rIconInfo.m_L3_Z;
}

//==========================================================================

void CPS2IconPropDlg::GetIconInfo( MC_PS2IconInfo& rIconInfo ) const
{
	rIconInfo.m_AMB_B    = m_AMB_B;
	rIconInfo.m_AMB_G    = m_AMB_G;
	rIconInfo.m_AMB_R    = m_AMB_R;
	rIconInfo.m_BK_Alpha = m_BK_Alpha;
	rIconInfo.m_BK_BL_B  = m_BK_BL_B;
	rIconInfo.m_BK_BL_G  = m_BK_BL_G;
	rIconInfo.m_BK_BL_R  = m_BK_BL_R;
	rIconInfo.m_BK_BR_B  = m_BK_BR_B;
	rIconInfo.m_BK_BR_G  = m_BK_BR_G;
	rIconInfo.m_BK_BR_R  = m_BK_BR_R;
	rIconInfo.m_BK_TL_B  = m_BK_TL_B;
	rIconInfo.m_BK_TL_G  = m_BK_TL_G;
	rIconInfo.m_BK_TL_R  = m_BK_TL_R;
	rIconInfo.m_BK_TR_B  = m_BK_TR_B;
	rIconInfo.m_BK_TR_G  = m_BK_TR_G;
	rIconInfo.m_BK_TR_R  = m_BK_TR_R;
	rIconInfo.m_L1_B     = m_L1_B;
	rIconInfo.m_L1_G     = m_L1_G;
	rIconInfo.m_L1_R     = m_L1_R;
	rIconInfo.m_L1_X     = m_L1_X;
	rIconInfo.m_L1_Y     = m_L1_Y;
	rIconInfo.m_L1_Z     = m_L1_Z;
	rIconInfo.m_L2_B     = m_L2_B;
	rIconInfo.m_L2_G     = m_L2_G;
	rIconInfo.m_L2_R     = m_L2_R;
	rIconInfo.m_L2_X     = m_L2_X;
	rIconInfo.m_L2_Y     = m_L2_Y;
	rIconInfo.m_L2_Z     = m_L2_Z;
	rIconInfo.m_L3_B     = m_L3_B;
	rIconInfo.m_L3_G     = m_L3_G;
	rIconInfo.m_L3_R     = m_L3_R;
	rIconInfo.m_L3_X     = m_L3_X;
	rIconInfo.m_L3_Y     = m_L3_Y;
	rIconInfo.m_L3_Z     = m_L3_Z;
}

//==========================================================================
