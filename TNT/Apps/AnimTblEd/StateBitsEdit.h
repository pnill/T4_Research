#if !defined(AFX_STATEBITSEDIT_H__A4F7C703_E046_11D2_9248_00105A29F84B__INCLUDED_)
#define AFX_STATEBITSEDIT_H__A4F7C703_E046_11D2_9248_00105A29F84B__INCLUDED_

#include <afxtempl.h>
#include "StateClassList.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StateBitsEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStateBitsEdit dialog

class CStateBitsEdit : public CDialog
{
// Construction
public:
	int m_TotalBits;
	int m_TotalPriorityBits;
	int m_TotalMethodBits;
	int StoreStateClassName(void);
	void GetChange(int &Operation, int &StateClass, int &State, int &Bit, int &Num);
	CStateBitsEdit(CWnd* pParent = NULL);   // standard constructor
	~CStateBitsEdit();   // standard destructor

	CStateClassList	m_StateClassList;
	int m_TotalFlagBits;

// Dialog Data
	//{{AFX_DATA(CStateBitsEdit)
	enum { IDD = IDD_STATEBITSEDIT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStateBitsEdit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStateBitsEdit)
	afx_msg void OnNewstateclass();
	afx_msg void OnDeletestateclass();
	afx_msg void OnAddstate();
	afx_msg void OnDeletestate();
	afx_msg void OnUpdateStateclassname();
	virtual BOOL OnInitDialog();
	afx_msg void OnModifystate();
	afx_msg void OnSelchangeStateclasstab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusStateclassname();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
//	int m_SelState;
	int m_SelTab;
	void UpdateStateClassDialog(int Index);
	void UpdateMainDialog(void);
	void AddChange(int Operation, int StateClass, int State, int Bit, int Num);
	CList<int,int> m_Changes;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATEBITSEDIT_H__A4F7C703_E046_11D2_9248_00105A29F84B__INCLUDED_)
