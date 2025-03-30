#include "x_plus.hpp"
#include "x_stdio.hpp"
#include "x_memory.hpp"
#include "x_debug.hpp"
#include "GifLd.hpp"

///////////////////////////////////////////////////////////////////////////////
// PC_ONLY
///////////////////////////////////////////////////////////////////////////////
//#ifndef TARGET_PC 
//#ifndef TARGET_PS2
//#error "This file compiles ONLY for PC"
//#endif
//#endif

//////////////////////////////////////////////////////////////////////////////
//  Defines

#define LZ_MAX_CODE         4095                    // Biggest code possible in 12 bits. 
#define LZ_BITS             12

#define GIF_STAMP_LEN       sizeof(GIF_STAMP) - 1
#define GIF_STAMP           "GIFVER"                // First chars in file - GIF stamp. 
#define GIF_VERSION_POS     3                       // Version first character in stamp. 

#define NO_SUCH_CODE        4098                    // Impossible code, to signal empty. 

#define RED                 2
#define GREEN               1
#define BLUE                0
#define RESERVED            3


typedef enum
{
    UNDEFINED_RECORD_TYPE,
    SCREEN_DESC_RECORD_TYPE,
    IE_IMAGE_DESC_RECORD_TYPE,                  // Begin with ',' 
    EXTENSION_RECORD_TYPE,                      // Begin with '!' 
    TERMINATE_RECORD_TYPE                       // Begin with ';' 

} gifrecordtype;


//////////////////////////////////////////////////////////////////////////////
//  Types

typedef struct 
{
    signed   short int  scr_len;
    signed   short int  scr_hgt;
    signed   short int  scr_colorres;
    signed   short int  scr_background;
    signed   short int  scr_bitsperpixel;
    int                 scr_coloramt;
    unsigned short int  img_left;
    unsigned short int  img_top;
    unsigned short int  img_len;
    unsigned short int  img_hgt;
    int                 img_size;
    int                 img_coloramt;
    gifrecordtype       type;
} 
gif;


typedef struct
{
    u8 r, g, b;
} 
rgb;


typedef struct
{
    s32     Width,  Height;
    s32     AWidth, AHeight;
    s32     ColorDepth;
    byte*   Data;
    rgb*    Palette;
} 
gifbmp;

//////////////////////////////////////////////////////////////////////////////
//  Variables

static s32          m_BitsPerPixel;             // Bits per pixel (Codes uses at list this + 1). 
static s32          m_ClearCode;                // The CLEAR LZ code. 
static s32          m_EOFCode;                  // The EOF LZ code. 
static s32          m_RunningCode;              // The next code algorithm can generate. 
static s32          m_RunningBits;              // The number of bits required to represent RunningCode. 
static s32          m_MaxCode1;                 // 1 bigger than max. possible code, in RunningBits bits. 
static s32          m_LastCode;                 // The code before the current code. 
static s32          m_StackPtr;                 // For character stack (see below). 
static s32          m_CrntShiftState;           // Number of bits in CrntShiftDWord. 

static u32          m_CrntShiftDWord;           // For bytes decomposition into codes. 
static u32          m_PixelCount;               // Number of pixels in image. 

static X_FILE*      m_Fp;                       // File as stream. 
static u8           m_Buf[256];                 // Compressed input is buffered here. 
static u8           m_Stack[LZ_MAX_CODE];       // Decoded pixels are stacked here. 
static u8           m_Suffix[LZ_MAX_CODE+1];    // So we can trace the codes. 
static unsigned int m_Prefix[LZ_MAX_CODE+1];

static gif          m_GifInfo;                  // decoded gif info
static rgb          m_GIFPalette[256];
static gifbmp       m_DesBmp;                   // Destination structure

//////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
//////////////////////////////////////////////////////////////////////////////


//==================================================================
// ReadWord
//==================================================================
static 
u16 ReadWord( X_FILE* Fp )
{
    u16 t;

    x_fread( &t, sizeof(u16), 1, Fp );

    return t;
}

//==================================================================
// GetScreenDesc
//------------------------------------------------------------------
// This routine should be called before any other calls. Note that
// this routine is called automatically from the file load routine.
//==================================================================
static 
void GetScreenDesc( void )
{
    s32 i, Size;
    u8  Buf;

    // Get width & height from file
    m_GifInfo.scr_len = ReadWord( m_Fp );
    m_GifInfo.scr_hgt = ReadWord( m_Fp );

    Buf = (u8)x_fgetc( m_Fp );
    m_GifInfo.scr_colorres      = (((Buf & 0x70) + 1) >> 4) + 1;
    m_GifInfo.scr_bitsperpixel  = (Buf & 0x07) + 1;
    m_GifInfo.scr_background    = (u8)x_fgetc( m_Fp );
    m_GifInfo.scr_coloramt      = (1 << m_GifInfo.scr_bitsperpixel );

    x_fgetc( m_Fp );

    // Do we have global color map? 
    if (Buf & 0x80)
    {
        Size = (1 << m_GifInfo.scr_bitsperpixel) ;
        for (i = 0; i < Size; i ++)
        {
            m_GIFPalette[i].r = (u8)x_fgetc( m_Fp );
            m_GIFPalette[i].g = (u8)x_fgetc( m_Fp );
            m_GIFPalette[i].b = (u8)x_fgetc( m_Fp );
        }
    }
}

//==================================================================
// GetRecordType
//==================================================================
static 
xbool GetRecordType(void)
{
    u8  Buf;
            
    if ( x_fread( &Buf, 1, 1, m_Fp ) != 1 ) return FALSE;

    switch( Buf )
    {
        case ',':
            m_GifInfo.type = IE_IMAGE_DESC_RECORD_TYPE;
            break;
        case '!':
            m_GifInfo.type = EXTENSION_RECORD_TYPE;

            // Fast forward looking for IMAGE DESC
            while( Buf != ',' )
            {
                if ( x_fread( &Buf, 1, 1, m_Fp ) != 1 ) return FALSE;
                m_GifInfo.type = IE_IMAGE_DESC_RECORD_TYPE;
            }
            break;
        case ';':
            m_GifInfo.type = TERMINATE_RECORD_TYPE;
            break;
        default:
            m_GifInfo.type = UNDEFINED_RECORD_TYPE;
            return FALSE;
    }

    return TRUE;
}
 
//==================================================================
// DGifSetupDecompress
//------------------------------------------------------------------
// Setup the LZ decompression for this image:
//==================================================================
static 
void DGifSetupDecompress( s32 size )
{
    s32         i, BitsPerPixel;
    u8          CodeSize;
    u32*        Prefix;

    m_PixelCount = size;

    x_fread( &CodeSize, 1, 1, m_Fp );               // Read Code size from file. 
    BitsPerPixel = CodeSize;

    m_Buf[0]            = 0;                        // Input Buffer empty. 
    m_BitsPerPixel      = BitsPerPixel;
    m_ClearCode         = (1 << BitsPerPixel);
    m_EOFCode           = m_ClearCode + 1;
    m_RunningCode       = m_EOFCode + 1;
    m_RunningBits       = BitsPerPixel + 1;         // Number of bits per code. 
    m_MaxCode1          = 1 << m_RunningBits;       // Max. code + 1. 
    m_StackPtr          = 0;                        // No pixels on the pixel stack. 
    m_LastCode          = NO_SUCH_CODE;
    m_CrntShiftState    = 0;                        // No information in CrntShiftDWord. 
    m_CrntShiftDWord    = 0;

    Prefix = (u32 *)m_Prefix;
    for (i = 0; i < LZ_MAX_CODE; i++)
        Prefix[i] = NO_SUCH_CODE;
}

//==================================================================
// GetImageDesc
//------------------------------------------------------------------
// This routine should be called before any attemp to read an image.
// Note it is assumed the Image desc. header (',') has been read.
//==================================================================
static 
void GetImageDesc( void )
{
    s32     Size, i;
    s32     BitsPerPixel;
    u8      Buf;

    m_GifInfo.img_left      = ReadWord( m_Fp );
    m_GifInfo.img_top       = ReadWord( m_Fp );
    m_GifInfo.img_len       = ReadWord( m_Fp );
    m_GifInfo.img_hgt       = ReadWord( m_Fp );

    Buf = (u8)x_fgetc( m_Fp );

    BitsPerPixel = ( Buf & 0x07 ) + 1;
    m_GifInfo.img_coloramt  = (1 << BitsPerPixel);

    // Does this image have local color map? 
    if (Buf & 0x80)
    {
        Size = (1 << BitsPerPixel) ;
        for (i = 0; i < Size; i ++)
        {
            m_GIFPalette[i].r = (u8)x_fgetc( m_Fp );
            m_GIFPalette[i].g = (u8)x_fgetc( m_Fp );
            m_GIFPalette[i].b = (u8)x_fgetc( m_Fp );
        }
    }

    // Reset decompress algorithm parameters. 
    DGifSetupDecompress((long) m_GifInfo.img_len * (long) m_GifInfo.img_hgt);  
}

//==================================================================
// DGifGetCodeNext
//------------------------------------------------------------------
// Continue to get the image code in compressed form. This routine should be
// called until NULL block is returned.                       *
// The block should NOT be freed by the user (not dynamically allocated).
//==================================================================
static 
xbool DGifGetCodeNext( u8** CodeBlock )
{
    u8  Buf;

    if ( x_fread( &Buf, 1, 1, m_Fp ) != 1 )
        return FALSE;

    if ( Buf > 0 )
    {
        *CodeBlock = m_Buf;             // Use private unused buffer. 
        (*CodeBlock)[0] = Buf;          // Pascal strings notation (pos. 0 is len.). 

        if ( x_fread( &((*CodeBlock)[1]), 1, Buf, m_Fp ) != Buf )
            return FALSE;
    }
    else
    {
        *CodeBlock      = NULL;
        m_Buf[0]        = 0;          // Make sure the buffer is empty! 
        m_PixelCount    = 0;          // And local info. indicate image read. 
    }

    return TRUE;
}
 
//==================================================================
// DGifDecompressLine
//------------------------------------------------------------------
// This routines read one gif data block at a time and buffers it internally
// so that the decompression routine could access it.
// The routine returns the next byte from its internal buffer (or read next
// block in if buffer empty) and returns GIF_OK if succesful.
//==================================================================
static 
xbool DGifBufferedInput( X_FILE* Fp, u8* Buf, u8* NextByte )
{
    if (Buf[0] == 0)
    {
        // Needs to read the next buffer - this one is empty: 
        if ( x_fread(Buf, 1, 1, Fp) != 1 )
            return FALSE;

        if ( x_fread(&Buf[1], 1, Buf[0], Fp) != Buf[0] )
            return FALSE;

        *NextByte = Buf[1];
        Buf[1] = 2;      // We use now the second place as last char read! 
        Buf[0]--;
    }
    else
    {
        *NextByte = Buf[Buf[1]++];
        Buf[0]--;
    }

    return TRUE;
}

//==================================================================
// DGifDecompressLine
//------------------------------------------------------------------
// Routine to trace the Prefixes linked list until we get a prefix which is
// not code, but a pixel value (less than ClearCode). Returns that pixel value.
// If image is defective, we might loop here forever, so we limit the loops to
// the maximum possible if image O.k. - LZ_MAX_CODE times.
//==================================================================
static 
s32 DGifGetPrefixChar( u32* Prefix, s32 Code, s32 ClearCode )
{
    s32 i = 0;

    while ( Code > ClearCode && i++ <= LZ_MAX_CODE )
        Code = Prefix[Code];

    return Code;
}

//==================================================================
// DGifDecompressInput
//------------------------------------------------------------------
// The LZ decompression input routine:
// This routine is responsable for the decompression of the bit stream from
// 8 bits (bytes) packets, into the real codes.
// Returns GIF_OK if read succesfully.
//==================================================================
static 
xbool DGifDecompressInput( s32* Code )
{
    u8         NextByte;
    static u32 CodeMasks[] =
    {
        0x0000, 0x0001, 0x0003, 0x0007,
        0x000f, 0x001f, 0x003f, 0x007f,
        0x00ff, 0x01ff, 0x03ff, 0x07ff,
        0x0fff
    };

    while ( m_CrntShiftState < m_RunningBits )
    {
        // Needs to get more bytes from input stream for next code: 
        if ( !DGifBufferedInput( m_Fp, m_Buf, &NextByte ) )
            return FALSE;

        m_CrntShiftDWord |= ((u32) NextByte) << m_CrntShiftState;
        m_CrntShiftState += 8;
    }

    *Code = (s32) m_CrntShiftDWord & CodeMasks[m_RunningBits];

    m_CrntShiftDWord >>= m_RunningBits;
    m_CrntShiftState  -= m_RunningBits;

    // If code cannt fit into RunningBits bits, must raise its size. Note 
    // however that codes above 4095 are used for special signaling.      
    if (++m_RunningCode > m_MaxCode1 && m_RunningBits < LZ_BITS)
    {
        m_MaxCode1 <<= 1;
        m_RunningBits++;
    }

    return TRUE;
}

//==================================================================
// DGifDecompressLine
//------------------------------------------------------------------
// Get one full scanned line (Line) of length LineLen from GIF file.
// The LZ decompression routine:
// This version decompress the given gif file into Line of length LineLen.
// This routine can be called few times (one per scan line, for example), in
// order the complete the whole image.
//==================================================================
static 
xbool DGifDecompressLine( u8* Line, s32 LineLen )
{
    s32     i = 0, j, CrntCode, EOFCode, ClearCode, CrntPrefix, LastCode, StackPtr;
    u8*     Stack,* Suffix;
    u32*    Prefix;

    StackPtr    = m_StackPtr;
    Prefix      = (u32 *)m_Prefix;
    Suffix      = m_Suffix;
    Stack       = m_Stack;
    EOFCode     = m_EOFCode;
    ClearCode   = m_ClearCode;
    LastCode    = m_LastCode;

    if (StackPtr != 0)
    {
        // Let pop the stack off before continueing to read the gif file: 
        while ( StackPtr != 0 && i < LineLen )
            Line[i++] = Stack[--StackPtr];
    }

    // Decode LineLen items. 
    while ( i < LineLen )
    {
        if ( !DGifDecompressInput(&CrntCode) )
            return FALSE;

        if (CrntCode == EOFCode)
        {
            // Note however that usually we will not be here as we will stop 
            // decoding as soon as we got all the pixel, or EOF code will    
            // not be read at all, and DGifGetLine/Pixel clean everything.   
            if (i != LineLen - 1 || m_PixelCount != 0)
                return FALSE;
            i++;
        }
        else if ( CrntCode == ClearCode )
        {
            // We need to start over again: 
            for ( j = 0; j < LZ_MAX_CODE; j++ )
                Prefix[j] = NO_SUCH_CODE;

            m_RunningCode   = m_EOFCode + 1;
            m_RunningBits   = m_BitsPerPixel + 1;
            m_MaxCode1      = 1 << m_RunningBits;
            LastCode        = m_LastCode = NO_SUCH_CODE;
        }
        else
        {
            // Its regular code - if in pixel range simply add it to output  
            // stream, otherwise trace to codes linked list until the prefix 
            // is in pixel range:                        
            if ( CrntCode < ClearCode )
            {
                // This is simple - its pixel scalar, so add it to output:   
                Line[i++] = (u8)CrntCode;
            }
            else
            {
                // Its a code to needed to be traced: trace the linked list  
                // until the prefix is a pixel, while pushing the suffix     
                // pixels on our stack. If we done, pop the stack in reverse 
                // (thats what stack is good for!) order to output.      
                if ( Prefix[CrntCode] == NO_SUCH_CODE )
                {
                    // Only allowed if CrntCode is exactly the running code: 
                    // In that case CrntCode = XXXCode, CrntCode or the      
                    // prefix code is last code and the suffix char is       
                    // exactly the prefix of last code!              
                    if (CrntCode == m_RunningCode - 2)
                    {
                        CrntPrefix = LastCode;
                        Suffix[m_RunningCode - 2] =
                        Stack[StackPtr++] = (u8)DGifGetPrefixChar(Prefix, LastCode, ClearCode);
                    }
                    else
                        return FALSE;
                }
                else
                    CrntPrefix = CrntCode;

                // Now (if image is O.K.) we should not get and NO_SUCH_CODE 
                // During the trace. As we might loop forever, in case of    
                // defective image, we count the number of loops we trace    
                // and stop if we got LZ_MAX_CODE. obviously we can not      
                // loop more than that.                      
                j = 0;

                while ( j++ <= LZ_MAX_CODE && CrntPrefix > ClearCode && CrntPrefix <= LZ_MAX_CODE )
                {
                    Stack[StackPtr++] = Suffix[CrntPrefix];
                    CrntPrefix = Prefix[CrntPrefix];
                }

                if (j >= LZ_MAX_CODE || CrntPrefix > LZ_MAX_CODE)
                    return FALSE;

                // Push the last character on stack: 
                Stack[StackPtr++] = (u8)CrntPrefix;

                // Now lets pop all the stack into output: 
                while ( StackPtr != 0 && i < LineLen )
                    Line[i++] = Stack[--StackPtr];
            }

            if ( LastCode != NO_SUCH_CODE )
            {
                Prefix[m_RunningCode - 2] = LastCode;

                if (CrntCode == m_RunningCode - 2)
                {
                    // Only allowed if CrntCode is exactly the running code: 
                    // In that case CrntCode = XXXCode, CrntCode or the      
                    // prefix code is last code and the suffix char is       
                    // exactly the prefix of last code!              
                    Suffix[m_RunningCode - 2] =
                    (u8)DGifGetPrefixChar(Prefix, LastCode, ClearCode);
                }
                else
                {
                    Suffix[m_RunningCode - 2] =
                    (u8)DGifGetPrefixChar(Prefix, CrntCode, ClearCode);
                }
            }
            LastCode = CrntCode;
        }
    }

    m_LastCode = LastCode;
    m_StackPtr = StackPtr;

    return TRUE;
}

//==================================================================
// DGifGetCodeNext
//------------------------------------------------------------------
// Get one full scanned line (Line) of length LineLen from GIF file.
//==================================================================
static 
xbool GetLine( u8* Line, s32 LineLen )
{
    u8* Dummy;

    if ( ( m_PixelCount -= LineLen ) < 0 )
        return FALSE;

    if ( DGifDecompressLine( Line, LineLen ) )
        return FALSE;

    if (m_PixelCount == 0)
    {
        // We probably would not be called any more, so lets clean       
        // everything before we return: need to flush out all rest of    
        // image until empty block (size 0) detected. We use GetCodeNext.
        do 
        {
            if (!DGifGetCodeNext(&Dummy) ) return FALSE;
        }
        while (Dummy != NULL);
    }
    return FALSE;
}

//==================================================================
// Load
//==================================================================
static 
xbool Load( const char* FileName )
{
    char    Buffer[GIF_STAMP_LEN+1];
    s32     padlen;
    s32     hgt;

    m_Fp = x_fopen( FileName, "rb" );

    if ( m_Fp == NULL ) 
        return FALSE;

    if ( x_fread( Buffer, 1, GIF_STAMP_LEN, m_Fp ) != GIF_STAMP_LEN )
    {
        x_fclose( m_Fp );
        return FALSE;
    }

    // check if valid type
    Buffer[GIF_STAMP_LEN] = 0;
    if (x_strncmp(GIF_STAMP, Buffer, GIF_VERSION_POS) != 0)
    {
        x_fclose (m_Fp);
        return FALSE;
    }

    // read gifinfo from file
    GetScreenDesc();

    if ( !GetRecordType() )
    {
        x_fclose ( m_Fp );
        return FALSE;
    }

    // read gif image info from file
    GetImageDesc();

    // allocate memory for gif picture file & set image size
    padlen = ( m_GifInfo.img_len + 3 ) & (~3);
    m_GifInfo.img_size = padlen * m_GifInfo.img_hgt;

    m_DesBmp.AWidth         = m_GifInfo.img_len;
    m_DesBmp.AHeight        = m_GifInfo.img_hgt;
    m_DesBmp.Width          = padlen ;
    m_DesBmp.Height         = m_GifInfo.img_hgt ;
    m_DesBmp.ColorDepth     = m_GifInfo.scr_bitsperpixel ;
    m_DesBmp.ColorDepth     = 1 ;

    // Allocate the picture
    m_DesBmp.Data = (u8*)x_malloc( m_DesBmp.Width * m_DesBmp.Height * m_DesBmp.ColorDepth ) ;
    if ( m_DesBmp.Data == NULL )
    {
        x_fclose ( m_Fp );
        return FALSE;
    }

    // Alloc the palette
    m_DesBmp.Palette = (rgb*)x_malloc (sizeof(rgb) * 256 );
    ASSERT( m_DesBmp.Palette ) ;

    // read in all lines of gif file
    for (hgt = 0; hgt < m_GifInfo.img_hgt; hgt++)
    {

        // decode a single line
        if ( GetLine( m_DesBmp.Data + (padlen * hgt), m_GifInfo.img_len ) )
        {
            x_fclose ( m_Fp );
            return FALSE;
        }
    }

    x_memcpy( m_DesBmp.Palette, m_GIFPalette, sizeof(rgb) * 256 );

    x_fclose( m_Fp );

    return TRUE;
}

//==================================================================
// BGIF_Load
//==================================================================
xbool GIF_Load( x_bitmap& This, const char* FileName )
{
    ASSERT(FileName);

    if ( !Load( FileName ) ) 
        return FALSE;

    This.SetupBitmap( x_bitmap::FMT_P8_RGB_888,        //  Format,         
                      m_DesBmp.AWidth,                 //  Width,          
                      m_DesBmp.AHeight,                //  Height,         
                      m_DesBmp.Width,                  //  PWidth,         
                      m_DesBmp.Height,                 //  PHeight,        
                      TRUE,                            //  IsDataOwned     
                      m_DesBmp.Height*m_DesBmp.Width,  //  DataSizeInBytes 
                      m_DesBmp.Data,                   //  PixelData,      
                      TRUE,                            //  IsClutOwned     
                      256*3,                           //  ClutSizeInBytes 
                      (byte*)m_DesBmp.Palette,         //  ClutData,       
                      256,
                      0);

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
