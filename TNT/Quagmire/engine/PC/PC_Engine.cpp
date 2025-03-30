//=====================================================================================================================================
//
//  PC_Engine.CPP
//
//=====================================================================================================================================
#include "x_files.hpp"

#include "Q_Engine.hpp"
#include "Q_PC.hpp"
#include "PC_Draw.hpp"
#include "Q_SMem.hpp"
#include "Q_View.hpp"
#include "Q_Draw.hpp"
#include "E_SMem.hpp"
#include "E_AccumTimer.hpp"
#include "E_Text.hpp"
#include "PC_Input.hpp"
#include "PC_Font.hpp"
#include "PC_Video.hpp"
#include "PC_MemoryUnit.hpp"

color eng_fog_color(0,0,0,0);
eng_fog g_EngineFog={0,0,0,0};


//=====================================================================================================================================
// Functions that are to be defined written by the application using the Quagmire Engine.
xbool QuagInit( void );
s32   QuagMain( s32 argc, char* argv[] );


//=====================================================================================================================================
// Local Functions.
static err MakeWindow(	HINSTANCE   hInstance,		// In:  Instance of the aplication
						DWORD		dwStyle,		// In:  Style of the window. (See windows help );
						DWORD		dwStyleEx,		// In:  Extended styles of the window (See windows help)
						HWND        hParent);		// In:  The Parent windows handle. NULL if not parents


// =====================================================================================================================================
// Globals
// Engine status
static xbool			s_EngActivated = FALSE;
static xbool			s_bZBuffer;					// Value to indicate that a ZBUFFER was created at initialization.
static xbool			s_InsideRenderMode;
static xbool			s_InsideScene;
static xbool			s_bClearBuffers = TRUE;
static color			s_BackgroundColor(0,0,0);
static s32				s_SMemSize  = -1;

// Engine Timers and Counters
static f64				s_StartRunTime;
static accum_timer		s_FPSTimer;
static accum_timer		s_CPUTimer;
static u32				s_Frame;


// Engine matricies
static matrix4          s_MWorldToScreen;

// Windows handling, View and clipping.
static xbool			s_bWindowed;
static HWND				s_hWnd;
static HINSTANCE		s_hInstance;
static message_fnptr	s_pfMessageHandler = NULL;
static char*			s_WindowTitle;

static D3DVIEWPORT8		s_Viewport;
static view				s_View;
static s32				s_XRes;
static s32				s_YRes;
static s32				s_2DClipL;
static s32				s_2DClipT;
static s32				s_2DClipR;
static s32				s_2DClipB;

// Render and Blending flags
static u32				s_RenderFlags   = ENG_ZBUFFER_TEST_ON | ENG_ZBUFFER_FILL_ON | ENG_ALPHA_TEST_OFF | ENG_ALPHA_BLEND_OFF;
static u32				s_BlendFlags    = ENG_BLEND_NORMAL;
static u8               s_FixedAlpha    = 128;

static lighting         s_ActiveLighting;
static matrix4          s_LightingDir;
static matrix4          s_LightingColor;

static D3DLIGHT8        s_Light0;
static D3DLIGHT8        s_Light1;
static D3DLIGHT8        s_Light2;

//=========================================================================
// FUNCTIONS
//=========================================================================
void ENG_SetBlendMode( u32 BlendFlags, u8 FixedAlpha )
{
    ASSERT( BlendFlags );

    // Make sure that there is a difference in the current states and the new states to authorize a change.
	if( (BlendFlags == s_BlendFlags) && (FixedAlpha == s_FixedAlpha) )
		return;

    // Save the blend flags and fixed alpha flags.
    s_BlendFlags = BlendFlags;
    s_FixedAlpha = FixedAlpha;

	// If there is a request to set a fixed alpha, set it.
	if( BlendFlags & ENG_BLEND_FIXED_ALPHA )
    {
        DXWARN(g_pD3DDevice->SetRenderState( D3DRS_TEXTUREFACTOR , D3DCOLOR_ARGB( FixedAlpha, 255, 255, 255 ) ));
    }

	//--- set the blending equation
	switch( BlendFlags & 0x0F )
	{
		case ENG_BLEND_NORMAL:
            // (SC * SA) + (DC * (1 - SA))
            DXWARN(g_pD3DDevice->SetRenderState( D3DRS_BLENDOP,   D3DBLENDOP_ADD ));
            DXWARN(g_pD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA ));
            DXWARN(g_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA ));
			break;

		case ENG_BLEND_ADDITIVE:
            // (SC * SA) + (DC)
            DXWARN(g_pD3DDevice->SetRenderState( D3DRS_BLENDOP,   D3DBLENDOP_ADD ));
            DXWARN(g_pD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA ));
            DXWARN(g_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE ));
			break;

		case ENG_BLEND_SUBTRACTIVE:
            // (DC * DA)) - (SC * 1-DA)
            DXWARN(g_pD3DDevice->SetRenderState( D3DRS_BLENDOP,   D3DBLENDOP_REVSUBTRACT ));
            DXWARN(g_pD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA ));
            DXWARN(g_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA ));
			break;

		case ENG_BLEND_DEST:
            // (DC * 1)
            DXWARN(g_pD3DDevice->SetRenderState( D3DRS_BLENDOP,   D3DBLENDOP_ADD ));
            DXWARN(g_pD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ZERO ));
            DXWARN(g_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_DESTALPHA ));
			break;

		case ENG_BLEND_MULTIPLY:
            DXWARN(g_pD3DDevice->SetRenderState( D3DRS_BLENDOP,   D3DBLENDOP_ADD ));
            DXWARN(g_pD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ONE ));
            DXWARN(g_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE ));
			break;

		default:
			ASSERT( FALSE );
			break;
	}
}

//=====================================================================================================================================
void ENG_GetBlendMode( u32& BlendMode, u8& FixedAlpha )
{
    BlendMode  = s_BlendFlags;
    FixedAlpha = s_FixedAlpha;
}

//=====================================================================================================================================
void ENG_SetRenderFlags( u32 RenderFlags )
{
	// Test to see if the new render state is different from the current render state
	// If it is, then set to the new values.
	if( s_RenderFlags != RenderFlags )
	{
		u32   ZTestMode;
		u32   ZFillMode;
		xbool AlphaTestMode;
		xbool AlphaBlendMode;

		// Record the new flag values.
		s_RenderFlags = RenderFlags;

		//------------------------------
		//---   flags sanity checking
		//------------------------------
		ASSERT(( RenderFlags & ENG_ZBUFFER_TEST_ON ) | ( RenderFlags & ENG_ZBUFFER_TEST_OFF ));
		ASSERT(( RenderFlags & ENG_ZBUFFER_FILL_ON ) | ( RenderFlags & ENG_ZBUFFER_FILL_OFF ));
		ASSERT(( RenderFlags & ENG_ALPHA_TEST_ON   ) | ( RenderFlags & ENG_ALPHA_TEST_OFF   ));
		ASSERT(( RenderFlags & ENG_ALPHA_BLEND_ON  ) | ( RenderFlags & ENG_ALPHA_BLEND_OFF  ));

		// Determine which modes to use.
		ZFillMode      = (RenderFlags & ENG_ZBUFFER_FILL_ON ) ? D3DZB_TRUE       : D3DZB_FALSE;
		ZTestMode      = (RenderFlags & ENG_ZBUFFER_TEST_ON)  ? D3DCMP_LESSEQUAL : D3DCMP_ALWAYS;
		AlphaTestMode  = (RenderFlags & ENG_ALPHA_TEST_ON )   ? TRUE             : FALSE;
		AlphaBlendMode = (RenderFlags & ENG_ALPHA_BLEND_ON )  ? TRUE             : FALSE;

		// Set those modes.
		DXWARN(g_pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE,     ZFillMode ));
        DXWARN(g_pD3DDevice->SetRenderState( D3DRS_ZFUNC,            ZTestMode ));
		DXWARN(g_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, AlphaBlendMode ));
 		DXWARN(g_pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  AlphaTestMode ));
	}
}

//=====================================================================================================================================
void ENG_GetRenderFlags( u32& RenderFlags )
{
    RenderFlags = s_RenderFlags;
}

//=====================================================================================================================================
void ENG_SetBackgroundColor( color C )
{
	s_BackgroundColor = C;
}

//=====================================================================================================================================
HWND ENG_GetHWND(void)
{
	return s_hWnd;
}

//=====================================================================================================================================
HINSTANCE ENG_GetHINSTANCE(void)
{
	return s_hInstance;
}

//=====================================================================================================================================
void ENG_SetMessageHandler(message_fnptr pfHandler)
{
	s_pfMessageHandler = pfHandler;
}


//=====================================================================================================================================
xbool ENG_Activate( s32 XRes, s32 YRes, xbool ZBuffer, s32 SMemSize, char* ApplicationName )
{
    ASSERT( !s_EngActivated );
	ASSERT( SMemSize );
	ASSERT( s_hWnd == 0);

    // Set the parameters for the window, and the rest of the render environment.
    s_XRes = XRes;
    s_YRes = YRes;
    s_View.SetViewport( 0, 0, XRes, YRes );
    
    s_WindowTitle = "DX8 Quagmire";

	s_bZBuffer = ZBuffer;
    s_bWindowed = TRUE; // FALSE;

	// Make the window and show it.
	VERIFY (MakeWindow(s_hInstance, WS_CAPTION | WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_APPWINDOW, NULL));
	ASSERT(s_hWnd);
    ShowWindow  ( s_hWnd, SW_SHOWDEFAULT );
    UpdateWindow( s_hWnd );

    // Setup PC Render environment.
	x_printf("---------------------------------------\n");
    x_printf("ENGINE: Initializing render environment.\n");
	VERIFY(VIDEO_InitModule(s_hWnd, s_bWindowed, s_bZBuffer));

	// Initialize the basic draw layer.
	x_printf("ENGINE: Initializing draw.\n");
    VERIFY(DRAW_InitModule());

    // Initialize scratch memory.
    x_printf("ENGINE: Initializing scratch memory.\n");
    s_SMemSize = SMemSize;
    SMEM_InitModule(s_SMemSize);

    // Set the 2D clip area.
    ENG_Set2DClipArea( 0, 0, XRes-1, YRes-1 );
    s_InsideRenderMode = FALSE;
    s_InsideScene = FALSE;

    // Initialize the Memory Unit / Card support for the XBOX.
    // *** THIS MUST COME After the input system is initialized ***
    x_printf("ENGINE: Initializing Memory Unit Support.\n");
    MEMORY_UNIT_Create( ApplicationName );
    
    // Initalize the VRAM manager.
	x_printf("ENGINE: Initializing vram manager.\n");
    VRAM_Init();

    // Initialize the FONT drawing module
    x_printf("ENGINE: Initializing font.\n");
    VERIFY(FONT_InitModule());

	// Initialize text handling routines.
	x_printf("ENGINE: Initializing text manager.\n");
    TEXT_InitModule( s_XRes, s_YRes, 16, 16, 16, 16, FONT_RenderText );

	// Initialize input module.
	x_printf("ENGINE: Initializing controllers.\n");
    VERIFY(INPUT_InitModule(s_hWnd));

    //--- Initialize debug module
    x_printf( "ENGINE: Initializing debug module.\n" );
    extern void DEBUG_InitModule( void );
    DEBUG_InitModule();
    
	// Initialize the default lighting.
    x_printf("ENGINE: Initializing default lighting.\n");
	ENG_SetLight(0, vector3(1.0f, 0.6f, 0.2f), color(255,255,255,255));

    // Initialize timers and counters.
    s_FPSTimer.Clear();
    s_CPUTimer.Clear();
	s_Frame = 0;
    s_StartRunTime = x_GetTime();

    // Mark engine as activated and return
	x_printf("ENGINE: Initialized.\n");
	x_printf("---------------------------------------\n");
    s_EngActivated = TRUE;

    return( ERR_SUCCESS );
}

//=====================================================================================================================================
void ENG_Deactivate( void )
{
	// Make sure that the engine was activated in the first place.
    ASSERT( s_EngActivated );

	x_printf("---------------------------------------\n");
	x_printf("ENGINE: Closing...\n");

	x_printf("[input]\n");
	INPUT_KillModule();

	x_printf("[vram]\n");
	VRAM_Kill();

	x_printf("[draw]\n");
    DRAW_KillModule();

	x_printf("[font]\n");
    FONT_KillModule();

	x_printf("[video]\n");
	VIDEO_KillModule();

	x_printf("[win32]\n");

	UnregisterClass(s_WindowTitle, s_hInstance);

    s_EngActivated = FALSE;    

	x_printf("ENGINE: Closed.\n");
}

//=====================================================================================================================================
void ENG_PageFlip( xbool ClearNewScreen )
{
    // Render the text into current dlist
    ENG_BeginRenderMode();
    FONT_BeginRender();
    TEXT_RenderAllText();
    FONT_EndRender();
    ENG_EndRenderMode();
//    ENG_PrintStats( );

    // End timing of CPU
    s_CPUTimer.EndTiming();

    // Change Draw/Disp buffers 
    // End the scene
	ASSERT(s_InsideScene);
    DXWARN(g_pD3DDevice->EndScene());
	s_InsideScene = FALSE;
    
    // Present the backbuffer contents to the front buffer
    DXWARN(g_pD3DDevice->Present(NULL, NULL, NULL, NULL));

	INPUT_CheckDevices();

   // Update FPS timer
    s_FPSTimer.EndTiming();
    s_FPSTimer.StartTiming();

    // Increment to next frame
    s_Frame++;
    
    // Toggle scratch buffers and text buffers
    SMEM_Toggle();
    TEXT_SwitchBuffers();

    // Begin timing of user CPU again
    s_CPUTimer.StartTiming();
}

//=====================================================================================================================================
void ENG_GetW2S( matrix4& pM )
{
    pM = s_MWorldToScreen;
}


//=====================================================================================================================================
void ENG_SetActiveView( const view& View )
{
    D3DXMATRIX  D3DProjectionMatrix;
    matrix4     WorldToView;
    radian      Xfov,Yfov;
    s32         Left, Top, Right, Bottom;
    f32         AspectRatio;
    f32         ZNear, ZFar;

    // Copy view into active view and get ingredients
    x_memcpy( &s_View, &View, sizeof(view));

    // Get viewport bounds and other info
    s_View.GetAspectRatio( AspectRatio );
    s_View.GetViewport   ( Left, Top, Right, Bottom );
    s_View.GetZLimits    ( ZNear, ZFar );
    s_View.GetXFOV       ( Xfov );
    Yfov = 2.0f * x_atan( x_tan( Xfov / 2.0f ) / AspectRatio );
    
    // Set the world to view matrix as identity.
    s_View.GetW2VMatrix ( WorldToView );
 	g_pD3DDevice->SetTransform( D3DTS_VIEW, (D3DXMATRIX*)&WorldToView.M );

    // Set the viewport clipping bounds.
	s_Viewport.X      = Left;
	s_Viewport.Y      = Top;
	s_Viewport.Width  = (Right - Left);
	s_Viewport.Height = (Bottom - Top);
	s_Viewport.MinZ   = 0.0f;
	s_Viewport.MaxZ   = 1.0f;
	DXWARN(g_pD3DDevice->SetViewport(&s_Viewport));

    // Build and set the perspective projection matrix.
    D3DXMatrixPerspectiveFovLH( &D3DProjectionMatrix, Yfov, AspectRatio, ZNear, ZFar );
	D3DProjectionMatrix._11 = -D3DProjectionMatrix._11;     // Scale inverse along the Y axis.
    DXWARN(g_pD3DDevice->SetTransform( D3DTS_PROJECTION, &D3DProjectionMatrix ));
}

//=====================================================================================================================================
view* ENG_GetActiveView( void )
{
    return( &s_View );
}

//=====================================================================================================================================
void ENG_Set2DClipArea( s32  L, s32  T, s32  R, s32  B )
{
    s_2DClipL = L;
    s_2DClipT = T;
    s_2DClipR = R;
    s_2DClipB = B;
}

//=====================================================================================================================================
void ENG_Get2DClipArea( s32& L, s32& T, s32& R, s32& B )
{
    L = s_2DClipL;
    T = s_2DClipT;
    R = s_2DClipR;
    B = s_2DClipB;
}

//=====================================================================================================================================
void ENG_SetActiveLighting( const lighting& Lighting )
{

    s32 i;

    // Copy lighting for engine use
    x_memcpy( &s_ActiveLighting, &Lighting, sizeof(lighting) );

    // Setup lighting direction matrix.  If you push an xyz1 normal vector 
    // through this matrix you will get the 3 intensities for the three
    // directional lights in xyz and a 1.0 in the w for ambient
    s_LightingDir.Zero();
    s_LightingColor.Zero();
    for( i=0; i<3;  i++ )
    {
        if( s_ActiveLighting.Directional[i].Active )
        {
            // Setup lighting direction for this light
            vector3* pD;
            pD = &s_ActiveLighting.Directional[i].Direction;
            pD->Normalize();
            s_LightingDir.M[0][i] = pD->X;
            s_LightingDir.M[1][i] = pD->Y;
            s_LightingDir.M[2][i] = pD->Z;

            // Setup lighting color for this light
            color* pC;
            pC = &s_ActiveLighting.Directional[i].Color;
            s_LightingColor.M[i][0] = (f32)pC->R;
            s_LightingColor.M[i][1] = (f32)pC->G;
            s_LightingColor.M[i][2] = (f32)pC->B;
            s_LightingColor.M[i][3] = (f32)pC->A;
        }
    }
    s_LightingDir.M[3][3] = 1.0f;
    s_LightingColor.M[3][0] = (f32)s_ActiveLighting.Ambient.R;
    s_LightingColor.M[3][1] = (f32)s_ActiveLighting.Ambient.G;
    s_LightingColor.M[3][2] = (f32)s_ActiveLighting.Ambient.B;
    s_LightingColor.M[3][3] = (f32)s_ActiveLighting.Ambient.A;


    ///////////////////////////////////////////////////////////////////////////////
//  x_memset(&s_Material, 0, sizeof(D3DMATERIAL8));
//  s_Material.Ambient.r = 1.0f;
//  s_Material.Ambient.g = 1.0f;
//  s_Material.Ambient.b = 1.0f;
//  s_Material.Ambient.a = 1.0f;
//  s_Material.Diffuse.r = 1.0f;
//  s_Material.Diffuse.g = 1.0f;
//  s_Material.Diffuse.b = 1.0f;
//  s_Material.Diffuse.a = 1.0f;
//  s_Material.Specular.r = 0.0f;
//  s_Material.Specular.g = 1.0f;
//  s_Material.Specular.b = 0.0f;
//  s_Material.Specular.a = 1.0f;
//  s_Material.Emissive.r = 0.2f;
//  s_Material.Emissive.g = 0.2f;
//  s_Material.Emissive.b = 0.2f;
//  s_Material.Emissive.a = 1.0f;
//  s_Material.Power = 4.0f;
//  //XBOX_GetDevice()->SetMaterial(&s_Material);

    
    color* pC;

    x_memset(&s_Light0, 0, sizeof(D3DLIGHT8));
    x_memset(&s_Light1, 0, sizeof(D3DLIGHT8));
    x_memset(&s_Light2, 0, sizeof(D3DLIGHT8));

// DIRECTIONAL lights don't need position or range
// POINT lights don't need direction
    for (i=0; i<3; i++)
    {
        if( s_ActiveLighting.Directional[i].Active == FALSE )
		{
	        DXWARN( PC_GetDevice()->LightEnable( i, FALSE ) );
			continue;
		}

        D3DLIGHT8   *pLight;

        switch (i)
        {
            case 0: pLight = &s_Light0; break;
            case 1: pLight = &s_Light1; break;
            case 2: pLight = &s_Light2; break;
            default: ASSERT(FALSE); break;

        }

        pLight->Type = D3DLIGHT_DIRECTIONAL;
        pC = &s_ActiveLighting.Directional[i].Color;
        pLight->Diffuse.r = (f32)pC->R / 255.0f;
        pLight->Diffuse.g = (f32)pC->G / 255.0f;
        pLight->Diffuse.b = (f32)pC->B / 255.0f;
        pLight->Diffuse.a = 1.0f;
        pC = &s_ActiveLighting.Ambient;
        pLight->Ambient.r = (f32)pC->R / 255.0f;
        pLight->Ambient.g = (f32)pC->G / 255.0f;
        pLight->Ambient.b = (f32)pC->B / 255.0f;
        pLight->Ambient.a = 1.0f;
        pLight->Specular.r = pLight->Specular.g = pLight->Specular.b = 1.0f;
        pLight->Attenuation0 = pLight->Attenuation1 = pLight->Attenuation2 = 0.0f;
        pLight->Range = 32767.0f;
        pLight->Falloff = 0.0f;
        pLight->Theta = 0.0f;
        pLight->Phi = 0.0f;
        pLight->Position.x = 0.0f;
        pLight->Position.y = 0.0f;
        pLight->Position.z = 0.0f;

        vector3* pD;
        pD = &s_ActiveLighting.Directional[i].Direction;
		if( pD->SquaredLength() < 0.0005f )
			pD->Z = 1.0f;
        pD->Normalize();
        pLight->Direction.x = pD->X;
        pLight->Direction.y = pD->Y;
        pLight->Direction.z = pD->Z;

        DXWARN( PC_GetDevice()->SetLight( i, pLight ) );
        DXWARN( PC_GetDevice()->LightEnable( i, TRUE ) );
    }

    DWORD dwAmbient = D3DCOLOR_RGBA( s_ActiveLighting.Ambient.R,
                                     s_ActiveLighting.Ambient.G,
                                     s_ActiveLighting.Ambient.B,
                                     s_ActiveLighting.Ambient.A );

// Please note, in ASB we never really use the built in D3D lighting...
// instead we do all our lighting through vertex shaders and OP modes.
// so we don't turn on the internal D3D lighting engine
    DXWARN( PC_GetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE ) );
    DXWARN( PC_GetDevice()->SetRenderState( D3DRS_AMBIENT, dwAmbient ) );

}

//=====================================================================================================================================
//void ENG_GetLightDirAndColor( matrix4& LDir, matrix4& LColor )
//{
//    LDir = s_LightingDir;
//    LColor = s_LightingColor;
//}

//=====================================================================================================================================
void    ENG_GetActiveLighting( lighting& Lighting )
{
    Lighting = s_ActiveLighting;
}


//=========================================================================================================================================
void ENG_SetLight( s32 LightID, vector3& Dir, color& Color )
{
    D3DLIGHT8 Light;

    //
    // Here we fill up the structure for D3D. The first thing we say 
    // is the type of light that we want. In this case DIRECTIONAL. 
    // Which basically means that it doesn't have an origin. The 
    // second thing that we fill is the diffuse lighting. Basically 
    // is the color of the light. Finally we fill up the Direction. 
    // Note how we negate to compensate for the D3D way of thinking.
    //

    ZeroMemory( &Light, sizeof(Light) );   // Clear the hold structure
    Light.Type          = D3DLIGHT_DIRECTIONAL;

	Light.Diffuse.r = Color.R / 255.0f;
	Light.Diffuse.g = Color.G / 255.0f;
	Light.Diffuse.b = Color.B / 255.0f;
	Light.Diffuse.a = Color.A / 255.0f;

    Light.Specular = Light.Diffuse;

    Light.Direction.x = -Dir.X;   // Set the direction of
    Light.Direction.y = -Dir.Y;   // the light and compensate
    Light.Direction.z = -Dir.Z;   // for DX way of thinking.

    //
    // Here we set the light number zero to be the light which we just
    // describe. What is light 0? Light 0 is one of the register that 
    // D3D have for lighting. You can overwrite registers at any time. 
    // Only lights that are set in registers are use in the rendering 
    // of the scene.
    //
    g_pD3DDevice->SetLight( LightID, &Light );

    //
    // Here we enable out register 0. That way what ever we render 
    // from now on it will use register 0. The other registers are by 
    // default turn off.
    //
    g_pD3DDevice->LightEnable( LightID, TRUE );
}

//=========================================================================================================================================
void ENG_SetAmbientLight( color& Color )
{
    D3DMATERIAL8 mtrl;

    //
    // What we do here is to create a material that will be use for 
    // all the render objects. why we need to do this? We need to do 
    // this to describe to D3D how we want the light to reflected 
    // from our objects. Here you can fin more info about materials. 
    // We set the color base of the material in this case just white. 
    // Then we set the contribution for the ambient lighting, in this 
    // case 0.3f. 
    //
    ZeroMemory( &mtrl, sizeof(mtrl) );

    // Color of the material
    mtrl.Diffuse.r  = mtrl.Diffuse.g = mtrl.Diffuse.b   = 1.0f; 
    mtrl.Specular.r = mtrl.Specular.g = mtrl.Specular.b = 0.5f;
    mtrl.Power      = 50;


    // ambient light
    mtrl.Ambient.r = Color.R / 255.0f;
    mtrl.Ambient.g = Color.G / 255.0f;
    mtrl.Ambient.b = Color.B / 255.0f;
    mtrl.Ambient.a = Color.A / 255.0f;

    //
    // Finally we activate the material
    //
    g_pD3DDevice->SetMaterial( &mtrl );

    //
    // This function will set the ambient color. In this case white.
    // R=G=B=A=255. which is like saying 0xffffffff. Because the color
    // is describe in 32bits. One each of the bytes in those 32bits
    // describe a color component. You can also use a macro provided 
    // by d3d to build the color.
    //
    g_pD3DDevice->SetRenderState( D3DRS_AMBIENT, Color.Get() );
}

//==========================================================================

void    ENG_SetFogColor         ( u8 Red, u8 Green, u8 Blue )
{
    eng_fog_color.Set( Red, Green, Blue );
}

//==========================================================================

void ENG_SetFogParameters    ( xbool bEnable, f32 fStart, f32 fAccumDepth, f32 fMaxFog, f32 fDensity )
{
    g_EngineFog.bEnabled = bEnable;
    g_EngineFog.fStart = fStart;
    g_EngineFog.fDepth = fAccumDepth;
    g_EngineFog.fMaxFog = fMaxFog;
    g_EngineFog.fDensity = fDensity;
}

//==========================================================================

void ENG_ActivateFog( xbool bEnable )
{
    (void)bEnable;
}

//=====================================================================================================================================

void ENG_BeginRenderMode( void )
{
    ASSERT( !s_InsideRenderMode );
    s_InsideRenderMode = TRUE;

	if (s_InsideScene == FALSE)
	{
	    DXWARN(g_pD3DDevice->BeginScene());
		s_InsideScene = TRUE;

		if (s_bClearBuffers)
		{
			if (s_bZBuffer)
			{
				DXWARN(g_pD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, s_BackgroundColor.Get(), 1.0f, 0L));
			}
			else
			{
				DXWARN(g_pD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET, s_BackgroundColor.Get(), 0.0f, 0L));
			}
		}
	}
}

//=====================================================================================================================================
void ENG_EndRenderMode( void )
{
    ASSERT( s_InsideRenderMode );
    s_InsideRenderMode = FALSE;
}

//=====================================================================================================================================
xbool ENG_GetRenderMode( void )
{
    return( s_InsideRenderMode );
}

//=====================================================================================================================================
void ENG_FillScreenArea( s32 Area, color C )
{
	D3DRECT	rect;
	s32		x,y,w,h;

	ASSERT(g_pD3DDevice);

    switch( Area )
    {
    case ENG_AREA_SCREEN:
        {
            ENG_GetResolution( w,h );
			rect.x1 = 0;
			rect.y1 = 0;
			rect.x2 = w;
			rect.y2 = h;
        }
        break;
    case ENG_AREA_2D_CLIP:
        {
            ENG_Get2DClipArea( x, y, w, h );
			rect.x1 = x;
			rect.y1 = y;
			rect.x2 = w;
			rect.y2 = h;
        }
        break;
    case ENG_AREA_3D_VIEWPORT:
        {
            view*   pView;
            pView = ENG_GetActiveView();
            pView->GetViewport( x, y, w, h );
			rect.x1 = x;
			rect.y1 = y;
			rect.x2 = w;
			rect.y2 = h;
        }
        break;
    default:
        ASSERT( FALSE );
        break;
    }

	// perform the clear
    DXWARN(g_pD3DDevice->Clear( 1, &rect, D3DCLEAR_TARGET, C.Get(), 1.0f, 0L));
}

//=====================================================================================================================================
void ENG_FillZBufferArea( s32 Area, f32   Depth )
{
	D3DRECT	rect;
	s32		x,y,w,h;

	ASSERT(g_pD3DDevice);

    switch( Area )
    {
    case ENG_AREA_SCREEN:
        {
            ENG_GetResolution( w, h);
			rect.x1 = 0;
			rect.y1 = 0;
			rect.x2 = w;
			rect.y2 = h;
        }
        break;
    case ENG_AREA_2D_CLIP:
        {
            ENG_Get2DClipArea( x, y, w, h );
			rect.x1 = x;
			rect.y1 = y;
			rect.x2 = w;
			rect.y2 = h;
        }
        break;
    case ENG_AREA_3D_VIEWPORT:
        {
            view*   pView;
            pView = ENG_GetActiveView();
            pView->GetViewport( x, y, w, h );
			rect.x1 = x;
			rect.y1 = y;
			rect.x2 = w;
			rect.y2 = h;
        }
        break;
    default:
        ASSERT( FALSE );
        break;
    }

	// perform the clear
    DXWARN(g_pD3DDevice->Clear( 1, &rect, D3DCLEAR_ZBUFFER, 0x00000000u, 1.0f, 0L ));
}

//-------------------------------------------------------------------------------------------------------------------------------------
void ENG_EnableRGBAChannels( xbool R, xbool G, xbool B, xbool A )
{
    // Not supported, don't even know what the heck this is.
}

//-------------------------------------------------------------------------------------------------------------------------------------
void ENG_GetResolution( s32& Width, s32& Height )
{
    Width  = s_XRes;
    Height = s_YRes;
}

//=====================================================================================================================================
void ENG_WaitForGraphicsSynch( void )
{
    // Not supported.
}

//-------------------------------------------------------------------------------------------------------------------------------------
f32 ENG_GetFPS( void )
{
    return( 1.0f / s_FPSTimer.GetAvgTime() );
}

//-------------------------------------------------------------------------------------------------------------------------------------
f64 ENG_GetRunTime( void )
{
    return( x_GetTime() - s_StartRunTime );
}

//-------------------------------------------------------------------------------------------------------------------------------------
s32 ENG_GetPageFlipCount( void )
{
    return( s_Frame );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void ENG_SetBilinear( xbool OnOff )
{
    // NOT SUPPORTED
}

//-------------------------------------------------------------------------------------------------------------------------------------
xbool ENG_GetBilinear( void )
{
    // NOT SUPPORTED
    return FALSE;
}

//-------------------------------------------------------------------------------------------------------------------------------------
void ENG_PrintStats( void )
{
//    x_printfxy(0,2,"FRAME: %1d\n",s_Frame);
    x_printfxy(0,0,"FPS:   %1.1f\n",ENG_GetFPS());
    x_printfxy(0,1,"CPU:   %1.1f\n",1000.0f*s_CPUTimer.GetAvgTime());
    x_printfxy(0,2,"TOTAL: %1.1f\n",1000.0f*s_FPSTimer.GetAvgTime());
}


//====================================================================
// ConvertComandLine
//====================================================================
static void ConvertComandLine( int* pargc, char* argv[], LPSTR lpCmdLine )
{
	int argc = 1;

	if( *lpCmdLine )
	{
		argv[1] = lpCmdLine;
		argc = 2;

		do
		{
			if( *lpCmdLine == ' ' )
			{
				do
				{
					*lpCmdLine = 0;
					lpCmdLine++;
				} while( *lpCmdLine == ' ' );

				if( *lpCmdLine == 0 )
					break;
				argv[argc++] = lpCmdLine;
			}

			lpCmdLine++;

		} while( *lpCmdLine );
	}

	*pargc = argc;
}

//==========================================================================================================================================
// WinMain
//------------------------------------------------------------------------------
// This is the entry point when a win32 is created.
// (MFC Apps have their own entry point)
//==========================================================================================================================================
#ifndef TARGET_PC_NO_ENTRY

// dummy printf function, prints to output window
void DEBUG_PrintString(const char* pString)
{
	OutputDebugString(pString);
}

int WINAPI WinMain(	HINSTANCE   Instance,	    // handle to current instance
					HINSTANCE   PrevInstance,	// handle to previous instance
					LPSTR       CmdLine,	    // pointer to command line
					INT         ShowCmd) 	    // show state of window

{
	int     argc;
	char*   argv[20] = { NULL };
 	xbool	InitDone;
	s32		Val;

	//----------------------------------------------------------------
	//
	s_hWnd = 0;
	s_hInstance = Instance;

	//----------------------------------------------------------------
	// Convert windows command line to standard command line
	//----------------------------------------------------------------
	ConvertComandLine( &argc, argv, CmdLine );

	//----------------------------------------------------------------
    // Call the application init function to allow application specific changes
	InitDone = QuagInit();

	//---	if the initialization has not been completed, init x_files now
    if( !InitDone )
		x_Init();

    x_SetPrintHandlers( (print_fnptr)DEBUG_PrintString, (print_at_fnptr)TEXT_PrintStringAt );

    Val = QuagMain( argc, argv );
    x_Kill();
    return( Val );
}
#endif

//====================================================================
// Window_Handler
//====================================================================
static long CALLBACK Window_Handler( HWND hWnd, WORD Message, WORD wParam, LONG lParam )
{
	switch( Message )
	{
		case WM_DESTROY:
			PostQuitMessage( 0 );
			break;

		case WM_ACTIVATEAPP:
		{
			static xbool m_bActive = FALSE;
			m_bActive = wParam;
			break;
		}

        case WM_SYSCOMMAND:
            // Prevent moving/sizing and power loss in fullscreen mode
            switch( wParam )
            {
                case SC_MOVE:
                case SC_SIZE:
                case SC_MAXIMIZE:
                case SC_KEYMENU:
                case SC_MONITORPOWER:
                    if( s_bWindowed == FALSE)
                        return 1;
                    break;
            }
            break;
	}
	if (s_pfMessageHandler)
	{
		s_pfMessageHandler(hWnd, Message, wParam, lParam);
	}

	return DefWindowProc ( hWnd, Message, wParam, lParam);
}


//====================================================================
// MakeWindow
//====================================================================
static err MakeWindow(	HINSTANCE   hInstance,		// In:  Instance of the aplication
						DWORD		dwStyle,		// In:  Style of the window. (See windows help );
						DWORD		dwStyleEx,		// In:  Extended styles of the window (See windows help)
						HWND        hParent)		// In:  The Parent windows handle. NULL if not parents
{
	WNDCLASS	wClass;           // Window Class
	HWND		hWnd;             // Handle for the new window

	//-------------------------------------------------------------------
	// Register Class
	//-------------------------------------------------------------------
	wClass.lpszClassName	= s_WindowTitle;
	wClass.style			= CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wClass.lpfnWndProc		= (WNDPROC)Window_Handler;
	wClass.cbClsExtra		= 0;
	wClass.cbWndExtra		= 0;
	wClass.hInstance		= hInstance;
	wClass.hIcon			= NULL;//LoadIcon (Instance, MAKEINTRESOURCE (IDI_MAIN_ICON));
	wClass.hCursor			= LoadCursor ((HINSTANCE)NULL, IDC_ARROW);
	wClass.hbrBackground	= NULL;//(HBRUSH)GetStockObject (BLACK_BRUSH);
	wClass.lpszMenuName		= NULL;// MAKEINTRESOURCE (IDR_MAIN_MENU);

	if(! RegisterClass ( &wClass ))
	{
		ASSERTS(FALSE, "MakeWindow - RegisterClass failed." );
		return ERR_FAILURE;
	}

	//-------------------------------------------------------------------
	// Register Window
	//-------------------------------------------------------------------
	// Get the default Parent
	if ( hParent == NULL )
		hParent = GetDesktopWindow ();

	// adjust to fit size in client area
	{
		RECT	client;
		client.left = 0;
		client.top = 0;
		client.right = client.left + s_XRes;
		client.bottom = client.top + s_YRes;
		AdjustWindowRect(&client, dwStyle, FALSE);

		hWnd = CreateWindowEx(	dwStyleEx,			// extended window style
								s_WindowTitle,		// window class
								s_WindowTitle,		// window name (title)
								dwStyle,			// window style
								CW_USEDEFAULT,		// horizontal position of window
								CW_USEDEFAULT,		// vertical position of window
								client.right-client.left,		// window width
								client.bottom-client.top,		// window height
								hParent,			// handle to parent window
								NULL,				// handle to menu
								hInstance,			// handle to app instance
								NULL );				// Extra Data
	}

	if (! hWnd )
	{
		ASSERTS (FALSE, "Unable to open a win32 window");
		return ERR_FAILURE;
	}

	//-------------------------------------------------------------------
	// Set varaibles
	//-------------------------------------------------------------------
	s_hWnd = hWnd;

	return ERR_SUCCESS;
}


//=====================================================================================================================================
void ENGPC_TestAllRenderModesAndTextureStages( s32 MaxTextureStage )
{
    DWORD Value;
    s32 i;
    char DynamicString[100];

    OutputDebugString( "\n-----------------------------------");
    OutputDebugString( "\n----     D3D RenderStates     -----");
    OutputDebugString( "\n-----------------------------------\n");

    //-------------------------------------------------------------------------------------------------------------------------------------
    OutputDebugString( "\n------ ZBUFFER SETTINGS ------");
    //-------------------------------------------------------------------------------------------------------------------------------------
    // Test the cull mode.
    //-------------------------------------------------------------------------------------------------------------------------------------
    DXWARN(g_pD3DDevice->GetRenderState( D3DRS_CULLMODE, &Value ));
    switch( Value )
    {
        case D3DCULL_NONE:  OutputDebugString( "\nCullMode: None"); break;
        case D3DCULL_CW:    OutputDebugString( "\nCullMode: Clockwise"); break;
        case D3DCULL_CCW:   OutputDebugString( "\nCullMode: Counter Clockwise"); break;
        default:            OutputDebugString( "\nCullMode: Unknown"); break;
    }

    // Test if the zbuffer is on.
    //-------------------------------------------------------------------------------------------------------------------------------------
    DXWARN(g_pD3DDevice->GetRenderState( D3DRS_ZENABLE, &Value ));
    switch( Value )
    {
        case D3DZB_FALSE: OutputDebugString( "\nZBuffer:  OFF"); break;
        case D3DZB_TRUE:  OutputDebugString( "\nZBuffer:  ON"); break;
        default:          OutputDebugString( "\nZBuffer:  State is Unknown"); break;
    }

    // Test Z Buffer Write
    //-------------------------------------------------------------------------------------------------------------------------------------
    DXWARN(g_pD3DDevice->GetRenderState( D3DRS_ZWRITEENABLE, &Value ));
    switch( Value )
    {
        case TRUE:  OutputDebugString( "\nZBuffer Write: ON" ); break;
        case FALSE: OutputDebugString( "\nZBuffer Write: OFF"); break;
        default:    OutputDebugString( "\nZBuffer Write: State is Unknown"); break;
    }

    // Get the Z Buffer test function type.
    //-------------------------------------------------------------------------------------------------------------------------------------
    DXWARN(g_pD3DDevice->GetRenderState( D3DRS_ZFUNC, &Value ));
    switch( Value )
    {
        case D3DCMP_NEVER:          OutputDebugString( "\nZCompare: NEVER"); break;
        case D3DCMP_LESS:           OutputDebugString( "\nZCompare: LESS"); break;
        case D3DCMP_EQUAL:          OutputDebugString( "\nZCompare: EQUAL"); break;
        case D3DCMP_LESSEQUAL:      OutputDebugString( "\nZCompare: LESSEQUAL"); break;
        case D3DCMP_GREATER:        OutputDebugString( "\nZCompare: GREATER"); break;
        case D3DCMP_NOTEQUAL:       OutputDebugString( "\nZCompare: NOTEQUAL"); break;
        case D3DCMP_GREATEREQUAL:   OutputDebugString( "\nZCompare: GREATEREQUAL"); break;
        case D3DCMP_ALWAYS:         OutputDebugString( "\nZCompare: ALWAYS"); break;
        default:                    OutputDebugString( "\nZCompare: UNKNOWN"); break;
    }

    //-------------------------------------------------------------------------------------------------------------------------------------
    OutputDebugString( "\n\n------ ALPHA BLENDING SETTINGS ------");
    //-------------------------------------------------------------------------------------------------------------------------------------
    // Test if Alpha blending is on.
    //-------------------------------------------------------------------------------------------------------------------------------------
    DXWARN(g_pD3DDevice->GetRenderState( D3DRS_ALPHABLENDENABLE, &Value ));
    switch( Value )
    {
        case TRUE:  OutputDebugString( "\nAlpha Blending: ON"); break;
        case FALSE: OutputDebugString( "\nAlpha Blending: OFF"); break;
        default:    OutputDebugString( "\nAlpha Blending State is Unknown"); break;
    }

    // Test to see if the test alpha function is activated
    //-------------------------------------------------------------------------------------------------------------------------------------
    DXWARN(g_pD3DDevice->GetRenderState( D3DRS_ALPHATESTENABLE, &Value ));
    switch( Value )
    {
        case TRUE:
        {
            OutputDebugString( "\nAlpha Test: ON");

            // Determine what the alpha blending test is.
            DXWARN(g_pD3DDevice->GetRenderState( D3DRS_ALPHAFUNC, &Value ));
            switch( Value )
            {
                case D3DCMP_NEVER:          OutputDebugString( "\nAlpha Test Function is NEVER"); break;
                case D3DCMP_LESS:           OutputDebugString( "\nAlpha Test Function is LESS"); break;
                case D3DCMP_EQUAL:          OutputDebugString( "\nAlpha Test Function is EQUAL"); break;
                case D3DCMP_LESSEQUAL:      OutputDebugString( "\nAlpha Test Function is LESSEQUAL"); break;
                case D3DCMP_GREATER:        OutputDebugString( "\nAlpha Test Function is GREATER"); break;
                case D3DCMP_NOTEQUAL:       OutputDebugString( "\nAlpha Test Function is NOTEQUAL"); break;
                case D3DCMP_GREATEREQUAL:   OutputDebugString( "\nAlpha Test Function is GREATEREQUAL"); break;
                case D3DCMP_ALWAYS:         OutputDebugString( "\nAlpha Test Function is ALWAYS"); break;
                default:                    OutputDebugString( "\nAlpha Test Function is UNKNOWN"); break;
            }

            // Determine the alpha compare value.
            DXWARN(g_pD3DDevice->GetRenderState( D3DRS_ALPHAREF, &Value ));
            x_sprintf( DynamicString, "\nAlphaRef: %d", Value );
            OutputDebugString( DynamicString );
            break;
        }

        case FALSE: OutputDebugString( "\nAlpha Test: OFF"); break;
        default:    OutputDebugString( "\nAlpha Test: State is Unknown"); break;
    }

    // Get the last fixed alphs render state.
    //-------------------------------------------------------------------------------------------------------------------------------------
    DXWARN(g_pD3DDevice->GetRenderState( D3DRS_TEXTUREFACTOR , &Value ));
    x_sprintf( DynamicString, "\nFixedAlpha Color is is A:%x, R:0x%2x, G:%x, B:%x", (Value & 0xff000000) >> 24, (Value & 0x00ff0000) >> 16, (Value & 0x0000ff00) >> 8, Value & 0xff );
    OutputDebugString( DynamicString );


    //-------------------------------------------------------------------------------------------------------------------------------------
    OutputDebugString( "\n\n------ BLENDING MODE SETTINGS ------");
    //-------------------------------------------------------------------------------------------------------------------------------------
    // Test the Current Blend Operation
    //-------------------------------------------------------------------------------------------------------------------------------------
    DXWARN(g_pD3DDevice->GetRenderState( D3DRS_BLENDOP, &Value ));
    switch( Value )
    {
        case D3DBLENDOP_ADD:         OutputDebugString( "\nBlendOp: ADD"); break;
        case D3DBLENDOP_SUBTRACT:    OutputDebugString( "\nBlendOp: SUBTRACT"); break;
        case D3DBLENDOP_REVSUBTRACT: OutputDebugString( "\nBlendOp: REVSUBTRACT"); break;
        case D3DBLENDOP_MIN:         OutputDebugString( "\nBlendOp: MIN"); break;
        case D3DBLENDOP_MAX:         OutputDebugString( "\nBlendOp: MAX"); break;
        default:                     OutputDebugString( "\nBlendOp: UNKNOWN"); break;
    }

    // Get the source and destination blend settings.
    //-------------------------------------------------------------------------------------------------------------------------------------
    DXWARN(g_pD3DDevice->GetRenderState( D3DRS_SRCBLEND, &Value ));
    switch( Value )
    {
        case D3DBLEND_ZERO:             OutputDebugString( "\nSourceBlend: ZERO"); break;
        case D3DBLEND_ONE:              OutputDebugString( "\nSourceBlend: ONE"); break;
        case D3DBLEND_SRCCOLOR:         OutputDebugString( "\nSourceBlend: SRCCOLOR"); break;
        case D3DBLEND_INVSRCCOLOR:      OutputDebugString( "\nSourceBlend: INVSRCCOLOR"); break;
        case D3DBLEND_SRCALPHA:         OutputDebugString( "\nSourceBlend: SRCALPHA"); break;
        case D3DBLEND_INVSRCALPHA:      OutputDebugString( "\nSourceBlend: INVSRCALPHA"); break;
        case D3DBLEND_DESTALPHA:        OutputDebugString( "\nSourceBlend: DESTALPHA"); break;
        case D3DBLEND_INVDESTALPHA:     OutputDebugString( "\nSourceBlend: INVDESTALPHA"); break;
        case D3DBLEND_DESTCOLOR:        OutputDebugString( "\nSourceBlend: DESTCOLOR"); break;
        case D3DBLEND_INVDESTCOLOR:     OutputDebugString( "\nSourceBlend: INVDESTCOLOR"); break;
        case D3DBLEND_SRCALPHASAT:      OutputDebugString( "\nSourceBlend: SRCALPHASAT"); break;
        case D3DBLEND_BOTHSRCALPHA:     OutputDebugString( "\nSourceBlend: BOTHSRCALPHA"); break;
        case D3DBLEND_BOTHINVSRCALPHA:  OutputDebugString( "\nSourceBlend: BOTHINVSRCALPHA"); break;
        default:                        OutputDebugString( "\nSourceBlend: UNKNOWN"); break;
    }

    DXWARN(g_pD3DDevice->GetRenderState( D3DRS_DESTBLEND, &Value ));
    switch( Value )
    {
        case D3DBLEND_ZERO:             OutputDebugString( "\nDestBlend: ZERO"); break;
        case D3DBLEND_ONE:              OutputDebugString( "\nDestBlend: ONE"); break;
        case D3DBLEND_SRCCOLOR:         OutputDebugString( "\nDestBlend: SRCCOLOR"); break;
        case D3DBLEND_INVSRCCOLOR:      OutputDebugString( "\nDestBlend: INVSRCCOLOR"); break;
        case D3DBLEND_SRCALPHA:         OutputDebugString( "\nDestBlend: SRCALPHA"); break;
        case D3DBLEND_INVSRCALPHA:      OutputDebugString( "\nDestBlend: INVSRCALPHA"); break;
        case D3DBLEND_DESTALPHA:        OutputDebugString( "\nDestBlend: DESTALPHA"); break;
        case D3DBLEND_INVDESTALPHA:     OutputDebugString( "\nDestBlend: INVDESTALPHA"); break;
        case D3DBLEND_DESTCOLOR:        OutputDebugString( "\nDestBlend: DESTCOLOR"); break;
        case D3DBLEND_INVDESTCOLOR:     OutputDebugString( "\nDestBlend: INVDESTCOLOR"); break;
        case D3DBLEND_SRCALPHASAT:      OutputDebugString( "\nDestBlend: SRCALPHASAT"); break;
        case D3DBLEND_BOTHSRCALPHA:     OutputDebugString( "\nDestBlend: BOTHSRCALPHA"); break;
        case D3DBLEND_BOTHINVSRCALPHA:  OutputDebugString( "\nDestBlend: BOTHINVSRCALPHA"); break;
        default:                        OutputDebugString( "\nDestBlend: UNKNOWN"); break;
    }

    // Test the settings for each texture stage up to the max texture stage set.
    //-------------------------------------------------------------------------------------------------------------------------------------
    OutputDebugString( "\n\n------ TEXTURE STAGE SETTINGS ------");
    //-------------------------------------------------------------------------------------------------------------------------------------
    for( i = 0; i < MaxTextureStage; i++ )
    {
        x_sprintf( DynamicString, "\n----- Texture Stage %d -----", i );
        OutputDebugString( DynamicString );

        // Get the color operator.
        DXWARN(g_pD3DDevice->GetTextureStageState( i, D3DTSS_COLOROP, &Value ));
        switch( Value )
        {   
            case D3DTOP_DISABLE:                    OutputDebugString( "\nColorOP: DISABLE"); break;
            case D3DTOP_SELECTARG1:                 OutputDebugString( "\nColorOP: SELECTARG1"); break;
            case D3DTOP_SELECTARG2:                 OutputDebugString( "\nColorOP: SELECTARG2"); break;
            case D3DTOP_MODULATE:                   OutputDebugString( "\nColorOP: MODULATE"); break;
            case D3DTOP_MODULATE2X:                 OutputDebugString( "\nColorOP: MODULATE2X"); break;
            case D3DTOP_MODULATE4X:                 OutputDebugString( "\nColorOP: MODULATE4X"); break;
            case D3DTOP_ADD:                        OutputDebugString( "\nColorOP: ADD"); break;
            case D3DTOP_ADDSIGNED:                  OutputDebugString( "\nColorOP: ADDSIGNED"); break;
            case D3DTOP_ADDSIGNED2X:                OutputDebugString( "\nColorOP: ADDSIGNED2X"); break;
            case D3DTOP_SUBTRACT:                   OutputDebugString( "\nColorOP: SUBTRACT"); break;
            case D3DTOP_ADDSMOOTH:                  OutputDebugString( "\nColorOP: ADDSMOOTH"); break;
            case D3DTOP_BLENDDIFFUSEALPHA:          OutputDebugString( "\nColorOP: BLENDDIFFUSEALPHA"); break;
            case D3DTOP_BLENDTEXTUREALPHA:          OutputDebugString( "\nColorOP: BLENDTEXTUREALPHA"); break;
            case D3DTOP_BLENDFACTORALPHA:           OutputDebugString( "\nColorOP: BLENDFACTORALPHA"); break;
            case D3DTOP_BLENDTEXTUREALPHAPM:        OutputDebugString( "\nColorOP: BLENDTEXTUREALPHAPM"); break;
            case D3DTOP_BLENDCURRENTALPHA:          OutputDebugString( "\nColorOP: BLENDCURRENTALPHA"); break;
            case D3DTOP_PREMODULATE:                OutputDebugString( "\nColorOP: PREMODULATE"); break;
            case D3DTOP_MODULATEALPHA_ADDCOLOR:     OutputDebugString( "\nColorOP: MODULATEALPHA_ADDCOLOR"); break;
            case D3DTOP_MODULATECOLOR_ADDALPHA:     OutputDebugString( "\nColorOP: MODULATECOLOR_ADDALPHA"); break;
            case D3DTOP_MODULATEINVALPHA_ADDCOLOR:  OutputDebugString( "\nColorOP: MODULATEINVALPHA_ADDCOLOR"); break;
            case D3DTOP_MODULATEINVCOLOR_ADDALPHA:  OutputDebugString( "\nColorOP: MODULATEINVCOLOR_ADDALPHA"); break;
            case D3DTOP_BUMPENVMAP:                 OutputDebugString( "\nColorOP: BUMPENVMAP"); break;
            case D3DTOP_BUMPENVMAPLUMINANCE:        OutputDebugString( "\nColorOP: BUMPENVMAPLUMINANCE"); break;
            case D3DTOP_DOTPRODUCT3:                OutputDebugString( "\nColorOP: DOTPRODUCT3"); break;
            case D3DTOP_MULTIPLYADD:                OutputDebugString( "\nColorOP: MULTIPLYADD"); break;
            case D3DTOP_LERP:                       OutputDebugString( "\nColorOP: LERP"); break;
            default:                                OutputDebugString( "\nColorOP: UNKNOWN"); break;
        }


        // Get Color Arg1.
        DXWARN(g_pD3DDevice->GetTextureStageState( i, D3DTSS_COLORARG1, &Value ));
        switch( Value )
        {   
            case D3DTA_CURRENT:         OutputDebugString( "\nColorARG1: CURRENT"); break;
            case D3DTA_DIFFUSE:         OutputDebugString( "\nColorARG1: DIFFUSE"); break;
            case D3DTA_SELECTMASK:      OutputDebugString( "\nColorARG1: SELECTMASK"); break;
            case D3DTA_SPECULAR:        OutputDebugString( "\nColorARG1: SPECULAR"); break;
            case D3DTA_TEMP:            OutputDebugString( "\nColorARG1: TEMP"); break;
            case D3DTA_TEXTURE:         OutputDebugString( "\nColorARG1: TEXTURE"); break;
            case D3DTA_TFACTOR:         OutputDebugString( "\nColorARG1: TFACTOR"); break;
            case D3DTA_ALPHAREPLICATE:  OutputDebugString( "\nColorARG1: ALPHAREPLICATE"); break;
            case D3DTA_COMPLEMENT:      OutputDebugString( "\nColorARG1: COMPLEMENT"); break;
            default:                    OutputDebugString( "\nColorARG1: UNKNOWN"); break;
        }
        
        // Get Color Arg2.
        DXWARN(g_pD3DDevice->GetTextureStageState( i, D3DTSS_COLORARG2, &Value ));
        switch( Value )
        {   
            case D3DTA_CURRENT:         OutputDebugString( "\nColorARG2: CURRENT"); break;
            case D3DTA_DIFFUSE:         OutputDebugString( "\nColorARG2: DIFFUSE"); break;
            case D3DTA_SELECTMASK:      OutputDebugString( "\nColorARG2: SELECTMASK"); break;
            case D3DTA_SPECULAR:        OutputDebugString( "\nColorARG2: SPECULAR"); break;
            case D3DTA_TEMP:            OutputDebugString( "\nColorARG2: TEMP"); break;
            case D3DTA_TEXTURE:         OutputDebugString( "\nColorARG2: TEXTURE"); break;
            case D3DTA_TFACTOR:         OutputDebugString( "\nColorARG2: TFACTOR"); break;
            case D3DTA_ALPHAREPLICATE:  OutputDebugString( "\nColorARG2: ALPHAREPLICATE"); break;
            case D3DTA_COMPLEMENT:      OutputDebugString( "\nColorARG2: COMPLEMENT"); break;
            default:                    OutputDebugString( "\nColorARG2: UNKNOWN"); break;
        }


        // Get the ALPHA operator.
        DXWARN(g_pD3DDevice->GetTextureStageState( i, D3DTSS_ALPHAOP, &Value ));
        switch( Value )
        {   
            case D3DTOP_DISABLE:                    OutputDebugString( "\nAlphaOP: DISABLE"); break;
            case D3DTOP_SELECTARG1:                 OutputDebugString( "\nAlphaOP: SELECTARG1"); break;
            case D3DTOP_SELECTARG2:                 OutputDebugString( "\nAlphaOP: SELECTARG2"); break;
            case D3DTOP_MODULATE:                   OutputDebugString( "\nAlphaOP: MODULATE"); break;
            case D3DTOP_MODULATE2X:                 OutputDebugString( "\nAlphaOP: MODULATE2X"); break;
            case D3DTOP_MODULATE4X:                 OutputDebugString( "\nAlphaOP: MODULATE4X"); break;
            case D3DTOP_ADD:                        OutputDebugString( "\nAlphaOP: ADD"); break;
            case D3DTOP_ADDSIGNED:                  OutputDebugString( "\nAlphaOP: ADDSIGNED"); break;
            case D3DTOP_ADDSIGNED2X:                OutputDebugString( "\nAlphaOP: ADDSIGNED2X"); break;
            case D3DTOP_SUBTRACT:                   OutputDebugString( "\nAlphaOP: SUBTRACT"); break;
            case D3DTOP_ADDSMOOTH:                  OutputDebugString( "\nAlphaOP: ADDSMOOTH"); break;
            case D3DTOP_BLENDDIFFUSEALPHA:          OutputDebugString( "\nAlphaOP: BLENDDIFFUSEALPHA"); break;
            case D3DTOP_BLENDTEXTUREALPHA:          OutputDebugString( "\nAlphaOP: BLENDTEXTUREALPHA"); break;
            case D3DTOP_BLENDFACTORALPHA:           OutputDebugString( "\nAlphaOP: BLENDFACTORALPHA"); break;
            case D3DTOP_BLENDTEXTUREALPHAPM:        OutputDebugString( "\nAlphaOP: BLENDTEXTUREALPHAPM"); break;
            case D3DTOP_BLENDCURRENTALPHA:          OutputDebugString( "\nAlphaOP: BLENDCURRENTALPHA"); break;
            case D3DTOP_PREMODULATE:                OutputDebugString( "\nAlphaOP: PREMODULATE"); break;
            case D3DTOP_MODULATEALPHA_ADDCOLOR:     OutputDebugString( "\nAlphaOP: MODULATEALPHA_ADDCOLOR"); break;
            case D3DTOP_MODULATECOLOR_ADDALPHA:     OutputDebugString( "\nAlphaOP: MODULATECOLOR_ADDALPHA"); break;
            case D3DTOP_MODULATEINVALPHA_ADDCOLOR:  OutputDebugString( "\nAlphaOP: MODULATEINVALPHA_ADDCOLOR"); break;
            case D3DTOP_MODULATEINVCOLOR_ADDALPHA:  OutputDebugString( "\nAlphaOP: MODULATEINVCOLOR_ADDALPHA"); break;
            case D3DTOP_BUMPENVMAP:                 OutputDebugString( "\nAlphaOP: BUMPENVMAP"); break;
            case D3DTOP_BUMPENVMAPLUMINANCE:        OutputDebugString( "\nAlphaOP: BUMPENVMAPLUMINANCE"); break;
            case D3DTOP_DOTPRODUCT3:                OutputDebugString( "\nAlphaOP: DOTPRODUCT3"); break;
            case D3DTOP_MULTIPLYADD:                OutputDebugString( "\nAlphaOP: MULTIPLYADD"); break;
            case D3DTOP_LERP:                       OutputDebugString( "\nAlphaOP: LERP"); break;
            default:                                OutputDebugString( "\nAlphaOP: UNKNOWN"); break;
        }


        // Get Alpha Arg1.
        DXWARN(g_pD3DDevice->GetTextureStageState( i, D3DTSS_ALPHAARG1, &Value ));
        switch( Value )
        {   
            case D3DTA_CURRENT:         OutputDebugString( "\nAlphaARG1: CURRENT"); break;
            case D3DTA_DIFFUSE:         OutputDebugString( "\nAlphaARG1: DIFFUSE"); break;
            case D3DTA_SELECTMASK:      OutputDebugString( "\nAlphaARG1: SELECTMASK"); break;
            case D3DTA_SPECULAR:        OutputDebugString( "\nAlphaARG1: SPECULAR"); break;
            case D3DTA_TEMP:            OutputDebugString( "\nAlphaARG1: TEMP"); break;
            case D3DTA_TEXTURE:         OutputDebugString( "\nAlphaARG1: TEXTURE"); break;
            case D3DTA_TFACTOR:         OutputDebugString( "\nAlphaARG1: TFACTOR"); break;
            case D3DTA_ALPHAREPLICATE:  OutputDebugString( "\nAlphaARG1: ALPHAREPLICATE"); break;
            case D3DTA_COMPLEMENT:      OutputDebugString( "\nAlphaARG1: COMPLEMENT"); break;
            default:                    OutputDebugString( "\nAlphaARG1: UNKNOWN"); break;
        }
        
        // Get Alpha Arg2.
        DXWARN(g_pD3DDevice->GetTextureStageState( i, D3DTSS_ALPHAARG2, &Value ));
        switch( Value )
        {   
            case D3DTA_CURRENT:         OutputDebugString( "\nAlphaARG2: CURRENT"); break;
            case D3DTA_DIFFUSE:         OutputDebugString( "\nAlphaARG2: DIFFUSE"); break;
            case D3DTA_SELECTMASK:      OutputDebugString( "\nAlphaARG2: SELECTMASK"); break;
            case D3DTA_SPECULAR:        OutputDebugString( "\nAlphaARG2: SPECULAR"); break;
            case D3DTA_TEMP:            OutputDebugString( "\nAlphaARG2: TEMP"); break;
            case D3DTA_TEXTURE:         OutputDebugString( "\nAlphaARG2: TEXTURE"); break;
            case D3DTA_TFACTOR:         OutputDebugString( "\nAlphaARG2: TFACTOR"); break;
            case D3DTA_ALPHAREPLICATE:  OutputDebugString( "\nAlphaARG2: ALPHAREPLICATE"); break;
            case D3DTA_COMPLEMENT:      OutputDebugString( "\nAlphaARG2: COMPLEMENT"); break;
            default:                    OutputDebugString( "\nAlphaARG2: UNKNOWN"); break;
        }

        // Wrapping mode U then V.
        DXWARN(g_pD3DDevice->GetTextureStageState( i, D3DTSS_ADDRESSU, &Value ));
        switch( Value )
        {   
            case D3DTADDRESS_WRAP:       OutputDebugString( "\nWrapU Mode: WRAP"); break;
            case D3DTADDRESS_MIRROR:     OutputDebugString( "\nWrapU Mode: MIRROR"); break;
            case D3DTADDRESS_CLAMP:      OutputDebugString( "\nWrapU Mode: CLAMP"); break;
            case D3DTADDRESS_BORDER:     OutputDebugString( "\nWrapU Mode: BORDER"); break;
            case D3DTADDRESS_MIRRORONCE: OutputDebugString( "\nWrapU Mode: MIRRORONCE"); break;
            default:                     OutputDebugString( "\nWrapU Mode: UNKNOWN"); break;
        }

        DXWARN(g_pD3DDevice->GetTextureStageState( i, D3DTSS_ADDRESSV, &Value ));
        switch( Value )
        {   
            case D3DTADDRESS_WRAP:       OutputDebugString( "\nWrapV Mode: WRAP"); break;
            case D3DTADDRESS_MIRROR:     OutputDebugString( "\nWrapV Mode: MIRROR"); break;
            case D3DTADDRESS_CLAMP:      OutputDebugString( "\nWrapV Mode: CLAMP"); break;
            case D3DTADDRESS_BORDER:     OutputDebugString( "\nWrapV Mode: BORDER"); break;
            case D3DTADDRESS_MIRRORONCE: OutputDebugString( "\nWrapV Mode: MIRRORONCE"); break;
            default:                     OutputDebugString( "\nWrapV Mode: UNKNOWN"); break;
        }
    }    


    OutputDebugString( "\n-----------------------------------");
    OutputDebugString( "\n---  END of D3D RenderStates  -----");
    OutputDebugString( "\n-----------------------------------\n");
}

LPDIRECT3DDEVICE8 PC_GetDevice()
{
    return g_pD3DDevice;
}

void ENG_ForceNonBlockingVSync( void )
{
}
