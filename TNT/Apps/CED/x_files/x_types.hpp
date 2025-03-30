////////////////////////////////////////////////////////////////////////////
//
//  X_TYPES.HPP
//
////////////////////////////////////////////////////////////////////////////

#ifndef X_TYPES_HPP
#define X_TYPES_HPP

////////////////////////////////////////////////////////////////////////////
//  INCLUDES
////////////////////////////////////////////////////////////////////////////

#ifndef X_TARGET_HPP
#include "x_target.hpp"
#endif

//
// For each major platform, certain include files are essentially 
// ubiquitous.  Go ahead and take care of it here.
// 
#ifdef TARGET_PC
#endif

#ifdef TARGET_PS2
#endif

#ifdef TARGET_DOLPHIN
#endif

#ifdef TARGET_XBOX
#endif

////////////////////////////////////////////////////////////////////////////
//  DEFINES
////////////////////////////////////////////////////////////////////////////

#ifdef TRUE
#undef TRUE
#undef FALSE
#endif

#define FALSE           (0)
#define TRUE            (1)

#ifndef NULL
  #ifdef __cplusplus
    #define NULL            0
  #else
    #define NULL            ((void*)0)
  #endif
#endif

#define	ERR_SUCCESS	((s32)TRUE)
#define	ERR_FAILURE	((s32)FALSE)

//
// Simple macros that the x_files provide free of charge.
//

#ifndef ABS
    #define ABS(a)          ( (a) < 0 ? -(a) : (a) )
#endif

#ifndef MAX
    #define MAX(a,b)        ( (a)>(b) ? (a) : (b) )
#endif

#ifndef MIN
    #define MIN(a,b)        ( (a)<(b) ? (a) : (b) )
#endif

#ifndef IN_RANGE
    #define IN_RANGE(a,v,b)     ( ((a) <= (v)) && ((v) <= (b)) )
#endif

////////////////////////////////////////////////////////////////////////////
//  TYPES
////////////////////////////////////////////////////////////////////////////

#ifdef TARGET_PC

  // Types for PC Targets under Developer Studio.
//  #ifdef _MSC_VER
    typedef unsigned char       u8;
    typedef unsigned short      u16;
    typedef unsigned int        u32;
    typedef unsigned __int64    u64;
    typedef   signed char       s8;
    typedef   signed short      s16;
    typedef   signed int        s32;
    typedef   signed __int64    s64;
    typedef          float      f32;
    typedef          double     f64;
//  #endif
  
#endif

//--------------------------------------------------------------------------

#ifdef TARGET_PS2

  // Types for PS2 Targets under SN compiler.
  #ifdef __GNUC__
    typedef unsigned char       u8;
    typedef unsigned short      u16;
    typedef unsigned int        u32;
    typedef unsigned long       u64;
    typedef   signed char       s8;
    typedef   signed short      s16;
    typedef   signed int        s32;
    typedef   signed long		s64;
    typedef          float      f32;
    typedef          double     f64;
  #endif

  // Types for PS2 Targets under Developer Studio.  (iCL needs this.)
  #ifdef _MSC_VER
    typedef unsigned char       u8;
    typedef unsigned short      u16;
    typedef unsigned int        u32;
    typedef unsigned __int64    u64;
    typedef   signed char       s8;
    typedef   signed short      s16;
    typedef   signed int        s32;
    typedef   signed __int64    s64;
    typedef          float      f32;
    typedef          double     f64;
  #endif

#endif      


#ifdef TARGET_DOLPHIN
    typedef unsigned char       u8;
    typedef unsigned short      u16;
    typedef unsigned int        u32;
    typedef unsigned __int64    u64;
    typedef   signed char       s8;
    typedef   signed short      s16;
    typedef   signed int        s32;
    typedef   signed __int64    s64;
    typedef          float      f32;
    typedef          double     f64;
#endif


#ifdef TARGET_XBOX
    typedef unsigned char       u8;
    typedef unsigned short      u16;
    typedef unsigned int        u32;
    typedef unsigned __int64    u64;
    typedef   signed char       s8;
    typedef   signed short      s16;
    typedef   signed int        s32;
    typedef   signed __int64    s64;
    typedef          float      f32;
    typedef          double     f64;
#endif

////////////////////////////////////////////////////////////////////////////

typedef u8      byte;
typedef s32     xbool;
typedef	s32		err;

////////////////////////////////////////////////////////////////////////////
//
//  Global redefinition of operators new, new[], delete, and delete[].  The
//  new versions of these functions use the memory manager within the 
//  x_files.
//
//  Why is this done here?  Because we want new and delete to ALWAYS be 
//  redefined, and EVERY source file must include x_types.hpp.
//
////////////////////////////////////////////////////////////////////////////

template <class T> inline void* placement_new(T *pAddress ) 
    { return new (reinterpret_cast<x_PlacementNewDummy*>(pAddress)) T; }
template <class T> inline void* placement_new(void* pAddress, const T& val) 
    { return new (reinterpret_cast<x_PlacementNewDummy*>(pAddress)) T(val); }
template <class T> inline void placement_delete(T *pAddress ) 
    { pAddress->~T(); }

#define USE_NATIVE_NEW_AND_DELETE
#ifndef USE_NATIVE_NEW_AND_DELETE

class x_PlacementNewDummy ; // A dummy class to uniquely identify our "new with placement"

void* operator new   ( u32 Size, x_PlacementNewDummy * pAddress  );
void  operator delete( void * Address , x_PlacementNewDummy * );
                          
void* operator new       ( u32 Size, char* pFileName, s32 LineNumber );
void* operator new    [] ( u32 Size, char* pFileName, s32 LineNumber );
void  operator delete    ( void* pMemory );
void  operator delete [] ( void* pMemory );
void  operator delete    ( void* pMemory, char*, s32 );
void  operator delete [] ( void* pMemory, char*, s32 );

#ifdef X_DEBUG
    #define new new( __FILE__, __LINE__ )
#else
    #define new new( NULL, 0 )
#endif

#endif // USE_NATIVE_NEW_AND_DELETE

////////////////////////////////////////////////////////////////////////////

#endif
