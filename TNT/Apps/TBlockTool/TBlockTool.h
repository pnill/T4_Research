////////////////////////////////////////////////////////////////////////////
//
// TBlockTool.h : main header file for the TBLOCKTOOL application
//
////////////////////////////////////////////////////////////////////////////

#ifndef TBLOCKTOOL_H_INCLUDED
#define TBLOCKTOOL_H_INCLUDED


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#ifndef TBLOCK_STDAFX_H_INCLUDED
    #error "You must include StdAfx.h before this file."
#endif


#include "x_debug.hpp"


////////////////////////////////////////////////////////////////////////////
// CTBlockToolApp:
////////////////////////////////////////////////////////////////////////////

class CTBlockToolApp : public CWinApp
{
public:
    CTBlockToolApp();
    ~CTBlockToolApp();

    static xbool MFCAppDBGVector( s32 Type, char* pFile, s32 Line, char* pExpStr, char* pMsgStr );

    enum { MAX_SUPPRESSED_ASSERTS = 16 };

    static s32   s_NumSuppressed;
    static char* s_SuppressedFiles[MAX_SUPPRESSED_ASSERTS];
    static s32   s_SuppressedLines[MAX_SUPPRESSED_ASSERTS];

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CTBlockToolApp)
    public:
    virtual BOOL InitInstance();
    //}}AFX_VIRTUAL

    //{{AFX_MSG(CTBlockToolApp)
        // NOTE - the ClassWizard will add and remove member functions here.
        //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // TBLOCKTOOL_H_INCLUDED
