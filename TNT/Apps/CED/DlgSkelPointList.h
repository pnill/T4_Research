#if !defined(AFX_DLGSKELPOINTLIST_H__9AC95B60_6542_11D2_9507_444553540001__INCLUDED_)
#define AFX_DLGSKELPOINTLIST_H__9AC95B60_6542_11D2_9507_444553540001__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgSkelPointList.h : header file
//

#include "MouseStatic.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgSkelPointList dialog

class CMotionEditView ;
class CCharacter ;

class CDlgSkelPointList : public CDialog
{
// Attributes
public:
	CMotionEditView	*m_pView ;								// Pointer to View

	CMouseStatic	m_StaticX ;
	CMouseStatic	m_StaticY ;
	CMouseStatic	m_StaticZ ;
	CMouseStatic	m_StaticR ;

	bool			m_CheckLocked ;
	bool			m_EditsLocked ;

// Operations
public:
	int  GetSelectedMotionsSkelPointCheck( int Index ) ;				// Get State of Skel Point Check
	void SetSelectedMotionsSkelPointCheck( int Index, BOOL State ) ;	// Set State of Skel Point Check
	void UpdateList				( CCharacter *pCharacter ) ;			// Update List of Skel Points
	void UpdateControls			( CCharacter *pCharacter ) ;			// Update Controls for Skel Point

// Construction
public:
	CDlgSkelPointList(CWnd* pParent = NULL);				// standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgSkelPointList)
	enum { IDD = IDD_POINTLIST };
	CButton	m_Delete;
	CButton	m_Add;
	CListCtrl	m_List;
	CEdit	m_Z;
	CEdit	m_Y;
	CEdit	m_X;
	CEdit	m_R;
	CComboBox	m_Bone;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgSkelPointList)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);		// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgSkelPointList)
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	afx_msg void OnSelchangeBone();
	afx_msg void OnChangeRadius();
	afx_msg void OnChangeX();
	afx_msg void OnChangeY();
	afx_msg void OnChangeZ();
	afx_msg void OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	afx_msg void OnOk();
	afx_msg void OnCancel();
	
	afx_msg LONG OnMouseStaticMove( UINT wParam, LONG lParam ) ;

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSKELPOINTLIST_H__9AC95B60_6542_11D2_9507_444553540001__INCLUDED_)
