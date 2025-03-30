#if !defined(AFX_STRINGEDIT_H__BD6B9F81_ECD2_11D2_9248_00105A29F8F3__INCLUDED_)
#define AFX_STRINGEDIT_H__BD6B9F81_ECD2_11D2_9248_00105A29F8F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StringEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStringEdit dialog

class CStringEdit : public CDialog
{
// Construction
public:
	CStringEdit(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CStringEdit)
	enum { IDD = IDD_DIALOG1 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStringEdit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStringEdit)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STRINGEDIT_H__BD6B9F81_ECD2_11D2_9248_00105A29F8F3__INCLUDED_)
