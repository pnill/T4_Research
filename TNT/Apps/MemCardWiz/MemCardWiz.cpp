// MemCardWiz.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "MemCardWiz.h"
#include "MemCardWizDlg.h"
/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/
#include "x_files.hpp"


/////////////////////////////////////////////////////////////////////////////
// CMemCardWizApp

BEGIN_MESSAGE_MAP(CMemCardWizApp, CWinApp)
	//{{AFX_MSG_MAP(CMemCardWizApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMemCardWizApp construction

CMemCardWizApp::CMemCardWizApp()
{
    // init debug assert vector and x_files
//    x_DebugSetVector( (debug_fnptr)MFCAppDBGVector );
	x_Init();
}

CMemCardWizApp::~CMemCardWizApp()
{
	x_Kill();
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMemCardWizApp object

CMemCardWizApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CMemCardWizApp initialization

BOOL CMemCardWizApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CMemCardWizDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// APPLICATION DEBUG ASSERT VECTOR
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

int   CMemCardWizApp::s_NumSuppressed = 0;
char* CMemCardWizApp::s_SuppressedFiles[ CMemCardWizApp::MAX_SUPPRESSED_ASSERTS ];
int   CMemCardWizApp::s_SuppressedLines[ CMemCardWizApp::MAX_SUPPRESSED_ASSERTS ];

//--------------------------------------------------------------------------

int CMemCardWizApp::MFCAppDBGVector( int   Type, 
                                     char* pFile, 
                                     int   Line, 
                                     char* pExpStr, 
                                     char* pMsgStr )
{
    int  i;
    int  Proceed;
    char MsgText[1024];
    char Instructions[256];
    char TmpExpStr[256];
    char TmpMsgStr[256];

    BOOL InDebugger;

    InDebugger = IsDebuggerPresent();

    // Check if this assertion line has been suppressed
    for( i = 0; i < s_NumSuppressed; i++ )
    {
        if( (s_SuppressedLines[i] == Line) && (strcmp( s_SuppressedFiles[i], pFile ) == 0) )
        {
            return TRUE;
        }
    }

    // Create message to tell user what to do
    if( InDebugger )
        sprintf( TmpMsgStr, "Debug" );
    else
        sprintf( TmpMsgStr, "Terminate" );

    sprintf( Instructions, "\n\nPress Abort to %s application.\nPress Retry to Continue operation.\nPress Ignore to Supress this error and continue.", TmpMsgStr );

    // Create message text for assert context
    if( pExpStr )
        sprintf( TmpExpStr, "\nValue:\t%s", pExpStr );
    else
        TmpExpStr[0] = '\0';

    if( pMsgStr )
        sprintf( TmpMsgStr, "\nMessage:\n\"%s\"", pMsgStr );
    else
        TmpMsgStr[0] = '\0';

    if( pFile )
        sprintf( MsgText, "File:\t%s\nLine:\t%ld%s%s%s", pFile, Line, TmpExpStr, TmpMsgStr, Instructions );
    else
        sprintf( MsgText, "File:\t%s\nLine:\t%ld%s%s%s", "\0", Line, TmpExpStr, TmpMsgStr, Instructions );

    // Display a message box
    Proceed = ::MessageBox( NULL, MsgText, "Application Failure", MB_ABORTRETRYIGNORE );

    if( Proceed == IDABORT )
    {
        if( InDebugger )
            return FALSE;
        else
            AfxAbort();
    }

    if( Proceed == IDIGNORE )
    {
        if( s_NumSuppressed < MAX_SUPPRESSED_ASSERTS )
        {
            s_SuppressedFiles[s_NumSuppressed] = pFile;
            s_SuppressedLines[s_NumSuppressed] = Line;
            s_NumSuppressed++;
        }
    }

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////
