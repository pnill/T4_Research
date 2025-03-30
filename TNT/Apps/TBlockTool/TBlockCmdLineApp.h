////////////////////////////////////////////////////////////////////////////
//
// TBlockCmdLineApp.h
//
////////////////////////////////////////////////////////////////////////////

#ifndef TBLOCKCMDLINEAPP_H_INCLUDED
#define TBLOCKCMDLINEAPP_H_INCLUDED

////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#ifndef TBLOCK_STDAFX_H_INCLUDED
    #error "You must include StdAfx.h before this file."
#endif


////////////////////////////////////////////////////////////////////////////
// CONSTANTS
////////////////////////////////////////////////////////////////////////////

const int MAX_NUM_COMMANDS  = 32;
const int MAX_PARAM_LENGTH  = 256;


enum CMDLINE_FLAG
{
    CMD_NOTHING = -1,

    CMD_EXIT_ONDONE,
    CMD_WAIT_ONCLOSE,
    CMD_NO_WAIT_ONERROR,
    CMD_LOGFILE,
    CMD_MINIMIZE,

    CMD_IMPORT_FILE,
    CMD_EXPORT_FILE,
    CMD_IMPORT_EXPORT_FILE,

    CMD_TARGET_PS2,
    CMD_TARGET_GAMECUBE,
    CMD_TARGET_XBOX,
    CMD_TARGET_PC,

    CMD_DEBUG_TINTMIPS,

    CMDLINE_FLAG_COUNT
};

const char CMD_FLAG_STRING[CMDLINE_FLAG_COUNT][8] =
{
    "EXIT",     // CMD_EXIT_ONDONE
    "WAIT",     // CMD_WAIT_ONCLOSE
    "NOERR",    // CMD_NO_WAIT_ONERROR
    "LOG",      // CMD_LOGFILE
    "MIN",      // CMD_MINIMIZE

    "I",        // CMD_IMPORT_FILE
    "E",        // CMD_EXPORT_FILE
    "IE",       // CMD_IMPORT_EXPORT_FILE

    "PS2",      // CMD_TARGET_PS2
    "GC",       // CMD_TARGET_GAMECUBE
    "XBOX",     // CMD_TARGET_XBOX
    "PC",       // CMD_TARGET_PC

    "TINT",     // CMD_DEBUG_TINTMIPS
};

const bool CMD_FLAG_NEEDS_PARAM[CMDLINE_FLAG_COUNT] =
{
    false,      // CMD_EXIT_ONDONE
    false,      // CMD_WAIT_ONCLOSE
    false,      // CMD_NO_WAIT_ONERROR
    true,       // CMD_LOGFILE
    false,      // CMD_MINIMIZE

    true,       // CMD_IMPORT_FILE
    true,       // CMD_EXPORT_FILE
    true,       // CMD_IMPORT_EXPORT_FILE

    false,      // CMD_TARGET_PS2
    false,      // CMD_TARGET_GAMECUBE
    false,      // CMD_TARGET_XBOX
    false,      // CMD_TARGET_PC

    true,       // CMD_DEBUG_TINTMIPS
};


////////////////////////////////////////////////////////////////////////////
// TBlockCmdLineApp class
////////////////////////////////////////////////////////////////////////////

class TBlockCmdLineApp : public CCommandLineInfo
{
  public:
    
    void Start  ( void )    { m_CurIndex = -1; }
    int  NumCmds( void )    { return m_NumCmds; }

    CMDLINE_FLAG NextCommand( char** pParam = NULL );
    CMDLINE_FLAG PrevCommand( char** pParam = NULL );

    void Clear      ( void );
    void ParseParam ( LPCTSTR pParam, BOOL IsFlag, BOOL IsLast );

    TBlockCmdLineApp( void );

  protected:
    CMDLINE_FLAG    m_Commands[MAX_NUM_COMMANDS];
    char            m_CmdParams[MAX_NUM_COMMANDS][MAX_PARAM_LENGTH];
    int             m_CurIndex;
    int             m_NumCmds;
};

////////////////////////////////////////////////////////////////////////////

#endif // TBLOCKCMDLINEAPP_H_INCLUDED