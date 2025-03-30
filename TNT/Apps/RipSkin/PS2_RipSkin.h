///////////////////////////////////////////////////////////////////////////
//  RipSkin.h
///////////////////////////////////////////////////////////////////////////

#ifndef __PS2_RIPSKIN_H_INCLUDED__
#define __PS2_RIPSKIN_H_INCLUDED__

#include "x_math.hpp"
#include "x_color.hpp"

#include "RipSkin.h"


////////////////////////////////////////////////////////////////////////////
// TYPEDEFS
////////////////////////////////////////////////////////////////////////////

typedef struct SRipTransform
{
    s32                 NMatrices;      // # of matrices in this transform
    s32                 MatrixID[8];    // matrices used by this transform
    s32                 NVerts;         // # of verts which use this transform

    xbool               Used;           // whether or not this transform has been used during the current cache frame
} t_RipTransform;

typedef struct SRipTri
{
    s32                 VertID[3];          // indices into m_pVert
    s32                 NeighborTriID[3];   // neighboring triangles
    s32                 MeshID;             // which mesh this tri belongs to
    s32                 TextureID;          // which texture this tri uses
    f32                 Area;               // area of triangle
    vector3             Normal;             // triangle normal
    s32                 VertNextTriID[3];   // next tri in the vert linked lists
    xbool               Used;               // whether or not this tri was used in determining a cache frame path
    xbool               Rendered;           // whether or not the tri has been rendered by a previous cache frame
    xbool               Satisfied;          // whether the tri has been satisfied by verts in the cache frame
} t_RipTri;

typedef struct SRipTexture
{
    char                Name[X_MAX_PATH];
    s32                 Width;
    s32                 Height;
} t_RipTexture;

typedef struct SRipSubMesh
{
    xbool               EnvPass;            // is this the env. map pass?
    xbool               Shadow;             // is this a shadow?
    xbool               Alpha;              // should this sub-mesh support alpha
    xbool               Morph;              // does this submesh contain morph deltas?
    s32                 TextureID;          // texture that this submesh uses
    s32                 TriID;              // first tri of the submesh
    s32                 NTris;              // # of tris in submesh
    s32                 MeshID;             // Mesh that this submesh belongs to

    s32                 NVertsLoaded;       // # of verts that have to be loaded to the cache (some verts may have to be loaded more than once)
    s32                 NTriFlushes;        // # of times tris get flushed from cache
    s32                 NTransforms;        // # of transform switches

    s32                 CacheFrameID;       // first cache frame used by this submesh
    s32                 NCacheFrames;       // # of cache frames used by this submesh
} t_RipSubMesh;

typedef struct SRipMesh
{
    char                Name[64];           // length of SKIN_MESH_NAME_LENGTH
    xbool               EnvMapped;          // mesh is environment mapped
    xbool               Shadow;             // is this a shadow?
    xbool               Alpha;              // should this mesh support alpha?
    s32                 SubmeshID;          // first submesh of the mesh
    s32                 NSubmeshes;         // # of submeshes in this mesh
    s32                 VertID;             // first vert in this mesh
    s32                 NVerts;             // # of verts in this mesh
    s32                 TriID;              // first tri in this mesh
    s32                 NTris;              // # of tris in this mesh
    s32                 MorphTargetID;      // first morph target in mesh
    s32                 NMorphTargets;      // # of morph targets in mesh
} t_RipMesh;

typedef struct SRipMorphDelta
{
    s32     VertID;             // corresponding vert in mesh
    vector3 Delta;              // the delta value
} t_RipMorphDelta;

typedef struct SRipMorphTarget
{
    char                Name[64];
    s32                 DeltaID;            // first morph delta
    s32                 NDeltas;            // # of morph deltas
} t_RipMorphTarget;

typedef struct SRipBone
{
    char                Name[32];
} t_RipBone;

////////////////////////////////////////////////////////////////////////////
// The QRipSkin class
////////////////////////////////////////////////////////////////////////////

class QPS2RipSkin : public QRipSkin
{
public:
    ////////////////////////////////////////////////////////////////////////
    // Constructor/Destructor
    ////////////////////////////////////////////////////////////////////////
             QPS2RipSkin   ( void );
    virtual ~QPS2RipSkin   ( void );

    ////////////////////////////////////////////////////////////////////////
    // Functions for the user
    ////////////////////////////////////////////////////////////////////////
    void Clear              ( void );
    void SetSkinName        ( char* Name );
    void AddBone            ( char* Name );
    void NewTexture         ( char* TexturePath, s32 Width, s32 Height );
    void NewMesh            ( char* MeshName, xbool EnvMapped, xbool Shadow, xbool Alpha );
    void AddVert            ( t_RipVert& rRipVert );
    void AddTri             ( s32   Vert1,
                              s32   Vert2,
                              s32   Vert3,
                              char* pTextureName );
    void NewMorphTarget     ( char* TargetName );
    void AddMorphDelta      ( s32 VertID, vector3 Delta );
    void ForceSingleMatrix  ( void );
    void ForceDoubleMatrix  ( void );
    void ForceTripleMatrix  ( void );
    void PrepareStructures  ( void );
    void DisplayStats       ( void );
    void Save               ( char* pFileName );

    ////////////////////////////////////////////////////////////////////////
    //  Member data (all public for simplicity)
    ////////////////////////////////////////////////////////////////////////
    char                m_Name[32];
    t_RipVert*          m_pVert;
    t_RipTri*           m_pTri;
    t_RipSubMesh*       m_pSubMesh;
    t_RipMesh*          m_pMesh;
    t_RipTexture*       m_pTexture;
    t_RipTransform*     m_pTransform;
    t_RipBone*          m_pBone;
    t_RipMorphTarget*   m_pMorphTarget;
    t_RipMorphDelta*    m_pMorphDelta;

    s32             m_NVerts;
    s32             m_NTris;
    s32             m_NSubmeshes;
    s32             m_NMeshes;
    s32             m_NTextures;
    s32             m_NTransforms;
    s32             m_NBones;
    s32             m_NMorphTargets;
    s32             m_NMorphDeltas;

    s32             m_NVertsAllocated;
    s32             m_NTrisAllocated;
    s32             m_NSubmeshesAllocated;
    s32             m_NMeshesAllocated;
    s32             m_NTexturesAllocated;
    s32             m_NTransformsAllocated;
    s32             m_NBonesAllocated;
    s32             m_NMorphTargetsAllocated;
    s32             m_NMorphDeltasAllocated;

private:
    ////////////////////////////////////////////////////////////////////////
    // Internal routines
    ////////////////////////////////////////////////////////////////////////
    void    ForceSingleMatrix           ( s32 VertID );
    void    ForceDoubleMatrix           ( s32 VertID );
    void    ForceTripleMatrix           ( s32 VertID );

    void    ComputeNormals              ( void );
    void    BlendVertexNormals          ( void );
    void    BuildTransforms             ( void );
    void    ConnectTrisToVerts          ( void );
    void    ConnectTrisToTris           ( void );
    void    SolveSubMesh                ( s32 SubMeshID );
    void    SolveTransformPath          ( s32 SubMeshID,
                                          s32 StartTri,
                                          s32 EndTri );
    f32     AddTriToCache               ( s32 TriID );
    s32     FlushTrisFromCache          ( void );
    s32     UpdateVertDependencies      ( s32 VertID );
    s32     FlushUnusedVertsFromCache   ( void );
    s32     FindCacheEntries            ( s32 TriID, s32* pCacheID );
    void    FindBestNextTri             ( s32& rBestTri,
                                          f32& rBestError,
                                          s32 StartTri,
                                          s32 NTris );
    void    NewCacheFrame               ( void );
    void    EndCacheFrame               ( void );
    void    AddTriToCacheFrame          ( s32 TriID,
                                          s32 CacheID0,
                                          s32 CacheID1,
                                          s32 CacheID2 );
    void    AddVertToCacheFrame         ( s32 VertID, s32 CacheID );
    s32     ExportFrameTransforms       ( byte* pDList,
                                          s32 FrameID,
                                          xbool EnvPass,
                                          xbool Shadow,
                                          xbool Morph,
                                          f32 MinY,
                                          f32 MaxY );
    s32     ExportFramePacks            ( byte* pDList,
                                          s32 FrameID,
                                          xbool EnvPass,
                                          xbool Shadow,
                                          xbool Alpha );
    s32     ExportSubMesh               ( byte* pDList,
                                          s32 SubMeshID );
    xbool   IsEnvPass                   ( s32 SubMeshID );
    xbool   IsShadow                    ( s32 SubMeshID );
    xbool   IsAlpha                     ( s32 SubMeshID );
    xbool   IsMorph                     ( s32 SubMeshID );
};

#endif // __PS2_RIPSKIN_H_INCLUDED__
