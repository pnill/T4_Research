// DlgFilePaths.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "CeDDoc.h"
#include "DlgFilePaths.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgFilePaths dialog


CDlgFilePaths::CDlgFilePaths(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgFilePaths::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgFilePaths)
	m_ReplaceString = _T("");
	m_SearchString = _T("");
	//}}AFX_DATA_INIT
}


void CDlgFilePaths::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgFilePaths)
	DDX_Text(pDX, IDC_FILEPATH_REPLACE, m_ReplaceString);
	DDX_Text(pDX, IDC_FILEPATH_SEARCH, m_SearchString);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgFilePaths, CDialog)
	//{{AFX_MSG_MAP(CDlgFilePaths)
	ON_BN_CLICKED(IDC_RADIO_ANYWHERE, OnRadioAnywhere)
	ON_BN_CLICKED(IDC_RADIO_FROMEND, OnRadioFromend)
	ON_BN_CLICKED(IDC_RADIO_FROMSTART, OnRadioFromstart)
	ON_BN_CLICKED(IDC_BROWSESEARCH, OnBrowsesearch)
	ON_BN_CLICKED(IDC_BROWSEREPLACE, OnBrowsereplace)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgFilePaths message handlers

BOOL CDlgFilePaths::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetRadioButtons( FILEPATHS_LOOKANYWHERE );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgFilePaths::SetRadioButtons( int Where )
{
	m_WhereToLook = Where;
	((CButton*)GetDlgItem( IDC_RADIO_FROMSTART ))->SetCheck( Where == FILEPATHS_LOOKFROMSTART ? TRUE : FALSE );
	((CButton*)GetDlgItem( IDC_RADIO_FROMEND ))->SetCheck( Where == FILEPATHS_LOOKFROMEND ? TRUE : FALSE );
	((CButton*)GetDlgItem( IDC_RADIO_ANYWHERE ))->SetCheck( Where == FILEPATHS_LOOKANYWHERE ? TRUE : FALSE );
}

void CDlgFilePaths::OnRadioAnywhere() 
{
	SetRadioButtons( FILEPATHS_LOOKANYWHERE );
}

void CDlgFilePaths::OnRadioFromend() 
{
	SetRadioButtons( FILEPATHS_LOOKFROMEND );
}

void CDlgFilePaths::OnRadioFromstart() 
{
	SetRadioButtons( FILEPATHS_LOOKFROMSTART );
}

void CDlgFilePaths::OnBrowsesearch() 
{
	CFileDialog fd( TRUE, NULL, "choose_a_file_or_path" ) ;

	fd.m_ofn.lpstrTitle = "Choose File Path to be replaced" ;
	fd.m_ofn.lpstrFilter = "All Files (*.*)\000*.*\000" ;

	if( fd.DoModal() == IDOK )
	{
		CString Text = fd.GetPathName();
		if( Text.Find( "choose_a_file_or_path", 0 ) != -1 )
			Text = Text.Left( Text.GetLength() - strlen( "choose_a_file_or_path" ) );
		SetDlgItemText( IDC_FILEPATH_SEARCH, Text );
	}
}

void CDlgFilePaths::OnBrowsereplace() 
{
	CFileDialog fd( TRUE, NULL, "choose_a_file_or_path" ) ;

	fd.m_ofn.lpstrTitle = "Choose Replacement File Path" ;
	fd.m_ofn.lpstrFilter = "All Files (*.*)\000*.*\000" ;

	if( fd.DoModal() == IDOK )
	{
		CString Text = fd.GetPathName();
		if( Text.Find( "choose_a_file_or_path", 0 ) != -1 )
			Text = Text.Left( Text.GetLength() - strlen( "choose_a_file_or_path" ) );
		SetDlgItemText( IDC_FILEPATH_REPLACE, Text );
	}
}
