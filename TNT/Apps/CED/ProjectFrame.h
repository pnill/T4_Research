#if !defined(AFX_PROJECTFRAME_H__B8064080_E009_11D1_AEC0_00A024569FF3__INCLUDED_)
#define AFX_PROJECTFRAME_H__B8064080_E009_11D1_AEC0_00A024569FF3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ProjectFrame.h : header file
//

#include "ChildFrm.h"

/////////////////////////////////////////////////////////////////////////////
// CProjectFrame frame

class CProjectFrame : public CChildFrame
{
	DECLARE_DYNCREATE(CProjectFrame)
public:
	CProjectFrame();

// Attributes
public:
	CComboBox		*m_pCameraBoneCtrl ;

// Operations
public:

// control bar embedded members
public:
	//CToolBar		m_wndToolBar ;
	CDialogBar		m_wndDialogBar ;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProjectFrame)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CProjectFrame();

protected:
	// Generated message map functions
	//{{AFX_MSG(CProjectFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROJECTFRAME_H__B8064080_E009_11D1_AEC0_00A024569FF3__INCLUDED_)
