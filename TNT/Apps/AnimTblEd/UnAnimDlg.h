#if !defined(AFX_UNANIMDLG_H__E5E96220_7014_11D2_9248_00105A29F8F3__INCLUDED_)
#define AFX_UNANIMDLG_H__E5E96220_7014_11D2_9248_00105A29F8F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UnAnimDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUnAnimDlg dialog

class CUnAnimDlg : public CDialog
{
// Construction
public:
	void AddListItem(const char *lpszID);
	bool m_Visible;
	CUnAnimDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUnAnimDlg)
	enum { IDD = IDD_UNASSIGNED };
	CListBox	m_UnassignedList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUnAnimDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUnAnimDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UNANIMDLG_H__E5E96220_7014_11D2_9248_00105A29F8F3__INCLUDED_)
