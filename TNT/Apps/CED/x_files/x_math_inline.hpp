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
//  suffice for R_90..R_360 if you're smart, and from that you can also get
//  cosine...pretty cool for one table, eh?
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
extern f32 g_SinCos[SIN_TABLE_SIZE+1];

////////////////////////////////////////////////////////////////////////////
// INLINE SINE/COSINE FUNCTIONS
////////////////////////////////////////////////////////////////////////////

inline f32 x_fastsin( radian R )
{
    s32 Lookup = (s32)(R * SIN_TABLE_MULT);

    //---   get the angle within a proper range for lookup
    Lookup = Lookup & (SIN_TABLE_SIZE*4-1);

    //---   perform the lookup based on the ranges 0..90,90..180,180..270,270..360
    if ( Lookup < SIN_TABLE_SIZE )          return g_SinCos[Lookup];
    else if ( Lookup < SIN_TABLE_SIZE*2 )   return g_SinCos[SIN_TABLE_SIZE*2-Lookup];
    else if ( Lookup < SIN_TABLE_SIZE*3 )   return -g_SinCos[Lookup - SIN_TABLE_SIZE*2];
    else                                    return -g_SinCos[SIN_TABLE_SIZE*4 - Lookup];
}

//==========================================================================

inline f32 x_sin( radian R )
{
	return x_fastsin( R );
}

//==========================================================================

inline f32 x_fastcos( radian R )
{
    // cos(x) = sin(x+R_90)
    s32 Lookup = (s32)(R * SIN_TABLE_MULT) + SIN_TABLE_SIZE;

    //---   get the angle within a proper range for lookup
    Lookup = Lookup & (SIN_TABLE_SIZE*4-1);

    //---   perform the lookup based on the ranges 0..90,90..180,180..270,270..360
    if ( Lookup < SIN_TABLE_SIZE )          return g_SinCos[Lookup];
    else if ( Lookup < SIN_TABLE_SIZE*2 )   return g_SinCos[SIN_TABLE_SIZE*2-Lookup];
    else if ( Lookup < SIN_TABLE_SIZE*3 )   return -g_SinCos[Lookup - SIN_TABLE_SIZE*2];
    else                                    return -g_SinCos[SIN_TABLE_SIZE*4 - Lookup];
}

//==========================================================================

inline f32 x_cos( radian R )
{
	return x_fastcos( R );
}

//==========================================================================

inline void x_fastsincos( radian R, f32& rSin, f32& rCos )
{
    rSin = x_fastsin( R );
    rCos = x_fastcos( R );
}

//==========================================================================

inline void x_sincos( radian R, f32& rSin, f32& rCos )
{
    rSin = x_sin( R );
    rCos = x_cos( R );
}
