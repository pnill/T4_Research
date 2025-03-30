////////////////////////////////////////////////////////////////////////////
//
// TBlockCmdLineTxt.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "TBlockCmdLineTxt.h"


namespace TBCMDLINE
{

////////////////////////////////////////////////////////////////////////////
// CONSTANTS
////////////////////////////////////////////////////////////////////////////

#define CMD_SEPARATOR       ','
#define PARAM_SEPARATOR     ':'

// Command names
#define CMD_GLOBAL          "GBL"
#define CMD_SWIZZLE         "SWZ"
#define CMD_COMPRESS        "CMP"
#define CMD_INTENSITYMAP    "IMAP"
#define CMD_4BIT_CONVERSION "4BIT"
#define CMD_BUILDMIPS       "MIP"
#define CMD_CUSTOMMIP       "MIPCSTM"
#define CMD_NMIPS           "NUMMIPS"
#define CMD_APPLYALPHA      "ALPHA"
#define CMD_ALPHABYFNAME    "ALFNAME"
#define CMD_ALPHAPUNCH      "ALFPUNCH"
#define CMD_ALPHAGENCLR     "ALFGENC"
#define CMD_COLORSCALE      "CLRSCL"

////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

static s32 ProcessCommand( const char* pCmd, TBlockSettings& rTBS )
{
    s32 i;
    const char* pParam;
    char  CmdName[32];
    xbool SeparatorFound;
    xbool bT0, bT1, bT2, bT3;
    u32   uTV0;

    // Find first non-whitespace character
    while( *pCmd != '\0' )
    {
        if( x_isspace( *pCmd ) )
            pCmd++;
        else
            break;
    }

    if( *pCmd == '\0' )
        return RCODE_NO_SWITCH;

    pParam = pCmd;

    SeparatorFound = FALSE;

    // find command's parameter
    while( *pParam != '\0' )
    {
        if( SeparatorFound )
        {
            // when non-whitespace is found after separator, that's the value
            if( !x_isspace( *pParam ) )
                break;
        }
        if( *pParam == PARAM_SEPARATOR )
        {
            // Found separator, next non-whitespace char is parameter value
            SeparatorFound = TRUE;
        }
        pParam++;
    }

    // check if parameter wasn't found
    if( *pParam == '\0' )
        return RCODE_SWITCH_PARAM_MISSING;

    // check if command string length is too large
    if( (pParam - pCmd) > 32 )
        return RCODE_UNKNOWN_SWITCH;

    // copy command name only to test with
    for( i = 0; i < 32; i++ )
    {
        if( x_isspace( pCmd[i] ) || (pCmd[i] == PARAM_SEPARATOR) || (&pCmd[i] == pParam) )
        {
            CmdName[i] = '\0';
            break;
        }
        CmdName[i] = pCmd[i];
    }

    // check for specific switches, and do whatever operation is necessary for it
    if( !x_stricmp( CmdName, CMD_GLOBAL ) )
    {
        rTBS.SetUseGlobal( pParam[0] != '0' );
    }
    else if( !x_stricmp( CmdName, CMD_SWIZZLE ) )
    {
        rTBS.SetSwizzle( pParam[0] != '0' );
    }
    else if( !x_stricmp( CmdName, CMD_COMPRESS ) )
    {
        rTBS.SetCompressFmt( pParam[0] != '0' );
    }
    else if( !x_stricmp( CmdName, CMD_INTENSITYMAP ) )
    {
        rTBS.SetIntensityMap( pParam[0] != '0' );
    }
    else if( !x_stricmp( CmdName, CMD_4BIT_CONVERSION ) )
    {
        rTBS.SetAllow4BitConversionTest( pParam[0] != '0' );
    }
    else if( !x_stricmp( CmdName, CMD_BUILDMIPS ) )
    {
        rTBS.GetBuildMips( bT1, uTV0 );
        rTBS.SetBuildMips( pParam[0] != '0', bT1, uTV0 );
    }
    else if( !x_stricmp( CmdName, CMD_CUSTOMMIP ) )
    {
        bT0 = rTBS.GetBuildMips( bT1, uTV0 );
        rTBS.SetBuildMips( bT0, pParam[0] != '0', uTV0 );
    }
    else if( !x_stricmp( CmdName, CMD_NMIPS ) )
    {
        bT0 = rTBS.GetBuildMips( bT1, uTV0 );
        uTV0 = x_atoi( pParam );
        rTBS.SetBuildMips( bT0, bT1, uTV0 );
    }
    else if( !x_stricmp( CmdName, CMD_APPLYALPHA ) )
    {
        rTBS.GetApplyAlpha( bT1, bT2, bT3 );
        rTBS.SetApplyAlpha( pParam[0] != '0', bT1, bT2, bT3 );
    }
    else if( !x_stricmp( CmdName, CMD_ALPHABYFNAME ) )
    {
        bT0 = rTBS.GetApplyAlpha( bT1, bT2, bT3 );
        rTBS.SetApplyAlpha( bT0, pParam[0] != '0', bT2, bT3 );
    }
    else if( !x_stricmp( CmdName, CMD_ALPHAPUNCH ) )
    {
        bT0 = rTBS.GetApplyAlpha( bT1, bT2, bT3 );
        rTBS.SetApplyAlpha( bT0, bT1, pParam[0] != '0', bT3 );
    }
    else if( !x_stricmp( CmdName, CMD_ALPHAGENCLR ) )
    {
        bT0 = rTBS.GetApplyAlpha( bT1, bT2, bT3 );
        rTBS.SetApplyAlpha( bT0, bT1, bT2, pParam[0] != '0' );
    }
    else if( !x_stricmp( CmdName, CMD_COLORSCALE ) )
    {
        char ExParam[4];
        s32  CurIdx;
        u32  uRGBA[4];

        rTBS.GetColorScale( uRGBA[0], uRGBA[1], uRGBA[2], uRGBA[3] );
        bT0 = (pParam[0] != '0') ? TRUE : FALSE;

        // move to next param, which should be the start of the RGBA values
        while( *pParam != '\0' )
        {
            if( *pParam == PARAM_SEPARATOR )
                break;
            pParam++;
        }

        if( (pParam[0] == '\0') || (pParam[1] == '\0') )
            return RCODE_SWITCH_PARAM_ERROR;

        // move past the separator
        pParam++;

        i          = 0;
        CurIdx     = 0;
        ExParam[0] = '\0';

        while( *pParam != '\0' )
        {
            // if separator reached, convert current string to value
            if( *pParam == PARAM_SEPARATOR )
            {
                // make sure we got characters to work with
                if( i == 0 )
                    return RCODE_SWITCH_PARAM_ERROR;

                // null terminate parameter, convert to number, start over
                ExParam[i]    = '\0';
                uRGBA[CurIdx] = x_atoi( ExParam );
                ExParam[0]    = '\0';
                i = 0;

                // move to next input parameter
                CurIdx++;
                if( CurIdx >= 4 )
                    break;
            }
            else
            {
                // only save non-whitespace chars
                if( !x_isspace( *pParam ) )
                {
                    ExParam[i] = *pParam;
                    i++;
                    if( i >= 4 )
                        return RCODE_SWITCH_PARAM_ERROR;
                }
            }

            // step to next char
            pParam++;

            if( *pParam == '\0' )
            {
                if( CurIdx < 3 )
                    return RCODE_SWITCH_PARAM_ERROR;
                if( i == 0 )
                    return RCODE_SWITCH_PARAM_ERROR;

                // null terminate parameter, convert to number, end loop
                ExParam[i]    = '\0';
                uRGBA[CurIdx] = x_atoi( ExParam );
                break;
            }
        }

        rTBS.SetColorScale( bT0, uRGBA[0], uRGBA[1], uRGBA[2], uRGBA[3] );
    }
    else
    {
        return RCODE_UNKNOWN_SWITCH;
    }

    return RCODE_OK;
}

//==========================================================================

s32 Parse( const char* pCmdLine, TBlockSettings& rTBS, s32& rFirstCmdIndex )
{
    s32  i;
    s32  NCmdsFound;
    s32  CmdStartIdx;
    s32  CmdEndIdx;
    s32  RCode;
    char SingleCmd[128];

    NCmdsFound  = 0;
    CmdStartIdx = 0;
    CmdEndIdx   = 0;

    // Find first command
    while( TRUE )
    {
        if( pCmdLine[CmdStartIdx] == CMD_SEPARATOR )
        {
            CmdStartIdx++;
            break;
        }
        else if( pCmdLine[CmdStartIdx] == '\0' )
            break;

        CmdStartIdx++;
    }

    // No commands found?
    if( pCmdLine[CmdStartIdx] == '\0' )
    {
        rFirstCmdIndex = CmdStartIdx;
        return RCODE_NO_SWITCH;
    }

    // Save start char index of commands(might be end of cmd string)
    rFirstCmdIndex = CmdStartIdx - 1;

    // Run through the rest of cmdline string
    while( TRUE )
    {
        // Find end of command(which is start of next command)
        CmdEndIdx = CmdStartIdx + 1;

        while( TRUE )
        {
            if( pCmdLine[CmdEndIdx] == CMD_SEPARATOR )
                break;
            else if( pCmdLine[CmdEndIdx] == '\0' )
                break;

            CmdEndIdx++;
        }

        // Check command string size
        if( (CmdEndIdx - CmdStartIdx) > sizeof(SingleCmd) )
            return RCODE_UNKNOWN_SWITCH;

        // Copy single command to temp string
        for( i = 0; i < (CmdEndIdx - CmdStartIdx); i++ )
        {
            SingleCmd[i] = pCmdLine[CmdStartIdx + i];
        }
        SingleCmd[ (CmdEndIdx - CmdStartIdx) ] = '\0';

        // Process the command
        RCode = ProcessCommand( SingleCmd, rTBS );

        // Increment number of commands
        NCmdsFound++;

        // if end of command string, end parsing
        if( pCmdLine[CmdEndIdx] == '\0' )
            break;

        CmdStartIdx = CmdEndIdx + 1;
    }

    return RCODE_OK;
}

//==========================================================================

s32 Generate( char* pCmdLine, TBlockSettings& rTBS, TBlockSettings& rGlobalTBS )
{
    char TmpCmd[32];

    xbool bT0, bT1, bT2, bT3;
    xbool bG0, bG1, bG2, bG3;
    u32   uTV0, uTV1, uTV2, uTV3;
    u32   uGV0, uGV1, uGV2, uGV3;

    // if settings are different than the global ones, then output those switch changes
    if( rTBS.GetUseGlobal() != rGlobalTBS.GetUseGlobal() )
    {
        x_sprintf( TmpCmd, "%c%s%c%c", CMD_SEPARATOR, CMD_GLOBAL, PARAM_SEPARATOR, (rTBS.GetUseGlobal() ?'1':'0') );
        x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );
    }
    if( rTBS.GetSwizzle() != rGlobalTBS.GetSwizzle() )
    {
        x_sprintf( TmpCmd, "%c%s%c%c", CMD_SEPARATOR, CMD_SWIZZLE, PARAM_SEPARATOR, (rTBS.GetSwizzle() ?'1':'0') );
        x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );
    }
    if( rTBS.GetCompressFmt() != rGlobalTBS.GetCompressFmt() )
    {
        x_sprintf( TmpCmd, "%c%s%c%c", CMD_SEPARATOR, CMD_COMPRESS, PARAM_SEPARATOR, (rTBS.GetCompressFmt() ?'1':'0') );
        x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );
    }
    if( rTBS.GetIntensityMap() != rGlobalTBS.GetIntensityMap() )
    {
        x_sprintf( TmpCmd, "%c%s%c%c", CMD_SEPARATOR, CMD_INTENSITYMAP, PARAM_SEPARATOR, (rTBS.GetIntensityMap() ?'1':'0') );
        x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );
    }
    if( rTBS.GetAllow4BitConversionTest( ) != rGlobalTBS.GetAllow4BitConversionTest( ) )
    {
        x_sprintf( TmpCmd, "%c%s%c%c", CMD_SEPARATOR, CMD_4BIT_CONVERSION, PARAM_SEPARATOR, (rTBS.GetAllow4BitConversionTest() ?'1':'0') );
        x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );
    }

    bT0 = rTBS.GetBuildMips( bT1, uTV0 );
    bG0 = rGlobalTBS.GetBuildMips( bG1, uGV0 );

    if( bT0 != bG0 )
    {
        x_sprintf( TmpCmd, "%c%s%c%c", CMD_SEPARATOR, CMD_BUILDMIPS, PARAM_SEPARATOR, (bT0 ?'1':'0') );
        x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );
    }
    if( bT1 != bG1 )
    {
        x_sprintf( TmpCmd, "%c%s%c%c", CMD_SEPARATOR, CMD_CUSTOMMIP, PARAM_SEPARATOR, (bT1 ?'1':'0') );
        x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );
    }
    if( uTV0 != uGV0 )
    {
        x_sprintf( TmpCmd, "%c%s%c%d", CMD_SEPARATOR, CMD_NMIPS, PARAM_SEPARATOR, (s32)uTV0 );
        x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );
    }

    bT0 = rTBS.GetApplyAlpha( bT1, bT2, bT3 );
    bG0 = rGlobalTBS.GetApplyAlpha( bG1, bG2, bG3 );

    if( bT0 != bG0 )
    {
        x_sprintf( TmpCmd, "%c%s%c%c", CMD_SEPARATOR, CMD_APPLYALPHA, PARAM_SEPARATOR, (bT0 ?'1':'0') );
        x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );
    }
    if( bT1 != bG1 )
    {
        x_sprintf( TmpCmd, "%c%s%c%c", CMD_SEPARATOR, CMD_ALPHABYFNAME, PARAM_SEPARATOR, (bT1 ?'1':'0') );
        x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );
    }
    if( bT2 != bG2 )
    {
        x_sprintf( TmpCmd, "%c%s%c%c", CMD_SEPARATOR, CMD_ALPHAPUNCH, PARAM_SEPARATOR, (bT2 ?'1':'0') );
        x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );
    }
    if( bT3 != bG3 )
    {
        x_sprintf( TmpCmd, "%c%s%c%c", CMD_SEPARATOR, CMD_ALPHAGENCLR, PARAM_SEPARATOR, (bT3 ?'1':'0') );
        x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );
    }

    bT0 = rTBS.GetColorScale( uTV0, uTV1, uTV2, uTV3 );
    bG0 = rGlobalTBS.GetColorScale( uGV0, uGV1, uGV2, uGV3 );

    if( (bT0 != bG0) || (uTV0 != uGV0) || (uTV1 != uGV1) || (uTV2 != uGV2) || (uTV3 != uGV3) )
    {
        x_sprintf( TmpCmd, "%c%s%c%c%c%d%c%d%c%d%c%d", CMD_SEPARATOR, CMD_COLORSCALE, PARAM_SEPARATOR, (bT0 ?'1':'0'),
                    PARAM_SEPARATOR, uTV0, PARAM_SEPARATOR, uTV1, PARAM_SEPARATOR, uTV2, PARAM_SEPARATOR, uTV3 );
        x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );
    }

    return RCODE_OK;
}

//==========================================================================

s32 Generate( char* pCmdLine, TBlockSettings& rTBS )
{
    char TmpCmd[32];

    xbool bT0, bT1, bT2, bT3;
    u32   uTV0, uTV1, uTV2, uTV3;

    // Generate all switches regardless of their value
    x_sprintf( TmpCmd, "%c%s%c%c", CMD_SEPARATOR, CMD_GLOBAL,           PARAM_SEPARATOR, (rTBS.GetUseGlobal() ?'1':'0') );
    x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );
    x_sprintf( TmpCmd, "%c%s%c%c", CMD_SEPARATOR, CMD_SWIZZLE,          PARAM_SEPARATOR, (rTBS.GetSwizzle() ?'1':'0') );
    x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );
    x_sprintf( TmpCmd, "%c%s%c%c", CMD_SEPARATOR, CMD_COMPRESS,         PARAM_SEPARATOR, (rTBS.GetCompressFmt() ?'1':'0') );
    x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );
    x_sprintf( TmpCmd, "%c%s%c%c", CMD_SEPARATOR, CMD_INTENSITYMAP,     PARAM_SEPARATOR, (rTBS.GetIntensityMap() ?'1':'0') );
    x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );
    x_sprintf( TmpCmd, "%c%s%c%c", CMD_SEPARATOR, CMD_4BIT_CONVERSION,  PARAM_SEPARATOR, (rTBS.GetAllow4BitConversionTest() ?'1':'0') );
    x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );

    bT0 = rTBS.GetBuildMips( bT1, uTV0 );
    x_sprintf( TmpCmd, "%c%s%c%c", CMD_SEPARATOR, CMD_BUILDMIPS, PARAM_SEPARATOR, (bT0 ?'1':'0') );
    x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );
    x_sprintf( TmpCmd, "%c%s%c%c", CMD_SEPARATOR, CMD_CUSTOMMIP, PARAM_SEPARATOR, (bT1 ?'1':'0') );
    x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );
    x_sprintf( TmpCmd, "%c%s%c%d", CMD_SEPARATOR, CMD_NMIPS,     PARAM_SEPARATOR, (s32)uTV0 );
    x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );

    bT0 = rTBS.GetApplyAlpha( bT1, bT2, bT3 );
    x_sprintf( TmpCmd, "%c%s%c%c", CMD_SEPARATOR, CMD_APPLYALPHA,   PARAM_SEPARATOR, (bT0 ?'1':'0') );
    x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );
    x_sprintf( TmpCmd, "%c%s%c%c", CMD_SEPARATOR, CMD_ALPHABYFNAME, PARAM_SEPARATOR, (bT1 ?'1':'0') );
    x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );
    x_sprintf( TmpCmd, "%c%s%c%c", CMD_SEPARATOR, CMD_ALPHAPUNCH,   PARAM_SEPARATOR, (bT2 ?'1':'0') );
    x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );
    x_sprintf( TmpCmd, "%c%s%c%c", CMD_SEPARATOR, CMD_ALPHAGENCLR,  PARAM_SEPARATOR, (bT3 ?'1':'0') );
    x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );

    bT0 = rTBS.GetColorScale( uTV0, uTV1, uTV2, uTV3 );
    x_sprintf( TmpCmd, "%c%s%c%c%c%d%c%d%c%d%c%d", CMD_SEPARATOR, CMD_COLORSCALE, PARAM_SEPARATOR, (bT0 ?'1':'0'),
                PARAM_SEPARATOR, uTV0, PARAM_SEPARATOR, uTV1, PARAM_SEPARATOR, uTV2, PARAM_SEPARATOR, uTV3 );
    x_strncat( pCmdLine, TmpCmd, MAX_LENGTH );


    return RCODE_OK;
}

////////////////////////////////////////////////////////////////////////////

} // namespace TBCMDLINE
