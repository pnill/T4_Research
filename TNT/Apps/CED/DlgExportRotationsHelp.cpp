// DlgExportRotationsHelp.cpp : implementation file
//

#include "stdafx.h"
#include "ced.h"
#include "DlgExportRotationsHelp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgExportRotationsHelp dialog


CDlgExportRotationsHelp::CDlgExportRotationsHelp(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgExportRotationsHelp::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgExportRotationsHelp)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgExportRotationsHelp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgExportRotationsHelp)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgExportRotationsHelp, CDialog)
	//{{AFX_MSG_MAP(CDlgExportRotationsHelp)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgExportRotationsHelp message handlers
