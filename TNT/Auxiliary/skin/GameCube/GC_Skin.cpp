////////////////////////////////////////////////////////////////////////////
//
// GC_Skin.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_memory.hpp"

#include "Q_Engine.hpp"
#include "Q_GC.hpp"

#include "Skin.hpp"
#include "GC_SkinHelp.hpp"

#include "AM_Group.hpp" //###


////////////////////////////////////////////////////////////////////////////
// QSkin IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

QSkin::QSkin( void )
{
    InitData();
}

//==========================================================================

QSkin::QSkin( char* pSkinFilename )
{
    InitData();
    SetupFromResource( pSkinFilename );
}

//==========================================================================

QSkin::~QSkin( void )
{
    KillData();
}

//==========================================================================

void QSkin::InitData( void )
{
    m_Name[0]       = '\0';
    m_NBones        = 0;
    m_pBoneNames    = NULL;
    m_Flags         = 0;
    m_NMeshes       = 0;
    m_pMeshes       = NULL;
    m_NSubMeshes    = 0;
    m_pSubMeshes    = NULL;
    m_NMorphTargets = 0;
    m_pMorphTargets = NULL;
    m_NTextures     = 0;
    m_pTextureNames = NULL;
    m_pTextureArray = NULL;

    m_pRawData      = NULL;

    m_DeltaDataSize   = 0;
    m_pDeltaValues    = NULL;
    m_VertexDataSize  = 0;
    m_DListDataSize   = 0;
}

//==========================================================================

void QSkin::KillData( void )
{
    if( m_pRawData != NULL )
        x_free( m_pRawData );

    InitData();
}

//==========================================================================

void QSkin::SetupFromResource( char* pFilename )
{
    X_FILE* pFilePtr;

    //--- open the file
    pFilePtr = x_fopen( pFilename, "rb" );
    ASSERT( pFilePtr != NULL );

    SetupFromResource( pFilePtr );

    //--- close the file
    x_fclose( pFilePtr );
}

//==========================================================================

static void OffsetSubMeshDataPtrs( GCSkinSubMesh* pSubMeshes, s32 NSubMeshes, u32 VtxDataPtr, u32 DListDataPtr, u32 DeltaDataPtr )
{
    s32 i;

    #define ADD_PTR_OFFSET( Ptr, Offset )   (((u32)(Ptr) == GC_INVALID_PTR_OFFSET) ? NULL : ((u32)(Ptr) + (u32)(Offset)))

    for( i = 0; i < NSubMeshes; i++ )
    {
        pSubMeshes[i].pDListGP   = (void*)          ADD_PTR_OFFSET( pSubMeshes[i].pDListGP,   DListDataPtr );
        pSubMeshes[i].pDListCPU  = (void*)          ADD_PTR_OFFSET( pSubMeshes[i].pDListCPU,  DListDataPtr );
        pSubMeshes[i].pVtxGP     = (GCSkinVert_PN*) ADD_PTR_OFFSET( pSubMeshes[i].pVtxGP,     VtxDataPtr   );
        pSubMeshes[i].pVtxCPU    = (GCSkinVert_CPU*)ADD_PTR_OFFSET( pSubMeshes[i].pVtxCPU,    VtxDataPtr   );
        pSubMeshes[i].pVtxShadow = (GCSkinVert_P*)  ADD_PTR_OFFSET( pSubMeshes[i].pVtxShadow, VtxDataPtr   );
        pSubMeshes[i].pUVs       = (GCSkinVert_UV*) ADD_PTR_OFFSET( pSubMeshes[i].pUVs,       VtxDataPtr   );
        pSubMeshes[i].pDeltasCPU = (GCSkinDelta*)   ADD_PTR_OFFSET( pSubMeshes[i].pDeltasCPU, DeltaDataPtr );
    }
}

//==========================================================================

void QSkin::SetupFromResource( X_FILE* pFilePtr )
{
    ////////////////////////////////////////////////////////////////////////
    // SPECIAL NOTE ABOUT LOADING:
    //      ANY CHANGES TO THE DATA FORMAT WILL ALSO AFFECT THE OTHER
    //      OVERLOADED SetupFromResource FUNCTIONS. TAKE THAT INTO ACCOUNT!
    ////////////////////////////////////////////////////////////////////////

    ASSERT( pFilePtr != NULL );

    GCSkinFileHeader Hdr;

    u32   RawDataSize;
    byte* pTempRaw;

    //--- clear out any previous data
    KillData();

    //--- read in the file header
    x_fread( &Hdr, sizeof(GCSkinFileHeader), 1, pFilePtr );

    //--- fill out as much of the skin data as we can from the header
    x_strcpy( m_Name, Hdr.Name );
    m_Flags           = Hdr.Flags;
    m_NTextures       = Hdr.NTextures;
    m_NBones          = Hdr.NBones;
    m_NMeshes         = Hdr.NMeshes;
    m_NSubMeshes      = Hdr.NSubMeshes;
    m_NMorphTargets   = Hdr.NMorphTargets;
    m_DeltaDataSize   = Hdr.DeltaDataSize;
    m_VertexDataSize  = Hdr.VertexDataSize;
    m_DListDataSize   = Hdr.DListDataSize;

    ASSERT( m_NBones <= SKIN_MAX_MATRICES );

    //--- Find the amount of memory to allocate
    RawDataSize  = m_NTextures      * SKIN_TEXTURE_NAME_LENGTH;
    RawDataSize += m_NBones         * SKIN_BONE_NAME_LENGTH;
    RawDataSize += m_NMeshes        * sizeof(GCSkinMesh);
    RawDataSize += m_NSubMeshes     * sizeof(GCSkinSubMesh);
    RawDataSize += m_NMorphTargets  * sizeof(GCSkinMorphTgt);
    RawDataSize += m_DeltaDataSize  + Hdr.NPadBytesDeltas + Hdr.NPadBytesVtxData;
    RawDataSize += m_VertexDataSize + m_DListDataSize;

    //--- Allocate memory for all the data
    m_pRawData = (byte*)x_malloc( RawDataSize );
    ASSERT( m_pRawData != NULL );

    //--- Assign real data pointers into raw data buffer
    pTempRaw = m_pRawData;

    // Set data pointer to raw data position;        Then increment the temp raw pointer by data size.
    m_pTextureNames = (char*)          pTempRaw;     pTempRaw += m_NTextures      * SKIN_TEXTURE_NAME_LENGTH;
    m_pBoneNames    = (char*)          pTempRaw;     pTempRaw += m_NBones         * SKIN_BONE_NAME_LENGTH;
    m_pMeshes       = (GCSkinMesh*)    pTempRaw;     pTempRaw += m_NMeshes        * sizeof(GCSkinMesh);
    m_pSubMeshes    = (GCSkinSubMesh*) pTempRaw;     pTempRaw += m_NSubMeshes     * sizeof(GCSkinSubMesh);
    m_pMorphTargets = (GCSkinMorphTgt*)pTempRaw;     pTempRaw += m_NMorphTargets  * sizeof(GCSkinMorphTgt) + Hdr.NPadBytesDeltas;
    m_pDeltaValues  = (byte*)          pTempRaw;     pTempRaw += m_DeltaDataSize  + Hdr.NPadBytesVtxData;
    m_pVertexData   = (byte*)          pTempRaw;     pTempRaw += m_VertexDataSize;
    m_pDListData    = (byte*)          pTempRaw;     pTempRaw += m_DListDataSize;

    //--- Read in the data
    x_fread( m_pRawData, sizeof(byte), RawDataSize, pFilePtr );

    //--- adjust any info in the Mesh/SubMesh data structures that relate to pointer offsets
    OffsetSubMeshDataPtrs( m_pSubMeshes, m_NSubMeshes, (u32)m_pVertexData, (u32)m_pDListData, (u32)m_pDeltaValues );

    //--- flush the data from the CPU cache
    DCFlushRange( m_pVertexData, m_VertexDataSize + m_DListDataSize );
}

//==========================================================================

void QSkin::SetupFromResource( byte* pByteStream )
{
    ////////////////////////////////////////////////////////////////////////
    // SPECIAL NOTE ABOUT LOADING:
    //      ANY CHANGES TO THE DATA FORMAT WILL ALSO AFFECT THE OTHER
    //      OVERLOADED SetupFromResource FUNCTIONS. TAKE THAT INTO ACCOUNT!
    ////////////////////////////////////////////////////////////////////////

    ASSERT( pByteStream != NULL );

    GCSkinFileHeader    Hdr;

    u32   RawDataSize;
    byte* pTempRaw;

    //--- clear out any previous data
    KillData();

    //--- read in the file header
    x_memcpy( &Hdr, pByteStream, sizeof(GCSkinFileHeader) );
    pByteStream += sizeof(GCSkinFileHeader);

    //--- fill out as much of the skin data as we can from the header
    x_strcpy( m_Name, Hdr.Name );
    m_Flags           = Hdr.Flags;
    m_NTextures       = Hdr.NTextures;
    m_NBones          = Hdr.NBones;
    m_NMeshes         = Hdr.NMeshes;
    m_NSubMeshes      = Hdr.NSubMeshes;
    m_NMorphTargets   = Hdr.NMorphTargets;
    m_DeltaDataSize   = Hdr.DeltaDataSize;
    m_VertexDataSize  = Hdr.VertexDataSize;
    m_DListDataSize   = Hdr.DListDataSize;

    ASSERT( m_NBones <= SKIN_MAX_MATRICES );

    //--- Find the amount of memory to allocate
    RawDataSize  = m_NTextures      * SKIN_TEXTURE_NAME_LENGTH;
    RawDataSize += m_NBones         * SKIN_BONE_NAME_LENGTH;
    RawDataSize += m_NMeshes        * sizeof(GCSkinMesh);
    RawDataSize += m_NSubMeshes     * sizeof(GCSkinSubMesh);
    RawDataSize += m_NMorphTargets  * sizeof(GCSkinMorphTgt);
    RawDataSize += m_DeltaDataSize  + Hdr.NPadBytesDeltas + Hdr.NPadBytesVtxData;
    RawDataSize += m_VertexDataSize + m_DListDataSize;

    //--- Allocate memory for all the data
    m_pRawData = (byte*)x_malloc( RawDataSize );
    ASSERT( m_pRawData != NULL );

    //--- Assign real data pointers into raw data buffer
    pTempRaw = m_pRawData;

    // Set data pointer to raw data position;        Then increment the temp raw pointer by data size.
    m_pTextureNames = (char*)          pTempRaw;     pTempRaw += m_NTextures      * SKIN_TEXTURE_NAME_LENGTH;
    m_pBoneNames    = (char*)          pTempRaw;     pTempRaw += m_NBones         * SKIN_BONE_NAME_LENGTH;
    m_pMeshes       = (GCSkinMesh*)    pTempRaw;     pTempRaw += m_NMeshes        * sizeof(GCSkinMesh);
    m_pSubMeshes    = (GCSkinSubMesh*) pTempRaw;     pTempRaw += m_NSubMeshes     * sizeof(GCSkinSubMesh);
    m_pMorphTargets = (GCSkinMorphTgt*)pTempRaw;     pTempRaw += m_NMorphTargets  * sizeof(GCSkinMorphTgt) + Hdr.NPadBytesDeltas;
    m_pDeltaValues  = (byte*)          pTempRaw;     pTempRaw += m_DeltaDataSize  + Hdr.NPadBytesVtxData;
    m_pVertexData   = (byte*)          pTempRaw;     pTempRaw += m_VertexDataSize;
    m_pDListData    = (byte*)          pTempRaw;     pTempRaw += m_DListDataSize;

    //---  Read in the data
    x_memcpy( m_pRawData, pByteStream, RawDataSize );
    pByteStream += RawDataSize;

    //--- adjust any info in the Mesh/SubMesh data structures that relate to pointer offsets
    OffsetSubMeshDataPtrs( m_pSubMeshes, m_NSubMeshes, (u32)m_pVertexData, (u32)m_pDListData, (u32)m_pDeltaValues );

    //--- flush the data from the CPU cache
    DCFlushRange( m_pVertexData, m_VertexDataSize + m_DListDataSize );
}

//==========================================================================

void QSkin::SetupTEV( u32 SubMeshFlags )
{
    xbool   FixAlphaOn;
    GXColor FixedClr = {0,0,0,0};

    GC_GetFixedAlpha( FixAlphaOn, FixedClr.a );

    s32 LastAlphaTevStage;
    s32 DXT3AlphaMapID = GC_GetDXT3AlphaMapStage( 0 );

    if( SubMeshFlags & SKIN_SUBMESH_FLAG_SHADOW )
    {
        if( !FixAlphaOn )
            FixedClr.a = 255;

        GXSetNumChans    ( 1 );
        GXSetNumTevStages( 1 );
        GXSetNumTexGens  ( 0 );
        GXSetTevOrder    ( GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0 );
        GXSetTevColor    ( GX_TEVREG0, FixedClr );
        GXSetTevColorIn  ( GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,    GX_CC_C0 );
        GXSetTevColorOp  ( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );
        GXSetTevAlphaIn  ( GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,    GX_CA_A0 );
        GXSetTevAlphaOp  ( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );
    }
    else if( SubMeshFlags & SKIN_SUBMESH_FLAG_ENVPASS )
    {
        GXColor EnvColorA  = { 0, 0, 0, 50 };

        if( (DXT3AlphaMapID == -1) || FixAlphaOn )
            LastAlphaTevStage = 1;
        else
            LastAlphaTevStage = 2;

        GXSetNumChans    ( 1 );
        GXSetNumTevStages( LastAlphaTevStage + 1 );
        GXSetNumTexGens  ( 2 );
        GXSetTexCoordGen ( GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY );
        GXSetTexCoordGen2( GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_NRM,  GX_IDENTITY, GX_FALSE, GX_PTTEXMTX0 );
        GXSetTevOrder    ( GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0 );
        GXSetTevOrder    ( GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0 );

        if( LastAlphaTevStage == 2 )
        {
            GXSetTevOrder  ( GX_TEVSTAGE2, GX_TEXCOORD0, TEXMAP_ID[DXT3AlphaMapID], GX_COLOR0A0 );

            GXSetTevColorOp( GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );
            GXSetTevAlphaOp( GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );

            GXSetTevColorIn( GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV );
            GXSetTevAlphaIn( GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_RASA, GX_CA_TEXA, GX_CA_ZERO );
        }

        GXSetTevColorIn  ( GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC,    GX_CC_ZERO );
        GXSetTevColorOp  ( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );
        GXSetTevAlphaOp  ( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );

        GXSetTevColor    ( GX_TEVREG1, EnvColorA );
        GXSetTevColorIn  ( GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_A1,   GX_CC_TEXC,    GX_CC_CPREV );
        GXSetTevColorOp  ( GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );
        GXSetTevAlphaOp  ( GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );

        if( FixAlphaOn )
        {
            GXSetTevColor  ( GX_TEVREG0, FixedClr );
            GXSetTevAlphaIn( GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A0 );
            GXSetTevAlphaIn( GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A0 );
        }
        else
        {
            GXSetTevAlphaIn( GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_RASA, GX_CA_TEXA, GX_CA_ZERO );
            GXSetTevAlphaIn( GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV );
        }
    }
    else
    {
        if( (DXT3AlphaMapID == -1) || FixAlphaOn )
            LastAlphaTevStage = 0;
        else
            LastAlphaTevStage = 1;

        GXSetNumChans    ( 1 );
        GXSetNumTevStages( LastAlphaTevStage + 1 );
        GXSetNumTexGens  ( 1 );
        GXSetTexCoordGen ( GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY );
        GXSetTevOrder    ( GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0 );

        if( LastAlphaTevStage == 1 )
        {
            GXSetTevOrder  ( GX_TEVSTAGE1, GX_TEXCOORD0, TEXMAP_ID[DXT3AlphaMapID], GX_COLOR0A0 );

            GXSetTevColorOp( GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );
            GXSetTevAlphaOp( GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );

            GXSetTevColorIn( GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV );

            GXSetTevAlphaIn( GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO );
        }

        GXSetTevColorOp( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );
        GXSetTevAlphaOp( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );

        GXSetTevColorIn( GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC,    GX_CC_ZERO );

        if( FixAlphaOn )
        {
            GXSetTevColor  ( GX_TEVREG0, FixedClr );
            GXSetTevAlphaIn( TEVSTAGE_ID[LastAlphaTevStage], GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A0 );
        }
        else
        {
            GXSetTevAlphaIn( TEVSTAGE_ID[LastAlphaTevStage], GX_CA_ZERO, GX_CA_RASA, GX_CA_TEXA, GX_CA_ZERO );
        }
    }
}

//==========================================================================

//////////////////////////////////////////////////////////////////
// ABOUT ANIMATION MIRRORING:
//
//  The skeleton is assumed to be aligned facing positive Z axis,
//  with the left side on positive X axis, which means mirroring
//  for left-right is done using the YZ plane.
//
//  To get the mirrored animation, instead of using the current
//  bone's ParentToBone offset, we need to get that offset from
//  the bone's mirrored counterpart.  That offset is mirrored
//  across the YZ plane by simply negating the X component.
//
//  The rotation values are handled in a similar fashion, getting
//  the mirrored couterpart's rotation, and mirroring it across
//  the YZ plane as well.  To do that we just negate the Yaw and
//  Roll(Y and Z) values.
//
//////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////
// ABOUT ANIMATION BONE ROTATIONS:
//
//  The bone rotations are in XZY order. That is, you are
//  expected to pitch, roll, and yaw, in that order.
//
//  In matrix form, this would be A' = RyRzRx * A, where:
//
//  Rx = |   1    0    0    0 |
//       |   0   cx  -sx    0 |
//       |   0   sx   cx    0 |
//       |   0    0    0    1 |
//
//  Ry = |  cy    0   sy    0 |
//       |   0    1    0    0 |
//       | -sy    0   cy    0 |
//       |   0    0    0    1 |
//
//  Rz = |  cz  -sz    0    0 |
//       |  sz   cz    0    0 |
//       |   0    0    1    0 |
//       |   0    0    0    1 |
//
//  We then add the translation. The translation is equal to
//  the bone's local translation run through the parent's
//  transformation matrix.  OR ...
//
//  P' = Tp * P,    where Tp is the parent's transform matrix
//
//  Now, the transformation looks like:
//      A' = TRyRzRx * A
//
//  So...the transformation matrix (TRyRzRx) is:
//
//  T = |     cycz   -cxcysz+sxsy    sxcysz+cxsy      Tx |
//      |       sz           cxcz          -sxcz      Ty |
//      |    -sycz    cxsysz+sxcy   -sxsysz+cxcy      Tz |
//      |        0              0              0       1 |
//
//////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////
// ABOUT ANIMATION - SKIN VERTEX DATA TRANSFORM:
//
//  TempM currently contains the transforms for each bone, assuming
//  that the points for the bone are given with respect to that
//  bone. i.e. the points for Bone X are given assuming Bone X is
//  at the origin.
//
//  Well, it turns out that the data is given so that each of the
//  points are given with respect to the root node, so... the first
//  thing we have to do is translate the points so that they are
//  given with respect to the bone, not the origin.
//
//  This is done by translating the point by -OriginToBone
//
//  NOTE: for mirroring, OriginToBone must be retrieved from the
//  bone's mirror counterpart, and then mirrored across the YZ plane.
//  This is easily done by negating the X component, but since we are
//  already using -OriginToBone, we negate the Y and Z for optimization.
//
//////////////////////////////////////////////////////////////////
void QSkin::BuildMatrices( GCMtxGroup*    pMtxData,
                           radian3*       pBoneRots,
                           vector3&       WorldPos,
                           radian         WorldYaw,
                           xbool          bMirrored,
                           f32            WorldScale,
                           vector3*       pBoneScales,
                           void*          pAnimGroupPtr,
                           s32            NShadowMtx,
                           matrix4*       pShadowMatrices,
                           GCPosMtxArray* pShadowMtxData )
{
    // Starting rotations of the model.
    f32         sinYaw, cosYaw;
    GCPosMtx    WorldOrientation;

    x_fastsincos( WorldYaw, sinYaw, cosYaw );

    MTXIdentity( (MtxPtr)WorldOrientation );
    MTXRotTrig(  (MtxPtr)WorldOrientation, 'Y', sinYaw, cosYaw );

    BuildMatrices( pMtxData,
                   pBoneRots,
                   WorldPos,
                   &WorldOrientation,
                   bMirrored,
                   WorldScale,
                   pBoneScales,
                   pAnimGroupPtr,
                   NShadowMtx,
                   pShadowMatrices,
                   pShadowMtxData );
}


//-------------------------------------------------------------------------------------------------------------------------------------
void QSkin::BuildMatrices( GCMtxGroup*    pMtxData,
                           radian3*       pBoneRots,
                           vector3&       WorldPos,
                           GCPosMtx*      pWorldOrientation,
                           xbool          bMirrored,
                           f32            WorldScale,
                           vector3*       pBoneScales,
                           void*          pAnimGroupPtr,
                           s32            NShadowMtx      = 0,
                           matrix4*       pShadowMatrices = NULL,
                           GCPosMtxArray* pShadowMtxData  = NULL )
{
    GCPosMtx    BaseMtx;
    GCPosMtx    IDMtx;
    GCPosMtx    GCW2V;
    s32         i, j;
    s32         NBones;
    f32         FZERO = 0.0f;
    anim_bone*  pAnimBones;
    radian3     Rot;
    vector3     Trans;
    vector3     Orig2Bone;
    matrix4     TempW2V;

    ASSERT( pMtxData != NULL );
    ASSERT( pBoneRots != NULL );
    ASSERT( pAnimGroupPtr != NULL );

    ASSERT( (NShadowMtx <= 0) || ((NShadowMtx > 0) && (pShadowMatrices != NULL) && (pShadowMtxData != NULL)) );

    //--- Get pointers for anim data
    pAnimBones = ((anim_group*)pAnimGroupPtr)->Bone;
    NBones     = ((anim_group*)pAnimGroupPtr)->NBones;

    ASSERT( NBones == m_NBones );

    //--- Build the base matrices(local-2-world translation for GX, ID for CPU)
    MTXTrans( (MtxPtr)BaseMtx, WorldPos.X, WorldPos.Y, WorldPos.Z );
    PSMTXIdentity( (MtxPtr)IDMtx );

    //--- Build world-2-view matrix: RY(180) * W2V * T(WPos)
    //--- Also build W2V matrix without L2W translation: RY(180) * W2V
    ENG_GetActiveView()->GetW2VMatrix( TempW2V );
    TempW2V.Transpose();
    MTXRotTrig ( (MtxPtr)GCW2V, 'Y', 0.0f, -1.0f );                             // GCW2V = RY(180)
    PSMTXConcat( (MtxPtr)GCW2V, (MtxPtr)TempW2V.M, (MtxPtr)GCW2V );             // GCW2V = RY(180) * W2V
    PSMTXConcat( (MtxPtr)GCW2V, (MtxPtr)BaseMtx, (MtxPtr)(pMtxData->WT2V) );    // pMtxData->WT2V = RY(180) * W2V * T(WPos)

    //--- Loop through all bones hitting parents before children
    for( i = 0; i < NBones; i++ )
    {
        //--- Get rotation and translation values for bone
        if( bMirrored )
        {
            Rot.Pitch =  pBoneRots[ pAnimBones[i].MirrorID ].Pitch;
            Rot.Yaw   = -pBoneRots[ pAnimBones[i].MirrorID ].Yaw;
            Rot.Roll  = -pBoneRots[ pAnimBones[i].MirrorID ].Roll;
            Trans.X   = -pAnimBones[ pAnimBones[i].MirrorID ].ParentToBone.X;
            Trans.Y   =  pAnimBones[ pAnimBones[i].MirrorID ].ParentToBone.Y;
            Trans.Z   =  pAnimBones[ pAnimBones[i].MirrorID ].ParentToBone.Z;
        }
        else
        {
            Rot     = pBoneRots[i];
            Trans.X = pAnimBones[i].ParentToBone.X;
            Trans.Y = pAnimBones[i].ParentToBone.Y;
            Trans.Z = pAnimBones[i].ParentToBone.Z;
        }

        //--- Apply world scale to bone translation
        //Trans.X *= WorldScale;
        //Trans.Y *= WorldScale;
        //Trans.Z *= WorldScale;

        asm __volatile__
        ("
            psq_l       f0, 0(%0),    0, 0      # f0  = (T.X, T.Y)
            psq_l       f1, 8(%0),    1, 0      # f1  = (T.Z, 1.0)
            ps_muls0    f0,    f0, (%1)         # f0  = (T.X, T.Y) * Scale
            ps_muls0    f1,    f1, (%1)         # f1  = (T.Z, 1.0) * Scale
            psq_st      f0, 0(%0),    0, 0      # store (T.X, T.Y)
            psq_st      f1, 8(%0),    1, 0      # store (T.Z, ---)
        "
        :
        : "b" (&Trans), "f" (WorldScale)
        : "fr0", "fr1"
        );


        //--- enable this code to put the characters in their zero pose
        //{
        //    Rot.Pitch = 0.0f;
        //    Rot.Yaw   = 0.0f;
        //    Rot.Roll  = 0.0f;
        //}

        register GCPosMtx* pParentMtx;
        register GCPosMtx* pParentMtxAnim;
        register GCPosMtx* pM;
        register GCPosMtx* pMA;

        //--- Get parent matrix
        if( pAnimBones[i].ParentID != -1 )
        {
            ASSERT( pAnimBones[i].ParentID < i );
            pParentMtx     = &pMtxData->L2V[ pAnimBones[i].ParentID ];
            pParentMtxAnim = &pMtxData->LAnim[ pAnimBones[i].ParentID ];
        }
        else
        {
            pParentMtx     = &BaseMtx;
            pParentMtxAnim = &IDMtx;
        }

        pM  = &pMtxData->L2V[i];
        pMA = &pMtxData->LAnim[i];

        //--- Build local XForm matrix
        {
            register f32  sx, sy, sz;
            register f32  cx, cy, cz;
            //register f32  cxcy, sxcy, sxsy, cxsy;

            //--- get sine and cosine for the rotations
            x_fastsincos( Rot.Pitch,  sx, cx );
            x_fastsincos( Rot.Yaw,    sy, cy );
            x_fastsincos( Rot.Roll,   sz, cz );

            ////--- factor out some of the multiplication(simple optimization)
            //sxcy = sx * cy;
            //cxcy = cx * cy;
            //sxsy = sx * sy;
            //cxsy = cx * sy;

            ////--- Build the transformation matrix(rotation part)
            ////--- the L2V and L2VAnim use the same rotation values
            //pM[0][0] = pMA[0][0] =  (cy * cz);
            //pM[0][1] = pMA[0][1] = -(cxcy * sz) + sxsy;
            //pM[0][2] = pMA[0][2] =  (sxcy * sz) + cxsy;
            //pM[1][0] = pMA[1][0] =  (sz);
            //pM[1][1] = pMA[1][1] =  (cx * cz);
            //pM[1][2] = pMA[1][2] = -(sx * cz);
            //pM[2][0] = pMA[2][0] = -(sy * cz);
            //pM[2][1] = pMA[2][1] =  (cxsy * sz) + sxcy;
            //pM[2][2] = pMA[2][2] = -(sxsy * sz) + cxcy;

            asm __volatile__
            ("
                ps_muls0    f0,   (%6), (%7)        # f0 = cy * cz
                ps_muls0    f3,   (%2), (%6)        # f3 = sxcy
                psq_st    (%4), 16(%0),    1,  0    # Store pM[1][0]
                ps_muls0    f6,   (%5), (%3)        # f6 = cxsy
                ps_muls0    f4,   (%5), (%6)        # f4 = cxcy
                psq_st      f0,  0(%0),    1,  0    # Store pM[0][0]
                ps_muls0    f5,   (%2), (%3)        # f5 = sxsy
                psq_st      f0,  0(%1),    1,  0    # Store pMA[0][0]
                ps_madd     f2,     f3, (%4), f6    # f2 =  ((sxcy * sz) + cxsy)
                ps_madd     f7,     f6, (%4), f3    # f7 =  ((cxsy * sz) + sxcy)
                psq_st    (%4), 16(%1),    1,  0    # Store pMA[1][0]
                ps_nmsub    f1,     f4, (%4), f5    # f1 = -((cxcy * sz) - sxsy)
                psq_st      f2,  8(%0),    1,  0    # Store pM[0][2]
                ps_neg      f3,   (%7)              # f3 = -cz
                psq_st      f2,  8(%1),    1,  0    # Store pMA[0][2]
                ps_nmsub    f8,     f5, (%4), f4    # f8 = -((sxsy * sz) - cxcy)
                psq_st      f7, 36(%0),    1,  0    # Store pM[2][1]
                ps_muls0    f5,   (%2),   f3        # f5 = sx * -cz
                psq_st      f7, 36(%1),    1,  0    # Store pMA[2][1]
                ps_muls0    f6,   (%3),   f3        # f6 = sy * -cz
                psq_st      f1,  4(%0),    1,  0    # Store pM[0][1]
                ps_muls0    f4,   (%5), (%7)        # f4 = cx * cz
                psq_st      f1,  4(%1),    1,  0    # Store pMA[0][1]
                psq_st      f8, 40(%0),    1,  0    # Store pM[2][2]
                psq_st      f8, 40(%1),    1,  0    # Store pMA[2][2]
                psq_st      f5, 24(%0),    1,  0    # Store pM[1][2]
                psq_st      f5, 24(%1),    1,  0    # Store pMA[1][2]
                psq_st      f6, 32(%0),    1,  0    # Store pM[2][0]
                psq_st      f6, 32(%1),    1,  0    # Store pMA[2][0]
                psq_st      f4, 20(%0),    1,  0    # Store pM[1][1]
                psq_st      f4, 20(%1),    1,  0    # Store pMA[1][1]
            "
            :
            : "b" (pM), "b" (pMA), "f" (sx), "f" (sy), "f" (sz), "f" (cx), "f" (cy), "f" (cz)
            : "fr0", "fr1", "fr2", "fr3", "fr4", "fr5", "fr6", "fr7", "fr8"
            );
        }

        // At this point rotate the current bone matrix by the skeleton's world orientation.
        MTXConcat( (MtxPtr)pWorldOrientation, (MtxPtr)pM, (MtxPtr)pM );
        *pMA = *pM;
        
        ////--- Apply the bone translation for the L2V matrix
        //pM[0][3] = pParentMtx[0][0] * Trans.X +
        //           pParentMtx[0][1] * Trans.Y +
        //           pParentMtx[0][2] * Trans.Z +
        //           pParentMtx[0][3];
        //pM[1][3] = pParentMtx[1][0] * Trans.X +
        //           pParentMtx[1][1] * Trans.Y +
        //           pParentMtx[1][2] * Trans.Z +
        //           pParentMtx[1][3];
        //pM[2][3] = pParentMtx[2][0] * Trans.X +
        //           pParentMtx[2][1] * Trans.Y +
        //           pParentMtx[2][2] * Trans.Z +
        //           pParentMtx[2][3];
        //
        ////--- Apply the bone translation for the L2VAnim matrix
        ////    (note that this does not include the L2W translation)
        //pMA[0][3] = pParentMtxAnim[0][0] * Trans.X +
        //            pParentMtxAnim[0][1] * Trans.Y +
        //            pParentMtxAnim[0][2] * Trans.Z +
        //            pParentMtxAnim[0][3];
        //pMA[1][3] = pParentMtxAnim[1][0] * Trans.X +
        //            pParentMtxAnim[1][1] * Trans.Y +
        //            pParentMtxAnim[1][2] * Trans.Z +
        //            pParentMtxAnim[1][3];
        //pMA[2][3] = pParentMtxAnim[2][0] * Trans.X +
        //            pParentMtxAnim[2][1] * Trans.Y +
        //            pParentMtxAnim[2][2] * Trans.Z +
        //            pParentMtxAnim[2][3];

        asm __volatile__
        ("
            psq_l       f0,  0(%4),  0,  0      # f0  = (Trans.X, Trans.Y)
            psq_l       f2,  0(%2),  0,  0      # f2  = (pPMtx[0][0], pPMtx[0][1])
            psq_l       f1,  8(%4),  1,  0      # f1  = (Trans.Z, 1.0f)
            psq_l       f3,  8(%2),  0,  0      # f3  = (pPMtx[0][2], pPMtx[0][3])
            ps_mul      f2,     f2, f0          # f2  = (pPMtx[0][0], pPMtx[0][1]) * (Trans.X, Trans.Y)
            psq_l       f4, 16(%2),  0,  0      # f4  = (pPMtx[1][0], pPMtx[1][1])
            ps_madd     f3,     f3, f1, f2      # f3  = (pPMtx[0][2], pPMtx[0][3]) * (Trans.Z, 1.0f)
            psq_l       f5, 24(%2),  0,  0      # f5  = (pPMtx[1][2], pPMtx[1][3])
            ps_sum0     f3,     f3, f2, f3      # f3  = (f3(ps0) + f3(ps1)), ----)
            psq_l       f6, 32(%2),  0,  0      # f6  = (pPMtx[2][0], pPMtx[2][1])
            ps_mul      f4,     f4, f0          # f4  = (pPMtx[1][0], pPMtx[1][1]) * (Trans.X, Trans.Y)
            psq_l       f7, 40(%2),  0,  0      # f7  = (pPMtx[2][2], pPMtx[2][3])
            ps_madd     f5,     f5, f1, f4      # f5  = (pPMtx[1][2], pPMtx[1][3]) * (Trans.Z, 1.0f)
            psq_st      f3, 12(%0),  1,  0      # Store (pM[0][3], ----)
            ps_sum0     f5,     f5, f4, f5      # f5  = (f5(ps0) + f5(ps1)), ----)
            ps_mul      f6,     f6, f0          # f6  = (pPMtx[2][0], pPMtx[2][1]) * (Trans.X, Trans.Y)
            psq_st      f5, 28(%0),  1,  0      # Store (pM[1][3], ----)
            ps_madd     f7,     f7, f1, f6      # f7  = (pPMtx[2][2], pPMtx[2][3]) * (Trans.Z, 1.0f)
            psq_l       f2,  0(%3),  0,  0      # f2  = (pPMtxAnim[0][0], pPMtxAnim[0][1])
            ps_sum0     f7,     f7, f6, f7      # f7  = (f7(ps0) + f7(ps1)), ----)
            psq_l       f3,  8(%3),  0,  0      # f3  = (pPMtxAnim[0][2], pPMtxAnim[0][3])
            psq_st      f7, 44(%0),  1,  0      # Store (pM[2][3], ----)
            ps_mul      f2,     f2, f0          # f2  = (pPMtxAnim[0][0], pPMtxAnim[0][1]) * (Trans.X, Trans.Y)
            psq_l       f4, 16(%3),  0,  0      # f4  = (pPMtxAnim[1][0], pPMtxAnim[1][1])
            ps_madd     f3,     f3, f1, f2      # f3  = (pPMtxAnim[0][2], pPMtxAnim[0][3]) * (Trans.Z, 1.0f)
            psq_l       f5, 24(%3),  0,  0      # f5  = (pPMtxAnim[1][2], pPMtxAnim[1][3])
            ps_sum0     f3,     f3, f2, f3      # f3  = (f3(ps0) + f3(ps1)), ----)
            psq_l       f6, 32(%3),  0,  0      # f6  = (pPMtxAnim[2][0], pPMtxAnim[2][1])
            ps_mul      f4,     f4, f0          # f4  = (pPMtxAnim[1][0], pPMtxAnim[1][1]) * (Trans.X, Trans.Y)
            psq_l       f7, 40(%3),  0,  0      # f7  = (pPMtxAnim[2][2], pPMtxAnim[2][3])
            ps_madd     f5,     f5, f1, f4      # f5  = (pPMtxAnim[1][2], pPMtxAnim[1][3]) * (Trans.Z, 1.0f)
            psq_st      f3, 12(%1),  1,  0      # Store (pMA[0][3], ----)
            ps_sum0     f5,     f5, f4, f5      # f5  = (f5(ps0) + f5(ps1)), ----)
            ps_mul      f6,     f6, f0          # f6  = (pPMtxAnim[2][0], pPMtxAnim[2][1]) * (Trans.X, Trans.Y)
            psq_st      f5, 28(%1),  1,  0      # Store (pMA[1][3], ----)
            ps_madd     f7,     f7, f1, f6      # f7  = (pPMtxAnim[2][2], pPMtxAnim[2][3]) * (Trans.Z, 1.0f)
            ps_sum0     f7,     f7, f6, f7      # f7  = (f7(ps0) + f7(ps1)), ----)
            psq_st      f7, 44(%1),  1,  0      # Store (pMA[2][3], ----)
        "
        :
        : "b" (pM), "b" (pMA), "b" (pParentMtx), "b" (pParentMtxAnim), "b" (&Trans)
        : "fr0", "fr1", "fr2", "fr3", "fr4", "fr5", "fr6", "fr7"
        );
    }

    //--- Compute final matrices
    for( i = 0; i < NBones; i++ )
    {
        register GCNrmMtx* pNL2W;
        register GCPosMtx* pL2W;
        register GCPosMtx* pL2WAnim;
        register GCPosMtx* pNL2WAnim;

        pL2W      = &pMtxData->L2V[i];
        pNL2W     = &pMtxData->NL2V[i];
        pL2WAnim  = &pMtxData->LAnim[i];
        pNL2WAnim = &pMtxData->NLAnim[i];

        //--- add in the bone scales
        if( pBoneScales != NULL )
        {
            //pL2W[0][0] *= pBoneScales[i].X;
            //pL2W[0][1] *= pBoneScales[i].Y;
            //pL2W[0][2] *= pBoneScales[i].Z;
            //pL2W[1][0] *= pBoneScales[i].X;
            //pL2W[1][1] *= pBoneScales[i].Y;
            //pL2W[1][2] *= pBoneScales[i].Z;
            //pL2W[2][0] *= pBoneScales[i].X;
            //pL2W[2][1] *= pBoneScales[i].Y;
            //pL2W[2][2] *= pBoneScales[i].Z;
            //pL2WAnim[0][0] *= pBoneScales[i].X;
            //pL2WAnim[0][1] *= pBoneScales[i].Y;
            //pL2WAnim[0][2] *= pBoneScales[i].Z;
            //pL2WAnim[1][0] *= pBoneScales[i].X;
            //pL2WAnim[1][1] *= pBoneScales[i].Y;
            //pL2WAnim[1][2] *= pBoneScales[i].Z;
            //pL2WAnim[2][0] *= pBoneScales[i].X;
            //pL2WAnim[2][1] *= pBoneScales[i].Y;
            //pL2WAnim[2][2] *= pBoneScales[i].Z;

            asm __volatile__
            ("
                psq_l       f0,  0(%2),  0, 0       # f0  = (pBoneScales[i].X, pBoneScales[i].Y)
                psq_l       f2,  0(%0),  0, 0       # f2  = (pL2W[0][0], pL2W[0][1])
                psq_l       f1,  8(%2),  1, 0       # f1  = (pBoneScales[i].Z, 1.0f)
                psq_l       f3,  8(%0),  1, 0       # f3  = (pL2W[0][2], 1.0f)
                ps_mul      f2,     f2, f0          # f2  = (pL2W[0][0], pL2W[0][1]) * (Scale.X, Scale.Y)
                psq_l       f4, 16(%0),  0, 0       # f4  = (pL2W[1][0], pL2W[1][1])
                ps_muls0    f3,     f3, f1          # f3  = (pL2W[0][2], 1.0f) * Scale.Z
                psq_l       f5, 24(%0),  1, 0       # f5  = (pL2W[1][2], 1.0f)
                ps_mul      f4,     f4, f0          # f4  = (pL2W[1][0], pL2W[1][1]) * (Scale.X, Scale.Y)
                psq_l       f6, 32(%0),  0, 0       # f6  = (pL2W[2][0], pL2W[2][1])
                ps_muls0    f5,     f5, f1          # f5  = (pL2W[1][2], 1.0f) * Scale.Z
                psq_l       f7, 40(%0),  1, 0       # f7  = (pL2W[2][2], 1.0f)
                ps_mul      f6,     f6, f0          # f6  = (pL2W[2][0], pL2W[2][1]) * (Scale.X, Scale.Y)
                psq_st      f2,  0(%0),  0, 0       # Store (pL2W[0][0], pL2W[0][1])
                ps_muls0    f7,     f7, f1          # f7  = (pL2W[2][2], 1.0f) * Scale.Z
                psq_st      f3,  8(%0),  1, 0       # Store (pL2W[0][2], ----)
                psq_l       f2,  0(%1),  0, 0       # f2  = (pL2WAnim[0][0], pL2WAnim[0][1])
                psq_st      f4, 16(%0),  0, 0       # Store (pL2W[1][0], pL2W[1][1])
                psq_l       f3,  8(%1),  1, 0       # f3  = (pL2WAnim[0][2], 1.0f)
                ps_mul      f2,     f2, f0          # f2  = (pL2WAnim[0][0], pL2WAnim[0][1]) * (Scale.X, Scale.Y)
                psq_st      f5, 24(%0),  1, 0       # Store (pL2W[1][2], ----)
                ps_muls0    f3,     f3, f1          # f3  = (pL2WAnim[0][2], 1.0f) * Scale.Z
                psq_st      f6, 32(%0),  0, 0       # Store (pL2W[2][0], pL2W[2][1])
                psq_l       f4, 16(%1),  0, 0       # f4  = (pL2WAnim[1][0], pL2WAnim[1][1])
                psq_st      f7, 40(%0),  1, 0       # Store (pL2W[2][2], ----)
                ps_mul      f4,     f4, f0          # f4  = (pL2WAnim[1][0], pL2WAnim[1][1]) * (Scale.X, Scale.Y)
                psq_l       f5, 24(%1),  1, 0       # f5  = (pL2WAnim[1][2], 1.0f)
                psq_st      f2,  0(%1),  0, 0       # Store (pL2WAnim[0][0], pL2WAnim[0][1])
                psq_l       f6, 32(%1),  0, 0       # f6  = (pL2WAnim[2][0], pL2WAnim[2][1])
                ps_muls0    f5,     f5, f1          # f5  = (pL2WAnim[1][2], 1.0f) * Scale.Z
                psq_st      f3,  8(%1),  1, 0       # Store (pL2WAnim[0][2], ----)
                psq_l       f7, 40(%1),  1, 0       # f7  = (pL2WAnim[2][2], 1.0f)
                ps_mul      f6,     f6, f0          # f6  = (pL2WAnim[2][0], pL2WAnim[2][1]) * (Scale.X, Scale.Y)
                psq_st      f4, 16(%1),  0, 0       # Store (pL2WAnim[1][0], pL2WAnim[1][1])
                ps_muls0    f7,     f7, f1          # f7  = (pL2WAnim[2][2], 1.0f) * Scale.Z
                psq_st      f5, 24(%1),  1, 0       # Store (pL2WAnim[1][2], ----)
                psq_st      f6, 32(%1),  0, 0       # Store (pL2WAnim[2][0], pL2WAnim[2][1])
                psq_st      f7, 40(%1),  1, 0       # Store (pL2WAnim[2][2], ----)
            "
            :
            : "b" (pL2W), "b" (pL2WAnim), "b" (&pBoneScales[i])
            : "fr0", "fr1", "fr2", "fr3", "fr4", "fr5", "fr6", "fr7"
            );
        }

        //--- Save out the Normal L2W(rotation part) for GX loading
        //    (GXLoadNrmMtxIndx3x3 needs these) IMPORTANT NOTE: pNL2W is a 3x3 matrix
        //pNL2W[0][0] = pL2W[0][0];
        //pNL2W[0][1] = pL2W[0][1];
        //pNL2W[0][2] = pL2W[0][2];
        //pNL2W[1][0] = pL2W[1][0];
        //pNL2W[1][1] = pL2W[1][1];
        //pNL2W[1][2] = pL2W[1][2];
        //pNL2W[2][0] = pL2W[2][0];
        //pNL2W[2][1] = pL2W[2][1];
        //pNL2W[2][2] = pL2W[2][2];

        asm __volatile__
        ("
            psq_l       f0,  0(%1), 0, 0        # f0  = (pL2W[0][0], pL2W[0][1])
            psq_l       f1,  8(%1), 1, 0        # f1  = (pL2W[0][2], 1.0f)
            psq_st      f0,  0(%0), 0, 0        # Store (pNL2W[0][0], pNL2W[0][1])
            psq_l       f2, 16(%1), 0, 0        # f2  = (pL2W[1][0], pL2W[1][1])
            psq_st      f1,  8(%0), 1, 0        # Store (pNL2W[0][2], ----)
            psq_l       f3, 24(%1), 1, 0        # f3  = (pL2W[1][2], 1.0f)
            psq_st      f2, 12(%0), 0, 0        # Store (pNL2W[1][0], pNL2W[1][1])
            psq_l       f4, 32(%1), 0, 0        # f4  = (pL2W[2][0], pL2W[2][1])
            psq_st      f3, 20(%0), 1, 0        # Store (pNL2W[1][2], ----)
            psq_l       f5, 40(%1), 1, 0        # f5  = (pL2W[2][2], 1.0f)
            psq_st      f4, 24(%0), 0, 0        # Store (pNL2W[2][0], pNL2W[2][1])
            psq_st      f5, 32(%0), 1, 0        # Store (pNL2W[2][2], ----)
        "
        :
        : "b" (pNL2W), "b" (pL2W)
        : "fr0", "fr1", "fr2", "fr3", "fr4", "fr5"
        );


        //--- Save out the Normal L2WAnim(rotation part) for texture effects
        //pNL2WAnim[0][0] = pL2WAnim[0][0];
        //pNL2WAnim[0][1] = pL2WAnim[0][1];
        //pNL2WAnim[0][2] = pL2WAnim[0][2];
        //pNL2WAnim[1][0] = pL2WAnim[1][0];
        //pNL2WAnim[1][1] = pL2WAnim[1][1];
        //pNL2WAnim[1][2] = pL2WAnim[1][2];
        //pNL2WAnim[2][0] = pL2WAnim[2][0];
        //pNL2WAnim[2][1] = pL2WAnim[2][1];
        //pNL2WAnim[2][2] = pL2WAnim[2][2];
        //pNL2WAnim[0][3] = 0;
        //pNL2WAnim[1][3] = 0;
        //pNL2WAnim[2][3] = 0;

        asm __volatile__
        ("
            psq_l       f6,  0(%2),  1, 0       # f6  = (0.0, 1.0)
            psq_l       f1,  8(%1),  1, 0       # f1  = (pL2WAnim[0][2], 1.0f)
            psq_l       f3, 24(%1),  1, 0       # f3  = (pL2WAnim[1][2], 1.0f)
            psq_l       f5, 40(%1),  1, 0       # f5  = (pL2WAnim[2][2], 1.0f)
            psq_l       f0,  0(%1),  0, 0       # f0  = (pL2WAnim[0][0], pL2WAnim[0][1])
            ps_sub      f1,     f1, f6          # f1 -= (0.0, 1.0)
            psq_l       f2, 16(%1),  0, 0       # f2  = (pL2WAnim[1][0], pL2WAnim[1][1])
            ps_sub      f3,     f3, f6          # f3 -= (0.0, 1.0)
            psq_st      f0,  0(%0),  0, 0       # Store (pNL2WAnim[0][0], pNL2WAnim[0][1])
            psq_l       f4, 32(%1),  0, 0       # f4  = (pL2WAnim[2][0],  pL2WAnim[2][1])
            psq_st      f1,  8(%0),  0, 0       # Store (pNL2WAnim[0][2], pNL2WAnim[0][3])
            ps_sub      f5,     f5, f6          # f5 -= (0.0, 1.0)
            psq_st      f2, 16(%0),  0, 0       # Store (pNL2WAnim[1][0], pNL2WAnim[1][1])
            psq_st      f3, 24(%0),  0, 0       # Store (pNL2WAnim[1][2], pNL2WAnim[1][3])
            psq_st      f4, 32(%0),  0, 0       # Store (pNL2WAnim[2][0], pNL2WAnim[2][1])
            psq_st      f5, 40(%0),  0, 0       # Store (pNL2WAnim[2][2], pNL2WAnim[2][3])
        "
        :
        : "b" (pNL2WAnim), "b" (pL2WAnim), "b" (&FZERO)
        : "fr0", "fr1", "fr2", "fr3", "fr4", "fr5", "fr6"
        );


        // Translate the point by -OriginToBone to get the point
        // with respect to the bone.
        // This will give us the Local-To-World matrix
        // L2W = L2W * T( -pAnimBones[i].OriginToBone )

        if( bMirrored )
        {
            Orig2Bone.X =  pAnimBones[ pAnimBones[i].MirrorID ].OriginToBone.X;
            Orig2Bone.Y = -pAnimBones[ pAnimBones[i].MirrorID ].OriginToBone.Y;
            Orig2Bone.Z = -pAnimBones[ pAnimBones[i].MirrorID ].OriginToBone.Z;
        }
        else
        {
            Orig2Bone.X = -pAnimBones[i].OriginToBone.X;
            Orig2Bone.Y = -pAnimBones[i].OriginToBone.Y;
            Orig2Bone.Z = -pAnimBones[i].OriginToBone.Z;
        }

        ////--- Apply the Orig2Bone to the L2W matrix
        //pL2W[0][3] += pL2W[0][0] * Orig2Bone.X +
        //              pL2W[0][1] * Orig2Bone.Y +
        //              pL2W[0][2] * Orig2Bone.Z;
        //pL2W[1][3] += pL2W[1][0] * Orig2Bone.X +
        //              pL2W[1][1] * Orig2Bone.Y +
        //              pL2W[1][2] * Orig2Bone.Z;
        //pL2W[2][3] += pL2W[2][0] * Orig2Bone.X +
        //              pL2W[2][1] * Orig2Bone.Y +
        //              pL2W[2][2] * Orig2Bone.Z;
        //
        ////--- Apply the Orig2Bone to the L2WAnim matrix
        //pL2WAnim[0][3] += pL2WAnim[0][0] * Orig2Bone.X +
        //                  pL2WAnim[0][1] * Orig2Bone.Y +
        //                  pL2WAnim[0][2] * Orig2Bone.Z;
        //pL2WAnim[1][3] += pL2WAnim[1][0] * Orig2Bone.X +
        //                  pL2WAnim[1][1] * Orig2Bone.Y +
        //                  pL2WAnim[1][2] * Orig2Bone.Z;
        //pL2WAnim[2][3] += pL2WAnim[2][0] * Orig2Bone.X +
        //                  pL2WAnim[2][1] * Orig2Bone.Y +
        //                  pL2WAnim[2][2] * Orig2Bone.Z;

        asm __volatile__
        ("
            psq_l       f0,  0(%2),  0,  0      # f0  = (Orig2Bone.X, Orig2Bone.Y)
            psq_l       f2,  0(%0),  0,  0      # f2  = (pL2W[0][0], pL2W[0][1])
            psq_l       f1,  8(%2),  1,  0      # f1  = (Orig2Bone.Z, 1.0f)
            psq_l       f3,  8(%0),  0,  0      # f3  = (pL2W[0][2], pL2W[0][3])
            ps_mul      f2,     f2, f0          # f2  = (pL2W[0][0], pL2W[0][1]) * (Orig2Bone.X, Orig2Bone.Y)
            psq_l       f4, 16(%0),  0,  0      # f4  = (pL2W[1][0], pL2W[1][1])
            ps_madd     f3,     f3, f1, f2      # f3  = (pL2W[0][2], pL2W[0][3]) * (Orig2Bone.Z, 1.0f)
            psq_l       f5, 24(%0),  0,  0      # f5  = (pL2W[1][2], pL2W[1][3])
            ps_sum0     f3,     f3, f2, f3      # f3  = (f3(ps0) + f3(ps1)), ----)
            psq_l       f6, 32(%0),  0,  0      # f6  = (pL2W[2][0], pL2W[2][1])
            ps_mul      f4,     f4, f0          # f4  = (pL2W[1][0], pL2W[1][1]) * (Orig2Bone.X, Orig2Bone.Y)
            psq_l       f7, 40(%0),  0,  0      # f7  = (pL2W[2][2], pL2W[2][3])
            ps_madd     f5,     f5, f1, f4      # f5  = (pL2W[1][2], pL2W[1][3]) * (Orig2Bone.Z, 1.0f)
            psq_st      f3, 12(%0),  1,  0      # Store (pL2W[0][3], ----)
            ps_sum0     f5,     f5, f4, f5      # f5  = (f5(ps0) + f5(ps1)), ----)
            ps_mul      f6,     f6, f0          # f6  = (pL2W[2][0], pL2W[2][1]) * (Orig2Bone.X, Orig2Bone.Y)
            psq_st      f5, 28(%0),  1,  0      # Store (pL2W[1][3], ----)
            ps_madd     f7,     f7, f1, f6      # f7  = (pL2W[2][2], pL2W[2][3]) * (Orig2Bone.Z, 1.0f)
            psq_l       f2,  0(%1),  0,  0      # f2  = (pL2WAnim[0][0], pL2WAnim[0][1])
            ps_sum0     f7,     f7, f6, f7      # f7  = (f7(ps0) + f7(ps1)), ----)
            psq_l       f3,  8(%1),  0,  0      # f3  = (pL2WAnim[0][2], pL2WAnim[0][3])
            psq_st      f7, 44(%0),  1,  0      # Store (pL2W[2][3], ----)
            ps_mul      f2,     f2, f0          # f2  = (pL2WAnim[0][0], pL2WAnim[0][1]) * (Orig2Bone.X, Orig2Bone.Y)
            psq_l       f4, 16(%1),  0,  0      # f4  = (pL2WAnim[1][0], pL2WAnim[1][1])
            ps_madd     f3,     f3, f1, f2      # f3  = (pL2WAnim[0][2], pL2WAnim[0][3]) * (Orig2Bone.Z, 1.0f)
            psq_l       f5, 24(%1),  0,  0      # f5  = (pL2WAnim[1][2], pL2WAnim[1][3])
            ps_sum0     f3,     f3, f2, f3      # f3  = (f3(ps0) + f3(ps1)), ----)
            psq_l       f6, 32(%1),  0,  0      # f6  = (pL2WAnim[2][0], pL2WAnim[2][1])
            ps_mul      f4,     f4, f0          # f4  = (pL2WAnim[1][0], pL2WAnim[1][1]) * (Orig2Bone.X, Orig2Bone.Y)
            psq_l       f7, 40(%1),  0,  0      # f7  = (pL2WAnim[2][2], pL2WAnim[2][3])
            ps_madd     f5,     f5, f1, f4      # f5  = (pL2WAnim[1][2], pL2WAnim[1][3]) * (Orig2Bone.Z, 1.0f)
            psq_st      f3, 12(%1),  1,  0      # Store (pL2WAnim[0][3], ----)
            ps_sum0     f5,     f5, f4, f5      # f5  = (f5(ps0) + f5(ps1)), ----)
            ps_mul      f6,     f6, f0          # f6  = (pL2WAnim[2][0], pL2WAnim[2][1]) * (Orig2Bone.X, Orig2Bone.Y)
            psq_st      f5, 28(%1),  1,  0      # Store (pL2WAnim[1][3], ----)
            ps_madd     f7,     f7, f1, f6      # f7  = (pL2WAnim[2][2], pL2WAnim[2][3]) * (Orig2Bone.Z, 1.0f)
            ps_sum0     f7,     f7, f6, f7      # f7  = (f7(ps0) + f7(ps1)), ----)
            psq_st      f7, 44(%1),  1,  0      # Store (pL2WAnim[2][3], ----)
        "
        :
        : "b" (pL2W), "b" (pL2WAnim), "b" (&Orig2Bone)
        : "fr0", "fr1", "fr2", "fr3", "fr4", "fr5", "fr6", "fr7"
        );


        for( j = 0; j < NShadowMtx; j++ )
        {
            matrix4 Temp = pShadowMatrices[j];
            Temp.Transpose();

            PSMTXConcat( GCW2V, (MtxPtr)Temp.M, pShadowMtxData[j][i] );
            PSMTXConcat( pShadowMtxData[j][i], *pL2W, pShadowMtxData[j][i] );
        }

        //--- The L2W and NL2W matrices need the final XForm to view
        //    space, so the world-2-view transform is applied here
        //  L2W = GCW2V * L2W
        // NL2W = GCW2V * NL2W
        asm __volatile__
        ("
            psq_l       f5,  0(%2),  0,  0      # f5  = (W2V[0][0], W2V[0][1])
            psq_l      f10,  8(%0),  0,  0      # f10 = (L2W[0][2], L2W[0][3])
            psq_l       f7,  0(%0),  0,  0      # f7  = (L2W[0][0], L2W[0][1])
            psq_l      f11, 24(%0),  0,  0      # f11 = (L2W[1][2], L2W[1][3])
            psq_l       f8, 16(%0),  0,  0      # f8  = (L2W[1][0], L2W[1][1])
            ps_muls0    f2,    f10, f5          # f2  = (L2W[0][2], L2W[0][3]) * (W2V[0][0])
            psq_l      f16,  8(%1),  1,  0      # f16 = (NLW[0][2], 1.0)
            ps_muls0    f1,     f7, f5          # f1  = (L2W[0][0], L2W[0][1]) * (W2V[0][0])
            psq_l       f6,  8(%2),  0,  0      # f6  = (W2V[0][2], W2V[0][3])
            ps_madds1   f2,    f11, f5, f2      # f2 += (L2W[1][2], L2W[1][3]) * (W2V[0][1])
            psq_l       f0,  0(%3),  1,  0      # f0  = (0.0, 1.0)
            ps_muls0    f4,    f16, f5          # f4  = (NLW[0][2],    1.0   ) * (W2V[0][0])
            psq_l      f12, 40(%0),  0,  0      # f12 = (L2W[2][2], L2W[2][3])
            psq_l      f13,  0(%1),  0,  0      # f13 = (NLW[0][0], NLW[0][1])
            ps_madds1   f1,     f8, f5, f1      # f1 += (L2W[1][0], L2W[1][1]) * (W2V[0][1])
            psq_l       f9, 32(%0),  0,  0      # f9  = (L2W[2][0], L2W[2][1])
            ps_madds0   f2,    f12, f6, f2      # f2 += (L2W[2][2], L2W[2][3]) * (W2V[0][2])
            psq_l      f15, 24(%1),  0,  0      # f15 = (NLW[2][0], NLW[2][1])
            ps_muls0    f3,    f13, f5          # f3  = (NLW[0][0], NLW[0][1]) * (W2V[0][0])
            psq_l      f17, 20(%1),  1,  0      # f17 = (NLW[1][2], 1.0)
            ps_madds0   f1,     f9, f6, f1      # f1 += (L2W[2][0], L2W[2][1]) * (W2V[0][2])
            psq_l      f14, 12(%1),  0,  0      # f14 = (NLW[1][0], NLW[1][1])
            ps_madds1   f2,     f0, f6, f2      # f2 += (   0.0   ,    1.0   ) * (W2V[0][3])
            psq_l      f18, 32(%1),  1,  0      # f18 = (NLW[2][2], 1.0)
            ps_madds1   f4,    f17, f5, f4      # f4 += (NLW[1][2],    1.0   ) * (W2V[0][1])
            psq_st      f1,  0(%0),  0,  0      # store (L2W[0][0], L2W[0][1])
            ps_madds1   f3,    f14, f5, f3      # f3 += (NLW[1][0], NLW[1][1]) * (W2V[0][1])
            psq_st      f2,  8(%0),  0,  0      # store (L2W[0][2], L2W[0][3])
            ps_madds0   f4,    f18, f6, f4      # f4 += (NLW[2][2],    1.0   ) * (W2V[0][2])
            psq_l       f5, 16(%2),  0,  0      # f2  = (W2V[1][0], W2V[1][1])
            ps_madds0   f3,    f15, f6, f3      # f3 += (NLW[2][0], NLW[2][1]) * (W2V[0][2])
            psq_st      f4,  8(%1),  1,  0      # store (NLW[0][2], ---------)
            ps_muls0    f2,    f10, f5          # f2  = (L2W[0][2], L2W[0][3]) * (W2V[1][0])
            psq_l       f6, 24(%2),  0,  0      # f3  = (W2V[1][2], W2V[1][3])
            ps_muls0    f1,     f7, f5          # f1  = (L2W[0][0], L2W[0][1]) * (W2V[1][0])
            psq_st      f3,  0(%1),  0,  0      # store (NLW[0][0], NLW[0][1])
            ps_madds1   f2,    f11, f5, f2      # f2 += (L2W[1][2], L2W[1][3]) * (W2V[1][1])
            ps_muls0    f4,    f16, f5          # f4  = (NLW[0][2],    1.0   ) * (W2V[1][0])
            ps_madds1   f1,     f8, f5, f1      # f1 += (L2W[1][0], L2W[1][1]) * (W2V[1][1])
            ps_muls0    f3,    f13, f5          # f3  = (NLW[0][0], NLW[0][1]) * (W2V[1][0])
            ps_madds0   f2,    f12, f6, f2      # f2 += (L2W[2][2], L2W[2][3]) * (W2V[1][2])
            ps_madds1   f4,    f17, f5, f4      # f4 += (NLW[1][2],    1.0   ) * (W2V[1][1])
            ps_madds0   f1,     f9, f6, f1      # f1 += (L2W[2][0], L2W[2][1]) * (W2V[1][2])
            ps_madds1   f3,    f14, f5, f3      # f3 += (NLW[1][0], NLW[1][1]) * (W2V[1][1])
            ps_madds1   f2,     f0, f6, f2      # f2 += (   0.0   ,    1.0   ) * (W2V[1][3])
            ps_madds0   f4,    f18, f6, f4      # f4 += (NLW[2][2],    1.0   ) * (W2V[1][2])
            psq_st      f1, 16(%0),  0,  0      # store (L2W[1][0], L2W[1][1])
            ps_madds0   f3,    f15, f6, f3      # f3 += (NLW[2][0], NLW[2][1]) * (W2V[1][2])
            psq_st      f2, 24(%0),  0,  0      # store (L2W[1][2], L2W[1][3])
            psq_l       f5, 32(%2),  0,  0      # f4  = (W2V[2][0], W2V[2][1])
            psq_st      f4, 20(%1),  1,  0      # store (NLW[1][2], ---------)
            psq_st      f3, 12(%1),  0,  0      # store (NLW[1][0], NLW[1][1])
            psq_l       f6, 40(%2),  0,  0      # f5  = (W2V[2][2], W2V[2][3])
            ps_muls0    f2,    f10, f5          # f2  = (L2W[0][2], L2W[0][3]) * (W2V[2][0])
            ps_muls0    f1,     f7, f5          # f1  = (L2W[0][0], L2W[0][1]) * (W2V[2][0])
            ps_muls0    f4,    f16, f5          # f4  = (NLW[0][2],    1.0   ) * (W2V[2][0])
            ps_muls0    f3,    f13, f5          # f3  = (NLW[0][0], NLW[0][1]) * (W2V[2][0])
            ps_madds1   f2,    f11, f5, f2      # f2 += (L2W[1][2], L2W[1][3]) * (W2V[2][1])
            ps_madds1   f1,     f8, f5, f1      # f1 += (L2W[1][0], L2W[1][1]) * (W2V[2][1])
            ps_madds1   f4,    f17, f5, f4      # f4 += (NLW[1][2],    1.0   ) * (W2V[2][1])
            ps_madds1   f3,    f14, f5, f3      # f3 += (NLW[1][0], NLW[1][1]) * (W2V[2][1])
            ps_madds0   f2,    f12, f6, f2      # f2 += (L2W[2][2], L2W[2][3]) * (W2V[2][2])
            ps_madds0   f1,     f9, f6, f1      # f1 += (L2W[2][0], L2W[2][1]) * (W2V[2][2])
            ps_madds0   f4,    f18, f6, f4      # f4 += (NLW[2][2],    1.0   ) * (W2V[2][2])
            ps_madds0   f3,    f15, f6, f3      # f3 += (NLW[2][0], NLW[2][1]) * (W2V[2][2])
            ps_madds1   f2,     f0, f6, f2      # f2 += (   0.0   ,    1.0   ) * (W2V[2][3])
            psq_st      f1, 32(%0),  0,  0      # store (L2W[2][0], L2W[2][1])
            psq_st      f4, 32(%1),  1,  0      # store (NLW[2][2], ---------)
            psq_st      f3, 24(%1),  0,  0      # store (NLW[2][0], NLW[2][1])
            psq_st      f2, 40(%0),  0,  0      # store (L2W[2][2], L2W[2][3])
        "
        :
        : "b" (pL2W), "b" (pNL2W), "b" (&GCW2V[0][0]), "b" (&FZERO)
        : "fr0",  "fr1",  "fr2",  "fr3",  "fr4",  "fr5",  "fr6",  "fr7",  "fr8",
          "fr9",  "fr10", "fr11", "fr12", "fr13", "fr14", "fr15", "fr16", "fr17", "fr18"
        );
    }

    //--- Before using the matrices, the data must be flushed from the CPU cache
    DCFlushRange( pMtxData, sizeof(GCMtxGroup) );

    if( NShadowMtx > 0 )
    {
        DCFlushRange( pShadowMtxData, sizeof(GCPosMtxArray) * NShadowMtx );
    }
}


//==========================================================================
