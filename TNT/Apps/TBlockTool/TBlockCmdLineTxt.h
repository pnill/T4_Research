////////////////////////////////////////////////////////////////////////////
//
// TBlockCmdLine.h
//
////////////////////////////////////////////////////////////////////////////

#ifndef TBLOCKCMDLINE_H_INCLUDED
#define TBLOCKCMDLINE_H_INCLUDED


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "TBlockTexture.h"


////////////////////////////////////////////////////////////////////////////
// TBlock Command Line Interface
////////////////////////////////////////////////////////////////////////////

namespace TBCMDLINE
{
    const s32 MAX_LENGTH = 256;

    enum CMDLINE_RETURN_CODE
    {
        RCODE_OK,
        RCODE_UNKNOWN_SWITCH,
        RCODE_SWITCH_PARAM_MISSING,
        RCODE_SWITCH_PARAM_ERROR,
        RCODE_NO_SWITCH,

        RCODE_COUNT
    };


    // Parses the cmdline text, only changing the settings that the cmdline
    // has(ex. if cmdline indicates apply alpha, then only that setting is changed)
    // FirstCmdIndex is filled with the index into pCmdLine that the first valid
    // command starts at.
    s32 Parse( const char* pCmdLine, TBlockSettings& rTBS, s32& rFirstCmdIndex );

    // Generates a cmdline for settings, but only generates the settings that are
    // different from the global settings(to avoid redundancy).
    s32 Generate( char* pCmdLine, TBlockSettings& rTBS, TBlockSettings& rGlobalTBS );

    // Generates a cmdline with all settings specified, useful for DOS-prompt global
    // settings
    s32 Generate( char* pCmdLine, TBlockSettings& rTBS );

} // namespace TBCMDLINE

////////////////////////////////////////////////////////////////////////////

#endif // TBLOCKCMDLINE_H_INCLUDED