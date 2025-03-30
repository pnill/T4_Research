// GroupNodeEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AnimTblEd.h"
#include "AnimTblEdDoc.h"
#include "GroupNodeEditDlg.h"
#include "AnimTblEdGlobals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	FLAG_ID_COUNT	10

extern int FlagIDs[FLAG_ID_COUNT];
extern int StateIDs[4];
extern int StateNameIDs[4];

extern void UpdateScrollBar(CScrollBar *pScrollBar, int nSBCode);

/////////////////////////////////////////////////////////////////////////////
// CGroupNodeEditDlg dialog


CGroupNodeEditDlg::CGroupNodeEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGroupNodeEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGroupNodeEditDlg)
	m_Name = _T("");
	//}}AFX_DATA_INIT
}


void CGroupNodeEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGroupNodeEditDlg)
	DDX_Text(pDX, IDC_EDITNAME, m_Name);
	DDV_MaxChars(pDX, m_Name, 96);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGroupNodeEditDlg, CDialog)
	//{{AFX_MSG_MAP(CGroupNodeEditDlg)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_FLAG0, OnFlag0)
	ON_BN_CLICKED(IDC_FLAG1, OnFlag1)
	ON_BN_CLICKED(IDC_FLAG2, OnFlag2)
	ON_BN_CLICKED(IDC_FLAG3, OnFlag3)
	ON_BN_CLICKED(IDC_FLAG4, OnFlag4)
	ON_BN_CLICKED(IDC_FLAG5, OnFlag5)
	ON_BN_CLICKED(IDC_FLAG6, OnFlag6)
	ON_BN_CLICKED(IDC_FLAG7, OnFlag7)
	ON_CBN_SELCHANGE(IDC_STATECOMBO0, OnSelchangeStatecombo0)
	ON_CBN_SELCHANGE(IDC_STATECOMBO1, OnSelchangeStatecombo1)
	ON_CBN_SELCHANGE(IDC_STATECOMBO2, OnSelchangeStatecombo2)
	ON_CBN_SELCHANGE(IDC_STATECOMBO3, OnSelchangeStatecombo3)
	ON_BN_CLICKED(IDC_FLAG8, OnFlag8)
	ON_BN_CLICKED(IDC_FLAG9, OnFlag9)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGroupNodeEditDlg message handlers

void CGroupNodeEditDlg::OnOK() 
{
	CString	GroupName;

	//---	Get the new name from the dialog.
	this->GetDlgItemText( IDC_EDITNAME, GroupName );

	//---	Remove leading white space from the name.
	GroupName.TrimLeft();

	if (GroupName.GetLength() > 0)
	{
		//---	Save the name to the group.
		m_pGroupNode->m_Name.Format( "%s", GroupName );

		//---	Fill the comment edit box.
		this->GetDlgItemText( IDC_EDIT_COMMENT, m_pGroupNode->m_Comment );
		m_pGroupNode->m_Comment.TrimLeft();
		m_pGroupNode->m_Comment.TrimRight();

		//---	Set the node bitset data
		m_pGroupNode->BS_SetMethod(((CComboBox*) GetDlgItem(IDC_METHOD))->GetCurSel());
		m_pGroupNode->BS_SetFlags(m_Flags);
		m_pGroupNode->BS_SetPriority(((CComboBox*) GetDlgItem(IDC_PRIORITY))->GetCurSel());
		m_pGroupNode->BS_SetStates(m_States);

		//---	Mark the document as having changed.
		GBL_SetModifiedFlag( TRUE );

		CDialog::OnOK();
	}
	else
		this->MessageBox( "The group must have a name.", "Error", MB_ICONERROR );
}

BOOL CGroupNodeEditDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	//---	Set the name from the group structure.
	SetDlgItemText( IDC_EDITNAME, m_pGroupNode->m_Name );

	//---	Fill the comment edit box.
	this->SetDlgItemText( IDC_EDIT_COMMENT, m_pGroupNode->m_Comment );

#ifdef _DEBUG
	//---	Display the number of children the node has.
	{
		CString	NChildrenString;
		NChildrenString.Format( "Has %d children.", m_pGroupNode->GetNumChildren() );
		this->SetDlgItemText( IDC_STATIC_NCHILDREN, NChildrenString );
	}
#endif

	//---	get the node bitset data
	m_Method	= m_pGroupNode->BS_GetMethod();
	m_Flags		= m_pGroupNode->BS_GetFlags();
	m_States	= m_pGroupNode->BS_GetStates();
	m_Priority	= m_pGroupNode->BS_GetPriority();
	if (m_pGroupNode->m_pParent && (m_pGroupNode->m_pParent->m_Type == ANIMTREENODE_TYPE_GROUP))
	{
		m_GroupMethod	= m_pGroupNode->BS_GetGroupMethod();
		m_GroupFlags	= m_pGroupNode->BS_GetGroupFlags();
		m_GroupStates	= m_pGroupNode->BS_GetGroupStates();
		m_GroupPriority	= m_pGroupNode->BS_GetGroupPriority();
	}
	else
	{
		m_GroupMethod	= 0;
		m_GroupFlags	= 0;
		m_GroupStates	= -1;	// dont display a default
		m_GroupPriority	= -1;	// dont display a default
	}

	InitMethodDialog();
	InitFlagsDialog();
	InitStatesDialog();
	InitPriorityDialog();
	
	GetDlgItem( IDC_EDITNAME )->SetFocus();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGroupNodeEditDlg::InitMethodDialog()
{
	CMethods *pMethods;
	int c;

	//---	initialize the priority selection
	pMethods = &m_pGroupNode->GetDocument()->m_Methods;
	for (c=0; c<pMethods->GetCount(); c++)
	{
		if (c == m_GroupMethod)
			((CComboBox*) GetDlgItem(IDC_METHOD))->AddString("<def> " + pMethods->GetMethod(c));
		else
			((CComboBox*) GetDlgItem(IDC_METHOD))->AddString("          " + pMethods->GetMethod(c));
	}

	//---	set the selected priority
	((CComboBox*) GetDlgItem(IDC_METHOD))->SetCurSel(m_Method);
}

void CGroupNodeEditDlg::InitFlagsDialog()
{
	int c;

	//---	set the flag scroll bar range
	m_FlagsScrollIndex = 0;
	c = m_pGroupNode->GetDocument()->m_FlagList.GetNumFlags();
	if (c > FLAG_ID_COUNT)
	{
		((CScrollBar*) GetDlgItem(IDC_FLAGSCROLL))->SetScrollRange(0, c - FLAG_ID_COUNT);
		((CScrollBar*) GetDlgItem(IDC_FLAGSCROLL))->ShowScrollBar(TRUE);
	}
	else
		((CScrollBar*) GetDlgItem(IDC_FLAGSCROLL))->ShowScrollBar(FALSE);

	UpdateFlagsDialog();
}

void CGroupNodeEditDlg::UpdateFlagsDialog()
{
	int i;
	CFlag *pFlag;
	CButton *pButton;

	for (i=0; i<FLAG_ID_COUNT; i++)
	{
		pButton = (CButton*) GetDlgItem(FlagIDs[i]);
		pFlag = m_pGroupNode->GetDocument()->m_FlagList.GetFlag(i + m_FlagsScrollIndex);

		if (pFlag)
		{
			pButton->SetWindowText(pFlag->GetName());
			if (m_GroupFlags & pFlag->GetMask())
				pButton->SetCheck(2);
			else
				pButton->SetCheck((m_Flags & pFlag->GetMask()) ? TRUE : FALSE);
		}
		else
			pButton->ModifyStyle(WS_VISIBLE, 0);
	}

	GetDlgItem(IDC_FLAGSBOX)->Invalidate();
}

void CGroupNodeEditDlg::InitStatesDialog()
{
	int c;

	//---	set the flag scroll bar range
	m_StatesScrollIndex = 0;
	c = m_pGroupNode->GetDocument()->m_StateClassList.GetCount();
	if (c > 4)
	{
		((CScrollBar*) GetDlgItem(IDC_STATESCROLL))->SetScrollRange(0, c - 4);
		((CScrollBar*) GetDlgItem(IDC_STATESCROLL))->ShowScrollBar(TRUE);
	}
	else
		((CScrollBar*) GetDlgItem(IDC_STATESCROLL))->ShowScrollBar(FALSE);

	UpdateStatesDialog();
}

void CGroupNodeEditDlg::UpdateStatesDialog()
{
	int i, j, Default;
	CStateClass *pClass;
	CComboBox* pCombo;
	CButton* pName;

	for (i=0; i<4; i++)
	{
		pCombo = (CComboBox*)GetDlgItem(StateIDs[i]);
		pName = (CButton*) GetDlgItem(StateNameIDs[i]);

		pClass = m_pGroupNode->GetDocument()->m_StateClassList.GetStateClass(i + m_StatesScrollIndex);
		if (pClass)
		{
			pName->SetWindowText(pClass->GetName());
			pCombo->ResetContent();

			if (m_GroupStates == -1)
				Default = -1;
			else
				Default = pClass->GetSetting(m_GroupStates);

			for (j = 0; j<pClass->GetNumStates(); j++)
			{
				if (j == Default)
					pCombo->AddString("<def> " + pClass->GetState(j)->GetName());
				else
					pCombo->AddString("          " + pClass->GetState(j)->GetName());
			}

			pCombo->SetCurSel(pClass->GetSetting(m_States));
		}
		else
		{
			pCombo->ModifyStyle(WS_VISIBLE, 0);
			pName->ModifyStyle(WS_VISIBLE, 0);
		}
	}

	GetDlgItem(IDC_STATESBOX)->Invalidate();
}


void CGroupNodeEditDlg::InitPriorityDialog()
{
	CPriorities *pPriorities;
	int c;

	//---	initialize the priority selection
	pPriorities = &m_pGroupNode->GetDocument()->m_Priorities;
	for (c=0; c<pPriorities->GetCount(); c++)
	{
		if (c == m_GroupPriority)
			((CComboBox*) GetDlgItem(IDC_PRIORITY))->AddString("<def> " + pPriorities->GetPriority(c));
		else
			((CComboBox*) GetDlgItem(IDC_PRIORITY))->AddString("          " + pPriorities->GetPriority(c));
	}

	//---	set the selected priority
	((CComboBox*) GetDlgItem(IDC_PRIORITY))->SetCurSel(m_Priority);
}

void CGroupNodeEditDlg::OnFlag(int Index)
{
	CButton *pCheck = (CButton*)GetDlgItem(FlagIDs[Index]);

	//---	set the flags appropriately
	if (pCheck->GetCheck() != 0) // 0 means that it was set to 2 before being clicked so it should not be changed
	{
		if (pCheck->GetCheck() == 1)
			m_Flags |= m_pGroupNode->GetDocument()->m_FlagList.GetFlag(m_FlagsScrollIndex + Index)->GetMask();
		else
			m_Flags &= ~m_pGroupNode->GetDocument()->m_FlagList.GetFlag(m_FlagsScrollIndex + Index)->GetMask();
	}

	//---	update the flags dialog
	UpdateFlagsDialog();
}

void CGroupNodeEditDlg::OnState(int Index)
{
	CStateClass *pClass = m_pGroupNode->GetDocument()->m_StateClassList.GetStateClass(Index + m_StatesScrollIndex);
	CComboBox *pCombo = (CComboBox*)GetDlgItem(StateIDs[Index]);

	//---	set the state appropriately
	pClass->SetSetting(m_States, pCombo->GetCurSel());

	//---	update the states dialog
	UpdateStatesDialog();
}


void CGroupNodeEditDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);

	//---	update the scroll bar
	UpdateScrollBar(pScrollBar, nSBCode);

	//---	update the dialog
	if (pScrollBar == (CScrollBar*) GetDlgItem(IDC_FLAGSCROLL))
	{
		if (pScrollBar->GetScrollPos() != m_FlagsScrollIndex)
		{
			m_FlagsScrollIndex = pScrollBar->GetScrollPos();
			UpdateFlagsDialog();
		}
	}
	else if (pScrollBar == (CScrollBar*) GetDlgItem(IDC_STATESCROLL))
	{
		if (pScrollBar->GetScrollPos() != m_StatesScrollIndex)
		{
			m_StatesScrollIndex = pScrollBar->GetScrollPos();
			UpdateStatesDialog();
		}
	}
}

void CGroupNodeEditDlg::OnFlag0() 
{
	OnFlag(0);
}

void CGroupNodeEditDlg::OnFlag1() 
{
	OnFlag(1);
}

void CGroupNodeEditDlg::OnFlag2() 
{
	OnFlag(2);
}

void CGroupNodeEditDlg::OnFlag3() 
{
	OnFlag(3);
}

void CGroupNodeEditDlg::OnFlag4() 
{
	OnFlag(4);
}

void CGroupNodeEditDlg::OnFlag5() 
{
	OnFlag(5);
}

void CGroupNodeEditDlg::OnFlag6() 
{
	OnFlag(6);
}

void CGroupNodeEditDlg::OnFlag7() 
{
	OnFlag(7);
}


void CGroupNodeEditDlg::OnFlag8() 
{
	OnFlag(8);
}

void CGroupNodeEditDlg::OnFlag9() 
{
	OnFlag(9);
}

void CGroupNodeEditDlg::OnSelchangeStatecombo0() 
{
	OnState(0);
}

void CGroupNodeEditDlg::OnSelchangeStatecombo1() 
{
	OnState(1);
}

void CGroupNodeEditDlg::OnSelchangeStatecombo2() 
{
	OnState(2);
}

void CGroupNodeEditDlg::OnSelchangeStatecombo3() 
{
	OnState(3);
}
