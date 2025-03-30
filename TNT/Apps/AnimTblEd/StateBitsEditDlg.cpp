// StateBitsEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AnimTblEd.h"
#include "StateBitsEditDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStateBitsEditDlg dialog


CStateBitsEditDlg::CStateBitsEditDlg(CAnimTblEdDoc *pDoc, CWnd* pParent /*=NULL*/)
	: CDialog(CStateBitsEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStateBitsEditDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	//---	get the number of flag bits
	m_NumFlagBits = pDoc->GetTotalFlagBits();

	POSITION pos;

	pos = pDoc->m_StateClassList.GetHead();
	CStateClass *pStateClass;
	CStateClass *pCopyClass;
	while (pos)
	{
		pStateClass = pDoc->m_StateClassList.GetNext(pos);
		pCopyClass = new CStateClass(pStateClass->GetName());
		m_StateClassList.AddTail(pCopyClass);
	}
}

CStateBitsEditDlg::~CStateBitsEditDlg()
{
	POSITION pos;
	CStateClass *pStateClass;
	pos = m_StateClassList.GetHead();
	while (pos)
	{
		pStateClass = pDoc->m_StateClassList.GetNext(pos);
		delete pStateClass;
	}
}


void CStateBitsEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStateBitsEditDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStateBitsEditDlg, CDialog)
	//{{AFX_MSG_MAP(CStateBitsEditDlg)
	ON_BN_CLICKED(IDC_NEWSTATECLASS, OnNewstateclass)
	ON_BN_CLICKED(IDC_DELETESTATECLASS, OnDeletestateclass)
	ON_BN_CLICKED(IDC_ADDSTATE, OnAddstate)
	ON_BN_CLICKED(IDC_DELETESTATE, OnDeletestate)
	ON_EN_UPDATE(IDC_STATECLASSNAME, OnUpdateStateclassname)
	ON_LBN_DBLCLK(IDC_STATELIST, OnDblclkStatelist)
	ON_WM_CANCELMODE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStateBitsEditDlg message handlers

BOOL CStateBitsEditDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	//---	set the 
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CStateBitsEditDlg::OnCancelMode() 
{
	CDialog::OnCancelMode();
	
	// TODO: Add your message handler code here
	
}

void CStateBitsEditDlg::OnNewstateclass() 
{
	// TODO: Add your control notification handler code here
	
}

void CStateBitsEditDlg::OnDeletestateclass() 
{
	// TODO: Add your control notification handler code here
	
}

void CStateBitsEditDlg::OnAddstate() 
{
	// TODO: Add your control notification handler code here
	
}

void CStateBitsEditDlg::OnDeletestate() 
{
	// TODO: Add your control notification handler code here
	
}

void CStateBitsEditDlg::OnUpdateStateclassname() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
}

void CStateBitsEditDlg::OnDblclkStatelist() 
{
	// TODO: Add your control notification handler code here
	
}

