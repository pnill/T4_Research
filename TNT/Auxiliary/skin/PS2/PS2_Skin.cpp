////////////////////////////////////////////////////////////////////////////
//
// PS2_Skin.cpp
//  - implementation of QSkin class
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_memory.hpp"
#include "x_bitmap.hpp"
#include "x_time.hpp"

#include "Q_Engine.hpp"
#include "Q_PS2.hpp"
#include "Q_Draw.hpp"
#include "Q_SMem.hpp"

#include "Skin.hpp"
#include "AnimPlay.h" // ####

#include "dmahelp.hpp"
#include "vifhelp.hpp"
#include "sprhelp.hpp"


////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////

typedef struct t_FlushData
{
    u32     VIF[ 12 ];
} t_FlushData;

typedef struct t_FlushShadowMatrix
{
    u32     VIF[ 4 ];
} t_FlushShadowMatrix;

////////////////////////////////////////////////////////////////////////////
// Statics
////////////////////////////////////////////////////////////////////////////

byte* QSkin::s_pSkinBuffer        = NULL;
s32   QSkin::s_NMtxGroupsInBuffer = 0;
s32   QSkin::s_MaxMtxGroups       = 0;
s32   QSkin::s_MaxSubMeshes       = 0;
s32   QSkin::s_SkinMicroCodeID    = -1;

static t_MatrixGroup* s_pMatrices  = NULL;

static lighting s_CurrentLighting;

static byte *s_pFlushDataBuffer = NULL;
static t_FlushData *s_pFlushData = NULL;
static byte *s_pFlushShadowMatrixBuffer = NULL;
static t_FlushShadowMatrix *s_pFlushShadowMatrix = NULL;

////////////////////////////////////////////////////////////////////////////
// Externs
////////////////////////////////////////////////////////////////////////////

extern u32 VUM_Skin_Code     __attribute__((section(".vudata")));
extern u32 VUM_Skin_Code_End __attribute__((section(".vudata")));


////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////

void QSkin::InitData( void )
{
    m_Name[0] = '\0';
    m_NBones = 0;
    m_pBoneNames = NULL;
    m_Flags = 0;
    m_NMeshes = 0;
    m_pMeshes = NULL;
    m_NSubMeshes = 0;
    m_pSubMeshes = NULL;
    m_NTextures = NULL;
    m_pTextureNames = NULL;
    m_pTextureArray = NULL;
    m_NCacheFrames = 0;
    m_pCacheFrames = NULL;
    m_NVertLoads = 0;
    m_pVertLoads = NULL;
    m_NMorphTargets = 0;
    m_pMorphTargets = NULL;
    m_NDeltaLocations = 0;
    m_pDeltaLocations = NULL;
    m_NDeltaValues = 0;
    m_pDeltaValues = NULL;
    m_pPacketData = NULL;
    m_pDeltaData = NULL;

    m_pRawData = NULL;

    s_CurrentLighting.Locked = FALSE;
    s_CurrentLighting.Ambient.Set( 0,0,0,0 );
    s32 i;
    for (i=0; i<MAX_DIR_LIGHTS; ++i)
    {
        s_CurrentLighting.Directional[ i ].Active = FALSE;
        s_CurrentLighting.Directional[ i ].Color.Set( 0,0,0,0 );
        s_CurrentLighting.Directional[ i ].Direction.Zero();
    }



    if (s_pFlushDataBuffer == NULL)
    {
        s_pFlushDataBuffer = new byte[ sizeof(t_FlushData) + 16 ];
        s_pFlushData = (t_FlushData *)s_pFlushDataBuffer;
        while ((u32)s_pFlushData & 0xF)
            ((byte *)s_pFlushData) ++;
        s_pFlushData->VIF[ 0 ] = SCE_VIF1_SET_STCYCL( 1, 1, 0 );
        s_pFlushData->VIF[ 1 ] = SCE_VIF1_SET_BASE(SKIN_INPUT_BUFFER_BASE, 0);
        s_pFlushData->VIF[ 2 ] = SCE_VIF1_SET_OFFSET(SKIN_INPUT_BUFFER_SIZE, 0);
        s_pFlushData->VIF[ 3 ] = SCE_VIF1_SET_MSCAL(0, 0);
        s_pFlushData->VIF[ 4 ] = SCE_VIF1_SET_STROW(0);
        *((f32 *)&s_pFlushData->VIF[ 5 ]) = 1.0f;
        *((f32 *)&s_pFlushData->VIF[ 6 ]) = 1.0f;
        *((f32 *)&s_pFlushData->VIF[ 7 ]) = 1.0f;
        *((f32 *)&s_pFlushData->VIF[ 8 ]) = 1.0f;
        VIFHELP_BuildTagMask( &s_pFlushData->VIF[ 9 ], 1, 0, 0, 0,
                                                    1, 0, 0, 0,
                                                    1, 0, 0, 0,
                                                    1, 0, 0, 0 );
        s_pFlushData->VIF[ 11 ] = 0;
    }


    if (s_pFlushShadowMatrixBuffer == NULL)
    {
        s_pFlushShadowMatrixBuffer = (byte *)new byte[ sizeof( t_FlushShadowMatrix ) + 16 ];
        s_pFlushShadowMatrix = (t_FlushShadowMatrix *)s_pFlushShadowMatrixBuffer;
        while ((u32)s_pFlushShadowMatrix & 0xF)
            ((byte *)s_pFlushShadowMatrix) ++;
        s_pFlushShadowMatrix->VIF[ 0 ] = 0;
        s_pFlushShadowMatrix->VIF[ 1 ] = 0;
        s_pFlushShadowMatrix->VIF[ 2 ] = SCE_VIF1_SET_FLUSH( 0 );
        VIFHELP_BuildTagUnpack( &s_pFlushShadowMatrix->VIF[ 3 ],
                                SKIN_SHADOW_MATRIX_BASE,
                                4,
                                VIFHELP_UNPACK_V4_32,
                                FALSE,
                                FALSE,
                                TRUE );
    }
}

//==========================================================================

void QSkin::KillData( void )
{
    if ( m_pRawData != NULL )
        x_free( m_pRawData );
}

//==========================================================================

QSkin::QSkin( void )
{
    InitData();
}

//==========================================================================

QSkin::QSkin( char* SkinFile )
{
    InitData();
    SetupFromResource( SkinFile );
}

//==========================================================================

QSkin::~QSkin( void )
{
    KillData();
}

//==========================================================================

void QSkin::SetupFromResource( char* filename )
{
    X_FILE* pFilePtr;

    //---   open the file
    pFilePtr = x_fopen( filename, "rb" );
    ASSERT( pFilePtr != NULL );

    SetupFromResource( pFilePtr );

    //---   close the file
    x_fclose( pFilePtr );
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

    t_SkinFileHeader    Hdr;

    u32     RawDataSize;
    u32     RawTexMeshSize;
    byte*   pTempRaw;

    //---   clear out any previous data
    KillData();
    InitData();

    //---   read in the file header
    x_fread( &Hdr, sizeof(t_SkinFileHeader), 1, pFilePtr );

    //---   fill out as much of the skin data as we can from the header
    x_strcpy( m_Name, Hdr.Name );
    m_Flags = Hdr.Flags;
    m_NTextures = Hdr.NTextures;
    m_NBones = Hdr.NBones;
    m_NMeshes = Hdr.NMeshes;
    m_NSubMeshes = Hdr.NSubMeshes;
    m_NCacheFrames = Hdr.NCacheFrames;
    m_NVertLoads = Hdr.NVertLoads;
    m_NMorphTargets = Hdr.NMorphTargets;
    m_NDeltaLocations = Hdr.NDeltaLocations;
    m_NDeltaValues = Hdr.NDeltaValues;
    ASSERT( m_NBones <= SKIN_MAX_MATRICES );

    //---  Find the amount of memory to allocate
    RawDataSize      = m_NTextures * 32;
    RawDataSize     += m_NBones * 32;
    RawDataSize     += m_NMeshes * sizeof(t_SkinMesh);
    RawDataSize     += m_NSubMeshes * sizeof(t_SkinSubMesh);
    RawDataSize     += m_NCacheFrames * sizeof(t_SkinCacheFrame);
    RawDataSize     += m_NVertLoads * sizeof(t_SkinVertLoad);
    RawDataSize     += m_NMorphTargets * sizeof(t_SkinMorphTarget);
    RawDataSize     += m_NDeltaLocations * sizeof(t_SkinDeltaLoc);
    RawDataSize     += m_NDeltaValues * sizeof(t_SkinDeltaValue);
    RawTexMeshSize   = RawDataSize;
    RawDataSize     += Hdr.PacketSize + 16; //Add 16 for byte-alignment on packets
    RawDataSize     += Hdr.DeltaDataSize;

    //---  Allocate memory for all the data
    m_pRawData = (byte*)x_malloc( RawDataSize );
    ASSERT( m_pRawData );

    //---  Assign real data pointers into raw data buffer
    pTempRaw = m_pRawData;

    m_pTextureNames = (char*)pTempRaw;
    pTempRaw += m_NTextures * 32;

    m_pBoneNames = (char*)pTempRaw;
    pTempRaw += m_NBones * 32;

    m_pMeshes = (t_SkinMesh*)pTempRaw;
    pTempRaw += m_NMeshes * sizeof(t_SkinMesh);

    m_pSubMeshes = (t_SkinSubMesh*)pTempRaw;
    pTempRaw += m_NSubMeshes * sizeof(t_SkinSubMesh);

    m_pCacheFrames = (t_SkinCacheFrame*)pTempRaw;
    pTempRaw += m_NCacheFrames * sizeof(t_SkinCacheFrame);

    m_pVertLoads = (t_SkinVertLoad*)pTempRaw;
    pTempRaw += m_NVertLoads * sizeof(t_SkinVertLoad);

    m_pMorphTargets = (t_SkinMorphTarget*)pTempRaw;
    pTempRaw += m_NMorphTargets * sizeof(t_SkinMorphTarget);

    m_pDeltaLocations = (t_SkinDeltaLoc*)pTempRaw;
    pTempRaw += m_NDeltaLocations * sizeof(t_SkinDeltaLoc);

    m_pDeltaValues = (t_SkinDeltaValue*)pTempRaw;
    pTempRaw += m_NDeltaValues * sizeof(t_SkinDeltaValue);

    //---  Align packet data on 16 byte boundary
    if( ((u32)pTempRaw & 0x0F) != 0 )
        pTempRaw += 0x10 - ((u32)pTempRaw & 0x0F);

    m_pPacketData = (byte*)pTempRaw;
    pTempRaw += Hdr.PacketSize;

    m_pDeltaData = (byte*)pTempRaw;
    pTempRaw += Hdr.DeltaDataSize;

    //---  Read in the data
    x_fread( m_pRawData, sizeof(byte), RawTexMeshSize, pFilePtr );
    x_fread( m_pPacketData, sizeof(byte), Hdr.PacketSize + Hdr.DeltaDataSize, pFilePtr );
}

//==========================================================================

void QSkin::SetupFromResource( byte* pByteStream )
{
    ////////////////////////////////////////////////////////////////////////
    // SPECIAL NOTE ABOUT LOADING:
    //      ANY CHANGES TO THE DATA FORMAT WILL ALSO AFFECT THE OTHER
    //      OVERLOADED SetupFromResource FUNCTIONS. TAKE THAT INTO ACCOUNT!
    ////////////////////////////////////////////////////////////////////////

    t_SkinFileHeader    Hdr;

    u32     RawDataSize;
    u32     RawTexMeshSize;
    byte*   pTempRaw;

    //---   clear out any previous data
    KillData();
    InitData();

    //---   read in the file header
    x_memcpy( &Hdr, pByteStream, sizeof(t_SkinFileHeader) );
    pByteStream += sizeof(t_SkinFileHeader);

    //---   fill out as much of the skin data as we can from the header
    x_strcpy( m_Name, Hdr.Name );
    m_Flags = Hdr.Flags;
    m_NTextures = Hdr.NTextures;
    m_NBones = Hdr.NBones;
    m_NMeshes = Hdr.NMeshes;
    m_NSubMeshes = Hdr.NSubMeshes;
    m_NCacheFrames = Hdr.NCacheFrames;
    m_NVertLoads = Hdr.NVertLoads;
    m_NMorphTargets = Hdr.NMorphTargets;
    m_NDeltaLocations = Hdr.NDeltaLocations;
    m_NDeltaValues = Hdr.NDeltaValues;
    ASSERT( m_NBones <= SKIN_MAX_MATRICES );

    //---  Find the amount of memory to allocate
    RawDataSize      = m_NTextures * 32;
    RawDataSize     += m_NBones * 32;
    RawDataSize     += m_NMeshes * sizeof(t_SkinMesh);
    RawDataSize     += m_NSubMeshes * sizeof(t_SkinSubMesh);
    RawDataSize     += m_NCacheFrames * sizeof(t_SkinCacheFrame);
    RawDataSize     += m_NVertLoads * sizeof(t_SkinVertLoad);
    RawDataSize     += m_NMorphTargets * sizeof(t_SkinMorphTarget);
    RawDataSize     += m_NDeltaLocations * sizeof(t_SkinDeltaLoc);
    RawDataSize     += m_NDeltaValues * sizeof(t_SkinDeltaValue);
    RawTexMeshSize   = RawDataSize;
    RawDataSize     += Hdr.PacketSize + 16; //Add 16 for byte-alignment on packets
    RawDataSize     += Hdr.DeltaDataSize;

    //---  Allocate memory for all the data
    m_pRawData = (byte*)x_malloc( RawDataSize );
    ASSERT( m_pRawData );

    //---  Assign real data pointers into raw data buffer
    pTempRaw = m_pRawData;

    m_pTextureNames = (char*)pTempRaw;
    pTempRaw += m_NTextures * 32;

    m_pBoneNames = (char*)pTempRaw;
    pTempRaw += m_NBones * 32;

    m_pMeshes = (t_SkinMesh*)pTempRaw;
    pTempRaw += m_NMeshes * sizeof(t_SkinMesh);

    m_pSubMeshes = (t_SkinSubMesh*)pTempRaw;
    pTempRaw += m_NSubMeshes * sizeof(t_SkinSubMesh);

    m_pCacheFrames = (t_SkinCacheFrame*)pTempRaw;
    pTempRaw += m_NCacheFrames * sizeof(t_SkinCacheFrame);

    m_pVertLoads = (t_SkinVertLoad*)pTempRaw;
    pTempRaw += m_NVertLoads * sizeof(t_SkinVertLoad);

    m_pMorphTargets = (t_SkinMorphTarget*)pTempRaw;
    pTempRaw += m_NMorphTargets * sizeof(t_SkinMorphTarget);

    m_pDeltaLocations = (t_SkinDeltaLoc*)pTempRaw;
    pTempRaw += m_NDeltaLocations * sizeof(t_SkinDeltaLoc);

    m_pDeltaValues = (t_SkinDeltaValue*)pTempRaw;
    pTempRaw += m_NDeltaValues * sizeof(t_SkinDeltaValue);

    //---  Align packet data on 16 byte boundary
    if( ((u32)pTempRaw & 0x0F) != 0 )
        pTempRaw += 0x10 - ((u32)pTempRaw & 0x0F);

    m_pPacketData = (byte*)pTempRaw;
    pTempRaw += Hdr.PacketSize;

    m_pDeltaData = (byte*)pTempRaw;
    pTempRaw += Hdr.DeltaDataSize;

    //---  Read in the data
    x_memcpy( m_pRawData, pByteStream, RawTexMeshSize );
    pByteStream += RawTexMeshSize;

    x_memcpy( m_pPacketData, pByteStream, Hdr.PacketSize + Hdr.DeltaDataSize );
    pByteStream += Hdr.PacketSize + Hdr.DeltaDataSize;
}

//==========================================================================

void QSkin::ActivateMicroCode( void )
{
    if ( s_SkinMicroCodeID == -1 )
    {
        s_SkinMicroCodeID = PS2_RegisterMicroCode( "Skin", &VUM_Skin_Code, NULL );
    }
    PS2_ActivateMicroCode( s_SkinMicroCodeID );
}

//==========================================================================

void QSkin::OpenSkinBuffer( s32 MaxNMatrixSets, s32 MaxNSubMeshes )
{
    //---   allocate space off the SMEM for the skin buffer
    s_pSkinBuffer = SMEM_BufferAlloc( sizeof(t_MatrixGroup) * MaxNMatrixSets +
                                      sizeof(s32) +
                                      sizeof(t_SubMeshGroup) * MaxNSubMeshes +
                                      16 ); // add 16 for alignment purposes

    ASSERT( s_pSkinBuffer != NULL );

    //---   because the matrices need to be 16-byte aligned, move s_pSkinBuffer around
    while ( (u32)(s_pSkinBuffer) & 0xf )
        s_pSkinBuffer++;

    s_MaxSubMeshes       = MaxNSubMeshes;
    s_MaxMtxGroups       = MaxNMatrixSets;
    s_NMtxGroupsInBuffer = 0;

    //---   set the skin buffer's sub-mesh count to zero
    *((s32*)(&s_pSkinBuffer[sizeof(t_MatrixGroup) * MaxNMatrixSets])) = 0;
}

//==========================================================================

static
void AsmM4_Mult( matrix4& rDest, matrix4& rSrc1, matrix4& rSrc2 )
{
    // VU0 Assembly version:
    asm __volatile__
    ("

        LQC2    vf08, 0x00(%1)  # load the Src1 matrix vectors into vf8..vf11
        LQC2    vf09, 0x10(%1)
        LQC2    vf10, 0x20(%1)
        LQC2    vf11, 0x30(%1)

        LQC2    vf12, 0x00(%2)  # load the src2 matrix vectors into vf12..vf15
        LQC2    vf13, 0x10(%2)
        LQC2    vf14, 0x20(%2)
        LQC2    vf15, 0x30(%2)

        VMULAx.xyzw     ACC,    vf08,   vf12x       # do the first column
        VMADDAy.xyzw    ACC,    vf09,   vf12y
        VMADDAz.xyzw    ACC,    vf10,   vf12z
        VMADDw.xyzw    vf02,    vf11,   vf12w

        VMULAx.xyzw     ACC,    vf08,   vf13x       # do the second column
        VMADDAy.xyzw    ACC,    vf09,   vf13y
        VMADDAz.xyzw    ACC,    vf10,   vf13z
        VMADDw.xyzw    vf03,    vf11,   vf13w

        VMULAx.xyzw     ACC,    vf08,   vf14x       # do the first column
        VMADDAy.xyzw    ACC,    vf09,   vf14y
        VMADDAz.xyzw    ACC,    vf10,   vf14z
        VMADDw.xyzw    vf04,    vf11,   vf14w

        VMULAx.xyzw     ACC,    vf08,   vf15x       # do the first column
        VMADDAy.xyzw    ACC,    vf09,   vf15y
        VMADDAz.xyzw    ACC,    vf10,   vf15z
        VMADDw.xyzw    vf05,    vf11,   vf15w

        SQC2    vf02, 0x00(%0)        # store the result
        SQC2    vf03, 0x10(%0)
        SQC2    vf04, 0x20(%0)
        SQC2    vf05, 0x30(%0)

    ": "+r" (&rDest) : "r" (&rSrc1) , "r" (&rSrc2) );
}


//=====================================================================================================================================
inline void BuildRotationXZY( matrix4& RotMatrix, radian3 &Rotation )
{
    //=====================================================================================================================================
    //  This function takes Pitch, Roll and Yaw rotations and 
    //  applies those rotations into a matrix in that order, ( Rotations are around X, then Z, then Y axis )
    //
    //  In matrix form, this would be A' = RyRzRx * A, where:
    //
    //  Rx = | 1      0      0      0 |
    //       | 0      CosX   -SinX  0 |
    //       | 0      SinZ   CosX   0 |
    //       | 0      0      0      1 |
    //
    //  Rz = | CosZ   -SinZ  0      0 |
    //       | SinZ   CosZ   0      0 |
    //       | 0      0      1      0 |
    //       | 0      0      0      1 |
    //
    //  Ry = | CosY   0      SinY   0 |
    //       | 0      1      0      0 |
    //       | -SinY  0      CosY   0 |
    //       | 0      0      0      1 |
    //
    //
    //  So...the Rotatation XZY rotation transformation matrix (RyRzRx) is:
    //
    //  R = |  CosYCosZ     -CosXCosYSinZ + SinXSinY    CosYSinXSinZ + CosXSinY     0 |
    //      |  SinZ         CosXCosZ                    -CosZSinX                   0 |
    //      |  -CosZSinY    CosXSinYSinZ + CosYSinX     -SinXSinYSinZ + CosXCosY    0 |
    //      |  0            0                           0                           1 |
    //
    //=====================================================================================================================================
    register f32 SinX, SinY, SinZ;
    register f32 CosX, CosY, CosZ;
    register f32 CosXCosY,SinXCosY,SinYSinZ;

    x_fastsincos( Rotation.Pitch, SinX, CosX );
    x_fastsincos( Rotation.Yaw,   SinY, CosY );
    x_fastsincos( Rotation.Roll,  SinZ, CosZ );

    CosXCosY = CosX * CosY;
    SinXCosY = SinX * CosY;
    SinYSinZ = SinY * SinZ;

    RotMatrix.Identity( );
    RotMatrix.M[0][0] = ( CosY * CosZ );
    RotMatrix.M[1][0] = (-CosXCosY * SinZ + SinX * SinY );
    RotMatrix.M[2][0] = ( SinXCosY * SinZ + CosX * SinY );
    RotMatrix.M[0][1] = ( SinZ );
    RotMatrix.M[1][1] = ( CosX * CosZ );
    RotMatrix.M[2][1] = (-CosZ * SinX );
    RotMatrix.M[0][2] = (-CosZ * SinY );
    RotMatrix.M[1][2] = ( CosX * SinYSinZ + SinXCosY );
    RotMatrix.M[2][2] = (-SinX * SinYSinZ + CosXCosY );
}


//=====================================================================================================================================
// The core matrix is basically the rotations that are required for this bone, and a translation matrix
// that attaches this bone to it's parent.
inline void BuildCoreBoneMatrix(matrix4* pBoneMatrix, radian3* pBoneRotations, vector4* pParentToBone, matrix4* pParentMatrix, matrix4* pWorldOrientation )
{
//    vector4 vTransformedPlayerToBone;

    ////////////////////////////////////////////////////////////////
    // Building the CORE matrix. (Local Matrix)
    ////////////////////////////////////////////////////////////////
    //  The bone rotations are in XZY order. That is, you are
    //  expected to pitch, roll, and yaw, in that order.
    //
    //  In matrix form, this would be A' = RyRzRx * A, where:
    //
    //  Rx = | 1      0      0      0 |
    //       | 0      CosX   -SinX  0 |
    //       | 0      SinZ   CosX   0 |
    //       | 0      0      0      1 |
    //
    //  Rz = | CosZ   -SinZ  0      0 |
    //       | SinZ   CosZ   0      0 |
    //       | 0      0      1      0 |
    //       | 0      0      0      1 |
    //
    //  Ry = | CosY   0      SinY   0 |
    //       | 0      1      0      0 |
    //       | -SinY  0      CosY   0 |
    //       | 0      0      0      1 |
    //
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
    //  T = |  CosYCosZ     -CosXCosYSinZ + SinXSinY    CosYSinXSinZ + CosXSinY     Tx |
    //      |  SinZ         CosXCosZ                    -CosZSinX                   Ty |
    //      |  -CosZSinY    CosXSinYSinZ + CosYSinX     -SinXSinYSinZ + CosXCosY    Tz |
    //      |  0            0                           0                           1  |
    //
    ////////////////////////////////////////////////////////////////
    BuildRotationXZY( *pBoneMatrix, *pBoneRotations );

    // Now add the additional rotations to the bone matrix to orient the bone properly into the world.
    pBoneMatrix->Transform( *pWorldOrientation );

// In C
/*
    // Transform the BoneToParent vector by the parent matrix.
    vTransformedPlayerToBone.X = (pParentMatrix->M[0][0] * pParentToBone->X) + (pParentMatrix->M[1][0] * pParentToBone->Y) +
                                 (pParentMatrix->M[2][0] * pParentToBone->Z) +  pParentMatrix->M[3][0];

    vTransformedPlayerToBone.Y = (pParentMatrix->M[0][1] * pParentToBone->X) + (pParentMatrix->M[1][1] * pParentToBone->Y) +
                                 (pParentMatrix->M[2][1] * pParentToBone->Z) +  pParentMatrix->M[3][1];

    vTransformedPlayerToBone.Z = (pParentMatrix->M[0][2] * pParentToBone->X) + (pParentMatrix->M[1][2] * pParentToBone->Y) +
                                 (pParentMatrix->M[2][2] * pParentToBone->Z) +  pParentMatrix->M[3][2];

    // Set those translation values into the core bone matrix.
    pBoneMatrix->M[3][0] = vTransformedPlayerToBone.X;
    pBoneMatrix->M[3][1] = vTransformedPlayerToBone.Y;
    pBoneMatrix->M[3][2] = vTransformedPlayerToBone.Z;
*/
    // Using VU0 Assembly
    asm __volatile__
    ("
        LQC2    vf12, 0x00(%2)  # load the Trans vector
        LQC2    vf08, 0x00(%1)  # load pParent col 0
        LQC2    vf09, 0x10(%1)  # load pParent col 1
        LQC2    vf10, 0x20(%1)  # load pParent col 2
        LQC2    vf11, 0x30(%1)  # load pParent col 3
        VMULAx.xyz  ACC,  vf08, vf12x   # translate
        VMADDAy.xyz ACC,  vf09, vf12y   # translate
        VMADDAz.xyz ACC,  vf10, vf12z   # translate
        VMADDw.xyz  vf02, vf11, vf00w   # translate
        SQC2    vf02, 0x30(%0)  # store result in col 3 of pBoneMatrix

    ": : "r" (&pBoneMatrix->M[0][0]) , "r" (&pParentMatrix->M[0][0]) , "r" (pParentToBone) );

    // Fill out edges of the matrix.
    pBoneMatrix->M[0][3] = 0.0f;
    pBoneMatrix->M[1][3] = 0.0f;
    pBoneMatrix->M[2][3] = 0.0f;
    pBoneMatrix->M[3][3] = 1.0f;
}



/*
//=====================================================================================================================================
void QSkin::BuildMatrices( t_MatrixGroup* pMatrixGroup,
                           radian3*       pBoneRots,
                           vector3&       WorldPos,
                           radian         WorldYaw,
                           xbool          bMirrored,
                           f32            WorldScale,
                           vector3*       pBoneScales,
                           void*          pAnimGroupPtr )
{
    matrix4 WorldOrientation;
    WorldOrientation.Identity( );
    WorldOrientation.RotateY( WorldYaw );

    BuildMatrices( pMatrixGroup,
                   pBoneRots,
                   WorldPos,
                   WorldOrientation,
                   bMirrored,
                   WorldScale,
                   pBoneScales,
                   pAnimGroupPtr );
				   
}

//=====================================================================================================================================
void QSkin::BuildMatrices( t_MatrixGroup*  pMatrixGroup,
                           radian3*        pBoneRots,
                           vector3&        WorldPos,
                           matrix4&        WorldOrientation,
                           xbool           bMirrored,
                           f32             WorldScale,
                           vector3*        pBoneScales,
                           void*           pAnimGroupPtr )
{
}
*/
//==========================================================================
void QSkin::BuildMatrices( t_MatrixGroup*		pMatrixGroup,
						   const matrix4*		pBoneMatrices,
                           const vector3*		pBoneScales,
						   xbool				bMirrored,
						   const anim_group*	pAnimGroup )
{
    ASSERT( pMatrixGroup );

    // Build the lighting info
    BuildLightingMatrix( &pMatrixGroup->LightDir, &pMatrixGroup->LightColor );

    //---   Build the bone matrices
	BuildFinalBoneMatrices(	pMatrixGroup,
							pBoneMatrices,
							pBoneScales,
							bMirrored,
							pAnimGroup );
}

//==========================================================================
void QSkin::BuildFinalBoneMatrices( t_MatrixGroup*		pMatrixGroup,
									const matrix4*		pBoneMatrices,		
									const vector3*		pBoneScales,
									xbool				bMirrored,
									const anim_group*	pAnimGroup )
{

    t_MatrixData*	pMatricesDataArray	= &pMatrixGroup->BoneMatrices[0];
    matrix4			W2S			__attribute__ ((aligned(16)));

    //-- Sanity Check
    ASSERT( ((u32)pMatricesDataArray & 0xf) == 0 );   // matrices must be 16-byte aligned
    ASSERT( pAnimGroup->NBones == m_NBones );

    //---   Get W2S
    ENG_GetW2S( W2S );

    //s32 nNumBones = pAnimGroup->NBones;

    if (pAnimGroup )
    {
        t_MatrixData*  pMatrixData  = pMatricesDataArray;
        const matrix4* pL2W         = (matrix4*)pBoneMatrices;

        for ( s32 i = 0; i < pAnimGroup->NBones; i++ )
        {
            ////////////////////////////////////////////////////////////////////
            //-- Find origin to bone vector
            ////////////////////////////////////////////////////////////////////
            vector4 O2B;
            if ( bMirrored )
            {
                O2B = pAnimGroup->Bone[pAnimGroup->Bone[i].MirrorID].OriginToBone;
                O2B.Y = -O2B.Y;
                O2B.Z = -O2B.Z;
            }
            else
            {
                O2B = -pAnimGroup->Bone[i].OriginToBone;
            }

            vector4 BoneScale = (pBoneScales) ? pBoneScales[i] : vector4(1.0f, 1.0f, 1.0f, 1.0f);

            //--- calc bones
            asm __volatile__
                ("
                    /*-- Load Params*/
                    LQC2    vf08, 0x00(%1)  # load pL2W col 0
                    LQC2    vf09, 0x10(%1)  # load pL2W col 1
                    LQC2    vf10, 0x20(%1)  # load pL2W col 2
                    LQC2    vf11, 0x30(%1)  # load pL2W col 3
                    LQC2    vf12, 0x00(%4)  # load W2S col 0
                    LQC2    vf13, 0x10(%4)  # load W2S col 1
                    LQC2    vf14, 0x20(%4)  # load W2S col 2
                    LQC2    vf15, 0x30(%4)  # load W2S col 3
                    LQC2    vf16, 0x00(%3)  # load the Bone Scales vector
                    LQC2    vf17, 0x00(%2)  # load the Origin2Bone vector

                    /*-- Apply Bone Scales to L2W */
                    /* TODO:  make conditional    */
                    VMULx.xyz vf08, vf08, vf16x
                    VMULy.xyz vf09, vf09, vf16y
                    VMULz.xyz vf10, vf10, vf16z
                    
                    /*-- Apply Origin2Bone to L2W */
                    VMULAx.xyz   ACC, vf08, vf17x   # ACC = X * col 0
                    VMADDAy.xyz  ACC, vf09, vf17y   # ACC = Y * col 1
                    VMADDAz.xyz  ACC, vf10, vf17z   # ACC = Z * col 2
                    VMADDw.xyz  vf11, vf11, vf17w   # vf11 += ACC + W * col 3

                    /*-- L2S = W2S * L2W  */
                    VMULAx.xyzw     ACC,    vf12,   vf08x       # do the first column
                    VMADDAy.xyzw    ACC,    vf13,   vf08y
                    VMADDAz.xyzw    ACC,    vf14,   vf08z
                    VMADDw.xyzw    vf02,    vf15,   vf08w
                    VMULAx.xyzw     ACC,    vf12,   vf09x       # do the second column
                    VMADDAy.xyzw    ACC,    vf13,   vf09y
                    VMADDAz.xyzw    ACC,    vf14,   vf09z
                    VMADDw.xyzw    vf03,    vf15,   vf09w
                    VMULAx.xyzw     ACC,    vf12,   vf10x       # do the third column
                    VMADDAy.xyzw    ACC,    vf13,   vf10y
                    VMADDAz.xyzw    ACC,    vf14,   vf10z
                    VMADDw.xyzw    vf04,    vf15,   vf10w
                    VMULAx.xyzw     ACC,    vf12,   vf11x       # do the fourth column
                    VMADDAy.xyzw    ACC,    vf13,   vf11y
                    VMADDAz.xyzw    ACC,    vf14,   vf11z
                    VMADDw.xyzw    vf05,    vf15,   vf11w

                    /*-- Move col3.xyz of L2W to row0.w,row1.w,row2.w of L2W*/
                    VMULx  vf12, vf00, vf11x            # vf12(0,0,0,vf11x)
                    VMULy  vf13, vf00, vf11y            # vf12(0,0,0,vf11y)
                    VMULz  vf14, vf00, vf11z            # vf12(0,0,0,vf11z)
                    VMOVE.w     vf08w,  vf12w           # vf8(vf8x,vf8y,vf8z,vf12w)
                    VMOVE.w     vf09w,  vf13w           # vf8(vf9x,vf9y,vf9z,vf13w)
                    VMOVE.w     vf10w,  vf14w           # vf8(vf10x,vf10y,vf10z,vf14w)

                    /*-- Store to SPR */
                    SQC2    vf02, 0x00(%0)  # store result in col 0 of pMatrixData->m_L2S
                    SQC2    vf03, 0x10(%0)  # store result in col 1 of pMatrixData->m_L2S
                    SQC2    vf04, 0x20(%0)  # store result in col 2 of pMatrixData->m_L2S
                    SQC2    vf05, 0x30(%0)  # store result in col 3 of pMatrixData->m_L2S
                    SQC2    vf08, 0x40(%0)  # store result in col 1 of pMatrixData->m_L2W[0]
                    SQC2    vf09, 0x50(%0)  # store result in col 2 of pMatrixData->m_L2W[1]
                    SQC2    vf10, 0x60(%0)  # store result in col 3 of pMatrixData->m_L2W[2]

                    ": "+r" (pMatrixData) : "r" (&pL2W->M[0][0]) , "r" (&O2B) , "r" (&BoneScale) , "r" (&W2S.M[0][0]) );

                        /*
            pMatrixData->L2W[0].Set( pL2W->M[0][0], pL2W->M[0][1], pL2W->M[0][2], pL2W->M[3][0] );
            pMatrixData->L2W[1].Set( pL2W->M[1][0], pL2W->M[1][1], pL2W->M[1][2], pL2W->M[3][1] );
            pMatrixData->L2W[2].Set( pL2W->M[2][0], pL2W->M[2][1], pL2W->M[2][2], pL2W->M[3][2] );
            */

#ifdef RENDER_BONE_CENTERS
            //BoneCenter[i] = pL2W->Transform( pAnimGroup->Bone[i].OriginToBone );
#endif
            //---   Move to next matrices
            ++pL2W;
            ++pMatrixData;
        }
    }
    
}

/* version3
void QSkin::BuildFinalBoneMatrices( t_MatrixGroup*		pMatrixGroup,
									const matrix4*		pBoneMatrices,		
									const vector3*		pBoneScales,
									xbool				bMirrored,
									const anim_group*	pAnimGroup )
{

    t_MatrixData*	pMatrices	= &pMatrixGroup->BoneMatrices[0];
    matrix4			W2S			__attribute__ ((aligned(16)));

    //-- Sanity Check
    ASSERT( ((u32)pMatrices & 0xf) == 0 );   // matrices must be 16-byte aligned
    ASSERT( pAnimGroup->NBones == m_NBones );

    //---   Get W2S
    ENG_GetW2S( W2S );

    s32 nNumBones = pAnimGroup->NBones;

    // SPR Memory Pointers
    static s32 s_nCurrentBonesID = 0;
    ++s_nCurrentBonesID;
    //register u32 * pVIF = (u32 *)((s_nCurrentBonesID & 0x00000001) ? 0x70002000 : 0x70000000);
    register f32 * pSPR = (f32 *)((s_nCurrentBonesID & 0x00000001) ? 0x70002010 : 0x70000010);

    /*
    // Build VIF Code On SPR
    pVIF[0] = 0;
    pVIF[1] = 0;
    pVIF[2] = 0;

    // VIF Command To Load The Bones Into VU1
    VIFHELP_BuildTagUnpack( &pVIF[3],                                   // The VIF command
                            SKIN_INPUT_BUFFER_BASE - (nNumBones * 4),   // Where to transfer the data to
                            nNumBones * 4,                              // Number of QWords to transfer
                            VIFHELP_UNPACK_V4_32,                       // The source format of the data
                            FALSE,
                            FALSE,
                            TRUE ); // The address is absolute
*

    if (pAnimGroup)
    {
		const anim_bone* pBones = pAnimGroup->Bone;

        vector4 T(0.0f, 0.0f, 0.0f, 1.0f);

        if (bMirrored == FALSE)
        {
            // Build Matrices On SPR
            for (s32 iBone = 0; iBone < nNumBones; iBone++, pSPR += 16)
            {
				const anim_bone& rBone	= pBones[iBone];
                T.X = -rBone.OriginToBone.X;
                T.Y = -rBone.OriginToBone.Y;
                T.Z = -rBone.OriginToBone.Z;

                asm __volatile__
                ("
                    LQC2            vf12,   0x00(%2)            # load the Translation vector

                    LQC2            vf08,   0x00(%1)            # load M col 0
                    LQC2            vf09,   0x10(%1)            # load M col 1
                    LQC2            vf10,   0x20(%1)            # load M col 2
                    LQC2            vf11,   0x30(%1)            # load M col 3

                    VMULAx.xyz      ACC,    vf08,   vf12x       # ACC = X * col 0
                    VMADDAy.xyz     ACC,    vf09,   vf12y       # ACC = Y * col 1
                    VMADDAz.xyz     ACC,    vf10,   vf12z       # ACC = Z * col 2
                    VMADDw.xyz      vf11,   vf11,   vf12w       # vf11 += ACC + W * col 3

                    SQC2            vf08,   0x00(%0)            # store result in col 0 of M
                    SQC2            vf09,   0x10(%0)            # store result in col 1 of M
                    SQC2            vf10,   0x20(%0)            # store result in col 2 of M
                    SQC2            vf11,   0x30(%0)            # store result in col 3 of M
                
                "
                : "+r" (pSPR)
                :  "r" (&pBoneMatrices[iBone])
                ,  "r" (&T)
                );
            }
        }
        else    // Mirrored Version
        {
            // Build Matrices On SPR
            for (s32 iBone = 0; iBone < nNumBones; iBone++, pSPR += 16)
            {
				const anim_bone& rBone	= pBones[pBones[iBone].MirrorID];
                T.X = rBone.OriginToBone.X;
                T.Y = -rBone.OriginToBone.Y;
                T.Z = -rBone.OriginToBone.Z;

                asm __volatile__
                ("
                    LQC2            vf12,   0x00(%2)            # load the Translation vector

                    LQC2            vf08,   0x00(%1)            # load M col 0
                    LQC2            vf09,   0x10(%1)            # load M col 1
                    LQC2            vf10,   0x20(%1)            # load M col 2
                    LQC2            vf11,   0x30(%1)            # load M col 3

                    VMULAx.xyz      ACC,    vf08,   vf12x       # ACC = X * col 0
                    VMADDAy.xyz     ACC,    vf09,   vf12y       # ACC = Y * col 1
                    VMADDAz.xyz     ACC,    vf10,   vf12z       # ACC = Z * col 2
                    VMADDw.xyz      vf11,   vf11,   vf12w       # vf11 += ACC + W * col 3

                    SQC2            vf08,   0x00(%0)            # store result in col 0 of M
                    SQC2            vf09,   0x10(%0)            # store result in col 1 of M
                    SQC2            vf10,   0x20(%0)            # store result in col 2 of M
                    SQC2            vf11,   0x30(%0)            # store result in col 3 of M
                "
                : "+r" (pSPR)
                :  "r" (&pBoneMatrices[iBone])
                ,  "r" (&T)
                );
            }
        }
    }
    else
    {
        // Build Matrices On SPR
        for (s32 iBone = 0; iBone < nNumBones; iBone++)
        {
            *pSPR++ = 1.0f;
            *pSPR++ = 0.0f;
            *pSPR++ = 0.0f;
            *pSPR++ = 0.0f;

            *pSPR++ = 0.0f;
            *pSPR++ = 1.0f;
            *pSPR++ = 0.0f;
            *pSPR++ = 0.0f;

            *pSPR++ = 0.0f;
            *pSPR++ = 0.0f;
            *pSPR++ = 1.0f;
            *pSPR++ = 0.0f;

            *pSPR++ = 0.0f;
            *pSPR++ = 0.0f;
            *pSPR++ = 0.0f;
            *pSPR++ = 1.0f;
        }
    }

    // Make Sure Any Previous 'From SPR' DMA Transfer Is Done
    if (s_nCurrentBonesID)
    {
        DMAHELP_Wait();
    }
    
    // Set Up The DMA To Transfer From SPR To Main MEM
    *D8_QWC  = (nNumBones << 2);
    *D8_SADR = (u32)pSPR;
    *D8_MADR = (u32)pBoneMatrices;

    // Setup Our Status Flags Before The Send
    *D_PCR  |= D_PCR_CPC8_M;
    *D_STAT = D_STAT_CIS8_M;

    // This Starts The DMA Transfer
    *D8_CHCR = 0x100;
    

    //SPRHELP_DmaFrom( (void*)pBoneMatrices, nNumBones*sizeof(matrix4), (void*)((s_nCurrentBonesID & 0x00000001) ? 0x70002010 : 0x70000010) );


////  Phase 2

    const matrix4*	pL2W = pBoneMatrices;
    for ( s32 i = 0; i < pAnimGroup->NBones; i++ )
    {

        t_MatrixData* pMatrixData = &pMatrices[i];
        
        // VU0 Asm Version:
        AsmM4_Mult( pMatrixData->L2S, W2S, (matrix4&)*pL2W );

        //---   Copy the L2W for lighting
        //      This deserves some explanation, 'cause I'm doing this in a weird manner.
        //      The L2W is used for lighting and for the shadows. For the lighting, you
        //      want the L2W without translation, and for the shadows, you want the L2W
        //      with translation. In order to save on memory space, I'm simply throwing
        //      away the last column of the matrix (translation), then moving the
        //      translation into the last row of the matrix (which is usually zero). The
        //      last row will be ignored when doing the lighting calculations, and the
        //      last row will be transposed into another column when doing shadows. All
        //      of this gets done in the VU1 microcode, but we need to be set up properly
        //      for that. Thus, the weird setup here...
        pMatrixData->L2W[0].Set( pL2W->M[0][0], pL2W->M[0][1], pL2W->M[0][2], pL2W->M[3][0] );
        pMatrixData->L2W[1].Set( pL2W->M[1][0], pL2W->M[1][1], pL2W->M[1][2], pL2W->M[3][1] );
        pMatrixData->L2W[2].Set( pL2W->M[2][0], pL2W->M[2][1], pL2W->M[2][2], pL2W->M[3][2] );

#ifdef RENDER_BONE_CENTERS
        BoneCenter[i] = pL2W->Transform( pAnimGroup->Bone[i].OriginToBone );
#endif
        //---   Move to next matrices
        pL2W++;
    }

}
*/
/* version 2

void QSkin::BuildFinalBoneMatrices( t_MatrixGroup*		pMatrixGroup,
									const matrix4*		pBoneMatrices,		
									const vector3*		pBoneScales,
									xbool				bMirrored,
									const anim_group*	pAnimGroup )
{
    matrix4			W2S			__attribute__ ((aligned(16)));
    t_MatrixData*	pMatrices	= &pMatrixGroup->BoneMatrices[0];
    ASSERT( ((u32)pMatrices & 0xf) == 0 );   // matrices must be 16-byte aligned

    ASSERT( pAnimGroup->NBones == m_NBones );

    //---   Get W2S
    ENG_GetW2S( W2S );

    //---   Compute matrices
    {
        s32             i;
        const matrix4*	pL2W;
        t_MatrixData*   pMatrixData;

        ////////////////////////////////////////////////////////////////////
        // TIME FOR SOME MORE NOTES:
        //
        //  pBoneMatrices currently contains the transforms for each bone, assuming
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
        ////////////////////////////////////////////////////////////////////
        pL2W = pBoneMatrices;
        for ( i = 0; i < pAnimGroup->NBones; i++ )
        {
            pMatrixData = &pMatrices[i];

            //---   add in the bone scales
            if ( pBoneScales )
            {
                vector4 BoneScale = pBoneScales[i];
                // CPU Version:
//                pL2W->M[0][0] *= m_pBoneScales[i].X;
//                pL2W->M[0][1] *= m_pBoneScales[i].X;
//                pL2W->M[0][2] *= m_pBoneScales[i].X;
//                pL2W->M[1][0] *= m_pBoneScales[i].Y;
//                pL2W->M[1][1] *= m_pBoneScales[i].Y;
//                pL2W->M[1][2] *= m_pBoneScales[i].Y;
//                pL2W->M[2][0] *= m_pBoneScales[i].Z;
//                pL2W->M[2][1] *= m_pBoneScales[i].Z;
//                pL2W->M[2][2] *= m_pBoneScales[i].Z;
                // VU Asm Version:
                asm __volatile__
                ("
                    LQC2    vf12, 0x00(%1)  # load the Bone Scales vector
                    LQC2    vf08, 0x00(%0)  # load pL2W col 0
                    LQC2    vf09, 0x10(%0)  # load pL2W col 1
                    LQC2    vf10, 0x20(%0)  # load pL2W col 2
                    VMULx.xyz vf08, vf08, vf12x
                    VMULy.xyz vf09, vf09, vf12y
                    VMULz.xyz vf10, vf10, vf12z
                    SQC2    vf08, 0x00(%0)  # store result in col 0 of pL2W
                    SQC2    vf09, 0x10(%0)  # store result in col 1 of pL2W
                    SQC2    vf10, 0x20(%0)  # store result in col 2 of pL2W

                ": "+r" (&pL2W->M[0][0]) : "r" (&BoneScale) );
            }

            //---   Translate the point by -OriginToBone to get the point
            //      with respect to the bone.
            //      This will give us the Local-To-World matrix

            //---   The easy way:
            //      MT.Identity();
            //      MT.Translate( -pAnimGroup->Bone[i].OriginToBone );
            //      (*pL2W) = (*pL2W) * MT

            //---   The more efficient way:
            //      Just check out the math, and you'll see that since we
            //      have a very special case where the last row of the
            //      matrix is 0,0,0,1 we can take shortcuts.

            vector4 O2B;
            if ( bMirrored )
            {
                O2B = pAnimGroup->Bone[pAnimGroup->Bone[i].MirrorID].OriginToBone;
                O2B.Y = -O2B.Y;
                O2B.Z = -O2B.Z;
            }
            else
            {
                O2B = -pAnimGroup->Bone[i].OriginToBone;
            }
            // CPU Version:
//            pL2W->M[3][0] += O2B.X * pL2W->M[0][0] +
//                             O2B.Y * pL2W->M[1][0] +
//                             O2B.Z * pL2W->M[2][0];
//            pL2W->M[3][1] += O2B.X * pL2W->M[0][1] +
//                             O2B.Y * pL2W->M[1][1] +
//                             O2B.Z * pL2W->M[2][1];
//            pL2W->M[3][2] += O2B.X * pL2W->M[0][2] +
//                             O2B.Y * pL2W->M[1][2] +
//                             O2B.Z * pL2W->M[2][2];
            // VU0 Asm Version:
            asm __volatile__
            ("
                LQC2    vf12, 0x00(%1)  # load the OriginToBone vector
                LQC2    vf08, 0x00(%0)  # load pL2W col 0
                LQC2    vf09, 0x10(%0)  # load pL2W col 1
                LQC2    vf10, 0x20(%0)  # load pL2W col 2
                LQC2    vf11, 0x30(%0)  # load pL2W col 3
                VMULAx.xyz   ACC, vf08, vf12x   # ACC = X * col 0
                VMADDAy.xyz  ACC, vf09, vf12y   # ACC = Y * col 1
                VMADDAz.xyz  ACC, vf10, vf12z   # ACC = Z * col 2
                VMADDw.xyz  vf11, vf11, vf12w   # vf11 += ACC + W * col 3
                SQC2    vf11, 0x30(%0)  # store result in col 3 of pL2W

            ": "+r" (&pL2W->M[0][0]) : "r" (&O2B) );

            //---   Concatenate the W2S to get the Local-To-Screen matrix

            // CPU Version:
            //pMatrixData->L2S = W2S * (*pL2W);
            
            // VU0 Asm Version:
            AsmM4_Mult( pMatrixData->L2S, W2S, (matrix4&)*pL2W );

            //---   Copy the L2W for lighting
            //      This deserves some explanation, 'cause I'm doing this in a weird manner.
            //      The L2W is used for lighting and for the shadows. For the lighting, you
            //      want the L2W without translation, and for the shadows, you want the L2W
            //      with translation. In order to save on memory space, I'm simply throwing
            //      away the last column of the matrix (translation), then moving the
            //      translation into the last row of the matrix (which is usually zero). The
            //      last row will be ignored when doing the lighting calculations, and the
            //      last row will be transposed into another column when doing shadows. All
            //      of this gets done in the VU1 microcode, but we need to be set up properly
            //      for that. Thus, the weird setup here...
            pMatrixData->L2W[0].Set( pL2W->M[0][0], pL2W->M[0][1], pL2W->M[0][2], pL2W->M[3][0] );
            pMatrixData->L2W[1].Set( pL2W->M[1][0], pL2W->M[1][1], pL2W->M[1][2], pL2W->M[3][1] );
            pMatrixData->L2W[2].Set( pL2W->M[2][0], pL2W->M[2][1], pL2W->M[2][2], pL2W->M[3][2] );

#ifdef RENDER_BONE_CENTERS
            BoneCenter[i] = pL2W->Transform( pAnimGroup->Bone[i].OriginToBone );
#endif
            //---   Move to next matrices
            pL2W++;
        }
    }
}

/* version 1
//=====================================================================================================================================
void QSkin::BuildMatrices( t_MatrixGroup*  pMatrixGroup,
                           radian3*        pBoneRots,
                           vector3&        WorldPos,
                           matrix4&        WorldOrientation,
                           xbool           bMirrored,
                           f32             WorldScale,
                           vector3*        pBoneScales,
                           void*           pAnimGroupPtr )
{
    matrix4*      pParentMatrix;
    t_MatrixData* pMatrices;
    radian3       Rot;
    vector4       Trans                                     __attribute__ ((aligned(16)));
    matrix4       WorldTranslation                          __attribute__ ((aligned(16)));
    matrix4       TempM[32]                                 __attribute__ ((aligned(16)));
    matrix4       W2S                                       __attribute__ ((aligned(16)));
    s32           BoneID;
    anim_group*   pAnimGroup;

    pAnimGroup = (anim_group*)pAnimGroupPtr;

    // Test to make sure all is well.
    ASSERT( pMatrixGroup );
    ASSERT( pBoneRots );
    ASSERT( pAnimGroup->NBones == m_NBones );

    pMatrices = &pMatrixGroup->BoneMatrices[0];
    ASSERT( ((u32)pMatrices & 0xf) == 0 );   // matrices must be 16-byte aligned

    // Build the lighting info
    BuildLightingMatrix( &pMatrixGroup->LightDir, &pMatrixGroup->LightColor );

    // Build the world translation matrix which identifies where in world space the skin will be located.
    WorldTranslation.Identity();
    WorldTranslation.Translate( WorldPos );

    // Loop through all bones (The parent bones will AND must be processed prior to the children bones)
    // Determine the rotation and translation values for the bones.
    for( BoneID = 0; BoneID < pAnimGroup->NBones; BoneID++ )
    {
        //////////////////////////////////////////////////////////////////
        // ABOUT ANIMATION MIRRORING:                                               Y
        //                                                                          |
        //  The skeleton is assumed to be aligned facing positive Z axis,     X     |     Z
        //  with the left side on positive X axis, which means mirroring       \    |    /
        //  for left-right is done using the YZ plane.                           \  |  /
        //                                                                         \|/
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
        ////////////////////////////////////////////////////////////////////
        // Decide on rotation and translation values for bone
        if( bMirrored )
        {
            Rot.Pitch =  pBoneRots[pAnimGroup->Bone[BoneID].MirrorID].Pitch;
            Rot.Yaw   = -pBoneRots[pAnimGroup->Bone[BoneID].MirrorID].Yaw;
            Rot.Roll  = -pBoneRots[pAnimGroup->Bone[BoneID].MirrorID].Roll;
            Trans     =  pAnimGroup->Bone[pAnimGroup->Bone[BoneID].MirrorID].ParentToBone;

            Trans.X   = -Trans.X;
        }
        else
        {
            Rot       = pBoneRots[BoneID];
            Trans     = pAnimGroup->Bone[BoneID].ParentToBone;
        }

        // Translation of the bones should be scaled by the global scaling value.
        Trans.X *= WorldScale;
        Trans.Y *= WorldScale;
        Trans.Z *= WorldScale;

        // Get the this bones parent matrix.  If there isn't a parent, then this matrix is for the 
        // ROOT node.  So give it the plain world translation.
        if ( pAnimGroup->Bone[BoneID].ParentID != -1 )
        {
            ASSERT( pAnimGroup->Bone[BoneID].ParentID < BoneID );
            pParentMatrix = &TempM[pAnimGroup->Bone[BoneID].ParentID];
        }
        else
        {
            pParentMatrix = &WorldTranslation;
        }

        // Build the core matrix for this bone.
        BuildCoreBoneMatrix( &TempM[BoneID], &Rot, &Trans, pParentMatrix, &WorldOrientation );
    }

    // Get W2S
    ENG_GetW2S( W2S );

    // Complete each bone matrix
    {
        s32             i;
        matrix4*        pL2W;
        t_MatrixData*   pMatrixData;

        ////////////////////////////////////////////////////////////////////
        // TIME FOR SOME MORE NOTES:
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
        ////////////////////////////////////////////////////////////////////
        pL2W = TempM;
        for ( i = 0; i < pAnimGroup->NBones; i++ )
        {
            pMatrixData = &pMatrices[i];

            //---   add in the bone scales
            if ( pBoneScales )
            {
                vector4 BoneScale = pBoneScales[i];
                // CPU Version:
//                pL2W->M[0][0] *= m_pBoneScales[i].X;
//                pL2W->M[0][1] *= m_pBoneScales[i].X;
//                pL2W->M[0][2] *= m_pBoneScales[i].X;
//                pL2W->M[1][0] *= m_pBoneScales[i].Y;
//                pL2W->M[1][1] *= m_pBoneScales[i].Y;
//                pL2W->M[1][2] *= m_pBoneScales[i].Y;
//                pL2W->M[2][0] *= m_pBoneScales[i].Z;
//                pL2W->M[2][1] *= m_pBoneScales[i].Z;
//                pL2W->M[2][2] *= m_pBoneScales[i].Z;
                // VU Asm Version:
                asm __volatile__
                ("
                    LQC2    vf12, 0x00(%1)  # load the Bone Scales vector
                    LQC2    vf08, 0x00(%0)  # load pL2W col 0
                    LQC2    vf09, 0x10(%0)  # load pL2W col 1
                    LQC2    vf10, 0x20(%0)  # load pL2W col 2
                    VMULx.xyz vf08, vf08, vf12x
                    VMULy.xyz vf09, vf09, vf12y
                    VMULz.xyz vf10, vf10, vf12z
                    SQC2    vf08, 0x00(%0)  # store result in col 0 of pL2W
                    SQC2    vf09, 0x10(%0)  # store result in col 1 of pL2W
                    SQC2    vf10, 0x20(%0)  # store result in col 2 of pL2W

                ": "+r" (&pL2W->M[0][0]) : "r" (&BoneScale) );
            }

            //---   Translate the point by -OriginToBone to get the point
            //      with respect to the bone.
            //      This will give us the Local-To-World matrix

            //---   The easy way:
            //      MT.Identity();
            //      MT.Translate( -pAnimGroup->Bone[i].OriginToBone );
            //      (*pL2W) = (*pL2W) * MT

            //---   The more efficient way:
            //      Just check out the math, and you'll see that since we
            //      have a very special case where the last row of the
            //      matrix is 0,0,0,1 we can take shortcuts.

            vector4 O2B;
            if( bMirrored )
            {
                O2B = pAnimGroup->Bone[pAnimGroup->Bone[i].MirrorID].OriginToBone;
                O2B.Y = -O2B.Y;
                O2B.Z = -O2B.Z;
            }
            else
            {
                O2B = -pAnimGroup->Bone[i].OriginToBone;
            }
            // CPU Version:
//            pL2W->M[3][0] += O2B.X * pL2W->M[0][0] +
//                             O2B.Y * pL2W->M[1][0] +
//                             O2B.Z * pL2W->M[2][0];
//            pL2W->M[3][1] += O2B.X * pL2W->M[0][1] +
//                             O2B.Y * pL2W->M[1][1] +
//                             O2B.Z * pL2W->M[2][1];
//            pL2W->M[3][2] += O2B.X * pL2W->M[0][2] +
//                             O2B.Y * pL2W->M[1][2] +
//                             O2B.Z * pL2W->M[2][2];
            // VU0 Asm Version:
            asm __volatile__
            ("
                LQC2    vf12, 0x00(%1)  # load the OriginToBone vector
                LQC2    vf08, 0x00(%0)  # load pL2W col 0
                LQC2    vf09, 0x10(%0)  # load pL2W col 1
                LQC2    vf10, 0x20(%0)  # load pL2W col 2
                LQC2    vf11, 0x30(%0)  # load pL2W col 3
                VMULAx.xyz   ACC, vf08, vf12x   # ACC = X * col 0
                VMADDAy.xyz  ACC, vf09, vf12y   # ACC = Y * col 1
                VMADDAz.xyz  ACC, vf10, vf12z   # ACC = Z * col 2
                VMADDw.xyz  vf11, vf11, vf12w   # vf11 += ACC + W * col 3
                SQC2    vf11, 0x30(%0)  # store result in col 3 of pL2W

            ": "+r" (&pL2W->M[0][0]) : "r" (&O2B) );

            //---   Concatenate the W2S to get the Local-To-Screen matrix

            // CPU Version:
            //pMatrixData->L2S = W2S * (*pL2W);
            
            // VU0 Asm Version:
            AsmM4_Mult( pMatrixData->L2S, W2S, *pL2W );

            //---   Copy the L2W for lighting
            //      This deserves some explanation, 'cause I'm doing this in a weird manner.
            //      The L2W is used for lighting and for the shadows. For the lighting, you
            //      want the L2W without translation, and for the shadows, you want the L2W
            //      with translation. In order to save on memory space, I'm simply throwing
            //      away the last column of the matrix (translation), then moving the
            //      translation into the last row of the matrix (which is usually zero). The
            //      last row will be ignored when doing the lighting calculations, and the
            //      last row will be transposed into another column when doing shadows. All
            //      of this gets done in the VU1 microcode, but we need to be set up properly
            //      for that. Thus, the weird setup here...
            pMatrixData->L2W[0].Set( pL2W->M[0][0], pL2W->M[0][1], pL2W->M[0][2], pL2W->M[3][0] );
            pMatrixData->L2W[1].Set( pL2W->M[1][0], pL2W->M[1][1], pL2W->M[1][2], pL2W->M[3][1] );
            pMatrixData->L2W[2].Set( pL2W->M[2][0], pL2W->M[2][1], pL2W->M[2][2], pL2W->M[3][2] );

#ifdef RENDER_BONE_CENTERS
            BoneCenter[i] = pL2W->Transform( pAnimGroup->Bone[i].OriginToBone );
#endif
            //---   Move to next matrices
            pL2W++;
        }
    }
}
*/
//==========================================================================
void QSkin::BuildLightingMatrix( matrix4* pLightDir, matrix4* pLightColor )
{
    lighting        EngLighting;

    //---   Figure out the Light Dir matrix

/***********************************************\
|                                               |
|   Build a Dirty Flag for use with Lighting?   |
|                                               |
\***********************************************/
    s32 iLightSet = 0;
    if (m_Flags & ENG_LIGHTING_SET_DARK)
        iLightSet = 1;
    else if (m_Flags & ENG_LIGHTING_SET_ALTERNATE)
        iLightSet = 2;

    ENG_GetActiveLighting( EngLighting, iLightSet );
    if ( EngLighting.Directional[0].Active )
    {
        pLightDir->M[0][0] = -EngLighting.Directional[0].Direction.X;
        pLightDir->M[1][0] = -EngLighting.Directional[0].Direction.Y;
        pLightDir->M[2][0] = -EngLighting.Directional[0].Direction.Z;
    }
    else
    {   // direction doesn't matter because color will be zero...
        pLightDir->M[0][0] = 0.0f;
        pLightDir->M[1][0] = 0.0f;
        pLightDir->M[2][0] = 0.0f;
    }
    pLightDir->M[3][0] = 0.0f;
    if ( EngLighting.Directional[1].Active )
    {
        pLightDir->M[0][1] = -EngLighting.Directional[1].Direction.X;
        pLightDir->M[1][1] = -EngLighting.Directional[1].Direction.Y;
        pLightDir->M[2][1] = -EngLighting.Directional[1].Direction.Z;
    }
    else
    {   // direction doesn't matter because color will be zero
        pLightDir->M[0][1] = 0.0f;
        pLightDir->M[1][1] = 0.0f;
        pLightDir->M[2][1] = 0.0f;
    }
    pLightDir->M[3][1] = 0.0f;
    if ( EngLighting.Directional[2].Active )
    {
        pLightDir->M[0][2] = -EngLighting.Directional[2].Direction.X;
        pLightDir->M[1][2] = -EngLighting.Directional[2].Direction.Y;
        pLightDir->M[2][2] = -EngLighting.Directional[2].Direction.Z;
    }
    else
    {   // direction doesn't matter because color will be zero
        pLightDir->M[0][2] = 0.0f;
        pLightDir->M[1][2] = 0.0f;
        pLightDir->M[2][2] = 0.0f;
    }
    pLightDir->M[3][2] = 0.0f;
    pLightDir->M[0][3] = 0.0f;
    pLightDir->M[1][3] = 0.0f;
    pLightDir->M[2][3] = 0.0f;
    pLightDir->M[3][3] = 1.0f;

    //---   Figure out the Light Colors matrix
    if ( EngLighting.Directional[0].Active )
    {
        pLightColor->M[0][0] = (f32)(EngLighting.Directional[0].Color.R);
        pLightColor->M[0][1] = (f32)(EngLighting.Directional[0].Color.G);
        pLightColor->M[0][2] = (f32)(EngLighting.Directional[0].Color.B);
        pLightColor->M[0][3] = 255.0f; //(f32)(EngLighting.Directional[0].Color.A);
    }
    else
    {
        pLightColor->M[0][0] = 0.0f;
        pLightColor->M[0][1] = 0.0f;
        pLightColor->M[0][2] = 0.0f;
        pLightColor->M[0][3] = 0.0f;
    }
    if ( EngLighting.Directional[1].Active )
    {
        pLightColor->M[1][0] = (f32)(EngLighting.Directional[1].Color.R);
        pLightColor->M[1][1] = (f32)(EngLighting.Directional[1].Color.G);
        pLightColor->M[1][2] = (f32)(EngLighting.Directional[1].Color.B);
        pLightColor->M[1][3] = 255.0f; //(f32)(EngLighting.Directional[1].Color.A);
    }
    else
    {
        pLightColor->M[1][0] = 0.0f;
        pLightColor->M[1][1] = 0.0f;
        pLightColor->M[1][2] = 0.0f;
        pLightColor->M[1][3] = 0.0f;
    }
    if ( EngLighting.Directional[2].Active )
    {
        pLightColor->M[2][0] = (f32)(EngLighting.Directional[2].Color.R);
        pLightColor->M[2][1] = (f32)(EngLighting.Directional[2].Color.G);
        pLightColor->M[2][2] = (f32)(EngLighting.Directional[2].Color.B);
        pLightColor->M[2][3] = 255.0f; //(f32)(EngLighting.Directional[2].Color.A);
    }
    else
    {
        pLightColor->M[2][0] = 0.0f;
        pLightColor->M[2][1] = 0.0f;
        pLightColor->M[2][2] = 0.0f;
        pLightColor->M[2][3] = 0.0f;
    }
    pLightColor->M[3][0] = (f32)(EngLighting.Ambient.R);
    pLightColor->M[3][1] = (f32)(EngLighting.Ambient.G);
    pLightColor->M[3][2] = (f32)(EngLighting.Ambient.B);
    pLightColor->M[3][3] = 255.0f; //(f32)(EngLighting.Ambient.A);
}

//==========================================================================

byte* QSkin::BuildMorphTargetData( s32 MeshID )
{
    s32                 TargetID;
    s32                 DeltaID;
    byte*               pDeltaData;
    s32                 FirstDeltaValue;
    s32                 FirstDeltaLoc;
    t_SkinDeltaLoc*     pLoc;
    t_SkinDeltaValue*   pDest;
    t_SkinDeltaValue    DeltaValue;

    //---   if the target ID is the default target, and there is no weight
    //      we can just return the default delta data
    if ( m_pMorphTargets[m_pMeshes[MeshID].FirstMorphTarget].Active &&
         m_pMorphTargets[m_pMeshes[MeshID].FirstMorphTarget].Weight == 1.0f )
    {
        //#### Read the comment in SetActiveTargets about my hackish bug fix.
        return &m_pDeltaData[m_pMeshes[MeshID].DeltaDataOffset];
    }

    //---   it appears we must build new morph target data, allocate some
    //      room of the scratch memory heap for that, and copy over the
    //      default delta data
    pDeltaData = SMEM_BufferAlloc( m_pMeshes[MeshID].DeltaDataSize + 16 );
    if ( ((u32)pDeltaData & 0xf) != 0 )
        pDeltaData += 0x10 - ((u32)pDeltaData & 0xf);  // align to 16 byte boundary
    ASSERT( pDeltaData );
    ASSERT( ((u32)pDeltaData & 0xf) == 0 );
    ASSERT( (m_pMeshes[MeshID].DeltaDataSize & 0xf) == 0 );
    x_memcpy( pDeltaData,
              &m_pDeltaData[m_pMeshes[MeshID].DeltaDataOffset],
              m_pMeshes[MeshID].DeltaDataSize );

    //---   now loop through each of the targets, adding deltas based on weight
    //      as appropriate. (Note we can skip the first one because it is just 0.0f anyway)
    for ( TargetID = (m_pMeshes[MeshID].FirstMorphTarget + 1);
          TargetID < (m_pMeshes[MeshID].FirstMorphTarget + m_pMeshes[MeshID].NMorphTargets);
          TargetID++ )
    {
        if ( m_pMorphTargets[TargetID].Active )
        {
            //---   set the appropriate offsets for this morph target
            FirstDeltaValue  = m_pMeshes[MeshID].FirstDeltaValue;
            FirstDeltaValue += m_pMeshes[MeshID].NDeltaLocations *
                               (TargetID - m_pMeshes[MeshID].FirstMorphTarget);
            FirstDeltaLoc    = m_pMeshes[MeshID].FirstDeltaLocation;

            for ( DeltaID = 0; DeltaID < m_pMeshes[MeshID].NDeltaLocations; DeltaID++ )
            {
                pLoc = &m_pDeltaLocations[FirstDeltaLoc + DeltaID];

                DeltaValue.XDelta = (s16)(m_pMorphTargets[TargetID].Weight * (f32)m_pDeltaValues[FirstDeltaValue + DeltaID].XDelta);
                DeltaValue.YDelta = (s16)(m_pMorphTargets[TargetID].Weight * (f32)m_pDeltaValues[FirstDeltaValue + DeltaID].YDelta);
                DeltaValue.ZDelta = (s16)(m_pMorphTargets[TargetID].Weight * (f32)m_pDeltaValues[FirstDeltaValue + DeltaID].ZDelta);

                pDest = (t_SkinDeltaValue*)&pDeltaData[*pLoc];
                pDest->XDelta += DeltaValue.XDelta;
                pDest->YDelta += DeltaValue.YDelta;
                pDest->ZDelta += DeltaValue.ZDelta;
            }
        }
    }

    return pDeltaData;
}

//=====================================================================================================================================
/*
void QSkin::RenderSkin( radian3* pBoneRots,
                        vector3& WorldPos,
                        radian   WorldYaw,
                        void*    pAnimGroupPtr,
                        xbool    Mirrored,
                        f32      WorldScale,
                        vector3* pBoneScales,
                        u32      UserData1,
                        u32      UserData2,
                        u32      UserData3
                      )
*/
void QSkin::RenderSkin( matrix4*	pBoneMatrices,
                        vector3*	pBoneScales,
						xbool		bMirror,
						anim_group*	pAnimGroup,
                        u32			UserData1,
                        u32			UserData2,
                        u32			UserData3
                      )
{
//    return;
//    matrix4       TempM[34] __attribute__ ((aligned(16)));
    t_MatrixGroup*  pMatrixGroup;

    //---   confirm we are in render mode & the buffer is open
    ASSERT( ENG_GetRenderMode() );
    ASSERT( s_pSkinBuffer );
    ASSERT( s_NMtxGroupsInBuffer < s_MaxMtxGroups );

    //---   figure out the pointers to the data stuff
    pMatrixGroup = (t_MatrixGroup*)
        &s_pSkinBuffer[sizeof(t_MatrixGroup) * s_NMtxGroupsInBuffer];

    //---   keep our count current
    s_NMtxGroupsInBuffer++;

	BuildMatrices(	pMatrixGroup,
					pBoneMatrices,
					pBoneScales,
					bMirror,
					pAnimGroup );

    RenderSkin( pMatrixGroup, UserData1, UserData2, UserData3, 0, NULL );
}

//=====================================================================================================================================
void QSkin::RenderSkin( t_MatrixGroup* pMatrixGroup,
                        u32            UserData1,
                        u32            UserData2,
                        u32            UserData3,
					    s32			   NShadowMtx,
					    matrix4*	   pShadowMtx )
{
//    return;
    s32             i, j, k;
	s32				LastSubMesh;
    t_SubMeshGroup* pSubMeshGroup;
    s32*            pNSubMeshes;
    byte*           pCurrDeltaData;

	ASSERT( pMatrixGroup != NULL );
    ASSERT( (NShadowMtx <= 0) || ((NShadowMtx > 0) && (pShadowMtx != NULL)) );

    //---   figure out the pointers to the data stuff
    pNSubMeshes = (s32*)
        &s_pSkinBuffer[sizeof(t_MatrixGroup) * s_MaxMtxGroups];

    pSubMeshGroup = (t_SubMeshGroup*)
        &s_pSkinBuffer[sizeof(t_MatrixGroup) * s_MaxMtxGroups +
                       sizeof(s32) +
                       sizeof(t_SubMeshGroup) * (*pNSubMeshes)];


    //---   loop through meshes and add references
    for ( i = 0; i < m_NMeshes; i++ )
    {
        if ( (m_pMeshes[i].Flags & SKIN_MESH_FLAG_VISIBLE) == 0 )
            continue;

        if ( m_pMeshes[i].Flags & SKIN_MESH_FLAG_MORPH )
        {
            pCurrDeltaData = BuildMorphTargetData( i );
        }
        else
        {
            pCurrDeltaData = NULL;
        }

		LastSubMesh = m_pMeshes[i].FirstSubMesh + m_pMeshes[i].NSubMeshes;

        for ( j = m_pMeshes[i].FirstSubMesh; j < LastSubMesh; j++ )
        {
            //--- check if submesh is marked as visible
            if ( (m_pSubMeshes[j].Flags & SKIN_SUBMESH_FLAG_VISIBLE) == 0 )
                continue;

            if ( m_pSubMeshes[j].Flags & SKIN_SUBMESH_FLAG_SHADOW )
            {
				for ( k = 0; k < NShadowMtx; k++ )
				{
					ASSERT( *pNSubMeshes < s_MaxSubMeshes );
					pSubMeshGroup->Flags        = m_pSubMeshes[j].Flags;
					pSubMeshGroup->NBones       = m_NBones;
					pSubMeshGroup->MeshID       = i;
					pSubMeshGroup->NTris        = m_pSubMeshes[j].NTris;
					pSubMeshGroup->NVerts       = m_pSubMeshes[j].NVerts;
					pSubMeshGroup->pPacketData  = m_pPacketData;
					pSubMeshGroup->pDeltaData   = pCurrDeltaData;
					pSubMeshGroup->NCacheFrames = m_pSubMeshes[j].NCacheFrames;
					pSubMeshGroup->pCacheFrames = &m_pCacheFrames[m_pSubMeshes[j].FirstCacheFrame];
					pSubMeshGroup->pVertLoads   = m_pVertLoads;
					pSubMeshGroup->pMatrices    = pMatrixGroup;
					pSubMeshGroup->pShadowMtx   = &pShadowMtx[k];
					pSubMeshGroup->UserData1    = UserData1;
					pSubMeshGroup->UserData2    = UserData2;
					pSubMeshGroup->UserData3    = UserData3;
					pSubMeshGroup->pBitmap      = NULL;

					pSubMeshGroup++;
					(*pNSubMeshes) += 1;
				}
            }
            else
            {
				ASSERT( *pNSubMeshes < s_MaxSubMeshes );
				pSubMeshGroup->Flags        = m_pSubMeshes[j].Flags;
				pSubMeshGroup->NBones       = m_NBones;
				pSubMeshGroup->MeshID       = i;
				pSubMeshGroup->NTris        = m_pSubMeshes[j].NTris;
				pSubMeshGroup->NVerts       = m_pSubMeshes[j].NVerts;
				pSubMeshGroup->pPacketData  = m_pPacketData;
				pSubMeshGroup->pDeltaData   = pCurrDeltaData;
				pSubMeshGroup->NCacheFrames = m_pSubMeshes[j].NCacheFrames;
				pSubMeshGroup->pCacheFrames = &m_pCacheFrames[m_pSubMeshes[j].FirstCacheFrame];
				pSubMeshGroup->pVertLoads   = m_pVertLoads;
				pSubMeshGroup->pMatrices    = pMatrixGroup;
				pSubMeshGroup->pShadowMtx   = NULL;
				pSubMeshGroup->UserData1    = UserData1;
				pSubMeshGroup->UserData2    = UserData2;
                pSubMeshGroup->UserData3    = UserData3;
                ASSERT( m_pTextureArray );
                ASSERT( m_pTextureArray[m_pSubMeshes[j].TextureID] );
                pSubMeshGroup->pBitmap = m_pTextureArray[m_pSubMeshes[j].TextureID];

				pSubMeshGroup++;
				(*pNSubMeshes) += 1;
            }
        }
    }
}

//==========================================================================

inline void iDMAHELP_BuildTagRef( sceDmaTag* pTag, u32 RefAddress, s32 NBytes )
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

//==========================================================================

void QSkin::RenderSubMeshGroup( t_SubMeshGroup* pGroup )
{
//    return;
    s32                 CacheFrameID;
    s32                 VertLoadID;
    sceDmaTag*          pDMA;
    t_SkinCacheFrame*   pCacheFrame;
    t_SkinVertLoad*     pVertLoad;

    /*if ( (pGroup->Flags & SKIN_SUBMESH_FLAG_MORPH) == 0 )
    {
        pCacheFrame = &pGroup->pCacheFrames[0];
        for ( CacheFrameID = 0; CacheFrameID < pGroup->NCacheFrames; CacheFrameID++ )
        {
            pVertLoad = &pGroup->pVertLoads[pCacheFrame->FirstVertLoad];
            for ( VertLoadID = 0; VertLoadID < pCacheFrame->NVertLoads; VertLoadID++ )
            {
                //---   add reference to vert load and transform kick all at once
                pDMA = (sceDmaTag*)pDList;
                iDMAHELP_BuildTagRef( pDMA,
                                      (u32)&pGroup->pPacketData[pVertLoad->VertPacketOffset],
                                      pVertLoad->VertPacketSize + pVertLoad->KickPacketSize );
                pDList += sizeof(sceDmaTag);

                //---   bump up our stats
                if ( s_pStatNBytes )
                    *s_pStatNBytes = *s_pStatNBytes + pVertLoad->VertPacketSize + pVertLoad->KickPacketSize;

                //---   advance to the next vert load
                pVertLoad++;
            }

            //---   add reference to strip flush
            pDMA = (sceDmaTag*)pDList;
            iDMAHELP_BuildTagRef( pDMA,
                                 (u32)&pGroup->pPacketData[pCacheFrame->FlushPacketOffset],
                                 pCacheFrame->FlushPacketSize );
            pDList += sizeof(sceDmaTag);
            if ( s_pStatNBytes )
                *s_pStatNBytes = *s_pStatNBytes + pCacheFrame->FlushPacketSize;

            //---   advance to next cache frame
            pCacheFrame++;
        }
    }
    else
    {*/
        pCacheFrame = &pGroup->pCacheFrames[0];
        for ( CacheFrameID = 0; CacheFrameID < pGroup->NCacheFrames; CacheFrameID++ )
        {
            pVertLoad = &pGroup->pVertLoads[pCacheFrame->FirstVertLoad];
            for ( VertLoadID = 0; VertLoadID < pCacheFrame->NVertLoads; VertLoadID++ )
            {
                //---   add reference to vert load
                pDMA = (sceDmaTag*)pDList;
                iDMAHELP_BuildTagRef( pDMA,
                                     (u32)&pGroup->pPacketData[pVertLoad->VertPacketOffset],
                                     pVertLoad->VertPacketSize );
                pDList += sizeof(sceDmaTag);

                //---   add reference to deltas
                if ( pGroup->Flags & SKIN_SUBMESH_FLAG_MORPH )
                {
                    pDMA = (sceDmaTag*)pDList;
                    iDMAHELP_BuildTagRef( pDMA,
                                         (u32)&pGroup->pDeltaData[pVertLoad->DeltaPacketOffset],
                                         pVertLoad->DeltaPacketSize );
                    pDList += sizeof(sceDmaTag);
                }

                //---   add reference to vert transform kick
                pDMA = (sceDmaTag*)pDList;
                iDMAHELP_BuildTagRef( pDMA,
                                     (u32)&pGroup->pPacketData[pVertLoad->KickPacketOffset],
                                     pVertLoad->KickPacketSize );
                pDList += sizeof(sceDmaTag);

                //---   bump up our stats
                if ( s_pStatNBytes )
                    *s_pStatNBytes = *s_pStatNBytes + pVertLoad->VertPacketSize + pVertLoad->DeltaPacketSize + pVertLoad->KickPacketSize;

                //---   advance to the next vert load
                pVertLoad++;
            }

            //---   add reference to strip flush
            pDMA = (sceDmaTag*)pDList;
            iDMAHELP_BuildTagRef( pDMA,
                                 (u32)&pGroup->pPacketData[pCacheFrame->FlushPacketOffset],
                                 pCacheFrame->FlushPacketSize );
            pDList += sizeof(sceDmaTag);
            if ( s_pStatNBytes )
                *s_pStatNBytes = *s_pStatNBytes + pCacheFrame->FlushPacketSize;

            //---   advance to next cache frame
            pCacheFrame++;
        }
    /*}*/

    //---   keep our stats current
    if ( s_pStatNVerts )
        *s_pStatNVerts = *s_pStatNVerts + pGroup->NVerts;
    if ( s_pStatNTris )
        *s_pStatNTris = *s_pStatNTris + pGroup->NTris;
}

//==========================================================================

void QSkin::RenderSubMesh( u32 SkinData )
{
//    return;
    t_SubMeshGroup* pSubMesh = (t_SubMeshGroup*)SkinData;
    ResendMatrices( SkinData );
    RenderSubMeshGroup( pSubMesh );
}

//==========================================================================

void QSkin::ResendMatrices( u32 SkinData )
{
    t_SubMeshGroup* pSubMesh = (t_SubMeshGroup*)SkinData;
    sceDmaTag*      pDMA;

    if ( pSubMesh && pSubMesh->pMatrices != s_pMatrices )
    {
        s32 NVectors;

        s_pMatrices = pSubMesh->pMatrices;
        NVectors = 0;

/*******************************************************************************\
|                                                                               |
|                     Please to be Prebuilding of this Stuff                    |
|                                                                               |
\*******************************************************************************/
        {                                                                      //
            //---   grab some space for the dma tag for the matrices           //
            pDMA = (sceDmaTag*)pDList;  // we'll fill it in later!             //
            pDList += sizeof(sceDmaTag);                                       //
                                                                               //
            //---   set up the unpack command for the lighting data            //
            s_pMatrices->VIFLight[0] = 0;                                      //
            s_pMatrices->VIFLight[1] = 0;                                      //
            s_pMatrices->VIFLight[2] = 0;                                      //
            VIFHELP_BuildTagUnpack( &s_pMatrices->VIFLight[3],                 //
                                    SKIN_COLOR_MATRIX_BASE,                    //
                                    8,                                         //
                                    VIFHELP_UNPACK_V4_32,                      //
                                    FALSE,                                     //
                                    FALSE,                                     //
                                    TRUE );                                    //
            NVectors += 9;  // 1 for unpack command, 8 for matrix              //
                                                                               //
            //---   set up the unpack command for the bone matrix data         //
            s_pMatrices->VIFMatrix[0] = 0;                                     //
            s_pMatrices->VIFMatrix[1] = 0;                                     //
            s_pMatrices->VIFMatrix[2] = 0;                                     //
            VIFHELP_BuildTagUnpack( &s_pMatrices->VIFMatrix[3],                //
                                    SKIN_MATRIX_CACHE_BASE,                    //
                                    pSubMesh->NBones * 7,                      //
                                    VIFHELP_UNPACK_V4_32,                      //
                                    FALSE,                                     //
                                    FALSE,                                     //
                                    TRUE );                                    //
            NVectors += 1 + pSubMesh->NBones * 7;                              //
                                                                               //
            //---   now we can fill in the DMA tag                             //
            iDMAHELP_BuildTagRef( pDMA, (u32)s_pMatrices, NVectors * 16 );     //
        }                                                                      //
/*******************************************************************************\
|                                                                               |
|                     Please to be Prebuilding of this Stuff                    |
|                                                                               |
\*******************************************************************************/
    }
}

//==========================================================================

#define NUM_BUCKETS                 16
#define BITS_PER_PASS               4
#define BUCKET_AND                  0x0f
#define NUM_PASSES                  4

typedef struct
{
    s16     SortKey;
    s16     Index;
} SSortSubMesh;

////////////////////////////////////////////////////////////////////////////
// NEED: 1) Sort key for every submesh
//       2) Values to sort (either ptrs or indices) for each submesh
////////////////////////////////////////////////////////////////////////////

static void RadixSort( s32 NSubMeshes, SSortSubMesh* pSubMeshes )
{
    s32*            pBucketCount;
    SSortSubMesh*   pBuckets;
    s32             Pass;
    s32             BucketID;
    s32             Shifter;
    s32             SubMeshID;
    s32             Count;
    s32             BucketOffset;

    //---   allocate temporary space for the buckets and bucket counts
    SMEM_StackPushMarker();
    pBuckets = (SSortSubMesh*)SMEM_StackAlloc( sizeof(SSortSubMesh) * NUM_BUCKETS * NSubMeshes );
    pBucketCount = (s32*)SMEM_StackAlloc( sizeof(s32) * NUM_BUCKETS );
    ASSERT( pBuckets && pBucketCount );

    //---   do each pass
    Shifter = 0;
    for ( Pass = 0; Pass < NUM_PASSES; Pass++ )
    {
        //---   empty the buckets
        for ( BucketID = 0; BucketID < NUM_BUCKETS; BucketID++ )
            pBucketCount[BucketID] = 0;

        //---   put the submeshes in their buckets
        for ( SubMeshID = 0; SubMeshID < NSubMeshes; SubMeshID++ )
        {
            BucketID = (pSubMeshes[SubMeshID].SortKey>>Shifter) & BUCKET_AND;
            BucketOffset = BucketID*NSubMeshes;
            pBuckets[BucketOffset+pBucketCount[BucketID]] = pSubMeshes[SubMeshID];
            pBucketCount[BucketID]++;
            ASSERT( pBucketCount[BucketID] <= NSubMeshes );
        }

        //---   remerge the list from the buckets
        Count = 0;
        for ( BucketID = 0; BucketID < NUM_BUCKETS; BucketID++ )
        {
            BucketOffset = BucketID*NSubMeshes;
            for ( SubMeshID = 0; SubMeshID < pBucketCount[BucketID]; SubMeshID++ )
            {
                pSubMeshes[Count] = pBuckets[BucketOffset+SubMeshID];
                Count++;
            }
        }
        ASSERT( Count == NSubMeshes );

        //---   change shifter to inspect next bits
        Shifter += BITS_PER_PASS;
    }

    //---   free temporary space
    SMEM_StackPopToMarker();
}

//==========================================================================


void QSkin::FlushSkinBuffer( void )
{
    s32             i;
    s32             NSubMeshes;
    sceDmaTag*      pDMA;
    t_SubMeshGroup* pSubMesh;
    t_SubMeshGroup* pSubMeshStart;
    x_bitmap*       pBitmap = (x_bitmap*)0xffffffff;
    matrix4*        pMatrix;
    SSortSubMesh*   pSubMeshOrder;
    s_pMatrices = NULL;

    //---   Confirm we are in render mode
    ASSERT( ENG_GetRenderMode() );
    ASSERT( s_pSkinBuffer );

    //---   try to bail out early
    NSubMeshes =
        *((s32*)&s_pSkinBuffer[s_MaxMtxGroups * sizeof(t_MatrixGroup)]);

    if ( NSubMeshes == 0 )
        return;

    //---   Be sure microcode is loaded
    ActivateMicroCode();

    //---   Setup DMA tag for boot packet and row&mask settings
    pDMA = (sceDmaTag*)pDList;
    pDList += sizeof(sceDmaTag);
    DMAHELP_BuildTagRef( pDMA, 
                        (u32)s_pFlushData, 
                        sizeof( t_FlushData ) );

    //---   grab a ptr to the submeshes
    pSubMesh = (t_SubMeshGroup*)
        &s_pSkinBuffer[s_MaxMtxGroups * sizeof(t_MatrixGroup) + sizeof(s32)];

    //---   create a temp buffer to store the submesh order
    SMEM_StackPushMarker();
    pSubMeshOrder = (SSortSubMesh*)SMEM_StackAlloc( sizeof(SSortSubMesh) * NSubMeshes );
    ASSERT( pSubMeshOrder );

    for ( i = 0; i < NSubMeshes; i++ )
    {
        if ( pSubMesh[i].pBitmap )
        {
            ASSERT( pSubMesh[i].pBitmap->GetVRAMID() < 1024 );
            ASSERTS( pSubMesh[ i ].pBitmap->GetVRAMID() >= 0, "Texture not initialized or deleted." );
            if (pSubMesh[ i ].pBitmap->GetVRAMID() < 0)
                pSubMeshOrder[i].SortKey = (0x1<<10);   // bleh
            else
                pSubMeshOrder[i].SortKey = pSubMesh[i].pBitmap->GetVRAMID();
        }
        else
        {
            pSubMeshOrder[i].SortKey = (0x1<<10);
        }
        //ASSERT( pSubMesh[i].Flags < 0x10 );

        pSubMeshOrder[i].SortKey += (pSubMesh[i].Flags & 0x0F) << 11;
        pSubMeshOrder[i].Index = i;
    }

    //---   sort the submeshes by texture and render flags
    RadixSort( NSubMeshes, pSubMeshOrder );

    //---   now just loop through the sub-meshes, rendering them at will
    pSubMeshStart = pSubMesh;

    for ( i = 0; i < NSubMeshes; i++ )
    {
        pSubMesh = &pSubMeshStart[pSubMeshOrder[i].Index];

        //---   function behaves differently if a user-callback is present
        if ( s_PreRenderCallback == NULL )
        {
            ENG_SpecialCaseFog( TRUE );
            //---   set the render flags based on the sub-mesh flags

            if ( pSubMesh->Flags & SKIN_SUBMESH_FLAG_SHADOW )
            {
                ENG_SetRenderFlags( ENG_ZBUFFER_TEST_ON | ENG_ZBUFFER_FILL_ON | ENG_ALPHA_TEST_OFF | ENG_ALPHA_BLEND_ON );
                ENG_SetBlendMode( ENG_BLEND_NORMAL | ENG_BLEND_FIXED_ALPHA, 32 );
            }
            else
            if ( pSubMesh->Flags & SKIN_SUBMESH_FLAG_ENVPASS )
            {
                ENG_SetRenderFlags( ENG_ZBUFFER_TEST_ON | ENG_ZBUFFER_FILL_OFF | ENG_ALPHA_TEST_OFF | ENG_ALPHA_BLEND_ON );
                ENG_SetBlendMode( ENG_BLEND_FIXED_ALPHA | ENG_BLEND_ADDITIVE, 50 );
            }
            else
            if ( pSubMesh->Flags & SKIN_SUBMESH_FLAG_ALPHA )
            {
                ENG_SetRenderFlags( ENG_ZBUFFER_TEST_ON | ENG_ZBUFFER_FILL_OFF | ENG_ALPHA_TEST_OFF | ENG_ALPHA_BLEND_ON );
                ENG_SetBlendMode( ENG_BLEND_NORMAL );
            }
            else
            {
                ENG_SetRenderFlags( ENG_ZBUFFER_TEST_ON | ENG_ZBUFFER_FILL_ON | ENG_ALPHA_TEST_OFF | ENG_ALPHA_BLEND_OFF );
            }
            ENG_SpecialCaseFog( FALSE );
        }
        else
        {
            ENG_SpecialCaseFog( TRUE );
            //---   call the user's callback function
            if ( s_PreRenderCallback( pSubMesh->MeshID,
                                      (u32)pSubMesh,
                                      pSubMesh->UserData1,
                                      pSubMesh->UserData2,
                                      pSubMesh->UserData3 ) == FALSE )
            {
                ENG_SpecialCaseFog( FALSE );
                continue;
            }
            ENG_SpecialCaseFog( FALSE );
        }

        //---   load up a new texture if needed
        if ( (pSubMesh->pBitmap != pBitmap) && pSubMesh->pBitmap )
        {
            pBitmap = pSubMesh->pBitmap;
            VRAM_Activate( *pBitmap );
        }

        //---   load in a different set of matrices if necessary
        ResendMatrices( (u32)pSubMesh );

        //---   Send multiple shadows?
        if ( pSubMesh->Flags & SKIN_SUBMESH_FLAG_SHADOW )
        {
            //---   Turn off the FBA register. Then render the shadows. Because
            //      the dest alpha test only passes if the dest alpha is 0x1, this
            //      should theoretically keep the shadows from stacking up.
            //      Ask me if you need more info. -Darrin
            ENG_SpecialCaseFog( TRUE );
            PS2_SetDestAlphaTest( TRUE, 0x0 );
            ENG_EnableRGBAChannels( TRUE, TRUE, TRUE, TRUE );
            PS2_SetFBA( TRUE );
            ENG_SpecialCaseFog( FALSE );

            //===   send the shadow matrix directly to vumem
            pDMA = (sceDmaTag*)pDList;
            pDList += sizeof( sceDmaTag );
            DMAHELP_BuildTagRef( pDMA, 
                                 (u32)s_pFlushShadowMatrix, 
                                 sizeof( t_FlushShadowMatrix ) );

            //---   set up the shadow matrix
            pDMA = (sceDmaTag*)pDList;
            pDList += sizeof( sceDmaTag );
            DMAHELP_BuildTagCont( pDMA, sizeof(f32) * 16 );
            pMatrix = (matrix4*)pDList;
            pDList += (sizeof( f32 ) * 16);
            ENG_GetW2S( *pMatrix );
            AsmM4_Mult( *pMatrix, *pMatrix, *pSubMesh->pShadowMtx );

            //---   Render the SubMesh
            RenderSubMeshGroup( pSubMesh );

            PS2_SetDestAlphaTest( FALSE, 0x0 );
            ENG_EnableRGBAChannels( TRUE, TRUE, TRUE, FALSE );
            PS2_SetFBA( FALSE );
        }
        else
        {
            RenderSubMeshGroup( pSubMesh );
        }
    }

    //---   the buffer is no longer open!
    SMEM_StackPopToMarker();
    s_pSkinBuffer        = NULL;
    s_NMtxGroupsInBuffer = 0;
    s_MaxMtxGroups       = 0;

    //---   let the user know we are finished
    if ( s_FinishedCallback )
    {
        s_FinishedCallback();
    }
}

//==========================================================================
