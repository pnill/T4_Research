// FlagsEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AnimTblEd.h"
#include "FlagsEditDlg.h"
#include "StateName.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// FlagsEditDlg dialog


CFlagsEditDlg::CFlagsEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFlagsEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFlagsEditDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CFlagsEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFlagsEditDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFlagsEditDlg, CDialog)
	//{{AFX_MSG_MAP(CFlagsEditDlg)
	ON_BN_CLICKED(IDC_ADDFLAG, OnAddflag)
	ON_BN_CLICKED(IDC_DELETEFLAG, OnDeleteflag)
	ON_BN_CLICKED(IDC_MODIFYFLAG, OnModifyflag)
	ON_LBN_DBLCLK(IDC_FLAGLIST, OnDblclkFlaglist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFlagsEditDlg message handlers

BOOL CFlagsEditDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	UpdateMainDialog();
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFlagsEditDlg::UpdateMainDialog()
{
	int i;

	//---	set the read only fields
	SetDlgItemInt(IDC_TOTALSTATEBITS, m_TotalStateBits);
	SetDlgItemInt(IDC_TOTALPRIORITYBITS, m_TotalPriorityBits);
	SetDlgItemInt(IDC_TOTALFLAGBITS, m_FlagList.GetTotalFlagBits());
	SetDlgItemInt(IDC_TOTALMETHODBITS, m_TotalMethodBits);
	SetDlgItemInt(IDC_REMAININGBITS, m_TotalBits - m_TotalStateBits - m_TotalPriorityBits - m_FlagList.GetTotalFlagBits() - m_TotalMethodBits);

	//---	enable/disable buttons
	GetDlgItem(IDC_MODIFYFLAG)->EnableWindow(m_FlagList.GetNumFlags());
	GetDlgItem(IDC_DELETEFLAG)->EnableWindow(m_FlagList.GetNumFlags());
	((CListBox*) GetDlgItem(IDC_FLAGLIST))->ResetContent();
	if (m_FlagList.GetNumFlags() > 0)
	{
		//---	add the flags to the flag list
		for (i=0; i<m_FlagList.GetNumFlags(); i++)
			((CListBox*) GetDlgItem(IDC_FLAGLIST))->AddString(m_FlagList.GetFlag(i)->GetName());

		//((CListBox*) GetDlgItem(IDC_FLAGLIST))->SetSel(m_SelState);
	}
}

void CFlagsEditDlg::OnAddflag() 
{
	CStateName NameDlg;
	int index;

	index = ((CListBox*) GetDlgItem(IDC_FLAGLIST))->GetCurSel();
	if (index == LB_ERR)
		index = -1;

	NameDlg.m_StateName = "NEW_NAME";
	NameDlg.m_StateNamePrefix = "ANIMFLAG_";

	if (NameDlg.DoModal() == IDOK)
	{
		//---	if a Flag of this same name already exists, do not add this one
		if (m_FlagList.FindFlag(NameDlg.m_StateName))
		{
			this->MessageBox("Flag name is not unique.", NULL, MB_ICONERROR);
			return;
		}

		//---	add this flag
		m_FlagList.AddFlag(NameDlg.m_StateName, index);
		AddChange(1, index);
		UpdateMainDialog();
	}
}

void CFlagsEditDlg::OnDeleteflag() 
{
	int index = ((CListBox*) GetDlgItem(IDC_FLAGLIST))->GetCurSel();

	if (index == LB_ERR)
		return;

	m_FlagList.RemoveFlag(index);
	AddChange(-1, index);
	UpdateMainDialog();
}

void CFlagsEditDlg::OnModifyflag() 
{
	CStateName NameDlg;
	CFlag *pFlag;
	int i = ((CListBox*) GetDlgItem(IDC_FLAGLIST))->GetCurSel();

	if (i == LB_ERR)
		return;

	pFlag = m_FlagList.GetFlag(i);

	NameDlg.m_StateName = pFlag->GetName();
	NameDlg.m_StateNamePrefix = "ANIMFLAG_";

	if (NameDlg.DoModal() == IDOK)
	{
		//---	if a Flag of this same name already exists, do not add this one
		if (m_FlagList.FindFlag(NameDlg.m_StateName))
		{
			this->MessageBox("Flag name is not unique.", NULL, MB_ICONERROR);
			return;
		}

		//---	modify the flag perminently
		pFlag->SetName(NameDlg.m_StateName);
		UpdateMainDialog();
	}
}

void CFlagsEditDlg::OnDblclkFlaglist() 
{
	// TODO: Add your control notification handler code here
	
}

void CFlagsEditDlg::AddChange(int Operation, int Flag)
{
	int Change;

	Change = (Operation << 16) | (Flag & 0xffff);
	m_Changes.AddTail(Change);
}

void CFlagsEditDlg::GetChange(int &Operation, int &Flag)
{
	if (m_Changes.GetCount() <= 0)
	{
		Operation = 0;
		return;
	}

	int Change = m_Changes.GetHead();
	Operation = (Change >> 16);
	Flag = Change & 0xffff;

	m_Changes.RemoveAt(m_Changes.GetHeadPosition());
}

