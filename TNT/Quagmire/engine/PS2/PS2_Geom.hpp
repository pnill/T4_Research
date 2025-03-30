////////////////////////////////////////////////////////////////////////////
// Geom structures and defines specific to the PS2
////////////////////////////////////////////////////////////////////////////

#ifndef __PS2_GEOM_HPP_INCLUDED__
#define __PS2_GEOM_HPP_INCLUDED__

#include "x_files.hpp"
////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////

// PS2 OPCODES - VERY IMPORTANT!!!! - THESE OPCODES MUST MATCH WHAT IS IN
// THE GEOMETRY MICROCODE
#define GEOM_OPCODE_BOOT                    0
#define GEOM_OPCODE_LOAD_MATRICES           2
#define GEOM_OPCODE_LOAD_LIGHTS             4
#define GEOM_OPCODE_TRI_NOCLIP_NOLIGHT      6
#define GEOM_OPCODE_TRI_CLIP_NOLIGHT        8
#define GEOM_OPCODE_TRI_NOCLIP_LIGHT       10
#define GEOM_OPCODE_TRI_CLIP_LIGHT         12
#define GEOM_OPCODE_TRI_SHADOW             14
#define GEOM_OPCODE_TRI_ENVMAPPED          16
#define GEOM_OPCODE_LOAD_NL2V              18
#define GEOM_OPCODE_QUAD_CLIP_NOLIGHT      20

////////////////////////////////////////////////////////////////////////////
// Mesh & SubMesh typedefs
////////////////////////////////////////////////////////////////////////////

typedef struct SGeomSubMesh
{
    s32             MeshID;         // id of the mesh that this submesh belongs to

    s32             NVertsTrans;    // Stats for runtime collection
    s32             NTris;

    s32             TextureID;
    u32             Flags;          // GEOM_SUBMESH_FLAG_xxx bits

    // region data (for detecting when the object should be culled)
    vector3         Corner;                 // corner of the region
    vector3         Axis[3];                // region axis
    vector3         MidPoint;               // midpoint of the region

    s32             PacketOffset;   // For shipping to the VU
    s32             PacketSize;     // Size of packet to send

    f32             MinPixelSize;   // for calculating mips
    f32             MaxPixelSize;   // for calculating mips
    f32             AvgPixelSize;   // for calculating mips

//    s32             nPacketCount;           // number of VU packets
//    s32             *pPacketOffsets;        // which packets belong to this submesh
//    s32             *pPacketSizes;          // size of each packet
} t_GeomSubMesh;

typedef struct SGeomSubMesh_output
{
    s32             MeshID;         // id of the mesh that this submesh belongs to

    s32             NVertsTrans;    // Stats for runtime collection
    s32             NTris;

    s32             TextureID;
    u32             Flags;          // GEOM_SUBMESH_FLAG_xxx bits

    // region data (for detecting when the object should be culled)
    vector3         Corner;                 // corner of the region
    vector3         Axis[3];                // region axis
    vector3         MidPoint;               // midpoint of the region

//    s32             PacketOffset;   // For shipping to the VU
//    s32             PacketSize;     // Size of packet to send

    f32             MinPixelSize;   // for calculating mips
    f32             MaxPixelSize;   // for calculating mips
    f32             AvgPixelSize;   // for calculating mips
} t_GeomSubMesh_output;
//==========================================================================

typedef struct SGeomMesh
{
    char            Name[16];       // Name of mesh
    u32             Flags;          // GEOM_MESH_FLAG_xxx bits

    s32             FirstSubMesh;   // First submesh from complete list
    s32             NSubMeshes;     // Number of submeshes for this mesh

    f32             Radius;         // Radius of model

    s32             NVerts;         // # of verts in this mesh
    void**          pPositions;     // optional ptr to verts within packets
    void**          pUVs;           // optional ptr to UVs within packets
    void**          pRGBAs;         // optional ptr to RGBAs within packets
    void**          pNormals;       // optional ptr to Normals within packets

} t_GeomMesh;

////////////////////////////////////////////////////////////////////////////
// Private data for reading from a geometry file
//
// A PS2 geometry file is laid out like this:
//
//      Header          (t_GeomFileHeader)
//      Texture names   (array of texture names 32 chars/name)
//      Mesh Data       (array of t_GeomMesh)
//      SubMesh Data    (array of t_GeomSubMesh)
//      Packet Data     (array of bytes)
//      Texture Data    (use x_bitmap routines to save/load)
//      Pos locations   (optional list of Pos locations in packets)
//      UV locations    (optional list of UV locations in packets)
//      RGBA locations  (optional list of RGBA locations in packets)
//      Normal locations(optional list of Normal locations in packets)
//
////////////////////////////////////////////////////////////////////////////

typedef struct SGeomFileHeader
{
    char    Name[16];       // Name of Geom
    u32     Flags;          // GEOM_FLAG_xxx bits
    s32     NTextures;
    s32     NTexturesInFile;
    s32     NMeshes;
    s32     NSubMeshes;
    s32     PacketSize;
    s32     NPosPtrs;
    s32     NUVPtrs;
    s32     NRGBAPtrs;
    s32     NNormalPtrs;

} t_GeomFileHeader;

typedef struct SGeomFileHeader_Multipass
{
    char    Name[16];       // Name of Geom
    u32     Flags;          // GEOM_FLAG_xxx bits
    s32     NTextures;
    s32     NTexturesInFile;
    s32     NMaterials;
    s32     NMatStages;
    s32     NMeshes;
    s32     NSubMeshes;
    s32     PacketSize;
    s32     NPosPtrs;
    s32     NUVPtrs;
    s32     NRGBAPtrs;
    s32     NNormalPtrs;

} t_GeomFileHeader_Multipass;


#endif