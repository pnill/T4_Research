// DlgSkin.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "DlgSkin.h"
#include "Skin.h"
#include "mfcutils.h"
#include "MotionEditView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgSkin dialog


CDlgSkin::CDlgSkin(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSkin::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgSkin)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgSkin::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSkin)
	DDX_Control(pDX, IDC_SKIN, m_Combo);
	//}}AFX_DATA_MAP
}


void CDlgSkin::Reset( )
{
	m_Combo.ResetContent() ;
	int i = m_Combo.AddString( "<no skin>" ) ;
	m_Combo.SetItemData( i, NULL );

	((CButton*) GetDlgItem( IDC_TRANSLUCENT ))->SetCheck( m_pView->m_pCharacter->RenderSkinTranslucent() ? TRUE : FALSE );
}

void CDlgSkin::AddSkin( CSkin *pSkin )
{
	int Pos = m_Combo.AddString( pSkin->GetName() ) ;
	ASSERT( Pos != CB_ERR ) ;
	ASSERT( Pos != CB_ERRSPACE ) ;
	m_Combo.SetItemData( Pos, (DWORD)pSkin ) ;
}

void CDlgSkin::SelectSkin( CSkin *pSkin )
{
	int Index = MFCU_FindComboBoxItemByData( &m_Combo, (DWORD)pSkin ) ;
	if( Index != -1 )
	{
		m_Combo.SetCurSel( Index ) ;
		UpdateDetails( ) ;
	}
}

void CDlgSkin::UpdateDetails( )
{
	bool	Enable = false ;

	//---	Get Selected Character
	CSkin *pSkin = (CSkin*)m_Combo.GetItemData( m_Combo.GetCurSel() ) ;
	if( pSkin )
	{
		Enable = TRUE;

	}
}


BEGIN_MESSAGE_MAP(CDlgSkin, CDialog)
	//{{AFX_MSG_MAP(CDlgSkin)
	ON_CBN_SELCHANGE(IDC_SKIN, OnSelchangeSkin)
	ON_BN_CLICKED(IDC_TRANSLUCENT, OnTranslucent)
	ON_BN_CLICKED(IDC_IMPORT, OnImport)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSkin message handlers



BOOL CDlgSkin::OnInitDialog() 
{
	CDialog::OnInitDialog();
	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgSkin::OnSelchangeSkin() 
{
	// TODO: Add your control notification handler code here
	ASSERT( m_pView ) ;
	m_pView->SetSkin( m_Combo.GetCurSel() ) ;
	UpdateDetails( ) ;
}

void CDlgSkin::OnTranslucent() 
{
	ASSERT( m_pView );
	bool bTranslucent = ((CButton*)GetDlgItem( IDC_TRANSLUCENT ))->GetCheck() ? TRUE : FALSE;
	m_pView->SetSkinTranslucent( bTranslucent );
}

void CDlgSkin::OnImport() 
{
	m_pView->GetDocument()->CharacterImportSkin( m_pView->m_pCharacter );
	m_pView->m_pCharacter->SetCurSkin( m_pView->m_pCharacter->GetNumSkins()-1 );
	m_pView->UpdateSkinList();
	m_pView->Invalidate();
}

void CDlgSkin::OnDelete() 
{
	m_pView->GetDocument()->CharacterDeleteSkin( m_pView->m_pCharacter );
	m_pView->UpdateSkinList();
	m_pView->Invalidate();
}
