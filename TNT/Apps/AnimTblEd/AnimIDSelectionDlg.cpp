// AnimIDSelectionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AnimTblEd.h"
#include "AnimIDSelectionDlg.h"
#include "AnimID1.h"
#include "AnimTblEdGlobals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


enum
{
	DISPLAY_IDS_ALL,
	DISPLAY_IDS_ANIMS,
	DISPLAY_IDS_GROUPS,

	DISPLAY_TABS_COUNT,
};

static char* DisplayTabsStrings[DISPLAY_TABS_COUNT] =
{
	"ALL",
	"ANIMATIONS",
	"GROUPS",
};


int CAnimIDSelectionDlg::m_Mode = DISPLAY_IDS_ALL;


/////////////////////////////////////////////////////////////////////////////
// CAnimIDSelectionDlg dialog

CAnimIDSelectionDlg::CAnimIDSelectionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAnimIDSelectionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAnimIDSelectionDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAnimIDSelectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAnimIDSelectionDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAnimIDSelectionDlg, CDialog)
	//{{AFX_MSG_MAP(CAnimIDSelectionDlg)
	ON_LBN_DBLCLK(IDC_LIST_ANIMATIONS, OnDblclkListAnimations)
	ON_BN_CLICKED(IDC_CHECK_ANIMGROUPLIST, OnCheckAnimgrouplist)
	ON_NOTIFY(TCN_SELCHANGE, IDC_DISPLAYTAB, OnSelchangeDisplaytab)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnimIDSelectionDlg message handlers

void CAnimIDSelectionDlg::BuildList( void )
{
	CTabCtrl	*pTab;
	CListBox	*pListBox;
//	CAnimID		*pAnimID;
	int			ListFlags;

	//---	get the current selected tab
	pTab = (CTabCtrl*) GetDlgItem( IDC_DISPLAYTAB );
	m_Mode = pTab->GetCurSel();

	pListBox = (CListBox*)this->GetDlgItem( IDC_LIST_ANIMATIONS );

	//---	Make sure that the list box starts empty.
	pListBox->ResetContent();

	//---	If we have an extra choice place it at the top of the box.
	if (!m_ExtraChoice.IsEmpty())
		pListBox->AddString( m_ExtraChoice );

	//---	determine the flags for adding tree items to the list box
	ListFlags = 0;
	if ((m_Mode == DISPLAY_IDS_ALL) || (m_Mode == DISPLAY_IDS_ANIMS))
		ListFlags |= LISTBOX_FLAG_INCLUDEANIMS;
	if ((m_Mode == DISPLAY_IDS_ALL) || (m_Mode == DISPLAY_IDS_GROUPS))
		ListFlags |= LISTBOX_FLAG_INCLUDEGROUPS;

	//---	add the tree items to the list box
	m_pDoc->m_AnimTree.AddToListBox( pListBox, ListFlags );
}

BOOL CAnimIDSelectionDlg::OnInitDialog() 
{
	CTabCtrl	*pTab;
	CDialog::OnInitDialog();
	int i;

	//---	Initialize the Unassigned radio button to be selected.
	pTab = (CTabCtrl*) GetDlgItem( IDC_DISPLAYTAB );
	for( i=0; i<DISPLAY_TABS_COUNT; i++ )
		pTab->InsertItem( i, DisplayTabsStrings[i] );
	pTab->SetCurSel( m_Mode );

	// TODO: Add extra initialization here
	this->BuildList();
	
    this->GetDlgItem( IDC_LIST_ANIMATIONS )->SetFocus();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAnimIDSelectionDlg::OnDblclkListAnimations() 
{
	// TODO: Add your control notification handler code here
	this->OnOK();
}

void CAnimIDSelectionDlg::OnOK() 
{
	CListBox	*pListBox;
	int			CurSel;

	// TODO: Add extra validation here
	pListBox = (CListBox*)this->GetDlgItem( IDC_LIST_ANIMATIONS );

	CurSel = pListBox->GetCurSel();

	if (CurSel != -1)
	{
		pListBox->GetText( CurSel, m_SelectedAnim );

		//---	Mark the document as having changed.
		GBL_SetModifiedFlag( TRUE );

		CDialog::OnOK();
	}
	else
		this->MessageBox( "Must select an animation.", "Error" );
}

void CAnimIDSelectionDlg::AddExtraChoice(const char *lpszExtraChoice)
{
	m_ExtraChoice.Format( "%s", lpszExtraChoice );
}

void CAnimIDSelectionDlg::SetOmitChoice(const char *lpszAnimID)
{
	m_OmitChoice.Format( "%s", lpszAnimID );
}

void CAnimIDSelectionDlg::OnCheckAnimgrouplist() 
{
	BuildList();
}

void CAnimIDSelectionDlg::OnSelchangeDisplaytab(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CTabCtrl	*pTab = (CTabCtrl*) GetDlgItem( IDC_DISPLAYTAB );

	BuildList();

	*pResult = 0;
}
