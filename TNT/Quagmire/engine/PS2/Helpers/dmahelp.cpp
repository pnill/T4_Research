///////////////////////////////////////////////////////////////////////////
//
//  DMAHELP.C
//
///////////////////////////////////////////////////////////////////////////

#include "dmahelp.hpp"
#include "x_debug.hpp"
#include "x_stdio.hpp"
#include "x_plus.hpp"
#include <eekernel.h>
#include <eeregs.h>
#include <libdma.h>

#include "Q_SMEM.hpp"
#include "DMAMAN.hpp"
#include <libgraph.h>

#include "Q_PS2.hpp"

//============================================================================
void DMAHELP_Wait(void)
{
    asm __volatile__
    ("
        SYNC.L

        WDMA:

        BC0F WDMA
        NOP
    ");
}
//============================================================================

void DMAHELP_BuildTagCont( sceDmaTag* pTag, s32 NBytes )
{
    // Make sure size is 16 byte multiple
    ASSERT( (NBytes & 0x0F) == 0 );
    pTag->qwc  = (u16)(NBytes>>4);
    pTag->id   = 0x10;
    pTag->next = NULL;
    pTag->mark = 0;
    pTag->p[0] = 0;
    pTag->p[1] = 0;
}

//============================================================================

void DMAHELP_BuildTagEnd( sceDmaTag* pTag, s32 NBytes )
{
    // Make sure size is 16 byte multiple
    ASSERT( (NBytes & 0x0F) == 0 );
    pTag->qwc  = (u16)(NBytes>>4);
    pTag->id   = 0x70;
    pTag->next = 0;
    pTag->mark = 17;
    pTag->p[0] = 0;
    pTag->p[1] = 0;

    // Set IRQ bit
    ((u32*)pTag)[0] |= 0x80000000;
}

//============================================================================

void DMAHELP_BuildTagRef( sceDmaTag* pTag, u32 RefAddress, s32 NBytes )
{
    // Make sure size is 16 byte multiple
    ASSERT( (NBytes & 0x0F) == 0 );
    pTag->qwc  = (u16)(NBytes>>4);
    pTag->id   = 0x30;
    pTag->next = (sceDmaTag*)RefAddress;
    pTag->mark = 0;
    pTag->p[0] = 0;
    pTag->p[1] = 0;
}

//============================================================================

void DMAHELP_BuildTagRefe( sceDmaTag* pTag, u32 RefAddress, s32 NBytes )
{
    // Make sure size is 16 byte multiple
    ASSERT( (NBytes & 0x0F) == 0 );
    pTag->qwc  = (u16)(NBytes>>4);
    pTag->id   = 0x00;
    pTag->next = (sceDmaTag*)RefAddress;
    pTag->mark = 0;
    pTag->p[0] = 0;
    pTag->p[1] = 0;
}

//=========================================================================

void DMAHELP_BuildTagCall   ( sceDmaTag* pTag, u32 NewAddress, s32 NBytes )
{
    // Make sure size is 16 byte multiple
    ASSERT( (NBytes & 0x0F) == 0 );
    pTag->qwc  = (u16)(NBytes>>4);
    pTag->id   = 0x50;
    pTag->next = (sceDmaTag*)NewAddress;
    pTag->mark = 0;
    pTag->p[0] = 0;
    pTag->p[1] = 0;
}

//=========================================================================

void DMAHELP_BuildTagRet    ( sceDmaTag* pTag, s32 NBytes )
{
    // Make sure size is 16 byte multiple
    ASSERT( (NBytes & 0x0F) == 0 );
    pTag->qwc  = (u16)(NBytes>>4);
    pTag->id   = 0x60;
    pTag->next = 0;
    pTag->mark = 0;
    pTag->p[0] = 0;
    pTag->p[1] = 0;
}

//=========================================================================
//=========================================================================

enum ImmErrors
{
    DMAIMMEDIATE_ERROR_NONE = 0,
    DMAIMMEDIATE_ERROR_UNALIGNED,
    DMAIMMEDIATE_ERROR_INVALID_DLIST_INDEX,
    DMAIMMEDIATE_ERROR_OUT_OF_DLIST_SLOTS,
    DMAIMMEDIATE_ERROR_DLIST_SIZE_TOO_BIG,
    DMAIMMEDIATE_ERROR_ENTRY_OUT_OF_BOUNDS,
    DMAIMMEDIATE_ERROR_INVALID_ACCESS,
    DMAIMMEDIATE_ERROR_INVALID_DATA,
};

static ImmErrors    s_LastImmediateErrorIndex   =   DMAIMMEDIATE_ERROR_NONE;
static s32          s_LastImmediateErrorDList   =   -1;
static u32          s_LastImmediateErrorLine    =   0;
static u32          s_ImmediateErrorCount       =   0;

static const s32    kMaxImmediateDLists         =   4;

struct ImmDList
{
    xbool           bActive;
    xbool           bIsEnded;
    byte            *pDataStart;            // where the allocated data lives so it may be properly freed
    u32             DataSize;               // how much is *actually* allocated
    byte            *pAdjustedStart;        // pointer to start of actual dlist after alignment
    byte            *pCurrent;              // walking pointer for adding to dlist
};

static ImmDList     s_ImmDLists[ kMaxImmediateDLists ];

void DMAIMMEDIATE_Init( void )
{
    x_memset( s_ImmDLists, 0, sizeof( ImmDList )*kMaxImmediateDLists );

    s_LastImmediateErrorIndex = DMAIMMEDIATE_ERROR_NONE;
    s_LastImmediateErrorDList = -1;
    s_LastImmediateErrorLine = 0;
    s_ImmediateErrorCount = 0;
}

void DMAIMMEDIATE_Kill( void )
{
    s32 i;
    for (i=0; i<kMaxImmediateDLists; ++i)
    {
        if (s_ImmDLists[ i ].bActive)
        {
            DMAIMMEDIATE_KillDisplayList( i );
        }
    }
}

#ifdef X_DEBUG
static void DMAIMMEDIATE_BuildError( xbool bTest, s32 iDListIndex, ImmErrors iError, u32 nLineNum )
{
    if (!bTest)
    {
        s_LastImmediateErrorIndex = iError;
        s_LastImmediateErrorDList = iDListIndex;
        s_LastImmediateErrorLine = nLineNum;
        s_ImmediateErrorCount ++;
    }
}
#endif

#ifdef X_DEBUG
#define DMAIMMEDIATE_ERRCHECK(test,dl,err)    DMAIMMEDIATE_BuildError(test,dl,err,__LINE__)
#else
#define DMAIMMEDIATE_ERRCHECK(test,dl,err)    
#endif

//=========================================================================

s32 DMAIMMEDIATE_InitDisplayList( s32 size )
{
    // find an empty slot
    //  allocate enough space using SMEM
    //  

    s32 i;
    for (i=0; i<kMaxImmediateDLists; ++i)
    {
        if (!s_ImmDLists[ i ].bActive)
        {
            s_ImmDLists[ i ].bActive = TRUE;
            s_ImmDLists[ i ].bIsEnded = FALSE;
            SMEM_StackPushMarker();
            s_ImmDLists[ i ].pDataStart = SMEM_StackAlloc( size + 16 );
            s_ImmDLists[ i ].DataSize = (u32)size;
            s_ImmDLists[ i ].pCurrent = s_ImmDLists[ i ].pDataStart;
            while (((u32)s_ImmDLists[ i ].pCurrent) & 0x0F)
                s_ImmDLists[ i ].pCurrent ++;
            s_ImmDLists[ i ].pAdjustedStart = s_ImmDLists[ i ].pCurrent;
            return i;
        }
    }

    DMAIMMEDIATE_ERRCHECK( 0, -1, DMAIMMEDIATE_ERROR_OUT_OF_DLIST_SLOTS );

    return -1;
}

//=========================================================================

xbool DMAIMMEDIATE_SendDisplayList( s32 iDListIndex )
{
    DMAIMMEDIATE_ERRCHECK( (iDListIndex>=0 && iDListIndex<kMaxImmediateDLists), iDListIndex, DMAIMMEDIATE_ERROR_INVALID_DLIST_INDEX );
    

    sceDmaChan *pDMAChannel = sceDmaGetChan(SCE_DMA_VIF1);
    
	while( !DMAMAN_AreTasksFinished() );        //  intentionally left empty

    sceGsSyncPath( 0, 0 );
    DMAMAN_HandlersOff();

    // Terminate our temporary display list.
    if (!s_ImmDLists[ iDListIndex ].bIsEnded)
    {
        DMAIMMEDIATE_DisplayListEnd( iDListIndex );
    }

    // Flush the cache so all data is in main memory.
    FlushCache( WRITEBACK_DCACHE );

    // Execute the temporary display list
    pDMAChannel->chcr.TTE = 0;
    pDMAChannel->chcr.TIE = 1;
    u32 C = *D_STAT;

    // If the channel interrupt mask bit is not set, then turn it on.
    if( !(C & 0x20000) )
    {
        *D_STAT = 0x20000;
    }

    sceDmaSend( pDMAChannel, s_ImmDLists[ iDListIndex ].pAdjustedStart );
    asm __volatile__
    ("
        sync.p
        sync.l
    ");

    sceGsSyncPath( 0, 0 );
    PS2_WaitVSync();

    return TRUE;
}

//=========================================================================

xbool DMAIMMEDIATE_KillDisplayList( s32 iDListIndex )
{
    DMAIMMEDIATE_ERRCHECK( (iDListIndex>=0 && iDListIndex<kMaxImmediateDLists), iDListIndex, DMAIMMEDIATE_ERROR_INVALID_DLIST_INDEX );

    if (iDListIndex < 0 || iDListIndex >= kMaxImmediateDLists)
        return FALSE;

    s_ImmDLists[ iDListIndex ].bActive = FALSE;
    SMEM_StackPopToMarker();
    return TRUE;
}

//=========================================================================

xbool DMAIMMEDIATE_AddDisplayListRef( s32 iDListIndex, s32 nBytes, u32 RefAddress, xbool bUseRefe, xbool bUseAbsoluteSize )
{
    // Make sure size is 16 byte multiple
    DMAIMMEDIATE_ERRCHECK( ((nBytes & 0x0F) == 0), iDListIndex, DMAIMMEDIATE_ERROR_UNALIGNED );
    DMAIMMEDIATE_ERRCHECK( (iDListIndex>=0 && iDListIndex<kMaxImmediateDLists), iDListIndex, DMAIMMEDIATE_ERROR_INVALID_DLIST_INDEX );
    if (iDListIndex >= 0 && iDListIndex < kMaxImmediateDLists)
    {
        DMAIMMEDIATE_ERRCHECK( (s_ImmDLists[ iDListIndex ].pCurrent < s_ImmDLists[ iDListIndex ].pDataStart + s_ImmDLists[ iDListIndex ].DataSize), iDListIndex, DMAIMMEDIATE_ERROR_ENTRY_OUT_OF_BOUNDS );
        DMAIMMEDIATE_ERRCHECK( (s_ImmDLists[ iDListIndex ].bActive == TRUE), iDListIndex, DMAIMMEDIATE_ERROR_INVALID_ACCESS );
    }
    DMAIMMEDIATE_ERRCHECK( (RefAddress != 0), iDListIndex, DMAIMMEDIATE_ERROR_INVALID_DATA );

    if ((nBytes & 0x0F) != 0)
        return FALSE;
    if ((iDListIndex<0) || (iDListIndex>=kMaxImmediateDLists))
        return FALSE;
    if (s_ImmDLists[ iDListIndex ].pCurrent >= (s_ImmDLists[ iDListIndex ].pDataStart + s_ImmDLists[ iDListIndex ].DataSize))
        return FALSE;
    if (!s_ImmDLists[ iDListIndex ].bActive)
        return FALSE;
    if (RefAddress == NULL)
        return FALSE;

    sceDmaTag * pTag = (sceDmaTag *)s_ImmDLists[ iDListIndex ].pCurrent;
    s_ImmDLists[ iDListIndex ].pCurrent += sizeof( sceDmaTag );
                            
    if (bUseRefe)
        pTag->id   = 0x00;
    else
        pTag->id   = 0x30;
    if (bUseAbsoluteSize)
        pTag->qwc = (u16)nBytes;
    else
        pTag->qwc = (u16)(nBytes>>4);
    pTag->next = (sceDmaTag*)RefAddress;
    pTag->mark = 0;
    pTag->p[0] = 0;
    pTag->p[1] = 0;
    return TRUE;
}

//=========================================================================
xbool DMAIMMEDIATE_AddDisplayListCont( s32 iDListIndex, s32 nBytes )
{
    // Make sure size is 16 byte multiple
    DMAIMMEDIATE_ERRCHECK( ((nBytes & 0x0F) == 0), iDListIndex, DMAIMMEDIATE_ERROR_UNALIGNED );
    DMAIMMEDIATE_ERRCHECK( (iDListIndex>=0 && iDListIndex<kMaxImmediateDLists), iDListIndex, DMAIMMEDIATE_ERROR_INVALID_DLIST_INDEX );
    if (iDListIndex >= 0 && iDListIndex < kMaxImmediateDLists)
    {
        DMAIMMEDIATE_ERRCHECK( (s_ImmDLists[ iDListIndex ].pCurrent < s_ImmDLists[ iDListIndex ].pDataStart + s_ImmDLists[ iDListIndex ].DataSize), iDListIndex, DMAIMMEDIATE_ERROR_ENTRY_OUT_OF_BOUNDS );
        DMAIMMEDIATE_ERRCHECK( (s_ImmDLists[ iDListIndex ].bActive == TRUE), iDListIndex, DMAIMMEDIATE_ERROR_INVALID_ACCESS );
    }

    if ((nBytes & 0x0F) != 0)
        return FALSE;
    if ((iDListIndex<0) || (iDListIndex>=kMaxImmediateDLists))
        return FALSE;
    if (s_ImmDLists[ iDListIndex ].pCurrent >= (s_ImmDLists[ iDListIndex ].pDataStart + s_ImmDLists[ iDListIndex ].DataSize))
        return FALSE;
    if (!s_ImmDLists[ iDListIndex ].bActive)
        return FALSE;

    sceDmaTag * pTag = (sceDmaTag *)s_ImmDLists[ iDListIndex ].pCurrent;
    s_ImmDLists[ iDListIndex ].pCurrent += sizeof( sceDmaTag );

    pTag->qwc  = (u16)(nBytes>>4);
    pTag->id   = 0x10;
    pTag->next = NULL;
    pTag->mark = 0;
    pTag->p[0] = 0;
    pTag->p[1] = 0;
    return TRUE;
}

//=========================================================================

xbool DMAIMMEDIATE_AddDisplayListCall( s32 iDListIndex, s32 nBytes, u32 RefAddress )
{
    // Make sure size is 16 byte multiple
    DMAIMMEDIATE_ERRCHECK( ((nBytes & 0x0F) == 0), iDListIndex, DMAIMMEDIATE_ERROR_UNALIGNED );
    DMAIMMEDIATE_ERRCHECK( (iDListIndex>=0 && iDListIndex<kMaxImmediateDLists), iDListIndex, DMAIMMEDIATE_ERROR_INVALID_DLIST_INDEX );
    if (iDListIndex >= 0 && iDListIndex < kMaxImmediateDLists)
    {
        DMAIMMEDIATE_ERRCHECK( (s_ImmDLists[ iDListIndex ].pCurrent < s_ImmDLists[ iDListIndex ].pDataStart + s_ImmDLists[ iDListIndex ].DataSize), iDListIndex, DMAIMMEDIATE_ERROR_ENTRY_OUT_OF_BOUNDS );
        DMAIMMEDIATE_ERRCHECK( (s_ImmDLists[ iDListIndex ].bActive == TRUE), iDListIndex, DMAIMMEDIATE_ERROR_INVALID_ACCESS );
    }
    DMAIMMEDIATE_ERRCHECK( (RefAddress != 0), iDListIndex, DMAIMMEDIATE_ERROR_INVALID_DATA );

    if ((nBytes & 0x0F) != 0)
        return FALSE;
    if ((iDListIndex<0) || (iDListIndex>=kMaxImmediateDLists))
        return FALSE;
    if (s_ImmDLists[ iDListIndex ].pCurrent >= (s_ImmDLists[ iDListIndex ].pDataStart + s_ImmDLists[ iDListIndex ].DataSize))
        return FALSE;
    if (!s_ImmDLists[ iDListIndex ].bActive)
        return FALSE;
    if (RefAddress == NULL)
        return FALSE;

    sceDmaTag * pTag = (sceDmaTag *)s_ImmDLists[ iDListIndex ].pCurrent;
    s_ImmDLists[ iDListIndex ].pCurrent += sizeof( sceDmaTag );

    pTag->qwc  = (u16)(nBytes>>4);
    pTag->id   = 0x50;
    pTag->next = (sceDmaTag*)RefAddress;
    pTag->mark = 0;
    pTag->p[0] = 0;
    pTag->p[1] = 0;
    return TRUE;
}

//=========================================================================

xbool DMAIMMEDIATE_DisplayListEnd( s32 iDListIndex )
{
    DMAIMMEDIATE_ERRCHECK( (iDListIndex>=0 && iDListIndex<kMaxImmediateDLists), iDListIndex, DMAIMMEDIATE_ERROR_INVALID_DLIST_INDEX );
    if (iDListIndex >= 0 && iDListIndex < kMaxImmediateDLists)
    {
        DMAIMMEDIATE_ERRCHECK( (s_ImmDLists[ iDListIndex ].pCurrent < s_ImmDLists[ iDListIndex ].pDataStart + s_ImmDLists[ iDListIndex ].DataSize), iDListIndex, DMAIMMEDIATE_ERROR_ENTRY_OUT_OF_BOUNDS );
        DMAIMMEDIATE_ERRCHECK( (s_ImmDLists[ iDListIndex ].bActive == TRUE), iDListIndex, DMAIMMEDIATE_ERROR_INVALID_ACCESS );
    }

    if ((iDListIndex<0) || (iDListIndex>=kMaxImmediateDLists))
        return FALSE;
    if (s_ImmDLists[ iDListIndex ].pCurrent >= (s_ImmDLists[ iDListIndex ].pDataStart + s_ImmDLists[ iDListIndex ].DataSize))
        return FALSE;
    if (!s_ImmDLists[ iDListIndex ].bActive)
        return FALSE;

    sceDmaTag * pTag = (sceDmaTag *)s_ImmDLists[ iDListIndex ].pCurrent;
    s_ImmDLists[ iDListIndex ].pCurrent += sizeof( sceDmaTag );
    s_ImmDLists[ iDListIndex ].bIsEnded = TRUE;

    pTag->qwc  = 0;
    pTag->id   = 0x70;
    pTag->next = 0;
    pTag->mark = 17;
    pTag->p[0] = 0;
    pTag->p[1] = 0;
    return TRUE;
}

//=========================================================================
// Special function to dump error info to the TTY and ASSERT on badness
void DMAIMMEDIATE_ReportLastError()
{
#if X_DEBUG
    if (s_LastImmediateErrorIndex != DMAIMMEDIATE_ERROR_NONE)
    {
        x_printf( "DMA Immediate Error - \n" );
        if (s_ImmediateErrorCount > 1)
        {
            x_printf( " ** MULTIPLE ERRORS **\n");
            x_printf( "Last Error - \n" );
        }
        if (s_LastImmediateErrorIndex == DMAIMMEDIATE_ERROR_UNALIGNED)
            x_printf( "  DList #%d added out-of-alignment object on line %d of DMAHELP.CPP\n", s_LastImmediateErrorDList, s_LastImmediateErrorLine );
        else if (s_LastImmediateErrorIndex == DMAIMMEDIATE_ERROR_INVALID_DLIST_INDEX)
            x_printf( "  DList #%d referenced on line %d of DMAHELP.CPP\n", s_LastImmediateErrorDList, s_LastImmediateErrorLine );
        else if (s_LastImmediateErrorIndex == DMAIMMEDIATE_ERROR_OUT_OF_DLIST_SLOTS)
        {
            x_printf( "  Attempt to add too many Immediate DLists\n" );
            x_printf( "  Either increment the DList count in DMAHELP.CPP\n" );
            x_printf( "  Or ensure all created lists are getting shut down.\n");
        }
        else if (s_LastImmediateErrorIndex == DMAIMMEDIATE_ERROR_ENTRY_OUT_OF_BOUNDS)
            x_printf( "  Attempt to write out of DList %d memory bounds at DMAHELP.CPP line %d\n", s_LastImmediateErrorDList, s_LastImmediateErrorLine );
        else if (s_LastImmediateErrorIndex == DMAIMMEDIATE_ERROR_DLIST_SIZE_TOO_BIG)
            x_printf( "  Out of memory on DList Init - DMAHELP.CPP line %d\n", s_LastImmediateErrorLine );
        else if (s_LastImmediateErrorIndex == DMAIMMEDIATE_ERROR_INVALID_ACCESS)
            x_printf( "  Attempt to access inactive DList %d at DMAHELP.CPP line %d\n", s_LastImmediateErrorDList, s_LastImmediateErrorLine );
        else if (s_LastImmediateErrorIndex == DMAIMMEDIATE_ERROR_INVALID_DATA)
            x_printf( "  Attempting to add invalid data to DList %d at DMAHELP.CPP line %d\n", s_LastImmediateErrorDList, s_LastImmediateErrorLine );
        else
            x_printf( "  UNKNOWN -- WTF?\n" );
        s_ImmediateErrorCount = 0;
        s_LastImmediateErrorIndex = DMAIMMEDIATE_ERROR_NONE;
        ASSERT( FALSE );
    }
#endif
}
