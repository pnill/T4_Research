// RenderContext.cpp: implementation of the CRenderContext class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CeD.h"
#include "RenderContext.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRenderContext::CRenderContext( CDC *pDC, CCamera *pCamera )
{
	m_pDC = pDC ;
	m_pCamera = pCamera ;
	m_MatrixStackIndex = 0 ;
	m_NodeStackIndex = 0 ;
}

CRenderContext::~CRenderContext()
{
}

//////////////////////////////////////////////////////////////////////
// Geometry Functions

void CRenderContext::PushMatrix( matrix4 *pMatrix )
{
	ASSERT( m_MatrixStackIndex < MATRIX_STACK_SIZE ) ;

	M4_Copy( &m_MatrixStack[m_MatrixStackIndex], &m_pCamera->m_Matrix ) ;
	M4_PostMultOn( &m_pCamera->m_Matrix, pMatrix ) ;

	m_MatrixStackIndex++ ;
}

void CRenderContext::PopMatrix( )
{
	m_MatrixStackIndex-- ;
	ASSERT( m_MatrixStackIndex >= 0 ) ;

	M4_Copy( &m_pCamera->m_Matrix, &m_MatrixStack[m_MatrixStackIndex] ) ;
}

//////////////////////////////////////////////////////////////////////
// Node Functions

void CRenderContext::PushNode( CeNode *pNode )
{
	ASSERT( m_NodeStackIndex < NODE_STACK_SIZE ) ;
	m_NodeStack[m_NodeStackIndex] = pNode ;
	m_NodeStackIndex++ ;
}

void CRenderContext::PopNode( )
{
	m_NodeStackIndex-- ;
	ASSERT( m_NodeStackIndex >= 0 ) ;
}


//////////////////////////////////////////////////////////////////////
// Drawing Functions

#define	NEAR_PLANE	8

void CRenderContext::RENDER_ClipProjLine3d( vector3d *tp1, vector3d *tp2 )
{
	vector3d	tp[2] ;
	V3_Copy( &tp[0], tp1 ) ;
	V3_Copy( &tp[1], tp2 ) ;
	RENDER_ClipProjLine3d( &tp[0] ) ;
}

void CRenderContext::RENDER_ClipProjLine3d( vector3d *tp )
{
	vector3d	cp[2] ;
	vector3d	pp[2] ;

	f32			d = m_pCamera->m_DX ;
	f32			cx, cy ;
    f32         ox, oy ;
	m_pCamera->GetProjectCenter( &cx, &cy, &ox, &oy ) ;

	//---	Get Clip Codes
	bool c0 = (tp[0].Z < NEAR_PLANE) ;
	bool c1 = (tp[1].Z < NEAR_PLANE) ;

	//---	If both points clipped then exit
	if( c0 && c1 )
		return ;

	//---	Check for clipped
	if( c0 || c1 )
	{
		vector3d	CV ;
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
		vector3d	CV ;
		vector3d	n ;
		V3_Set( &n, -d, 0, cx ) ;
		V3_Normalize( &n ) ;
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
		vector3d	CV ;
		vector3d	n ;
		V3_Set( &n, d, 0, cx ) ;
		V3_Normalize( &n ) ;
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
		vector3d	CV ;
		vector3d	n ;
		V3_Set( &n, 0, -d, cy ) ;
		V3_Normalize( &n ) ;
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
		vector3d	CV ;
		vector3d	n ;
		V3_Set( &n, 0, d, cy ) ;
		V3_Normalize( &n ) ;
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
	V3_Project( pp, cp, 2, cx, -d, cy, -d ) ;

	//---	Draw Line
	m_pDC->MoveTo( (int)(ox+pp[0].X), (int)(oy+pp[0].Y) ) ;
	m_pDC->LineTo( (int)(ox+pp[1].X), (int)(oy+pp[1].Y) ) ;
}

void CRenderContext::RENDER_TransClipProjLine3d( vector3d *tp1, vector3d *tp2 )
{
	vector3d	tp[2] ;
	V3_Copy( &tp[0], tp1 ) ;
	V3_Copy( &tp[1], tp2 ) ;
	RENDER_TransClipProjLine3d( &tp[0] ) ;
}

void CRenderContext::RENDER_TransClipProjLine3d( vector3d *p )
{
	vector3d	tp[2] ;
	M4_TransformVerts( &m_pCamera->m_Matrix, tp, p, 2 ) ;

	RENDER_ClipProjLine3d( tp ) ;
}

#define	MIN_CIRCLE_POINTS	4
#define	MAX_CIRCLE_POINTS	32

void CRenderContext::RENDER_TransClipProjCircle3d( f32 r )
{
	vector3d	p[MAX_CIRCLE_POINTS+1] ;
	vector3d	tp[MAX_CIRCLE_POINTS+1] ;

	//---	Generate Circle
	int CirclePoints = (int)((MAX_CIRCLE_POINTS-MIN_CIRCLE_POINTS)*x_fabs(r)/20.0) + MIN_CIRCLE_POINTS ;
	if( CirclePoints > MAX_CIRCLE_POINTS ) CirclePoints = MAX_CIRCLE_POINTS ;
	for( int i = 0 ; i < (CirclePoints+1) ; i++ )
	{
		f32	a = DEG_TO_RAD(360.0) * (f32)i / (CirclePoints) ;
		p[i].X = x_cos( a ) * r ;
		p[i].Y = 0 ;
		p[i].Z = x_sin( a ) * r ;
	}

	//---	Transform all Points
	M4_TransformVerts( &m_pCamera->m_Matrix, tp, p, CirclePoints+1 ) ;

	//---	Draw Line Segments
	for( i = 0 ; i < (CirclePoints) ; i++ )
	{
		RENDER_ClipProjLine3d( &tp[i] ) ;
	}
}



void CRenderContext::RENDER_GridLines( f32 x1, f32 x2, f32 dx,
								 f32 z1, f32 z2, f32 dz )
{
	for( f32 x = x1 ; x <= x2 ; x += dx )
	{
		vector3d	p[2] ;
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
		vector3d	p[2] ;
		p[0].X = x1 ;
		p[0].Y = 0 ;
		p[0].Z = z ;
		p[1].X = x2 ;
		p[1].Y = 0 ;
		p[1].Z = z ;
		RENDER_TransClipProjLine3d( p ) ;
	}
}

void CRenderContext::RENDER_Grid( COLORREF ColorMajor, COLORREF ColorMinor )
{
	CPen PenMajor( PS_SOLID, 1, ColorMajor ) ;
	CPen PenMinor( PS_SOLID, 1, ColorMinor ) ;
	CPen *pOldPen = m_pDC->SelectObject( &PenMinor ) ;
	RENDER_GridLines( -400, 400, 10, -400, 400, 10 ) ;
	m_pDC->SelectObject( &PenMajor ) ;
	RENDER_GridLines( -400, 400, 100, -400, 400, 100 ) ;
	m_pDC->SelectObject( pOldPen ) ;
}

void CRenderContext::RENDER_Arrow( f32 Len, COLORREF Color )
{
	CPen Pen( PS_SOLID, 1, Color ) ;
	CPen *pOldPen = m_pDC->SelectObject( &Pen ) ;

	//---	Render Arrow Lines
	vector3d	p[2] ;
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

	m_pDC->SelectObject( pOldPen ) ;
}

void CRenderContext::RENDER_Axes( f32 Len, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ )
{
	vector3d	p[2] ;
	V3_Set( &p[0], 0, 0, 0 ) ;

	CPen PenX( PS_SOLID, 1, ColorX ) ;
	CPen PenY( PS_SOLID, 1, ColorY ) ;
	CPen PenZ( PS_SOLID, 1, ColorZ ) ;
	CPen *pOldPen = m_pDC->SelectObject( &PenX ) ;
	p[1].X = Len ;
	p[1].Y = 0 ;
	p[1].Z = 0 ;
	RENDER_TransClipProjLine3d( p ) ;
	m_pDC->SelectObject( &PenY ) ;
	p[1].X = 0 ;
	p[1].Y = Len ;
	p[1].Z = 0 ;
	RENDER_TransClipProjLine3d( p ) ;
	m_pDC->SelectObject( &PenZ ) ;
	p[1].X = 0 ;
	p[1].Y = 0 ;
	p[1].Z = Len ;
	RENDER_TransClipProjLine3d( p ) ;

	m_pDC->SelectObject( pOldPen ) ;
}

void CRenderContext::RENDER_GrabAxes( f32 Len, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ )
{
	vector3d	p[6], tp[6] ;

	CPen PenX( PS_SOLID, 1, ColorX ) ;
	CPen PenY( PS_SOLID, 1, ColorY ) ;
	CPen PenZ( PS_SOLID, 1, ColorZ ) ;

	V3_Set( &p[0], -Len, 0, 0 ) ;
	V3_Set( &p[1],  Len, 0, 0 ) ;
	V3_Set( &p[2], 0, -Len, 0 ) ;
	V3_Set( &p[3], 0,  Len, 0 ) ;
	V3_Set( &p[4], 0, 0, -Len ) ;
	V3_Set( &p[5], 0, 0,  Len ) ;

	//---	Transform all Points
	M4_TransformVerts( &m_pCamera->m_Matrix, tp, p, 6 ) ;

	//---	Draw Line Segments
	CPen *pOldPen = m_pDC->SelectObject( &PenX ) ;
	RENDER_ClipProjLine3d( &tp[0] ) ;
	m_pDC->SelectObject( &PenY ) ;
	RENDER_ClipProjLine3d( &tp[2] ) ;
	m_pDC->SelectObject( &PenZ ) ;
	RENDER_ClipProjLine3d( &tp[4] ) ;

	m_pDC->SelectObject( pOldPen ) ;
}

void CRenderContext::RENDER_Sphere( f32 Radius, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ )
{
	matrix4		m, m2 ;

	M4_Identity( &m ) ;

	CPen PenX( PS_SOLID, 1, ColorX ) ;
	CPen PenY( PS_SOLID, 1, ColorY ) ;
	CPen PenZ( PS_SOLID, 1, ColorZ ) ;
	CPen *pOldPen = m_pDC->SelectObject( &PenY ) ;
	PushMatrix( &m ) ;
	RENDER_TransClipProjCircle3d( Radius ) ;
	PopMatrix( ) ;

	M4_Identity( &m2 ) ;
	M4_RotateXOn( &m2, DEG_TO_RAD(90) ) ;
	M4_PostMultOn( &m, &m2 ) ;
	m_pDC->SelectObject( &PenZ ) ;
	PushMatrix( &m ) ;
	RENDER_TransClipProjCircle3d( Radius ) ;
	PopMatrix( ) ;

	M4_Identity( &m2 ) ;
	M4_RotateZOn( &m2, DEG_TO_RAD(90) ) ;
	M4_PostMultOn( &m, &m2 ) ;
	m_pDC->SelectObject( &PenX ) ;
	PushMatrix( &m ) ;
	RENDER_TransClipProjCircle3d( Radius ) ;
	PopMatrix( ) ;

	m_pDC->SelectObject( pOldPen ) ;
}

void CRenderContext::RENDER_Marker( f32 Len, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ )
{
	vector3d	p[6], tp[6] ;

	CPen PenX( PS_SOLID, 1, ColorX ) ;
	CPen PenY( PS_SOLID, 1, ColorY ) ;
	CPen PenZ( PS_SOLID, 1, ColorZ ) ;

	V3_Set( &p[0], 0, 0,  Len ) ;
	V3_Set( &p[1],  Len, 0, 0 ) ;
	V3_Set( &p[2], 0, 0, -Len ) ;
	V3_Set( &p[3], -Len, 0, 0 ) ;
	V3_Set( &p[4], 0, -Len, 0 ) ;
	V3_Set( &p[5], 0,  Len, 0 ) ;

	//---	Transform all Points
	M4_TransformVerts( &m_pCamera->m_Matrix, tp, p, 6 ) ;

	//---	Draw Line Segments
	CPen *pOldPen = m_pDC->SelectObject( &PenY ) ;
	RENDER_ClipProjLine3d( &tp[0], &tp[1] ) ;
	RENDER_ClipProjLine3d( &tp[1], &tp[2] ) ;
	RENDER_ClipProjLine3d( &tp[2], &tp[3] ) ;
	RENDER_ClipProjLine3d( &tp[3], &tp[0] ) ;
	m_pDC->SelectObject( &PenX ) ;
	RENDER_ClipProjLine3d( &tp[0], &tp[4] ) ;
	RENDER_ClipProjLine3d( &tp[4], &tp[2] ) ;
	RENDER_ClipProjLine3d( &tp[2], &tp[5] ) ;
	RENDER_ClipProjLine3d( &tp[5], &tp[0] ) ;
	m_pDC->SelectObject( &PenZ ) ;
	RENDER_ClipProjLine3d( &tp[3], &tp[4] ) ;
	RENDER_ClipProjLine3d( &tp[4], &tp[1] ) ;
	RENDER_ClipProjLine3d( &tp[1], &tp[5] ) ;
	RENDER_ClipProjLine3d( &tp[5], &tp[3] ) ;
	m_pDC->SelectObject( pOldPen ) ;
}

void CRenderContext::RENDER_TransProjPointArray( vector3d *p, int nPoints, COLORREF Color, bool DrawSolid )
{
	vector3d	*tp ;
	vector3d	*pp ;

	tp = (vector3d*)malloc( nPoints * sizeof(vector3d) ) ;
	if( tp )
	{
		pp = (vector3d*)malloc( nPoints * sizeof(vector3d) ) ;
		if( pp )
		{
			//---	Transform and Project
			f32	cx, cy ;
            f32 ox, oy ;
			m_pCamera->GetProjectCenter( &cx, &cy, &ox, &oy ) ;
			M4_TransformVerts( &m_pCamera->m_Matrix, &tp[0], p, nPoints ) ;

			if( !DrawSolid || (nPoints <= 1) )
			{
				f32 d = m_pCamera->m_DX ;
				V3_Project( &pp[0], &tp[0], nPoints, cx, -d, cy, -d ) ;

				//---	Plot Points
				for( int i = 0 ; i < nPoints ; i++ )
				{
					if( (tp[i].Z > NEAR_PLANE) &&
						(pp[i].X > -16384) &&
						(pp[i].X <  16384) &&
						(pp[i].Y > -16384) &&
						(pp[i].Y <  16384) )
					{
						m_pDC->SetPixel( (int)(ox+pp[i].X), (int)(oy+pp[i].Y), Color ) ;
					}
				}
			}
			else
			{
				for( int i = 1 ; i < nPoints ; i++ )
				{
					RENDER_ClipProjLine3d( &tp[i-1], &tp[i] ) ;
				}
			}

			//---	Free Projected Points
			free( pp ) ;
		}

		//---	Free Transformed Points
		free( tp ) ;
	}
}
