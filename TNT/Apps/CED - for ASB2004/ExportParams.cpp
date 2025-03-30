// ExportParams.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "ExportParams.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExportParams dialog


CExportParams::CExportParams(CWnd* pParent /*=NULL*/)
	: CDialog(CExportParams::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExportParams)
	m_OptForceUpper = FALSE;
	m_EnableGAMECUBE = FALSE;
	m_EnablePC = FALSE;
	m_EnableXBOX = FALSE;
    m_EnablePS2 = FALSE;
	m_FileGAMECUBE = _T("");
	m_FilePC = _T("");
	m_FileXBOX = _T("");
    m_FilePS2 = _T("");
	m_CharacterIncludeFile = _T("");
	m_OptCollisionBubbles = FALSE;
	m_OptMotionFlags = FALSE;
	m_OptFrontEndSpecific = FALSE;
	m_FramesPerBlock = 0;
	//}}AFX_DATA_INIT
}


void CExportParams::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportParams)
	DDX_Check(pDX, IDC_EXPORT_FORCE_UPPER, m_OptForceUpper);
	DDX_Check(pDX, IDC_EXPORT_ENABLE_GAMECUBE, m_EnableGAMECUBE);
	DDX_Check(pDX, IDC_EXPORT_ENABLE_PC, m_EnablePC);
	DDX_Check(pDX, IDC_EXPORT_ENABLE_XBOX, m_EnableXBOX);
	DDX_Check(pDX, IDC_EXPORT_ENABLE_PS2, m_EnablePS2);
	DDX_Text(pDX, IDC_EXPORT_FILE_GAMECUBE, m_FileGAMECUBE);
	DDX_Text(pDX, IDC_EXPORT_FILE_PC, m_FilePC);
	DDX_Text(pDX, IDC_EXPORT_FILE_XBOX, m_FileXBOX);
	DDX_Text(pDX, IDC_EXPORT_FILE_PS2, m_FilePS2);
	DDX_Text(pDX, IDC_EXPORT_FILE_GLOBAL, m_CharacterIncludeFile);
	DDX_Check(pDX, IDC_EXPORT_COLLISIONBUBBLES, m_OptCollisionBubbles);
	DDX_Check(pDX, IDC_EXPORT_MOTIONFLAGS, m_OptMotionFlags);
	DDX_Check(pDX, IDC_EXPORT_FRONT_END, m_OptFrontEndSpecific);
	DDX_Text(pDX, IDC_EXPORT_FRAMESPERBLOCK, m_FramesPerBlock);
	DDV_MinMaxInt(pDX, m_FramesPerBlock, 4, 1000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExportParams, CDialog)
	//{{AFX_MSG_MAP(CExportParams)
	ON_BN_CLICKED(IDC_EXPORT_BROWSE_GAMECUBE, OnExportBrowseGAMECUBE)
	ON_BN_CLICKED(IDC_EXPORT_BROWSE_PC, OnExportBrowsePC)
	ON_BN_CLICKED(IDC_EXPORT_BROWSE_XBOX, OnExportBrowseXBOX)
	ON_BN_CLICKED(IDC_EXPORT_BROWSE_PS2, OnExportBrowsePS2)
	ON_BN_CLICKED(IDC_EXPORT_ENABLE_GAMECUBE, OnExportEnableGAMECUBE)
	ON_BN_CLICKED(IDC_EXPORT_ENABLE_PC, OnExportEnablePC)
	ON_BN_CLICKED(IDC_EXPORT_ENABLE_XBOX, OnExportEnableXBOX)
	ON_BN_CLICKED(IDC_EXPORT_ENABLE_PS2, OnExportEnablePS2)
	ON_BN_CLICKED(IDC_EXPORT_BROWSE_GLOBAL, OnExportBrowseGlobal)
	ON_EN_CHANGE(IDC_EXPORT_FRAMESPERBLOCK, OnChangeExportFramesperblock)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportParams message handlers

void CExportParams::OnExportBrowseGAMECUBE() 
{
	// TODO: Add your control notification handler code here
	
	CFileDialog fd( FALSE, NULL, NULL ) ;

	fd.m_ofn.lpstrTitle = "Browse for GameCube Export File" ;
	fd.m_ofn.lpstrFilter = "All Files (*.*)\000*.*\000" ;
	if( fd.DoModal() == IDOK )
	{
		m_FileGAMECUBE = fd.GetPathName( ) ;
		UpdateData( FALSE ) ;
	}
}

void CExportParams::OnExportBrowsePC() 
{
	// TODO: Add your control notification handler code here
	
	CFileDialog fd( FALSE, NULL, NULL ) ;

	fd.m_ofn.lpstrTitle = "Browse for PC Export File" ;
	fd.m_ofn.lpstrFilter = "All Files (*.*)\000*.*\000" ;
	if( fd.DoModal() == IDOK )
	{
		m_FilePC = fd.GetPathName( ) ;
		UpdateData( FALSE ) ;
	}
}

void CExportParams::OnExportBrowseXBOX() 
{
	// TODO: Add your control notification handler code here
	
	CFileDialog fd( FALSE, NULL, NULL ) ;

	fd.m_ofn.lpstrTitle = "Browse for XBox Export File" ;
	fd.m_ofn.lpstrFilter = "All Files (*.*)\000*.*\000" ;
	if( fd.DoModal() == IDOK )
	{
		m_FileXBOX = fd.GetPathName( ) ;
		UpdateData( FALSE ) ;
	}
}

void CExportParams::OnExportBrowsePS2() 
{
	// TODO: Add your control notification handler code here
	
	CFileDialog fd( FALSE, NULL, NULL ) ;

	fd.m_ofn.lpstrTitle = "Browse for PS2 Export File" ;
	fd.m_ofn.lpstrFilter = "All Files (*.*)\000*.*\000" ;
	if( fd.DoModal() == IDOK )
	{
		m_FilePS2 = fd.GetPathName( ) ;
		UpdateData( FALSE ) ;
	}
}

void CExportParams::OnExportEnableGAMECUBE() 
{
	// TODO: Add your control notification handler code here
	
}

void CExportParams::OnExportEnablePC() 
{
	// TODO: Add your control notification handler code here
	
}

void CExportParams::OnExportEnableXBOX() 
{
	// TODO: Add your control notification handler code here
	
}

void CExportParams::OnExportEnablePS2() 
{
	// TODO: Add your control notification handler code here
	
}

void CExportParams::OnExportBrowseGlobal() 
{
	// TODO: Add your control notification handler code here
	
	CFileDialog fd( FALSE, NULL, NULL ) ;

	fd.m_ofn.lpstrTitle = "Browse for Character Include Export File" ;
	fd.m_ofn.lpstrFilter = "All Files (*.*)\000*.*\000" ;
	if( fd.DoModal() == IDOK )
	{
		m_CharacterIncludeFile = fd.GetPathName( ) ;
		UpdateData( FALSE ) ;
	}
}

void CExportParams::OnChangeExportFramesperblock() 
{
		
}

void CExportParams::OnButton1() 
{
	// TODO: Add your control notification handler code here
	
}
