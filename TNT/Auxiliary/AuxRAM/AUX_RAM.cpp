////////////////////////////////////////////////////////////////////////////
//
//  AUX_RAM.CPP
//
//  ARAM memory is the auxilary ram on the Dolphin system.
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_target.hpp"
#include "x_debug.hpp"
#include "x_memory.hpp"
#include "x_plus.hpp"
#include "x_stdio.hpp"

#include "AUX_RAM.hpp"


////////////////////////////////////////////////////////////////////////////
//  MODULE DEBUG OPTIONS
////////////////////////////////////////////////////////////////////////////

//#define ARAM_DO_HEAP_DUMP_ON_MALLOC_FAIL


////////////////////////////////////////////////////////////////////////////
//  DEFINES
////////////////////////////////////////////////////////////////////////////

#define MEMBLOCK_ARRAY_SIZE     128

////////////////////////////////////////////////////////////////////////////
//  STRUCTURES
////////////////////////////////////////////////////////////////////////////

struct aram_mem_block
{
    struct aram_mem_block*      GlobalNext;     // Next ptr in global list.
    struct aram_mem_block*      GlobalPrev;     // Prev ptr in global list.

    byte*                       Address;        // This is the user's aligned address
    byte*                       RealAddress;    // This is the real physical address

    s32                         UserSize;       // Physical size of memory requested by user.

  #ifdef X_DEBUG
    s32                         Sequence;       // number of allocs before this block
    s32                         LineNumber;     // *.c line number of malloc call
    char                        FileName[16];   // *.c name
  #endif    

}; // 20 bytes release, 44 bytes debug

//--------------------------------------------------------------------------

struct aram_blockarray
{
    aram_mem_block MemBlocks[MEMBLOCK_ARRAY_SIZE];

    struct aram_blockarray* pNextGroup;
};


////////////////////////////////////////////////////////////////////////////
//  GLOBALS
////////////////////////////////////////////////////////////////////////////

static xbool            Aram_Initialized    = FALSE;
static s32              Aram_NumAllocs      = 0;
static s32              Aram_NumFrees       = 0;
static u32              Aram_TotalAllocated = 0;
static aram_mem_block*  Aram_HeadAlloc      = NULL;
static aram_mem_block*  Aram_TailAlloc      = NULL;
static volatile xbool   Aram_SendInProgress = FALSE;
static u32              Aram_TotalMemory    = 0;
static u32              Aram_MainAddress    = 0;

static aram_blockarray* Aram_BlockArrayList = NULL;

static ARQRequest       arq_dma_task;


////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

void AUXRAM_MemoryInit( void )
{
    ASSERT( !Aram_Initialized );

    // Dolphin API Init
    Aram_MainAddress = ARInit( NULL, 0 );
    ARQInit();
    //ARQSetChunkSize(4096); // Default is 4096

    // get the size of the available memory
    Aram_TotalMemory = ARGetSize() - 0x4000;

    //////////////////////////////////////
    // clear global list and statistics
    Aram_NumAllocs      = 0;
    Aram_NumFrees       = 0;
    Aram_TotalAllocated = 0;
    Aram_HeadAlloc      = NULL;
    Aram_TailAlloc      = NULL;
    Aram_SendInProgress = FALSE;

    Aram_BlockArrayList = NULL;

    //////////////////////////////////////
    // mark manager as initialized
    Aram_Initialized = TRUE;
}

//==========================================================================

void AUXRAM_MemoryKill( void )
{
    ASSERT( Aram_Initialized );

    aram_blockarray* BA;
    aram_blockarray* DA;

    BA = Aram_BlockArrayList;
    while( BA != NULL )
    {
        DA = BA;
        BA = BA->pNextGroup;
        x_free( DA );
    }

    ARReset();  // reset aram

    Aram_Initialized = FALSE;
}

//==========================================================================

static void AUXRAM_FreeSendBuffer( u32 /*task*/ )
{
    Aram_SendInProgress = FALSE;
}

//==========================================================================

void AUXRAM_SendToAram( void* pSrc, void* pARAMDest, u32 Size )
{
    // Make sure pointers and size are 32-byte aligned
    ASSERT( AUXRAM_IS_ALIGN32( pSrc      ) );
    ASSERT( AUXRAM_IS_ALIGN32( pARAMDest ) );
    ASSERT( AUXRAM_IS_ALIGN32( Size      ) );

    DCStoreRange( pSrc, Size );

    Aram_SendInProgress = TRUE;

    ARQPostRequest( &arq_dma_task,
                    0,
                    ARQ_TYPE_MRAM_TO_ARAM,
                    ARQ_PRIORITY_HIGH,
                    (u32)pSrc,
                    (u32)pARAMDest,
                    Size,
                    &AUXRAM_FreeSendBuffer );

    while( Aram_SendInProgress )
    {
        // intentional empty loop, makes this a blocking function
    }
}

//==========================================================================

void AUXRAM_ReadFromAram( void* pDest, void* pARAMSrc, u32 Size )
{
    // Make sure pointers and size are 32-byte aligned
    ASSERT( AUXRAM_IS_ALIGN32( pDest    ) );
    ASSERT( AUXRAM_IS_ALIGN32( pARAMSrc ) );
    ASSERT( AUXRAM_IS_ALIGN32( Size     ) );

    DCInvalidateRange( pDest, Size );

    Aram_SendInProgress = TRUE;

    ARQPostRequest( &arq_dma_task,
                    0,
                    ARQ_TYPE_ARAM_TO_MRAM,
                    ARQ_PRIORITY_HIGH,
                    (u32)pARAMSrc,
                    (u32)pDest,
                    Size,
                    &AUXRAM_FreeSendBuffer );

    while( Aram_SendInProgress )
    {
        // intentional empty loop, makes this a blocking function
    }
}

//==========================================================================

void AUXRAM_StopTransfers( void )
{
    ARQFlushQueue();
}

//==========================================================================

void AUXRAM_DumpHeapInfo( char* /*FileName*/ )
{
    s32             Count;
    aram_mem_block* M;
    //X_FILE*         fp;

    ASSERT( Aram_Initialized );

/*    // Open file to write to
    fp = x_fopen( FileName, "wt" );
    ASSERT( fp );

    Count = 0;
    M = Aram_HeadAlloc;

    x_fprintf( fp,"\n" );

    x_fprintf( fp, "---------------------------------------------------------------------------------------------\n" );
    x_fprintf( fp, "\n" );
    x_fprintf( fp, "Total Allocated Memory : %d \n", Aram_TotalAllocated );
    x_fprintf( fp, "Total Free Memory      : %d \n", ARGetSize() - Aram_TotalAllocated );

    x_fprintf( fp, "\n" );
    x_fprintf( fp, "                       /----------------------------------\\\n" );
    x_fprintf( fp, "                       |       Aram Memory Allocations     |\n" );
    x_fprintf( fp, "---------------------------------------------------------------------------------------------\n" );
    x_fprintf( fp, "Address   |    USize    |    ASize    |   SeqNum   |   LNum   |   FileName\n" );
    x_fprintf( fp, "---------------------------------------------------------------------------------------------\n" );

    while( M != NULL )
    {
        x_fprintf( fp, " 0x%8.8X |   %7d   |   %7d   |   %7d   |", (u32)(M->Address), M->UserSize, ARAM_ROUNDUP32(M->UserSize) );

#ifdef X_DEBUG
        x_fprintf( fp, "   %6d   |   %4d   |   %s", M->Sequence, M->LineNumber, M->FileName );
#endif

        x_fprintf( fp, "\n" );
        M = M->GlobalNext;
        Count++;
    }

    x_fprintf( fp, "---------------------------------------------------------------------------------------------\n" );

    x_fclose( fp );
*/

    Count = 0;
    M = Aram_HeadAlloc;

    x_printf( "\n" );

    x_printf( "--------------------------------------------------------------------------------\n" );
    x_printf( "\n" );
    x_printf( "Total Allocated Memory : %d \n", Aram_TotalAllocated );
    x_printf( "Total Free Memory      : %d \n", ARGetSize() - Aram_TotalAllocated );

    x_printf( "\n" );
    x_printf( "                       /-----------------------------------\\\n" );
    x_printf( "                       |       Aram Memory Allocations     |\n" );
    x_printf( "--------------------------------------------------------------------------------\n" );
    x_printf( "Address    |    USize    |    ASize    |   SeqNum   |   LNum   |   FileName\n" );
    x_printf( "--------------------------------------------------------------------------------\n" );

    while( M != NULL )
    {
        x_printf( "0x%8.8X |   %7d   |   %7d   |", (u32)(M->Address), M->UserSize, AUXRAM_ROUNDUP32(M->UserSize) );

#ifdef X_DEBUG
        x_printf( "   %6d   |   %4d   |   %s", M->Sequence, M->LineNumber, M->FileName );
#endif
        x_printf( "\n" );

        if( M->GlobalNext )
            if( M->GlobalNext->RealAddress - (M->Address+AUXRAM_ROUNDUP32(M->UserSize)) > 0 )
                x_printf( "0x%8.8X |   %7d   |   %7d   | FREE MEM\n", (u32)(M->Address+AUXRAM_ROUNDUP32(M->UserSize)), M->GlobalNext->RealAddress - (M->Address+AUXRAM_ROUNDUP32(M->UserSize)), M->GlobalNext->RealAddress - (M->Address+AUXRAM_ROUNDUP32(M->UserSize)) );

        M = M->GlobalNext;
        Count++;
    }

    x_printf( "--------------------------------------------------------------------------------\n" );
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS WHICH ARE AFFECTED BY DEBUG/RELEASE
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


static aram_mem_block* AllocMemBlock( void )
{
    s32 i;
    aram_blockarray* pBlockArray;

    // check if no mem_block arrays have been allocated, and make one if needed
    if( Aram_BlockArrayList == NULL )
    {
        Aram_BlockArrayList = (aram_blockarray*)x_malloc( sizeof(aram_blockarray) );

        ASSERT( Aram_BlockArrayList != NULL );
        if( Aram_BlockArrayList == NULL )
            return NULL;

        x_memset( Aram_BlockArrayList, 0, sizeof(aram_blockarray) );
    }

    pBlockArray = Aram_BlockArrayList;

    // loop through all mem_block arrays
    while( pBlockArray != NULL )
    {
        // loop through the array to find an unused mem_block
        for( i = 0; i < 64; i++ )
        {
            if( pBlockArray->MemBlocks[i].RealAddress == NULL )
                return &(pBlockArray->MemBlocks[i]);
        }

        pBlockArray = pBlockArray->pNextGroup;
    }

    // no free blocks were found in current array list, make another array
    pBlockArray = (aram_blockarray*)x_malloc( sizeof(aram_blockarray) );

    ASSERT( pBlockArray != NULL );
    if( pBlockArray == NULL )
        return NULL;

    // zero out array data and add to current list
    x_memset( pBlockArray, 0, sizeof(aram_blockarray) );

    pBlockArray->pNextGroup = Aram_BlockArrayList;
    Aram_BlockArrayList     = pBlockArray;

    // return the first block in array since we know its free
    return &(pBlockArray->MemBlocks[0]);
}

//==========================================================================

static void FreeMemBlock( aram_mem_block* pMemBlock )
{
    ASSERT( pMemBlock != NULL );

    x_memset( pMemBlock, 0, sizeof(aram_mem_block) );
}

//==========================================================================

void* AUXRAM_fn_malloc( s32 NBytes, char* FileName, s32 LineNumber )
{
    ASSERT( Aram_Initialized );

    ASSERT( NBytes > 0 );
    if( NBytes <= 0 )
        return NULL;

    aram_mem_block* B = AllocMemBlock();
    ASSERT( B != NULL );
    if( B == NULL )
        return NULL;

    x_memset( (void *)B, 0, sizeof(aram_mem_block) );

    byte* Addr = NULL;

    if( Aram_HeadAlloc == NULL )
    {
        Aram_HeadAlloc = B;
        Addr = (byte*)Aram_MainAddress;
        B->GlobalNext = NULL;
        B->GlobalPrev = NULL;
    }
    else
    {
        aram_mem_block* CheckBlock = Aram_HeadAlloc;

        while( CheckBlock )
        {
            if( !CheckBlock->GlobalNext )
            {
                Addr = CheckBlock->Address + CheckBlock->UserSize;

                if( ((u32)Addr + NBytes - 1) > (Aram_MainAddress + Aram_TotalMemory) )
                {
                  #ifdef ARAM_DO_HEAP_DUMP_ON_MALLOC_FAIL
                    AUXRAM_DumpHeapInfo( "AUXRAM_Memfail.txt" );
                  #endif

                    return NULL;
                }

                CheckBlock->GlobalNext = B;
                B->GlobalPrev  = CheckBlock;
                B->GlobalNext  = NULL;
                Aram_TailAlloc = B;
                break;
            }

            if( CheckBlock->GlobalNext->RealAddress - (CheckBlock->Address + AUXRAM_ROUNDUP32(CheckBlock->UserSize)) >= NBytes )
            {
                Addr = CheckBlock->Address + CheckBlock->UserSize;

                CheckBlock->GlobalNext->GlobalPrev = B;
                B->GlobalNext          = CheckBlock->GlobalNext;
                CheckBlock->GlobalNext = B;
                B->GlobalPrev          = CheckBlock;
                break;
            }

            CheckBlock = CheckBlock->GlobalNext;
        }

        if( Addr == NULL )
        {
            return NULL;
        }
    }

    // increment allocation counter
    Aram_NumAllocs++;

    // fill allocation data
    B->Address     = (byte*)ALIGN_32( Addr );
    B->RealAddress = (byte*)Addr;
    B->UserSize    = NBytes;

  #ifdef X_DEBUG
    if( FileName != NULL )
    {
        s32 NameLen = x_strlen( FileName );

        if( NameLen > 15 )
            x_strncpy( B->FileName, &FileName[NameLen-15], 16 );
        else
            x_strncpy( B->FileName, FileName, 16 );
    }

    B->FileName[15] = '\0';
    B->LineNumber   = LineNumber;
    B->Sequence     = Aram_NumAllocs;
  #endif

    // add amount to the total allocated
    Aram_TotalAllocated += AUXRAM_ROUNDUP32( NBytes );

    // Return user's address
    return B->Address;
}

//==========================================================================

void AUXRAM_free( void* Ptr )
{
    aram_mem_block* B;

    ASSERT( Aram_Initialized );

    // Check for deallocation of a NULL ptr
    if( Ptr == NULL )
        return;

    B = Aram_HeadAlloc;
    while( B != NULL )
    {
        if( B->Address == (byte *)Ptr )
            break;
        B = B->GlobalNext;
    }
    ASSERT( B );
    if( B == NULL )
        return;

//  Aram_NumAllocs--;
    Aram_TotalAllocated -= AUXRAM_ROUNDUP32( B->UserSize );

    if( B->GlobalNext != NULL )
        B->GlobalNext->GlobalPrev = B->GlobalPrev;

    if( B->GlobalPrev != NULL && B->GlobalNext != NULL )
        B->GlobalPrev->GlobalNext = B->GlobalNext;

    if( B->GlobalPrev != NULL && B->GlobalNext == NULL )
        B->GlobalPrev->GlobalNext = NULL;

    if( Aram_HeadAlloc == B )
        Aram_HeadAlloc = B->GlobalNext;

    if( Aram_TailAlloc == B )
        Aram_TailAlloc = B->GlobalPrev;

    FreeMemBlock( B );
}

//==========================================================================

void* AUXRAM_memset( void* pBuf, s32 C, s32 Count )
{
    byte pBuffer[ (8 * 1024) + 32 ];
	byte* pMemBuffer = (byte*)AUXRAM_ROUNDUP32( pBuffer );

    Count = AUXRAM_ROUNDUP32( Count );

    if( Count > (8*1024) )
        x_memset( pMemBuffer, C, 8 * 1024 );
    else
        x_memset( pMemBuffer, C, Count );

    s32   SendCount;
    byte* pAramAddr = (byte*)pBuf;

    while( Count > 0 )
    {
        if( Count > (8*1024) )
            SendCount = (8*1024);
        else
            SendCount = Count;

        AUXRAM_SendToAram( pMemBuffer, pAramAddr, SendCount );

        pAramAddr += SendCount;
        Count -= SendCount;
    }

    return pBuf;
}

//==========================================================================
