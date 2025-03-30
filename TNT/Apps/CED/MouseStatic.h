#if !defined(AFX_MOUSESTATIC_H__8F346AE0_F352_11D1_9507_00207811EE70__INCLUDED_)
#define AFX_MOUSESTATIC_H__8F346AE0_F352_11D1_9507_00207811EE70__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MouseStatic.h : header file
//

enum
{
	MSM_MOUSEMOVE		= WM_USER+1000,
} ;

/////////////////////////////////////////////////////////////////////////////
// CMouseStatic window

class CMouseStatic : public CStatic
{
// Construction
public:
	CMouseStatic();

// Attributes
public:
	CPoint		m_MouseClick ;
	CWnd		*m_pNotifyWindow ;

// Operations
public:

	void		SetNotifyWindow( CWnd *pWnd ) ;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMouseStatic)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMouseStatic();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMouseStatic)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOUSESTATIC_H__8F346AE0_F352_11D1_9507_00207811EE70__INCLUDED_)
