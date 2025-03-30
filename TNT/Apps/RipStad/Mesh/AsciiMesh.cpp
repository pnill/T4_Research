/*****************************************************************************
*
*	AsciiMesh.cpp - generic mesh structure
*
*	4/14/00
*
*	Major Update -- 6/4/01
*
*****************************************************************************/
//#include "stdafx.h"

#include "x_vector.h"

#include "asciimesh.h"

#include <float.h>


//JONNY: this needs to go somewhere !!!
#define	ARRAY_LENGTH(array)	(sizeof(array) / sizeof(array[0]))



//****************************************************************************
static const char	g_kIdVersion[]					= "VERSION";
static const char	g_kIdReference[]				= "REFERENCE";

static const char	g_kIdDateStamp[]				= "DATESTAMP";
										
static const char	g_kIdMesh[]						= "MESH";
										
static const char	g_kIdHierarchy[]				= "HIERARCHY";
										
static const char	g_kIdNumChunks[]				= "NUM_CHUNKS";
static const char	g_kIdChunk[]					= "CHUNK";
										
static const char	g_kIdVertex[]					= "VERTEX";
static const char	g_kIdNormal[]					= "NORMAL";
static const char	g_kIdColor[]					= "COLOR";
										
static const char	g_kIdUvMap[]					= "UVMAP";
static const char	g_kIdWeight[]					= "WEIGHT";

static const char	g_kIdNumMaterialSets[]			= "NUM_MATERIAL_SETS";
static const char	g_kIdMaterialSet[]				= "MATERIAL_SET";
static const char	g_kIdTexture[]					= "TEXTURE";
static const char	g_kIdStage[]					= "STAGE";
static const char	g_kIdMaterial[]					= "MATERIAL";
static const char	g_kIdTransform[]				= "TRANSFORM";

static const char	g_kIdAlphaModeNone[]			= "NONE";
static const char	g_kIdAlphaModePunchthrough[]	= "PUNCHTHRU";
static const char	g_kIdAlphaModeTransparent[]		= "TRANSPARENT";

static const char	g_kIdUvModModeNone[]			= "NONE";
static const char	g_kIdUvModModeScroll[]			= "SCROLL";
static const char	g_kIdUvModModeRotate[]			= "ROTATE";

static const char	g_kIdUvModAnimModeNone[]		= "NONE";
static const char	g_kIdUvModAnimModeForwards[]	= "FORWARDS";
static const char	g_kIdUvModAnimModeBackwards[]	= "BACKWARDS";
static const char	g_kIdUvModAnimModePingPong[]	= "PINGPONG";
static const char	g_kIdUvModAnimModeSin[]			= "SIN";

static const char	g_kIdTri[]						= "TRI";
static const char	g_kIdQuad[]						= "QUAD";

static const char	g_kIdTargetNone[]				= "TARGET_NONE";
static const char	g_kIdTargetGeneric[]			= "TARGET_GENERIC";
static const char	g_kIdTargetPc[]					= "TARGET_PC";
static const char	g_kIdTargetPs2[]				= "TARGET_PS2";
static const char	g_kIdTargetXBox[]				= "TARGET_XBOX";
static const char	g_kIdTargetGc[]					= "TARGET_GC";

static const char	g_kIdUserData[]					= "USER_DATA";



//****************************************************************************
static const char	g_kIdOpNone[]					= "NONE";
static const char	g_kIdOpDiffuse[]				= "DIFFUSE";
static const char	g_kIdOpDiffuseUv[]				= "DIFFUSE_UV";
static const char	g_kIdOpLightmap[]				= "LIGHTMAP";
static const char	g_kIdOpMask[]					= "MASK";
static const char	g_kIdOpMaskUv[]					= "MASK_UV";
static const char	g_kIdOpEnv[]					= "ENV";
static const char	g_kIdOpCubeEnv[]				= "CUBE_ENV";
static const char	g_kIdOpBump[]					= "BUMP";
static const char	g_kIdOpBumpUv[]					= "BUMP_UV";
static const char	g_kIdOpDetail[]					= "DETAIL";
static const char	g_kIdOpDetailUv[]				= "DETAIL_UV";
static const char	g_kIdOpNormal[]					= "NORMAL";
static const char	g_kIdOpEmbm[]					= "EMBM";
static const char	g_kIdOpDot3[]					= "DOT3";
static const char	g_kIdOpDiffuseMask[]			= "DIFFUSE_MASK";
static const char	g_kIdOpSelfIllumMask[]			= "SELF_ILLUM_MASK";
static const char	g_kIdOpEffectGlow[]				= "GLOW";
static const char	g_kIdOpEffectCaustic[]			= "CAUSTIC";
static const char	g_kIdOpSpecular[]				= "SPECULAR";
static const char	g_kIdOpSelfIllum[]				= "SELF_ILLUM";
static const char	g_kIdOpSelfIllumUV[]			= "SELF_ILLUM_UV";

static const char	g_IdBlendOpNone[]				= "NONE";
static const char	g_IdBlendOpTextureAlpha[]		= "TEXTURE_ALPHA";
static const char	g_IdBlendOpVertAlpha[]			= "VERT_ALPHA";

static const char	g_kIdWrapModeNone[]				= "NONE";
static const char	g_kIdWrapModeClamp[]			= "CLAMP";
static const char	g_kIdWrapModeTile[]				= "TILE";
static const char	g_kIdWrapModeMirror[]			= "MIRROR";



//****************************************************************************
// legacy tokens and constants

static const s32	g_kMaxNumPasses_v1_0	= 8;

static const char	g_kIdSubmaterial_v1_0[]			= "SUBMATERIAL";
static const char	g_kIdOpOpaque_v1_0[]			= "OPAQUE";
static const char	g_kIdOpPunchThrough_v1_0[]		= "PUNCH_THROUGH";
static const char	g_kIdOpAlpha_v1_0[]				= "ALPHA";
static const char	g_kIdOpEmboss_v1_0[]			= "EMBOSS";
static const char	g_kIdOpDot3_v1_0[]				= "DOT3";
static const char	g_kIdOpSpecular_v1_0[]			= "SPECULAR";
static const char	g_kIdOpDetail_v1_0[]			= "DETAIL";
static const char	g_kIdOpLightmap_v1_0[]			= "LIGHTMAP";
static const char	g_kIdOpMult_v1_0[]				= "MULT";
static const char	g_kIdOpAdd_v1_0[]				= "ADD";
static const char	g_kIdOpSub_v1_0[]				= "SUB";
static const char	g_kIdOpDiffuse1_v1_5[]			= "DIFFUSE_1_UV";
static const char	g_kIdOpMask_v1_0[]				= "MASK";
static const char	g_kIdOpBump_v1_0[]				= "BUMP";



//****************************************************************************
static const s32	g_kVertsPrecision		= 5;
static const s32	g_kNormalsPrecision		= 5;
static const s32	g_kUVsPrecision			= 4;
static const s32	g_kWeightsPrecision		= 5;

static const s32	g_kMaxStrLen			= 256;

static f32			g_MergeTolerancePos		= mesh::kTolerancePos;			// tolerance for snapping
static f32			g_MergeToleranceNormals	= mesh::kToleranceNormals;
static f32			g_MergeToleranceUVs		= mesh::kToleranceUVs;
static f32			g_MergeToleranceColor	= mesh::kToleranceColors;

static const f32	g_kCurrentSaveVersion	= 3.1f;



//****************************************************************************
namespace mesh
{
//note: these tables must be in the same order as the relevant enums

struct _AlphaModeTable
{
	alpha_mode	Mode;
	const char	*pId;
}
AlphaModeTable[] =
{
	ALPHA_MODE_NONE,			g_kIdAlphaModeNone,
	ALPHA_MODE_PUNCHTHROUGH,	g_kIdAlphaModePunchthrough,
	ALPHA_MODE_TRANSPARENT,		g_kIdAlphaModeTransparent,
};



struct _OpTable
{
	operation	Op;
	const char	*pId;
}
OpTable[] =
{
	OP_NONE,			g_kIdOpNone,	
	OP_DIFFUSE,			g_kIdOpDiffuse,
	OP_DIFFUSE_UV,		g_kIdOpDiffuseUv,
	OP_LIGHTMAP,		g_kIdOpLightmap,
	OP_MASK,			g_kIdOpMask,
	OP_MASK_UV,			g_kIdOpMaskUv,
	OP_ENV,				g_kIdOpEnv,
	OP_CUBE_ENV,		g_kIdOpCubeEnv,
	OP_BUMP,			g_kIdOpBump,
	OP_BUMP_UV,			g_kIdOpBumpUv,
	OP_DETAIL,			g_kIdOpDetail,
	OP_DETAIL_UV,		g_kIdOpDetailUv,
	OP_NORMAL,			g_kIdOpNormal,
	OP_EMBM,			g_kIdOpEmbm,
	OP_DOT3,			g_kIdOpDot3,
	OP_DIFFUSE_MASK,	g_kIdOpDiffuseMask,
	OP_SELF_ILLUM_MASK,	g_kIdOpSelfIllumMask,
	OP_EFFECT_GLOW,		g_kIdOpEffectGlow,
	OP_EFFECT_CAUSTIC,	g_kIdOpEffectCaustic,
	OP_SPECULAR,		g_kIdOpSpecular,
	OP_SELF_ILLUM,		g_kIdOpSelfIllum,
	OP_SELF_ILLUM_UV,	g_kIdOpSelfIllumUV,
};



struct _BlendOpTable
{
	blend_operation	Op;
	const char		*pId;
}
BlendOpTable[] =
{
	BLEND_OP_NONE,			g_IdBlendOpNone,

	BLEND_OP_TEXTURE_ALPHA,	g_IdBlendOpTextureAlpha,
	BLEND_OP_VERTEX_ALPHA,	g_IdBlendOpVertAlpha,
};



struct _WrapTable
{
	uvwrap_mode	Mode;
	const char	*pId;
}
WrapTable[] =
{
	UVWRAP_MODE_NONE,	g_kIdWrapModeNone,
	UVWRAP_MODE_CLAMP,	g_kIdWrapModeClamp,
	UVWRAP_MODE_TILE,	g_kIdWrapModeTile,
	UVWRAP_MODE_MIRROR,	g_kIdWrapModeMirror,
};



struct _UvModTable
{
	uvmod_mode	Mode;
	const char	*pId;
}
UvModTable[] =
{
	UVMOD_MODE_NONE,	g_kIdUvModModeNone,
	UVMOD_MODE_SCROLL,	g_kIdUvModModeScroll,
	UVMOD_MODE_ROTATE,	g_kIdUvModModeRotate,
};



struct _UvModAnimTable
{
	uvmod_anim_mode	Mode;
	const char		*pId;
}
UvModAnimTable[] =
{
	UVMOD_ANIM_MODE_NONE,		g_kIdUvModAnimModeNone,
	UVMOD_ANIM_MODE_FORWARDS,	g_kIdUvModAnimModeForwards,
	UVMOD_ANIM_MODE_BACKWARDS,	g_kIdUvModAnimModeBackwards,
	UVMOD_ANIM_MODE_PINGPONG,	g_kIdUvModAnimModePingPong,
	UVMOD_ANIM_MODE_SIN,		g_kIdUvModAnimModeSin,
};



struct _TargetTable
{
	target		Target;
	const char	*pId;
}
TargetTable[] =
{
	MESH_TARGET_NONE,		g_kIdTargetNone,
	MESH_TARGET_GENERIC,	g_kIdTargetGeneric,
	MESH_TARGET_PC,			g_kIdTargetPc,
	MESH_TARGET_PS2,		g_kIdTargetPs2,
	MESH_TARGET_XBOX,		g_kIdTargetXBox,
	MESH_TARGET_GC,			g_kIdTargetGc,
};


// ---------------------------------------------------------------------------
struct _OpTable_v1_0
{
	operation	Op;
	const char	*pId;
}
OpTable_v1_0[] =
{
	OP_NONE,				g_kIdOpOpaque_v1_0,
	OP_NONE,				g_kIdOpPunchThrough_v1_0,
	OP_NONE,				g_kIdOpAlpha_v1_0,
	OP_NONE,				g_kIdOpEmboss_v1_0,
	OP_DOT3,				g_kIdOpDot3_v1_0,
	OP_SPECULAR,			g_kIdOpSpecular_v1_0,
	OP_DETAIL,				g_kIdOpDetail_v1_0,
	OP_LIGHTMAP,			g_kIdOpLightmap_v1_0,
	OP_DIFFUSE_UV,			g_kIdOpMult_v1_0,
	OP_DIFFUSE,				g_kIdOpAdd_v1_0,
	OP_DIFFUSE,				g_kIdOpSub_v1_0,
	OP_DIFFUSE_UV, 			g_kIdOpDiffuse1_v1_5,
	OP_MASK,				g_kIdOpMask_v1_0,
	OP_BUMP,				g_kIdOpBump_v1_0,
};



/*****************************************************************************
*
*	Class:	mesh::object
*
*****************************************************************************/
object::object()
{
	Init();
}

//****************************************************************************
// copy constructor

object::object(const object &From)
{
	Copy(From);
}

//****************************************************************************
object::~object()
{
	Unload();
}

//****************************************************************************
object& object::operator= (const object& From)
{
	if (this != &From)
	{
		Unload();
		Copy(From);
	}

	return *this;
}

//****************************************************************************
void object::Copy(const object& From)
{
	Init();

	m_Version = From.m_Version;

	x_memcpy(m_Name, From.m_Name, sizeof(m_Name));

	m_nBones = From.m_nBones;
	if (m_nBones)
	{
		m_pBones = new bone [m_nBones];
		x_std::copy (From.m_pBones, From.m_pBones + m_nBones, m_pBones);
	}


	m_nChunks = From.m_nChunks;
	if (m_nChunks)
	{
		m_pChunks = new chunk [m_nChunks];
		x_std::copy (From.m_pChunks, From.m_pChunks + m_nChunks, m_pChunks);
	}


	m_nMaterialSets = From.m_nMaterialSets;
	if (m_nMaterialSets)
	{
		m_pMaterialSets = new material_set [m_nMaterialSets];
		x_std::copy (From.m_pMaterialSets, From.m_pMaterialSets + m_nMaterialSets, m_pMaterialSets);
	}

	m_nMaterialRemapIndices = From.m_nMaterialRemapIndices;
	if ( m_nMaterialRemapIndices )
	{
		m_pMaterialRemapIndices = new s32 [m_nMaterialRemapIndices];
		x_std::copy (From.m_pMaterialRemapIndices,
					 From.m_pMaterialRemapIndices + m_nMaterialRemapIndices,
					 m_pMaterialRemapIndices);
	}
}

//****************************************************************************
void object::Init(void)
{
	m_Version = g_kCurrentSaveVersion;

	m_nBones = 0;
	m_pBones = NULL;

	m_nChunks = 0;
	m_pChunks = NULL;

	m_nMaterialSets = 0;
	m_pMaterialSets = NULL;

	m_pMaterialRemapIndices = NULL;
}

//****************************************************************************
// unload loaded object

void object::Unload(void)
{
	delete [] m_pBones;
	m_pBones = NULL;
	delete [] m_pChunks;
	m_pChunks = NULL;
//	delete [] m_pMaterialSets;
	m_pMaterialSets = NULL;
	delete [] m_pMaterialRemapIndices;
	m_pMaterialRemapIndices = NULL;
}

//****************************************************************************
f32 object::GetVersion(void) const
{
	return m_Version;
}

//****************************************************************************
// save an object

xbool object::Save(CTextFile& Tf, fnTickCallback fnCallback) const
{
	Tf.BeginToken(g_kIdMesh, m_Name);

	// output version
	Tf.Token(g_kIdVersion, m_Version);


	// -----------------------------------------------------------------------
	// output userdata

	if (m_UserDataList.size())
	{
		Tf.Put("\n");

		Tf.BeginToken(g_kIdUserData, (s32)m_UserDataList.size());

		for(u32 i = 0; i < m_UserDataList.size(); i++)
		{
			Tf.Puts(m_UserDataList[i].m_Id.x_c_str0());

			Tf.Put(", ");

			if (m_UserDataList[i].m_Value.length())
			{
				Tf.Puts(m_UserDataList[i].m_Value.x_c_str0());
			}
			else
			{
				Tf.Puts("""");
			}

			Tf.Put(",\n");
		}

		Tf.EndToken();
	}

	// -----------------------------------------------------------------------
	// output hierarchy
	if (m_pBones)
	{
		Tf.BeginToken(g_kIdHierarchy, m_nBones);
		Tf.Put("; nChildren   Name\n");

		// output root
		Tf.Put(fs("%d,\t\"%s\"\n", m_pBones[0].m_nChildren, m_pBones[0].m_Name));

		// recurse on children
		s32	Id = 1;
		RecurseSaveHierarchy(Tf, Id, 0, 1);

		Tf.EndToken();
	}

	// -----------------------------------------------------------------------
	// output chunks
	if (m_nChunks)
	{
		Tf.Token(g_kIdNumChunks, m_nChunks);
		SaveChunks(Tf, fnCallback);
	}

	// -----------------------------------------------------------------------
	// output material sets
	if (m_pMaterialSets)
	{
		ASSERT(m_nMaterialSets);
		Tf.Token(g_kIdNumMaterialSets, m_nMaterialSets);

		for(s32 cMaterialSet = 0; cMaterialSet < m_nMaterialSets; cMaterialSet++)
		{
			material_set	*pMaterialSet = &m_pMaterialSets[cMaterialSet];

			Tf.BeginToken(g_kIdMaterialSet, TargetTable[pMaterialSet->m_Target].pId);

			// ---------------------------------------------------------------
			// output textures
			if (pMaterialSet->m_pTextures)
			{
				ASSERT(pMaterialSet->m_nTextures);

				Tf.BeginToken(g_kIdTexture, pMaterialSet->m_nTextures);
				Tf.Put("; W   H   BPP   AlphaMode   nFiles   TextureName(Base)\n");

				for(s32 cTexture = 0; cTexture < pMaterialSet->m_nTextures; cTexture++)
				{
					texture	*pTexture = &pMaterialSet->m_pTextures[cTexture];

					// output texture dimensions
					Tf.Put(fs("%d,\t%d,\t%d,\t", pTexture->m_Width, pTexture->m_Height, pTexture->m_Bpp));

					// output alpha mode
					Tf.Put(fs("\"%s\",\t", AlphaModeTable[pTexture->m_AlphaMode].pId));

					// output num names and filename (basename)
					Tf.Put(pTexture->m_nFiles);

					ASSERTS(x_strlen(pTexture->m_Filename), "empty texture filename");
					Tf.Put(fs("\"%s\"\n", pTexture->m_Filename));
				}

				Tf.EndToken();
			}

			// ---------------------------------------------------------------
			// output stages
			if (pMaterialSet->m_pStages)
			{
				ASSERT(pMaterialSet->m_nStages);

				Tf.BeginToken(g_kIdStage, pMaterialSet->m_nStages);
				Tf.Put("; UvId   TextureId   TransformId   Flags   Alpha   ActiveDistance   Operation   BlendOp   UWrap   VWrap   UScale   VScale   UvModMode   UvModAnimMode   UvModOrigin   UvModScrollSpeed   UvModRotateSpeed   UvModAmplitude   UvModBias\n");

				for(s32 cStage = 0; cStage < pMaterialSet->m_nStages; cStage++)
				{
					stage	*pStage = &pMaterialSet->m_pStages[cStage];

					// output uv index
					Tf.Put(pStage->m_iUV);
					Tf.Put("\t");

					// output texture index
					Tf.Put(pStage->m_iTexture);
					Tf.Put("\t");

					// output transform index
					Tf.Put(pStage->m_iTransform);
					Tf.Put("\t");

					// output flags
					Tf.Put(pStage->m_Flags);
					Tf.Put("\t");

					// output alpha
					Tf.Put(fs("%.3f,\t", pStage->m_Alpha));

					// output active distance
					Tf.Put(fs("%.3f,\t", pStage->m_ActiveDistance));

					// output operation
					Tf.Put(fs("\"%s\",\t", OpTable[pStage->m_Op].pId));

					// output blend operation
					Tf.Put(fs("\"%s\",\t", BlendOpTable[pStage->m_BlendOp].pId));

					// output u and v wrap modes
					Tf.Put(fs("\"%s\", \"%s\",\t", WrapTable[pStage->m_WrapU].pId, WrapTable[pStage->m_WrapV].pId));

					// output uv scale
					Tf.Put(pStage->m_UvScale);

					// output uvmod
					Tf.Put(fs("\"%s\",\t", UvModTable[pStage->m_UvMod.m_Mode].pId));

					Tf.Put(fs("\"%s\",\t", UvModAnimTable[pStage->m_UvMod.m_AnimMode].pId));

					Tf.Put(pStage->m_UvMod.m_vOrigin);
					Tf.Put(pStage->m_UvMod.m_ScrollSpeed);
					Tf.Put(pStage->m_UvMod.m_RotateSpeed);
					Tf.Put(pStage->m_UvMod.m_Amplitude);
					Tf.Put(pStage->m_UvMod.m_Bias);

					Tf.Put("\n");
				}

				Tf.EndToken();
			}

			// ---------------------------------------------------------------
			// output materials

			if(pMaterialSet->m_pMaterials)
			{
				ASSERT(pMaterialSet->m_nMaterials);

				Tf.BeginToken(g_kIdMaterial, pMaterialSet->m_nMaterials);
				Tf.Put("; MaterialName   nStages   StageIds   Flags   Alpha   OverallAlphaMode\n");

				for(s32 cMaterial = 0; cMaterial < pMaterialSet->m_nMaterials; cMaterial++)
				{
					material	*pMaterial = &pMaterialSet->m_pMaterials[cMaterial];

					Tf.Put(fs("\"%s\",\t", pMaterial->m_Name));

					// output the stage ids
					Tf.Put(pMaterial->m_nStages);
					Tf.Put("\t");

					for(s32 cStage = 0; cStage < pMaterial->m_nStages; cStage++)
					{
						Tf.Put(pMaterial->m_iStages[cStage]);
					}

					// output flags
//					Tf.Put(fs("\"%s\"\n", (pMaterial->m_bDoubleSided ? "TRUE" : "FALSE")));
					Tf.Put(pMaterial->m_Flags);
					Tf.Put("\t");

					// output alpha
					Tf.Put(fs("%.3f,\t", pMaterial->m_Alpha));

					// output overall alpha mode
					Tf.Put(AlphaModeTable[pMaterial->m_OverallAlphaMode].pId);
					Tf.Put("\n");
				}

				Tf.EndToken();
			}

			// ---------------------------------------------------------------
			// output materials
			if (pMaterialSet->m_pTransforms)
			{
				ASSERT(pMaterialSet->m_nTransforms);

				Tf.BeginToken(g_kIdTransform, pMaterialSet->m_nTransforms);
				Tf.Put("; matrix4 by columns\n");

				for(s32 cTrans = 0; cTrans < pMaterialSet->m_nTransforms; cTrans++)
				{
					matrix4 &pMat = pMaterialSet->m_pTransforms[cTrans];

					Tf.Put(pMat.M[0][0]);	Tf.Put("\t");
					Tf.Put(pMat.M[0][1]);	Tf.Put("\t");
					Tf.Put(pMat.M[0][2]);	Tf.Put("\t");
					Tf.Put(pMat.M[0][3]);	Tf.Put("\n");
					Tf.Put(pMat.M[1][0]);	Tf.Put("\t");
					Tf.Put(pMat.M[1][1]);	Tf.Put("\t");
					Tf.Put(pMat.M[1][2]);	Tf.Put("\t");
					Tf.Put(pMat.M[1][3]);	Tf.Put("\n");
					Tf.Put(pMat.M[2][0]);	Tf.Put("\t");
					Tf.Put(pMat.M[2][1]);	Tf.Put("\t");
					Tf.Put(pMat.M[2][2]);	Tf.Put("\t");
					Tf.Put(pMat.M[2][3]);	Tf.Put("\n");
					Tf.Put(pMat.M[3][0]);	Tf.Put("\t");
					Tf.Put(pMat.M[3][1]);	Tf.Put("\t");
					Tf.Put(pMat.M[3][2]);	Tf.Put("\t");
					Tf.Put(pMat.M[3][3]);	Tf.Put("\n");
				}

				Tf.EndToken();
			}

			Tf.EndToken();
		}
	}

	// -----------------------------------------------------------------------

	Tf.EndToken();

	return TRUE;
}

//****************************************************************************
// save a bone and then recurse on any children

// iBone is the index of the bone
// iParent is the index of iBone's parent
// Indent is the current indent

void object::RecurseSaveHierarchy(CTextFile& Tf, s32& iBone, s32 iParent, s32 Indent) const
{
	for(s32 cChild = 0; cChild < m_pBones[iParent].m_nChildren; cChild++)
	{
		// output child count
		Tf.Put(fs("%d,\t", m_pBones[iBone].m_nChildren));

		// indent
		for(s32 cIndent = 0; cIndent < Indent; cIndent++)
		{
			Tf.Put(" ");
		}

		// output name
		Tf.Put(fs("\"%s\"\n", m_pBones[iBone].m_Name));

		// recurse on children
		iBone++;
		RecurseSaveHierarchy(Tf, iBone, (iBone - 1), (Indent + 1));
	}
}

//****************************************************************************
// save object chunks

void object::SaveChunks(CTextFile& Tf, fnTickCallback fnCallback) const
{
	s32	cVert;

	for(s32 cChunk = 0; cChunk < m_nChunks; cChunk++)
	{
		chunk *pChunk = &m_pChunks[cChunk];

		Tf.BeginToken(g_kIdChunk, pChunk->m_Name);


		// -------------------------------------------------------------------
		// output verts
		if (pChunk->m_pVerts)
		{
			Tf.BeginToken(g_kIdVertex, pChunk->m_nVerts);
			Tf.Put("; x   y   z\n");

			for(cVert = 0; cVert < pChunk->m_nVerts; cVert++)
			{
				if (fnCallback && (cVert & 100))
				{
					fnCallback();
				}

				Tf.Put(fs("%.*f, %.*f, %.*f,\n",
							g_kVertsPrecision, pChunk->m_pVerts[cVert].m_vPos.X, 
							g_kVertsPrecision, pChunk->m_pVerts[cVert].m_vPos.Y, 
							g_kVertsPrecision, pChunk->m_pVerts[cVert].m_vPos.Z));
			}

			Tf.EndToken();
		}

		// -------------------------------------------------------------------
		// output normals
		if (pChunk->m_pVerts)
		{
			Tf.BeginToken(g_kIdNormal, pChunk->m_nVerts);
			Tf.Put("; x   y   z\n");

			for(cVert = 0; cVert < pChunk->m_nVerts; cVert++)
			{
				if (fnCallback && (cVert & 100))
				{
					fnCallback();
				}

				if (!_finite(pChunk->m_pVerts[cVert].m_vNormal.X))
				{
					pChunk->m_pVerts[cVert].m_vNormal.X = 0.f;
				}
				if (!_finite(pChunk->m_pVerts[cVert].m_vNormal.Y))
				{
					pChunk->m_pVerts[cVert].m_vNormal.Y = 0.f;
				}
				if (!_finite(pChunk->m_pVerts[cVert].m_vNormal.Z))
				{
					pChunk->m_pVerts[cVert].m_vNormal.Z = 0.f;
				}

				Tf.Put(fs("%.*f, %.*f, %.*f,\n",
							g_kNormalsPrecision, pChunk->m_pVerts[cVert].m_vNormal.X, 
							g_kNormalsPrecision, pChunk->m_pVerts[cVert].m_vNormal.Y, 
							g_kNormalsPrecision, pChunk->m_pVerts[cVert].m_vNormal.Z));
			}

			Tf.EndToken();
		}

		// -------------------------------------------------------------------
		// output colors
		if (pChunk->m_pVerts)
		{
			const vector4 white (1.0f, 1.0f, 1.0f, 1.0f);
			s32	nColors = 0;

			// count the assigned colors (if they're all white we don't bother outputting the color block)
			for(cVert = 0; cVert < pChunk->m_nVerts; cVert++)
			{
				if (pChunk->m_pVerts[cVert].m_Color != white)
				{
					nColors++;
				}
			}

			// we have some colors !
			if (nColors != 0)
			{
				Tf.BeginToken(g_kIdColor, pChunk->m_nVerts);
				Tf.Put("; r   g   b   a\n");

				for(cVert = 0; cVert < pChunk->m_nVerts; cVert++)
				{
					if (fnCallback && (cVert & 100))
					{
						fnCallback();
					}

//					// if the color is unassigned force it to 1
//					if (pChunk->m_pVerts[cVert].m_Color == vector4(-1.0f, -1.0f, -1.0f, -1.0f))
//					{
//						Tf.Put(vector4(1.0f, 1.0f, 1.0f, 1.0f));
//					}
//					else
					{
						Tf.Put(pChunk->m_pVerts[cVert].m_Color);
					}

					Tf.Put("\n");
				}

				Tf.EndToken();
			}
		}

		// -------------------------------------------------------------------
		// output uvs
		if (pChunk->m_pVerts)
		{
			s32	nStages = 0;

			// count the number of verts with uvs
			for(cVert = 0; cVert < pChunk->m_nVerts; cVert++)
			{
				nStages += pChunk->m_pVerts[cVert].m_nStages;
			}

			// we have uvs - output them
			if (nStages)
			{
				Tf.BeginToken(g_kIdUvMap, nStages);
				Tf.Put("; VIndex   StageId   U   V\n");

				// output the uv for each pass
				for(cVert = 0; cVert < pChunk->m_nVerts; cVert++)
				{
					if (fnCallback && (cVert & 100))
					{
						fnCallback();
					}

					for(s32 cStage = 0; cStage < pChunk->m_pVerts[cVert].m_nStages; cStage++)
					{
						Tf.Put(fs("%d,\t%d,\t", cVert, cStage));
						Tf.Put(fs("%.*f, %.*f,\n",
									g_kUVsPrecision, pChunk->m_pVerts[cVert].m_UVs[cStage].X,
									g_kUVsPrecision, pChunk->m_pVerts[cVert].m_UVs[cStage].Y));
					}
				}

				Tf.EndToken();
			}
		}

		// -------------------------------------------------------------------
		// output weights
		if (pChunk->m_pVerts)
		{
			// see if we have any weights
			for(cVert = 0; cVert < pChunk->m_nVerts; cVert++)
			{
				if (pChunk->m_pVerts[cVert].m_nWeights)
				{
					break;
				}
			}

			// we have weights - output them
			if (cVert < pChunk->m_nVerts)
			{
				Tf.BeginToken(g_kIdWeight, pChunk->m_nVerts);
				Tf.Put("; NMatrices   MIndex   %   ...\n");

				for(cVert = 0; cVert < pChunk->m_nVerts; cVert++)
				{
					if (fnCallback && (cVert & 100))
					{
						fnCallback();
					}

					ASSERT(pChunk->m_pVerts[cVert].m_nWeights);
					Tf.Put(fs("%d,\t", pChunk->m_pVerts[cVert].m_nWeights));

					for(s32 cWeight = 0; cWeight < pChunk->m_pVerts[cVert].m_nWeights; cWeight++)
					{
						Tf.Put(fs("%d,\t%.*f",
									pChunk->m_pVerts[cVert].m_Weights[cWeight].m_iMatrix,
									g_kWeightsPrecision, pChunk->m_pVerts[cVert].m_Weights[cWeight].m_Weight));

						if (cWeight < (pChunk->m_pVerts[cVert].m_nWeights - 1))
						{
							Tf.Put(",\t");
						}
					}

					Tf.Put("\n");
				}

				Tf.EndToken();
			}
		}

		// -------------------------------------------------------------------
		// output faces
		if (pChunk->m_pFaces)
		{
			face	*pTris = NULL;
			face	*pQuads = NULL;
			s32		nTris = 0;
			s32		nQuads = 0;
			s32		cFace;
			s32		cTri;
			s32		cQuad;
			s32		iTri;
			s32		iQuad;


			// count the number of quads and tris
			for(cFace = 0; cFace < pChunk->m_nFaces; cFace++)
			{
				if (pChunk->m_pFaces[cFace].m_bQuad)
				{
					nQuads++;
				}
				else
				{
					nTris++;
				}
			}


			// extract the tris
			if (nTris)
			{
				pTris = new face [nTris];

				iTri = 0;
				for(cFace = 0; cFace < pChunk->m_nFaces; cFace++)
				{
					if (pChunk->m_pFaces[cFace].m_bQuad == FALSE)
					{
						pTris[iTri++] = pChunk->m_pFaces[cFace];
					}
				}

				ASSERT(iTri == nTris);


				// output the tris
				Tf.BeginToken(g_kIdTri, nTris);
				Tf.Put("; VIndex   VIndex   VIndex\tMaterialId\tSmoothingGroup\tFlags\n");

				for(cTri = 0; cTri < nTris; cTri++)
				{
					if (fnCallback && (cTri & 100))
					{
						fnCallback();
					}

					Tf.Put(pTris[cTri].m_Index[0]);
					Tf.Put(pTris[cTri].m_Index[1]);
					Tf.Put(pTris[cTri].m_Index[2]);
					Tf.Put("\t");

					Tf.Put(pTris[cTri].m_iMaterial);
					Tf.Put("\t");

					Tf.Put(pTris[cTri].m_SmoothingGroup);
					Tf.Put("\t");

					Tf.Put(pTris[cTri].m_Flags);
					Tf.Put("\n");
				}

				Tf.EndToken();
			}


			// extract the quads
			if (nQuads)
			{
				pQuads = new face [nQuads];

				iQuad = 0;
				for(cFace = 0; cFace < pChunk->m_nFaces; cFace++)
				{
					if (pChunk->m_pFaces[cFace].m_bQuad == TRUE)
					{
						pQuads[iQuad++] = pChunk->m_pFaces[cFace];
					}
				}

				ASSERT(iQuad == nQuads);


				// output the quads
				Tf.BeginToken(g_kIdQuad, nQuads);
				Tf.Put("; VIndex   VIndex   VIndex   VIndex   MaterialIndex   SmoothingGroup\n");

				for(cQuad = 0; cQuad < nQuads; cQuad++)
				{
					if (fnCallback && (cQuad & 100))
					{
						fnCallback();
					}

					Tf.Put(pQuads[cQuad].m_Index[0]);
					Tf.Put(pQuads[cQuad].m_Index[1]);
					Tf.Put(pQuads[cQuad].m_Index[2]);
					Tf.Put(pQuads[cQuad].m_Index[3]);
					Tf.Put(",\t");

					Tf.Put(pQuads[cQuad].m_iMaterial);
					Tf.Put("\t");

					Tf.Put(pQuads[cQuad].m_SmoothingGroup);
					Tf.Put("\n");
				}

				Tf.EndToken();
			}


			delete [] pTris;
			pTris = NULL;

			delete [] pQuads;
			pQuads = NULL;
		}

		// -------------------------------------------------------------------
		Tf.EndToken();
	}
}

//****************************************************************************
// load an object

xbool object::Load(CTextFile& Tf, fnTickCallback fnCallback)
{
	CTextFile::tokentype	Token;
	xbool					bFound = FALSE;
	xbool					bSuccess = FALSE;


	ASSERT(m_pChunks == NULL);
	ASSERT(m_pMaterialSets == NULL);

	Tf.SeekStart();
	while(Tf.GetToken(&Token))
	{
		// -------------------------------------------------------------------
		// read datestamp
		if (Token == g_kIdDateStamp)
		{
			m_nDateStamp = Token.Int;
		}
		else
		// -------------------------------------------------------------------
		// read mesh
		if (Token == g_kIdMesh)
		{
			ASSERTS((bFound == FALSE), "file contains more than one mesh block");
			bFound = TRUE;

			// get the object name
			x_strncpy(m_Name, Token.String, sizeof(m_Name));


			// get version
			Tf.MarkSection();
			Tf.GetSectionToken(&Token);

			if (Token != g_kIdVersion)
			{
				m_Version = 1.0f;
			}
			else
			{
				ASSERT(Token.NumberType == TF_NUMBER_FLOAT);
				m_Version = Token.Float;

				Tf.UnmarkSection();
			}

			if (m_Version == 1.0f)
			{
				bSuccess = Load_v1_0(Tf, Token, fnCallback);
			}
			else if (m_Version == 2.0f)
			{
				bSuccess = Load_v2_0_v3_0(Tf, 0, fnCallback);
			}
			else if (m_Version == 3.0f)
			{
				bSuccess = Load_v2_0_v3_0(Tf, 1, fnCallback);
			}
			else if (m_Version == 3.1f)
			{
				bSuccess = Load_v2_0_v3_0(Tf, 2, fnCallback);
			}
			else
			{
				ASSERTS(FALSE, "trying to load unsupported version");
				x_printf("ERROR: trying to load unsupported version\n");
			}
		}
		// -------------------------------------------------------------------
		// skip any other data
		else
		{
			Tf.SkipSection();
		}
	}


	// promote to the latest version
	m_Version = g_kCurrentSaveVersion;


	if (bSuccess)
	{
		bSuccess = FixInvalidData();
	}

//	UpdateOverallAlphaModes();
//	{
//		CTextFile file;
//		file.CreateTextFile("c:\\test.mtf");
//		Save(file);
//	}

	return bSuccess;
}

//****************************************************************************
// load v1.0 mesh
// note: the major difference is a change from sub_material to stage

xbool object::Load_v1_0(CTextFile& Tf, CTextFile::tokentype FirstToken, fnTickCallback fnCallback)
{
	//UNREF(fnCallback);

	CTextFile::tokentype	Token;
	s32						iChunk = 0;

	xbool					bHasTextures = FALSE;
	xbool					bHasSubMaterials = FALSE;
	xbool					bHasMaterials = FALSE;


	Token = FirstToken;
	do
	{
		// -----------------------------------------------------------
		// read hierarchy
		if (Token == g_kIdHierarchy)
		{
			m_nBones = Token.Count;
			ASSERT(m_nBones);

			m_pBones = new bone [m_nBones];

			for(s32 cBone = 0; cBone < m_nBones; cBone++)
			{
				Tf.Get(&m_pBones[cBone].m_nChildren);
				Tf.Get(m_pBones[cBone].m_Name);
			}
		}
		// -----------------------------------------------------------
		// read chunk count
		else if (Token == g_kIdNumChunks)
		{
			ASSERTS((m_pChunks == NULL), "mesh contains more than one chunk block");

			m_nChunks = Token.Int;
			ASSERT(Token.NumberType == TF_NUMBER_INT);

			ASSERT(m_nChunks);
			m_pChunks = new chunk [m_nChunks];

			iChunk = 0;
		}
		// -----------------------------------------------------------
		// read chunks
		else if (Token == g_kIdChunk)
		{
			// get the chunk name
			ASSERT(m_pChunks);
			x_strncpy(m_pChunks[iChunk].m_Name, Token.String, sizeof(m_pChunks[iChunk].m_Name));

			LoadChunk_v1_0(Tf, iChunk);
			iChunk++;
		}
		// -------------------------------------------------------
		// read textures
		else if (Token == g_kIdTexture)
		{
			ASSERTS((m_pMaterialSets == NULL), "mesh contains more than one texture block");

			bHasTextures = TRUE;

			m_nMaterialSets = 1;
			m_pMaterialSets = new material_set [m_nMaterialSets];


			material_set	*pMaterialSet = &m_pMaterialSets[0];

			pMaterialSet->m_Target = MESH_TARGET_GENERIC;
			pMaterialSet->m_nTextures = Token.Count;
			ASSERT(pMaterialSet->m_nTextures);

			pMaterialSet->m_pTextures = new texture [pMaterialSet->m_nTextures];

			for(s32 cTexture = 0; cTexture < pMaterialSet->m_nTextures; cTexture++)
			{
				Tf.Get(&pMaterialSet->m_pTextures[cTexture].m_Width);
				Tf.Get(&pMaterialSet->m_pTextures[cTexture].m_Height);

				Tf.Get(pMaterialSet->m_pTextures[cTexture].m_Filename);
			}
		}
		// -------------------------------------------------------
		// read sub-materials
		else if (Token == g_kIdSubmaterial_v1_0)
		{
			bHasSubMaterials = TRUE;
			if (!m_pMaterialSets)
			{
				// there are no textures in this material
				m_nMaterialSets = 1;
				m_pMaterialSets = new material_set [m_nMaterialSets];


				m_pMaterialSets[0].m_Target = MESH_TARGET_GENERIC;
				m_pMaterialSets[0].m_nTextures = 0;
			}

			material_set	*pMaterialSet = &m_pMaterialSets[0];

			ASSERTS((pMaterialSet->m_pStages == NULL), "mesh contains more than one sub_material block");

			pMaterialSet->m_nStages = Token.Count;
			ASSERT(pMaterialSet->m_nStages);

			pMaterialSet->m_pStages = new stage [pMaterialSet->m_nStages];

			for(s32 cStage = 0; cStage < pMaterialSet->m_nStages; cStage++)
			{
				stage	*pStage = &pMaterialSet->m_pStages[cStage];
				char	Str[g_kMaxStrLen];
				s32		cWrap;
				s32		cOp;


				// get wrap u
				Tf.Get(Str);
				for(cWrap = 0; cWrap < ARRAY_LENGTH(WrapTable); cWrap++)
				{
					if (x_strcmp(Str, WrapTable[cWrap].pId) == 0)
						break;
				}

				ASSERT(cWrap < ARRAY_LENGTH(WrapTable));
				pStage->m_WrapU = WrapTable[cWrap].Mode;


				// get wrap v
				Tf.Get(Str);
				for(cWrap = 0; cWrap < ARRAY_LENGTH(WrapTable); cWrap++)
				{
					if (x_strcmp(Str, WrapTable[cWrap].pId) == 0)
						break;
				}

				ASSERT(cWrap < ARRAY_LENGTH(WrapTable));
				pStage->m_WrapV = WrapTable[cWrap].Mode;


				// get texture id
				Tf.Get(&pStage->m_iTexture);

				// get alpha
				Tf.Get(&pStage->m_Alpha);

				// get operation
				Tf.Get(Str);
				for(cOp = 0; cOp < ARRAY_LENGTH(OpTable); cOp++)
				{
					if (x_strcmp(Str, OpTable_v1_0[cOp].pId) == 0)
						break;
				}

				ASSERT(cOp < ARRAY_LENGTH(OpTable_v1_0));
				pStage->m_Op = OpTable_v1_0[cOp].Op;
			}
		}
		// -------------------------------------------------------
		// read materials
		else if (Token == g_kIdMaterial)
		{
			ASSERTS(m_pMaterialSets, "material block found before texture block");
			ASSERTS(m_pMaterialSets->m_pStages, "material block found before texture block");
			bHasMaterials = TRUE;

			material_set	*pMaterialSet = &m_pMaterialSets[0];

			ASSERTS((pMaterialSet->m_pMaterials == NULL), "mesh contains more than one material block");

			pMaterialSet->m_nMaterials = Token.Count;
			ASSERT(pMaterialSet->m_nMaterials);

			pMaterialSet->m_pMaterials = new material [pMaterialSet->m_nMaterials];

			for(s32 cMaterial = 0; cMaterial < pMaterialSet->m_nMaterials; cMaterial++)
			{
				material	*pMaterial = &pMaterialSet->m_pMaterials[cMaterial];
				s32			cPass;

				// get name
				Tf.Get(pMaterial->m_Name);

				// get pass count
				Tf.Get(&pMaterial->m_nStages);

				// get sub-material indices
				for(cPass = 0; cPass < pMaterial->m_nStages; cPass++)
				{
					Tf.Get(&pMaterial->m_iStages[cPass]);

					if (pMaterial->m_iStages[cPass] == -1) {
						x_printf("ERROR: material '%s' has texture stage = -1\n", pMaterial->m_Name);
						return FALSE;
					}
				}

				// get uv-pass indices (note: these are now stored in the stage - not the material)
				for(cPass = 0; cPass < pMaterial->m_nStages; cPass++)
				{
					//s32 iStage = pMaterial->m_iStages[cPass];

					Tf.Get(&pMaterialSet->m_pStages[cPass].m_iUV);
				}
			}
		}
		// -----------------------------------------------------------
		// skip any other data
		else
		{
			Tf.SkipSection();
		}
	}
	while(Tf.GetSectionToken(&Token));

	Tf.UnmarkSection();


	ASSERT(iChunk == m_nChunks);

	s32 count = bHasTextures + bHasSubMaterials + bHasMaterials;
	if (count < 0 || count > 3)
		return FALSE;

	return TRUE;
}

//****************************************************************************
// load a v1.0 chunk

xbool object::LoadChunk_v1_0(CTextFile& Tf, s32 iChunk)
{
	CTextFile::tokentype	Token;
	chunk					*pChunk = NULL;
	face					*pTris = NULL;
	face					*pQuads = NULL;
	face					*pTri = NULL;
	face					*pQuad = NULL;
	xbool					bColors = FALSE;
	s32						nTris = 0,
							nQuads = 0;
	s32						cVert;


	ASSERT(m_pChunks);
	ASSERT(m_nChunks);
	ASSERT((iChunk >= 0) && (iChunk < m_nChunks));


	pChunk = &m_pChunks[iChunk];


	Tf.MarkSection();
	while(Tf.GetSectionToken(&Token))
	{
		// -------------------------------------------------------------------
		// read verts
		if (Token == g_kIdVertex)
		{
			// alloc the verts if we don't have them yet
			if (pChunk->m_pVerts == NULL)
			{
				pChunk->m_nVerts = Token.Count;
				ASSERT(pChunk->m_nVerts);

				pChunk->m_pVerts = new vertex [pChunk->m_nVerts];
			}

			ASSERT(Token.Count == pChunk->m_nVerts);

			// get the verts
			for(cVert = 0; cVert < pChunk->m_nVerts; cVert++)
			{
				Tf.Get(&pChunk->m_pVerts[cVert].m_vPos);

				pChunk->m_pVerts[cVert].m_vRenderPos = pChunk->m_pVerts[cVert].m_vPos;
			}
		}

		// -------------------------------------------------------------------
		// read normals
		else if (Token == g_kIdNormal)
		{
			// alloc the verts if we don't have them yet
			if (pChunk->m_pVerts == NULL)
			{
				pChunk->m_nVerts = Token.Count;
				ASSERT(pChunk->m_nVerts);

				pChunk->m_pVerts = new vertex [pChunk->m_nVerts];
			}

			ASSERT(Token.Count == pChunk->m_nVerts);


			// get the normals
			for(cVert = 0; cVert < pChunk->m_nVerts; cVert++)
			{
				Tf.Get(&pChunk->m_pVerts[cVert].m_vNormal);
			}
		}

		// ---------------------------------------------------------------
		// read colors
		else if (Token == g_kIdColor)
		{
			// alloc the verts if we don't have them yet
			if (pChunk->m_pVerts == NULL)
			{
				pChunk->m_nVerts = Token.Count;
				ASSERT(pChunk->m_nVerts);

				pChunk->m_pVerts = new vertex [pChunk->m_nVerts];
			}

			ASSERT(Token.Count == pChunk->m_nVerts);

			// get the colors
			for(cVert = 0; cVert < pChunk->m_nVerts; cVert++)
			{
				Tf.Get(&pChunk->m_pVerts[cVert].m_Color);
			}

			bColors = TRUE;
		}

		// ---------------------------------------------------------------
		// read uvs
		else if (Token == g_kIdUvMap)
		{
			// alloc the verts if we don't have them yet
			if (pChunk->m_pVerts == NULL)
			{
				pChunk->m_nVerts = Token.Count;
				ASSERT(pChunk->m_nVerts);

				pChunk->m_pVerts = new vertex [pChunk->m_nVerts];
			}

			s32	nUVs = Token.Count;
			ASSERT(nUVs);

			// get the uvs
			for(s32 cUV = 0; cUV < nUVs; cUV++)
			{
				vertex	*pVert;
				s32		iVert,
						Pass;

				// get vert index
				Tf.Get(&iVert);
				ASSERT((iVert >= 0) && (iVert < pChunk->m_nVerts));

				// get pass
				Tf.Get(&Pass);
				ASSERT((Pass >= 0) && (Pass < g_kMaxNumPasses_v1_0));

				pVert = &pChunk->m_pVerts[iVert];
				ASSERT(pVert->m_nStages < g_kMaxNumPasses_v1_0);

				// get the uv
				Tf.Get(&pVert->m_UVs[Pass]);

				pVert->m_nStages++;
			}
		}

		// ---------------------------------------------------------------
		// read vertex weights
		else if (Token == g_kIdWeight)
		{
			// alloc the verts if we don't have them yet
			if (pChunk->m_pVerts == NULL)
			{
				pChunk->m_nVerts = Token.Count;
				ASSERT(pChunk->m_nVerts);

				pChunk->m_pVerts = new vertex [pChunk->m_nVerts];
			}

			ASSERT(Token.Count == pChunk->m_nVerts);


			for(s32 cVert = 0; cVert < pChunk->m_nVerts; cVert++)
			{
				s32	nWeights;


				Tf.Get(&nWeights);
				ASSERT(nWeights <= kMaxNumWeights);


				pChunk->m_pVerts[cVert].m_nWeights = nWeights;

				for(s32 cWeight = 0; cWeight < nWeights; cWeight++)
				{
					Tf.Get(&pChunk->m_pVerts[cVert].m_Weights[cWeight].m_iMatrix);
					ASSERT(pChunk->m_pVerts[cVert].m_Weights[cWeight].m_iMatrix < m_nBones);

					Tf.Get(&pChunk->m_pVerts[cVert].m_Weights[cWeight].m_Weight);
				}
			}
		}

		// ---------------------------------------------------------------
		// read tri-faces
		else if (Token == g_kIdTri)
		{
			ASSERTS((pTris == NULL), "mesh contains more than one tri block");

			nTris = Token.Count;
			ASSERT(nTris);

			pTris = new face [nTris];

			// get the tris
			for(s32 cTri = 0; cTri < nTris; cTri++)
			{
				pTri = &pTris[cTri];

				pTri->m_bQuad = FALSE;

				Tf.Get(&pTri->m_Index[0]);
				Tf.Get(&pTri->m_Index[1]);
				Tf.Get(&pTri->m_Index[2]);

				ASSERT(pTri->m_Index[0] < pChunk->m_nVerts);
				ASSERT(pTri->m_Index[1] < pChunk->m_nVerts);
				ASSERT(pTri->m_Index[2] < pChunk->m_nVerts);

				Tf.Get(&pTri->m_iMaterial);
			}
		}

		// ---------------------------------------------------------------
		// read quad-faces
		else if (Token == g_kIdQuad)
		{
			ASSERTS((pQuads == NULL), "mesh contains more than one quad block");

			nQuads = Token.Count;
			ASSERT(nQuads);

			pQuads = new face [nQuads];

			// get the quads
			for(s32 cQuad = 0; cQuad < nQuads; cQuad++)
			{
				pQuad = &pQuads[cQuad];

				pQuad->m_bQuad = TRUE;

				Tf.Get(&pQuad->m_Index[0]);
				Tf.Get(&pQuad->m_Index[1]);
				Tf.Get(&pQuad->m_Index[2]);
				Tf.Get(&pQuad->m_Index[3]);

				ASSERT(pQuad->m_Index[0] < pChunk->m_nVerts);
				ASSERT(pQuad->m_Index[1] < pChunk->m_nVerts);
				ASSERT(pQuad->m_Index[2] < pChunk->m_nVerts);
				ASSERT(pQuad->m_Index[3] < pChunk->m_nVerts);

				Tf.Get(&pQuad->m_iMaterial);
			}
		}
		// ---------------------------------------------------------------
		// skip any other data
		else
		{
			Tf.SkipSection();
		}
	}
    Tf.UnmarkSection();



	// -------------------------------------------------------------------
	// verts have no assigned colors - force to white
	if (bColors == FALSE)
	{
		for(cVert = 0; cVert < pChunk->m_nVerts; cVert++)
		{
			pChunk->m_pVerts[cVert].m_Color = vector4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}



	// -------------------------------------------------------------------
	// merge quads and tris into one list of facets
	if ((pTris) || (pQuads))
	{
		s32	iFace = 0;

		pChunk->m_nFaces = (nTris + nQuads);
		pChunk->m_pFaces = new face [pChunk->m_nFaces];

		for(s32 cTri = 0; cTri < nTris; cTri++)
		{
			pChunk->m_pFaces[iFace++] = pTris[cTri];
		}

		for(s32 cQuad = 0; cQuad < nQuads; cQuad++)
		{
			pChunk->m_pFaces[iFace++] = pQuads[cQuad];
		}

		ASSERT(iFace == pChunk->m_nFaces);

		// free the separate tris and quads pool
		delete [] pTris;
		pTris = NULL;

		delete [] pQuads;
		pQuads = NULL;
	}

	return TRUE;
}

//****************************************************************************
//****************************************************************************
//****************************************************************************
// load v2.0 mesh

xbool object::Load_v2_0_v3_0(CTextFile& Tf, s32 nVariation, fnTickCallback fnCallback)
{
	//UNREF(fnCallback);

	CTextFile::tokentype	Token;
	s32						iChunk = 0;
	s32						iMaterialSet = 0;


	Tf.MarkSection();
	while(Tf.GetSectionToken(&Token))
	{
		// -----------------------------------------------------------
		// read userdata
		if (Token == g_kIdUserData)
		{
			u32	nUserData = Token.Count;
			ASSERT(nUserData);

			m_UserDataList.reserve(nUserData);

			for(u32 cUserData = 0; cUserData < nUserData; cUserData++)
			{
				t_UserData	UserData;

				Tf.Get(&UserData.m_Id);
				Tf.Get(&UserData.m_Value);

				m_UserDataList.push_back(UserData);
			}
		}

		// -----------------------------------------------------------
		// read hierarchy
		else if (Token == g_kIdHierarchy)
		{
			m_nBones = Token.Count;
			ASSERT(m_nBones);

			m_pBones = new bone [m_nBones];

			for(s32 cBone = 0; cBone < m_nBones; cBone++)
			{
				Tf.Get(&m_pBones[cBone].m_nChildren);
				Tf.Get(m_pBones[cBone].m_Name);
			}
		}

		// -----------------------------------------------------------
		// read chunk count
		else if (Token == g_kIdNumChunks)
		{
			ASSERTS((m_pChunks == NULL), "mesh contains more than one chunk block");

			m_nChunks = Token.Int;
//j: should assert on the type of the number here

			ASSERT(m_nChunks);
			m_pChunks = new chunk [m_nChunks];

			iChunk = 0;
		}
		// -----------------------------------------------------------
		// read chunks
		else if (Token == g_kIdChunk)
		{
			// get the chunk name
			ASSERT(m_pChunks);
			x_strncpy(m_pChunks[iChunk].m_Name, Token.String, sizeof(m_pChunks[iChunk].m_Name));

			if (!(LoadChunk_v2_0_v3_0(Tf, iChunk, nVariation)))
			{
				return FALSE;
			}

			iChunk++;
		}
		// -----------------------------------------------------------
		// read num material sets
		else if (Token == g_kIdNumMaterialSets)
		{
			ASSERTS((m_pMaterialSets == NULL), "mesh contains more than one num materials block");

			m_nMaterialSets = Token.Int;
			ASSERT(Token.NumberType == TF_NUMBER_INT);

			ASSERT(m_nMaterialSets);
			m_pMaterialSets = new material_set [m_nMaterialSets];

			iMaterialSet = 0;
		}
		// -----------------------------------------------------------
		// read material sets
		else if (Token == g_kIdMaterialSet)
		{
			ASSERT((iMaterialSet >= 0) && (iMaterialSet < m_nMaterialSets));
			material_set	*pMaterialSet = &m_pMaterialSets[iMaterialSet];

			// set target
			for(s32 cTarget = 0; cTarget < ARRAY_LENGTH(TargetTable); cTarget++)
			{
				if (x_strcmp(Token.String, TargetTable[cTarget].pId) == 0)
					break;
			}
			
			ASSERT(cTarget < ARRAY_LENGTH(TargetTable));
			pMaterialSet->m_Target = TargetTable[cTarget].Target;

			if (nVariation == 0)
			{
				if (!(LoadMaterialSet_v2_0(Tf, pMaterialSet)))
				{
					return FALSE;
				}
			}
			else
			{
				// Variations 1&2 both use the standard v3.1 materials
				if (!(LoadMaterialSet_v3_0(Tf, pMaterialSet)))
				{
					return FALSE;
				}
			}

			iMaterialSet++;
		}
		// -----------------------------------------------------------
		// skip any other data
		else
		{
			Tf.SkipSection();
		}
	}
	Tf.UnmarkSection();


	ASSERT(iChunk == m_nChunks);

	return TRUE;
}

//****************************************************************************
// load a v3.0 chunk

xbool object::LoadChunk_v2_0_v3_0(CTextFile& Tf, s32 iChunk, s32 nVariation)
{
	CTextFile::tokentype	Token;
	chunk					*pChunk = NULL;
	face					*pTris = NULL;
	face					*pQuads = NULL;
	face					*pTri = NULL;
	face					*pQuad = NULL;
	xbool					bColors = FALSE;
	s32						nTris = 0;
	s32						nQuads = 0;
	s32						cVert;


	ASSERT(m_pChunks);
	ASSERT(m_nChunks);
	ASSERT((iChunk >= 0) && (iChunk < m_nChunks));

	pChunk = &m_pChunks[iChunk];

	Tf.MarkSection();
	while(Tf.GetSectionToken(&Token))
	{
		// -------------------------------------------------------------------
		// read verts
		if (Token == g_kIdVertex)
		{
			// alloc the verts if we don't have them yet
			if (pChunk->m_pVerts == NULL)
			{
				pChunk->m_nVerts = Token.Count;
				ASSERT(pChunk->m_nVerts);

				pChunk->m_pVerts = new vertex [pChunk->m_nVerts];
			}

			ASSERT(Token.Count == pChunk->m_nVerts);

			// get the verts
			for(cVert = 0; cVert < pChunk->m_nVerts; cVert++)
			{
				xbool test = Tf.Get(&pChunk->m_pVerts[cVert].m_vPos);

				char temp_str[80];
				x_sprintf(temp_str, "Unable to read vertex %d\n", cVert);
				ASSERTS(test, temp_str);

				pChunk->m_pVerts[cVert].m_vRenderPos = pChunk->m_pVerts[cVert].m_vPos;
			}
		}

		// -------------------------------------------------------------------
		// read normals
		else if (Token == g_kIdNormal)
		{
			// alloc the verts if we don't have them yet
			if (pChunk->m_pVerts == NULL)
			{
				pChunk->m_nVerts = Token.Count;
				ASSERT(pChunk->m_nVerts);

				pChunk->m_pVerts = new vertex [pChunk->m_nVerts];
			}

			ASSERT(Token.Count == pChunk->m_nVerts);


			// get the normals
			for(cVert = 0; cVert < pChunk->m_nVerts; cVert++)
			{
				xbool test = Tf.Get(&pChunk->m_pVerts[cVert].m_vNormal);

				char temp_str[80];
				x_sprintf(temp_str, "Unable to read normal for vert %d\n", cVert);
				ASSERTS(test, temp_str);
			}
		}

		// ---------------------------------------------------------------
		// read colors
		else if (Token == g_kIdColor)
		{
			// alloc the verts if we don't have them yet
			if (pChunk->m_pVerts == NULL)
			{
				pChunk->m_nVerts = Token.Count;
				ASSERT(pChunk->m_nVerts);

				pChunk->m_pVerts = new vertex [pChunk->m_nVerts];
			}

			ASSERT(Token.Count == pChunk->m_nVerts);

			// get the colors
			for(cVert = 0; cVert < pChunk->m_nVerts; cVert++)
			{
				xbool test = Tf.Get(&pChunk->m_pVerts[cVert].m_Color);

				char temp_str[80];
				x_sprintf(temp_str, "Unable to read color for vert %d\n", cVert);
				ASSERTS(test, temp_str);
			}

			bColors = TRUE;
		}

		// ---------------------------------------------------------------
		// read uvs
		else if (Token == g_kIdUvMap)
		{
			// alloc the verts if we don't have them yet
			if (pChunk->m_pVerts == NULL)
			{
				pChunk->m_nVerts = Token.Count;
				ASSERT(pChunk->m_nVerts);

				pChunk->m_pVerts = new vertex [pChunk->m_nVerts];
			}

			s32	nUVs = Token.Count;
			ASSERT(nUVs);

			// get the uvs
			for(s32 cUV = 0; cUV < nUVs; cUV++)
			{
				vertex	*pVert;
				s32		iVert;
				s32		iStage;

				// get vert index
				Tf.Get(&iVert);
				ASSERT((iVert >= 0) && (iVert < pChunk->m_nVerts));

				// get stage
				Tf.Get(&iStage);
				ASSERT((iStage >= 0) && (iStage < kMaxNumStages));

				pVert = &pChunk->m_pVerts[iVert];
				ASSERT(pVert->m_nStages < kMaxNumStages);

				// get the uv
				Tf.Get(&pVert->m_UVs[iStage]);

				pVert->m_nStages++;
			}
		}

		// ---------------------------------------------------------------
		// read vertex weights
		else if (Token == g_kIdWeight)
		{
			// alloc the verts if we don't have them yet
			if (pChunk->m_pVerts == NULL)
			{
				pChunk->m_nVerts = Token.Count;
				ASSERT(pChunk->m_nVerts);

				pChunk->m_pVerts = new vertex [pChunk->m_nVerts];
			}

			ASSERT(Token.Count == pChunk->m_nVerts);


			for(s32 cVert = 0; cVert < pChunk->m_nVerts; cVert++)
			{
				s32	nWeights;

				Tf.Get(&nWeights);
				ASSERT(nWeights <= kMaxNumWeights);

				pChunk->m_pVerts[cVert].m_nWeights = nWeights;

				for(s32 cWeight = 0; cWeight < nWeights; cWeight++)
				{
					// get the matrix index
					Tf.Get(&pChunk->m_pVerts[cVert].m_Weights[cWeight].m_iMatrix);
					ASSERT(pChunk->m_pVerts[cVert].m_Weights[cWeight].m_iMatrix < m_nBones);

					// get the weight
					Tf.Get(&pChunk->m_pVerts[cVert].m_Weights[cWeight].m_Weight);
				}
			}
		}

		// ---------------------------------------------------------------
		// read tri-faces
		else if (Token == g_kIdTri)
		{
			ASSERTS((pTris == NULL), "mesh contains more than one tri block");

			nTris = Token.Count;
			ASSERT(nTris);

			pTris = new face [nTris];

			// get the tris
			for(s32 cTri = 0; cTri < nTris; cTri++)
			{
				s32 count;
				pTri = &pTris[cTri];

				pTri->m_bQuad = FALSE;

				// get vert indices
				count = Tf.Get(&pTri->m_Index[0]);
				count += Tf.Get(&pTri->m_Index[1]);
				count += Tf.Get(&pTri->m_Index[2]);

				ASSERT(pTri->m_Index[0] < pChunk->m_nVerts);
				ASSERT(pTri->m_Index[1] < pChunk->m_nVerts);
				ASSERT(pTri->m_Index[2] < pChunk->m_nVerts);

				// get material index
				count += Tf.Get(&pTri->m_iMaterial);

				// get smoothing group (v3 or later)
				if (nVariation > 0)
				{
					count += Tf.Get(&pTri->m_SmoothingGroup);

					// get flags (v3.1 or later)
					if (nVariation > 1)
					{
						count += Tf.Get(&pTri->m_Flags);
					}
				}

				// force the smoothing group (this has to be done in v3 because the
				// artists have meshes with smoothing info when that wasn't really
				// what they meant - we ignore this in all versions except 3.1
				// (variation 2) and above
				if (nVariation < 2)
				{
					pTri->m_SmoothingGroup = 1;
				}

				// Do some parameter count checking
				switch (nVariation)
				{
					case 0:	if (count != 4) return FALSE; else break;
					case 1:	if (count != 5) return FALSE; else break;
					case 2:	if (count != 6) return FALSE; else break;
				}
			}
		}

		// ---------------------------------------------------------------
		// read quad-faces
		else if (Token == g_kIdQuad)
		{
			ASSERTS((pQuads == NULL), "mesh contains more than one quad block");

			nQuads = Token.Count;
			ASSERT(nQuads);

			pQuads = new face [nQuads];

			// get the quads
			for(s32 cQuad = 0; cQuad < nQuads; cQuad++)
			{
				pQuad = &pQuads[cQuad];

				pQuad->m_bQuad = TRUE;

				// get vert indices
				Tf.Get(&pQuad->m_Index[0]);
				Tf.Get(&pQuad->m_Index[1]);
				Tf.Get(&pQuad->m_Index[2]);
				Tf.Get(&pQuad->m_Index[3]);

				ASSERT(pQuad->m_Index[0] < pChunk->m_nVerts);
				ASSERT(pQuad->m_Index[1] < pChunk->m_nVerts);
				ASSERT(pQuad->m_Index[2] < pChunk->m_nVerts);
				ASSERT(pQuad->m_Index[3] < pChunk->m_nVerts);

				// get material index
				Tf.Get(&pQuad->m_iMaterial);

				// get smoothing group (v3 or later)
				if (nVariation > 0)
				{
					Tf.Get(&pQuad->m_SmoothingGroup);

					// get flags (v3.1 or later)
					if (nVariation > 1)
					{
						Tf.Get(&pTri->m_Flags);
					}
				}

				// force the smoothing group (this has to be done in v3 because the
				// artists have meshes with smoothing info when that watn't really
				// what they meant - we'll ignore it in all v3 meshes and start
				// using it in v4
				pTri->m_SmoothingGroup = 1;
			}
		}
		// ---------------------------------------------------------------
		// skip any other data
		else
		{
			Tf.SkipSection();
		}
	}

    Tf.UnmarkSection();



	// -------------------------------------------------------------------
	// verts have no assigned colors - force to white
	if (bColors == FALSE)
	{
		for(cVert = 0; cVert < pChunk->m_nVerts; cVert++)
		{
			pChunk->m_pVerts[cVert].m_Color = vector4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}



	// -------------------------------------------------------------------
	// merge quads and tris into one list of facets
	if ((pTris) || (pQuads))
	{
		s32	iFace = 0;

		pChunk->m_nFaces = (nTris + nQuads);
		pChunk->m_pFaces = new face [pChunk->m_nFaces];

		for(s32 cTri = 0; cTri < nTris; cTri++)
		{
			pChunk->m_pFaces[iFace++] = pTris[cTri];
		}

		for(s32 cQuad = 0; cQuad < nQuads; cQuad++)
		{
			pChunk->m_pFaces[iFace++] = pQuads[cQuad];
		}

		ASSERT(iFace == pChunk->m_nFaces);

		// free the separate tris and quads pool
		delete [] pTris;
		pTris = NULL;

		delete [] pQuads;
		pQuads = NULL;
	}

	return TRUE;
}

//****************************************************************************
// load a v2.0 material set

xbool object::LoadMaterialSet_v2_0(CTextFile& Tf, material_set *pMaterialSet)
{
	CTextFile::tokentype	Token;
	char					Str[g_kMaxStrLen];
	xbool					bHasTextures = FALSE;
	xbool					bHasSubMaterials = FALSE;
	xbool					bHasMaterials = FALSE;



	ASSERT(pMaterialSet);

	Tf.MarkSection();
	while(Tf.GetSectionToken(&Token))
	{
		// -----------------------------------------------------------
		// read textures
		if (Token == g_kIdTexture)
		{
			ASSERTS((pMaterialSet->m_pTextures == NULL), "mesh contains more than one texture block");
			bHasTextures = TRUE;

			pMaterialSet->m_nTextures = Token.Count;
			ASSERT(pMaterialSet->m_nTextures);

			pMaterialSet->m_pTextures = new texture [pMaterialSet->m_nTextures];

			for(s32 cTexture = 0; cTexture < pMaterialSet->m_nTextures; cTexture++)
			{
				texture	*pTexture = &pMaterialSet->m_pTextures[cTexture];

				s32 count;

				count = Tf.Get(&pTexture->m_Width);
				count += Tf.Get(&pTexture->m_Height);
				count += Tf.Get(&pTexture->m_Bpp);

				// get alpha mode
				count += Tf.Get(Str);
				for(s32 cMode = 0; cMode < ARRAY_LENGTH(AlphaModeTable); cMode++)
				{
					if (x_strcmp(Str, AlphaModeTable[cMode].pId) == 0)
					{
						break;
					}
				}

				ASSERT(cMode < ARRAY_LENGTH(AlphaModeTable));
				pTexture->m_AlphaMode = AlphaModeTable[cMode].Mode;

				count += Tf.Get(&pTexture->m_nFiles);
				count += Tf.Get(pTexture->m_Filename);

				if (count != 6)
				{
					return FALSE;
				}
			}
		}
		// -----------------------------------------------------------
		// read stages
		else if (Token == g_kIdStage)
		{
			ASSERTS((pMaterialSet->m_pStages == NULL), "mesh contains more than one stage block");
			bHasSubMaterials = TRUE;

			pMaterialSet->m_nStages = Token.Count;
			ASSERT(pMaterialSet->m_nStages);

			pMaterialSet->m_pStages = new stage [pMaterialSet->m_nStages];

			for(s32 cStage = 0; cStage < pMaterialSet->m_nStages; cStage++)
			{
				stage	*pStage = &pMaterialSet->m_pStages[cStage];
				s32		cOp;
				s32		cWrap;
				s32		cMode;

				s32		count;


				// get uv, texture, and transform indices
				count = Tf.Get(&pStage->m_iUV);
				count += Tf.Get(&pStage->m_iTexture);

				// get flags
				count += Tf.Get(&pStage->m_Flags);

				// get intensity
				count += Tf.Get(&pStage->m_Alpha);

				// get operation
				count += Tf.Get(Str);
				for(cOp = 0; cOp < ARRAY_LENGTH(OpTable); cOp++)
				{
					if (x_strcmp(Str, OpTable[cOp].pId) == 0)
					{
						break;
					}
				}

				ASSERT(cOp < ARRAY_LENGTH(OpTable));
				pStage->m_Op = OpTable[cOp].Op;


				// get blend operation
				count += Tf.Get(Str);
				for(cOp = 0; cOp < ARRAY_LENGTH(BlendOpTable); cOp++)
				{
					if (x_strcmp(Str, BlendOpTable[cOp].pId) == 0)
					{
						break;
					}
				}

				ASSERT(cOp < ARRAY_LENGTH(BlendOpTable));
				pStage->m_BlendOp = BlendOpTable[cOp].Op;


				// get wrap u
				count += Tf.Get(Str);
				for(cWrap = 0; cWrap < ARRAY_LENGTH(WrapTable); cWrap++)
				{
					if (x_strcmp(Str, WrapTable[cWrap].pId) == 0)
					{
						break;
					}
				}

				ASSERT(cWrap < ARRAY_LENGTH(WrapTable));
				pStage->m_WrapU = WrapTable[cWrap].Mode;


				// get wrap v
				count += Tf.Get(Str);
				for(cWrap = 0; cWrap < ARRAY_LENGTH(WrapTable); cWrap++)
				{
					if (x_strcmp(Str, WrapTable[cWrap].pId) == 0)
					{
						break;
					}
				}

				ASSERT(cWrap < ARRAY_LENGTH(WrapTable));
				pStage->m_WrapV = WrapTable[cWrap].Mode;


				// get uv scale
				count += Tf.Get(&pStage->m_UvScale);


				// get uvmod mode
				count += Tf.Get(Str);
				for(cMode = 0; cMode < ARRAY_LENGTH(UvModTable); cMode++)
				{
					if (x_strcmp(Str, UvModTable[cMode].pId) == 0)
					{
						break;
					}
				}

				ASSERT(cMode < ARRAY_LENGTH(UvModTable));
				pStage->m_UvMod.m_Mode = UvModTable[cMode].Mode;


				// get uvmod anim mode
				count += Tf.Get(Str);
				for(cMode = 0; cMode < ARRAY_LENGTH(UvModAnimTable); cMode++)
				{
					if (x_strcmp(Str, UvModAnimTable[cMode].pId) == 0)
					{
						break;
					}
				}

				ASSERT(cMode < ARRAY_LENGTH(UvModAnimTable));
				pStage->m_UvMod.m_AnimMode = UvModAnimTable[cMode].Mode;


				// get uvmod
				count += Tf.Get(&pStage->m_UvMod.m_vOrigin);
				count += Tf.Get(&pStage->m_UvMod.m_ScrollSpeed);
				count += Tf.Get(&pStage->m_UvMod.m_RotateSpeed);
				count += Tf.Get(&pStage->m_UvMod.m_Amplitude);
				count += Tf.Get(&pStage->m_UvMod.m_Bias);

				if (count != 16)
				{
					return FALSE;
				}
			}
		}
		// -----------------------------------------------------------
		// read materials
		else if (Token == g_kIdMaterial)
		{
			ASSERTS((pMaterialSet->m_pMaterials == NULL), "mesh contains more than one material block");
			bHasMaterials = TRUE;

			pMaterialSet->m_nMaterials = Token.Count;
			ASSERT(pMaterialSet->m_nMaterials);

			pMaterialSet->m_pMaterials = new material [pMaterialSet->m_nMaterials];

			for(s32 cMaterial = 0; cMaterial < pMaterialSet->m_nMaterials; cMaterial++)
			{
				material	*pMaterial = &pMaterialSet->m_pMaterials[cMaterial];
				s32			count;

				// get name
				count = Tf.Get(pMaterial->m_Name);

				// get num stages
				count += Tf.Get(&pMaterial->m_nStages);

				// get sub-material indices
				for(s32 cStage = 0; cStage < pMaterial->m_nStages; cStage++)
				{
					count += Tf.Get(&pMaterial->m_iStages[cStage]);

					if (pMaterial->m_iStages[cStage] == -1) {
						x_printf("ERROR: material '%s' has texture stage = -1\n", pMaterial->m_Name);
						return FALSE;
					}
				}

				// get flags
				count += Tf.Get(&pMaterial->m_Flags);

				// get overall alpha mode
				count += Tf.Get(Str);
				for(s32 cMode = 0; cMode < ARRAY_LENGTH(AlphaModeTable); cMode++)
				{
					if (x_strcmp(Str, AlphaModeTable[cMode].pId) == 0)
					{
						break;
					}
				}

				ASSERT(cMode < ARRAY_LENGTH(AlphaModeTable));
				pMaterial->m_OverallAlphaMode = AlphaModeTable[cMode].Mode;

				if (count != (4 + pMaterial->m_nStages))
				{
					return FALSE;
				}
			}
		}
		// ---------------------------------------------------------------
		// skip any other data
		else
		{
			Tf.SkipSection();
		}
	}
    Tf.UnmarkSection();

	s32 test = bHasTextures + bHasSubMaterials + bHasMaterials;
	if (test < 0 || test > 3)
	{
		return FALSE;
	}


	return TRUE;
}

//****************************************************************************
// load a v3.0 material set

xbool object::LoadMaterialSet_v3_0(CTextFile& Tf, material_set *pMaterialSet)
{
	CTextFile::tokentype	Token;
	char					Str[g_kMaxStrLen];


	ASSERT(pMaterialSet);

	Tf.MarkSection();
	while(Tf.GetSectionToken(&Token))
	{
		// -----------------------------------------------------------
		// read textures
		if (Token == g_kIdTexture)
		{
			ASSERTS((pMaterialSet->m_pTextures == NULL), "mesh contains more than one texture block");

			pMaterialSet->m_nTextures = Token.Count;
			ASSERT(pMaterialSet->m_nTextures);

			pMaterialSet->m_pTextures = new texture [pMaterialSet->m_nTextures];

			for(s32 cTexture = 0; cTexture < pMaterialSet->m_nTextures; cTexture++)
			{
				texture	*pTexture = &pMaterialSet->m_pTextures[cTexture];

				s32 count;

				count = Tf.Get(&pTexture->m_Width);
				count += Tf.Get(&pTexture->m_Height);
				count += Tf.Get(&pTexture->m_Bpp);

				// get alpha mode
				count += Tf.Get(Str);
				for(s32 cMode = 0; cMode < ARRAY_LENGTH(AlphaModeTable); cMode++)
				{
					if (x_strcmp(Str, AlphaModeTable[cMode].pId) == 0)
					{
						break;
					}
				}

				ASSERT(cMode < ARRAY_LENGTH(AlphaModeTable));
				pTexture->m_AlphaMode = AlphaModeTable[cMode].Mode;

				count += Tf.Get(&pTexture->m_nFiles);
				count += Tf.Get(pTexture->m_Filename);

				if (count != 6)
				{
					return FALSE;
				}
			}
		}
		// -----------------------------------------------------------
		// read stages
		else if (Token == g_kIdStage)
		{
			ASSERTS((pMaterialSet->m_pStages == NULL), "mesh contains more than one stage block");

			pMaterialSet->m_nStages = Token.Count;
			ASSERT(pMaterialSet->m_nStages);

			pMaterialSet->m_pStages = new stage [pMaterialSet->m_nStages];

			for(s32 cStage = 0; cStage < pMaterialSet->m_nStages; cStage++)
			{
				stage	*pStage = &pMaterialSet->m_pStages[cStage];
				s32		cOp;
				s32		cWrap;
				s32		cMode;

				s32 count;

				// get uv, texture, and transform indices
				count = Tf.Get(&pStage->m_iUV);
				if (pStage->m_iUV < 0)
				{
					// should bail, but we really can't
//					return FALSE;

					// so gonna force it to 0 instead
					pStage->m_iUV = 0;
				}
				count += Tf.Get(&pStage->m_iTexture);

				// get transform index
				count += Tf.Get(&pStage->m_iTransform);

				// get flags
				count += Tf.Get(&pStage->m_Flags);

				// get alpha
				count += Tf.Get(&pStage->m_Alpha);

				// get active distance
				count += Tf.Get(&pStage->m_ActiveDistance);

				// get operation
				count += Tf.Get(Str);
				for(cOp = 0; cOp < ARRAY_LENGTH(OpTable); cOp++)
				{
					if (x_strcmp(Str, OpTable[cOp].pId) == 0)
					{
						break;
					}
				}

				ASSERT(cOp < ARRAY_LENGTH(OpTable));
				pStage->m_Op = OpTable[cOp].Op;


				// get blend operation
				count += Tf.Get(Str);
				for(cOp = 0; cOp < ARRAY_LENGTH(BlendOpTable); cOp++)
				{
					if (x_strcmp(Str, BlendOpTable[cOp].pId) == 0)
					{
						break;
					}
				}

				ASSERT(cOp < ARRAY_LENGTH(BlendOpTable));
				pStage->m_BlendOp = BlendOpTable[cOp].Op;


				// get wrap u
				count += Tf.Get(Str);
				for(cWrap = 0; cWrap < ARRAY_LENGTH(WrapTable); cWrap++)
				{
					if (x_strcmp(Str, WrapTable[cWrap].pId) == 0)
					{
						break;
					}
				}

				ASSERT(cWrap < ARRAY_LENGTH(WrapTable));
				pStage->m_WrapU = WrapTable[cWrap].Mode;


				// get wrap v
				count += Tf.Get(Str);
				for(cWrap = 0; cWrap < ARRAY_LENGTH(WrapTable); cWrap++)
				{
					if (x_strcmp(Str, WrapTable[cWrap].pId) == 0)
					{
						break;
					}
				}

				ASSERT(cWrap < ARRAY_LENGTH(WrapTable));
				pStage->m_WrapV = WrapTable[cWrap].Mode;


				// get uv scale
				count += Tf.Get(&pStage->m_UvScale);


				// get uvmod mode
				count += Tf.Get(Str);
				for(cMode = 0; cMode < ARRAY_LENGTH(UvModTable); cMode++)
				{
					if (x_strcmp(Str, UvModTable[cMode].pId) == 0)
					{
						break;
					}
				}

				ASSERT(cMode < ARRAY_LENGTH(UvModTable));
				pStage->m_UvMod.m_Mode = UvModTable[cMode].Mode;


				// get uvmod anim mode
				count += Tf.Get(Str);
				for(cMode = 0; cMode < ARRAY_LENGTH(UvModAnimTable); cMode++)
				{
					if (x_strcmp(Str, UvModAnimTable[cMode].pId) == 0)
					{
						break;
					}
				}

				ASSERT(cMode < ARRAY_LENGTH(UvModAnimTable));
				pStage->m_UvMod.m_AnimMode = UvModAnimTable[cMode].Mode;


				// get uvmod
				count += Tf.Get(&pStage->m_UvMod.m_vOrigin);
				count += Tf.Get(&pStage->m_UvMod.m_ScrollSpeed);
				count += Tf.Get(&pStage->m_UvMod.m_RotateSpeed);
				count += Tf.Get(&pStage->m_UvMod.m_Amplitude);
				count += Tf.Get(&pStage->m_UvMod.m_Bias);

				if (count != 18)
				{
					return FALSE;
				}
			}
		}
		// -----------------------------------------------------------
		// read materials
		else if (Token == g_kIdMaterial)
		{
			ASSERTS((pMaterialSet->m_pMaterials == NULL), "mesh contains more than one material block");

			pMaterialSet->m_nMaterials = Token.Count;
			ASSERT(pMaterialSet->m_nMaterials);

			pMaterialSet->m_pMaterials = new material [pMaterialSet->m_nMaterials];

			for(s32 cMaterial = 0; cMaterial < pMaterialSet->m_nMaterials; cMaterial++)
			{
				material	*pMaterial = &pMaterialSet->m_pMaterials[cMaterial];

				s32 count;

				// get name
				count = Tf.Get(pMaterial->m_Name);

				// get num stages
				count += Tf.Get(&pMaterial->m_nStages);

				// get sub-material indices
				for(s32 cStage = 0; cStage < pMaterial->m_nStages; cStage++)
				{
					Tf.Get(&pMaterial->m_iStages[cStage]);

					if (pMaterial->m_iStages[cStage] == -1) {
						x_printf("ERROR: material '%s' has texture stage = -1\n", pMaterial->m_Name);
						return FALSE;
					}

				}

				// get flags
				count += Tf.Get(&pMaterial->m_Flags);

				// get alpha
				count += Tf.Get(&pMaterial->m_Alpha);

				// get overall alpha mode
				count += Tf.Get(Str);

				// ********* ERROR check
				if (count != 5)
				{
					return FALSE;
				}

				for(s32 cMode = 0; cMode < ARRAY_LENGTH(AlphaModeTable); cMode++)
				{
					if (x_strcmp(Str, AlphaModeTable[cMode].pId) == 0)
					{
						break;
					}
				}

				ASSERT(cMode < ARRAY_LENGTH(AlphaModeTable));
				pMaterial->m_OverallAlphaMode = AlphaModeTable[cMode].Mode;
			}
		}
		// -----------------------------------------------------------
		// read transform
		else if (Token == g_kIdTransform)
		{
			ASSERT(m_Version >= 3.0f);

			pMaterialSet->m_nTransforms = Token.Count;
			ASSERT (pMaterialSet->m_nTransforms > 0);

			pMaterialSet->m_pTransforms = new matrix4 [pMaterialSet->m_nTransforms];
			for(s32 cTrans = 0; cTrans < pMaterialSet->m_nTransforms; cTrans++)
			{
				matrix4 &pMat = pMaterialSet->m_pTransforms[cTrans];

				s32 count;

				count = Tf.Get(&pMat.M[0][0]);
				count += Tf.Get(&pMat.M[0][1]);
				count += Tf.Get(&pMat.M[0][2]);
				count += Tf.Get(&pMat.M[0][3]);
				count += Tf.Get(&pMat.M[1][0]);
				count += Tf.Get(&pMat.M[1][1]);
				count += Tf.Get(&pMat.M[1][2]);
				count += Tf.Get(&pMat.M[1][3]);
				count += Tf.Get(&pMat.M[2][0]);
				count += Tf.Get(&pMat.M[2][1]);
				count += Tf.Get(&pMat.M[2][2]);
				count += Tf.Get(&pMat.M[2][3]);
				count += Tf.Get(&pMat.M[3][0]);
				count += Tf.Get(&pMat.M[3][1]);
				count += Tf.Get(&pMat.M[3][2]);
				count += Tf.Get(&pMat.M[3][3]);
				if (count != 16)
				{
					return FALSE;
				}
			}
		}
		// ---------------------------------------------------------------
		// skip any other data
		else
		{
			Tf.SkipSection();
		}
	}

    Tf.UnmarkSection();

	return TRUE;
}

//****************************************************************************
// find next prime > n

s32 NextPrime(s32 n)
{
	if (n <= 3)	return n;
	if (n == 4) return 5;

	if ((n & 1) == 0)
	{
		n++;
	}

	while(1)
	{
		s32	i;
		for(i = 3; (i * i) <= n; i += 2)
		{
			if ((n % i) == 0)
			{
				break;
			}
		}

		if ((i * i) > n)
		{
			return n;
		}

		n += 2;
	}
}

//****************************************************************************
// merge any coincident verts

struct MergedItem
{
	mesh::vertex	m_Vert;
	s32				m_iMerged;
};

typedef x_std::vector<MergedItem>	MergedItemList;
typedef x_std::vector<mesh::face>	FaceList;


s32 FindMergedItem(const MergedItemList& List, const MergedItem& Item);


void object::MergeVerts(fnTickCallback fnCallback, f32 TolerancePos, f32 ToleranceNormals, f32 ToleranceUVs, f32 ToleranceColors)
{
	mesh::chunk		*pChunk;
	mesh::face		*pFace;
	mesh::vertex	*pMergedVerts;
	s32				*piMergedVerts;
	s32				nMergedVerts;
	s32				nVerts;
	s32				NumMerged;
	MergedItemList	*pMergedBuckets = NULL;
	f32				MinX, MaxX;
	u32				Key;
	s32				iItem;
	s32				cVert;
	s32				cFace;
	s32				cChunk;
	s32				nBuckets;
	s32				cBucket;
	s32				nMaxVerts;
	FaceList		Faces;
	s32				*piMergedToOrigVerts;


	g_MergeTolerancePos		= TolerancePos;
	g_MergeToleranceNormals = ToleranceNormals;
	g_MergeToleranceUVs		= ToleranceUVs;
	g_MergeToleranceColor	= ToleranceColors;


	ASSERT(m_pChunks);


	// count the max number of verts in all chunks
	nMaxVerts = 0;
	for(cChunk = 0; cChunk < m_nChunks; cChunk++)
	{
		nMaxVerts = MAX(nMaxVerts, m_pChunks[cChunk].m_nVerts);
	}


	// we want at max 5 verts per bucket upto a max of 5000 buckets
	nBuckets = MAX(1, NextPrime(nMaxVerts / 5));
	ASSERT(nBuckets);

	nBuckets = MIN(5000, nBuckets);
	pMergedBuckets = new MergedItemList [nBuckets];


	pMergedVerts = new mesh::vertex [nMaxVerts];
	piMergedVerts = new s32 [nMaxVerts];
	piMergedToOrigVerts = new s32 [nMaxVerts];


	for(cChunk = 0; cChunk < m_nChunks; cChunk++)
	{
		pChunk = &m_pChunks[cChunk];

		// calc min and max x for the verts in this chunk
		MinX = MaxX = pChunk->m_pVerts[0].m_vPos.X;

		for(cVert = 1; cVert < pChunk->m_nVerts; cVert++)
		{
			MinX = MIN(MinX, pChunk->m_pVerts[cVert].m_vPos.X);
			MaxX = MAX(MaxX, pChunk->m_pVerts[cVert].m_vPos.X);
		}


		for(cVert = 0; cVert < nMaxVerts; cVert++)
		{
			piMergedToOrigVerts[cVert] = -1;
		}


		nMergedVerts = 0;
		NumMerged = 0;

		for(cVert = 0; cVert < pChunk->m_nVerts; cVert++)
		{
			const mesh::vertex&	Vert = pChunk->m_pVerts[cVert];
			MergedItemList		*pBucket = NULL;
			MergedItem			SearchItem;


			if (fnCallback)
			{
				fnCallback();
			}

			// create the key
			Key = (u32)(((Vert.m_vPos.X - MinX) / (MaxX - MinX)) * (f32)(nBuckets - 1));
			ASSERT((Key >= 0) && (Key < (u32)nBuckets));

			pBucket = &pMergedBuckets[Key];

			SearchItem.m_iMerged = -1;
			SearchItem.m_Vert = Vert;

//			iItem = pBucket->Find(SearchItem, fnMergeCompareVerts);
			iItem = FindMergedItem(*pBucket, SearchItem);


			// we already have this vert - remember a reference from the original vert to the merged one
			if (iItem != -1)
			{
				MergedItem	*pMergedItem = &(*pBucket)[iItem];

				piMergedVerts[cVert] = pMergedItem->m_iMerged;

				NumMerged++;
			}
			// this is a new vert - add it
			else
			{
//				MergedItem	*pNewItem;
				MergedItem	NewItem;


				pMergedVerts[nMergedVerts] = Vert;
				piMergedVerts[cVert] = nMergedVerts;
//				piMergedToOrigVerts[cVert] = m_piToOrigVerts[cChunk][cVert];

//				pNewItem = new MergedItem;
//				pNewItem->m_iMerged = nMergedVerts;
//				pNewItem->m_Vert = Vert;
//				pBucket->Add(*pNewItem);

				NewItem.m_iMerged = nMergedVerts;
				NewItem.m_Vert = Vert;
				pBucket->push_back(NewItem);

				nMergedVerts++;
			}
		}

		if(NumMerged > 0)
		{
			// delete the original verts and replace with the merged list
			delete [] pChunk->m_pVerts;
	
			pChunk->m_nVerts = nMergedVerts;
			pChunk->m_pVerts = new mesh::vertex [pChunk->m_nVerts];
	
			for(cVert = 0; cVert < pChunk->m_nVerts; cVert++)
			{
				pChunk->m_pVerts[cVert] = pMergedVerts[cVert];
			}
	
	
			// update face indices
			for(cFace = 0; cFace < pChunk->m_nFaces; cFace++)
			{
				pFace = &pChunk->m_pFaces[cFace];
	
				nVerts = pFace->m_bQuad ? 4 : 3;
				for(cVert = 0; cVert < nVerts; cVert++)
				{
					pFace->m_Index[cVert] = piMergedVerts[pFace->m_Index[cVert]];
				}
			}
		}

		// empty the buckets
		for(cBucket = 0; cBucket < nBuckets; cBucket++)
		{
			pMergedBuckets[cBucket].clear();
		}


		// -------------------------------------------------------------------
		// remove degenerate faces

		Faces.clear();
		Faces.reserve(pChunk->m_nFaces);

		for(cFace = 0; cFace < pChunk->m_nFaces; cFace++)
		{
			xbool	bDegenerate = FALSE;


			pFace = &pChunk->m_pFaces[cFace];

			nVerts = pFace->m_bQuad ? 4 : 3;
			for(cVert = 0; cVert < nVerts; cVert++)
			{
				if (bDegenerate)
				{
					break;
				}

				for(s32 cVert2 = (cVert + 1); cVert2 < nVerts; cVert2++)
				{
					s32	iVertA = pFace->m_Index[cVert];
					s32	iVertB = pFace->m_Index[cVert2];

					// remove faces with duplicate vert indices
					if (iVertA == iVertB)
					{
						bDegenerate = TRUE;
						break;
					}

					// remove faces with same vert positions (these can exist after the merge if they have different uvs)
					if ((pChunk->m_pVerts[iVertA].m_vPos - pChunk->m_pVerts[iVertB].m_vPos).SquaredLength() < x_sqr(kTolerancePos))
					{
						bDegenerate = TRUE;
						break;
					}
				}
			}

			if (bDegenerate == FALSE)
			{
				Faces.push_back(*pFace);
			}
		}


		// we removed some faces - copy the new faces over the original ones
		if (Faces.size() != (u32)pChunk->m_nFaces)
		{
			pChunk->m_nFaces = Faces.size();

			mesh::face	*pNewFaces = new mesh::face [pChunk->m_nFaces];

			for(s32 cFace = 0; cFace < pChunk->m_nFaces; cFace++)
			{
				pNewFaces[cFace] = Faces[cFace];
			}


			delete [] pChunk->m_pFaces;
			pChunk->m_pFaces = pNewFaces;
		}
	}

	RemoveUnusedVerts(fnCallback); // removes unused verts and updates face indices


	// clean up
	delete [] pMergedVerts;
	pMergedVerts = NULL;

	delete [] piMergedVerts;
	piMergedVerts = NULL;

	delete [] piMergedToOrigVerts;
	piMergedToOrigVerts = NULL;

	delete [] pMergedBuckets;
	pMergedBuckets = NULL;
}

//****************************************************************************
xbool fnCompareMergedItems(const MergedItem& ItemA, const MergedItem& ItemB);

s32 FindMergedItem(const MergedItemList& List, const MergedItem& Item)
{
	for(u32 cItem = 0; cItem < List.size(); cItem++)
	{
		if (fnCompareMergedItems(List[cItem], Item))
		{
			return cItem;
		}
	}

	return -1;
}

//****************************************************************************
// return true if vert A and B are "the same"

xbool fnCompareMergedItems(const MergedItem& ItemA, const MergedItem& ItemB)
{
	const mesh::vertex	*pVert1 = &ItemA.m_Vert;
	const mesh::vertex	*pVert2 = &ItemB.m_Vert;

	ASSERT(pVert1 != pVert2);
	ASSERT((pVert1) && (pVert2));

	// position
	if ((pVert1->m_vPos - pVert2->m_vPos).SquaredLength() >= x_sqr(g_MergeTolerancePos))
	{
		return FALSE;
	}

	// normal
	if ((pVert1->m_vNormal - pVert2->m_vNormal).SquaredLength() >= x_sqr(g_MergeToleranceNormals))
	{
		return FALSE;
	}

	// color
	if ((pVert1->m_Color - pVert2->m_Color).SquaredLength() >= x_sqr(g_MergeToleranceColor))
	{
		return FALSE;
	}

	// uvs
	for(s32 cStage = 0; cStage < kMaxNumStages; cStage++)
	{
		if ((pVert1->m_UVs[cStage] - pVert2->m_UVs[cStage]).SquaredLength() >= x_sqr(g_MergeToleranceUVs))
		{
			return FALSE;
		}
	}

	// they're the same !
	return TRUE;
}

//****************************************************************************
// update the overall alpha mode of each material

void object::UpdateOverallAlphaModes()
{
	// This is a new attempt at doing this function.  See the commented out code below to see how
	// the old version worked.
	//
	// We iterate the stages, allowing each possibly opaque stage to restrict the maximum opacity.
	// Store the maximum transparency allowable in the material set.
	//
	// There is a special case when the texture stages don't specify any transparency -
	// It's possible that the vertex color's alpha might specify transparency.
	// Because of this possiblity, any time we have less than ALPHA_MODE_TRANSPARENT, we
	// need to check the vertices to see if they have transparency.
	// As an additional wrinkle, if there are two adjacent opaque stages of the same type,
	// they may be using the vertex alpha to crossfade themselves, and will have the transparency
	// of their textures ONLY.

	if (!m_pMaterialSets)
	{
		// The MTF has no materials
//		ASSERT(m_pMaterialSets);
		return;
	}

	s32 iMaterialSet;
	for(iMaterialSet = 0; iMaterialSet < m_nMaterialSets; iMaterialSet++)
	{
		material_set	*pMaterialSet = &m_pMaterialSets[iMaterialSet];
		ASSERT(pMaterialSet);
		if (!pMaterialSet->m_pMaterials)
		{
			continue;
		}

		s32 iMaterial;
		for(iMaterial = 0; iMaterial < pMaterialSet->m_nMaterials; iMaterial++)
		{
			mesh::material	*pMaterial = &pMaterialSet->m_pMaterials[iMaterial];
			ASSERT(pMaterial);
			ASSERT(pMaterial->m_iStages);
			ASSERT(pMaterialSet->m_pStages);

			// Don't try to second-guess if we are given an explicit alpha mode
			if (pMaterial->m_OverallAlphaMode != ALPHA_MODE_NONE)
			{
				continue;
			}

			// Variables for crossfade determination
			operation	last_op		= OP_NONE;
			xbool		crossfade	= false;
			alpha_mode	mode		= ALPHA_MODE_TRANSPARENT;

			s32 iStage;
			for (iStage = 0; iStage < pMaterial->m_nStages; iStage++)
			{
				const stage& pStage = pMaterialSet->m_pStages[pMaterial->m_iStages[iStage]];

				switch (pStage.m_Op)
				{
				case OP_DIFFUSE_MASK:
				case OP_SELF_ILLUM_MASK:
					// These use their alpha channels to modulate the next stage, so do not
					// contribute to the equation.  The stage they are modulating does contribute,
					// however, and will be picked up on the next iteration.

					// DDH - as it turns out, what is probably desired in these situations is
					// the exact OPPOSITE of the above comment - _MASK stages restrict
					// transparency completely.  If the artists want transparency, they'll
					// have to ask for it when using ambiguous stages
					mode = ALPHA_MODE_NONE;
					break;

				case OP_DIFFUSE:
				case OP_DIFFUSE_UV:
				case OP_SELF_ILLUM:
				case OP_SELF_ILLUM_UV:
					// These might be opaque

					// This is a crossfade - remember that for later
					// NB: This test isn't *exactly* right, but it's close enough.
					if (pStage.m_Op == last_op)
						crossfade = true;

					// If this stage has a forced alpha value, then it is not opaque - skip it
					if (pStage.m_Alpha < 1.0f)
						break;

					// If there is an attached texture, check its transparency status
					if (pStage.m_iTexture != -1)
					{
						const texture&	texture = pMaterialSet->m_pTextures[pStage.m_iTexture];
						const char*		test	= texture.m_Filename;
						s32				len		= x_strlen(test);

						if (test[len-6] == '_' && x_toupper(test[len-5]) == 'P')
						{
							// This is punchthrough by definition - don't even check the flags
							mode = x_std::min (ALPHA_MODE_PUNCHTHROUGH, mode);
						}
						else
						{
							// otherwise, this stage might restrict the mode - accumulate it
							mode = x_std::min (texture.m_AlphaMode, mode);
						}
					}
					break;

				default:
					// All other operations are ignored, since they don't influence alpha
					break;
				}

				// Remember for next time
				last_op = pStage.m_Op;
			}


#if 0		// DDH - we don't want to do this!  Make the artists be explicit if they want this behavior

			// If this material looks opaque by this point, and we're not using vertex alpha
			// for a crossfade, then we need to make sure that the vertex color's alpha doesn't
			// force us to go transparent.
			if (!crossfade && mode == ALPHA_MODE_NONE)
			{
				// We need to check the vertex alpha to see if maybe it brings in some alpha
				s32 iChunk;
				for (iChunk=0; iChunk < m_nChunks; ++iChunk)
				{
					const chunk& Chunk = m_pChunks[iChunk];

					s32 iFace;
					for (iFace=0; iFace < Chunk.m_nFaces; ++iFace)
					{
						const face& Face = Chunk.m_pFaces[iFace];

						if (Face.m_iMaterial == iMaterial)
						{
							if (   Chunk.m_pVerts[Face.m_Index[0]].m_Color.W < 1.0f
								|| Chunk.m_pVerts[Face.m_Index[1]].m_Color.W < 1.0f
								|| Chunk.m_pVerts[Face.m_Index[2]].m_Color.W < 1.0f
								|| (Face.m_bQuad && Chunk.m_pVerts[Face.m_Index[3]].m_Color.W < 1.0f))
							{
								// If any one of these vertices is transparent, then we consider
								// the entire material to be transparent.  Remember that crossfade
								// alpha has already been ruled out by this point.
								mode   = ALPHA_MODE_TRANSPARENT;
								iChunk = m_nChunks-1;	// Force outer loop to exit
								break;					// Force inner loop to exit
							}
						}
					}
				}
			}
#endif

			//  store the computed mode
			pMaterial->m_OverallAlphaMode = mode;
		}
	}
}
		

//-----------------------------------------------------------------------------
// DDH - OLD VERSION
//-----------------------------------------------------------------------------
	// How will this material be TRANSPARENT?
	//	If any of the following is true
	//	1) If the vertices have alpha < 1.0f AND any of the following is true
	//		A) There is one OR FEWER "solid" textures
	//		B) The first normally "solid" texture has "VERTEX_ALPHA" set in its blend_op
	//		C) The first normally "solid" texture has an Alpha value < 1.0f
	//	2) If the first normally "solid" texture has "VERTEX_ALPHA" set in it's blend_op
	//	3) If the first normally "solid" texture has an Alpha value < 1.0f
	//
	//	"Solid" textures are any of {DIFFUSE, DIFFUSE_UV, SELF_ILLUM, or SELF_ILLUM_UV}
	//	We look at the first "solid" texture as a guideline and expect any followup textures to blend to it.
	//	We look at the vertex alpha and expect it to match the material settings.
	//		Vertex Alpha is used to blend 2 solid textures into 1 solid texture.
	//		If the first solid texture uses alpha, the entire blended texture should use alpha

//			s32 nSolidCount=0;
//			xbool bOpIsAlpha = FALSE;
//			xbool bBOpIsAlpha = FALSE;
//			for (iStage = 0; iStage < pMaterial->m_nStages; iStage++)
//			{
//				mesh::stage *pStage = &pMaterialSet->m_pStages[pMaterial->m_iStages[iStage]];
//				ASSERT(pStage);
//
//				if (   (pStage->m_Op == mesh::OP_DIFFUSE)
//					|| (pStage->m_Op == mesh::OP_DIFFUSE_UV)
//					|| (pStage->m_Op == mesh::OP_SELF_ILLUM)
//					|| (pStage->m_Op == mesh::OP_SELF_ILLUM_UV)
//				   )
//				{
//					++nSolidCount;
//					if (nSolidCount == 1)
//					{
//						// make sure it isn't a punch-through texture...
//						if(pStage->m_iTexture != -1)
//						{
//							const char *test = pMaterialSet->m_pTextures[pStage->m_iTexture].m_Filename;
//							s32 len = x_strlen(test);
//							if (!(test[len-6] == '_' && (test[len-5] == 'p' || test[len-5] == 'P')))
//							{
//								// this should be the first instance of a "solid" texture within the material
//								// check the two different alphas of this stage
//								bOpIsAlpha = (pStage->m_Alpha < 1.0f);
//								bBOpIsAlpha = ((pStage->m_BlendOp == mesh::BLEND_OP_VERTEX_ALPHA) || (pStage->m_BlendOp == mesh::BLEND_OP_TEXTURE_ALPHA));
//							}
//						}
//					}
//				}
//			}
//
//			if (   (!bOpIsAlpha && !bBOpIsAlpha)
//				&& nSolidCount <= 1)
//			{
//				// The material itself isn't using any alpha, 
//				//	but there are very few solid textures.
//				//	If the vertices use any alpha, the entire material should be set as using alpha
//
//				xbool bUsesVertAlpha = FALSE;
//
//				s32 iChunk;
//				for (iChunk=0; (iChunk<m_nChunks && !bUsesVertAlpha); iChunk++)
//				{
//					mesh::chunk *pChunk = &m_pChunks[iChunk];
//					ASSERT(pChunk);
//
//					s32 iFace;
//					for (iFace=0; (iFace<pChunk->m_nFaces && !bUsesVertAlpha); iFace++)
//					{
//						mesh::face *pFace = &pChunk->m_pFaces[iFace];
//						ASSERT(pFace);
//
//						if (pFace->m_iMaterial == iMaterial)
//						{
//							s32 nLimit = (pFace->m_bQuad?4:3);
//							s32 iVert;
//							for (iVert=0; iVert<nLimit; iVert++)
//							{
//								mesh::vertex *pVert = &pChunk->m_pVerts[pFace->m_Index[iVert]];
//								ASSERT(pVert);
//
//								if (pVert->m_Color.W < 1.0f)
//								{
//									bUsesVertAlpha = TRUE;
//									break;
//								}
//							}
//						}
//					}
//				}
//				pMaterial->m_OverallAlphaMode = (bUsesVertAlpha?ALPHA_MODE_TRANSPARENT:ALPHA_MODE_NONE);
//			}
//			else if ((bOpIsAlpha || bBOpIsAlpha) && nSolidCount <= 1)
//			{
//				// we have either an appropriately alpha'd solid, or a material using vertex alpha
//				// the vertex alpha might be opaque, but we still treat it as alpha...
//				pMaterial->m_OverallAlphaMode = ALPHA_MODE_TRANSPARENT;
//			}
//			else
//			{
//				// no alpha!
//				pMaterial->m_OverallAlphaMode = ALPHA_MODE_NONE;
//			}
//		}
//	}
//}

//****************************************************************************
// return the id of a chunk from a name

s32 object::GetChunkId(const x_std::string& Name) const
{
	s32	iChunk = -1;

	for(s32 cChunk = 0; cChunk < m_nChunks; cChunk++)
	{
		if (Name == m_pChunks[cChunk].m_Name)
		{
			iChunk = cChunk;
			break;
		}
	}

	return iChunk;
}

//****************************************************************************
// return the id of a material set for a particular target - or -1 if not present

s32 object::GetMaterialSetId(target Target) const
{
	for(s32 cMaterialSet = 0; cMaterialSet < m_nMaterialSets; cMaterialSet++)
	{
		if (m_pMaterialSets[cMaterialSet].m_Target == Target)
		{
			return cMaterialSet;
		}
	}
	
	return -1;
}

//****************************************************************************
// rotate all verts (and normals) in the mesh

void object::Rotate(const radian3& Rot)
{
	matrix4	mRot;
	mRot.Identity();
	mRot.SetRotation(Rot);

	for(s32 cChunk = 0; cChunk < m_nChunks; cChunk++)
	{
		chunk *pChunk = &m_pChunks[cChunk];

		for(s32 cVert = 0; cVert < pChunk->m_nVerts; cVert++)
		{
			// transform pos
			pChunk->m_pVerts[cVert].m_vPos = (mRot * pChunk->m_pVerts[cVert].m_vPos);
			pChunk->m_pVerts[cVert].m_vRenderPos = (mRot * pChunk->m_pVerts[cVert].m_vRenderPos);

			// transform normal (transpose of the inverse of M = M if it is orthogonal)
			pChunk->m_pVerts[cVert].m_vNormal = (mRot * pChunk->m_pVerts[cVert].m_vNormal);
			pChunk->m_pVerts[cVert].m_vRenderNormal = (mRot * pChunk->m_pVerts[cVert].m_vRenderNormal);
		}
	}
}

//****************************************************************************
template <class T>
inline void merge_part (T *& data, s32& size, const T *extra_data, s32 extra)
{
	ASSERT(size + extra >= 0);

	if (extra != 0)
	{
		T* tmp	= data;
		data	= new T[size + extra];
		if (tmp != 0)
		{
			x_std::copy (tmp, tmp + size, data);
			delete[] tmp;
		}

		ASSERT (extra_data != 0);
		x_std::copy (extra_data, extra_data + extra, data + size);

		size += extra;
	}
}

//****************************************************************************
s32 material_set::merge_set (const material_set& other)
{
	const s32 old_nMaterials	= m_nMaterials;
	const s32 old_nStages		= m_nStages;
	const s32 old_nTextures		= m_nTextures;
	const s32 old_nTransforms	= m_nTransforms;
	s32 i, j;

	merge_part (m_pMaterials, m_nMaterials, other.m_pMaterials, other.m_nMaterials );
	merge_part (m_pStages,    m_nStages,    other.m_pStages,    other.m_nStages    );
	merge_part (m_pTextures,  m_nTextures,  other.m_pTextures,  other.m_nTextures  );
	merge_part (m_pTransforms,m_nTransforms,other.m_pTransforms,other.m_nTransforms);

	// Fixup indices
	for (i = old_nMaterials; i < m_nMaterials; ++i)
	{
		for (j = 0; j < m_pMaterials[i].m_nStages; ++j)
		{
			m_pMaterials[i].m_iStages[j] += old_nStages;
		}
	}

	for (i = old_nStages; i < m_nStages; ++i)
	{
		if (m_pStages[i].m_iTexture != -1)
		{
			m_pStages[i].m_iTexture += old_nTextures;
		}

		if (m_pStages[i].m_iTransform != -1)
		{
			m_pStages[i].m_iTransform += old_nTransforms;
		}
	}

	return other.m_nMaterials;
}

//****************************************************************************
color material_set::estimated_color (s32 material_index, f32 u_coord, f32 v_coord, f32 t, color diffuse) const
{
	// WARNING: This function can get VERY slow for any 

	ASSERT(material_index < m_nMaterials);

	// Create and load the required bitmaps
	x_bitmap*	bitmap_cache = new x_bitmap[m_nTextures];
	for (s32 i = 0; i < m_nTextures; ++i)
		bitmap_cache[i].Load (m_pTextures[i].m_Filename);

	// Look up the texel
	color c = m_pMaterials[material_index].estimated_color (u_coord, v_coord, t, diffuse, m_pStages, bitmap_cache);

	// Free the bitmap
	delete[] bitmap_cache;

	return c;
}

//****************************************************************************
vector4	material::estimated_color (const vector2& UVCoords, f32 t, const vector4& Diffuse, const stage* pStages, x_bitmap* pTextures) const
{
//	f32 u_coord = UVCoords.X; 
//	f32 v_coord = UVCoords.Y; 
	color diffuse;

	diffuse.R = (u8)(Diffuse.X * 255);
	diffuse.G = (u8)(Diffuse.Y * 255);
	diffuse.B = (u8)(Diffuse.Z * 255);
	diffuse.A = (u8)(Diffuse.W * 255);

	color Ret = estimated_color(UVCoords.X,UVCoords.Y,t,diffuse, pStages, pTextures);
	return vector4((f32)(Ret.R)/255.0f,(f32)(Ret.G)/255.0f,(f32)(Ret.B)/255.0f,(f32)(Ret.A)/255.0f);
}

//****************************************************************************
color material::estimated_color (f32 u_coord, f32 v_coord, f32 t, color diffuse, const stage* pStages, x_bitmap* pBitmaps) const
{
	//UNREF(t);
	color		texel, light, mask, illum, Result;
	operation	last_stage;
	xbool		has_diffuse;

	// This is where we evaluate the stages
	Result.Set (0);
	mask.Set (255,255,255,255);
	light.Set (0);
	illum.Set (0);
	has_diffuse = FALSE;

	last_stage = OP_NONE;
	for (s32 i = 0; i < m_nStages; ++i)
	{
		const mesh::stage& s = pStages[m_iStages[i]];

		if (s.m_iTexture != -1)
		{
			f32 u = u_coord * s.m_UvScale.X;

			switch (s.m_WrapU)
			{
			case UVWRAP_MODE_TILE:
				u -= (f32)x_floor (u);
				break;
			case UVWRAP_MODE_MIRROR:
				u = (f32)x_fmod (x_abs (u), 2.0f);
				if (u > 1.0f)		u  = 2.0f - u;
			case UVWRAP_MODE_CLAMP:
				if (u > 1.0f)		u = 1.0f;
				else if (u < 0.0f)	u = 0.0f;
				break;
			}

			f32 v = v_coord * s.m_UvScale.Y;
			switch (s.m_WrapV)
			{
			case UVWRAP_MODE_TILE:
				v -= (f32)x_floor (v);
				break;
			case UVWRAP_MODE_MIRROR:
				u = (f32)x_fmod (x_abs (v), 2.0f);
				if (v > 1.0f)		v  = 2.0f - v;
				break;
			case UVWRAP_MODE_CLAMP:
				if (v > 1.0f)		v = 1.0f;
				else if (v < 0.0f)	v = 0.0f;
				break;
			}

			if (s.m_Op == OP_ENV || s.m_Op == OP_CUBE_ENV)
			{
				// Get environment map color
				// For now, no env
				texel.Set (0);
			}
			else
			{
				// Get the texture color
				x_bitmap&	tex = pBitmaps[s.m_iTexture];
				u32			tu	= u32 (f32 (tex.GetWidth()) * u);
				u32			tv	= u32 (f32 (tex.GetHeight()) * v);

				// Very simple sub-sampling for now - will need to add blending later
				x_bitmap::SetPixelFormat (tex);

				if(tu >= (u32)tex.GetWidth()  ) tu = tex.GetWidth() -1;
				if(tv >= (u32)tex.GetHeight() ) tv = tex.GetHeight()-1;

				texel = x_bitmap::GetPixelColor (tu, tv);
			}

			texel *= u8 (x_std::max (x_std::min (s32 (s.m_Alpha * 255.0f), 255), 0));
			texel *= mask;
		}
		else
		{
			texel.Set (0);
		}

		// Clear the mask - it was already used
		mask.Set (255,255,255,255);

		// Adjust for this stage's blend operation
#if 0
		if (s.m_BlendOp == BLEND_OP_VERTEX_ALPHA)
		{
			texel.A = diffuse.A;
		}
		else 
		{
			if (s.m_BlendOp == BLEND_OP_NONE)
			{
				texel.A = 255;
			}
		}
#endif

		const operation old_last = last_stage;
		last_stage = s.m_Op;
		switch (s.m_Op)
		{
		case OP_DIFFUSE_MASK:
			mask = color(texel.A,texel.A,texel.A,texel.A);
			texel.A = 255;	// "Consume" the alpha
			// Fall through

		case OP_DIFFUSE:
		case OP_DIFFUSE_UV:
			if (old_last == OP_DIFFUSE || old_last == OP_DIFFUSE_UV || old_last == OP_DIFFUSE_MASK)
			{
				u8 a = diffuse.A;

				// Alpha cross-fade d = s0*s1 + (1-s0)*s2
				texel = (texel - Result) * a;
			}

			Result += texel;
			if (!has_diffuse)
			{
				light += diffuse;
				has_diffuse = TRUE;
			}
			break;

		case OP_MASK:
		case OP_MASK_UV:
			mask = texel;
			break;

		case OP_LIGHTMAP:														// lightmap
			light += texel;
			break;

		case OP_SPECULAR:
			Result += texel;
			break;

		case OP_SELF_ILLUM_MASK:
			mask = color(texel.A,texel.A,texel.A,texel.A);
			texel.A = 255;	// "Consume" the alpha
			// Fall through

		case OP_SELF_ILLUM:
		case OP_SELF_ILLUM_UV:
			if (old_last == OP_SELF_ILLUM || old_last == OP_SELF_ILLUM_UV || old_last == OP_SELF_ILLUM_MASK)
			{
				// Alpha cross-fade d = s0*s1 + (1-s0)*s2
				texel = (texel - illum) * diffuse.A;
			}
			illum += texel;
			break;

		case OP_DETAIL:
		case OP_DETAIL_UV:
//			Result *= texel;
			break;

		// Not implemented due to lack of rendering context data
		case OP_BUMP:															// bump using main uvs
		case OP_BUMP_UV:														// bump with own uvs
		case OP_DOT3:
			// DDH:
			// Should we fake bumping by reducing intensity by, say 30%?
			// This would simulate a random darkening.

		case OP_ENV:															// environment
		case OP_CUBE_ENV:														// cubic environment
		case OP_NORMAL:															// normal map
		case OP_EMBM:															// environment mapped bump
		case OP_EFFECT_GLOW:
		case OP_EFFECT_CAUSTIC:
			break;

		case OP_NONE:
			break;
		}
	}

	// Compute the lit/unlit result
	Result = Result * light + illum;

	// Modulate the final alpha value
	Result.A = (u8)((f32)(Result.A) * m_Alpha);	//BUG Fix Ziggy * 255.0f);

	return Result;
}

//***************************************************************************
//***************************************************************************
//***************************************************************************
template<class T> static s32 Find(const x_std::vector<T> List, const T& Item)
{
	for(u32 cItem = 0; cItem < List.size(); cItem++)
	{
		if (List[cItem] == Item)
		{
			return cItem;
		}
	}

	return -1;
}

//****************************************************************************
xbool object::FixInvalidData(fnTickCallback fnCallback)
{
	// Fix materials first so we can figure out if we need to re-index chunk data
	// loop over the MaterialSets
	//	Validate and correct any Texture errors
	//		build a texture re-index list to keep track of deleted textures
	//	Validate and correct any Stage errors
	//		Build a Stage re-index list to keep track of deleted stages (complete duplicates only)
	//		Remove any unused Textures
	//		Reindex and remove any unneeded Textures
	//	Validate and correct any Material errors
	//		Build a material re-index list to keep track of deleted materials (duplicates, empties, etc.)
	//		Remove any unused Stages
	// Confirm each MaterialSet has the same number of Materials (they MUST match in size)
	// Re-index any necessary chunk data
	// Validate Stage UV data (return FALSE on validation fail)


	// first, create the material index remap list for use in the collision compilers
	if (m_nMaterialSets)
	{
		m_nMaterialRemapIndices = m_pMaterialSets[0].m_nMaterials;
		m_pMaterialRemapIndices = new s32 [m_nMaterialRemapIndices];

		s32 i;
		for (i=0; i<m_nMaterialRemapIndices; i++)
		{
			m_pMaterialRemapIndices[i] = i;
		}
	}
	else
	{
		m_nMaterialRemapIndices = 0 ;
		m_pMaterialRemapIndices = 0 ;
	}

	//s32 nRemapDeleteCount = 0;

	s32	cChunk;

	// if we have no MaterialSets, we need to make sure all of the triangle data is not trying to reference materials
	if (m_nMaterialSets == 0)
	{
		for(cChunk = 0; cChunk < m_nChunks; cChunk++)
		{
			chunk	*pChunk = &m_pChunks[cChunk];

			for(s32 cFace = 0; cFace < pChunk->m_nFaces; cFace++)
			{
				if (fnCallback && (cFace & 100))
				{
					fnCallback();
				}

				pChunk->m_pFaces[cFace].m_iMaterial = -1;
			}
		}

		return TRUE;
	}




	s32	cMaterialSet;



	// -----------------------------------------------------------------------
	// make sure all material sets have the same number of materials (and they have stages and textures)

	s32	nMaterials = m_pMaterialSets[0].m_nMaterials;
	for(cMaterialSet = 0; cMaterialSet < m_nMaterialSets; cMaterialSet++)
	{
		if (m_pMaterialSets[cMaterialSet].m_nMaterials != nMaterials)
		{
			x_printf("AsciiMesh::FixInvalidData FATAL: material sets have different material counts\n");
			return FALSE;
		}

		// if it has materials, it should also have stages and textures
		if (nMaterials)
		{
			if (m_pMaterialSets[cMaterialSet].m_nStages == 0)
			{
				x_printf("AsciiMesh::FixInvalidData FATAL: material set %d has no stages\n", cMaterialSet);
				return FALSE;
			}

//			if (m_pMaterialSets[cMaterialSet].m_nTextures == 0)
//			{
//				x_printf("AsciiMesh::FixInvalidData FATAL: material set %d has no textures\n", cMaterialSet);
//				return FALSE;
//			}
		}
	}

	// for objects with no materials, we have to go through the triangle data and ensure nobody is trying to use materials
	if (nMaterials == 0)
	{
		for(cChunk = 0; cChunk < m_nChunks; cChunk++)
		{
			chunk	*pChunk = &m_pChunks[cChunk];

			for(s32 cFace = 0; cFace < pChunk->m_nFaces; cFace++)
			{
				if (fnCallback && (cFace & 100))
				{
					fnCallback();
				}

				pChunk->m_pFaces[cFace].m_iMaterial = -1;
			}
		}

		return TRUE;
	}


	// build a list of used materials
	x_std::vector<xbool>	bMaterialUsed(nMaterials, FALSE);
	s32						nUnassignedFaces = 0;

	//s32	nTotalFaces = 0;
	for(cChunk = 0; cChunk < m_nChunks; cChunk++)
	{
		chunk	*pChunk = &m_pChunks[cChunk];

		for(s32 cFace = 0; cFace < pChunk->m_nFaces; cFace++)
		{
			s32	iMaterial = pChunk->m_pFaces[cFace].m_iMaterial;

			if (iMaterial == -1)
			{
//				x_printf("AsciiMesh::FixInvalidData WARNING: unused material removed\n");
				nUnassignedFaces++;
			}
			else if ((iMaterial < 0) || (iMaterial >= nMaterials))
			{
				x_printf("AsciiMesh::FixInvalidData : face %d in chunk %d has an invalid material index %d, marking as unassigned\n", cFace, cChunk, iMaterial);
				pChunk->m_pFaces[cFace].m_iMaterial = -1;
				nUnassignedFaces++;
			}
			else
			{
				// flag it as being used
				bMaterialUsed[iMaterial] = TRUE;
			}
		}
	}



	s32	nUnusedMaterials = 0;
	for(u32 cMaterial = 0; cMaterial < bMaterialUsed.size(); cMaterial++)
	{
		if (bMaterialUsed[cMaterial] == FALSE)
		{
			nUnusedMaterials++;
			m_pMaterialRemapIndices[cMaterial] = -1;
			for (s32 i=cMaterial+1; i<m_nMaterialRemapIndices; i++)
			{
				m_pMaterialRemapIndices[i] --;
			}
		}
	}

	if (nUnusedMaterials)
	{
		x_printf("AsciiMesh::FixInvalidData WARNING: %d unused materials removed\n", nUnusedMaterials);
	}

	if (nUnassignedFaces)
	{
		x_printf("AsciiMesh::FixInvalidData WARNING: %d faces have no assigned material\n", nUnassignedFaces);
	}


	if (fnCallback)
	{
		fnCallback();
	}




	x_std::vector<s32>		iMaterialsToNew(nMaterials, -1);



	// -----------------------------------------------------------------------
	for(cMaterialSet = 0; cMaterialSet < m_nMaterialSets; cMaterialSet++)
	{
		material_set			*pMaterialSet = &m_pMaterialSets[cMaterialSet];
		x_std::vector<texture>	NewTextures;
		x_std::vector<s32>		iTexturesToNew(pMaterialSet->m_nTextures, -1);
		x_std::vector<stage>	NewStages;
		x_std::vector<s32>		iStagesToNew(pMaterialSet->m_nStages, -1);
		x_std::vector<material>	NewMaterials;
		u32						cTexture;
		u32						cStage;
		u32						cMaterial;


		// -------------------------------------------------------------------
		// build the list of unique materials

		for(cMaterial = 0; cMaterial < (u32)pMaterialSet->m_nMaterials; cMaterial++)
		{
			if (fnCallback)
			{
				fnCallback();
			}

			material *pMaterial = &pMaterialSet->m_pMaterials[cMaterial];

			// ignore unused materials
			if (bMaterialUsed[cMaterial] == FALSE)
			{
				continue;
			}

			// ignore materials with no stages
			if (pMaterial->m_nStages == 0)
			{
				m_pMaterialRemapIndices[cMaterial] = -1;
				for (s32 i=cMaterial+1; i<m_nMaterialRemapIndices; i++)
				{
					m_pMaterialRemapIndices[i] --;
				}
				x_printf("AsciiMesh::FixInvalidData WARNING: material with no stages removed\n");
				continue;
			}

			// ignore materials whose stages have an invalid texture index
			for(cStage = 0; cStage < (u32)pMaterial->m_nStages; cStage++)
			{
				stage *pStage = &pMaterialSet->m_pStages[pMaterial->m_iStages[cStage]];

				if (pStage->m_iTexture > pMaterialSet->m_nTextures)
				{
					x_printf("AsciiMesh::FixInvalidData WARNING: invalid texture index (m=%d,s=%d,t=%d) removed\n", cMaterial, cStage, pStage->m_iTexture);
					break;
				}
			}

			// test to see if we did a break in the cStage loop
			if (cStage < (u32)pMaterial->m_nStages)
			{
				// don't add this material
				continue;
			}


			// ---------------------------------------------------------------
			// see if we have this material

			s32 iMaterial = Find(NewMaterials, *pMaterial);

			// we already have it - keep a reference to it
			if (iMaterial != -1)
			{
				iMaterialsToNew[cMaterial] = iMaterial;

				m_pMaterialRemapIndices[cMaterial] = iMaterial;
				for (s32 i=cMaterial+1; i<m_nMaterialRemapIndices; i++)
				{
					m_pMaterialRemapIndices[i] --;
				}

				m_pMaterialRemapIndices[cMaterial] = iMaterial;
				for (i=cMaterial+1; i<m_nMaterialRemapIndices; i++)
				{
					m_pMaterialRemapIndices[i] --;
				}

				x_printf("AsciiMesh::FixInvalidData WARNING: duplicate material removed\n");
			}
			// we dont have it - add it to the list and keep a reference
			else
			{
				iMaterialsToNew[cMaterial] = NewMaterials.size();
				NewMaterials.push_back(*pMaterial);
			}
		}






		// -------------------------------------------------------------------
		// build the list of unique stages

		for(cStage = 0; cStage < (u32)pMaterialSet->m_nStages; cStage++)
		{
			stage *pStage = &pMaterialSet->m_pStages[cStage];

			if (fnCallback)
			{
				fnCallback();
			}

			// ignore stages with an invalid texture index
			if (pStage->m_iTexture > pMaterialSet->m_nTextures)
			{
				x_printf("AsciiMesh::FixInvalidData WARNING: invalid texture index (s=%d, t=%d) removed\n", cStage, pStage->m_iTexture);
				continue;
			}

			// ignore stages in unused materials
			for(cMaterial = 0; cMaterial < (u32)pMaterialSet->m_nMaterials; cMaterial++)
			{
				if (bMaterialUsed[cMaterial])
				{
					// see if this material references our stage
					material *pMaterial = &pMaterialSet->m_pMaterials[cMaterial];
					for(u32 cMaterialStage = 0; cMaterialStage < (u32)pMaterial->m_nStages; cMaterialStage++)
					{
						if (cStage == (u32)pMaterial->m_iStages[cMaterialStage])
						{
							break;
						}
					}

					// our stage is refernced by this material
					if (cMaterialStage < (u32)pMaterial->m_nStages)
					{
						break;
					}
				}
			}

			// if we got through all the materials, then our stage isn't referenced
			if (cMaterial == (u32)pMaterialSet->m_nMaterials)
			{
				continue;
			}

			// see if we have this stage
			s32 iStage = Find(NewStages, *pStage);

			// we already have it - keep a reference to it
			if (iStage != -1)
			{
				iStagesToNew[cStage] = iStage;

				x_printf("AsciiMesh::FixInvalidData WARNING: duplicate stage removed\n");
			}
			// we dont have it - add it to the list and keep a reference
			else
			{
				iStagesToNew[cStage] = NewStages.size();
				NewStages.push_back(*pStage);
			}
		}




		// -------------------------------------------------------------------
		// build the list of unique textures

		for(cTexture = 0; cTexture < (u32)pMaterialSet->m_nTextures; cTexture++)
		{
			if (fnCallback)
			{
				fnCallback();
			}

			texture	*pTexture = &pMaterialSet->m_pTextures[cTexture];


			// ignore textures in unused stages
			for(cStage = 0; cStage < NewStages.size(); cStage++)
			{
				// see if this stage references our texture
				if ((NewStages[cStage].m_iTexture != -1) && (NewStages[cStage].m_iTexture == (s32)cTexture))
				{
					break;
				}
			}

			// if we got through all the stages, then our texture isn't referenced
			if (cStage == NewStages.size())
			{
				continue;
			}


			// see if we have this texture
			s32 iTexture = Find(NewTextures, *pTexture);

			// we already have it - keep a reference to it
			if (iTexture != -1)
			{
				iTexturesToNew[cTexture] = iTexture;

				x_printf("AsciiMesh::FixInvalidData WARNING: duplicate texture removed\n");
			}
			// we dont have it - add it to the list and keep a reference
			else
			{
				iTexturesToNew[cTexture] = NewTextures.size();
				NewTextures.push_back(*pTexture);

				// check the dimensions
				if ((pTexture->m_Width == -1) || (pTexture->m_Height == -1) || (pTexture->m_Bpp == -1))
				{
					x_printf("AsciiMesh::FixInvalidData WARNING: texture wasn't found, dimensions and bit-depth are incorrect (%s)\n", pTexture->m_Filename);
				}
				else if ((pTexture->m_Width & (pTexture->m_Width-1)) || (pTexture->m_Height & (pTexture->m_Height-1)))
				{
					x_printf("AsciiMesh::FixInvalidData WARNING: texture dimensions aren't powers of two (%s)\n", pTexture->m_Filename);
				}
			}
		}




		// -------------------------------------------------------------------
		// if we have more than one material set and we have removed materials then bail

		s32	nNewMaterials = 0;
		for(cMaterial = 0; cMaterial < iMaterialsToNew.size(); cMaterial++)
		{
			if (iMaterialsToNew[cMaterial] != -1)
			{
				nNewMaterials++;
			}
		}

		if ((m_nMaterialSets > 1) && (nNewMaterials != nMaterials))
		{
			x_printf("AsciiMesh::FixInvalidData FATAL: material sets ended up with different material counts\n");
			return FALSE;
		}





		// -------------------------------------------------------------------
		// replace the textures
		delete [] pMaterialSet->m_pTextures;
		pMaterialSet->m_pTextures = NULL;

		pMaterialSet->m_nTextures = NewTextures.size();
		if (pMaterialSet->m_nTextures)
		{
			pMaterialSet->m_pTextures = new texture [pMaterialSet->m_nTextures];

			for(cTexture = 0; cTexture < (u32)pMaterialSet->m_nTextures; cTexture++)
			{
				pMaterialSet->m_pTextures[cTexture] = NewTextures[cTexture];
			}
		}


		// replace the stages
		delete [] pMaterialSet->m_pStages;
		pMaterialSet->m_pStages = NULL;

		pMaterialSet->m_nStages = NewStages.size();
		if (pMaterialSet->m_nStages)
		{
			pMaterialSet->m_pStages = new stage [pMaterialSet->m_nStages];

			for(cStage = 0; cStage < (u32)pMaterialSet->m_nStages; cStage++)
			{
				// re-index texture
				if (NewStages[cStage].m_iTexture != -1)
				{
					NewStages[cStage].m_iTexture = iTexturesToNew[NewStages[cStage].m_iTexture];
				}

				// replace
				pMaterialSet->m_pStages[cStage] = NewStages[cStage];
			}
		}


		// replace the materials
		delete [] pMaterialSet->m_pMaterials;
		pMaterialSet->m_pMaterials = NULL;

		pMaterialSet->m_nMaterials = NewMaterials.size();
		if (pMaterialSet->m_nMaterials)
		{
			pMaterialSet->m_pMaterials = new material [pMaterialSet->m_nMaterials];

			for(cMaterial = 0; cMaterial < (u32)(pMaterialSet->m_nMaterials); cMaterial++)
			{
				// re-index stages
				for(cStage = 0; cStage < (u32)NewMaterials[cMaterial].m_nStages; cStage++)
				{
					if (NewMaterials[cMaterial].m_iStages[cStage] != -1)
					{
						NewMaterials[cMaterial].m_iStages[cStage] = iStagesToNew[NewMaterials[cMaterial].m_iStages[cStage]];
					}
				}

				// replace
				pMaterialSet->m_pMaterials[cMaterial] = NewMaterials[cMaterial];
			}
		}
	}





	// -----------------------------------------------------------------------
	// re-index the geometry

	for(cChunk = 0; cChunk < m_nChunks; cChunk++)
	{
		chunk	*pChunk = &m_pChunks[cChunk];

		for(s32 cFace = 0; cFace < pChunk->m_nFaces; cFace++)
		{
			if (fnCallback && (cFace & 10))
			{
				fnCallback();
			}

			s32	iMaterial = pChunk->m_pFaces[cFace].m_iMaterial;

			if (iMaterial == -1)
				continue;

			ASSERT((iMaterial >= 0) && (iMaterial < nMaterials));
			if ((iMaterial >= 0) && (iMaterial < nMaterials))
			{
				pChunk->m_pFaces[cFace].m_iMaterial = iMaterialsToNew[iMaterial];
			}
			else
			{
				pChunk->m_pFaces[cFace].m_iMaterial = -1;
			}
		}
	}




#ifdef X_DEBUG
	// sanity check

	// check face material indices are valid
	for(cChunk = 0; cChunk < m_nChunks; cChunk++)
	{
		for(s32 cFace = 0; cFace < m_pChunks[cChunk].m_nFaces; cFace++)
		{
			s32	iMaterial = m_pChunks[cChunk].m_pFaces[cFace].m_iMaterial;
			ASSERT((iMaterial == -1) || ((iMaterial >= 0) && (iMaterial < nMaterials)));
		}
	}
#endif


	return TRUE;
}

//****************************************************************************
void object::GetBounds(vector3 bounds[3])
{ // return bounding box expressed as 3 vectors: origin, extents, and rotation
 // this routine simply finds min and max coordinates; the rotation returned is 0

	chunk			*pChunk;
	f32				MinX, MinY, MinZ, MaxX, MaxY, MaxZ;
	s32				cChunk;
	s32				cVert;

	for (u32 i=0; i<3; i++) bounds[i].Set(0.f, 0.f, 0.f);

	if (!m_nChunks) return;
	if (!m_pChunks[0].m_nVerts) return;

	MinX = MaxX = m_pChunks[0].m_pVerts[0].m_vPos.X;
	MinY = MaxY = m_pChunks[0].m_pVerts[0].m_vPos.Y;
	MinZ = MaxZ = m_pChunks[0].m_pVerts[0].m_vPos.Z;

	//--------------- loop on chunks
	for(cChunk = 0; cChunk < m_nChunks; cChunk++)
	{
		pChunk = &m_pChunks[cChunk];


		for(cVert = 1; cVert < pChunk->m_nVerts; cVert++)
		{
			MinX = MIN(MinX, pChunk->m_pVerts[cVert].m_vRenderPos.X);
			MaxX = MAX(MaxX, pChunk->m_pVerts[cVert].m_vRenderPos.X);
			MinY = MIN(MinY, pChunk->m_pVerts[cVert].m_vRenderPos.Y);
			MaxY = MAX(MaxY, pChunk->m_pVerts[cVert].m_vRenderPos.Y);
			MinZ = MIN(MinZ, pChunk->m_pVerts[cVert].m_vRenderPos.Z);
			MaxZ = MAX(MaxZ, pChunk->m_pVerts[cVert].m_vRenderPos.Z);
		}

	} // end loop on chunks


	bounds[0].Set((MaxX + MinX)/2.f, (MaxY + MinY)/2.f, (MaxZ + MinZ)/2.f);
	bounds[1].Set((MaxX - MinX), (MaxY - MinY), (MaxZ - MinZ));
}

//****************************************************************************
// works in O(n) where n <= 2*(faces + verts)

xbool object::RemoveUnusedVerts(fnTickCallback fnCallback)
{
	for(s32 cChunk = 0; cChunk < m_nChunks; cChunk++)
	{
		chunk *pChunk = &m_pChunks[cChunk];


		x_std::vector<xbool>	vertUsedList;
		x_std::vector<s32>		vertNewPosList;

		vertUsedList.	resize(pChunk->m_nVerts, false); // mark all verts as not referenced
		vertNewPosList.	resize(pChunk->m_nVerts);		 // the new index of the vert

		// ----------set vertices to initial index---------------
		for(s32 cVert = 0; cVert < pChunk->m_nVerts; cVert++)
		{
			vertNewPosList[cVert] = cVert;
		}

		if(fnCallback)
		{
			fnCallback();
		}


		//--------------- scan faces and mark all verts touched if face references them--------
		s32 highestUsedVert = -1;

		for(s32 cFace = 0; cFace < pChunk->m_nFaces; cFace++)
		{
			face& face = pChunk->m_pFaces[cFace]; // shortcut

			if(	face.m_Index[0] < 0 || face.m_Index[0] >= pChunk->m_nVerts ||
				face.m_Index[1] < 0 || face.m_Index[1] >= pChunk->m_nVerts ||
				face.m_Index[2] < 0 || face.m_Index[2] >= pChunk->m_nVerts ||
				(face.m_bQuad && (face.m_Index[3] < 0 || face.m_Index[3] >= pChunk->m_nVerts)))
			{
				x_printf("AsciiMesh::RemoveUnusedVerts FATAL: face %d in chunk %d has an invalid vertex index\n", cFace, cChunk);
				return false;
			}

			vertUsedList[face.m_Index[0]] = true; // flag the verts as used
			vertUsedList[face.m_Index[1]] = true;
			vertUsedList[face.m_Index[2]] = true;
			if(face.m_bQuad) vertUsedList[face.m_Index[3]] = true;

			// remember the highest vert index that's used
			if(face.m_Index[0] > highestUsedVert) highestUsedVert = face.m_Index[0];
			if(face.m_Index[1] > highestUsedVert) highestUsedVert = face.m_Index[1];
			if(face.m_Index[2] > highestUsedVert) highestUsedVert = face.m_Index[2];
			if(face.m_bQuad) if(face.m_Index[3] > highestUsedVert) highestUsedVert = face.m_Index[3];

		}

		if(fnCallback)
		{
			fnCallback();
		}


		//----------- move last used vert onto first unused vert until we have a smaller list with no unused verts------------
		for(cVert = 0; cVert < pChunk->m_nVerts; cVert++)
		{
			if(cVert >= highestUsedVert)
			{
				break;
			}

			// if vert is unused swap in the highest used vert
			if(!vertUsedList[cVert])
			{
				// move highest used vert into the gap
				pChunk->m_pVerts[cVert]			= pChunk->m_pVerts[highestUsedVert];
				vertNewPosList[highestUsedVert]	= cVert; // remember where we moved it

				// lower highestusedVert to the next highest used vert
				highestUsedVert--;
				while(highestUsedVert > cVert && !vertUsedList[highestUsedVert])
				{
					highestUsedVert--;
				}
			}
		}
		
			
		if(fnCallback)
		{
			fnCallback();
		}

		//------------- make faces point to correct verts------------------
		// number verts deleted = (pChunk->m_nVerts - (highestUsedVert + 1))
		if(pChunk->m_nVerts	!= highestUsedVert + 1)
		{
			pChunk->m_nVerts	= highestUsedVert + 1; // the new vert count

			for(cFace = 0; cFace < pChunk->m_nFaces; cFace++)
			{
				face& face = pChunk->m_pFaces[cFace]; // shortcut
	
				face.m_Index[0] = vertNewPosList[face.m_Index[0]];
				face.m_Index[1] = vertNewPosList[face.m_Index[1]];
				face.m_Index[2] = vertNewPosList[face.m_Index[2]];

				if (face.m_bQuad)
				{
					face.m_Index[3] = vertNewPosList[face.m_Index[3]];
				}
			}
		}
	} // end for chunk

	return true;
}

//****************************************************************************
// remaps weights from removed bones (generally to their parents)

xbool object::RemapWeights(const x_std::vector<s32>& iBonesOldToNew)
{
	if (iBonesOldToNew.size() == 0)
	{
		return TRUE;
	}

	ASSERT(m_nBones == (s32)iBonesOldToNew.size());


	u32	cBone;


	// -----------------------------------------------------------------------
	bone	*pNewBones = NULL;
	s32		nNewBones = 0;

	for(cBone = 0; cBone < iBonesOldToNew.size(); cBone++)
	{
		if (iBonesOldToNew[cBone] != -1)
		{
			nNewBones++;
		}
	}

	if (nNewBones)
	{
		pNewBones = new bone [nNewBones];

		s32	iBone = 0;
		for(cBone = 0; cBone < iBonesOldToNew.size(); cBone++)
		{
			if (iBonesOldToNew[cBone] != -1)
			{
				pNewBones[iBonesOldToNew[cBone]] = m_pBones[cBone];
			}
		}

		delete [] m_pBones;
		m_pBones = pNewBones;

		m_nBones = nNewBones;

		pNewBones = NULL;
	}



	// -----------------------------------------------------------------------
	for(s32 cChunk = 0; cChunk < m_nChunks; cChunk++)
	{
		chunk *pChunk = &m_pChunks[cChunk];

		for(s32 cVert = 0; cVert < pChunk->m_nVerts; cVert++)
		{
			vertex	*pVert = &pChunk->m_pVerts[cVert];
			s32		cWeight;



			weight	OrigWeights[kMaxNumWeights];
			s32		nOrigWeights = pVert->m_nWeights;

			x_std::copy (pVert->m_Weights, pVert->m_Weights + kMaxNumWeights, OrigWeights);

			pVert->m_nWeights = 0;
			for(cWeight = 0; cWeight < kMaxNumWeights; cWeight++)
			{
				pVert->m_Weights[cWeight].m_iMatrix = -1;
				pVert->m_Weights[cWeight].m_Weight = 0.0f;
			}


			for(cWeight = 0; cWeight < nOrigWeights; cWeight++)
			{
				weight	*pWeight = &OrigWeights[cWeight];

				ASSERT((pWeight->m_iMatrix >= 0) && (pWeight->m_iMatrix < (s32)iBonesOldToNew.size()));
				pWeight->m_iMatrix = iBonesOldToNew[pWeight->m_iMatrix];

				if (pWeight->m_iMatrix != -1)
				{
					pVert->m_Weights[pVert->m_nWeights] = *pWeight;
					pVert->m_nWeights++;
				}
//				else
//				{
//					x_printf("vert %4d is weighted to removed bone\n", cVert);
//				}
			}
		}
	}


	return TRUE;
}

//****************************************************************************
xbool object::FixPS2CrowdUVs(void)
{
//    return TRUE;

    // this function no worky right now.


    // going to assume this is only being run on a PS2 export.



    // main task, loop over the chunks, looking at each triangle
    // we need to fix the uv ranges on each triangle to go from 0.0 to 0.5 or 0.5 to 1.0
    // ranges should be in the 0.0 to 0.25, 0.25 to 0.5, 0.5 to 0.75, and 0.75 to 1.0 ranges
    // of course, they can be negative.  or greater than 1.0...  Must compensate.

    // also, this currently does NOT properly compensate for shadow splits!!!
    // That's a BIG TODO!
    s32 cChunk;
	for(cChunk = 0; cChunk < m_nChunks; ++cChunk)
	{
		chunk *pChunk = &m_pChunks[cChunk];

        if( x_strstr(pChunk->m_Name, "POLY_CROWDFAR"))
        {
            s32 *pPool = new s32[ pChunk->m_nVerts ];
            x_memset( pPool, 0, sizeof( s32 ) * pChunk->m_nVerts );

            // this is one of them.  We need to look at all of the UVs and "correct" any broken values
            s32 cT;
            for (cT=0; cT < pChunk->m_nFaces; ++cT)
            {
                face *pFace = &pChunk->m_pFaces[ cT ];
                vector2 UV[ 3 ];        // assuming only triangles.  No quads!

                UV[ 0 ] = pChunk->m_pVerts[ pFace->m_Index[ 0 ] ].m_UVs[ 0 ];
                UV[ 1 ] = pChunk->m_pVerts[ pFace->m_Index[ 1 ] ].m_UVs[ 0 ];
                UV[ 2 ] = pChunk->m_pVerts[ pFace->m_Index[ 2 ] ].m_UVs[ 0 ];

                s32 iMinIndex = -1;
                s32 iMaxIndex = -1;
                f32 fMinV = 10000.0f;
                f32 fMaxV = -10000.0f;
                s32 i;
                for (i=0; i<3; ++i)
                {
                    if (UV[ i ].Y <= fMinV)
                    {
                        iMinIndex = i;
                        fMinV = UV[ i ].Y;
                    }
                    if (UV[ i ].Y >= fMaxV)
                    {
                        iMaxIndex = i;
                        fMaxV = UV[ i ].Y;
                    }
                }

                // build the info about who uses Min and who uses Max
                xbool bUseMin[ 3 ];
                xbool bUseMax[ 3 ];
                s32 bShift;
                for (i=0; i<3; ++i)
                {
                    if (pPool[ pFace->m_Index[ i ] ] != 0)
                        bShift = pPool[ pFace->m_Index[ i ] ];
                    else
                        bShift = 0;
                }
                for (i=0; i<3; ++i)
                {
                    if ((UV[ i ].Y >= (fMinV-0.01f)) && (UV[ i ].Y <= (fMinV+0.01f)))
                        bUseMin[ i ] = TRUE;
                    else
                        bUseMin[ i ] = FALSE;
                    if ((UV[ i ].Y >= (fMaxV-0.01f)) && (UV[ i ].Y <= (fMaxV+0.01f)))
                        bUseMax[ i ] = TRUE;
                    else
                        bUseMax[ i ] = FALSE;
                }

                f32 fRemapMin = fMinV;
                f32 fRemapMax = fMaxV;

                f32 fMin = x_abs( fMinV );
                f32 fMax = x_abs( fMaxV );
                while (fMin > 1.0f)
                    fMin -= 1.0f;
                while (fMax > 1.0f)
                    fMax -= 1.0f;

                // fMin and fMax are 0.0 to 1.0
                if (   (fMin >= 0.24f && fMin <= 0.26f)
                    || (fMin >= 0.74f && fMin <= 0.76f))
                {
                    // Push Min back to a 0.5f boundary (0.0, 0.5, 1.0, etc.)
                    fRemapMin -= 0.25f;
                }
                if (   (fMax >= 0.24f && fMax <= 0.26f)
                    || (fMax >= 0.74f && fMax <= 0.76f))
                {
                    // Push Max up to a 0.5f boundary (0.0, 0.5, 1.0, etc.)
                    fRemapMax += 0.25f;
                }

                // so now, fRemap* has values that are (0.0 to 0.5), (0.5 to 1.0), etc.
                for (i=0; i<3; ++i)
                {
//                    x_printf( "UV[%d] (%3.2f,%3.2f) becomes ", pFace->m_Index[ i ], 
//                                                               pChunk->m_pVerts[ pFace->m_Index[ i ] ].m_UVs[ 0 ].X,
//                                                               pChunk->m_pVerts[ pFace->m_Index[ i ] ].m_UVs[ 0 ].Y );
                    if (bUseMin[ i ])
                    {
                        if (fRemapMin != pChunk->m_pVerts[ pFace->m_Index[ i ] ].m_UVs[ 0 ].Y)
                            pPool[ pFace->m_Index[ i ] ] = 1;
                        pChunk->m_pVerts[ pFace->m_Index[ i ] ].m_UVs[ 0 ].Y = fRemapMin;
                    }
                    else if (bUseMax[ i ])
                    {
                        if (fRemapMax != pChunk->m_pVerts[ pFace->m_Index[ i ] ].m_UVs[ 0 ].Y)
                            pPool[ pFace->m_Index[ i ] ] = 2;
                        pChunk->m_pVerts[ pFace->m_Index[ i ] ].m_UVs[ 0 ].Y = fRemapMax;
                    }
                    else if (bShift == 1)
                    {
                        pChunk->m_pVerts[ pFace->m_Index[ i ] ].m_UVs[ 0 ].Y = fRemapMin;
                    }
                    else if (bShift == 2)
                    {
                        pChunk->m_pVerts[ pFace->m_Index[ i ] ].m_UVs[ 0 ].Y = fRemapMax;
                    }
//                    x_printf( "(%3.2f,%3.2f)\n", pChunk->m_pVerts[ pFace->m_Index[ i ] ].m_UVs[ 0 ].X,
//                                                 pChunk->m_pVerts[ pFace->m_Index[ i ] ].m_UVs[ 0 ].Y );
                }
                x_printf( "\n" );
            }
        }
    }

    return TRUE;
}


}

//****************************************************************************
