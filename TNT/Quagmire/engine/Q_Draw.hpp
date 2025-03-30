///////////////////////////////////////////////////////////////////////////
//
//  Q_DRAW.HPP
//
///////////////////////////////////////////////////////////////////////////

#ifndef Q_DRAW_HPP
#define Q_DRAW_HPP

///////////////////////////////////////////////////////////////////////////
//
//                            Y - up
//                            |
//                            |
//                            |
//                            |
//                            +---------X - left
//                           /
//                          /
//                         /
//                        Z - forward
//
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////

#include "x_math.hpp"
#include "x_color.hpp"
#include "Q_VRAM.hpp"

///////////////////////////////////////////////////////////////////////////
// DEFINES FOR DRAW SET MODE
///////////////////////////////////////////////////////////////////////////
//
// Values for Draw Mode specification constants.  These values are subject
// to change in future versions.
//
///////////////////////////////////////////////////////////////////////////

#define DRAW_2D             0x00000003
#define DRAW_3D             0x00000005
                                  
#define DRAW_FILL           0x00000030
#define DRAW_NO_FILL        0x00000050
                                  
#define DRAW_TEXTURE        0x00000300
#define DRAW_NO_TEXTURE     0x00000500
                                  
#define DRAW_ALPHA          0x00003000
#define DRAW_NO_ALPHA       0x00005000
                                  
#define DRAW_ZBUFFER        0x00030000
#define DRAW_NO_ZBUFFER     0x00050000

#define DRAW_CLIP           0x00300000
#define DRAW_NO_CLIP        0x00500000

#define DRAW_LIGHT          0x03000000
#define DRAW_NO_LIGHT       0x05000000


///////////////////////////////////////////////////////////////////////////
// DRAW MODE SPECIFICATION
///////////////////////////////////////////////////////////////////////////
//
// The Draw Mode is specified as a collection of Draw Mode (DRAW_...) 
// constants OR'd together.  One value is explicitly required from each 
// row.
// 
//    DRAW_2D ......... DRAW_3D
//    DRAW_FILL ....... DRAW_NO_FILL         // NO_FILL    = wireframe
//    DRAW_TEXTURE .... DRAW_NO_TEXTURE      // NO_TEXTURE = solid/Gouraud
//    DRAW_ALPHA ...... DRAW_NO_ALPHA  
//    DRAW_ZBUFFER .... DRAW_NO_ZBUFFER
//    DRAW_CLIP ....... DRAW_NO_CLIP
//    DRAW_LIGHT ...... DRAW_NO_LIGHT
// 
// For example, the following call...
//
//      DRAW_SetMode( DRAW_3D | 
//                    DRAW_FILL | 
//                    DRAW_TEXTURE | 
//                    DRAW_NO_ALPHA | 
//                    DRAW_ZBUFFER |
//                    DRAW_CLIP |
//                    DRAW_LIGHT );
//
// specifies "3D, filled, textured, opaque, z-buffered, clipped, lit" operations.
// 
// When alpha is not enabled in the draw mode, all alpha values given in
// colors will be ignored.  If you are not going to need alpha, then it 
// should be disabled.  This may result in slightly better performance.
// 
// The draw mode remains in effect until it is either reset or the current
// render mode (see ENG_EndRenderMode) is concluded.  All DRAW_ functions
// which cause "drawing" to occur require that the draw mode is defined.
//
///////////////////////////////////////////////////////////////////////////

void DRAW_SetMode       ( u32 DrawMode );

///////////////////////////////////////////////////////////////////////////
// TEXTURE DESIGNATION
///////////////////////////////////////////////////////////////////////////
//
// Specifies a texture to use on all subsequent applicable operations.  
// The texture/bitmap provided will be used as a sprite or texture as needed.
// A value of NULL for the texture specifies "no texture". The bActivate
// flag specifies whether to call VRAM_Activate on the Texture.
//
///////////////////////////////////////////////////////////////////////////
void DRAW_SetTexture    ( x_bitmap* Texture, xbool bActivate = TRUE );

///////////////////////////////////////////////////////////////////////////
// LOCAL TO WORLD 3D TRANSFORMATION
///////////////////////////////////////////////////////////////////////////
//
// Specifies a Local to World matrix to be used on all subsequent 
// applicable operations.  The NormalL2W contains only the rotations from
// local to world.
//
///////////////////////////////////////////////////////////////////////////

void DRAW_SetL2W        ( matrix4* VertexL2W, matrix4* NormalL2W );

void DRAW_GetL2W        ( matrix4& VertexL2W );

///////////////////////////////////////////////////////////////////////////
// PRIMITIVE RENDERING FUNCTIONS
///////////////////////////////////////////////////////////////////////////

void DRAW_Points        ( s32       NVerts, 
                          vector3*  Pos, 
                          color*    Color   // Can be NULL
                        );

void DRAW_Lines         ( s32       NVerts, 
                          vector3*  Pos, 
                          color*    Color   // Can be NULL
                        );

void DRAW_LineStrip     ( s32       NVerts, 
                          vector3*  Pos, 
                          color*    Color   // Can be NULL
                        );

void DRAW_Triangles     ( s32       NVerts, 
                          vector3*  Pos, 
                          color*    Color,  // Can be NULL
                          vector2*  UV,     // Can be NULL
                          vector3*  Normal  // Can be NULL
                        );

void DRAW_TriangleStrip ( s32       NVerts, 
                          vector3*  Pos, 
                          color*    Color,  // Can be NULL
                          vector2*  UV,     // Can be NULL
                          vector3*  Normal  // Can be NULL
                        );

void DRAW_TriangleFan   ( s32       NVerts, 
                          vector3*  Pos, 
                          color*    Color,  // Can be NULL
                          vector2*  UV,     // Can be NULL
                          vector3*  Normal  // Can be NULL
                        );

void DRAW_Triangles2D   ( s32 NVerts,
                          vector3* Pos,
                          color* Color,
                          vector2* UV 
                        );

///////////////////////////////////////////////////////////////////////////
// PRIMITIVES
///////////////////////////////////////////////////////////////////////////

void DRAW_Sprite        ( f32 X, f32 Y, f32 Z,    // Hot spot (2D Left-Top), (3D Center)
                          f32 W, f32 H,           // (2D pixel W&H), (3D World W&H)
                          color  Color );         // 

void DRAW_SpriteUV      ( f32 X,  f32 Y, f32 Z,   // Hot spot (2D Left-Top), (3D Center)
                          f32 W,  f32 H,          // (2D pixel W&H), (3D World W&H)
                          f32 U0, f32 V0,         // Upper Left   UV  [0.0 - 1.0]
                          f32 U1, f32 V1,         // Bottom Right UV  [0.0 - 1.0]
                          color   Color );        // 

void DRAW_Rectangle     ( f32 X, f32 Y, f32 Z,    // Hot spot (2D Left-Top), (3D Center)
                          f32 W, f32 H,           // (2D pixel W&H), (3D World W&H)
                          color  TLColor,         // Top Left     color
                          color  TRColor,         // Top Right    color
                          color  BLColor,         // Bottom Left  color
                          color  BRColor );       // Bottom Right color


////////////////////////////////////////////////////////////////////////////
//  ADVANCED DRAWING
////////////////////////////////////////////////////////////////////////////
//
//  DRAW_BeginTriangles()
//      Calling DRAW_BeginTriangles is optional. If called outside of a Begin/End sequence, 
//      the draw functions will internally call Begin and End. To avoid extra 
//      overhead, this method should be used if more than one DRAW_Triangles 
//      function will be called successively.
//
//      While inside a Begin/End sequence you cannot change draw settings or 
//      the active texture.
//
//  DRAW_Begin2DTriangles()
//      Same as DRAW_BeginTriangles except strictly for 2D triangles
//
//  DRAW_EndTriangles() 
//      Called after you have completed your calls to DRAW_Triangles.  This
//      function must have a matching call to DRAW_BeginTriangles
//
//  DRAW_BeginSprite()
//      Calling DRAW_BeginSprite is optional. If called outside of a Begin/End sequence, 
//      the draw functions will internally call Begin and End. To avoid extra 
//      overhead, this method should be used if more than one DRAW_Sprite 
//      function will be called successively.
//
//      While inside a Begin/End sequence you cannot change draw settings or 
//      the active texture.
//
//  DRAW_EndSprite() 
//      Called after you have completed your calls to DRAW_Sprite.  This
//      function must have a matching call to DRAW_BeginSprite
//
////////////////////////////////////////////////////////////////////////////

void DRAW_BeginTriangles    ( void );
void DRAW_Begin2DTriangles  ( void );
void DRAW_EndTriangles      ( void );

void DRAW_BeginSprite       ( void );
void DRAW_EndSprite         ( void );


////////////////////////////////////////////////////////////////////////////
// STAT TRACKING
////////////////////////////////////////////////////////////////////////////

void DRAW_SetStatTrackers( s32* pNVerts, s32* pNTris, s32* pNBytes );


///////////////////////////////////////////////////////////////////////////
// END
///////////////////////////////////////////////////////////////////////////

#endif
