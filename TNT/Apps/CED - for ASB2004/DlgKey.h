#if !defined(AFX_DLGKEY_H__147873A0_0487_11D2_9507_00207811EE70__INCLUDED_)
#define AFX_DLGKEY_H__147873A0_0487_11D2_9507_00207811EE70__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DlgKey.h : header file
//

#include "MouseStatic.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgKey dialog

class CMovieView ;
class CKey ;
class CActor ;
class CCharacter ;
class CMotion ;

/////////////////////////////////////////////////////////////////////////////
// CDlgKeyPosition dialog

class CDlgKey : public CDialog
{
//---	Attributes
public:
	bool			m_Updating ;							// Updating Edits, do not react to change messages

	CMovieView		*m_pView ;								// Pointer to View
	CActor			*m_pActor ;								// Pointer to Actor

	CMouseStatic	m_StaticX ;								// Static Mouse Controls
	CMouseStatic	m_StaticY ;
	CMouseStatic	m_StaticZ ;
	CMouseStatic	m_StaticVX ;
	CMouseStatic	m_StaticVY ;
	CMouseStatic	m_StaticVZ ;
	CMouseStatic	m_StaticRX ;
	CMouseStatic	m_StaticRY ;
	CMouseStatic	m_StaticRZ ;

//---	Operations
public:
	void	SetActor			( CActor *pActor ) ;				// Set Actor to reference
	void	SetupMotions		( CCharacter *pCharacter ) ;		// Setup Motions
	void	SelectMotion		( CMotion *pMotion ) ;				// Select a Motion
	void	SelectKey			( CKey *pKey ) ;					// Select Key
	void	UpdateDetails		( CKey *pKey ) ;					// Update Details

// Construction
public:
	CDlgKey(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgKey)
	enum { IDD = IDD_KEY };
	CButton	m_AutoLinkDir;
	CButton	m_Mirror;
	CEdit	m_FrameRate;
	CEdit	m_Blend;
	CComboBox	m_Event;
	CButton	m_CameraLock;
	CButton	m_Visible;
	CEdit	m_Scale;
	CEdit	m_RZ;
	CEdit	m_RY;
	CEdit	m_RX;
	CButton	m_AutoLink;
	CComboBox	m_Motion;
	CButton	m_Radio1;
	CButton	m_Radio2;
	CEdit	m_StartFrame;
	CEdit	m_NumFrames;
	CEdit	m_Frame;
	CEdit	m_VZ;
	CEdit	m_VY;
	CEdit	m_VX;
	CEdit	m_Z;
	CEdit	m_Y;
	CEdit	m_X;
	CListCtrl	m_List;
	int		m_KeyMode;
	BOOL	m_BoolAutoLink;
	BOOL	m_BoolVisible;
	BOOL	m_BoolCameraLock;
	BOOL	m_BoolAutoLinkDir;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgKey)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgKey)
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnDel();
	afx_msg void OnSelectAll();
	afx_msg void OnSelectNone();
	afx_msg void OnAutolink();
	afx_msg void OnChangeFrame();
	afx_msg void OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeMotion();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnChangeStartframe();
	afx_msg void OnChangeX();
	afx_msg void OnChangeY();
	afx_msg void OnChangeZ();
	afx_msg void OnChangeVx();
	afx_msg void OnChangeVy();
	afx_msg void OnChangeVz();
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeRx();
	afx_msg void OnChangeRy();
	afx_msg void OnChangeRz();
	afx_msg void OnChangeScale();
	afx_msg void OnVisible();
	afx_msg void OnCameralock();
	afx_msg void OnSelchangeEvent();
	afx_msg void OnChangeRate();
	afx_msg void OnChangeBlend();
	afx_msg void OnMirror();
	afx_msg void OnAutolinkdir();
	//}}AFX_MSG
	
	afx_msg void OnOk();
	afx_msg void OnCancel();
	
	afx_msg LONG OnMouseStaticMove( UINT wParam, LONG lParam ) ;

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGKEY_H__147873A0_0487_11D2_9507_00207811EE70__INCLUDED_)
