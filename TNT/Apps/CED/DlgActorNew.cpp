// DlgActorNew.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "DlgActorNew.h"
#include "CharacterList.h"
#include "MeshList.h"
#include "Mesh.h"
#include "Actor.h"
#include "Skin.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgActorNew dialog


CDlgActorNew::CDlgActorNew(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgActorNew::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgActorNew)
	m_Name = _T("");
	m_Type = -1;
	//}}AFX_DATA_INIT

	m_pCharacterList	= NULL;
	m_pCharacter		= NULL;
	m_Skin				= -1;
	m_pMesh				= NULL;
	m_LockType			= FALSE;
}


void CDlgActorNew::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgActorNew)
	DDX_Control(pDX, IDC_SKINSELECT, m_SkinSelect);
	DDX_Control(pDX, IDC_MESHES, m_Meshes);
	DDX_Control(pDX, IDC_CHARACTERS, m_Characters);
	DDX_Text(pDX, IDC_NAME, m_Name);
	DDX_Radio(pDX, IDC_RADIO1, m_Type);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgActorNew, CDialog)
	//{{AFX_MSG_MAP(CDlgActorNew)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_BN_CLICKED(IDC_RADIO3, OnRadio3)
	ON_BN_CLICKED(IDC_RADIO4, OnRadio4)
	ON_CBN_SELCHANGE(IDC_CHARACTERS, OnSelchangeCharacters)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgActorNew message handlers

BOOL CDlgActorNew::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	SetDlgItemText( IDC_NAME, m_Name );

	((CButton*)GetDlgItem(IDC_RADIO1))->SetCheck( FALSE );
	((CButton*)GetDlgItem(IDC_RADIO2))->SetCheck( FALSE );
	((CButton*)GetDlgItem(IDC_RADIO3))->SetCheck( FALSE );
	((CButton*)GetDlgItem(IDC_RADIO4))->SetCheck( FALSE );
	switch( m_Type )
	{
	case ACTOR_CHARACTER:	((CButton*)GetDlgItem(IDC_RADIO1))->SetCheck( TRUE );	break;
	case ACTOR_CAMERA:		((CButton*)GetDlgItem(IDC_RADIO2))->SetCheck( TRUE );	break;
	case ACTOR_LIGHT:		((CButton*)GetDlgItem(IDC_RADIO3))->SetCheck( TRUE );	break;
	case ACTOR_SCENERY:		((CButton*)GetDlgItem(IDC_RADIO4))->SetCheck( TRUE );	break;
	}
	
	//---	Add All characters to List
	ASSERT( m_pCharacterList ) ;
	s32 CharIndex = 0;
	m_Characters.ResetContent( ) ;
	m_Characters.AddString( "<no character>" ) ;
	POSITION Pos = m_pCharacterList->GetHeadPosition() ;
	while( Pos )
	{
		CCharacter *pCharacter = m_pCharacterList->GetNext( Pos ) ;
		ASSERT( pCharacter ) ;
		int Item = m_Characters.AddString( pCharacter->GetName() ) ;
		if( Item != -1 )
			m_Characters.SetItemData( Item, (DWORD)pCharacter ) ;
	}
	if( m_pCharacter && (m_pCharacter->GetName() != "") )
	{
		CharIndex = m_Characters.FindStringExact( 0, m_pCharacter->GetName() );
		m_Characters.SetCurSel( CharIndex );
	}

	//---	fill the skin list with this character's skins
	FillSkinList();

	//---	Add All meshes to List
	ASSERT( m_pMeshList ) ;
	s32 MeshIndex = 0;
	m_Meshes.ResetContent( );
	m_Meshes.AddString( "<no mesh>" ) ;
	Pos = m_pMeshList->GetHeadPosition() ;
	while( Pos )
	{
		CMesh *pMesh = m_pMeshList->GetNext( Pos ) ;
		ASSERT( pMesh ) ;
		int Item = m_Meshes.AddString( pMesh->GetPathName() ) ;
		if( Item != -1 )
		{
			m_Meshes.SetItemData( Item, (DWORD)pMesh ) ;
			if( pMesh == m_pMesh )
				MeshIndex = Item;
			if( Item <= MeshIndex )
				MeshIndex++;
		}
	}
	if( m_pMesh && (m_pMesh->GetPathName() != "") )
	{
		MeshIndex = m_Meshes.FindStringExact( 0, m_pMesh->GetPathName() );
		m_Meshes.SetCurSel( MeshIndex ) ;
	}


	m_Characters.EnableWindow( m_Type == ACTOR_CHARACTER ) ;
	m_Meshes.EnableWindow( m_Type == ACTOR_SCENERY ) ;
	UpdateData( false ) ;

	if( m_LockType )
	{
		GetDlgItem( IDC_RADIO1 )->EnableWindow( FALSE );
		GetDlgItem( IDC_RADIO2 )->EnableWindow( FALSE );
		GetDlgItem( IDC_RADIO3 )->EnableWindow( FALSE );
		GetDlgItem( IDC_RADIO4 )->EnableWindow( FALSE );
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgActorNew::FillSkinList( void )
{
	m_SkinSelect.ResetContent();
	m_SkinSelect.SetCurSel( -1 );

	s32 Index = m_SkinSelect.AddString( "<no skin>" );
	m_SkinSelect.SetItemData( Index, (u32)-1 );
	m_SkinSelect.SetCurSel( Index );

	if( m_pCharacter )
	{
		s32 i;
		s32 c = m_pCharacter->GetNumSkins();
		for( i=0; i<c; i++ )
		{
			Index = m_SkinSelect.AddString( m_pCharacter->GetSkin(i)->GetName() );
			m_SkinSelect.SetItemData( Index, (u32)i );
		}

		if(( m_Skin >= 0 ) && ( m_Skin < c ))
			m_SkinSelect.SetCurSel( m_Skin+1 );
	}
}

void CDlgActorNew::OnOK() 
{
	// TODO: Add extra validation here

	UpdateData( ) ;

	//---	Check for Valid Character
	if( m_Type == 0 )
	{
		if( m_Characters.GetCurSel() == -1 )
			return;

		m_pCharacter = (CCharacter*)m_Characters.GetItemData( m_Characters.GetCurSel() ) ;
		if( !m_pCharacter )
			return ;

		if( m_SkinSelect.GetCurSel() >= 0 )
			m_Skin = (s32)m_SkinSelect.GetItemData( m_SkinSelect.GetCurSel() );
		else
			m_Skin = -1;
	}
	if( m_Type == 3 )
	{
		if( m_Meshes.GetCurSel() == -1 )
			return;

		m_pMesh = (CMesh*)m_Meshes.GetItemData( m_Meshes.GetCurSel() ) ;
		if( !m_pMesh )
			return ;
	}

	CDialog::OnOK();
}

void CDlgActorNew::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	m_Characters.EnableWindow( true ) ;
	m_Meshes.EnableWindow( false ) ;
}

void CDlgActorNew::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	m_Characters.EnableWindow( false ) ;
	m_Meshes.EnableWindow( false ) ;
}

void CDlgActorNew::OnRadio3() 
{
	// TODO: Add your control notification handler code here
	m_Characters.EnableWindow( false ) ;
	m_Meshes.EnableWindow( false ) ;
}

void CDlgActorNew::OnRadio4() 
{
	// TODO: Add your control notification handler code here
	m_Characters.EnableWindow( false ) ;
	m_Meshes.EnableWindow( true ) ;
}

void CDlgActorNew::OnSelchangeCharacters() 
{
	m_Skin = -1;
	if( m_Characters.GetCurSel() >= 0 )
		m_pCharacter = (CCharacter*)m_Characters.GetItemData( m_Characters.GetCurSel() ) ;
	else
		m_pCharacter = NULL;
	FillSkinList();
}
