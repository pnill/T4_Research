#if !defined(AFX_ANIMBAR_H__D6EFDBE3_E8C5_11D1_AEC0_00A024569FF3__INCLUDED_)
#define AFX_ANIMBAR_H__D6EFDBE3_E8C5_11D1_AEC0_00A024569FF3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// AnimBar.h : header file
//

#include "Motion.h"

class CMotionEditView ;

#define	MAX_RANGES	100

/////////////////////////////////////////////////////////////////////////////
// CAnimBar window

class CAnimBar : public CDialogBar
{
// Construction
public:
	CAnimBar();

// Attributes
public:
	CBitmapButton	m_bbFrameStart ;
	CBitmapButton	m_bbFrameEnd ;

	CMotionEditView	*m_pView ;
	int				m_nFrame ;
	int				m_NumFrames ;
	int				m_nStartFrame ;
	int				m_nEndFrame ;

	//---	transition view additions
	bool			m_bRangesActive ;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnimBar)
	//}}AFX_VIRTUAL

	virtual BOOL Create( CWnd* pParentWnd, UINT nIDTemplate, UINT nStyle, UINT nID ) ;

// Implementation
public:
	virtual ~CAnimBar();

	void	SetView( CMotionEditView *pView ) ;
	void	Update( ) ;
	void	SetMotion( CMotion *pMotion ) ;
	void	SetFrame( int nFrame ) ;
	void	SetSelStart( ) ;
	void	SetSelEnd( ) ;
	void	GetSelection( int *pStart, int *pEnd ) ;

	void	UpdateOnScrollChange( s32 NewFrame );

	//---	transition view additions
	void	SetRange( s32 Range ) ;

	// Generated message map functions
protected:
	//{{AFX_MSG(CAnimBar)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG

	afx_msg void OnChangeFrame( ) ;

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIMBAR_H__D6EFDBE3_E8C5_11D1_AEC0_00A024569FF3__INCLUDED_)
