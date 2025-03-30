#if !defined(AFX_GCICONPROPDLG_H__5678BA04_F9D4_4EFA_A619_2F1B5AD242EC__INCLUDED_)
#define AFX_GCICONPROPDLG_H__5678BA04_F9D4_4EFA_A619_2F1B5AD242EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GCIconPropDlg.h : header file
//

#include "MemCardProject.h"


/////////////////////////////////////////////////////////////////////////////
// CGCIconPropDlg dialog

class CGCIconPropDlg : public CDialog
{
// Construction
public:
	CGCIconPropDlg(CWnd* pParent = NULL);   // standard constructor

	void SetIconInfo( const MC_GCIconInfo& rIconInfo );
	void GetIconInfo( MC_GCIconInfo& rIconInfo ) const;

// Dialog Data
	//{{AFX_DATA(CGCIconPropDlg)
	enum { IDD = IDD_DLG_GCICON };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGCIconPropDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	MC_GCIconInfo	m_IconInfo;

	// Generated message map functions
	//{{AFX_MSG(CGCIconPropDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GCICONPROPDLG_H__5678BA04_F9D4_4EFA_A619_2F1B5AD242EC__INCLUDED_)
