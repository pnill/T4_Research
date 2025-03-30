#ifndef __PS2_SKIN_HPP_INCLUDED__
#define __PS2_SKIN_HPP_INCLUDED__

#include "CommonSkinDefines.hpp"

////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////

// VERY IMPORTANT NOTE: THESE OPCODES AND MEMORY LAYOUT DEFINES MUST MATCH
//                      WHAT IS IN THE MICROCODE
#define SKIN_MATRIX_CACHE_BASE      384
#define SKIN_INPUT_BUFFER_BASE      545
#define SKIN_INPUT_BUFFER_SIZE      172
#define SKIN_INPUT_BUFFER_MORPH_SIZE    70  // #### SET THIS UP CORRECTLY!!!!
#define SKIN_OUTPUT_BUFFER_SIZE     61
#define SKIN_SHADOW_MATRIX_BASE     1012
#define SKIN_COLOR_MATRIX_BASE      1016
#define SKIN_LIGHT_DIR_BASE         1020

#define SKIN_OPCODE_TRANSFORM           4
#define SKIN_OPCODE_FLUSH_TRIS          6
#define SKIN_OPCODE_FLUSH_SHADOW        8
#define SKIN_OPCODE_TRANSFORM_MORPH     10

#define SKIN_TRANSFORM_TYPE_SINGLE  0   // single-matrix transform
#define SKIN_TRANSFORM_TYPE_DOUBLE  2   // double-matrix transform
#define SKIN_TRANSFORM_TYPE_TRIPLE  4   // triple-matrix transform
#define SKIN_TRANSFORM_TYPE_ENVI    6   // single-matrix environment mapped
#define SKIN_TRANSFORM_TYPE_SHADOW  8   // single-matrix shadow transform

////////////////////////////////////////////////////////////////////////////
// TYPEDEFS
////////////////////////////////////////////////////////////////////////////

typedef struct SSkinMesh
{
    char    Name[SKIN_MESH_NAME_LENGTH];
    u32     Flags;
    s32     FirstSubMesh;
    s32     NSubMeshes;
    s32     FirstMorphTarget;
    s32     NMorphTargets;
    s32     FirstDeltaLocation; // Offset in m_pDeltaLocations
    s32     FirstDeltaValue;    // from start of m_pDeltaValues, the # of deltas should match NDeltaLocations * NTargets
    s32     NDeltaLocations;    // Number of delta locations to modify
    s32     DeltaDataOffset;    // Offset in m_pDeltaData (constant)
    s32     DeltaDataSize;      // size of delta data for this mesh
} t_SkinMesh;

typedef struct SSkinSubMesh
{
    u32     Flags;
    s32     TextureID;
    s32     NVerts;
    s32     NTris;
    s32     FirstCacheFrame;
    s32     NCacheFrames;
} t_SkinSubMesh;

typedef struct SSkinCacheFrame
{
    s32 NVertLoads;
    s32 FirstVertLoad;
    s32 FlushPacketOffset;
    s32 FlushPacketSize;
} t_SkinCacheFrame;

typedef struct SSkinVertLoad
{
    s32     VertPacketOffset;   // from start of m_pPacketData
    s32     VertPacketSize;     // for DMA
    s32     KickPacketOffset;   // from start of m_pPacketData
    s32     KickPacketSize;     // for DMA
    s32     DeltaPacketOffset;  // #### from start of pSubMeshGroup->pDeltaData
    s32     DeltaPacketSize;    // #### for DMA
} t_SkinVertLoad;

typedef struct SSkinMorphTarget
{
    char    Name[SKIN_MORPH_NAME_LENGTH];
    xbool   Active;
    f32     Weight;
} t_SkinMorphTarget;

typedef s32 t_SkinDeltaLoc;     // #### byte offset from pSubMeshGroup->pDeltaData
typedef struct SSkinDeltaValue
{
    s16     XDelta;             // 1.3.12 fixed point
    s16     YDelta;             // 1.3.12 fixed point
    s16     ZDelta;             // 1.3.12 fixed point
} t_SkinDeltaValue;

typedef struct SMatrixData
{
    matrix4     L2S;
    vector4     L2W[3];
} t_MatrixData;

typedef struct SMatrixGroup
{
    // IMPORTANT NOTE: Whatever you do to this structure, keep the matrices
    //                 16-byte aligned!!!
    u32             VIFLight[4];    // unpack info for light matrix
    matrix4         LightColor;
    matrix4			LightDir;
    u32             VIFMatrix[4];   // unpack info for bone matrices
    t_MatrixData    BoneMatrices[SKIN_MAX_MATRICES];
} t_MatrixGroup;

typedef struct SSubMeshGroup
{
    x_bitmap*           pBitmap;
    t_MatrixGroup*      pMatrices;
    matrix4*            pShadowMtx;
    u16                 NTris;
    u16                 NVerts;
    byte*               pPacketData;    // should match m_pPacketData
    byte*               pDeltaData;     //#### should match m_pMesh[MeshID].pCurrDeltaData
    s32                 NCacheFrames;
    t_SkinCacheFrame*   pCacheFrames;
    t_SkinVertLoad*     pVertLoads;
    s32                 Flags;
    s16                 NBones;
    s16                 MeshID;
    u32                 UserData1;
    u32                 UserData2;
    u32                 UserData3;
} t_SubMeshGroup;

////////////////////////////////////////////////////////////////////////////
// Private data for reading from a skin file
//
// A PS2 skin file is laid out like this:
//
//      Header          (t_SkinFileHeader)
//      Texture names   (array of texture names 32 chars/name)
//      Bone Name Data  (array of 32-byte strings)
//      Mesh Data       (array of t_SkinMesh)
//      SubMesh Data    (array of t_SkinSubMesh)
//      CacheFrame Data (array of t_SkinCacheFrame)
//      VertLoadData    (array of t_SkinVertLoad)
//      MorphTargetData (array of t_SkinMorphTarget)
//      DeltaLocData    (array of t_SkinDeltaLoc)
//      DeltaValueData  (array of t_SkinDeltaValue)
//      Packet Data     (array of bytes)
//      Delta Data      (array of bytes)
//
////////////////////////////////////////////////////////////////////////////

typedef struct SSkinFileHeader
{
    char    Name[32];       // Name of skin
    u32     Flags;          // SKIN_FLAG_xxx bits
    s32     NTextures;
    s32     NBones;
    s32     NMeshes;
    s32     NSubMeshes;
    s32     NCacheFrames;
    s32     NVertLoads;
    s32     NMorphTargets;
    s32     NDeltaLocations;
    s32     NDeltaValues;
    s32     PacketSize;
    s32     DeltaDataSize;
} t_SkinFileHeader;

#endif // __PS2_SKIN_HPP_INCLUDED__