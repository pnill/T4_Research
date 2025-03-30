xvs.1.1

#include "XBOX_GeomShaderDefines.h"

// Basically the same shader as the normal object shader, except for color/texture
//******************************************************************************

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
dp4 oPos.x, r2, c[VSH_M_PROJECT_1]
dp4 oPos.y, r2, c[VSH_M_PROJECT_2]
dp4 oPos.z, r2, c[VSH_M_PROJECT_3]
dp4 oPos.w, r2, c[VSH_M_PROJECT_4]

//mov oD0, c[VSH_M_LIGHTCOL_1]
