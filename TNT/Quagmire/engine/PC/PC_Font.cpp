//=========================================================================
//
//  PC_FONT.CPP
//
//=========================================================================
#include "Q_Draw.hpp"
#include "Q_Engine.hpp"
#include "Q_PC.hpp"
#include "Q_VRAM.hpp"
#include "x_files.hpp"
#include "AUX_Bitmap.hpp"
#include "PC_Font.hpp"
#include "PC_Video.hpp"


//=========================================================================

extern unsigned char FontData[];
extern unsigned char FontGlyph[];

struct FONT2DVERTEX
{
	D3DXVECTOR4	p;
	u32			color;
	f32			tu, tv;
};

#define D3DFVF_FONT2DVERTEX ( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 )

// Max size for the font class' vertex buffer
#define FONT_MAX_VERTICES 50*6



// Font texture
static x_bitmap					s_FontBMP;
static LPDIRECT3DVERTEXBUFFER8	s_pFontVB = NULL; // Buffer to hold vertices


// Font and texture dimensions
static u32			s_dwFontHeight;
static u32			s_dwTexWidth;
static u32			s_dwTexHeight;

// Unicode ranges
static char			s_cLowChar;
static char			s_cHighChar;

// Glyph data for the font
static u32			s_dwNumGlyphs;
static GLYPH_ATTR*	s_Glyphs;

//=========================================================================

inline FONT2DVERTEX InitFont2DVertex( const D3DXVECTOR4& p, u32 color, f32 tu, f32 tv )
{
    FONT2DVERTEX v;
	v.p = p;
	v.color = color;
	v.tu = tu;
	v.tv = tv;
    return v;
}

//=========================================================================

err FONT_InitModule( void )
{
    s_FontBMP.SetupBitmap( x_bitmap::FMT_32_ABGR_8888,
                           128, 256,
                           FALSE,
                           FontData,
                           NULL,
                           0 );
    
    AUXBMP_ConvertToPC( s_FontBMP );

    VRAM_Register( s_FontBMP );

    // Confirm size and format
    ASSERT( s_FontBMP.GetWidth() == 128 );
    ASSERT( s_FontBMP.GetHeight() == 256 );

	DXCHECK(g_pD3DDevice->CreateVertexBuffer(FONT_MAX_VERTICES * sizeof(FONT2DVERTEX),
                                            D3DUSAGE_DYNAMIC, 0L,
                                            D3DPOOL_DEFAULT, &s_pFontVB ));
	ASSERT(s_pFontVB);


    // Read header
	u32	*pGlyph32;
	u8	*pGlyph8;

    u32 dwVersion, dwBPP;
	
	pGlyph32 = (u32*)FontGlyph;

    dwVersion = *pGlyph32++;
    s_dwFontHeight = *pGlyph32++;
    s_dwTexWidth = *pGlyph32++;
    s_dwTexHeight = *pGlyph32++;
    dwBPP = *pGlyph32++;

    // Check version of file (to make sure it matches up with the FontMaker tool)
    if( dwVersion != 0x00000004 )
    {
        x_printf("Font: Incorrect version number on font glyph data!\n");
        return ERR_FAILURE;
    }

    // Read the low and high char
	pGlyph8 = (u8*)pGlyph32;

    s_cLowChar = *pGlyph8++;
    s_cHighChar = *pGlyph8++;


    // Read the glyph attributes from the file
	pGlyph32 = (u32*)pGlyph8;

    s_dwNumGlyphs = *pGlyph32++;
    s_Glyphs = new GLYPH_ATTR[s_dwNumGlyphs];
	ASSERT(s_Glyphs);

    x_memcpy( s_Glyphs, (void*)pGlyph32, s_dwNumGlyphs*sizeof(GLYPH_ATTR)); 

	return (ERR_SUCCESS);
}

//=========================================================================

void FONT_KillModule( void )
{
	VRAM_UnRegister(s_FontBMP);
    s_FontBMP.KillBitmap();

	delete[] s_Glyphs;

	SAFE_RELEASE(s_pFontVB);
}


//=========================================================================
void FONT_BeginRender( void )
{
	if (s_pFontVB == NULL)
		return;

    VRAM_Activate(s_FontBMP);

    DXWARN(g_pD3DDevice->SetStreamSource( 0, s_pFontVB, sizeof(FONT2DVERTEX) ));
    DXWARN(g_pD3DDevice->SetVertexShader( D3DFVF_FONT2DVERTEX ));

    ENG_SetRenderFlags( ENG_ZBUFFER_TEST_OFF | ENG_ZBUFFER_FILL_OFF | ENG_ALPHA_TEST_OFF | ENG_ALPHA_BLEND_ON );
    ENG_SetBlendMode( ENG_BLEND_NORMAL );

    DXWARN(g_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE ));
    DXWARN(g_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ));
    DXWARN(g_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE ));

    DXWARN(g_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE ));
    DXWARN(g_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ));
    DXWARN(g_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE ));
    
    DXWARN(g_pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE ));
    DXWARN(g_pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE ));
}

//=========================================================================
void FONT_EndRender( void )
{
	VRAM_Deactivate( s_FontBMP );
}

//=========================================================================
void FONT_Render( char* Str, s32 SX, s32 SY, color Color )
{
    // Set the starting screen position
    f32 sx = (f32)SX;
    f32 sy = (f32)SY;

	u32 dwColor = Color.Get();

    // Fill vertex buffer
    FONT2DVERTEX* pVertices;
    u32         dwNumTriangles = 0;
    s_pFontVB->Lock( 0, 0, (BYTE**)&pVertices, D3DLOCK_DISCARD );

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
        if( letter<s_cLowChar || letter>s_cHighChar )
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
            DXWARN(g_pD3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, dwNumTriangles ));
            s_pFontVB->Lock( 0, 0, (BYTE**)&pVertices, 0L );
            dwNumTriangles = 0L;
        }
    }

    // If there's any vertices left in the vertex buffer, render it
    s_pFontVB->Unlock();
    if( dwNumTriangles > 0 )
        DXWARN(g_pD3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, dwNumTriangles ));

}




//=========================================================================
void FONT_RenderText( s32 x, s32 y, char* s, xbool Large )
{
    FONT_Render( s, x, y, color(255,255,255,255) );
}

//=========================================================================


