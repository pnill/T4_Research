#if !defined(AFX_MOTIONFLAGS_H__6018E980_4BA8_11D2_9507_444553540001__INCLUDED_)
#define AFX_MOTIONFLAGS_H__6018E980_4BA8_11D2_9507_444553540001__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MotionFlags.h : header file
//

#include "Motion.h"

/////////////////////////////////////////////////////////////////////////////
// CMotionFlags dialog

class CMotionFlags : public CDialog
{
// Construction
public:
	CMotionFlags(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMotionFlags)
	enum { IDD = IDD_MOTION_FLAGS };
	CButton	m_CBit9;
	CButton	m_CBit8;
	CButton	m_CBit7;
	CButton	m_CBit6;
	CButton	m_CBit5;
	CButton	m_CBit4;
	CButton	m_CBit3;
	CButton	m_CBit2;
	CButton	m_CBit15;
	CButton	m_CBit14;
	CButton	m_CBit13;
	CButton	m_CBit12;
	CButton	m_CBit11;
	CButton	m_CBit10;
	CButton	m_CBit1;
	CButton	m_CBit0;
	CString	m_Def0;
	CString	m_Def1;
	CString	m_Def10;
	CString	m_Def11;
	CString	m_Def12;
	CString	m_Def13;
	CString	m_Def14;
	CString	m_Def15;
	CString	m_Def2;
	CString	m_Def3;
	CString	m_Def4;
	CString	m_Def5;
	CString	m_Def6;
	CString	m_Def7;
	CString	m_Def8;
	CString	m_Def9;
	CString	m_Txt0;
	CString	m_Txt1;
	CString	m_Txt10;
	CString	m_Txt11;
	CString	m_Txt12;
	CString	m_Txt13;
	CString	m_Txt14;
	CString	m_Txt15;
	CString	m_Txt2;
	CString	m_Txt3;
	CString	m_Txt4;
	CString	m_Txt5;
	CString	m_Txt6;
	CString	m_Txt7;
	CString	m_Txt8;
	CString	m_Txt9;
	//}}AFX_DATA

	int	m_Bit[MOTION_FLAG_BITS] ;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMotionFlags)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMotionFlags)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBit0();
	afx_msg void OnBit1();
	afx_msg void OnBit2();
	afx_msg void OnBit3();
	afx_msg void OnBit4();
	afx_msg void OnBit5();
	afx_msg void OnBit6();
	afx_msg void OnBit7();
	afx_msg void OnBit8();
	afx_msg void OnBit9();
	afx_msg void OnBit10();
	afx_msg void OnBit11();
	afx_msg void OnBit12();
	afx_msg void OnBit13();
	afx_msg void OnBit14();
	afx_msg void OnBit15();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOTIONFLAGS_H__6018E980_4BA8_11D2_9507_444553540001__INCLUDED_)
