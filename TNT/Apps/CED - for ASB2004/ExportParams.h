#if !defined(AFX_EXPORTPARAMS_H__C992D8C0_4B40_11D2_9507_444553540001__INCLUDED_)
#define AFX_EXPORTPARAMS_H__C992D8C0_4B40_11D2_9507_444553540001__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportParams.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportParams dialog

class CExportParams : public CDialog
{
// Construction
public:
	CExportParams(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExportParams)
	enum { IDD = IDD_EXPORT_DIALOG };
	BOOL	m_OptForceUpper;
	BOOL	m_EnableGAMECUBE;
	BOOL	m_EnablePC;
	BOOL	m_EnableXBOX;
	BOOL	m_EnablePS2;
	CString	m_FileGAMECUBE;
	CString	m_FilePC;
	CString	m_FileXBOX;
	CString	m_FilePS2;
	CString	m_CharacterIncludeFile;
	BOOL	m_OptCollisionBubbles;
	BOOL	m_OptMotionFlags;
	BOOL	m_OptFrontEndSpecific;
	int		m_FramesPerBlock;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportParams)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CExportParams)
	afx_msg void OnExportBrowseGAMECUBE();
	afx_msg void OnExportBrowsePC();
	afx_msg void OnExportBrowseXBOX();
	afx_msg void OnExportBrowsePS2();
	afx_msg void OnExportEnableGAMECUBE();
	afx_msg void OnExportEnablePC();
	afx_msg void OnExportEnableXBOX();
	afx_msg void OnExportEnablePS2();
	afx_msg void OnExportBrowseGlobal();
	afx_msg void OnChangeExportFramesperblock();
	afx_msg void OnButton1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTPARAMS_H__C992D8C0_4B40_11D2_9507_444553540001__INCLUDED_)
