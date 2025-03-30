xvs.1.1

//#define BLEND_NORMALS
#include "XBOX_VBShaderDefines.h"

def c[ 75 ], 0.3f, 0.75f, 0.11f, 1.0f

//******************************************************************************

// -------------------------------------------------------------------------
// position

// --- BONE 1 ---
// transform by mA and scale by WeightA
mul r3.x, VSH_V_MATRIX_INDEX.x, c[VSH_CONST_1].x
mov	a0.x, r3.x											// a0x = imA
dp4	r1.x, VSH_V_POS, c[a0.x + VSH_M_BONE_CACHE_START_1]
dp4	r1.y, VSH_V_POS, c[a0.x + VSH_M_BONE_CACHE_START_2]
dp4	r1.z, VSH_V_POS, c[a0.x + VSH_M_BONE_CACHE_START_3]
mul	r2.xyz, r1.xyz, VSH_V_MATRIX_WEIGHT.x								// r0 = r1 * v1.x (Dest = Src0 * Src1)
// and now for the normals
dp3 r1.x, VSH_V_NORMAL, c[a0.x + VSH_M_BONE_CACHE_START_1]
dp3 r1.y, VSH_V_NORMAL, c[a0.x + VSH_M_BONE_CACHE_START_2]
dp3 r1.z, VSH_V_NORMAL, c[a0.x + VSH_M_BONE_CACHE_START_3]
mul	r4.xyz, r1.xyz, VSH_V_MATRIX_WEIGHT.x								// weight the normal

// --- BONE 2 ---
// combine transform by mB and scale by WeightB
mul	r3.x, VSH_V_MATRIX_INDEX.y, c[VSH_CONST_1].x											// a0x = imB
mov	a0.x, r3.x											// a0x = imA
dp4	r1.x, VSH_V_POS, c[a0.x + VSH_M_BONE_CACHE_START_1]
dp4	r1.y, VSH_V_POS, c[a0.x + VSH_M_BONE_CACHE_START_2]
dp4	r1.z, VSH_V_POS, c[a0.x + VSH_M_BONE_CACHE_START_3]
mad	r2.xyz, r1.xyz, VSH_V_MATRIX_WEIGHT.y, r2.xyz						// r0 = r1 * v1.y + r0 (Dest = Src0 * Src1 + Src2)
// and now for the normals
dp3 r1.x, VSH_V_NORMAL, c[a0.x + VSH_M_BONE_CACHE_START_1]
dp3 r1.y, VSH_V_NORMAL, c[a0.x + VSH_M_BONE_CACHE_START_2]
dp3 r1.z, VSH_V_NORMAL, c[a0.x + VSH_M_BONE_CACHE_START_3]
mad	r4.xyz, r1.xyz, VSH_V_MATRIX_WEIGHT.y, r4.xyz                       // weight this normal with earlier bones

// --- BONE 3 ---
// combine transform by mC and scale by WeightC
mul	r3.x, VSH_V_MATRIX_INDEX.z, c[VSH_CONST_1].x											// a0x = imC
mov	a0.x, r3.x											// a0x = imA
dp4	r1.x, VSH_V_POS, c[a0.x + VSH_M_BONE_CACHE_START_1]
dp4	r1.y, VSH_V_POS, c[a0.x + VSH_M_BONE_CACHE_START_2]
dp4	r1.z, VSH_V_POS, c[a0.x + VSH_M_BONE_CACHE_START_3]
mad	r2.xyz, r1.xyz, VSH_V_MATRIX_WEIGHT.z, r2.xyz						// r0 = r1 * v1.z + r0 (Dest = Src0 * Src1 + Src2)
// and now for the normals
dp3 r1.x, VSH_V_NORMAL, c[a0.x + VSH_M_BONE_CACHE_START_1]
dp3 r1.y, VSH_V_NORMAL, c[a0.x + VSH_M_BONE_CACHE_START_2]
dp3 r1.z, VSH_V_NORMAL, c[a0.x + VSH_M_BONE_CACHE_START_3]
mad	r4.xyz, r1.xyz, VSH_V_MATRIX_WEIGHT.z, r4.xyz                       // weight this normal with earlier bones

// project pos
mov r2.w, c[VSH_CONST_1].w												// r0.w = 1.0f
dp4 oPos.x, r2, c[VSH_M_PROJECT_1]
dp4 oPos.y, r2, c[VSH_M_PROJECT_2]
dp4 oPos.z, r2, c[VSH_M_PROJECT_3]
dp4 oPos.w, r2, c[VSH_M_PROJECT_4]

// need to write something into oD0 here
mov r4.w, c[VSH_CONST_1].w
dp4 r5.x, r4, c[VSH_M_LIGHTDIR_1]
dp4 r5.y, r4, c[VSH_M_LIGHTDIR_2]
dp4 r5.z, r4, c[VSH_M_LIGHTDIR_3]
dp4 r5.w, r4, c[VSH_M_LIGHTDIR_4]												// r0.w = 1.0f
min r5, r5, c[VSH_CONST_MAX]    // clamp vector range to between 0 and 1
max r5, r5, c[VSH_CONST_MIN]    // clamp vector range to between 0 and 1
dp4 r1.x, r5, c[VSH_M_LIGHTCOL_1]
dp4 r1.y, r5, c[VSH_M_LIGHTCOL_2]
dp4 r1.z, r5, c[VSH_M_LIGHTCOL_3]
dp4 r1.w, r5, c[VSH_M_LIGHTCOL_4] 
mov oD0, r1

//mul r1.x, r1.x, c[ 75 ].y

mov r6.x, c[ VSH_M_LIGHTDIR_1 ].z
mov r6.y, c[ VSH_M_LIGHTDIR_2 ].z
mov r6.z, c[ VSH_M_LIGHTDIR_3 ].z
dp3 r1.z, r4, r6

//sub r1.x, c[ VSH_CONST_MAX ].x, r1.x
//mul r1.x, r1.x, r1.x
mov oT3.x, r1.x
mov oT3.y, r1.z

// -------------------------------------------------------------------------
// uvs

mov oT0,  VSH_V_UV1														// pass through
mad oT1.x, r4.x, c[VSH_CONST_1].y, c[VSH_CONST_1].y						// nX * 0.5 + 0.5 // scaled world space normal x
mad oT1.y, r4.y, c[VSH_CONST_1].z, c[VSH_CONST_1].y						// nX * -0.5 + 0.5 // scaled world space normal y

;vertex->shadowbuffer texcoords
dp4 oT2.x, r2, c[VSH_M_SHADOWTEX_1]
dp4 oT2.y, r2, c[VSH_M_SHADOWTEX_2]
dp4 oT2.z, r2, c[VSH_M_SHADOWTEX_3]
dp4 r0.w,  r2, c[VSH_M_SHADOWTEX_4]

;clamp w (q) to 0
max r0.w, r0.w, c[VSH_CONST_MIN].w
mov oT2.w, r0.w

