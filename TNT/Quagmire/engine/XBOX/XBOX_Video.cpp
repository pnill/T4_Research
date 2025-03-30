////////////////////////////////////////////////////////////////////////////
//
// XBOX_Video.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////
#include "x_files.hpp"

#include "Q_Engine.hpp"
#include "Q_XBOX.hpp"

#include "XBOX_Video.hpp"

#include "xtl.h"

////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
////////////////////////////////////////////////////////////////////////////
HRESULT				g_LastError	 = NULL;
LPDIRECT3D8         g_pD3D       = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE8   g_pD3DDevice = NULL; // Quagmire's main rendering device


////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
////////////////////////////////////////////////////////////////////////////

void VIDEO_SetDefaultRenderStates( xbool bUseZBuffer );


////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

LPDIRECT3DDEVICE8 XBOX_GetDevice( void )
{
    return g_pD3DDevice;
}

//==========================================================================

err VIDEO_InitModule( xbool bUseZBuffer )
{
    D3DPRESENT_PARAMETERS   d3dpp;
    s32                     DisplayWidth;
    s32                     DisplayHeight;

    DWORD                   videoFlags;

    videoFlags = XGetVideoFlags();

    // Get the screen resolution.
    ENG_GetResolution( DisplayWidth, DisplayHeight );

    // Create the D3D object, which is needed to create the D3DDevice.
    g_pD3D = Direct3DCreate8( D3D_SDK_VERSION );
    if( g_pD3D == NULL )
        return ERR_FAILURE;

    // Set up the parameters that we would like to use for the D3D Device used by this app.
    // Settings such as windowed mode, and debug/release may play a part in how you wish to 
    // setup the device.
    ZeroMemory( &d3dpp, sizeof(d3dpp) );

    d3dpp.BackBufferCount   = 1;
    d3dpp.SwapEffect        = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferWidth   = DisplayWidth;
    d3dpp.BackBufferHeight  = DisplayHeight;

// Ok, this code appeared to work correctly, but at least on the PAL TV that I was testing on, it looked pretty screwed up
// so I guess we just won't support PAL-60 mode.
//    if( XC_VIDEO_STANDARD_PAL_I & XGetVideoStandard() && 
//        XC_VIDEO_FLAGS_PAL_60Hz & XGetVideoFlags() ) // Check for PAL-60 mode
//    {
//        d3dpp.FullScreen_RefreshRateInHz = 60;        
//    }

    if( bUseZBuffer )
    {
        d3dpp.EnableAutoDepthStencil = TRUE;
        d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
    }

    d3dpp.Windowed          = FALSE;
    d3dpp.BackBufferFormat  = D3DFMT_A8R8G8B8;///*( UseTruColor ) ? D3DFMT_A8R8G8B8 :*/ D3DFMT_R5G6B5;
	

//	d3dpp.Flags = D3DPRESENTFLAG_10X11PIXELASPECTRATIO;

	// Set up for simple full scene anti-aliasing.
//#if !defined( X_DEBUG )
//    d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
    d3dpp.MultiSampleType = D3DMULTISAMPLE_2_SAMPLES_SUPERSAMPLE_VERTICAL_LINEAR;     // Use this one!!!
//    d3dpp.MultiSampleType = D3DMULTISAMPLE_2_SAMPLES_MULTISAMPLE_QUINCUNX;
//    d3dpp.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES_MULTISAMPLE_GAUSSIAN;
//#endif

    if (videoFlags & XC_VIDEO_FLAGS_HDTV_480p)
    {
        d3dpp.Flags = D3DPRESENTFLAG_PROGRESSIVE;
    }

    // Create the D3D Device interface.
    DXCHECK( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT,
                                   D3DDEVTYPE_HAL,
                                   NULL,
                                   D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                  &d3dpp,
                                  &g_pD3DDevice ) );

    D3DRECT rect;
    rect.x1 = 0;
    rect.y1 = 0;
    rect.x2 = DisplayWidth;
    rect.y2 = DisplayHeight;
    color C(0,0,0);
    DXWARN(XBOX_GetDevice()->Clear( 1, &rect, D3DCLEAR_TARGET, C.Get(), 1.0f, 0L));
    g_pD3DDevice->Present( NULL, NULL, NULL, NULL );
//    g_pD3DDevice->SetSoftDisplayFilter( TRUE );
    g_pD3DDevice->SetFlickerFilter( 3 );

    VIDEO_SetDefaultRenderStates( bUseZBuffer );

    return ERR_SUCCESS;
}

//==========================================================================

void VIDEO_SetDefaultRenderStates( xbool bUseZBuffer )
{
    f32 pointSize = 1.0f;
    f32 pointMin  = 0.0f;
    f32 pointMax  = 1.0f;
    f32 fBias     = -0.6f;
    s8  TextureStageID;

    g_pD3DDevice->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_RED  );

    // Cull Primitives assembles in Clockwise order.
    DXWARN( g_pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE ) );

    // Set the ambient light
    ENG_SetAmbientLight( color(128,128,128,255) );

    // Disable D3D Real Time Lighting.
    DXWARN( g_pD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE ) );

    // Set the current lighting shade mode.
    DXWARN( g_pD3DDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD ) );

    // Enable Per vertex color.
    DXWARN( g_pD3DDevice->SetRenderState( D3DRS_COLORVERTEX, TRUE ) );

    // Disable D3D Clipping.
    //DXWARN( g_pD3DDevice->SetRenderState( D3DRS_CLIPPING, FALSE ) );

    // Enable auto vertex normal normalizing. (Are we sure that we want this?  Very processor intensive)
    DXWARN( g_pD3DDevice->SetRenderState( D3DRS_NORMALIZENORMALS, FALSE ) );

    // Disable point sprites.
    DXWARN( g_pD3DDevice->SetRenderState( D3DRS_POINTSPRITEENABLE, FALSE ) );

    // Disable scalable points primitives.
    DXWARN( g_pD3DDevice->SetRenderState( D3DRS_POINTSCALEENABLE, FALSE ) );

    // Set the default, min, and max point sizes
    DXWARN( g_pD3DDevice->SetRenderState( D3DRS_POINTSIZE,     *((DWORD*)(&pointSize))) );
    DXWARN( g_pD3DDevice->SetRenderState( D3DRS_POINTSIZE_MIN, *((DWORD*)(&pointMin)) ) );
    DXWARN( g_pD3DDevice->SetRenderState( D3DRS_POINTSIZE_MAX, *((DWORD*)(&pointMax)) ) );
    DXWARN( g_pD3DDevice->SetRenderState( D3DRS_POINTSCALE_A,  *((DWORD*)(&pointMax)) ) );
    DXWARN( g_pD3DDevice->SetRenderState( D3DRS_POINTSCALE_B,  *((DWORD*)(&pointMax)) ) );
    DXWARN( g_pD3DDevice->SetRenderState( D3DRS_POINTSCALE_C,  *((DWORD*)(&pointMax)) ) );

    // Set Z Buffering state.
    if( bUseZBuffer )
    {
        DXWARN( g_pD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE ) );
    }
    else
    {
        DXWARN( g_pD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE ) );
    }

    // Set on the Z Buffering Mode.
    DXWARN( g_pD3DDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL ) );

    // Set the primitive fill mode.
    DXWARN( g_pD3DDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID ) );

    // Disable Alpha blending.
    DXWARN( g_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE ) );

    // Disable the Alpha test function.
    DXWARN( g_pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE ) );

    // Set the default alpha test value.
    DXWARN( g_pD3DDevice->SetRenderState( D3DRS_ALPHAREF, 128 ) );

    // Set the default Alpha test
    DXWARN( g_pD3DDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL ) );

    // Set Defaults for the first texture stage.

    // Set the color modes.
    DXWARN( g_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_DISABLE ) );
    DXWARN( g_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE  ) );
    DXWARN( g_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE  ) );
    // Set the alpha modes.
    DXWARN( g_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE ) );
    DXWARN( g_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE  ) );
    DXWARN( g_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE  ) );
    // Set the Texture Scaling methods.
    DXWARN( g_pD3DDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR ) );
    DXWARN( g_pD3DDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR ) );
    // Set the MipMap modes.
//    DXWARN( g_pD3DDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR ) );
    DXWARN( g_pD3DDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_POINT ) );
    DXWARN( g_pD3DDevice->SetTextureStageState( 0, D3DTSS_MIPMAPLODBIAS, *((LPDWORD)&fBias) ) );
    DXWARN( g_pD3DDevice->SetTextureStageState( 0, D3DTSS_MAXMIPLEVEL, 0 ) );
    // Set other texture state defaults.
    DXWARN( g_pD3DDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE ) );
    DXWARN( g_pD3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 ) );

    //DXWARN(g_pD3DDevice->SetTextureStageState( 0, (D3DTEXTURESTAGESTATETYPE)(D3DTSS_ADDRESSU | D3DTSS_ADDRESSV | D3DTSS_ADDRESSW), D3DTADDRESS_CLAMP ));

    // Fill in the other texture stages to default values
    for( TextureStageID = 1; TextureStageID < 4; TextureStageID++ )
    {
        DXWARN( g_pD3DDevice->SetTextureStageState( TextureStageID, D3DTSS_COLOROP,   D3DTOP_DISABLE ) );
        DXWARN( g_pD3DDevice->SetTextureStageState( TextureStageID, D3DTSS_ALPHAOP,   D3DTOP_DISABLE ) );

        DXWARN( g_pD3DDevice->SetTextureStageState( TextureStageID, D3DTSS_MINFILTER, D3DTEXF_LINEAR ) );
        DXWARN( g_pD3DDevice->SetTextureStageState( TextureStageID, D3DTSS_MAGFILTER, D3DTEXF_LINEAR ) );

//        DXWARN( g_pD3DDevice->SetTextureStageState( TextureStageID, D3DTSS_MIPFILTER, D3DTEXF_LINEAR ) );
        DXWARN( g_pD3DDevice->SetTextureStageState( TextureStageID, D3DTSS_MIPFILTER, D3DTEXF_POINT ) );

        DXWARN( g_pD3DDevice->SetTextureStageState( TextureStageID, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE ) );
        DXWARN( g_pD3DDevice->SetTextureStageState( TextureStageID, D3DTSS_TEXCOORDINDEX, TextureStageID ) );
    }

    // Clear the buffers.
    g_pD3DDevice->Clear( 0L, NULL, D3DCLEAR_STENCIL | D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0L );
//    g_pD3DDevice->Present( NULL, NULL, NULL, NULL );

//    g_pD3DDevice->Clear( 0L, NULL, D3DCLEAR_STENCIL |D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0L );
//    g_pD3DDevice->Present( NULL, NULL, NULL, NULL );
}

//==========================================================================

void VIDEO_KillModule( void )
{
    SAFE_RELEASE( g_pD3DDevice );
    SAFE_RELEASE( g_pD3D );
}

//==========================================================================

void XBOX_Warning( HRESULT hr, char* pFile, u32 line )
{
//  ASSERTS( hr >= 0, fs( "DXWARN FAIL: HR=%X, Line %ld in %s", hr, (s32)line, pFile ) );

    if( hr < 0 )
        x_printf( "%s\n", fs( "DXWARN FAIL: HR=%X, Line %ld in %s", hr, (s32)line, pFile ) );
}

//==========================================================================
