#if !defined(AFX_SOURCESCANRESULTS_H__68CE03AD_C3A6_4A48_8273_6ABEA7DE139C__INCLUDED_)
#define AFX_SOURCESCANRESULTS_H__68CE03AD_C3A6_4A48_8273_6ABEA7DE139C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SourceScanResults.h : header file
//

class CScanSourceCodeDlg;

/////////////////////////////////////////////////////////////////////////////
// CSourceScanResults dialog

class CSourceScanResults : public CDialog
{
// Construction
public:
	CSourceScanResults(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSourceScanResults)
	enum { IDD = IDD_SCANRESULTS };
	int		m_NumFiles;
	int		m_NumKeywords;
	UINT	m_NumLOC;
	int		m_NumKeywordsFound;
	//}}AFX_DATA

    void Initialize( const CScanSourceCodeDlg *scanDlg );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSourceScanResults)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

    const CScanSourceCodeDlg *m_ScanDlg;

    virtual BOOL OnInitDialog();
    virtual void OnOK();

	// Generated message map functions
	//{{AFX_MSG(CSourceScanResults)
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOURCESCANRESULTS_H__68CE03AD_C3A6_4A48_8273_6ABEA7DE139C__INCLUDED_)
