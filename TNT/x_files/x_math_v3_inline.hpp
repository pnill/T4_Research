////////////////////////////////////////////////////////////////////////////
//
//  X_MATH_V3_INLINE.HPP
//
////////////////////////////////////////////////////////////////////////////

#ifndef X_MATH_V3_INLINE_HPP
#define X_MATH_V3_INLINE_HPP
#else
#error "File " __FILE__ " has been included twice!"
#endif

////////////////////////////////////////////////////////////////////////////
//  INCLUDES
////////////////////////////////////////////////////////////////////////////

#ifndef X_DEBUG_HPP
#include "x_debug.hpp"
#endif

////////////////////////////////////////////////////////////////////////////
//  Functions which do nothing, but are declared for readability.
////////////////////////////////////////////////////////////////////////////

inline vector3:: vector3( void )        {  }
//inline vector3::~vector3( void )        {  }

////////////////////////////////////////////////////////////////////////////
//  Functions which actually have some work to do.
////////////////////////////////////////////////////////////////////////////

//==========================================================================

inline vector3::vector3( const vector3& V )
{
    X = V.X;   Y = V.Y;   Z = V.Z;
}

//==========================================================================

inline vector3::vector3( const vector2& V )
{
    X = V.X;   Y = V.Y;   Z = 0.0f;
}

//==========================================================================

inline vector3::vector3( const radian3& R )
{
    X = x_cos( R.Yaw );     
    Y = x_sin( R.Pitch );
    Z = x_sin( R.Yaw   );
}

//==========================================================================

inline vector3::vector3( f32 aX, f32 aY, f32 aZ )
{
    X = aX;   Y = aY;   Z = aZ;
}

//==========================================================================

inline void vector3::Zero( void )
{
    X = Y = Z = 0.0f;
}

//==========================================================================

inline void vector3::Set( f32 aX, f32 aY, f32 aZ )
{
    X = aX;   Y = aY;   Z = aZ;
}

//==========================================================================

inline void vector3::Negate( void )
{
    X = -X;   Y = -Y;   Z = -Z;
}

//==========================================================================

inline void vector3::Normalize( void )
{
    f32 L = x_sqrt( (X*X) + (Y*Y) + (Z*Z) );
    if( L < 0.001f )
    {
//        X = 1.0f;
        L = 1.0f;
    }
    ASSERT( L > 0.0f );
    L = 1.0f / L;
    X *= L;   Y *= L;   Z *= L;
}

//==========================================================================

inline void vector3::Scale( f32 S )
{
    //Change:  Y *= Y --> Y *= S
    X *= S;   Y *= S;   Z *= S;
}

//==========================================================================

inline void vector3::RotateX( radian Rx )
{
    radian S, C;
    f32    YY, ZZ;       // YY and ZZ are the "old" Y and Z values.
    x_sincos( Rx, S, C );
    YY = Y;
    ZZ = Z;
    Y  = (C * YY) - (S * ZZ);
    Z  = (C * ZZ) + (S * YY);
}

//==========================================================================

inline void vector3::RotateY( radian Ry )
{
    radian  S, C;
    f32     XX, ZZ;       // XX and ZZ are the "old" X and Z values.
    x_sincos( Ry, S, C );
    XX = X;
    ZZ = Z;
    X  = (C * XX) + (S * ZZ);
    Z  = (C * ZZ) - (S * XX);
}

//==========================================================================

inline void vector3::RotateZ( radian Rz )
{
    radian  S, C; 
    f32     XX, YY;       // XX and YY are the "old" X and Y values.
    x_sincos( Rz, S, C );
    XX = X;
    YY = Y;
    X  = (C * XX) - (S * YY);
    Y  = (C * YY) + (S * XX);
}

//==========================================================================
 
inline void vector3::Rotate( const radian3& Rotation )
{
    RotateX( Rotation.Pitch );
    RotateY( Rotation.Yaw   );
    RotateZ( Rotation.Roll  );
}
 
//==========================================================================

inline f32 vector3::Length( void ) const
{
    return( x_sqrt( (X*X) + (Y*Y) + (Z*Z) ) );
}

//==========================================================================

inline f32 vector3::SquaredLength( void ) const
{
    return( (X*X) + (Y*Y) + (Z*Z) );
}

//==========================================================================

inline float vector3::Dot( const vector3& V ) const
{
    return( (X * V.X) + (Y * V.Y) + (Z * V.Z) );
}

//==========================================================================

inline vector3 vector3::Cross( const vector3& V ) const
{
    return( vector3( (Y * V.Z) - (Z * V.Y),
                     (Z * V.X) - (X * V.Z),
                     (X * V.Y) - (Y * V.X) ) );
}

//==========================================================================

inline radian vector3::Pitch( void ) const
{
    f32 L = (f32)x_sqrt( X*X + Z*Z );
    return( -x_atan2( Y, L ) );
}

//==========================================================================

inline radian vector3::Yaw( void ) const
{
    return( x_atan2( X, Z ) );
}

//==========================================================================

inline void vector3::PitchYaw( radian& aPitch, radian& aYaw ) const
{
    aPitch = Pitch();
    aYaw   = Yaw();
}

////////////////////////////////////////////////////////////////////////////
//  Operators.
////////////////////////////////////////////////////////////////////////////

inline const vector3& vector3::operator = ( const vector3& V )
{
    if( this != &V )
    {
        X = V.X;   Y = V.Y;   Z = V.Z;
    }
    return( *this );
}

//==========================================================================
// You probably don't want to change "this" vector on
// unary minus.
//==========================================================================

inline vector3 vector3::operator - ( void ) const
{
    vector3 lhs(*this);
    lhs.Negate();
    return ( lhs );
}

//==========================================================================

/*
inline vector3& vector3::operator - ( void )
{
    X = -X;   Y = -Y;   Z = -Z;
    return( *this );
}
*/

//==========================================================================

inline vector3& vector3::operator += ( const vector3& V )
{
    X += V.X;   Y += V.Y;   Z += V.Z;
    return( *this );
}

//==========================================================================

inline vector3& vector3::operator -= ( const vector3& V )
{
    X -= V.X;   Y -= V.Y;   Z -= V.Z;
    return( *this );
}

//==========================================================================

inline vector3& vector3::operator *= ( const f32 S )
{
    X *= S;   Y *= S;   Z *= S;
    return( *this );
}

//==========================================================================

inline vector3& vector3::operator /= ( const f32 S )
{
    ASSERT( S != 0.0f );
    f32 D = 1.0f / S;
    X *= D;   Y *= D;   Z *= D;
    return( *this );
}

//==========================================================================

inline xbool vector3::operator == ( const vector3& V ) const
{
    if( (X == V.X) && (Y == V.Y) && (Z == V.Z) )
        return TRUE;
    else
        return FALSE;
}

//==========================================================================

inline xbool vector3::operator != ( const vector3& V ) const
{
    if( (X == V.X) && (Y == V.Y) && (Z == V.Z) )
        return FALSE;
    else
        return TRUE;
}

//==========================================================================

inline vector3 operator + ( const vector3& V1, const vector3& V2 )
{
    return( vector3( V1.X + V2.X, 
                     V1.Y + V2.Y, 
                     V1.Z + V2.Z ) );
}

//==========================================================================

inline vector3 operator - ( const vector3& V1, const vector3& V2 )
{
    return( vector3( V1.X - V2.X, 
                     V1.Y - V2.Y, 
                     V1.Z - V2.Z ) );
}

//==========================================================================

inline vector3 operator / ( const vector3& V, f32 Scalar )
{
    ASSERT( Scalar != 0.0f );
    Scalar = 1.0f / Scalar;
    return( vector3( V.X * Scalar, 
                     V.Y * Scalar, 
                     V.Z * Scalar ) );
}

//==========================================================================

inline vector3 operator * ( const vector3& V, f32 Scalar )
{
    return( vector3( V.X * Scalar, 
                     V.Y * Scalar, 
                     V.Z * Scalar ) );
}

//==========================================================================

inline vector3 operator * ( f32 Scalar, const vector3& V )
{
    return( vector3( V.X * Scalar, 
                     V.Y * Scalar, 
                     V.Z * Scalar ) );
}

////////////////////////////////////////////////////////////////////////////
//  Non-member function forms.
////////////////////////////////////////////////////////////////////////////

inline f32 Dot( const vector3& V1, const vector3& V2 )
{
    return( (V1.X * V2.X) + (V1.Y * V2.Y) + (V1.Z * V2.Z) );
}

//==========================================================================

inline f32 Distance( const vector3& V1, const vector3& V2 )
{
    return x_sqrt( (V1.X - V2.X) * (V1.X - V2.X) +
                   (V1.Y - V2.Y) * (V1.Y - V2.Y) +
                   (V1.Z - V2.Z) * (V1.Z - V2.Z) );
}

//==========================================================================

inline f32 DistanceSquared( const vector3& V1, const vector3& V2 )
{
    return (V1.X - V2.X) * (V1.X - V2.X) +
           (V1.Y - V2.Y) * (V1.Y - V2.Y) +
           (V1.Z - V2.Z) * (V1.Z - V2.Z);
}

//==========================================================================

inline vector3 Cross( const vector3& V1, const vector3& V2 )
{
    return( vector3( (V1.Y * V2.Z) - (V1.Z * V2.Y),
                     (V1.Z * V2.X) - (V1.X * V2.Z),
                     (V1.X * V2.Y) - (V1.Y * V2.X) ) );
}

//==========================================================================

inline radian AngleBetween( const vector3& V1, 
                            const vector3& V2 )
{
    f32 D, Cos;
    
    D = V1.Length() * V2.Length();
    
    if( D == 0.0f )  return( R_0 );
    
    Cos = Dot( V1, V2 ) / D;
    
    if     ( Cos >  1.0f )  Cos =  1.0f;
    else if( Cos < -1.0f )  Cos = -1.0f;
    
    return( x_acos( Cos ) );
}

//==========================================================================

inline f32 Length( const vector3& V )
{
    return( V.Length() );
}

//==========================================================================

inline f32 SquaredLength( const vector3& V )
{
    return( V.SquaredLength() );
}

//==========================================================================

inline radian Pitch( const vector3& V )
{
    return( V.Pitch() );
}

//==========================================================================

inline radian Yaw( const vector3& V )
{
    return( V.Yaw() );
}

//==========================================================================

inline void PitchYaw( const vector3& V, radian& Pitch, radian& Yaw )
{
    V.PitchYaw( Pitch, Yaw );
}

//==========================================================================
