// CeD.h : main header file for the CED application
//

#if !defined(AFX_CED_H__68F53CE9_D3EC_11D1_AEC0_00A024569FF3__INCLUDED_)
#define AFX_CED_H__68F53CE9_D3EC_11D1_AEC0_00A024569FF3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


/////////////////////////////////////////////////////////////////////////////
// CCeDApp:
// See CeD.cpp for the implementation of this class
//

class CCeDApp : public CWinApp
{
public:
	CCeDApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCeDApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL OnIdle(LONG lCount);
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CCeDApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CED_H__68F53CE9_D3EC_11D1_AEC0_00A024569FF3__INCLUDED_)
