vs.1.1

// This shader is similar to the Skinning shader, except that we don't use any bones
//******************************************************************************

// -------------------------------------------------------------------------
// position

dp4 r2.x, v0, c[15]
dp4 r2.y, v0, c[16]
dp4 r2.z, v0, c[17]
mov r2.w, c[0].w

// transform normals
dp3 r3.x, v1, c[15]
dp3 r3.y, v1, c[16]
dp3 r3.z, v1, c[17]

// project pos
dp4 oPos.x, r2, c[11]
dp4 oPos.y, r2, c[12]
dp4 oPos.z, r2, c[13]
dp4 oPos.w, r2, c[14]

// need to write something into oD0 here
mov r3.w, c[0].w
dp4 r4.x, r3, c[3]
dp4 r4.y, r3, c[4]
dp4 r4.z, r3, c[5]
dp4 r4.w, r3, c[6]												// r0.w = 1.0f
min r4, r4, c[2]    // clamp vector range to between 0 and 1
max r4, r4, c[1]    // clamp vector range to between 0 and 1
dp4 oD0.x, r4, c[7]
dp4 oD0.y, r4, c[8]
dp4 oD0.z, r4, c[9]
dp4 oD0.w, r4, c[10] 

// -------------------------------------------------------------------------
// uvs

mov oT0,  v2														// pass through
mad oT1.x, r4.x, c[0].y, c[0].y						// nX * 0.5 + 0.5 // scaled world space normal x
mad oT1.y, r4.y, c[0].z, c[0].y						// nX * -0.5 + 0.5 // scaled world space normal y
