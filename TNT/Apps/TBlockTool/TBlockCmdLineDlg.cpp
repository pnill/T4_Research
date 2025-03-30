////////////////////////////////////////////////////////////////////////////
//
// TBlockToolDlg.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "x_debug.hpp"

#include "TBlockCmdLineDlg.h"


#pragma warning( disable : 4311 ) // disable warning(4311): 'type cast' : pointer truncation from 'SOME_TYPE *' to 'SOME_OTHER_TYPE'
#pragma warning( disable : 4312 ) // disable warning(4312): 'type cast' : conversion from 'SOME_TYPE' to 'SOME_OTHER_TYPE *' of greater size


////////////////////////////////////////////////////////////////////////////
// TBlockCmdLineDlg dialog IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

TBlockCmdLineDlg::TBlockCmdLineDlg( TBlockCmdLineApp& rAppCmdLine, TBlockIMEXObj& rIMEXObj, CWnd* pParent )
    :CDialog()
{
    //{{AFX_DATA_INIT(TBlockCmdLineDlg)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT

    AfxInitRichEdit();

    m_bExitOnFinish      = FALSE;
    m_bWaitAfterFinish   = FALSE;
    m_bSkipErrorOnFinish = FALSE;

    m_pAppCmdLine = &rAppCmdLine;
    m_pIMEXObj    = &rIMEXObj;

    m_SavedTarget = TBLOCK_TGT_NOTSET;
    m_bImporting  = FALSE;
    m_bExporting  = FALSE;

    m_pLogFile = NULL;

    m_NumMsgNormal    = 0;
    m_NumMsgImportant = 0;
    m_NumMsgWarning   = 0;
    m_NumMsgError     = 0;

    m_pAppCmdLine->Start();

    // setup output message handling
    STATUS::InitHandlers( (u32)this, StatusPrintCB, StatusProgStartCB, StatusProgStepCB );

    this->Create( TBlockCmdLineDlg::IDD, pParent );
    this->ShowWindow( TRUE );
}

//==========================================================================

TBlockCmdLineDlg::~TBlockCmdLineDlg( void )
{
    this->CloseLogFile();

    this->DestroyWindow();
}

//==========================================================================

BEGIN_MESSAGE_MAP(TBlockCmdLineDlg, CDialog)
    //{{AFX_MSG_MAP(TBlockCmdLineDlg)
    ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnButtonClose)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////
// TBlockCmdLineDlg MESSAGE HANDLERS
////////////////////////////////////////////////////////////////////////////

void TBlockCmdLineDlg::OnOK( void )
{
    // overloaded to keep default input from closing window
}

void TBlockCmdLineDlg::OnCancel( void )
{
	OnButtonClose();
}

void TBlockCmdLineDlg::OnButtonClose( void ) 
{
    if( this->m_pIMEXObj->IsFinished() )
    {
        this->EndModalLoop( 1 );
    }
    else
    {
        this->m_bWaitAfterFinish = TRUE;
        this->m_pIMEXObj->StopProcess();
    }
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

BOOL TBlockCmdLineDlg::Execute( void )
{
    this->CommandLineExecute();
    this->RunModalLoop();

    this->CloseLogFile();

    if( m_bExitOnFinish )
        return FALSE;

    return TRUE;
}

//==========================================================================

void TBlockCmdLineDlg::CloseLogFile( void )
{
    if( m_pLogFile == NULL )
        return;

    fprintf( m_pLogFile, "\n\n\n" );

    fclose( m_pLogFile );

    m_pLogFile = NULL;
}

//==========================================================================

void TBlockCmdLineDlg::CommandLineExecute( void )
{
    CMDLINE_FLAG CLSwitch;
    char*        pParam;

    // get commands until none are returned
    while( CMD_NOTHING != (CLSwitch = m_pAppCmdLine->NextCommand( &pParam )) )
    {
        switch( CLSwitch )
        {
            case CMD_EXIT_ONDONE:
                m_bExitOnFinish = TRUE;
                break;

            case CMD_WAIT_ONCLOSE:
                m_bWaitAfterFinish = TRUE;
                break;

            case CMD_NO_WAIT_ONERROR:
                m_bSkipErrorOnFinish = TRUE;
                break;

            case CMD_LOGFILE:
                this->CloseLogFile();

                m_pLogFile = fopen( pParam, "a+" );

                if( m_pLogFile == NULL )
                {
                    STATUS::Print( STATUS::MSG_WARNING, "WARNING: Couldn't open/create log file.\n" );
                }
                break;

            case CMD_MINIMIZE:
                this->ShowWindow( SW_MINIMIZE );
                break;

            case CMD_IMPORT_FILE:
                m_bImporting = TRUE;
                m_bExporting = FALSE;
                if( TB_RCODE_SUCCESS(m_pIMEXObj->Import( pParam, NULL, IMEX_Done, (u32)this )) )
                    return;

                break;

            case CMD_EXPORT_FILE:
                if( m_SavedTarget == TBLOCK_TGT_NOTSET )
                {
                    STATUS::Print( STATUS::MSG_ERROR, "ERROR: No target specified for %s.\n", pParam );
                }
                else
                {
                    m_bImporting = FALSE;
                    m_bExporting = TRUE;
                    if( TB_RCODE_SUCCESS(m_pIMEXObj->ExportCurData( pParam, m_SavedTarget, IMEX_Done, (u32)this )) )
                        return;
                }
                break;

            case CMD_IMPORT_EXPORT_FILE:
                m_bImporting = TRUE;
                m_bExporting = TRUE;
                if( TB_RCODE_SUCCESS(m_pIMEXObj->Import( pParam, NULL, IMEX_Done, (u32)this )) )
                    return;

                break;

            case CMD_TARGET_PS2:        m_SavedTarget = TBLOCK_TGT_PS2;         break;
            case CMD_TARGET_GAMECUBE:   m_SavedTarget = TBLOCK_TGT_GAMECUBE;    break;
            case CMD_TARGET_XBOX:       m_SavedTarget = TBLOCK_TGT_XBOX;        break;
            case CMD_TARGET_PC:         m_SavedTarget = TBLOCK_TGT_PCDX8;       break;

            case CMD_DEBUG_TINTMIPS:
                m_pIMEXObj->EnableTintMips( pParam[0] == '1' );
                break;
        }
    }

    m_bImporting = FALSE;
    m_bExporting = FALSE;

    // Done executing command line, if any messages were shown that deserve attention,
    // leave window open otherwise close automatically
    if( !m_bWaitAfterFinish && ((m_NumMsgError == 0) || m_bSkipErrorOnFinish) )
    {
        //OnButtonClose();
        HWND hCtrl;
        this->GetDlgItem( IDC_BUTTON_CLOSE, &hCtrl );
        this->PostMessage( WM_COMMAND, MAKEWPARAM(IDC_BUTTON_CLOSE, BN_CLICKED), (LPARAM)hCtrl );
    }
    else
    {
        CButton* pBtn = (CButton*)GetDlgItem( IDC_BUTTON_CLOSE );
        pBtn->SetWindowText( "Close" );
        pBtn->SetFocus();
    }
}

//==========================================================================

xbool TBlockCmdLineDlg::IMEX_Done( u32 cbParam, TBlockIMEXObj* pIMEXObj, s32 RetCode )
{
    TBlockCmdLineDlg* pTHIS = (TBlockCmdLineDlg*)cbParam;

    if( pTHIS->m_bImporting )
    {
        pTHIS->m_bImporting = FALSE;

        // Check if a re-export command was issued
        if( pTHIS->m_bExporting )
        {
            if( RetCode == TB_RCODE_OK )
            {
                if( pTHIS->m_SavedTarget == TBLOCK_TGT_NOTSET )
                    pTHIS->m_SavedTarget = pTHIS->m_pIMEXObj->GetTarget();

                if( pTHIS->m_SavedTarget == TBLOCK_TGT_NOTSET )
                {
                    STATUS::Print( STATUS::MSG_ERROR, "ERROR: No target specified for %s.\n", pTHIS->m_pIMEXObj->GetFilename() );
                }
                else
                {
                    // execute a re-export of imported data
                    pTHIS->m_pIMEXObj->ExportCurData( NULL, pTHIS->m_SavedTarget, IMEX_Done, (u32)pTHIS );
                    return TRUE;
                }
            }
        }
    }
    else if( pTHIS->m_bExporting )
    {
        pTHIS->m_bExporting = FALSE;
    }

    // once other processing has ceased, continue to execute more cmdline params
    pTHIS->CommandLineExecute();

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// OUTPUT MESSAGE HANDLERS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void TBlockCmdLineDlg::StatusPrintCB( u32 cbParam, s32 MsgType, const char* pMsg )
{
    TBlockCmdLineDlg* pTHIS = (TBlockCmdLineDlg*)cbParam;

    int            MsgCount;
    CHARRANGE      CurSel;
    CHARFORMAT     CharFmt;
    CRichEditCtrl* pREdit;

    CharFmt.cbSize = sizeof(CHARFORMAT);

    // Setup text format for type of message displayed
    switch( MsgType )
    {
        case STATUS::MSG_IMPORTANT:
            CharFmt.dwMask      = CFM_COLOR | CFM_BOLD;
            CharFmt.dwEffects   = CFE_BOLD;
            CharFmt.crTextColor = RGB( 0, 0, 0 );

            pTHIS->m_NumMsgImportant++;
            break;

        case STATUS::MSG_WARNING:
            CharFmt.dwMask      = CFM_COLOR | CFM_BOLD;
            CharFmt.dwEffects   = 0;
            CharFmt.crTextColor = RGB( 0, 0, 255 );

            pTHIS->m_NumMsgWarning++;
            break;

        case STATUS::MSG_ERROR:
            CharFmt.dwMask      = CFM_COLOR | CFM_BOLD;
            CharFmt.dwEffects   = CFE_BOLD;
            CharFmt.crTextColor = RGB( 255, 0, 0 );

            pTHIS->m_NumMsgError++;
            break;

        case STATUS::MSG_NORMAL:
        default:
            CharFmt.dwMask      = CFM_COLOR | CFM_BOLD;
            CharFmt.dwEffects   = 0; //CFE_AUTOCOLOR;
            CharFmt.crTextColor = RGB( 0, 0, 0 );

            pTHIS->m_NumMsgNormal++;
            break;
    }

    pREdit = (CRichEditCtrl*)pTHIS->GetDlgItem( IDC_RICHEDIT_OUTPUT );

    // Save current selection
    pREdit->GetSel( CurSel );

    // Set new selection to end of text
    pREdit->SetSel( -1, -1 );

    // Set text format for message and print it
    pREdit->SetSelectionCharFormat( CharFmt );
    pREdit->ReplaceSel( pMsg );

    // restore the previous selection
    pREdit->SetSel( CurSel );

    // handle logging to file
    if( pTHIS->m_pLogFile != NULL )
    {
        fprintf( pTHIS->m_pLogFile, pMsg );

        MsgCount = pTHIS->m_NumMsgError + pTHIS->m_NumMsgImportant + pTHIS->m_NumMsgNormal + pTHIS->m_NumMsgWarning;

        // if several messages have been generated, flush them to disk to
        // ensure that the log file is updated in the event of program failure
        if( 0 == (MsgCount % 8) )
            fflush( pTHIS->m_pLogFile );
    }
}

//==========================================================================

void TBlockCmdLineDlg::StatusProgStartCB( u32 cbParam, s32 EndValue )
{
    TBlockCmdLineDlg* pTHIS = (TBlockCmdLineDlg*)cbParam;

    CProgressCtrl* pProgress;

    pProgress = (CProgressCtrl*)pTHIS->GetDlgItem( IDC_PROGRESS_EXPORT );

    // Set the new range of the progress bar and start at zero
    pProgress->SetRange32( 0, (int)EndValue );
    pProgress->SetPos( 0 );
}

//==========================================================================

void TBlockCmdLineDlg::StatusProgStepCB( u32 cbParam, s32 NSteps )
{
    TBlockCmdLineDlg* pTHIS = (TBlockCmdLineDlg*)cbParam;

    CProgressCtrl* pProgress;

    pProgress = (CProgressCtrl*)pTHIS->GetDlgItem( IDC_PROGRESS_EXPORT );

    // Increment progress bar position
    pProgress->OffsetPos( NSteps );
}

//==========================================================================
