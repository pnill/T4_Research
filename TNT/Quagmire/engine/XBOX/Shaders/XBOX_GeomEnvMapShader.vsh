xvs.1.1

#include "XBOX_GeomShaderDefines.h"

#define Start	c[VSH_CONST_FOG].x
#define Depth	c[VSH_CONST_FOG].y
#define MaxFog  c[VSH_CONST_FOG].z
#define Density c[VSH_CONST_FOG].w
#define	one		c[VSH_CONST_1].w
#define View    r8
#define	tmp		r10

// -------------------------------------------------------------------------
// position

mov r2, VSH_V_POS

// project pos
dp4 r8.x, r2, c[VSH_M_PROJECT_1]
dp4 r8.y, r2, c[VSH_M_PROJECT_2]
dp4 r8.z, r2, c[VSH_M_PROJECT_3]
dp4 r8.w, r2, c[VSH_M_PROJECT_4]
mov oPos, r8

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


// need to write something into oD0 here
mov oD0, VSH_V_COLOR
             

// -------------------------------------------------------------------------
// uvs

mov oT0,  VSH_V_UV1														// pass through

//vertex->shadowbuffer texcoords
;dp4 oT2.x, r2, c[VSH_M_SHADOWTEX_1]
;dp4 oT2.y, r2, c[VSH_M_SHADOWTEX_2]
;dp4 oT2.z, r2, c[VSH_M_SHADOWTEX_3]
;dp4 r0.w,  r2, c[VSH_M_SHADOWTEX_4]

//clamp w (q) to 0
;max r0.w, r0.w, c[VSH_CONST_MIN].w
;mov oT2.w, r0.w

// -------------------------------------------------------------------------
// environment

// world norm
dp3 r1.x, VSH_V_NORMAL, c[VSH_M_L2W_1]
dp3 r1.y, VSH_V_NORMAL, c[VSH_M_L2W_2]
dp3 r1.z, VSH_V_NORMAL, c[VSH_M_L2W_3]
dp3 r1.w, r1, r1
rsq r1.w, r1.w
mul r1, r1, r1.w

// world vert
dp4 r0.x, VSH_V_POS, c[VSH_M_L2W_1]
dp4 r0.y, VSH_V_POS, c[VSH_M_L2W_2]
dp4 r0.z, VSH_V_POS, c[VSH_M_L2W_3]
mov r0.w, c[VSH_CONST_1].w

// eye vector
add r3, r0, -c[VSH_V_WORLD_EYE]
dp3 r3.w, r3, r3
rsq r3.w, r3.w
mul r3, r3, r3.w

// reflection
dp3 r4.w, r1, r3
add r4.w, r4.w, r4.w
mad r4.xyz, r4.w, -r1.xyz, r3.xyz

// sphere env
mad oT0.x, r4.x, c[VSH_CONST_1].y, c[VSH_CONST_1].y
mad oT0.y, -r4.y, c[VSH_CONST_1].y, c[VSH_CONST_1].y

