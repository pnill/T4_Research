#if !defined(AFX_DLGMOVIEPREVIEW_H__82637301_BE37_11D2_A3E0_00104B5EB1F8__INCLUDED_)
#define AFX_DLGMOVIEPREVIEW_H__82637301_BE37_11D2_A3E0_00104B5EB1F8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgMoviePreview.h : header file
//

class CMovieView ;
class CMovie ;

/////////////////////////////////////////////////////////////////////////////
// CDlgMoviePreview dialog

class CDlgMoviePreview : public CDialog
{
//---	Attributes
public:
	CMovieView		*m_pView ;								// Pointer to View
	CMovie			*m_pMovie ;								// Pointer to Movie

//---	Operations
public:

// Construction
public:
	CDlgMoviePreview(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgMoviePreview)
	enum { IDD = IDD_MOVIEPREVIEW };
	CEdit	m_CtrlPreviewH;
	CEdit	m_CtrlPreviewW;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgMoviePreview)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgMoviePreview)
	afx_msg void OnPlay();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangePreviewW();
	afx_msg void OnChangePreviewH();
	//}}AFX_MSG
	
    afx_msg void OnOk();
	afx_msg void OnCancel();

    DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGMOVIEPREVIEW_H__82637301_BE37_11D2_A3E0_00104B5EB1F8__INCLUDED_)
