////////////////////////////////////////////////////////////////////////////
//
// TBlockTexture.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "TBlockTexture.h"


////////////////////////////////////////////////////////////////////////////
// CONSTANTS
////////////////////////////////////////////////////////////////////////////

#define TS_FLAG_USE_GLOBAL                  (1<< 0)
#define TS_FLAG_SWIZZLE                     (1<< 1)
#define TS_FLAG_COMPRESS                    (1<< 2)
#define TS_FLAG_INTENSITY_MAP               (1<< 3)
#define TS_FLAG_APPLY_ALPHA                 (1<< 4)
#define TS_FLAG_ALPHA_BYFILENAME            (1<< 5)
#define TS_FLAG_ALPHA_PUNCH                 (1<< 6)
#define TS_FLAG_ALPHA_GENCLEARCLR           (1<< 7)
#define TS_FLAG_BUILD_MIPS                  (1<< 8)
#define TS_FLAG_MIPS_CUSTOM                 (1<< 9)
#define TS_FLAG_HOTCLR_TEST                 (1<<10)
#define TS_FLAG_HOTCLR_FIX                  (1<<11)
#define TS_FLAG_HOTCLRFIX_NTSC              (1<<12)
#define TS_FLAG_HOTCLRFIX_LUMINANCE         (1<<13)
#define TS_FLAG_COLOR_SCALE                 (1<<14)
#define TS_FLAG_CONVERT_CLR_SPACE           (1<<15)
#define TS_FLAG_GAMMA_CORRECT               (1<<16)
#define TS_FLAG_GAMMA_INVERT                (1<<17)
#define TS_FLAG_GAMMA_USE_PRESET            (1<<18)
#define TS_FLAG_ALLOW_4BIT_CONVERSION_TEST  (1<<19)


#define TS_FLAG_NMIPS_SHIFT         (28)
#define TS_FLAG_NMIPS_MASK          (0xF0000000)

#define TS_CLRSCALE_R_SHIFT         (0)
#define TS_CLRSCALE_G_SHIFT         (8)
#define TS_CLRSCALE_B_SHIFT         (16)
#define TS_CLRSCALE_A_SHIFT         (24)
#define TS_CLRSCALE_R_MASK          (0x000000FF)
#define TS_CLRSCALE_G_MASK          (0x0000FF00)
#define TS_CLRSCALE_B_MASK          (0x00FF0000)
#define TS_CLRSCALE_A_MASK          (0xFF000000)

#define TS_CLRSPACE_SRC_CS_SHIFT    (0)
#define TS_CLRSPACE_SRC_WP_SHIFT    (8)
#define TS_CLRSPACE_DST_CS_SHIFT    (16)
#define TS_CLRSPACE_DST_WP_SHIFT    (24)
#define TS_CLRSPACE_SRC_CS_MASK     (0x000000FF)
#define TS_CLRSPACE_SRC_WP_MASK     (0x0000FF00)
#define TS_CLRSPACE_DST_CS_MASK     (0x00FF0000)
#define TS_CLRSPACE_DST_WP_MASK     (0xFF000000)

#define TS_HOTCLRTEST_R_SHIFT       (0)
#define TS_HOTCLRTEST_G_SHIFT       (8)
#define TS_HOTCLRTEST_B_SHIFT       (16)
#define TS_HOTCLRTEST_A_SHIFT       (24)
#define TS_HOTCLRTEST_R_MASK        (0x000000FF)
#define TS_HOTCLRTEST_G_MASK        (0x0000FF00)
#define TS_HOTCLRTEST_B_MASK        (0x00FF0000)
#define TS_HOTCLRTEST_A_MASK        (0xFF000000)


////////////////////////////////////////////////////////////////////////////
// TBLOCK PATH IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

void TBLOCKPATH_InitDefault( TBlockPath& rPath )
{
    x_strncpy( rPath, TBLOCK_EMPTY_PATH, TBLOCK_PATH_LENGTH );
}

//==========================================================================

void TBLOCKPATH_GetShortName( TBlockPath& rPath, TBlockShortName& rShortName )
{
    TBlockPath Filename;

    if( 0 != x_strcmp( rPath, TBLOCK_EMPTY_PATH ) )
    {
        x_splitpath( rPath, NULL, NULL, Filename, NULL );
        x_strncpy( rShortName, Filename, TBLOCK_SHORTNAME_LENGTH );
    }
    else
    {
        x_strncpy( rShortName, TBLOCK_EMPTY_SHORTNAME, TBLOCK_SHORTNAME_LENGTH );
    }
}

//==========================================================================

xbool TBLOCKPATH_IsDefault( TBlockPath& rPath )
{
    if( x_strcmp( rPath, TBLOCK_EMPTY_PATH ) )
        return FALSE;

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////
// TBLOCK SETTINGS IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

void TBlockSettings::InitDefault( void )
{
    m_Flags        = 0;
    m_ColorSpace   = 0;
    m_ColorScale   = 0;
    m_HotColorTest = 0;
    m_PresetGamma  = 0;

    SetUseGlobal   ( TRUE );
    SetSwizzle     ( TRUE );
    SetCompressFmt ( FALSE );
    SetIntensityMap( FALSE );
    SetApplyAlpha  ( FALSE, TRUE, FALSE, FALSE );
    SetBuildMips   ( FALSE, FALSE, TBLOCK_BUILD_MIPS_MAX );
    SetColorScale  ( FALSE, 255, 255, 255, 255 );
    SetColorSpace  ( FALSE, COLORSPACE_SRGB, WHITEPOINT_D65, COLORSPACE_NTSC, WHITEPOINT_CIE_STD_C );
    SetGamma       ( FALSE, FALSE, GAMMA_NTSC );
    SetHotColorTest( FALSE, TRUE, 255, 255, 255, 255 );
    SetHotColorFix ( FALSE, TRUE, TRUE );
    SetAllow4BitConversionTest( TRUE );
}

//==========================================================================

void TBlockSettings::SetUseGlobal( xbool bSetOn )
{
    if( bSetOn )
        m_Flags |= TS_FLAG_USE_GLOBAL;
    else
        m_Flags &= ~TS_FLAG_USE_GLOBAL;
}

//==========================================================================

xbool TBlockSettings::GetUseGlobal( void )
{
    return (m_Flags & TS_FLAG_USE_GLOBAL) ? TRUE : FALSE;
}

//==========================================================================

void TBlockSettings::SetSwizzle( xbool bSetOn )
{
    if( bSetOn )
        m_Flags |= TS_FLAG_SWIZZLE;
    else
        m_Flags &= ~TS_FLAG_SWIZZLE;
}

//==========================================================================

xbool TBlockSettings::GetSwizzle( void )
{
    return (m_Flags & TS_FLAG_SWIZZLE) ? TRUE : FALSE;
}

//==========================================================================

void TBlockSettings::SetCompressFmt( xbool bSetOn )
{
    if( bSetOn )
        m_Flags |= TS_FLAG_COMPRESS;
    else
        m_Flags &= ~TS_FLAG_COMPRESS;
}

//==========================================================================

xbool TBlockSettings::GetCompressFmt( void )
{
    return (m_Flags & TS_FLAG_COMPRESS) ? TRUE : FALSE;
}

//==========================================================================

void TBlockSettings::SetIntensityMap( xbool bSetOn )
{
    if( bSetOn )
        m_Flags |= TS_FLAG_INTENSITY_MAP;
    else
        m_Flags &= ~TS_FLAG_INTENSITY_MAP;
}

//==========================================================================

xbool TBlockSettings::GetIntensityMap( void )
{
    return (m_Flags & TS_FLAG_INTENSITY_MAP) ? TRUE : FALSE;
}

//==========================================================================

void TBlockSettings::SetBuildMips( xbool bSetOn, xbool bUseCustom, u32 NMips )
{
    if( bSetOn )
        m_Flags |= TS_FLAG_BUILD_MIPS;
    else
        m_Flags &= ~TS_FLAG_BUILD_MIPS;

    if( bUseCustom )
        m_Flags |= TS_FLAG_MIPS_CUSTOM;
    else
        m_Flags &= ~TS_FLAG_MIPS_CUSTOM;

    m_Flags &= ~TS_FLAG_NMIPS_MASK;

    m_Flags |= ((NMips << TS_FLAG_NMIPS_SHIFT) & TS_FLAG_NMIPS_MASK);
}

//==========================================================================

xbool TBlockSettings::GetBuildMips( xbool& bUseCustom, u32& NMips )
{
    if( m_Flags & TS_FLAG_MIPS_CUSTOM )
        bUseCustom = TRUE;
    else
        bUseCustom = FALSE;

    NMips = (m_Flags & TS_FLAG_NMIPS_MASK) >> TS_FLAG_NMIPS_SHIFT;

    return (m_Flags & TS_FLAG_BUILD_MIPS) ? TRUE : FALSE;
}

//==========================================================================

xbool TBlockSettings::GetBuildMips( void )
{
    return (m_Flags & TS_FLAG_BUILD_MIPS) ? TRUE : FALSE;
}

//==========================================================================

void TBlockSettings::SetApplyAlpha( xbool bSetOn, xbool bUseFilename, xbool bPunch, xbool bGenClear )
{
    if( bSetOn )
        m_Flags |= TS_FLAG_APPLY_ALPHA;
    else
        m_Flags &= ~TS_FLAG_APPLY_ALPHA;

    if( bUseFilename )
        m_Flags |= TS_FLAG_ALPHA_BYFILENAME;
    else
        m_Flags &= ~TS_FLAG_ALPHA_BYFILENAME;

    if( bPunch )
        m_Flags |= TS_FLAG_ALPHA_PUNCH;
    else
        m_Flags &= ~TS_FLAG_ALPHA_PUNCH;

    if( bGenClear )
        m_Flags |= TS_FLAG_ALPHA_GENCLEARCLR;
    else
        m_Flags &= ~TS_FLAG_ALPHA_GENCLEARCLR;
}

//==========================================================================

xbool TBlockSettings::GetApplyAlpha( xbool& bUseFilename, xbool& bPunch, xbool& bGenClear )
{
    if( m_Flags & TS_FLAG_ALPHA_BYFILENAME )
        bUseFilename = TRUE;
    else
        bUseFilename = FALSE;

    if( m_Flags & TS_FLAG_ALPHA_PUNCH )
        bPunch = TRUE;
    else
        bPunch = FALSE;

    if( m_Flags & TS_FLAG_ALPHA_GENCLEARCLR )
        bGenClear = TRUE;
    else
        bGenClear = FALSE;

    return (m_Flags & TS_FLAG_APPLY_ALPHA) ? TRUE : FALSE;
}

//==========================================================================

xbool TBlockSettings::GetApplyAlpha( void )
{
    return (m_Flags & TS_FLAG_APPLY_ALPHA) ? TRUE : FALSE;
}

//==========================================================================

void TBlockSettings::SetColorScale( xbool bSetOn, u32 R, u32 G, u32 B, u32 A )
{
    if( bSetOn )
        m_Flags |= TS_FLAG_COLOR_SCALE;
    else
        m_Flags &= ~TS_FLAG_COLOR_SCALE;

    m_ColorScale  = (R << TS_CLRSCALE_R_SHIFT) & TS_CLRSCALE_R_MASK;
    m_ColorScale |= (G << TS_CLRSCALE_G_SHIFT) & TS_CLRSCALE_G_MASK;
    m_ColorScale |= (B << TS_CLRSCALE_B_SHIFT) & TS_CLRSCALE_B_MASK;
    m_ColorScale |= (A << TS_CLRSCALE_A_SHIFT) & TS_CLRSCALE_A_MASK;
}

//==========================================================================

xbool TBlockSettings::GetColorScale( u32& R, u32& G, u32& B, u32& A )
{
    R = (m_ColorScale & TS_CLRSCALE_R_MASK) >> TS_CLRSCALE_R_SHIFT;
    G = (m_ColorScale & TS_CLRSCALE_G_MASK) >> TS_CLRSCALE_G_SHIFT;
    B = (m_ColorScale & TS_CLRSCALE_B_MASK) >> TS_CLRSCALE_B_SHIFT;
    A = (m_ColorScale & TS_CLRSCALE_A_MASK) >> TS_CLRSCALE_A_SHIFT;

    return (m_Flags & TS_FLAG_COLOR_SCALE) ? TRUE : FALSE;
}

//==========================================================================

xbool TBlockSettings::GetColorScale( void )
{
    return (m_Flags & TS_FLAG_COLOR_SCALE) ? TRUE : FALSE;
}

//==========================================================================

void TBlockSettings::SetColorSpace( xbool bSetOn, EColorSpace SrcCS, EWhitePoint SrcWP, EColorSpace DstCS, EWhitePoint DstWP )
{
    if( bSetOn )
        m_Flags |= TS_FLAG_CONVERT_CLR_SPACE;
    else
        m_Flags &= ~TS_FLAG_CONVERT_CLR_SPACE;

    m_ColorSpace  = ((u32)SrcCS << TS_CLRSPACE_SRC_CS_SHIFT) & TS_CLRSPACE_SRC_CS_MASK;
    m_ColorSpace |= ((u32)SrcWP << TS_CLRSPACE_SRC_WP_SHIFT) & TS_CLRSPACE_SRC_WP_MASK;
    m_ColorSpace |= ((u32)DstCS << TS_CLRSPACE_DST_CS_SHIFT) & TS_CLRSPACE_DST_CS_MASK;
    m_ColorSpace |= ((u32)DstWP << TS_CLRSPACE_DST_WP_SHIFT) & TS_CLRSPACE_DST_WP_MASK;
}

//==========================================================================

xbool TBlockSettings::GetColorSpace( EColorSpace& SrcCS, EWhitePoint& SrcWP, EColorSpace& DstCS, EWhitePoint& DstWP )
{
    SrcCS = (EColorSpace)((m_ColorSpace & TS_CLRSPACE_SRC_CS_MASK) >> TS_CLRSPACE_SRC_CS_SHIFT);
    SrcWP = (EWhitePoint)((m_ColorSpace & TS_CLRSPACE_SRC_WP_MASK) >> TS_CLRSPACE_SRC_WP_SHIFT);
    DstCS = (EColorSpace)((m_ColorSpace & TS_CLRSPACE_DST_CS_MASK) >> TS_CLRSPACE_DST_CS_SHIFT);
    DstWP = (EWhitePoint)((m_ColorSpace & TS_CLRSPACE_DST_WP_MASK) >> TS_CLRSPACE_DST_WP_SHIFT);

    return (m_Flags & TS_FLAG_CONVERT_CLR_SPACE) ? TRUE : FALSE;
}

//==========================================================================

xbool TBlockSettings::GetColorSpace( void )
{
    return (m_Flags & TS_FLAG_CONVERT_CLR_SPACE) ? TRUE : FALSE;
}

//==========================================================================

void TBlockSettings::SetGamma( xbool bSetOn, xbool bInvert, f32 Gamma )
{
    if( bSetOn )
        m_Flags |= TS_FLAG_GAMMA_CORRECT;
    else
        m_Flags &= ~TS_FLAG_GAMMA_CORRECT;

    m_Flags &= ~TS_FLAG_GAMMA_USE_PRESET;

    if( bInvert )
        m_Flags |= TS_FLAG_GAMMA_INVERT;
    else
        m_Flags &= ~TS_FLAG_GAMMA_INVERT;

    m_CustomGamma = Gamma;
}

//==========================================================================

void TBlockSettings::SetGamma( xbool bSetOn, xbool bInvert, EGamma PresetGamma )
{
    if( bSetOn )
        m_Flags |= TS_FLAG_GAMMA_CORRECT;
    else
        m_Flags &= ~TS_FLAG_GAMMA_CORRECT;

    m_Flags |= TS_FLAG_GAMMA_USE_PRESET;

    if( bInvert )
        m_Flags |= TS_FLAG_GAMMA_INVERT;
    else
        m_Flags &= ~TS_FLAG_GAMMA_INVERT;

    m_PresetGamma = PresetGamma;
}

//==========================================================================

xbool TBlockSettings::GetGamma( xbool& bInvert, EGamma& PresetGamma, f32& Gamma )
{
    if( m_Flags & TS_FLAG_GAMMA_INVERT )
        bInvert = TRUE;
    else
        bInvert = FALSE;

    if( m_Flags & TS_FLAG_GAMMA_USE_PRESET )
    {
        PresetGamma = (EGamma)m_PresetGamma;
        Gamma       = BMPCOLOR_GetGammaValue( (EGamma)m_PresetGamma );
    }
    else
    {
        PresetGamma = GAMMA_COUNT;
        Gamma       = m_CustomGamma;
    }

    return (m_Flags & TS_FLAG_GAMMA_CORRECT) ? TRUE : FALSE;
}

//==========================================================================

xbool TBlockSettings::GetGamma( void )
{
    return (m_Flags & TS_FLAG_GAMMA_CORRECT) ? TRUE : FALSE;
}

//==========================================================================

void TBlockSettings::SetHotColorFix( xbool bSetOn, xbool bFixNTSC, xbool bFixLuminance )
{
    if( bSetOn )
    {
        m_Flags |= TS_FLAG_HOTCLR_FIX;
        m_Flags &= ~TS_FLAG_HOTCLR_TEST;
    }
    else
    {
        m_Flags &= ~TS_FLAG_HOTCLR_FIX;
    }

    if( bFixNTSC )
        m_Flags |= TS_FLAG_HOTCLRFIX_NTSC;
    else
        m_Flags &= ~TS_FLAG_HOTCLRFIX_NTSC;

    if( bFixLuminance )
        m_Flags |= TS_FLAG_HOTCLRFIX_LUMINANCE;
    else
        m_Flags &= ~TS_FLAG_HOTCLRFIX_LUMINANCE;
}

//==========================================================================

xbool TBlockSettings::GetHotColorFix( xbool& bFixNTSC, xbool& bFixLuminance )
{
    if( m_Flags & TS_FLAG_HOTCLRFIX_NTSC )
        bFixNTSC = TRUE;
    else
        bFixNTSC = FALSE;

    if( m_Flags & TS_FLAG_HOTCLRFIX_LUMINANCE )
        bFixLuminance = TRUE;
    else
        bFixLuminance = FALSE;

    return (m_Flags & TS_FLAG_HOTCLR_FIX) ? TRUE : FALSE;
}

//==========================================================================

xbool TBlockSettings::GetHotColorFix( void )
{
    return (m_Flags & TS_FLAG_HOTCLR_FIX) ? TRUE : FALSE;
}

//==========================================================================

void TBlockSettings::SetHotColorTest( xbool bSetOn, xbool bFixNTSC, u32 R, u32 G, u32 B, u32 A )
{
    if( bSetOn )
    {
        m_Flags |= TS_FLAG_HOTCLR_TEST;
        m_Flags &= ~TS_FLAG_HOTCLR_FIX;
    }
    else
    {
        m_Flags &= ~TS_FLAG_HOTCLR_TEST;
    }

    if( bFixNTSC )
        m_Flags |= TS_FLAG_HOTCLRFIX_NTSC;
    else
        m_Flags &= ~TS_FLAG_HOTCLRFIX_NTSC;

    m_HotColorTest  = (R << TS_HOTCLRTEST_R_SHIFT) & TS_HOTCLRTEST_R_MASK;
    m_HotColorTest |= (G << TS_HOTCLRTEST_G_SHIFT) & TS_HOTCLRTEST_G_MASK;
    m_HotColorTest |= (B << TS_HOTCLRTEST_B_SHIFT) & TS_HOTCLRTEST_B_MASK;
    m_HotColorTest |= (A << TS_HOTCLRTEST_A_SHIFT) & TS_HOTCLRTEST_A_MASK;
}

//==========================================================================

xbool TBlockSettings::GetHotColorTest( xbool& bFixNTSC, u32& R, u32& G, u32& B, u32& A )
{
    if( m_Flags & TS_FLAG_HOTCLRFIX_NTSC )
        bFixNTSC = TRUE;
    else
        bFixNTSC = FALSE;

    R = (m_HotColorTest & TS_HOTCLRTEST_R_MASK) >> TS_HOTCLRTEST_R_SHIFT;
    G = (m_HotColorTest & TS_HOTCLRTEST_G_MASK) >> TS_HOTCLRTEST_G_SHIFT;
    B = (m_HotColorTest & TS_HOTCLRTEST_B_MASK) >> TS_HOTCLRTEST_B_SHIFT;
    A = (m_HotColorTest & TS_HOTCLRTEST_A_MASK) >> TS_HOTCLRTEST_A_SHIFT;

    return (m_Flags & TS_FLAG_HOTCLR_TEST) ? TRUE : FALSE;
}

//==========================================================================

xbool TBlockSettings::GetHotColorTest( void )
{
    return (m_Flags & TS_FLAG_HOTCLR_TEST) ? TRUE : FALSE;
}

//==========================================================================
void TBlockSettings::SetAllow4BitConversionTest( xbool bSetOn )
{
    if( bSetOn )
        m_Flags |= TS_FLAG_ALLOW_4BIT_CONVERSION_TEST;
    else
        m_Flags &= ~TS_FLAG_ALLOW_4BIT_CONVERSION_TEST;
}


//==========================================================================
xbool TBlockSettings::GetAllow4BitConversionTest( void )
{
    return (m_Flags & TS_FLAG_ALLOW_4BIT_CONVERSION_TEST) ? TRUE : FALSE;
}
