// RenderContextD3D.h: interface for the CRenderContextD3D class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RENDERCONTEXTD3D_H__2C859F62_E603_11D1_9507_00207811EE70__INCLUDED_)
#define AFX_RENDERCONTEXTD3D_H__2C859F62_E603_11D1_9507_00207811EE70__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "RenderContext.h"
#include "d3d8.h"


//==========================================================================
class CCamera;
class CD3DView;
class CSkin;
class CCharacter;
class CSkeleton;

//==========================================================================

//#define INDEXED_BLENDING
//#define RENDER_POINTS_IN_SCREENSPACE

//==========================================================================
class CRenderContextD3D : public CRenderContext
{
//---	Construction
public:
	CRenderContextD3D( CD3DView *pD3DView, CCamera *pCamera );
	virtual ~CRenderContextD3D();

public:
	enum{ RCTYPE = 2 };
	s32	GetType( void )		{ return RCTYPE; }

//---	structs/types
protected:

	//---	SKIN support structures
	struct SSkinD3D
	{
		struct SSkinMesh
		{
			s32							BoneMatrixID[4];
			s32							NumBoneWeight;
			s32							VertSize;
			u32							VB_FVF;
			s32							NumVerts;
			s32							NumTriangles;
			LPDIRECT3DVERTEXBUFFER8		pVB;	// vertex buffer
			LPDIRECT3DINDEXBUFFER8		pIB;	// index buffer
		};

		CSkin*						pSkin;	// pointer to the source skin
		s32							NumMeshes;
		SSkinMesh*					pSkinMeshes;
	};

#ifndef RENDER_POINTS_IN_SCREENSPACE
#ifdef INDEXED_BLENDING
	//---	structure for skin verts
	enum
	{
		MAX_NUM_SKINS	= 32,

		SKINB0VB_FVF	= D3DFVF_XYZB1 | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_LASTBETA_UBYTE4,
		SKINB1VB_FVF	= D3DFVF_XYZB2 | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_LASTBETA_UBYTE4,
		SKINB2VB_FVF	= D3DFVF_XYZB3 | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_LASTBETA_UBYTE4,
		SKINB3VB_FVF	= D3DFVF_XYZB4 | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_LASTBETA_UBYTE4,
	};

	struct SSkinVertB0
	{
		f32 x, y, z;
		u32 MatrixIndices;
		f32 nx, ny, nz;
		u32 color;
	};

	struct SSkinVertB1
	{
		f32 x, y, z;
		f32	b;
		u32 MatrixIndices;
		f32 nx, ny, nz;
		u32 color;
	};

	struct SSkinVertB2
	{
		f32 x, y, z;
		f32	b[2];
		u32 MatrixIndices;
		f32 nx, ny, nz;
		u32 color;
	};

	struct SSkinVertB3
	{
		f32 x, y, z;
		f32	b[3];
		u32 MatrixIndices;
		f32 nx, ny, nz;
		u32 color;
	};

#else

	enum
	{
		MAX_NUM_SKINS	= 32,

		SKINB0VB_FVF	= D3DFVF_XYZ   | D3DFVF_NORMAL | D3DFVF_DIFFUSE,
		SKINB1VB_FVF	= D3DFVF_XYZB1 | D3DFVF_NORMAL | D3DFVF_DIFFUSE,
		SKINB2VB_FVF	= D3DFVF_XYZB2 | D3DFVF_NORMAL | D3DFVF_DIFFUSE,
		SKINB3VB_FVF	= D3DFVF_XYZB3 | D3DFVF_NORMAL | D3DFVF_DIFFUSE,
	};

	struct SSkinVertB0
	{
		f32 x, y, z;
		f32 nx, ny, nz;
		u32 color;
	};

	struct SSkinVertB1
	{
		f32 x, y, z;
		f32	b;
		f32 nx, ny, nz;
		u32 color;
	};

	struct SSkinVertB2
	{
		f32 x, y, z;
		f32	b[2];
		f32 nx, ny, nz;
		u32 color;
	};

	struct SSkinVertB3
	{
		f32 x, y, z;
		f32	b[3];
		f32 nx, ny, nz;
		u32 color;
	};

#endif	// INDEXED_BLENDING
#endif	// !defined(RENDER_POINTS_IN_SCREENSPACE)

	//---	structure for line verts
	enum
	{
		#ifdef RENDER_POINTS_IN_SCREENSPACE
			TRANSVB_FVF		= D3DFVF_XYZRHW | D3DFVF_DIFFUSE,
			TRANSVB_SIZE	= (s32)(16*1024), // extra space needed to hold the skins
		#else
			TRANSVB_FVF		= D3DFVF_XYZ | D3DFVF_DIFFUSE,
			TRANSVB_SIZE	= (s32)(16*1024),
		#endif
	};

	struct STransVert
	{
		f32 x, y, z;

		#ifdef RENDER_POINTS_IN_SCREENSPACE
			f32	rhw;
		#endif

		u32	color;
	};

	//---	structure for line verts
	enum
	{
		VB2D_FVF	= D3DFVF_XYZRHW | D3DFVF_DIFFUSE,
		VB2D_SIZE	= (s32)(256),
	};

	struct SVert2D
	{
		f32 x, y, z;
		f32	rhw;
		u32	color;
	};

	enum EDrawManyState
	{
		INDIVIDUAL	= 0,
		KNOWNCOUNT,
		UNKNOWNCOUNT
	};


//---	Attributes
protected:
//	CDC*					m_pDC ;
	CD3DView*				m_pD3DView;				// The view getting rendered to
    LPDIRECT3D8				m_pD3D;					// The main D3D object
    LPDIRECT3DDEVICE8		m_pd3dDevice;			// The D3D rendering device

	//===	SKIN RENDERING DATA
	SSkinD3D				m_pSkins[MAX_NUM_SKINS];	// array of skin data
	s32						m_NumSkins;					// number of skins registered

	//===	2D DRAWING VERTEX BUFFER
	LPDIRECT3DVERTEXBUFFER8	m_pVB2D;				// 2D vertex drawing buffer
	u32						m_VB2DOffset;			// offset withint the buffer

	//===	GENERAL TRANSFORMED POINTS/LINES RENDERING DATA
	LPDIRECT3DVERTEXBUFFER8	m_pTransVB;				// vertex buffer to hold line verts
	u32						m_TransVBOffset;		// current offset into the line vertex buffer
	STransVert*				m_pLockedTransVerts;	// locked trans vert buffer
	u32						m_nLockedTransVerts;	// number of locked trans vert buffer

	COLORREF				m_PenColor;				// pen color to draw with

	//---	Draw Lines function variables
	EDrawManyState			m_DrawTrianglesState;	// state of the draw triangles functions
	u32						m_DrawTrianglesCount;
	u32						m_DrawTrianglesOffset;

	//---	Draw Lines function variables
	EDrawManyState			m_DrawLinesState;		// state of the draw lines functions
	u32						m_DrawLinesCount;
	u32						m_DrawLinesOffset;

	//---	Draw Pixels function variables
	EDrawManyState			m_DrawPixelsState;		// state of the draw pixels functions
	u32						m_DrawPixelsCount;
	u32						m_DrawPixelsOffset;

protected:
	void SetAndIncVertData( s32* MatrixList, s32 MatrixListLength, u8*& pVertD3D, u8* pVertex, s32 NumWeights );
	virtual void PushMatrix( matrix4 *pMatrix ) ;
	virtual void PopMatrix( ) ;

//---	Operations
public:
	//---	Selecting Drawing Utencils
	virtual	CPen*		SelectObject	( CPen* pPen );
//	virtual	CBrush*		SelectObject	( CBrush* pBrush );
//	virtual CFont*		SelectObject	( CFont* pFont );
//	virtual	CBitmap*	SelectObject	( CBitmap* pBitmap );
//	virtual	int			SelectObject	( CRgn* pRgn );

	void Reset							( void );
	void ReleaseDevices					( void );
	void AquireDevices					( void );

	SVert2D*	Reserve2DVerts			( s32 Count );
	STransVert*	Reserve3DVerts			( s32 Count );

	//---	Draw triangles
	void StartDrawTriangles				( s32 nLiness = 0 );	// if nPixels is left zero, any number can be drawn,but it will be a bit slower.
	void EndDrawTriangles				( void );
	void DrawTriangle					( vector3& rvPos1, vector3& rNormCol1,
										  vector3& rvPos2, vector3& rNormCol2,
										  vector3& rvPos3, vector3& rNormCol3 );

	//---	Draw lines
	void StartDrawLines					( s32 nLiness = 0 );	// if nPixels is left zero, any number can be drawn,but it will be a bit slower.
	void EndDrawLines					( void );
	void DrawLine						( vector3& rvPos1, vector3& rvPos2 );

	//---	Draw points
	void StartDrawPixels				( s32 nPixels = 0 );	// if nPixels is left zero, any number can be drawn,but it will be a bit slower.
	void EndDrawPixels					( void );
	void DrawPixel						( vector3& rvPos );

	//---	Draw Skins functions
	SSkinD3D*	GetSkinD3D				( CSkin* pSkin ); // given a pointer to a skin, get the D3D conversion of the skin data
	void		ClearSkins				( void );
	xbool		SkinToSkinD3D			( CSkin* pSkin, SSkinD3D* pSkinD3D );

	//---	Drawing
	void RENDER_ClipProjLine3d			( vector3  *tp1, vector3  *tp2 ) ;
	void RENDER_ClipProjLine3d			( vector3  *tp ) ;
	void RENDER_ClipProjTriangle3d		( vector3 *tp, vector3 *nc );
	void RENDER_TransClipProjLine3d		( vector3  *p ) ;
	void RENDER_TransClipProjLine3d		( vector3  *tp1, vector3  *tp2 ) ;
	void RENDER_TransClipProjTriangle3d	( vector3  *p, vector3 *nc );
	void RENDER_TransClipProjTriangle3d	( vector3 &v1, vector3& nc1, vector3 &v2, vector3& nc2, vector3 &v3, vector3& nc3 );
	void RENDER_TransClipProjCircle3d	( f32 r ) ;
	void RENDER_GridLines				( f32 x1, f32 x2, f32 dx,
										  f32 y1, f32 y2, f32 dy ) ;
	void RENDER_Grid					( COLORREF ColorMajor, COLORREF ColorMinor ) ;
	void RENDER_Arrow					( f32 Len, COLORREF Color ) ;
	void RENDER_Axes					( f32 Len, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ ) ;
	void RENDER_GrabAxes				( f32 Len, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ ) ;
	void RENDER_Sphere					( f32 Radius, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ ) ;
	void RENDER_Prop					( f32 Size, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ )	;
	void RENDER_Marker					( f32 Len, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ ) ;
	void RENDER_TransProjPointArray		( vector3  *p, int nPoints, COLORREF Color, bool DrawSolid = false ) ;

	void RENDER_FillSolidRect			( s32 X, s32 Y, s32 W, s32 H, COLORREF rColor ) ;

	void RENDER_Character				( CCharacter* pCharacter, matrix4& rTransform, COLORREF Color, u32 Flags ) ;
	void RENDER_Skin					( CSkin* pSkin, CSkeleton* pSkel, matrix4* pMat, s32 MatCount, COLORREF Color );
} ;

#endif // !defined(AFX_RENDERCONTEXTD3D_H__2C859F62_E603_11D1_9507_00207811EE70__INCLUDED_)
