#if !defined(AFX_DLGCAMERA1_H__570E9703_FFC1_11D1_9507_00207811EE70__INCLUDED_)
#define AFX_DLGCAMERA1_H__570E9703_FFC1_11D1_9507_00207811EE70__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DlgCamera1.h : header file
//

#include "x_color.hpp"
#include "x_debug.hpp"
#include "x_plus.hpp"
#include "x_stdio.hpp"
#include "x_memory.hpp"

#include "MFCutils.h"

class CMotionEditView ;

/////////////////////////////////////////////////////////////////////////////
// CDlgCamera1 dialog

class CDlgCamera1 : public CDialog
{
//---	Attributes
public:
	CMotionEditView	*m_pView ;								// Pointer to View
	bool			m_EditsLocked ;							// Ignore Edit Change messages

//---	Operations
public:
	void	SetTarget		( vector3  *pVector ) ;			// Set Target Vector
	void	SetRotation		( f32 Rotation ) ;				// Set Rotation
	void	SetElevation	( f32 Elevation ) ;				// Set Elevation
	void	Reset			( ) ;							// Reset List of Bones
	void	EnableBoneList	( bool Enable ) ;				// Enable Bone List
	void	AddBone			( CString Name ) ;				// Add Bone to List
	void	SelectBone		( int iBone ) ;					// Select Bone from List

// Construction
public:
	CDlgCamera1(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCamera1)
	enum { IDD = IDD_CAMERA1 };
	CButton	m_LockToFaceDir;
	CButton	m_TimeAccurate;
	CEdit	m_Z;
	CEdit	m_Y;
	CEdit	m_X;
	CEdit	m_Rotation;
	CEdit	m_Elevation;
	CComboBox	m_Bone;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCamera1)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCamera1)
	afx_msg void OnChangeCameraTargetx();
	afx_msg void OnChangeCameraTargety();
	afx_msg void OnChangeCameraTargetz();
	afx_msg void OnChangeCameraRotation();
	afx_msg void OnChangeCameraElevation();
	afx_msg void OnSelchangeCameraBone();
	afx_msg void OnTimeaccurate();
	afx_msg void OnLocktofacedir();
	//}}AFX_MSG

	afx_msg void OnOk();
	afx_msg void OnCancel();
	
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCAMERA1_H__570E9703_FFC1_11D1_9507_00207811EE70__INCLUDED_)
