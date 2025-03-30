// NumFrames.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "NumFrames.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNumFrames dialog


CNumFrames::CNumFrames(CWnd* pParent /*=NULL*/)
	: CDialog(CNumFrames::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNumFrames)
	m_NumFrames = 0;
	//}}AFX_DATA_INIT
}


void CNumFrames::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNumFrames)
	DDX_Text(pDX, IDC_EDIT1, m_NumFrames);
	DDV_MinMaxInt(pDX, m_NumFrames, 0, 10000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNumFrames, CDialog)
	//{{AFX_MSG_MAP(CNumFrames)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNumFrames message handlers

BOOL CNumFrames::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	CWnd *pFocus = GetDlgItem( IDC_EDIT1 ) ;
	if( pFocus )
		pFocus->SetFocus( ) ;
	return FALSE ;

//	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
