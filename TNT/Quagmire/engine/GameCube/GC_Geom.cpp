////////////////////////////////////////////////////////////////////////////
//
// GC_Geom.cpp
//
// Source file for Gamecube specific code in QGeom
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_stdio.hpp"
#include "x_memory.hpp"

#include "Q_Engine.hpp"
#include "Q_GC.hpp"
#include "Q_View.hpp"
#include "Q_VRAM.hpp"
#include "Q_Geom.hpp"

#include <dolphin/gx.h>
#include <dolphin/mtx.h>


////////////////////////////////////////////////////////////////////////////
// GAMECUBE VERTEX DESC/FORMAT SETUP
////////////////////////////////////////////////////////////////////////////

inline void SET_GC_VTXFMT_POS_ONLY( void* pArray )
{
    GXClearVtxDesc();

    GXSetVtxDesc( GX_VA_POS, GX_INDEX16 );

    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0 );

    GXSetArray( GX_VA_POS, pArray, sizeof(vector3) );
}

inline void SET_GC_VTXFMT_NORMAL( void )
{
    GXClearVtxDesc();

    GXSetVtxDesc( GX_VA_POS,  GX_INDEX16 );
    GXSetVtxDesc( GX_VA_NRM,  GX_INDEX16 );
    GXSetVtxDesc( GX_VA_TEX0, GX_INDEX16 );

    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_POS,  GX_POS_XYZ, GX_F32, 0 );
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_NRM,  GX_NRM_XYZ, GX_F32, 0 );
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST,  GX_F32, 0 );
}

inline void SET_GC_VTXFMT_COLOR( void )
{
    GXClearVtxDesc();

    GXSetVtxDesc( GX_VA_POS,  GX_INDEX16 );
    GXSetVtxDesc( GX_VA_CLR0, GX_INDEX16 );
    GXSetVtxDesc( GX_VA_TEX0, GX_INDEX16 );

    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_POS,  GX_POS_XYZ,  GX_F32,   0 );
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0 );
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST,   GX_F32,   0 );
}


inline void SET_GC_VTX_ARRAY_NORMAL( void* pArray )
{
    GXSetArray( GX_VA_POS,  &((t_GCVertNormal*)pArray)[0].Pos,    sizeof(t_GCVertNormal) );
    GXSetArray( GX_VA_NRM,  &((t_GCVertNormal*)pArray)[0].Normal, sizeof(t_GCVertNormal) );
    GXSetArray( GX_VA_TEX0, &((t_GCVertNormal*)pArray)[0].UV,     sizeof(t_GCVertNormal) );
}

inline void SET_GC_VTX_ARRAY_COLOR( void* pArray )
{
    GXSetArray( GX_VA_POS,  &((t_GCVertColor*)pArray)[0].Pos,   sizeof(t_GCVertColor) );
    GXSetArray( GX_VA_CLR0, &((t_GCVertColor*)pArray)[0].Color, sizeof(t_GCVertColor) );
    GXSetArray( GX_VA_TEX0, &((t_GCVertColor*)pArray)[0].UV,    sizeof(t_GCVertColor) );
}


////////////////////////////////////////////////////////////////////////////
// GAMECUBE TEV SETUP
////////////////////////////////////////////////////////////////////////////

inline void SET_TEV_ONE_TEXTURE( void )
{
    // TEV SETUP:
    // using TEVSTAGE0
    // stage 0 color op: DestColor = VtxLightColor * Texture0Color (GX_MODULATE)
    // stage 0 alpha op: DestAlpha = VtxLightAlpha * Texture0Alpha (GX_MODULATE)
    //            OR     DestAlpha = FixedAlpha(from engine settings)

    xbool   FixAlphaOn;
    GXColor FixedColor = {0,0,0,0};

    GC_GetFixedAlpha( FixAlphaOn, FixedColor.a );

    s32 LastAlphaTevStage;
    s32 DXT3AlphaMapID = GC_GetDXT3AlphaMapStage( 0 );

    if( (DXT3AlphaMapID == -1) || FixAlphaOn )
        LastAlphaTevStage = 0;
    else
        LastAlphaTevStage = 1;

    GXSetNumChans    ( 1 );
    GXSetNumTevStages( LastAlphaTevStage + 1 );
    GXSetNumTexGens  ( 1 );
    GXSetTexCoordGen ( GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY );
    GXSetTevOrder    ( GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0 );

    // if alpha channel from DXT3 texture, set color stage 1 and alpha stage 0 to pass through
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

    // setup color channel, texture color * vtx lighting
    GXSetTevColorIn( GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO );

    // setup alpha output, either texture alpha * vtx alpha or engine fixed alpha value
    if( FixAlphaOn )
    {
        GXSetTevColor  ( GX_TEVREG0, FixedColor );
        GXSetTevAlphaIn( TEVSTAGE_ID[LastAlphaTevStage], GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A0 );
    }
    else
    {
        GXSetTevAlphaIn( TEVSTAGE_ID[LastAlphaTevStage], GX_CA_ZERO, GX_CA_RASA, GX_CA_TEXA, GX_CA_ZERO );
    }
}

//--------------------------------------------------------------------------

inline void SET_TEV_ENV_MAP( void )
{
    // TEV SETUP:
    // using TEVSTAGE0, TEVSTAGE1
    // stage 0 color op: DestColor  = VtxLightColor * Texture0Color (GX_MODULATE)
    // stage 0 alpha op: DestAlpha  = VtxLightAlpha * Texture0Alpha (GX_MODULATE)
    // stage 1 color op: DestColor += EnvFixedAlpha * Texture1Color (Additive blend)
    // stage 1 alpha op: DestAlpha  = DestAlpha from stage 0
    //            OR     DestAlpha  = FixedAlpha(from engine settings)

    xbool   FixAlphaOn;
    GXColor FixedColor = { 0, 0, 0,  0 };
    GXColor EnvColorA  = { 0, 0, 0, 50 };

    GC_GetFixedAlpha( FixAlphaOn, FixedColor.a );

    s32 LastAlphaTevStage;
    s32 DXT3AlphaMapID = GC_GetDXT3AlphaMapStage( 0 );

    if( (DXT3AlphaMapID == -1) || FixAlphaOn )
        LastAlphaTevStage = 1;
    else
        LastAlphaTevStage = 2;


    GXSetNumChans    ( 1 );
    GXSetNumTevStages( LastAlphaTevStage + 1 );
    GXSetNumTexGens  ( 2 );
    GXSetTexCoordGen ( GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY );
    GXSetTexCoordGen2( GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_NRM,  GX_TEXMTX0, GX_FALSE, GX_PTTEXMTX0 );
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
        GXSetTevColor  ( GX_TEVREG0, FixedColor );
        GXSetTevAlphaIn( GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A0 );
        GXSetTevAlphaIn( GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A0 );
    }
    else
    {
        GXSetTevAlphaIn( GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_RASA, GX_CA_TEXA, GX_CA_ZERO );
        GXSetTevAlphaIn( GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV );
    }
}


////////////////////////////////////////////////////////////////////////////
// QGeom IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

QGeom::QGeom( void )
{
    InitData();
}

//==========================================================================

QGeom::QGeom( char* filename )
{
    InitData();

    SetupFromResource( filename );
}

//==========================================================================

QGeom::~QGeom( void )
{
    KillData();
}

//==========================================================================

void QGeom::InitData( void )
{
    m_Name[0]           = '\0';
    m_Flags             = 0;
    m_pTextureNames     = NULL;
    m_pTextures         = NULL;
    m_pTexturePtrs      = NULL;
    m_pMeshes           = NULL;
    m_pSubMeshes        = NULL;
    m_NTextures         = 0;
    m_NTexturesOwned    = 0;
    m_NMeshes           = 0;
    m_NSubMeshes        = 0;
    m_MeshLocked        = -1;
    m_L2W.Identity();

    m_pRawData          = NULL;

    m_pVertexData       = NULL;
    m_pDispListData     = NULL;
    m_VertexDataSize    = 0;
    m_DispListDataSize  = 0;

    m_MeshTextureArray  = NULL;
}

//==========================================================================

void QGeom::KillData( void )
{
    if( m_pRawData != NULL )
    {
        x_free( m_pRawData );
        m_pRawData = NULL;
    }

    // delete the mesh texture array
    SAFE_DELETE_ARRAY( m_MeshTextureArray )

    KillOwnedTextures();
}

//==========================================================================

void QGeom::SetupFromResource( char* pFilename )
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

void QGeom::SetupFromResource( X_FILE* pFilePtr )
{
    ////////////////////////////////////////////////////////////////////////
    // VERY IMPORTANT NOTE: THE SetupFromResource ROUTINE IS OVERLOADED A
    // COUPLE OF TIMES. IF THE DATA FORMAT CHANGES AT ALL, MAKE SURE THE
    // OVERLOADED ROUTINES GET CHANGED AS WELL.
    ////////////////////////////////////////////////////////////////////////

    ASSERT( pFilePtr != NULL );

    s32     i;
    u32     RawDataSize;
    u32     RawMeshSize;
    byte*   pTempRaw;

    t_GeomFileHeader hdr;

    //--- Clear out any previous data
    KillData();
    InitData();

    //--- Read in the file header
    x_fread( &hdr, sizeof(t_GeomFileHeader), 1, pFilePtr );

    //--- Copy header info to members
    x_strcpy( m_Name, hdr.Name );
    m_Flags             = hdr.Flags;
    m_NTextures         = hdr.NTextures;
    m_NMeshes           = hdr.NMeshes;
    m_NSubMeshes        = hdr.NSubMeshes;
    m_VertexDataSize    = hdr.VertexDataSize;
    m_DispListDataSize  = hdr.DispListDataSize;

    //--- Calc the size of the raw data to allocate
    RawDataSize = sizeof(char) * TEXTURE_NAME_LENGTH * m_NTextures;
    RawDataSize += sizeof(t_GeomMesh) * m_NMeshes;
    RawDataSize += sizeof(t_GeomSubMesh) * m_NSubMeshes;
    RawMeshSize = RawDataSize;
    RawDataSize += m_VertexDataSize + m_DispListDataSize + 32;  // add 32 bytes for alignment purposes

    m_pRawData = (byte*)x_malloc( RawDataSize );
    ASSERT( m_pRawData != NULL );

    pTempRaw = m_pRawData;

    //--- Assign real data pointers into raw data buffer
    if( m_NTextures != 0 )
    {
        m_pTextureNames = (char*)pTempRaw;
        pTempRaw += sizeof(char) * TEXTURE_NAME_LENGTH * m_NTextures;
    }

    m_pMeshes = (t_GeomMesh*)pTempRaw;
    pTempRaw += sizeof(t_GeomMesh) * m_NMeshes;

    m_pSubMeshes = (t_GeomSubMesh*)pTempRaw;
    pTempRaw += sizeof(t_GeomSubMesh) * m_NSubMeshes;

    //--- Align packet data on 32 byte boundary if needed, and then read in
    //--- texture names, mesh data, submesh data vertex and display list data
    if( ((u32)pTempRaw & 0x1F) != 0 )
    {
        pTempRaw += 0x20 - ((u32)pTempRaw & 0x1F);

        m_pVertexData = pTempRaw;
        m_pDispListData = pTempRaw + m_VertexDataSize;

        x_fread( m_pRawData, sizeof(byte), RawMeshSize, pFilePtr );
        x_fread( pTempRaw, sizeof(byte), m_VertexDataSize + m_DispListDataSize, pFilePtr );
    }
    else
    {
        m_pVertexData = pTempRaw;
        m_pDispListData = pTempRaw + m_VertexDataSize;

        //--- If no alignment was needed, read in all data at once
        x_fread( m_pRawData, sizeof(byte), RawMeshSize + m_VertexDataSize + m_DispListDataSize, pFilePtr );
    }

    //--- Allocate bitmap memory if textures stored in file
    if( hdr.NTexturesInFile > 0 )
    {
        m_Flags             |= GEOM_FLAG_OWNS_TEXTURES;
        m_NTexturesOwned    = hdr.NTexturesInFile;
        m_pTextures         = new x_bitmap[ m_NTexturesOwned ];
        ASSERT( m_pTextures != NULL );

        //--- Load the texture data
        for( i = 0; i < hdr.NTexturesInFile; i++ )
        {
            VERIFY( m_pTextures[i].Load( pFilePtr ) );
            VRAM_Register( m_pTextures[i] );
        }
    }


    //--- Since the pointers to vertex data in the meshes are read in as offsets,
    //--- we need to add the actual pointer location to give them correct address values
    for( i = 0; i < m_NMeshes; i++ )
        m_pMeshes[i].pVertData = m_pVertexData + ((u32)m_pMeshes[i].pVertData);

    //--- The same thing applies to the submeshes and the display lists
    for( i = 0; i < m_NSubMeshes; i++ )
        m_pSubMeshes[i].pDispList = m_pDispListData + ((u32)m_pSubMeshes[i].pDispList);


    //--- For the display lists to work, they must be invalidated from the CPU cache
    DCFlushRange( m_pDispListData, m_DispListDataSize );
    DCFlushRange( m_pVertexData,   m_VertexDataSize );

    // create the mesh texture array (for overriding a meshes default texture)
    m_MeshTextureArray = (x_bitmap**)x_malloc( sizeof(x_bitmap*) * m_NMeshes );
    ASSERT( m_MeshTextureArray != NULL );
    x_memset( m_MeshTextureArray, NULL, sizeof(x_bitmap*) * m_NMeshes );
}

//==========================================================================

void QGeom::SetupFromResource( byte* pByteStream )
{
    ////////////////////////////////////////////////////////////////////////
    // VERY IMPORTANT NOTE: THE SetupFromResource ROUTINE IS OVERLOADED A
    // COUPLE OF TIMES. IF THE DATA FORMAT CHANGES AT ALL, MAKE SURE THE
    // OVERLOADED ROUTINES GET CHANGED AS WELL.
    ////////////////////////////////////////////////////////////////////////

    ASSERT( pByteStream != NULL );

    s32     i;
    u32     RawDataSize;
    u32     RawMeshSize;
    byte*   pTempRaw;

    t_GeomFileHeader hdr;

    //--- Clear out any previous data
    KillData();
    InitData();

    //--- Read in the file header
    x_memcpy( &hdr, pByteStream, sizeof(t_GeomFileHeader) );
    pByteStream += sizeof(t_GeomFileHeader);

    //--- Copy header info to members
    x_strcpy( m_Name, hdr.Name );
    m_Flags             = hdr.Flags;
    m_NTextures         = hdr.NTextures;
    m_NMeshes           = hdr.NMeshes;
    m_NSubMeshes        = hdr.NSubMeshes;
    m_VertexDataSize    = hdr.VertexDataSize;
    m_DispListDataSize  = hdr.DispListDataSize;

    //--- Calc the size of the raw data to allocate
    RawDataSize = sizeof(char) * TEXTURE_NAME_LENGTH * m_NTextures;
    RawDataSize += sizeof(t_GeomMesh) * m_NMeshes;
    RawDataSize += sizeof(t_GeomSubMesh) * m_NSubMeshes;
    RawMeshSize = RawDataSize;
    RawDataSize += m_VertexDataSize + m_DispListDataSize + 32;  // add 32 bytes for alignment purposes

    m_pRawData = (byte*)x_malloc( RawDataSize );
    ASSERT( m_pRawData != NULL );

    pTempRaw = m_pRawData;

    //--- Assign real data pointers into raw data buffer
    if( m_NTextures != 0 )
    {
        m_pTextureNames = (char*)pTempRaw;
        pTempRaw += sizeof(char) * TEXTURE_NAME_LENGTH * m_NTextures;
    }

    m_pMeshes = (t_GeomMesh*)pTempRaw;
    pTempRaw += sizeof(t_GeomMesh) * m_NMeshes;

    m_pSubMeshes = (t_GeomSubMesh*)pTempRaw;
    pTempRaw += sizeof(t_GeomSubMesh) * m_NSubMeshes;

    //--- Align packet data on 32 byte boundary if needed, and then read in
    //--- texture names, mesh data, submesh data vertex and display list data
    if( ((u32)pTempRaw & 0x1F) != 0 )
    {
        pTempRaw += 0x20 - ((u32)pTempRaw & 0x1F);

        m_pVertexData = pTempRaw;
        m_pDispListData = pTempRaw + m_VertexDataSize;

        x_memcpy( m_pRawData, pByteStream, RawMeshSize );
        pByteStream += RawMeshSize;
        x_memcpy( pTempRaw, pByteStream, m_VertexDataSize + m_DispListDataSize );
        pByteStream += m_VertexDataSize + m_DispListDataSize;
    }
    else
    {
        m_pVertexData = pTempRaw;
        m_pDispListData = pTempRaw + m_VertexDataSize;

        //--- If no alignment was needed, read in all data at once
        x_memcpy( m_pRawData, pByteStream, RawMeshSize + m_VertexDataSize + m_DispListDataSize );
        pByteStream += RawMeshSize + m_VertexDataSize + m_DispListDataSize;
    }

    //--- Allocate bitmap memory if textures stored in file
    if( hdr.NTexturesInFile > 0 )
    {
        m_Flags             |= GEOM_FLAG_OWNS_TEXTURES;
        m_NTexturesOwned    = hdr.NTexturesInFile;
        m_pTextures         = new x_bitmap[ m_NTexturesOwned ];
        ASSERT( m_pTextures != NULL );

        s32 NBytesRead;

        //--- Load the texture data
        for( i = 0; i < hdr.NTexturesInFile; i++ )
        {
            VERIFY( m_pTextures[i].Load( pByteStream, NBytesRead ) );
            pByteStream += NBytesRead;
            VRAM_Register( m_pTextures[i] );
        }
    }


    //--- Since the pointers to vertex data in the meshes are read in as offsets,
    //--- we need to add the actual pointer location to give them correct address values
    for( i = 0; i < m_NMeshes; i++ )
        m_pMeshes[i].pVertData = m_pVertexData + ((u32)m_pMeshes[i].pVertData);

    //--- The same thing applies to the submeshes and the display lists
    for( i = 0; i < m_NSubMeshes; i++ )
        m_pSubMeshes[i].pDispList = m_pDispListData + ((u32)m_pSubMeshes[i].pDispList);


    //--- For the display lists to work, they must be invalidated from the CPU cache
    DCFlushRange( m_pDispListData, m_DispListDataSize );
    DCFlushRange( m_pVertexData,   m_VertexDataSize );

    // create the mesh texture array (for overriding a meshes default texture)
    m_MeshTextureArray = (x_bitmap**)x_malloc( sizeof(x_bitmap*) * m_NMeshes );
    ASSERT( m_MeshTextureArray != NULL );
    x_memset( m_MeshTextureArray, NULL, sizeof(x_bitmap*) * m_NMeshes );
}

//==========================================================================

static f32 CalcScaleMagnitude( matrix4& L2W )
{
    s32 LargestScaleAxis;
    f32 Scale[3];

    //--- calculate the scale on each axis without the square root
    Scale[0] = L2W.M[0][0] * L2W.M[0][0] +
               L2W.M[0][1] * L2W.M[0][1] +
               L2W.M[0][2] * L2W.M[0][2];
    Scale[1] = L2W.M[1][0] * L2W.M[1][0] +
               L2W.M[1][1] * L2W.M[1][1] +
               L2W.M[1][2] * L2W.M[1][2];
    Scale[2] = L2W.M[2][0] * L2W.M[2][0] +
               L2W.M[2][1] * L2W.M[2][1] +
               L2W.M[2][2] * L2W.M[2][2];

    //--- now we can figure out which axis has the largest scale
    LargestScaleAxis = 0;
    if ( Scale[1] > Scale[LargestScaleAxis] )
        LargestScaleAxis = 1;
    if ( Scale[2] > Scale[LargestScaleAxis] )
        LargestScaleAxis = 2;

    //--- apply the square root to get the proper magnitude
    return x_sqrt( Scale[LargestScaleAxis] );
}

//==========================================================================

static void ASM_MtxMultFast( register matrix4* pDest, register matrix4* pSrc1, register matrix4* pSrc2 )
{
    //Assuming that the last math-row has nothing in it(0,0,0,1) for optimization
    asm __volatile__
    ("
        li            0,       0                # r0 = 0
        mtspr      GQR7,       0                # set GQR7 to zero for f32's
        psq_l       4  ,   0(%2),   0 ,  7      # f4  = (M2[0][0], M2[0][1])
        psq_l       5  ,   8(%2),   1 ,  7      # f5  = (M2[0][2], --------)
        psq_l       6  ,   0(%1),   0 ,  7      # f6  = (M1[0][0], M1[0][1])
        psq_l       7  ,   8(%1),   1 ,  7      # f7  = (M1[0][2], --------)
        ps_muls0    0  ,      6 ,   4           # f0  = (M1[0][0], M1[0][1]) * (M2[0][0])
        psq_l       8  ,  16(%1),   0 ,  7      # f8  = (M1[1][0], M1[1][1])
        ps_muls0    1  ,      7 ,   4           # f1  = (M1[0][2], --------) * (M2[0][0])
        psq_l       9  ,  24(%1),   1 ,  7      # f9  = (M1[1][2], --------)
        ps_madds1   0  ,      8 ,   4 ,  0      # f0 += (M1[1][0], M1[1][1]) * (M2[0][1])
        psq_l       10 ,  32(%1),   0 ,  7      # f10 = (M1[2][0], M1[2][1])
        ps_madds1   1  ,      9 ,   4 ,  1      # f1 += (M1[1][2], --------) * (M2[0][1])
        psq_l       11 ,  40(%1),   1 ,  7      # f11 = (M1[2][2], --------)
        ps_madds0   0  ,      10,   5 ,  0      # f0 += (M1[2][0], M1[2][1]) * (M2[0][2])
        psq_l       12 ,  16(%2),   0 ,  7      # f12 = (M2[1][0], M2[1][1])
        ps_madds0   1  ,      11,   5 ,  1      # f1 += (M1[2][2], --------) * (M2[0][2])
        psq_st      0  ,   0(%0),   0 ,  7      # store (MD[0][0], MD[0][1])
        ps_muls0    2  ,      6 ,  12           # f2  = (M1[0][0], M1[0][1]) * (M2[1][0])
        psq_st      1  ,   8(%0),   1 ,  7      # store (MD[0][2], --------)
        ps_muls0    3  ,      7 ,  12           # f3  = (M1[0][2], --------) * (M2[1][0])
        psq_l       13 ,  24(%2),   1 ,  7      # f13 = (M2[1][2], --------)
        ps_madds1   2  ,      8 ,  12 ,  2      # f2 += (M1[1][0], M1[1][1]) * (M2[1][1])
        psq_l       4  ,  32(%2),   0 ,  7      # f4  = (M2[2][0], M2[2][1])
        ps_madds1   3  ,      9 ,  12 ,  3      # f3 += (M1[1][2], --------) * (M2[1][1])
        psq_l       5  ,  40(%2),   1 ,  7      # f5  = (M2[2][2], --------)
        ps_madds0   2  ,      10,  13 ,  2      # f2 += (M1[2][0], M1[2][1]) * (M2[1][2])
        ps_muls0    0  ,      6 ,   4           # f0  = (M1[0][0], M1[0][1]) * (M2[2][0])
        psq_st      2  ,  16(%0),   0 ,  7      # store (MD[1][0], MD[1][1])
        ps_madds0   3  ,      11,  13 ,  3      # f3 += (M1[2][2], --------) * (M2[1][2])
        ps_muls0    1  ,      7 ,   4           # f1  = (M1[0][2], --------) * (M2[2][0])
        psq_st      3  ,  24(%0),   1 ,  7      # store (MD[1][2], --------)
        ps_madds1   0  ,      8 ,   4 ,  0      # f0 += (M1[1][0], M1[1][1]) * (M2[2][1])
        ps_madds1   1  ,      9 ,   4 ,  1      # f1 += (M1[1][2], --------) * (M2[2][1])
        psq_l       2  ,  48(%1),   0 ,  7      # f2  = (M1[3][0], M1[3][1])
        ps_madds0   0  ,      10,   5 ,  0      # f0 += (M1[2][0], M1[2][1]) * (M2[2][2])
        psq_l       12 ,  48(%2),   0 ,  7      # f12 = (M2[3][0], M2[3][1])
        ps_madds0   1  ,      11,   5 ,  1      # f1 += (M1[2][2], --------) * (M2[2][2])
        psq_l       3  ,  56(%1),   1 ,  7      # f3  = (M1[3][2], --------)
        ps_madds0   2  ,      6 ,  12 ,  2      # f2 += (M1[0][0], M1[0][1]) * (M2[3][0])
        psq_l       13 ,  56(%2),   1 ,  7      # f13 = (M2[3][2], --------)
        ps_madds0   3  ,      7 ,  12 ,  3      # f3 += (M1[0][2], --------) * (M2[3][0])
        ps_madds1   2  ,      8 ,  12 ,  2      # f2 += (M1[1][0], M1[1][1]) * (M2[3][1])
        psq_st      0  ,  32(%0),   0 ,  7      # store (MD[2][0], MD[2][1])
        ps_madds1   3  ,      9 ,  12 ,  3      # f3 += (M1[1][2], --------) * (M2[3][1])
        ps_madds0   2  ,      10,  13 ,  2      # f2 += (M1[2][0], M1[2][1]) * (M2[3][2])
        psq_st      1  ,  40(%0),   1 ,  7      # store (MD[2][2], --------)
        psq_st      2  ,  48(%0),   0 ,  7      # store (MD[3][0], MD[3][1])
        ps_madds0   3  ,      11,  13 ,  3      # f3 += (M1[2][2], --------) * (M2[3][2])
        psq_st      3  ,  56(%0),   1 ,  7      # store (MD[3][2], --------)
    "
    :
    : "b" (pDest), "b" (pSrc1), "b" (pSrc2)
    : "fr0", "fr1", "fr2", "fr3", "fr4", "fr5", "fr6", "fr7", "fr8", "fr9", "fr10", "fr11", "fr12", "fr13"
    );
}

//==========================================================================

void QGeom::PreRender( matrix4* pShadowMatrix )
{
    matrix4 W2V;
    matrix4 L2V;
    Mtx     GC_L2V;
    Mtx     GC_NRM;
    Mtx     GC_TEXMTX;

    ENG_GetActiveView()->GetW2VMatrix( W2V );

    //--- apply shadow matrix if supplied
    if( pShadowMatrix != NULL )
    {
        // CPU multiply:
        //L2V = (*pShadowMatrix) * m_L2W;
        //L2V = W2V * L2V;
        ASM_MtxMultFast( &L2V, pShadowMatrix, &m_L2W );
        ASM_MtxMultFast( &L2V, &W2V, &L2V );
    }
    else
    {
        // CPU multiply:
        //L2V = W2V * m_L2W;
        ASM_MtxMultFast( &L2V, &W2V, &m_L2W );
    }

    //--- rotate 180 degrees for gamecube viewing(done by negating rows 0 & 2),
    //    and copy matrix data into gamecube specific matrix
    MTXRowCol( GC_L2V, 0, 0 ) = -L2V.M[0][0];
    MTXRowCol( GC_L2V, 1, 0 ) =  L2V.M[0][1];
    MTXRowCol( GC_L2V, 2, 0 ) = -L2V.M[0][2];
    MTXRowCol( GC_L2V, 0, 1 ) = -L2V.M[1][0];
    MTXRowCol( GC_L2V, 1, 1 ) =  L2V.M[1][1];
    MTXRowCol( GC_L2V, 2, 1 ) = -L2V.M[1][2];
    MTXRowCol( GC_L2V, 0, 2 ) = -L2V.M[2][0];
    MTXRowCol( GC_L2V, 1, 2 ) =  L2V.M[2][1];
    MTXRowCol( GC_L2V, 2, 2 ) = -L2V.M[2][2];
    MTXRowCol( GC_L2V, 0, 3 ) = -L2V.M[3][0];
    MTXRowCol( GC_L2V, 1, 3 ) =  L2V.M[3][1];
    MTXRowCol( GC_L2V, 2, 3 ) = -L2V.M[3][2];

    PSMTXInvXpose( GC_L2V, GC_NRM );

    //--- G_L2V can also be used for the normal matrix(unless there's non-uniform scaling)
    GXLoadNrmMtxImm( GC_NRM, GX_PNMTX0 );
    GXLoadPosMtxImm( GC_L2V, GX_PNMTX0 );
    GXSetCurrentMtx( GX_PNMTX0 );

    //--- setup environment-map matrix
    MTXRowCol( GC_TEXMTX, 0, 0 ) = m_L2W.M[0][0];
    MTXRowCol( GC_TEXMTX, 1, 0 ) = m_L2W.M[0][1];
    MTXRowCol( GC_TEXMTX, 2, 0 ) = m_L2W.M[0][2];
    MTXRowCol( GC_TEXMTX, 0, 1 ) = m_L2W.M[1][0];
    MTXRowCol( GC_TEXMTX, 1, 1 ) = m_L2W.M[1][1];
    MTXRowCol( GC_TEXMTX, 2, 1 ) = m_L2W.M[1][2];
    MTXRowCol( GC_TEXMTX, 0, 2 ) = m_L2W.M[2][0];
    MTXRowCol( GC_TEXMTX, 1, 2 ) = m_L2W.M[2][1];
    MTXRowCol( GC_TEXMTX, 2, 2 ) = m_L2W.M[2][2];
    MTXRowCol( GC_TEXMTX, 0, 3 ) = 0;
    MTXRowCol( GC_TEXMTX, 1, 3 ) = 0;
    MTXRowCol( GC_TEXMTX, 2, 3 ) = 0;

    Mtx MR;
    MTXRotDeg( MR, 'X', 180 );
    PSMTXConcat( MR, GC_TEXMTX, GC_TEXMTX );

    PSMTXInvXpose( GC_TEXMTX, GC_TEXMTX );

    GXLoadTexMtxImm( GC_TEXMTX, GX_TEXMTX0, GX_MTX3x4 );
}

//==========================================================================

void QGeom::RenderMesh( s32 MeshID, xbool SkipVisCheck )
{
    s32         SubMeshID;
    s32         LastSubMeshID;
    t_GeomMesh* pMesh;
    view*       pView;
    vector3     Location;
    x_bitmap*   pTexture;
    s32         bIsDXT3;

    ASSERT( ENG_GetRenderMode() );
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );

    pMesh = &m_pMeshes[MeshID];

    //--- perform trivial rejections first
    if( !(pMesh->Flags & MESH_FLAG_VISIBLE) && !SkipVisCheck )
        return;

    if( pMesh->Flags & MESH_FLAG_SHADOW )
        return; // You should use QGeom::RenderMeshAsShadow in this case!

    //--- get visibility info
    pView    = ENG_GetActiveView();
    Location = m_L2W.GetTranslation();

    if( SkipVisCheck == FALSE )
    {
        f32 Radius = pMesh->Radius * CalcScaleMagnitude( m_L2W );

        if( !pView->SphereInView( view::V_WORLD, Location, Radius ) )
            return;
    }

    //--- Do pre-rendering setup
    PreRender();

    //--- setup the vertex format
    if( pMesh->VertFormat == GC_VTXFMT_NORMAL )
    {
        SET_GC_VTXFMT_NORMAL();
        SET_GC_VTX_ARRAY_NORMAL( pMesh->pVertData );
    }
    else if( pMesh->VertFormat == GC_VTXFMT_COLOR )
    {
        SET_GC_VTXFMT_COLOR();
        SET_GC_VTX_ARRAY_COLOR( pMesh->pVertData );
    }
    else
    {
        ASSERTS( FALSE, "Invalid vertex format" );
    }

    //--- set up tev for dynamic lighting
    if( pMesh->Flags & MESH_FLAG_DYNAMIC_LIGHTING )
    {
        GC_EnableLighting( TRUE, FALSE );
    }
    else
    {
        GC_EnableLighting( FALSE, FALSE );
    }


    //--- set up tev for environment mapping
    if( pMesh->Flags & MESH_FLAG_ENVMAPPED )
    {
        ASSERTS( !(m_Flags & GEOM_FLAG_OWNS_TEXTURES), "You must supply the textures for an env. mapped object!" );

        //--- set up the texgen matrix for env. mapping
        Mtx EnvXForm;
        Mtx EnvXFormS;
        Mtx EnvXFormT;
        MTXScale( EnvXFormS, 0.5f, -0.5f, 0.0f );
        MTXTrans( EnvXFormT, 0.5f, 0.5f, 1.0f );
        MTXConcat( EnvXFormT, EnvXFormS, EnvXForm );
        GXLoadTexMtxImm( EnvXForm, GX_PTTEXMTX0, GX_MTX3x4 );

        if( m_pTextures != NULL )
        {
            VRAM_GC_Activate( m_pTextures[m_NTextures], 1 );
        }
        else if( m_pTexturePtrs )
        {
            VRAM_GC_Activate( *m_pTexturePtrs[m_NTextures], 1 );
        }

        SET_TEV_ENV_MAP();
    }
    else
    {
        SET_TEV_ONE_TEXTURE();
    }

    bIsDXT3 = GC_GetDXT3AlphaMapStage( 0 );


    LastSubMeshID = pMesh->FirstSubMesh + pMesh->NSubMeshes;

    //--- loop through the sub-meshes
    for( SubMeshID = pMesh->FirstSubMesh; SubMeshID < LastSubMeshID; SubMeshID++ )
    {
        //--- activate the texture for this submesh
        if( m_pSubMeshes[SubMeshID].TextureID >= 0 )
        {
            //--- get a pointer to the texture(s)
            if( m_pTextures )
                pTexture = &m_pTextures[ m_pSubMeshes[SubMeshID].TextureID ];
            else if( m_pTexturePtrs )
                pTexture = m_pTexturePtrs[ m_pSubMeshes[SubMeshID].TextureID ];
            else
                pTexture = NULL;

            //--- activate the texture
            ASSERT( pTexture != NULL );

            VRAM_GC_Activate( *pTexture, 0 );
        }

        if( bIsDXT3 != GC_GetDXT3AlphaMapStage( 0 ) )
        {
            bIsDXT3 = GC_GetDXT3AlphaMapStage( 0 );

            if( pMesh->Flags & MESH_FLAG_ENVMAPPED )
                SET_TEV_ENV_MAP();
            else
                SET_TEV_ONE_TEXTURE();
        }

        //--- Execute the display list for the submesh
        ASSERT( ((u32)m_pSubMeshes[SubMeshID].pDispList & 0x1F) == 0 );
        GXCallDisplayList( m_pSubMeshes[SubMeshID].pDispList, m_pSubMeshes[SubMeshID].DispListSize );

        //--- Update geom stats
        if( s_pStatNVerts ) *s_pStatNVerts += m_pSubMeshes[SubMeshID].NVerts;
        if( s_pStatNTris )  *s_pStatNTris  += m_pSubMeshes[SubMeshID].NTris;
        //if( s_pStatNBytes )   *s_pStatNBytes += m_pSubMeshes[SubMeshID].???;
    }
}

//==========================================================================

void QGeom::Render( xbool SkipVisCheck )
{
    s32         MeshID;
    s32         SubMeshID;
    s32         LastSubMeshID;
    t_GeomMesh* pMesh;
    view*       pView;
    vector3     Location;
    x_bitmap*   pTexture;
    f32         RadiusScale;
    s32         CurVtxFormat     = GC_VTXFMT_NONE;
    xbool       bTexMatrixIsSent = FALSE;
    s32         bIsDXT3;

    ASSERT( ENG_GetRenderMode() );

    //--- Do pre-rendering setup
    PreRender();

    //--- get visibility info
    pView    = ENG_GetActiveView();
    Location = m_L2W.GetTranslation();

    //--- find mesh radius scale
    if( SkipVisCheck == FALSE )
    {
        RadiusScale = CalcScaleMagnitude( m_L2W );
    }

    for( MeshID = 0; MeshID < m_NMeshes; MeshID++ )
    {
        pMesh = &m_pMeshes[MeshID];

        //--- perform trivial rejections first
        if( !(pMesh->Flags & MESH_FLAG_VISIBLE) && !SkipVisCheck )
            continue;

        if( pMesh->Flags & MESH_FLAG_SHADOW )
            continue; // You should use QGeom::RenderMeshAsShadow in this case!

        if( SkipVisCheck == FALSE )
        {
            f32 Radius = pMesh->Radius * CalcScaleMagnitude( m_L2W );

            if( !pView->SphereInView( view::V_WORLD, Location, Radius ) )
                continue;
        }

        //--- setup the vertex format
        if( CurVtxFormat != pMesh->VertFormat )
        {
            CurVtxFormat = pMesh->VertFormat;

            if( pMesh->VertFormat == GC_VTXFMT_NORMAL )
            {
                SET_GC_VTXFMT_NORMAL();
            }
            else if( pMesh->VertFormat == GC_VTXFMT_COLOR )
            {
                SET_GC_VTXFMT_COLOR();
            }
            else
            {
                ASSERTS( FALSE, "Invalid vertex format" );
            }
        }

        //--- set pointers to vertex data
        if( pMesh->VertFormat == GC_VTXFMT_NORMAL )
        {
            SET_GC_VTX_ARRAY_NORMAL( pMesh->pVertData );
        }
        else
        {
            SET_GC_VTX_ARRAY_COLOR( pMesh->pVertData );
        }

        //--- set up tev for dynamic lighting
        if( pMesh->Flags & MESH_FLAG_DYNAMIC_LIGHTING )
        {
            GC_EnableLighting( TRUE, FALSE );
        }
        else
        {
            GC_EnableLighting( FALSE, FALSE );
        }

        //--- set up tev for environment mapping
        if( pMesh->Flags & MESH_FLAG_ENVMAPPED )
        {
            ASSERTS( !(m_Flags & GEOM_FLAG_OWNS_TEXTURES), "You must supply the textures for an env. mapped object!" );

            if( bTexMatrixIsSent == FALSE )
            {
                //--- set up the texgen matrix for env. mapping
                Mtx EnvXForm;
                Mtx EnvXFormS;
                Mtx EnvXFormT;
                MTXScale( EnvXFormS, 0.5f, -0.5f, 0.0f );
                MTXTrans( EnvXFormT, 0.5f, 0.5f, 1.0f );
                MTXConcat( EnvXFormT, EnvXFormS, EnvXForm );
                GXLoadTexMtxImm( EnvXForm, GX_PTTEXMTX0, GX_MTX3x4 );
                bTexMatrixIsSent = TRUE;
            }

            if( m_pTextures != NULL )
            {
                VRAM_GC_Activate( m_pTextures[m_NTextures], 1 );
            }
            else if( m_pTexturePtrs )
            {
                VRAM_GC_Activate( *m_pTexturePtrs[m_NTextures], 1 );
            }

            SET_TEV_ENV_MAP();
        }
        else
        {
            SET_TEV_ONE_TEXTURE();
        }

        bIsDXT3 = GC_GetDXT3AlphaMapStage( 0 );

        LastSubMeshID = pMesh->FirstSubMesh + pMesh->NSubMeshes;

        //--- loop through the sub-meshes
        for( SubMeshID = pMesh->FirstSubMesh; SubMeshID < LastSubMeshID; SubMeshID++ )
        {
            //--- activate the texture for this submesh
            // check for a over ride texture and set if there
            if( m_MeshTextureArray[MeshID] != NULL )
            {
                VRAM_GC_Activate( *m_MeshTextureArray[MeshID], 0 );
            }
            else if( m_pSubMeshes[SubMeshID].TextureID >= 0 )
            {
                //--- get a pointer to the texture(s)
                if( m_pTextures )
                    pTexture = &m_pTextures[ m_pSubMeshes[SubMeshID].TextureID ];
                else if( m_pTexturePtrs )
                    pTexture = m_pTexturePtrs[ m_pSubMeshes[SubMeshID].TextureID ];
                else
                    pTexture = NULL;

                //--- activate the texture
                ASSERT( pTexture != NULL );

                VRAM_GC_Activate( *pTexture, 0 );
            }

            if( bIsDXT3 != GC_GetDXT3AlphaMapStage( 0 ) )
            {
                bIsDXT3 = GC_GetDXT3AlphaMapStage( 0 );

                if( pMesh->Flags & MESH_FLAG_ENVMAPPED )
                    SET_TEV_ENV_MAP();
                else
                    SET_TEV_ONE_TEXTURE();
            }

            //--- Execute the display list for the submesh
            ASSERT( ((u32)m_pSubMeshes[SubMeshID].pDispList & 0x1F) == 0 );
            GXCallDisplayList( m_pSubMeshes[SubMeshID].pDispList, m_pSubMeshes[SubMeshID].DispListSize );

            //--- Update geom stats
            if( s_pStatNVerts ) *s_pStatNVerts += m_pSubMeshes[SubMeshID].NVerts;
            if( s_pStatNTris )  *s_pStatNTris  += m_pSubMeshes[SubMeshID].NTris;
            //if( s_pStatNBytes )   *s_pStatNBytes += m_pSubMeshes[SubMeshID].???;
        }
    }
}

s32 QGeom::RenderSubMeshes( xbool *SubMeshes, xbool SkipVisCheck, s32 SubMeshFlags, xbool SkipInViewCheck )
{
    s32         SubMeshID;
    s32         CurrentMeshID = -1;
    t_GeomMesh* pMesh;
    view*       pView;
    vector3     Location;
    x_bitmap*   pTexture;
    u32         CurrentRenderFlags;
    u32         NewRenderFlags;
    s32         NSubMeshesRendered = 0;
    s32         bIsDXT3;
    s32         bIsEnvMapped;

    // Get the current blend modes and render settings from the engine.
    ENG_GetRenderFlags( CurrentRenderFlags );

    ASSERT( ENG_GetRenderMode() );

    bIsDXT3 = GC_GetDXT3AlphaMapStage( 0 );
	bIsEnvMapped = FALSE;

    for( SubMeshID = 0; SubMeshID < m_NSubMeshes; SubMeshID++ )
    {
        if( !SubMeshes[SubMeshID] )
            continue;

        if( SubMeshFlags == -1 )
            SubMeshes[SubMeshID] = FALSE;

        if( (SubMeshFlags == SUBMESH_FLAG_NONE) && (m_pSubMeshes[SubMeshID].Flags != SUBMESH_FLAG_NONE) )
            continue;

        if( (SubMeshFlags > SUBMESH_FLAG_NONE) && !(m_pSubMeshes[SubMeshID].Flags & SubMeshFlags) )
            continue;

        if( !SkipVisCheck && !SkipInViewCheck )
            if( !SubMeshInView( &m_pSubMeshes[SubMeshID] ) )
                continue;

        if( m_pSubMeshes[SubMeshID].MeshID != CurrentMeshID )
        {
            CurrentMeshID = m_pSubMeshes[SubMeshID].MeshID;

            pMesh = &m_pMeshes[CurrentMeshID];

            //--- perform trivial rejections first
            if( !(pMesh->Flags & MESH_FLAG_VISIBLE) && !SkipVisCheck )
                continue;

            if( pMesh->Flags & MESH_FLAG_SHADOW )
                continue; // You should use QGeom::RenderMeshAsShadow in this case!

            //--- get visibility info
            pView    = ENG_GetActiveView();
            Location = m_L2W.GetTranslation();

            //--- Do pre-rendering setup
            PreRender();

            //--- setup the vertex format
            if( pMesh->VertFormat == GC_VTXFMT_NORMAL )
            {
                SET_GC_VTXFMT_NORMAL();
                SET_GC_VTX_ARRAY_NORMAL( pMesh->pVertData );
            }
            else if( pMesh->VertFormat == GC_VTXFMT_COLOR )
            {
                SET_GC_VTXFMT_COLOR();
                SET_GC_VTX_ARRAY_COLOR( pMesh->pVertData );
            }
            else
            {
                ASSERTS( FALSE, "Invalid vertex format" );
            }

            //--- set up tev for dynamic lighting
            if( pMesh->Flags & MESH_FLAG_DYNAMIC_LIGHTING )
            {
                GC_EnableLighting( TRUE, FALSE );
            }
            else
            {
                GC_EnableLighting( FALSE, FALSE );
            }


            //--- set up tev for environment mapping
            if( pMesh->Flags & MESH_FLAG_ENVMAPPED )
            {
                ASSERTS( !(m_Flags & GEOM_FLAG_OWNS_TEXTURES), "You must supply the textures for an env. mapped object!" );

                //--- set up the texgen matrix for env. mapping
                Mtx EnvXForm;
                Mtx EnvXFormS;
                Mtx EnvXFormT;
                MTXScale( EnvXFormS, 0.5f, -0.5f, 0.0f );
                MTXTrans( EnvXFormT, 0.5f, 0.5f, 1.0f );
                MTXConcat( EnvXFormT, EnvXFormS, EnvXForm );
                GXLoadTexMtxImm( EnvXForm, GX_PTTEXMTX0, GX_MTX3x4 );

                if( m_pTextures != NULL )
                {
                    VRAM_GC_Activate( m_pTextures[m_NTextures], 1 );
                }
                else if( m_pTexturePtrs )
                {
                    VRAM_GC_Activate( *m_pTexturePtrs[m_NTextures], 1 );
                }

                bIsEnvMapped = TRUE;
                SET_TEV_ENV_MAP();
            }
            else
            {
                bIsEnvMapped = FALSE;
                SET_TEV_ONE_TEXTURE();
            }
        }

        if( !(m_pMeshes[CurrentMeshID].Flags & MESH_FLAG_VISIBLE) && !SkipVisCheck )
            continue;

        SubMeshes[SubMeshID] = TRUE;
        NSubMeshesRendered++;

        if( m_pSubMeshes[SubMeshID].Flags )
        {
            NewRenderFlags = CurrentRenderFlags;
            NewRenderFlags &= ~ENG_ALPHA_TEST_ON;
            NewRenderFlags |= ENG_ALPHA_TEST_OFF;
            NewRenderFlags &= ~ENG_ALPHA_BLEND_ON;
            NewRenderFlags |= ENG_ALPHA_BLEND_OFF;
            if( m_pSubMeshes[SubMeshID].Flags & SUBMESH_FLAG_ALPHA_P )
            {
                // turn the alpha test mode on
                NewRenderFlags &= ~ENG_ALPHA_TEST_OFF;
                NewRenderFlags |= ENG_ALPHA_TEST_ON;
            }
            if( m_pSubMeshes[SubMeshID].Flags & SUBMESH_FLAG_ALPHA_T )
            {
                // turn the alpha blend mode on
                NewRenderFlags &= ~ENG_ALPHA_BLEND_OFF;
                NewRenderFlags |= ENG_ALPHA_BLEND_ON;
            }
            ENG_SetRenderFlags( NewRenderFlags );
        }

        //--- activate the texture for this submesh
        // check for a over ride texture and set if there
        if( m_MeshTextureArray[CurrentMeshID] != NULL )
        {
            VRAM_GC_Activate( *m_MeshTextureArray[CurrentMeshID], 0 );
        }
        else if( m_pSubMeshes[SubMeshID].TextureID >= 0 )
        {
            //--- get a pointer to the texture(s)
            if( m_pTextures )
                pTexture = &m_pTextures[ m_pSubMeshes[SubMeshID].TextureID ];
            else if( m_pTexturePtrs )
                pTexture = m_pTexturePtrs[ m_pSubMeshes[SubMeshID].TextureID ];
            else
                pTexture = NULL;

            //--- activate the texture
            ASSERT( pTexture != NULL );

            VRAM_GC_Activate( *pTexture, 0 );
        }

        if( bIsDXT3 != GC_GetDXT3AlphaMapStage( 0 ) )
        {
            bIsDXT3 = GC_GetDXT3AlphaMapStage( 0 );

            if( bIsEnvMapped )
                SET_TEV_ENV_MAP();
            else
                SET_TEV_ONE_TEXTURE();
        }

        //--- Execute the display list for the submesh
        ASSERT( ((u32)m_pSubMeshes[SubMeshID].pDispList & 0x1F) == 0 );
        GXCallDisplayList( m_pSubMeshes[SubMeshID].pDispList, m_pSubMeshes[SubMeshID].DispListSize );

        //--- Update geom stats
        if( s_pStatNVerts ) *s_pStatNVerts += m_pSubMeshes[SubMeshID].NVerts;
        if( s_pStatNTris )  *s_pStatNTris  += m_pSubMeshes[SubMeshID].NTris;
        //if( s_pStatNBytes )   *s_pStatNBytes += m_pSubMeshes[SubMeshID].???;

        if( m_pSubMeshes[SubMeshID].Flags ) ENG_SetRenderFlags( CurrentRenderFlags );  // reset display mode
    }
    return NSubMeshesRendered;
}

//==========================================================================

void QGeom::RenderMeshAsShadow( s32 MeshID, matrix4 ShadowMatrix, xbool SkipVisCheck )
{
    s32         SubMeshID;
    s32         LastSubMeshID;
    t_GeomMesh* pMesh;
    view*       pView;
    vector3     Location;

    ASSERT( ENG_GetRenderMode() );
    ASSERT( (MeshID >= 0) && (MeshID < m_NMeshes) );

    //--- get pointer to mesh
    pMesh = &m_pMeshes[MeshID];
    ASSERT( pMesh->Flags & MESH_FLAG_SHADOW );

    //--- perform trivial rejections first
    if( !(pMesh->Flags & MESH_FLAG_VISIBLE) )
        return;

    //--- get visibility info
    pView = ENG_GetActiveView();
    Location = m_L2W.GetTranslation();

    if( SkipVisCheck == FALSE )
    {
        f32 Radius = pMesh->Radius * CalcScaleMagnitude( m_L2W );

        if( !pView->SphereInView( view::V_WORLD, Location, Radius ) )
            return;
    }

    //--- Do pre-rendering setup
    PreRender( &ShadowMatrix );

    //--- setup the vertex format
    SET_GC_VTXFMT_POS_ONLY( pMesh->pVertData );

    //--- disable lighting
    GC_EnableLighting( FALSE, FALSE );

    //--- Set TEV to not use texture, and use constant color
    xbool   FixAlphaOn;
    GXColor ShadowClr = {0,0,0,0};

    GC_GetFixedAlpha( FixAlphaOn, ShadowClr.a );
    if( !FixAlphaOn )
        ShadowClr.a = 255;

    GXSetNumChans    ( 1 );
    GXSetNumTevStages( 1 );
    GXSetNumTexGens  ( 0 );
    GXSetTevOrder    ( GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0 );

    GXSetTevColor  ( GX_TEVREG0, ShadowClr );
    GXSetTevColorIn( GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,    GX_CC_C0 );
    GXSetTevColorOp( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );
    GXSetTevAlphaIn( GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,    GX_CA_A0 );
    GXSetTevAlphaOp( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );

    LastSubMeshID = pMesh->FirstSubMesh + pMesh->NSubMeshes;

    //--- loop through the sub-meshes
    for( SubMeshID = pMesh->FirstSubMesh; SubMeshID < LastSubMeshID; SubMeshID++ )
    {
        //--- Execute the display list for the submesh
        ASSERT( ((u32)m_pSubMeshes[SubMeshID].pDispList & 0x1F) == 0 );
        GXCallDisplayList( m_pSubMeshes[SubMeshID].pDispList, m_pSubMeshes[SubMeshID].DispListSize );

        //--- Update geom stats
        if( s_pStatNVerts ) *s_pStatNVerts += m_pSubMeshes[SubMeshID].NVerts;
        if( s_pStatNTris )  *s_pStatNTris  += m_pSubMeshes[SubMeshID].NTris;
        //if( s_pStatNBytes )   *s_pStatNBytes += m_pSubMeshes[SubMeshID].???;
    }
}

//==========================================================================

void QGeom::LockMesh( s32 MeshID )
{
    ASSERT( ( MeshID >= 0) && (MeshID < m_NMeshes ) );
    ASSERT( !(m_pMeshes[MeshID].Flags & MESH_FLAG_LOCKED)  );

    m_pMeshes[MeshID].Flags |= MESH_FLAG_LOCKED;
}

//==========================================================================

void QGeom::UnlockMesh( s32 MeshID )
{
    ASSERT( ( MeshID >= 0) && (MeshID < m_NMeshes ) );
    ASSERT( m_pMeshes[MeshID].Flags & MESH_FLAG_LOCKED );

    m_pMeshes[MeshID].Flags &= ~MESH_FLAG_LOCKED;

    //--- calc amount of data to flush from CPU cache
    u32 vtxDataSize = m_pMeshes[MeshID].NVerts;

    if( m_pMeshes[MeshID].VertFormat == GC_VTXFMT_POS_ONLY )
    {
        vtxDataSize *= sizeof(vector3);
    }
    else if( m_pMeshes[MeshID].VertFormat == GC_VTXFMT_NORMAL )
    {
        vtxDataSize *= sizeof(t_GCVertNormal);
    }
    else if( m_pMeshes[MeshID].VertFormat == GC_VTXFMT_COLOR )
    {
        vtxDataSize *= sizeof(t_GCVertColor);
    }

    //--- must make sure data is out of CPU cache before sending to GP
    DCStoreRange( m_pMeshes[MeshID].pVertData, vtxDataSize );
}

//==========================================================================

xbool QGeom::SubMeshInView( SGeomSubMesh *pSubMesh )
{
    s32     i;
    view*   pView;
    matrix4 W2V;
    f32     FX,FY;
    f32     NZ,FZ;
    vector3 PlaneN[6];
    f32     PlaneD[6];

    // Get view info
    pView = ENG_GetActiveView();
    pView->GetFrustum( FX, FY );
    pView->GetW2VMatrix( W2V );
    pView->GetZLimits( NZ, FZ );
    pView->GetViewPlanes( view::V_WORLD, PlaneN[3], PlaneD[3],
                                         PlaneN[4], PlaneD[4],
                                         PlaneN[1], PlaneD[1],
                                         PlaneN[2], PlaneD[2],
                                         PlaneN[0], PlaneD[0],
                                         PlaneN[5], PlaneD[5]);

    ////////////////////////////////////////////////////////////////////
    // Check if midpt is in view
    ////////////////////////////////////////////////////////////////////
    vector4     V;

    ////////////////////////////////////////////////////////////////
    // Convert pt into camera space
    ////////////////////////////////////////////////////////////////

    // CPU version:
    V.X  = (W2V.M[0][0]*pSubMesh->MidPoint.X) + (W2V.M[1][0]*pSubMesh->MidPoint.Y) + (W2V.M[2][0]*pSubMesh->MidPoint.Z) + W2V.M[3][0];
    V.Y  = (W2V.M[0][1]*pSubMesh->MidPoint.X) + (W2V.M[1][1]*pSubMesh->MidPoint.Y) + (W2V.M[2][1]*pSubMesh->MidPoint.Z) + W2V.M[3][1];
    V.Z  = (W2V.M[0][2]*pSubMesh->MidPoint.X) + (W2V.M[1][2]*pSubMesh->MidPoint.Y) + (W2V.M[2][2]*pSubMesh->MidPoint.Z) + W2V.M[3][2];

    if((  V.Z      >= NZ  ) &&
       (  V.Z      <= FZ  ) &&
       (  V.X * FX <= V.Z ) && 
       ( -V.X * FX <= V.Z ) && 
       (  V.Y * FY <= V.Z ) && 
       ( -V.Y * FY <= V.Z ))
    {
        return TRUE;;
    }

    for( i=0; i<6; i++ )
    {
        vector3 BestPt;
        vector3 WorstPt;

        BestPt  = pSubMesh->Corner;
        WorstPt = pSubMesh->Corner;

        if( Dot(pSubMesh->Axis[0],PlaneN[i]) > 0 )
            BestPt  += pSubMesh->Axis[0];
        else
            WorstPt += pSubMesh->Axis[0];

        if( Dot(pSubMesh->Axis[1],PlaneN[i]) > 0 )
            BestPt  += pSubMesh->Axis[1];
        else
            WorstPt += pSubMesh->Axis[1];

        if( Dot(pSubMesh->Axis[2],PlaneN[i]) > 0 )
            BestPt  += pSubMesh->Axis[2];
        else
            WorstPt += pSubMesh->Axis[2];

        // Check if best point is outside
        if( (Dot(PlaneN[i],BestPt) + PlaneD[i]) < 0 )
        {
            return FALSE;
        }

    }
    return TRUE;
}