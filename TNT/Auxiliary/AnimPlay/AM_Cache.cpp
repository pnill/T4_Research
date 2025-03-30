///////////////////////////////////////////////////////////////////////////
//
// AM_CACHE.C
//
///////////////////////////////////////////////////////////////////////////

#include "x_debug.hpp"
#include "x_stdio.hpp"
#include "x_math.hpp"
#include "x_memory.hpp"
#include "x_time.hpp"
#include "x_plus.hpp"
#include "acccmp.h"
#include "AM_Play.h"
#include "AM_MocapPlayer.h"
#include "AM_Cache.h"
#include "AM_Stats.h"

#if defined( TARGET_DOLPHIN )
    #include "AUX_RAM.hpp"
#endif


///////////////////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////////////////

#define FRAMES_FRACTIONAL

#define DECOMP_BUFFER_SIZE  (4 * 1024)


///////////////////////////////////////////////////////////////////////////
// STRUCTURES
///////////////////////////////////////////////////////////////////////////

typedef struct
{
    f32         LastUsed;
    s16*        DataPtr;
    s16         Locks;
    s32         AnimGroupID;
    s32         AnimID;
    s32         BlockID;
} decomp_buffer;


///////////////////////////////////////////////////////////////////////////
// GLOBALS
///////////////////////////////////////////////////////////////////////////

static xbool            AMCACHE_Inited              = FALSE;
static decomp_buffer*   AMCACHE_DecompBuffer        = NULL;
static byte*            AMCACHE_StaticBuffers       = NULL;
static s32              AMCACHE_NStaticBuffers;
static s32              AMCACHE_MaxStreams;
static s32              AMCACHE_MaxFramesPerBlock;
static s32              AMCACHE_BufferSize;
static s32              AMCACHE_NDecompBuffers;
static byte*            AMCACHE_CompDataPtr         = NULL;

//--- profiling data
static s32              AMCACHE_TotalDecompBuffersUsed;

///////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////

void AMCACHE_DecompBlock( anim_group* AnimGroup, s32 AnimID, s32 BlockID, s32 DB )
{
    s32         NFrames;
    s32         CompDataOffset;
    s32         CompDataSize;
    anim_info*  AnimInfo;
    s32         i;
    s32         NFails;
//    s32         NStreams;


    // ASSERT on inputs and get AnimInfo
    ASSERT( AnimID >= 0 && AnimID < AnimGroup->NAnimations );
    ASSERT( DB >= 0 && DB < AMCACHE_NDecompBuffers );
    ASSERT( AMCACHE_DecompBuffer[DB].Locks == 0 );
    ASSERT( AMCACHE_DecompBuffer[DB].DataPtr != NULL );
    AnimInfo = &AnimGroup->AnimInfo[AnimID];
    ASSERT( BlockID >= 0 && (BlockID <= (AnimInfo->ExportNFrames / (AnimGroup->FramesPerBlock-1))) );

    // Compute compressed data size and offset in raw binary file
    CompDataOffset  = AnimInfo->BlockOffset;
    for( i = 0; i < BlockID; i++ )
        CompDataOffset += ((s32)(AnimGroup->AnimBlockSize[AnimInfo->BlockSizeIndex + i]))*16;

    CompDataSize    = ((s32)(AnimGroup->AnimBlockSize[AnimInfo->BlockSizeIndex + BlockID]))*16;

	s32 NFrames1 = AnimInfo->ExportNFrames - BlockID*(AnimGroup->FramesPerBlock-1);
	s32	NFrames2 = AnimGroup->FramesPerBlock;
	NFrames	= NFrames1 < NFrames2 ? NFrames1 : NFrames2;

    AMCACHE_DecompBuffer[DB].Locks       = 0;
    AMCACHE_DecompBuffer[DB].AnimGroupID = (((u32)AnimGroup)&0x00FFFFFF);
    AMCACHE_DecompBuffer[DB].AnimID      = (s16)AnimID;
    AMCACHE_DecompBuffer[DB].BlockID     = (s16)BlockID;

    // Do decompression
    NFails = 0;
    while( 1 )
    {
        // Load raw data to memory and decompress
        #ifdef ANIMPLAY_PRIMARY_STORAGE
            #if defined( TARGET_DOLPHIN )
                // Read compressed block into Main RAM from AUXRAM
                void* pARAMSrc      = (void*)((u32)AnimGroup->AnimCompData + (u32)CompDataOffset);
                void* pARAMAlignSrc = (void*)AUXRAM_ROUNDDOWN32( pARAMSrc );
                u32   ARAMReadSize  = AUXRAM_ROUNDUP32( (u32)CompDataSize + ((u32)pARAMSrc - (u32)pARAMAlignSrc) );

                ASSERT( AMCACHE_CompDataPtr );
                ASSERT( ARAMReadSize < DECOMP_BUFFER_SIZE );
                AUXRAM_ReadFromAram( AMCACHE_CompDataPtr, pARAMAlignSrc, ARAMReadSize );
            #else
                AMCACHE_CompDataPtr = (byte*)(((u32)AnimGroup->AnimCompData)+CompDataOffset);
            #endif
        #else
            // Read compressed block into RAM
            ASSERT( AMCACHE_CompDataPtr );
            ASSERT( CompDataSize < DECOMP_BUFFER_SIZE );
            x_fseek( (X_FILE*)AnimGroup->AnimCompData, CompDataOffset, X_SEEK_SET );
            x_fread( AMCACHE_CompDataPtr, CompDataSize, 1, (X_FILE*)AnimGroup->AnimCompData );
        #endif

        #if defined( ANIMPLAY_PRIMARY_STORAGE ) && defined( TARGET_DOLPHIN )
            byte* pCompData = (byte*)(AMCACHE_CompDataPtr + ((u32)pARAMSrc - (u32)pARAMAlignSrc));
        #else
            byte* pCompData = (byte*)AMCACHE_CompDataPtr;
        #endif

		//---	if the data is realy uncompressed, simply copy it to the uncompressed buffer, otherwise decompress the data.
		if( AnimInfo->Flags & ANIMINFO_FLAG_UNCOMPRESSED )
		{
			//---	copy out the uncompressed data
			x_memcpy( AMCACHE_DecompBuffer[DB].DataPtr, pCompData, AnimInfo->NStreams*AnimGroup->FramesPerBlock*sizeof(s16) );
			break;
		}
		else
		{
			//====================
			AMSTATS_DECOMP_START//==== PROFILING
			//====================

			if( ACCCMP_Decode( pCompData, (s16*)AMCACHE_DecompBuffer[DB].DataPtr, AnimInfo->NStreams, (s16)NFrames ) )
			{
				//================================================
				AMSTATS_DECOMP_END(NFrames,AnimInfo->NStreams);//==== PROFILING
				//================================================
				break;
			}
		}

        NFails++;
        if( NFails == 5 )
        {
            ASSERTS( FALSE, "Anim decompression has failed!!!" );
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////

s16* AMCACHE_LockAnimFrames( anim_group* AnimGroup, s32 AnimID, s32 BlockID )
{
    s32 i,b;
    s32 AnimGroupID;

    ASSERT( AMCACHE_Inited );

    // Search decomp buffers for a match.
    AnimGroupID = ((u32)AnimGroup) & 0x00FFFFFF;
    for( b = 0; b < AMCACHE_NDecompBuffers; b++ )
    {
        if( (AMCACHE_DecompBuffer[b].AnimGroupID == AnimGroupID) &&
            (AMCACHE_DecompBuffer[b].AnimID      == AnimID     ) &&
            (AMCACHE_DecompBuffer[b].BlockID     == BlockID    ) &&
            (AMCACHE_DecompBuffer[b].DataPtr     != NULL       ) )
        {
            break;
        }
    }

    // If buffer was not found, decompress the requested block into the
    // 'best' free buffer
    if( b == AMCACHE_NDecompBuffers )
    {
        f32 LastUsed;

        // Search through static buffers looking for one that is unlocked
        // and has the lowest LastUsed value (Oldest).
        b = -1;
        LastUsed = 9999999999.0f;
        for( i = 0; i < AMCACHE_NStaticBuffers; i++ )
        {
            if( (AMCACHE_DecompBuffer[i].Locks == 0) &&
                (AMCACHE_DecompBuffer[i].LastUsed < LastUsed) )
            {
                b = i;
                LastUsed = AMCACHE_DecompBuffer[i].LastUsed;
            }
        }

        // No static buffer available, so choose an extra buffer that
        // has no locks.
        if( b == -1 )
        {
            for( i = AMCACHE_NStaticBuffers; i < AMCACHE_NDecompBuffers; i++ )
            {
                if( AMCACHE_DecompBuffer[i].Locks == 0 )
                    break;
            }
            ASSERT( i != AMCACHE_NDecompBuffers );
            b = i;

            // Allocate the 'extra' buffer
            AMCACHE_DecompBuffer[b].LastUsed    = (f32)x_GetTime();
            AMCACHE_DecompBuffer[b].Locks       = 0;
            AMCACHE_DecompBuffer[b].AnimGroupID = -1;
            AMCACHE_DecompBuffer[b].AnimID      = -1;
            AMCACHE_DecompBuffer[b].BlockID     = -1;
            AMCACHE_DecompBuffer[b].DataPtr     = (s16*)x_malloc( AMCACHE_BufferSize );
            ASSERT( AMCACHE_DecompBuffer[b].DataPtr != NULL );
        }

        ASSERT( b != -1 );
        AMCACHE_DecompBlock( AnimGroup, AnimID, BlockID, b );
    }

    // Touch block
    AMCACHE_DecompBuffer[b].LastUsed = (f32)x_GetTime();

    // Add a lock to the buffer
    AMCACHE_DecompBuffer[b].Locks++;

    // Record the number of buffers used for profiling buffer usage
    if( b >= AMCACHE_TotalDecompBuffersUsed )
        AMCACHE_TotalDecompBuffersUsed = b+1;

    return AMCACHE_DecompBuffer[b].DataPtr;
}

///////////////////////////////////////////////////////////////////////////

void AMCACHE_UnlockAnimFrames( anim_group* AnimGroup, s32 AnimID, s32 BlockID )
{
    s32 i;
    s32 AnimGroupID;

    ASSERT( AMCACHE_Inited );

    // Find which decomp_block if any user is talking about
    AnimGroupID = ((u32)AnimGroup) & 0x00FFFFFF;
    for( i = 0; i < AMCACHE_NDecompBuffers; i++ )
    {
        if( (AMCACHE_DecompBuffer[i].AnimGroupID == AnimGroupID)  &&
            (AMCACHE_DecompBuffer[i].AnimID      == AnimID     )  &&
            (AMCACHE_DecompBuffer[i].BlockID     == BlockID    ) )
        {
            break;
        }
    }
    ASSERT( i < AMCACHE_NDecompBuffers );

    // Touch block
    AMCACHE_DecompBuffer[i].LastUsed = (f32)x_GetTime();

    // Remove a lock from this buffer.
    AMCACHE_DecompBuffer[i].Locks--;

    // If all locks are gone and it is an extra buffer, clear the info
    // and deallocate the buffer
    if( (AMCACHE_DecompBuffer[i].Locks == 0) && (i >= AMCACHE_NStaticBuffers) )
    {
        x_free( AMCACHE_DecompBuffer[i].DataPtr );
        AMCACHE_DecompBuffer[i].DataPtr     = NULL;
        AMCACHE_DecompBuffer[i].LastUsed    = 0;
        AMCACHE_DecompBuffer[i].Locks       = 0;
        AMCACHE_DecompBuffer[i].AnimGroupID = -1;
        AMCACHE_DecompBuffer[i].AnimID      = -1;
        AMCACHE_DecompBuffer[i].BlockID     = -1;
    }
}

///////////////////////////////////////////////////////////////////////////
void AMCACHE_LockBlock( anim_group* pAnimGroup, anim_info* pAnimInfo, SMotionBlock& rBlock, f32 ExportFrame )
{
    ASSERT( pAnimGroup );
    ASSERT( pAnimInfo );

    // Confirm that no block is locked
    ASSERT( rBlock.pData == NULL );

    // Load new block
    s32 FramesPerBlock = pAnimGroup->FramesPerBlock-1;
    rBlock.ID		= ((s32)ExportFrame) / FramesPerBlock;
    rBlock.pData	= AMCACHE_LockAnimFrames( pAnimGroup, pAnimInfo->ID, rBlock.ID );
    ASSERT( rBlock.pData );

    // Compute block bounds
    rBlock.Frame0 = rBlock.ID * FramesPerBlock;
    rBlock.FrameN = MIN( (rBlock.ID + 1)*FramesPerBlock, pAnimInfo->ExportNFrames - 1 );
}

///////////////////////////////////////////////////////////////////////////
void AMCACHE_UnlockBlock( anim_group* pAnimGroup, anim_info* pAnimInfo, SMotionBlock& rBlock )
{
    ASSERT( pAnimGroup );
    ASSERT( pAnimInfo );

    // Confirm that block is present
    ASSERT( rBlock.pData );

    // Release current block
    AMCACHE_UnlockAnimFrames( pAnimGroup, pAnimInfo->ID, rBlock.ID );
    rBlock.pData = NULL;
}

///////////////////////////////////////////////////////////////////////////
void AMCACHE_InitModule( s32 TotalStaticBuffers,
                         s32 TotalDynamicBuffers,
                         s32 MaxStreams,
                         s32 MaxFramesPerBlock )
{
    s32 i;
    s32 TotalNBuffers;

//    ASSERT( !AMCACHE_Inited );
    ASSERT( TotalStaticBuffers > 0 );
    ASSERT( TotalDynamicBuffers >= 0 );
    ASSERT( MaxStreams >= 8 );

    //--- if the module has already been inited, make sure that the new values are worth while
    if( AMCACHE_Inited &&
        (TotalStaticBuffers == AMCACHE_NStaticBuffers) &&
        ((TotalStaticBuffers + TotalDynamicBuffers) == AMCACHE_NDecompBuffers) &&
        (MaxStreams <= AMCACHE_MaxStreams) &&
        (MaxFramesPerBlock <= AMCACHE_MaxFramesPerBlock) )
    {
        return;
    }

    //--- if the module has already been inited, kill the module before continueing
    if( AMCACHE_Inited )
    {
        //--- make sure that none of the decompression buffers have yet been used
        for( i = 0; i < AMCACHE_NDecompBuffers; i++ )
        {
            if( AMCACHE_DecompBuffer[i].AnimGroupID != -1 )
                break;
        }

        ASSERTS( (i == AMCACHE_NDecompBuffers), "Call ANIM_UseAnimGroup for all used groups before setting any animations" );

        AMCACHE_KillModule();
    }

    AMCACHE_Inited              = TRUE;
    TotalNBuffers               = TotalStaticBuffers + TotalDynamicBuffers;
    AMCACHE_NStaticBuffers      = TotalStaticBuffers;

    AMCACHE_MaxStreams          = MaxStreams;
    AMCACHE_MaxFramesPerBlock   = MaxFramesPerBlock;
    AMCACHE_BufferSize          = AMCACHE_MaxStreams * AMCACHE_MaxFramesPerBlock * sizeof(s16);

    AMCACHE_TotalDecompBuffersUsed  = 0;

    //
    // Allocate the decompression buffer headers
    //
    AMCACHE_NDecompBuffers = TotalNBuffers;
    AMCACHE_DecompBuffer   = (decomp_buffer*)x_malloc( sizeof(decomp_buffer) * TotalNBuffers );
    ASSERT( AMCACHE_DecompBuffer != NULL );

    //
    // Clear the decompression buffers
    //
    for( i = 0; i < AMCACHE_NDecompBuffers; i++ )
    {
        AMCACHE_DecompBuffer[i].LastUsed    = 0;
        AMCACHE_DecompBuffer[i].Locks       = 0;
        AMCACHE_DecompBuffer[i].AnimGroupID = -1;
        AMCACHE_DecompBuffer[i].AnimID      = -1;
        AMCACHE_DecompBuffer[i].BlockID     = -1;
        AMCACHE_DecompBuffer[i].DataPtr     = NULL;
    }

    //
    // Allocate the static buffers
    //
    AMCACHE_StaticBuffers = (byte*)x_malloc( AMCACHE_BufferSize * AMCACHE_NStaticBuffers );
	ASSERT( AMCACHE_StaticBuffers != NULL );

    for( i = 0; i < AMCACHE_NStaticBuffers; i++ )
    {
        AMCACHE_DecompBuffer[i].DataPtr = (s16*)((u32)AMCACHE_StaticBuffers + i * AMCACHE_BufferSize);
        ASSERT( AMCACHE_DecompBuffer[i].DataPtr );
    }

    //
    // Allocate the raw compressed data ptr
    //
#ifndef ANIMPLAY_PRIMARY_STORAGE
    AMCACHE_CompDataPtr = (byte*)x_malloc( DECOMP_BUFFER_SIZE );
    ASSERT( AMCACHE_CompDataPtr != NULL );
#else
  #if defined( TARGET_DOLPHIN )
    AMCACHE_CompDataPtr = (byte*)x_malloc( DECOMP_BUFFER_SIZE );
    ASSERT( AMCACHE_CompDataPtr != NULL );
  #else
    AMCACHE_CompDataPtr = NULL;
  #endif
#endif
}

///////////////////////////////////////////////////////////////////////////

void AMCACHE_KillModule( void )
{
    s32 i;

    ASSERT( AMCACHE_Inited );
    AMCACHE_Inited = FALSE;

    //
    // Release all the decompression buffers
    //
    for( i = 0; i < AMCACHE_NDecompBuffers; i++ )
    {
        AMCACHE_DecompBuffer[i].LastUsed    = 0;
        AMCACHE_DecompBuffer[i].Locks       = 0;
        AMCACHE_DecompBuffer[i].AnimGroupID = -1;
        AMCACHE_DecompBuffer[i].AnimID      = -1;
        AMCACHE_DecompBuffer[i].BlockID     = -1;

        //--- free any allocated memory in the dynamic buffers
        if( (i >= AMCACHE_NStaticBuffers) && AMCACHE_DecompBuffer[i].DataPtr )
            x_free( AMCACHE_DecompBuffer[i].DataPtr );

        AMCACHE_DecompBuffer[i].DataPtr = NULL;
    }

    if( AMCACHE_StaticBuffers ) x_free( AMCACHE_StaticBuffers );
    if( AMCACHE_DecompBuffer )  x_free( AMCACHE_DecompBuffer );
    AMCACHE_StaticBuffers       = NULL;
    AMCACHE_DecompBuffer        = NULL;
    AMCACHE_NDecompBuffers      = 0;
    AMCACHE_NStaticBuffers      = 0;
    AMCACHE_MaxFramesPerBlock   = 0;
    AMCACHE_BufferSize          = 0;

#ifndef ANIMPLAY_PRIMARY_STORAGE
    if( AMCACHE_CompDataPtr )   x_free( AMCACHE_CompDataPtr );

#elif defined( TARGET_DOLPHIN )
    if( AMCACHE_CompDataPtr )   x_free( AMCACHE_CompDataPtr );
#endif

    AMCACHE_CompDataPtr = NULL;
}

///////////////////////////////////////////////////////////////////////////
// FINISHED
///////////////////////////////////////////////////////////////////////////
