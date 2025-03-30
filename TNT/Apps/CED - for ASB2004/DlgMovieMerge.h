#if !defined(AFX_DLGMOVIEMERGE_H__A35101A0_14EF_11D2_AEC0_00A024569FF3__INCLUDED_)
#define AFX_DLGMOVIEMERGE_H__A35101A0_14EF_11D2_AEC0_00A024569FF3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DlgMovieMerge.h : header file
//

class CCeDDoc ;
class CMovie ;

/////////////////////////////////////////////////////////////////////////////
// CDlgMovieMerge dialog

class CDlgMovieMerge : public CDialog
{
//---	Attributes
public:
	CCeDDoc		*m_pDoc ;								// Document
	CMovie		*m_pMovie ;								// Current Movie
	CMovie		*m_pMergeMovie ;						// Movie To Merge in

// Construction
public:
	CDlgMovieMerge(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgMovieMerge)
	enum { IDD = IDD_MOVIEMERGE };
	CComboBox	m_Combo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgMovieMerge)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgMovieMerge)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeCombo1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGMOVIEMERGE_H__A35101A0_14EF_11D2_AEC0_00A024569FF3__INCLUDED_)
