////////////////////////////////////////////////////////////////////////////
//
//  X_MATH_INLINE.HPP
//
////////////////////////////////////////////////////////////////////////////

#ifndef X_MATH_INLINE_HPP
#define X_MATH_INLINE_HPP
#else
#error "File " __FILE__ " has been included twice!"
#endif

#include "x_debug.hpp"

//==========================================================================

inline s8  x_abs ( s8  a )  { return( a < 0 ? (s8 )-a : a ); }
inline s16 x_abs ( s16 a )  { return( a < 0 ? (s16)-a : a ); }
inline s32 x_abs ( s32 a )  { return( a < 0 ? (s32)-a : a ); }
inline s64 x_abs ( s64 a )  { return( a < 0 ? (s64)-a : a ); }
inline f32 x_abs ( f32 a )  { return( a < 0 ? (f32)-a : a ); }
inline f64 x_abs ( f64 a )  { return( a < 0 ? (f64)-a : a ); }

inline f32 x_sqr ( f32 a )  { return( a * a ); }

//==========================================================================

//--------------------------------------------------------------------------
// Fast sin/cos/sincos functions using lookup tables
//
//  The lookup table contains the values of sin from R_0 to R_90, which will
//  suffice for R_90..R_360, and from that you can also get cosine.
//--------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////
// SIN TABLE CONSTS
////////////////////////////////////////////////////////////////////////////
#ifdef SMALL_SIN_LOOKUP
    const s32 SIN_TABLE_SIZE = 4096;
#else
    const s32 SIN_TABLE_SIZE = 8192;
#endif

const f32 SIN_TABLE_MULT = (SIN_TABLE_SIZE*4) / R_360;

extern f32 g_SinCos[ SIN_TABLE_SIZE+1 ];


////////////////////////////////////////////////////////////////////////////
// INLINE SINE/COSINE FUNCTIONS
////////////////////////////////////////////////////////////////////////////

inline f32 x_fastsin( radian R )
{
    s32 Lookup = (s32)(R * SIN_TABLE_MULT);

    //---   get the angle within a proper range for lookup
    Lookup = Lookup & (SIN_TABLE_SIZE*4-1);

    //---   perform the lookup based on the ranges 0..90,90..180,180..270,270..360
    if( Lookup < SIN_TABLE_SIZE )           return  g_SinCos[ Lookup ];
    else if( Lookup < SIN_TABLE_SIZE*2 )    return  g_SinCos[ SIN_TABLE_SIZE*2 - Lookup ];
    else if( Lookup < SIN_TABLE_SIZE*3 )    return -g_SinCos[ Lookup - SIN_TABLE_SIZE*2 ];
    else                                    return -g_SinCos[ SIN_TABLE_SIZE*4 - Lookup ];
}

//==========================================================================

inline f32 x_fastcos( radian R )
{
    // cos(x) = sin(x+R_90)
    s32 Lookup = (s32)(R * SIN_TABLE_MULT) + SIN_TABLE_SIZE;

    //---   get the angle within a proper range for lookup
    Lookup = Lookup & (SIN_TABLE_SIZE*4-1);

    //---   perform the lookup based on the ranges 0..90,90..180,180..270,270..360
    if ( Lookup < SIN_TABLE_SIZE )          return  g_SinCos[ Lookup ];
    else if ( Lookup < SIN_TABLE_SIZE*2 )   return  g_SinCos[ SIN_TABLE_SIZE*2 - Lookup ];
    else if ( Lookup < SIN_TABLE_SIZE*3 )   return -g_SinCos[ Lookup - SIN_TABLE_SIZE*2 ];
    else                                    return -g_SinCos[ SIN_TABLE_SIZE*4 - Lookup ];
}

//==========================================================================

inline void x_fastsincos( radian R, f32& rSin, f32& rCos )
{
    rSin = x_fastsin( R );
    rCos = x_fastcos( R );
}

//==========================================================================

static f32 const halfPi   = f32( 3.141592653589793238462643 / 2.0 );
static f32 const ooHalfPi = f32( 2.0 / 3.141592653589793238462643 );

inline void x_sincos( radian radians, f32& Sin, f32& Cos )
{
    s32 n;

#if !defined( TARGET_XBOX )
    n = s32( radians * ooHalfPi + ( radians < 0 ? -0.5f : 0.5f ) );
#else
    __asm
    {
        fld     radians 
        fmul    ooHalfPi 
        fistp   n
    }
#endif

    f32 const a  = radians - n * halfPi;
    f32 const a2 = a * a;

    if ( n & 1 )
    {
        Sin = ((((( + f32( 1.0f / 40320.0f  ) ) * a2  // Cosine
                    - f32( 1.0f / 720.0f    ) ) * a2
                    + f32( 1.0f / 24.0f     ) ) * a2
                    - f32( 1.0f / 2.0f      ) ) * a2
                    + f32( 1.0f             ) );      

        Cos = ((((( - f32( 1.0f / 362880.0f ) ) * a2  // -Sine
                    + f32( 1.0f / 5040.0f   ) ) * a2
                    - f32( 1.0f / 120.0f    ) ) * a2
                    + f32( 1.0f / 6.0f      ) ) * a2
                    - f32( 1.0f             ) ) * a;
    }
    else
    {
        Sin = ((((( + f32( 1 / 362880.0 ) ) * a2  // Sine
                    - f32( 1 / 5040.0   ) ) * a2
                    + f32( 1 / 120.0    ) ) * a2
                    - f32( 1 / 6.0      ) ) * a2
                    + f32( 1            ) ) * a;

        Cos = ((((( + f32( 1 / 40320.0  ) ) * a2  // Cosine
                    - f32( 1 / 720.0    ) ) * a2
                    + f32( 1 / 24.0     ) ) * a2
                    - f32( 1 / 2.0      ) ) * a2
                    + f32( 1            ) );      
    }

#if defined( TARGET_XBOX )
    u32 const mask = ( n & 2 ) << ( 32 - 2 );
    *( u32* )( &Sin ) ^= mask;
    *( u32* )( &Cos ) ^= mask;
#else
    if ( n & 2 )
    {
        Sin = -Sin;
        Cos = -Cos;
    }
#endif
}

//==========================================================================
inline f32 x_sqrt( f32 a )
{
    // Make sure value is not negative
    ASSERT( a >= 0.0f );
    // sqrt(0) = zero, and any negative numbers will result in zero.
    if( a <= 0.0f )
        return 0.0f;

#if defined( TARGET_PS2 )

    register f32 Result;

    asm __volatile__
    ("
        sqrt.s  %0, %1
    " : "=f" (Result) : "f" (a) );
    
    return Result;

#elif defined( TARGET_DOLPHIN )

    register f32 Result;
    register f32 kHalf  = 0.5f;
    register f32 kTwo   = 2.0f;
    register f32 kThree = 3.0f;

    // Newton's approximation x 1
    // 1.0/sqrt(a) == estimation 'E'
    // InvSqrt = (E/2)(3 - (a * E * E))
    //
    // Newton's approximation x 1
    // 1.0/InvSqrt == estimation 'E'
    // Result = (E*2) - (E * E * InvSqrt)
    //
    asm __volatile__
    ("
        ps_rsqrte  2, %4            #f2 = 1/sqrt(a) -> 'E'
        ps_mul     0,  2,  2        #f0 = (E*E)
        ps_mul     1,  2, %1        #f1 = (E*0.5)
        ps_mul     0, %4,  0        #f0 = (a * (E*E))
        ps_sub     0, %3,  0        #f0 = (3 - (a * (E*E)))
        ps_mul     3,  0,  1        #f3 = (E*0.5) * (3 - (a * (E*E)))  -> InvSqrt

        ps_res     2,  3            #f2 = 1 / f3  -> 1/InvSqrt == 'E'
        ps_mul     0,  2,  2        #f0 = (E*E)
        ps_mul     1,  2, %2        #f1 = (E*2)
        ps_mul     0,  0,  3        #f0 = (E*E * InvSqrt)
        ps_sub    %0,  1,  0        #Result = (E*2) - (E*E * InvSqrt)
    "
    : "=f" (Result)
    : "f" (kHalf), "f" (kTwo), "f" (kThree), "f" (a)
    : "fr0", "fr1", "fr2", "fr3"
    );

    return Result;

#elif defined(TARGET_PC) || defined(TARGET_XBOX)
	//return sqrtf(a);
	//	This was a tiny bit faster than the sqrtf(a)
	//	I wasn't getting a warning for no return, so I didn't know what warning to turn off.
	__asm
	{
		fld	a
		fsqrt
	}

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
}

//==========================================================================

inline f32 x_1oversqrt( f32 a )
{
    ASSERT( a > 0.0f );

    if( a <= 0.0f )
        return 0.0f;
//        a = 0.001f;

#if defined( TARGET_PS2 )
    f32 Result __attribute__ ((aligned(4)));
    f32 One    __attribute__ ((aligned(4)));
    Result = a;
    One = 1.0f;

    asm __volatile__
    ("
        mtc1    %1, $f2
        mtc1    %0, $f1
        rsqrt.s $f1, $f2, $f1
        mfc1    %0, $f1
    ": "+r" (Result) : "r" (One) );
    
    return Result;

#elif defined( TARGET_DOLPHIN )

    register f32 Result;
    register f32 kHalf  = 0.5f;
    register f32 kThree = 3.0f;

    // Newton's approximation x 1
    // 1.0/sqrt(a) == estimation 'E'
    // InvSqrt = (E/2)(3 - (a * E * E))
    //
    asm __volatile__
    ("
        ps_rsqrte  2, %3            #f2 = 1/sqrt(a) -> 'E'
        ps_mul     0,  2,  2        #f0 = (E*E)
        ps_mul     1,  2, %1        #f1 = (E*0.5)
        ps_mul     0, %3,  0        #f0 = (a * (E*E))
        ps_sub     0, %2,  0        #f0 = (3 - (a * (E*E)))
        ps_mul    %0,  0,  1        #Result = (E*0.5) * (3 - (a * (E*E)))
    "
    : "=f" (Result)
    : "f" (kHalf), "f" (kThree), "f" (a)
    : "fr0", "fr1", "fr2"
    );
 
    return Result;

#else
    return( 1.0f / x_sqrt(a) );
#endif
}