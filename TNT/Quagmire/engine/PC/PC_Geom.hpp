#ifndef __PC_GEOM_HPP_INCLUDED__
#define __PC_GEOM_HPP_INCLUDED__

#if defined( TARGET_PC )
//#include "d3d8.h"
#endif
#include "x_types.hpp"
#include "x_math.hpp"
#include "x_color.hpp"

//
// A PC GEOM file is laid out like this:
//
//      Header          (t_GeomFileHeader)
//
//      Geometry
//          Mesh Data       (array of t_GeomMesh)
//          Vert Data       (array of D3D Flexible Vertex Format(FVF) Verts.
//          Index Data      (array of D3D Indices (16-bit signed int))
//          SubMesh Data    (array of t_GeomSubMesh)
//
//      Textures
//          Texture names   (array of texture names 32 chars/name)
//          Texture BMP's   (array of texture xbitmaps
//
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// Mesh & SubMesh typedefs/structs/defines
////////////////////////////////////////////////////////////////////////////
#define MAX_MESH_NAME   16
#define MAX_STAGES      3

//-------------------------------------------------------------------------------------------------------------------------------------
struct SGeomFileHeader
{
    char    Name[MAX_MESH_NAME];    // Name of Geom
    u32     Flags;                  // GEOM_FLAG_xxx bits
    s32     NMeshes;                // Number of Meshes
    s32     NSubMeshes;             // Number of Submeshes
    s32     NTextures;              // Number of Textures
    s32     NTexturesInFile;        // Number of Textures in file

};


//-------------------------------------------------------------------------------------------------------------------------------------
struct SGeomMesh
{
    char            Name[MAX_MESH_NAME];    // Name of mesh
    u32             Flags;                  // GEOM_MESH_FLAG_xxx bits
    u32             PrivateFlags;           // PC Flags;

    s32             FirstSubMesh;           // First submesh from complete list   (Assuming this encompases all others)
    s32             NSubMeshes;             // Number of submeshes for this mesh
    s32             NVerts;                 // Number of Verts
    s32             NIndices;               // Number of Indices
    u32             VertUsage;              // Vert Usage
    u32             VertFormat;             // Vert D3D format
    u32             VertFormatSize;         // Vert D3D struct size

    f32             Radius;                 // Radius of model

    union
    {
        void*                   pVerts;     // Pointer to Verts
        IDirect3DVertexBuffer8* pD3DVerts;  
    };

    union
    {
        u16*                    pIndices;   // Pointer to Indices
        IDirect3DIndexBuffer8*  pD3DIndices;
    };
};


//-------------------------------------------------------------------------------------------------------------------------------------
struct SGeomSubMesh
{
    s32             MeshID;                 // meshid, so we know which one this belongs to

    s32             NVertsTrans;            // Stats for runtime collection
    s32             NTris;                  // Number of tris

    s32             TextureID[MAX_STAGES];  // Submesh Flags
    u32             Flags;                  // GEOM_SUBMESH_FLAG_xxx bits

    // region data (for detecting when the object should be clipped)
    vector3         Corner;                 // corner of the region
    vector3         Axis[3];                // region axis
    vector3         MidPoint;               // midpoint of the region

    f32             MinPixelSize;           // for calculating mips
    f32             MaxPixelSize;           // for calculating mips
    f32             AvgPixelSize;           // for calculating mips

    u32             PrimType;               // Type of Primitive

    s32             IndexMin;               // Min Value in Index
    s32             IndexCount;             // Index Count
    s32 			IndexStart;	            // Index Buffer
};


////////////////////////////////////////////////////////////////////////////
// Vertex formats for DirectX 8
////////////////////////////////////////////////////////////////////////////
// Legend of Vertex Types.
//
// XYZ   - 3D Location.
// TEXx  - Textured (x represents how many sets, 1 - 4)
// ARGB  - Diffuse color for lit vertex
// NORM  - Vertex normals are provided
#define VERTEX_FORMAT_XYZ_ARGB_TEX1 ( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 )
struct VertFormat_XYZ_ARGB_TEX1
{
	vector3  Pos;    // 12 bytes.
	u32      Color;  // 4  bytes. A << 24 | R << 16 | G << 8 | B
	vector2  UV;     // 8  bytes.
                     // 24 bytes total.
};

#define VERTEX_FORMAT_XYZ_NORM_TEX1 ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 )
struct VertFormat_XYZ_NORM_TEX1
{
	vector3 Pos;    // 12 Bytes.
	vector3 Normal; // 12 Bytes.
	vector2 UV;     // 8  Bytes.
                    // 32 Bytes Total.
};

#define VERTEX_FORMAT_XYZ_ARGB_NORM_TEX1_TEX2_TEX3 ( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_TEX2 | D3DFVF_TEX3 )
struct VertFormat_XYZ_ARGB_NORM_TEX1_TEX2_TEX3
{
	vector3 Pos;
    u32     Color;
	vector3 Normal; 
	vector2 UV1;    
	vector2 UV2;    
	vector2 UV3;    
                    
};

///////////////////////////////////////////////////////////////////////////////
//  Private Flags
///////////////////////////////////////////////////////////////////////////////
#define PRIVFLAG_MESH_AA        0x1
#define PRIVFLAG_MESH_ALPHA     0x2

////////////////////////////////////////////////////////////////////////////
// TYPEDEFS
////////////////////////////////////////////////////////////////////////////
typedef struct SGeomSubMesh     t_GeomSubMesh;
typedef struct SGeomMesh        t_GeomMesh;
typedef struct SGeomFileHeader  t_GeomFileHeader;


#endif // __PC_GEOM_HPP_INCLUED__