#include "x_bitmap.hpp"
#include "x_debug.hpp"
#include "x_memory.hpp"
#include "x_plus.hpp"
#include "x_color.hpp"
#include "x_math.hpp"
#include "x_stdio.hpp"
#include "x_time.hpp"


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

//--- Macros used when we have alpha. for PSX
#define SET_BGRA(A,B,G,R) M_ARGBM( A,0,             R,(A),           G,(R)+(A),   B,(G)+(R)+(A) )

//--- Macro used when we don't have alpha. for PSX
#define SET_BGRU(A,B,G,R) M_ARGBM( 0,0,             R,(A),           G,(R)+(A),   B,(G)+(R)+(A) )

///////////////////////////////////////////////////////////////////////////
//  DEFINES
///////////////////////////////////////////////////////////////////////////

#define ALL_FLAGS ( FLAG_DATA_OWNED | FLAG_CLUT_OWNED | FLAG_CLUT_SWIZZLED | FLAG_DATA_SWIZZLED |\
                    FLAG_REDUCED_INTENSITY | FLAG_COMPRESSED_S3TC | FLAG_XBITMAP_IS_VALID )

///////////////////////////////////////////////////////////////////////////
// VARIABLES
///////////////////////////////////////////////////////////////////////////

const x_bitmap::fmt_desc x_bitmap::s_Format[] = 
{
    // FORMAT              CLUT? BPP BPC  COLOR INFO
    { FMT_NULL           , FALSE,  0,  0, SET_ARGB(0,0,0,0) },

    { FMT_P4_RGB_565     ,  TRUE,  4, 16, SET_RGBU(5,6,5,0) },
    { FMT_P4_RGB_888     ,  TRUE,  4, 24, SET_RGBU(8,8,8,0) },
    { FMT_P4_URGB_8888   ,  TRUE,  4, 32, SET_URGB(8,8,8,8) },
    { FMT_P4_RGBU_8888   ,  TRUE,  4, 32, SET_RGBU(8,8,8,8) },
    { FMT_P4_ARGB_8888   ,  TRUE,  4, 32, SET_ARGB(8,8,8,8) },
    { FMT_P4_RGBA_8888   ,  TRUE,  4, 32, SET_RGBA(8,8,8,8) },
    { FMT_P4_ABGR_8888   ,  TRUE,  4, 32, SET_ABGR(8,8,8,8) },  // NEW
    { FMT_P4_UBGR_8888   ,  TRUE,  4, 32, SET_UBGR(8,8,8,8) },  // NEW
    { FMT_P4_RGBA_5551   ,  TRUE,  4, 16, SET_RGBA(5,5,5,1) },
    { FMT_P4_ARGB_1555   ,  TRUE,  4, 16, SET_ARGB(1,5,5,5) },                                       
    { FMT_P4_RGBU_5551   ,  TRUE,  4, 16, SET_RGBU(5,5,5,1) },
    { FMT_P4_URGB_1555   ,  TRUE,  4, 16, SET_URGB(1,5,5,5) },
    { FMT_P4_ABGR_1555   ,  TRUE,  4, 16, SET_ABGR(1,5,5,5) },  // NEW
    { FMT_P4_UBGR_1555   ,  TRUE,  4, 16, SET_UBGR(1,5,5,5) },  // NEW

    { FMT_P8_RGB_565     ,  TRUE,  8, 16, SET_RGBU(5,6,5,0) },
    { FMT_P8_RGB_888     ,  TRUE,  8, 24, SET_RGBU(8,8,8,0) },
    { FMT_P8_URGB_8888   ,  TRUE,  8, 32, SET_URGB(8,8,8,8) },
    { FMT_P8_RGBU_8888   ,  TRUE,  8, 32, SET_RGBU(8,8,8,8) },
    { FMT_P8_ARGB_8888   ,  TRUE,  8, 32, SET_ARGB(8,8,8,8) },
    { FMT_P8_RGBA_8888   ,  TRUE,  8, 32, SET_RGBA(8,8,8,8) },
    { FMT_P8_ABGR_8888   ,  TRUE,  8, 32, SET_ABGR(8,8,8,8) },  // NEW
    { FMT_P8_UBGR_8888   ,  TRUE,  8, 32, SET_UBGR(8,8,8,8) },  // NEW
    { FMT_P8_RGBA_5551   ,  TRUE,  8, 16, SET_RGBA(5,5,5,1) },
    { FMT_P8_ARGB_1555   ,  TRUE,  8, 16, SET_ARGB(1,5,5,5) },
    { FMT_P8_RGBU_5551   ,  TRUE,  8, 16, SET_RGBU(5,5,5,1) },
    { FMT_P8_URGB_1555   ,  TRUE,  8, 16, SET_URGB(1,5,5,5) }, 
    { FMT_P8_ABGR_1555   ,  TRUE,  8, 16, SET_ABGR(1,5,5,5) },  // NEW
    { FMT_P8_UBGR_1555   ,  TRUE,  8, 16, SET_UBGR(1,5,5,5) },  // NEW
    
    { FMT_16_RGB_565     , FALSE, 16, 16, SET_RGBU(5,6,5,0) },
    { FMT_16_ARGB_4444   , FALSE, 16, 16, SET_ARGB(4,4,4,4) },
    { FMT_16_RGBA_4444   , FALSE, 16, 16, SET_RGBA(4,4,4,4) },
    { FMT_16_ARGB_1555   , FALSE, 16, 16, SET_ARGB(1,5,5,5) },
    { FMT_16_RGBA_5551   , FALSE, 16, 16, SET_RGBA(5,5,5,1) },
    { FMT_16_URGB_1555   , FALSE, 16, 16, SET_URGB(1,5,5,5) },
    { FMT_16_RGBU_5551   , FALSE, 16, 16, SET_RGBU(5,5,5,1) },
    { FMT_16_ABGR_1555   , FALSE, 16, 16, SET_ABGR(1,5,5,5) },  // NEW
    { FMT_16_UBGR_1555   , FALSE, 16, 16, SET_UBGR(1,5,5,5) },  // NEW
                           
    { FMT_24_RGB_888     , FALSE, 24, 24, SET_RGBU(8,8,8,0) },
    { FMT_24_ARGB_8565   , FALSE, 24, 24, SET_ARGB(8,5,6,5) },     
                           
    { FMT_32_RGBU_8888   , FALSE, 32, 32, SET_RGBU(8,8,8,8) },
    { FMT_32_URGB_8888   , FALSE, 32, 32, SET_URGB(8,8,8,8) },
    { FMT_32_ARGB_8888   , FALSE, 32, 32, SET_ARGB(8,8,8,8) },
    { FMT_32_RGBA_8888   , FALSE, 32, 32, SET_RGBA(8,8,8,8) },      
    { FMT_32_ABGR_8888   , FALSE, 32, 32, SET_ABGR(8,8,8,8) },  // NEW
    { FMT_32_BGRA_8888   , FALSE, 32, 32, SET_BGRA(8,8,8,8) },  // NEW               
                           
    { FMT_I4             ,  TRUE,  4,  4, SET_RGBU(4,0,0,0) },
    { FMT_I8             ,  TRUE,  8,  8, SET_RGBU(8,0,0,0) },    
    
};

///////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////

x_bitmap::x_bitmap( const x_bitmap& Src )
{
	Reset();
    CopyFrom( Src );
}

//=========================================================================

x_bitmap::~x_bitmap()
{
    if( m_Flags & FLAG_DATA_OWNED )
    {
        ASSERT( m_pData );
        x_free( m_pData );
    }

    if( m_Flags & FLAG_CLUT_OWNED )
    {
        if( m_ClutSize > 0 )
        {                    
            ASSERT( m_pClut );
            x_free( m_pClut );
        }
    }

    // Clean up and be safe
    Reset();
}

//=========================================================================

void x_bitmap::operator=( const x_bitmap& Src )        // Assignment
{
    CopyFrom( Src );    
}


//=========================================================================

void x_bitmap::SetupBitmap( format    Format,
                            s32       Width,
                            s32       Height,
                            s32       PWidth,
                            s32       PHeight,
                            xbool     IsDataOwned,
                            s32       DataSizeInBytes,
                            void*     pPixelData,
                            xbool     IsClutOwned,
                            s32       ClutSizeInBytes,
                            void*     pClutData,
                            s32       NClutColors,
                            s32       NMipsToBuild)
{
    //
    // Check the parameters
    //
    ASSERT( pPixelData   );
    ASSERT( Width   > 0 );
    ASSERT( Height  > 0 );
    ASSERT( PWidth  > 0 );
    ASSERT( PHeight > 0 );
    ASSERT( Format  > FMT_NULL  );
    ASSERT( Format  < FMT_END   );
    ASSERT( NClutColors <= 256 );
    
    //
    // Setup the structure
    //
    m_Format      = (s8)Format;

    if (pClutData)
        m_MaxClutID = NClutColors-1;
    else
        m_MaxClutID = 0;

    m_Width       = (s16)Width;
    m_Height      = (s16)Height;
    m_PW          = (s16)PWidth;
    m_PH          = (s16)PHeight;
    m_pData       = (byte*)pPixelData;
    m_DataSize    = DataSizeInBytes;
    m_pClut       = (byte*)pClutData;
    m_ClutSize    = ClutSizeInBytes;
    m_NCluts      = 1;


    if( IsDataOwned )  m_Flags |= FLAG_DATA_OWNED;
    if( IsClutOwned )  m_Flags |= FLAG_CLUT_OWNED;

    m_Flags |= FLAG_XBITMAP_IS_VALID;

    m_NMips = 0;//NMipsToBuild;

    //
    //  Only build mips if they are requested, otherwise, just use the normal
    //  member functions to keep the memory overhead lower.
    //
    //  Make a simple test here to save ourselves some work
    if ( (m_Width <= 8) || (m_Height <= 8) )
        NMipsToBuild = 0;

    if (NMipsToBuild)
        BuildMips( NMipsToBuild );
}

//=========================================================================

xbool x_bitmap::SanityCheck( void ) const
{ 
    ASSERTS( m_Format < FMT_END, "BITMAP: Unknown format" );

    if( m_Format == FMT_NULL )
        return TRUE;

    // If the data pointer is set, check the valid flag
    if (m_pData)
        ASSERTS( m_Flags & FLAG_XBITMAP_IS_VALID, "BITMAP: Not constructed through proper methods" );

// Cannot do test because of mipmaps
//    ASSERTS( m_DataSize == ( m_PH * m_PW * s_Format[ m_Format ].BPP ) / 8, 
//             "BITMAP: Data size inconsistant with PW, PH, and BPP" );

    ASSERTS( (m_Flags & (~(ALL_FLAGS)) ) == 0, 
             "BITMAP: Flags field corrupted" );

    ASSERTS( m_DataSize > 0, "BITMAP: Invalid data size" );

    if ( s_Format[ m_Format ].IsClutUsed )
    {
        //ASSERTS( m_MaxClutID   >    0, "BITMAP: Palette has no colors" );
        ASSERTS( m_ClutSize    >=   0, "BITMAP: Clut size invalid" );
        ASSERTS( m_MaxClutID   <   (1 << s_Format[m_Format].BPP), 
                 "BITMAP: Too many colors in palette" );
        ASSERTS( m_NCluts      >    0, "BITMAP: No cluts exist for a palettized bitmap" );
    }

    ASSERTS( m_Width  >     0, "BITMAP: Width invalid" );
    ASSERTS( m_Width  < 32000, "BITMAP: Width invalid" );
    ASSERTS( m_Height >     0, "BITMAP: Height invalid" );
    ASSERTS( m_Height < 32000, "BITMAP: Height invalid" );

    ASSERTS( m_PW >     0, "BITMAP: Physical Width invalid" );
    ASSERTS( m_PW < 32000, "BITMAP: Physical Width invalid" );
    ASSERTS( m_PH >     0, "BITMAP: Physical Height invalid" );
    ASSERTS( m_PH < 32000, "BITMAP: Physical Height invalid" );

    ASSERTS( m_Width  <= m_PW, "BITMAP: Width > Physical Width" );
    ASSERTS( m_Height <= m_PH, "BITMAP: Height > Physical Height" );

    return TRUE;
}

//=========================================================================

x_bitmap::format 
x_bitmap::FindFormat( fmt_desc& FormatDesc )
{
    s32 i;

    for( i = FMT_NULL; i < FMT_END; i++ )
    {
        if( s_Format[i].IsClutUsed != FormatDesc.IsClutUsed ) continue;
        if( s_Format[i].BPP        != FormatDesc.BPP        ) continue;
        if( s_Format[i].RMask      != FormatDesc.RMask      ) continue;
        if( s_Format[i].GMask      != FormatDesc.GMask      ) continue;
        if( s_Format[i].BMask      != FormatDesc.BMask      ) continue;
        if( s_Format[i].AMask      != FormatDesc.AMask      ) continue;

        return (format)i;
    }

    return FMT_NULL;
}

//=========================================================================

u32 x_bitmap::ColorToFormat( color C, format DstFormat )
{
    const fmt_desc* Fmt;
    u32             R,G,B,A,U;
    
    // Check Parameters
    ASSERT( DstFormat > FMT_NULL );
    ASSERT( DstFormat < FMT_END  );

    // get format info
    Fmt = &s_Format[ DstFormat ];

    // Get the individual RGBA
    R = C.R;
    G = C.G;
    B = C.B;
    A = C.A;

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

color x_bitmap::FormatToColor( u32 C, format SrcFormat )
{
    const fmt_desc* Fmt;
    color           Color;

    // Check Parameters
    ASSERT( SrcFormat );
    ASSERT( SrcFormat > FMT_NULL );
    ASSERT( SrcFormat < FMT_END );

    // get format info
    Fmt = &s_Format[ SrcFormat ];

    // Convert the color to RGBA components
    Color.R = (u8)((Fmt->RShift<0) ? ((C & Fmt->RMask) << (-Fmt->RShift)) : ((C & Fmt->RMask) >> (Fmt->RShift)));
    Color.G = (u8)((Fmt->GShift<0) ? ((C & Fmt->GMask) << (-Fmt->GShift)) : ((C & Fmt->GMask) >> (Fmt->GShift)));
    Color.B = (u8)((Fmt->BShift<0) ? ((C & Fmt->BMask) << (-Fmt->BShift)) : ((C & Fmt->BMask) >> (Fmt->BShift)));
    Color.A = (u8)((Fmt->AShift<0) ? ((C & Fmt->AMask) << (-Fmt->AShift)) : ((C & Fmt->AMask) >> (Fmt->AShift)));

    // Conver the conponets to the color format and return
    return Color;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// CONVERSION ROUTINES
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static s32                  CR_T;
static byte*                CR_Data;
static u32                  CR_C;
static s32                  CW_T;
static byte*                CW_Data;
static u32                  CW_C;
static s32                  IR_T;
static byte*                IR_Data;
static s32                  IR_I;
static s32                  IW_T;
static byte*                IW_Data;
static s32                  IW_I;
static u32                  CR2CW_RSrcMask, CR2CW_GSrcMask, CR2CW_BSrcMask, CR2CW_ASrcMask;
static u32                  CR2CW_RDstMask, CR2CW_GDstMask, CR2CW_BDstMask, CR2CW_ADstMask;
static s32                  CR2CW_RSrcShift,CR2CW_GSrcShift,CR2CW_BSrcShift,CR2CW_ASrcShift;
static s32                  CR2CW_RDstShift,CR2CW_GDstShift,CR2CW_BDstShift,CR2CW_ADstShift;
static void (*CR) (void);
static void (*CW) (void);
static void (*IR) (void);
static void (*IW) (void);
static x_bitmap::fmt_desc   SrcFormat;
static x_bitmap::fmt_desc   DstFormat;
static x_bitmap*            SrcBitmap = NULL;
static x_bitmap*            DstBitmap = NULL;


///////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////

//=========================================================================

static
void CR_16( void )
{
    CR_C = (u32)((u16*)CR_Data)[CR_T];
}

//=========================================================================

static
void CR_32( void )
{
    CR_C = (u32)((u32*)CR_Data)[CR_T];
}

//=========================================================================

static
void CR_24( void )
{
    CR_C = (((u32)CR_Data[(CR_T*3)+0])<<16)|
           (((u32)CR_Data[(CR_T*3)+1])<<8) |
           (((u32)CR_Data[(CR_T*3)+2])) ;
}

//=========================================================================

static
void CW_16( void )
{
    ((u16*)CW_Data)[CW_T] = (u16)CW_C;
}

//=========================================================================

static
void CW_32( void )
{
    ((u32*)CW_Data)[CW_T] = (u32)(CW_C);
}

//=========================================================================

static
void CW_24( void )
{
    CW_Data[(CW_T*3)+0] = (byte)((CW_C & 0x00FF0000)>>16);
    CW_Data[(CW_T*3)+1] = (byte)((CW_C & 0x0000FF00)>>8);
    CW_Data[(CW_T*3)+2] = (byte)((CW_C & 0x000000FF));
}

//=========================================================================

static
void IR_8( void )
{
    IR_I = IR_Data[IR_T];
    CR_T = IR_I;
}

//=========================================================================

static
void IR_4( void )
{
    IR_I = (s32)((IR_T&0x01)?(IR_Data[IR_T>>1]&0x0F):(IR_Data[IR_T>>1]>>4));
    CR_T = IR_I;
}

//=========================================================================

static
void IR_16( void )
{
    IR_I = IR_T;
    CR_T = IR_I;
}

//=========================================================================

static
void IR_24( void )
{
    IR_I = IR_T;
    CR_T = IR_I;
}

//=========================================================================

static
void IR_32( void )
{
    IR_I = IR_T;
    CR_T = IR_I;
}

//=========================================================================

static
void IW_8( void )
{
    IW_Data[IW_T] = (byte)IW_I;
}

//=========================================================================

static
void IW_4( void )
{
    if (IW_T & 0x01)
        IW_Data[IW_T>>1] = (IW_Data[IW_T>>1] & 0xF0) | ((IW_I&0x0F)<<0);
    else
        IW_Data[IW_T>>1] = (IW_Data[IW_T>>1] & 0x0F) | ((IW_I&0x0F)<<4);
}

//=========================================================================

void x_bitmap::SetPixelFormat( x_bitmap& DstBMP,    x_bitmap& SrcBMP, 
                               s8        aDstClut,  s8        aSrcClut,
							   s8		 nMipID )
{
    fmt_desc FDst;
    fmt_desc FSrc;

    // Clear functions, data ptrs, and counters
    CR      = CW      = IR      = IW      = NULL;
    CR_Data = CW_Data = IR_Data = IW_Data = NULL;
    CR_T    = CW_T    = IR_T    = IW_T    = 0;
    SrcBitmap = DstBitmap = NULL;

    // Check if we need to setup Dst
    {
        ASSERT( DstBMP.SanityCheck() );

        // Dst bitmaps
        DstBitmap = &DstBMP;

        // Dest format info
        x_bitmap::GetFormatDesc( FDst, DstBMP.GetFormat() );
        DstFormat = FDst;

        // Setup data and index ptrs
        if (FDst.IsClutUsed) CW_Data = (byte*)DstBMP.GetClutPtr(aDstClut);
        else                 CW_Data = (byte*)DstBMP.GetDataPtr(nMipID);
        if (FDst.IsClutUsed) IW_Data = (byte*)DstBMP.GetDataPtr(nMipID);
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
    {
        ASSERT( SrcBMP.SanityCheck() );

        // Dst bitmaps
        SrcBitmap = &SrcBMP;

        // Src format info
        x_bitmap::GetFormatDesc( FSrc, SrcBMP.GetFormat() );
        SrcFormat = FSrc;

        // Setup data and index ptrs
        if (FSrc.IsClutUsed) CR_Data = (byte*)SrcBMP.GetClutPtr(aSrcClut);
        else                 CR_Data = (byte*)SrcBMP.GetDataPtr(nMipID);
        if (FSrc.IsClutUsed) IR_Data = (byte*)SrcBMP.GetDataPtr(nMipID);
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

//=========================================================================

s32 x_bitmap::GetPixelIndex( s32 X, s32 Y, s32 nMipID )
{
    ASSERT(SrcBitmap);
    ASSERT(SrcFormat.IsClutUsed);
    ASSERT((X>=0) && (X<SrcBitmap->GetWidth()));
    ASSERT((Y>=0) && (Y<SrcBitmap->GetHeight()));

    // Compute pixel index
    IR_T = X + (Y*SrcBitmap->GetPWidth());

    // Call read function and return value
    IR();
    return IR_I;
}

//=========================================================================

color x_bitmap::GetClutColor( s32 Index )
{
    color Color;

    ASSERT( SrcBitmap );
    ASSERT( SrcFormat.IsClutUsed );
    ASSERT( (Index>=0) && (Index<=SrcBitmap->GetNClutColors()) );

	if ( SrcBitmap->IsClutSwizzled() )
	{
		if ( ((Index & 0x1f) >= 8) && ((Index & 0x1f) < 16) )
		{
			Index += 8;
		}
		else if ( ((Index & 0x1f) >= 16) && ((Index & 0x1f) < 24) )
		{
			Index -= 8;
		}
	}

    CR_T = Index;
    CR();

    Color.R = (u8)((CR2CW_RSrcShift<0) ? ((CR_C & CR2CW_RSrcMask) << (-CR2CW_RSrcShift)) : ((CR_C & CR2CW_RSrcMask) >> (CR2CW_RSrcShift)));
    Color.G = (u8)((CR2CW_GSrcShift<0) ? ((CR_C & CR2CW_GSrcMask) << (-CR2CW_GSrcShift)) : ((CR_C & CR2CW_GSrcMask) >> (CR2CW_GSrcShift)));
    Color.B = (u8)((CR2CW_BSrcShift<0) ? ((CR_C & CR2CW_BSrcMask) << (-CR2CW_BSrcShift)) : ((CR_C & CR2CW_BSrcMask) >> (CR2CW_BSrcShift)));
    Color.A = (u8)((CR2CW_ASrcShift<0) ? ((CR_C & CR2CW_ASrcMask) << (-CR2CW_ASrcShift)) : ((CR_C & CR2CW_ASrcMask) >> (CR2CW_ASrcShift)));
    if( CR2CW_ASrcMask == 0 ) Color.A = 255;

    return Color;
}

//=========================================================================

color x_bitmap::GetPixelColor( s32 X, s32 Y, s32 nMipID )
{
    color Color;

    ASSERT(SrcBitmap);
    ASSERT((X>=0) && (X<SrcBitmap->GetWidth()));
    ASSERT((Y>=0) && (Y<SrcBitmap->GetHeight()));


    IR_T = X + (Y*SrcBitmap->GetPWidth(nMipID));

    IR();
    CR();

    Color.R = (u8)((CR2CW_RSrcShift<0) ? ((CR_C & CR2CW_RSrcMask) << (-CR2CW_RSrcShift)) : ((CR_C & CR2CW_RSrcMask) >> (CR2CW_RSrcShift)));
    Color.G = (u8)((CR2CW_GSrcShift<0) ? ((CR_C & CR2CW_GSrcMask) << (-CR2CW_GSrcShift)) : ((CR_C & CR2CW_GSrcMask) >> (CR2CW_GSrcShift)));
    Color.B = (u8)((CR2CW_BSrcShift<0) ? ((CR_C & CR2CW_BSrcMask) << (-CR2CW_BSrcShift)) : ((CR_C & CR2CW_BSrcMask) >> (CR2CW_BSrcShift)));
    Color.A = (u8)((CR2CW_ASrcShift<0) ? ((CR_C & CR2CW_ASrcMask) << (-CR2CW_ASrcShift)) : ((CR_C & CR2CW_ASrcMask) >> (CR2CW_ASrcShift)));
    if (CR2CW_ASrcMask==0) Color.A = 255;

    return Color;
}

//=========================================================================

void x_bitmap::SetPixelColor( s32 X, s32 Y, color C, s32 nMipID )
{
    u32 R, G, B, A, U;

    ASSERT(DstBitmap);
    ASSERT((X>=0) && (X<DstBitmap->GetWidth()));
    ASSERT((Y>=0) && (Y<DstBitmap->GetHeight()));
    ASSERT(!DstFormat.IsClutUsed);

    // Build destination color
    R = C.R;
    G = C.G;
    B = C.B;
    A = C.A;

    R = (CR2CW_RDstShift<0) ? ((R >> (-CR2CW_RDstShift)) & CR2CW_RDstMask) : ((R << CR2CW_RDstShift) & CR2CW_RDstMask);
    G = (CR2CW_GDstShift<0) ? ((G >> (-CR2CW_GDstShift)) & CR2CW_GDstMask) : ((G << CR2CW_GDstShift) & CR2CW_GDstMask);
    B = (CR2CW_BDstShift<0) ? ((B >> (-CR2CW_BDstShift)) & CR2CW_BDstMask) : ((B << CR2CW_BDstShift) & CR2CW_BDstMask);
    A = (CR2CW_ADstShift<0) ? ((A >> (-CR2CW_ADstShift)) & CR2CW_ADstMask) : ((A << CR2CW_ADstShift) & CR2CW_ADstMask);
    
    // Set up the "unused" bits.  They should always be 1.
    U = ~( CR2CW_RDstMask | CR2CW_GDstMask | CR2CW_BDstMask | CR2CW_ADstMask );

    // Compute pixel index
    CW_T = X + (Y*DstBitmap->GetPWidth(nMipID));
    CW_C = R | G | B | A | U;
    CW();
}

//=========================================================================

void x_bitmap::SetPixelIndex( s32 X, s32 Y, s32 Index, s32 nMipID )
{
    ASSERT(DstBitmap);
    ASSERT((X>=0) && (X<DstBitmap->GetWidth()));
    ASSERT((Y>=0) && (Y<DstBitmap->GetHeight()));
    ASSERT((Index>=0) && (Index<=DstBitmap->GetNClutColors()));
    ASSERT(DstFormat.IsClutUsed);

    // Compute pixel index
    IW_T = X + (Y*DstBitmap->GetPWidth(nMipID));
    IW_I = Index;
    IW();
}

//=========================================================================

void x_bitmap::SetClutColor( s32 Index, color C )
{
    u32 R, G, B, A, U;

    ASSERT(DstBitmap);
    ASSERT((Index>=0) && (Index<=DstBitmap->GetNClutColors()));
    ASSERT(DstFormat.IsClutUsed);

	if ( DstBitmap->IsClutSwizzled() )
	{
		if ( ((Index & 0x1f) >= 8) && ((Index & 0x1f) < 16) )
		{
			Index += 8;
		}
		else if ( ((Index & 0x1f) >= 16) && ((Index & 0x1f) < 24) )
		{
			Index -= 8;
		}
	}

    // Build destination color
    R = C.R;
    G = C.G;
    B = C.B;
    A = C.A;

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


//=========================================================================

void x_bitmap::Reset( void )
{
    m_pClut             = NULL;
    m_pData             = NULL;
    m_DataSize          = 0;
    m_ClutSize          = 0;

    m_Width             = 0;
    m_Height            = 0;
    m_PW                = 0;
    m_PH                = 0;
    m_VramId            = -1;
    m_Flags             = 0;

    m_MaxClutID         = 0;
    m_Format            = -1;
    m_NCluts            = 0;
    m_NMips             = 0;
}

//=========================================================================

void x_bitmap::KillBitmap( void )
{
    
    if (m_Flags & FLAG_XBITMAP_IS_VALID)
        ASSERT( SanityCheck() );

    if ( (m_Flags & FLAG_DATA_OWNED) && m_pData)
        x_free(m_pData);
    m_pData = NULL;

    if ( (m_Flags & FLAG_CLUT_OWNED) && m_pClut)
        x_free(m_pClut);
    m_pClut = NULL;

    Reset();
}

//=========================================================================

void x_bitmap::CopyFrom( const x_bitmap& Src )
{
    // Start off with everything clean
    KillBitmap();

    // Dupe the source pixel data
    m_DataSize = Src.GetTotalDataSize();
    m_pData = (byte*)x_malloc( m_DataSize );
    ASSERT( m_pData );
    x_memcpy( m_pData, Src.GetRawDataPtr(), m_DataSize );

    // Optionally dupe the source clut data
    if ( Src.GetClutSize() )
    {
        m_ClutSize = Src.GetClutSize();
        m_pClut = (byte*)x_malloc( m_ClutSize );
        ASSERT( m_pClut );

        x_memcpy( m_pClut, Src.GetClutPtr(0), m_ClutSize );
    }
    
    // Copy the members
    m_Width         = Src.GetWidth();
    m_Height        = Src.GetHeight();
    m_PW            = Src.GetPWidth();
    m_PH            = Src.GetPHeight();
    m_MaxClutID     = Src.GetNClutColors()-1;
    m_NCluts        = Src.GetNCluts();
    m_Format        = Src.GetFormat();

    m_NMips         = Src.GetNMips();

    // Set the flags
    m_Flags = FLAG_DATA_OWNED | FLAG_XBITMAP_IS_VALID;
    if (m_ClutSize>0)
        m_Flags |= FLAG_CLUT_OWNED;

	if (Src.IsClutSwizzled())
		m_Flags |= FLAG_CLUT_SWIZZLED;
	if (Src.IsDataSwizzled())
		m_Flags |= FLAG_DATA_SWIZZLED;
	if (Src.IsReducedIntensity())
		m_Flags |= FLAG_REDUCED_INTENSITY;

}   

//=========================================================================

void x_bitmap::CopyHighFrom( const x_bitmap& Src )
{
    // Start off with everything clean
    KillBitmap();

    // Dupe the source pixel data
    m_DataSize = Src.GetTotalDataSize();
    m_pData = (byte*)x_malloc_top( m_DataSize );
    ASSERT( m_pData );
    x_memcpy( m_pData, Src.GetRawDataPtr(), m_DataSize );

    // Optionally dupe the source clut data
    if ( Src.GetClutSize() )
    {
        m_ClutSize = Src.GetClutSize();
        m_pClut = (byte*)x_malloc_top( m_ClutSize );
        ASSERT( m_pClut );

        x_memcpy( m_pClut, Src.GetClutPtr(0), m_ClutSize );
    }
    
    // Copy the members
    m_Width         = Src.GetWidth();
    m_Height        = Src.GetHeight();
    m_PW            = Src.GetPWidth();
    m_PH            = Src.GetPHeight();
    m_MaxClutID     = Src.GetNClutColors()-1;
    m_NCluts        = Src.GetNCluts();
    m_Format        = Src.GetFormat();

    m_NMips         = Src.GetNMips();

    // Set the flags
    m_Flags = FLAG_DATA_OWNED | FLAG_XBITMAP_IS_VALID;
    if (m_ClutSize>0)
        m_Flags |= FLAG_CLUT_OWNED;

	if (Src.IsClutSwizzled())
		m_Flags |= FLAG_CLUT_SWIZZLED;
	if (Src.IsDataSwizzled())
		m_Flags |= FLAG_DATA_SWIZZLED;
	if (Src.IsReducedIntensity())
		m_Flags |= FLAG_REDUCED_INTENSITY;
}

///////////////////////////////////////////////////////////////////////////

static 
void   RGBToHSV( f32 R, f32 G, f32 B, 
                 f32* H, f32* S, f32* V )
{
    f32     Max,Min;

    Max = MAX(R,G);
    Max = MAX(Max,B);

    Min = MIN(R,G);
    Min = MIN(Min,B);

    *V = Max;

    *S = (Max != 0) ? ((Max-Min)/Max) : 0;

    if (*S == 0)
        *H = 0;
    else
    {
        f32     Delta = 1.0f/(Max-Min);

        if ( R == Max )
            *H = (G-B)*Delta;

        else if ( G == Max )
            *H = 2.0f + (B-R)*Delta;

        else if ( B == Max )
            *H = 4.0f + (R-G)*Delta;

        *H *= R_60;

        if (*H < 0)
            *H += R_360;
    }
}

///////////////////////////////////////////////////////////////////////////
//
//  HSVIs2BetterThan1 
//
//  Given 2 RGB triplets, 1 and 2, compare them to Src to see if 2 is 
//  better than 1
//
///////////////////////////////////////////////////////////////////////////
/*
static
xbool   HSVIs2BetterThan1( s32 R1, s32 G1, s32 B1,
                           s32 R2, s32 G2, s32 B2,
                           s32 RSrc, s32 GSrc, s32 BSrc )
{
    f32 H1,S1,V1;
    f32 H2,S2,V2;
    f32 H0,S0,V0;

    //x_printf("HSV: Comparing %ld,%ld,%ld and %ld,%ld,%ld for match with %ld,%ld,%ld\n",
    //            R1,G1,B1,R2,G2,B2,RSrc,GSrc,BSrc);

    RGBToHSV((f32)RSrc/255.0f, (f32)GSrc/255.0f, (f32)BSrc/255.0f, &H0, &S0, &V0 );
    RGBToHSV((f32)R1/255.0f,   (f32)G1/255.0f,   (f32)B1/255.0f,   &H1, &S1, &V1 );
    RGBToHSV((f32)R2/255.0f,   (f32)G2/255.0f,   (f32)B2/255.0f,   &H2, &S2, &V2 );

    f32     Ang1,Ang2;

    Ang1 = x_MinAngleDiff( H1, H0 );
    Ang2 = x_MinAngleDiff( H2, H0 );

    if (Ang2 < Ang1)
        return TRUE;

    //x_printf("** Fallback to saturation test\n");

    if ( x_abs( S2 - S0 ) < x_abs( S1 - S0 ) )
        return TRUE;

    //x_printf("*** Cannot decide\n");

    return FALSE;
}
*/
///////////////////////////////////////////////////////////////////////////

s32 FindBestColorInClut( color PC, color* pClut, vector3* pClutHSV, s32 NClutColors )
{
    s32 i;
    f32 PH,PS,PV; 
    s32 BestRGBDist;
    f32 BestHDist;
    f32 BestSDist;
    s32 BestIndex;

    // Convert pixel color into HSV
    RGBToHSV(((f32)PC.R/255.0f),
             ((f32)PC.G/255.0f),
             ((f32)PC.B/255.0f),
             &PH, &PS, &PV );

    // Set initial values
    BestHDist = 1000000000.0f;
    BestSDist = 1000000000.0f;
    BestRGBDist = 256*256*256*4;
    BestIndex = 0;

    // Loop through all clut entries
    for( i=0; i<NClutColors; i++ )
    {
        color   CC;
        s32     RGBDist;

        // Read clut color
        CC = pClut[i];

        // Get RGB Dist
        RGBDist =   ((s32)PC.R-(s32)CC.R)*((s32)PC.R-(s32)CC.R) +
                    ((s32)PC.G-(s32)CC.G)*((s32)PC.G-(s32)CC.G) +
                    ((s32)PC.B-(s32)CC.B)*((s32)PC.B-(s32)CC.B) +
                    ((s32)PC.A-(s32)CC.A)*((s32)PC.A-(s32)CC.A);

        // Check if this dist beats previous
        if( RGBDist <= BestRGBDist )
        {
            BestRGBDist = RGBDist;
            BestIndex = i;

            if( RGBDist <= 4 )
                return BestIndex;
        }

/*
        // Check if this dist beats previous
        if( RGBDist <= BestRGBDist )
        {
            f32     CH,CS,CV; 
            f32     HDist;
            f32     SDist;

            // Get HSV version of clut color
            CH = pClutHSV[i].X;
            CS = pClutHSV[i].Y;
            CV = pClutHSV[i].Z;

            // Get angular hue distance between +-R_180
            HDist = (CH-PH)+PI;
            while( HDist > R_360 )  HDist -= R_360;
            while( HDist < R_0   )  HDist += R_360;
            HDist -= PI;
            if( HDist < 0 ) HDist = -HDist;

            // Get saturation dist
            SDist = (CS-PS);
            if( SDist < 0 ) SDist = -SDist;

            if( RGBDist != BestRGBDist )
            {
                BestRGBDist = RGBDist;
                BestHDist = HDist;
                BestSDist = SDist;
                BestIndex = i;
            }
            else
            // Check if we beat previous using hue
            if( HDist <= BestHDist )
            {
                if(( HDist != BestHDist ) ||
                   ( SDist < BestSDist ))
                {
                    BestRGBDist = RGBDist;
                    BestHDist = HDist;
                    BestSDist = SDist;
                    BestIndex = i;
                }
            }
        }
*/
    }

    // return best index
    return BestIndex;
}

///////////////////////////////////////////////////////////////////////////
//
//  NOTES:  Steps performed in this version:
//          * Build basic info about each mip level (W,H,Datasize)
//          * Determine size of the prefix table
//          * Extract the source clut, if using a pal
//          * Convert the source image into ARGB_8888 32 bit format for 
//            ease of use
//          * For each mip level to build:
//              * Resample the original data to the mip dimensions
//              * If using a clut, remap each RGBA in the mip back into
//                the closest clut entry
//              * Use the xbitmap conversion routine to pull out either
//                indices or color values to put back into the mip's
//                data array
//          * Cleanup memory
//          * Fill in the entries in the prefix table
//          * Done
//
//  x_printf's to be removed later, after revisions are done.
//
///////////////////////////////////////////////////////////////////////////
void x_bitmap::BuildMips( s32 NMipsToBuild )
{
    s32     PixelW,PixelH;
    s32     MaxMip;
    s32     MipW[16],MipH[16];
    s32     PaddedByteSize[16];
    s32     Offset[16];
    s32     TotalDataSize;
    byte*   pData = NULL;                   // Place to build the data
    xbool   UsePal;                         // True is source is clut based
    s32     i;
    color   SrcColor[256];
    vector3 SrcColorHSV[256];
//s64     Ticks;

    ///////////////////////////////////////////////////////////////////////
    
//Ticks = x_GetTimerTicks();
    ASSERT( m_pData );
    ASSERT( NMipsToBuild > 0 );

    //---   if texture is small, no need to build mips!
    if ( (m_PW == 8) || (m_PH == 8) )
        return;

    //=====================================================================
    //=====================================================================
    //
    //  Build the reference info needed while building the mips
    //
    //=====================================================================
    //=====================================================================

    UsePal = IsClutBased();

    //
    //  Get the pixel dimensions of the texture
    //
    PixelW = m_PW;
    PixelH = m_PH;

    //
    //  Assert that they are valid, only if we are building mips below level 0
    //
    ASSERTS( ((PixelW-1) & (PixelW))==0, "x_bitmap: Bitmap width is not a power of 2" );
    ASSERTS( ((PixelH-1) & (PixelH))==0, "x_bitmap: Bitmap height is not a power of 2" );
    
    //
    //  Store base W/H in mip0
    //
    MipW[0]             = PixelW;
    MipH[0]             = PixelH;
    PaddedByteSize[0]   = ALIGN_16( (MipW[0] * MipH[0] * s_Format[m_Format].BPP) / 8 );
    MaxMip              = NMipsToBuild;
    TotalDataSize       = PaddedByteSize[0];

    //
    //  Setup the dimensions of each mip level, and find the max # of mips
    //  that should be constructed
    //
    //x_printf("Setting up mip dims\n");
    if ( (MipW[0] == 8) || (MipH[0] == 8) )
    {
        MaxMip = 0; 
    }
    else
    {
        for (i=1;i<=NMipsToBuild;i++)
        {
            MipW[i] = MipW[i-1] >> 1;    
            MipH[i] = MipH[i-1] >> 1;

            PaddedByteSize[i] = ALIGN_16( (MipW[i] * MipH[i] * s_Format[m_Format].BPP) / 8 );

            TotalDataSize += PaddedByteSize[i];

            if ( (MipW[i] == 8) || (MipH[i] == 8) )
            {
                MaxMip = i; 
                break;
            }
        }
    }
    //x_printf("NMipsToBuild = %ld, MaxMip = %ld\n",NMipsToBuild,MaxMip);

    //
    //  Figure out the size of the prefix table
    //
    s32 TableSize = sizeof(s32)*2 * (MaxMip+1);     // Size of offset table (+1 for Mip#0)
                                                    // which contains [Offset][Size]

    TableSize = ALIGN_16(TableSize);                // Make it aligned

    //  Get the total data size, and allocate the memory
    TotalDataSize += TableSize;
    pData = (byte*)x_malloc( TotalDataSize );
    ASSERTS( pData, "x_bitmap: Out of memory while building mips" );

    //
    //  If the source has a clut, build useful data for later
    //
    if (UsePal)
    {     
        SetPixelFormat( *this );
        //x_printf("** Extracting colors from CLUT\n");

        for (i=0;i<=m_MaxClutID;i++)
        {
            // Get platform independent color
            SrcColor[i] = GetClutColor(i);

            // Build HSV representation
            RGBToHSV( SrcColor[i].R,
                      SrcColor[i].G,
                      SrcColor[i].B,
                      &SrcColorHSV[i].X,
                      &SrcColorHSV[i].Y,
                      &SrcColorHSV[i].Z );
        }        
    }


    //=====================================================================
    //=====================================================================
    //
    //  We have the memory and info, now we need e-z to use source data
    //
    //=====================================================================
    //=====================================================================

    s32         CurOffset = TableSize;
    x_bitmap*   Temp;
    u32*        SrcData;

    //
    //  Explode the src data into a temporary 32 bit ARGB_8888 array
    //
    //x_printf("Converting source to 32bit ARGB\n");
    SrcData = (u32*)x_malloc( sizeof(u32) * PixelW*PixelH );
    ASSERT(SrcData);

    Temp = (x_bitmap*)x_malloc( sizeof(x_bitmap) );
    x_memset(Temp,0,sizeof(x_bitmap));
    ASSERT(Temp);

    Temp->SetupBitmap( x_bitmap::FMT_32_ARGB_8888, PixelW, PixelH, FALSE, (void*)SrcData );
    SetPixelFormat( *Temp, *this ); 

    // Copy pixels from actual bitmap into temp bitmap
    {
        s32 X,Y;

        for (Y=0;Y<PixelH;Y++)
        {
            for (X=0;X<PixelW;X++)
            {
                SetPixelColor( X, Y, GetPixelColor( X, Y ) );
            }
        }
    }
    //x_printf("Conversion done\n");

//Ticks = x_GetTimerTicks()-Ticks;
//x_printf("------------ MIP MAKING --------------\n");
//x_printf("%6.2f Conversion\n",x_TicksToMs( Ticks ));

    //=====================================================================
    //=====================================================================
    //
    //  Since the built-in conversion routines will be used to handle all
    //  of the many inbound and outbound data formats, we need to 
    //  allocate 2 x_bitmaps to perform the work with.
    //
    //  IntermediateBMP will be used to store the FMT_32_ARGB_8888 data
    //  DestBMP will be used as the target for a conversion, and will
    //           hold the data in the format required by the original bmp
    //
    //=====================================================================
    //=====================================================================

    x_bitmap*   IntermediateBmp;
    x_bitmap*   DestBmp;

    //
    //  Allocate them once, and reuse them until we're done
    //
    IntermediateBmp = (x_bitmap*)x_malloc( sizeof(x_bitmap) );
    ASSERT(IntermediateBmp);
    x_memset( IntermediateBmp, 0, sizeof(x_bitmap) );

    DestBmp = (x_bitmap*)x_malloc( sizeof(x_bitmap) );
    ASSERT(DestBmp);
    x_memset(DestBmp,0,sizeof(x_bitmap));
    
    for( i=0; i<=MaxMip; i++ )
    {
        u32*        DestData;    

//Ticks = x_GetTimerTicks();
        
        //x_printf("\n - Building mip %ld of %ld\n",i,MaxMip);
        Offset[i] = CurOffset;
        //x_printf("     This mip's offset == %ld\n",Offset[i]);

        //
        // Special case for level 0, it's just a straight memcpy
        //
        if ( i==0 )
        {
            x_memcpy( &(pData[Offset[i]]), m_pData, PaddedByteSize[i] );
            CurOffset += PaddedByteSize[i];
            continue;
        }

        //  
        //  Build a mip level using the original data as a source
        //
        DestData = (u32*)x_malloc( sizeof(u32) * MipW[i] * MipH[i] );
        ASSERT(DestData);
    
        Build32BitMipData( SrcData,  MipW[0], MipH[0],
                           DestData, MipW[i], MipH[i] );

//x_printf("%6.2f Build 32Bit mip\n",x_TicksToMs( x_GetTimerTicks() - Ticks ));
//Ticks = x_GetTimerTicks();

        //
        //  Decide whether we are dealing with palletized or not
        //
        if (UsePal)
        {
            //
            // Setup bitmap formats
            //
            IntermediateBmp->SetupBitmap( x_bitmap::FMT_32_ARGB_8888, 
                                          MipW[i], MipH[i], MipW[i], MipH[i], 
                                          FALSE, sizeof(u32)*MipW[i]*MipH[i], (void*)DestData,
                                          FALSE,m_ClutSize, m_pClut, m_MaxClutID+1,0);

            DestBmp->SetupBitmap( (x_bitmap::format)m_Format, 
                                   MipW[i], MipH[i], MipW[i], MipH[i],
                                   FALSE, PaddedByteSize[i], (void*)&(pData[Offset[i]]),
                                   FALSE, m_ClutSize, m_pClut, m_MaxClutID+1, 0);

            //
            //  Convert the bitmap data into the final format
            //
            SetPixelFormat( *DestBmp, *IntermediateBmp );
            {
                s32 X,Y;

                for (Y=0;Y<MipH[i];Y++)
                {
                    for (X=0;X<MipW[i];X++)
                    {
                        s32 I;
                        I = FindBestColorInClut( GetPixelColor( X, Y ), SrcColor, SrcColorHSV, m_MaxClutID+1 );
                        SetPixelIndex( X, Y, I );
                    }
                }
            }
        }
        else
        {
            //
            // Setup bitmap formats
            //
            IntermediateBmp->SetupBitmap( x_bitmap::FMT_32_ARGB_8888, MipW[i], MipH[i], FALSE, (void*)DestData );
            DestBmp->SetupBitmap( (x_bitmap::format)m_Format, MipW[i], MipH[i], FALSE, (void*)&(pData[Offset[i]]) );

            //
            //  Convert the bitmap data into the final format
            //
            SetPixelFormat( *DestBmp, *IntermediateBmp );
            {
                s32 X,Y;

                for (Y=0;Y<MipH[i];Y++)
                {
                    for (X=0;X<MipW[i];X++)
                    {
                        SetPixelColor( X, Y, GetPixelColor( X, Y ) );
                    }
                }
            }
        }

//x_printf("%6.2f Convert into format\n",x_TicksToMs( x_GetTimerTicks() - Ticks ));
//Ticks = x_GetTimerTicks();
       
        //
        //  Clean up the temp bitmaps
        //
        x_free(DestData);
        DestData = NULL;
        IntermediateBmp->KillBitmap();
        DestBmp->KillBitmap();
    
        ///////////////////////////////////////////////////////////////////

        CurOffset += PaddedByteSize[i];

//x_printf("%6.2f Cleanup\n",x_TicksToMs( x_GetTimerTicks() - Ticks ));
       
    }
    //x_printf("Done building mips\n");

    //=====================================================================
    //=====================================================================
    //
    //  Cleanup phase
    //
    //=====================================================================
    //=====================================================================

//Ticks = x_GetTimerTicks();

    //x_printf("Cleaning up\n");
    Temp->KillBitmap();
    //x_printf("- Free Temp\n");
    x_free(Temp);
    //x_printf("- Free Intermed\n");
    x_free(IntermediateBmp);
    //x_printf("- Free Dest\n");
    x_free(DestBmp);
    //x_printf("- Free Src\n");
    x_free(SrcData);

    if (IsDataOwned())
        x_free(m_pData);
    m_pData = pData;

    
    //
    //  Copy somne other data over
    //
    m_DataSize = TotalDataSize;
    m_NMips = MaxMip;

    //=====================================================================
    //=====================================================================
    //
    //  Setup the table at the head of the data
    //
    //=====================================================================
    //=====================================================================
    miptable* Table;
    Table = (miptable*)pData;
    for ( i=0; i<=MaxMip; i++ )
    {
        Table[i].Offset = Offset[i];
        Table[i].W      = MipW[i];
        Table[i].H      = MipH[i];
        //x_printf("Mip %ld: Offset = %ld, W = %ld, H = %ld\n",i,Offset[i],MipW[i],MipH[i]);
    }   

    //x_printf("Mipping done, NMips = %ld\n=========\n",m_NMips);
//x_printf("%6.2f Final Cleanup\n",x_TicksToMs( x_GetTimerTicks() - Ticks ));

}

///////////////////////////////////////////////////////////////////////////
//
//  Build32BitMipData
//
//  Src and Dst are assumed to be pointers to raw ARGB_8888 data.
//  
//
//
///////////////////////////////////////////////////////////////////////////
void x_bitmap::Build32BitMipData( u32* Src, s32 SrcW, s32 SrcH,
                                  u32* Dst, s32 DstW, s32 DstH )
{
    s32     BlockW,BlockH;
    s32     X,Y;
    s32     Size;
    
    BlockW = SrcW / DstW;                                       // Width of a block
    BlockH = SrcH / DstH;                                       // Height of a block

    Size = BlockW * BlockH;

    //x_printf("     Resampling %ld x %ld to %ld x %ld\n",SrcW,SrcH,DstW,DstH);

    for ( Y=0; Y<DstH; Y++)
    {
        s32     SY;
        u32*    SrcRow;

        SY = Y*BlockH;                                          // Y in srcimg of block start
        
        for ( X=0; X<DstW; X++ )
        {
            s32     SX;
            s32     SubX,SubY;
            s32     OP_R,OP_G,OP_B,OP_T;
            s32     TR_R,TR_G,TR_B,TR_T;
            s32     A,R,G,B,ASum;
            u32     C;

            SX = X*BlockW;                                      // X in srcimg of block start
            SrcRow = Src + ( SrcW*SY + SX );              

            // Clear totals
            OP_R=OP_G=OP_B=OP_T=0;
            TR_R=TR_G=TR_B=TR_T=0;
            ASum=0;

            // Loop through block and do sums
            for ( SubY=0; SubY<BlockH; SubY++ )
            {
                for ( SubX=0; SubX<BlockW; SubX++ )
                {
                    // Decode color
                    C = *(SrcRow+SubY*SrcW+SubX);
                    A = (C>>24) & 0xFF;
                    R = (C>>16) & 0xFF;
                    G = (C>> 8) & 0xFF;
                    B = (C>> 0) & 0xFF;

                    ASum += A;
                    if( A >= 128 )
                    {
                        OP_R += R;
                        OP_G += G;
                        OP_B += B;
                        OP_T ++;
                    }
                    else
                    {
                        TR_R += R;
                        TR_G += G;
                        TR_B += B;
                        TR_T ++;
                    }
                }
            }

            // Compute alpha and decide whether to treat as opaque or transparent
            A = ASum / Size;

            if( A >= 128 )
            {
                R = OP_R / OP_T;
                G = OP_G / OP_T;
                B = OP_B / OP_T;
                //A = 255;
            }
            else
            {
                R = TR_R / TR_T;
                G = TR_G / TR_T;
                B = TR_B / TR_T;
                //A = 0;
            }

            Dst[ Y*DstW + X ] = (((u32)(A))<<24) | 
                                (((u32)(R))<<16) | 
                                (((u32)(G))<< 8) | 
                                ((u32)(B));
        }
    }
}

//==========================================================================

s32 x_bitmap::GetSerializedSize( void )
{   
    return m_DataSize + m_ClutSize + sizeof(x_bitmap);
}   


//==========================================================================

xbool x_bitmap::Save( X_FILE* fp )
{
    ASSERT( fp );

    // Write out header
    x_fwrite( this, sizeof(x_bitmap), 1, fp );

    // Write out Pixel data
    x_fwrite( m_pData, m_DataSize, 1, fp );

    // Write out CLUT if present
    if( m_ClutSize > 0 )
        x_fwrite( m_pClut, m_ClutSize, 1, fp );

    // Return success
    return TRUE;
}

//==========================================================================

xbool x_bitmap::Save( byte* pStream )
{
    ASSERT( pStream );

    byte* Cur = pStream;

    // Write out header
    x_memcpy( Cur, this, sizeof(x_bitmap));
    Cur += sizeof(x_bitmap);

    // Write out Pixel data
    x_memcpy( Cur, m_pData, m_DataSize );
    Cur += m_DataSize;

    // Write out CLUT if present
    if( m_ClutSize > 0 )
    {
        x_memcpy( Cur, m_pClut, m_ClutSize );
    }

    // Return success
    return TRUE;
}

//==========================================================================

xbool x_bitmap::Load( byte* pStream, s32& rNBytesRead )
{
    ASSERT( pStream );

    byte* Cur = pStream;

    // Read header
    x_memcpy( this, Cur, sizeof(x_bitmap));
    Cur += sizeof(x_bitmap);

    // Read pixel data
    m_pData = (byte*)x_malloc(m_DataSize);
    ASSERT( m_pData );
    x_memcpy( m_pData, Cur, m_DataSize );
    Cur += m_DataSize;

    // Read in CLUT if present
    if( m_ClutSize > 0 )
    {
        m_pClut = (byte*)x_malloc(m_ClutSize);
        ASSERT( m_pClut );
        x_memcpy( m_pClut, Cur, m_ClutSize );
        Cur += m_ClutSize;
    }
    rNBytesRead = (u32)Cur - (u32)pStream;

    // Return success
    return TRUE;
}

//==========================================================================

xbool x_bitmap::Load( byte* pStream )
{
    s32 NBytesRead;

    return Load( pStream, NBytesRead );
}

//==========================================================================

xbool x_bitmap::Load( X_FILE* fp )
{
    ASSERT( fp );

    // Read header
    x_fread( this, sizeof(x_bitmap), 1, fp );

#ifdef BIG_ENDIAN
	m_DataSize = ENDIAN_SWAP_32(m_DataSize);
	m_ClutSize = ENDIAN_SWAP_32(m_ClutSize);

	m_Width = ENDIAN_SWAP_16(m_Width);
	m_Height = ENDIAN_SWAP_16(m_Height);
	m_PW = ENDIAN_SWAP_16(m_PW);
	m_PH = ENDIAN_SWAP_16(m_PH);
	m_Flags = ENDIAN_SWAP_16(m_Flags);
#endif

    // Read pixel data
    m_pData = (byte*)x_malloc(m_DataSize);
    ASSERT( m_pData );
    x_fread( m_pData, m_DataSize, 1, fp );

    // Read in CLUT if present
    if( m_ClutSize > 0 )
    {
        m_pClut = (byte*)x_malloc(m_ClutSize);
        ASSERT( m_pClut );
        x_fread( m_pClut, m_ClutSize, 1, fp );
    }

#ifdef BIG_ENDIAN
    if (m_NMips)
    {
        miptable* M = (miptable*)m_pData;
		for (s32 mip=0; mip<m_NMips; mip++)
		{
	        M[mip].Offset = ENDIAN_SWAP_32(M[mip].Offset);
	        M[mip].W = ENDIAN_SWAP_16(M[mip].W);
	        M[mip].H = ENDIAN_SWAP_16(M[mip].H);
		}
    }

#endif

    // Return success
    return TRUE;
}

//==========================================================================

xbool x_bitmap::Save( char* pFileName )
{
    X_FILE* fp;

    // Open file for writing
    fp = x_fopen(pFileName,"wb");
    if( !fp )
        return FALSE;

    // Save to file
    Save(fp);

    // Close file
    x_fclose(fp);

    // Return success
    return TRUE;
}

//==========================================================================

xbool x_bitmap::Load( char* pFileName )
{
    X_FILE* fp;

    // Open file for reading
    fp = x_fopen(pFileName,"rb");
    if( !fp )
        return FALSE;

    // Save to file
    Load(fp);

    // Close file
    x_fclose(fp);

    // Return success
    return TRUE;
}

//==========================================================================

void x_bitmap::SetAllAlpha( s32 Alpha )
{
    s32 i,j;
    color C;

    ASSERT( (Alpha>=0) && (Alpha<=255) );

    // Setup format conversions
    SetPixelFormat( *this );

    // Deal with color per pixel
    if( GetBPP() > 8 )
    {
        for( i=0; i<m_Height; i++ )
        for( j=0; j<m_Width; j++ )
        {
            C = GetPixelColor(j,i);
            C.A = (byte)Alpha;
            SetPixelColor(j,i,C);
        }
    }
    else
    // Deal with clut
    {
        for( i=0; i<=m_MaxClutID; i++ )
        {
            C = GetClutColor(i);
            C.A = (byte)Alpha;
            SetClutColor(i,C);
        }
    }
}

//==========================================================================


