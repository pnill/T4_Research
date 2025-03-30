// Camera.cpp: implementation of the CCamera class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CeD.h"
#include "Camera.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCamera::CCamera()
{
	m_Target.Zero() ;
	m_Distance = 200.0 ;
	m_Elevation = -DEG_TO_RAD(10.0) ;
	m_Rotation = DEG_TO_RAD(0.0) ;
	m_NZ		= NEAR_PLANE;
	m_FZ		= FAR_PLANE;
	SetProjectWindow( 0, 0, 640, 480 );
	SetFOV( 60.0f );

	ComputeMatrix() ;
}

CCamera::~CCamera()
{

}

void CCamera::SetRotation( radian Angle )
{
	m_Rotation = Angle ;
}

void CCamera::SetElevation( radian Angle )
{
	m_Elevation = Angle ;
}

void CCamera::SetDistance( f32 Distance )
{
	m_Distance = Distance ;
}

void CCamera::SetProjectWindow( f32 ox, f32 oy, f32 wx, f32 wy )
{
	m_WX = wx ;			// window width
	m_WY = wy ;			// window height
	m_CX = 0.5f*wx ;	// window center width
	m_CY = 0.5f*wy ;	// window center height
	m_OX = ox ;			// window offset width
	m_OY = oy ;			// window offset height

	ComputeProjMatrix();
}

void CCamera::SetTarget( vector3& rTarget )
{
	m_Target = rTarget;
/*
	vector3 vEye(0.0f, 0.0f, -1.0f);

	vEye.Scale( m_Distance );
	vEye.RotateX( -m_Elevation );
	vEye.RotateY( -m_Rotation );

	SetEyeTarget( vEye, rTarget );
*/
}

void CCamera::SetEye( vector3& rEye )
{
	vector3 vTarget = m_Target;
	SetEyeTarget( rEye, vTarget );
}

void CCamera::SetEyeTarget( vector3& rEye, vector3& rTarget )
{
	//---	target is set directly
	m_Target	= rTarget;

	//---	eye must be calculated and set as rotation, elevation, and distance values
	vector3 v;
	v = rTarget - rEye;

	radian RX, RY ;
	v.ZToVecRot( &RX, &RY );

	m_Distance		= v.Length() ;
	m_Rotation		= -RY ;
	m_Elevation		= -RX ;

	//---	compute the new matrix
	ComputeMatrix( ) ;
}

void CCamera::SetFOV( f32 degrees )
{
	m_FOV	= degrees;
	m_D = ((f32)m_WX/2.0f) / x_tan(DEG_TO_RAD(m_FOV)/2.0f) ;
	ComputeProjMatrix();
/*
	m_Camera.m_DX = ((f32)r.Width()/2.0f) / x_tan(DEG_TO_RAD(m_Camera.m_FOV)/2.0f) ;
#if 0
	m_Camera.m_DY = m_Camera.m_DX; //((f32)r.Width()/2.0f) / x_tan(DEG_TO_RAD(m_Camera.m_FOV)/2.0f) ;
#else
	radian FOVy	= x_atan( (r.Height()/2.0f) / m_Camera.m_DX );
	m_Camera.m_DY = ((f32)r.Height()/2.0f) / x_tan(DEG_TO_RAD(FOVy)/2.0f) ;
#endif
	*/

}

void CCamera::SetMatrix( matrix4 *pMatrix )
{
	ASSERT(pMatrix);
	m_Matrix = *pMatrix;
}

void CCamera::GetTarget( vector3  *pTarget )
{
	*pTarget = m_Target;
}

//=============================================================================
radian CCamera::GetRotation( )
{
	return( m_Rotation ) ;
}

//=============================================================================
radian CCamera::GetElevation( )
{
	return( m_Elevation ) ;
}

//=============================================================================
f32 CCamera::GetDistance( )
{
	return( m_Distance ) ;
}

//=============================================================================
f32 CCamera::GetFOV( )
{
	return( m_FOV );
}

//=============================================================================
void CCamera::GetMatrix( matrix4 *pMatrix )
{
	ASSERT(pMatrix);
	*pMatrix = m_Matrix;
}

//=============================================================================
void CCamera::GetProjectCenter( f32 *cx, f32 *cy, f32 *ox, f32 *oy )
{
	if( cx ) *cx = m_CX ;
	if( cy ) *cy = m_CY ;
    if( ox ) *ox = m_OX ;
    if( oy ) *oy = m_OY ;
}

#define	OLD_SYSTEM_TO_CONVERT_FROM_RHAND_TO_LHAND

//=============================================================================
void CCamera::ComputeMatrix()
{
    vector3	vTarget	= m_Target;
    f32		Dist	= m_Distance;
	radian	Yaw		= m_Rotation;
	radian	Pitch	= m_Elevation;
	matrix4	VMat;
	VMat.Identity();

#ifndef OLD_SYSTEM_TO_CONVERT_FROM_RHAND_TO_LHAND
	Yaw += R_180;
#endif

	VMat.Translate( -vTarget );
	VMat.RotateY( Yaw );
	VMat.RotateX( Pitch );
	VMat.Translate( vector3( 0.0f, 0.0f, Dist ) );

	m_Matrix	= VMat;

	//---	screen coordinates end up, upside down and mirrored. Must compensate
	m_Matrix.Scale( vector3( -1.0f, -1.0f, 1.0f ) );
}

//=============================================================================
matrix4 CCamera::ComputeMatrixD3D()
{
    vector3	vTarget	= m_Target;
    f32		Dist	= m_Distance;
	radian	Yaw		= m_Rotation;
	radian	Pitch	= m_Elevation;
	matrix4	VMat;
	VMat.Identity();

	//---	convert from R-Hand system to L-Hand system
#ifdef OLD_SYSTEM_TO_CONVERT_FROM_RHAND_TO_LHAND
	vTarget.X = -vTarget.X;							// target point X, must be negated
	Yaw = -Yaw;										// rotation goes the other way
	VMat.Scale( vector3( 1.0f, 1.0f, -1.0f ) );		// invert the Z direction
#else
	vTarget.X = -vTarget.X;							// target point X, must be negated
	Yaw = -Yaw;										// rotation goes the other way
	VMat.Scale( vector3( -1.0f, 1.0f, 1.0f ) );		// invert the Z direction
#endif

	VMat.Translate( -vTarget );
	VMat.RotateY( Yaw );
	VMat.RotateX( Pitch );
	VMat.Translate( vector3( 0.0f, 0.0f, Dist ) );

	return VMat;
}


#include <math.h>

//=============================================================================
void CCamera::ComputeProjMatrix( void )
{
	f32	HFOV	= DEG_TO_RAD(m_FOV);
	f32 VFOV	= 2.0f * (f32)atan( (f32)m_WY / (f32)m_WX * (f32)tan( HFOV*0.5f ) );

//	HFOV = -HFOV;
//	VFOV = -VFOV;

	f32 w, h;
	f32 Q;

	w = (f32)(1.0/tan(HFOV*0.5f));  // 1/tan(x) == cot(x)
	h = (f32)(1.0/tan(VFOV*0.5f));   // 1/tan(x) == cot(x)
	Q = m_FZ/(m_FZ - m_NZ);

	matrix4 PMat;
	memset(&PMat, 0, sizeof(matrix4));

	PMat.M[0][0]	= w;
	PMat.M[1][1]	= h;
	PMat.M[2][2]	= Q;
	PMat.M[3][2]	= -Q*m_NZ;
	PMat.M[2][3]	= 1.0f;

	m_ProjMat	= PMat;
}

//=============================================================================
matrix4 CCamera::ComputeProjMatrixD3D( void )
{
	f32	HFOV			= DEG_TO_RAD(m_FOV);
	f32	VFOV			= 2.0f * (f32)atan( (f32)m_WY / (f32)m_WX * (f32)tan( HFOV*0.5f ) );

	f32 w, h;
	f32 Q;

	w = (f32)(1.0/tan(HFOV*0.5f));  // 1/tan(x) == cot(x)
	h = (f32)(1.0/tan(VFOV*0.5f));   // 1/tan(x) == cot(x)
	Q = m_FZ/(m_FZ - m_NZ);

	matrix4 PMat;
	memset(&PMat, 0, sizeof(matrix4));

	PMat.M[0][0]	= w;
	PMat.M[1][1]	= h;
	PMat.M[2][2]	= Q;
	PMat.M[3][2]	= -Q*m_NZ;
	PMat.M[2][3]	= 1.0f;

	m_ProjMatD3D	= PMat;
	return PMat;
}


//===================================================
//
// Project
//
//	function to project points when not using D3D (which does it internally)
//
//===================================================
void CCamera::Project( vector3* Dest, vector3* Src, s32 NVerts )
{
    s32 i;
	f32 Z;

	f32 cx	= m_CX;
	f32 cy	= m_CY;
	f32	ox	= m_OX;
	f32	oy	= m_OY;
	f32 d	= m_D;

    ASSERT(Dest);
    ASSERT(Src);
    ASSERT(NVerts > 0);
    ASSERT(Dest != Src);

    for (i=0; i<NVerts; i++)
    {
		Z = Src[i].Z;

        // Check if Z is in front of 0.01f
        if (Z < 0.01f)
		{
			// Snap SZ to 0.01f if it's too close to zero
			if (Z > -0.01f)
				Z = 0.01f;

			// Flip Z if it is negative
			if (Z < 0.0f) 
				Z = -Z;
		}

#if 0
		Dest[i] = Src[i];
		Dest[i].Z = Z;
		m_ProjMat.Transform( &Dest[i], &Dest[i], 1 );
		Dest[i].X += ox + cx;
		Dest[i].Y += oy + cy;
#else
		Dest[i].X = ox + cx + d * (Src[i].X / Z);
		Dest[i].Y = oy + cy + d * (Src[i].Y / Z);
		Dest[i].Z = Z;
#endif
    }
}


//===================================================
//
// Project
//
//	function to project points when not using D3D (which does it internally)
//
//===================================================
void CCamera::ProjectD3D( vector3* Dest, vector3* Src, s32 NVerts )
{
    s32 i;
	f32 Z;

	f32 cx	= m_CX;
	f32 cy	= m_CY;
	f32	ox	= m_OX;
	f32	oy	= m_OY;
	f32 d	= m_D;

    ASSERT(Dest);
    ASSERT(Src);
    ASSERT(NVerts > 0);
    ASSERT(Dest != Src);

    for (i=0; i<NVerts; i++)
    {
		Z = Src[i].Z;

        // Check if Z is in front of 0.01f
        if (Z < 0.01f)
		{
			// Snap SZ to 0.01f if it's too close to zero
			if (Z > -0.01f)
				Z = 0.01f;

			// Flip Z if it is negative
			if (Z < 0.0f) 
				Z = -Z;
		}

		Dest[i] = Src[i];
		Dest[i].Z = Z;
		m_ProjMatD3D.Transform( &Dest[i], &Dest[i], 1 );
		Dest[i].X += ox + cx;
		Dest[i].Y += oy + cy;
    }
}
