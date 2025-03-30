// RenderContext.h: interface for the CRenderContextCDC class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RENDERCONTEXTCDC_H__2C859F62_E603_11D1_9507_00207811EE70__INCLUDED_)
#define AFX_RENDERCONTEXTCDC_H__2C859F62_E603_11D1_9507_00207811EE70__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "RenderContext.h"

//==========================================================================
class CSkeleton;
class CSkin;

//==========================================================================
class CRenderContextCDC : public CRenderContext
{
//---	Construction
public:
	CRenderContextCDC( CDC *pDC, CCamera *pCamera ) ;
	virtual ~CRenderContextCDC();

//---	Attributes
public:
	enum{ RCTYPE = 1 };

	CDC		*m_pDC ;

//---	Operations
public:
	virtual void PushMatrix( matrix4 *pMatrix ) ;
	virtual void PopMatrix( ) ;

	s32	GetType( void )		{ return RCTYPE; }

	//---	Selecting Drawing Utencils
	virtual	CPen*		SelectObject	( CPen* pPen );
//	virtual	CBrush*		SelectObject	( CBrush* pBrush );
//	virtual CFont*		SelectObject	( CFont* pFont );
//	virtual	CBitmap*	SelectObject	( CBitmap* pBitmap );
//	virtual	int			SelectObject	( CRgn* pRgn );

	//---	Drawing
	void RENDER_ClipProjLine3d			( vector3  *tp1, vector3  *tp2 ) ;
	void RENDER_ClipProjLine3d			( vector3  *tp ) ;
	void RENDER_TransClipProjLine3d		( vector3  *p ) ;
	void RENDER_TransClipProjLine3d		( vector3  *tp1, vector3  *tp2 ) ;
	void RENDER_TransClipProjTriangle3d	( vector3  *p, vector3 *nc );
	void RENDER_TransClipProjTriangle3d	( vector3 &v1, vector3& nc1,
										  vector3 &v2, vector3& nc2,
										  vector3 &v3, vector3& nc3 );
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

#endif // !defined(AFX_RENDERCONTEXTCDC_H__2C859F62_E603_11D1_9507_00207811EE70__INCLUDED_)
