// SourceIDDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AnimTblEd.h"
#include "SourceIDDlg.h"
#include "EditStringDlg.h"
#include "AnimTblEdGlobals.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum
{
	DISPLAY_IDS_ALL,
	DISPLAY_IDS_ASSIGNED,
	DISPLAY_IDS_UNASSIGNED,

	DISPLAY_TABS_COUNT,
};

static char* DisplayTabsStrings[DISPLAY_TABS_COUNT] =
{
	"ALL",
	"ASSIGNED",
	"UNASSIGNED",
};

int CSourceIDDlg::m_Mode = DISPLAY_IDS_ALL;

/////////////////////////////////////////////////////////////////////////////
// CSourceIDDlg dialog


CSourceIDDlg::CSourceIDDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSourceIDDlg::IDD, pParent)
{
	m_MultipleSelections = FALSE;
	m_NumSelections = 0;

	//{{AFX_DATA_INIT(CSourceIDDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSourceIDDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSourceIDDlg)
	DDX_Control(pDX, IDC_SOURCEIDLISTMULT, m_SourceIDListMult);
	DDX_Control(pDX, IDC_SOURCEIDLIST, m_SourceIDList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSourceIDDlg, CDialog)
	//{{AFX_MSG_MAP(CSourceIDDlg)
	ON_BN_CLICKED(IDC_RADIOASSIGNED, OnRadioassigned)
	ON_BN_CLICKED(IDC_RADIOUNASSIGNED, OnRadiounassigned)
	ON_LBN_DBLCLK(IDC_SOURCEIDLIST, OnDblclkSourceidlist)
	ON_BN_CLICKED(IDC_CHECK_ENABLEFILTERS, OnCheckEnablefilters)
	ON_BN_CLICKED(IDC_BUTTON_NEWFILTER, OnButtonNewfilter)
	ON_BN_CLICKED(IDC_BUTTON_DELETEFILTER, OnButtonDeletefilter)
	ON_LBN_SELCHANGE(IDC_LIST_FILTER, OnSelchangeListFilter)
	ON_LBN_SELCANCEL(IDC_LIST_FILTER, OnSelcancelListFilter)
	ON_NOTIFY(TCN_SELCHANGE, IDC_DISPLAYTAB, OnSelchangeDisplaytab)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSourceIDDlg message handlers

BOOL CSourceIDDlg::OnInitDialog() 
{
	CTabCtrl* pTab;
	CString	IDName;
	int		sel = 0;
	int		i;

	CDialog::OnInitDialog();
	
	//---	Set the id window to have focus at startup.
	if( m_MultipleSelections )
	{
		GetDlgItem( IDC_SOURCEIDLISTMULT )->SetFocus();
		GetDlgItem( IDC_SOURCEIDLIST )->EnableWindow( FALSE );
		GetDlgItem( IDC_SOURCEIDLIST )->ModifyStyle( WS_VISIBLE, 0 );
		m_pSourceIDList = &m_SourceIDListMult;
	}
	else
	{
		GetDlgItem( IDC_SOURCEIDLIST )->SetFocus();
		GetDlgItem( IDC_SOURCEIDLISTMULT )->EnableWindow( FALSE );
		GetDlgItem( IDC_SOURCEIDLISTMULT )->ModifyStyle( WS_VISIBLE, 0 );
		m_pSourceIDList = &m_SourceIDList;
	}

	// TODO: Add extra initialization here
	m_Modified = FALSE;

	//---	Initialize the Unassigned radio button to be selected.
	pTab = (CTabCtrl*) GetDlgItem( IDC_DISPLAYTAB );
	for( i=0; i<DISPLAY_TABS_COUNT; i++ )
		pTab->InsertItem( i, DisplayTabsStrings[i] );
	pTab->SetCurSel( m_Mode );

	//---	Build the list of possible identifiers.
	BuildList();

	//---	Try to find a good starting point for the list selection based on the animation.
	FindString( m_AnimID );

	//---	Set the enable filtering check box to reflect the current settings.
	((CButton*)GetDlgItem( IDC_CHECK_ENABLEFILTERS ))->SetCheck( m_pIDList->GetFilteringEnabled() );

	//---	Build the list of filters.
	BuildFilterList();

//	IDName.Format( "%s", m_pAnimNode->GetFirstAnim() );
//	while (!IDName.IsEmpty() || (IDName.Compare( "(null)" ) != 0))
//	{
//		sel = m_pSourceIDList->FindString( 0, IDName );
//	}
//	m_pSourceIDList->SetCurSel( sel );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSourceIDDlg::OnOK() 
{
	CString	NewID;
	int		CurSel;
	int		i;
	int		SelItems[MAX_SELECTIONS];

    //---   get the current selected item
	CurSel = m_pSourceIDList->GetCurSel();

	if (!m_MultipleSelections && (CurSel >= 0))
	{
		m_pSourceIDList->GetText( CurSel, NewID );

		m_Modified = (m_AnimID != NewID) ? TRUE : FALSE;
		m_AnimID = NewID;
    }
    else if (m_MultipleSelections && (m_pSourceIDList->GetSelCount() > 0))
    {
		//---	fill the multi-select array
    	m_pSourceIDList->GetSelItems( MAX_SELECTIONS, SelItems );
		m_NumSelections = m_pSourceIDList->GetSelCount();
		for( i=0; i<m_NumSelections; i++ )
		{
			m_pSourceIDList->GetText( SelItems[ i ], NewID );
			m_AnimIDs[i] = NewID;
		}

		m_Modified = TRUE;
	}
	else
		m_Modified = FALSE;

	CDialog::OnOK();
}

void CSourceIDDlg::BuildList()
{
	CAnimID		*pAnimID;
	CTabCtrl	*pTab;
	CString		TopString;
	int			TopIndex;

	pTab = (CTabCtrl*) GetDlgItem( IDC_DISPLAYTAB );
	m_Mode = pTab->GetCurSel();

	//---	If we have a list currently.
	if (m_pSourceIDList->GetCount() > 0)
	{
		//---	Get the currently selected string.
		m_pSourceIDList->GetText( m_pSourceIDList->GetTopIndex(), TopString );
		TopIndex = 1;
	}
	else
		TopIndex = 0;

	//---	Make sure that the list box starts empty.
	while (m_pSourceIDList->GetCount() > 0)
		m_pSourceIDList->DeleteString( 0 );

	//---	Get the start of the list.
	pAnimID = m_pIDList->GetHead();

	//---	Loop through the list adding them to our list box.
	while (pAnimID)
	{
		if ((m_Mode == DISPLAY_IDS_ALL)
			|| ((m_Mode == DISPLAY_IDS_ASSIGNED) && pAnimID->m_nOccurances)
			|| ((m_Mode == DISPLAY_IDS_UNASSIGNED) && !pAnimID->m_nOccurances))
			m_pSourceIDList->AddString( pAnimID->m_AnimID );
		
		pAnimID = m_pIDList->GetNext();
	}

	//---	If we had a valid top index before...
	if (TopIndex)
	{
		//---	Search for and reset the top index if it is still in the list.
		TopIndex = m_pSourceIDList->FindString( -1, TopString );
		if (TopIndex >= 0)
			m_pSourceIDList->SetTopIndex( TopIndex );
	}
}

void CSourceIDDlg::OnDblclkSourceidlist() 
{
	// TODO: Add your control notification handler code here
	CSourceIDDlg::OnOK();
}

void CSourceIDDlg::OnCheckEnablefilters() 
{
	// TODO: Add your control notification handler code here
	CButton	*pButton;

	//---	Get a pointer to the check box.
	pButton = (CButton*)this->GetDlgItem( IDC_CHECK_ENABLEFILTERS );

	//---	Set the ID list filtering flag to coincide with the check box state.
	m_pIDList->EnableFiltering( pButton->GetCheck() );

	//---	Rebuild the list with the new filter settings.
	BuildList();
}

void CSourceIDDlg::OnButtonNewfilter() 
{
	// TODO: Add your control notification handler code here
	CEditStringDlg	NewFilter;

	//---	Initialize the aspects of the string edit dialog.
	NewFilter.m_Title.Format( "New Filter" );
	NewFilter.m_Description.Format( "Create a new prefix to filter from the ID list." );
	
	//---	Execute the dialog.  If IDOK is returned then create the new filter.
	if (NewFilter.DoModal() == IDOK)
	{
		//---	If there is a string then create the new filter.
		if (!NewFilter.m_EditString.IsEmpty())
		{
			//---	Add the new filter to the list.
			m_pIDList->AddFilter( NewFilter.m_EditString );

			//---	Rebuild the list with the new filter.
			BuildList();

			//---	Rebuild the filter list
			BuildFilterList();
		}
	}
}

void CSourceIDDlg::OnButtonDeletefilter() 
{
	// TODO: Add your control notification handler code here
	CListBox	*pFilterList;
	CString		Prefix;

	//---	Get a pointer to the list box.
	pFilterList = (CListBox*)this->GetDlgItem( IDC_LIST_FILTER );

	//---	If we have a currently selected filter prefix.
	if (pFilterList->GetCurSel() != -1)
	{
		//---	Get the selected string.
		pFilterList->GetText( pFilterList->GetCurSel(), Prefix );

		//---	Remove the string from the list of filters.
		m_pIDList->RemoveFilter( Prefix );

		//---	Rebuild the list without the deleted filter.
		BuildList();

		//---	Rebuild the filter list
		BuildFilterList();
	}
}

void CSourceIDDlg::OnSelchangeListFilter() 
{
	// TODO: Add your control notification handler code here
	CButton *pButton;

	pButton = (CButton*)this->GetDlgItem( IDC_BUTTON_DELETEFILTER );

	pButton->EnableWindow( TRUE );
}

void CSourceIDDlg::OnSelcancelListFilter() 
{
	// TODO: Add your control notification handler code here
	CButton *pButton;

	pButton = (CButton*)this->GetDlgItem( IDC_BUTTON_DELETEFILTER );

	pButton->EnableWindow( FALSE );
}

void CSourceIDDlg::BuildFilterList()
{
	CAnimID		*pFilter;
	CListBox	*pFilterList;
	CString		TopString;
	int			TopIndex;

	//---	Get a pointer to the filter list.
	pFilterList = (CListBox*)GetDlgItem( IDC_LIST_FILTER );

	//---	If we have a list currently.
	if (pFilterList->GetCount() > 0)
	{
		//---	Get the currently selected string.
		pFilterList->GetText( pFilterList->GetTopIndex(), TopString );
		TopIndex = 1;
	}
	else
		TopIndex = 0;

	//---	Make sure that the list box starts empty.
	while (pFilterList->GetCount() > 0)
		pFilterList->DeleteString( 0 );

	//---	Get the start of the list.
	pFilter = m_pIDList->GetFirstFilter();

	//---	Loop through the list adding them to our list box.
	while (pFilter)
	{
		pFilterList->AddString( pFilter->m_AnimID );
		
		pFilter = m_pIDList->GetNextFilter();
	}

	//---	If we had a valid top index before...
	if (TopIndex)
	{
		//---	Search for and reset the top index if it is still in the list.
		TopIndex = pFilterList->FindString( -1, TopString );
		if (TopIndex >= 0)
			pFilterList->SetTopIndex( TopIndex );
	}
}

void CSourceIDDlg::FindString(const char *StringToFind)
{
	CString	String;
	int		FoundIndex;

	String = StringToFind;
	if (String.Left(3) != "ID_")
		String = "ID_" + String;

	while (!String.IsEmpty())
	{
		//---	Search for the string that starts with this string.
		FoundIndex = m_pSourceIDList->FindString( 0, String );

		if (FoundIndex != LB_ERR)
		{
			//---	Set the current selection to the found index.
			m_pSourceIDList->SetCurSel( FoundIndex );
			return;
		}

		String.Delete( String.GetLength()-1, 1 );
	}
}

void CSourceIDDlg::OnSelchangeDisplaytab(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	BuildList();
	FindString( m_AnimID );
	
	*pResult = 0;
}

void CSourceIDDlg::OnRadioassigned() 
{
	// TODO: Add your control notification handler code here
	BuildList();
	FindString( m_AnimID );
}

void CSourceIDDlg::OnRadiounassigned() 
{
	// TODO: Add your control notification handler code here
	BuildList();
	FindString( m_AnimID );
}
