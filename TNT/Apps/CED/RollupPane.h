#if !defined(AFX_ROLLUPPANE_H__2C1E6503_FBEF_11D1_9507_00207811EE70__INCLUDED_)
#define AFX_ROLLUPPANE_H__2C1E6503_FBEF_11D1_9507_00207811EE70__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// RollupPane.h : header file
//

class CRollupCtrl ;

/////////////////////////////////////////////////////////////////////////////
// CRollupPane window

class CRollupPane : public CWnd
{
// Construction
public:
	CRollupPane();

// Attributes
public:
	CRollupCtrl	*m_pRollupCtrl ;			// Owner Rollup Ctrl
	CDialog		*m_pDialog ;				// Dialog for this Pane
	bool		m_Open ;					// true = Open, false = Closed
	bool		m_ButtonPressed ;			// Button is Pressed

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRollupPane)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CRollupPane();

	int		GetHeight		( ) ;				// Return Height of RollupPane
	bool	IsOpen			( ) ;				// true if control open
	bool	IsClosed		( ) ;				// true if control closed
	bool	SetOpen			( bool Open ) ;		// true is Open

	// Generated message map functions
protected:
	//{{AFX_MSG(CRollupPane)
	afx_msg void OnPaint();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROLLUPPANE_H__2C1E6503_FBEF_11D1_9507_00207811EE70__INCLUDED_)
