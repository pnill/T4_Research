////////////////////////////////////////////////////////////////////////////
//
//  X_MEMORY.CPP
//
////////////////////////////////////////////////////////////////////////////

#ifndef X_DEBUG_HPP
#include "x_debug.hpp"
#endif

#ifndef X_MEMORY_HPP
#include "x_memory.hpp"
#endif

#ifndef X_PLUS_HPP
#include "x_plus.hpp"
#endif

#ifndef X_STDIO_HPP
#include "x_stdio.hpp"
#endif

#ifndef X_TARGET_HPP
#include "x_target.hpp"
#endif

////////////////////////////////////////////////////////////////////////////

#if defined(TARGET_PC) || defined(TARGET_XBOX)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#endif

#ifdef TARGET_PS2
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
//#define DO_HEAP_DUMP_ON_MALLOC_FAIL
#endif

////////////////////////////////////////////////////////////////////////////
//  DEFINES
////////////////////////////////////////////////////////////////////////////

#define MIN_SIZE_FOR_SMALLS     (16*1024)

#define MEM_MIN_FREE_POWER      0                           // 1       byte
#define MEM_MAX_FREE_POWER      20                          // 1048576 bytes
#define MEM_HASH_ENTRIES        (MEM_MAX_FREE_POWER+1)

#define MIN_DONATED_BLOCK_SIZE  1024                        // 16 * 64



#if defined (TARGET_PC) || defined (TARGET_XBOX)
#define	DONATE_SIZE				(16*1024*1024)
#else
#define DONATE_SIZE				(1*1024*1024)
#endif


#if defined (TARGET_DOLPHIN)
#define	SYSALLOC(size)		OSAlloc((u32)Size)
#else
#define	SYSALLOC(size)		malloc((u32)Size)
#endif



#if defined(TARGET_PS2)
#define	ALIGN_BITS				5
#elif defined(TARGET_DOLPHIN)
#define	ALIGN_BITS				5
#else
#define	ALIGN_BITS				4
#endif


#define	ALIGN_SIZE				(1<<ALIGN_BITS)
#define	ALIGN_MASK				(ALIGN_SIZE-1)

#define ALIGN_UP(x)             ((((u32)(x))+ALIGN_MASK)&~ALIGN_MASK)
#define ALIGN_DOWN(x)           (((u32)(x))&~ALIGN_MASK)
#define IS_ALIGNED(x)           ((((u32)(x))&ALIGN_MASK)?(0):(1))


#define ZERO_SIZE_ADDRESS       ((void*)(0xFFFFFFF0))

#ifdef X_DEBUG
    #define PAD  16
#else
    #define PAD   0
#endif


////////////////////////////////////////////////////////////////////////////
//  MEMORY SIZES
////////////////////////////////////////////////////////////////////////////

//---	PS2
#ifdef TARGET_PS2

#ifdef TARGET_PS2_DEV
#define	DEFAULT_MAX_RAM			(128*1024*1024)
#else
#ifdef X_DEBUG
#define	DEFAULT_MAX_RAM			(128*1024*1024)
#else
#define	DEFAULT_MAX_RAM			(32*1024*1024)
#endif
#endif

#define	DEFAULT_STACK_SIZE		(1*1024*1024)

#if X_DEBUG
#define	STACK_CHECK
#endif

//---	PC
#elif defined(TARGET_PC)

#define	DEFAULT_MAX_RAM			(1024*1024*1024)
#define	DEFAULT_STACK_SIZE		(1*1024*1024)

//---	else
#else

#define	DEFAULT_MAX_RAM			(32*1024*1024)
#define	DEFAULT_STACK_SIZE		(1*1024*1024)

#endif

////////////////////////////////////////////////////////////////////////////



//#define MEM_TRACE

#ifdef MEM_TRACE
X_FILE		*pLogFile = NULL;
#endif

////////////////////////////////////////////////////////////////////////////
//  STRUCTURES
////////////////////////////////////////////////////////////////////////////

typedef struct s_mem_link       
{
    struct s_mem_link* Next;
    struct s_mem_link* Prev;
} mem_link;                     

////////////////////////////////////////////////////////////////////////////

typedef struct s_mem_block
{
    ////////////////////////////////////
    // info for release and debug
    ////////////////////////////////////
    mem_link                LocalList;      // Links in Free or Heap list.
    struct s_mem_block*     GlobalNext;     // Next ptr in global list.
    struct s_mem_block*     GlobalPrev;     // Prev ptr in global list.
    byte*                   BlockAddr;      // Physical address of block 
                                            // beginning. This is not necessarily
                                            // the beginning of the mem_block.
    s32                     BlockSize;      // Physical size of block from
                                            // BlockAddr to end of the block.
                                            // includes ALL memory involved in
                                            // block.
    s32                     UserSize;       // Physical size of memory requested
                                            // by user.
    u16                     FreeFlag;       // Block is free if FreeFlag == 1
    s16                     HeapID;         // If block is allocated, it belongs
                                            // in one of the heaps.  This is the
                                            // index to the correct heap.

    ////////////////////////////////////
    // info for debug only
    ////////////////////////////////////
#ifdef X_DEBUG
    s32                     Sequence;       // number of allocs before this block
    s32                     LineNumber;     // *.c line number of malloc call
    char                    FileName[16];   // *.c name
    u32                     FrontLining1;   // front lining
    u32                     FrontLining2;   // front lining
#endif    

} mem_block; // 32 bytes release, 64 bytes debug

////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////
//  GLOBALS
////////////////////////////////////////////////////////////////////////////

static xbool        Initialized			= FALSE;
										
static u32			StackSize			= DEFAULT_STACK_SIZE;
static u32			MaxRAM				= DEFAULT_MAX_RAM;
static s32			CapGrowth			= 0;					// 0 means FALSE, non-zero is the cap size
static xbool        AllowGrowth			= TRUE;
static s32          Locked              = -1;

// Global list organization
static mem_block*   GlobalHead;

// Free memory table
static mem_link     FreeHead    [MEM_HASH_ENTRIES];   // Contains blocksize >= (2^i) 
static mem_link     FreeTail    [MEM_HASH_ENTRIES];

// Heap organization
static mem_link     HeapHead;
static mem_link     HeapTail;
static s32          HeapBudget;
static s32          HeapAmount;

// Statistics
static s32          NumAllocs;
static s32          NumFrees;
static s32          MinUsefulBlockSize;

static s32			NDonations = 0;

#ifdef X_DEBUG
static u32          Lining4Byte = 0x21212121;           // '!!!!'
static u8           Lining1Byte = 0x21;                 // '!'
static heap_info    HeapInfo;
#endif

static byte*		SysMemPtr[32] = {0};
static s32			SysMemSize = 0;

////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
////////////////////////////////////////////////////////////////////////////

mem_block*  UserPtrToMemBlock          ( byte* UserPtr );
void        InitBlock                  ( mem_block* B, s32 BlockSize, byte* BlockAddr );
void        LinkLocalBlock             ( mem_link* Parent, mem_link* Block );
void        UnlinkLocalBlock           ( mem_link* Block );
s32         CompFreeHash               ( s32 NBytes );
void        InsertFreeBlockIntoTable   ( mem_block* B );
void        FreeBlock                  ( mem_block* B );
void        FindAlignedMemory          ( mem_block** SB, 
                                         byte*       Addr, 
                                         s32         NBytes );
mem_block*  AllocateBlock              ( mem_block* SB, 
                                         byte*      UserAddr, 
                                         s32        UserSize, 
                                         char*      FileName, 
                                         s32        LineNumber );

void*       ReallocShrink              ( mem_block* B, s32 NewNBytes );
void*       ReallocGrow                ( mem_block* B, s32 NewNBytes );
mem_block*  MergeNeighboringFrees      ( mem_block* B );

#ifdef X_DEBUG

void        SetLining                  ( mem_block* B );
u8          LiningCorrupt              ( mem_block* B );

void		x_PaintBSS					( u64 PaintValue );
void		x_PaintVoid					( u64 PaintValue );
void		x_PaintStack				( u64 PaintValue );

#endif

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//  LOCK FUNCTIONS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
inline void Lock( s32 LineNumber )
{
//    while (Locked > -1 );
//    Locked = LineNumber;
}

inline void Unlock( void )
{
//    Locked = -1;
}

#define LOCK_X_MEMORY()   Lock( __LINE__ );
#define UNLOCK_X_MEMORY() Unlock();


////////////////////////////////////////////////////////////////////////////
//DEBUG-DEBUG-DEBUG-DEBUG-DEBUG-DEBUG-DEBUG-DEBUG-DEBUG-DEBUG-DEBUG-DEBUG-
////////////////////////////////////////////////////////////////////////////
#ifdef TARGET_PS2
void ENG_PageFlip(xbool ClearNewScreen);
#endif
#if defined( TARGET_PS2 )
void DebugOutput( char* String, char* File, s32 LineNumber );
void WalterPayton( char* FileName, s32 LineNumber )
{
	DebugOutput( "Walter Payton", FileName, LineNumber );
}
#else
void WalterPayton( char* FileName, s32 LineNumber )
{
    return;
}
#endif
////////////////////////////////////////////////////////////////////////////
//DEBUG-DEBUG-DEBUG-DEBUG-DEBUG-DEBUG-DEBUG-DEBUG-DEBUG-DEBUG-DEBUG-DEBUG-
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//  PRIVATE FUNCTIONS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

mem_block*  UserPtrToMemBlock( byte* UserPtr )
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

////////////////////////////////////////////////////////////////////////////

void InitBlock      ( mem_block* B, s32 BlockSize, byte* BlockAddr ) 
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
    B->FrontLining1     = Lining4Byte;
    B->FrontLining2     = Lining4Byte;

    /////////////
    //
    // Here I'm trying to preserve the filename and line that allocated
    // this memory block if that data exists.  This lets us more easily
    // locate code that is causing memory fragmentation
    // 
    // I search for character /0 in the string data...if that is found
    // I assume the data is a valid string and the data is preserved.
    s32 x=0;
    for (;x<16;++x)
    {
        if (B->FileName[x] == 0)
        {
            break;
        }
    }
    if (x == 16)
    {
        //I don't think the filename has ever been initialized, so
        //initialize the filename and linenumber with defaults.
        B->LineNumber = 0;
        x_strncpy((B->FileName),"FREE",16);
    }
#endif
}

////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////

void InsertFreeBlockIntoTable   ( mem_block* B )
{
    s32 HashIndex;
    ASSERT(B);

    // Assuming block has been initialized and is already
    // in the global list.  By now, mem_block should be at
    // the beginning of the block meaning BlockAddr == B
    ASSERT( (mem_block*)(B->BlockAddr) == B);

    // Insert block into free hash table
    HashIndex = CompFreeHash( B->BlockSize );
    LinkLocalBlock( &FreeHead[HashIndex], &(B->LocalList) );

    //-----------------------------------------------------
    // Clear the allocated storage
    //-----------------------------------------------------
#ifdef X_DEBUG
    {
        s32 NBytes;
        NBytes = (B->BlockSize - (s32)sizeof(mem_block));
        if (NBytes > 4096) NBytes = 4096;
        x_memset( (void*)( B->BlockAddr + sizeof(mem_block) ), 
                  0xBE, 
                  NBytes );
    }
#endif
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//  PUBLIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void  x_MemoryInit      ( void )
{
    s32 i;

    ASSERT( !Initialized );
    //ASSERT(IS_ALIGNED(sizeof(mem_block)));

    //////////////////////////////////////
    // clear heaps
    HeapBudget       = 0;
    HeapAmount       = 0;
    HeapHead.Next    = &HeapTail;
    HeapHead.Prev    = NULL;
    HeapTail.Next    = NULL;
    HeapTail.Prev    = &HeapHead;

#ifdef X_DEBUG
    HeapInfo.NumAllocs               = 0;
    HeapInfo.NumFrees                = 0;
    HeapInfo.CurrentMemoryAllocated  = 0;
    HeapInfo.MaxMemoryAllocated      = 0;
    HeapInfo.SumMemoryAllocated      = 0;
#endif
    HeapBudget = (s32)0x7FFFFFFF;

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
    MinUsefulBlockSize = ALIGN_UP(sizeof(mem_block));
    GlobalHead  = NULL;
    NumAllocs   = 0;
    NumFrees    = 0;

    //////////////////////////////////////
    // mark manager as initialized
    Initialized = TRUE;

#ifdef X_DEBUG
#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == 0 );
#endif
#endif

#ifdef TARGET_PS2

	extern	void *_end, *_stack_size;
	u32 Size;
	u32	BSSEnd;
	u32	ActualStackSize;

	BSSEnd = (u32)&_end;
	ActualStackSize = (u32)&_stack_size;

	//---	make sure the stack size requested isn't larger than the real stack size
	if( ActualStackSize < StackSize )
		StackSize = ActualStackSize;

	#define	HEAP_ADJUSTMENT_SIZE	1024	// it will take a long time, but we will get a more exact count an the RAM available

	Size = MaxRAM - StackSize - (u32)BSSEnd;

	//---	cap the size of block allocated
	if( CapGrowth && ((s32)Size > CapGrowth) )
		Size = CapGrowth;

	//---	allocate the block of heap, if it doesn't fit
	//		adjust it till it does
	while( !(SysMemPtr[0] = (byte*)malloc( (u32)Size )) )
		Size -= HEAP_ADJUSTMENT_SIZE;
	ASSERT(SysMemPtr[0]);

	// Give the memory to our manager.
	SysMemSize = Size;
	x_DonateBlock( SysMemPtr[0], Size );
	NDonations = 1;

#endif

}

////////////////////////////////////////////////////////////////////////////

void  x_MemoryKill      ( void )
{
    ASSERT(Initialized);

#if defined(TARGET_PC) || defined(TARGET_XBOX) || defined(TARGET_PS2)
    {
        s32 i;
        for( i=0; i<32; ++i )
            if( SysMemPtr[i] != NULL )
                free( SysMemPtr[i] );
		SysMemSize = 0;
    }
#endif

#ifdef MEM_TRACE
		x_fclose(pLogFile);
#endif // MEM_TRACE

    Initialized = 0;
}

////////////////////////////////////////////////////////////////////////////

void x_MemoryMaxRAM( u32 Setting )
{
	MaxRAM = Setting;
}

////////////////////////////////////////////////////////////////////////////

void x_MemoryStackSize( u32 Setting )
{
	StackSize = Setting;
}

////////////////////////////////////////////////////////////////////////////

void x_MemoryAllowGrowth( xbool Setting )
{
	AllowGrowth = Setting;
}

////////////////////////////////////////////////////////////////////////////

void x_MemoryCapGrowth( s32 Size )
{
	CapGrowth = Size;
}

////////////////////////////////////////////////////////////////////////////

void  x_DonateBlock     ( byte* aBlockAddr, s32 NBytes )
{
    byte* LowBlockAddr;
    byte* HighBlockAddr;
    s32   AlignedBlockSize;
    byte* BlockAddr;
    mem_block* Block;

    ASSERT(Initialized);
    ASSERT(aBlockAddr);
    ASSERT(NBytes > 0);
    BlockAddr = aBlockAddr;

    //////////////////////////////////////////////////
    // Align block to correct boundries
    LowBlockAddr     = (byte*)ALIGN_UP(BlockAddr);
    HighBlockAddr    = (byte*)ALIGN_DOWN(BlockAddr + NBytes);
    AlignedBlockSize = (HighBlockAddr - LowBlockAddr);
    
    //////////////////////////////////////////////////
    // Be sure block is worth dealing with
    if (AlignedBlockSize < MIN_DONATED_BLOCK_SIZE) return;
    if (AlignedBlockSize < MinUsefulBlockSize)  return;

    LOCK_X_MEMORY();
    
    //////////////////////////////////////////////////
    // Create block and add to free list
    Block = (mem_block*)LowBlockAddr;
    InitBlock(Block,AlignedBlockSize,LowBlockAddr);

    //////////////////////////////////////////////////
    // Sort block in global list by address
    if (GlobalHead==NULL)
        GlobalHead = Block;
    else
    if( (byte*)Block < (byte*)GlobalHead )
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
    ASSERT( x_MemSanityCheck() == 0 );
#endif
#endif

    UNLOCK_X_MEMORY();
;
}

////////////////////////////////////////////////////////////////////////////
/*
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
*/
////////////////////////////////////////////////////////////////////////////


//============================================================
// x_TotalMemory
//============================================================
s32 x_TotalMemory( void )
{
    mem_block* Blk;
    s32        Total = 0;

    ASSERT(Initialized);

    LOCK_X_MEMORY();

    for ( Blk = GlobalHead; Blk; Blk = Blk->GlobalNext )
    {
        if ( Blk->FreeFlag == FALSE )
            Total += Blk->BlockSize;
    }

    UNLOCK_X_MEMORY();

    return Total;
}

//============================================================
// x_TotalMemoryFree
//============================================================
s32 x_TotalMemoryFree( void )
{
    mem_block* Blk;
    s32        Total = 0;

    LOCK_X_MEMORY();
    ASSERT(Initialized);
    for ( Blk = GlobalHead; Blk; Blk = Blk->GlobalNext )
    {
        if ( Blk->FreeFlag == TRUE )
            Total += Blk->BlockSize;
    }
    UNLOCK_X_MEMORY();
    return Total;
}

//============================================================
// x_LargestBlockFree
//============================================================
s32 x_LargestBlockFree( void )
{
    mem_block* Blk;
    s32        Largest = 0;

    LOCK_X_MEMORY();
    ASSERT(Initialized);
    for ( Blk = GlobalHead; Blk; Blk = Blk->GlobalNext )
    {
        if ( Blk->FreeFlag == TRUE )
        {
            if (Blk->BlockSize > Largest)
                Largest = Blk->BlockSize;
        }
    }
    UNLOCK_X_MEMORY();
    return Largest;
}

//============================================================
// x_LargestBlockFree
//============================================================
s32 x_NumBlocksFree( s32* pBlockSizes, s32 BlockSizesLength )
{
    mem_block* Blk;
    s32        Count = 0;

    LOCK_X_MEMORY();
    ASSERT(Initialized);
    for ( Blk = GlobalHead; Blk; Blk = Blk->GlobalNext )
    {
        if ( Blk->FreeFlag == TRUE )
        {
			if(( pBlockSizes ) && (Count<BlockSizesLength) )
				pBlockSizes[Count] = Blk->BlockSize;

			Count++;
        }
    }
    UNLOCK_X_MEMORY();
    return Count;
}

//============================================================
// x_SizeOfBlock
//============================================================
u32 x_SizeOfBlock( void* Ptr )
{
    LOCK_X_MEMORY();
    mem_block* MemBlock;

    ASSERT(Initialized);
    ASSERT( Ptr );

#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == 0 );
#endif

    // Check for address of block with ZERO size
    if (Ptr == ZERO_SIZE_ADDRESS) return 0;

    // Attempt to free the block
    MemBlock = UserPtrToMemBlock((byte*)Ptr);

    UNLOCK_X_MEMORY();
    // Return the Size of the block
    return MemBlock->BlockSize;     
}

//============================================================
// x_SizeOfUserBlock
//============================================================
u32 x_SizeOfUserBlock( void* Ptr )
{
    mem_block* MemBlock;

    LOCK_X_MEMORY();
    ASSERT(Initialized);
    ASSERT( Ptr );

#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == 0 );
#endif

    // Check for address of block with ZERO size
    if (Ptr == ZERO_SIZE_ADDRESS) return 0;

    // Attempt to free the block
    MemBlock = UserPtrToMemBlock((byte*)Ptr);
    // Return the Size of the block
    UNLOCK_X_MEMORY();

    return MemBlock->UserSize;     
}

////////////////////////////////////////////////////////////////////////////

s32   x_TotalAllocatedBlocks    ( void )
{
    ASSERT(Initialized);
    return NumAllocs;
}

////////////////////////////////////////////////////////////////////////////

s32   x_TotalFreedBlocks        ( void )
{
    ASSERT(Initialized);
    return NumFrees;
}

////////////////////////////////////////////////////////////////////////////

void  x_DumpHeapInfo ( char* FileName )
{
    byte*       NextMemBlock=NULL;
    s32         Count;
    mem_block*  M;
    X_FILE*     fp;
    s32         BlockSizeTotal = 0;
    s32         UserSizeTotal = 0;

	////////////////////////////////////////////////////////////////////////
	//  CD based systems should not do this
	////////////////////////////////////////////////////////////////////////
	#if defined(TARGET_PS2_DVD) || defined(TARGET_DOLPHIN_DVD) || defined(TARGET_XBOX_DVD)
	ASSERTS(0,"Cannot write to CD/DVD for heap dump");
	return;
	#endif

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
    x_fprintf(fp,"BFR |  Address   |  BSize   |  USize   |  State   | HI | SeqNum | LNum | FileName\n"            );
    x_fprintf(fp,"---------------------------------------------------------------------------------------------\n");

    while (M != NULL)
    {
        if (NextMemBlock != M->BlockAddr)  x_fprintf(fp,"B");
        else                               x_fprintf(fp,".");
    
        if (M->FreeFlag) x_fprintf(fp,".. |");
        else
        {
            if (M->BlockAddr != (byte*)M) x_fprintf(fp,"F");
            else                        x_fprintf(fp,".");
        
            if (M->BlockAddr + M->BlockSize != ((byte*)M) + sizeof(mem_block) + ALIGN_UP(M->UserSize) + PAD) 
                    x_fprintf(fp,"R |");
            else    
                    x_fprintf(fp,". |");
        }

        x_fprintf(fp," 0x%8.8X | %8d | %8d |",(u32)(M->BlockAddr),M->BlockSize,M->UserSize);
        NextMemBlock = M->BlockAddr + M->BlockSize;

        if (M->FreeFlag) x_fprintf(fp," --FREE-- | %2d |",CompFreeHash(M->BlockSize));
        else             x_fprintf(fp," AAAAAAAA | %2d |",M->HeapID);

#ifdef X_DEBUG
        x_fprintf(fp," %6d | %4d | %s",M->Sequence,M->LineNumber,M->FileName);
#endif

        BlockSizeTotal += M->BlockSize;
        UserSizeTotal += M->UserSize;

        x_fprintf(fp,"\n");

        M = M->GlobalNext;
        Count++;
    }

    x_fprintf(fp,"---------------------------------------------------------------------------------------------\n");
    x_fprintf(fp,"Total user bytes allocated: %8d\n",UserSizeTotal);
    x_fprintf(fp,"Total bytes allocated:      %8d\n",BlockSizeTotal);
    x_fprintf(fp,"---------------------------------------------------------------------------------------------\n");
    
#ifdef X_DEBUG
    s32 AvgSize=0;
    x_fprintf(fp,"NA:%6d  NF:%6d  ",HeapInfo.NumAllocs,HeapInfo.NumFrees);
    if (HeapInfo.NumAllocs > 0) AvgSize = (HeapInfo.SumMemoryAllocated / HeapInfo.NumAllocs);
    x_fprintf(fp,"AS:%6d  MS:%6d\n",AvgSize,HeapInfo.MaxMemoryAllocated);
    x_fprintf(fp,"---------------------------------------------------------------------------------------------\n");
#endif

    x_fclose(fp);
}

////////////////////////////////////////////////////////////////////////////

void  x_DumpHeapInfo2( void )
{
    x_DumpHeapInfo( "HeapDump.txt" );
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

mem_block* AllocateBlock( mem_block* SB, byte* UserAddr, s32 UserSize, char* FileName, s32 LineNumber )
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
    ASSERT(SB);
    ASSERT(UserSize > 0);
    ASSERT(IS_ALIGNED(UserAddr));


    ///////////////////////////////////////////////////////
    // Check if all the memory for the heap is taken
    ASSERT( HeapAmount + UserSize < HeapBudget );

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

    ///////////////////////////////////////////////////////
    // Add block to correct heap list
    LinkLocalBlock(&HeapHead,(mem_link*)SB);
    HeapAmount += UserSize;

    ///////////////////////////////////////////////////////
    // Decide if front waste is big enough to break into 
    // a new block of it's own.
    if (FrontWasteSize >= MinUsefulBlockSize)
    {
        mem_block* FWB;
        
        // Split FWB from SB
        FWB = (mem_block*)(SB->BlockAddr);
        InitBlock(FWB,FrontWasteSize,(byte*)FWB);
        SB->BlockAddr = (byte*)SB;
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
        InitBlock(BWB,BackWasteSize,(byte*)BWB);
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
    HeapInfo.NumAllocs++;
    HeapInfo.CurrentMemoryAllocated += SB->UserSize;
    HeapInfo.SumMemoryAllocated     += SB->UserSize;
    if (HeapInfo.CurrentMemoryAllocated > HeapInfo.MaxMemoryAllocated)
        HeapInfo.MaxMemoryAllocated = HeapInfo.CurrentMemoryAllocated;

    ///////////////////////////////////////////////////////
    // Setup debug info
    {
        s32 Len;
        u8* BackLiningPtr;
        Len = x_strlen(FileName)+1;
        if (Len > 16) FileName += (Len-16);
        SB->Sequence   = HeapInfo.NumAllocs;
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

////////////////////////////////////////////////////////////////////////////

mem_block* MergeNeighboringFrees( mem_block* B )
{
    mem_block* BPrev;
    mem_block* BNext;
    s32        BlockSize;

    BlockSize = B->BlockSize;
    BPrev     = B->GlobalPrev;
    BNext     = B->GlobalNext;

    ///////////////////////////////////////////////////////
    // check if we can merge BPrev and B
    if (BPrev && BPrev->FreeFlag)
    {
        ASSERT(BPrev->BlockAddr == (byte*)BPrev);
        if (ALIGN_UP(BPrev->BlockAddr + BPrev->BlockSize) == (u32)B->BlockAddr)
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
        ASSERT(BNext->BlockAddr == (byte*)BNext);
        if (ALIGN_UP(B->BlockAddr + B->BlockSize) == (u32)BNext->BlockAddr)
        {
            UnlinkLocalBlock((mem_link*)BNext);
            B->GlobalNext = BNext->GlobalNext;
            if (B->GlobalNext) B->GlobalNext->GlobalPrev = B;
            B->BlockSize += BNext->BlockSize;
        }
    }

    return B;
}

////////////////////////////////////////////////////////////////////////////

void FreeBlock( mem_block* B )
{
    mem_block* BPrev;
    mem_block* BNext;
    s32        BlockSize;

#ifdef MEM_TRACE
		x_fprintf(pLogFile, "Freeing %d bytes\n", B->BlockSize);
#endif // MEM_TRACE

    ///////////////////////////////////////////////////////
    // Set free bit-flag, and remove from heap list
    B->FreeFlag = 1;
    UnlinkLocalBlock( (mem_link*)B );
    HeapAmount -= B->UserSize;
    ASSERT(HeapAmount >= 0);
#ifdef X_DEBUG
    HeapInfo.NumFrees++;
    HeapInfo.CurrentMemoryAllocated -= B->UserSize;
    ASSERT(HeapInfo.CurrentMemoryAllocated >= 0);
#endif

    ///////////////////////////////////////////////////////
    // Convert block to free block.  Move control info to
    // front of the memory block and update global list ptrs.
    BlockSize                       = B->BlockSize;
    BPrev                           = B->GlobalPrev;
    BNext                           = B->GlobalNext;
    B                               = (mem_block*)(B->BlockAddr);
    InitBlock(B,BlockSize,(byte*)B); 
    B->GlobalPrev                   = BPrev;
    B->GlobalNext                   = BNext;
    if (BNext) BNext->GlobalPrev    = B;
    if (BPrev) BPrev->GlobalNext    = B;
    else       GlobalHead           = B;
    ASSERT(B->BlockAddr == (byte*)B);
    
    ///////////////////////////////////////////////////////
    // Update statistics
    NumFrees++;
    B = MergeNeighboringFrees( B );

    ///////////////////////////////////////////////////////
    // place B in free table
    InsertFreeBlockIntoTable(B);
}

////////////////////////////////////////////////////////////////////////////
// Searches the free list for a block that contains a chunk NBytes long
// that is aligned to a 2^Alignment boundary.  In the case that the alignment
// is 16 bytes (AlignmentPower==4), a slight shortcut is taken since ANY free 
// block is aligned to 16 bytes after it's control block.

void FindAlignedMemory ( mem_block** SB, byte** Addr, s32 NBytes )
{
    s32 MinimumBlockSize;
    s32 HashIndex;
    byte* AlignedBlockStartAddr;
    byte* AlignedBlockEndAddr;
    mem_block* BestBlock;
    byte* BestStartAddress;
    s32 BestScore;
    
    ///////////////////////////////////////////////////////
    // Assert on parameters
    ASSERT(SB);
    ASSERT(Addr);
    ASSERT(NBytes>0);
    
    ///////////////////////////////////////////////////////
    // Compute MINIMUM block size to search for
    MinimumBlockSize = (s32)ALIGN_UP(NBytes + PAD + (s32)sizeof(mem_block));
    ASSERT(IS_ALIGNED(MinimumBlockSize));

    ///////////////////////////////////////////////////////
    // Decide on hash table index
    HashIndex = CompFreeHash( MinimumBlockSize );

    ///////////////////////////////////////////////////////////////////////
    // Loop through hash table looking for a block.
    ///////////////////////////////////////////////////////////////////////
    NBytes           = (s32)ALIGN_UP(NBytes+PAD);
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
            ASSERT(B->BlockAddr == (byte*)B);

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

            // move on to next block
            B = (mem_block*)(B->LocalList.Next);
        }
    }

    // Report no block found
    *SB     = BestBlock;
    *Addr   = BestStartAddress;
}

////////////////////////////////////////////////////////////////////////////
void FindAlignedMemoryTop ( mem_block** SB, byte** Addr, s32 NBytes )
{
    s32 MinimumBlockSize;
    s32 HashIndex;
    byte* AlignedBlockStartAddr;
    byte* AlignedBlockEndAddr;
    mem_block* BestBlock;
    byte* BestStartAddress;
    
    ///////////////////////////////////////////////////////
    // Assert on parameters
    ASSERT(SB);
    ASSERT(Addr);
    ASSERT(NBytes>0);
    
    ///////////////////////////////////////////////////////
    // Compute MINIMUM block size to search for
    MinimumBlockSize = (s32)ALIGN_UP(NBytes+PAD) + (s32)sizeof(mem_block);
    ASSERT(IS_ALIGNED(MinimumBlockSize));

    ///////////////////////////////////////////////////////
    // Decide on hash table index
    //HashIndex = CompFreeHash( MinimumBlockSize );

    ///////////////////////////////////////////////////////////////////////
    // Loop through hash table looking for a block.
    ///////////////////////////////////////////////////////////////////////
    NBytes           = (s32)ALIGN_UP(NBytes+PAD);
    BestBlock        = NULL;
    BestStartAddress = 0;

    for(HashIndex=MEM_HASH_ENTRIES-1 ; HashIndex>0; HashIndex-- )
    {
        mem_block* B = (mem_block*)(FreeTail[HashIndex].Prev);

        ///////////////////////////////////////////////////////////////////
        // loop through all blocks at this hash index till we find a block
        ///////////////////////////////////////////////////////////////////
        while (B != (mem_block*)(&FreeHead[HashIndex]) && !BestBlock)
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
                if (AlignedBlockEndAddr <= B->BlockAddr + B->BlockSize)
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
        if (BestBlock) break;
    }

    // Report no block found
    *SB     = BestBlock;
    *Addr   = BestStartAddress;
}

////////////////////////////////////////////////////////////////////////////
void*   x_fn_hmalloc         ( s32 NBytes, char* FileName, s32 LineNumber );
void*   x_fn_hmalloc_top     ( s32 NBytes, char* FileName, s32 LineNumber );

////////////////////////////////////////////////////////////////////////////
void*   x_fn_malloc         ( s32 NBytes, char* FileName, s32 LineNumber )
{
    ASSERT(Initialized);
    ASSERT(NBytes>=0);

#ifdef MEM_TRACE
	if (pLogFile == NULL)
	{
		pLogFile = x_fopen("mem_log.txt", "wt");
		ASSERT(pLogFile);
	}
	x_fprintf(pLogFile, "Alloc: %d Bytes from %s (%d)\n", NBytes, FileName, LineNumber);
#endif

    void* Ret;

    LOCK_X_MEMORY();
    Ret = x_fn_hmalloc( NBytes, FileName, LineNumber );
    UNLOCK_X_MEMORY();
    return Ret;
}

////////////////////////////////////////////////////////////////////////////
void*   x_fn_malloc_top     ( s32 NBytes, char* FileName, s32 LineNumber )
{
    ASSERT(Initialized);
    ASSERT(NBytes>=0);

#ifdef MEM_TRACE
	if (pLogFile == NULL)
	{
		pLogFile = x_fopen("mem_log.txt", "wt");
		ASSERT(pLogFile);
	}
	x_fprintf(pLogFile, "AllocTop: %d Bytes from %s (%d)\n", NBytes, FileName, LineNumber);
#endif

    void* Ret;

    LOCK_X_MEMORY();
    Ret = x_fn_hmalloc_top( NBytes, FileName, LineNumber );
    UNLOCK_X_MEMORY();
    return Ret;
}

////////////////////////////////////////////////////////////////////////////
void*   x_fn_calloc         ( s32 NBytes, char* FileName, s32 LineNumber )
{
    ASSERT(Initialized);
    ASSERT(NBytes>=0);
    LOCK_X_MEMORY();

#ifdef MEM_TRACE
	if (pLogFile == NULL)
	{
		pLogFile = x_fopen("mem_log.txt", "W");
		ASSERT(pLogFile);
	}
	x_fprintf(pLogFile, "Alloc: %d Bytes from %s (%d)\n", NBytes, FileName, LineNumber);
#endif

	void *pMem = x_fn_hmalloc( NBytes, FileName, LineNumber );
	if (pMem)
		x_memset(pMem, 0, NBytes);

    UNLOCK_X_MEMORY();

	return (pMem);
}

////////////////////////////////////////////////////////////////////////////
void*   x_fn_hmalloc         ( s32 NBytes, char* FileName, s32 LineNumber )
{
    mem_block* B;
    byte* Addr;

    ASSERT(Initialized);
    ASSERT(NBytes>=0);

#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == 0 );
#endif

    // Check for allocation of NO memory
    if (NBytes==0)
    {
		return ZERO_SIZE_ADDRESS;
    }

    // Search for block 
    FindAlignedMemory( &B, &Addr, NBytes );

    s32 MinSize = DONATE_SIZE;

    // On the PC, we may be allowed to get some system memory...
    if( (B == NULL) && (AllowGrowth) )
    {
#ifdef MEM_TRACE
		x_fprintf(pLogFile, "ALLOC FAILED! - Attempting to grow heap\n");
#endif // MEM_TRACE
        s32 i;
		// CARL - the plus 64 is for alignment
        s32 Size = (s32)ALIGN_UP( MAX( NBytes + MinUsefulBlockSize*2 +64, MinSize ) );
        for( i=0; i < 32; ++i )
            if( SysMemPtr[i] == NULL )
                break;
        if( i < 32 )
        {
#ifdef MEM_TRACE
			x_fprintf(pLogFile, "    allocating %d bytes of system memory...\n", Size);
#endif // MEM_TRACE
            // Allocate some system memory.
			if( CapGrowth && (( SysMemSize + Size ) > CapGrowth ))
			{
				Size = CapGrowth - SysMemSize;
				ASSERTS( Size > 0, "Attempt to allocate memory over the cap" );
			}

            SysMemPtr[i] = (byte*)SYSALLOC( (u32)Size );
            if( SysMemPtr[i] )
            {
                // Give the memory to our manager.
				SysMemSize += Size;
                x_DonateBlock( SysMemPtr[i], Size );
                NDonations++;

#ifdef MEM_TRACE
		x_fprintf(pLogFile, "    SUCCESS. %d Donations ($%08x - $%08x)\n", NDonations, SysMemPtr[i], SysMemPtr[i]+Size);
#endif // MEM_TRACE

                // Try to find a suitable block now.
                FindAlignedMemory( &B, &Addr, NBytes );
            }
#ifdef MEM_TRACE
			else
			{
				x_fprintf(pLogFile, "**** MEMORY FULL! ****\n");
			}
#endif // MEM_TRACE
        }
    }    


#ifdef DO_HEAP_DUMP_ON_MALLOC_FAIL
    if (B==NULL)
		x_DumpHeapInfo("memfail.txt");
#endif

    // Do we have a block?
    if (B == NULL)
	{
WalterPayton( FileName, LineNumber );
		return NULL;
	}

    // Allocate section from the block
    B = AllocateBlock( B, Addr, NBytes, FileName, LineNumber );

#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == 0 );
#endif

#ifdef DO_HEAP_DUMP_ON_MALLOC_FAIL
    if (B==NULL)
		x_DumpHeapInfo("memfail.txt");
#endif
    
    // If we came up empty, return NULL
    if (B == NULL)
	{
WalterPayton( FileName, LineNumber );
		return NULL;
	}

#ifdef MEM_TRACE
		x_fprintf(pLogFile, "    Allocatted at:%08x\n", (u32)B);
#endif // MEM_TRACE

    // Return user's address
    return (void*)( ((u32)B) + sizeof(mem_block) );
}


////////////////////////////////////////////////////////////////////////////

void*   x_fn_hmalloc_top ( s32 NBytes, char* FileName, s32 LineNumber )
{
    mem_block* B;
    byte* Addr;

    ASSERT(Initialized);
    ASSERT(NBytes>=0);

#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == 0 );
#endif

    // Check for allocation of NO memory
    if (NBytes==0)
    {
		return ZERO_SIZE_ADDRESS;
    }

    // Search for block 
    FindAlignedMemoryTop( &B, &Addr, NBytes );

    s32     MinSize = DONATE_SIZE;

    // On the PC, we may be allowed to get some system memory...
    if( (B == NULL) && (AllowGrowth) )
    {
#ifdef MEM_TRACE
		x_fprintf(pLogFile, "ALLOC FAILED! - Attempting to grow heap\n");
#endif // MEM_TRACE
        s32 i;
		// CARL - the plus 64 is for alignment
        s32 Size = (s32)ALIGN_UP( MAX( NBytes + MinUsefulBlockSize*2 +64, MinSize ) );
        for( i=0; i < 32; ++i )
            if( SysMemPtr[i] == NULL )
                break;
        if( i < 32 )
        {
#ifdef MEM_TRACE
			x_fprintf(pLogFile, "    allocating %d bytes of system memory...\n", Size);
#endif // MEM_TRACE
            // Allocate some system memory.
			if( CapGrowth && (( SysMemSize + Size ) > CapGrowth ))
			{
				Size = CapGrowth - SysMemSize;
				ASSERTS( Size > 0, "Attempt to allocate memory over the cap" );
			}

            // Allocate some system memory.
            SysMemPtr[i] = (byte*)SYSALLOC( Size );
            if( SysMemPtr[i] )
            {
                // Give the memory to our manager.
				SysMemSize += Size;
                x_DonateBlock( SysMemPtr[i], Size );
                NDonations++;

#ifdef MEM_TRACE
		x_fprintf(pLogFile, "    SUCCESS. %d Donations ($%08x - $%08x)\n", NDonations, SysMemPtr[i], SysMemPtr[i]+Size);
#endif // MEM_TRACE

                // Try to find a suitable block now.
                FindAlignedMemoryTop( &B, &Addr, NBytes );
            }
#ifdef MEM_TRACE
			else
			{
				x_fprintf(pLogFile, "**** MEMORY FULL! ****\n");
			}
#endif // MEM_TRACE
        }
    }    

#ifdef DO_HEAP_DUMP_ON_MALLOC_FAIL
    if (B==NULL)
		x_DumpHeapInfo("memfail.txt");
#endif

    // Do we have a block?
    if (B == NULL)
	{
WalterPayton( FileName, LineNumber );
		return NULL;
	}

    // Allocate section from the block
    B = AllocateBlock( B, Addr, NBytes, FileName, LineNumber );

#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == 0 );
#endif

#ifdef DO_HEAP_DUMP_ON_MALLOC_FAIL
    if (B==NULL)
		x_DumpHeapInfo("memfail.txt");
#endif
    
    // If we came up empty, return NULL
    if (B == NULL)
	{
WalterPayton( FileName, LineNumber );
		return NULL;
	}

#ifdef MEM_TRACE
		x_fprintf(pLogFile, "    Allocatted at:%08x\n", (u32)B);
#endif // MEM_TRACE

    // Return user's address
    return (void*)( ((u32)B) + sizeof(mem_block) );
}

////////////////////////////////////////////////////////////////////////////

void  x_free            ( void* Ptr )
{
    ASSERT(Initialized);

#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == 0 );
#endif

    // Check for address of block with ZERO size
    if (Ptr == ZERO_SIZE_ADDRESS)
		return;

    // Check for deallocation of a NULL ptr
    if (Ptr == NULL)
		return;


    // Attempt to free the block
    LOCK_X_MEMORY();
    FreeBlock( UserPtrToMemBlock((byte*)Ptr) );
    UNLOCK_X_MEMORY();

#ifdef X_DEBUG
#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == 0 );
#endif
#endif
}

////////////////////////////////////////////////////////////////////////////

void*  x_fn_realloc( void* Ptr, s32 NewNBytes, char* FileName, s32 LineNumber )
{
    mem_block*  MemBlock;       // block of original memory allocation
    s32         Delta;

    ASSERT( Initialized );

#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == 0 );
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

    LOCK_X_MEMORY();

    // Get the control block.
    MemBlock = UserPtrToMemBlock( (byte*)Ptr );
    Delta    = NewNBytes- MemBlock->UserSize;

    // Reduction in size requested?
    if ( Delta < 0 )
    {
        void* Ptr = ReallocShrink( MemBlock, NewNBytes );

        if( Ptr == NULL )
        {
            x_DumpHeapInfo("memfail.txt");
        }

        UNLOCK_X_MEMORY();
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

        UNLOCK_X_MEMORY();
        return( Ptr );
    }

#ifdef X_DEBUG
#ifdef SLOW_MEMORY
    ASSERT( x_MemSanityCheck() == 0 );
#endif
#endif

    UNLOCK_X_MEMORY();

    return( Ptr );
}
    
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
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
    HeapInfo.CurrentMemoryAllocated -= B->UserSize;
    HeapInfo.CurrentMemoryAllocated += NewNBytes;

#endif

    // Update heap budget usage.
    HeapAmount -= B->UserSize;
    HeapAmount += NewNBytes;

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
    InitBlock( NewBlock, NewBSize, (byte*)NewBlock );

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

////////////////////////////////////////////////////////////////////////////
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
                InitBlock( NewBlock, NewBSize, (byte*)NewBlock );

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
                HeapInfo.CurrentMemoryAllocated -= B->UserSize;
                HeapInfo.CurrentMemoryAllocated += NewNBytes;
                HeapInfo.SumMemoryAllocated -= B->UserSize;
                HeapInfo.SumMemoryAllocated += NewNBytes;
                if( HeapInfo.CurrentMemoryAllocated > HeapInfo.MaxMemoryAllocated )
                    HeapInfo.MaxMemoryAllocated = HeapInfo.CurrentMemoryAllocated;

            #endif

            // Update heap budget usage.
            HeapAmount -= B->UserSize;
            HeapAmount += NewNBytes;
            ASSERT( HeapAmount < HeapBudget );

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
        HeapAmount -= B->UserSize;

        // Allocate a new block of the desired size.
        NewMem = x_fn_hmalloc( NewNBytes, "", 0 );

        // Undo the temporary decrease of the heap budget usage.
        HeapAmount += B->UserSize;

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
                x_memcpy( NewBlock->FileName, B->FileName, 16 );
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

s32   x_MemSanityCheck( void )
{
    s32         Error;
    mem_block*  M;

    LOCK_X_MEMORY();

    ASSERT( Initialized );
    Error   = 0;
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
    if ( Error == 0 )
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
            nbytes      = Blk->BlockSize - (s32)sizeof(mem_block);
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
            {
                if ( *((u32*)pdst) != 0xBEBEBEBE )
                {
                    Error = 12;
                    ASSERTS( 0, "FREE MEMORY OVERRUN" );
                }
                pdst+=4;
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
    
    UNLOCK_X_MEMORY();
    return( Error );
}

#if 0 // X_DEBUG

////////////////////////////////////////////////////////////////////////////
//
//	x_PainVoid paints the regean of memory above the heap and below the stack.
//	this region only exists when a dev kit provides more RAM than a target
//	system, and a programmer wants to limit the dev kit to that target platform.
//
void x_PaintBSS( u64 PaintValue )
{
#ifdef TARGET_PS2
	u64*			pBSS	= (u64*)__bss_obj;
	u64*			pBSSEnd = (u64*)__bss_objend;
	u64*			pRAM;
	s32				RoundOffBytes;
	s32				Size;

	//---	if the heap hasn't been capped, there is nothing to paint
	if( !CapGrowth )
		return;

	//---	get the size of the void
	Size = (u32)pBSSEnd - (u32)pBSS;
	pRAM = pBSS;

	//---	round off the process
	RoundOffBytes = (u32)pRAM % sizeof(u64);
	if( RoundOffBytes )
	{
		RoundOffBytes = sizeof(u64)-RoundOffBytes;
		x_memcpy( pRAM, &PaintValue, RoundOffBytes );
		pRAM = (u64*) (((u32)pRAM)+RoundOffBytes);
	}

	//---	round off the process
	RoundOffBytes = (u32)pBSSEnd % sizeof(u64);
	if( RoundOffBytes )
		pBSSEnd = (u64*)(((u32)pBSSEnd) - RoundOffBytes);

	//---	fill the void space with a value
	while( pRAM!=(u64*)pBSSEnd )
		*pRAM++ = PaintValue;
#endif
}

////////////////////////////////////////////////////////////////////////////
//
//	x_PainVoid paints the regean of memory above the heap and below the stack.
//	this region only exists when a dev kit provides more RAM than a target
//	system, and a programmer wants to limit the dev kit to that target platform.
//
void x_PaintVoid( u64 PaintValue )
{
#ifdef TARGET_PS2
	u64*			pBSSEnd = (u64*)__bss_objend;
	u32				EndSP;
	u64*			pRAM;
	s32				RoundOffBytes;
	s32				Size;

	//---	if the heap hasn't been capped, there is nothing to paint
	if( !CapGrowth )
		return;

	//---	Get the top of the stack
	EndSP = MaxRAM - StackSize;
	ASSERT(!(EndSP%sizeof(u64)));

	//---	get the size of the void
	Size = EndSP - (u32)pBSSEnd;
	if( Size <= CapGrowth )
		return;

	//---	size now contains the size of the void
	Size -= CapGrowth;
	pRAM = pBSSEnd;
	pRAM = (u64*)(((u32)pBSSEnd) + CapGrowth);

	//---	round off the process
	RoundOffBytes = (u32)pRAM%sizeof(u64);
	if( RoundOffBytes )
	{
		RoundOffBytes = sizeof(u64)-RoundOffBytes;
		x_memcpy( pRAM, &PaintValue, RoundOffBytes );
		pRAM = (u64*) (((u32)pRAM)+RoundOffBytes);
	}

	//---	fill the void space with a value
	while( pRAM!=(u64*)EndSP )
		*pRAM++ = PaintValue;
#endif
}
#endif

////////////////////////////////////////////////////////////////////////////
//
//	Be careful modifying this function.  You don't want to be changing the stack
//	while using this function.
//
////////////////////////////////////////////////////////////////////////////
void x_PaintStack( u64 PaintValue )
{
#ifdef TARGET_PS2

	volatile s32	sp[100];
	static u64		CurSP;
	static u64		EndSP;
	static u64*		pRAM;

	//---	Get the stack bounds. 
	//		NOTE: we cannot work with it above its current location
	CurSP = (s32)&sp[0];
	EndSP = MaxRAM - StackSize;

	if( CurSP&0x7 )	CurSP += sizeof(u64)-(CurSP&0x7);
	ASSERT( !(CurSP & 0x7) );

	//---	fill the stack space with a value
	pRAM=(u64*)EndSP;
	while( pRAM!=(u64*)CurSP )
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
	return StackSize;
}

////////////////////////////////////////////////////////////////////////////
//
//	x_StackCheckInit
//
//	Paints the stack to initialize the stack check system.  Should be called
//	near the start of the program in a function which is close to the bottom
//	of expected stack growth.
//
////////////////////////////////////////////////////////////////////////////
void x_StackCheckInit( void )
{
	x_PaintStack( 0xDDDDDDDDDDDDDDDD );
}

////////////////////////////////////////////////////////////////////////////
//
//	x_StackCheckUsed
//
//	returns the current amount of the stack used.  This function has no way
//	to detect if the stack has been overwritten. If the amount used
//	get close to the total amount, a warning signal or ASSERT should be hit
//	to suggest a stack size increase.
//
////////////////////////////////////////////////////////////////////////////
u32 x_StackCheckUsed( void )
{
	u32 UsedSize;
	u32* pRAM;

	pRAM = (u32*)(MaxRAM - StackSize);
	while( (u32)pRAM < MaxRAM )
		if( *pRAM++ != 0xDDDDDDDD )
			break;

	UsedSize = MaxRAM - (u32)pRAM + 4;

	return UsedSize;
}

////////////////////////////////////////////////////////////////////////////
//
//	x_StackCheckLeft
//
//	This function returns the number of bytes in the stack which have been
//	currently unused.
//
//	NOTE: This function cannot detect if a stack has overflowed.  The lower
//	the size of unused stack gets, the greater likelyhood that it has or will
//	overflow.
//
//	returns the amount of stack left
//
////////////////////////////////////////////////////////////////////////////
u32 x_StackCheckLeft( void )
{
	u32 UsedSize = x_StackCheckUsed();
	return StackSize - UsedSize;
}

////////////////////////////////////////////////////////////////////////////
//
//	x_StackCheck
//
//	This function can be called regularly to check for the stack to have
//	shrunk dangerously low.  It doesn't have to be called in a function
//	which uses much stack, but it should be called regularly in the game loop.
//
//	returns the amount of stack left
//
////////////////////////////////////////////////////////////////////////////
u32 x_StackCheck( void )
{
	u32 StackLeft = x_StackCheckLeft();
	ASSERTS(StackLeft > 1024, "WARNING: Stack used to within 1K of its limit");
	return StackLeft;
}

#else

void x_StackCheckInit( void )	{}
u32 x_StackCheckSize( void )	{ return StackSize; }
u32 x_StackCheckUsed( void )	{ return 0; }
u32 x_StackCheckLeft( void )	{ return StackSize; }
u32 x_StackCheck( void )		{ return StackSize; }

#endif


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS FOR C++   new  new[]  delete  delete[]
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#ifndef USE_NATIVE_NEW_AND_DELETE

#ifdef new
#undef new
#endif

//==========================================================================
void* operator new( u32 /*Size*/, x_PlacementNewDummy * pAddress  )
{
    return( reinterpret_cast<void*>(pAddress) );
}

void operator delete( void * Address , x_PlacementNewDummy * )
{
}

//==========================================================================

void* operator new( u32 Size, char* pFileName, s32 LineNumber )
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

void* operator new [] ( u32 Size, char* pFileName, s32 LineNumber )
{
    return( x_fn_malloc( (s32)Size, pFileName, LineNumber ) );
}

//==========================================================================

void operator delete [] ( void* pMemory )
{
    x_free( pMemory );
}

//==========================================================================

void operator delete [] ( void* pMemory, char*, s32 )
{
    x_free( pMemory );
}

//==========================================================================

#endif // USE_NATIVE_NEW_AND_DELETE

//==========================================================================
