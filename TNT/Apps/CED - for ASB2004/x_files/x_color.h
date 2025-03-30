///////////////////////////////////////////////////////////////////////////
//
//  X_COLOR.H
//
///////////////////////////////////////////////////////////////////////////

#ifndef X_COLOR_H
#define X_COLOR_H

///////////////////////////////////////////////////////////////////////////
//  GENERIC COLOR MANIPULATION MACROS
///////////////////////////////////////////////////////////////////////////

#define RGB_TO_COLOR(r,g,b)     ( 0xFF000000       | (((s32)(r))<<16) | (((s32)(g))<<8) | ((s32)(b)) )
#define RGBA_TO_COLOR(r,g,b,a)  ( (((s32)(a))<<24) | (((s32)(r))<<16) | (((s32)(g))<<8) | ((s32)(b)) )

#define COLOR_TO_A(c)           ( (u8)(((c) & 0xFF000000) >> 24) )
#define COLOR_TO_R(c)           ( (u8)(((c) & 0x00FF0000) >> 16) )
#define COLOR_TO_G(c)           ( (u8)(((c) & 0x0000FF00) >>  8) )
#define COLOR_TO_B(c)           ( (u8)(((c) & 0x000000FF) >>  0) )

#define COLOR_SET_A(c,a)        ( (c) = (((c) & 0x00FFFFFF) | (((a) & 0xFF) << 24)) )
#define COLOR_SET_R(c,r)        ( (c) = (((c) & 0xFF00FFFF) | (((r) & 0xFF) << 16)) )
#define COLOR_SET_G(c,g)        ( (c) = (((c) & 0xFFFF00FF) | (((g) & 0xFF) <<  8)) )
#define COLOR_SET_B(c,b)        ( (c) = (((c) & 0xFFFFFF00) | (((b) & 0xFF) <<  0)) )

#define COLOR_TO_RGB(c,r,g,b)       ( ((r) = COLOR_TO_R(c)),    \
                                      ((g) = COLOR_TO_G(c)),    \
                                      ((b) = COLOR_TO_B(c)) )

#define COLOR_TO_RGBA(c,r,g,b,a)    ( ((r) = COLOR_TO_R(c)),    \
                                      ((g) = COLOR_TO_G(c)),    \
                                      ((b) = COLOR_TO_B(c)),    \
                                      ((a) = COLOR_TO_A(c)) )

#define COLOR_TO_RGBU5551(c)        ( (((c)&0x00F80000) >> 8) |   \
                                      (((c)&0x0000F800) >> 5) |   \
                                      (((c)&0x000000F8) >> 2) |   \
                                      (     0x00000001      ) )

#define COLOR_TO_RGBA5551(c)        ( (((c)&0x00F80000) >> 8) |   \
                                      (((c)&0x0000F800) >> 5) |   \
                                      (((c)&0x000000F8) >> 2) |   \
                                      (((c)&0x80000000) >>31) )

#define RGB_TO_RGBU5551(r,g,b)      ( (((r)&0xF8)<<8) |         \
                                      (((g)&0xF8)<<3) |         \
                                      (((b)&0xF8)>>2) |         \
                                      (     0x01    ) )

#define RGBA_TO_RGBA5551(r,g,b,a)   ( (((r)&0xF8)<<8) |         \
                                      (((g)&0xF8)<<3) |         \
                                      (((b)&0xF8)>>2) |         \
                                      (((a)&0x80)>>7) )

#define COLOR_TO_RGBU8888(c)    ( ((c)<<8) | 0xFF )
#define RGB_TO_RGBU8888(r,g,b)  ( ((r)<<24) | ((g)<<16) | ((b)<<8) | 0xFF )

///////////////////////////////////////////////////////////////////////////
//  PREDEFINED COLORS
///////////////////////////////////////////////////////////////////////////

#define COLOR_BLACK  (0xFF000000)
#define COLOR_WHITE  (0xFFFFFFFF)
#define COLOR_RED    (0xFFFF0000)
#define COLOR_GREEN  (0xFF00FF00)
#define COLOR_BLUE   (0xFF0000FF)
#define COLOR_AQUA   (0xFF00FFFF)
#define COLOR_PURPLE (0xFFFF00FF)
#define COLOR_YELLOW (0xFFFFFF00)
#define COLOR_GREY   (0xFF808080)

///////////////////////////////////////////////////////////////////////////
//  PREDEFINED ALPHA CONSTANTS
///////////////////////////////////////////////////////////////////////////

#define ALPHA_OPAQUE        255
#define ALPHA_TRANSLUCENT   127
#define ALPHA_TRANSPARENT     0

///////////////////////////////////////////////////////////////////////////

#endif