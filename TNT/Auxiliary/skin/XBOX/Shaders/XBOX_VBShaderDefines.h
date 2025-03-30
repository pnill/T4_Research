// XBOX Shader Definitions
#define VSH_V_POS                v0
#define VSH_V_MATRIX_WEIGHT      v1
#define VSH_V_MATRIX_INDEX       v2
#define VSH_V_NORMAL             v3
#define VSH_V_UV1                v4

#define VSH_M_BONE_CACHE_START_1 0
#define VSH_M_BONE_CACHE_START_2 1
#define VSH_M_BONE_CACHE_START_3 2
//#define VSH_M_BONE_CACHE_START_4 3
// Little note about our shader constants:
// Currently, there are 23 bones on the main player model and we only need the first three vectors in those bone
// matrices.  This means we'll use 69 constant slots (0-68).  The other slots used are defined below.  If you need
// to add new constants, you will have to stay above 68.

#define VSH_CONST_1              76
#define VSH_CONST_MIN            77
#define VSH_CONST_MAX            78
#define VSH_V_LIGHTPOS           79
#define VSH_M_SHADOWTEX_1        80   // DO NOT CHANGE!  NON-SKIN CODE USES THIS LOCATION, BUT NOT THIS
#define VSH_M_SHADOWTEX_2        81   // DO NOT CHANGE!  HEADER FILE.
#define VSH_M_SHADOWTEX_3        82   // DO NOT CHANGE! 
#define VSH_M_SHADOWTEX_4        83   // DO NOT CHANGE!
#define VSH_M_LIGHTDIR_1         84
#define VSH_M_LIGHTDIR_2         85
#define VSH_M_LIGHTDIR_3         86
#define VSH_M_LIGHTDIR_4         87
#define VSH_M_LIGHTCOL_1         88
#define VSH_M_LIGHTCOL_2         89
#define VSH_M_LIGHTCOL_3         90
#define VSH_M_LIGHTCOL_4         91
#define VSH_M_PROJECT_1          92
#define VSH_M_PROJECT_2          93
#define VSH_M_PROJECT_3          94
#define VSH_M_PROJECT_4          95
