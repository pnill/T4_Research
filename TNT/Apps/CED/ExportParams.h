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

	CExportParams& operator = ( CExportParams& ep );

	bool	m_bNoExport;
	bool	m_bNoSave;

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
	int		m_FramesPerBlock;
	BOOL	m_ExportDataFile;
	BOOL	m_ExportHeaderFile;
	BOOL	m_ExportTextFile;
	BOOL	m_ExportHeaderFilePath;
	BOOL	m_bExportQuaternion;
	BOOL	m_bExportWorldRots;
	BOOL	m_bExportQuatBlend;
	BOOL	m_bExportQuatW;
	BOOL	m_bExportGlobalRots;
	BOOL	m_bExportPropQuatBlend;
	CString	m_MotionPrefix;
	CString	m_EventPrefix;
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
	afx_msg void OnExportdatafile();
	afx_msg void OnExportheaderfile();
	afx_msg void OnExporttextfile();
	virtual BOOL OnInitDialog();
	afx_msg void OnExportheaderpathname();
	afx_msg void OnSavenoexport();
	afx_msg void OnExportnosave();
	afx_msg void OnQuaternion();
	afx_msg void OnWorldrots();
	afx_msg void OnQuatblending();
	afx_msg void OnQuatbitsOn();
	afx_msg void OnGlobalrots();
	afx_msg void OnBonerotationhelp();
	afx_msg void OnQuatblendingforprops();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTPARAMS_H__C992D8C0_4B40_11D2_9507_444553540001__INCLUDED_)
