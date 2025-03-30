////////////////////////////////////////////////////////////////////////////
//
//  x_memory.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_debug.hpp"
#include "x_memory.hpp"
#include "x_plus.hpp"
#include "x_stdio.hpp"
#include "x_target.hpp"


////////////////////////////////////////////////////////////////////////////
// PLATFORM-SPECIFIC INCLUDES
////////////////////////////////////////////////////////////////////////////

#if defined( TARGET_PC ) || defined( TARGET_XBOX )
    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>
#endif

#if defined( TARGET_PS2 )

  #ifndef _MSC_VER
    #include <malloc.h>
  #else
    void* malloc( size_t Size  );
    void  free  ( void *pBlock );
  #endif

#endif


////////////////////////////////////////////////////////////////////////////
//  MODULE DEBUG OPTIONS
////////////////////////////////////////////////////////////////////////////

#ifdef X_DEBUG
    #define DO_HEAP_DUMP_ON_MALLOC_FAIL
#endif


#if defined( TARGET_DOLPHIN )
    #define DISABLE_HEAP_DUMP
#endif

//#define SLOW_MEMORY

//#define ENABLE_THREAD_SAFETY //Rusty: This is a joke -- NOT THREAD SAFE -- Do Not Use


////////////////////////////////////////////////////////////////////////////
//  DEFINES
////////////////////////////////////////////////////////////////////////////

#define MIN_SIZE_FOR_SMALLS         (16*1024)

#define MEM_MIN_FREE_POWER          0                           // 1       byte
#define MEM_MAX_FREE_POWER          20                          // 1048576 bytes
#define MEM_HASH_ENTRIES            (MEM_MAX_FREE_POWER + 1)

#define MIN_DONATED_BLOCK_SIZE      1024                        // 16 * 64


//--- SYSTEM SPECIFIC ALLOCATORS ----------------
#if defined( TARGET_DOLPHIN )
    #define SYSALLOC( Size )        OSAlloc( (u32)Size )
    #define SYSFREE( Ptr )          OSFree( (void*)Ptr )
#else
    #define SYSALLOC( Size )        malloc( (u32)Size )
    #define SYSFREE( Ptr )          free( (void*)Ptr )
#endif


#if defined( TARGET_PC ) || defined( TARGET_XBOX )
    #define DONATE_SIZE             (16*1024*1024)
#else
    #define DONATE_SIZE             (1*1024*1024)
#endif


//--- SYSTEM MEMORY ALIGNMENT REQUIREMENTS ------
#if defined( TARGET_PS2 )
    #define ALIGN_BITS              5
#elif defined( TARGET_DOLPHIN )
    #define ALIGN_BITS              5
#else
    #define ALIGN_BITS              4
#endif


#define ALIGN_SIZE                  (1 << ALIGN_BITS)
#define ALIGN_MASK                  (ALIGN_SIZE - 1)

#define ALIGN_UP(x)                 ((((u32)(x))+ALIGN_MASK)&~ALIGN_MASK)
#define ALIGN_DOWN(x)               (((u32)(x))&~ALIGN_MASK)
#define IS_ALIGNED(x)               ((((u32)(x))&ALIGN_MASK)?(0):(1))


#define ZERO_SIZE_ADDRESS           ((void*)(0xFFFFFFF0))

#ifdef X_DEBUG
    #define PAD  16
#else
    #define PAD   0
#endif


////////////////////////////////////////////////////////////////////////////
//  MEMORY SIZES
////////////////////////////////////////////////////////////////////////////

//////// PS2 ////////////////
#if defined( TARGET_PS2 )

  //-------------------------------------------------------------------------------------------------------------------------------------
  #if defined( X_DEBUG )
    #if defined( TARGET_PS2_DEV )
      #define DEFAULT_MAX_RAM       (124*1024*1024)         //-- Devkit Debug
    #else
      #define DEFAULT_MAX_RAM       (124*1024*1024)         //-- DVD Debug
    #endif

  #else
    #if defined( TARGET_PS2_DEV )
      #define DEFAULT_MAX_RAM       (32*1024*1024)          //-- Devkit Release
    #else
      #define DEFAULT_MAX_RAM       (32*1024*1024)          //-- DVD Release
    #endif

  #endif
  //-------------------------------------------------------------------------------------------------------------------------------------
  #define DEFAULT_STACK_SIZE        (1*1024*1024)

  #if X_DEBUG
    #define STACK_CHECK
  #endif


//////// DOLPHIN ////////////
#elif defined( TARGET_DOLPHIN )
    #define DEFAULT_MAX_RAM         (24*1024*1024)
    #define DEFAULT_STACK_SIZE      (64*1024)


//////// XBOX ///////////////
#elif defined( TARGET_XBOX )
#ifdef X_DEBUG
    #define DEFAULT_MAX_RAM         (64*1024*1024)
#else
    #define DEFAULT_MAX_RAM         (30*1024*1024)
#endif
    #define DEFAULT_STACK_SIZE      (1*1024*1024)


//////// PC /////////////////
#elif defined( TARGET_PC )
    #define DEFAULT_MAX_RAM         (1024*1024*1024)
    #define DEFAULT_STACK_SIZE      (1*1024*1024)


//////// DEFAULT ////////////
#else
    #define DEFAULT_MAX_RAM         (32*1024*1024)
    #define DEFAULT_STACK_SIZE      (1*1024*1024)

#endif


////////////////////////////////////////////////////////////////////////////
//  STRUCTURES
////////////////////////////////////////////////////////////////////////////

struct mem_link
{
    struct mem_link* Next;
    struct mem_link* Prev;
};

//--------------------------------------------------------------------------
#define SIZE_OF_MEM_BLOCK_FILENAME      48
struct mem_block
{
    ////////////////////////////////////
    // info for release and debug
    ////////////////////////////////////
    mem_link                LocalList;      // Links in Free or Heap list.              8 bytes
    struct mem_block*       GlobalNext;     // Next ptr in global list.                 4 bytes
    struct mem_block*       GlobalPrev;     // Prev ptr in global list.                 4 bytes
    byte*                   BlockAddr;      // Physical address of block                4 bytes
                                            // beginning. This is not necessarily
                                            // the beginning of the mem_block.
    s32                     BlockSize;      // Physical size of block from              4 bytes
                                            // BlockAddr to end of the block.
                                            // includes ALL memory involved in
                                            // block.
    s32                     UserSize;       // Physical size of memory requested        4 bytes
                                            // by user.
    u16                     FreeFlag;       // Block is free if FreeFlag == 1
    s16                     HeapID;         // If block is allocated, it belongs        2 bytes
                                            // in one of the heaps.  This is the        2 bytes
                                            // index to the correct heap.
//                                                                                      32 Bytes to this point.

    ////////////////////////////////////
    // info for debug only
    ////////////////////////////////////
#ifdef X_DEBUG
    s32                     Sequence;       // number of allocs before this block       4 bytes
    s32                     LineNumber;     // *.c line number of malloc call           4 bytes
    char                    FileName[SIZE_OF_MEM_BLOCK_FILENAME];   // *.c name         SIZE_OF_MEM_BLOCK_FILENAME
    u32                     FrontLining1;   // front lining                             4 bytes
    u32                     FrontLining2;   // front lining                             4 bytes
#endif

}; // 32 bytes release, 96 bytes debug


//--------------------------------------------------------------------------

#ifdef X_DEBUG
struct heap_info
{
    s32 NumAllocs;
    s32 NumFrees;
    s32 CurrentMemoryAllocated;
    s32 MaxMemoryAllocated;
    s32 SumMemoryAllocated;
};
#endif

////////////////////////////////////////////////////////////////////////////
//  GLOBALS
////////////////////////////////////////////////////////////////////////////

static xbool        s_Initialized       = FALSE;

static u32          s_StackSize         = DEFAULT_STACK_SIZE;
static u32          s_MaxRAM            = DEFAULT_MAX_RAM;
static s32          s_CapGrowth         = 0;            // 0 means FALSE, non-zero is the cap size
static xbool        s_AllowGrowth       = FALSE;
static s32          s_Locked            = -1;

// Global list organization
static mem_block*   s_GlobalHead;

// Free memory table
static mem_link     s_FreeHead[MEM_HASH_ENTRIES];       // Contains blocksize >= (2^i)
static mem_link     s_FreeTail[MEM_HASH_ENTRIES];

// Heap organization
static mem_link     s_HeapHead;
static mem_link     s_HeapTail;
static s32          s_HeapBudget;
static s32          s_HeapAmount;

// Statistics
static s32          s_NumAllocs;
static s32          s_NumFrees;
static s32          s_MinUsefulBlockSize;

static s32          s_NDonations = 0;

#ifdef X_DEBUG
static u32          s_Lining4Byte = 0x21212121;         // '!!!!'
static u8           s_Lining1Byte = 0x21;               // '!'
static heap_info    s_HeapInfo;
#endif


static byte* s_SysMemPtr[32] = {0};
static s32   s_SysMemSize    = 0;
static s32   s_SizeofExecutable = 0;    // For GameCube and PS2.  XBOX will remain zero for now.


////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
////////////////////////////////////////////////////////////////////////////

static mem_block*   UserPtrToMemBlock           ( byte* UserPtr );
static void         InitBlock                   ( mem_block* B, s32 BlockSize, byte* BlockAddr );
static void         LinkLocalBlock              ( mem_link* Parent, mem_link* Block );
static void         UnlinkLocalBlock            ( mem_link* Block );
static s32          CompFreeHash                ( s32 NBytes );
static void         InsertFreeBlockIntoTable    ( mem_block* B );
static void         FreeBlock                   ( mem_block* B );
static void         FindAlignedMemory           ( mem_block** SB,  byte** Addr, s32 NBytes );
static void         FindAlignedMemoryTop        ( mem_block** SB,  byte** Addr, s32 NBytes );

static mem_block*   AllocateBlock               ( mem_block* SB, 
                                                  byte*      UserAddr, 
                                                  s32        UserSize, 
                                                  char*      FileName, 
                                                  s32        LineNumber );

static void*        ReallocShrink               ( mem_block* B, s32 NewNBytes );
static void*        ReallocGrow                 ( mem_block* B, s32 NewNBytes );
static mem_block*   MergeNeighboringFrees       ( mem_block* B );

static void         x_fn_DonateBlock            ( byte* aBlockAddr, s32 NBytes );
static s32          x_fn_MemSanityCheck         ( void );

static void*        x_fn_hmalloc                ( s32 NBytes, char* FileName, s32 LineNumber );
static void*        x_fn_hmalloc_top            ( s32 NBytes, char* FileName, s32 LineNumber );


#ifdef X_DEBUG

void x_PaintBSS  ( u64 PaintValue );
void x_PaintVoid ( u64 PaintValue );
void x_PaintStack( u64 PaintValue );

#endif

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//  LOCK FUNCTIONS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#if defined( ENABLE_THREAD_SAFETY )

    #define LOCK_X_MEMORY()     Lock( __LINE__ )
    #define UNLOCK_X_MEMORY()   Unlock()
#else

    #define LOCK_X_MEMORY()
    #define UNLOCK_X_MEMORY()

#endif

//--------------------------------------------------------------------------

inline void Lock( s32 LineNumber )
{
    while( s_Locked > -1 )
    {
    }

    s_Locked = LineNumber;
}

//--------------------------------------------------------------------------

inline void Unlock( void )
{
    s_Locked = -1;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//  PRIVATE FUNCTIONS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

static mem_block* UserPtrToMemBlock( byte* UserPtr )
{
    mem_block* B;

    ASSERT( UserPtr );
    B = ((mem_block*)(UserPtr)) - 1;

    //////////////////////////////////////////////////
    // Verify that this is an actual in-use memory block
    ASSERT( !B->FreeFlag );
    ASSERT( ((mem_link*)B)->Next->Prev == ((mem_link*)B) );
    ASSERT( ((mem_link*)B)->Prev->Next == ((mem_link*)B) );

#ifdef X_DEBUG

    //////////////////////////////////////////////////
    // Check linings
    s32 i;
    u8* LiningPtr;

    ASSERT( (B->FrontLining1 == s_Lining4Byte) && (B->FrontLining2 == s_Lining4Byte) );

    LiningPtr = (u8*)(((u32)B) + sizeof(mem_block) + B->UserSize);
    for( i = 0; i < PAD; i++ )
        ASSERT( LiningPtr[i] == s_Lining1Byte );

#endif

    return( B );
}

////////////////////////////////////////////////////////////////////////////

static void InitBlock( mem_block* B, s32 BlockSize, byte* BlockAddr )
{
    ASSERT( B );
    ASSERT( BlockSize > 0 );
    B->LocalList.Next   = NULL;
    B->LocalList.Prev   = NULL;
    B->GlobalNext       = NULL;
    B->GlobalPrev       = NULL;
    B->BlockAddr        = BlockAddr;
    B->BlockSize        = BlockSize;
    B->UserSize         = 0;
    B->FreeFlag         = 1;
    B->HeapID           = 0;

#ifdef X_DEBUG
    B->FrontLining1     = s_Lining4Byte;
    B->FrontLining2     = s_Lining4Byte;

    //////////////////////////////////////////////////
    // Try to preserve the filename and line that allocated
    // this memory block if that data exists.  This helps to
    // locate code that is causing memory fragmentation.
    //
    // If this is a 'new' block, check the filename for valid
    // characters- if a non-printable value is found, then
    // it was never a valid filename in the first place.
    // Also check the line number- if it is a super-high value
    // then it is probably not valid.
    //////////////////////////////////////////////////
    s32 i;

    if( B->LineNumber > 30000 )
        B->LineNumber = 30000;  // 30,000 lines in a file is pretty crazy

    for( i = 0; i < SIZE_OF_MEM_BLOCK_FILENAME; i++ )
    {
        if( x_isspace( B->FileName[i] ) )   continue;
        if( x_isdigit( B->FileName[i] ) )   continue;
        if( x_isalpha( B->FileName[i] ) )   continue;
        if( '.'  == B->FileName[i] )        continue;
        if( '\\' == B->FileName[i] )        continue;
        if( '/'  == B->FileName[i] )        continue;
        if( '_'  == B->FileName[i] )        continue;

        if( B->FileName[i] == '\0' )
            break;

        B->Sequence   = 0;
        B->LineNumber = 0;
        x_memset( B->FileName, 0, SIZE_OF_MEM_BLOCK_FILENAME );
        break;
    }

#endif
}

////////////////////////////////////////////////////////////////////////////

static void LinkLocalBlock( mem_link* Parent, mem_link* Block )
{
    ASSERT( Parent );
    ASSERT( Block );
    ASSERT( Parent->Next );
    ASSERT( Parent->Next->Prev );
    Block->Prev       = Parent;
    Block->Next       = Parent->Next;
    Block->Prev->Next = Block;
    Block->Next->Prev = Block;
}

////////////////////////////////////////////////////////////////////////////

static void UnlinkLocalBlock( mem_link* Block )
{
    ASSERT( Block );
    ASSERT( Block->Prev );
    ASSERT( Block->Next );
    ASSERT( Block->Prev->Next == Block );
    ASSERT( Block->Next->Prev == Block );
    Block->Prev->Next   = Block->Next;
    Block->Next->Prev   = Block->Prev;
    Block->Next         = NULL;
    Block->Prev         = NULL;
}

////////////////////////////////////////////////////////////////////////////

static s32 CompFreeHash( s32 NBytes )
{
    s32 Index;
    ASSERT( NBytes > 0 );

    Index    =  MEM_MIN_FREE_POWER;
    NBytes >>= (MEM_MIN_FREE_POWER + 1);

    while( (NBytes > 0) && (Index < MEM_MAX_FREE_POWER) )
    {
        NBytes >>= 1;
        Index++;
    }

    return Index;
}

////////////////////////////////////////////////////////////////////////////

static void InsertFreeBlockIntoTable( mem_block* B )
{
    s32 HashIndex;
    ASSERT( B );

    // Assuming block has been initialized and is already
    // in the global list.  By now, mem_block should be at
    // the beginning of the block meaning BlockAddr == B
    ASSERT( (mem_block*)(B->BlockAddr) == B );

    // Insert block into free hash table
    HashIndex = CompFreeHash( B->BlockSize );
    LinkLocalBlock( &s_FreeHead[HashIndex], &(B->LocalList) );

    //-----------------------------------------------------
    // Clear the allocated storage
    //-----------------------------------------------------
#ifdef X_DEBUG
    s32 NBytes = (B->BlockSize - (s32)sizeof(mem_block));

    if( NBytes > 4096 )
        NBytes = 4096;

    x_memset( (void*)( B->BlockAddr + sizeof(mem_block) ), 0xBE, NBytes );
#endif
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//  PUBLIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void x_MemoryInit( void )
{
    s32 i;

    ASSERT( !s_Initialized );
    //ASSERT( IS_ALIGNED(sizeof(mem_block)) );

    //--- clear heaps
    s_HeapBudget      = 0;
    s_HeapAmount      = 0;
    s_HeapHead.Next   = &s_HeapTail;
    s_HeapHead.Prev   = NULL;
    s_HeapTail.Next   = NULL;
    s_HeapTail.Prev   = &s_HeapHead;

#ifdef X_DEBUG
    s_HeapInfo.NumAllocs              = 0;
    s_HeapInfo.NumFrees               = 0;
    s_HeapInfo.CurrentMemoryAllocated = 0;
    s_HeapInfo.MaxMemoryAllocated     = 0;
    s_HeapInfo.SumMemoryAllocated     = 0;
#endif
    s_HeapBudget = (s32)0x7FFFFFFF;

#ifdef TARGET_PC
    s_AllowGrowth = TRUE;
#endif

    //////////////////////////////////////////////////
    // clear free lists
    for( i = 0; i < MEM_HASH_ENTRIES; i++ )
    {
        s_FreeHead[i].Next = &s_FreeTail[i];
        s_FreeHead[i].Prev = NULL;
        s_FreeTail[i].Next = NULL;
        s_FreeTail[i].Prev = &s_FreeHead[i];
    }

    //////////////////////////////////////////////////
    // clear global list and statistics
    s_MinUsefulBlockSize = ALIGN_UP( sizeof(mem_block) );
    s_GlobalHead = NULL;
    s_NumAllocs    = 0;
    s_NumFrees     = 0;

    //////////////////////////////////////////////////
    // initialize stack checking
    x_StackCheckInit();

    //////////////////////////////////////////////////
    // mark manager as initialized
    s_Initialized = TRUE;

#if defined( X_DEBUG ) && defined( SLOW_MEMORY )
    ASSERT( x_MemSanityCheck() == 0 );
#endif

#if defined( TARGET_PS2 )
    extern void *_end, *_stack_size;
    u32  Size;
    u32  BSSEnd;
    u32  ActualStackSize;
    #define HEAP_ADJUSTMENT_SIZE    1024    // it will take a long time, but we will get a more exact count an the RAM available

    BSSEnd = (u32)&_end;
    s_SizeofExecutable = BSSEnd;

    ActualStackSize = (u32)&_stack_size;

    Size = s_MaxRAM - s_StackSize - BSSEnd;

    //--- make sure the stack size requested isn't larger than the real stack size
    if( ActualStackSize < s_StackSize )
        s_StackSize = ActualStackSize;

    //--- cap the size of block allocated
    if( s_CapGrowth && ((s32)Size > s_CapGrowth) )
        Size = s_CapGrowth;

    //--- allocate the block of heap, if it doesn't fit adjust it till it does
    while( TRUE )
    {
        s_SysMemPtr[0] = (byte*)SYSALLOC( (u32)Size );
        if( s_SysMemPtr[0] != NULL )
            break;

        Size -= HEAP_ADJUSTMENT_SIZE;

        if( Size <= 0 )
        {
            ASSERTS( FALSE, "X_MEMORY: Couldn't allocate system memory" );
            BREAK;
        }
    }

#ifdef X_DEBUG
	//-- Fill in the heap with something so that we can "catch" uninitiazed crizap
	for ( i = 0; i < (s32)Size; i+=4 )
	{
		(s_SysMemPtr[0][i])	 = 0xBE;
		(s_SysMemPtr[0][i + 1]) = 0xBE;
		(s_SysMemPtr[0][i + 2]) = 0xBE;
		(s_SysMemPtr[0][i + 3]) = 0xBE;
	}
#endif

    //--- Give the memory to our manager.
    s_SysMemSize = Size;
    x_DonateBlock( s_SysMemPtr[0], Size );
    s_NDonations = 1;

#elif defined( TARGET_DOLPHIN )

    #define HEAP_ADJUSTMENT_SIZE    1024
    u32 MemAvailable;
    u32 AppMemReserved;

    // calc memory used by code and how much is available to heap space
    AppMemReserved = (u32)OSGetArenaLo() + (2 * 1024);
    MemAvailable   = (u32)OSGetArenaHi() - AppMemReserved;

    // removing cache flag bits from memory address to save elf size
    s_SizeofExecutable = (s32)(AppMemReserved & 0x0FFFFFFF);

    if( s_MaxRAM < MemAvailable )
        MemAvailable = s_MaxRAM;

    //--- Try to allocate maximum amount of system RAM, reduce size until successful
    while( TRUE )
    {
        s_SysMemPtr[0] = (byte*)SYSALLOC( (u32)MemAvailable );
        if( s_SysMemPtr[0] != NULL )
            break;

        MemAvailable -= HEAP_ADJUSTMENT_SIZE;

        if( MemAvailable <= 0 )
        {
            ASSERTS( FALSE, "X_MEMORY: Couldn't allocate system memory" );
            BREAK;
        }
    }

    s_SysMemSize = MemAvailable;
    x_DonateBlock( s_SysMemPtr[0], MemAvailable );
    s_NDonations = 1;

#elif defined( TARGET_XBOX )

    #define HEAP_ADJUSTMENT_SIZE    1024

    u32 MemAvailable = 32 * 1024 * 1024;

    if( s_MaxRAM < MemAvailable )
        MemAvailable = s_MaxRAM;

    //--- Try to allocate maximum amount of system RAM, reduce size until successful
    while( TRUE )
    {
        s_SysMemPtr[0] = (byte*)SYSALLOC( (u32)MemAvailable );
        if( s_SysMemPtr[0] != NULL )
            break;

        MemAvailable -= HEAP_ADJUSTMENT_SIZE;

        if( MemAvailable <= 0 )
        {
            ASSERTS( FALSE, "X_MEMORY: Couldn't allocate system memory" );
            BREAK;
        }
    }

#ifdef X_DEBUG
	//-- Fill in the heap with something so that we can "catch" uninitiazed crizap
	for ( i = 0; i < (s32)MemAvailable; i+=4 )
	{
		(s_SysMemPtr[0][i])	 = 0xBE;
		(s_SysMemPtr[0][i + 1]) = 0xBE;
		(s_SysMemPtr[0][i + 2]) = 0xBE;
		(s_SysMemPtr[0][i + 3]) = 0xBE;
	}
#endif

    s_SysMemSize = MemAvailable;
    x_DonateBlock( s_SysMemPtr[0], MemAvailable );
    s_NDonations = 1;

#endif
}

////////////////////////////////////////////////////////////////////////////

void x_MemoryKill( void )
{
    ASSERT( s_Initialized );

    s32 i;
    for( i = 0; i < 32; ++i )
    {
        if( s_SysMemPtr[i] != NULL )
            SYSFREE( s_SysMemPtr[i] );
    }

    s_SysMemSize = 0;

    s_Initialized = 0;
}

////////////////////////////////////////////////////////////////////////////

void x_MemoryMaxRAM( u32 Setting )
{
    s_MaxRAM = Setting;
}

////////////////////////////////////////////////////////////////////////////

void x_MemoryStackSize( u32 Setting )
{
    s_StackSize = Setting;
}

////////////////////////////////////////////////////////////////////////////

void x_MemoryAllowGrowth( xbool Setting )
{
    s_AllowGrowth = Setting;
}

////////////////////////////////////////////////////////////////////////////

void x_MemoryCapGrowth( s32 Size )
{
    s_CapGrowth = Size;
}

////////////////////////////////////////////////////////////////////////////

static void x_fn_DonateBlock( byte* aBlockAddr, s32 NBytes )
{
    byte*      LowBlockAddr;
    byte*      HighBlockAddr;
    s32        AlignedBlockSize;
    byte*      BlockAddr;
    mem_block* Block;

    ASSERT( s_Initialized );
    ASSERT( aBlockAddr );
    ASSERT( NBytes > 0 );
    BlockAddr = aBlockAddr;

    //////////////////////////////////////////////////
    // Align block to correct boundries
    LowBlockAddr     = (byte*)ALIGN_UP(BlockAddr);
    HighBlockAddr    = (byte*)ALIGN_DOWN(BlockAddr + NBytes);
    AlignedBlockSize = (HighBlockAddr - LowBlockAddr);

    //////////////////////////////////////////////////
    // Be sure block is worth dealing with
    if( AlignedBlockSize < MIN_DONATED_BLOCK_SIZE )
        return;
    if( AlignedBlockSize < s_MinUsefulBlockSize )
        return;

    //////////////////////////////////////////////////
    // Create block and add to free list
    Block = (mem_block*)LowBlockAddr;
    InitBlock( Block, AlignedBlockSize, LowBlockAddr );

    //////////////////////////////////////////////////
    // Sort block in global list by address
    if( s_GlobalHead == NULL )
    {
        s_GlobalHead = Block;
    }
    else if( (byte*)Block < (byte*)s_GlobalHead )
    {
        Block->GlobalNext        = s_GlobalHead;
        s_GlobalHead->GlobalPrev = Block;
        s_GlobalHead             = Block;
    }
    else
    {
        mem_block* B = s_GlobalHead;
        while( B )
        {
            if( (u32)Block < (u32)B )
            {
                Block->GlobalNext = B;
                Block->GlobalPrev = B->GlobalPrev;
                Block->GlobalPrev->GlobalNext = Block;
                Block->GlobalNext->GlobalPrev = Block;
                break;
            }

            if( B->GlobalNext == NULL )
            {
                B->GlobalNext = Block;
                Block->GlobalPrev = B;
                break;
            }

            B = B->GlobalNext;
        }
    }

    Block = MergeNeighboringFrees( Block );

    InsertFreeBlockIntoTable( Block );

#if defined( X_DEBUG ) && defined( SLOW_MEMORY )
    ASSERT( x_fn_MemSanityCheck() == 0 );
#endif
}

////////////////////////////////////////////////////////////////////////////

void x_DonateBlock( byte* aBlockAddr, s32 NBytes )
{
    LOCK_X_MEMORY();

    x_fn_DonateBlock( aBlockAddr, NBytes );

    UNLOCK_X_MEMORY();
}

////////////////////////////////////////////////////////////////////////////

s32 x_TotalMemory( void )
{
    LOCK_X_MEMORY();

    mem_block* Blk;
    s32        Total = 0;

    ASSERT( s_Initialized );

    for( Blk = s_GlobalHead; Blk; Blk = Blk->GlobalNext )
    {
        if( Blk->FreeFlag == FALSE )
            Total += Blk->BlockSize;
    }

    UNLOCK_X_MEMORY();

    return Total;
}

////////////////////////////////////////////////////////////////////////////

s32 x_TotalMemoryFree( void )
{
    LOCK_X_MEMORY();

    mem_block* Blk;
    s32        Total = 0;

    ASSERT( s_Initialized );

    for( Blk = s_GlobalHead; Blk; Blk = Blk->GlobalNext )
    {
        if( Blk->FreeFlag == TRUE )
            Total += Blk->BlockSize;
    }

    UNLOCK_X_MEMORY();

    return Total;
}

////////////////////////////////////////////////////////////////////////////

s32 x_LargestBlockFree( void )
{
    LOCK_X_MEMORY();

    mem_block* Blk;
    s32        Largest = 0;

    ASSERT( s_Initialized );

    for( Blk = s_GlobalHead; Blk; Blk = Blk->GlobalNext )
    {
        if( Blk->FreeFlag == TRUE )
        {
            if( Blk->BlockSize > Largest )
                Largest = Blk->BlockSize;
        }
    }

    UNLOCK_X_MEMORY();

    return Largest;
}

////////////////////////////////////////////////////////////////////////////

s32 x_NumBlocksFree( s32* pBlockSizes, s32 BlockSizesLength )
{
    LOCK_X_MEMORY();

    mem_block* Blk;
    s32        Count = 0;

    ASSERT( s_Initialized );

    for( Blk = s_GlobalHead; Blk; Blk = Blk->GlobalNext )
    {
        if( Blk->FreeFlag == TRUE )
        {
            if(( pBlockSizes ) && (Count<BlockSizesLength) )
                pBlockSizes[Count] = Blk->BlockSize;

            Count++;
        }
    }

    UNLOCK_X_MEMORY();

    return Count;
}

////////////////////////////////////////////////////////////////////////////

u32 x_SizeOfBlock( void* Ptr )
{
    LOCK_X_MEMORY();

    mem_block* MemBlock;

    ASSERT( s_Initialized );
    ASSERT( Ptr );

#ifdef SLOW_MEMORY
    ASSERT( x_fn_MemSanityCheck() == 0 );
#endif

    // Check for address of block with ZERO size
    if( Ptr == ZERO_SIZE_ADDRESS )
        return 0;

    // Attempt to free the block
    MemBlock = UserPtrToMemBlock( (byte*)Ptr );

    UNLOCK_X_MEMORY();

    // Return the Size of the block
    return MemBlock->BlockSize;
}

////////////////////////////////////////////////////////////////////////////

u32 x_SizeOfUserBlock( void* Ptr )
{
    LOCK_X_MEMORY();

    mem_block* MemBlock;

    ASSERT( s_Initialized );
    ASSERT( Ptr );

#ifdef SLOW_MEMORY
    ASSERT( x_fn_MemSanityCheck() == 0 );
#endif

    // Check for address of block with ZERO size
    if( Ptr == ZERO_SIZE_ADDRESS )
        return 0;

    // Attempt to free the block
    MemBlock = UserPtrToMemBlock( (byte*)Ptr );

    UNLOCK_X_MEMORY();

    // Return the Size of the block
    return MemBlock->UserSize;
}

////////////////////////////////////////////////////////////////////////////

s32 x_TotalAllocatedBlocks( void )
{
    ASSERT( s_Initialized );
    return s_NumAllocs;
}

////////////////////////////////////////////////////////////////////////////

s32 x_TotalFreedBlocks( void )
{
    ASSERT( s_Initialized );
    return s_NumFrees;
}

////////////////////////////////////////////////////////////////////////////

void x_DumpHeapInfo( char* FileName )
{
#ifdef DISABLE_HEAP_DUMP
    return;
#endif

#ifdef TARGET_XBOX
    char    XBOXFilenameBuffer[255];

    x_sprintf( XBOXFilenameBuffer, "T:\\%s", FileName );
    FileName = XBOXFilenameBuffer;
#endif

    s32         Count;
    u32         Waste          = 0;
    s32         BlockSizeTotal = 0;
    s32         UserSizeTotal  = 0;
    byte*       NextMemBlock   = NULL;
    mem_block*  M;
    X_FILE*     fp = NULL;

    ASSERT( s_Initialized );

    // Identify where the dump file is.
    x_printf( "Dumping Heap to File:%s\n", FileName );

    ////////////////////////////////////////////////////////////////////////
    //  CD based systems should not do this
    ////////////////////////////////////////////////////////////////////////
#if defined( TARGET_PS2_DVD ) || defined( TARGET_DOLPHIN_DVD )
    x_printf( "X_MEMORY ERROR: Cannot write to CD/DVD for heap dump\n" );
    return;
#endif


#if !defined( TARGET_DOLPHIN )
    #define START_PRINT     x_fprintf(fp,

    // Open file to write to
    fp = x_fopen( FileName, "wt" );
    //ASSERT( fp != NULL );
    if( fp == NULL )
    {
        x_printf( "X_MEMORY ERROR: Couldn't open file for x_DumpHeapInfo.\n" );
        return;
    }
#else
    #define START_PRINT     x_printf(
#endif

    #define END_PRINT       );


    //////////////////////////////////////////////////
    // Print out the memory info
    START_PRINT "\n" END_PRINT
    START_PRINT "B: Break   F: Front waste   R: Rear waste   HI: Heap Index\n" END_PRINT
#ifdef X_DEBUG
    START_PRINT "NA: Number of Allocs   AS: Average size\n" END_PRINT
    START_PRINT "NF: Number of Frees    MS: Maximum size\n" END_PRINT
#endif
    
    START_PRINT "---------------------------------------------------------------------------------------------\n" END_PRINT
    START_PRINT "\n" END_PRINT

    START_PRINT "Total Heap Size        : %d \n", x_GetSizeOfHeap()   END_PRINT
    START_PRINT "Total Allocated Memory : %d \n", x_TotalMemory()     END_PRINT
    START_PRINT "Total Free Memory      : %d \n", x_TotalMemoryFree() END_PRINT


    START_PRINT "\n" END_PRINT
    START_PRINT "                       /----------------------------------\\\n" END_PRINT
    START_PRINT "                       |       BLOCKS IN GLOBAL HEAP      |\n"  END_PRINT
    START_PRINT "---------------------------------------------------------------------------------------------\n" END_PRINT
    START_PRINT "BFR |  Address   |   BSize  |   USize  |  State   | HI | SeqNum | LNum | FileName\n"             END_PRINT
    START_PRINT "---------------------------------------------------------------------------------------------\n" END_PRINT

    LOCK_X_MEMORY();

    Count = 0;
    M     = s_GlobalHead;

    while( M != NULL )
    {
        if( NextMemBlock != M->BlockAddr)  START_PRINT "B" END_PRINT
        else                               START_PRINT "." END_PRINT

        if( M->FreeFlag )
        {
            START_PRINT ".. |" END_PRINT
        }
        else
        {
            if( M->BlockAddr != (byte*)M ) START_PRINT "F" END_PRINT
            else                           START_PRINT "." END_PRINT

            if( M->BlockAddr + M->BlockSize != ((byte*)M) + sizeof(mem_block) + ALIGN_UP(M->UserSize) + PAD )
                START_PRINT "R |" END_PRINT
            else
                START_PRINT ". |" END_PRINT
        }

        START_PRINT " 0x%8.8X | %8d | %8d |", (u32)(M->BlockAddr), M->BlockSize, M->UserSize  END_PRINT

        if( !M->FreeFlag )
            Waste += M->BlockSize - M->UserSize;

        NextMemBlock = M->BlockAddr + M->BlockSize;

        if( M->FreeFlag ) START_PRINT " --FREE-- | %2d |", CompFreeHash(M->BlockSize)  END_PRINT
        else              START_PRINT " AAAAAAAA | %2d |", M->HeapID                   END_PRINT

#ifdef X_DEBUG
        START_PRINT " %6d | %4d | %s", M->Sequence, M->LineNumber, M->FileName  END_PRINT
#endif

        BlockSizeTotal += M->BlockSize;
        UserSizeTotal  += M->UserSize;

        START_PRINT "\n" END_PRINT
        M = M->GlobalNext;
        Count++;
    }

    START_PRINT "---------------------------------------------------------------------------------------------\n" END_PRINT
    START_PRINT "Total wasted space = %ld bytes\n\n", Waste          END_PRINT
    START_PRINT "Total user bytes allocated: %8d\n", UserSizeTotal   END_PRINT
    START_PRINT "Total bytes allocated:      %8d\n", BlockSizeTotal  END_PRINT
    START_PRINT "---------------------------------------------------------------------------------------------\n" END_PRINT

#ifdef X_DEBUG
    s32 AvgSize = 0;
    START_PRINT "NA:%6d  NF:%6d  ", s_HeapInfo.NumAllocs, s_HeapInfo.NumFrees  END_PRINT
    if( s_HeapInfo.NumAllocs > 0 )
        AvgSize = (s_HeapInfo.SumMemoryAllocated / s_HeapInfo.NumAllocs);
    START_PRINT "AS:%6d  MS:%6d\n", AvgSize, s_HeapInfo.MaxMemoryAllocated  END_PRINT

    START_PRINT "---------------------------------------------------------------------------------------------\n" END_PRINT
#endif

    UNLOCK_X_MEMORY();

    if( fp != NULL )
        x_fclose( fp );
}

////////////////////////////////////////////////////////////////////////////

void x_DumpHeapInfo2( void )
{
    static s32 HDCount = 0;

    x_DumpHeapInfo( fs( "HeapDump%2d.txt", HDCount ) );

    HDCount++;
}


////////////////////////////////////////////////////////////////////////////
s32 x_GetSizeOfHeap( void )
{
    if( s_Initialized )
        return s_SysMemSize;
    else
        return -1;
}

////////////////////////////////////////////////////////////////////////////
s32 x_GetSizeOfExecutable( void )
{
    if( s_Initialized )
        return s_SizeofExecutable;
    else
        return -1;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS WHICH ARE AFFECTED BY DEBUG/RELEASE
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// Some other portion of the code has decided that we need to allocate a
// chunk of memory UserSize large, starting at address UserAddr.  The caller
// code has already found the source block SB that contains the memory
// requested.  AllocateBlock splits SB into whatever parts it needs and
// returns the final block.  AllocateBlock can be called from general
// mallocs, special alignment mallocs, or user address mallocs.

static mem_block* AllocateBlock( mem_block* SB, byte* UserAddr, s32 UserSize, char* FileName, s32 LineNumber )
{
    byte* UserStartAddress;
    byte* UserEndAddress;
    byte* BlockEndAddress;
    byte* BlockStartAddress;
    byte* ControlStartAddress;
    s32   FrontWasteSize;
    s32   BackWasteSize;
    mem_block TempB;

    ///////////////////////////////////////////////////////
    // Assert incoming parameters
    ASSERT( SB );
    ASSERT( UserSize > 0 );
    ASSERT( IS_ALIGNED(UserAddr) );

    ///////////////////////////////////////////////////////
    // Check if all the memory for the heap is taken
    ASSERT( s_HeapAmount + UserSize < s_HeapBudget );

    ///////////////////////////////////////////////////////
    // Update statistics
    s_NumAllocs++;

    ///////////////////////////////////////////////////////
    // Solve boundary addresses
    UserStartAddress    = UserAddr;
    UserEndAddress      = UserStartAddress + ALIGN_UP(UserSize+PAD);
    BlockEndAddress     = SB->BlockAddr + SB->BlockSize;
    BlockStartAddress   = SB->BlockAddr;
    ControlStartAddress = UserStartAddress - sizeof(mem_block);
    FrontWasteSize      = ControlStartAddress - BlockStartAddress;
    BackWasteSize       = BlockEndAddress - UserEndAddress;

    ///////////////////////////////////////////////////////
    // Assert boundary addresses
    ASSERT( BlockStartAddress    <= ControlStartAddress );
    ASSERT( ControlStartAddress  <  UserStartAddress    );
    ASSERT( UserStartAddress     <  UserEndAddress      );
    ASSERT( UserEndAddress       <= BlockEndAddress     );
    ASSERT( FrontWasteSize       >= 0                   );
    ASSERT( BackWasteSize        >= 0                   );
    ASSERT( IS_ALIGNED( BlockStartAddress    ) );
    ASSERT( IS_ALIGNED( ControlStartAddress  ) );
    ASSERT( IS_ALIGNED( UserStartAddress     ) );
    ASSERT( IS_ALIGNED( UserEndAddress       ) );
    ASSERT( IS_ALIGNED( FrontWasteSize       ) );
    ASSERT( IS_ALIGNED( BackWasteSize        ) );

    ///////////////////////////////////////////////////////
    // Remove SB from free list
    UnlinkLocalBlock( (mem_link*)SB );

    ///////////////////////////////////////////////////////
    // Move control block to appropriate position in block
    // and relink in global list
    TempB = *SB;
    SB = (mem_block*)(ControlStartAddress);
    InitBlock( SB, TempB.BlockSize, TempB.BlockAddr );
    SB->GlobalNext = TempB.GlobalNext;
    SB->GlobalPrev = TempB.GlobalPrev;

    if( SB->GlobalNext )
        SB->GlobalNext->GlobalPrev = SB;

    if( SB->GlobalPrev )
        SB->GlobalPrev->GlobalNext = SB;
    else
        s_GlobalHead = SB;

    SB->UserSize = UserSize;
    SB->FreeFlag = 0;

    ///////////////////////////////////////////////////////
    // Add block to correct heap list
    LinkLocalBlock( &s_HeapHead, (mem_link*)SB );
    s_HeapAmount += UserSize;

    ///////////////////////////////////////////////////////
    // Decide if front waste is big enough to break into 
    // a new block of it's own.
    if( FrontWasteSize >= s_MinUsefulBlockSize )
    {
        mem_block* FWB;

        // Split FWB from SB
        FWB = (mem_block*)(SB->BlockAddr);
        InitBlock( FWB, FrontWasteSize, (byte*)FWB );
        SB->BlockAddr = (byte*)SB;
        SB->BlockSize -= FrontWasteSize;

        // Link FWB into global list
        FWB->GlobalPrev = SB->GlobalPrev;

        if( FWB->GlobalPrev )
            FWB->GlobalPrev->GlobalNext = FWB;
        else
            s_GlobalHead = FWB;

        FWB->GlobalNext = SB;
        SB->GlobalPrev  = FWB;

        // Add FWB to free list
        InsertFreeBlockIntoTable( FWB );
    }

    ///////////////////////////////////////////////////////
    // Decide if back waste is big enough to break into 
    // a new block of it's own.
    if( BackWasteSize >= s_MinUsefulBlockSize )
    {
        mem_block* BWB;

        // Split BWB from SB
        BWB = (mem_block*)(UserEndAddress);
        InitBlock( BWB, BackWasteSize, (byte*)BWB );
        SB->BlockSize -= BackWasteSize;

        // Link BWB into global list
        BWB->GlobalNext = SB->GlobalNext;

        if( BWB->GlobalNext )
            BWB->GlobalNext->GlobalPrev = BWB;

        BWB->GlobalPrev = SB;
        SB->GlobalNext  = BWB;

        // Add BWB to free list
        InsertFreeBlockIntoTable( BWB );
    }

#ifdef X_DEBUG

    ///////////////////////////////////////////////////////
    // Update heap statistics
    s_HeapInfo.NumAllocs++;
    s_HeapInfo.CurrentMemoryAllocated += SB->UserSize;
    s_HeapInfo.SumMemoryAllocated     += SB->UserSize;
    if( s_HeapInfo.CurrentMemoryAllocated > s_HeapInfo.MaxMemoryAllocated )
        s_HeapInfo.MaxMemoryAllocated = s_HeapInfo.CurrentMemoryAllocated;

    ///////////////////////////////////////////////////////
    // Setup debug info
    s32 Len;
    u8* BackLiningPtr;
    Len = x_strlen(FileName)+1;
    if( Len > SIZE_OF_MEM_BLOCK_FILENAME )
        FileName += (Len-SIZE_OF_MEM_BLOCK_FILENAME);

    SB->Sequence   = s_HeapInfo.NumAllocs;
    SB->LineNumber = LineNumber;
    x_strncpy( SB->FileName, FileName, SIZE_OF_MEM_BLOCK_FILENAME );

    SB->FileName[SIZE_OF_MEM_BLOCK_FILENAME - 1] = 0;
    SB->FrontLining1 = s_Lining4Byte;
    SB->FrontLining2 = s_Lining4Byte;

    BackLiningPtr = (u8*)(((u32)SB) + sizeof(mem_block) + SB->UserSize);

    for( Len = 0; Len < PAD; Len++ )
        BackLiningPtr[Len] = s_Lining1Byte;

#endif

    ///////////////////////////////////////////////////////
    // We're done.
    return SB;
}

////////////////////////////////////////////////////////////////////////////

static mem_block* MergeNeighboringFrees( mem_block* B )
{
    mem_block* BPrev;
    mem_block* BNext;
    s32        BlockSize;

    BlockSize = B->BlockSize;
    BPrev     = B->GlobalPrev;
    BNext     = B->GlobalNext;

    ///////////////////////////////////////////////////////
    // check if we can merge BPrev and B
    if( BPrev && BPrev->FreeFlag )
    {
        ASSERT( BPrev->BlockAddr == (byte*)BPrev );
        if( ALIGN_UP(BPrev->BlockAddr + BPrev->BlockSize) == (u32)B->BlockAddr )
        {
            UnlinkLocalBlock( (mem_link*)BPrev );
            BPrev->GlobalNext = BNext;
            if( BNext )
                BNext->GlobalPrev = BPrev;
            BPrev->BlockSize += B->BlockSize;
            B = BPrev;
        }
    }

    ///////////////////////////////////////////////////////
    // check if we can merge B and BNext
    if( BNext && BNext->FreeFlag )
    {
        ASSERT( BNext->BlockAddr == (byte*)BNext );
        if( ALIGN_UP(B->BlockAddr + B->BlockSize) == (u32)BNext->BlockAddr )
        {
            UnlinkLocalBlock( (mem_link*)BNext );
            B->GlobalNext = BNext->GlobalNext;
            if( B->GlobalNext )
                B->GlobalNext->GlobalPrev = B;
            B->BlockSize += BNext->BlockSize;
        }
    }

    return B;
}

////////////////////////////////////////////////////////////////////////////

static void FreeBlock( mem_block* B )
{
    mem_block* BPrev;
    mem_block* BNext;
    s32        BlockSize;

    ///////////////////////////////////////////////////////
    // Set free bit-flag, and remove from heap list
    B->FreeFlag = 1;
    UnlinkLocalBlock( (mem_link*)B );
    s_HeapAmount -= B->UserSize;
    ASSERT( s_HeapAmount >= 0 );
#ifdef X_DEBUG
    s_HeapInfo.NumFrees++;
    s_HeapInfo.CurrentMemoryAllocated -= B->UserSize;
    ASSERT( s_HeapInfo.CurrentMemoryAllocated >= 0 );
#endif

    ///////////////////////////////////////////////////////
    // Convert block to free block.  Move control info to
    // front of the memory block and update global list ptrs.
    BlockSize                       = B->BlockSize;
    BPrev                           = B->GlobalPrev;
    BNext                           = B->GlobalNext;
    B                               = (mem_block*)(B->BlockAddr);
    InitBlock( B, BlockSize, (byte*)B );
    B->GlobalPrev                   = BPrev;
    B->GlobalNext                   = BNext;

    if( BNext )
        BNext->GlobalPrev = B;

    if( BPrev )
        BPrev->GlobalNext = B;
    else
        s_GlobalHead = B;

    ASSERT( B->BlockAddr == (byte*)B );

    ///////////////////////////////////////////////////////
    // Update statistics
    s_NumFrees++;
    B = MergeNeighboringFrees( B );

    ///////////////////////////////////////////////////////
    // place B in free table
    InsertFreeBlockIntoTable( B );
}

////////////////////////////////////////////////////////////////////////////
// Searches the free list for a block that contains a chunk NBytes long
// that is aligned to a 2^Alignment boundary.  In the case that the alignment
// is 16 bytes (AlignmentPower==4), a slight shortcut is taken since ANY free 
// block is aligned to 16 bytes after it's control block.

static void FindAlignedMemory( mem_block** SB, byte** Addr, s32 NBytes )
{
    s32        MinimumBlockSize;
    s32        HashIndex;
    byte*      AlignedBlockStartAddr;
    byte*      AlignedBlockEndAddr;
    mem_block* BestBlock;
    byte*      BestStartAddress;
    s32        BestScore;

    ///////////////////////////////////////////////////////
    // Assert on parameters
    ASSERT( SB );
    ASSERT( Addr );
    ASSERT( NBytes > 0 );

    ///////////////////////////////////////////////////////
    // Compute MINIMUM block size to search for
    MinimumBlockSize = (s32)ALIGN_UP( NBytes + PAD + (s32)sizeof(mem_block) );
    ASSERT( IS_ALIGNED(MinimumBlockSize) );

    ///////////////////////////////////////////////////////
    // Decide on hash table index
    HashIndex = CompFreeHash( MinimumBlockSize );

    ///////////////////////////////////////////////////////////////////////
    // Loop through hash table looking for a block.
    ///////////////////////////////////////////////////////////////////////
    NBytes           = (s32)ALIGN_UP( NBytes + PAD );
    BestBlock        = NULL;
    BestStartAddress = 0;
    BestScore        = 0;

    for( ; HashIndex < MEM_HASH_ENTRIES; HashIndex++ )
    {
        mem_block* B = (mem_block*)(s_FreeHead[HashIndex].Next);

        ///////////////////////////////////////////////////////////////////
        // loop through all blocks at this hash index
        ///////////////////////////////////////////////////////////////////
        while( B != (mem_block*)(&s_FreeTail[HashIndex]) )
        {
            ASSERT( B->BlockAddr == (byte*)B );

            ///////////////////////////////////////////////////////////////
            // Determine if block could hold us
            ///////////////////////////////////////////////////////////////
            if( (s32)B->BlockSize >= MinimumBlockSize )
            {
                ///////////////////////////////////////////////////////////
                // Compute possible block boundaries, left justified
                ///////////////////////////////////////////////////////////
                AlignedBlockStartAddr = (byte*)ALIGN_UP((byte*)B + sizeof(mem_block));
                AlignedBlockEndAddr   = AlignedBlockStartAddr + NBytes;

                ///////////////////////////////////////////////////////////
                // Check if boundries are inside of block B
                ///////////////////////////////////////////////////////////
                if( AlignedBlockEndAddr <= B->BlockAddr + B->BlockSize )
                {
                    // Check if this block is smaller than others
                    if( ((s32)B->BlockSize < BestScore) || (BestStartAddress == 0) )
                    {
                        BestScore        = B->BlockSize;
                        BestStartAddress = (AlignedBlockStartAddr);
                        BestBlock        = B;
                    }
                }
            }

            // move on to next block
            B = (mem_block*)(B->LocalList.Next);
        }
    }

    // Report no block found
    *SB     = BestBlock;
    *Addr   = BestStartAddress;
}

////////////////////////////////////////////////////////////////////////////

static void FindAlignedMemoryTop( mem_block** SB, byte** Addr, s32 NBytes )
{
    s32        MinimumBlockSize;
    s32        HashIndex;
    byte*      AlignedBlockStartAddr;
    byte*      AlignedBlockEndAddr;
    mem_block* BestBlock;
    byte*      BestStartAddress;

    ///////////////////////////////////////////////////////
    // Assert on parameters
    ASSERT( SB );
    ASSERT( Addr );
    ASSERT( NBytes > 0 );

    ///////////////////////////////////////////////////////
    // Compute MINIMUM block size to search for
    MinimumBlockSize = (s32)ALIGN_UP(NBytes+PAD) + (s32)sizeof(mem_block);
    ASSERT( IS_ALIGNED(MinimumBlockSize) );

    ///////////////////////////////////////////////////////
    // Decide on hash table index
    //HashIndex = CompFreeHash( MinimumBlockSize );

    ///////////////////////////////////////////////////////////////////////
    // Loop through hash table looking for a block.
    ///////////////////////////////////////////////////////////////////////
    NBytes           = (s32)ALIGN_UP(NBytes+PAD);
    BestBlock        = NULL;
    BestStartAddress = 0;

    for( HashIndex = MEM_HASH_ENTRIES - 1; HashIndex > 0; HashIndex-- )
    {
        mem_block* B = (mem_block*)(s_FreeTail[HashIndex].Prev);

        ///////////////////////////////////////////////////////////////////
        // loop through all blocks at this hash index till we find a block
        ///////////////////////////////////////////////////////////////////
        while( B != (mem_block*)(&s_FreeHead[HashIndex]) && !BestBlock )
        {
            ASSERT(B->BlockAddr == (byte*)B);

            ///////////////////////////////////////////////////////////////
            // Determine if block could hold us
            ///////////////////////////////////////////////////////////////
            if( (s32)(B->BlockSize - sizeof(mem_block)) >= NBytes )
            {
                ///////////////////////////////////////////////////////////
                // Compute possible block boundaries, left justified
                ///////////////////////////////////////////////////////////
                AlignedBlockStartAddr = (byte*)ALIGN_UP((byte*)B + B->BlockSize - NBytes);
                AlignedBlockEndAddr   = AlignedBlockStartAddr + NBytes;

                ///////////////////////////////////////////////////////////
                // Check if boundries are inside of block B
                ///////////////////////////////////////////////////////////
                if( AlignedBlockEndAddr <= B->BlockAddr + B->BlockSize )
                {
                    BestStartAddress = (AlignedBlockStartAddr);
                    BestBlock        = B;
                }
            }

            ////////////////////////////////////////////////////////////////
            // move on to next block
            ////////////////////////////////////////////////////////////////
            B = (mem_block*)(B->LocalList.Prev);
        }

        ////////////////////////////////////////////////////////////////////
        //  We found a block, we are done!
        ////////////////////////////////////////////////////////////////////
        if( BestBlock )
            break;
    }

    // Report no block found
    *SB     = BestBlock;
    *Addr   = BestStartAddress;
}

////////////////////////////////////////////////////////////////////////////

void* x_fn_malloc( s32 NBytes, char* FileName, s32 LineNumber )
{
    ASSERT( s_Initialized );
    ASSERT( NBytes >= 0 );

    void* Ret;

    LOCK_X_MEMORY();

    Ret = x_fn_hmalloc( NBytes, FileName, LineNumber );

    UNLOCK_X_MEMORY();

    return Ret;
}

////////////////////////////////////////////////////////////////////////////

void* x_fn_malloc_top( s32 NBytes, char* FileName, s32 LineNumber )
{
    ASSERT( s_Initialized );
    ASSERT( NBytes >= 0 );

    void* Ret;

    LOCK_X_MEMORY();

    Ret = x_fn_hmalloc_top( NBytes, FileName, LineNumber );

    UNLOCK_X_MEMORY();

    return Ret;
}

////////////////////////////////////////////////////////////////////////////

void* x_fn_calloc( s32 NBytes, char* FileName, s32 LineNumber )
{
    ASSERT( s_Initialized );
    ASSERT( NBytes >= 0 );

    LOCK_X_MEMORY();

    void* pMem = x_fn_hmalloc( NBytes, FileName, LineNumber );

    if( pMem != NULL )
        x_memset( pMem, 0, NBytes );

    UNLOCK_X_MEMORY();

    return (pMem);
}

////////////////////////////////////////////////////////////////////////////

static void* x_fn_hmalloc( s32 NBytes, char* FileName, s32 LineNumber )
{
    mem_block* B;
    byte*      Addr;

    ASSERT( s_Initialized );
    ASSERT( NBytes >= 0 );

#ifdef SLOW_MEMORY
    ASSERT( x_fn_MemSanityCheck() == 0 );
#endif

    // Initialize these pointers
    B = NULL;
    Addr = NULL;

    // Check for allocation of NO memory
    if( NBytes == 0 )
        return ZERO_SIZE_ADDRESS;

    // Search for block 
    FindAlignedMemory( &B, &Addr, NBytes );

    s32 MinSize = DONATE_SIZE;

    // On the PC, we may be allowed to get some system memory...
    if( (B == NULL) && (s_AllowGrowth) )
    {
        s32 i;
        s32 Size = (s32)ALIGN_UP( MAX( NBytes + s_MinUsefulBlockSize*2 + 64, MinSize ) ); //ADD 64 for alignment
        for( i = 0; i < 32; ++i )
            if( s_SysMemPtr[i] == NULL )
                break;

        if( i < 32 )
        {
            // Allocate some system memory.
            if( s_CapGrowth && (( s_SysMemSize + Size ) > s_CapGrowth ))
            {
                Size = s_CapGrowth - s_SysMemSize;
                ASSERTS( Size > 0, "Attempt to allocate memory over the cap" );
            }

            s_SysMemPtr[i] = (byte*)SYSALLOC( (u32)Size );
            if( s_SysMemPtr[i] )
            {
                // Give the memory to our manager.
                s_SysMemSize += Size;
                x_fn_DonateBlock( s_SysMemPtr[i], Size );
                s_NDonations++;

                // Try to find a suitable block now.
                FindAlignedMemory( &B, &Addr, NBytes );
            }
        }
    }


#ifdef DO_HEAP_DUMP_ON_MALLOC_FAIL
    if( B == NULL )
    {
        s32 LockedLine = s_Locked;
        s_Locked = -1;
        x_DumpHeapInfo( "memfail.txt" );
        s_Locked = LockedLine;
    }
#endif

    // Do we have a block?
    if( B == NULL )
        return NULL;

    // Allocate section from the block
    B = AllocateBlock( B, Addr, NBytes, FileName, LineNumber );

#ifdef SLOW_MEMORY
    ASSERT( x_fn_MemSanityCheck() == 0 );
#endif

#ifdef DO_HEAP_DUMP_ON_MALLOC_FAIL
    if( B == NULL )
    {
        s32 LockedLine = s_Locked;
        s_Locked = -1;
        x_DumpHeapInfo( "memfail.txt" );
        s_Locked = LockedLine;
    }
#endif

    // If we came up empty, return NULL
    if( B == NULL )
        return NULL;

    // Return user's address
    return (void*)( ((u32)B) + sizeof(mem_block) );
}

////////////////////////////////////////////////////////////////////////////

static void* x_fn_hmalloc_top( s32 NBytes, char* FileName, s32 LineNumber )
{
    mem_block*  B;
    byte*       Addr;
    s32         MinSize = DONATE_SIZE;
    s32         i;
    s32         Size;

    ASSERT( s_Initialized );
    ASSERT( NBytes >= 0 );

#ifdef SLOW_MEMORY
    ASSERT( x_fn_MemSanityCheck() == 0 );
#endif
    // Initialize these pointers
    B = NULL;
    Addr = NULL;

    // Check for allocation of NO memory
    if( NBytes == 0 )
        return ZERO_SIZE_ADDRESS;

    // Search for block 
    FindAlignedMemoryTop( &B, &Addr, NBytes );

    // On the PC, we may be allowed to get some system memory...
    if( (B == NULL) && (s_AllowGrowth) )
    {
        Size = (s32)ALIGN_UP( MAX( NBytes + s_MinUsefulBlockSize*2 + 64, MinSize ) ); // ADD 64 for alignment

        for( i = 0; i < 32; ++i )
        {
            if( s_SysMemPtr[i] == NULL )
            {
                break;
            }
        }

        if( i < 32 )
        {
            // Allocate some system memory.
            if( s_CapGrowth && (( s_SysMemSize + Size ) > s_CapGrowth ))
            {
                Size = s_CapGrowth - s_SysMemSize;
                ASSERTS( Size > 0, "Attempt to allocate memory over the cap" );
            }

            // Allocate some system memory.
            s_SysMemPtr[i] = (byte*)SYSALLOC( Size );

            if( s_SysMemPtr[i] )
            {
                // Give the memory to our manager.
                s_SysMemSize += Size;
                x_fn_DonateBlock( s_SysMemPtr[i], Size );
                s_NDonations++;

                // Try to find a suitable block now.
                FindAlignedMemoryTop( &B, &Addr, NBytes );
            }
        }
    }

#ifdef DO_HEAP_DUMP_ON_MALLOC_FAIL
    if( B == NULL )
    {
        s32 LockedLine = s_Locked;
        s_Locked = -1;

        x_DumpHeapInfo( "memfail.txt" );
        s_Locked = LockedLine;
    }
#endif

    // Do we have a block?
    if( B == NULL )
        return NULL;

    // Allocate section from the block
    B = AllocateBlock( B, Addr, NBytes, FileName, LineNumber );

#if defined( X_DEBUG ) && defined( SLOW_MEMORY )
    ASSERT( x_fn_MemSanityCheck() == 0 );
#endif

#ifdef DO_HEAP_DUMP_ON_MALLOC_FAIL
    if( B == NULL )
    {
        s32 LockedLine = s_Locked;
        s_Locked = -1;
        x_DumpHeapInfo( "memfail.txt" );
        s_Locked = LockedLine;
    }
#endif

    // If we came up empty, return NULL
    if( B == NULL )
        return NULL;

    // Return user's address
    return (void*)( ((u32)B) + sizeof(mem_block) );
}

////////////////////////////////////////////////////////////////////////////

void x_free( void* Ptr )
{
    ASSERT( s_Initialized );

#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == 0 );
#endif

    // Check for address of block with ZERO size
    if( Ptr == ZERO_SIZE_ADDRESS )
        return;

    // Check for deallocation of a NULL ptr
    if( Ptr == NULL )
        return;


    // Attempt to free the block
    LOCK_X_MEMORY();

    FreeBlock( UserPtrToMemBlock((byte*)Ptr) );

    UNLOCK_X_MEMORY();

#if defined( X_DEBUG ) && defined( SLOW_MEMORY )
    ASSERT( x_MemSanityCheck() == 0 );
#endif
}

////////////////////////////////////////////////////////////////////////////

void* x_fn_realloc( void* Ptr, s32 NewNBytes, char* FileName, s32 LineNumber )
{
    mem_block*  MemBlock;       // block of original memory allocation
    s32         Delta;

    ASSERT( s_Initialized );

#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == 0 );
#endif

    // If the pointer is NULL, or points to a 0 size allocation, just malloc.
    if( (Ptr == NULL) || 
        (Ptr == ZERO_SIZE_ADDRESS) )
    {
        return( x_fn_malloc( NewNBytes, FileName, LineNumber ) );
    }

    // If the pointer is not NULL but the size requested is 0, just free.
    if( (Ptr != NULL) && (NewNBytes == 0) )
    {
        x_free( Ptr );
        return( NULL );
    }

    LOCK_X_MEMORY();

    // Get the control block.
    MemBlock = UserPtrToMemBlock( (byte*)Ptr );
    Delta    = NewNBytes- MemBlock->UserSize;

    // Reduction in size requested?
    if( Delta < 0 )
    {
        void* Ptr = ReallocShrink( MemBlock, NewNBytes );

#ifdef DO_HEAP_DUMP_ON_MALLOC_FAIL
        if( Ptr == NULL )
        {
            s32 LockedLine = s_Locked;
            s_Locked = -1;
            x_DumpHeapInfo( "memfail.txt" );
            s_Locked = LockedLine;
        }
#endif

        UNLOCK_X_MEMORY();
        return( Ptr );
    }

    // Increase in size requested?
    if( Delta > 0 )
    {
        void* Ptr = ReallocGrow( MemBlock, NewNBytes );

#ifdef DO_HEAP_DUMP_ON_MALLOC_FAIL
        if( Ptr == NULL )
        {
            s32 LockedLine = s_Locked;
            s_Locked = -1;
            x_DumpHeapInfo( "memfail.txt" );
            s_Locked = LockedLine;
        }
#endif

        UNLOCK_X_MEMORY();
        return( Ptr );
    }

    UNLOCK_X_MEMORY();

#if defined( X_DEBUG ) && defined( SLOW_MEMORY )
    ASSERT( x_MemSanityCheck() == 0 );
#endif

    return( Ptr );
}

////////////////////////////////////////////////////////////////////////////
// Assumed preconditions:
//  + B is valid.
//  + NewNBytes is less than the original number of bytes.

static void* ReallocShrink( mem_block* B, s32 NewNBytes )
{
    byte*       U;          // points to the user memory
    byte*       P;          // points at end of resized user memory
    byte*       End;        // points to end of original complete block
    mem_block*  NewBlock;
    s32         NewBSize;

    // Set up some useful pointers.
    U        = (byte*)(B+1);
    P        = U + NewNBytes;
    End      = (byte*)(B->BlockAddr + B->BlockSize);
    NewBlock = (mem_block*)ALIGN_UP( P + PAD );

#ifdef X_DEBUG

    // Place the back lining on the reduced size memory block.
    x_memset( P, s_Lining1Byte, PAD );

    // Update heap statistics.
    s_HeapInfo.CurrentMemoryAllocated -= B->UserSize;
    s_HeapInfo.CurrentMemoryAllocated += NewNBytes;

#endif

    // Update heap budget usage.
    s_HeapAmount -= B->UserSize;
    s_HeapAmount += NewNBytes;

    // Update user size in original memory block.
    B->UserSize  = NewNBytes;

    // Is the amount of memory reduction worth any further effort?
    NewBSize = End - (byte*)(NewBlock);
    if( NewBSize < s_MinUsefulBlockSize )
    {
        // Nothing more we can do here.
        return( (void*)U );
    }

    // Update the block size.
    B->BlockSize = (byte*)(NewBlock) - (byte*)(B->BlockAddr);

    // Start setting up a new free block in the excess memory.
    InitBlock( NewBlock, NewBSize, (byte*)NewBlock );

    // Insert the new block in the global list.
    if( B->GlobalNext )
    {
        // There is a 'next' block.  Insert in sequence.
        NewBlock->GlobalNext = B->GlobalNext;
        NewBlock->GlobalPrev = B;
        B->GlobalNext->GlobalPrev = NewBlock;
        B->GlobalNext             = NewBlock;
    }
    else
    {
        // There was no next block.  Add to end of list.
        NewBlock->GlobalNext = NULL;
        NewBlock->GlobalPrev = B;
        B->GlobalNext        = NewBlock;
    }

    // If the next global block is adjacent and free, then merge it in.
    if( (NewBlock->GlobalNext) &&
        (NewBlock->GlobalNext->FreeFlag) &&
        ((byte*)(NewBlock->GlobalNext->BlockAddr) == End) )
    {
        UnlinkLocalBlock( (mem_link*)(NewBlock->GlobalNext) );
        NewBlock->BlockSize += NewBlock->GlobalNext->BlockSize;
        NewBlock->GlobalNext = NewBlock->GlobalNext->GlobalNext;
        if( NewBlock->GlobalNext )
            NewBlock->GlobalNext->GlobalPrev = NewBlock;
    }

    // Add the new free block to the free pool.
    InsertFreeBlockIntoTable( NewBlock );

    // We're finally done.
    return( (void*)U );
}

////////////////////////////////////////////////////////////////////////////
// Assumed preconditions:
//  + B is valid.
//  + NewNBytes is larger than the original number of bytes.

static void* ReallocGrow( mem_block* B, s32 NewNBytes )
{
    // We have three possible cases:
    //  (1)  Can expand into sufficient adjacent free memory.
    //  (2)  Can't (1); allocate new memory and copy old data.
    //  (3)  Can't (2); thus, fail.

    // Attempt case (1):
    {
        // Set up some useful pointers.
        byte*       U;          // points to the user memory
        byte*       P;          // points at end of resized user memory
        mem_block*  Next;       // next memory block after B

        U        = (byte*)(B+1);
        P        = U + NewNBytes;
        Next     = B->GlobalNext;

        // Check for necessary conditions:
        //  + Next exists.
        //  + Next is free.
        //  + Next is adjacent.
        //  + Next is large enough to handle the expansion.

        if( (Next) &&
            (Next->FreeFlag) &&
            ((byte*)(Next->BlockAddr) == 
                (byte*)(B->BlockAddr + B->BlockSize)) &&
            ((byte*)(Next->BlockAddr + Next->BlockSize) > 
                (byte*)ALIGN_UP( P+PAD )) )
        {
            // Looks like we can do case (2).

            // Get a pointer to the end of the Next block.
            byte* End = (byte*)(Next->BlockAddr + Next->BlockSize);

            // Remove the free block from its local list.
            UnlinkLocalBlock( (mem_link*)(Next) );

            // Remove the free block from the global list.
            Next->GlobalPrev->GlobalNext = Next->GlobalNext;
            if( Next->GlobalNext ) 
                Next->GlobalNext->GlobalPrev = Next->GlobalPrev;

            // See if there is enough of Next left over for a new block.
            if( (s32)(End - ALIGN_UP( P+PAD )) >= s_MinUsefulBlockSize )
            {
                mem_block* NewBlock;
                s32        NewBSize;

                // Determine location and size of new free block.
                NewBlock = (mem_block*) ALIGN_UP( P+PAD );
                NewBSize = End - (byte*)NewBlock;

                // Compute and set the size of the adjusted block.
                B->BlockSize = (byte*)(NewBlock) - (byte*)(B->BlockAddr);

                // Start setting up a new free block in the excess memory.
                InitBlock( NewBlock, NewBSize, (byte*)NewBlock );

                // Insert the new block in the global list.
                if( B->GlobalNext )
                {
                    // There is a 'next' block.  Insert in sequence.
                    NewBlock->GlobalNext = B->GlobalNext;
                    NewBlock->GlobalPrev = B;
                    B->GlobalNext->GlobalPrev = NewBlock;
                    B->GlobalNext             = NewBlock;
                }
                else
                {
                    // There was no next block.  Add to end of list.
                    NewBlock->GlobalNext = NULL;
                    NewBlock->GlobalPrev = B;
                    B->GlobalNext        = NewBlock;
                }

                // Add the new free block to the free pool.
                InsertFreeBlockIntoTable( NewBlock );
            }
            else
            {
                // Set the size of the adjusted original block.
                B->BlockSize = (End) - (byte*)(B->BlockAddr);
            }

            #ifdef X_DEBUG
            {
                // Place the back lining on the reduced size memory block.
                x_memset( P, s_Lining1Byte, PAD );

                // Update heap statistics.
                s_HeapInfo.CurrentMemoryAllocated -= B->UserSize;
                s_HeapInfo.CurrentMemoryAllocated += NewNBytes;
                s_HeapInfo.SumMemoryAllocated     -= B->UserSize;
                s_HeapInfo.SumMemoryAllocated     += NewNBytes;
                if( s_HeapInfo.CurrentMemoryAllocated > s_HeapInfo.MaxMemoryAllocated )
                    s_HeapInfo.MaxMemoryAllocated = s_HeapInfo.CurrentMemoryAllocated;
            }
            #endif

            // Update heap budget usage.
            s_HeapAmount -= B->UserSize;
            s_HeapAmount += NewNBytes;
            ASSERT( s_HeapAmount < s_HeapBudget );

            // Update the original memory block.
            B->UserSize = NewNBytes;

            // Return the original user pointer.
            return( (void*)(B+1) );
        }
    }

    // Try for case (2):
    {
        void* NewMem;

        // Temporarily decrease the heap budget usage.
        s_HeapAmount -= B->UserSize;

        // Allocate a new block of the desired size.
        NewMem = x_fn_hmalloc( NewNBytes, "", 0 );

        // Undo the temporary decrease of the heap budget usage.
        s_HeapAmount += B->UserSize;

        // Did we get the requested memory?
        if( NewMem )
        {
            // Yes!  Go ahead and copy the data over.
            x_memcpy( NewMem, (void*)(B+1), B->UserSize );

            #ifdef X_DEBUG
            {
                mem_block*  NewBlock;
                // Copy over debug information.
                NewBlock = UserPtrToMemBlock( (byte*)NewMem );
                NewBlock->LineNumber = B->LineNumber;
                x_memcpy( NewBlock->FileName, B->FileName, SIZE_OF_MEM_BLOCK_FILENAME );
            }
            #endif

            // Release the old allocation.
            FreeBlock( UserPtrToMemBlock((byte*)((void*)(B+1))) );

            // Return the new memory block.
            return( NewMem );
        }
    }

    // We are doomed to case (3):
    return( NULL );
}

////////////////////////////////////////////////////////////////////////////

static s32 x_fn_MemSanityCheck( void )
{
    s32         Error;
    mem_block*  M;

    ASSERT( s_Initialized );

    Error   = 0;
    M       = s_GlobalHead;

    while( M != NULL )
    {
        //////////////////////////////////////////////////
        // Check that block is aligned
        if( IS_ALIGNED(M) == 0 )
        {
            ASSERTS( 0, "MISALIGNED" );
            Error = 1;
            break;
        }

        //////////////////////////////////////////////////
        // Check that global ptrs are intact
        if( (M->GlobalNext != NULL) && (M->GlobalNext->GlobalPrev != M) )
        {
            ASSERTS( 0, "GLOBAL PTRS FRIED" );
            Error = 2;
            break;
        }

        if( (M->GlobalPrev != NULL) && (M->GlobalPrev->GlobalNext != M) )
        {
            ASSERTS( 0, "GLOBAL PTRS FRIED" );
            Error = 3;
            break;
        }

        //////////////////////////////////////////////////
        // Check that local ptrs are intact
        if( ((mem_link*)M)->Next->Prev != (mem_link*)M )
        {
            ASSERTS( 0, "LOCAL PTRS FRIED" );
            Error = 4;
            break;
        }

        if( ((mem_link*)M)->Prev->Next != (mem_link*)M )
        {
            ASSERTS( 0, "LOCAL PTRS FRIED" );
            Error = 5;
            break;
        }

#ifdef X_DEBUG

        //////////////////////////////////////////////////
        // Check liners for overwrite
        if( !M->FreeFlag )
        {
            s32 i;
            u8* LiningPtr;

            if( (M->FrontLining1 != s_Lining4Byte) | (M->FrontLining2 != s_Lining4Byte) )
            {
                ASSERTS( 0, "FRONT LINING OVERWRITTEN" );
                Error = 6;
                break;
            }

            LiningPtr = (u8*)(((u32)M) + sizeof(mem_block) + M->UserSize);
            for( i = 0; i < PAD; i++ )
            {
                if( LiningPtr[i] != s_Lining1Byte )
                {
                    ASSERTS( 0, "REAR LINING OVERWRITTEN" );
                    Error = 7;
                    break;
                }
            }

            if( i < PAD )
                break;
        }

#endif

        //////////////////////////////////////////////////
        // Check that list is in memory address order
        if( (M->GlobalNext != NULL) && ((u32)(M->GlobalNext->BlockAddr) <= (u32)(M->BlockAddr)) )
        {
            ASSERTS( 0, "OUT OF ADDRESS ORDER" );
            Error = 8;
            break;
        }

        if( (M->GlobalPrev != NULL) && ((u32)(M->GlobalPrev->BlockAddr) >= (u32)(M->BlockAddr)) )
        {
            ASSERTS( 0, "OUT OF ADDRESS ORDER" );
            Error = 9;
            break;
        }

        //////////////////////////////////////////////////
        // Check for neighboring free blocks in global list
        if( M->FreeFlag )
        {
            if( M->GlobalNext && M->GlobalNext->FreeFlag )
            {
                if( M->BlockAddr + M->BlockSize == M->GlobalNext->BlockAddr )
                {
                    ASSERTS( 0, "UN-MERGED FREE BLOCKS" );
                    Error = 10;
                    break;
                }
            }

            if( (u32)M->BlockAddr != (u32)M )
            {
                ASSERTS( 0, "FREE BLOCK HEADER WAS OVERRUN" );
                Error = 11;
                break;
            }
        }

        //////////////////////////////////////////////////
        // Move to next block
        M = M->GlobalNext;
    }

    //--------------------------------------------------------
    // Make sure that all the free memory has not been touched
    //--------------------------------------------------------
#ifdef X_DEBUG
    if( Error == 0 )
    {
        mem_block* Blk;

        for( Blk = s_GlobalHead; Blk; Blk = Blk->GlobalNext )
        {
            register byte *pdst;
            register byte *pdstend;
            s32      nbytes;

            // Skip blocks which are not free
            if( Blk->FreeFlag == FALSE )
                continue;

            //------------------------------------
            // Check the memory for our character
            //------------------------------------

            pdst        = (byte*)( Blk->BlockAddr + sizeof(mem_block) );
            nbytes      = Blk->BlockSize - (s32)sizeof(mem_block);
            if( nbytes > 4096 )
                nbytes = 4096;
            pdstend     = pdst + ( nbytes );

            // check starting bytes
            while( (pdst < pdstend) && (((u32)pdst) & 0x03) )
            {
                if( *pdst++ != 0xBE ) 
                {
                    ASSERTS( 0, "FREE MEMORY OVERRUN" );
                    Error = 12;
                }
            }

            // check 4bytes at a time
            while( (pdst+4 < pdstend) )
            {
                if( *((u32*)pdst) != 0xBEBEBEBE )
                {
                    ASSERTS( 0, "FREE MEMORY OVERRUN" );
                    Error = 12;
                }
                pdst += 4;
            }

            // check ending bytes
            while( pdst < pdstend )
            {
                if( *pdst++ != 0xBE )
                {
                    ASSERTS( 0, "FREE MEMORY OVERRUN" );
                    Error = 12;
                }
            }
        }
    }
#endif

    ASSERTS( Error !=  1, "MISALIGNED" );
    ASSERTS( Error !=  2, "GLOBAL PTRS FRIED" );
    ASSERTS( Error !=  3, "GLOBAL PTRS FRIED" );
    ASSERTS( Error !=  4, "LOCAL PTRS FRIED" );
    ASSERTS( Error !=  5, "LOCAL PTRS FRIED" );
    ASSERTS( Error !=  6, "FRONT LINING OVERWRITTEN" );
    ASSERTS( Error !=  7, "REAR LINING OVERWRITTEN" );
    ASSERTS( Error !=  8, "OUT OF ADDRESS ORDER" );
    ASSERTS( Error !=  9, "OUT OF ADDRESS ORDER" );
    ASSERTS( Error != 10, "UN-MERGED FREE BLOCKS" );
    ASSERTS( Error != 11, "FREE BLOCK HEADER WAS OVERRUN" );
    ASSERTS( Error != 12, "FREE MEMORY OVERRUN" );
    ASSERT ( Error ==  0 );

    return( Error );
}

////////////////////////////////////////////////////////////////////////////

s32 x_MemSanityCheck( void )
{
    s32 Error = 0;

    LOCK_X_MEMORY();

    Error = x_fn_MemSanityCheck();

    UNLOCK_X_MEMORY();

    return Error;
}

////////////////////////////////////////////////////////////////////////////

#if 0 // X_DEBUG

void x_PaintBSS( u64 PaintValue )
{
#ifdef TARGET_PS2
    //### CHANGE THIS FUNCTION TO USE NEW 2.34 TOOLS(i.e. __bss_obj no longer exists)
    extern void *_end, *_stack_size;

    u64*            pBSS    = (u64*)__bss_obj;
    u64*            pBSSEnd = (u64*)__bss_objend;
    u64*            pRAM;
    s32             RoundOffBytes;
    s32             Size;

    //--- if the heap hasn't been capped, there is nothing to paint
    if( !s_CapGrowth )
        return;

    //--- get the size of the void
    Size = (u32)pBSSEnd - (u32)pBSS;
    pRAM = pBSS;

    //--- round off the process
    RoundOffBytes = (u32)pRAM % sizeof(u64);
    if( RoundOffBytes )
    {
        RoundOffBytes = sizeof(u64) - RoundOffBytes;
        x_memcpy( pRAM, &PaintValue, RoundOffBytes );
        pRAM = (u64*) (((u32)pRAM) + RoundOffBytes);
    }

    //--- round off the process
    RoundOffBytes = (u32)pBSSEnd % sizeof(u64);
    if( RoundOffBytes )
        pBSSEnd = (u64*)(((u32)pBSSEnd) - RoundOffBytes);

    //--- fill the void space with a value
    while( pRAM != (u64*)pBSSEnd )
        *pRAM++ = PaintValue;
#endif
}

////////////////////////////////////////////////////////////////////////////
//
//  x_PaintVoid paints the regean of memory above the heap and below the stack.
//  this region only exists when a dev kit provides more RAM than a target
//  system, and a programmer wants to limit the dev kit to that target platform.
//
void x_PaintVoid( u64 PaintValue )
{
#ifdef TARGET_PS2
    //### CHANGE THIS FUNCTION TO USE NEW 2.34 TOOLS(i.e. __bss_obj no longer exists)
    extern void *_end, *_stack_size;

    u64*            pBSSEnd = (u64*)__bss_objend;
    u32             EndSP;
    u64*            pRAM;
    s32             RoundOffBytes;
    s32             Size;

    //--- if the heap hasn't been capped, there is nothing to paint
    if( !s_CapGrowth )
        return;

    //--- Get the top of the stack
    EndSP = s_MaxRAM - s_StackSize;
    ASSERT( !(EndSP % sizeof(u64)) );

    //--- get the size of the void
    Size = EndSP - (u32)pBSSEnd;
    if( Size <= s_CapGrowth )
        return;

    //--- size now contains the size of the void
    Size -= s_CapGrowth;
    pRAM = pBSSEnd;
    pRAM = (u64*)(((u32)pBSSEnd) + s_CapGrowth);

    //--- round off the process
    RoundOffBytes = (u32)pRAM % sizeof(u64);
    if( RoundOffBytes )
    {
        RoundOffBytes = sizeof(u64) - RoundOffBytes;
        x_memcpy( pRAM, &PaintValue, RoundOffBytes );
        pRAM = (u64*)(((u32)pRAM) + RoundOffBytes);
    }

    //--- fill the void space with a value
    while( pRAM != (u64*)EndSP )
        *pRAM++ = PaintValue;
#endif
}
#endif

////////////////////////////////////////////////////////////////////////////
//
//  Be careful modifying this function.  You don't want to be changing the stack
//  while using this function.
//
////////////////////////////////////////////////////////////////////////////
void x_PaintStack( u64 PaintValue )
{
#ifdef TARGET_PS2

    volatile s32    sp[100];
    static u64      CurSP;
    static u64      EndSP;
    static u64*     pRAM;

    //--- Get the stack bounds. 
    //    NOTE: we cannot work with it above its current location
    CurSP = (s32)&sp[0];
    EndSP = s_MaxRAM - s_StackSize;

    if( CurSP&0x7 )
        CurSP += sizeof(u64) - (CurSP & 0x7);
    ASSERT( !(CurSP & 0x7) );

    //--- fill the stack space with a value
    pRAM = (u64*)EndSP;
    while( pRAM != (u64*)CurSP )
        *pRAM++ = PaintValue;

#endif
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS FOR DEALING WITH THE STACK
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#ifdef STACK_CHECK

////////////////////////////////////////////////////////////////////////////

u32 x_StackCheckSize( void )
{
    return s_StackSize;
}

////////////////////////////////////////////////////////////////////////////
//  x_StackCheckInit-
//  Paints the stack to initialize the stack check system.  Should be called
//  near the start of the program in a function which is close to the bottom
//  of expected stack growth.

void x_StackCheckInit( void )
{
#ifndef __MWERKS__
    x_PaintStack( (u64)0xDDDDDDDDDDDDDDDDuLL );
#endif

#ifdef TARGET_PS2
    //--- paint the RAM above the PS2 RAM max and below the stack on the dev-tool
    //x_PaintVoid( (u64)0xEEEEEEEEEEEEEEEEEuLL );
#endif

}

////////////////////////////////////////////////////////////////////////////
//  x_StackCheckUsed-
//  returns the current amount of the stack used.  This function has no way
//  to detect if the stack has been overwritten. If the amount used
//  get close to the total amount, a warning signal or ASSERT should be hit
//  to suggest a stack size increase.

u32 x_StackCheckUsed( void )
{
    u32 UsedSize;
    u32* pRAM;

    pRAM = (u32*)(s_MaxRAM - s_StackSize);

    while( (u32)pRAM < s_MaxRAM )
        if( *pRAM++ != 0xDDDDDDDD )
            break;

    UsedSize = s_MaxRAM - (u32)pRAM + 4;

    return UsedSize;
}

////////////////////////////////////////////////////////////////////////////
//  x_StackCheckLeft-
//  This function returns the number of bytes in the stack which have been
//  currently unused.
//
//  NOTE: This function cannot detect if a stack has overflowed.  The lower
//  the size of unused stack gets, the greater likelyhood that it has or will
//  overflow.
//
//  returns the amount of stack left

u32 x_StackCheckLeft( void )
{
    u32 UsedSize = x_StackCheckUsed();
    return s_StackSize - UsedSize;
}

////////////////////////////////////////////////////////////////////////////
//  x_StackCheck-
//  This function can be called regularly to check for the stack to have
//  shrunk dangerously low.  It doesn't have to be called in a function
//  which uses much stack, but it should be called regularly in the game loop.
//
//  returns the amount of stack left

u32 x_StackCheck( void )
{
    u32 StackLeft = x_StackCheckLeft();
    ASSERTS( StackLeft > 1024, "WARNING: Stack used to within 1K of its limit" );
    return StackLeft;
}

#else

void x_StackCheckInit( void )   {}
u32  x_StackCheckSize( void )   { return s_StackSize; }
u32  x_StackCheckUsed( void )   { return 0; }
u32  x_StackCheckLeft( void )   { return s_StackSize; }
u32  x_StackCheck    ( void )   { return s_StackSize; }

#endif // STACK_CHECK


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//  MEMORY FRAGMENTATION INFO
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void x_GetFragmentationInfo( xmem_fraginfo* pFI )
{
    mem_block* Blk;

    ASSERT( s_Initialized );
    ASSERT( pFI );

    x_memset( pFI, 0, sizeof(xmem_fraginfo) );

    for( Blk = s_GlobalHead; Blk; Blk = Blk->GlobalNext )
    {
        if( Blk->FreeFlag == TRUE )
        {
            s32 Idx = 0;
            s32 Size = Blk->BlockSize;
            while( Size > 0 )
            {
                Idx++;
                Size >>= 1;
            }
            pFI->Pow2Range[ Idx ] ++;
            pFI->FreeSpace[ Idx ] += Blk->BlockSize;
        }
    }
}

////////////////////////////////////////////////////////////////////////////

#define FRAG_SMALL_SIDE_DEFINITION          128

void x_DumpFragmentationInfo( const char* Filename )
{
    ASSERT( s_Initialized );

    if( Filename == NULL )
        Filename = "memfrag.txt";

    X_FILE* pFile;
    pFile = x_fopen( Filename, "w" );
    if( pFile == NULL )
        return;

    xmem_fraginfo FI;

    x_GetFragmentationInfo( &FI );

    s32 i;
    s32 Size = 1;

    for( i = 0; i < 26; i++, Size <<= 1 )
    {
        x_fprintf( pFile, "%10ld = %8ld", Size, FI.Pow2Range[i] );
        if( FI.Pow2Range[i] )
        {
            x_fprintf( pFile, "       [ %10ld ]\n",FI.FreeSpace[i] );
        }
        else
        {
            x_fprintf( pFile, "\n" );
        }
    }

    mem_block*  Blk;
    s32         Counter[FRAG_SMALL_SIDE_DEFINITION];

    x_memset( Counter, 0, FRAG_SMALL_SIDE_DEFINITION * 4 );

    for( Blk = s_GlobalHead; Blk; Blk = Blk->GlobalNext )
    {
        if( !Blk->FreeFlag )
        {
            if( Blk->UserSize < FRAG_SMALL_SIDE_DEFINITION )
            {
                Counter[Blk->UserSize]++;
            }
        }
    }

    x_fprintf( pFile, "\n\nSmall Allocation Count\n----------------------\n\n" );
    for( i = 0; i < FRAG_SMALL_SIDE_DEFINITION; i++ )
    {
        if( Counter[i] > 0 )
            x_fprintf( pFile, "%4ld  : %ld\n", i, Counter[i] );
    }
    x_fprintf( pFile, "\n\nDone\n" );

    x_fclose( pFile );
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS FOR C++   new  new[]  delete  delete[]
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#ifndef USE_NATIVE_NEW_AND_DELETE

#ifdef new
#undef new
#endif

//==========================================================================

void* operator new( x_size_t Size )
{
    return( x_malloc( (s32)Size ) );
}

//==========================================================================

void* operator new[] ( x_size_t Size )
{
    return( x_malloc( (s32)Size ) );
}

//==========================================================================

void* operator new( x_size_t Size, char* pFileName, s32 LineNumber )
{
    return( x_fn_malloc( (s32)Size, pFileName, LineNumber ) );
}

//==========================================================================

void operator delete( void* pMemory )
{
    x_free( pMemory );
}

//==========================================================================

void operator delete( void* pMemory, char*, s32 )
{
    x_free( pMemory );
}

//==========================================================================

void* operator new[] ( x_size_t Size, char* pFileName, s32 LineNumber )
{
    return( x_fn_malloc( (s32)Size, pFileName, LineNumber ) );
}

//==========================================================================

void operator delete[] ( void* pMemory )
{
    x_free( pMemory );
}

//==========================================================================

void operator delete[] ( void* pMemory, char*, s32 )
{
    x_free( pMemory );
}

//==========================================================================

void* operator new( x_size_t /*Size*/, x_PlacementNewDummy* pMemory )
{
    return( reinterpret_cast<void*>(pMemory) );
}

//==========================================================================

void operator delete( void* pMemory, x_PlacementNewDummy* pND )
{

}

////////////////////////////////////////////////////////////////////////////

#else

void* operator new( x_size_t /*Size*/, x_PlacementNewDummy* pMemory )
{
    return( reinterpret_cast<void*>(pMemory) );
}

//==========================================================================

void operator delete( void* pMemory, x_PlacementNewDummy* pND )
{

}

#endif // USE_NATIVE_NEW_AND_DELETE

////////////////////////////////////////////////////////////////////////////
