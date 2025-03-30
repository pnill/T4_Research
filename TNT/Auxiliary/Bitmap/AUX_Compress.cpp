////////////////////////////////////////////////////////////////////////////
//
// AUX_Compress.cpp
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
#include "x_math.hpp"

#include "AUX_Bitmap.hpp"



////////////////////////////////////////////////////////////////////////////
// TYPES
////////////////////////////////////////////////////////////////////////////

#define SQR(a)      ((a)*(a))


struct color_block
{
    u16     col0;       // 565 min extreme (color00)
    u16     col1;       // 565 max extreme (color01)

    u8      row[4];     // 1byte per row for the 4x4 color block, 2 bits per pixel
};

struct explicit_alpha_block
{
    u16     row[4];     // 1word per row for the 4x4 alpha block, 4 bits per pixel
};

struct implicit_alpha_block
{
    u8      alpha0;     // min alpha (alpha00)
    u8      alpha1;     // max alpha (alpha01)

    u16     row[3];     // 3bits per pixel alpha value
};


////////////////////////////////////////////////////////////////////////////
// VARIABLES
////////////////////////////////////////////////////////////////////////////

static color_block*             pColorBlocks         = NULL;
static explicit_alpha_block*    pExplicitAlphaBlocks = NULL;
static implicit_alpha_block*    pImplicitAlphaBlocks = NULL;

static vector3                  colorBlock[16];     // 4x4 block of 565 colors
static u8                       alphaBlock[16];     // 4x4 block of 8bit alpha

static vector3                  uniqueColors[16];   // list of unique colors used in the colorBlock
static s32                      nUniqueColors;      // number of unique colors used
static u8                       uniqueAlphas[16];   // list of unique alphas used in the alphaBlock
static s32                      nUniqueAlphas;      // number of unique alphas used

static vector3                  linearColors[4];    // the 4 colors that this block will use
static u8                       linearAlphas[8];    // the 6 alphas that this block will use

static xbool                    bAlpha0;            // TRUE if alpha contains 0
static xbool                    bAlpha255;          // TRUE if alpha contains 255

static s32                      destFormat;

const u32 ScanRange = 32;   // Uses a range of +/- ScanRange/2 for brute force
const u32 ScanStep  = 4;    // Step increment for the brute force scan


////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// count how many unique alpha values are used across the entire bitmap.
// this lets us decide the best format for conversion
////////////////////////////////////////////////////////////////////////////
static
s32 AlphaCount( x_bitmap& Bitmap )
{
    s32     x, y, c;
    xbool   bFound;
    u8      alphaValues[256];
    u8      alpha;

    // re-using the nUniqueAlphas variable,
    // since this is only called once at the start of compression,
    // we won't screw anything up
    nUniqueAlphas = 0;

    for( y = 0; y < Bitmap.GetHeight(); y++ )
    {
        for( x = 0; x < Bitmap.GetWidth(); x++ )
        {
            alpha = Bitmap.GetPixelColor( x, y ).A;

            bFound = FALSE;
            for( c = 0; c < nUniqueAlphas; c++ )
            {
                if( alpha == alphaValues[c] )
                {
                    bFound = TRUE;
                    break;
                }
            }

            if( bFound == FALSE )
            {
                alphaValues[nUniqueAlphas] = alpha;
                nUniqueAlphas++;
                ASSERT( nUniqueAlphas < 256 );
            }
        }
    }

    return nUniqueAlphas;
}

////////////////////////////////////////////////////////////////////////////
// copy a block of 4x4 pixels into internal storage for compression
////////////////////////////////////////////////////////////////////////////
static
void EncodeColorRAW( x_bitmap& Bitmap, s32 x, s32 y )
{
    s32      i;
    s32      c;
    color    col;
    vector3* pDest;
    vector3  value;
    xbool    bFound;

    pDest = colorBlock;
    for( i = 0; i < 16; i++ )
    {
        col = Bitmap.GetPixelColor( x+(i%4), y+(i/4) );
        *pDest++ = vector3( (f32)col.R, (f32)col.G, (f32)col.B );
    }

    // now count how many unique values there are (may seem like overhead,
    // but speeds up the compression quite a bit!)
    nUniqueColors = 0;

    for( i = 0; i < 16; i++ )
    {
        value = colorBlock[i];

        bFound = FALSE;
        for( c = 0; c < nUniqueColors; c++ )
        {
            if( value == uniqueColors[c] )
            {
                bFound = TRUE;
                break;
            }
        }

        if( bFound == FALSE )
        {
            uniqueColors[nUniqueColors] = value;
            nUniqueColors++;
            ASSERT( nUniqueColors <= 16 );
        }
    }
}

////////////////////////////////////////////////////////////////////////////
// copy a block of 4x4 alphas into internal storage for compression
////////////////////////////////////////////////////////////////////////////
static
void EncodeAlphaRAW( x_bitmap& Bitmap, s32 x, s32 y )
{
    s32     i;
    s32     c;
    u8*     pDest;
    u8      value;
    xbool   bFound;

    pDest = alphaBlock;
    for( i =0 ; i < 16; i++ )
    {
        *pDest++ = Bitmap.GetPixelColor( x+(i%4), y+(i/4) ).A;
    }

    // now count how many unique values there are (may seem like
    // overhead, but speeds up the compression quite a bit!)
    bAlpha0       = FALSE;
    bAlpha255     = FALSE;
    nUniqueAlphas = 0;

    for( i = 0; i < 16; i++ )
    {
        value = alphaBlock[i];

        bFound = FALSE;
        for( c = 0; c < nUniqueAlphas; c++ )
        {
            if( value == uniqueAlphas[c] )
            {
                bFound = TRUE;
                break;
            }
        }

        if( bFound == FALSE )
        {
            if( value == 0 )
                bAlpha0 = TRUE;
            if( value == 255 )
                bAlpha255 = TRUE;

            uniqueAlphas[nUniqueAlphas] = value;
            nUniqueAlphas++;
            ASSERT( nUniqueAlphas <= 16 );
        }
    }
}

////////////////////////////////////////////////////////////////////////////
// Build the 3 colors to choose from
////////////////////////////////////////////////////////////////////////////
static
void Build3Colors( vector3& v1, vector3& v2 )
{
    linearColors[0] = v1;
    linearColors[1] = v2;

    linearColors[2] = (v1 + v2) / 2;

    linearColors[3] = vector3(0,0,0);
}

//==========================================================================

static
void ChannelBuild3Colors( s32 channel, vector3& v1, vector3& v2 )
{
    switch( channel )
    {
        case 0:
            linearColors[0].X = v1.X;
            linearColors[1].X = v2.X;
            linearColors[2].X = (v1.X + v2.X) / 2;
            break;
        case 1:
            linearColors[0].Y = v1.Y;
            linearColors[1].Y = v2.Y;
            linearColors[2].Y = (v1.Y + v2.Y) / 2;
            break;
        case 2:
            linearColors[0].Z = v1.Z;
            linearColors[1].Z = v2.Z;
            linearColors[2].Z = (v1.Z + v2.Z) / 2;
            break;
        default:
            ASSERT( FALSE );
    }
    linearColors[3] = vector3(0,0,0);
}

////////////////////////////////////////////////////////////////////////////
// Build the 4 colors to choose from
////////////////////////////////////////////////////////////////////////////
static
void Build4Colors( vector3& v1, vector3& v2 )
{
    linearColors[0] = v1;
    linearColors[1] = v2;

    linearColors[2] = ((v1 * 2) + v2) / 3;
    linearColors[3] = ((v2 * 2) + v1) / 3;
}

//==========================================================================

static
void ChannelBuild4Colors( s32 channel, vector3& v1, vector3& v2 )
{
    switch( channel )
    {
        case 0:
            linearColors[0].X = v1.X;
            linearColors[1].X = v2.X;
            linearColors[2].X = ((v1.X * 2) + v2.X) / 3;
            linearColors[3].X = ((v2.X * 2) + v1.X) / 3;
            break;
        case 1:
            linearColors[0].Y = v1.Y;
            linearColors[1].Y = v2.Y;
            linearColors[2].Y = ((v1.Y * 2) + v2.Y) / 3;
            linearColors[3].Y = ((v2.Y * 2) + v1.Y) / 3;
            break;
        case 2:
            linearColors[0].Z = v1.Z;
            linearColors[1].Z = v2.Z;
            linearColors[2].Z = ((v1.Z * 2) + v2.Z) / 3;
            linearColors[3].Z = ((v2.Z * 2) + v1.Z) / 3;
            break;
        default:
            ASSERT( FALSE );
    }
}

////////////////////////////////////////////////////////////////////////////
// Build the 4 alphas to choose from
////////////////////////////////////////////////////////////////////////////
static
void Build4Alphas( u8& a1, u8& a2 )
{
    linearAlphas[0] = a1;
    linearAlphas[1] = a2;

    linearAlphas[2] = ((a1*4) + (a2*1)) / 5;
    linearAlphas[3] = ((a1*3) + (a2*2)) / 5;
    linearAlphas[4] = ((a1*2) + (a2*3)) / 5;
    linearAlphas[5] = ((a1*1) + (a2*4)) / 5;

    linearAlphas[6] = 0;
    linearAlphas[7] = 255;
}

////////////////////////////////////////////////////////////////////////////
// Build the 6 alphas to choose from
////////////////////////////////////////////////////////////////////////////
static
void Build6Alphas( u8& a1, u8& a2 )
{
    linearAlphas[0] = a1;
    linearAlphas[1] = a2;

    linearAlphas[2] = ((a1*6) + (a2*1)) / 7;
    linearAlphas[3] = ((a1*5) + (a2*2)) / 7;
    linearAlphas[4] = ((a1*4) + (a2*3)) / 7;
    linearAlphas[5] = ((a1*3) + (a2*4)) / 7;
    linearAlphas[6] = ((a1*2) + (a2*5)) / 7;
    linearAlphas[7] = ((a1*1) + (a2*6)) / 7;
}

////////////////////////////////////////////////////////////////////////////
// find the closest linear color to the one given
////////////////////////////////////////////////////////////////////////////
static
u32 FindClosestColor( vector3& col )
{
    u32     i, closestIndex;
    f32     testMag, closest;

    closest = 1<<30; //col.DiffMag(linearColors[0]);
    closestIndex = 0;

    for( i = 0; i < 4; i++ )
    {
        testMag = (col - linearColors[i]).SquaredLength();

        if( testMag < closest )
        {
            closest = testMag;
            closestIndex = i;
            // if match is exact, may as well stop now
            if( closest == 0 )
                break;
        }
    }

    return closestIndex;
}

////////////////////////////////////////////////////////////////////////////
// find the closest linear alpha to the one given
////////////////////////////////////////////////////////////////////////////
static
u32 FindClosestAlpha( u8& alpha )
{
    u32     i, closest, closestIndex, testMag;

    closest = 1<<30; //col.DiffMag(linearColors[0]);
    closestIndex = 0;

    for( i = 0; i < 8; i++ )
    {
        testMag = (u32)SQR( alpha - linearAlphas[i] );

        if( testMag < closest )
        {
            closest = testMag;
            closestIndex = i;
            // if match is exact, may as well stop now
            if( closest == 0 )
                break;
        }
    }

    return closestIndex;
}

////////////////////////////////////////////////////////////////////////////
// Compute the error Sum(dist^2) of a block when compressed
////////////////////////////////////////////////////////////////////////////
static
f32 CalculateColorError( void )
{
    f32 error;
    u32 index;
    s32 i;

    error = 0;

    for( i = 0; i < 16; i++ )
    {
        index = FindClosestColor( colorBlock[i] );
        error += (colorBlock[i] - linearColors[index]).SquaredLength();
    }

    return error;
}

////////////////////////////////////////////////////////////////////////////
// Compute the error Sum(dist^2) of a block when compressed
////////////////////////////////////////////////////////////////////////////
static
s32 CalculateAlphaError( void )
{
    s32 error;
    u32 index;
    s32 i;

    error = 0;
    for( i = 0; i < 16; i++ )
    {
        index = FindClosestAlpha( alphaBlock[i] );
        error += SQR( alphaBlock[i] - linearAlphas[index] );
    }

    return error;
}

////////////////////////////////////////////////////////////////////////////
// Map a 4x4 pixel block to a 4 color DXT1 block
//
// (in)  Source = unique colors in "Pixels"  (1 - 16 codes)
// (in)  Pixels = 4 x 4 pixels to be remapped
// (out) linearColors[4] = 4 output colors that best approximate the input
////////////////////////////////////////////////////////////////////////////
static
f32 CreateBest4ColorMatch( color_block* pCBlock )
{
    f32     error, bestError;
    s32     start0 = 0;
    s32     start1 = 0;
    s32     end0, end1;
    u32     bestIndex[2];
    vector3 best[2], test[2];
    s32     i, j, c;


    bestError    = 1<<30;
    bestIndex[0] = 0;
    bestIndex[1] = 0;

    // match every color against every color to find best fit for colors
    for( i = 0; i < (nUniqueColors-1); i++ )
    {
        for( j = (i+1); j < nUniqueColors; j++ )
        {
            Build4Colors( uniqueColors[i], uniqueColors[j] );

            error = CalculateColorError();
            if( error < bestError )
            {
                bestError    = error;
                bestIndex[0] = i;
                bestIndex[1] = j;
            }
        }
    }

    best[0] = uniqueColors[ bestIndex[0] ];
    best[1] = uniqueColors[ bestIndex[1] ];

    if( bestError == 0 )
    {
        Build4Colors( best[0], best[1] );
    }
    else
    {
        for( c = 0; c < 3; c++ )
        {
            test[0] = best[0];
            test[1] = best[1];
            Build4Colors( test[0], test[1] );   // Build the full set of interpolants

            switch( c )
            {
             case 0: start0 = (u32)test[0].X - ScanRange/2; break;
             case 1: start0 = (u32)test[0].Y - ScanRange/2; break;
             case 2: start0 = (u32)test[0].Z - ScanRange/2; break;
            }
            end0 = start0 + ScanRange;

            start0 = MAX( 0, start0 );
            end0   = MIN( 255, end0 );

            switch( c )
            {
             case 0: start1 = (u32)test[1].X - ScanRange/2; break;
             case 1: start1 = (u32)test[1].Y - ScanRange/2; break;
             case 2: start1 = (u32)test[1].Z - ScanRange/2; break;
            }
            end1 = start1 + ScanRange;

            start1 = MAX( 0, start1 );
            end1   = MIN( 255, end1 );

            for( i = start0; i <= end0; i += ScanStep )
            {
                switch( c )
                {
                 case 0: test[0].X = (f32)i; break;
                 case 1: test[0].Y = (f32)i; break;
                 case 2: test[0].Z = (f32)i; break;
                }

                for( j = start1; j <= end1; j += ScanStep )
                {
                    switch( c )
                    {
                     case 0: test[1].X = (f32)j; break;
                     case 1: test[1].Y = (f32)j; break;
                     case 2: test[1].Z = (f32)j; break;
                    }

                    ChannelBuild4Colors( c, test[0], test[1] ); // Build the channel interpolants
                    error = CalculateColorError();              // Compute the RMS error for Pixels

                    if( error < bestError )
                    {
                        bestError = error;
                        switch( c )
                        {
                         case 0: best[0].X = (f32)i; best[1].X = (f32)j; break;
                         case 1: best[0].Y = (f32)i; best[1].Y = (f32)j; break;
                         case 2: best[0].Z = (f32)i; best[1].Z = (f32)j; break;
                        }

                        if( bestError == 0 )
                            break;

                    }
                }

                if( bestError == 0 )
                    break;
            }
        }

        Build4Colors( best[0], best[1] );
    }


    // store in compressed block as 565 format
    pCBlock->col0 = ((u16)linearColors[0].X>>3 <<11) | ((u16)linearColors[0].Y>>2 <<5) | ((u16)linearColors[0].Z>>3);
    pCBlock->col1 = ((u16)linearColors[1].X>>3 <<11) | ((u16)linearColors[1].Y>>2 <<5) | ((u16)linearColors[1].Z>>3);

    // no punchthrough, col0 must be greater than col1
    if( destFormat == S3TC_DXT1 )
    {
        if( pCBlock->col0 < pCBlock->col1 )
        {
            u16     temp;
            vector3 vTemp;

            temp  = pCBlock->col0;
            vTemp = linearColors[0];
            pCBlock->col0   = pCBlock->col1;
            pCBlock->col1   = temp;
            linearColors[0] = linearColors[1];
            linearColors[1] = vTemp;
        }
    }

    return bestError;
}

////////////////////////////////////////////////////////////////////////////
// Map a 4x4 pixel block to a 3 color + punchthrough DXT1 block
//
// (in)  Source = unique colors in "Pixels"  (1 - 16 codes)
// (in)  Pixels = 4 x 4 pixels to be remapped
// (out) linearColors[4] = 4 output colors that best approximate the input
////////////////////////////////////////////////////////////////////////////
static
f32 CreateBest3ColorMatch( color_block* pCBlock )
{
    f32     error, bestError;
    s32     start0 = 0;
    s32     start1 = 0;
    s32     end0, end1;
    u32     bestIndex[2];
    vector3 best[2], test[2];
    s32     i, j, c;


    bestError    = 1<<30;
    bestIndex[0] = 0;
    bestIndex[1] = 0;

    // match every color against every color to find best fit for colors
    for( i = 0; i < (nUniqueColors-1); i++ )
    {
        for( j = (i+1); j < nUniqueColors; j++ )
        {
            Build3Colors( uniqueColors[i], uniqueColors[j] );

            error = CalculateColorError();
            if( error < bestError )
            {
                bestError    = error;
                bestIndex[0] = i;
                bestIndex[1] = j;
            }
        }
    }

    best[0] = uniqueColors[bestIndex[0]];
    best[1] = uniqueColors[bestIndex[1]];

    if( bestError == 0 )
    {
        Build3Colors( best[0], best[1] );
    }
    else
    {
        for( c = 0; c < 3; c++ )
        {
            test[0] = best[0];
            test[1] = best[1];
            Build3Colors( test[0], test[1] );   // Build the full set of interpolants

            switch( c )
            {
             case 0: start0 = (u32)test[0].X - ScanRange/2; break;
             case 1: start0 = (u32)test[0].Y - ScanRange/2; break;
             case 2: start0 = (u32)test[0].Z - ScanRange/2; break;
            }
            end0 = start0 + ScanRange;

            start0 = MAX( 0, start0 );
            end0   = MIN( 255, end0 );

            switch( c )
            {
             case 0: start1 = (u32)test[1].X - ScanRange/2; break;
             case 1: start1 = (u32)test[1].Y - ScanRange/2; break;
             case 2: start1 = (u32)test[1].Z - ScanRange/2; break;
            }
            end1 = start1 + ScanRange;

            start1 = MAX( 0, start1 );
            end1   = MIN( 255, end1 );

            for( i = start0; i <= end0; i += ScanStep )
            {
                switch( c )
                {
                 case 0: test[0].X = (f32)i; break;
                 case 1: test[0].Y = (f32)i; break;
                 case 2: test[0].Z = (f32)i; break;
                }

                for( j = start1; j <= end1; j += ScanStep )
                {
                    switch( c )
                    {
                     case 0: test[1].X = (f32)j; break;
                     case 1: test[1].Y = (f32)j; break;
                     case 2: test[1].Z = (f32)j; break;
                    }

                    ChannelBuild3Colors( c, test[0], test[1] ); // Build the channel interpolants
                    error = CalculateColorError();              // Compute the RMS error for Pixels

                    if( error < bestError )
                    {
                        bestError = error;
                        switch( c )
                        {
                         case 0: best[0].X = (f32)i; best[1].X = (f32)j; break;
                         case 1: best[0].Y = (f32)i; best[1].Y = (f32)j; break;
                         case 2: best[0].Z = (f32)i; best[1].Z = (f32)j; break;
                        }

                        if( bestError == 0 )
                            break;
                    }
                }

                if( bestError == 0 )
                    break;
            }
        }

        Build3Colors( best[0], best[1] );
    }


    // store in compressed block as 565 format
    pCBlock->col0 = ((u16)linearColors[0].X>>3 <<11) | ((u16)linearColors[0].Y>>2 <<5) | ((u16)linearColors[0].Z>>3);
    pCBlock->col1 = ((u16)linearColors[1].X>>3 <<11) | ((u16)linearColors[1].Y>>2 <<5) | ((u16)linearColors[1].Z>>3);

    // punchthrough, col0 must be less than col1
    if( destFormat == S3TC_DXT1 )
    {
        if( pCBlock->col0 > pCBlock->col1 )
        {
            u16     temp;
            vector3 vTemp;

            temp  = pCBlock->col0;
            vTemp = linearColors[0];
            pCBlock->col0   = pCBlock->col1;
            pCBlock->col1   = temp;
            linearColors[0] = linearColors[1];
            linearColors[1] = vTemp;
        }
    }

    return bestError;
}

////////////////////////////////////////////////////////////////////////////
// Map a 4x4 alpha block to a 6 or 8 alpha block
//
// uniqueAlphas[16] - nUniqueAlphas
// alphaBlock[16] = 4 x 4 pixels to be remapped
// (out) linearAlphas[8] = 6 or 8 output alphas that best approximate the input
////////////////////////////////////////////////////////////////////////////
static
s32 CreateBestAlphaMatch( implicit_alpha_block* pIABlock )
{
    s32     error, bestError;
    u32     bestIndex[2];
    s32     i, j;

    bestError    = 1<<30;
    bestIndex[0] = 0;
    bestIndex[1] = 0;

    // match every color against every color to find best fit for colors
    for( i = 0; i < (nUniqueAlphas-1); i++ )
    {
        for( j = (i+1); j < nUniqueAlphas; j++ )
        {
            if( bAlpha0 && bAlpha255 )
                Build4Alphas( uniqueAlphas[i], uniqueAlphas[j] );
            else
                Build6Alphas( uniqueAlphas[i], uniqueAlphas[j] );

            error = CalculateAlphaError();
            if( error < bestError )
            {
                bestError    = error;
                bestIndex[0] = i;
                bestIndex[1] = j;
            }
        }
    }


    if( bAlpha0 && bAlpha255 )
    {
        Build4Alphas( uniqueAlphas[ bestIndex[0] ], uniqueAlphas[ bestIndex[1] ] );
        if( linearAlphas[0] > linearAlphas[1] )
        {
            u8 temp;
            temp = linearAlphas[0];
            linearAlphas[0] = linearAlphas[1];
            linearAlphas[1] = temp;
        }
    }
    else
    {
        Build6Alphas( uniqueAlphas[ bestIndex[0] ], uniqueAlphas[ bestIndex[1] ] );
        if( linearAlphas[0] < linearAlphas[1] )
        {
            u8 temp;
            temp = linearAlphas[0];
            linearAlphas[0] = linearAlphas[1];
            linearAlphas[1] = temp;
        }
    }

    // store in compressed block as 565 format
    pIABlock->alpha0 = linearAlphas[0];
    pIABlock->alpha1 = linearAlphas[1];

    return bestError;
}

////////////////////////////////////////////////////////////////////////////
static
void CompressColor( color_block* pBlock )
{
    vector3 currentcol;
    u32     index;
    s32     i, j;

    // create the 2bit per color index by finding the closest match to the 4 colors
    for( j = 0; j < 4; j++ )
    {
        pBlock->row[j] = 0;

        for( i = 0; i < 4; i++ )
        {
            currentcol = colorBlock[ (j*4) + i ];

            if( (destFormat == S3TC_DXT1) && (alphaBlock[ (j*4) + i ] == 0) )
                index = 3;
            else
                index = FindClosestColor( currentcol );

            // store index in compressed block as 2bits per pixel index
            index = index << (i*2);
            pBlock->row[j] |= index;
        }
    }
}

////////////////////////////////////////////////////////////////////////////
static
void CompressExplicitAlpha( explicit_alpha_block* pBlock )
{
    u32 currentAlpha;
    u32 index;
    s32 i, j;

    // create the 4bit per pixel alpha
    for( j = 0; j < 4; j++ )
    {
        pBlock->row[j] = 0;

        for( i = 0; i < 4; i++ )
        {
            currentAlpha = (u32)alphaBlock[ (j*4) + i ];

            currentAlpha = currentAlpha * 16 / 256;
            currentAlpha = MAX( 0, MIN(currentAlpha, 15) );

            // store index in compressed block as 3bits per pixel
            index = currentAlpha << (i*4);
            pBlock->row[j] |= index;
        }
    }
}

////////////////////////////////////////////////////////////////////////////
static
void CompressImplicitAlpha( implicit_alpha_block* pBlock )
{
    u8      currentAlpha;
    u32     index;
    u16     index1, index2, index3;
    u32     shift, value;

    pBlock->row[0] = 0;
    pBlock->row[1] = 0;
    pBlock->row[2] = 0;

    // create the 3bit per alpha index by finding the closest match to the 8 alphas
    for( s32 i = 0; i < 16; i++ )
    {
        currentAlpha = alphaBlock[i];

        index = FindClosestAlpha( currentAlpha );

        shift  = i * 3;
        index1 = 0;
        index2 = 0;
        index3 = 0;

        if( shift < 16 )
        {
            value  = index << shift;
            index1 = (u16)value & 0xffff;
            index2 = (u16)value >> 16;
        }
        else
        {
            value  = index << (shift-16);
            index2 = (u16)value & 0xffff;
            index3 = (u16)value >> 16;
        }
        pBlock->row[0] |= index1;
        pBlock->row[1] |= index2;
        pBlock->row[2] |= index3;
    }
}


////////////////////////////////////////////////////////////////////////////
static
void CompressBlockDXT1( color_block* pBlock )
{
    // create interpolants
    if( bAlpha0 )
        CreateBest3ColorMatch( pBlock );
    else
        CreateBest4ColorMatch( pBlock );

    // fill in the 4 rows with 2bits per pixel
    CompressColor( pBlock );
}

////////////////////////////////////////////////////////////////////////////
static
void CompressBlockDXT3( color_block* pColorBlock, explicit_alpha_block* pAlphaBlock )
{
    // create interpolants
    CreateBest4ColorMatch( pColorBlock );

    // fill in the 4 rows with 2bits per pixel
    CompressColor( pColorBlock );

    // fill in the 4 rows with 4bits per pixel alpha
    CompressExplicitAlpha( pAlphaBlock );
}

////////////////////////////////////////////////////////////////////////////
static
void CompressBlockDXT5( color_block* pColorBlock, implicit_alpha_block* pAlphaBlock )
{
    // create interpolants
    CreateBest4ColorMatch( pColorBlock );
    CreateBestAlphaMatch( pAlphaBlock );

    // fill in the 4 rows with 2bits per pixel
    CompressColor( pColorBlock );

    // fill in the 4 rows with 4bits per pixel alpha
    CompressImplicitAlpha( pAlphaBlock );
}

////////////////////////////////////////////////////////////////////////////

xbool AUXBMP_Compress( x_bitmap& Bitmap, S3TC_TYPE CompFormat )
{
    s32                     nXBlocks;
    s32                     nYBlocks;
    s32                     x, y;
    color_block*            pCBlock;
    explicit_alpha_block*   pEABlock;
    implicit_alpha_block*   pIABlock;

    // make sure it's valid for compressed
    ASSERT( Bitmap.GetWidth() %4 == 0 );
    ASSERT( Bitmap.GetHeight() %4 == 0 );
    ASSERT( Bitmap.IsCompressed() == FALSE );

    // clear pointers so we don't free what wasn't used
    pColorBlocks         = NULL;
    pExplicitAlphaBlocks = NULL;
    pImplicitAlphaBlocks = NULL;

    // output something since this could take a while
    //x_printf("compressing...\n");

    // set the bitmap format pointer so we can read pixels
    Bitmap.SetPixelFormat( Bitmap );

    // how many blocks are needed
    nXBlocks = Bitmap.GetWidth() / 4;
    nYBlocks = Bitmap.GetHeight() / 4;


    // allocate space for compressed blocks, needed by all compression types
    pColorBlocks = (color_block*)x_malloc( nXBlocks * nYBlocks * sizeof(color_block) );
    ASSERT( pColorBlocks );

    // choose best format for type
    if( CompFormat == S3TC_AUTO )
    {
        // calculate what the best dest format would be
        destFormat = S3TC_DXT1;

        // see how much alpha we are dealing with
        AlphaCount( Bitmap );

        // check if more than 0 and 1 alpha values
        if( nUniqueAlphas > 2 )
        {
            destFormat = S3TC_DXT3;
            pExplicitAlphaBlocks = (explicit_alpha_block*)x_malloc( nXBlocks * nYBlocks * sizeof(explicit_alpha_block) );
            ASSERT( pExplicitAlphaBlocks );

// why would we choose DXT5?
// should probably only be used if requested
//          destFormat = S3TC_DXT5;
//          pImplicitAlphaBlocks = (implicit_alpha_block*)x_malloc(nXBlocks * nYBlocks * sizeof(implicit_alpha_block));
//          ASSERT(pImplicitAlphaBlocks);
        }
    }
    else
    {
        destFormat = CompFormat;
    }


    // parse bitmap, compressing each block in turn
    for( y = 0; y < nYBlocks; y++ )
    {
        //x_printf( "row[%d/%d]\n", y, nYBlocks );

        for( x = 0; x < nXBlocks; x++ )
        {
            // grab the color and alpha, and calculate the unique values
            EncodeColorRAW( Bitmap, x*4, y*4 );
            EncodeAlphaRAW( Bitmap, x*4, y*4 );

            switch( destFormat )
            {
                case S3TC_DXT1:
                {
                    pCBlock = &pColorBlocks[ (y*nXBlocks) + x ];

                    CompressBlockDXT1( pCBlock );
                }
                break;

                case S3TC_DXT2:
                case S3TC_DXT3:
                {
                    pCBlock  = &pColorBlocks[ (y*nXBlocks) + x ];
                    pEABlock = &pExplicitAlphaBlocks[ (y*nXBlocks) + x ];

                    CompressBlockDXT3( pCBlock, pEABlock );
                }
                break;

                case S3TC_DXT4:
                case S3TC_DXT5:
                {
                    pCBlock  = &pColorBlocks[ (y*nXBlocks) + x ];
                    pIABlock = &pImplicitAlphaBlocks[ (y*nXBlocks) + x ];

                    CompressBlockDXT5( pCBlock, pIABlock );
                }
                break;

                default:
                    ASSERTS( FALSE, "Unknown format requested for AUXBMP_Compress." );
                    break;
            }
        }
    }

    // check the data is smaller than the original size
    s32     size;

    switch( destFormat )
    {
        case S3TC_DXT1:
            size = nXBlocks * nYBlocks * sizeof(color_block);
            break;

        case S3TC_DXT2:
        case S3TC_DXT3:
            size = nXBlocks * nYBlocks * (sizeof(color_block) + sizeof(explicit_alpha_block));
            break;

        case S3TC_DXT4:
        case S3TC_DXT5:
            size = nXBlocks * nYBlocks * (sizeof(color_block) + sizeof(implicit_alpha_block));
            break;

        default:
            size = 0;
            ASSERT( FALSE );
    }

    ASSERT( size <= Bitmap.GetDataSize() );

    // copy the data over the original space (CARL - TEMPORARY)
    // ######## FIX THIS #########
    // ## Need to verify that all the bitmap's members will match up to new format ##
    u8* pData = Bitmap.GetDataPtr();

    for( y = 0; y < nYBlocks; y++ )
    {
        for( x = 0; x < nXBlocks; x++ )
        {
            if( pExplicitAlphaBlocks )
            {
                pEABlock = &pExplicitAlphaBlocks[ (y*nXBlocks) + x ];
                x_memcpy( pData, pEABlock, sizeof(explicit_alpha_block) );
                pData += sizeof(explicit_alpha_block);
            }
            else if( pImplicitAlphaBlocks )
            {
                pIABlock = &pImplicitAlphaBlocks[ (y*nXBlocks) + x ];
                x_memcpy( pData, pIABlock, sizeof(implicit_alpha_block) );
                pData += sizeof(implicit_alpha_block);
            }

            pCBlock = &pColorBlocks[ (y*nXBlocks) + x ];
            x_memcpy( pData, pCBlock, sizeof(color_block) );
            pData += sizeof(color_block);
        }
    }

    // free up the temporary memory
    x_free( pColorBlocks );
    pColorBlocks = NULL;

    if( pExplicitAlphaBlocks )
    {
        x_free( pExplicitAlphaBlocks );
        pExplicitAlphaBlocks = NULL;
    }
    if( pImplicitAlphaBlocks )
    {
        x_free( pImplicitAlphaBlocks );
        pImplicitAlphaBlocks = NULL;
    }

    // Set bitmap flag for compressed format
    Bitmap.SetCompressed();

    // Set bitmap's format to a compressed one
    struct x_comp_bitmap : public x_bitmap
    {
        void SetFormat( format Fmt )    { m_Format = Fmt; }
    };

    switch( destFormat )
    {
     case S3TC_DXT1: ((x_comp_bitmap*)(&Bitmap))->SetFormat( x_bitmap::FMT_DXT1 ); break;
     case S3TC_DXT2: ((x_comp_bitmap*)(&Bitmap))->SetFormat( x_bitmap::FMT_DXT2 ); break;
     case S3TC_DXT3: ((x_comp_bitmap*)(&Bitmap))->SetFormat( x_bitmap::FMT_DXT3 ); break;
     case S3TC_DXT4: ((x_comp_bitmap*)(&Bitmap))->SetFormat( x_bitmap::FMT_DXT4 ); break;
     case S3TC_DXT5: ((x_comp_bitmap*)(&Bitmap))->SetFormat( x_bitmap::FMT_DXT5 ); break;
    }

    //x_printf("done.\n");

    return ERR_SUCCESS;
}


#if 0  // this code isn't used within this file

#pragma pack(1)

struct TGAHEADER
{
    u8      IdLength;       // Image ID Field Length 
    u8      CmapType;       // Color Map Type 
    u8      ImageType;      // Image Type 

    u16     CmapIndex;      // First Entry Index 
    u16     CmapLength;     // Color Map Length 
    u8      CmapEntrySize;  // Color Map Entry Size 

    u16     X_Origin;       // X-origin of Image 
    u16     Y_Origin;       // Y-origin of Image 
    u16     ImageWidth;     // Image Width 
    u16     ImageHeight;    // Image Height 
    u8      PixelDepth;     // Pixel Depth 
    u8      ImagDesc;       // Image Descriptor 
};

#pragma pack()

static void WriteTGA32( x_bitmap& xBMP, const char* pFilename )
{
    char    pFullPath[X_MAX_PATH];
    char    pDrive[X_MAX_DRIVE];
    char    pDir[X_MAX_DIR];
    char    pFName[X_MAX_FNAME];
    char    pFNameMip[X_MAX_FNAME];
    char    pExt[X_MAX_EXT];
    X_FILE* pFile;
    s32     NMips;
    s32     i;
    s32     X, Y, W, H;
    color   PixelColor;
    TGAHEADER TgaHdr;

    x_memset( &TgaHdr, 0, sizeof(TGAHEADER) );

    TgaHdr.ImageType  = 2;      // uncompressed rgb data
    TgaHdr.ImagDesc   = 8;      // image is flipped, or ???
    TgaHdr.PixelDepth = 32;     // 32 bpp, RGBA (actually BGRA)

    x_splitpath( pFilename, pDrive, pDir, pFName, pExt );

    NMips = xBMP.GetNMips();

    // loop through each mip level to write out to a file
    for( i = 0; i <= NMips; i++ )
    {
        W = xBMP.GetWidth( i );
        H = xBMP.GetHeight( i );

        TgaHdr.ImageWidth  = W;
        TgaHdr.ImageHeight = H;

        // make filename, including mip level number
        x_sprintf( pFNameMip, "%s_mip%ld", pFName, i );
        x_makepath( pFullPath, pDrive, pDir, pFNameMip, pExt );

        pFile = x_fopen( pFullPath, "wb" );

        // write tga header to file
        x_fwrite( &TgaHdr, sizeof(TGAHEADER), 1, pFile );

        x_bitmap::SetPixelFormat( xBMP, 0, i );

        // get each pixel color, and write it to file
        // image is stored bottom-up, so last line is stored first
        for( Y = H-1; Y >= 0; Y-- )
        {
            for( X = 0; X < W; X++ )
            {
                PixelColor = x_bitmap::GetPixelColor( X, Y, i );

                // need to reverse the R and B components
                PixelColor.Set( PixelColor.B, PixelColor.G, PixelColor.R, PixelColor.A );

                x_fwrite( &PixelColor.R, sizeof(u8), 4, pFile );
            }
        }

        x_fclose( pFile );
    }
}

static u8 GetIntensityPixel( byte *pIntensityBuff, s32 x, s32 y, s32 width )
{
    if (!pIntensityBuff)
        return 0;

    byte retval = 0;

    byte *pIntense = (pIntensityBuff + (x>>1) + ((y * width)>>1));

    if (x&0x1)  // lower
        retval = ((*pIntense) & 0x0F);      // grab the lower 4-bits
    else        // upper
        retval = (((*pIntense)>>4)&0x0F);    // grab the upper 4-bits

    return retval;
}

#endif // #if 0

static void SetIntensityPixel( byte *pIntensityBuff, s32 x, s32 y, s32 width, u8 val )
{
    if (!pIntensityBuff)
        return;

    byte *pIntense = (pIntensityBuff + (x>>1) + ((y * width)>>1));

    if (x&0x1)
    {
        // lower
        *pIntense &= 0xF0;              // clear the lower 4-bits
        *pIntense |= (val & 0x0F);      // set the lower 4-bits
    }
    else
    {
        // upper
        *pIntense &= 0x0F;              // clear the upper 4-bits
        *pIntense |= ((val<<4)&0xF0);   // set the upper 4-bits
    }
}
static void Build8BitIntensityMap( byte *pDst, x_bitmap &Src, s32 stride, s32 MIPlevel = 0 );

static void Build8BitIntensityMap( byte *pDst, x_bitmap &Src, s32 stride, s32 MIPlevel )
{
    s32 n4BitWidth = Src.GetWidth( MIPlevel );
    s32 n4BitHeight = Src.GetHeight( MIPlevel );

    byte *pSrc = Src.GetDataPtr( MIPlevel );

    s32 i;
    for (i=0; i<n4BitHeight; ++i)
    {
        s32 j;
        for (j=0; j<n4BitWidth; ++j)
        {
            // grab a pixel from source
            byte R = *(pSrc + 0);
            byte G = *(pSrc + 1);
            byte B = *(pSrc + 2);

            // calc the luminance/intensity (actually the Value in HSV space)
            byte Lum = ((R>G)?((R>B)?R:B):((G>B)?G:B));
            pSrc += stride;

            *pDst = Lum;
            pDst ++;
        }
    }
}

static s32 kWidthStep = 2;
static s32 kHeightStep = 2;

static void Build24BitColorMap( byte *pDst, x_bitmap &Src, s32 stride, s32 MIPlevel = 0, xbool bDivide = TRUE );

static void Build24BitColorMap( byte *pDst, x_bitmap &Src, s32 stride, s32 MIPlevel, xbool bDivide )
{
    byte *pDstStep = pDst;
    byte *pSrcStep = Src.GetDataPtr( MIPlevel );
    s32 wstep = 0;
    s32 hstep = 0;
    s32 width = Src.GetWidth( MIPlevel );
    s32 n24BitPixels;

    s32 WidthStep = 1;
    s32 HeightStep = 1;
    if (bDivide)
    {
        WidthStep = kWidthStep;
        HeightStep = kHeightStep;
    }
    n24BitPixels = (Src.GetWidth( MIPlevel ) / WidthStep) * (Src.GetHeight( MIPlevel ) / HeightStep);

    s32 i;
    for (i=0; i<n24BitPixels; ++i)
    {
        byte R[ 4 ];
        byte G[ 4 ];
        byte B[ 4 ];
        s32 j;
        s32 step = 0;
        s32 offset = 0;
        // collect the colors
        for (j=0; j<HeightStep; ++j)
        {
            s32 k;
            byte *pSrc = pSrcStep;
            for (k=0; k<WidthStep; ++k)
            {
                R[ step ] = *(pSrc+2+offset);
                G[ step ] = *(pSrc+1+offset);
                B[ step ] = *(pSrc+0+offset);
                pSrc += stride;
                step ++;
            }
            offset += (width * stride);
        }

        // now average them
        s32 Red = 0;
        s32 Green = 0;
        s32 Blue = 0;
        s32 count = WidthStep * HeightStep;
        for (j=0; j<count; ++j)
        {
            Red += R[ j ];
            Green += G[ j ];
            Blue += B[ j ];
        }
        Red /= count;
        Green /= count;
        Blue /= count;

        // store them in pColStep
        *(pDstStep + 2) = (byte)Red;
        *(pDstStep + 1) = (byte)Green;
        *(pDstStep + 0) = (byte)Blue;

        // increment pDstStep
        pDstStep += 3;

        // increment pSrcStep
        wstep += WidthStep;
        if (wstep >= width)
        {
            wstep = 0;
            hstep += HeightStep;
        }
        pSrcStep = (Src.GetDataPtr( MIPlevel ) + (stride * wstep) + (hstep * width * stride));
    }
}

static void Scale4BitIntensitiesToMax( byte *pDst, x_bitmap &Src, s32 MIPlevel = 0 );

static void Scale4BitIntensitiesToMax( byte *pDst, x_bitmap &Src, s32 MIPlevel )
{

    // loop back over the intensities again, looking at 4x4 blocks of intensities
    //  take the highest intensity value, 
    //  build a scale factor that will make full intensity,
    //  scale the 4x4 block by that scale factor (brightening everything)
    s32 n24BitWidth = Src.GetWidth( MIPlevel ) / kWidthStep;
    s32 n24BitHeight = Src.GetHeight( MIPlevel ) / kHeightStep;
    s32 n4BitWidth = Src.GetWidth( MIPlevel );

    s32 i;
    for (i=0; i<n24BitHeight; ++i)
    {
        s32 j;
        for (j=0; j<n24BitWidth; ++j)
        {
            s32 Intensity[ 16 ];
            s32 MaxIntensity = -1;
            s32 iMaxIntensity = -1;

            s32 iNext = 0;

            s32 k;
            for (k=0; k<kHeightStep; ++k)
            {
                s32 l;
                for (l=0; l<kWidthStep; ++l)
                {
                    Intensity[ iNext ] = *(pDst + ((j * kWidthStep) + l) + (((i * kHeightStep) + k) * n4BitWidth ));
                    iNext ++;
                }
            }

            s32 count = (kWidthStep * kHeightStep);
            for (k=0; k<count; ++k)
            {
                if (Intensity[ k ] > MaxIntensity)
                {
                    MaxIntensity = Intensity[ k ];
                    iMaxIntensity = k;
                }
            }
            if (iMaxIntensity != -1)
            {
                f32 fIntensity = (f32)MaxIntensity / 255.0f;
                f32 fMultiplier = 0.0f;
                if (fIntensity > 0.0f)
                    fMultiplier = 1.0f / fIntensity;

                for (k=0; k<count; ++k)
                {
                    f32 temp = (f32)Intensity[ k ];
                    temp *= fMultiplier;
                    temp += 0.5f;   // round up
                    Intensity[ k ] = (s32)temp;
                }
            }
            else
                ASSERT( iMaxIntensity != -1 );
            // now store the intensities back in our array
            iNext = 0;
            for (k=0; k<kWidthStep; ++k)
            {
                s32 l;
                for (l=0; l<kWidthStep; ++l)
                {
                    *(pDst + ((j * kWidthStep) + l) + (((i * kWidthStep) + k) * n4BitWidth )) = Intensity[ iNext ];
                    iNext ++;
                }
            }
        }
    }
}

static void Build4BitIntensityPalette( byte CLUT4bit[ 16 ], byte *pSrcBuffer, x_bitmap &Src, s32 MIPlevel = 0 );

static void Build4BitIntensityPalette( byte CLUT4bit[ 16 ], byte *pSrcBuffer, x_bitmap &Src, s32 MIPlevel )
{
    byte CLUT8bit[ 256 ];
    x_memset( CLUT8bit, 0, 256 );

    byte *pStep = pSrcBuffer;

    s32 n4BitWidth = Src.GetWidth( MIPlevel );
    s32 n4BitHeight = Src.GetHeight( MIPlevel );

    s32 minIntensity = 256;
    s32 maxIntensity = -1;

    s32 i;
    for (i=0; i<n4BitHeight; ++i)
    {
        s32 j;
        for (j=0; j<n4BitWidth; ++j)
        {
            CLUT8bit[ *pStep ] ++;
            // find min/max
            if (*pStep > maxIntensity)
                maxIntensity = *pStep;
            if (*pStep < minIntensity)
                minIntensity = *pStep;
            pStep ++;
        }
    }

    x_memset( CLUT4bit, 0, 16 );
    f32 step = ((f32)maxIntensity - (f32)minIntensity ) / 16.0f;
    for (i=0; i<16; ++i)
    {
        CLUT4bit[ i ] = (s32)((f32) minIntensity + (step * (f32)i) + 0.5f);
    }
}

static void RemapIntensityToCLUT( byte *pDst, byte CLUT4bit[ 16 ], byte *pSrcBuffer, x_bitmap &Src, s32 MIPlevel = 0 );

static void RemapIntensityToCLUT( byte *pDst, byte CLUT4bit[ 16 ], byte *pSrcBuffer, x_bitmap &Src, s32 MIPlevel )
{
    s32 n4BitHeight = Src.GetHeight( MIPlevel );
    s32 n4BitWidth = Src.GetWidth( MIPlevel );

    byte *pStep = pSrcBuffer;

    s32 i;
    for (i=0; i<n4BitHeight; ++i)
    {
        s32 j;
        for (j=0; j<n4BitWidth; ++j)
        {
            s32 Intensity = *pStep;

            s32 nLeastDiff = 256;
            s32 iLeastDiff = -1;

            s32 k;
            for (k=0; k<16; ++k)
            {
                // pick the "closest" value in CLUT4bit to Intensity
                s32 diff = x_abs( CLUT4bit[ k ] - Intensity );
                if (diff < nLeastDiff)
                {
                    nLeastDiff = diff;
                    iLeastDiff = k;
                }
            }
            ASSERT( iLeastDiff != -1 );

            SetIntensityPixel( pDst, j, i, n4BitWidth, iLeastDiff );
            pStep ++;
        }
    }
}

static s32 Build240ColorCLUT( u32 *pDst, byte *pSrc, s32 nWidth, s32 nHeight, s32 MIPlevel = 0, s32 stride = 3 );

static s32 Build240ColorCLUT( u32 *pDst, byte *pSrc, s32 nWidth, s32 nHeight, s32 MIPlevel, s32 stride )
{
    struct temp_color
    {
        byte R;
        byte G;
        byte B;
        s32 count;
    };

    s32 nTotal = nWidth * nHeight;
    temp_color *pColArray = new temp_color[ nTotal ];
    temp_color *pTempArray = new temp_color[ nTotal ];


    byte *pSrcStep = pSrc;


    s32 nCount = 0;

    s32 i;
    for (i=0; i<nTotal; ++i)
    {
        xbool bAdd = TRUE;
        s32 j;
        for (j=0; j<nCount; ++j)
        {
            if (   pColArray[ j ].R == *(pSrcStep + 2)
                && pColArray[ j ].G == *(pSrcStep + 1)
                && pColArray[ j ].B == *(pSrcStep + 0))
            {
                bAdd = FALSE;
                pColArray[ j ].count ++;
                break;
            }
        }
        if (bAdd)
        {
            pColArray[ nCount ].R = *(pSrcStep + 2);
            pColArray[ nCount ].G = *(pSrcStep + 1);
            pColArray[ nCount ].B = *(pSrcStep + 0);
            pColArray[ nCount ].count ++;
            nCount ++;
        }

        pSrcStep += stride;
    }

    if (nCount <= 240)
    {
        // hooray!  Something EASY happened!  Yeah!
        for (i=0; i<nCount; ++i)
        {
            // go straight from the color array to the palette buffer
            pDst[ i ] = (0xFF << 24) | (pColArray[ i ].B << 16) | (pColArray[ i ].G << 8) | (pColArray[ i ].R);
        }
        for (i=nCount; i<240; ++i)
        {
            // stuff opaque black.
            pDst[ i ] = (0xFF << 24);
        }
    }
    else
    {
        // oh well.  can't win them all.
        // let's look over the pColArray and try merging "close" colors together.

        s32 nVariance = 0;
        while (nCount > 240)
        {
            nVariance ++;
            for (i=0; i<nCount; ++i)
            {
                s32 j;
                for (j=0; j<nCount; ++j)
                {
                    if (j==i)
                        continue;

                    if (   (   pColArray[ i ].R >= pColArray[ j ].R - nVariance
                            && pColArray[ i ].R <= pColArray[ j ].R + nVariance)
                        && (   pColArray[ i ].G >= pColArray[ j ].G - nVariance
                            && pColArray[ i ].G <= pColArray[ j ].G + nVariance)
                        && (   pColArray[ i ].B >= pColArray[ j ].B - nVariance
                            && pColArray[ i ].B <= pColArray[ j ].B + nVariance))
                    {
                        // merge these 2 together
                        // removing [ j ] from pColArray
                        x_memcpy( pTempArray, pColArray, sizeof( temp_color ) * nCount );
                        x_memcpy( &pColArray[ j ], &pTempArray[ j + 1 ], sizeof( temp_color ) * (nCount - (j + 1)) );
                        nCount --;
                    }
                }
            }
        }
        for (i=0; i<nCount; ++i)
        {
            // go straight from the color array to the palette buffer
            pDst[ i ] = (0xFF << 24) | (pColArray[ i ].B << 16) | (pColArray[ i ].G << 8) | (pColArray[ i ].R);
        }
        for (i=nCount; i<240; ++i)
        {
            // stuff opaque black.
            pDst[ i ] = (0xFF << 24);
        }
    }

    delete [] pTempArray;
    pTempArray = NULL;

    delete [] pColArray;
    pColArray = NULL;

    return nCount;
}

static void Build8BitColorMapFrom24BitData( byte *pDst, byte *pSrc, u32 *CLUT, s32 nCLUTElements, s32 offset, s32 Width, s32 Height )
{
    s32 i;
    for (i=0; i<Height; ++i)
    {
        s32 j;
        for (j=0; j<Width; ++j)
        {
            // grab a color from pSrc
            s32 R = *(pSrc + 2);
            s32 G = *(pSrc + 1);
            s32 B = *(pSrc + 0);

            // find the closest match to the RGB in CLUT
            s32 closestIndex = -1;
            s32 RDiff = 256;
            s32 GDiff = 256;
            s32 BDiff = 256;
            s32 TotalDiff = RDiff + GDiff + BDiff;
            s32 loop;
            for (loop=0; loop<240; ++loop)
            {
                // palette is ABGR, not ARGB...
                s32 col = CLUT[ loop ];
                s32 tempB = (col >> 16) & 0xFF;
                s32 tempG = (col >> 8) & 0xFF;
                s32 tempR = (col >> 0) & 0xFF;

                s32 diff1;
                s32 diff2;
                s32 diff3;
                diff1 = x_abs( tempR - R );
                diff2 = x_abs( tempG - G );
                diff3 = x_abs( tempB - B );

                s32 tempTotalDiff = diff1 + diff2 + diff3;

                if (tempTotalDiff < TotalDiff)//diff1 <= RDiff && diff2 <= GDiff && diff3 <= BDiff)
                {
                    closestIndex = loop;
                    TotalDiff = tempTotalDiff;
                    RDiff = diff1;
                    GDiff = diff2;
                    BDiff = diff3;
                }
            }
            if (closestIndex == -1)
                closestIndex = 0;

            // store the result in pDst
            *pDst = (closestIndex + offset);

            // increment the pointers
            pSrc += 3;
            pDst ++;
        }
    }
}


static void Add4BitIntensities( u32 *pDst, byte *pSrc )
{
    s32 i;
    for (i=0; i<16; ++i)
    {
        *pDst = (*pSrc << 24) | (*pSrc << 16) | (*pSrc << 8) | (*pSrc);
        pDst ++;
        pSrc ++;
    }
}


#if defined( TARGET_PS2 ) || defined( TARGET_PC )

static const s32 kMIPAddresses[ 7 ] =
{
    14560,
    15584,
    15928,
    15992,
    16056,
    16120,
    16184,
};
static const s32 kCLUTAddress = 16248;



typedef struct {
	unsigned long NLOOP:15;
	unsigned long EOP:1;
	unsigned long pad16:16;
	unsigned long id:14;
	unsigned long PRE:1;
	unsigned long PRIM:11;
	unsigned long FLG:2;
	unsigned long NREG:4;
	unsigned long REGS0:4;
	unsigned long REGS1:4;
	unsigned long REGS2:4;
	unsigned long REGS3:4;
	unsigned long REGS4:4;
	unsigned long REGS5:4;
	unsigned long REGS6:4;
	unsigned long REGS7:4;
	unsigned long REGS8:4;
	unsigned long REGS9:4;
	unsigned long REGS10:4;
	unsigned long REGS11:4;
	unsigned long REGS12:4;
	unsigned long REGS13:4;
	unsigned long REGS14:4;
	unsigned long REGS15:4;
} sceGifTag;


// macros for sony happy
#define SCE_GS_PSMCT32			(0)
#define SCE_GS_PSMT8			(19)
#define SCE_GS_PSMT4			(20)

#define SCE_GS_SET_BITBLTBUF(sbp, sbw, spsm, dbp, dbw, dpsm) \
	((u64)(sbp)         | ((u64)(sbw) << 16) | \
	((u64)(spsm) << 24) | ((u64)(dbp) << 32) | \
	((u64)(dbw) << 48)  | ((u64)(dpsm) << 56))
#define SCE_GS_SET_TRXPOS(ssax, ssay, dsax, dsay, dir) \
	((u64)(ssax)        | ((u64)(ssay) << 16) | \
	((u64)(dsax) << 32) | ((u64)(dsay) << 48) | \
	((u64)(dir) << 59))
#define SCE_GS_SET_TRXREG(rrw, rrh) \
	((u64)(rrw) | ((u64)(rrh) << 32))
#define SCE_GS_SET_TRXDIR(xdr) ((u64)(xdr))
#define SCE_GS_SET_TEST(ate, atst, aref, afail, date, datm, zte, ztst) \
	((u64)(ate)         | ((u64)(atst) << 1) | \
	((u64)(aref) << 4)  | ((u64)(afail) << 12) | \
	((u64)(date) << 14) | ((u64)(datm) << 15) | \
	((u64)(zte) << 16)  | ((u64)(ztst) << 17))
#define SCE_GS_SET_FRAME(fbp, fbw, psm, fbmask) \
	((u64)(fbp)        | ((u64)(fbw) << 16) | \
	((u64)(psm) << 24) | ((u64)(fbmask) << 32))
#define SCE_GS_SET_TEX1(lcm, mxl, mmag, mmin, mtba, l, k) \
	((u64)(lcm)        | ((u64)(mxl) << 2)  | \
	((u64)(mmag) << 5) | ((u64)(mmin) << 6) | \
	((u64)(mtba) << 9) | ((u64)(l) << 19) | \
	((u64)(k) << 32))
#define SCE_GS_SET_TEX0(tbp, tbw, psm, tw, th, tcc, tfx, \
			cbp, cpsm, csm, csa, cld) \
	((u64)(tbp)         | ((u64)(tbw) << 14) | \
	((u64)(psm) << 20)  | ((u64)(tw) << 26) | \
	((u64)(th) << 30)   | ((u64)(tcc) << 34) | \
	((u64)(tfx) << 35)  | ((u64)(cbp) << 37) | \
	((u64)(cpsm) << 51) | ((u64)(csm) << 55) | \
	((u64)(csa) << 56)  | ((u64)(cld) << 61))
#define SCE_GS_SET_PRIM(prim, iip, tme, fge, abe, aa1, fst, ctxt, fix) \
	((u64)(prim)      | ((u64)(iip) << 3)  | ((u64)(tme) << 4) | \
	((u64)(fge) << 5) | ((u64)(abe) << 6)  | ((u64)(aa1) << 7) | \
	((u64)(fst) << 8) | ((u64)(ctxt) << 9) | ((u64)(fix) << 10))
#define SCE_GS_SET_UV(u, v) ((u64)(u) | ((u64)(v) << 16))
#define SCE_GS_SET_RGBAQ(r, g, b, a, q) \
	((u64)(r)        | ((u64)(g) << 8) | ((u64)(b) << 16) | \
	((u64)(a) << 24) | ((u64)(q) << 32))
#define SCE_GS_SET_XYZ(x, y, z) \
	((u64)(x) | ((u64)(y) << 16) | ((u64)(z) << 32))
#define SCE_GS_SET_ALPHA(a, b, c, d, fix) \
	((u64)(a)       | ((u64)(b) << 2)     | ((u64)(c) << 4) | \
	((u64)(d) << 6) | ((u64)(fix) << 32))
#define SCE_GS_SET_PABE(pabe) ((u64)(pabe))

#define SCE_GS_PRIM		    0x00
#define SCE_GS_RGBAQ		0x01
#define SCE_GS_UV		    0x03
#define SCE_GS_XYZ2		    0x05
#define SCE_GS_TEX0_1		0x06
#define SCE_GS_TEX1_1		0x14
#define SCE_GS_TEXFLUSH		0x3f
#define SCE_GS_ALPHA_1		0x42
#define SCE_GS_TEST_1		0x47
#define SCE_GS_PABE		    0x49
#define SCE_GS_FRAME_1		0x4c
#define SCE_GS_BITBLTBUF	0x50
#define SCE_GS_TRXPOS		0x51
#define SCE_GS_TRXREG		0x52
#define SCE_GS_TRXDIR		0x53


class x_ps2_compressed_bitmap : public x_bitmap
{
public:
    xbool RebuildDataPtr( byte *pMIPtable, byte *pNewTextureData[ 7 ], xbool bCompressed[ 7 ] );
    static void  SetupMipTable ( byte** pRawData, s32 W, s32 H, s32 NMips, xbool bCompressed[ 7 ] );
    void SetClut( s32 nCLUTElements, u32 *CLUT );
};

xbool x_ps2_compressed_bitmap::RebuildDataPtr( byte *pMIPtable, byte *pNewTextureData[ 7 ], xbool bCompressed[ 7 ] )
{
    if (!IsDataOwned())
    {
        ASSERT( IsDataOwned() );
        return FALSE;
    }

    s32 i;
    s32 size[ 7 ];
    s32 MIPTableSize = 0;
    s32 MIPTablePad = 0;
    xbool bUseCompression = FALSE;
    if (GetNMips())
    {
        MIPTableSize = (sizeof( miptable ) * (GetNMips() + 2));
        MIPTablePad = 0;

        if( (((GetNMips() + 2) * sizeof ( miptable )) & 0x1F) != 0 )
            MIPTablePad = 0x20 - (((GetNMips() + 2) * sizeof( miptable )) & 0x1F);

        for (i=0; i<=GetNMips(); ++i)
        {
            s32 W = GetWidth( i );
            s32 H = GetHeight( i );
            s32 intensitymapsize = W * H * 4 / 8;
            s32 colormapsize = (W>>1) * (H>>1) * 8 / 8;
            s32 intensitymapsetupsize = (sizeof( u64 ) * 12) + (sizeof( sceGifTag ) * 2);
            s32 colormapresizesize = (sizeof( u64 ) * 26) + (sizeof( sceGifTag ));
            s32 intensitymapblendsize = (sizeof( u64 ) * 26) + (sizeof( sceGifTag ));
            if (bCompressed[ i ])
            {
                size[ i ] = intensitymapsize + colormapsize + intensitymapsetupsize + colormapresizesize + intensitymapblendsize;
                bUseCompression = TRUE;
            }
            else
                size[ i ] = W * H;
        }
    }
    else
    {
        s32 W = GetWidth();
        s32 H = GetHeight();
        s32 intensitymapsize = W * H * 4 / 8;
        s32 colormapsize = (W>>1) * (H>>1) * 8 / 8;
        s32 intensitymapsetupsize = (sizeof( u64 ) * 12) + (sizeof( sceGifTag ) * 2);
        s32 colormapresizesize = (sizeof( u64 ) * 26) + (sizeof( sceGifTag ));
        s32 intensitymapblendsize = (sizeof( u64 ) * 26) + (sizeof( sceGifTag ));
        if (bCompressed[ 0 ])
            size[ 0 ] = intensitymapsize + colormapsize + intensitymapsetupsize + colormapresizesize + intensitymapblendsize;
        else
            size[ 0 ] = W * H;
    }



    x_free( m_pData );
    m_pData = NULL;

    s32 totalSize = 0;
    if (GetNMips())
    {
        for (i=0; i<=GetNMips(); ++i)
        {
            totalSize += size[ i ];
        }
    }
    else
        totalSize = size[ 0 ];
    totalSize += MIPTableSize + MIPTablePad;
    m_pData = (byte *)x_malloc( totalSize );
    if (!m_pData)
    {
        ASSERT( m_pData );
        return FALSE;
    }
    m_Flags |= FLAG_DATA_OWNED;             // just in case
    m_Flags |= FLAG_DATA_PS2COMPRESSED;

    byte *pStep = m_pData;

    m_DataSize = 0;

    if (GetNMips())
    {
        // build the MIPtable

        // copy the MIPtable
        x_memcpy( pStep, pMIPtable, MIPTableSize );
        pStep += MIPTableSize;
        pStep += MIPTablePad;
        m_DataSize += MIPTableSize;
        m_DataSize += MIPTablePad;

        // now copy the new texture data
        for (i=0; i<=GetNMips(); ++i)
        {
            x_memcpy( pStep, pNewTextureData[ i ], size[ i ] );
            m_DataSize += size[ i ];
            pStep += size[ i ];
        }
    }
    else
    {
        x_memcpy( pStep, pNewTextureData[ 0 ], size[ 0 ] );
        m_DataSize += size[ 0 ];
    }

    return TRUE;
}

void x_ps2_compressed_bitmap::SetupMipTable ( byte** pMIPtable, s32 W, s32 H, s32 NMips, xbool bCompressed[ 7 ] )
{
    if (NMips)
    {
        // need to build the MIPtable
        s32 nMIPTableSize = sizeof( miptable ) * (NMips + 2);
        *pMIPtable = (byte *)x_malloc( nMIPTableSize );

        s32 i;
        s32 CurOffset = (NMips + 2) * sizeof( miptable );

        s32 intensitymapsize = W * H * 4 / 8;
        s32 colormapsize = (W>>1) * (H>>1) * 8 / 8;
        s32 intensitymapsetupsize = (sizeof( sceGifTag ) * 2) + (sizeof( u64 ) * 12);
        s32 colormapresizesize = sizeof( sceGifTag ) + (sizeof( u64 ) * 26);
        s32 intensitymapblendsize = sizeof( sceGifTag ) + (sizeof( u64 ) * 26);

        // verify width and height are powers of 2
        ASSERT( !(W & (W - 1)) );
        ASSERT( !(H & (H - 1)) );

        // Align bitmap data on 32-byte boundary by padding miptable size
        if( (((NMips + 2) * sizeof ( miptable )) & 0x1F) != 0 )
            CurOffset += 0x20 - (((NMips + 2) * sizeof( miptable )) & 0x1F);

        // adjust the size so it matches up to the first offset.
        nMIPTableSize = CurOffset;

        // fill out the mip table
        for( i = 0; i <= NMips; i++ )
        {
            ASSERT( W >= 8 );
            ASSERT( H >= 8 );

            ((miptable *)*pMIPtable)[i].W      = W;
            ((miptable *)*pMIPtable)[i].H      = H;
            ((miptable *)*pMIPtable)[i].Offset = CurOffset;

            if (bCompressed[ i ])
            {
                CurOffset += colormapsize + intensitymapsize + intensitymapsetupsize + colormapresizesize + intensitymapblendsize;
            }
            else
                CurOffset += (W * H);
            colormapsize >>= 2;
            intensitymapsize >>= 2;
            W >>= 1;
            H >>= 1;
        }
        ((miptable *)*pMIPtable)[ NMips + 1 ].W = 0;
        ((miptable *)*pMIPtable)[ NMips + 1 ].H = 0;
        ((miptable *)*pMIPtable)[ NMips + 1 ].Offset = CurOffset;
    }
}

void x_ps2_compressed_bitmap::SetClut( s32 nCLUTElements, u32 *CLUT )
{
    if (m_pClut && IsClutOwned())
    {
        x_free( m_pClut );
    }
    m_pClut = (byte *)x_malloc( sizeof( u32 ) * nCLUTElements );
    x_memcpy( m_pClut, (byte *)CLUT, sizeof( u32 ) * nCLUTElements );
    m_ClutSize = sizeof( u32 ) * nCLUTElements;

    m_Format = FMT_P8_ABGR_8888;
}


#define ERR_FORCE_4BIT (2)
#define ERR_NO_COMPRESS_ALPHA (3)

xbool AUXBMP_CompressForPS2     ( x_bitmap& Bitmap )
{
    // several things need to happen here
    //  First and foremost, the data needs to know it's new format.  Where the hell do we add the data type?
    //  Second, we are creating 2 new maps that get stored in one data buffer
    //  Third, the data that is normally considered to be the image data needs to be hijacked and redirected somehow
    //      probably a dummy header
    //  Fourth, the data needs to be "laced" with GIF commands that tell how to decompress the data upon arrival.
    //      This is assuming I can hijack the setup for the thing
    //      The VRAM manager is sending the texture with a header already.
    //      We can modify VRAM to do what we need, but it would be even better if we didn't have to...

    // Output Format per MIP and some insundry details
    //      Color Texture
    //      GIF command for texture load to memory
    //      Intensity Texture - is an ALPHA multiply texture
    //      GIF command to setup the CLUT as a 4-bit CLUT
    //      GIF command to setup a multiplicative blend
    //      GIF command to setup the Intensity Texture as a render target
    //      GIF command to setup bilinear filtering
    //      GIF command to tell the Color Texture to blend-render the Intensity Texture

    // Using "7" because its our theoretical PS2 MAXMIP count
    byte*   pLuminanceBuffer[ 7 ];
    byte*   p8bitIntensityBuffer[ 7 ];
    byte*   p24BitColorBuffer[ 7 ];
    byte*   p8BitColorBuffer[ 7 ];
    byte*   pOutputBuffer[ 7 ];

    s32 i;
    for (i=0; i<7; ++i)
    {
        pLuminanceBuffer[ i ] = NULL;
        p8bitIntensityBuffer[ i ] = NULL;
        p24BitColorBuffer[ i ] = NULL;
        p8BitColorBuffer[ i ] = NULL;
        pOutputBuffer[ i ] = NULL;
    }

    u32 CLUT[ 256 ];


    // check for early exit conditions
    // already compressed, or not the proper format to be compressed
    // we cannot compress textures with Alpha reliably
    // Really only handles 24-bit textures for now...
    if (Bitmap.IsCompressed())
    {
        // let's not try and compress it again - not that kind of compression!
        return ERR_FAILURE;
    }
    if (Bitmap.GetFormat() != x_bitmap::FMT_24_RGB_888
        && Bitmap.GetFormat() != x_bitmap::FMT_32_ARGB_8888)
    {
        // only compressing 24-bit textures
        return ERR_FAILURE;
    }

    if (Bitmap.GetFormat() == x_bitmap::FMT_32_ARGB_8888)
    {
        s32 w = Bitmap.GetWidth();
        s32 h = Bitmap.GetHeight();
        byte *alpha = Bitmap.GetDataPtr();
        alpha += 3; // point to the alpha
        for (i=0; i<h; ++i)
        {
            s32 j;
            for (j=0; j<w; ++j)
            {
                if (*alpha < 255)
                    return ERR_NO_COMPRESS_ALPHA;       // texture uses alpha.  not compressing.
                alpha += 4;
            }
        }
    }

    xbool bCompressed[ 7 ];

    s32 stride;
    if (Bitmap.GetFormat() == x_bitmap::FMT_24_RGB_888)
        stride = 3;
    else
        stride = 4;

#if 0
    // test - assuming the texture is 2X in Width and Height where we want it, so reducing immediately!
    s32 remapwidth = Bitmap.GetWidth(0)>>1;
    s32 remapheight = Bitmap.GetHeight(0)>>1;
    byte *pTemporaryRemapBuffer = (byte *)x_malloc( sizeof( byte ) * (remapwidth * remapheight * 4) );
    byte *pOut = pTemporaryRemapBuffer;
    byte *pSrc = Bitmap.GetDataPtr();
    byte *pStep = pSrc;
    for (i=0; i<remapheight; ++i)
    {
        s32 j;
        for (j=0; j<remapwidth; ++j)
        {
            s32 k;

            // read 4 colors from Bitmap
            s32 R[4];
            s32 G[4];
            s32 B[4];
            for (k=0; k<4; ++k)
            {
                if (k<=1)
                {
                    R[ k ] = *(pStep + 2 + k);
                    G[ k ] = *(pStep + 1 + k);
                    B[ k ] = *(pStep + 0 + k);
                }
                else
                {
                    R[ k ] = *(pStep + 2 + (stride * remapwidth) + (k - 2));
                    G[ k ] = *(pStep + 1 + (stride * remapwidth) + (k - 2));
                    B[ k ] = *(pStep + 0 + (stride * remapwidth) + (k - 2));
                }
            }

            // average them
            s32 Red = 0;
            s32 Green = 0;
            s32 Blue = 0;
            s32 Alpha = 255;
            for (k=0; k<4; ++k)
            {
                Red += R[ k ];
                Green += G[ k ];
                Blue += B[ k ];
            }
            Red /= 4;
            Green /= 4;
            Blue /= 4;

            // store 1 color in pTemporaryRemapBuffer
            *(pOut + 3) = Alpha;
            *(pOut + 2) = Red;
            *(pOut + 1) = Green;
            *(pOut + 0) = Blue;
            // increment pointers
            pStep += (stride * 2);
            pOut += 4;
        }
        pStep += (stride * remapwidth); // skip an entire row of pixels
    }

    // now that we have a remap map, rebuild Bitmap using this new info...
    Bitmap.SetupBitmap( x_bitmap::FMT_32_ARGB_8888,
                        remapwidth,
                        remapheight,
                        remapwidth,
                        remapheight,
                        TRUE,
                        remapwidth*remapheight*4,
                        pTemporaryRemapBuffer,
                        FALSE,
                        0,
                        NULL,
                        0,
                        Bitmap.GetNMips()-1,
                        TRUE );
#endif



    s32 MIPCount = Bitmap.GetNMips();

    s32 nColors = Build240ColorCLUT( &(CLUT[ 16 ]), Bitmap.GetDataPtr(), Bitmap.GetWidth(), Bitmap.GetHeight(), 0, stride );

    if (nColors <= 16)
    {
        // this texture should be 4-bit!  It's guaranteed to be smaller!
        u32 *tempCLUT;
        tempCLUT = (u32 *)x_malloc( sizeof( u32 ) * 16 );
        for (i=0; i<16; ++i)
        {
            tempCLUT[ i ] = CLUT[ i + 16 ];

//            s32 tempR = (tempCLUT[ i ]>>16)&0xFF;
//            s32 G = (tempCLUT[ i ]>>8)&0xFF;
//            s32 tempB = tempCLUT[ i ]&0xFF;
//            s32 R = tempB;
//            s32 B = tempR;
//            tempCLUT[ i ] = (0xFF<<24)|(R<<16)|(G<<8)|(B);
        }
        s32 size = Bitmap.GetDataSize();
        byte *pTemp = (byte *)x_malloc( size );
        x_memcpy( pTemp, Bitmap.GetDataPtr(), size );
        byte *pTemp2 = (byte *)x_malloc( size >> 1 );
        byte *pSrc = pTemp;
        byte *pDst = pTemp2;
        for (i=0; i<size; ++i)
        {
            s32 R = *(pSrc + 2);
            s32 G = *(pSrc + 1);
            s32 B = *(pSrc + 0);
            s32 j;
            for (j=0; j<16; ++j)
            {
                u32 col = tempCLUT[ j ];
                s32 tempB = (col>>16)&0xFF;
                s32 tempG = (col>>8)&0xFF;
                s32 tempR = col&0xFF;
                if (tempR == R && tempG == G && tempB == B)
                {
                    if (i&0x1)
                    {
                        *pDst |= ((j<<4)&0xF0);
                        pDst ++;
                    }
                    else
                        *pDst = j;
                    break;
                }
            }
            pSrc += stride;
        }
        Bitmap.SetupBitmap( x_bitmap::FMT_P4_ABGR_8888, 
                            Bitmap.GetWidth(), 
                            Bitmap.GetHeight(), 
                            Bitmap.GetPWidth(), 
                            Bitmap.GetPHeight(), 
                            FALSE, 
                            Bitmap.GetDataSize()>>1, 
                            pTemp2, 
                            FALSE, 
                            16*4, 
                            tempCLUT, 
                            16, 
                            Bitmap.GetNMips(), 
                            TRUE );

        return ERR_FORCE_4BIT;
    }

    s32 MIPlevel;
    for (MIPlevel = 0; MIPlevel <= MIPCount; ++MIPlevel)
    {
        s32 ColorSpriteResizeCommandSize = (sizeof( u64 ) * 26) + sizeof( sceGifTag );
        s32 IntensitySetupCommandSize = (sizeof( u64 ) * 12) + (sizeof( sceGifTag ) * 2);
        s32 IntensitySpriteBlendCommandSize = (sizeof( u64 ) * 26) + sizeof( sceGifTag );

        s32 width = Bitmap.GetWidth( MIPlevel );
        s32 height = Bitmap.GetHeight( MIPlevel );
        s32 n8BitUncompressedSize = width * height;
        s32 n24BitColorSize = (width / kWidthStep) * (height / kHeightStep) * 3;
        s32 LumSize = ((width * height) >> 1);
        s32 n8BitIntensitySize = (width * height);

        s32 n8BitColorSize = (width / kWidthStep) * (height / kHeightStep);
        s32 nCompressedSize = n8BitColorSize + 
                              IntensitySetupCommandSize + 
                              LumSize + 
                              ColorSpriteResizeCommandSize + 
                              IntensitySpriteBlendCommandSize;
        if (nCompressedSize > n8BitUncompressedSize || width < 16 || height < 16)
        {
            bCompressed[ MIPlevel ] = FALSE;
            // this needs to get tacked on "as is" as the last MIPlevel
            s32 n24BitColorSize = (Bitmap.GetWidth( MIPlevel )) * (Bitmap.GetHeight( MIPlevel )) * 3;
            p24BitColorBuffer[ MIPlevel ] = (byte *)x_malloc( n24BitColorSize );
            s32 n8BitColorSize = (Bitmap.GetWidth( MIPlevel )) * (Bitmap.GetHeight( MIPlevel ));
            p8BitColorBuffer[ MIPlevel ] = (byte *)x_malloc( n8BitColorSize );
            Build24BitColorMap( p24BitColorBuffer[ MIPlevel ], Bitmap, stride, MIPlevel, FALSE );
            s32 n24BitWidth = Bitmap.GetWidth( MIPlevel );
            s32 n24BitHeight = Bitmap.GetHeight( MIPlevel );
            Build8BitColorMapFrom24BitData( p8BitColorBuffer[ MIPlevel ], p24BitColorBuffer[ MIPlevel ], &CLUT[ 16 ], 240, 16, n24BitWidth, n24BitHeight ); 
            s32 OutputSize = n8BitColorSize;
            pOutputBuffer[ MIPlevel ] = (byte *)x_malloc( OutputSize );

            byte *pOutputPtr = pOutputBuffer[ MIPlevel ];

            x_memcpy( pOutputPtr, p8BitColorBuffer[ MIPlevel ], n8BitColorSize );
            pOutputPtr += n8BitColorSize;
            continue;
        }

        bCompressed[ MIPlevel ] = TRUE;
        // build some work buffers - we still need the original throughout this process as well.
        // 4-bit luminance buffer
        pLuminanceBuffer[ MIPlevel ] = (byte *)x_malloc( LumSize );

        // 24-bit color buffer
        p24BitColorBuffer[ MIPlevel ] = (byte *)x_malloc( n24BitColorSize );
        p8BitColorBuffer[ MIPlevel ] = (byte *)x_malloc( n8BitColorSize );

        // 8-bit version of the luminance buffer, for quantization
        p8bitIntensityBuffer[ MIPlevel ] = (byte *)x_malloc( n8BitIntensitySize );


        // Final output buffer
        //  Sizeof( LuminanceBuffer + ColorBuffer + GIF commands to do combinations )
        //  each GIF command is 128-bits, or 16-bytes

        s32 GIFCommandSize = ColorSpriteResizeCommandSize + 
                             IntensitySetupCommandSize + 
                             IntensitySpriteBlendCommandSize;

        s32 OutputSize = LumSize + n8BitColorSize + GIFCommandSize;
        pOutputBuffer[ MIPlevel ] = (byte *)x_malloc( OutputSize );

        byte *pOutputPtr = pOutputBuffer[ MIPlevel ];

        // build the Luminance map based on the highest value of the RGBs in the original
        //  If Red was highest, use Red, etc.
        // first, let's build an 8-bit intensity map as a basis
        x_MemSanityCheck();
        Build8BitIntensityMap( p8bitIntensityBuffer[ MIPlevel ], Bitmap, stride, MIPlevel );
        x_MemSanityCheck();

        // color is an average of 16 pixels
        x_MemSanityCheck();
        Build24BitColorMap( p24BitColorBuffer[ MIPlevel ], Bitmap, stride, MIPlevel );
        if (MIPlevel == 0)
            Build240ColorCLUT( &(CLUT[ 16 ]), p24BitColorBuffer[ MIPlevel ], width/kWidthStep, height/kHeightStep, MIPlevel );
        x_MemSanityCheck();

        x_MemSanityCheck();
        Scale4BitIntensitiesToMax( p8bitIntensityBuffer[ MIPlevel ], Bitmap, MIPlevel );
        x_MemSanityCheck();

        // need to quantize the 8-bit intensity map down to a 4-bit CLUT-based intensity map.

        x_MemSanityCheck();
        byte IntensityCLUT4bit[ 16 ];
        Build4BitIntensityPalette( IntensityCLUT4bit, p8bitIntensityBuffer[ MIPlevel ], Bitmap, MIPlevel );
        x_MemSanityCheck();

        // now we need to build the luminance map as a lookup to the 16 entry CLUT.
        x_MemSanityCheck();
        RemapIntensityToCLUT( pLuminanceBuffer[ MIPlevel ], IntensityCLUT4bit, p8bitIntensityBuffer[ MIPlevel ], Bitmap, MIPlevel );
        x_MemSanityCheck();

        // and make sure we stuff the 4-bit intensities into our "actual" palette as gray values with Alpha
        x_MemSanityCheck();
        if (MIPlevel == 0)
            Add4BitIntensities( CLUT, IntensityCLUT4bit );
        x_MemSanityCheck();

        // we now have our 2 maps that need to act as 1!

        // we need to build the 8-Bit version of the color map, though!
        s32 n24BitWidth = Bitmap.GetWidth( MIPlevel ) / kWidthStep;
        s32 n24BitHeight = Bitmap.GetHeight( MIPlevel ) / kHeightStep;
        Build8BitColorMapFrom24BitData( p8BitColorBuffer[ MIPlevel ], 
                                        p24BitColorBuffer[ MIPlevel ], 
                                        &CLUT[ 16 ], 
                                        240, 
                                        16, 
                                        n24BitWidth, 
                                        n24BitHeight ); 


        // copy the Color Map
        x_MemSanityCheck();
        x_memcpy( pOutputPtr, p8BitColorBuffer[ MIPlevel ], n8BitColorSize );
        pOutputPtr += n8BitColorSize;
        x_MemSanityCheck();

        // add a GIF command to setup for the next texture
        //
        // first, calculate some values for the texture
        s32 SwizzledW;
        s32 SwizzledH;
        s32 Format;
        s32 BPP = 8;
        s32 n4BitWidth	= 0;
        s32 n4BitHeight = 0;

        switch (width)
        {
            case 8:                n4BitWidth = 3;                break;
            case 16:               n4BitWidth = 4;                break;
            case 32:               n4BitWidth = 5;                break;
            case 64:               n4BitWidth = 6;                break;
            case 128:              n4BitWidth = 7;                break;
            case 256:              n4BitWidth = 8;                break;
            case 512:              n4BitWidth = 9;                break;
            case 1024:             n4BitWidth = 10;               break;
        };
        switch (height)
        {
            case 8:                n4BitHeight = 3;               break;
            case 16:               n4BitHeight = 4;               break;
            case 32:               n4BitHeight = 5;               break;
            case 64:               n4BitHeight = 6;               break;
            case 128:              n4BitHeight = 7;               break;
            case 256:              n4BitHeight = 8;               break;
            case 512:              n4BitHeight = 9;               break;
            case 1024:             n4BitHeight = 10;              break;
        };
        switch (width / kWidthStep)
        {
            case 8:                n24BitWidth = 3;               break;
            case 16:               n24BitWidth = 4;               break;
            case 32:               n24BitWidth = 5;               break;
            case 64:               n24BitWidth = 6;               break;
            case 128:              n24BitWidth = 7;               break;
            case 256:              n24BitWidth = 8;               break;
            case 512:              n24BitWidth = 9;               break;
            case 1024:             n24BitWidth = 10;              break;
        };
        switch (height / kHeightStep)
        {
            case 8:                n24BitHeight = 3;              break;
            case 16:               n24BitHeight = 4;              break;
            case 32:               n24BitHeight = 5;              break;
            case 64:               n24BitHeight = 6;              break;
            case 128:              n24BitHeight = 7;              break;
            case 256:              n24BitHeight = 8;              break;
            case 512:              n24BitHeight = 9;              break;
            case 1024:             n24BitHeight = 10;             break;
        };


        x_MemSanityCheck();
        if ( Bitmap.IsDataSwizzled() && (width >= 16) )
        {
            ASSERTS( Bitmap.GetBPP() == 8, "No support for swizzled 4- and 16-bit images yet!" );
            SwizzledW = (((width / kWidthStep) * BPP) >> 4);
            SwizzledH = (((height / kHeightStep) * BPP) >> 4);
            Format = SCE_GS_PSMCT32;
        }
        else
        {
            SwizzledW = (width / kWidthStep);
            SwizzledH = (height / kHeightStep);
            Format = SCE_GS_PSMT4;
        }
        x_MemSanityCheck();

        s32 sbw = MAX(1, (SwizzledW >> 6) );
        s32 BlockAddr = kMIPAddresses[ MIPlevel ];
        s32 ImageSize = (width * height * BPP);

// building a DMA tag directly into the data so the DMA controller can take things in chunks
//pDList += sizeof( sceDmaTag );
//DMAHELP_BuildTagCont( pHeaderMIPs[ i ], sizeof( TextureOrCLUTPacket ) );

        // Build the GIFtag for the Commands
        ((u64*)pOutputPtr)[0] = 0;
        ((u64*)pOutputPtr)[1] = 0;
        ((sceGifTag *)pOutputPtr)->FLG   = 0;       //packed
        ((sceGifTag *)pOutputPtr)->NREG  = 1;
        ((sceGifTag *)pOutputPtr)->NLOOP = 4;
        ((sceGifTag *)pOutputPtr)->PRE   = 0;
        ((sceGifTag *)pOutputPtr)->PRIM  = 0;
        ((sceGifTag *)pOutputPtr)->EOP   = 1;
        ((sceGifTag *)pOutputPtr)->REGS0 = 0xE;     //A+D
        ((sceGifTag *)pOutputPtr)->REGS1 = 0;
        ((sceGifTag *)pOutputPtr)->REGS2 = 0;
        ((sceGifTag *)pOutputPtr)->REGS3 = 0;
        pOutputPtr += sizeof( sceGifTag );
        x_MemSanityCheck();

        // A+D Command data
        *((u64 *)pOutputPtr) = SCE_GS_SET_BITBLTBUF( BlockAddr, sbw, Format, BlockAddr, sbw, Format);
        pOutputPtr += (byte)sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_BITBLTBUF;                pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_SET_TRXPOS( 0,0,0,0,0 );  pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_TRXPOS;                   pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_SET_TRXREG( SwizzledW, SwizzledH );
        pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_TRXREG;                   pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_SET_TRXREG( SwizzledW, SwizzledH );
        pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_TRXREG;                   pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_SET_TRXDIR( 0 );          pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_TRXDIR;                   pOutputPtr += sizeof( u64 );

        // GIF tag for Texture Image
        ((u64*)pOutputPtr)[0] = 0;
        ((u64*)pOutputPtr)[1] = 0;
        ((sceGifTag *)pOutputPtr)->FLG   = 2;           //IMAGE
        ((sceGifTag *)pOutputPtr)->NREG  = 0;
        ((sceGifTag *)pOutputPtr)->NLOOP = ImageSize;
        ((sceGifTag *)pOutputPtr)->PRE   = 0;
        ((sceGifTag *)pOutputPtr)->PRIM  = 0;    
        ((sceGifTag *)pOutputPtr)->EOP   = 1;
        ((sceGifTag *)pOutputPtr)->REGS0 = 0xE;         // A+D
        ((sceGifTag *)pOutputPtr)->REGS1 = 0;
        ((sceGifTag *)pOutputPtr)->REGS2 = 0;
        ((sceGifTag *)pOutputPtr)->REGS3 = 0;
        pOutputPtr += sizeof( sceGifTag );
        x_MemSanityCheck();

        // copy the Intensity Map image
        x_memcpy( pOutputPtr, pLuminanceBuffer[ MIPlevel ], LumSize );
        pOutputPtr += LumSize;
        x_MemSanityCheck();


        // add the SPRITE command to resize the Color Map using bilinear filtering
        // GIF tag
        ((u64*)pOutputPtr)[0] = 0;
        ((u64*)pOutputPtr)[1] = 0;
        ((sceGifTag *)pOutputPtr)->FLG   = 0;           //PACKED
        ((sceGifTag *)pOutputPtr)->NREG  = 1;
        ((sceGifTag *)pOutputPtr)->NLOOP = 14;
        ((sceGifTag *)pOutputPtr)->PRE   = 0;
        ((sceGifTag *)pOutputPtr)->PRIM  = 0;    
        ((sceGifTag *)pOutputPtr)->EOP   = 1;
        ((sceGifTag *)pOutputPtr)->REGS0 = 0xE;         //A+D
        ((sceGifTag *)pOutputPtr)->REGS1 = 0;
        ((sceGifTag *)pOutputPtr)->REGS2 = 0;
        ((sceGifTag *)pOutputPtr)->REGS3 = 0;
        pOutputPtr += sizeof( sceGifTag );
        x_MemSanityCheck();

        // SPRITE setup
        *((u64 *)pOutputPtr) = SCE_GS_SET_TEST( 0,0,0,0,0,0,1,1 );      pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_TEST_1;                           pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_SET_ALPHA( 0, 2, 0, 2, 0 );       pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_ALPHA_1;                          pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_SET_PABE( 0 );                    pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_PABE;                             pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_SET_FRAME( BlockAddr, width, SCE_GS_PSMCT32, 0 );
        pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_FRAME_1;                          pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_SET_TEX1( 1,1,1,1,1,1,1 );        pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_TEX1_1;                           pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_SET_TEX0( BlockAddr, width/kWidthStep, SCE_GS_PSMT8, n24BitWidth, n24BitHeight, 1, 0, kCLUTAddress, SCE_GS_PSMCT32, 0, 0, 0 );
        pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_TEX0_1;                           pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = 0;                                       pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_TEXFLUSH;                         pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_SET_PRIM( 6,1,1,0,0,0,1,0,0 );    pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_PRIM;                             pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_SET_UV( 0,0 );                    pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_UV;                               pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_SET_RGBAQ( 127, 127, 127, 127, 0x3f800000 );
        pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_RGBAQ;                            pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_SET_XYZ( (2048 - ((width / kWidthStep) >> 1)) << 4, (2048 - ((height / kHeightStep) >> 1)) << 4, 1 );
        pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_XYZ2;                             pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_SET_UV( (width/kWidthStep)<<4, (height/kHeightStep)<<4 );
        pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_UV;                               pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_SET_XYZ( (2048 + ((width / kWidthStep) >> 1))<<4, (2048 + ((height / kHeightStep)>>1)) << 4, 1 );
        pOutputPtr += sizeof( u64 );
        *((u64 *)pOutputPtr) = SCE_GS_XYZ2;                             pOutputPtr += sizeof( u64 );

        // add the SPRITE command to draw the Luminance Map onto the Color map, again using bilinear filtering
        // GIF tag
        x_MemSanityCheck();
        ((u64*)pOutputPtr)[0] = 0;
        ((u64*)pOutputPtr)[1] = 0;
        x_MemSanityCheck();
        ((sceGifTag *)pOutputPtr)->FLG   = 0;           //PACKED
        ((sceGifTag *)pOutputPtr)->NREG  = 1;
        ((sceGifTag *)pOutputPtr)->NLOOP = 14;
        ((sceGifTag *)pOutputPtr)->PRE   = 0;
        ((sceGifTag *)pOutputPtr)->PRIM  = 0;    
        ((sceGifTag *)pOutputPtr)->EOP   = 1;
        ((sceGifTag *)pOutputPtr)->REGS0 = 0xE;         //A+D
        ((sceGifTag *)pOutputPtr)->REGS1 = 0;
        ((sceGifTag *)pOutputPtr)->REGS2 = 0;
        ((sceGifTag *)pOutputPtr)->REGS3 = 0;
        pOutputPtr += sizeof( sceGifTag );
        x_MemSanityCheck();
        // SPRITE setup
        *((u64 *)pOutputPtr) = SCE_GS_SET_TEST( 0,0,0,0,0,0,1,1 );      pOutputPtr += sizeof( u64 );
        x_MemSanityCheck();
        *((u64 *)pOutputPtr) = SCE_GS_TEST_1;                           pOutputPtr += sizeof( u64 );
        x_MemSanityCheck();
        *((u64 *)pOutputPtr) = SCE_GS_SET_ALPHA( 1,2,1,2,0 );           pOutputPtr += sizeof( u64 );
        x_MemSanityCheck();
        *((u64 *)pOutputPtr) = SCE_GS_ALPHA_1;                          pOutputPtr += sizeof( u64 );
        x_MemSanityCheck();
        *((u64 *)pOutputPtr) = SCE_GS_SET_PABE( 1 );                    pOutputPtr += sizeof( u64 );
        x_MemSanityCheck();
        *((u64 *)pOutputPtr) = SCE_GS_PABE;                             pOutputPtr += sizeof( u64 );
        x_MemSanityCheck();
        *((u64 *)pOutputPtr) = SCE_GS_SET_FRAME( BlockAddr, width, SCE_GS_PSMCT32, 0 );
        x_MemSanityCheck();
        *((u64 *)pOutputPtr) = SCE_GS_FRAME_1;                          pOutputPtr += sizeof( u64 );
        x_MemSanityCheck();
        *((u64 *)pOutputPtr) = SCE_GS_SET_TEX1( 1,1,1,1,1,1,1 );        pOutputPtr += sizeof( u64 );
        x_MemSanityCheck();
        *((u64 *)pOutputPtr) = SCE_GS_TEX1_1;                           pOutputPtr += sizeof( u64 );
        x_MemSanityCheck();
        *((u64 *)pOutputPtr) = SCE_GS_SET_TEX0( BlockAddr, width, SCE_GS_PSMT8, n4BitWidth, n4BitHeight, 1, 0, kCLUTAddress, SCE_GS_PSMCT32, 0, 0, 0 );
        pOutputPtr += sizeof( u64 );
        x_MemSanityCheck();
        *((u64 *)pOutputPtr) = SCE_GS_TEX0_1;                           pOutputPtr += sizeof( u64 );
        x_MemSanityCheck();
        *((u64 *)pOutputPtr) = 0;                                       pOutputPtr += sizeof( u64 );
        x_MemSanityCheck();
        *((u64 *)pOutputPtr) = SCE_GS_TEXFLUSH;                         pOutputPtr += sizeof( u64 );
        x_MemSanityCheck();
        *((u64 *)pOutputPtr) = SCE_GS_SET_PRIM( 6,1,1,0,0,0,1,0,0 );    pOutputPtr += sizeof( u64 );
        x_MemSanityCheck();
        *((u64 *)pOutputPtr) = SCE_GS_PRIM;                             pOutputPtr += sizeof( u64 );
        x_MemSanityCheck();
        *((u64 *)pOutputPtr) = SCE_GS_SET_UV( 0,0 );                    pOutputPtr += sizeof( u64 );
        x_MemSanityCheck();
        *((u64 *)pOutputPtr) = SCE_GS_UV;                               pOutputPtr += sizeof( u64 );
        x_MemSanityCheck();
        *((u64 *)pOutputPtr) = SCE_GS_SET_RGBAQ( 127, 127, 127, 127, 0x3f800000 );
        pOutputPtr += sizeof( u64 );
        x_MemSanityCheck();
        *((u64 *)pOutputPtr) = SCE_GS_RGBAQ;                            pOutputPtr += sizeof( u64 );
        x_MemSanityCheck();
        *((u64 *)pOutputPtr) = SCE_GS_SET_XYZ( (2048 - (width >> 1)) << 4, (2048 - (height >> 1)) << 4, 1 );
        pOutputPtr += sizeof( u64 );
        x_MemSanityCheck();
        *((u64 *)pOutputPtr) = SCE_GS_XYZ2;                             pOutputPtr += sizeof( u64 );
        x_MemSanityCheck();
        *((u64 *)pOutputPtr) = SCE_GS_SET_UV( (width)<<4, (height) << 4 );
        pOutputPtr += sizeof( u64 );
        x_MemSanityCheck();
        *((u64 *)pOutputPtr) = SCE_GS_UV;                               pOutputPtr += sizeof( u64 );
        x_MemSanityCheck();
        *((u64 *)pOutputPtr) = SCE_GS_SET_XYZ( (2048 + (width >> 1)) << 4, (2048 + (height >> 1)) << 4, 1 );
        pOutputPtr += sizeof( u64 );
        x_MemSanityCheck();
        *((u64 *)pOutputPtr) = SCE_GS_XYZ2;                             pOutputPtr += sizeof( u64 );
        x_MemSanityCheck();
    }

    // now that we have all of the MIPs for this bitmap, try to string it all back together... \\
//*************************************************************************************************\\
//  Each MIP map is now a new size.  Each MIP calculates its data size based on (Width*Height*BPP)/8
//      The data size is now much bigger than that.  
//      IntensityMap    ColorMap                 SetupIntensity   ResizeColor      BlendIntensity
//      ((W*H*BPP)>>3))+(((W>>1)*(H>>1)*BPP)>>3)+(sizeof(u64)*12)+(sizeof(u64)*26)+(sizeof(u64)*26)

    miptable *pMIPtable = NULL;
    x_ps2_compressed_bitmap::SetupMipTable( (byte **)&pMIPtable, Bitmap.GetWidth(), Bitmap.GetHeight(), Bitmap.GetNMips(), bCompressed );

    // rebuild the Bitmap data ptr
    // Some notes:
    //  The Width, Height, PWidth, and PHeight of the texture are based on the 
    //      final size of the texture rather than the size of the color map.
    //  VRAM_Register will need to handle making sure a compressed texture 
    //      activates with width/2 and height/2...
    x_ps2_compressed_bitmap *pOverload = (x_ps2_compressed_bitmap *)&Bitmap;
    pOverload->RebuildDataPtr( (byte *)pMIPtable, pOutputBuffer, bCompressed );

    pOverload->SetClut( 256, &CLUT[ 0 ] );

    // free up the temporary memory
    if (pMIPtable)
    {
        x_free( pMIPtable );
        pMIPtable = NULL;
    }
    for (i=0; i<7; ++i)
    {
        if (pOutputBuffer[ i ])
            x_free( pOutputBuffer[ i ] );
        pOutputBuffer[ i ] = NULL;
        if (pLuminanceBuffer[ i ])
            x_free( pLuminanceBuffer[ i ] );
        pLuminanceBuffer[ i ] = NULL;
    }

    return ERR_SUCCESS;
}
#endif

///////////////////////////////////////////////////////////////////////////
