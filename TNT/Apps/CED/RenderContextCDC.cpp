// RenderContext.cpp: implementation of the CRenderContextCDC class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CeD.h"
#include "RenderContextCDC.h"
#include "Skeleton.h"
#include "Skin.h"
#include "Character.h"
#include "CeDView.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRenderContextCDC::CRenderContextCDC( CDC *pDC, CCamera *pCamera )
	: CRenderContext( pCamera )
{
	m_pDC = pDC ;
}

CRenderContextCDC::~CRenderContextCDC()
{
}

//////////////////////////////////////////////////////////////////////
// Geometry Functions



////////////////////////////////////////////////////////////////////////////
// Selecting Drawing Utencils

CPen* CRenderContextCDC::SelectObject( CPen* pPen )
{
	CRenderContext::SelectObject( pPen );
	return m_pDC->SelectObject( pPen );
}


//////////////////////////////////////////////////////////////////////
// Drawing Functions


//==========================================================================
void CRenderContextCDC::RENDER_ClipProjLine3d( vector3  *tp1, vector3  *tp2 )
{
	vector3 	tp[2] ;
	tp[0] = *tp1;
	tp[1] = *tp2;
//	V3_Copy( &tp[0], tp1 ) ;
//	V3_Copy( &tp[1], tp2 ) ;
	RENDER_ClipProjLine3d( &tp[0] ) ;
}

//==========================================================================
void CRenderContextCDC::RENDER_ClipProjLine3d( vector3  *tp )
{
	vector3 	cp[2] ;
	vector3 	pp[2] ;

	f32			d = m_pCamera->m_D ;
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
	m_pCamera->Project( pp, cp, 2 ) ;

	//---	Draw Line
	m_pDC->MoveTo( (int)(pp[0].X), (int)(pp[0].Y) ) ;
	m_pDC->LineTo( (int)(pp[1].X), (int)(pp[1].Y) ) ;
}

//==========================================================================
void CRenderContextCDC::RENDER_TransClipProjLine3d( vector3  *tp1, vector3  *tp2 )
{
	vector3 	tp[2] ;
	tp[0] = *tp1;
	tp[1] = *tp2;
//	V3_Copy( &tp[0], tp1 ) ;
//	V3_Copy( &tp[1], tp2 ) ;
	RENDER_TransClipProjLine3d( &tp[0] ) ;
}

//==========================================================================
void CRenderContextCDC::RENDER_TransClipProjLine3d( vector3  *p )
{
	vector3 	tp[2] ;
	m_pCamera->m_Matrix.Transform( tp, p, 2 ) ;
//	M4_TransformVerts( &m_pCamera->m_Matrix, tp, p, 2 ) ;

	RENDER_ClipProjLine3d( tp ) ;
}

#define	MIN_CIRCLE_POINTS	4
#define	MAX_CIRCLE_POINTS	32

//==========================================================================
void CRenderContextCDC::RENDER_TransClipProjCircle3d( f32 r )
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
	m_pCamera->m_Matrix.Transform( tp, p, CirclePoints+1 ) ;
//	M4_TransformVerts( &m_pCamera->m_Matrix, tp, p, CirclePoints+1 ) ;

	//---	Draw Line Segments
	for( i = 0 ; i < (CirclePoints) ; i++ )
	{
		RENDER_ClipProjLine3d( &tp[i] ) ;
	}
}

//==========================================================================
void CRenderContextCDC::RENDER_TransClipProjTriangle3d	( vector3  *p, vector3 *nc )
{
}

//==========================================================================
void CRenderContextCDC::RENDER_TransClipProjTriangle3d	( vector3 &v1, vector3& nc1,
									  vector3 &v2, vector3& nc2,
									  vector3 &v3, vector3& nc3 )
{
}

//==========================================================================
void CRenderContextCDC::RENDER_GridLines( f32 x1, f32 x2, f32 dx,
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
void CRenderContextCDC::RENDER_Grid( COLORREF ColorMajor, COLORREF ColorMinor )
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
void CRenderContextCDC::RENDER_Arrow( f32 Len, COLORREF Color )
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
void CRenderContextCDC::RENDER_Axes( f32 Len, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ )
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
void CRenderContextCDC::RENDER_GrabAxes( f32 Len, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ )
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
//	V3_Set( &p[0], -Len, 0, 0 ) ;
//	V3_Set( &p[1],  Len, 0, 0 ) ;
//	V3_Set( &p[2], 0, -Len, 0 ) ;
//	V3_Set( &p[3], 0,  Len, 0 ) ;
//	V3_Set( &p[4], 0, 0, -Len ) ;
//	V3_Set( &p[5], 0, 0,  Len ) ;

	//---	Transform all Points
	m_pCamera->m_Matrix.Transform( tp, p, 6 ) ;
//	M4_TransformVerts( &m_pCamera->m_Matrix, tp, p, 6 ) ;

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
void CRenderContextCDC::RENDER_Sphere( f32 Radius, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ )
{
	matrix4		m, m2 ;

	m.Identity();

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
void CRenderContextCDC::RENDER_Prop( f32 Size, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ )
{
//	matrix4		m, m2 ;
//	m.Identity();
//	m = GetMatrix();

	CPen PenX( PS_SOLID, 1, ColorX );
	CPen PenY( PS_SOLID, 1, ColorY );
	CPen PenZ( PS_SOLID, 1, ColorZ );

	vector3 Points[7];
	Points[0].Set(  0.0f,  0.0f,  0.0f );
	Points[1].Set(  0.0f,  0.0f,  Size );
	Points[2].Set(  0.0f,  0.0f, -Size );
	Points[3].Set(  Size*0.5f,  0.0f,  0.0f );
	Points[4].Set( -Size*0.5f,  0.0f,  0.0f );
	Points[5].Set(  0.0f,  Size*0.5f,  0.0f );
	Points[6].Set(  0.0f, -Size*0.5f,  0.0f );

//	PushMatrix( &m );

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

//	PopMatrix();

	SelectObject( pOldPen );
}

//==========================================================================
void CRenderContextCDC::RENDER_Marker( f32 Len, COLORREF ColorX, COLORREF ColorY, COLORREF ColorZ )
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
//	V3_Set( &p[0], 0, 0,  Len ) ;
//	V3_Set( &p[1],  Len, 0, 0 ) ;
//	V3_Set( &p[2], 0, 0, -Len ) ;
//	V3_Set( &p[3], -Len, 0, 0 ) ;
//	V3_Set( &p[4], 0, -Len, 0 ) ;
//	V3_Set( &p[5], 0,  Len, 0 ) ;

	//---	Transform all Points
	m_pCamera->m_Matrix.Transform( tp, p, 6 ) ;
//	M4_TransformVerts( &m_pCamera->m_Matrix, tp, p, 6 ) ;

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
void CRenderContextCDC::RENDER_TransProjPointArray( vector3  *p, int nPoints, COLORREF Color, bool DrawSolid )
{
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
				m_pCamera->Project( &pp[0], &tp[0], nPoints ) ;

				//---	Plot Points
				for( int i = 0 ; i < nPoints ; i++ )
				{
					if( (tp[i].Z > NEAR_PLANE) &&
						(pp[i].X > -16384) &&
						(pp[i].X <  16384) &&
						(pp[i].Y > -16384) &&
						(pp[i].Y <  16384) )
					{
						m_pDC->SetPixel( (int)(pp[i].X), (int)(pp[i].Y), Color ) ;
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
			x_free( pp ) ;
		}

		//---	Free Transformed Points
		x_free( tp ) ;
	}
}

//==========================================================================
void CRenderContextCDC::RENDER_FillSolidRect( s32 X, s32 Y, s32 W, s32 H, COLORREF rColor )
{
	m_pDC->FillSolidRect( X, Y, W, H, rColor );
}

//==========================================================================
void CRenderContextCDC::RENDER_Character( CCharacter* pCharacter, matrix4& rTransform, COLORREF Color, u32 Flags )
{
	matrix4		m[100] ;
	vector3 	p[100] ;
	CSkeleton*	pSkel = pCharacter->GetSkeleton();
	CSkin*		pSkin = pCharacter->GetCurSkin();

	ASSERT( pSkel->GetNumBones() < 100 ) ;

	for( int i = 0 ; i < pSkel->GetNumBones() ; i++ )
	{
		int ID = pSkel->GetBoneFromIndex(i)->BoneID ;
		m[ID] = *pSkel->GetPosedBoneMatrix(ID);
		rTransform.Transform( m[ID] );
		p[ID] = m[ID].GetTranslation();
//		M4_GetTranslation( GetPosedBoneMatrix(ID), &p[ID] ) ;
	}

	//---	Set Pen to Draw in
	CPen PenBone( PS_SOLID, 1, Color ) ;
	CPen *pOldPen = SelectObject( &PenBone ) ;

	//---	Loop through each Bone
	for( i = 0 ; i < pSkel->GetNumBones() ; i++ )
	{
		//---	Draw Bone Axis if required
		if( Flags&RFLAG_AXES )
		{
			matrix4	m ;
			m = *pSkel->GetPosedBoneMatrix( i );
//			M4_Copy( &m, GetPosedBoneMatrix( i ) ) ;
			PushMatrix( &m ) ;
			RENDER_Axes( 2, RGB(255*MOD_COLOR,0,0), RGB(0,255*MOD_COLOR,0), RGB(0,0,255*MOD_COLOR) ) ;
			PopMatrix( ) ;
		}

		//---	Draw Bone to Parent
		if( pSkel->GetBoneFromIndex(i)->ParentID != -1 )
		{
			RENDER_TransClipProjLine3d( &p[pSkel->GetBoneFromIndex(i)->ParentID], &p[pSkel->GetBoneFromIndex(i)->BoneID] ) ;
		}
	}

	//---	Restore Pen
	SelectObject( pOldPen ) ;
}

//==========================================================================
void CRenderContextCDC::RENDER_Skin( CSkin* pSkin, CSkeleton* pSkel, matrix4* pMat, s32 MatCount, COLORREF Color )
{
}


//==========================================================================
void CRenderContextCDC::PushMatrix( matrix4 *pMatrix )
{
#if 0
	CRenderContext::PushMatrix( pMatrix );
	m_pCamera->m_Matrix = GetMatrix();
#elif 0
	CRenderContext::PushMatrix( &m_pCamera->m_Matrix );
	m_pCamera->m_Matrix.PreTransform( GetMatrix() );
#else
	ASSERT( m_MatrixStackIndex < MATRIX_STACK_SIZE ) ;

	m_MatrixStack[m_MatrixStackIndex] = m_pCamera->m_Matrix;
	m_pCamera->m_Matrix.PreTransform( *pMatrix );

	m_MatrixStackIndex++ ;
#endif
}

//==========================================================================
void CRenderContextCDC::PopMatrix( )
{
#if 0
	CRenderContext::PopMatrix( );
	m_pCamera->m_Matrix = GetMatrix();
#elif 0
	m_pCamera->m_Matrix = GetMatrix();
	CRenderContext::PopMatrix( );
#else
	m_MatrixStackIndex-- ;
	ASSERT( m_MatrixStackIndex >= 0 ) ;

	m_pCamera->m_Matrix = m_MatrixStack[m_MatrixStackIndex];
#endif
}
