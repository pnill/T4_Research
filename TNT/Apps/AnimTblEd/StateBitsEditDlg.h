#if !defined(AFX_STATEBITSEDITDLG_H__64B841E0_DCC9_11D2_8D9C_00A0CC23CE22__INCLUDED_)
#define AFX_STATEBITSEDITDLG_H__64B841E0_DCC9_11D2_8D9C_00A0CC23CE22__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StateBitsEditDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStateBitsEditDlg dialog

class CStateBitsEditDlg : public CDialog
{
// Construction
public:
	CObList m_StateClassList;
	int m_NumFlagBits;
	CStateBitsEditDlg(class CAnimTblEdDoc *pDoc, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CStateBitsEditDlg)
	enum { IDD = IDD_STATEBITSEDIT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStateBitsEditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStateBitsEditDlg)
	afx_msg void OnNewstateclass();
	afx_msg void OnDeletestateclass();
	afx_msg void OnAddstate();
	afx_msg void OnDeletestate();
	afx_msg void OnUpdateStateclassname();
	afx_msg void OnDblclkStatelist();
	virtual BOOL OnInitDialog();
	afx_msg void OnCancelMode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATEBITSEDITDLG_H__64B841E0_DCC9_11D2_8D9C_00A0CC23CE22__INCLUDED_)
