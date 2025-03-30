// StateName.cpp : implementation file
//

#include "stdafx.h"
#include "AnimTblEd.h"
#include "StateName.h"
#include "AnimTblEdGlobals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStateName dialog


CStateName::CStateName(CWnd* pParent /*=NULL*/)
	: CDialog(CStateName::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStateName)
	m_StateName = _T("");
	m_StateNamePrefix = _T("");
	//}}AFX_DATA_INIT
}


void CStateName::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStateName)
	DDX_Text(pDX, IDC_STATENAME, m_StateName);
	DDX_Text(pDX, IDC_STATEDEFINEPREFIX, m_StateNamePrefix);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStateName, CDialog)
	//{{AFX_MSG_MAP(CStateName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStateName message handlers

void CStateName::OnOK() 
{
	int i;

	GetDlgItemText(IDC_STATENAME, m_StateName);

	m_StateName.MakeUpper();
	while ((i = m_StateName.FindOneOf(" ")) != -1)
		m_StateName = m_StateName.Left(i) + "_" + m_StateName.Right(m_StateName.GetLength() - i - 1);

	SetDlgItemText(IDC_STATENAME, m_StateName);

	//---	if there are errors in the string, flag them now
	if (m_StateName.FindOneOf("`~!@#$%^&*()-+=\\|]}[{'"";:/?.>,<") >= 0)
	{
		MessageBox("Illegal characters found in string.", NULL, MB_ICONERROR);
		return;
	}

	//---	Mark the document as having changed.
	GBL_SetModifiedFlag( TRUE );
	
	CDialog::OnOK();
}
