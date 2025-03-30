//=========================================================================
//
//  PC_VIDEO.CPP
//
//=========================================================================
#include "x_files.hpp"
#include "Q_Engine.hpp"
#include "PC_Video.hpp"

//=========================================================================
// DEFINES
//=========================================================================


//=========================================================================
// GLOBAL VARIABLES
//=========================================================================
LPDIRECT3D8             g_pD3D       = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE8       g_pD3DDevice = NULL; // Quagmire's main rendering device


//=========================================================================
// LOCAL FUNCTIONS
//=========================================================================
void VIDEO_SetDefaultRenderStates( xbool bUseZBuffer );


//-----------------------------------------------------------------------------
// Name: VIDEO_InitModule()
// Desc: Initializes Direct3D
//-----------------------------------------------------------------------------
err VIDEO_InitModule( HWND hWnd, xbool bWindowed, xbool bUseZBuffer )
{
    D3DPRESENT_PARAMETERS	d3dpp;
	s32						DisplayWidth, DisplayHeight;

	// Get the screen resolution.
	ENG_GetResolution(DisplayWidth ,DisplayHeight);
	
	// Create the D3D object, which is needed to create the D3DDevice.
	g_pD3D = Direct3DCreate8( D3D_SDK_VERSION );
	if (g_pD3D == NULL)
		return ERR_FAILURE;

	// Set up the parameters that we would like to use for the D3D Device used by this app.
	// Settings such as windowed mode, and debug/release may play a part in how you wish to 
	// setup the device.
	ZeroMemory( &d3dpp, sizeof(d3dpp) );

    d3dpp.BackBufferCount	= 1;
    d3dpp.SwapEffect        = D3DSWAPEFFECT_COPY_VSYNC; //D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow     = hWnd;
	d3dpp.BackBufferWidth	= DisplayWidth;
	d3dpp.BackBufferHeight  = DisplayHeight;


	if (bUseZBuffer)
	{
	    d3dpp.EnableAutoDepthStencil = TRUE;
	    d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	}

	// If the application is windowed, then the current display mode is needed to determine what the back buffers
	// format should be.
    if( bWindowed )
    {
        D3DDISPLAYMODE	d3ddm;
		HRESULT			Error;

        // Get the desktop window info
        Error = g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );
        ASSERT( Error == 0 );
                 
        // fill the struct
        d3dpp.Windowed			= TRUE;
        d3dpp.BackBufferFormat  = d3ddm.Format;
    }
    else
    {
        d3dpp.Windowed          = FALSE;
        d3dpp.BackBufferFormat  = D3DFMT_A8R8G8B8;///*( UseTruColor ) ? D3DFMT_A8R8G8B8 :*/ D3DFMT_R5G6B5;
    }


    // Create the D3D Device interface.
    DXCHECK(g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
								  D3DCREATE_HARDWARE_VERTEXPROCESSING,
								  //D3DCREATE_SOFTWARE_VERTEXPROCESSING,
								  &d3dpp, &g_pD3DDevice ));



	VIDEO_SetDefaultRenderStates( bUseZBuffer );
   
    return ERR_SUCCESS;
}


//=========================================================================
// VIDEO_SetDefaultRenderStates()
// Sets all of the default render states for D3D.
//=========================================================================
void VIDEO_SetDefaultRenderStates( xbool bUseZBuffer )
{
	f32 pointSize = 1.0f;
	f32	pointMin = 0.0f;
	f32	pointMax = 1.0f;
	f32	fBias = 0.0f;
	s8	TextureStageID;

	// Cull Primitives assembles in Clockwise order.
    DXWARN(g_pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE ));

    // Set the ambient light
    ENG_SetAmbientLight( color(128,128,128,255) );
	
	// Disable D3D Real Time Lighting.
	DXWARN(g_pD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE ));

	// Set the current lighting shade mode.
	DXWARN(g_pD3DDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD ));

	// Enable Per vertex color.
	DXWARN(g_pD3DDevice->SetRenderState( D3DRS_COLORVERTEX, TRUE ));

	// Disable D3D Clipping.
	//DXWARN(g_pD3DDevice->SetRenderState( D3DRS_CLIPPING, FALSE ));

	// Enable auto vertex normal normalizing. (Are we sure that we want this?  Very processor intensive)
	DXWARN(g_pD3DDevice->SetRenderState( D3DRS_NORMALIZENORMALS, FALSE ));

	// Disable point sprites.
	DXWARN(g_pD3DDevice->SetRenderState( D3DRS_POINTSPRITEENABLE, FALSE ));

	// Disable scalable points primitives.
	DXWARN(g_pD3DDevice->SetRenderState( D3DRS_POINTSCALEENABLE,  FALSE ));

	// Set the default point size for point sizes are not specified for each vertex.
	DXWARN(g_pD3DDevice->SetRenderState( D3DRS_POINTSIZE,     *((DWORD*)(&pointSize)) ));

	// Set the min and max point size for points.
	DXWARN(g_pD3DDevice->SetRenderState( D3DRS_POINTSIZE_MIN, *((DWORD*)(&pointMin)) ));
	DXWARN(g_pD3DDevice->SetRenderState( D3DRS_POINTSIZE_MAX, *((DWORD*)(&pointMax)) ));

	DXWARN(g_pD3DDevice->SetRenderState( D3DRS_POINTSCALE_A,  *((DWORD*)(&pointMax)) ));
	DXWARN(g_pD3DDevice->SetRenderState( D3DRS_POINTSCALE_B,  *((DWORD*)(&pointMax)) ));
	DXWARN(g_pD3DDevice->SetRenderState( D3DRS_POINTSCALE_C,  *((DWORD*)(&pointMax)) ));

	// Set Z Buffering state.
	if( bUseZBuffer )
	{
		DXWARN(g_pD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE ));
	}
	else
	{
		DXWARN(g_pD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE ));
	}

	// Set on the Z Buffering Mode.
	DXWARN(g_pD3DDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL ));

	// Set the primitive fill mode.
	DXWARN(g_pD3DDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID ));

	// Disable Alpha blending.
	DXWARN(g_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE ));

    // Disable the Alpha test function.
	DXWARN(g_pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE ));

    // Set the default alpha test value.
    DXWARN(g_pD3DDevice->SetRenderState( D3DRS_ALPHAREF, 128 ));

    // Set the default Alpha test
    DXWARN(g_pD3DDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL ));


	// Set Defaults for the first texture stage.
	//-------------------------------------------------------------------------------------------------------------------------------------
	// Set the color modes.
    DXWARN(g_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_DISABLE));
    DXWARN(g_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ));
    DXWARN(g_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE ));
	// Set the alpha modes.
    DXWARN(g_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE));
    DXWARN(g_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ));
    DXWARN(g_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE ));
	// Set the Texture Scaling methods.
    DXWARN(g_pD3DDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR ));
    DXWARN(g_pD3DDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR ));
	// Set the MipMap modes.
    DXWARN(g_pD3DDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR ));
	DXWARN(g_pD3DDevice->SetTextureStageState( 0, D3DTSS_MIPMAPLODBIAS, *((LPDWORD) (&fBias)) ));
	DXWARN(g_pD3DDevice->SetTextureStageState( 0, D3DTSS_MAXMIPLEVEL, 0 ));
	// Set other texture state defaults.
    DXWARN(g_pD3DDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE ));
    DXWARN(g_pD3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 ));

	// Fill in the other texture stages to default values
	for( TextureStageID = 1; TextureStageID < 4; TextureStageID++)
	{
	    DXWARN(g_pD3DDevice->SetTextureStageState( TextureStageID, D3DTSS_COLOROP,   D3DTOP_DISABLE));
	    DXWARN(g_pD3DDevice->SetTextureStageState( TextureStageID, D3DTSS_ALPHAOP,   D3DTOP_DISABLE));

		DXWARN(g_pD3DDevice->SetTextureStageState( TextureStageID, D3DTSS_MINFILTER, D3DTEXF_LINEAR ));
		DXWARN(g_pD3DDevice->SetTextureStageState( TextureStageID, D3DTSS_MAGFILTER, D3DTEXF_LINEAR ));

		DXWARN(g_pD3DDevice->SetTextureStageState( TextureStageID, D3DTSS_MIPFILTER, D3DTEXF_NONE ));

		DXWARN(g_pD3DDevice->SetTextureStageState( TextureStageID, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE ));
	    DXWARN(g_pD3DDevice->SetTextureStageState( TextureStageID, D3DTSS_TEXCOORDINDEX, TextureStageID ));
	}

    // Clear the buffers.
	g_pD3DDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0xFFFFFFFF, 1.0f, 0L );


}




//=========================================================================
// VIDEO_KillModule()
// Releases all previously initialized objects
//=========================================================================
void VIDEO_KillModule()
{
	SAFE_RELEASE(g_pD3DDevice);
    SAFE_RELEASE(g_pD3D);
}


//=========================================================================
// VIDEO_Warning
// Prints any D3D error messages in a format that can be double
// clicked to take you to the error line.
//=========================================================================
void VIDEO_Warning(HRESULT hr, char* pFile, u32 line)
{
	const char	*pMessage;
	static char	*pError = "error";
	static char	*pWarning = "warning";
	char	*pType;

	if (hr <0)
		pType = pError;
	else
		pType = pWarning;

	pMessage = DXGetErrorString8(hr);

	if (pMessage)
	{
		x_printf("%s(%d) : %s (%08x)(%s)\n", pFile, line, pType, hr, pMessage);
	}
}

//=========================================================================