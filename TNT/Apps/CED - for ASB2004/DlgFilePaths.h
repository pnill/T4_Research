#if !defined(AFX_DLGFILEPATHS_H__0F94C324_783E_11D4_9398_0050DA2C723D__INCLUDED_)
#define AFX_DLGFILEPATHS_H__0F94C324_783E_11D4_9398_0050DA2C723D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgFilePaths.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgFilePaths dialog

class CDlgFilePaths : public CDialog
{
// Construction
public:
	CDlgFilePaths(CWnd* pParent = NULL);   // standard constructor
	void SetRadioButtons( int Where );

// Dialog Data
	//{{AFX_DATA(CDlgFilePaths)
	enum { IDD = IDD_FILEPATHS };
	CString	m_ReplaceString;
	CString	m_SearchString;
	//}}AFX_DATA

	int m_WhereToLook;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgFilePaths)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgFilePaths)
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioAnywhere();
	afx_msg void OnRadioFromend();
	afx_msg void OnRadioFromstart();
	afx_msg void OnBrowsesearch();
	afx_msg void OnBrowsereplace();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGFILEPATHS_H__0F94C324_783E_11D4_9398_0050DA2C723D__INCLUDED_)
