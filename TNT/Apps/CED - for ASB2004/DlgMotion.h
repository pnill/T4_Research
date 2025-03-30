#if !defined(AFX_DLGMOTION_H__0F94C321_783E_11D4_9398_0050DA2C723D__INCLUDED_)
#define AFX_DLGMOTION_H__0F94C321_783E_11D4_9398_0050DA2C723D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgMotion.h : header file
//

#include "Motion.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgMotion dialog

class CDlgMotion : public CDialog
{
// Construction
public:
	CDlgMotion(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgMotion)
	enum { IDD = IDD_MOTION };
	CButton	m_ExportBitsButton;
	CEdit	m_ExportBits;
	CEdit	m_ExportFrameRate;
	CButton	m_ExportFrameRateButton;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgMotion)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void OnOK( void );

public:
	CMotion	m_Motion;

	// Generated message map functions
	//{{AFX_MSG(CDlgMotion)
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowse();
	afx_msg void OnExport();
	afx_msg void OnExportFramerateSet();
	afx_msg void OnChangeExportFramerate();
	afx_msg void OnKillfocusExportFramerate();
	afx_msg void OnExportBitsSet();
	afx_msg void OnChangeExportBits();
	afx_msg void OnKillfocusExportBits();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGMOTION_H__0F94C321_783E_11D4_9398_0050DA2C723D__INCLUDED_)
