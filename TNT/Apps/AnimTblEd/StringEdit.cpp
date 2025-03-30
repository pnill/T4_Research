// StringEdit.cpp : implementation file
//

#include "stdafx.h"
#include "AnimTblEd.h"
#include "StringEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStringEdit dialog


CStringEdit::CStringEdit(CWnd* pParent /*=NULL*/)
	: CDialog(CStringEdit::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStringEdit)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CStringEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStringEdit)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStringEdit, CDialog)
	//{{AFX_MSG_MAP(CStringEdit)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStringEdit message handlers
