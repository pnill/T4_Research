#if !defined(AFX_EXPORTCONFIGDLG_H__93592620_394B_11D3_9248_00105A29F8F3__INCLUDED_)
#define AFX_EXPORTCONFIGDLG_H__93592620_394B_11D3_9248_00105A29F8F3__INCLUDED_

#include "ExportDef.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportConfigDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportConfigDlg dialog

class CExportConfigDlg : public CDialog
{
// Construction
public:
	CExportDef m_ExportDef;
	CExportConfigDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExportConfigDlg)
	enum { IDD = IDD_EXPORTCONFIG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CExportConfigDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnButtonChangepath();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTCONFIGDLG_H__93592620_394B_11D3_9248_00105A29F8F3__INCLUDED_)
