////////////////////////////////////////////////////////////////////////////
//
// TBlockCmdLineDlg.h
//
////////////////////////////////////////////////////////////////////////////

#ifndef TBLOCKCMDLINEDLG_H_INCLUDED
#define TBLOCKCMDLINEDLG_H_INCLUDED

////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#ifndef TBLOCK_STDAFX_H_INCLUDED
    #error "You must include StdAfx.h before this file."
#endif

#include "Resource.h"

#include "StatusOutput.h"
#include "TBlockIMEXObj.h"
#include "TBlockCmdLineApp.h"


////////////////////////////////////////////////////////////////////////////
// TBlockCmdLineDlg dialog
////////////////////////////////////////////////////////////////////////////

class TBlockCmdLineDlg : public CDialog
{
public:
    TBlockCmdLineDlg( TBlockCmdLineApp& rAppCmdLine,
                      TBlockIMEXObj&    rIMEXObj,
                      CWnd*             pParent = NULL );

    ~TBlockCmdLineDlg( void );

    BOOL Execute( void );

    //{{AFX_DATA(TBlockCmdLineDlg)
    enum { IDD = IDD_CMDLINE_DIALOG };
        // NOTE: the ClassWizard will add data members here
    //}}AFX_DATA

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(TBlockCmdLineDlg)
    //}}AFX_VIRTUAL

protected:

    BOOL                m_bExitOnFinish;
    BOOL                m_bWaitAfterFinish;
    BOOL                m_bSkipErrorOnFinish;

    TBlockCmdLineApp*   m_pAppCmdLine;
    TBlockIMEXObj*      m_pIMEXObj;

    TBLOCK_TARGET       m_SavedTarget;
    BOOL                m_bImporting;
    BOOL                m_bExporting;

    FILE*               m_pLogFile;

    int                 m_NumMsgNormal;
    int                 m_NumMsgImportant;
    int                 m_NumMsgWarning;
    int                 m_NumMsgError;

    void CommandLineExecute( void );
    void CloseLogFile( void );

    static xbool IMEX_Done( u32 cbParam, TBlockIMEXObj* pIMEXObj, s32 RetCode );

    static void StatusPrintCB    ( u32 cbParam, s32 MsgType, const char* pMsg );
    static void StatusProgStartCB( u32 cbParam, s32 EndValue );
    static void StatusProgStepCB ( u32 cbParam, s32 NSteps );

    // Generated message map functions
    //{{AFX_MSG(TBlockCmdLineDlg)
    virtual void OnOK();
    virtual void OnCancel();
    afx_msg void OnButtonClose();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // TBLOCKCMDLINEDLG_H_INCLUDED
