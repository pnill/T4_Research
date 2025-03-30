////////////////////////////////////////////////////////////////////////////
//
// GameCube_RipSkin.h
//
////////////////////////////////////////////////////////////////////////////

#ifndef GAMECUBE_RIPSKIN_H_INCLUDED
#define GAMECUBE_RIPSKIN_H_INCLUDED


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "RipSkin.h"


////////////////////////////////////////////////////////////////////////////
// STRUCTURES
////////////////////////////////////////////////////////////////////////////

struct GCSkinTri
{
    s32                 VertID[3];          // vertex indices for tri
    s16                 MeshID;             // mesh this tri belongs to
    s16                 TextureID;          // texture this tri uses
    f32                 Area;               // area of triangle
    vector3             Normal;             // triangle normal
};

struct GCSkinTexture
{
    char                Name[X_MAX_PATH];   // texture name
    s16                 Width;              // width of texture
    s16                 Height;             // height of texture
};

struct GCSkinMorphDelta
{
    s32                 VertID;             // corresponding vert in mesh
    vector3             Delta;              // the delta value
};

struct GCRipSkinSubMesh
{
    s8                  bEnvPass;           // is submesh env. mapped?
    s8                  bShadow;            // is submesh a shadow?
    s8                  bAlpha;             // is submesh alpha-blended?
    s8                  bMorph;             // does submesh contain morph deltas?
    s16                 ParentMesh;         // Mesh this submesh belongs to
    s16                 TextureID;          // texture this submesh uses
    s32                 FirstTriID;         // first tri of the submesh
    s32                 NTris;

    s32                 NTrisGP;
    s32                 NVertsGP;
    GCSkinTri*          pTriListGP;
    SRipVert*           pVertListGP;

    s32                 NTrisCPU;
    s32                 NVertsCPU1;
    s32                 NVertsCPU2;
    s32                 NVertsCPU3;
    GCSkinTri*          pTriListCPU;
    SRipVert*           pVertListCPU;

    s32                 NDeltasCPU;
    GCSkinMorphDelta*   pDeltaListCPU;

    u32                 SizeDListGP;
    u32                 SizeDListCPU;
    u32                 OffsetDListGP;
    u32                 OffsetDListCPU;
    u32                 OffsetVtxGP;
    u32                 OffsetVtxCPU;
    u32                 OffsetVtxShadow;
    u32                 OffsetUVs;
    u32                 OffsetDeltasCPU;

    f32                 AvgVtxPerTriGP;
    f32                 AvgVtxPerTriCPU;
    s32                 NTriStripsGP;
    s32                 NTriStripsCPU;
    s32                 NMtxGroups;
    s32                 NMtxSlots;
    s32                 NMtxLoads;
};

struct GCRipSkinMesh
{
    char                Name[64];           // mesh name, length of SKIN_MESH_NAME_LENGTH
    xbool               bEnvMapped;         // is mesh env. mapped?
    xbool               bShadow;            // is mesh a shadow?
    xbool               bAlpha;             // is mesh alpha-blended?
    s32                 FirstSubmesh;       // first submesh of the mesh
    s32                 NSubmeshes;         // # of submeshes in this mesh
    s32                 FirstVertID;        // first vert in this mesh
    s32                 NVerts;             // # of verts in this mesh
    s32                 FirstTriID;         // first tri in this mesh
    s32                 NTris;              // # of tris in this mesh
    s32                 FirstMorphTgtID;    // first morph target in mesh
    s32                 NMorphTargets;      // # of morph targets in mesh
};

struct GCRipSkinMorphTarget
{
    char                Name[64];           // morph target name
    s32                 FirstDeltaID;       // first morph delta
    s32                 NDeltas;            // # of morph deltas
};

struct GCSkinBone
{
    char                Name[32];           // bone name
};


////////////////////////////////////////////////////////////////////////////
// QGameCubeRipSkin Class
////////////////////////////////////////////////////////////////////////////

class QGameCubeRipSkin : public QRipSkin
{
public:
    ////////////////////////////////////////////////////////////////////////
    // Constructor/Destructor
    ////////////////////////////////////////////////////////////////////////

     QGameCubeRipSkin( void );
    ~QGameCubeRipSkin( void );

    ////////////////////////////////////////////////////////////////////////
    // Functions for the user
    ////////////////////////////////////////////////////////////////////////

    void    Clear               ( void );
    void    SetSkinName         ( char* pName );
    void    AddBone             ( char* pName );
    void    NewTexture          ( char* pTexturePath, s32 Width, s32 Height );
    void    NewMesh             ( char* pMeshName, xbool EnvMapped, xbool Shadow, xbool Alpha );
    void    AddVert             ( t_RipVert& rRipVert );
    void    AddTri              ( s32 Vert0, s32 Vert1, s32 Vert2, char* pTextureName );
    void    NewMorphTarget      ( char* pTargetName );
    void    AddMorphDelta       ( s32 VertID, vector3 Delta );
    void    ForceSingleMatrix   ( void );
    void    ForceDoubleMatrix   ( void );
    void    ForceTripleMatrix   ( void );
    void    PrepareStructures   ( void );
    void    DisplayStats        ( void );
    void    Save                ( char* pFileName );


    ////////////////////////////////////////////////////////////////////////
    //  Data members
    ////////////////////////////////////////////////////////////////////////
    char                  m_Name[32];
    t_RipVert*            m_pVert;
    GCSkinTri*            m_pTri;
    GCSkinTexture*        m_pTexture;
    GCRipSkinSubMesh*     m_pSubMesh;
    GCRipSkinMesh*        m_pMesh;
    GCSkinMorphDelta*     m_pMorphDelta;
    GCRipSkinMorphTarget* m_pMorphTarget;
    GCSkinBone*           m_pBone;
    byte*                 m_pVtxData;
    byte*                 m_pDListData;
    byte*                 m_pDeltaData;

    s32                   m_NVerts;
    s32                   m_NTris;
    s32                   m_NTextures;
    s32                   m_NSubmeshes;
    s32                   m_NMeshes;
    s32                   m_NMorphDeltas;
    s32                   m_NMorphTargets;
    s32                   m_NBones;
    u32                   m_VtxDataSize;
    u32                   m_DListDataSize;
    u32                   m_DeltaDataSize;

    s32                   m_NVertsAllocated;
    s32                   m_NTrisAllocated;
    s32                   m_NTexturesAllocated;
    s32                   m_NSubmeshesAllocated;
    s32                   m_NMeshesAllocated;
    s32                   m_NMorphDeltasAllocated;
    s32                   m_NMorphTargetsAllocated;
    s32                   m_NBonesAllocated;
    u32                   m_VtxDataSizeAllocated;
    u32                   m_DListDataSizeAllocated;
    u32                   m_DeltaDataSizeAllocated;

protected:
    ////////////////////////////////////////////////////////////////////////
    // Internal routines
    ////////////////////////////////////////////////////////////////////////
    void    ForceSingleMatrix   ( s32 VertID );
    void    ForceDoubleMatrix   ( s32 VertID );
    void    ForceTripleMatrix   ( s32 VertID );

    void    ComputeNormals      ( void );
    void    BlendVertexNormals  ( void );

    void    BuildSubMeshData    ( s32 SubMeshID );

    // Helpers for BuildSubMeshData
    void    BuildSubMeshVtxData  ( GCRipSkinSubMesh* pSubMesh );
    void    BuildSubMeshDListData( GCRipSkinSubMesh* pSubMesh );
    void    BuildSubMeshDeltaData( GCRipSkinSubMesh* pSubMesh );

    // Helpers for BuildSubMeshVtxData
    void    BuildSubMeshVtxDataShadow( GCRipSkinSubMesh* pSubMesh );
    void    BuildSubMeshVtxDataUV    ( GCRipSkinSubMesh* pSubMesh );
    void    BuildSubMeshVtxDataGP    ( GCRipSkinSubMesh* pSubMesh );
    void    BuildSubMeshVtxDataCPU   ( GCRipSkinSubMesh* pSubMesh );

    // Helpers for BuildSubMeshDListData
    void    BuildSubMeshDListDataShadow( GCRipSkinSubMesh* pSubMesh );
    void    BuildSubMeshDListDataGP    ( GCRipSkinSubMesh* pSubMesh );
    void    BuildSubMeshDListDataCPU   ( GCRipSkinSubMesh* pSubMesh );
    void    PadDListData               ( GCRipSkinSubMesh* pSubMesh, xbool GPDList );

    xbool   IsEnvPass( s32 SubMeshID );
    xbool   IsShadow ( s32 SubMeshID );
    xbool   IsAlpha  ( s32 SubMeshID );
    xbool   IsMorph  ( s32 SubMeshID );
};


////////////////////////////////////////////////////////////////////////////

#endif // GAMECUBE_RIPSKIN_H_INCLUDED