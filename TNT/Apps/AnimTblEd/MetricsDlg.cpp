// MetricsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AnimTblEd.h"
#include "MetricsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMetricsDlg dialog


CMetricsDlg::CMetricsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMetricsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMetricsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMetricsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMetricsDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMetricsDlg, CDialog)
	//{{AFX_MSG_MAP(CMetricsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMetricsDlg message handlers

BOOL CMetricsDlg::OnInitDialog() 
{
	CString	NumString;
	int		nAnims;
	int		nTransitions;
	int		nAnimsWTrans;
	int		nGroups;
	int		nBytes;

	CDialog::OnInitDialog();
	
	//---	Get the numbers of these entries in the table.
	nAnims = pDoc->m_AnimTree.GetNumExportedAnims();
	nTransitions = pDoc->m_AnimTree.GetTotalNumDependancies( TRUE );
	nAnimsWTrans = pDoc->m_AnimTree.GetNumAnimsWithDependancies( TRUE );
	nGroups = pDoc->m_AnimTree.GetGroupExportCount();

	//---	Total the amount of RAM that will be required.
	//---	Number of animations * the sizeof each anim.
	int nAnimBytes = nAnims * pDoc->SizeOfAnimTableEntry();

	//---	Size of the Transition table. is equal to the size of each transition(4 bytes) * the number of transitions + 1(termination -1) for each anim.
	int nTransBytes = 4*nTransitions + nAnimsWTrans;

	int nGroupBytes = nGroups*8;

	nBytes = nAnimBytes+nTransBytes+nGroupBytes;

	//---	Write the number of anims.
	NumString.Format( "%d", nAnims );
	this->SetDlgItemText( IDC_NUMANIMS, NumString );

	//---	Write the number of groups.
	NumString.Format( "%d", nGroups );
	this->SetDlgItemText( IDC_NUMGROUPS, NumString );

	//---	Write the number of transitions.
	NumString.Format( "%d", nTransitions );
	this->SetDlgItemText( IDC_NUMTRANSITIONS, NumString );

	//---	Write the number of anims with transitions.
	NumString.Format( "%d", nAnimsWTrans );
	this->SetDlgItemText( IDC_NUMANIMSWTRANS, NumString );


	NumString.Format( "%d * %d = %d bytes", pDoc->SizeOfAnimTableEntry(), nAnims, nAnimBytes );
	this->SetDlgItemText( IDC_SIZEPERANIM, NumString );

	NumString.Format( "%d * %d + %d = %d bytes", 4, nTransitions, nAnimsWTrans, nTransBytes );
	this->SetDlgItemText( IDC_SIZEPERTRANSITION, NumString );

	NumString.Format( "%d * %d = %d bytes", 8, nGroups, nGroupBytes );
	this->SetDlgItemText( IDC_SIZEPERGROUP, NumString );


	//---	Write the number of bytes required.
	NumString.Format( "%d bytes", nBytes );
	this->SetDlgItemText( IDC_SIZEINBYTES, NumString );

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
