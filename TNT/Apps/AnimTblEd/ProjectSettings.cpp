// ProjectSettings.cpp : implementation file
//

#include "stdafx.h"
#include "AnimTblEd.h"
#include "ProjectSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProjectSettings dialog


CProjectSettings::CProjectSettings(CWnd* pParent /*=NULL*/)
	: CDialog(CProjectSettings::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProjectSettings)
	m_BitFieldSize = 0;
	//}}AFX_DATA_INIT
}


void CProjectSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProjectSettings)
	DDX_Control(pDX, IDC_BASEANIMRATE, m_BaseAnimRateEdit);
	DDX_Text(pDX, IDC_BITFIELDBITS, m_BitFieldSize);
	DDV_MinMaxUInt(pDX, m_BitFieldSize, 0, 64);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProjectSettings, CDialog)
	//{{AFX_MSG_MAP(CProjectSettings)
	ON_EN_KILLFOCUS(IDC_BASEANIMRATE, OnKillfocusBaseanimrate)
	ON_EN_UPDATE(IDC_BASEANIMRATE, OnUpdateBaseanimrate)
	ON_EN_CHANGE(IDC_BASEANIMRATE, OnChangeBaseanimrate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProjectSettings message handlers

BOOL CProjectSettings::OnInitDialog() 
{
	CDialog::OnInitDialog();
	CString AnimRate, Truncate;

	AnimRate.Format("%10.8f", m_BaseAnimRate);

	AnimRate.MakeReverse();
	Truncate = AnimRate.SpanIncluding("0");
	AnimRate.MakeReverse();
	AnimRate = AnimRate.Left(AnimRate.GetLength() - Truncate.GetLength());

	m_BaseAnimRateEdit.SetWindowText(AnimRate);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CProjectSettings::OnKillfocusBaseanimrate() 
{
	CString AnimRate;
	int find, find2;

	//---	get the string from the edit box
	m_BaseAnimRateEdit.GetWindowText(AnimRate.GetBufferSetLength(20), 19);

	//---	search the string for unwanted characters
	do
	{
		find = AnimRate.FindOneOf("`~!@#$%^&*()-_+=\\|}][{PpOoIiUuYyTtRrEeWwQqAaSsDdFfGgHhJjKkLl:;""'ZzXxCcVvBbNnMm<,>?/");
		if (find != -1)
			AnimRate = AnimRate.Left(find) + AnimRate.Right(AnimRate.GetLength() - find - 1);
	} while (find != -1);

	find = AnimRate.FindOneOf(".");
	if (find != -1)
	{
		CString Right = AnimRate.Right(AnimRate.GetLength() - find);
		do
		{
			find2 = Right.FindOneOf(".");
			if (find2 != -1)
			{
				Right = Right.Left(find2) + Right.Right(Right.GetLength() - find2 - 1);
				AnimRate = AnimRate.Left(find + 1) + Right;
			}
		} while (find2 != -1);
	}
}

void CProjectSettings::OnUpdateBaseanimrate() 
{
}

void CProjectSettings::OnOK() 
{
	// TODO: Add extra validation here
	CString AnimRate;

	OnKillfocusBaseanimrate();

	//---	get the string from the edit box
	m_BaseAnimRateEdit.GetWindowText(AnimRate.GetBufferSetLength(20), 19);

	sscanf(AnimRate, "%f", &m_BaseAnimRate);
	
	CDialog::OnOK();
}

void CProjectSettings::OnChangeBaseanimrate() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
}
