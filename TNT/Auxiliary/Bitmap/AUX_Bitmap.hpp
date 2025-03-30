////////////////////////////////////////////////////////////////////////////
//
// AUX_BITMAP.HPP
//
////////////////////////////////////////////////////////////////////////////

#ifndef AUX_BITMAP_HPP
#define AUX_BITMAP_HPP


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_bitmap.hpp"


////////////////////////////////////////////////////////////////////////////
// CONSTANTS
////////////////////////////////////////////////////////////////////////////

enum S3TC_TYPE
{
    S3TC_AUTO = -1, // compression routine picks best S3TC format

    S3TC_DXT1,      // S3TC format 1
    S3TC_DXT2,      // S3TC format 2 (treated same as 3)
    S3TC_DXT3,      // S3TC format 3
    S3TC_DXT4,      // S3TC format 4 (treated same as 5)
    S3TC_DXT5,      // S3TC format 5

    S3TC_TYPE_COUNT
};


///////////////////////////////////////////////////////////////////////////
// Load bitmap from arbitrary format.  If err==ERR_FAILURE then the
// bitmap will be returned containing the default bitmap [?]
///////////////////////////////////////////////////////////////////////////

xbool AUXBMP_LoadBitmap         ( x_bitmap& Bitmap, const char* Filename );

///////////////////////////////////////////////////////////////////////////
// Save an x_bitmap into a standard file format such gif, tga, depending on 
// the extension in the FileName.
///////////////////////////////////////////////////////////////////////////

xbool AUXBMP_SaveBitmap         ( x_bitmap& Bitmap, const char* Filename );

///////////////////////////////////////////////////////////////////////////
// Create instance of default bitmap [?]
///////////////////////////////////////////////////////////////////////////

void  AUXBMP_SetupDefaultBitmap ( x_bitmap& Bitmap );

///////////////////////////////////////////////////////////////////////////
// Conversion utilities
///////////////////////////////////////////////////////////////////////////

xbool AUXBMP_Convert            ( x_bitmap&        DstBMP, 
                                  x_bitmap::format DstFormat, 
                                  x_bitmap&        SrcBMP );


x_bitmap::format AUXBMP_FindBestFormat( x_bitmap::format SrcFormat, 
                                        xbool*           CanUseFormat );

///////////////////////////////////////////////////////////////////////////
// Take alpha values from AlphaBMP and apply to BMP.
// This works for Palletized and Color per pixel formats.
// PunchThrough will snap alpha to 0 or 255.
// The format of BMP can change during this process.
///////////////////////////////////////////////////////////////////////////

xbool AUXBMP_ApplyAlpha     ( x_bitmap& BMP, 
                              x_bitmap& AlphaBMP, 
                              xbool     PunchThrough,
                              xbool     GenerateTransparentColor );

///////////////////////////////////////////////////////////////////////////
// Generic Loaders, compression and swizzle functions.
///////////////////////////////////////////////////////////////////////////

void  AUXBMP_ClutSwizzle            ( x_bitmap& Bitmap );
xbool AUXBMP_ReduceColorIntensity   ( x_bitmap& SrcBMP, f32 fR = 0.5f, f32 fG = 0.5f, f32 fB = 0.5f,  f32 fA = 0.5f );
xbool AUXBMP_Compress               ( x_bitmap& Bitmap, S3TC_TYPE CompFormat = S3TC_AUTO );


////////////////////////////////////////////////////////////////////////////
// Platform Specific Loaders, compression and swizzle functions.
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// PS2
#if defined( TARGET_PS2 ) || defined( TARGET_PC )

xbool AUXBMP_LoadPS2Bitmap      ( x_bitmap& Bitmap, const char* Filename, xbool SwizzleClut = TRUE, xbool SwizzleData = TRUE );
xbool AUXBMP_GetFormatForPS2    ( x_bitmap::format SrcFormat, x_bitmap::format& DstFormat );
xbool AUXBMP_ConvertToPS2       ( x_bitmap& Bitmap, xbool BuildMips = FALSE, xbool SwizzleClut = TRUE, xbool SwizzleData = TRUE );
void  AUXBMP_DataSwizzlePS2     ( x_bitmap& Bitmap );

xbool AUXBMP_CompressForPS2     ( x_bitmap& Bitmap );

#endif

////////////////////////////////////////////////////////////////////////////
// GAMECUBE
#if defined( TARGET_DOLPHIN ) || defined( TARGET_PC )

xbool AUXBMP_GetFormatForGameCube( x_bitmap::format SrcFormat, x_bitmap::format& DstFormat );
xbool AUXBMP_ConvertToGameCube   ( x_bitmap& Bitmap, xbool BuildMips = FALSE, xbool EndianSwapClut = TRUE, xbool SwizzleData = TRUE );
xbool AUXBMP_SwizzleGameCube     ( x_bitmap& BMP );
void  AUXBMP_EndianSwap16bitCLUT ( x_bitmap& BMP );

#endif

////////////////////////////////////////////////////////////////////////////
// XBOX
#if defined( TARGET_XBOX ) || defined( TARGET_PC )

xbool AUXBMP_GetFormatForXBOX   ( x_bitmap::format SrcFormat, x_bitmap::format& DstFormat );
xbool AUXBMP_ConvertToXBOX      ( x_bitmap& Bitmap, xbool BuildMips = FALSE, xbool SwizzleClut = TRUE, xbool SwizzleData = TRUE );
void  AUXBMP_DataSwizzleXBOX    ( x_bitmap& Bitmap );
xbool AUXBMP_CompressS3TC       ( x_bitmap& Bitmap, S3TC_TYPE CompFormat = S3TC_AUTO );

#endif

////////////////////////////////////////////////////////////////////////////
// PC
#if defined( TARGET_PC )

xbool AUXBMP_GetFormatForPC     ( x_bitmap::format SrcFormat, x_bitmap::format& DstFormat );
xbool AUXBMP_ConvertToPC        ( x_bitmap& Bitmap, xbool BuildMips = FALSE, xbool SwizzleClut = TRUE, xbool SwizzleData = TRUE );

#endif

///////////////////////////////////////////////////////////////////////////

#endif // AUX_BITMAP_HPP
