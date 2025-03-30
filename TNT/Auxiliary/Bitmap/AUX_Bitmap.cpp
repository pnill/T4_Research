////////////////////////////////////////////////////////////////////////////
//
// AUX_Bitmap.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_color.hpp"
#include "x_debug.hpp"
#include "x_memory.hpp"
#include "x_plus.hpp"
#include "x_stdio.hpp"

#include "AUX_Bitmap.hpp"

#include "BmpLd.hpp"
#include "PcxLd.hpp"
#include "TgaLd.hpp"
#include "GifLd.hpp"


////////////////////////////////////////////////////////////////////////////
// VARIABLES
////////////////////////////////////////////////////////////////////////////

#define DB_COLOR_BLACK      (0x000000FF)
#define DB_COLOR_YELLOW     (0xFFFF00FF)
#define DB_COLOR_RED        (0xFF0000FF)

static 
byte DefaultBitmapData[] =      // 4bpp, 16x16 image
{
    0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
    0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
    0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
    0x10,0x00,0x00,0x11,0x11,0x00,0x00,0x10,
    0x01,0x00,0x01,0x11,0x11,0x10,0x00,0x01,
    0x10,0x00,0x01,0x10,0x01,0x10,0x00,0x10,
    0x01,0x00,0x00,0x00,0x01,0x10,0x00,0x01,
    0x10,0x00,0x00,0x00,0x11,0x10,0x00,0x10,
    0x01,0x00,0x00,0x01,0x11,0x00,0x00,0x01,
    0x10,0x00,0x00,0x01,0x10,0x00,0x00,0x10,
    0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
    0x10,0x00,0x00,0x01,0x10,0x00,0x00,0x10,
    0x01,0x00,0x00,0x01,0x10,0x00,0x00,0x01,
    0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x10,
    0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
    0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
};

static
byte FlippedNibbles[16*16/2];


static
color DefaultBitmapClut[] =     // 32bpc, ARGB_8888 (standard color) format
{
    DB_COLOR_BLACK,
    DB_COLOR_YELLOW,
    DB_COLOR_RED,      // Entries 2 thru 15 are not used.
    DB_COLOR_RED,
    DB_COLOR_RED,
    DB_COLOR_RED,
    DB_COLOR_RED,
    DB_COLOR_RED,
    DB_COLOR_RED,
    DB_COLOR_RED,
    DB_COLOR_RED,
    DB_COLOR_RED,
    DB_COLOR_RED,
    DB_COLOR_RED,
    DB_COLOR_RED,
    DB_COLOR_RED,
};


////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
////////////////////////////////////////////////////////////////////////////

xbool AUXBMP_SaveBitmap( x_bitmap& /*Bitmap*/, const char* /*Filename*/ )
{
/*
    char    Ext  [ X_MAX_EXT ];
    err     Result = 231;

    ASSERT( Bitmap );
    ASSERT( Filename );

    //---------------------------------------------------------------------
    // Break up the filename.
    //---------------------------------------------------------------------
    x_splitpath( Filename, NULL, NULL, NULL, Ext );

    //---------------------------------------------------------------------
    // Upper case the extension.
    //---------------------------------------------------------------------
    x_strtoupper( Ext );

    //---------------------------------------------------------------------
    // Are we trying to save a GIF file?
    //---------------------------------------------------------------------
    if( x_strcmp( Ext, ".GIF" ) == 0 )
    {
        Result = BGIF_Save( Bitmap, Filename );
    }
    
    //---------------------------------------------------------------------
    // Are we trying to save a BMP file?
    //---------------------------------------------------------------------
    if( x_strcmp( Ext, ".BMP" ) == 0 )
    {
        Result = BBMP_Save( Bitmap, Filename );
    }

    //---------------------------------------------------------------------
    // Are we trying to save a TARGA file?
    //---------------------------------------------------------------------
    if( x_strcmp( Ext, ".TGA" ) == 0 )
    {
        Result = BTGA_Save( Bitmap, Filename );
    }    

    if (Result == 231)
    {
        RETURN_FAILURE(fs("Unknown File Extension [%s] ", Ext) );
    }

    return Result;
*/
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////

xbool AUXBMP_LoadBitmap( x_bitmap& Bitmap, const char* Filename )
{
    char    Drive[ X_MAX_DRIVE ];
    char    Dir  [ X_MAX_DIR   ];
    char    FName[ X_MAX_FNAME ];
    char    Ext  [ X_MAX_EXT   ];

    xbool   Result = FALSE;


    ASSERT( Filename );

    // Break up the filename.
    x_splitpath( Filename, Drive, Dir, FName, Ext );

    // Upper case the extension.
    x_strtoupper( Ext );

    // Are we trying to load a GIF file?
    if( x_strcmp( Ext, ".GIF" ) == 0 )
    {
        Result = GIF_Load( Bitmap, Filename );
    }

    // Are we trying to load a BMP file?
    if( x_strcmp( Ext, ".BMP" ) == 0 )
    {
        Result = BMP_Load( Bitmap, Filename );
    }

    // Are we trying to load a PCX file?
    if( x_strcmp( Ext, ".PCX" ) == 0 )
    {
        Result = PCX_Load( Bitmap, Filename );
    }

    // Are we trying to load a TGA file?
    if( x_strcmp( Ext, ".TGA" ) == 0 )
    {
        Result = TGA_Load( Bitmap, Filename );
    }


    if( !Result )
    {
        // Either we found no extension match, or the load failed.  In any 
        // case, set up the default bitmap, but let the error stand.
        AUXBMP_SetupDefaultBitmap( Bitmap );
    }

    return( Result );

    return FALSE;
}

////////////////////////////////////////////////////////////////////////////

void AUXBMP_SetupDefaultBitmap( x_bitmap& Bitmap )
{
    s32 x, y;

    for( y = 0; y < 16; y++ )
    {
        for( x = 0; x < 8; x++ )
        {
            u8 Pix;

            Pix = DefaultBitmapData[y*8+x];

            FlippedNibbles[y*8+x] = ((Pix & 0x0F) << 4) | ((Pix & 0xF0) >> 4);
        }
    }

    Bitmap.SetupBitmap( x_bitmap::FMT_P4_ARGB_8888,
                        16,
                        16,
                        16,
                        16,
                        FALSE,
                        128,
                        FlippedNibbles,
                        FALSE,
                        64,
                        (byte*)DefaultBitmapClut,
                        16 );
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// PS2 BITMAP FUNCTIONS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#if defined( TARGET_PS2 ) || defined( TARGET_PC )

xbool AUXBMP_GetFormatForPS2( x_bitmap::format SrcFormat, x_bitmap::format& DstFormat )
{
    x_bitmap::fmt_desc FmtDesc;

    // Get x_bitmap format description
    x_bitmap::GetFormatDesc( FmtDesc, SrcFormat );

    // Determine best PS2 format based on source format
    switch( FmtDesc.BPP )
    {
        case 4:
            DstFormat = x_bitmap::FMT_P4_ABGR_8888;
            break;
        case 8:
            DstFormat = x_bitmap::FMT_P8_ABGR_8888;
            break;
        case 16:
            DstFormat = x_bitmap::FMT_16_ABGR_1555;
            break;
        case 24:
        case 32:
            DstFormat = x_bitmap::FMT_32_ABGR_8888;
            break;

        default:
            DstFormat = SrcFormat;
            return FALSE;
    }

    return TRUE;
}

//==========================================================================

xbool AUXBMP_ConvertToPS2( x_bitmap& Bitmap, xbool BuildMips, xbool SwizzleClut, xbool SwizzleData )
{
    xbool            RetVal;
    x_bitmap::format PS2Fmt;

    // Get the destination format based on the current format.
    RetVal = AUXBMP_GetFormatForPS2( Bitmap.GetFormat(), PS2Fmt );
    if( !RetVal )
        return FALSE;

    // Convert data
    RetVal = AUXBMP_Convert( Bitmap, PS2Fmt, Bitmap );
    if( !RetVal )
        return FALSE;

    RetVal = AUXBMP_ReduceColorIntensity( Bitmap, 1.0f, 1.0f, 1.0f, 0.5f );
    if( !RetVal )
        return FALSE;

    // Build mips
    if( ( BuildMips ) && ( Bitmap.GetWidth() > 8 ) && ( Bitmap.GetHeight() > 8 ) )
        Bitmap.BuildMips();

    // Swizzle Palette
    if( SwizzleClut )
        AUXBMP_ClutSwizzle( Bitmap );

    // Swizzle Data
    if( SwizzleData )
        AUXBMP_DataSwizzlePS2( Bitmap );

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////

xbool AUXBMP_LoadPS2Bitmap( x_bitmap& Bitmap, const char* Filename, xbool SwizzleClut, xbool SwizzleData )
{
    if (!AUXBMP_LoadBitmap( Bitmap, Filename ))
        return FALSE;

    return AUXBMP_ConvertToPS2( Bitmap, FALSE, SwizzleClut, SwizzleData );
}

#endif // #if defined( TARGET_PS2 ) || defined( TARGET_PC )


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// GAMECUBE BITMAP FUNCTIONS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#if defined( TARGET_DOLPHIN ) || defined( TARGET_PC )

void AUXBMP_EndianSwap16bitCLUT( x_bitmap& BMP )
{
    ASSERT( BMP.IsClutBased() );
    ASSERT( BMP.GetBPC() == 16 );

    if( !BMP.IsClutBased() )
        return;

    s32  i;
    s32  NClutEntries = BMP.GetNClutColors();
    u16* pCLUT        = (u16*)BMP.GetClutPtr();

    // Endian-swap the clut entries
    for( i = 0; i < NClutEntries; i++ )
    {
        pCLUT[i] = ENDIAN_SWAP_16( pCLUT[i] );
    }

    // Mark bitmap from operation
    BMP.SetClutSwizzled();

    #if defined( TARGET_DOLPHIN )
        DCFlushRange( (void*)pCLUT, NClutEntries * sizeof(u16) );
    #endif
}

//==========================================================================

xbool AUXBMP_GetFormatForGameCube( x_bitmap::format SrcFormat, x_bitmap::format& DstFormat )
{
    x_bitmap::fmt_desc FmtDesc;

    // Get x_bitmap format description
    x_bitmap::GetFormatDesc( FmtDesc, SrcFormat );

    //### TODO: don't assume BPP will have correct format(i.e. 8-bit intensity map)
    // Intensity formats:  GX_TF_I4, GX_TF_I8, GX_TF_IA4, GX_TF_IA8
    // RGB formats:        GX_TF_RGB565, GX_TF_RGB5A3, GX_TF_RGBA8
    // Compressed formats: GX_TF_CMPR
    // Paletized formats:  GX_TF_C4, GX_TF_C8, GX_TF_C14X2

    // Determine best GameCube format based on source format
    switch( FmtDesc.BPP )
    {
        case 4:
            if( FmtDesc.AMask != 0 )
                DstFormat = x_bitmap::FMT_P4_ARGB_3444; // GX_TF_C4, GX_TF_RGB5A3 palette(with 3bit-alpha)
            else
                DstFormat = x_bitmap::FMT_P4_URGB_1555; // GX_TF_C4, GX_TF_RGB5A3 palette(RGB555, no alpha)
            break;

        case 8:
            if( FmtDesc.AMask != 0 )
                DstFormat = x_bitmap::FMT_P8_ARGB_3444; // GX_TF_C8, GX_TF_RGB5A3 palette(with 3bit-alpha)
            else
                DstFormat = x_bitmap::FMT_P8_URGB_1555; // GX_TF_C8, GX_TF_RGB5A3 palette(RGB555, no alpha)
            break;

        case 16:
            if( FmtDesc.AMask != 0 )
                DstFormat = x_bitmap::FMT_16_ARGB_3444; // GX_TF_RGB5A3, 3bit-alpha
            else
                DstFormat = x_bitmap::FMT_16_URGB_1555; // GX_TF_RGB5A3, RGB555, no alpha
            break;

        case 24:
        case 32:
            DstFormat = x_bitmap::FMT_32_ARGB_8888;     // GX_TF_RGBA8
            break;

        default:
            DstFormat = SrcFormat;
            return FALSE;
    }

    return TRUE;
}

//==========================================================================

xbool AUXBMP_ConvertToGameCube( x_bitmap& Bitmap, xbool BuildMips, xbool EndianSwapClut, xbool SwizzleData )
{
    xbool            RetVal;
    x_bitmap::format GCFmt;

    // Get the destination format based on the current format.
    RetVal = AUXBMP_GetFormatForGameCube( Bitmap.GetFormat(), GCFmt );
    if( !RetVal )
        return FALSE;

    // Convert the data
    RetVal = AUXBMP_Convert( Bitmap, GCFmt, Bitmap );
    if( !RetVal )
        return FALSE;

    // Build mips
    if( BuildMips && (Bitmap.GetWidth() > 8) && (Bitmap.GetHeight() > 8) )
        Bitmap.BuildMips();

    #if defined( TARGET_DOLPHIN )
        if( Bitmap.IsClutBased() )
            DCFlushRange( (void*)Bitmap.GetClutPtr(), Bitmap.GetClutSize() );

        DCFlushRange( (void*)Bitmap.GetDataPtr(), Bitmap.GetDataSize() );
    #endif

    // Swizzle Clut
    if( Bitmap.IsClutBased() )
    {
        if( EndianSwapClut && !Bitmap.IsClutSwizzled() )
        {
            AUXBMP_EndianSwap16bitCLUT( Bitmap );
        }
    }

    // Swizzle Data
    if( SwizzleData && !Bitmap.IsDataSwizzled() )
    {
        RetVal = AUXBMP_SwizzleGameCube( Bitmap );
        if( !RetVal )
            return FALSE;
    }

    return TRUE;
}

#endif // #if defined( TARGET_DOLPHIN ) || defined( TARGET_PC )


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// XBOX BITMAP FUNCTIONS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#if defined( TARGET_XBOX ) || defined( TARGET_PC )

xbool AUXBMP_GetFormatForXBOX( x_bitmap::format SrcFormat, x_bitmap::format& DstFormat )
{
    x_bitmap::fmt_desc FmtDesc;

    // Get x_bitmap format description
    x_bitmap::GetFormatDesc( FmtDesc, SrcFormat );

    // Determine best XBOX format based on source format
    switch( FmtDesc.BPP )
    {
        case 4:
        case 8:
            DstFormat = x_bitmap::FMT_P8_ARGB_8888;     // D3DFMT_P8
            break;

        case 16:
            if( FmtDesc.AMask != 0 )
                DstFormat = x_bitmap::FMT_16_ARGB_4444; // D3DFMT_A4R4G4B4 
            else
                DstFormat = x_bitmap::FMT_16_RGB_565;   // D3DFMT_R5G6B5 
            break;

        case 24:
        case 32:
            if( FmtDesc.Format == x_bitmap::FMT_32_URGB_8888 )
                DstFormat = x_bitmap::FMT_32_URGB_8888;     // D3DFMT_X8R8G8B8 
            else
                DstFormat = x_bitmap::FMT_32_ARGB_8888;     // D3DFMT_A8R8G8B8 
            break;

        default:
            DstFormat = SrcFormat;
            return FALSE;
    }

    return TRUE;
}

//==========================================================================

xbool AUXBMP_ConvertToXBOX( x_bitmap& Bitmap, xbool BuildMips, xbool SwizzleClut, xbool SwizzleData )
{
    xbool            RetVal;
    x_bitmap::format XBOXFmt;

    // Ignore SwizzleClut
    SwizzleClut = FALSE;

    // Get the destination format based on the current format.
    RetVal = AUXBMP_GetFormatForXBOX( Bitmap.GetFormat(), XBOXFmt );
    if( !RetVal )
        return FALSE;

    // Convert the data
    RetVal = AUXBMP_Convert( Bitmap, XBOXFmt, Bitmap );
    if( !RetVal )
        return FALSE;

    // Build mips
    if( BuildMips && (Bitmap.GetWidth() > 8) && (Bitmap.GetHeight() > 8) )
        Bitmap.BuildMips();

    // Swizzle data
    if( SwizzleData )
        AUXBMP_DataSwizzleXBOX( Bitmap );

    return TRUE;
}

#endif // defined( TARGET_XBOX ) || defined( TARGET_PC )


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// PC BITMAP FUNCTIONS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#if defined( TARGET_PC )

xbool AUXBMP_GetFormatForPC( x_bitmap::format SrcFormat, x_bitmap::format& DstFormat )
{
    x_bitmap::fmt_desc FmtDesc;

    // Get x_bitmap format description
    x_bitmap::GetFormatDesc( FmtDesc, SrcFormat );

    // Determine best PC format based on source format
    switch( FmtDesc.BPP )
    {
        case 4:
        case 8:
            DstFormat = x_bitmap::FMT_P8_ARGB_8888;     // D3DFMT_P8
            break;

        case 16:
            if( FmtDesc.AMask != 0 )
                DstFormat = x_bitmap::FMT_16_ARGB_4444; // D3DFMT_A4R4G4B4 
            else
                DstFormat = x_bitmap::FMT_16_RGB_565;   // D3DFMT_R5G6B5 
            break;

        case 24:
        case 32:
            DstFormat = x_bitmap::FMT_32_ARGB_8888;     // D3DFMT_A8R8G8B8 
            break;

        default:
            DstFormat = SrcFormat;
            return FALSE;
    }

    return TRUE;
}

//==========================================================================

xbool AUXBMP_ConvertToPC( x_bitmap& Bitmap, xbool BuildMips, xbool SwizzleClut, xbool SwizzleData )
{
    xbool            RetVal;
    x_bitmap::format PCFormat;

    // Ignore MIPS, and the Clut and Data swizzle flags for now.
    BuildMips   = FALSE;
    SwizzleClut = FALSE;
    SwizzleData = FALSE;

    // Get the destination format based on the current format.
    RetVal = AUXBMP_GetFormatForPC( Bitmap.GetFormat(), PCFormat );
    if( !RetVal )
        return FALSE;

    // Convert the data
    RetVal = AUXBMP_Convert( Bitmap, PCFormat, Bitmap );
    if( !RetVal )
        return FALSE;

    if( BuildMips && (Bitmap.GetWidth() > 8) && (Bitmap.GetHeight() > 8) )
        Bitmap.BuildMips();

    //if( SwizzleData )
    //    AUXBMP_DataSwizzlePC( Bitmap );

    return TRUE;
}

#endif // defined( TARGET_PC )


////////////////////////////////////////////////////////////////////////////
