#if !defined(AFX_DLGEVENTLIST_H__29EF31E3_0132_11D2_AEC0_00A024569FF3__INCLUDED_)
#define AFX_DLGEVENTLIST_H__29EF31E3_0132_11D2_AEC0_00A024569FF3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DlgEventList.h : header file
//

#include "MouseStatic.h"
#include "EventList.h"

class CMotionEditView ;
class CMotion ;

/////////////////////////////////////////////////////////////////////////////
// CDlgEventList dialog

class CDlgEventList : public CDialog
{
// Attributes
public:
	CMotionEditView	*m_pView ;								// Pointer to View

	CMouseStatic	m_StaticEventX ;
	CMouseStatic	m_StaticEventY ;
	CMouseStatic	m_StaticEventZ ;
	CMouseStatic	m_StaticEventR ;

	s32			m_EditsLocked ;

// Operations
public:
	void UpdateEventList		( CMotion *pMotion ) ;		// Update Event List
	void UpdateEventControls	( CMotion *pMotion ) ;		// Update Event Controls

// Construction
public:
	void UpdateEventDefinedEvents();
	CDlgEventList(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgEventList)
	enum { IDD = IDD_EVENTLIST };
	CButton	m_ButtonAttachBoneNow;
	CEdit	m_DistLocal;
	CEdit	m_Dist;
	CEdit	m_DirLocal;
	CEdit	m_Dir;
	CEdit	m_Zlocal;
	CEdit	m_Ylocal;
	CEdit	m_Xlocal;
	CStatic	m_Bone;
	CButton	m_ButtonAttachBone;
	CButton	m_ButtonInsertEvent;
	CComboBox	m_ComboBoxProjectEvents;
	CEdit	m_R;
	CEdit	m_Frame;
	CEdit	m_Z;
	CEdit	m_Y;
	CEdit	m_X;
	CListBox	m_List;
	//}}AFX_DATA
	//}}AFX_DATA
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgEventList)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL OnToolTipNotify( UINT id, NMHDR * pTTTStruct, LRESULT * pResult );
	void AddCurrentEventAttachedList( CEvent* pAddEvent );
	void AddCurrentEventProjectList( CEvent* pAddEvent );
	void AddDefinedEvent( CEvent* pAddEvent );
	CEvent* GetAttachedEventbyName(CString EventName);
	CEvent* GetProjectEventbyName( CString EventName );
	void UpdateEventAttachedEvents();
	void UpdateEventProjectEvents();
	void UpdateEventPositionDisplay( u32 ControlChanged, CMotion* pMotion, CEvent* pEvent );
	void UpdateEventPosition( CEvent* pEvent );

	// Generated message map functions
	//{{AFX_MSG(CDlgEventList)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeEventList();
	afx_msg void OnEventAdd();
	afx_msg void OnEventDelete();
	afx_msg void OnChangeEventX();
	afx_msg void OnChangeEventY();
	afx_msg void OnChangeEventZ();
	afx_msg void OnChangeEventXlocal();
	afx_msg void OnChangeEventYlocal();
	afx_msg void OnChangeEventZlocal();
	afx_msg void OnChangeEventDir();
	afx_msg void OnChangeEventDirlocal();
	afx_msg void OnChangeEventDist();
	afx_msg void OnChangeEventDistlocal();
	afx_msg void OnChangeEventRadius();
	afx_msg void OnChangeEventFrame();
	afx_msg void OnChangeEventName();
	afx_msg void OnButtonNewProjectEvent();
	afx_msg void OnButtonAddCurrentEvent();
	afx_msg void OnButtonInsertEvent();
	afx_msg void OnButtonAttachBone();
	afx_msg void OnButtonAttachBoneNow();
	//}}AFX_MSG

	afx_msg void OnOk();
	afx_msg void OnCancel();
	
	afx_msg LONG OnMouseStaticMove( UINT wParam, LONG lParam ) ;
	
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGEVENTLIST_H__29EF31E3_0132_11D2_AEC0_00A024569FF3__INCLUDED_)
