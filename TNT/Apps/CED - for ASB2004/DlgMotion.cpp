// DlgMotion.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "DlgMotion.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgMotion dialog


CDlgMotion::CDlgMotion(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgMotion::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgMotion)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgMotion::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgMotion)
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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgMotion message handlers

BOOL CDlgMotion::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetDlgItemText( IDC_NAME, m_Motion.m_ExportName );
	SetDlgItemText( IDC_FILEPATHNAME, m_Motion.m_PathName );
	((CButton*)GetDlgItem( IDC_EXPORT ))->SetCheck( m_Motion.m_bExport );
	
	m_ExportFrameRateButton.SetCheck( m_Motion.m_bExportFrameRate );
	CString FrameRate;
	FrameRate.Format( "%5.2f", m_Motion.m_ExportFrameRate );
	m_ExportFrameRate.SetWindowText( FrameRate );
	if( !m_Motion.m_bExportFrameRate )
		m_ExportFrameRate.EnableWindow( FALSE );
	
	m_ExportBitsButton.SetCheck( m_Motion.m_bExportBits );
	CString Bits;
	Bits.Format( "%d", m_Motion.m_ExportBits );
	m_ExportBits.SetWindowText( Bits );
	if( !m_Motion.m_bExportBits )
		m_ExportBits.EnableWindow( FALSE );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgMotion::OnOK( void )
{
	GetDlgItemText( IDC_NAME, m_Motion.m_ExportName );
	GetDlgItemText( IDC_FILEPATHNAME, m_Motion.m_PathName );
	m_Motion.m_bExport = ((CButton*)GetDlgItem( IDC_EXPORT ))->GetCheck() ? TRUE : FALSE;

	m_Motion.m_bExportFrameRate = m_ExportFrameRateButton.GetCheck() ? TRUE : FALSE;

	CString FrameRate;
	m_ExportFrameRate.GetWindowText( FrameRate );
	m_Motion.m_ExportFrameRate = (f32)atof( FrameRate );
	m_Motion.m_ExportFrameRate = m_Motion.GetExportFrameRate();

	CString Bits;
	m_ExportBits.GetWindowText( Bits );
	m_Motion.m_ExportBits = atoi( Bits );
	m_Motion.m_ExportBits = m_Motion.GetExportBits();

	CDialog::OnOK();
}


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

void CDlgMotion::OnExport() 
{
	
}

void CDlgMotion::OnExportFramerateSet() 
{
	m_Motion.m_bExportFrameRate = m_ExportFrameRateButton.GetCheck() ? TRUE : FALSE;
	m_ExportFrameRate.EnableWindow( m_Motion.m_bExportFrameRate );
}

void CDlgMotion::OnChangeExportFramerate() 
{
	CString FrameRate;
	m_ExportFrameRate.GetWindowText( FrameRate );
	f32 fFrameRate = (f32)atof( FrameRate );

	bool save = m_Motion.m_bExportFrameRate;
	m_Motion.m_ExportFrameRate = fFrameRate;
	m_Motion.m_ExportFrameRate = m_Motion.GetExportFrameRate();
	m_Motion.m_bExportFrameRate = save;
}

void CDlgMotion::OnKillfocusExportFramerate() 
{
	CString FrameRate;
	FrameRate.Format( "%5.2f", m_Motion.m_ExportFrameRate );
	m_ExportFrameRate.SetWindowText( FrameRate );
}

void CDlgMotion::OnExportBitsSet() 
{
	m_Motion.m_bExportBits = m_ExportBitsButton.GetCheck() ? TRUE : FALSE;
	m_ExportBits.EnableWindow( m_Motion.m_bExportBits );
}

void CDlgMotion::OnChangeExportBits() 
{
	CString Bits;
	m_ExportBits.GetWindowText( Bits );
	s32 eBits = atoi( Bits );

	bool save = m_Motion.m_bExportBits;
	m_Motion.m_ExportBits = eBits;
	m_Motion.m_ExportBits = m_Motion.GetExportBits();
	m_Motion.m_bExportBits = save;
}

void CDlgMotion::OnKillfocusExportBits() 
{
	CString Bits;
	Bits.Format( "%d", m_Motion.m_ExportBits );
	m_ExportBits.SetWindowText( Bits );
}
