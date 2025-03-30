////////////////////////////////////////////////////////////////////////////
//
// TBlockTool.cpp : Defines the class behaviors for the application.
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

// MFC includes
#include "stdafx.h"

#include "TBlockTool.h"
#include "TBlockToolDlg.h"
#include "TBlockCmdLineDlg.h"

// Application includes
#include "x_files.hpp"

#include "StatusOutput.h"
#include "TBlockCmdLineApp.h"
#include "TBlockIMEXObj.h"


#pragma warning( disable : 4996 ) // disable warning C4996: 'SOME_FUNCTION' was declared deprecated


////////////////////////////////////////////////////////////////////////////
// GLOBALS
////////////////////////////////////////////////////////////////////////////

CTBlockToolApp theApp;


////////////////////////////////////////////////////////////////////////////
// CTBlockToolApp
////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CTBlockToolApp, CWinApp)
    //{{AFX_MSG_MAP(CTBlockToolApp)
        // NOTE - the ClassWizard will add and remove mapping macros here.
        //    DO NOT EDIT what you see in these blocks of generated code!
    //}}AFX_MSG
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

//==========================================================================

CTBlockToolApp::CTBlockToolApp( void )
{
    // init debug assert vector and x_files
    x_DebugSetVector( MFCAppDBGVector );
    x_Init();
}

//==========================================================================

CTBlockToolApp::~CTBlockToolApp( void )
{
    // kill x_files
    x_Kill();
}

//==========================================================================

BOOL CTBlockToolApp::InitInstance( void )
{
    AfxEnableControlContainer();

    // Standard initialization
    // If you are not using these features and wish to reduce the size
    //  of your final executable, you should remove from the following
    //  the specific initialization routines you do not need.

#ifdef _AFXDLL
    Enable3dControls();         // Call this when using MFC in a shared DLL
#else
    Enable3dControlsStatic();   // Call this when linking to MFC statically
#endif

    //---------------------------------------------------------------------

    TBlockCmdLineApp AppCmdLine;
    TBlockIMEXObj    IMEXObj;

    // parse any command line parameters
    this->ParseCommandLine( AppCmdLine );

    if( AppCmdLine.NumCmds() > 0 )
    {
        // Show output window if processing cmdline
        TBlockCmdLineDlg CmdLineDialog( AppCmdLine, IMEXObj );
        m_pMainWnd = &CmdLineDialog;

        if( !CmdLineDialog.Execute() )
            return FALSE;

        STATUS::InitHandlers( 0, NULL, NULL, NULL );
        m_pMainWnd = NULL;
    }

    TBlockToolDlg ToolDialog( &IMEXObj );
    m_pMainWnd = &ToolDialog;

    // Execute normal application mode
    ToolDialog.DoModal();


    // Since the dialog has been closed, return FALSE so that we exit the
    //  application, rather than start the application's message pump.
    return FALSE;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// APPLICATION DEBUG ASSERT VECTOR
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

s32   CTBlockToolApp::s_NumSuppressed = 0;
char* CTBlockToolApp::s_SuppressedFiles[ CTBlockToolApp::MAX_SUPPRESSED_ASSERTS ];
s32   CTBlockToolApp::s_SuppressedLines[ CTBlockToolApp::MAX_SUPPRESSED_ASSERTS ];

//--------------------------------------------------------------------------

xbool CTBlockToolApp::MFCAppDBGVector( s32   Type, 
                                       char* pFile, 
                                       s32   Line, 
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
        if( (s_SuppressedLines[i] == Line) && (x_strcmp( s_SuppressedFiles[i], pFile ) == 0) )
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
