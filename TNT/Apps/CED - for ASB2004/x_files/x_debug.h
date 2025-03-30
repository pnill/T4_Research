///////////////////////////////////////////////////////////////////////////
//
//  X_DEBUG.H
//
///////////////////////////////////////////////////////////////////////////

#ifndef X_DEBUG_H
#define X_DEBUG_H

///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////

#ifndef X_TYPES_H
#include "x_types.h"
#endif

#ifndef X_TARGET_H
#include "x_target.h"
#endif

///////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////
// ERROR MESSAGE MANAGEMENT
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// DEFINES

#define SET_ERROR_MSG(msg)      x_SetErrorMsg( __FILE__, __LINE__, (msg) ) 

#define RETURN_FAILURE(msg)   { SET_ERROR_MSG(msg); return( ERR_FAILURE ); }
#define RETURN_ERROR(msg,val) { SET_ERROR_MSG(msg); return( val );         }
#define GOTO_ERROR(msg)       { SET_ERROR_MSG(msg); goto ERROR_HANDLER;    }

///////////////////////////////////////////////////////////////////////////
// FUNCTIONS

const char* x_GetErrorMsg  ( void );
err         x_SetErrorMsg  ( const char* FileName, s32 Line, const char* FormatStr, ... );

///////////////////////////////////////////////////////////////////////////
//  DEBUGGING MACROS
///////////////////////////////////////////////////////////////////////////
//
//  ASSERT  ( exp )
//  ASSERTS ( exp, str )
//  VERIFY  ( exp )
//  VERIFYS ( exp, str )
//  ABORT   ( str )
//  TRACE   ( str )
//
//  HINT -- The function  "char* fs( formatstr, ... )"  provided in
//  x_stdio can be used to pass runtime constructed strings into the
//  macros.  For example:
//      ASSERTS( ThingID < MAX_THINGS, fs( "Thing: %d", ThingID ) );
//
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// List of reasons why the application aborted.

enum
{
    XDB_ASSERT,
    XDB_ASSERTS,
    XDB_VERIFY,
    XDB_VERIFYS,
    XDB_ABORT,
    XDB_TRACE,
    XDB_SIGNAL,
};

///////////////////////////////////////////////////////////////////////////

#ifdef ASSERT
  #undef ASSERT
#endif

#ifdef ASSERTS
  #undef ASSERTS
#endif

#ifdef VERIFY
  #undef VERIFY
#endif

#ifdef VERIFYS
  #undef VERIFYS
#endif

#ifdef ABORT
  #undef ABORT
#endif

#ifdef TRACE
  #undef TRACE
#endif

extern s32 DebugDivideByZero;

#ifdef X_DEBUG

  #ifdef ASSERT_LITE
    #define ASSERT(exp)         ((void)( (exp) || ((void)DebugAbortVector( XDB_ASSERT,      NULL, __LINE__, NULL, NULL ), DebugDivideByZero=1/DebugDivideByZero) ))
    #define ASSERTS(exp,str)    ((void)( (exp) || ((void)DebugAbortVector( XDB_ASSERTS,     NULL, __LINE__, NULL, NULL ), DebugDivideByZero=1/DebugDivideByZero) ))
    #define VERIFY(exp)         ((void)( (exp) || ((void)DebugAbortVector( XDB_VERIFY,      NULL, __LINE__, NULL, NULL ), DebugDivideByZero=1/DebugDivideByZero) ))
    #define VERIFYS(exp,str)    ((void)( (exp) || ((void)DebugAbortVector( XDB_VERIFYS,     NULL, __LINE__, NULL, NULL ), DebugDivideByZero=1/DebugDivideByZero) ))
    #define ABORT(str)                            ((void)DebugAbortVector( XDB_ABORT,       NULL, __LINE__, NULL, NULL ), DebugDivideByZero=1/DebugDivideByZero)  
    #define TRACE(str)                            ((void)DebugAbortVector( XDB_TRACE,       NULL, __LINE__, NULL, NULL ))
  #else
	#define ASSERT(exp)         ((void)( (exp) || ((void)DebugAbortVector( XDB_ASSERT,  __FILE__, __LINE__, #exp, NULL ), DebugDivideByZero=1/DebugDivideByZero) ))
	#define ASSERTS(exp,str)    ((void)( (exp) || ((void)DebugAbortVector( XDB_ASSERTS, __FILE__, __LINE__, #exp, str  ), DebugDivideByZero=1/DebugDivideByZero) ))
	#define VERIFY(exp)         ((void)( (exp) || ((void)DebugAbortVector( XDB_VERIFY,  __FILE__, __LINE__, #exp, NULL ), DebugDivideByZero=1/DebugDivideByZero) ))
	#define VERIFYS(exp,str)    ((void)( (exp) || ((void)DebugAbortVector( XDB_VERIFYS, __FILE__, __LINE__, #exp, str  ), DebugDivideByZero=1/DebugDivideByZero) ))
    #define ABORT(str)                            ((void)DebugAbortVector( XDB_ABORT,   __FILE__, __LINE__, NULL, str  ), DebugDivideByZero=1/DebugDivideByZero)  
	#define TRACE(str)                            ((void)DebugAbortVector( XDB_TRACE,   __FILE__, __LINE__, NULL, str  ))
  #endif

#else

	#define ASSERT(exp)         
	#define ASSERTS(exp,str)    
	#define VERIFY(exp)         ((void)(exp))
	#define VERIFYS(exp,str)    ((void)(exp))
    #define ABORT(str)          
	#define TRACE(str)          

#endif

#if defined( TARGET_N64 ) && !defined( _MSC_VER )
	#define BREAK   __asm__("break 0x402")
#endif

#ifdef TARGET_SDC
	#define BREAK	__asm("trapa #1") 
#endif

#ifdef TARGET_PC
	#define BREAK   {__asm int 3}
#endif

#ifndef BREAK
#define BREAK
#endif

///////////////////////////////////////////////////////////////////////////
//  DEBUG ABORT VECTOR
//
//  Each debug macro expands into a DebugAbortVector call at some point.
///////////////////////////////////////////////////////////////////////////

// Define a "declaration of function" for our standard debug abort
// function signature.  Then define a "pointer to" this type of function.

typedef void (debug_fndec)( s32 Type, char* File, s32 Line, char* ExpStr, char* MsgStr );
typedef debug_fndec* debug_fnptr;

// Declare an extern pointer to the DebugAbortVector function.
// This pointer is instantiated in x_debug.c.

extern debug_fnptr DebugAbortVector;

///////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

///////////////////////////////////////////////////////////////////////////

#endif
