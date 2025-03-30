#ifndef __VRAM_HPP__
#define	__VRAM_HPP__

#include "x_types.hpp"
#include "x_color.hpp"
#include "x_bitmap.hpp"

///////////////////////////////////////////////////////////////////////////

err  VRAM_InitModule    ( void );
void VRAM_KillModule    ( void );

s32  VRAM_AddTexture    ( s32 Width, s32 Height, s32 BPP, void* PixelAddr, void* ClutAddr );
void VRAM_DelTexture    ( s32 Handle );

void VRAM_Deactivate     ( x_bitmap& BMP );
void VRAM_DeactivateStage( s32 Stage = 0 );

///////////////////////////////////////////////////////////////////////////

#endif		// __VRAM_HPP__

