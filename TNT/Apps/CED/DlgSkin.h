#if !defined(AFX_DLGSKIN_H__4433690D_E8B6_4B08_8585_05C798501643__INCLUDED_)
#define AFX_DLGSKIN_H__4433690D_E8B6_4B08_8585_05C798501643__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgSkin.h : header file
//

class CMotionEditView;
class CSkin;

/////////////////////////////////////////////////////////////////////////////
// CDlgSkin dialog

class CDlgSkin : public CDialog
{
//---	Attributes
public:
	CMotionEditView	*m_pView ;								// Pointer to View

//---	Operations
public:
	void		Reset( ) ;									// Reset all Combo Entries
	void		AddSkin( CSkin *pSkin ) ;					// Add Skin to List
	void		SelectSkin( CSkin *pSkin ) ;				// Select Skin in List
	void		UpdateDetails( ) ;							// Update Details for Selected Skin

// Construction
public:
	CDlgSkin(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgSkin)
	enum { IDD = IDD_SKIN_SELECT };
	CComboBox	m_Combo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgSkin)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgSkin)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeSkin();
	afx_msg void OnTranslucent();
	afx_msg void OnImport();
	afx_msg void OnDelete();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSKIN_H__4433690D_E8B6_4B08_8585_05C798501643__INCLUDED_)
