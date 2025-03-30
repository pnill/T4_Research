// PrioritiesEdit.cpp : implementation file
//

#include "stdafx.h"
#include "AnimTblEd.h"
#include "PrioritiesEdit.h"
#include "StateName.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPrioritiesEdit dialog


CPrioritiesEdit::CPrioritiesEdit(CWnd* pParent /*=NULL*/)
	: CDialog(CPrioritiesEdit::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPrioritiesEdit)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPrioritiesEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrioritiesEdit)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrioritiesEdit, CDialog)
	//{{AFX_MSG_MAP(CPrioritiesEdit)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_MODIFY, OnModify)
	ON_LBN_DBLCLK(IDC_LIST, OnDblclkList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrioritiesEdit message handlers

void CPrioritiesEdit::OnAdd() 
{
	CStateName NameDlg;
	int index;

	index = ((CListBox*) GetDlgItem(IDC_LIST))->GetCurSel();
	if (index == LB_ERR)
		index = -1;

	NameDlg.m_StateName = "NEW_PRIORITY";
	NameDlg.m_StateNamePrefix = "ANIMPRI_";

	if (NameDlg.DoModal() == IDOK)
	{
		//---	if a Priority of this same name already exists, do not add this one
		if (m_Priorities.FindPriority(NameDlg.m_StateName) >= 0)
		{
			this->MessageBox("Priority name is not unique.", NULL, MB_ICONERROR);
			return;
		}

		//---	add this priority
		m_Priorities.AddPriority(NameDlg.m_StateName, index);
		AddChange(1, index);

		UpdateMainDialog();
	}
}

void CPrioritiesEdit::OnDelete() 
{
	int index = ((CListBox*) GetDlgItem(IDC_LIST))->GetCurSel();

	if (index == LB_ERR)
		return;

	m_Priorities.RemovePriority(index);

	AddChange(-1, index);

	UpdateMainDialog();
}

void CPrioritiesEdit::OnModify() 
{
	CStateName NameDlg;
	CString Name;
	int f;
	int i = ((CListBox*) GetDlgItem(IDC_LIST))->GetCurSel();

	if (i == LB_ERR)
		return;

	Name = m_Priorities.GetPriority(i);

	NameDlg.m_StateName = Name;
	NameDlg.m_StateNamePrefix = "ANIMPRIORITY_";

	if (NameDlg.DoModal() == IDOK)
	{
		//---	if a priority of this same name already exists, do not add this one
		f = m_Priorities.FindPriority(NameDlg.m_StateName);
		if ((f >= 0) && (f != i))
		{
			this->MessageBox("Priority is not unique.", NULL, MB_ICONERROR);
			return;
		}

		//---	modify the priority perminently
		m_Priorities.GetPriority(i) = NameDlg.m_StateName;
		UpdateMainDialog();
	}
}

void CPrioritiesEdit::OnDblclkList() 
{
	OnModify();
}

BOOL CPrioritiesEdit::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	UpdateMainDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPrioritiesEdit::AddChange(int Operation, int Priority)
{
	int Change;

	Change = (Operation << 16) | (Priority & 0xffff);
	m_Changes.AddTail(Change);
}

void CPrioritiesEdit::GetChange(int &Operation, int &Priority)
{
	if (m_Changes.GetCount() <= 0)
	{
		Operation = 0;
		return;
	}

	int Change = m_Changes.GetHead();
	Operation = Change >> 16;
	Priority = Change & 0xffff;

	m_Changes.RemoveAt(m_Changes.GetHeadPosition());
}

void CPrioritiesEdit::UpdateMainDialog()
{
	int i;

	//---	set the read only fields
	SetDlgItemInt(IDC_TOTALSTATEBITS, m_TotalStateBits);
	SetDlgItemInt(IDC_TOTALPRIORITYBITS, m_Priorities.GetTotalPriorityBits());
	SetDlgItemInt(IDC_TOTALFLAGBITS, m_TotalFlagBits);
	SetDlgItemInt(IDC_TOTALMETHODBITS, m_TotalMethodBits);
	SetDlgItemInt(IDC_REMAININGBITS, m_TotalBits - m_TotalStateBits - m_Priorities.GetTotalPriorityBits() - m_TotalFlagBits - m_TotalMethodBits);

	//---	enable/disable buttons
	GetDlgItem(IDC_MODIFY)->EnableWindow(m_Priorities.GetCount());
	GetDlgItem(IDC_DELETE)->EnableWindow(m_Priorities.GetCount());
	((CListBox*) GetDlgItem(IDC_LIST))->ResetContent();
	if (m_Priorities.GetCount() > 0)
	{
		//---	add the states to the state list
		for (i=0; i<m_Priorities.GetCount(); i++)
			((CListBox*) GetDlgItem(IDC_LIST))->AddString(m_Priorities.GetPriority(i));

		//((CListBox*) GetDlgItem(IDC_FLAGLIST))->SetSel(m_SelState);
	}
}
