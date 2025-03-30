#if !defined(AFX_EQUIVDLG_H__007C0F02_D7D2_11D2_9248_00105A29F8F3__INCLUDED_)
#define AFX_EQUIVDLG_H__007C0F02_D7D2_11D2_9248_00105A29F8F3__INCLUDED_

#include "IDList.h"	// Added by ClassView
#include "AnimTblEdDoc.h"	// Added by ClassView
#include "EquivNode.h"	// Added by ClassView

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EquivDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEquivDlg dialog

class CEquivDlg : public CDialog
{
// Construction
public:
	void ShowData();
	CIDList *m_pIDList;
	CString m_OldRightID;
	CString m_OldLeftID;
	CEquivNode * m_pEquivNode;
	CEquivDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEquivDlg)
	enum { IDD = IDD_EQUIVALENCYEDIT };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEquivDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEquivDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnButtonLeftchangeid();
	afx_msg void OnButtonRightchangeid();
	afx_msg void OnButtonSwap();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EQUIVDLG_H__007C0F02_D7D2_11D2_9248_00105A29F8F3__INCLUDED_)
