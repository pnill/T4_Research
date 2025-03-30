////////////////////////////////////////////////////////////////////////////
//
// BMPColor.h
//
////////////////////////////////////////////////////////////////////////////

#ifndef BMPCOLOR_H_INCLUDED
#define BMPCOLOR_H_INCLUDED


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_types.hpp"
#include "x_bitmap.hpp"


////////////////////////////////////////////////////////////////////////////
// This library contains various image manipulation routines, specifically
// those routines that deal with overall color changes, such as color-space
// conversions, hot color detection and correction, and gamma correction.
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
//  Enumeration of the different color spaces and white points, and gammas.
////////////////////////////////////////////////////////////////////////////

enum EColorSpace
{
    COLORSPACE_APPLE_RGB,           // typically used with whitepoint D65
    COLORSPACE_SMPTE_C,             // typically used with whitepoint D65
    COLORSPACE_SRGB,                // typically used with whitepoint D65
    COLORSPACE_PAL_SECAM,           // typically used with whitepoint D65
    COLORSPACE_COLORMATCH_RGB,      // typically used with whitepoint D50
    COLORSPACE_ADOBE_RGB,           // typically used with whitepoint D65
    COLORSPACE_NTSC,                // typically used with whitepoint Std. Illuminant C
    COLORSPACE_CIE_RGB,             // typically used with whitepoint Std. Illuminant E
    COLORSPACE_WIDEGAMUT_RGB,       // typically used with whitepoint D50

    COLORSPACE_COUNT
};

enum EWhitePoint
{
    WHITEPOINT_CIE_STD_A,           // CIE Std. Illuminant A        (Tungsten lamp, normal incandescent)
    WHITEPOINT_D50,                 // D5000 or D50                 (Bright tungsten (low voltage bulb at serious overvoltage))
    WHITEPOINT_CIE_STD_B,           // CIE Std. Illuminant B        (Direct sunlight)
    WHITEPOINT_D55,                 // D5500 or D55                 (Cloudy daylight)
    WHITEPOINT_CIE_STD_E,           // CIE Std. Illuminant E        (Normalized reference source)
    WHITEPOINT_D65,                 // D6500 or D65                 (Daylight)
    WHITEPOINT_CIE_STD_C,           // CIE Std. Illuminant C        (Average sunlight)
    WHITEPOINT_D75,                 // D7500 or D75                 (?)
    WHITEPOINT_D9300,               // D9300                        (old or low-quality monitors - Very bad)

    WHITEPOINT_COUNT
};

enum EGamma
{
    GAMMA_APPLE_RGB,
    GAMMA_SMPTE_C,
    GAMMA_SRGB,
    GAMMA_PAL_SECAM,
    GAMMA_COLORMATCH_RGB,
    GAMMA_ADOBE_RGB,
    GAMMA_NTSC,
    GAMMA_CIE_RGB,
    GAMMA_WIDEGAMUT_RGB,

    GAMMA_COUNT
};

enum EHotColorFix
{
    HOTCOLORFIX_SATURATION,
    HOTCOLORFIX_LUMINANCE,

    HOTCOLORFIX_COUNT
};


////////////////////////////////////////////////////////////////////////////
//
//  Color-space conversions
//
////////////////////////////////////////////////////////////////////////////
//
//  BMPCOLOR_PrepConversionLookups
//      Prepare the lookup tables used for converting between different
//      RGB formats.
//
//  BMPCOLOR_ConvertColorSpace
//      Converts the bitmap from one color space to another. This
//      routine assumes it is doing a conversion BEFORE gamma
//      correction.
//
////////////////////////////////////////////////////////////////////////////

void BMPCOLOR_PrepConversionLookups     ( EColorSpace SrcCS,
                                          EWhitePoint SrcWP,
                                          EColorSpace DstCS,
                                          EWhitePoint DstWP );

void BMPCOLOR_ConvertColorSpace         ( x_bitmap&   rBMP );


////////////////////////////////////////////////////////////////////////////
//
//  "Hot" color detection routines
//
////////////////////////////////////////////////////////////////////////////
//
//  BMPCOLOR_PrepNTSCHotColorLookups
//      Prepare the lookup tables used for "hot" color detection on NTSC
//      televesions.
//
//  BMPCOLOR_NTSCHotColorTest
//      Checks all pixels of a BMP for "hot" colors (those colors which
//      the television is not capable of displaying. These pixels will
//      be filled with a user-specified color, marking them as being
//      "hot".
//
//  BMPCOLOR_NTSCHotColorFix
//      Checks all pixels of a BMP for "hot" colors (those colors which
//      the television is not capable of displaying). These pixels will
//      automatically be fixed by reducing either saturation or
//      luminance.
//
//  BMPCOLOR_PrepPALHotColorLookups
//  BMPCOLOR_PALHotColorTest
//  BMPCOLOR_PALHotColorFix
//      These functions behave similar to their NTSC counterparts, only
//      they are for use with PAL televisions.
//
////////////////////////////////////////////////////////////////////////////

void BMPCOLOR_PrepNTSCHotColorLookups   ( void );
void BMPCOLOR_NTSCHotColorTest          ( x_bitmap& rBMP, color C );
void BMPCOLOR_NTSCHotColorFix           ( x_bitmap& rBMP, EHotColorFix FixMethod = HOTCOLORFIX_LUMINANCE );

void BMPCOLOR_PrepPALHotColorLookups    ( void );
void BMPCOLOR_PALHotColorTest           ( x_bitmap& rBMP, color C );
void BMPCOLOR_PALHotColorFix            ( x_bitmap& rBMP, EHotColorFix FixMethod = HOTCOLORFIX_LUMINANCE );


////////////////////////////////////////////////////////////////////////////
//
//  Gamma-correction functions
//
////////////////////////////////////////////////////////////////////////////
//
//  BMPCOLOR_GetGammaValue
//      Returns the value of a pre-defined gamma
//
//  BMPCOLOR_PrepGammaLookups
//      These two functions prepare lookup tables for doing gamma
//      correction of an image. You can either use one of the
//      pre-defined gamma values, or pass in a custom gamma value.
//
//  BMPCOLOR_DoGammaCorrection
//      Perform gamma correction on an image using the previously
//      calculated lookup table.
//
//  BMPCOLOR_PrepInvGammaLookups
//      These two functions prepare lookup tables for doing inverse
//      gamma correction of an image (making a gamma-corrected image
//      non gamma-corrected). You can either use one of the pre-defined
//      gamma values, or pass in a custom gamma value.
//
//  BMPCOLOR_DoInvGammaCorrection
//      Perform inverse gamma correction on an image using the
//      previously calculated lookup table.
//
////////////////////////////////////////////////////////////////////////////

f32  BMPCOLOR_GetGammaValue             ( EGamma    Gamma );

void BMPCOLOR_PrepGammaLookups          ( EGamma    Gamma );
void BMPCOLOR_PrepGammaLookups          ( f32       Gamma );
void BMPCOLOR_DoGammaCorrection         ( x_bitmap& rBMP );

void BMPCOLOR_PrepInvGammaLookups       ( EGamma    Gamma );
void BMPCOLOR_PrepInvGammaLookups       ( f32       Gamma );
void BMPCOLOR_DoInvGammaCorrection      ( x_bitmap& rBMP );


////////////////////////////////////////////////////////////////////////////

#endif // BMPCOLOR_H_INCLUDED