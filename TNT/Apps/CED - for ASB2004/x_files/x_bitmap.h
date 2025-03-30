///////////////////////////////////////////////////////////////////////////
//
//  X_BITMAP.H
//
///////////////////////////////////////////////////////////////////////////

#ifndef X_BITMAP_H
#define X_BITMAP_H

///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////

#ifndef X_TYPES_H
#include "x_types.h"
#endif

///////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////////////////

//*************************************************************************
// WARNING - These defines are used as indexes into tables.
//           Do not change under any circumstances!  
//*************************************************************************

#define BMP_FMT_NONE                 0

#define BMP_FMT_16_RGB_565           1
#define BMP_FMT_16_ARGB_4444         2
#define BMP_FMT_16_RGBA_4444         3
#define BMP_FMT_16_ARGB_1555         4
#define BMP_FMT_16_RGBA_5551         5
#define BMP_FMT_16_URGB_1555         6
#define BMP_FMT_16_RGBU_5551         7

#define BMP_FMT_24_RGB_888           8
#define BMP_FMT_24_ARGB_8565         9

#define BMP_FMT_32_RGBU_8888        10
#define BMP_FMT_32_URGB_8888        11
#define BMP_FMT_32_ARGB_8888        12
#define BMP_FMT_32_RGBA_8888        13

#define BMP_FMT_P4_RGB_565          14
#define BMP_FMT_P4_RGB_888          15
#define BMP_FMT_P4_URGB_8888        16
#define BMP_FMT_P4_RGBU_8888        17
#define BMP_FMT_P4_ARGB_8888        18
#define BMP_FMT_P4_RGBA_8888        19
#define BMP_FMT_P4_RGBA_5551        20
#define BMP_FMT_P4_ARGB_1555        21
#define BMP_FMT_P4_RGBU_5551        22
#define BMP_FMT_P4_URGB_1555        23

#define BMP_FMT_P8_RGB_565          24
#define BMP_FMT_P8_RGB_888          25
#define BMP_FMT_P8_URGB_8888        26
#define BMP_FMT_P8_RGBU_8888        27
#define BMP_FMT_P8_ARGB_8888        28   // This is the 'color' format.
#define BMP_FMT_P8_RGBA_8888        29
#define BMP_FMT_P8_RGBA_5551        30
#define BMP_FMT_P8_ARGB_1555        31
#define BMP_FMT_P8_RGBU_5551        32
#define BMP_FMT_P8_URGB_1555        33

#define BMP_FMT_I4                  34
#define BMP_FMT_I8                  35

#define BMP_FMT_P4_ABGR_1555        36   // New formats for PSX support
#define BMP_FMT_P4_UBGR_1555        37
#define BMP_FMT_P8_ABGR_1555        38
#define BMP_FMT_P8_UBGR_1555        39
#define BMP_FMT_16_ABGR_1555        40
#define BMP_FMT_16_UBGR_1555        41

#define BMP_FMT_NUM_FORMATS         42

//*************************************************************************
// WARNING - Those defines are used as indexes into tables.
//           Do not change under any circumstances!  
//*************************************************************************

#define BMP_FLAG_DATA_OWNED             ( 1 << 0 )
#define BMP_FLAG_CLUT_OWNED             ( 1 << 1 )

///////////////////////////////////////////////////////////////////////////
// TYPES
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//  FORMAT_INFO
//  Given a color C, the 8bit R,G,B,A can be decoded...
//      u8 R = (RShift<0) ? ((C & RMask) << (-RShift)) : ((C & RMask) >> (RShift));
//      u8 G = (GShift<0) ? ((C & GMask) << (-GShift)) : ((C & GMask) >> (GShift)); 
//      u8 B = (BShift<0) ? ((C & BMask) << (-BShift)) : ((C & BMask) >> (BShift)); 
//      u8 A = (AShift<0) ? ((C & AMask) << (-AShift)) : ((C & AMask) >> (AShift)); 
///////////////////////////////////////////////////////////////////////////

typedef struct
{
    s32   Format;        // Format define
    xbool IsClutUsed;    // Is a palette used?
    s32   BPP;           // Bits per pixel
    s32   BPC;           // Bits per color
    s32   BU;            // Bits used
    u32   RMask;         // Red   mask
    u32   GMask;         // Green mask
    u32   BMask;         // Blue  mask
    u32   AMask;         // Alpha mask
    s32   RShift;        // Red   shift
    s32   GShift;        // Green shift
    s32   BShift;        // Blue  shift
    s32   AShift;        // Alpha shift

} x_bitmap_format;


typedef struct
{
    u8      Format;             // Data format from define list
    u8      Flags;              // Flags 
    s16     NClutColors;        // Number of colors in the clut
    s16     W,  H;              // Width, Height ( In Pixels )
    s16     PW, PH;             // Physical Width, Height ( In Pixels )
    s32     DataSize;           // Size of the data ( In bytes )
    s32     ClutSize;           // Size of the clut ( In bytes )
    byte*   Clut;               // Pointer for the clut memory
    byte*   Data;               // Pointer for the pixel memory
    byte    Pad[4];             // Just here to achieve 32 bit alignment

} x_bitmap;


///////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////

void        BMP_InitInstance         ( x_bitmap* Bmp );
void        BMP_KillInstance         ( x_bitmap* Bmp );

err         BMP_SanityCheck          ( x_bitmap* Bmp );
err         BMP_Copy                 ( x_bitmap* Dest, x_bitmap* Source );

///////////////////////////////////////////////////////////////////////////
// GETTERS AND QUERIES
///////////////////////////////////////////////////////////////////////////

s32         BMP_GetFormat            ( x_bitmap* Bmp );
s32         BMP_GetBPP               ( x_bitmap* Bmp );
s32         BMP_GetBPC               ( x_bitmap* Bmp );
s32         BMP_GetNClutColors       ( x_bitmap* Bmp );
s32         BMP_GetWidth             ( x_bitmap* Bmp );
s32         BMP_GetHeight            ( x_bitmap* Bmp );
s32         BMP_GetPWidth            ( x_bitmap* Bmp );
s32         BMP_GetPHeight           ( x_bitmap* Bmp );

s32         BMP_GetClutSize          ( x_bitmap* Bmp );
s32         BMP_GetDataSize          ( x_bitmap* Bmp );
byte*       BMP_GetDataPtr           ( x_bitmap* Bmp );
byte*       BMP_GetClutPtr           ( x_bitmap* Bmp );

xbool       BMP_IsClutBased          ( x_bitmap* Bmp );
xbool       BMP_IsAlpha              ( x_bitmap* Bmp );
xbool       BMP_IsClutOwned          ( x_bitmap* Bmp );
xbool       BMP_IsDataOwned          ( x_bitmap* Bmp );

///////////////////////////////////////////////////////////////////////////
// BUILDING A BITMAP FROM SCRATCH
///////////////////////////////////////////////////////////////////////////

void        BMP_SetupBitmap          ( x_bitmap* Bmp,
                                       s32       Format,
                                       s32       Width,
                                       s32       Height,
                                       s32       PWidth,
                                       s32       PHeight,
                                       xbool     IsDataOwned,
                                       s32       DataSizeInBytes,
                                       byte*     PixelData,
                                       xbool     IsClutOwned,
                                       s32       ClutSizeInBytes,
                                       byte*     ClutData,
                                       s32       NClutColors );

///////////////////////////////////////////////////////////////////////////
// FORMAT INFO
///////////////////////////////////////////////////////////////////////////
//
// GetFormatInfo will fill out the FormatInfo structure with the correct
// information for that particular FormatID.  FindFormat will search 
// through it's internal tables and look for the format that matches the
// description in FormatInfo.  The search only involves comparing IsClutUsed,
// BPP, and the four masks.  If the format was not found FindFormat will
// return -1.
//
///////////////////////////////////////////////////////////////////////////

void        BMP_GetFormatInfo       ( s32 FormatID, x_bitmap_format* FormatInfo );
s32         BMP_FindFormat          ( x_bitmap_format* FormatInfo );

///////////////////////////////////////////////////////////////////////////
// MAGIC PUT AND GET PIXEL
///////////////////////////////////////////////////////////////////////////
//
// Using the Get and Set routines below allows you to read and write color
// and index data while not worrying about format specifics.  You must 
// call SetPixelFormat which will setup a number of values allowing the
// set and get routines to operate as quickly as possible.  If you are only
// going to read or only going to write then you may pass in NULL for the
// DstBMP or SrcBMP in SetPixelFormat.  If you do not call SetPixelFormat
// before the set and get calls the behavior is undefined.
//
///////////////////////////////////////////////////////////////////////////

void        BMP_SetPixelFormat      ( x_bitmap* DstBMP, x_bitmap* SrcBMP );

color       BMP_GetPixelColor       ( s32 X, s32 Y );
s32         BMP_GetPixelIndex       ( s32 X, s32 Y );
color       BMP_GetClutColor        ( s32 Index );
                            
void        BMP_SetPixelColor       ( s32 X, s32 Y, color C );
void        BMP_SetPixelIndex       ( s32 X, s32 Y, s32 Index );
void        BMP_SetClutColor        ( s32 Index, color C );
                       
///////////////////////////////////////////////////////////////////////////
// FORMAT CONVERSIONS
///////////////////////////////////////////////////////////////////////////
//
// These three routines are used strictly to convert between color formats.
// To be honest they are quite slow to be using heavily.
//
///////////////////////////////////////////////////////////////////////////

u32         BMP_ColorToFormat       ( color SrcColor, s32 DstFormat );
color       BMP_FormatToColor       ( u32 SrcColor, s32 SrcFormat );
u32         BMP_FormatToFormat      ( u32 SrcColor, s32 SrcFormat, s32 DstFormat );
                                  
///////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

///////////////////////////////////////////////////////////////////////////

#endif
