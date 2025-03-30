#if !defined(AFX_GROUPNODEEDITDLG_H__03F3D900_788D_11D2_9248_00105A29F8F3__INCLUDED_)
#define AFX_GROUPNODEEDITDLG_H__03F3D900_788D_11D2_9248_00105A29F8F3__INCLUDED_

#include <afxtempl.h>
#include "GroupNode.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GroupNodeEditDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGroupNodeEditDlg dialog

class CGroupNodeEditDlg : public CDialog
{
// Construction
public:
	CGroupNode *m_pGroupNode;
	CGroupNodeEditDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGroupNodeEditDlg)
	enum { IDD = IDD_GROUPEDIT };
	CString	m_Name;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGroupNodeEditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGroupNodeEditDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnFlag0();
	afx_msg void OnFlag1();
	afx_msg void OnFlag2();
	afx_msg void OnFlag3();
	afx_msg void OnFlag4();
	afx_msg void OnFlag5();
	afx_msg void OnFlag6();
	afx_msg void OnFlag7();
	afx_msg void OnSelchangeStatecombo0();
	afx_msg void OnSelchangeStatecombo1();
	afx_msg void OnSelchangeStatecombo2();
	afx_msg void OnSelchangeStatecombo3();
	afx_msg void OnFlag8();
	afx_msg void OnFlag9();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void InitMethodDialog(void);
	void InitFlagsDialog(void);
	void UpdateFlagsDialog(void);
	void InitStatesDialog(void);
	void UpdateStatesDialog(void);
	void InitPriorityDialog(void);
	void OnFlag(int Index);
	void OnState(int Index);

	int m_StatesScrollIndex;
	int m_FlagsScrollIndex;
	int m_Method;
	int m_Flags;
	int m_States;
	int m_Priority;
	int m_GroupMethod;
	int m_GroupFlags;
	int m_GroupStates;
	int m_GroupPriority;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GROUPNODEEDITDLG_H__03F3D900_788D_11D2_9248_00105A29F8F3__INCLUDED_)

