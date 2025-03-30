////////////////////////////////////////////////////////////////////////////
//
// TBlockCmdLineApp.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "x_debug.hpp"

#include "TBlockCmdLineApp.h"


////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

TBlockCmdLineApp::TBlockCmdLineApp( void )
{
    Clear();
}

//==========================================================================

void TBlockCmdLineApp::Clear( void )
{
    int i;

    m_CurIndex = 0;
    m_NumCmds  = 0;

    for( i = 0; i < MAX_NUM_COMMANDS; i++ )
        m_Commands[i] = CMD_NOTHING;

    memset( m_CmdParams, 0, sizeof(m_CmdParams) );
}

//==========================================================================

void TBlockCmdLineApp::ParseParam( LPCTSTR pParam, BOOL IsFlag, BOOL IsLast )
{
    int i;

    if( m_NumCmds >= MAX_NUM_COMMANDS )
    {
        ASSERTS( m_NumCmds < MAX_NUM_COMMANDS, "Too many command line parameters" );
        return;
    }

    if( IsFlag )    //Command
    {
        for( i = 0; i < CMDLINE_FLAG_COUNT; i++ )
        {
            if( 0 == strcmp( pParam, CMD_FLAG_STRING[i] ) )
            {
                if( CMD_FLAG_NEEDS_PARAM[i] )
                {
                    if( !IsLast )
                    {
                        m_Commands[m_NumCmds] = (CMDLINE_FLAG)i;
                    }
                    else
                    {
                        //cmd ignored
                        ASSERTS( FALSE, "Command line switch needs parameter" );
                    }
                    break;
                }
                else
                {
                    m_Commands[m_NumCmds] = (CMDLINE_FLAG)i;
                    m_NumCmds++;
                    break;
                }
            }
        }
    }
    else    //Parameter
    {
        if( m_Commands[m_NumCmds] == CMD_NOTHING )
        {
            // If no command has been set, it's just a filename.
            // Interpret that as a file to import
            m_Commands[m_NumCmds] = CMD_IMPORT_FILE;
            strncpy( m_CmdParams[m_NumCmds], pParam, MAX_PARAM_LENGTH );
            m_NumCmds++;
        }
        else if( CMD_FLAG_NEEDS_PARAM[ m_Commands[m_NumCmds] ] )
        {
            strncpy( m_CmdParams[m_NumCmds], pParam, MAX_PARAM_LENGTH );
            m_NumCmds++;
        }
        else
        {
            //weird stuff happening if getting here
            ASSERTS( FALSE, "Unknown Error parsing command line" );
        }
    }
}

//==========================================================================

CMDLINE_FLAG TBlockCmdLineApp::NextCommand( char** pParam )
{
    if( m_CurIndex + 1 >= MAX_NUM_COMMANDS )
        return CMD_NOTHING;

    if( m_CurIndex + 1 >= m_NumCmds )
        return CMD_NOTHING;

    m_CurIndex++;
    if( pParam != NULL )
        *pParam = m_CmdParams[m_CurIndex];

    return m_Commands[m_CurIndex];
}

//==========================================================================

CMDLINE_FLAG TBlockCmdLineApp::PrevCommand( char** pParam )
{
    if( m_CurIndex <= 0 )
        return CMD_NOTHING;

    m_CurIndex--;
    if( pParam != NULL )
        *pParam = m_CmdParams[m_CurIndex];

    return m_Commands[m_CurIndex];
}

//==========================================================================
