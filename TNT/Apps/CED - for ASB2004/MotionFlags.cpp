// MotionFlags.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "MotionFlags.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMotionFlags dialog


CMotionFlags::CMotionFlags(CWnd* pParent /*=NULL*/)
	: CDialog(CMotionFlags::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMotionFlags)
	m_Def0 = _T("");
	m_Def1 = _T("");
	m_Def10 = _T("");
	m_Def11 = _T("");
	m_Def12 = _T("");
	m_Def13 = _T("");
	m_Def14 = _T("");
	m_Def15 = _T("");
	m_Def2 = _T("");
	m_Def3 = _T("");
	m_Def4 = _T("");
	m_Def5 = _T("");
	m_Def6 = _T("");
	m_Def7 = _T("");
	m_Def8 = _T("");
	m_Def9 = _T("");
	m_Txt0 = _T("");
	m_Txt1 = _T("");
	m_Txt10 = _T("");
	m_Txt11 = _T("");
	m_Txt12 = _T("");
	m_Txt13 = _T("");
	m_Txt14 = _T("");
	m_Txt15 = _T("");
	m_Txt2 = _T("");
	m_Txt3 = _T("");
	m_Txt4 = _T("");
	m_Txt5 = _T("");
	m_Txt6 = _T("");
	m_Txt7 = _T("");
	m_Txt8 = _T("");
	m_Txt9 = _T("");
	//}}AFX_DATA_INIT

	for( int i = 0 ; i < MOTION_FLAG_BITS ; i++ )
	{
		m_Bit[i] = -1 ;
	}
}


void CMotionFlags::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMotionFlags)
	DDX_Control(pDX, IDC_BIT9, m_CBit9);
	DDX_Control(pDX, IDC_BIT8, m_CBit8);
	DDX_Control(pDX, IDC_BIT7, m_CBit7);
	DDX_Control(pDX, IDC_BIT6, m_CBit6);
	DDX_Control(pDX, IDC_BIT5, m_CBit5);
	DDX_Control(pDX, IDC_BIT4, m_CBit4);
	DDX_Control(pDX, IDC_BIT3, m_CBit3);
	DDX_Control(pDX, IDC_BIT2, m_CBit2);
	DDX_Control(pDX, IDC_BIT15, m_CBit15);
	DDX_Control(pDX, IDC_BIT14, m_CBit14);
	DDX_Control(pDX, IDC_BIT13, m_CBit13);
	DDX_Control(pDX, IDC_BIT12, m_CBit12);
	DDX_Control(pDX, IDC_BIT11, m_CBit11);
	DDX_Control(pDX, IDC_BIT10, m_CBit10);
	DDX_Control(pDX, IDC_BIT1, m_CBit1);
	DDX_Control(pDX, IDC_BIT0, m_CBit0);
	DDX_Text(pDX, IDC_DEFINE0, m_Def0);
	DDX_Text(pDX, IDC_DEFINE1, m_Def1);
	DDX_Text(pDX, IDC_DEFINE10, m_Def10);
	DDX_Text(pDX, IDC_DEFINE11, m_Def11);
	DDX_Text(pDX, IDC_DEFINE12, m_Def12);
	DDX_Text(pDX, IDC_DEFINE13, m_Def13);
	DDX_Text(pDX, IDC_DEFINE14, m_Def14);
	DDX_Text(pDX, IDC_DEFINE15, m_Def15);
	DDX_Text(pDX, IDC_DEFINE2, m_Def2);
	DDX_Text(pDX, IDC_DEFINE3, m_Def3);
	DDX_Text(pDX, IDC_DEFINE4, m_Def4);
	DDX_Text(pDX, IDC_DEFINE5, m_Def5);
	DDX_Text(pDX, IDC_DEFINE6, m_Def6);
	DDX_Text(pDX, IDC_DEFINE7, m_Def7);
	DDX_Text(pDX, IDC_DEFINE8, m_Def8);
	DDX_Text(pDX, IDC_DEFINE9, m_Def9);
	DDX_Text(pDX, IDC_DESC0, m_Txt0);
	DDX_Text(pDX, IDC_DESC1, m_Txt1);
	DDX_Text(pDX, IDC_DESC10, m_Txt10);
	DDX_Text(pDX, IDC_DESC11, m_Txt11);
	DDX_Text(pDX, IDC_DESC12, m_Txt12);
	DDX_Text(pDX, IDC_DESC13, m_Txt13);
	DDX_Text(pDX, IDC_DESC14, m_Txt14);
	DDX_Text(pDX, IDC_DESC15, m_Txt15);
	DDX_Text(pDX, IDC_DESC2, m_Txt2);
	DDX_Text(pDX, IDC_DESC3, m_Txt3);
	DDX_Text(pDX, IDC_DESC4, m_Txt4);
	DDX_Text(pDX, IDC_DESC5, m_Txt5);
	DDX_Text(pDX, IDC_DESC6, m_Txt6);
	DDX_Text(pDX, IDC_DESC7, m_Txt7);
	DDX_Text(pDX, IDC_DESC8, m_Txt8);
	DDX_Text(pDX, IDC_DESC9, m_Txt9);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMotionFlags, CDialog)
	//{{AFX_MSG_MAP(CMotionFlags)
	ON_BN_CLICKED(IDC_BIT0, OnBit0)
	ON_BN_CLICKED(IDC_BIT1, OnBit1)
	ON_BN_CLICKED(IDC_BIT2, OnBit2)
	ON_BN_CLICKED(IDC_BIT3, OnBit3)
	ON_BN_CLICKED(IDC_BIT4, OnBit4)
	ON_BN_CLICKED(IDC_BIT5, OnBit5)
	ON_BN_CLICKED(IDC_BIT6, OnBit6)
	ON_BN_CLICKED(IDC_BIT7, OnBit7)
	ON_BN_CLICKED(IDC_BIT8, OnBit8)
	ON_BN_CLICKED(IDC_BIT9, OnBit9)
	ON_BN_CLICKED(IDC_BIT10, OnBit10)
	ON_BN_CLICKED(IDC_BIT11, OnBit11)
	ON_BN_CLICKED(IDC_BIT12, OnBit12)
	ON_BN_CLICKED(IDC_BIT13, OnBit13)
	ON_BN_CLICKED(IDC_BIT14, OnBit14)
	ON_BN_CLICKED(IDC_BIT15, OnBit15)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMotionFlags message handlers

BOOL CMotionFlags::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	m_CBit0.SetCheck( m_Bit[0] ) ;
	m_CBit1.SetCheck( m_Bit[1] ) ;
	m_CBit2.SetCheck( m_Bit[2] ) ;
	m_CBit3.SetCheck( m_Bit[3] ) ;
	m_CBit4.SetCheck( m_Bit[4] ) ;
	m_CBit5.SetCheck( m_Bit[5] ) ;
	m_CBit6.SetCheck( m_Bit[6] ) ;
	m_CBit7.SetCheck( m_Bit[7] ) ;
	m_CBit8.SetCheck( m_Bit[8] ) ;
	m_CBit9.SetCheck( m_Bit[9] ) ;
	m_CBit10.SetCheck( m_Bit[10] ) ;
	m_CBit11.SetCheck( m_Bit[11] ) ;
	m_CBit12.SetCheck( m_Bit[12] ) ;
	m_CBit13.SetCheck( m_Bit[13] ) ;
	m_CBit14.SetCheck( m_Bit[14] ) ;
	m_CBit15.SetCheck( m_Bit[15] ) ;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMotionFlags::OnOK() 
{
	// TODO: Add extra validation here
	
	m_Bit[0]  = m_CBit0.GetCheck( ) ;
	m_Bit[1]  = m_CBit1.GetCheck( ) ;
	m_Bit[2]  = m_CBit2.GetCheck( ) ;
	m_Bit[3]  = m_CBit3.GetCheck( ) ;
	m_Bit[4]  = m_CBit4.GetCheck( ) ;
	m_Bit[5]  = m_CBit5.GetCheck( ) ;
	m_Bit[6]  = m_CBit6.GetCheck( ) ;
	m_Bit[7]  = m_CBit7.GetCheck( ) ;
	m_Bit[8]  = m_CBit8.GetCheck( ) ;
	m_Bit[9]  = m_CBit9.GetCheck( ) ;
	m_Bit[10] = m_CBit10.GetCheck( ) ;
	m_Bit[11] = m_CBit11.GetCheck( ) ;
	m_Bit[12] = m_CBit12.GetCheck( ) ;
	m_Bit[13] = m_CBit13.GetCheck( ) ;
	m_Bit[14] = m_CBit14.GetCheck( ) ;
	m_Bit[15] = m_CBit15.GetCheck( ) ;

	CDialog::OnOK();
}

void CMotionFlags::OnBit0()
{
	m_CBit0.SetButtonStyle( BS_AUTOCHECKBOX | BS_LEFTTEXT ) ;
}
void CMotionFlags::OnBit1()
{
	m_CBit1.SetButtonStyle( BS_AUTOCHECKBOX | BS_LEFTTEXT ) ;
}
void CMotionFlags::OnBit2()
{
	m_CBit2.SetButtonStyle( BS_AUTOCHECKBOX | BS_LEFTTEXT ) ;
}
void CMotionFlags::OnBit3()
{
	m_CBit3.SetButtonStyle( BS_AUTOCHECKBOX | BS_LEFTTEXT ) ;
}
void CMotionFlags::OnBit4()
{
	m_CBit4.SetButtonStyle( BS_AUTOCHECKBOX | BS_LEFTTEXT ) ;
}
void CMotionFlags::OnBit5()
{
	m_CBit5.SetButtonStyle( BS_AUTOCHECKBOX | BS_LEFTTEXT ) ;
}
void CMotionFlags::OnBit6()
{
	m_CBit6.SetButtonStyle( BS_AUTOCHECKBOX | BS_LEFTTEXT ) ;
}
void CMotionFlags::OnBit7()
{
	m_CBit7.SetButtonStyle( BS_AUTOCHECKBOX | BS_LEFTTEXT ) ;
}
void CMotionFlags::OnBit8()
{
	m_CBit8.SetButtonStyle( BS_AUTOCHECKBOX | BS_LEFTTEXT ) ;
}
void CMotionFlags::OnBit9()
{
	m_CBit9.SetButtonStyle( BS_AUTOCHECKBOX | BS_LEFTTEXT ) ;
}
void CMotionFlags::OnBit10()
{
	m_CBit10.SetButtonStyle( BS_AUTOCHECKBOX | BS_LEFTTEXT ) ;
}
void CMotionFlags::OnBit11()
{
	m_CBit11.SetButtonStyle( BS_AUTOCHECKBOX | BS_LEFTTEXT ) ;
}
void CMotionFlags::OnBit12()
{
	m_CBit12.SetButtonStyle( BS_AUTOCHECKBOX | BS_LEFTTEXT ) ;
}
void CMotionFlags::OnBit13()
{
	m_CBit13.SetButtonStyle( BS_AUTOCHECKBOX | BS_LEFTTEXT ) ;
}
void CMotionFlags::OnBit14()
{
	m_CBit14.SetButtonStyle( BS_AUTOCHECKBOX | BS_LEFTTEXT ) ;
}
void CMotionFlags::OnBit15()
{
	m_CBit15.SetButtonStyle( BS_AUTOCHECKBOX | BS_LEFTTEXT ) ;
}
