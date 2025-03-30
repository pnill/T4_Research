////////////////////////////////////////////////////////////////////////////
//
// XBmpProcess.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "x_debug.hpp"
#include "x_memory.hpp"
#include "x_math.hpp"

#include "AUX_Bitmap.hpp"
#include "BMPColor.h"

#include "XBmpProcess.h"
#include "StatusOutput.h"
#include "TBlockIMEXObj.h"


//#define USE_PS2_COMPRESSION (TRUE)


////////////////////////////////////////////////////////////////////////////
// CONSTANTS
////////////////////////////////////////////////////////////////////////////

#define XBMP_MIN_WIDTH              8
#define XBMP_MAX_WIDTH              1024
#define XBMP_MIN_HEIGHT             8
#define XBMP_MAX_HEIGHT             1024

#define XBMP_COMPRESS_MULT_WIDTH    8
#define XBMP_COMPRESS_MULT_HEIGHT   8

#define XBMP_ALPHA_FILE_COUNT       4


static const char* s_AlphaMapName[XBMP_ALPHA_FILE_COUNT] =
{
    "Alpha",    // Accronym for Punch through.
    "_alpha",   // Accronym for Alpha Mask.
    "_mask",
    "_punch",
};

static const xbool s_AlphaPunchParam[XBMP_ALPHA_FILE_COUNT] =
{
    TRUE,
    FALSE,
    FALSE,
    TRUE,
};

static const xbool s_AlphaGenClearParam[XBMP_ALPHA_FILE_COUNT] =
{
    TRUE,
    FALSE,
    FALSE,
    TRUE,
};


////////////////////////////////////////////////////////////////////////////
// STRUCTURES
////////////////////////////////////////////////////////////////////////////

struct TBlockSplitPath
{
    char Drive [X_MAX_DRIVE];
    char Folder[X_MAX_DIR];
    char FName [X_MAX_FNAME];
    char Ext   [X_MAX_EXT];
};

struct SimilarPaletteColor
{
    color   Color;
    u8      PaletteIndex;
};


////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION DECLARATIONS
////////////////////////////////////////////////////////////////////////////

static void  InitDefaultSplitPath       ( TBlockSplitPath& rSplitPath );

static void  XBMP_ApplyAlpha            ( x_bitmap&        xBMP,
                                          TBlockSplitPath& rBMPSplitPath,
                                          TBlockSettings*  pTBS );

static void  XBMP_MakeFormatAlpha       ( x_bitmap&        xBMP );

static xbool XBMP_ConvertFmtForPlatform ( x_bitmap&        xBMP,
                                          s32              TgtPlatform,
                                          TBlockSplitPath& rBMPSplitPath );

static xbool XBMP_RotateTallToWide      ( x_bitmap&        xBMP );

static xbool XBMP_CheckPS2Dimensions    ( x_bitmap&        xBMP,
                                          TBlockSplitPath& BMPSplitPath );

static xbool XBMP_CheckDimensions       ( x_bitmap&        xBMP,
                                          TBlockSplitPath& BMPSplitPath,
                                          s32              TgtPlatform,
                                          xbool&           rbDoSwizzle,
                                          xbool&           rbDoCompress );

static void  XBMP_Flip4bitNibbles       ( x_bitmap&        xBMP );

static xbool XBMP_CompressForPlatform   ( x_bitmap&        xBMP,
                                          s32              TgtPlatform );

static void  XBMP_SwizzleForPlatform    ( x_bitmap&        xBMP,
                                          s32              TgtPlatform,
                                          TBlockSplitPath& rBMPSplitPath );

static void  XBMP_CreateMips            ( x_bitmap&        xBMP,
                                          s32              TgtPlatform,
                                          TBlockSplitPath& rBMPSplitPath,
                                          TBlockSettings*  pTBS,
                                          xbool            bTintMips );

static void  XBMP_MakeDefaultBitmap     ( x_bitmap&        xBMP,
                                          s32              TgtPlatform,
                                          TBlockSplitPath& rBMPSplitPath,
                                          xbool            bSwizzle );

static void  XBMP_TryReduce256ColorTo16 ( x_bitmap&        xBMP,
                                          TBlockPath&      rBMPFullPath,
                                          TBlockSettings*  pTBS );

static xbool XBMP_GenerateBMP_PreMips   ( x_bitmap&        xBMP,
                                          s32              TgtPlatform,
                                          TBlockPath&      rBMPFullPath,
                                          TBlockSettings*  pTBS,
                                          TBlockSplitPath& BMPSplitPath,
                                          xbool&           bDoSwizzle,
                                          xbool&           bDoCompress );



////////////////////////////////////////////////////////////////////////////
// XBMP IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

static void InitDefaultSplitPath( TBlockSplitPath& rSplitPath )
{
    rSplitPath.Drive[0]  = '\0';
    rSplitPath.Folder[0] = '\0';
    rSplitPath.Ext[0]    = '\0';

    x_sprintf( rSplitPath.FName, "%s", TBLOCK_EMPTY_PATH );
}

//==========================================================================

static void XBMP_ApplyAlpha( x_bitmap& xBMP, TBlockSplitPath& rBMPSplitPath, TBlockSettings* pTBS )
{
    s32      i;
    xbool    bAttempt;
    xbool    bNoError;
    xbool    bUseFName;
    xbool    bPunch;
    xbool    bGenClear;
    char     cTemp  [ X_MAX_FNAME ];
    char     AlphaFN[ X_MAX_PATH  ];
    x_bitmap AlphaBMP;

    bAttempt = FALSE;
    bNoError = TRUE;

    pTBS->GetApplyAlpha( bUseFName, bPunch, bGenClear );

    // Find an alpha file to apply
    for( i = 0; i < XBMP_ALPHA_FILE_COUNT; i++ )
    {
        // Add naming convention for alpha map to filename
        x_sprintf( cTemp, "%s%s", rBMPSplitPath.FName, s_AlphaMapName[i] );
        x_makepath( AlphaFN, rBMPSplitPath.Drive, rBMPSplitPath.Folder, cTemp, rBMPSplitPath.Ext );

        // Try to load the alpha map
        if( AUXBMP_LoadBitmap( AlphaBMP, AlphaFN ) )
        {
            bAttempt = TRUE;

            // Use default parameters for filename if settings indicate it
            if( bUseFName )
                bNoError = AUXBMP_ApplyAlpha( xBMP, AlphaBMP, s_AlphaPunchParam[i], s_AlphaGenClearParam[i] );
            else
                bNoError = AUXBMP_ApplyAlpha( xBMP, AlphaBMP, bPunch, bGenClear );

            // Print status on error
            if( bNoError == FALSE )
                STATUS::Print( STATUS::MSG_WARNING, "WARNING: couldn't apply alpha on %s%s\n", rBMPSplitPath.FName, rBMPSplitPath.Ext );

            AlphaBMP.KillBitmap();

            break;
        }
    }

    // report status
    if( bNoError )
    {
        if( bAttempt )
            STATUS::Print( STATUS::MSG_NORMAL, "Applied alpha on %s%s\n", rBMPSplitPath.FName, rBMPSplitPath.Ext );
        //else
        //    STATUS::Print( STATUS::MSG_WARNING, "WARNING: No alpha map for %s%s\n", rBMPSplitPath.FName, rBMPSplitPath.Ext );
    }
}

//==========================================================================

static void XBMP_MakeFormatAlpha( x_bitmap& xBMP )
{
    xbool RetVal;

    // If bitmap already has alpha, nothing to do
    if( xBMP.IsAlpha() )
        return;

    // Convert to an alpha format
    if( xBMP.IsClutBased() )
    {
        if( xBMP.GetBPP() == 4 )
            RetVal = AUXBMP_Convert( xBMP, x_bitmap::FMT_P4_ARGB_8888, xBMP );
        else
            RetVal = AUXBMP_Convert( xBMP, x_bitmap::FMT_P8_ARGB_8888, xBMP );
    }
    else
    {
        RetVal = AUXBMP_Convert( xBMP, x_bitmap::FMT_32_ARGB_8888, xBMP );
    }

    if( !RetVal )
    {
        STATUS::Print( STATUS::MSG_WARNING, "Couldn't convert bitmap to an alpha format.\n" );
    }
    else
    {
        // Since bitmap had no alpha before, it was opaque. Set alpha to keep that.
        xBMP.SetAllAlpha( 255 );
    }
}

//==========================================================================

static xbool XBMP_ConvertFmtForPlatform( x_bitmap& xBMP, s32 TgtPlatform, TBlockSplitPath& rBMPSplitPath )
{
    xbool            RetVal;
    x_bitmap::format DstFormat;

    // Get fomat for specific platform
    switch( TgtPlatform )
    {
        case TBLOCK_TGT_PS2:
            RetVal = AUXBMP_GetFormatForPS2( xBMP.GetFormat(), DstFormat );
            break;

        case TBLOCK_TGT_GAMECUBE:
            RetVal = AUXBMP_GetFormatForGameCube( xBMP.GetFormat(), DstFormat );
            break;

        case TBLOCK_TGT_XBOX:
            RetVal = AUXBMP_GetFormatForXBOX( xBMP.GetFormat(), DstFormat );
            break;

        case TBLOCK_TGT_PCDX8:
            RetVal = AUXBMP_GetFormatForPC( xBMP.GetFormat(), DstFormat );
            break;
    }

    if( !RetVal )
    {
        STATUS::Print( STATUS::MSG_ERROR, "ERROR: Couldn't get a valid platform format for %s%s\n", rBMPSplitPath.FName, rBMPSplitPath.Ext );
        return FALSE;
    }

    // Convert bitmap
    RetVal = AUXBMP_Convert( xBMP, DstFormat, xBMP );
    if( !RetVal )
    {
        STATUS::Print( STATUS::MSG_ERROR, "ERROR: Unable to convert to platform format: %s%s\n", rBMPSplitPath.FName, rBMPSplitPath.Ext );
        return FALSE;
    }

    // Post-format conversion process
    switch( TgtPlatform )
    {
        case TBLOCK_TGT_PS2:
            RetVal = AUXBMP_ReduceColorIntensity( xBMP, 1.0f, 1.0f, 1.0f, 0.5f );
            if( !RetVal )
            {
                STATUS::Print( STATUS::MSG_WARNING, "WARNING: Unable to reduce alpha channel by half: %s%s\n", rBMPSplitPath.FName, rBMPSplitPath.Ext );
                //return FALSE;
            }
            break;

        case TBLOCK_TGT_GAMECUBE:
            break;

        case TBLOCK_TGT_XBOX:
            break;

        case TBLOCK_TGT_PCDX8:
            break;
    }

    return TRUE;
}

//==========================================================================

static xbool XBMP_RotateTallToWide( x_bitmap& xBMP )
{
    color    PixClr;
    s32      index;
    s32      X, Y;
    s32      W, H;
    byte*    pNewData;
    byte*    pClutData;
    x_bitmap NewBMP;

    W = xBMP.GetWidth();
    H = xBMP.GetHeight();

    // Allocate memory for new rotated bitmap
    pNewData = (byte*)x_malloc( xBMP.GetDataSize() );
    ASSERT( pNewData != NULL );

    if( pNewData == NULL )
        return FALSE;

    x_memset( pNewData, 0, xBMP.GetDataSize() );

    if( xBMP.IsClutBased() )
    {
        // Allocate new clut for rotated bitmap
        pClutData = (byte*)x_malloc( xBMP.GetClutSize() );
        ASSERT( pClutData != NULL );

        if( pClutData == NULL )
        {
            x_free( pNewData );
            return FALSE;
        }

        // Copy the CLUT from old to new
        x_memcpy( pClutData, xBMP.GetClutPtr(), xBMP.GetClutSize() );

        // Setup the new paletized bitmap
        NewBMP.SetupBitmap( xBMP.GetFormat(),
                            H, W, H, W,
                            TRUE, xBMP.GetDataSize(), pNewData,
                            TRUE, xBMP.GetClutSize(), pClutData, xBMP.GetNClutColors(),
                            0, FALSE );

        // Setup x_bitmap for per-pixel operations
        x_bitmap::SetPixelFormat( NewBMP, xBMP );

        // copy pixels into new bitmap, rotating it in the process
        for( Y = 0; Y < H; Y++ )
        {
            for( X = 0; X < W; X++ )
            {
                index = x_bitmap::GetPixelIndex( X, Y );

                x_bitmap::SetPixelIndex( Y, (W-1) - X, index );
            }
        }
    }
    else
    {
        // Setup the new bitmap
        NewBMP.SetupBitmap( xBMP.GetFormat(),
                            H, W, H, W,
                            TRUE, xBMP.GetDataSize(), pNewData,
                            FALSE, 0, NULL, 0,
                            0, FALSE );

        // Setup x_bitmap for per-pixel operations
        x_bitmap::SetPixelFormat( NewBMP, xBMP );

        // copy pixels into new bitmap, rotating it in the process
        for( Y = 0; Y < H; Y++ )
        {
            for( X = 0; X < W; X++ )
            {
                PixClr = x_bitmap::GetPixelColor( X, Y );

                x_bitmap::SetPixelColor( Y, (W-1) - X, PixClr );
            }
        }
    }

    // Copy new bitmap back into original
    xBMP = NewBMP;
    NewBMP.KillBitmap();

    return TRUE;
}

//==========================================================================

static xbool XBMP_CheckPS2Dimensions( x_bitmap& xBMP, TBlockSplitPath& BMPSplitPath )
{
    xbool RetVal;
    xbool bDoTall2Wide;
    s32   W;
    s32   H;
    s32   NHorzPages;
    s32   NVertPages;
    s32   NBlocks;
    x_bitmap::format DstFormat;

    W = xBMP.GetWidth();
    H = xBMP.GetHeight();

    // check for tall texture
    if( H > W )
    {
        STATUS::Print( STATUS::MSG_IMPORTANT, "NOTICE: PS2 cannot have textures taller than they are wide.\n" );
        bDoTall2Wide = TRUE;
    }
    else
    {
        bDoTall2Wide = FALSE;
    }

    // make sure an available format exists
    RetVal = AUXBMP_GetFormatForPS2( xBMP.GetFormat(), DstFormat );

    if( !RetVal )
        return FALSE;

    // NOTE: These calculations are from PS2_VRAM.cpp.  If the engine calculations
    //       change, then these need to change also.
    switch( DstFormat )
    {
        case x_bitmap::FMT_P4_ABGR_8888:
        case x_bitmap::FMT_P4_UBGR_8888:
            NHorzPages = ALIGN_128(W) / 128;
            NVertPages = ALIGN_128(H) / 128;
            break;

        case x_bitmap::FMT_P8_ABGR_8888:
        case x_bitmap::FMT_P8_UBGR_8888:
            NHorzPages = ALIGN_128(W) / 128;
            NVertPages = ALIGN_64(H)  / 64;
            break;

        case x_bitmap::FMT_16_ABGR_1555:
        case x_bitmap::FMT_16_UBGR_1555:
            NHorzPages = ALIGN_64(W) / 64;
            NVertPages = ALIGN_64(H) / 64;
            break;

        case x_bitmap::FMT_32_ABGR_8888:
            NHorzPages = ALIGN_64(W) / 64;
            NVertPages = ALIGN_32(H) / 32;
            break;

        default:
            NHorzPages = ALIGN_128(W) / 128;
            NVertPages = ALIGN_128(H) / 128;
            break;
    }

    NBlocks = NHorzPages * NVertPages * 32;

    // The magic number on the PS2 is 256 blocks.  If larger than that
    // the texture isn't going to work with the VRAM manager
#if !defined(USE_PS2_COMPRESSION)
    if( NBlocks > 256 )
    {
        STATUS::Print( STATUS::MSG_IMPORTANT, "NOTICE: Texture size(W*H*BPP) is too large for PS2.\n" );
        return FALSE;
    }
#endif

    if( bDoTall2Wide )
    {
        // rotate texture 90 degrees CCW
        STATUS::Print( STATUS::MSG_NORMAL, "Rotating CCW(90): %s%s\n", BMPSplitPath.FName, BMPSplitPath.Ext );
        RetVal = XBMP_RotateTallToWide( xBMP );

        if( !RetVal )
        {
            STATUS::Print( STATUS::MSG_WARNING, "WARNING: Couldn't rotate texture.\n" );
            return FALSE;
        }
    }

    return TRUE;
}

//==========================================================================

static xbool XBMP_CheckDimensions( x_bitmap&        xBMP,
                                   TBlockSplitPath& BMPSplitPath,
                                   s32              TgtPlatform,
                                   xbool&           rbDoSwizzle,
                                   xbool&           rbDoCompress )
{
    xbool RetVal;
    s32   W, H;

    RetVal = TRUE;
    W = xBMP.GetWidth();
    H = xBMP.GetHeight();

    // Check Width and Height for problems
    if( W < XBMP_MIN_WIDTH )
    {
        STATUS::Print( STATUS::MSG_IMPORTANT, "NOTICE: Texture width %ld is less than MIN(%ld).\n", W, (s32)XBMP_MIN_WIDTH );
        RetVal = FALSE;
    }
    if( H < XBMP_MIN_HEIGHT )
    {
        STATUS::Print( STATUS::MSG_IMPORTANT, "NOTICE: Texture height %ld is less than MIN(%ld).\n", H, (s32)XBMP_MIN_HEIGHT );
        RetVal = FALSE;
    }
    if( W > XBMP_MAX_WIDTH )
    {
        STATUS::Print( STATUS::MSG_IMPORTANT, "NOTICE: Texture width %ld is greater than MAX(%ld).\n", W, (s32)XBMP_MAX_WIDTH );
        RetVal = FALSE;
    }
    if( H > XBMP_MAX_HEIGHT )
    {
        STATUS::Print( STATUS::MSG_IMPORTANT, "NOTICE: Texture height %ld is greater than MAX(%ld).\n", H, (s32)XBMP_MAX_HEIGHT );
        RetVal = FALSE;
    }

    if( rbDoSwizzle )
    {
        if( 0 != (W & (W-1)) )
        {
            STATUS::Print( STATUS::MSG_IMPORTANT, "NOTICE: Non-power of 2 width of %ld.\n", W );
            //RetVal = FALSE;
            rbDoSwizzle = FALSE;
        }
        if( 0 != (H & (H-1)) )
        {
            STATUS::Print( STATUS::MSG_IMPORTANT, "NOTICE: Non-power of 2 height of %ld.\n", H );
            //RetVal = FALSE;
            rbDoSwizzle = FALSE;
        }
    }

    if( rbDoCompress )
    {
        if( 0 != (W % XBMP_COMPRESS_MULT_WIDTH) )
        {
            STATUS::Print( STATUS::MSG_IMPORTANT, "NOTICE: Width not multiple of %ld.\n", (s32)XBMP_COMPRESS_MULT_WIDTH );
            //RetVal = FALSE;
            rbDoCompress = FALSE;
        }
        if( 0 != (H % XBMP_COMPRESS_MULT_HEIGHT) )
        {
            STATUS::Print( STATUS::MSG_IMPORTANT, "NOTICE: Height not multiple of %ld.\n", (s32)XBMP_COMPRESS_MULT_HEIGHT );
            //RetVal = FALSE;
            rbDoCompress = FALSE;
        }
    }

    // Platform-specific checks
    if( TgtPlatform == TBLOCK_TGT_PS2 )
    {
#if !defined( USE_PS2_COMPRESSION )

        if( rbDoCompress )
        {
            STATUS::Print( STATUS::MSG_IMPORTANT, "NOTICE: PS2 does NOT support compressed textures.\n" );
            rbDoCompress = FALSE;
        }

#endif

        RetVal = XBMP_CheckPS2Dimensions( xBMP, BMPSplitPath );
    }
    else if( TgtPlatform == TBLOCK_TGT_GAMECUBE )
    {
    }
    else if( TgtPlatform == TBLOCK_TGT_XBOX )
    {
    }
    else if( TgtPlatform == TBLOCK_TGT_PCDX8 )
    {
    }

    return RetVal;
}

//==========================================================================

static void XBMP_Flip4bitNibbles( x_bitmap& xBMP )
{
    s32 i, m;
    s32 DataSize;
    u8* pData;
    s32 NMips;

    // cannot do operation if bitmap isn't 4-bit
    if( xBMP.GetBPP() != 4 )
        return;

    NMips = xBMP.GetNMips();

    for( m = 0; m <= NMips; m++ )
    {
        pData    = (u8*)xBMP.GetDataPtr( m );
        DataSize = xBMP.GetDataSize( m );

        // run through the bytes of the texture, flipping the nibbles
        for( i = 0; i < DataSize; i++ )
        {
            pData[i] = ((pData[i] & 0xF0)>>4) | ((pData[i] & 0x0F)<<4);
        }
    }
}

//==========================================================================

static xbool XBMP_CompressForPlatform( x_bitmap& xBMP, s32 TgtPlatform )
{
    xbool RetVal;

    switch( TgtPlatform )
    {
        case TBLOCK_TGT_PS2:
            // no compression available on PS2
#if !defined( USE_PS2_COMPRESSION )
            RetVal = AUXBMP_CompressForPS2( xBMP );
#else
            RetVal = FALSE;
#endif
            break;

        case TBLOCK_TGT_GAMECUBE:
            RetVal = AUXBMP_CompressS3TC( xBMP, S3TC_AUTO );
            break;

        case TBLOCK_TGT_XBOX:
            RetVal = AUXBMP_CompressS3TC( xBMP, S3TC_AUTO );
            break;

        case TBLOCK_TGT_PCDX8:
            RetVal = AUXBMP_CompressS3TC( xBMP, S3TC_DXT1 );
            break;

        default:
            RetVal = FALSE;
            break;
    }

    return RetVal;
}

//==========================================================================

static void XBMP_SwizzleForPlatform( x_bitmap& xBMP, s32 TgtPlatform, TBlockSplitPath& rBMPSplitPath )
{
    xbool RetVal = TRUE;

    switch( TgtPlatform )
    {
        case TBLOCK_TGT_PS2:
            // because of the setup of x_bitmap/AUX_Bitmap, the nibbles in a 4bit texture are
            //flipped compared to what the PS2 requires for a data format
            if( xBMP.GetBPP() == 4 )
            {
                XBMP_Flip4bitNibbles( xBMP );
            }

            if( xBMP.IsClutBased() )
                AUXBMP_ClutSwizzle( xBMP );

            AUXBMP_DataSwizzlePS2( xBMP );
            break;

        case TBLOCK_TGT_GAMECUBE:
            if( xBMP.IsClutBased() )
                AUXBMP_EndianSwap16bitCLUT( xBMP );

            RetVal = AUXBMP_SwizzleGameCube( xBMP );
            break;

        case TBLOCK_TGT_XBOX:
            AUXBMP_DataSwizzleXBOX( xBMP );
            break;

        case TBLOCK_TGT_PCDX8:
            //AUXBMP_DataSwizzlePC( xBMP );
            break;
    }

    if( !RetVal )
    {
        //### TODO:
        // Error occurred in swizzle
    }
}

//==========================================================================

static void XBMP_CreateMips( x_bitmap&        xBMP,
                             s32              TgtPlatform,
                             TBlockSplitPath& rBMPSplitPath,
                             TBlockSettings*  pTBS,
                             xbool            bTintMips )
{
    u32   i;
    s32   W, H;
    u32   NMips;
    xbool bUseCustomMips;

    // make sure mips are enabled
    if( !pTBS->GetBuildMips( bUseCustomMips, NMips ) )
        return;

    if (TgtPlatform == TBLOCK_TGT_PS2)
    {
        if (NMips > 2)
            NMips = 2;
    }

    W = xBMP.GetWidth();
    H = xBMP.GetHeight();

    // Smallest mip dimension is 8, to build any mips the size must be >= 16
    if( (W < 16) || (H < 16) )
        return;

    // In order to build mips the dimensions must be a power of two.
    if( (W & (W-1)) || (H & (H-1)) )
    {
        STATUS::Print( STATUS::MSG_WARNING, "%s has non-power-of-2 dimensions, cannot build mips.\n", rBMPSplitPath.FName );
        return;
    }

    // generate the mip LODs for texture
    xBMP.BuildMips( NMips, bTintMips );

    // CUSTOM MIP SETUP
    if( bUseCustomMips )
    {
        NMips = xBMP.GetNMips();

        x_bitmap        NewMipBMP;
        TBlockPath      NewMipFullPath;
        TBlockSplitPath NewMipSplitPath;
        xbool           bDoSwizzle  = FALSE;
        xbool           bDoCompress = FALSE;
        xbool           RetVal;

        for( i = 1; i <= NMips; i++ )
        {
            NewMipBMP.KillBitmap();

            // move width and height down to next mip level
            W >>= 1;
            H >>= 1;

            // create file path for mip file and try to load it
            x_memcpy( &NewMipSplitPath, &rBMPSplitPath, sizeof(TBlockSplitPath) );
            x_sprintf( NewMipSplitPath.FName, "%s_mip%ld", rBMPSplitPath.FName, i );
            x_makepath( NewMipFullPath, NewMipSplitPath.Drive, NewMipSplitPath.Folder, NewMipSplitPath.FName, NewMipSplitPath.Ext );

            RetVal = AUXBMP_LoadBitmap( NewMipBMP, NewMipFullPath );
            if( !RetVal )
                continue;

            // check the dimensions to make sure it's valid for this mip level
            if( (NewMipBMP.GetWidth() != W) || (NewMipBMP.GetHeight() != H) )
            {
                STATUS::Print( STATUS::MSG_WARNING, "%s has incorrect dimensions for custom mip level.\n", NewMipFullPath );
                continue;
            }

            // process the new mip the same as mip 0
            if( XBMP_GenerateBMP_PreMips( NewMipBMP, TgtPlatform, NewMipFullPath, pTBS, NewMipSplitPath, bDoSwizzle, bDoCompress ) )
                continue;

            // if formats are different, try to convert so they match
            if( NewMipBMP.GetFormat() != xBMP.GetFormat() )
            {
                RetVal = AUXBMP_Convert( NewMipBMP, xBMP.GetFormat(), NewMipBMP );
                if( !RetVal )
                {
                    STATUS::Print( STATUS::MSG_WARNING, "WARNING: Custom mip %ld could not be converted to mip 0 format, no custom mip used for this level.\n", i );
                    continue;
                }
            }

            // if texture is paletized, give warning if the new mip has a different CLUT than mip 0
            if( xBMP.IsClutBased() )
            {
                color Mip0Clut[256];
                s32   NClutColors;
                s32   index;

                NClutColors = xBMP.GetNClutColors();
                if( (NClutColors != NewMipBMP.GetNClutColors()) || (NClutColors > 256) )
                {
                    STATUS::Print( STATUS::MSG_WARNING, "WARNING: Custom mip %ld has different palette than mip 0.\n", i );
                }
                else
                {
                    // get clut colors from mip 0
                    x_bitmap::SetPixelFormat( xBMP );
                    for( index = 0; index < NClutColors; index++ )
                        Mip0Clut[index] = x_bitmap::GetClutColor( index );

                    // now compare them to new mip level
                    x_bitmap::SetPixelFormat( NewMipBMP );
                    for( index = 0; index < NClutColors; index++ )
                    {
                        if( !(Mip0Clut[index] == x_bitmap::GetClutColor( index )) )
                        {
                            STATUS::Print( STATUS::MSG_WARNING, "WARNING: Custom mip %ld has different palette than mip 0.\n", i );
                            break;
                        }
                    }
                }
            }

            // now everything appears correct with new mip level, copy it over old mip
            x_memcpy( xBMP.GetDataPtr( i ), NewMipBMP.GetDataPtr(0), xBMP.GetDataSize( i ) );
        }

        NewMipBMP.KillBitmap();
    }
}

//==========================================================================

static void XBMP_MakeDefaultBitmap( x_bitmap& xBMP, s32 TgtPlatform, TBlockSplitPath& rBMPSplitPath, xbool bSwizzle )
{
    x_bitmap DefBMP;

    // Setup default BMP, convert to platform format, and swizzle
    AUXBMP_SetupDefaultBitmap( DefBMP );

    // make xBMP own the data(copy it)
    xBMP = DefBMP;

    XBMP_ConvertFmtForPlatform( xBMP, TgtPlatform, rBMPSplitPath );

    if( bSwizzle )
        XBMP_SwizzleForPlatform( xBMP, TgtPlatform, rBMPSplitPath );
}


//==========================================================================

static void XBMP_TryReduce256ColorTo16( x_bitmap& xBMP, TBlockPath& rBMPFullPath, TBlockSettings* pTBS )
{
    ASSERT( xBMP.IsClutBased( ) );
    ASSERT( xBMP.GetBPP( ) == 8 );

    x_bitmap    xBMPOriginalCopy;

    const   s16 cRedColorLimit      = 4;
    const   s16 cBlueColorLimit     = 4;
    const   s16 cGreenColorLimit    = 4;
    const   s16 cAlphaColorLimit    = 0;
    const   s16 cSimilarColorAVGMax = 8;

    u8      PaletteUsedCount[256];

    color   TestColor1;
    color   TestColor2;

    color   TempPalette[16];
    u8      TempPaletteIndex;

    SimilarPaletteColor SimilarColors[cSimilarColorAVGMax];
    u8      SimilarColorIndex;
    s16     BlendedRed;
    s16     BlendedBlue;
    s16     BlendedGreen;
    s16     BlendedAlpha;
    u8      RedDiff;
    u8      GreenDiff;
    u8      BlueDiff;
    u8      AlphaDiff;

    s32     Width;
    s32     Height;
    s32     W;
    s32     H;
    s32     Index;
    s32     Index2;
    s32     PaletteIndex;
    u16     ColorsUsed;
    xbool   bConversionIsPossible;
    xbool   bBitmapWasTouched;

    bConversionIsPossible = FALSE;
    bBitmapWasTouched = FALSE;

    // Make a copy of the original bitmap.
    // If while messing with the palette we can't find a way to reduce the texture, we need to revert it back to
    // it's original form.
    xBMPOriginalCopy = xBMP;


    Width  = xBMP.GetPWidth();
    Height = xBMP.GetPHeight();

    // Setup for some testing
    x_bitmap::SetPixelFormat( xBMP, xBMP );

    // Similar Palette Color test.
    // 
    // Go through the color values within the palette and find colors that are very similar.
    // If any similar colors are found, then try to build an average color that is shared by all
    // of those palette colors.
    //-------------------------------------------------------------------------------------------------------------------------------------
    x_memset( (void*)PaletteUsedCount, 0, sizeof( u8 ) * 256 );

    for( Index = 0; Index < 255; Index++ )
    {
        TestColor1 = xBMP.GetClutColor( Index );

        SimilarColorIndex = 0;
        SimilarColors[SimilarColorIndex].Color = TestColor1;
        SimilarColors[SimilarColorIndex].PaletteIndex = Index;
        SimilarColorIndex++;

        for( Index2 = Index + 1; Index2 < 256; Index2++ )
        {
            // Test to see if this color has been used to compare against other palette colors already.
            // We don't want to continuously blend palette colors until we mush them all together into one
            // small bland palette.
            if( PaletteUsedCount[Index2] == 0 && SimilarColorIndex < cSimilarColorAVGMax )
            {
                TestColor2 = xBMP.GetClutColor( Index2 );

                RedDiff   = x_abs( (s16)TestColor1.R - (s16)TestColor2.R );
                GreenDiff = x_abs( (s16)TestColor1.G - (s16)TestColor2.G );
                BlueDiff  = x_abs( (s16)TestColor1.B - (s16)TestColor2.B );
                AlphaDiff = x_abs( (s16)TestColor1.A - (s16)TestColor2.A );

                // Test to see if the color is close.
                if( ( RedDiff   <= cRedColorLimit   ) &&
                    ( GreenDiff <= cGreenColorLimit ) &&
                    ( BlueDiff  <= cBlueColorLimit  ) &&
                    ( AlphaDiff <= cAlphaColorLimit ) )
                {
                    PaletteUsedCount[Index2] = 1;

                    // Copy this color into the similar color table.
                    SimilarColors[SimilarColorIndex].Color = TestColor2;
                    SimilarColors[SimilarColorIndex].PaletteIndex = Index2;
                    SimilarColorIndex++;
                }
            }
        }

        // If similar colors were found, it's time to blend them together.
        if( SimilarColorIndex > 1 )
        {
            BlendedRed   = 0;
            BlendedBlue  = 0;
            BlendedGreen = 0;
            BlendedAlpha = 0;

            for( Index2 = 0; Index2 < SimilarColorIndex; Index2++ )
            {
                BlendedRed   += SimilarColors[Index2].Color.R;
                BlendedGreen += SimilarColors[Index2].Color.G;
                BlendedBlue  += SimilarColors[Index2].Color.B;
                BlendedAlpha += SimilarColors[Index2].Color.A;
            }

            BlendedRed   /= SimilarColorIndex;
            BlendedGreen /= SimilarColorIndex;
            BlendedBlue  /= SimilarColorIndex;
            BlendedAlpha /= SimilarColorIndex;

            if( BlendedRed < 0   ) BlendedRed = 0;
            if( BlendedRed > 255 ) BlendedRed = 255;

            if( BlendedGreen < 0   ) BlendedGreen = 0;
            if( BlendedGreen > 255 ) BlendedGreen = 255;

            if( BlendedBlue < 0   ) BlendedBlue = 0;
            if( BlendedBlue > 255 ) BlendedBlue = 255;

            if( BlendedAlpha < 0   ) BlendedAlpha = 0;
            if( BlendedAlpha > 255 ) BlendedAlpha = 255;

            TestColor1.Set( (u8)BlendedRed, (u8)BlendedGreen, (u8)BlendedBlue, (u8)BlendedAlpha );

            // Create an average color, and update each color in the palette with this blended color.
            for( Index2 = 0; Index2 < SimilarColorIndex; Index2++ )
            {
                xBMP.SetClutColor( SimilarColors[Index2].PaletteIndex, TestColor1 );
            }

            // Make a note that the Image was messed with.
            bBitmapWasTouched = TRUE;
        }
    }

    // Identical color palette test.
    //-------------------------------------------------------------------------------------------------------------------------------------
    for( Index = 0; Index < 255; Index++ )
    {
        TestColor1 = xBMP.GetClutColor( Index );

        for( Index2 = Index + 1; Index2 < 256; Index2++ )
        {
            TestColor2 = xBMP.GetClutColor( Index2 );

            // Test to see if the colors are the same.
            if( TestColor1 == TestColor2 )
            {
                // Go through the entire bitmap color data and remap anything using index 2 to index 1.
                for( H = 0; H < Height; H++ )
                {
                    for( W = 0; W < Width; W++ )
                    {
                        // Find the clut index for this color.
                        PaletteIndex = xBMP.GetPixelIndex( W, H );

                        if( PaletteIndex == Index2 )
                            xBMP.SetPixelIndex( W, H, Index );
                    }
                }

                // Make a note that the source bitmap has been modified.
                bBitmapWasTouched = TRUE;
            }
        }
    }


    // Reduction
    //-------------------------------------------------------------------------------------------------------------------------------------
    // Go through all of the pixels within the bitmap, and get it's clut index, then increase the count of how many times
    // that index was used.
    x_memset( (void*)PaletteUsedCount, 0, sizeof( u8 ) * 256 );

    for( H = 0; H < Height; H++ )
    {
        for( W = 0; W < Width; W++ )
        {
            // Find the clut index for this color.
            PaletteIndex = xBMP.GetPixelIndex( W, H );
            ASSERT( PaletteIndex <= 255 );

            if( PaletteUsedCount[PaletteIndex] < 0xFF )
                PaletteUsedCount[PaletteIndex]++;
        }
    }

    // Now find out how many colors were actually used.
    ColorsUsed = 0;
    for( Index = 0; Index < 256; Index++ )
    {
        if( PaletteUsedCount[Index] != 0 )
            ColorsUsed++;
    }

    // If there are less than 16 colors used, then it's time to rebuild the clut and color data.
    if( ColorsUsed <= 16 )
    {
        bConversionIsPossible = TRUE;
    }
    else
    {
        // If we were close, put out a message indicating that an artist may be able to reduce this texture
        // further and make it 16 colors.
        if( ColorsUsed < 20 )
        {
            STATUS::Print( STATUS::MSG_IMPORTANT, "%s, uses only %d colors and is 8 bit.\n", rBMPFullPath, ColorsUsed );
        }
    }

    // If the conversion is possible, do it.
    if( bConversionIsPossible )
    {
        x_bitmap::format    xDstFormat;
        x_bitmap::format    xSourceFormat;

        // Since the conversion is possible, time to move all of the colors used into the 1st 16 clut slots 
        // in the Source Bitmap's palette.
        TempPaletteIndex = 0;
        for( Index = 0; Index < 256; Index++ )
        {
            // This index into the source clut was used.
            if( PaletteUsedCount[Index] )
            {
                // Copy the used color into the new temporary palette.
                TempPalette[TempPaletteIndex] = xBMP.GetClutColor( Index );

                // Go through the entire bitmap color data and remap anything using this color
                // to the new index into the temp clut.
                for( H = 0; H < Height; H++ )
                {
                    for( W = 0; W < Width; W++ )
                    {
                        // Find the clut index for this color.
                        PaletteIndex = xBMP.GetPixelIndex( W, H );

                        if( PaletteIndex == Index )
                            xBMP.SetPixelIndex( W, H, TempPaletteIndex );
                    }
                }

                // Increment the temporary palette offset.
                TempPaletteIndex++;
            }
        }

        // Make sure that the number of colors we expected in the palette are correct.
        ASSERT( TempPaletteIndex == ColorsUsed );

        // Now put the new temp pallette into the Source Bitmap
        for( TempPaletteIndex = 0; TempPaletteIndex < ColorsUsed; TempPaletteIndex++ )
        {
            // Copy the used color into the new temporary palette.
            xBMP.SetClutColor( TempPaletteIndex, TempPalette[TempPaletteIndex] );
        }

        // Get the source format, and let it rip, try to convert down to a 4 bit texture.
        xSourceFormat = xBMP.GetFormat( );

        switch( xSourceFormat )
        {
            case x_bitmap::FMT_P8_RGB_888:
            case x_bitmap::FMT_P8_URGB_8888:
                xDstFormat = x_bitmap::FMT_P4_URGB_8888;  break;

            case x_bitmap::FMT_P8_RGBU_8888:  xDstFormat = x_bitmap::FMT_P4_RGBU_8888;  break;
            case x_bitmap::FMT_P8_ARGB_8888:  xDstFormat = x_bitmap::FMT_P4_ARGB_8888;  break;
            case x_bitmap::FMT_P8_RGBA_8888:  xDstFormat = x_bitmap::FMT_P4_RGBA_8888;  break;
            case x_bitmap::FMT_P8_ABGR_8888:  xDstFormat = x_bitmap::FMT_P4_ABGR_8888;  break;
            case x_bitmap::FMT_P8_UBGR_8888:  xDstFormat = x_bitmap::FMT_P4_UBGR_8888;  break;

            default:
            {
                STATUS::Print( STATUS::MSG_IMPORTANT, "%s was using %d colors, but had invalid bmp format. \n", rBMPFullPath, ColorsUsed );
                break;
            }
        }

        AUXBMP_Convert( xBMP, xDstFormat, xBMP );
        STATUS::Print( STATUS::MSG_WARNING, "%s was converted to 4-Bit, now using %d colors. \n", rBMPFullPath, ColorsUsed );
    }
    else
    {
        // We know at this point that we could not convert the image to 4-bit, so 
        // test to see if the image was messed with trying to reduce it.  If it was touched, or changed,
        // then restore the bitmap to it's original form.
        if( bBitmapWasTouched )
        {
            xBMP = xBMPOriginalCopy;
        }
    }
}

//==========================================================================

static xbool XBMP_GenerateBMP_PreMips( x_bitmap&        xBMP,
                                       s32              TgtPlatform,
                                       TBlockPath&      rBMPFullPath,
                                       TBlockSettings*  pTBS,
                                       TBlockSplitPath& BMPSplitPath,
                                       xbool&           bDoSwizzle,
                                       xbool&           bDoCompress )
{
    xbool       RetVal;
    u32         R, G, B, A;
    xbool       bFixNTSC;
    xbool       bFixLuminance;
    xbool       bInvertGamma;
    f32         GammaVal;
    EGamma      PresetGamma;
    EColorSpace SrcCS, DstCS;
    EWhitePoint SrcWP, DstWP;

    // Apply Alpha
    if( pTBS->GetApplyAlpha() )
        XBMP_ApplyAlpha( xBMP, BMPSplitPath, pTBS );


    // Do some error checking on size of texture if swizzling it, otherwise
    // the bitmap won't be used on the platform for rendering
    bDoSwizzle  = pTBS->GetSwizzle();
    bDoCompress = pTBS->GetCompressFmt();

    if( bDoSwizzle || bDoCompress )
    {
        RetVal = XBMP_CheckDimensions( xBMP, BMPSplitPath, TgtPlatform, bDoSwizzle, bDoCompress );

        // Check if a change in swizzle flag will occur, alert the user
        if( !bDoSwizzle && pTBS->GetSwizzle() )
            STATUS::Print( STATUS::MSG_WARNING, "WARNING: Swizzle disabled on %s%s\n", BMPSplitPath.FName, BMPSplitPath.Ext );

        // Check if a change in compress flag will occur, alert the user
        if( !bDoCompress && pTBS->GetCompressFmt() )
            STATUS::Print( STATUS::MSG_WARNING, "WARNING: Compression disabled on %s%s\n", BMPSplitPath.FName, BMPSplitPath.Ext );

        if( !RetVal )
        {
            STATUS::Print( STATUS::MSG_ERROR, "ERROR: Non-conformant texture dimensions on %s%s, creating default texture.\n", BMPSplitPath.FName, BMPSplitPath.Ext );

            XBMP_MakeDefaultBitmap( xBMP, TgtPlatform, BMPSplitPath, pTBS->GetSwizzle() );
            return TRUE;
        }
    }


    // Color Scale
    if( pTBS->GetColorScale( R, G, B, A ) )
    {
        if( (A != 0) && (A != 255) && !xBMP.IsAlpha() )
            XBMP_MakeFormatAlpha( xBMP );

        AUXBMP_ReduceColorIntensity( xBMP,
                                     (f32)R / TBLOCK_COLOR_INT_MAX,
                                     (f32)G / TBLOCK_COLOR_INT_MAX,
                                     (f32)B / TBLOCK_COLOR_INT_MAX,
                                     (f32)A / TBLOCK_COLOR_INT_MAX );
    }

    // Gamma Correction
    if( pTBS->GetGamma( bInvertGamma, PresetGamma, GammaVal ) )
    {
        if( bInvertGamma )
        {
            BMPCOLOR_PrepInvGammaLookups( GammaVal );
            BMPCOLOR_DoInvGammaCorrection( xBMP );
        }
        else
        {
            BMPCOLOR_PrepGammaLookups( GammaVal );
            BMPCOLOR_DoGammaCorrection( xBMP );
        }
    }

    // Convert Color Space
    if( pTBS->GetColorSpace( SrcCS, SrcWP, DstCS, DstWP ) )
    {
        BMPCOLOR_PrepConversionLookups( SrcCS, SrcWP, DstCS, DstWP );
        BMPCOLOR_ConvertColorSpace( xBMP );
    }

    // Hot Color Fix/Test
    if( pTBS->GetHotColorFix( bFixNTSC, bFixLuminance ) )
    {
        if( bFixNTSC )
            BMPCOLOR_NTSCHotColorFix( xBMP, (bFixLuminance ? HOTCOLORFIX_LUMINANCE : HOTCOLORFIX_SATURATION) );
        else
            BMPCOLOR_PALHotColorFix( xBMP, (bFixLuminance ? HOTCOLORFIX_LUMINANCE : HOTCOLORFIX_SATURATION) );
    }
    else if( pTBS->GetHotColorTest( bFixNTSC, R, G, B, A ) )
    {
        if( bFixNTSC )
            BMPCOLOR_NTSCHotColorTest( xBMP, color( R, G, B, A ) );
        else
            BMPCOLOR_PALHotColorTest( xBMP, color( R, G, B, A ) );
    }

    // If this export is for the PS2, and the conditions are right for down converting the
    // image to 4-bit from 8-bit, then make the attempt.
    if( TgtPlatform == TBLOCK_TGT_PS2 )
    {
        if( pTBS->GetAllow4BitConversionTest( ) &&  // The 4 bit conversion test has been allowed for this bmp.
            xBMP.IsClutBased( ) &&                  // The BMP is clut based.
            xBMP.GetBPP( ) == 8 )                   // The BMP is 8 bits per pixel.
        {
            XBMP_TryReduce256ColorTo16( xBMP, rBMPFullPath, pTBS );
        }
    }

    if( !bDoCompress )
    {
        // Convert to platform format
        RetVal = XBMP_ConvertFmtForPlatform( xBMP, TgtPlatform, BMPSplitPath );
        if( !RetVal )
        {
            STATUS::Print( STATUS::MSG_ERROR, "ERROR: Couldn't convert to platform format: %s%s, creating default texture.\n", BMPSplitPath.FName, BMPSplitPath.Ext );

            XBMP_MakeDefaultBitmap( xBMP, TgtPlatform, BMPSplitPath, pTBS->GetSwizzle() );
            return TRUE;
        }
    }

    return FALSE;
}

//==========================================================================

void XBMP_GenerateTBlockBMP( x_bitmap&        xBMP,
                             s32              TgtPlatform,
                             TBlockPath&      rBMPFullPath,
                             TBlockSettings*  pTBS,
                             xbool            bTintMips )
{
    xbool RetVal;
    xbool bDoSwizzle  = FALSE;
    xbool bDoCompress = FALSE;

    TBlockSplitPath BMPSplitPath;


    // Check if bitmap is a default one
    if( TBLOCKPATH_IsDefault( rBMPFullPath ) )
    {
        InitDefaultSplitPath( BMPSplitPath );

        XBMP_MakeDefaultBitmap( xBMP, TgtPlatform, BMPSplitPath, pTBS->GetSwizzle() );
        return;
    }
    else
    {
        // Get pathname componnents
        x_splitpath( rBMPFullPath, BMPSplitPath.Drive, BMPSplitPath.Folder, BMPSplitPath.FName, BMPSplitPath.Ext );
    }


    // Load the bitmap
    RetVal = AUXBMP_LoadBitmap( xBMP, rBMPFullPath );
    if( !RetVal )
    {
        //couldn't load or find BMP
        STATUS::Print( STATUS::MSG_ERROR, "ERROR: Couldn't load %s%s, creating default texture.\n", BMPSplitPath.FName, BMPSplitPath.Ext );

        XBMP_MakeDefaultBitmap( xBMP, TgtPlatform, BMPSplitPath, pTBS->GetSwizzle() );
        return;
    }


    // perform all texture processing up to the point where mips get generated
    if( XBMP_GenerateBMP_PreMips( xBMP, TgtPlatform, rBMPFullPath, pTBS, BMPSplitPath, bDoSwizzle, bDoCompress ) )
        return;

    // Build Mips
    if( pTBS->GetBuildMips() )
    {
        if( bDoCompress )
        {
            // if building mips AND compressing, convert to 32bit texture for less loss of data(if src is paletized)
            RetVal = AUXBMP_Convert( xBMP, x_bitmap::FMT_32_ARGB_8888, xBMP );
            if( !RetVal )
            {
                STATUS::Print( STATUS::MSG_ERROR, "ERROR: Couldn't convert to 32bit format: %s%s, creating default texture.\n", BMPSplitPath.FName, BMPSplitPath.Ext );

                XBMP_MakeDefaultBitmap( xBMP, TgtPlatform, BMPSplitPath, pTBS->GetSwizzle() );
                return;
            }
        }

        XBMP_CreateMips( xBMP, TgtPlatform, BMPSplitPath, pTBS, bTintMips );
    }


    if( bDoCompress )
    {
        // Compress Data
        RetVal = XBMP_CompressForPlatform( xBMP, TgtPlatform );

        if( RetVal == 0 )
        {
            STATUS::Print( STATUS::MSG_ERROR, "ERROR: Couldn't compress bitmap: %s%s, creating default texture.\n", BMPSplitPath.FName, BMPSplitPath.Ext );

            XBMP_MakeDefaultBitmap( xBMP, TgtPlatform, BMPSplitPath, pTBS->GetSwizzle() );
            return;
        }
        else if ( RetVal == 2 )
        {
            STATUS::Print( STATUS::MSG_WARNING, "WARNING: Texture converted to 4-bit: %s%s\n", BMPSplitPath.FName, BMPSplitPath.Ext );
        }
        else if ( RetVal == 3 )
        {
            // PS2 Texture with Alpha - The compresser ignored it.  We don't want a default bmp
            STATUS::Print( STATUS::MSG_WARNING, "WARNING: Texture has alpha and will not compress: %s%s\n", BMPSplitPath.FName, BMPSplitPath.Ext );
            if( bDoSwizzle )
            {
                // Swizzle Data
                XBMP_SwizzleForPlatform( xBMP, TgtPlatform, BMPSplitPath );
            }
            return;
        }

        STATUS::Print( STATUS::MSG_NORMAL, "Compressed to " );

        switch( xBMP.GetFormat() )
        {
            case x_bitmap::FMT_DXT1:    STATUS::Print( STATUS::MSG_NORMAL, "DXT1" );    break;
            case x_bitmap::FMT_DXT2:    STATUS::Print( STATUS::MSG_NORMAL, "DXT2" );    break;
            case x_bitmap::FMT_DXT3:    STATUS::Print( STATUS::MSG_NORMAL, "DXT3" );    break;
            case x_bitmap::FMT_DXT4:    STATUS::Print( STATUS::MSG_NORMAL, "DXT4" );    break;
            case x_bitmap::FMT_DXT5:    STATUS::Print( STATUS::MSG_NORMAL, "DXT5" );    break;
            default:                    
                {
                    if (xBMP.IsPS2Compressed())
                        STATUS::Print( STATUS::MSG_NORMAL, "PS2 Mixed Compression" );
                    else
                        STATUS::Print( STATUS::MSG_NORMAL, "Unknown?" );    
                }
                break;
        }

        STATUS::Print( STATUS::MSG_NORMAL, " on: %s%s\n", BMPSplitPath.FName, BMPSplitPath.Ext );
    }

    if( bDoSwizzle )
    {
        // Swizzle Data
        XBMP_SwizzleForPlatform( xBMP, TgtPlatform, BMPSplitPath );
    }
}

//==========================================================================
