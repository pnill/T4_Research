// EditStringDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AnimTblEd.h"
#include "EditStringDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditStringDlg dialog


CEditStringDlg::CEditStringDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEditStringDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditStringDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditStringDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditStringDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditStringDlg, CDialog)
	//{{AFX_MSG_MAP(CEditStringDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditStringDlg message handlers



BOOL CEditStringDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	if (!m_Title.IsEmpty())
		this->SetWindowText( m_Title );

	if (!m_Description.IsEmpty())
		this->SetDlgItemText( IDC_STATIC_DESCRIPTION, m_Description );
	else if (m_EditString.IsEmpty())
		this->SetDlgItemText( IDC_STATIC_DESCRIPTION, "Enter a string." );
	else
		this->SetDlgItemText( IDC_STATIC_DESCRIPTION, "Edit the string." );

	if (!m_EditString.IsEmpty())
		this->SetDlgItemText( IDC_EDIT_STRING, m_EditString );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditStringDlg::OnOK() 
{
	// TODO: Add extra validation here
	this->GetDlgItemText( IDC_EDIT_STRING, m_EditString );
	
	CDialog::OnOK();
}
