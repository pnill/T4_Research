///////////////////////////////////////////////////////////////////////////////
//
// BMP LOADER
//
// Only the 8 and 4 bits are tested.
///////////////////////////////////////////////////////////////////////////////
#include "x_types.hpp"
#include "x_debug.hpp"
#include "x_stdio.hpp"
#include "x_plus.hpp"
#include "x_debug.hpp"
#include "x_memory.hpp"

#include "bmpld.hpp"

///////////////////////////////////////////////////////////////////////////////
// DEFINES 
///////////////////////////////////////////////////////////////////////////////
#define C_WIN   1       // Image class
#define C_OS2   2

#define CT_BI_RGB  0    // Compression type, same as BI_RGB  in <wingdi.h>
#define CT_BI_RLE8 1    // Compression type, same as BI_RLE8 in <wingdi.h>
#define CT_BI_RLE4 2    // Compression type, same as BI_RLE4 in <wingdi.h>

#define BMP_FILEHEADER_LEN 14

#define WIN_INFOHEADER_LEN 40
#define OS2_INFOHEADER_LEN 12

#define GULONG4(bp) ((u32)(bp)[0] + 256 * (u32)(bp)[1] + \
                     65536 * (u32)(bp)[2] + 16777216 * (u32)(bp)[3])

#define GULONG2(bp) ((u32)(bp)[0] + 256 * (u32)(bp)[1])
#define GUINT2(bp)  ((u32)(bp)[0] + 256 * (u32)(bp)[1])


typedef struct
{
    s32     W;
    s32     H;
    s32     BPP;
    byte*   Data;
    byte*   Clut;
} bmp;


///////////////////////////////////////////////////////////////////////////////
// TYPES
///////////////////////////////////////////////////////////////////////////////

typedef struct 
{
    char*       Name;                   // stash pointer to name here too
    s32         Class;                  // Windows or OS/2

    s32         FileSize;               // Size of file in bytes
    s32         XHotSpot;               // Not used
    s32         YHotSpot;               // Not used
    s32         OffBits;                // Offset of image bits from start of header

    s32         HeaderSize;             // Size of info header in bytes
    s32         Width;                  // Image width in pixels
    s32         Height;                 // Image height in pixels
    s32         Planes;                 // Planes. Must == 1
    s32         BitCount;               // Bits per pixels. Must be 1, 4, 8 or 24
    s32         Compression;            // Compression type
    s32         SizeImage;              // Size of image in bytes
    s32         XPelsPerMeter;          // X pixels per meter
    s32         YPelsPerMeter;          // Y pixels per meter
    s32         ClrUsed;                // Number of colormap entries (0 == max)
    s32         ClrImportant;           // Number of important colors

} bmp_header;


///////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
// ReadBmpHeader
//=============================================================================
static
s32 ReadBmpHeader( X_FILE* Fp, bmp_header* Header )
{
    u8 Buf[ WIN_INFOHEADER_LEN ];  // largest we'll need

    //-------------------------------------------------------------------------
    // Read the header
    //-------------------------------------------------------------------------
    if ( x_fread( Buf, 1, BMP_FILEHEADER_LEN, Fp ) != BMP_FILEHEADER_LEN )
        return 1;

    //-------------------------------------------------------------------------
    // Check BMP signature
    //-------------------------------------------------------------------------
    if ( Buf[0] != 'B' || Buf[1] != 'M' )
        return 1;

    //-------------------------------------------------------------------------
    // Copy some basic info
    //-------------------------------------------------------------------------
    Header->FileSize = (s32)GULONG4(&Buf[2]);
    Header->XHotSpot = (s32)GUINT2(&Buf[6]);
    Header->YHotSpot = (s32)GUINT2(&Buf[8]);
    Header->OffBits  = (s32)GULONG4(&Buf[10]);

    //-------------------------------------------------------------------------
    // Read enough of the file info to figure the type out
    //-------------------------------------------------------------------------
    if( x_fread( Buf, 1, 4, Fp ) != 4 )
        return 1;

    Header->HeaderSize = (s32)GULONG4( &Buf[0] );

    if (Header->HeaderSize == WIN_INFOHEADER_LEN)
    {
        Header->Class = C_WIN;
    }
    else if (Header->HeaderSize == OS2_INFOHEADER_LEN)
    {
        Header->Class = C_OS2;
    }
    else
    {
        return 1;
    }

    //-------------------------------------------------------------------------
    // Fill the rest of the inforation
    //-------------------------------------------------------------------------
    if ( Header->Class == C_WIN )
    {
        // read in the rest of the info header 
        if ( x_fread( Buf + 4, 1, WIN_INFOHEADER_LEN - 4, Fp ) != WIN_INFOHEADER_LEN - 4 )
        return 1;

        Header->Width         = (s32)GULONG4(&Buf[4]);
        Header->Height        = (s32)GULONG4(&Buf[8]);
        Header->Planes        = (s32)GUINT2(&Buf[12]);
        Header->BitCount      = (s32)GUINT2(&Buf[14]);
        Header->Compression   = (s32)GULONG4(&Buf[16]);
        Header->SizeImage     = (s32)GULONG4(&Buf[20]);
        Header->XPelsPerMeter = (s32)GULONG4(&Buf[24]);
        Header->YPelsPerMeter = (s32)GULONG4(&Buf[28]);
        Header->ClrUsed       = (s32)GULONG4(&Buf[32]);
        Header->ClrImportant  = (s32)GULONG4(&Buf[36]);
    }
    else
    {
        // C_OS2
        // read in the rest of the info header

        if( x_fread( Buf + 4, 1, OS2_INFOHEADER_LEN - 4, Fp ) != OS2_INFOHEADER_LEN - 4)
            return 1;

        Header->Width           = (s32)GULONG2(&Buf[4]);
        Header->Height          = (s32)GULONG2(&Buf[6]);
        Header->Planes          = (s32)GUINT2(&Buf[8]);
        Header->BitCount        = (s32)GUINT2(&Buf[10]);
        Header->Compression     = CT_BI_RGB;
        Header->SizeImage       = 0;
        Header->XPelsPerMeter   = 0;
        Header->YPelsPerMeter   = 0;
        Header->ClrUsed         = 0;
        Header->ClrImportant    = 0;
    }

    //-------------------------------------------------------------------------
    // Check for file corruption 
    //-------------------------------------------------------------------------
    if (   Header->BitCount != 1
        && Header->BitCount != 4
        && Header->BitCount != 8
        && Header->BitCount != 24)
        return 1;


    if ((    Header->Compression != CT_BI_RGB
         &&  Header->Compression != CT_BI_RLE8
         &&  Header->Compression != CT_BI_RLE4)
         || (Header->Compression == CT_BI_RLE8 && Header->BitCount != 8)
         || (Header->Compression == CT_BI_RLE4 && Header->BitCount != 4))
        return 1;


    if ( Header->Planes != 1 )
        return 1;

    //-------------------------------------------------------------------------
    // Fix up a few things 
    //-------------------------------------------------------------------------
    if ( Header->BitCount < 24 )
    {
        if ( Header->ClrUsed == 0 || Header->ClrUsed > (1 << Header->BitCount) )
        Header->ClrUsed = (1 << Header->BitCount);
    }
    else
        Header->ClrUsed = 0;

    return 0;
}

//=============================================================================
// B_BmpLoad
//=============================================================================
static
s32 LoadBMP( x_bitmap& BMP, const char* FileName )
{
    xbool       Data_Bounds;
    xbool       Data_Short;
    X_FILE*     Fp;
    bmp_header  Header;
    s32         Skip;
    byte*       Data;
    byte*       Clut;
    s32         DataSize;

    //-------------------------------------------------------------------------
    // Check Parameters
    //-------------------------------------------------------------------------
    ASSERT( FileName );

    //-------------------------------------------------------------------------
    // Initialzie variables
    //-------------------------------------------------------------------------
    Data = NULL;
    Clut = NULL;
    Data_Bounds = FALSE;
    Data_Short  = FALSE;

    //-------------------------------------------------------------------------
    // open the file
    //-------------------------------------------------------------------------
    Fp = x_fopen(FileName, "rb");

    if ( Fp == NULL )
        goto ERROR_HANDLER;

    //-------------------------------------------------------------------------
    // Read the bmp header
    //-------------------------------------------------------------------------
    if ( ReadBmpHeader( Fp, &Header ) )
        goto ERROR_HANDLER;

    //-------------------------------------------------------------------------
    // Allocate bitmap data. Make sure to pad the rows to a byte.
    //-------------------------------------------------------------------------
    if ( Header.BitCount < 8 )
    {
        DataSize = 0;
        if( Header.BitCount == 4 )  DataSize = (Header.Width & 0x01);
        if( Header.BitCount == 1 )  DataSize = (Header.Width & 0x03);
        DataSize = ((Header.Width + DataSize) * Header.Height * Header.BitCount) / 8;
    }
    else
        DataSize = (Header.Width * Header.Height * Header.BitCount) / 8;

    Data = (byte*)x_malloc( DataSize );
    if ( Data == NULL )
        goto ERROR_HANDLER;

    //-------------------------------------------------------------------------
    // Create the appropriate image and colormap 
    // Must be 1, 4 or 8 bit mapped type
    //-------------------------------------------------------------------------
    if ( Header.BitCount < 16 ) 
    {
        s32   i;
        s32   ColorSize;
        byte  Buf[4*256];
        byte* Color;
        s32   MaxColors = (1 << Header.BitCount);   // maximum number of colors 

        //---------------------------------------------------------------------
        // Allocate the Clut
        //---------------------------------------------------------------------
        Clut = (byte*)x_malloc( (4 * MaxColors) );
        if ( Clut == NULL )
            goto ERROR_HANDLER;

        // Initialize the palette
        x_memset( Clut, 0, 4 * MaxColors );

        //---------------------------------------------------------------------
        // Set the size of the colors in the palette
        //---------------------------------------------------------------------
        if ( Header.Class == C_WIN ) 
            ColorSize = 4;
        else
            ColorSize = 3;

        //---------------------------------------------------------------------
        // Read in all the colors
        //---------------------------------------------------------------------
        Color = Clut;
        x_fread( Buf, 1, ColorSize*Header.ClrUsed, Fp );
        for ( i = 0; i < Header.ClrUsed; i++ )
        {
#ifdef BIG_ENDIAN
            Color[3] = Buf[(i*ColorSize)+0];
            Color[2] = Buf[(i*ColorSize)+1];
            Color[1] = Buf[(i*ColorSize)+2];
            Color[0] = 255;
#else
            Color[0] = Buf[(i*ColorSize)+0];
            Color[1] = Buf[(i*ColorSize)+1];
            Color[2] = Buf[(i*ColorSize)+2];
            Color[3] = 255;
#endif
            Color+=4;
        }
    }

    //-------------------------------------------------------------------------
    // From this point own if we have an error the dafaul will be Data is short
    //-------------------------------------------------------------------------
    Data_Short = TRUE;

    //-------------------------------------------------------------------------
    // Skip to offset specified in file header for image data
    //-------------------------------------------------------------------------
    if ( Header.Class == C_WIN )
        Skip = Header.OffBits - ( BMP_FILEHEADER_LEN + WIN_INFOHEADER_LEN + 4 * Header.ClrUsed );
    else
        Skip = Header.OffBits - ( BMP_FILEHEADER_LEN + OS2_INFOHEADER_LEN + 3 * Header.ClrUsed );

    for ( ; Skip > 0; Skip-- )
    {
        if ( x_fgetc( Fp ) == X_EOF )
            goto ERROR_HANDLER;
    }

    //-------------------------------------------------------------------------
    // Read the pixel data
    //-------------------------------------------------------------------------
    if ( Header.BitCount == 1 )
    {
        byte* DataCursor, Pad[4];
        s32 Illen, PadLen, y;

        Illen      = (Header.Width + 7) / 8;                      // round bits up to byte
        PadLen     = (((Header.Width + 31) / 32) * 4) - Illen;    // extra bytes to word boundary
        DataCursor = Data + (Header.Height -1) * Illen;           // start at bottom

        // BMP files are left bit == ms bit, so read straight in.
        for ( y = Header.Height; y > 0; y--, DataCursor -= Illen )
        {
            if(   x_fread( DataCursor, 1, Illen, Fp ) != Illen
               || x_fread( Pad,       1, PadLen, Fp ) != PadLen )
                goto ERROR_HANDLER;
        }
    }
    else if ( Header.BitCount == 4 )
    {
        byte* DataCursor;
        s32   Illen, x, y;

        Illen = (Header.Width+1)/2;
        DataCursor = Data + (Header.Height -1) * Illen;    // start at bottom 

        if ( Header.Compression == CT_BI_RLE4 )
        {
            // this define puts the correct 4-bit value at a given pixel.  I is read from file, no shifting needed
            #define SET_RLE4_PIXEL( X, Y, I ) (DataCursor[(-(Y)*Illen)+((X)>>1)] = (byte)(((X)&0x1) ? ((DataCursor[(-(Y)*Illen)+((X)>>1)] & 0xF0) | ((u8)(I) & 0x0F)) : ((DataCursor[(-(Y)*Illen)+((X)>>1)] & 0x0F) | ((u8)(I) & 0xF0))) )

            //------------------------------------------------------------
            // 4 bit rle compression
            //------------------------------------------------------------
            s32 d, e;

            x_memset( Data, 0, DataSize );

            for( x = y = 0;;)
            {
                s32 i;

                //----------------------------------------------------------------
                // Read control character
                //----------------------------------------------------------------
                if ((d = x_fgetc( Fp )) == X_EOF)
                    goto ERROR_HANDLER;

                d = (s32)((u8)((s8)d)); // need an unsigned 8-bit value(avoid sign-extend)

                //----------------------------------------------------------------
                // If the is a run to be make then...
                //----------------------------------------------------------------
                if ( d != 0 )
                {

                    //------------------------------------------------------------
                    // Make sure that we still in bonds
                    //------------------------------------------------------------
                    if ( (x + d) > Header.Width || y > Header.Height )
                    {
                        Data_Bounds = TRUE; // don't run off Buffer, ignore this run

                        if ((e = x_fgetc( Fp )) == X_EOF )
                            goto ERROR_HANDLER;

                        continue;
                    }

                    //------------------------------------------------------------
                    // Get the bits that needs to be repeated
                    //------------------------------------------------------------
                    if ((e = x_fgetc( Fp )) == X_EOF )
                            goto ERROR_HANDLER;

                    e = (s32)((u8)((s8)e)); // need an unsigned 8-bit value(avoid sign-extend)

                    //------------------------------------------------------------
                    // Do the current run
                    //------------------------------------------------------------
                    for ( i = d; i > 0; i-- )
                    {
                        SET_RLE4_PIXEL( x, y, e );
                        x++;
                    }

                    continue;
                }

                //------------------------------------------------------------
                // We didn't have a run so read control character
                //------------------------------------------------------------
                if (( d = x_fgetc( Fp )) == X_EOF )
                    goto ERROR_HANDLER;

                d = (s32)((u8)((s8)d)); // need an unsigned 8-bit value(avoid sign-extend)

                //------------------------------------------------------------
                // end of line 
                //------------------------------------------------------------
                if ( d == 0 )
                {
                    x = 0;
                    y++;
                    continue;
                }

                //------------------------------------------------------------
                // end of bitmap
                //------------------------------------------------------------
                if (d == 1)
                    break;

                //------------------------------------------------------------
                // delta 
                //------------------------------------------------------------
                if ( d == 2 )
                {
                    if ((d = x_fgetc( Fp )) == X_EOF || (e = x_fgetc( Fp )) == X_EOF )
                            goto ERROR_HANDLER;

                    x += (s32)((u8)((s8)d));    // need an unsigned 8-bit value(avoid sign-extend)
                    y += (s32)((u8)((s8)e));    // need an unsigned 8-bit value(avoid sign-extend)

                    continue;
                }

                //------------------------------------------------------------
                // else run of literals 
                //------------------------------------------------------------

                //------------------------------------------------------------
                // Make sure that we are going to be in bonds
                //------------------------------------------------------------
                if ( (x + d) > Header.Width  || y > Header.Height )
                {
                    s32 Btr;

                    Data_Bounds = TRUE; // don't run off Buffer, ignore this run

                    Btr = d/2 + (d & 1) + (((d+1) & 2) >> 1);

                    for (; Btr > 0; Btr--)
                    {
                        if ((e = x_fgetc( Fp )) == X_EOF)
                            goto ERROR_HANDLER;
                    }

                    continue;
                }

                //------------------------------------------------------------
                // Do the literals
                //------------------------------------------------------------
                e = 0;
                for ( i = 0; i < d; i++ )
                {
                    if( (i & 0x1) == 0 )
                    {
                        if (( e = x_fgetc( Fp)) == X_EOF )
                            goto ERROR_HANDLER;

                        e = (s32)((u8)((s8)e)); // need an unsigned 8-bit value(avoid sign-extend)
                    }

                    SET_RLE4_PIXEL( x, y, e );
                    x++;
                }

                //------------------------------------------------------------
                // read pad byte 
                //------------------------------------------------------------
                if ((d+1) & 2)
                {
                    if ( x_fgetc( Fp ) == X_EOF)
                            goto ERROR_HANDLER;
                }
            }

        }
        else
        {
            //------------------------------------------------------------
            // No 4 bit rle compression
            //------------------------------------------------------------
            int d, s, p;
            int i, e;

            d = Header.Width  / 2;      // double pixel count
            s = Header.Width  & 1;      // single pixel
            p = (4 - (d + s)) & 0x3;    // byte pad

            // Check for fast special (USUAL) case
            if( (s==0) && (p==0) )
            {
                DataCursor = &Data[0];//(y-1)*Illen]

                x_fread( (byte*)DataCursor, d * Header.Height, 1, Fp );

                DataCursor += d*Header.Height;

                for( y = 0; y < Header.Height/2; y++ )
                {
                    for( i = 0; i < d; i++ )
                    {
                        e = Data[(y*Illen)+i];
                        Data[(y*Illen)+i] = Data[((Header.Height-1-y)*Illen)+i];
                        Data[((Header.Height-1-y)*Illen)+i] = (u8)e;
                    }
                }
            }
            else
            {
                for( y = Header.Height-1; y >= 0; y-- )
                {
                    ASSERT( y < 512 );
                    ASSERT( y >= 0 );

                    DataCursor = &Data[ y * Illen ];

                    //------------------------------------------------------------
                    // Read two pixels per time
                    //------------------------------------------------------------
                    x_fread( (byte*)DataCursor, d, 1, Fp );
                    DataCursor += d;

                    //------------------------------------------------------------
                    // If we have an extra pixel that falls in half a byte
                    //------------------------------------------------------------
                    if( s )
                    {
                        if( (e = x_fgetc(Fp)) == X_EOF )
                            goto ERROR_HANDLER;

                        *DataCursor = (u8)(e >> 4);
                    }

                    // don't need to read the padding at the end of the file at last line
                    if( y == 0 )
                        break;

                    //------------------------------------------------------------
                    // Read off what ever padding bytes are left
                    //------------------------------------------------------------
                    for( i = 0; i < p; i++ )
                    {
                        if( x_fgetc( Fp ) == X_EOF )
                            goto ERROR_HANDLER;
                    }

                }
            }
        }
    }
    else if( Header.BitCount == 8 )
    {
        byte* DataCursor;
        s32   Illen,x,y;

        Illen      = Header.Width;
        DataCursor = Data + ( Header.Height -1) * Illen;    // start at bottom 

        if ( Header.Compression == CT_BI_RLE8 )
        {
            s32 d, e;

            x_memset( Data, 0, (Header.Height * Header.Width * Header.BitCount) / 8 );

            for( x = y = 0 ;; )
            {
                //------------------------------------------------------------
                // Read control character
                //------------------------------------------------------------
                if ((d = x_fgetc( Fp )) == X_EOF )
                            goto ERROR_HANDLER;

                //------------------------------------------------------------
                // run of pixels
                //------------------------------------------------------------
                if ( d != 0 )
                {
                    x += d;

                    //------------------------------------------------------------
                    // Make sure that stays in bounds
                    //------------------------------------------------------------
                    if ( x > Header.Width  || y > Header.Height )
                    {
                        Data_Bounds = TRUE;         // don't run off Buffer 
                        x -= d;                     // ignore this run 

                        if ((e = x_fgetc( Fp )) == X_EOF )
                            goto ERROR_HANDLER;

                        continue;
                    }

                    //------------------------------------------------------------
                    // Read the character to repear
                    //------------------------------------------------------------
                    if ((e = x_fgetc( Fp )) == X_EOF)
                            goto ERROR_HANDLER;

                    //------------------------------------------------------------
                    // Copy the character
                    //------------------------------------------------------------
                    x_memset( DataCursor, (u8)e, d );
                    DataCursor += d;

                    continue;
                }

                //------------------------------------------------------------
                // Is not a run, read next control character.
                //------------------------------------------------------------
                if ( (d = x_fgetc(Fp)) == X_EOF ) 
                            goto ERROR_HANDLER;

                //------------------------------------------------------------
                // end of line 
                //------------------------------------------------------------
                if ( d == 0 )
                {
                    DataCursor -= (x + Illen);
                    x = 0;
                    y++;
                    continue;
                }

                //------------------------------------------------------------
                // end of bitmap
                //------------------------------------------------------------
                if (d == 1)
                    break;

                //------------------------------------------------------------
                // delta
                //------------------------------------------------------------
                if ( d == 2 )
                {
                    if ((d = x_fgetc( Fp )) == X_EOF || (e = x_fgetc( Fp )) == X_EOF)
                            goto ERROR_HANDLER;

                    x           += d;
                    DataCursor  += d;
                    y           += e;
                    DataCursor  -= (e * Illen);
                    continue;
                }

                //------------------------------------------------------------
                // else run of literals 
                //------------------------------------------------------------
                x += d;

                //------------------------------------------------------------
                // make sure that we are going to be in bounds
                //------------------------------------------------------------
                if ( x > Header.Width  || y > Header.Height )
                {
                    int Btr;

                    Data_Bounds = TRUE;         // don't run off Buffer 
                    x          -= d;            // ignore this run 
                    Btr         = d + (d & 1);

                    for (; Btr > 0; Btr-- )
                    {
                        if ((e = x_fgetc( Fp )) == X_EOF)
                            goto ERROR_HANDLER;
                    }

                    continue;
                }

                //------------------------------------------------------------
                // Read literals directly into our Buffer
                //------------------------------------------------------------
                if ( x_fread( DataCursor, 1, d, Fp ) != d )
                            goto ERROR_HANDLER;

                DataCursor += d;

                //------------------------------------------------------------
                // read pad byte
                //------------------------------------------------------------
                if (d & 1) 
                {
                    if ( x_fgetc( Fp ) == X_EOF)
                            goto ERROR_HANDLER;
                }
            }
        }
        else
        {
            //------------------------------------------------------------
            // No 8 bit rle compression 
            //------------------------------------------------------------
            byte Pad[4];
            int  Padlen;

            //------------------------------------------------------------
            // extra bytes to word boundary 
            //------------------------------------------------------------
            Padlen = ((Header.Width + 3) & ~3) - Illen; 

            //------------------------------------------------------------
            // Read Data
            //------------------------------------------------------------

            // Check for fast special (USUAL) case
            if( Padlen == 0 )
            {
                s32 e,i;
                DataCursor = &Data[0];
                x_fread( (byte*)DataCursor, Illen*Header.Height, 1, Fp );
                DataCursor += Illen*Header.Height;
                for( y=0; y<Header.Height/2; y++ )
                {
                    for( i=0; i<Illen; i++ )
                    {
                        e = Data[(y*Illen)+i];
                        Data[(y*Illen)+i] = Data[((Header.Height-1-y)*Illen)+i];
                        Data[((Header.Height-1-y)*Illen)+i] = (u8)e;
                    }
                }
            }
            else
            {
                for (y = Header.Height; y > 0; y--, DataCursor -= Illen)
                {
                    if( x_fread( DataCursor, 1, Illen, Fp) != Illen )
                            goto ERROR_HANDLER;

                    if( x_fread( Pad, 1, Padlen, Fp) != Padlen )
                            goto ERROR_HANDLER;

                }
            }
        }
    }
    else if( Header.BitCount == 24 || Header.BitCount == 32 )
    {
        byte* DataCursor;
        byte* Pad[4];
        s32 Illen, Padlen, y;
        s32 BytesPerPixel;

        BytesPerPixel = (Header.BitCount/8);
        Illen         = Header.Width * BytesPerPixel;
        Padlen        = (((Header.Width * BytesPerPixel) + 3) & ~3) - Illen;         // extra bytes to word boundary 
        DataCursor    = Data + (Header.Height -1) * Illen;         // start at bottom 

        for ( y = Header.Height; y > 0; y--, DataCursor -= Illen )
        {
            if( x_fread( DataCursor, 1, Illen, Fp) != Illen )
                goto ERROR_HANDLER;

            if( x_fread( Pad, 1, Padlen, Fp ) != Padlen )
                goto ERROR_HANDLER;
        }
    }
    else
    {
        Data_Short = FALSE;
        goto ERROR_HANDLER;
    }
    
    //
    // Convert 24bit to 32bit
    //
    if( Header.BitCount == 24 )
    {
        s32 i;
        byte* NewData;
        NewData = (byte*)x_malloc((4*Header.Width*Header.Height));
        for( i=0; i<Header.Width*Header.Height; i++ )
        {
#ifdef BIG_ENDIAN
            NewData[ (i*4)+3 ] = Data[ (i*3)+0 ];
            NewData[ (i*4)+2 ] = Data[ (i*3)+1 ];
            NewData[ (i*4)+1 ] = Data[ (i*3)+2 ];
            NewData[ (i*4)+0 ] = 255;
#else
            NewData[ (i*4)+0 ] = Data[ (i*3)+0 ];
            NewData[ (i*4)+1 ] = Data[ (i*3)+1 ];
            NewData[ (i*4)+2 ] = Data[ (i*3)+2 ];
            NewData[ (i*4)+3 ] = 255;
#endif
        }
        x_free(Data);
        Data = NewData;
        Header.BitCount = 32;
        DataSize = (Header.Width * Header.Height) << 2;
    }

    //------------------------------------------------------------
    // Success!! Close the File
    //------------------------------------------------------------
    x_fclose( Fp );

    //------------------------------------------------------------
    // Build the Bitmap
    //------------------------------------------------------------
    {
        x_bitmap::format    Format = x_bitmap::FMT_NULL;
        s32                 PWidth;

        switch(Header.BitCount )
        {
        case 1: 
            Data_Short = FALSE;
            ASSERTS(FALSE,"x_bitmap doesn't support 1 bit formats");
            goto ERROR_HANDLER; 

        case 2: 
            Data_Short = FALSE;
            ASSERTS(FALSE,"x_bitmap doesn't support 2 bits format");
            goto ERROR_HANDLER; 

        case 4:     Format = x_bitmap::FMT_P4_URGB_8888; break;
        case 8:     Format = x_bitmap::FMT_P8_URGB_8888; break;
        case 16:    Format = x_bitmap::FMT_16_URGB_1555; break;
        case 24:    Format = x_bitmap::FMT_24_RGB_888;   break;
        case 32:    Format = x_bitmap::FMT_32_URGB_8888; break;
        }


        PWidth = (DataSize*8) / Header.Height;
        PWidth = PWidth / Header.BitCount; 


        BMP.SetupBitmap( Format,
                         Header.Width,
                         Header.Height,
                         PWidth,
                         Header.Height,
                         TRUE,
                         DataSize,
                         Data,
                         Clut != NULL,
                         Clut ? 4 * (1<<Header.BitCount) : 0,
                         Clut,
                         Clut ? (1<<Header.BitCount) : 0 );
    }

    return 1;

    //------------------------------------------------------------
    // ERROR_HANDLER
    //------------------------------------------------------------
ERROR_HANDLER:

    //------------------------------------------------------------
    // Clean up
    //------------------------------------------------------------
    if ( Fp )   x_fclose( Fp );
    if ( Data ) x_free( Data );
    if ( Clut ) x_free( Clut );
    return 0;
}


//================================================================

xbool BMP_Load( x_bitmap& BMP, const char* Filename )
{
    if( !LoadBMP( BMP, Filename ) )
    {
        return FALSE;
    }

    return TRUE;
}

//================================================================

