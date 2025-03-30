vs.1.1
// This Shader is basically the same as the player model shader, except that lighting and color are ignored and black with
// alpha level set is used instead.  This may end up using a pixel shader to handle zbuffer issues

//******************************************************************************

// -------------------------------------------------------------------------
// position

// --- BONE 1 ---
// transform by mA and scale by WeightA
mul r3.x, v2.x, c[81].x
mov	a0.x, r3.x											// a0x = imA
dp4	r1.x, v0, c[a0.x + 0]
dp4	r1.y, v0, c[a0.x + 1]
dp4	r1.z, v0, c[a0.x + 2]
mul	r2.xyz, r1.xyz, v1.x								// r0 = r1 * v1.x (Dest = Src0 * Src1)
// and now for the normals
dp3 r1.x, v3, c[a0.x + 0]
dp3 r1.y, v3, c[a0.x + 1]
dp3 r1.z, v3, c[a0.x + 2]
mul	r4.xyz, r1.xyz, v1.x								// weight the normal

// --- BONE 2 ---
// combine transform by mB and scale by WeightB
mul	r3.x, v2.y, c[81].x											// a0x = imB
mov	a0.x, r3.x											// a0x = imA
dp4	r1.x, v0, c[a0.x + 0]
dp4	r1.y, v0, c[a0.x + 1]
dp4	r1.z, v0, c[a0.x + 2]
mad	r2.xyz, r1.xyz, v1.y, r2.xyz						// r0 = r1 * v1.y + r0 (Dest = Src0 * Src1 + Src2)
// and now for the normals
dp3 r1.x, v3, c[a0.x + 0]
dp3 r1.y, v3, c[a0.x + 1]
dp3 r1.z, v3, c[a0.x + 2]
mad	r4.xyz, r1.xyz, v1.y, r4.xyz                       // weight this normal with earlier bones

// --- BONE 3 ---
// combine transform by mC and scale by WeightC
mul	r3.x, v2.z, c[81].x											// a0x = imC
mov	a0.x, r3.x											// a0x = imA
dp4	r1.x, v0, c[a0.x + 0]
dp4	r1.y, v0, c[a0.x + 1]
dp4	r1.z, v0, c[a0.x + 2]
mad	r2.xyz, r1.xyz, v1.z, r2.xyz						// r0 = r1 * v1.z + r0 (Dest = Src0 * Src1 + Src2)
// and now for the normals
dp3 r1.x, v3, c[a0.x + 0]
dp3 r1.y, v3, c[a0.x + 1]
dp3 r1.z, v3, c[a0.x + 2]
mad	r4.xyz, r1.xyz, v1.z, r4.xyz                       // weight this normal with earlier bones

// need to write something into oD0 here
// (mmm... maybe not) mov oD0, c[VSH_M_LIGHTCOL_1]

// now we need to build the shadow volume by moving the backfacing verts away from the lightsource
mov r2.w, c[81].w												// r0.w = 1.0f
//sub r3, c[84], r2
//dp3 r5.x, r4.xyz, r3.xyz
//sge r5.y, r5.x, c[88].x  // not actually color const in the case of shadows
//mul r5.x, r5.y, c[88].y  // need to change r5.y from 0 or 1 to 0 or 10
//mul r3, c[88], r5.x
//sub r2, r2, r3

// project pos
dp4 oPos.x, r2, c[92]
dp4 oPos.y, r2, c[93]
dp4 oPos.z, r2, c[94]
dp4 oPos.w, r2, c[95]
