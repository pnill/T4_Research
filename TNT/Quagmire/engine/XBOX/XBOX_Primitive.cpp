//==============================================================================
//
//  Primitive Implemenation
//
//==============================================================================

#include "Q_XBOX.hpp"
#include "x_plus.hpp"
#include "Q_Primitive.hpp"

//==============================================================================
//  Private Primitive Structs
//==============================================================================

struct PRIM_Packet
{
	u32						PrimType;
	u32						PrimCount;
	u32						PrimFormat;
	u32						PrimFormatSize;
	u32						PrimIndexCount;
	LPDIRECT3DVERTEXBUFFER8	pVertexBuffer;
	LPDIRECT3DINDEXBUFFER8	pIndexBuffer;
    matrix4                 L2W;
};

//////////////////////////////////////////
//--Lit Transformed 2D Vertex
//////////////////////////////////////////
#define VERTFORMAT_2DLIT (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)
struct PRIM_2DLitVert  
{
	vector2 Pos;		// Position
	f32		Z;			// Z Depth
	f32		RHW;		// The transformed position for the vertex
    color	Color;      // The vertex color
};

/////////////////////////////////////////
//--Lit Transformed 2D Vertex w/ Texture
/////////////////////////////////////////
#define VERTFORMAT_2DUVLIT (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)
struct PRIM_2DLitUVVert  
{
	vector2 Pos;		// Position
	f32		Z;			// Z Depth
	f32		RHW;		// The transformed position for the vertex
    color	Color;      // The vertex color
	vector2 UV;
};

/////////////////////////////////////////
//--Lit Untransformed 3D Vertex
/////////////////////////////////////////
#define VERTFORMAT_3DLIT (D3DFVF_XYZ|D3DFVF_DIFFUSE)
struct PRIM_3DLitVert
{
	vector3 Pos;
	color   Color;
};

/////////////////////////////////////////
//--Lit Untransformed 3D Vertex w/ Texture
/////////////////////////////////////////
#define VERTFORMAT_3DUVLIT (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)
struct PRIM_3DLitUVVert
{
	vector3 Pos;
	color   Color;
	vector2 UV;
};

/////////////////////////////////////////
//--Unlit Untransformed 3D Vertex
/////////////////////////////////////////
#define VERTFORMAT_3D (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_NORMAL)
struct PRIM_3DVert
{
	vector3 Pos;
	vector3 Normal;
	color   Color;
};

/////////////////////////////////////////
//--Unlit Untransformed 3D Vertex w/ Texture
/////////////////////////////////////////
#define VERTFORMAT_3DUV (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_NORMAL|D3DFVF_TEX1)
struct PRIM_3DUVVert
{
	vector3 Pos;
	vector3 Normal;
	color   Color;
	vector2 UV;
};

//==============================================================================
//  Engine Globals
//==============================================================================
 
extern bool						g_Initialized;
extern bool						g_SceneActive;
extern bool						g_WndActive;
extern HRESULT					g_LastError;

extern LPDIRECT3D8				g_pD3D;
extern LPDIRECT3DDEVICE8		g_pD3DDevice;


//==============================================================================
//  SPrimParam Constructors
//==============================================================================
SPrimParam::SPrimParam( void )
{
	memset(this,0,sizeof(SPrimParam));
	PrimType = 9;
}



//==============================================================================
//  CreatePrimitive()
//		Setups a Primitive based on the Primitive parmaters passed to the
//		function.  Returns a handle to the Primitive.
//==============================================================================
PRIMHANDLE	PRIM_CreatePrimitive( const SPrimParam& PrimParam )
{
	u32						PrimCount;
	u32						VertFormat;
	s32						VertFormatSize;
	u32						VertUsage;

	PRIM_Packet*			pPacket			= NULL;
	LPDIRECT3DVERTEXBUFFER8 pVertexBuffer	= NULL;
	LPDIRECT3DINDEXBUFFER8	pIndexBuffer	= NULL;
	byte*					pData			= NULL;

	///////////////////////////////////////////////////////////////////////////
	//  Find Number of Primitives to create
	///////////////////////////////////////////////////////////////////////////

	ASSERT( g_pD3DDevice );
	ASSERT( PrimParam.pVert );
//	ASSERT( (PrimParam.NVerts > 0) && (PrimParam.NVerts < D3D8MAXNUMVERTICES) );
//	ASSERT( (PrimParam.NIndex < D3D8MAXNUMVERTICES) );
	
	PrimCount = (PrimParam.NIndex > 0) ? PrimParam.NIndex : PrimParam.NVerts;

	switch ((PRIM_PrimType)(PrimParam.PrimType & 0x0000000F))
	{
		case PRIMTYPE_POINTLIST:		
			break;
		case PRIMTYPE_LINELIST: 		
			PrimCount = PrimCount >> 1;	
			break;
		case PRIMTYPE_LINESTRIP: 		
			PrimCount = PrimCount - 1;	
			break;
		case PRIMTYPE_TRIANGLELIST: 	
			PrimCount = (s32)(PrimCount / 3);	
			break;
		case PRIMTYPE_TRIANGLESTRIP: 	
		case PRIMTYPE_TRIANGLEFAN: 
			PrimCount = PrimCount - 2;
			break;		
		default: ASSERTS(0,"Invalid Primitive");
	}

//	ASSERT( PrimCount < D3DMAXNUMPRIMITIVES );

	///////////////////////////////////////////////////////////////////////////
	//  Build Format Info and Usage
	///////////////////////////////////////////////////////////////////////////

	if ( ( PrimParam.PrimType & PRIMTYPE_2D ) && ( PrimParam.pUV ))
	{

		ASSERT(PrimParam.pUV);
		VertFormat		= VERTFORMAT_2DUVLIT;
		VertFormatSize	= sizeof(PRIM_2DLitUVVert);	
	}
	else if ( (PrimParam.PrimType & PRIMTYPE_2D ) )
	{
		VertFormat		= VERTFORMAT_2DLIT;
		VertFormatSize	= sizeof(PRIM_2DLitVert);
	}
	else if ( ( PrimParam.PrimType & PRIMTYPE_3D ) && ( PrimParam.pUV ) )
	{
		if (PrimParam.pNormal)
		{
			VertFormat		= VERTFORMAT_3DUV;
			VertFormatSize	= sizeof(PRIM_3DUVVert);
		}
		else
		{
			VertFormat		= VERTFORMAT_3DUVLIT;
			VertFormatSize	= sizeof(PRIM_3DLitUVVert);
		}
	}
	else if ( PrimParam.PrimType & (PRIMTYPE_3D) )
	{
		if (PrimParam.pNormal)
		{
			VertFormat		= VERTFORMAT_3D;
			VertFormatSize	= sizeof(PRIM_3DVert);
		}
		else
		{
			VertFormat		= VERTFORMAT_3DLIT;
			VertFormatSize	= sizeof(PRIM_3DLitVert);
		}

	}
	else
	{
		ASSERTS(0, "Unsupported Primitive Type");
	}

	VertUsage = 0;
//	if ( PrimParam.PrimType & PRIMTYPE_NOCLIP ) VertUsage |= D3DUSAGE_DONOTCLIP; 
	if ( PrimParam.PrimType & PRIMTYPE_STATIC ) VertUsage |= D3DUSAGE_WRITEONLY;  
	if ( (PrimParam.PrimType & 0x0000000F) == PRIMTYPE_POINTLIST ) VertUsage |= D3DUSAGE_POINTS;

	///////////////////////////////////////////////////////////////////////////
	//  Create Vertex Buffer
	///////////////////////////////////////////////////////////////////////////


	g_LastError = g_pD3DDevice->CreateVertexBuffer(
		VertFormatSize * PrimParam.NVerts,
		VertUsage,
		VertFormat,
		D3DPOOL_DEFAULT,
		&pVertexBuffer );

	if (FAILED(g_LastError)) return NULL;

	///////////////////////////////////////////////////////////////////////////
	//  Fill in Vertex Buffer
	///////////////////////////////////////////////////////////////////////////

	g_LastError = pVertexBuffer->Lock(0, VertFormatSize * PrimParam.NVerts, &pData, 0 );
	if (FAILED(g_LastError)) 
	{
		pVertexBuffer->Release();
		return NULL;
	}

	if ( ( PrimParam.PrimType & PRIMTYPE_2D ) && ( PrimParam.pUV ) )
	{
		PRIM_2DLitUVVert* pVert = (PRIM_2DLitUVVert*)pData;

		for (s32 v = 0; v < PrimParam.NVerts; v++ )
		{
			pVert[v].Pos.X	= PrimParam.pVert[v].X;
			pVert[v].Pos.Y	= PrimParam.pVert[v].Y;
			pVert[v].Z		= 0.5f;
			pVert[v].RHW	= 1.0f;
			pVert[v].UV     = PrimParam.pUV[v];
			if (PrimParam.pColor)
				pVert[v].Color	= PrimParam.pColor[v];
			else
				pVert[v].Color = color(0xffffffff);
		}
	}
	else if ( (PrimParam.PrimType & PRIMTYPE_2D) )
	{
		PRIM_2DLitVert* pVert = (PRIM_2DLitVert*)pData;

		for (s32 v = 0; v < PrimParam.NVerts; v++ )
		{
			pVert[v].Pos.X	= PrimParam.pVert[v].X;
			pVert[v].Pos.Y	= PrimParam.pVert[v].Y;
			pVert[v].Z		= 0.5f;
			pVert[v].RHW	= 1.0f;
			if (PrimParam.pColor)
				pVert[v].Color	= PrimParam.pColor[v];
			else
				pVert[v].Color = color(0xffffffff);
		}
	}
	else if ( ( PrimParam.PrimType & PRIMTYPE_3D ) && ( PrimParam.pUV ) )
	{
		if (PrimParam.pNormal)
		{
			PRIM_3DUVVert* pVert = (PRIM_3DUVVert*)pData;
			for (s32 v = 0; v < PrimParam.NVerts; v++ )
			{
				pVert[v].Pos    = PrimParam.pVert[v];
				pVert[v].Normal = PrimParam.pNormal[v];
				pVert[v].UV     = PrimParam.pUV[v];
				if (PrimParam.pColor)
					pVert[v].Color	= PrimParam.pColor[v];
				else
					pVert[v].Color = color(0xffffffff);
			}
		}
		else
		{
			PRIM_3DLitUVVert* pVert = (PRIM_3DLitUVVert*)pData;
			for (s32 v = 0; v < PrimParam.NVerts; v++ )
			{
				pVert[v].Pos	= PrimParam.pVert[v];
				pVert[v].UV     = PrimParam.pUV[v];
				if (PrimParam.pColor)
					pVert[v].Color	= PrimParam.pColor[v];
				else
					pVert[v].Color = color(0xffffffff);
			}
		}	
	}
	else if ( (PrimParam.PrimType & PRIMTYPE_3D) )
	{
		if (PrimParam.pNormal)
		{
			PRIM_3DVert* pVert = (PRIM_3DVert*)pData;
			for (s32 v = 0; v < PrimParam.NVerts; v++ )
			{
				pVert[v].Pos    = PrimParam.pVert[v];
				pVert[v].Normal = PrimParam.pNormal[v];
				if (PrimParam.pColor)
					pVert[v].Color	= PrimParam.pColor[v];
				else
					pVert[v].Color = color(0xffffffff);
			}
		}
		else
		{
			PRIM_3DLitVert* pVert = (PRIM_3DLitVert*)pData;
			for (s32 v = 0; v < PrimParam.NVerts; v++ )
			{
				pVert[v].Pos	= PrimParam.pVert[v];
				if (PrimParam.pColor)
					pVert[v].Color	= PrimParam.pColor[v];
				else
					pVert[v].Color = color(0xffffffff);
			}
		}
	}
	else
	{
		ASSERTS(0, "Unsupported Primitive Type");
	}

	pVertexBuffer->Unlock();


	///////////////////////////////////////////////////////////////////////////
	//  Create Index Buffer
	///////////////////////////////////////////////////////////////////////////

	if ( PrimParam.pIndex )
	{
		g_LastError = g_pD3DDevice->CreateIndexBuffer(
			sizeof(s16) * PrimParam.NIndex,
			VertUsage,
			D3DFMT_INDEX16,
			D3DPOOL_DEFAULT,
			&pIndexBuffer );

		pIndexBuffer->Lock(0, sizeof(s16) * PrimParam.NIndex, &pData, 0 );
		if (FAILED(g_LastError)) 
		{
			pVertexBuffer->Release();
			return NULL;
		}

		x_memcpy(pData, PrimParam.pIndex, sizeof(s16)*PrimParam.NVerts);
		pIndexBuffer->Unlock();

		if (FAILED(g_LastError)) 
		{
			pVertexBuffer->Release();
			return NULL;
		}
	}

	
	///////////////////////////////////////////////////////////////////////////
	//  Fill in Packet
	///////////////////////////////////////////////////////////////////////////

	pPacket = new PRIM_Packet;
	ASSERT(pPacket);
	pPacket->PrimType		= PrimParam.PrimType;
	pPacket->PrimCount		= PrimCount;
	pPacket->PrimIndexCount = PrimParam.NIndex;
	pPacket->pVertexBuffer	= pVertexBuffer;
	pPacket->pIndexBuffer   = pIndexBuffer;
	pPacket->PrimFormat		= VertFormat;
	pPacket->PrimFormatSize	= VertFormatSize;
    pPacket->L2W.Identity();

	return (void*)pPacket;
}


//==============================================================================
//  DestroyPrimitive()
//		Destroys the Primitive and releases resources allocated by 
//		CreatePrimitive.
//==============================================================================
void PRIM_DestroyPrimitive( PRIMHANDLE& PrimHandle )
{

	PRIM_Packet* pPrimPacket = (PRIM_Packet*)PrimHandle;

	if (PrimHandle)
	{
		if (pPrimPacket->pVertexBuffer)
		{
			pPrimPacket->pVertexBuffer->Release();
			pPrimPacket->pVertexBuffer = NULL;
		}
		if (pPrimPacket->pIndexBuffer)
		{
			pPrimPacket->pIndexBuffer->Release();
			pPrimPacket->pIndexBuffer = NULL;
		}
		delete PrimHandle;
		PrimHandle = NULL;
	}
}


//==============================================================================
//  RenderPrimitive()
//		Renders the Primitive
//==============================================================================
s32	PRIM_RenderPrimitive( const PRIMHANDLE PrimHandle )
{
	ASSERT(PrimHandle);

	PRIM_Packet* pPrimPacket = (PRIM_Packet*)PrimHandle;

    ////////////////////////////////////////////////////////////////////////
    //  Set Local to World Matrix
    ////////////////////////////////////////////////////////////////////////

    g_pD3DDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&pPrimPacket->L2W );

    ////////////////////////////////////////////////////////////////////////
    //  Set stream source and shader for verts 
    ////////////////////////////////////////////////////////////////////////

    g_pD3DDevice->SetStreamSource( 
		0, 
		pPrimPacket->pVertexBuffer, 
		pPrimPacket->PrimFormatSize );

    XBOX_ActivateVertexShader( pPrimPacket->PrimFormat );

    ////////////////////////////////////////////////////////////////////////
    //  Render.  If indexed also set indices.
    ////////////////////////////////////////////////////////////////////////

	if (pPrimPacket->pIndexBuffer)
	{
		g_pD3DDevice->SetIndices( pPrimPacket->pIndexBuffer, 0 );
		g_LastError = g_pD3DDevice->DrawIndexedPrimitive( 
			(_D3DPRIMITIVETYPE)(pPrimPacket->PrimType & 0x0000000F), 
			0, 
			pPrimPacket->PrimIndexCount,
			0,
			pPrimPacket->PrimCount );
	}
	else
	{
		g_LastError = g_pD3DDevice->DrawPrimitive( 
			(_D3DPRIMITIVETYPE)(pPrimPacket->PrimType & 0x0000000F), 
			0, 
			pPrimPacket->PrimCount );
	}

	return g_LastError;
}

//==============================================================================
//  SetL2W()
//		Sets the local to world matrix
//==============================================================================

void PRIM_SetL2W( const PRIMHANDLE PrimHandle, const matrix4& L2W )
{
	ASSERT(PrimHandle);

	PRIM_Packet* pPrimPacket = (PRIM_Packet*)PrimHandle;

    pPrimPacket->L2W = L2W;
}