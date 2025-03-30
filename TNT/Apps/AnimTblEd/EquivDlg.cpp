// EquivDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AnimTblEd.h"
#include "EquivDlg.h"
#include "SourceIDDlg.h"
#include "AnimTblEdGlobals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEquivDlg dialog


CEquivDlg::CEquivDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEquivDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEquivDlg)
	//}}AFX_DATA_INIT
}


void CEquivDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEquivDlg)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEquivDlg, CDialog)
	//{{AFX_MSG_MAP(CEquivDlg)
	ON_BN_CLICKED(IDC_BUTTON_LEFTCHANGEID, OnButtonLeftchangeid)
	ON_BN_CLICKED(IDC_BUTTON_RIGHTCHANGEID, OnButtonRightchangeid)
	ON_BN_CLICKED(IDC_BUTTON_SWAP, OnButtonSwap)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEquivDlg message handlers

BOOL CEquivDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	GetDlgItem( IDC_BUTTON_LEFTCHANGEID )->SetFocus();

	this->ShowData();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEquivDlg::OnOK() 
{
	CButton		*pCheckBox;

	if ((strlen( m_pEquivNode->GetLeftID() ) > 0) && (strlen( m_pEquivNode->GetRightID() ) > 0))
	{
		//---	The Left and Right IDs should already be set up so we just need to save the flag settings.
		pCheckBox = (CButton*)GetDlgItem( IDC_CHECK_LEFTMIRRORED );
		m_pEquivNode->SetLeftMirrored( pCheckBox->GetCheck() ? TRUE : FALSE );

		pCheckBox = (CButton*)GetDlgItem( IDC_CHECK_LEFTREVERSED );
		m_pEquivNode->SetLeftReversed( pCheckBox->GetCheck() ? TRUE : FALSE );

		pCheckBox = (CButton*)GetDlgItem( IDC_CHECK_RIGHTMIRRORED );
		m_pEquivNode->SetRightMirrored( pCheckBox->GetCheck() ? TRUE : FALSE );

		pCheckBox = (CButton*)GetDlgItem( IDC_CHECK_RIGHTREVERSED );
		m_pEquivNode->SetRightReversed( pCheckBox->GetCheck() ? TRUE : FALSE );

		//---	Mark the document as having changed.
		GBL_SetModifiedFlag( TRUE );

		CDialog::OnOK();
	}
	else
		this->MessageBox( "The equivalency must have a left and right ID.", "Error", MB_ICONERROR );
}


void CEquivDlg::OnCancel() 
{
	//---	Put the old IDs back.
	m_pEquivNode->SetLeftID( m_OldLeftID, m_pIDList );
	m_pEquivNode->SetRightID( m_OldRightID, m_pIDList );
	
	CDialog::OnCancel();
}

void CEquivDlg::OnButtonLeftchangeid() 
{
/*
	CSourceIDDlg	SourceIDDlg;

	SourceIDDlg.m_LeftID = TRUE;
	SourceIDDlg.m_pEquivNode = m_pEquivNode;
	SourceIDDlg.m_pIDList = m_pIDList;
	SourceIDDlg.m_FindString.Format( "%s", m_pEquivNode->m_pParent->GetName() );

	if (SourceIDDlg.DoModal() == IDOK)
		this->GetDlgItem( IDOK )->SetFocus();

	this->ShowData();
	*/
}

void CEquivDlg::OnButtonRightchangeid() 
{
/*
	CSourceIDDlg	SourceIDDlg;

	SourceIDDlg.m_LeftID = FALSE;
	SourceIDDlg.m_pEquivNode = m_pEquivNode;
	SourceIDDlg.m_pIDList = m_pIDList;
	SourceIDDlg.m_FindString.Format( "%s", m_pEquivNode->m_pParent->GetName() );

	SourceIDDlg.DoModal();

	this->ShowData();
	*/
}

void CEquivDlg::OnButtonSwap() 
{
	CString	RightName;
	int		RightMirrored;
	int		RightReversed;
	int		LeftMirrored;
	int		LeftReversed;

	//---	Swap the identifiers and the flags.	
	RightName.Format( "%s", m_pEquivNode->GetRightID() );
	RightMirrored = ((CButton*)GetDlgItem( IDC_CHECK_RIGHTMIRRORED ))->GetCheck();
	RightReversed = ((CButton*)GetDlgItem( IDC_CHECK_RIGHTREVERSED ))->GetCheck();
	LeftMirrored = ((CButton*)GetDlgItem( IDC_CHECK_LEFTMIRRORED ))->GetCheck();
	LeftReversed = ((CButton*)GetDlgItem( IDC_CHECK_LEFTREVERSED ))->GetCheck();

	m_pEquivNode->SetRightID( m_pEquivNode->GetLeftID(), m_pIDList );
	m_pEquivNode->SetLeftID( RightName, m_pIDList );

	m_pEquivNode->SetRightMirrored( LeftMirrored ? TRUE : FALSE );
	m_pEquivNode->SetRightReversed( LeftReversed ? TRUE : FALSE );
	m_pEquivNode->SetLeftMirrored( RightMirrored ? TRUE : FALSE );
	m_pEquivNode->SetLeftReversed( RightReversed ? TRUE : FALSE );

	GBL_SetModifiedFlag( TRUE );

	this->ShowData();
}

void CEquivDlg::ShowData()
{
	CButton		*pCheckBox;
	
	//---	Assign the left ID name.
	SetDlgItemText( IDC_EDIT_LEFTID, m_pEquivNode->GetLeftID() );
	m_OldLeftID.Format( "%s", m_pEquivNode->GetLeftID() );

	//---	Mark the left ID as mirrored.
	pCheckBox = (CButton*)GetDlgItem( IDC_CHECK_LEFTMIRRORED );
	pCheckBox->SetCheck( m_pEquivNode->GetLeftMirrored() );

	//---	Mark the left ID as reversed.
	pCheckBox = (CButton*)GetDlgItem( IDC_CHECK_LEFTREVERSED );
	pCheckBox->SetCheck( m_pEquivNode->GetLeftReversed() );

	//---	Assign the right ID name.
	SetDlgItemText( IDC_EDIT_RIGHTID, m_pEquivNode->GetRightID() );
	m_OldRightID.Format( "%s", m_pEquivNode->GetRightID() );

	//---	Mark the right ID as mirrored.
	pCheckBox = (CButton*)GetDlgItem( IDC_CHECK_RIGHTMIRRORED );
	pCheckBox->SetCheck( m_pEquivNode->GetRightMirrored() );

	//---	Mark the right ID as reversed.
	pCheckBox = (CButton*)GetDlgItem( IDC_CHECK_RIGHTREVERSED );
	pCheckBox->SetCheck( m_pEquivNode->GetRightReversed() );
}
