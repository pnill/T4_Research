///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////

#ifndef X_BITMAP_H
#include "x_bitmap.h"
#endif

#ifndef X_DEBUG_H
#include "x_debug.h"
#endif

#ifndef X_MEMORY_H
#include "x_memory.h"
#endif

#ifndef X_PLUS_H
#include "x_plus.h"
#endif

#ifndef X_COLOR_H
#include "x_color.h"
#endif

///////////////////////////////////////////////////////////////////////////
// MACROS FOR THE FORMAT TABLE
///////////////////////////////////////////////////////////////////////////

//--- Sets A bits on and then shifts them right B bits.
#define M_BITM(A,B)                     (((1<<(A))-1) << (B)) 

//--- Given A bits at B position finds how much to shift left to get a 8bpp.
#define M_SFTM(A,B)                     (( (A) - 8 ) + (B))

//--- This macro expands to Bit-per-color, all the masks, and all the shifts.
#define M_ARGBM(A,AL,R,RL,G,GL,B,BL)                                            \
                        ((R)+(G)+(B)+(A)),                                      \
                        M_BITM(R,RL), M_BITM(G,GL), M_BITM(B,BL), M_BITM(A,AL), \
                        M_SFTM(R,RL), M_SFTM(G,GL), M_SFTM(B,BL), M_SFTM(A,AL)

//--- Macros used when we have alpha.
#define SET_ARGB(A,R,G,B) M_ARGBM( A,(R)+(G)+(B),   R,(G)+(B),       G,(B),       B,0 )
#define SET_RGBA(R,G,B,A) M_ARGBM( A,0,             R,(G)+(B)+(A),   G,(B)+(A),   B,(A) )

//--- Macro used when we don't have alpha.
#define SET_URGB(A,R,G,B) M_ARGBM( 0,(R)+(G)+(B),   R,(G)+(B),       G,(B),       B,0 )

//--- Macro used when we don't have alpha or when there are no unused bits.
#define SET_RGBU(R,G,B,A) M_ARGBM( 0,0,             R,(G)+(B)+(A),   G,(B)+(A),   B,(A) )

//--- Macros used when we have alpha. for PSX
#define SET_ABGR(A,B,G,R) M_ARGBM( A,(B)+(G)+(R),   R,0,             G,(R),       B,(G)+(R) )

//--- Macro used when we don't have alpha. for PSX
#define SET_UBGR(A,B,G,R) M_ARGBM( 0,(B)+(G)+(R),   R,0,             G,(R),       B,(G)+(R) )


///////////////////////////////////////////////////////////////////////////
// VARIABLES
///////////////////////////////////////////////////////////////////////////

static x_bitmap_format m_Format[] = 
{
    // FORMAT                  CLUT? BPP BPC  COLOR INFO
    { BMP_FMT_NONE           , FALSE,  0,  0, SET_ARGB(0,0,0,0) },
    { BMP_FMT_16_RGB_565     , FALSE, 16, 16, SET_RGBU(5,6,5,0) },
    { BMP_FMT_16_ARGB_4444   , FALSE, 16, 16, SET_ARGB(4,4,4,4) },
    { BMP_FMT_16_RGBA_4444   , FALSE, 16, 16, SET_RGBA(4,4,4,4) },
    { BMP_FMT_16_ARGB_1555   , FALSE, 16, 16, SET_ARGB(1,5,5,5) },
    { BMP_FMT_16_RGBA_5551   , FALSE, 16, 16, SET_RGBA(5,5,5,1) },
    { BMP_FMT_16_URGB_1555   , FALSE, 16, 16, SET_URGB(1,5,5,5) },
    { BMP_FMT_16_RGBU_5551   , FALSE, 16, 16, SET_RGBU(5,5,5,1) },
                                             
    { BMP_FMT_24_RGB_888     , FALSE, 24, 24, SET_RGBU(8,8,8,0) },
    { BMP_FMT_24_ARGB_8565   , FALSE, 24, 24, SET_ARGB(8,5,6,5) },     
                                             
    { BMP_FMT_32_RGBU_8888   , FALSE, 32, 32, SET_RGBU(8,8,8,8) },
    { BMP_FMT_32_URGB_8888   , FALSE, 32, 32, SET_URGB(8,8,8,8) },
    { BMP_FMT_32_ARGB_8888   , FALSE, 32, 32, SET_ARGB(8,8,8,8) },
    { BMP_FMT_32_RGBA_8888   , FALSE, 32, 32, SET_RGBA(8,8,8,8) },      
                                             
    { BMP_FMT_P4_RGB_565     ,  TRUE,  4, 16, SET_RGBU(5,6,5,0) },
    { BMP_FMT_P4_RGB_888     ,  TRUE,  4, 24, SET_RGBU(8,8,8,0) },
    { BMP_FMT_P4_URGB_8888   ,  TRUE,  4, 32, SET_URGB(8,8,8,8) },
    { BMP_FMT_P4_RGBU_8888   ,  TRUE,  4, 32, SET_RGBU(8,8,8,8) },
    { BMP_FMT_P4_ARGB_8888   ,  TRUE,  4, 32, SET_ARGB(8,8,8,8) },
    { BMP_FMT_P4_RGBA_8888   ,  TRUE,  4, 32, SET_RGBA(8,8,8,8) },
    { BMP_FMT_P4_RGBA_5551   ,  TRUE,  4, 16, SET_RGBA(5,5,5,1) },
    { BMP_FMT_P4_ARGB_1555   ,  TRUE,  4, 16, SET_ARGB(1,5,5,5) },                                       
    { BMP_FMT_P4_RGBU_5551   ,  TRUE,  4, 16, SET_RGBU(5,5,5,1) },
    { BMP_FMT_P4_URGB_1555   ,  TRUE,  4, 16, SET_URGB(1,5,5,5) },                                       
                                             
    { BMP_FMT_P8_RGB_565     ,  TRUE,  8, 16, SET_RGBU(5,6,5,0) },
    { BMP_FMT_P8_RGB_888     ,  TRUE,  8, 24, SET_RGBU(8,8,8,0) },
    { BMP_FMT_P8_URGB_8888   ,  TRUE,  8, 32, SET_URGB(8,8,8,8) },
    { BMP_FMT_P8_RGBU_8888   ,  TRUE,  8, 32, SET_RGBU(8,8,8,8) },
    { BMP_FMT_P8_ARGB_8888   ,  TRUE,  8, 32, SET_ARGB(8,8,8,8) },
    { BMP_FMT_P8_RGBA_8888   ,  TRUE,  8, 32, SET_RGBA(8,8,8,8) },
    { BMP_FMT_P8_RGBA_5551   ,  TRUE,  8, 16, SET_RGBA(5,5,5,1) },
    { BMP_FMT_P8_ARGB_1555   ,  TRUE,  8, 16, SET_ARGB(1,5,5,5) },
    { BMP_FMT_P8_RGBU_5551   ,  TRUE,  8, 16, SET_RGBU(5,5,5,1) },
    { BMP_FMT_P8_URGB_1555   ,  TRUE,  8, 16, SET_URGB(1,5,5,5) }, 
                                             
    { BMP_FMT_I4             ,  TRUE,  4,  4, SET_RGBU(4,0,0,0) },
    { BMP_FMT_I8             ,  TRUE,  8,  8, SET_RGBU(8,0,0,0) },

    { BMP_FMT_P4_ABGR_1555   , TRUE,   4, 16, SET_ABGR(1,5,5,5) },  
    { BMP_FMT_P4_UBGR_1555   , TRUE,   4, 16, SET_UBGR(1,5,5,5) },  
    { BMP_FMT_P8_ABGR_1555   , TRUE,   8, 16, SET_ABGR(1,5,5,5) },  
    { BMP_FMT_P8_UBGR_1555   , TRUE,   8, 16, SET_UBGR(1,5,5,5) },  
    { BMP_FMT_16_ABGR_1555   , FALSE, 16, 16, SET_ABGR(1,5,5,5) },  
    { BMP_FMT_16_UBGR_1555   , FALSE, 16, 16, SET_UBGR(1,5,5,5) } 
};

///////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////

//=========================================================================
// BMP_InitInstance
//=========================================================================
void BMP_InitInstance( x_bitmap* Bmp )
{
    ASSERT( Bmp );
    x_memset( Bmp, 0, sizeof(x_bitmap) );
}

//=========================================================================
// BMP_InitInstance
//=========================================================================
void BMP_KillInstance( x_bitmap* Bmp )
{
    ASSERT( Bmp );

    if( Bmp->Flags & BMP_FLAG_DATA_OWNED )
    {
        ASSERT( Bmp->Data );
        x_free( Bmp->Data );
    }

    if( Bmp->Flags & BMP_FLAG_CLUT_OWNED )
    {
        if (Bmp->ClutSize > 0)
        {
            ASSERT( Bmp->Clut );
            x_free( Bmp->Clut );
        }
    }

    BMP_InitInstance( Bmp );
}

//=========================================================================
// BMP_InitInstance
//=========================================================================
void BMP_SetupBitmap( x_bitmap* Bmp,
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
                      s32       NClutColors )
{
    //
    // Check the parameters
    //
    ASSERT( Bmp         );
    ASSERT( PixelData   );
    ASSERT( Width   > 0 );
    ASSERT( Height  > 0 );
    ASSERT( PWidth  > 0 );
    ASSERT( PHeight > 0 );
    ASSERT( Format  > BMP_FMT_NONE        );
    ASSERT( Format  < BMP_FMT_NUM_FORMATS );
    
    BMP_InitInstance( Bmp );

    //
    // Setup the structure
    //
    Bmp->Format      = (u8) Format;
    Bmp->NClutColors = (s16)NClutColors;
    Bmp->W           = (s16)Width;
    Bmp->H           = (s16)Height;
    Bmp->PW          = (s16)PWidth;
    Bmp->PH          = (s16)PHeight;
    Bmp->Data        = PixelData;
    Bmp->DataSize    = DataSizeInBytes;
    Bmp->Clut        = ClutData;
    Bmp->ClutSize    = ClutSizeInBytes;
    Bmp->Pad[0]      = 
    Bmp->Pad[1]      = 
    Bmp->Pad[2]      = 
    Bmp->Pad[3]      = 0;

    Bmp->Flags = 0;
    if( IsDataOwned )  Bmp->Flags |= BMP_FLAG_DATA_OWNED;
    if( IsClutOwned )  Bmp->Flags |= BMP_FLAG_CLUT_OWNED;
}

//=========================================================================
// BMP_Copy
//=========================================================================
err BMP_Copy( x_bitmap* Dest, x_bitmap* Source )
{
    s32 Size;

    ASSERT( Dest );
    ASSERT( Source );
    ASSERT( BMP_SanityCheck( Source ) == ERR_SUCCESS );
    ASSERT( Dest != Source );

    //---------------------------------------------------------------------
    // Delete the destination picture
    //---------------------------------------------------------------------
    BMP_KillInstance( Dest );

    //---------------------------------------------------------------------
    // Copy the Headers
    //---------------------------------------------------------------------
    *Dest = *Source;
    if (Dest->ClutSize==0) Dest->Clut = NULL;

    //---------------------------------------------------------------------
    // Make sure that Dest has a Dynamic memory flag
    //---------------------------------------------------------------------
    Dest->Flags = BMP_FLAG_DATA_OWNED;
    if( Dest->Clut )  Dest->Flags |= BMP_FLAG_CLUT_OWNED;

    //---------------------------------------------------------------------
    // Create the bitmap buffer
    //---------------------------------------------------------------------
    Size       = BMP_GetDataSize( Source );
    Dest->Data = (byte*)x_malloc( Size );
    
    if( Dest->Data == NULL )
        return ERR_FAILURE;

    x_memcpy( Dest->Data, Source->Data, Size );

    //---------------------------------------------------------------------
    // Create the clut buffer
    //---------------------------------------------------------------------
    if( Source->ClutSize )
    {
        Size = BMP_GetClutSize( Source );
        Dest->Clut = (byte*)x_malloc( Size );

        if( Dest->Clut == NULL )
        {
            x_free( Dest->Data );
            BMP_InitInstance( Dest );

            return ERR_FAILURE;
        }

        x_memcpy( Dest->Clut, Source->Clut, Size );
    }

    return ERR_SUCCESS;
}

//=========================================================================
// BMP_SanityCheck
//=========================================================================
err BMP_SanityCheck( x_bitmap* Bmp )
{

    ASSERT( Bmp );

    ASSERTS( Bmp->Format < BMP_FMT_NUM_FORMATS, "BITMAP: Unknown format" );

    ASSERTS( (Bmp->Pad[0] |  
              Bmp->Pad[1] | 
              Bmp->Pad[2] | 
              Bmp->Pad[3]) == 0, "BITMAP: Not initialized or corrupted" );

    if( Bmp->Format == BMP_FMT_NONE )
        return( ERR_SUCCESS );

    ASSERTS( Bmp->DataSize == ( Bmp->PH * Bmp->PW * m_Format[ Bmp->Format ].BPP ) / 8, 
             "BITMAP: Data size inconsistant with PW, PH, and BPP" );

    ASSERTS( (Bmp->Flags & (~(BMP_FLAG_DATA_OWNED | BMP_FLAG_CLUT_OWNED)) ) == 0, 
             "BITMAP: Flags field corrupted" );

    ASSERTS( Bmp->DataSize > 0, "BITMAP: Invalid data size" );

    if ( m_Format[ Bmp->Format ].IsClutUsed )
    {
        ASSERTS( Bmp->NClutColors >    0, "BITMAP: Palette has no colors" );
        ASSERTS( Bmp->NClutColors <= 256, "BITMAP: Palette has more than 256 colors" );
        ASSERTS( Bmp->ClutSize    >=   0, "BITMAP: Clut size invalid" );
        ASSERTS( Bmp->NClutColors <= (1 << m_Format[Bmp->Format].BPP), 
                 "BITMAP: Too many colors in palette" );
    }

    ASSERTS( Bmp->W >     0, "BITMAP: Width invalid" );
    ASSERTS( Bmp->W < 32000, "BITMAP: Width invalid" );
    ASSERTS( Bmp->H >     0, "BITMAP: Height invalid" );
    ASSERTS( Bmp->H < 32000, "BITMAP: Height invalid" );

    ASSERTS( Bmp->PW >     0, "BITMAP: Physical Width invalid" );
    ASSERTS( Bmp->PW < 32000, "BITMAP: Physical Width invalid" );
    ASSERTS( Bmp->PH >     0, "BITMAP: Physical Height invalid" );
    ASSERTS( Bmp->PH < 32000, "BITMAP: Physical Height invalid" );

    ASSERTS( Bmp->W <= Bmp->PW, "BITMAP: Width > Physical Width" );
    ASSERTS( Bmp->H <= Bmp->PH, "BITMAP: Height > Physical Height" );
     
    return( ERR_SUCCESS );
}

//=========================================================================
// BMP_GetFormatInfo
//=========================================================================
void BMP_GetFormatInfo( s32 FormatID, x_bitmap_format* FormatInfo )
{
    ASSERT( FormatInfo );
    ASSERT( FormatID > BMP_FMT_NONE );
    ASSERT( FormatID < BMP_FMT_NUM_FORMATS );

    *FormatInfo = m_Format[ FormatID ];
}

//=========================================================================
// BMP_FindFormat
//=========================================================================
s32 BMP_FindFormat( x_bitmap_format* FormatInfo )
{
    s32 i;

    ASSERT( FormatInfo );

    for ( i = 0; i < BMP_FMT_NUM_FORMATS; i++ )
    {
        if( m_Format[i].IsClutUsed != FormatInfo->IsClutUsed ) continue;
        if( m_Format[i].BPP        != FormatInfo->BPP        ) continue;
        if( m_Format[i].RMask      != FormatInfo->RMask      ) continue;
        if( m_Format[i].GMask      != FormatInfo->GMask      ) continue;
        if( m_Format[i].BMask      != FormatInfo->BMask      ) continue;
        if( m_Format[i].AMask      != FormatInfo->AMask      ) continue;

        return i;
    }

    return BMP_FMT_NONE;
}

//=========================================================================
// BMP_ColorToFormat
//=========================================================================
u32 BMP_ColorToFormat( color C, s32 DstFormat )
{
    x_bitmap_format* Fmt;
    u32              R,G,B,A,U;
    
    // Check Parameters
    ASSERT( DstFormat > BMP_FMT_NONE );
    ASSERT( DstFormat < BMP_FMT_NUM_FORMATS );

    // get format info
    Fmt = &m_Format[ DstFormat ];

    // Get the individual RGBA
    COLOR_TO_RGBA( C, R, G, B, A );

    // Convert each component to the proper format
    R = (Fmt->RShift<0) ? ((R >> (-Fmt->RShift)) & Fmt->RMask ) : ((R << Fmt->RShift) & Fmt->RMask );
    G = (Fmt->GShift<0) ? ((G >> (-Fmt->GShift)) & Fmt->GMask ) : ((G << Fmt->GShift) & Fmt->GMask );
    B = (Fmt->BShift<0) ? ((B >> (-Fmt->BShift)) & Fmt->BMask ) : ((B << Fmt->BShift) & Fmt->BMask );
    A = (Fmt->AShift<0) ? ((A >> (-Fmt->AShift)) & Fmt->AMask ) : ((A << Fmt->AShift) & Fmt->AMask );
        
    // Set up the "unused" bits.  They should always be 1.
    U = ~( Fmt->RMask | Fmt->GMask | Fmt->BMask | Fmt->AMask );

    // Build the color and return
    return( R | G | B | A | U );
}

//=========================================================================
// BMP_FormatToColor
//=========================================================================
color BMP_FormatToColor( u32 C, s32 SrcFormat )
{
    x_bitmap_format* Fmt;
    u32              R,G,B,A;

    // Check Parameters
    ASSERT( SrcFormat );
    ASSERT( SrcFormat > BMP_FMT_NONE );
    ASSERT( SrcFormat < BMP_FMT_NUM_FORMATS );

    // get format info
    Fmt = &m_Format[ SrcFormat ];

    // Convert the color to RGBA components
    R = (Fmt->RShift<0) ? ((C & Fmt->RMask) << (-Fmt->RShift)) : ((C & Fmt->RMask) >> (Fmt->RShift));
    G = (Fmt->GShift<0) ? ((C & Fmt->GMask) << (-Fmt->GShift)) : ((C & Fmt->GMask) >> (Fmt->GShift));
    B = (Fmt->BShift<0) ? ((C & Fmt->BMask) << (-Fmt->BShift)) : ((C & Fmt->BMask) >> (Fmt->BShift));
    A = (Fmt->AShift<0) ? ((C & Fmt->AMask) << (-Fmt->AShift)) : ((C & Fmt->AMask) >> (Fmt->AShift));

    // Conver the conponets to the color format and return
    return RGBA_TO_COLOR(R,G,B,A);
}

//=========================================================================
// BMP_FormatToFormat
//=========================================================================
u32 BMP_FormatToFormat( u32 SrcColor, s32 SrcFormat, s32 DstFormat )
{
    color Color;
    Color = BMP_FormatToColor( SrcColor, SrcFormat );
    return BMP_ColorToFormat( Color, DstFormat );
}

//=========================================================================
// BMP_IsClutBased
//=========================================================================
xbool BMP_IsClutBased( x_bitmap* Bmp )
{
    ASSERT( Bmp );
    ASSERT( BMP_SanityCheck( Bmp ) == ERR_SUCCESS );

    return (Bmp->NClutColors > 0);
}

//=========================================================================
// BMP_IsAlpha
//=========================================================================
xbool BMP_IsAlpha( x_bitmap* Bmp )
{
    ASSERT( Bmp );
    ASSERT( BMP_SanityCheck( Bmp ) == ERR_SUCCESS );

    return m_Format[ Bmp->Format ].AMask != 0;
}

//=========================================================================
// BMP_IsClutOwned
//=========================================================================
xbool BMP_IsClutOwned( x_bitmap* Bmp )
{
    ASSERT( Bmp );
    ASSERT( BMP_SanityCheck( Bmp ) == ERR_SUCCESS );
                                                                         
    return( ( Bmp->Flags & BMP_FLAG_CLUT_OWNED ) == BMP_FLAG_CLUT_OWNED );
}

//=========================================================================
// BMP_IsDataOwned
//=========================================================================
s32 BMP_IsDataOwned( x_bitmap* Bmp )
{
    ASSERT( Bmp );
    ASSERT( BMP_SanityCheck( Bmp ) == ERR_SUCCESS );

    return ( Bmp->Flags & BMP_FLAG_DATA_OWNED ) == BMP_FLAG_DATA_OWNED;
}

//=========================================================================
// BMP_GetFormat
//=========================================================================
s32 BMP_GetFormat( x_bitmap* Bmp )
{
    ASSERT( Bmp );
    ASSERT( BMP_SanityCheck( Bmp ) == ERR_SUCCESS );

    return Bmp->Format;
}

//=========================================================================
// BMP_GetBPP
//=========================================================================
s32 BMP_GetBPP( x_bitmap* Bmp )
{
    ASSERT( Bmp );
    return m_Format[ Bmp->Format ].BPP;        
}

//=========================================================================
// BMP_GetBPC
//=========================================================================
s32 BMP_GetBPC( x_bitmap* Bmp )
{
    ASSERT( Bmp );
    ASSERT( BMP_SanityCheck( Bmp ) == ERR_SUCCESS );

    return m_Format[ Bmp->Format ].BPC;
}

//=========================================================================
// BMP_GetNClutColors
//=========================================================================
s32 BMP_GetNClutColors( x_bitmap* Bmp )
{
    ASSERT( Bmp );
    ASSERT( BMP_SanityCheck( Bmp ) == ERR_SUCCESS );

    return Bmp->NClutColors;        
}

//=========================================================================
// BMP_GetWidth
//=========================================================================
s32 BMP_GetWidth( x_bitmap* Bmp )
{
    ASSERT( Bmp );
    ASSERT( BMP_SanityCheck( Bmp ) == ERR_SUCCESS );

    return  Bmp->W;
}

//=========================================================================
// BMP_GetHeight
//=========================================================================
s32 BMP_GetHeight( x_bitmap* Bmp )
{
    ASSERT( Bmp );
    ASSERT( BMP_SanityCheck( Bmp ) == ERR_SUCCESS );

    return  Bmp->H;
}

//=========================================================================
// BMP_GetPWidth
//=========================================================================
s32 BMP_GetPWidth( x_bitmap* Bmp )
{
    ASSERT( Bmp );
    ASSERT( BMP_SanityCheck( Bmp ) == ERR_SUCCESS );

    return  Bmp->PW;
}

//=========================================================================
// BMP_GetPHeight
//=========================================================================
s32 BMP_GetPHeight( x_bitmap* Bmp )
{
    ASSERT( Bmp );
    ASSERT( BMP_SanityCheck( Bmp ) == ERR_SUCCESS );

    return  Bmp->PH;
}

//=========================================================================
// BMP_GetDataSize
//=========================================================================
s32 BMP_GetDataSize( x_bitmap* Bmp )
{
    ASSERT( Bmp );
    ASSERT( BMP_SanityCheck( Bmp ) == ERR_SUCCESS );

    return Bmp->DataSize;
}

//=========================================================================
// BMP_GetClutSize
//=========================================================================
s32 BMP_GetClutSize( x_bitmap* Bmp )
{
    ASSERT( Bmp );
    ASSERT( BMP_SanityCheck( Bmp ) == ERR_SUCCESS );

    return Bmp->ClutSize;
}

//=========================================================================
// BMP_GetDataPtr
//=========================================================================
byte* BMP_GetDataPtr( x_bitmap* Bmp )
{
    ASSERT( Bmp );
    ASSERT( BMP_SanityCheck( Bmp ) == ERR_SUCCESS );

    return Bmp->Data;
}

//=========================================================================
// BMP_GetClutPtr
//=========================================================================
byte* BMP_GetClutPtr( x_bitmap* Bmp )
{
    ASSERT( Bmp );
    ASSERT( BMP_SanityCheck( Bmp ) == ERR_SUCCESS );

    return Bmp->Clut;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// CONVERSION ROUTINES
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static s32       CR_T;
static byte*     CR_Data;
static u32       CR_C;
static s32       CW_T;
static byte*     CW_Data;
static u32       CW_C;
static s32       IR_T;
static byte*     IR_Data;
static s32       IR_I;
static s32       IW_T;
static byte*     IW_Data;
static s32       IW_I;
static u32 CR2CW_RSrcMask, CR2CW_GSrcMask, CR2CW_BSrcMask, CR2CW_ASrcMask;
static u32 CR2CW_RDstMask, CR2CW_GDstMask, CR2CW_BDstMask, CR2CW_ADstMask;
static s32 CR2CW_RSrcShift,CR2CW_GSrcShift,CR2CW_BSrcShift,CR2CW_ASrcShift;
static s32 CR2CW_RDstShift,CR2CW_GDstShift,CR2CW_BDstShift,CR2CW_ADstShift;
static void (*CR) (void);
static void (*CW) (void);
static void (*IR) (void);
static void (*IW) (void);
static x_bitmap_format SrcFormat;
static x_bitmap_format DstFormat;
static x_bitmap* SrcBitmap;
static x_bitmap* DstBitmap;


///////////////////////////////////////////////////////////////////////////

void CR_16(void)
{
    CR_C = (u32)((u16*)CR_Data)[CR_T];
}

///////////////////////////////////////////////////////////////////////////

void CR_32(void)
{
    CR_C = (u32)((u32*)CR_Data)[CR_T];
}

///////////////////////////////////////////////////////////////////////////

void CR_24(void)
{
    CR_C = (((u32)CR_Data[(CR_T*3)+0])<<16)|
           (((u32)CR_Data[(CR_T*3)+1])<<8) |
           (((u32)CR_Data[(CR_T*3)+2])) ;
}

///////////////////////////////////////////////////////////////////////////

void CW_16(void)
{
    ((u16*)CW_Data)[CW_T] = (u16)CW_C;
}

///////////////////////////////////////////////////////////////////////////

void CW_32(void)
{
    ((u32*)CW_Data)[CW_T] = (u32)(CW_C);
}

///////////////////////////////////////////////////////////////////////////

void CW_24(void)
{
    CW_Data[(CW_T*3)+0] = (byte)((CW_C & 0x00FF0000)>>16);
    CW_Data[(CW_T*3)+1] = (byte)((CW_C & 0x0000FF00)>>8);
    CW_Data[(CW_T*3)+2] = (byte)((CW_C & 0x000000FF));
}

///////////////////////////////////////////////////////////////////////////

void IR_8(void)
{
    IR_I = IR_Data[IR_T];
    CR_T = IR_I;
}

///////////////////////////////////////////////////////////////////////////

void IR_4(void)
{
    IR_I = (s32)((IR_T&0x01)?(IR_Data[IR_T>>1]&0x0F):(IR_Data[IR_T>>1]>>4));
    CR_T = IR_I;
}

///////////////////////////////////////////////////////////////////////////

void IR_16(void)
{
    IR_I = IR_T;
    CR_T = IR_I;
}

///////////////////////////////////////////////////////////////////////////

void IR_24(void)
{
    IR_I = IR_T;
    CR_T = IR_I;
}

///////////////////////////////////////////////////////////////////////////

void IR_32(void)
{
    IR_I = IR_T;
    CR_T = IR_I;
}

///////////////////////////////////////////////////////////////////////////

void IW_8(void)
{
    IW_Data[IW_T] = (byte)IW_I;
}

///////////////////////////////////////////////////////////////////////////

void IW_4(void)
{
    if (IW_T & 0x01)
        IW_Data[IW_T>>1] = (IW_Data[IW_T>>1] & 0xF0) | ((IW_I&0x0F)<<0);
    else
        IW_Data[IW_T>>1] = (IW_Data[IW_T>>1] & 0x0F) | ((IW_I&0x0F)<<4);
}

///////////////////////////////////////////////////////////////////////////

void BMP_SetPixelFormat      ( x_bitmap* DstBMP, x_bitmap* SrcBMP )
{
    x_bitmap_format FDst;
    x_bitmap_format FSrc;

    // Clear functions, data ptrs, and counters
    CR      = CW      = IR      = IW      = NULL;
    CR_Data = CW_Data = IR_Data = IW_Data = NULL;
    CR_T    = CW_T    = IR_T    = IW_T    = 0;
    SrcBitmap = DstBitmap = NULL;

    // Check if we need to setup Dst
    if (DstBMP)
    {
        ASSERT(DstBMP);
        ASSERT(!BMP_SanityCheck(DstBMP));

        // Dst bitmaps
        DstBitmap = DstBMP;

        // Src format info
        BMP_GetFormatInfo( DstBMP->Format, &FDst );
        DstFormat = FDst;

        // Setup data and index ptrs
        if (FDst.IsClutUsed) CW_Data = DstBMP->Clut;
        else                 CW_Data = DstBMP->Data;
        if (FDst.IsClutUsed) IW_Data = DstBMP->Data;
        else                 IW_Data = NULL;

        // Setup CW
        if (FDst.BPC==16) CW = CW_16; else
        if (FDst.BPC==24) CW = CW_24; else
        if (FDst.BPC==32) CW = CW_32;

        // Setup IW
        if (FDst.BPP== 4) IW = IW_4;  else
        if (FDst.BPP== 8) IW = IW_8;

        // Dst masks and shifts
        CR2CW_RDstMask  = FDst.RMask;
        CR2CW_GDstMask  = FDst.GMask;
        CR2CW_BDstMask  = FDst.BMask;
        CR2CW_ADstMask  = FDst.AMask;
        CR2CW_RDstShift = FDst.RShift;
        CR2CW_GDstShift = FDst.GShift;
        CR2CW_BDstShift = FDst.BShift;
        CR2CW_ADstShift = FDst.AShift;
    }

    // Check if we need to setup Src
    if (SrcBMP)
    {
        ASSERT(SrcBMP);
        ASSERT(!BMP_SanityCheck(SrcBMP));

        // Dst bitmaps
        SrcBitmap = SrcBMP;

        // Src format info
        BMP_GetFormatInfo( SrcBMP->Format, &FSrc );
        SrcFormat = FSrc;

        // Setup data and index ptrs
        if (FSrc.IsClutUsed) CR_Data = SrcBMP->Clut;
        else                 CR_Data = SrcBMP->Data;
        if (FSrc.IsClutUsed) IR_Data = SrcBMP->Data;
        else                 IR_Data = NULL;

        // Setup CR
        if (FSrc.BPC==16) CR = CR_16; else
        if (FSrc.BPC==24) CR = CR_24; else
        if (FSrc.BPC==32) CR = CR_32;

        // Setup IR
        if (FSrc.BPP== 4) IR = IR_4;  else
        if (FSrc.BPP== 8) IR = IR_8;  else
        if (FSrc.BPP==16) IR = IR_16; else
        if (FSrc.BPP==24) IR = IR_24; else
        if (FSrc.BPP==32) IR = IR_32; 

        // Dst masks and shifts
        CR2CW_RSrcMask  = FSrc.RMask; 
        CR2CW_GSrcMask  = FSrc.GMask; 
        CR2CW_BSrcMask  = FSrc.BMask; 
        CR2CW_ASrcMask  = FSrc.AMask; 
        CR2CW_RSrcShift = FSrc.RShift;
        CR2CW_GSrcShift = FSrc.GShift;
        CR2CW_BSrcShift = FSrc.BShift;
        CR2CW_ASrcShift = FSrc.AShift;
    }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// MAGIC PIXEL
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

s32 BMP_GetPixelIndex( s32 X, s32 Y )
{
    ASSERT(SrcBitmap);
    ASSERT(SrcFormat.IsClutUsed);
    ASSERT((X>=0) && (X<SrcBitmap->W));
    ASSERT((Y>=0) && (Y<SrcBitmap->H));

    // Compute pixel index
    IR_T = X + (Y*SrcBitmap->PW);

    // Call read function and return value
    IR();
    return IR_I;
}

///////////////////////////////////////////////////////////////////////////

color BMP_GetClutColor( s32 Index )
{
    u32 R, G, B, A;

    ASSERT(SrcBitmap);
    ASSERT(SrcFormat.IsClutUsed);
    ASSERT((Index>=0) && (Index<SrcBitmap->NClutColors));

    CR_T = Index;
    CR();

    R = (CR2CW_RSrcShift<0) ? ((CR_C & CR2CW_RSrcMask) << (-CR2CW_RSrcShift)) : ((CR_C & CR2CW_RSrcMask) >> (CR2CW_RSrcShift));
    G = (CR2CW_GSrcShift<0) ? ((CR_C & CR2CW_GSrcMask) << (-CR2CW_GSrcShift)) : ((CR_C & CR2CW_GSrcMask) >> (CR2CW_GSrcShift));
    B = (CR2CW_BSrcShift<0) ? ((CR_C & CR2CW_BSrcMask) << (-CR2CW_BSrcShift)) : ((CR_C & CR2CW_BSrcMask) >> (CR2CW_BSrcShift));
    A = (CR2CW_ASrcShift<0) ? ((CR_C & CR2CW_ASrcMask) << (-CR2CW_ASrcShift)) : ((CR_C & CR2CW_ASrcMask) >> (CR2CW_ASrcShift));
    if (CR2CW_ASrcMask==0) A = 255;

    return RGBA_TO_COLOR(R,G,B,A);
}

///////////////////////////////////////////////////////////////////////////

color BMP_GetPixelColor( s32 X, s32 Y )
{
    u32 R, G, B, A;

    ASSERT(SrcBitmap);

    ASSERT((X>=0) && (X<SrcBitmap->W));
    ASSERT((Y>=0) && (Y<SrcBitmap->H));

    IR_T = X + (Y*SrcBitmap->PW);

    IR();
    CR();

    R = (CR2CW_RSrcShift<0) ? ((CR_C & CR2CW_RSrcMask) << (-CR2CW_RSrcShift)) : ((CR_C & CR2CW_RSrcMask) >> (CR2CW_RSrcShift));
    G = (CR2CW_GSrcShift<0) ? ((CR_C & CR2CW_GSrcMask) << (-CR2CW_GSrcShift)) : ((CR_C & CR2CW_GSrcMask) >> (CR2CW_GSrcShift));
    B = (CR2CW_BSrcShift<0) ? ((CR_C & CR2CW_BSrcMask) << (-CR2CW_BSrcShift)) : ((CR_C & CR2CW_BSrcMask) >> (CR2CW_BSrcShift));
    A = (CR2CW_ASrcShift<0) ? ((CR_C & CR2CW_ASrcMask) << (-CR2CW_ASrcShift)) : ((CR_C & CR2CW_ASrcMask) >> (CR2CW_ASrcShift));
    if (CR2CW_ASrcMask==0) A = 255;

    return RGBA_TO_COLOR(R,G,B,A);
}

///////////////////////////////////////////////////////////////////////////

void BMP_SetPixelColor( s32 X, s32 Y, color C )
{
    u32 R, G, B, A, U;

    ASSERT(DstBitmap);
    ASSERT((X>=0) && (X<DstBitmap->W));
    ASSERT((Y>=0) && (Y<DstBitmap->H));
    ASSERT(!DstFormat.IsClutUsed);

    // Build destination color
    COLOR_TO_RGBA( C, R, G, B, A );

    R = (CR2CW_RDstShift<0) ? ((R >> (-CR2CW_RDstShift)) & CR2CW_RDstMask) : ((R << CR2CW_RDstShift) & CR2CW_RDstMask);
    G = (CR2CW_GDstShift<0) ? ((G >> (-CR2CW_GDstShift)) & CR2CW_GDstMask) : ((G << CR2CW_GDstShift) & CR2CW_GDstMask);
    B = (CR2CW_BDstShift<0) ? ((B >> (-CR2CW_BDstShift)) & CR2CW_BDstMask) : ((B << CR2CW_BDstShift) & CR2CW_BDstMask);
    A = (CR2CW_ADstShift<0) ? ((A >> (-CR2CW_ADstShift)) & CR2CW_ADstMask) : ((A << CR2CW_ADstShift) & CR2CW_ADstMask);
    
    // Set up the "unused" bits.  They should always be 1.
    U = ~( CR2CW_RDstMask | CR2CW_GDstMask | CR2CW_BDstMask | CR2CW_ADstMask );

    // Compute pixel index
    CW_T = X + (Y*DstBitmap->PW);
    CW_C = R | G | B | A | U;
    CW();
}

///////////////////////////////////////////////////////////////////////////

void BMP_SetPixelIndex( s32 X, s32 Y, s32 Index )
{
    ASSERT(DstBitmap);
    ASSERT((X>=0) && (X<DstBitmap->W));
    ASSERT((Y>=0) && (Y<DstBitmap->H));
    ASSERT((Index>=0) && (Index<DstBitmap->NClutColors));
    ASSERT(DstFormat.IsClutUsed);

    // Compute pixel index
    IW_T = X + (Y*DstBitmap->PW);
    IW_I = Index;
    IW();
}

///////////////////////////////////////////////////////////////////////////

void BMP_SetClutColor( s32 Index, color C )
{
    u32 R, G, B, A, U;

    ASSERT(DstBitmap);
    ASSERT((Index>=0) && (Index<DstBitmap->NClutColors));
    ASSERT(DstFormat.IsClutUsed);

    // Build destination color
    COLOR_TO_RGBA( C, R, G, B, A );

    R = (CR2CW_RDstShift<0) ? ((R >> (-CR2CW_RDstShift)) & CR2CW_RDstMask) : ((R << CR2CW_RDstShift) & CR2CW_RDstMask);
    G = (CR2CW_GDstShift<0) ? ((G >> (-CR2CW_GDstShift)) & CR2CW_GDstMask) : ((G << CR2CW_GDstShift) & CR2CW_GDstMask);
    B = (CR2CW_BDstShift<0) ? ((B >> (-CR2CW_BDstShift)) & CR2CW_BDstMask) : ((B << CR2CW_BDstShift) & CR2CW_BDstMask);
    A = (CR2CW_ADstShift<0) ? ((A >> (-CR2CW_ADstShift)) & CR2CW_ADstMask) : ((A << CR2CW_ADstShift) & CR2CW_ADstMask);

    // Set up the "unused" bits.  They should always be 1.
    U = ~( CR2CW_RDstMask | CR2CW_GDstMask | CR2CW_BDstMask | CR2CW_ADstMask );

    // Compute pixel index
    CW_T = Index;
    CW_C = R | G | B | A | U;
    CW();
}

///////////////////////////////////////////////////////////////////////////
