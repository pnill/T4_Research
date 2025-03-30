// StateBitsEdit.cpp : implementation file
//

#include "stdafx.h"
#include "AnimTblEd.h"
#include "AnimTblEdDoc.h"
#include "StateBitsEdit.h"
#include "StateName.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStateBitsEdit dialog


CStateBitsEdit::CStateBitsEdit(CWnd* pParent /*=NULL*/)
	: CDialog(CStateBitsEdit::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStateBitsEdit)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CStateBitsEdit::~CStateBitsEdit()
{
}


void CStateBitsEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStateBitsEdit)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStateBitsEdit, CDialog)
	//{{AFX_MSG_MAP(CStateBitsEdit)
	ON_BN_CLICKED(IDC_DELETESTATE, OnDeletestate)
	ON_BN_CLICKED(IDC_DELETESTATECLASS, OnDeletestateclass)
	ON_BN_CLICKED(IDC_MODIFYSTATE, OnModifystate)
	ON_BN_CLICKED(IDC_NEWSTATECLASS, OnNewstateclass)
	ON_BN_CLICKED(IDC_ADDSTATE, OnAddstate)
	ON_NOTIFY(TCN_SELCHANGE, IDC_STATECLASSTAB, OnSelchangeStateclasstab)
	ON_EN_KILLFOCUS(IDC_STATECLASSNAME, OnKillfocusStateclassname)
	ON_EN_UPDATE(IDC_STATECLASSNAME, OnUpdateStateclassname)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStateBitsEdit message handlers

BOOL CStateBitsEdit::OnInitDialog() 
{
	CDialog::OnInitDialog();

	//---	if there are states set the selected tab to zero, otherwise set it to -1
	if (m_StateClassList.GetCount() > 0)
		m_SelTab = 0;
	else
		m_SelTab = -1;

	//---	update the main dialog
	UpdateMainDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CStateBitsEdit::OnNewstateclass() 
{
	//---	create the new state class
	m_SelTab = m_StateClassList.AddStateClass();	

	//---	select and display this tab
	UpdateMainDialog();

	GetDlgItem(IDC_STATECLASSNAME)->SetFocus();
	((CEdit*) GetDlgItem(IDC_STATECLASSNAME))->SetSel(0, ((CEdit*) GetDlgItem(IDC_STATECLASSNAME))->LineLength());
}

void CStateBitsEdit::OnDeletestateclass() 
{
	if (m_SelTab < 0)
		return;

	CStateClass *pClass = m_StateClassList.GetStateClass(m_SelTab);
	AddChange(-2, m_SelTab, 0, pClass->GetStartBit(), pClass->GetNumBits());

	//---	delete the selected state class
	m_StateClassList.RemoveStateClass(m_SelTab);
	if (m_StateClassList.GetCount() <= m_SelTab)
		m_SelTab = m_StateClassList.GetCount() - 1;

	//---	select and display this tab
	UpdateMainDialog();
}

void CStateBitsEdit::OnAddstate() 
{
	CStateName NameDlg;
	int nBits;
	int index;
	CString Name;

	index = ((CListBox*) GetDlgItem(IDC_STATELIST))->GetCurSel();
	if (index == LB_ERR)
		index = -1;

	NameDlg.m_StateName = "NEW_NAME";
	NameDlg.m_StateNamePrefix = "ANIMSTATE_";
	Name = m_StateClassList.GetStateClass(m_SelTab)->GetName();
	if (Name.GetLength())
		NameDlg.m_StateNamePrefix += m_StateClassList.GetStateClass(m_SelTab)->GetName() + "_";

	if (NameDlg.DoModal() == IDOK)
	{
		CStateClass *pClass = m_StateClassList.GetStateClass(m_SelTab);

		nBits = pClass->GetNumBits();

		//---	if a state of this same name already exists, do not add this one
		if (pClass->FindState(NameDlg.m_StateName))
		{
			this->MessageBox("State name is not unique.", NULL, MB_ICONERROR);
			return;
		}

		//---	add this state
		pClass->AddState(NameDlg.m_StateName, index);

		//---	if the number of bits has changed, add or delete bits to fit new size
		if (nBits != pClass->GetNumBits())
		{
			//---	make sure that there are enough bits left to allow this addition
			if (GetDlgItemInt(IDC_REMAININGBITS) == 0)
			{
				this->MessageBox("Not enough bits remaining to add this state!", NULL, MB_ICONERROR);
				pClass->RemoveState(pClass->GetNumStates() - 1);
				return;
			}

			AddChange(2, m_SelTab, 0, pClass->GetStartBit() + nBits, 1);
			UpdateMainDialog();
		}
		else
		{
			UpdateStateClassDialog(m_SelTab);
		}

		AddChange(1, m_SelTab, index, pClass->GetStartBit(), pClass->GetNumBits());
	}
}

void CStateBitsEdit::OnModifystate() 
{
	CStateName NameDlg;
	int i = ((CListBox*) GetDlgItem(IDC_STATELIST))->GetCurSel();
	CStateClass *pClass = m_StateClassList.GetStateClass(m_SelTab);
	CState *pState;

	if (i == LB_ERR)
		return;

	pState = pClass->GetState(i);
	NameDlg.m_StateName = pState->GetName();
	NameDlg.m_StateNamePrefix = "ANIMSTATE_" + m_StateClassList.GetStateClass(m_SelTab)->GetName() + "_";

	if (NameDlg.DoModal() == IDOK)
	{
		//---	if a state of this same name already exists, do not add this one
		if (pClass->FindState(NameDlg.m_StateName, pState))
		{
			this->MessageBox("State name is not unique.", NULL, MB_ICONERROR);
			pClass->RemoveState(pClass->GetNumStates() - 1);
			return;
		}

		pState->SetName(NameDlg.m_StateName);

		UpdateStateClassDialog(m_SelTab);
	}
}

void CStateBitsEdit::OnDeletestate() 
{
	int nBits;
	int i = ((CListBox*) GetDlgItem(IDC_STATELIST))->GetCurSel();
	CStateClass *pClass = m_StateClassList.GetStateClass(m_SelTab);

	if (i == LB_ERR)
		return;

	nBits = pClass->GetNumBits();

	AddChange(-1, m_SelTab, i, pClass->GetStartBit(), nBits);
	pClass->RemoveState(i);

	//---	if the number of bits has changed, add or delete bits to fit new size
	if (nBits != pClass->GetNumBits())
	{
		AddChange(-2, m_SelTab, 0, pClass->GetStartBit() + pClass->GetNumBits(), 1);
		UpdateMainDialog();
	}
	else
		UpdateStateClassDialog(m_SelTab);
}

void CStateBitsEdit::OnKillfocusStateclassname() 
{
	if (StoreStateClassName())
		UpdateMainDialog();
}

void CStateBitsEdit::OnUpdateStateclassname() 
{
}

void CStateBitsEdit::OnSelchangeStateclasstab(NMHDR* pNMHDR, LRESULT* pResult) 
{
	static InThisFunc=FALSE;

	if (InThisFunc)
		return;
	InThisFunc = TRUE;

	if (GetFocus() == GetDlgItem(IDC_STATECLASSNAME))
	{
		if (!StoreStateClassName())
		{
			((CTabCtrl*) GetDlgItem(IDC_STATECLASSTAB))->SetCurSel(m_SelTab);
			InThisFunc = FALSE;
			UpdateStateClassDialog(m_SelTab);
			return;
		}
	}

	m_SelTab = ((CTabCtrl*) GetDlgItem(IDC_STATECLASSTAB))->GetCurSel();
	UpdateMainDialog();

	*pResult = 0;
	InThisFunc = FALSE;
}

void CStateBitsEdit::GetChange(int &Operation, int &StateClass, int &State, int &Bit, int &Num)
{
	if (m_Changes.GetCount() <= 0)
	{
		Operation = 0;
		return;
	}

	int Change = m_Changes.GetHead();
	Operation = Change >> 28;
	StateClass = (Change >> 20) & 0xff;
	State = (Change >> 12) & 0xff;
	Bit	= (Change >> 6) & 0x3f;
	Num = Change & 0x3f;

	m_Changes.RemoveAt(m_Changes.GetHeadPosition());
}

void CStateBitsEdit::AddChange(int Operation, int StateClass, int State, int Bit, int Num)
{
	int Change;

	Change = (Operation << 28) | ((StateClass & 0xff) << 20) | ((State & 0xff) << 12) | ((Bit & 0x3f) << 6) | (Num & 0x3f);
	m_Changes.AddTail(Change);
}

void CStateBitsEdit::UpdateMainDialog()
{
	int i;

	//---	set the read only fields
	SetDlgItemInt(IDC_TOTALSTATEBITS, m_StateClassList.GetTotalStateBits());
	SetDlgItemInt(IDC_TOTALPRIORITYBITS, m_TotalPriorityBits);
	SetDlgItemInt(IDC_TOTALFLAGBITS, m_TotalFlagBits);
	SetDlgItemInt(IDC_TOTALMETHODBITS2, m_TotalMethodBits);
	SetDlgItemInt(IDC_REMAININGBITS, m_TotalBits - m_TotalFlagBits - m_TotalPriorityBits - m_StateClassList.GetTotalStateBits() - m_TotalMethodBits);

	//---	deactivate the delete button if there are no classes
	GetDlgItem(IDC_DELETESTATECLASS)->EnableWindow(m_StateClassList.GetCount());

	//---	set up the new tabs
	((CTabCtrl*) GetDlgItem(IDC_STATECLASSTAB))->DeleteAllItems();
	if (m_StateClassList.GetCount())
	{
		//---	make the tabs
		for (i=0; i<m_StateClassList.GetCount(); i++)
			((CTabCtrl*) GetDlgItem(IDC_STATECLASSTAB))->InsertItem(i, m_StateClassList.GetStateClass(i)->GetName());
	}
	else
	{
		//---	make a tab
		((CTabCtrl*) GetDlgItem(IDC_STATECLASSTAB))->InsertItem(0, "No State Class");
	}

	//---	update the state class dialog by the selected tabs
	UpdateStateClassDialog(m_SelTab);
}

void CStateBitsEdit::UpdateStateClassDialog(int Index)
{
	int i;
	int Enable;

	Enable = (Index >= 0);

	GetDlgItem(IDC_STATECLASSTAB)->EnableWindow(Enable);
	GetDlgItem(IDC_STATECLASSNAME)->EnableWindow(Enable);
	GetDlgItem(IDC_STATELIST)->EnableWindow(Enable);
	GetDlgItem(IDC_STATEDEFINEPREFIX)->EnableWindow(Enable);
	GetDlgItem(IDC_STATICNAME)->EnableWindow(Enable);
	GetDlgItem(IDC_STATICSTATES)->EnableWindow(Enable);
	GetDlgItem(IDC_ADDSTATE)->EnableWindow(Enable);
	GetDlgItem(IDC_MODIFYSTATE)->EnableWindow(Enable);
	GetDlgItem(IDC_DELETESTATE)->EnableWindow(Enable);
	GetDlgItem(IDC_STATIC_)->EnableWindow(Enable);
	GetDlgItem(IDC_STATICNUMSTATES)->EnableWindow(Enable);
	GetDlgItem(IDC_STATICSTARTBIT)->EnableWindow(Enable);
	GetDlgItem(IDC_STATICNUMBITS)->EnableWindow(Enable);
	GetDlgItem(IDC_NUMSTATES)->EnableWindow(Enable);
	GetDlgItem(IDC_STARTBIT)->EnableWindow(Enable);
	GetDlgItem(IDC_NUMBITS)->EnableWindow(Enable);
	this->Invalidate();

	((CListBox*) GetDlgItem(IDC_STATELIST))->ResetContent();
	if (Enable)
	{
		CStateClass *pClass = m_StateClassList.GetStateClass(Index);
		CState *pState;

		//---	enable/disable buttons
		GetDlgItem(IDC_MODIFYSTATE)->EnableWindow(pClass->GetNumStates());
		GetDlgItem(IDC_DELETESTATE)->EnableWindow(pClass->GetNumStates());

		//---	select the first tab
		((CTabCtrl*) GetDlgItem(IDC_STATECLASSTAB))->SetCurSel(m_SelTab);

		//---	set the read only fields
		SetDlgItemInt(IDC_NUMSTATES, pClass->GetNumStates());
		SetDlgItemInt(IDC_STARTBIT, pClass->GetStartBit());
		SetDlgItemInt(IDC_NUMBITS, pClass->GetNumBits());

		//---	set the name
		SetDlgItemText(IDC_STATECLASSNAME, pClass->GetName());

		if (pClass->GetNumStates() > 0)
		{
			//---	add the states to the state list
			for (i=0; i<pClass->GetNumStates(); i++)
			{
				pState = pClass->GetState(i);
				((CListBox*) GetDlgItem(IDC_STATELIST))->AddString(pState->GetName());
			}

			//((CListBox*) GetDlgItem(IDC_STATELIST))->SetSel(m_SelState);
		}
	}
}


int CStateBitsEdit::StoreStateClassName()
{
	int i;
	CString Name;
	CStateClass *pClass = m_StateClassList.GetStateClass(m_SelTab);

	GetDlgItemText(IDC_STATECLASSNAME, Name);

	Name.MakeUpper();
	while ((i = Name.FindOneOf(" ")) >= 0)
		Name = Name.Left(i) + "_" + Name.Right(Name.GetLength() - i - 1);

	SetDlgItemText(IDC_STATECLASSNAME, Name);

	//---	make sure that now illegal characters exist in the string
	if (Name.FindOneOf("`~!@#$%^&*()-=+\\|}][{""';:?/.><,") >= 0)
	{
		MessageBox("An illegal character was found in the name.", NULL, MB_ICONERROR);
		GetDlgItem(IDC_STATECLASSNAME)->SetFocus();
		((CEdit*) GetDlgItem(IDC_STATECLASSNAME))->SetSel(0, ((CEdit*) GetDlgItem(IDC_STATECLASSNAME))->LineLength());
		return FALSE;
	}

	//---	make sure that it is unique
	if (m_StateClassList.FindStateClass(Name, pClass))
	{
		this->MessageBox("State classification must have a unique name!", NULL, MB_ICONERROR);
		GetDlgItem(IDC_STATECLASSNAME)->SetFocus();
		((CEdit*) GetDlgItem(IDC_STATECLASSNAME))->SetSel(0, ((CEdit*) GetDlgItem(IDC_STATECLASSNAME))->LineLength());
		return FALSE;
	}

	m_StateClassList.SetStateClassName(m_SelTab, Name);
	return TRUE;
}
