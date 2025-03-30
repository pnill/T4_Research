// TextEntryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "memcardwiz.h"
#include "TextEntryDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextEntryDlg dialog


CTextEntryDlg::CTextEntryDlg(int MaxChars, const char* pWindowText, CWnd* pParent /*=NULL*/)
	: CDialog(CTextEntryDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTextEntryDlg)
	m_TextEntry = _T("");
	//}}AFX_DATA_INIT

	m_MaxChars = MaxChars;

	if( pWindowText != NULL )
		m_TitleText = pWindowText;
}

//==========================================================================

void CTextEntryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTextEntryDlg)
	DDX_Text(pDX, IDC_EDIT_TEXTENTRY, m_TextEntry);
	//}}AFX_DATA_MAP
}

////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CTextEntryDlg, CDialog)
	//{{AFX_MSG_MAP(CTextEntryDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextEntryDlg message handlers

BOOL CTextEntryDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	this->SetWindowText( m_TitleText );

	((CEdit*)GetDlgItem( IDC_EDIT_TEXTENTRY ))->SetLimitText( m_MaxChars );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//==========================================================================

void CTextEntryDlg::OnOK( void )
{
	CDialog::OnOK();
}

//==========================================================================
