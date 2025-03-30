// MethodEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AnimTblEd.h"
#include "MethodEditDlg.h"
#include "StateName.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMethodEditDlg dialog


CMethodEditDlg::CMethodEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMethodEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMethodEditDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMethodEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMethodEditDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMethodEditDlg, CDialog)
	//{{AFX_MSG_MAP(CMethodEditDlg)
	ON_LBN_DBLCLK(IDC_LIST, OnDblclkList)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_MODIFY, OnModify)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMethodEditDlg message handlers

BOOL CMethodEditDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	UpdateMainDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMethodEditDlg::OnDblclkList() 
{
	OnModify();
}

void CMethodEditDlg::OnAdd() 
{
	CStateName NameDlg;
	int index;

	index = ((CListBox*) GetDlgItem(IDC_LIST))->GetCurSel();
	if (index == LB_ERR)
		index = -1;

	NameDlg.m_StateName = "NEW_METHOD";
	NameDlg.m_StateNamePrefix = "ANIMMETHOD_";

	if (NameDlg.DoModal() == IDOK)
	{
		//---	if a Priority of this same name already exists, do not add this one
		if (m_Methods.FindMethod(NameDlg.m_StateName) >= 0)
		{
			this->MessageBox("Method name is not unique.", NULL, MB_ICONERROR);
			return;
		}

		//---	add this priority
		m_Methods.AddMethod(NameDlg.m_StateName, index);
		AddChange(1, index);

		UpdateMainDialog();
	}
}

void CMethodEditDlg::OnDelete() 
{
	int index = ((CListBox*) GetDlgItem(IDC_LIST))->GetCurSel();

	if (index == LB_ERR)
		return;

	m_Methods.RemoveMethod(index);

	AddChange(-1, index);

	UpdateMainDialog();
}

void CMethodEditDlg::OnModify() 
{
	CStateName NameDlg;
	CString Name;
	int f;
	int i = ((CListBox*) GetDlgItem(IDC_LIST))->GetCurSel();

	if (i == LB_ERR)
		return;

	Name = m_Methods.GetMethod(i);

	NameDlg.m_StateName = Name;
	NameDlg.m_StateNamePrefix = "ANIMMETHOD_";

	if (NameDlg.DoModal() == IDOK)
	{
		//---	if a priority of this same name already exists, do not add this one
		f = m_Methods.FindMethod(NameDlg.m_StateName);
		if ((f >= 0) && (f != i))
		{
			this->MessageBox("Method is not unique.", NULL, MB_ICONERROR);
			return;
		}

		//---	modify the priority perminently
		m_Methods.GetMethod(i) = NameDlg.m_StateName;
		UpdateMainDialog();
	}
}

void CMethodEditDlg::AddChange(int Operation, int Method)
{
	int Change;

	Change = (Operation << 16) | (Method & 0xffff);
	m_Changes.AddTail(Change);
}

void CMethodEditDlg::GetChange(int &Operation, int &Method)
{
	if (m_Changes.GetCount() <= 0)
	{
		Operation = 0;
		return;
	}

	int Change = m_Changes.GetHead();
	Operation = Change >> 16;
	Method = Change & 0xffff;

	m_Changes.RemoveAt(m_Changes.GetHeadPosition());
}

void CMethodEditDlg::UpdateMainDialog()
{
	int i;

	//---	set the read only fields
	SetDlgItemInt(IDC_TOTALSTATEBITS, m_TotalStateBits);
	SetDlgItemInt(IDC_TOTALPRIORITYBITS, m_TotalPriorityBits);
	SetDlgItemInt(IDC_TOTALFLAGBITS, m_TotalFlagBits);
	SetDlgItemInt(IDC_TOTALMETHODBITS, m_Methods.GetTotalMethodBits());
	SetDlgItemInt(IDC_REMAININGBITS, m_TotalBits - m_TotalStateBits - m_TotalPriorityBits - m_TotalFlagBits - m_Methods.GetTotalMethodBits());

	//---	enable/disable buttons
	GetDlgItem(IDC_MODIFY)->EnableWindow(m_Methods.GetCount());
	GetDlgItem(IDC_DELETE)->EnableWindow(m_Methods.GetCount());
	((CListBox*) GetDlgItem(IDC_LIST))->ResetContent();
	if (m_Methods.GetCount() > 0)
	{
		//---	add the methods to the method list
		for (i=0; i<m_Methods.GetCount(); i++)
			((CListBox*) GetDlgItem(IDC_LIST))->AddString(m_Methods.GetMethod(i));

		//((CListBox*) GetDlgItem(IDC_FLAGLIST))->SetSel(m_SelState);
	}
}
