////////////////////////////////////////////////////////////////////////////
//
//  X_MATH_R3_INLINE.HPP
//
////////////////////////////////////////////////////////////////////////////

#ifndef X_MATH_R3_INLINE_HPP
#define X_MATH_R3_INLINE_HPP
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

inline radian3:: radian3( void )        {   }
inline radian3::~radian3( void )        {   }

////////////////////////////////////////////////////////////////////////////
//  Functions which actually have some work to do.
////////////////////////////////////////////////////////////////////////////

//==========================================================================

inline radian3::radian3( const radian3& V )
{
    Pitch = V.Pitch;    Yaw = V.Yaw;    Roll = V.Roll;
}

//==========================================================================

inline radian3::radian3( radian aPitch, radian aYaw, radian aRoll )
{
    Pitch = aPitch;     Yaw = aYaw;     Roll = aRoll;
}

//==========================================================================

inline void radian3::Zero( void )
{
    Pitch = 0.0f;       Yaw = 0.0f;     Roll = 0.0f;
}

//==========================================================================

inline void radian3::Set( radian aPitch, radian aYaw, radian aRoll )
{
    Pitch = aPitch;     Yaw = aYaw;     Roll = aRoll;
}

//==========================================================================

inline radian3 radian3::operator - ( void )
{
    radian3 lhs(*this);
	lhs.Pitch = -lhs.Pitch;
	lhs.Yaw = -lhs.Yaw;
	lhs.Roll = -lhs.Roll;
    return ( lhs );
}

//==========================================================================

inline radian3& radian3::operator += ( const radian3& R )
{
	Pitch += R.Pitch; Yaw += R.Yaw; Roll += R.Roll;
	return (*this);
}

//==========================================================================

inline radian3& radian3::operator -= ( const radian3& R )
{
	Pitch -= R.Pitch; Yaw -= R.Yaw; Roll -= R.Roll;
	return (*this);
}

//==========================================================================

inline radian3& radian3::operator *= ( f32 Scalar )
{
	Pitch *= Scalar; Yaw *= Scalar; Roll *= Scalar;
	return (*this);
}

//==========================================================================

inline radian3& radian3::operator /= ( f32 Scalar )
{
	Pitch /= Scalar; Yaw /= Scalar; Roll /= Scalar;
	return (*this);
}

//==========================================================================

inline xbool radian3::operator == ( const radian3& R ) const
{
	if ( (Pitch == R.Pitch) && (Yaw == R.Yaw) && (Roll == R.Roll) )
		return TRUE;

	return FALSE;
}

//==========================================================================

inline xbool radian3::operator != ( const radian3& R ) const
{
	if ( (Pitch == R.Pitch) && (Yaw == R.Yaw) && (Roll == R.Roll) )
		return FALSE;

	return TRUE;
}

//==========================================================================

inline const radian3& radian3::operator =  ( const radian3& R )
{
	if (this != &R)
	{
		Pitch = R.Pitch;
		Yaw = R.Yaw;
		Roll = R.Roll;
	}

	return (*this);
}

//==========================================================================

inline radian3 operator +  ( const radian3& R1, const radian3& R2 )
{
	return ( radian3( R1.Pitch + R2.Pitch,
					  R1.Yaw + R2.Yaw,
					  R1.Roll + R2.Roll ) );
}

//==========================================================================

inline radian3 operator -  ( const radian3& R1, const radian3& R2 )
{
	return ( radian3( R1.Pitch - R2.Pitch,
					  R1.Yaw - R2.Yaw,
					  R1.Roll - R2.Roll ) );
}

//==========================================================================

inline radian3 operator /  ( const radian3& R,  f32 Scalar )
{
	ASSERT( Scalar != 0.0 );
	Scalar = 1.0f / Scalar;

	return ( radian3( R.Pitch * Scalar,
					  R.Yaw * Scalar,
					  R.Roll * Scalar ) );
}

//==========================================================================

inline radian3 operator *  ( const radian3& R,  f32 Scalar )
{
	return ( radian3( R.Pitch * Scalar,
					  R.Yaw * Scalar,
					  R.Roll * Scalar ) );
}

//==========================================================================

inline radian3 operator *  ( f32 Scalar, const radian3& R )
{
	return ( radian3( R.Pitch * Scalar,
					  R.Yaw * Scalar,
					  R.Roll * Scalar ) );
}

//==========================================================================

