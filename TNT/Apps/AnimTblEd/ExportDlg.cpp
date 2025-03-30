// ExportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AnimTblEd.h"
#include "ExportDlg.h"
#include "AnimTblEdDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExportDlg dialog


CExportDlg::CExportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExportDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExportDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportDlg)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExportDlg, CDialog)
	//{{AFX_MSG_MAP(CExportDlg)
	ON_BN_CLICKED(IDC_BUTTON_NEWEXPORT, OnButtonNewexport)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT, OnButtonExport)
	ON_BN_CLICKED(IDC_BUTTON_EDIT, OnButtonEdit)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_LBN_DBLCLK(IDC_LIST_EXPORT, OnDblclkListExport)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportDlg message handlers

void CExportDlg::SetAllCaps(bool Set)
{
//	((CButton*)this->GetDlgItem(IDC_CHECK_ALLCAPS))->SetCheck( m_AllCaps );
}

//DEL void CExportDlg::SetPathFile(const char *lpszPathFile, int which)
//DEL {
//DEL 	m_OutputFilePath[which].Format( "%s", lpszPathFile );
//DEL 	switch( which )
//DEL 	{
//DEL 	case 0:
//DEL 		SetDlgItemText( IDC_EDIT_PATHFILE, m_OutputFilePath[0] );
//DEL 		break;
//DEL 	case 1:
//DEL 		SetDlgItemText( IDC_EDIT_PATHFILE2, m_OutputFilePath[1] );
//DEL 		break;
//DEL 	case 2:
//DEL 		SetDlgItemText( IDC_EDIT_PATHFILE3, m_OutputFilePath[2] );
//DEL 		break;
//DEL 	}
//DEL }

BOOL CExportDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	this->BuildExportList();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//DEL void CExportDlg::ChangePath(int which)
//DEL {
//DEL 	CString			sFilter = "C Include Files (*.h)||";
//DEL 	CFileDialog		FileDlg( TRUE, "*.h", m_OutputFilePath[which], 0, sFilter, this );
//DEL 
//DEL 	if (FileDlg.DoModal() == IDOK)
//DEL 	{
//DEL 		CString		PathFile;
//DEL 
//DEL 		PathFile = FileDlg.GetPathName();
//DEL 
//DEL 
//DEL 		SetPathFile( PathFile, which );
//DEL 	}
//DEL }


//DEL void CExportDlg::OnButtonChangepath() 
//DEL {
//DEL 	// TODO: Add your control notification handler code here
//DEL 	ChangePath( 0 );
//DEL }

//DEL void CExportDlg::OnButtonChangepath2() 
//DEL {
//DEL 	// TODO: Add your control notification handler code here
//DEL 	ChangePath( 1 );
//DEL }

//DEL void CExportDlg::OnButtonChangepath3() 
//DEL {
//DEL 	// TODO: Add your control notification handler code here
//DEL 	ChangePath( 2 );
//DEL }

//DEL void CExportDlg::OnButtonDelete() 
//DEL {
//DEL 	// TODO: Add your control notification handler code here
//DEL 	SetPathFile( "", 0 );	
//DEL }

//DEL void CExportDlg::OnButtonDelete2() 
//DEL {
//DEL 	// TODO: Add your control notification handler code here
//DEL 	SetPathFile( "", 1 );	
//DEL }

//DEL void CExportDlg::OnButtonDelete3() 
//DEL {
//DEL 	// TODO: Add your control notification handler code here
//DEL 	SetPathFile( "", 2 );	
//DEL }

void CExportDlg::OnButtonNewexport() 
{
	CExportDef	*pExportDef = new CExportDef;

	if (pExportDef->Edit())
		m_pDoc->m_ExportList.Add( pExportDef );

	//---	Rebuild the list.
	this->BuildExportList();
}

void CExportDlg::OnButtonExport() 
{
	m_pDoc->CallExport();
}

void CExportDlg::OnButtonEdit() 
{
	CListBox	*pListBox;
	CExportDef	*pExportDef = NULL;
	int			CurSel;
	
	//---	Get the selected item from the export list and edit it.
	pListBox = ((CListBox*)this->GetDlgItem( IDC_LIST_EXPORT ));

	if (!pListBox)
		return;

	CurSel = pListBox->GetCurSel();

	if (CurSel == -1)
		return;

	pExportDef = m_pDoc->m_ExportList.GetNth( CurSel );

	if (!pExportDef)
		return;

	//---	If we have an export then edit it.
	pExportDef->Edit();

	//---	Rebuild the list.
	this->BuildExportList();
}

void CExportDlg::OnButtonDelete() 
{
	CListBox	*pListBox;
	CExportDef	*pExportDef = NULL;
	int			CurSel;
	
	//---	Get the selected item from the export list and edit it.
	pListBox = ((CListBox*)this->GetDlgItem( IDC_LIST_EXPORT ));

	if (!pListBox)
		return;

	CurSel = pListBox->GetCurSel();

	if (CurSel == -1)
		return;

	pExportDef = m_pDoc->m_ExportList.GetNth( CurSel );

	if (!pExportDef)
		return;

	//---	If we have an export then delete it.
	m_pDoc->m_ExportList.Remove( pExportDef );

	//---	Rebuild the list.
	this->BuildExportList();
}

void CExportDlg::BuildExportList()
{
	CListBox	*pListBox;
	CExportDef	*pExportDef;

	//---	Alias a pointer to the export list box.
	pListBox = (CListBox*)this->GetDlgItem( IDC_LIST_EXPORT );

	ASSERT(pListBox);

	//---	If the list is not empty then empty it.
	pListBox->ResetContent();

	//---	Add all the export definition names to the export list on the dialog.
	pExportDef = m_pDoc->m_ExportList.GetFirst();
	while (pExportDef)
	{
		pListBox->AddString( pExportDef->GetName() );
		pExportDef = m_pDoc->m_ExportList.GetNext();
	}
}

void CExportDlg::OnDblclkListExport() 
{
	// TODO: Add your control notification handler code here
	this->OnButtonEdit();
}
