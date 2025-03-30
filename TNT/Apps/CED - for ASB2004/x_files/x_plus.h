///////////////////////////////////////////////////////////////////////////
//
//  X_PLUS.H
//
///////////////////////////////////////////////////////////////////////////

#ifndef X_PLUS_H
#define X_PLUS_H

///////////////////////////////////////////////////////////////////////////
// INCLUDE
///////////////////////////////////////////////////////////////////////////

#ifndef X_TYPES_H
#include "x_types.h"
#endif

///////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////
//  HANDY MACROS
///////////////////////////////////////////////////////////////////////////

#define ALIGN_64(n)     ( (((u32)(n)) + 63) & (-64) )
#define ALIGN_32(n)     ( (((u32)(n)) + 31) & (-32) )
#define ALIGN_16(n)     ( (((u32)(n)) + 15) & (-16) )
#define ALIGN_8(n)      ( (((u32)(n)) +  7) & ( -8) )
#define ALIGN_4(n)      ( (((u32)(n)) +  3) & ( -4) )

#define ENDIAN_SWAP_16(A) \
    ( (((u16)A) >> 8) |   \
      (((u16)A) << 8) )

#define ENDIAN_SWAP_32(A) \
    ( (((u32)A) >> 24) |  \
      (((u32)A) << 24) |  \
      ((((u32)A) & 0x00FF0000) >> 8) | \
      ((((u32)A) & 0x0000FF00) << 8) )

///////////////////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////////////////

#define X_MAX_PATH   260
#define X_MAX_DRIVE    3
#define X_MAX_DIR    256
#define X_MAX_FNAME  256
#define X_MAX_EXT    256

///////////////////////////////////////////////////////////////////////////

#define X_RAND_MAX   0x7FFF 

///////////////////////////////////////////////////////////////////////////
// TYPES
///////////////////////////////////////////////////////////////////////////

typedef s32 (*compare_fnptr)( const void* Element1, const void* Element2 );

///////////////////////////////////////////////////////////////////////////
// SORT/SEARCH
///////////////////////////////////////////////////////////////////////////

void x_qsort   ( void*          Base, 
                 s32            Num, 
                 s32            Size, 
                 compare_fnptr  Compare );

xbool x_bsearch( s32*           Index,              // return index of the item found
                 const void*    Base,               // First item in the array
                 s32            NItems,             // number of items in the array
                 const void*    DesiredItem,        // reference item that we are looking for
                 s32            ItemSize,           // size of the items in the array
                 compare_fnptr  Compare );          // function use to comprare referece with item

///////////////////////////////////////////////////////////////////////////
// RANDOM
///////////////////////////////////////////////////////////////////////////

void    x_srand     ( s32 Seed );
s32     x_rand      ( void );               // [0 - X_RAND_MAX]
s32     x_rand_int  ( s32 min, s32 max );   // [min - max]
f32     x_rand_float( f32 min, f32 max );   // [min - max]

///////////////////////////////////////////////////////////////////////////
// STRING
///////////////////////////////////////////////////////////////////////////

s32     x_strlen    ( const char* Str );
char*   x_strcpy    ( char* Dst,   const char* Src );
char*   x_strncpy   ( char* Dest,  const char* Src, s32 Count );
char*   x_strcat    ( char* Dst,   const char* Src );
char*   x_strncat   ( char* Front, const char* Back, s32 Count );
s32     x_strcmp    ( const char* Src0,  const char* Src1 );
s32     x_strncmp   ( const char* Src0,  const char* Src1, s32 Count );
s32     x_stricmp   ( const char* Src0,  const char* Src1 );
char*   x_strrchr   ( const char* str, s32 c );
char*   x_strtoupper( char* Src );
char*   x_strtolower( char* Src );
char*   x_strstr    ( const char* MainStr, const char* SubStr );

///////////////////////////////////////////////////////////////////////////
// ASCII
///////////////////////////////////////////////////////////////////////////

s32     x_toupper   ( s32 C );
s32     x_tolower   ( s32 C );
s32     x_atoi      ( const char* Str );
f64     x_atof      ( const char* Str );

///////////////////////////////////////////////////////////////////////////
// BLOCK MEMORY OPERATIONS
///////////////////////////////////////////////////////////////////////////

void*   x_memcpy    ( void* Dest, const void* Src, s32 Count );
void*   x_memset    ( void* Dest, byte C, s32 Count );
void*   x_memchr    ( void* Buf,  s32 Chr, s32 Count );
void*   x_memmove   ( void* Dest, const void* Src, s32 Count );
s32     x_memcmp    ( const void* Buf1, const void* Buf2, s32 Count );
u32     x_chksum    ( const void* Buf, s32 Count, u32 CRC );

///////////////////////////////////////////////////////////////////////////
// PATH MANIPULATION
// Pass NULL for path components you are not interested in.
///////////////////////////////////////////////////////////////////////////

void x_splitpath( const char* Path, char* Drive,
                                    char* Dir,
                                    char* FName,
                                    char* Ext );

void x_makepath( char* path,  const char* drive,
                              const char* dir,
                              const char* fname,
                              const char* ext );

///////////////////////////////////////////////////////////////////////////
// MUTEX
//
// DECLARE_MUTEX(I)    Declares mutex info named I
// INIT_MUTEX(I)       Init mutex info before first use
// KILL_MUTEX(I)       Destroy mutex after last use
// ENTER_MUTEX(I)      Mark beginning of mutex section
// EXIT_MUTEX(I)       Mark end of mutex section
//
///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_N64
    typedef struct
    {
        OSMesgQueue		MsgQ;
        OSMesg			Msg[1];
    } mutex_info;

    #define DECLARE_MUTEX(I)    mutex_info Mutex_ ## I;
    #define INIT_MUTEX(I)       InitMutexInfo( &(Mutex_ ## I) );
    #define KILL_MUTEX(I)       
    #define ENTER_MUTEX(I)      EnterMutexSection( &(Mutex_ ## I) );
    #define EXIT_MUTEX(I)       ExitMutexSection( &(Mutex_ ## I) );

    void InitMutexInfo      ( mutex_info* MI );
    void EnterMutexSection  ( mutex_info* MI );
    void ExitMutexSection   ( mutex_info* MI );
#elif defined TARGET_PSX
    #include "libapi.h"
    #include "r3000.h"
    #define DECLARE_MUTEX(I)
    #define INIT_MUTEX(I)
    #define KILL_MUTEX(I)       
    #define ENTER_MUTEX(I)      { u32 SR = GetSr();  if (!(SR & SR_IEP)) EnterCriticalSection(); }
    #define EXIT_MUTEX(I)       { u32 SR = GetSr();  if (!(SR & SR_IEP)) ExitCriticalSection();  }
#else
    #define DECLARE_MUTEX(I)
    #define INIT_MUTEX(I)
    #define KILL_MUTEX(I)       
    #define ENTER_MUTEX(I)
    #define EXIT_MUTEX(I)
#endif

///////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

///////////////////////////////////////////////////////////////////////////

#endif
