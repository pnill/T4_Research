#if !defined(AFX_ROLLUPCTRL_H__9935FD42_FC5F_11D1_AEC0_00A024569FF3__INCLUDED_)
#define AFX_ROLLUPCTRL_H__9935FD42_FC5F_11D1_AEC0_00A024569FF3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// RollupCtrl.h : header file
//

class CRollupPane ;

/////////////////////////////////////////////////////////////////////////////
// CRollupCtrl window

class CRollupCtrl : public CWnd
{
// Construction
public:
	CRollupCtrl();

// Attributes
public:
	int			m_TotalChildHeight ;		// Combined Height of Child Panels
	int			m_ScrollPos ;				// Scroll Position

	CPtrArray	m_Panes ;					// Array of Panes
	CPoint		m_MouseClick ;				// Mouse Click Position
	CPoint		m_MouseLast ;				// Mouse Last Position
	CPoint		m_MouseDelta ;				// Delta Mouse Move

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRollupCtrl)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CRollupCtrl();

	//---	Pane Management Functions
	CRollupPane	*AddPaneDialog			( CString Name, UINT nTemplateID, CDialog *pDialog ) ;
	void		DeleteAllPanes			( ) ;
	void		PaneChanged				( ) ;
	void		RepositionPanes			( ) ;
	int			GetTotalPaneHeight		( ) ;
	int			GetVisiblePaneHeight	( ) ;
	int			SetScrollPos			( int ScrollPos ) ;
	int			GetScrollPos			( ) ;
	void		LimitScrollPos			( ) ;

	//---	Drawing Assistance Functions
	void		DrawSlider				( CDC *pDC ) ;

	// Generated message map functions
protected:
	//{{AFX_MSG(CRollupCtrl)
	afx_msg void OnNcPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROLLUPCTRL_H__9935FD42_FC5F_11D1_AEC0_00A024569FF3__INCLUDED_)
