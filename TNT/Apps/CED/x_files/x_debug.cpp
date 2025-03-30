////////////////////////////////////////////////////////////////////////////
//
//  X_DEBUG.CPP
//
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//  INCLUDES
////////////////////////////////////////////////////////////////////////////

#ifndef X_DEBUG_HPP
#include "x_debug.hpp"
#endif

#ifndef X_PLUS_HPP
#include "x_plus.hpp"
#endif

#ifndef X_STDIO_HPP
#include "x_stdio.hpp"
#endif


///////////////////////////////////////////////////////////////////////////
//  DEFINES
///////////////////////////////////////////////////////////////////////////

#if defined(TARGET_PC) || defined(TARGET_XBOX)
//  #include <stdio.h>
//  #include <stdarg.h>
//  #include <assert.h>
    #include <windows.h>

  #ifdef X_DEBUG
    #define DEBUGGER_MESSAGE(str)       OutputDebugString(str)
  #endif
#endif

#ifdef TARGET_PS2
  #ifdef X_DEBUG
    #define	DEBUGGER_MESSAGE(str)		x_printf(str);
  #endif
#endif

#ifndef DEBUGGER_MESSAGE
#define DEBUGGER_MESSAGE(str)   ((void)0)
#endif

#define	VERIFY_STRING	"\n*** VERIFICATION FAILURE ***\n"
#define	ASSERT_STRING	"\n*** ASSERTION FAILURE ***\n"
#define	UNKNOWN_STRING	"\n*** UNKNOWN FAILURE ***\n"

#define	MAX_SUPPRESSED_ASSERTS	20

////////////////////////////////////////////////////////////////////////////
//  STORAGE
////////////////////////////////////////////////////////////////////////////

//---	initial debug vector function (prints debug message)
debug_fndec BaseDebugVector;
debug_fnptr DebugVector = BaseDebugVector;

//---	debug handler function called to perform application specific processing
debug_fnptr DebugHandler = NULL;

//---	suppressed assert data
static s32 NumSuppressed = 0;
static char* SuppressedFiles[MAX_SUPPRESSED_ASSERTS];
static s32 SuppressedLines[MAX_SUPPRESSED_ASSERTS];

////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
////////////////////////////////////////////////////////////////////////////

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

	if( NumSuppressed >= MAX_SUPPRESSED_ASSERTS )
		return;

	SuppressedFiles[NumSuppressed] = pFile;
	SuppressedLines[NumSuppressed] = Line;
	NumSuppressed++;

#endif
}

///////////////////////////////////////////////////////////////////////////////
xbool BaseDebugVector( s32   Type, 
                       char* pFile, 
                       s32   Line, 
                       char* pExpStr, 
                       char* pMsgStr )
{
    xbool	Proceed = FALSE;
	s32		i;
	char	Location[256];

	///////////////////////////////////////////////////////////////////////////
	//	Check for suppressed debug items
	///////////////////////////////////////////////////////////////////////////

	//---	look for this assert in the array of surpressed asserts
	for( i=0; i<NumSuppressed; i++ )
	{
		//---	if this is the file, break now
		if(( SuppressedLines[i] == Line ) && ( x_strcmp( SuppressedFiles[i], pFile ) == 0 ))
			break;
	}

	//---	if this assert was found, skip it now
	if( i!=NumSuppressed )
		return TRUE;


	///////////////////////////////////////////////////////////////////////////
	//	Display the debug information
	///////////////////////////////////////////////////////////////////////////

	//---	display the title
    switch( Type )
	{
        case XDB_VERIFY:	DEBUGGER_MESSAGE( VERIFY_STRING );		break;
        case XDB_VERIFYS:	DEBUGGER_MESSAGE( VERIFY_STRING );		break;
		case XDB_ASSERT:	DEBUGGER_MESSAGE( ASSERT_STRING );		break;
        case XDB_ASSERTS:	DEBUGGER_MESSAGE( ASSERT_STRING );		break;
        default:			DEBUGGER_MESSAGE( UNKNOWN_STRING );		break;
	}

	//---	display the location of the debug check
	x_sprintf( Location, "File: %s\nLine: %d\n", pFile, Line );
	DEBUGGER_MESSAGE( Location );

	//---	display the expression (if there is one)
	if( pExpStr )
	{
		DEBUGGER_MESSAGE( pExpStr );

		if( pMsgStr )
			DEBUGGER_MESSAGE( "\n" );
	}

	//---	display the message (if there is one)
	if( pMsgStr )
		DEBUGGER_MESSAGE( pMsgStr );

	//---	new line
	DEBUGGER_MESSAGE( "\n" );


	///////////////////////////////////////////////////////////////////////////
	//	Call the debug handler for any platform specific debug processing
	///////////////////////////////////////////////////////////////////////////

	//---	if there is a debug handler, let it determine what to do
	if( DebugHandler )
		Proceed = DebugHandler( Type, pFile, Line, pExpStr, pMsgStr );


    return( Proceed );
}
//
//#endif
//
////==========================================================================
//
//#ifdef TARGET_PS2
//
volatile s32  DDBZ = 1;
//
//xbool BaseDebugVector( s32   Type, 
//                       char* pFile, 
//                       s32   Line, 
//                       char* pExpStr, 
//                       char* pMsgStr )
//{
//    switch( Type )
//    {
//        case XDB_ASSERT:    // fall through
//        case XDB_ASSERTS:   
//            x_printf( ASSERT_STRING );
//            break;
//            
//        case XDB_VERIFY:    // fall through
//        case XDB_VERIFYS:   
//            x_printf( VERIFY_STRING);
//            break;
//            
//        default:
//            x_printf( UNKNOWN_STRING);
//            break;
//    }
//
//    if( pFile )     x_printf( "*** File: %s on line %d\n", pFile, Line );
//    else            x_printf( "*** File: <unknown> on line %d\n", Line );
//    if( pExpStr )   x_printf( "*** Expr: %s\n", pExpStr );
//    if( pMsgStr )   x_printf( "*** Msg : %s\n", pMsgStr );
//    
//    x_printf( "***\n" );
//    return( FALSE );
//}
//
//#endif

//==========================================================================
