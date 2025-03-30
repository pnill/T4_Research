///////////////////////////////////////////////////////////////////////////
//  RipSkin.cpp
///////////////////////////////////////////////////////////////////////////

#include <stdlib.h>

#include "x_files.hpp"
#include "PS2_RipSkin.h"
#define TARGET_PS2
#include "Skin.hpp"
#undef TARGET_PS2
#include "strippack.h"
#include "rip_helper.h"
#include "PS2Help.h"

////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////

#define NUM_PATH_STARTS         0.01f   // # of different starting places to try
#define STRIP_OPTIMIZE_TRIES    0
//#define NUM_PATH_STARTS         1.0f    // # of different starting places to try
//#define STRIP_OPTIMIZE_TRIES    100

#define INPUT_VERTS_NORMAL   33     // #### SET THIS VALUE PROPERLY
#define INPUT_VERTS_ENVMAP   33     // #### SET THIS VALUE PROPERLY
#define INPUT_VERTS_MORPH    33     // #### SET THIS VALUE PROPERLY

#define CACHE_NVERTS            128
#define CACHE_NTRIS             (CACHE_NVERTS * 3)
#define MAX_TRANSFORMS          256

#define NUKE_CACHE_PENALTY      900000
#define TRANSFORM_PENALTY       100000
#define VERT_LOAD_PENALTY         1000
#define LAST_1_TRI_BONUS           500
#define LAST_2_TRI_BONUS           100
#define LAST_3_TRI_BONUS            50
#define LAST_4_TRI_BONUS            20
#define NEIGHBOR_ADJACENT_BONUS     10
#define NEIGHBOR_SATISFIED_BONUS    20

#define CACHE_FLUSH_BOUNDARY       120

#define EPSILON                 0.0001f

////////////////////////////////////////////////////////////////////////////
// Typedefs
////////////////////////////////////////////////////////////////////////////

//==========================================================================
// t_CacheEntry and t_Cache are what the vertex cache would look like at
// any step along the way in both our test cases and the final run.
//==========================================================================

typedef struct SCacheEntry
{
    xbool Used;     // whether or not this vert in the cache is used
    s32   VertID;   // if it is used, the corresponding vert in m_pVerts
} t_CacheEntry;

typedef struct SCache
{
    s32             NUsed;              // the number of used verts in the cache
    t_CacheEntry    Vert[CACHE_NVERTS]; // a snapshot of the cache verts
} t_Cache;

//==========================================================================
// t_CacheVertList is the vert list for a single transform of a single frame
//==========================================================================

typedef struct SCacheVertList
{
    s32 NVerts;                 // # of verts using this transform
    s16 VertID[CACHE_NVERTS];   // corresponding verts in m_pVerts
    s16 CacheID[CACHE_NVERTS];  // corresponding verts in t_Cache
    s16 NMatrices;              // # of matrices used by the transforms
    s16 MatrixID[8];            // matrices used by this transform
} t_CacheVertList;

//==========================================================================
// t_CacheTriList is the tri list for a single frame
//==========================================================================

typedef struct SCacheTriList
{
    s32 NTris;                  // # of tris in this cache frame
    s32 TriID[CACHE_NTRIS];     // correspoding tri in m_pTri
    s32 CacheID[CACHE_NTRIS*3]; // which verts in cache each tri uses
} t_CacheTriList;

//==========================================================================
// t_CacheFrame is one frame of the cache, which means, a load of verts
// along with a list of tris using those verts
//==========================================================================

typedef struct SCacheFrame
{
    t_CacheVertList Transform[MAX_TRANSFORMS];  // verts and cache organized by transforms they use
    t_CacheTriList  Tri;                        // tris in cache
    t_StripPack*    pPack;                      // packs of tris in strip from
    s32             NPacks;                     // # of packs of tris
} t_CacheFrame;

//==========================================================================
// t_PathTri uses an array of tris to wind a path, slurping up verts as it
// goes, trying to maximize vertex re-usage.
//==========================================================================

typedef struct SPathTri
{
    s32     BestPathID;               // the best path triangle ids
    f32     BestPathRelativeError;    // the best path's error for this tri
    f32     BestPathTotalError;       // the best path's error total so far
    s32     TestPathID;               // which triangle we choose at each iteration
    f32     TestPathRelativeError;    // score of triangle chosen at each iteration
    f32     TestPathTotalError;       // running total of score so far
} t_PathTri;

//==========================================================================
// t_MorphData contains info used by the ExportFrameTransforms() function
// to properly build the delta packets and also set up s_pExpDeltaLocs and
// s_pExpDeltaValues
//==========================================================================

typedef struct SMorphData
{
    s32     ByteOffsetInVUMem;        // the offset in VU mem to alter
    s32     VertID;                   // the VertID in m_pVert
} t_MorphData;

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
// STATICS
////////////////////////////////////////////////////////////////////////////

static t_Cache          s_Cache;
static t_CacheFrame*    s_pFrame = NULL;
static s32              s_NFrames = 0;
static s32              s_NFramesAllocated = 0; 
static s32              s_NVertsLoaded;
static s32              s_NTriFlushes;
static s32              s_TotalTrisFlushed;
static s32              s_TotalVertsFlushed;
static s32              s_NTransformSwitches;
static t_PathTri*       s_pTri;
static xbool            s_Record = FALSE;

////////////////////////////////////////////////////////////////////////////
// STATICS USED BY EXPORT PROCESS
////////////////////////////////////////////////////////////////////////////

static byte*                s_pDataStart;
static s32                  s_NExpCacheFrames = 0;
static s32                  s_NExpCacheFramesAllocated = 0;
static t_SkinCacheFrame*    s_pExpCacheFrames = NULL;
static s32                  s_NExpVertLoads = 0;
static s32                  s_NExpVertLoadsAllocated = 0;
static t_SkinVertLoad*      s_pExpVertLoads = NULL;
static s32                  s_NExpDeltaLocs = 0;
static s32                  s_NExpDeltaLocsAllocated = 0;
static t_SkinDeltaLoc*      s_pExpDeltaLocs = NULL;
static s32                  s_NExpDeltaValues = 0;
static s32                  s_NExpDeltaValuesAllocated = 0;
static t_SkinDeltaValue*    s_pExpDeltaValues = NULL;
static byte*                s_pExpDeltaData;
static s32                  s_ExpDeltaDataSize;
static byte*                s_pCurrMeshDeltaPacket;

static t_MorphData*         s_pMorphData = NULL;
static s32                  s_NMorphDatas = 0;
static s32                  s_NMorphDatasAllocated = 0;

////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////

//==========================================================================
// Public functions
//==========================================================================

QPS2RipSkin::QPS2RipSkin( void )
{
    m_pVert         = NULL;
    m_pTri          = NULL;
    m_pSubMesh      = NULL;
    m_pMesh         = NULL;
    m_pTexture      = NULL;
    m_pTransform    = NULL;
    m_pBone         = NULL;
    m_pMorphTarget  = NULL;
    m_pMorphDelta   = NULL;

    m_NVerts        = 0;
    m_NTris         = 0;
    m_NSubmeshes    = 0;
    m_NMeshes       = 0;
    m_NTextures     = 0;
    m_NTransforms   = 0;
    m_NBones        = 0;
    m_NMorphTargets = 0;
    m_NMorphDeltas  = 0;


    m_NVertsAllocated           = 0;
    m_NTrisAllocated            = 0;
    m_NSubmeshesAllocated       = 0;
    m_NMeshesAllocated          = 0;
    m_NTexturesAllocated        = 0;
    m_NTransformsAllocated      = 0;
    m_NBonesAllocated           = 0;
    m_NMorphTargetsAllocated    = 0;
    m_NMorphDeltasAllocated     = 0;
}

//==========================================================================

QPS2RipSkin::~QPS2RipSkin( void )
{
    Clear();
    if ( s_pFrame )
    {
        x_free( s_pFrame );
        s_NFramesAllocated = 0;
        s_NFrames = 0;
    }
}

//==========================================================================

void QPS2RipSkin::Clear( void )
{
    //---   free up any data
    if ( m_pVert )
        x_free( m_pVert );
    if ( m_pTri )
        x_free( m_pTri );
    if ( m_pSubMesh )
        x_free( m_pSubMesh );
    if ( m_pMesh )
        x_free( m_pMesh );
    if ( m_pTexture )
        x_free( m_pTexture );
    if ( m_pTransform )
        x_free( m_pTransform );
    if ( m_pBone )
        x_free( m_pBone );
    if ( m_pMorphTarget )
        x_free( m_pMorphTarget );
    if ( m_pMorphDelta )
        x_free( m_pMorphDelta );

    //---   zero out the structure
    m_pVert         = NULL;
    m_pTri          = NULL;
    m_pSubMesh      = NULL;
    m_pMesh         = NULL;
    m_pTexture      = NULL;
    m_pTransform    = NULL;
    m_pBone         = NULL;
    m_pMorphTarget  = NULL;
    m_pMorphDelta   = NULL;

    m_NVerts        = 0;
    m_NTris         = 0;
    m_NSubmeshes    = 0;
    m_NMeshes       = 0;
    m_NTextures     = 0;
    m_NTransforms   = 0;
    m_NBones        = 0;
    m_NMorphTargets = 0;
    m_NMorphDeltas  = 0;

    m_NVertsAllocated           = 0;
    m_NTrisAllocated            = 0;
    m_NSubmeshesAllocated       = 0;
    m_NMeshesAllocated          = 0;
    m_NTexturesAllocated        = 0;
    m_NTransformsAllocated      = 0;
    m_NBonesAllocated           = 0;
    m_NMorphTargetsAllocated    = 0;
    m_NMorphDeltasAllocated     = 0;
}

//==========================================================================

static
void FillName( char* Name, char* src, s32 NChars )
{
    s32 length = x_strlen( src );
    s32 start;

    x_memset( Name, 0, NChars );

    if ( length < NChars )
        start = 0;
    else
        start = length - NChars + 1;

    x_strcpy( Name, &src[start] );
}

//==========================================================================

void QPS2RipSkin::SetSkinName( char* Name )
{
    FillName( m_Name, Name, 32 );
}

//==========================================================================

void QPS2RipSkin::AddBone( char* Name )
{
    s32 i;

    //---   Check if we need to allocate a new bone
    if( m_NBonesAllocated <= m_NBones )
    {
        m_NBonesAllocated += 32;
        m_pBone = (t_RipBone*)x_realloc( m_pBone,
                                         sizeof(t_RipBone) * m_NBonesAllocated );
        ASSERT( m_pBone );
    }

    //---   If this bone is already in the list, don't add another one
    for ( i = 0; i < m_NBones; i++ )
    {
        char TempName[33];
        FillName( TempName, Name, 32 );
        TempName[32] = '\0';
        if ( !x_stricmp( m_pBone[i].Name, TempName ) )
            return;
    }

    FillName( m_pBone[m_NBones].Name, Name, 32 );
    m_NBones++;
}

//==========================================================================

void QPS2RipSkin::NewTexture( char* TextureName, s32 Width, s32 Height )
{
    //---   Check if we need to allocate a new Texture
    if ( m_NTexturesAllocated <= m_NTextures )
    {
        m_NTexturesAllocated += 32;
        m_pTexture = (t_RipTexture*)x_realloc( m_pTexture,
                                               sizeof(t_RipTexture) * m_NTexturesAllocated );
        ASSERT( m_pTexture );
    }

    //---   Add Texture
    x_strcpy( m_pTexture[m_NTextures].Name, TextureName );
    m_pTexture[m_NTextures].Width = Width;
    m_pTexture[m_NTextures].Height = Height;

    //---   Increment number of Textures
    m_NTextures++;
}

//==========================================================================

void QPS2RipSkin::NewMesh( char* MeshName, xbool EnvMapped, xbool Shadow, xbool Alpha )
{
    //---   Check if we need to allocate a new mesh
    if ( m_NMeshesAllocated <= m_NMeshes )
    {
        m_NMeshesAllocated += 32;
        m_pMesh = (t_RipMesh*)x_realloc( m_pMesh,
                                         sizeof(t_RipMesh) * m_NMeshesAllocated );
        ASSERT( m_pMesh );
    }

    //---   Add mesh
    x_strcpy( m_pMesh[m_NMeshes].Name, MeshName );
    m_pMesh[m_NMeshes].EnvMapped = EnvMapped;
    m_pMesh[m_NMeshes].Shadow = Shadow;
    m_pMesh[m_NMeshes].Alpha = Alpha;
    m_pMesh[m_NMeshes].SubmeshID = -1;
    m_pMesh[m_NMeshes].NSubmeshes = 0;
    m_pMesh[m_NMeshes].NTris = 0;
    m_pMesh[m_NMeshes].TriID = m_NTris;
    m_pMesh[m_NMeshes].NVerts = 0;
    m_pMesh[m_NMeshes].VertID = m_NVerts;
    m_pMesh[m_NMeshes].NMorphTargets = 0;
    m_pMesh[m_NMeshes].MorphTargetID = m_NMorphTargets;

    //---   Increment number of meshes
    m_NMeshes++;
}

//==========================================================================

void QPS2RipSkin::AddVert( t_RipVert& rRipVert )
{
    //---   Check if we need to allocate a new Vert
    if ( m_NVertsAllocated <= m_NVerts )
    {
        m_NVertsAllocated += 1024;
        m_pVert = (t_RipVert*)x_realloc( m_pVert,
                                         sizeof(t_RipVert) * m_NVertsAllocated );
        ASSERT( m_pVert );
    }

    //---   Add vert to list
    m_pVert[m_NVerts] = rRipVert;
    m_NVerts++;
    m_pMesh[m_NMeshes - 1].NVerts++;
}

//==========================================================================

#define SWAP_VERTS(a,b)  { temp = a; a = b; b = temp; }

void QPS2RipSkin::AddTri( s32 Vert0, s32 Vert1, s32 Vert2, char* pTextureName )
{
    vector3     DV1,DV2;
    f32         Length;
    s32         TextureID;

    //---   get the verts in the QPS2RipSkin's offset by adding
    //      the mesh's starting vert
    Vert0 += m_pMesh[m_NMeshes-1].VertID;
    Vert1 += m_pMesh[m_NMeshes-1].VertID;
    Vert2 += m_pMesh[m_NMeshes-1].VertID;

    //---   Check if we need to allocate new tris
    if( m_NTrisAllocated <= m_NTris )
    {
        m_NTrisAllocated += 4096;
        m_pTri = (t_RipTri*)x_realloc( m_pTri,
                                       sizeof(t_RipTri) * m_NTrisAllocated );
        ASSERT( m_pTri );
    }

    //---   Check for a collapsed triangle, and don't add it if there is one
    if ( (m_pVert[Vert0].Pos == m_pVert[Vert1].Pos) ||
         (m_pVert[Vert0].Pos == m_pVert[Vert2].Pos) ||
         (m_pVert[Vert1].Pos == m_pVert[Vert2].Pos) )
    {
        return;
    }

    //---   Figure out texture id
    if ( m_NTextures == 0 )
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

    //---   Do a check to see if this triangle is already in our list (this
    //      will take out double-sided polys which is okay, 'cause the
    //      PlayStation2 doesn't do backface culling anyway)
    {
        s32 i;
        s32 temp;
        s32 VertsA[3];

        //---   get the verts in sorted order
        VertsA[0] = Vert0;
        VertsA[1] = Vert1;
        VertsA[2] = Vert2;
        if ( VertsA[0] > VertsA[1] )  SWAP_VERTS( VertsA[0], VertsA[1] );
        if ( VertsA[1] > VertsA[2] )  SWAP_VERTS( VertsA[1], VertsA[2] );
        if ( VertsA[0] > VertsA[1] )  SWAP_VERTS( VertsA[0], VertsA[1] );
        ASSERT( (VertsA[2] > VertsA[1]) && (VertsA[1] > VertsA[0]) );

        for ( i = 0; i < m_NTris; i++ )
        {
            t_RipTri*   pT;
            s32         VertsB[3];

            //---   grab a ptr to the tri
            pT = &m_pTri[i];

            //---   if its using a different texture, its okay to add
            if ( pT->TextureID != TextureID )
                continue;
            //---   if its part of a different mesh, its okay to add
            if ( pT->MeshID != (m_NMeshes-1) )
                continue;

            //---   get pT's verts in sorted order
            VertsB[0] = pT->VertID[0];
            VertsB[1] = pT->VertID[1];
            VertsB[2] = pT->VertID[2];
            if ( VertsB[0] > VertsB[1] )  SWAP_VERTS( VertsB[0], VertsB[1] );
            if ( VertsB[1] > VertsB[2] )  SWAP_VERTS( VertsB[1], VertsB[2] );
            if ( VertsB[0] > VertsB[1] )  SWAP_VERTS( VertsB[0], VertsB[1] );
            ASSERT( (VertsB[2] > VertsB[1]) && (VertsB[1] > VertsB[0]) );

            //---   now we can determine whether or not this triangle already
            //      exists in this mesh with this texture
            if ( (VertsA[0] == VertsB[0]) &&
                 (VertsA[1] == VertsB[1]) &&
                 (VertsA[2] == VertsB[2]) )
            {
                //---   don't add a duplicate poly
                return;
            }
        }
    }

    //---   Compute tri normal and area
    DV1 = m_pVert[Vert1].Pos - m_pVert[Vert0].Pos;
    DV2 = m_pVert[Vert2].Pos - m_pVert[Vert0].Pos;
    m_pTri[m_NTris].Normal = Cross( DV1, DV2 );
    Length = m_pTri[m_NTris].Normal.Length();

    // If the length of the normal is jacked it means that the polygon is a splinter,
    // so force it to be minute.  Basically this should
    if( Length <= 0.0f )
    {
        x_printf( "Triangle: %d, Verts: %d -> %d -> %d is a splinter\n", m_NTris, Vert0, Vert1, Vert2 );
        Length = 0.001f;
    }

    m_pTri[m_NTris].Area = Length * 0.5f;
    m_pTri[m_NTris].Normal *= (1.0f / Length);

    //---   Add tri to list
    m_pTri[m_NTris].VertID[0] = Vert0;
    m_pTri[m_NTris].VertID[1] = Vert1;
    m_pTri[m_NTris].VertID[2] = Vert2;
    m_pTri[m_NTris].MeshID = m_NMeshes - 1;
    m_pTri[m_NTris].TextureID = TextureID;

    //---   increment the tri count ptr
    m_NTris++;
    m_pMesh[m_NMeshes - 1].NTris++;
}

//==========================================================================

void QPS2RipSkin::NewMorphTarget( char* TargetName )
{
    //---   Check if we need to allocate a new morph target
    if ( m_NMorphTargetsAllocated <= m_NMorphTargets )
    {
        m_NMorphTargetsAllocated += 32;
        m_pMorphTarget = (t_RipMorphTarget*)x_realloc( m_pMorphTarget,
                                                       sizeof(t_RipMorphTarget) * m_NMorphTargetsAllocated );
        ASSERT( m_pMorphTarget );
    }

    //---   Add morph target
    FillName( m_pMorphTarget[m_NMorphTargets].Name, TargetName, 64 );
    m_pMorphTarget[m_NMorphTargets].DeltaID = m_NMorphDeltas;
    m_pMorphTarget[m_NMorphTargets].NDeltas = 0;

    //---   Increment number of morph targets
    m_pMesh[m_NMeshes - 1].NMorphTargets++;
    m_NMorphTargets++;
}

//==========================================================================

void QPS2RipSkin::AddMorphDelta( s32 VertID, vector3 Delta )
{
    //---   Check if we need to allocate more deltas
    if ( m_NMorphDeltasAllocated <= m_NMorphDeltas )
    {
        m_NMorphDeltasAllocated += 128;
        m_pMorphDelta = (t_RipMorphDelta*)x_realloc( m_pMorphDelta,
                                                     sizeof(t_RipMorphDelta) * m_NMorphDeltasAllocated );
        ASSERT( m_pMorphDelta );
    }

    //---   Add the delta
    m_pMorphDelta[m_NMorphDeltas].VertID = m_pMesh[m_NMeshes-1].VertID + VertID;
    m_pMorphDelta[m_NMorphDeltas].Delta = Delta;
    
    //---   let the vert know it may be morphed
    m_pVert[m_pMorphDelta[m_NMorphDeltas].VertID].HasMorphDelta = TRUE;

    //---   Increment number of morph deltas
    m_pMorphTarget[m_NMorphTargets-1].NDeltas++;
    m_NMorphDeltas++;
}

//==========================================================================

void QPS2RipSkin::ForceSingleMatrix( s32 VertID )
{
    s32 i;
    s32 MaxIndex=0;

    //---   find the matrix with the heighest weight
    for( i = 0; i < m_pVert[VertID].NMatrices; i++ )
    {
        if( m_pVert[VertID].MatrixWeight[i] > m_pVert[VertID].MatrixWeight[MaxIndex] )
            MaxIndex = i;
    }

    //---   force the vert to use the matrix with the highest weight
    m_pVert[VertID].MatrixID[0] = m_pVert[VertID].MatrixID[MaxIndex];
    m_pVert[VertID].MatrixWeight[0] = 1.0f;
    m_pVert[VertID].NMatrices = 1;
    /*m_pVert[VertID].MatrixID[0] = 0;     // ####
    m_pVert[VertID].MatrixWeight[0] = 1.0f;
    m_pVert[VertID].NMatrices = 1;*/
}

//==========================================================================

void QPS2RipSkin::ForceDoubleMatrix( s32 VertID )
{
    s32 MaxIndex0 = 0;
    s32 MaxIndex1 = 1;
    s32 TempIndices[8];
    f32 W,W0,W1;
    s32 i;

    //---   the MaxIndices start off as zero and one, but make sure that
    //      the weight of Index0 is greater than Index1
    if( m_pVert[VertID].MatrixWeight[MaxIndex0] < m_pVert[VertID].MatrixWeight[MaxIndex1] )
    {
        MaxIndex0 = 1;
        MaxIndex1 = 0;
    }

    //---   now find the two greatest weights
    for( i = 2; i < m_pVert[VertID].NMatrices; i++ )
    {
        f32 W = m_pVert[VertID].MatrixWeight[i];

        if( W > m_pVert[VertID].MatrixWeight[MaxIndex0] )
        {
            //---   W is the biggest weight so far
            if ( m_pVert[VertID].MatrixWeight[MaxIndex0] >
                 m_pVert[VertID].MatrixWeight[MaxIndex1] )
            {
                //---   since MaxIndex0 will be re-assigned to a greater weight,
                //      see if its bigger than MaxIndex1, and re-assign that too,
                //      if we can
                MaxIndex1 = MaxIndex0;
            }
            MaxIndex0 = i;
        }
        else
        {
            //---   is W bigger than the second-biggest weight?
            if( W > m_pVert[VertID].MatrixWeight[MaxIndex1] )
                MaxIndex1 = i;
        }
    }

    //---   do some calculations to get the total weight back to 1.0
    W0 = m_pVert[VertID].MatrixWeight[MaxIndex0];
    W1 = m_pVert[VertID].MatrixWeight[MaxIndex1];
    W  = 1.0f / (W0+W1);
    W0 *= W;
    W1 *= W;

    //---   re-assign the new weights
    for ( i = 0; i < m_pVert[VertID].NMatrices; i++ )
        TempIndices[i] = m_pVert[VertID].MatrixID[i];
    
    m_pVert[VertID].MatrixID[0] = TempIndices[MaxIndex0];
    m_pVert[VertID].MatrixWeight[0] = W0;
    m_pVert[VertID].MatrixID[1] = TempIndices[MaxIndex1];
    m_pVert[VertID].MatrixWeight[1] = W1;
    m_pVert[VertID].NMatrices = 2;

    //---   sanity check
    for ( i = 0; i < m_pVert[VertID].NMatrices - 1; i++ )
    {
        ASSERT( m_pVert[VertID].MatrixWeight[i] >=
                m_pVert[VertID].MatrixWeight[i + 1] );
    }
}

//==========================================================================

void QPS2RipSkin::ForceTripleMatrix( s32 VertID )
{
    xbool   done = FALSE;
    s32     MaxIndices[8];
    s32     TempIndices[8];
    f32     W, W0, W1, W2;
    s32     i;

    //---   default the max indices
    for ( i = 0; i < m_pVert[VertID].NMatrices; i++ )
        MaxIndices[i] = i;

    //---   now sort the indices based on their corresponding weight
    //      (just do a quick bubble sort)
    while ( !done )
    {
        done = TRUE;
        for ( i = 0; i < m_pVert[VertID].NMatrices - 1; i++ )
        {
            if ( m_pVert[VertID].MatrixWeight[MaxIndices[i + 1]] > 
                 m_pVert[VertID].MatrixWeight[MaxIndices[i]] )
            {
                s32 temp;
                temp = MaxIndices[i];
                MaxIndices[i] = MaxIndices[i + 1];
                MaxIndices[i + 1] = temp;
                done = FALSE;
            }
        }
    }

    //---   sanity check to make sure its sorted correctly
    for ( i = 0; i < m_pVert[VertID].NMatrices - 1; i++ )
    {
        ASSERT( m_pVert[VertID].MatrixWeight[MaxIndices[i]] >=
                m_pVert[VertID].MatrixWeight[MaxIndices[i + 1]] );
    }

    //---   do some calculations to get the total weight back to 1.0
    W0 = m_pVert[VertID].MatrixWeight[MaxIndices[0]];
    W1 = m_pVert[VertID].MatrixWeight[MaxIndices[1]];
    W2 = m_pVert[VertID].MatrixWeight[MaxIndices[2]];
    W  = 1.0f / (W0 + W1 + W2);
    W0 *= W;
    W1 *= W;
    W2 *= W;

    //---   re-assign the new weights
    for ( i = 0; i < m_pVert[VertID].NMatrices; i++ )
        TempIndices[i] = m_pVert[VertID].MatrixID[i];
    
    m_pVert[VertID].MatrixID[0] = TempIndices[MaxIndices[0]];
    m_pVert[VertID].MatrixWeight[0] = W0;
    m_pVert[VertID].MatrixID[1] = TempIndices[MaxIndices[1]];
    m_pVert[VertID].MatrixWeight[1] = W1;
    m_pVert[VertID].MatrixID[2] = TempIndices[MaxIndices[2]];
    m_pVert[VertID].MatrixWeight[2] = W2;
    m_pVert[VertID].NMatrices = 3;

    //---   sanity check
    for ( i = 0; i < m_pVert[VertID].NMatrices - 1; i++ )
    {
        ASSERT( m_pVert[VertID].MatrixWeight[i] >=
                m_pVert[VertID].MatrixWeight[i + 1] );
    }
}

//==========================================================================

#define INSIG_VALUE     0.03f

static
void RemoveInsigWeights( SRipVert* pVert )
{
    s32 i;
    s32 SrcID;
    s32 DstID;
    f32 TotalWeight;
    f32 W;

    //---   remove any insignificant weights in this vert
    for ( i = 0; i < pVert->NMatrices; i++ )
    {
        if ( pVert->MatrixWeight[i] < INSIG_VALUE )
            pVert->MatrixWeight[i] = 0.0f;
    }
    for ( i = pVert->NMatrices; i < 8; i++ )
        pVert->MatrixWeight[i] = 0.0f;  // just being safe here...

    //---   re-count the weights
    SrcID = DstID = 0;
    TotalWeight = 0.0f;
    while ( (DstID < 8) && (SrcID < 8) )
    {
        if ( pVert->MatrixWeight[SrcID] > 0.0f )
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
    for ( i = DstID; i < 8; i++ )
        pVert->MatrixWeight[DstID] = 0.0f;

    //---   re-normalize the weights
    if ( TotalWeight == 0.0f )
        W = 1.0f;
    else
        W = 1.0f / TotalWeight;

    for ( i = 0; i < pVert->NMatrices; i++ )
        pVert->MatrixWeight[i] *= W;
}

//==========================================================================

void QPS2RipSkin::ForceSingleMatrix( void )
{
    s32 i;

    for( i = 0; i < m_NVerts; i++ )
    {
        RemoveInsigWeights( &m_pVert[i] );
        ForceSingleMatrix( i );
    }
}

//==========================================================================

void QPS2RipSkin::ForceDoubleMatrix( void )
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

void QPS2RipSkin::ForceTripleMatrix( void )
{
    s32 i;

    for( i = 0; i < m_NVerts; i++ )
    {
        RemoveInsigWeights( &m_pVert[i] );
        if( m_pVert[i].NMatrices >= 3 )
            ForceTripleMatrix( i );
        else
        if ( m_pVert[i].NMatrices == 2 )
            ForceDoubleMatrix( i );
        else
            ForceSingleMatrix( i );
    }
}

//==========================================================================

static
s32 TriSortCompare( const void* Element1, const void* Element2 )
{
    t_RipTri* Tri1 = (t_RipTri*)Element1;
    t_RipTri* Tri2 = (t_RipTri*)Element2;

    //---   the sort criteria are the the MeshIndex and TextureIndex
    if ( Tri1->MeshID == Tri2->MeshID ) 
    {
        //---   the meshes are the same so, sort on texture id
        if ( Tri1->TextureID == Tri2->TextureID )
            return 0;

        if ( Tri1->TextureID > Tri2->TextureID )
            goto INBACK;
        else
            goto INFRONT;
    }

    if ( Tri1->MeshID > Tri2->MeshID )
        goto INBACK;
    else
        goto INFRONT;
    

    // TOGGLE THESE TO SWITCH ORDER
INFRONT:
    return -1;
INBACK:
    return 1;
}

//==========================================================================

static
void SortFrameTransforms( t_CacheFrame* pFrame )
{
    s32 i, j, k;

    //---   Try sorting transforms by lowest matrix in set
    for( i = 0; i < MAX_TRANSFORMS; i++ )
    {   
        s32 BestIndex = i;
        s32 BestMin0;
        s32 BestMin1;

        //---   Find lowest two matrices in list
        BestMin0 = 1000;
        BestMin1 = 1000;
        for( k = 0; k < pFrame->Transform[i].NMatrices; k++ )
        {
            s32 M = pFrame->Transform[i].MatrixID[k];
            if( M < BestMin0 )
            {
                BestMin1 = BestMin0;
                BestMin0 = M;
            }
            else
            if( M < BestMin1 )
            {
                BestMin1 = M;
            }
        }
        if( BestMin1 == 1000 )
            BestMin1 = BestMin0;

        for( j = i + 1; j < MAX_TRANSFORMS; j++ )
        {
            s32 Min0, Min1;

            //---   Find lowest two matrices in list
            Min0 = 1000;
            Min1 = 1000;
            for( k = 0; k < pFrame->Transform[j].NMatrices; k++ )
            {
                s32 M = pFrame->Transform[j].MatrixID[k];
                if( M < Min0 )
                {
                    Min1 = Min0;
                    Min0 = M;
                }
                else
                if( M < Min1 )
                {
                    Min1 = M;
                }
            }
            if( Min1 == 1000 )
                Min1 = Min0;

            //---   Check if B is lower than A
            if( (Min0 < BestMin0) || 
                ((Min0 == BestMin0) && (Min1 < BestMin1)) ||
                ((Min0 == BestMin0) && (pFrame->Transform[j].NMatrices < pFrame->Transform[BestIndex].NMatrices)) )
            {
                BestIndex = j;
                BestMin0 = Min0;
                BestMin1 = Min1;
            }
        }

        //---   Do swap
        if( BestIndex != i )
        {
            t_CacheVertList Temp;
            Temp = pFrame->Transform[i];
            pFrame->Transform[i] = pFrame->Transform[BestIndex];
            pFrame->Transform[BestIndex] = Temp;
        }
    }
}    

//==========================================================================

static
void BuildFrameStrip( s32 FrameID )
{
    s32           i;
    t_CacheFrame* pFrame;

    //---   grab a ptr to the frame
    pFrame = &s_pFrame[FrameID];

    //---   Write out transforms for debugging
    if ( g_ReportPathDetails )
    {
        x_printf( "============ CACHE FRAME #%1d ============\n", FrameID );
        x_printf( "NTRIS: %1d\n", pFrame->Tri.NTris );
        
        for( i = 0; i < MAX_TRANSFORMS; i++ )
        {
            if( pFrame->Transform[i].NVerts > 0 )
                x_printf( "Transform [%2d] %4d verts\n", i, pFrame->Transform[i].NVerts );
        }
        
        x_printf( "Packing strips for frame %1d of %1d\n", FrameID, s_NFrames );
    }
        
    //---   pack the strips
    STRIPPACK_PackStrips( &pFrame->pPack, 
                          pFrame->NPacks, 
                          pFrame->Tri.NTris,
                          pFrame->Tri.CacheID,
                          pFrame->Tri.TriID,
                          (SKIN_OUTPUT_BUFFER_SIZE - 1) / 3,
                          STRIP_OPTIMIZE_TRIES );

    if ( g_ReportPathDetails )
        x_printf("\n");
}

//==========================================================================

void QPS2RipSkin::PrepareStructures( void )
{
    s32 CurrentMeshID;
    s32 CurrentTextureID;
    s32 i, j, k;
    s32 TotalVertsLoaded = 0;
    s32 TotalTriFlushes = 0;
    s32 TotalTransforms = 0;

    //---   Compute Normals
    ComputeNormals();

    // Go through all of the vertex points in the entire skin, and blend any normals that share the same vertex position.
    //
    // The reason this needs to be done is because some vertices are copied to allow the skin to be broken up into 
    // several parts, such and hands, and legs etc...  These copied vertices, will not share the same polygon connections, 
    // and because of that, even though the vertices share the same space, they don't share the same vertex normal.
    // As a result you will see a distinct difference in the two connecting pieces because the lighting will emphasize the
    // different vertex normals.
    BlendVertexNormals( );

    //---   Build list of transforms
    BuildTransforms();

    //---   Sort tris for building submeshes
    x_qsort ( m_pTri,
              m_NTris,
              sizeof(t_RipTri),
              TriSortCompare );

    //---   Connect tris to verts
    ConnectTrisToVerts();

    //---   Connect tris to tris
    ConnectTrisToTris();

    //---   Build the submeshes
    //      REMEMBER: THE TRIS ARE ALREADY SORTED BY MESH AND TEXTURE
    i = 0;
    while ( 1 )
    {
        //---   Setup starting info
        CurrentMeshID    = m_pTri[i].MeshID;
        CurrentTextureID = m_pTri[i].TextureID;

        //---   Allocate more submeshes if needed
        if( m_NSubmeshesAllocated <= m_NSubmeshes )
        {
            m_NSubmeshesAllocated += 32;
            m_pSubMesh = (t_RipSubMesh*)x_realloc( m_pSubMesh,
                                                   sizeof(t_RipSubMesh) * m_NSubmeshesAllocated );
            ASSERT( m_pSubMesh );
        }

        //---   Loop until we find a break
        m_pSubMesh[m_NSubmeshes].EnvPass = FALSE;
        m_pSubMesh[m_NSubmeshes].Shadow = m_pMesh[CurrentMeshID].Shadow;
        m_pSubMesh[m_NSubmeshes].Alpha = m_pMesh[CurrentMeshID].Alpha;
        m_pSubMesh[m_NSubmeshes].TriID = i;
        m_pSubMesh[m_NSubmeshes].NTris = 1;
        m_pSubMesh[m_NSubmeshes].TextureID = CurrentTextureID;
        m_pSubMesh[m_NSubmeshes].MeshID = CurrentMeshID;
        i++;
        while( 1 )
        {
            if( i == m_NTris )
                break;
            if( m_pTri[i].MeshID != CurrentMeshID )
                break;
            if( (m_pTri[i].TextureID != CurrentTextureID) && (m_pMesh[CurrentMeshID].Shadow == FALSE) )
                break;
            m_pSubMesh[m_NSubmeshes].NTris++;
            i++;
        }
        m_NSubmeshes++;

        if ( m_pMesh[CurrentMeshID].EnvMapped )
        {
            //---   if this is an environment mapped sub-mesh, copy the
            //      sub-mesh over, only change the EnvPass bool value

            //---   Allocate more submeshes if needed
            if( m_NSubmeshesAllocated <= m_NSubmeshes )
            {
                m_NSubmeshesAllocated += 32;
                m_pSubMesh = (t_RipSubMesh*)x_realloc( m_pSubMesh,
                                                       sizeof(t_RipSubMesh) * m_NSubmeshesAllocated );
                ASSERT( m_pSubMesh );
            }

            x_memcpy( &m_pSubMesh[m_NSubmeshes],
                      &m_pSubMesh[m_NSubmeshes - 1],
                      sizeof(t_RipSubMesh) );
            m_pSubMesh[m_NSubmeshes].EnvPass = TRUE;
            m_NSubmeshes++;
        }

        //---   Check if we are finished
        if( i == m_NTris )
            break;
    }

    //---   Build the meshes
    for( i = 0; i < m_NMeshes; i++ )
    {
        s32 First,Last;

        First = -1;
        Last  = -1;
        for( j = 0; j < m_NSubmeshes; j++ )
        {
            if( First == -1 )
            {
                if( m_pSubMesh[j].MeshID == i )
                    First = j;
            }

            if( m_pSubMesh[j].MeshID == i )
                Last = j;
        }
        ASSERT( (First>=0) && (Last>=0) );

        m_pMesh[i].SubmeshID = First;
        m_pMesh[i].NSubmeshes = (Last - First) + 1;
    }

    //---   Mark which sub-meshes require morphing
    for ( i = 0; i < m_NSubmeshes; i++ )
    {
        m_pSubMesh[i].Morph = FALSE;
        for ( j = m_pSubMesh[i].TriID;
              j < (m_pSubMesh[i].TriID + m_pSubMesh[i].NTris);
              j++ )
        {
            if ( m_pVert[m_pTri[j].VertID[0]].HasMorphDelta ||
                 m_pVert[m_pTri[j].VertID[1]].HasMorphDelta ||
                 m_pVert[m_pTri[j].VertID[2]].HasMorphDelta )
            {
                m_pSubMesh[i].Morph = TRUE;
                break;
            }
        }
    }

    //---   Allocate structures for path description
    s_pTri = (t_PathTri*)x_malloc( sizeof(t_PathTri) * m_NTris );
    ASSERT( s_pTri );

    //---   Solve submeshes
    for ( i = 0; i < m_NSubmeshes; i++ )
    {
        if ( g_Verbose )
            x_printf( "************************** SUBMESH %3d **************************\n", i );
        SolveSubMesh( i );
    }

    //---   Sort transforms for all frames
    for( i = 0; i < s_NFrames; i++ )
    {
        SortFrameTransforms( &s_pFrame[i] );
    }

    //---   Let the user know whats going on
    x_printf( "Packing triangle strips...\n" );

    //---   Build strips for each Frame and pack
    for ( i = 0; i < m_NMeshes; i++ )
    {
        for ( j = m_pMesh[i].SubmeshID;
              j < (m_pMesh[i].SubmeshID + m_pMesh[i].NSubmeshes);
              j++ )
        {
            for ( k = m_pSubMesh[j].CacheFrameID;
                  k < (m_pSubMesh[j].CacheFrameID + m_pSubMesh[j].NCacheFrames);
                  k++ )
            {
                BuildFrameStrip( k );
            }
        }
    }

    if ( g_Verbose )
    {
        s32 TotalPacks = 0;
        s32 TotalVerts = 0;
        s32 TotalTris  = 0;

        //---   collect some stats about the packs
        for ( i = 0; i < s_NFrames; i++ )
        {
            for ( j = 0; j < s_pFrame[i].NPacks; j++ )
            {
                TotalVerts += s_pFrame[i].pPack[j].NVerts;
                TotalTris  += s_pFrame[i].pPack[j].NTris;
            }
            TotalPacks += s_pFrame[i].NPacks;
        }

        //---   Show some final stats
        x_printf("\n");
        x_printf("NPACKS         %1d\n",TotalPacks);
        x_printf("TOTAL VERTS    %1d\n",TotalVerts);
        x_printf("TOTAL TRIS     %1d\n",TotalTris);
        x_printf("VPT            %4.2f\n",(f32)TotalVerts / (f32)TotalTris );
    }

    //---   Write out some more stats
    x_printf("\nSUMMARY:\n");
    x_printf( "# Meshes in model:     %1d\n", m_NMeshes );
    x_printf( "# Submeshes in model   %1d\n", m_NSubmeshes );
    x_printf( "# Verts in model:      %1d\n", m_NVerts );
    x_printf( "# Tris in model:       %1d\n", m_NTris );
    x_printf( "# Transforms in model: %1d\n", m_NTransforms );
    for( i = 0; i < m_NSubmeshes; i++ )
    {
        TotalVertsLoaded += m_pSubMesh[i].NVertsLoaded;
        TotalTriFlushes  += m_pSubMesh[i].NTriFlushes;
        TotalTransforms  += m_pSubMesh[i].NTransforms;

        x_printf( "%2d] %4d:NT  %4d:VL  %4d:TFL  %4d:TFM\n",
                  i, 
                  m_pSubMesh[i].NTris,
                  m_pSubMesh[i].NVertsLoaded,
                  m_pSubMesh[i].NTriFlushes,
                  m_pSubMesh[i].NTransforms );
    }

    x_printf( "TT]          %4d:VL  %4d:TFL  %4d:TFM\n",
              TotalVertsLoaded,
              TotalTriFlushes,
              TotalTransforms );

    //---   Write out version of mesh showing bone weights
    if ( g_BoneWeightingFileName[0] != '\0' )
    {
        X_FILE*     fp;
        color       WeightColors[3] =
        {
            color( 0, 0, 255, 255 ),
            color( 0, 255, 255, 255 ),
            color( 255, 255, 0, 255 )
        };

        //---   let the user know whats going on
        x_printf( "Exporting bone weighting ASC file...\n" );

        //---   get the file handle
        fp = x_fopen( g_BoneWeightingFileName, "wt" );
        if ( !fp )
        {
            x_printf( "ERROR: Could not open %s for writing.\n", g_BoneWeightingFileName );
            PauseOnError();
            exit( 0 );
        }

        for ( i = 0; i < m_NMeshes; i++ )
        {
            char    Mesh[64];
            x_sprintf( Mesh, "%1c%1s%1c", '"', fs("MESH%03d", i), '"');
            for ( j = 0; j < m_pMesh[i].NTris; j++ )
            {
                t_RipTri*   pTri;
                t_RipVert*  pRV;

                pTri = &m_pTri[m_pMesh[i].TriID + j];
                    
                //---   add the tri
                pRV = &m_pVert[pTri->VertID[0]];
                ASSERT( pRV->NMatrices <= 3 );
                x_fprintf( fp, "[%f, %f, %f, 0, 0, 0, 0, 0, %1d, %1d, %1d, %1d, 0, %1s,\n",
                           pRV->Pos.X, pRV->Pos.Y, pRV->Pos.Z,
                           WeightColors[pRV->NMatrices - 1].R,
                           WeightColors[pRV->NMatrices - 1].G,
                           WeightColors[pRV->NMatrices - 1].B,
                           255,
                           Mesh );

                pRV = &m_pVert[pTri->VertID[1]];
                ASSERT( pRV->NMatrices <= 3 );
                x_fprintf( fp, "%f, %f, %f, 0, 0, 0, 0, 0, %1d, %1d, %1d, %1d, 0, %1c%1s%1c,\n",
                           pRV->Pos.X, pRV->Pos.Y, pRV->Pos.Z,
                           WeightColors[pRV->NMatrices - 1].R,
                           WeightColors[pRV->NMatrices - 1].G,
                           WeightColors[pRV->NMatrices - 1].B,
                           255,
                           '"', "MAT0", '"' );

                pRV = &m_pVert[pTri->VertID[2]];
                ASSERT( pRV->NMatrices <= 3 );
                x_fprintf( fp, "%f, %f, %f, 0, 0, 0, 0, 0, %1d, %1d, %1d, %1d, 0 ]\n",
                           pRV->Pos.X, pRV->Pos.Y, pRV->Pos.Z,
                           WeightColors[pRV->NMatrices - 1].R,
                           WeightColors[pRV->NMatrices - 1].G,
                           WeightColors[pRV->NMatrices - 1].B,
                           255 );

                pRV = &m_pVert[pTri->VertID[2]];
                ASSERT( pRV->NMatrices <= 3 );
                x_fprintf( fp, "[%f, %f, %f, 0, 0, 0, 0, 0, %1d, %1d, %1d, %1d, 0, %1s,\n",
                           pRV->Pos.X, pRV->Pos.Y, pRV->Pos.Z,
                           WeightColors[pRV->NMatrices - 1].R,
                           WeightColors[pRV->NMatrices - 1].G,
                           WeightColors[pRV->NMatrices - 1].B,
                           255,
                           Mesh );

                pRV = &m_pVert[pTri->VertID[1]];
                ASSERT( pRV->NMatrices <= 3 );
                x_fprintf( fp, "%f, %f, %f, 0, 0, 0, 0, 0, %1d, %1d, %1d, %1d, 0, %1c%1s%1c,\n",
                           pRV->Pos.X, pRV->Pos.Y, pRV->Pos.Z,
                           WeightColors[pRV->NMatrices - 1].R,
                           WeightColors[pRV->NMatrices - 1].G,
                           WeightColors[pRV->NMatrices - 1].B,
                           255,
                           '"', "MAT0", '"' );

                pRV = &m_pVert[pTri->VertID[0]];
                ASSERT( pRV->NMatrices <= 3 );
                x_fprintf( fp, "%f, %f, %f, 0, 0, 0, 0, 0, %1d, %1d, %1d, %1d, 0 ]\n",
                           pRV->Pos.X, pRV->Pos.Y, pRV->Pos.Z,
                           WeightColors[pRV->NMatrices - 1].R,
                           WeightColors[pRV->NMatrices - 1].G,
                           WeightColors[pRV->NMatrices - 1].B,
                           255 );
            }
        }

        x_fprintf( fp, "(   64,   64,    0.50,     0.50,       0,       0,   0.50,     0.50,      0   ,  %1c%1s%1c,  %1c%1s%1c)\n",
                   '"', "MAT0", '"', '"', "DUMMY.GIF", '"' );
        x_fclose( fp );
    }

    //---   Write out stripped versions of meshes
    if( g_StripFileName[0] != '\0' )
    {
        X_FILE*     fp;
        char        Mesh[64];

        //---   let the user know whats going on
        x_printf( "Exporting strip ASC file...\n" );

        //---   get the file handle
        fp = x_fopen( g_StripFileName, "wt" );
        if ( !fp )
        {
            x_printf( "ERROR: Could not open %s for writing.\n", g_StripFileName );
            PauseOnError();
            exit( 0 );
        }

        for ( i = 0; i < s_NFrames; i++ )
        {
            x_sprintf( Mesh, "%1c%1s%1c", '"', fs("MESH%03d", i + 100), '"');

            for ( j = 0; j < s_pFrame[i].NPacks; j++ )
            {
                color C;

                //---   Build color
                C.R = C.G = C.B = 32;
                if( x_irand( 0, 100 ) > 25 ) C.R = x_irand( 64, 200 );
                if( x_irand( 0, 100 ) > 25 ) C.G = x_irand( 64, 200 );
                if( x_irand( 0, 100 ) > 25 ) C.B = x_irand( 64, 200 );
                C.A = 255;

                //---   Loop through tris contained in pack
                for( k = 0; k < s_pFrame[i].pPack[j].NTris; k++ )
                {
                    t_RipVert* pRV;

                    pRV = &m_pVert[m_pTri[s_pFrame[i].pPack[j].TriID[k]].VertID[0]];
                    x_fprintf( fp, "[%f, %f, %f, 0, 0, 0, 0, 0, %1d, %1d, %1d, %1d, 0, %1s,\n",
                               pRV->Pos.X, pRV->Pos.Y, pRV->Pos.Z,
                               C.R, C.G, C.B, 255,
                               Mesh );

                    pRV = &m_pVert[m_pTri[s_pFrame[i].pPack[j].TriID[k] ].VertID[1]];
                    x_fprintf( fp, "%f, %f, %f, 0, 0, 0, 0, 0, %1d, %1d, %1d, %1d, 0, %1c%1s%1c,\n",
                               pRV->Pos.X, pRV->Pos.Y, pRV->Pos.Z,
                               C.R, C.G, C.B, 255,
                               '"', "MAT0", '"' );

                    pRV = &m_pVert[m_pTri[s_pFrame[i].pPack[j].TriID[k]].VertID[2]];
                    x_fprintf( fp, "%f, %f, %f, 0, 0, 0, 0, 0, %1d, %1d, %1d, %1d, 0 ]\n",
                               pRV->Pos.X, pRV->Pos.Y, pRV->Pos.Z,
                               C.R, C.G, C.B, 255 );

                    pRV = &m_pVert[m_pTri[s_pFrame[i].pPack[j].TriID[k]].VertID[2]];
                    x_fprintf( fp, "[%f, %f, %f, 0, 0, 0, 0, 0, %1d, %1d, %1d, %1d, 0, %1s,\n",
                               pRV->Pos.X, pRV->Pos.Y, pRV->Pos.Z,
                               C.R, C.G, C.B, 255,
                               Mesh );

                    pRV = &m_pVert[m_pTri[s_pFrame[i].pPack[j].TriID[k]].VertID[1]];
                    x_fprintf( fp, "%f, %f, %f, 0, 0, 0, 0, 0, %1d, %1d, %1d, %1d, 0, %1c%1s%1c,\n",
                               pRV->Pos.X, pRV->Pos.Y, pRV->Pos.Z,
                               C.R, C.G, C.B, 255,
                               '"', "MAT0", '"');

                    pRV = &m_pVert[m_pTri[s_pFrame[i].pPack[j].TriID[k]].VertID[0]];
                    x_fprintf( fp, "%f, %f, %f, 0, 0, 0, 0, 0, %1d, %1d, %1d, %1d, 0 ]\n",
                               pRV->Pos.X, pRV->Pos.Y, pRV->Pos.Z,
                               C.R, C.G, C.B, 255 );
                }
            }
        }

        x_fprintf( fp, "(   64,   64,    0.50,     0.50,       0,       0,   0.50,     0.50,      0   ,  %1c%1s%1c,  %1c%1s%1c)\n",
                   '"', "MAT0", '"', '"', "DUMMY.GIF", '"' );
        x_fclose( fp );
    }

    //---   Display texture boundries
    if( g_TextureSeamFileName[0] != '\0' )
    {
        X_FILE*     fp;
        char        Mesh[64];
        color       CR[10];
        s32         CI = 64;

        x_printf( "Exporting texture seam ASC file...\n" );

        CR[0] = color(   CI,      0,      0,      255 );
        CR[1] = color(    0,     CI,      0,      255 );
        CR[2] = color(    0,      0,     CI,      255 );
        CR[3] = color(   CI,     CI,      0,      255 );
        CR[4] = color(   CI,      0,     CI,      255 );
        CR[5] = color(    0,     CI,     CI,      255 );
        CR[6] = color(   CI,     CI,     CI,      255 );
        CR[7] = color( CI*2,      0,     CI,      255 );
        CR[8] = color(    0,   CI*2,     CI,      255 );
        CR[9] = color(    0,     CI,   CI*2,      255 );

        fp = x_fopen( g_TextureSeamFileName, "wt" );
        if ( !fp )
        {
            x_printf( "ERROR: Could not open %s for writing.\n", g_TextureSeamFileName );
            PauseOnError();
            exit( 0 );
        }

        x_sprintf( Mesh, "%1c%1s%1c", '"', fs( "MESH%03d", 0 ), '"' ); 

        for( i = 0; i < m_NTris; i++ )
        {
            t_RipVert*  pRV;
            color       C;

            C = CR[m_pTri[i].TextureID % 10];

            pRV = &m_pVert[m_pTri[i].VertID[0]];
            x_fprintf( fp, "[%f, %f, %f, 0, 0, 0, 0, 0, %1d, %1d, %1d, %1d, 0, %1s,\n",
                       pRV->Pos.X, pRV->Pos.Y, pRV->Pos.Z,
                       C.R,C.G,C.B,C.A,
                       Mesh );

            pRV = &m_pVert[m_pTri[i].VertID[1]];
            x_fprintf( fp, "%f, %f, %f, 0, 0, 0, 0, 0, %1d, %1d, %1d, %1d, 0, %1c%1s%1c,\n",
                       pRV->Pos.X, pRV->Pos.Y, pRV->Pos.Z,
                       C.R,C.G,C.B,C.A,
                       '"', "MAT0", '"' );

            pRV = &m_pVert[m_pTri[i].VertID[2]];
            x_fprintf( fp, "%f, %f, %f, 0, 0, 0, 0, 0, %1d, %1d, %1d, %1d, 0 ]\n",
                       pRV->Pos.X, pRV->Pos.Y, pRV->Pos.Z,
                       C.R,C.G,C.B,C.A  );
        }

        x_fprintf( fp, "(   64,   64,    0.50,     0.50,       0,       0,   0.50,     0.50,      0   ,  %1c%1s%1c,  %1c%1s%1c)\n",
                   '"', "MAT0", '"', '"', "DUMMY.GIF", '"');
        x_fclose( fp );
    }

    //---   Free tris
    x_free(s_pTri);
}

//==========================================================================

void QPS2RipSkin::DisplayStats( void )
{
    s32 i, j, k;
    s32 TotalTris;

    //---   Display meshes
    x_printf( "\n\nMESHES:\n" );
    for( i = 0; i < m_NMeshes; i++ )
    {
        x_printf( "%3d] %4d:SM  %4d:NSMs  %1s\n",
                  i,
                  m_pMesh[i].SubmeshID,
                  m_pMesh[i].NSubmeshes,
                  m_pMesh[i].Name );
    }

    //---   Display submeshes
    x_printf("\n\nSUBMESHES:\n");
    TotalTris = 0;
    for( i = 0; i < m_NSubmeshes; i++ )
    {
        TotalTris += m_pSubMesh[i].NTris;

        if ( m_pSubMesh[i].TextureID >= 0 )
        {
            x_printf( "%3d]  %5d:Tri   %4d:NTris   %2d:Mesh  %2d:Texture (%3d,%3d) %1s\n",
                      i,
                      m_pSubMesh[i].TriID,
                      m_pSubMesh[i].NTris,
                      m_pSubMesh[i].MeshID,
                      m_pSubMesh[i].TextureID,
                      m_pTexture[ m_pSubMesh[i].TextureID ].Width,
                      m_pTexture[ m_pSubMesh[i].TextureID ].Height,
                      m_pTexture[ m_pSubMesh[i].TextureID ].Name );
        }
        else
        {
            x_printf( "%3d]  %5d:Tri   %4d:NTris   %2d:Mesh  %2d:Texture (%3d,%3d) %1s\n",
                      i,
                      m_pSubMesh[i].TriID,
                      m_pSubMesh[i].NTris,
                      m_pSubMesh[i].MeshID,
                      m_pSubMesh[i].TextureID,
                      0,
                      0,
                      "" );
        }
    }

    //---   Display transforms
    x_printf( "\n\nTRANSFORMS\n" );
    {
        s32 NVerts[8] = {0};
        s32 NTransforms[8] = {0};

        for( i = 0; i < m_NTransforms; i++ )
        {
            x_printf( "%3d]  %4d:NV  %4d:NM   |",
                      i,
                      m_pTransform[i].NVerts,
                      m_pTransform[i].NMatrices );

            NVerts[m_pTransform[i].NMatrices] += m_pTransform[i].NVerts;
            NTransforms[m_pTransform[i].NMatrices]++;

            for( j = 0; j < m_pTransform[i].NMatrices; j++ )
                x_printf( " %8d ",
                          m_pTransform[i].MatrixID[j] );

            x_printf("\n");
        }

        x_printf("\nNUM OF VERTS AND TRANSFORMS BASED ON n MATRICES\n");
        for( i = 0; i < 8; i++ )
        {
            x_printf( "%3d] %4d:NV  %4d:NT\n",
                      i,
                      NVerts[i],
                      NTransforms[i] );
        }

        for( k = 0; k < 8; k++ )
        {
            s32 NVerts, NTransforms;

            x_printf( "\n\n(%1d) MATRIX TRANSFORMS\n", k );
            NVerts = NTransforms = 0;

            for( i = 0; i < m_NTransforms; i++ )
            {
                if( m_pTransform[i].NMatrices == k )
                {
                    NVerts += m_pTransform[i].NVerts;
                    NTransforms++;

                    x_printf( "%3d]  %4d:NV  %4d:NM   |",
                              i,
                              m_pTransform[i].NVerts,
                              m_pTransform[i].NMatrices );
                    for( j = 0; j < m_pTransform[i].NMatrices; j++ )
                        x_printf( " %3d", m_pTransform[i].MatrixID[j] );
                    x_printf( "\n" );
                }
            }
            x_printf( "NVERTS:%1d  NTRANSFORMS:%1d\n", NVerts, NTransforms );
        }
    }

    //---   Display matrices
    x_printf( "\n\nMATRICES\n" );
    {
        s32 NVerts[9][128] = {0};
        s32 NMatrices = 0;

        for( i = 0; i < m_NTransforms; i++ )
        {
            for( k = 0; k < m_pTransform[i].NMatrices; k++ )
            {
                NMatrices = MAX( NMatrices, m_pTransform[i].MatrixID[k] );
                NVerts[m_pTransform[i].NMatrices][m_pTransform[i].MatrixID[k]] += m_pTransform[i].NVerts;
                NVerts[8][m_pTransform[i].MatrixID[k]] += m_pTransform[i].NVerts;
            }
        }

        for( i = 0; i < NMatrices; i++ )
        {
            x_printf( "%3d]  (%4d)", i, NVerts[8][i] );
            for( j=0; j<8; j++ )
            {
                if( NVerts[j][i] )
                    x_printf( "%4d ", NVerts[j][i] );
                else
                    x_printf("     ");
            }
            x_printf("\n");
        }
    }

    //---   Display stats
    x_printf( "\n\nSTATS:\n" );
    x_printf( "%5d Verts\n", m_NVerts );
    x_printf( "%5d Tris\n", m_NTris );
    x_printf( "%5d Textures\n", m_NTextures );
    x_printf( "%5d Submeshes\n", m_NSubmeshes );
    x_printf( "%5d Transforms\n", m_NTransforms );
    x_printf( "\n" );
}

//==========================================================================

static
void ResizeTransformsToInputBuffer( t_CacheFrame* pFrame, xbool EnvPass, xbool Morph )
{
    s32             NTransforms;
    s32             i, j;
    t_CacheFrame    SrcFrame;

    //---   Save a copy of the cache frame
    x_memcpy( &SrcFrame, pFrame, sizeof(t_CacheFrame) );

    //---   clear out the dest transforms
    for ( i = 0; i < MAX_TRANSFORMS; i++ )
        pFrame->Transform[i].NVerts = 0;

    //---   now re-create the transforms, so that no transform has more
    //      verts than the input buffer can handle
    NTransforms = 0;
    for ( i = 0; i < MAX_TRANSFORMS; i++ )
    {
        if ( SrcFrame.Transform[i].NVerts > 0 )
        {
            s32 NVertsAdded;

            ASSERT( NTransforms < MAX_TRANSFORMS );

            //---   begin copying the verts over, but do it a little at a
            //      time, so that we don't overfill the input buffer
            NVertsAdded = 0;
            while ( NVertsAdded < SrcFrame.Transform[i].NVerts )
            {
                s32 NVertsToAdd;
                s32 NVertsMax;

                //---   how many verts can we safely add?
                if ( EnvPass )
                    NVertsMax = INPUT_VERTS_ENVMAP;
                else
                if ( Morph )
                    NVertsMax = INPUT_VERTS_MORPH;
                else
                    NVertsMax = INPUT_VERTS_NORMAL;
                NVertsToAdd = SrcFrame.Transform[i].NVerts - NVertsAdded;
                if ( NVertsToAdd > NVertsMax )
                    NVertsToAdd = NVertsMax;

                //---   copy the src transform into the dest
                x_memcpy( &pFrame->Transform[NTransforms],
                          &SrcFrame.Transform[i], sizeof(t_CacheVertList) );

                //---   now, the only thing that has changed between
                //      the source and dest is the # of verts
                pFrame->Transform[NTransforms].NVerts = NVertsToAdd;

                //---   add the verts
                ASSERT( NVertsToAdd );
                for ( j = 0; j < NVertsToAdd; j++ )
                {
                    pFrame->Transform[NTransforms].CacheID[j] =
                        SrcFrame.Transform[i].CacheID[NVertsAdded + j];
                    pFrame->Transform[NTransforms].VertID[j] =
                        SrcFrame.Transform[i].VertID[NVertsAdded + j];
                }

                //---   keep our totals current
                NVertsAdded += NVertsToAdd;
                NTransforms++;
            }
        }
    }
}

//==========================================================================

static
void GetTextureName( char* TextureName, char* FullPathName )
{
    char    Drive[256];
    char    Path[256];
    char    File[256];
    char    Ext[256];
    
    x_splitpath( FullPathName,
                 Drive,
                 Path,
                 File,
                 Ext );
    FillName( TextureName, File, 32 );
}

//==========================================================================

static
s32 ExportTransformHeader( byte* pDList,
                           t_CacheFrame* pFrame,
                           s32 TransformID,
                           xbool EnvPass,
                           xbool Shadow )
{
    s32* pHDR;

    pHDR = (s32*)pDList;
    
    //---   the header takes this form:
    //      HDR[0] = transform type
    //      HDR[1] = NVerts
    //      HDR[2] = unused (TransformID)
    //      HDR[3] = Matrix A
    //      HDR[4] = Matrix B
    //      HDR[5] = Matrix C
    x_memset( pHDR, 0, sizeof(s32) * 6 );

    //---   fill in the header info that is common to all types
    pHDR[1] = pFrame->Transform[TransformID].NVerts;
    pHDR[2] = TransformID;

    //---   fill in the header info which is dependent on the transform type
    if ( EnvPass )
    {
        pHDR[0] = SKIN_TRANSFORM_TYPE_ENVI;
        pHDR[3] = 7 * pFrame->Transform[TransformID].MatrixID[0];
    }
    else
    if ( Shadow )
    {
        pHDR[0] = SKIN_TRANSFORM_TYPE_SHADOW;
        pHDR[3] = 7 * pFrame->Transform[TransformID].MatrixID[0];
    }
    else
    {
        switch( pFrame->Transform[TransformID].NMatrices )
        {
        case 1:
            pHDR[0] = SKIN_TRANSFORM_TYPE_SINGLE;
            pHDR[3] = 7 * pFrame->Transform[TransformID].MatrixID[0];
            break;
        case 2:
            pHDR[0] = SKIN_TRANSFORM_TYPE_DOUBLE;
            pHDR[3] = 7 * pFrame->Transform[TransformID].MatrixID[0];
            pHDR[4] = 7 * pFrame->Transform[TransformID].MatrixID[1];
            break;
        case 3:
            pHDR[0] = SKIN_TRANSFORM_TYPE_TRIPLE;
            pHDR[3] = 7 * pFrame->Transform[TransformID].MatrixID[0];
            pHDR[4] = 7 * pFrame->Transform[TransformID].MatrixID[1];
            pHDR[5] = 7 * pFrame->Transform[TransformID].MatrixID[2];
            break;
        default:
            ASSERT( FALSE );
            break;
        }
    }

    //---   return the size of the hdr
    return (6 * sizeof(s32));
}

//==========================================================================

s32 QPS2RipSkin::ExportFrameTransforms( byte* pDList,
                                        s32 FrameID,
                                        xbool EnvPass,
                                        xbool Shadow,
                                        xbool Morph,
                                        f32 MinY,
                                        f32 MaxY )
{
    s32             i;
    t_CacheFrame*   pFrame;
    s32             NTransformsToAdd = 0;
    s32             NTransformsAdded = 0;
    s32             TransformID;
    s32             TotalSize = 0;
    s32             VertSize;
    s32             ExportSize;
    s32             MorphDataStart;
    s32             MorphDataEnd;
    
    //---   don't allow morphing of an EnvPass or Shadow
    if ( EnvPass || Shadow )
        Morph = FALSE;

    //---   the # of vectors per vert depends on the transform type
    if ( Shadow )
        VertSize = 1;
    else
    if ( EnvPass )
        VertSize = 2;
    else
        VertSize = 3;
    
    //---   get a ptr to the frame
    pFrame = &s_pFrame[FrameID];

    //---   Re-size the transforms, so that they will all
    //      fit in the input buffer
    ResizeTransformsToInputBuffer( pFrame, EnvPass, Morph );

    //---   count the # of transforms to add
    for ( i = 0; i < MAX_TRANSFORMS; i++ )
    {
        if ( pFrame->Transform[i].NVerts > 0 )
            NTransformsToAdd++;
    }

    //---   Loop through and build the transforms
    NTransformsAdded = 0;
    TransformID = 0;
    while ( NTransformsAdded < NTransformsToAdd )
    {
        s32     NVectors;
        u32     NTransformsInCommand;
        byte*   pVectorStart;
        u32*    pVIF;
        u32*    pCMD;

        ASSERT( TransformID < MAX_TRANSFORMS );
        
        //---   don't add the transform if there is no verts in it
        if ( pFrame->Transform[TransformID].NVerts == 0 )
        {
            TransformID++;
            continue;
        }

        //---   Start a new input buffer
        NTransformsInCommand = 0;
        MorphDataStart = MorphDataEnd = s_NMorphDatas;

        //---   each time we fill the input buffer, it is considered a vert load
        //---   allocate space for a new vert load
        if ( s_NExpVertLoads >= s_NExpVertLoadsAllocated )
        {
            s_NExpVertLoadsAllocated += 16;
            s_pExpVertLoads = (t_SkinVertLoad*)x_realloc(
                s_pExpVertLoads,
                s_NExpVertLoadsAllocated * sizeof(t_SkinVertLoad) );
            ASSERT( s_pExpVertLoads );
        }

        //---   fill in as much of the vert load data as we can
        s_pExpVertLoads[s_NExpVertLoads].VertPacketOffset = (u32)pDList - (u32)s_pDataStart;
        s_pExpVertLoads[s_NExpVertLoads].VertPacketSize = 0;    // we will have to fill this in later
        s_pExpVertLoads[s_NExpVertLoads].KickPacketOffset = 0;  // we will have to fill this in later
        s_pExpVertLoads[s_NExpVertLoads].KickPacketSize = 0;    // we will have to fill this in later
        s_pExpVertLoads[s_NExpVertLoads].DeltaPacketOffset =
            (u32)&s_pExpDeltaData[s_ExpDeltaDataSize] - (u32)s_pCurrMeshDeltaPacket;
        s_pExpVertLoads[s_NExpVertLoads].DeltaPacketSize = 0;   // we will have to fill this in later

        //---   Reserve space for the VIFCode
        pVIF = (u32*)pDList;
        pVIF[0] = 0;        // to be filled in later
        NVectors = 0;
        pDList += sizeof(u32);
        TotalSize += sizeof(u32);

        //---   Save the DList ptr for sanity purposes
        pVectorStart = pDList;

        //---   Set up the transform CMD
        pCMD = (u32*)pDList;
        pCMD[0] = SKIN_OPCODE_TRANSFORM;        // opcode
        pCMD[1] = 0;    // offset to next command
        pCMD[2] = 0; // NTransformsInCommand (to be filled in later in this func.)
        NVectors++;
        pDList += sizeof(u32) * 3;
        TotalSize += sizeof(u32) * 3;

        //---   Confirm transform will fit
        if ( Morph )
        {
            //---   part of the input buffer is taken up by the morph deltas
            ASSERT( (2 + (pFrame->Transform[TransformID].NVerts * VertSize)) <=
                    (SKIN_INPUT_BUFFER_SIZE - NVectors - SKIN_INPUT_BUFFER_MORPH_SIZE) );
        }
        else
        {
            //---   we can use the entire input buffer
            ASSERT( (2 + (pFrame->Transform[TransformID].NVerts * VertSize)) <=
                    (SKIN_INPUT_BUFFER_SIZE - NVectors) );
        }

        //---   Now this will get a little confusing, but if you relax and
        //      read the comments carefully, we should be able to get through
        //      this. Let's begin. We're going to add as many transforms as
        //      we can into one input buffer. We just keep looping through
        //      transforms until either a) the input buffer is full, or
        //      b) there are no more transforms to add.
        //      If 'a' occurs, we'll finish off this transform CMD, and
        //      let the outer loop start up a fresh one, and we'll
        //      pick up where we left off.
        //      If 'b' occurs we are done.
        while ( NTransformsAdded < NTransformsToAdd )
        {
            ASSERT( TransformID < MAX_TRANSFORMS );

            //---   don't add this transform if it has no verts
            if ( pFrame->Transform[TransformID].NVerts == 0 )
            {
                TransformID++;
                continue;
            }
            
            //---   if this transform won't fit, break out of the inner
            //      loop so this packet can be finished, and then a
            //      new packet started
            if ( Morph )
            {
                //---   part of the input buffer is taken up by the morph deltas
                if ( (2 + (pFrame->Transform[TransformID].NVerts * VertSize)) >
                     (SKIN_INPUT_BUFFER_SIZE - NVectors - SKIN_INPUT_BUFFER_MORPH_SIZE) )
                {
                    break;
                }
            }
            else
            {
                //---   we can use the entire input buffer
                if ( (2 + (pFrame->Transform[TransformID].NVerts * VertSize)) >
                     (SKIN_INPUT_BUFFER_SIZE - NVectors) )
                {
                    break;
                }
            }

            //---   export the transform header info
            ExportSize = ExportTransformHeader( pDList, pFrame, TransformID, EnvPass, Shadow );
            pDList += ExportSize;
            TotalSize += ExportSize;
            NVectors += ExportSize / (3 * 4);

            //---   Fill out verts
            for ( i = 0; i < pFrame->Transform[TransformID].NVerts; i++ )
            {
                f32*    pVERT;
                s32     VertID;

                //---   get the vert ID
                VertID = pFrame->Transform[TransformID].VertID[i];

                //---   does this vert morph?
                if ( m_pVert[VertID].HasMorphDelta )
                {
                    if ( s_NMorphDatas >= s_NMorphDatasAllocated )
                    {
                        s_NMorphDatasAllocated += 16;
                        s_pMorphData = (t_MorphData*)x_realloc(
                            s_pMorphData, s_NMorphDatasAllocated * sizeof(t_MorphData) );
                        ASSERT( s_pMorphData );
                    }

                    s_pMorphData[s_NMorphDatas].ByteOffsetInVUMem = SKIN_INPUT_BUFFER_MORPH_SIZE + NVectors;
                    s_pMorphData[s_NMorphDatas].VertID = VertID;
                    s_NMorphDatas++;
                    MorphDataEnd++;
                }

                //---   store vert data
                pVERT = (f32*)pDList;
                pVERT[0] = m_pVert[VertID].Pos.X;
                pVERT[1] = m_pVert[VertID].Pos.Y;
                pVERT[2] = m_pVert[VertID].Pos.Z;
                if ( Shadow )
                {
                    // Nothing to add...
                }
                else
                {
                    pVERT[3] = m_pVert[VertID].Normal.X;
                    pVERT[4] = m_pVert[VertID].Normal.Y;
                    pVERT[5] = m_pVert[VertID].Normal.Z;
                }

                //---   the environment pass and shadow don't require UV coords
                if ( !EnvPass && !Shadow )
                {
                    pVERT[6] = m_pVert[VertID].UV.X;
                    pVERT[7] = m_pVert[VertID].UV.Y;
                    pVERT[8] = 1.0f;
                }

                if ( EnvPass || Shadow )
                {
                    //---   EnvPasses and Shadows hide destination in X-component of position
                    ((byte*)&pVERT[0])[0] = (byte)pFrame->Transform[TransformID].CacheID[i];
                }
                else
                {
                    //---   everything else hides its cache index in X-component of UV AND
                    //      in the X-component of the position. The reason for the x-component
                    //      of the position is that the X-Components need to match EXACTLY
                    //      for environment mapping, so that there isn't z-buffer fighting.
                    ((byte*)&pVERT[0])[0] = (byte)pFrame->Transform[TransformID].CacheID[i];
                    ((byte*)&pVERT[6])[0] = (byte)pFrame->Transform[TransformID].CacheID[i];
                }

                if ( EnvPass || Shadow || pFrame->Transform[TransformID].NMatrices == 1 )
                {
                    //---   no need to add weights if there's only one matrix
                    ;
                }
                else
                if ( pFrame->Transform[TransformID].NMatrices == 2 )
                {
                    s32 weight;

                    //---   hide the weights in the Normal's X and Y
                    weight = (s32)(m_pVert[VertID].MatrixWeight[0] * 256.0f);
                    ASSERT( weight < 256 );
                    ((u8*)&pVERT[3])[0] = (u8)weight;

                    weight = (s32)(m_pVert[VertID].MatrixWeight[1] * 256.0f);
                    ASSERT( weight < 256 );
                    ((u8*)&pVERT[4])[0] = (u8)weight;
                }
                else
                {
                    s32 weight;
                    
                    //---   hide the weights in the Normal's X, Y, and Z
                    weight = (s32)(m_pVert[VertID].MatrixWeight[0] * 256.0f);
                    ASSERT( weight < 256 );
                    ((u8*)&pVERT[3])[0] = (u8)weight;

                    weight = (s32)(m_pVert[VertID].MatrixWeight[1] * 256.0f);
                    ASSERT( weight < 256 );
                    ((u8*)&pVERT[4])[0] = (u8)weight;

                    weight = (s32)(m_pVert[VertID].MatrixWeight[2] * 256.0f);
                    ASSERT( weight < 256 );
                    ((u8*)&pVERT[5])[0] = (u8)weight;
                }

                //---   increment our running counters
                if ( Shadow )
                {
                    pDList += sizeof(f32) * 3;
                    TotalSize += sizeof(f32) * 3;
                    NVectors += 1;
                }
                else
                if ( EnvPass )
                {
                    pDList += sizeof(f32) * 6;
                    TotalSize += sizeof(f32) * 6;
                    NVectors += 2;
                }
                else
                {
                    pDList += sizeof(f32) * 9;
                    TotalSize += sizeof(f32) * 9;
                    NVectors += 3;
                }
            }

            //---   update counters
            NTransformsInCommand++;
            NTransformsAdded++;
            TransformID++;
        }

        //---   Now we can fill in the missing part of the transform CMD
        pCMD[2] = NTransformsInCommand;

        //---   Now we can fill in the VIF code
        if ( Morph )
        {
            //---   the morph data will go at the beginning of the input buffer,
            //      so we need to offset the transform data.
            PS2HELP_BuildVIFTagUnpack( pVIF,
                                       SKIN_INPUT_BUFFER_MORPH_SIZE,
                                       NVectors,
                                       VIFHELP_UNPACK_V3_32,
                                       FALSE, TRUE, FALSE );
        }
        else
        {
            //---   we can let the transform data take up the full amount of the
            //      input buffer
            PS2HELP_BuildVIFTagUnpack( pVIF,
                                       0, 
                                       NVectors,
                                       VIFHELP_UNPACK_V3_32,
                                       FALSE, TRUE, FALSE );
        }

        //---   do a sanity check
        ASSERT( ((u32)pDList - (u32)pVectorStart) == (u32)NVectors * 3 * 4 );

        //---   Align data
        while ( (TotalSize & 0xf) != 0x00 )
        {
            *((u32*)pDList) = 0;
            pDList += sizeof(u32);
            TotalSize += sizeof(u32);
        }

        //---   fill in some more of the vert load info
        s_pExpVertLoads[s_NExpVertLoads].VertPacketSize =
            ((u32)pDList - (u32)s_pDataStart) - s_pExpVertLoads[s_NExpVertLoads].VertPacketOffset;

        if ( Morph )
        {
            byte*   pMorphDList = &s_pExpDeltaData[s_ExpDeltaDataSize];
            s16*    pVERT;
            s16*    pMorphCMD;

            //---   Save out the VIF code for the morph command
            pVIF = (u32*)pMorphDList;
            pVIF[0] = 0;    // to be filled in later
            NVectors = 0;
            pMorphDList += sizeof(u32);
            s_ExpDeltaDataSize += sizeof(u32);

            //---   export the morph CMD
            pMorphCMD = (s16*)pMorphDList;
            pMorphCMD[0] = SKIN_OPCODE_TRANSFORM_MORPH;  // opcode
            pMorphCMD[1] = SKIN_INPUT_BUFFER_MORPH_SIZE; // offset to next CMD (the transform command)
            pMorphCMD[2] = MorphDataEnd - MorphDataStart;
            //pMorphCMD[2] = 0;    // ####
            pMorphCMD[3] = 0;
            pMorphDList += sizeof(s16) * 4;
            s_ExpDeltaDataSize += sizeof(s16) * 4;
            NVectors++;
            
            for ( i = MorphDataStart; i < MorphDataEnd; i++ )
            {
                //---   store the delta location
                if ( s_NExpDeltaLocs >= s_NExpDeltaLocsAllocated )
                {
                    s_NExpDeltaLocsAllocated += 16;
                    s_pExpDeltaLocs = (t_SkinDeltaLoc*)x_realloc(
                        s_pExpDeltaLocs, s_NExpDeltaLocsAllocated * sizeof(t_SkinDeltaLoc) );
                }
                s_pExpDeltaLocs[s_NExpDeltaLocs++] = (u32)pMorphDList - (u32)s_pCurrMeshDeltaPacket;

                //---   export the default delta...the w component contains
                //      where the matching vert is in the input buffer
                pVERT = (s16*)pMorphDList;
                pVERT[0] = 0;
                pVERT[1] = 0;
                pVERT[2] = 0;
                pVERT[3] = s_pMorphData[i].ByteOffsetInVUMem;
                pMorphDList += sizeof(s16) * 4;
                s_ExpDeltaDataSize += sizeof(s16) * 4;
                NVectors++;
            }
            
            //---   we can fill in the VIF code now...
            PS2HELP_BuildVIFTagUnpack( pVIF, 0, NVectors, VIFHELP_UNPACK_V4_16, TRUE, FALSE, FALSE );

            //---   Align data
            while ( (s_ExpDeltaDataSize & 0xf) != 0x00 )
            {
                *((u32*)pMorphDList) = 0;
                pMorphDList += sizeof(u32);
                s_ExpDeltaDataSize += sizeof(u32);
            }
        }

        //---   now we can fill in the delta packet size
        s_pExpVertLoads[s_NExpVertLoads].DeltaPacketSize =
            (u32)&s_pExpDeltaData[s_ExpDeltaDataSize] - (u32)s_pCurrMeshDeltaPacket -
            s_pExpVertLoads[s_NExpVertLoads].DeltaPacketOffset;

        //---   fill in some more of the vert load info
        s_pExpVertLoads[s_NExpVertLoads].KickPacketOffset = (u32)pDList - (u32)s_pDataStart;

        //---   Input buffer is full, add kick
        pVIF = (u32*)pDList;
        pVIF[0] = VIFHELP_SET_MSCNT(0);
        pDList += sizeof(u32);
        TotalSize += sizeof(u32);

        //---   Align data
        while ( (TotalSize & 0xf) != 0x00 )
        {
            *((u32*)pDList) = 0;
            pDList += sizeof(u32);
            TotalSize += sizeof(u32);
        }

        //---   fill in some more of the vert load info
        s_pExpVertLoads[s_NExpVertLoads].KickPacketSize =
            ((u32)pDList - (u32)s_pDataStart) - s_pExpVertLoads[s_NExpVertLoads].KickPacketOffset;

        //---   another successful vert load completed
        s_NExpVertLoads++;
    }

    return TotalSize;
}

//==========================================================================

s32 QPS2RipSkin::ExportFramePacks( byte* pDList,
                                s32 FrameID,
                                xbool EnvPass,
                                xbool Shadow,
                                xbool Alpha )
{
    u32             PrimFlags;
    u32*            pVIF;
    u32*            pCMD;
    t_CacheFrame*   pFrame;
    s32             TotalSize = 0;
    sceGifTag*      pGIF;
    s32             NVectors;
    s32             PackID;
    s32             i, j;

    //---   grab a ptr to the frame
    pFrame = &s_pFrame[FrameID];

    //---   Build the VIF unpack info for the command and GifTags
    pVIF = (u32*)pDList;
    pVIF[0] = 0;
    pVIF[1] = 0;
    pVIF[2] = 0;
    PS2HELP_BuildVIFTagUnpack( &pVIF[3], 0, 3, VIFHELP_UNPACK_V4_32, FALSE, FALSE, FALSE );
    pDList += sizeof(u32) * 4;
    TotalSize += sizeof(u32) * 4;

    //---   Build the CMD
    pCMD = (u32*)pDList;
    if ( Shadow )
        pCMD[0] = SKIN_OPCODE_FLUSH_SHADOW; // opcode
    else
        pCMD[0] = SKIN_OPCODE_FLUSH_TRIS;   // opcode
    pCMD[1] = 0;                            // offset
    pCMD[2] = pFrame->NPacks;               // # of packs
    pCMD[3] = 0;                            // pad
    pDList += sizeof(u32) * 4;
    TotalSize += sizeof(u32) * 4;

    //---   Add the GIF tags
    if ( Shadow )
        PrimFlags = VIFHELP_GIFPRIMFLAGS_SMOOTHSHADE | VIFHELP_GIFPRIMFLAGS_ALPHA;
    else
    if ( EnvPass )
        PrimFlags = VIFHELP_GIFPRIMFLAGS_SMOOTHSHADE | VIFHELP_GIFPRIMFLAGS_TEXTURE | VIFHELP_GIFPRIMFLAGS_ALPHA;
    else
        PrimFlags = VIFHELP_GIFPRIMFLAGS_SMOOTHSHADE | VIFHELP_GIFPRIMFLAGS_TEXTURE;
    if ( Alpha )
        PrimFlags |= VIFHELP_GIFPRIMFLAGS_ALPHA;

    pGIF = (sceGifTag*)pDList;
    if ( Shadow )
    {
        PS2HELP_BuildGIFTag1( pGIF,
                              VIFHELP_GIFMODE_PACKED,
                              2, 0, TRUE,
                              VIFHELP_GIFPRIMTYPE_TRIANGLESTRIP,
                              PrimFlags,
                              TRUE );
        PS2HELP_BuildGIFTag2( pGIF,
                              VIFHELP_GIFREG_RGBAQ,
                              VIFHELP_GIFREG_XYZ2,
                              VIFHELP_GIFREG_NOP,
                              VIFHELP_GIFREG_NOP );
    }
    else
    {
        PS2HELP_BuildGIFTag1( pGIF,
                              VIFHELP_GIFMODE_PACKED,
                              3, 0, TRUE,
                              VIFHELP_GIFPRIMTYPE_TRIANGLESTRIP,
                              PrimFlags,
                              TRUE );
        PS2HELP_BuildGIFTag2( pGIF,
                              VIFHELP_GIFREG_ST,
                              VIFHELP_GIFREG_RGBAQ,
                              VIFHELP_GIFREG_XYZ2,
                              VIFHELP_GIFREG_NOP );
    }
    pDList += sizeof(sceGifTag);
    TotalSize += sizeof(sceGifTag);

    pGIF = (sceGifTag*)pDList;
    if ( Shadow )
    {
        PS2HELP_BuildGIFTag1( pGIF,
                              VIFHELP_GIFMODE_PACKED,
                              2, 0, TRUE,
                              VIFHELP_GIFPRIMTYPE_TRIANGLE,
                              PrimFlags,
                              TRUE );
        PS2HELP_BuildGIFTag2( pGIF,
                              VIFHELP_GIFREG_RGBAQ,
                              VIFHELP_GIFREG_XYZ2,
                              VIFHELP_GIFREG_NOP,
                              VIFHELP_GIFREG_NOP );
    }
    else
    {
        PS2HELP_BuildGIFTag1( pGIF,
                              VIFHELP_GIFMODE_PACKED,
                              3, 0, TRUE,
                              VIFHELP_GIFPRIMTYPE_TRIANGLE,
                              PrimFlags,
                              TRUE );
        PS2HELP_BuildGIFTag2( pGIF,
                              VIFHELP_GIFREG_ST,
                              VIFHELP_GIFREG_RGBAQ,
                              VIFHELP_GIFREG_XYZ2,
                              VIFHELP_GIFREG_NOP );
    }
    pDList += sizeof(sceGifTag);
    TotalSize += sizeof(sceGifTag);

    //---   Sanity check to make sure we're still aligned
    ASSERT( (TotalSize & 0xf) == 0x00 );

    //---   Reserve the VIF unpack info for the verts
    pVIF = (u32*)pDList;
    pVIF[0] = 0;
    pVIF[1] = 0;
    pVIF[2] = 0;
    pVIF[3] = 0;    // this will be the unpack command, to be filled in later in this func.
    pDList += sizeof(u32) * 4;
    TotalSize += sizeof(u32) * 4;

    //---   loop through, adding the packs
    NVectors = 0;
    ASSERT( pFrame->NPacks );
    for ( PackID = 0; PackID < pFrame->NPacks; PackID++ )
    {
        s32     NVerts;
        u16*    pPACK;
        s32     NFours;
        
        //---   Get the number of Verts
        NVerts = pFrame->pPack[PackID].NVerts;
        ASSERT( NVerts );

        //---   the verts have to get added four at a time,
        //      figure out how many four-packs we need
        NFours = NVerts / 4;
        if ( NVerts % 4 )
            NFours++;

        //---   create the pack command
        pPACK = (u16*)pDList;
        pPACK[0] = NFours;      // # of four-packs
        pPACK[1] = NVerts;      // # verts
        pPACK[2] = 1;           // Is strip?
        pPACK[3] = 0;           // unused
        NVectors++;
        pDList += sizeof(u16) * 4;
        TotalSize += sizeof(u16) * 4;

        //---   now loop, setting up the four-packs
        for ( i = 0; i < NFours; i++ )
        {
            pPACK = (u16*)pDList;

            for ( j = 0; j < 4; j++ )
            {
                if ( (i * 4 + j) < NVerts )
                {
                    //---   add the cache IDs
                    pPACK[j] = pFrame->pPack[PackID].VertID[i * 4 + j];
                }
                else
                {
                    //---   we're through the verts, just add padding
                    pPACK[j] = 0;
                }
            }

            //---   keep our totals current
            NVectors++;
            pDList += sizeof(u16) * 4;
            TotalSize += sizeof(u16) * 4;
        }
    }

    //---   sanity check to make sure we didn't overflow the buffer
    ASSERT( (NVectors + 3) < SKIN_INPUT_BUFFER_SIZE );

    //---   now we know enough to fill in the VIF unpack info
    PS2HELP_BuildVIFTagUnpack( &pVIF[3],
                               3, 
                               NVectors,
                               VIFHELP_UNPACK_V4_16,
                               FALSE, FALSE, FALSE );

    //---   And kick off the flush tris command
    pVIF = (u32*)pDList;
    pVIF[0] = VIFHELP_SET_MSCNT(0);
    pDList += sizeof(u32);
    TotalSize += sizeof(u32);

    return TotalSize;
}

//==========================================================================

s32 QPS2RipSkin::ExportSubMesh( byte*          pDList,
                                s32            SubMeshID )
{
    s32             ExportSize;
    t_CacheFrame*   pFrame;
    byte*           pStart;
    s32             TotalSize = 0;
    s32             FrameID;
    f32             MinY = 0.0f, MaxY = 0.0f;

    //---   save out a ptr to the start of the dlist
    pStart = pDList;

    //---   figure out the MinY and MaxY if this is a shadow
    if ( IsShadow(SubMeshID) )
    {
        s32 TransformID, VertID;

        MinY = 50000.0f;
        MaxY = -50000.0f;
        for ( FrameID = m_pSubMesh[SubMeshID].CacheFrameID;
              FrameID < (m_pSubMesh[SubMeshID].NCacheFrames +
                         m_pSubMesh[SubMeshID].CacheFrameID);
              FrameID++ )
        {
            for ( TransformID = 0; TransformID < MAX_TRANSFORMS; TransformID++ )
            {
                for ( VertID = 0; VertID < s_pFrame[FrameID].Transform[TransformID].NVerts; VertID++ )
                {
                    t_RipVert*  pVert;
                    pVert = &m_pVert[s_pFrame[FrameID].Transform[TransformID].VertID[VertID]];
                    if ( pVert->Pos.Y < MinY )
                        MinY = pVert->Pos.Y;
                    if ( pVert->Pos.Y > MaxY )
                        MaxY = pVert->Pos.Y;
                }
            }
        }
    }

    //---   Loop through and build the frames
    for ( FrameID = m_pSubMesh[SubMeshID].CacheFrameID;
          FrameID < (m_pSubMesh[SubMeshID].NCacheFrames +
                     m_pSubMesh[SubMeshID].CacheFrameID);
          FrameID++ )
    {
        s32         NTransformsAdded = 0;
        
        ASSERT( s_pFrame );

        pFrame = &s_pFrame[FrameID];
        if ( !pFrame->NPacks )
            continue;

        //---   allocate room for a new cache frame
        if ( s_NExpCacheFrames >= s_NExpCacheFramesAllocated )
        {
            s_NExpCacheFramesAllocated += 16;
            s_pExpCacheFrames = (t_SkinCacheFrame*)x_realloc(
                s_pExpCacheFrames,
                s_NExpCacheFramesAllocated * sizeof(t_SkinCacheFrame) );
            ASSERT( s_pExpCacheFrames );
        }

        //---   fill in the cache frame data
        s_pExpCacheFrames[s_NExpCacheFrames].FirstVertLoad = s_NExpVertLoads;
        s_pExpCacheFrames[s_NExpCacheFrames].NVertLoads = 0;        // have to fill this in later
        s_pExpCacheFrames[s_NExpCacheFrames].FlushPacketOffset = 0; // have to fill this in later
        s_pExpCacheFrames[s_NExpCacheFrames].FlushPacketSize = 0;   // have to fill this in later

        //---   export the frame transforms
        ExportSize = ExportFrameTransforms( pDList, FrameID, IsEnvPass(SubMeshID), IsShadow(SubMeshID), IsMorph(SubMeshID), MinY, MaxY );
        pDList += ExportSize;
        TotalSize += ExportSize;

        //---   Pad out the VIF if necessary
        while ( (TotalSize & 0xf) != 0x00 )
        {
            *((u32*)pDList) = 0;
            pDList += sizeof(u32);
            TotalSize += sizeof(u32);
        }

        //---   export the frame strip packs
        s_pExpCacheFrames[s_NExpCacheFrames].FlushPacketOffset = (u32)pDList - (u32)s_pDataStart;
        ExportSize = ExportFramePacks( pDList, FrameID, IsEnvPass(SubMeshID), IsShadow(SubMeshID), IsAlpha(SubMeshID) );
        pDList += ExportSize;
        TotalSize += ExportSize;
        
        //---   and make sure we're still aligned for the VIF
        while ( (TotalSize & 0xf) != 0x00 )
        {
            *((u32*)pDList) = 0;
            pDList += sizeof(u32);
            TotalSize += sizeof(u32);
        }

        //---   now we can fill in the # of vert loads and flush packet size
        s_pExpCacheFrames[s_NExpCacheFrames].FlushPacketSize =
            ((u32)pDList - (u32)s_pDataStart) - s_pExpCacheFrames[s_NExpCacheFrames].FlushPacketOffset;
        s_pExpCacheFrames[s_NExpCacheFrames].NVertLoads =
            s_NExpVertLoads - s_pExpCacheFrames[s_NExpCacheFrames].FirstVertLoad;

        //---   another cache frame added successfully!
        s_NExpCacheFrames++;
    }

    //---   Pad out the DMA if necessary
    while ( (TotalSize & 0xf) != 0x00 )
    {
        *((u32*)pDList) = 0;
        pDList += sizeof(u32);
        TotalSize += sizeof(u32);
    }

    ASSERT( TotalSize = (s32)((u32)pDList - (u32)pStart) );

    return (s32)((u32)pDList - (u32)pStart);
}

//==========================================================================

xbool QPS2RipSkin::IsEnvPass( s32 SubMeshID )
{
    return m_pSubMesh[SubMeshID].EnvPass;
}

//==========================================================================

xbool QPS2RipSkin::IsShadow( s32 SubMeshID )
{
    return m_pSubMesh[SubMeshID].Shadow;
}

//==========================================================================

xbool QPS2RipSkin::IsAlpha( s32 SubMeshID )
{
    return m_pSubMesh[SubMeshID].Alpha;
}

//==========================================================================

xbool QPS2RipSkin::IsMorph( s32 SubMeshID )
{
    return m_pSubMesh[SubMeshID].Morph;
}

//==========================================================================

void QPS2RipSkin::Save( char* pFileName )
{
    s32                 i, j;
    s32                 MeshID, SubMeshID, MorphID, DeltaID;
    s32                 TotalSize = 0;
    s32                 TotalSizeAllocated = 0;
    t_SkinFileHeader    Hdr;
    t_SkinMesh*         pMeshInfo;
    t_SkinSubMesh*      pSubMeshInfo;
    t_SkinMorphTarget*  pMorphInfo;
    byte*               pDList;
    X_FILE*             fh;
    char                TextureName[32];

    //---   Allocate space for the data
    pMeshInfo = (t_SkinMesh*)x_malloc( sizeof(t_SkinMesh) * m_NMeshes );
    pSubMeshInfo = (t_SkinSubMesh*)x_malloc( sizeof(t_SkinSubMesh) * m_NSubmeshes );
    pMorphInfo = (t_SkinMorphTarget*)x_malloc( sizeof(t_SkinMorphTarget) * m_NMorphTargets );
    s_pDataStart = (byte*)x_malloc( 3 * 1024 * 1024 );
    TotalSizeAllocated += 3 * 1024 * 1024;
    ASSERT( pMeshInfo );
    ASSERT( pSubMeshInfo );
    ASSERT( s_pDataStart );
    pDList = s_pDataStart;

    //---   zero out the exported data
    s_NExpCacheFrames = 0;
    s_NExpCacheFramesAllocated = 0;
    s_pExpCacheFrames = NULL;
    s_NExpVertLoads = 0;
    s_NExpVertLoadsAllocated = 0;
    s_pExpVertLoads = NULL;
    s_NExpDeltaLocs = 0;
    s_NExpDeltaLocsAllocated = 0;
    s_pExpDeltaLocs = NULL;
    s_NExpDeltaValues = 0;
    s_NExpDeltaValuesAllocated = 0;
    s_pExpDeltaValues = NULL;
    s_pExpDeltaData = (byte*)x_malloc( 2 * 1024 * 1024 );
    ASSERT( s_pExpDeltaData );
    s_ExpDeltaDataSize = 0;
    s_pMorphData = NULL;
    s_NMorphDatas = 0;
    s_NMorphDatasAllocated = 0;

    //---   loop through Meshes and fill in the mesh data
    for ( MeshID = 0; MeshID < m_NMeshes; MeshID++ )
    {
        //---   fill in the mesh data
        FillName( pMeshInfo[MeshID].Name, m_pMesh[MeshID].Name, SKIN_MESH_NAME_LENGTH );
        pMeshInfo[MeshID].Flags = SKIN_MESH_FLAG_VISIBLE;
        if ( m_pMesh[MeshID].EnvMapped )
            pMeshInfo[MeshID].Flags |= SKIN_MESH_FLAG_ENVMAPPED;
        if ( m_pMesh[MeshID].Shadow )
            pMeshInfo[MeshID].Flags |= SKIN_MESH_FLAG_SHADOW;
        if ( m_pMesh[MeshID].Alpha )
            pMeshInfo[MeshID].Flags |= SKIN_MESH_FLAG_ALPHA;
        if ( m_pMesh[MeshID].NMorphTargets )
            pMeshInfo[MeshID].Flags |= SKIN_MESH_FLAG_MORPH;
        pMeshInfo[MeshID].NSubMeshes = m_pMesh[MeshID].NSubmeshes;
        pMeshInfo[MeshID].FirstSubMesh = m_pMesh[MeshID].SubmeshID;
        pMeshInfo[MeshID].NMorphTargets = m_pMesh[MeshID].NMorphTargets;
        pMeshInfo[MeshID].FirstMorphTarget = m_pMesh[MeshID].MorphTargetID;
        pMeshInfo[MeshID].FirstDeltaLocation = s_NExpDeltaLocs;
        pMeshInfo[MeshID].FirstDeltaValue = s_NExpDeltaValues;
        pMeshInfo[MeshID].DeltaDataOffset = s_ExpDeltaDataSize;

        if ( g_Verbose )
        {
            x_printf( "Mesh %2d: %3d SMs %3d MTs  EnvMap-%c  Shadow-%c  Alpha-%c\n",
                      MeshID,
                      m_pMesh[MeshID].NSubmeshes,
                      m_pMesh[MeshID].NMorphTargets,
                      m_pMesh[MeshID].EnvMapped ? 'Y' : 'N',
                      m_pMesh[MeshID].Shadow ? 'Y' : 'N',
                      m_pMesh[MeshID].Alpha ? 'Y' : 'N' );
        }

        //---   initialize variables that need to be used by the export sub-mesh phase
        s_pCurrMeshDeltaPacket = &s_pExpDeltaData[s_ExpDeltaDataSize];

        //---   reset the morph datas that will be used to generate delta values
        s_NMorphDatas = 0;

        //---   loop through SubMeshes and build packets
        for ( SubMeshID = pMeshInfo[MeshID].FirstSubMesh;
              SubMeshID < (pMeshInfo[MeshID].FirstSubMesh + pMeshInfo[MeshID].NSubMeshes);
              SubMeshID++ )
        {
            xbool   EnvPass;
            xbool   IsShad;
            xbool   Alpha;
            xbool   Morph;
            s32     SubMeshSize;

            //---   is this the environment pass of an environment-mapped mesh?
            EnvPass = IsEnvPass( SubMeshID );

            //---   is this a shadow?
            IsShad = IsShadow( SubMeshID );

            //---   should this mesh support alpha
            Alpha = IsAlpha( SubMeshID );

            //---   can this sub-mesh morph
            Morph = IsMorph( SubMeshID );

            //---   fill in some basic info
            pSubMeshInfo[SubMeshID].Flags = SKIN_SUBMESH_FLAG_VISIBLE;
            if ( EnvPass )
                pSubMeshInfo[SubMeshID].Flags |= SKIN_SUBMESH_FLAG_ENVPASS;
            if ( IsShad )
                pSubMeshInfo[SubMeshID].Flags |= SKIN_SUBMESH_FLAG_SHADOW;
            if ( Alpha )
                pSubMeshInfo[SubMeshID].Flags |= SKIN_SUBMESH_FLAG_ALPHA;
            if ( Morph )
                pSubMeshInfo[SubMeshID].Flags |= SKIN_SUBMESH_FLAG_MORPH;
            pSubMeshInfo[SubMeshID].TextureID = m_pSubMesh[SubMeshID].TextureID;
            if ( m_pSubMesh[SubMeshID].EnvPass )
            {
                pSubMeshInfo[SubMeshID].TextureID = m_NTextures - 1;
            }
            pSubMeshInfo[SubMeshID].FirstCacheFrame = s_NExpCacheFrames;
            pSubMeshInfo[SubMeshID].NCacheFrames = 0;   // we'll have to wait to fill this in
            pSubMeshInfo[SubMeshID].NVerts = m_pSubMesh[SubMeshID].NVertsLoaded;
            pSubMeshInfo[SubMeshID].NTris = m_pSubMesh[SubMeshID].NTris;

            ASSERT( TotalSize < (TotalSizeAllocated - 64 * 1024) );

            //---   export the byte stream for the sub-mesh
            SubMeshSize = ExportSubMesh( pDList, SubMeshID );
            pDList += SubMeshSize;
            TotalSize += SubMeshSize;
    
            //---   now we know the number of cache frames
            pSubMeshInfo[SubMeshID].NCacheFrames = s_NExpCacheFrames -
                                                   pSubMeshInfo[SubMeshID].FirstCacheFrame;

            //---   give the user some stats
            if ( g_Verbose )
            {
                s32 DMASize;
                s32 NVertLoads;

                //---   calculate the total DMA size for this sub-mesh
                DMASize = 0;
                NVertLoads = 0;
                for ( i = pSubMeshInfo[SubMeshID].FirstCacheFrame;
                      i < (pSubMeshInfo[SubMeshID].FirstCacheFrame + pSubMeshInfo[SubMeshID].NCacheFrames);
                      i++ )
                {
                    for ( j = s_pExpCacheFrames[i].FirstVertLoad;
                          j < (s_pExpCacheFrames[i].FirstVertLoad + s_pExpCacheFrames[i].NVertLoads);
                          j++ )
                    {
                        DMASize += s_pExpVertLoads[j].VertPacketSize;
                        DMASize += s_pExpVertLoads[j].KickPacketSize;
                        DMASize += s_pExpVertLoads[j].DeltaPacketSize;
                    }
                    NVertLoads += s_pExpCacheFrames[i].NVertLoads;
                    DMASize += s_pExpCacheFrames[i].FlushPacketSize;
                }
                x_printf( "  SubMesh %2d: %6d bytes %3d frames %3d vert loads Morph-%c\n",
                    SubMeshID,
                    DMASize,
                    pSubMeshInfo[SubMeshID].NCacheFrames,
                    NVertLoads,
                    m_pSubMesh[SubMeshID].Morph ? 'Y' : 'N' );
            }

            //---   Sanity check time!
            ASSERT( (TotalSize & 0xf) == 0 );
            ASSERT( ((u32)pDList - (u32)s_pDataStart) == (u32)TotalSize );

        }

        //---   fill in some more of the mesh information now that we know some more
        pMeshInfo[MeshID].NDeltaLocations = s_NExpDeltaLocs - pMeshInfo[MeshID].FirstDeltaLocation;
        pMeshInfo[MeshID].DeltaDataSize = s_ExpDeltaDataSize - pMeshInfo[MeshID].DeltaDataOffset;

        //---   build the delta values for each morph target
        ASSERT( s_NMorphDatas == pMeshInfo[MeshID].NDeltaLocations );
        for ( MorphID = pMeshInfo[MeshID].FirstMorphTarget;
              MorphID < (pMeshInfo[MeshID].FirstMorphTarget + pMeshInfo[MeshID].NMorphTargets);
              MorphID++ )
        {
            for ( i = 0; i < s_NMorphDatas; i++ )
            {
                //---   allocate space for a new delta value
                if ( s_NExpDeltaValues >= s_NExpDeltaValuesAllocated )
                {
                    s_NExpDeltaValuesAllocated += 32;
                    s_pExpDeltaValues = (t_SkinDeltaValue*)x_realloc(
                        s_pExpDeltaValues, s_NExpDeltaValuesAllocated * sizeof(t_SkinDeltaValue) );
                    ASSERT( s_pExpDeltaValues );
                }

                //---   find the delta value for this morph target
                for ( DeltaID = m_pMorphTarget[MorphID].DeltaID;
                      DeltaID < (m_pMorphTarget[MorphID].DeltaID + m_pMorphTarget[MorphID].NDeltas);
                      DeltaID++ )
                {
                    if ( m_pMorphDelta[DeltaID].VertID == s_pMorphData[i].VertID )
                        break;
                }
                ASSERT( DeltaID < (m_pMorphTarget[MorphID].DeltaID + m_pMorphTarget[MorphID].NDeltas) );
                s_pExpDeltaValues[s_NExpDeltaValues].XDelta = (s16)(m_pMorphDelta[DeltaID].Delta.X * 4096.0f);
                s_pExpDeltaValues[s_NExpDeltaValues].YDelta = (s16)(m_pMorphDelta[DeltaID].Delta.Y * 4096.0f);
                s_pExpDeltaValues[s_NExpDeltaValues].ZDelta = (s16)(m_pMorphDelta[DeltaID].Delta.Z * 4096.0f);
                s_NExpDeltaValues++;
            }
        }

        //---   loop through the morph targets, building them
        for ( MorphID = pMeshInfo[MeshID].FirstMorphTarget;
              MorphID < (pMeshInfo[MeshID].FirstMorphTarget + pMeshInfo[MeshID].NMorphTargets);
              MorphID++ )
        {
            FillName( pMorphInfo[MorphID].Name, m_pMorphTarget[MorphID].Name, SKIN_MORPH_NAME_LENGTH );
            
            //---   default the active morph target as the first morph target
            if ( MorphID == pMeshInfo[MeshID].FirstMorphTarget )
            {
                pMorphInfo[MorphID].Active = TRUE;
                pMorphInfo[MorphID].Weight = 1.0f;
            }
            else
            {
                pMorphInfo[MorphID].Active = FALSE;
                pMorphInfo[MorphID].Weight = 0.0f;
            }
        }

    }

    //---   give the user some more stats
    x_printf( "Total Data Size: %d bytes\n\n", TotalSize ); // #### INCLUDE DELTA PACKETS???

    //---   Fill in the header info
    FillName( Hdr.Name, m_Name, 32 );
    Hdr.Flags = 0;
    Hdr.NTextures = m_NTextures;
    Hdr.NBones = m_NBones;
    Hdr.NMeshes = m_NMeshes;
    Hdr.NSubMeshes = m_NSubmeshes;
    Hdr.NCacheFrames = s_NExpCacheFrames;
    Hdr.NVertLoads = s_NExpVertLoads;
    Hdr.NMorphTargets = m_NMorphTargets;
    Hdr.NDeltaLocations = s_NExpDeltaLocs;
    Hdr.NDeltaValues = s_NExpDeltaValues;
    Hdr.DeltaDataSize = s_ExpDeltaDataSize;
    Hdr.PacketSize = TotalSize;

    //---   open the destination file for writing
    fh = x_fopen( pFileName, "wb" );
    if ( !fh )
    {
        x_printf( "ERROR: Unable to open %s for writing\n", pFileName );
        PauseOnError();
        exit( 0 );
    }

    //---   export the header
    x_fwrite( &Hdr, sizeof(t_SkinFileHeader), 1, fh );

    //---   export the texture names
    for ( i = 0; i < m_NTextures; i++ )
    {
        GetTextureName( TextureName, m_pTexture[i].Name );
        x_fwrite( TextureName, 1, 32, fh );
    }

    //---   export the bone name data
    for ( i = 0; i < m_NBones; i++ )
    {
        x_fwrite( m_pBone[i].Name, sizeof(char), 32, fh );
    }

    //---   export the mesh data
    x_fwrite( pMeshInfo, sizeof(t_SkinMesh), m_NMeshes, fh );

    //---   export the sub-mesh data
    x_fwrite( pSubMeshInfo, sizeof(t_SkinSubMesh), m_NSubmeshes, fh );

    //---   export the cache frame data
    x_fwrite( s_pExpCacheFrames, sizeof(t_SkinCacheFrame), s_NExpCacheFrames, fh );

    //---   export the vert load data
    x_fwrite( s_pExpVertLoads, sizeof(t_SkinVertLoad), s_NExpVertLoads, fh );

    //---   export the morph target data
    x_fwrite( pMorphInfo, sizeof(t_SkinMorphTarget), m_NMorphTargets, fh );

    //---   export the delta location data
    x_fwrite( s_pExpDeltaLocs, sizeof(t_SkinDeltaLoc), s_NExpDeltaLocs, fh );

    //---   export the delta value data
    x_fwrite( s_pExpDeltaValues, sizeof(t_SkinDeltaValue), s_NExpDeltaValues, fh );

    //---   export the packet data
    x_fwrite( s_pDataStart, 1, TotalSize, fh );

    //---   export the delta data
    x_fwrite( s_pExpDeltaData, sizeof(byte), s_ExpDeltaDataSize, fh );

    //---   close the destination file
    x_fclose( fh );

    //---   Free up our temporary data
    x_free( pMeshInfo );
    x_free( pSubMeshInfo );
    x_free( s_pDataStart );
    x_free( s_pExpVertLoads );
    x_free( s_pExpCacheFrames );
    x_free( pMorphInfo );
    if ( s_pExpDeltaLocs )
        x_free( s_pExpDeltaLocs );
    if ( s_pExpDeltaValues )
        x_free( s_pExpDeltaValues );
    if ( s_pMorphData )
        x_free( s_pMorphData );
    x_free( s_pExpDeltaData );
}

//==========================================================================
// Internal routines
//==========================================================================

typedef struct SNormalVert
{
    vector3 Pos;
    vector3 Normal;
} t_NormalVert;

void QPS2RipSkin::ComputeNormals( void )
{
    s32             i, j, TriID;
    s32             NTempVerts;
    t_NormalVert*   pTempVerts;

    //---   Clear normals
    for( i = 0; i < m_NVerts; i++ )
    {
        m_pVert[i].Normal.Zero();
    }

    //---   allocate some temp space
    pTempVerts = new SNormalVert[m_NVerts];
    ASSERT( pTempVerts );
    NTempVerts = 0;

    //===   to avoid normals looking weird across texture seams, do them
    //      based on position (NOT INDEX)

    //---   Create a list of vertices with nothing but position
    for ( i = 0; i < m_NVerts; i++ )
    {
        //---   find this vert in the temporary list
        for ( j = 0; j < NTempVerts; j++ )
        {
            if ( (x_abs(pTempVerts[j].Pos.X - m_pVert[i].Pos.X) < EPSILON) &&
                 (x_abs(pTempVerts[j].Pos.Y - m_pVert[i].Pos.Y) < EPSILON) &&
                 (x_abs(pTempVerts[j].Pos.Z - m_pVert[i].Pos.Z) < EPSILON) )
            {
                break;
            }
        }
        if ( j == NTempVerts )
        {
            //---   the current vert wasn't found, so add it
            pTempVerts[NTempVerts].Normal.Zero();
            pTempVerts[NTempVerts].Pos = m_pVert[i].Pos;
            NTempVerts++;
        }
    }

    //---   for each vert that a triangle uses, add in the triangle's normal, weighted by its area
    for ( TriID = 0; TriID < m_NTris; TriID++ )
    {
        for ( i = 0; i < 3; i++ )
        {
            //---   find each of the verts in the temporary list
            for ( j = 0; j < NTempVerts; j++ )
            {
                if ( (x_abs(pTempVerts[j].Pos.X - m_pVert[m_pTri[TriID].VertID[i]].Pos.X) < EPSILON) &&
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


    //---   now, copy the temp verts normals back into the real normals
    for ( i = 0; i < NTempVerts; i++ )
    {
        //---   find every occurence of the temp vert in the mesh
        for ( j = 0; j < m_NVerts; j++ )
        {
            if ( (x_abs(pTempVerts[i].Pos.X - m_pVert[j].Pos.X) < EPSILON) &&
                 (x_abs(pTempVerts[i].Pos.Y - m_pVert[j].Pos.Y) < EPSILON) &&
                 (x_abs(pTempVerts[i].Pos.Z - m_pVert[j].Pos.Z) < EPSILON) )
            {
                m_pVert[j].Normal = pTempVerts[i].Normal;
            }
        }
    }

    //---   clean up, eh?
    delete []pTempVerts;

    //---   Average out normals by renormalizing
    for( i = 0; i < m_NVerts; i++ )
    {
        m_pVert[i].Normal.Normalize();
    }
}


//=====================================================================================================================================
void QPS2RipSkin::BlendVertexNormals( void )
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

void QPS2RipSkin::BuildTransforms( void )
{
    s32 i, j, k;

    //---   Loop through verts
    for( i = 0; i < m_NVerts; i++ )
    {
        //---   Sort vertex weights by matrix (selection sort)
        for( j = 0; j < m_pVert[i].NMatrices; j++ )
        {
            s32 BestIndex = j;
            s32 TempIndex;
            f32 TempWeight;

            //---   Find best 
            for( k = j + 1; k < m_pVert[i].NMatrices; k++ )
            {
                if( m_pVert[i].MatrixID[k] < m_pVert[i].MatrixID[BestIndex] )
                    BestIndex = k;
            }

            //---   Swap the best index and the j-th index
            TempIndex = m_pVert[i].MatrixID[BestIndex];
            m_pVert[i].MatrixID[BestIndex] = m_pVert[i].MatrixID[j];
            m_pVert[i].MatrixID[j] = TempIndex;

            TempWeight = m_pVert[i].MatrixWeight[BestIndex];
            m_pVert[i].MatrixWeight[BestIndex] = m_pVert[i].MatrixWeight[j];
            m_pVert[i].MatrixWeight[j] = TempWeight;
        }

        //---   Search for match in current transform list
        for( j = 0; j < m_NTransforms; j++ )
        {
            //---   Check if same number of matrices
            if( m_pTransform[j].NMatrices != m_pVert[i].NMatrices )
                continue;

            //---   Check all indices
            for( k = 0; k < m_pTransform[j].NMatrices; k++ )
            {
                if( m_pTransform[j].MatrixID[k] != m_pVert[i].MatrixID[k] )
                    break;
            }
            
            //---   If the transforms match... break
            if( k == m_pTransform[j].NMatrices )
                break;            
        }

        //---   Check if we did not find a matching transform
        if( j == m_NTransforms )
        {
            //---   Check if we need to allocate a new Transform
            if( m_NTransformsAllocated <= m_NTransforms )
            {
                m_NTransformsAllocated += 32;
                m_pTransform = (t_RipTransform*)x_realloc( m_pTransform,
                                                           sizeof(t_RipTransform) * m_NTransformsAllocated );
                ASSERT( m_pTransform );
            }

            //---   Add transform
            m_pTransform[m_NTransforms].NVerts = 0;
            m_pTransform[m_NTransforms].NMatrices = m_pVert[i].NMatrices;
            for( k = 0; k < 8; k++ )
                m_pTransform[m_NTransforms].MatrixID[k] = m_pVert[i].MatrixID[k];
            m_NTransforms++;
        }

        m_pTransform[j].NVerts++;

    }

    //---   Sort transforms by nverts (selection sort)
    for( j = 0; j < m_NTransforms; j++ )
    {
        s32             BestIndex = j;
        t_RipTransform  TempRipTransform;

        //---   Find best 
        for( k = j + 1; k < m_NTransforms; k++ )
        {
            if( m_pTransform[k].NVerts > m_pTransform[BestIndex].NVerts )
                BestIndex = k;
        }

        //---   Swap
        TempRipTransform = m_pTransform[BestIndex];
        m_pTransform[BestIndex] = m_pTransform[j];
        m_pTransform[j] = TempRipTransform;
    }

    //---   Map verts into sorted transforms
    for( i = 0; i < m_NVerts; i++ )
    {
        //---   Search for match in current transform list
        for( j = 0; j < m_NTransforms; j++ )
        {
            //---   Check if same number of matrices
            if( m_pTransform[j].NMatrices != m_pVert[i].NMatrices )
                continue;

            //---   Check all indices
            for( k = 0; k < m_pTransform[j].NMatrices; k++ )
            {
                if( m_pTransform[j].MatrixID[k] != m_pVert[i].MatrixID[k] )
                    break;
            }
            
            //---   If the transforms match... break
            if( k == m_pTransform[j].NMatrices )
                break;            
        }
        ASSERT( j != m_NTransforms );
        m_pVert[i].TransformID = j;
    }

    //---   if verbose is on spit out the transform counts
    s32 OneBoneTotal = 0;
    s32 TwoBoneTotal = 0;
    s32 ThreeBoneTotal = 0;
    for ( i = 0; i < m_NTransforms; i++ )
    {
        if ( m_pTransform[i].NMatrices == 1 )
            OneBoneTotal++;
        else
        if ( m_pTransform[i].NMatrices == 2 )
            TwoBoneTotal++;
        else
        if ( m_pTransform[i].NMatrices == 3 )
            ThreeBoneTotal++;
        else
        {
            ASSERT( FALSE );
        }
    }

    if ( g_Verbose )
    {
        x_printf( "  One bone transforms: %d\n", OneBoneTotal );
        x_printf( "  Two bone transforms: %d\n", TwoBoneTotal );
        x_printf( "Three bone transforms: %d\n", ThreeBoneTotal );
    }
}

//==========================================================================

void QPS2RipSkin::ConnectTrisToVerts( void )
{
    s32 i, j;

    //---   Clear verts
    for( i = 0; i < m_NVerts; i++ )
    {
        m_pVert[i].FirstTri = -1;
        m_pVert[i].NTris    = 0;
    }

    //---   Loop through tris
    for( i = 0; i < m_NTris; i++ )
    {
        for( j = 0; j < 3; j++ )
        {
            //---   insert the tri into each verts linked list of tris
            s32 VertID = m_pTri[i].VertID[j];
            m_pTri[i].VertNextTriID[j] = m_pVert[VertID].FirstTri;
            m_pVert[VertID].FirstTri = i;
            m_pVert[VertID].NTris++;
        }
    }
}

//==========================================================================

void QPS2RipSkin::ConnectTrisToTris( void )
{
    s32 i, j;

    //---   Clear tris' neighbors
    for( i = 0; i < m_NTris; i++ )
    {
        m_pTri[i].NeighborTriID[0] = -1;
        m_pTri[i].NeighborTriID[1] = -1;
        m_pTri[i].NeighborTriID[2] = -1;
    }

    //---   Loop through tris
    for( i = 0; i < m_NTris; i++ )
    {
        //---   Loop through verts looking for triangle match
        for( j = 0; j < 3; j++ )
        {
            s32 VertID = m_pTri[i].VertID[j];
            s32 TriLink = m_pVert[VertID].FirstTri;

            //---   go through the linked list of tris that use this vert
            while( TriLink != -1 )
            {
                //---   See if this tri is already marked as a neighbor, or
                //      if it is the same tri
                if( (TriLink != i) && 
                    (m_pTri[i].NeighborTriID[0] != TriLink) &&
                    (m_pTri[i].NeighborTriID[1] != TriLink) &&
                    (m_pTri[i].NeighborTriID[2] != TriLink) )
                {
                    //---   we have found another tri that uses this vert
                    s32 NShared = 0;

                    //---   Determine if tris share two verts
                    if( (m_pTri[i].VertID[0] == m_pTri[TriLink].VertID[0]) ||
                        (m_pTri[i].VertID[0] == m_pTri[TriLink].VertID[1]) ||
                        (m_pTri[i].VertID[0] == m_pTri[TriLink].VertID[2]) )
                    {
                        NShared++;
                    }
                    if( (m_pTri[i].VertID[1] == m_pTri[TriLink].VertID[0]) ||
                        (m_pTri[i].VertID[1] == m_pTri[TriLink].VertID[1]) ||
                        (m_pTri[i].VertID[1] == m_pTri[TriLink].VertID[2]) )
                    {
                        NShared++;
                    }
                    if( (m_pTri[i].VertID[2] == m_pTri[TriLink].VertID[0]) ||
                        (m_pTri[i].VertID[2] == m_pTri[TriLink].VertID[1]) ||
                        (m_pTri[i].VertID[2] == m_pTri[TriLink].VertID[2]) )
                    {
                        NShared++;
                    }

                    if( NShared >= 2 )
                    {
                        if ( m_pTri[i].TextureID == m_pTri[TriLink].TextureID )
                        {
                            //---   the tris share two verts so, it is a neighbor
                            if( m_pTri[i].NeighborTriID[0] == -1 )
                                m_pTri[i].NeighborTriID[0] = TriLink;
                            else
                            if( m_pTri[i].NeighborTriID[1] == -1 )
                                m_pTri[i].NeighborTriID[1] = TriLink;
                            else
                            if( m_pTri[i].NeighborTriID[2] == -1 )
                                m_pTri[i].NeighborTriID[2] = TriLink;
                        }
                    }
                }

                //---   Move to next tri in the linked list
                if( m_pTri[TriLink].VertID[0] == VertID )
                    TriLink = m_pTri[TriLink].VertNextTriID[0];
                else
                if( m_pTri[TriLink].VertID[1] == VertID )
                    TriLink = m_pTri[TriLink].VertNextTriID[1];
                else
                if( m_pTri[TriLink].VertID[2] == VertID )
                    TriLink = m_pTri[TriLink].VertNextTriID[2];
            }

            //---   If we've found 3 neighbors already, quit
            if( m_pTri[i].NeighborTriID[2] != -1 )
                break;
        }
    }
}

//==========================================================================

void QPS2RipSkin::SolveSubMesh( s32 SubMeshID )
{
    if( g_ReportPathDetails )
        x_printf( "%1d Tris in submesh.\n", m_pSubMesh[SubMeshID].NTris );

    //---   Set the starting cache frame
    m_pSubMesh[SubMeshID].CacheFrameID = s_NFrames;

    //---   figure out the cache frames for this submesh
    SolveTransformPath( SubMeshID,
                        m_pSubMesh[SubMeshID].TriID, 
                        m_pSubMesh[SubMeshID].TriID + m_pSubMesh[SubMeshID].NTris - 1 );

    //---   we now know the number of cache frames for this submesh
    m_pSubMesh[SubMeshID].NCacheFrames = (s_NFrames - m_pSubMesh[SubMeshID].CacheFrameID);
}

//==========================================================================

static
void CACHE_ClearVerts( void )
{
    s32 i;

    s_Cache.NUsed = 0;
    for( i = 0; i < CACHE_NVERTS; i++ )
    {
        s_Cache.Vert[i].Used = FALSE;
        s_Cache.Vert[i].VertID  = -1;
    }
}

//==========================================================================

static
void CACHE_Init( void )
{
    CACHE_ClearVerts();
}

//==========================================================================

void QPS2RipSkin::SolveTransformPath( s32 SubMeshID, s32 StartTri, s32 EndTri )
{
    s32     i;
    s32     Try, CurrentTri;
    f32     BestPathScore;  // a lower score is better
    s32     NStarts;
    t_Cache BackupCache;
    s32     NTrisSolved;
    s32     NTris;
    s32     NTrisAdded;
    s64     LoopTime;
    s64     TotalLoopTime;
    s64     TotalTime;
    f32     Score = 0;

    //---   clear out the cache
    CACHE_Init();

    //---   Backup the cache
    BackupCache     = s_Cache;

    //---   Get some start values
    NTris           = (EndTri - StartTri + 1);
    NStarts         = (s32)(NTris * NUM_PATH_STARTS);
    if ( NStarts == 0 )
        NStarts = 1;
    BestPathScore   = 1000000000.0f;

    //---   Again, get some starting values
    NTrisAdded = 0;

    //---   set up some timer stats
    TotalTime = x_GetTimerTicks();
    TotalLoopTime = 0;

    //---   let the user know what's going on...
    if ( g_ReportPathDetails && g_Verbose )
        x_printf( "---- Solving transform path with %1d tries ----\n", NStarts );

    for( Try = 0; Try < NStarts; Try++ )
    {
        f32 PathScore;

        //---   give the user some idea that we haven't just crashed
        if ( g_ReportPathDetails && g_Verbose )
            x_printf( "." );

        //---   Set cache to starting state
        s_Cache = BackupCache;

        //---   Set all tris as available
        for( i = 0; i < NTris; i++ )
        {
            m_pTri[StartTri + i].Used = FALSE;
            m_pTri[StartTri + i].Rendered = FALSE;
            m_pTri[StartTri + i].Satisfied = FALSE;
        }

        //---   Clear all transforms as used
        for( i = 0; i < m_NTransforms; i++ )
            m_pTransform[i].Used = FALSE;

        //---   Clear all verts as cached
        for( i = 0; i < m_NVerts; i++ )
        {
            m_pVert[i].CacheID = -1;
            m_pVert[i].NTrisDepending = m_pVert[i].NTris;
        }

        //---   Choose starting current Tri
        //      NOTE: This should probably be random, but since
        //      the number of tries equals the number of triangles
        //      lets just use the current triangle, as is

        //CurrentTri = x_irand( StartTri, EndTri );
        CurrentTri = StartTri + (Try % NTris);
        m_pTri[CurrentTri].Used = TRUE;
        s_pTri[0].TestPathID = CurrentTri;
        PathScore = 0;
        NTrisSolved = 1;
        PathScore = AddTriToCache( CurrentTri );

        while( NTrisSolved < NTris )
        {
            s32 BestTri;
            f32 BestError;

            //---   Find the best tri to add next
            LoopTime = x_GetTimerTicks();
            FindBestNextTri( BestTri, BestError, StartTri, NTris );
            TotalLoopTime += x_GetTimerTicks() - LoopTime;

            //---   could we find a tri to add?
            if( BestTri == -1 )
            {
                //---   if BestTri is -1, that means all of the tris have already
                //      been added through other means (in other words, this path
                //      has already included all the tris through shared verts)
                break;
            }
            ASSERT( (BestTri >= StartTri) && (BestTri < (StartTri + NTris)) );

            //---   Update cache with new tri
            BestError = AddTriToCache( BestTri );

            //---   Choose this as next tri
            s_pTri[NTrisSolved].TestPathID = BestTri;
            m_pTri[BestTri].Used = TRUE;

            //---   Add error to totals
            PathScore += BestError;
            s_pTri[NTrisSolved].TestPathRelativeError = BestError;
            s_pTri[NTrisSolved].TestPathTotalError = PathScore;
            NTrisSolved++;

            //---   If PathScore is already worse than best found so far...break
            if( PathScore > BestPathScore )
                break;
        }

        //---   Check if this path beats previous
        if( PathScore < BestPathScore )
        {
            BestPathScore = PathScore;
            NTrisAdded = NTrisSolved;
            for( i = 0; i < NTrisAdded; i++ )
            {
                s_pTri[i].BestPathID              = s_pTri[i].TestPathID;
                s_pTri[i].BestPathRelativeError   = s_pTri[i].TestPathRelativeError;
                s_pTri[i].BestPathTotalError      = s_pTri[i].TestPathTotalError;
            }
        }
    }

    //---   give the user some idea that we haven't just crashed
    if ( g_ReportPathDetails && g_Verbose )
        x_printf( "\n" );

    //---   We have found 'best' path through triangles. This time, record it
    s_Cache = BackupCache;
    s_Record = TRUE;
    s_NVertsLoaded = 0;
    s_NTriFlushes = 0;
    s_TotalTrisFlushed = 0;
    s_TotalVertsFlushed = 0;
    s_NTransformSwitches = 0;

    //---   Set cache to starting state
    s_Cache = BackupCache;

    //---   Set all tris as available
    for( i = 0; i < NTris; i++ )
    {
        m_pTri[StartTri + i].Used = FALSE;
        m_pTri[StartTri + i].Rendered = FALSE;
        m_pTri[StartTri + i].Satisfied = FALSE;
    }

    //---   Clear all transforms as used
    for( i = 0; i < m_NTransforms; i++ )
        m_pTransform[i].Used = FALSE;

    //---   Clear all verts as cached
    for( i = 0; i < m_NVerts; i++ )
    {
        m_pVert[i].CacheID = -1;
        m_pVert[i].NTrisDepending = m_pVert[i].NTris;
    }

    //---   Start cache frame
    NewCacheFrame();

    //---   loop through the best path, adding tris to the cache
    for( i = 0; i < NTrisAdded; i++ )
    {
        f32 SubScore;
        s32 TriID = s_pTri[i].BestPathID;
        SubScore = AddTriToCache( TriID );
        Score += SubScore;

        if( g_ReportPathDetails )
        {
            x_printf( "%4d]  %4d:V  %4d:Try  %10f  %10f  (%4d,%4d,%4d)\n",
                i,
                s_Cache.NUsed,
                s_pTri[i].BestPathID,
                Score, SubScore,
                m_pTri[TriID].VertID[0],
                m_pTri[TriID].VertID[1],
                m_pTri[TriID].VertID[2] );
        }
    }

    FlushTrisFromCache();
    FlushUnusedVertsFromCache();

    //---   Back up one frame, because FlushTrisFromCache() would have
    //      incremented it one too many times
    s_NFrames--;

    s_Record = FALSE;

    //---   Display BEST CASE
    if( g_ReportPathDetails )
    {
        s32 BestNVertsLoaded;
        s32 BestNTriFlushes;

        //---   find what the theoretical optimal solution would have been
        BestNTriFlushes = 1 + (m_NTris / (CACHE_NVERTS*2));
        BestNVertsLoaded = m_NVerts;

        //---   now let the user know how RipSkin compared to the
        //      theoretical optimal solution
        x_printf( "\n" );
        x_printf( "VERT LOADS:            %1d\n", s_NVertsLoaded );
        x_printf( "BEST CASE VERT LOADS:  %1d\n", BestNVertsLoaded );
        x_printf( "\n" );
        x_printf( "TRI FLUSHES:           %1d\n", s_NTriFlushes );
        x_printf( "BEST CASE TRI FLUSHES: %1d\n", BestNTriFlushes);
        x_printf( "\n");
        x_printf( "TotalTrisFlushed %1d\n", s_TotalTrisFlushed);
        x_printf( "TotalVertsFlushed %1d\n", s_TotalVertsFlushed);
        x_printf( "TotalTransformSwitched %1d\n", s_NTransformSwitches);
        x_printf( "MinTransformSwitches %1d\n", m_NTransforms);
    }

    m_pSubMesh[SubMeshID].NVertsLoaded = s_NVertsLoaded;
    m_pSubMesh[SubMeshID].NTriFlushes = s_NTriFlushes;
    m_pSubMesh[SubMeshID].NTransforms = s_NTransformSwitches;

    //---   Return cache to original state
    s_Cache = BackupCache;
}

//==========================================================================

f32 QPS2RipSkin::AddTriToCache( s32 TriID )
{
    s32 i;
    s32 NVertsFound;
    s32 CacheID[3];
    f32 Error = 0;
    s32 NNewTransforms;
    s32 NTrisAdded;

    //---   Compute number of new transforms needed
    NNewTransforms = 0;
    for( i = 0; i < 3; i++ )
    {
        s32 VertID = m_pTri[TriID].VertID[i];
        s32 TransformID = m_pVert[VertID].TransformID;
        if( !m_pTransform[TransformID].Used )
            NNewTransforms++;
    }

    //---   Check to see if it would be worth while to flush
    if( (s_Cache.NUsed >= CACHE_FLUSH_BOUNDARY) &&
        (NNewTransforms > 0) )
    {
        if( s_Record && g_ReportPathDetails )
        {
            x_printf("EARLY FLUSH DUE TO NEW TRANSFORM\n");
        }
        FlushTrisFromCache();
        FlushUnusedVertsFromCache();

        //---   Clear all transforms as used
        for ( i = 0; i < m_NTransforms; i++ )
            m_pTransform[i].Used = FALSE;
    }

    //---   Find locations in cache for verts needed
    NVertsFound = FindCacheEntries( TriID, CacheID );

    //---   If not enough spots in the cache free, flush tris
    if( NVertsFound < 3 )
    {
        s32 NTrisFlushed;
        s32 NVertsFlushed;

        Error += 100;   // assign a small penalty, since we had to flush the cache
        NTrisFlushed = FlushTrisFromCache();
        NVertsFlushed = FlushUnusedVertsFromCache();

        //---   Clear all transforms as used
        for( i = 0; i < m_NTransforms; i++ )
            m_pTransform[i].Used = FALSE;

        //---   Find locations in cache for verts needed
        NVertsFound = FindCacheEntries( TriID, CacheID );
    }

    //---   If still not enough verts in cache, nuke it
    //      NOTE: It is EXTREMELY rare that we would ever get to this point,
    //      and frankly it should NEVER happen in the best case
    if( NVertsFound < 3 )
    {
        Error += NUKE_CACHE_PENALTY;

        //---   report a nuke to the user
        if( s_Record && g_ReportPathDetails )
        {
            x_printf( "NUKING CACHE:\n" );
            //ASSERT( FALSE );
        }

        //---   clear out the cache
        FlushTrisFromCache();
        FlushUnusedVertsFromCache();

        //---   Clear all verts from cache
        for( i = 0; i < CACHE_NVERTS; i++ )
        {
            if ( s_Cache.Vert[i].Used )
            {
                t_RipVert* pV = &m_pVert[ s_Cache.Vert[i].VertID ];
                pV->CacheID = -1;
                s_Cache.Vert[i].VertID = -1;
                s_Cache.Vert[i].Used = FALSE;
            }
        }
        s_Cache.NUsed = 0;

        // ####
        for ( i = 0 ; i < m_NTris; i++ )
            m_pTri[i].Used = FALSE;

        //---   Clear all transforms as used
        for( i=0; i<m_NTransforms; i++ )
            m_pTransform[i].Used = FALSE;

        //---   Find locations in cache for verts needed
        NVertsFound = FindCacheEntries( TriID, CacheID );
    }

    ASSERT( NVertsFound == 3 );

    //---   Add verts to cache and turn on any transforms used.
    for( i = 0; i < 3; i++ )
    {
        if( CacheID[i] < 1000 )
        {
            //---   No cost since vert is already in cache
        }
        else
        {
            s32 VertID = m_pTri[TriID].VertID[i];
            s32 VertCacheID = CacheID[i] - 1000;
            s32 TransformID = m_pVert[VertID].TransformID;

            //---   we had to load the vert, assign a penalty for that
            Error += VERT_LOAD_PENALTY;
            s_NVertsLoaded++;

            //---   do we have to do a new transform?
            if( !m_pTransform[TransformID].Used )
            {
                if( s_Record && g_ReportPathDetails )
                {
                    x_printf( "NEW TRANSFORM: %1d\n", TransformID );
                }
                s_NTransformSwitches++;
                Error += TRANSFORM_PENALTY;
            }

            //---   mark the transform as used, and assign the CacheID
            m_pTransform[TransformID].Used = TRUE;
            ASSERT( m_pVert[VertID].CacheID == -1 );
            m_pVert[VertID].CacheID = VertCacheID;
            s_Cache.Vert[VertCacheID].VertID = VertID;
            s_Cache.Vert[VertCacheID].Used = TRUE;
            
            if( s_Record )
            {
                AddVertToCacheFrame( VertID, VertCacheID );
            }
        }
    }

    //---   Add bonus for being connected to previous tris
    for( i = 0; i < 3; i++ )
    {
        s32 NeighborID = m_pTri[TriID].NeighborTriID[i];

        if( NeighborID != -1 )
        {
            if( (m_pTri[NeighborID].Satisfied) && (!m_pTri[NeighborID].Rendered) )
            {
                Error -= NEIGHBOR_ADJACENT_BONUS;
            }
        }
    }

    //---   Since this tri is in the cache, update dependencies
    s_Cache.NUsed = 0;
    NTrisAdded = 0;
    for( i = 0; i < CACHE_NVERTS; i++ )
    {
        if( s_Cache.Vert[i].Used )
        {
            NTrisAdded += UpdateVertDependencies( s_Cache.Vert[i].VertID );
            s_Cache.NUsed++;
        }
    }

    //---   Add bonus for bringing in new tris
    if( NTrisAdded > 1 )
    {
        Error -= (NTrisAdded - 1) * NEIGHBOR_SATISFIED_BONUS;
    }

    return Error;
}

//==========================================================================

s32 QPS2RipSkin::FlushTrisFromCache( void )
{
    s32 i;
    s32 NRendered;

    if( s_Record )
    {
        if( s_NFrames > 0 )
            EndCacheFrame();
        NewCacheFrame();
    }

    //---   increment the number of tri flushes
    s_NTriFlushes++;

    //---   Loop through tris and see if they are satisfied
    NRendered = 0;
    for( i = 0; i < m_NTris; i++ )
    {
        if( (!m_pTri[i].Rendered) && (m_pTri[i].Satisfied) )
        {
            m_pTri[i].Rendered = TRUE;
            NRendered++;
        }
    }

    //---   give the user some info
    if( s_Record && g_ReportPathDetails )
    {
        x_printf("FLUSHING TRIS FROM CACHE: %1d\n",NRendered);
    }

    s_TotalTrisFlushed += NRendered;
    
    return NRendered;
}

//==========================================================================

void QPS2RipSkin::EndCacheFrame( void )
{
}

//==========================================================================

void QPS2RipSkin::NewCacheFrame( void )
{

    s32 i,j;

    //---   Check if we need to allocate a new frame
    if( s_NFramesAllocated <= s_NFrames )
    {
        s_NFramesAllocated += 16;
        s_pFrame = (t_CacheFrame*)x_realloc( s_pFrame,
                                             sizeof(t_CacheFrame) * s_NFramesAllocated );
        ASSERT( s_pFrame );
    }

    //---   Clear structure
    x_memset( &s_pFrame[s_NFrames], 0, sizeof(t_CacheFrame) );

    //---   Copy transforms from the RipSkin data
    for( i = 0; i < m_NTransforms; i++ )
    {   
        s_pFrame[s_NFrames].Transform[i].NMatrices = m_pTransform[i].NMatrices;
        for( j = 0; j < m_pTransform[i].NMatrices; j++ )
            s_pFrame[s_NFrames].Transform[i].MatrixID[j] = m_pTransform[i].MatrixID[j];
    }

    //---   increment the number of frames
    s_NFrames++;
}

//==========================================================================

s32 QPS2RipSkin::FlushUnusedVertsFromCache( void )
{
    s32 i;
    s32 NFlushed = 0;

    //---   Remove unused verts
    for( i = 0; i < CACHE_NVERTS; i++ )
    {
        if( s_Cache.Vert[i].Used )
        {
            s32 VertID = s_Cache.Vert[i].VertID;

            if( m_pVert[VertID].NTrisDepending == 0 )
            {
                m_pVert[VertID].CacheID = -1;
                s_Cache.Vert[i].VertID = -1;
                s_Cache.Vert[i].Used = FALSE;
                NFlushed++;
            }
        }
    }

    //---   Re-calculate the number of verts that are used
    s_Cache.NUsed = 0;
    for( i = 0; i < CACHE_NVERTS; i++ )
    {
        if( s_Cache.Vert[i].Used )
            s_Cache.NUsed++;
    }

    //---   Let the user know what's going on
    if( s_Record && g_ReportPathDetails )
    {
        x_printf("FLUSHING VERTS FROM CACHE: %1d\n",NFlushed);
    }

    s_TotalVertsFlushed += NFlushed;
    return NFlushed;
}

//==========================================================================

s32 QPS2RipSkin::FindCacheEntries( s32 TriID, s32* pCacheID )
{
    s32 i, j;
    s32 NVertsFound;
    s32 LastEmpty;

    //---   Find locations in cache for verts needed
    NVertsFound=0;
    LastEmpty=0;
    for( i = 0; i < 3; i++ )
    {
        t_RipVert* pV = &m_pVert[m_pTri[TriID].VertID[i]];

        if( pV->CacheID != -1 )
        {
            //---   the vert has a cache id, so use that
            pCacheID[i] = pV->CacheID;
            NVertsFound++;
        }
        else
        {
            //---   look for an empty spot in the cache
            for( j = LastEmpty; j < CACHE_NVERTS; j++ )
            {
                if( !s_Cache.Vert[j].Used ) 
                    break;
            }

            if( j < CACHE_NVERTS )
            {
                //---   the vert is not in the cache, but there is room for
                //      it, signal this by putting what its index should be
                //      plus 1000. (We'll do a compare on > 1000 later on)
                pCacheID[i] = 1000 + j;
                LastEmpty = j + 1;
                NVertsFound++;
            }
        }
    }

    return NVertsFound;
}

//==========================================================================

void QPS2RipSkin::AddVertToCacheFrame( s32 VertID, s32 CacheID )
{
    t_CacheVertList* pVertList;

    //---   get a pointer the the cache vert list
    ASSERT( s_NFrames>0 );
    pVertList = &s_pFrame[s_NFrames - 1].Transform[m_pVert[VertID].TransformID];

    //---   add the vert to the cache frame
    pVertList->VertID[ pVertList->NVerts ] = VertID;
    pVertList->CacheID[ pVertList->NVerts ] = CacheID;

    //---   increment our counter
    pVertList->NVerts++;
}

//==========================================================================

s32 QPS2RipSkin::UpdateVertDependencies( s32 VertID )
{
    s32 NSatisfied = 0;
    s32 NNewSatisfied = 0;
    s32 NTotal = 0;
    s32 OldTriID;
    s32 TriID = m_pVert[VertID].FirstTri;

    ASSERT( m_pVert[VertID].CacheID != -1 );

    //---   Loop through all tris using this vert and see if all are rendered
    OldTriID = -2;
    while( TriID != -1 )
    {
        ASSERT( TriID != OldTriID );
        OldTriID = TriID;

        //---   Count totals
        NTotal++;
        if( m_pTri[TriID].Rendered || m_pTri[TriID].Satisfied )
        {
            NSatisfied++;
        }
        else
        {
            if( ( m_pVert[m_pTri[TriID].VertID[0]].CacheID != -1 ) &&
                ( m_pVert[m_pTri[TriID].VertID[1]].CacheID != -1 ) &&
                ( m_pVert[m_pTri[TriID].VertID[2]].CacheID != -1 ) )
            {
                NSatisfied++;
                NNewSatisfied++;
                m_pTri[TriID].Satisfied = TRUE;  

                if( s_Record )
                {
                    AddTriToCacheFrame( TriID, 
                                        m_pVert[m_pTri[TriID].VertID[0]].CacheID,
                                        m_pVert[m_pTri[TriID].VertID[1]].CacheID,
                                        m_pVert[m_pTri[TriID].VertID[2]].CacheID );
                }

            }
        }

        //---   move to the next tri in the linked list
        if( m_pTri[TriID].VertID[0] == VertID )
            TriID = m_pTri[TriID].VertNextTriID[0];     
        else
        if( m_pTri[TriID].VertID[1] == VertID )
            TriID = m_pTri[TriID].VertNextTriID[1];
        else
        if( m_pTri[TriID].VertID[2] == VertID )
            TriID = m_pTri[TriID].VertNextTriID[2];
    }
    ASSERT( NTotal == m_pVert[VertID].NTris );

    //---   Compute number of dependencies on vert still remaining
    m_pVert[VertID].NTrisDepending = NTotal - NSatisfied;

    //---   Return total tris satisfied
    return NNewSatisfied;
}

//==========================================================================

void QPS2RipSkin::AddTriToCacheFrame( s32 TriID, s32 CacheID0, s32 CacheID1, s32 CacheID2 )
{
    t_CacheTriList* pTriList;

    //---   get a ptr to the cache frame tris
    ASSERT( s_NFrames>0 );
    pTriList = &s_pFrame[s_NFrames-1].Tri;

    //---   add the tri to the cache frame
    pTriList->TriID[ pTriList->NTris ] = TriID;
    pTriList->CacheID[(pTriList->NTris * 3) + 0] = CacheID0;
    pTriList->CacheID[(pTriList->NTris * 3) + 1] = CacheID1;
    pTriList->CacheID[(pTriList->NTris * 3) + 2] = CacheID2;

    //---   increment our counter
    pTriList->NTris++;
    ASSERT( pTriList->NTris <= CACHE_NTRIS );
}

//==========================================================================

void QPS2RipSkin::FindBestNextTri( s32& rBestTri, f32& rBestError, s32 StartTri, s32 NTris )
{
    s32 i, j;

    //---   Find next tri with least error
    rBestTri    = -1;
    rBestError  = 10000000.0f;
  
    //---   Search through tris connected to cache
    for( i = 0; i < CACHE_NVERTS; i++ )
    {
        if( s_Cache.Vert[i].Used )
        {
            s32 VertID = s_Cache.Vert[i].VertID;
            s32 TriID = m_pVert[VertID].FirstTri;

            //---   loop through the linked list of tris using this vert
            while( TriID != -1 )
            {
                //---   has this tri already been taken care of?
                if( (!m_pTri[TriID].Used) && 
                    (!m_pTri[TriID].Rendered) && 
                    (!m_pTri[TriID].Satisfied) )
                {
                    f32     Error=0;
                    xbool   DoTopologyCheck = FALSE;

                    //---   Get error for this tri
                    for( j = 0; j < 3; j++ )
                    {
                        t_RipVert* pV = &m_pVert[m_pTri[TriID].VertID[j]];

                        //---   Check if we need to use a new transform
                        if( m_pTransform[ pV->TransformID ].Used == FALSE )
                            Error += TRANSFORM_PENALTY;

                        //---   Check if we need to load vert into cache
                        if( pV->CacheID == -1 )
                            Error += VERT_LOAD_PENALTY;
                                                                            
                        //---   Check if this is the last tris for a vert
                        if( pV->NTrisDepending == 1 )
                        {
                            Error -= LAST_1_TRI_BONUS;
                            DoTopologyCheck = TRUE;
                        }

                        //---   Check if this is the next to last tri for a vert
                        if( pV->NTrisDepending == 2 )
                        {
                            Error -= LAST_2_TRI_BONUS;
                            DoTopologyCheck = TRUE;
                        }

                        //---   Check if this is one of the last tris for a vert
                        if( pV->NTrisDepending == 3 )
                            Error -= LAST_3_TRI_BONUS;

                        if( pV->NTrisDepending == 4 )
                            Error -= LAST_4_TRI_BONUS;
                    }

                    //---   Apply bonus for being connected to tris already satisfied by cache
                    if( DoTopologyCheck )
                    {
                        //---   apply a bonus for connecting to tris that have been satisfied,
                        //      but not yet rendered
                        for( j = 0; j < 3; j++ )
                        {
                            s32 NeighborID = m_pTri[TriID].NeighborTriID[j];

                            if( NeighborID != -1 )
                            {
                                if( (m_pTri[NeighborID].Satisfied) && (!m_pTri[NeighborID].Rendered) )
                                {
                                    Error -= NEIGHBOR_ADJACENT_BONUS;
                                }
                            }
                        }

                        //---   Apply bonuses if this tri would bring in other tris
                        for( j = 0; j < 3; j++ )
                        {
                            s32 NeighborID = m_pTri[TriID].NeighborTriID[j];

                            if( !m_pTri[NeighborID].Satisfied )
                            {
                                s32 NMatches = 0;
                                s32 k;
                                s32 NeighborVertID;

                                for( k = 0; k < 3; k++ )
                                {
                                    NeighborVertID = m_pTri[NeighborID].VertID[k];
                                    if( m_pVert[NeighborVertID].CacheID != -1 )
                                    {
                                        //---   neighbor's vert is in cache already
                                        NMatches++;
                                    }
                                    else
                                    {
                                        //---   is the neighbor's vert part of this tri?
                                        if( m_pTri[TriID].VertID[0] == NeighborVertID )
                                            NMatches++;
                                        else
                                        if( m_pTri[TriID].VertID[1] == NeighborVertID )
                                            NMatches++;
                                        else
                                        if( m_pTri[TriID].VertID[2] == NeighborVertID )
                                            NMatches++;
                                    }
                                }

                                //---   we would have brought in a tri
                                if( NMatches==3 )
                                {
                                    Error -= NEIGHBOR_SATISFIED_BONUS;
                                }
                            }
                        }
                    }

                    //---   See if this tri is the best so far
                    if( Error < rBestError )
                    {
                        if ( (TriID < StartTri) ||
                             (TriID >= (StartTri+NTris)) )
                        {
                            ASSERT( FALSE );
                        }
                        rBestError = Error;
                        rBestTri = TriID;
                    }
                }

                //---   Move to next tri in the linked list
                if( m_pTri[TriID].VertID[0] == VertID )
                    TriID = m_pTri[TriID].VertNextTriID[0];     
                else
                if( m_pTri[TriID].VertID[1] == VertID )
                    TriID = m_pTri[TriID].VertNextTriID[1];
                else
                if( m_pTri[TriID].VertID[2] == VertID )
                    TriID = m_pTri[TriID].VertNextTriID[2];
            }
        }
    }

    //---   Check if this tri is satisfactory
    if( rBestError <= VERT_LOAD_PENALTY )
    {
        return;
    }

    //---   None of the tris to this point have been good enough, look through
    //      the entire set of tris now...
    for( i = 0; i < NTris; i++ )
    {
        //---   Just search through the tris that haven't been touched yet
        if( (!m_pTri[StartTri + i].Used) &&
            (!m_pTri[StartTri + i].Rendered) &&
            (!m_pTri[StartTri + i].Satisfied) )
        {
            f32 Error = 0;

            for( j = 0; j < 3; j++ )
            {
                t_RipVert* pV = &m_pVert[m_pTri[StartTri + i].VertID[j]];

                //---   Check if we need to use a new transform
                if( m_pTransform[ pV->TransformID ].Used == FALSE )
                    Error += TRANSFORM_PENALTY;

                //---   Check if we need to load vert into cache
                if( pV->CacheID == -1 )
                    Error += VERT_LOAD_PENALTY;

                //---   Check if this is one of the last tris for any verts
                if( pV->NTrisDepending == 1 )
                    Error -= LAST_1_TRI_BONUS;

                if( pV->NTrisDepending == 2 )
                    Error -= LAST_2_TRI_BONUS;

                if( pV->NTrisDepending == 3 )
                    Error -= LAST_3_TRI_BONUS;

                if( pV->NTrisDepending == 4 )
                    Error -= LAST_4_TRI_BONUS;
            }

            //---   See if this tri is best
            if( Error < rBestError )
            {
                rBestError = Error;
                rBestTri = StartTri + i;
            }
        }
    }
}

