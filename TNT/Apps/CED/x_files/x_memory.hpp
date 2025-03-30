////////////////////////////////////////////////////////////////////////////
//
//  X_MEMORY.HPP
//
////////////////////////////////////////////////////////////////////////////

#ifndef X_MEMORY_HPP
#define X_MEMORY_HPP

////////////////////////////////////////////////////////////////////////////
//  INCLUDES
////////////////////////////////////////////////////////////////////////////

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#endif

////////////////////////////////////////////////////////////////////////////
//  DEBUG DEFINES
////////////////////////////////////////////////////////////////////////////
//
//  The following values can be defined to assist in debugging problems
//  with dynamic memory.
//
//  X_DEBUG     - Will catch most errors without severe performance hit.
//  SLOW_MEMORY - Will catch even more errors, but runs slowly.
//              
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//
//  Memory initialization
//
//  When x_MemoryInit is called the memory manager is initialized but
//  there is no available memory for allocation.  Calls to x_DonateBlock
//  inform the manager of areas in memory that it may distribute.  A call
//  to x_MemoryKill shuts down the manager.
//
//  (Note that the PC version will allocate memory from the OS if it runs
//  out.  So, on the PC, explicit use of x_DonateBlock is not needed.)
//
////////////////////////////////////////////////////////////////////////////

void	x_MemoryInit		( void );
void	x_MemoryKill		( void );
void	x_DonateBlock		( byte* BlockAddr, s32 NBytes );


void	x_MemoryMaxRAM		( u32 MaxRAM );
void	x_MemoryStackSize	( u32 Size );
void	x_MemoryAllowGrowth	( xbool Setting );
void	x_MemoryCapGrowth	( s32 Size );



////////////////////////////////////////////////////////////////////////////
//
//  Memory allocation
//
//  Regular malloc and free are replaced with the equivalent x_malloc and
//  x_free.  These allocations are 16 BYTE ALIGNED!
//
////////////////////////////////////////////////////////////////////////////

void*	x_malloc			( s32   NBytes  );
void*	x_malloc_top		( s32   NBytes  );
void*	x_calloc			( s32   NBytes  );
void	x_free				( void* pMemory );

////////////////////////////////////////////////////////////////////////////
//
//  Resize existing memory allocation
//  Works just like standard realloc().
//
////////////////////////////////////////////////////////////////////////////

void*	x_realloc			( void* Ptr, s32 NewNBytes );

////////////////////////////////////////////////////////////////////////////
//  DEBUG routines - always available
////////////////////////////////////////////////////////////////////////////

s32		x_TotalMemory				( void );       // total memory in manager
s32		x_TotalMemoryFree			( void );       // total free memory in manager
s32		x_LargestBlockFree			( void );
s32		x_NumBlocksFree				( s32* pBlockSizes=NULL, s32 BlockSizesLength=0 );

s32		x_TotalAllocatedBlocks		( void );       // total num of malloc()'s so far
s32		x_TotalFreedBlocks			( void );       // total num of free()'s so far
s32		x_MemSanityCheck			( void );
u32		x_SizeOfBlock				( void* pBlock );  // returns the size of the actual block
u32		x_SizeOfUserBlock			( void* pBlock );  // User Allocated size buffer

void	x_DumpHeapInfo				( char* pFileName );
void	x_DumpHeapInfo2				( void );       // Dump to 'HeapDump.txt'

////////////////////////////////////////////////////////////////////////////
//
//	Stack checking functions
//
////////////////////////////////////////////////////////////////////////////

void	x_StackCheckInit		( void );	// inits/re-inits the stack checking
u32		x_StackCheckSize		( void );	// returns the overall size of the stack (fast call)
u32		x_StackCheckUsed		( void );	// returns the amount of stack used (slow call)
u32		x_StackCheckLeft		( void );	// returns the amount of stack left (slow call)
u32		x_StackCheck			( void );	// ASSERTS if the stack gets low (slow call)


////////////////////////////////////////////////////////////////////////////
//
//  DON'T LOOK DOWN HERE!
//
//  These prototypes and macros are here to change the manager's
//  behavior under debug and release compiles.
//
////////////////////////////////////////////////////////////////////////////

#ifdef X_DEBUG

    #define x_malloc(S)                 x_fn_malloc  ( (S),      __FILE__, __LINE__ )
    #define x_realloc(A,S)              x_fn_realloc ( (A), (S), __FILE__, __LINE__ )
    #define x_malloc_top(S)             x_fn_malloc_top( (S), __FILE__, __LINE__ )
#else

    #define x_malloc(S)                 x_fn_malloc  ( (S),      "", 0 )
    #define x_realloc(A,S)              x_fn_realloc ( (A), (S), "", 0 )
    #define x_malloc_top(S)             x_fn_malloc_top ( (S),      "", 0 )
#endif

////////////////////////////////////////////////////////////////////////////
// Prototype the 'fn' functions.

void*       x_fn_malloc                ( s32    NBytes,
                                         char*  pFileName,
                                         s32    LineNumber );
void*       x_fn_realloc               ( void*  pPtr,
                                         s32    NewNBytes,
                                         char*  pFileName,
                                         s32    LineNumber );
void*       x_fn_malloc_top            ( s32    NBytes,
                                         char*  pFileName,
                                         s32    LineNumber );
void*       x_fn_MallocAtAddress       ( byte*  Addr,
                                         s32    NBytes,
                                         s32    HeapID,
                                         char*  pFileName,
                                         s32    LineNumber );
void*       x_fn_MallocAligned         ( s32    NBytes,
                                         s32    AlignmentPower,
                                         s32    HeapID,
                                         char*  pFileName,
                                         s32    LineNumber );

////////////////////////////////////////////////////////////////////////////

#endif
