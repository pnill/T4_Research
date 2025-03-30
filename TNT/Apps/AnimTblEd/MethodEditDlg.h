#if !defined(AFX_METHODEDITDLG_H__74AA98E1_208A_11D4_A627_00A0CC23CE22__INCLUDED_)
#define AFX_METHODEDITDLG_H__74AA98E1_208A_11D4_A627_00A0CC23CE22__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MethodEditDlg.h : header file
//

#include <afxtempl.h>
#include "method.h"

/////////////////////////////////////////////////////////////////////////////
// CMethodEditDlg dialog

class CMethodEditDlg : public CDialog
{
// Construction
public:
	CMethods m_Methods;
	int m_TotalStateBits;
	int m_TotalFlagBits;
	int m_TotalPriorityBits;
	int m_TotalBits;
	void UpdateMainDialog(void);
	void GetChange(int &Operation, int &Priority);
	CMethodEditDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMethodEditDlg)
	enum { IDD = IDD_METHODEDIT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMethodEditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMethodEditDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkList();
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	afx_msg void OnModify();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void AddChange(int Operation, int Priority);
	CList<int,int> m_Changes;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_METHODEDITDLG_H__74AA98E1_208A_11D4_A627_00A0CC23CE22__INCLUDED_)
