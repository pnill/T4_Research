xvs.1.1

#include "XBOX_GeomShaderDefines.h"

// This shader is similar to the Skinning shader, except that we don't use any bones
//******************************************************************************

#define Start	c[VSH_CONST_FOG].x
#define Depth	c[VSH_CONST_FOG].y
#define MaxFog  c[VSH_CONST_FOG].z
#define Density c[VSH_CONST_FOG].w
#define	one		c[VSH_CONST_1].w
#define View    r8
#define	tmp		r10

// -------------------------------------------------------------------------
// position

dp4 r2.x, VSH_V_POS, c[VSH_M_MOVE_1]
dp4 r2.y, VSH_V_POS, c[VSH_M_MOVE_2]
dp4 r2.z, VSH_V_POS, c[VSH_M_MOVE_3]
mov r2.w, c[VSH_CONST_1].w

// transform normals
dp3 r3.x, VSH_V_NORMAL, c[VSH_M_MOVE_1]
dp3 r3.y, VSH_V_NORMAL, c[VSH_M_MOVE_2]
dp3 r3.z, VSH_V_NORMAL, c[VSH_M_MOVE_3]

// project pos
dp4 r8.x, r2, c[VSH_M_PROJECT_1]
dp4 r8.y, r2, c[VSH_M_PROJECT_2]
dp4 r8.z, r2, c[VSH_M_PROJECT_3]
dp4 r8.w, r2, c[VSH_M_PROJECT_4]
mov oPos, r8

// need to write something into oD0 here
mov r3.w, c[VSH_CONST_1].w
dp4 r4.x, r3, c[VSH_M_LIGHTDIR_1]
dp4 r4.y, r3, c[VSH_M_LIGHTDIR_2]
dp4 r4.z, r3, c[VSH_M_LIGHTDIR_3]
dp4 r4.w, r3, c[VSH_M_LIGHTDIR_4]												// r0.w = 1.0f
min r4, r4, c[VSH_CONST_MAX]    // clamp vector range to between 0 and 1
max r4, r4, c[VSH_CONST_MIN]    // clamp vector range to between 0 and 1
dp4 oD0.x, r4, c[VSH_M_LIGHTCOL_1]
dp4 oD0.y, r4, c[VSH_M_LIGHTCOL_2]
dp4 oD0.z, r4, c[VSH_M_LIGHTCOL_3]
dp4 oD0.w, r4, c[VSH_M_LIGHTCOL_4] 

//==== calculate fog
dp3		tmp.w,	View,	View
rsq		tmp.w,	tmp.w
rcp		tmp.x,	tmp.w
// tmp.x = distance

add		tmp.x,	-Start,	tmp.x
mad		tmp.x,	tmp.x,	Density, Depth
// tmp.x = old_depth + ((distance - fog_start) / Density)

// Clip the fog (.z == minimum visibility value allowed)
min		tmp.x,	MaxFog,	tmp.x

// oFog = 1 - fog
add		oFog.x,	one,	-tmp.x



// -------------------------------------------------------------------------
// uvs

mov oT0,  VSH_V_UV1														// pass through
mad oT1.x, r4.x, c[VSH_CONST_1].y, c[VSH_CONST_1].y						// nX * 0.5 + 0.5 // scaled world space normal x
mad oT1.y, r4.y, c[VSH_CONST_1].z, c[VSH_CONST_1].y						// nX * -0.5 + 0.5 // scaled world space normal y
