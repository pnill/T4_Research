// RenderContext.h: interface for the CRenderContext class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RENDERCONTEXT_H__2C859F62_E603_11D1_9507_00207811EE70__INCLUDED_)
#define AFX_RENDERCONTEXT_H__2C859F62_E603_11D1_9507_00207811EE70__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "x_math.h"
#include "Camera.h"
#include "eNode.h"

#define MATRIX_STACK_SIZE	32
#define NODE_STACK_SIZE		32
//#define	PERSPECTIVE_DISTX	(800)
//#define	PERSPECTIVE_DISTY	(800)

class CRenderContext  
{
//---	Construction
public:
	CRenderContext( CDC *pDC, CCamera *pCamera ) ;
	virtual ~CRenderContext();

//---	Attributes
public:
	CDC		*m_pDC ;
	CCamera	*m_pCamera ;
	matrix4	m_MatrixStack[MATRIX_STACK_SIZE] ;
	CeNode	*m_NodeStack[NODE_STACK_SIZE] ;
	int		m_MatrixStackIndex ;
	int		m_NodeStackIndex ;

//---	Operations
public:
	//---	Geometry
	void PushMatrix( matrix4 *pMatrix ) ;
	void PopMatrix( ) ;

	//---	Nodes
	void PushNode( CeNode *pNode ) ;
	void PopNode( ) ;

	//---	Drawing
	void RENDER_ClipProjLine3d			( vector3d *tp1, vector3d *tp2 ) ;
	void RENDER_ClipProjLine3d			( vector3d *tp ) ;
	void RENDER_TransClipProjLine3d		( vector3d *p ) ;
	void RENDER_TransClipProjLine3d		( vector3d *tp1, vector3d *tp2 ) ;
	void RENDER_TransClipProjCircle3d	( f32 r ) ;
	void RENDER_GridLines				( f32 x1, f32 x2, f32 dx,
										  f32 y1, f32 y2, f32 dy ) ;
	void RENDER_Grid					( COLORREF ColorMajor, COLORREF ColorMinor ) ;
	void RENDER_Arrow					( f32 Len, COLORREF Color ) ;
	void RENDER_Axes					( f32 Len, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ ) ;
	void RENDER_GrabAxes				( f32 Len, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ ) ;
	void RENDER_Sphere					( f32 Radius, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ ) ;
	void RENDER_Marker					( f32 Len, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ ) ;
	void RENDER_TransProjPointArray		( vector3d *p, int nPoints, COLORREF Color, bool DrawSolid = false ) ;
} ;

#endif // !defined(AFX_RENDERCONTEXT_H__2C859F62_E603_11D1_9507_00207811EE70__INCLUDED_)
