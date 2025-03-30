#if !defined(AFX_EXPORTPROGRESS_H__1A17CA80_EFDE_11D1_AEC0_00A024569FF3__INCLUDED_)
#define AFX_EXPORTPROGRESS_H__1A17CA80_EFDE_11D1_AEC0_00A024569FF3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ExportProgress.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportProgress dialog

class CExportProgress : public CDialog
{
// Construction
public:
	CExportProgress(CWnd* pParent = NULL);   // standard constructor

	void	Fmt( const char *pFormat, ... ) ;
	void	SetRange( int Min, int Max ) ;
	void	SetPos( int Pos ) ;
	void	EnableOk( bool State ) ;

	bool	m_Closed ;

// Dialog Data
	//{{AFX_DATA(CExportProgress)
	enum { IDD = IDD_EXPORT_PROGRESS };
	CButton	m_CtrlOk;
	CListBox	m_CtrlList;
	CProgressCtrl	m_CtrlProgress;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportProgress)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CExportProgress)
	virtual void OnOK();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTPROGRESS_H__1A17CA80_EFDE_11D1_AEC0_00A024569FF3__INCLUDED_)
