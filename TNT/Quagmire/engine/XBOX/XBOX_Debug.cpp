////////////////////////////////////////////////////////////////////////////
//
// XBOX_Debug.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_debug.hpp"
#include "x_plus.hpp"
#include "x_time.hpp"
#include "x_stdio.hpp"

#include "Q_Engine.hpp"
#include "Q_Input.hpp"
#include "Q_XBOX.hpp"

#include "E_SMem.hpp"

#include "XBOX_Input.hpp"


////////////////////////////////////////////////////////////////////////////
// DEFINES AND CONSTANTS
////////////////////////////////////////////////////////////////////////////

#ifdef X_DEBUG
    #define DEBUGGER_MESSAGE( str )     x_printf( str );
#else
    #define DEBUGGER_MESSAGE( str )     ((void)0)
#endif

#define CHARS_PER_LINE      31

#define VERIFY_STRING       "*** VERIFICATION FAILURE ***"
#define ASSERT_STRING       "*** PC LOAD LETTER ***"
#define UNKNOWN_STRING      "*** UNKNOWN FAILURE ***"

#define UNKNOWN_FILENAME    "NO FILE"

#define BTN_MSG_DEBUG       "Press (L)+(R)+(A) to Debug"
#define BTN_MSG_SKIP        "Press (L)+(R)+(B) to Skip"
#define BTN_MSG_SUPPRESS    "Press (L)+(R)+(Y) to Suppress"

#define ONSCREEN_LOC_X      1
#define ONSCREEN_LOC_Y      2

#define FLIP_ONSCREEN_TIME    1.00f
#define FLIP_OFFSCREEN_TIME   0.25f


////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

inline void GetStrLen( s32& StrLen, char* pStr )
{
    StrLen = 0;
    while( pStr[ StrLen ] != '\0' )
    {
        StrLen++;
    }
}

//==========================================================================

static void XBOXPrintOnScreenAssert( s32    Type,
                                   char*  pFilename,
                                   s32    Line,
                                   char*  pExpStr,
                                   char*  pMsgStr )
{
    s32   StrLen   = 0;
    char  CharSave = 0;
    char* pFilePtr = NULL;

    // Create a local buffer to store the complete message, if it's too long, this function will try to cut it down
    // and if that happens, we could be accessing memory that is reserved for constant strings, which in turn 
    // will cause a crash.
    static char LocalMessageBuffer[2048];

    if( pMsgStr )
        x_strncpy( LocalMessageBuffer, pMsgStr, 2048 );
    else
        LocalMessageBuffer[0] = NULL;

    //--- only attempt to draw the last few characters of the file on the screen
    if( pFilename != NULL )
    {
        GetStrLen( StrLen, pFilename );

        if( StrLen > CHARS_PER_LINE )
            pFilePtr = &pFilename[ StrLen-CHARS_PER_LINE ];
        else
            pFilePtr = pFilename;
    }
    else
    {
        pFilePtr = UNKNOWN_FILENAME;
    }

    //--- display the title
    switch( Type )
    {
        case XDB_VERIFY:
        case XDB_VERIFYS:
            x_printfxy( ONSCREEN_LOC_X, ONSCREEN_LOC_Y, VERIFY_STRING );
            break;

        case XDB_ASSERT:
        case XDB_ASSERTS:
            x_printfxy( ONSCREEN_LOC_X, ONSCREEN_LOC_Y, ASSERT_STRING );
            break;

        default:
            x_printfxy( ONSCREEN_LOC_X, ONSCREEN_LOC_Y, UNKNOWN_STRING );
            break;
    }

    //--- display the location of the debug check
    if( pFilePtr != NULL )
        x_printfxy( ONSCREEN_LOC_X, ONSCREEN_LOC_Y + 1, "File: %s", pFilePtr );

    x_printfxy( ONSCREEN_LOC_X, ONSCREEN_LOC_Y + 2, "Line: %d", Line );

    //--- display the expression (if there is one)
    if( pExpStr )
        x_printfxy( ONSCREEN_LOC_X, ONSCREEN_LOC_Y + 3, pExpStr );

    //--- display the message (if there is one)
    if( pMsgStr )
    {
        //--- get the length of the message string
        GetStrLen( StrLen, LocalMessageBuffer );

        //--- if it is two long for one line, cut it in half
        if( StrLen > CHARS_PER_LINE )
        {
            CharSave = LocalMessageBuffer[CHARS_PER_LINE];
            LocalMessageBuffer[CHARS_PER_LINE] = '\0';
            x_printfxy( ONSCREEN_LOC_X, ONSCREEN_LOC_Y + 4, LocalMessageBuffer );
            LocalMessageBuffer[CHARS_PER_LINE] = CharSave;
            x_printfxy( ONSCREEN_LOC_X, ONSCREEN_LOC_Y + 5, &LocalMessageBuffer[CHARS_PER_LINE] );
        }
        else
        {   
            x_printfxy( ONSCREEN_LOC_X, ONSCREEN_LOC_Y + 4, LocalMessageBuffer );
        }
    }

    x_printfxy( ONSCREEN_LOC_X, ONSCREEN_LOC_Y + 6, BTN_MSG_DEBUG );
    x_printfxy( ONSCREEN_LOC_X, ONSCREEN_LOC_Y + 7, BTN_MSG_SKIP );
    x_printfxy( ONSCREEN_LOC_X, ONSCREEN_LOC_Y + 8, BTN_MSG_SUPPRESS );
}

//==========================================================================

// returns TRUE if chose to debug
xbool XBOXAssertHandler( s32    Type,
                         char*  pFilename,
                         s32    Line,
                         char*  pExpStr,
                         char*  pMsgStr )
{
    f64     fTime;
    s32     i;
    xbool   LTrig, RTrig, YBtn, BBtn, ABtn;
    xbool   bInRenderMode = FALSE;
    xbool   RetValue      = TRUE;
    s32     FlipTimeIdx   = 0;
    f64     PageFlipTime[2] = { FLIP_ONSCREEN_TIME, FLIP_OFFSCREEN_TIME };

    input_event IEvent;

    static xbool bAssertInProgress = FALSE;

    if( bAssertInProgress )
    {
        x_printf( "\nRecursive ASSERT has happened. (Eng. Graphics?)\n" );
        return FALSE;
    }
    bAssertInProgress = TRUE;

    //--- For the PageFlip to succeed, we must not be in a render mode.
    if( ENG_GetRenderMode() )
    {
        bInRenderMode = TRUE;
        ENG_EndRenderMode();
    }

    //--- If ASSERT happens before client app can pop stack markers, we must
    //    reset SMEM so we don't get an unneeded recursive assert.
    SMEM_ResetBuffer();

    DEBUGGER_MESSAGE( "\n" );
    DEBUGGER_MESSAGE( BTN_MSG_DEBUG"\n" );
    DEBUGGER_MESSAGE( BTN_MSG_SKIP"\n" );
    DEBUGGER_MESSAGE( BTN_MSG_SUPPRESS"\n" );

    //--- Wait until the user has noticed the message and reacted.
    fTime = x_GetTime();
    while( TRUE )
    {
        //--- Flip between screens every FLIP_SCREEN_TIME amount.
        if( x_GetElapsedTime(fTime) > PageFlipTime[FlipTimeIdx] )
        {
            XBOXPrintOnScreenAssert( Type, pFilename, Line, pExpStr, pMsgStr );
            ENG_PageFlip( FlipTimeIdx );
            FlipTimeIdx ^= 1;
            fTime = x_GetTime();
        }
        else
        {
            INPUT_CheckDevices();
        }


        INPUT_ClearDebounce();

        //--- Empty the input event buffer
        while( INPUT_GetNextEvent( IEvent ) )
        {
        }

        //--- Check for button input
        LTrig = RTrig = ABtn = BBtn = YBtn = FALSE;
        for( i = 0; i < 4; i++ )
        {
            LTrig = LTrig || PRESSED( i, ANALOG_L );
            RTrig = RTrig || PRESSED( i, ANALOG_R );
            ABtn  = ABtn  || PRESSED( i, ANALOG_A );
            BBtn  = BBtn  || PRESSED( i, ANALOG_B );
            YBtn  = YBtn  || PRESSED( i, ANALOG_Y );
        }

        if( LTrig && RTrig && ABtn )
        {
            RetValue = FALSE;   // DEBUG
            break;
        }
        else if( LTrig && RTrig && BBtn )
        {
            RetValue = TRUE;    // SKIP
            break;
        }
        else if( LTrig && RTrig && YBtn )
        {
            x_DebugSuppress( pFilename, Line );
            RetValue = TRUE;    // SUPPRESS
            break;
        }
    }

    //--- If we had to end a RenderMode, then we need to restart it
    if( bInRenderMode )
        ENG_BeginRenderMode();

    bAssertInProgress = FALSE;

    return RetValue;
}

//==========================================================================

void DEBUG_InitModule( void )
{
    //--- Set x_files debug handler
    x_DebugSetHandler( XBOXAssertHandler );

}

//==========================================================================
