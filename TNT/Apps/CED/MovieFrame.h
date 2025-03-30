#if !defined(AFX_MOVIEFRAME_H__3328CA40_FE13_11D1_9507_00207811EE70__INCLUDED_)
#define AFX_MOVIEFRAME_H__3328CA40_FE13_11D1_9507_00207811EE70__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MovieFrame.h : header file
//

#include "ChildFrm.h"
#include "MovieBar.h"
#include "MultiBar.h"

class CMovie ;

/////////////////////////////////////////////////////////////////////////////
// CMovieFrame frame

class CMovieFrame : public CChildFrame
{
	DECLARE_DYNCREATE(CMovieFrame)

public:
	CMovieFrame();							// protected constructor used by dynamic creation

// Attributes
public:
	CMovie		*m_pMovie ;					// Pointer to Movie

// Operations
public:

// control bar embedded members
public:
	CMovieBar		m_MovieBar ;			// Movie Control bar

	CToolBar		m_ToolBar ;
	CMultiBar		m_RollupBar ;			// Rollup Toolbar

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMovieFrame)
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CMovieFrame();

	// Generated message map functions
	//{{AFX_MSG(CMovieFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOVIEFRAME_H__3328CA40_FE13_11D1_9507_00207811EE70__INCLUDED_)
