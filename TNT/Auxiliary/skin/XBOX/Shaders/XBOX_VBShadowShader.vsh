xvs.1.1
// This Shader is basically the same as the player model shader, except that lighting and color are ignored and black with
// alpha level set is used instead.  This may end up using a pixel shader to handle zbuffer issues
#include "XBOX_VBShaderDefines.h"

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

// need to write something into oD0 here
// (mmm... maybe not) mov oD0, c[VSH_M_LIGHTCOL_1]

// now we need to build the shadow volume by moving the backfacing verts away from the lightsource
mov r2.w, c[VSH_CONST_1].w												// r0.w = 1.0f
//sub r3, c[VSH_M_LIGHTDIR_1], r2
//dp3 r5.x, r4.xyz, r3.xyz
//sge r5.y, r5.x, c[VSH_M_LIGHTCOL_1].x  // not actually color const in the case of shadows
//mul r5.x, r5.y, c[VSH_M_LIGHTCOL_1].y  // need to change r5.y from 0 or 1 to 0 or 10
//mul r3, c[VSH_M_LIGHTDIR_1], r5.x
//sub r2, r2, r3

// project pos
dp4 oPos.x, r2, c[VSH_M_PROJECT_1]
dp4 oPos.y, r2, c[VSH_M_PROJECT_2]
dp4 oPos.z, r2, c[VSH_M_PROJECT_3]
dp4 oPos.w, r2, c[VSH_M_PROJECT_4]
