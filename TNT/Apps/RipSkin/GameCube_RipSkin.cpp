////////////////////////////////////////////////////////////////////////////
//
// GameCube_RipSkin.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>

#include "x_files.hpp"

#include "GameCube_RipSkin.h"
#include "GameCube/GC_Skin.hpp"

#include "Strip.h"
#include "GC_TriStrip.h"



//#define DOLPHIN_SDK_INSTALLED

#if defined( DOLPHIN_SDK_INSTALLED )
    #define __TYPES_H__                   // To remove duplicate typedefs of basic types
    #include <dolphin/gx/GXEnum.h>        // for GX_VTXFMT0
    #include <dolphin/gx/GXCommandList.h> // for GX_DRAW_* commands

#else
    // NOTE: these commands MUST match what's in GXEnum.h and GXCommandList.h
    #define GX_NOP                  0x00
    #define GX_DRAW_TRIANGLES       0x90
    #define GX_DRAW_TRIANGLE_STRIP  0x98

    #define GX_VTXFMT0              0x00

    #define GX_PNMTX0       0
    #define GX_PNMTX1       3
    #define GX_PNMTX2       6
    #define GX_PNMTX3       9
    #define GX_PNMTX4       12
    #define GX_PNMTX5       15
    #define GX_PNMTX6       18
    #define GX_PNMTX7       21
    #define GX_PNMTX8       24
    #define GX_PNMTX9       27
    #define GX_TEXMTX0      30
    #define GX_TEXMTX1      33
    #define GX_TEXMTX2      36
    #define GX_TEXMTX3      39
    #define GX_TEXMTX4      42
    #define GX_TEXMTX5      45
    #define GX_TEXMTX6      48
    #define GX_TEXMTX7      51
    #define GX_TEXMTX8      54
    #define GX_TEXMTX9      57
    #define GX_IDENTITY     60

#endif // defined( DOLPHIN_SDK_INSTALLED )


////////////////////////////////////////////////////////////////////////////
// DEFINES AND CONSTANTS
////////////////////////////////////////////////////////////////////////////

#define INSIG_VALUE         0.03f       // minimum matrix weight value

#define EPSILON             0.0001f

#define MAX_SUBMESH_VERTS   65000

//#define DONT_USE_GP_XFORM             // define this if all verts should be XForm'd on CPU
//#define DONT_MIX_GP_CPU_XFORM         // define this if submeshes should only use CPU XForm if multi-bones/vtx exist


//--------------------------------------------------------------------------

#define GC_CMD_MTXLOAD_POS      0x20
#define GC_CMD_MTXLOAD_NRM      0x28
#define GC_CMD_MTXLOAD_TEX      0x30

#define GC_CMD_MTXSIZE_2x4      0x70
#define GC_CMD_MTXSIZE_3x3      0x84
#define GC_CMD_MTXSIZE_3x4      0xB0

enum GC_MTX_SLOT_ID
{
    GC_MTX_ID_0     = 0,
    GC_MTX_ID_1     = 1,
    GC_MTX_ID_2     = 2,
    GC_MTX_ID_3     = 3,
    GC_MTX_ID_4     = 4,
    GC_MTX_ID_5     = 5,
    GC_MTX_ID_6     = 6,
    GC_MTX_ID_7     = 7,
    GC_MTX_ID_8     = 8,
    GC_MTX_ID_9     = 9,

    GC_MTX_ID_MAX   = 10,
};

#define GC_GET_MTXROW_POSNRM(  MtxSlotID )  (GX_PNMTX0  + (MtxSlotID * 3))
#define GC_GET_MTXROW_TEXTURE( MtxSlotID )  (GX_TEXMTX0 + (MtxSlotID * 3))

#define GC_GET_MTXADDR_POS( MtxSlotID )     (GC_GET_MTXROW_POSNRM(  MtxSlotID ) * 4)
#define GC_GET_MTXADDR_NRM( MtxSlotID )     (GC_GET_MTXROW_POSNRM(  MtxSlotID ) * 3)
#define GC_GET_MTXADDR_TEX( MtxSlotID )     (GC_GET_MTXROW_TEXTURE( MtxSlotID ) * 4)



////////////////////////////////////////////////////////////////////////////
// STRUCTURES
////////////////////////////////////////////////////////////////////////////

#pragma pack(1)

struct SDListMtxLoad
{
    u8      LoadCmd;        // matrix load command(GC_CMD_MTXLOAD_*)
    u16     MtxIdx;         // index in matrix array to load
    u8      SizeCmd;        // matrix size command(GC_CMD_MTXSIZE_*)
    u8      MtxAddr;        // address in matrix memory to load to(use GC_GET_MTXADDR_*)
};

struct SDListDrawPrimHdr
{
    u8      DrawCmd;        // primitive draw command, prim type + vtxfmt ID
    u16     VtxCount;       // number of verts to draw with
};

struct SDListVert_Shadow
{
    u8      MtxIdx;         // matrix slot ID to XForm vert with
    u16     PosIdx;         // index into position array
    u8      ClrIdx;         // used because of a HW bug(see HW2 Errata, Matrix+Pos Idx Bug)
};

struct SDListVert_GP
{
    u8      MtxIdx;         // matrix slot ID to XForm vert with
    u16     PosIdx;         // index into position array
    u16     NrmIdx;         // index into normal array
    u16     UVIdx;          // index into UV array
};

struct SDListVert_GP_ENV
{
    u8      MtxIdx;         // matrix slot ID to XForm vert with
    u8      TexMtxIdx;      // texture matrix slot ID to XForm UVs(for env mapping)
    u16     PosIdx;         // index into position array
    u16     NrmIdx;         // index into normal array
    u16     UVIdx;          // index into UV array
};

struct SDListVert_CPU
{
    u16     PosIdx;         // index into position array
    u16     NrmIdx;         // index into normal array
    u16     UVIdx;          // index into UV array
};

#pragma pack()



////////////////////////////////////////////////////////////////////////////
// EXTERNS
////////////////////////////////////////////////////////////////////////////

extern void  PauseOnError( void );
extern char  g_StripFileName[];
extern char  g_TextureSeamFileName[];
extern char  g_BoneWeightingFileName[];
extern xbool g_Verbose;
extern xbool g_ReportPathDetails;


////////////////////////////////////////////////////////////////////////////
// STATIC VARIABLES
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// GAMECUBE ENDIAN SWAPPING
////////////////////////////////////////////////////////////////////////////

template< class TYPE >
inline TYPE GC_ENDIAN_16( TYPE V )
{
    u16 EndV = ( ((*(u16*)&(V)) >> 8) |
                 ((*(u16*)&(V)) << 8) );

    return *((TYPE*)&EndV);
}

//--------------------------------------------------------------------------

template< class TYPE >
inline TYPE GC_ENDIAN_32( TYPE V )
{
    u32 EndV = ( ((*(u32*)&(V)) >> 24) |
                 ((*(u32*)&(V)) << 24) |
                 (((*(u32*)&(V)) & 0x00FF0000) >> 8) |
                 (((*(u32*)&(V)) & 0x0000FF00) << 8) );

    return *((TYPE*)&EndV);
}


////////////////////////////////////////////////////////////////////////////
// INLINE UTILITIES
////////////////////////////////////////////////////////////////////////////

template< class TYPE >
inline xbool IS_ALIGNED_32( TYPE V )
{
    return (((u32)V & 0x1F) == 0) ? TRUE : FALSE;
}

//--------------------------------------------------------------------------

template< class PTYPE, class ITYPE >
inline void GrowDataArray( PTYPE*& rpT, ITYPE& rCurSize, s32 ElementGrowCount )
{
    rCurSize += (ITYPE)ElementGrowCount;
    rpT = (PTYPE*)x_realloc( rpT, sizeof(PTYPE) * rCurSize );
    ASSERT( rpT != NULL );
}


////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

QGameCubeRipSkin::QGameCubeRipSkin( void )
{
    m_pVert        = NULL;
    m_pTri         = NULL;
    m_pTexture     = NULL;
    m_pSubMesh     = NULL;
    m_pMesh        = NULL;
    m_pMorphDelta  = NULL;
    m_pMorphTarget = NULL;
    m_pBone        = NULL;
    m_pVtxData     = NULL;
    m_pDListData   = NULL;
    m_pDeltaData   = NULL;

    Clear();
}

//==========================================================================

QGameCubeRipSkin::~QGameCubeRipSkin( void )
{
    Clear();
}

//==========================================================================

void QGameCubeRipSkin::Clear( void )
{
    if( m_pVert        != NULL )    x_free( m_pVert );
    if( m_pTri         != NULL )    x_free( m_pTri );
    if( m_pTexture     != NULL )    x_free( m_pTexture );
    if( m_pSubMesh     != NULL )    x_free( m_pSubMesh );
    if( m_pMesh        != NULL )    x_free( m_pMesh );
    if( m_pMorphDelta  != NULL )    x_free( m_pMorphDelta );
    if( m_pMorphTarget != NULL )    x_free( m_pMorphTarget );
    if( m_pBone        != NULL )    x_free( m_pBone );
    if( m_pVtxData     != NULL )    x_free( m_pVtxData );
    if( m_pDListData   != NULL )    x_free( m_pDListData );
    if( m_pDeltaData   != NULL )    x_free( m_pDeltaData );

    m_pVert        = NULL;
    m_pTri         = NULL;
    m_pTexture     = NULL;
    m_pSubMesh     = NULL;
    m_pMesh        = NULL;
    m_pMorphDelta  = NULL;
    m_pMorphTarget = NULL;
    m_pBone        = NULL;
    m_pVtxData     = NULL;
    m_pDListData   = NULL;
    m_pDeltaData   = NULL;

    m_NVerts        = 0;
    m_NTris         = 0;
    m_NTextures     = 0;
    m_NSubmeshes    = 0;
    m_NMeshes       = 0;
    m_NMorphDeltas  = 0;
    m_NMorphTargets = 0;
    m_NBones        = 0;
    m_VtxDataSize   = 0;
    m_DListDataSize = 0;
    m_DeltaDataSize = 0;

    m_NVertsAllocated        = 0;
    m_NTrisAllocated         = 0;
    m_NTexturesAllocated     = 0;
    m_NSubmeshesAllocated    = 0;
    m_NMeshesAllocated       = 0;
    m_NMorphDeltasAllocated  = 0;
    m_NMorphTargetsAllocated = 0;
    m_NBonesAllocated        = 0;
    m_VtxDataSizeAllocated   = 0;
    m_DListDataSizeAllocated = 0;
    m_DeltaDataSizeAllocated = 0;
}

//==========================================================================

static void FillName( char* pName, char* pSrc, s32 NChars )
{
    s32 Length = x_strlen( pSrc );
    s32 Start;

    x_memset( pName, 0, NChars );

    if( Length < NChars )
        Start = 0;
    else
        Start = Length - NChars + 1;

    x_strcpy( pName, pSrc + Start );
}

//==========================================================================

static void GetTextureName( char* pTextureName, char* pFullPathName )
{
    char File[X_MAX_FNAME];

    x_splitpath( pFullPathName, NULL, NULL, File, NULL );

    FillName( pTextureName, File, SKIN_TEXTURE_NAME_LENGTH );
}

//==========================================================================

void QGameCubeRipSkin::SetSkinName( char* pName )
{
    FillName( m_Name, pName, SKIN_NAME_LENGTH );
}

//==========================================================================

void QGameCubeRipSkin::AddBone( char* pName )
{
    s32 i;
    char TempName[SKIN_BONE_NAME_LENGTH + 1];

    FillName( TempName, pName, SKIN_BONE_NAME_LENGTH );
    TempName[SKIN_BONE_NAME_LENGTH] = '\0';

    //--- If this bone is already in the list, don't add another one
    for( i = 0; i < m_NBones; i++ )
    {
        if( !x_stricmp( m_pBone[i].Name, TempName ) )
            return;
    }

    //--- Check if we need to allocate a new bone
    if( m_NBonesAllocated <= m_NBones )
        GrowDataArray( m_pBone, m_NBonesAllocated, 32 );

    FillName( m_pBone[m_NBones].Name, pName, SKIN_BONE_NAME_LENGTH );
    m_NBones++;
}

//==========================================================================

void QGameCubeRipSkin::NewTexture( char* pTexturePath, s32 Width, s32 Height )
{
    //--- Check if we need to allocate a new texture
    if( m_NTexturesAllocated <= m_NTextures )
        GrowDataArray( m_pTexture, m_NTexturesAllocated, 32 );

    //--- Add texture
    x_strcpy( m_pTexture[m_NTextures].Name, pTexturePath );
    m_pTexture[m_NTextures].Width  = Width;
    m_pTexture[m_NTextures].Height = Height;

    //--- Increment number of textures
    m_NTextures++;
}

//==========================================================================

void QGameCubeRipSkin::NewMesh( char* pMeshName, xbool EnvMapped, xbool Shadow, xbool Alpha )
{
    //--- Check if we need to allocate a new mesh
    if( m_NMeshesAllocated <= m_NMeshes )
        GrowDataArray( m_pMesh, m_NMeshesAllocated, 32 );

    //--- Add mesh
    FillName( m_pMesh[m_NMeshes].Name, pMeshName, SKIN_MESH_NAME_LENGTH );
    m_pMesh[m_NMeshes].bEnvMapped      = EnvMapped;
    m_pMesh[m_NMeshes].bShadow         = Shadow;
    m_pMesh[m_NMeshes].bAlpha          = Alpha;
    m_pMesh[m_NMeshes].FirstSubmesh    = -1;
    m_pMesh[m_NMeshes].NSubmeshes      = 0;
    m_pMesh[m_NMeshes].FirstVertID     = m_NVerts;
    m_pMesh[m_NMeshes].NVerts          = 0;
    m_pMesh[m_NMeshes].FirstTriID      = m_NTris;
    m_pMesh[m_NMeshes].NTris           = 0;
    m_pMesh[m_NMeshes].FirstMorphTgtID = m_NMorphTargets;
    m_pMesh[m_NMeshes].NMorphTargets   = 0;

    //--- Increment number of meshes
    m_NMeshes++;
}

//==========================================================================

void QGameCubeRipSkin::AddVert( t_RipVert& rRipVert )
{
    //--- Check if we need to allocate a new Vert
    if( m_NVertsAllocated <= m_NVerts )
        GrowDataArray( m_pVert, m_NVertsAllocated, 1024 );

    //--- Add vert to list
    m_pVert[m_NVerts] = rRipVert;
    m_NVerts++;
    m_pMesh[m_NMeshes - 1].NVerts++;
}

//==========================================================================

void QGameCubeRipSkin::AddTri( s32 Vert0, s32 Vert1, s32 Vert2, char* pTextureName )
{
    vector3 DV1,DV2;
    f32     Length;
    s32     TextureID;

    //--- get the verts in the RipSkin's offset by adding the mesh's starting vert
    Vert0 += m_pMesh[m_NMeshes-1].FirstVertID;
    Vert1 += m_pMesh[m_NMeshes-1].FirstVertID;
    Vert2 += m_pMesh[m_NMeshes-1].FirstVertID;

    //--- Check for a collapsed triangle, and don't add it if there is one
    if( (Vert0 == Vert1) || (Vert0 == Vert2) || (Vert1 == Vert2) )
        return;

    if( (m_pVert[Vert0].Pos == m_pVert[Vert1].Pos) ||
        (m_pVert[Vert0].Pos == m_pVert[Vert2].Pos) ||
        (m_pVert[Vert1].Pos == m_pVert[Vert2].Pos) )
    {
        return;
    }

    //--- Figure out texture id
    if( m_NTextures == 0 )
    {
        TextureID = -1;
    }
    else
    {
        for( TextureID = 0; TextureID < m_NTextures; TextureID++ )
        {
            if( x_stricmp( m_pTexture[TextureID].Name, pTextureName ) == 0 )
                break;
        }
        ASSERT( TextureID < m_NTextures );
    }

    //--- Do a check to see if this triangle is already in our list
    //    (this will take out double-sided polys)
    {
        #define SWAP_VERTS(a,b)  { temp = a; a = b; b = temp; }

        s32 i;
        s32 temp;
        s32 VertsA[3];

        //--- get the verts in sorted order
        VertsA[0] = Vert0;
        VertsA[1] = Vert1;
        VertsA[2] = Vert2;

        if( VertsA[0] > VertsA[1] )  SWAP_VERTS( VertsA[0], VertsA[1] );
        if( VertsA[1] > VertsA[2] )  SWAP_VERTS( VertsA[1], VertsA[2] );
        if( VertsA[0] > VertsA[1] )  SWAP_VERTS( VertsA[0], VertsA[1] );

        ASSERT( (VertsA[2] > VertsA[1]) && (VertsA[1] > VertsA[0]) );

        for( i = 0; i < m_NTris; i++ )
        {
            GCSkinTri* pT;
            s32        VertsB[3];

            //--- grab a ptr to the tri
            pT = &m_pTri[i];

            //--- if its using a different texture, its okay to add
            if( pT->TextureID != TextureID )
                continue;
            //--- if its part of a different mesh, its okay to add
            if( pT->MeshID != (m_NMeshes-1) )
                continue;

            //--- get pT's verts in sorted order
            VertsB[0] = pT->VertID[0];
            VertsB[1] = pT->VertID[1];
            VertsB[2] = pT->VertID[2];

            if( VertsB[0] > VertsB[1] )  SWAP_VERTS( VertsB[0], VertsB[1] );
            if( VertsB[1] > VertsB[2] )  SWAP_VERTS( VertsB[1], VertsB[2] );
            if( VertsB[0] > VertsB[1] )  SWAP_VERTS( VertsB[0], VertsB[1] );

            ASSERT( (VertsB[2] > VertsB[1]) && (VertsB[1] > VertsB[0]) );

            //--- now we can determine whether or not this triangle already
            //    exists in this mesh with this texture
            if( (VertsA[0] == VertsB[0]) &&
                (VertsA[1] == VertsB[1]) &&
                (VertsA[2] == VertsB[2]) )
            {
                //--- don't add a duplicate poly
                return;
            }
        }
    }

    //--- Check if we need to allocate new tris
    if( m_NTrisAllocated <= m_NTris )
        GrowDataArray( m_pTri, m_NTrisAllocated, 4096 );

    //--- Compute tri normal and area
    DV1 = m_pVert[Vert1].Pos - m_pVert[Vert0].Pos;
    DV2 = m_pVert[Vert2].Pos - m_pVert[Vert0].Pos;

    m_pTri[m_NTris].Normal = Cross( DV1, DV2 );
    Length = m_pTri[m_NTris].Normal.Length();
    ASSERT( Length > 0.0f );
    m_pTri[m_NTris].Area = Length * 0.5f;
    m_pTri[m_NTris].Normal *= (1.0f / Length);

    //--- Add tri to list
    m_pTri[m_NTris].VertID[0] = Vert0;
    m_pTri[m_NTris].VertID[1] = Vert1;
    m_pTri[m_NTris].VertID[2] = Vert2;
    m_pTri[m_NTris].MeshID    = m_NMeshes - 1;
    m_pTri[m_NTris].TextureID = TextureID;

    //--- increment the tri count ptr
    m_NTris++;
    m_pMesh[m_NMeshes - 1].NTris++;
}

//==========================================================================

void QGameCubeRipSkin::NewMorphTarget( char* pTargetName )
{
    //--- Check if we need to allocate a new morph target
    if( m_NMorphTargetsAllocated <= m_NMorphTargets )
        GrowDataArray( m_pMorphTarget, m_NMorphTargetsAllocated, 32 );

    //--- Add morph target
    FillName( m_pMorphTarget[m_NMorphTargets].Name, pTargetName, SKIN_MORPH_NAME_LENGTH );
    m_pMorphTarget[m_NMorphTargets].FirstDeltaID = m_NMorphDeltas;
    m_pMorphTarget[m_NMorphTargets].NDeltas      = 0;

    //--- Increment number of morph targets
    m_pMesh[m_NMeshes - 1].NMorphTargets++;
    m_NMorphTargets++;
}

//==========================================================================

void QGameCubeRipSkin::AddMorphDelta( s32 VertID, vector3 Delta )
{
    //--- Check if we need to allocate more deltas
    if( m_NMorphDeltasAllocated <= m_NMorphDeltas )
        GrowDataArray( m_pMorphDelta, m_NMorphDeltasAllocated, 128 );

    //--- Add the delta
    m_pMorphDelta[m_NMorphDeltas].VertID = m_pMesh[m_NMeshes-1].FirstVertID + VertID;
    m_pMorphDelta[m_NMorphDeltas].Delta  = Delta;
    
    //--- let the vert know it may be morphed
    m_pVert[m_pMorphDelta[m_NMorphDeltas].VertID].HasMorphDelta = TRUE;

    //--- Increment number of morph deltas
    m_pMorphTarget[m_NMorphTargets-1].NDeltas++;
    m_NMorphDeltas++;
}

//==========================================================================

void QGameCubeRipSkin::ForceSingleMatrix( s32 VertID )
{
    s32 i;
    s32 MaxIndex = 0;

    //---   find the matrix with the heighest weight
    for( i = 0; i < m_pVert[VertID].NMatrices; i++ )
    {
        if( m_pVert[VertID].MatrixWeight[i] > m_pVert[VertID].MatrixWeight[MaxIndex] )
            MaxIndex = i;
    }

    //--- force the vert to use the matrix with the highest weight
    m_pVert[VertID].MatrixID[0]     = m_pVert[VertID].MatrixID[MaxIndex];
    m_pVert[VertID].MatrixWeight[0] = 1.0f;
    m_pVert[VertID].NMatrices       = 1;
}

//==========================================================================

void QGameCubeRipSkin::ForceDoubleMatrix( s32 VertID )
{
    s32 MaxIndex0 = 0;
    s32 MaxIndex1 = 1;
    s32 TempIndices[MAX_NUM_MATRIX];
    f32 W, W0, W1;
    s32 i;

    //--- the MaxIndices start off as zero and one, but make sure that
    //    the weight of Index0 is greater than Index1
    if( m_pVert[VertID].MatrixWeight[MaxIndex0] < m_pVert[VertID].MatrixWeight[MaxIndex1] )
    {
        MaxIndex0 = 1;
        MaxIndex1 = 0;
    }

    //--- now find the two greatest weights
    for( i = 2; i < m_pVert[VertID].NMatrices; i++ )
    {
        W = m_pVert[VertID].MatrixWeight[i];

        if( W > m_pVert[VertID].MatrixWeight[MaxIndex0] )
        {
            //--- W is the biggest weight so far
            if( m_pVert[VertID].MatrixWeight[MaxIndex0] >
                m_pVert[VertID].MatrixWeight[MaxIndex1] )
            {
                //--- since MaxIndex0 will be re-assigned to a greater weight, see if
                //    its bigger than MaxIndex1, and re-assign that too, if we can
                MaxIndex1 = MaxIndex0;
            }
            MaxIndex0 = i;
        }
        else
        {
            //--- is W bigger than the second-biggest weight?
            if( W > m_pVert[VertID].MatrixWeight[MaxIndex1] )
                MaxIndex1 = i;
        }
    }

    //--- do some calculations to get the total weight back to 1.0
    W0 = m_pVert[VertID].MatrixWeight[MaxIndex0];
    W1 = m_pVert[VertID].MatrixWeight[MaxIndex1];
    W  = 1.0f / (W0+W1);
    W0 *= W;
    W1 *= W;

    //--- re-assign the new weights
    for( i = 0; i < m_pVert[VertID].NMatrices; i++ )
        TempIndices[i] = m_pVert[VertID].MatrixID[i];
    
    m_pVert[VertID].MatrixID[0]     = TempIndices[MaxIndex0];
    m_pVert[VertID].MatrixWeight[0] = W0;
    m_pVert[VertID].MatrixID[1]     = TempIndices[MaxIndex1];
    m_pVert[VertID].MatrixWeight[1] = W1;
    m_pVert[VertID].NMatrices       = 2;

    //--- sanity check
    for( i = 0; i < m_pVert[VertID].NMatrices - 1; i++ )
    {
        ASSERT( m_pVert[VertID].MatrixWeight[i] >=
                m_pVert[VertID].MatrixWeight[i + 1] );
    }
}

//==========================================================================

void QGameCubeRipSkin::ForceTripleMatrix( s32 VertID )
{
    xbool done = FALSE;
    s32   MaxIndices[MAX_NUM_MATRIX];
    s32   TempIndices[MAX_NUM_MATRIX];
    f32   W, W0, W1, W2;
    s32   i;
    s32   temp;

    //--- default the max indices
    for( i = 0; i < m_pVert[VertID].NMatrices; i++ )
        MaxIndices[i] = i;

    //--- now sort the indices based on their corresponding weight
    //    (just do a quick bubble sort)
    while( !done )
    {
        done = TRUE;
        for( i = 0; i < m_pVert[VertID].NMatrices - 1; i++ )
        {
            if( m_pVert[VertID].MatrixWeight[ MaxIndices[i + 1] ] > 
                m_pVert[VertID].MatrixWeight[ MaxIndices[i]     ] )
            {
                temp            = MaxIndices[i];
                MaxIndices[i]   = MaxIndices[i+1];
                MaxIndices[i+1] = temp;
                done = FALSE;
            }
        }
    }

    //--- sanity check to make sure its sorted correctly
    for( i = 0; i < m_pVert[VertID].NMatrices - 1; i++ )
    {
        ASSERT( m_pVert[VertID].MatrixWeight[MaxIndices[i]] >=
                m_pVert[VertID].MatrixWeight[MaxIndices[i + 1]] );
    }

    //--- do some calculations to get the total weight back to 1.0
    W0 = m_pVert[VertID].MatrixWeight[ MaxIndices[0] ];
    W1 = m_pVert[VertID].MatrixWeight[ MaxIndices[1] ];
    W2 = m_pVert[VertID].MatrixWeight[ MaxIndices[2] ];
    W  = 1.0f / (W0 + W1 + W2);
    W0 *= W;
    W1 *= W;
    W2 *= W;

    //--- re-assign the new weights
    for( i = 0; i < m_pVert[VertID].NMatrices; i++ )
        TempIndices[i] = m_pVert[VertID].MatrixID[i];
    
    m_pVert[VertID].MatrixID[0]     = TempIndices[MaxIndices[0]];
    m_pVert[VertID].MatrixWeight[0] = W0;
    m_pVert[VertID].MatrixID[1]     = TempIndices[MaxIndices[1]];
    m_pVert[VertID].MatrixWeight[1] = W1;
    m_pVert[VertID].MatrixID[2]     = TempIndices[MaxIndices[2]];
    m_pVert[VertID].MatrixWeight[2] = W2;
    m_pVert[VertID].NMatrices       = 3;

    //--- sanity check
    for( i = 0; i < m_pVert[VertID].NMatrices - 1; i++ )
    {
        ASSERT( m_pVert[VertID].MatrixWeight[i] >=
                m_pVert[VertID].MatrixWeight[i + 1] );
    }
}

//==========================================================================

static void RemoveInsigWeights( SRipVert* pVert )
{
    s32 i;
    s32 SrcID;
    s32 DstID;
    f32 TotalWeight;
    f32 W;

    //--- remove any insignificant weights in this vert
    for( i = 0; i < pVert->NMatrices; i++ )
    {
        if ( pVert->MatrixWeight[i] < INSIG_VALUE )
            pVert->MatrixWeight[i] = 0.0f;
    }

    for( i = pVert->NMatrices; i < MAX_NUM_MATRIX; i++ )
        pVert->MatrixWeight[i] = 0.0f;  // just being safe here...

    //--- re-count the weights
    SrcID = DstID = 0;
    TotalWeight = 0.0f;
    while( (DstID < MAX_NUM_MATRIX) && (SrcID < MAX_NUM_MATRIX) )
    {
        if( pVert->MatrixWeight[SrcID] > 0.0f )
        {
            pVert->MatrixWeight[DstID] = pVert->MatrixWeight[SrcID];
            pVert->MatrixID[DstID] = pVert->MatrixID[SrcID];
            TotalWeight += pVert->MatrixWeight[DstID];
            DstID++;
            i++;
        }

        SrcID++;
    }
    pVert->NMatrices = DstID;
    for( i = DstID; i < MAX_NUM_MATRIX; i++ )
        pVert->MatrixWeight[DstID] = 0.0f;

    //--- re-normalize the weights
    if( TotalWeight == 0.0f )
        W = 1.0f;
    else
        W = 1.0f / TotalWeight;

    for( i = 0; i < pVert->NMatrices; i++ )
        pVert->MatrixWeight[i] *= W;
}

//==========================================================================

void QGameCubeRipSkin::ForceSingleMatrix( void )
{
    s32 i;

    for( i = 0; i < m_NVerts; i++ )
    {
        RemoveInsigWeights( &m_pVert[i] );
        ForceSingleMatrix( i );
    }
}

//==========================================================================

void QGameCubeRipSkin::ForceDoubleMatrix( void )
{
    s32 i;

    for( i = 0; i < m_NVerts; i++ )
    {
        RemoveInsigWeights( &m_pVert[i] );
        if ( m_pVert[i].NMatrices >= 2 )
            ForceDoubleMatrix( i );
        else
            ForceSingleMatrix( i );
    }
}

//==========================================================================

void QGameCubeRipSkin::ForceTripleMatrix( void )
{
    s32 i;

    for( i = 0; i < m_NVerts; i++ )
    {
        RemoveInsigWeights( &m_pVert[i] );
        if( m_pVert[i].NMatrices >= 3 )
            ForceTripleMatrix( i );
        else if ( m_pVert[i].NMatrices == 2 )
            ForceDoubleMatrix( i );
        else
            ForceSingleMatrix( i );
    }
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void QGameCubeRipSkin::BuildSubMeshVtxDataShadow( GCRipSkinSubMesh* pSubMesh )
{
    s32          CurVert;
    GCSkinVert_P ShadowVert;

    //--- set submesh's offset into data array
    pSubMesh->OffsetVtxShadow = m_VtxDataSize;

    //--- convert data into quantized format, and convert endian-ness
    for( CurVert = 0; CurVert < pSubMesh->NVertsGP; CurVert++ )
    {
        //--- allocate more data if needed
        if( m_VtxDataSizeAllocated <= m_VtxDataSize )
            GrowDataArray( m_pVtxData, m_VtxDataSizeAllocated, (512*1024) );

        ShadowVert.PX = (s16)(pSubMesh->pVertListGP[CurVert].Pos.X * (1<<GC_FRAC_POS));
        ShadowVert.PY = (s16)(pSubMesh->pVertListGP[CurVert].Pos.Y * (1<<GC_FRAC_POS));
        ShadowVert.PZ = (s16)(pSubMesh->pVertListGP[CurVert].Pos.Z * (1<<GC_FRAC_POS));

        ShadowVert.PX = GC_ENDIAN_16( ShadowVert.PX );
        ShadowVert.PY = GC_ENDIAN_16( ShadowVert.PY );
        ShadowVert.PZ = GC_ENDIAN_16( ShadowVert.PZ );

        //--- copy converted data into final array, update array size
        *((GCSkinVert_P*)&m_pVtxData[m_VtxDataSize]) = ShadowVert;
        m_VtxDataSize += sizeof(GCSkinVert_P);
    }

    //--- since we want the vertex data to start on a 32B boundary, pad it
    m_VtxDataSize = (u32)ALIGN_32( m_VtxDataSize );

    if( m_VtxDataSizeAllocated <= m_VtxDataSize )
        GrowDataArray( m_pVtxData, m_VtxDataSizeAllocated, (512*1024) );
}

//==========================================================================

void QGameCubeRipSkin::BuildSubMeshVtxDataUV( GCRipSkinSubMesh* pSubMesh )
{
    s32           CurVert;
    s32           NVerts;
    GCSkinVert_UV TCoord;

    //--- set submesh's offset into data array
    pSubMesh->OffsetUVs = m_VtxDataSize;

    for( CurVert = 0; CurVert < pSubMesh->NVertsGP; CurVert++ )
    {
        //--- allocate more data if needed
        if( m_VtxDataSizeAllocated <= m_VtxDataSize )
            GrowDataArray( m_pVtxData, m_VtxDataSizeAllocated, (512*1024) );

        TCoord.U = (s16)(pSubMesh->pVertListGP[CurVert].UV.X * (1<<GC_FRAC_UV));
        TCoord.V = (s16)(pSubMesh->pVertListGP[CurVert].UV.Y * (1<<GC_FRAC_UV));

        TCoord.U = GC_ENDIAN_16( TCoord.U );
        TCoord.V = GC_ENDIAN_16( TCoord.V );

        //--- copy converted data into final array, update array size
        *((GCSkinVert_UV*)&m_pVtxData[m_VtxDataSize]) = TCoord;
        m_VtxDataSize += sizeof(GCSkinVert_UV);
    }

    NVerts = pSubMesh->NVertsCPU1 + pSubMesh->NVertsCPU2 + pSubMesh->NVertsCPU3;

    for( CurVert = 0; CurVert < NVerts; CurVert++ )
    {
        //--- allocate more data if needed
        if( m_VtxDataSizeAllocated <= m_VtxDataSize )
            GrowDataArray( m_pVtxData, m_VtxDataSizeAllocated, (512*1024) );

        TCoord.U = (s16)(pSubMesh->pVertListCPU[CurVert].UV.X * (1<<GC_FRAC_UV));
        TCoord.V = (s16)(pSubMesh->pVertListCPU[CurVert].UV.Y * (1<<GC_FRAC_UV));

        TCoord.U = GC_ENDIAN_16( TCoord.U );
        TCoord.V = GC_ENDIAN_16( TCoord.V );

        //--- copy converted data into final array, update array size
        *((GCSkinVert_UV*)&m_pVtxData[m_VtxDataSize]) = TCoord;
        m_VtxDataSize += sizeof(GCSkinVert_UV);
    }

    //--- since we want the vertex data to start on a 32B boundary, pad it
    m_VtxDataSize = (u32)ALIGN_32( m_VtxDataSize );

    if( m_VtxDataSizeAllocated <= m_VtxDataSize )
        GrowDataArray( m_pVtxData, m_VtxDataSizeAllocated, (512*1024) );
}

//==========================================================================

void QGameCubeRipSkin::BuildSubMeshVtxDataGP( GCRipSkinSubMesh* pSubMesh )
{
    s32           CurVert;
    GCSkinVert_PN PNVert;

    //--- set submesh's offset into data array
    pSubMesh->OffsetVtxGP = m_VtxDataSize;

    //--- convert data into quantized format, and convert endian-ness
    for( CurVert = 0; CurVert < pSubMesh->NVertsGP; CurVert++ )
    {
        //--- allocate more data if needed
        if( m_VtxDataSizeAllocated <= m_VtxDataSize )
            GrowDataArray( m_pVtxData, m_VtxDataSizeAllocated, (512*1024) );

        PNVert.PX = (s16)(pSubMesh->pVertListGP[CurVert].Pos.X * (1<<GC_FRAC_POS));
        PNVert.PY = (s16)(pSubMesh->pVertListGP[CurVert].Pos.Y * (1<<GC_FRAC_POS));
        PNVert.PZ = (s16)(pSubMesh->pVertListGP[CurVert].Pos.Z * (1<<GC_FRAC_POS));

        PNVert.NX = (s16)(pSubMesh->pVertListGP[CurVert].Normal.X * (1<<GC_FRAC_NRM));
        PNVert.NY = (s16)(pSubMesh->pVertListGP[CurVert].Normal.Y * (1<<GC_FRAC_NRM));
        PNVert.NZ = (s16)(pSubMesh->pVertListGP[CurVert].Normal.Z * (1<<GC_FRAC_NRM));

        PNVert.PX = GC_ENDIAN_16( PNVert.PX );
        PNVert.PY = GC_ENDIAN_16( PNVert.PY );
        PNVert.PZ = GC_ENDIAN_16( PNVert.PZ );

        PNVert.NX = GC_ENDIAN_16( PNVert.NX );
        PNVert.NY = GC_ENDIAN_16( PNVert.NY );
        PNVert.NZ = GC_ENDIAN_16( PNVert.NZ );

        //--- copy converted data into final array, update array size
        *((GCSkinVert_PN*)&m_pVtxData[m_VtxDataSize]) = PNVert;
        m_VtxDataSize += sizeof(GCSkinVert_PN);
    }

    //--- since we want the vertex data to start on a 32B boundary, pad it
    m_VtxDataSize = (u32)ALIGN_32( m_VtxDataSize );

    if( m_VtxDataSizeAllocated <= m_VtxDataSize )
        GrowDataArray( m_pVtxData, m_VtxDataSizeAllocated, (512*1024) );
}

//==========================================================================

void QGameCubeRipSkin::BuildSubMeshVtxDataCPU( GCRipSkinSubMesh* pSubMesh )
{
    s32            CurVert;
    s32            NVerts;
    GCSkinVert_CPU CPUVert;

    //--- set submesh's offset into data array
    pSubMesh->OffsetVtxCPU = m_VtxDataSize;

    NVerts = pSubMesh->NVertsCPU1 + pSubMesh->NVertsCPU2 + pSubMesh->NVertsCPU3;

    //--- convert data into quantized format, and convert endian-ness
    for( CurVert = 0; CurVert < NVerts; CurVert++ )
    {
        //--- allocate more data if needed
        if( m_VtxDataSizeAllocated <= m_VtxDataSize )
            GrowDataArray( m_pVtxData, m_VtxDataSizeAllocated, (512*1024) );

        CPUVert.PX = (s16)(pSubMesh->pVertListCPU[CurVert].Pos.X * (1<<GC_FRAC_POS));
        CPUVert.PY = (s16)(pSubMesh->pVertListCPU[CurVert].Pos.Y * (1<<GC_FRAC_POS));
        CPUVert.PZ = (s16)(pSubMesh->pVertListCPU[CurVert].Pos.Z * (1<<GC_FRAC_POS));

        CPUVert.NX = (s16)(pSubMesh->pVertListCPU[CurVert].Normal.X * (1<<GC_FRAC_NRM));
        CPUVert.NY = (s16)(pSubMesh->pVertListCPU[CurVert].Normal.Y * (1<<GC_FRAC_NRM));
        CPUVert.NZ = (s16)(pSubMesh->pVertListCPU[CurVert].Normal.Z * (1<<GC_FRAC_NRM));

        if( pSubMesh->pVertListCPU[CurVert].NMatrices == 1 )
        {
            CPUVert.W0 = 0xFF;
            CPUVert.W1 = 0x00;
        }
        else
        {
            CPUVert.W0 = (u8)(pSubMesh->pVertListCPU[CurVert].MatrixWeight[0] * (1<<GC_FRAC_WEIGHT));
            CPUVert.W1 = (u8)(pSubMesh->pVertListCPU[CurVert].MatrixWeight[1] * (1<<GC_FRAC_WEIGHT));
        }

        CPUVert.MtxIdx  = (u16)((pSubMesh->pVertListCPU[CurVert].MatrixID[0] & 0x1F) <<  0);
        CPUVert.MtxIdx |= (u16)((pSubMesh->pVertListCPU[CurVert].MatrixID[1] & 0x1F) <<  5);
        CPUVert.MtxIdx |= (u16)((pSubMesh->pVertListCPU[CurVert].MatrixID[2] & 0x1F) << 10);

        CPUVert.PX = GC_ENDIAN_16( CPUVert.PX );
        CPUVert.PY = GC_ENDIAN_16( CPUVert.PY );
        CPUVert.PZ = GC_ENDIAN_16( CPUVert.PZ );

        CPUVert.NX = GC_ENDIAN_16( CPUVert.NX );
        CPUVert.NY = GC_ENDIAN_16( CPUVert.NY );
        CPUVert.NZ = GC_ENDIAN_16( CPUVert.NZ );

        CPUVert.MtxIdx = GC_ENDIAN_16( CPUVert.MtxIdx );

        //--- copy converted data into final array, update array size
        *((GCSkinVert_CPU*)&m_pVtxData[m_VtxDataSize]) = CPUVert;
        m_VtxDataSize += sizeof(GCSkinVert_CPU);
    }

    //--- since we want the vertex data to start on a 32B boundary, pad it
    m_VtxDataSize = (u32)ALIGN_32( m_VtxDataSize );

    if( m_VtxDataSizeAllocated <= m_VtxDataSize )
        GrowDataArray( m_pVtxData, m_VtxDataSizeAllocated, (512*1024) );
}

//==========================================================================

void QGameCubeRipSkin::BuildSubMeshVtxData( GCRipSkinSubMesh* pSubMesh )
{
    //--- if this mesh is a shadow, its vertex data is position only
    if( pSubMesh->bShadow )
    {
        ASSERT( pSubMesh->NVertsGP > 0 );
        BuildSubMeshVtxDataShadow( pSubMesh );
        return;
    }

    //--- store the UV data separate from other data, then do pos-nrm verts
    //    for GP, and then the CPU XForm verts
    s32 NVertsCPU = pSubMesh->NVertsCPU1 + pSubMesh->NVertsCPU2 + pSubMesh->NVertsCPU3;

    if( (NVertsCPU + pSubMesh->NVertsGP) > 0 )
        BuildSubMeshVtxDataUV( pSubMesh );

    if( pSubMesh->NVertsGP > 0 )
        BuildSubMeshVtxDataGP( pSubMesh );

    if( NVertsCPU > 0 )
        BuildSubMeshVtxDataCPU( pSubMesh );
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

static s32 GetMatrixSlotID( s32* pCurSlotMap, s32 NSlotsUsed, s32 VtxMatrixID )
{
    s32 i;

    for( i = 0; i < NSlotsUsed; i++ )
    {
        if( VtxMatrixID == pCurSlotMap[i] )
            return i;
    }

    return -1;
}

//==========================================================================

void QGameCubeRipSkin::BuildSubMeshDListDataShadow( GCRipSkinSubMesh* pSubMesh )
{
    s32             i;
    s32             CurGroup;
    s32             CurStrip;
    s32             NGroups;
    GC_StripTri*    pStripTris;
    GC_TStripGroup* pStripGroups;
    GC_TStrip*      pCurStrip;
    u32             DListStripSize;
    u32             DListCmdSize;
    byte*           pDList;
    s32             MtxSlotToMtxID[GC_MTX_ID_MAX];
    s32             VertMtxID;
    s32             VertCount;

    //--- allocate memory for the triangle indices array for building strips
    pStripTris = (GC_StripTri*)x_malloc( sizeof(GC_StripTri) * pSubMesh->NTrisGP );
    ASSERT( pStripTris != NULL );

    //--- copy vert IDs to temp array
    for( i = 0; i < pSubMesh->NTrisGP; i++ )
    {
        pStripTris[i].Verts[0] = pSubMesh->pTriListGP[i].VertID[0];
        pStripTris[i].Verts[1] = pSubMesh->pTriListGP[i].VertID[1];
        pStripTris[i].Verts[2] = pSubMesh->pTriListGP[i].VertID[2];
        pStripTris[i].MtxID[0] = pSubMesh->pVertListGP[ pStripTris[i].Verts[0] ].MatrixID[0];
        pStripTris[i].MtxID[1] = pSubMesh->pVertListGP[ pStripTris[i].Verts[1] ].MatrixID[0];
        pStripTris[i].MtxID[2] = pSubMesh->pVertListGP[ pStripTris[i].Verts[2] ].MatrixID[0];
    }

    //--- build the triangle strips
    GCTRISTRIP_Build( &pStripGroups, NGroups, pStripTris, pSubMesh->NTrisGP, MAX_SUBMESH_VERTS, GC_MTX_ID_MAX );

    //--- set submesh's GP DList offset into array
    pSubMesh->OffsetDListGP = m_DListDataSize;

    VertCount = 0;

    for( CurGroup = 0; CurGroup < NGroups; CurGroup++ )
    {
        pSubMesh->NMtxGroups   += 1;
        pSubMesh->NMtxSlots    += pStripGroups[CurGroup].NMatrices;
        pSubMesh->NTriStripsGP += pStripGroups[CurGroup].NStrips;

        ASSERT( pStripGroups[CurGroup].NMatrices <= GC_MTX_ID_MAX );

        //--- copy matrix indices to matrix slot map
        for( i = 0; i < GC_MTX_ID_MAX; i++ )
        {
            if( i < pStripGroups[CurGroup].NMatrices )
                MtxSlotToMtxID[i] = pStripGroups[CurGroup].pMtxIDs[i];
            else
                MtxSlotToMtxID[i] = -1;
        }

        DListCmdSize = sizeof(SDListMtxLoad) * pStripGroups[CurGroup].NMatrices;

        //--- alocate more memory if needed for display list
        if( m_DListDataSizeAllocated <= (m_DListDataSize + DListCmdSize) )
            GrowDataArray( m_pDListData, m_DListDataSizeAllocated, (256*1024) );

        //--- get a pointer to current position in display list and update size
        pDList = &m_pDListData[m_DListDataSize];
        m_DListDataSize       += DListCmdSize;
        pSubMesh->SizeDListGP += DListCmdSize;

        //--- build matrix load commands into display list
        for( i = 0; i < pStripGroups[CurGroup].NMatrices; i++ )
        {
            ((SDListMtxLoad*)pDList)->LoadCmd = GC_CMD_MTXLOAD_POS;
            ((SDListMtxLoad*)pDList)->MtxIdx  = GC_ENDIAN_16( (u16)MtxSlotToMtxID[i] );
            ((SDListMtxLoad*)pDList)->SizeCmd = GC_CMD_MTXSIZE_3x4;
            ((SDListMtxLoad*)pDList)->MtxAddr = GC_GET_MTXADDR_POS( i );

            pDList += sizeof(SDListMtxLoad);
            pSubMesh->NMtxLoads++;
        }


        //--- loop through the strips, adding data to main DList array
        for( CurStrip = 0; CurStrip < pStripGroups[CurGroup].NStrips; CurStrip++ )
        {
            pCurStrip = &pStripGroups[CurGroup].pStrips[CurStrip];

            DListStripSize = sizeof(SDListDrawPrimHdr) + (sizeof(SDListVert_Shadow) * pCurStrip->NVerts);

            //--- alocate more memory if needed for display list
            if( m_DListDataSizeAllocated <= (m_DListDataSize + DListStripSize) )
                GrowDataArray( m_pDListData, m_DListDataSizeAllocated, (256*1024) );

            //--- get a pointer to current position in display list and update size
            pDList = &m_pDListData[m_DListDataSize];
            m_DListDataSize       += DListStripSize;
            pSubMesh->SizeDListGP += DListStripSize;

            //--- build tri-strip header
            ((SDListDrawPrimHdr*)pDList)->DrawCmd  = GX_DRAW_TRIANGLE_STRIP | GX_VTXFMT0;
            ((SDListDrawPrimHdr*)pDList)->VtxCount = GC_ENDIAN_16( (u16)pCurStrip->NVerts );

            pDList += sizeof(SDListDrawPrimHdr);

            VertCount += pCurStrip->NVerts;

            //--- add vertex indices for position, normal, and UV.
            for( i = 0; i < pCurStrip->NVerts; i++ )
            {
                VertMtxID = pSubMesh->pVertListGP[ pCurStrip->pVerts[i] ].MatrixID[0];
                VertMtxID = GetMatrixSlotID( MtxSlotToMtxID, pStripGroups[CurGroup].NMatrices, VertMtxID );
                ASSERT( VertMtxID >= 0 );

                ((SDListVert_Shadow*)pDList)->MtxIdx = (u8)GC_GET_MTXROW_POSNRM( VertMtxID );
                ((SDListVert_Shadow*)pDList)->PosIdx = GC_ENDIAN_16( (u16)pCurStrip->pVerts[i] );
                ((SDListVert_Shadow*)pDList)->ClrIdx = (u8)0;

                pDList += sizeof(SDListVert_Shadow);
            }
        }
    }

    pSubMesh->AvgVtxPerTriGP = (f32)VertCount / pSubMesh->NTrisGP;

    //--- pad the display list
    PadDListData( pSubMesh, TRUE );

    //--- free the memory used in this function
    GCTRISTRIP_Free( &pStripGroups, NGroups );
    x_free( pStripTris );
}

//==========================================================================

void QGameCubeRipSkin::BuildSubMeshDListDataGP( GCRipSkinSubMesh* pSubMesh )
{
    s32             i;
    s32             CurGroup;
    s32             CurStrip;
    s32             NGroups;
    GC_StripTri*    pStripTris;
    GC_TStripGroup* pStripGroups;
    GC_TStrip*      pCurStrip;
    u32             DListStripSize;
    u32             DListCmdSize;
    byte*           pDList;
    s32             MtxSlotToMtxID[GC_MTX_ID_MAX];
    s32             VertMtxID;
    s32             VertCount;

    //--- allocate memory for the triangle indices array for building strips
    pStripTris = (GC_StripTri*)x_malloc( sizeof(GC_StripTri) * pSubMesh->NTrisGP );
    ASSERT( pStripTris != NULL );

    //--- copy vert IDs to temp array
    for( i = 0; i < pSubMesh->NTrisGP; i++ )
    {
        pStripTris[i].Verts[0] = pSubMesh->pTriListGP[i].VertID[0];
        pStripTris[i].Verts[1] = pSubMesh->pTriListGP[i].VertID[1];
        pStripTris[i].Verts[2] = pSubMesh->pTriListGP[i].VertID[2];
        pStripTris[i].MtxID[0] = pSubMesh->pVertListGP[ pStripTris[i].Verts[0] ].MatrixID[0];
        pStripTris[i].MtxID[1] = pSubMesh->pVertListGP[ pStripTris[i].Verts[1] ].MatrixID[0];
        pStripTris[i].MtxID[2] = pSubMesh->pVertListGP[ pStripTris[i].Verts[2] ].MatrixID[0];
    }

    //--- build the triangle strips
    GCTRISTRIP_Build( &pStripGroups, NGroups, pStripTris, pSubMesh->NTrisGP, MAX_SUBMESH_VERTS, GC_MTX_ID_MAX );

    //--- set submesh's GP DList offset into array
    pSubMesh->OffsetDListGP = m_DListDataSize;

    VertCount = 0;

    for( CurGroup = 0; CurGroup < NGroups; CurGroup++ )
    {
        ASSERT( pStripGroups[CurGroup].NMatrices <= GC_MTX_ID_MAX );

        pSubMesh->NMtxGroups   += 1;
        pSubMesh->NMtxSlots    += pStripGroups[CurGroup].NMatrices;
        pSubMesh->NTriStripsGP += pStripGroups[CurGroup].NStrips;

        //--- copy matrix indices to matrix slot map
        for( i = 0; i < GC_MTX_ID_MAX; i++ )
        {
            if( i < pStripGroups[CurGroup].NMatrices )
                MtxSlotToMtxID[i] = pStripGroups[CurGroup].pMtxIDs[i];
            else
                MtxSlotToMtxID[i] = -1;
        }

        if( pSubMesh->bEnvPass )
            DListCmdSize = (sizeof(SDListMtxLoad) * 3) * pStripGroups[CurGroup].NMatrices;
        else
            DListCmdSize = (sizeof(SDListMtxLoad) * 2) * pStripGroups[CurGroup].NMatrices;

        //--- alocate more memory if needed for display list
        if( m_DListDataSizeAllocated <= (m_DListDataSize + DListCmdSize) )
            GrowDataArray( m_pDListData, m_DListDataSizeAllocated, (256*1024) );

        //--- get a pointer to current position in display list and update size
        pDList = &m_pDListData[m_DListDataSize];
        m_DListDataSize       += DListCmdSize;
        pSubMesh->SizeDListGP += DListCmdSize;

        //--- build matrix load commands into display list
        for( i = 0; i < pStripGroups[CurGroup].NMatrices; i++ )
        {
            ((SDListMtxLoad*)pDList)->LoadCmd = GC_CMD_MTXLOAD_POS;
            ((SDListMtxLoad*)pDList)->MtxIdx  = GC_ENDIAN_16( (u16)MtxSlotToMtxID[i] );
            ((SDListMtxLoad*)pDList)->SizeCmd = GC_CMD_MTXSIZE_3x4;
            ((SDListMtxLoad*)pDList)->MtxAddr = GC_GET_MTXADDR_POS( i );

            pDList += sizeof(SDListMtxLoad);
            pSubMesh->NMtxLoads++;

            ((SDListMtxLoad*)pDList)->LoadCmd = GC_CMD_MTXLOAD_NRM;
            ((SDListMtxLoad*)pDList)->MtxIdx  = GC_ENDIAN_16( (u16)MtxSlotToMtxID[i] );
            ((SDListMtxLoad*)pDList)->SizeCmd = GC_CMD_MTXSIZE_3x3;
            ((SDListMtxLoad*)pDList)->MtxAddr = GC_GET_MTXADDR_NRM( i );

            pDList += sizeof(SDListMtxLoad);
            pSubMesh->NMtxLoads++;

            if( pSubMesh->bEnvPass )
            {
                ((SDListMtxLoad*)pDList)->LoadCmd = GC_CMD_MTXLOAD_TEX;
                ((SDListMtxLoad*)pDList)->MtxIdx  = GC_ENDIAN_16( (u16)MtxSlotToMtxID[i] );
                ((SDListMtxLoad*)pDList)->SizeCmd = GC_CMD_MTXSIZE_3x4;
                ((SDListMtxLoad*)pDList)->MtxAddr = GC_GET_MTXADDR_TEX( i );

                pDList += sizeof(SDListMtxLoad);
                pSubMesh->NMtxLoads++;
            }
        }


        //--- loop through the strips, adding data to main DList array
        for( CurStrip = 0; CurStrip < pStripGroups[CurGroup].NStrips; CurStrip++ )
        {
            pCurStrip = &pStripGroups[CurGroup].pStrips[CurStrip];

            if( pSubMesh->bEnvPass )
                DListStripSize = sizeof(SDListDrawPrimHdr) + (sizeof(SDListVert_GP_ENV) * pCurStrip->NVerts);
            else
                DListStripSize = sizeof(SDListDrawPrimHdr) + (sizeof(SDListVert_GP) * pCurStrip->NVerts);

            //--- alocate more memory if needed for display list
            if( m_DListDataSizeAllocated <= (m_DListDataSize + DListStripSize) )
                GrowDataArray( m_pDListData, m_DListDataSizeAllocated, (256*1024) );

            //--- get a pointer to current position in display list and update size
            pDList = &m_pDListData[m_DListDataSize];
            m_DListDataSize       += DListStripSize;
            pSubMesh->SizeDListGP += DListStripSize;

            //--- build tri-strip header
            ((SDListDrawPrimHdr*)pDList)->DrawCmd  = GX_DRAW_TRIANGLE_STRIP | GX_VTXFMT0;
            ((SDListDrawPrimHdr*)pDList)->VtxCount = GC_ENDIAN_16( (u16)pCurStrip->NVerts );

            pDList += sizeof(SDListDrawPrimHdr);

            VertCount += pCurStrip->NVerts;

            //--- add vertex indices for position, normal, and UV.
            for( i = 0; i < pCurStrip->NVerts; i++ )
            {
                VertMtxID = pSubMesh->pVertListGP[ pCurStrip->pVerts[i] ].MatrixID[0];
                VertMtxID = GetMatrixSlotID( MtxSlotToMtxID, pStripGroups[CurGroup].NMatrices, VertMtxID );
                if( VertMtxID >= 0 )
                {
                    if( pSubMesh->bEnvPass )
                    {
                        ((SDListVert_GP_ENV*)pDList)->MtxIdx    = (u8)GC_GET_MTXROW_POSNRM( VertMtxID );
                        ((SDListVert_GP_ENV*)pDList)->TexMtxIdx = (u8)GC_GET_MTXROW_TEXTURE( VertMtxID );
                        ((SDListVert_GP_ENV*)pDList)->PosIdx    = GC_ENDIAN_16( (u16)pCurStrip->pVerts[i] );
                        ((SDListVert_GP_ENV*)pDList)->NrmIdx    = GC_ENDIAN_16( (u16)pCurStrip->pVerts[i] );
                        ((SDListVert_GP_ENV*)pDList)->UVIdx     = GC_ENDIAN_16( (u16)pCurStrip->pVerts[i] );

                        pDList += sizeof(SDListVert_GP_ENV);
                    }
                    else
                    {
                        ((SDListVert_GP*)pDList)->MtxIdx = (u8)GC_GET_MTXROW_POSNRM( VertMtxID );
                        ((SDListVert_GP*)pDList)->PosIdx = GC_ENDIAN_16( (u16)pCurStrip->pVerts[i] );
                        ((SDListVert_GP*)pDList)->NrmIdx = GC_ENDIAN_16( (u16)pCurStrip->pVerts[i] );
                        ((SDListVert_GP*)pDList)->UVIdx  = GC_ENDIAN_16( (u16)pCurStrip->pVerts[i] );

                        pDList += sizeof(SDListVert_GP);
                    }
                }
                else
                {
                    if( g_Verbose )
                    {
                        x_printf( "%s, Invalid weighting on vertex: %d\n", m_pMesh[pSubMesh->ParentMesh].Name, i );
                    }
                }
            }
        }
    }

    pSubMesh->AvgVtxPerTriGP = (f32)VertCount / pSubMesh->NTrisGP;

    //--- pad the display list
    PadDListData( pSubMesh, TRUE );

    //--- free the memory used in this function
    GCTRISTRIP_Free( &pStripGroups, NGroups );
    x_free( pStripTris );
}

//==========================================================================

void QGameCubeRipSkin::BuildSubMeshDListDataCPU( GCRipSkinSubMesh* pSubMesh )
{
    s32             i;
    s32             CurStrip;
    s32             NStrips;
    STriangle*      pStripTris;
    STriangleStrip* pTriStrips;
    STriangleStrip* pCurStrip;
    u32             DListStripSize;
    byte*           pDList;
    s32             VertCount;


    //--- allocate memory for the triangle indices array for building strips
    pStripTris = (STriangle*)x_malloc( sizeof(STriangle) * pSubMesh->NTrisCPU );
    ASSERT( pStripTris != NULL );

    //--- copy vert IDs to temp array
    for( i = 0; i < pSubMesh->NTrisCPU; i++ )
    {
        pStripTris[i].Index[0] = pSubMesh->pTriListCPU[i].VertID[0];
        pStripTris[i].Index[1] = pSubMesh->pTriListCPU[i].VertID[1];
        pStripTris[i].Index[2] = pSubMesh->pTriListCPU[i].VertID[2];
    }

    //--- build the triangle strips
    BuildStrips( pStripTris, pSubMesh->NTrisCPU, &pTriStrips, NStrips, MAX_SUBMESH_VERTS );

    //--- set submesh's CPU DList offset into array
    pSubMesh->OffsetDListCPU = m_DListDataSize;

    VertCount = 0;

    pSubMesh->NTriStripsCPU = NStrips;

    //--- loop through the strips, adding data to main DList array
    for( CurStrip = 0; CurStrip < NStrips; CurStrip++ )
    {
        pCurStrip = &pTriStrips[CurStrip];

        DListStripSize = sizeof(SDListDrawPrimHdr) + (sizeof(SDListVert_CPU) * pCurStrip->NVerts);

        //--- alocate more memory if needed for display list
        if( m_DListDataSizeAllocated <= (m_DListDataSize + DListStripSize) )
            GrowDataArray( m_pDListData, m_DListDataSizeAllocated, (256*1024) );

        //--- get a pointer to current position in display list and update size
        pDList = &m_pDListData[m_DListDataSize];
        m_DListDataSize        += DListStripSize;
        pSubMesh->SizeDListCPU += DListStripSize;

        //--- build tri-strip header
        ((SDListDrawPrimHdr*)pDList)->DrawCmd  = GX_DRAW_TRIANGLE_STRIP | GX_VTXFMT0;
        ((SDListDrawPrimHdr*)pDList)->VtxCount = GC_ENDIAN_16( (u16)pCurStrip->NVerts );

        pDList += sizeof(SDListDrawPrimHdr);

        VertCount += pCurStrip->NVerts;

        //--- add vertex indices for position, normal, and UV; since UVs are stored in one array AFTER
        //    the UVs for the GP list, the UV indices need to be adjusted for it.
        for( i = 0; i < pCurStrip->NVerts; i++ )
        {
            ((SDListVert_CPU*)pDList)->PosIdx = GC_ENDIAN_16( (u16)pCurStrip->pVerts[i] );
            ((SDListVert_CPU*)pDList)->NrmIdx = GC_ENDIAN_16( (u16)pCurStrip->pVerts[i] );
            ((SDListVert_CPU*)pDList)->UVIdx  = GC_ENDIAN_16( (u16)(pCurStrip->pVerts[i] + pSubMesh->NVertsGP) );

            pDList += sizeof(SDListVert_CPU);
        }
    }

    pSubMesh->AvgVtxPerTriCPU = (f32)VertCount / pSubMesh->NTrisCPU;

    //--- pad the display list
    PadDListData( pSubMesh, FALSE );

    //--- free the memory used in this function
    for( i = 0; i < NStrips; i++ )
        x_free( pTriStrips[i].pVerts );

    x_free( pTriStrips );
    x_free( pStripTris );
}

//==========================================================================

void QGameCubeRipSkin::PadDListData( GCRipSkinSubMesh* pSubMesh, xbool GPDList )
{
    //--- Since all display lists are multiples of 32B, the whole chunk of
    //    display list data will be aligned to 32B.  All that needs to be
    //    done for padding the DList is adjusting the main array size and
    //    then filling it with NOPs.

    if( !IS_ALIGNED_32( m_DListDataSize ) )
    {
        u32 NewDListDataSize = (u32)ALIGN_32( m_DListDataSize );
        u32 i;

        //--- alocate more memory if needed for padding
        if( m_DListDataSizeAllocated <= NewDListDataSize )
            GrowDataArray( m_pDListData, m_DListDataSizeAllocated, (256*1024) );

        //--- pad with GX_NOP
        for( i = m_DListDataSize; i < NewDListDataSize; i++ )
            m_pDListData[i] = GX_NOP;

        //--- update new size on main array and submesh
        m_DListDataSize = NewDListDataSize;

        if( GPDList )
            pSubMesh->SizeDListGP = (u32)ALIGN_32( pSubMesh->SizeDListGP );
        else
            pSubMesh->SizeDListCPU = (u32)ALIGN_32( pSubMesh->SizeDListCPU );
    }
}

//==========================================================================

void QGameCubeRipSkin::BuildSubMeshDListData( GCRipSkinSubMesh* pSubMesh )
{
    if( pSubMesh->bShadow )
    {
        ASSERT( pSubMesh->NTrisGP > 0 );
        BuildSubMeshDListDataShadow( pSubMesh );
        return;
    }

    if( pSubMesh->NTrisGP > 0 )
        BuildSubMeshDListDataGP( pSubMesh );

    if( pSubMesh->NTrisCPU > 0 )
        BuildSubMeshDListDataCPU( pSubMesh );
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void QGameCubeRipSkin::BuildSubMeshDeltaData( GCRipSkinSubMesh* pSubMesh )
{
    s32         CurDelta;
    s32         NDeltas;
    GCSkinDelta DeltaValue;

    if( pSubMesh->bMorph == FALSE )
        return;

    NDeltas = pSubMesh->NDeltasCPU;

    if( NDeltas <= 0 )
    {
        ASSERT( FALSE );
        pSubMesh->bMorph = FALSE;
        return;
    }

    //--- set submesh's offset into data array
    pSubMesh->OffsetDeltasCPU = m_DeltaDataSize;

    for( CurDelta = 0; CurDelta < NDeltas; CurDelta++ )
    {
        //--- allocate more data if needed
        if( m_DeltaDataSizeAllocated <= m_DeltaDataSize )
            GrowDataArray( m_pDeltaData, m_DeltaDataSizeAllocated, (256 * sizeof(GCSkinDelta)) );

        //--- convert deltas to fixed-point format
        DeltaValue.XDelta = (s16)(pSubMesh->pDeltaListCPU[CurDelta].Delta.X * (1<<GC_FRAC_DELTA));
        DeltaValue.YDelta = (s16)(pSubMesh->pDeltaListCPU[CurDelta].Delta.Y * (1<<GC_FRAC_DELTA));
        DeltaValue.ZDelta = (s16)(pSubMesh->pDeltaListCPU[CurDelta].Delta.Z * (1<<GC_FRAC_DELTA));

        //--- convert delta data for endian-ness
        DeltaValue.XDelta  = GC_ENDIAN_16( DeltaValue.XDelta );
        DeltaValue.YDelta  = GC_ENDIAN_16( DeltaValue.YDelta );
        DeltaValue.ZDelta  = GC_ENDIAN_16( DeltaValue.ZDelta );
        DeltaValue.VertIdx = GC_ENDIAN_16( (u16)pSubMesh->pDeltaListCPU[CurDelta].VertID );

        //--- copy converted data into final array, update array size
        *((GCSkinDelta*)&m_pDeltaData[m_DeltaDataSize]) = DeltaValue;
        m_DeltaDataSize += sizeof(GCSkinDelta);
    }
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

inline xbool TriHasMultiBonesPerVtx( GCSkinTri* pTri, SRipVert* pVerts )
{
    if( (pVerts[ pTri->VertID[0] ].NMatrices > 1) ||
        (pVerts[ pTri->VertID[1] ].NMatrices > 1) ||
        (pVerts[ pTri->VertID[2] ].NMatrices > 1) )
    {
        return TRUE;
    }

    return FALSE;
}

//==========================================================================

static SRipVert* s_pCompVert = NULL;

static s32 CompareTri_BonesPerVtx( const void* Element1, const void* Element2 )
{
    ASSERT( s_pCompVert != NULL );

    GCSkinTri* pT1 = (GCSkinTri*)Element1;
    GCSkinTri* pT2 = (GCSkinTri*)Element2;

    //--- Check if the tris have verts with more than 1 bone
    xbool bT1HasMultiBonesPerVtx = TriHasMultiBonesPerVtx( pT1, s_pCompVert );
    xbool bT2HasMultiBonesPerVtx = TriHasMultiBonesPerVtx( pT2, s_pCompVert );

    if( bT1HasMultiBonesPerVtx && bT2HasMultiBonesPerVtx )
    {
        //--- both tris have multi-bones per vertex
        //--- do a simple count on total matrices to sort them

        s32 T1MtxRating, T2MtxRating;

        T1MtxRating = s_pCompVert[ pT1->VertID[0] ].NMatrices +
                      s_pCompVert[ pT1->VertID[1] ].NMatrices +
                      s_pCompVert[ pT1->VertID[2] ].NMatrices;

        T2MtxRating = s_pCompVert[ pT2->VertID[0] ].NMatrices +
                      s_pCompVert[ pT2->VertID[1] ].NMatrices +
                      s_pCompVert[ pT2->VertID[2] ].NMatrices;

        if( T1MtxRating < T2MtxRating )
            return -1;
        else if( T1MtxRating > T2MtxRating )
            return 1;
        else
            return 0;
    }
    else if( bT1HasMultiBonesPerVtx && !bT2HasMultiBonesPerVtx )
    {
        return 1;   // T1 has multi-bones per vertex, it goes after T2
    }
    else if( !bT1HasMultiBonesPerVtx && bT2HasMultiBonesPerVtx )
    {
        return -1;  // T2 has multi-bones per vertex, it goes after T1
    }
    else
    {
        return 0;   // none of the tris have multi-bones per vertex, they're equal
    }
}

//==========================================================================

static s32 CompareVert_NBones( const void* Element1, const void* Element2 )
{
    SRipVert* pV1 = (SRipVert*)Element1;
    SRipVert* pV2 = (SRipVert*)Element2;

    if( pV1->NMatrices < pV2->NMatrices )
        return -1;
    if( pV1->NMatrices > pV2->NMatrices )
        return 1;

    //--- NMatrices are the same, sort by matrix index
    if( pV1->NMatrices == 1 )
    {
        //--- 1 mtx, simple compare between pV1 and pV2
        if( pV1->MatrixID[0] < pV2->MatrixID[0] )
            return -1;
        if( pV1->MatrixID[0] > pV2->MatrixID[0] )
            return 1;
    }
    else if( pV1->NMatrices == 2 )
    {
        //--- 2 mtx, check if Mtx0 is different
        if( pV1->MatrixID[0] < pV2->MatrixID[0] )
            return -1;
        if( pV1->MatrixID[0] > pV2->MatrixID[0] )
            return 1;

        //--- if Mtx0 is same, check Mtx1
        if( pV1->MatrixID[1] < pV2->MatrixID[1] )
            return -1;
        if( pV1->MatrixID[1] > pV2->MatrixID[1] )
            return 1;
    }
    else
    {
        //--- 3 mtx, check if Mtx0 is different
        if( pV1->MatrixID[0] < pV2->MatrixID[0] )
            return -1;
        if( pV1->MatrixID[0] > pV2->MatrixID[0] )
            return 1;

        //--- if Mtx0 is same, check Mtx1
        if( pV1->MatrixID[1] < pV2->MatrixID[1] )
            return -1;
        if( pV1->MatrixID[1] > pV2->MatrixID[1] )
            return 1;

        //--- if Mtx1 is same, check Mtx2
        if( pV1->MatrixID[2] < pV2->MatrixID[2] )
            return -1;
        if( pV1->MatrixID[2] > pV2->MatrixID[2] )
            return 1;
    }

    //--- NMatrices and MatrixIDs are all same, no sort
    return 0;
}

//==========================================================================

static s32 CompareVert_BoneID( const void* Element1, const void* Element2 )
{
    SRipVert* pV1 = (SRipVert*)Element1;
    SRipVert* pV2 = (SRipVert*)Element2;

    if( pV1->MatrixID[0] < pV2->MatrixID[0] )
        return -1;
    if( pV1->MatrixID[0] > pV2->MatrixID[0] )
        return 1;

    return 0;
}

//==========================================================================

static s32 CopyVertsToList( GCSkinTri* pTriList, s32 NTris, SRipVert* pOldVertList, SRipVert** ppNewVertList )
{
    ASSERT( pTriList != NULL );
    ASSERT( pOldVertList != NULL );
    ASSERT( ppNewVertList != NULL );

    s32 i;
    s32 CurTri;
    s32 CurVert;
    s32 NVerts          = 0;
    s32 NVertsAllocated = 0;

    SRipVert* pNewVerts = NULL;

    //--- loop through all the tris in the GP list, adding only unique verts to list
    for( CurTri = 0; CurTri < NTris; CurTri++ )
    {
        //--- check each vert ID from tri
        for( i = 0; i < 3; i++ )
        {
            xbool VertInList = FALSE;
            s32   VertID     = pTriList[CurTri].VertID[i];

            //--- try to find the vert in the current list
            for( CurVert = 0; CurVert < NVerts; CurVert++ )
            {
                if( VertID == pNewVerts[CurVert].OldVertID )
                {
                    VertInList = TRUE;  // vert found
                    break;
                }
            }

            if( !VertInList )
            {
                //--- make sure there's room to store more verts, allocate more space if needed
                if( NVertsAllocated <= NVerts )
                    GrowDataArray( pNewVerts, NVertsAllocated, 256 );

                //--- copy the vert to the list, update count
                pNewVerts[NVerts] = pOldVertList[VertID];
                NVerts++;
            }
        }
    }

    //--- return pointer to new list of verts and the count
    *ppNewVertList = pNewVerts;

    return NVerts;
}

//==========================================================================

static void CopyDeltasToList( GCRipSkinSubMesh*     pSubMesh,
                              GCRipSkinMorphTarget* pMorphTgts,
                              s32                   NTargets,
                              GCSkinMorphDelta*     pDeltas )
{
    ASSERT( pSubMesh != NULL );
    ASSERT( pMorphTgts != NULL );
    ASSERT( pDeltas != NULL );

    s32 i, j;
    s32 CurTgt;
    s32 CurVert;
    s32 LastDelta;
    s32 DeltasFound;
    s32 NDeltas          = 0;
    s32 NDeltasAllocated = 0;

    s32       NVerts    = pSubMesh->NVertsCPU1 + pSubMesh->NVertsCPU2 + pSubMesh->NVertsCPU3;
    SRipVert* pVertList = pSubMesh->pVertListCPU;

    GCSkinMorphDelta* pNewDeltas = NULL;

    //--- loop through the vert list to find deltas used for them
    for( CurVert = 0; CurVert < NVerts; CurVert++ )
    {
        if( FALSE == pVertList[CurVert].HasMorphDelta )
            continue;

        DeltasFound = 0;

        //--- run through morph target list to get all deltas for this vert
        for( CurTgt = 0; CurTgt < NTargets; CurTgt++ )
        {
            LastDelta = pMorphTgts[CurTgt].FirstDeltaID + pMorphTgts[CurTgt].NDeltas;

            //--- loop through list to find a delta for this vert
            for( i = pMorphTgts[CurTgt].FirstDeltaID; i < LastDelta; i++ )
            {
                //--- since the verts are no longer in their original list, the check
                //    to find a matching delta uses the "old" vert ID
                if( pVertList[CurVert].OldVertID == pDeltas[i].VertID )
                {
                    if( DeltasFound < CurTgt )
                    {
                        //--- if a previous morph target didn't have a delta for a vert,
                        //    fill in empty deltas for those targets
                        for( j = DeltasFound; j < CurTgt; j++ )
                        {
                            if( NDeltasAllocated <= NDeltas )
                                GrowDataArray( pNewDeltas, NDeltasAllocated, (256 * NTargets) );

                            pNewDeltas[NDeltas].Delta.Set( 0, 0, 0 );
                            pNewDeltas[NDeltas].VertID = CurVert;
                            NDeltas++;
                            DeltasFound++;
                        }
                    }
                    else
                    {
                        if( NDeltasAllocated <= NDeltas )
                            GrowDataArray( pNewDeltas, NDeltasAllocated, (256 * NTargets) );

                        //--- copy delta to new list, remap delta vert ID to new list index, and increment count
                        pNewDeltas[NDeltas].Delta  = pDeltas[i].Delta;
                        pNewDeltas[NDeltas].VertID = CurVert;
                        NDeltas++;
                        DeltasFound++;
                    }

                    break;
                }
            }
        }

        if( (DeltasFound > 0) && (DeltasFound < NTargets) )
        {
            //--- if there were morph target didn't have a delta(s) for a vert,
            //    and there was deltas present for previous targets(hence the DeltasFound > 0),
            //    fill in empty deltas for those targets
            for( j = DeltasFound; j < NTargets; j++ )
            {
                if( NDeltasAllocated <= NDeltas )
                    GrowDataArray( pNewDeltas, NDeltasAllocated, (256 * NTargets) );

                pNewDeltas[NDeltas].Delta.Set( 0, 0, 0 );
                pNewDeltas[NDeltas].VertID = CurVert;
                NDeltas++;
                DeltasFound++;
            }
        }
    }

    //--- do some sanity checks on delta data
    ASSERT( (NDeltas % NTargets) == 0 );

    for( i = 0; i < NDeltas; i += NTargets )
    {
        s32 VertID = pNewDeltas[i].VertID;

        for( j = 1; j < NTargets; j++ )
        {
            ASSERT( VertID == pNewDeltas[ i + j ].VertID );
        }
    }

    //--- assign submesh members
    pSubMesh->pDeltaListCPU = pNewDeltas;
    pSubMesh->NDeltasCPU    = NDeltas;
}

//==========================================================================

static void RemapTriVertIDs( GCSkinTri* pTris, s32 NTris, SRipVert* pVerts, s32 NVerts )
{
    ASSERT( pTris != NULL );
    ASSERT( pVerts != NULL );

    s32 CurTri;
    s32 CurVert;

    for( CurTri = 0; CurTri < NTris; CurTri++ )
    {
        ASSERT( (pTris[CurTri].VertID[0] != pTris[CurTri].VertID[1]) );
        ASSERT( (pTris[CurTri].VertID[0] != pTris[CurTri].VertID[2]) );
        ASSERT( (pTris[CurTri].VertID[1] != pTris[CurTri].VertID[2]) );

        s32 NTriVertsFound = 0;

        //--- check if any of this tri's vert IDs match this vert's original one,
        //    and set the tri's indices to match the vert's new ID
        for( CurVert = 0; CurVert < NVerts; CurVert++ )
        {
            if( pTris[CurTri].VertID[0] == pVerts[CurVert].OldVertID )
            {
                pTris[CurTri].VertID[0] = CurVert;
                NTriVertsFound++;
                break;
            }
        }
        for( CurVert = 0; CurVert < NVerts; CurVert++ )
        {
            if( pTris[CurTri].VertID[1] == pVerts[CurVert].OldVertID )
            {
                pTris[CurTri].VertID[1] = CurVert;
                NTriVertsFound++;
                break;
            }
        }
        for( CurVert = 0; CurVert < NVerts; CurVert++ )
        {
            if( pTris[CurTri].VertID[2] == pVerts[CurVert].OldVertID )
            {
                pTris[CurTri].VertID[2] = CurVert;
                NTriVertsFound++;
                break;
            }
        }

        ASSERT( NTriVertsFound == 3 );
    }

    //--- set the vert original IDs to match their current one
    for( CurVert = 0; CurVert < NVerts; CurVert++ )
    {
        pVerts[CurVert].OldVertID = CurVert;
    }
}

//==========================================================================

void QGameCubeRipSkin::BuildSubMeshData( s32 SubMeshID )
{
    s32        CurVert;
    s32        CurTri;
    GCSkinTri* pTriList;

    GCRipSkinSubMesh* pSubMesh = &m_pSubMesh[SubMeshID];

    //--- clear out submesh info that's about to be calculated
    pSubMesh->NTrisGP         = 0;
    pSubMesh->NTrisCPU        = 0;
    pSubMesh->NVertsGP        = 0;
    pSubMesh->NVertsCPU1      = 0;
    pSubMesh->NVertsCPU2      = 0;
    pSubMesh->NVertsCPU3      = 0;
    pSubMesh->pTriListGP      = NULL;
    pSubMesh->pTriListCPU     = NULL;
    pSubMesh->pVertListGP     = NULL;
    pSubMesh->pVertListCPU    = NULL;
    pSubMesh->NDeltasCPU      = 0;
    pSubMesh->pDeltaListCPU   = NULL;
    pSubMesh->SizeDListGP     = 0;
    pSubMesh->SizeDListCPU    = 0;
    pSubMesh->OffsetDListGP   = GC_INVALID_PTR_OFFSET;
    pSubMesh->OffsetDListCPU  = GC_INVALID_PTR_OFFSET;
    pSubMesh->OffsetVtxGP     = GC_INVALID_PTR_OFFSET;
    pSubMesh->OffsetVtxCPU    = GC_INVALID_PTR_OFFSET;
    pSubMesh->OffsetVtxShadow = GC_INVALID_PTR_OFFSET;
    pSubMesh->OffsetUVs       = GC_INVALID_PTR_OFFSET;
    pSubMesh->OffsetDeltasCPU = GC_INVALID_PTR_OFFSET;
    pSubMesh->AvgVtxPerTriGP  = 0.0f;
    pSubMesh->AvgVtxPerTriCPU = 0.0f;
    pSubMesh->NTriStripsGP    = 0;
    pSubMesh->NTriStripsCPU   = 0;
    pSubMesh->NMtxGroups      = 0;
    pSubMesh->NMtxSlots       = 0;
    pSubMesh->NMtxLoads       = 0;


    ASSERT( pSubMesh->NTris > 0 );

    //--- allocated new tri list for submesh, and copy tris to it
    pTriList = (GCSkinTri*)x_malloc( sizeof(GCSkinTri) * pSubMesh->NTris );
    ASSERT( pTriList != NULL );
    x_memcpy( pTriList, &m_pTri[ pSubMesh->FirstTriID ], sizeof(GCSkinTri) * pSubMesh->NTris );

    //--- sort tri list based on the bones per vertex each has
    s_pCompVert = m_pVert;
    x_qsort( pTriList, pSubMesh->NTris, sizeof(GCSkinTri), CompareTri_BonesPerVtx );


    if( pSubMesh->bShadow )
    {
        //--- shadow mesh is always done on GP
        pSubMesh->NTrisGP = pSubMesh->NTris;
    }
    else if( pSubMesh->bMorph )
    {
        //--- submesh with morph deltas always done on CPU
        pSubMesh->NTrisCPU = pSubMesh->NTris;
    }
    else
    {
        #ifdef DONT_USE_GP_XFORM
            pSubMesh->NTrisCPU = pSubMesh->NTris;
        #else

            //--- find first tri with verts that have more than 1 bone, save the count for splitting list
            if( TriHasMultiBonesPerVtx( &pTriList[ pSubMesh->NTris-1 ], m_pVert ) )
            {
                //--- Last vtx has multiple bones, find the count of tris for both GP and CPU lists
                for( CurTri = 0; CurTri < pSubMesh->NTris; CurTri++ )
                {
                    if( TriHasMultiBonesPerVtx( &pTriList[CurTri], m_pVert ) )
                    {
                        #ifdef DONT_MIX_GP_CPU_XFORM
                            pSubMesh->NTrisCPU = pSubMesh->NTris;
                        #else
                            pSubMesh->NTrisGP  = CurTri;
                            pSubMesh->NTrisCPU = pSubMesh->NTris - CurTri;
                        #endif

                        break;
                    }
                }
            }
            else
            {   //--- All of the tris are part of GP list
                pSubMesh->NTrisGP = pSubMesh->NTris;
            }

        #endif //DONT_USE_GP_XFORM
    }

    //--- allocated new tri lists for GP and CPU, and copy tris to them
    if( pSubMesh->NTrisGP > 0 )
    {
        pSubMesh->pTriListGP = (GCSkinTri*)x_malloc( sizeof(GCSkinTri) * pSubMesh->NTrisGP );
        ASSERT( pSubMesh->pTriListGP != NULL );
        x_memcpy( pSubMesh->pTriListGP, &pTriList[0], sizeof(GCSkinTri) * pSubMesh->NTrisGP );
    }

    if( pSubMesh->NTrisCPU > 0 )
    {
        pSubMesh->pTriListCPU = (GCSkinTri*)x_malloc( sizeof(GCSkinTri) * pSubMesh->NTrisCPU );
        ASSERT( pSubMesh->pTriListCPU != NULL );
        x_memcpy( pSubMesh->pTriListCPU, &pTriList[pSubMesh->NTrisGP], sizeof(GCSkinTri) * pSubMesh->NTrisCPU );
    }

    //--- free original tri list since it's not needed
    x_free( pTriList );


    //--- Now add verts to the GP list
    if( pSubMesh->NTrisGP > 0 )
    {
        pSubMesh->NVertsGP = CopyVertsToList( pSubMesh->pTriListGP, pSubMesh->NTrisGP, m_pVert, &pSubMesh->pVertListGP );
        ASSERT( pSubMesh->NVertsGP < MAX_SUBMESH_VERTS );

        //--- sort GP vert list so they're arranged by bone ID
        x_qsort( pSubMesh->pVertListGP, pSubMesh->NVertsGP, sizeof(SRipVert), CompareVert_BoneID );

        RemapTriVertIDs( pSubMesh->pTriListGP, pSubMesh->NTrisGP, pSubMesh->pVertListGP, pSubMesh->NVertsGP );
    }


    //--- Now add verts to the CPU list
    if( pSubMesh->NTrisCPU > 0 )
    {
        s32 NVertsCPU = CopyVertsToList( pSubMesh->pTriListCPU, pSubMesh->NTrisCPU, m_pVert, &pSubMesh->pVertListCPU );
        ASSERT( NVertsCPU < MAX_SUBMESH_VERTS );

        //--- sort CPU vert list so they're arranged by # bones per vert(1,2,3)
        x_qsort( pSubMesh->pVertListCPU, NVertsCPU, sizeof(SRipVert), CompareVert_NBones );

        //--- get a count on the number of each kind of vert
        for( CurVert = 0; CurVert < NVertsCPU; CurVert++ )
        {
            if( pSubMesh->pVertListCPU[CurVert].NMatrices == 1 )
                pSubMesh->NVertsCPU1++;
            else if( pSubMesh->pVertListCPU[CurVert].NMatrices == 2 )
                pSubMesh->NVertsCPU2++;
            else if( pSubMesh->pVertListCPU[CurVert].NMatrices == 3 )
                pSubMesh->NVertsCPU3++;
        }
        ASSERT( (pSubMesh->NVertsCPU1 + pSubMesh->NVertsCPU2 + pSubMesh->NVertsCPU3) == NVertsCPU );

        //--- create morph deltas for submesh if there is data for them
        if( pSubMesh->bMorph )
        {
            GCRipSkinMesh* pMesh = &(m_pMesh[ pSubMesh->ParentMesh ]);

            CopyDeltasToList( pSubMesh, &m_pMorphTarget[ pMesh->FirstMorphTgtID ], pMesh->NMorphTargets, m_pMorphDelta );
        }

        //--- convert the triangle vert IDs to match new vert array
        RemapTriVertIDs( pSubMesh->pTriListCPU, pSubMesh->NTrisCPU, pSubMesh->pVertListCPU, NVertsCPU );
    }

    //--- now that our tri and vertex data is sorted out, put it all in the skin's
    //    big data arrays, converting to correct data formats and endian-ness.
    BuildSubMeshVtxData  ( pSubMesh );
    BuildSubMeshDListData( pSubMesh );
    BuildSubMeshDeltaData( pSubMesh );

    //--- free pointers sice data is now in skin vertex/display list data arrays
    if( pSubMesh->pTriListGP    != NULL )   x_free( pSubMesh->pTriListGP );
    if( pSubMesh->pVertListGP   != NULL )   x_free( pSubMesh->pVertListGP );
    if( pSubMesh->pTriListCPU   != NULL )   x_free( pSubMesh->pTriListCPU );
    if( pSubMesh->pVertListCPU  != NULL )   x_free( pSubMesh->pVertListCPU );
    if( pSubMesh->pDeltaListCPU != NULL )   x_free( pSubMesh->pDeltaListCPU );

    pSubMesh->pTriListGP    = NULL;
    pSubMesh->pVertListGP   = NULL;
    pSubMesh->pTriListCPU   = NULL;
    pSubMesh->pVertListCPU  = NULL;
    pSubMesh->pDeltaListCPU = NULL;
}

//==========================================================================

static s32 CompareTri_TextureMesh( const void* Element1, const void* Element2 )
{
    GCSkinTri* pT1 = (GCSkinTri*)Element1;
    GCSkinTri* pT2 = (GCSkinTri*)Element2;

    //--- sort by MeshID and TextureID
    if( pT1->MeshID == pT2->MeshID ) 
    {
        if( pT1->TextureID == pT2->TextureID )
            return 0;

        if( pT1->TextureID < pT2->TextureID )
            return -1;
        else
            return 1;
    }

    if( pT1->MeshID < pT2->MeshID )
        return -1;
    else
        return 1;
}

//==========================================================================

void QGameCubeRipSkin::PrepareStructures( void )
{
    s32 CurVtx;
    s32 CurTri;
    s32 CurMeshID;
    s32 CurSubMeshID;
    s32 CurTextureID;

    GCRipSkinMesh* pCurMesh;
    GCRipSkinSubMesh* pCurSubMesh;

    //--- compute vertex normals
    ComputeNormals();

    // Go through all of the vertex points in the entire skin, and blend any normals that share the same vertex position.
    //
    // The reason this needs to be done is because some vertices are copied to allow the skin to be broken up into 
    // several parts, such and hands, and legs etc...  These copied vertices, will not share the same polygon connections, 
    // and because of that, even though the vertices share the same space, they don't share the same vertex normal.
    // As a result you will see a distinct difference in the two connecting pieces because the lighting will emphasize the
    // different vertex normals.
    BlendVertexNormals(  );

    //--- sort tri list based on texture and mesh
    x_qsort( m_pTri, m_NTris, sizeof(GCSkinTri), CompareTri_TextureMesh );

    //--- build submesh info based on groups of tris with same texture
    CurTri = 0;
    while( TRUE )
    {
        CurMeshID    = m_pTri[CurTri].MeshID;
        CurTextureID = m_pTri[CurTri].TextureID;

        pCurMesh = &m_pMesh[CurMeshID];

        //--- allocated more space for submeshes if needed
        if( m_NSubmeshesAllocated <= m_NSubmeshes )
            GrowDataArray( m_pSubMesh, m_NSubmeshesAllocated, 32 );

        pCurSubMesh = &m_pSubMesh[m_NSubmeshes];

        //--- fill out submesh structure
        pCurSubMesh->bAlpha     = pCurMesh->bAlpha;
        pCurSubMesh->bEnvPass   = pCurMesh->bEnvMapped;
        pCurSubMesh->bMorph     = FALSE;
        pCurSubMesh->bShadow    = pCurMesh->bShadow;
        pCurSubMesh->FirstTriID = CurTri;
        pCurSubMesh->NTris      = 1;
        pCurSubMesh->ParentMesh = CurMeshID;
        pCurSubMesh->TextureID  = CurTextureID;

        //--- get tri count for submesh, when a different texture is found,
        //    that's the end of this submesh
        CurTri++;
        while( TRUE )
        {
            if( CurTri == m_NTris )
                break;
            if( m_pTri[CurTri].MeshID != CurMeshID )
                break;
            if( (m_pTri[CurTri].TextureID != CurTextureID) && (pCurMesh->bShadow == FALSE) )
                break;

            pCurSubMesh->NTris++;
            CurTri++;
        }

        //--- submesh has been added, update count
        m_NSubmeshes++;

        if( CurTri == m_NTris )
            break;
    }

    //--- build mesh info, counting the number of submeshes it has
    for( CurMeshID = 0; CurMeshID < m_NMeshes; CurMeshID++ )
    {
        s32 FirstSubMesh, LastSubMesh;

        FirstSubMesh = -1;
        LastSubMesh  = -1;
        for( CurSubMeshID = 0; CurSubMeshID < m_NSubmeshes; CurSubMeshID++ )
        {
            if( FirstSubMesh == -1 )
            {
                if( m_pSubMesh[CurSubMeshID].ParentMesh == CurMeshID )
                    FirstSubMesh = CurSubMeshID;
            }

            if( m_pSubMesh[CurSubMeshID].ParentMesh == CurMeshID )
                LastSubMesh = CurSubMeshID;
        }
        ASSERT( (FirstSubMesh >= 0) && (LastSubMesh >= 0) );

        m_pMesh[CurMeshID].FirstSubmesh = FirstSubMesh;
        m_pMesh[CurMeshID].NSubmeshes   = (LastSubMesh - FirstSubMesh) + 1;
    }

    //--- find submeshes that have morph deltas, and mark them
    for( CurSubMeshID = 0; CurSubMeshID < m_NSubmeshes; CurSubMeshID++ )
    {
        CurTri = m_pSubMesh[CurSubMeshID].FirstTriID;
        while( CurTri < (m_pSubMesh[CurSubMeshID].FirstTriID + m_pSubMesh[CurSubMeshID].NTris) )
        {
            if( m_pVert[ m_pTri[CurTri].VertID[0] ].HasMorphDelta ||
                m_pVert[ m_pTri[CurTri].VertID[1] ].HasMorphDelta ||
                m_pVert[ m_pTri[CurTri].VertID[2] ].HasMorphDelta )
            {
                m_pSubMesh[CurSubMeshID].bMorph = TRUE;
                break;
            }

            CurTri++;
        }
    }

    //--- set vertex indices so tri lists(after sort) can find verts
    for( CurVtx = 0; CurVtx < m_NVerts; CurVtx++ )
    {
        m_pVert[CurVtx].OldVertID = CurVtx;
    }

    //--- build vertex and display list data for every submesh
    x_printf( "\n" );
    for( CurSubMeshID = 0; CurSubMeshID < m_NSubmeshes; CurSubMeshID++ )
    {
        x_printf( "**************** SUBMESH %3d ****************\n", (s32)CurSubMeshID );
        BuildSubMeshData( CurSubMeshID );
    }
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void QGameCubeRipSkin::DisplayStats( void )
{
    s32  i;
    char TexName[X_MAX_FNAME];
    s32  NVertsGP      = 0;
    s32  NVertsCPU1    = 0;
    s32  NVertsCPU2    = 0;
    s32  NVertsCPU3    = 0;
    s32  NTrisGP       = 0;
    s32  NTrisCPU      = 0;
    u32  DLSizeGP      = 0;
    u32  DLSizeCPU     = 0;
    s32  NTriStripsGP  = 0;
    s32  NTriStripsCPU = 0;
    s32  NMatrixGroups = 0;
    s32  NMtxSlots     = 0;
    s32  NMatrixLoads  = 0;

    //--- Display meshes
    x_printf( "\n\nMESHES:\n" );
    for( i = 0; i < m_NMeshes; i++ )
    {
        x_printf( "%3d] 1stSub: %3d  NSubs: %3d   %1s\n",
                  i,
                  m_pMesh[i].FirstSubmesh,
                  m_pMesh[i].NSubmeshes,
                  m_pMesh[i].Name );
    }

    x_printf( "\nOTHER MESH INFO:\n" );
    for( i = 0; i < m_NMeshes; i++ )
    {
        x_printf( "%3d] %3d MTgts  Env-%c  Shadow-%c  Alpha-%c\n",
                  i,
                  m_pMesh[i].NMorphTargets,
                  m_pMesh[i].bEnvMapped ? 'Y' : 'N',
                  m_pMesh[i].bShadow ? 'Y' : 'N',
                  m_pMesh[i].bAlpha ? 'Y' : 'N' );
    }


    //--- Display submeshes
    x_printf( "\n\nSUBMESHES:\n" );
    for( i = 0; i < m_NSubmeshes; i++ )
    {
        if( m_pSubMesh[i].TextureID >= 0 )
        {
            x_splitpath( m_pTexture[ m_pSubMesh[i].TextureID ].Name, NULL, NULL, TexName, NULL );

            x_printf( "%3d] Mesh: %3d   TexID: %2d  (%3d,%3d)  %1s\n",
                      i,
                      m_pSubMesh[i].ParentMesh,
                      m_pSubMesh[i].TextureID,
                      m_pTexture[ m_pSubMesh[i].TextureID ].Width,
                      m_pTexture[ m_pSubMesh[i].TextureID ].Height,
                      TexName );
        }
        else
        {
            x_printf( "%3d] Mesh: %3d   TexID: %2d  (%3d,%3d)\n",
                      i,
                      m_pSubMesh[i].ParentMesh,
                      m_pSubMesh[i].TextureID,
                      m_pTexture[ m_pSubMesh[i].TextureID ].Width,
                      m_pTexture[ m_pSubMesh[i].TextureID ].Height );
        }
    }


    x_printf( "\n\nSUBMESH STRIP INFO:\n" );
    x_printf( "   Mtx[Grps Slots Loads] GPStrips CPUStrips GP Vtx/Tri CPU Vtx/Tri\n" );
    for( i = 0; i < m_NSubmeshes; i++ )
    {
        NMatrixGroups += m_pSubMesh[i].NMtxGroups;
        NMtxSlots     += m_pSubMesh[i].NMtxSlots;
        NMatrixLoads  += m_pSubMesh[i].NMtxLoads;
        NTriStripsGP  += m_pSubMesh[i].NTriStripsGP;
        NTriStripsCPU += m_pSubMesh[i].NTriStripsCPU;

        x_printf( "%3d]   %3d  %3d   %3d    %3d      %3d       %3.2f       %3.2f\n",
                  i,
                  m_pSubMesh[i].NMtxGroups,
                  m_pSubMesh[i].NMtxSlots,
                  m_pSubMesh[i].NMtxLoads,
                  m_pSubMesh[i].NTriStripsGP,
                  m_pSubMesh[i].NTriStripsCPU,
                  m_pSubMesh[i].AvgVtxPerTriGP,
                  m_pSubMesh[i].AvgVtxPerTriCPU );
    }
    x_printf( "\nTOT]   %3d  %3d   %3d    %3d     %3d\n",
              NMatrixGroups,
              NMtxSlots,
              NMatrixLoads,
              NTriStripsGP,
              NTriStripsCPU );


    x_printf( "\n\nSUBMESH TRI COUNTS:\n" );
    for( i = 0; i < m_NSubmeshes; i++ )
    {
        NTrisGP    += m_pSubMesh[i].NTrisGP;
        NTrisCPU   += m_pSubMesh[i].NTrisCPU;
        DLSizeGP   += m_pSubMesh[i].SizeDListGP;
        DLSizeCPU  += m_pSubMesh[i].SizeDListCPU;

        x_printf( "%3d] %4d :GP  %4d :CPU   %4d :DListGP  %4d :DListCPU\n",
                  i,
                  m_pSubMesh[i].NTrisGP,
                  m_pSubMesh[i].NTrisCPU,
                  m_pSubMesh[i].SizeDListGP,
                  m_pSubMesh[i].SizeDListCPU );
    }
    x_printf( "\nTOT] %4d :GP  %4d :CPU   %4d :DListGP  %4d :DListCPU\n",
              NTrisGP,
              NTrisCPU,
              DLSizeGP,
              DLSizeCPU );


    x_printf( "\n\nSUBMESH VERT COUNTS:\n" );
    for( i = 0; i < m_NSubmeshes; i++ )
    {
        NVertsGP   += m_pSubMesh[i].NVertsGP;
        NVertsCPU1 += m_pSubMesh[i].NVertsCPU1;
        NVertsCPU2 += m_pSubMesh[i].NVertsCPU2;
        NVertsCPU3 += m_pSubMesh[i].NVertsCPU3;

        x_printf( "%3d] %4d :GP  %4d :CPU1  %4d :CPU2  %4d :CPU3\n",
                  i,
                  m_pSubMesh[i].NVertsGP,
                  m_pSubMesh[i].NVertsCPU1,
                  m_pSubMesh[i].NVertsCPU2,
                  m_pSubMesh[i].NVertsCPU3 );
    }
    x_printf( "\nTOT] %4d :GP  %4d :CPU1  %4d :CPU2  %4d :CPU3\n",
              NVertsGP,
              NVertsCPU1,
              NVertsCPU2,
              NVertsCPU3 );

    //--- Display stats
    x_printf( "\n\nSTATS:\n" );
    x_printf( "%5d Verts\n", (s32)(NVertsGP + NVertsCPU1 + NVertsCPU2 + NVertsCPU3) );
    x_printf( "%5d Tris\n", (s32)(NTrisGP + NTrisCPU) );
    x_printf( "%5d Textures\n", m_NTextures );
    x_printf( "%5d Submeshes\n", m_NSubmeshes );
    x_printf( "\n" );
}

//==========================================================================

void QGameCubeRipSkin::Save( char* pFilename )
{
    char             TextureName[SKIN_TEXTURE_NAME_LENGTH];
    GCSkinFileHeader Header;
    GCSkinMesh*      pMeshData;
    GCSkinSubMesh*   pSubMeshData;
    GCSkinMorphTgt*  pMorphData;

    X_FILE*         pFile;
    s32             i, j;
    GCSkinMesh*     pCurMesh;
    GCSkinSubMesh*  pCurSubMesh;


    //--- Allocate the space for data to export
    pMeshData     = (GCSkinMesh*)    x_malloc( sizeof(GCSkinMesh)     * (m_NMeshes + 1) );
    pSubMeshData  = (GCSkinSubMesh*) x_malloc( sizeof(GCSkinSubMesh)  * (m_NSubmeshes + 1) );
    pMorphData    = (GCSkinMorphTgt*)x_malloc( sizeof(GCSkinMorphTgt) * (m_NMorphTargets + 1) );

    ASSERT( pMeshData != NULL );
    ASSERT( pSubMeshData != NULL );
    ASSERT( pMorphData != NULL );

    ASSERT( m_pVtxData != NULL );
    ASSERT( m_pDListData != NULL );

    //--- Copy/build info for each mesh
    for( i = 0; i < m_NMeshes; i++ )
    {
        pCurMesh = &pMeshData[i];

        FillName( pCurMesh->Name, m_pMesh[i].Name, SKIN_MESH_NAME_LENGTH );
        pCurMesh->Flags = SKIN_MESH_FLAG_VISIBLE;

        if( m_pMesh[i].bAlpha )             pCurMesh->Flags |= SKIN_MESH_FLAG_ALPHA;
        if( m_pMesh[i].bEnvMapped )         pCurMesh->Flags |= SKIN_MESH_FLAG_ENVMAPPED;
        if( m_pMesh[i].bShadow )            pCurMesh->Flags |= SKIN_MESH_FLAG_SHADOW;
        if( m_pMesh[i].NMorphTargets > 0 )  pCurMesh->Flags |= SKIN_MESH_FLAG_MORPH;

        pCurMesh->Flags              = GC_ENDIAN_32( (u32)pCurMesh->Flags );
        pCurMesh->FirstSubMesh       = GC_ENDIAN_16( (u16)m_pMesh[i].FirstSubmesh );
        pCurMesh->NSubMeshes         = GC_ENDIAN_16( (u16)m_pMesh[i].NSubmeshes );
        pCurMesh->FirstMorphTarget   = GC_ENDIAN_16( (u16)m_pMesh[i].FirstMorphTgtID );
        pCurMesh->NMorphTargets      = GC_ENDIAN_16( (u16)m_pMesh[i].NMorphTargets );

        //--- copy/build submesh info
        j = m_pMesh[i].FirstSubmesh;
        while( j < (m_pMesh[i].FirstSubmesh + m_pMesh[i].NSubmeshes) )
        {
            pCurSubMesh = &pSubMeshData[j];

            pCurSubMesh->Flags = SKIN_SUBMESH_FLAG_VISIBLE;
            if( m_pSubMesh[j].bAlpha )      pCurSubMesh->Flags |= SKIN_SUBMESH_FLAG_ALPHA;
            if( m_pSubMesh[j].bEnvPass )    pCurSubMesh->Flags |= SKIN_SUBMESH_FLAG_ENVPASS;
            if( m_pSubMesh[j].bShadow )     pCurSubMesh->Flags |= SKIN_SUBMESH_FLAG_SHADOW;
            if( m_pSubMesh[j].bMorph )      pCurSubMesh->Flags |= SKIN_SUBMESH_FLAG_MORPH;

            pCurSubMesh->Flags        = GC_ENDIAN_32( (u32)pCurSubMesh->Flags );
            pCurSubMesh->TextureID    = GC_ENDIAN_16( (s16)m_pSubMesh[j].TextureID );
            pCurSubMesh->DListSizeCPU = GC_ENDIAN_32( (u32)m_pSubMesh[j].SizeDListCPU );
            pCurSubMesh->DListSizeGP  = GC_ENDIAN_32( (u32)m_pSubMesh[j].SizeDListGP );
            pCurSubMesh->NVertsGP     = GC_ENDIAN_16( (u16)m_pSubMesh[j].NVertsGP );
            pCurSubMesh->NVertsCPU1   = GC_ENDIAN_16( (u16)m_pSubMesh[j].NVertsCPU1 );
            pCurSubMesh->NVertsCPU2   = GC_ENDIAN_16( (u16)m_pSubMesh[j].NVertsCPU2 );
            pCurSubMesh->NVertsCPU3   = GC_ENDIAN_16( (u16)m_pSubMesh[j].NVertsCPU3 );
            pCurSubMesh->pDListCPU    = (void*)          GC_ENDIAN_32( m_pSubMesh[j].OffsetDListCPU );
            pCurSubMesh->pDListGP     = (void*)          GC_ENDIAN_32( m_pSubMesh[j].OffsetDListGP );
            pCurSubMesh->pUVs         = (GCSkinVert_UV*) GC_ENDIAN_32( m_pSubMesh[j].OffsetUVs );
            pCurSubMesh->pVtxCPU      = (GCSkinVert_CPU*)GC_ENDIAN_32( m_pSubMesh[j].OffsetVtxCPU );
            pCurSubMesh->pVtxGP       = (GCSkinVert_PN*) GC_ENDIAN_32( m_pSubMesh[j].OffsetVtxGP );
            pCurSubMesh->pVtxShadow   = (GCSkinVert_P*)  GC_ENDIAN_32( m_pSubMesh[j].OffsetVtxShadow );
            pCurSubMesh->pDeltasCPU   = (GCSkinDelta*)   GC_ENDIAN_32( m_pSubMesh[j].OffsetDeltasCPU );
            pCurSubMesh->NDeltasCPU   = GC_ENDIAN_16( (u16)m_pSubMesh[j].NDeltasCPU );

            j++;
        }

        //--- copy/build morph target info
        j = m_pMesh[i].FirstMorphTgtID;
        while( j < (m_pMesh[i].FirstMorphTgtID + m_pMesh[i].NMorphTargets) )
        {
            FillName( pMorphData[j].Name, m_pMorphTarget[j].Name, SKIN_MORPH_NAME_LENGTH );
            if( j == m_pMesh[i].FirstMorphTgtID )
            {
                pMorphData[j].Active = GC_ENDIAN_32( (s32)TRUE );
                pMorphData[j].Weight = GC_ENDIAN_32( (f32)1.0f );
            }
            else
            {
                pMorphData[j].Active = GC_ENDIAN_32( (s32)FALSE );
                pMorphData[j].Weight = GC_ENDIAN_32( (f32)0.0f );
            }

            j++;
        }
    }

    //--- calculate amount of padding needed to align vertex and DList data on 32B
    u32  NPadBytesDeltas  = 0;
    u32  NPadBytesVtxData = 0;
    byte BytePad32B[32];

    x_memset( BytePad32B, 0, 32 );

    NPadBytesVtxData  = m_NTextures     * SKIN_TEXTURE_NAME_LENGTH;
    NPadBytesVtxData += m_NBones        * SKIN_BONE_NAME_LENGTH;
    NPadBytesVtxData += m_NMeshes       * sizeof(GCSkinMesh);
    NPadBytesVtxData += m_NSubmeshes    * sizeof(GCSkinSubMesh);
    NPadBytesVtxData += m_NMorphTargets * sizeof(GCSkinMorphTgt);

    NPadBytesDeltas   = ALIGN_32(NPadBytesVtxData) - NPadBytesVtxData;
    NPadBytesVtxData += NPadBytesDeltas;

    NPadBytesVtxData += m_DeltaDataSize;

    NPadBytesVtxData  = ALIGN_32(NPadBytesVtxData) - NPadBytesVtxData;

    //--- setup file header
    FillName( Header.Name, m_Name, SKIN_NAME_LENGTH );
    Header.Flags            = GC_ENDIAN_32( (u32)0 );
    Header.NTextures        = GC_ENDIAN_32( (s32)m_NTextures );
    Header.NBones           = GC_ENDIAN_32( (s32)m_NBones );
    Header.NMeshes          = GC_ENDIAN_32( (s32)m_NMeshes );
    Header.NSubMeshes       = GC_ENDIAN_32( (s32)m_NSubmeshes );
    Header.NMorphTargets    = GC_ENDIAN_32( (s32)m_NMorphTargets );
    Header.NPadBytesDeltas  = GC_ENDIAN_32( (s32)NPadBytesDeltas );
    Header.NPadBytesVtxData = GC_ENDIAN_32( (s32)NPadBytesVtxData );
    Header.DeltaDataSize    = GC_ENDIAN_32( (u32)m_DeltaDataSize );
    Header.VertexDataSize   = GC_ENDIAN_32( (u32)m_VtxDataSize );
    Header.DListDataSize    = GC_ENDIAN_32( (u32)m_DListDataSize );


    //--- Open the file for export
    pFile = x_fopen( pFilename, "wb" );
    if( pFile == NULL )
    {
        x_printf( "ERROR: Unable to open %s for export.\n", pFilename );
        PauseOnError();
        exit( 0 );
    }

    //--- write out the header
    x_fwrite( &Header, sizeof(GCSkinFileHeader), 1, pFile );

    //--- write the texture names
    for( i = 0; i < m_NTextures; i++ )
    {
        GetTextureName( TextureName, m_pTexture[i].Name );
        x_fwrite( TextureName, sizeof(char), SKIN_TEXTURE_NAME_LENGTH, pFile );
    }

    //--- write the bone names
    for( i = 0; i < m_NBones; i++ )
    {
        x_fwrite( m_pBone[i].Name, sizeof(char), SKIN_BONE_NAME_LENGTH, pFile );
    }

    //--- write mesh, submesh, morph target, vertex, and other data
    x_fwrite( pMeshData,    sizeof(GCSkinMesh),    m_NMeshes,    pFile );
    x_fwrite( pSubMeshData, sizeof(GCSkinSubMesh), m_NSubmeshes, pFile );

    if( m_NMorphTargets > 0 )
        x_fwrite( pMorphData, sizeof(GCSkinMorphTgt), m_NMorphTargets, pFile );

    if( NPadBytesDeltas > 0 )
        x_fwrite( BytePad32B, sizeof(byte), NPadBytesDeltas, pFile );

    if( m_DeltaDataSize > 0 )
        x_fwrite( m_pDeltaData, sizeof(byte), m_DeltaDataSize, pFile );

    if( NPadBytesVtxData > 0 )
        x_fwrite( BytePad32B, sizeof(byte), NPadBytesVtxData, pFile );

    x_fwrite( m_pVtxData,   sizeof(byte), m_VtxDataSize,   pFile );
    x_fwrite( m_pDListData, sizeof(byte), m_DListDataSize, pFile );

    //--- close the file
    x_fclose( pFile );

    //--- free memory used for export
    x_free( pMeshData );
    x_free( pSubMeshData );
    x_free( pMorphData );
}


////////////////////////////////////////////////////////////////////////////
// Internal functions to QGameCubeRipSkin
////////////////////////////////////////////////////////////////////////////

xbool QGameCubeRipSkin::IsEnvPass( s32 SubMeshID )
{
    return m_pSubMesh[SubMeshID].bEnvPass;
}

//==========================================================================

xbool QGameCubeRipSkin::IsShadow( s32 SubMeshID )
{
    return m_pSubMesh[SubMeshID].bShadow;
}

//==========================================================================

xbool QGameCubeRipSkin::IsAlpha( s32 SubMeshID )
{
    return m_pSubMesh[SubMeshID].bAlpha;
}

//==========================================================================

xbool QGameCubeRipSkin::IsMorph( s32 SubMeshID )
{
    return m_pSubMesh[SubMeshID].bMorph;
}

//==========================================================================

void QGameCubeRipSkin::ComputeNormals( void )
{
    struct SNormalVert
    {
        vector3 Pos;
        vector3 Normal;
    };

    s32          i, j, TriID;
    s32          NTempVerts;
    SNormalVert* pTempVerts;

    //--- Clear normals
    for( i = 0; i < m_NVerts; i++ )
    {
        m_pVert[i].Normal.Zero();
    }

    //---   allocate some temp space
    pTempVerts = new SNormalVert[m_NVerts];
    ASSERT( pTempVerts != NULL );
    NTempVerts = 0;

    //=== to avoid normals looking weird across texture seams, do them
    //    based on position (NOT INDEX)

    //--- Create a list of vertices with nothing but position
    for( i = 0; i < m_NVerts; i++ )
    {
        //--- find this vert in the temporary list
        for( j = 0; j < NTempVerts; j++ )
        {
            if( (x_abs(pTempVerts[j].Pos.X - m_pVert[i].Pos.X) < EPSILON) &&
                (x_abs(pTempVerts[j].Pos.Y - m_pVert[i].Pos.Y) < EPSILON) &&
                (x_abs(pTempVerts[j].Pos.Z - m_pVert[i].Pos.Z) < EPSILON) )
            {
                break;
            }
        }

        if( j == NTempVerts )
        {
            //--- the current vert wasn't found, so add it
            pTempVerts[NTempVerts].Normal.Zero();
            pTempVerts[NTempVerts].Pos = m_pVert[i].Pos;
            NTempVerts++;
        }
    }

    //--- for each vert that a triangle uses, add in the triangle's normal, weighted by its area
    for( TriID = 0; TriID < m_NTris; TriID++ )
    {
        for( i = 0; i < 3; i++ )
        {
            //--- find each of the verts in the temporary list
            for( j = 0; j < NTempVerts; j++ )
            {
                if( (x_abs(pTempVerts[j].Pos.X - m_pVert[m_pTri[TriID].VertID[i]].Pos.X) < EPSILON) &&
                    (x_abs(pTempVerts[j].Pos.Y - m_pVert[m_pTri[TriID].VertID[i]].Pos.Y) < EPSILON) &&
                    (x_abs(pTempVerts[j].Pos.Z - m_pVert[m_pTri[TriID].VertID[i]].Pos.Z) < EPSILON) )
                {
                    break;
                }
            }
            ASSERT( j < NTempVerts );

            pTempVerts[j].Normal += m_pTri[TriID].Normal * m_pTri[TriID].Area;
        }
    }


    //--- now, copy the temp verts normals back into the real normals
    for( i = 0; i < NTempVerts; i++ )
    {
        //--- find every occurence of the temp vert in the mesh
        for( j = 0; j < m_NVerts; j++ )
        {
            if( (x_abs(pTempVerts[i].Pos.X - m_pVert[j].Pos.X) < EPSILON) &&
                (x_abs(pTempVerts[i].Pos.Y - m_pVert[j].Pos.Y) < EPSILON) &&
                (x_abs(pTempVerts[i].Pos.Z - m_pVert[j].Pos.Z) < EPSILON) )
            {
                m_pVert[j].Normal = pTempVerts[i].Normal;
            }
        }
    }

    //--- clean up
    delete[] pTempVerts;

    //--- Make sure the normals have length of 1.0
    for( i = 0; i < m_NVerts; i++ )
    {
        m_pVert[i].Normal.Normalize();
    }
}

//=====================================================================================================================================
void QGameCubeRipSkin::BlendVertexNormals( void )
{
    VertList        VList;
    VertList*       pCurrentList;
    VertNode*       pTestNode;

    vector3*        pvTestPosition;
    vector3*        pvArchivedPosition;
    vector3*        pvTestNormal;
    vector3*        pvArchivedNormal;
    f32             NormalDot;
    s32             VertID;

    s32             CommonVerts;
    vector3         AvgNormal;
    
    // Go through the entire skin to find duplicate points.    
    if( g_Verbose )
    {
        x_printf( "Blending the vertex normals.\n" );
    }

    // Go through each of the vertices to find any vertex position that shares the
    // same space as any other vertex, and whos normal shares the same half space.
    // (meaning the normals aren't facing in completely opposite direction )
    for( VertID = 0; VertID < m_NVerts; VertID++ )
    {
        if( g_Verbose )
        {
            if( VertID % 25 == 0 )
            {
                x_printf( "." );
            }
        }

        // Start at the top of the list.
        pCurrentList = &VList;

        // Grab the vertex to test against.
        pvTestPosition = &m_pVert[VertID].Pos;
        pvTestNormal   = &m_pVert[VertID].Normal;

        // Loop through the list of points until something is found.
        while( TRUE )
        {
            pTestNode = pCurrentList->mpHead;

            // If there is a node there, test the stored vertex with the current vert.
            if( pTestNode )
            {
                pvArchivedPosition = &m_pVert[pTestNode->VertID].Pos;
                pvArchivedNormal   = &m_pVert[pTestNode->VertID].Normal;

                NormalDot = pvTestNormal->Dot( *pvArchivedNormal );

                // Test to see if this vertex is a duplicate.
                if( (x_abs( pvTestPosition->X - pvArchivedPosition->X) < EPSILON) &&
                    (x_abs( pvTestPosition->Y - pvArchivedPosition->Y) < EPSILON) &&
                    (x_abs( pvTestPosition->Z - pvArchivedPosition->Z) < EPSILON) && 
                    NormalDot > 0.0f )
                {
                    // Add the duplicate node to the current list, then break from the while loop
                    pCurrentList->AddNode( -1, VertID );
                    break;
                }
                // Not a duplicate, so walk the lists of lists to the next linked list.
                else
                {
                    // If you can walk the list, then go to the next list.
                    if( pCurrentList->mpNextList )
                    {
                        pCurrentList = pCurrentList->mpNextList;
                    }
                    // If there are no more lists, and you haven't found a match, this must be a unique
                    // vertex, so create a new Linked list.
                    else
                    {
                        VList.AttachList( -1, VertID );
                        break;
                    }
                }
            }
            // There are no nodes in this list, so just add this one.
            else
            {
                pCurrentList->AddNode( -1, VertID );
                break;
            }
        }
    }

    if( g_Verbose )
    {
        x_printf( "\n" );
    }


    // Now we have a linked list of linked lists where each 2nd generation list has a chain of verts that share the same
    // space.  Time to average these normals and shove them back into the correct place.
    //-------------------------------------------------------------------------------------------------------------------------------------
    pCurrentList = &VList;
    
    while( pCurrentList )
    {
        // Find how many verts share this point, and add average any normals that are different.
        CommonVerts = 0;
        AvgNormal.Zero( );

        pTestNode = pCurrentList->mpHead;
        while( pTestNode )
        {
            CommonVerts++;

            AvgNormal.X += m_pVert[pTestNode->VertID].Normal.X;
            AvgNormal.Y += m_pVert[pTestNode->VertID].Normal.Y;
            AvgNormal.Z += m_pVert[pTestNode->VertID].Normal.Z;

            pTestNode = pTestNode->pNext;
        }
        

        // No find the average and set that average for all of those normals.
        AvgNormal.X /= CommonVerts;
        AvgNormal.Y /= CommonVerts;
        AvgNormal.Z /= CommonVerts;

        AvgNormal.Normalize( );

        pTestNode = pCurrentList->mpHead;
        while( pTestNode )
        {
            m_pVert[pTestNode->VertID].Normal.X = AvgNormal.X;
            m_pVert[pTestNode->VertID].Normal.Y = AvgNormal.Y;
            m_pVert[pTestNode->VertID].Normal.Z = AvgNormal.Z;
            pTestNode = pTestNode->pNext;
        }

        pCurrentList = pCurrentList->mpNextList;
    }
}

//==========================================================================
