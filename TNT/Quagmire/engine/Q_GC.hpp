////////////////////////////////////////////////////////////////////////////
//
// Q_GC.hpp
//
// GameCube specific engine support
//
////////////////////////////////////////////////////////////////////////////

#ifndef Q_GC_HPP_INCLUDED
#define Q_GC_HPP_INCLUDED


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_math.hpp"
#include "x_color.hpp"


////////////////////////////////////////////////////////////////////////////
// GAMECUBE ENGINE SPECIFIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////
//
// GC_GetFixedAlpha     - Returns if fixed alpha is on, and what the alpha value is(0x00 - 0xFF).
//                        Used in rendering to setup the Tev with correct alpha.
//
// GC_SetZMode          - Turn Z-buffer test and fill on or off
//
// GC_SetAlphaBlending  - Sets the engine blend mode, using ENG_BLEND_* flags. The ForceChange argument
//                        will cause the function to ignore the check for BlendFlags == CurrentBlendFlags
//                        (to save on needless state changing).
//
// GC_SetAlphaTest      - Turns the alpha test on or off
//
// GC_GetProjMatrix     - Gets the current projection matrix, which can be restored with GXSetProjection().
//
// GC_SetViewport       - Sets the engine viewport. This function is for internal engine use.
//
// GC_EnableLighting    - Turns the lighting on or off, specifying VtxColors with TRUE and enabling lighting
//                        will use the vertex color data as material colors when computing final per-vertex color.
//
// GC_SetupLighting     - Takes the current lighting info and sets up the GameCube hardware to use it.  This
//                        function is for internal engine use.
//
// GC_UpdateLights      - If the viewpoint changes, the lights need to be updated to match the new view-space coord.
//                        This function is for internal engine use.
//
// GC_SetEFBClear       - If set to TRUE, embedded frame buffer is cleared to engine background color otherwise
//                        contents of the EFB are left unchanged.  TRUE is the default setting.
//
// GC_EnableRefPlane    - Enables a reference plane so all subsequent polys have the same Z coefficients(useful for shadows)
//
// GC_DisableRefPlane   - Disables the reference plane so normal Z values are used, if passed TRUE the values used to calc
//                        the plane will be reset(needed when the camera view changes).
//
// GC_CheckForReset     - checks if the GameCube reset button has been pressed, and resets the system if so.
//
// GC_SetMipK           - sets the mipmap LOD bias value
//
// GC_GetMipK           - gets the mipmap LOD bias value
//
// GC_GetDXT3AlphaMapStage - gets the texmap slot index the corresponding alpha map is set to for a DXT3 texture.
//                           returns -1 if no DXT3 texture present at the specified slot.
//
////////////////////////////////////////////////////////////////////////////

void  GC_GetFixedAlpha   ( xbool& IsFixedAlphaUsed, u8& FixedAlpha );

void  GC_SetZMode        ( xbool ZTestOn, xbool ZFillOn, xbool bUseLEQUAL = TRUE );
void  GC_SetAlphaBlending( u32 BlendFlags, u8 FixedAlpha, xbool ForceChange = FALSE );
void  GC_SetAlphaTest    ( xbool AlphaTestOn );

void  GC_GetProjMatrix   ( f32 pMtx[4][4] );
void  GC_SetViewport     ( s32  L, s32  T, s32  W, s32  H );
void  GC_GetViewport     ( s32& L, s32& T, s32& W, s32& H );

void  GC_EnableLighting  ( xbool Enable, xbool VtxColors = FALSE );
void  GC_SetupLighting   ( void );
void  GC_UpdateLights    ( void );

void  GC_SetEFBClear     ( xbool bClearEachFrame );

void  GC_EnableRefPlane  ( const vector4& rPlane );
void  GC_DisableRefPlane ( xbool bPlaneCalcReset = FALSE );

void  GC_CheckForReset   ( void );

void  GC_SetMipK         ( f32  MipK );
void  GC_GetMipK         ( f32& MipK );

s32   GC_GetDXT3AlphaMapStage( s32 TexSlotID );


////////////////////////////////////////////////////////////////////////////
// TEV SETUP CONSTANTS
////////////////////////////////////////////////////////////////////////////

const GXTevStageID TEVSTAGE_ID[GX_MAX_TEVSTAGE] =
{
    GX_TEVSTAGE0,
    GX_TEVSTAGE1,
    GX_TEVSTAGE2,
    GX_TEVSTAGE3,
    GX_TEVSTAGE4,
    GX_TEVSTAGE5,
    GX_TEVSTAGE6,
    GX_TEVSTAGE7,
    GX_TEVSTAGE8,
    GX_TEVSTAGE9,
    GX_TEVSTAGE10,
    GX_TEVSTAGE11,
    GX_TEVSTAGE12,
    GX_TEVSTAGE13,
    GX_TEVSTAGE14,
    GX_TEVSTAGE15,
};

const GXTexCoordID TEXCOORD_ID[GX_MAX_TEXCOORD] =
{
    GX_TEXCOORD0,
    GX_TEXCOORD1,
    GX_TEXCOORD2,
    GX_TEXCOORD3,
    GX_TEXCOORD4,
    GX_TEXCOORD5,
    GX_TEXCOORD6,
    GX_TEXCOORD7,
};

const GXTexMapID TEXMAP_ID[GX_MAX_TEXMAP] =
{
    GX_TEXMAP0,
    GX_TEXMAP1,
    GX_TEXMAP2,
    GX_TEXMAP3,
    GX_TEXMAP4,
    GX_TEXMAP5,
    GX_TEXMAP6,
    GX_TEXMAP7,
};

const GXTlut TLUT_ID[GX_MAX_TEXMAP] =
{
    GX_TLUT0,
    GX_TLUT1,
    GX_TLUT2,
    GX_TLUT3,
    GX_TLUT4,
    GX_TLUT5,
    GX_TLUT6,
    GX_TLUT7,
};

////////////////////////////////////////////////////////////////////////////

#endif // Q_GC_HPP_INCLUDED
