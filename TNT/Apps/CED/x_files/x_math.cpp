////////////////////////////////////////////////////////////////////////////
//
//  X_MATH.CPP
//
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//  INCLUDES
////////////////////////////////////////////////////////////////////////////

#ifndef X_MATH_HPP
#include "x_math.hpp"
#endif

#ifndef X_DEBUG_HPP
#include "x_debug.hpp"
#endif


////////////////////////////////////////////////////////////////////////////
//  DEFINES
////////////////////////////////////////////////////////////////////////////

#define HUGE    1.7976931348623158e+308

////////////////////////////////////////////////////////////////////////////
//  STORAGE
////////////////////////////////////////////////////////////////////////////

static u64 s_PosInf64Bits = 0x7FF0000000000000;
static f64 s_PosInf64     = *((f64*)(&s_PosInf64Bits));

////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
////////////////////////////////////////////////////////////////////////////

//==========================================================================

f32 x_sqrt( f32 a )
{
#if 1

	__asm
	{
		fld		a
		fsqrt
	};

#elif 1

	f32 result;

	__asm
	{
		fld		a
		fsqrt
		fstp	result
	};

	return result;
	
#else // ! I86 optimization

#ifdef TARGET_PS2

    register f32 Result;

    asm __volatile__
    ("
        sqrt.s  %0, %1
    " : "=f" (Result) : "f" (a) );
    
    return Result;

#elif defined(TARGET_WITH_A_BETTER_SQUARE_ROOT)

    // Put platform specific optimized implementation here.

#else

    //
    // SOFTWARE IMPLEMENTATION
    //
    // WARNING:     It uses double precision arithmetic.
    // Precision:   11.05 E-10
    // Range:       0 to e308
    //

    s32 Exponent;
    f64 Mantissa;
    f64 re;
    f64 Answer, p, q;

    //
    // There is no answer for negative numbers.
    //
    ASSERT( a >= 0.0f );

    //
    // Square root of 0 is 0.
    //
    if( a <= 0.0f ) 
        return( 0.0f );

    //
    // Get mantissa and exponent. 
    // The absolute value of the matissa will be in [0.5, 1.0).
    //
    Mantissa = x_frexp( a, &Exponent );

    // 
    // If the last bit in the exponent is set (it is odd), then
    // we need to set 're' to sqrt(2).
    //
    re = (Exponent & 1) ? 1.4142135623730950488016887242097 : 1.0;

    //
    // Shift out the last bit.
    //
    Exponent >>= 1;

    //
    // Goof with the mantissa and 're' in uncomprehensible ways.
    // Note the following:
    //         sqrt(2)    = 1.4142135623730950488016887242097
    //         sqrt(2)/2  = 0.707106781186547524400844362104849
    //   sqrt( sqrt(2) )  = 1.18920711500272106671749997056048
    //
    if( Mantissa < 0.707106781186547524400844362104849 )
    {
        Mantissa *= 1.4142135623730950488016887242097;

        if( re > 1.0 )  re =       1.18920711500272106671749997056048;
        else            re = 1.0 / 1.18920711500272106671749997056048;
    }

    //
    // Polynomial approximation from: 
    //   COMPUTER APPROXIMATIONS by Hart, J.F. et al. 1968
    //

    p =   0.54525387389085 + Mantissa * 
        (13.65944682358639 + Mantissa * 
        (27.09012271265500 + Mantissa *   
          6.43107724139784));

    q =   4.17003771413707 + Mantissa * 
        (24.84175210765124 + Mantissa * 
        (17.71411083016702 + Mantissa));

    Mantissa = p / q;

    //
    // Construct the answer from the mantissa, exponent, and 're' thing.
    //
    Answer = x_ldexp( Mantissa, Exponent ) * re;

    return( (f32)Answer );

#endif

#endif // ! I86 optimization

}

//==========================================================================
#if !defined (TARGET_PC) && !defined (TARGET_XBOX)
f64 x_floor( f64 a )
{
    f64 b;

    // If parameter is NaN, or already an integer...
    if( (a != a) || (a >= 4503599627370496.0) ) 
        return( a );

    // If parameter is negative, use negative ceiling.
    if( a < (f64)0 ) 
    {
        // Manual inline expansion of "return( -x_ceil(-a) );".

        a = -a;

        // If parameter is NaN, or already an integer...
        if( (a != a) || (a >= 4503599627370496.0) ) 
            return( -a );

        // Since we have already "recursed", we cannot do it again.
        ASSERT( a >= (f64)0 );

        // Now: 0 <= a < L
        b  = 4503599627370496.0 + a;        // Destructive store must be forced.
        b -= 4503599627370496.0;            // An integer, and |a-b| < 1.

        if( a > b )  return( -(b + (f64)1) );
        return( -b );
    }

    // Now: 0 <= a < L
    b  = 4503599627370496.0 + a;        // Destructive store must be forced.
    b -= 4503599627370496.0;            // An integer, and |a-b| < 1.

    if( a < b )  return( b - (f64)1 );
    return( b );
}

//==========================================================================

f64 x_ceil( f64 a )
{
    f64 b;

    // If parameter is NaN, or already an integer...
    if( (a != a) || (a >= 4503599627370496.0) ) 
        return( a );

    // If parameter is negative, use negative floor.
    if( a < (f64)0 ) 
    {
        // Manual inline expansion of "return( -x_floor(-a) );".

        a = -a;

        // If parameter is NaN, or already an integer...
        if( (a != a) || (a >= 4503599627370496.0) ) 
            return( -a );

        // Since we have already "recursed", we cannot do it again.
        ASSERT( a >= (f64)0 );

        // Now: 0 <= a < L
        b  = 4503599627370496.0 + a;        // Destructive store must be forced.
        b -= 4503599627370496.0;            // An integer, and |a-b| < 1.

        if( a < b )  return( -(b - (f64)1) );
        return( -b );
    }

    // Now: 0 <= a < L
    b  = 4503599627370496.0 + a;        // Destructive store must be forced.
    b -= 4503599627370496.0;            // An integer, and |a-b| < 1.

    if( a > b )  return( b + (f64)1 );
    return( b );
}
#endif	// !TARGET_PC && !TARGET_XBOX
//==========================================================================

f64 x_log( f64 x )
{
    f64 Rz, f, z, w, znum, zden, xn, a, b;
    s32 n;

    if( x <= 0.0 ) 
    {
        return( s_PosInf64 );  //return( -HUGE );
    }

    f = x_frexp( x, &n );

    if( f > 0.70710678118654752440 ) 
    {
        znum = (znum = f-0.5) - 0.5; // the assignment prevents const. eval 
        zden = f*0.5 + 0.5;
    } 
    else 
    {
        --n;
        znum = f - 0.5;
        zden = znum*0.5 + 0.5;
    }

    z = znum/zden;
    w = z*z;
    a = ((-0.78956112887491257267e+0 * w + 0.16383943563021534222e+2) * w - 0.64124943423745581147e+2);
    b = (((w - 0.35667977739034646171e+2) * w + 0.31203222091924532844e+3) * w - 0.76949932108494879777e+3);

    Rz = z + z * (w * a / b );
    xn = n;

    return( (xn*-2.121944400546905827679e-4 + Rz) + xn*0.693359375 );
}

//==========================================================================

f64 x_log10( f64 a )
{
    return( x_log(a) * 0.43429448190325182765 );
}

//==========================================================================

f64 x_log2( f64 a )
{
    return( x_log(a) * 1.442695041 );
}

//==========================================================================

f64 x_exp( f64 x )
{
    s32 n;
    f64 xn, g, r, z, p, q;

    if( (x > -2.710505e-20 ) && 
        (x <  2.710505e-20 ) ) 
        return( 1.0 );

    z = x * 1.4426950408889634074;
    n = (s32)z;
    if( n < 0 )      --n;
    if( z-n >= 0.5 ) ++n;

    xn = n;
    g = ((x - xn*0.693359375)) + xn*2.1219444005469058277e-4;
    z = g*g;

    p = ((0.165203300268279130e-4*z + 0.694360001511792852e-2)*z + 0.249999999999999993e+0);
    q = ((0.495862884905441294e-3*z + 0.555538666969001188e-1)*z + 0.500000000000000000e+0);

    r = p*g;
    r = 0.5 + r/(q-r);

    return( x_ldexp( r, n+1 ) );
}

//==========================================================================

f64 x_pow( f64 x, f64 y )
{
    f64 logx;

    if( y == 0.0 ) 
        return( 1 );

    if( x == 0.0 ) 
    {
        if ( y < 0.0 )  
        {
            return( s_PosInf64 );  //return( HUGE );
        }

        return( 0.0 );
    }

    // Handle the nasty case where x is negative.

    if( x < 0.0 )
    {
        s32 t;
        f64 j;

        // Handle when y is integral.

        t = (s32)y;        
        if( t == y )
        {
            logx  = x_log(-x);
            logx *= y;

            if( t & 1 ) return( -x_exp(logx) );
            return( x_exp(logx) );
        }

        // Handle when y is fractional.

        t = (s32)(1/y);
        j = y - t*y;

        // n must be an integer where n = 1/y.
        if( !((j > -2.710505e-20 ) && (j < 2.710505e-20)) )
            return( s_PosInf64 );  //return( -HUGE );

        // If it is even then blow up.
        if( (t&1) == 0 ) 
            return( s_PosInf64 );  //return( -HUGE );

        logx  = x_log(-x);
        logx *= y;

        return( -x_exp(logx) );
    }

    // Handle the happy case of x is positive.

    logx  = x_log(x);
    logx *= y;

    return( x_exp(logx) );
}

//==========================================================================
//  The fmod function calculates the floating-point remainder f of x / y 
//  such that x = i * y + f, where i is an integer, f has the same sign as 
//  x, and the absolute value of f is less than the absolute value of y.
//==========================================================================

f64 x_fmod( f64 x, f64 y )
{
    f64 quot = x/y;
    f64 mod  = x;

    mod -= (quot < 0.0) ? (x_ceil(quot) * y) : (x_floor(quot) * y);
    ASSERT( ABS(mod) < ABS(y) );
    return( mod );
}

//==========================================================================
//  The modf function breaks down the floating-point value x into fractional 
//  and integer parts, each of which has the same sign as x. The signed 
//  fractional portion of x is returned. The integer portion is stored as a 
//  floating-point value at *pWhole.
//==========================================================================

f64 x_modf( f64 a, f64* pWhole )
{
    if( a > 0 )
        *pWhole = x_floor(a);
    else
        *pWhole = x_ceil(a);

    return( a - *pWhole );
}

//==========================================================================
//  The frexp function breaks down the floating-point value (a) into a 
//  mantissa (m) and an exponent (n), such that the absolute value of m is 
//  greater than or equal to 0.5 and less than 1.0, and a = m*(2^n). The 
//  integer exponent n is stored at the location pointed to by pExp.  The
//  mantissa is returned.  If a is 0, the function returns 0 for both the
//  mantissa and the exponent.  There is no error return.
//==========================================================================

f64 x_frexp( f64 a, s32* pExp )
{
    typedef union 
    {
        double v;
        struct 
        {
#ifdef LITTLE_ENDIAN
			u32 u_mant2 : 32;
			u32 u_mant1 : 20;
			u32 u_exp   : 11;
            u32 u_sign  :  1;
#else
            u32 u_sign  :  1;
			u32 u_exp   : 11;
			u32 u_mant1 : 20;
			u32 u_mant2 : 32;
#endif
        } s;
    } ieee;

    ieee u;

    ASSERT( pExp );

	if( a ) 
    {
		u.v = a;
		*pExp = u.s.u_exp - 1022;
		u.s.u_exp = 1022;

		return( u.v );
	} 

	*pExp = 0;
	return( (f64)0 );
}

//==========================================================================
// f32 version
//==========================================================================
/*
void x_frexp( f32 x, f32& Man, s32& Exp )
{
    typedef union 
    {
        f32 v;
        struct 
        {
            #ifdef LITTLE_ENDIAN
			u32 u_mant1 : 23;
			u32 u_exp   :  8;
            u32 u_sign  :  1;
            #else
            u32 u_sign  :  1;
			u32 u_exp   :  8;
			u32 u_mant1 : 23;
            #endif
        } s;
    } ieee;

    ieee u;

	if( x ) 
    {
		u.v = x;
		Exp = u.s.u_exp - 127;
		u.s.u_exp = 127;
        Man = u.v;
	} 
    else
    {
        Exp = 0;
        Man = 0;
    }
}
*/
//==========================================================================

f64 x_ldexp( f64 x, s32 exp )
{
    typedef union
    {
        double v;
        struct 
        {
#ifdef LITTLE_ENDIAN
			u32 u_mant2 : 32;
			u32 u_mant1 : 20;
			u32 u_exp   : 11;
            u32 u_sign  :  1;
#else
            u32 u_sign  :  1;
			u32 u_exp   : 11;
			u32 u_mant1 : 20;
			u32 u_mant2 : 32;
#endif
        } s;
    } ieee;

    ieee* u;
    u32 exponent;

    u = (ieee*)&x;

    exponent = u->s.u_exp;

    // The order of the tests is carefully chosen to handle
    // the usual case first, with no branches taken.  
    if( exponent != 0 )
    {
        // X is nonzero and not denormalized.  
        if( exponent <= (1024 - (-1021) + 1) )
        {
	        // X is finite.  When exp < 0, overflow is actually underflow.
	        exponent = (u32)((s32)exponent + exp);

	        if( exponent != 0 )
	        {
                //
                // exponent is in valid range.
                //
	            if( exponent <= (1024 - (-1021) + 1) )
		        {
		            // In range.  
		            u->s.u_exp = exponent;
		            return( x );
		        }

                if( exp >= 0 )
overflow:
		        {
                    // Overflow.
                    s32 negative = u->s.u_sign;                    
                    x = HUGE;
		            u->s.u_sign = negative;

                    // ANSI C function would set: errno = ERANGE
		            return( s_PosInf64 );  //return( x );
		        }

                if (exponent <= (u32)(-(53 + 1)))
		        {
		            // Underflow.  
		            s32 negative = u->s.u_sign;
		            x = 0.0L;
		            u->s.u_sign = negative;

                    // ANSI C function would set: errno = ERANGE
		            return( x );
		        }
	        }

	        // Gradual underflow.  
	        u->s.u_exp = 1;
	        x *= x_ldexp( 1.0L, (s32)exponent - 1 );

            // if( (u->s.u_mant1 == 0) && (u->s.u_mant2 == 0) )
            // then ANSI C function would set: errno = ERANGE

	        return( x );
        }

        // X is +-infinity or NaN.  
        if( (u->s.u_mant1 == 0) && (u->s.u_mant2 == 0) )
  	    {
            // X is +-infinity.  
            if( exp >= 0 )
            {
                goto overflow;
            }
            else
	        {
                // (infinity * number < 1).  With infinite precision,
                // (infinity / finite) would be infinity, but otherwise 
                // it's safest to regard (infinity / 2) as indeterminate.
		        // The infinity might be (2 * finite).

                // ANSI C function would set: errno = EDOM

                return( x );
	        }
	    }

        // X is NaN.  
        // ANSI C function would set: errno = EDOM
        return( x );
    }

    // X is zero or denormalized.  
    if( (u->s.u_mant1 == 0) && (u->s.u_mant2 == 0) )
    {
        // X is +-0.0. 
        return( x );
    }

    // X is denormalized.
    // Multiplying by 2^LDBL_MANT_DIG normalizes it; we then subtract the 
    // LDBL_MANT_DIG we added to the exponent.
    // #define DBL_MANT_DIG    53

    return( x_ldexp( x * x_ldexp( 1.0L, 53 ), exp - 53 ) );
}

//==========================================================================

f32 x_1oversqrt( f32 a )
{
    ASSERT( a >= 0 );
    if( a == 0 )  a = 0.001f;
#ifdef TARGET_PS2
    register f32 Result;
    register f32 One = 1.0f;

    asm __volatile__
    ("
        rsqrt.s %0, %1, %2
    ": "=f" (Result) : "f" (One) , "f" (a) );
    
    return Result;
#else
    return( 1/x_sqrt(a) );
#endif
}

//==========================================================================

f32 x_lpr( f32 a, f32 Range )
{
    ASSERT( Range > 0 );

    a = (f32)x_fmod( a, Range );

    if( a < 0 ) 
        a += Range;

    return( a );
}

//==========================================================================
// Implementation of sin() as a Taylor series.
// Precision: +/- 0.000,000,005

/*
f32 x_sin( radian Angle  )
{
#if 1
    radian s, a, a2;
    s32 Sign;

    Sign = 1;

    // Get the angle into range [ -90 .. +270 ].
    Angle = x_ModAngle( Angle );
    while( Angle < -R_90  )  Angle += R_360;
    while( Angle >  R_270 )  Angle -= R_360;

    if( Angle > R_90 )
    {
        Angle -= R_180;
        Sign   = -1;
    }

    // And now the fun math...
    a  = Angle;
    a2 = a * a;    
    
    s  = (((((  + ( 1/362880.0f ) ) * a2        // error: 0.000014
                - ( 1/5040.0f   ) ) * a2        // error: 0.000544
                + ( 1/120.0f    ) ) * a2        // error: 0.013482
                - ( 1/6.0f      ) ) * a2        // error: 0.210513
                + ( 1.0f        ) ) * a;        // error: > 1

    return( Sign < 0 ? -s : s );
#else
    f32 xi, y, q, q2;
    s32 sign;

    xi = x;
    sign = 1;

    // Make sure that falls inside the range
    x = x_ModAngle(x);
    while (xi < -1.57079632679489661923f) xi += 6.28318530717958647692f;
    while (xi > 4.71238898038468985769f)  xi -= 6.28318530717958647692f;

    if (xi > 1.57079632679489661923f)
    {
        xi -= 3.141592653589793238462643f;
        sign = -1;
    }

    // solve sin
    q   = xi;
    q2  = q * q;
    y  = (((((  + ( 1/362880.0f ) ) * q2        //error: 0.000014
                - ( 1/5040.0f   ) ) * q2        //error: 0.000544
                + ( 1/120.0f    ) ) * q2        //error: 0.013482
                - ( 1/6.0f      ) ) * q2        //error: 0.210513
                + ( 1.0f        ) ) * q;        //error: > 1

    return(sign < 0 ? -y : y);
#endif 
}
#endif
*/

/*
#if 0
    f32 xi, y; 
    radian q, q2;
    s32 sign;

    xi = x;
    sign = 1;

    // Make sure that falls inside the range
    x = x_ModAngle(x);
    while (xi < -1.57079632679489661923f) xi += 6.28318530717958647692f;
    while (xi > 4.71238898038468985769f)  xi -= 6.28318530717958647692f;

    if (xi > 1.57079632679489661923f)
    {
        xi -= 3.141592653589793238462643f;
        sign = -1;
    }

    // solve sin
    q   = xi;
    q2  = q * q;
    y  = (((((  + ( 1/362880.0f ) ) * q2        //error: 0.000014
                - ( 1/5040.0f   ) ) * q2        //error: 0.000544
                + ( 1/120.0f    ) ) * q2        //error: 0.013482
                - ( 1/6.0f      ) ) * q2        //error: 0.210513
                + ( 1.0f        ) ) * q;        //error: > 1

    return(sign < 0 ? -y : y);
#endif
*/
//==========================================================================

/*
f32 x_cos( radian Angle )
{
    return( x_sin( Angle + R_90 ) );
}
*/

//==========================================================================

f32 x_tan( radian Angle )
{
    radian s, c;
    x_sincos( Angle, s, c );   
    return( s / c );
}

//===============================================
// x_tanh
//-----------------------------------------------
// tanh returns the hyperbolic tangent of x. 
//===============================================
f64 x_tanh( f64 x )
{
    f64 f,g,r;

    if (x<0) f = -x;
    else     f =  x;

    if (f > 25.3) r = 1.0;
    else 
    {
        if (f > 0.54930614433405484570) 
        {
            r = 0.5 - 1.0/(x_exp(f+f)+1.0);
            r += r;
        } 
        else 
        {
            if (f < 2.3e-10) r = f;
            else 
            {
                f64 p,q;

                g = f*f;
                p = (((-0.96437492777225469787e+0*g -0.99225929672236083313e+2)*g -0.16134119023996228053e+4)*g);
                q = (((g +0.11274474380534949335e+3)*g +0.22337720718962312926e+4)*g +0.48402357071988688686e+4);
                r = f + f* p/q;
            }
        }
    }

    if (x < 0.0) r = -r;
    return r;
}

//===============================================
// x_asin
//-----------------------------------------------
// Calculates arcsin(x)
// Range: 0 <= x <= 1
// Precision: +/- .000,000,02
//===============================================
radian x_asin( f32 Sine )
{
    f32 y;
    u32 sign;

    if (Sine>1.0f)     Sine = 1.0f;
    if (Sine<-1.0f)    Sine = -1.0f;

    sign = 0;

    if (Sine < 0)
    {
        sign = 1;
        Sine = -Sine;
    }

    y = ((((((- 0.0012624911f //  * Sine      //error: 0.000030
              + 0.0066700901f )// * Sine      //error: 0.000133
              - 0.0170881256f ) * Sine      //error: 0.000336
              + 0.0308918810f ) * Sine      //error: 0.000771
              - 0.0501743046f ) * Sine      //error: 0.002115
              + 0.0889789874f ) * Sine      //error: 0.009013
              - 0.2145988016f ) * Sine      //error: > 1
              + 1.5707963050f;

    y = 1.57079632679f - x_sqrt(1.0f - Sine) * y;

    return (sign ? -y:y);
}

//===============================================
// x_acos
//-----------------------------------------------
// Calculates arccos(Cosine)
// Range: 0 <= Cosine <= 1
// Precision: +/- .000,000,02
//===============================================
radian x_acos( f32 Cosine )
{
    return PI/2 - x_asin(Cosine);
}

//===============================================
// x_atan
//-----------------------------------------------
// Calculates arctan(Tangent)
// Range: -infinite <= Tangent <= infinite (Output -pi/2 to +pi/2)
// Precision: +/- .000,000,04
//===============================================
radian x_atan( f32 Tangent )
{
    f32 xi, q, q2, y;

    xi = (Tangent < 0 ? -Tangent : Tangent);
    q = (xi - 1.0f) / (xi + 1.0f);
    q2 = q * q;

    y = ((((((( - 0.0040540580f  * q2          //error: 0.00213
                + 0.0218612286f) * q2          //error: 0.000883
                - 0.0559098861f) * q2          //error: 0.001939
                + 0.0964200441f) * q2          //error: 0.003476
                - 0.1390853351f) * q2          //error: 0.006585
                + 0.1994653599f) * q2          //error: 0.015963
                - 0.3332985605f) * q2          //error: 0.070948
                + 0.9999993329f) * q           //error: > 1
                + 0.785398163397f;

    return(Tangent < 0 ? -y: y);
}

//===============================================
// x_atan2
//-----------------------------------------------
// Calculates arctan2(y,x)
// Range: -infinite <= x <= infinite (Output -pi to +pi)
// Precision: (Depends on the x_atan)
//===============================================
radian x_atan2 ( f32 y, f32 x )
{
    f32 z;
    f32 zabs;
    u32 sign;
    static f32 SignArray[] = {
                               0,   // atan(+,+)
                               PI,  // atan(+,-)
                               0,   // atan(-,+)
                              -PI   // atan(-,-)
                             };

    //----------------------------------------------
    // Collapse small values to 0.0
    //----------------------------------------------
    if ( x > -0.000001f && x < 0.000001f ) x = 0.0f;
    if ( y > -0.000001f && y < 0.000001f ) y = 0.0f;

    //----------------------------------------------
    // Check if vector has no length
    //----------------------------------------------
    if (x == 0.0f && y == 0.0f)
    {
        return 0;
    }

    //----------------------------------------------
    // Check if at 90 or -90 degrees
    //----------------------------------------------
    if (x == 0.0f)
    {
        if (y >= 0.0f) return PI/2;
        else return -PI/2;
    }

    //----------------------------------------------
    // Check if at 0 or 180 degrees
    //----------------------------------------------
    if (y == 0.0f)
    {
        if (x >= 0.0f) return 0;
        else return PI;
    }

    //----------------------------------------------
    // decide on sign index
    //----------------------------------------------
    if (x < 0)  sign = 1;
    else        sign = 0;
    if (y < 0) sign |= 2;

    //----------------------------------------------
    // find the ratio
    //----------------------------------------------
    z = (y/x);
    if (sign && sign != 3) zabs = -z;
    else                   zabs =  z;

    //----------------------------------------------
    // Handle very small numbers
    //----------------------------------------------
    if ( zabs < 0.000001f )
    {
        return SignArray[sign];
    }

    //----------------------------------------------
    // solve regular expresion
    //----------------------------------------------
    z = x_atan(z);

    return z + SignArray[sign];
}  

//==========================================================================
// Implementation of sin and cos functions as Taylor series.
// Precision: +/- 0.000,000,005

/*
void x_sincos( radian Angle, f32& Sine, f32& Cosine )
{
    radian sq, sq2;
    radian cq, cq2;
    radian sa, ca;

    // Get the angle into range [ 0 .. R360 ].
    Angle = x_ModAngle( Angle );

    //
    // Choose angles to feed into the Taylor series equation.
    //
    if( Angle >= R_180 )
    {
        if( Angle >= R_270 )
        {
            Angle -= R_270;
            sa     = Angle - R_90;
            ca     = Angle;
        }
        else
        {
            Angle -= R_180;
            sa     = -Angle;
            ca     =  Angle - R_90;
        }
    }
    else
    {
        if( Angle >= R_90 )
        {
            Angle -= R_90;
            sa     = R_90 - Angle;
            ca     = -Angle;
        }
        else
        {
            sa = Angle;
            ca = R_90 - Angle;
        }
    }

    //----------------------------------------------
    // solve sin
    //----------------------------------------------
    sq  = sa;
    sq2 = sq * sq;
    Sine = (((((  + ( 1/362880.0f ) ) * sq2       //error: 0.000014
                  - ( 1/5040.0f   ) ) * sq2       //error: 0.000544
                  + ( 1/120.0f    ) ) * sq2       //error: 0.013482
                  - ( 1/6.0f      ) ) * sq2       //error: 0.210513
                  + ( 1.0f        ) ) * sq;       //error: > 1

    //----------------------------------------------
    // solve cos
    //----------------------------------------------
    cq  = ca;
    cq2 = cq * cq;
    Cosine = (((((  + ( 1/362880.0f ) ) * cq2       //error: 0.000014
                    - ( 1/5040.0f   ) ) * cq2       //error: 0.000544
                    + ( 1/120.0f    ) ) * cq2       //error: 0.013482
                    - ( 1/6.0f      ) ) * cq2       //error: 0.210513
                    + ( 1.0f        ) ) * cq;       //error: > 1
}
*/

//==========================================================================

radian x_ModAngle( radian x )
{
    if( ( x > (4*R_360) ) || ( x < (-4*R_360) ) )
    {
        x = (f32)x_fmod( x, R_360 );
    }

    while( x >= R_360 )  x -= R_360;
    while( x <  R_0   )  x += R_360;

    ASSERT( (x >= (R_0-0.01f)) && (x < (R_360+0.01f)) );
    return( x );
}

//==========================================================================

radian x_ModAngle2( radian x )
{
    x += PI;
    x  = x_ModAngle( x );
    x -= PI;
    
    return( x );
}

//==========================================================================

radian x_MinAngleDiff( radian a, radian b )
{
    return x_ModAngle2( a - b );
}

//==========================================================================

f32 x_round( f32 x, f32 y )
{
    f32 quot;
    
    ASSERT( x_abs( y ) > 0.0001f );

    quot = x/y;

    if ( quot < 0.0 )
    {
        x -= ( x - ((f32)x_ceil( quot - 0.5f )) * y );
    }
    else
    {
        x -= ( x - ((f32)x_floor( quot + 0.5f )) * y );
    }

    return( x );
}

//==========================================================================
//==========================================================================
// CLIP
//==========================================================================
//==========================================================================


//==========================================================================

xbool ClipLineToPlane( vector3& Dest, vector3& V0, vector3& V1,
                       f32 NX, f32 NY, f32 NZ, f32 D )
{
    f32 s, t;
    
    s = ( V0.X*NX + V0.Y*NY + V0.Z*NZ + D );
    t = ( V1.X*NX + V1.Y*NY + V1.Z*NZ + D );
    
    if( (s<0) == (t<0) ) return FALSE;

    t = s - t;

    if( t < 0.001f && t > -0.001f ) t = 0.5f;
    else                            t = s / t;
    
    Dest.X = V0.X + t*( V1.X - V0.X );
    Dest.Y = V0.Y + t*( V1.Y - V0.Y );
    Dest.Z = V0.Z + t*( V1.Z - V0.Z );    

    return TRUE;    
}

//==========================================================================

xbool ClipRayToPlane ( f32& DestT, vector3& Origin, vector3& Direction,                       
                       f32 NX, f32 NY, f32 NZ, f32 D )
{
    f32 a,b;
    
    a = Direction.X * NX + Direction.Y * NY + Direction.Z * NZ;
    b = NX*Origin.X +  NY*Origin.Y + NZ*Origin.Z + D;
    
    if( a < 0.001f && a > -0.001f ) 
    {
        DestT = 0;
        
        if( b < 0.001f && b > -0.001f ) return TRUE;
        return FALSE;
    }
                       
    DestT = -b / a;
    
    return TRUE;                                                 
}

//==========================================================================

xbool ClipPolyToPlane( vector3*  pDest,
                       s32&      NDest,
                       vector3*  pSrc,
                       s32       NSrc,
                       f32       NX,
                       f32       NY,
                       f32       NZ,
                       f32       D )
{

    vector3*    p0;
    vector3*    p1;
    vector3*    cp;
    s32         NewCount=0;
    xbool       p0in,p1in;
    s32         i;
    xbool       AnythingClipped = FALSE;

    ASSERT(pDest);
    ASSERT(pSrc);

    // Check for simple return;
    NDest = 0;
    if( NSrc == 0 ) return FALSE;

    // Get side of plane point is on
    p0   = &pSrc[ NSrc - 1 ];
    p0in = ( (p0->X*NX + p0->Y*NY + p0->Z*NZ + D) >= 0);

    // Loop through points
    for( i = 0; i < NSrc; i++ )
    {
        // Get side of plane point is on
        p1   = &pSrc[i];
        p1in = ( (p1->X*NX + p1->Y*NY + p1->Z*NZ + D) >= 0);

        // Check if we include this point
        if( p0in )
        {
            pDest[ NewCount ] = *p0;
            NewCount++;
        }

        // Check if we need to clip these points
        if( p0in != p1in )
        {
            AnythingClipped = TRUE;

            // get new point in destination for clipped point
            cp = &pDest[NewCount];
            NewCount++;

            VERIFY( ClipLineToPlane( *cp, *p0, *p1, NX, NY, NZ, D ) == TRUE );
        }

        // Move to next line segment
        p0   = p1;
        p0in = p1in;
    }

    // Return new count to user
    NDest = NewCount;
    
    return AnythingClipped;
}

//===============================================
// x_XYZToZXYRot
//===============================================
void radian3::XYZ_to_ZXY( void )
{
    f32 M20,M22,M01,M11,M02,M00;
    f32 sx,cx,sy,cy,sz,cz;
    radian RX,RY,RZ;
    f32 s=0,CX,cxcz,sxsz;

    ASSERT(this);

    x_sincos(this->Pitch,sx,cx);
    x_sincos(this->Yaw,sy,cy);
    x_sincos(this->Roll,sz,cz);

    // compute RX
    s = cx*sz*sy - sx*cz;
    if (s > 1.0f) s= 1.0f;
    if (s <-1.0f) s=-1.0f;
    RX = (f32)x_asin(-s);

    // compute RY,RZ
    CX = x_cos(RX);
    if ( (CX>0.0001f) || (CX<-0.0001f) )
    {
        cxcz = cx*cz;
        sxsz = sx*sz;
        M20  = cxcz*sy + sxsz;
        M22  = cx*cy;
        M01  = cy*sz;
        M11  = sxsz*sy + cxcz;
        RY   = x_atan2(M20,M22);
        RZ   = x_atan2(M01,M11);
    }
    else
    {
        M02  = -sy;
        M00  = cy*cz;
        RZ   = x_atan2(M02,M00);
        RY   = 0;
    }

    this->Pitch	= RX;
    this->Yaw	= RY;
    this->Roll	= RZ;
}

//===============================================
// x_XYZToXZYRot
//===============================================
void radian3::XYZ_to_XZY( void )
{
#if 0

	matrix4 m;
	m.SetRotationXYZ(

#else
    f32 M01,M21,M11,M02,M00,M12,M10;
    f32 sx,cx,sy,cy,sz,cz;
    radian RX,RY,RZ;
    f32 s=0,CZ,sysz;

    ASSERT(this);

    x_sincos(this->Pitch,sx,cx);
    x_sincos(this->Yaw,sy,cy);
    x_sincos(this->Roll,sz,cz);

    // compute RZ
    M01 = cy*sz;
    s   = M01;
    if (s > 1.0f) s= 1.0f;
    if (s <-1.0f) s=-1.0f;
    RZ  = (f32)x_asin(s);

    // compute RX,RY
    CZ = x_cos(RZ);
    if ( (CZ>0.0001f) || (CZ<-0.0001f) )
    {
        sysz = sy*sz;
        M21 = cx*sysz - sx*cz;
        M11 = sx*sysz + cx*cz;
        M02 = -sy;
        M00 = cy*cz;
        RX   = x_atan2(-M21,M11);
        RY   = x_atan2(-M02,M00);
    }
    else
    {
        M12  = sx*cy;
        M10  = sx*sy*cz - cx*sz;
        RX   = x_atan2(M12,-M10);
        RY   = 0;
    }

    this->Pitch	= RX;
    this->Yaw	= RY;
    this->Roll	= RZ;
#endif
}

//===============================================
// x_XZYToXYZRot
//===============================================
void radian3::XZY_to_XYZ( void )
{
    f32 M02,M12,M22,M01,M00,M21,M11;
    f32 sx,cx,sy,cy,sz,cz;
    radian RX,RY,RZ;
    f32 s=0,CY,sysz;

    ASSERT(this);

    x_sincos(this->Pitch,sx,cx);
    x_sincos(this->Yaw,sy,cy);
    x_sincos(this->Roll,sz,cz);

    // compute RY
    M02 = -sy*cz;
    s = M02;
    if (s > 1.0f) s= 1.0f;
    if (s <-1.0f) s=-1.0f;
    RY = (f32)x_asin(-s);

    // compute RX,RZ
    CY = x_cos(RY);
    if ( (CY>0.0001f) || (CY<-0.0001f) )
    {
        sysz = sy*sz;
        M12  =  cx*sysz + sx*cy;
        M22  = -sx*sysz + cy*cx;
        M01  = sz;
        M00  = cy*cz;
        RX   = x_atan2(M12,M22);
        RZ   = x_atan2(M01,M00);
    }
    else
    {
        M21  = -cz*sx;
        M11  = cz*cx;
        RX   = x_atan2(M21,M11);
        RZ   = 0;
    }

    this->Pitch	= RX;
    this->Yaw	= RY;
    this->Roll	= RZ;
}

//===============================================
// ZXY_to_XYZ
//===============================================
void radian3::ZXY_to_XYZ( void )
{
    f32 M12,M22,M01,M00,M21,M11;
    f32 sx,cx,sy,cy,sz,cz;
    radian RX,RY,RZ;
    f32 s=0,CY,cycz,sysz;

    ASSERT(this);

    x_sincos(this->Pitch,sx,cx);
    x_sincos(this->Yaw,sy,cy);
    x_sincos(this->Roll,sz,cz);

    // compute RY
    s = cy*sx*sz - sy*cz;
    if (s > 1.0f) s= 1.0f;
    if (s <-1.0f) s=-1.0f;
    RY = (f32)x_asin(-s);

    // compute RX,RZ
    CY = x_cos(RY);
    if ( (CY>0.0001f) || (CY<-0.0001f) )
    {
        cycz = cy*cz;
        sysz = sy*sz;
        M12  = sysz + sx*cycz;
        M22  = cx*cy;
        M01  = cx*sz;
        M00  = cycz + sysz*sx;
        RX   = x_atan2(M12,M22);
        RZ   = x_atan2(M01,M00);
    }
    else
    {
        M21  = -sx;
        M11  = cx*cz;
        RX   = x_atan2(-M21,M11);
        RZ   = 0;
    }

    this->Pitch	= RX;
    this->Yaw	= RY;
    this->Roll	= RZ;
}

//==========================================================================
//==========================================================================
//==========================================================================
//==========================================================================
//==========================================================================
//==========================================================================
//==========================================================================
//==========================================================================
//==========================================================================
//==========================================================================
//==========================================================================
