// Camera.h: interface for the CCamera class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAMERA_H__B86D6AA2_DA12_11D1_AEC0_00A024569FF3__INCLUDED_)
#define AFX_CAMERA_H__B86D6AA2_DA12_11D1_AEC0_00A024569FF3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "x_math.h"

class CCamera  
{
public:
	CCamera();
	virtual ~CCamera();

	void	SetRotation( radian Angle ) ;
	void	SetElevation( radian Angle ) ;
	void	SetDistance( f32 Distance ) ;
	void	SetProjectCenter( f32 cx, f32 cy, f32 ox, f32 oy ) ;
	void	SetTarget( vector3d *pTarget ) ;
	void	SetFOV( s32 radians );
	void	SetMatrix( matrix4 *pMatrix );

	radian	GetRotation( ) ;
	radian	GetElevation( ) ;
	f32		GetDistance( ) ;
	void	GetProjectCenter( f32 *cx, f32 *cy, f32 *ox, f32 *oy ) ;
	void	GetTarget( vector3d *pTarget ) ;
	void	GetMatrix( matrix4 *pMatrix );

	s32		GetFOV( );

	void ComputeMatrix() ;

	matrix4		m_Matrix ;				// Camera Matrix

public:
	vector3d	m_Target ;				// Target Point
	radian		m_Rotation ;			// Rotation
	radian		m_Elevation ;			// Elevation
	f32			m_Distance ;			// Distance from Eye to Target

	f32			m_CX ;					// Center X
	f32			m_CY ;					// Center Y
    f32         m_OX ;                  // Offset X
    f32         m_OY ;                  // Offset Y
	f32			m_DX ;					// Distance X
	f32			m_DY ;					// Distance Y
	s32			m_FOV;					// Field of view in degrees
};

#endif // !defined(AFX_CAMERA_H__B86D6AA2_DA12_11D1_AEC0_00A024569FF3__INCLUDED_)
