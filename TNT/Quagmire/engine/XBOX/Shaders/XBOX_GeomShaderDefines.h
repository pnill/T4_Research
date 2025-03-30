// XBOX Geom Shader Definitions
#define VSH_V_POS               v0
#define VSH_V_COLOR             v5
#define VSH_V_NORMAL            v1
#define VSH_V_UV1               v2
#define VSH_V_UV2               v3
#define VSH_V_UV3               v4

#define VSH_CONST_1             0
#define VSH_CONST_MIN           1
#define VSH_CONST_MAX           2
#define VSH_M_LIGHTDIR_1        3
#define VSH_M_LIGHTDIR_2        4
#define VSH_M_LIGHTDIR_3        5
#define VSH_M_LIGHTDIR_4        6
#define VSH_M_LIGHTCOL_1        7
#define VSH_M_LIGHTCOL_2        8
#define VSH_M_LIGHTCOL_3        9
#define VSH_M_LIGHTCOL_4        10
#define VSH_M_PROJECT_1         11
#define VSH_M_PROJECT_2         12
#define VSH_M_PROJECT_3         13
#define VSH_M_PROJECT_4         14
#define VSH_M_MOVE_1            15
#define VSH_M_MOVE_2            16
#define VSH_M_MOVE_3            17
#define VSH_M_MOVE_4            18
#define VSH_CONST_FOG           19
#define VSH_V_WORLD_EYE         20
#define VSH_M_L2W_1             21  // mLocalToWorld (note: this is a 4x3)
#define VSH_M_L2W_2             22
#define VSH_M_L2W_3             23
#define VSH_V_LIGHTPOS          79  // DO NOT CHANGE! These values are based on the values defined
#define VSH_M_SHADOWTEX_1       80  // DO NOT CHANGE! in the skinning shader header.
#define VSH_M_SHADOWTEX_2       81  // DO NOT CHANGE! Also used in game code (AVMan) but doesn't
#define VSH_M_SHADOWTEX_3       82  // DO NOT CHANGE! use the define... so please don't change them.
#define VSH_M_SHADOWTEX_4       83  // DO NOT CHANGE!
