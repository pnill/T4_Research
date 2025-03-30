#if !defined(AFX_PRIORITIESEDIT_H__A4F7C708_E046_11D2_9248_00105A29F84B__INCLUDED_)
#define AFX_PRIORITIESEDIT_H__A4F7C708_E046_11D2_9248_00105A29F84B__INCLUDED_

#include <afxtempl.h>
#include "priorities.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PrioritiesEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPrioritiesEdit dialog

class CPrioritiesEdit : public CDialog
{
// Construction
public:
	CPriorities m_Priorities;
	int m_TotalStateBits;
	int m_TotalFlagBits;
	int m_TotalMethodBits;
	int m_TotalBits;
	void UpdateMainDialog(void);
	void GetChange(int &Operation, int &Priority);
	CPrioritiesEdit(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrioritiesEdit)
	enum { IDD = IDD_PRIORITYEDIT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrioritiesEdit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrioritiesEdit)
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	afx_msg void OnModify();
	afx_msg void OnDblclkList();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void AddChange(int Operation, int Priority);
	CList<int,int> m_Changes;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRIORITIESEDIT_H__A4F7C708_E046_11D2_9248_00105A29F84B__INCLUDED_)
