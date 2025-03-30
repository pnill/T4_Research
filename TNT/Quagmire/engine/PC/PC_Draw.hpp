#ifndef PC_DRAW_HPP
#define PC_DRAW_HPP

#include "x_types.hpp"

err   DRAW_InitModule( void );
void  DRAW_KillModule( void );
void  DRAW_SetPCTextureStages( void );
xbool DRAW_IsMode2D( void );
void  DRAW_Build2DProjectionMatrix( void );
void  DRAW_RestoreSavedProjectionMatrix( void );

#endif