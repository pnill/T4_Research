#if !defined(AFX_SOURCEFILESDLG_H__B889FD22_5873_11D2_ABF7_00A024569875__INCLUDED_)
#define AFX_SOURCEFILESDLG_H__B889FD22_5873_11D2_ABF7_00A024569875__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SourceFilesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSourceFilesDlg dialog

class CSourceFilesDlg : public CDialog
{
// Construction
public:
	bool FileInList(const char *lpszFile);
	CString m_FilesString;
	bool m_Modified;
	CSourceFilesDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSourceFilesDlg)
	enum { IDD = IDD_SOURCEFILES };
	CListBox	m_FileList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSourceFilesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSourceFilesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOURCEFILESDLG_H__B889FD22_5873_11D2_ABF7_00A024569875__INCLUDED_)
