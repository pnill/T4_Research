// Camera.h: interface for the CCamera class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAMERA_H__B86D6AA2_DA12_11D1_AEC0_00A024569FF3__INCLUDED_)
#define AFX_CAMERA_H__B86D6AA2_DA12_11D1_AEC0_00A024569FF3__INCLUDED_

#define	NEAR_PLANE	10.0f
#define	FAR_PLANE	10000.0f


#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "x_math.hpp"

class CCamera  
{
public:
	CCamera();
	virtual ~CCamera();

	void	SetRotation( radian Angle ) ;
	void	SetElevation( radian Angle ) ;
	void	SetDistance( f32 Distance ) ;
	void	SetProjectWindow( f32 cx, f32 cy, f32 ox, f32 oy ) ;
	void	SetTarget( vector3& rTarget ) ;
	void	SetEye( vector3& rEye ) ;
	void	SetEyeTarget( vector3& rEye, vector3& rTarget );
	void	SetFOV( f32 radians ) ;
	void	SetMatrix( matrix4 *pMatrix );

	radian	GetRotation( ) ;
	radian	GetElevation( ) ;
	f32		GetDistance( ) ;
	void	GetProjectCenter( f32 *cx, f32 *cy, f32 *ox, f32 *oy ) ;
	void	GetTarget( vector3  *pTarget ) ;
	void	GetMatrix( matrix4 *pMatrix );

	f32		GetFOV( );

	void	ComputeMatrix() ;
	matrix4	ComputeMatrixD3D() ;

	void	ComputeProjMatrix() ;
	matrix4	ComputeProjMatrixD3D();

	//---	function to project points when not using D3D (which does it internally)
	void	Project		( vector3* Dest, vector3* Src, s32 NVerts );
	void	ProjectD3D	( vector3* Dest, vector3* Src, s32 NVerts );


public:
	matrix4		m_ProjMat ;				// Projection matrix
	matrix4		m_ProjMatD3D ;			// projection matrix for D3D

	matrix4		m_Matrix ;				// Camera Matrix

	vector3 	m_Target ;				// Target Point
	radian		m_Rotation ;			// Rotation
	radian		m_Elevation ;			// Elevation
	f32			m_Distance ;			// Distance from Eye to Target

	f32			m_WX ;					// Window Width
	f32			m_WY ;					// Window Height
	f32			m_CX ;					// Center X
	f32			m_CY ;					// Center Y
    f32         m_OX ;                  // Offset X
    f32         m_OY ;                  // Offset Y
	f32			m_D ;					// Distance from the user's eye point to the screen
//	f32			m_DX ;					// Distance X
//	f32			m_DY ;					// Distance Y
	f32			m_FOV ;					// Field of view in degrees

	f32			m_NZ ;					// near clip plane
	f32			m_FZ ;					// far clip plane
};

#endif // !defined(AFX_CAMERA_H__B86D6AA2_DA12_11D1_AEC0_00A024569FF3__INCLUDED_)
