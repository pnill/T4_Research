///////////////////////////////////////////////////////////////////////////
//
//  X_TYPES.H
//
///////////////////////////////////////////////////////////////////////////

#ifndef X_TYPES_H
#define X_TYPES_H

///////////////////////////////////////////////////////////////////////////
//  INCLUDES
///////////////////////////////////////////////////////////////////////////

#ifndef X_TARGET_H
#include "x_target.h"
#endif

#ifdef TARGET_N64
    #define	F3DEX_GBI_2
    #include <ultra64.h>
#endif

#ifdef TARGET_PSX
    #include <stddef.h>
    #include <libgte.h>
#endif

///////////////////////////////////////////////////////////////////////////
//  DEFINITIONS
///////////////////////////////////////////////////////////////////////////

#ifndef FALSE
    #define FALSE           (0)
#endif

#ifndef TRUE
    #define TRUE            (1)
#endif

#ifndef NULL
  #ifdef __cplusplus
    #define NULL            0
  #else
    #define NULL            ((void*)0)
  #endif
#endif

#define ERR_FAILURE         ((s32)1)
#define ERR_SUCCESS         ((s32)0)   

///////////////////////////////////////////////////////////////////////////
//  PREPROCESSOR KEY WORDS
///////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#define x_inline __inline
#endif

#ifdef __GNUC__
#define x_inline __inline__
#endif

///////////////////////////////////////////////////////////////////////////
//  TYPEDEFS
///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_SDC

    typedef unsigned char       u8;
    typedef unsigned short      u16;
    typedef unsigned long       u32;
    typedef unsigned __int64    u64;
    typedef signed   char       s8;
    typedef signed   short      s16;
    typedef signed   long       s32;
    typedef signed   __int64    s64;
    typedef float               f32;
    typedef double              f64;

#endif

///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_N64

//  typedef unsigned char       u8;
//  typedef unsigned short      u16;
//  typedef unsigned long       u32;
//                              u64;
//  typedef signed   char       s8;
//  typedef signed   short      s16;
//  typedef signed   long       s32;
//                              s64;
//  typedef float               f32;
//  typedef double              f64;
    
#endif

///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_PC

    typedef unsigned char       u8;
    typedef unsigned short      u16;
    typedef unsigned long       u32;
    typedef unsigned __int64    u64;
    typedef signed   char       s8;
    typedef signed   short      s16;
    typedef signed   long       s32;
    typedef signed   __int64    s64;
    typedef float               f32;
    typedef double              f64;

#endif

///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_PSX

    typedef unsigned char       u8;
    typedef unsigned short      u16;
    typedef unsigned long       u32;
    typedef signed   char       s8;
    typedef signed   short      s16;
    typedef signed   long       s32;
    typedef float               f32;
    typedef double              f64;
#ifndef _MSC_VER
    typedef unsigned long long  u64;
    typedef signed   long long  s64;
#else
    typedef unsigned __int64    u64;
    typedef signed   __int64    s64;
#endif

#endif

///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_NUON

    typedef unsigned char       u8;
    typedef unsigned short      u16;
    typedef unsigned long       u32;
//  typedef unsigned ???????    u64;
    typedef signed   char       s8;
    typedef signed   short      s16;
    typedef signed   long       s32;
//  typedef signed   ????????   s64;
    typedef float               f32;
    typedef double              f64;

#endif

///////////////////////////////////////////////////////////////////////////

typedef u8                  byte;
typedef s32                 err;
typedef u32                 color;
typedef s32                 fixed;
typedef s32                 xbool;

///////////////////////////////////////////////////////////////////////////
// SIMPLE MACROS EVERYONE EXPECTS TO BE PRESENT
///////////////////////////////////////////////////////////////////////////

#ifndef ABS
    #define ABS(x)          ( ((x) < 0) ? (-(x)) : (x) )
#endif

#ifndef MAX
    #define MAX(a,b)        ((a)>(b)?(a):(b))
#endif

#ifndef MIN
    #define MIN(a,b)        ((a)<(b)?(a):(b))
#endif

#ifndef IN_RANGE
    #define IN_RANGE(a,v,b)     ( ((a) <= (v)) && ((v) <= (b)) )
#endif

///////////////////////////////////////////////////////////////////////////
// CODE LOCALITY SPECIFIERS
///////////////////////////////////////////////////////////////////////////
// For the N64 with the GNU compiler:
//-------------------------------------------------------------------------
// void SeldomUsedFunction( void )   X_SECTION(infrequent);
// For this example, the link must define a section named ".infrequent".
///////////////////////////////////////////////////////////////////////////
// For the PC:
//-------------------------------------------------------------------------
// No support yet.
///////////////////////////////////////////////////////////////////////////

#if defined(TARGET_N64) && defined(__GNUC__)
    #define X_SECTION(name)   __attribute__((section("."#name)))
#else
    #define X_SECTION(name)
#endif

///////////////////////////////////////////////////////////////////////////
// FINISHED
///////////////////////////////////////////////////////////////////////////

#endif
