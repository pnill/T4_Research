#if !defined(AFX_DLGEXPORTROTATIONSHELP_H__4EEACF7D_B299_4441_A40B_673DE9D8D5AC__INCLUDED_)
#define AFX_DLGEXPORTROTATIONSHELP_H__4EEACF7D_B299_4441_A40B_673DE9D8D5AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgExportRotationsHelp.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgExportRotationsHelp dialog

class CDlgExportRotationsHelp : public CDialog
{
// Construction
public:
	CDlgExportRotationsHelp(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgExportRotationsHelp)
	enum { IDD = IDD_EXPORTROTATIONSHELP };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgExportRotationsHelp)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgExportRotationsHelp)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGEXPORTROTATIONSHELP_H__4EEACF7D_B299_4441_A40B_673DE9D8D5AC__INCLUDED_)
