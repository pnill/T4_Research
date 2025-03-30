// DlgMovieExport.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "DlgMovieExport.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgMovieExport dialog


CDlgMovieExport::CDlgMovieExport(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgMovieExport::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgMovieExport)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgMovieExport::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgMovieExport)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgMovieExport, CDialog)
	//{{AFX_MSG_MAP(CDlgMovieExport)
	ON_BN_CLICKED(IDC_EXPORT_ENABLE_GAMECUBE, OnExportEnableGamecube)
	ON_BN_CLICKED(IDC_EXPORT_ENABLE_PC, OnExportEnablePc)
	ON_BN_CLICKED(IDC_EXPORT_ENABLE_XBOX, OnExportEnableXbox)
	ON_BN_CLICKED(IDC_EXPORT_ENABLE_PS2, OnExportEnablePs2)
	ON_BN_CLICKED(IDC_EXPORT_BROWSE_GAMECUBE, OnExportBrowseGamecube)
	ON_BN_CLICKED(IDC_EXPORT_BROWSE_PC, OnExportBrowsePc)
	ON_BN_CLICKED(IDC_EXPORT_BROWSE_XBOX, OnExportBrowseXbox)
	ON_BN_CLICKED(IDC_EXPORT_BROWSE_PS2, OnExportBrowsePs2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgMovieExport message handlers


//==========================================================================
BOOL CDlgMovieExport::OnInitDialog() 
{
	CDialog::OnInitDialog();

	LoadFromMemberVars();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//==========================================================================
void CDlgMovieExport::OnExportEnableGamecube() 
{
	bool Enable = ((CButton*)GetDlgItem( IDC_EXPORT_ENABLE_GAMECUBE ))->GetCheck() ? TRUE : FALSE;
	GetDlgItem( IDC_EXPORT_FILE_GAMECUBE )->EnableWindow( Enable );
}

//==========================================================================
void CDlgMovieExport::OnExportEnablePc() 
{
	bool Enable = ((CButton*)GetDlgItem( IDC_EXPORT_ENABLE_PC ))->GetCheck() ? TRUE : FALSE;
	GetDlgItem( IDC_EXPORT_FILE_PC )->EnableWindow( Enable );
}

//==========================================================================
void CDlgMovieExport::OnExportEnableXbox() 
{
	bool Enable = ((CButton*)GetDlgItem( IDC_EXPORT_ENABLE_XBOX ))->GetCheck() ? TRUE : FALSE;
	GetDlgItem( IDC_EXPORT_FILE_XBOX )->EnableWindow( Enable );
}

//==========================================================================
void CDlgMovieExport::OnExportEnablePs2() 
{
	bool Enable = ((CButton*)GetDlgItem( IDC_EXPORT_ENABLE_PS2 ))->GetCheck() ? TRUE : FALSE;
	GetDlgItem( IDC_EXPORT_FILE_PS2 )->EnableWindow( Enable );
}

//==========================================================================
void CDlgMovieExport::OnExportBrowseGamecube() 
{
	CString Path;

	GetDlgItemText( IDC_EXPORT_FILE_GAMECUBE, Path );

	if( Browse( Path ) )
		SetDlgItemText( IDC_EXPORT_FILE_GAMECUBE, Path );
}

//==========================================================================
void CDlgMovieExport::OnExportBrowsePc() 
{
	CString Path;

	GetDlgItemText( IDC_EXPORT_FILE_PC, Path );

	if( Browse( Path ) )
		SetDlgItemText( IDC_EXPORT_FILE_PC, Path );
}

//==========================================================================
void CDlgMovieExport::OnExportBrowseXbox() 
{
	CString Path;

	GetDlgItemText( IDC_EXPORT_FILE_XBOX, Path );

	if( Browse( Path ) )
		SetDlgItemText( IDC_EXPORT_FILE_XBOX, Path );
}

//==========================================================================
void CDlgMovieExport::OnExportBrowsePs2() 
{
	CString Path;

	GetDlgItemText( IDC_EXPORT_FILE_PS2, Path );

	if( Browse( Path ) )
		SetDlgItemText( IDC_EXPORT_FILE_PS2, Path );
}

//==========================================================================
bool CDlgMovieExport::Browse( CString& Path )
{
	CString File = Path;
	CString Root;
	CString FileTypes;

	int index = File.ReverseFind('\\');
	if( index == -1 )
	{
		index = File.ReverseFind('/');
		if( index == -1 )
			index = File.ReverseFind(':');
	}

	if( index != -1 )
		File = File.Right( File.GetLength() - index - 1 );

	Root = Path.Left( index + 1 );

	FileTypes = "Movie File (*.MVI)|*.mvi\0";

	//---	browse for the aiff file
	CFileDialog Dialog( TRUE, "MVI", File, 0, FileTypes, NULL );

	Dialog.m_ofn.lpstrInitialDir	= Root;
//	Dialog.m_ofn.Flags				|= OFN_NOCHANGEDIR;
	Dialog.m_ofn.nFilterIndex		= 0;

	if (Dialog.DoModal() == IDOK)
	{
		Path = Dialog.GetPathName();
		return TRUE;
	}

	return FALSE;
}

//==========================================================================
void CDlgMovieExport::OnOK() 
{
	// TODO: Add extra validation here
	StoreToMemberVars();
	
	CDialog::OnOK();
}

//==========================================================================
void CDlgMovieExport::LoadFromMemberVars()
{
	int i;
	for( i=0; i<NUM_EXPORT_TYPES; i++ )
	{
		switch(i)
		{
		case EXPORT_TYPE_GAMECUBE:
			((CButton*)GetDlgItem( IDC_EXPORT_ENABLE_GAMECUBE ))->SetCheck( m_ExportActive[i] ? TRUE : FALSE );
			SetDlgItemText( IDC_EXPORT_FILE_GAMECUBE, m_ExportPaths[i] );
			GetDlgItem( IDC_EXPORT_FILE_GAMECUBE )->EnableWindow( m_ExportActive[i] ? TRUE : FALSE );
			break;

		case EXPORT_TYPE_PC:
			((CButton*)GetDlgItem( IDC_EXPORT_ENABLE_PC ))->SetCheck( m_ExportActive[i] ? TRUE : FALSE );
			SetDlgItemText( IDC_EXPORT_FILE_PC, m_ExportPaths[i] );
			GetDlgItem( IDC_EXPORT_FILE_PC )->EnableWindow( m_ExportActive[i] ? TRUE : FALSE );
			break;

		case EXPORT_TYPE_XBOX:
			((CButton*)GetDlgItem( IDC_EXPORT_ENABLE_XBOX ))->SetCheck( m_ExportActive[i] ? TRUE : FALSE );
			SetDlgItemText( IDC_EXPORT_FILE_XBOX, m_ExportPaths[i] );
			GetDlgItem( IDC_EXPORT_FILE_XBOX )->EnableWindow( m_ExportActive[i] ? TRUE : FALSE );
			break;

		case EXPORT_TYPE_PS2:
			((CButton*)GetDlgItem( IDC_EXPORT_ENABLE_PS2 ))->SetCheck( m_ExportActive[i] ? TRUE : FALSE );
			SetDlgItemText( IDC_EXPORT_FILE_PS2, m_ExportPaths[i] );
			GetDlgItem( IDC_EXPORT_FILE_PS2 )->EnableWindow( m_ExportActive[i] ? TRUE : FALSE );
			break;

		}
	}
}

//==========================================================================
void CDlgMovieExport::StoreToMemberVars()
{
	int i;
	for( i=0; i<NUM_EXPORT_TYPES; i++ )
	{
		switch(i)
		{
		case EXPORT_TYPE_GAMECUBE:
			m_ExportActive[i] = ((CButton*)GetDlgItem( IDC_EXPORT_ENABLE_GAMECUBE ))->GetCheck() ? TRUE : FALSE ;
			GetDlgItemText( IDC_EXPORT_FILE_GAMECUBE, m_ExportPaths[i] );
			break;

		case EXPORT_TYPE_PC:
			m_ExportActive[i] = ((CButton*)GetDlgItem( IDC_EXPORT_ENABLE_PC ))->GetCheck() ? TRUE : FALSE ;
			GetDlgItemText( IDC_EXPORT_FILE_PC, m_ExportPaths[i] );
			break;

		case EXPORT_TYPE_XBOX:
			m_ExportActive[i] = ((CButton*)GetDlgItem( IDC_EXPORT_ENABLE_XBOX ))->GetCheck() ? TRUE : FALSE ;
			GetDlgItemText( IDC_EXPORT_FILE_XBOX, m_ExportPaths[i] );
			break;

		case EXPORT_TYPE_PS2:
			m_ExportActive[i] = ((CButton*)GetDlgItem( IDC_EXPORT_ENABLE_PS2 ))->GetCheck() ? TRUE : FALSE ;
			GetDlgItemText( IDC_EXPORT_FILE_PS2, m_ExportPaths[i] );
			break;

		}
	}
}
