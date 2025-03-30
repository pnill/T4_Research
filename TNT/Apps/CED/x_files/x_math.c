///////////////////////////////////////////////////////////////////////////
//
//  X_MATH.C
//
///////////////////////////////////////////////////////////////////////////

#ifndef X_MATH_H
#include "x_math.h"
#endif

#ifndef X_DEBUG_H
#include "x_debug.h"
#endif

#ifndef X_PLUS_H
#include "x_plus.h"
#endif

///////////////////////////////////////////////////////////////////////////
// There is a large block of miscellaneous comments in the x_math.h file. 
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// LOCAL STORAGE
///////////////////////////////////////////////////////////////////////////

#ifndef TARGET_SDC

s16  x_SinTable[4096];

#endif

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//  INIT / KILL
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void    x_InitMATH (void)
{
#ifndef TARGET_SDC
    s32 i;

    // Initialize the sin table.
    for (i=0; i<4096; i++)
        x_SinTable[i] = (s16)( x_sin((f32)i * (PI/2048)) * 32000.0f );
#endif
}

void    x_KillMATH (void)
{
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//  STANDARD MATH
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////


//==========================================================
// x_sqr
//==========================================================
f32 x_sqr( f32 x )
{
    return x * x;
}

//==========================================================
// x_fabs
//==========================================================
#ifndef TARGET_SDC

f32 x_fabs( f32 x )
{
    if (x >= 0) return x;
    return -x;
}

#endif

//==========================================================
// x_abs
//==========================================================
s32 x_abs( s32 x )
{
    if (x >= 0) return x;
    return -x;
}

//==========================================================
// x_ldexp
//----------------------------------------------------------
//==========================================================
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
    if ( exponent != 0 )
    {
        // X is nonzero and not denormalized.  
        if (exponent <= 1024 - (-1021) + 1)
        {
	        // X is finite.  When EXP < 0, overflow is actually underflow.  
	        exponent += exp;

	        if (exponent != 0)
	        {
	            if (exponent <= 1024 - (-1021) + 1)
		        {
		            // In range.  
		            u->s.u_exp = exponent;
		            return x;
		        }

                //------------------------------------------------
                // errno = ERANGE
                //------------------------------------------------
	            if (exp >= 0)
overflow:
		        {
                    s32 negative = u->s.u_sign;                    
                    x = 1.7976931348623158e+308; // x = HUGE_VAL; 
		            u->s.u_sign = negative;

                    // errno = ERANGE;
		            return x;
		        }

                //------------------------------------------------
                // Underflow.
                //------------------------------------------------
                // #define DBL_MANT_DIG    53
                if (exponent <= (u32)(-(53 + 1)))
		        {
		            // Underflow.  
		            s32 negative = u->s.u_sign;
		            x = 0.0L;
		            u->s.u_sign = negative;

		            // errno = ERANGE;
		            return x;
		        }
	        }

	        // Gradual underflow.  
	        u->s.u_exp = 1;
	        x *= x_ldexp (1.0L, (s32) exponent - 1);

	        if (u->s.u_mant1 == 0 && u->s.u_mant2 == 0)
            {
	            // Underflow.  
	            //errno = ERANGE;
            }
	        return x;
        }

        // X is +-infinity or NaN.  
        if (u->s.u_mant1 == 0 && u->s.u_mant2 == 0)
  	    {
            // X is +-infinity.  
            if (exp >= 0)
            {
                goto overflow;
            }
            else
	        {
                // (infinity * number < 1).  With infinite precision,
                // (infinity / finite) would be infinity, but otherwise it's
                // safest to regard (infinity / 2) as indeterminate.  The
		        // infinity might be (2 * finite).  
                s32 negative = u->s.u_sign;

                // Create a NaN.  
                //x = __infnanl(EDOM);

                u->s.u_sign = negative;
                //errno = EDOM;
                return x;
	        }
	    }

        // X is NaN.  
        //errno = EDOM;
        return x;
    }

    // X is zero or denormalized.  
    if (u->s.u_mant1 == 0 && u->s.u_mant2 == 0)
    {
        // X is +-0.0. 
        return x;
    }

    // X is denormalized.
    // Multiplying by 2 ** LDBL_MANT_DIG normalizes it;
    // we then subtract the LDBL_MANT_DIG we added to the exponent.  
    // #define DBL_MANT_DIG    53
    return x_ldexp(x * x_ldexp(1.0L, 53), exp - 53);
}

//==========================================================
// x_frexpl
//----------------------------------------------------------
// returns the mantissa. If x is 0, the function returns 0 for both the
// mantissa and the exponent. There is no error return.
//
// The frexp function breaks down the floating-point value (x) into a mantissa
// (m) and an exponent (n), such that the absolute value of m is greater than or
// equal to 0.5 and less than 1.0, and x = m*2n. The integer exponent n is stored
// at the location pointed to by expptr.
//==========================================================
f64 x_frexp( f64 value, s32* eptr )
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

    ASSERT( eptr );

	if (value) 
    {
		u.v = value;
		*eptr = u.s.u_exp - 1022;
		u.s.u_exp = 1022;

		return u.v;
	} 

	*eptr = 0;
	return (f64)0;
}

//==========================================================
// x_log
//---------------------------------------------------------- 
// Calculates logarithms (LN)
//==========================================================
f64 x_log( f64 x )
{
    f64 Rz, f, z, w, znum, zden, xn,a,b;
    s32 n;

    if (x <= 0.0) 
    {
        return -1.7976931348623158e+308; // HUGE
    }

    f = x_frexp(x, &n);

    if (f > 0.70710678118654752440) 
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
    a = ((-0.78956112887491257267e+0*w +0.16383943563021534222e+2)*w -0.64124943423745581147e+2);
    b = (((w -0.35667977739034646171e+2)*w +0.31203222091924532844e+3)*w -0.76949932108494879777e+3);

    Rz = z + z * (w * a / b );
    xn = n;

    return (xn*-2.121944400546905827679e-4 + Rz) + xn*0.693359375;
}

//==========================================================
// log10
//----------------------------------------------------------
// Calculates logarithms of base 10
//==========================================================
f64 x_log10( f64 x )
{
    return x_log(x)*0.43429448190325182765;
}

//==========================================================
// x_exp
//----------------------------------------------------------
// The exp function returns the exponential value of the floating-point 
// parameter, x, if successful. 
//==========================================================
f64 x_exp( f64 x )
{
    s32 n;
    f64 xn, g, r, z,p,q;

//    if (x > LOGHUGE) 
//  {
//        errno = ERANGE;
//        return 1.7976931348623158e+308; // HUGE;
//  }
//    if (x < LOGTINY) 
//  {
//        errno = ERANGE;
//        return 0.0;
//  }

    if ( (x > -2.710505e-20 ) && (x < 2.710505e-20 ) ) return 1.0;

    z = x * 1.4426950408889634074;
    n = (s32)z;
    if (n < 0)      --n;
    if (z-n >= 0.5) ++n;

    xn = n;
    g = ((x - xn*0.693359375)) + xn*2.1219444005469058277e-4;
    z = g*g;

    p = ((0.165203300268279130e-4*z + 0.694360001511792852e-2)*z + 0.249999999999999993e+0);
    q = ((0.495862884905441294e-3*z + 0.555538666969001188e-1)*z + 0.500000000000000000e+0);

    r = p*g;
    r = 0.5 + r/(q-r);

    return x_ldexp(r,n+1);
}


//==========================================================
// x_pow
//----------------------------------------------------------
// pow returns the value of x^y.
//==========================================================
f64 x_pow( f64 x, f64 y )
{
    f64 logx;

    if ( y == 0 ) return 1;
    if ( x == 0.0 ) 
    {
        if ( y < 0.0 )  
        {
            return 1.7976931348623158e+308; //HUGE;
        }

        return 0.0;
    }

    // Handle the nasty case where x in negative
    if ( x < 0 )
    {
        s32 t;
        f64 j;

        // handle when y is integer
        t = (s32)y;        
        if ( t == y )
        {
            logx  =  x_log(-x);
            logx *=  y;

            if (t&1) return( -x_exp(logx) );
            return( x_exp(logx) );
        }

        // handle when y is fractional
        t = (s32)(1/y);
        j = y - t*y;

        // n mush be an integer where n = 1/y
        if ( !((j > -2.710505e-20 ) && (j < 2.710505e-20 )) )
            return -1.7976931348623158e+308; //HUGE;

        // if it is even then blow up
        if ((t&1) == 0) 
            return -1.7976931348623158e+308; //HUGE;

        logx  = x_log(-x);
        logx *= y;
        return( -x_exp(logx) );
    }

    // Handle the happy case of x is positive
    logx  = x_log(x);
    logx *= y;

    return( x_exp(logx) );
}

//==========================================================
// Recursive Floor and Ceil routines
//==========================================================
f64 RecursiveCeil  ( f64 x, s32 Depth );
f64 RecursiveFloor ( f64 x, s32 Depth );

f64 RecursiveFloor( f64 x, s32 Depth )
{
    f64 y;

    ASSERT( Depth < 128 );

    // if (NaN || already an even integer )
    if ( x != x || x >= 4503599627370496.0) return x;

    if (x < (f64)0) return -RecursiveCeil( -x, Depth+1 );

    // now 0 <= x < L
    y = 4503599627370496.0+x;           // destructive store must be forced
    y -= 4503599627370496.0;            // an integer, and |x-y| < 1
    return x < y ? y-(f64)1 : y;
}

f64 RecursiveCeil( f64 x, s32 Depth )
{
    f64 y;

    ASSERT( Depth < 128 );

    // if (NaN || already an even integer )
    if ( x != x || x >= 4503599627370496.0) return x;

    if (x < (f64) 0) return -RecursiveFloor( -x, Depth+1 );

    // now 0 <= x < L
    y  = 4503599627370496.0 + x;        // destructive store must be forced
    y -= 4503599627370496.0;            // an integer, and |x-y| < 1

    return x > y ? y+(f64)1 : y;
}

//==========================================================
// ceil and floor
//==========================================================

f64 x_floor( f64 x )
{
    return RecursiveFloor( x, 0 );
}

f64 x_ceil( f64 x )
{
    return RecursiveCeil( x, 0 );
}

//==========================================================
// fmod
//----------------------------------------------------------
// The fmod function calculates the floating-point remainder
// f of x / y such that x = i * y + f, where i is an integer,
// f has the same sign as x, and the absolute value of f is
// less than the absolute value of y.
//==========================================================
f64 x_fmod( f64 x, f64 y )
{
    f64 quot = x/y;
    f64 mod;

    mod = x - (quot < 0.0 ? x_ceil(quot) : x_floor(quot)) * y;
    ASSERT( ABS(mod) < ABS(y) );
    return mod;
}

//==========================================================
// modf
//----------------------------------------------------------
// The modf function breaks down the floating-point value x
// into fractional and integer parts, each of which has the
// same sign as x. The signed fractional portion of x is
// returned. The integer portion is stored as a floating-
// point value at whole.
//==========================================================
f64 x_modf( f64 x, f64* whole )
{
    *whole = x_floor(x);
    return x - *whole;
}

//========================================================
// x_flpr
//========================================================
f32 x_flpr( f32 x, f32 Range )
{
    ASSERT (Range > 0);

    x = (f32)x_fmod( x, Range );

    if (x < 0) x += Range;

    return x;
}

//===============================================
// x_sqrt
//===============================================
#ifndef TARGET_SDC

f32 x_sqrt( f32 x )
{
/////////////////////////////////////////////////
// N64
/////////////////////////////////////////////////
#ifdef TARGET_N64
    ASSERT(x>=0);               // catch bad input
    if (x<=0) return 0.0f;      // protect from crash
    return sqrtf(x);            // compute sqrt

#elif defined(TARGET_PSX)

	// this bit of code increases the performance of the x_sqrt
	// function by about 40 times (2.5 ms vs .06 ms) on the PSX

	s32 fx;

	// protect from crash
	if ( x <= 0 ) 
	{
		return 0.0f;      // protect from crash
	}

	fx = x * (1 << 12);
	x  = SquareRoot12(fx) * (1.0f / (1 << 12));

	return x;

#else

/////////////////////////////////////////////////
// SOFTWARE
//-----------------------------------------------
// WARNING: It uses doubles inside
// Precision: 11.05 E-10
// Range: 0 to e308
/////////////////////////////////////////////////

    f64 mantisa;
    s32 exponent;
    f64 re;
    f64 Answer, p, q;

    //---------------------------------------------------------------------
    // There is no answer for negative numbers.
    //---------------------------------------------------------------------
    ASSERT( x >= 0 );
    if ( x <= 0 ) return 0;

    //---------------------------------------------------------------------
    // Get the mantisa and the exponent. 
    // The matisa will be from a range of 0.5 to 1.0.
    //---------------------------------------------------------------------
    mantisa = x_frexp( x, &exponent );

    // odd or even?
    re = exponent & 1 ? 1.4142135623730950488f : 1.0;

    // divide exponent by 2
    exponent >>= 1;

    //---------------------------------------------------------------------
    // Multiply by sqrt(2) if mantissa < 0.7
    //---------------------------------------------------------------------
    if ( mantisa < .7071067812f )
    {
        mantisa *= 1.4142135623730950488;

        if (re > 1.0) re = 1.189207115002721;
        else re = 1.0f / 1.18920711500271;
    }

    //---------------------------------------------------------------------
    // Polynomial approximation from: 
    // COMPUTER APPROXIMATIONS by Hart, J.F. et al. 1968
    //---------------------------------------------------------------------
    p =             0.54525387389085 +
        mantisa * (13.65944682358639 +
        mantisa * (27.090122712655   +
        mantisa *   6.43107724139784));

    q =             4.17003771413707 +
        mantisa * (24.84175210765124 +
        mantisa * (17.71411083016702 +
        mantisa ));

    mantisa = p / q;

    //---------------------------------------------------------------------
    // Regroup the mantisa and exponent into one number
    //---------------------------------------------------------------------
    Answer  = x_ldexp( mantisa, exponent );
    Answer *= re;

    return( (f32)Answer );

#endif

}

#endif
//===============================================
// x_sqrt
//-----------------------------------------------
// SDC - implementation is the SDC assembly file.
//===============================================
#ifndef TARGET_SDC

f32 x_1oversqrt( f32 x )
{
    ASSERT( x >= 0 );
    if (x==0) x=0.001f;
    return 1/x_sqrt(x);
}

#endif


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//  TRIGONOMETRIC MATH
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////


//===============================================
// x_XYZToZXYRot
//===============================================
void x_XYZToZXYRot( radian3d* Rotations )
{
    f32 M20,M22,M01,M11,M02,M00;
    f32 sx,cx,sy,cy,sz,cz;
    radian RX,RY,RZ;
    f32 s=0,CX,cxcz,sxsz;

    ASSERT(Rotations);

    x_sincos(Rotations->X,&sx,&cx);
    x_sincos(Rotations->Y,&sy,&cy);
    x_sincos(Rotations->Z,&sz,&cz);

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

    Rotations->X = RX;
    Rotations->Y = RY;
    Rotations->Z = RZ;
}

//===============================================
// x_XYZToXZYRot
//===============================================
void x_XYZToXZYRot( radian3d* Rotations )
{
    f32 M01,M21,M11,M02,M00,M12,M10;
    f32 sx,cx,sy,cy,sz,cz;
    radian RX,RY,RZ;
    f32 s=0,CZ,sysz;

    ASSERT(Rotations);

    x_sincos(Rotations->X,&sx,&cx);
    x_sincos(Rotations->Y,&sy,&cy);
    x_sincos(Rotations->Z,&sz,&cz);

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

    Rotations->X = RX;
    Rotations->Y = RY;
    Rotations->Z = RZ;
}

//===============================================
// x_XZYToXYZRot
//===============================================
void x_XZYToXYZRot( radian3d* Rotations )
{
    f32 M02,M12,M22,M01,M00,M21,M11;
    f32 sx,cx,sy,cy,sz,cz;
    radian RX,RY,RZ;
    f32 s=0,CY,sysz;

    ASSERT(Rotations);

    x_sincos(Rotations->X,&sx,&cx);
    x_sincos(Rotations->Y,&sy,&cy);
    x_sincos(Rotations->Z,&sz,&cz);

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

    Rotations->X = RX;
    Rotations->Y = RY;
    Rotations->Z = RZ;
}

//===============================================
// x_ZXYToXYZRot
//===============================================
void x_ZXYToXYZRot( radian3d* Rotations )
{
    f32 M12,M22,M01,M00,M21,M11;
    f32 sx,cx,sy,cy,sz,cz;
    radian RX,RY,RZ;
    f32 s=0,CY,cycz,sysz;

    ASSERT(Rotations);

    x_sincos(Rotations->X,&sx,&cx);
    x_sincos(Rotations->Y,&sy,&cy);
    x_sincos(Rotations->Z,&sz,&cz);

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

    Rotations->X = RX;
    Rotations->Y = RY;
    Rotations->Z = RZ;
}


//===============================================
// x_asin
//-----------------------------------------------
// Calculates arcsin(x)
// Range: 0 <= x <= 1
// Precision: +/- .000,000,02
//===============================================
radian x_asin( f32 x )
{
    f32 y;
    u32 sign;

    if (x>1.0f)     x = 1.0f;
    if (x<-1.0f)    x = -1.0f;

    sign = 0;

    if (x < 0)
    {
        sign = 1;
        x = -x;
    }

    y = ((((((- 0.0012624911f //  * x      //error: 0.000030
              + 0.0066700901f )// * x      //error: 0.000133
              - 0.0170881256f ) * x      //error: 0.000336
              + 0.0308918810f ) * x      //error: 0.000771
              - 0.0501743046f ) * x      //error: 0.002115
              + 0.0889789874f ) * x      //error: 0.009013
              - 0.2145988016f ) * x      //error: > 1
              + 1.5707963050f;

    y = 1.57079632679f - x_sqrt(1.0f - x) * y;

    return (sign ? -y:y);
}

//===============================================
// x_acos
//-----------------------------------------------
// Calculates arccos(x)
// Range: 0 <= x <= 1
// Precision: +/- .000,000,02
//===============================================
radian x_acos( f32 x )
{
    return PI/2 - x_asin(x);
}

//===============================================
// x_ModAngle
//-----------------------------------------------
// Sets and angle form [0 to 2PI)
//===============================================
radian x_ModAngle( radian x )
{
    if ( ( x  > (4*R_360) ) || ( x < (-4*R_360) ) )
    {
        x = (f32)x_fmod(  x, R_360 );
    }

    while (x > R_360 ) x -= R_360;
    while (x < 0 )     x += R_360;

    ASSERT( (x >= (R_0-0.01f)) && (x < (R_360+0.01f)) );
    return x;
}

//===============================================
// x_ModAngle2
//-----------------------------------------------
// Sets and angle form (-PI to PI]
//===============================================
radian x_ModAngle2( radian x )
{
    x += PI;
    x  = x_ModAngle( x );
    x -= PI;
    
    return x;
}

//===============================================
// x_MinAngleDiff
//-----------------------------------------------
// Finds the shortest distance between two angles
//===============================================
radian x_MinAngleDiff( radian a, radian b )
{
    return x_ModAngle2( a - b );
}


//===============================================
// x_sin
//-----------------------------------------------
// Calculates sin(x), angle x must be in rad.
// Range: -pi/2 <= x <= pi/2
// Precision: +/- .000,000,005
//===============================================
#ifndef TARGET_SDC

f32 x_sin( radian x )
{
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

    if ( y > 1 ) y = 1;

    return(sign < 0 ? -y : y);
}
#endif

//===============================================
// x_sincos
//-----------------------------------------------
// Calculates sin(x) and cos(x), angle x must be in rad.
// Range: -pi/2 <= x <= pi/2
// Precision: +/- .000,000,005
//===============================================

#ifndef TARGET_SDC

void x_sincos( radian x, f32* S, f32* C )
{
    f32 sq,sq2;
    f32 cq,cq2;
    f32 sa,ca;

    ASSERT(S && C);

#if defined (TARGET_PSX)

    // KLUDGE: convert to fixed point and then use PSX native 
    // commands to compute function.

    {
        s32 psx_angle;

	    // input to rsin/rcos is 0 -> 4096 ( where 4096 = 360 degrees)
        psx_angle = (s32)(RAD_TO_DEG(x) * 4096 / 360) % 4096;

        // rsin/rcos return fixed point format 1.19.12
	    *S = rsin(psx_angle) * (1.0f / (1 << 12));
	    *C = rcos(psx_angle) * (1.0f / (1 << 12));

    	return;
    }

#endif

    //----------------------------------------------
    // bring x into range 0 -> 2PI
    //----------------------------------------------
    x = x_ModAngle(x);
    while (x < 0)                        x += 6.28318530717958647692f;
    while (x >= 6.28318530717958647692f) x -= 6.28318530717958647692f;

    //----------------------------------------------
    // choose angles for sin and cosine
    //----------------------------------------------
    if (x >= R_180)
    {
        if (x >= R_270)
        {
            x  -=   R_270;
            sa =    x - R_90;
            ca =    x;
        }
        else
        {
            x -=    R_180;
            sa =    -x;
            ca =    x - R_90;
        }
    }
    else
    {
        if (x >= R_90)
        {
            x -=    R_90;
            sa =    R_90 - x;
            ca =    -x;
        }
        else
        {
            sa =    x;
            ca =    R_90 - x;
        }
    }

    //----------------------------------------------
    // solve sin
    //----------------------------------------------
    sq  = sa;
    sq2 = sq * sq;
    *S = (((((  + ( 1/362880.0f ) ) * sq2       //error: 0.000014
                - ( 1/5040.0f   ) ) * sq2       //error: 0.000544
                + ( 1/120.0f    ) ) * sq2       //error: 0.013482
                - ( 1/6.0f      ) ) * sq2       //error: 0.210513
                + ( 1.0f        ) ) * sq;       //error: > 1

    //----------------------------------------------
    // solve cos
    //----------------------------------------------
    cq  = ca;
    cq2 = cq * cq;
    *C = (((((  + ( 1/362880.0f ) ) * cq2       //error: 0.000014
                - ( 1/5040.0f   ) ) * cq2       //error: 0.000544
                + ( 1/120.0f    ) ) * cq2       //error: 0.013482
                - ( 1/6.0f      ) ) * cq2       //error: 0.210513
                + ( 1.0f        ) ) * cq;       //error: > 1

    if (*S < -1) *S = -1;
    if (*S >  1) *S =  1;
    if (*C < -1) *C = -1;
    if (*C >  1) *C =  1;
}

#endif

//===============================================
// x_cos
//-----------------------------------------------
// Calculates cos(x), angle x must be in rad.
// Range: -pi/2 <= x <= pi/2
// Precision: +/- .000,000,005
//===============================================
#ifndef TARGET_SDC

f32 x_cos( radian x )
{
    return x_sin ( PI/2 + x );
}
#endif

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
// x_tan
//-----------------------------------------------
// Calculates tan(x), angle x must be in rad.
// Range: -pi/2 <= x <= pi/2
// Precision: dependent upon sin and cos
//===============================================
f32 x_tan( radian x )
{
    return (x_sin(x) / x_cos(x));
}

//===============================================
// x_atan
//-----------------------------------------------
// Calculates arctan(x)
// Range: -infinite <= x <= infinite (Output -pi/2 to +pi/2)
// Precision: +/- .000,000,04
//===============================================
radian x_atan( f32 x )
{
    f32 xi, q, q2, y;

    xi = (x < 0 ? -x : x);
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

    return(x < 0 ? -y: y);
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
    if (x > -0.000001 && x < 0.000001) x = 0.0;
    if (y > -0.000001 && y < 0.000001) y = 0.0;

    //----------------------------------------------
    // Check if vector has no length
    //----------------------------------------------
    if (x == 0.0 && y == 0.0)
    {
        return 0;
    }

    //----------------------------------------------
    // Check if at 90 or -90 degrees
    //----------------------------------------------
    if (x == 0.0)
    {
        if (y >= 0.0) return PI/2;
        else return -PI/2;
    }

    //----------------------------------------------
    // Check if at 0 or 180 degrees
    //----------------------------------------------
    if (y == 0.0)
    {
        if (x >= 0.0) return 0;
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
    if ( zabs < 0.000001 )
    {
        return SignArray[sign];
    }

    //----------------------------------------------
    // solve regular expresion
    //----------------------------------------------
    z = x_atan(z);

    return z + SignArray[sign];
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//  VECTOR 3D
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

//===================================================
// V3_Zero
//===================================================
void V3_Zero( vector3d* V )
{
    ASSERT(V);

    V->X = V->Y = V->Z = 0;
}

//===================================================
// V3_Set
//===================================================
void V3_Set( vector3d* V, f32 X, f32 Y, f32 Z )
{
    ASSERT(V);

    V->X = X;
    V->Y = Y;
    V->Z = Z;
}

//===================================================
// V3_Get
//===================================================
void V3_Get( vector3d* V, f32* X, f32* Y, f32* Z )
{
    ASSERT(V && X && Y && Z);

    *X = V->X;
    *Y = V->Y;
    *Z = V->Z;
}

//===================================================
// V3_Copy
//===================================================
void V3_Copy( vector3d* Dest, vector3d* Src )
{
    ASSERT(Dest && Src);

    *Dest = *Src;
}

//===================================================
// V3_Add
//===================================================
void V3_Add( vector3d* Dest, vector3d* Va, vector3d* Vb )
{
    ASSERT(Dest);
    ASSERT(Va);
    ASSERT(Vb);

    Dest->X = Va->X + Vb->X;
    Dest->Y = Va->Y + Vb->Y;
    Dest->Z = Va->Z + Vb->Z;
}

//===================================================
// V3_Sub
//===================================================
void V3_Sub( vector3d* Dest, vector3d* Va, vector3d* Vb )
{
    ASSERT(Dest);
    ASSERT(Va);
    ASSERT(Vb);

    Dest->X = Va->X - Vb->X;
    Dest->Y = Va->Y - Vb->Y;
    Dest->Z = Va->Z - Vb->Z;
}

//===================================================
// V3_Scale
//===================================================
void V3_Scale( vector3d* Dest, vector3d* Src, f32 S )
{
    ASSERT(Dest);
    ASSERT(Src);

    Dest->X = Src->X * S;
    Dest->Y = Src->Y * S;
    Dest->Z = Src->Z * S;
}

//===================================================
// V3_Negate
//===================================================
void V3_Negate( vector3d* Dest, vector3d* Src )
{
    ASSERT(Dest);
    ASSERT(Src);

    Dest->X = -Src->X;
    Dest->Y = -Src->Y;
    Dest->Z = -Src->Z;
}

//===================================================
// V3_Distance
//===================================================
f32 V3_Distance( vector3d* Va, vector3d* Vb )
{
    ASSERT(Va);
    ASSERT(Vb);

    return( x_sqrt( (Va->X - Vb->X) * (Va->X - Vb->X) +
                    (Va->Y - Vb->Y) * (Va->Y - Vb->Y) +
                    (Va->Z - Vb->Z) * (Va->Z - Vb->Z) ) );
}

//===================================================
// V3_Dot
//===================================================
f32 V3_Dot( vector3d* Va, vector3d* Vb )
{
    ASSERT(Va);
    ASSERT(Vb);

    return ( Va->X * Vb->X + Va->Y * Vb->Y + Va->Z * Vb->Z );
}

//===================================================
// V3_Cross
//===================================================
void V3_Cross( vector3d* Dest, vector3d* Va, vector3d* Vb )
{
    vector3d Vt;

    ASSERT(Dest);
    ASSERT(Va);
    ASSERT(Vb);

    Vt.X = Va->Y * Vb->Z - Va->Z * Vb->Y;
    Vt.Y = Va->Z * Vb->X - Va->X * Vb->Z;
    Vt.Z = Va->X * Vb->Y - Va->Y * Vb->X;

    V3_Copy( Dest, &Vt );
}

//===================================================
// V3_Length
//===================================================
f32 V3_Length( vector3d* V )
{
    ASSERT(V);

    return (f32)x_sqrt( V->X * V->X + V->Y * V->Y + V->Z * V->Z );
}

//===================================================
// V3_Normalize
//===================================================
void V3_Normalize( vector3d* V )
{
    ASSERT(V);
    V3_Scale( V, V, x_1oversqrt( V->X * V->X + V->Y * V->Y + V->Z * V->Z ) );
}

//===================================================
// V3_ZToVecRot
//---------------------------------------------------
// Returns the angles (Rx, Ry) for given vector3d
//===================================================
void V3_ZToVecRot( vector3d* V, radian* Rx, radian* Ry )
{
    f32 L;

    ASSERT(Rx);
    ASSERT(Ry);
    ASSERT(V);

    L   = (f32)x_sqrt( V->X * V->X + V->Z * V->Z );

    *Ry = (f32) x_atan2( V->X, V->Z );
    *Rx = (f32)-x_atan2( V->Y, L );
}

//===================================================
// V3_VecToZRot
//===================================================
void V3_VecToZRot( vector3d* V, radian* Rx, radian* Ry )
{
    ASSERT(Rx);
    ASSERT(Ry);
    ASSERT(V);

    V3_ZToVecRot( V, Rx, Ry );
    *Rx = - *Rx;
    *Ry = - *Ry;
}

//===================================================
// V3_AngleBetween
//---------------------------------------------------
// Returns the angle between two vectors
//===================================================
radian V3_AngleBetween ( vector3d* Va, vector3d* Vb )
{
    f32 D;
    f32 Dot;

    ASSERT(Va);
    ASSERT(Vb);

    D = V3_Length( Va ) * V3_Length( Vb );

    if ( D == 0 ) return 0;

    Dot = V3_Dot( Va, Vb ) / D;

    if ( Dot > 1 ) Dot = 1;
    else if ( Dot < -1 ) Dot = -1;

    return x_acos( Dot );
}

//===================================================
// V3_Project
//===================================================
void V3_Project ( vector3d* Dest, vector3d* Src, s32 NVerts,
                  f32 PXC0, f32 PXC1, f32 PYC0, f32 PYC1 )
{
    s32 i;

    ASSERT(Dest);
    ASSERT(Src);
    ASSERT(NVerts > 0);
    ASSERT(Dest != Src);

    for (i=0; i<NVerts; i++)
    {
        // Check if Z is in front of 0.01f
        if (Src[i].Z >=  0.01f)
        {
            Dest[i].X = PXC0 + PXC1 * (Src[i].X / Src[i].Z);
            Dest[i].Y = PYC0 + PYC1 * (Src[i].Y / Src[i].Z);
            Dest[i].Z = Src[i].Z;
        }
        else
        {
            // Z is less than 0.01f so check if it's too
            // close to zero

            f32 SZ = Src[i].Z;

            // Snap SZ to 0.01f if it's too close to zero
            if (SZ > -0.01f)
                SZ = 0.01f;

            // Flip Z if it is negative
            if (SZ < 0.0f) 
                SZ = -SZ;

            Dest[i].X = PXC0 + PXC1 * (Src[i].X / SZ);
            Dest[i].Y = PYC0 + PYC1 * (Src[i].Y / SZ);
            Dest[i].Z = SZ;
        }
    }
}

//===================================================
// V3_RotateX
//===================================================
void V3_RotateX( vector3d* Dest, vector3d* V, radian Rx )
{
    f32 S,C;
    vector3d T;

    ASSERT(Dest);
    ASSERT(V);

    x_sincos(Rx,&S,&C);

    T.X = V->X;
    T.Y = ( C * V->Y ) - ( S * V->Z );
    T.Z = ( C * V->Z ) + ( S * V->Y );

    V3_Copy (Dest, &T);
}

//===================================================
// V3_RotateY
//===================================================
void V3_RotateY( vector3d* Dest, vector3d* V, radian Ry )
{
    f32 S,C;
    vector3d T;

    ASSERT(Dest);
    ASSERT(V);

    x_sincos(Ry,&S,&C);

    T.X = ( C * V->X ) + ( S * V->Z );
    T.Y = V->Y;
    T.Z = ( C * V->Z ) - ( S * V->X );

    V3_Copy (Dest, &T);
}

//===================================================
// V3_RotateZ
//===================================================
void V3_RotateZ( vector3d* Dest, vector3d* V, radian Rz )
{
    f32     S,C;
    vector3d T;

    ASSERT(Dest);
    ASSERT(V);

    x_sincos(Rz,&S,&C);

    T.X = ( C * V->X ) - ( S * V->Y );
    T.Y = ( C * V->Y ) + ( S * V->X );
    T.Z = V->Z;

    V3_Copy (Dest, &T);
}
/*
//===================================================
// V3_RotateAboutAxis
//===================================================
void V3_RotateAboutAxis( vector3d* Dest, vector3d* V,
                         vector3d* AxisStart, vector3d* AxisEnd, radian R )
{
    vector3d Normal, Vt;
    matrix4  Mat;

    // Check parameters
    ASSERT(Dest);
    ASSERT(AxisStart);
    ASSERT(AxisEnd);
    ASSERT(V);
    ASSERT(Dest != V); // lets keep it simple

    // Decide direction of the axis
    V3_Sub( &Normal, AxisEnd, AxisStart );

    // Normalize the vector
    V3_Normalize( &Normal );

    // Build the matrix
    M4_SetupRotation( &Mat, &Normal, R );

    // Make sure to take the vector back to where it cames from
    M4_SetTranslation( &Mat, AxisStart );

    // Move Vertor to the origin
    V3_Sub( &Vt, V, AxisStart );

    // Transform Vector
    V3_Transform ( Dest, &Vt, 1, &Mat );
}
*/
//===================================================
// V3_ClipLineSegment
//===================================================
void V3_ClipLineSegment( vector3d* CV, vector3d* V0, vector3d* V1,
                         f32 NX, f32 NY, f32 NZ, f32 D )
{
    vector3d    dp;
    f32         Numerator;
    f32         Denominator;
    f32         t;

    ASSERT(CV && V0 && V1);

    // compute delta vector from p0 -> p1
    dp.X = V1->X - V0->X;
    dp.Y = V1->Y - V0->Y;
    dp.Z = V1->Z - V0->Z;

    // compute parametric t-value of intersection with plane
    Numerator   = -(NX*V0->X + NY*V0->Y + NZ*V0->Z + D);
    Denominator = NX*dp.X + NY*dp.Y + NZ*dp.Z;

    // if Denominator is close to 0, then V0->V1 is parallel to plane.
    // Use t=0.5 as approximation.
    if (Denominator < 0.001 && Denominator > -0.001) t=0.5f;
    else t = (Numerator / Denominator);

    // confirm that points straddle plane
    //ASSERT(t<=1.0001 && t>=-0.0001);

    // compute coordinates of clipped point
    CV->X = V0->X + t*dp.X;
    CV->Y = V0->Y + t*dp.Y;
    CV->Z = V0->Z + t*dp.Z;
}

//===================================================
// V3_ClipLineSegment
//---------------------------------------------------
// plane is described as ((X*NX + Y*NY + Z*NY + D) == 0)
//===================================================
xbool V3_ClipPoly( vector3d* Dest,
                  s32*      NDest,
                  vector3d* Src,
                  s32       NSrc,
                  f32       NX,
                  f32       NY,
                  f32       NZ,
                  f32       D )
{

    vector3d*   p0;
    vector3d*   p1;
    vector3d*   cp;
    s32         NewCount=0;
    xbool        p0in,p1in;
    s32         i;
    xbool        AnythingClipped=FALSE;

    ASSERT(Dest);
    ASSERT(NDest);
    ASSERT(Src);

    // Check for simple return;
    *NDest = 0;
    if (NSrc==0) return FALSE;

    // Get side of plane point is on
    p0   = &Src[NSrc-1];
    p0in = ( (p0->X*NX + p0->Y*NY + p0->Z*NZ + D) >= 0);

    // Loop through points
    for (i=0; i<NSrc; i++)
    {
        // Get side of plane point is on
        p1   = &Src[i];
        p1in = ( (p1->X*NX + p1->Y*NY + p1->Z*NZ + D) >= 0);

        // Check if we include this point
        if (p0in)
        {
            Dest[NewCount] = *p0;
            NewCount++;
        }

        // Check if we need to clip these points
        if (p0in!=p1in)
        {
            AnythingClipped = TRUE;

            // get new point in destination for clipped point
            cp = &Dest[NewCount];
            NewCount++;

            V3_ClipLineSegment(cp,p0,p1,NX,NY,NZ,D);
        }

        // Move to next line segment
        p0   = p1;
        p0in = p1in;
    }

    // Return new count to user
    *NDest = NewCount;
    return AnythingClipped;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//  MATRIX4
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

//===================================================
// M4_Copy
//===================================================
void M4_Copy( matrix4* Dest, matrix4* Src )
{
    ASSERT(Dest);
    ASSERT(Src);

    *Dest = *Src;
}

//===================================================
// M4_Clear
//===================================================
void M4_Zero( matrix4* Dest )
{
    ASSERT(Dest);
    x_memset(Dest, 0, sizeof(matrix4));
}

//===================================================
// M4_Set
//===================================================
void M4_Set( matrix4* M, float M11, float M21, float M31, float M41,
                         float M12, float M22, float M32, float M42,
                         float M13, float M23, float M33, float M43,
                         float M14, float M24, float M34, float M44 )
{
    M->M[1][0] = M11;
    M->M[2][0] = M21;
    M->M[3][0] = M31;
    M->M[4][0] = M41;

    M->M[1][1] = M12;
    M->M[2][1] = M22;
    M->M[3][1] = M32;
    M->M[4][1] = M42;

    M->M[1][2] = M13;
    M->M[2][2] = M23;
    M->M[3][2] = M33;
    M->M[4][2] = M43;

    M->M[1][3] = M14;
    M->M[2][3] = M24;
    M->M[3][3] = M34;
    M->M[4][3] = M44;
}

//===================================================
// M4_ClearRotations
//===================================================
void M4_ClearRotations( matrix4* M )
{
    s32 i,j;
    ASSERT(M);

    for (i=0; i<3; i++)
    for (j=0; j<3; j++) M->M[j][i] = 0;

    M->M[0][0] = 1;
    M->M[1][1] = 1;
    M->M[2][2] = 1;
}

//===================================================
// M4_ClearTranslations
//===================================================
void M4_ClearTranslations( matrix4* M )
{
    ASSERT(M);

    M->M[3][0] = 0;
    M->M[3][1] = 0;
    M->M[3][2] = 0;
}

//===================================================
// M4_Identity
//===================================================
void M4_Identity( matrix4* M )
{
    ASSERT(M);

    M4_Zero( M );

    M->M[0][0] = 1;
    M->M[1][1] = 1;
    M->M[2][2] = 1;
    M->M[3][3] = 1;
}

//===================================================
// M4_SetScale
//===================================================
void M4_SetScale( matrix4* M, vector3d* Scale )
{
    ASSERT(M);

    M->M[0][0] = Scale->X;
    M->M[1][1] = Scale->Y;
    M->M[2][2] = Scale->Z;
}

//===================================================
// M4_SetUniScale
//===================================================
void M4_SetUniScale( matrix4* M, f32 Scale )
{
    ASSERT(M);

    M->M[0][0] = Scale;
    M->M[1][1] = Scale;
    M->M[2][2] = Scale;
}


//===================================================
// M4_SetTranslation
//===================================================
void M4_SetTranslation( matrix4* M, vector3d* Translation )
{
    ASSERT(M);

    M->M[3][0] = Translation->X;
    M->M[3][1] = Translation->Y;
    M->M[3][2] = Translation->Z;
}

//===================================================
// M4_SetRotationsXYZ
//===================================================
void M4_SetRotationsXYZ( matrix4* Dest, radian3d* Rotation )
{
    f32 sx, sy, sz;
    f32 cx, cy, cz;
    f32 sxy, cxz, cxsz;
    u16 XYZOn;

    ASSERT(Dest);
    ASSERT(Rotation);

    // determine which angles are non-zero
    XYZOn = 0;
    if (Rotation->X != 0.0f)  XYZOn |= (1<<2);
    if (Rotation->Y != 0.0f)  XYZOn |= (1<<1);
    if (Rotation->Z != 0.0f)  XYZOn |= (1<<0);

    switch (XYZOn)
    {
        // ___
        case 0:     M4_Identity(Dest);
                    return;
        // _Y_
        case 2:     M4_Identity(Dest);
                    x_sincos(Rotation->Y,&sy,&cy);
                    Dest->M[0][0] =  cy;
                    Dest->M[2][0] =  sy;
                    Dest->M[0][2] = -sy;
                    Dest->M[2][2] =  cy;
                    return;
        // X__
        case 4:     M4_Identity(Dest);
                    x_sincos(Rotation->X,&sx,&cx);
                    Dest->M[1][1] =  cx;
                    Dest->M[2][1] = -sx;
                    Dest->M[1][2] =  sx;
                    Dest->M[2][2] =  cx;
                    return;
        // __Z
        case 1:     M4_Identity(Dest);
                    x_sincos(Rotation->Z,&sz,&cz);
                    Dest->M[0][0] =  cz;
                    Dest->M[1][0] = -sz;
                    Dest->M[0][1] =  sz;
                    Dest->M[1][1] =  cz;
                    return;
        // XYZ
        default:    // calc basic values
                    x_sincos(Rotation->X,&sx,&cx);
                    x_sincos(Rotation->Y,&sy,&cy);
                    x_sincos(Rotation->Z,&sz,&cz);
                    sxy     = sx * sy;
                    cxz     = cx * cz;
                    cxsz    = cx * sz;

                    // fill out 3x3 rotations with XYZ no zero
                    Dest->M[0][0] =   cy   * cz;
                    Dest->M[1][0] = ( sxy  * cz ) - cxsz;
                    Dest->M[2][0] = ( cxz  * sy ) + ( sx * sz );
                    Dest->M[0][1] =   cy   * sz;
                    Dest->M[1][1] = ( sxy  * sz ) + cxz;
                    Dest->M[2][1] = ( cxsz * sy ) - ( sx * cz );
                    Dest->M[0][2] =               - sy;
                    Dest->M[1][2] = ( sx   * cy );
                    Dest->M[2][2] = ( cx   * cy );

                    // fill out edges of matrix
                    Dest->M[3][0] = Dest->M[3][1] = Dest->M[3][2] = 0;
                    Dest->M[0][3] = Dest->M[1][3] = Dest->M[2][3] = 0;
                    Dest->M[3][3] = 1;
                    return;
    }
}

//===================================================
// M4_SetRows
//===================================================
void M4_SetRows( matrix4* M, vector3d* Va, vector3d* Vb, vector3d* Vc )
{
    ASSERT(M && Va && Vb && Vc);

    M->M[0][0] = Va->X;
    M->M[1][0] = Va->Y;
    M->M[2][0] = Va->Z;
    M->M[0][1] = Vb->X;
    M->M[1][1] = Vb->Y;
    M->M[2][1] = Vb->Z;
    M->M[0][2] = Vc->X;
    M->M[1][2] = Vc->Y;
    M->M[2][2] = Vc->Z;
}


//===================================================
// M4_SetColumns
//===================================================
void M4_SetColumns( matrix4* M, vector3d* Va, vector3d* Vb, vector3d* Vc )
{
    ASSERT(M && Va && Vb && Vc);

    M->M[0][0] = Va->X;
    M->M[0][1] = Va->Y;
    M->M[0][2] = Va->Z;
    M->M[1][0] = Vb->X;
    M->M[1][1] = Vb->Y;
    M->M[1][2] = Vb->Z;
    M->M[2][0] = Vc->X;
    M->M[2][1] = Vc->Y;
    M->M[2][2] = Vc->Z;
}

//===================================================
// M4_GetColumns
//===================================================
void M4_GetColumns( matrix4* M, vector3d* Va, vector3d* Vb, vector3d* Vc )
{
    ASSERT(M && Va && Vb && Vc);

    Va->X = M->M[0][0];
    Va->Y = M->M[0][1];
    Va->Z = M->M[0][2];
    Vb->X = M->M[1][0];
    Vb->Y = M->M[1][1];
    Vb->Z = M->M[1][2];
    Vc->X = M->M[2][0];
    Vc->Y = M->M[2][1];
    Vc->Z = M->M[2][2];
}

//===================================================
// M4_GetRows
//===================================================
void M4_GetRows( matrix4* M, vector3d* Va, vector3d* Vb, vector3d* Vc )
{
    ASSERT(M && Va && Vb && Vc);

    Va->X = M->M[0][0];
    Va->Y = M->M[1][0];
    Va->Z = M->M[2][0];
    Vb->X = M->M[0][1];
    Vb->Y = M->M[1][1];
    Vb->Z = M->M[2][1];
    Vc->X = M->M[0][2];
    Vc->Y = M->M[1][2];
    Vc->Z = M->M[2][2];
}

//===================================================
// M4_SetRotationsZXY
//===================================================
void M4_SetRotationsZXY ( matrix4* Dest, radian3d* Rotation )
{
    f32 sx,cx;
    f32 sy,cy;
    f32 sz,cz;
    f32 sxsz;
    f32 sxcz;

    ASSERT(Dest && Rotation);

    x_sincos(Rotation->X,&sx,&cx);
    x_sincos(Rotation->Y,&sy,&cy);
    x_sincos(Rotation->Z,&sz,&cz);
    sxsz = sx*sz;
    sxcz = sx*cz;

    // fill out 3x3 rotations with ZXY no zero
    Dest->M[0][0] = cy*cz + sy*sxsz;
    Dest->M[1][0] = sy*sxcz - sz*cy;
    Dest->M[2][0] = cx*sy;
    Dest->M[0][1] = cx*sz;
    Dest->M[1][1] = cx*cz;
    Dest->M[2][1] = -sx;
    Dest->M[0][2] = cy*sxsz - sy*cz;
    Dest->M[1][2] = sy*sz + sxcz*cy;
    Dest->M[2][2] = cx*cy;

    // fill out edges of matrix
    Dest->M[3][0] = Dest->M[3][1] = Dest->M[3][2] = 0;
    Dest->M[0][3] = Dest->M[1][3] = Dest->M[2][3] = 0;
    Dest->M[3][3] = 1;
}

//===================================================
// M4_SetRotationsZXY
//===================================================
void M4_SetRotationsXZY ( matrix4* Dest, radian3d* Rotation )
{
    f32 sx,cx;
    f32 sy,cy;
    f32 sz,cz;
    f32 sxsz;
    f32 sxcz;

    ASSERT(Dest && Rotation);

    x_sincos(Rotation->X,&sx,&cx);
    x_sincos(Rotation->Y,&sy,&cy);
    x_sincos(Rotation->Z,&sz,&cz);
    sxsz = sx*sz;
    sxcz = sx*cz;

    // fill out 3x3 rotations with ZXY no zero
    Dest->M[0][0] = cy*cz;
    Dest->M[1][0] = sy*sx - sz*cy*cx;
    Dest->M[2][0] = cx*sy + cy*sx*sz;
    Dest->M[0][1] = sz;
    Dest->M[1][1] = cx*cz;
    Dest->M[2][1] = cz*sx;
    Dest->M[0][2] = -sy*cz;
    Dest->M[1][2] = cy*sx + cx*sy*sz;
    Dest->M[2][2] = cx*cy - sx*sy*sz;

    // fill out edges of matrix
    Dest->M[3][0] = Dest->M[3][1] = Dest->M[3][2] = 0;
    Dest->M[0][3] = Dest->M[1][3] = Dest->M[2][3] = 0;
    Dest->M[3][3] = 1;
}

//===================================================
// M4_GetRotationsZXY
//===================================================
void M4_GetRotationsZXY ( matrix4* M, radian3d* Rotation )
{
    radian X, Y, Z;
    f32    s, cx;

    ASSERT(M);
    ASSERT(Rotation);

    // Get rotation about X
    s = M->M[2][1];
    if (s >  1) s = 1;
    if (s < -1) s = -1;
    X = (f32)x_asin(-s);

    // Get rotation about X and Z
    cx = (f32)x_cos(X);
    if ((cx > 0.0001f) || (cx < 0.0001f))
    {
        Y = (f32)x_atan2( M->M[2][0], M->M[2][2] );
        Z = (f32)x_atan2( M->M[0][1], M->M[1][1] );
    }
    else
    {
        Z = (f32)x_atan2( M->M[0][2], M->M[0][0] );
        Y = 0;
    }

    Rotation->X = X;
    Rotation->Y = Y;
    Rotation->Z = Z;
}

//===================================================
// M4_GetRotationsXYZ
//===================================================
void M4_GetRotationsXYZ( matrix4* M, radian3d* Rotation )
{
    radian X, Y, Z;
    f32    s, cy;

    ASSERT(M);
    ASSERT(Rotation);

    // Get rotation about Y
    s = M->M[0][2];
    if (s >  1) s = 1;
    if (s < -1) s = -1;
    Y = (f32)x_asin(-s);

    // Get rotation about X and Z
    cy = (f32)x_cos(Y);
    if ((cy > 0) || (cy < 0))
    {
        X = (f32)x_atan2( M->M[1][2], M->M[2][2] );
        Z = (f32)x_atan2( M->M[0][1], M->M[0][0] );
    }
    else
    {
        X = (f32)x_atan2( -M->M[2][1], M->M[1][1] );
        Z = 0;
    }

    Rotation->X = X;
    Rotation->Y = Y;
    Rotation->Z = Z;
}

//===================================================
// M4_GetRotationsXZY
//===================================================
void M4_GetRotationsXZY( matrix4* M, radian3d* Rotation )
{
    radian X, Y, Z;
    f32    s, cz;

    ASSERT(M);
    ASSERT(Rotation);

    // Get rotation about Z
    s = M->M[0][1];
    if (s >  1) s = 1;
    if (s < -1) s = -1;
    Z = (f32)x_asin(-s);

    // Get rotation about X and Y
    cz = (f32)x_cos(Z);
    if ((cz > 0) || (cz < 0))
    {
        X = (f32)x_atan2( -M->M[2][1], M->M[1][1] );
        Y = (f32)x_atan2( -M->M[0][2], M->M[0][0] );
    }
    else
    {
        X = (f32)x_atan2( M->M[1][2], M->M[2][2] );
        Y = 0;
    }

    Rotation->X = X;
    Rotation->Y = Y;
    Rotation->Z = Z;
}


//===================================================
// M4_GetTranslation
//===================================================
void M4_GetTranslation( matrix4* M, vector3d* Translation )
{
    ASSERT(M);
    ASSERT( Translation );
    Translation->X = M->M[3][0];
    Translation->Y = M->M[3][1];
    Translation->Z = M->M[3][2];
}

//===================================================
// M4_Mult
//---------------------------------------------------
// Dest = Left * Right
// This means that Right will happen before Left
//===================================================
/************************************************************************
void M4_Mult( matrix4* Dest, matrix4* Left, matrix4* Right )
{
    matrix4 T;
    s32     i, j;

    ASSERT(Dest);
    ASSERT(Left);
    ASSERT(Right);

    if (Dest == Left)
    {
        T = *Left;
        if (Left == Right) Right = &T;
        Left = &T;
    }
    else
    if (Dest == Right)
    {
        T = *Right;
        Right = &T;
    }

    for (i=0; i<4; i++)
    for (j=0; j<4; j++)
    {
        Dest->M[i][j]  = Left->M[0][j] * Right->M[i][0];
        Dest->M[i][j] += Left->M[1][j] * Right->M[i][1];
        Dest->M[i][j] += Left->M[2][j] * Right->M[i][2];
        Dest->M[i][j] += Left->M[3][j] * Right->M[i][3];
    }
}
************************************************************************/

void M4_Mult( matrix4* Dest, matrix4* Left, matrix4* Right )
{
    matrix4 T;

    s32  i;
    f32* D;
    f32* L;
    f32* R;

    ASSERT(Dest);
    ASSERT(Left);
    ASSERT(Right);

    if (Dest == Left)
    {
        T = *Left;
        if (Left == Right) Right = &T;
        Left = &T;
    }
    else
    if (Dest == Right)
    {
        T = *Right;
        Right = &T;
    }

    D = (f32*)Dest;
    R = (f32*)Right;
    L = (f32*)Left;

    for (i=0; i<4; i++)
    {
        D[0] =   (L[0]*R[0]) + (L[4]*R[1]) + (L[8]*R[2])  + (L[12]*R[3]);
        D[1] =   (L[1]*R[0]) + (L[5]*R[1]) + (L[9]*R[2])  + (L[13]*R[3]);
        D[2] =   (L[2]*R[0]) + (L[6]*R[1]) + (L[10]*R[2]) + (L[14]*R[3]);
        D[3] =   (L[3]*R[0]) + (L[7]*R[1]) + (L[11]*R[2]) + (L[15]*R[3]);
        D+=4;
        R+=4;
    }
}

//===================================================
// M4_PreMultOn
//---------------------------------------------------
// M will happen after Dest: Dest = M * Dest
//===================================================
void M4_PreMultOn( matrix4* Dest, matrix4* M )
{
    ASSERT(Dest);
    ASSERT(M);
    M4_Mult( Dest, M, Dest );
}

//===================================================
// M4_PostMultOn
//--------------------------------------------------
// M will happen before Dest: Dest = Dest * M
//===================================================
void M4_PostMultOn( matrix4* Dest, matrix4* M )
{
    ASSERT(Dest);
    ASSERT(M);
    M4_Mult( Dest, Dest, M );
}

//===================================================
// M4_Translate
//===================================================
void M4_TranslateOn( matrix4* Dest, vector3d* Translation )
{
    ASSERT(Dest);
    ASSERT(Translation);
    Dest->M[3][0] += Translation->X;
    Dest->M[3][1] += Translation->Y;
    Dest->M[3][2] += Translation->Z;
}

//===================================================
// M4_ScaleOn
//===================================================
void M4_ScaleOn( matrix4* Dest, vector3d* Scale )
{
    ASSERT(Dest);
    ASSERT(Scale);

    // Scale rotation vectors
    Dest->M[0][0] *= Scale->X;
    Dest->M[1][0] *= Scale->X;
    Dest->M[2][0] *= Scale->X;
    Dest->M[0][1] *= Scale->Y;
    Dest->M[1][1] *= Scale->Y;
    Dest->M[2][1] *= Scale->Y;
    Dest->M[0][2] *= Scale->Z;
    Dest->M[1][2] *= Scale->Z;
    Dest->M[2][2] *= Scale->Z;

    // Scale translation vectors
    Dest->M[3][0] *= Scale->X;
    Dest->M[3][1] *= Scale->Y;
    Dest->M[3][2] *= Scale->Z;
}

//===================================================
// M4_UniScaleOn
//===================================================
void M4_UniScaleOn( matrix4* M, f32 Scale )
{
    ASSERT(M);

    // Scale rotation vectors
    M->M[0][0] *= Scale;
    M->M[1][0] *= Scale;
    M->M[2][0] *= Scale;
    M->M[0][1] *= Scale;
    M->M[1][1] *= Scale;
    M->M[2][1] *= Scale;
    M->M[0][2] *= Scale;
    M->M[1][2] *= Scale;
    M->M[2][2] *= Scale;

    // Scale translation vectors
    M->M[3][0] *= Scale;
    M->M[3][1] *= Scale;
    M->M[3][2] *= Scale;
}

//===================================================
// M4_RotateXOn
//===================================================
void M4_RotateXOn( matrix4* Dest, radian Rx )
{
    matrix4 T;
    f32     S,C;

    ASSERT(Dest);

    if ( Rx == 0 ) return;

    x_sincos(Rx,&S,&C);

    M4_Identity( &T );

    T.M[1][1] =  C;
    T.M[2][1] = -S;
    T.M[1][2] =  S;
    T.M[2][2] =  C;

    M4_PreMultOn( Dest, &T );
}

//===================================================
// M4_RotateYOn
//===================================================
void M4_RotateYOn( matrix4* Dest, radian Ry )
{
    matrix4 T;
    f32    S,C;

    ASSERT(Dest);

    if ( Ry == 0 ) return;

    x_sincos(Ry,&S,&C);

    M4_Identity( &T );

    T.M[0][0] =  C;
    T.M[2][0] =  S;
    T.M[0][2] = -S;
    T.M[2][2] =  C;

    M4_PreMultOn( Dest, &T );
}

//===================================================
// M4_RotateZOn
//===================================================
void M4_RotateZOn( matrix4* Dest, radian Rz )
{
    matrix4 T;
    f32    S,C;

    ASSERT(Dest);

    if ( Rz == 0 ) return;

    x_sincos(Rz,&S,&C);

    M4_Identity( &T );

    T.M[0][0] =  C;
    T.M[1][0] = -S;
    T.M[0][1] =  S;
    T.M[1][1] =  C;

    M4_PreMultOn( Dest, &T );
}

//===================================================
// M4_RotateXYZOn
//===================================================
void M4_RotateXYZOn( matrix4* Dest, radian3d* Rotation )
{
    matrix4 M;
    ASSERT(Dest);
    M4_SetRotationsXYZ( &M, Rotation );
    M4_PreMultOn( Dest, &M );
}

//===================================================
// M4_SetupSRT
//===================================================
void M4_SetupSRT ( matrix4* Dest, vector3d* Scale,
                                  radian3d* Rotation,
                                  vector3d* Translation )
{
    ASSERT(Dest && Scale && Rotation && Translation);

    // Build rotation matrix
    M4_SetRotationsXYZ(Dest,Rotation);

    // Post mult scaling
    Dest->M[0][0] *= Scale->X;
    Dest->M[0][1] *= Scale->X;
    Dest->M[0][2] *= Scale->X;
    Dest->M[1][0] *= Scale->Y;
    Dest->M[1][1] *= Scale->Y;
    Dest->M[1][2] *= Scale->Y;
    Dest->M[2][0] *= Scale->Z;
    Dest->M[2][1] *= Scale->Z;
    Dest->M[2][2] *= Scale->Z;

    // Pre mult translations
    Dest->M[3][0] += Translation->X;
    Dest->M[3][1] += Translation->Y;
    Dest->M[3][2] += Translation->Z;
}

//===================================================
// M4_Orthogonalize
//===================================================
void M4_Orthogonalize( matrix4* M )
{
    vector3d VX, VY, VZ;

    ASSERT( M );

    VX.X = M->M[0][0]; 
    VX.Y = M->M[0][1];
    VX.Z = M->M[0][2];
    
    VY.X = M->M[1][0]; 
    VY.Y = M->M[1][1];
    VY.Z = M->M[1][2];

    V3_Normalize( &VX );

    V3_Cross    ( &VZ, &VX, &VY );
    V3_Normalize( &VZ );

    V3_Cross    ( &VY, &VZ, &VX );
    V3_Normalize( &VY );

    M4_SetColumns ( M, &VX, &VY, &VZ );
}

//===================================================
// M4_Transpose
//===================================================
void M4_Transpose( matrix4* M )
{
    f32     t;
    s32     i,j;
    ASSERT(M);

    for (i=0; i<=2; i++)
    for (j=i+1; j<=3; j++)
    {
        t           = M->M[j][i];
        M->M[j][i]  = M->M[i][j];
        M->M[i][j]  = t;
    }
}

//===================================================
// M4_SetupRotation
//===================================================
void M4_SetupRotation( matrix4* Dest, vector3d* Normal, radian R )
{
    f32     s;
    f32     c;
    f32     t;
    f32     XX,XY,XZ,YY,YZ,ZZ;
    f32     SX,SY,SZ;
    f32     tX,tY;

    ASSERT(Dest);
    ASSERT(Normal);

    x_sincos(R,&s,&c);
    t = 1 - c ;

    tX = t  * Normal->X;
    tY = t  * Normal->Y;
    XX = tX * Normal->X;
    XY = tX * Normal->Y;
    XZ = tX * Normal->Z;
    YY = tY * Normal->Y;
    YZ = tY * Normal->Z;
    ZZ = t  * Normal->Z * Normal->Z;
    SX = s  * Normal->X;
    SY = s  * Normal->Y;
    SZ = s  * Normal->Z;

    Dest->M[0][0] = XX +  c;
    Dest->M[0][1] = XY + SZ;
    Dest->M[0][2] = XZ - SY;
    Dest->M[1][0] = XY - SZ;
    Dest->M[1][1] = YY +  c;
    Dest->M[1][2] = YZ + SX;
    Dest->M[2][0] = XZ + SY;
    Dest->M[2][1] = YZ - SX;
    Dest->M[2][2] = ZZ +  c;
    Dest->M[3][0] = Dest->M[3][1] = Dest->M[3][2] = (f32)0;
    Dest->M[0][3] = Dest->M[1][3] = Dest->M[2][3] = (f32)0;
    Dest->M[3][3] = (f32)1.0;
}

//===================================================
// M4_SRTInvert
//===================================================
err M4_SRTInvert( matrix4* Dest, matrix4* Src )
{
    matrix4 Temp;
    f32 Determinant;

    ASSERT(Dest && Src);

    //-------------------------------------------------------
    // Handle same destination than source
    //-------------------------------------------------------
    if ( Src == Dest )
    {
        M4_Copy( &Temp, Src );    
        Src = &Temp;
    }

    //-------------------------------------------------------
    // calculate the determinant
    //-------------------------------------------------------
    Determinant = ( Src->M[0][0] * ( Src->M[1][1] * Src->M[2][2] - Src->M[1][2] * Src->M[2][1] ) -
                    Src->M[0][1] * ( Src->M[1][0] * Src->M[2][2] - Src->M[1][2] * Src->M[2][0] ) +
                    Src->M[0][2] * ( Src->M[1][0] * Src->M[2][1] - Src->M[1][1] * Src->M[2][0] ) );

    if ( x_fabs( Determinant ) < 0.0001f ) return ERR_FAILURE;

    Determinant = 1.0f / Determinant;

    //-------------------------------------------------------
    // Find the Inverse of the Source matrix
    //-------------------------------------------------------
    Dest->M[0][0] =  Determinant * ( Src->M[1][1] * Src->M[2][2] - Src->M[1][2] * Src->M[2][1] );
    Dest->M[0][1] = -Determinant * ( Src->M[0][1] * Src->M[2][2] - Src->M[0][2] * Src->M[2][1] );
    Dest->M[0][2] =  Determinant * ( Src->M[0][1] * Src->M[1][2] - Src->M[0][2] * Src->M[1][1] );
    Dest->M[0][3] = 0.0f;

    Dest->M[1][0] = -Determinant * ( Src->M[1][0] * Src->M[2][2] - Src->M[1][2] * Src->M[2][0] );
    Dest->M[1][1] =  Determinant * ( Src->M[0][0] * Src->M[2][2] - Src->M[0][2] * Src->M[2][0] );
    Dest->M[1][2] = -Determinant * ( Src->M[0][0] * Src->M[1][2] - Src->M[0][2] * Src->M[1][0] );
    Dest->M[1][3] = 0.0f;

    Dest->M[2][0] =  Determinant * ( Src->M[1][0] * Src->M[2][1] - Src->M[1][1] * Src->M[2][0] );
    Dest->M[2][1] = -Determinant * ( Src->M[0][0] * Src->M[2][1] - Src->M[0][1] * Src->M[2][0] );
    Dest->M[2][2] =  Determinant * ( Src->M[0][0] * Src->M[1][1] - Src->M[0][1] * Src->M[1][0] );
    Dest->M[2][3] = 0.0f;

    Dest->M[3][0] = -( Src->M[3][0] * Dest->M[0][0] + Src->M[3][1] * Dest->M[1][0] + Src->M[3][2] * Dest->M[2][0] );
    Dest->M[3][1] = -( Src->M[3][0] * Dest->M[0][1] + Src->M[3][1] * Dest->M[1][1] + Src->M[3][2] * Dest->M[2][1] );
    Dest->M[3][2] = -( Src->M[3][0] * Dest->M[0][2] + Src->M[3][1] * Dest->M[1][2] + Src->M[3][2] * Dest->M[2][2] );
    Dest->M[3][3] = 1.0f;

    return ERR_SUCCESS;
}

//===================================================
// M4_Invert
//---------------------------------------------------
// do a GEBS invert of a 4x4 with pivoting
//===================================================
err M4_Invert( matrix4* Dest, matrix4* Src )
{
    f32 work_space[4][8], a;
    s32 j, jr, k, pivot;
    s32 i, row[4];

    ASSERT(Dest && Src);

    // Initialize
    for (j = 0; j < 4; j++)
    {
        for (k = 0; k < 4; k++)
        {
            work_space[j][k] = Src->M[j][k];
            work_space[j][4+k] = 0.0;
        }

        work_space[j][4+j] = 1.0;
        row[j] = j;
    }

    // Eliminate columns
    for (i=0; i < 4; i++)
    {
        // Find pivot
        k = i;
        a = x_fabs( work_space[row[k]][k] );

        for (j = i + 1; j < 4; j++)
        {
            jr = row[j];

            if (a < x_fabs( work_space[jr][i]) )
            {
                k = j;
                a = x_fabs( work_space[jr][i] );
            }
        }

        // Swap PIVOT row with row I
        pivot  = row[k];
        row[k] = row[i];
        row[i] = pivot;

        // Normalize pivot row
        a = work_space[pivot][i];

        if (a == 0.0) return ERR_FAILURE; // Singular

        work_space[pivot][i] = 1.0;

        for (k = i + 1; k < 8; k++) work_space[pivot][k] /= a;

        // Eliminate pivot from all remaining rows
        for ( j = i + 1; j < 4; j++ )
        {
            jr = row[j];
            a = - work_space[jr][i];
            if (a == 0.0) continue;

            work_space[jr][i] = 0.0;

            for (k = i + 1; k < 8; k++)
                work_space[jr][k] += ( a * work_space[pivot][k] );
        }
    }

    // Back solve
    for (i = 3; i > 0; --i)
    {
        pivot = row[i];
        for (j = i-1; j >= 0; --j)
        {
            jr = row[j];
            a = work_space[jr][i];

            for (k = i; k < 8; k++)
                work_space[jr][k] -= ( a * work_space[pivot][k] );
        }
    }

    // Copy inverse back into Dest
    for (j=0; j<4; j++)
    {
        jr = row[j];
        for (k=0; k<4; k++)
        {
            Dest->M[j][k] = work_space[jr][k+4];
        }
    }

    return ERR_SUCCESS;
}

//===================================================
// M4_TransformVerts
//===================================================
void M4_TransformVerts( matrix4* M, vector3d* Dest, vector3d* Src, s32 NVerts )
{
    s32 i;
    f32 X, Y, Z;

    ASSERT(Dest);
    ASSERT(Src);
    ASSERT(M);
    ASSERT(NVerts > 0);

    for ( i = 0; i < NVerts; i++ )
    {
        X = Src[i].X;
        Y = Src[i].Y;
        Z = Src[i].Z;
        Dest[i].X  = M->M[0][0] * X + M->M[1][0] * Y + M->M[2][0] * Z + M->M[3][0];
        Dest[i].Y  = M->M[0][1] * X + M->M[1][1] * Y + M->M[2][1] * Z + M->M[3][1];
        Dest[i].Z  = M->M[0][2] * X + M->M[1][2] * Y + M->M[2][2] * Z + M->M[3][2];
    }
}

                
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//  FINISHED
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

#if 0

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void TestTrig( void )
{
    float i;
    float MaxErr;

    ///////////////////////////////////////////
    MaxErr = 0;
    for (i = -1.0; i < 1.0; i += 0.01f)
    {
        float Err;
        float S1, S2;
        S1 = x_asin (i);
        S2 = (f32)asin (i);

        Err = (f32)fabs(fabs (S1) - fabs (S2));

        if (Err > MaxErr) MaxErr = Err;
    }
    printf (" asin Max Error = %f Degrees(%f)\n", MaxErr, MaxErr*180/PI);

    ///////////////////////////////////////////
    MaxErr = 0;
    for (i = -1.0; i < 1.0; i += 0.01f)
    {
        float Err;
        float S1, S2;
        S1 = x_acos (i);
        S2 = (f32)acos (i);

        Err = (f32)fabs(fabs (S1) - fabs (S2));

        if (Err > MaxErr) MaxErr = Err;
    }
    printf (" acos Max Error = %f Degrees(%f)\n", MaxErr, MaxErr*180/PI);

    ///////////////////////////////////////////
    MaxErr = 0;
    for (i = -PI; i < PI; i += 0.01f)
    {
        float Err;
        float S1, S2;
        S1 = x_sin (i);
        S2 = (f32)sin (i);

        Err = (f32)fabs(fabs (S1) - fabs (S2));

        if (Err > MaxErr) MaxErr = Err;
    }
    printf (" sin Max Error = %f \n", MaxErr);

    ///////////////////////////////////////////
    MaxErr = 0;
    for (i = -PI; i < PI; i += 0.01f)
    {
        float Err;
        float S1, S2;
        S1 = x_cos (i);
        S2 = (f32)cos (i);

        Err = (f32)fabs(fabs (S1) - fabs (S2));

        if (Err > MaxErr) MaxErr = Err;
    }
    printf (" cos Max Error = %f \n", MaxErr);

    ///////////////////////////////////////////
    MaxErr = 0;
    for (i = -10000; i < 10000; i += 1)
    {
        float Err;
        float S1, S2;
        S1 = x_atan (i);
        S2 = (f32)atan (i);

        Err = (f32)fabs(fabs (S1) - fabs (S2));

        if (Err > MaxErr) MaxErr = Err;
    }
    printf (" atan Max Error = %f Degrees(%f)\n", MaxErr, MaxErr*180/PI);

    ///////////////////////////////////////////
    MaxErr = 0;
    for (i = 0; i < 2*PI; i += 0.001f)
    {
        float x = x_cos(i);
        float y = x_sin(i);
        float S1, S2;
        float Err;
        float Scale = (f32)rand();

        x *= Scale;
        y *= Scale;

        S1 = x_atan2 ( y, x );
        S2 = (f32)atan2 ( y, x );

        Err = (f32)fabs(fabs (S1) - fabs (S2));

        if (Err > MaxErr) MaxErr = Err;
    }
    printf (" atan2 Max Error = %f Degrees(%f)\n", MaxErr, MaxErr*180/PI);

    ///////////////////////////////////////////
    MaxErr = 0;
    for (i = 0; i < 10000; i += 1)
    {
        float S1, S2;
        float Err;

        S1 = x_sqrt ( i);
        S2 = (f32)sqrt ( i );

        Err = (f32)fabs(fabs (S1) - fabs (S2));

        if (Err > MaxErr) MaxErr = Err;
    }
    printf (" sqrt Max Error = %f\n", MaxErr);

}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void PrintMatrix (matrix4* M)
{
    int i, j;

    for (j = 0; j < 4; j++)
    {
        for (i = 0; i < 4; i++)
        {
            printf ("%8.4f ", M->M[i][j]);
        }

        printf ("\n");
    }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void PrintVector (vector3d* V)
{
    printf ("%8.4f %8.4f %8.4f\n", V->X, V->Y, V->Z);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void TestMatrix(void)
{
    matrix4  Matrix;
    vector3d T = { 2, 3, 4 };

    printf ("\n M4_Identity( &Matrix );\n");
    M4_Identity( &Matrix );
    PrintMatrix( &Matrix );

    printf ("\n M4_SetTranslation( &Matrix, 2, 3, 4 );\n");
    M4_SetTranslation( &Matrix, &T );
    PrintMatrix( &Matrix );

    printf ("\n M4_ClearTranslations( &Matrix );\n");
    M4_ClearTranslations( &Matrix );
    PrintMatrix( &Matrix );

    {
        vector3d V1 = {0, 0, 30};
        vector3d V2;

        printf ("\n\n");

        M4_RotateYOn( &Matrix, R90 );
        M4_RotateZOn( &Matrix, R90 );
        M4_Translate( &Matrix, &Matrix, 0, 30, 0 );
        PrintMatrix( &Matrix );
        printf ("\n");
        M4_TransformVerts( &Matrix, &V2, &V1, 1 );

        printf ("Original Vector : ");
        PrintVector( &V1 );
        printf ("Tansform Vector : ");
        PrintVector( &V2 );


        printf ("\n M4_Invert( &Matrix, &Matrix );\n");
        M4_Invert( &Matrix, &Matrix );
        PrintMatrix( &Matrix );
        V3_Transform( &V1, &V2, 1, &Matrix );
        printf ("\nTansform Vector : ");
        PrintVector( &V1 );
    }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void main( void )
{

    TestTrig();
    TestMatrix();
}

///////////////////////////////////////////////////////////////////////////

#endif
