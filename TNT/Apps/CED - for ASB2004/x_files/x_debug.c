///////////////////////////////////////////////////////////////////////////
//
//  X_DEBUG.C
//
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_SDC_D3D
#include "windows.h"
#endif

#ifndef X_DEBUG_H
#include "x_debug.h"
#endif

#ifndef X_TYPES_H
#include "x_types.h"
#endif

#ifndef X_STDIO_H
#include "x_stdio.h"
#endif

#ifndef X_PLUS_H
#include "x_plus.h"
#endif

///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_PC
  #include <stdio.h>
  #include <stdarg.h>
  #include <assert.h>

  #ifdef X_DEBUG
    #include <windows.h>
    #define DEBUGGER_MESSAGE(str)       OutputDebugString(str)
  #endif
#endif

#ifdef TARGET_SDC
  #ifdef X_DEBUG
	#define DEBUGGER_MESSAGE(str)    NKDbgPrintfW( StringToUnicode(str) )	
  #endif
#endif

#ifndef DEBUGGER_MESSAGE
  #define DEBUGGER_MESSAGE(str)
#endif

///////////////////////////////////////////////////////////////////////////
//  GLOBALS
///////////////////////////////////////////////////////////////////////////

debug_fndec BaseAbortVector;
debug_fnptr DebugAbortVector = BaseAbortVector;

// This is needed to cause a "divide by zero" error for debugging.
// Do not use "ifndef X_DEBUG" to remove this variable.  (Why?  Because
// other source files may be under X_DEBUG even if this file is not.)
s32 DebugDivideByZero = 0;

static char LastErrorMsg[512];

///////////////////////////////////////////////////////////////////////////
//  ERROR MESSAGE FUNCTIONS
///////////////////////////////////////////////////////////////////////////

const char* x_GetErrorMsg( void )
{
    return( LastErrorMsg );
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// DREAM CAST
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
#ifdef TARGET_SDC
#ifdef X_DEBUG
//====================================================================
// StringToUnicode
//====================================================================
static
s16* StringToUnicode( char* String )
{
	s32			i;
	static s16	String16[512];

    //----------------------------------------------------------------
	// convert string to s16
    //----------------------------------------------------------------
	for (i=0;String16[i]=String[i];i++);

	return String16;
}

#endif
#endif

///////////////////////////////////////////////////////////////////////////
// END OF TARGET_SDC
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////

err x_SetErrorMsg( const char* FileName, s32 Line, const char* FormatStr, ... )
{
    if( FormatStr )
    {
        s32        StrLen; 
        x_va_list  Args;

        x_va_start( Args, FormatStr );

        if( FileName )
            StrLen = x_sprintf( LastErrorMsg, "[ %s : %d ]\n", FileName, Line );
        else
            StrLen = x_sprintf( LastErrorMsg, "[ %s : %d ]\n", "UNKNOWN", Line );

        StrLen += x_vsprintf( &LastErrorMsg[StrLen], FormatStr, Args );

        ASSERT( StrLen <= 512 );
        DEBUGGER_MESSAGE( fs("***QUAGMIRE*** %s\n", LastErrorMsg) );
    }

    return( ERR_FAILURE );
}

///////////////////////////////////////////////////////////////////////////
//  ABORT VECTOR FUNCTIONS
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
#ifdef TARGET_PC
///////////////////////////////////////////////////////////////////////////

// If compiled with _NDEBUG, then the Microsoft function _assert() will not
// be defined.  So we, must provide a placebo.
#ifdef NDEBUG
#define _assert(e,f,l)
#endif


void BaseAbortVector( s32 Type, char* File, s32 Line, char* ExpStr, char* MsgStr )
{
    #define ABORT_BUFFER_SIZE 512
    char Buffer[ABORT_BUFFER_SIZE];

    // Assume we don't need to cause a dived by zero to stop.
    DebugDivideByZero = 1;

    if( (Type == XDB_ASSERTS) || (Type == XDB_VERIFYS) )
    {
        // Need to get both the ExpStr and MsgStr into one string.
        Buffer[0] = Buffer[ABORT_BUFFER_SIZE-1] = '\0';
        if( ExpStr )
            x_strcat( Buffer, ExpStr );
        if( ExpStr && MsgStr )
            x_strcat( Buffer, "\n" );
        if( MsgStr )
            x_strcat( Buffer, MsgStr );
        if( Buffer[ABORT_BUFFER_SIZE-1] )
            DebugDivideByZero = 1;
    }

    if( Type == XDB_ABORT )
    {
        // Want to prefix MsgStr with "ABORT".
        Buffer[ABORT_BUFFER_SIZE-1] = '\0';

        if (MsgStr) x_sprintf( Buffer, "ABORT - %s", MsgStr );
        else x_sprintf( Buffer, "ABORT" );

        if( Buffer[ABORT_BUFFER_SIZE-1] )
            DebugDivideByZero = 1;
    }

    switch( Type )
    {
        // Microsoft has:   _assert( #exp, __FILE__, __LINE__ )
        
        case XDB_VERIFY:    // fall through
        case XDB_ASSERT:    if (MsgStr) DEBUGGER_MESSAGE( MsgStr );      
                            _assert( ExpStr, File, Line );   
                            break;

        case XDB_ASSERTS:   // fall through
        case XDB_VERIFYS:   // fall through
        case XDB_ABORT:     DEBUGGER_MESSAGE( Buffer );      
                            _assert( Buffer, File, Line );   
                            break;

        case XDB_TRACE:     DEBUGGER_MESSAGE( MsgStr );      
                            break;

        case XDB_SIGNAL:    _assert( "SIGNAL", File, Line ); 
                            break;

        default:            _assert( "Unknown Abort Vector Code", File, Line ); 
                            break;
    }
}

///////////////////////////////////////////////////////////////////////////
#endif  // PC IMPLEMENTATION
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
#ifdef TARGET_N64
///////////////////////////////////////////////////////////////////////////

// Abort Vectors on the N64 require getting a bit more intimate with the
// machine than the x_files normally do.  So all N64 Abort Vector functions 
// are in N64Debug.c.

///////////////////////////////////////////////////////////////////////////
#endif  // N64 IMPLEMENTATION
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
// FINISHED
///////////////////////////////////////////////////////////////////////////
