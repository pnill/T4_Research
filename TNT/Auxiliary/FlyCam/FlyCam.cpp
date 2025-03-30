//=========================================================================
//
//  FLYCAM.CPP
//
//=========================================================================

#include "FlyCam.hpp"


//=========================================================================
// FUNCTIONS
//=========================================================================

flycam::flycam( void )
{
    m_Pitch = 0;
    m_Yaw   = 0;
    m_Roll  = 0;
    m_Pos.Zero();
}

//=========================================================================

flycam::flycam( f32 NearZ, f32 FarZ )
{
    m_Pitch = 0;
    m_Yaw   = 0;
    m_Roll  = 0;

    m_Pos.Zero();

    SetZLimits( NearZ, FarZ );
}

//=========================================================================

void flycam::SetPosition( const vector3& Pos )
{
    m_Pos = Pos;
    view::SetPosition( V_WORLD, m_Pos );
}

void flycam::GetPosition( vector3& Pos )
{
    Pos = m_Pos;
}

//=========================================================================

void flycam::LookAtPoint( const vector3& T )  
{  
    //  Tell the view to look at the point
    HorizLookAt( V_WORLD, T );

    //  Pull the rotations back out and plug them back into the camera
    GetPitchYaw( m_Pitch, m_Yaw );
}

//=========================================================================

void flycam::AddPitch( radian Pitch )  
{ 
    m_Pitch += Pitch;  
    
    if( m_Pitch >  R_89  )   m_Pitch  =  R_89;
    if( m_Pitch < -R_89  )   m_Pitch  = -R_89;

    view::SetRotations( V_WORLD, m_Pitch, m_Yaw, m_Roll );
}

//=========================================================================

void flycam::AddYaw( radian Yaw )  
{ 
    m_Yaw   += Yaw;     

    view::SetRotations( V_WORLD, m_Pitch, m_Yaw, m_Roll );
}


//=========================================================================

void flycam::AddRoll( radian Roll )  
{ 
    m_Roll   += Roll;     

    view::SetRotations( V_WORLD, m_Pitch, m_Yaw, m_Roll );
}


//=========================================================================

void flycam::SetPitch( radian Pitch )  
{ 
    m_Pitch = Pitch;  
    
    if( m_Pitch >  R_89  )   m_Pitch  =  R_89;
    if( m_Pitch < -R_89  )   m_Pitch  = -R_89;

    view::SetRotations( V_WORLD, m_Pitch, m_Yaw, m_Roll );
}

void flycam::GetPitch( radian& Pitch )  
{ 
    Pitch = m_Pitch;  
}

//=========================================================================

void flycam::SetYaw( radian Yaw )  
{ 
    m_Yaw   = Yaw;     

    view::SetRotations( V_WORLD, m_Pitch, m_Yaw, m_Roll );
}

void flycam::GetYaw( radian& Yaw )  
{ 
    Yaw   = m_Yaw;     
}

//=========================================================================

void flycam::SetRoll( radian Roll )  
{ 
    m_Roll  = Roll;

    view::SetRotations( V_WORLD, m_Pitch, m_Yaw, m_Roll );
}

void flycam::GetRoll( radian& Roll )  
{ 
    Roll  = m_Roll;
}

//=========================================================================

void flycam::SetRotations( const radian3& Rot )
{
    m_Yaw       = Rot.Yaw;
    m_Pitch     = Rot.Pitch;
    m_Roll      = Rot.Roll;

    view::SetRotations( V_WORLD, m_Pitch, m_Yaw, m_Roll );
}

//=========================================================================

void flycam::GetRotations( radian3& Rot )
{
    Rot.Yaw    = m_Yaw;
    Rot.Pitch  = m_Pitch;
    Rot.Roll   = m_Roll;
}


//=========================================================================

void flycam::MoveForward( f32 Amt )
{
    vector3     Temp;

    Temp.Set(0,0,Amt);
    Temp.RotateY( m_Yaw );
    
    m_Pos += Temp;

    view::SetPosition( V_WORLD, m_Pos );
}


//=========================================================================

void flycam::MoveRight( f32 Amt )
{
    vector3     Temp;

    Temp.Set(0,0,Amt);
    Temp.RotateY( m_Yaw - R_90 );
    m_Pos += Temp;

    view::SetPosition( V_WORLD, m_Pos );
}


//=========================================================================

void flycam::MoveUp( f32 Amt )
{
    m_Pos += vector3(0,Amt,0);

    view::SetPosition( V_WORLD, m_Pos );
}

//=========================================================================

void flycam::operator=( view& Src  )
{
    Src.GetPitchYaw( m_Pitch, m_Yaw );
    m_Roll = 0;
    Src.GetPosition( m_Pos );

    view::SetPosition( V_WORLD, m_Pos );
    view::SetRotations( V_WORLD, m_Pitch, m_Yaw, m_Roll );
}