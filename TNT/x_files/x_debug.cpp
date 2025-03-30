////////////////////////////////////////////////////////////////////////////
//
//  X_DEBUG.CPP
//
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_types.hpp"
#include "x_debug.hpp"
#include "x_plus.hpp"
#include "x_stdio.hpp"


///////////////////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////////////////

#if defined( TARGET_PC ) || defined( TARGET_XBOX )
      //#include <stdio.h>
      //#include <stdarg.h>
      //#include <assert.h>
    #if defined( TARGET_XBOX )
      #include <xtl.h>
    #endif

    #if defined( TARGET_PC )
      #include <windows.h>
    #endif

    #ifdef X_DEBUG
      #define DEBUGGER_MESSAGE(str)         OutputDebugString(str)
    #endif

#elif defined( TARGET_PS2 )
    #ifdef X_DEBUG
        #define DEBUGGER_MESSAGE(str)       x_printf(str)
    #endif

#elif defined( TARGET_DOLPHIN )
    #ifdef X_DEBUG
        #define DEBUGGER_MESSAGE(str)       x_printf(str)
    #endif

#endif

#ifndef DEBUGGER_MESSAGE
    #define DEBUGGER_MESSAGE(str)   ((void)0)
#endif


#define VERIFY_STRING   "\n*** VERIFICATION FAILURE ***\n"
#define ASSERT_STRING   "\n*** ASSERTION FAILURE ***\n"
#define UNKNOWN_STRING  "\n*** UNKNOWN FAILURE ***\n"

#define MAX_SUPPRESSED_ASSERTS  20


////////////////////////////////////////////////////////////////////////////
// STORAGE
////////////////////////////////////////////////////////////////////////////

//--- initial debug vector function (prints debug message)
debug_fndec BaseDebugVector;
debug_fnptr DebugVector = BaseDebugVector;

//--- debug handler function called to perform application specific processing
debug_fnptr DebugHandler = NULL;

//--- suppressed assert data
static s32   s_NumSuppressed = 0;
static char* s_SuppressedFiles[MAX_SUPPRESSED_ASSERTS];
static s32   s_SuppressedLines[MAX_SUPPRESSED_ASSERTS];

volatile s32  DDBZ = 1;


///////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

debug_fnptr x_DebugSetVector( debug_fnptr NewVector )
{
    debug_fnptr Old = DebugVector;
    DebugVector = NewVector;
    return Old;
}

///////////////////////////////////////////////////////////////////////////////

debug_fnptr x_DebugSetHandler( debug_fnptr NewHandler )
{
    debug_fnptr Old = DebugHandler;
    DebugHandler = NewHandler;
    return Old;
}

///////////////////////////////////////////////////////////////////////////////

void x_DebugSuppress( char* pFile, s32 Line )
{
#ifdef X_DEBUG

    if( s_NumSuppressed >= MAX_SUPPRESSED_ASSERTS )
        return;

    s_SuppressedFiles[s_NumSuppressed] = pFile;
    s_SuppressedLines[s_NumSuppressed] = Line;
    s_NumSuppressed++;

#endif
}

///////////////////////////////////////////////////////////////////////////////

xbool BaseDebugVector( s32   Type, 
                       char* pFile, 
                       s32   Line, 
                       char* pExpStr, 
                       char* pMsgStr )
{
    xbool   Proceed = FALSE;
    s32     i;
    char    Location[256];

    ///////////////////////////////////////////////////////////////////////////
    //  Check for suppressed debug items
    ///////////////////////////////////////////////////////////////////////////

    //--- look for this assert in the array of surpressed asserts
    for( i = 0; i < s_NumSuppressed; i++ )
    {
        //--- if this is the file, break now
        if( (s_SuppressedLines[i] == Line) && (x_strcmp( s_SuppressedFiles[i], pFile ) == 0) )
            break;
    }

    //--- if this assert was found, skip it now
    if( i != s_NumSuppressed )
        return TRUE;


    ///////////////////////////////////////////////////////////////////////////
    //  Display the debug information
    ///////////////////////////////////////////////////////////////////////////

    //--- display the title
    switch( Type )
    {
        case XDB_VERIFY:    DEBUGGER_MESSAGE( VERIFY_STRING );      break;
        case XDB_VERIFYS:   DEBUGGER_MESSAGE( VERIFY_STRING );      break;
        case XDB_ASSERT:    DEBUGGER_MESSAGE( ASSERT_STRING );      break;
        case XDB_ASSERTS:   DEBUGGER_MESSAGE( ASSERT_STRING );      break;
        default:            DEBUGGER_MESSAGE( UNKNOWN_STRING );     break;
    }

    //--- display the location of the debug check
    x_sprintf( Location, "File: %s\nLine: %ld\n", pFile, Line );
    DEBUGGER_MESSAGE( Location );

    //--- display the expression (if there is one)
    if( pExpStr )
    {
        DEBUGGER_MESSAGE( pExpStr );

        if( pMsgStr )
            DEBUGGER_MESSAGE( "\n" );
    }

    //--- display the message (if there is one)
    if( pMsgStr )
        DEBUGGER_MESSAGE( pMsgStr );

    //--- new line
    DEBUGGER_MESSAGE( "\n" );


    ///////////////////////////////////////////////////////////////////////////
    //  Call the debug handler for any platform specific debug processing
    ///////////////////////////////////////////////////////////////////////////

    //--- if there is a debug handler, let it determine what to do
    if( DebugHandler )
        Proceed = DebugHandler( Type, pFile, Line, pExpStr, pMsgStr );

    return( Proceed );
}

////////////////////////////////////////////////////////////////////////////

xbool x_IsValidPtr( const void* UserPtr )
{
	if( UserPtr == NULL )
		return FALSE;
	
	// for each platform, perform bounds checks to see if ptr is within valid
	// memory range.  Could also check for bad ptr values, like 0xbebebebe or 0xcccccccc.

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
