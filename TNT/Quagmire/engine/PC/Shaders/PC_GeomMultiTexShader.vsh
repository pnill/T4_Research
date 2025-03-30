vs.1.1

// -------------------------------------------------------------------------
// position

mov r2, v0

// project pos
dp4 oPos.x, r2, c[11]
dp4 oPos.y, r2, c[12]
dp4 oPos.z, r2, c[13]
dp4 oPos.w, r2, c[14]

// need to write something into oD0 here
mov oD0, v5

// -------------------------------------------------------------------------
// uvs

mov oT0,  v2														// pass through
mov oT1,  v3
mul oT2,  v2, c[0].x
