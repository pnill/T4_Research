////////////////////////////////////////////////////////////////////////////
//
// GC_SkinRender.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "Q_Engine.hpp"
#include "Q_VRAM.hpp"
#include "Q_SMEM.hpp"
#include "Q_GC.hpp"

#include "Skin.hpp"
#include "GC_SkinHelp.hpp"


////////////////////////////////////////////////////////////////////////////
// STRUCTURES
////////////////////////////////////////////////////////////////////////////

struct SSortSubMesh
{
    s16 SortKey;
    s16 Index;
};

struct GC_BufferVtxCount
{
    s32 NCPU1;
    s32 NCPU2;
    s32 NCPU3;
};


////////////////////////////////////////////////////////////////////////////
// PROTOTYPES
////////////////////////////////////////////////////////////////////////////

static void XFormVertsLockedCache( GCSubMeshGroup* pGroup,
                                   GCSkinVert_PN*  pFinalVtxData,
                                   s32             NCPUVerts );

static void CalcVertCounts( GC_BufferVtxCount& rBufVtxCount,
                            GC_BufferVtxCount& rCurVtxTotal,
                            s32                VertCount );

static void ApplyMorphDeltas( GCSkinVert_CPU* pVertData,
                              s32             CurVertID,
                              s32             NVerts,
                              s32&            rCurDeltaID,
                              GCSubMeshGroup* pGroup );

static void XFormVerts( register GCSkinVert_CPU* pVtxDataSrc,
                        register GCSkinVert_PN*  pVtxDataDst,
                        register GCPosMtx*       pMtxData,
                        register s32             NVertsCPU1,
                        register s32             NVertsCPU2,
                        register s32             NVertsCPU3 );


////////////////////////////////////////////////////////////////////////////
// GCSKIN STATIC DATA
////////////////////////////////////////////////////////////////////////////

s32              GCSKIN::s_NMtxGroups     = 0;
s32              GCSKIN::s_NSubMeshes     = 0;
s32              GCSKIN::s_MaxMtxGroups   = 0;
s32              GCSKIN::s_MaxSubMeshes   = 0;
byte*            GCSKIN::s_pSkinBuffer    = NULL;
GCMtxGroup*      GCSKIN::s_pMtxGroups     = NULL;
GCSubMeshGroup*  GCSKIN::s_pSubmeshGroups = NULL;
s32              GCSKIN::s_FixedAlpha     = 0;
u32              GCSKIN::s_SaveGQR4       = GQR::LD_S8_ST_S8;
u32              GCSKIN::s_SaveGQR5       = GQR::LD_S16_ST_S16;
GCSKIN::SDMABuf  GCSKIN::s_DMABuf[DMA_BUFFER_COUNT];


////////////////////////////////////////////////////////////////////////////
// QSkin IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

void QSkin::OpenSkinBuffer( s32 MaxNMatrixSets, s32 MaxNSubMeshes )
{
    GCSKIN::CloseBuffer();

    //--- allocate space off the SMEM for the skin buffer
    GCSKIN::s_pSkinBuffer = SMEM_BufferAlloc( sizeof(GCMtxGroup) * MaxNMatrixSets +
                                              sizeof(GCSubMeshGroup) * MaxNSubMeshes +
                                              32 );  // add 32 for alignment purposes

    ASSERT( GCSKIN::s_pSkinBuffer != NULL );

    //--- Align buffer on 32-byte boundary
    GCSKIN::s_pSkinBuffer = (byte*)ALIGN_32( GCSKIN::s_pSkinBuffer );

    GCSKIN::s_MaxSubMeshes   = MaxNSubMeshes;
    GCSKIN::s_MaxMtxGroups   = MaxNMatrixSets;
    GCSKIN::s_pMtxGroups     = (GCMtxGroup*)(GCSKIN::s_pSkinBuffer);
    GCSKIN::s_pSubmeshGroups = (GCSubMeshGroup*)(GCSKIN::s_pSkinBuffer + (sizeof(GCMtxGroup) * MaxNMatrixSets));
}

//==========================================================================

void QSkin::RenderSkin( radian3* pBoneRots,
                        vector3& WorldPos,
                        radian   WorldYaw,
                        void*    pAnimGroupPtr,
                        xbool    Mirrored,
                        f32      WorldScale,
                        vector3* pBoneScales,
                        u32      UserData1,
                        u32      UserData2,
                        u32      UserData3)
{
    GCMtxGroup* pMatrices;

    //--- confirm we are in render mode and skin buffer is open
    ASSERT( ENG_GetRenderMode() );
    ASSERT( GCSKIN::IsBufferOpen() );

    //--- get pointer for matrix data
    pMatrices = GCSKIN::AddMtxGroup();

    //--- build the bone matrices
    BuildMatrices( pMatrices, pBoneRots, WorldPos, WorldYaw, Mirrored, WorldScale, pBoneScales, pAnimGroupPtr, 0, NULL, NULL );

    //--- render the skin with matrices built
    RenderSkin( pMatrices, UserData1, UserData2, UserData3, 0, NULL );
}

//==========================================================================

void QSkin::RenderSkin( GCMtxGroup*    pMatrices,
                        u32            UserData1,
                        u32            UserData2,
                        u32            UserData3,
                        s32            NShadowMtx,
                        GCPosMtxArray* pShadowMtxData )
{
    s32             i, j, k;
    s32             LastSubMesh;
    GCSubMeshGroup* pSubMeshGroup;

    //--- confirm we are in render mode and skin buffer is open
    ASSERT( ENG_GetRenderMode() );
    ASSERT( GCSKIN::IsBufferOpen() );

    ASSERT( (NShadowMtx <= 0) || ((NShadowMtx > 0) && (pShadowMtxData != NULL)) );

    //--- loop through meshes
    for( i = 0; i < m_NMeshes; ++i )
    {
        if( (m_pMeshes[i].Flags & SKIN_MESH_FLAG_VISIBLE) == 0 )
            continue;

        LastSubMesh = m_pMeshes[i].FirstSubMesh + m_pMeshes[i].NSubMeshes;

        //--- loop through mesh's submeshes and add them to render list
        for( j = m_pMeshes[i].FirstSubMesh; j < LastSubMesh; ++j )
        {
            //--- check if submesh is marked as visible
            if( (m_pSubMeshes[j].Flags & SKIN_SUBMESH_FLAG_VISIBLE) == 0 )
                continue;

            if( m_pSubMeshes[j].Flags & SKIN_SUBMESH_FLAG_SHADOW )
            {
                for( k = 0; k < NShadowMtx; ++k )
                {
                    pSubMeshGroup = GCSKIN::AddSubMeshGroup();

                    pSubMeshGroup->pMesh           = &m_pMeshes[i];
                    pSubMeshGroup->pSubMesh        = &m_pSubMeshes[j];
                    pSubMeshGroup->pMorphTgts      = NULL;
                    pSubMeshGroup->Flags           = m_pSubMeshes[j].Flags;
                    pSubMeshGroup->pShadowMatrices = &pShadowMtxData[k];
                    pSubMeshGroup->UserData1       = UserData1;
                    pSubMeshGroup->UserData2       = UserData2;
                    pSubMeshGroup->UserData3       = UserData3;
                    pSubMeshGroup->MeshID          = i;
                    pSubMeshGroup->pBitmap         = NULL;
                    pSubMeshGroup->pBitmap2        = NULL;
                }
            }
            else
            {
                pSubMeshGroup = GCSKIN::AddSubMeshGroup();

                pSubMeshGroup->pMesh      = &m_pMeshes[i];
                pSubMeshGroup->pSubMesh   = &m_pSubMeshes[j];
                pSubMeshGroup->pMorphTgts = &m_pMorphTargets[ m_pMeshes[i].FirstMorphTarget ];
                pSubMeshGroup->Flags      = m_pSubMeshes[j].Flags;
                pSubMeshGroup->pMatrices  = pMatrices;
                pSubMeshGroup->UserData1  = UserData1;
                pSubMeshGroup->UserData2  = UserData2;
                pSubMeshGroup->UserData3  = UserData3;
                pSubMeshGroup->MeshID     = i;

                ASSERT( m_pTextureArray != NULL );
                pSubMeshGroup->pBitmap = m_pTextureArray[ m_pSubMeshes[j].TextureID ];
                ASSERT( pSubMeshGroup->pBitmap != NULL );

                if( m_pSubMeshes[j].Flags & SKIN_SUBMESH_FLAG_ENVPASS )
                {
                    pSubMeshGroup->pBitmap2 = m_pTextureArray[ m_NTextures-1 ];
                    ASSERT( pSubMeshGroup->pBitmap2 != NULL );
                }
                else
                {
                    pSubMeshGroup->pBitmap2 = NULL;
                }
            }

        } //End SubMesh Loop

    } //End Mesh Loop

}

//==========================================================================

static void RadixSort( s32 NSubMeshes, SSortSubMesh* pSubMeshes )
{
    //--- NEED: 1) Sort key for every submesh
    //          2) Values to sort (either ptrs or indices) for each submesh

    #define NUM_BUCKETS     16
    #define BUCKET_AND      0x0F
    #define BITS_PER_PASS   4
    #define NUM_PASSES      4

    s32*            pBucketCount;
    SSortSubMesh*   pBuckets;
    s32             Pass;
    s32             BucketID;
    s32             Shifter;
    s32             SubMeshID;
    s32             Count;
    s32             BucketOffset;

    //--- allocate temporary space for the buckets and bucket counts
    SMEM_StackPushMarker();
    pBuckets     = (SSortSubMesh*)SMEM_StackAlloc( sizeof(SSortSubMesh) * NUM_BUCKETS * NSubMeshes );
    pBucketCount = (s32*)         SMEM_StackAlloc( sizeof(s32) * NUM_BUCKETS );
    ASSERT( (pBuckets != NULL) && (pBucketCount != NULL) );

    //--- do each pass
    Shifter = 0;
    for( Pass = 0; Pass < NUM_PASSES; ++Pass )
    {
        //--- empty the buckets
        for( BucketID = 0; BucketID < NUM_BUCKETS; ++BucketID )
            pBucketCount[BucketID] = 0;

        //--- put the submeshes in their buckets
        for( SubMeshID = 0; SubMeshID < NSubMeshes; ++SubMeshID )
        {
            BucketID     = (pSubMeshes[SubMeshID].SortKey >> Shifter) & BUCKET_AND;
            BucketOffset = BucketID * NSubMeshes;

            pBuckets[ BucketOffset + pBucketCount[BucketID] ] = pSubMeshes[SubMeshID];
            pBucketCount[BucketID]++;

            ASSERT( pBucketCount[BucketID] <= NSubMeshes );
        }

        //--- remerge the list from the buckets
        Count = 0;
        for( BucketID = 0; BucketID < NUM_BUCKETS; ++BucketID )
        {
            BucketOffset = BucketID*NSubMeshes;
            for( SubMeshID = 0; SubMeshID < pBucketCount[BucketID]; ++SubMeshID )
            {
                pSubMeshes[Count] = pBuckets[BucketOffset + SubMeshID];
                Count++;
            }
        }
        ASSERT( Count == NSubMeshes );

        //--- change shifter to inspect next bits
        Shifter += BITS_PER_PASS;
    }

    //--- free temporary space
    SMEM_StackPopToMarker();
}

//==========================================================================

void QSkin::FlushSkinBuffer( void )
{
    s32             i;
    GCSubMeshGroup* pSubMeshGroup;
    GCSubMeshGroup* pSubMeshArray;
    x_bitmap*       pCurBitmap    = (x_bitmap*)0xFFFFFFFF;
    x_bitmap*       pCurEnvBitmap = (x_bitmap*)0xFFFFFFFF;
    SSortSubMesh*   pSubMeshOrder;

    //--- Confirm we are in render mode
    ASSERT( ENG_GetRenderMode() );
    ASSERT( GCSKIN::IsBufferOpen() );

    //--- try to bail out early
    if( GCSKIN::IsBufferEmpty() )
    {
        GCSKIN::CloseBuffer();
        return;
    }

    GCSKIN::SetupLockedCache();

    pSubMeshArray = GCSKIN::GetSubMeshGroupPtr();

    //--- create a temp buffer to store the submesh order
    SMEM_StackPushMarker();
    pSubMeshOrder = (SSortSubMesh*)SMEM_StackAlloc( sizeof(SSortSubMesh) * GCSKIN::GetSubMeshCount() );
    ASSERT( pSubMeshOrder != NULL );

    //--- init sort structures
    for( i = 0; i < GCSKIN::GetSubMeshCount(); ++i )
    {
        if( pSubMeshArray[i].pBitmap != NULL )
        {
            ASSERT( pSubMeshArray[i].pBitmap->GetVRAMID() < 2048 );
            pSubMeshOrder[i].SortKey = pSubMeshArray[i].pBitmap->GetVRAMID();
        }
        else
        {
            pSubMeshOrder[i].SortKey  = (1 << 11);
        }

        //if( pSubMeshArray[i].Flags & SKIN_SUBMESH_FLAG_ )
        //    pSubMeshOrder[i].SortKey |= (1 << 12);

        if( pSubMeshArray[i].Flags & SKIN_SUBMESH_FLAG_SHADOW )
            pSubMeshOrder[i].SortKey |= (1 << 13);

        if( pSubMeshArray[i].Flags & SKIN_SUBMESH_FLAG_ALPHA )
            pSubMeshOrder[i].SortKey |= (1 << 14);

        if( pSubMeshArray[i].Flags & SKIN_SUBMESH_FLAG_MORPH )
            pSubMeshOrder[i].SortKey |= (1 << 15);

        pSubMeshOrder[i].Index = i;
    }

    //--- sort the submeshes by texture and render flags
    RadixSort( GCSKIN::GetSubMeshCount(), pSubMeshOrder );

    //--- setup a texture matrix for env. mapping
    Mtx EnvXForm;
    Mtx EnvXFormS;
    Mtx EnvXFormT;
    Mtx EnvXFormR;
    MTXRotDeg( EnvXFormR, 'Y', 95 );
    MTXScale ( EnvXFormS, 0.5f, -0.5f, 0.0f );
    MTXTrans ( EnvXFormT, 0.5f, 0.5f, 1.0f );
    MTXConcat( EnvXFormS, EnvXFormR, EnvXForm );
    MTXConcat( EnvXFormT, EnvXForm, EnvXForm );
    GXLoadTexMtxImm( EnvXForm, GX_PTTEXMTX0, GX_MTX3x4 );

    //--- setup GQR registers for fast loads/stores
    GCSKIN::SetupGQR();

    //--- now just loop through the sub-meshes, rendering them at will
    for( i = 0; i < GCSKIN::GetSubMeshCount(); ++i )
    {
        pSubMeshGroup = &pSubMeshArray[ pSubMeshOrder[i].Index ];

        //--- turn lighting off for shadows, on for everything else
        if( pSubMeshGroup->Flags & SKIN_SUBMESH_FLAG_SHADOW )
            GC_EnableLighting( FALSE, FALSE );
        else
            GC_EnableLighting( TRUE, FALSE );

        //--- check if a user-callback is present, and call it
        if( s_PreRenderCallback != NULL )
        {
            if( FALSE == s_PreRenderCallback( pSubMeshGroup->MeshID, (u32)pSubMeshGroup, pSubMeshGroup->UserData1, pSubMeshGroup->UserData2, pSubMeshGroup->UserData3 ) )
                continue;
        }
        else
        {
            //--- set the render flags based on the sub-mesh flags
            if( pSubMeshGroup->Flags & SKIN_SUBMESH_FLAG_SHADOW )
            {
                //--- save current ZTest and ZFill to avoid reset of Z state
                u32 RenderFlags;
                ENG_GetRenderFlags( RenderFlags );
                RenderFlags &= ~(ENG_ALPHA_TEST_ON | ENG_ALPHA_TEST_OFF | ENG_ALPHA_BLEND_ON | ENG_ALPHA_BLEND_OFF);

                ENG_SetRenderFlags( RenderFlags | ENG_ALPHA_TEST_OFF | ENG_ALPHA_BLEND_ON );
                ENG_SetBlendMode( ENG_BLEND_NORMAL | ENG_BLEND_FIXED_ALPHA, 32 );

                GC_SetZMode( TRUE, TRUE, FALSE );

                GC_EnableRefPlane( vector4(0,1,0,0) );
            }
            else if( pSubMeshGroup->Flags & SKIN_SUBMESH_FLAG_ALPHA )
            {
                ENG_SetRenderFlags( ENG_ZBUFFER_TEST_ON | ENG_ZBUFFER_FILL_OFF | ENG_ALPHA_TEST_OFF | ENG_ALPHA_BLEND_ON );
                ENG_SetBlendMode( ENG_BLEND_NORMAL );

                GC_DisableRefPlane();
            }
            else
            {
                ENG_SetRenderFlags( ENG_ZBUFFER_TEST_ON | ENG_ZBUFFER_FILL_ON | ENG_ALPHA_TEST_OFF | ENG_ALPHA_BLEND_OFF );

                GC_DisableRefPlane();
            }
        }

        //--- load up a new texture if needed
        if( (pSubMeshGroup->pBitmap != NULL) && (pSubMeshGroup->pBitmap != pCurBitmap) )
        {
            pCurBitmap = pSubMeshGroup->pBitmap;
            VRAM_GC_Activate( *pCurBitmap, 0 );

            //--- load environment map if needed
            if( pSubMeshGroup->Flags & SKIN_SUBMESH_FLAG_ENVPASS )
            {
                if( (pSubMeshGroup->pBitmap2 != NULL) && (pSubMeshGroup->pBitmap2 != pCurEnvBitmap) )
                {
                    pCurEnvBitmap = pSubMeshGroup->pBitmap2;
                    VRAM_GC_Activate( *pCurEnvBitmap, 1 );
                }
            }
        }

        //--- setup TEV for submesh
        QSkin::SetupTEV( pSubMeshGroup->Flags );

        RenderSubMeshGroup( pSubMeshGroup );
    }

    GC_DisableRefPlane( TRUE );

    //--- close skin buffer, free any allocated SMEM
    SMEM_StackPopToMarker();

    GCSKIN::CloseBuffer();

    GCSKIN::RestoreGQR();

    GCSKIN::RestoreL1Cache();

    //--- let the user know we are finished
    if( s_FinishedCallback )
    {
        s_FinishedCallback();
    }
}

//==========================================================================

void QSkin::RenderSubMesh( u32 SkinData )
{
    RenderSubMeshGroup( (GCSubMeshGroup*)SkinData );
}

//==========================================================================

void QSkin::RenderSubMeshGroup( GCSubMeshGroup* pGroup )
{
    //--- if there's vertex data with only 1 bone per vertex, send that data
    //    to the GP by setting up the attrib. arrays and calling the display list
    if( pGroup->pSubMesh->DListSizeGP != 0 )
    {
        GCSKIN::SetupVtxFmt( pGroup->Flags, FALSE );

        //--- make sure display list pointer and size are 32B aligned
        ASSERT( IS_ALIGN32( pGroup->pSubMesh->pDListGP ) );
        ASSERT( IS_ALIGN32( pGroup->pSubMesh->DListSizeGP ) );

        if( pGroup->Flags & SKIN_SUBMESH_FLAG_SHADOW )
        {
            GCSKIN::SetMtxArray( pGroup->pShadowMatrices );
            GCSKIN::SetVtxArray( pGroup->pSubMesh->pVtxShadow );
            GXCallDisplayList( pGroup->pSubMesh->pDListGP, pGroup->pSubMesh->DListSizeGP );
        }
        else
        {
            GCSKIN::SetMtxArray( pGroup->pMatrices );
            GCSKIN::SetVtxArray( pGroup->pSubMesh->pVtxGP );
            GCSKIN::SetVtxArray( pGroup->pSubMesh->pUVs );
            GXCallDisplayList( pGroup->pSubMesh->pDListGP, pGroup->pSubMesh->DListSizeGP );
        }
    }

    //--- then do CPU XForms
    if( pGroup->pSubMesh->DListSizeCPU != 0 )
    {
        s32             NCPUVerts;
        GCSkinVert_PN*  pFinalVtxData;

        //--- setup vertex format and data pointers
        GCSKIN::SetupVtxFmt( pGroup->Flags, TRUE );
        GCSKIN::SetVtxArray( pGroup->pSubMesh->pUVs );

        NCPUVerts = (s32)pGroup->pSubMesh->NVertsCPU1 + (s32)pGroup->pSubMesh->NVertsCPU2 + (s32)pGroup->pSubMesh->NVertsCPU3;

        //--- allocate destination buffer in SMEM, adding 64 bytes pad for DMA start address and size
        pFinalVtxData = (GCSkinVert_PN*)SMEM_BufferAlloc( NCPUVerts * sizeof(GCSkinVert_PN) + 64 );
        ASSERT( pFinalVtxData != NULL );
        pFinalVtxData = (GCSkinVert_PN*)ALIGN_32( pFinalVtxData );

        XFormVertsLockedCache( pGroup, pFinalVtxData, NCPUVerts );

//     NON-LOCKED CACHE METHOD
//        //--- XForm the verts
//        XFormVerts( &pGroup->pSubMesh->pVtxCPU[0],
//                    pFinalVtxData,
//                    pGroup->pMatrices->LAnim,
//                    pGroup->pSubMesh->NVertsCPU1,
//                    pGroup->pSubMesh->NVertsCPU2,
//                    pGroup->pSubMesh->NVertsCPU3 );
//    
//        //--- flush CPU cache before using in GP
//        DCFlushRange( pFinalVtxData, NCPUVerts * sizeof(GCSkinVert_PN) );

        //--- load the WT2V matrix(W2V * T(WPos)) for XForming the verts from local-2-view
        GXLoadPosMtxImm( pGroup->pMatrices->WT2V, GX_PNMTX0 );
        GXLoadNrmMtxImm( pGroup->pMatrices->WT2V, GX_PNMTX0 );
        GXSetCurrentMtx( GX_PNMTX0 );

        GCSKIN::SetVtxArray( pFinalVtxData );

        //--- make sure display list pointer and size are 32B aligned
        ASSERT( IS_ALIGN32( pGroup->pSubMesh->pDListCPU ) );
        ASSERT( IS_ALIGN32( pGroup->pSubMesh->DListSizeCPU ) );

        GXCallDisplayList( pGroup->pSubMesh->pDListCPU, pGroup->pSubMesh->DListSizeCPU );
    }
}

//==========================================================================

static void XFormVertsLockedCache( GCSubMeshGroup* pGroup, GCSkinVert_PN* pFinalVtxData, s32 NCPUVerts )
{
    s32 i;
    s32 NVertsToDMA;
    s32 NDMAVertsLeft  = NCPUVerts;
    s32 NBuffersFilled = 0;
    s32 CurWorkBuffer  = 0;
    s32 NextFillBuffer;
    s32 CurVertID      = 0;
    s32 CurDeltaID     = 0;

    GCSkinVert_PN*  pDstVtxMainRAM = pFinalVtxData;
    GCSkinVert_CPU* pSrcVtxMainRAM = pGroup->pSubMesh->pVtxCPU;

    GC_BufferVtxCount CurVtxTotal;
    GC_BufferVtxCount BufVtxCount[DMA_BUFFER_COUNT];

    //--- Setup initial vert counts
    CurVtxTotal.NCPU1 = (s32)pGroup->pSubMesh->NVertsCPU1;
    CurVtxTotal.NCPU2 = (s32)pGroup->pSubMesh->NVertsCPU2;
    CurVtxTotal.NCPU3 = (s32)pGroup->pSubMesh->NVertsCPU3;

    //--- touch(i.e. load) matrix data into normal cache
    for( i = 0; i < (ALIGN_32(sizeof(GCPosMtx) * SKIN_MAX_MATRICES)/32); ++i )
        DCTouchLoad( (byte*)pGroup->pMatrices + (i * 32) );

    //--- queue up DMA transfers of vertex data into locked cache buffers
    for( i = 0; i < DMA_BUFFER_COUNT; ++i )
    {
        if( NDMAVertsLeft >= NUM_VTX_PER_BUFFER )
        {
            NVertsToDMA = NUM_VTX_PER_BUFFER;
        }
        else
        {
            NVertsToDMA = NDMAVertsLeft;
        }

        //--- DMA the verts into buffer, update counts and pointers
        LCLoadBlocks( GCSKIN::GetDMASrcBuf( i ), pSrcVtxMainRAM, (ALIGN_32(NVertsToDMA * sizeof(GCSkinVert_CPU)) / 32) );
        NBuffersFilled++;
        NDMAVertsLeft  -= NVertsToDMA;
        pSrcVtxMainRAM += NVertsToDMA;

        //--- figure how many of 1,2, and 3 bone/vtx verts were DMA'd
        CalcVertCounts( BufVtxCount[i], CurVtxTotal, NVertsToDMA );

        //--- check if all the verts have been DMA'd, break out of loop
        if( NDMAVertsLeft <= 0 )
            break;
    }

    //--- wait until at least 1 of the buffers has finished the DMA
    LCQueueWait( NBuffersFilled - 1 );

    while( TRUE )
    {
        register s32 NVerts = (BufVtxCount[CurWorkBuffer].NCPU1 + BufVtxCount[CurWorkBuffer].NCPU2 + BufVtxCount[CurWorkBuffer].NCPU3);

        if( pGroup->Flags & SKIN_SUBMESH_FLAG_MORPH )
        {
            ApplyMorphDeltas( GCSKIN::GetDMASrcBuf( CurWorkBuffer ),
                              CurVertID,
                              NVerts,
                              CurDeltaID,
                              pGroup );
        }

        CurVertID += NVerts;

        //--- operate on the current buffer
        XFormVerts( GCSKIN::GetDMASrcBuf( CurWorkBuffer ),
                    GCSKIN::GetDMADstBuf( CurWorkBuffer ),
                    pGroup->pMatrices->LAnim,
                    BufVtxCount[CurWorkBuffer].NCPU1,
                    BufVtxCount[CurWorkBuffer].NCPU2,
                    BufVtxCount[CurWorkBuffer].NCPU3 );

        //--- DMA the dest verts back to main RAM
        NVertsToDMA = BufVtxCount[CurWorkBuffer].NCPU1 + BufVtxCount[CurWorkBuffer].NCPU2 + BufVtxCount[CurWorkBuffer].NCPU3;
        LCStoreBlocks( pDstVtxMainRAM, GCSKIN::GetDMADstBuf( CurWorkBuffer ), (ALIGN_32(NVertsToDMA * sizeof(GCSkinVert_PN)) / 32) );

        pDstVtxMainRAM += NVertsToDMA;
        NBuffersFilled--;
        CurWorkBuffer++;

        if( CurWorkBuffer >= DMA_BUFFER_COUNT )
            CurWorkBuffer = 0;

        //--- one buffer is now free, if there's verts that haven't been DMA'd then put them in unused buffer
        if( NDMAVertsLeft > 0 )
        {
            NextFillBuffer = CurWorkBuffer - 1;
            if( NextFillBuffer < 0 )
                NextFillBuffer = DMA_BUFFER_COUNT - 1;

            if( NDMAVertsLeft >= NUM_VTX_PER_BUFFER )
            {
                NVertsToDMA = NUM_VTX_PER_BUFFER;
            }
            else
            {
                NVertsToDMA = NDMAVertsLeft;
            }

            //--- DMA the verts into buffer, update counts and pointers
            LCLoadBlocks( GCSKIN::GetDMASrcBuf( NextFillBuffer ), pSrcVtxMainRAM, (ALIGN_32(NVertsToDMA * sizeof(GCSkinVert_CPU)) / 32) );
            NBuffersFilled += 1;
            NDMAVertsLeft  -= NVertsToDMA;
            pSrcVtxMainRAM += NVertsToDMA;

            //--- figure how many of 1,2, and 3 bone/vtx verts were DMA'd
            CalcVertCounts( BufVtxCount[NextFillBuffer], CurVtxTotal, NVertsToDMA );
        }

        //--- if all buffers are empty, then all work is completed
        if( NBuffersFilled <= 0 )
            break;
    }

    //--- wait for all DMA transfers to main RAM to finish
    LCQueueWait( 0 );
}

//==========================================================================

static void CalcVertCounts( GC_BufferVtxCount& rBufVtxCount, GC_BufferVtxCount& rCurVtxTotal, s32 VertCount )
{
    //--- check 1 bone/vtx count
    if( (rCurVtxTotal.NCPU1 > 0) && (rCurVtxTotal.NCPU1 >= VertCount) )
    {
        rBufVtxCount.NCPU1  = VertCount;
        rCurVtxTotal.NCPU1 -= VertCount;
        VertCount = 0;
    }
    else
    {
        VertCount -= rCurVtxTotal.NCPU1;
        rBufVtxCount.NCPU1 = rCurVtxTotal.NCPU1;
        rCurVtxTotal.NCPU1 = 0;
    }

    //--- if all verts were used up, none in 2 and 3 vert list
    if( VertCount <= 0 )
    {
        rBufVtxCount.NCPU2 = 0;
        rBufVtxCount.NCPU3 = 0;
        return;
    }

    //--- check 2 bones/vtx count
    if( (rCurVtxTotal.NCPU2 > 0) && (rCurVtxTotal.NCPU2 >= VertCount) )
    {
        rBufVtxCount.NCPU2  = VertCount;
        rCurVtxTotal.NCPU2 -= VertCount;
        VertCount = 0;
    }
    else
    {
        VertCount -= rCurVtxTotal.NCPU2;
        rBufVtxCount.NCPU2 = rCurVtxTotal.NCPU2;
        rCurVtxTotal.NCPU2 = 0;
    }

    //--- if all verts have been used by 1 and 2 lists, nothing in 3
    if( VertCount <= 0 )
    {
        rBufVtxCount.NCPU3 = 0;
        return;
    }

    //--- check 3 bones/vtx list
    if( (rCurVtxTotal.NCPU3 > 0) && (rCurVtxTotal.NCPU3 >= VertCount) )
    {
        rBufVtxCount.NCPU3  = VertCount;
        rCurVtxTotal.NCPU3 -= VertCount;
        VertCount = 0;
    }
    else
    {
        VertCount -= rCurVtxTotal.NCPU3;
        rBufVtxCount.NCPU3 = rCurVtxTotal.NCPU3;
        rCurVtxTotal.NCPU3 = 0;
    }
}

//==========================================================================

static void ApplyMorphDeltas( GCSkinVert_CPU* pVertData,
                              s32             CurVertID,
                              s32             NVerts,
                              s32&            rCurDeltaID,
                              GCSubMeshGroup* pGroup )
{
    ASSERT( pVertData != NULL );
    ASSERT( pGroup != NULL );

    s32             NMorphTgts = pGroup->pMesh->NMorphTargets;
    s32             NDeltas    = pGroup->pSubMesh->NDeltasCPU;
    GCSkinDelta*    pDeltas    = pGroup->pSubMesh->pDeltasCPU;
    GCSkinMorphTgt* pMorphTgt  = pGroup->pMorphTgts;
    s32             MaxVertID  = CurVertID + NVerts - 1;

    if( (pMorphTgt[0].Active == TRUE) && (pMorphTgt[0].Weight == 1.0f) )
    {
        //--- only the default morph is active, do nothing
        return;
    }

    if( (s32)(pDeltas[rCurDeltaID].VertIdx) > MaxVertID )
    {
        //--- current deltas don't apply to any verts in this list
        return;
    }

    //--- sanity check, if this fails then delta data is not sync'd up to vert list
    ASSERT( CurVertID <= (s32)(pDeltas[rCurDeltaID].VertIdx) );

    ASSERT( NMorphTgts > 0 );
    ASSERT( (rCurDeltaID % NMorphTgts) == 0 );

    while( rCurDeltaID < NDeltas )
    {
        if( (s32)(pDeltas[rCurDeltaID].VertIdx) > MaxVertID )
            break;

        s32             i;
        vector3         vTemp;
        GCSkinVert_CPU* pCurVert;

        pCurVert = &(pVertData[ pDeltas[rCurDeltaID].VertIdx - CurVertID ]);

        vTemp.X = (f32)pCurVert->PX * (1.0f/(1<<GC_FRAC_POS));
        vTemp.Y = (f32)pCurVert->PY * (1.0f/(1<<GC_FRAC_POS));
        vTemp.Z = (f32)pCurVert->PZ * (1.0f/(1<<GC_FRAC_POS));

        for( i = 0; i < NMorphTgts; i++ )
        {
            vTemp.X += (f32)(pDeltas[rCurDeltaID].XDelta) * (1.0f/(1<<GC_FRAC_DELTA)) * pMorphTgt[i].Weight;
            vTemp.Y += (f32)(pDeltas[rCurDeltaID].YDelta) * (1.0f/(1<<GC_FRAC_DELTA)) * pMorphTgt[i].Weight;
            vTemp.Z += (f32)(pDeltas[rCurDeltaID].ZDelta) * (1.0f/(1<<GC_FRAC_DELTA)) * pMorphTgt[i].Weight;
            rCurDeltaID++;
        }

        pCurVert->PX = (s16)(vTemp.X * (1<<GC_FRAC_POS));
        pCurVert->PY = (s16)(vTemp.Y * (1<<GC_FRAC_POS));
        pCurVert->PZ = (s16)(vTemp.Z * (1<<GC_FRAC_POS));
    }

/*
    asm __volatile__
    ("
        .equr   pVtx, %%r31             #
        .equr    pWt, %%r30             #
        .equr      i, %%r0              #

        b       WHILE_LOOP_CHECK        # do loop test now

    WHILE_LOOP_START:
        lhz        i, 6(%2)             # i = pDeltas[rCurDeltaID].VertIdx
        cmpw     cr1,    i, %4          # compare pDeltas[rCurDeltaID].VertIdx with MaxVertID
        bgt      cr1, WHILE_LOOP_END    # break out of loop if VertIdx > MaxVertID

        subf       i, %7,  i            # to get propper index into pVertData, subtract CurVertID from delta's index
        mulli      i,  i, 16            # compute offset into vert data, VertIdx * sizeof(GCSkinVert_CPU)
        add     pVtx, %1,  i            # add offset to vert data ptr to get address of vert to apply delta to

        psq_l     f0, 0(pVtx), 0, 5     # f0 = pVtx->PX, pVtx->PY  (vTemp.X, vTemp.Y)
        psq_l     f1, 4(pVtx), 1, 5     # f1 = pVtx->PZ, 1.0       (vTemp.Z)

        li         i,  0                # i = 0
        mr       pWt, %6                # initialize pWt

        b       FOR_LOOP_CHECK          # do loop test now

    FOR_LOOP_START:
        psq_l   f4, 0(pWt), 1, 0        # f4 = pMorphTgt[i].Weight
        psq_l   f2,  0(%2), 0, 4        # f2 = pCurDelta->XDelta, pCurDelta->YDelta
        psq_l   f3,  4(%2), 1, 4        # f3 = pCurDelta->ZDelta, 1.0
        addi    %2,     %2, 8           # pCurDelta++  (sizeof(GCSkinDelta) == 8)
        addi    %0,     %0, 1           # rCurDeltaID++

        ps_madds0   f0, f2, f4, f0      # vTemp += CurDelta * Weight
        ps_madds0   f1, f3, f4, f1      #

    FOR_LOOP_CHECK:
        cmpw    cr1, i, %5              # compare i with NMorphTgts
        blt     cr1, FOR_LOOP_START     #

    FOR_LOOP_END:

        psq_st  f0, 0(pVtx), 0, 5       # store final result, weighted deltas added to
        psq_st  f1, 4(pVtx), 1, 5       # the original vertex

    WHILE_LOOP_CHECK:
        cmpw    cr0, %0, %3             # compare rCurDeltaID with NDeltas
        blt     cr0, WHILE_LOOP_START   #

    WHILE_LOOP_END:
    "
    : "=r" (rCurDeltaID)
    : "r" (pVertData), "b" (&pDeltas[rCurDeltaID]), "r" (NDeltas),
      "r" (MaxVertID), "r" (NMorphTgts), "b" (&pMorphTgt[0].Weight), "r" (CurVertID)
    : "r0", "r30", "r31", "fr0", "fr1", "fr2", "fr3", "fr4"
    );
*/
}

//==========================================================================

static void XFormVerts( register GCSkinVert_CPU* pVtxDataSrc,
                        register GCSkinVert_PN*  pVtxDataDst,
                        register GCPosMtx*       pMtxData,
                        register s32             NVertsCPU1,
                        register s32             NVertsCPU2,
                        register s32             NVertsCPU3 )
{
    f32 FZERO = 0.0f;

    asm __volatile__
    ("
        .equr    pMtx0, %%r26               # name r26 as pMtx0, pointer to mtx0
        .equr    pMtx1, %%r27               # name r27 as pMtx1, pointer to mtx1
        .equr    pMtx2, %%r28               # name r28 as pMtx2, pointer to mtx2
        .equr   CMIdx0, %%r29               # name r29 as CMIdx0, index to current mtx0
        .equr   CMIdx1, %%r30               # name r30 as CMIdx1, index to current mtx1
        .equr   CMIdx2, %%r31               # name r31 as CMIdx2, index to current mtx2

    INIT_VARIABLES:
        cmpwi      cr5,     %3, 0           # compare NCPU1 with 0 (done early for branch prediction)
        cmpwi      cr6,     %4, 0           # compare NCPU2 with 0 (done early for branch prediction)
        cmpwi      cr7,     %5, 0           # compare NCPU3 with 0 (done early for branch prediction)
        li       pMtx0, 0x0000              # pMtx0 = 0
        li       pMtx1, 0x0000              # pMtx1 = 0
        li       pMtx2, 0x0000              # pMtx2 = 0
        psq_l       f0,  0(%6), 1, 0        # f0 = (0.0, 1.0)
        lis     CMIdx0, 0xFFFF              # CMIdx0 = 0xFFFF0000
        lis     CMIdx1, 0xFFFF              # CMIdx1 = 0xFFFF0000
        lis     CMIdx2, 0xFFFF              # CMIdx2 = 0xFFFF0000


    CPU1_LOOP_SETUP:
        ble     cr5, CPU1_END               # (NCPU1 <= 0) ? skip loop : continue
        mtspr   ctr, %3                     # move NCPU1 to count register

    CPU1_START:

    CPU1_GET_MTX_INDICES:
        lhz        r0, 14(%0)               # r0    = pVSrc->MtxIdx
        andi.   pMtx0,     r0, 0x001F       # pMtx0 = pVSrc->MtxIdx & 0x001F
        cmpw      cr0, CMIdx0, pMtx0        # compare CMIdx0 with pMtx0

        beq       cr0, CPU1_END_LOAD_MTX_0  # (CMIdx0 == pMtx0) ? skip load of Mtx0 : load Mtx0

    CPU1_LOAD_MTX_0:
        mr      CMIdx0, pMtx0               # CMIdx0 = pMtx0 (save current matrix index)
        mulli    pMtx0, pMtx0, 48           # pMtx0 *= 48 (calc offset into mtx array, index * sizeof(GCPosMtx))
        add      pMtx0, pMtx0, %2           # pMtx0 += pMtxData (pMtx0 = &pMtxData[ pVtxDataSrc->MtxIdx & 0x001F ])

        psq_l   f14,  0(pMtx0), 0, 0        # f14 = (M0[0][0], M0[0][1])
        psq_l   f15,  8(pMtx0), 0, 0        # f15 = (M0[0][2], M0[0][3])
        psq_l   f16, 16(pMtx0), 0, 0        # f16 = (M0[1][0], M0[1][1])
        psq_l   f17, 24(pMtx0), 0, 0        # f17 = (M0[1][2], M0[1][3])
        psq_l   f18, 32(pMtx0), 0, 0        # f18 = (M0[2][0], M0[2][1])
        psq_l   f19, 40(pMtx0), 0, 0        # f19 = (M0[2][2], M0[2][3])
    CPU1_END_LOAD_MTX_0:


    CPU1_XFORM_VTX:
        psq_l        f5, 4(%0),   0,   6    # f5   = (---, N.Z)
        psq_l        f4, 8(%0),   0,   6    # f4   = (N.X, N.Y)
        psq_l        f2, 0(%0),   0,   5    # f2   = (V.X, V.Y)
        psq_l        f3, 4(%0),   1,   5    # f3   = (V.Z, 1.0)
        ps_merge10   f5,    f5, f0          # f5   = (N.Z, 0.0)
        ps_mul      f10,    f4, f14         # f10  = (M0[0][0], M0[0][1]) * (N.X, N.Y)
        ps_mul       f6,    f2, f14         # f6   = (M0[0][0], M0[0][1]) * (V.X, V.Y)
        ps_mul      f11,    f4, f16         # f11  = (M0[1][0], M0[1][1]) * (N.X, N.Y)
        ps_mul       f7,    f2, f16         # f7   = (M0[1][0], M0[1][1]) * (V.X, V.Y)
        ps_mul      f12,    f4, f18         # f12  = (M0[2][0], M0[2][1]) * (N.X, N.Y)
        ps_mul       f8,    f2, f18         # f8   = (M0[2][0], M0[2][1]) * (V.X, V.Y)
        ps_madd     f10,    f5, f15, f10    # f10 += (M0[0][2], M0[0][3]) * (N.Z, 0.0)
        ps_madd      f6,    f3, f15,  f6    # f6  += (M0[0][2], M0[0][3]) * (V.Z, 1.0)
        ps_madd     f11,    f5, f17, f11    # f11 += (M0[1][2], M0[1][3]) * (N.Z, 0.0)
        ps_madd      f7,    f3, f17,  f7    # f7  += (M0[1][2], M0[1][3]) * (V.Z, 1.0)
        ps_madd     f12,    f5, f19, f12    # f12 += (M0[2][2], M0[2][3]) * (N.Z, 0.0)
        ps_madd      f8,    f3, f19,  f8    # f8  += (M0[2][2], M0[2][3]) * (V.Z, 1.0)
        ps_sum0      f2,    f6,  f6,  f6    # f2   = ( f6[0] +  f6[1],   f6[1])
        ps_sum1      f4,   f10, f10, f10    # f4   = (f10[0],  f10[0] + f10[1])
        ps_sum0      f5,   f11, f12, f11    # f5   = (f11[0] + f11[1],  f12[1])
        ps_sum1      f2,    f7,  f2,  f7    # f2   = ( f2[0],   f7[0] +  f7[1])
        ps_sum0      f3,    f8,  f8,  f8    # f3   = ( f8[0] +  f8[1],   f8[1])
        psq_st       f4, 4(%1),   0,   6    # store (---, N.X)
        ps_sum1      f5,   f12,  f5,  f5    # f5   = ( f5[0],  f12[0] +  f5[1])
        psq_st       f2, 0(%1),   0,   5    # store (V.X, V.Y)
        psq_st       f3, 4(%1),   1,   5    # store (V.Z, ---)
        addi         %0,    %0,  16         # pVtxDataSrc++  (sizeof(GCSkinVert_CPU) == 16)
        psq_st       f5, 8(%1),   0,   6    # store (N.Y, N.Z)
        addi         %1,    %1,  12         # pVtxDataDst++  (sizeof(GCSkinVert_PN)  == 12)

        bdnz        CPU1_START              # decrement CTR, if > 0, goto start of loop
    CPU1_END:



    CPU2_LOOP_SETUP:
        ble     cr6, CPU2_END               # (NCPU2 <= 0) ? skip loop : continue
        mtspr   ctr, %4                     # move NCPU2 to count register

    CPU2_START:

    CPU2_GET_MTX_INDICES:
        lhz        r0, 14(%0)               # r0    = pVSrc->MtxIdx
        srwi    pMtx1,     r0, 5            # pMtx1 = MtxIdx >> 5
        andi.   pMtx0,     r0, 0x001F       # pMtx0 = MtxIdx & 0x001F
        andi.   pMtx1,  pMtx1, 0x001F       # pMtx1 = (MtxIdx >> 5) & 0x001F
        cmpw      cr0, CMIdx0, pMtx0        # compare CMIdx0 with pMtx0
        cmpw      cr1, CMIdx1, pMtx1        # compare CMIdx1 with pMtx1

        beq       cr0, CPU2_END_LOAD_MTX_0  # (CMIdx0 == pMtx0) ? skip load of Mtx0 : load Mtx0

    CPU2_LOAD_MTX_0:
        mr      CMIdx0, pMtx0               # CMIdx0 = pMtx0 (save current matrix index)
        mulli    pMtx0, pMtx0, 48           # pMtx0 *= 48 (calc offset into mtx array, index * sizeof(GCPosMtx))
        add      pMtx0, pMtx0, %2           # pMtx0 += pMtxData (pMtx0 = &pMtxData[ pVtxDataSrc->MtxIdx & 0x001F ])

        psq_l   f14,  0(pMtx0), 0, 0        # f14 = (M0[0][0], M0[0][1])
        psq_l   f15,  8(pMtx0), 0, 0        # f15 = (M0[0][2], M0[0][3])
        psq_l   f16, 16(pMtx0), 0, 0        # f16 = (M0[1][0], M0[1][1])
        psq_l   f17, 24(pMtx0), 0, 0        # f17 = (M0[1][2], M0[1][3])
        psq_l   f18, 32(pMtx0), 0, 0        # f18 = (M0[2][0], M0[2][1])
        psq_l   f19, 40(pMtx0), 0, 0        # f19 = (M0[2][2], M0[2][3])
    CPU2_END_LOAD_MTX_0:

        beq     cr1, CPU2_END_LOAD_MTX_1    # (CMIdx1 == pMtx1) ? skip load of Mtx1 : load Mtx1

    CPU2_LOAD_MTX_1:
        mr      CMIdx1, pMtx1               # CMIdx1 = pMtx1 (save current matrix index)
        mulli    pMtx1, pMtx1, 48           # pMtx1 *= 48 (calc offset into mtx array, index * sizeof(GCPosMtx))
        add      pMtx1, pMtx1, %2           # pMtx1 += pMtxData (pMtx1 = &pMtxData[ (pVtxDataSrc->MtxIdx >> 5) & 0x001F ])

        psq_l   f20,  0(pMtx1), 0, 0        # f20 = (M1[0][0], M1[0][1])
        psq_l   f21,  8(pMtx1), 0, 0        # f21 = (M1[0][2], M1[0][3])
        psq_l   f22, 16(pMtx1), 0, 0        # f22 = (M1[1][0], M1[1][1])
        psq_l   f23, 24(pMtx1), 0, 0        # f23 = (M1[1][2], M1[1][3])
        psq_l   f24, 32(pMtx1), 0, 0        # f24 = (M1[2][0], M1[2][1])
        psq_l   f25, 40(pMtx1), 0, 0        # f25 = (M1[2][2], M1[2][3])
    CPU2_END_LOAD_MTX_1:


    CPU2_XFORM_VTX:
        psq_l        f5, 4(%0),   0,   6    # f5   = (---, N.Z)
        psq_l        f4, 8(%0),   0,   6    # f4   = (N.X, N.Y)
        psq_l        f1,12(%0),   0,   7    # f1   = (W0, W1)
        psq_l        f2, 0(%0),   0,   5    # f2   = (V.X, V.Y)
        ps_merge10   f5,    f5,  f0         # f5   = (N.Z, 0.0)
        psq_l        f3, 4(%0),   1,   5    # f3   = (V.Z, 1.0)
        ps_muls1     f6,    f4,  f1         # f6   = (N.X, N.Y) * W1
        ps_muls0     f4,    f4,  f1         # f4   = (N.X, N.Y) * W0
        ps_muls1     f7,    f5,  f1         # f7   = (N.Z, 0.0) * W1
        ps_muls0     f5,    f5,  f1         # f5   = (N.Z, 0.0) * W0
        ps_mul      f11,    f6, f20         # f11  = (M1[0][0], M1[0][1]) * (N1.X, N1.Y)
        ps_mul       f8,    f4, f14         # f8   = (M0[0][0], M0[0][1]) * (N0.X, N0.Y)
        ps_mul      f12,    f6, f22         # f12  = (M1[1][0], M1[1][1]) * (N1.X, N1.Y)
        ps_mul       f9,    f4, f16         # f9   = (M0[1][0], M0[1][1]) * (N0.X, N0.Y)
        ps_mul      f13,    f6, f24         # f13  = (M1[2][0], M1[2][1]) * (N1.X, N1.Y)
        ps_mul      f10,    f4, f18         # f10  = (M0[2][0], M0[2][1]) * (N0.X, N0.Y)
        ps_madd      f8,    f5, f15,  f8    # f8  += (M0[0][2], M0[0][3]) * (N0.Z, 0.0)
        ps_madd     f11,    f7, f21, f11    # f11 += (M1[0][2], M1[0][3]) * (N1.Z, 0.0)
        ps_madd      f9,    f5, f17,  f9    # f9  += (M0[1][2], M0[1][3]) * (N0.Z, 0.0)
        ps_madd     f12,    f7, f23, f12    # f12 += (M1[1][2], M1[1][3]) * (N1.Z, 0.0)
        ps_madd     f10,    f5, f19, f10    # f10 += (M0[2][2], M0[2][3]) * (N0.Z, 0.0)
        ps_madd     f13,    f7, f25, f13    # f13 += (M1[2][2], M1[2][3]) * (N1.Z, 0.0)
        ps_add       f4,    f8, f11         # f4   = NX0 + NX1
        ps_add       f6,    f9, f12         # f6   = NY0 + NY1
        ps_add       f5,   f10, f13         # f5   = NZ0 + NZ1
        ps_sum0      f7,    f6,  f5,  f6    # f7   = (f6[0] + f6[1],  f5[1])
        ps_sum1      f8,    f4,  f4,  f4    # f8   = (f4[0],  f4[0] + f4[1])
        ps_sum1      f7,    f5,  f7,  f7    # f7   = (f7[0],  f5[0] + f7[1])
        ps_muls1     f6,    f2,  f1         # f6   = (V.X, V.Y) * W1
        ps_muls0     f4,    f2,  f1         # f4   = (V.X, V.Y) * W0
        psq_st       f8, 4(%1),   0,   6    # store (---, N.X)
        ps_muls0     f5,    f3,  f1         # f5   = (V.Z, 1.0) * W0
        psq_st       f7, 8(%1),   0,   6    # store (N.Y, N.Z)
        ps_mul      f11,    f6, f20         # f11  = (M1[0][0], M1[0][1]) * (V1.X, V1.Y)
        ps_mul       f8,    f4, f14         # f8   = (M0[0][0], M0[0][1]) * (V0.X, V0.Y)
        ps_mul      f12,    f6, f22         # f12  = (M1[1][0], M1[1][1]) * (V1.X, V1.Y)
        ps_muls1     f7,    f3,  f1         # f7   = (V.Z, 1.0) * W1
        ps_mul       f9,    f4, f16         # f9   = (M0[1][0], M0[1][1]) * (V0.X, V0.Y)
        ps_mul      f13,    f6, f24         # f13  = (M1[2][0], M1[2][1]) * (V1.X, V1.Y)
        ps_mul      f10,    f4, f18         # f10  = (M0[2][0], M0[2][1]) * (V0.X, V0.Y)
        ps_madd      f8,    f5, f15,  f8    # f8  += (M0[0][2], M0[0][3]) * (V0.Z, W0)
        ps_madd     f11,    f7, f21, f11    # f11 += (M1[0][2], M1[0][3]) * (V1.Z, W1)
        ps_madd      f9,    f5, f17,  f9    # f9  += (M0[1][2], M0[1][3]) * (V0.Z, W0)
        ps_madd     f12,    f7, f23, f12    # f12 += (M1[1][2], M1[1][3]) * (V1.Z, W1)
        ps_madd     f10,    f5, f19, f10    # f10 += (M0[2][2], M0[2][3]) * (V0.Z, W0)
        ps_madd     f13,    f7, f25, f13    # f13 += (M1[2][2], M1[2][3]) * (V1.Z, W1)
        ps_add       f4,    f8, f11         # f4   = VX0 + VX1
        ps_add       f6,    f9, f12         # f6   = VY0 + VY1
        ps_add       f5,   f10, f13         # f5   = VZ0 + VZ1
        ps_sum0      f2,    f4,  f6,  f4    # f2   = (f4[0] + f4[1],  f6[1])
        ps_sum0      f3,    f5,  f5,  f5    # f3   = (f8[0] + f8[1],  1.0)
        ps_sum1      f2,    f6,  f2,  f2    # f2   = (f2[0],  f6[0] + f2[1])
        addi         %0,    %0,  16         # pVtxDataSrc++  (sizeof(GCSkinVert_CPU) == 16)
        psq_st       f3, 4(%1),   1,   5    # store (V.Z, ---)
        psq_st       f2, 0(%1),   0,   5    # store (V.X, V.Y)
        addi         %1,    %1,  12         # pVtxDataDst++  (sizeof(GCSkinVert_PN)  == 12)

        bdnz        CPU2_START              # decrement CTR, if > 0, goto start of loop
    CPU2_END:



    CPU3_LOOP_SETUP:
        ble     cr7, CPU3_END               # (NCPU3 <= 0) ? skip loop : continue
        mtspr   ctr, %5                     # move NCPU3 to count register

    CPU3_START:

    CPU3_GET_MTX_INDICES:
        lhz        r0, 14(%0)               # r0    = pVSrc->MtxIdx
        srwi    pMtx1,     r0, 5            # pMtx1 = MtxIdx >> 5
        srwi    pMtx2,     r0, 10           # pMtx2 = MtxIdx >> 10
        andi.   pMtx0,     r0, 0x001F       # pMtx0 = MtxIdx & 0x001F
        andi.   pMtx1,  pMtx1, 0x001F       # pMtx1 = (MtxIdx >> 5) & 0x001F
        andi.   pMtx2,  pMtx2, 0x001F       # pMtx2 = (MtxIdx >> 10) & 0x001F
        cmpw      cr0, CMIdx0, pMtx0        # compare CMIdx0 with pMtx0
        cmpw      cr1, CMIdx1, pMtx1        # compare CMIdx1 with pMtx1
        cmpw      cr2, CMIdx2, pMtx2        # compare CMIdx2 with pMtx2

        beq     cr0, CPU3_END_LOAD_MTX_0    # (CMIdx0 == pMtx0) ? skip load of Mtx0 : load Mtx0

    CPU3_LOAD_MTX_0:
        mr      CMIdx0, pMtx0               # CMIdx0 = pMtx0 (save current matrix index)
        mulli    pMtx0, pMtx0, 48           # pMtx0 *= 48 (calc offset into mtx array, index * sizeof(GCPosMtx))
        add      pMtx0, pMtx0, %2           # pMtx0 += pMtxData (pMtx0 = &pMtxData[ pVtxDataSrc->MtxIdx & 0x001F ])

        psq_l   f14,  0(pMtx0), 0, 0        # f14 = (M0[0][0], M0[0][1])
        psq_l   f15,  8(pMtx0), 0, 0        # f15 = (M0[0][2], M0[0][3])
        psq_l   f16, 16(pMtx0), 0, 0        # f16 = (M0[1][0], M0[1][1])
        psq_l   f17, 24(pMtx0), 0, 0        # f17 = (M0[1][2], M0[1][3])
        psq_l   f18, 32(pMtx0), 0, 0        # f18 = (M0[2][0], M0[2][1])
        psq_l   f19, 40(pMtx0), 0, 0        # f19 = (M0[2][2], M0[2][3])
    CPU3_END_LOAD_MTX_0:

        beq     cr1, CPU3_END_LOAD_MTX_1    # (CMIdx1 == pMtx1) ? skip load of Mtx1 : load Mtx1

    CPU3_LOAD_MTX_1:
        mr      CMIdx1, pMtx1               # CMIdx1 = pMtx1 (save current matrix index)
        mulli    pMtx1, pMtx1, 48           # pMtx1 *= 48 (calc offset into mtx array, index * sizeof(GCPosMtx))
        add      pMtx1, pMtx1, %2           # pMtx1 += pMtxData (pMtx1 = &pMtxData[ (pVtxDataSrc->MtxIdx >> 5) & 0x001F ])

        psq_l   f20,  0(pMtx1), 0, 0        # f20 = (M1[0][0], M1[0][1])
        psq_l   f21,  8(pMtx1), 0, 0        # f21 = (M1[0][2], M1[0][3])
        psq_l   f22, 16(pMtx1), 0, 0        # f22 = (M1[1][0], M1[1][1])
        psq_l   f23, 24(pMtx1), 0, 0        # f23 = (M1[1][2], M1[1][3])
        psq_l   f24, 32(pMtx1), 0, 0        # f24 = (M1[2][0], M1[2][1])
        psq_l   f25, 40(pMtx1), 0, 0        # f25 = (M1[2][2], M1[2][3])
    CPU3_END_LOAD_MTX_1:

        beq     cr2, CPU3_END_LOAD_MTX_2    # (CMIdx2 == pMtx2) ? skip load of Mtx2 : load Mtx2

    CPU3_LOAD_MTX_2:
        mr      CMIdx2, pMtx2               # CMIdx2 = pMtx2 (save current matrix index)
        mulli    pMtx2, pMtx2, 48           # pMtx2 *= 48 (calc offset into mtx array, index * sizeof(GCPosMtx))
        add      pMtx2, pMtx2, %2           # pMtx2 += pMtxData (pMtx1 = &pMtxData[ (pVtxDataSrc->MtxIdx >> 10) & 0x001F ])

        psq_l   f26,  0(pMtx2), 0, 0        # f26 = (M2[0][0], M2[0][1])
        psq_l   f27,  8(pMtx2), 0, 0        # f27 = (M2[0][2], M2[0][3])
        psq_l   f28, 16(pMtx2), 0, 0        # f28 = (M2[1][0], M2[1][1])
        psq_l   f29, 24(pMtx2), 0, 0        # f29 = (M2[1][2], M2[1][3])
        psq_l   f30, 32(pMtx2), 0, 0        # f30 = (M2[2][0], M2[2][1])
        psq_l   f31, 40(pMtx2), 0, 0        # f31 = (M2[2][2], M2[2][3])
    CPU3_END_LOAD_MTX_2:


    CPU3_XFORM_VTX:
        psq_l        f1,12(%0),   0,   7    # f1   = (W0, W1)
        psq_l        f3, 4(%0),   0,   6    # f3   = (---, N.Z)
        psq_l        f2, 8(%0),   0,   6    # f2   = (N.X, N.Y)
        ps_neg      f10,    f1              # f10  = (-W0, -W1)
        ps_merge10   f3,    f3,  f0         # f3   = (N.Z, 0.0)
        ps_muls1     f4,    f2,  f1         # f4   = (N.X, N.Y) * W1
        ps_sum0     f10,   f10,  f0, f10    # f10  = (-W0-W1, 1.0)
        ps_mul      f11,    f4, f20         # f11  = (M1[0][0], M1[0][1]) * (N1.X, N1.Y)
        ps_mul      f12,    f4, f22         # f12  = (M1[1][0], M1[1][1]) * (N1.X, N1.Y)
        ps_sum0     f10,   f10, f10, f10    # f10  = (1.0-W0-W1, 1.0)
        ps_muls1     f5,    f3,  f1         # f5   = (N.Z, 0.0) * W1
        ps_muls0     f6,    f2, f10         # f6   = (N.X, N.Y) * W2
        ps_muls0     f2,    f2,  f1         # f2   = (N.X, N.Y) * W0
        ps_muls0     f7,    f3, f10         # f7   = (N.Z, 0.0) * W2
        ps_mul      f13,    f4, f24         # f13  = (M1[2][0], M1[2][1]) * (N1.X, N1.Y)
        ps_mul       f8,    f2, f14         # f8   = (M0[0][0], M0[0][1]) * (N0.X, N0.Y)
        ps_madd     f11,    f5, f21, f11    # f11 += (M1[0][2], M1[0][3]) * (N1.Z, 0.0)
        ps_muls0     f3,    f3,  f1         # f3   = (N.Z, 0.0) * W0
        ps_mul      f10,    f2, f18         # f10  = (M0[2][0], M0[2][1]) * (N0.X, N0.Y)
        ps_mul       f9,    f2, f16         # f9   = (M0[1][0], M0[1][1]) * (N0.X, N0.Y)
        ps_madd     f12,    f5, f23, f12    # f12 += (M1[1][2], M1[1][3]) * (N1.Z, 0.0)
        ps_madd      f8,    f3, f15,  f8    # f8  += (M0[0][2], M0[0][3]) * (N0.Z, 0.0)
        ps_madd      f9,    f3, f17,  f9    # f9  += (M0[1][2], M0[1][3]) * (N0.Z, 0.0)
        ps_madd     f10,    f3, f19, f10    # f10 += (M0[2][2], M0[2][3]) * (N0.Z, 0.0)
        ps_madd     f13,    f5, f25, f13    # f13 += (M1[2][2], M1[2][3]) * (N1.Z, 0.0)
        ps_madd      f8,    f6, f26,  f8    # f8  += (M2[0][0], M2[0][1]) * (N2.X, N2.Y)
        ps_madd     f11,    f7, f27, f11    # f11 += (M2[0][2], M2[0][3]) * (N2.Z, 0.0)
        ps_madd      f9,    f6, f28,  f9    # f9  += (M2[1][0], M2[1][1]) * (N2.X, N2.Y)
        ps_madd     f12,    f7, f29, f12    # f12 += (M2[1][2], M2[1][3]) * (N2.Z, 0.0)
        ps_madd     f10,    f6, f30, f10    # f10 += (M2[2][0], M2[2][1]) * (N2.X, N2.Y)
        ps_madd     f13,    f7, f31, f13    # f13 += (M2[2][2], M2[2][3]) * (N2.Z, 0.0)
        ps_add       f2,    f8, f11         # f2   = NX0 + NX1
        ps_add       f3,    f9, f12         # f3   = NY0 + NY1
        ps_add       f4,   f10, f13         # f4   = NZ0 + NZ1
        ps_sum0      f5,    f3,  f4,  f3    # f7   = (f3[0] + f3[1],  f4[1])
        ps_sum1      f6,    f2,  f2,  f2    # f6   = (f2[0],  f2[0] + f2[1])
        ps_neg      f10,    f1              # f10  = (-W0, -W1)
        psq_l        f3, 4(%0),   1,   5    # f3   = (V.Z, 1.0)
        ps_sum1      f5,    f4,  f5,  f5    # f5   = (f5[0],  f4[0] + f5[1])
        psq_l        f2, 0(%0),   0,   5    # f2   = (V.X, V.Y)
        ps_sum0     f10,   f10,  f0, f10    # f10  = (-W0-W1, 1.0)
        psq_st       f6, 4(%1),   0,   6    # store (---, N.X)
        psq_st       f5, 8(%1),   0,   6    # store (N.Y, N.Z)
        ps_muls1     f5,    f3,  f1         # f5   = (V.Z, 1.0) * W1
        ps_sum0     f10,   f10, f10, f10    # f10  = (1.0-W0-W1, 1.0)
        ps_muls1     f4,    f2,  f1         # f4   = (V.X, V.Y) * W1
        ps_mul      f11,    f4, f20         # f11  = (M1[0][0], M1[0][1]) * (V1.X, V1.Y)
        ps_mul      f12,    f4, f22         # f12  = (M1[1][0], M1[1][1]) * (V1.X, V1.Y)
        ps_muls0     f6,    f2, f10         # f6   = (V.X, V.Y) * W2
        ps_muls0     f7,    f3, f10         # f7   = (V.Z, 1.0) * W2
        ps_muls0     f2,    f2,  f1         # f2   = (V.X, V.Y) * W0
        ps_muls0     f3,    f3,  f1         # f3   = (V.Z, 1.0) * W0
        ps_mul      f13,    f4, f24         # f13  = (M1[2][0], M1[2][1]) * (V1.X, V1.Y)
        ps_madd     f11,    f5, f21, f11    # f11 += (M1[0][2], M1[0][3]) * (V1.Z, W1)
        ps_mul       f8,    f2, f14         # f8   = (M0[0][0], M0[0][1]) * (V0.X, V0.Y)
        ps_mul       f9,    f2, f16         # f9   = (M0[1][0], M0[1][1]) * (V0.X, V0.Y)
        ps_mul      f10,    f2, f18         # f10  = (M0[2][0], M0[2][1]) * (V0.X, V0.Y)
        ps_madd     f12,    f5, f23, f12    # f12 += (M1[1][2], M1[1][3]) * (V1.Z, W1)
        ps_madd      f8,    f3, f15,  f8    # f8  += (M0[0][2], M0[0][3]) * (V0.Z, W0)
        ps_madd      f9,    f3, f17,  f9    # f9  += (M0[1][2], M0[1][3]) * (V0.Z, W0)
        ps_madd     f10,    f3, f19, f10    # f10 += (M0[2][2], M0[2][3]) * (V0.Z, W0)
        ps_madd     f13,    f5, f25, f13    # f13 += (M1[2][2], M1[2][3]) * (V1.Z, W1)
        ps_madd      f8,    f6, f26,  f8    # f8  += (M2[0][0], M2[0][1]) * (V2.X, V2.Y)
        ps_madd     f11,    f7, f27, f11    # f11 += (M2[0][2], M2[0][3]) * (V2.Z, W2)
        ps_madd      f9,    f6, f28,  f9    # f9  += (M2[1][0], M2[1][1]) * (V2.X, V2.Y)
        ps_madd     f12,    f7, f29, f12    # f12 += (M2[1][2], M2[1][3]) * (V2.Z, W2)
        ps_madd     f10,    f6, f30, f10    # f10 += (M2[2][0], M2[2][1]) * (V2.X, V2.Y)
        ps_madd     f13,    f7, f31, f13    # f13 += (M2[2][2], M2[2][3]) * (V2.Z, W2)
        ps_add       f2,    f8, f11         # f2   = VX0 + VX1
        ps_add       f3,    f9, f12         # f3   = VY0 + VY1
        ps_add       f4,   f10, f13         # f4   = VZ0 + VZ1
        addi         %0,    %0,  16         # pVtxDataSrc++  (sizeof(GCSkinVert_CPU) == 16)
        ps_sum0      f5,    f2,  f3,  f2    # f7   = (f2[0] + f2[1],  f3[1])
        ps_sum0      f6,    f4,  f4,  f4    # f6   = (f4[0],  f4[0] + f4[1])
        ps_sum1      f5,    f3,  f5,  f5    # f5   = (f5[0],  f3[0] + f5[1])
        psq_st       f6, 4(%1),   1,   5    # store (V.Z, ---)
        psq_st       f5, 0(%1),   0,   5    # store (V.X, V.Y)
        addi         %1,    %1,  12         # pVtxDataDst++  (sizeof(GCSkinVert_PN)  == 12)

        bdnz        CPU3_START              # decrement CTR, if > 0, goto start of loop
    CPU3_END:

    "
    : "+r" (pVtxDataSrc), "+r" (pVtxDataDst)
    : "b" (pMtxData), "r" (NVertsCPU1), "r" (NVertsCPU2), "r" (NVertsCPU3), "b" (&FZERO)
    : "ctr",
      "r0",   "r26",  "r27",  "r28",  "r29",  "r30",  "r31",
      "fr0",  "fr1",  "fr2",  "fr3",  "fr4",  "fr5",  "fr6",  "fr7",  "fr8",  "fr9",  "fr10",
      "fr11", "fr12", "fr13", "fr14", "fr15", "fr16", "fr17", "fr18", "fr19", "fr20", "fr21",
      "fr22", "fr23", "fr24", "fr25", "fr26", "fr27", "fr28", "fr29", "fr30", "fr31"
    );
}

//==========================================================================
/*
static void XFormVerts( register GCSkinVert_CPU* pVtxDataSrc,
                        register GCSkinVert_PN*  pVtxDataDst,
                        register GCPosMtx*       pMtxData,
                        register s32             NVertsCPU1,
                        register s32             NVertsCPU2,
                        register s32             NVertsCPU3 )
{
    register Vec       VPOS0, VPOS1, VPOS2;
    register Vec       VNRM0, VNRM1, VNRM2;
    register GCPosMtx* pMtx0;
    register GCPosMtx* pMtx1;
    register GCPosMtx* pMtx2;
    register f32       W[3];

    //--- XForm the 1 Bone/Vtx list
    while( NVertsCPU1 > 0 )
    {
        //--- get the matrix pointer(s)
        pMtx0 = &pMtxData[ (pVtxDataSrc->MtxIdx & 0x001F) ];

        //--- dequantize the position and normal
        VPOS0.x = (f32)pVtxDataSrc->PX * (1.0f/(1<<GC_FRAC_POS));
        VPOS0.y = (f32)pVtxDataSrc->PY * (1.0f/(1<<GC_FRAC_POS));
        VPOS0.z = (f32)pVtxDataSrc->PZ * (1.0f/(1<<GC_FRAC_POS));
        VNRM0.x = (f32)pVtxDataSrc->NX * (1.0f/(1<<GC_FRAC_NRM));
        VNRM0.y = (f32)pVtxDataSrc->NY * (1.0f/(1<<GC_FRAC_NRM));
        VNRM0.z = (f32)pVtxDataSrc->NZ * (1.0f/(1<<GC_FRAC_NRM));

        //--- transform the vert and normal
        PSMTXMultVec( (MtxPtr)pMtx0, &VPOS0, &VPOS0 );
        MTXMultVecSR( (MtxPtr)pMtx0, &VNRM0, &VNRM0 );

        //--- quantize the pos/nrm back in the destination array
        pVtxDataDst->PX = (s16)(VPOS0.x * (1<<GC_FRAC_POS));
        pVtxDataDst->PY = (s16)(VPOS0.y * (1<<GC_FRAC_POS));
        pVtxDataDst->PZ = (s16)(VPOS0.z * (1<<GC_FRAC_POS));
        pVtxDataDst->NX = (s16)(VNRM0.x * (1<<GC_FRAC_NRM));
        pVtxDataDst->NY = (s16)(VNRM0.y * (1<<GC_FRAC_NRM));
        pVtxDataDst->NZ = (s16)(VNRM0.z * (1<<GC_FRAC_NRM));

        NVertsCPU1--;
        pVtxDataSrc++;
        pVtxDataDst++;
    }

    //--- XForm the 2 Bones/Vtx list
    while( NVertsCPU2 > 0 )
    {
        //--- get the matrix pointer(s)
        pMtx0 = &pMtxData[ (pVtxDataSrc->MtxIdx & 0x001F) ];
        pMtx1 = &pMtxData[ ((pVtxDataSrc->MtxIdx >> 5) & 0x001F) ];

        //--- dequantize the position, normal and weights
        VPOS0.x = (f32)pVtxDataSrc->PX * (1.0f/(1<<GC_FRAC_POS));
        VPOS0.y = (f32)pVtxDataSrc->PY * (1.0f/(1<<GC_FRAC_POS));
        VPOS0.z = (f32)pVtxDataSrc->PZ * (1.0f/(1<<GC_FRAC_POS));
        VNRM0.x = (f32)pVtxDataSrc->NX * (1.0f/(1<<GC_FRAC_NRM));
        VNRM0.y = (f32)pVtxDataSrc->NY * (1.0f/(1<<GC_FRAC_NRM));
        VNRM0.z = (f32)pVtxDataSrc->NZ * (1.0f/(1<<GC_FRAC_NRM));
        W[0]    = (f32)pVtxDataSrc->W0 * (1.0f/(1<<GC_FRAC_WEIGHT));
        W[1]    = (f32)pVtxDataSrc->W1 * (1.0f/(1<<GC_FRAC_WEIGHT));

        //--- transform the vert, V = ((M0 * V) * W0) + ((M1 * V) * W1)
        PSMTXMultVec( (MtxPtr)pMtx1,  &VPOS0, &VPOS1 );
        PSMTXMultVec( (MtxPtr)pMtx0,  &VPOS0, &VPOS0 );
        PSVECScale  ( &VPOS1, &VPOS1, W[1] );
        PSVECScale  ( &VPOS0, &VPOS0, W[0] );
        PSVECAdd    ( &VPOS0, &VPOS1, &VPOS0 );

        //--- transform the normal, N = ((M0 * N) * W0) + ((M1 * N) * W1)
        MTXMultVecSR( (MtxPtr)pMtx1,  &VNRM0, &VNRM1 );
        MTXMultVecSR( (MtxPtr)pMtx0,  &VNRM0, &VNRM0 );
        PSVECScale  ( &VNRM1, &VNRM1, W[1] );
        PSVECScale  ( &VNRM0, &VNRM0, W[0] );
        PSVECAdd    ( &VNRM0, &VNRM1, &VNRM0 );

        //--- quantize the pos/nrm back in the destination array
        pVtxDataDst->PX = (s16)(VPOS0.x * (1<<GC_FRAC_POS));
        pVtxDataDst->PY = (s16)(VPOS0.y * (1<<GC_FRAC_POS));
        pVtxDataDst->PZ = (s16)(VPOS0.z * (1<<GC_FRAC_POS));
        pVtxDataDst->NX = (s16)(VNRM0.x * (1<<GC_FRAC_NRM));
        pVtxDataDst->NY = (s16)(VNRM0.y * (1<<GC_FRAC_NRM));
        pVtxDataDst->NZ = (s16)(VNRM0.z * (1<<GC_FRAC_NRM));

        NVertsCPU2--;
        pVtxDataSrc++;
        pVtxDataDst++;
    }

    //--- XForm the 3 Bones/Vtx list
    while( NVertsCPU3 > 0 )
    {
        //--- get the matrix pointer(s)
        pMtx0 = &pMtxData[ (pVtxDataSrc->MtxIdx & 0x001F) ];
        pMtx1 = &pMtxData[ ((pVtxDataSrc->MtxIdx >> 5) & 0x001F) ];
        pMtx2 = &pMtxData[ ((pVtxDataSrc->MtxIdx >> 10) & 0x001F) ];

        //--- dequantize the position, normal and weights
        VPOS0.x = (f32)pVtxDataSrc->PX * (1.0f/(1<<GC_FRAC_POS));
        VPOS0.y = (f32)pVtxDataSrc->PY * (1.0f/(1<<GC_FRAC_POS));
        VPOS0.z = (f32)pVtxDataSrc->PZ * (1.0f/(1<<GC_FRAC_POS));
        VNRM0.x = (f32)pVtxDataSrc->NX * (1.0f/(1<<GC_FRAC_NRM));
        VNRM0.y = (f32)pVtxDataSrc->NY * (1.0f/(1<<GC_FRAC_NRM));
        VNRM0.z = (f32)pVtxDataSrc->NZ * (1.0f/(1<<GC_FRAC_NRM));
        W[0]    = (f32)pVtxDataSrc->W0 * (1.0f/(1<<GC_FRAC_WEIGHT));
        W[1]    = (f32)pVtxDataSrc->W1 * (1.0f/(1<<GC_FRAC_WEIGHT));

        W[2] = 1.0f - W[0] - W[1];

        //--- transform the vert, V = ((M0 * V) * W0) + ((M1 * V) * W1) + ((M2 * V) * W2)
        PSMTXMultVec( (MtxPtr)pMtx2,  &VPOS0, &VPOS2 );
        PSMTXMultVec( (MtxPtr)pMtx1,  &VPOS0, &VPOS1 );
        PSMTXMultVec( (MtxPtr)pMtx0,  &VPOS0, &VPOS0 );
        PSVECScale  ( &VPOS2, &VPOS2, W[2] );
        PSVECScale  ( &VPOS1, &VPOS1, W[1] );
        PSVECScale  ( &VPOS0, &VPOS0, W[0] );
        PSVECAdd    ( &VPOS1, &VPOS2, &VPOS1 );
        PSVECAdd    ( &VPOS0, &VPOS1, &VPOS0 );

        //--- transform the normal, N = ((M0 * N) * W0) + ((M1 * N) * W1) + ((M2 * N) * W2)
        MTXMultVecSR( (MtxPtr)pMtx2,  &VNRM0, &VNRM2 );
        MTXMultVecSR( (MtxPtr)pMtx1,  &VNRM0, &VNRM1 );
        MTXMultVecSR( (MtxPtr)pMtx0,  &VNRM0, &VNRM0 );
        PSVECScale  ( &VNRM2, &VNRM2, W[2] );
        PSVECScale  ( &VNRM1, &VNRM1, W[1] );
        PSVECScale  ( &VNRM0, &VNRM0, W[0] );
        PSVECAdd    ( &VNRM1, &VNRM2, &VNRM1 );
        PSVECAdd    ( &VNRM0, &VNRM1, &VNRM0 );

        //--- quantize the pos/nrm back in the destination array
        pVtxDataDst->PX = (s16)(VPOS0.x * (1<<GC_FRAC_POS));
        pVtxDataDst->PY = (s16)(VPOS0.y * (1<<GC_FRAC_POS));
        pVtxDataDst->PZ = (s16)(VPOS0.z * (1<<GC_FRAC_POS));
        pVtxDataDst->NX = (s16)(VNRM0.x * (1<<GC_FRAC_NRM));
        pVtxDataDst->NY = (s16)(VNRM0.y * (1<<GC_FRAC_NRM));
        pVtxDataDst->NZ = (s16)(VNRM0.z * (1<<GC_FRAC_NRM));

        NVertsCPU3--;
        pVtxDataSrc++;
        pVtxDataDst++;
    }
}
*/
//==========================================================================
