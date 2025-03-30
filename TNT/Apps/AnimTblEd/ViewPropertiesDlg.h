#if !defined(AFX_VIEWPROPERTIESDLG_H__45453361_1B00_11D4_A627_00A0CC23CE22__INCLUDED_)
#define AFX_VIEWPROPERTIESDLG_H__45453361_1B00_11D4_A627_00A0CC23CE22__INCLUDED_

#include "DataType.h"	// Added by ClassView
#include "AnimTblEd.h"
#include "AnimTblEdDoc.h"
#include "AnimTblEdView.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewPropertiesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CViewPropertiesDlg dialog

class CViewPropertiesDlg : public CDialog
{
// Construction
public:
	void OnOK( void );
	u32 m_VPDisplayOrder[NUM_VP_OPTIONS];
	CViewPropertiesDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CViewPropertiesDlg)
	enum { IDD = IDD_VIEWPROPERTIES };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewPropertiesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CViewPropertiesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnVpAdd();
	afx_msg void OnVpRemove();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWPROPERTIESDLG_H__45453361_1B00_11D4_A627_00A0CC23CE22__INCLUDED_)
