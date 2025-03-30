///////////////////////////////////////////////////////////////////////////
//
//  X_MEMORY.H
//
///////////////////////////////////////////////////////////////////////////

#ifndef X_MEMORY_H
#define X_MEMORY_H

///////////////////////////////////////////////////////////////////////////
//  INCLUDES
///////////////////////////////////////////////////////////////////////////

#ifndef X_TYPES_H
#include "x_types.h"
#endif

///////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif
    
///////////////////////////////////////////////////////////////////////////
//  DEBUG DEFINES
///////////////////////////////////////////////////////////////////////////

// The following values can be defined to assist in debugging problems
// with dynamic memory.

// X_DEBUG      - Will catch most errors without severe performance hit.
// SLOW_MEMORY  - Will catch even more errors, but runs slowly.

///////////////////////////////////////////////////////////////////////////
//  ALIGNMENT DEFINES
///////////////////////////////////////////////////////////////////////////

#define ALIGN_TO_16         4
#define ALIGN_TO_32         5
#define ALIGN_TO_64         6
#define ALIGN_TO_128        7
#define ALIGN_TO_256        8
#define ALIGN_TO_512        9
#define ALIGN_TO_1024      10

///////////////////////////////////////////////////////////////////////////
//  Memory initialization
//
//  When x_MemoryInit is called the memory manager is initialized but
//  there is no available memory for allocation.  Calls to x_DonateBlock
//  inform the manager of areas in memory that it may distribute.  A call
//  to x_MemoryKill shuts down the manager.
//
///////////////////////////////////////////////////////////////////////////

void  x_MemoryInit      ( void );
void  x_MemoryKill      ( void );
void  x_DonateBlock     ( u32 BlockAddr, s32 NBytes );

///////////////////////////////////////////////////////////////////////////
//  PLATFORM SPECIFIC Memory Prep Functions
///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_N64
void x_N64_PrepMemory   ( u32 HeapStart, s32 HeapSize );
#endif

#ifdef TARGET_N64
void x_PC_PrepMemory    ( s32 HeapSize );
#endif

///////////////////////////////////////////////////////////////////////////
//  Memory allocation
//
//  Regular malloc and free are replaced with the equivalent x_malloc and
//  x_free.  These allocations are 16 BYTE ALIGNED!
//
///////////////////////////////////////////////////////////////////////////

void* x_malloc          ( s32 NBytes );
void  x_free            ( void* Ptr );

///////////////////////////////////////////////////////////////////////////
//  Resize existing memory allocation
//
//  Works just like standard realloc().
//
///////////////////////////////////////////////////////////////////////////

void* x_realloc         ( void* Ptr, s32 NewNBytes );

///////////////////////////////////////////////////////////////////////////
//  Memory at address
//
//  For those who know where they want a buffer placed, here's a malloc
//  that let's you specify an address and size.  If the block is available,
//  the manager will give it to you. The address requested must be
//  16 BYTE ALIGNED!
//
///////////////////////////////////////////////////////////////////////////

void* x_MallocAtAddress ( u32 Addr, s32 NBytes, s32 HeapID );

///////////////////////////////////////////////////////////////////////////
//  Custom alignment
//
//  This allocation allows the user to specify an 2^X byte alignment where
//  X >= 4.  Alignment defines are available above.
//
///////////////////////////////////////////////////////////////////////////

void* x_MallocAligned ( s32 NBytes, s32 AlignmentPower, s32 HeapID );

///////////////////////////////////////////////////////////////////////////
//  DEBUG routines - always available
///////////////////////////////////////////////////////////////////////////

s32   x_TotalMemory             ( void );       // total memory in manager
s32   x_TotalMemoryFree         ( void );       // total free memory in manager
s32   x_LargestBlockFree        ( void );

s32   x_TotalAllocatedBlocks    ( void );       // total num of malloc()'s so far
s32   x_TotalFreedBlocks        ( void );       // total num of free()'s so far
err   x_MemSanityCheck          ( void );
u32   x_SizeOfBlock             ( void* Ptr );  // returns the size of the actual block
u32   x_SizeOfUserBlock         ( void* Ptr );  // User Allocated size buffer

void  x_DumpHeapInfo            ( char* FileName );
void  x_DumpHeapInfo2           ( void );       // Dump to 'HeapDump.txt'

///////////////////////////////////////////////////////////////////////////
//
//  DON'T LOOK DOWN HERE!
//
//  These prototypes and macros are here to change the manager's
//  behavior under debug and release compiles.
//
///////////////////////////////////////////////////////////////////////////

#ifdef X_DEBUG

    #define x_malloc(S)                 x_fn_malloc         ( (S),           __FILE__, __LINE__ )
    #define x_realloc(A,S)              x_fn_realloc        ( (A), (S),      __FILE__, __LINE__ )
    #define x_MallocAtAddress(A,S,H)    x_fn_MallocAtAddress( (A), (S), (H), __FILE__, __LINE__ )
    #define x_MallocAligned(S,A,H)      x_fn_MallocAligned  ( (S), (A), (H), __FILE__, __LINE__ )

#else

    #define x_malloc(S)                 x_fn_malloc         ( (S),           "", 0 )
    #define x_realloc(A,S)              x_fn_realloc        ( (A), (S),      "", 0 )
    #define x_MallocAtAddress(A,S,H)    x_fn_MallocAtAddress( (A), (S), (H), "", 0 )
    #define x_MallocAligned(S,A,H)      x_fn_MallocAligned  ( (S), (A), (H), "", 0 )

#endif

///////////////////////////////////////////////////////////////////////////
// Prototype the 'fn' functions.

void*       x_fn_malloc                ( s32    NBytes,
                                         char*  FileName,
                                         s32    LineNumber );
void*       x_fn_realloc               ( void*  Ptr,
                                         s32    NewNBytes,
                                         char*  FileName,
                                         s32    LineNumber );
void*       x_fn_MallocAtAddress       ( u32    Addr,
                                         s32    NBytes,
                                         s32    HeapID,
                                         char*  FileName,
                                         s32    LineNumber );
void*       x_fn_MallocAligned         ( s32    NBytes,
                                         s32    AlignmentPower,
                                         s32    HeapID,
                                         char*  FileName,
                                         s32    LineNumber );
void*       x_fn_hmalloc               ( s32    HeapID,
                                         s32    NBytes,
                                         char*  FileName,
                                         s32    LineNumber );

///////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

///////////////////////////////////////////////////////////////////////////

#endif
