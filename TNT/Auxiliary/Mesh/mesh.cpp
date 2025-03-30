/*****************************************************************************
*
*	mesh.cpp - generic mesh structure
*
*	4/14/00
*
*****************************************************************************/

#include "mesh.h"

#include "x_plus.hpp"

/*****************************************************************************/
#define ID_REFERENCE		"REFERENCE"
							
#define ID_MESH				"MESH"

#define ID_HIERARCHY		"HIERARCHY"
							
#define ID_NUM_CHUNKS		"NUM_CHUNKS"
#define ID_CHUNK			"CHUNK"
							
#define ID_VERTEX			"VERTEX"
#define ID_NORMAL			"NORMAL"
#define ID_COLOR			"COLOR"
							
#define ID_UVMAP			"UVMAP"
#define ID_WEIGHT			"WEIGHT"
							
#define ID_TEXTURE			"TEXTURE"
#define ID_SUBMATERIAL		"SUBMATERIAL"
#define ID_MATERIAL			"MATERIAL"
							
#define ID_TRI				"TRI"
#define ID_QUAD				"QUAD"

/*****************************************************************************/

#define ID_OP_OPAQUE		"OPAQUE"
#define ID_OP_PUNCH_THROUGH	"PUNCH_THROUGH"
#define ID_OP_ALPHA			"ALPHA"
#define ID_OP_EMBOSS		"EMBOSS"
#define ID_OP_DOT3			"DOT3"
#define ID_OP_SPECULAR		"SPECULAR"
#define ID_OP_DETAIL		"DETAIL"
#define ID_OP_LIGHTMAP		"LIGHTMAP"
#define ID_OP_MULT			"MULT"
#define ID_OP_ADD			"ADD"
#define ID_OP_SUB			"SUB"

#define ID_WMODE_NONE       "NONE"
#define ID_WMODE_CLAMP		"CLAMP"
#define ID_WMODE_TILE		"TILE"
#define ID_WMODE_MIRROR		"MIRROR"

/*****************************************************************************/

using namespace mesh;


struct _WrapTable
{
	char		Id[256];
	wrap_mode	Mode;
}
WrapTable[] =
{
    ID_WMODE_NONE,      WMODE_NONE,
	ID_WMODE_CLAMP,		WMODE_CLAMP,
	ID_WMODE_TILE,		WMODE_TILE,
	ID_WMODE_MIRROR,	WMODE_MIRROR,
};



struct _OpTable
{
	char		Id[256];
	operation	Mode;
}
OpTable[] =
{
	ID_OP_OPAQUE,			OP_OPAQUE,
	ID_OP_PUNCH_THROUGH,	OP_PUNCH_THROUGH,
	ID_OP_ALPHA,			OP_ALPHA,
	ID_OP_EMBOSS,			OP_EMBOSS,
	ID_OP_DOT3,				OP_DOT3,
	ID_OP_SPECULAR,			OP_SPECULAR,
	ID_OP_DETAIL,			OP_DETAIL,
	ID_OP_LIGHTMAP,			OP_LIGHTMAP,
	ID_OP_MULT,				OP_MULT,
	ID_OP_ADD,				OP_ADD,
	ID_OP_SUB,				OP_SUB,
};



/*****************************************************************************
*
*	Class:	mesh::object
*
*****************************************************************************/
object::object()
{
	m_nBones = 0;
	m_pBone = NULL;

	m_nChunks = 0;
	m_pChunk = NULL;

	m_nTextures = 0;
	m_pTexture = NULL;

	m_nSubMaterials = 0;
	m_pSubMaterial = NULL;

	m_nMaterials = 0;
	m_pMaterial = NULL;
}


object::~object()
{
	delete [] m_pBone;
	m_pBone = NULL;

	delete [] m_pChunk;
	m_pChunk = NULL;

	delete [] m_pTexture;
	m_pTexture = NULL;

	delete [] m_pSubMaterial;
	m_pSubMaterial = NULL;

	delete [] m_pMaterial;
	m_pMaterial = NULL;
}

/*****************************************************************************/
// save an object

xbool object::Save(CTextFile& Tf)
{
	Tf.BeginToken(ID_MESH, m_Name);

	// -----------------------------------------------------------------------
	// output hierarchy
	if (m_pBone)
	{
		Tf.BeginToken(ID_HIERARCHY, m_nBones);
		Tf.Put("; nChildren\tName\n");

		// output root
		Tf.Put(m_pBone[0].m_nChildren);
		Tf.Put(",\t\"");

		Tf.Put(m_pBone[0].m_Name);
		Tf.Put("\"\n");

		// recurse on children
		s32	Id = 1;
		RecurseSaveHierarchy(Tf, Id, 0, 1);

		Tf.EndToken();
	}

	// -----------------------------------------------------------------------
	// output chunks
	if (m_nChunks)
	{
		Tf.Token(ID_NUM_CHUNKS, m_nChunks);
		SaveChunks(Tf);
	}

	// -----------------------------------------------------------------------
	// output textures
	if (m_pTexture)
	{
		Tf.BeginToken(ID_TEXTURE, m_nTextures);
		Tf.Put("; W\tH\tTextureName\n");

		for(s32 cTexture = 0; cTexture < m_nTextures; cTexture++)
		{
			Tf.Put(m_pTexture[cTexture].m_Width);
			Tf.Put(",\t");

			Tf.Put(m_pTexture[cTexture].m_Height);
			Tf.Put(",\t\"");

			Tf.Put(m_pTexture[cTexture].m_FileName);
			Tf.Put("\"\n");
		}

		Tf.EndToken();
	}

	// -----------------------------------------------------------------------
	// output sub-materials
	if (m_pSubMaterial)
	{
		Tf.BeginToken(ID_SUBMATERIAL, m_nSubMaterials);
		Tf.Put("; UWrap VWrap TextureID Intensity Operation\n");

		for(s32 cSubMat = 0; cSubMat < m_nSubMaterials; cSubMat++)
		{
			sub_material	*pSubmat = &m_pSubMaterial[cSubMat];

			// output u and v wrap modes
			Tf.Put("\"");

			switch(m_pSubMaterial[cSubMat].m_WrapU)
			{
				case WMODE_CLAMP:		Tf.Put(ID_WMODE_CLAMP);		break;
				case WMODE_TILE:		Tf.Put(ID_WMODE_TILE);		break;
				case WMODE_MIRROR:		Tf.Put(ID_WMODE_MIRROR);	break;
                case WMODE_NONE:        Tf.Put(ID_WMODE_NONE);      break;
				default:				ASSERT(FALSE);
			}

			Tf.Put("\", \"");

			switch(m_pSubMaterial[cSubMat].m_WrapV)
			{
				case WMODE_CLAMP:		Tf.Put(ID_WMODE_CLAMP);		break;
				case WMODE_TILE:		Tf.Put(ID_WMODE_TILE);		break;
				case WMODE_MIRROR:		Tf.Put(ID_WMODE_MIRROR);	break;
                case WMODE_NONE:        Tf.Put(ID_WMODE_NONE);      break;
				default:				ASSERT(FALSE);
			}


			Tf.Put("\",\t");

			// output texture index
			Tf.Put(pSubmat->m_iTexture);
			Tf.Put(",\t");

			// output intensity
			Tf.Put(pSubmat->m_Intensity);
			Tf.Put(",\t\"");


			// output pass operation
			switch(m_pSubMaterial[cSubMat].m_Operation)
			{
				case OP_OPAQUE:			Tf.Put(ID_OP_OPAQUE);			break;
				case OP_PUNCH_THROUGH:	Tf.Put(ID_OP_PUNCH_THROUGH);	break;
				case OP_ALPHA:			Tf.Put(ID_OP_ALPHA);			break;
				case OP_EMBOSS:			Tf.Put(ID_OP_EMBOSS);			break;
				case OP_DOT3:			Tf.Put(ID_OP_DOT3);				break;
				case OP_SPECULAR:		Tf.Put(ID_OP_SPECULAR);			break;
				case OP_DETAIL:			Tf.Put(ID_OP_DETAIL);			break;
				case OP_LIGHTMAP:		Tf.Put(ID_OP_LIGHTMAP);			break;
				case OP_MULT:			Tf.Put(ID_OP_MULT);				break;
				case OP_ADD:			Tf.Put(ID_OP_ADD);				break;
				case OP_SUB:			Tf.Put(ID_OP_SUB);				break;
				default:				ASSERT(FALSE);
			}

			Tf.Put("\"\n");
		}

		Tf.EndToken();
	}

	// -----------------------------------------------------------------------
	// output materials
	if (m_pMaterial)
	{
		Tf.BeginToken(ID_MATERIAL, m_nMaterials);
		Tf.Put("; MaterialName   NPasses SubmaterialIDs  UVPass\n");

		for(s32 cMaterial = 0; cMaterial < m_nMaterials; cMaterial++)
		{
			s32	cPass;


			Tf.Put(fs("\"%s\",\t", m_pMaterial[cMaterial].m_Name));

			Tf.Put(m_pMaterial[cMaterial].m_nPasses);
			Tf.Put(",\t");


			// output the sub-material ids
			for(cPass = 0; cPass < m_pMaterial[cMaterial].m_nPasses; cPass++)
			{
				Tf.Put(m_pMaterial[cMaterial].m_iSubMat[cPass]);

				if (cPass < (m_pMaterial[cMaterial].m_nPasses - 1))
				{
					Tf.Put(", ");
				}
			}


			Tf.Put(",\t");


			// output the uv indices
			for(cPass = 0; cPass < m_pMaterial[cMaterial].m_nPasses; cPass++)
			{
				Tf.Put(m_pMaterial[cMaterial].m_iUV[cPass]);

				if (cPass < (m_pMaterial[cMaterial].m_nPasses - 1))
				{
					Tf.Put(", ");
				}
			}

			Tf.Put("\n");
		}

		Tf.EndToken();
	}

	// -----------------------------------------------------------------------


	Tf.EndToken();

	return(TRUE);
}

/*****************************************************************************/
// save a bone and then recurse on any children

// iBone is the index of the bone
// iParent is the index of iBone's parent
// Indent is the current indent

void object::RecurseSaveHierarchy(CTextFile& Tf, s32& iBone, s32 iParent, s32 Indent)
{
	for(s32 cChild = 0; cChild < m_pBone[iParent].m_nChildren; cChild++)
	{
		// output child count
		Tf.Put(m_pBone[iBone].m_nChildren);
		Tf.Put(",\t");

		// indent
		for(s32 cIndent = 0; cIndent < Indent; cIndent++)
			Tf.Put(" ");

		// output name
		Tf.Put(fs("\"%s\"\n", m_pBone[iBone].m_Name));

		// recurse on children
		iBone++;
		RecurseSaveHierarchy(Tf, iBone, (iBone - 1), (Indent + 1));
	}
}

/*****************************************************************************/
// save object chunks

void object::SaveChunks(CTextFile& Tf)
{
	for(s32 cChunk = 0; cChunk < m_nChunks; cChunk++)
	{
		chunk *pChunk = &m_pChunk[cChunk];

		Tf.BeginToken(ID_CHUNK, pChunk->m_Name);


		// -------------------------------------------------------------------
		// output verts
		if (pChunk->m_pVertex)
		{
			Tf.BeginToken(ID_VERTEX, pChunk->m_nVerts);
			Tf.Put("; x\ty\tz\n");

			for(s32 cVert = 0; cVert < pChunk->m_nVerts; cVert++)
			{
				Tf.Put(pChunk->m_pVertex[cVert].m_vPos);
				Tf.Put("\n");
			}

			Tf.EndToken();
		}

		// -------------------------------------------------------------------
		// output normals
		if (pChunk->m_pVertex)
		{
			Tf.BeginToken(ID_NORMAL, pChunk->m_nVerts);
			Tf.Put("; x\ty\tz\n");

			for(s32 cVert = 0; cVert < pChunk->m_nVerts; cVert++)
			{
				Tf.Put(pChunk->m_pVertex[cVert].m_vNormal);
				Tf.Put("\n");
			}

			Tf.EndToken();
		}

		// -------------------------------------------------------------------
		// output colors
		if (pChunk->m_pVertex)
		{
			s32	nColors = 0;

			// count the assigned colors
			for(s32 cVert = 0; cVert < pChunk->m_nVerts; cVert++)
			{
				if (pChunk->m_pVertex[cVert].m_Color != vector4(-1.0f, -1.0f, -1.0f, -1.0f))
				{
					nColors++;
				}
			}

			// we have some colors !
			if (nColors != 0)
			{
				Tf.BeginToken(ID_COLOR, pChunk->m_nVerts);
				Tf.Put("; r\tg\tb\n");

				for(s32 cVert = 0; cVert < pChunk->m_nVerts; cVert++)
				{
					// if the color is unassigned force it to 1
					if (pChunk->m_pVertex[cVert].m_Color == vector4(-1.0f, -1.0f, -1.0f, -1.0f))
					{
						Tf.Put(vector4(1.0f, 1.0f, 1.0f, 1.0f));
					}
					else
					{
						Tf.Put(pChunk->m_pVertex[cVert].m_Color);
					}

					Tf.Put("\n");
				}

				Tf.EndToken();
			}
		}

		// -------------------------------------------------------------------
		// output uvs
		if (pChunk->m_pVertex)
		{
			s32	Count = 0;

			// count the number of verts with uvs
			for(s32 cVert = 0; cVert < pChunk->m_nVerts; cVert++)
			{
				Count += pChunk->m_pVertex[cVert].m_nPasses;
			}

			// we have uvs - output them
			if (Count)
			{
				Tf.BeginToken(ID_UVMAP, Count);
				Tf.Put("; VIndex\tPassNum\tU\tV\n");

				// output the uv for each pass
				for(cVert = 0; cVert < pChunk->m_nVerts; cVert++)
				{
					for(s32 cPass = 0; cPass < pChunk->m_pVertex[cVert].m_nPasses; cPass++)
					{
						Tf.Put(fs("%d,\t%d,\t", cVert, cPass));

						Tf.Put(pChunk->m_pVertex[cVert].m_UV[cPass]);
						Tf.Put("\n");
					}
				}

				Tf.EndToken();
			}
		}

		// -------------------------------------------------------------------
		// output weights
		if (pChunk->m_pVertex)
		{
			s32	Count = 0;

			// count the number of weights
			for(s32 cVert = 0; cVert < pChunk->m_nVerts; cVert++)
			{
				Count += pChunk->m_pVertex[cVert].m_nWeights;
			}

			// we have weights - output them
			if (Count)
			{
				Tf.BeginToken(ID_WEIGHT, pChunk->m_nVerts);
				Tf.Put("; NMatrices\tMIndex\t%\t...\n");

				for(cVert = 0; cVert < pChunk->m_nVerts; cVert++)
				{
					if (pChunk->m_pVertex[cVert].m_nWeights > 0)
					{
						Tf.Put(fs("%d,\t", pChunk->m_pVertex[cVert].m_nWeights));

						for(s32 cWeight = 0; cWeight < pChunk->m_pVertex[cVert].m_nWeights; cWeight++)
						{
							Tf.Put(fs("%d,\t%f",
										pChunk->m_pVertex[cVert].m_Weight[cWeight].m_iMatrix,
										pChunk->m_pVertex[cVert].m_Weight[cWeight].m_Weight));

							if (cWeight < (pChunk->m_pVertex[cVert].m_nWeights - 1))
							{
								Tf.Put(",\t");
							}
						}

						Tf.Put("\n");
					}
				}

				Tf.EndToken();
			}
		}

		// -------------------------------------------------------------------
		// output faces
		if (pChunk->m_pFace)
		{
			face	*pTris = NULL,
					*pQuads = NULL;
			s32		nTris = 0,
					nQuads = 0;
			s32		cFace,
					cTri,
					cQuad;
			s32		iTri,
					iQuad;


			// count the number of quads and tris
			for(cFace = 0; cFace < pChunk->m_nFaces; cFace++)
			{
				if (pChunk->m_pFace[cFace].m_bQuad)
					nQuads++;
				else
					nTris++;
			}


			// extract the tris
			if (nTris)
			{
				pTris = new face [nTris];

				iTri = 0;
				for(cFace = 0; cFace < pChunk->m_nFaces; cFace++)
				{
					if (pChunk->m_pFace[cFace].m_bQuad == FALSE)
					{
						pTris[iTri++] = pChunk->m_pFace[cFace];
					}
				}

				ASSERT(iTri == nTris);


				// output the tris
				Tf.BeginToken(ID_TRI, nTris);
				Tf.Put("; VIndex\tVIndex\tVIndex\tMaterialIndex\n");

				for(cTri = 0; cTri < nTris; cTri++)
				{
					Tf.Put(pTris[cTri].m_Index[0]);
					Tf.Put(",\t");

					Tf.Put(pTris[cTri].m_Index[1]);
					Tf.Put(",\t");

					Tf.Put(pTris[cTri].m_Index[2]);
					Tf.Put(",\t");

					Tf.Put(pTris[cTri].m_iMaterial);
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
					if (pChunk->m_pFace[cFace].m_bQuad == TRUE)
					{
						pQuads[iQuad++] = pChunk->m_pFace[cFace];
					}
				}

				ASSERT(iQuad == nQuads);


				// output the quads
				Tf.BeginToken(ID_QUAD, nQuads);
				Tf.Put("; VIndex\tVIndex\tVIndex\tVIndex\tMaterialIndex\n");

				for(cQuad = 0; cQuad < nQuads; cQuad++)
				{
					Tf.Put(pQuads[cQuad].m_Index[0]);
					Tf.Put(",  ");

					Tf.Put(pQuads[cQuad].m_Index[1]);
					Tf.Put(",  ");

					Tf.Put(pQuads[cQuad].m_Index[2]);
					Tf.Put(",  ");

					Tf.Put(pQuads[cQuad].m_Index[3]);
					Tf.Put(",  ");

					Tf.Put(pQuads[cQuad].m_iMaterial);
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

/*****************************************************************************/
// load an object

xbool object::Load(CTextFile& Tf)
{
	CTextFile::tokentype	Token;
	xbool					bFound = FALSE;
	s32						iChunk = 0;


	ASSERT(m_pChunk == NULL);
	ASSERT(m_pTexture == NULL);
	ASSERT(m_pSubMaterial == NULL);
	ASSERT(m_pMaterial == NULL);

	while(Tf.GetToken(&Token))
	{
		// -------------------------------------------------------------------
		// read mesh
		if (Token == ID_MESH)
		{
			ASSERTS((bFound == FALSE), "file contains more than one mesh block");
			bFound = TRUE;

			// get the object name
			x_strncpy(m_Name, Token.String, sizeof(m_Name));

			Tf.MarkSection();
			while(Tf.GetSectionToken(&Token))
			{
				// -----------------------------------------------------------
				// read hierarchy
				if (Token == ID_HIERARCHY)
				{
					m_nBones = Token.Count;
					ASSERT(m_nBones);

					m_pBone = new bone [m_nBones];

					for(s32 cBone = 0; cBone < m_nBones; cBone++)
					{
						Tf.Get(&m_pBone[cBone].m_nChildren);
						Tf.Get(m_pBone[cBone].m_Name);
					}
				}

				// -----------------------------------------------------------
				// read chunk count
				else if (Token == ID_NUM_CHUNKS)
				{
					ASSERTS((m_pChunk == NULL), "mesh contains more than one chunk block");

					m_nChunks = Token.Int;
//j: should assert on the type of the number here

					ASSERT(m_nChunks);
					m_pChunk = new chunk [m_nChunks];

					iChunk = 0;
				}
				// -----------------------------------------------------------
				// read chunks
				else if (Token == ID_CHUNK)
				{
					// get the chunk name
					ASSERT(m_pChunk);
					x_strncpy(m_pChunk[iChunk].m_Name, Token.String, sizeof(m_pChunk[iChunk].m_Name));

					LoadChunk(Tf, iChunk);
					iChunk++;
				}
				// -----------------------------------------------------------
				// read textures
				else if (Token == ID_TEXTURE)
				{
					ASSERTS((m_pTexture == NULL), "mesh contains more than one texture block");

					m_nTextures = Token.Count;
					ASSERT(m_nTextures);

					m_pTexture = new texture [m_nTextures];

					for(s32 cTexture = 0; cTexture < m_nTextures; cTexture++)
					{
						Tf.Get(&m_pTexture[cTexture].m_Width);
						Tf.Get(&m_pTexture[cTexture].m_Height);

						Tf.Get(m_pTexture[cTexture].m_FileName);
					}
				}
				// -----------------------------------------------------------
				// read sub-materials
				else if (Token == ID_SUBMATERIAL)
				{
					ASSERTS((m_pSubMaterial == NULL), "mesh contains more than one sub-material block");

					m_nSubMaterials = Token.Count;
					ASSERT(m_nSubMaterials);

					m_pSubMaterial = new sub_material [m_nSubMaterials];

					for(s32 cSubMat = 0; cSubMat < m_nSubMaterials; cSubMat++)
					{
						char	Str[256];
						s32		nWrapModes = sizeof(WrapTable) / sizeof(WrapTable[0]);
						s32		cWrap;


						// get wrap u
						Tf.Get(Str);
						for(cWrap = 0; cWrap < nWrapModes; cWrap++)
						{
							if (x_strcmp(Str, WrapTable[cWrap].Id) == 0)
								break;
						}

						ASSERT(cWrap < nWrapModes);
						m_pSubMaterial[cSubMat].m_WrapU = WrapTable[cWrap].Mode;



						// get wrap v
						Tf.Get(Str);
						for(cWrap = 0; cWrap < nWrapModes; cWrap++)
						{
							if (x_strcmp(Str, WrapTable[cWrap].Id) == 0)
								break;
						}

						ASSERT(cWrap < nWrapModes);
						m_pSubMaterial[cSubMat].m_WrapV = WrapTable[cWrap].Mode;



						// get texture id
						Tf.Get(&m_pSubMaterial[cSubMat].m_iTexture);

						// get intensity
						Tf.Get(&m_pSubMaterial[cSubMat].m_Intensity);



						// get operation
						Tf.Get(Str);

						s32 nOps = sizeof(OpTable) / sizeof(OpTable[0]);
						for(s32 cOp = 0; cOp < nOps; cOp++)
						{
							if (x_strcmp(Str, OpTable[cOp].Id) == 0)
								break;
						}

						ASSERT(cOp < nOps);
						m_pSubMaterial[cSubMat].m_Operation = OpTable[cOp].Mode;
					}
				}
				// -----------------------------------------------------------
				// read materials
				else if (Token == ID_MATERIAL)
				{
					s32	cPass;


					ASSERTS((m_pMaterial == NULL), "mesh contains more than one material block");

					m_nMaterials = Token.Count;
					ASSERT(m_nMaterials);

					m_pMaterial = new material [m_nMaterials];

					for(s32 cMaterial = 0; cMaterial < m_nMaterials; cMaterial++)
					{
						// get name
						Tf.Get(m_pMaterial[cMaterial].m_Name);

						// get pass count
						Tf.Get(&m_pMaterial[cMaterial].m_nPasses);

						// get sub-material indices
						for(cPass = 0; cPass < m_pMaterial[cMaterial].m_nPasses; cPass++)
						{
							Tf.Get(&m_pMaterial[cMaterial].m_iSubMat[cPass]);
						}

						// get uv-pass indices
						for(cPass = 0; cPass < m_pMaterial[cMaterial].m_nPasses; cPass++)
						{
							Tf.Get(&m_pMaterial[cMaterial].m_iUV[cPass]);
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
		}
		// -------------------------------------------------------------------
		// skip any other data
		else
		{
			Tf.SkipSection();
		}
	}


	// check we read all the chunks
	if (bFound)
	{
		ASSERT(iChunk == m_nChunks);
	}


	return(bFound);
}

/*****************************************************************************/
// load a chunk

void object::LoadChunk(CTextFile& Tf, const s32 iChunk)
{
	CTextFile::tokentype	Token;
	chunk					*pChunk;
	face					*pTris = NULL,
							*pQuads = NULL;
	face					*pTri,
							*pQuad;
	xbool					bColors = FALSE;
	s32						nTris = 0,
							nQuads = 0;
	s32						cVert;


	ASSERT(m_pChunk);
	ASSERT(m_nChunks);
	ASSERT((iChunk >= 0) && (iChunk < m_nChunks));


	pChunk = &m_pChunk[iChunk];


	Tf.MarkSection();
	while(Tf.GetSectionToken(&Token))
	{
		// -------------------------------------------------------------------
		// read verts
		if (Token == ID_VERTEX)
		{
			// alloc the verts if we don't have them yet
			if (pChunk->m_pVertex == NULL)
			{
				pChunk->m_nVerts = Token.Count;
				ASSERT(pChunk->m_nVerts);

				pChunk->m_pVertex = new vertex [pChunk->m_nVerts];
			}

			ASSERT(Token.Count == pChunk->m_nVerts);

			// get the verts
			for(cVert = 0; cVert < pChunk->m_nVerts; cVert++)
			{
				Tf.Get(&pChunk->m_pVertex[cVert].m_vPos);
			}
		}

		// -------------------------------------------------------------------
		// read normals
		else if (Token == ID_NORMAL)
		{
			// alloc the verts if we don't have them yet
			if (pChunk->m_pVertex == NULL)
			{
				pChunk->m_nVerts = Token.Count;
				ASSERT(pChunk->m_nVerts);

				pChunk->m_pVertex = new vertex [pChunk->m_nVerts];
			}

			ASSERT(Token.Count == pChunk->m_nVerts);


			// get the normals
			for(cVert = 0; cVert < pChunk->m_nVerts; cVert++)
			{
				Tf.Get(&pChunk->m_pVertex[cVert].m_vNormal);
			}
		}

		// ---------------------------------------------------------------
		// read colors
		else if (Token == ID_COLOR)
		{
			// alloc the verts if we don't have them yet
			if (pChunk->m_pVertex == NULL)
			{
				pChunk->m_nVerts = Token.Count;
				ASSERT(pChunk->m_nVerts);

				pChunk->m_pVertex = new vertex [pChunk->m_nVerts];
			}

			ASSERT(Token.Count == pChunk->m_nVerts);

			// get the colors
			for(cVert = 0; cVert < pChunk->m_nVerts; cVert++)
			{
				Tf.Get(&pChunk->m_pVertex[cVert].m_Color);
			}

			bColors = TRUE;
		}

		// ---------------------------------------------------------------
		// read uvs
		else if (Token == ID_UVMAP)
		{
			// alloc the verts if we don't have them yet
			if (pChunk->m_pVertex == NULL)
			{
				pChunk->m_nVerts = Token.Count;
				ASSERT(pChunk->m_nVerts);

				pChunk->m_pVertex = new vertex [pChunk->m_nVerts];
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
				ASSERT((Pass >= 0) && (Pass < MAX_PASSES));

				pVert = &pChunk->m_pVertex[iVert];
				ASSERT(pVert->m_nPasses < MAX_PASSES);

				// get the uv
				Tf.Get(&pVert->m_UV[Pass]);

				pVert->m_nPasses++;
			}
		}

		// ---------------------------------------------------------------
		// read vertex weights
		else if (Token == ID_WEIGHT)
		{
			// alloc the verts if we don't have them yet
			if (pChunk->m_pVertex == NULL)
			{
				pChunk->m_nVerts = Token.Count;
				ASSERT(pChunk->m_nVerts);

				pChunk->m_pVertex = new vertex [pChunk->m_nVerts];
			}

			ASSERT(Token.Count == pChunk->m_nVerts);


			for(s32 cVert = 0; cVert < pChunk->m_nVerts; cVert++)
			{
				s32	nWeights;


				Tf.Get(&nWeights);
				ASSERT(nWeights <= MAX_WEIGHTS);


				pChunk->m_pVertex[cVert].m_nWeights = nWeights;

				for(s32 cWeight = 0; cWeight < nWeights; cWeight++)
				{
					Tf.Get(&pChunk->m_pVertex[cVert].m_Weight[cWeight].m_iMatrix);
					Tf.Get(&pChunk->m_pVertex[cVert].m_Weight[cWeight].m_Weight);
				}
			}
		}

		// ---------------------------------------------------------------
		// read tri-faces
		else if (Token == ID_TRI)
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

				Tf.Get(&pTri->m_iMaterial);
			}
		}

		// ---------------------------------------------------------------
		// read quad-faces
		else if (Token == ID_QUAD)
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



	// -------------------------------------------------------------------
	// verts have no assigned colors - force to 1
	if (bColors == FALSE)
	{
		for(cVert = 0; cVert < pChunk->m_nVerts; cVert++)
		{
			pChunk->m_pVertex[cVert].m_Color = vector4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}



	// -------------------------------------------------------------------
	// merge quads and tris into one list of facets
	if ((pTris) || (pQuads))
	{
		s32	iFace = 0;


		pChunk->m_nFaces = (nTris + nQuads);
		pChunk->m_pFace = new face [pChunk->m_nFaces];

		for(s32 cTri = 0; cTri < nTris; cTri++)
		{
			pChunk->m_pFace[iFace++] = pTris[cTri];
		}

		for(s32 cQuad = 0; cQuad < nQuads; cQuad++)
		{
			pChunk->m_pFace[iFace++] = pQuads[cQuad];
		}

		// free the separate tris and quads pool
		delete [] pTris;
		pTris = NULL;

		delete [] pQuads;
		pQuads = NULL;
	}
}

/*****************************************************************************/
