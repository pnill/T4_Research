#ifndef __ASCII_MESH_H
#define __ASCII_MESH_H

//****************************************************************************
#include "x_math.hpp"
#include "x_bitmap.hpp"
#include "x_vector.h"
#include "x_algorithm.h"

#include "CTextFile.h"



//****************************************************************************
namespace mesh
{
	const char	kIdMesh[]			= "MESH";							// mesh id in mtf

	const s32	kMaxNameLen			= 256;
	const s32	kMaxNumStages		= 8;								// max num texture stages
	const s32	kMaxNumWeights		= 8;								// max num weights assigned to a vert

	const f32	kTolerancePos		= 0.0001f;							//
	const f32	kToleranceNormals	= 0.03f;							// 
	const f32	kToleranceUVs		= 0.004f;							//
	const f32	kToleranceColors	= 0.01f;							// 



// ---------------------------------------------------------------------------
enum target
{
	MESH_TARGET_NONE = 0,

	MESH_TARGET_GENERIC,
	MESH_TARGET_PC,
	MESH_TARGET_PS2,
	MESH_TARGET_XBOX,
	MESH_TARGET_GC,

	MESH_TARGET_MAX
};

// ---------------------------------------------------------------------------
enum operation															// texture pass operation
{
	OP_NONE = 0,

	OP_DIFFUSE,															// diffuse using main uvs
	OP_DIFFUSE_UV,														// diffuse with own uvs

	OP_LIGHTMAP,														// lightmap

	OP_MASK,															// gloss map using main uvs
	OP_MASK_UV,															// gloss map with own uvs

	OP_ENV,																// environment
	OP_CUBE_ENV,														// cubic environment

	OP_BUMP,															// bump using main uvs
	OP_BUMP_UV,															// bump with own uvs

	OP_DETAIL,
	OP_DETAIL_UV,

	OP_NORMAL,															// normal map
	OP_EMBM,															// environment mapped bump
	OP_DOT3,

	OP_DIFFUSE_MASK,
	OP_SELF_ILLUM_MASK,

	OP_EFFECT_GLOW,
	OP_EFFECT_CAUSTIC,

	OP_SPECULAR,
	OP_SELF_ILLUM,
	OP_SELF_ILLUM_UV,

	OP_END,																// terminator
};

// ---------------------------------------------------------------------------
enum material_flags								// material flags
{
	MATERIAL_FLAG_NONE				= 0,
									
	MATERIAL_FLAG_DOUBLE_SIDED		= (1<<0),	// whether double sided	
	MATERIAL_FLAG_REFLECTIVE		= (1<<1),
	MATERIAL_FLAG_LIGHTING			= (1<<2),	// Does this material get lit?
	MATERIAL_FLAG_DYNAMIC			= (1<<3),	// Is this material for a dynamic mesh?
	MATERIAL_FLAG_FACETED			= (1<<4),	// whether facet
	MATERIAL_FLAG_RENDERTARGET		= (1<<5),	// Material is a render target
	MATERIAL_FLAG_UNSORTED_ALPHA	= (1<<6),	// Do not sort objects with this material
	MATERIAL_FLAG_BLEND_ANIM_TEX	= (1<<7),	// Blend any animated textures
	MATERIAL_FLAG_ANIM_TEX			= (1<<8),	// This material has animated textures
	MATERIAL_FLAG_MORPH				= (1<<9),	// verts morph
	MATERIAL_FLAG_MORPH_NORMALS		= (1<<10),	// verts and normals morph
	MATERIAL_FLAG_RENDERS			= (1<<11),	// Does this material render?
	MATERIAL_FLAG_VIDEO_TEXTURE		= (1<<12),	// Does this material render with a video stream?
	MATERIAL_FLAG_EXTRA_PASS		= (1<<13),	// This material is not used in a first render pass
	MATERIAL_FLAG_SHADOW_PASS		= (1<<14),	// This material requires a separate shadowing pass
	MATERIAL_FLAG_UNSHADOWED		= (1<<15),	// This material does not accept shadows
};

// ---------------------------------------------------------------------------
enum stage_flags														// stage flags
{
	STAGE_FLAG_NONE				= 0,

	STAGE_FLAG_DUNNO			= (1<<0),
};

// ---------------------------------------------------------------------------
enum blend_operation													// texture blend op
{
	BLEND_OP_NONE = 0,

	BLEND_OP_TEXTURE_ALPHA,
	BLEND_OP_VERTEX_ALPHA,

	BLEND_OP_END,														// terminator
};

// ---------------------------------------------------------------------------
enum alpha_mode															// texture alpha mode
{
	ALPHA_MODE_NONE = 0,
	ALPHA_MODE_PUNCHTHROUGH,
	ALPHA_MODE_TRANSPARENT,
};

// ---------------------------------------------------------------------------
enum sided_mode															// single vs. double
{
	SINGLE_SIDED_MODE = 0,
	DOUBLE_SIDED_MODE,

};

// ---------------------------------------------------------------------------
enum shadowed_mode															// single vs. double
{
	SHADOWED_MODE = 0,
	UNSHADOWED_MODE,

};

// ---------------------------------------------------------------------------
enum uvwrap_mode														// uv wrap mode
{
	UVWRAP_MODE_NONE = 0,
	UVWRAP_MODE_CLAMP,
	UVWRAP_MODE_TILE,
	UVWRAP_MODE_MIRROR,
};

// ---------------------------------------------------------------------------
enum uvmod_mode															// uv anim mode
{
	UVMOD_MODE_NONE = 0,
	UVMOD_MODE_SCROLL,
	UVMOD_MODE_ROTATE,

	UVMOD_MODE_END,														// terminator
};

// ---------------------------------------------------------------------------
enum uvmod_anim_mode													// uv_mod anim mode
{
	UVMOD_ANIM_MODE_NONE = 0,
	UVMOD_ANIM_MODE_FORWARDS,
	UVMOD_ANIM_MODE_BACKWARDS,
	UVMOD_ANIM_MODE_PINGPONG,

	UVMOD_ANIM_MODE_SIN,
};

// ---------------------------------------------------------------------------
struct weight
{
// construction
	weight() : m_Weight(0.0f), m_iMatrix(-1) {}

// data
	f32				m_Weight;											// in range [ 0, 1 ]
	s32				m_iMatrix;											// matrix index 
};

// ---------------------------------------------------------------------------
struct vertex
{
// construction
	vertex() : m_vPos(0.0f, 0.0f, 0.0f), m_vRenderPos(0.0f, 0.0f, 0.0f),
			   m_vNormal(0.0f, 0.0f, 0.0f), m_vRenderNormal(0.0f, 0.0f, 0.0f),
			   m_Color(1.0f, 1.0f, 1.0f, 1.0f), m_nStages(0), m_nWeights(0)
	{
		for(s32 cStage = 0; cStage < kMaxNumStages; cStage++)
		{
			m_UVs[cStage] = vector2(0.0f, 0.0f);
		}

		for(s32 cWeight = 0; cWeight < kMaxNumWeights; cWeight++)
		{
			m_Weights[cWeight].m_iMatrix = -1;
			m_Weights[cWeight].m_Weight = 0.0f;
		}
	}

// data
	vector3			m_vPos;												// original vert pos
	vector3			m_vRenderPos;										// possibly updated vert pos
	vector3			m_vNormal;											// origin normal
	vector3			m_vRenderNormal;									// possibly updated normal
	vector4			m_Color;											// (RGBA) (XYZW)

	s32				m_nStages;											// num stages
	vector2			m_UVs[kMaxNumStages];								// uvs for each stage

	s32				m_nWeights;											// num weights assigned to this vert
	weight			m_Weights[kMaxNumWeights];							// the weights
};

// ---------------------------------------------------------------------------
struct texture
{
// construction
	texture() : m_Width(-1), m_Height(-1), m_Bpp(-1), m_AlphaMode(ALPHA_MODE_NONE), m_nFiles(1)
	{
		m_Filename[0] = '\0';
	}

// operators
	xbool	 operator== (const texture &Source) const;

// data
	s32				m_Width;
	s32				m_Height;
	s32				m_Bpp;

	alpha_mode		m_AlphaMode;

	s32				m_nFiles;											// number of filenames
	char			m_Filename[kMaxNameLen];							// filename (basename if more than one)
};

// ---------------------------------------------------------------------------
struct uv_mod															// uv modification data
{
// construction
	uv_mod() : m_Mode(UVMOD_MODE_NONE), m_AnimMode(UVMOD_ANIM_MODE_NONE),
			   m_vOrigin(0.0f, 0.0f), m_ScrollSpeed(0.0f, 0.0f), m_RotateSpeed(0.0f), m_Amplitude(1.0f), m_Bias(0.0f) {}

	xbool		operator==		(const uv_mod &Source) const;

// data
	uvmod_mode		m_Mode;												// uv mod mode
//JONNY: note would want to do scroll AND rotate !?!?!?!

	uvmod_anim_mode	m_AnimMode;

	vector2			m_vOrigin;											// uv origin point
	vector2			m_ScrollSpeed;										// uv scroll speed in units per second
	radian			m_RotateSpeed;										// uv rotate in radians per second

	f32				m_Amplitude;
	f32				m_Bias;
};

// ---------------------------------------------------------------------------
struct stage
{
// construction
	stage() : m_iUV(-1), m_iTexture(-1), m_iTransform(-1), 
			  m_Flags(STAGE_FLAG_NONE), m_Alpha(1.0f), m_ActiveDistance(0.0f),
			  m_Op(OP_NONE), m_BlendOp(BLEND_OP_NONE), 
			  m_WrapU(UVWRAP_MODE_NONE), m_WrapV(UVWRAP_MODE_NONE), m_UvScale (1.0f, 1.0f)	{}

	xbool		operator==		(const stage &Source) const;
// data
	s32				m_iUV;												// index to uvs to be used (in vertex)
	s32				m_iTexture;											// index to texture
	s32				m_iTransform;										// index to transform

	u32				m_Flags;											// misc flags

	f32				m_Alpha;											// alpha of this stage
	f32				m_ActiveDistance;									// distance from camera upto which stage is active (0.0 = always active)

	operation		m_Op;												// operation
	blend_operation	m_BlendOp;

	uvwrap_mode		m_WrapU;											// wrap mode for u and v
	uvwrap_mode		m_WrapV;

	vector2			m_UvScale;											// uv scale values
	uv_mod			m_UvMod;											// uv mod
};

// ---------------------------------------------------------------------------
struct material
{
// construction
	material() : m_nStages(0), m_OverallAlphaMode(ALPHA_MODE_NONE), m_Alpha(1.0f), m_Flags(MATERIAL_FLAG_NONE)
	{
		m_Name[0] = '\0';

		for(s32 cStage = 0; cStage < kMaxNumStages; cStage++)
		{
			m_iStages[cStage] = -1;
		}
	}

// operators
	xbool		operator==		(const material &Source) const;

	vector4		estimated_color	(const vector2& UVCoords, f32 t, const vector4& Diffuse, const stage* pStages, x_bitmap* pTextures) const;
	color		estimated_color	(f32 u_coord, f32 v_coord, f32 t, color diffuse, const stage* pStages, x_bitmap* pTextures) const;

// data
	char			m_Name[kMaxNameLen];

	s32				m_nStages;
	s32				m_iStages[kMaxNumStages];							// stage indices

	alpha_mode		m_OverallAlphaMode;									// overall alpha mode

	f32				m_Alpha;											// alpha of the whole material

	u32				m_Flags;											// flags
};

// ---------------------------------------------------------------------------
struct material_set
{
// construction
	material_set	() : m_Target(MESH_TARGET_NONE),
						 m_nTextures(0), m_pTextures(NULL),  
						 m_nStages(0), m_pStages(NULL), 
						 m_nMaterials(0), m_pMaterials(NULL), 
						 m_nTransforms(0), m_pTransforms(NULL) {}
	material_set	(const material_set& From);
	~material_set	();

// operations
	material_set&	operator=		(const material_set& From);
	s32				merge_set		(const material_set&);

	color			estimated_color	(s32 material_index, f32 u_coord, f32 v_coord, f32 t, color diffuse) const;
		// WARNING - this is exceptionally slow!
		// It is better to create an x_bitmap array yourself, and call the material
		// directly.  This member is only provided as a convenience.

// data
	target			m_Target;

	s32				m_nTextures;										// texture list
	texture			*m_pTextures;

	s32				m_nStages;											// stage list
	stage			*m_pStages;

	s32				m_nMaterials;										// material list
	material		*m_pMaterials;

	s32				m_nTransforms;										// transform list
	matrix4			*m_pTransforms;
};

// ---------------------------------------------------------------------------
struct face
{
// construction
	face() : m_bQuad(FALSE), m_iMaterial(-1), m_SmoothingGroup(1), m_Flags(0)
	{
		m_Index[0] = m_Index[1] = m_Index[2] = m_Index[3] = -1;
	}

// data
	xbool           m_bQuad;											// if FALSE then is a triangle
	s32             m_Index[4];											// vert indices (3 for a tri, 4 for a quad)
	s32             m_iMaterial;										// material index
	u32				m_SmoothingGroup;									// smoothing group
	u32				m_Flags;
};

// ---------------------------------------------------------------------------
struct bone
{
// construction
	bone() : m_Id(-1), m_nChildren(0)
	{
		m_Name[0] = '\0';
	}

// data
	s32				m_Id;												// bone id
	char			m_Name[kMaxNameLen];								// name of the bone
	s32				m_nChildren;										// num children
};

// ---------------------------------------------------------------------------
struct chunk
{
// construction
	chunk	() : m_nVerts(0), m_pVerts(NULL), m_nFaces(0), m_pFaces(NULL), m_userData(0)
	{
		m_Name[0] = '\0';
	}

	chunk	(const chunk& From);
	~chunk	();

// operations
	chunk& operator= (const chunk& From);

// data
	char			m_Name[kMaxNameLen];								// name of the chunk

	s32				m_nVerts;											// num verts
	vertex			*m_pVerts;											// ptr to verts

	s32				m_nFaces;											// num faces
	face			*m_pFaces;											// ptr to faces

	s32       m_userData;
};

// ---------------------------------------------------------------------------
typedef void (*fnTickCallback)(void);



// ---------------------------------------------------------------------------
struct t_UserData
{
	t_UserData() {}
	t_UserData(const char *pId_, const char* pValue_) : m_Id(pId_)
	{
		if (pValue_)
		{
			m_Value = x_std::string(pValue_);
		}
	}

	t_UserData(const x_std::string& Id_, const x_std::string& Value) : m_Id(Id_), m_Value(Value) {}
	t_UserData(const x_std::string& Id_) : m_Id(Id_) {}

	x_std::string	m_Id;
	x_std::string	m_Value;
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
	object	();
	object	(const object &From);
	~object	();

// operations
public:
// ---------------------------------------------------------------------------
// assignment
	object& operator=				(const object& From);
	void	Init					(void);
	void	Unload					(void);
	void	Copy					(const object& From);

	f32		GetVersion				(void) const;

	xbool	Save					(CTextFile& Tf, fnTickCallback fnCallback = NULL) const;
	xbool	Load					(CTextFile& Tf, fnTickCallback fnCallback = NULL);

	void	MergeVerts				(fnTickCallback	fnCallback	= NULL,
									 f32 TolerancePos			= kTolerancePos,
									 f32 ToleranceNormals		= kToleranceNormals,
									 f32 ToleranceUVs			= kToleranceUVs,
									 f32 ToleranceColors		= kToleranceColors);
	xbool	UpdateTextureInfo		(void);
//JONNY: this updates the texture info by first loading the file - if it doesnt exist it returns false
//		 dunno if this is a good way to do it - as the caller doesnt know which texture failed !?
//       - it would also tie AsciiMesh to AuxBitmap - is this something we'd want to do ?

	s32		GetChunkId				(const x_std::string& Name) const;
	s32		GetMaterialSetId		(target Target) const;

	void	Rotate					(const radian3& Rot);
	f32		GetVersion()			{ return m_Version; }
// return bounding box expressed as	3 vectors: origin, extents, and rotation
	void	GetBounds				(vector3 bounds[3]); 

	xbool	FixInvalidData			(fnTickCallback fnCallback = NULL);
    xbool   FixPS2CrowdUVs          (void);
	void	UpdateOverallAlphaModes	(void);
	xbool	RemoveUnusedVerts		(fnTickCallback fnCallback = NULL);
	xbool	RemapWeights			(const x_std::vector<s32>& iBonesOldToNew);

protected:
	void	RecurseSaveHierarchy	(CTextFile& Tf, s32& iBone, s32 iParent, s32 Indent) const;
	void	SaveChunks				(CTextFile& Tf, fnTickCallback fnCallback) const;

	xbool	Load_v1_0				(CTextFile& Tf, CTextFile::tokentype FirstToken, fnTickCallback fnCallback);
	xbool	LoadChunk_v1_0			(CTextFile& Tf, s32 iChunk);

	xbool	Load_v2_0_v3_0			(CTextFile& Tf, s32 nVariation, fnTickCallback fnCallback);
	xbool	LoadChunk_v2_0_v3_0		(CTextFile& Tf, s32 iChunk, s32 nVariation);
	xbool	LoadMaterialSet_v2_0	(CTextFile& Tf, material_set *pMaterialSet);
	xbool	LoadMaterialSet_v3_0	(CTextFile& Tf, material_set *pMaterialSet);

// data
public:
	s32							m_nDateStamp;

	char						m_Name[kMaxNameLen];					// name of the object

	s32							m_nBones;								// bone list
	bone						*m_pBones;

	s32							m_nChunks;								// chunk list
	chunk						*m_pChunks;

	s32							m_nMaterialSets;
	material_set				*m_pMaterialSets;

	s32							m_nMaterialRemapIndices;
	s32							*m_pMaterialRemapIndices;

	x_std::vector<t_UserData>	m_UserDataList;

private:
	f32							m_Version;								// version
};



//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
inline material_set::~material_set()
{
	delete [] m_pTextures;
	m_pTextures = NULL;

	delete [] m_pStages;
	m_pStages = NULL;

	delete [] m_pMaterials;
	m_pMaterials = NULL;

	delete [] m_pTransforms;
	m_pTransforms = NULL;
}

// ---------------------------------------------------------------------------
// copy constructor
inline material_set::material_set(const material_set& From) :
	m_Target		(From.m_Target),
	m_nTextures		(From.m_nTextures),
	m_pTextures		(NULL),
	m_nStages		(From.m_nStages),
	m_pStages		(NULL),
	m_nMaterials	(From.m_nMaterials),
	m_pMaterials	(NULL),
	m_nTransforms	(From.m_nTransforms),
	m_pTransforms	(NULL)
{
	if (m_nTextures)
	{
		m_pTextures = new texture [m_nTextures];
		x_std::copy (From.m_pTextures, From.m_pTextures + m_nTextures, m_pTextures);
	}

	if (m_nStages)
	{
		m_pStages = new stage [m_nStages];
		x_std::copy (From.m_pStages, From.m_pStages + m_nStages, m_pStages);
	}

	if (m_nMaterials)
	{
		m_pMaterials = new material [m_nMaterials];
		x_std::copy (From.m_pMaterials, From.m_pMaterials + m_nMaterials, m_pMaterials);
	}

	if (m_nTransforms)
	{
		m_pTransforms = new matrix4 [m_nTransforms];
		x_std::copy (From.m_pTransforms, From.m_pTransforms + m_nTransforms, m_pTransforms);
	}
}

// ---------------------------------------------------------------------------
// assignment
inline material_set& material_set::operator= (const material_set& From)
{
	if (this != &From)
	{
		m_Target = From.m_Target;

		m_nTextures = From.m_nTextures;
		delete [] m_pTextures;
		if (m_nTextures)
		{
			m_pTextures = new texture [m_nTextures];
			x_std::copy (From.m_pTextures, From.m_pTextures + m_nTextures, m_pTextures);
		}
		else
		{
			m_pTextures = NULL;
		}

		m_nStages = From.m_nStages;
		delete [] m_pStages;
		if (m_nStages)
		{
			m_pStages = new stage [m_nStages];
			x_std::copy (From.m_pStages, From.m_pStages + m_nStages, m_pStages);
		}
		else
		{
			m_pStages = NULL;
		}

		m_nMaterials = From.m_nMaterials;
		delete [] m_pMaterials;
		if (m_nMaterials)
		{
			m_pMaterials = new material [m_nMaterials];
			x_std::copy (From.m_pMaterials, From.m_pMaterials + m_nMaterials, m_pMaterials);
		}
		else
		{
			m_pMaterials = NULL;
		}

		m_nTransforms = From.m_nTransforms;
		delete [] m_pTransforms;
		if (m_nTransforms)
		{
			m_pTransforms = new matrix4 [m_nTransforms];
			x_std::copy (From.m_pTransforms, From.m_pTransforms + m_nTransforms, m_pTransforms);
		}
		else
		{
			m_pTransforms = NULL;
		}
	}

	return *this;
}

// ---------------------------------------------------------------------------
inline chunk::~chunk()
{
	delete [] m_pVerts;
	m_pVerts = NULL;

	delete [] m_pFaces;
	m_pFaces = NULL;
}

// ---------------------------------------------------------------------------
// copy constructor
inline chunk::chunk(const chunk& From)
{
	x_strncpy (m_Name, From.m_Name, sizeof (m_Name));

	m_nVerts = From.m_nVerts;
	m_pVerts = NULL;
	if (m_nVerts)
	{
		m_pVerts = new vertex [m_nVerts];
		x_std::copy (From.m_pVerts, From.m_pVerts + m_nVerts, m_pVerts);
	}

	m_nFaces = From.m_nFaces;
	m_pFaces = NULL;
	if (m_nFaces)
	{
		m_pFaces = new face [m_nFaces];
		x_std::copy (From.m_pFaces, From.m_pFaces + m_nFaces, m_pFaces);
	}
}

// ---------------------------------------------------------------------------
// assignment
inline chunk& chunk::operator= (const chunk& From)
{
	if (this != &From)
	{
		x_strncpy (m_Name, From.m_Name, sizeof (m_Name));

		m_nVerts = From.m_nVerts;
		delete [] m_pVerts;
		if (m_nVerts)
		{
			m_pVerts = new vertex [m_nVerts];
			x_std::copy (From.m_pVerts, From.m_pVerts + m_nVerts, m_pVerts);
		}
		else
		{
			m_pVerts = NULL;
		}

		m_nFaces = From.m_nFaces;
		delete [] m_pFaces;
		if (m_nFaces)
		{
			m_pFaces = new face [m_nFaces];
			x_std::copy (From.m_pFaces, From.m_pFaces + m_nFaces, m_pFaces);
		}
		else
		{
			m_pFaces = NULL;
		}

		m_userData=From.m_userData;
	}

	return *this;
}



//****************************************************************************
// uv_mod class inline functions
//****************************************************************************
inline xbool uv_mod::operator == (const uv_mod &Source) const
{
	return (x_memcmp(this, &Source, sizeof(uv_mod)) == 0);
}


//****************************************************************************
// stage class inline functions.
//****************************************************************************
inline xbool stage::operator == (const stage &Source) const
{
	return (x_memcmp(this, &Source, sizeof(stage)) == 0);
}




//****************************************************************************
// texture class inline functions.
//****************************************************************************
inline xbool texture::operator == (const texture &Source) const
{
	if ( (x_strcmp(m_Filename, Source.m_Filename) != 0) ||	(m_nFiles != Source.m_nFiles) )
	{
		return FALSE;
	}

	return TRUE;
}


//****************************************************************************
// Inline funbctions for material class operators.
//****************************************************************************
inline xbool material::operator == (const material &Source) const
{
	if ((x_strcmp(m_Name, Source.m_Name) != 0) ||
		(m_nStages != Source.m_nStages) ||
		(m_OverallAlphaMode != Source.m_OverallAlphaMode) ||
		(m_Alpha != Source.m_Alpha) ||
		(m_Flags != Source.m_Flags))
	{
		return FALSE;
	}

	for(s32 cStage = 0; cStage < m_nStages; cStage++)
	{
		if (m_iStages[cStage] != Source.m_iStages[cStage])
		{
			return FALSE;
		}
	}

	return TRUE;
}

//****************************************************************************

};

//****************************************************************************

#endif //__ASCII_MESH_H
