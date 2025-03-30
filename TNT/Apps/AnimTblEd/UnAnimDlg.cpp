// UnAnimDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AnimTblEd.h"
#include "UnAnimDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUnAnimDlg dialog


CUnAnimDlg::CUnAnimDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUnAnimDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUnAnimDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CUnAnimDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUnAnimDlg)
	DDX_Control(pDX, IDC_LIST, m_UnassignedList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUnAnimDlg, CDialog)
	//{{AFX_MSG_MAP(CUnAnimDlg)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUnAnimDlg message handlers

BOOL CUnAnimDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CUnAnimDlg::AddListItem(const char *lpszID)
{
	m_UnassignedList.AddString( lpszID );
}

void CUnAnimDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	//---	Resize the List Box window.
	CWnd	*pListBoxWnd;
	CRect	aRect;
	CRect	DlgRect;

	pListBoxWnd = this->GetDlgItem( IDC_LIST );
	if (pListBoxWnd)
	{
		pListBoxWnd->GetWindowRect( &aRect );
		this->GetClientRect( &DlgRect );

		pListBoxWnd->MoveWindow( 12, 12, cx - 24, cy - 24, TRUE );
	}
}


