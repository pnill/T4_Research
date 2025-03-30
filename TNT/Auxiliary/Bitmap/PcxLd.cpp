///////////////////////////////////////////////////////////////////////////////
//
//  PCX LOADER
//
///////////////////////////////////////////////////////////////////////////////

#include "x_bitmap.hpp"

#include "x_types.hpp"
#include "x_debug.hpp"
#include "x_stdio.hpp"
#include "x_bitmap.hpp"
#include "x_memory.hpp"
#include "x_plus.hpp"


///////////////////////////////////////////////////////////////////////////////
// TYPES
///////////////////////////////////////////////////////////////////////////////

typedef struct 
{
	s8		Identifier ;
	s8		Version ;
	s8		Encoding ;		// (1 RLE) (0 Nothing)
	s8		BitsPerPixel ;
	s16		XStart ;
	s16		YStart ;
	s16		XEnd ;
	s16		YEnd ;
	s16		HRes ;
	s16		VRes ;
	s8		Palette[48] ;				// 16 color EGA palette
	s8		Reserved1 ;
	s8		Planes ;
	s16		BytesPerLine ;
	s16		PaletteType ;
	s16		HScreenSize ;
	s16		VScreenSize ;
	s8		Reserved2[54] ;
}pcx_header;

typedef struct 
{
	s32     Width;      // image width 
	s32     Height;     // image height 
	u8*     Data;       // image data, format [y][x] 
	u8*     Pal;        // image palette, format [r][g][b] 
    s32     NClutColors;
}pcx_image;

///////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
// ReadBmpHeader
//=============================================================================
pcx_image* LoadPCX ( const char* Filename )
{
	X_FILE*     fp;
	s32         Len;
	u8*         pInBuffer;
	u8*         pIn;
	pcx_header  PCXHeader;
	u8          Byte, cByteCount, cPlane = 0, cMask = 1;
	s32         wPad, wByteCount = 0, wRow = 0;
	u8*         pImageData;
    u8*         pOldImageData;
	s32         j;
	pcx_image*  newImage = NULL;
    s32         offset;
	s32         bEGA;
    s32         wColors;
    s32         CurrPos;


	fp = x_fopen(Filename, "rb") ;
	if (fp == NULL)
		return NULL;

	// Load Entire File
	x_fseek( fp, 0, X_SEEK_END ) ;
	Len = x_ftell( fp ) ;
	x_fseek( fp, 0, X_SEEK_SET ) ;
	pInBuffer = (u8*)x_malloc( Len ) ;
	ASSERT( pInBuffer ) ;
	x_fread( pInBuffer, 1, Len, fp ) ;
	x_fclose( fp ) ;

	// Read Header
    x_memcpy( (u8*)&PCXHeader, pInBuffer, sizeof(pcx_header) ) ;
	if (PCXHeader.Identifier != 0x0a)
	{
		x_fclose(fp) ;
		return NULL;
	}

	newImage = (pcx_image*)x_malloc(sizeof(pcx_image));
    ASSERT(newImage);
	x_memset( newImage, 0, sizeof(pcx_image) );

	newImage->Width= PCXHeader.XEnd - PCXHeader.XStart + 1; 
	newImage->Height = PCXHeader.YEnd - PCXHeader.YStart +1;

	if( PCXHeader.BitsPerPixel == 1 )
		wPad = PCXHeader.BytesPerLine*8 - newImage->Width;
	else
		wPad = PCXHeader.BytesPerLine - newImage->Width;
	
    offset = PCXHeader.Planes;
	bEGA = 0;

	if( (PCXHeader.Planes==4) && (PCXHeader.BitsPerPixel==1) )
	{
		offset = 1;
		bEGA = 1;
	}

	wColors = (1<<PCXHeader.BitsPerPixel) * PCXHeader.Planes;

	// Allocate Data
	newImage->Data = (u8*)x_malloc(sizeof(u8)*(newImage->Width * newImage->Height + wPad));
	ASSERT(newImage->Data);
	x_memset( newImage->Data, 0, newImage->Width*newImage->Height );

	// Allocate Palette
	newImage->Pal = (u8*)x_malloc(sizeof(u8)*256*4);
	ASSERT( newImage->Pal );
	x_memset( newImage->Pal, 0, 256 * 4 );

	// Read Palette
	CurrPos = x_ftell(fp);
	if( PCXHeader.BitsPerPixel == 8 )
	{
		if( pInBuffer[Len-769] == 0x0c )
			x_memcpy( newImage->Pal, &pInBuffer[Len-768], 256*3 );
        newImage->NClutColors = 256;
	}
	else
	{
		x_memcpy( newImage->Pal, &pInBuffer[16], 16*3 );
        newImage->NClutColors = 16;
	}

	// start decoding image data
	pImageData = pOldImageData = &newImage->Data[offset-1];

	// Loop until all done
	pIn = &pInBuffer[128];
	while( 1 )
	{
		Byte = *pIn++ ;
		if ((Byte & 0xc0) == 0xc0)
		{
			cByteCount = Byte &0x3f ;
			Byte = *pIn++ ;
		}
		else
			cByteCount = 1 ;

		while( cByteCount-- )
		{
			if( PCXHeader.BitsPerPixel == 1 )
			{
				if( bEGA )
				{
					for( j = 7 ; j >= 0 ; j-- )
					{
						if ( (pImageData - pOldImageData) < newImage->Width )
						{
							if( Byte & (1<<j) )
								*pImageData++ |= 1<<cPlane ;
							else
								pImageData++ ;
						} 
                        else 
                        {
							pImageData++;
						}
					}
				}
				else
				{
					for( j = 7 ; j >= 0 ; j-- )
					{
						if ( (pImageData - pOldImageData) < newImage->Width )
						{
							*pImageData++ = Byte & ((1<<j) ? (wColors-1) : 0 ) ;
						} 
                        else 
                        {
							pImageData++;
						}
					}
				}
			}
			else
			{
				*pImageData = Byte ;
				pImageData += offset ;
			}
			wByteCount++ ;

			if( wByteCount == PCXHeader.BytesPerLine )
			{
				wByteCount = 0 ;
				cPlane++ ;

				if( cPlane >= PCXHeader.Planes )
				{
					cPlane = 0 ;
					pImageData -= wPad ;
					pImageData += offset-1 ;
					pOldImageData = pImageData ;
					wRow++ ;
					if( wRow >= newImage->Height )
						goto DataRead ;
				}
				else
				{
					if( bEGA )
						pImageData = pOldImageData ;
					else
						pImageData = pOldImageData - cPlane ;
				}
				cMask = 1<<cPlane ;
			}
		}
	}

DataRead:

	x_free( pInBuffer ) ;

	return newImage;
}


xbool PCX_Load( x_bitmap& Bitmap, const char * Filename )
{
    pcx_image*  Image;

	Image = LoadPCX( Filename );

	if (Image)
	{
        // Quick step: add alpha to the palette
        //
        // NOTE: This block removed for now
        //       It's purpose was to convert the data into a PS2 compatible
        //       format by making it P8_RGBA_8888.  This can now be done
        //       as a post process using the bitmap converter function.
        //       This block was removed to leave the pcx data in neutral form.
        //

        /*
        {
            s32     i;
            u8      Pal[256*4];
            for (i=0;i<Image->NClutColors;i++)
            {
                Pal[i*4]    = Image->Pal[i*3];
                Pal[i*4+1]  = Image->Pal[i*3+1];
                Pal[i*4+2]  = Image->Pal[i*3+2];
                Pal[i*4+3]  = 0xff;
            }
            x_memcpy(Image->Pal, Pal, Image->NClutColors*4);
        }
        */

        Bitmap.SetupBitmap( x_bitmap::FMT_P8_RGB_888,    //  Format,
						    Image->Width,                  //  Width,
						    Image->Height,                 //  Height,
						    Image->Width,                  //  PWidth,
						    Image->Height,                 //  PHeight,
						    TRUE,                          //  IsDataOwned
						    Image->Width*Image->Height,    //  DataSizeInBytes
						    Image->Data,                   //  PixelData,
						    TRUE,                          //  IsClutOwned
						    Image->NClutColors*3,          //  ClutSizeInBytes
						    Image->Pal,                    //  ClutData,
						    Image->NClutColors );

        // Free the PCX image itself.
        x_free( Image );
        Image = NULL;
		return TRUE;
	}
	return FALSE;
}
