////////////////////////////////////////////////////////////////////////////
//
//  X_MATH_V4_INLINE.HPP
//
////////////////////////////////////////////////////////////////////////////

#ifndef X_MATH_V4_INLINE_HPP
#define X_MATH_V4_INLINE_HPP
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

inline vector4:: vector4( void )        {  }
//inline vector4::~vector4( void )        {  }

////////////////////////////////////////////////////////////////////////////
//  Functions which actually have some work to do.
////////////////////////////////////////////////////////////////////////////

//==========================================================================

inline vector4::vector4( const vector4& V )
{
    X = V.X;   Y = V.Y;   Z = V.Z;   W = V.W;
}

//==========================================================================

inline vector4::vector4( const vector3& V )
{
    X = V.X;   Y = V.Y;   Z = V.Z;   W = 1.0f;
}

//==========================================================================

inline vector4::vector4( f32 aX, f32 aY, f32 aZ, f32 aW )
{
    X = aX;   Y = aY;   Z = aZ;   W = aW;
}

//==========================================================================

inline void vector4::Zero( void )
{
    X = Y = Z = W = 0.0f;
}

//==========================================================================

inline void vector4::Set( f32 aX, f32 aY, f32 aZ, f32 aW )
{
    X = aX;   Y = aY;   Z = aZ;   W = aW;
}

//==========================================================================

inline void vector4::Negate( void )
{
    X = -X;   Y = -Y;   Z = -Z;   W = -W;
}

//==========================================================================

inline void vector4::Normalize( void )
{
    f32 L = x_sqrt( (X*X) + (Y*Y) + (Z*Z) + (W*W) );
    ASSERT( L > 0.0f );
    L = 1.0f / L;
    X *= L;   Y *= L;   Z *= L;   W *= L;
}

//==========================================================================

inline void vector4::Scale( f32 S )
{
    //Change:  Y *= Y --> Y *= S
    X *= S;   Y *= S;   Z *= S;   W *= S;
}

//==========================================================================

inline f32 vector4::Length( void ) const
{
    return( x_sqrt( (X*X) + (Y*Y) + (Z*Z) + (W*W) ) );
}

//==========================================================================

inline f32 vector4::SquaredLength( void ) const
{
    return( ( (X*X) + (Y*Y) + (Z*Z) + (W*W) ) );
}

//==========================================================================

////////////////////////////////////////////////////////////////////////////
//  Operators.
////////////////////////////////////////////////////////////////////////////

inline const vector4& vector4::operator = ( const vector4& V )
{
    if( this != &V )
    {
        X = V.X;   Y = V.Y;   Z = V.Z;   W = V.W;
    }
    return( *this );
}

//==========================================================================
// You probably don't want to change "this" vector on
// unary minus.
//==========================================================================

inline vector4 vector4::operator - ( void )
{
    vector4 lhs(*this);
    lhs.Negate();
    return ( lhs );
}

//==========================================================================

/*
inline vector4& vector4::operator - ( void )
{
    X = -X;   Y = -Y;   Z = -Z;   W = -W;
    return( *this );
}
*/

//==========================================================================

inline vector4& vector4::operator += ( const vector4& V )
{
    X += V.X;   Y += V.Y;   Z += V.Z;   W += V.W;
    return( *this );
}

//==========================================================================

inline vector4& vector4::operator -= ( const vector4& V )
{
    X -= V.X;   Y -= V.Y;   Z -= V.Z;   W -= V.W;
    return( *this );
}

//==========================================================================

inline vector4& vector4::operator *= ( const f32 S )
{
    X *= S;   Y *= S;   Z *= S;   W *= S;
    return( *this );
}

//==========================================================================

inline vector4& vector4::operator /= ( const f32 S )
{
    ASSERT( S != 0.0f );
    f32 D = 1.0f / S;
    X *= D;   Y *= D;   Z *= D;   W *= D;
    return( *this );
}

//==========================================================================

inline xbool vector4::operator == ( const vector4& V ) const
{
    if( (X == V.X) && (Y == V.Y) && (Z == V.Z) && (W == V.W) )
        return TRUE;
    else
        return FALSE;
}

//==========================================================================

inline xbool vector4::operator != ( const vector4& V ) const
{
    if( (X == V.X) && (Y == V.Y) && (Z == V.Z) && (W == V.W) )
        return FALSE;
    else
        return TRUE;
}

//==========================================================================

inline vector4 operator + ( const vector4& V1, const vector4& V2 )
{
    return( vector4( V1.X + V2.X, 
                     V1.Y + V2.Y, 
                     V1.Z + V2.Z,
                     V1.W + V2.W ) );
}

//==========================================================================

inline vector4 operator - ( const vector4& V1, const vector4& V2 )
{
    return( vector4( V1.X - V2.X, 
                     V1.Y - V2.Y, 
                     V1.Z - V2.Z,
                     V1.W - V2.W ) );
}

//==========================================================================

inline vector4 operator / ( const vector4& V, f32 Scalar )
{
    ASSERT( Scalar != 0.0f );
    Scalar = 1.0f / Scalar;
    return( vector4( V.X * Scalar, 
                     V.Y * Scalar, 
                     V.Z * Scalar,
                     V.W * Scalar ) );
}

//==========================================================================

inline vector4 operator * ( const vector4& V, f32 Scalar )
{
    return( vector4( V.X * Scalar, 
                     V.Y * Scalar, 
                     V.Z * Scalar,
                     V.W * Scalar ) );
}

//==========================================================================

inline vector4 operator * ( f32 Scalar, const vector4& V )
{
    return( vector4( V.X * Scalar, 
                     V.Y * Scalar, 
                     V.Z * Scalar,
                     V.W * Scalar ) );
}

//==========================================================================


////////////////////////////////////////////////////////////////////////////
//  Non-member function forms.
////////////////////////////////////////////////////////////////////////////

//==========================================================================

inline f32 Length( const vector4& V )
{
    return( V.Length() );
}

//==========================================================================
