// DlgMotion.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "DlgMotion.h"
#include "Character.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define	BLEND_PAGE_LENGTH	4

static CEdit*			s_BlendWeightArray[BLEND_PAGE_LENGTH] = {NULL};
static CComboBox*		s_BlendAnimArray[BLEND_PAGE_LENGTH] = {NULL};
static CSpinButtonCtrl*	s_BlendSpinnerArray[BLEND_PAGE_LENGTH] = {NULL};

/////////////////////////////////////////////////////////////////////////////
// CDlgMotion dialog

CDlgMotion::CDlgMotion(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgMotion::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgMotion)
	//}}AFX_DATA_INIT

	m_UpdatingBlendDialog	= FALSE;
	m_bIsMotionBlended		= FALSE;
}


void CDlgMotion::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgMotion)
	DDX_Control(pDX, IDC_FILEPATHNAME, m_SourceFile);
	DDX_Control(pDX, IDC_BROWSE, m_Browse);
	DDX_Control(pDX, IDC_BLENDED, m_Blended);
	DDX_Control(pDX, IDC_EXPORT_BITS_SET, m_ExportBitsButton);
	DDX_Control(pDX, IDC_EXPORT_BITS, m_ExportBits);
	DDX_Control(pDX, IDC_EXPORT_FRAMERATE, m_ExportFrameRate);
	DDX_Control(pDX, IDC_EXPORT_FRAMERATE_SET, m_ExportFrameRateButton);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgMotion, CDialog)
	//{{AFX_MSG_MAP(CDlgMotion)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_EXPORT, OnExport)
	ON_BN_CLICKED(IDC_EXPORT_FRAMERATE_SET, OnExportFramerateSet)
	ON_EN_CHANGE(IDC_EXPORT_FRAMERATE, OnChangeExportFramerate)
	ON_EN_KILLFOCUS(IDC_EXPORT_FRAMERATE, OnKillfocusExportFramerate)
	ON_BN_CLICKED(IDC_EXPORT_BITS_SET, OnExportBitsSet)
	ON_EN_CHANGE(IDC_EXPORT_BITS, OnChangeExportBits)
	ON_EN_KILLFOCUS(IDC_EXPORT_BITS, OnKillfocusExportBits)
	ON_BN_CLICKED(IDC_QUATERNION, OnQuaternion)
	ON_BN_CLICKED(IDC_WORLDROTS, OnWorldrots)
	ON_BN_CLICKED(IDC_QUATBLENDING, OnQuatblending)
	ON_BN_CLICKED(IDC_EXPORTMETHOD_SET, OnExportmethodSet)
	ON_BN_CLICKED(IDC_QUATW_ON, OnQuatbitsOn)
	ON_BN_CLICKED(IDC_GLOBALROTS, OnGlobalrots)
	ON_EN_CHANGE(IDC_BLENDWEIGHT0, OnChangeBlendweight0)
	ON_EN_CHANGE(IDC_BLENDWEIGHT1, OnChangeBlendweight1)
	ON_EN_CHANGE(IDC_BLENDWEIGHT2, OnChangeBlendweight2)
	ON_EN_CHANGE(IDC_BLENDWEIGHT3, OnChangeBlendweight3)
	ON_CBN_SELCHANGE(IDC_BLENDANIM0, OnSelchangeBlendanim0)
	ON_CBN_SELCHANGE(IDC_BLENDANIM1, OnSelchangeBlendanim1)
	ON_CBN_SELCHANGE(IDC_BLENDANIM2, OnSelchangeBlendanim2)
	ON_CBN_SELCHANGE(IDC_BLENDANIM3, OnSelchangeBlendanim3)
	ON_BN_CLICKED(IDC_BLENDED, OnBlended)
	ON_WM_DESTROY()
	ON_WM_CANCELMODE()
	ON_BN_CLICKED(IDC_QUATBLENDINGFORPROPS, OnQuatblendingforprops)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgMotion message handlers

BOOL CDlgMotion::OnInitDialog() 
{
	s32 i;

	//---	save a copy of the current motion in case the user cancel's the changes
	m_Motion = *m_pMotion;

	m_bIsMotionBlended = (m_pMotion->m_BlendedMotionCount > 0) ? TRUE : FALSE;

	CDialog::OnInitDialog();

	s_BlendWeightArray[0]	= (CEdit*) GetDlgItem( IDC_BLENDWEIGHT0 );
	s_BlendWeightArray[1]	= (CEdit*) GetDlgItem( IDC_BLENDWEIGHT1 );
	s_BlendWeightArray[2]	= (CEdit*) GetDlgItem( IDC_BLENDWEIGHT2 );
	s_BlendWeightArray[3]	= (CEdit*) GetDlgItem( IDC_BLENDWEIGHT3 );

	s_BlendAnimArray[0]		= (CComboBox*) GetDlgItem( IDC_BLENDANIM0 );
	s_BlendAnimArray[1]		= (CComboBox*) GetDlgItem( IDC_BLENDANIM1 );
	s_BlendAnimArray[2]		= (CComboBox*) GetDlgItem( IDC_BLENDANIM2 );
	s_BlendAnimArray[3]		= (CComboBox*) GetDlgItem( IDC_BLENDANIM3 );

	s_BlendSpinnerArray[0]	= (CSpinButtonCtrl*) GetDlgItem( IDC_WEIGHTSPIN0 );
	s_BlendSpinnerArray[1]	= (CSpinButtonCtrl*) GetDlgItem( IDC_WEIGHTSPIN1 );
	s_BlendSpinnerArray[2]	= (CSpinButtonCtrl*) GetDlgItem( IDC_WEIGHTSPIN2 );
	s_BlendSpinnerArray[3]	= (CSpinButtonCtrl*) GetDlgItem( IDC_WEIGHTSPIN3 );

	for( i=0; i<BLEND_PAGE_LENGTH; i++ )
		s_BlendSpinnerArray[i]->SetRange( 0, 100 );

	((CButton*)GetDlgItem( IDC_QUATBLENDING ))->SetCheck( m_pMotion->m_bExportQuatBlending ? TRUE : FALSE );
	((CButton*)GetDlgItem( IDC_QUATBLENDINGFORPROPS ))->SetCheck( m_pMotion->m_bExportPropQuatBlending ? TRUE : FALSE );
	((CButton*)GetDlgItem( IDC_WORLDROTS ))->SetCheck( m_pMotion->m_bExportWorldRots ? TRUE : FALSE );
	((CButton*)GetDlgItem( IDC_GLOBALROTS ))->SetCheck( m_pMotion->m_bExportGlobalRots ? TRUE : FALSE );
	((CButton*)GetDlgItem( IDC_QUATERNION ))->SetCheck( m_pMotion->m_bExportQuaternion ? TRUE : FALSE );
	((CButton*)GetDlgItem( IDC_EXPORTMETHOD_SET ))->SetCheck( m_pMotion->m_bExportDefault ? TRUE : FALSE );
	((CButton*)GetDlgItem( IDC_QUATW_ON ))->SetCheck( m_pMotion->m_bExportQuatW ? TRUE : FALSE );
	OnExportmethodSet();
	
	InitBlendDialog();
	UpdateDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnDestroy() 
{
	s32 i;

	CDialog::OnDestroy();
	
	for( i=0; i<BLEND_PAGE_LENGTH; i++ )
	{
		s_BlendWeightArray[i]	= NULL;
		s_BlendAnimArray[i]		= NULL;
		s_BlendSpinnerArray[i]	= NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::InitBlendAnimCombo( s32 iCombo )
{
	CMotion*	pCurSelect = NULL;
	CMotion*	pMotion;
	s32			iMotion;
	s32			MotionIndex;

	s32			CurSel = s_BlendAnimArray[iCombo]->GetCurSel();
	if( CurSel >= 0 )
		pCurSelect = (CMotion*) s_BlendAnimArray[iCombo]->GetItemData( CurSel );

	s_BlendAnimArray[iCombo]->ResetContent();
	s_BlendAnimArray[iCombo]->AddString( "<no motion>" );

	for( iMotion=0; iMotion<m_pMotion->m_pCharacter->GetNumMotions(); iMotion++ )
	{
		//---	get the next motion, do not add the edited motion to the drop list
		pMotion = m_pMotion->m_pCharacter->GetMotion( iMotion );
		if( pMotion == m_pMotion )
			continue;

		MotionIndex = s_BlendAnimArray[iCombo]->AddString( pMotion->GetExportName() );
		s_BlendAnimArray[iCombo]->SetItemData( MotionIndex, (u32)pMotion );
	}
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::InitBlendDialog( void )
{
	s32 i;

	for( i=0; i<BLEND_PAGE_LENGTH; i++ )
		InitBlendAnimCombo( i );
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::UpdateBlendDialog( s32 Keeper )
{
	f32 Blend;
	f32	Percent;
	char Buff[256];
	s32	iBlendAnim;
	CComboBox*	pCombo;

	m_UpdatingBlendDialog = TRUE;

	m_Blended.SetCheck( m_bIsMotionBlended ? TRUE : FALSE );

	for( iBlendAnim=0; iBlendAnim<BLEND_PAGE_LENGTH; iBlendAnim++ )
	{
		if( iBlendAnim != Keeper )
		{
			//---	set the blend amount
			Blend = m_pMotion->m_BlendedMotions[iBlendAnim].Ratio;
			Percent = 100.0f*Blend + 0.5f;
			s_BlendWeightArray[iBlendAnim]->GetWindowText( Buff, 255 );
			if( atoi( Buff ) != (s32)Percent )
				s_BlendWeightArray[iBlendAnim]->SetWindowText( itoa( (s32)Percent, Buff, 10 ) );
		}

		//---	set the blend motion
		pCombo = s_BlendAnimArray[iBlendAnim];
		if( !m_pMotion->m_BlendedMotions[iBlendAnim].pMotion )
			pCombo->SetCurSel( 0 );
		else
		{
			for( s32 iMotion=0; iMotion<pCombo->GetCount(); iMotion++ )
			{
				if( (u32)m_pMotion->m_BlendedMotions[iBlendAnim].pMotion == pCombo->GetItemData( iMotion ) )
					pCombo->SetCurSel( iMotion );
			}
		}

		s_BlendWeightArray[iBlendAnim]->EnableWindow( m_bIsMotionBlended );
		s_BlendAnimArray[iBlendAnim]->EnableWindow( m_bIsMotionBlended );
	}

	m_UpdatingBlendDialog = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::UpdateDialog( void )
{
	UpdateBlendDialog();
	
	SetDlgItemText( IDC_NAME, m_pMotion->m_ExportName );
	SetDlgItemText( IDC_FILEPATHNAME, m_pMotion->m_PathName );
	m_SourceFile.EnableWindow( m_bIsMotionBlended ? FALSE : TRUE );
	m_Browse.EnableWindow( m_bIsMotionBlended ? FALSE : TRUE );
	((CButton*)GetDlgItem( IDC_EXPORT ))->SetCheck( m_pMotion->m_bExport );
	
	m_ExportFrameRateButton.SetCheck( m_pMotion->m_bExportFrameRate );
	CString FrameRate;
	FrameRate.Format( "%5.2f", m_pMotion->m_ExportFrameRate );
	m_ExportFrameRate.SetWindowText( FrameRate );
	if( !m_pMotion->m_bExportFrameRate )
		m_ExportFrameRate.EnableWindow( FALSE );
	
	m_ExportBitsButton.SetCheck( m_pMotion->m_bExportBits );
	CString Bits;
	Bits.Format( "%d", m_pMotion->m_ExportBits );
	m_ExportBits.SetWindowText( Bits );
	if( !m_pMotion->m_bExportBits )
		m_ExportBits.EnableWindow( FALSE );
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnCancel() 
{
	//---	restore the motion as it was
	*m_pMotion = m_Motion;
	
	CDialog::OnCancel();
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnOK( void )
{
	GetDlgItemText( IDC_NAME, m_pMotion->m_ExportName );
	GetDlgItemText( IDC_FILEPATHNAME, m_pMotion->m_PathName );
	m_pMotion->m_bExport = ((CButton*)GetDlgItem( IDC_EXPORT ))->GetCheck();

	m_pMotion->m_bExportFrameRate = m_ExportFrameRateButton.GetCheck() ? TRUE : FALSE;

	CString FrameRate;
	m_ExportFrameRate.GetWindowText( FrameRate );
	m_pMotion->m_ExportFrameRate = (f32)atof( FrameRate );
	m_pMotion->m_ExportFrameRate = m_pMotion->GetExportFrameRate();

	CString Bits;
	m_ExportBits.GetWindowText( Bits );
	m_pMotion->m_ExportBits = atoi( Bits );
	m_pMotion->m_ExportBits = m_pMotion->GetExportBits();

	m_pMotion->m_bExportQuatBlending		= ((CButton*)GetDlgItem( IDC_QUATBLENDING ))->GetCheck() ? TRUE : FALSE;
	m_pMotion->m_bExportPropQuatBlending	= ((CButton*)GetDlgItem( IDC_QUATBLENDINGFORPROPS ))->GetCheck() ? TRUE : FALSE;
	m_pMotion->m_bExportWorldRots			= ((CButton*)GetDlgItem( IDC_WORLDROTS ))->GetCheck() ? TRUE : FALSE;
	m_pMotion->m_bExportGlobalRots			= ((CButton*)GetDlgItem( IDC_GLOBALROTS ))->GetCheck() ? TRUE : FALSE;
	m_pMotion->m_bExportQuaternion			= ((CButton*)GetDlgItem( IDC_QUATERNION ))->GetCheck() ? TRUE : FALSE;
	m_pMotion->m_bExportDefault				= ((CButton*)GetDlgItem( IDC_EXPORTMETHOD_SET ))->GetCheck() ? TRUE : FALSE;
	m_pMotion->m_bExportQuatW				= ((CButton*)GetDlgItem( IDC_QUATW_ON ))->GetCheck() ? TRUE : FALSE;

	CDialog::OnOK();
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnBrowse() 
{
	CFileDialog fd( TRUE, NULL, "*.amc" ) ;

	fd.m_ofn.lpstrTitle = "Import Motion Files" ;
	fd.m_ofn.lpstrFilter = "AMC Files (*.amc)\000*.amc\000ANM Files (*.anm)\000*.anm\000All Files (*.*)\000*.*\000" ;

	if( fd.DoModal() == IDOK )
	{
		CString Text = fd.GetPathName();
		if( Text.Find( "*.amc", 0 ) != -1 )
			return;

		SetDlgItemText( IDC_FILEPATHNAME, Text );
	}
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnExport() 
{
	
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnExportFramerateSet() 
{
	m_pMotion->m_bExportFrameRate = m_ExportFrameRateButton.GetCheck() ? TRUE : FALSE;
	m_ExportFrameRate.EnableWindow( m_pMotion->m_bExportFrameRate );
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnChangeExportFramerate() 
{
	CString FrameRate;
	m_ExportFrameRate.GetWindowText( FrameRate );
	f32 fFrameRate = (f32)atof( FrameRate );

	bool save = m_pMotion->m_bExportFrameRate;
	m_pMotion->m_ExportFrameRate = fFrameRate;
	m_pMotion->m_ExportFrameRate = m_pMotion->GetExportFrameRate();
	m_pMotion->m_bExportFrameRate = save;
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnKillfocusExportFramerate() 
{
	CString FrameRate;
	FrameRate.Format( "%5.2f", m_pMotion->m_ExportFrameRate );
	m_ExportFrameRate.SetWindowText( FrameRate );
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnExportBitsSet() 
{
	m_pMotion->m_bExportBits = m_ExportBitsButton.GetCheck() ? TRUE : FALSE;
	m_ExportBits.EnableWindow( m_pMotion->m_bExportBits );
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnChangeExportBits() 
{
	CString Bits;
	m_ExportBits.GetWindowText( Bits );
	s32 eBits = atoi( Bits );

	bool save = m_pMotion->m_bExportBits;
	m_pMotion->m_ExportBits = eBits;
	m_pMotion->m_ExportBits = m_pMotion->GetExportBits();
	m_pMotion->m_bExportBits = save;
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnKillfocusExportBits() 
{
	CString Bits;
	Bits.Format( "%d", m_pMotion->m_ExportBits );
	m_ExportBits.SetWindowText( Bits );
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnQuaternion() 
{
	bool bEnabled = ((CButton*)GetDlgItem( IDC_QUATERNION ))->GetCheck() ? TRUE : FALSE;

	GetDlgItem( IDC_QUATBLENDING )->EnableWindow( !bEnabled );
	GetDlgItem( IDC_QUATBLENDINGFORPROPS )->EnableWindow( !bEnabled );
	GetDlgItem( IDC_QUATW_ON )->EnableWindow( bEnabled );

	if( bEnabled )
		OnQuatbitsOn();
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnWorldrots() 
{
	
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnGlobalrots() 
{
	
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnQuatblending() 
{
	
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnQuatblendingforprops() 
{
	
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnExportmethodSet() 
{
	bool bEnable = ((CButton*)GetDlgItem( IDC_EXPORTMETHOD_SET ))->GetCheck() ? FALSE : TRUE;

	GetDlgItem( IDC_QUATERNION )->EnableWindow( bEnable );
	GetDlgItem( IDC_WORLDROTS )->EnableWindow( bEnable );
	GetDlgItem( IDC_GLOBALROTS )->EnableWindow( bEnable );
	GetDlgItem( IDC_QUATBLENDING )->EnableWindow( bEnable );
	GetDlgItem( IDC_QUATBLENDINGFORPROPS )->EnableWindow( bEnable );
	GetDlgItem( IDC_ROTATIONMETHODTITLE )->EnableWindow( bEnable );
	GetDlgItem( IDC_QUATW_ON )->EnableWindow( bEnable );

	if( bEnable )
		OnQuaternion();
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnQuatbitsOn() 
{
	bool bQuatWOn = ((CButton*)GetDlgItem( IDC_QUATW_ON ))->GetCheck() ? TRUE : FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::SaveAndAdjustBlendedWeights( s32 Keeper )
{
	char	WeightStr[32];
	f32		Weights[4];
	f32		fWeight;
	s32		i;

	//---	store the display weights into the motion weights
	for( i=0; i<BLEND_PAGE_LENGTH; i++ )
	{
		s_BlendWeightArray[i]->GetWindowText( WeightStr, 31 );
		fWeight = (f32)atoi( WeightStr )*0.01f;
		m_pMotion->m_BlendedMotions[i].Ratio = fWeight;
	}

	//---	read all of the weights into a temporary buffer
	for( i=0; i<MAX_NUM_BLENDED_MOTIONS; i++ )
	{
		//---	make sure that no position which has no motion has a weight
		if( !m_pMotion->m_BlendedMotions[i].pMotion )
			m_pMotion->m_BlendedMotions[i].Ratio = 0.0f;

		Weights[i] = m_pMotion->m_BlendedMotions[i].Ratio;

		if( Weights[i] < 0.0f )	Weights[i] = 0.0f;
		if( Weights[i] > 1.0f )	Weights[i] = 1.0f;
	}

	//---	process all of the weights starting with the keeper and limit the total weight to 1.0f
	f32 TotalWeight = 0.0f;
	if( Keeper>=0 )
		TotalWeight += Weights[Keeper];

	for( i=0; i<MAX_NUM_BLENDED_MOTIONS; i++ )
	{
		if( i==Keeper )
			continue;

		TotalWeight += Weights[i];
	}

	//---	if the total weight is less than 100 percent, adjust the weights
	if( TotalWeight < 1.0f )
	{
		for( i=1; i<=MAX_NUM_BLENDED_MOTIONS; i++ )
		{
			s32 BlendedMotion = (Keeper+i)%MAX_NUM_BLENDED_MOTIONS;
			if( m_pMotion->m_BlendedMotions[BlendedMotion].pMotion )
			{
				Weights[BlendedMotion] += 1.0f - TotalWeight;
				break;
			}
		}
	}
	if( TotalWeight > 1.0f )
	{
		for( i=1; i<=MAX_NUM_BLENDED_MOTIONS; i++ )
		{
			s32 BlendedMotion = (Keeper+i)%MAX_NUM_BLENDED_MOTIONS;
			if( Weights[BlendedMotion] > 0.0f )
			{
				f32 AmountToRemove = TotalWeight - 1.0f;
				if( Weights[BlendedMotion] < AmountToRemove )
					AmountToRemove = Weights[BlendedMotion];

				Weights[BlendedMotion] -= AmountToRemove;
				TotalWeight -= AmountToRemove;

				if( TotalWeight <= 1.0f )
					break;
			}
		}
	}

	//---	change the values of the weights
	s32	NumMotions = 0;
	for( i=0; i<MAX_NUM_BLENDED_MOTIONS; i++ )
	{
		if( Weights[i]>0.0f )
			NumMotions = i+1;

		m_pMotion->m_BlendedMotions[i].Ratio = Weights[i];
	}

	m_pMotion->m_BlendedMotionCount = NumMotions;

	//---	update the dialog to display the changes to the motion
	UpdateBlendDialog( Keeper );
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnBlended() 
{
	if(!GetSafeHwnd())
		return;

	//---	if the value was set while updating the blend dialog, exit now
	if( m_UpdatingBlendDialog )
		return;

	m_bIsMotionBlended = m_Blended.GetCheck() ? TRUE : FALSE;

	//---	update the dialog to display the changes to the motion
	UpdateDialog();
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnSelChangeBlendAnim( s32 iCombo )
{
	if(!GetSafeHwnd())
		return;

	//---	if the value was set while updating the blend dialog, exit now
	if( m_UpdatingBlendDialog )
		return;

	if( !s_BlendAnimArray[0] )
		return;

	//---	get the pointer to the selected motion
	s32 Index = s_BlendAnimArray[iCombo]->GetCurSel();
	if( Index == -1 )
	{
		m_pMotion->m_BlendedMotions[iCombo].pMotion			= NULL;
		m_pMotion->m_BlendedMotions[iCombo].Ratio			= 0.0f;
		m_pMotion->m_BlendedMotions[iCombo].RuntimeVersion	= -1;
	}
	else
	{
		CMotion* pMotion = (CMotion*)s_BlendAnimArray[iCombo]->GetItemData( Index );
		if( m_pMotion->m_BlendedMotions[iCombo].pMotion != pMotion )
		{
			m_pMotion->m_BlendedMotions[iCombo].pMotion			= (CMotion*)s_BlendAnimArray[iCombo]->GetItemData( Index );
			m_pMotion->m_BlendedMotions[iCombo].RuntimeVersion	= -1;	// force an update
		}
	}

	//---	adjust the blended weights
	SaveAndAdjustBlendedWeights( -1 );
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnSelChangeWeight( s32 iWeight )
{
	if(!GetSafeHwnd())
		return;

	//---	if the value was set while updating the blend dialog, exit now
	if( m_UpdatingBlendDialog )
		return;

	if( !s_BlendAnimArray[0] )
		return;

	//---	get the pointer to the selected motion
	SaveAndAdjustBlendedWeights( iWeight );
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnChangeBlendweight0() 
{
	OnSelChangeWeight( 0 );
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnChangeBlendweight1() 
{
	OnSelChangeWeight( 1 );
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnChangeBlendweight2() 
{
	OnSelChangeWeight( 2 );
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnChangeBlendweight3() 
{
	OnSelChangeWeight( 3 );
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnSelchangeBlendanim0() 
{
	OnSelChangeBlendAnim( 0 );
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnSelchangeBlendanim1() 
{
	OnSelChangeBlendAnim( 1 );
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnSelchangeBlendanim2() 
{
	OnSelChangeBlendAnim( 2 );
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnSelchangeBlendanim3() 
{
	OnSelChangeBlendAnim( 3 );
}

/////////////////////////////////////////////////////////////////////////////
void CDlgMotion::OnCancelMode() 
{
	CDialog::OnCancelMode();
	
	// TODO: Add your message handler code here
	
}
