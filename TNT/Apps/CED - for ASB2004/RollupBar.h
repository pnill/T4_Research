#if !defined(AFX_ROLLUPBAR_H__53C3B020_FB3F_11D1_9507_00207811EE70__INCLUDED_)
#define AFX_ROLLUPBAR_H__53C3B020_FB3F_11D1_9507_00207811EE70__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// RollupBar.h : header file
//

#include "RollupCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CRollupBar window

class CRollupBar : public CControlBar
{
// Construction
public:
	CRollupBar( ) ;

// Attributes
public:
	int				m_Width ;
	CRollupCtrl		m_RollupCtrl ;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRollupBar)
	//}}AFX_VIRTUAL

	//---	Overriden CControlBar Functions
	void		OnUpdateCmdUI		( CFrameWnd* pTarget, BOOL bDisableIfNoHndler ) ;
	CSize		CalcFixedLayout		( BOOL bStretch, BOOL bHorz) ;
	CSize		CalcDynamicLayout	( int nLength, DWORD dwMode ) ;

// Implementation
public:
	virtual		~CRollupBar();

	//---	Set the Width of the Bar
	void		SetWidth			( int Width ) ;
	CRollupPane *AddPaneDialog		( CString Name, UINT nTemplateID, CDialog *pDialog ) ;

	//---	Create Function
	bool		Create				( CWnd* pParentWnd, DWORD dwStyle = WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE | CBRS_RIGHT, UINT nID = 0 ) ;

protected:

	// Generated message map functions
protected:
	//{{AFX_MSG(CRollupBar)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROLLUPBAR_H__53C3B020_FB3F_11D1_9507_00207811EE70__INCLUDED_)
