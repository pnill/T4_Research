//=========================================================================
//
//  ORBITCAM.CPP
//
//=========================================================================

#include "OrbitCam.hpp"


//=========================================================================
// FUNCTIONS
//=========================================================================

orbitcam::orbitcam( void )
{
    m_Pitch = 0;
    m_Yaw   = 0;
    m_Target.Zero();
    m_Dist  = 100;       
}

//=========================================================================

orbitcam::orbitcam( f32 NearZ, f32 FarZ )
{
    m_Pitch = 0;
    m_Yaw   = 0;
    m_Target.Zero();
    m_Dist  = 100;       

    SetZLimits( NearZ, FarZ );
}

//=========================================================================

void orbitcam::SetTarget( const vector3& T )  
{ 
    m_Target = T;  
    
    OrbitPoint( T, m_Dist, m_Pitch, m_Yaw );
}

//=========================================================================

void orbitcam::GetTarget( vector3& T )  
{ 
    T = m_Target;       
}

//=========================================================================

void orbitcam::AddDistance( f32 Distance )  
{ 
    f32 NearZ, FarZ;
    
    GetZLimits( NearZ, FarZ );

    m_Dist  += Distance;    
    
    if ( m_Dist < m_ZNear ) m_Dist = m_ZNear;
    if ( m_Dist > m_ZFar  ) m_Dist = m_ZFar;

    OrbitPoint( m_Target, m_Dist, m_Pitch, m_Yaw );
}

//=========================================================================

void orbitcam::AddPitch( radian Pitch )  
{ 
    m_Pitch += Pitch;  
    
    if( m_Pitch >  R_89  )   m_Pitch  =  R_89;
    if( m_Pitch < -R_89  )   m_Pitch  = -R_89;

    OrbitPoint( m_Target, m_Dist, m_Pitch, m_Yaw );
}

//=========================================================================

void orbitcam::AddYaw( radian Yaw )  
{ 
    m_Yaw   += Yaw;     

    OrbitPoint( m_Target, m_Dist, m_Pitch, m_Yaw );
}

//=========================================================================

void orbitcam::SetDistance( f32 Distance )  
{ 
    f32 NearZ, FarZ;
    
    GetZLimits( NearZ, FarZ );

    m_Dist  = Distance;    
    
    if ( m_Dist < m_ZNear ) m_Dist = m_ZNear;
    if ( m_Dist > m_ZFar  ) m_Dist = m_ZFar;

    OrbitPoint( m_Target, m_Dist, m_Pitch, m_Yaw );
}

void orbitcam::GetDistance( f32& Distance )
{
	Distance = m_Dist;
}


//=========================================================================

void orbitcam::SetPitch( radian Pitch )  
{ 
    m_Pitch = Pitch;  
    
    if( m_Pitch >  R_89  )   m_Pitch  =  R_89;
    if( m_Pitch < -R_89  )   m_Pitch  = -R_89;

    OrbitPoint( m_Target, m_Dist, m_Pitch, m_Yaw );
}

void orbitcam::GetPitch( radian& Pitch )
{
	Pitch = m_Pitch;
}

//=========================================================================

void orbitcam::SetYaw( radian Yaw )  
{ 
    m_Yaw   = Yaw;     

    OrbitPoint( m_Target, m_Dist, m_Pitch, m_Yaw );
}

void orbitcam::GetYaw( radian& Yaw )
{
	Yaw = m_Yaw;
}

//=========================================================================

void orbitcam::GetRotations( radian3& Rot )
{
    Rot.Yaw    = m_Yaw;
    Rot.Pitch  = m_Pitch;
    Rot.Roll   = 0;
}

//=========================================================================

void orbitcam::operator=( view& Src  )
{
    vector3     Pos;

    Src.GetPosition( Pos );
    Pos -= m_Target;
    Pos.PitchYaw( m_Pitch, m_Yaw );
    m_Dist = Pos.Length();

    OrbitPoint( m_Target, m_Dist, m_Pitch, m_Yaw );
}
