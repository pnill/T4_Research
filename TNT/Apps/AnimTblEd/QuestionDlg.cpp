// QuestionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AnimTblEd.h"
#include "QuestionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CQuestionDlg dialog


CQuestionDlg::CQuestionDlg(CString &Question /*"Question"*/, CWnd* pParent /*=NULL*/)
	: CDialog(CQuestionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CQuestionDlg)
	m_Question = _T("");
	//}}AFX_DATA_INIT

	m_Question = Question;
}


void CQuestionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQuestionDlg)
	DDX_Text(pDX, IDC_QUESTION, m_Question);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQuestionDlg, CDialog)
	//{{AFX_MSG_MAP(CQuestionDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQuestionDlg message handlers
