// RenderContext.cpp: implementation of the CRenderContextD3D class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CeD.h"
#include "RenderContextD3D.h"
#include "D3DView.h"
#include "CeDView.h"
#include "d3d8.h"
#include "Skeleton.h"
#include "Skin.h"
#include "Character.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

//==========================================================================
matrix4& ConvertMatrixToD3D( matrix4& rDest )
{
	rDest.RotateY( R_180 );

	return rDest;
}


//==========================================================================
static matrix4& ConvertD3DToMatrix( matrix4& rDest )
{
	rDest.RotateY( R_180 );

	return rDest;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRenderContextD3D::CRenderContextD3D( CD3DView *pD3DView, CCamera *pCamera )
	: CRenderContext( pCamera )
{
//	m_pDC = pDC ;
	m_pD3DView		= pD3DView;
	m_pD3D			= pD3DView->m_pD3D;
	m_pd3dDevice	= m_pD3DView->m_pd3dDevice;

	m_NumSkins		= 0;

	AquireDevices();

	Reset();

	matrix4 m;
	m.Identity();
	PushMatrix( &m );
}

CRenderContextD3D::~CRenderContextD3D()
{
	ReleaseDevices();
	ClearSkins();
}

//////////////////////////////////////////////////////////////////////
// Geometry Functions

////////////////////////////////////////////////////////////////////////////
// Selecting Drawing Utencils

////////////////////////////////////////////////////////////////////////////
void CRenderContextD3D::Reset( void )
{
	m_TransVBOffset = 0;
	m_VB2DOffset	= 0;

	//---	set the draw states to indevidual for default
	m_DrawLinesState	= INDIVIDUAL;
	m_DrawPixelsState	= INDIVIDUAL;
}

////////////////////////////////////////////////////////////////////////////
void CRenderContextD3D::AquireDevices( void )
{
	//---	attempt to create the line vertex buffer
	#ifdef RENDER_POINTS_IN_SCREENSPACE
		HRESULT hres = m_pd3dDevice->CreateVertexBuffer( TRANSVB_SIZE, D3DUSAGE_DONOTCLIP | D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
														TRANSVB_FVF, D3DPOOL_DEFAULT, &m_pTransVB );
	#else
		HRESULT hres = m_pd3dDevice->CreateVertexBuffer( TRANSVB_SIZE, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
														TRANSVB_FVF, D3DPOOL_DEFAULT, &m_pTransVB );
	#endif

	if( hres != D3D_OK )
		m_pD3DView->DisplayErrorMsg( hres, CD3DView::MSGERR_APPMUSTEXIT );
	ASSERT(hres == D3D_OK);


	hres = m_pd3dDevice->CreateVertexBuffer( VB2D_SIZE, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
													VB2D_FVF, D3DPOOL_DEFAULT, &m_pVB2D );
	if( hres != D3D_OK )
		m_pD3DView->DisplayErrorMsg( hres, CD3DView::MSGERR_APPMUSTEXIT );
	ASSERT(hres == D3D_OK);
	
	Reset();

	//---	set the device to use this vertex stream
    m_pd3dDevice->SetStreamSource( 0, m_pTransVB, sizeof(STransVert) );
    m_pd3dDevice->SetVertexShader( TRANSVB_FVF );

	//---	the world transform is set to the identity matrix by default.
	matrix4 mat;
	mat.Identity();
	m_pd3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&mat );


}

////////////////////////////////////////////////////////////////////////////
void CRenderContextD3D::ReleaseDevices( void )
{
	if( m_pTransVB )
		m_pTransVB->Release();
	m_pTransVB = NULL;

	if( m_pVB2D )
		m_pVB2D->Release();
	m_pVB2D = NULL;

	ClearSkins();
}

////////////////////////////////////////////////////////////////////////////
CPen* CRenderContextD3D::SelectObject( CPen* pPen )
{
	LOGPEN PenLog;

	if( pPen == NULL )
		m_PenColor = 0;
	else
	{
		pPen->GetLogPen( &PenLog );
		m_PenColor = D3DCOLOR_XRGB(GetRValue(PenLog.lopnColor),GetGValue(PenLog.lopnColor),GetBValue(PenLog.lopnColor));
	}
	return CRenderContext::SelectObject( pPen );
}

//==========================================================================
CRenderContextD3D::SVert2D* CRenderContextD3D::Reserve2DVerts( s32 Count )
{
	s32 BlockSize = Count*sizeof( SVert2D );
	s32 LockFlags;

	//---	if the buffer isn't big enough, exit now
	if( BlockSize > TRANSVB_SIZE )
		return NULL;

	//---	determine if the buffer has to be reset
	if( (m_VB2DOffset+BlockSize) > TRANSVB_SIZE )
	{
		m_VB2DOffset = 0;
		LockFlags = D3DLOCK_DISCARD;
	}
	else
		LockFlags = D3DLOCK_NOOVERWRITE;

	//---	attempt to lock the buffer
	u8* pBuffer;
	HRESULT hres = m_pVB2D->Lock( m_VB2DOffset, BlockSize, (u8**)&pBuffer, LockFlags );
	if( hres != S_OK )
		return NULL;

	return (SVert2D*)pBuffer;
}

//==========================================================================
CRenderContextD3D::STransVert*	CRenderContextD3D::Reserve3DVerts( s32 Count )
{
	s32 BlockSize = Count*sizeof( STransVert );
	s32 LockFlags;

	//---	if the buffer isn't big enough, exit now
	if( BlockSize > TRANSVB_SIZE )
		return NULL;

	//---	determine if the buffer has to be reset
	if( (m_TransVBOffset+BlockSize) > TRANSVB_SIZE )
	{
		m_TransVBOffset = 0;
		LockFlags = D3DLOCK_DISCARD;
	}
	else
		LockFlags = D3DLOCK_NOOVERWRITE;

	//---	attempt to lock the buffer
	u8* pBuffer;
	HRESULT hres = m_pTransVB->Lock( m_TransVBOffset, BlockSize, &pBuffer, LockFlags );
	if( hres != S_OK )
		return NULL;

	return (STransVert*)pBuffer;
}

//////////////////////////////////////////////////////////////////////
// Drawing Functions

//==========================================================================
void CRenderContextD3D::StartDrawTriangles( s32 nTriangles )
{
	if( nTriangles == 0 )
		m_DrawTrianglesState = UNKNOWNCOUNT;
	else
	{
		m_nLockedTransVerts = 3*nTriangles;
		m_pLockedTransVerts = Reserve3DVerts( m_nLockedTransVerts );
		if( !m_pLockedTransVerts )
			return;

		m_DrawTrianglesState = KNOWNCOUNT;
	}

	//---	clear the drawn Triangles count
	m_DrawTrianglesCount	= 0;

	//---	store the offset at the start
	m_DrawTrianglesOffset	= m_TransVBOffset;
}

//==========================================================================
void CRenderContextD3D::EndDrawTriangles( void )
{
	//---	nothing is done here for individual Triangles, they were drawn one at a time
	if( m_DrawTrianglesState == INDIVIDUAL )
		return;

	//---	unlock the buffer if the count was known
	if( m_DrawTrianglesState == KNOWNCOUNT )
	{
		//---	make sure the locked buffer wasn't overrun (if it was locked)
		ASSERT( 3*m_DrawTrianglesCount <= m_nLockedTransVerts );

		m_pTransVB->Unlock();
	}

	//---	draw the Triangles
    m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, (u32)(m_DrawTrianglesOffset*(1.0f/sizeof(STransVert))), m_DrawTrianglesCount );

	//---	go back to indevidual mode by default
	m_DrawTrianglesState = INDIVIDUAL;
}


//==========================================================================
//
//	vector3	rvPos1, rvPos2, rvPos3			- 3 points of triangle
//	vector3 rNormCol1, rNormCol2, rNormCol3	- 3 colors/normals of the points (depending on what is compiled for), if they are normals, then the color
//											  of the triangle is the same as the pen color
//
void CRenderContextD3D::DrawTriangle( vector3& rvPos1, vector3& rNormCol1, vector3& rvPos2, vector3& rNormCol2, vector3& rvPos3, vector3& rNormCol3 )
{
	STransVert* pTriangleVerts;

	s32 NewVBOffset = m_TransVBOffset+3*sizeof(STransVert);
	if( NewVBOffset > TRANSVB_SIZE ) 
		return;

	if( m_DrawTrianglesState != KNOWNCOUNT )
	{
		pTriangleVerts = Reserve3DVerts( 3 );
		if( !pTriangleVerts )	return;
	}
	else
	{
		pTriangleVerts = &m_pLockedTransVerts[3*m_DrawTrianglesCount];
	}

#ifndef RENDER_POINTS_IN_SCREENSPACE

	//---	coordinates given are in world coordinates

	//---	set vertex 0
	pTriangleVerts[0].x		= rvPos1.X;
	pTriangleVerts[0].y		= rvPos1.Y;
	pTriangleVerts[0].z		= rvPos1.Z;
//	pTriangleVerts[0].nx	= rNormCol1.X;
//	pTriangleVerts[0].ny	= rNormCol1.Y;
//	pTriangleVerts[0].nz	= rNormCol1.Z;
	pTriangleVerts[0].color	= m_PenColor;

	//---	set vertex 1
	pTriangleVerts[1].x		= rvPos2.X;
	pTriangleVerts[1].y		= rvPos2.Y;
	pTriangleVerts[1].z		= rvPos2.Z;
//	pTriangleVerts[1].nx	= rNormCol1.X;
//	pTriangleVerts[1].ny	= rNormCol1.Y;
//	pTriangleVerts[1].nz	= rNormCol1.Z;
	pTriangleVerts[1].color	= m_PenColor;

	//---	set vertex 1
	pTriangleVerts[2].x		= rvPos3.X;
	pTriangleVerts[2].y		= rvPos3.Y;
	pTriangleVerts[2].z		= rvPos3.Z;
//	pTriangleVerts[2].nx	= rNormCol1.X;
//	pTriangleVerts[2].ny	= rNormCol1.Y;
//	pTriangleVerts[2].nz	= rNormCol1.Z;
	pTriangleVerts[2].color	= m_PenColor;

#else

	//---	coordinates given are in screen coordinates

	f32 p0zbuff = (rvPos1.Z-NEAR_PLANE) / FAR_PLANE;
	f32 p1zbuff = (rvPos2.Z-NEAR_PLANE) / FAR_PLANE;
	f32 p2zbuff = (rvPos3.Z-NEAR_PLANE) / FAR_PLANE;
	if		( p0zbuff < 0.0f ) p0zbuff = 0.0f;
	else if	( p0zbuff > 1.0f ) p0zbuff = 1.0f;
	if		( p1zbuff < 0.0f ) p1zbuff = 0.0f;
	else if	( p1zbuff > 1.0f ) p1zbuff = 1.0f;
	if		( p2zbuff < 0.0f ) p2zbuff = 0.0f;
	else if	( p2zbuff > 1.0f ) p2zbuff = 1.0f;
	p0zbuff *= p0zbuff;
	p1zbuff *= p1zbuff;
	p2zbuff *= p2zbuff;

	//---	set vertex 0
	pTriangleVerts[0].x		= rvPos1.X;
	pTriangleVerts[0].y		= rvPos1.Y;
	pTriangleVerts[0].z		= p0zbuff;
	pTriangleVerts[0].rhw	= 1.0f;
	pTriangleVerts[0].color	= D3DCOLOR_XRGB((s32)(rNormCol1.X*255), (s32)(rNormCol1.Y*255), (s32)(rNormCol1.Z*255) );

	//---	set vertex 1
	pTriangleVerts[1].x		= rvPos2.X;
	pTriangleVerts[1].y		= rvPos2.Y;
	pTriangleVerts[1].z		= p1zbuff;
	pTriangleVerts[1].rhw	= 1.0f;
	pTriangleVerts[1].color	= D3DCOLOR_XRGB((s32)(rNormCol2.X*255), (s32)(rNormCol2.Y*255), (s32)(rNormCol2.Z*255) );

	//---	set vertex 2
	pTriangleVerts[2].x		= rvPos3.X;
	pTriangleVerts[2].y		= rvPos3.Y;
	pTriangleVerts[2].z		= p2zbuff;
	pTriangleVerts[2].rhw	= 1.0f;
	pTriangleVerts[2].color	= D3DCOLOR_XRGB((s32)(rNormCol3.X*255), (s32)(rNormCol3.Y*255), (s32)(rNormCol3.Z*255) );

#endif

	//---	if the drawn Triangles are not known
	if( m_DrawTrianglesState != KNOWNCOUNT )
	{
		m_pTransVB->Unlock();

		//---	if the Triangles are drawn indevidually, draw this Triangle
		if( m_DrawTrianglesState == INDIVIDUAL )
			m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, (u32)(m_TransVBOffset*(1.0f/sizeof(STransVert))), 1 );
	}

	//---	increment our buffer offset to reflect these to verts
	m_TransVBOffset += 3*sizeof( STransVert );

	//---	record the number of drawn Triangles for this group
	m_DrawTrianglesCount++;
}

//==========================================================================
void CRenderContextD3D::StartDrawLines( s32 nLines )
{
	if( nLines == 0 )
		m_DrawLinesState = UNKNOWNCOUNT;
	else
	{
		m_nLockedTransVerts = 2*nLines;

		//---	lock the buffer now to optimize the process
		m_pLockedTransVerts = Reserve3DVerts( m_nLockedTransVerts );
		if( !m_pLockedTransVerts )	return;

		m_DrawLinesState = KNOWNCOUNT;
	}

	//---	clear the drawn lines count
	m_DrawLinesCount	= 0;

	//---	store the offset at the start
	m_DrawLinesOffset	= m_TransVBOffset;
}

//==========================================================================
void CRenderContextD3D::EndDrawLines( void )
{
	//---	nothing is done here for individual lines, they were drawn one at a time
	if( m_DrawLinesState == INDIVIDUAL )
		return;

	//---	unlock the buffer if the count was known
	if( m_DrawLinesState == KNOWNCOUNT )
	{
		//---	make sure the locked buffer wasn't overrun (if it was locked)
		ASSERT( 2*m_DrawLinesCount <= m_nLockedTransVerts );

		m_pTransVB->Unlock();
	}

	//---	draw the lines
    m_pd3dDevice->DrawPrimitive( D3DPT_LINELIST, (u32)(m_DrawLinesOffset*(1.0f/sizeof(STransVert))), m_DrawLinesCount );

	//---	go back to indevidual mode by default
	m_DrawLinesState = INDIVIDUAL;
}


//==========================================================================
void CRenderContextD3D::DrawLine( vector3& rvPos0, vector3& rvPos1 )
{
	STransVert* pLineVerts;

	s32 NewVBOffset = m_TransVBOffset+2*sizeof(STransVert);
	if( NewVBOffset > TRANSVB_SIZE ) 
		return;

	if( m_DrawLinesState != KNOWNCOUNT )
	{
		pLineVerts = Reserve3DVerts( 2 );
		if( !pLineVerts )	return;
	}
	else
	{
		pLineVerts = &m_pLockedTransVerts[2*m_DrawLinesCount];
	}

#ifndef RENDER_POINTS_IN_SCREENSPACE

	//---	coordinates given are in world coordinates

	//---	set vertex 0
	pLineVerts[0].x		= rvPos0.X;
	pLineVerts[0].y		= rvPos0.Y;
	pLineVerts[0].z		= rvPos0.Z;
	pLineVerts[0].color	= m_PenColor;

	//---	set vertex 1
	pLineVerts[1].x		= rvPos1.X;
	pLineVerts[1].y		= rvPos1.Y;
	pLineVerts[1].z		= rvPos1.Z;
	pLineVerts[1].color	= m_PenColor;

#else

	//---	coordinates given are in screen coordinates

	f32 p0zbuff = (rvPos0.Z-NEAR_PLANE) / FAR_PLANE;
	f32 p1zbuff = (rvPos1.Z-NEAR_PLANE) / FAR_PLANE;
	if		( p0zbuff < 0.0f ) p0zbuff = 0.0f;
	else if	( p0zbuff > 1.0f ) p0zbuff = 1.0f;
	if		( p1zbuff < 0.0f ) p1zbuff = 0.0f;
	else if	( p1zbuff > 1.0f ) p1zbuff = 1.0f;
	p0zbuff *= p0zbuff;
	p1zbuff *= p1zbuff;

	//---	set vertex 0
	pLineVerts[0].x		= rvPos0.X;
	pLineVerts[0].y		= rvPos0.Y;
	pLineVerts[0].z		= p0zbuff;
	pLineVerts[0].rhw	= 1.0f;
	pLineVerts[0].color	= m_PenColor;

	//---	set vertex 1
	pLineVerts[1].x		= rvPos1.X;
	pLineVerts[1].y		= rvPos1.Y;
	pLineVerts[1].z		= p1zbuff;
	pLineVerts[1].rhw	= 1.0f;
	pLineVerts[1].color	= m_PenColor;

#endif

	//---	if the drawn lines are not known
	if( m_DrawLinesState != KNOWNCOUNT )
	{
		m_pTransVB->Unlock();

		//---	if the lines are drawn indevidually, draw this line
		if( m_DrawLinesState == INDIVIDUAL )
			m_pd3dDevice->DrawPrimitive( D3DPT_LINELIST, (u32)(m_TransVBOffset*(1.0f/sizeof(STransVert))), 1 );
	}

	//---	increment our buffer offset to reflect these to verts
	m_TransVBOffset += 2*sizeof( STransVert );

	//---	record the number of drawn lines for this group
	m_DrawLinesCount++;
}

//==========================================================================
void CRenderContextD3D::StartDrawPixels( s32 nPixels )
{
	if( nPixels == 0 )
		m_DrawPixelsState = UNKNOWNCOUNT;
	else
	{
		m_nLockedTransVerts = nPixels;

		//---	lock the buffer now to optimize the process
		m_pLockedTransVerts = Reserve3DVerts( m_nLockedTransVerts );
		if( !m_pLockedTransVerts )	return;

		m_DrawPixelsState = KNOWNCOUNT;
	}

	//---	clear the drawn lines count
	m_DrawPixelsCount	= 0;

	//---	store the offset at the start
	m_DrawPixelsOffset	= m_TransVBOffset;
}

//==========================================================================
void CRenderContextD3D::EndDrawPixels( void )
{
	//---	nothing is done here for individual pixels, they were drawn one at a time
	if( m_DrawPixelsState == INDIVIDUAL )
		return;

	//---	unlock the buffer if the count was known
	if( m_DrawPixelsState == KNOWNCOUNT )
	{
		//---	make sure the locked buffer wasn't overrun (if it was locked)
		ASSERT( m_DrawPixelsCount <= m_nLockedTransVerts );

		m_pTransVB->Unlock();
	}

	//---	draw the lines
    m_pd3dDevice->DrawPrimitive( D3DPT_POINTLIST, (u32)(m_DrawPixelsOffset*(1.0f/sizeof(STransVert))), m_DrawPixelsCount );

	//---	go back to indevidual mode by default
	m_DrawPixelsState = INDIVIDUAL;
}

//==========================================================================
void CRenderContextD3D::DrawPixel( vector3& rvPos )
{
	STransVert* pPixelVerts;

	s32 NewVBOffset = m_TransVBOffset+sizeof(STransVert);
	if( NewVBOffset > TRANSVB_SIZE ) 
		return;

	if( m_DrawPixelsState != KNOWNCOUNT )
	{
		pPixelVerts = Reserve3DVerts( 1 );
		if( !pPixelVerts )	return;
	}
	else
	{
		pPixelVerts = &m_pLockedTransVerts[m_DrawPixelsCount];
	}

#ifdef RENDER_POINTS_IN_SCREENSPACE

	f32 pzbuff = (rvPos.Z-NEAR_PLANE) / FAR_PLANE;
	if		( pzbuff < 0.0f ) pzbuff = 0.0f;
	else if	( pzbuff > 1.0f ) pzbuff = 1.0f;
	pzbuff *= pzbuff;

	//---	set vertex 0
	pPixelVerts[0].x		= rvPos.X;
	pPixelVerts[0].y		= rvPos.Y;
	pPixelVerts[0].z		= pzbuff;
	pPixelVerts[0].rhw		= 1.0f;
	pPixelVerts[0].color	= m_PenColor;

#else

	//---	set vertex 0
	pPixelVerts[0].x		= rvPos.X;
	pPixelVerts[0].y		= rvPos.Y;
	pPixelVerts[0].z		= rvPos.Z;
	pPixelVerts[0].color	= m_PenColor;

#endif

	//---	if the drawn lines are not known
	if( m_DrawPixelsState != KNOWNCOUNT )
	{
		m_pTransVB->Unlock();

		//---	if the lines are drawn indevidually, draw this line
		if( m_DrawPixelsState == INDIVIDUAL )
			m_pd3dDevice->DrawPrimitive( D3DPT_POINTLIST, (u32)(m_TransVBOffset*(1.0f/sizeof(STransVert))), 1 );
	}

	//---	increment our buffer offset to reflect these to verts
	m_TransVBOffset += sizeof( STransVert );

	//---	record the number of drawn lines for this group
	m_DrawPixelsCount++;
}


//==========================================================================
//==========================================================================
//		SKIN STUFF
//==========================================================================
//==========================================================================

//==========================================================================
CRenderContextD3D::SSkinD3D* CRenderContextD3D::GetSkinD3D( CSkin* pSkin )
{
	s32 i;

	//---	return NULL if no skin was requested
	if( pSkin == NULL )
		return NULL;

	//---	attempt to find the skin in the list
	for( i=0; i<m_NumSkins; i++ )
		if( m_pSkins[i].pSkin == pSkin )
			break;

	//---	if the skin wasn't found, add it (unless we are out of room in our list)
	if( i==m_NumSkins )
	{
		ASSERT((m_NumSkins >= 0)&&(m_NumSkins <= MAX_NUM_SKINS));

		//---	if there isn't any room in the list return now
		if( m_NumSkins == MAX_NUM_SKINS )
			return NULL;

		//---	build the D3D skin mesh data, return if there isn't enough RAM
		if( !SkinToSkinD3D( pSkin, &m_pSkins[i] ) )
			return NULL;

		//---	increment the number of skins to show that we have added one
		m_NumSkins++;
	}

	//---	return the found skin
	return &m_pSkins[i];
}


//==========================================================================
void CRenderContextD3D::ClearSkins( void )
{
	SSkinD3D*				pSkinD3D;
	SSkinD3D::SSkinMesh*	pMeshD3D;
	s32						i, j;

	for( i=0; i<m_NumSkins; i++ )
	{
		pSkinD3D = &m_pSkins[i];

		if( pSkinD3D->pSkinMeshes )
		{
			for( j=0; j<pSkinD3D->NumMeshes; j++ )
			{
				pMeshD3D = &pSkinD3D->pSkinMeshes[j];
				if( pMeshD3D->pVB ) pMeshD3D->pVB->Release();
				if( pMeshD3D->pIB ) pMeshD3D->pIB->Release();
			}

			delete[] pSkinD3D->pSkinMeshes;
		}
	}

	m_NumSkins = 0;
}


#define	REMAP_VERTEX_MATRICES


//==========================================================================
void CRenderContextD3D::SetAndIncVertData( s32* MatrixList, s32 MatrixListLength, u8*& pVertD3D, u8* pVertex, s32 NumWeights )
{
	mesh::vertex* pVert = (mesh::vertex*)pVertex;
	u32 Color;
	s32	i;
//	f32 r, g, b; r = g = b = 0.8f;
//	Color	= D3DCOLOR_XRGB((u8)(r*255.0f), (u8)(g*255.0f), (u8)(b*255.0f));
	Color = D3DCOLOR_XRGB((u8)(pVert->m_Color.X*255.0f), (u8)(pVert->m_Color.Y*255.0f), (u8)(pVert->m_Color.Z*255.0f));

#ifdef INDEXED_BLENDING

	//---	remap the matrix weights and matrix
	s32	j;
	s32	Mats[4];
	f32	Weights[4];
	for( i=0; i<pVert->m_nWeights; i++ )
	{
#ifdef REMAP_VERTEX_MATRICES
		for( j=0; j<MatrixListLength; j++ )
			if( pVert->m_Weight[i].m_iMatrix == MatrixList[j] )
				break;
		ASSERT(j!=MatrixListLength);
		Mats[i] = j;
#else
		Mats[i] = pVert->m_Weight[i].m_iMatrix;
#endif
		Weights[i] = pVert->m_Weight[i].m_Weight;
	}
	for( ; i<4; i++ )
	{
		Mats[i]		= 0;
		Weights[i]	= 0.0f;
	}

	u32 MatrixIndices	= (Mats[0] << 0) |
						  (Mats[1] << 8) |
						  (Mats[2] << 16) |
						  (Mats[3] << 24);

#else

	f32	Weights[4];
	for( i=0; i<pVert->m_nWeights; i++ )
		Weights[i] = pVert->m_Weight[i].m_Weight;

#endif

	if( NumWeights == 1 )
	{
		SSkinVertB0* pV = (SSkinVertB0*) pVertD3D;

		pV->x				= -pVert->m_vPos.X;
		pV->y				= pVert->m_vPos.Y;
		pV->z				= -pVert->m_vPos.Z;
#ifdef INDEXED_BLENDING
		pV->MatrixIndices	= MatrixIndices;
#endif
		pV->nx				= -pVert->m_vNormal.X;
		pV->ny				= pVert->m_vNormal.Y;
		pV->nz				= -pVert->m_vNormal.Z;
		pV->color			= Color;

		pVertD3D = (u8*) ++pV;
	}
	else if( NumWeights == 2 )
	{
		SSkinVertB1* pV = (SSkinVertB1*) pVertD3D;

		pV->x				= -pVert->m_vPos.X;
		pV->y				= pVert->m_vPos.Y;
		pV->z				= -pVert->m_vPos.Z;
		pV->b				= Weights[0];
#ifdef INDEXED_BLENDING
		pV->MatrixIndices	= MatrixIndices;
#endif
		pV->nx				= -pVert->m_vNormal.X;
		pV->ny				= pVert->m_vNormal.Y;
		pV->nz				= -pVert->m_vNormal.Z;
		pV->color			= Color;

		pVertD3D = (u8*) ++pV;
	}
	else if( NumWeights == 3 )
	{
		SSkinVertB2* pV = (SSkinVertB2*) pVertD3D;

		pV->x				= -pVert->m_vPos.X;
		pV->y				= pVert->m_vPos.Y;
		pV->z				= -pVert->m_vPos.Z;
		pV->b[0]			= Weights[0];
		pV->b[1]			= Weights[1];
#ifdef INDEXED_BLENDING
		pV->MatrixIndices	= MatrixIndices;
#endif
		pV->nx				= -pVert->m_vNormal.X;
		pV->ny				= pVert->m_vNormal.Y;
		pV->nz				= -pVert->m_vNormal.Z;
		pV->color			= Color;

		pVertD3D = (u8*) ++pV;
	}
	else if( NumWeights == 4 )
	{
		SSkinVertB3* pV = (SSkinVertB3*) pVertD3D;

		pV->x				= -pVert->m_vPos.X;
		pV->y				= pVert->m_vPos.Y;
		pV->z				= -pVert->m_vPos.Z;
		pV->b[0]			= Weights[0];
		pV->b[1]			= Weights[1];
		pV->b[2]			= Weights[2];
#ifdef INDEXED_BLENDING
		pV->MatrixIndices	= MatrixIndices;
#endif
		pV->nx				= -pVert->m_vNormal.X;
		pV->ny				= pVert->m_vNormal.Y;
		pV->nz				= -pVert->m_vNormal.Z;
		pV->color			= Color;

		pVertD3D = (u8*) ++pV;
	}
}

//==========================================================================
xbool CRenderContextD3D::SkinToSkinD3D( CSkin* pSkin, CRenderContextD3D::SSkinD3D* pSkinD3D )
{
	mesh::object*			pMesh = pSkin->GetMesh();
	mesh::chunk*			pChunk;
	mesh::vertex*			pVert;
	mesh::face*				pFace;
	SSkinD3D::SSkinMesh*	pD3DMesh;
	s32						i, j, k, l;
	s32						nVerts;
	LPDIRECT3DVERTEXBUFFER8	pVB;
	u8*						pVertD3D;
	s32						nIndices;
	LPDIRECT3DINDEXBUFFER8	pIB;
	u16*					pIndD3D;
	static HRESULT			hres = D3D_OK;

	//---	without a mesh, there is nothing to convert
	if( pMesh == NULL )
		return FALSE;

	//---	this was to help with a debugging issue.
	if( hres != D3D_OK )
		return FALSE;

	pSkinD3D->pSkin			= pSkin;

	pSkinD3D->NumMeshes		= pMesh->m_nChunks;
	pSkinD3D->pSkinMeshes	= new SSkinD3D::SSkinMesh[pSkinD3D->NumMeshes];

	if( pSkinD3D->pSkinMeshes == NULL )
		return FALSE;

	u32 BaseIndex = 0;	// used for combining chunks
	for( i=0; i<pSkinD3D->NumMeshes; i++ )
	{
		pChunk		= &pMesh->m_pChunk[i];
		pD3DMesh	= &pSkinD3D->pSkinMeshes[i];

		//---	count the number of verts and indices
		nVerts = pChunk->m_nVerts;
		nIndices	= 0;
		for( j=0; j<pChunk->m_nFaces; j++ )
		{
			if( pChunk->m_pFace[j].m_bQuad )
				nIndices += 6;
			else
				nIndices += 3;
		}

		s32	NumWeights			= pChunk->m_pVertex->m_nWeights;
		if		( NumWeights >= 4 )	NumWeights = 4;
		else if ( NumWeights < 1 )	NumWeights = 1;

		pD3DMesh->VB_FVF		= (NumWeights==1) ? SKINB0VB_FVF :
								  (NumWeights==2) ? SKINB1VB_FVF :
								  (NumWeights==3) ? SKINB2VB_FVF :
													SKINB3VB_FVF;
		pD3DMesh->VertSize		= (NumWeights==1) ? sizeof(SSkinVertB0) :
								  (NumWeights==2) ? sizeof(SSkinVertB1) :
								  (NumWeights==3) ? sizeof(SSkinVertB2) :
												    sizeof(SSkinVertB3);
		pD3DMesh->NumVerts		= nVerts;
		pD3DMesh->NumTriangles	= nIndices / 3;

#ifdef REMAP_VERTEX_MATRICES
		//---	build the list of bone matrix indices
		pD3DMesh->NumBoneWeight	= 0;
		for( j=0; j<pChunk->m_nVerts; j++ )
		{
			for( k=0; k<pChunk->m_pVertex[j].m_nWeights; k++ )
			{
				for( l=0; l<pD3DMesh->NumBoneWeight; l++ )
					if( pD3DMesh->BoneMatrixID[l] == pChunk->m_pVertex[j].m_Weight[k].m_iMatrix )
						break;

				if( l==pD3DMesh->NumBoneWeight )
				{
					pD3DMesh->BoneMatrixID[l] = pChunk->m_pVertex[j].m_Weight[k].m_iMatrix;
					pD3DMesh->NumBoneWeight++;
				}
			}
		}
#else
		pD3DMesh->NumBoneWeight		= pChunk->m_pVertex[0].m_nWeights;
		pD3DMesh->BoneMatrixID[0]	= pChunk->m_pVertex[0].m_Weight[0].m_iMatrix;
		pD3DMesh->BoneMatrixID[1]	= pChunk->m_pVertex[0].m_Weight[1].m_iMatrix;
		pD3DMesh->BoneMatrixID[2]	= pChunk->m_pVertex[0].m_Weight[2].m_iMatrix;
		pD3DMesh->BoneMatrixID[3]	= pChunk->m_pVertex[0].m_Weight[3].m_iMatrix;
#endif

		//---	attempt to create the vertex buffer
		hres = m_pd3dDevice->CreateVertexBuffer( nVerts*pD3DMesh->VertSize, D3DUSAGE_WRITEONLY,
										  SKINB0VB_FVF, D3DPOOL_MANAGED, &pD3DMesh->pVB );
		if( hres != D3D_OK )
			m_pD3DView->DisplayErrorMsg( hres, CD3DView::MSGERR_APPMUSTEXIT );
		ASSERT(hres == D3D_OK);

		//---	attempt to create the index buffer
		hres = m_pd3dDevice->CreateIndexBuffer( nIndices*sizeof(u16), D3DUSAGE_WRITEONLY,
										  D3DFMT_INDEX16, D3DPOOL_MANAGED, &pD3DMesh->pIB );
		if( hres != D3D_OK )
			m_pD3DView->DisplayErrorMsg( hres, CD3DView::MSGERR_APPMUSTEXIT );
		ASSERT(hres == D3D_OK);

		//---	get the vertex data buffer
		pVB	= pD3DMesh->pVB;
		hres = pVB->Lock( 0, 0, (u8**)&pVertD3D, D3DLOCK_NOSYSLOCK );
		if( hres != D3D_OK )
			m_pD3DView->DisplayErrorMsg( hres, CD3DView::MSGERR_APPMUSTEXIT );
		ASSERT(hres == D3D_OK);

		//---	get the index data buffer
		pIB = pD3DMesh->pIB;
		hres = pIB->Lock( 0, 0, (u8**)&pIndD3D, D3DLOCK_NOSYSLOCK );
		if( hres != D3D_OK )
			m_pD3DView->DisplayErrorMsg( hres, CD3DView::MSGERR_APPMUSTEXIT );
		ASSERT(hres == D3D_OK);

		//---	add the vertices of this chunk
		for( j=0; j<pChunk->m_nVerts; j++ )
		{
			pVert = &pChunk->m_pVertex[j];

			SetAndIncVertData( pD3DMesh->BoneMatrixID, pD3DMesh->NumBoneWeight, (u8*)pVertD3D, (u8*)pVert, NumWeights );
		}

		//---	add the indices of this chunk
		for( j=0; j<pChunk->m_nFaces; j++ )
		{
			pFace = &pChunk->m_pFace[j];
			if( pFace->m_bQuad )
			{
			#ifndef DONT_CONVERT_TO_LEFTHANDRULE
				*pIndD3D++ = pFace->m_Index[0];
				*pIndD3D++ = pFace->m_Index[2];
				*pIndD3D++ = pFace->m_Index[1];
				*pIndD3D++ = pFace->m_Index[2];
				*pIndD3D++ = pFace->m_Index[3];
				*pIndD3D++ = pFace->m_Index[2];
			#else
				*pIndD3D++ = pFace->m_Index[0];
				*pIndD3D++ = pFace->m_Index[1];
				*pIndD3D++ = pFace->m_Index[2];
				*pIndD3D++ = pFace->m_Index[2];
				*pIndD3D++ = pFace->m_Index[1];
				*pIndD3D++ = pFace->m_Index[3];
			#endif
			}
			else
			{
			#ifndef DONT_CONVERT_TO_LEFTHANDRULE
				*pIndD3D++ = pFace->m_Index[0];
				*pIndD3D++ = pFace->m_Index[2];
				*pIndD3D++ = pFace->m_Index[1];
			#else
				*pIndD3D++ = pFace->m_Index[0];
				*pIndD3D++ = pFace->m_Index[1];
				*pIndD3D++ = pFace->m_Index[2];
			#endif
			}
		}

		pVB->Unlock();
		pIB->Unlock();
	}

	return TRUE;
}


//==========================================================================
//==========================================================================
//==========================================================================
//==========================================================================
//==========================================================================
void CRenderContextD3D::RENDER_ClipProjLine3d( vector3  *tp1, vector3  *tp2 )
{
	vector3 	tp[2] ;
	tp[0] = *tp1;
	tp[1] = *tp2;
	RENDER_ClipProjLine3d( &tp[0] ) ;
}

//==========================================================================
void CRenderContextD3D::RENDER_ClipProjLine3d( vector3  *tp )
{
#ifdef RENDER_POINTS_IN_SCREENSPACE

	vector3 	cp[2] ;
	vector3 	pp[2] ;

	f32			d = m_pCamera->m_DX ;

	//---	Get Clip Codes
	bool c0 = (tp[0].Z < NEAR_PLANE) ;
	bool c1 = (tp[1].Z < NEAR_PLANE) ;

	//---	If both points clipped then exit
	if( c0 && c1 )
		return ;

	//---	Check for clipped
	if( c0 || c1 )
	{
		vector3 	CV ;
		
		V3_ClipLineSegment( &CV, &tp[0], &tp[1], 0, 0, 1, -NEAR_PLANE ) ;

		if( c0 )
		{
			//---	Point 0 clipped
			cp[0] = CV ;
			cp[1] = tp[1] ;
		}
		else
		{
			//---	Point 1 clipped
			cp[0] = tp[0] ;
			cp[1] = CV ;
		}
	}
	else
	{
		//---	No Clipping
		cp[0] = tp[0] ;
		cp[1] = tp[1] ;
	}

	//---	Clip to Right of Screen
	c0 = (cp[0].Z * (cx/d)) < cp[0].X ;
	c1 = (cp[1].Z * (cx/d)) < cp[1].X ;

	//---	If both points clipped then exit
	if( c0 && c1 )
		return ;

	if( c0 || c1 )
	{
		vector3 	CV ;
		vector3 	n ;
		n.Set( -d, 0, cx ) ;
		n.Normalize();
		V3_ClipLineSegment( &CV, &cp[0], &cp[1], n.X, n.Y, n.Z, 0 ) ;

		if( c0 )
		{
			//---	Point 0 clipped
			cp[0] = CV ;
		}
		else
		{
			//---	Point 1 clipped
			cp[1] = CV ;
		}
	}

	//---	Clip to Left of Screen
	c0 = (cp[0].Z * (-cx/d)) > cp[0].X ;
	c1 = (cp[1].Z * (-cx/d)) > cp[1].X ;

	//---	If both points clipped then exit
	if( c0 && c1 )
		return ;

	if( c0 || c1 )
	{
		vector3 	CV ;
		vector3 	n ;
		n.Set( d, 0, cx ) ;
		n.Normalize();
		V3_ClipLineSegment( &CV, &cp[0], &cp[1], n.X, n.Y, n.Z, 0 ) ;

		if( c0 )
		{
			//---	Point 0 clipped
			cp[0] = CV ;
		}
		else
		{
			//---	Point 1 clipped
			cp[1] = CV ;
		}
	}

	//---	Clip to Top of Screen
	c0 = (cp[0].Z * (cy/d)) < cp[0].Y ;
	c1 = (cp[1].Z * (cy/d)) < cp[1].Y ;

	//---	If both points clipped then exit
	if( c0 && c1 )
		return ;

	if( c0 || c1 )
	{
		vector3 	CV ;
		vector3 	n ;
		n.Set( 0, -d, cy ) ;
		n.Normalize();
		V3_ClipLineSegment( &CV, &cp[0], &cp[1], n.X, n.Y, n.Z, 0 ) ;

		if( c0 )
		{
			//---	Point 0 clipped
			cp[0] = CV ;
		}
		else
		{
			//---	Point 1 clipped
			cp[1] = CV ;
		}
	}

	//---	Clip to Bottom of Screen
	c0 = (cp[0].Z * (-cy/d)) > cp[0].Y ;
	c1 = (cp[1].Z * (-cy/d)) > cp[1].Y ;

	//---	If both points clipped then exit
	if( c0 && c1 )
		return ;

	if( c0 || c1 )
	{
		vector3 	CV ;
		vector3 	n ;
		n.Set( 0, d, cy ) ;
		n.Normalize();
		V3_ClipLineSegment( &CV, &cp[0], &cp[1], n.X, n.Y, n.Z, 0 ) ;

		if( c0 )
		{
			//---	Point 0 clipped
			cp[0] = CV ;
		}
		else
		{
			//---	Point 1 clipped
			cp[1] = CV ;
		}
	}

	//---	Project Clipped points to screen
	m_pCamera->ProjectD3D( pp, cp, 2 ) ;

	//---	Draw Line
	DrawLine( pp[0], pp[1] );

#else

	DrawLine( tp[0], tp[1] );

#endif
}


//==========================================================================
void CRenderContextD3D::RENDER_ClipProjTriangle3d( vector3 *tp, vector3 *nc )
{
#ifdef RENDER_POINTS_IN_SCREENSPACE

	vector3 	pp[3] ;

	f32			d = m_pCamera->m_DX ;

	//---	Get Clip Codes
	if(tp[0].Z < NEAR_PLANE)	return;
	if(tp[1].Z < NEAR_PLANE)	return;
	if(tp[2].Z < NEAR_PLANE)	return;

	//---	Clip to Right of Screen
	if( tp[0].Z * (cx/d)) < tp[0].X)	return;
	if( tp[1].Z * (cx/d)) < tp[1].X)	return;
	if( tp[2].Z * (cx/d)) < tp[2].X)	return;

	//---	Clip to Left of Screen
	if( tp[0].Z * (-cx/d)) > tp[0].X)	return;
	if( tp[1].Z * (-cx/d)) > tp[1].X)	return;
	if( tp[2].Z * (-cx/d)) > tp[2].X)	return;

	//---	Clip to Top of Screen
	if( tp[0].Z * (cy/d)) < tp[0].Y)	return;
	if( tp[1].Z * (cy/d)) < tp[1].Y)	return;
	if( tp[2].Z * (cy/d)) < tp[2].Y)	return;

	//---	Clip to Bottom of Screen
	if( tp[0].Z * (-cy/d)) > tp[0].Y)	return;
	if( tp[1].Z * (-cy/d)) > tp[1].Y)	return;
	if( tp[2].Z * (-cy/d)) > tp[2].Y)	return;

	//---	Project Clipped points to screen
	m_pCamera->ProjectD3D( pp, tp, 3 ) ;

	DrawTriangle( pp[0], nc[0], pp[1], nc[1], pp[2], nc[2] );

#else

	DrawTriangle( tp[0], nc[0], tp[1], nc[1], tp[2], nc[2] );

#endif
}
//==========================================================================
void CRenderContextD3D::RENDER_TransClipProjLine3d( vector3  *tp1, vector3  *tp2 )
{
	vector3 	tp[2] ;
	tp[0] = *tp1;
	tp[1] = *tp2;
	RENDER_TransClipProjLine3d( &tp[0] ) ;
}

//==========================================================================
void CRenderContextD3D::RENDER_TransClipProjLine3d( vector3  *p )
{
	#ifdef RENDER_POINTS_IN_SCREENSPACE

		vector3 	tp[2] ;
		m_pCamera->m_Matrix.Transform( tp, p, 2 ) ;
		RENDER_ClipProjLine3d( tp ) ;

	#else

		DrawLine( p[0], p[1] );

	#endif
}

//==========================================================================
void CRenderContextD3D::RENDER_TransClipProjTriangle3d( vector3 *p, vector3 *nc )
{
	#ifdef RENDER_POINTS_IN_SCREENSPACE

		vector3 	tp[3] ;

		//---	transform the vertices
		m_pCamera->m_Matrix.Transform( tp, p, 3 ) ;

		//---	calculate the normal of the triangle to determine clipping
		vector3		v1( p[1].X - p[0].X, p[1].Y - p[0].Y, p[1].Z - p[0].Z );
		vector3		v2( p[2].X - p[0].X, p[2].Y - p[0].Y, p[2].Z - p[0].Z );

		//---	if the Z of the normal is facing away from the screen, don't bother drawing it
		if( ((v1.X * v2.Y) - (v1.Y * v2.X)) < 0.0f )
			return;

		RENDER_ClipProjTriangle3d( tp, nc ) ;

	#else

		DrawTriangle( p[0], nc[0], p[1], nc[1], p[2], nc[2] );

	#endif
}

//==========================================================================
void CRenderContextD3D::RENDER_TransClipProjTriangle3d( vector3 &v1, vector3& nc1, vector3 &v2, vector3& nc2, vector3 &v3, vector3& nc3 )
{
	vector3 p[3];
	vector3 nc[3];

	p[0] = v1;
	p[1] = v2;
	p[2] = v3;

	nc[0] = nc1;
	nc[1] = nc2;
	nc[2] = nc3;

	RENDER_TransClipProjTriangle3d( p, nc );
}


#define	MIN_CIRCLE_POINTS	4
#define	MAX_CIRCLE_POINTS	32

//==========================================================================
void CRenderContextD3D::RENDER_TransClipProjCircle3d( f32 r )
{
	vector3 	p[MAX_CIRCLE_POINTS+1] ;
	vector3 	tp[MAX_CIRCLE_POINTS+1] ;

	//---	Generate Circle
	int CirclePoints = (int)((MAX_CIRCLE_POINTS-MIN_CIRCLE_POINTS)*x_abs(r)/20.0) + MIN_CIRCLE_POINTS ;
	if( CirclePoints > MAX_CIRCLE_POINTS ) CirclePoints = MAX_CIRCLE_POINTS ;
	for( int i = 0 ; i < (CirclePoints+1) ; i++ )
	{
		f32	a = DEG_TO_RAD(360.0) * (f32)i / (CirclePoints) ;
		p[i].X = x_cos( a ) * r ;
		p[i].Y = 0 ;
		p[i].Z = x_sin( a ) * r ;
	}

	//---	Transform all Points
	#ifdef RENDER_POINTS_IN_SCREENSPACE

		m_pCamera->m_Matrix.Transform( tp, p, CirclePoints+1 ) ;

		//---	Draw Line Segments
		for( i = 0 ; i < (CirclePoints) ; i++ )
		{
			RENDER_ClipProjLine3d( &tp[i] ) ;
		}

	#else

		//---	Draw Line Segments
		for( i = 0 ; i < (CirclePoints) ; i++ )
		{
			RENDER_ClipProjLine3d( &p[i] ) ;
		}

	#endif
}

//==========================================================================
void CRenderContextD3D::RENDER_GridLines( f32 x1, f32 x2, f32 dx,
								 f32 z1, f32 z2, f32 dz )
{
	for( f32 x = x1 ; x <= x2 ; x += dx )
	{
		vector3 	p[2] ;
		p[0].X = x ;
		p[0].Y = 0 ;
		p[0].Z = z1 ;
		p[1].X = x ;
		p[1].Y = 0 ;
		p[1].Z = z2 ;
		RENDER_TransClipProjLine3d( p ) ;
	}
	for( f32 z = z1 ; z <= z2 ; z += dz )
	{
		vector3 	p[2] ;
		p[0].X = x1 ;
		p[0].Y = 0 ;
		p[0].Z = z ;
		p[1].X = x2 ;
		p[1].Y = 0 ;
		p[1].Z = z ;
		RENDER_TransClipProjLine3d( p ) ;
	}
}

//==========================================================================
void CRenderContextD3D::RENDER_Grid( COLORREF ColorMajor, COLORREF ColorMinor )
{
	CPen PenMajor( PS_SOLID, 1, ColorMajor ) ;
	CPen PenMinor( PS_SOLID, 1, ColorMinor ) ;
	CPen *pOldPen = SelectObject( &PenMinor ) ;
	RENDER_GridLines( -400, 400, 10, -400, 400, 10 ) ;
	SelectObject( &PenMajor ) ;
	RENDER_GridLines( -400, 400, 100, -400, 400, 100 ) ;
	SelectObject( pOldPen ) ;
}

//==========================================================================
void CRenderContextD3D::RENDER_Arrow( f32 Len, COLORREF Color )
{
	CPen Pen( PS_SOLID, 1, Color ) ;
	CPen *pOldPen = SelectObject( &Pen ) ;

	//---	Render Arrow Lines
	vector3 	p[2] ;
	p[0].X = 0 ;
	p[0].Y = 0 ;
	p[0].Z = 0 ;
	p[1].X = 0 ;
	p[1].Y = 0 ;
	p[1].Z = Len ;
	f32	PointLen = Len/20 ;
	RENDER_TransClipProjLine3d( p ) ;
	p[0].X = -x_sin(DEG_TO_RAD(30)) * PointLen ;
	p[0].Z = Len -  x_cos(DEG_TO_RAD(30)) * PointLen ;
	RENDER_TransClipProjLine3d( p ) ;
	p[0].X = x_sin(DEG_TO_RAD(30)) * PointLen ;
	p[0].Z = Len -  x_cos(DEG_TO_RAD(30)) * PointLen ;
	RENDER_TransClipProjLine3d( p ) ;

	SelectObject( pOldPen ) ;
}

//==========================================================================
void CRenderContextD3D::RENDER_Axes( f32 Len, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ )
{
	vector3 	p[2] ;
	p[0].Set( 0, 0, 0 ) ;

	CPen PenX( PS_SOLID, 1, ColorX ) ;
	CPen PenY( PS_SOLID, 1, ColorY ) ;
	CPen PenZ( PS_SOLID, 1, ColorZ ) ;
	CPen *pOldPen = SelectObject( &PenX ) ;
	p[1].X = Len ;
	p[1].Y = 0 ;
	p[1].Z = 0 ;
	RENDER_TransClipProjLine3d( p ) ;
	SelectObject( &PenY ) ;
	p[1].X = 0 ;
	p[1].Y = Len ;
	p[1].Z = 0 ;
	RENDER_TransClipProjLine3d( p ) ;
	SelectObject( &PenZ ) ;
	p[1].X = 0 ;
	p[1].Y = 0 ;
	p[1].Z = Len ;
	RENDER_TransClipProjLine3d( p ) ;

	SelectObject( pOldPen ) ;
}

//==========================================================================
void CRenderContextD3D::RENDER_GrabAxes( f32 Len, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ )
{
	vector3 	p[6], tp[6] ;

	CPen PenX( PS_SOLID, 1, ColorX ) ;
	CPen PenY( PS_SOLID, 1, ColorY ) ;
	CPen PenZ( PS_SOLID, 1, ColorZ ) ;

	p[0].Set( -Len, 0, 0 ) ;
	p[1].Set(  Len, 0, 0 ) ;
	p[2].Set( 0, -Len, 0 ) ;
	p[3].Set( 0,  Len, 0 ) ;
	p[4].Set( 0, 0, -Len ) ;
	p[5].Set( 0, 0,  Len ) ;

	//---	Transform all Points
	#ifdef RENDER_POINTS_IN_SCREENSPACE
		m_pCamera->m_Matrix.Transform( tp, p, 6 ) ;
	#endif

	//---	Draw Line Segments
	CPen *pOldPen = SelectObject( &PenX ) ;
	RENDER_ClipProjLine3d( &tp[0] ) ;
	SelectObject( &PenY ) ;
	RENDER_ClipProjLine3d( &tp[2] ) ;
	SelectObject( &PenZ ) ;
	RENDER_ClipProjLine3d( &tp[4] ) ;

	SelectObject( pOldPen ) ;
}

//==========================================================================
void CRenderContextD3D::RENDER_Sphere( f32 Radius, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ )
{
	matrix4		m, m2 ;

	m.Identity();
//	M4_Identity( &m ) ;

	m = GetMatrix();

	CPen PenX( PS_SOLID, 1, ColorX ) ;
	CPen PenY( PS_SOLID, 1, ColorY ) ;
	CPen PenZ( PS_SOLID, 1, ColorZ ) ;
	CPen *pOldPen = SelectObject( &PenY ) ;
	PushMatrix( &m ) ;
	RENDER_TransClipProjCircle3d( Radius ) ;
	PopMatrix( ) ;

	m2.Identity();
	m2.RotateX( DEG_TO_RAD(90) );
	m.PreTransform( m2 );
	SelectObject( &PenZ ) ;
	PushMatrix( &m ) ;
	RENDER_TransClipProjCircle3d( Radius ) ;
	PopMatrix( ) ;

	m2.Identity();
	m2.RotateZ( DEG_TO_RAD(90) );
	m.PreTransform( m2 );
	SelectObject( &PenX ) ;
	PushMatrix( &m ) ;
	RENDER_TransClipProjCircle3d( Radius ) ;
	PopMatrix( ) ;

	SelectObject( pOldPen ) ;
}

//==========================================================================
void CRenderContextD3D::RENDER_Prop( f32 Size, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ )
{
//	matrix4		m;
//	m.Identity();
//	m = GetMatrix();

	CPen PenX( PS_SOLID, 1, ColorX ) ;
	CPen PenY( PS_SOLID, 1, ColorY ) ;
	CPen PenZ( PS_SOLID, 1, ColorZ ) ;

	vector3 Points[7];
	Points[0].Set(  0.0f,  0.0f,  0.0f );
	Points[1].Set(  0.0f,  0.0f,  Size );
	Points[2].Set(  0.0f,  0.0f, -Size );
	Points[3].Set(  Size,  0.0f,  0.0f );
	Points[4].Set( -Size,  0.0f,  0.0f );
	Points[5].Set(  0.0f,  Size,  0.0f );
	Points[6].Set(  0.0f, -Size,  0.0f );

//	PushMatrix( &m ) ;

	//---	Draw X part (with Y color)
	CPen *pOldPen = SelectObject( &PenY );
	RENDER_TransClipProjLine3d( &Points[0], &Points[5] );
	RENDER_TransClipProjLine3d( &Points[5], &Points[1] );
	RENDER_TransClipProjLine3d( &Points[1], &Points[6] );
	RENDER_TransClipProjLine3d( &Points[6], &Points[2] );
	RENDER_TransClipProjLine3d( &Points[2], &Points[5] );

	//---	Draw Y part (with Z color)
	SelectObject( &PenZ );
	RENDER_TransClipProjLine3d( &Points[0], &Points[1] );
	RENDER_TransClipProjLine3d( &Points[1], &Points[3] );
	RENDER_TransClipProjLine3d( &Points[3], &Points[2] );
	RENDER_TransClipProjLine3d( &Points[2], &Points[4] );
	RENDER_TransClipProjLine3d( &Points[4], &Points[1] );

	//---	Draw Z part (with X color)
	SelectObject( &PenX );
	RENDER_TransClipProjLine3d( &Points[0], &Points[3] );
	RENDER_TransClipProjLine3d( &Points[3], &Points[5] );
	RENDER_TransClipProjLine3d( &Points[5], &Points[4] );
	RENDER_TransClipProjLine3d( &Points[4], &Points[6] );
	RENDER_TransClipProjLine3d( &Points[6], &Points[3] );

//	PopMatrix( ) ;

	SelectObject( pOldPen ) ;
}

//==========================================================================
void CRenderContextD3D::RENDER_Marker( f32 Len, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ )
{
	vector3 	p[6], tp[6] ;

	CPen PenX( PS_SOLID, 1, ColorX ) ;
	CPen PenY( PS_SOLID, 1, ColorY ) ;
	CPen PenZ( PS_SOLID, 1, ColorZ ) ;

	p[0].Set( 0, 0,  Len ) ;
	p[1].Set(  Len, 0, 0 ) ;
	p[2].Set( 0, 0, -Len ) ;
	p[3].Set( -Len, 0, 0 ) ;
	p[4].Set( 0, -Len, 0 ) ;
	p[5].Set( 0,  Len, 0 ) ;

	//---	Transform all Points
	#ifdef RENDER_POINTS_IN_SCREENSPACE
		m_pCamera->m_Matrix.Transform( tp, p, 6 ) ;
	#else
		tp[0] = p[0];
		tp[1] = p[1];
		tp[2] = p[2];
		tp[3] = p[3];
		tp[4] = p[4];
		tp[5] = p[5];
	#endif

	//---	Draw Line Segments
	CPen *pOldPen = SelectObject( &PenY ) ;
	RENDER_ClipProjLine3d( &tp[0], &tp[1] ) ;
	RENDER_ClipProjLine3d( &tp[1], &tp[2] ) ;
	RENDER_ClipProjLine3d( &tp[2], &tp[3] ) ;
	RENDER_ClipProjLine3d( &tp[3], &tp[0] ) ;
	SelectObject( &PenX ) ;
	RENDER_ClipProjLine3d( &tp[0], &tp[4] ) ;
	RENDER_ClipProjLine3d( &tp[4], &tp[2] ) ;
	RENDER_ClipProjLine3d( &tp[2], &tp[5] ) ;
	RENDER_ClipProjLine3d( &tp[5], &tp[0] ) ;
	SelectObject( &PenZ ) ;
	RENDER_ClipProjLine3d( &tp[3], &tp[4] ) ;
	RENDER_ClipProjLine3d( &tp[4], &tp[1] ) ;
	RENDER_ClipProjLine3d( &tp[1], &tp[5] ) ;
	RENDER_ClipProjLine3d( &tp[5], &tp[3] ) ;
	SelectObject( pOldPen ) ;
}

//==========================================================================
void CRenderContextD3D::RENDER_TransProjPointArray( vector3  *p, int nPoints, COLORREF Color, bool DrawSolid )
{
	CPen Pen( PS_SOLID, 1, Color );
	CPen* pOldPen = SelectObject( &Pen );

#ifndef RENDER_POINTS_IN_SCREENSPACE

	if( DrawSolid && (nPoints >= 2) )
	{
		for( int i = 1 ; i < nPoints ; i++ )
			DrawLine( p[i-1], p[i] ) ;
	}
	else
	{
		for( int i = 0 ; i < nPoints ; i++ )
			DrawPixel( p[i] ) ;
	}

#else
	vector3 	*tp ;
	vector3 	*pp ;


	tp = (vector3 *)x_malloc( nPoints * sizeof(vector3 ) ) ;
	if( tp )
	{
		pp = (vector3 *)x_malloc( nPoints * sizeof(vector3 ) ) ;
		if( pp )
		{
			//---	Transform and Project
			m_pCamera->m_Matrix.Transform( &tp[0], p, nPoints ) ;

			if( !DrawSolid || (nPoints <= 1) )
			{
				f32 d = m_pCamera->m_DX ;
				m_pCamera->ProjectD3D( &pp[0], &tp[0], nPoints ) ;

				//---	let renderer know that many pixels will be drawn in succession (optimization)
				StartDrawPixels( nPoints );

				//---	Plot Points
				for( int i = 0 ; i < nPoints ; i++ )
				{
					if( (tp[i].Z > NEAR_PLANE) &&
						(pp[i].X > -16384) &&
						(pp[i].X <  16384) &&
						(pp[i].Y > -16384) &&
						(pp[i].Y <  16384) )
					{
						DrawPixel( vector3(pp[i].X, pp[i].Y, pp[i].Z) ) ;
					}
				}

				EndDrawPixels();
			}
			else
			{
				for( int i = 1 ; i < nPoints ; i++ )
				{
					RENDER_ClipProjLine3d( &tp[i-1], &tp[i] ) ;
				}
			}

			//---	Free Projected Points
			x_free( pp ) ;
		}

		//---	Free Transformed Points
		x_free( tp ) ;
	}

#endif

	SelectObject( pOldPen );
}

//==========================================================================
void CRenderContextD3D::RENDER_FillSolidRect( s32 X, s32 Y, s32 W, s32 H, COLORREF rColor )
{
	SVert2D*	pVerts;
	u32 Color;

	Color = rColor; //D3DCOLOR_XRGB((u8)(rColor.X*255.0f), (u8)(rColor.Y*255.0f), (u8)(rColor.Z*255.0f));
	
	pVerts = Reserve2DVerts( 4 );
	if( !pVerts )	return;

	//---	vert 1
	pVerts->x		= (f32)X;
	pVerts->y		= (f32)Y;
	pVerts->z		= 0.0f;
	pVerts->rhw		= 1.0f;
	pVerts->color = Color;
	pVerts++;

	//---	vert 2
	pVerts->x		= (f32)(X+W);
	pVerts->y		= (f32)Y;
	pVerts->z		= 0.0f;
	pVerts->rhw		= 1.0f;
	pVerts->color = Color;
	pVerts++;

	//---	vert 3
	pVerts->x		= (f32)X;
	pVerts->y		= (f32)(Y+H);
	pVerts->z		= 0.0f;
	pVerts->rhw		= 1.0f;
	pVerts->color = Color;
	pVerts++;

	//---	vert 4
	pVerts->x		= (f32)(X+W);
	pVerts->y		= (f32)(Y+H);
	pVerts->z		= 0.0f;
	pVerts->rhw		= 1.0f;
	pVerts->color = Color;
	pVerts++;

	m_pVB2D->Unlock();

	s32 Index = m_VB2DOffset/sizeof(SVert2D);
	m_VB2DOffset += 4*sizeof(SVert2D);

	//---	change the device context for the next operation
	m_pd3dDevice->SetStreamSource( 0, m_pVB2D, sizeof(SVert2D) );
	m_pd3dDevice->SetVertexShader( VB2D_FVF );

	m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, Index, 2 );

	//---	set the device to use this vertex stream
    m_pd3dDevice->SetStreamSource( 0, m_pTransVB, sizeof(STransVert) );
    m_pd3dDevice->SetVertexShader( TRANSVB_FVF );
}


//==========================================================================
void CRenderContextD3D::RENDER_Skin( CSkin* pSkin, CSkeleton* pSkel, matrix4* pMat, s32 MatCount, COLORREF Color )
{
	s32 i;
	SSkinD3D*				pSkinD3D = GetSkinD3D( pSkin );
	SSkinD3D::SSkinMesh*	pMeshD3D;
	SSkelBone*				pBone;
	if( !pSkinD3D )
		return;

	//---	set the light to the given color
//    m_pD3DView->m_Light.Diffuse.r		= ((f32)GetRValue(Color))/255.0f;
//    m_pD3DView->m_Light.Diffuse.g		= ((f32)GetGValue(Color))/255.0f;
//    m_pD3DView->m_Light.Diffuse.b		= ((f32)GetBValue(Color))/255.0f;
//	m_pd3dDevice->SetLight( 0, &m_pD3DView->m_Light );

	vector3			rt = pMat[0].GetTranslation(); // get the root translation

	D3DMATERIAL8	mtrl;
	memset( &mtrl, 0, sizeof(mtrl) );
	mtrl.Diffuse.r = 0.9f;
	mtrl.Diffuse.g = 0.9f;
	mtrl.Diffuse.b = 0.9f;
	mtrl.Diffuse.a = 1.0f;
	mtrl.Ambient.r = 0.3f;
	mtrl.Ambient.g = 0.3f;
	mtrl.Ambient.b = 0.3f;
	mtrl.Ambient.a = 1.0f;
	m_pd3dDevice->SetMaterial( &mtrl );
	m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );

	for( i=0; i<pSkinD3D->NumMeshes; i++ )
	{
		pMeshD3D = &pSkinD3D->pSkinMeshes[i];

		s32 iMatrix;
		s32 j;

		s32 WeightsState;
		switch( pMeshD3D->NumBoneWeight )
		{
		case 1:		WeightsState = D3DVBF_0WEIGHTS;	break;
		case 2:		WeightsState = D3DVBF_1WEIGHTS;	break;
		case 3:		WeightsState = D3DVBF_2WEIGHTS;	break;
		case 4:		WeightsState = D3DVBF_3WEIGHTS;	break;
		}
		HRESULT hres = m_pd3dDevice->SetRenderState( D3DRS_VERTEXBLEND, WeightsState );
		if( hres != D3D_OK )
			m_pD3DView->DisplayErrorMsg( hres, CD3DView::MSGERR_APPMUSTEXIT );

#ifdef INDEXED_BLENDING

		hres = m_pd3dDevice->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, TRUE );
		if( hres != D3D_OK )
			m_pD3DView->DisplayErrorMsg( hres, CD3DView::MSGERR_APPMUSTEXIT );

#endif

		//---	set matrices
		for( j=0; j<pMeshD3D->NumBoneWeight; j++ )
		{
			pBone = pSkel->GetBoneFromIndex(pMeshD3D->BoneMatrixID[j]);
			iMatrix = pBone->BoneID;
			matrix4 m = pMat[iMatrix];
			m.PreTranslate( -pBone->OriginToBone );
			ASSERT(iMatrix<MatCount);
			hres = m_pd3dDevice->SetTransform( D3DTS_WORLDMATRIX(j), (D3DMATRIX*)&m ); //&pMat[iMatrix] );
			if( hres != D3D_OK )
				m_pD3DView->DisplayErrorMsg( hres, CD3DView::MSGERR_APPMUSTEXIT );
		}

		//---	change the device context for the next operation
		m_pd3dDevice->SetStreamSource( 0, pMeshD3D->pVB, pMeshD3D->VertSize );
		m_pd3dDevice->SetVertexShader( pMeshD3D->VB_FVF );
		m_pd3dDevice->SetIndices( pMeshD3D->pIB, 0 );

		//---	draw the primitive
		m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, pMeshD3D->NumVerts, 0, pMeshD3D->NumTriangles );
	}

	//---	set the render state back
	m_pd3dDevice->SetRenderState( D3DRS_VERTEXBLEND, D3DVBF_0WEIGHTS );

	//---	set the device to use this vertex stream
	m_pd3dDevice->SetStreamSource( 0, m_pTransVB, sizeof(STransVert) );
	m_pd3dDevice->SetVertexShader( TRANSVB_FVF );

	//---	the world transform is set to the identity matrix by default.
	matrix4 mat;
	mat.Identity();
	m_pd3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&mat );
	m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
}

//==========================================================================
void CRenderContextD3D::RENDER_Character( CCharacter* pCharacter, matrix4& rTransform, COLORREF Color, u32 Flags )
{
	matrix4		m[100] ;
	vector3 	p[100] ;
	CSkeleton*	pSkel = pCharacter->GetSkeleton();
	CSkin*		pSkin = pCharacter->GetCurSkin();
	s32			i;
	s32			NumMatricies = 0;

	ASSERT( pSkel->GetNumBones() < 100 ) ;

	//---	if we are rendering the bones, get the bone world positions ahead of time
	for( i = 0 ; i < pSkel->GetNumBones() ; i++ )
	{
		int ID = pSkel->GetBoneFromIndex(i)->BoneID ;
		m[ID] = *pSkel->GetPosedBoneMatrix(ID);
		m[ID].Transform( rTransform ); //rTransform.Transform( m[ID] );
		ConvertMatrixToD3D( m[ID] );
		p[ID] = m[ID].GetTranslation();

		if( ID >= NumMatricies )
			NumMatricies = ID+1;
	}

	//---	Set Pen to Draw in
	CPen PenBone( PS_SOLID, 1, Color ) ;
	CPen *pOldPen = SelectObject( &PenBone ) ;

	matrix4 BoneIdentityMatrix;
	BoneIdentityMatrix.Identity();
	ConvertD3DToMatrix( BoneIdentityMatrix );

	//---	Loop through each Bone
	for( i = 0 ; i < pSkel->GetNumBones() ; i++ )
	{
		//---	Draw Bone Axis if required
		if( Flags & RFLAG_AXES )
		{
			matrix4	m ;
			m = *pSkel->GetPosedBoneMatrix( i );
			PushMatrix( &m ) ;
			RENDER_Axes( 2, RGB(255*MOD_COLOR,0,0), RGB(0,255*MOD_COLOR,0), RGB(0,0,255*MOD_COLOR) ) ;
			PopMatrix( ) ;
		}

		//---	Draw Bone to Parent
		if(( Flags & RFLAG_BONES ) && ( pSkel->GetBoneFromIndex(i)->ParentID != -1 ))
		{
			vector3* p1 = &p[pSkel->GetBoneFromIndex(i)->ParentID];
			vector3* p2 = &p[pSkel->GetBoneFromIndex(i)->BoneID];
			PushMatrix( &BoneIdentityMatrix ) ;
			RENDER_TransClipProjLine3d( p1, p2 ) ;
			PopMatrix( ) ;
		}
	}

	//---	Render the mesh tied to this bone
	if( Flags & (RFLAG_POINTS|RFLAG_WIREFRAME|RFLAG_FACETS) )
	{
		if( pCharacter->RenderSkinTranslucent() )
		{
			m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
			m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DCOLOR_ARGB(128,256,256,256) );
			m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DCOLOR_ARGB(128,256,256,256) );
		}

		//---	render this skin
		RENDER_Skin( pSkin, pSkel, m, NumMatricies, Color );
	}

	//---	Restore Pen
	SelectObject( pOldPen ) ;
}

//==========================================================================
void CRenderContextD3D::PushMatrix( matrix4 *pMatrix )
{
	ASSERT( m_MatrixStackIndex < MATRIX_STACK_SIZE ) ;
#if 1

	CRenderContext::PushMatrix( pMatrix );
	matrix4 m = GetMatrix();
	ConvertMatrixToD3D( m );
	m_pd3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&m );

#else

	m_MatrixStack[m_MatrixStackIndex] = *pMatrix;
	ConvertMatrixToD3D( m_MatrixStack[m_MatrixStackIndex] );
	m_pd3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&m_MatrixStack[m_MatrixStackIndex] );

	m_MatrixStackIndex++;
#endif
}

//==========================================================================
void CRenderContextD3D::PopMatrix( )
{
#if 1
	CRenderContext::PopMatrix( );
	matrix4 m = GetMatrix();
	ConvertMatrixToD3D( m );
	m_pd3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&m );
#else
	m_MatrixStackIndex-- ;
	ASSERT( m_MatrixStackIndex >= 0 ) ;

	m_pd3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&m_MatrixStack[m_MatrixStackIndex] );
#endif
}
