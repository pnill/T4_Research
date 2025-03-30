// AnimTblEd.h : main header file for the ANIMTBLED application
//

#if !defined(AFX_ANIMTBLED_H__CE6EAFE5_57C5_11D2_ABF7_00A024569875__INCLUDED_)
#define AFX_ANIMTBLED_H__CE6EAFE5_57C5_11D2_ABF7_00A024569875__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


/////////////////////////////////////////////////////////////////////////////
// CAnimTblEdApp:
// See AnimTblEd.cpp for the implementation of this class
//

class CAnimTblEdApp : public CWinApp
{
public:
	CAnimTblEdApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnimTblEdApp)
	public:
	virtual BOOL InitInstance();
	virtual BOOL OnIdle(LONG lCount);
	//}}AFX_VIRTUAL

// Implementation


	//{{AFX_MSG(CAnimTblEdApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIMTBLED_H__CE6EAFE5_57C5_11D2_ABF7_00A024569875__INCLUDED_)
