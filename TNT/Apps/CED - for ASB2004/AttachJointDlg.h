#if !defined(AFX_ATTACHJOINTDLG_H__6EDB96C1_DAEF_11D3_88A6_00105A29F84C__INCLUDED_)
#define AFX_ATTACHJOINTDLG_H__6EDB96C1_DAEF_11D3_88A6_00105A29F84C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AttachJointDlg.h : header file
//

#include "MotionEditView.h"

/////////////////////////////////////////////////////////////////////////////
// CAttachJointDlg dialog

class CAttachJointDlg : public CDialog
{
// Construction
public:
	void SetView( CMotionEditView* pView );
	void SetEvent( CEvent* pEvent );
	void SetImmediate( void ) {m_Immediate=TRUE;}
	CAttachJointDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAttachJointDlg)
	enum { IDD = IDD_ATTACH_EVENT_TO_JOINT };
	CButton	m_LinkedCharacter;
	CString	m_StringSelectedEvent;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAttachJointDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL OnToolTipNotify( UINT id, NMHDR * pTTTStruct, LRESULT * pResult );
	void AddEventtoAttachedEventList( INT iBone );
	void InitSelectedEvent();
	void InitBoneCombo();
	CEvent* m_pSelectedEvent;
	CMotionEditView* m_pView;
	bool m_Immediate;

	static s32 s_LastBone;
	static s32 s_LastLinkedChar;

	// Generated message map functions
	//{{AFX_MSG(CAttachJointDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonAttachToJoint();
	afx_msg void OnButtonAttachToLinkedChar();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ATTACHJOINTDLG_H__6EDB96C1_DAEF_11D3_88A6_00105A29F84C__INCLUDED_)
