#if !defined(AFX_MOVIEBAR_H__D6EFDBE3_E8C5_11D1_AEC0_00A024569FF3__INCLUDED_)
#define AFX_MOVIEBAR_H__D6EFDBE3_E8C5_11D1_AEC0_00A024569FF3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MovieBar.h : header file
//

#include "Motion.h"

class CMovieView ;

/////////////////////////////////////////////////////////////////////////////
// CMovieBar window

class CMovieBar : public CDialogBar
{
// Construction
public:
	CMovieBar();

// Attributes
public:
	CMovieView		*m_pView ;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMovieBar)
	//}}AFX_VIRTUAL

	virtual BOOL Create( CWnd* pParentWnd, UINT nIDTemplate, UINT nStyle, UINT nID ) ;

// Implementation
public:
	virtual ~CMovieBar();

	void	SetView( CMovieView *pView ) ;
	void	Update( ) ;
	void	SetNumFrames( int NumFrames ) ;
	void	SetFrame( int nFrame ) ;

	// Generated message map functions
protected:
	//{{AFX_MSG(CMovieBar)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG

	afx_msg void OnChangeFrame( ) ;
	afx_msg void OnChangeNumFrames( ) ;

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOVIEBAR_H__D6EFDBE3_E8C5_11D1_AEC0_00A024569FF3__INCLUDED_)
