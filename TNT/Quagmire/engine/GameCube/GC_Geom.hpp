////////////////////////////////////////////////////////////////////////////
//
// GC_Geom.hpp
//
// Geom structures and defines specific to the GameCube
//
////////////////////////////////////////////////////////////////////////////

#ifndef GC_GEOM_HPP_INCLUDED
#define GC_GEOM_HPP_INCLUDED


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_math.hpp"
#include "x_color.hpp"


////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DEFINES
////////////////////////////////////////////////////////////////////////////

#define MAX_MESH_NAME   16


////////////////////////////////////////////////////////////////////////////
// TYPEDEFS
////////////////////////////////////////////////////////////////////////////

typedef struct SGeomSubMesh     t_GeomSubMesh;
typedef struct SGeomMesh        t_GeomMesh;
typedef struct SGeomFileHeader  t_GeomFileHeader;


////////////////////////////////////////////////////////////////////////////
// STRUCTURES
////////////////////////////////////////////////////////////////////////////

struct SGeomSubMesh
{
    s32         MeshID;                 // mesh id that this submesh belongs to
    u32         Flags;                  // GEOM_SUBMESH_FLAG_xxx bits
    s32         TextureID;              // Texture index
    void*       pDispList;              // pointer to submesh's display list
    u32         DispListSize;           // size in bytes of display list
    s32         NVerts;                 // number of verts in submesh
    s32         NTris;                  // number of tris in submesh
    // region data (for detecting when the object should be culled)
    vector3         Corner;                 // corner of the region
    vector3         Axis[3];                // region axis
    vector3         MidPoint;               // midpoint of the region
};


struct SGeomMesh
{
    char        Name[MAX_MESH_NAME];    // Name of mesh
    u32         Flags;                  // GEOM_MESH_FLAG_xxx bits
    s32         FirstSubMesh;           // First submesh from complete list
    s32         NSubMeshes;             // Number of submeshes in this mesh
    s32         NVerts;                 // Number of verts
    f32         Radius;                 // Radius of mesh
    s32         VertFormat;             // Vertex format
    void*       pVertData;              // Pointer to vertex data
};


struct SGeomFileHeader
{
    char        Name[MAX_MESH_NAME];    // Name of Geom
    u32         Flags;                  // GEOM_FLAG_xxx bits
    s32         NTextures;              // Number of Textures
    s32         NTexturesInFile;        // Number of Textures in file
    s32         NMeshes;                // Number of Meshes
    s32         NSubMeshes;             // Number of Submeshes
    u32         VertexDataSize;         // Size of vertex data
    u32         DispListDataSize;       // Size of display list data
};


///////////////////////////////////////////////////////////////
//
// A GameCube Geom file is laid out like this:
//
//      Header          (t_GeomFileHeader)
//      Texture Names   (array of texture names 32 chars/name)
//      Mesh Data       (array of t_GeomMesh)
//      Submesh Data    (array of t_GeomSubMesh)
//      Vertex Data     (binary data)
//      DispList Data   (binary data)
//      Textures        (array of x_bitmaps)
//
///////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// VERTEX STRUCTURES AND DEFINES
////////////////////////////////////////////////////////////////////////////

#define GC_VTXFMT_NONE          0

#define GC_VTXFMT_POS_ONLY      1   // vertex has position only
#define GC_VTXFMT_NORMAL        2   // vertex has position, normal, and UV
#define GC_VTXFMT_COLOR         3   // vertex has position, color, and UV


typedef struct SGCVertNormal t_GCVertNormal;
typedef struct SGCVertColor  t_GCVertColor;

//==========================================================================

struct SGCVertNormal
{
    vector3     Pos;
    vector3     Normal;
    vector2     UV;
};

struct SGCVertColor
{
    vector3     Pos;
    color       Color;
    vector2     UV;
};

////////////////////////////////////////////////////////////////////////////

#endif // GC_GEOM_HPP_INCLUED
