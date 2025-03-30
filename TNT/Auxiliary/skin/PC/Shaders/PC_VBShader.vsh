vs.1.1

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

// project pos
mov r2.w, c[81].w												// r0.w = 1.0f
dp4 oPos.x, r2, c[92]
dp4 oPos.y, r2, c[93]
dp4 oPos.z, r2, c[94]
dp4 oPos.w, r2, c[95]

// need to write something into oD0 here
mov r4.w, c[81].w
dp4 r5.x, r4, c[84]
dp4 r5.y, r4, c[85]
dp4 r5.z, r4, c[86]
dp4 r5.w, r4, c[87]												// r0.w = 1.0f
min r5, r5, c[83]    // clamp vector range to between 0 and 1
max r5, r5, c[82]    // clamp vector range to between 0 and 1
dp4 oD0.x, r5, c[88]
dp4 oD0.y, r5, c[89]
dp4 oD0.z, r5, c[90]
dp4 oD0.w, r5, c[91] 

// -------------------------------------------------------------------------
// uvs

mov oT0,  v4														// pass through
mad oT1.x, r4.x, c[81].y, c[81].y						// nX * 0.5 + 0.5 // scaled world space normal x
mad oT1.y, r4.y, c[81].z, c[81].y						// nX * -0.5 + 0.5 // scaled world space normal y

