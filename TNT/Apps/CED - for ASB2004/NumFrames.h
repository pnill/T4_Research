#if !defined(AFX_NUMFRAMES_H__40A201A2_03C7_11D2_9507_00207811EE70__INCLUDED_)
#define AFX_NUMFRAMES_H__40A201A2_03C7_11D2_9507_00207811EE70__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// NumFrames.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNumFrames dialog

class CNumFrames : public CDialog
{
// Construction
public:
	CString	m_Title ;

	CNumFrames(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNumFrames)
	enum { IDD = IDD_NUMFRAMES };
	int		m_NumFrames;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNumFrames)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNumFrames)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NUMFRAMES_H__40A201A2_03C7_11D2_9507_00207811EE70__INCLUDED_)
