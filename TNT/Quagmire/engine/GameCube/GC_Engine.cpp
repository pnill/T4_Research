////////////////////////////////////////////////////////////////////////////
//
//  GC_Engine.CPP
//
//
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_files.hpp"

#include "Q_Engine.hpp"
#include "Q_GC.hpp"
#include "Q_VRAM.hpp"
#include "Q_SMem.hpp"
#include "Q_View.hpp"
#include "Q_Draw.hpp"

#include "E_SMem.hpp"
#include "E_AccumTimer.hpp"
#include "E_Text.hpp"
#include "Q_FileReadQueue.h"

#include "GC_Input.hpp"
#include "GC_Font.hpp"
#include "GC_Video.hpp"
#include "GC_DVD.hpp"
#include "GC_Draw.hpp"
#include "GC_MemoryUnit.hpp"
#include "GC_PERF.hpp"

////////////////////////////////////////////////////////////////////////////
// DEBUGGING SWITCHES
////////////////////////////////////////////////////////////////////////////

#if defined( X_DEBUG )
	//#define ENABLE_SN_PROFILING
#endif


////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////

//--- Lighting Mode Flags
#define ENG_LIGHTING_ON     0x0100
#define ENG_LIGHTING_OFF    0x0200


////////////////////////////////////////////////////////////////////////////
// GLOBAL-STATIC VARIABLES
////////////////////////////////////////////////////////////////////////////

static xbool        s_EngActivated = FALSE;
static s32          s_XRes;
static s32          s_YRes;
static xbool        s_bZBuffer;
static view         s_View;

static s32          s_2DClipL;
static s32          s_2DClipT;
static s32          s_2DClipR;
static s32          s_2DClipB;

static s32          s_ViewportL;
static s32          s_ViewportT;
static s32          s_ViewportW;
static s32          s_ViewportH;

static xbool        s_InsideRenderMode;

static s64          s_StartRunTime;
static accum_timer  s_FPSTimer;
static accum_timer  s_CPUTimer;
static u32          s_Frame; 

static s32          s_SMemSize  = -1;

static color        s_BackgroundColor(0,0,0);

static matrix4      s_MWorldToScreen;
//static matrix4      s_MWorldToView;
static matrix4      s_MWorldToClip;
static matrix4      s_MClipToScreen;

static lighting     s_ActiveLighting;
static GXLightObj   s_GCLights[MAX_DIR_LIGHTS];

static u32          s_GCActiveLights = GX_LIGHT_NULL;
static u32          s_LightingFlags;

static xbool        s_ZCompUseLEQUAL = TRUE;
static u32          s_RenderFlags;
static u32          s_BlendFlags;
static u32          s_FixedAlpha;

static xbool        s_RefPlaneCalculated = FALSE;
static xbool        s_RefPlaneEnabled    = FALSE;
static vector4      s_RefPlane;
static vector3      s_RefPlaneVerts[3];

static xbool          s_ResetPressed             = FALSE;
static volatile xbool s_ResetPressedFromCallback = FALSE;

////////////////////////////////////////////////////////////////////////////
// Global non-statics
////////////////////////////////////////////////////////////////////////////

color eng_fog_color(0,0,0,0);
eng_fog g_EngineFog={0,0,0,0};



////////////////////////////////////////////////////////////////////////////
// PROTOTYPES
////////////////////////////////////////////////////////////////////////////

//--- special client-side init function, returns TRUE if x_files
//    was initialized(i.e. called x_Init), FALSE if not
xbool QuagInit( void );

//--- client entry point
s32 QuagMain( s32 argc, char* argv[] );


#if defined( ENABLE_GC_PERF_LIB )
    static void GCPERF_Update     ( void );
    static void GCPERF_StartSample( void );
    static void GCPERF_Init       ( void );
#else
    #define GCPERF_Update()
    #define GCPERF_StartSample()
    #define GCPERF_Init()
#endif


////////////////////////////////////////////////////////////////////////////
// SN PROFILER SETUP
////////////////////////////////////////////////////////////////////////////

#if defined( ENABLE_SN_PROFILING ) //&& defined( X_DEBUG )

	#include "libsn.h"

	#define SN_PROFILE_BUFFER_SIZE	(64 * 1024)

	static u32 s_SNProfileBuffer[SN_PROFILE_BUFFER_SIZE/4];

	static void GC_InitSNProfiler( void )
	{
		snProfInit( _20KHZ, s_SNProfileBuffer, sizeof(s_SNProfileBuffer) );
	}

#else

	#define GC_InitSNProfiler()

#endif //defined( ENABLE_SN_PROFILING )


////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

xbool ENG_Activate( s32   XRes,    
                    s32   YRes, 
                    xbool ZBuffer,
                    s32   SMemSize,
                    char* ApplicationName )
{

    ASSERT( !s_EngActivated );
    ASSERT( SMemSize );

    //--- Confirm resolution
    ASSERT( XRes == 640 );
    ASSERT( YRes == 480 );

    s_XRes = XRes;
    s_YRes = YRes;

    s_View.SetViewport( 0, 0, XRes, YRes );

    s_bZBuffer = ZBuffer;

    s_Frame = 0;

    //--- Clear timers
    s_FPSTimer.Clear();
    s_CPUTimer.Clear();

    x_printf( "---------------------------------------\n" );

    //--- Setup render environment  
    x_printf( "ENGINE: Initializing render environment.\n" );
    VERIFY( VIDEO_InitModule() );

    //--- Initialize debug module
    x_printf( "ENGINE: Initializing debug module.\n" );
    extern void DEBUG_InitModule( void );
    DEBUG_InitModule();

    //--- setup the draw layer
    x_printf( "ENGINE: Initializing draw.\n" );
    VERIFY( DRAW_InitModule() );

    //--- Initialize scratch memory
    x_printf( "ENGINE: Initializing scratch memory.\n" );
    s_SMemSize = SMemSize;
    SMEM_InitModule( s_SMemSize );

    //--- Initialize VRAM
    x_printf( "ENGINE: Initializing vram manager.\n" );
    VRAM_Init();

    //--- Init text
    x_printf( "ENGINE: Initializing font.\n" );
    VERIFY( FONT_InitModule() );

    x_printf( "ENGINE: Initializing text manager.\n" );
    TEXT_InitModule( s_XRes, s_YRes, 12, 16, 16, 16, FONT_RenderText );

    //--- Initialize input
    x_printf( "ENGINE: Initializing controllers.\n" );
    VERIFY( INPUT_InitModule() );

    x_printf( "ENGINE: Initializing default lighting.\n" );
    {
        lighting L;
        L.Locked = FALSE;
        L.Ambient = color(32,32,32,255);
        L.Directional[0].Active     = TRUE;
        L.Directional[0].Color      = color(255,50,50,255);
        L.Directional[0].Direction  = vector3(0,-1,0);
        L.Directional[1].Active     = TRUE;
        L.Directional[1].Color      = color(50,255,50,255);
        L.Directional[1].Direction  = vector3(1,0,0);
        L.Directional[2].Active     = TRUE;
        L.Directional[2].Color      = color(50,50,255,255);
        L.Directional[2].Direction  = vector3(0,0,-1);

        L.Directional[0].Direction.Normalize();
        L.Directional[1].Direction.Normalize();
        L.Directional[2].Direction.Normalize();

        s_LightingFlags = 0;
        ENG_SetActiveLighting( L );
    }

    //--- Initialize the GameCube memory card support.
    x_printf( "ENGINE: Initializing Memory Card Support.\n" );
    MEMORY_UNIT_Create( ApplicationName );

    //--- Mark engine as activated
    x_printf( "ENGINE: Initialized.\n" );
    x_printf( "---------------------------------------\n" );
    s_EngActivated = TRUE;

    //--- Store engine startup time
    s_StartRunTime = (s64)OSGetTime();

    //--- Setup 2D clip area
    ENG_Set2DClipArea( 0, 0, XRes-1, YRes-1 );

    GC_SetViewport( 0, 0, XRes, YRes );

    s_ZCompUseLEQUAL = TRUE;
    s_RenderFlags    = 0;
    s_BlendFlags     = 0;
    s_FixedAlpha     = 0x00;

    s_InsideRenderMode = FALSE;

    s_RefPlaneCalculated = FALSE;
    s_RefPlaneEnabled    = FALSE;
    s_RefPlane.Set( 0, 1, 0, 0 );

    ENG_BeginRenderMode();
    ENG_SetRenderFlags( ENG_ZBUFFER_TEST_ON | ENG_ZBUFFER_FILL_ON | ENG_ALPHA_TEST_OFF | ENG_ALPHA_BLEND_OFF );
    ENG_SetBlendMode( ENG_BLEND_NORMAL, 0x00 );
    ENG_EndRenderMode();

    GCPERF_Init();

    return( ERR_SUCCESS );
}

//==========================================================================

void ENG_Deactivate( void )
{
    ASSERT( s_EngActivated );

    x_printf( "---------------------------------------\n" );
    x_printf( "ENGINE: Closing...\n" );

    x_printf( "[MemoryCard]\n");
    MEMORY_UNIT_Destroy( );

    x_printf( "[input]\n" );
        INPUT_KillModule();

    x_printf( "[font]\n" );
        FONT_KillModule();

    x_printf( "[vram]\n" );
        VRAM_Kill();

    x_printf( "[smem]\n" );
        SMEM_KillModule();

    x_printf( "[draw]\n" );
        DRAW_KillModule();

    x_printf( "[video]\n" );
        VIDEO_KillModule();

    s_EngActivated = FALSE;    

    x_printf( "ENGINE: Closed.\n" );
}

//==========================================================================
/*
static void GC_ResetCallback( void )
{
    if( OSGetResetSwitchState() )
        s_ResetPressedFromCallback = TRUE;
}
*/
//==========================================================================

void GC_CheckForReset( void )
{
    xbool NoMemCardWriteInProgress = !gpGCMemoryUnitManager->IsWriteInProgress( );

    xbool ResetPressedNow = OSGetResetSwitchState();

    if( (s_ResetPressed || s_ResetPressedFromCallback) && !ResetPressedNow && NoMemCardWriteInProgress ) 
    {
		//x_printf( "MACHINE RESET FROM:\n%s\n", pCallstackStr );

        //--- shut down GameCube systems
        AXQuit();
		GXAbortFrame();

        //--- shut down engine and x_files first
        ENG_Deactivate();
        //DVD_KillModule();
        //x_Kill();

        //GXFlush();
        //GXDrawDone();
        VISetBlack( TRUE );
        VIFlush();
        VIWaitForRetrace();

        //--- reset the machine
        OSResetSystem( OS_RESET_RESTART, 0, FALSE );
    }

	// If the button has been pressed, arm the reset test so it will fire when the memory card write
	// has completed.
	if( ResetPressedNow )
		s_ResetPressed = ResetPressedNow;
}

//==========================================================================

void ENG_PageFlip( xbool ClearNewScreen )
{
    //--- End timing of CPU
    s_CPUTimer.EndTiming();

    GC_CheckForReset();

    //--- Render the text
    ENG_BeginRenderMode();
    FONT_BeginRender();
    TEXT_RenderAllText();
    FONT_EndRender();
    ENG_EndRenderMode();

    //--- Change Draw/Disp buffers, and end the scene
    GXColor BKColor = { s_BackgroundColor.R, s_BackgroundColor.G, s_BackgroundColor.B, s_BackgroundColor.A };
    GXSetCopyClear( BKColor, GX_MAX_Z24 );

    GCPERF_Update();

    GC_CheckForReset();

    VIDEO_PageFlip( ClearNewScreen );

    GC_CheckForReset();

    GCPERF_StartSample();

    //--- Update FPS timer
    s_FPSTimer.EndTiming();
    s_FPSTimer.StartTiming();

    //--- Increment to next frame
    s_Frame++;

    //--- Toggle scratch buffers and text buffers
    SMEM_Toggle();
    TEXT_SwitchBuffers();

    //--- Check game pads for input
    INPUT_CheckDevices();

    // Pulse the memcard system
    MEMORY_UNIT_Update();

    // Update any File Reads from the FileReadQueue/
    ENG_UpdateFileReadQueue( );

    //--- Begin timing of user CPU again
    s_CPUTimer.StartTiming();
}

//==========================================================================

void ENG_GetW2S( matrix4& M )
{
    M = s_MWorldToScreen;
}

//==========================================================================

void ENG_GetW2C( matrix4& M )
{
    M = s_MWorldToClip;
}

//==========================================================================

void ENG_GetC2S( matrix4& M )
{
    M = s_MClipToScreen;
}

//==========================================================================

void ENG_SetActiveView( const view& View )
{
    matrix4 MW2V;
    matrix4 mProj;
    radian  XFOV, YFOV;
    s32     L, T, R, B;
    f32     AspectRatio;
    f32     ZN, ZF;
    f32     cot;

    //--- Copy view into active view
    x_memcpy( &s_View, &View, sizeof(view) );

    //--- Get viewport bounds and other info
    s_View.GetViewport( L, T, R, B );
    s_View.GetZLimits ( ZN, ZF );
    s_View.GetXFOV    ( XFOV );

    //--- Get half height YFOV
    s_View.GetAspectRatio( AspectRatio );
    YFOV = 2.0f * x_atan( (B-T+1) * x_tan(XFOV*0.5f) / (R-L+1) );

    //--- setup device scissor region and viewport
    ENG_Set2DClipArea( L, T, R, B );
    GC_SetViewport( L, T, (R-L), (B-T) );

    //--- setup the projection matrix
    //MTXPerspective( mProj.M, RAD_TO_DEG(YFOV), AspectRatio, ZN, ZF );
    {   mProj.Identity();

        cot = 1.0f / x_tan( YFOV * 0.5f );

        mProj.M[0][0] =  cot * (B-T+1) / (R-L+1);
        mProj.M[1][1] =  cot;

        //--- scale z to (-w, 0) range for Dolphin
        mProj.M[2][2] = -(ZN) / (ZF-ZN);
        mProj.M[2][3] = -(ZF*ZN) / (ZF-ZN);
        mProj.M[3][2] = -1.0f;
        mProj.M[3][3] = 0.0f;
    }
    GXSetProjection( mProj.M, GX_PERSPECTIVE );

    GC_UpdateLights();
}

//==========================================================================

view* ENG_GetActiveView( void )
{
    return( &s_View );
}

//==========================================================================

void GC_SetViewport( s32 L, s32 T, s32 W, s32 H )
{
/*
    if( L < 0 ) L = 0;
    if( T < 0 ) T = 0;

    if( L >= s_XRes ) L = s_XRes - 1;
    if( T >= s_YRes ) T = s_YRes - 1;

    if( (L + W) > s_XRes ) W = s_XRes - L;
    if( (T + H) > s_YRes ) H = s_YRes - T;
*/
    if( W <= 0 ) W = 1;
    if( H <= 0 ) H = 1;


    if( VIDEO_IsFieldMode() )
    {
        GXSetViewportJitter( L, T, W, H, 0.0f, 1.0f, VIGetNextField() );
    }
    else
    {
        GXSetViewport( L, T, W, H, 0.0f, 1.0f );
    }

    s_ViewportL = L;
    s_ViewportT = T;
    s_ViewportW = W;
    s_ViewportH = H;
}

//==========================================================================

void GC_GetViewport( s32& L, s32& T, s32& W, s32& H )
{
    L = s_ViewportL;
    T = s_ViewportT;
    W = s_ViewportW;
    H = s_ViewportH;
}

//==========================================================================

void ENG_Set2DClipArea( s32 L, s32 T, s32 R, s32 B )
{
    if( L < 0 ) L = 0;
    if( T < 0 ) T = 0;

    if( L > s_XRes ) L = s_XRes;
    if( T > s_YRes ) T = s_YRes;

    if( R < L ) R = L;
    if( B < T ) B = T;

    if( R > s_XRes ) R = s_XRes;
    if( B > s_YRes ) B = s_YRes;


    if( (s_2DClipL != L) ||
        (s_2DClipT != T) ||
        (s_2DClipR != R) ||
        (s_2DClipB != B) )
    {
        s_2DClipL = L;
        s_2DClipT = T;
        s_2DClipR = R;
        s_2DClipB = B;

        GXSetScissor( L, T, (R-L), (B-T) );
    }
}

//==========================================================================

void ENG_Get2DClipArea( s32& L, s32& T, s32& R, s32& B )
{
    L = s_2DClipL;
    T = s_2DClipT;
    R = s_2DClipR;
    B = s_2DClipB;
}

//==========================================================================

void ENG_SetActiveLighting( const lighting& Lighting )
{
    //--- Copy lighting for engine use
    x_memcpy( &s_ActiveLighting, &Lighting, sizeof(lighting) );

    GC_SetupLighting();
}

//==========================================================================

void GC_UpdateLights( void )
{
    const f32       LARGE_DIST = 1000000;
    const GXLightID GC_LightID[MAX_DIR_LIGHTS] = { GX_LIGHT0, GX_LIGHT1, GX_LIGHT2 };

    s32     i;
    vector3 LightPos;
    matrix4 W2V;

    ENG_GetActiveView()->GetW2VMatrix( W2V );
    W2V.RotateY( R_180 );
    W2V.ClearTranslation();

    for( i = 0; i < MAX_DIR_LIGHTS; i++ )
    {
        //--- Transform light dir from world-space to view space
        LightPos.X = s_ActiveLighting.Directional[i].Direction.X * -LARGE_DIST;
        LightPos.Y = s_ActiveLighting.Directional[i].Direction.Y * -LARGE_DIST;
        LightPos.Z = s_ActiveLighting.Directional[i].Direction.Z * -LARGE_DIST;
        LightPos   = W2V * LightPos;

        //--- Directional lights are simply "light bulbs" that are really far away
        GXInitLightPos( &s_GCLights[i], LightPos.X, LightPos.Y, LightPos.Z );

        if( s_ActiveLighting.Directional[i].Active )
            GXLoadLightObjImm( &s_GCLights[i], GC_LightID[i] );
    }
}

//==========================================================================

void GC_SetupLighting( void )
{
    const f32       LARGE_DIST = 1000000;
    const GXLightID GC_LightID[MAX_DIR_LIGHTS] = { GX_LIGHT0, GX_LIGHT1, GX_LIGHT2 };

    s32     i;
    vector3 LightPos;
    matrix4 W2V;
    GXColor LightClr;

    ENG_GetActiveView()->GetW2VMatrix( W2V );
    W2V.RotateY( R_180 );
    W2V.ClearTranslation();

    for( i = 0; i < MAX_DIR_LIGHTS; i++ )
    {
        if( s_ActiveLighting.Directional[i].Active )
        {
            s_ActiveLighting.Directional[i].Direction.Normalize();
        }
        else
        {
            s_ActiveLighting.Directional[i].Direction.Set( 0.0f, 0.0f, -1.0f );
            s_ActiveLighting.Directional[i].Color.Set( 255, 255, 255, 255 );
        }

        //--- Transform light dir from world-space to view space
        LightPos.X = s_ActiveLighting.Directional[i].Direction.X * -LARGE_DIST;
        LightPos.Y = s_ActiveLighting.Directional[i].Direction.Y * -LARGE_DIST;
        LightPos.Z = s_ActiveLighting.Directional[i].Direction.Z * -LARGE_DIST;
        LightPos   = W2V * LightPos;

        //--- Directional lights are simply "light bulbs" that are really far away
        GXInitLightPos( &s_GCLights[i], LightPos.X, LightPos.Y, LightPos.Z );

        //--- set the light's color
        LightClr.r = s_ActiveLighting.Directional[i].Color.R;
        LightClr.g = s_ActiveLighting.Directional[i].Color.G;
        LightClr.b = s_ActiveLighting.Directional[i].Color.B;
        LightClr.a = s_ActiveLighting.Directional[i].Color.A;

        GXInitLightColor( &s_GCLights[i], LightClr );

        //--- load the light into hardware if active
        if( s_ActiveLighting.Directional[i].Active )
        {
            s_GCActiveLights |= GC_LightID[i];
            GXLoadLightObjImm( &s_GCLights[i], GC_LightID[i] );
        }
        else
        {
            s_GCActiveLights &= ~GC_LightID[i];
        }
    }

    //--- Set number of channels to light
    GXSetNumChans( 1 );

    //--- set the ambient color
    LightClr.r = s_ActiveLighting.Ambient.R;
    LightClr.g = s_ActiveLighting.Ambient.G;
    LightClr.b = s_ActiveLighting.Ambient.B;
    LightClr.a = s_ActiveLighting.Ambient.A;

    GXSetChanAmbColor( GX_COLOR0, LightClr );
    GXSetChanAmbColor( GX_ALPHA0, LightClr );

    //--- set the material color
    LightClr.r = 255;
    LightClr.g = 255;
    LightClr.b = 255;
    LightClr.a = 255;

    GXSetChanMatColor( GX_COLOR0, LightClr );
    GXSetChanMatColor( GX_ALPHA0, LightClr );

    GC_EnableLighting( TRUE, FALSE );
}

//==========================================================================

void GC_EnableLighting( xbool Enable, xbool VtxColors )
{
    xbool ENGLightOn = (s_LightingFlags & ENG_LIGHTING_ON) ? TRUE : FALSE;

    if( Enable && !ENGLightOn )
    {
        s_LightingFlags &= ~(ENG_LIGHTING_ON  | ENG_LIGHTING_OFF);
        s_LightingFlags |= ENG_LIGHTING_ON;

        GXColorSrc MatClr = VtxColors ? GX_SRC_VTX : GX_SRC_REG;

        //--- set up the lighting for the color channel
        GXSetChanCtrl(  GX_COLOR0,          // color channel register
                        GX_TRUE,            // enable/disable lighting
                        GX_SRC_REG,         // ambient color src(register or vtx)
                        MatClr,             // material color src(register or vtx)
                        s_GCActiveLights,   // lights to apply to this channel
                        GX_DF_CLAMP,        // normal diffuse light
                        GX_AF_NONE );       // no attenuation

        GXSetChanCtrl( GX_ALPHA0, GX_TRUE, GX_SRC_REG, MatClr, s_GCActiveLights, GX_DF_CLAMP, GX_AF_NONE );
    }
    else if( !Enable && ENGLightOn )
    {
        s_LightingFlags &= ~(ENG_LIGHTING_ON  | ENG_LIGHTING_OFF);
        s_LightingFlags |= ENG_LIGHTING_OFF;

        //--- disable lighting on color channel 0
        GXSetChanCtrl( GX_COLOR0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE );
        GXSetChanCtrl( GX_ALPHA0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE );
    }
}

//==========================================================================

void ENG_GetActiveLighting( lighting& Lighting )
{
    Lighting = s_ActiveLighting;
}

//==========================================================================

void ENG_SetLight( s32 LightID, vector3& Dir, color& Color )
{
    const f32       LARGE_DIST = 1000000;
    const GXLightID GC_LightID[MAX_DIR_LIGHTS] = { GX_LIGHT0, GX_LIGHT1, GX_LIGHT2 };

    ASSERT( (LightID >= 0) && (LightID < MAX_DIR_LIGHTS) );

    vector3 LightPos;
    matrix4 W2V;
    GXColor LightClr;

    //--- copy info to engine light
    s_ActiveLighting.Directional[LightID].Active    = TRUE;
    s_ActiveLighting.Directional[LightID].Color     = Color;
    s_ActiveLighting.Directional[LightID].Direction = Dir;
    s_ActiveLighting.Directional[LightID].Direction.Normalize();

    ENG_GetActiveView()->GetW2VMatrix( W2V );
    W2V.RotateY( R_180 );
    W2V.ClearTranslation();

    //--- Transform light dir from world-space to view space
    LightPos.X = s_ActiveLighting.Directional[LightID].Direction.X * -LARGE_DIST;
    LightPos.Y = s_ActiveLighting.Directional[LightID].Direction.Y * -LARGE_DIST;
    LightPos.Z = s_ActiveLighting.Directional[LightID].Direction.Z * -LARGE_DIST;
    LightPos   = W2V * LightPos;

    //--- Directional lights are simply "light bulbs" that are really far away
    GXInitLightPos( &s_GCLights[LightID], LightPos.X, LightPos.Y, LightPos.Z );

    //--- set the light's color
    LightClr.r = Color.R;
    LightClr.g = Color.G;
    LightClr.b = Color.B;
    LightClr.a = Color.A;

    GXInitLightColor( &s_GCLights[LightID], LightClr );

    //--- load the light into hardware
    s_GCActiveLights |= GC_LightID[LightID];
    GXLoadLightObjImm( &s_GCLights[LightID], GC_LightID[LightID] );
}

//==========================================================================

void ENG_SetAmbientLight( color& Color )
{
    s_ActiveLighting.Ambient = Color;

    GXColor AmbientClr;

    AmbientClr.r = Color.R;
    AmbientClr.g = Color.G;
    AmbientClr.b = Color.B;
    AmbientClr.a = Color.A;

    GXSetChanAmbColor( GX_COLOR0, AmbientClr );
    GXSetChanAmbColor( GX_ALPHA0, AmbientClr );
}

//==========================================================================

void ENG_SpecialCaseFog( xbool bActivateSpecial )
{
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

//==========================================================================

void ENG_BeginRenderMode( void )
{
    ASSERT( !s_InsideRenderMode );

    s_InsideRenderMode = TRUE;
}

//==========================================================================

void ENG_EndRenderMode( void )
{
    ASSERT( s_InsideRenderMode );

    s_InsideRenderMode = FALSE;
}

//==========================================================================

xbool ENG_GetRenderMode( void )
{
    return s_InsideRenderMode;
}

//==========================================================================

void ENG_GetRenderFlags( u32& RenderFlags )
{
    RenderFlags = s_RenderFlags;
}

//==========================================================================

void ENG_GetBlendMode( u32& BlendMode, u8& FixedAlpha )
{
    BlendMode  = s_BlendFlags;
    FixedAlpha = s_FixedAlpha;
}

//==========================================================================

void GC_GetFixedAlpha( xbool& IsFixedAlphaUsed, u8& FixedAlpha )
{
    IsFixedAlphaUsed = (s_BlendFlags & ENG_BLEND_FIXED_ALPHA) ? TRUE : FALSE;
    FixedAlpha = s_FixedAlpha;
}

//==========================================================================

void ENG_SetBlendMode( u32 BlendFlags, u8 FixedAlpha )
{
    ASSERT( ENG_GetRenderMode() );

    GC_SetAlphaBlending( BlendFlags, FixedAlpha, FALSE );
}

//==========================================================================

void ENG_SetRenderFlags( u32 RenderFlags )
{
    ASSERT( ENG_GetRenderMode() );

    ASSERT( RenderFlags & (ENG_ZBUFFER_TEST_ON | ENG_ZBUFFER_TEST_OFF) );
    ASSERT( RenderFlags & (ENG_ZBUFFER_FILL_ON | ENG_ZBUFFER_FILL_OFF) );
    ASSERT( RenderFlags & (ENG_ALPHA_BLEND_ON  | ENG_ALPHA_BLEND_OFF) );
    ASSERT( RenderFlags & (ENG_ALPHA_TEST_ON   | ENG_ALPHA_TEST_OFF) );

    if( s_RenderFlags == RenderFlags )
        return;

    //--- set the Z buffer mode
    GC_SetZMode( RenderFlags & ENG_ZBUFFER_TEST_ON, RenderFlags & ENG_ZBUFFER_FILL_ON );

    //--- check if new blend state is needed
    if( s_RenderFlags & ENG_ALPHA_BLEND_ON )
    {
        if( RenderFlags & ENG_ALPHA_BLEND_OFF )
        {
            //--- changing from BLEND_ON to BLEND_OFF
            if( s_BlendFlags & ENG_BLEND_FIXED_ALPHA )
                GXSetDstAlpha( GX_FALSE, 0x00 ); //turn off fixed alpha for no-alpha mode

            GXSetBlendMode( GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR );

            s_RenderFlags &= ~(ENG_ALPHA_BLEND_OFF | ENG_ALPHA_BLEND_ON);
            s_RenderFlags |= ENG_ALPHA_BLEND_OFF;
        }
    }
    else if( RenderFlags & ENG_ALPHA_BLEND_ON )
    {
        //--- changing from BLEND_OFF to BLEND_ON
        s_RenderFlags &= ~(ENG_ALPHA_BLEND_OFF | ENG_ALPHA_BLEND_ON);
        s_RenderFlags |= ENG_ALPHA_BLEND_ON;

        GC_SetAlphaBlending( s_BlendFlags, s_FixedAlpha, TRUE );
    }

    //--- set alpha test mode
    GC_SetAlphaTest( RenderFlags & ENG_ALPHA_TEST_ON );
}

//==========================================================================

void GC_SetZMode( xbool ZTestOn, xbool ZFillOn, xbool bUseLEQUAL )
{
    u32 ZTestFlag = ZTestOn ? ENG_ZBUFFER_TEST_ON : ENG_ZBUFFER_TEST_OFF;
    u32 ZFillFlag = ZFillOn ? ENG_ZBUFFER_FILL_ON : ENG_ZBUFFER_FILL_OFF;

    //--- if Z test and fill are the same, no need to do anything
    if( (ZTestFlag == (s_RenderFlags & (ENG_ZBUFFER_TEST_ON | ENG_ZBUFFER_TEST_OFF))) &&
        (ZFillFlag == (s_RenderFlags & (ENG_ZBUFFER_FILL_ON | ENG_ZBUFFER_FILL_OFF))) )
    {
        bUseLEQUAL = bUseLEQUAL ? TRUE : FALSE;

        //--- if Z test is on and ZComp is different, we can't bail out early
        if( !(ZTestOn && (bUseLEQUAL != s_ZCompUseLEQUAL)) )
            return;
    }

    //--- save Z test and fill mode
    s_RenderFlags &= ~(ENG_ZBUFFER_TEST_ON | ENG_ZBUFFER_TEST_OFF | ENG_ZBUFFER_FILL_ON | ENG_ZBUFFER_FILL_OFF);
    s_RenderFlags |= (ZTestFlag | ZFillFlag);

    //--- set GameCube ZMode
    if( ZTestOn )
    {
        s_ZCompUseLEQUAL = bUseLEQUAL ? TRUE : FALSE;

        GXCompare ZComp = bUseLEQUAL ? GX_LEQUAL : GX_LESS;

        if( ZFillOn )
            GXSetZMode( GX_TRUE, ZComp, GX_TRUE );
        else
            GXSetZMode( GX_TRUE, ZComp, GX_FALSE );
    }
    else
    {
        if( ZFillOn )
            GXSetZMode( GX_TRUE, GX_ALWAYS, GX_TRUE );
        else
            GXSetZMode( GX_FALSE, GX_ALWAYS, GX_FALSE );
    }
}

//==========================================================================

void GC_SetAlphaBlending( u32 BlendFlags, u8 FixedAlpha, xbool ForceChange )
{
    ASSERT( BlendFlags != 0 );

    if( !ForceChange && (BlendFlags == s_BlendFlags) && (FixedAlpha == s_FixedAlpha) )
        return;

    //--- save blend flags and fixed alpha
    s_BlendFlags = BlendFlags;
    s_FixedAlpha = FixedAlpha;

    if( s_RenderFlags & ENG_ALPHA_BLEND_OFF )
        return;

    //--- set fixed alpha if needed
    //if( BlendFlags & ENG_BLEND_FIXED_ALPHA )
    //    GXSetDstAlpha( GX_TRUE, FixedAlpha );
    //else
    //    GXSetDstAlpha( GX_FALSE, 0x00 );

    //--- set the blending equation
    switch( BlendFlags & 0x0F )
    {
        case ENG_BLEND_NORMAL:
            GXSetBlendMode( GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR );
            break;

        case ENG_BLEND_ADDITIVE:
            GXSetBlendMode( GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR );
            break;

        case ENG_BLEND_SUBTRACTIVE:
            //### NOTE: this only gives (DC - SC), not (DC - (SC * SA)) like it should be
            GXSetBlendMode( GX_BM_SUBTRACT, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR );
            break;

        case ENG_BLEND_DEST:
            GXSetBlendMode( GX_BM_BLEND, GX_BL_ZERO, GX_BL_ONE, GX_LO_CLEAR );
            break;

        case ENG_BLEND_MULTIPLY:
            //### NOTE: assuming the multiply function is (SC * DC)
            GXSetBlendMode( GX_BM_BLEND, GX_BL_DSTCLR, GX_BL_ONE, GX_LO_CLEAR );
            break;

        default:
            ASSERT( FALSE );
            break;
    }
}

//==========================================================================

void GC_SetAlphaTest( xbool AlphaTestOn )
{
    xbool ENGATestOn = (s_RenderFlags & ENG_ALPHA_TEST_ON) ? TRUE : FALSE;

    if( AlphaTestOn && !ENGATestOn )
    {
        //--- Alpha test is being turned on(from off state)
        s_RenderFlags &= ~(ENG_ALPHA_TEST_ON | ENG_ALPHA_TEST_OFF);

        //--- Alpha passes if SrcAlpha >= 0x80 (128)
        GXSetAlphaCompare( GX_EQUAL, 0xFF, GX_AOP_OR, GX_GEQUAL, 0x80 );
        GXSetZCompLoc( GX_FALSE );

        s_RenderFlags |= ENG_ALPHA_TEST_ON;
    }
    else if( !AlphaTestOn && ENGATestOn )
    {
        //--- Alpha test is being turned off(from on state)
        s_RenderFlags &= ~(ENG_ALPHA_TEST_ON | ENG_ALPHA_TEST_OFF);

        //--- Alpha passes always
        GXSetAlphaCompare( GX_ALWAYS, 0x00, GX_AOP_AND, GX_ALWAYS, 0x00 );
        GXSetZCompLoc( GX_TRUE );

        s_RenderFlags |= ENG_ALPHA_TEST_OFF;
    }
}

//==========================================================================

void ENG_EnableRGBAChannels( xbool R, xbool G, xbool B, xbool A )
{
    if( R || G || B )
        GXSetColorUpdate( GX_TRUE );
    else
        GXSetColorUpdate( GX_FALSE );

    if( A )
        GXSetAlphaUpdate( GX_TRUE );
    else
        GXSetAlphaUpdate( GX_FALSE );
}

//==========================================================================

void ENG_SetBilinear( xbool /*OnOff*/ )
{
    //### GAMECUBE NOT SUPPORTED
}

//==========================================================================

xbool ENG_GetBilinear( void )
{
    return FALSE;   //### GAMECUBE NOT SUPPORTED
}

//==========================================================================

void ENG_SetBackgroundColor( color C )
{
    s_BackgroundColor = C;
}

//==========================================================================

color ENG_GetBackgroundColor( void )
{
    return s_BackgroundColor;
}

//==========================================================================

void GC_GetProjMatrix( f32 pMtx[4][4] )
{
    f32 ProjValues[GX_PROJECTION_SZ];

    //--- Projection values are in elements 1 through 6, 0 is ignored
    GXGetProjectionv( ProjValues );

    //--- fill out the matrix according to how the GameCube needs it
    pMtx[0][0] = ProjValues[1];
    pMtx[0][1] = 0;
    pMtx[0][2] = ProjValues[2];
    pMtx[0][3] = 0;
    pMtx[1][0] = 0;
    pMtx[1][1] = ProjValues[3];
    pMtx[1][2] = ProjValues[4];
    pMtx[1][3] = 0;
    pMtx[2][0] = 0;
    pMtx[2][1] = 0;
    pMtx[2][2] = ProjValues[5];
    pMtx[2][3] = ProjValues[6];
    pMtx[3][0] = 0;
    pMtx[3][1] = 0;
    pMtx[3][2] = -1;
    pMtx[3][3] = 0;
}

//==========================================================================

void ENG_FillScreenArea( s32 Area, color C )
{
    ASSERT( (Area == ENG_AREA_SCREEN) || (Area == ENG_AREA_2D_CLIP) || (Area == ENG_AREA_3D_VIEWPORT) );

    s32 L, T, R, B;
    s32 VPL, VPT, VPW, VPH;
    s32 XRes, YRes;

    ENG_GetResolution( XRes, YRes );

    //--- Get rectangle coordinates for quad
    switch( Area )
    {
        case ENG_AREA_SCREEN:
            L = 0;
            T = 0;
            R = XRes - 1;
            B = YRes - 1;
            break;

        case ENG_AREA_2D_CLIP:
            ENG_Get2DClipArea( L, T, R, B );
            break;

        case ENG_AREA_3D_VIEWPORT:
            ENG_GetActiveView()->GetViewport( L, T, R, B );
            break;
    }

    //--- save current viewport and make new full-screen one
    GC_GetViewport( VPL, VPT, VPW, VPH );
    GC_SetViewport( 0, 0, XRes, YRes );

    //--- set z buffer test to always pass for clearing Zbuffer
    GC_SetZMode( FALSE, TRUE );

    //--- disable lighting
    GC_EnableLighting( FALSE, FALSE );

    //--- save current projection matrix and set up an orthographic one
    Mtx44 MOrthoProj;
    Mtx44 MSavedProj;

    GC_GetProjMatrix( MSavedProj );

    MTXOrtho( MOrthoProj, 0, YRes-1, 0, XRes-1, 0.0f, 1.0f );

    GXSetProjection( MOrthoProj, GX_ORTHOGRAPHIC );

    //--- clear the current local to view matrix so quad isn't affected by it
    Mtx GC_L2V;
    MTXIdentity( GC_L2V );
    GXLoadPosMtxImm( GC_L2V, GX_PNMTX5 );
    GXLoadNrmMtxImm( GC_L2V, GX_PNMTX5 );
    GXSetCurrentMtx( GX_PNMTX5 );

    //--- set the vertex description and format
    GXClearVtxDesc ();
    GXSetVtxDesc   ( GX_VA_POS,  GX_DIRECT );
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_POS,  GX_POS_XYZ,  GX_F32,   0 );

    //--- set tev to one stage, no texture, fixed color
    GXColor BKColor = { C.R, C.G, C.B, C.A };

    GXSetNumChans    ( 1 );
    GXSetNumTevStages( 1 );
    GXSetNumTexGens  ( 0 );
    GXSetTevOrder    ( GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0 );
    GXSetTevColor    ( GX_TEVREG0, BKColor );
    GXSetTevColorIn  ( GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,    GX_CC_C0 );
    GXSetTevColorOp  ( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );
    GXSetTevAlphaIn  ( GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,    GX_CA_A0 );
    GXSetTevAlphaOp  ( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );

    const f32 ZDEPTH = -0.9999995f;

    //--- draw the quad
    GXBegin( GX_QUADS, GX_VTXFMT0, 4 );
    {
        GXPosition3f32( L, T, ZDEPTH );
        GXPosition3f32( R, T, ZDEPTH );
        GXPosition3f32( R, B, ZDEPTH );
        GXPosition3f32( L, B, ZDEPTH );
    }
    GXEnd();

    //--- restore the projection matrix and viewport
    GXSetProjection( MSavedProj, GX_PERSPECTIVE );
    GC_SetViewport( VPL, VPT, VPW, VPH );
}

//==========================================================================

void ENG_FillZBufferArea( s32 Area, f32 Depth )
{
    ASSERT( (Area == ENG_AREA_SCREEN) || (Area == ENG_AREA_2D_CLIP) || (Area == ENG_AREA_3D_VIEWPORT) );

    s32 L, T, R, B;
    s32 VPL, VPT, VPW, VPH;
    s32 XRes, YRes;

    ENG_GetResolution( XRes, YRes );

    //--- Get rectangle coordinates for quad
    switch( Area )
    {
        case ENG_AREA_SCREEN:
            L = 0;
            T = 0;
            R = XRes - 1;
            B = YRes - 1;
            break;

        case ENG_AREA_2D_CLIP:
            ENG_Get2DClipArea( L, T, R, B );
            break;

        case ENG_AREA_3D_VIEWPORT:
            ENG_GetActiveView()->GetViewport( L, T, R, B );
            break;
    }

    //--- save current viewport and make new full-screen one
    GC_GetViewport( VPL, VPT, VPW, VPH );
    GC_SetViewport( 0, 0, XRes, YRes );

    //--- set z buffer test to always pass for clearing Zbuffer
    //--- also disable color buffer update since we don't want to clear it
    ENG_SetRenderFlags( ENG_ZBUFFER_TEST_OFF | ENG_ZBUFFER_FILL_ON | ENG_ALPHA_TEST_OFF | ENG_ALPHA_BLEND_OFF );
    GXSetColorUpdate( GX_FALSE );
    GXSetAlphaUpdate( GX_FALSE );

    //--- disable lighting
    GC_EnableLighting( FALSE, FALSE );

    //--- save current projection matrix and set up an orthographic one
    Mtx44 MOrthoProj;
    Mtx44 MSavedProj;

    GC_GetProjMatrix( MSavedProj );

    MTXOrtho( MOrthoProj, 0, YRes-1, 0, XRes-1, 0.0f, 1.0f );

    GXSetProjection( MOrthoProj, GX_ORTHOGRAPHIC );

    //--- clear the current local to view matrix so quad isn't affected by it
    Mtx GC_L2V;
    MTXIdentity( GC_L2V );
    GXLoadPosMtxImm( GC_L2V, GX_PNMTX5 );
    GXLoadNrmMtxImm( GC_L2V, GX_PNMTX5 );
    GXSetCurrentMtx( GX_PNMTX5 );

    //--- set the vertex description and format
    GXClearVtxDesc ();
    GXSetVtxDesc   ( GX_VA_POS,  GX_DIRECT );
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0 );

    //--- set tev to one stage, no texture
    GXColor BKColor = { 0, 0, 0, 0 };
    GXSetNumChans    ( 1 );
    GXSetNumTevStages( 1 );
    GXSetNumTexGens  ( 0 );
    GXSetTevOrder    ( GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0 );
    GXSetTevColor    ( GX_TEVREG0, BKColor );
    GXSetTevColorIn  ( GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,    GX_CC_C0 );
    GXSetTevColorOp  ( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );
    GXSetTevAlphaIn  ( GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,    GX_CA_A0 );
    GXSetTevAlphaOp  ( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );

    const f32 MAX_Z_VALUE = -0.9999995f;

    if( Depth < 0.0f )  Depth = 0.0f;
    if( Depth > 1.0f )  Depth = 1.0f;

    Depth *= MAX_Z_VALUE;

    //--- draw the quad
    GXBegin( GX_QUADS, GX_VTXFMT0, 4 );
    {
        GXPosition3f32( L, T, Depth );
        GXPosition3f32( R, T, Depth );
        GXPosition3f32( R, B, Depth );
        GXPosition3f32( L, B, Depth );
    }
    GXEnd();

    //--- restore the projection matrix and viewport
    GXSetProjection( MSavedProj, GX_PERSPECTIVE );
    GC_SetViewport( VPL, VPT, VPW, VPH );

    //--- restore color update
    GXSetColorUpdate( GX_TRUE );
    GXSetAlphaUpdate( GX_TRUE );
}

//==========================================================================

static void GC_CalcRefPlane( const vector4& rPlane )
{
    f32     NearZ;
    f32     FarZ;
    matrix4 W2V;
    matrix4 V2W;
    vector3 TempV[3];

    //--- get camera matrices and view distances
    ENG_GetActiveView()->GetW2VMatrix( W2V );
    ENG_GetActiveView()->GetV2WMatrix( V2W );
    ENG_GetActiveView()->GetZLimits( NearZ, FarZ );

    //--- rotate W2V 180 on Y to XForm verts to GameCube camera space
    W2V.RotateY( R_180 );

    //--- create large triangle within camera's view
    TempV[0].Set( 100.0f, -50.0f, NearZ + 0.5f );
    TempV[1].Set(-100.0f, -50.0f, NearZ + 0.5f );
    TempV[2].Set(   0.0f,  50.0f, FarZ  - 0.5f );

    //--- XForm those verts to world-space
    TempV[0] = V2W.Transform( TempV[0] );
    TempV[1] = V2W.Transform( TempV[1] );
    TempV[2] = V2W.Transform( TempV[2] );

    //--- 'project' verts onto plane, by solving for one componnent in plane equation
    if( rPlane.Y != 0.0f )
    {
        TempV[0].Y = (rPlane.W - (rPlane.X * TempV[0].X) - (rPlane.Z * TempV[0].Z)) / rPlane.Y;
        TempV[1].Y = (rPlane.W - (rPlane.X * TempV[1].X) - (rPlane.Z * TempV[1].Z)) / rPlane.Y;
        TempV[2].Y = (rPlane.W - (rPlane.X * TempV[2].X) - (rPlane.Z * TempV[2].Z)) / rPlane.Y;
    }
    else if( rPlane.X != 0.0f )
    {
        TempV[0].X = (rPlane.W - (rPlane.Y * TempV[0].Y) - (rPlane.Z * TempV[0].Z)) / rPlane.X;
        TempV[1].X = (rPlane.W - (rPlane.Y * TempV[1].Y) - (rPlane.Z * TempV[1].Z)) / rPlane.X;
        TempV[2].X = (rPlane.W - (rPlane.Y * TempV[2].Y) - (rPlane.Z * TempV[2].Z)) / rPlane.X;
    }
    else if( rPlane.Z != 0.0f )
    {
        TempV[0].Z = (rPlane.W - (rPlane.X * TempV[0].X) - (rPlane.Y * TempV[0].Y)) / rPlane.Z;
        TempV[1].Z = (rPlane.W - (rPlane.X * TempV[1].X) - (rPlane.Y * TempV[1].Y)) / rPlane.Z;
        TempV[2].Z = (rPlane.W - (rPlane.X * TempV[2].X) - (rPlane.Y * TempV[2].Y)) / rPlane.Z;
    }
    else
    {
        ASSERT( FALSE );
    }

    //--- XForm verts(now on plane) from world-space to GameCube camera space
    s_RefPlaneVerts[0] = W2V.Transform( TempV[0] );
    s_RefPlaneVerts[1] = W2V.Transform( TempV[1] );
    s_RefPlaneVerts[2] = W2V.Transform( TempV[2] );

    s_RefPlane = rPlane;

    s_RefPlaneCalculated = TRUE;
}

//==========================================================================

void GC_EnableRefPlane( const vector4& rPlane )
{
    //--- do nothing if plane is same as current one and already setup
    if( s_RefPlaneEnabled && (rPlane == s_RefPlane) )
        return;

    //--- re-calc plane verts if needed
    if( !s_RefPlaneCalculated || (rPlane != s_RefPlane) )
        GC_CalcRefPlane( rPlane );

    //--- verts are already in camera space, no XForm necessary
    GXSetCurrentMtx( GX_IDENTITY );

    //--- setup vertex desc and format
    GXClearVtxDesc();
    GXSetVtxDesc( GX_VA_POS, GX_DIRECT );
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0 );

    //--- setup TEV
    GXSetNumChans    ( 1 );
    GXSetNumTevStages( 1 );
    GXSetNumTexGens  ( 0 );
    GXSetTevOrder    ( GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL );
    GXSetTevColorIn  ( GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,    GX_CC_ZERO );
    GXSetTevAlphaIn  ( GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,    GX_CA_ZERO );
    GXSetTevColorOp  ( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );
    GXSetTevAlphaOp  ( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );

    //--- make this triangle is invisible and render it
    GXSetCoPlanar( GX_FALSE );
    GXSetCullMode( GX_CULL_ALL );

    GXBegin( GX_TRIANGLES, GX_VTXFMT0, 3 );
    {
        GXPosition3f32( s_RefPlaneVerts[0].X, s_RefPlaneVerts[0].Y, s_RefPlaneVerts[0].Z );
        GXPosition3f32( s_RefPlaneVerts[1].X, s_RefPlaneVerts[1].Y, s_RefPlaneVerts[1].Z );
        GXPosition3f32( s_RefPlaneVerts[2].X, s_RefPlaneVerts[2].Y, s_RefPlaneVerts[2].Z );
    }
    GXEnd();

    //--- set co-planar mode on and restore cull mode
    GXSetCullMode( GX_CULL_NONE );
    GXSetCoPlanar( GX_TRUE );

    s_RefPlaneEnabled = TRUE;
}

//==========================================================================

void GC_DisableRefPlane( xbool bPlaneCalcReset )
{
    //--- disable the reference plane
    if( s_RefPlaneEnabled )
    {
        GXSetCoPlanar( GX_FALSE );

        s_RefPlaneEnabled = FALSE;
    }

    //--- check if forced calculation needed(i.e. camera view changes)
    if( bPlaneCalcReset )
    {
        s_RefPlaneCalculated = FALSE;
    }
}

//==========================================================================

void ENG_GetResolution( s32& Width, s32& Height )
{
    Width  = s_XRes;
    Height = s_YRes;
}

//==========================================================================

void ENG_WaitForGraphicsSynch( void )
{
    //### TODO: fill this out
}

//==========================================================================

f32 ENG_GetFPS( void )
{
    return( 1.0f / s_FPSTimer.GetAvgTime() );
}

//==========================================================================

f64 ENG_GetRunTime( void )
{
    return (f64)((s64)OSGetTime() - s_StartRunTime);
}

//==========================================================================

s32 ENG_GetPageFlipCount( void )
{
    return (s32)s_Frame;
}

//==========================================================================

void ENG_PrintStats( void )
{
    //if( (s_Frame & 0x0F) == 0 )
    {
        x_printfxy( 0, 1, "FPS:  %3.1f", ENG_GetFPS() );
        x_printfxy( 0, 2, "CPU:  %4.2f", 1000.0f * s_CPUTimer.GetAvgTime() );
        x_printfxy( 0, 3, "TOT:  %4.2f", 1000.0f * s_FPSTimer.GetAvgTime() );
        //x_printfxy( 0, 4, "FRAME: %ud", s_Frame );
    }
}

//==========================================================================

static void GAMECUBE_PrintString( const char* pString )
{
#ifdef X_DEBUG
    // dummy printf function, prints to output window
	BOOL bPrevState = OSDisableInterrupts();
    OSReport( (char*)pString );
	OSRestoreInterrupts( bPrevState );
#endif
}

//==========================================================================

static void GC_InitMemory( void )
{
    void* pArenaLo;
    void* pArenaHi;

    //--- Get pointers to low and high points of memory
    pArenaLo = OSGetArenaLo();
    pArenaHi = OSGetArenaHi();

    //--- Ensure boundaries are 32B aligned, and pad ends by 1K
    pArenaLo = (void*)OSRoundUp32B( (u32)pArenaLo + 1024 );
    pArenaHi = (void*)OSRoundDown32B( (u32)pArenaHi - 1024 );

#ifdef X_DEBUG
	x_memset( pArenaLo, 0xBE, (u32)pArenaHi - (u32)pArenaLo );
#endif

    //--- Init OS memory system, 1 heap max
    pArenaLo = OSInitAlloc( pArenaLo, pArenaHi, 1 );
    OSSetArenaLo( pArenaLo );

    //--- Create heap and make it the current one
    OSSetCurrentHeap( OSCreateHeap(pArenaLo, pArenaHi) );

    u32 ArenaSize = (u32)pArenaHi - (u32)pArenaLo;

#if defined( X_DEBUG ) && 1
	if( ArenaSize > (24*1024*1024) )
	{
		ArenaSize = (u32)(20.0f * 1024*1024);
	}
#endif

    x_MemoryMaxRAM( ArenaSize );
}

//==========================================================================

s32 main( s32 argc, char* argv[] )
{
    //--- Init GameCube OS and DVD systems
    OSInit();
    DVDInit();

    s_ResetPressed = FALSE;
    s_ResetPressedFromCallback = FALSE;

    //OSSetResetCallback( GC_ResetCallback );

    GC_InitMemory();

    //--- Setup filesystem  
    DVD_InitModule();

	GC_InitSNProfiler();

    //--- Call the application init function to allow application specific changes
    xbool InitDone = QuagInit();

    //--- if the initialization has not been completed, init x_files now
    if( !InitDone )
        x_Init();

    x_SetPrintHandlers( (print_fnptr)GAMECUBE_PrintString, (print_at_fnptr)TEXT_PrintStringAt );

    //--- Call the clients main
    s32 Val = QuagMain( argc, argv );

    //--- clean up
    x_Kill();

    DVD_KillModule();

    return Val;
}

void ENG_ForceNonBlockingVSync( void )
{
}

//==========================================================================



////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// GAMECUBE PERFORMANCE GRAPH LIBRARY
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#if defined( ENABLE_GC_PERF_LIB )

//--------------------------------------------------------------------------

// header include to use the perf library
#include "dolphin/perf.h"


// size of display list used to render the graphs
const s32 GCPERF_DLIST_SIZE = (128 * 1024);

// variables for perf graph display list
static byte* s_GCPERF_DListA  = NULL;
static byte* s_GCPERF_DListB  = NULL;
static s32   s_GCPERF_DLSize  = 0;

static u32   s_GCPERF_Count   = 0;

static xbool s_GCPERF_Enabled = FALSE;

static f64   s_GCPERF_EnableTimer = -1;


////////////////////////////////////////////////////////////////////////////

static void* GCPERF_Alloc( u32 size )
{
    void* pMem;

    pMem = x_malloc( size );

    ASSERT( pMem != NULL );

    return pMem;
}

//==========================================================================

static void GCPERF_Free( void* pMem )
{
    ASSERT( pMem != NULL );

    x_free( pMem );
}

//==========================================================================

static void GCPERF_RestoreGXState( void )
{
    // NOTE: this function is only used so the application can restore any states
    //       modified by the PERF lib.  See documentation on PERF to know more.

    GXColor white = { 255, 255, 255, 255 };
    GXColor black = {   0,   0,   0, 255 };

    GXSetChanCtrl( GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE );

    GXSetChanAmbColor( GX_COLOR0A0, black );
    GXSetChanMatColor( GX_COLOR0A0, white );

    GXSetBlendMode( GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR );

    GXSetZMode( GX_TRUE, GX_LEQUAL, GX_TRUE );
    GXSetZCompLoc( GX_TRUE );
}

//==========================================================================

static void GCPERF_Init( void )
{
    s32     i;

    // Setup the PERF lib
    PERFInit( GCPERF_NUM_SAMPLES,
              1,                    //NUM_FRAMES, always 1
              GCPERF_EVENT_COUNT,
              GCPERF_Alloc,
              GCPERF_Free,
              GCPERF_RestoreGXState );

    // Setup defined events
    for( i = 0; i < GCPERF_EVENT_COUNT; i++ )
    {
        PERFSetEvent( i, "EVENT", PERF_CPU_GP_EVENT );
        PERFSetEventColor( i, GCPERF_EVENT_COLOR[i] );
    }

    // turn on bandwidth color key
    PERFSetDrawBWBarKey( TRUE );

    // create display list buffer
    s_GCPERF_DListA = (byte*)GCPERF_Alloc( GCPERF_DLIST_SIZE );
    s_GCPERF_DListB = (byte*)GCPERF_Alloc( GCPERF_DLIST_SIZE );

    ASSERT( s_GCPERF_DListA != NULL );
    ASSERT( s_GCPERF_DListB != NULL );

    s_GCPERF_Count = 0;

    s_GCPERF_Enabled = FALSE;
}

//==========================================================================

static void GCPERF_Toggle( void )
{
    if( s_GCPERF_Enabled )
    {
        s_GCPERF_Enabled = FALSE;

        GCPERF_EventEnd( GCPERF_EVENT_APP_TIME );

        PERFEndFrame();
        PERFStopAutoSampling();
    }
    else
    {
        s_GCPERF_Enabled = TRUE;

        PERFStartFrame();
        PERFStartAutoSampling( GCPERF_AUTOSAMPLE_RATE );

        GCPERF_EventStart( GCPERF_EVENT_APP_TIME );
    }
}

//==========================================================================

static void GCPERF_Update( void )
{
	if( INPUT_GetButton( 0, GADGET_GAMECUBE_L_TRIG ) )
    {
		if( s_GCPERF_EnableTimer != -1 )
		{
			if( x_GetElapsedTime( s_GCPERF_EnableTimer ) >= 0.25f )
			{
				s_GCPERF_EnableTimer = -1;
				GCPERF_Toggle();
			}
		}
		else
		{
			s_GCPERF_EnableTimer = x_GetTime();
		}
    }
	else
	{
		s_GCPERF_EnableTimer = -1;
	}

    if( !s_GCPERF_Enabled )
        return;

    // wait until at LEAST ONE FRAME has completed, otherwise there won't be any
    // display list data to render the perf graph
    if( s_GCPERF_Count > 1 )
    {
        // Render the perf graph data
        PERFPreDraw();

        if( s_GCPERF_Count & 0x1 )
            GXCallDisplayList( s_GCPERF_DListA, s_GCPERF_DLSize );
        else
            GXCallDisplayList( s_GCPERF_DListB, s_GCPERF_DLSize );

        PERFPostDraw();
    }

    GCPERF_EventEnd( GCPERF_EVENT_APP_TIME );
}

//==========================================================================

static void GCPERF_StartSample( void )
{
    if( !s_GCPERF_Enabled )
        return;

    // end sampling for this frame
    PERFEndFrame();
    PERFStopAutoSampling();

    s_GCPERF_Count++;

    // draw graph for next frame's DL
    if( s_GCPERF_Count & 0x1 )
        GXBeginDisplayList( s_GCPERF_DListA, GCPERF_DLIST_SIZE );
    else
        GXBeginDisplayList( s_GCPERF_DListB, GCPERF_DLIST_SIZE );

    PERFDumpScreen();

    s_GCPERF_DLSize = GXEndDisplayList();

    // begin sampling again
    PERFStartFrame();
    PERFStartAutoSampling( GCPERF_AUTOSAMPLE_RATE );

    GCPERF_EventStart( GCPERF_EVENT_APP_TIME );
}

//==========================================================================

void GCPERF_EventStart( s32 EventID )
{
    if( EventID < 0 || EventID >= GCPERF_EVENT_COUNT )
        return;

    if( !s_GCPERF_Enabled )
        return;

    PERFEventStart( EventID );
}

//==========================================================================

void GCPERF_EventEnd( s32 EventID )
{
    if( EventID < 0 || EventID >= GCPERF_EVENT_COUNT )
        return;

    if( !s_GCPERF_Enabled )
        return;

    PERFEventEnd( EventID );
}

//==========================================================================

#endif // defined( ENABLE_GC_PERF_LIB )