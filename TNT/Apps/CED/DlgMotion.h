#if !defined(AFX_DLGMOTION_H__0F94C321_783E_11D4_9398_0050DA2C723D__INCLUDED_)
#define AFX_DLGMOTION_H__0F94C321_783E_11D4_9398_0050DA2C723D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgMotion.h : header file
//

#include "Motion.h"
#include "resource.h"

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
	CEdit	m_SourceFile;
	CButton	m_Browse;
	CButton	m_Blended;
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
	void InitBlendAnimCombo	( s32 i );
	void InitBlendDialog	( void );
	void UpdateDialog		( void );
	void UpdateBlendDialog	( s32 Keeper=-1 );

	void	OnSelChangeBlendAnim( s32 iCombo );
	void	OnSelChangeWeight( s32 iWeight );
	void	SaveAndAdjustBlendedWeights( s32 Keeper );

public:
	CMotion		m_Motion;
	CMotion*	m_pMotion;

	bool		m_UpdatingBlendDialog;	// T/F is the blend dialog being updated (informs the message handling functions to ignore changes to the controls)
	bool		m_bIsMotionBlended;

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
	afx_msg void OnQuaternion();
	afx_msg void OnWorldrots();
	afx_msg void OnQuatblending();
	afx_msg void OnExportmethodSet();
	afx_msg void OnQuatbitsOn();
	afx_msg void OnGlobalrots();
	afx_msg void OnChangeBlendweight0();
	afx_msg void OnChangeBlendweight1();
	afx_msg void OnChangeBlendweight2();
	afx_msg void OnChangeBlendweight3();
	afx_msg void OnSelchangeBlendanim0();
	afx_msg void OnSelchangeBlendanim1();
	afx_msg void OnSelchangeBlendanim2();
	afx_msg void OnSelchangeBlendanim3();
	afx_msg void OnBlended();
	afx_msg void OnDestroy();
	afx_msg void OnCancelMode();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnQuatblendingforprops();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGMOTION_H__0F94C321_783E_11D4_9398_0050DA2C723D__INCLUDED_)
