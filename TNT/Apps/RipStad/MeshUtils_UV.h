#ifndef __MESHUTILS_UV_H_INCLUDED__
#define __MESHUTILS_UV_H_INCLUDED__

#include "AsciiMesh.h"

void NormalizeUVs( mesh::chunk& Chunk );

xbool IsUVsNormalized( vector2* pUV, s32 nUVs );
void AdjustUV( vector2* pUV, s32 nUVs );

#endif