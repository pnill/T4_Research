// ExportConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AnimTblEd.h"
#include "ExportConfigDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExportConfigDlg dialog


CExportConfigDlg::CExportConfigDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExportConfigDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExportConfigDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CExportConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportConfigDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExportConfigDlg, CDialog)
	//{{AFX_MSG_MAP(CExportConfigDlg)
	ON_BN_CLICKED(IDC_BUTTON_CHANGEPATH, OnButtonChangepath)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportConfigDlg message handlers

BOOL CExportConfigDlg::OnInitDialog() 
{
	CButton *pButton;

	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	//---	Name of export
	this->SetDlgItemText( IDC_EDIT_NAME, m_ExportDef.GetName() );

	//---	Path and file to export to.
	this->SetDlgItemText( IDC_EDIT_PATHFILE, m_ExportDef.GetPath() );

	//---	Enabled check box.
	pButton = ((CButton*)this->GetDlgItem( IDC_CHECK_ONOFF ));
	pButton->SetCheck( m_ExportDef.GetEnabled() );

	//---	Export data separate
	pButton = ((CButton*)this->GetDlgItem( IDC_DATASEPARATE ));
	pButton->SetCheck( m_ExportDef.ExportDataSeparately() );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CExportConfigDlg::OnOK() 
{
	CButton *pButton;
	CString String;

	// TODO: Add extra validation here
	this->GetDlgItemText( IDC_EDIT_NAME, String );
	String.TrimLeft();

	if (String.IsEmpty())
	{
		this->MessageBox( "Name must not be empty.", "Error" );
		return;
	}

	this->GetDlgItemText( IDC_EDIT_PATHFILE, String );
	String.TrimLeft();
	if (String.IsEmpty())
	{
		if (this->MessageBox( "Path not specified.  Export will be disabled.", "Error" ) == IDOK)
			m_ExportDef.SetEnabled( FALSE );
		else
			return;
	}

	//---	Move the data from the dialog back into the ExportDef structure.
	this->GetDlgItemText( IDC_EDIT_NAME, String );
	String.TrimLeft();
	m_ExportDef.SetName( String );

	this->GetDlgItemText( IDC_EDIT_PATHFILE, String );
	String.TrimLeft();
	m_ExportDef.SetPath( String );

	//---	Build the flags
	m_ExportDef.SetEnabled( ((CButton*)this->GetDlgItem( IDC_CHECK_ONOFF ))->GetCheck() );

	//---	Export data separate
	pButton = ((CButton*)this->GetDlgItem( IDC_DATASEPARATE ));
	m_ExportDef.ExportDataSeparately( pButton->GetCheck() );

	CDialog::OnOK();
}

void CExportConfigDlg::OnButtonChangepath() 
{
	// TODO: Add your control notification handler code here
	CString			sFilter = "C Include Files (*.h)||";
	CString			PathFile;

	this->GetDlgItemText( IDC_EDIT_PATHFILE, PathFile );

 	CFileDialog		FileDlg( TRUE, "*.h", PathFile, 0, sFilter, this );
 
 	if (FileDlg.DoModal() == IDOK)
 	{
 
 		PathFile = FileDlg.GetPathName();
 
		this->SetDlgItemText( IDC_EDIT_PATHFILE, PathFile );
 	}
}
