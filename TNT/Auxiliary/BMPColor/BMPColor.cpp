////////////////////////////////////////////////////////////////////////////
//
// BMPColor.cpp - routines for manipulating bitmap colors
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_math.hpp"

#include "BMPColor.h"


////////////////////////////////////////////////////////////////////////////
// INTERNAL STRUCTS
////////////////////////////////////////////////////////////////////////////

struct SChromaticities
{
    f32 rx;
    f32 ry;
    f32 gx;
    f32 gy;
    f32 bx;
    f32 by;
};

struct SWhitePoint
{
    f32 wx;
    f32 wy;
};


////////////////////////////////////////////////////////////////////////////
// STATIC CONST ARRAYS
////////////////////////////////////////////////////////////////////////////

static
const SChromaticities s_ColorSpaceChromas[COLORSPACE_COUNT] =
{
    //  red-x     red-y   green-x   green-y    blue-x   blue-y 
    { 0.625f,   0.340f,   0.280f,   0.595f,   0.155f,   0.070f },   // Apple RGB
    { 0.630f,   0.340f,   0.310f,   0.595f,   0.155f,   0.070f },   // SMPTE-C
    { 0.640f,   0.330f,   0.300f,   0.600f,   0.150f,   0.060f },   // sRGB
    { 0.640f,   0.330f,   0.290f,   0.600f,   0.150f,   0.060f },   // PAL/SECAM
    { 0.630f,   0.340f,   0.295f,   0.605f,   0.155f,   0.077f },   // Color Match RGB
    { 0.640f,   0.330f,   0.210f,   0.710f,   0.150f,   0.060f },   // Adobe RGB
    { 0.670f,   0.330f,   0.210f,   0.710f,   0.140f,   0.080f },   // NTSC (1953)
    { 0.735f,   0.265f,   0.274f,   0.717f,   0.167f,   0.009f },   // CIE RGB
    { 0.735f,   0.265f,   0.115f,   0.826f,   0.157f,   0.018f },   // Wide Gamut RGB
};

static
const f32 s_Gammas[GAMMA_COUNT] =
{
    1.8f,   // Apple RGB
    2.2f,   // SMPTE-C
    2.2f,   // sRGB
    2.2f,   // PAL/SECAM
    1.8f,   // Color Match RGB
    2.2f,   // Adobe RGB
    2.2f,   // NTSC (1953)
    2.2f,   // CIE RGB
    2.2f,   // Wide Gamut RGB
};

static
const SWhitePoint s_WhitePointChromas[WHITEPOINT_COUNT] =
{
    //      x        y
    { 0.4476f, 0.4074f },   // CIE Std. Illuminant A        (Tungsten lamp, normal incandescent)
    { 0.3457f, 0.3585f },   // D5000 or D50                 (Bright tungsten (low voltage bulb at serious overvoltage)
    { 0.3484f, 0.3516f },   // CIE Std. Illuminant B        (Direct sunlight)
    { 0.3324f, 0.3474f },   // D5500 or D55                 (Cloudy daylight)
    { 0.3333f, 0.3333f },   // CIE Std. Illuminant E        (Normalized reference source)
    { 0.3127f, 0.3290f },   // D6500 or D65                 (Daylight)
    { 0.3101f, 0.3162f },   // CIE Std. Illuminant C        (Average sunlight)
    { 0.2990f, 0.3149f },   // D7500 or D75                 (?)
    { 0.2848f, 0.2932f },   // D9300                        (old or low-quality monitors - Very bad)
};


////////////////////////////////////////////////////////////////////////////
//  STATIC LOOK-UP TABLES AND VALUES
////////////////////////////////////////////////////////////////////////////

//---   color-space conversion matrix
static matrix4 s_ColorConversionMatrix;

//---   gamma-correction look-up tables
static u8 s_GammaLookup[256];
static u8 s_InvGammaLookup[256];

//---   hot-color detection lookups
static f32 s_HotColorMul[3][3][256];

static f32 s_NTSC_RGBtoYIQ[3][3] =
{
    0.2989f,    0.5866f,    0.1144f,
    0.5959f,   -0.2741f,   -0.3218f,
    0.2113f,   -0.5227f,    0.3113f,
};

static f32 s_PAL_RGBtoYUV[3][3] =
{
    0.2989f,    0.5866f,    0.1144f,
   -0.1473f,   -0.2891f,    0.4364f,
    0.6149f,   -0.5145f,   -0.1004f,
};

static f32 s_HotChromaLimit;
static f32 s_HotChromaLimitSquared;
static f32 s_HotComposLimit;


////////////////////////////////////////////////////////////////////////////
//  IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

//==========================================================================
//==========================================================================
// Helper routines
//==========================================================================
//==========================================================================

static
void ConvertColorViaLookups( x_bitmap& rBMP,
                             const u8 RLookup[],
                             const u8 GLookup[],
                             const u8 BLookup[] )
{
    s32     i, j;
    color   C;

    //---   do color conversions via lookup tables
    x_bitmap::SetPixelFormat( rBMP, rBMP );

    if ( rBMP.IsClutBased() )
    {
        for ( i = 0; i < rBMP.GetNClutColors(); i++ )
        {
            C = rBMP.GetClutColor( i );

            if ( C.A < 3 )
                continue;   // if there's no alpha, don't muck with the colors

            C.R = RLookup[C.R];
            C.G = GLookup[C.G];
            C.B = BLookup[C.B];

            rBMP.SetClutColor( i, C );
        }
    }
    else
    {
        for ( j = 0; j < rBMP.GetHeight(); j++ )
        {
            for ( i = 0; i < rBMP.GetWidth(); i++ )
            {
                C = rBMP.GetPixelColor( i, j );

                if ( C.A < 3 )
                    continue;   // if there's no alpha, don't muck with the colors

                C.R = RLookup[C.R];
                C.G = GLookup[C.G];
                C.B = BLookup[C.B];

                rBMP.SetPixelColor( i, j, C );
            }
        }
    }
}

//==========================================================================

static
f32 GammaCorrection( f32 Color, f32 Gamma )
{
    f64 C = (f64)Color;
    f64 G = 1.0 / (f64)Gamma;

    return (f32)x_pow( C, G );
}

//==========================================================================

static
f32 InvGammaCorrection( f32 Color, f32 Gamma )
{
    f64 C = (f64)Color;
    f64 G = (f64)Gamma;

    return (f32)x_pow( C, G );
}

//==========================================================================
//==========================================================================
// Color-space conversions
//==========================================================================
//==========================================================================

void BMPCOLOR_PrepConversionLookups( EColorSpace SrcCS,
                                     EWhitePoint SrcWP,
                                     EColorSpace DstCS,
                                     EWhitePoint DstWP )
{
    matrix4 SrcToXYZ;
    matrix4 XYZToDst;
    vector3 WhiteVector;
    vector3 AChromaCorrect;
    matrix4 Chroma;
    matrix4 ChromaInverse;

    //======================================================================
    // Building the source to xyz matrix
    //======================================================================


    //---   build the WhitePoint chromaticity vector
    WhiteVector.X = s_WhitePointChromas[SrcWP].wx / s_WhitePointChromas[SrcWP].wy;
    WhiteVector.Y = 1.0f;
    WhiteVector.Z = (1.0f - s_WhitePointChromas[SrcWP].wx - s_WhitePointChromas[SrcWP].wy) /
                    s_WhitePointChromas[SrcWP].wy;

    //---   build the color chromaticity matrix
    Chroma.Identity();
    Chroma.M[0][0] = s_ColorSpaceChromas[SrcCS].rx;
    Chroma.M[1][0] = s_ColorSpaceChromas[SrcCS].gx;
    Chroma.M[2][0] = s_ColorSpaceChromas[SrcCS].bx;
    Chroma.M[0][1] = s_ColorSpaceChromas[SrcCS].ry;
    Chroma.M[1][1] = s_ColorSpaceChromas[SrcCS].gy;
    Chroma.M[2][1] = s_ColorSpaceChromas[SrcCS].by;
    Chroma.M[0][2] = 1.0f - s_ColorSpaceChromas[SrcCS].rx - s_ColorSpaceChromas[SrcCS].ry;
    Chroma.M[1][2] = 1.0f - s_ColorSpaceChromas[SrcCS].gx - s_ColorSpaceChromas[SrcCS].gy;
    Chroma.M[2][2] = 1.0f - s_ColorSpaceChromas[SrcCS].bx - s_ColorSpaceChromas[SrcCS].by;

    //---   and the inverse
    ChromaInverse = Chroma;
    ChromaInverse.Invert();

    //---   the achromatic vector
    AChromaCorrect = ChromaInverse * WhiteVector;

    //---   finally the source to xyz matrix
    SrcToXYZ = Chroma;
    SrcToXYZ.M[0][0] *= AChromaCorrect.X;
    SrcToXYZ.M[1][0] *= AChromaCorrect.Y;
    SrcToXYZ.M[2][0] *= AChromaCorrect.Z;
    SrcToXYZ.M[0][1] *= AChromaCorrect.X;
    SrcToXYZ.M[1][1] *= AChromaCorrect.Y;
    SrcToXYZ.M[2][1] *= AChromaCorrect.Z;
    SrcToXYZ.M[0][2] *= AChromaCorrect.X;
    SrcToXYZ.M[1][2] *= AChromaCorrect.Y;
    SrcToXYZ.M[2][2] *= AChromaCorrect.Z;

    //======================================================================
    // Building the xyz to dst matrix
    //======================================================================

    //---   build the WhitePoint chromaticity vector
    WhiteVector.X = s_WhitePointChromas[DstWP].wx / s_WhitePointChromas[DstWP].wy;
    WhiteVector.Y = 1.0f;
    WhiteVector.Z = (1.0f - s_WhitePointChromas[DstWP].wx - s_WhitePointChromas[DstWP].wy) /
                    s_WhitePointChromas[DstWP].wy;

    //---   build the color chromaticity matrix
    Chroma.Identity();
    Chroma.M[0][0] = s_ColorSpaceChromas[DstCS].rx;
    Chroma.M[1][0] = s_ColorSpaceChromas[DstCS].gx;
    Chroma.M[2][0] = s_ColorSpaceChromas[DstCS].bx;
    Chroma.M[0][1] = s_ColorSpaceChromas[DstCS].ry;
    Chroma.M[1][1] = s_ColorSpaceChromas[DstCS].gy;
    Chroma.M[2][1] = s_ColorSpaceChromas[DstCS].by;
    Chroma.M[0][2] = 1.0f - s_ColorSpaceChromas[DstCS].rx - s_ColorSpaceChromas[DstCS].ry;
    Chroma.M[1][2] = 1.0f - s_ColorSpaceChromas[DstCS].gx - s_ColorSpaceChromas[DstCS].gy;
    Chroma.M[2][2] = 1.0f - s_ColorSpaceChromas[DstCS].bx - s_ColorSpaceChromas[DstCS].by;

    //---   and the inverse
    ChromaInverse = Chroma;
    ChromaInverse.Invert();

    //---   the achromatic vector
    AChromaCorrect = ChromaInverse * WhiteVector;

    //---   finally the xyz to dest matrix
    XYZToDst = Chroma;
    XYZToDst.M[0][0] *= AChromaCorrect.X;
    XYZToDst.M[1][0] *= AChromaCorrect.Y;
    XYZToDst.M[2][0] *= AChromaCorrect.Z;
    XYZToDst.M[0][1] *= AChromaCorrect.X;
    XYZToDst.M[1][1] *= AChromaCorrect.Y;
    XYZToDst.M[2][1] *= AChromaCorrect.Z;
    XYZToDst.M[0][2] *= AChromaCorrect.X;
    XYZToDst.M[1][2] *= AChromaCorrect.Y;
    XYZToDst.M[2][2] *= AChromaCorrect.Z;
    XYZToDst.Invert();

    //======================================================================
    // the conversion is just C' = XYZToDst * SrcToXYZ * C
    // combine XYZToDst * SrcToXYZ to reduce the amount of multiplies
    //======================================================================

    s_ColorConversionMatrix = XYZToDst * SrcToXYZ;
}

//==========================================================================

void BMPCOLOR_ConvertColorSpace( x_bitmap& rBMP )
{
    s32     i, j;
    color   C;
    vector3 CVector;

    x_bitmap::SetPixelFormat( rBMP, rBMP );

    if ( rBMP.IsClutBased() )
    {
        for ( i = 0; i < rBMP.GetNClutColors(); i++ )
        {
            C = rBMP.GetClutColor( i );

            if ( C.A < 3 )
                continue;   // if there's no alpha, don't muck with the colors

            CVector.X = (f32)C.R / 255.0f;
            CVector.Y = (f32)C.G / 255.0f;
            CVector.Z = (f32)C.B / 255.0f;

            CVector = s_ColorConversionMatrix * CVector;
            CVector.X = MIN( 1.0f, CVector.X );
            CVector.X = MAX( 0.0f, CVector.X );
            CVector.Y = MIN( 1.0f, CVector.Y );
            CVector.Y = MAX( 0.0f, CVector.Y );
            CVector.Z = MIN( 1.0f, CVector.Z );
            CVector.Z = MAX( 0.0f, CVector.Z );

            C.R = (u8)(CVector.X * 255.0f + 0.5f);
            C.G = (u8)(CVector.Y * 255.0f + 0.5f);
            C.B = (u8)(CVector.Z * 255.0f + 0.5f);

            rBMP.SetClutColor( i, C );
        }
    }
    else
    {
        for ( j = 0; j < rBMP.GetHeight(); j++ )
        {
            for ( i = 0; i < rBMP.GetWidth(); i++ )
            {
                C = rBMP.GetPixelColor( i, j );

                if ( C.A < 3 )
                    continue;   // if there's no alpha, don't muck with the colors

                CVector.X = (f32)C.R / 255.0f;
                CVector.Y = (f32)C.G / 255.0f;
                CVector.Z = (f32)C.B / 255.0f;

                CVector = s_ColorConversionMatrix * CVector;
                CVector.X = MIN( 1.0f, CVector.X );
                CVector.X = MAX( 0.0f, CVector.X );
                CVector.Y = MIN( 1.0f, CVector.Y );
                CVector.Y = MAX( 0.0f, CVector.Y );
                CVector.Z = MIN( 1.0f, CVector.Z );
                CVector.Z = MAX( 0.0f, CVector.Z );

                C.R = (u8)(CVector.X * 255.0f + 0.5f);
                C.G = (u8)(CVector.Y * 255.0f + 0.5f);
                C.B = (u8)(CVector.Z * 255.0f + 0.5f);

                rBMP.SetPixelColor( i, j, C );
            }
        }
    }
}

//==========================================================================
//==========================================================================
//  "HOT" COLOR DETECTION ROUTINES
//==========================================================================
//==========================================================================

static
void BuildHotColorConvTable( f32 ColorConvMatrix[3][3] )
{
    s32 i;

    for ( i = 0; i < 256; i++ )
    {
        s_HotColorMul[0][0][i] = ((f32)i / 255.0f) * ColorConvMatrix[0][0];
        s_HotColorMul[0][1][i] = ((f32)i / 255.0f) * ColorConvMatrix[0][1];
        s_HotColorMul[0][2][i] = ((f32)i / 255.0f) * ColorConvMatrix[0][2];
        s_HotColorMul[1][0][i] = ((f32)i / 255.0f) * ColorConvMatrix[1][0];
        s_HotColorMul[1][1][i] = ((f32)i / 255.0f) * ColorConvMatrix[1][1];
        s_HotColorMul[1][2][i] = ((f32)i / 255.0f) * ColorConvMatrix[1][2];
        s_HotColorMul[2][0][i] = ((f32)i / 255.0f) * ColorConvMatrix[2][0];
        s_HotColorMul[2][1][i] = ((f32)i / 255.0f) * ColorConvMatrix[2][1];
        s_HotColorMul[2][2][i] = ((f32)i / 255.0f) * ColorConvMatrix[2][2];
    }
}

//==========================================================================

static
void HotColorDetect( color& C,
                     xbool TestOnly,
                     color TestFillColor,
                     EHotColorFix FixMethod,
                     f32 ColorConvMatrix[3][3] )
{
    f32 y, i, q;
    f32 ChromaSquared;
    f32 CompLimitMinusYSquared;

    //---   NTSC chrominance = sqrt(I^2 + Q^2)
    //       PAL chrominance = sqrt(U^2 + V^2)  (where U is scaled by B-Y and V is scaled by R-Y)

    //---   NTSC composite signal amplitude = Y + sqrt(I^2 + Q^2)
    //       PAL composite signal amplitude = Y + sqrt(U^2 + V^2)

    //---   for PAL, we'll use i & q interchangeably with U & V...not quite
    //      the correct terminology, but the math is the same

    //---   find yiq via the lookup tables
    y = s_HotColorMul[0][0][C.R] + s_HotColorMul[0][1][C.G] + s_HotColorMul[0][2][C.B];
    i = s_HotColorMul[1][0][C.R] + s_HotColorMul[1][1][C.G] + s_HotColorMul[1][2][C.B];
    q = s_HotColorMul[2][0][C.R] + s_HotColorMul[2][1][C.G] + s_HotColorMul[2][2][C.B];

    //---   check to see if the chrominance or composite amplitude is too big
    //
    //      chrominance:   sqrt(i^2 + q^2) > limit
    //                     i^2 + q^2 > limit^2
    //
    //        composite:   y + sqrt(i^2 + q^2) > limit
    //                     sqrt(i^2 + q^2) > limit - y
    //                     i^2 + q^2 > (limit - y)^2

    ChromaSquared = i*i + q*q;
    CompLimitMinusYSquared = s_HotComposLimit - y;
    CompLimitMinusYSquared *= CompLimitMinusYSquared;

    if ( (ChromaSquared <= s_HotChromaLimitSquared) &&
         (ChromaSquared <= CompLimitMinusYSquared) )
        return; // the pixel passes

    //---   the pixel has failed if we have reached this point...now we need
    //      to repair it or mark it in some way
    
    if ( TestOnly )
    {
        //---   just mark it with a user-specified color
        C = TestFillColor;
    }
    else
    {
        f32 scale, temp;
        f32 chroma;

        chroma = x_sqrt( ChromaSquared );

        //---   do a proper repair job by either reducing the saturation or luminance
        if ( FixMethod == HOTCOLORFIX_LUMINANCE )
        {
            //---   calculate the scale factor so that the pixel will be below
            //      both chroma and composite limits
            scale = s_HotChromaLimit / chroma;
            temp = s_HotComposLimit / (y + chroma);
            if ( temp < scale )
                scale = temp;
            ASSERT( (scale >= 0.0f) && (scale <= 1.0f) );

            //---   reduce luminance...this is just scaling R, G, and B by
            //      some factor, this will reduce Y, I, and Q by the same factor
            C.R = (u8)(scale * (f32)C.R + 0.5f);
            C.G = (u8)(scale * (f32)C.G + 0.5f);
            C.B = (u8)(scale * (f32)C.B + 0.5f);
        }
        else
        {
            //---   reduce saturation by linearly interpolating between the
            //      original pixel value and a gray pixel with the same luminance
            //      (i.e. R=G=B=Y). This will change saturation without affecting
            //      luminance.

            //---   calculate the scale factor so that luminance will be unchanged
            scale = s_HotChromaLimit / chroma;
            temp = (s_HotComposLimit - y) / chroma;
            if ( temp < scale )
                scale = temp;

            C.R = (u8)((y + scale * ((f32)C.R / 255.0f - y)) * 255.0f + 0.5f);
            C.G = (u8)((y + scale * ((f32)C.G / 255.0f - y)) * 255.0f + 0.5f);
            C.B = (u8)((y + scale * ((f32)C.B / 255.0f - y)) * 255.0f + 0.5f);
        }
    }
}

//==========================================================================

static
void HotColorDetect( x_bitmap& rBMP,
                     xbool TestOnly,
                     color TestFillColor,
                     EHotColorFix FixMethod,
                     f32 ColorConvMatrix[3][3] )
{
    s32     i, j;
    color   C;
    vector3 CVector;

    x_bitmap::SetPixelFormat( rBMP, rBMP );

    if ( rBMP.IsClutBased() )
    {
        for ( i = 0; i < rBMP.GetNClutColors(); i++ )
        {
            C = rBMP.GetClutColor( i );

            if ( C.A < 3 )
                continue;   // if there's no alpha, don't muck with the colors

            HotColorDetect( C, TestOnly, TestFillColor, FixMethod, ColorConvMatrix );

            rBMP.SetClutColor( i, C );
        }
    }
    else
    {
        for ( j = 0; j < rBMP.GetHeight(); j++ )
        {
            for ( i = 0; i < rBMP.GetWidth(); i++ )
            {
                C = rBMP.GetPixelColor( i, j );

                if ( C.A < 3 )
                    continue;   // if there's no alpha, don't muck with the colors

                HotColorDetect( C, TestOnly, TestFillColor, FixMethod, ColorConvMatrix );

                rBMP.SetPixelColor( i, j, C );
            }
        }
    }
}

//==========================================================================

void BMPCOLOR_PrepNTSCHotColorLookups( void )
{
    //---   build the color conversion table
    BuildHotColorConvTable( s_NTSC_RGBtoYIQ );

    //---   find the chrominance limit...on NTSC this is around 52 or 53 IRE
    //      units, where 100 is max white and 7.5 is black. To be safe, we'll
    //      limit it further to around 50
    s_HotChromaLimit = 50.0f / (100.0f - 7.5f);
    s_HotChromaLimitSquared = s_HotChromaLimit * s_HotChromaLimit;

    //---   find the composite signal amplitude limit...we already established
    //      100 IRE units is white, the absolute maximum signal that can be
    //      carried is 120 IRE units, so a limit of 110 is a decent compromise.
    s_HotComposLimit = (110.0f - 7.5f) / (100.0f - 7.5f);
}

//==========================================================================

void BMPCOLOR_NTSCHotColorTest( x_bitmap& rBMP, color C )
{
    //---   once the lookup tables are calculated PAL and NTSC tests look
    //      the same, so call a shared function
    HotColorDetect( rBMP, TRUE, C, HOTCOLORFIX_SATURATION, s_NTSC_RGBtoYIQ );
}

//==========================================================================

void BMPCOLOR_NTSCHotColorFix( x_bitmap& rBMP, EHotColorFix FixMethod )
{
    //---   once the lookup tables are calculated PAL and NTSC tests look
    //      the same, so call a shared function
    HotColorDetect( rBMP, FALSE, color(0,0,0), FixMethod, s_NTSC_RGBtoYIQ );
}

//==========================================================================

void BMPCOLOR_PrepPALHotColorLookups( void )
{
    //---   build the color conversion table
    BuildHotColorConvTable( s_NTSC_RGBtoYIQ );

    //---   find the chrominance limit...on PAL this is around 52 or 53 IRE
    //      units, where 100 is max white and 0.0 is black. To be safe, we'll
    //      limit it further to around 50
    s_HotChromaLimit = 50.0f / (100.0f - 0.0f);
    s_HotChromaLimitSquared = s_HotChromaLimit * s_HotChromaLimit;

    //---   find the composite signal amplitude limit...we already established
    //      100 IRE units is white, the absolute maximum signal that can be
    //      carried is 120 IRE units, so a limit of 110 is a decent compromise.
    s_HotComposLimit = (110.0f - 0.0f) / (100.0f - 0.0f);
}

//==========================================================================

void BMPCOLOR_PALHotColorTest( x_bitmap& rBMP, color C )
{
    //---   once the lookup tables are calculated PAL and NTSC tests look
    //      the same, so call a shared function
    HotColorDetect( rBMP, TRUE, C, HOTCOLORFIX_SATURATION, s_PAL_RGBtoYUV );
}

//==========================================================================

void BMPCOLOR_PALHotColorFix( x_bitmap& rBMP, EHotColorFix FixMethod )
{
    //---   once the lookup tables are calculated PAL and NTSC tests look
    //      the same, so call a shared function
    HotColorDetect( rBMP, FALSE, color(0,0,0), FixMethod, s_PAL_RGBtoYUV );
}

//==========================================================================
//==========================================================================
//  GAMMA-CORRECTION FUNCTIONS
//==========================================================================
//==========================================================================

f32 BMPCOLOR_GetGammaValue( EGamma Gamma )
{
    return s_Gammas[Gamma];
}

//==========================================================================

void BMPCOLOR_PrepGammaLookups( EGamma Gamma )
{
    BMPCOLOR_PrepGammaLookups( s_Gammas[Gamma] );
}

//==========================================================================

void BMPCOLOR_PrepGammaLookups( f32 Gamma )
{
    s32 i;
    f32 F;

    for ( i = 0; i < 256; i++ )
    {
        F = (f32)i / 255.0f;
        F = GammaCorrection( F, Gamma );
        F = MIN( 1.0f, F );
        F = MAX( 0.0f, F );
        s_GammaLookup[i] = (u8)(F * 255.0f + 0.5f);
    }
}

//==========================================================================

void BMPCOLOR_DoGammaCorrection( x_bitmap& rBMP )
{
    ConvertColorViaLookups( rBMP,
                            s_GammaLookup,
                            s_GammaLookup,
                            s_GammaLookup );
}

//==========================================================================

void BMPCOLOR_PrepInvGammaLookups( EGamma Gamma )
{
    BMPCOLOR_PrepInvGammaLookups( s_Gammas[Gamma] );
}

//==========================================================================

void BMPCOLOR_PrepInvGammaLookups( f32 Gamma )
{
    s32 i;
    f32 F;

    for ( i = 0; i < 256; i++ )
    {
        F = (f32)i / 255.0f;
        F = InvGammaCorrection( F, Gamma );
        F = MIN( 1.0f, F );
        F = MAX( 0.0f, F );
        s_InvGammaLookup[i] = (u8)(F * 255.0f + 0.5f);
    }
}

//==========================================================================

void BMPCOLOR_DoInvGammaCorrection( x_bitmap& rBMP )
{
    ConvertColorViaLookups( rBMP,
                            s_InvGammaLookup,
                            s_InvGammaLookup,
                            s_InvGammaLookup );
}

//==========================================================================
