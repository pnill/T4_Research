////////////////////////////////////////////////////////////////////////////
//
//  AUX_Swizzle.cpp
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


////////////////////////////////////////////////////////////////////////////
// PROTOTYPES
////////////////////////////////////////////////////////////////////////////
#if defined( TARGET_PS2 ) || defined( TARGET_PC )

static void PS2_BlockConv4To32 ( byte* pInput, byte* pOutput );
static void PS2_PageConv4To32  ( s32 width, s32 height, u8* p_input, u8* p_output );
static void PS2_Swizzle4BitData( byte* pDst, byte* pSrc, s32 W, s32 H );
static void PS2_Swizzle4Bit    ( x_bitmap& rBMP );

static void PS2_BlockConv8To32 ( byte* pInput, byte* pOutput );
static void PS2_PageConv8To32  ( byte* pInput, byte* pOutput );
static void PS2_Swizzle8BitData( byte* pDst, byte* pSrc, s32 W, s32 H );
static void PS2_Swizzle8Bit    ( x_bitmap& rBMP );

#endif // if defined( TARGET_PS2 ) || defined( TARGET_PC )


////////////////////////////////////////////////////////////////////////////
// Generic platform swizzle functions.
////////////////////////////////////////////////////////////////////////////

void AUXBMP_ClutSwizzle( x_bitmap& Bitmap )
{
    s32  i, j, idx;
    u32* P;
    u32  Clut8[256];

    if( Bitmap.GetBPP() != 8 ) 
        return;

    P = (u32*)Bitmap.GetClutPtr();

    idx = 0;
    for( i = 0; i < Bitmap.GetNClutColors(); i+=32 )
    {
        for( j = i;    j < i+8;    j++ )  Clut8[ idx++ ] = P[j];
        for( j = i+16; j < i+16+8; j++ )  Clut8[ idx++ ] = P[j];
        for( j = i+8;  j < i+8+8;  j++ )  Clut8[ idx++ ] = P[j];
        for( j = i+24; j < i+24+8; j++ )  Clut8[ idx++ ] = P[j];
    }

    for( i = 0; i < Bitmap.GetNClutColors(); i++ )
        P[i] = Clut8[i];

    Bitmap.SetClutSwizzled();
}


////////////////////////////////////////////////////////////////////////////
// PS2 Specific Swizzlers
////////////////////////////////////////////////////////////////////////////

#if defined( TARGET_PS2 ) || defined( TARGET_PC )

// the following values can be gleaned from section 8 in the GS User's Manual version 3.0
static const s32    PS2_PAGE_WIDTH_4BIT         = 128;
static const s32    PS2_PAGE_HEIGHT_4BIT        = 128;
static const s32    PS2_PAGE_WIDTH_8BIT         = 128;
static const s32    PS2_PAGE_HEIGHT_8BIT        = 64;
static const s32    PS2_PAGE_WIDTH_32BIT        = 64;
static const s32    PS2_PAGE_HEIGHT_32BIT       = 32;
static const s32    PS2_BLOCK_WIDTH_4BIT        = 32;
static const s32    PS2_BLOCK_HEIGHT_4BIT       = 16;
static const s32    PS2_BLOCK_WIDTH_8BIT        = 16;
static const s32    PS2_BLOCK_HEIGHT_8BIT       = 16;
static const s32    PS2_BLOCK_WIDTH_32BIT       = 8;
static const s32    PS2_BLOCK_HEIGHT_32BIT      = 8;
static const s32    PS2_PAGE_W_IN_BLOCKS_4BIT   = 4;    //(PAGE_WIDTH/BLOCK_WIDTH)
static const s32    PS2_PAGE_H_IN_BLOCKS_4BIT   = 8;    //(PAGE_HEIGHT/BLOCK_HEIGHT)
static const s32    PS2_PAGE_W_IN_BLOCKS_8BIT   = 8;    //(PAGE_WIDTH/BLOCK_WIDTH)
static const s32    PS2_PAGE_H_IN_BLOCKS_8BIT   = 4;    //(PAGE_HEIGHT/BLOCK_HEIGHT)
static const s32    PS2_PAGE_W_IN_BLOCKS_32BIT  = 8;    //(PAGE_WIDTH/BLOCK_WIDTH)
static const s32    PS2_PAGE_H_IN_BLOCKS_32BIT  = 4;    //(PAGE_HEIGHT/BLOCK_HEIGHT)

////////////////////////////////////////////////////////////////////////////

static
void PS2_BlockConv4To32( u8* p_input, u8* p_output )
{
    static int lut[] =
    {
        // even column
        0, 68, 8,  76, 16, 84, 24, 92,
        1, 69, 9,  77, 17, 85, 25, 93,
        2, 70, 10, 78, 18, 86, 26, 94,
        3, 71, 11, 79, 19, 87, 27, 95,
        4, 64, 12, 72, 20, 80, 28, 88,
        5, 65, 13, 73, 21, 81, 29, 89,
        6, 66, 14, 74, 22, 82, 30, 90,
        7, 67, 15, 75, 23, 83, 31, 91,

        32, 100, 40, 108, 48, 116, 56, 124,
        33, 101, 41, 109, 49, 117, 57, 125,
        34, 102, 42, 110, 50, 118, 58, 126,
        35, 103, 43, 111, 51, 119, 59, 127,
        36, 96,  44, 104, 52, 112, 60, 120,
        37, 97,  45, 105, 53, 113, 61, 121,
        38, 98,  46, 106, 54, 114, 62, 122,
        39, 99,  47, 107, 55, 115, 63, 123,

        // odd column
        4, 64, 12, 72, 20, 80, 28, 88,
        5, 65, 13, 73, 21, 81, 29, 89,
        6, 66, 14, 74, 22, 82, 30, 90,
        7, 67, 15, 75, 23, 83, 31, 91,
        0, 68, 8,  76, 16, 84, 24, 92,
        1, 69, 9,  77, 17, 85, 25, 93,
        2, 70, 10, 78, 18, 86, 26, 94,
        3, 71, 11, 79, 19, 87, 27, 95,

        36, 96,  44, 104, 52, 112, 60, 120,
        37, 97,  45, 105, 53, 113, 61, 121,
        38, 98,  46, 106, 54, 114, 62, 122,
        39, 99,  47, 107, 55, 115, 63, 123,
        32, 100, 40, 108, 48, 116, 56, 124,
        33, 101, 41, 109, 49, 117, 57, 125,
        34, 102, 42, 110, 50, 118, 58, 126,
        35, 103, 43, 111, 51, 119, 59, 127
    };

    u32 i, j, k;
    u32 i0, i1, i2;
    u32 index0, index1;
    u8  c_in, c_out, *pIn;

    pIn = p_input;

    // for first step, we only think for a single block. (4bits, 32x16)
    index1 = 0;

    for( k = 0; k < 4; k++ )
    {
        index0 = (k % 2) * 128;

        for( i = 0; i < 16; i++ )
        {
            for( j = 0; j < 4; j++ )
            {
                c_out = 0x00;

                // lower 4bit.
                i0 = lut[index0++];
                i1 = i0 / 2;
                i2 = (i0 & 0x1) * 4;
                c_in = (pIn[i1] & (0x0f << i2)) >> i2;
                c_out = c_out | c_in;

                // uppper 4bit
                i0 = lut[index0++];
                i1 = i0 / 2;
                i2 = (i0 & 0x1) * 4;
                c_in = (pIn[i1] & (0x0f << i2)) >> i2;
                c_out = c_out | ((c_in << 4) & 0xf0);

                p_output[index1++] = c_out;
            }
        }
        pIn += 64;
    }
}

////////////////////////////////////////////////////////////////////////////

static
void PS2_PageConv4To32( s32 width, s32 height, u8* p_input, u8* p_output )
{
    static u32 block_table4[] =
    {
         0,  2,  8, 10,
         1,  3,  9, 11,
         4,  6, 12, 14,
         5,  7, 13, 15,
        16, 18, 24, 26,
        17, 19, 25, 27,
        20, 22, 28, 30,
        21, 23, 29, 31
    };

    static u32 block_table32[] =
    {
         0,  1,  4,  5, 16, 17, 20, 21,
         2,  3,  6,  7, 18, 19, 22, 23,
         8,  9, 12, 13, 24, 25, 28, 29,
        10, 11, 14, 15, 26, 27, 30, 31
    };

    u32 index32_h[8 * 4];
    u32 index32_v[8 * 4];
    u32 in_block_nb;

    u8  input_block[16 * 16];
    u8  output_block[16 * 16];
    u8* pi0;
    u8* pi1;
    u8* po0;
    u8* po1;
    s32 index0, index1;
    s32 i, j, k;
    s32 n_width, n_height;
    s32 input_page_line_size;
    s32 output_page_line_size;


    index0 = 0;
    for( i = 0; i < 4; i++ )
    {
        for( j = 0; j < 8; j++ )
        {
            index1 = block_table32[index0];
            index32_h[index1] = j;
            index32_v[index1] = i;
            index0++;
        }
    }


    n_width  = width / 32;
    n_height = height / 16;

    x_memset( input_block,  0, 16 * 16 );
    x_memset( output_block, 0, 16 * 16 );

    input_page_line_size  = 128 / 2;   // PSMT4 page width (byte)
    output_page_line_size = 256;       // PSMCT32 page width (byte)

    // now assume copying from page top. 
    for( i = 0; i < n_height; i++ )
    {
        for( j = 0; j < n_width; j++ )
        {
            pi0 = input_block;
            pi1 = p_input + 16 * i * input_page_line_size + j * 16;

            in_block_nb = block_table4[i * n_width + j]; 

            for( k = 0; k < PS2_BLOCK_HEIGHT_4BIT; k++ )
            {
                x_memcpy( pi0, pi1, PS2_BLOCK_WIDTH_4BIT / 2 ); // copy full 1 line of 1 block.
                pi0 += PS2_BLOCK_WIDTH_4BIT / 2;
                pi1 += input_page_line_size;
            }

            PS2_BlockConv4To32( input_block, output_block );

            po0 = output_block;
            po1 = p_output + 8 * index32_v[in_block_nb] * output_page_line_size + index32_h[in_block_nb] * 32;
            for( k = 0; k < PS2_BLOCK_HEIGHT_32BIT; k++ )
            {
                x_memcpy( po1, po0, PS2_BLOCK_WIDTH_32BIT * 4 );
                po0 += PS2_BLOCK_WIDTH_32BIT * 4;
                po1 += output_page_line_size;   
            }
        }
    }

}

////////////////////////////////////////////////////////////////////////////

static
void PS2_Swizzle4BitData( u8* p_input, u8* p_output, s32 width, s32 height )
{
    s32 i, j, k;
    s32 n_page_h, n_page_w;
    s32 n_page4_width_byte, n_page32_width_byte;
    u8* pi0;
    u8* pi1;
    u8* po0;
    u8* po1;
    s32 n_input_width_byte, n_output_width_byte;
    s32 n_input_height, n_output_height;
    u8  input_page[PS2_PAGE_WIDTH_4BIT / 2 * PS2_PAGE_HEIGHT_4BIT];
    u8  output_page[PS2_PAGE_WIDTH_32BIT * 4 * PS2_PAGE_HEIGHT_32BIT];

    //---   verify dimensions are a power of 2
    ASSERT( !(width & (width-1)) );
    ASSERT( !(height & (height-1)) );

    x_memset( input_page,  0, PS2_PAGE_WIDTH_4BIT / 2 * PS2_PAGE_HEIGHT_4BIT );
    x_memset( output_page, 0, PS2_PAGE_WIDTH_32BIT * 4 * PS2_PAGE_HEIGHT_32BIT );

    n_page_w = (width - 1) / PS2_PAGE_WIDTH_4BIT + 1;
    n_page_h = (height - 1) / PS2_PAGE_HEIGHT_4BIT + 1;

    n_page4_width_byte  = PS2_PAGE_WIDTH_4BIT / 2;
    n_page32_width_byte = PS2_PAGE_WIDTH_32BIT * 4;

    // --- set in/out buffer size (for image smaller than one page) ---
    if( n_page_w == 1 )
    {
        n_input_width_byte = width / 2;
        n_output_height    = width / 4;
    }
    else
    {
        n_input_width_byte = n_page4_width_byte;
        n_output_height    = PS2_PAGE_HEIGHT_32BIT;
    }

    if( n_page_h == 1 )
    {
        n_input_height      = height;
        n_output_width_byte = height * 2;
    }
    else
    {
        n_input_height      = PS2_PAGE_HEIGHT_4BIT;
        n_output_width_byte = n_page32_width_byte;
    }


    for( i = 0; i < n_page_h; i++ )
    {
        for( j = 0; j < n_page_w; j++ )
        {
            pi0 = p_input + (n_input_width_byte * PS2_PAGE_HEIGHT_4BIT) * n_page_w * i + n_input_width_byte * j;
            pi1 = input_page;

            for( k = 0; k < n_input_height; k++ )
            {
                x_memcpy( pi1, pi0, n_input_width_byte );
                pi0 += n_input_width_byte * n_page_w;
                pi1 += n_page4_width_byte;
            }

            PS2_PageConv4To32( PS2_PAGE_WIDTH_4BIT, PS2_PAGE_HEIGHT_4BIT, input_page, output_page );

            po0 = p_output + (n_output_width_byte * PS2_PAGE_HEIGHT_32BIT) * n_page_w * i + n_output_width_byte * j;
            po1 = output_page;
            for( k = 0; k < n_output_height; k++ )
            {
                x_memcpy( po0, po1, n_output_width_byte );
                po0 += n_output_width_byte * n_page_w;
                po1 += n_page32_width_byte;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////

static
void PS2_Swizzle4Bit( x_bitmap& rBMP )
{
    s32     i;
    byte*   pTemp;

    //---   allocate some temporary storage for munged data
    pTemp = (byte*)x_malloc( rBMP.GetWidth() * rBMP.GetHeight() / 2 );
    ASSERT( pTemp );

    //---   munge each mip
    for ( i = 0; i <= rBMP.GetNMips(); i++ )
    {
        PS2_Swizzle4BitData( pTemp, rBMP.GetDataPtr(i), rBMP.GetWidth(i), rBMP.GetHeight(i) );
        x_memcpy( rBMP.GetDataPtr(i), pTemp, rBMP.GetWidth(i) * rBMP.GetHeight(i) / 2 );
    }

    //---   clean up, eh? its a good idea...
    x_free( pTemp );
}

////////////////////////////////////////////////////////////////////////////

static
void PS2_BlockConv8To32( byte* pInput, byte* pOutput )
{
    static u32 PixelLUT[] =     // from section 8.6.2 in GS User's manual v. 3.0
    {
        // even column
        0, 36, 8,  44,
        1, 37, 9,  45,
        2, 38, 10, 46,
        3, 39, 11, 47,
        4, 32, 12, 40,
        5, 33, 13, 41,
        6, 34, 14, 42,
        7, 35, 15, 43,

        16, 52, 24, 60,
        17, 53, 25, 61,
        18, 54, 26, 62,
        19, 55, 27, 63, 
        20, 48, 28, 56,
        21, 49, 29, 57,
        22, 50, 30, 58,
        23, 51, 31, 59,

        // odd column
        4, 32, 12, 40,
        5, 33, 13, 41,
        6, 34, 14, 42,
        7, 35, 15, 43,
        0, 36, 8,  44,
        1, 37, 9,  45,
        2, 38, 10, 46,
        3, 39, 11, 47,

        20, 48, 28, 56,
        21, 49, 29, 57,
        22, 50, 30, 58,
        23, 51, 31, 59,
        16, 52, 24, 60,
        17, 53, 25, 61,
        18, 54, 26, 62,
        19, 55, 27, 63
    };

    s32     i, j;
    byte*   pIn;
    byte*   pOut;
    u32     LutIndex;
    u32     SrcByteIndex;

    pIn = pInput;
    pOut = pOutput;

    //---   loop through each column
    for( j = 0; j < 4; j++ )
    {
        //---   ODD or EVEN lut?
        LutIndex = (j % 2) * 64;

        //---   64 pixels in a column
        for( i = 0; i < 64; i++ )
        {
            //---   the src byte is...
            SrcByteIndex = PixelLUT[LutIndex++];

            //---   which means the dest byte is...
            *pOut = pIn[SrcByteIndex];
            pOut++;
        }
        pIn += 64;
    }
}

////////////////////////////////////////////////////////////////////////////

static
void PS2_PageConv8To32( byte* pInput, byte* pOutput )
{
    static u32 BlockTable[] =
    {
        // from sections 8.3.1 and 8.3.3 in GS User's Manual version 3.0.
        // In this case the block layout is the same bewteen 8- and 32-bit,
        // but may not be for 16-bit or 4-bit conversions!
         0,  1,  4,  5, 16, 17, 20, 21,
         2,  3,  6,  7, 18, 19, 22, 23,
         8,  9, 12, 13, 24, 25, 28, 29,
        10, 11, 14, 15, 26, 27, 30, 31
    };

    s32  i, j, k;
    byte InputBlock[PS2_BLOCK_WIDTH_8BIT * PS2_BLOCK_HEIGHT_8BIT];
    byte OutputBlock[PS2_BLOCK_WIDTH_32BIT * 4 * PS2_BLOCK_HEIGHT_32BIT];
    u32  DestIndexH[32];
    u32  DestIndexV[32];
    s32  Index0, Index1;
    s32  NInputLineBytes;
    s32  NOutputLineBytes;
    s32  TableIndex;

    byte*   pIn0;
    byte*   pIn1;
    byte*   pOut0;
    byte*   pOut1;

    //---   create a table for figuring out destination block addresses
    Index0 = 0;
    for( i = 0; i < 4; i++ )
    {
        for( j = 0; j < 8; j++ )
        {
            Index1 = BlockTable[Index0];
            DestIndexH[Index1] = j;
            DestIndexV[Index1] = i;
            Index0++;
        }
    }

    NInputLineBytes  = PS2_PAGE_WIDTH_8BIT;
    NOutputLineBytes = PS2_PAGE_WIDTH_32BIT * 4;

    //---   now start copying
    for( i = 0; i < PS2_PAGE_H_IN_BLOCKS_8BIT; i++ )
    {
        for( j = 0; j < PS2_PAGE_W_IN_BLOCKS_8BIT; j++ )
        {
            //---   copy the input block to a temp buffer for conversion
            pIn0 = pInput +                                         // src data +
                  PS2_BLOCK_HEIGHT_8BIT * i * NInputLineBytes +     // row offset +
                  j * PS2_BLOCK_WIDTH_8BIT;                         // col offset
            pIn1 = InputBlock;

            for( k = 0; k < PS2_BLOCK_HEIGHT_8BIT; k++ )
            {
                x_memcpy( pIn1, pIn0, PS2_BLOCK_WIDTH_8BIT );
                pIn0 += NInputLineBytes;
                pIn1 += PS2_BLOCK_WIDTH_8BIT;
            }

            //---   convert the block inside our temp buffers
            PS2_BlockConv8To32( InputBlock, OutputBlock );

            //---   move the output block from the temp buffer to output buffer
            TableIndex = BlockTable[i * PS2_PAGE_W_IN_BLOCKS_8BIT + j];
            pOut0 = pOutput +                                                               // dst buffer +
                    PS2_BLOCK_HEIGHT_32BIT * DestIndexV[TableIndex] * NOutputLineBytes +    // row offset +
                    DestIndexH[TableIndex] * PS2_BLOCK_HEIGHT_32BIT * 4;                    // col offset
            pOut1 = OutputBlock;
            for( k = 0; k < PS2_BLOCK_HEIGHT_32BIT; k++ )
            {
                x_memcpy( pOut0, pOut1, PS2_BLOCK_WIDTH_32BIT * 4 );
                pOut1 += PS2_BLOCK_WIDTH_32BIT * 4;
                pOut0 += NOutputLineBytes;   
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////

static
void PS2_Swizzle8BitData( byte* pDst, byte* pSrc, s32 W, s32 H )
{
    s32 i, j, k;
    s32 NPagesAcross, NPagesDown;
    s32 InputPageW, InputPageH;
    s32 OutputPageW, OutputPageH;

    byte InputPage[PS2_PAGE_WIDTH_8BIT * PS2_PAGE_HEIGHT_8BIT];
    byte OutputPage[PS2_PAGE_WIDTH_32BIT * 4 * PS2_PAGE_HEIGHT_32BIT];

    byte* pIn0;
    byte* pIn1;
    byte* pOut0;
    byte* pOut1;

    //---   verify dimensions are a power of 2
    ASSERT( !(W & (W-1)) );
    ASSERT( !(H & (H-1)) );

    //---   because weird things will happen if you start swizzling a texture
    //      less than the block width, and it won't really save much, we won't
    //      bother doing it!
    if( W < PS2_BLOCK_WIDTH_8BIT )
    {
        x_memcpy( pDst, pSrc, W*H );
        return;
    }

    //---   how many pages does this texture use?
    NPagesAcross = (W-1) / PS2_PAGE_WIDTH_8BIT + 1;
    NPagesDown   = (H-1) / PS2_PAGE_HEIGHT_8BIT + 1;

    //---   set in/out buffer size (for image smaller than one page)
    if( NPagesAcross == 1 )
    {
        InputPageW  = W;
        OutputPageW = W * 2;    // * ((PAGE_WIDTH_32BIT / PAGE_WIDTH_8BIT)*4)
    }
    else
    {
        InputPageW  = PS2_PAGE_WIDTH_8BIT;
        OutputPageW = PS2_PAGE_WIDTH_32BIT * 4;
    }

    if( NPagesDown == 1 )
    {
        InputPageH  = H;
        OutputPageH = H / 2;    // * (PAGE_HEIGHT_32BIT / PAGE_HEIGTH_8BIT)
    }
    else
    {
        InputPageH  = PS2_PAGE_HEIGHT_8BIT;
        OutputPageH = PS2_PAGE_HEIGHT_32BIT;
    }

    //---   do the conversion
    for( i = 0; i < NPagesDown; i++ )
    {
        for( j = 0; j < NPagesAcross; j++ )
        {
            //---   copy the input page to a temp buffer for conversion
            pIn0 = pSrc +                                                       // src data +
                   (InputPageW * PS2_PAGE_HEIGHT_8BIT) * NPagesAcross * i +     // page row offset +
                   InputPageW * j;                                              // page col offset
            pIn1 = InputPage;

            for( k = 0; k < InputPageH; k++ )
            {
                x_memcpy( pIn1, pIn0, InputPageW );
                pIn0 += InputPageW * NPagesAcross;
                pIn1 += PS2_PAGE_WIDTH_8BIT;
            }

            //---   convert the page inside our temp buffers
            PS2_PageConv8To32( InputPage, OutputPage );

            //---   move the output page from temp buffer to output buffer
            pOut0 = pDst +                                                  // dst data +
                    (OutputPageW * OutputPageH) * NPagesAcross * i +        // page row offset +
                    OutputPageW * j;                                        // page col offset
            pOut1 = OutputPage;
            for( k = 0; k < OutputPageH; k++ )
            {
                x_memcpy( pOut0, pOut1, OutputPageW );
                pOut0 += OutputPageW * NPagesAcross;
                pOut1 += PS2_PAGE_WIDTH_32BIT*4;
            }
        }
    }

}

////////////////////////////////////////////////////////////////////////////

static
void PS2_Swizzle8Bit( x_bitmap& rBMP )
{
    s32     i;
    byte*   pTemp;

    //---   allocate some temporary storage for munged data
    pTemp = (byte*)x_malloc( rBMP.GetWidth() * rBMP.GetHeight() );
    ASSERT( pTemp );

    //---   munge each mip
    for ( i = 0; i <= rBMP.GetNMips(); i++ )
    {
        PS2_Swizzle8BitData( pTemp, rBMP.GetDataPtr(i), rBMP.GetWidth(i), rBMP.GetHeight(i) );
        x_memcpy( rBMP.GetDataPtr(i), pTemp, rBMP.GetWidth(i) * rBMP.GetHeight(i) );
    }

    //---   clean up, eh? its a good idea...
    x_free( pTemp );
}

////////////////////////////////////////////////////////////////////////////

void AUXBMP_DataSwizzlePS2( x_bitmap& Bitmap )
{
    if( Bitmap.GetBPP() == 32 )
    {
        // no need to swizzle 32 bit textures
    }
    else
    if( Bitmap.GetBPP() == 8 )
    {
        PS2_Swizzle8Bit( Bitmap );
        Bitmap.SetDataSwizzled();
    }
    else
    if( Bitmap.GetBPP() == 4 )
    {
        //PS2_Swizzle4Bit( Bitmap ); //### 4-bit SWIZZLE NOT QUITE SUPPORTED IN VRAM
        //Bitmap.SetDataSwizzled();
    }
    else
    {
        ASSERTS( FALSE, "No support swizzling for 16-bit textures yet!" );
    }
}

#endif // #if defined( TARGET_PS2 ) || defined( TARGET_PC )

////////////////////////////////////////////////////////////////////////////
// End of PS2 Specific Swizzlers
////////////////////////////////////////////////////////////////////////////