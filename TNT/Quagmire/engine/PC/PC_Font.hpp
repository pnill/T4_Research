//=========================================================================
//
//  PC_FONT.HPP
//
//=========================================================================

#ifndef __FONT_HPP__
#define __FONT_HPP__

//-----------------------------------------------------------------------------
// Name: struct GLYPH_ATTR
// Desc: Structure to hold information about one glyph (font character image)
//-----------------------------------------------------------------------------
struct GLYPH_ATTR
{
    f32	left, top, right, bottom; // Texture coordinates for the image
    s16	wOffset;                  // Pixel offset for glyph start
    s16	wWidth;                   // Pixel width of the glyph
    s16	wAdvance;                 // Pixels to advance after the glyph
};


//=========================================================================
err  FONT_InitModule ( void );
void FONT_KillModule ( void );

void FONT_BeginRender( void );
void FONT_Render     ( char* Str, s32 X, s32 Y, color Color );
void FONT_RenderText ( s32 x, s32 y, char* s, xbool Large );
void FONT_EndRender  ( void );

//=========================================================================
#endif // __FONT_HPP__
//=========================================================================

