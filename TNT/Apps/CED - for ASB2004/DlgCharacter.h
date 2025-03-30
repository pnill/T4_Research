#if !defined(AFX_DLGCHARACTER_H__570E9700_FFC1_11D1_9507_00207811EE70__INCLUDED_)
#define AFX_DLGCHARACTER_H__570E9700_FFC1_11D1_9507_00207811EE70__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DlgCharacter.h : header file
//

class CCharacter ;
class CMotionEditView ;

/////////////////////////////////////////////////////////////////////////////
// CDlgCharacter dialog

class CDlgCharacter : public CDialog
{
//---	Attributes
public:
	CMotionEditView	*m_pView ;								// Pointer to View

//---	Operations
public:
	void		Reset( ) ;									// Reset all Combo Entries
	void		AddCharacter( CCharacter *pCharacter ) ;	// Add Character to List
	void		SelectCharacter( CCharacter *pCharacter ) ;	// Select Character in List
	void		UpdateDetails( ) ;							// Update Details for Selected Character

// Construction
public:
	CDlgCharacter(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCharacter)
	enum { IDD = IDD_CHARACTER_SELECT };
	CSpinButtonCtrl	m_ExportBitsSpin;
	CSpinButtonCtrl	m_ExportFPSSpin;
	CStatic	m_ExportBitsTitle;
	CEdit	m_ExportBits;
	CStatic	m_ExportFPSTitle;
	CEdit	m_ExportFPS;
	CButton	m_NoExport;
	CComboBox	m_Combo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCharacter)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCharacter)
	afx_msg void OnSelchangeCharacter();
	afx_msg void OnNoexport();
	afx_msg void OnKillfocusCharacterExportfps();
	afx_msg void OnChangeCharacterExportfps();
	afx_msg void OnChangeCharacterExportBits();
	afx_msg void OnKillfocusCharacterExportBits();
	afx_msg void OnDeltaposCharacterExportbitsSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposCharacterExportfpsSpin(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG

	afx_msg void OnOk();
	afx_msg void OnCancel();
	
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCHARACTER_H__570E9700_FFC1_11D1_9507_00207811EE70__INCLUDED_)
