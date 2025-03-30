#if !defined(AFX_STATENAME_H__A4F7C704_E046_11D2_9248_00105A29F84B__INCLUDED_)
#define AFX_STATENAME_H__A4F7C704_E046_11D2_9248_00105A29F84B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StateName.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStateName dialog

class CStateName : public CDialog
{
// Construction
public:
	CStateName(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CStateName)
	enum { IDD = IDD_EDITSTATENAME };
	CString	m_StateName;
	CString	m_StateNamePrefix;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStateName)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStateName)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATENAME_H__A4F7C704_E046_11D2_9248_00105A29F84B__INCLUDED_)
