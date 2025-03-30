////////////////////////////////////////////////////////////////////////////
//
// TBlockTexture.h
//
////////////////////////////////////////////////////////////////////////////

#ifndef TBLOCKTEXTURE_H_INCLUDED
#define TBLOCKTEXTURE_H_INCLUDED

////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_types.hpp"

#include "BMPColor.h"


////////////////////////////////////////////////////////////////////////////
// CONSTANTS
////////////////////////////////////////////////////////////////////////////

#define TBLOCK_MAXFILES             (3 * 1024)

#define TBLOCK_BUILD_MIPS_MIN       1
#define TBLOCK_BUILD_MIPS_MAX       15

#define TBLOCK_COLOR_INT_MIN        0
#define TBLOCK_COLOR_INT_MAX        255

#define TBLOCK_COLOR_FLOAT_MIN      0.0f
#define TBLOCK_COLOR_FLOAT_MAX      1.0f

#define TBLOCK_GAMMA_VAL_MIN        0.25f
#define TBLOCK_GAMMA_VAL_MAX        4.0f

#define TBLOCK_PATH_LENGTH          256
#define TBLOCK_SHORTNAME_LENGTH     32

#define TBLOCK_GLOBAL_PATHNAME      "[GLOBAL]"
#define TBLOCK_EMPTY_PATH           "[EMPTY]"
#define TBLOCK_EMPTY_SHORTNAME      "EMPTY"

#define TBLOCK_FILE_EXT_TEX         "tex"
#define TBLOCK_FILE_EXT_TXT         "txt"

#define TBLOCK_FILE_EXT_BMP         "bmp"
#define TBLOCK_FILE_EXT_GIF         "gif"
#define TBLOCK_FILE_EXT_TGA         "tga"


////////////////////////////////////////////////////////////////////////////
// MACROS
////////////////////////////////////////////////////////////////////////////

#define TBLOCK_CLAMP_CLR_INT( C )       (((C) < TBLOCK_COLOR_INT_MIN) ? TBLOCK_COLOR_INT_MIN : (((C) > TBLOCK_COLOR_INT_MAX) ? TBLOCK_COLOR_INT_MAX : (C)))
#define TBLOCK_CLAMP_CLR_FLOAT( C )     (((C) < TBLOCK_COLOR_FLOAT_MIN) ? TBLOCK_COLOR_FLOAT_MIN : (((C) > TBLOCK_COLOR_FLOAT_MAX) ? TBLOCK_COLOR_FLOAT_MAX : (C)))

#define TBLOCK_CLAMP_GAMMA_VAL( G )     (((G) < TBLOCK_GAMMA_VAL_MIN) ? TBLOCK_GAMMA_VAL_MIN : (((G) > TBLOCK_GAMMA_VAL_MAX) ? TBLOCK_GAMMA_VAL_MAX : (G)))


////////////////////////////////////////////////////////////////////////////
// TBLOCK PATH INTERFACE
////////////////////////////////////////////////////////////////////////////

typedef char TBlockPath[ TBLOCK_PATH_LENGTH ];

typedef char TBlockShortName[ TBLOCK_SHORTNAME_LENGTH ];


void TBLOCKPATH_InitDefault ( TBlockPath& rPath );
void TBLOCKPATH_GetShortName( TBlockPath& rPath, TBlockShortName& rShortName );

xbool TBLOCKPATH_IsDefault( TBlockPath& rPath );


////////////////////////////////////////////////////////////////////////////
// TBLOCK SETTINGS INTERFACE
////////////////////////////////////////////////////////////////////////////

struct TBlockSettings
{
    ////////////////////////////////////////////////////////////
    // FUNCTIONS
    ////////////////////////////////////////////////////////////

    void  InitDefault    ( void );

    void  SetUseGlobal   ( xbool bSetOn );
    xbool GetUseGlobal   ( void );

    void  SetSwizzle     ( xbool bSetOn );
    xbool GetSwizzle     ( void );

    void  SetCompressFmt ( xbool bSetOn );
    xbool GetCompressFmt ( void );

    void  SetIntensityMap( xbool bSetOn );
    xbool GetIntensityMap( void );

    void  SetBuildMips   ( xbool bSetOn, xbool bUseCustom, u32 NMips );
    xbool GetBuildMips   ( xbool& bUseCustom, u32& NMips );
    xbool GetBuildMips   ( void );

    void  SetApplyAlpha  ( xbool bSetOn, xbool bUseFilename, xbool bPunch, xbool bGenClear );
    xbool GetApplyAlpha  ( xbool& bUseFilename, xbool& bPunch, xbool& bGenClear );
    xbool GetApplyAlpha  ( void );

    void  SetColorScale  ( xbool bSetOn, u32 R, u32 G, u32 B, u32 A );
    xbool GetColorScale  ( u32& R, u32& G, u32& B, u32& A );
    xbool GetColorScale  ( void );

    void  SetColorSpace  ( xbool bSetOn, EColorSpace SrcCS, EWhitePoint SrcWP, EColorSpace DstCS, EWhitePoint DstWP );
    xbool GetColorSpace  ( EColorSpace& SrcCS, EWhitePoint& SrcWP, EColorSpace& DstCS, EWhitePoint& DstWP );
    xbool GetColorSpace  ( void );

    void  SetGamma       ( xbool bSetOn, xbool bInvert, f32 Gamma );
    void  SetGamma       ( xbool bSetOn, xbool bInvert, EGamma PresetGamma );
    xbool GetGamma       ( xbool& bInvert, EGamma& PresetGamma, f32& Gamma );
    xbool GetGamma       ( void );

    void  SetHotColorFix ( xbool bSetOn, xbool bFixNTSC, xbool bFixLuminance );
    xbool GetHotColorFix ( xbool& bFixNTSC, xbool& bFixLuminance );
    xbool GetHotColorFix ( void );

    void  SetHotColorTest( xbool bSetOn, xbool bFixNTSC, u32 R, u32 G, u32 B, u32 A );
    xbool GetHotColorTest( xbool& bFixNTSC, u32& R, u32& G, u32& B, u32& A );
    xbool GetHotColorTest( void );
    
    void  SetAllow4BitConversionTest( xbool bSetOn );
    xbool GetAllow4BitConversionTest( void );

protected:
    ////////////////////////////////////////////////////////////
    // DATA
    ////////////////////////////////////////////////////////////

    u32     m_Flags;            // flags indicating various settings
    u32     m_ColorSpace;       // source-dest colorspace and whitepoints
    u32     m_ColorScale;       // amount to scale bitmap intensity
    u32     m_HotColorTest;     // color value set to hot color regions

    union
    {
        s32 m_PresetGamma;      // EGamma constant(if flags indicate its use)
        f32 m_CustomGamma;      // float value for gamma(if custom)
    };
};

////////////////////////////////////////////////////////////////////////////

#endif // TBLOCKTEXTURE_H_INCLUDED