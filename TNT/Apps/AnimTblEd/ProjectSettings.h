#if !defined(AFX_PROJECTSETTINGS_H__3A485562_FC83_11D2_9248_00105A29F84B__INCLUDED_)
#define AFX_PROJECTSETTINGS_H__3A485562_FC83_11D2_9248_00105A29F84B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProjectSettings.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProjectSettings dialog

class CProjectSettings : public CDialog
{
// Construction
public:
	float m_BaseAnimRate;
	CProjectSettings(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CProjectSettings)
	enum { IDD = IDD_PROJECTSETTINGS };
	CEdit	m_BaseAnimRateEdit;
	UINT	m_BitFieldSize;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProjectSettings)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CProjectSettings)
	afx_msg void OnKillfocusBaseanimrate();
	afx_msg void OnUpdateBaseanimrate();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnChangeBaseanimrate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROJECTSETTINGS_H__3A485562_FC83_11D2_9248_00105A29F84B__INCLUDED_)
