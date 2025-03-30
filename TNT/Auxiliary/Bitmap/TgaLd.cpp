///////////////////////////////////////////////////////////////////////////
//
//  B_TGALD.C
//
///////////////////////////////////////////////////////////////////////////

#include "x_plus.hpp"
#include "x_stdio.hpp"
#include "x_memory.hpp"
#include "x_debug.hpp"
#include "TgaLd.hpp"

#define RETURN_FAILURE(msg)   { x_printf("%s\n",(msg)); return FALSE; }

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// STRUCTURES
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
#ifdef TARGET_PC
#pragma pack (push,1)
#endif

// TGA File Header 
typedef struct s_TGAHEADER
{
    u8      IdLength;       // Image ID Field Length 
    u8      CmapType;       // Color Map Type 
    u8      ImageType;      // Image Type 

    // Color Map Specification 
    u16     CmapIndex;      // First Entry Index 
    u16     CmapLength;     // Color Map Length 
    u8      CmapEntrySize;  // Color Map Entry Size 

    // Image Specification 
    u16     X_Origin;       // X-origin of Image 
    u16     Y_Origin;       // Y-origin of Image 
    u16     ImageWidth;     // Image Width 
    u16     ImageHeight;    // Image Height 
    u8      PixelDepth;     // Pixel Depth 
    u8      ImagDesc;       // Image Descriptor 
} t_TGAHEADER;

// Image/Color Map Data 
typedef struct s__TGACMAP
{
    u8      *IdField;       // Image ID Field 
    u8      *CmapData;      // Color Map Data 
} t_TGACMAP;


// Developer Area Tag Structure 
typedef struct s_TGATAG
{
    u16     TagNumber;      // Number of the Tag 
    u32     TagOffset;      // Offset of the Tag Data 
    u32     TagSize;        // Size of the Tag Data 
    u8      *TagData;       // Pointer to the Tag Data 
    struct s_TGATAG *Next;  // Link to next Tag 
} t_TGATAG;


// Developer Area 
typedef struct s_TGADEVELOPER
{
    u16         NumberOfTags;   // Number of Tags in Directory 
    t_TGATAG    *TagList;       // Link to list of Tags 
} t_TGADEVELOPER;


// Extension Area 
typedef struct s__TGAEXTENSION
{
    u16     Size;               // Extension Size 
    u8      AuthorName[41];     // Author Name 
    u8      AuthorComment[324]; // Author Comment 
    u16     StampMonth;         // Date/Time Stamp: Month 
    u16     StampDay;           // Date/Time Stamp: Day 
    u16     StampYear;          // Date/Time Stamp: Year 
    u16     StampHour;          // Date/Time Stamp: Hour 
    u16     StampMinute;        // Date/Time Stamp: Minute 
    u16     StampSecond;        // Date/Time Stamp: Second 
    u8      JobName[41];        // Job Name/ID 
    u16     JobHour;            // Job Time: Hours 
    u16     JobMinute;          // Job Time: Minutes 
    u16     JobSecond;          // Job Time: Seconds 
    u8      SoftwareId[41];     // Software ID 
    u16     VersionNumber;      // Software Version Number 
    u8      VersionLetter;      // Software Version Letter 
    u32     KeyColor;           // Key Color 
    u16     PixelNumerator;     // Pixel Aspect Ratio Numerator 
    u16     PixelDenominator;   // Pixel Aspect Ratio Denominator 
    u16     GammaNumerator;     // Gamma Value 
    u16     GammaDenominator;   // Gamma Value 
    u32     ColorOffset;        // Color Correction Offset 
    u32     StampOffset;        // Postage Stamp Offset 
    u32     ScanOffset;         // Scan Line Table Offset 
    u8      AttributesType;     // Attributes Types 
    u32     *ScanLineTable;     // Scan Line Table 
    u8      StampWidth;         // Width of postage stamp image 
    u8      StampHeight;        // Height of postage stamp image 
    u8      *StampImage;        // Postage Stamp Image 
    u16     ColorTable[1024];   // Color Correction Table 
} t_TGAEXTENSION;


// TGA File Footer 
typedef struct s_TGAFOOTER
{
    u32     ExtensionOffset;    // Extension Area Offset 
    u32     DeveloperOffset;    // Developer Directory Offset 
    u8      Signature[18];      // Signature, dot, and NULL 
} t_TGAFOOTER;

#ifdef TARGET_PC
#pragma pack (pop)
#endif

// Complete TGA File Format 
typedef struct s_TGAFORMAT
{
    t_TGAHEADER     Head;           // Header Area 
    t_TGACMAP       Cmap;           // Image/Color Map Area 
    t_TGADEVELOPER  Developer;      // Developer Area 
    t_TGAEXTENSION  Extension;      // Extension Area 
    t_TGAFOOTER     Foot;           // File Footer Area 
    s8              NewTgaFormat;   // TRUE if file is v2.0 TGA format (was BOOL type [carl]) 
    u32             TgaDataOffset;  // Offset of the image data in the file 
} t_TGA;

#define TGASIGNATURE   "TRUEVISION-XFILE.\0"
#define BYTESPERPIXEL   ((m_TGAHead.Head.PixelDepth + 7) >> 3)

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// GLOBALS
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static t_TGA        m_TGAHead;
static X_FILE*      m_FP;
static s32          m_Width;
static s32          m_Height;
static s32          m_ByteDepth;
static byte*        m_PixelData;

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// PROTOTYPES
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static xbool      ReadTGAHeader(t_TGA *TgaHead, X_FILE *FpTga) ;
static xbool      TGADecodeScanLine(u8 *DecodedBuffer, u32 LineLength, u32 PixelSize, X_FILE *FpTga) ;
static u16      GetWord (X_FILE *fptga) ;
//static u32      GetDword (X_FILE *fptga) ;
static byte     GetByte (X_FILE *fptga) ;

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

xbool TGA_Load(const char *filename)
{
    s32     i, j, k ;           // Loop Counters
    u8      *Ptr1, *Ptr2 ;      // Scanning Pointers
    u8      ImageFlipped;       // is it stored y flipped

    // Open file
    m_FP = x_fopen(filename, "rb");
    if (m_FP == NULL)
        RETURN_FAILURE("Unable to open TGA file for reading");

    //////////////////////////////////////////
    // Read the TGA image file header 
    // information and decide if format is 
    // recognized.  FP should now be at
    // pixel data.
    //////////////////////////////////////////
    if (ReadTGAHeader(&m_TGAHead, m_FP) == FALSE)
    {
        x_fclose(m_FP);
        RETURN_FAILURE("Unable to read TGA header");
    }

    //////////////////////////////////////////
    // Allocate pixel data for final image
    //////////////////////////////////////////
    m_Width     = m_TGAHead.Head.ImageWidth ;
    m_Height    = m_TGAHead.Head.ImageHeight ;
    m_ByteDepth = BYTESPERPIXEL ;
    m_PixelData = (byte*)x_malloc( sizeof(byte) * m_Width * m_Height * m_ByteDepth );
    if (!m_PixelData) RETURN_FAILURE("Out of memory loading TGA");
    ImageFlipped = (u8) (!(m_TGAHead.Head.ImagDesc & 0x20)) ;

    //////////////////////////////////////////
    // Decide whether data is 'encoded' or not
    //////////////////////////////////////////
    if (m_TGAHead.Head.ImageType == 10 )
    {
        if (TGADecodeScanLine( m_PixelData, 
                               m_TGAHead.Head.ImageWidth * m_TGAHead.Head.ImageHeight, 
                               BYTESPERPIXEL, m_FP) == FALSE) 
        {
            x_free(m_PixelData);
            RETURN_FAILURE("Unable to decode scanline in TGA");
        }
    }
    else 
    if (m_TGAHead.Head.ImageType == 2 )
    {
        x_fread (m_PixelData, 1, m_TGAHead.Head.ImageWidth*m_TGAHead.Head.ImageHeight*BYTESPERPIXEL, m_FP) ;
    }
    else
    if (m_TGAHead.Head.ImageType == 1 )
    {
        RETURN_FAILURE("Unable to load a palettized TGA");
    }
    else
    {
        ASSERT(FALSE);
    }

    //////////////////////////////////////////
    // post process image
    // if flipped vertically, flip it back
    //////////////////////////////////////////
    if (ImageFlipped)
    {
        Ptr1 = (u8*)m_PixelData ;                                     // First Line
        Ptr2 = (u8*)m_PixelData + (m_Height-1)*m_Width*m_ByteDepth ;  // Last Line
        while (Ptr1 < Ptr2)
        {
            for (j = m_Width*m_ByteDepth ; j > 0 ; j--)
            {
                k = *Ptr1 ;
                *Ptr1++ = *Ptr2 ;
                *Ptr2++ = (u8)k ;
            }
            Ptr2 -= 2*m_Width*m_ByteDepth ;
        }
    }

    //////////////////////////////////////////
    // post process image
    // if 24 bit, swap R with B
    //////////////////////////////////////////
    if (m_ByteDepth >= 3)
    {
        Ptr1 = (u8*)m_PixelData ;
        Ptr2 = (u8*)m_PixelData + 2 ;
        for (i = m_Width*m_Height ; i > 0 ; i--)
        {
            k = *Ptr1 ;
            *Ptr1 = *Ptr2 ;
            *Ptr2 = (u8)k ;
            Ptr1 += m_ByteDepth;
            Ptr2 += m_ByteDepth;
        }
    }

    //////////////////////////////////////////
    // post process image
    // if 16 bit, convert 1555 to 5551
    //////////////////////////////////////////
    if (m_ByteDepth == 2)
    {
        u16  S;
        byte* Ptr = (byte*)m_PixelData;
        for (i=0; i<m_Width*m_Height; i++)
        {
            S = ((u16)(*Ptr)) | (((u16)(*(Ptr+1)))<<8);
            *((u16*)Ptr) = (S<<1);
            Ptr+=2;
        }
    }


    x_fclose(m_FP) ;

    return TRUE ;
}



///////////////////////////////////////////////////////////////////////////

static xbool ReadTGAHeader(t_TGA *TgaHead, X_FILE *FpTga)
{
    s32 i,cmapsize;

    // these _may_ be alloc'ed below.  need to be initialized
    TgaHead->Cmap.IdField       = NULL;
    TgaHead->Cmap.CmapData      = NULL;

    /* Read the TGA header (Version 1.0 and 2.0). */
    TgaHead->Head.IdLength      = GetByte(FpTga);
    TgaHead->Head.CmapType      = GetByte(FpTga);
    TgaHead->Head.ImageType     = GetByte(FpTga);
    TgaHead->Head.CmapIndex     = GetWord(FpTga);
    TgaHead->Head.CmapLength    = GetWord(FpTga);
    TgaHead->Head.CmapEntrySize = GetByte(FpTga);
    TgaHead->Head.X_Origin      = GetWord(FpTga);
    TgaHead->Head.Y_Origin      = GetWord(FpTga);
    TgaHead->Head.ImageWidth    = GetWord(FpTga);
    TgaHead->Head.ImageHeight   = GetWord(FpTga);
    TgaHead->Head.PixelDepth    = GetByte(FpTga);
    TgaHead->Head.ImagDesc      = GetByte(FpTga);

	// Skip the Image ID field. 
	if (TgaHead->Head.IdLength)
	{
		for (i = 0; i < TgaHead->Head.IdLength; i++)
			GetByte(FpTga);
	}

	// Skip the color map data (Version 1.0 and 2.0). 
	if (TgaHead->Head.CmapType)
	{
		cmapsize = ((TgaHead->Head.CmapEntrySize + 7) >> 3) * TgaHead->Head.CmapLength;
		for (i = 0; i < cmapsize; i++)
			GetByte(FpTga);
	}

    // Decide if we support this format
    if ( (TgaHead->Head.ImageType != 2) ||      // Uncompressed RGB
         (TgaHead->Head.ImageType != 10) )      // Compressed RGB
    {
        return TRUE;
    }

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////

static xbool TGADecodeScanLine(u8 *DecodedBuffer, u32 LineLength, u32 PixelSize, X_FILE *FpTga)
{
    u32     i = 0 ;             // Loop counter 
    s32     byteCount = 0 ;     // Number of bytes written to the buffer 
    u32     runCount = 0 ;      // The pixel run count 
    u32     bufIndex = 0 ;      // The index of DecodedBuffer 
    u32     bufMark = 0 ;       // Index marker of DecodedBuffer 
    u32     pixelCount = 0 ;    // The number of pixels read from the scan line 


    while (pixelCount < LineLength)
    {
        // Get the pixel count 
        runCount = GetByte(FpTga);

        // Make sure writing this next run will not overflow the buffer 
        if (pixelCount + (runCount & 0x7f) + 1 > LineLength)
            RETURN_FAILURE("Error decompressing TGA");

        // If the run is encoded... 
        if (runCount & 0x80)
        {
            runCount &= ~0x80;              // Mask off the upper bit 

            bufMark = bufIndex;             // Save the start-of-run index 

            // Update total pixel count 
            pixelCount += (runCount + 1);

            // Update the buffer byte count 
            byteCount += ((runCount + 1) * PixelSize);

            // Write the first pixel of the run to the buffer 
            for (i = 0; i < PixelSize; i++)
                DecodedBuffer[bufIndex++] = GetByte(FpTga);

            // Write remainder of pixel run to buffer 'runCount' times 
            while (runCount--)
            {
                for (i = 0; i < PixelSize; i++)
                    DecodedBuffer[bufIndex++] = DecodedBuffer[bufMark + i];
            }
        }
        else    // ...the run is unencoded (raw) 
        {
            // Update total pixel count 
            pixelCount += (runCount + 1);

            // Update the buffer byte count 
            byteCount  += ((runCount + 1) * PixelSize);

            // Write runCount pixels 
            do
            {
                for (i = 0; i < PixelSize; i++)
                    DecodedBuffer[bufIndex++] = GetByte(FpTga);
            }
            while (runCount--);
        }
    }
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////

static u8 GetByte (X_FILE *fptga)
{
    byte value ;
    x_fread (&value, 1, sizeof(byte), fptga) ;
    return  (value) ;
}

///////////////////////////////////////////////////////////////////////////

static u16 GetWord (X_FILE *fptga)
{
    u16     value ;
    x_fread (&value, 1, sizeof(u16), fptga) ;
    return  (value) ;
}

///////////////////////////////////////////////////////////////////////////
/*
static u32 GetDword (X_FILE *fptga)
{
    u32     value ;
    x_fread (&value, 1, sizeof(u32), fptga) ;
    return  (value) ;
}
*/
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

xbool TGA_Load( x_bitmap& BMP, const char* Filename )
{

    if (!TGA_Load(Filename))
    {
        return FALSE;
    }

    if (m_ByteDepth == 2)
    {
        // Build 16bit 5551 bitmap
        BMP.SetupBitmap( x_bitmap::FMT_16_RGBU_5551,         // Format,
                         m_Width,
                         m_Height,
                         m_Width,
                         m_Height,
                         TRUE,
                         m_Width*m_Height*m_ByteDepth, // DataSizeInBytes,
                         m_PixelData,
                         FALSE,
                         FALSE,
                         NULL,
                         0 );

        m_PixelData = NULL;
        return TRUE;
    }
    else
    if (m_ByteDepth == 3)
    {
        // Build 24bit 888 bitmap
        BMP.SetupBitmap( x_bitmap::FMT_24_RGB_888,         // Format,
                         m_Width,
                         m_Height,
                         m_Width,
                         m_Height,
                         TRUE,
                         m_Width*m_Height*m_ByteDepth, // DataSizeInBytes,
                         m_PixelData,
                         FALSE,
                         FALSE,
                         NULL,
                         0 );

        m_PixelData = NULL;
        return TRUE;
    }
    else
    if (m_ByteDepth == 4)
    {
        // Build 24bit 888 bitmap
        BMP.SetupBitmap( x_bitmap::FMT_32_ABGR_8888,         // Format,
                         m_Width,
                         m_Height,
                         m_Width,
                         m_Height,
                         TRUE,
                         m_Width*m_Height*m_ByteDepth, // DataSizeInBytes,
                         m_PixelData,
                         FALSE,
                         FALSE,
                         NULL,
                         0 );

        m_PixelData = NULL;
        return TRUE;
    }

    RETURN_FAILURE("Unknown bytedepth...not 16 or 24bit");
}

///////////////////////////////////////////////////////////////////////////
/*
void TGA_PrintInfo(void)
{
    x_printf("TGA Info:\n");
    x_printf("Width:         %1d\n",m_Width);
    x_printf("Height:        %1d\n",m_Height);
    x_printf("BytesPerPixel: %1d\n",m_ByteDepth);
    x_printf("ImageType:     %1d\n",m_TGAHead.Head.ImageType);
}

///////////////////////////////////////////////////////////////////////////

void TGA_DumpRaw(void)
{
    X_FILE* fp;
    s32 i;

    fp = x_fopen("d:/temp/rawtga.rgb","wb");
    ASSERT(fp);

    if (m_ByteDepth==3)
    {
        for (i=0; i<m_Width*m_Height; i++)
        {
            x_fputc(m_PixelData[(i*3)+0],fp);
            x_fputc(m_PixelData[(i*3)+1],fp);
            x_fputc(m_PixelData[(i*3)+2],fp);
        }
    }
    else
    if (m_ByteDepth==2)
    {
        u16 S;
        for (i=0; i<m_Width*m_Height; i++)
        {
            S = ((u16*)m_PixelData)[i];
            x_fputc((S>>8)&0x00F8,fp);
            x_fputc((S>>3)&0x00F8,fp);
            x_fputc((S<<2)&0x00F8,fp);
        }
    }

    x_fclose(fp);
}

///////////////////////////////////////////////////////////////////////////
*/
