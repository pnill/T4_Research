// DlgMovieMerge.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "DlgMovieMerge.h"
#include "CeDDoc.h"
#include "Movie.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgMovieMerge dialog


CDlgMovieMerge::CDlgMovieMerge(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgMovieMerge::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgMovieMerge)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pDoc			= NULL ;
	m_pMovie		= NULL ;
	m_pMergeMovie	= NULL ;
}


void CDlgMovieMerge::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgMovieMerge)
	DDX_Control(pDX, IDC_COMBO1, m_Combo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgMovieMerge, CDialog)
	//{{AFX_MSG_MAP(CDlgMovieMerge)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnSelchangeCombo1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgMovieMerge message handlers

BOOL CDlgMovieMerge::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	//---	Initialize Combo
	int Index = m_Combo.AddString( "<no movie>" ) ;

	if( m_pDoc )
	{
		POSITION Pos = m_pDoc->m_MovieList.GetHeadPosition( ) ;
		while( Pos )
		{
			CMovie *pMovie = m_pDoc->m_MovieList.GetNext( Pos ) ;
			if( pMovie != m_pMovie )
			{
				Index = m_Combo.AddString( pMovie->GetName() ) ;
				m_Combo.SetItemData( Index, (u32)pMovie ) ;
			}
		}
	}
	m_Combo.SetCurSel( 0 ) ;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgMovieMerge::OnSelchangeCombo1() 
{
	// TODO: Add your control notification handler code here
	m_pMergeMovie = (CMovie*)m_Combo.GetItemData( m_Combo.GetCurSel() ) ;
}
