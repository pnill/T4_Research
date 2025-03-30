#if !defined(AFX_DLGPROPS_H__95031DF5_4154_40F4_90FA_DDE9E984EA64__INCLUDED_)
#define AFX_DLGPROPS_H__95031DF5_4154_40F4_90FA_DDE9E984EA64__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgProps.h : header file
//

class CMotionEditView;

/////////////////////////////////////////////////////////////////////////////
// CDlgProps dialog

class CDlgProps : public CDialog
{
public:

	CMotionEditView*	m_pView;
	void	UpdateDetails( bool bNewMotionSelected=FALSE );

protected:
	void	UpdatePropList( bool bNewMotionSelected );
	void	UpdatePropData( void );
	void	UpdateLinkMotionList( void );

// Construction
public:
	CDlgProps(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgProps)
	enum { IDD = IDD_PROPS };
	CStatic	m_LinkMotionComboTitle;
	CComboBox	m_PropSelectCombo;
	CComboBox	m_LinkMotionCombo;
	CButton	m_Export;
	CButton	m_Active;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgProps)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgProps)
	afx_msg void OnActive();
	afx_msg void OnExport();
	afx_msg void OnSelchangeLinkmotion();
	afx_msg void OnSelchangePropselect();
	afx_msg void OnEditdef();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGPROPS_H__95031DF5_4154_40F4_90FA_DDE9E984EA64__INCLUDED_)
