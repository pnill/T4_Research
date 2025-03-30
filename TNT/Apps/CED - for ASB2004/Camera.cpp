// Camera.cpp: implementation of the CCamera class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CeD.h"
#include "Camera.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCamera::CCamera()
{
	V3_Zero( &m_Target ) ;
	m_Distance = 200.0 ;
	m_Elevation = -DEG_TO_RAD(10.0) ;
	m_Rotation = DEG_TO_RAD(0.0) ;
	m_CX = 0.0 ;
	m_CY = 0.0 ;
    m_OX = 0.0 ;
    m_OY = 0.0 ;
	m_DX = 540.0 ; //400.0 ;
	m_DY = 540.0 ; //400.0 ;
	m_FOV = 60;

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

void CCamera::SetProjectCenter( f32 cx, f32 cy, f32 ox, f32 oy )
{
	m_CX = cx ;
	m_CY = cy ;
	m_OX = ox ;
	m_OY = oy ;
}

void CCamera::SetTarget( vector3d *pTarget )
{
	V3_Copy( &m_Target, pTarget ) ;
}

void CCamera::SetFOV( s32 degrees )
{
	m_FOV = degrees;
}

void CCamera::SetMatrix( matrix4 *pMatrix )
{
	ASSERT(pMatrix);
	m_Matrix = *pMatrix;
}

void CCamera::GetTarget( vector3d *pTarget )
{
	V3_Copy( pTarget, &m_Target ) ;
}

radian CCamera::GetRotation( )
{
	return( m_Rotation ) ;
}

radian CCamera::GetElevation( )
{
	return( m_Elevation ) ;
}

f32 CCamera::GetDistance( )
{
	return( m_Distance ) ;
}

s32 CCamera::GetFOV( )
{
	return( m_FOV );
}

void CCamera::GetMatrix( matrix4 *pMatrix )
{
	ASSERT(pMatrix);
	*pMatrix = m_Matrix;
}

void CCamera::GetProjectCenter( f32 *cx, f32 *cy, f32 *ox, f32 *oy )
{
	if( cx ) *cx = m_CX ;
	if( cy ) *cy = m_CY ;
    if( ox ) *ox = m_OX ;
    if( oy ) *oy = m_OY ;
}

void CCamera::ComputeMatrix()
{
    vector3d v = {-m_Target.X, -m_Target.Y, -m_Target.Z};
    vector3d d = {0,0,m_Distance};

	M4_Identity( &m_Matrix ) ;
	M4_TranslateOn( &m_Matrix, &v ) ;
	M4_RotateYOn( &m_Matrix, m_Rotation ) ;
	M4_RotateXOn( &m_Matrix, m_Elevation ) ;
	M4_TranslateOn( &m_Matrix, &d ) ;
}
