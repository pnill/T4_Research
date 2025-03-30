// MemCardWiz.h : main header file for the MEMCARDWIZ application
//

#if !defined(AFX_MEMCARDWIZ_H__FF0758FF_0C1C_4E0C_A017_2C07D385CA98__INCLUDED_)
#define AFX_MEMCARDWIZ_H__FF0758FF_0C1C_4E0C_A017_2C07D385CA98__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMemCardWizApp:
// See MemCardWiz.cpp for the implementation of this class
//

class CMemCardWizApp : public CWinApp
{
public:
	CMemCardWizApp();
	~CMemCardWizApp();

	static int MFCAppDBGVector( int Type, char* pFile, int Line, char* pExpStr, char* pMsgStr );

	enum { MAX_SUPPRESSED_ASSERTS = 20 };
	static int   s_NumSuppressed;
	static char* s_SuppressedFiles[ MAX_SUPPRESSED_ASSERTS ];
	static int   s_SuppressedLines[ MAX_SUPPRESSED_ASSERTS ];

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMemCardWizApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMemCardWizApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MEMCARDWIZ_H__FF0758FF_0C1C_4E0C_A017_2C07D385CA98__INCLUDED_)
