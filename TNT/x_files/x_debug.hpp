////////////////////////////////////////////////////////////////////////////
//
//  X_DEBUG.HPP
//
////////////////////////////////////////////////////////////////////////////

#ifndef X_DEBUG_HPP
#define X_DEBUG_HPP

////////////////////////////////////////////////////////////////////////////
//
//  This file provides basic debugging assistance in the following forms:
//   - Macro ASSERT and other similar runtime validations.
//   - Macro BREAK which is a "compiled breakpoint".
//   - Error message support.
//
//  Available options:
//
//   - When the symbol X_DEBUG is defined, the runtime validation macros are
//     active.  When X_DEBUG is not defined, these macros evaporate into
//     the appropriate runtime code causing no additional overhead.
//
//   - When the symbol DEBUG_LITE is defined in addition to X_DEBUG, the
//     various runtime validation macros take on a form that performs the
//     validation, but sacrifices feedback (such as the file name and the
//     expression) for compiled code size.
//
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//  INCLUDES
////////////////////////////////////////////////////////////////////////////

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#endif

////////////////////////////////////////////////////////////////////////////
//  DEBUGGING FUNCTIONS AND MACROS
////////////////////////////////////////////////////////////////////////////
//
//  BREAK
//
//  ASSERT  ( exp )
//  ASSERTS ( exp, str )
//  VERIFY  ( exp )
//  VERIFYS ( exp, str )
//
//  Hint:  The function fs(), provided by x_stdio, can be used to pass 
//  runtime constructed strings into the macros.  For example:
//      pFile = x_fopen( pFileName, "r" );
//      ASSERTS( pFile != NULL, fs( "Failed to open %s", pFileName ) );
//
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
#if defined( TARGET_PC ) || defined( TARGET_XBOX )

#define BREAK           __asm int 3
#define DEBUG_HALT      DebugHalt()

inline xbool DebugHalt( void )
{
    BREAK;
    return( TRUE );
}

#endif
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
#if defined( TARGET_PS2 )

extern volatile s32 DDBZ;

#ifdef __MWERKS__
	#define BREAK			asm __volatile__ (" breakc 0x0 ");
#else
	#define BREAK           asm __volatile__ (" break ");//(DDBZ=0,DDBZ=1/DDBZ)
#endif
#define DEBUG_HALT		DebugHalt()

inline xbool DebugHalt( void )
{
	BREAK;
	return( TRUE );
}

#endif
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
#if defined( TARGET_DOLPHIN )

    #if defined( __SN__ )
        #ifdef X_DEBUG_HARD_BREAK
            #define BREAK       asm(".long 0x00000000")     //Hard break
        #else
            #define BREAK       asm(".long 0x00000001")     //Pause debugger
        #endif
    #else
        #define BREAK           OSHalt( "ASSERT" )
    #endif

#define DEBUG_HALT      DebugHalt()

inline xbool DebugHalt( void )
{
    BREAK;
    return( TRUE );
}

#endif
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// Enumeration of codes generated internally by each debugging macro.

enum
{
    XDB_ASSERT,
    XDB_ASSERTS,
    XDB_VERIFY,
    XDB_VERIFYS
};

////////////////////////////////////////////////////////////////////////////

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


#ifdef ASSERT_PTR
#undef ASSERT_PTR
#endif

#ifdef ASSERTS_PTR
#undef ASSERTS_PTR
#endif

////////////////////////////////////////////////////////////////////////////

#ifdef X_DEBUG

  #ifdef DEBUG_LITE
    #define ASSERT(exp)         ((void)( (exp) || (DebugVector( XDB_ASSERT,      NULL, __LINE__, NULL, NULL )) || DEBUG_HALT ))
    #define ASSERTS(exp,str)    ((void)( (exp) || (DebugVector( XDB_ASSERTS,     NULL, __LINE__, NULL, NULL )) || DEBUG_HALT ))
    #define VERIFY(exp)         ((void)( (exp) || (DebugVector( XDB_VERIFY,      NULL, __LINE__, NULL, NULL )) || DEBUG_HALT ))
    #define VERIFYS(exp,str)    ((void)( (exp) || (DebugVector( XDB_VERIFYS,     NULL, __LINE__, NULL, NULL )) || DEBUG_HALT ))

    #define ASSERT_PTR(ptr)      ((void)( (x_IsValidPtr(ptr)) || (DebugVector( XDB_ASSERT,      NULL, __LINE__, NULL, NULL )) || DEBUG_HALT ))
    #define ASSERTS_PTR(ptr,str) ((void)( (x_IsValidPtr(ptr)) || (DebugVector( XDB_ASSERTS,     NULL, __LINE__, NULL, NULL )) || DEBUG_HALT ))
  #else
    #define ASSERT(exp)         ((void)( (exp) || (DebugVector( XDB_ASSERT,  __FILE__, __LINE__, #exp, NULL )) || DEBUG_HALT ))
    #define ASSERTS(exp,str)    ((void)( (exp) || (DebugVector( XDB_ASSERTS, __FILE__, __LINE__, #exp, str  )) || DEBUG_HALT ))
    #define VERIFY(exp)         ((void)( (exp) || (DebugVector( XDB_VERIFY,  __FILE__, __LINE__, #exp, NULL )) || DEBUG_HALT ))
    #define VERIFYS(exp,str)    ((void)( (exp) || (DebugVector( XDB_VERIFYS, __FILE__, __LINE__, #exp, str  )) || DEBUG_HALT ))

    #define ASSERT_PTR(ptr)      ((void)( (x_IsValidPtr(ptr)) || (DebugVector( XDB_ASSERT,  __FILE__, __LINE__, #ptr, NULL )) || DEBUG_HALT ))
    #define ASSERTS_PTR(ptr,str) ((void)( (x_IsValidPtr(ptr)) || (DebugVector( XDB_ASSERTS, __FILE__, __LINE__, #ptr, str  )) || DEBUG_HALT ))
  #endif

#else

    #define ASSERT(exp)         ((void)  0  )
    #define ASSERTS(exp,str)    ((void)  0  )
    #define VERIFY(exp)         ((void)(exp))
    #define VERIFYS(exp,str)    ((void)(exp))

    #define ASSERT_PTR(exp)      ((void)  0  )
    #define ASSERTS_PTR(exp,str) ((void)  0  )

#endif

////////////////////////////////////////////////////////////////////////////
//  DEBUG VECTOR FUNCTION
////////////////////////////////////////////////////////////////////////////

// Define a "declaration of function" for our standard debug vector function
// signature.  Then define a "pointer to" this type of function.

typedef xbool (debug_fndec)( s32   Type,
                             char* pFile,
                             s32   Line,
                             char* pExpStr,
                             char* pMsgStr );

typedef debug_fndec* debug_fnptr;

// Declare an extern pointer to the DebugVector function.
// This pointer is instantiated in x_debug.cpp.

extern debug_fnptr DebugVector;
extern debug_fnptr DebugHandler;

////////////////////////////////////////////////////////////////////////////

debug_fnptr x_DebugSetVector ( debug_fnptr NewVector );
debug_fnptr x_DebugSetHandler( debug_fnptr NewHandler );
void        x_DebugSuppress  ( char* pFile, s32 Line );

xbool       x_IsValidPtr     ( const void* UserPtr );

#endif
