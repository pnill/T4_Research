#if !defined(AFX_FLAGSEDITDLG_H__A4F7C705_E046_11D2_9248_00105A29F84B__INCLUDED_)
#define AFX_FLAGSEDITDLG_H__A4F7C705_E046_11D2_9248_00105A29F84B__INCLUDED_

#include <afxtempl.h>
#include "FlagList.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FlagsEditDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// FlagsEditDlg dialog

class CFlagsEditDlg : public CDialog
{
// Construction
public:
	int m_TotalBits;
	int m_TotalPriorityBits;
	int m_TotalMethodBits;
	void GetChange(int &Operation, int &Flag);
	void UpdateMainDialog(void);
	int m_TotalStateBits;
	CFlagList m_FlagList;
	CFlagsEditDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(FlagsEditDlg)
	enum { IDD = IDD_FLAGEDIT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(FlagsEditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(FlagsEditDlg)
	afx_msg void OnAddflag();
	afx_msg void OnDeleteflag();
	afx_msg void OnModifyflag();
	afx_msg void OnDblclkFlaglist();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void AddChange(int Operation, int Flag);
	CList<int,int> m_Changes;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FLAGSEDITDLG_H__A4F7C705_E046_11D2_9248_00105A29F84B__INCLUDED_)
