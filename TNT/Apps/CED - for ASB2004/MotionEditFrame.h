#if !defined(AFX_MOTIONEDITFRAME_H__B8064080_E009_11D1_AEC0_00A024569FF3__INCLUDED_)
#define AFX_MOTIONEDITFRAME_H__B8064080_E009_11D1_AEC0_00A024569FF3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MotionEditFrame.h : header file
//

#include "ChildFrm.h"
#include "Character.h"
#include "AnimBar.h"
//#include "MultiBar.h"
#include "RollupBar.h"
#include "DlgCharacter.h"
#include "DlgCamera1.h"
#include "DlgMotionList.h"
#include "DlgEventList.h"
#include "DlgSkelPointList.h"

class CMotionEditView ;

/////////////////////////////////////////////////////////////////////////////
// CMotionEditFrame frame

class CMotionEditFrame : public CChildFrame
{
	DECLARE_DYNCREATE(CMotionEditFrame)
public:
	CMotionEditFrame();

// Attributes
public:
//	CComboBox			*m_pCameraBoneCtrl ;
	CCharacter			*m_pCharacter ;
	CMotionEditView		*m_pView ;

// Operations
public:

// control bar embedded members
public:
	CToolBar			m_wndToolBar ;
//	CMultiBar			m_wndDialogBar ;
	CAnimBar			m_wndAnimBar ;

	CRollupBar			m_RollupBar ;						// Rollup Bar
	CDlgCharacter		m_DlgCharacter ;					// Character Dialog Pane
	CDlgCamera1			m_DlgCamera ;						// Camera Dialog Pane
	CDlgMotionList		m_DlgMotionList ;					// Motion List Dialog Pane
	CDlgEventList		m_DlgEventList ;					// Event List Dialog Pane
	CDlgSkelPointList	m_DlgSkelPointList ;				// Point List Dialog Pane

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMotionEditFrame)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMotionEditFrame();

protected:
	// Generated message map functions
	//{{AFX_MSG(CMotionEditFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOTIONEDITFRAME_H__B8064080_E009_11D1_AEC0_00A024569FF3__INCLUDED_)
