#ifndef PC_RIPSKIN_H_INCLUDED
#define PC_RIPSKIN_H_INCLUDED

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Skin File Format
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// HEADER INFO
//  PCSkinFileHeader
//
// MESH DATA - For each mesh.
//  Name                    - SKIN_MESH_NAME_LENGTH
//  Flags                   - u32
//  Radius                  - f32
//  First SubMesh           - s32
//  Number Of SubMeshes     - s32
//  VertexFormat(D3D)       - u32
//  Number of Verts         - s32
//  VertexData              - PCSkinD3DVert * Number Of Verts
//  Number of Indices       - s32
//  Indices Data            - (s16) * Number of Indices
//
//  MORPH TARGETS ARE NOT EXPORTED TO PC AS OF YET, SO THEY ARE NOT BEING SAVED OUT EITHER!!!
//
// SUBMESH DATA - For Each Submesh.
//  PCSkinSubmesh
//
// TEXTUER DATA - For each texture.
//  PCSkinTexture
//    
// SKELETON DATA - For each bone.
//  PCSkinBone
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////


// INCLUDES
//=====================================================================================================================================
//#include "d3d8.h"
//#include "d3d8types.h"
#include "RipSkin.h"
#include "x_types.hpp"
#include "x_math.hpp"
#include "x_color.hpp"
#include "Strip.h"
#include "CommonSkinDefines.hpp"
#include "d3d8.h"
#include "d3d8types.h"
//#include "PC\PC_Skin.hpp"

#define MAX_PC_MATRICES_PER_VERT 3
//=====================================================================================================================================
// PC Rip Skin Defines
//=====================================================================================================================================
#define SKIN_MAX_INVALID_BONES  10

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


//-------------------------------------------------------------------------------------------------------------------------------------
struct PCSkinTexture
{
    char mName[X_MAX_PATH];
};


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



//-------------------------------------------------------------------------------------------------------------------------------------
struct PCSkinMorphDelta
{
    s32     mVertID; // corresponding vert in mesh
    vector3 mDelta;  // the delta value
};


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
    PCSkinMorphTarget*      pMorphTarget;

    s32                     NMorphDeltas;                      
    PCSkinMorphDelta*       pMorphDelta;                   

    s32                     FirstMorphTarget;              
};



//=====================================================================================================================================
// PC Rip Skin Types 
//=====================================================================================================================================
struct PCRipSkinTri
{
    s32     mIndex[3];      // Indices that make up the triangle.
    s32     mMaterialID;    // Texture applied to this triangle.
    f32     mArea;          // Area of the surface of the triangle.
    vector3 mNormal;        // Normal of the triangle.
};


//-------------------------------------------------------------------------------------------------------------------------------------
struct PCRipSkinMesh
{
    char                mName[SKIN_MESH_NAME_LENGTH];// Name of mesh
    u32                 mFlags;                      // SKIN_MESHFLAG_xxx bits
    f32                 mRadius;                     // Radius of the mesh.

    s32                 mFirstSubMesh;               // First submesh from complete array in the skin
    s32                 mNSubMeshes;                 // Number of submeshes for this mesh

    u32                 mVertexFormat;               // Vertex format for D3D verts.
    s32                 mNVerts;                     // Number of Verts
    PCSkinD3DVert*      mpD3DVerts;                  // Pointer to D3D Verts

    s32                 mNIndices;                   // Number of Indices
    s16*                mpIndices;                   // Pointer to Indices

    s32                 mNTris;                      // Number of Tris
    PCRipSkinTri*       mpTris;                      // Pointer to Tris

    ///////////////////////////////////////////////////////////////////////////
    //-- Morph Targets
    ///////////////////////////////////////////////////////////////////////////
    PCSkinMorphTarget*  mpMorphTarget;
    s32                 mNMorphTargets;
    
    PCSkinMorphDelta*   mpMorphDelta;
    s32                 mNMorphDeltas;
};



//-------------------------------------------------------------------------------------------------------------------------------------
class QPCRipSkin : public QRipSkin
{
    public:
         QPCRipSkin             ( void );
        ~QPCRipSkin             ( void );

        void Clear              ( void );
        void SetSkinName        ( char* Name );
        void AddBone            ( char* Name );
        void NewTexture         ( char* TexturePath, s32 Width, s32 Height );
        void NewMesh            ( char* MeshName, xbool EnvMapped, xbool Shadow, xbool Alpha );
        void AddVert            ( t_RipVert& rRipVert );
        void AddTri             ( s32 Vert0, s32 Vert1, s32 Vert2, char* pTextureName );
        void NewMorphTarget     ( char* TargetName );
        void AddMorphDelta      ( s32 VertID, vector3 Delta );
        void ForceSingleMatrix  ( void );
        void ForceDoubleMatrix  ( void );
        void ForceTripleMatrix  ( void );
        void PrepareStructures  ( void );
        void DisplayStats       ( void );
        void Save               ( char* pFileName );

    private:
        void ReWeightToSingleMatrix( PCSkinD3DVert* pCurrentVert );
        void ReWeightToDoubleMatrix( PCSkinD3DVert* pCurrentVert );
        void ReWeightToTripleMatrix( PCSkinD3DVert* pCurrentVert );
        void ReComputeNormals( void );
        void BlendVertexNormals( void );
        void DetermineTheTotalSubMeshCount( void );
        void SeparateMeshesIntoSubmeshes( void );
        void CalcalateMeshRadius( void );
        
        // File Saving helper functions.
        void BuildAndWriteSkinFileHeader( X_FILE* xfp );
        void WriteMeshData( X_FILE* pSkinFile );
        void WriteSubmeshData( X_FILE* pSkinFile );
        void WriteTextureData( X_FILE* pSkinFile );
        void WriteSkeletonData( X_FILE* pSkinFile );

        s32  mInvalidBones[SKIN_MAX_INVALID_BONES];

    protected:
        char                mSkinName[SKIN_NAME_LENGTH];    // Name of the skin.
        s32                 mActiveMesh;                    // Flag to indicate if the Skin should be used or not.

        // Geometry members
        s32                 mNMeshes;    // Number of Skin Meshes.
        PCRipSkinMesh*      mpMesh;      // Skin Meshes.

        s32                 mNSubMeshes; // Number of Submeshes in this skin.
        PCSkinSubmesh*      mpSubMesh;   // Skin Submeshes.

        s32                 mNTextures;  // Number of textures.
        PCSkinTexture*      mpTexture;   // Textures used by this SKIN.

        // Skeleton members.
        s32                 mNBones;     // Number of bones used.
        PCSkinBone*         mpBone;      // Bones for this Skeleton.
};


#endif // PC_RIPSKIN_H_INCLUDED