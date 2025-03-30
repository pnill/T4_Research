// DlgCharacter.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "DlgCharacter.h"

#include "Character.h"
#include "MotionEditView.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCharacter dialog


CDlgCharacter::CDlgCharacter(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgCharacter::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCharacter)
	//}}AFX_DATA_INIT

	m_pView = NULL ;
}


void CDlgCharacter::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCharacter)
	DDX_Control(pDX, IDC_QUATERNION_ON, m_Quaternion);
	DDX_Control(pDX, IDC_CHARACTER_EXPORTQUATW_ON, m_ExportQuatW);
	DDX_Control(pDX, IDC_CHARACTER_EXPORTBITS_SPIN, m_ExportBitsSpin);
	DDX_Control(pDX, IDC_CHARACTER_EXPORTFPS_SPIN, m_ExportFPSSpin);
	DDX_Control(pDX, IDC_CHARACTER_EXPORTBITS_TITLE, m_ExportBitsTitle);
	DDX_Control(pDX, IDC_CHARACTER_EXPORT_BITS, m_ExportBits);
	DDX_Control(pDX, IDC_CHARACTER_EXPORTFPS_TITLE, m_ExportFPSTitle);
	DDX_Control(pDX, IDC_CHARACTER_EXPORTFPS, m_ExportFPS);
	DDX_Control(pDX, IDC_NOEXPORT, m_NoExport);
	DDX_Control(pDX, IDC_CHARACTER, m_Combo);
	//}}AFX_DATA_MAP
}




void CDlgCharacter::Reset( )
{
	m_Combo.ResetContent() ;
	m_Combo.AddString( "<no character>" ) ;
}

void CDlgCharacter::AddCharacter( CCharacter *pCharacter )
{
	int Pos = m_Combo.AddString( pCharacter->GetName() ) ;
	ASSERT( Pos != CB_ERR ) ;
	ASSERT( Pos != CB_ERRSPACE ) ;
	m_Combo.SetItemData( Pos, (DWORD)pCharacter ) ;
}

void CDlgCharacter::SelectCharacter( CCharacter *pCharacter )
{
	int Index = MFCU_FindComboBoxItemByData( &m_Combo, (DWORD)pCharacter ) ;
	if( Index != -1 )
	{
		m_Combo.SetCurSel( Index ) ;
		UpdateDetails( ) ;
	}
}

void CDlgCharacter::UpdateDetails( )
{
	bool	Enable = false ;

	//---	Get Selected Character
	CCharacter *pCharacter = (CCharacter*)m_Combo.GetItemData( m_Combo.GetCurSel() ) ;
	if( pCharacter )
	{
		Enable = TRUE;

		m_NoExport.SetCheck( pCharacter->m_NoExport ? TRUE : FALSE );
		m_Quaternion.SetCheck( pCharacter->m_bExportQuaternion ? TRUE : FALSE );
		m_ExportQuatW.SetCheck( pCharacter->m_bExportQuatW ? TRUE : FALSE );

		CMotion dummyMotion;
		dummyMotion.m_bExportFrameRate = TRUE;
		dummyMotion.m_ExportFrameRate = m_pView->m_pCharacter->m_ExportFrameRate;
		f32 fFrameRate = dummyMotion.GetExportFrameRate();
		CString FrameRate;
		FrameRate.Format( "%5.2f", fFrameRate );
		m_ExportFPS.SetWindowText( FrameRate );
		m_ExportFPSSpin.SetPos( (s32)fFrameRate );

		CString Bits;
		s32 eBits = m_pView->m_pCharacter->m_ExportBits;
		if( eBits > 16 ) eBits = 16;
		else if( eBits < 0 ) eBits = 0;
		Bits.Format( "%d", eBits );
		m_ExportBits.SetWindowText( Bits );
		m_ExportBitsSpin.SetPos( eBits );
	}

	m_NoExport.EnableWindow( Enable ) ;
	m_ExportFPSTitle.EnableWindow( Enable );
	m_ExportFPSSpin.EnableWindow( Enable );
	m_ExportFPS.EnableWindow( Enable );
	m_ExportBits.EnableWindow( Enable );
	m_ExportBitsSpin.EnableWindow( Enable );
	m_ExportBitsTitle.EnableWindow( Enable );
	m_ExportQuatW.EnableWindow( Enable && m_Quaternion.GetCheck() );
}



BEGIN_MESSAGE_MAP(CDlgCharacter, CDialog)
	//{{AFX_MSG_MAP(CDlgCharacter)
	ON_CBN_SELCHANGE(IDC_CHARACTER, OnSelchangeCharacter)
	ON_BN_CLICKED(IDC_NOEXPORT, OnNoexport)
	ON_EN_KILLFOCUS(IDC_CHARACTER_EXPORTFPS, OnKillfocusCharacterExportfps)
	ON_EN_CHANGE(IDC_CHARACTER_EXPORTFPS, OnChangeCharacterExportfps)
	ON_EN_CHANGE(IDC_CHARACTER_EXPORT_BITS, OnChangeCharacterExportBits)
	ON_EN_KILLFOCUS(IDC_CHARACTER_EXPORT_BITS, OnKillfocusCharacterExportBits)
	ON_NOTIFY(UDN_DELTAPOS, IDC_CHARACTER_EXPORTBITS_SPIN, OnDeltaposCharacterExportbitsSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_CHARACTER_EXPORTFPS_SPIN, OnDeltaposCharacterExportfpsSpin)
	ON_BN_CLICKED(IDC_EXPORTOPTIONS, OnExportoptions)
	ON_BN_CLICKED(IDC_CHARACTER_EXPORTQUATW_ON, OnCharacterExportquatbitsOn)
	ON_BN_CLICKED(IDC_QUATERNION_ON, OnQuaternionOn)
	//}}AFX_MSG_MAP

	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDCANCEL, OnCancel)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCharacter message handlers

void CDlgCharacter::OnOk() 
{
}

void CDlgCharacter::OnCancel() 
{
}

void CDlgCharacter::OnSelchangeCharacter() 
{
	// TODO: Add your control notification handler code here
	ASSERT( m_pView ) ;
	m_pView->SetCharacter( (CCharacter*)m_Combo.GetItemData( m_Combo.GetCurSel() ) ) ;
	UpdateDetails( ) ;
}

void CDlgCharacter::OnNoexport() 
{
	// TODO: Add your control notification handler code here
	CCharacter *pCharacter = (CCharacter*)m_Combo.GetItemData( m_Combo.GetCurSel() ) ;
	if( pCharacter )
	{
		pCharacter->m_NoExport = !!m_NoExport.GetCheck() ;
	}
}

void CDlgCharacter::OnKillfocusCharacterExportfps() 
{
	CString FrameRate;
	FrameRate.Format( "%5.2f", this->m_pView->m_pCharacter->m_ExportFrameRate );
	m_ExportFPS.SetWindowText( FrameRate );
}

void CDlgCharacter::OnChangeCharacterExportfps() 
{
	CMotion dummyMotion;
	
	if( !m_ExportFPS.m_hWnd )
		return;

	dummyMotion.m_bExportFrameRate = TRUE;

	CString FrameRate;
	m_ExportFPS.GetWindowText( FrameRate );
	f32 fFrameRate = (f32)atof( FrameRate );
	dummyMotion.m_ExportFrameRate = fFrameRate;

	//---	keep the value within range
	if(( fFrameRate > 60.0f ) || ( fFrameRate < 0.0f ))
	{
		if( fFrameRate > 10.0f )
		{
			fFrameRate = 60.0f;
			FrameRate.Format( "%5.2f", fFrameRate );
		}
		else
		{
			fFrameRate = 0.0f;
			FrameRate.Format( "%5.2f", fFrameRate );
		}
		m_ExportFPS.SetWindowText( FrameRate );
	}

	fFrameRate = dummyMotion.GetExportFrameRate();

	if( m_pView->m_pCharacter->m_ExportFrameRate == fFrameRate )
		return;

	m_ExportFPSSpin.SetPos( (s32)fFrameRate );
	m_pView->m_pCharacter->m_ExportFrameRate = fFrameRate;	
	m_pView->Invalidate();
}

void CDlgCharacter::OnChangeCharacterExportBits() 
{
	CString Bits;

	if( !m_ExportBits.m_hWnd || !m_pView->m_pCharacter )
		return;

	m_ExportBits.GetWindowText( Bits );	
	s32 eBits = atoi( Bits );
	if( eBits > 16 )		eBits = 16;
	else if( eBits < 0 )	eBits = 0;
	m_pView->m_pCharacter->m_ExportBits = eBits;
	m_ExportBitsSpin.SetPos( eBits );

	//---	set the direction values to reflect the new export bits
	m_pView->UpdateMotionControls();
	m_pView->UpdateEventControls();
	m_pView->Invalidate();

	if( m_pView->m_pCharacter->m_pCurMotion )
		m_pView->m_pCharacter->m_pCurMotion->ForceCompressedVersionUpdate();
}

void CDlgCharacter::OnKillfocusCharacterExportBits() 
{
	CString Bits;
	Bits.Format( "%d", m_pView->m_pCharacter->m_ExportBits );
	m_ExportBits.SetWindowText( Bits );

	//---	set the direction values to reflect the new export bits
	m_pView->UpdateMotionControls();
	m_pView->UpdateEventControls();
}

void CDlgCharacter::OnDeltaposCharacterExportbitsSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

//	OnChangeCharacterExportBits();

//	CString Bits;
//	m_ExportBits.GetWindowText( Bits );	
//	s32 eBits = atoi( Bits );
//	if( eBits > 16 )		eBits = 16;
//	else if( eBits < 0 )	eBits = 0;
//	m_pView->m_pCharacter->m_ExportBits = eBits;
//
//	//---	set the direction values to reflect the new export bits
//	m_pView->UpdateMotionControls();
//	m_pView->UpdateEventControls();
//	m_pView->Invalidate();

	*pResult = 0;
}

void CDlgCharacter::OnDeltaposCharacterExportfpsSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	float Change = 1.0f*(float)pNMUpDown->iDelta;
	CString Text;
	
	m_ExportFPS.GetWindowText( Text );
	f32 Value = (f32)atof( Text );
	Value += Change;
	Text.Format( "%5.2f", Value );
	m_ExportFPS.SetWindowText( Text );
	
	*pResult = 0;
}

BOOL CDlgCharacter::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_ExportBitsSpin.SetRange( 0, 16 );
	m_ExportFPSSpin.SetRange( 0, 60 );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCharacter::OnExportoptions() 
{
	m_pView->GetDocument()->CharacterExportFull( m_pView->m_pCharacter );
}

void CDlgCharacter::OnCharacterExportquatbitsOn() 
{
	if( !m_pView )	return;
	if( !m_pView->m_pCharacter )	return;

	m_pView->m_pCharacter->m_bExportQuatW	= m_ExportQuatW.GetCheck() ? TRUE : FALSE;

	if( m_pView->m_pCharacter->m_pCurMotion )
		m_pView->m_pCharacter->m_pCurMotion->ForceCompressedVersionUpdate();

	UpdateDetails();
	m_pView->Invalidate();
}

void CDlgCharacter::OnQuaternionOn() 
{
	if( !m_pView )	return;
	if( !m_pView->m_pCharacter )	return;

	m_pView->m_pCharacter->m_bExportQuaternion	= m_Quaternion.GetCheck() ? TRUE : FALSE;
	if( m_pView->m_pCharacter->m_pCurMotion )
		m_pView->m_pCharacter->m_pCurMotion->ForceCompressedVersionUpdate();

	UpdateDetails();
	m_pView->Invalidate();
}
