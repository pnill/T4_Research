////////////////////////////////////////////////////////////////////////////
//
//  X_PLUS.HPP
//
////////////////////////////////////////////////////////////////////////////
//  
//  This file provides various groups of functionality pulled together from
//  various places.  Some parts mimic standard C library functions.  Other
//  parts are original.  No single part was large enough to warrant its own
//  file, so x_plus was created.
//
//  Groups of functionality include the following:
//
//    - Numeric value alignment.
//    - Endian swapping.
//    - Quick sort and binary search.
//    - Pseudo random number generation.
//    - NULL terminated string functions, such as x_strcpy.
//    - Memory block functions, such as x_memcpy.
//    - Standard ASCII conversions, such as x_toupper and x_atoi.
//    - Standard character classifiers, such as x_isspace.
//    - Path manipulation functions.
//    - Mutex (single entry critical section) functions.
//  
////////////////////////////////////////////////////////////////////////////

#ifndef X_PLUS_HPP
#define X_PLUS_HPP

////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#endif

////////////////////////////////////////////////////////////////////////////
//  MACROS
////////////////////////////////////////////////////////////////////////////
//  
//  The ALIGN_xx(n) macros result in the next number greater than n which is
//  a multiple of xx.  For example, ALIGN_32(58) is 64.
//  
//  The ENDIAN_SWAP_xx macros toggle the Endian of a 16 or 32 bit value.
//  
////////////////////////////////////////////////////////////////////////////

#define ALIGN_128(n)    ( (((s32)(n)) + 127) & (-128) )
#define ALIGN_64(n)     ( (((s32)(n)) +  63) & ( -64) )
#define ALIGN_32(n)     ( (((s32)(n)) +  31) & ( -32) )
#define ALIGN_16(n)     ( (((s32)(n)) +  15) & ( -16) )
#define ALIGN_8(n)      ( (((s32)(n)) +   7) & (  -8) )
#define ALIGN_4(n)      ( (((s32)(n)) +   3) & (  -4) )
#define ALIGN_2(n)      ( (((s32)(n)) +   1) & (  -2) )

#define ENDIAN_SWAP_16(A) \
    ( (((u16)A) >> 8) |   \
      (((u16)A) << 8) )

#define ENDIAN_SWAP_32(A) \
    ( (((u32)A) >> 24) |  \
      (((u32)A) << 24) |  \
      ((((u32)A) & 0x00FF0000) >> 8) | \
      ((((u32)A) & 0x0000FF00) << 8) )

////////////////////////////////////////////////////////////////////////////
//  TYPES
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//
//  A "standard compare function pointer" type is defined.  Functions which
//  satisfy this pattern take the addresses of two items (expressed as void
//  pointers) and return a value indicating the relationship between the two
//  items.  The return values are as follows:
//  
//      < 0  means  Item1 < Item2
//      = 0  means  Item1 = Item2
//      > 0  means  Item1 > Item2
//  
//  Note that the functions x_strcmp and x_stricmp fit this pattern and can
//  be used as compare functions.
//
//  This type definition is used by the quick sort and binary search 
//  functions declared below.
//
////////////////////////////////////////////////////////////////////////////

typedef s32 (*compare_fnptr)( const void* Item1, const void* Item2 );

////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//  
//  x_qsort     - Cleverly optimized quick sort function.
//
//  x_bsearch   - Binary search function.  Return value is address of 
//                desired item in the list, or NULL if item was not found.
//  
//  Note that the quick sort function is recursive.  It is guaranteed that
//  the function will not recurse more than "log base 2 of NItems".  Uses
//  of x_qsort in a critical program (such as a game) should be tested under
//  the most extreme potential conditions to avoid stack overflows.
//  
//  The input to the binary search funtion should be a list sorted in 
//  ascending order with no duplicates.  The behavior of the function is
//  not defined if the list is out of order.  If the list contains duplicate
//  items (relative to the Compare function), then one of the items will be
//  found and reported.
//  
////////////////////////////////////////////////////////////////////////////
                                
void x_qsort    ( const void*     pBase,          // Address of first item in array.
                  s32             NItems,         // Number of items in array.
                  s32             ItemSize,       // Size of one item.
                  compare_fnptr   Compare );      // Compare function.
                
void* x_bsearch ( const void*     pKey,           // Reference item to search for.
                  const void*     pBase,          // First item in the array.
                  s32             NItems,         // Number of items in array.
                  s32             ItemSize,       // Size of one item.
                  compare_fnptr   Compare );      // Compare function.
                  
////////////////////////////////////////////////////////////////////////////
//  Pseudo random number generation functions.
////////////////////////////////////////////////////////////////////////////
//
//  x_srand      - Set seed for the random number generator.
//  x_rand       - Random integer in [0, X_RAND_MAX].
//  x_irand      - Random integer in [Min, Max].
//  x_frand      - Random float   in [Min, Max].
//
//  X_RAND_MAX is the largest value which can be returned by x_rand().
//  
////////////////////////////////////////////////////////////////////////////

#define X_RAND_MAX   0x7FFF 

////////////////////////////////////////////////////////////////////////////

void    x_srand     ( s32 Seed );
s32     x_rand      ( void );               // [0 - X_RAND_MAX]
s32     x_irand     ( s32 Min, s32 Max );   // [Min - Max]
f32     x_frand     ( f32 Min, f32 Max );   // [Min - Max]

////////////////////////////////////////////////////////////////////////////
//  Standard NULL terminated string manipulation functions.
////////////////////////////////////////////////////////////////////////////
//  
//  x_strlen        - Return length of string.
//  x_strcpy        - Copy string.
//  x_strcat        - Concatenate one string onto end of another.
//  x_strncat       - Concatenate, but with character limit.
//  x_strcmp        - String comparision.
//  x_strncmp       - String comparision, but with character limit.
//  x_stricmp       - String comparision, but ignore upper/lower case.
//  x_strstr        - Find substring within string.
//  x_strrchr       - Find last occurrence of given character in string.
//  x_strtok        - Parse input string searching for delimiters.
//  x_sscanf        - Parse input string according to the format string.
//  
//  Bonus functions included free with every copy of the x_files!
//  
//  s_strtoupper    - Upper case entire string.
//  s_strtolower    - Lower case entire string.
//  
////////////////////////////////////////////////////////////////////////////

s32     x_strlen    ( const char* pStr );
char*   x_strcpy    ( char* pDest,  const char* pSrc );
char*   x_strncpy   ( char* pDest,  const char* pSrc, s32 Count );
char*   x_strcat    ( char* pFront, const char* pBack );
char*   x_strncat   ( char* pFront, const char* pBack, s32 Count );
s32     x_strcmp    ( const char* pStr1,  const char* pStr2 );
s32     x_strncmp   ( const char* pStr1,  const char* pStr2, s32 Count );
s32     x_stricmp   ( const char* pStr1,  const char* pStr2 );
char*   x_strstr    ( const char* pMainStr, const char* pSubStr );
char*   x_strrchr   ( const char* pStr, s32 c );
char*   x_strtok    ( char* pString, const char* pDelimit );
s32     x_sscanf    ( const char *in_string, const char *format, ... );

char*   x_strtoupper( char* pStr );
char*   x_strtolower( char* pStr );

////////////////////////////////////////////////////////////////////////////
//  Standard block memory functions.
////////////////////////////////////////////////////////////////////////////
//  
//  x_memcpy    - Copy memory block.  Blocks must not overlap.
//  x_memmove   - Copy memory block.  Blocks may overlap.
//  x_memset    - Fill memory block with given byte (or character) value.
//  x_memchr    - Find first occurrence of character within memory block.
//  x_memcmp    - Compare memory blocks.
//  
//  Bonus function included free with every copy of the x_files!
//  
//  x_chksum    - Create a check sum value from a memory block.
//  
////////////////////////////////////////////////////////////////////////////

void*   x_memcpy    ( void* pDest, const void* pSrc, s32 Count );
void*   x_memmove   ( void* pDest, const void* pSrc, s32 Count );
void*   x_memset    ( void* pBuf, s32 C, s32 Count );
void*   x_memchr    ( void* pBuf, s32 C, s32 Count );
s32     x_memcmp    ( const void* pBuf1, const void* pBuf2, s32 Count );

u32     x_chksum    ( const void* pBuf, s32 Count, u32 CRC );

////////////////////////////////////////////////////////////////////////////
//  Standard ASCII conversion functions.
////////////////////////////////////////////////////////////////////////////
//  
//  x_toupper   - Return upper case version of given character.
//  x_tolower   - Return lower case version of given character.
//  x_atoi      - Return integer value based on string such as "123".
//  x_atof      - Return float value based on string such as "1.5".
//  
////////////////////////////////////////////////////////////////////////////

s32     x_toupper   ( s32 C );
s32     x_tolower   ( s32 C );
s32     x_atoi      ( const char* pStr );
f64     x_atof      ( const char* pStr );

////////////////////////////////////////////////////////////////////////////
//  Standard character classifiers.
////////////////////////////////////////////////////////////////////////////
//  
//  x_isspace   - Whitespace?  (Space, tab, cariage return, line feed.)
//  x_isdigit   - '0' thru '9'.
//  x_isalpha   - 'A' thru 'Z' and 'a' thru 'z'.
//  
////////////////////////////////////////////////////////////////////////////

inline xbool x_isspace( s32 c ) { return ( c == 0x09 ) || ( c == 0x0A ) || ( c == 0x0D ) || ( c == ' '); }
inline xbool x_isdigit( s32 c ) { return (c >= '0') && (c <= '9'); }
inline xbool x_isalpha( s32 c ) { return ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')); }


////////////////////////////////////////////////////////////////////////////
//  Path manipulation functions.
////////////////////////////////////////////////////////////////////////////
//  
//  x_splitpath - Break a fully or partially qualified path/filename into
//                its components.
//  
//  x_makepath  - Assemble a path/filename from given parts.
//  
//  Note that, except for the pPath pointer, you may pass NULL for any 
//  parameter which is not needed.  For example, if you have a file name 
//  which may or may not have path information and you just need the base
//  file name, then use something like...
//  
//      char* pFilename;
//      char  FName[ X_MAX_FNAME ];
//      ...
//      x_splitpath( pFilename, NULL, NULL, FName, NULL );
//  
//  The X_MAX_... values represent the largest sizes of their respective
//  portions of a fully qualified path and file name.
//  
////////////////////////////////////////////////////////////////////////////

#define X_MAX_PATH   260
#define X_MAX_DRIVE    3
#define X_MAX_DIR    256
#define X_MAX_FNAME  256
#define X_MAX_EXT    256

////////////////////////////////////////////////////////////////////////////

void x_splitpath    ( const char* pPath, char* pDrive,
                                         char* pDir,
                                         char* pFName,
                                         char* pExt );

void x_makepath     ( char* pPath, const char* pDrive,
                                   const char* pDir,  
                                   const char* pFName,
                                   const char* pExt );

////////////////////////////////////////////////////////////////////////////
//  Mutex stuff: type, macros, and functions.
////////////////////////////////////////////////////////////////////////////
//  
//  A simple mutex system is in place for taking care of critical sections
//  on systems which are utilizing multiple execution threads.  Note that
//  the x_files do NOT provide a threading model or system.  Rather, the
//  x_files are thread aware, and provide thread safe functionality.  The
//  following macros are used to generically work with mutex constructs.
//  
//  Critical sections within the x_files use these macros.
//  
//  On systems where there is no thread support, the macros evaporate.
//  
//  DECLARE_MUTEX(I)    - Declares mutex named I.
//  INIT_MUTEX(I)       - Initialize mutex before first use.
//  KILL_MUTEX(I)       - Destroy mutex after last use.
//  ENTER_MUTEX(I)      - Mark beginning of critical section.
//  EXIT_MUTEX(I)       - Mark end of critical section.
//
////////////////////////////////////////////////////////////////////////////

#ifdef TARGET_N64
    typedef struct
    {
        OSMesgQueue		MsgQ;
        OSMesg			Msg[1];
    } x_mutex;

    #define DECLARE_MUTEX(I)    x_mutex     Mutex_ ## I;
    #define INIT_MUTEX(I)       InitMutex ( Mutex_ ## I );
    #define KILL_MUTEX(I)       KillMutex ( Mutex_ ## I );
    #define ENTER_MUTEX(I)      EnterMutex( Mutex_ ## I );
    #define EXIT_MUTEX(I)       ExitMutex ( Mutex_ ## I );

    void InitMutex        ( x_mutex& Mutex );
    void EnterMutexSection( x_mutex& Mutex );
    void ExitMutexSection ( x_mutex& Mutex );
    void KillMutex        ( x_mutex& Mutex );
#else
    #define DECLARE_MUTEX(I)
    #define INIT_MUTEX(I)
    #define KILL_MUTEX(I)       
    #define ENTER_MUTEX(I)
    #define EXIT_MUTEX(I)
#endif

////////////////////////////////////////////////////////////////////////////

#endif
