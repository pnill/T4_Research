// ViewPropertiesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AnimTblEd.h"
#include "ViewPropertiesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

char* g_VPOptionStrings[NUM_VP_OPTIONS] =
{
	"Name",
	"Left/Right IDs",
	"Priority",
	"Link",
	"Method",
	"Flags",
	"States",
	"Comment",
};


/////////////////////////////////////////////////////////////////////////////
// CViewPropertiesDlg dialog


CViewPropertiesDlg::CViewPropertiesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CViewPropertiesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CViewPropertiesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CViewPropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CViewPropertiesDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CViewPropertiesDlg, CDialog)
	//{{AFX_MSG_MAP(CViewPropertiesDlg)
	ON_BN_CLICKED(IDC_VP_ADD, OnVpAdd)
	ON_BN_CLICKED(IDC_VP_REMOVE, OnVpRemove)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewPropertiesDlg message handlers

BOOL CViewPropertiesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CListBox* pListBox = (CListBox*) GetDlgItem( IDC_VP_OPTIONS );
	int i, index;

	//---	add the options to the option list
	for( i=0; i<NUM_VP_OPTIONS; i++ )
		pListBox->AddString( g_VPOptionStrings[ i ] ); 

	pListBox = (CListBox*) GetDlgItem( IDC_VP_DISPLAYED );

	//---	add the selected options to the displayed list
	for( i=0; i<NUM_VP_OPTIONS; i++ )
	{
		index = m_VPDisplayOrder[ i ];
		if (index != VP_OPTION_NONE)
			pListBox->AddString( g_VPOptionStrings[ index ] ); 
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CViewPropertiesDlg::OnOK()
{
	int i, j, c;
	CListBox* pListBox = (CListBox*) GetDlgItem( IDC_VP_DISPLAYED );
	CString Option;

	c = 0;

	//---	record the new view options
	for( i=0; i<pListBox->GetCount(); i++ )
	{
		//---	get the options string
		pListBox->GetText( i, Option );

		//---	find option by comparing strings
		for( j=0; j<NUM_VP_OPTIONS; j++ )
		{
			if (Option == g_VPOptionStrings[ j ])
				m_VPDisplayOrder[ c++ ] = j;
		}
	}

	//---	clear the rest of the option view list	
	for(; c<NUM_VP_OPTIONS; c++ )
		m_VPDisplayOrder[ c ] = VP_OPTION_NONE;

	CDialog::OnOK();
}

void CViewPropertiesDlg::OnVpAdd() 
{
	CListBox* pListBox = (CListBox*) GetDlgItem( IDC_VP_OPTIONS );
	CString Option, Displayed;
	int i;

	i = pListBox->GetCurSel();
	if (i == -1)
		return;

	pListBox->GetText( i, Option );

	pListBox = (CListBox*) GetDlgItem( IDC_VP_DISPLAYED );

	//---	make sure that string has not already been added
	for( i=0; i<pListBox->GetCount(); i++ )
	{
		pListBox->GetText( i, Displayed );
		if (Option == Displayed)
			break;
	}

	if (i == pListBox->GetCount())
		pListBox->InsertString( pListBox->GetCurSel(), Option );
}

void CViewPropertiesDlg::OnVpRemove() 
{
	CListBox* pListBox = (CListBox*) GetDlgItem( IDC_VP_DISPLAYED );
	pListBox->DeleteString( pListBox->GetCurSel() );
}
