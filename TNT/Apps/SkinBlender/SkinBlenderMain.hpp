#ifndef SKIN_BLENDER_MAIN_HPP
#define SKIN_BLENDER_MAIN_HPP

#include "x_files.hpp"
//=====================================================================================================================================

//=====================================================================================================================================
// Common Skin Defines
//=====================================================================================================================================
#define SKIN_NAME_LENGTH            32
#define SKIN_MESH_NAME_LENGTH       64



//=====================================================================================================================================
// XBOX Related
//=====================================================================================================================================
#define MAX_XBOX_MATRICES_PER_VERT  3

//-------------------------------------------------------------------------------------------------------------------------------------
struct XBOXSkinFileHeader
{
    char mName[SKIN_NAME_LENGTH];    // Name of Skin
    u32  mFlags;                     // SKIN_FLAG_xxx bits
    s32  mNBones;                    // Number of bones in the skeleton.
    s32  mNMeshes;                   // Number of Meshes
    s32  mNSubMeshes;                // Number of Submeshes
    s32  mNTextures;                 // Number of Textures
};


//=====================================================================================================================================
struct XBOXSkinSubmesh
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


//=====================================================================================================================================
struct XBOXSkinMeshHeader
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


//=====================================================================================================================================
struct XBOXSkinD3DVert
{
    vector3 mPos;                                       // Vert Pos
    f32     mBlendWeights[MAX_XBOX_MATRICES_PER_VERT];  // MAX_XBOX_MATRICES_PER_VERT Blending Weights.
    f32     mMatrixIDs[MAX_XBOX_MATRICES_PER_VERT];       // Matrix IDs in a simple to use format.
    vector3 mNormal;    // Vert Normal
	vector2 mUV;        // Vert UV
};


//=====================================================================================================================================
struct XBOXSkinInfo
{
    XBOXSkinInfo( void )
    { 
        mpFile          = NULL;
        mpFileHeader    = NULL;
        mpMeshHeader    = NULL;
        mpVerts         = NULL;
        mpRawData       = NULL;
        mSizeOfFile     = 0;
    }

    X_FILE*              mpFile;
    XBOXSkinFileHeader*  mpFileHeader;
    XBOXSkinMeshHeader** mpMeshHeader;
    XBOXSkinD3DVert**    mpVerts;

    byte*               mpRawData;
    s32                 mSizeOfFile;
};

#endif