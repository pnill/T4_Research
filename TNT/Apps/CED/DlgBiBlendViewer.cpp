// DlgBiBlendViewer.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "DlgBiBlendViewer.h"
#include "MotionEditView.h"
#include "Motion.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	SLIDER_RANGE	10000

/////////////////////////////////////////////////////////////////////////////
// DlgBiBlendViewer dialog


DlgBiBlendViewer::DlgBiBlendViewer(CWnd* pParent /*=NULL*/)
	: CDialog(DlgBiBlendViewer::IDD, pParent)
{
	m_pView			= NULL;
	m_bChangeRatio	= FALSE;
	m_Motion0		= "";
	m_Motion1		= "";
	m_Ratio			= 0.5f;

	//{{AFX_DATA_INIT(DlgBiBlendViewer)
	//}}AFX_DATA_INIT
}


void DlgBiBlendViewer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgBiBlendViewer)
	DDX_Control(pDX, IDC_SLIDER1, m_Slider);
	DDX_Control(pDX, IDC_ANIM1PERCENT, m_Anim1Percent);
	DDX_Control(pDX, IDC_ANIM0PERCENT, m_Anim0Percent);
	DDX_Control(pDX, IDC_ANIM1, m_Anim1);
	DDX_Control(pDX, IDC_ANIM0, m_Anim0);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DlgBiBlendViewer, CDialog)
	//{{AFX_MSG_MAP(DlgBiBlendViewer)
	ON_CBN_EDITCHANGE(IDC_ANIM0, OnEditchangeAnim0)
	ON_CBN_EDITCHANGE(IDC_ANIM1, OnEditchangeAnim1)
	ON_EN_CHANGE(IDC_ANIM0PERCENT, OnChangeAnim0percent)
	ON_EN_CHANGE(IDC_ANIM1PERCENT, OnChangeAnim1percent)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER1, OnReleasedcaptureSlider)
	ON_EN_KILLFOCUS(IDC_ANIM0PERCENT, OnKillfocusAnim0percent)
	ON_EN_KILLFOCUS(IDC_ANIM1PERCENT, OnKillfocusAnim1percent)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSlider1)
	ON_CBN_SELCHANGE(IDC_ANIM1, OnSelchangeAnim1)
	ON_CBN_SELCHANGE(IDC_ANIM0, OnSelchangeAnim0)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DlgBiBlendViewer message handlers

void DlgBiBlendViewer::OnOK() 
{
	m_pView->ToggleBiBlendViewer();
}

BOOL DlgBiBlendViewer::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_Slider.SetRange( 0, SLIDER_RANGE );
	for( int i=0; i<11; i++ )
		m_Slider.SetTic( i*SLIDER_RANGE/11 );
	SetAnimRatio( m_Ratio, -1 );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void DlgBiBlendViewer::Reset()
{
	POSITION	pos;
	CMotion*	pMotion;
	s32			i;

	if( !m_pView )					return;
	if( !m_pView->m_pCharacter )	return;

	//---	fill the drop lists with the motions
	m_Anim0.ResetContent();
	m_Anim1.ResetContent();
	pos = m_pView->m_pCharacter->m_MotionList.GetHeadPosition();
	while( pos )
	{
		pMotion = m_pView->m_pCharacter->m_MotionList.GetNext( pos );
		m_Anim0.AddString( pMotion->GetExportName() );
		m_Anim1.AddString( pMotion->GetExportName() );
	}

	//---	find the previously selected motion
	m_pMotion0	= NULL;
	m_pMotion1	= NULL;
	pos = m_pView->m_pCharacter->m_MotionList.GetHeadPosition();
	i = 0;
	while( pos )
	{
		pMotion = m_pView->m_pCharacter->m_MotionList.GetNext( pos );

		if( (m_pMotion0==NULL) && (pMotion->GetExportName()==m_Motion0) )
		{
			m_pMotion0 = pMotion;
			m_Anim0.SetCurSel( i );
		}

		if( (m_pMotion1==NULL) && (pMotion->GetExportName()==m_Motion1) )
		{
			m_pMotion1 = pMotion;
			m_Anim1.SetCurSel( i );
		}

		i++;
	}

	//---	set the characters into the view
	m_pView->m_pCharacter->SetCurMotion( m_pMotion0 );
	m_pView->m_pCharacter->SetSecondMotion( m_pMotion1, m_Ratio );
	m_pView->Invalidate();
}

void DlgBiBlendViewer::OnEditchangeAnim0() 
{
}

void DlgBiBlendViewer::OnEditchangeAnim1() 
{
}

void DlgBiBlendViewer::OnChangeAnim0percent() 
{
	if( !GetSafeHwnd() )			return;
	if( !m_pView )					return;
	if( !m_pView->m_pCharacter )	return;

	CString str;
	m_Anim0Percent.GetWindowText( str );
	f32 value = (f32)atof( str.GetBuffer( 0 ) );
	str.ReleaseBuffer();

	SetAnimRatio( value, 0 );
}

void DlgBiBlendViewer::OnChangeAnim1percent() 
{
	if( !GetSafeHwnd() )			return;
	if( !m_pView )					return;
	if( !m_pView->m_pCharacter )	return;

	CString str;
	m_Anim1Percent.GetWindowText( str );
	f32 value = (f32)atof( str.GetBuffer( 0 ) );
	str.ReleaseBuffer();

	SetAnimRatio( value, 1 );
}

void DlgBiBlendViewer::OnReleasedcaptureSlider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if( !GetSafeHwnd() )			return;
	if( !m_pView )					return;
	if( !m_pView->m_pCharacter )	return;
	
	*pResult = 0;
}

void DlgBiBlendViewer::SetAnimRatio( f32 Ratio, s32 WhichAnim )
{
	if( !GetSafeHwnd() )			return;
	if( !m_pView )					return;
	if( !m_pView->m_pCharacter )	return;

	if( m_bChangeRatio )
		return;
	m_bChangeRatio = TRUE;

	if( Ratio > 1.0f )
		Ratio = 1.0f;
	else if( Ratio < 0.0f )
		Ratio = 0.0f;

	if( WhichAnim == 1 )
		m_Ratio = 1.0f - Ratio;
	else
		m_Ratio = Ratio;

	CString str;
	if( WhichAnim != 0 )
	{
		str.Format( "%f", m_Ratio );
		m_Anim0Percent.SetWindowText( str );
	}

	if( WhichAnim != 1 )
	{
		str.Format( "%f", 1.0f - m_Ratio );
		m_Anim1Percent.SetWindowText( str );
	}

	m_Slider.SetPos( (int)(SLIDER_RANGE*(1.0f-m_Ratio)) );

	m_pView->m_pCharacter->SetSecondMotion( m_pMotion1, 1.0f-m_Ratio );
	m_pView->Invalidate();

	m_bChangeRatio = FALSE;

}

void DlgBiBlendViewer::OnKillfocusAnim0percent() 
{
	SetAnimRatio( m_Ratio, -1 );	
}

void DlgBiBlendViewer::OnKillfocusAnim1percent() 
{
	SetAnimRatio( m_Ratio, -1 );	
}

void DlgBiBlendViewer::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	s32 Pos = m_Slider.GetPos();

	f32 Ratio = 1.0f - ((f32)Pos/(f32)SLIDER_RANGE);

	if( Ratio != m_Ratio )
		SetAnimRatio( Ratio, -1 );
	
	*pResult = 0;
}

void DlgBiBlendViewer::OnSelchangeAnim0() 
{
	if( !GetSafeHwnd() )			return;
	if( !m_pView )					return;
	if( !m_pView->m_pCharacter )	return;

	s32 i = m_Anim0.GetCurSel();
	if( i != -1 )
		m_Anim0.GetLBText( i, m_Motion0 );

	m_pMotion0 = m_pView->m_pCharacter->FindMotionByExportName( m_Motion0 );
	m_pView->m_pCharacter->SetCurMotion( m_pMotion0 );
	m_pView->Invalidate();
}

void DlgBiBlendViewer::OnSelchangeAnim1() 
{
	if( !GetSafeHwnd() )			return;
	if( !m_pView )					return;
	if( !m_pView->m_pCharacter )	return;

	s32 i = m_Anim1.GetCurSel();
	if( i != -1 )
		m_Anim1.GetLBText( i, m_Motion1 );

	m_pMotion1 = m_pView->m_pCharacter->FindMotionByExportName( m_Motion1 );
	m_pView->m_pCharacter->SetSecondMotion( m_pMotion1, 1.0f-m_Ratio );
	m_pView->Invalidate();
}
