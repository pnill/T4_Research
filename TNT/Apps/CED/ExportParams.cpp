// ExportParams.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "ExportParams.h"
#include "DlgExportRotationsHelp.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
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
	m_ExportDataFile = FALSE;
	m_ExportHeaderFile = FALSE;
	m_ExportTextFile = FALSE;
	m_ExportHeaderFilePath = FALSE;
	m_bExportQuatW = FALSE;
	m_bExportGlobalRots = FALSE;
	m_bExportPropQuatBlend = FALSE;
	m_MotionPrefix = _T("");
	m_EventPrefix = _T("");
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
	DDX_Text(pDX, IDC_EXPORT_FRAMESPERBLOCK, m_FramesPerBlock);
	DDV_MinMaxInt(pDX, m_FramesPerBlock, 4, 1000);
	DDX_Check(pDX, IDC_EXPORTDATAFILE, m_ExportDataFile);
	DDX_Check(pDX, IDC_EXPORTHEADERFILE, m_ExportHeaderFile);
	DDX_Check(pDX, IDC_EXPORTTEXTFILE, m_ExportTextFile);
	DDX_Check(pDX, IDC_EXPORTHEADERPATHNAME, m_ExportHeaderFilePath);
	DDX_Check(pDX, IDC_QUATERNION, m_bExportQuaternion);
	DDX_Check(pDX, IDC_WORLDROTS, m_bExportWorldRots);
	DDX_Check(pDX, IDC_QUATBLENDING, m_bExportQuatBlend);
	DDX_Check(pDX, IDC_QUATW_ON, m_bExportQuatW);
	DDX_Check(pDX, IDC_GLOBALROTS, m_bExportGlobalRots);
	DDX_Check(pDX, IDC_QUATBLENDINGFORPROPS, m_bExportPropQuatBlend);
	DDX_Text(pDX, IDC_MOTIONPREFIX, m_MotionPrefix);
	DDX_Text(pDX, IDC_EVENTPREFIX, m_EventPrefix);
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
	ON_BN_CLICKED(IDC_EXPORTDATAFILE, OnExportdatafile)
	ON_BN_CLICKED(IDC_EXPORTHEADERFILE, OnExportheaderfile)
	ON_BN_CLICKED(IDC_EXPORTTEXTFILE, OnExporttextfile)
	ON_BN_CLICKED(IDC_EXPORTHEADERPATHNAME, OnExportheaderpathname)
	ON_BN_CLICKED(IDC_SAVENOEXPORT, OnSavenoexport)
	ON_BN_CLICKED(IDC_EXPORTNOSAVE, OnExportnosave)
	ON_BN_CLICKED(IDC_QUATERNION, OnQuaternion)
	ON_BN_CLICKED(IDC_WORLDROTS, OnWorldrots)
	ON_BN_CLICKED(IDC_QUATBLENDING, OnQuatblending)
	ON_BN_CLICKED(IDC_QUATW_ON, OnQuatbitsOn)
	ON_BN_CLICKED(IDC_GLOBALROTS, OnGlobalrots)
	ON_BN_CLICKED(IDC_BONEROTATIONHELP, OnBonerotationhelp)
	ON_BN_CLICKED(IDC_QUATBLENDINGFORPROPS, OnQuatblendingforprops)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportParams message handlers

BOOL CExportParams::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_bNoExport	= FALSE;
	m_bNoSave	= FALSE;

	//m_ExportDataFile	= TRUE;
	//m_ExportHeaderFile	= TRUE;
	//m_ExportTextFile	= FALSE;
	//m_ExportHeaderFilePath	= TRUE;
	SetDlgItemInt( IDC_EXPORT_FRAMESPERBLOCK, m_FramesPerBlock );

	((CButton*)GetDlgItem( IDC_EXPORTDATAFILE ))->SetCheck( m_ExportDataFile ? TRUE : FALSE );
	((CButton*)GetDlgItem( IDC_EXPORTHEADERFILE ))->SetCheck( m_ExportHeaderFile ? TRUE : FALSE );
	((CButton*)GetDlgItem( IDC_EXPORTTEXTFILE ))->SetCheck( m_ExportTextFile ? TRUE : FALSE );
	((CButton*)GetDlgItem( IDC_EXPORTHEADERPATHNAME ))->SetCheck( m_ExportHeaderFilePath ? TRUE : FALSE );

	((CButton*)GetDlgItem( IDC_QUATERNION ))->SetCheck( m_bExportQuaternion ? TRUE : FALSE );
	((CButton*)GetDlgItem( IDC_WORLDROTS ))->SetCheck( m_bExportWorldRots ? TRUE : FALSE );
	((CButton*)GetDlgItem( IDC_GLOBALROTS ))->SetCheck( m_bExportGlobalRots ? TRUE : FALSE );
	((CButton*)GetDlgItem( IDC_QUATBLENDING ))->SetCheck( m_bExportQuatBlend ? TRUE : FALSE );
	((CButton*)GetDlgItem( IDC_QUATBLENDINGFORPROPS ))->SetCheck( m_bExportPropQuatBlend ? TRUE : FALSE );
	((CButton*)GetDlgItem( IDC_QUATW_ON ))->SetCheck( m_bExportQuatW ? TRUE : FALSE );
	OnQuaternion();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

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
	
}

void CExportParams::OnExportdatafile() 
{
	m_ExportDataFile	= ((CButton*)GetDlgItem( IDC_EXPORTDATAFILE ))->GetCheck() ? TRUE : FALSE;
}

void CExportParams::OnExportheaderfile() 
{
	m_ExportHeaderFile	= ((CButton*)GetDlgItem( IDC_EXPORTHEADERFILE ))->GetCheck() ? TRUE : FALSE;
}

void CExportParams::OnExporttextfile() 
{
	m_ExportTextFile	= ((CButton*)GetDlgItem( IDC_EXPORTTEXTFILE ))->GetCheck() ? TRUE : FALSE;
}

void CExportParams::OnExportheaderpathname() 
{
	m_ExportHeaderFilePath	= ((CButton*)GetDlgItem( IDC_EXPORTHEADERPATHNAME ))->GetCheck() ? TRUE : FALSE;
}

void CExportParams::OnSavenoexport() 
{
	m_bNoExport = TRUE;
	OnOK();
}

void CExportParams::OnExportnosave() 
{
	m_bNoSave	= TRUE;
	OnOK();
}

CExportParams& CExportParams::operator= ( CExportParams& ep )
{
	m_CharacterIncludeFile	= ep.m_CharacterIncludeFile ;
	m_EnableGAMECUBE		= !!ep.m_EnableGAMECUBE ;
	m_EnablePC				= !!ep.m_EnablePC ;
	m_EnableXBOX			= !!ep.m_EnableXBOX ;
	m_EnablePS2				= !!ep.m_EnablePS2 ;
	m_FilePC				= ep.m_FilePC ;
	m_FileXBOX				= ep.m_FileXBOX ;
	m_FilePS2				= ep.m_FilePS2 ;
	m_OptForceUpper			= !!ep.m_OptForceUpper ;
	m_OptCollisionBubbles	= !!ep.m_OptCollisionBubbles ;
	m_OptMotionFlags		= !!ep.m_OptMotionFlags ;
	m_ExportDataFile		= !!ep.m_ExportDataFile ;
	m_ExportHeaderFile		= !!ep.m_ExportHeaderFile ;
	m_ExportTextFile		= !!ep.m_ExportTextFile ;
	m_ExportHeaderFilePath	= !!ep.m_ExportHeaderFilePath ;
	m_bExportQuaternion		= !!ep.m_bExportQuaternion ;
	m_bExportWorldRots		= !!ep.m_bExportWorldRots ;
	m_bExportGlobalRots		= !!ep.m_bExportGlobalRots ;
	m_bExportQuatBlend		= !!ep.m_bExportQuatBlend ;
	m_bExportQuatW			= !!ep.m_bExportQuatW ;
	m_MotionPrefix			= ep.m_MotionPrefix;
	m_EventPrefix			= ep.m_EventPrefix;
	return *this;
}

void CExportParams::OnQuaternion() 
{
	bool bEnabled = ((CButton*)GetDlgItem( IDC_QUATERNION ))->GetCheck() ? TRUE : FALSE;

	GetDlgItem( IDC_QUATBLENDING )->EnableWindow( !bEnabled );
	GetDlgItem( IDC_QUATBLENDINGFORPROPS )->EnableWindow( !bEnabled );
	GetDlgItem( IDC_QUATW_ON )->EnableWindow( bEnabled );

	if( bEnabled )
		OnQuatbitsOn();
}

void CExportParams::OnWorldrots() 
{
	
}

void CExportParams::OnQuatblending() 
{
	
}

void CExportParams::OnQuatblendingforprops() 
{
	
}

void CExportParams::OnQuatbitsOn() 
{
	m_bExportQuatW = ((CButton*)GetDlgItem( IDC_QUATW_ON ))->GetCheck() ? TRUE : FALSE;
}

void CExportParams::OnGlobalrots() 
{
		
}

void CExportParams::OnBonerotationhelp() 
{
	CDlgExportRotationsHelp dlg;
	
	dlg.DoModal();
}


