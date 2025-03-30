////////////////////////////////////////////////////////////////////////////
//
// AUX_SwizzleGC.cpp
//
// Code for packing/swizzling a texture into the gamecube format
// Pieced together from source code from texconv.exe provided by NOA
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_types.hpp"
#include "x_bitmap.hpp"
#include "x_memory.hpp"


#if defined( TARGET_DOLPHIN ) || defined( TARGET_PC )
////////////////////////////////////////////////////////////////////////////
// PROTOTYPES
////////////////////////////////////////////////////////////////////////////

// Get bitmap pixel color(or index)
static void  GC_GetPixel         ( x_bitmap& BMP, s32 MipID, u32 x, u32 y, u8& RI, u8& G, u8& B, u8& A );

// Tile-packing functions
static void  GC_PackTile_RGBA8   ( x_bitmap& BMP, s32 MipID, s32 W, s32 H, u32 x, u32 y, byte* dstPtr );
static void  GC_PackTile_RGB565  ( x_bitmap& BMP, s32 MipID, s32 W, s32 H, u32 x, u32 y, byte* dstPtr );
static void  GC_PackTile_RGBA4443( x_bitmap& BMP, s32 MipID, s32 W, s32 H, u32 x, u32 y, byte* dstPtr );
static void  GC_PackTile_CI8     ( x_bitmap& BMP, s32 MipID, s32 W, s32 H, u32 x, u32 y, byte* dstPtr );
static void  GC_PackTile_CI4     ( x_bitmap& BMP, s32 MipID, s32 W, s32 H, u32 x, u32 y, byte* dstPtr );

// Bitmap swizzle functions
static void  GC_Swizzle_RGBA8    ( x_bitmap& BMP, byte* pDstBuffer );
static void  GC_Swizzle_RGB565   ( x_bitmap& BMP, byte* pDstBuffer );
static void  GC_Swizzle_RGBA4443 ( x_bitmap& BMP, byte* pDstBuffer );
static void  GC_Swizzle_CI8      ( x_bitmap& BMP, byte* pDstBuffer );
static void  GC_Swizzle_CI4      ( x_bitmap& BMP, byte* pDstBuffer );


////////////////////////////////////////////////////////////////////////////
// LOCAL CLASSES-STRUCTURES
////////////////////////////////////////////////////////////////////////////

class x_gc_bitmap : public x_bitmap
{
  public:

    void KillDataPtr( void );
    void SetDataPtr ( xbool IsDataOwned, s32 DataSizeInBytes, void* pData );

    static s32   CalcSwizzleSize     ( s32 W, s32 H, s32 NMips, s32 BPP );
    static s32   CalcSwizzleSizeDXTn ( s32 W, s32 H, s32 NMips, xbool bIsDXT1 );

    static void  SetupMipTable       ( s32 NMips, s32 W, s32 H, s32 BPP, byte* pRawData );
    static void  SetupMipTableDXTn   ( s32 NMips, s32 W, s32 H, byte* pRawData, xbool bIsDXT1 );

    static void  GetTileDimensions   ( s32 BPP, s32& rTileW, s32& rTileH );
    static s32   CalcDataSize        ( s32 BPP, s32 W, s32 H, s32 TileW, s32 TileH );
    static byte* GetMipDataPtr       ( byte* pData, s32 MipID );
    static s32   GetMipWidth         ( byte* pData, s32 MipID );
    static s32   GetMipHeight        ( byte* pData, s32 MipID );
};

//--------------------------------------------------------------------------

struct DXT1_Block
{
    u16 RGB0;       // 16-bit color, RGB_565
    u16 RGB1;       // 16-bit color, RGB_565
    u32 Texels;     // 4x4 block, 2-bit indices
};

struct DXT3_Alpha_Block
{
    u16 row[4];     // 4 texels, 4 bits per texel
};

struct DXT3_Block
{
    DXT3_Alpha_Block    Alpha;
    DXT1_Block          Color;
};


////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

void x_gc_bitmap::KillDataPtr( void )
{
    if( (m_Flags & FLAG_DATA_OWNED) && m_pData )
        x_free( m_pData );

    m_Flags    &= ~FLAG_DATA_OWNED;
    m_pData    = NULL;
    m_DataSize = 0;
}

//==========================================================================

void x_gc_bitmap::SetDataPtr( xbool IsDataOwned, s32 DataSizeInBytes, void* pData )
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

s32 x_gc_bitmap::CalcSwizzleSize( s32 W, s32 H, s32 NMips, s32 BPP )
{
    s32 i;
    s32 TileW = 0;
    s32 TileH = 0;
    s32 DataSize;

    //--- Get tile width and height for BPP to pad the BMP width and height
    x_gc_bitmap::GetTileDimensions( BPP, TileW, TileH );

    if( NMips > 1 )
    {
        //--- verify width and height are powers of 2
        ASSERT( !(W & (W-1)) );
        ASSERT( !(H & (H-1)) );

        DataSize = 0;

        //--- Add size of each mip(padded proper width & height) with miptable
        for( i = 0; i < NMips; i++ )
        {
            ASSERT( W >= 8 );
            ASSERT( H >= 8 );

            DataSize += x_gc_bitmap::CalcDataSize( BPP, W, H, TileW, TileH );
            DataSize += (s32)sizeof(miptable);
            W >>= 1;
            H >>= 1;
        }

        //--- To align bitmap data properly on 32-byte boundary, miptable size needs to be padded
        if( ((NMips * sizeof(miptable)) & 0x1F) != 0 )
            DataSize += 0x20 - ((NMips * sizeof(miptable)) & 0x1F);
    }
    else
    {   //--- pad the width and height out to tile sizes
        DataSize = x_gc_bitmap::CalcDataSize( BPP, W, H, TileW, TileH );
    }

    return DataSize;
}

//==========================================================================

s32 x_gc_bitmap::CalcSwizzleSizeDXTn( s32 W, s32 H, s32 NMips, xbool bIsDXT1 )
{
    s32 i;
    s32 DataSize;
    s32 BlockSize;
    s32 MipTableSize;

    if( bIsDXT1 )
    {
        BlockSize    = (s32)sizeof(DXT1_Block);
        MipTableSize = (s32)sizeof(miptable);
    }
    else
    {
        BlockSize    = (s32)sizeof(DXT3_Block);
        MipTableSize = (s32)sizeof(miptable) * 2;
    }

    if( NMips > 1 )
    {
        //--- verify width and height are powers of 2
        ASSERT( !(W & (W-1)) );
        ASSERT( !(H & (H-1)) );

        DataSize = 0;

        //--- Add size of each mip(padded proper width & height) with miptable
        for( i = 0; i < NMips; i++ )
        {
            ASSERT( W >= 8 );
            ASSERT( H >= 8 );

            DataSize += (W / 4) * (H / 4) * BlockSize;
            DataSize += MipTableSize;
            W >>= 1;
            H >>= 1;
        }

        //--- To align bitmap data properly on 32-byte boundary, miptable size needs to be padded
        if( ((NMips * MipTableSize) & 0x1F) != 0 )
            DataSize += 0x20 - ((NMips * MipTableSize) & 0x1F);
    }
    else
    {
        //--- calc size of one LOD
        DataSize = (W / 4) * (H / 4) * BlockSize;
    }

    return DataSize;
}

//==========================================================================

inline void x_gc_bitmap::GetTileDimensions( s32 BPP, s32& rTileW, s32& rTileH )
{
    switch( BPP )
    {
        case  4:  rTileW = 8;   rTileH = 8;  break;
        case  8:  rTileW = 8;   rTileH = 4;  break;
        case 16:  rTileW = 4;   rTileH = 4;  break;

        //NOTE: 32-bit is padded to 2 tiles instead of one, but the BPP is twice
        //      that of 16-bit and makes up for that in the size calculations.
        case 32:  rTileW = 4;   rTileH = 4;  break;

        default:  ASSERT( FALSE );           break;
    }
}

//==========================================================================

inline s32 x_gc_bitmap::CalcDataSize( s32 BPP, s32 W, s32 H, s32 TileW, s32 TileH )
{
    //--- pads width and height out to proper tile-friendly values
    if( BPP == 4 )
    {
        return( (W + ((W % TileW) ? (TileW - (W % TileW)) : 0)) *
                (H + ((H % TileH) ? (TileH - (H % TileH)) : 0)) ) >> 1;
    }
    else
    {
        return  (W + ((W % TileW) ? (TileW - (W % TileW)) : 0)) *
                (H + ((H % TileH) ? (TileH - (H % TileH)) : 0)) *
                (BPP >> 3);
    }
}

//==========================================================================

void x_gc_bitmap::SetupMipTable( s32 NMips, s32 W, s32 H, s32 BPP, byte* pRawData )
{
    ASSERT( pRawData != NULL );

    s32 i;
    s32 TileW     = 0;
    s32 TileH     = 0;
    s32 CurOffset = NMips * sizeof(miptable);

    miptable* pMipTable = (miptable*)pRawData;

    //--- verify width and height are powers of 2
    ASSERT( !(W & (W-1)) );
    ASSERT( !(H & (H-1)) );

    //--- To align bitmap data properly on 32-byte boundary, account for padded miptable size
    if( ((NMips * sizeof(miptable)) & 0x1F) != 0 )
        CurOffset += 0x20 - ((NMips * sizeof(miptable)) & 0x1F);

    x_gc_bitmap::GetTileDimensions( BPP, TileW, TileH );

    //--- fill out the mip table
    for( i = 0; i < NMips; i++ )
    {
        ASSERT( W >= 8 );
        ASSERT( H >= 8 );

        pMipTable[i].W      = (s16)W;
        pMipTable[i].H      = (s16)H;
        pMipTable[i].Offset = CurOffset;

        CurOffset += x_gc_bitmap::CalcDataSize( BPP, W, H, TileW, TileH );
        W >>= 1;
        H >>= 1;
    }
}

//==========================================================================

void x_gc_bitmap::SetupMipTableDXTn( s32 NMips, s32 W, s32 H, byte* pRawData, xbool bIsDXT1 )
{
    ASSERT( pRawData != NULL );

    s32 i;
    s32 Width  = W;
    s32 Height = H;
    s32 CurOffset;

    miptable* pMipTable = (miptable*)pRawData;

    //--- verify width and height are powers of 2
    ASSERT( !(W & (W-1)) );
    ASSERT( !(H & (H-1)) );

    CurOffset = NMips * sizeof(miptable);

    //--- add another miptable set for alpha map if not dxt1
    if( bIsDXT1 == FALSE )
        CurOffset += NMips * sizeof(miptable);

    //--- To align bitmap data properly on 32-byte boundary, account for padded miptable size
    if( ((NMips * sizeof(miptable)) & 0x1F) != 0 )
        CurOffset += 0x20 - ((NMips * sizeof(miptable)) & 0x1F);

    //--- fill out the mip table
    for( i = 0; i < NMips; i++ )
    {
        ASSERT( W >= 8 );
        ASSERT( H >= 8 );

        pMipTable[i].W      = (s16)W;
        pMipTable[i].H      = (s16)H;
        pMipTable[i].Offset = CurOffset;

        CurOffset += (W / 4) * (H / 4) * sizeof(DXT1_Block);
        W >>= 1;
        H >>= 1;
    }

    //--- fill out alpha map miptable if needed
    if( bIsDXT1 == FALSE )
    {
        W = Width;
        H = Height;

        for( i = 0; i < NMips; i++ )
        {
            pMipTable[NMips+i].W      = (s16)W;
            pMipTable[NMips+i].H      = (s16)H;
            pMipTable[NMips+i].Offset = CurOffset;

            CurOffset += ((W * H) / 2);
            W >>= 1;
            H >>= 1;
        }
    }
}

//==========================================================================

inline byte* x_gc_bitmap::GetMipDataPtr( byte* pData, s32 MipID )
{
    miptable* MipTable = (miptable*)pData;
    return pData + MipTable[MipID].Offset;
}

//==========================================================================

inline s32 x_gc_bitmap::GetMipWidth( byte* pData, s32 MipID )
{
    miptable* MipTable = (miptable*)pData;
    return (s32)MipTable[MipID].W;
}

//==========================================================================

inline s32 x_gc_bitmap::GetMipHeight( byte* pData, s32 MipID )
{
    miptable* MipTable = (miptable*)pData;
    return (s32)MipTable[MipID].H;
}

//==========================================================================


////////////////////////////////////////////////////////////////////////////
// GC_GetPixel 
// 
// return the RGBA components from the pixel at XY,
// or the index itself if palettized
////////////////////////////////////////////////////////////////////////////
static void GC_GetPixel( x_bitmap& BMP, s32 MipID, u32 x, u32 y, u8& RI, u8& G, u8& B, u8& A )
{
    color C;

    switch( BMP.GetBPP() )
    {
        case 4:
        case 8:
            if( BMP.IsClutBased() )
            {
                RI = BMP.GetPixelIndex( x, y, MipID );
            }
            else
            {   // Intensity map, no CLUT
                C  = BMP.GetPixelColor( x, y, MipID );
                RI = C.R;
                G  = C.G;
                B  = C.B;
                A  = C.A;
            }
            break;

        case 16:
        case 24:
        case 32:
            C  = BMP.GetPixelColor( x, y, MipID );
            RI = C.R;
            G  = C.G;
            B  = C.B;
            A  = C.A;
            break;

        default:
            break;
    }
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// TILE PACKING FUNCTIONS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// GC_PackTile_RGBA8 
// 
// 4x4 tile, 16-bit texels
// x and y represent starting texel position of this tile
// pack AR in low half, GB in high half dest. buffer 
////////////////////////////////////////////////////////////////////////////
static void GC_PackTile_RGBA8( x_bitmap& BMP, s32 MipID, s32 W, s32 H, u32 x, u32 y, byte* dstPtr )
{
    u32 row, col;
    u32 realRows, realCols;
    u8* arPtr;
    u8* gbPtr;
    u8  r, g, b, a;

    // 'realRows', 'realCols' represent actual source image texels remaining
    realRows = H - y;
    realCols = W - x;

    if( realRows > 4 )
        realRows = 4;

    if( realCols > 4 )
        realCols = 4;

    // pack 2 32B tiles
    for( row = 0; row < realRows; row++ )
    {                                               // pack 2 cache lines at once
        arPtr = dstPtr  +      (row * 8);           // move 8 bytes (4 16-bit texels) per row
        gbPtr = dstPtr  + 32 + (row * 8);           // need to reset ptr each row to account for
                                                    // column padding

        for( col = 0; col < realCols; col++ )
        {
            GC_GetPixel( BMP, MipID, (x+col), (y+row), r, g, b, a );

            *arPtr       = a;                       // alpha is byte 0, red is byte 1
            *(arPtr + 1) = r;

            *gbPtr       = g;                       // green is byte 0, blue is byte 1
            *(gbPtr + 1) = b;

            arPtr += 2;
            gbPtr += 2;

        } // end for col loop
    } // end for row loop
}


////////////////////////////////////////////////////////////////////////////
// GC_PackTile_RGB565
//
// 4x4 tile, 16-bit texels
// x and y represent starting texel position of this tile
////////////////////////////////////////////////////////////////////////////
static void GC_PackTile_RGB565( x_bitmap& BMP, s32 MipID, s32 W, s32 H, u32 x, u32 y, u8* dstPtr )
{
    u32 row, col;
    u32 realRows, realCols;
    u8* tilePtr;
    u8  r, g, b, a;

    // 'realRows', 'realCols' represent actual source image texels remaining
    realRows = H - y;
    realCols = W - x;

    if( realRows > 4 )
        realRows = 4;

    if(realCols > 4 )
        realCols = 4;

    // pack 32B tile
    for( row = 0; row < realRows; row++ )
    {
        tilePtr = dstPtr + (row * 8);               // move 8 bytes (4 16-bit texels) per row
                                                    // need to reset ptr each row to account for column padding
        for( col = 0; col < realCols; col++ )
        {
            GC_GetPixel( BMP, MipID, (x+col), (y+row), r, g, b, a );

            *tilePtr       = (u8)( ( r & 0xF8)       | (u8)((g & 0xE0) >> 5) );  // byte0 is upper 5 bits of red, upper 3 of green
            *(tilePtr + 1) = (u8)( ((g & 0x1C) << 3) | (u8)( b >> 3)         );  // byte1 is lower 3 bits of green, upper 5 of blue

            tilePtr += 2;
        }
    }
}

////////////////////////////////////////////////////////////////////////////
// GC_PackTile_RGBA4442
//
// 4x4 tile, 16-bit texels
// x and y represent starting texel position of this tile
////////////////////////////////////////////////////////////////////////////
static void GC_PackTile_RGBA4443( x_bitmap& BMP, s32 MipID, s32 W, s32 H, u32 x, u32 y, u8* dstPtr )
{
    u32 row, col;
    u32 realRows, realCols;
    u8* tilePtr;
    u8  r, g, b, a;

    // 'realRows', 'realCols' represent actual source image texels remaining
    realRows = H - y;
    realCols = W - x;

    if( realRows > 4 )
        realRows = 4;

    if( realCols > 4 )
        realCols = 4;

    // pack 32B tile 
    for( row = 0; row < realRows; row++ )
    {
        tilePtr = dstPtr + (row * 8);               // move 8 bytes (4 16-bit texels) per row
                                                    // need to reset ptr each row to account for column padding
        for( col = 0; col < realCols; col++ )
        {
            GC_GetPixel( BMP, MipID, (x+col), (y+row), r, g, b, a );

            // check alpha to determine whether to pack color 5551 or 4443
            // since hw replicates msbs, everything >= 224 of original alpha 
            // will unpack as 255 ( 1110 0000 unpacks as 1111 1111 )

            if( a >= 224 )  // pixel is opaque
            {
                // pack in 5551 format, msb is set to 1
                *tilePtr       = (u8)( (0x0080)          | ((r & 0xF8) >> 1) | (g >> 6) );  // byte0 is 1 bit alpha, upper 5-bits
                                                                                            // of red, upper 2-bits of green
                *(tilePtr + 1) = (u8)( ((g & 0x38) << 2) | (b >> 3) );                      // byte1 is bits 3-5 of green, upper 5 of blue
            }
            else            // pixel is translucent
            {
                // pack in 4443 format,  shift alpha by 1 and set msb to 0
                *tilePtr       = (u8)( ( (a >> 1) & 0x70 ) | ((r & 0xF0)   >> 4) );     // byte0 is 1 bit 0, 3 alpha, 4-bits red

                *(tilePtr + 1) = (u8)( (g & 0xF0)          | ((b   & 0xF0)   >> 4) );   // 4-bits green, 4-bits blue
            }

            tilePtr += 2;

        } // end for col loop
    } // end for row loop
}


////////////////////////////////////////////////////////////////////////////
// GC_PackTile_CI8
//
// 4x8 tile
// x and y represent starting texel position of this tile
////////////////////////////////////////////////////////////////////////////
static void GC_PackTile_CI8( x_bitmap& BMP, s32 MipID, s32 W, s32 H, u32 x, u32 y, u8* dstPtr )
{
    u8  index, d;
    u32 row, col;
    u32 realRows, realCols;
    u8* tilePtr;

    // 'realRows', 'realCols' represent actual source image texels remaining
    realRows = H - y;
    realCols = W - x;

    if( realRows > 4 )
        realRows = 4;

    if( realCols > 8 )
        realCols = 8;

    // pack 32B tile
    for( row = 0; row < realRows; row++ )
    {
        tilePtr = dstPtr + (row * 8);               // move 8 bytes (8 8-bit texels) per row
                                                    // need to reset ptr each row to account for column padding
        for( col = 0; col < realCols; col++ )
        {
            // fetch an 8-bit color index value
            GC_GetPixel( BMP, MipID, (x+col), (y+row), index, d, d, d );

            *tilePtr = index;
            tilePtr++;
        }
    }
}


////////////////////////////////////////////////////////////////////////////
// GC_PackTile_CI4
//
// 8x8 tile
// x and y represent starting texel position of this tile
// assume the 8-bit layer indices only run from 0 to 15,
// so take the low 4 bits as is.
////////////////////////////////////////////////////////////////////////////
static void GC_PackTile_CI4( x_bitmap& BMP, s32 MipID, s32 W, s32 H, u32 x, u32 y, u8* dstPtr )
{
    u8  index, d;
    u32 row, col;
    u32 realRows, realCols;
    u8* tilePtr;

    // 'realRows', 'realCols' represent actual source image texels remaining
    realRows = H - y;
    realCols = W - x;

    if( realRows > 8 )
        realRows = 8;

    if( realCols > 8 )
        realCols = 8;

    // pack 32B tile
    for( row = 0; row < realRows; row++ )
    {
        tilePtr = dstPtr + (row * 4);               // move 4 bytes (8 4-bit texels) per row
                                                    // need to reset ptr each row to account for column padding
        for( col = 0; col < realCols; col++ )
        {
            // fetch an 8-bit color-index value and convert it to 4 bits.
            GC_GetPixel( BMP, MipID, (x+col), (y+row), index, d, d, d );
            if( col %2 == 0 )
            {
                *tilePtr = (u8)(index & 0xF0);
            }
            else
            {
                *tilePtr |= ((index >> 4) & 0x0F);
                tilePtr++;
            }
        }
    }
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// SWIZZLE FUNCTIONS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// GC_Swizzle_RGBA8
//
// convert from layer to final hw format
// 2 4x4 texel tiles @ 8B per tile row, 32B per tile, 2 cache lines (64B) total
//
// AR tiles are stored contiguously in the low half of image->tplBuffer;
// GB tiles are stored contiguously in the high half of image->tplBuffer;
//
// note:  source color layer must be in the format LY_IMAGE_COLOR_RGB24.
//        if no alpha layer found, set all alphas to max. value
////////////////////////////////////////////////////////////////////////////
static void GC_Swizzle_RGBA8( x_bitmap& BMP, byte* pDstBuffer )
{
    u32   NTileRows, TileRow;
    u32   NTileCols, TileCol;
    byte* pDstPixData;
    s32   W, H;
    s32   NMips = BMP.GetNMips() + 1;

    if( NMips <= 1 )
    {
        //--- setup x_bitmap for accessing data
        x_bitmap::SetPixelFormat( BMP, 0, 0 );

        pDstPixData = pDstBuffer;
        W = BMP.GetWidth();
        H = BMP.GetHeight();

        // number of 4x4 texel tile cols, rows including any partial tiles
        NTileCols = ((W + 3) >> 2);
        NTileRows = ((H + 3) >> 2);

        // NTileRows, NTileCols includes any partial tiles
        for( TileRow = 0; TileRow < NTileRows; TileRow++ )
        {
            for( TileCol = 0; TileCol < NTileCols; TileCol++ )
            {
                GC_PackTile_RGBA8( BMP, 0, W, H, (TileCol * 4), (TileRow * 4), pDstPixData );
                pDstPixData += 64;       // move to next 2 (32B) cache lines
            }
        }
    }
    else
    {
        s32 MipID;
        for( MipID = 0; MipID < NMips; MipID++ )
        {
            //--- setup x_bitmap for accessing data
            x_bitmap::SetPixelFormat( BMP, 0, MipID );

            pDstPixData = x_gc_bitmap::GetMipDataPtr( pDstBuffer, MipID );

            W = x_gc_bitmap::GetMipWidth( pDstBuffer, MipID );
            H = x_gc_bitmap::GetMipHeight( pDstBuffer, MipID );

            // number of 4x4 texel tile cols, rows including any partial tiles
            NTileCols = ((W + 3) >> 2);
            NTileRows = ((H + 3) >> 2);

            // NTileRows, NTileCols includes any partial tiles
            for( TileRow = 0; TileRow < NTileRows; TileRow++ )
            {
                for( TileCol = 0; TileCol < NTileCols; TileCol++ )
                {
                    GC_PackTile_RGBA8( BMP, MipID, W, H, (TileCol * 4), (TileRow * 4), pDstPixData );
                    pDstPixData += 64;       // move to next 2 (32B) cache lines
                }
            }
        }
    }
}


////////////////////////////////////////////////////////////////////////////
// GC_Swizzle_RGB565
//
// convert from layer to final hw format
// 4x4 texel tiles @ 8B per row, 32B per tile
// note:  color layer must be in LY_IMAGE_COLOR_RGB24 format
////////////////////////////////////////////////////////////////////////////
static void GC_Swizzle_RGB565( x_bitmap& BMP, byte* pDstBuffer )
{
    u32   NTileRows, TileRow;
    u32   NTileCols, TileCol;
    byte* pDstPixData;
    s32   W, H;
    s32   NMips = BMP.GetNMips() + 1;

    if( NMips <= 1 )
    {
        //--- setup x_bitmap for accessing data
        x_bitmap::SetPixelFormat( BMP, 0, 0 );

        pDstPixData = pDstBuffer;
        W = BMP.GetWidth();
        H = BMP.GetHeight();

        // number of 4x4 texel tile cols, rows including any partial tiles
        NTileCols = ((W + 3) >> 2);
        NTileRows = ((H + 3) >> 2);

        // numTileRows, numTileCols includes any partial tiles
        for( TileRow = 0; TileRow < NTileRows; TileRow++ )
        {
            for( TileCol = 0; TileCol < NTileCols; TileCol++ )
            {
                GC_PackTile_RGB565( BMP, 0, W, H, (TileCol * 4), (TileRow * 4), pDstPixData );
                pDstPixData += 32;       // next 32B cache line
            }
        }
    }
    else
    {
        s32 MipID;
        for( MipID = 0; MipID < NMips; MipID++ )
        {
            //--- setup x_bitmap for accessing data
            x_bitmap::SetPixelFormat( BMP, 0, MipID );

            pDstPixData = x_gc_bitmap::GetMipDataPtr( pDstBuffer, MipID );

            W = x_gc_bitmap::GetMipWidth( pDstBuffer, MipID );
            H = x_gc_bitmap::GetMipHeight( pDstBuffer, MipID );

            // number of 4x4 texel tile cols, rows including any partial tiles
            NTileCols = ((W + 3) >> 2);
            NTileRows = ((H + 3) >> 2);

            // numTileRows, numTileCols includes any partial tiles
            for( TileRow = 0; TileRow < NTileRows; TileRow++ )
            {
                for( TileCol = 0; TileCol < NTileCols; TileCol++ )
                {
                    GC_PackTile_RGB565( BMP, MipID, W, H, (TileCol * 4), (TileRow * 4), pDstPixData );
                    pDstPixData += 32;       // next 32B cache line
                }
            }
        }
    }
}


////////////////////////////////////////////////////////////////////////////
// GC_Swizzle_RGBA4443
//
// convert from layer to final hw format
// 4x4 texel tiles @ 8B per row, 32B per tile
//
// note:  source color layer must be in the format LY_IMAGE_COLOR_RGB24.
//        if no alpha layer found, set all alphas to max. value
////////////////////////////////////////////////////////////////////////////
static void GC_Swizzle_RGBA4443( x_bitmap& BMP, byte* pDstBuffer )
{
    u32   NTileRows, TileRow;
    u32   NTileCols, TileCol;
    byte* pDstPixData;
    s32   W, H;
    s32   NMips = BMP.GetNMips() + 1;

    if( NMips <= 1 )
    {
        //--- setup x_bitmap for accessing data
        x_bitmap::SetPixelFormat( BMP, 0, 0 );

        pDstPixData = pDstBuffer;
        W = BMP.GetWidth();
        H = BMP.GetHeight();

        // number of 4x4 texel tile cols, rows including any partial tiles
        NTileCols = ((W + 3) >> 2);
        NTileRows = ((H + 3) >> 2);

        // numTileRows, numTileCols includes any partial tiles
        for( TileRow = 0; TileRow < NTileRows; TileRow++ )
        {
            for( TileCol = 0; TileCol < NTileCols; TileCol++ )
            {
                GC_PackTile_RGBA4443( BMP, 0, W, H, (TileCol * 4), (TileRow * 4), pDstPixData );
                pDstPixData += 32;       // next 32B cache line
            }
        }
    }
    else
    {
        s32 MipID;
        for( MipID = 0; MipID < NMips; MipID++ )
        {
            //--- setup x_bitmap for accessing data
            x_bitmap::SetPixelFormat( BMP, 0, MipID );

            pDstPixData = x_gc_bitmap::GetMipDataPtr( pDstBuffer, MipID );

            W = x_gc_bitmap::GetMipWidth( pDstBuffer, MipID );
            H = x_gc_bitmap::GetMipHeight( pDstBuffer, MipID );

            // number of 4x4 texel tile cols, rows including any partial tiles
            NTileCols = ((W + 3) >> 2);
            NTileRows = ((H + 3) >> 2);

            // numTileRows, numTileCols includes any partial tiles
            for( TileRow = 0; TileRow < NTileRows; TileRow++ )
            {
                for( TileCol = 0; TileCol < NTileCols; TileCol++ )
                {
                    GC_PackTile_RGBA4443( BMP, MipID, W, H, (TileCol * 4), (TileRow * 4), pDstPixData );
                    pDstPixData += 32;       // next 32B cache line
                }
            }
        }
    }
}


////////////////////////////////////////////////////////////////////////////
// GC_Swizzle_CI8
//
// convert from layer to final hw format
// 4x8 texel tiles @ 8B per row, 32B per tile
//
// note:  source color layer must be in the format LY_IMAGE_COLOR_CI8
////////////////////////////////////////////////////////////////////////////
static void GC_Swizzle_CI8( x_bitmap& BMP, byte* pDstBuffer )
{
    u32   NTileRows, TileRow;
    u32   NTileCols, TileCol;
    byte* pDstPixData;
    s32   W, H;
    s32   NMips = BMP.GetNMips() + 1;

    if( NMips <= 1 )
    {
        //--- setup x_bitmap for accessing data
        x_bitmap::SetPixelFormat( BMP, 0, 0 );

        pDstPixData = pDstBuffer;
        W = BMP.GetWidth();
        H = BMP.GetHeight();

        // number of 4x8 texel tile cols, rows including any partial tiles
        NTileCols = ((W + 7) >> 3);
        NTileRows = ((H + 3) >> 2);

        // NTileRows, NTileCols includes any partial tiles
        for( TileRow = 0; TileRow < NTileRows; TileRow++ )
        {
            for( TileCol = 0; TileCol < NTileCols; TileCol++ )
            {
                GC_PackTile_CI8( BMP, 0, W, H, (TileCol * 8), (TileRow * 4), pDstPixData );
                pDstPixData += 32;       // next 32B cache line
            }
        }
    }
    else
    {
        s32 MipID;
        for( MipID = 0; MipID < NMips; MipID++ )
        {
            //--- setup x_bitmap for accessing data
            x_bitmap::SetPixelFormat( BMP, 0, MipID );

            pDstPixData = x_gc_bitmap::GetMipDataPtr( pDstBuffer, MipID );

            W = x_gc_bitmap::GetMipWidth( pDstBuffer, MipID );
            H = x_gc_bitmap::GetMipHeight( pDstBuffer, MipID );

            // number of 4x8 texel tile cols, rows including any partial tiles
            NTileCols = ((W + 7) >> 3);
            NTileRows = ((H + 3) >> 2);

            // NTileRows, NTileCols includes any partial tiles
            for( TileRow = 0; TileRow < NTileRows; TileRow++ )
            {
                for( TileCol = 0; TileCol < NTileCols; TileCol++ )
                {
                    GC_PackTile_CI8( BMP, MipID, W, H, (TileCol * 8), (TileRow * 4), pDstPixData );
                    pDstPixData += 32;       // next 32B cache line
                }
            }
        }
    }
}


////////////////////////////////////////////////////////////////////////////
// GC_Swizzle_CI4
//
// convert from layer to final hw format
// 8x8 texel tiles @ 4B per row, 32B per tile
//
// note:  source color layer must be in the format LY_IMAGE_COLOR_CI8
////////////////////////////////////////////////////////////////////////////
static void GC_Swizzle_CI4( x_bitmap& BMP, byte* pDstBuffer )
{
    u32   NTileRows, TileRow;
    u32   NTileCols, TileCol;
    byte* pDstPixData;
    s32   W, H;
    s32   NMips = BMP.GetNMips() + 1;

    if( NMips <= 1 )
    {
        //--- setup x_bitmap for accessing data
        x_bitmap::SetPixelFormat( BMP, 0, 0 );

        pDstPixData = pDstBuffer;
        W = BMP.GetWidth();
        H = BMP.GetHeight();

        // number of 8x8 texel tile cols, rows including any partial tiles
        NTileCols = ((W + 7) >> 3);
        NTileRows = ((H + 7) >> 3);

        // NTileRows, NTileCols includes any partial tiles
        for( TileRow = 0; TileRow < NTileRows; TileRow++ )
        {
            for( TileCol = 0; TileCol < NTileCols; TileCol++ )
            {
                GC_PackTile_CI4( BMP, 0, W, H, (TileCol * 8), (TileRow * 8), pDstPixData );
                pDstPixData += 32;       // next 32B cache line
            }
        }
    }
    else
    {
        s32 MipID;
        for( MipID = 0; MipID < NMips; MipID++ )
        {
            //--- setup x_bitmap for accessing data
            x_bitmap::SetPixelFormat( BMP, 0, MipID );

            pDstPixData = x_gc_bitmap::GetMipDataPtr( pDstBuffer, MipID );

            W = x_gc_bitmap::GetMipWidth( pDstBuffer, MipID );
            H = x_gc_bitmap::GetMipHeight( pDstBuffer, MipID );

            // number of 8x8 texel tile cols, rows including any partial tiles
            NTileCols = ((W + 7) >> 3);
            NTileRows = ((H + 7) >> 3);

            // NTileRows, NTileCols includes any partial tiles
            for( TileRow = 0; TileRow < NTileRows; TileRow++ )
            {
                for( TileCol = 0; TileCol < NTileCols; TileCol++ )
                {
                    GC_PackTile_CI4( BMP, MipID, W, H, (TileCol * 8), (TileRow * 8), pDstPixData );
                    pDstPixData += 32;       // next 32B cache line
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////
// GC_Swizzle_DXT1
//
// Swizzles a valid S3TC_DXT1 bitmap
//
// Puts compressed blocks(4x4 texels) into 32 byte cache lines- 4 blocks per
// cache line.  Therefore each cache line holds an 8x8 grid of texels, and
// bitmap width and height must be a multiple of 8 texels.
//
// NOTE: When rearranging the compressed blocks, the RGB's are byte-swapped
//       and the 4x4 grid of texels get the rows flipped, so they are assumed
//       to start like this:
//       Byte Order:[0][1][2][3]
//       [00,01,02,03][10,11,12,13][20,21,22,23][30,31,32,33]
//
//       They are then flipped so they are arranged like this:
//       [03,02,01,00][13,12,11,10][23,22,21,20][33,32,31,30]
////////////////////////////////////////////////////////////////////////////

inline u32 GC_FlipDXT1Texels( u32 Texels )
{
    return
    (
        ((Texels & 0xC0000000) >> 6) |
        ((Texels & 0x30000000) >> 2) |
        ((Texels & 0x0C000000) << 2) |
        ((Texels & 0x03000000) << 6) |

        ((Texels & 0x00C00000) >> 6) |
        ((Texels & 0x00300000) >> 2) |
        ((Texels & 0x000C0000) << 2) |
        ((Texels & 0x00030000) << 6) |

        ((Texels & 0x0000C000) >> 6) |
        ((Texels & 0x00003000) >> 2) |
        ((Texels & 0x00000C00) << 2) |
        ((Texels & 0x00000300) << 6) |

        ((Texels & 0x000000C0) >> 6) |
        ((Texels & 0x00000030) >> 2) |
        ((Texels & 0x0000000C) << 2) |
        ((Texels & 0x00000003) << 6)
    );
}

//==========================================================================

static xbool GC_Swizzle_DXT1( x_bitmap& BMP )
{
    // Make sure bitmap is compressed and has DXT1 format
    //ASSERT( BMP.IsCompressed() );
    //ASSERT( BMP.GetFormat() == x_bitmap::FMT_DXT1 );
    //ASSERT( (BMP.GetWidth() % 8) == 0 );
    //ASSERT( (BMP.GetHeight() % 8) == 0 );

    if( !BMP.IsCompressed()                     ||
        (BMP.GetFormat() != x_bitmap::FMT_DXT1) ||
        ((BMP.GetWidth()  % 8) != 0)            ||
        ((BMP.GetHeight() % 8) != 0) )
    {
        //if( (BMP.GetWidth() != 4) && (BMP.GetHeight() != 4) ) // allow a 4x4 image(one DXT1 block)
            return FALSE;
    }

    s32 CurMip;
    s32 NMips;
    s32 NBlocksX;
    s32 NBlocksY;
    s32 BlockOffset[4];
    s32 B;
    s32 SrcBlockX;
    s32 SrcBlockY;
    s32 SwizzleDataSize;

    DXT1_Block* pSrcBMPData;
    byte*       pSwizzleData;
    DXT1_Block* pSrcBlock;
    DXT1_Block* pDstBlock;

    // Create new bitmap data buffer
    SwizzleDataSize = x_gc_bitmap::CalcSwizzleSizeDXTn( BMP.GetWidth(), BMP.GetHeight(), BMP.GetNMips() + 1, TRUE );
    pSwizzleData = (byte*)x_malloc( SwizzleDataSize );
    ASSERT( pSwizzleData != NULL );

    if( pSwizzleData == NULL )
        return FALSE;

    x_memset( pSwizzleData, 0, SwizzleDataSize );

    // These offsets never change, so set them now
    BlockOffset[0] = 1;
    BlockOffset[2] = 1;
    BlockOffset[3] = 0;

    // Check if bitmap has mips or not
    NMips = BMP.GetNMips() + 1;

    if( NMips > 1 )
    {
        x_gc_bitmap::SetupMipTableDXTn( NMips, BMP.GetWidth(), BMP.GetHeight(), pSwizzleData, TRUE );

        // Swizzle each mip level separately
        for( CurMip = 0; CurMip < NMips; CurMip++ )
        {
            NBlocksX = x_gc_bitmap::GetMipWidth ( pSwizzleData, CurMip ) / 4;
            NBlocksY = x_gc_bitmap::GetMipHeight( pSwizzleData, CurMip ) / 4;

            pDstBlock = (DXT1_Block*)x_gc_bitmap::GetMipDataPtr( pSwizzleData, CurMip );

            pSrcBMPData = (DXT1_Block*)BMP.GetDataPtr( CurMip );

            if( NBlocksX < 2 || NBlocksY < 2 )
            {
                // Gamecube docs do not describe how to swizzle textures with dimensions
                // less than 8 pixels, so we'll just copy the blocks directly.
                pSrcBlock = pSrcBMPData;

                for( B = 0; B < (NBlocksX * NBlocksY); B++ )
                {
                    pDstBlock->RGB0   = ENDIAN_SWAP_16( pSrcBlock->RGB0 );
                    pDstBlock->RGB1   = ENDIAN_SWAP_16( pSrcBlock->RGB1 );
                    pDstBlock->Texels = GC_FlipDXT1Texels( pSrcBlock->Texels );

                    pSrcBlock++;
                    pDstBlock++;
                }
            }
            else
            {
                // Set the offset to increment source block data
                BlockOffset[1] = NBlocksX - 1;

                for( SrcBlockY = 0; SrcBlockY < NBlocksY; SrcBlockY += 2 )
                {
                    for( SrcBlockX = 0; SrcBlockX < NBlocksX; SrcBlockX += 2 )
                    {
                        pSrcBlock = &pSrcBMPData[ SrcBlockY * NBlocksX + SrcBlockX ];

                        for( B = 0; B < 4; B++ )
                        {
                            // Byte-swap the RGB's
                            pDstBlock[B].RGB0   = ENDIAN_SWAP_16( pSrcBlock->RGB0 );
                            pDstBlock[B].RGB1   = ENDIAN_SWAP_16( pSrcBlock->RGB1 );

                            // Flip each row of the 4x4 block(order becomes [03,02,01,00],[13,12,11,10],etc.)
                            pDstBlock[B].Texels = GC_FlipDXT1Texels( pSrcBlock->Texels );

                            pSrcBlock += BlockOffset[B];
                        }

                        pDstBlock += 4;
                    }
                }
            }
        }
    }
    else
    {
        // Get bitmap dimensions in DXT1 blocks
        NBlocksX = BMP.GetWidth() / 4;
        NBlocksY = BMP.GetHeight() / 4;

        pDstBlock = (DXT1_Block*)pSwizzleData;

        pSrcBMPData = (DXT1_Block*)BMP.GetDataPtr( 0 );

        if( NBlocksX < 2 || NBlocksY < 2 )
        {
            // Gamecube docs do not describe how to swizzle textures with dimensions
            // less than 8 pixels, so we'll just copy the blocks directly.
            pSrcBlock = pSrcBMPData;

            for( B = 0; B < (NBlocksX * NBlocksY); B++ )
            {
                pDstBlock->RGB0   = ENDIAN_SWAP_16( pSrcBlock->RGB0 );
                pDstBlock->RGB1   = ENDIAN_SWAP_16( pSrcBlock->RGB1 );
                pDstBlock->Texels = GC_FlipDXT1Texels( pSrcBlock->Texels );

                pSrcBlock++;
                pDstBlock++;
            }
        }
        else
        {
            // Set the offset to increment source block data
            BlockOffset[1] = NBlocksX - 1;

            for( SrcBlockY = 0; SrcBlockY < NBlocksY; SrcBlockY += 2 )
            {
                for( SrcBlockX = 0; SrcBlockX < NBlocksX; SrcBlockX += 2 )
                {
                    pSrcBlock = &pSrcBMPData[ SrcBlockY * NBlocksX + SrcBlockX ];

                    for( B = 0; B < 4; B++ )
                    {
                        // Byte-swap the RGB's
                        pDstBlock[B].RGB0   = ENDIAN_SWAP_16( pSrcBlock->RGB0 );
                        pDstBlock[B].RGB1   = ENDIAN_SWAP_16( pSrcBlock->RGB1 );

                        // Flip each row of the 4x4 block(order becomes [03,02,01,00],[13,12,11,10],etc.)
                        pDstBlock[B].Texels = GC_FlipDXT1Texels( pSrcBlock->Texels );

                        pSrcBlock += BlockOffset[B];
                    }

                    pDstBlock += 4;
                }
            }
        }
    }

    //--- set new data and kill off old
    ((x_gc_bitmap*)&BMP)->KillDataPtr();
    ((x_gc_bitmap*)&BMP)->SetDataPtr( TRUE, SwizzleDataSize, pSwizzleData );

    return TRUE;
}


////////////////////////////////////////////////////////////////////////////
// GC_Swizzle_DXT3
//
// Swizzles a valid DXT3 bitmap
//
// Since the gamecube only handles DXT1, this function separates the bitmap
// into its color(DXT1) channels, and its alpha channel(4-bit intensity).
// In order for this format to work on the gamecube, TWO tev stages must be
// used, since it must be treated as two separate textures.
//
// The DXT1 texture is stored first(with it's mips), and then the intensity
// texture(with mips) representing the alpha channel.  The format set is DXT3,
// so application code will have to use that to distinguish the two(otherwise
// the texture will appear as one big chunk of memory).
//
// Also, when mips are present, the miptable for the DXT1 section is stored
// first, followed by the I4 section miptable, then the data.
//
// Mem layout(m_pData):
//   DXT1 miptable(if mips present)
//   I4   miptable(if mips present, identical in size to DXT1 table)
//   DXT1 color data
//   I4   (alpha) data
//
////////////////////////////////////////////////////////////////////////////

static xbool GC_Swizzle_DXT3( x_bitmap& BMP )
{
    s32   x, y;
    s32   W, H;
    s32   NBlocksX, NBlocksY;
    s32   NMips;
    s32   TempDataSize;
    s32   I4DataSize;
    byte* pTempData;
    byte* pI4SwizzleData;

    DXT3_Block* pDXT3Data;
    x_bitmap    TempBMP;

    NMips = BMP.GetNMips() + 1;
    W     = BMP.GetWidth();
    H     = BMP.GetHeight();

    NBlocksX = W / 4;
    NBlocksY = H / 4;


    ///////////////////////////////////////////////////////
    // Alpha channel texture setup
    ///////////////////////////////////////////////////////

    // allocate memory for intermediate texture to hold alpha data,
    // and also allocate the buffer for the swizzled data
    TempDataSize = x_gc_bitmap::CalcSwizzleSize( W, H, NMips, 4 );

    pTempData = (byte*)x_malloc( TempDataSize );
    ASSERT( pTempData != NULL );

    if( pTempData == NULL )
        return FALSE;

    pI4SwizzleData = (byte*)x_malloc( TempDataSize );
    ASSERT( pI4SwizzleData != NULL );

    if( pI4SwizzleData == NULL )
    {
        x_free( pTempData );
        return FALSE;
    }

    I4DataSize = TempDataSize;

    // copy the alpha channel from the DXT3 blocks into a separate buffer(I4 format)
    if( NMips > 1 )
    {
        x_gc_bitmap::SetupMipTable( NMips, W, H, 4, pI4SwizzleData );
        //*****************************************
        //do mip processing here
        //*****************************************
    }
    else
    {
        pDXT3Data = (DXT3_Block*)BMP.GetDataPtr();

        for( y = 0; y < H; y++ )
        {
            byte* pI4Byte = &pTempData[y * (W/2)];

            for( x = 0; x < NBlocksX; x++ )
            {
                u16 BlockRow = pDXT3Data[(y/4)*NBlocksX + x].Alpha.row[y%4];

                pI4Byte[0] = (u8)( ((BlockRow & 0x00F0)>> 4) | ((BlockRow & 0x000F)<<4) );
                pI4Byte[1] = (u8)( ((BlockRow & 0xF000)>>12) | ((BlockRow & 0x0F00)>>4) );

                pI4Byte += 2;
            }
        }
    }

    // create a bitmap representing the alpha texture, and swizzle the data
    TempBMP.SetupBitmap( x_bitmap::FMT_I4, W, H, W, H, TRUE, TempDataSize, pTempData, FALSE, 0, NULL, 0, NMips-1, FALSE );

    GC_Swizzle_CI4( TempBMP, pI4SwizzleData );

    TempBMP.KillBitmap();


    ///////////////////////////////////////////////////////
    // DXT1 color channel setup
    ///////////////////////////////////////////////////////

    // allocate memory for the intermediate DXT1 texture holding the color data
    TempDataSize = x_gc_bitmap::CalcSwizzleSizeDXTn( W, H, NMips, TRUE );

    pTempData = (byte*)x_malloc( TempDataSize );
    ASSERT( pTempData != NULL );

    if( pTempData == NULL )
        return FALSE;

    // copy the DXT1 blocks out of the DXT3 data
    if( NMips > 1 )
    {
        x_gc_bitmap::SetupMipTableDXTn( NMips, W, H, pTempData, TRUE );
        //*****************************************
        //do mip processing here
        //*****************************************
    }
    else
    {
        pDXT3Data = (DXT3_Block*)BMP.GetDataPtr();

        for( x = 0; x < (NBlocksX * NBlocksY); x++ )
            ((DXT1_Block*)pTempData)[x] = pDXT3Data[x].Color;
    }

    // create a bitmap representing the color DXT1 texture, and swizzle it
    TempBMP.SetupBitmap( x_bitmap::FMT_DXT1, W, H, W, H, TRUE, TempDataSize, pTempData, FALSE, 0, NULL, 0, NMips-1, FALSE );
    TempBMP.SetCompressed();

    if( FALSE == GC_Swizzle_DXT1( TempBMP ) )
    {
        x_free( pI4SwizzleData );
        return FALSE;
    }

    ///////////////////////////////////////////////////////
    // Combine DXT1 color and I4 alpha channels
    ///////////////////////////////////////////////////////

    // allocate memory for final data of texture
    TempDataSize = x_gc_bitmap::CalcSwizzleSizeDXTn( W, H, NMips, FALSE );

    ASSERT( TempDataSize >= (TempBMP.GetTotalDataSize() + I4DataSize) );

    pTempData = (byte*)x_malloc( TempDataSize );
    ASSERT( pTempData != NULL );

    if( pTempData == NULL )
    {
        x_free( pI4SwizzleData );
        TempBMP.KillBitmap();
        return FALSE;
    }

    // copy the color data and alpha channel into final buffer
    if( NMips > 1 )
    {
        x_gc_bitmap::SetupMipTableDXTn( NMips, W, H, pTempData, FALSE );
        //*****************************************
        //process mips here
        //*****************************************
    }
    else
    {
        x_memcpy( pTempData, TempBMP.GetDataPtr(), TempBMP.GetDataSize() );
        x_memcpy( pTempData + TempBMP.GetDataSize(), pI4SwizzleData, I4DataSize );
    }

    // clean up
    x_free( pI4SwizzleData );
    TempBMP.KillBitmap();

    // kill old dxt3 data, and set pointer to new data
    ((x_gc_bitmap*)&BMP)->KillDataPtr();
    ((x_gc_bitmap*)&BMP)->SetDataPtr( TRUE, TempDataSize, pTempData );

    return TRUE;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

xbool AUXBMP_SwizzleGameCube( x_bitmap& BMP )
{
    s32   BPP;
    s32   SwizzleDataSize;
    byte* pSwizzleData;

    x_bitmap::format Fmt;

    BPP = BMP.GetBPP();
    Fmt = BMP.GetFormat();

    //--- verify supported formats
    ASSERT( (BPP ==  4) ||
            (BPP ==  8) ||
            (BPP == 16) ||
            (BPP == 32) );

    if( Fmt == x_bitmap::FMT_DXT3 )
    {
        if( FALSE == GC_Swizzle_DXT3( BMP ) )
            return FALSE;
    }
    else if( Fmt == x_bitmap::FMT_DXT1 )
    {
        if( FALSE == GC_Swizzle_DXT1( BMP ) )
            return FALSE;
    }
    else
    {
        //--- allocate new buffer for bitmap
        SwizzleDataSize = x_gc_bitmap::CalcSwizzleSize( BMP.GetWidth(), BMP.GetHeight(), BMP.GetNMips() + 1, BPP );
        pSwizzleData    = (byte*)x_malloc( SwizzleDataSize );
        ASSERT( pSwizzleData != NULL );

        if( pSwizzleData == NULL )
            return FALSE;

        x_memset( pSwizzleData, 0, SwizzleDataSize );

        if( BMP.GetNMips() > 0 )
            x_gc_bitmap::SetupMipTable( BMP.GetNMips() + 1, BMP.GetWidth(), BMP.GetHeight(), BPP, pSwizzleData );

        //--- swizzle the data
        switch( BPP )
        {
            case 4:
                GC_Swizzle_CI4( BMP, pSwizzleData );
                break;

            case 8:
                GC_Swizzle_CI8( BMP, pSwizzleData );
                break;

            case 16:
                if( Fmt == x_bitmap::FMT_16_RGB_565 )
                    GC_Swizzle_RGB565( BMP, pSwizzleData );
                else
                    GC_Swizzle_RGBA4443( BMP, pSwizzleData );

                break;

            case 32:
                GC_Swizzle_RGBA8( BMP, pSwizzleData );
                break;

            default:
                ASSERT( FALSE );
                break;
        }

        //--- set new data and kill off old
        ((x_gc_bitmap*)&BMP)->KillDataPtr();
        ((x_gc_bitmap*)&BMP)->SetDataPtr( TRUE, SwizzleDataSize, pSwizzleData );
    }

    #if defined( TARGET_DOLPHIN )
        DCFlushRange( (void*)BMP.GetRawDataPtr(), BMP.GetTotalDataSize() );
    #endif

    BMP.SetDataSwizzled();

    return TRUE;
}


////////////////////////////////////////////////////////////////////////////
#endif //defined( TARGET_DOLPHIN ) || defined( TARGET_PC )