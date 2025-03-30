////////////////////////////////////////////////////////////////////////////
//
// GC_Debug.cpp
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
#include "Q_GC.hpp"

#include "E_SMem.hpp"

#include "GC_Input.hpp"

#include <dolphin/gx.h>


////////////////////////////////////////////////////////////////////////////
// DEFINES AND CONSTANTS
////////////////////////////////////////////////////////////////////////////

// Use one of the following defines to control GameCube warnings
#define GC_WARNING_LEVEL    GX_WARN_NONE
//#define GC_WARNING_LEVEL    GX_WARN_SEVERE
//#define GC_WARNING_LEVEL    GX_WARN_MEDIUM
//#define GC_WARNING_LEVEL    GX_WARN_ALL

//--------------------------------------------------------------------------

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

//--------------------------------------------------------------------------

static void GCPrintOnScreenAssert( s32    Type,
                                   char*  pFilename,
                                   s32    Line,
                                   char*  pExpStr,
                                   char*  pMsgStr )
{
    s32   StrLen   = 0;
    char  CharSave = 0;
    char* pFilePtr = NULL;

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
        GetStrLen( StrLen, pMsgStr );

        //--- if it is two long for one line, cut it in half
        if( StrLen > CHARS_PER_LINE )
        {
            CharSave = pMsgStr[CHARS_PER_LINE];
            pMsgStr[CHARS_PER_LINE] = '\0';
            x_printfxy( ONSCREEN_LOC_X, ONSCREEN_LOC_Y + 4, pMsgStr );
            pMsgStr[CHARS_PER_LINE] = CharSave;
            x_printfxy( ONSCREEN_LOC_X, ONSCREEN_LOC_Y + 5, &pMsgStr[CHARS_PER_LINE] );
        }
        else
        {   x_printfxy( ONSCREEN_LOC_X, ONSCREEN_LOC_Y + 4, pMsgStr );
        }
    }

    x_printfxy( ONSCREEN_LOC_X, ONSCREEN_LOC_Y + 6, BTN_MSG_DEBUG );
    x_printfxy( ONSCREEN_LOC_X, ONSCREEN_LOC_Y + 7, BTN_MSG_SKIP );
    x_printfxy( ONSCREEN_LOC_X, ONSCREEN_LOC_Y + 8, BTN_MSG_SUPPRESS );
}

//--------------------------------------------------------------------------

// returns TRUE if chose to debug
xbool GameCubeAssertHandler( s32    Type, 
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
            GCPrintOnScreenAssert( Type, pFilename, Line, pExpStr, pMsgStr );
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
            LTrig = LTrig || PRESSED( i, L_TRIG );
            RTrig = RTrig || PRESSED( i, R_TRIG );
            ABtn  = ABtn  || PRESSED( i, A );
            BBtn  = BBtn  || PRESSED( i, B );
            YBtn  = YBtn  || PRESSED( i, Y );
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

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#if defined( X_DEBUG )

//==========================================================================

const  u8    MAX_WARNING_COUNT = 3;
const  u32   MAX_WARNING_ID    = 128;
static u8    GCWarningCount[MAX_WARNING_ID];

// Callback function for GameCube warnings, so breakpoint can be set here during
// debugging to determine where warnings happen.
static void GC_VerifyCB( GXWarningLevel Level,
                         u32            ID,
                         const char*    pMsg )
{
    if( ID >= MAX_WARNING_ID )
        return;

    //--- to prevent a warning from being printed many times, check the current count
    if( GCWarningCount[ID] < MAX_WARNING_COUNT )
    {
        x_printf( "GX WARNING %d, Level %d, %s\n", (s32)ID, (s32)Level, pMsg );
        GCWarningCount[ID]++;
    }
}

//==========================================================================

static OSErrorHandler s_OldEH_SystemReset;
static OSErrorHandler s_OldEH_MachineCheck;
static OSErrorHandler s_OldEH_DSI;
static OSErrorHandler s_OldEH_ISI;
static OSErrorHandler s_OldEH_Alignment;
static OSErrorHandler s_OldEH_PerformanceMonitor;
static OSErrorHandler s_OldEH_SystemInterrupt;

// Error handler function for GameCube OS, allows x_files assert to cut in
// before the default handler's exception loop(which the debugger can't seem to stop).
static void GC_OSErrorHandler( OSError Error, OSContext* pContext, ... )
{
    u32 dsisr;
    u32 dar;

    va_list vArgs;
    va_start( vArgs, pContext );

    dsisr = va_arg( vArgs, u32 );
    dar   = va_arg( vArgs, u32 );

    switch( Error )
    {
        case OS_ERROR_SYSTEM_RESET:
            ASSERTS( FALSE, "OS ERROR: SYSTEM RESET" );
            s_OldEH_SystemReset( Error, pContext, dsisr, dar );
            break;

        case OS_ERROR_MACHINE_CHECK:
            ASSERTS( FALSE, "OS ERROR: MACHINE CHECK" );
            s_OldEH_MachineCheck( Error, pContext, dsisr, dar );
            break;

        case OS_ERROR_DSI:
            ASSERTS( FALSE, "OS ERROR: DSI" );
            s_OldEH_DSI( Error, pContext, dsisr, dar );
            break;

        case OS_ERROR_ISI:
            ASSERTS( FALSE, "OS ERROR: ISI" );
            s_OldEH_ISI( Error, pContext, dsisr, dar );
            break;

        case OS_ERROR_ALIGNMENT:
            ASSERTS( FALSE, "OS ERROR: ALIGNMENT" );
            s_OldEH_Alignment( Error, pContext, dsisr, dar );
            break;

        case OS_ERROR_PERFORMACE_MONITOR:
            ASSERTS( FALSE, "OS ERROR: PERFORMANCE MONITOR" );
            s_OldEH_PerformanceMonitor( Error, pContext, dsisr, dar );
            break;

        case OS_ERROR_SYSTEM_INTERRUPT:
            ASSERTS( FALSE, "OS ERROR: SYSTEM INTERRUPT" );
            s_OldEH_SystemInterrupt( Error, pContext, dsisr, dar );
            break;
    }
}

//==========================================================================

#endif // defined( X_DEBUG )

//==========================================================================

void DEBUG_InitModule( void )
{
    //--- Set x_files debug handler
    x_DebugSetHandler( GameCubeAssertHandler );

#if defined( X_DEBUG )
    //--- clear out the array for warning count
    x_memset( GCWarningCount, 0, MAX_WARNING_ID * sizeof(u8) );

    //--- set gamecube warning level and callback
    GXSetVerifyLevel( GC_WARNING_LEVEL );
    GXSetVerifyCallback( (GXVerifyCallback)GC_VerifyCB );

    s_OldEH_SystemReset        = OSSetErrorHandler( OS_ERROR_SYSTEM_RESET,       GC_OSErrorHandler );
    s_OldEH_MachineCheck       = OSSetErrorHandler( OS_ERROR_MACHINE_CHECK,      GC_OSErrorHandler );
    s_OldEH_DSI                = OSSetErrorHandler( OS_ERROR_DSI,                GC_OSErrorHandler );
    s_OldEH_ISI                = OSSetErrorHandler( OS_ERROR_ISI,                GC_OSErrorHandler );
    s_OldEH_Alignment          = OSSetErrorHandler( OS_ERROR_ALIGNMENT,          GC_OSErrorHandler );
    s_OldEH_PerformanceMonitor = OSSetErrorHandler( OS_ERROR_PERFORMACE_MONITOR, GC_OSErrorHandler );
    s_OldEH_SystemInterrupt    = OSSetErrorHandler( OS_ERROR_SYSTEM_INTERRUPT,   GC_OSErrorHandler );

#else
    GXSetVerifyLevel( GX_WARN_NONE );
#endif

}

//==========================================================================
