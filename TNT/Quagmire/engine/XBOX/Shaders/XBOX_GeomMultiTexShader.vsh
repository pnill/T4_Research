xvs.1.1

#include "XBOX_GeomShaderDefines.h"

#define Start   c[VSH_CONST_FOG].x
#define Depth   c[VSH_CONST_FOG].y
#define MaxFog  c[VSH_CONST_FOG].z
#define Density c[VSH_CONST_FOG].w
#define one     c[VSH_CONST_1].w
#define View    r8
#define tmp     r10

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
dp3     tmp.w,  View,   View
rsq     tmp.w,  tmp.w
rcp     tmp.x,  tmp.w
// tmp.x = distance

add     tmp.x,  -Start, tmp.x
mad     tmp.x,  tmp.x,  Density, Depth
// tmp.x = old_depth + ((distance - fog_start) / Density)

// Clip the fog (.z == minimum visibility value allowed)
min     tmp.x,  MaxFog, tmp.x

// oFog = 1 - fog
add     oFog.x, one,    -tmp.x


// need to write something into oD0 here
mov oD0, VSH_V_COLOR
             
// -------------------------------------------------------------------------
// uvs
// we have a Diffuse input and Diffuse+ShadowBuffer output

mov oT0,  VSH_V_UV1                                                     // pass through
//mov oT1, VSH_V_UV1

;vertex->shadowbuffer texcoords
;dp4 oT2.x, r2, c[VSH_M_SHADOWTEX_1]
;dp4 oT2.y, r2, c[VSH_M_SHADOWTEX_2]
;dp4 oT2.z, r2, c[VSH_M_SHADOWTEX_3]
;dp4 r0.w,  r2, c[VSH_M_SHADOWTEX_4]

;clamp w (q) to 0
;max r0.w, r0.w, c[VSH_CONST_MIN].w
;mov oT2.w, r0.w





// -------------------------------------------------------------------------
// uvs
// old method - assumed 3 input uv pairs - diffuse+lightmap+shadow
//      shadow coords were generated, but stream had room for uv anyway

;mov oT0,  VSH_V_UV1                                                        // pass through
;mov oT1,  VSH_V_UV2

;vertex->shadowbuffer texcoords
;dp4 oT2.x, r2, c[VSH_M_SHADOWTEX_1]
;dp4 oT2.y, r2, c[VSH_M_SHADOWTEX_2]
;dp4 oT2.z, r2, c[VSH_M_SHADOWTEX_3]
;dp4 r0.w,  r2, c[VSH_M_SHADOWTEX_4]

;clamp w (q) to 0
;max r0.w, r0.w, c[VSH_CONST_MIN].w
;mov oT2.w, r0.w
