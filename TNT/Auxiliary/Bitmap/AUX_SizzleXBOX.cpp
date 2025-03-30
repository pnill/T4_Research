///////////////////////////////////////////////////////////////////////////
//
//  AUX_SwizzleXBOX.cpp
//
///////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_debug.hpp"
#include "x_memory.hpp"
#include "x_plus.hpp"
#include "x_stdio.hpp"
#include "x_math.hpp"

#include "AUX_Bitmap.hpp"


#if defined( TARGET_XBOX ) || defined( TARGET_PC )

////////////////////////////////////////////////////////////////////////////

#if defined( TARGET_XBOX )

    #include "xtl.h"
    #include "xgraphics.h"
    #define XBOX_SDK_INSTALLED
#else
    // Only define this if you have XBOX SDK installed on host machine
   // #define XBOX_SDK_INSTALLED

    #if defined( XBOX_SDK_INSTALLED )
        #include <windows.h>
        #include "d3d8.h"
        #include "xgraphics.h"
    #else
        #pragma message( "WARNING! XBOX SDK must be installed to swizzle and compress XBOX texture data." )
    #endif

#endif

////////////////////////////////////////////////////////////////////////////
// CONSTANTS
////////////////////////////////////////////////////////////////////////////

// NOTE: These constants MUST match what is in D3D8Types-XBox.h
enum X_D3DFMT
{
    X_D3DFMT_UNKNOWN        = 0xFFFFFFFF,

    X_D3DFMT_LIN_A8R8G8B8   = 0x00000012,
    X_D3DFMT_LIN_X8R8G8B8   = 0x0000001E,
    X_D3DFMT_LIN_A8B8G8R8   = 0x0000003F,
    X_D3DFMT_LIN_B8G8R8A8   = 0x00000040,
    X_D3DFMT_LIN_R8G8B8A8   = 0x00000041,
    X_D3DFMT_LIN_A1R5G5B5   = 0x00000010,
    X_D3DFMT_LIN_X1R5G5B5   = 0x0000001C,
    X_D3DFMT_LIN_R5G5B5A1   = 0x0000003D,
    X_D3DFMT_LIN_A4R4G4B4   = 0x0000001D,
    X_D3DFMT_LIN_R4G4B4A4   = 0x0000003E,
    X_D3DFMT_LIN_R5G6B5     = 0x00000011,

    X_D3DFMT_DXT1           = 0x0000000C,
    X_D3DFMT_DXT2           = 0x0000000E,
    X_D3DFMT_DXT3           = 0x0000000E,
    X_D3DFMT_DXT4           = 0x0000000F,
    X_D3DFMT_DXT5           = 0x0000000F,
};


////////////////////////////////////////////////////////////////////////////
// x_xbox_bitmap CLASS
////////////////////////////////////////////////////////////////////////////

class x_xbox_bitmap : public x_bitmap
{
  public:
    void SetFormat  ( format Fmt );

    void KillClutPtr( void );
    void KillDataPtr( void );
    void SetDataPtr ( xbool IsDataOwned, s32 DataSizeInBytes, void* pData );

    static s32   GetSwizzleSize( x_bitmap& BMP );
    static void  SetupMipTable ( s32 BPP, byte* pRawData, s32 W, s32 H, s32 NMips, s32 S3TCType );

    static byte* GetMipDataPtr ( byte* pData, s32 MipID );
    static s32   GetMipWidth   ( byte* pData, s32 MipID );
    static s32   GetMipHeight  ( byte* pData, s32 MipID );

    static s32   GetCompressedSize( x_bitmap& BMP, s32 S3TCType );

    static s32   GetS3TCBlockWidth ( s32 S3TCType );
    static s32   GetS3TCBlockHeight( s32 S3TCType );
    static s32   GetS3TCBlockSize  ( s32 S3TCType );
    static s32   GetS3TCPitch      ( s32 W, s32 S3TCType );

  protected:
    static s32 CalcDataSize( s32 BPP, s32 W, s32 H );
    static s32 CalcS3TCSize( s32 W, s32 H, s32 S3TCType );
};


////////////////////////////////////////////////////////////////////////////
// x_xbox_bitmap IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

inline void x_xbox_bitmap::SetFormat( format Fmt )
{
    m_Format = Fmt;
}

//==========================================================================

void x_xbox_bitmap::KillClutPtr( void )
{
    if( (m_Flags & FLAG_CLUT_OWNED) && (m_pClut != NULL) )
        x_free( m_pClut );

    m_Flags    &= ~FLAG_CLUT_OWNED;
    m_pClut    = NULL;
    m_ClutSize = 0;

    m_MaxClutID = 0;
    m_NCluts    = 0;
}

//==========================================================================

void x_xbox_bitmap::KillDataPtr( void )
{
    if( (m_Flags & FLAG_DATA_OWNED) && (m_pData != NULL) )
        x_free( m_pData );

    m_Flags    &= ~FLAG_DATA_OWNED;
    m_pData    = NULL;
    m_DataSize = 0;
}

//==========================================================================

void x_xbox_bitmap::SetDataPtr( xbool IsDataOwned, s32 DataSizeInBytes, void* pData )
{
    ASSERT( pData != NULL );
    ASSERT( DataSizeInBytes > 0 );

    if( IsDataOwned )
        m_Flags |= FLAG_DATA_OWNED;
    else
        m_Flags &= ~FLAG_DATA_OWNED;

    m_DataSize = DataSizeInBytes;
    m_pData    = (byte*)pData;
}

//==========================================================================

inline s32 x_xbox_bitmap::CalcDataSize( s32 BPP, s32 W, s32 H )
{
    if( BPP == 4 )
        return (W * H) >> 1;
    else
        return (W * H * BPP) >> 3;
}

//==========================================================================

s32 x_xbox_bitmap::GetSwizzleSize( x_bitmap& BMP )
{
    s32 i;
    s32 DataSize;

    s32 W     = BMP.GetWidth();
    s32 H     = BMP.GetHeight();
    s32 NMips = BMP.GetNMips();
    s32 BPP   = BMP.GetBPP();

    if( NMips > 0 )
    {
        //--- verify width and height are powers of 2
        ASSERT( !(W & (W-1)) );
        ASSERT( !(H & (H-1)) );

        DataSize = 0;

        //--- Add size of each mip with miptable
        for( i = 0; i <= NMips; i++ )
        {
            ASSERT( W >= 8 );
            ASSERT( H >= 8 );

            DataSize += x_xbox_bitmap::CalcDataSize( BPP, W, H );
            DataSize += (s32)sizeof(miptable);
            W >>= 1;
            H >>= 1;
        }

        //--- Align bitmap data on 32-byte boundary by padding miptable size
        if( (((NMips+1) * sizeof(miptable)) & 0x1F) != 0 )
            DataSize += 0x20 - (((NMips+1) * sizeof(miptable)) & 0x1F);
    }
    else
    {
        DataSize = x_xbox_bitmap::CalcDataSize( BPP, W, H );
    }

    return DataSize;
}

//==========================================================================

inline s32 x_xbox_bitmap::GetS3TCBlockWidth( s32 S3TCType )
{
//    switch( S3TCType )
//    {
//        case S3TC_DXT1: return 4;
//        case S3TC_DXT2: return 4;
//        case S3TC_DXT3: return 4;
//        case S3TC_DXT4: return 4;
//        case S3TC_DXT5: return 4;
//    }
//
//    ASSERT( FALSE );

    return 4;
}

//==========================================================================

inline s32 x_xbox_bitmap::GetS3TCBlockHeight( s32 S3TCType )
{
//    switch( S3TCType )
//    {
//        case S3TC_DXT1: return 4;
//        case S3TC_DXT2: return 4;
//        case S3TC_DXT3: return 4;
//        case S3TC_DXT4: return 4;
//        case S3TC_DXT5: return 4;
//    }
//
//    ASSERT( FALSE );

    return 4;
}

//==========================================================================

inline s32 x_xbox_bitmap::GetS3TCBlockSize( s32 S3TCType )
{
    switch( S3TCType )
    {
        case S3TC_DXT1: return 8;
        case S3TC_DXT2: return 16;
        case S3TC_DXT3: return 16;
        case S3TC_DXT4: return 16;
        case S3TC_DXT5: return 16;
    }

    ASSERT( FALSE );

    return 8;
}

//==========================================================================

inline s32 x_xbox_bitmap::GetS3TCPitch( s32 W, s32 S3TCType )
{
    // size of one S3TC(DXT1) block is 64 bits(8 bytes)
    // 16 for Color0, 16 for Color1, 16 * 2 bits for texel indices(4x4 grid)

    return (W / GetS3TCBlockWidth( S3TCType )) * GetS3TCBlockSize( S3TCType );
}

//==========================================================================

s32 x_xbox_bitmap::CalcS3TCSize( s32 W, s32 H, s32 S3TCType )
{
    s32 NBlocksY;
    s32 PitchW;

    NBlocksY = H / x_xbox_bitmap::GetS3TCBlockHeight( S3TCType );

    PitchW = x_xbox_bitmap::GetS3TCPitch( W, S3TCType );

    return NBlocksY * PitchW;
}

//==========================================================================

s32 x_xbox_bitmap::GetCompressedSize( x_bitmap& BMP, s32 S3TCType )
{
    s32 i;
    s32 DataSize;

    s32 W     = BMP.GetWidth();
    s32 H     = BMP.GetHeight();
    s32 NMips = BMP.GetNMips();

    if( NMips > 0 )
    {
        //--- verify width and height are powers of 2
        ASSERT( !(W & (W-1)) );
        ASSERT( !(H & (H-1)) );

        DataSize = 0;

        //--- Add size of each mip with miptable
        for( i = 0; i <= NMips; i++ )
        {
            ASSERT( W >= 8 );
            ASSERT( H >= 8 );

            DataSize += x_xbox_bitmap::CalcS3TCSize( W, H, S3TCType );
            DataSize += (s32)sizeof(miptable);
            W >>= 1;
            H >>= 1;
        }

        //--- Align bitmap data on 32-byte boundary by padding miptable size
        if( (((NMips+1) * sizeof(miptable)) & 0x1F) != 0 )
            DataSize += 0x20 - (((NMips+1) * sizeof(miptable)) & 0x1F);
    }
    else
    {
        DataSize = x_xbox_bitmap::CalcS3TCSize( W, H, S3TCType );
    }

    return DataSize;
}

//==========================================================================

void x_xbox_bitmap::SetupMipTable( s32 BPP, byte* pRawData, s32 W, s32 H, s32 NMips, s32 S3TCType )
{
    //--- verify width and height are powers of 2
    ASSERT( !(W & (W-1)) );
    ASSERT( !(H & (H-1)) );
    ASSERT( pRawData != NULL );

    miptable* pMipTable = (miptable*)pRawData;

    s32 CurOffset = (NMips+1) * sizeof(miptable);
    s32 i;

    //--- To align bitmap data properly on 32-byte boundary, account for padded miptable size
    if( (((NMips+1) * sizeof(miptable)) & 0x1F) != 0 )
        CurOffset += 0x20 - (((NMips+1) * sizeof(miptable)) & 0x1F);

    //--- fill out the mip table
    for( i = 0; i <= NMips; i++ )
    {
        ASSERT( W >= 8 );
        ASSERT( H >= 8 );

        pMipTable[i].W      = W;
        pMipTable[i].H      = H;
        pMipTable[i].Offset = CurOffset;

        if( BPP > 0 )
            CurOffset += x_xbox_bitmap::CalcDataSize( BPP, W, H );
        else
            CurOffset += x_xbox_bitmap::CalcS3TCSize( W, H, S3TCType );

        W >>= 1;
        H >>= 1;
    }
}

//==========================================================================

inline byte* x_xbox_bitmap::GetMipDataPtr( byte* pData, s32 MipID )
{
    miptable* MipTable = (miptable*)pData;
    return pData + MipTable[MipID].Offset;
}

//==========================================================================

inline s32 x_xbox_bitmap::GetMipWidth( byte* pData, s32 MipID )
{
    miptable* MipTable = (miptable*)pData;
    return (s32)MipTable[MipID].W;
}

//==========================================================================

inline s32 x_xbox_bitmap::GetMipHeight( byte* pData, s32 MipID )
{
    miptable* MipTable = (miptable*)pData;
    return (s32)MipTable[MipID].H;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// SWIZZLE FUNCTIONS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void AUXBMP_DataSwizzleXBOX( x_bitmap& rBMP )
{
#if defined( XBOX_SDK_INSTALLED )

    s32   i;
    s32   W, H;
    s32   BPP;
    s32   SwizzleSize;
    byte* pSwizzleData;
    byte* pDstPixData;
    s32   NMips;

    x_bitmap::format xFmt = rBMP.GetFormat();

    // no need to swizzle a compressed texture on xbox
    if( (xFmt == x_bitmap::FMT_DXT1) ||
        (xFmt == x_bitmap::FMT_DXT2) ||
        (xFmt == x_bitmap::FMT_DXT3) ||
        (xFmt == x_bitmap::FMT_DXT4) ||
        (xFmt == x_bitmap::FMT_DXT5) )
    {
        return;
    }

    BPP = rBMP.GetBPP();

    //--- verify supported BPP
    ASSERT( (BPP ==  8) ||
            (BPP == 16) ||
            (BPP == 32) );

    if( (BPP != 8) && (BPP != 16) && (BPP != 32) )
        return;

    W = rBMP.GetWidth();
    H = rBMP.GetHeight();

    //--- allocate new buffer for bitmap
    SwizzleSize = x_xbox_bitmap::GetSwizzleSize( rBMP );

    pSwizzleData = (byte*)x_malloc( SwizzleSize );
    ASSERT( pSwizzleData != NULL );

    x_memset( pSwizzleData, 0, SwizzleSize );

    //--- Get number of mips
    NMips = rBMP.GetNMips();

    if( NMips == 0 )
    {
        XGSwizzleRect( rBMP.GetDataPtr(), 0, NULL, pSwizzleData, W, H, NULL, BPP/8 );
    }
    else
    {
        x_xbox_bitmap::SetupMipTable( BPP, pSwizzleData, W, H, NMips, S3TC_AUTO );

        //--- go through each mip and swizzle it
        for( i = 0; i <= NMips; i++ )
        {
            pDstPixData = x_xbox_bitmap::GetMipDataPtr( pSwizzleData, i );

            W = x_xbox_bitmap::GetMipWidth( pSwizzleData, i );
            H = x_xbox_bitmap::GetMipHeight( pSwizzleData, i );

            XGSwizzleRect( rBMP.GetDataPtr( i ), 0, NULL, pDstPixData, W, H, NULL, BPP/8 );
        }
    }

    //--- set new data and kill off old
    ((x_xbox_bitmap*)&rBMP)->KillDataPtr();
    ((x_xbox_bitmap*)&rBMP)->SetDataPtr( TRUE, SwizzleSize, pSwizzleData );

    rBMP.SetDataSwizzled();

#endif
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// S3TC COMPRESSION FUNCTIONS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

static xbool IsFormatOkForS3TC( x_bitmap::format Fmt )
{
    switch( Fmt )
    {
        case x_bitmap::FMT_32_ARGB_8888:
        case x_bitmap::FMT_32_URGB_8888:
        case x_bitmap::FMT_32_ABGR_8888:
        case x_bitmap::FMT_32_BGRA_8888:
        case x_bitmap::FMT_32_RGBA_8888:
        case x_bitmap::FMT_16_ARGB_1555:
        case x_bitmap::FMT_16_URGB_1555:
        case x_bitmap::FMT_16_RGBA_5551:
        case x_bitmap::FMT_16_ARGB_4444:
        case x_bitmap::FMT_16_RGBA_4444:
        case x_bitmap::FMT_16_RGB_565:
            return TRUE;
    }

    return FALSE;
}

//==========================================================================

static s32 GetD3DLinearFmt( x_bitmap::format Fmt )
{
    switch( Fmt )
    {
        case x_bitmap::FMT_32_ARGB_8888:    return X_D3DFMT_LIN_A8R8G8B8;
        case x_bitmap::FMT_32_URGB_8888:    return X_D3DFMT_LIN_X8R8G8B8;
        case x_bitmap::FMT_32_ABGR_8888:    return X_D3DFMT_LIN_A8B8G8R8;
        case x_bitmap::FMT_32_BGRA_8888:    return X_D3DFMT_LIN_B8G8R8A8;
        case x_bitmap::FMT_32_RGBA_8888:    return X_D3DFMT_LIN_R8G8B8A8;
        case x_bitmap::FMT_16_ARGB_1555:    return X_D3DFMT_LIN_A1R5G5B5;
        case x_bitmap::FMT_16_URGB_1555:    return X_D3DFMT_LIN_X1R5G5B5;
        case x_bitmap::FMT_16_RGBA_5551:    return X_D3DFMT_LIN_R5G5B5A1;
        case x_bitmap::FMT_16_ARGB_4444:    return X_D3DFMT_LIN_A4R4G4B4;
        case x_bitmap::FMT_16_RGBA_4444:    return X_D3DFMT_LIN_R4G4B4A4;
        case x_bitmap::FMT_16_RGB_565:      return X_D3DFMT_LIN_R5G6B5;
    }

    ASSERT( FALSE );
    return X_D3DFMT_UNKNOWN;
}

//==========================================================================

static s32 GetD3DCompressedFmt( S3TC_TYPE CompType )
{
    switch( CompType )
    {
        case S3TC_DXT1: return X_D3DFMT_DXT1;
        case S3TC_DXT2: return X_D3DFMT_DXT2;
        case S3TC_DXT3: return X_D3DFMT_DXT3;
        case S3TC_DXT4: return X_D3DFMT_DXT4;
        case S3TC_DXT5: return X_D3DFMT_DXT5;
    }

    ASSERT( FALSE );
    return X_D3DFMT_UNKNOWN;
}

//==========================================================================

static x_bitmap::format GetXBMPCompressedFmt( S3TC_TYPE CompType )
{
    switch( CompType )
    {
        case S3TC_DXT1: return x_bitmap::FMT_DXT1;
        case S3TC_DXT2: return x_bitmap::FMT_DXT2;
        case S3TC_DXT3: return x_bitmap::FMT_DXT3;
        case S3TC_DXT4: return x_bitmap::FMT_DXT4;
        case S3TC_DXT5: return x_bitmap::FMT_DXT5;
    }

    ASSERT( FALSE );
    return x_bitmap::FMT_NULL;
}

//==========================================================================

static xbool CheckForDXT3Alpha( x_bitmap* pBMP, f32 DXT1_AlphaRef, f32 DXT3_AlphaError )
{
    s32   W, H;
    s32   x, y;
    s32   AlphaCounts[256];
    color C;

    ASSERT( pBMP != NULL );

    if( pBMP == NULL )
        return FALSE;

    // set alpha counts to zero
    x_memset( AlphaCounts, 0, sizeof(s32) * 256 );

    W = pBMP->GetWidth();
    H = pBMP->GetHeight();

    x_bitmap::SetPixelFormat( *pBMP );

    // run through all the pixels in the bitmap, tallying up the alphas
    for( y = 0; y < H; y++ )
    {
        for( x = 0; x < W; x++ )
        {
            C = x_bitmap::GetPixelColor( x, y );

            AlphaCounts[C.A] += 1;
        }
    }


    // Since the DXT3 alpha format is 4 bits per pixel, and the XGCompress routine
    // uses the upper 4 bits of alpha, that means if all the pixels' alpha values
    // fall in the 0-15 and 240-255 ranges it would be the same as DXT1 punchthrough.
    s32 iNPunchThrough = 0;

    for( x = 0;   x <= 15;  x++ )   iNPunchThrough += AlphaCounts[x];
    for( x = 240; x <= 255; x++ )   iNPunchThrough += AlphaCounts[x];

    if( iNPunchThrough == (W * H) )
        return FALSE;

    s32 iAlphaRef;
    f64 Error, TotalError;

    TotalError = 0.0f;

    if( DXT1_AlphaRef < 0.0f )  DXT1_AlphaRef = 0.0f;
    if( DXT1_AlphaRef > 1.0f )  DXT1_AlphaRef = 1.0f;

    iAlphaRef = (s32)(DXT1_AlphaRef * 255.0f);

    // error value is based on squared color "distance"
    for( x = 0; x <= 255; x++ )
    {
        // calc alpha value error, diff between DXT1 and DXT3 value (normalized in 0 to 1 range)
        if( x >= iAlphaRef )
            Error = (f64)(255 - (x & 0xF0)) / 255.0;
        else
            Error = (f64)(x & 0xF0) / 255.0;

        Error = Error * Error * (f64)AlphaCounts[x];    // total alpha value error is then squared and multiplied by # of pixels using it
        TotalError += Error;                            // accumulate that error for whole bitmap
    }

    TotalError = TotalError / (f64)(W * H);             // get average error per pixel
    TotalError = x_sqrt( (f32)TotalError );             // "normalize" it

    // if the error is less than X amount, then DXT1 is still okay, not enough
    // pixels have non-punchthrough alpha values
    if( TotalError < (f64)DXT3_AlphaError )
        return FALSE;

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

xbool AUXBMP_CompressS3TC( x_bitmap& Bitmap, S3TC_TYPE CompFormat )
{
#if defined( XBOX_SDK_INSTALLED )

    s32       i;
    s32       W, H;
    s32       NMips;
    s32       CompDataSize;
    byte*     pCompData;
    byte*     pDestData;
    s32       DstPitch;
    s32       SrcPitch;
    s32       RetVal;
    x_bitmap* pBMP;
    x_bitmap  ConvertedBMP;
    D3DFORMAT DstD3DFmt;
    D3DFORMAT SrcD3DFmt;
    DWORD     dwFlags;

    //--- Alpha cutoff point for opaque/transparent pixels(DXT1 only)
    const f32 DXT1_ALPHA_REF = 0.5f;

    //--- Error percentage for DXT1 or DXT3 determination
    const f32 DXT3_ALPHA_ERROR = 0.0f;  // was 0.16, set to zero to make any non-punchthrough texture be DXT3

    //--- Get width and height and check for valid dimensions
    W = Bitmap.GetWidth();
    H = Bitmap.GetHeight();

    ASSERT( (W % x_xbox_bitmap::GetS3TCBlockWidth(CompFormat)) == 0 );
    ASSERT( (H % x_xbox_bitmap::GetS3TCBlockHeight(CompFormat)) == 0 );

    if( (W % x_xbox_bitmap::GetS3TCBlockWidth(CompFormat)) != 0 )
        return FALSE;
    if( (H % x_xbox_bitmap::GetS3TCBlockHeight(CompFormat)) != 0 )
        return FALSE;

    //--- Check for valid format to compress
    if( IsFormatOkForS3TC( Bitmap.GetFormat() ) )
    {
        //--- format is ok, use original bitmap data
        pBMP = &Bitmap;
    }
    else
    {
        //--- create a temp bitmap with a valid format
        if( !AUXBMP_Convert( ConvertedBMP, x_bitmap::FMT_32_ARGB_8888, Bitmap ) )
        {
            ASSERT( FALSE );
            return FALSE;
        }

        pBMP = &ConvertedBMP;
    }

    //--- if format is S3TC_AUTO, choose best format
    if( CompFormat == S3TC_AUTO )
    {
        if( CheckForDXT3Alpha( pBMP, DXT1_ALPHA_REF, DXT3_ALPHA_ERROR ) )
            CompFormat = S3TC_DXT3;
        else
            CompFormat = S3TC_DXT1;
    }

    ASSERT( CompFormat != S3TC_AUTO );
    ASSERT( CompFormat != S3TC_TYPE_COUNT );

    //--- allocate new buffer for texture data
    CompDataSize = x_xbox_bitmap::GetCompressedSize( *pBMP, CompFormat );

    pCompData = (byte*)x_malloc( CompDataSize );
    ASSERT( pCompData != NULL );

    if( pCompData == NULL )
        return FALSE;

    x_memset( pCompData, 0, CompDataSize );

    //--- Gather info on formats, etc.
    NMips     = pBMP->GetNMips();
    DstD3DFmt = (D3DFORMAT)GetD3DCompressedFmt( CompFormat );
    SrcD3DFmt = (D3DFORMAT)GetD3DLinearFmt( pBMP->GetFormat() );
    dwFlags   = 0;

    //--- Check if format requires special flags
    if( (CompFormat == S3TC_DXT2) ||
        (CompFormat == S3TC_DXT4) )
    {
        dwFlags |= XGCOMPRESS_PREMULTIPLY;
    }

    // Other Flags(only needed for DXT4 and DXT5):
    //XGCOMPRESS_NEEDALPHA0 - Ensures that zero is available with interpolated alpha
    //XGCOMPRESS_NEEDALPHA1 - Ensures that one is available with interpolated alpha
    //XGCOMPRESS_PROTECTNONZERO - Ensures that non-zero values don't get quantized to zero

    RetVal = TRUE;

    //--- If no mips present, compress main image, otherwise compress each mip individually
    if( NMips == 0 )
    {
        DstPitch = x_xbox_bitmap::GetS3TCPitch( W, CompFormat );
        SrcPitch = (W * pBMP->GetBPP()) / 8;

        if( S_OK != XGCompressRect( pCompData, DstD3DFmt, DstPitch, W, H, pBMP->GetDataPtr(),
                                    SrcD3DFmt, SrcPitch, DXT1_ALPHA_REF, dwFlags ) )
        {
            RetVal = FALSE;
        }
    }
    else
    {
        //--- Create the bitmap's miptable
        x_xbox_bitmap::SetupMipTable( 0, pCompData, W, H, NMips, CompFormat );

        for( i = 0; i <= NMips; i++ )
        {
            pDestData = x_xbox_bitmap::GetMipDataPtr( pCompData, i );

            W = x_xbox_bitmap::GetMipWidth( pCompData, i );
            H = x_xbox_bitmap::GetMipHeight( pCompData, i );

            DstPitch = x_xbox_bitmap::GetS3TCPitch( W, CompFormat );
            SrcPitch = (W * pBMP->GetBPP()) / 8;

            if( S_OK != XGCompressRect( pDestData, DstD3DFmt, DstPitch, W, H, pBMP->GetDataPtr( i ),
                                        SrcD3DFmt, SrcPitch, DXT1_ALPHA_REF, dwFlags ) )
            {
                RetVal = FALSE;
            }
        }
    }

    //--- Kill original bitmap data and set it to new compressed data
    ((x_xbox_bitmap*)&Bitmap)->KillClutPtr();
    ((x_xbox_bitmap*)&Bitmap)->KillDataPtr();
    ((x_xbox_bitmap*)&Bitmap)->SetDataPtr( TRUE, CompDataSize, pCompData );

    ConvertedBMP.KillBitmap();

    //--- set bitmap's new format
    switch( CompFormat )
    {
      case S3TC_DXT1: ((x_xbox_bitmap*)(&Bitmap))->SetFormat( x_bitmap::FMT_DXT1 ); break;
      case S3TC_DXT2: ((x_xbox_bitmap*)(&Bitmap))->SetFormat( x_bitmap::FMT_DXT2 ); break;
      case S3TC_DXT3: ((x_xbox_bitmap*)(&Bitmap))->SetFormat( x_bitmap::FMT_DXT3 ); break;
      case S3TC_DXT4: ((x_xbox_bitmap*)(&Bitmap))->SetFormat( x_bitmap::FMT_DXT4 ); break;
      case S3TC_DXT5: ((x_xbox_bitmap*)(&Bitmap))->SetFormat( x_bitmap::FMT_DXT5 ); break;
    }

    Bitmap.SetCompressed();

    return RetVal;

#endif
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////

#endif // #if defined( TARGET_XBOX ) || defined( TARGET_PC )
