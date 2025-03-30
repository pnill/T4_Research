////////////////////////////////////////////////////////////////////////////
//
// GC_Skin.hpp
//
////////////////////////////////////////////////////////////////////////////

#ifndef GC_SKIN_HPP_INCLUDED
#define GC_SKIN_HPP_INCLUDED


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "CommonSkinDefines.hpp"


////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////

#define GC_FRAC_POS             8           // fraction bits for positions
#define GC_FRAC_NRM             14          // fraction bits for normals(MUST be 14 on 16-bit normal)
#define GC_FRAC_UV              12          // fraction bits for tex. coord.
#define GC_FRAC_WEIGHT          8           // fraction bits for vtx weights
#define GC_FRAC_DELTA           10          // fraction bits for morph deltas

#define GC_INVALID_PTR_OFFSET   0xFFFFFFFF  // submesh pointer value stored in file, indicates non-used member


////////////////////////////////////////////////////////////////////////////
// TYPEDEFS
////////////////////////////////////////////////////////////////////////////

typedef struct GCSkinMesh        t_SkinMesh;
typedef struct GCSkinSubMesh     t_SkinSubMesh;
typedef struct GCSkinMorphTgt    t_SkinMorphTarget;
typedef struct GCMtxGroup        t_MatrixGroup;
typedef struct GCSubMeshGroup    t_SubMeshGroup;
typedef struct GCSkinDelta       t_SkinDeltaValue;
typedef struct GCSkinFileHeader  t_SkinFileHeader;


////////////////////////////////////////////////////////////////////////////
// STRUCTURES
////////////////////////////////////////////////////////////////////////////

struct GCSkinVert_PN
{
    s16     PX;                             // position X
    s16     PY;                             // position Y
    s16     PZ;                             // position Z
    s16     NX;                             // normal X
    s16     NY;                             // normal Y
    s16     NZ;                             // normal Z
};

struct GCSkinVert_CPU
{
    s16     PX;                             // position X
    s16     PY;                             // position Y
    s16     PZ;                             // position Z
    s16     NZ;                             // normal Z
    s16     NX;                             // normal X
    s16     NY;                             // normal Y
    u8      W0;                             // weight value for Mtx0
    u8      W1;                             // weight value for Mtx1, weight for M2 is calc'd from (1.0 - W0 - W1)
    u16     MtxIdx;                         // matrix indices(0.5.5.5 format, [0][M2][M1][M0])
};

struct GCSkinVert_UV
{
    s16     U;                              // texture coord. U
    s16     V;                              // texture coord. V
};

struct GCSkinVert_P
{
    s16     PX;                             // position X
    s16     PY;                             // position Y
    s16     PZ;                             // position Z
};

struct GCSkinDelta
{
    s16     XDelta;                         // morph delta X
    s16     YDelta;                         // morph delta Y
    s16     ZDelta;                         // morph delta Z
    u16     VertIdx;                        // vert index this delta belongs to
};

//--------------------------------------------------------------------------

typedef f32 GCPosMtx[3][4];
typedef f32 GCNrmMtx[3][3];

typedef GCPosMtx GCPosMtxArray[SKIN_MAX_MATRICES];
typedef GCNrmMtx GCNrmMtxArray[SKIN_MAX_MATRICES];


struct GCMtxGroup
{
    GCPosMtx        WT2V;                   // L2W Translation plus W2V transformation

    GCPosMtxArray   LAnim;                  // local animation transformation(no L2W translation or W2V in XForm)
    GCPosMtxArray   NLAnim;                 // LAnim minus the local anim translation(rotation only)
    GCPosMtxArray   L2V;                    // full local-to-view matrix
    GCNrmMtxArray   NL2V;                   // full L2V mtx(rotation only, for normals, separate from L2V for GXLoadNrmMtxIndx3x3)
};

//--------------------------------------------------------------------------

struct GCSkinMesh
{
    char    Name[SKIN_MESH_NAME_LENGTH];    // mesh's name
    u32     Flags;                          // flags
    u16     FirstSubMesh;                   // start index in Submesh array
    u16     NSubMeshes;                     // number of Submeshes used
    u16     FirstMorphTarget;               // start index in morph target array
    u16     NMorphTargets;                  // number of morph targets this mesh has
};

struct GCSkinSubMesh
{
    u32             Flags;                  // flags

    u32             DListSizeGP;            // size in bytes of disp list(for GP)
    u32             DListSizeCPU;           // size in bytes of disp list(for CPU)
    u16             NVertsGP;               // number of verts in GP vertex list
    u16             NVertsCPU1;             // number of verts(1 bone per vtx) in CPU vertex list
    u16             NVertsCPU2;             // number of verts(2 bones per vtx) in CPU vertex list
    u16             NVertsCPU3;             // number of verts(3 bones per vtx) in CPU vertex list
    void*           pDListGP;               // GP disp list(verts have 1 bone)
    void*           pDListCPU;              // CPU disp list(verts have 1,2,3 bones)
    GCSkinVert_PN*  pVtxGP;                 // ptr to GP verts
    GCSkinVert_CPU* pVtxCPU;                // ptr to CPU verts
    GCSkinVert_P*   pVtxShadow;             // ptr to shadow verts(if this submesh is a shadow)
    GCSkinVert_UV*  pUVs;                   // ptr to vert UVs

    GCSkinDelta*    pDeltasCPU;             // ptr to CPU delta values
    u16             NDeltasCPU;             // number of delta values(for CPU) used in submesh

    s16             TextureID;              // texture ID the submesh uses
};

struct GCSkinMorphTgt
{
    char    Name[SKIN_MORPH_NAME_LENGTH];   // Morph target name
    xbool   Active;                         // Is morph target active
    f32     Weight;                         // weight of morph target
};

//--------------------------------------------------------------------------

struct GCSubMeshGroup
{
    GCSkinMesh*     pMesh;                  // ptr to mesh info
    GCSkinSubMesh*  pSubMesh;               // ptr to submesh info
    GCSkinMorphTgt* pMorphTgts;             // ptr to morph target information

  union
  {
    GCMtxGroup*     pMatrices;              // ptr to matrix data
    GCPosMtxArray*  pShadowMatrices;        // ptr to matrix data with shadow matrix applied
  };

    x_bitmap*       pBitmap;                // main bitmap for submesh
    x_bitmap*       pBitmap2;               // secondary bitmap(for env. map)
    u32             Flags;                  // flags
    u32             UserData1;              // user parameter 1
    u32             UserData2;              // user parameter 2
    u32             UserData3;              // user parameter 3
    u32             MeshID;                 // mesh index for this group
};


////////////////////////////////////////////////////////////////////////////
//
// GameCube Skin file layout:
//
//      Header                  (GCSkinFileHeader)
//      Texture names           (array of strings [SKIN_TEXTURE_NAME_LENGTH] in size)
//      Bone names              (array of strings [SKIN_BONE_NAME_LENGTH] in size)
//      Mesh data               (array of GCSkinMesh)
//      Submesh data            (array of GCSkinSubMesh)
//      Morph target data       (array of GCSkinMorphTarget)
//      Delta value data        (array of GCSkinDelta)
//      Vertex data             (array of vertex data)
//      Display list data       (array of bytes)
//
////////////////////////////////////////////////////////////////////////////

struct GCSkinFileHeader
{
    char    Name[SKIN_NAME_LENGTH]; // skin's name
    u32     Flags;                  // flags for skin
    s32     NTextures;              // number of textures skin uses
    s32     NBones;                 // number of bones skin has
    s32     NMeshes;                // number of Meshes in skin
    s32     NSubMeshes;             // number of Submeshes in skin
    s32     NMorphTargets;          // number of Morph targets
    s32     NPadBytesDeltas;        // number of bytes used to pad vertex and DList data to 32B
    s32     NPadBytesVtxData;       // number of bytes used to pad vertex and DList data to 32B
    u32     DeltaDataSize;          // size in bytes of Morph target delta values
    u32     VertexDataSize;         // size in bytes of vertex data
    u32     DListDataSize;          // size in bytes of all the display list data
};


////////////////////////////////////////////////////////////////////////////

#endif // GC_SKIN_HPP_INCLUDED