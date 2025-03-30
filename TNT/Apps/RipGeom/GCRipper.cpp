////////////////////////////////////////////////////////////////////////////
//
// GCRipper.cpp
//
//
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>

#include "x_plus.hpp"
#include "x_memory.hpp"

#include "GameCube/GC_Geom.hpp"
#include "Q_Geom.hpp"
#include "AUX_Bitmap.hpp"

#include "RipGeom.h"
#include "Strip.h"
#include "AsciiMesh.h"

//#define DOLPHIN_SDK_INSTALLED

#if defined( DOLPHIN_SDK_INSTALLED )
	#define __TYPES_H__                   // To remove duplicate typedefs of basic types
	#include <Dolphin/GX/GXEnum.h>        // for GX_VTXFMT0
	#include <Dolphin/GX/GXCommandList.h> // for GX_DRAW_* commands

#else
	// NOTE: these commands MUST match what's in GXEnum.h and GXCommandList.h
	#define GX_NOP                  0x00
	#define GX_DRAW_TRIANGLES       0x90
	#define GX_DRAW_TRIANGLE_STRIP  0x98
	#define GX_VTXFMT0              0x00

#endif // defined( DOLPHIN_SDK_INSTALLED )

////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////

#define MAX_NUM_MESHES				64
#define MAX_NUM_SUBMESHES			1024
#define MAX_VERT_COUNT				65504

#define MAX_TOTAL_VERT_DATA_SIZE	(7 * 1024 * 1024)	// 7 MB vert data
#define MAX_TOTAL_DLIST_DATA_SIZE	(3 * 1024 * 1024)	// 3 MB display list data
#define MAX_NUM_TRIS_IN_SUBMESH 100000


#define IS_ALIGNED_32( V )	(((u32)(V) & 0x1F) == 0)


static s32              s_StartTris[MAX_NUM_SUBMESHES]; 
static s32              s_RealMeshIDs[MAX_NUM_SUBMESHES]; 

static s32              s_SubMeshTriData[MAX_NUM_TRIS_IN_SUBMESH];


////////////////////////////////////////////////////////////////////////////
// GAMECUBE ENDIAN SWAPPING
////////////////////////////////////////////////////////////////////////////

template< class TYPE >
inline TYPE GC_ENDIAN_16( TYPE V )
{
    u16 EndV = ( ((*(u16*)&(V)) >> 8) |
                 ((*(u16*)&(V)) << 8) );

    return *((TYPE*)&EndV);
}

//--------------------------------------------------------------------------

template< class TYPE >
inline TYPE GC_ENDIAN_32( TYPE V )
{
    u32 EndV = ( ((*(u32*)&(V)) >> 24) |
                 ((*(u32*)&(V)) << 24) |
                 (((*(u32*)&(V)) & 0x00FF0000) >> 8) |
                 (((*(u32*)&(V)) & 0x0000FF00) << 8) );

    return *((TYPE*)&EndV);
}


////////////////////////////////////////////////////////////////////////////
// STRUCTURES
////////////////////////////////////////////////////////////////////////////

#pragma pack(1)

struct SDListHeader
{
	u8		Cmd;
	u16		VertCount;
};

#pragma pack()


typedef struct SDListHeader t_DListHeader;


////////////////////////////////////////////////////////////////////////////
// GLOBALS
////////////////////////////////////////////////////////////////////////////

static byte*			s_pVertexData;
static byte*			s_pDispListData;
static u32				s_CurVertexDataSize;
static u32				s_CurDispListDataSize;

static t_GeomMesh*		s_pMeshData;
static t_GeomSubMesh*	s_pSubMeshData;
static s32				s_CurSubMesh;

static t_Triangle*		s_pTriData;
static s32				s_NTris;

static mesh::material_set*    s_pMaterialSet = NULL;


////////////////////////////////////////////////////////////////////////////
// PROTOTYPES
////////////////////////////////////////////////////////////////////////////

static s32   GetTextureID         ( s32 TriID, mesh::object& rGeom );
static void  PadVertexData        ( u32 DataSize );

static void  SaveMeshVertsPosition( t_GeomMesh* pMesh, mesh::object& rGeom, s32 MeshIndex );
static void  SaveMeshVertsNormal  ( t_GeomMesh* pMesh, mesh::object& rGeom, s32 MeshIndex );
static void  SaveMeshVertsColor   ( t_GeomMesh* pMesh, mesh::object& rGeom, s32 MeshIndex );

static void  BuildSubMeshStrips   ( t_GeomMesh* pMesh, mesh::object& rGeom, s32 StartTri, s32 NTris, s32 MeshIndex );
static void  BuildSubMeshTris     ( t_GeomMesh* pMesh, mesh::object& rGeom, s32 StartTri, s32 NTris, s32 MeshIndex );
static void  BuildSubMeshShadow   ( t_GeomMesh* pMesh, mesh::object& rGeom, s32 StartTri, s32 NTris, s32 MeshIndex );

static void  CopyTriData          ( mesh::chunk& rMesh );
static s32   CompareMaterialIDs   ( const void* Element1, const void* Element2 );
static void  RipSubMeshData       ( s32 MeshIndex, t_GeomMesh* pMesh, mesh::object& rGeom );

static f32   CalculateMeshRadius  ( mesh::object& rGeom, s32 MeshID );
static void  GetTextureName       ( char* TextureName, mesh::object& rGeom, s32 Index );
static void  FillName             ( char* Name, char* src, s32 NChars );
static void  PadStringWithZero    ( char *str, s32 length );
static xbool IsShadow             ( char* MeshName );
static xbool IsEnvMapped          ( char* MeshName );

static void  ExportTextures       ( mesh::object& rGeom, X_FILE* pOutfile );
static void  EndianConvertData    ( t_GeomFileHeader* pHdr, s32 NMeshes, s32 NSubMeshes );


void ExportGCGeometry( mesh::object& rGeom, X_FILE* pOutfile );



////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////

static s32 GetTextureID( s32 TriID, mesh::object& rGeom )
{
    s32 MaterialID;

    MaterialID = s_pTriData[s_SubMeshTriData[0]].MaterialID;
    if ( MaterialID >= 0 )
    {
        mesh::material*         pMaterial;
        mesh::stage*     pStage;

        pMaterial = &s_pMaterialSet->m_pMaterials[MaterialID];
        pStage = &s_pMaterialSet->m_pStages[pMaterial->m_iStages[0]];

        if( pStage->m_iTexture < 0 )
            return 0;
        return pStage->m_iTexture;
    }
    else
    {
        return 0;
    }
}

//==========================================================================

static void PadVertexData( u32 DataSize )
{
	//--- pads the vertex data array to 32B, so the next mesh's vertex data is aligned on 32B

	if( !IS_ALIGNED_32( DataSize ) )
	{
		DataSize += 0x20 - (DataSize & 0x1F);
	}

	s_CurVertexDataSize += DataSize;

	ASSERT( s_CurVertexDataSize < MAX_TOTAL_VERT_DATA_SIZE );
}

//==========================================================================

static void SaveMeshVertsPosition( t_GeomMesh* pMesh, mesh::object& rGeom, s32 MeshIndex )
{
	s32				i;
	s32				NVerts    = rGeom.m_pChunks[MeshIndex].m_nVerts;
	mesh::vertex*	pMeshVert = rGeom.m_pChunks[MeshIndex].m_pVerts;
	vector3*		pVertData = (vector3*)(s_pVertexData + s_CurVertexDataSize);

	//--- fill in appropriate mesh data
	pMesh->NVerts     = NVerts;
	pMesh->VertFormat = GC_VTXFMT_POS_ONLY;
	pMesh->pVertData  = (void*)s_CurVertexDataSize; //stored as offset

	//--- copy the vertex data
	for( i = 0; i < NVerts; i++ )
	{
		pVertData[i].X = GC_ENDIAN_32( pMeshVert[i].m_vPos.X );
		pVertData[i].Y = GC_ENDIAN_32( pMeshVert[i].m_vPos.Y );
		pVertData[i].Z = GC_ENDIAN_32( pMeshVert[i].m_vPos.Z );
	}

	//--- pad the vertex array
	PadVertexData( NVerts * sizeof(vector3) );

	//--- print out some stats
	if( g_Verbose )
	{
		x_printf( "  Mesh #%3d:  %4d Verts   %6d bytes\n",
				  MeshIndex, NVerts, (s32)(NVerts * sizeof(vector3)) );
	}

}

//==========================================================================

static void SaveMeshVertsNormal( t_GeomMesh* pMesh, mesh::object& rGeom, s32 MeshIndex )
{
	s32				i;
	s32				NVerts    = rGeom.m_pChunks[MeshIndex].m_nVerts;
	mesh::vertex*	pMeshVert = rGeom.m_pChunks[MeshIndex].m_pVerts;
	t_GCVertNormal*	pVertData = (t_GCVertNormal*)(s_pVertexData + s_CurVertexDataSize);

	//--- fill in appropriate mesh data
	pMesh->NVerts     = NVerts;
	pMesh->VertFormat = GC_VTXFMT_NORMAL;
	pMesh->pVertData  = (void*)s_CurVertexDataSize; //stored as offset

	//--- copy the vertex data
	for( i = 0; i < NVerts; i++ )
	{
		pVertData[i].Pos.X    = GC_ENDIAN_32( pMeshVert[i].m_vPos.X );
		pVertData[i].Pos.Y    = GC_ENDIAN_32( pMeshVert[i].m_vPos.Y );
		pVertData[i].Pos.Z    = GC_ENDIAN_32( pMeshVert[i].m_vPos.Z );

		pVertData[i].Normal.X = GC_ENDIAN_32( pMeshVert[i].m_vNormal.X );
		pVertData[i].Normal.Y = GC_ENDIAN_32( pMeshVert[i].m_vNormal.Y );
		pVertData[i].Normal.Z = GC_ENDIAN_32( pMeshVert[i].m_vNormal.Z );

		pVertData[i].UV.X     = GC_ENDIAN_32( pMeshVert[i].m_UVs[0].X );
		pVertData[i].UV.Y     = GC_ENDIAN_32( pMeshVert[i].m_UVs[0].Y );
	}

	//--- pad the vertex array
	PadVertexData( NVerts * sizeof(t_GCVertNormal) );

	//--- print out some stats
	if( g_Verbose )
	{
		x_printf( "  Mesh #%3d:  %4d Verts   %6d bytes\n",
				  MeshIndex, NVerts, (s32)(NVerts * sizeof(t_GCVertNormal)) );
	}

}

//==========================================================================

static void SaveMeshVertsColor( t_GeomMesh* pMesh, mesh::object& rGeom, s32 MeshIndex )
{
	s32				i;
	s32				NVerts    = rGeom.m_pChunks[MeshIndex].m_nVerts;
	mesh::vertex*	pMeshVert = rGeom.m_pChunks[MeshIndex].m_pVerts;
	t_GCVertColor*	pVertData = (t_GCVertColor*)(s_pVertexData + s_CurVertexDataSize);

	//--- fill in appropriate mesh data
	pMesh->NVerts     = NVerts;
	pMesh->VertFormat = GC_VTXFMT_COLOR;
	pMesh->pVertData  = (void*)s_CurVertexDataSize; //stored as offset

	//--- copy the vertex data
	for( i = 0; i < NVerts; i++ )
	{
		pVertData[i].Pos.X   = GC_ENDIAN_32( pMeshVert[i].m_vPos.X );
		pVertData[i].Pos.Y   = GC_ENDIAN_32( pMeshVert[i].m_vPos.Y );
		pVertData[i].Pos.Z   = GC_ENDIAN_32( pMeshVert[i].m_vPos.Z );

		pVertData[i].Color.A = (u8)(pMeshVert[i].m_Color.W * 255.0f);
		pVertData[i].Color.R = (u8)(pMeshVert[i].m_Color.X * 255.0f);
		pVertData[i].Color.G = (u8)(pMeshVert[i].m_Color.Y * 255.0f);
		pVertData[i].Color.B = (u8)(pMeshVert[i].m_Color.Z * 255.0f);

		pVertData[i].UV.X    = GC_ENDIAN_32( pMeshVert[i].m_UVs[0].X );
		pVertData[i].UV.Y    = GC_ENDIAN_32( pMeshVert[i].m_UVs[0].Y );
	}

	//--- pad the vertex array
	PadVertexData( NVerts * sizeof(t_GCVertColor) );

	//--- print out some stats
	if( g_Verbose )
	{
		x_printf( "  Mesh #%3d:  %4d Verts   %6d bytes\n",
				  MeshIndex, NVerts, (s32)(NVerts * sizeof(t_GCVertColor)) );
	}

}

//==========================================================================

static void PadDisplayList( t_GeomSubMesh* pSubMesh, byte* pDList )
{
	//--- store offset into display list data, calc display list size
	pSubMesh->pDispList    = (void*)s_CurDispListDataSize;
	pSubMesh->DispListSize = (u32)(pDList - &s_pDispListData[s_CurDispListDataSize]);

	//--- if display list size is not 32B aligned, pad with NOPs
	if( !IS_ALIGNED_32( pSubMesh->DispListSize ) )
	{
		u32 i;
		u32 DListPad = 0x20 - (pSubMesh->DispListSize & 0x1F);

		for( i = 0; i < DListPad; i++ )
		{
			pDList[i] = GX_NOP;
		}

		pSubMesh->DispListSize += DListPad;
	}

	//--- update total size of display list data
	s_CurDispListDataSize += pSubMesh->DispListSize;

	ASSERT( s_CurDispListDataSize < MAX_TOTAL_DLIST_DATA_SIZE );
}

//==========================================================================

static void BuildSubMeshStrips( t_GeomMesh* pMesh, mesh::object& rGeom, s32 StartTri, s32 NTris, s32 MeshIndex )
{
	s32 				i, CurStrip;
	s32 				NVerts;
	s32 				NStrips;
	t_TriangleStrip*	pStrips;
    s32                 nTris = 0;
    t_Triangle*         pTris = NULL;
	t_TriangleStrip*	pCurStrip;
	byte*				pDList;

	//--- Build the triangle strips
	if( g_Verbose )
		x_printf( "Building triangle strips...(this may take a while)\n" );

//	BuildStrips( &s_pTriData[StartTri], NTris, &pStrips, NStrips, MAX_VERT_COUNT );


	//--- get pointer to display list data
	pDList = s_pDispListData + s_CurDispListDataSize;

	NVerts = 0;

	for( CurStrip = 0; CurStrip < NStrips; CurStrip++ )
	{
		pCurStrip = &pStrips[CurStrip];

		NVerts += pCurStrip->NVerts;
		ASSERT( NVerts < MAX_VERT_COUNT );

		//--- fill in display list header for this strip
		((t_DListHeader*)pDList)->Cmd       = GX_DRAW_TRIANGLE_STRIP | GX_VTXFMT0;
		((t_DListHeader*)pDList)->VertCount = GC_ENDIAN_16( (u16)pCurStrip->NVerts );

		pDList += sizeof(t_DListHeader);

		//--- put the tri indices in the display list for this strip
		for( i = 0; i < pCurStrip->NVerts; i++ )
		{
			((u16*)pDList)[0] = GC_ENDIAN_16( (u16)(pCurStrip->pVerts[i]) ); //index for position
			((u16*)pDList)[1] = GC_ENDIAN_16( (u16)(pCurStrip->pVerts[i]) ); //index for normal/color
			((u16*)pDList)[2] = GC_ENDIAN_16( (u16)(pCurStrip->pVerts[i]) ); //index for UV

			pDList += sizeof(u16) * 3;
		}
	}


	//--- figure out which texture to use
	s32 TextureID = GetTextureID( StartTri, rGeom );

	//--- set submesh values
	s_pSubMeshData[s_CurSubMesh].Flags        = (pMesh->Flags & MESH_FLAG_ENVMAPPED) ? SUBMESH_FLAG_ENVMAP : 0;
	s_pSubMeshData[s_CurSubMesh].TextureID    = TextureID;
	s_pSubMeshData[s_CurSubMesh].NVerts       = NVerts;
	s_pSubMeshData[s_CurSubMesh].NTris        = NTris;

	//--- pad out display list to 32B
	PadDisplayList( &s_pSubMeshData[s_CurSubMesh], pDList );


	//--- print out some stats
	if( g_Verbose )
	{
		x_printf( "    SubMesh #%3d: %2d Strips %4d Tris  %1.2f Verts/Tri %5d bytes Tex. %d\n",
				  s_CurSubMesh,
				  NStrips,
				  s_pSubMeshData[s_CurSubMesh].NTris,
				  (f32)NVerts / (f32)NTris,
				  s_pSubMeshData[s_CurSubMesh].DispListSize,
				  s_pSubMeshData[s_CurSubMesh].TextureID );
	}

	//--- free the data which was allocated for the strips
	for( i = 0; i < NStrips; i++ )
	{
		x_free( pStrips[i].pVerts );
	}
	x_free( pStrips );
}

//==========================================================================

static void BuildSubMeshTris( t_GeomMesh* pMesh, mesh::object& rGeom, s32 StartTri, s32 NTris, s32 MeshIndex )
{
	s32		i;
	byte*	pDList;

	//--- get pointer to display list data
	pDList = s_pDispListData + s_CurDispListDataSize;

	ASSERT( (NTris *3) < MAX_VERT_COUNT );

	//--- fill in display list header
	((t_DListHeader*)pDList)->Cmd       = GX_DRAW_TRIANGLES | GX_VTXFMT0;
	((t_DListHeader*)pDList)->VertCount = GC_ENDIAN_16( (u16)(NTris * 3) );

	pDList += sizeof(t_DListHeader);

	//--- put the tri indices in the display list
	for( i = 0; i < NTris; i++ )
	{
		t_Triangle* pTri = &s_pTriData[ StartTri + i ];

		((u16*)pDList)[0] = GC_ENDIAN_16( (u16)(pTri->Index[0]) ); //vertex 0, index for position
		((u16*)pDList)[1] = GC_ENDIAN_16( (u16)(pTri->Index[0]) ); //vertex 0, index for normal/color
		((u16*)pDList)[2] = GC_ENDIAN_16( (u16)(pTri->Index[0]) ); //vertex 0, index for UV

		((u16*)pDList)[3] = GC_ENDIAN_16( (u16)(pTri->Index[1]) ); //vertex 1, index for position
		((u16*)pDList)[4] = GC_ENDIAN_16( (u16)(pTri->Index[1]) ); //vertex 1, index for normal/color
		((u16*)pDList)[5] = GC_ENDIAN_16( (u16)(pTri->Index[1]) ); //vertex 1, index for UV

		((u16*)pDList)[6] = GC_ENDIAN_16( (u16)(pTri->Index[2]) ); //vertex 2, index for position
		((u16*)pDList)[7] = GC_ENDIAN_16( (u16)(pTri->Index[2]) ); //vertex 2, index for normal/color
		((u16*)pDList)[8] = GC_ENDIAN_16( (u16)(pTri->Index[2]) ); //vertex 2, index for UV

		pDList += sizeof(u16) * 9;
	}

	//--- figure out which texture to use
	s32 TextureID = GetTextureID( StartTri, rGeom );

	//--- set submesh values
	s_pSubMeshData[s_CurSubMesh].Flags        = 0;
	s_pSubMeshData[s_CurSubMesh].TextureID    = TextureID;
	s_pSubMeshData[s_CurSubMesh].NVerts       = NTris * 3;
	s_pSubMeshData[s_CurSubMesh].NTris        = NTris;

	//--- pad out display list to 32B
	PadDisplayList( &s_pSubMeshData[s_CurSubMesh], pDList );


	//--- print out some stats
	if( g_Verbose )
	{
		x_printf( "    SubMesh #%3d:  %4d Tris   %6d bytes   Texture %d\n",
				  s_CurSubMesh,
				  s_pSubMeshData[s_CurSubMesh].NTris,
				  s_pSubMeshData[s_CurSubMesh].DispListSize,
				  s_pSubMeshData[s_CurSubMesh].TextureID );
	}
}

//==========================================================================

static void BuildSubMeshShadow( t_GeomMesh* pMesh, mesh::object& rGeom, s32 StartTri, s32 NTris, s32 MeshIndex )
{
	s32		i;
	byte*	pDList;

	//--- get pointer to display list data
	pDList = s_pDispListData + s_CurDispListDataSize;

	ASSERT( (NTris *3) < MAX_VERT_COUNT );

	//--- fill in display list header
	((t_DListHeader*)pDList)->Cmd       = GX_DRAW_TRIANGLES | GX_VTXFMT0;
	((t_DListHeader*)pDList)->VertCount = GC_ENDIAN_16( (u16)(NTris * 3) );

	pDList += sizeof(t_DListHeader);

	//--- put the tri indices in the display list
	for( i = 0; i < NTris; i++ )
	{
		t_Triangle* pTri = &s_pTriData[ StartTri + i ];

		((u16*)pDList)[0] = GC_ENDIAN_16( (u16)(pTri->Index[0]) );
		((u16*)pDList)[1] = GC_ENDIAN_16( (u16)(pTri->Index[1]) );
		((u16*)pDList)[2] = GC_ENDIAN_16( (u16)(pTri->Index[2]) );

		pDList += sizeof(u16) * 3;
	}

	//--- set submesh values
	s_pSubMeshData[s_CurSubMesh].Flags        = 0;
	s_pSubMeshData[s_CurSubMesh].TextureID    = -1;
	s_pSubMeshData[s_CurSubMesh].NVerts       = NTris * 3;
	s_pSubMeshData[s_CurSubMesh].NTris        = NTris;

	//--- pad out display list to 32B
	PadDisplayList( &s_pSubMeshData[s_CurSubMesh], pDList );


	//--- print out some stats
	if( g_Verbose )
	{
		x_printf( "    SubMesh #%3d:  %4d Tris   %6d bytes\n",
				  s_CurSubMesh,
				  s_pSubMeshData[s_CurSubMesh].NTris,
				  s_pSubMeshData[s_CurSubMesh].DispListSize );
	}
}

//==========================================================================

static void CopyTriData( mesh::chunk& rMesh )
{
	s32 i;
	s32 count = 0;

	//--- count the number of triangles we need
	for( i = 0; i < rMesh.m_nFaces; i++ )
	{
		if( rMesh.m_pFaces[i].m_bQuad )
			count += 2;
		else
			count++;
	}

	//--- allocate space for the triangle data
	s_pTriData = (t_Triangle*)x_malloc( sizeof(t_Triangle) * count );
	ASSERT( s_pTriData != NULL );

	//--- copy the triangle data
	count = 0;
	for( i = 0; i < rMesh.m_nFaces; i++ )
	{
		if( rMesh.m_pFaces[i].m_bQuad )
		{
			//--- copy a quad, splitting it into two tris
			s_pTriData[count].Index[0]   = rMesh.m_pFaces[i].m_Index[0];
			s_pTriData[count].Index[1]   = rMesh.m_pFaces[i].m_Index[1];
			s_pTriData[count].Index[2]   = rMesh.m_pFaces[i].m_Index[2];
			s_pTriData[count].MaterialID = rMesh.m_pFaces[i].m_iMaterial;
			count++;
			s_pTriData[count].Index[0]   = rMesh.m_pFaces[i].m_Index[0];
			s_pTriData[count].Index[1]   = rMesh.m_pFaces[i].m_Index[2];
			s_pTriData[count].Index[2]   = rMesh.m_pFaces[i].m_Index[3];
			s_pTriData[count].MaterialID = rMesh.m_pFaces[i].m_iMaterial;
			count++;
		}
		else
		{
			if( (rMesh.m_pFaces[i].m_Index[0] == rMesh.m_pFaces[i].m_Index[1]) ||
				(rMesh.m_pFaces[i].m_Index[0] == rMesh.m_pFaces[i].m_Index[2]) ||
				(rMesh.m_pFaces[i].m_Index[1] == rMesh.m_pFaces[i].m_Index[2]) )
			{
				//--- skip degenerate triangles
				continue;
			}
			//--- just copy the tri
			s_pTriData[count].Index[0]   = rMesh.m_pFaces[i].m_Index[0];
			s_pTriData[count].Index[1]   = rMesh.m_pFaces[i].m_Index[1];
			s_pTriData[count].Index[2]   = rMesh.m_pFaces[i].m_Index[2];
			s_pTriData[count].MaterialID = rMesh.m_pFaces[i].m_iMaterial;
			count++;
		}
	}

	s_NTris = count;
}

//==========================================================================

static s32 CompareMaterialIDs( const void* Element1, const void* Element2 )
{
	if( ((t_Triangle*)Element1)->MaterialID < ((t_Triangle*)Element2)->MaterialID )
	{
		return -1;
	}
	else if( ((t_Triangle*)Element1)->MaterialID > ((t_Triangle*)Element2)->MaterialID )
	{
		return 1;
	}

	return 0;
}

//==========================================================================

static void RipSubMeshData( s32 MeshIndex, t_GeomMesh* pMesh, mesh::object& rGeom )
{
	s32 NTotalTris = 0;
	s32 TriCount;
	s32 CurMaterial;

	//--- create a workable copy of the triangle data
	CopyTriData( rGeom.m_pChunks[MeshIndex] );

	//--- sort the triangles by their material IDs
	x_qsort( s_pTriData, s_NTris, sizeof(t_Triangle), CompareMaterialIDs );

	//--- save out mesh vertex data
	if( pMesh->Flags & MESH_FLAG_SHADOW )
	{
		SaveMeshVertsPosition( pMesh, rGeom, MeshIndex );
	}
	else if( pMesh->Flags & MESH_FLAG_DYNAMIC_LIGHTING )
	{
		SaveMeshVertsNormal( pMesh, rGeom, MeshIndex );
	}
	else
	{
		SaveMeshVertsColor( pMesh, rGeom, MeshIndex );
	}

	//--- each set of faces with a unique material becomes a submesh
	while( NTotalTris < s_NTris )
	{
		//--- count the number of tris using the next material
		TriCount = NTotalTris + 1;
		CurMaterial = s_pTriData[ NTotalTris ].MaterialID;
		while( (s_pTriData[ TriCount ].MaterialID == CurMaterial) && (TriCount < s_NTris) )
		{
			TriCount++;
		}

		TriCount -= NTotalTris;

		//--- build SubMeshes using this material out of the next set of tris
		if( pMesh->Flags & MESH_FLAG_SHADOW )
		{
			BuildSubMeshShadow( pMesh, rGeom, NTotalTris, TriCount, MeshIndex );
		}
		else
		{
			if( g_BuildTriStrips )
				BuildSubMeshStrips( pMesh, rGeom, NTotalTris, TriCount, MeshIndex );
			else
				BuildSubMeshTris( pMesh, rGeom, NTotalTris, TriCount, MeshIndex );
		}

		NTotalTris += TriCount;

		//--- increment the mesh's submesh count
		pMesh->NSubMeshes++;

		//--- update the current total submesh count
		s_CurSubMesh++;
		ASSERT( s_CurSubMesh < MAX_NUM_SUBMESHES );
	}

	//--- free up the tri data
	x_free( s_pTriData );
}

//==========================================================================

static f32 CalculateMeshRadius( mesh::object& rGeom, s32 MeshID )
{
	s32		 i;
	f32		 Radius = 0.0f;
	f32		 length;
	vector3* v;

	for( i = 0; i < rGeom.m_pChunks[MeshID].m_nVerts; i++ )
	{
		v = &rGeom.m_pChunks[MeshID].m_pVerts[i].m_vPos;
		length = v->Length();

		if( length > Radius )
			Radius = length;
	}

	return Radius;
}

//==========================================================================

static void GetTextureName( char* TextureName, mesh::object& rGeom, s32 Index )
{
	char Drive[X_MAX_DRIVE];
	char Dir[X_MAX_DIR];
	char File[X_MAX_FNAME];
	char Ext[X_MAX_EXT];
	
	x_splitpath( s_pMaterialSet->m_pTextures[Index].m_Filename, Drive, Dir, File, Ext );

	x_strcpy( TextureName, File );
}

//==========================================================================

static void FillName( char* Name, char* src, s32 NChars )
{
	s32 length = x_strlen( src );
	s32 start;

	if( length < NChars )
		start = 0;
	else
		start = length - NChars + 1;

	x_strcpy( Name, &src[start] );
}

//==========================================================================

static void PadStringWithZero( char *str, s32 length )
{
	for( s32 i = x_strlen(str); i < length; i++ )
	{
		str[i] = '\0';
	}
}

//==========================================================================

static xbool IsShadow( char* MeshName )
{
	s32 i = x_strlen( MeshName ) - x_strlen( "_shadow" );

	if( i < 0 )
		return FALSE;

	if( !x_stricmp( &MeshName[i], "_shadow" ) )
		return TRUE;

	return FALSE;
}

//==========================================================================

static xbool IsEnvMapped( char* MeshName )
{
	s32 i = x_strlen( MeshName ) - x_strlen( "_env" );

	if( i < 0 )
		return FALSE;

	if( !x_stricmp( &MeshName[i], "_env" ) )
		return TRUE;

	return FALSE;
}

//==========================================================================

static void ExportTextures( mesh::object& rGeom, X_FILE* pOutfile )
{
	s32 i;

	//--- Export the texture names to a text file
	if( g_TextureNameFile )
	{
		for( i = 0; i < s_pMaterialSet->m_nTextures; i++ )
		{
            char NewDrive[5];
            char NewDir[255];
            char NewFileName[80];
            char NewExtension[80];
            u8   AlphaPercent = 255;;
            x_splitpath( s_pMaterialSet->m_pTextures[i].m_Filename, (char*)&NewDrive, (char*)&NewDir, (char*)&NewFileName, (char*)&NewExtension );

            // Force the extension to be a BMP then rebuild, and re-split the path.
            x_strcpy( NewExtension, "BMP" );
            x_makepath(  s_pMaterialSet->m_pTextures[i].m_Filename, (char*)&NewDrive, (char*)&NewDir, (char*)&NewFileName, (char*)&NewExtension );
            x_splitpath( s_pMaterialSet->m_pTextures[i].m_Filename, (char*)&NewDrive, (char*)&NewDir, (char*)&NewFileName, (char*)&NewExtension );
            
            x_sprintf( NewFileName, "%s", x_strtoupper(NewFileName) );

            if( NewFileName[2] >= '0' && NewFileName[2] <= '9' )
            {
                AlphaPercent = (u8)(255 * ((NewFileName[2] - '0') * .1));
            }

            // now check to see if there is a 24-bit version of this texture so we can use that on the all mighty Xbox
            char Check24bit[80];
            char Check24bitFull[255];
            x_sprintf( Check24bit, "%s_24bit", NewFileName );
            x_makepath( Check24bitFull,  NewDrive, NewDir, Check24bit, NewExtension );
            X_FILE* CheckFile = NULL;
            CheckFile = x_fopen( Check24bitFull, "rb" );
            if( CheckFile != NULL )
            {
                x_strcpy( s_pMaterialSet->m_pTextures[i].m_Filename, Check24bitFull );
                x_fclose( CheckFile );
            }

            // If there is a texture output file open, printf the current texture name to the output file.
            xbool AlphaFlag = ((!x_strncmp(&(NewFileName[3]),"CORONA",6)) || (!x_strncmp(&(NewFileName[4]),"CORONA",6)));
            x_fprintf( g_TextureNameFile, "%s,GBL:0,CMP:%d,ALPHA:1,ALFNAME:%d,ALFGENC:%d,MIP:%d,CLRSCL:%d:255:255:255:%d\n",
                s_pMaterialSet->m_pTextures[i].m_Filename,
                !AlphaFlag && AlphaPercent == 255,
                !AlphaFlag,
                AlphaFlag,
                g_UseMips,
                AlphaPercent != 255,
                AlphaPercent );
		}
	}

	if( FALSE == g_ExportTextures )
		return;

	//--- Export the textures
	for( i = 0; i < s_pMaterialSet->m_nTextures; i++ )
	{
		x_bitmap	BMP;
		xbool		ret;

		if( g_Verbose )
			x_printf( "Loading %s\n", s_pMaterialSet->m_pTextures[i].m_Filename );

		if( !x_stricmp( s_pMaterialSet->m_pTextures[i].m_Filename, "none" ) )
		{
			AUXBMP_SetupDefaultBitmap( BMP );
			ret = TRUE;
		}
		else
		{	ret = AUXBMP_LoadBitmap( BMP, s_pMaterialSet->m_pTextures[i].m_Filename );
		}

		if( !ret )
		{
			x_printf( "ERROR: Unable to load %s\n", s_pMaterialSet->m_pTextures[i].m_Filename );
			PauseOnError();
		}
		else
		{
			// Added for alpha bitmaps ------------------------------------------------
			char Drive[X_MAX_DRIVE];
			char Dir[X_MAX_DIR];
			char Name[X_MAX_FNAME];
			char Ext[X_MAX_EXT];
			char AlphaName[X_MAX_FNAME];
			char PunchPath[X_MAX_PATH];
			char AlphaPath1[X_MAX_PATH];
			char AlphaPath2[X_MAX_PATH];
			char AlphaPath3[X_MAX_PATH];

			x_bitmap AlphaBMP;

			x_splitpath( s_pMaterialSet->m_pTextures[i].m_Filename, Drive, Dir, Name, Ext );

			//--- Create possible alpha texture filenames
			x_sprintf( AlphaName, "%s_punch", Name );	x_makepath( PunchPath,  Drive, Dir, AlphaName, Ext );
			x_sprintf( AlphaName, "%s_alpha", Name );	x_makepath( AlphaPath1, Drive, Dir, AlphaName, Ext );
			x_sprintf( AlphaName, "%s_mask",  Name );	x_makepath( AlphaPath2, Drive, Dir, AlphaName, Ext );
			x_sprintf( AlphaName, "%sAlpha",  Name );	x_makepath( AlphaPath3, Drive, Dir, AlphaName, Ext );

			//--- Clear Alpha present
			BMP.SetAllAlpha( 255 );

			//--- Load alpha map if possible
			if( AUXBMP_LoadBitmap( AlphaBMP, PunchPath ) )
			{
				if( !AUXBMP_ApplyAlpha( BMP, AlphaBMP, TRUE, TRUE ) )
					x_printf( "--Could not apply punch using \"%s\" \n", PunchPath );
			}
			else if( AUXBMP_LoadBitmap( AlphaBMP, AlphaPath1 ) )
			{
				if( !AUXBMP_ApplyAlpha( BMP, AlphaBMP, FALSE, FALSE ) )
					x_printf( "--Could not apply alpha using \"%s\" \n", AlphaPath1 );
			}
			else if( AUXBMP_LoadBitmap( AlphaBMP, AlphaPath2 ) )
			{
				if( !AUXBMP_ApplyAlpha( BMP, AlphaBMP, FALSE, FALSE ) )
					x_printf( "--Could not apply alpha using \"%s\" \n", AlphaPath2 );
			}
			else if( AUXBMP_LoadBitmap( AlphaBMP, AlphaPath3 ) )
			{
				if( !AUXBMP_ApplyAlpha( BMP, AlphaBMP, FALSE, FALSE ) )
					x_printf( "--Could not apply alpha using \"%s\" \n", AlphaPath3 );
			}

			AlphaBMP.KillBitmap();
			//---------------------------------------------------------------------------

			ret = AUXBMP_ConvertToGameCube( BMP, g_UseMips, TRUE, TRUE );

			if( !ret )
			{
				x_printf( "ERROR: Unable to convert %s\n", s_pMaterialSet->m_pTextures[i].m_Filename );
				PauseOnError();
			}
			else
			{
				if( g_Verbose )
					x_printf( "Saving %s\n", s_pMaterialSet->m_pTextures[i].m_Filename );

				ret = BMP.Save( pOutfile );
				if( !ret )
				{
					x_printf( "ERROR: Unable to save %s\n", s_pMaterialSet->m_pTextures[i].m_Filename );
					PauseOnError();
				}
			}
		}
	}
}

//==========================================================================

static void EndianConvertData( t_GeomFileHeader* pHdr, s32 NMeshes, s32 NSubMeshes )
{
	s32 i;

	//--- NOTE that the vertex/display list data has already been converted

	//--- Swap endian-ness on the header
	pHdr->Flags            = GC_ENDIAN_32( pHdr->Flags );
	pHdr->NTextures        = GC_ENDIAN_32( pHdr->NTextures );
	pHdr->NTexturesInFile  = GC_ENDIAN_32( pHdr->NTexturesInFile );
	pHdr->NMeshes          = GC_ENDIAN_32( pHdr->NMeshes );
	pHdr->NSubMeshes       = GC_ENDIAN_32( pHdr->NSubMeshes );
	pHdr->VertexDataSize   = GC_ENDIAN_32( pHdr->VertexDataSize );
	pHdr->DispListDataSize = GC_ENDIAN_32( pHdr->DispListDataSize );

	//--- Swap endian-ness on the mesh data
	for( i = 0; i < NMeshes; i++ )
	{
		s_pMeshData[i].Flags        = GC_ENDIAN_32( s_pMeshData[i].Flags );
		s_pMeshData[i].FirstSubMesh = GC_ENDIAN_32( s_pMeshData[i].FirstSubMesh );
		s_pMeshData[i].NSubMeshes   = GC_ENDIAN_32( s_pMeshData[i].NSubMeshes );
		s_pMeshData[i].NVerts       = GC_ENDIAN_32( s_pMeshData[i].NVerts );
		s_pMeshData[i].Radius       = GC_ENDIAN_32( s_pMeshData[i].Radius );
		s_pMeshData[i].VertFormat   = GC_ENDIAN_32( s_pMeshData[i].VertFormat );
		s_pMeshData[i].pVertData    = GC_ENDIAN_32( s_pMeshData[i].pVertData );
	}

	//--- Swap endian-ness on the submesh data
	for( i = 0; i < NSubMeshes; i++ )
	{
		s_pSubMeshData[i].Flags        = GC_ENDIAN_32( s_pSubMeshData[i].Flags );
		s_pSubMeshData[i].TextureID    = GC_ENDIAN_32( s_pSubMeshData[i].TextureID );
		s_pSubMeshData[i].pDispList    = GC_ENDIAN_32( s_pSubMeshData[i].pDispList );
		s_pSubMeshData[i].DispListSize = GC_ENDIAN_32( s_pSubMeshData[i].DispListSize );
		s_pSubMeshData[i].NVerts       = GC_ENDIAN_32( s_pSubMeshData[i].NVerts );
		s_pSubMeshData[i].NTris        = GC_ENDIAN_32( s_pSubMeshData[i].NTris );
	}
}

//==========================================================================

void ExportGCGeometry( mesh::object& rGeom, X_FILE* pOutfile )
{
	t_GeomFileHeader hdr;
	s32 i;

	ASSERT( pOutfile != NULL );

	//--- fill in the header info
	x_memset( &hdr, 0, sizeof(t_GeomFileHeader) );
	FillName( hdr.Name, rGeom.m_Name, MAX_MESH_NAME );

	//--- Null and zero out static data
	s_pVertexData         = NULL;
	s_pDispListData       = NULL;
	s_CurVertexDataSize   = 0;
	s_CurDispListDataSize = 0;
	s_pMeshData           = NULL;
	s_pSubMeshData        = NULL;
	s_CurSubMesh          = 0;
	s_pTriData            = NULL;
	s_NTris               = 0;


	//--- allocate some space for the data we will be building
	s_pVertexData   = (byte*)          x_malloc( MAX_TOTAL_VERT_DATA_SIZE );
	s_pDispListData	= (byte*)          x_malloc( MAX_TOTAL_DLIST_DATA_SIZE );
	s_pMeshData     = (t_GeomMesh*)    x_malloc( MAX_NUM_MESHES * sizeof(t_GeomMesh) );
	s_pSubMeshData  = (t_GeomSubMesh*) x_malloc( MAX_NUM_SUBMESHES * sizeof(t_GeomSubMesh) );

	ASSERT( s_pVertexData   != NULL );
	ASSERT( s_pDispListData != NULL );
	ASSERT( s_pMeshData     != NULL );
	ASSERT( s_pSubMeshData  != NULL );

	x_memset( s_pVertexData,   0, MAX_TOTAL_VERT_DATA_SIZE );
	x_memset( s_pDispListData, 0, MAX_TOTAL_DLIST_DATA_SIZE );
	x_memset( s_pMeshData,     0, MAX_NUM_MESHES * sizeof(t_GeomMesh) );
	x_memset( s_pSubMeshData,  0, MAX_NUM_SUBMESHES * sizeof(t_GeomSubMesh) );

	if( g_Verbose )
		x_printf( "Geometry Name: %s\n", hdr.Name );


	//--- loop through each of the meshes adding them to the data
	for( i = 0; i < rGeom.m_nChunks; i++ )
	{
		ASSERT( hdr.NMeshes < MAX_NUM_MESHES );
		ASSERT( hdr.NSubMeshes < MAX_NUM_SUBMESHES );

		//--- fill in the basic mesh data
		FillName( s_pMeshData[i].Name, rGeom.m_pChunks[i].m_Name, MAX_MESH_NAME );

		s_pMeshData[i].FirstSubMesh = s_CurSubMesh;
		s_pMeshData[i].NSubMeshes   = 0;
		s_pMeshData[i].NVerts       = 0;
		s_pMeshData[i].Radius       = CalculateMeshRadius( rGeom, i );

		//--- set up the mesh flags
		s_pMeshData[i].Flags = MESH_FLAG_VISIBLE;

		if( g_UseMips )
		{
			s_pMeshData[i].Flags |= MESH_FLAG_USE_MIPS;
		}

		if( IsEnvMapped( s_pMeshData[i].Name ) )
		{
			s_pMeshData[i].Flags |= (MESH_FLAG_ENVMAPPED | MESH_FLAG_DYNAMIC_LIGHTING);
		}
		else if( IsShadow( s_pMeshData[i].Name ) )
		{
			s_pMeshData[i].Flags |= MESH_FLAG_SHADOW;
			s_pMeshData[i].Flags &= ~MESH_FLAG_USE_MIPS;
		}
		else if( g_DynamicLighting )
		{
			s_pMeshData[i].Flags |= MESH_FLAG_DYNAMIC_LIGHTING;
		}


		//--- print out some stats
		if( g_Verbose )
			x_printf( "  Mesh Name: %s\n", s_pMeshData[i].Name );

		//--- build the submeshes for this mesh
		RipSubMeshData( i, &s_pMeshData[i], rGeom );

		//--- print out some more stats
		if( g_Verbose )
			x_printf( "  NSubMeshes: %d\n", s_pMeshData[i].NSubMeshes );

		//--- increase the mesh count
		hdr.NMeshes++;
		hdr.NSubMeshes += s_pMeshData[i].NSubMeshes;
	}

	//--- finish filling in the header
	hdr.VertexDataSize   = s_CurVertexDataSize;
	hdr.DispListDataSize = s_CurDispListDataSize;
	hdr.NTextures        = s_pMaterialSet->m_nTextures;

	if( g_ExportTextures )
		hdr.NTexturesInFile = s_pMaterialSet->m_nTextures;


	//--- print out some more stats
	if( g_Verbose )
		x_printf( "Total Memory Usage: VertData %d   DList %d\n", s_CurVertexDataSize, s_CurDispListDataSize );

	//--- convert the endian-ness of the data, but save count of meshes and submeshes for writing to file later
	s32 NMeshes    = hdr.NMeshes;
	s32 NSubMeshes = hdr.NSubMeshes;

	EndianConvertData( &hdr, NMeshes, NSubMeshes );

	//=== now we can write out the data file

	//--- write out the header
	x_fwrite( &hdr, sizeof(t_GeomFileHeader), 1, pOutfile );

	//--- write out the texture names
	char TextureName[TEXTURE_NAME_LENGTH];

	for( i = 0; i < s_pMaterialSet->m_nTextures; i++ )
	{
		GetTextureName( TextureName, rGeom, i );
		PadStringWithZero( TextureName, TEXTURE_NAME_LENGTH );

		x_fwrite( TextureName, sizeof(char), TEXTURE_NAME_LENGTH, pOutfile );
	}

	//--- write out the mesh, submesh, vertex, and display list data
	x_fwrite( s_pMeshData,     sizeof(t_GeomMesh),    NMeshes,               pOutfile );
	x_fwrite( s_pSubMeshData,  sizeof(t_GeomSubMesh), NSubMeshes,            pOutfile );
	x_fwrite( s_pVertexData,   sizeof(byte),          s_CurVertexDataSize,   pOutfile );
	x_fwrite( s_pDispListData, sizeof(byte),          s_CurDispListDataSize, pOutfile );

	//--- write out the textures
	ExportTextures( rGeom, pOutfile );

	//--- free up the data
	x_free( s_pVertexData );
	x_free( s_pDispListData );
	x_free( s_pMeshData );
	x_free( s_pSubMeshData );
}

//==========================================================================
