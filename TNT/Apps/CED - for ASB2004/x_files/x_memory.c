///////////////////////////////////////////////////////////////////////////
//
//  X_MEMORY.C
//
///////////////////////////////////////////////////////////////////////////

#ifndef X_DEBUG_H
#include "x_debug.h"
#endif

#ifndef X_MEMORY_H
#include "x_memory.h"
#endif

#ifndef X_PLUS_H
#include "x_plus.h"
#endif

#ifndef X_STDIO_H
#include "x_stdio.h"
#endif

#ifndef X_TARGET_H
#include "x_target.h"
#endif

///////////////////////////////////////////////////////////////////////////

#if defined(TARGET_PC) || defined(TARGET_SDC)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#endif

///////////////////////////////////////////////////////////////////////////
// MODULE DEBUG OPTIONS
///////////////////////////////////////////////////////////////////////////

#define DO_HEAP_DUMP_ON_MALLOC_FAIL

///////////////////////////////////////////////////////////////////////////
//  DEFINES
///////////////////////////////////////////////////////////////////////////

#define MIN_SIZE_FOR_SMALLS     (16*1024)

#define MEM_MIN_FREE_POWER      0                           // 1       byte
#define MEM_MAX_FREE_POWER      20                          // 1048576 bytes
#define MEM_HASH_ENTRIES        (MEM_MAX_FREE_POWER+1)

#define MEM_MAX_HEAPS           8                           // num heaps available
#define MIN_DONATED_BLOCK_SIZE  1024                        // 16 * 64

#define ALIGN_UP(x)             ((((u32)(x))+15)&0xFFFFFFF0)
#define ALIGN_DOWN(x)           (((u32)(x))&0xFFFFFFF0)
#define IS_ALIGNED(x)           (((u32)(x) &0x0000000F)?(0):(1))
#define ZERO_SIZE_ADDRESS       ((void*)(0xFFFFFFF0))

#ifdef X_DEBUG
    #define PAD  16
#else
    #define PAD   0
#endif

///////////////////////////////////////////////////////////////////////////
//  STRUCTURES
///////////////////////////////////////////////////////////////////////////

typedef struct s_mem_link       
{
    struct s_mem_link* Next;
    struct s_mem_link* Prev;
} mem_link;                     

///////////////////////////////////////////////////////////////////////////

typedef struct s_mem_block
{
    ////////////////////////////////////
    // info for release and debug
    ////////////////////////////////////
    mem_link                LocalList;      // Links in Free or Heap list.
    struct s_mem_block*     GlobalNext;     // Next ptr in global list.
    struct s_mem_block*     GlobalPrev;     // Prev ptr in global list.
    u32                     BlockAddr;      // Physical address of block 
                                            // beginning. This is not necessarily
                                            // the beginning of the mem_block.
    u32                     BlockSize;      // Physical size of block from
                                            // BlockAddr to end of the block.
                                            // includes ALL memory involved in
                                            // block.
    u32                     UserSize;       // Physical size of memory requested
                                            // by user.
    u16                     FreeFlag;       // Block is free if FreeFlag == 1
    u16                     HeapID;         // If block is allocated, it belongs
                                            // in one of the heaps.  This is the
                                            // index to the correct heap.

    ////////////////////////////////////
    // info for debug only
    ////////////////////////////////////
#ifdef X_DEBUG
    u32                     Sequence;       // number of allocs before this block
    u32                     LineNumber;     // *.c line number of malloc call
    char                    FileName[16];   // *.c name
    u32                     FrontLining1;   // front lining
    u32                     FrontLining2;   // front lining
#endif    

} mem_block; // 32 bytes release, 64 bytes debug

///////////////////////////////////////////////////////////////////////////

#ifdef X_DEBUG
typedef struct 
{
    s32 NumAllocs;
    s32 NumFrees;
    s32 CurrentMemoryAllocated;
    s32 MaxMemoryAllocated;
    s32 SumMemoryAllocated;
} heap_info;
#endif

///////////////////////////////////////////////////////////////////////////
//  GLOBALS
///////////////////////////////////////////////////////////////////////////

static u8           Initialized = FALSE;
static u8           AllowGrowth = FALSE;

// Global list organization
static mem_block*   GlobalHead;

// Free memory table
static mem_link     FreeHead    [MEM_HASH_ENTRIES];   // Contains blocksize >= (2^i) 
static mem_link     FreeTail    [MEM_HASH_ENTRIES];

// Heap organization
static mem_link     HeapHead    [MEM_MAX_HEAPS];
static mem_link     HeapTail    [MEM_MAX_HEAPS];
static s32          HeapBudget  [MEM_MAX_HEAPS];
static s32          HeapAmount  [MEM_MAX_HEAPS];

// Statistics
static s32          NumAllocs;
static s32          NumFrees;
static s32          MinUsefulBlockSize;


#ifdef X_DEBUG
static u32          Lining4Byte = 0x21212121;           // '!!!!'
static u8           Lining1Byte = 0x21;                 // '!'
static heap_info    HeapInfo[MEM_MAX_HEAPS];
#endif

#if defined(TARGET_PC) || defined(TARGET_SDC)
#define PC_MEM_CHUNKS 256
static void* PCMemPtr[PC_MEM_CHUNKS] = {0};
#endif

///////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
///////////////////////////////////////////////////////////////////////////

mem_block*  UserPtrToMemBlock          ( void* UserPtr );
void        InitBlock                  ( mem_block* B, s32 BlockSize, u32 BlockAddr );
void        LinkLocalBlock             ( mem_link* Parent, mem_link* Block );
void        UnlinkLocalBlock           ( mem_link* Block );
s32         CompFreeHash               ( s32 NBytes );
void        InsertFreeBlockIntoTable   ( mem_block* B );
void        FreeBlock                  ( mem_block* B );
void        FindAlignedMemory          ( mem_block** SB, 
                                         u32*        Addr, 
                                         s32         NBytes, 
                                         s32         AlignmentPower );
mem_block*  AllocateBlock              ( mem_block* SB, 
                                         u32        UserAddr, 
                                         s32        UserSize, 
                                         s32        HeapID, 
                                         char*      FileName, 
                                         s32        LineNumber );

void*       ReallocShrink              ( mem_block* B, s32 NewNBytes );
void*       ReallocGrow                ( mem_block* B, s32 NewNBytes );
mem_block*  MergeNeighboringFrees      ( mem_block* B );

#ifdef X_DEBUG

void        SetLining                  ( mem_block* B );
u8          LiningCorrupt              ( mem_block* B );

#endif

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//  PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

mem_block*  UserPtrToMemBlock( void* UserPtr )
{
    mem_block* B;

    ASSERT( UserPtr );
    B = ((mem_block*)(UserPtr)) - 1;

    ///////////////////////////////////////////////////////
    // Verify that this is an actual in-use memory block
    ASSERT( !B->FreeFlag );
    ASSERT( ((mem_link*)B)->Next->Prev == ((mem_link*)B) );
    ASSERT( ((mem_link*)B)->Prev->Next == ((mem_link*)B) );
    
#ifdef X_DEBUG

    ///////////////////////////////////////////////////////
    // Check linings
    {
        s32 i;
        u8* LiningPtr;

        ASSERT( (B->FrontLining1 == Lining4Byte) && (B->FrontLining2 == Lining4Byte) );

        LiningPtr = (u8*)(((u32)B) + sizeof(mem_block) + B->UserSize);
        for (i=0; i<PAD; i++)
            ASSERT(LiningPtr[i] == Lining1Byte);
    }

#endif

    return( B );
}

///////////////////////////////////////////////////////////////////////////

void InitBlock      ( mem_block* B, s32 BlockSize, u32 BlockAddr ) 
{
    ASSERT(B);
    ASSERT(BlockSize > 0);
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
    B->Sequence         = 0;
    B->LineNumber       = 0;
    B->FrontLining1     = Lining4Byte;
    B->FrontLining2     = Lining4Byte;
    x_strncpy((B->FileName),"FREE",16);
#endif
}

///////////////////////////////////////////////////////////////////////////

void LinkLocalBlock      ( mem_link* Parent, mem_link* Block )
{
    ASSERT(Parent);
    ASSERT(Block);
    ASSERT(Parent->Next);
    ASSERT(Parent->Next->Prev);
    Block->Prev       = Parent;
    Block->Next       = Parent->Next;
    Block->Prev->Next = Block;
    Block->Next->Prev = Block;
}

///////////////////////////////////////////////////////////////////////////

void UnlinkLocalBlock    ( mem_link* Block )
{
    ASSERT(Block);
    ASSERT(Block->Prev);
    ASSERT(Block->Next);
    ASSERT(Block->Prev->Next == Block);
    ASSERT(Block->Next->Prev == Block);
    Block->Prev->Next   = Block->Next;
    Block->Next->Prev   = Block->Prev;
    Block->Next         = NULL;
    Block->Prev         = NULL;
}

///////////////////////////////////////////////////////////////////////////

s32  CompFreeHash   ( s32 NBytes )
{
    s32 Index;
    ASSERT(NBytes > 0);

    Index    = MEM_MIN_FREE_POWER;
    NBytes >>= (MEM_MIN_FREE_POWER+1);

    while ((NBytes > 0) && (Index<MEM_MAX_FREE_POWER))
    {
        NBytes >>= 1;
        Index++;
    }

    return Index;
}

///////////////////////////////////////////////////////////////////////////

void InsertFreeBlockIntoTable   ( mem_block* B )
{
    s32 HashIndex;
    ASSERT(B);

    // Assuming block has been initialized and is already
    // in the global list.  By now, mem_block should be at
    // the beginning of the block meaning BlockAddr == B
    ASSERT(B->BlockAddr == (u32)B);

    // Insert block into free hash table
    HashIndex = CompFreeHash( B->BlockSize );
    LinkLocalBlock( &FreeHead[HashIndex], &(B->LocalList) );

    //-----------------------------------------------------
    // Clear the allocated storage
    //-----------------------------------------------------
#ifdef X_DEBUG
    {
        s32 NBytes;
        NBytes = (B->BlockSize - sizeof(mem_block));
        if (NBytes > 4096) NBytes = 4096;
        x_memset( (void*)( B->BlockAddr + sizeof(mem_block) ), 
                  0xBE, 
                  NBytes );
    }
#endif
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//  PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void  x_MemoryInit      ( void )
{
    s32 i;

    ASSERT(!Initialized);
    //ASSERT(IS_ALIGNED(sizeof(mem_block)));

    //////////////////////////////////////
    // clear heaps
    for (i=0; i<MEM_MAX_HEAPS; i++)
    {
        HeapBudget[i]       = 0;
        HeapAmount[i]       = 0;
        HeapHead[i].Next    = &HeapTail[i];
        HeapHead[i].Prev    = NULL;
        HeapTail[i].Next    = NULL;
        HeapTail[i].Prev    = &HeapHead[i];

#ifdef X_DEBUG
        HeapInfo[i].NumAllocs               = 0;
        HeapInfo[i].NumFrees                = 0;
        HeapInfo[i].CurrentMemoryAllocated  = 0;
        HeapInfo[i].MaxMemoryAllocated      = 0;
        HeapInfo[i].SumMemoryAllocated      = 0;
#endif
    }
    HeapBudget[0] = (s32)0x7FFFFFFF;

    //////////////////////////////////////
    // clear free lists
    for (i=0; i<MEM_HASH_ENTRIES; i++)
    {
        FreeHead[i].Next = &FreeTail[i];
        FreeHead[i].Prev = NULL;
        FreeTail[i].Next = NULL;
        FreeTail[i].Prev = &FreeHead[i];
    }

    //////////////////////////////////////
    // clear global list and statistics
    MinUsefulBlockSize = sizeof(mem_block);
    GlobalHead  = NULL;
    NumAllocs   = 0;
    NumFrees    = 0;

    //////////////////////////////////////
    // mark manager as initialized
    Initialized = TRUE;
    AllowGrowth = TRUE;

#ifdef X_DEBUG
#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == ERR_SUCCESS );
#endif
#endif
}

///////////////////////////////////////////////////////////////////////////

void  x_MemoryKill      ( void )
{
    ASSERT(Initialized);

#if defined(TARGET_PC) || defined(TARGET_SDC)
    {
        s32 i;
        for( i=0; i<PC_MEM_CHUNKS; ++i )
            if( PCMemPtr[i] != NULL )
                free( PCMemPtr[i] );
    }
#endif

    Initialized = 0;
}

///////////////////////////////////////////////////////////////////////////

void  x_DonateBlock     ( u32 aBlockAddr, s32 NBytes )
{
    u32 LowBlockAddr;
    u32 HighBlockAddr;
    s32 AlignedBlockSize;
    void* BlockAddr;
    mem_block* Block;

    ASSERT(Initialized);
    ASSERT(aBlockAddr);
    ASSERT(NBytes > 0);
    BlockAddr = (void*)aBlockAddr;

    //////////////////////////////////////////////////
    // Align block to correct boundries
    LowBlockAddr     = ALIGN_UP(BlockAddr);
    HighBlockAddr    = ALIGN_DOWN(((u32)BlockAddr) + NBytes);
    AlignedBlockSize = (HighBlockAddr - LowBlockAddr);
    
    //////////////////////////////////////////////////
    // Be sure block is worth dealing with
    if (AlignedBlockSize < MIN_DONATED_BLOCK_SIZE) return;
    if (AlignedBlockSize < MinUsefulBlockSize)  return;
    
    //////////////////////////////////////////////////
    // Create block and add to free list
    Block = (mem_block*)LowBlockAddr;
    InitBlock(Block,AlignedBlockSize,LowBlockAddr);

    //////////////////////////////////////////////////
    // Sort block in global list by address
    if (GlobalHead==NULL)
        GlobalHead = Block;
    else
    if ((u32)Block < (u32)(GlobalHead))
    {
        Block->GlobalNext = GlobalHead;
        GlobalHead->GlobalPrev = Block;
        GlobalHead = Block;
    }
    else
    {
        mem_block* B = GlobalHead;
        while (B)
        {
            if ( (u32)Block < (u32)B )
            {
                Block->GlobalNext = B;
                Block->GlobalPrev = B->GlobalPrev;
                Block->GlobalPrev->GlobalNext = Block;
                Block->GlobalNext->GlobalPrev = Block;
                break;
            }
            
            if ( B->GlobalNext == NULL )
            {
                B->GlobalNext = Block;
                Block->GlobalPrev = B;
                break;
            }

            B = B->GlobalNext;
        }
    }

    Block = MergeNeighboringFrees( Block );

    InsertFreeBlockIntoTable(Block);

#ifdef X_DEBUG
#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == ERR_SUCCESS );
#endif
#endif
}

///////////////////////////////////////////////////////////////////////////

s32 x_CreateHeap( void )
{
    s32  HeapID;
    
    ASSERT( Initialized );
    
    // Find an available virtual heap.
    for ( HeapID = 1; HeapID < MEM_MAX_HEAPS; ++HeapID )
    {
        if ( HeapBudget[HeapID] == 0 )
        {
            HeapBudget[HeapID] = 0x40000000;  // 1G (unlimitted)
            return HeapID;
        }
    }

    ASSERT( FALSE );
    return( -1 );
}

///////////////////////////////////////////////////////////////////////////
#if 0
void x_DestroyHeap( s32 HeapID )
{
    ASSERT( Initialized );
    ASSERT( (HeapID > 0) && (HeapID < MEM_MAX_HEAPS) );
    ASSERT( HeapBudget[HeapID] > 0 );
    
    x_ClearHeap( HeapID );    
    HeapBudget[HeapID] = 0;
}
#endif
///////////////////////////////////////////////////////////////////////////
#if 0
void    x_SetHeapSize     ( s32 HeapID, s32 NBytes )
{
    ASSERT( Initialized );
    ASSERT( (HeapID > 0) && (HeapID < MEM_MAX_HEAPS) );
    ASSERT( HeapBudget[HeapID] > 0 );
    ASSERT( NBytes > 0 );

    // Not allowed to set heap size LESS than current consumption.    
    ASSERT( HeapAmount[HeapID] <= NBytes );

    // Set new heap size.
    HeapBudget[HeapID] = NBytes;
}
#endif
///////////////////////////////////////////////////////////////////////////
#if 0
s32     x_GetHeapSize     ( s32 HeapID )
{
    ASSERT(Initialized);
    ASSERT(HeapID>0 && HeapID<MEM_MAX_HEAPS);
    return HeapBudget[HeapID];
}
#endif
///////////////////////////////////////////////////////////////////////////
#if 0
void x_ClearHeap( s32 HeapID )
{
    mem_block* Blk;

    ASSERT(Initialized);
    ASSERT(HeapID>0 && HeapID<MEM_MAX_HEAPS);

    //-------------------------------------------------------
    // Go thru the global list and remove all the Heap Nodes
    //-------------------------------------------------------
    for ( Blk = GlobalHead; Blk;  )
    {        
        if (( Blk->FreeFlag ) || ( Blk->HeapID != HeapID ))
        {
            // Go to the next node
            Blk = Blk->GlobalNext;
            continue;
        }

        //--------------------------
        // Delete Node
        //--------------------------
        // Are we the first node in the list?
        if ( Blk->GlobalPrev  == NULL )
        {
            ASSERT(Blk == GlobalHead);

            // FreeBlock should have set the new head
            FreeBlock( Blk );

            // Start fresh
            Blk = GlobalHead;
        }
        else
        {
            mem_block* T;

            // Keep the previous node
            T = Blk->GlobalPrev;

            // FreeBlock Should fix the previouos node
            FreeBlock( Blk );

            // Set it to the previous node
            Blk = T;
        }
    }

////////////////////////////////////////////////////////
///////////////////// OLD WAY //////////////////////////
////////////////////////////////////////////////////////
#if 0

#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == ERR_SUCCESS );
#endif
    
    while (HeapHead[HeapID].Next != &HeapTail[HeapID])
    {
        FreeBlock( (mem_block*)(HeapHead[HeapID].Next) );
    }

#ifdef X_DEBUG
#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == ERR_SUCCESS );
#endif
#endif

#endif

}
#endif


//============================================================
// x_TotalMemory
//============================================================
s32 x_TotalMemory( void )
{
    mem_block* Blk;
    s32        Total = 0;

    ASSERT(Initialized);

    for ( Blk = GlobalHead; Blk; Blk = Blk->GlobalNext )
    {
        if ( Blk->FreeFlag == FALSE )
            Total += Blk->BlockSize;
    }

    return Total;
}

//============================================================
// x_TotalMemoryFree
//============================================================
s32 x_TotalMemoryFree( void )
{
    mem_block* Blk;
    s32        Total = 0;

    ASSERT(Initialized);

    for ( Blk = GlobalHead; Blk; Blk = Blk->GlobalNext )
    {
        if ( Blk->FreeFlag == TRUE )
            Total += Blk->BlockSize;
    }

    return Total;
}

//============================================================
// x_LargestBlockFree
//============================================================
s32 x_LargestBlockFree( void )
{
    mem_block* Blk;
    s32        Largest = 0;

    ASSERT(Initialized);

    for ( Blk = GlobalHead; Blk; Blk = Blk->GlobalNext )
    {
        if ( Blk->FreeFlag == TRUE )
        {
            if (Blk->BlockSize > (u32)Largest)
                Largest = Blk->BlockSize;
        }
    }

    return Largest;
}

//============================================================
// x_SizeOfBlock
//============================================================
u32 x_SizeOfBlock( void* Ptr )
{
    mem_block* MemBlock;

    ASSERT(Initialized);
    ASSERT( Ptr );

#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == ERR_SUCCESS );
#endif

    // Check for address of block with ZERO size
    if (Ptr == ZERO_SIZE_ADDRESS) return 0;

    // Attempt to free the block
    MemBlock = UserPtrToMemBlock(Ptr);

    // Return the Size of the block
    return MemBlock->BlockSize;     
}

//============================================================
// x_SizeOfUserBlock
//============================================================
u32 x_SizeOfUserBlock( void* Ptr )
{
    mem_block* MemBlock;

    ASSERT(Initialized);
    ASSERT( Ptr );

#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == ERR_SUCCESS );
#endif

    // Check for address of block with ZERO size
    if (Ptr == ZERO_SIZE_ADDRESS) return 0;

    // Attempt to free the block
    MemBlock = UserPtrToMemBlock(Ptr);

    // Return the Size of the block
    return MemBlock->UserSize;     
}

///////////////////////////////////////////////////////////////////////////

s32   x_TotalAllocatedBlocks    ( void )
{
    ASSERT(Initialized);
    return NumAllocs;
}

///////////////////////////////////////////////////////////////////////////

s32   x_TotalFreedBlocks        ( void )
{
    ASSERT(Initialized);
    return NumFrees;
}

///////////////////////////////////////////////////////////////////////////

void  x_DumpHeapInfo ( char* FileName )
{
    u32         NextMemBlock=0;
    s32         Count;
    mem_block*  M;
    X_FILE*     fp;

    ASSERT(Initialized);

    // Open file to write to
    fp = x_fopen(FileName,"wt");
    ASSERT(fp);

    Count = 0;
    M = GlobalHead;

    x_fprintf(fp,"\n");
    x_fprintf(fp,"B: Break   F: Front waste   R: Rear waste   HI: Heap Index\n");
#ifdef X_DEBUG
    x_fprintf(fp,"NA: Number of Allocs   AS: Average size\n");
    x_fprintf(fp,"NF: Number of Frees    MS: Maximum size\n");
#endif

    x_fprintf(fp,"---------------------------------------------------------------------------------------------\n");
    x_fprintf(fp,"\n");
    x_fprintf(fp,"Total Allocated Memory : %d \n", x_TotalMemory() );
    x_fprintf(fp,"Total Free Memory      : %d \n", x_TotalMemoryFree() );

    x_fprintf(fp,"\n");
    x_fprintf(fp,"                       /----------------------------------\\\n");
    x_fprintf(fp,"                       |       BLOCKS IN GLOBAL HEAP      |\n");
    x_fprintf(fp,"---------------------------------------------------------------------------------------------\n");
    x_fprintf(fp,"BFR |  Address   |  BSize  |  USize  |  State   | HI | SeqNum | LNum | FileName\n"              );
    x_fprintf(fp,"---------------------------------------------------------------------------------------------\n");

    while (M != NULL)
    {
        if (NextMemBlock != (u32)(M->BlockAddr)) x_fprintf(fp,"B");
        else                                     x_fprintf(fp,".");
    
        if (M->FreeFlag) x_fprintf(fp,".. |");
        else
        {
            if (M->BlockAddr != (u32)M) x_fprintf(fp,"F");
            else                        x_fprintf(fp,".");
        
            if (M->BlockAddr + M->BlockSize != ((u32)M) + sizeof(mem_block) + ALIGN_UP(M->UserSize) + PAD) 
                    x_fprintf(fp,"R |");
            else    
                    x_fprintf(fp,". |");
        }

        x_fprintf(fp," 0x%8.8X | %7d | %7d |",(u32)(M->BlockAddr),(s32)(M->BlockSize),(s32)(M->UserSize));
        NextMemBlock = ((u32)M->BlockAddr) + (s32)(M->BlockSize);

        if (M->FreeFlag) x_fprintf(fp," FFFFFFFF | %2d |",CompFreeHash(M->BlockSize));
        else             x_fprintf(fp," AAAAAAAA | %2d |",M->HeapID);

#ifdef X_DEBUG
        x_fprintf(fp," %6d | %4d | %s",M->Sequence,M->LineNumber,M->FileName);
#endif

        x_fprintf(fp,"\n");
        M = M->GlobalNext;
        Count++;
    }

    x_fprintf(fp,"---------------------------------------------------------------------------------------------\n");
    
#ifdef X_DEBUG
    {
        s32 i;
        for (i=0; i<MEM_MAX_HEAPS; i++)
        {
            s32 AvgSize=0;
            x_fprintf(fp,"HPID:%2d  NA:%6d  NF:%6d  ",i,HeapInfo[i].NumAllocs,HeapInfo[i].NumFrees);
            if (HeapInfo[i].NumAllocs > 0) AvgSize = (HeapInfo[i].SumMemoryAllocated / HeapInfo[i].NumAllocs);
            x_fprintf(fp,"AS:%6d  MS:%6d\n",AvgSize,HeapInfo[i].MaxMemoryAllocated);
        }
    }
    x_fprintf(fp,"---------------------------------------------------------------------------------------------\n");
#endif

    x_fclose(fp);
}

///////////////////////////////////////////////////////////////////////////

void  x_DumpHeapInfo2( void )
{
    x_DumpHeapInfo( "HeapDump.txt" );
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//  FUNCTIONS WHICH ARE AFFECTED BY DEBUG/RELEASE
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Some other portion of the code has decided that we need to allocate a
// chunk of memory UserSize large, starting at address UserAddr.  The caller
// code has already found the source block SB that contains the memory
// requested.  AllocateBlock splits SB into whatever parts it needs and
// returns the final block.  AllocateBlock can be called from general
// mallocs, special alignment mallocs, or user address mallocs.

mem_block* AllocateBlock( mem_block* SB, u32 UserAddr, s32 UserSize, s32 HeapID, char* FileName, s32 LineNumber )
{
    u32 UserStartAddress;
    u32 UserEndAddress;
    u32 BlockEndAddress;
    u32 BlockStartAddress;
    u32 ControlStartAddress;
    s32 FrontWasteSize;
    s32 BackWasteSize;
    mem_block TempB;

    ///////////////////////////////////////////////////////
    // Assert incoming parameters
    ASSERT(SB);
    ASSERT(UserSize > 0);
    ASSERT(IS_ALIGNED(UserAddr));
    ASSERT(HeapID>=0 && HeapID<MEM_MAX_HEAPS);

    ///////////////////////////////////////////////////////
    // Check if all the memory for the heap is taken
    ASSERT( HeapAmount[HeapID] + UserSize < HeapBudget[HeapID] );

    ///////////////////////////////////////////////////////
    // Update statistics
    NumAllocs++;

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
    UnlinkLocalBlock((mem_link*)SB);

    ///////////////////////////////////////////////////////
    // Move control block to appropriate position in block
    // and relink in global list
    TempB = *SB;
    SB = (mem_block*)(ControlStartAddress);
    InitBlock(SB,TempB.BlockSize,TempB.BlockAddr);
    SB->GlobalNext = TempB.GlobalNext;
    SB->GlobalPrev = TempB.GlobalPrev;
    if (SB->GlobalNext) SB->GlobalNext->GlobalPrev = SB;
    if (SB->GlobalPrev) SB->GlobalPrev->GlobalNext = SB;
    else                GlobalHead = SB;
    SB->UserSize = UserSize;
    SB->FreeFlag = 0;
    SB->HeapID   = (u16)HeapID;

    ///////////////////////////////////////////////////////
    // Add block to correct heap list
    LinkLocalBlock(&HeapHead[HeapID],(mem_link*)SB);
    HeapAmount[HeapID] += UserSize;

    ///////////////////////////////////////////////////////
    // Decide if front waste is big enough to break into 
    // a new block of it's own.
    if (FrontWasteSize >= MinUsefulBlockSize)
    {
        mem_block* FWB;
        
        // Split FWB from SB
        FWB = (mem_block*)(SB->BlockAddr);
        InitBlock(FWB,FrontWasteSize,(u32)FWB);
        SB->BlockAddr = (u32)SB;
        SB->BlockSize -= FrontWasteSize;

        // Link FWB into global list
        FWB->GlobalPrev = SB->GlobalPrev;
        if (FWB->GlobalPrev) FWB->GlobalPrev->GlobalNext = FWB;
        else                 GlobalHead = FWB;
        FWB->GlobalNext = SB;
        SB->GlobalPrev  = FWB;
        
        // Add FWB to free list
        InsertFreeBlockIntoTable(FWB);
    }

    ///////////////////////////////////////////////////////
    // Decide if back waste is big enough to break into 
    // a new block of it's own.
    if (BackWasteSize >= MinUsefulBlockSize)
    {
        mem_block* BWB;
        
        // Split BWB from SB
        BWB = (mem_block*)(UserEndAddress);
        InitBlock(BWB,BackWasteSize,(u32)BWB);
        SB->BlockSize -= BackWasteSize;

        // Link BWB into global list
        BWB->GlobalNext = SB->GlobalNext;
        if (BWB->GlobalNext) BWB->GlobalNext->GlobalPrev = BWB;
        BWB->GlobalPrev = SB;
        SB->GlobalNext  = BWB;
        
        // Add BWB to free list
        InsertFreeBlockIntoTable(BWB);
    }

#ifdef X_DEBUG

    ///////////////////////////////////////////////////////
    // Update heap statistics
    HeapInfo[SB->HeapID].NumAllocs++;
    HeapInfo[SB->HeapID].CurrentMemoryAllocated += SB->UserSize;
    HeapInfo[SB->HeapID].SumMemoryAllocated     += SB->UserSize;
    if (HeapInfo[SB->HeapID].CurrentMemoryAllocated > HeapInfo[SB->HeapID].MaxMemoryAllocated)
        HeapInfo[SB->HeapID].MaxMemoryAllocated = HeapInfo[SB->HeapID].CurrentMemoryAllocated;

    ///////////////////////////////////////////////////////
    // Setup debug info
    {
        s32 Len;
        u8* BackLiningPtr;
        Len = x_strlen(FileName)+1;
        if (Len > 16) FileName += (Len-16);
        SB->Sequence   = HeapInfo[SB->HeapID].NumAllocs;
        SB->LineNumber = LineNumber;
        x_strncpy(SB->FileName,FileName,16);
        SB->FileName[15] = 0;
        SB->FrontLining1 = Lining4Byte;
        SB->FrontLining2 = Lining4Byte;
        BackLiningPtr = (u8*)(((u32)SB)+sizeof(mem_block)+SB->UserSize);
        for (Len=0; Len<PAD; Len++) BackLiningPtr[Len] = Lining1Byte;
    }

#endif

    ///////////////////////////////////////////////////////
    // We're done.
    return SB;
}

///////////////////////////////////////////////////////////////////////////

mem_block* MergeNeighboringFrees( mem_block* B )
{
    mem_block* BPrev;
    mem_block* BNext;
    u32        BlockSize;

    BlockSize = B->BlockSize;
    BPrev     = B->GlobalPrev;
    BNext     = B->GlobalNext;

    ///////////////////////////////////////////////////////
    // check if we can merge BPrev and B
    if (BPrev && BPrev->FreeFlag)
    {
        ASSERT(BPrev->BlockAddr == (u32)BPrev);
        if ((BPrev->BlockAddr + BPrev->BlockSize) == B->BlockAddr)
        {
            UnlinkLocalBlock((mem_link*)BPrev);
            BPrev->GlobalNext = BNext;
            if (BNext) BNext->GlobalPrev = BPrev;
            BPrev->BlockSize += B->BlockSize;
            B = BPrev;
        }
    }

    ///////////////////////////////////////////////////////
    // check if we can merge B and BNext
    if (BNext && BNext->FreeFlag)
    {
        ASSERT(BNext->BlockAddr == (u32)BNext);
        if ((B->BlockAddr + B->BlockSize) == BNext->BlockAddr)
        {
            UnlinkLocalBlock((mem_link*)BNext);
            B->GlobalNext = BNext->GlobalNext;
            if (B->GlobalNext) B->GlobalNext->GlobalPrev = B;
            B->BlockSize += BNext->BlockSize;
        }
    }

    return B;
}

///////////////////////////////////////////////////////////////////////////

void FreeBlock( mem_block* B )
{
    mem_block* BPrev;
    mem_block* BNext;
    u32        BlockSize;

    ///////////////////////////////////////////////////////
    // Set free bit-flag, and remove from heap list
    B->FreeFlag = 1;
    UnlinkLocalBlock( (mem_link*)B );
    HeapAmount[B->HeapID] -= B->UserSize;
    ASSERT(HeapAmount[B->HeapID] >= 0);
#ifdef X_DEBUG
    HeapInfo[B->HeapID].NumFrees++;
    HeapInfo[B->HeapID].CurrentMemoryAllocated -= B->UserSize;
    ASSERT(HeapInfo[B->HeapID].CurrentMemoryAllocated >= 0);
#endif

    ///////////////////////////////////////////////////////
    // Convert block to free block.  Move control info to
    // front of the memory block and update global list ptrs.
    BlockSize                       = B->BlockSize;
    BPrev                           = B->GlobalPrev;
    BNext                           = B->GlobalNext;
    B                               = (mem_block*)(B->BlockAddr);
    InitBlock(B,BlockSize,(u32)B); 
    B->GlobalPrev                   = BPrev;
    B->GlobalNext                   = BNext;
    if (BNext) BNext->GlobalPrev    = B;
    if (BPrev) BPrev->GlobalNext    = B;
    else       GlobalHead           = B;
    ASSERT(B->BlockAddr == (u32)B);
    
    ///////////////////////////////////////////////////////
    // Update statistics
    NumFrees++;
    B = MergeNeighboringFrees( B );

    ///////////////////////////////////////////////////////
    // place B in free table
    InsertFreeBlockIntoTable(B);
}

///////////////////////////////////////////////////////////////////////////
// Searches the free list for a block that contains a chunk NBytes long
// that is aligned to a 2^Alignment boundary.  In the case that the alignment
// is 16 bytes (AlignmentPower==4), a slight shortcut is taken since ANY free 
// block is aligned to 16 bytes after it's control block.

void FindAlignedMemory ( mem_block** SB, u32* Addr, s32 NBytes, s32 AlignmentPower )
{
    u32 AlignConst;
    s32 MinimumBlockSize;
    s32 HashIndex;
    u32 AlignedBlockStartAddr;
    u32 AlignedBlockEndAddr;
    mem_block* BestBlock;
    u32 BestStartAddress;
    s32 BestScore;
    
    ///////////////////////////////////////////////////////
    // Assert on parameters
    ASSERT(SB);
    ASSERT(Addr);
    ASSERT(NBytes>0);
    ASSERT(AlignmentPower>=4 && AlignmentPower<=20);
    
    ///////////////////////////////////////////////////////
    // Compute Alignment constant
    AlignConst =  (((u32)1) << AlignmentPower) - 1;

    ///////////////////////////////////////////////////////
    // Compute MINIMUM block size to search for
    MinimumBlockSize = ALIGN_UP(NBytes+PAD) + sizeof(mem_block);
    ASSERT(IS_ALIGNED(MinimumBlockSize));

    ///////////////////////////////////////////////////////
    // Decide on hash table index
    HashIndex = CompFreeHash( MinimumBlockSize );

    ///////////////////////////////////////////////////////////////////////
    // Loop through hash table looking for a block.
    ///////////////////////////////////////////////////////////////////////
    NBytes           = ALIGN_UP(NBytes+PAD);
    BestBlock        = NULL;
    BestStartAddress = 0;
    BestScore        = 0;

    for( ; HashIndex<MEM_HASH_ENTRIES; HashIndex++ )
    {
        mem_block* B = (mem_block*)(FreeHead[HashIndex].Next);

        ///////////////////////////////////////////////////////////////////
        // loop through all blocks at this hash index
        ///////////////////////////////////////////////////////////////////
        while (B != (mem_block*)(&FreeTail[HashIndex]))
        {
            ASSERT(B->BlockAddr == (u32)B);

            ///////////////////////////////////////////////////////////////
            // Determine if block could hold us
            ///////////////////////////////////////////////////////////////
            if( (s32)(B->BlockSize - sizeof(mem_block)) >= NBytes )
            {
                ///////////////////////////////////////////////////////////////
                // Determine if block qualifies at all
                ///////////////////////////////////////////////////////////////
/*
                if (NBytes <= MIN_SIZE_FOR_SMALLS)
                {
                    ///////////////////////////////////////////////////////////
                    // Compute possible block boundaries, right justified
                    ///////////////////////////////////////////////////////////
                    AlignedBlockEndAddr   =  B->BlockAddr + B->BlockSize;
                    AlignedBlockStartAddr =  AlignedBlockEndAddr - NBytes;
                    AlignedBlockStartAddr &= (~AlignConst);
                    AlignedBlockEndAddr   =  AlignedBlockStartAddr + NBytes;

                    ///////////////////////////////////////////////////////////
                    // Check if boundries are inside of block B
                    ///////////////////////////////////////////////////////////
                    ASSERT(AlignedBlockEndAddr <= B->BlockAddr + B->BlockSize);

                    if (AlignedBlockStartAddr >= (u32)B + sizeof(mem_block))
                    {
                        // Check if this block is higher than previous
//                        if ( ((AlignedBlockStartAddr) > BestStartAddress)  || 
//                             ((BestStartAddress==0)) )
                        {
                            BestStartAddress = (AlignedBlockStartAddr);
                            BestBlock = B;
                        }
                    }
                }
                else
                {
                    ///////////////////////////////////////////////////////////
                    // Compute possible block boundaries, left justified
                    ///////////////////////////////////////////////////////////
                    AlignedBlockStartAddr = (u32)B + sizeof(mem_block);
                    AlignedBlockStartAddr = ((AlignedBlockStartAddr + AlignConst) & (~AlignConst));
                    AlignedBlockEndAddr   = AlignedBlockStartAddr + NBytes;

                    ///////////////////////////////////////////////////////////
                    // Check if boundries are inside of block B
                    ///////////////////////////////////////////////////////////
                    if (AlignedBlockEndAddr <= B->BlockAddr + B->BlockSize)
                    {
                        // Check if this block is lower than previous
                        if ( ((AlignedBlockStartAddr) < BestStartAddress)  || 
                             ((BestStartAddress==0)) )
                        {
                            BestStartAddress = (AlignedBlockStartAddr);
                            BestBlock = B;
                        }
                    }
                }
*/

                {
                    ///////////////////////////////////////////////////////////
                    // Compute possible block boundaries, left justified
                    ///////////////////////////////////////////////////////////
                    AlignedBlockStartAddr = (u32)B + sizeof(mem_block);
                    AlignedBlockStartAddr = ((AlignedBlockStartAddr + AlignConst) & (~AlignConst));
                    AlignedBlockEndAddr   = AlignedBlockStartAddr + NBytes;

                    ///////////////////////////////////////////////////////////
                    // Check if boundries are inside of block B
                    ///////////////////////////////////////////////////////////
                    if (AlignedBlockEndAddr <= B->BlockAddr + B->BlockSize)
                    {
                        // Check if this block is smaller than others
                        if( ((s32)B->BlockSize < BestScore) || (BestStartAddress==0) )
                        {
                            BestScore        = B->BlockSize;
                            BestStartAddress = (AlignedBlockStartAddr);
                            BestBlock        = B;
                        }
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
    return;
}

///////////////////////////////////////////////////////////////////////////

void*   x_fn_malloc         ( s32 NBytes, char* FileName, s32 LineNumber )
{
    ASSERT(Initialized);
    ASSERT(NBytes>=0);
    return x_fn_hmalloc( 0, NBytes, FileName, LineNumber );
}

///////////////////////////////////////////////////////////////////////////
s32 NDonations = 0;
void*   x_fn_hmalloc         ( s32 HeapID, s32 NBytes, char* FileName, s32 LineNumber )
{
    mem_block* B;
    u32 Addr;

    ASSERT(Initialized);
    ASSERT(HeapID>=0 && HeapID<MEM_MAX_HEAPS);
    ASSERT(NBytes>=0);

#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == ERR_SUCCESS );
#endif

    // Check for allocation of NO memory
    if (NBytes==0) return ZERO_SIZE_ADDRESS;

    // Search for block 
    FindAlignedMemory( &B, &Addr, NBytes, ALIGN_TO_16 );

#if defined(TARGET_PC) || defined(TARGET_SDC)
    // On the PC, we may be allowed to get some system memory...
    if( (B == NULL) && (AllowGrowth) )
    {
        s32 i;
        s32 Size = ALIGN_UP( MAX( NBytes + MinUsefulBlockSize*2, 1048576 ) );
        for( i=0; i < PC_MEM_CHUNKS; ++i )
            if( PCMemPtr[i] == NULL )
                break;
        if( i < PC_MEM_CHUNKS )
        {
            // Allocate some system memory.
            PCMemPtr[i] = malloc( Size );
            if( PCMemPtr[i] )
            {
                // Give the memory to our manager.
                x_DonateBlock( (u32)PCMemPtr[i], Size );
                NDonations++;

                // Try to find a suitable block now.
                FindAlignedMemory( &B, &Addr, NBytes, ALIGN_TO_16 );
            }
        }
    }    
#endif

#ifdef DO_HEAP_DUMP_ON_MALLOC_FAIL
#ifndef TARGET_N64_ROM
    if (B==NULL) x_DumpHeapInfo("memfail.txt");
#endif
#endif

    // Do we have a block?
    if (B == NULL) return NULL;

    // Allocate section from the block
    B = AllocateBlock( B, Addr, NBytes, HeapID, FileName, LineNumber );

#ifdef X_DEBUG
#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == ERR_SUCCESS );
#endif
#endif

#ifdef DO_HEAP_DUMP_ON_MALLOC_FAIL
#ifndef TARGET_N64_ROM
    if (B==NULL) x_DumpHeapInfo("memfail.txt");
#endif
#endif
    
#ifdef M_N64DEBUG_RELEASE
    ASSERT(B != NULL);
#endif

    // If we came up empty, return NULL
    if (B == NULL) return NULL;

    // Return user's address
    return (void*)( ((u32)B) + sizeof(mem_block) );
}

///////////////////////////////////////////////////////////////////////////

void  x_free            ( void* Ptr )
{
    ASSERT(Initialized);

#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == ERR_SUCCESS );
#endif

    // Check for address of block with ZERO size
    if (Ptr == ZERO_SIZE_ADDRESS) return;

    // Check for deallocation of a NULL ptr
    if (Ptr == NULL) return;

    // Attempt to free the block
    FreeBlock( UserPtrToMemBlock(Ptr) );

#ifdef X_DEBUG
#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == ERR_SUCCESS );
#endif
#endif
}

///////////////////////////////////////////////////////////////////////////

void*  x_fn_realloc( void* Ptr, s32 NewNBytes, char* FileName, s32 LineNumber )
{
    mem_block*  MemBlock;       // block of original memory allocation
    s32         Delta;

    ASSERT( Initialized );

#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == ERR_SUCCESS );
#endif

    // If the pointer is NULL, or points to a 0 size allocation, just malloc.
    if ( (Ptr == NULL) || 
         (Ptr == ZERO_SIZE_ADDRESS) )
    {
        return( x_fn_malloc( NewNBytes, FileName, LineNumber ) );
    }

    // If the pointer is not NULL but the size requested is 0, just free.
    if ( (Ptr != NULL) && (NewNBytes == 0) )
    {
        x_free( Ptr );
        return( NULL );
    }

    // Get the control block.
    MemBlock = UserPtrToMemBlock( Ptr );
    Delta    = NewNBytes- MemBlock->UserSize;

    // Reduction in size requested?
    if ( Delta < 0 )
    {
        void* Ptr = ReallocShrink( MemBlock, NewNBytes );

        if( Ptr == NULL )
        {
            x_DumpHeapInfo("memfail.txt");
        }

#ifdef M_N64DEBUG_RELEASE
        ASSERT(Ptr != NULL);
#endif
        return( Ptr );
    }

    // Increase in size requested?
    if( Delta > 0 )
    {        
        void* Ptr = ReallocGrow( MemBlock, NewNBytes );

        if( Ptr == NULL )
        {
            x_DumpHeapInfo("memfail.txt");
        }
#ifdef M_N64DEBUG_RELEASE
        ASSERT(Ptr != NULL);
#endif

        return( Ptr );
    }

#ifdef X_DEBUG
#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == ERR_SUCCESS );
#endif
#endif

#ifdef M_N64DEBUG_RELEASE
    // Make sure we are returning a good adress
    ASSERT(Ptr != NULL);
#endif
    return( Ptr );
}
    
///////////////////////////////////////////////////////////////////////////

void* x_fn_MallocAligned ( s32 NBytes, s32 AlignmentPower, s32 HeapID, char* FileName, s32 LineNumber )
{
    mem_block* B;
    u32 Addr;
    ASSERT( Initialized );
    ASSERT(HeapID>=0 && HeapID<MEM_MAX_HEAPS);
    ASSERT(NBytes>0);

#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == ERR_SUCCESS );
#endif

    // Search for block 
    FindAlignedMemory ( &B, &Addr, NBytes, AlignmentPower );
    if (B == NULL) return NULL;

    // Allocate section from the block
    B = AllocateBlock ( B, Addr, NBytes, HeapID, FileName, LineNumber );

#ifdef X_DEBUG
#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == ERR_SUCCESS );
#endif
#endif
    
#ifdef DO_HEAP_DUMP_ON_MALLOC_FAIL
#ifndef TARGET_N64_ROM
    if (B==NULL) x_DumpHeapInfo("memfail.txt");
#endif
#endif

    // If we came up empty, return NULL
    if (B == NULL) return NULL;

    // Return user's address
    return (void*)( ((u32)B) + sizeof(mem_block) );
}

///////////////////////////////////////////////////////////////////////////

void* x_fn_MallocAtAddress ( u32 Addr, s32 NBytes, s32 HeapID, char* FileName, s32 LineNumber )
{
    u32 MinimumBlockSize;
    s32 HashIndex;
    u32 BlockStartAddr;
    u32 BlockEndAddr;

    ///////////////////////////////////////////////////////
    // Assert on parameters
    ASSERT(IS_ALIGNED(Addr));
    ASSERT(NBytes>0);
    ASSERT(HeapID>=0 && HeapID<MEM_MAX_HEAPS);

    ///////////////////////////////////////////////////////
    // Check for Addr == 0x00000000
    if (Addr == 0)
        return x_fn_hmalloc( HeapID, NBytes, FileName, LineNumber );

#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == ERR_SUCCESS );
#endif

    ///////////////////////////////////////////////////////
    // Compute MINIMUM block size to search for
    MinimumBlockSize = ALIGN_UP(NBytes+PAD) + sizeof(mem_block);
    ASSERT(IS_ALIGNED(MinimumBlockSize));

    ///////////////////////////////////////////////////////
    // Decide on hash table index
    HashIndex = CompFreeHash( MinimumBlockSize );

    ///////////////////////////////////////////////////////
    // Loop through hash table looking for a block.
    for (; HashIndex<MEM_HASH_ENTRIES; HashIndex++)
    {
        mem_block* B = (mem_block*)(FreeHead[HashIndex].Next);

        // loop through all blocks at this hash index
        while (B != (mem_block*)(&FreeTail[HashIndex]))
        {
            ASSERT(B->BlockAddr == (u32)B);

            // Compute possible block boundaries
            BlockStartAddr = ((u32)B) + sizeof(mem_block);
            BlockEndAddr   = B->BlockAddr + B->BlockSize;

            // Check if boundries are inside of block B
            if ((Addr>=BlockStartAddr) && (Addr+ALIGN_UP(NBytes+PAD) <= BlockEndAddr))
            {
                // Allocate section from the block
                B = AllocateBlock( B, Addr, NBytes, HeapID, FileName, LineNumber );

            #ifdef X_DEBUG
            #ifdef SLOW_MEMORY
                ASSERT( x_MemSanityCheck() == ERR_SUCCESS );
            #endif
            #endif
                
            #ifdef DO_HEAP_DUMP_ON_MALLOC_FAIL
            #ifndef TARGET_N64_ROM
                if (B==NULL) x_DumpHeapInfo("memfail.txt");
            #endif
            #endif

                // If we came up empty, return NULL
                if (B == NULL) return NULL;

                // Return user's address
                return (void*)( ((u32)B) + sizeof(mem_block) );
            }

            // Move on to next block
            B = (mem_block*)(B->LocalList.Next);
        }
    }

#ifdef X_DEBUG
#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == ERR_SUCCESS );
#endif
#endif

    ///////////////////////////////////////////////////////
    // Report no block found
    return NULL;
}

///////////////////////////////////////////////////////////////////////////
// Assumed preconditions:
//  + B is valid.
//  + NewNBytes is less than the original number of bytes.

void* ReallocShrink( mem_block* B, s32 NewNBytes )
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
    x_memset( P, Lining1Byte, PAD );

    // Update heap statistics.
    HeapInfo[B->HeapID].CurrentMemoryAllocated -= B->UserSize;
    HeapInfo[B->HeapID].CurrentMemoryAllocated += NewNBytes;

#endif

    // Update heap budget usage.
    HeapAmount[B->HeapID] -= B->UserSize;
    HeapAmount[B->HeapID] += NewNBytes;

    // Update user size in original memory block.
    B->UserSize  = NewNBytes;

    // Is the amount of memory reduction worth any further effort?
    NewBSize = End - (byte*)(NewBlock);
    if ( NewBSize < MinUsefulBlockSize )
    {
        // Nothing more we can do here.
        return( (void*)U );
    }

    // Update the block size.
    B->BlockSize = (byte*)(NewBlock) - (byte*)(B->BlockAddr);
    
    // Start setting up a new free block in the excess memory.
    InitBlock( NewBlock, NewBSize, (u32)NewBlock );

    // Insert the new block in the global list.
    if ( B->GlobalNext )
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

///////////////////////////////////////////////////////////////////////////
// Assumed preconditions:
//  + B is valid.
//  + NewNBytes is larger than the original number of bytes.

void* ReallocGrow( mem_block* B, s32 NewNBytes )
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
            if( (s32)(End - ALIGN_UP( P+PAD )) >= MinUsefulBlockSize )
            {
                mem_block* NewBlock;
                s32        NewBSize;
                    
                // Determine location and size of new free block.
                NewBlock = (mem_block*) ALIGN_UP( P+PAD );
                NewBSize = End - (byte*)NewBlock;

                // Compute and set the size of the adjusted block.
                B->BlockSize = (byte*)(NewBlock) - (byte*)(B->BlockAddr);

                // Start setting up a new free block in the excess memory.
                InitBlock( NewBlock, NewBSize, (u32)NewBlock );

                // Insert the new block in the global list.
                if ( B->GlobalNext )
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

                // Place the back lining on the reduced size memory block.
                x_memset( P, Lining1Byte, PAD );

                // Update heap statistics.
                HeapInfo[B->HeapID].CurrentMemoryAllocated -= B->UserSize;
                HeapInfo[B->HeapID].CurrentMemoryAllocated += NewNBytes;
                HeapInfo[B->HeapID].SumMemoryAllocated -= B->UserSize;
                HeapInfo[B->HeapID].SumMemoryAllocated += NewNBytes;
                if( HeapInfo[B->HeapID].CurrentMemoryAllocated > HeapInfo[B->HeapID].MaxMemoryAllocated )
                    HeapInfo[B->HeapID].MaxMemoryAllocated = HeapInfo[B->HeapID].CurrentMemoryAllocated;

            #endif

            // Update heap budget usage.
            HeapAmount[B->HeapID] -= B->UserSize;
            HeapAmount[B->HeapID] += NewNBytes;
            ASSERT( HeapAmount[B->HeapID] < HeapBudget[B->HeapID] );

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
        HeapAmount[ B->HeapID ] -= B->UserSize;

        // Allocate a new block of the desired size.
        NewMem = x_fn_hmalloc( B->HeapID, NewNBytes, "", 0 );

        // Undo the temporary decrease of the heap budget usage.
        HeapAmount[ B->HeapID ] += B->UserSize;

        // Did we get the requested memory?
        if( NewMem )
        {
            // Yes!  Go ahead and copy the data over.
            x_memcpy( NewMem, (void*)(B+1), B->UserSize );

            #ifdef X_DEBUG            
            {
                mem_block*  NewBlock;
                // Copy over debug information.
                NewBlock = UserPtrToMemBlock( NewMem );
                NewBlock->LineNumber = B->LineNumber;
                x_memcpy( NewBlock->FileName, B->FileName, 16 );
            }
            #endif
            
            // Release the old allocation.
            x_free( (void*)(B+1) );

            // Return the new memory block.
            return( NewMem );
        }
    }

    // We are doomed to case (3):
    return( NULL );
}

///////////////////////////////////////////////////////////////////////////

err x_MemSanityCheck( void )
{
    err         Error;
    mem_block*  M;

    ASSERT( Initialized );
    Error   = ERR_SUCCESS;
    M       = GlobalHead;
    while (M != NULL)
    {
        //////////////////////////////////////////////////
        // Check that block is aligned
        if (IS_ALIGNED(M) == 0) 
        {
            Error = 1;
            break;
        }

        //////////////////////////////////////////////////
        // Check that global ptrs are intact
        if ((M->GlobalNext!=NULL) && (M->GlobalNext->GlobalPrev != M)) 
        {
            Error = 2;
            break;
        }

        if ((M->GlobalPrev!=NULL) && (M->GlobalPrev->GlobalNext != M)) 
        {
            Error = 3;
            break;
        }

        //////////////////////////////////////////////////
        // Check that local ptrs are intact
        if ( ((mem_link*)M)->Next->Prev != (mem_link*)M )
        {
            Error = 4;
            break;
        }

        if ( ((mem_link*)M)->Prev->Next != (mem_link*)M )
        {
            Error = 5;
            break;
        }

#ifdef X_DEBUG

        //////////////////////////////////////////////////
        // Check liners for overwrite
        if (!M->FreeFlag)
        {
            s32 i;
            u8* LiningPtr;

            if ((M->FrontLining1 != Lining4Byte) | (M->FrontLining2 != Lining4Byte))
            {
                Error = 6;
                break;
            }

            LiningPtr = (u8*)(((u32)M) + sizeof(mem_block) + M->UserSize);
            for (i=0; i<PAD; i++)
            {
                if (LiningPtr[i] != Lining1Byte)
                {
                    Error = 7;
                    break;
                }
            }
        }

#endif

        //////////////////////////////////////////////////
        // Check that list is in memory address order
        if ((M->GlobalNext!=NULL) && ((u32)(M->GlobalNext->BlockAddr) <= (u32)(M->BlockAddr))) 
        {
            Error = 8;
            break;
        }

        if ((M->GlobalPrev!=NULL) && ((u32)(M->GlobalPrev->BlockAddr) >= (u32)(M->BlockAddr))) 
        {
            Error = 9;
            break;
        }

        //////////////////////////////////////////////////
        // Check for neighboring free blocks in global list
        if (M->FreeFlag)
        {
            if (M->GlobalNext && M->GlobalNext->FreeFlag)
            {
                if ( M->BlockAddr + M->BlockSize == M->GlobalNext->BlockAddr )
                {
                    Error = 10;
                    break;
                }
			}

			if ((u32)M->BlockAddr != (u32)M)
			{
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
    if ( Error == ERR_SUCCESS )
    {
        mem_block* Blk;

        for ( Blk = GlobalHead; Blk; Blk = Blk->GlobalNext )
        {
            register byte *pdst;
            register byte *pdstend;
            s32      nbytes;

            // Skip blocks which are not free
            if ( Blk->FreeFlag == FALSE )
                continue;

            //------------------------------------
            // Check the memory for our character
            //------------------------------------

            pdst        = (byte*)( Blk->BlockAddr + sizeof(mem_block) );
            nbytes      = Blk->BlockSize - sizeof(mem_block);
            if (nbytes > 4096) nbytes = 4096;
            pdstend     = pdst + ( nbytes );

            // check starting bytes
            while ((pdst < pdstend) && (((u32)pdst) & 0x03)) 
                if ( *pdst++ != 0xBE ) 
                {
                    Error = 12;
                    ASSERTS( 0, "FREE MEMORY OVERRUN" );
                }

            // check 4bytes at a time
            while ((pdst+4 < pdstend))
                if ( *((u32*)pdst)++ != 0xBEBEBEBE )
                {
                    Error = 12;
                    ASSERTS( 0, "FREE MEMORY OVERRUN" );
                }

            // check ending bytes
            while (pdst < pdstend)
                if ( *pdst++ != 0xBE )
                {
                    Error = 12;
                    ASSERTS( 0, "FREE MEMORY OVERRUN" );
                }
            
        }
    }
#endif



    //////////////////////////////////////////////////
    // Move to next block
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
    ASSERT( Error == ERR_SUCCESS );
    return Error;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//  PLATFORM SPECIFIC INITIALIZATION
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
#if defined(TARGET_PC) || defined(TARGET_SDC)
///////////////////////////////////////////////////////////////////////////

void x_PC_MemoryInit    ( s32 NBytes )
{
    s32 MemAllocated;
    s32 MemSize;
    s32 i;

    // Start up our memory manager.
    x_MemoryInit();

    // Determine how many bytes we will try for.
    NBytes = ALIGN_UP(NBytes);

    // Clear all ptrs
    for (i=0; i<PC_MEM_CHUNKS; i++) 
        PCMemPtr[i] = NULL;

    // Acquire blocks from system and donate to our manager
    i            = 0;
    MemAllocated = 0;
    MemSize      = NBytes;

    while ((MemAllocated < NBytes) && (MemSize >= 1024) && (i<PC_MEM_CHUNKS))
    {
        PCMemPtr[i] = malloc( MemSize );

        if (PCMemPtr[i] == NULL)
        {
            MemSize = ALIGN_UP(MemSize/2);
        }
        else
        {
            MemAllocated += MemSize;
            x_DonateBlock( (u32)PCMemPtr[i], MemSize );
            i++;
        }
    }

    // Did we get what was requested?
    ASSERT(MemAllocated >= NBytes);

    // Since a memory pool size was given, further growth is not permitted.
    AllowGrowth = FALSE;

#ifdef X_DEBUG
#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == ERR_SUCCESS );
#endif
#endif
}

///////////////////////////////////////////////////////////////////////////
#endif
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
#ifdef TARGET_N64
///////////////////////////////////////////////////////////////////////////

void x_N64_MemoryInit( void )
{
    ASSERT( FALSE );
}

///////////////////////////////////////////////////////////////////////////
#endif
///////////////////////////////////////////////////////////////////////////
