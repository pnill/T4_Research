//=========================================================================
//
//  PS2_FONT.CPP
//
//=========================================================================
#include <stdlib.h>
#include <eekernel.h>
#include <eeregs.h>
#include <math.h>
#include <libdma.h>
#include <libgraph.h>
#include <sifdev.h>
#include <libvu0.h>
#include <stdio.h>

#include "Q_Draw.hpp"
#include "Q_Engine.hpp"
#include "Q_PS2.hpp"
#include "Q_VRAM.hpp"
#include "x_plus.hpp"
#include "x_debug.hpp"
#include "x_stdio.hpp"
#include "x_bitmap.hpp"

#include "dmahelp.hpp"
#include "vifhelp.hpp"
#include "dvtrace.hpp"

//=========================================================================

typedef struct
{
    sceDmaTag   DMA;
    s32         VIF[4];
    sceGifTag   GIF;
} char_header;

static char_header*     s_pCharHeader;
static s32              s_NChars;
static x_bitmap         s_FontBMP;
static sceGifTag        s_GIFTAG_Chars;

extern char FontClut[256*4];
extern char FontPixel[256*128];

//=========================================================================

void FONT_InitModule( void )
{
    ASSERT( (((u32)FontPixel)&0x0F) == 0 );
    ASSERT( (((u32)FontClut)&0x0F) == 0 );
    s_FontBMP.SetupBitmap( x_bitmap::FMT_P8_ABGR_8888,
                           256, 128,
                           FALSE,
                           FontPixel,
                           FontClut,
                           256 );
    ((color*)FontClut)[0].A = 0;

    VRAM_Register( s_FontBMP );

    s_FontBMP.SetAllAlpha(80);

    // Confirm size and format
    ASSERT( s_FontBMP.GetWidth() == 256 );
    ASSERT( s_FontBMP.GetHeight() == 128 );
    ASSERT( s_FontBMP.GetBPP() == 8 );

    // Build Giftag
    VIFHELP_BuildGifTag1(  &s_GIFTAG_Chars, VIFHELP_GIFMODE_PACKED, 
                           5, 0, TRUE, 
                           VIFHELP_GIFPRIMTYPE_SPRITE,
                           VIFHELP_GIFPRIMFLAGS_TEXTURE |
                           VIFHELP_GIFPRIMFLAGS_UV |
                           VIFHELP_GIFPRIMFLAGS_ALPHA, 
                           TRUE );

    VIFHELP_BuildGifTag2(  &s_GIFTAG_Chars, 
                           VIFHELP_GIFREG_RGBAQ, 
                           VIFHELP_GIFREG_UV, 
                           VIFHELP_GIFREG_XYZ2, 
                           VIFHELP_GIFREG_UV );

    VIFHELP_BuildGifTag3(  &s_GIFTAG_Chars, 
                           VIFHELP_GIFREG_XYZ2, 
                           0, 
                           0, 
                           0 );

}

//=========================================================================

void FONT_KillModule( void )
{
    s_FontBMP.KillBitmap();
}

//=========================================================================
 
static void BuildCharHeader( char_header* pHeader, s32 NChars )
{
    s32 DMASize;
    s32 TotalInstrSize;

    TotalInstrSize = NChars*5*16;
    DMASize = sizeof(char_header) - sizeof(sceDmaTag) + TotalInstrSize;
    ASSERT( (DMASize&0x0F) == 0 );
    DMAHELP_BuildTagCont( &pHeader->DMA, DMASize );

    pHeader->VIF[0] = 0;
    pHeader->VIF[1] = 0;
    pHeader->VIF[2] = 0;
    pHeader->VIF[3] = SCE_VIF1_SET_DIRECT( 1+(TotalInstrSize>>4), 0 );

    pHeader->GIF = s_GIFTAG_Chars;
    pHeader->GIF.NLOOP = NChars;
}

//=========================================================================

void FONT_BeginRender( void )
{
	s32 XRes, YRes;
	ENG_GetResolution( XRes, YRes );

	ENG_Set2DClipArea( 0, 0, XRes, YRes );

    ENG_SetRenderFlags( ENG_ZBUFFER_TEST_OFF |
                        ENG_ZBUFFER_FILL_OFF |
						ENG_ALPHA_TEST_OFF   |
                        ENG_ALPHA_BLEND_ON );
    ENG_SetBlendMode( ENG_BLEND_NORMAL );

    VRAM_Activate( s_FontBMP );

    // Compute size of header and skip over
    s_pCharHeader = (char_header*)pDList;
    pDList += sizeof(char_header);

    // Reset char count
    s_NChars = 0;
}

//=========================================================================

void FONT_EndRender( void )
{
    BuildCharHeader( s_pCharHeader, s_NChars );
}

//=========================================================================
#define FSAAToggle 2

inline s32 OffsetX( void )
{
    return (2048 - (ENG_GetScreenWidth()/2));
}
inline s32 OffsetY( void )
{
    return (2048 - (ENG_GetScreenHeight()/2));
}

void FONT_Render( char* Str, s32 SX, s32 SY, color Color )
{
    if (*Str == ' ')
        return;
    SX = (s32)((f32)SX * PS2_GetScreenXConversionMultiplier());
    SY = (s32)((f32)SY * PS2_GetScreenYConversionMultiplier());
    s32 C;
    s32 CX,CY;
	SY *= FSAAToggle;
    s32 width = (s32)(16.0f * PS2_GetScreenXConversionMultiplier());
    s32 height = (s32)(16.0f * PS2_GetScreenYConversionMultiplier());
    while( *Str )
    {
        C = (*Str) - 32;
        CX = (C & 0xF);
        CY = (C >> 4 );
        Str++;

        ((s32*)pDList)[ 0] = Color.R;
        ((s32*)pDList)[ 1] = Color.G;
        ((s32*)pDList)[ 2] = Color.B;
        ((s32*)pDList)[ 3] = Color.A;

        ((s32*)pDList)[ 4] = 8 + (((CX<<4)+2)<<4);
        ((s32*)pDList)[ 5] = 8 + (((CY<<4)+0)<<4);
        ((s32*)pDList)[ 6] = 0;
        ((s32*)pDList)[ 7] = 0;

        ((s32*)pDList)[ 8] = 8 + ((OffsetX()+SX)<<4);
        ((s32*)pDList)[ 9] = (OffsetY()<<4) + ((8 + ((SY)<<4))>>1);
        ((s32*)pDList)[10] = (1<<23);
        ((s32*)pDList)[11] = 0;

        ((s32*)pDList)[12] = 8+(((CX<<4)+14)<<4);
        ((s32*)pDList)[13] = 8+(((CY<<4)+16)<<4);
        ((s32*)pDList)[14] = 0;
        ((s32*)pDList)[15] = 0;

        ((s32*)pDList)[16] = 8 + ((OffsetX()+SX+width)<<4);
        ((s32*)pDList)[17] = (OffsetY()<<4) + ((8 + ((SY+(height*FSAAToggle))<<4))>>1);
        ((s32*)pDList)[18] = (1<<23);
        ((s32*)pDList)[19] = 0;

        SX += (s32)(16.0f * PS2_GetScreenXConversionMultiplier());
        pDList += 20*4;
        s_NChars++;
    }
}

//=========================================================================

void FONT_RenderText ( s32 x, s32 y, char* s, xbool Large )
{
    FONT_Render( s, x, y, color(128,128,128,128) );
}

//=========================================================================

s32 FONT_GetStringWidth( char* Str )
{
    if( Str == NULL )
        return 0;

    s32 sx = 0;
    s32 MaxW = 0;

    while( *Str )
    {
        u8 letter = (u8)*Str++;

        // on newline char save current str width if larger than max
        if( letter == '\n' )
        {
            if( sx > MaxW )
                MaxW = sx;

            continue;
        }

        // Skip invalid characters
//        if( (letter < s_cLowChar) || (letter > s_cHighChar) )
//            continue;

        // Get the glyph for this character
//        GLYPH_ATTR* pGlyph = &s_Glyphs[letter - s_cLowChar];

        // Add the character's width value(s) to total length of string
//        sx += pGlyph->wOffset;
//        sx += pGlyph->wAdvance;
        sx += 16;
    }

    if( sx > MaxW )
        MaxW = sx;

    return MaxW;
}

//==========================================================================

s32 FONT_GetLineHeight( void )
{
    //return s_dwFontHeight;
    return 16;
}

//==========================================================================

s32 FONT_GetStringLineCount( char* Str )
{
    if( Str == NULL )
        return 0;

    if( Str[0] == '\0' )
        return 0;

    s32 NLines = 1;

    while( *Str )
    {
        if( *Str == '\n' )
            NLines++;

        Str++;
    }

    return NLines;
}

//==========================================================================
