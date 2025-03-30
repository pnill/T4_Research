////////////////////////////////////////////////////////////////////////////
//
//  GC_FONT.CPP
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "Q_Engine.hpp"
#include "Q_GC.hpp"
#include "Q_VRAM.hpp"

#include "GC_Font.hpp"


////////////////////////////////////////////////////////////////////////////
// EXTERNS AND STATIC-GLOBAL VARIABLES
////////////////////////////////////////////////////////////////////////////

extern u8 g_GCFontClut[];
extern u8 g_GCFontData[];
extern u8 g_GCFontGlyph[];

// Font texture
x_bitmap            s_FontBMP;

// Font and texture dimensions
static u32          s_dwFontHeight;
static u32          s_dwTexWidth;
static u32          s_dwTexHeight;

// Unicode ranges
static u8           s_cLowChar;
static u8           s_cHighChar;

// Glyph data for the font
static u32          s_dwNumGlyphs;
static GLYPH_ATTR*  s_Glyphs;

// Saved projection matrix during render
static Mtx44        s_SavedProj;
static s32          s_VPL;
static s32          s_VPT;
static s32          s_VPW;
static s32          s_VPH;


////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

err FONT_InitModule( void )
{
    s_FontBMP.SetupBitmap( x_bitmap::FMT_P8_ARGB_3444,
                           128, 256,
                           FALSE,
                           g_GCFontData,
                           g_GCFontClut,
                           256 );

    s_FontBMP.SetDataSwizzled();

    VRAM_Register( s_FontBMP );

    u32* pGlyph32;
    u8*  pGlyph8;
    u8   Padding;
    u32  dwVersion;
    u32  dwBPP;

    pGlyph32 = (u32*)g_GCFontGlyph;

    // Get glyph header info
    dwVersion       = *pGlyph32++;
    s_dwFontHeight  = *pGlyph32++;
    s_dwTexWidth    = *pGlyph32++;
    s_dwTexHeight   = *pGlyph32++;
    dwBPP           = *pGlyph32++;

    // Check version of file (to make sure it matches up with the FontMaker tool)
    if( dwVersion != 0x00000004 )
    {
        x_printf( "Font: Incorrect version number on font glyph data!\n" );
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

//==========================================================================

void FONT_KillModule( void )
{
    VRAM_UnRegister( s_FontBMP );
    s_FontBMP.KillBitmap();
}

//==========================================================================

void FONT_BeginRender( void )
{
    VRAM_Activate( s_FontBMP );

    ENG_SetRenderFlags( ENG_ZBUFFER_TEST_OFF | ENG_ZBUFFER_FILL_ON | ENG_ALPHA_TEST_OFF | ENG_ALPHA_BLEND_ON );
    ENG_SetBlendMode( ENG_BLEND_NORMAL );

    GC_EnableLighting( FALSE, FALSE );

    GC_GetProjMatrix( s_SavedProj );

    Mtx44 mOrth;
    s32 XRes, YRes;

    ENG_GetResolution( XRes, YRes );

    //--- save current viewport and make new full-screen one
    GC_GetViewport( s_VPL, s_VPT, s_VPW, s_VPH );
    GC_SetViewport( 0, 0, XRes, YRes );

    MTXOrtho( mOrth, 0, YRes-1, 0, XRes-1, 0.0f, 1.0f );

    GXSetProjection( mOrth, GX_ORTHOGRAPHIC );

    Mtx GC_L2V;
    MTXIdentity( GC_L2V );
    GXLoadPosMtxImm( GC_L2V, GX_PNMTX5 );
    GXLoadNrmMtxImm( GC_L2V, GX_PNMTX5 );
    GXSetCurrentMtx( GX_PNMTX5 );

    GXSetNumChans    ( 1 );
    GXSetNumTexGens  ( 1 );
    GXSetNumTevStages( 1 );

    GXSetTevOp      ( GX_TEVSTAGE0, GX_MODULATE );
    GXSetTevOrder   ( GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0 );
    GXSetTexCoordGen( GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY );

    GXClearVtxDesc ();
    GXSetVtxDesc   ( GX_VA_POS,  GX_DIRECT );
    GXSetVtxDesc   ( GX_VA_CLR0, GX_DIRECT );
    GXSetVtxDesc   ( GX_VA_TEX0, GX_DIRECT );
    GXSetVtxAttrFmt( GX_VTXFMT7, GX_VA_POS,  GX_POS_XYZ,  GX_F32,   0 );
    GXSetVtxAttrFmt( GX_VTXFMT7, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0 );
    GXSetVtxAttrFmt( GX_VTXFMT7, GX_VA_TEX0, GX_TEX_ST,   GX_F32,   0 );
}

//==========================================================================

void FONT_EndRender( void )
{
    GXSetProjection( s_SavedProj, GX_PERSPECTIVE );
    GC_SetViewport( s_VPL, s_VPT, s_VPW, s_VPH );

    VRAM_Deactivate( s_FontBMP );
}

//==========================================================================

void FONT_Render( char* Str, s32 SX, s32 SY, color Color )
{
    if( Str == NULL )
        return;

    // Set the starting screen position
    f32 sx = (f32)SX;
    f32 sy = (f32)SY;

    while( *Str )
    {
        // Get the current letter in the string
        u8 letter = (u8)*Str++;

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


        #define FZ (0)

        GXBegin( GX_QUADS, GX_VTXFMT7, 4 );
        {
            GXPosition3f32( sx1, sy1, FZ );
            GXColor4u8    ( Color.R, Color.G, Color.B, Color.A );
            GXTexCoord2f32( tx1, ty1 );

            GXPosition3f32( sx2, sy1, FZ );
            GXColor4u8    ( Color.R, Color.G, Color.B, Color.A );
            GXTexCoord2f32( tx2, ty1 );

            GXPosition3f32( sx2, sy2, FZ );
            GXColor4u8    ( Color.R, Color.G, Color.B, Color.A );
            GXTexCoord2f32( tx2, ty2 );

            GXPosition3f32( sx1, sy2, FZ );
            GXColor4u8    ( Color.R, Color.G, Color.B, Color.A );
            GXTexCoord2f32( tx1, ty2 );
        }
        GXEnd();
    }

    GXFlush();
}

//==========================================================================

void FONT_RenderText ( s32 x, s32 y, char* s, xbool /*Large*/ )
{
    FONT_Render( s, x+2, y+2, color(0,0,0,180) );
    FONT_Render( s, x,   y,   color(255,255,255,255) );
}

//==========================================================================

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
