#ifndef __MESH_H
#define __MESH_H

/*****************************************************************************/
#include "x_types.hpp"
#include "x_math.hpp"

#include "CTextFile.h"

/*****************************************************************************/
/*
*MESH = "My dog pepe"
{
    *REFERENCE "MySkel"

    ; Vertex properties

	*NUM_CHUNKS = 4

    *CHUNK = "Legs.LOD1"
    {
        *VERTEX[ 4 ] = 
        {
        ;    X     Y     Z
            1.0, 1.0, 1.0,
            1.0, 1.0, 1.0,
            1.0, 1.0, 1.0,
            1.0, 1.0, 1.0,
        }

        *NORMAL[ 4 ] = 
        {
        ;    X     Y     Z
            1.0, 1.0, 1.0,
            1.0, 1.0, 1.0,
            1.0, 1.0, 1.0,
            1.0, 1.0, 1.0,
        }

        *COLOR[ 4 ] = 
        {
        ;   R  G  B  A
            1, 1, 1, 1,
            1, 1, 1, 1,
            1, 1, 1, 1,
            1, 1, 1, 1,
        }

        *UVMAP[ 8 ] = 
        {
        ; VIndex  PassNum    U    V
            1,       0,     0.2, 0.4,
            2,       0,     0.2, 0.4,
            3,       0,     0.2, 0.4,
            4,       0,     0.2, 0.4,
            1,       1,     0.2, 0.4,
            2,       1,     0.2, 0.4,
            3,       1,     0.2, 0.4,
            4,       1,     0.2, 0.4,
        }
    
        *WEIGHT[ 4 ] = 
        {
        ; NMatrices MIndex  %   MIndex   %    MIndex   %
            3,        1,   100,    1,   100,     1,    0,
            3,        1,   100,    1,   100,     1,    0,
            3,        1,   100,    1,   100,     1,    0,
            3,        1,   100,    1,   100,     1,    0,
        }

        *TRIS[ 3 ] =
        {
        ; VIndex VIndex VIndex  MaterialIndex
            1,     3,     4,    3,
            1,     3,     4,    3,
            1,     3,     4,    3,
        }

        *QUADS[ 3 ] =
        {
        ; VIndex VIndex VIndex VIndex  MaterialIndex
            1,     2,     3,     4,    3,
            1,     2,     3,     4,    3,
            1,     2,     3,     4,    3,
        }
    }

    *TEXTURE[ 4 ] =
    {
    ;  W   H   TextureName
       32, 32, "Texture.bmp",   
       32, 32, "Texture.bmp",   
       32, 32, "Texture.bmp",   
       32, 32, "Texture.bmp",   
    }

    *SUBMATERIAL[ 4 ] =
    {
    ; UMirrow VMirrow UTile VTile TextureID Intensity Operation
          0,      0,    1,    1,    2,         1.0,    "EMBOSS",         ; PUNCH_THROUGH, OPAQUE, ALPHA
          0,      0,    1,    1,    2,         1.0,    "SPECULAR",
          0,      0,    1,    1,    3,         1.0,    "DETAIL",
          0,      0,    1,    1,    3,         1.0,    "OP_MULT",
    }

    *MATERIAL[ 4 ] =
    {
    ;   MaterialName   NPasses SubmaterialIDs  UVPass
        "MaterialName",   1,   2,              0, 
        "MaterialName",   4,   2, 0, 1, 3      0, 1, 1, 0,
        "MaterialName",   4,   
        "MaterialName",   4,   
    }
}
*/
/*****************************************************************************/

namespace mesh
{

enum
{
	MAX_PASSES  = 8,										// max # texture passes
	MAX_WEIGHTS = 8,										// max # of weights assigned to a vert
};															



enum operation												// texture pass operation
{
	OP_OPAQUE,
	OP_PUNCH_THROUGH,
	OP_ALPHA,
	OP_EMBOSS,
	OP_DOT3,
	OP_SPECULAR,
	OP_DETAIL,
	OP_LIGHTMAP,
	OP_MULT,
	OP_ADD,
	OP_SUB,
};



enum wrap_mode												// uv wrap mode
{
    WMODE_NONE,
	WMODE_CLAMP,
	WMODE_TILE,
	WMODE_MIRROR,
};



struct weight
{
// construction
	weight() : m_iMatrix(-1) {}

// data
	f32             m_Weight;								// in range [ 0, 1 ]
	s32             m_iMatrix;								// matrix index 
};


struct vertex
{
// construction
	vertex() : m_nPasses(0), m_nWeights(0) {}

// data
	vector3			m_vPos;
	vector3			m_vNormal;
	vector4			m_Color;								// (RGBA) (XYZW)

	s32				m_nPasses;								// # passes
	vector2			m_UV[ MAX_PASSES ];						// uvs for each pass

	s32				m_nWeights;								// # weights assigned to this vert
	weight			m_Weight[ MAX_WEIGHTS ];				// the weights
};


struct texture
{
// construction
	texture() : m_Width(-1), m_Height(-1) {}

// data
	char			m_FileName[ 256 ];
	s32				m_Width;
	s32				m_Height;
};


struct sub_material
{
// construction
	sub_material() : m_iTexture(-1)	{}

// data
	operation		m_Operation;							// operation
	f32				m_Intensity;							// intensity of the blending
	s32				m_iTexture;								// index to texture
	wrap_mode		m_WrapU;								// wrap mode for u and v
	wrap_mode		m_WrapV;
};



struct material
{
// construction
	material() : m_nPasses(0) {}

// data
	char			m_Name[ 256 ];
	s32				m_nPasses;
	s32				m_iSubMat[ MAX_PASSES ];				// index to sub-material
	s32				m_iUV[ MAX_PASSES ];					// index to uvs to be used (in vertex)
};



struct face
{
// data
	xbool           m_bQuad;								// if FALSE then is a triangle
	s32             m_Index[ 4 ];							// vert indices (3 for a tri, 4 for a quad)
	s32             m_iMaterial;							// material index
};



struct bone
{
// data
	s32				m_Id;									// bone id
	char			m_Name[ 256 ];							// name of the bone
	s32				m_nChildren;							// # children
};



struct chunk
{
// construction
	chunk() : m_nVerts(0), m_pVertex(NULL), m_nFaces(0), m_pFace(NULL) {}
	~chunk()
	{
		delete [] m_pVertex;
		m_pVertex = NULL;

		delete [] m_pFace;
		m_pFace = NULL;
	}

// data
	char            m_Name[ 256 ];							// name of the chunk

	s32             m_nVerts;								// # verts
	vertex*         m_pVertex;								// ptr to verts

	s32             m_nFaces;								// # facets
	face*			m_pFace;								// ptr to facets
};



/*****************************************************************************
*
*	Class:	mesh::object
*
*****************************************************************************/
class object
{
// construction
public:
	object();
	~object();

// operations
public:
	xbool Save(CTextFile& Tf);
	xbool Load(CTextFile& Tf);

protected:
	void RecurseSaveHierarchy(CTextFile& Tf, s32& iBone, s32 iParent, s32 Indent);
	void SaveChunks(CTextFile& Tf);

	void LoadChunk(CTextFile& Tf, const s32 iChunk);


// data
public:
	char            m_Name[ 256 ];							// name of the object

	s32				m_nBones;								// # bones
	bone			*m_pBone;								// ptr to bones

	s32             m_nChunks;								// # chunks
	chunk			*m_pChunk;								// ptr to chunks

	s32             m_nTextures;							// # textures
	texture			*m_pTexture;							// ptr to textures

	s32             m_nSubMaterials;						// # sub-materials
	sub_material	*m_pSubMaterial;						// ptr to sub-materials

	s32             m_nMaterials;							// # materials
	material		*m_pMaterial;							// ptr to materials
};

/****************************************************************************/

};

/*****************************************************************************/

#endif //__MESH_H
