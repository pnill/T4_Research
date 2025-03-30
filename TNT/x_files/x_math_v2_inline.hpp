////////////////////////////////////////////////////////////////////////////
//
//  X_MATH_V2_INLINE.HPP
//
////////////////////////////////////////////////////////////////////////////

#ifndef X_MATH_V2_INLINE_HPP
#define X_MATH_V2_INLINE_HPP
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

inline vector2:: vector2( void )        {  }
//inline vector2::~vector2( void )        {  }

////////////////////////////////////////////////////////////////////////////
//  Functions which actually have some work to do.
////////////////////////////////////////////////////////////////////////////

//==========================================================================

inline vector2::vector2( const vector2& V )
{
    X = V.X;   Y = V.Y;
}

//==========================================================================

//inline vector2::vector2( const vector3& V )
//{
//    X = V.X;   Y = V.Y;
//}

//==========================================================================

inline vector2::vector2( const radian R )
{
    X = x_cos( R );     
    Y = x_sin( R );
}

//==========================================================================

inline vector2::vector2( f32 aX, f32 aY )
{
    X = aX;   Y = aY;
}

//==========================================================================

inline void vector2::Zero( void )
{
    X = Y = 0.0f;
}

//==========================================================================

inline void vector2::Set( f32 aX, f32 aY )
{
    X = aX;   Y = aY;
}

//==========================================================================

inline void vector2::Negate( void )
{
    X = -X;   Y = -Y;
}

//==========================================================================

inline void vector2::Normalize( void )
{
    f32 L = x_sqrt( (X*X) + (Y*Y) );
    ASSERT( L > 0.0f );
    L = 1.0f / L;
    X *= L;   Y *= L;
}

//==========================================================================

inline void vector2::Scale( f32 S )
{
    X *= S;   Y *= S;   
}

//==========================================================================

inline void vector2::Rotate( radian R )
{
    radian  S, C; 
    f32     XX, YY;       // XX and YY are the "old" X and Y values.
    x_sincos( R, S, C );
    XX = X;
    YY = Y;
    X  = (C * XX) - (S * YY);
    Y  = (C * YY) + (S * XX);
}

//==========================================================================

inline f32 vector2::Length( void ) const
{
    return( x_sqrt( (X*X) + (Y*Y) ) );
}

//==========================================================================

inline f32 vector2::SquaredLength( void ) const
{
    return( (X*X) + (Y*Y) );
}

//==========================================================================

inline radian vector2::Angle( void ) const
{
    return( x_atan2( Y, X ) );
}

////////////////////////////////////////////////////////////////////////////
//  Operators.
////////////////////////////////////////////////////////////////////////////

inline const vector2& vector2::operator = ( const vector2& V )
{
    if( this != &V )
    {
        X = V.X;   Y = V.Y;
    }
    return( *this );
}

//==========================================================================
// You probably don't want to change "this" vector on
// unary minus.
//==========================================================================

inline vector2 vector2::operator - ( void )
{
    vector2 lhs(*this);
    lhs.Negate();
    return ( lhs );
}

//==========================================================================

/*
inline vector2& vector2::operator - ( void )
{
    X = -X;   Y = -Y;
    return( *this );
}
*/

//==========================================================================

inline vector2& vector2::operator += ( const vector2& V )
{
    X += V.X;   Y += V.Y;
    return( *this );
}

//==========================================================================

inline vector2& vector2::operator -= ( const vector2& V )
{
    X -= V.X;   Y -= V.Y;
    return( *this );
}

//==========================================================================

inline vector2& vector2::operator *= ( const f32 S )
{
    X *= S;   Y *= S;
    return( *this );
}

//==========================================================================

inline vector2& vector2::operator /= ( const f32 S )
{
    ASSERT( S != 0.0f );
    f32 D = 1.0f / S;
    X *= D;   Y *= D;
    return( *this );
}

//==========================================================================

inline xbool vector2::operator == ( const vector2& V ) const
{
    if( (X == V.X) && (Y == V.Y) )
        return TRUE;
    else
        return FALSE;
}

//==========================================================================

inline xbool vector2::operator != ( const vector2& V ) const
{
    if( (X == V.X) && (Y == V.Y) )
        return FALSE;
    else
        return TRUE;
}

//==========================================================================

inline vector2 operator + ( const vector2& V1, const vector2& V2 )
{
    return( vector2( V1.X + V2.X, 
                     V1.Y + V2.Y ) );
}

//==========================================================================

inline vector2 operator - ( const vector2& V1, const vector2& V2 )
{
    return( vector2( V1.X - V2.X, 
                     V1.Y - V2.Y ) );
}

//==========================================================================

inline vector2 operator / ( const vector2& V, f32 Scalar )
{
    ASSERT( Scalar != 0.0f );
    Scalar = 1.0f / Scalar;
    return( vector2( V.X * Scalar, 
                     V.Y * Scalar ) );
}

//==========================================================================

inline vector2 operator * ( const vector2& V, f32 Scalar )
{
    return( vector2( V.X * Scalar, 
                     V.Y * Scalar ) );
}

//==========================================================================

inline vector2 operator * ( f32 Scalar, const vector2& V )
{
    return( vector2( V.X * Scalar, 
                     V.Y * Scalar ) );
}

////////////////////////////////////////////////////////////////////////////
//  Non-member function forms.
////////////////////////////////////////////////////////////////////////////

inline f32 Dot( const vector2& V1, const vector2& V2 )
{
    return( (V1.X * V2.X) + (V1.Y * V2.Y) );
}

//==========================================================================

inline radian AngleBetween( const vector2& V1, 
                            const vector2& V2 )
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

inline f32 Length( const vector2& V )
{
    return( V.Length() );
}

//==========================================================================

inline f32 SquaredLength( const vector2& V )
{
    return( V.SquaredLength() );
}

//==========================================================================

inline radian Angle( const vector2& V )
{
    return( V.Angle() );
}

//==========================================================================
