////////////////////////////////////////////////////////////////////////////
//
// XBOX_Font.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_debug.hpp"
#include "x_bitmap.hpp"
#include "x_plus.hpp"

#include "Q_Draw.hpp"
#include "Q_Engine.hpp"
#include "Q_XBOX.hpp"
#include "Q_VRAM.hpp"

#include "XBOX_Font.hpp"


////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////

#define D3DFVF_FONT2DVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

// Max size for the font class' vertex buffer
#define FONT_MAX_VERTICES 50*6


////////////////////////////////////////////////////////////////////////////
// STRUCTS
////////////////////////////////////////////////////////////////////////////

struct FONT2DVERTEX
{
    D3DXVECTOR4  p;
    u32          color;
    f32          tu, tv;
};


////////////////////////////////////////////////////////////////////////////
// EXTERNS
////////////////////////////////////////////////////////////////////////////

extern u8 g_XBOXFontClut[];
extern u8 g_XBOXFontData[];
extern u8 g_XBOXFontGlyph[];


////////////////////////////////////////////////////////////////////////////
// STATIC VARIABLES
////////////////////////////////////////////////////////////////////////////

// Font texture
static x_bitmap                 s_FontBMP;
static LPDIRECT3DVERTEXBUFFER8  s_pFontVB = NULL; // Buffer to hold vertices

// Font and texture dimensions
static u32 s_dwFontHeight;
static u32 s_dwTexWidth;
static u32 s_dwTexHeight;

// Unicode ranges
static char s_cLowChar;
static char s_cHighChar;

// Glyph data for the font
static u32          s_dwNumGlyphs;
static GLYPH_ATTR*  s_Glyphs;


////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

inline FONT2DVERTEX InitFont2DVertex( const D3DXVECTOR4& p, u32 color, f32 tu, f32 tv )
{
    FONT2DVERTEX v;

    v.p     = p;
    v.color = color;
    v.tu    = tu;
    v.tv    = tv;

    return v;
}

//=========================================================================

err FONT_InitModule( void )
{
    s_FontBMP.SetupBitmap( x_bitmap::FMT_P8_ARGB_8888,
                           128, 256,
                           FALSE,
                           g_XBOXFontData,
                           g_XBOXFontClut,
                           256 );

    s_FontBMP.SetDataSwizzled();

    VRAM_Register( s_FontBMP );


    DXCHECK( XBOX_GetDevice()->CreateVertexBuffer( FONT_MAX_VERTICES * sizeof(FONT2DVERTEX),
                                                   D3DUSAGE_DYNAMIC,
                                                   0L,
                                                   D3DPOOL_DEFAULT,
                                                   &s_pFontVB ) );
    ASSERT( s_pFontVB != NULL );

    // Read header
    u32* pGlyph32;
    u8*  pGlyph8;
	u8   Padding;
    u32  dwVersion;
	u32  dwBPP;

    pGlyph32 = (u32*)g_XBOXFontGlyph;

    // Get glyph header info
    dwVersion      = *pGlyph32++;
    s_dwFontHeight = *pGlyph32++;
    s_dwTexWidth   = *pGlyph32++;
    s_dwTexHeight  = *pGlyph32++;
    dwBPP          = *pGlyph32++;

    // Check version of file (to make sure it matches up with the FontMaker tool)
    if( dwVersion != 0x00000004 )
    {
        ASSERTS( FALSE, "Font: Incorrect version number on font glyph data!" );
        return ERR_FAILURE;
    }

    // Read the low and high char
    pGlyph8 = (u8*)pGlyph32;

    s_cLowChar  = *pGlyph8++;
    s_cHighChar = *pGlyph8++;

    Padding = *pGlyph8++;
    Padding = *pGlyph8++;

    pGlyph32 = (u32*)pGlyph8;

    // Read the number of glyph attributes
    s_dwNumGlyphs = *pGlyph32++;

    // Assign pointer to glyph data
    s_Glyphs = (GLYPH_ATTR*)pGlyph32;

    return (ERR_SUCCESS);
}

//=========================================================================

void FONT_KillModule( void )
{
    VRAM_UnRegister( s_FontBMP );
    s_FontBMP.KillBitmap();

    SAFE_RELEASE( s_pFontVB );
}

//=========================================================================

void FONT_BeginRender( void )
{
    if( s_pFontVB == NULL )
        return;

    VRAM_Activate( s_FontBMP );

    DXWARN( XBOX_GetDevice()->SetStreamSource( 0, s_pFontVB, sizeof(FONT2DVERTEX) ) );
    XBOX_ActivateVertexShader( D3DFVF_FONT2DVERTEX );
    XBOX_DeActivatePixelShader( 0 );

    DRAW_SetMode( DRAW_2D |
                  DRAW_FILL |
                  DRAW_TEXTURE |
                  DRAW_ALPHA |
                  DRAW_NO_CLIP |
                  DRAW_NO_LIGHT |
                  DRAW_NO_ZBUFFER );
}

//=========================================================================

void FONT_EndRender( void )
{
    VRAM_Deactivate( s_FontBMP );
}

//=========================================================================

void FONT_Render( char* Str, s32 SX, s32 SY, color Color )
{
    if( s_pFontVB == NULL )
        return;

	if( Str == NULL )
		return;

    // Set the starting screen position
    f32 sx = (f32)SX;
    f32 sy = (f32)SY;

    u32 dwColor = Color.Get();

    // Fill vertex buffer
    FONT2DVERTEX* pVertices;
    u32           dwNumTriangles = 0;

    s_pFontVB->Lock( 0, 0, (BYTE**)&pVertices, NULL );

    while( *Str )
    {
        // Get the current letter in the string
        char letter = *Str++;

        // Handle the newline character
        if( letter == '\n' )
        {
            sx  = (f32)SX;
            sy += s_dwFontHeight;
        }

        // Skip invalid characters
        if( (letter < s_cLowChar) || (letter > s_cHighChar) )
            continue;

        // Get the glyph for this character
        GLYPH_ATTR* pGlyph = &s_Glyphs[letter - s_cLowChar];

        // Setup the screen coordinates (note the 0.5f shift value which is to
        // align texel centers with pixel centers)
        sx += pGlyph->wOffset;
        f32 sx1 = sx - 0.5f;
        f32 sx2 = sx - 0.5f + ((f32)pGlyph->wWidth + 1);
        f32 sy1 = sy - 0.5f;
        f32 sy2 = sy - 0.5f + ((f32)s_dwFontHeight + 1);
        sx += pGlyph->wAdvance;

        // Setup the texture coordinates (note the fudge factor for converting
        // from integer texel values to f32ing point texture coords).
        f32 tx1 = ( pGlyph->left   * ( s_dwTexWidth-1) ) / s_dwTexWidth;
        f32 ty1 = ( pGlyph->top    * (s_dwTexHeight-1) ) / s_dwTexHeight;
        f32 tx2 = ( pGlyph->right  * ( s_dwTexWidth-1) ) / s_dwTexWidth;
        f32 ty2 = ( pGlyph->bottom * (s_dwTexHeight-1) ) / s_dwTexHeight;

        // Set up the vertices (1 quad = 2 triangles = 6 vertices)
        *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx1,sy2,0.9f,1.0f), dwColor, tx1, ty2 );
        *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx1,sy1,0.9f,1.0f), dwColor, tx1, ty1 );
        *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx2,sy2,0.9f,1.0f), dwColor, tx2, ty2 );
        *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx2,sy1,0.9f,1.0f), dwColor, tx2, ty1 );
        *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx2,sy2,0.9f,1.0f), dwColor, tx2, ty2 );
        *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx1,sy1,0.9f,1.0f), dwColor, tx1, ty1 );
        dwNumTriangles += 2;

        // If the vertex buffer is full, render it
        if( dwNumTriangles*3 > (FONT_MAX_VERTICES-6) )
        {
            // Unlock, render, and relock the vertex buffer
            s_pFontVB->Unlock();
            DXWARN( XBOX_GetDevice()->DrawPrimitive( D3DPT_TRIANGLELIST, 0, dwNumTriangles ) );
            s_pFontVB->Lock( 0, 0, (BYTE**)&pVertices, 0L );
            dwNumTriangles = 0L;
        }
    }

    // If there's any vertices left in the vertex buffer, render it
    s_pFontVB->Unlock();

    if( dwNumTriangles > 0 )
        DXWARN( XBOX_GetDevice()->DrawPrimitive( D3DPT_TRIANGLELIST, 0, dwNumTriangles ) );
}

//=========================================================================

void FONT_RenderText( s32 x, s32 y, char* s, xbool Large )
{
//    FONT_Render( s, x+2, y+2, color(0,0,0,180) );
    FONT_Render( s, x, y, color(255,255,255,255) );
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
        if( (letter < s_cLowChar) || (letter > s_cHighChar) )
            continue;

        // Get the glyph for this character
        GLYPH_ATTR* pGlyph = &s_Glyphs[letter - s_cLowChar];

        // Add the character's width value(s) to total length of string
        sx += pGlyph->wOffset;
        sx += pGlyph->wAdvance;
    }

    if( sx > MaxW )
        MaxW = sx;

    return MaxW;
}

//==========================================================================

s32 FONT_GetLineHeight( void )
{
    return s_dwFontHeight;
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
