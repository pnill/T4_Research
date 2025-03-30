// DlgSelectMesh.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "DlgSelectMesh.h"
#include "MeshList.h"
#include "Mesh.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgSelectMesh dialog


CDlgSelectMesh::CDlgSelectMesh(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSelectMesh::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgSelectMesh)
	m_Name = _T("");
	//}}AFX_DATA_INIT
}


void CDlgSelectMesh::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSelectMesh)
	DDX_Control(pDX, IDC_MESHES, m_Meshes);
	DDX_Text(pDX, IDC_NAME, m_Name);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgSelectMesh, CDialog)
	//{{AFX_MSG_MAP(CDlgSelectMesh)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSelectMesh message handlers

BOOL CDlgSelectMesh::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

    //---	Add All meshes to List
	ASSERT( m_pMeshList ) ;
	m_Meshes.ResetContent( ) ;
	m_Meshes.AddString( "<no mesh>" ) ;
	POSITION Pos = m_pMeshList->GetHeadPosition() ;
    int CurSel = 0 ;
	while( Pos )
	{
		CMesh *pMesh = m_pMeshList->GetNext( Pos ) ;
		ASSERT( pMesh ) ;
		int Item = m_Meshes.AddString( pMesh->GetPathName() ) ;
		if( Item != -1 )
		{
			m_Meshes.SetItemData( Item, (DWORD)pMesh ) ;
		}
        if( pMesh == m_pMesh )
            CurSel = Item ;
	}
	m_Meshes.SetCurSel( CurSel ) ;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgSelectMesh::OnOK() 
{
	// TODO: Add extra validation here

    m_pMesh = (CMesh*)m_Meshes.GetItemData( m_Meshes.GetCurSel() ) ;
	
	CDialog::OnOK();
}
