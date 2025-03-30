#if !defined(AFX_EDITSTRINGDLG_H__BD6B9F82_ECD2_11D2_9248_00105A29F8F3__INCLUDED_)
#define AFX_EDITSTRINGDLG_H__BD6B9F82_ECD2_11D2_9248_00105A29F8F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditStringDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditStringDlg dialog

class CEditStringDlg : public CDialog
{
// Construction
public:
	CString m_EditString;
	CString m_Description;
	CString m_Title;
	CEditStringDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditStringDlg)
	enum { IDD = IDD_DIALOG_EDITSTRING };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditStringDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditStringDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITSTRINGDLG_H__BD6B9F82_ECD2_11D2_9248_00105A29F8F3__INCLUDED_)
