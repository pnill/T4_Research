// DlgFaceDirRecipe.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "DlgFaceDirRecipe.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgFaceDirRecipe dialog


CDlgFaceDirRecipe::CDlgFaceDirRecipe(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgFaceDirRecipe::IDD, pParent)
{
	m_pCharacter	= NULL;
	m_RecipeLength	= 0;
	m_bInitCalled	= FALSE;

	//{{AFX_DATA_INIT(CDlgFaceDirRecipe)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgFaceDirRecipe::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgFaceDirRecipe)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgFaceDirRecipe, CDialog)
	//{{AFX_MSG_MAP(CDlgFaceDirRecipe)
	ON_BN_CLICKED(IDC_ADDBONE, OnAddbone)
	ON_BN_CLICKED(IDC_REMOVEBONE, OnRemovebone)
	ON_CBN_SELCHANGE(IDC_BONE0, OnSelchangeBone0)
	ON_CBN_SELCHANGE(IDC_BONE1, OnSelchangeBone1)
	ON_CBN_SELCHANGE(IDC_BONE2, OnSelchangeBone2)
	ON_CBN_SELCHANGE(IDC_BONE3, OnSelchangeBone3)
	ON_CBN_SELCHANGE(IDC_BONE4, OnSelchangeBone4)
	ON_CBN_SELCHANGE(IDC_BONE5, OnSelchangeBone5)
	ON_EN_CHANGE(IDC_WEIGHT0, OnChangeWeight0)
	ON_EN_CHANGE(IDC_WEIGHT1, OnChangeWeight1)
	ON_EN_CHANGE(IDC_WEIGHT2, OnChangeWeight2)
	ON_EN_CHANGE(IDC_WEIGHT3, OnChangeWeight3)
	ON_EN_CHANGE(IDC_WEIGHT4, OnChangeWeight4)
	ON_EN_CHANGE(IDC_WEIGHT5, OnChangeWeight5)
	ON_WM_CANCELMODE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgFaceDirRecipe message handlers

BOOL CDlgFaceDirRecipe::OnInitDialog() 
{
	CDialog::OnInitDialog();
	s32	BoneCombo[FACEDIR_RECIPE_PAGE_LENGTH]	= {IDC_BONE0, IDC_BONE1, IDC_BONE2, IDC_BONE3, IDC_BONE4, IDC_BONE5};
	s32	WeightEdit[FACEDIR_RECIPE_PAGE_LENGTH]	= {IDC_WEIGHT0, IDC_WEIGHT1, IDC_WEIGHT2, IDC_WEIGHT3, IDC_WEIGHT4, IDC_WEIGHT5};
	s32	Spinners[FACEDIR_RECIPE_PAGE_LENGTH]	= {IDC_SPIN0, IDC_SPIN1, IDC_SPIN2, IDC_SPIN3, IDC_SPIN4, IDC_SPIN5};
	s32	Percent[FACEDIR_RECIPE_PAGE_LENGTH]		= {IDC_PERCENT0, IDC_PERCENT1, IDC_PERCENT2, IDC_PERCENT3, IDC_PERCENT4, IDC_PERCENT5};
	
	s32 i;

	m_bInitCalled	= TRUE;

	//---	get pointers to the dialog components
	for( i=0; i<FACEDIR_RECIPE_PAGE_LENGTH; i++ )
	{
		m_Recipe[i].pBone		= (CComboBox*)			GetDlgItem( BoneCombo[i] );
		m_Recipe[i].pWeight		= (CEdit*)				GetDlgItem( WeightEdit[i] );
		m_Recipe[i].pSpin		= (CSpinButtonCtrl*)	GetDlgItem( Spinners[i] );
		m_Recipe[i].pPercent	= (CEdit*)				GetDlgItem( Percent[i] );

		m_Recipe[i].pSpin->SetRange( 0, 1000 );

		CSkeleton *pSkel = m_pCharacter->GetSkeleton() ;
		if( pSkel )
		{
			for( int j = 0 ; j < pSkel->GetNumBones() ; j++ )
			{
				CString	s ;
				for( int k = 0 ; k<pSkel->GetBoneNumParents( pSkel->GetBoneFromIndex(j) ) ; k++ )
				{
					s += "  " ;
				}
				s += pSkel->GetBoneFromIndex(j)->BoneName ;

				m_Recipe[i].pBone->AddString( s ) ;
			}
		}
	}

	//---	Set the display
	m_RecipeLength = m_pCharacter->m_FaceDirRecipeCount;
	for( i=0; i<m_RecipeLength; i++ )
	{
		char String[20];
		m_Recipe[i].pBone->SetCurSel( m_pCharacter->GetFaceDirRecipe(i).BoneID );
		m_Recipe[i].pWeight->SetWindowText( itoa( m_pCharacter->GetFaceDirRecipe(i).Weight, String, 20 ) );
	}
	UpdateDisplay();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgFaceDirRecipe::UpdateDisplay( void )
{
	s32		Weight[FACEDIR_RECIPE_PAGE_LENGTH];
	s32		TotalWeight=0;
	CString str;
	s32		i;

	if( !m_bInitCalled )
		return;

	//---	loop through and get the weights, calculate the total weight
	for( i=0; i<m_RecipeLength; i++ )
	{
		m_Recipe[i].pWeight->GetWindowText( str );
		Weight[i] = atoi( str );
		TotalWeight += Weight[i];
	}

	//---	set the percentages
	for( i=0; i<m_RecipeLength; i++ )
	{
		str.Format("%5.2f", (f32)Weight[i] / (f32)TotalWeight );
		m_Recipe[i].pPercent->SetWindowText( str );

		m_Recipe[i].pBone->ModifyStyle( 0, WS_VISIBLE );
		m_Recipe[i].pWeight->ModifyStyle( 0, WS_VISIBLE );
		m_Recipe[i].pSpin->ModifyStyle( 0, WS_VISIBLE );
		m_Recipe[i].pPercent->ModifyStyle( 0, WS_VISIBLE );
	}

	for( ; i<FACEDIR_RECIPE_PAGE_LENGTH; i++ )
	{
		m_Recipe[i].pBone->ModifyStyle( WS_VISIBLE, 0 );
		m_Recipe[i].pWeight->ModifyStyle( WS_VISIBLE, 0 );
		m_Recipe[i].pSpin->ModifyStyle( WS_VISIBLE, 0 );
		m_Recipe[i].pPercent->ModifyStyle( WS_VISIBLE, 0 );
	}

	Invalidate();
}

void CDlgFaceDirRecipe::OnCancelMode() 
{
	CDialog::OnCancelMode();
	
}

void CDlgFaceDirRecipe::OnOK() 
{
	s32		i;
	s32		BoneID;
	CString	str;

	//---	Set the display
	m_pCharacter->m_FaceDirRecipeCount = m_RecipeLength;
	for( i=0; i<m_RecipeLength; i++ )
	{
		m_Recipe[i].pWeight->GetWindowText( str );
		BoneID = m_Recipe[i].pBone->GetCurSel();

		m_pCharacter->m_FaceDirRecipe[i].Weight		= atoi( str );
		m_pCharacter->m_FaceDirRecipe[i].BoneID		= BoneID;
		m_pCharacter->m_FaceDirRecipe[i].BoneName.Format( "%s", m_pCharacter->GetSkeleton()->GetBoneFromIndex( BoneID )->BoneName );
	}

	//---	increment our recipe ID value
	m_pCharacter->m_FaceDirRecipeID++;

	CDialog::OnOK();
}

void CDlgFaceDirRecipe::OnAddbone() 
{
	if( m_RecipeLength == MAX_FACEDIR_RECIPE_LENGTH )
		return;

	m_Recipe[m_RecipeLength].pBone->SetCurSel( 0 );
	m_Recipe[m_RecipeLength].pWeight->SetWindowText( "0" );
	m_Recipe[m_RecipeLength].pPercent->SetWindowText( "0" );
	m_RecipeLength++;

	UpdateDisplay();
}

void CDlgFaceDirRecipe::OnRemovebone() 
{
	if( m_RecipeLength == 0 )
		return;

	m_RecipeLength--;

	UpdateDisplay();
}

void CDlgFaceDirRecipe::SelchangeBone( s32 Index )
{
}

void CDlgFaceDirRecipe::ChangeWeight( s32 Index )
{
	UpdateDisplay();
}

void CDlgFaceDirRecipe::OnSelchangeBone0() 
{
	SelchangeBone( 0 );
}

void CDlgFaceDirRecipe::OnSelchangeBone1() 
{
	SelchangeBone( 1 );
}

void CDlgFaceDirRecipe::OnSelchangeBone2() 
{
	SelchangeBone( 2 );
}

void CDlgFaceDirRecipe::OnSelchangeBone3() 
{
	SelchangeBone( 3 );
}

void CDlgFaceDirRecipe::OnSelchangeBone4() 
{
	SelchangeBone( 4 );
}

void CDlgFaceDirRecipe::OnSelchangeBone5() 
{
	SelchangeBone( 5 );
}

void CDlgFaceDirRecipe::OnChangeWeight0() 
{
	ChangeWeight(0);
}

void CDlgFaceDirRecipe::OnChangeWeight1() 
{
	ChangeWeight(1);
}

void CDlgFaceDirRecipe::OnChangeWeight2() 
{
	ChangeWeight(2);
}

void CDlgFaceDirRecipe::OnChangeWeight3() 
{
	ChangeWeight(3);
}

void CDlgFaceDirRecipe::OnChangeWeight4() 
{
	ChangeWeight(4);
}

void CDlgFaceDirRecipe::OnChangeWeight5() 
{
	ChangeWeight(5);
}
