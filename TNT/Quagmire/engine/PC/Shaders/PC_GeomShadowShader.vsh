vs.1.1

// Basically the same shader as the normal object shader, except for color/texture
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

//mov oD0, c[7]
