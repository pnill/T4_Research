// RenderContext.h: interface for the CRenderContext class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RENDERCONTEXT_H__2C859F62_E603_11D1_9507_00207811EE70__INCLUDED_)
#define AFX_RENDERCONTEXT_H__2C859F62_E603_11D1_9507_00207811EE70__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "x_math.hpp"
#include "Camera.h"
#include "eNode.h"

#define MATRIX_STACK_SIZE	32
#define NODE_STACK_SIZE		32
//#define	PERSPECTIVE_DISTX	(800)
//#define	PERSPECTIVE_DISTY	(800)


//==========================================================================
class CCharacter;
class CSkin;
class CSkeleton;

//==========================================================================
#define	RFLAG_BONES		0x01
#define	RFLAG_AXES		0x02
#define	RFLAG_POINTS	0x04
#define	RFLAG_WIREFRAME	0x08
#define	RFLAG_FACETS	0x10


//==========================================================================
class CRenderContext  
{
//---	Construction
public:
	CRenderContext( CCamera *pCamera ) ;
	virtual ~CRenderContext();

//---	Attributes
public:

	//---	camera
	CCamera	*m_pCamera ;

	//---	matrix stack
	matrix4	m_MatrixStack[MATRIX_STACK_SIZE] ;
	int		m_MatrixStackIndex ;

	//---	node stack
	CeNode	*m_NodeStack[NODE_STACK_SIZE] ;
	int		m_NodeStackIndex ;

protected:
	//---	writing utensils
	CPen	*m_pCurPen;
//	CBrush	*m_pCurBrush;
//	CFont	*m_pCurFont;
//	CBitmap	*m_pCurBitmap;

//---	Operations
public:
	virtual s32	GetType( void ) = 0;

	//---	Geometry
	virtual void	PushMatrix( matrix4 *pMatrix ) ;
	const matrix4&	GetMatrix( ) ;
	virtual void	PopMatrix( ) ;

	//---	Nodes
	virtual void PushNode( CeNode *pNode ) ;
	virtual void PopNode( ) ;

	//---	Selecting Drawing Utencils
	virtual	CPen*		SelectObject	( CPen* pPen )			{ CPen* pOldPen = m_pCurPen; m_pCurPen = pPen; return pOldPen; }
//	virtual	CBrush*		SelectObject	( CBrush* pBrush )		{ CBrush* pOldBrush = m_pCurBrush; m_pCurBrush = pBrush; return pOldBrush; }
//	virtual CFont*		SelectObject	( CFont* pFont )		{ CFont* pOldFont = m_pCurFont; m_pCurFont = pFont; return pOldFont; }
//	virtual	CBitmap*	SelectObject	( CBitmap* pBitmap )	{ CBitmap* pOldBitmap = m_pCurBitmap; m_pCurBitmap = pBitmap; return pOldBitmap; }
//	virtual	int			SelectObject	( CRgn* pRgn )			{}

	//---	Drawing
	virtual void RENDER_ClipProjLine3d			( vector3  *tp1, vector3  *tp2 )										= 0;
	virtual void RENDER_ClipProjLine3d			( vector3  *tp )														= 0;
	virtual void RENDER_TransClipProjLine3d		( vector3  *p )															= 0;
	virtual void RENDER_TransClipProjLine3d		( vector3  *tp1, vector3  *tp2 )										= 0;
	virtual void RENDER_TransClipProjCircle3d	( f32 r )																= 0;
	virtual void RENDER_TransClipProjTriangle3d	( vector3  *p, vector3 *nc )											= 0;
	virtual void RENDER_TransClipProjTriangle3d	( vector3 &v1, vector3& nc1,
												  vector3 &v2, vector3& nc2,
												  vector3 &v3, vector3& nc3 )											= 0;
	virtual void RENDER_GridLines				( f32 x1, f32 x2, f32 dx,												
										  f32 y1, f32 y2, f32 dy )														= 0;
	virtual void RENDER_Grid					( COLORREF ColorMajor, COLORREF ColorMinor )							= 0;
	virtual void RENDER_Arrow					( f32 Len, COLORREF Color )												= 0;
	virtual void RENDER_Axes					( f32 Len, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ )			= 0;
	virtual void RENDER_GrabAxes				( f32 Len, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ )			= 0;
	virtual void RENDER_Sphere					( f32 Radius, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ )		= 0;
	virtual void RENDER_Prop					( f32 Size, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ )			= 0;
	virtual void RENDER_Marker					( f32 Len, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ )			= 0;
	virtual void RENDER_TransProjPointArray		( vector3  *p, int nPoints, COLORREF Color, bool DrawSolid = false )	= 0;

	virtual void RENDER_FillSolidRect			( s32 X, s32 Y, s32 W, s32 H, COLORREF rColor )							= 0;
	virtual void RENDER_Character				( CCharacter* pCharacter, matrix4& rTransform, COLORREF Color, u32 Flags )					= 0;
	virtual void RENDER_Skin					( CSkin* pSkin, CSkeleton* pSkel, matrix4* pMat, s32 MatCount, COLORREF Color )							= 0;

//---	utility/math functions used to render
protected:
			void V3_ClipLineSegment				( vector3* CV, vector3* V0, vector3* V1,
												  f32 NX, f32 NY, f32 NZ, f32 D );


} ;

#endif // !defined(AFX_RENDERCONTEXT_H__2C859F62_E603_11D1_9507_00207811EE70__INCLUDED_)
