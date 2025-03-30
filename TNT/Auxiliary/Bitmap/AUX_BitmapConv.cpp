////////////////////////////////////////////////////////////////////////////
//
// AUX_BitmapConv.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_bitmap.hpp"
#include "x_debug.hpp"
#include "x_memory.hpp"
#include "x_plus.hpp"

#include "AUX_Bitmap.hpp"


////////////////////////////////////////////////////////////////////////////
// TYPES
////////////////////////////////////////////////////////////////////////////

typedef xbool (*bmp_conv_fnptr)( x_bitmap& SrcBMP, x_bitmap::format SrcFormat, 
                                 x_bitmap& DstBMP, x_bitmap::format DstFormat );


////////////////////////////////////////////////////////////////////////////
// CONVERSION TABLES
////////////////////////////////////////////////////////////////////////////

static x_bitmap::format CONV_FMT_NULL          [] =
{
    x_bitmap::FMT_NULL
};

//--------------------------------------------------------------------------

static x_bitmap::format CONV_FMT_16_RGB_565    [] =
{
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_16_ARGB_4444  [] =
{
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_16_RGBA_4444  [] =
{
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_16_ARGB_1555  [] =
{
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_16_RGBA_5551  [] =
{
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_16_URGB_1555  [] =
{
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_16_RGBU_5551  [] =
{
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_16_ABGR_1555  [] =
{
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_16_UBGR_1555  [] =
{
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_16_ARGB_3444  [] =
{
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_NULL
};

//--------------------------------------------------------------------------


static x_bitmap::format CONV_FMT_24_RGB_888    [] =
{
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_24_ARGB_8565  [] =
{
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_NULL
};


static x_bitmap::format CONV_FMT_32_RGBU_8888  [] =
{
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_NULL
};

//--------------------------------------------------------------------------

static x_bitmap::format CONV_FMT_32_URGB_8888  [] =
{
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_32_ARGB_8888  [] =
{
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_32_RGBA_8888  [] =
{
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_32_ABGR_8888  [] =
{
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_NULL
};


static x_bitmap::format CONV_FMT_32_BGRA_8888  [] =
{
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_NULL
};

//--------------------------------------------------------------------------

static x_bitmap::format CONV_FMT_P4_RGB_565   [] =
{
    x_bitmap::FMT_P4_RGB_565,
    x_bitmap::FMT_P4_RGBU_5551,
    x_bitmap::FMT_P4_URGB_1555,
    x_bitmap::FMT_P4_UBGR_1555,
    x_bitmap::FMT_P4_RGBA_5551,
    x_bitmap::FMT_P4_ARGB_1555,
    x_bitmap::FMT_P4_ABGR_1555,
    x_bitmap::FMT_P4_RGB_888,
    x_bitmap::FMT_P4_URGB_8888,
    x_bitmap::FMT_P4_UBGR_8888,
    x_bitmap::FMT_P4_RGBU_8888,
    x_bitmap::FMT_P4_ARGB_8888,
    x_bitmap::FMT_P4_ABGR_8888,
    x_bitmap::FMT_P4_RGBA_8888,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_P4_RGB_888   [] =
{
    x_bitmap::FMT_P4_RGB_888,
    x_bitmap::FMT_P4_RGB_565,
    x_bitmap::FMT_P4_RGBU_5551,
    x_bitmap::FMT_P4_URGB_1555,
    x_bitmap::FMT_P4_UBGR_1555,
    x_bitmap::FMT_P4_RGBA_5551,
    x_bitmap::FMT_P4_ARGB_1555,
    x_bitmap::FMT_P4_ABGR_1555,
    x_bitmap::FMT_P4_URGB_8888,
    x_bitmap::FMT_P4_UBGR_8888,
    x_bitmap::FMT_P4_RGBU_8888,
    x_bitmap::FMT_P4_ARGB_8888,
    x_bitmap::FMT_P4_ABGR_8888,
    x_bitmap::FMT_P4_RGBA_8888,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_P4_URGB_8888 [] =
{
    x_bitmap::FMT_P4_URGB_8888,
    x_bitmap::FMT_P4_UBGR_8888,
    x_bitmap::FMT_P4_RGBU_8888,
    x_bitmap::FMT_P4_RGB_888,
    x_bitmap::FMT_P4_RGB_565,
    x_bitmap::FMT_P4_RGBU_5551,
    x_bitmap::FMT_P4_URGB_1555,
    x_bitmap::FMT_P4_UBGR_1555,
    x_bitmap::FMT_P4_RGBA_5551,
    x_bitmap::FMT_P4_ARGB_1555,
    x_bitmap::FMT_P4_ABGR_1555,
    x_bitmap::FMT_P4_ARGB_8888,
    x_bitmap::FMT_P4_ABGR_8888,
    x_bitmap::FMT_P4_RGBA_8888,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_P4_RGBU_8888 [] =
{
    x_bitmap::FMT_P4_RGBU_8888,
    x_bitmap::FMT_P4_URGB_8888,
    x_bitmap::FMT_P4_UBGR_8888,
    x_bitmap::FMT_P4_RGB_888,
    x_bitmap::FMT_P4_RGB_565,
    x_bitmap::FMT_P4_RGBU_5551,
    x_bitmap::FMT_P4_URGB_1555,
    x_bitmap::FMT_P4_UBGR_1555,
    x_bitmap::FMT_P4_RGBA_5551,
    x_bitmap::FMT_P4_ARGB_1555,
    x_bitmap::FMT_P4_ABGR_1555,
    x_bitmap::FMT_P4_ARGB_8888,
    x_bitmap::FMT_P4_ABGR_8888,
    x_bitmap::FMT_P4_RGBA_8888,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_NULL
};


static x_bitmap::format CONV_FMT_P4_ARGB_8888 [] =
{
    x_bitmap::FMT_P4_ARGB_8888,
    x_bitmap::FMT_P4_RGBA_8888,
    x_bitmap::FMT_P4_ABGR_8888,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_P4_RGBA_5551,
    x_bitmap::FMT_P4_ARGB_1555,
    x_bitmap::FMT_P4_RGB_888,
    x_bitmap::FMT_P4_RGBU_8888,
    x_bitmap::FMT_P4_URGB_8888,
    x_bitmap::FMT_P4_UBGR_8888,
    x_bitmap::FMT_P4_RGB_565,
    x_bitmap::FMT_P4_RGBU_5551,
    x_bitmap::FMT_P4_URGB_1555,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_P4_RGBA_8888 [] =
{
    x_bitmap::FMT_P4_RGBA_8888,
    x_bitmap::FMT_P4_ARGB_8888,
    x_bitmap::FMT_P4_ABGR_8888,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_P4_RGBA_5551,
    x_bitmap::FMT_P4_ARGB_1555,
    x_bitmap::FMT_P4_RGB_888,
    x_bitmap::FMT_P4_RGBU_8888,
    x_bitmap::FMT_P4_URGB_8888,
    x_bitmap::FMT_P4_UBGR_8888,
    x_bitmap::FMT_P4_RGB_565,
    x_bitmap::FMT_P4_RGBU_5551,
    x_bitmap::FMT_P4_URGB_1555,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_NULL
};


static x_bitmap::format CONV_FMT_P4_ABGR_8888 [] =
{
    x_bitmap::FMT_P4_ABGR_8888,
    x_bitmap::FMT_P4_ARGB_8888,
    x_bitmap::FMT_P4_RGBA_8888,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_P4_RGBA_5551,
    x_bitmap::FMT_P4_ARGB_1555,
    x_bitmap::FMT_P4_RGB_888,
    x_bitmap::FMT_P4_RGBU_8888,
    x_bitmap::FMT_P4_URGB_8888,
    x_bitmap::FMT_P4_UBGR_8888,
    x_bitmap::FMT_P4_RGB_565,
    x_bitmap::FMT_P4_RGBU_5551,
    x_bitmap::FMT_P4_URGB_1555,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_NULL
};


static x_bitmap::format CONV_FMT_P4_UBGR_8888 [] =
{
    x_bitmap::FMT_P4_UBGR_8888,
    x_bitmap::FMT_P4_URGB_8888,
    x_bitmap::FMT_P4_RGBU_8888,
    x_bitmap::FMT_P4_RGB_888,
    x_bitmap::FMT_P4_RGB_565,
    x_bitmap::FMT_P4_RGBU_5551,
    x_bitmap::FMT_P4_URGB_1555,
    x_bitmap::FMT_P4_UBGR_1555,
    x_bitmap::FMT_P4_RGBA_5551,
    x_bitmap::FMT_P4_ARGB_1555,
    x_bitmap::FMT_P4_ABGR_1555,
    x_bitmap::FMT_P4_ARGB_8888,
    x_bitmap::FMT_P4_ABGR_8888,
    x_bitmap::FMT_P4_RGBA_8888,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_P4_RGBA_5551 [] =
{
    x_bitmap::FMT_P4_RGBA_5551,
    x_bitmap::FMT_P4_ARGB_1555,
    x_bitmap::FMT_P4_ABGR_1555,
    x_bitmap::FMT_P4_RGBA_8888,
    x_bitmap::FMT_P4_ARGB_8888,
    x_bitmap::FMT_P4_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_P4_RGB_888,
    x_bitmap::FMT_P4_RGBU_8888,
    x_bitmap::FMT_P4_URGB_8888,
    x_bitmap::FMT_P4_UBGR_8888,
    x_bitmap::FMT_P4_RGB_565,
    x_bitmap::FMT_P4_RGBU_5551,
    x_bitmap::FMT_P4_URGB_1555,
    x_bitmap::FMT_P4_UBGR_1555,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_P4_ARGB_1555 [] =
{
    x_bitmap::FMT_P4_ARGB_1555,
    x_bitmap::FMT_P4_ABGR_1555,
    x_bitmap::FMT_P4_RGBA_5551,
    x_bitmap::FMT_P4_RGBA_8888,
    x_bitmap::FMT_P4_ARGB_8888,
    x_bitmap::FMT_P4_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_P4_RGB_888,
    x_bitmap::FMT_P4_RGBU_8888,
    x_bitmap::FMT_P4_URGB_8888,
    x_bitmap::FMT_P4_UBGR_8888,
    x_bitmap::FMT_P4_RGB_565,
    x_bitmap::FMT_P4_RGBU_5551,
    x_bitmap::FMT_P4_UBGR_1555,
    x_bitmap::FMT_P4_URGB_1555,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_P4_RGBU_5551 [] =
{
    x_bitmap::FMT_P4_RGBU_5551,
    x_bitmap::FMT_P4_URGB_1555,
    x_bitmap::FMT_P4_UBGR_1555,
    x_bitmap::FMT_P4_RGB_565,
    x_bitmap::FMT_P4_RGB_888,
    x_bitmap::FMT_P4_RGBU_8888,
    x_bitmap::FMT_P4_URGB_8888,
    x_bitmap::FMT_P4_UBGR_8888,
    x_bitmap::FMT_P4_ARGB_1555,
    x_bitmap::FMT_P4_ABGR_1555,
    x_bitmap::FMT_P4_RGBA_5551,
    x_bitmap::FMT_P4_RGBA_8888,
    x_bitmap::FMT_P4_ARGB_8888,
    x_bitmap::FMT_P4_ABGR_8888,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_P4_URGB_1555 [] =
{
    x_bitmap::FMT_P4_URGB_1555,
    x_bitmap::FMT_P4_UBGR_1555,
    x_bitmap::FMT_P4_RGBU_5551,
    x_bitmap::FMT_P4_RGB_565,
    x_bitmap::FMT_P4_RGB_888,
    x_bitmap::FMT_P4_RGBU_8888,
    x_bitmap::FMT_P4_URGB_8888,
    x_bitmap::FMT_P4_UBGR_8888,
    x_bitmap::FMT_P4_ARGB_1555,
    x_bitmap::FMT_P4_ABGR_1555,
    x_bitmap::FMT_P4_RGBA_5551,
    x_bitmap::FMT_P4_RGBA_8888,
    x_bitmap::FMT_P4_ARGB_8888,
    x_bitmap::FMT_P4_ABGR_8888,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_NULL
};


static x_bitmap::format CONV_FMT_P4_ABGR_1555 [] =
{
    x_bitmap::FMT_P4_ARGB_1555,
    x_bitmap::FMT_P4_ABGR_1555,
    x_bitmap::FMT_P4_RGBA_5551,
    x_bitmap::FMT_P4_RGBA_8888,
    x_bitmap::FMT_P4_ARGB_8888,
    x_bitmap::FMT_P4_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_P4_RGB_888,
    x_bitmap::FMT_P4_RGBU_8888,
    x_bitmap::FMT_P4_URGB_8888,
    x_bitmap::FMT_P4_UBGR_8888,
    x_bitmap::FMT_P4_RGB_565,
    x_bitmap::FMT_P4_RGBU_5551,
    x_bitmap::FMT_P4_URGB_1555,
    x_bitmap::FMT_P4_UBGR_1555,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_P4_UBGR_1555 [] =
{
    x_bitmap::FMT_P4_URGB_1555,
    x_bitmap::FMT_P4_UBGR_1555,
    x_bitmap::FMT_P4_RGBU_5551,
    x_bitmap::FMT_P4_RGB_565,
    x_bitmap::FMT_P4_RGB_888,
    x_bitmap::FMT_P4_RGBU_8888,
    x_bitmap::FMT_P4_URGB_8888,
    x_bitmap::FMT_P4_UBGR_8888,
    x_bitmap::FMT_P4_ARGB_1555,
    x_bitmap::FMT_P4_ABGR_1555,
    x_bitmap::FMT_P4_RGBA_5551,
    x_bitmap::FMT_P4_RGBA_8888,
    x_bitmap::FMT_P4_ARGB_8888,
    x_bitmap::FMT_P4_ABGR_8888,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_P4_ARGB_3444  [] =
{
    x_bitmap::FMT_P4_ARGB_3444,
    x_bitmap::FMT_P4_ARGB_1555,
    x_bitmap::FMT_P4_ABGR_1555,
    x_bitmap::FMT_P4_RGBA_5551,
    x_bitmap::FMT_P4_RGBA_8888,
    x_bitmap::FMT_P4_ARGB_8888,
    x_bitmap::FMT_P4_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_P4_RGB_888,
    x_bitmap::FMT_P4_RGBU_8888,
    x_bitmap::FMT_P4_URGB_8888,
    x_bitmap::FMT_P4_UBGR_8888,
    x_bitmap::FMT_P4_RGB_565,
    x_bitmap::FMT_P4_RGBU_5551,
    x_bitmap::FMT_P4_UBGR_1555,
    x_bitmap::FMT_P4_URGB_1555,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_NULL
};

//--------------------------------------------------------------------------

static x_bitmap::format CONV_FMT_P8_RGB_565   [] =
{
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_ABGR_1555,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_UBGR_1555,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_P8_RGB_888   [] =
{
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_UBGR_1555,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_ABGR_1555,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_P8_URGB_8888 [] =
{
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_UBGR_1555,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_ABGR_1555,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_P8_RGBU_8888 [] =
{
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_UBGR_1555,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_ABGR_1555,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_P8_ARGB_8888 [] =
{
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_UBGR_1555,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_ABGR_1555,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_P8_RGBA_8888 [] =
{
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_ABGR_1555,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_P8_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_P8_ABGR_8888 [] =
{
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_UBGR_1555,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_ABGR_1555,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_P8_UBGR_8888 [] =
{
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_UBGR_1555,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_ABGR_1555,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_P8_RGBA_5551 [] =
{
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_ABGR_1555,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_UBGR_1555,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_P8_ARGB_1555 [] =
{
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_ABGR_1555,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_UBGR_1555,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_P8_RGBU_5551 [] =
{
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_UBGR_1555,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_ABGR_1555,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_P8_URGB_1555 [] =
{
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_UBGR_1555,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_ABGR_1555,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_P8_ABGR_1555 [] =
{
    x_bitmap::FMT_P8_ABGR_1555,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_UBGR_1555,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_NULL
};


static x_bitmap::format CONV_FMT_P8_UBGR_1555 [] =
{
    x_bitmap::FMT_P8_UBGR_1555,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_ABGR_1555,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_P8_ARGB_3444  [] =
{
    x_bitmap::FMT_P8_ARGB_3444,
    x_bitmap::FMT_P8_ARGB_1555,
    x_bitmap::FMT_P8_ABGR_1555,
    x_bitmap::FMT_P8_RGBA_5551,
    x_bitmap::FMT_P8_ARGB_8888,
    x_bitmap::FMT_P8_ABGR_8888,
    x_bitmap::FMT_P8_RGBA_8888,
    x_bitmap::FMT_16_ARGB_1555,
    x_bitmap::FMT_16_ABGR_1555,
    x_bitmap::FMT_16_RGBA_5551,
    x_bitmap::FMT_16_ARGB_4444,
    x_bitmap::FMT_16_ARGB_3444,
    x_bitmap::FMT_16_RGBA_4444,
    x_bitmap::FMT_24_ARGB_8565,
    x_bitmap::FMT_32_ARGB_8888,
    x_bitmap::FMT_32_ABGR_8888,
    x_bitmap::FMT_32_RGBA_8888,
    x_bitmap::FMT_32_BGRA_8888,
    x_bitmap::FMT_P8_RGBU_5551,
    x_bitmap::FMT_P8_URGB_1555,
    x_bitmap::FMT_P8_UBGR_1555,
    x_bitmap::FMT_P8_RGB_565,
    x_bitmap::FMT_P8_RGB_888,
    x_bitmap::FMT_P8_URGB_8888,
    x_bitmap::FMT_P8_UBGR_8888,
    x_bitmap::FMT_P8_RGBU_8888,
    x_bitmap::FMT_16_RGB_565,
    x_bitmap::FMT_16_URGB_1555,
    x_bitmap::FMT_16_UBGR_1555,
    x_bitmap::FMT_16_RGBU_5551,
    x_bitmap::FMT_24_RGB_888,
    x_bitmap::FMT_32_RGBU_8888,
    x_bitmap::FMT_32_URGB_8888,
    x_bitmap::FMT_NULL
};

//--------------------------------------------------------------------------

static x_bitmap::format CONV_FMT_I4 [] =
{
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_I8 [] =
{
    x_bitmap::FMT_NULL
};

//--------------------------------------------------------------------------

static x_bitmap::format CONV_FMT_DXT1 [] =
{
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_DXT2 [] =
{
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_DXT3 [] =
{
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_DXT4 [] =
{
    x_bitmap::FMT_NULL
};

static x_bitmap::format CONV_FMT_DXT5 [] =
{
    x_bitmap::FMT_NULL
};


////////////////////////////////////////////////////////////////////////////

static x_bitmap::format* m_ConvInfo[ x_bitmap::FMT_END + 1 ] =
{
    CONV_FMT_NULL,

    CONV_FMT_P4_RGB_565,
    CONV_FMT_P4_RGB_888,
    CONV_FMT_P4_URGB_8888,
    CONV_FMT_P4_RGBU_8888,
    CONV_FMT_P4_ARGB_8888,
    CONV_FMT_P4_RGBA_8888,
    CONV_FMT_P4_ABGR_8888,
    CONV_FMT_P4_UBGR_8888,
    CONV_FMT_P4_RGBA_5551,
    CONV_FMT_P4_ARGB_1555,
    CONV_FMT_P4_RGBU_5551,
    CONV_FMT_P4_URGB_1555,
    CONV_FMT_P4_ABGR_1555,
    CONV_FMT_P4_UBGR_1555,
    CONV_FMT_P4_ARGB_3444,

    CONV_FMT_P8_RGB_565,
    CONV_FMT_P8_RGB_888,
    CONV_FMT_P8_URGB_8888,
    CONV_FMT_P8_RGBU_8888,
    CONV_FMT_P8_ARGB_8888,
    CONV_FMT_P8_RGBA_8888,
    CONV_FMT_P8_ABGR_8888,
    CONV_FMT_P8_UBGR_8888,
    CONV_FMT_P8_RGBA_5551,
    CONV_FMT_P8_ARGB_1555,
    CONV_FMT_P8_RGBU_5551,
    CONV_FMT_P8_URGB_1555,
    CONV_FMT_P8_ABGR_1555,
    CONV_FMT_P8_UBGR_1555,
    CONV_FMT_P8_ARGB_3444,

    CONV_FMT_16_RGB_565,
    CONV_FMT_16_ARGB_4444,
    CONV_FMT_16_RGBA_4444,
    CONV_FMT_16_ARGB_1555,
    CONV_FMT_16_RGBA_5551,
    CONV_FMT_16_URGB_1555,
    CONV_FMT_16_RGBU_5551,
    CONV_FMT_16_ABGR_1555,
    CONV_FMT_16_UBGR_1555,
    CONV_FMT_16_ARGB_3444,

    CONV_FMT_24_RGB_888,
    CONV_FMT_24_ARGB_8565,

    CONV_FMT_32_RGBU_8888,
    CONV_FMT_32_URGB_8888,
    CONV_FMT_32_ARGB_8888,
    CONV_FMT_32_RGBA_8888,
    CONV_FMT_32_ABGR_8888,
    CONV_FMT_32_BGRA_8888,

    CONV_FMT_I4,
    CONV_FMT_I8,

    CONV_FMT_DXT1,
    CONV_FMT_DXT2,
    CONV_FMT_DXT3,
    CONV_FMT_DXT4,
    CONV_FMT_DXT5,

    CONV_FMT_NULL
};


////////////////////////////////////////////////////////////////////////////
// UTILITY FUNCTIONS
////////////////////////////////////////////////////////////////////////////

// This structure only exists to expose the m_NMips member
struct x_conv_bitmap : public x_bitmap
{
    x_bitmap::m_NMips;
};

//==========================================================================

static
s32 CalcDataSize( s32 BPP, s32 W, s32 H, s32 NMips )
{
    s32 i;
    s32 Size;

    if( NMips == 0 )
    {
        Size = ((BPP * W * H) / 8);
    }
    else
    {
        ASSERT( !(W & (W-1)) );
        ASSERT( !(H & (H-1)) );

        Size = 0;

        for( i = 0; i < NMips; i++ )
        {
            ASSERT( W >= 8 );
            ASSERT( H >= 8 );

            Size += ((BPP * W * H) / 8);
            Size += sizeof(miptable);

            W >>= 1;
            H >>= 1;
        }

        // Align bitmap data on 32-byte boundary by padding miptable size
        if( ((NMips * sizeof(miptable)) & 0x1F) != 0 )
            Size += 0x20 - ((NMips * sizeof(miptable)) & 0x1F);
    }

    return Size;
}

//==========================================================================

static
void SetupMipTable( byte* pRawData, s32 BPP, s32 W, s32 H, s32 NMips )
{
    ASSERT( pRawData != NULL );

    s32 i;
    s32 CurOffset = NMips * sizeof(miptable);

    miptable* pMipTable = (miptable*)pRawData;

    // verify width and height are powers of 2
    ASSERT( !(W & (W-1)) );
    ASSERT( !(H & (H-1)) );

    // Align bitmap data on 32-byte boundary by padding miptable size
    if( ((NMips * sizeof(miptable)) & 0x1F) != 0 )
        CurOffset += 0x20 - ((NMips * sizeof(miptable)) & 0x1F);

    // fill out the mip table
    for( i = 0; i < NMips; i++ )
    {
        ASSERT( W >= 8 );
        ASSERT( H >= 8 );

        pMipTable[i].W      = W;
        pMipTable[i].H      = H;
        pMipTable[i].Offset = CurOffset;

        CurOffset += ((BPP * W * H) / 8);
        W >>= 1;
        H >>= 1;
    }
}


////////////////////////////////////////////////////////////////////////////
// CONVERSION PROTOTYPES
////////////////////////////////////////////////////////////////////////////

static xbool CONV_Identity       ( x_bitmap& SrcBMP, x_bitmap::format SrcFormat, 
                                   x_bitmap& DstBMP, x_bitmap::format DstFormat );
static xbool CONV_PalToPal       ( x_bitmap& SrcBMP, x_bitmap::format SrcFormat, 
                                   x_bitmap& DstBMP, x_bitmap::format DstFormat );
static xbool CONV_PalToIntensity ( x_bitmap& SrcBMP, x_bitmap::format SrcFormat, 
                                   x_bitmap& DstBMP, x_bitmap::format DstFormat );
static xbool CONV_XToNonPaletized( x_bitmap& SrcBMP, x_bitmap::format SrcFormat, 
                                   x_bitmap& DstBMP, x_bitmap::format DstFormat );

////////////////////////////////////////////////////////////////////////////

static
bmp_conv_fnptr DecideConversion( x_bitmap::format SrcFormat, x_bitmap::format DstFormat )
{
    x_bitmap::fmt_desc SrcFmt;
    x_bitmap::fmt_desc DstFmt;

    ASSERT( SrcFormat >= x_bitmap::FMT_NULL && SrcFormat < x_bitmap::FMT_END );
    ASSERT( DstFormat >= x_bitmap::FMT_NULL && DstFormat < x_bitmap::FMT_END );

    // Check for NULL format
    if( (SrcFormat == x_bitmap::FMT_NULL) || (DstFormat == x_bitmap::FMT_NULL) )
        return NULL;

    // Check for Identity
    if( SrcFormat == DstFormat )
        return (bmp_conv_fnptr)CONV_Identity;

    // Setup format info ptrs
    x_bitmap::GetFormatDesc( SrcFmt, SrcFormat );
    x_bitmap::GetFormatDesc( DstFmt, DstFormat ); 

    // Look for conversion to Intensity.
    if( (DstFormat == x_bitmap::FMT_I4) || 
        (DstFormat == x_bitmap::FMT_I8) )
    {
        if( SrcFmt.IsClutUsed )
            return (bmp_conv_fnptr)CONV_PalToIntensity;
        else
            return( NULL );
    }

    // Check for XToNonPaletized
    if( DstFmt.IsClutUsed == FALSE )
        return (bmp_conv_fnptr)CONV_XToNonPaletized;

    // Check for Clut change
    if( SrcFmt.IsClutUsed && DstFmt.IsClutUsed )
        return (bmp_conv_fnptr)CONV_PalToPal;

    // Unknown conversion
    return NULL;
}

///////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////////////////////

xbool AUXBMP_Convert( x_bitmap& DstBMP, x_bitmap::format DstFormat, x_bitmap& SrcBMP )
{
    x_bitmap::format    SrcFormat;
    bmp_conv_fnptr      Conv;
    x_bitmap            Temp;
    xbool               Error;

    ASSERT( (DstFormat >= x_bitmap::FMT_NULL) && (DstFormat < x_bitmap::FMT_END) );

    // Choose function that knows how to convert bitmap
    SrcFormat = SrcBMP.GetFormat();
    Conv      = DecideConversion( SrcFormat, DstFormat );

    if( Conv == NULL )
        return FALSE;

    // Convert the Src Bitmap to Dst Bitmap
    Error = Conv( SrcBMP, SrcFormat, Temp, DstFormat );

    DstBMP.KillBitmap();

    DstBMP = Temp;

    return Error;
}

///////////////////////////////////////////////////////////////////////////

x_bitmap::format AUXBMP_FindBestFormat( x_bitmap::format SrcFormat, xbool* CanUseFormat )
{
    s32 i;

    ASSERT( (SrcFormat >= x_bitmap::FMT_NULL) && (SrcFormat < x_bitmap::FMT_END) );

    i = 0;

    while( ( !CanUseFormat[ m_ConvInfo[(s32)SrcFormat][i] ] ) &&
           ( m_ConvInfo[(s32)SrcFormat][i] != x_bitmap::FMT_END ) )
    {
        i++;
    }

    return m_ConvInfo[SrcFormat][i];
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// CONVERSION FUNCTIONS
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static
xbool CONV_Identity( x_bitmap& SrcBMP, x_bitmap::format SrcFormat, 
                     x_bitmap& DstBMP, x_bitmap::format DstFormat )
{
    ASSERT( SrcFormat == DstFormat );

    DstBMP = SrcBMP;

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////

static
xbool CONV_XToNonPaletized( x_bitmap& SrcBMP, x_bitmap::format SrcFormat, 
                            x_bitmap& DstBMP, x_bitmap::format DstFormat )
{
    s32   x, y;
    byte* Data;
    s32   DataSize;
    s32   NMips;
    s32   CurMip;
    color C;

    x_bitmap::fmt_desc FSrc;
    x_bitmap::fmt_desc FDst;

    // Get ptrs to the format info and confirm formats
    x_bitmap::GetFormatDesc( FSrc, SrcFormat );
    x_bitmap::GetFormatDesc( FDst, DstFormat );

    ASSERT( (FSrc.BPC == 16) || (FSrc.BPC == 24) || (FSrc.BPC == 32) );
    ASSERT( (FDst.BPC == 16) || (FDst.BPC == 24) || (FDst.BPC == 32) );
    ASSERT( (FDst.BPP == 16) || (FDst.BPP == 24) || (FDst.BPP == 32) );

    NMips = SrcBMP.GetNMips();

    // Allocate bitmap for data
    DataSize = CalcDataSize( FDst.BPP, SrcBMP.GetPWidth(), SrcBMP.GetPHeight(), NMips );
    Data     = (byte*)x_malloc( DataSize );
    ASSERT( Data != NULL );

    DstBMP.KillBitmap();

    // Setup miptable if there are mips
    if( NMips > 0 )
        SetupMipTable( Data, FDst.BPP, SrcBMP.GetPWidth(0), SrcBMP.GetPHeight(0), NMips );

    // Build the bitmap header
    DstBMP.SetupBitmap( DstFormat,              // Format
                        SrcBMP.GetWidth(),      // Width
                        SrcBMP.GetHeight(),     // Height
                        SrcBMP.GetPWidth(),     // PWidth
                        SrcBMP.GetPHeight(),    // PHeight
                        TRUE,                   // IsDataOwned,
                        DataSize,               // DataSizeInBytes
                        Data,                   // PixelData
                        FALSE,                  // IsClutOwned
                        0,                      // ClutSizeInBytes
                        NULL,                   // ClutData
                        0,                      // NClutColors
                        0 );                    // NMipsToBuild

    // Manually set the number of mips(since they will be converted instead of generated)
    ((x_conv_bitmap*)&DstBMP)->m_NMips = NMips;

    // This is done to make loop execute at least once
    if( NMips == 0 )
        NMips = 1;

    for( CurMip = 0; CurMip < NMips; CurMip++ )
    {
        // Setup format converters
        x_bitmap::SetPixelFormat( DstBMP, SrcBMP, 0, 0, CurMip );

        // Process all colors
        for( y = 0; y < SrcBMP.GetHeight( CurMip ); y++ )
        {
            for( x = 0; x < SrcBMP.GetWidth( CurMip ); x++ )
            {
                C = x_bitmap::GetPixelColor( x, y, CurMip );
                x_bitmap::SetPixelColor( x, y, C, CurMip );
            }
        }
    }

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////

static
xbool CONV_PalToPal( x_bitmap& SrcBMP, x_bitmap::format SrcFormat, 
                     x_bitmap& DstBMP, x_bitmap::format DstFormat )
{
    s32   i, x, y;
    byte* PixelData;
    byte* ClutData;
    s32   DataSize;
    s32   ClutSize;
    s32   NClutColors;
    s32   DPhysW;       // IN PIXELS
    s32   NMips;
    s32   CurMip;
    color C;

    x_bitmap::fmt_desc  FSrc;
    x_bitmap::fmt_desc  FDst;

    // Get ptrs to the format info and confirm formats
    x_bitmap::GetFormatDesc( FSrc, SrcFormat );
    x_bitmap::GetFormatDesc( FDst, DstFormat );
    ASSERT( (FSrc.IsClutUsed) && (FDst.IsClutUsed) );

    NMips = SrcBMP.GetNMips();

    // Allocate pixel data.
    // Make sure physical bytes per line is integral!
    DPhysW    = ALIGN_8( SrcBMP.GetPWidth() * FDst.BPP ) / FDst.BPP;
    DataSize  = CalcDataSize( FDst.BPP, DPhysW, SrcBMP.GetPHeight(), NMips );
    PixelData = (byte*)x_malloc( DataSize );
    ASSERT( PixelData != NULL );

    // Decide on number of colors in CLUT
    NClutColors = SrcBMP.GetNClutColors();
    NClutColors = MIN( NClutColors, (1<<FDst.BPP) );

    // Allocate clut data
    ClutSize = (FDst.BPC * NClutColors) / 8;
    ClutData = (byte*)x_malloc( ClutSize );
    ASSERT( ClutData != NULL );

    DstBMP.KillBitmap();

    // Setup miptable if there are mips
    if( NMips > 0 )
        SetupMipTable( PixelData, FDst.BPP, DPhysW, SrcBMP.GetPHeight(0), NMips );

    // Build the bitmap header
    DstBMP.SetupBitmap( DstFormat,              // Format
                        SrcBMP.GetWidth(),      // Width
                        SrcBMP.GetHeight(),     // Height
                        DPhysW,                 // PWidth
                        SrcBMP.GetPHeight(),    // PHeight
                        TRUE,                   // IsDataOwned,
                        DataSize,               // DataSizeInBytes
                        PixelData,              // PixelData
                        TRUE,                   // IsClutOwned
                        ClutSize,               // ClutSizeInBytes
                        ClutData,               // ClutData
                        NClutColors,            // NClutColors
                        0 );                    // NMipsToBuild

    // Manually set the number of mips(since they will be converted instead of generated)
    ((x_conv_bitmap*)&DstBMP)->m_NMips = NMips;

    // This is done to make loop execute at least once
    if( NMips == 0 )
        NMips = 1;

    for( CurMip = 0; CurMip < NMips; CurMip++ )
    {
        // Setup format converters
        x_bitmap::SetPixelFormat( DstBMP, SrcBMP, 0, 0, CurMip );

        // Process all pixels
        for( y = 0; y < SrcBMP.GetHeight( CurMip ); y++ )
        {
            for( x = 0; x < SrcBMP.GetWidth( CurMip ); x++ )
            {
                i = x_bitmap::GetPixelIndex( x, y, CurMip );
                x_bitmap::SetPixelIndex( x, y, i, CurMip );
            }
        }
    }

    // Process all clut colors
    for( i = 0; i < DstBMP.GetNClutColors(); i++ )
    {
        C = x_bitmap::GetClutColor( i );
        x_bitmap::SetClutColor( i, C );
    }

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////

static
xbool CONV_PalToIntensity( x_bitmap& SrcBMP, x_bitmap::format SrcFormat, 
                           x_bitmap& DstBMP, x_bitmap::format DstFormat )
{
    s32   i, x, y;
    byte* PixelData;
    s32   DataSize;
    s32   NClutColors;
    s32   DPhysW;       // IN PIXELS
    s32   NMips;
    s32   CurMip;

    x_bitmap::fmt_desc  FSrc;
    x_bitmap::fmt_desc  FDst;

    // Get ptrs to the format info and confirm formats
    x_bitmap::GetFormatDesc( FSrc, SrcFormat );
    x_bitmap::GetFormatDesc( FDst, DstFormat );
    ASSERT( (FSrc.IsClutUsed) && (FDst.IsClutUsed) );

    NMips = SrcBMP.GetNMips();

    // Allocate pixel data.  
    // Make sure physical bytes per line is integral!
    DPhysW    = ALIGN_8( SrcBMP.GetPWidth() * FDst.BPP ) / FDst.BPP;
    DataSize  = CalcDataSize( FDst.BPP, DPhysW, SrcBMP.GetPHeight(), NMips );
    PixelData = (byte*)x_malloc( DataSize );
    ASSERT( PixelData != NULL );

    // Decide on number of colors in CLUT
    NClutColors = SrcBMP.GetNClutColors();
    NClutColors = MIN( NClutColors, (1<<FDst.BPP) );

    DstBMP.KillBitmap();

    // Setup miptable if there are mips
    if( NMips > 0 )
        SetupMipTable( PixelData, FDst.BPP, DPhysW, SrcBMP.GetPHeight(0), NMips );

    // Build the bitmap header
    DstBMP.SetupBitmap( DstFormat,              // Format
                        SrcBMP.GetWidth(),      // Width
                        SrcBMP.GetHeight(),     // Height
                        DPhysW,                 // PWidth
                        SrcBMP.GetPHeight(),    // PHeight
                        TRUE,                   // IsDataOwned,
                        DataSize,               // DataSizeInBytes
                        PixelData,              // PixelData
                        FALSE,                  // IsClutOwned
                        0,                      // ClutSizeInBytes
                        NULL,                   // ClutData
                        NClutColors,            // NClutColors
                        0 );                    // NMipsToBuild

    // Manually set the number of mips(since they will be converted instead of generated)
    ((x_conv_bitmap*)&DstBMP)->m_NMips = NMips;

    // This is done to make loop execute at least once
    if( NMips == 0 )
        NMips = 1;

    for( CurMip = 0; CurMip < NMips; CurMip++ )
    {
        // Setup format converters
        x_bitmap::SetPixelFormat( DstBMP, SrcBMP, 0, 0, CurMip );

        // Process all pixels
        for( y = 0; y < SrcBMP.GetHeight( CurMip ); y++ )
        {
            for( x = 0; x < SrcBMP.GetWidth( CurMip ); x++ )
            {
                i = x_bitmap::GetPixelIndex( x, y, CurMip );

                if( (FSrc.BPP == 8) && (FDst.BPP == 4) )
                    i >>= 4;

                if( (FSrc.BPP == 4) && (FDst.BPP == 8) )
                    i <<= 4;

                x_bitmap::SetPixelIndex( x, y, i, CurMip );
            }
        }
    }

    return TRUE;
}


////////////////////////////////////////////////////////////////////////////
// AUXBMP_ReduceColorIntensity
//
//  Reduces the xbitmap color intensity by some specified amount for each
//  color channel.
////////////////////////////////////////////////////////////////////////////
xbool AUXBMP_ReduceColorIntensity( x_bitmap& SrcBMP, f32 fR, f32 fG, f32 fB, f32 fA )
{
    // Check params
    ASSERTS( ((fR >= 0.0f) && (fR <= 1.0f)), "Value must be from 0.0 to 1.0" );
    ASSERTS( ((fG >= 0.0f) && (fG <= 1.0f)), "Value must be from 0.0 to 1.0" );
    ASSERTS( ((fB >= 0.0f) && (fB <= 1.0f)), "Value must be from 0.0 to 1.0" );
    ASSERTS( ((fA >= 0.0f) && (fA <= 1.0f)), "Value must be from 0.0 to 1.0" );

    // Setup format converters
    x_bitmap::SetPixelFormat( SrcBMP, SrcBMP );

    if( SrcBMP.IsClutBased() )
    {
        s32   i;
        color c;

        for( i = 0; i < SrcBMP.GetNClutColors(); i++ )
        {
            c = SrcBMP.GetClutColor( i );
            c.R = (u8)((f32)c.R * fR);
            c.G = (u8)((f32)c.G * fG);
            c.B = (u8)((f32)c.B * fB);
            c.A = (u8)((f32)c.A * fA);
            SrcBMP.SetClutColor( i, c );
        }
    }
    else
    {
        s32 x, y;
        color c;

        for( y = 0; y < SrcBMP.GetHeight(); y++ )
        {
            for( x = 0; x < SrcBMP.GetWidth(); x++ )
            {
                c = SrcBMP.GetPixelColor( x, y );
                c.R = (u8)((f32)c.R * fR);
                c.G = (u8)((f32)c.G * fG);
                c.B = (u8)((f32)c.B * fB);
                c.A = (u8)((f32)c.A * fA);
                SrcBMP.SetPixelColor( x, y, c );
            }
        }
    }

    SrcBMP.SetReducedIntensity();

    return TRUE;
}

//==========================================================================

static
color CalcTransparentColor( s32 W, s32 H )
{
    s32   i, j;
    s32   R, G, B, T;
    color AvgC;
    color C;

    R = G = B = T = 0;

    // Compute average color of texture
    for( i = 0; i < H; i++ )
    {
        for( j = 0; j < W; j++ )
        {
            C = x_bitmap::GetPixelColor( j, i );
            if( C.A >= 128 )
            {
                R += C.R;
                G += C.G;
                B += C.B;
                T++;
            }
        }
    }

    if( T )
    {
        AvgC.R = R / T;
        AvgC.G = G / T;
        AvgC.B = B / T;
    }
    else
    {
        AvgC.R = 0;
        AvgC.G = 0;
        AvgC.B = 0;
    }

    return AvgC;
}

//==========================================================================

xbool AUXBMP_ApplyAlpha( x_bitmap& BMP, 
                         x_bitmap& AlphaBMP, 
                         xbool     PunchThrough,
                         xbool     GenerateTransparentColor )
{
    s32   i, j, k;
    s32   W, H;
    s32   NColors;
    s32   NClutColors;
    byte* pAlpha;
    s32   AlphaCount[256];
    s32   AlphaTotal[256];
    color C, ClearColor;
    x_bitmap::format BMPFormat = x_bitmap::FMT_NULL;

    // Confirm that sizes and formats match
    if( AlphaBMP.GetWidth()  != BMP.GetWidth()  ) return FALSE;
    if( AlphaBMP.GetHeight() != BMP.GetHeight() ) return FALSE;
    if( AlphaBMP.GetBPP()    != BMP.GetBPP()    ) return FALSE;

    // Decide on best alpha format for data
    switch( BMP.GetBPP() ) 
    {
        case 4:     BMPFormat = x_bitmap::FMT_P4_ARGB_8888;   break;
        case 8:     BMPFormat = x_bitmap::FMT_P8_ARGB_8888;   break;
        case 16:    BMPFormat = x_bitmap::FMT_16_ARGB_1555;   break;
        case 24:    BMPFormat = x_bitmap::FMT_24_ARGB_8565;   break;
        case 32:    BMPFormat = x_bitmap::FMT_32_ARGB_8888;   break;
        default: ASSERT( FALSE ); break;
    }

    // Convert source bmp to alpha format and clear alpha
    AUXBMP_Convert( BMP, BMPFormat, BMP );
    BMP.SetAllAlpha( 0 );

    // Allocate buffer to copy out alphas
    NColors = BMP.GetWidth() * BMP.GetHeight();
    pAlpha  = (byte*)x_malloc( NColors );
    ASSERT( pAlpha != NULL );

    W = BMP.GetWidth();
    H = BMP.GetHeight();

    // Deal with color per pixel
    if( BMP.GetBPP() > 8 )
    {
        // Setup format and read alphas
        AlphaBMP.SetPixelFormat( AlphaBMP );
        k = 0;
        for( i = 0; i < H; i++ )
        {
            for( j = 0; j < W; j++ )
            {
                C = AlphaBMP.GetPixelColor( j, i );
                pAlpha[k] = (byte)((C.R + C.G + C.B) / 3);
                k++;
            }
        }

        // Setup format and write alphas
        BMP.SetPixelFormat( BMP );
        k = 0;
        for( i = 0; i < H; i++ )
        {
            for( j = 0; j < W; j++ )
            {
                C = BMP.GetPixelColor( j, i );
                C.A = pAlpha[k];

                if( PunchThrough )
                {
                    if( C.A >= 128) C.A = 255;
                    else            C.A = 0;
                }

                BMP.SetPixelColor( j, i, C );
                k++;
            }
        }

        // Build transparent color
        if( GenerateTransparentColor )
        {
            ClearColor = CalcTransparentColor( W, H );

            // Find each transparent color in image data
            for( i = 0; i < H; i++ )
            {
                for( j = 0; j < W; j++ )
                {
                    C = BMP.GetPixelColor( j, i );
                    if( C.A < 128 )
                    {
                        ClearColor.A = C.A;
                        BMP.SetPixelColor( j, i, ClearColor );
                    }
                }
            }
        }
    }
    else
    // Deal with clut
    {
        NClutColors = BMP.GetNClutColors();

        for( i = 0; i < 256; i++ )
        {
            AlphaCount[i] = 0;
            AlphaTotal[i] = 0;
        }

        // Setup format and read alphas
        AlphaBMP.SetPixelFormat( AlphaBMP );
        k = 0;
        for( i = 0; i < H; i++ )
        {
            for( j = 0; j < W; j++ )
            {
                C = AlphaBMP.GetPixelColor( j, i );
                pAlpha[k] = (byte)((C.R + C.G + C.B) / 3);
                k++;
            }
        }

        // Sum up alphas for all clut entries
        BMP.SetPixelFormat( BMP );
        k = 0;
        for( i = 0; i < H; i++ )
        {
            for( j = 0; j < W; j++ )
            {
                s32 ID;
                ID = BMP.GetPixelIndex( j, i );
                AlphaCount[ID]++;
                AlphaTotal[ID] += pAlpha[k];
                k++;
            }
        }

        // Average out alphas
        for( i = 0; i < NClutColors; i++ )
        {
            C = BMP.GetClutColor(i);
            if( AlphaCount[i] )
                C.A = (byte)(AlphaTotal[i] / AlphaCount[i]);
            else
                C.A = 255;

            if( PunchThrough )
            {
                if( C.A >= 128 ) C.A = 255;
                else             C.A = 0;
            }

            BMP.SetClutColor( i, C );
        }

        // Build transparent color
        if( GenerateTransparentColor )
        {
            ClearColor = CalcTransparentColor( W, H );

            // Find each transparent color in Clut
            for( k = 0; k < NClutColors; k++ )
            {
                C = BMP.GetClutColor( k );
                if( C.A < 128 )
                {
                    ClearColor.A = C.A;
                    BMP.SetClutColor( k, ClearColor );
                }
            }
        }
    }

    // Free alpha buffer
    x_free( pAlpha );

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////
// FINISHED
///////////////////////////////////////////////////////////////////////////
