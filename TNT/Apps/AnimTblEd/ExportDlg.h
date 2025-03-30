#if !defined(AFX_EXPORTDLG_H__32680741_AA36_11D2_9248_00105A29F8F3__INCLUDED_)
#define AFX_EXPORTDLG_H__32680741_AA36_11D2_9248_00105A29F8F3__INCLUDED_

#include "ExportDef.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportDlg dialog

class CExportDlg : public CDialog
{
// Construction
public:
	void BuildExportList();
	class CAnimTblEdDoc * m_pDoc;
	void SetAllCaps(bool Set);
	CExportDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExportDlg)
	enum { IDD = IDD_EXPORT };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CExportDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonNewexport();
	afx_msg void OnButtonExport();
	afx_msg void OnButtonEdit();
	afx_msg void OnButtonDelete();
	afx_msg void OnDblclkListExport();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTDLG_H__32680741_AA36_11D2_9248_00105A29F8F3__INCLUDED_)
