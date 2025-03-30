#ifndef __PC_SKIN_HPP_INCLUDED__
#define __PC_SKIN_HPP_INCLUDED__

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Skin File Format
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// HEADER INFO
//  PCSkinFileHeader
//
// MESH DATA - For each mesh.
//  PCSkinMeshHeader
//  VertexData        - PCSkinD3DVert * Number Of Verts
//  Indices Data      - (s16) * Number of Indices
//
//  MORPH TARGETS ARE NOT EXPORTED TO PC AS OF YET, SO THEY ARE NOT BEING SAVED OUT EITHER!!!
//
// SUBMESH DATA - For Each Submesh.
//  PCSkinSubmesh
//
// TEXTURE DATA - For each texture.
//  PCSkinTexture
//    
// SKELETON DATA - For each bone.
//  PCSkinBone
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Includes
//=====================================================================================================================================
#include "x_files.hpp"
#if defined( TARGET_PC )
    #include "windows.h"
    #include "d3d8.h"
    #include "d3d8types.h"
#endif

#include "CommonSkinDefines.hpp"


// PC Skin DEFINES
//=====================================================================================================================================
#define SKIN_MAX_NUM_SUBMESHES  1024
#define SKIN_MAX_NUM_MESHES     64
#define SKIN_MAX_NUM_VERTS      65535
#define SKIN_MAX_NUM_TRIS       65535
#define SKIN_MAX_NUM_INDICES    65535
#define SKIN_MAX_NUM_MTARGETS   64
#define SKIN_MAX_NUM_MDELTAS    65535

#define MAX_PC_MATRICES_PER_VERT  3



// DATA TYPES
//=====================================================================================================================================
struct PCSkinFileHeader
{
    char mName[SKIN_NAME_LENGTH];    // Name of Skin
    u32  mFlags;                     // SKIN_FLAG_xxx bits
    s32  mNBones;                    // Number of bones in the skeleton.
    s32  mNMeshes;                   // Number of Meshes
    s32  mNSubMeshes;                // Number of Submeshes
    s32  mNTextures;                 // Number of Textures
};


//-------------------------------------------------------------------------------------------------------------------------------------
struct PCSkinD3DVert
{
    vector3 mPos;                                       // Vert Pos
    f32     mBlendWeights[MAX_PC_MATRICES_PER_VERT];  // MAX_PC_MATRICES_PER_VERT Blending Weights.
    f32     mMatrixIDs[MAX_PC_MATRICES_PER_VERT];       // Matrix IDs in a simple to use format.
    vector3 mNormal;    // Vert Normal
//	color   mDiffuse;   // Vert Diffuse Color
	vector2 mUV;        // Vert UV
};
typedef PCSkinD3DVert t_SkinVert;


//-------------------------------------------------------------------------------------------------------------------------------------
struct PCSkinTexture
{
    char mName[X_MAX_PATH];
};
typedef PCSkinTexture t_SkinTexture;


//-------------------------------------------------------------------------------------------------------------------------------------
struct PCSkinBone
{
    char mName[SKIN_BONE_NAME_LENGTH];
};


//-------------------------------------------------------------------------------------------------------------------------------------
struct PCSkinMorphTarget
{
    char    Name[SKIN_MORPH_NAME_LENGTH];
    xbool   Active;
    f32     Weight;
    s32     FirstDeltaID;
    s32     NDeltas;
};
typedef PCSkinMorphTarget t_SkinMorphTarget;



//-------------------------------------------------------------------------------------------------------------------------------------
struct PCSkinMorphDelta
{
    s32     mVertID; // corresponding vert in mesh
    vector3 mDelta;  // the delta value
};
typedef PCSkinMorphDelta t_SkinMorphDelta;


//-------------------------------------------------------------------------------------------------------------------------------------
struct PCSkinSubmesh
{
    u32 Flags;         // Submesh flags.
    s32 TextureID;     // ID that represents which texture is used to render this submesh.

    u32 mPrimType;      // Type of Primitive
    s32 mNPrims;        // Number of polygons used to build this Submesh.

    s32 mVertexMin;     // Lowest value in the meshes vertex buffer.
    s32 mVertexCount;   // Size of the vertex buffer that must be locked in order to render this submesh.
    s32 mIndexStart;    // Offset into the meshes indices buffer where this submeshes polygon data starts.

    xbool LookedUp;
    s8  *mIndexLookup;
};
typedef PCSkinSubmesh t_SkinSubMesh;


//-------------------------------------------------------------------------------------------------------------------------------------
struct PCSkinMeshHeader
{
    char mName[SKIN_MESH_NAME_LENGTH];
    u32  mFlags;
    f32  mRadius;

    s32  mFirstSubMesh;
    s32  mNSubMeshes;

    u32  mVertexFormat;
    s32  mNVerts;
    s32  mVertexDataSize;

    s32  mNIndices;
    s32  mIndexDataSize;
};


//-------------------------------------------------------------------------------------------------------------------------------------
struct PCSkinMesh
{
    PCSkinMesh( void ) { pVertexBuffer = NULL; pIndexBuffer = NULL; pMorphTarget = NULL; pMorphDelta = NULL; }

    char                    Name[SKIN_MESH_NAME_LENGTH];   // Name of the Mesh in the skin.
    u32                     Flags;                         // SKIN_MESHFLAG_xxx bits
    f32                     Radius;                        // Radius of this mesh.

    s32                     FirstSubMesh;                  // First submesh from complete array in the skin
    s32                     NSubMeshes;                    // Number of submeshes for this mesh

    u32                     VertexFormat;                  // Vertex format for D3D verts.
    s32                     NVerts;                        // Number of Verts
    LPDIRECT3DVERTEXBUFFER8 pVertexBuffer;                 // Pointer to D3D Verts

    s32                     NIndices;                      // Number of elements in the indices array.
    LPDIRECT3DINDEXBUFFER8  pIndexBuffer;                  // The index buffer.

    s32                     NMorphTargets;
    t_SkinMorphTarget*      pMorphTarget;

    s32                     NMorphDeltas;
    t_SkinMorphDelta*       pMorphDelta;

    s32                     FirstMorphTarget;
};
typedef PCSkinMesh t_SkinMesh;



//-------------------------------------------------------------------------------------------------------------------------------------
typedef struct SPCMatrixGroup
{
    matrix4 BoneMatrices[SKIN_MAX_MATRICES];
    matrix4 LightDirMatrix;
    matrix4 LightColMatrix;
} t_MatrixGroup;


//-------------------------------------------------------------------------------------------------------------------------------------
typedef struct PCSubMeshGroup
{
    s32             Flags;
    s32             MeshID;
    t_SkinMesh*     pMesh;
    t_SkinSubMesh*  pSubmesh;

    s32             NBones;
    t_MatrixGroup*  pMatrices;

    matrix4*        mpShadowMatrices;
    matrix4*        pShadowMtx;
    s32             NumShadows;
    u8*             pAlphaLevels;

    x_bitmap*       pBitmap;
    x_bitmap*       pBitmap2;
    u32             UserData1;
    u32             UserData2;
    u32             UserData3;
} t_SubMeshGroup;

#endif // __PC_SKIN_HPP_INCLUDED__