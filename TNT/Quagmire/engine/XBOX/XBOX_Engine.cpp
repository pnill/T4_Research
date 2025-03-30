//=====================================================================================================================================
//
//  XBOX_Engine.CPP
//
//=====================================================================================================================================
#include "x_files.hpp"

#include "Q_Engine.hpp"
#include "Q_XBOX.hpp"
#include "Q_SMEM.hpp"
#include "Q_View.hpp"
#include "Q_Draw.hpp"

#include "E_SMem.hpp"
#include "E_AccumTimer.hpp"
#include "E_Text.hpp"
#include "Q_FileReadQueue.h"

#include "XBOX_Input.hpp"
#include "XBOX_Font.hpp"
#include "XBOX_Video.hpp"
#include "XBOX_Draw.hpp"
#include "XBOX_Dev.hpp"
#include "XBOX_MemoryUnit.hpp"
#include "XBOX_GeomShaderDefines.h"

//-- Define this if you need the xbox to auto clear the target buffer.  This is not 
//-- recommended however, cause xbox GPU costs for fills are significant.
//#define XBOX_AUTO_CLEAR_TARGET

color eng_fog_color(0,0,0,0);
eng_fog g_EngineFog={0,0,0,0};
static xbool s_bSepiaToonCheatActive = FALSE;


//=====================================================================================================================================
// Functions that are to be defined written by the application using the Quagmire Engine.
xbool QuagInit( void );
s32   QuagMain( s32 argc, char* argv[] );

//#define ALLOW_MANUAL_INSTANT_REPLAY_CONTROL

// DEFINES
#define MAX_STENCIL_PASSES      4
#define DEFAULT_SHADOW_BIAS     16  // 16 is the max supported on the Xbox  (valid range is 0-16)
#if defined( X_DEBUG )
#ifndef ALLOW_MANUAL_INSTANT_REPLAY_CONTROL
    //#define SHOW_SAFE_BORDER
#endif
#endif

//#define FORCE_RUN_FROM_DVD
#ifdef FORCE_RUN_FROM_DVD
	void ForceRunFromDvd(void);
#endif //FORCE_RUN_FROM_DVD

// =====================================================================================================================================
// Globals
// Engine status
static xbool            s_EngActivated = FALSE;
static xbool            s_bZBuffer;                 // Value to indicate that a ZBUFFER was created at initialization.
static xbool            s_InsideRenderMode;
static xbool            s_InsideScene;
static xbool            s_bClearBuffers = TRUE;
static color            s_BackgroundColor(0,0,0);
static s32              s_SMemSize  = -1;

// Engine Timers and Counters
static f64              s_StartRunTime;
static accum_timer      s_FPSTimer;
static accum_timer      s_CPUTimer;
static u32              s_Frame;


// Engine matricies
static matrix4          s_MWorldToScreen;

static D3DVIEWPORT8     s_Viewport;
static view             s_View;
static s32              s_XRes;
static s32              s_YRes;
static s32              s_2DClipL;
static s32              s_2DClipT;
static s32              s_2DClipR;
static s32              s_2DClipB;

// Render and Blending flags
static u32              s_RenderFlags   = ENG_ZBUFFER_TEST_ON | ENG_ZBUFFER_FILL_ON | ENG_ALPHA_TEST_OFF | ENG_ALPHA_BLEND_OFF;
static u32              s_BlendFlags    = ENG_BLEND_NORMAL;
static u8               s_FixedAlpha    = 128;

static lighting         s_ActiveLighting[ ENG_LIGHTING_MAX_LEVELS ];
static matrix4          s_LightingDir[ ENG_LIGHTING_MAX_LEVELS ];
static matrix4          s_LightingColor[ ENG_LIGHTING_MAX_LEVELS ];

static D3DLIGHT8        s_Light0;
static D3DLIGHT8        s_Light1;
static D3DLIGHT8        s_Light2;

static s32              s_ShadowBias;

static s8               s_StencilRefQueue[MAX_STENCIL_PASSES] =
{
    20, 40, 60, 80,
};
static s32               s_StencilRefQueueCounts[MAX_STENCIL_PASSES];

// Stencil Buffer Overlay vertex data
struct STENCILVERTEX
{
    D3DXVECTOR4 p;
    D3DCOLOR    color;
};
#define D3DFVF_STENCILVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)

//=========================================================================
// FUNCTIONS
//=========================================================================

void ENG_SetBlendMode( u32 BlendFlags, u8 FixedAlpha )
{
    /**
	ASSERT( ENG_GetRenderMode() );
    ASSERT( BlendFlags );

    // Make sure that there is a difference in the current states and the new states to authorize a change.
    if( (BlendFlags == s_BlendFlags) && (FixedAlpha == s_FixedAlpha) )
        return;


    LPDIRECT3DDEVICE8 pD3DDevice = XBOX_GetDevice();

    // If there is a request to set a fixed alpha, set it.
    if( BlendFlags & ENG_BLEND_FIXED_ALPHA )
    {
        DXWARN(pD3DDevice->SetRenderState( D3DRS_TEXTUREFACTOR , D3DCOLOR_ARGB( FixedAlpha, 255, 255, 255 ) ));
    }
//    else
//    {
//        DXWARN(pD3DDevice->SetRenderState( D3DRS_TEXTUREFACTOR , D3DCOLOR_ARGB( 255, 255, 255, 255 ) ));
//    }

    // Save the blend flags and fixed alpha flags.
    s_BlendFlags = BlendFlags;

    static s32 s_BlendOp = -1;
    static s32 s_BlendArg0 = -1;
    static s32 s_BlendArg1 = -1;

    s32 BlendOp;
    s32 BlendArg0;
    s32 BlendArg1;

    //--- set the blending equation
    switch( BlendFlags & 0x0F )
    {
        case ENG_BLEND_NORMAL:
            // (SC * SA) + (DC * (1 - SA))
            BlendOp = D3DBLENDOP_ADD;
            BlendArg0 = D3DBLEND_SRCALPHA;
            BlendArg1 = D3DBLEND_INVSRCALPHA;
            break;

        case ENG_BLEND_ADDITIVE:
            // (SC * SA) + (DC)
            BlendOp = D3DBLENDOP_ADD;
            BlendArg0 = D3DBLEND_SRCALPHA;
            BlendArg1 = D3DBLEND_ONE;
            break;

        case ENG_BLEND_SUBTRACTIVE:
            // (DC * DA)) - (SC * 1-DA)
            BlendOp = D3DBLENDOP_REVSUBTRACT;
            BlendArg0 = D3DBLEND_SRCALPHA;
            BlendArg1 = D3DBLEND_ONE;//D3DBLEND_INVSRCALPHA ));
            break;

        case ENG_BLEND_DEST:
            // (DC * 1)
            BlendOp = D3DBLENDOP_ADD;
            BlendArg0 = D3DBLEND_ZERO;
            BlendArg1 = D3DBLEND_DESTALPHA;
            break;

        case ENG_BLEND_MULTIPLY:
            BlendOp = D3DBLENDOP_ADD;
            BlendArg0 = D3DBLEND_ONE;
            BlendArg1 = D3DBLEND_ONE;
            break;

        case ENG_BLEND_STENCIL:
            BlendOp = D3DBLENDOP_ADD;
            BlendArg0 = D3DBLEND_ZERO;
            BlendArg1 = D3DBLEND_ONE;
            break;

        default:
            BlendOp = D3DBLENDOP_ADD;
            BlendArg0 = D3DBLEND_SRCALPHA;
            BlendArg1 = D3DBLEND_INVSRCALPHA;
            ASSERT( FALSE );
            break;
    }

    if ( s_BlendOp != BlendOp ) DXWARN(pD3DDevice->SetRenderState( D3DRS_BLENDOP,   BlendOp ));
    if ( s_BlendArg0 != BlendArg0 ) DXWARN(pD3DDevice->SetRenderState( D3DRS_SRCBLEND,  BlendArg0 ));
    if ( s_BlendArg1 != BlendArg1 ) DXWARN(pD3DDevice->SetRenderState( D3DRS_DESTBLEND, BlendArg1 ));

    s_BlendOp = BlendOp;
    s_BlendArg0 = BlendArg0;
    s_BlendArg1 = BlendArg1;

    /**/
    ASSERT( BlendFlags );

    // Make sure that there is a difference in the current states and the new states to authorize a change.
    if( (BlendFlags == s_BlendFlags) && (FixedAlpha == s_FixedAlpha) )
        return;

    // Save the blend flags and fixed alpha flags.
    s_BlendFlags = BlendFlags;
    s_FixedAlpha = FixedAlpha;

    LPDIRECT3DDEVICE8 pD3DDevice = XBOX_GetDevice();

    // If there is a request to set a fixed alpha, set it.
    if( BlendFlags & ENG_BLEND_FIXED_ALPHA )
    {
        DXWARN(pD3DDevice->SetRenderState( D3DRS_TEXTUREFACTOR , D3DCOLOR_ARGB( FixedAlpha, 255, 255, 255 ) ));
    }
//    else
//    {
//        DXWARN(pD3DDevice->SetRenderState( D3DRS_TEXTUREFACTOR , D3DCOLOR_ARGB( 255, 255, 255, 255 ) ));
//    }

    //--- set the blending equation
    switch( BlendFlags & 0x0F )
    {
        case ENG_BLEND_NORMAL:
            // (SC * SA) + (DC * (1 - SA))
            DXWARN(pD3DDevice->SetRenderState( D3DRS_BLENDOP,   D3DBLENDOP_ADD ));
            DXWARN(pD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA ));
            DXWARN(pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA ));
            break;

        case ENG_BLEND_ADDITIVE:
            // (SC * SA) + (DC)
            DXWARN(pD3DDevice->SetRenderState( D3DRS_BLENDOP,   D3DBLENDOP_ADD ));
            DXWARN(pD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA ));
            DXWARN(pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE ));
            break;

        case ENG_BLEND_SUBTRACTIVE:
            // (DC * DA)) - (SC * 1-DA)
            DXWARN(pD3DDevice->SetRenderState( D3DRS_BLENDOP,   D3DBLENDOP_REVSUBTRACT ));
            DXWARN(pD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA ));
            DXWARN(pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA ));
            break;

        case ENG_BLEND_DEST:
            // (DC * 1)
            DXWARN(pD3DDevice->SetRenderState( D3DRS_BLENDOP,   D3DBLENDOP_ADD ));
            DXWARN(pD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ZERO ));
            DXWARN(pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_DESTALPHA ));
            break;

        case ENG_BLEND_MULTIPLY:
            DXWARN(pD3DDevice->SetRenderState( D3DRS_BLENDOP,   D3DBLENDOP_ADD ));
            DXWARN(pD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ONE ));
            DXWARN(pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE ));
            break;

        case ENG_BLEND_STENCIL:
            //DXWARN( pD3DDevice->SetRenderState( D3DRS_BLENDOP,      D3DBLENDOP_ADD ) );
            DXWARN( pD3DDevice->SetRenderState( D3DRS_SRCBLEND,     D3DBLEND_ZERO ) );
            DXWARN( pD3DDevice->SetRenderState( D3DRS_DESTBLEND,    D3DBLEND_ONE ) );
            break;

        default:
            ASSERT( FALSE );
            break;
    }
/**/
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
    /**/
	ASSERT( ENG_GetRenderMode() );

    // Test to see if the new render state is different from the current render state
    // If it is, then set to the new values.
    if( s_RenderFlags != RenderFlags )
    {
        static u32   s_ZTestMode        = 0xffffffff;
        static u32   s_ZFillMode        = 0xffffffff; 
        static s32   s_AlphaTestMode    = -1;
        static s32   s_AlphaBlendMode   = -1;

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
        u32 ZFillMode      = (RenderFlags & ENG_ZBUFFER_FILL_ON ) ? D3DZB_TRUE       : D3DZB_FALSE;
        u32 ZTestMode      = (RenderFlags & ENG_ZBUFFER_TEST_ON)  ? D3DCMP_LESSEQUAL : D3DCMP_ALWAYS;
        s32 AlphaBlendMode = (RenderFlags & ENG_ALPHA_BLEND_ON )  ? TRUE             : FALSE;
        s32 AlphaTestMode  = (RenderFlags & ENG_ALPHA_TEST_ON )   ? TRUE             : FALSE;

        //TODO:: Alpha Test should be on for alpha blending when alpha is zero
        //s32 AlphaTestMode  = (RenderFlags & ENG_ALPHA_TEST_ON )   ? TRUE             : AlphaBlendMode;

        LPDIRECT3DDEVICE8 pD3DDevice = XBOX_GetDevice();

        // Set those modes.
        if (  s_ZTestMode != ZTestMode )            DXWARN(pD3DDevice->SetRenderState( D3DRS_ZFUNC,            ZTestMode ));
        if (  s_ZFillMode != ZFillMode )            DXWARN(pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE,     ZFillMode ));  
        if (  s_AlphaTestMode != AlphaTestMode )    DXWARN(pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  AlphaTestMode ));
        if (  s_AlphaBlendMode != AlphaBlendMode )  DXWARN(pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, AlphaBlendMode ));

        s_ZTestMode         = ZTestMode;
        s_ZFillMode         = ZFillMode; 
        s_AlphaTestMode     = AlphaTestMode;
        s_AlphaBlendMode    = AlphaBlendMode;

        if ( !AlphaBlendMode ) ENG_SetBlendMode( ENG_BLEND_NORMAL, 0 );
    }
}

//=====================================================================================================================================
void ENG_GetRenderFlags( u32& RenderFlags )
{
    RenderFlags = s_RenderFlags;
}

//=====================================================================================================================================

void XBOX_SetStencilMode( s32 StencilMode, u8 StencilRef, xbool bFind )
{
    u8 WorkingStencil = StencilRef;
    s8 i;

    ASSERT( StencilRef );

    if( StencilRef < 255 && bFind )
    {
        xbool FoundRef = FALSE;
        for( i = 0; i < MAX_STENCIL_PASSES; i++ )
        {
            if (StencilRef <= s_StencilRefQueue[ i ])
            {
                FoundRef = TRUE;
                s_StencilRefQueueCounts[ i ] ++;
                WorkingStencil = s_StencilRefQueue[ i ];
                break;
            }
        }
        if (!FoundRef)
        {
            s_StencilRefQueueCounts[ MAX_STENCIL_PASSES - 1 ] ++;
            WorkingStencil = s_StencilRefQueue[ MAX_STENCIL_PASSES - 1 ];
        }
    }

    LPDIRECT3DDEVICE8 pD3DDevice = XBOX_GetDevice();

    switch( StencilMode )
    {
    case ENG_STENCIL_ON:
        DXWARN( pD3DDevice->SetRenderState( D3DRS_STENCILENABLE, TRUE ) );

        // Dont bother with interpolating color
        DXWARN( pD3DDevice->SetRenderState( D3DRS_SHADEMODE,     D3DSHADE_FLAT ) );

        // Set up stencil compare fuction, reference value, and masks.
        // Stencil test passes if ((ref & mask) cmpfn (stencil & mask)) is true.
        // Note: since we set up the stencil-test to always pass, the STENCILFAIL
        // renderstate is really not needed.
        DXWARN( pD3DDevice->SetRenderState( D3DRS_STENCILFUNC,   D3DCMP_ALWAYS ) );
        //DXWARN( pD3DDevice->SetRenderState( D3DRS_STENCILZFAIL,  D3DSTENCILOP_KEEP ) );
        //DXWARN( pD3DDevice->SetRenderState( D3DRS_STENCILFAIL,   D3DSTENCILOP_KEEP ) );

        // If ztest passes, inc/decrement stencil buffer value
        DXWARN( pD3DDevice->SetRenderState( D3DRS_STENCILREF,       WorkingStencil ) );
        //DXWARN( pD3DDevice->SetRenderState( D3DRS_STENCILMASK,      0xffffffff ) );
        //DXWARN( pD3DDevice->SetRenderState( D3DRS_STENCILWRITEMASK, 0xffffffff ) );
        DXWARN( pD3DDevice->SetRenderState( D3DRS_STENCILPASS,      D3DSTENCILOP_REPLACE ) );

        // Make sure that no pixels get drawn to the frame buffer
//        DXWARN( pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE ) );
//        DXWARN( pD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ZERO ) );
//        DXWARN( pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE ) );

        break;
    case ENG_STENCIL_REV:
        DXWARN( pD3DDevice->SetRenderState( D3DRS_STENCILENABLE, TRUE ) );

        // Dont bother with interpolating color
        DXWARN( pD3DDevice->SetRenderState( D3DRS_SHADEMODE,     D3DSHADE_FLAT ) );

        // Set up stencil compare fuction, reference value, and masks.
        // Stencil test passes if ((ref & mask) cmpfn (stencil & mask)) is true.
        // Note: since we set up the stencil-test to always pass, the STENCILFAIL
        // renderstate is really not needed.
        DXWARN( pD3DDevice->SetRenderState( D3DRS_STENCILFUNC,   D3DCMP_ALWAYS ) );
        DXWARN( pD3DDevice->SetRenderState( D3DRS_STENCILZFAIL,  D3DSTENCILOP_KEEP ) );
        DXWARN( pD3DDevice->SetRenderState( D3DRS_STENCILFAIL,   D3DSTENCILOP_KEEP ) );

        // If ztest passes, inc/decrement stencil buffer value
        DXWARN( pD3DDevice->SetRenderState( D3DRS_STENCILREF,       WorkingStencil ) );
        DXWARN( pD3DDevice->SetRenderState( D3DRS_STENCILMASK,      0xffffffff ) );
        DXWARN( pD3DDevice->SetRenderState( D3DRS_STENCILWRITEMASK, 0xffffffff ) );
        DXWARN( pD3DDevice->SetRenderState( D3DRS_STENCILPASS,      D3DSTENCILOP_REPLACE ) );

        // Make sure that no pixels get drawn to the frame buffer
//        DXWARN( pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE ) );
//        DXWARN( pD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ZERO ) );
//        DXWARN( pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE ) );

        break;
    case ENG_STENCIL_OFF:
        // Restore render states
        DXWARN( pD3DDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD ) );
        DXWARN( pD3DDevice->SetRenderState( D3DRS_STENCILENABLE,    FALSE ) );

        break;
    case ENG_STENCIL_CLEAR:
        x_memset( &s_StencilRefQueueCounts, 0, sizeof(s32) * MAX_STENCIL_PASSES );        
        DXWARN( pD3DDevice->Clear( 0, NULL, D3DCLEAR_STENCIL, 0, 0.0f, 0L ) );

        break;
    default:
        ASSERTS( FALSE, "Unknown Stencil Mode!" );
        break;
    }
}

//=====================================================================================================================================
//static LPDIRECT3DVERTEXBUFFER8 s_pStencilVertices = NULL;
//static WORD s_pStencilIndices[4] = {0,1,2,3};
//u32 s_StencilVertexStride;

void XBOX_InitStencilOverlay( void )
{
    /*
    // building an indexed buffer for use with the stencils/shadows
    s_StencilVertexStride = sizeof( STENCILVERTEX );
   // DXWARN( XBOX_GetDevice()->CreateVertexBuffer( 4 * s_StencilVertexStride, 0, 0, 0, &s_pStencilVertices ) );

    STENCILVERTEX *pVtx;
    // Lock the vertex buffer
    //DXWARN( s_pStencilVertices->Lock( 0, 0, (BYTE**)&pVtx, 0 ) );

    pVtx[ 0 ].p = D3DXVECTOR4(   0.0f, 480.0f, 0.0f, 1.0f );
    pVtx[ 1 ].p = D3DXVECTOR4(   0.0f,   0.0f, 0.0f, 1.0f );
    pVtx[ 2 ].p = D3DXVECTOR4( 640.0f, 480.0f, 0.0f, 1.0f );
    pVtx[ 3 ].p = D3DXVECTOR4( 640.0f,   0.0f, 0.0f, 1.0f );
    */
}

//=====================================================================================================================================

void XBOX_KillStencilOverlay( void )
{
    /*
    s_pStencilVertices->Release();
    s_pStencilVertices = NULL;
    */
}

//=====================================================================================================================================
static DWORD zHack = 16;
static u32 blendStuff = ENG_BLEND_ADDITIVE | ENG_BLEND_FIXED_ALPHA;
static u8 fixedAlpha = 255;

void XBOX_DrawStencilOverlay( u8 StencilRef )
{
    LPDIRECT3DDEVICE8 pD3DDevice = XBOX_GetDevice();

    u32 CurrentRenderFlags;
    u32 NewRenderFlags;

    ENG_GetRenderFlags( CurrentRenderFlags );

    // Set renderstates (disable z-buffering, enable stencil, disable fog, and
    // turn on alphablending)
    NewRenderFlags = CurrentRenderFlags;
    NewRenderFlags = (ENG_ZBUFFER_FILL_OFF | ENG_ALPHA_BLEND_ON | ENG_ALPHA_TEST_ON | ENG_ZBUFFER_TEST_ON);
    XBOX_SetStencilMode( ENG_STENCIL_ON, StencilRef, FALSE );
    ENG_SetRenderFlags( NewRenderFlags );
    ENG_SetBlendMode( ENG_BLEND_SUBTRACTIVE );

    DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ) );
    DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE ) );
    DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 ) );
    DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ) );
    DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE ) );
    DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 ) );

    DXWARN( pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE ) );
    DXWARN( pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE ) );
    DXWARN( pD3DDevice->SetTextureStageState( 2, D3DTSS_COLOROP,   D3DTOP_DISABLE ) );
    DXWARN( pD3DDevice->SetTextureStageState( 2, D3DTSS_ALPHAOP,   D3DTOP_DISABLE ) );
    DXWARN( pD3DDevice->SetTextureStageState( 3, D3DTSS_COLOROP,   D3DTOP_DISABLE ) );
    DXWARN( pD3DDevice->SetTextureStageState( 3, D3DTSS_ALPHAOP,   D3DTOP_DISABLE ) );

    // The stencilbuffer values indicates # of shadows that overlap each pixel.
    // We only want to draw pixels that are in shadow, which was set up in
    // RenderShadow() such that StencilBufferValue >= 1. In the Direct3D API, 
    // the stencil test is pseudo coded as:
    //    StencilRef CompFunc StencilBufferValue
    // so we set our renderstates with StencilRef = 1 and CompFunc = LESSEQUAL.
//    DXWARN( pD3DDevice->SetRenderState( D3DRS_STENCILENABLE,    TRUE ) );
//    DXWARN( pD3DDevice->SetRenderState( D3DRS_STENCILREF,  StencilRef ) );
    DXWARN( pD3DDevice->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_EQUAL ) );

    //DRAW_Build2DProjectionMatrix();

    ////////////////////////////////////////////////////////////////////////
    //  Calc Format and Format Size
    ////////////////////////////////////////////////////////////////////////
    
    u32 Format = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;
    u32 FormatSize = sizeof(vector4)+sizeof(color);
    u32 FormatDWORDs = (FormatSize/4);

    DWORD* pPush; 
    DWORD  dwordTotal = FormatDWORDs * 4;
    DWORD  dwordCount = 0;

    XBOX_ActivateVertexShader( D3DFVF_STENCILVERTEX );
    XBOX_DeActivatePixelShader( 0 );

    VRAM_XBOX_Deactivate( 0 );
    VRAM_XBOX_Deactivate( 1 );
    VRAM_XBOX_Deactivate( 2 );
    VRAM_XBOX_Deactivate( 3 );

    ////////////////////////////////////////////////////////////////////////
    // The "+5" is to reserve enough overhead for the encoding
    // parameters. NSections adds the overhead needed when the push
	// buffer spans multiple sections.
    ////////////////////////////////////////////////////////////////////////

	u32 NSections = ( ( dwordTotal + 2046 ) / 2047 );
    XBOX_GetDevice()->BeginPush( dwordTotal + 5 + NSections, &pPush );

    pPush[0] = D3DPUSH_ENCODE( D3DPUSH_SET_BEGIN_END, 1 );
    pPush[1] = D3DPT_TRIANGLESTRIP;
    pPush[2] = D3DPUSH_ENCODE( D3DPUSH_NOINCREMENT_FLAG | D3DPUSH_INLINE_ARRAY, dwordTotal );
    pPush += 3;

    static vector4 Pos[] = 
    {
        vector4(   0.0f, 480.0f, 0.0f, 1.0f ),
        vector4(   0.0f,   0.0f, 0.0f, 1.0f ),
        vector4( 640.0f, 480.0f, 0.0f, 1.0f ),
        vector4( 640.0f,   0.0f, 0.0f, 1.0f )
    };

    f32 fW(1.0f);

    for ( s32 i = 0; i < 4; i++)
    {
        pPush[0] = *((DWORD*) &Pos[i].X);
        pPush[1] = *((DWORD*) &Pos[i].Y);
        pPush[2] = *((DWORD*) &Pos[i].Z);
        pPush[3] = *((DWORD*) &fW );
        pPush += 4;

        u32 C = D3DCOLOR_ARGB( StencilRef, 0, 0, 0 );;
        pPush[0] = *((DWORD*) &C);
        pPush += 1;   
    }

    pPush[0] = D3DPUSH_ENCODE( D3DPUSH_SET_BEGIN_END, 1 );
    pPush[1] = 0;
    pPush += 2;

    XBOX_GetDevice()->EndPush( pPush );

    /*
    // Draw a big, gray square
    STENCILVERTEX *pVtx;
    // Lock the vertex buffer
    DXWARN( s_pStencilVertices->Lock( 0, 0, (BYTE**)&pVtx, 0 ) );
    // build a temporary color
    s32 ShadowColor = D3DCOLOR_ARGB( StencilRef, 0, 0, 0 );
    // stuff it into all of the verts
    pVtx[ 0 ].color = ShadowColor;
    pVtx[ 1 ].color = ShadowColor;
    pVtx[ 2 ].color = ShadowColor;
    pVtx[ 3 ].color = ShadowColor;

    VRAM_XBOX_Deactivate( 0 );
    VRAM_XBOX_Deactivate( 1 );
    VRAM_XBOX_Deactivate( 2 );
    VRAM_XBOX_Deactivate( 3 );

    DXWARN( pD3DDevice->SetStreamSource( 0, s_pStencilVertices, s_StencilVertexStride ) );
    DXWARN( pD3DDevice->DrawIndexedVertices( D3DPT_TRIANGLESTRIP, 4, s_pStencilIndices ) );
*/

    // Restore render states
    ENG_SetRenderFlags( CurrentRenderFlags );
//    DXWARN( pD3DDevice->SetRenderState( D3DRS_STENCILENABLE,    FALSE ) );
    XBOX_SetStencilMode( ENG_STENCIL_OFF );
}

//==========================================================================

void XBOX_DrawStencilOverlayAllRefs( void )
{
    s32 i;

    // Set the default alpha test value.
    DXWARN( XBOX_GetDevice()->SetRenderState( D3DRS_ALPHAREF, 4 ) );

    for( i = 0; i < MAX_STENCIL_PASSES; ++i )
    {
        if( s_StencilRefQueueCounts[i] != 0 )
        {
            XBOX_DrawStencilOverlay( s_StencilRefQueue[i] );
        }
    }
    //-- we dont need to clear.. since clear happens at beginning of the frame and
    //this happens at the end of the frame
    //XBOX_SetStencilMode( ENG_STENCIL_CLEAR );

    // Set the default alpha test value.
    DXWARN( XBOX_GetDevice()->SetRenderState( D3DRS_ALPHAREF, 128 ) );
}

//=====================================================================================================================================
void XBOX_NextShadowBiasLevel( void )
{
    s_ShadowBias--;
}

//=====================================================================================================================================
void XBOX_SetShadowBiasMode( s32 Mode, u8 ShadowBias )
{
    switch( Mode )
    {
    case ENG_SHADOW_BIAS_ON:
        if( ShadowBias < 255 )
        {
            DXWARN( XBOX_GetDevice()->SetRenderState( D3DRS_ZBIAS, ShadowBias ) );
        }
        else
        {
            DXWARN( XBOX_GetDevice()->SetRenderState( D3DRS_ZBIAS, s_ShadowBias ) );
        }

        break;
    case ENG_SHADOW_BIAS_OFF:
        DXWARN( XBOX_GetDevice()->SetRenderState( D3DRS_ZBIAS, 0 ) );
        break;
    case ENG_SHADOW_BIAS_RESET:
        s_ShadowBias = DEFAULT_SHADOW_BIAS;
        break;
    default:
        ASSERTS( FALSE, "Unknown Shadow Bias Mode!" );
        break;
    }
}

//=====================================================================================================================================
void ENG_SetBackgroundColor( color C )
{
    s_BackgroundColor = C;
}

//=====================================================================================================================================
xbool ENG_Activate( s32 XRes, s32 YRes, xbool ZBuffer, s32 SMemSize, char* ApplicationName )
{
    ASSERT( !s_EngActivated );
    ASSERT( SMemSize );

    // Set the parameters for the window, and the rest of the render environment.
    s_XRes = XRes;
    s_YRes = YRes;
    s_View.SetViewport( 0, 0, XRes, YRes );

    s_bZBuffer = ZBuffer;
    x_memset( &s_StencilRefQueueCounts, 0, sizeof(u8) * MAX_STENCIL_PASSES );        

#ifdef FORCE_RUN_FROM_DVD
	{
		x_printf("ENGINE: Mounting DVD.\n");
		ForceRunFromDvd();
	}
#endif	//FORCE_RUN_FROM_DVD

    // Initializing the XBOX File system.
    x_printf("ENGINE: Initializing filesystem.\n");
    XBOX_DVD_InitModule();

    // Setup XBOX Render environment.
    x_printf("---------------------------------------\n");
    x_printf("ENGINE: Initializing render environment.\n");
    VERIFY(VIDEO_InitModule( s_bZBuffer ));

    // Initialize input module.
    // *** THIS MUST COME BEFORE the Memory Unit Support system is initialized ***
    x_printf("ENGINE: Initializing controllers, and allocating system resourced for memory unit support.\n");
    VERIFY(INPUT_InitModule());

    // Initialize the Memory Unit / Card support for the XBOX.
    // *** THIS MUST COME After the input system is initialized ***
    x_printf("ENGINE: Initializing Memory Unit Support.\n");
    MEMORY_UNIT_Create( ApplicationName );

    // Initalize the VRAM manager.
    x_printf("ENGINE: Initializing vram manager.\n");
    VRAM_Init();

    // Initialize the basic draw layer.
    x_printf("ENGINE: Initializing draw.\n");
    VERIFY(DRAW_InitModule());

    // Initialize scratch memory.
    x_printf("ENGINE: Initializing scratch memory.\n");
    s_SMemSize = SMemSize;
    SMEM_InitModule(s_SMemSize);

    // Initialize the FONT drawing module
    x_printf("ENGINE: Initializing font.\n");
    VERIFY(FONT_InitModule());

    // Initialize text handling routines.
    x_printf("ENGINE: Initializing text manager.\n");
    TEXT_InitModule( s_XRes, s_YRes, 16, 16, 16, 16, FONT_RenderText );

    //--- Initialize debug module
    x_printf( "ENGINE: Initializing debug module.\n" );
    extern void DEBUG_InitModule( void );
    DEBUG_InitModule();

    // Initialize the default lighting.
    x_printf("ENGINE: Initializing default lighting.\n");
    ENG_SetLight(0, vector3(1.0f, 0.6f, 0.2f), color(255,255,255,255));

    // Mark engine as activated and return
    x_printf("ENGINE: Initialized.\n");
    x_printf("---------------------------------------\n");
    s_EngActivated = TRUE;

    // Set the 2D clip area.
    ENG_Set2DClipArea( 0, 0, XRes-1, YRes-1 );
    s_InsideRenderMode = FALSE;
    s_InsideScene = FALSE;


    // Initialize timers and counters.
    s_FPSTimer.Clear();
    s_CPUTimer.Clear();
    s_Frame = 0;
    s_StartRunTime = x_GetTime();


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

    x_printf("[Memory Unit Support]\n");
    MEMORY_UNIT_Destroy( );

    x_printf("[vram]\n");
    VRAM_Kill();

    x_printf("[draw]\n");
    DRAW_KillModule();

    x_printf("[font]\n");
    FONT_KillModule();

    x_printf("[video]\n");
    VIDEO_KillModule();

    x_printf("[filesystem]\n");
    XBOX_DVD_KillModule();


    x_printf("[win32]\n");

    s_EngActivated = FALSE;    

    x_printf("ENGINE: Closed.\n");
}

//=====================================================================================================================================
static s32 test = 3;
static s32 test2 = 1;
void ENG_PageFlip( xbool ClearNewScreen )
{
    // Render the text into current dlist
    ENG_BeginRenderMode();

    FONT_BeginRender();
    TEXT_RenderAllText();
    FONT_EndRender();

//	XBOX_DrawStencilOverlayAllRefs();

#if defined( SHOW_SAFE_BORDER )
    DRAW_SetMode( DRAW_2D | DRAW_NO_FILL | DRAW_NO_TEXTURE | DRAW_NO_ZBUFFER | DRAW_NO_ALPHA | DRAW_CLIP | DRAW_NO_LIGHT );
    color BoxColor( 255, 255, 0, 255 );
    DRAW_Rectangle( 47.0f, 35.0f, 0.0f, 544, 408, BoxColor, BoxColor, BoxColor, BoxColor );
    DRAW_SetMode( DRAW_3D | DRAW_FILL | DRAW_NO_TEXTURE | DRAW_ZBUFFER | DRAW_NO_ALPHA | DRAW_CLIP | DRAW_NO_LIGHT );
#endif

    ENG_EndRenderMode();

//    ENG_PrintStats( );

    // End timing of CPU
    s_CPUTimer.EndTiming();

    // Change Draw/Disp buffers 
    // End the scene
    ASSERT(s_InsideScene);
    DXWARN(XBOX_GetDevice()->EndScene());
    s_InsideScene = FALSE;
    
    // Present the backbuffer contents to the front buffer
    DXWARN(XBOX_GetDevice()->Present(NULL, NULL, NULL, NULL));
    XBOX_GetDevice()->SetFlickerFilter( test );
    XBOX_GetDevice()->SetSoftDisplayFilter( test2 );
//    test = !test;

    XBOX_SetShadowBiasMode( ENG_SHADOW_BIAS_RESET );

    INPUT_CheckDevices();

    // Pulse the memcard system
    MEMORY_UNIT_Update();

   // Update FPS timer
    s_FPSTimer.EndTiming();
    s_FPSTimer.StartTiming();

    // Increment to next frame
    s_Frame++;
    
    // Toggle scratch buffers and text buffers
    SMEM_Toggle();
    TEXT_SwitchBuffers();

    // Update any File Reads from the FileReadQueue/
    ENG_UpdateFileReadQueue( );

    // Pump the direct sound streams. (This was required when we added the XBOX soundtrack support).
    DirectSoundDoWork( );

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
    f32         ZNear, ZFar;

    // Copy view into active view and get ingredients
    x_memcpy( &s_View, &View, sizeof(view));

    // Get viewport bounds and other info
    s_View.GetViewport( Left, Top, Right, Bottom );

	if( Left < 0 )		Left   = 0;
	if( Top  < 0 )		Top    = 0;
	if( Right  < Left )	Right  = Left + 1;
	if( Bottom < Top )	Bottom = Top + 1;

    s_View.GetZLimits( ZNear, ZFar );
    s_View.GetXFOV( Xfov );
    Yfov = 2.0f * x_atan( (Bottom-Top+1) * x_tan(Xfov*0.5f) / (Right-Left+1) );
    
    // Set the world to view matrix as identity.
    s_View.GetW2VMatrix ( WorldToView );
    XBOX_GetDevice()->SetTransform( D3DTS_VIEW, (D3DXMATRIX*)&WorldToView.M );

    // Set the viewport clipping bounds.
    s_Viewport.X      = Left;
    s_Viewport.Y      = Top;
    s_Viewport.Width  = MIN( (Right - Left), s_XRes - Left - 1 );
    if( s_Viewport.Width <= 0 ) 
    {
		if( s_Viewport.X > 0 )
			s_Viewport.X -= 1;

        s_Viewport.Width = 1;
    }
    s_Viewport.Height = MIN( (Bottom - Top), s_YRes - Top - 1 );
    if( s_Viewport.Height <= 0 ) 
    {
		if( s_Viewport.Y > 0 )
			s_Viewport.Y -= 1;

        s_Viewport.Height = 1;
    }

    s_Viewport.MinZ   = 0.0f;
    s_Viewport.MaxZ   = 1.0f;
    DXWARN(XBOX_GetDevice()->SetViewport(&s_Viewport));

    // Build and set the perspective projection matrix.
    f32 AspectRatio = ((f32)Right-(f32)Left+1.0f)/((f32)Bottom-(f32)Top+1.0f);
    D3DXMatrixPerspectiveFovLH( &D3DProjectionMatrix, Yfov, AspectRatio, ZNear, ZFar );
    D3DProjectionMatrix._11 = -D3DProjectionMatrix._11;     // Scale inverse along the Y axis.
    DXWARN(XBOX_GetDevice()->SetTransform( D3DTS_PROJECTION, &D3DProjectionMatrix ));
}

//=====================================================================================================================================
D3DXMATRIX XBOX_GetProjectionMatrix( void )
{
    D3DXMATRIX  D3DProjectionMatrix;
    radian      Xfov,Yfov;
    s32         Left, Top, Right, Bottom;
    f32         AspectRatio;
    f32         ZNear, ZFar;

    // Get viewport bounds and other info
    s_View.GetAspectRatio( AspectRatio );
    s_View.GetViewport   ( Left, Top, Right, Bottom );
    s_View.GetZLimits    ( ZNear, ZFar );
    s_View.GetXFOV       ( Xfov );
    Yfov = 2.1f * x_atan( x_tan( Xfov / 2.0f ) / AspectRatio );
    
    // Build and set the perspective projection matrix.
    D3DXMatrixPerspectiveFovLH( &D3DProjectionMatrix, Yfov, AspectRatio, ZNear, ZFar );
    D3DProjectionMatrix._11 = -D3DProjectionMatrix._11;     // Scale inverse along the Y axis.

    return D3DProjectionMatrix;
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
void ENG_SetActiveLighting( const lighting& Lighting, s32 iSet )
{

    s32 i;

    if (iSet >= ENG_LIGHTING_MAX_LEVELS)
        iSet = ENG_LIGHTING_MAX_LEVELS - 1;
    else if (iSet < 0)
        iSet = 0;

    // Copy lighting for engine use
    x_memcpy( &s_ActiveLighting[ iSet ], &Lighting, sizeof(lighting) );

    // Setup lighting direction matrix.  If you push an xyz1 normal vector 
    // through this matrix you will get the 3 intensities for the three
    // directional lights in xyz and a 1.0 in the w for ambient
    s_LightingDir[ iSet ].Zero();
    s_LightingColor[ iSet ].Zero();
    for( i=0; i<3;  i++ )
    {
        if( s_ActiveLighting[ iSet ].Directional[i].Active )
        {
            // Setup lighting direction for this light
            vector3* pD;
            pD = &s_ActiveLighting[ iSet ].Directional[i].Direction;
            pD->Normalize();
            s_LightingDir[ iSet ].M[0][i] = pD->X;
            s_LightingDir[ iSet ].M[1][i] = pD->Y;
            s_LightingDir[ iSet ].M[2][i] = pD->Z;

            // Setup lighting color for this light
            color* pC;
            pC = &s_ActiveLighting[ iSet ].Directional[i].Color;
            s_LightingColor[ iSet ].M[i][0] = (f32)pC->R;
            s_LightingColor[ iSet ].M[i][1] = (f32)pC->G;
            s_LightingColor[ iSet ].M[i][2] = (f32)pC->B;
            s_LightingColor[ iSet ].M[i][3] = (f32)pC->A;
        }
    }
    s_LightingDir[ iSet ].M[3][3] = 1.0f;
    s_LightingColor[ iSet ].M[3][0] = (f32)s_ActiveLighting[ iSet ].Ambient.R;
    s_LightingColor[ iSet ].M[3][1] = (f32)s_ActiveLighting[ iSet ].Ambient.G;
    s_LightingColor[ iSet ].M[3][2] = (f32)s_ActiveLighting[ iSet ].Ambient.B;
    s_LightingColor[ iSet ].M[3][3] = (f32)s_ActiveLighting[ iSet ].Ambient.A;


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
        if( s_ActiveLighting[ iSet ].Directional[i].Active == FALSE )
		{
	        DXWARN( XBOX_GetDevice()->LightEnable( i, FALSE ) );
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
        pC = &s_ActiveLighting[ iSet ].Directional[i].Color;
        pLight->Diffuse.r = (f32)pC->R / 255.0f;
        pLight->Diffuse.g = (f32)pC->G / 255.0f;
        pLight->Diffuse.b = (f32)pC->B / 255.0f;
        pLight->Diffuse.a = 1.0f;
        pC = &s_ActiveLighting[ iSet ].Ambient;
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
        pD = &s_ActiveLighting[ iSet ].Directional[i].Direction;
		if( pD->SquaredLength() < 0.0005f )
			pD->Z = 1.0f;
        pD->Normalize();
        pLight->Direction.x = pD->X;
        pLight->Direction.y = pD->Y;
        pLight->Direction.z = pD->Z;

        DXWARN( XBOX_GetDevice()->SetLight( i, pLight ) );
        DXWARN( XBOX_GetDevice()->LightEnable( i, TRUE ) );
    }

    DWORD dwAmbient = D3DCOLOR_RGBA( s_ActiveLighting[ iSet ].Ambient.R,
                                     s_ActiveLighting[ iSet ].Ambient.G,
                                     s_ActiveLighting[ iSet ].Ambient.B,
                                     s_ActiveLighting[ iSet ].Ambient.A );

// Please note, in ASB we never really use the built in D3D lighting...
// instead we do all our lighting through vertex shaders and OP modes.
// so we don't turn on the internal D3D lighting engine
    DXWARN( XBOX_GetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE ) );
    DXWARN( XBOX_GetDevice()->SetRenderState( D3DRS_AMBIENT, dwAmbient ) );

}


//=====================================================================================================================================
//void ENG_GetLightDirAndColor( matrix4& LDir, matrix4& LColor )
//{
//    LDir = s_LightingDir;
//    LColor = s_LightingColor;
//}

//=====================================================================================================================================
void    ENG_GetActiveLighting( lighting& Lighting, s32 iSet )
{
    if (iSet >= ENG_LIGHTING_MAX_LEVELS)
        iSet = ENG_LIGHTING_MAX_LEVELS - 1;
    else if (iSet < 0)
        iSet = 0;
    Lighting = s_ActiveLighting[ iSet ];
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
    XBOX_GetDevice()->SetLight( LightID, &Light );

    //
    // Here we enable out register 0. That way what ever we render 
    // from now on it will use register 0. The other registers are by 
    // default turn off.
    //
    XBOX_GetDevice()->LightEnable( LightID, TRUE );
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
    XBOX_GetDevice()->SetMaterial( &mtrl );

    //
    // This function will set the ambient color. In this case white.
    // R=G=B=A=255. which is like saying 0xffffffff. Because the color
    // is describe in 32bits. One each of the bytes in those 32bits
    // describe a color component. You can also use a macro provided 
    // by d3d to build the color.
    //
    XBOX_GetDevice()->SetRenderState( D3DRS_AMBIENT, Color.Get() );
}

//==========================================================================

void ENG_SpecialCaseFog( xbool bActivateSpecial )
{
}

//==========================================================================

void    ENG_SetFogColor         ( u8 Red, u8 Green, u8 Blue )
{
    eng_fog_color.Set( Red, Green, Blue );
    DXWARN( XBOX_GetDevice()->SetRenderState( D3DRS_FOGCOLOR, D3DCOLOR_RGBA( eng_fog_color.R, eng_fog_color.G, eng_fog_color.B, eng_fog_color.A ) ) );
}

//==========================================================================

vector4 fog(0.0f,0.0f,1.0f,1.0f/30000.0f);
D3DCOLOR fog_color = 0x007f7f7f;
void ENG_SetFogParameters    ( xbool bEnable, f32 fStart, f32 fAccumDepth, f32 fMaxFog, f32 fDensity )
{
    g_EngineFog.fStart = fStart;
    g_EngineFog.fDepth = fAccumDepth;
    g_EngineFog.fMaxFog = fMaxFog;
    g_EngineFog.fDensity = fDensity;

    ENG_ActivateFog( bEnable );
}

//==========================================================================

void ENG_ActivateFog( xbool bEnable )
{
    g_EngineFog.bEnabled = bEnable;
    if (g_EngineFog.bEnabled)
    {
        // set the fog values for the shader
        vector4 fog( g_EngineFog.fStart, g_EngineFog.fDepth, g_EngineFog.fMaxFog, g_EngineFog.fDensity);

        DXWARN( XBOX_GetDevice()->SetVertexShaderConstant(VSH_CONST_FOG, &fog, 1) );
        DXWARN( XBOX_GetDevice()->SetRenderState( D3DRS_FOGENABLE, TRUE ) );
        DXWARN( XBOX_GetDevice()->SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_NONE ) );
    }
    else
    {
        DXWARN( XBOX_GetDevice()->SetRenderState( D3DRS_FOGENABLE, FALSE ) );
    }
}

//=====================================================================================================================================
void ENG_BeginRenderMode( void )
{
    ASSERT( !s_InsideRenderMode );
    s_InsideRenderMode = TRUE;

    if (s_InsideScene == FALSE)
    {
        DXWARN( XBOX_GetDevice()->BeginScene() );
        s_InsideScene = TRUE;

        if (s_bClearBuffers)
        {
            if (s_bZBuffer)
            {
                #if defined( XBOX_AUTO_CLEAR_TARGET )
                DXWARN(XBOX_GetDevice()->Clear( 0, NULL, D3DCLEAR_STENCIL | D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, s_BackgroundColor.Get(), 1.0f, 0L));
                #else
                DXWARN(XBOX_GetDevice()->Clear( 0, NULL, D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER, 0, 1.0f, 0L));
                #endif
            }
            #if defined( XBOX_AUTO_CLEAR_TARGET )
            else
            {
                DXWARN(XBOX_GetDevice()->Clear( 0, NULL, D3DCLEAR_TARGET, s_BackgroundColor.Get(), 0.0f, 0L));
            }
            #endif
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
    D3DRECT rect;
    s32     x,y,w,h;

    ASSERT( XBOX_GetDevice() != NULL );

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
    DXWARN(XBOX_GetDevice()->Clear( 1, &rect, D3DCLEAR_TARGET, C.Get(), 1.0f, 0L));
}

//=====================================================================================================================================
void ENG_FillZBufferArea( s32 Area, f32   Depth )
{
    D3DRECT rect;
    s32     x,y,w,h;

    ASSERT( XBOX_GetDevice() != NULL );

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
    DXWARN(XBOX_GetDevice()->Clear( 1, &rect, D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER, 0x00000000u, 1.0f, 0L ));
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
void ENG_GetPerfMetrics( s32& nCulled, s32& nClipped, s32& nTotal, f32& fXFOV, f32& fYFOV, f32& FPSTimer, f32& CPUTimer, f32& GFXTimer )
{
    nCulled = -1;
    nClipped = -1;
    nTotal = -1;
    fXFOV = -1.0f;
    fYFOV = -1.0f;
	FPSTimer = s_FPSTimer.GetAvgTime();
	CPUTimer = s_CPUTimer.GetAvgTime();
	GFXTimer = 0.0f; // to be implemented...
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

//-------------------------------------------------------------------------------------------------------------------------------------
#ifndef TARGET_XBOX_NO_ENTRY

// dummy printf function, prints to output window
void DEBUG_PrintString(const char* pString)
{
    OutputDebugString(pString);
}

void main( void )
{
    xbool   InitDone;

    //----------------------------------------------------------------
    // Call the application init function to allow application specific changes
    InitDone = QuagInit();

    //---   if the initialization has not been completed, init x_files now
    if( !InitDone )
        x_Init();

    x_SetPrintHandlers( (print_fnptr)DEBUG_PrintString, (print_at_fnptr)TEXT_PrintStringAt );

    QuagMain( NULL, NULL );
    x_Kill();
    return;
}
#endif

//=====================================================================================================================================
void ENGXBOX_TestAllRenderModesAndTextureStages( s32 MaxTextureStage )
{
    DWORD Value;
    s32 i;
    char DynamicString[100];

    LPDIRECT3DDEVICE8 pD3DDevice = XBOX_GetDevice();

    OutputDebugString( "\n-----------------------------------");
    OutputDebugString( "\n----     D3D RenderStates     -----");
    OutputDebugString( "\n-----------------------------------\n");

    //-------------------------------------------------------------------------------------------------------------------------------------
    OutputDebugString( "\n------ ZBUFFER SETTINGS ------");
    //-------------------------------------------------------------------------------------------------------------------------------------
    // Test the cull mode.
    //-------------------------------------------------------------------------------------------------------------------------------------
    DXWARN(pD3DDevice->GetRenderState( D3DRS_CULLMODE, &Value ));
    switch( Value )
    {
        case D3DCULL_NONE:  OutputDebugString( "\nCullMode: None"); break;
        case D3DCULL_CW:    OutputDebugString( "\nCullMode: Clockwise"); break;
        case D3DCULL_CCW:   OutputDebugString( "\nCullMode: Counter Clockwise"); break;
        default:            OutputDebugString( "\nCullMode: Unknown"); break;
    }

    // Test if the zbuffer is on.
    //-------------------------------------------------------------------------------------------------------------------------------------
    DXWARN(pD3DDevice->GetRenderState( D3DRS_ZENABLE, &Value ));
    switch( Value )
    {
        case D3DZB_FALSE: OutputDebugString( "\nZBuffer:  OFF"); break;
        case D3DZB_TRUE:  OutputDebugString( "\nZBuffer:  ON"); break;
        default:          OutputDebugString( "\nZBuffer:  State is Unknown"); break;
    }

    // Test Z Buffer Write
    //-------------------------------------------------------------------------------------------------------------------------------------
    DXWARN(pD3DDevice->GetRenderState( D3DRS_ZWRITEENABLE, &Value ));
    switch( Value )
    {
        case TRUE:  OutputDebugString( "\nZBuffer Write: ON" ); break;
        case FALSE: OutputDebugString( "\nZBuffer Write: OFF"); break;
        default:    OutputDebugString( "\nZBuffer Write: State is Unknown"); break;
    }

    // Get the Z Buffer test function type.
    //-------------------------------------------------------------------------------------------------------------------------------------
    DXWARN(pD3DDevice->GetRenderState( D3DRS_ZFUNC, &Value ));
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
    DXWARN(pD3DDevice->GetRenderState( D3DRS_ALPHABLENDENABLE, &Value ));
    switch( Value )
    {
        case TRUE:  OutputDebugString( "\nAlpha Blending: ON"); break;
        case FALSE: OutputDebugString( "\nAlpha Blending: OFF"); break;
        default:    OutputDebugString( "\nAlpha Blending State is Unknown"); break;
    }

    // Test to see if the test alpha function is activated
    //-------------------------------------------------------------------------------------------------------------------------------------
    DXWARN(pD3DDevice->GetRenderState( D3DRS_ALPHATESTENABLE, &Value ));
    switch( Value )
    {
        case TRUE:
        {
            OutputDebugString( "\nAlpha Test: ON");

            // Determine what the alpha blending test is.
            DXWARN(pD3DDevice->GetRenderState( D3DRS_ALPHAFUNC, &Value ));
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
            DXWARN(pD3DDevice->GetRenderState( D3DRS_ALPHAREF, &Value ));
            x_sprintf( DynamicString, "\nAlphaRef: %d", Value );
            OutputDebugString( DynamicString );
            break;
        }

        case FALSE: OutputDebugString( "\nAlpha Test: OFF"); break;
        default:    OutputDebugString( "\nAlpha Test: State is Unknown"); break;
    }

    // Get the last fixed alphs render state.
    //-------------------------------------------------------------------------------------------------------------------------------------
    DXWARN(pD3DDevice->GetRenderState( D3DRS_TEXTUREFACTOR , &Value ));
    x_sprintf( DynamicString, "\nFixedAlpha Color is is A:%x, R:0x%2x, G:%x, B:%x", (Value & 0xff000000) >> 24, (Value & 0x00ff0000) >> 16, (Value & 0x0000ff00) >> 8, Value & 0xff );
    OutputDebugString( DynamicString );


    //-------------------------------------------------------------------------------------------------------------------------------------
    OutputDebugString( "\n\n------ BLENDING MODE SETTINGS ------");
    //-------------------------------------------------------------------------------------------------------------------------------------
    // Test the Current Blend Operation
    //-------------------------------------------------------------------------------------------------------------------------------------
    DXWARN(pD3DDevice->GetRenderState( D3DRS_BLENDOP, &Value ));
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
    DXWARN(pD3DDevice->GetRenderState( D3DRS_SRCBLEND, &Value ));
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
//        case D3DBLEND_BOTHSRCALPHA:     OutputDebugString( "\nSourceBlend: BOTHSRCALPHA"); break;
//        case D3DBLEND_BOTHINVSRCALPHA:  OutputDebugString( "\nSourceBlend: BOTHINVSRCALPHA"); break;
        default:                        OutputDebugString( "\nSourceBlend: UNKNOWN"); break;
    }

    DXWARN(pD3DDevice->GetRenderState( D3DRS_DESTBLEND, &Value ));
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
//        case D3DBLEND_BOTHSRCALPHA:     OutputDebugString( "\nDestBlend: BOTHSRCALPHA"); break;
//        case D3DBLEND_BOTHINVSRCALPHA:  OutputDebugString( "\nDestBlend: BOTHINVSRCALPHA"); break;
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
        DXWARN(pD3DDevice->GetTextureStageState( i, D3DTSS_COLOROP, &Value ));
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
        DXWARN(pD3DDevice->GetTextureStageState( i, D3DTSS_COLORARG1, &Value ));
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
        DXWARN(pD3DDevice->GetTextureStageState( i, D3DTSS_COLORARG2, &Value ));
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
        DXWARN(pD3DDevice->GetTextureStageState( i, D3DTSS_ALPHAOP, &Value ));
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
        DXWARN(pD3DDevice->GetTextureStageState( i, D3DTSS_ALPHAARG1, &Value ));
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
        DXWARN(pD3DDevice->GetTextureStageState( i, D3DTSS_ALPHAARG2, &Value ));
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
        DXWARN(pD3DDevice->GetTextureStageState( i, D3DTSS_ADDRESSU, &Value ));
        switch( Value )
        {   
            case D3DTADDRESS_WRAP:       OutputDebugString( "\nWrapU Mode: WRAP"); break;
            case D3DTADDRESS_MIRROR:     OutputDebugString( "\nWrapU Mode: MIRROR"); break;
            case D3DTADDRESS_CLAMP:      OutputDebugString( "\nWrapU Mode: CLAMP"); break;
            case D3DTADDRESS_BORDER:     OutputDebugString( "\nWrapU Mode: BORDER"); break;
//            case D3DTADDRESS_MIRRORONCE: OutputDebugString( "\nWrapU Mode: MIRRORONCE"); break;
            default:                     OutputDebugString( "\nWrapU Mode: UNKNOWN"); break;
        }

        DXWARN(pD3DDevice->GetTextureStageState( i, D3DTSS_ADDRESSV, &Value ));
        switch( Value )
        {   
            case D3DTADDRESS_WRAP:       OutputDebugString( "\nWrapV Mode: WRAP"); break;
            case D3DTADDRESS_MIRROR:     OutputDebugString( "\nWrapV Mode: MIRROR"); break;
            case D3DTADDRESS_CLAMP:      OutputDebugString( "\nWrapV Mode: CLAMP"); break;
            case D3DTADDRESS_BORDER:     OutputDebugString( "\nWrapV Mode: BORDER"); break;
//            case D3DTADDRESS_MIRRORONCE: OutputDebugString( "\nWrapV Mode: MIRRORONCE"); break;
            default:                     OutputDebugString( "\nWrapV Mode: UNKNOWN"); break;
        }
    }    


    OutputDebugString( "\n-----------------------------------");
    OutputDebugString( "\n---  END of D3D RenderStates  -----");
    OutputDebugString( "\n-----------------------------------\n");
}

#ifdef FORCE_RUN_FROM_DVD

	typedef struct _UNICODE_STRING
	{
		USHORT	Length;
		USHORT	MaximumLength;
		PSTR	Buffer;
	} UNICODE_STRING, *PUNICODE_STRING;

	extern "C"
	XBOXAPI
	DWORD
	WINAPI
	IoCreateSymbolicLink(
	PUNICODE_STRING SymbolicLinkName,
	PUNICODE_STRING DeviceName
	);

	extern "C"
	XBOXAPI
	DWORD
	WINAPI
	IoDeleteSymbolicLink(
	PUNICODE_STRING SymbolicLinkName
	);

// this renames the dvd drive as drive d - allowing running from a dvd
void ForceRunFromDvd(void)
{
	#define DriveD	"\\??\\D:"
	#define CdRom	"\\Device\\Cdrom0"

	UNICODE_STRING	DSymbolicLinkName	= { (USHORT)x_strlen(DriveD), (USHORT)x_strlen(DriveD) + 1, DriveD };
	UNICODE_STRING	CdDeviceName		= { (USHORT)x_strlen(CdRom),  (USHORT)x_strlen(CdRom) + 1,  CdRom };

	// remap d as the cdrom
	VERIFY( IoDeleteSymbolicLink(&DSymbolicLinkName) == 0);
	VERIFY( IoCreateSymbolicLink(&DSymbolicLinkName, &CdDeviceName) == 0);
}

#else

void ForceRunFromDvd(void)
{
}

#endif	//FORCE_RUN_FROM_DVD


void ENG_ForceNonBlockingVSync( void )
{
}

inline
s32 ENG_GetScreenWidth          ( void )
{
    return s_XRes;
}

inline
s32 ENG_GetScreenHeight         ( void )
{
    return s_YRes;
}


u8 xbox_toon_texture[] = 
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x00, 0x10, 0x00, 0x40, 0x00, 0x10, 0x00,    0xFF, 0xFF, 0x51, 0x00, 0xFF, 0x2C, 0x01, 0x00,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,
    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0xB2, 0xBD, 0xBD, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0xB0, 0xBB, 0xBA, 0xFF,
    0xB3, 0xBC, 0xBC, 0xFF, 0xB2, 0xBC, 0xBD, 0xFF,    0xB0, 0xBB, 0xBB, 0xFF, 0xB0, 0xBB, 0xBA, 0xFF,
    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,
    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,
    0xB2, 0xBD, 0xBC, 0xFF, 0xB2, 0xBD, 0xBC, 0xFF,    0xB0, 0xBA, 0xBA, 0xFF, 0xB0, 0xBA, 0xBA, 0xFF,
    0xB3, 0xBD, 0xBC, 0xFF, 0xB2, 0xBC, 0xBD, 0xFF,    0xB0, 0xBB, 0xBA, 0xFF, 0xB0, 0xBB, 0xBA, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0xAE, 0xB8, 0xB8, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0xAA, 0xB5, 0xB5, 0xFF,
    0xAD, 0xB8, 0xB8, 0xFF, 0xAD, 0xB8, 0xB8, 0xFF,    0xAA, 0xB5, 0xB6, 0xFF, 0xAB, 0xB5, 0xB5, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0xA7, 0xB3, 0xB3, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0xA4, 0xB0, 0xAF, 0xFF,
    0xA7, 0xB3, 0xB2, 0xFF, 0xA7, 0xB3, 0xB2, 0xFF,    0xA4, 0xB0, 0xB0, 0xFF, 0xA4, 0xB0, 0xAF, 0xFF,
    0xAD, 0xB8, 0xB8, 0xFF, 0xAD, 0xB8, 0xB8, 0xFF,    0xAA, 0xB6, 0xB5, 0xFF, 0xAA, 0xB5, 0xB5, 0xFF,
    0xAD, 0xB8, 0xB8, 0xFF, 0xAE, 0xB8, 0xB8, 0xFF,    0xAA, 0xB5, 0xB5, 0xFF, 0xAA, 0xB5, 0xB5, 0xFF,
    0xA7, 0xB3, 0xB3, 0xFF, 0xA7, 0xB2, 0xB2, 0xFF,    0xA4, 0xB0, 0xAF, 0xFF, 0xA3, 0xB0, 0xB0, 0xFF,
    0xA7, 0xB3, 0xB2, 0xFF, 0xA7, 0xB3, 0xB3, 0xFF,    0xA4, 0xB0, 0xB0, 0xFF, 0xA3, 0xB0, 0xAF, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0xA1, 0xAE, 0xAD, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x9E, 0xAB, 0xAA, 0xFF,
    0xA1, 0xAD, 0xAD, 0xFF, 0xA1, 0xAD, 0xAD, 0xFF,    0x9E, 0xAA, 0xAA, 0xFF, 0x9E, 0xAB, 0xAA, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x9B, 0xA8, 0xA7, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x98, 0xA5, 0xA5, 0xFF,
    0x9B, 0xA8, 0xA8, 0xFF, 0x9B, 0xA8, 0xA8, 0xFF,    0x97, 0xA6, 0xA5, 0xFF, 0x98, 0xA5, 0xA5, 0xFF,
    0xA1, 0xAD, 0xAD, 0xFF, 0xA1, 0xAE, 0xAD, 0xFF,    0x9E, 0xAB, 0xAA, 0xFF, 0x9E, 0xAB, 0xAB, 0xFF,
    0xA1, 0xAD, 0xAD, 0xFF, 0xA1, 0xAE, 0xAD, 0xFF,    0x9F, 0xAB, 0xAB, 0xFF, 0x9E, 0xAA, 0xAA, 0xFF,
    0x9B, 0xA8, 0xA8, 0xFF, 0x9B, 0xA8, 0xA8, 0xFF,    0x98, 0xA5, 0xA5, 0xFF, 0x97, 0xA6, 0xA5, 0xFF,
    0x9B, 0xA8, 0xA8, 0xFF, 0x9B, 0xA9, 0xA8, 0xFF,    0x98, 0xA6, 0xA4, 0xFF, 0x97, 0xA5, 0xA5, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x94, 0xA3, 0xA2, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x92, 0xA0, 0xA0, 0xFF,
    0x95, 0xA3, 0xA2, 0xFF, 0x95, 0xA3, 0xA2, 0xFF,    0x92, 0xA0, 0xA0, 0xFF, 0x92, 0xA0, 0x9F, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0x90, 0x9E, 0x9E, 0xFF,    0x00, 0x00, 0x00, 0xFF, 0x8E, 0x9D, 0x9C, 0xFF,
    0x8F, 0x9E, 0x9E, 0xFF, 0x90, 0x9E, 0x9D, 0xFF,    0x8E, 0x9D, 0x9C, 0xFF, 0x8E, 0x9D, 0x9C, 0xFF,
    0x95, 0xA3, 0xA2, 0xFF, 0x95, 0xA3, 0xA2, 0xFF,    0x92, 0xA0, 0xA0, 0xFF, 0x92, 0xA1, 0xA0, 0xFF,
    0x94, 0xA3, 0xA3, 0xFF, 0x95, 0xA3, 0xA2, 0xFF,    0x92, 0xA1, 0xA0, 0xFF, 0x92, 0xA1, 0xA0, 0xFF,
    0x90, 0x9E, 0x9E, 0xFF, 0x8F, 0x9E, 0x9E, 0xFF,    0x8E, 0x9D, 0x9C, 0xFF, 0x8E, 0x9D, 0x9C, 0xFF,
    0x8F, 0x9E, 0x9D, 0xFF, 0x8F, 0x9E, 0x9E, 0xFF,    0x8E, 0x9D, 0x9C, 0xFF, 0x8E, 0x9D, 0x9C, 0xFF,
    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,
    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,
    0xB3, 0xBC, 0xBD, 0xFF, 0xB3, 0xBC, 0xBC, 0xFF,    0xAF, 0xBB, 0xBB, 0xFF, 0xB0, 0xBB, 0xBB, 0xFF,
    0xB2, 0xBC, 0xBD, 0xFF, 0xB2, 0xBD, 0xBC, 0xFF,    0xB0, 0xBB, 0xBA, 0xFF, 0xB0, 0xBA, 0xBA, 0xFF,
    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,
    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,
    0xB2, 0xBD, 0xBC, 0xFF, 0xB2, 0xBD, 0xBD, 0xFF,    0xB0, 0xBA, 0xBA, 0xFF, 0xB0, 0xBB, 0xBA, 0xFF,
    0xB3, 0xBC, 0xBC, 0xFF, 0xB2, 0xBC, 0xBD, 0xFF,    0xB0, 0xBB, 0xBB, 0xFF, 0xB0, 0xBB, 0xBA, 0xFF,
    0xAD, 0xB8, 0xB7, 0xFF, 0xAD, 0xB8, 0xB8, 0xFF,    0xAA, 0xB6, 0xB5, 0xFF, 0xAA, 0xB5, 0xB6, 0xFF,
    0xAD, 0xB8, 0xB8, 0xFF, 0xAD, 0xB8, 0xB8, 0xFF,    0xAB, 0xB5, 0xB5, 0xFF, 0xAA, 0xB6, 0xB5, 0xFF,
    0xA7, 0xB3, 0xB2, 0xFF, 0xA7, 0xB3, 0xB2, 0xFF,    0xA4, 0xB0, 0xAF, 0xFF, 0xA4, 0xB0, 0xB0, 0xFF,
    0xA7, 0xB3, 0xB2, 0xFF, 0xA7, 0xB3, 0xB3, 0xFF,    0xA4, 0xB0, 0xAF, 0xFF, 0xA4, 0xB0, 0xAF, 0xFF,
    0xAD, 0xB8, 0xB8, 0xFF, 0xAE, 0xB8, 0xB8, 0xFF,    0xAA, 0xB5, 0xB5, 0xFF, 0xAA, 0xB5, 0xB5, 0xFF,
    0xAD, 0xB8, 0xB8, 0xFF, 0xAD, 0xB8, 0xB8, 0xFF,    0xAA, 0xB5, 0xB6, 0xFF, 0xAB, 0xB5, 0xB5, 0xFF,
    0xA7, 0xB2, 0xB2, 0xFF, 0xA7, 0xB3, 0xB3, 0xFF,    0xA3, 0xB0, 0xB0, 0xFF, 0xA4, 0xB0, 0xAF, 0xFF,
    0xA7, 0xB3, 0xB2, 0xFF, 0xA7, 0xB3, 0xB2, 0xFF,    0xA4, 0xB0, 0xB0, 0xFF, 0xA4, 0xB0, 0xAF, 0xFF,
    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,
    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,
    0xB2, 0xBD, 0xBC, 0xFF, 0xB2, 0xBD, 0xBC, 0xFF,    0xB0, 0xBA, 0xBA, 0xFF, 0xB0, 0xBA, 0xBA, 0xFF,
    0xB3, 0xBD, 0xBC, 0xFF, 0xB2, 0xBC, 0xBD, 0xFF,    0xB0, 0xBB, 0xBA, 0xFF, 0xB0, 0xBB, 0xBA, 0xFF,
    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,
    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,
    0xB3, 0xBC, 0xBD, 0xFF, 0xB2, 0xBD, 0xBC, 0xFF,    0xAF, 0xBB, 0xBB, 0xFF, 0xB0, 0xBA, 0xBA, 0xFF,
    0xB3, 0xBD, 0xBC, 0xFF, 0xB2, 0xBC, 0xBD, 0xFF,    0xB0, 0xBB, 0xBA, 0xFF, 0xB0, 0xBB, 0xBA, 0xFF,
    0xAD, 0xB8, 0xB8, 0xFF, 0xAD, 0xB8, 0xB8, 0xFF,    0xAA, 0xB6, 0xB5, 0xFF, 0xAA, 0xB5, 0xB5, 0xFF,
    0xAD, 0xB8, 0xB8, 0xFF, 0xAE, 0xB8, 0xB8, 0xFF,    0xAA, 0xB5, 0xB5, 0xFF, 0xAA, 0xB5, 0xB5, 0xFF,
    0xA7, 0xB3, 0xB3, 0xFF, 0xA7, 0xB2, 0xB2, 0xFF,    0xA4, 0xB0, 0xAF, 0xFF, 0xA3, 0xB0, 0xB0, 0xFF,
    0xA7, 0xB3, 0xB2, 0xFF, 0xA7, 0xB3, 0xB3, 0xFF,    0xA4, 0xB0, 0xB0, 0xFF, 0xA3, 0xB0, 0xAF, 0xFF,
    0xAD, 0xB8, 0xB7, 0xFF, 0xAD, 0xB8, 0xB8, 0xFF,    0xAA, 0xB6, 0xB5, 0xFF, 0xAA, 0xB5, 0xB5, 0xFF,
    0xAD, 0xB8, 0xB8, 0xFF, 0xAE, 0xB8, 0xB8, 0xFF,    0xAA, 0xB5, 0xB5, 0xFF, 0xAA, 0xB5, 0xB5, 0xFF,
    0xA7, 0xB3, 0xB2, 0xFF, 0xA7, 0xB2, 0xB2, 0xFF,    0xA4, 0xB0, 0xAF, 0xFF, 0xA3, 0xB0, 0xB0, 0xFF,
    0xA7, 0xB3, 0xB2, 0xFF, 0xA7, 0xB3, 0xB3, 0xFF,    0xA4, 0xB0, 0xB0, 0xFF, 0xA3, 0xB0, 0xAF, 0xFF,
    0xA1, 0xAD, 0xAD, 0xFF, 0xA1, 0xAD, 0xAD, 0xFF,    0x9E, 0xAB, 0xAA, 0xFF, 0x9E, 0xAA, 0xAA, 0xFF,
    0xA1, 0xAD, 0xAD, 0xFF, 0xA1, 0xAD, 0xAD, 0xFF,    0x9E, 0xAB, 0xAA, 0xFF, 0x9E, 0xAB, 0xAA, 0xFF,
    0x9B, 0xA8, 0xA8, 0xFF, 0x9B, 0xA8, 0xA8, 0xFF,    0x98, 0xA6, 0xA5, 0xFF, 0x97, 0xA6, 0xA5, 0xFF,
    0x9B, 0xA8, 0xA8, 0xFF, 0x9B, 0xA8, 0xA8, 0xFF,    0x98, 0xA5, 0xA5, 0xFF, 0x98, 0xA5, 0xA5, 0xFF,
    0xA1, 0xAE, 0xAD, 0xFF, 0xA1, 0xAE, 0xAD, 0xFF,    0x9E, 0xAB, 0xAB, 0xFF, 0x9E, 0xAB, 0xAA, 0xFF,
    0xA1, 0xAD, 0xAD, 0xFF, 0xA1, 0xAD, 0xAD, 0xFF,    0x9E, 0xAA, 0xAA, 0xFF, 0x9E, 0xAB, 0xAA, 0xFF,
    0x9B, 0xA8, 0xA8, 0xFF, 0x9B, 0xA8, 0xA7, 0xFF,    0x97, 0xA6, 0xA5, 0xFF, 0x98, 0xA5, 0xA5, 0xFF,
    0x9B, 0xA8, 0xA8, 0xFF, 0x9B, 0xA8, 0xA8, 0xFF,    0x97, 0xA6, 0xA5, 0xFF, 0x98, 0xA5, 0xA5, 0xFF,
    0x95, 0xA3, 0xA2, 0xFF, 0x95, 0xA3, 0xA2, 0xFF,    0x92, 0xA1, 0xA0, 0xFF, 0x92, 0xA0, 0xA0, 0xFF,
    0x95, 0xA3, 0xA2, 0xFF, 0x95, 0xA3, 0xA2, 0xFF,    0x92, 0xA0, 0x9F, 0xFF, 0x92, 0xA0, 0xA0, 0xFF,
    0x90, 0x9E, 0x9D, 0xFF, 0x8F, 0x9E, 0x9E, 0xFF,    0x8E, 0x9D, 0x9C, 0xFF, 0x8E, 0x9D, 0x9C, 0xFF,
    0x90, 0x9E, 0x9D, 0xFF, 0x90, 0x9E, 0x9E, 0xFF,    0x8E, 0x9D, 0x9C, 0xFF, 0x8E, 0x9D, 0x9C, 0xFF,
    0x95, 0xA3, 0xA2, 0xFF, 0x94, 0xA3, 0xA2, 0xFF,    0x92, 0xA1, 0xA0, 0xFF, 0x92, 0xA0, 0xA0, 0xFF,
    0x95, 0xA3, 0xA2, 0xFF, 0x95, 0xA3, 0xA2, 0xFF,    0x92, 0xA0, 0xA0, 0xFF, 0x92, 0xA0, 0x9F, 0xFF,
    0x8F, 0x9E, 0x9E, 0xFF, 0x90, 0x9E, 0x9E, 0xFF,    0x8E, 0x9D, 0x9C, 0xFF, 0x8E, 0x9D, 0x9C, 0xFF,
    0x8F, 0x9E, 0x9E, 0xFF, 0x90, 0x9E, 0x9D, 0xFF,    0x8E, 0x9D, 0x9C, 0xFF, 0x8E, 0x9D, 0x9C, 0xFF,
    0xA1, 0xAD, 0xAD, 0xFF, 0xA1, 0xAE, 0xAD, 0xFF,    0x9E, 0xAB, 0xAA, 0xFF, 0x9E, 0xAB, 0xAB, 0xFF,
    0xA1, 0xAD, 0xAD, 0xFF, 0xA1, 0xAE, 0xAD, 0xFF,    0x9F, 0xAB, 0xAB, 0xFF, 0x9E, 0xAA, 0xAA, 0xFF,
    0x9B, 0xA8, 0xA8, 0xFF, 0x9B, 0xA8, 0xA8, 0xFF,    0x98, 0xA5, 0xA5, 0xFF, 0x97, 0xA6, 0xA5, 0xFF,
    0x9B, 0xA8, 0xA8, 0xFF, 0x9B, 0xA9, 0xA8, 0xFF,    0x98, 0xA6, 0xA4, 0xFF, 0x97, 0xA5, 0xA5, 0xFF,
    0xA1, 0xAD, 0xAD, 0xFF, 0xA1, 0xAE, 0xAD, 0xFF,    0x9E, 0xAB, 0xAA, 0xFF, 0x9E, 0xAB, 0xAB, 0xFF,
    0xA1, 0xAD, 0xAD, 0xFF, 0xA1, 0xAE, 0xAD, 0xFF,    0x9F, 0xAB, 0xAB, 0xFF, 0x9E, 0xAA, 0xAA, 0xFF,
    0x9B, 0xA8, 0xA8, 0xFF, 0x9B, 0xA8, 0xA8, 0xFF,    0x98, 0xA6, 0xA5, 0xFF, 0x97, 0xA6, 0xA5, 0xFF,
    0x9B, 0xA8, 0xA8, 0xFF, 0x9B, 0xA9, 0xA8, 0xFF,    0x98, 0xA6, 0xA4, 0xFF, 0x97, 0xA5, 0xA5, 0xFF,
    0x95, 0xA3, 0xA2, 0xFF, 0x95, 0xA3, 0xA2, 0xFF,    0x92, 0xA0, 0xA0, 0xFF, 0x92, 0xA1, 0xA0, 0xFF,
    0x94, 0xA3, 0xA3, 0xFF, 0x95, 0xA3, 0xA2, 0xFF,    0x92, 0xA1, 0xA0, 0xFF, 0x92, 0xA1, 0xA0, 0xFF,
    0x90, 0x9E, 0x9E, 0xFF, 0x8F, 0x9E, 0x9E, 0xFF,    0x8E, 0x9D, 0x9C, 0xFF, 0x8E, 0x9D, 0x9C, 0xFF,
    0x8F, 0x9E, 0x9D, 0xFF, 0x8F, 0x9E, 0x9E, 0xFF,    0x8E, 0x9D, 0x9C, 0xFF, 0x8E, 0x9D, 0x9C, 0xFF,
    0x95, 0xA3, 0xA2, 0xFF, 0x95, 0xA3, 0xA2, 0xFF,    0x92, 0xA1, 0xA0, 0xFF, 0x92, 0xA1, 0xA0, 0xFF,
    0x94, 0xA3, 0xA3, 0xFF, 0x95, 0xA3, 0xA2, 0xFF,    0x92, 0xA1, 0xA0, 0xFF, 0x92, 0xA1, 0xA0, 0xFF,
    0x90, 0x9E, 0x9D, 0xFF, 0x8F, 0x9E, 0x9E, 0xFF,    0x8E, 0x9D, 0x9C, 0xFF, 0x8E, 0x9D, 0x9C, 0xFF,
    0x8F, 0x9E, 0x9D, 0xFF, 0x8F, 0x9E, 0x9E, 0xFF,    0x8E, 0x9D, 0x9C, 0xFF, 0x8E, 0x9D, 0x9C, 0xFF,
    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,
    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,
    0xB3, 0xBC, 0xBD, 0xFF, 0xB3, 0xBC, 0xBD, 0xFF,    0xAF, 0xBB, 0xBB, 0xFF, 0xB0, 0xBA, 0xBB, 0xFF,
    0xB2, 0xBD, 0xBC, 0xFF, 0xB3, 0xBD, 0xBC, 0xFF,    0xB0, 0xBA, 0xBA, 0xFF, 0xB0, 0xBB, 0xBA, 0xFF,
    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,
    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,
    0xB2, 0xBC, 0xBD, 0xFF, 0xB3, 0xBC, 0xBD, 0xFF,    0xB0, 0xBB, 0xBA, 0xFF, 0xAF, 0xBB, 0xBB, 0xFF,
    0xB2, 0xBD, 0xBC, 0xFF, 0xB3, 0xBD, 0xBC, 0xFF,    0xB0, 0xBA, 0xBA, 0xFF, 0xB0, 0xBB, 0xBA, 0xFF,
    0xAD, 0xB8, 0xB7, 0xFF, 0xAD, 0xB8, 0xB8, 0xFF,    0xAA, 0xB6, 0xB5, 0xFF, 0xAA, 0xB6, 0xB5, 0xFF,
    0xAD, 0xB8, 0xB8, 0xFF, 0xAD, 0xB8, 0xB8, 0xFF,    0xAA, 0xB5, 0xB5, 0xFF, 0xAA, 0xB5, 0xB5, 0xFF,
    0xA7, 0xB3, 0xB2, 0xFF, 0xA7, 0xB3, 0xB3, 0xFF,    0xA4, 0xB0, 0xAF, 0xFF, 0xA4, 0xB0, 0xB0, 0xFF,
    0xA7, 0xB2, 0xB2, 0xFF, 0xA7, 0xB3, 0xB2, 0xFF,    0xA3, 0xB0, 0xB0, 0xFF, 0xA4, 0xB0, 0xB0, 0xFF,
    0xAE, 0xB8, 0xB8, 0xFF, 0xAD, 0xB8, 0xB7, 0xFF,    0xAA, 0xB5, 0xB5, 0xFF, 0xAA, 0xB6, 0xB5, 0xFF,
    0xAD, 0xB8, 0xB8, 0xFF, 0xAD, 0xB8, 0xB8, 0xFF,    0xAA, 0xB5, 0xB5, 0xFF, 0xAA, 0xB5, 0xB5, 0xFF,
    0xA7, 0xB3, 0xB3, 0xFF, 0xA7, 0xB3, 0xB2, 0xFF,    0xA3, 0xB0, 0xAF, 0xFF, 0xA4, 0xB0, 0xAF, 0xFF,
    0xA7, 0xB2, 0xB2, 0xFF, 0xA7, 0xB3, 0xB2, 0xFF,    0xA3, 0xB0, 0xB0, 0xFF, 0xA4, 0xB0, 0xB0, 0xFF,
    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,
    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,
    0xB2, 0xBD, 0xBC, 0xFF, 0xB3, 0xBD, 0xBC, 0xFF,    0xB0, 0xBA, 0xBA, 0xFF, 0xB0, 0xBB, 0xBA, 0xFF,
    0xB2, 0xBC, 0xBD, 0xFF, 0xB3, 0xBC, 0xBD, 0xFF,    0xB0, 0xBB, 0xBA, 0xFF, 0xAF, 0xBB, 0xBB, 0xFF,
    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,
    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,    0xB4, 0xBE, 0xBE, 0xFF, 0xB4, 0xBE, 0xBE, 0xFF,
    0xB3, 0xBC, 0xBD, 0xFF, 0xB3, 0xBD, 0xBC, 0xFF,    0xB0, 0xBA, 0xBB, 0xFF, 0xB0, 0xBB, 0xBA, 0xFF,
    0xB2, 0xBC, 0xBD, 0xFF, 0xB2, 0xBD, 0xBC, 0xFF,    0xB0, 0xBB, 0xBA, 0xFF, 0xB0, 0xBA, 0xBA, 0xFF,
    0xAD, 0xB8, 0xB8, 0xFF, 0xAD, 0xB8, 0xB8, 0xFF,    0xAA, 0xB5, 0xB5, 0xFF, 0xAA, 0xB5, 0xB5, 0xFF,
    0xAE, 0xB8, 0xB8, 0xFF, 0xAD, 0xB8, 0xB7, 0xFF,    0xAA, 0xB5, 0xB5, 0xFF, 0xAA, 0xB6, 0xB5, 0xFF,
    0xA7, 0xB2, 0xB2, 0xFF, 0xA7, 0xB3, 0xB2, 0xFF,    0xA3, 0xB0, 0xB0, 0xFF, 0xA4, 0xB0, 0xB0, 0xFF,
    0xA7, 0xB3, 0xB3, 0xFF, 0xA7, 0xB3, 0xB2, 0xFF,    0xA3, 0xB0, 0xAF, 0xFF, 0xA4, 0xB0, 0xAF, 0xFF,
    0xAD, 0xB8, 0xB8, 0xFF, 0xAD, 0xB8, 0xB8, 0xFF,    0xAA, 0xB6, 0xB5, 0xFF, 0xAA, 0xB5, 0xB5, 0xFF,
    0xAE, 0xB8, 0xB8, 0xFF, 0xAD, 0xB8, 0xB8, 0xFF,    0xAA, 0xB5, 0xB5, 0xFF, 0xAA, 0xB5, 0xB5, 0xFF,
    0xA7, 0xB3, 0xB3, 0xFF, 0xA7, 0xB3, 0xB2, 0xFF,    0xA4, 0xB0, 0xB0, 0xFF, 0xA4, 0xB0, 0xB0, 0xFF,
    0xA7, 0xB3, 0xB3, 0xFF, 0xA7, 0xB2, 0xB2, 0xFF,    0xA3, 0xB0, 0xAF, 0xFF, 0xA3, 0xB0, 0xB0, 0xFF,
    0xA1, 0xAD, 0xAD, 0xFF, 0xA1, 0xAE, 0xAD, 0xFF,    0x9E, 0xAB, 0xAA, 0xFF, 0x9E, 0xAB, 0xAA, 0xFF,
    0xA1, 0xAE, 0xAD, 0xFF, 0xA1, 0xAD, 0xAD, 0xFF,    0x9E, 0xAB, 0xAB, 0xFF, 0x9F, 0xAB, 0xAB, 0xFF,
    0x9B, 0xA8, 0xA8, 0xFF, 0x9B, 0xA9, 0xA8, 0xFF,    0x98, 0xA6, 0xA5, 0xFF, 0x98, 0xA5, 0xA5, 0xFF,
    0x9B, 0xA8, 0xA8, 0xFF, 0x9B, 0xA8, 0xA8, 0xFF,    0x97, 0xA6, 0xA5, 0xFF, 0x98, 0xA6, 0xA4, 0xFF,
    0xA1, 0xAE, 0xAD, 0xFF, 0xA1, 0xAD, 0xAD, 0xFF,    0x9E, 0xAA, 0xAA, 0xFF, 0x9E, 0xAB, 0xAA, 0xFF,
    0xA1, 0xAE, 0xAD, 0xFF, 0xA1, 0xAD, 0xAD, 0xFF,    0x9E, 0xAB, 0xAB, 0xFF, 0x9F, 0xAB, 0xAB, 0xFF,
    0x9B, 0xA9, 0xA8, 0xFF, 0x9B, 0xA8, 0xA8, 0xFF,    0x97, 0xA5, 0xA5, 0xFF, 0x98, 0xA6, 0xA5, 0xFF,
    0x9B, 0xA8, 0xA8, 0xFF, 0x9B, 0xA8, 0xA8, 0xFF,    0x97, 0xA6, 0xA5, 0xFF, 0x98, 0xA6, 0xA4, 0xFF,
    0x95, 0xA3, 0xA2, 0xFF, 0x95, 0xA3, 0xA2, 0xFF,    0x92, 0xA1, 0xA0, 0xFF, 0x92, 0xA0, 0x9F, 0xFF,
    0x95, 0xA3, 0xA2, 0xFF, 0x94, 0xA3, 0xA3, 0xFF,    0x92, 0xA1, 0xA0, 0xFF, 0x92, 0xA1, 0xA0, 0xFF,
    0x90, 0x9E, 0x9D, 0xFF, 0x8F, 0x9E, 0x9D, 0xFF,    0x8E, 0x9D, 0x9C, 0xFF, 0x8E, 0x9D, 0x9C, 0xFF,
    0x8F, 0x9E, 0x9E, 0xFF, 0x8F, 0x9E, 0x9D, 0xFF,    0x8E, 0x9D, 0x9C, 0xFF, 0x8E, 0x9D, 0x9C, 0xFF,
    0x95, 0xA3, 0xA2, 0xFF, 0x95, 0xA3, 0xA2, 0xFF,    0x92, 0xA1, 0xA0, 0xFF, 0x92, 0xA1, 0xA0, 0xFF,
    0x95, 0xA3, 0xA2, 0xFF, 0x94, 0xA3, 0xA3, 0xFF,    0x92, 0xA1, 0xA0, 0xFF, 0x92, 0xA1, 0xA0, 0xFF,
    0x8F, 0x9E, 0x9E, 0xFF, 0x90, 0x9E, 0x9D, 0xFF,    0x8E, 0x9D, 0x9C, 0xFF, 0x8E, 0x9D, 0x9C, 0xFF,
    0x8F, 0x9E, 0x9E, 0xFF, 0x8F, 0x9E, 0x9D, 0xFF,    0x8E, 0x9D, 0x9C, 0xFF, 0x8E, 0x9D, 0x9C, 0xFF,
    0xA1, 0xAE, 0xAD, 0xFF, 0xA1, 0xAD, 0xAD, 0xFF,    0x9E, 0xAB, 0xAB, 0xFF, 0x9F, 0xAB, 0xAB, 0xFF,
    0xA1, 0xAE, 0xAD, 0xFF, 0xA1, 0xAD, 0xAD, 0xFF,    0x9E, 0xAA, 0xAA, 0xFF, 0x9E, 0xAB, 0xAA, 0xFF,
    0x9B, 0xA8, 0xA8, 0xFF, 0x9B, 0xA8, 0xA8, 0xFF,    0x97, 0xA6, 0xA5, 0xFF, 0x98, 0xA6, 0xA4, 0xFF,
    0x9B, 0xA9, 0xA8, 0xFF, 0x9B, 0xA8, 0xA8, 0xFF,    0x97, 0xA5, 0xA5, 0xFF, 0x98, 0xA6, 0xA5, 0xFF,
    0xA1, 0xAE, 0xAD, 0xFF, 0xA1, 0xAD, 0xAD, 0xFF,    0x9E, 0xAB, 0xAA, 0xFF, 0x9F, 0xAB, 0xAB, 0xFF,
    0xA1, 0xAE, 0xAD, 0xFF, 0xA1, 0xAE, 0xAD, 0xFF,    0x9E, 0xAA, 0xAA, 0xFF, 0x9E, 0xAB, 0xAB, 0xFF,
    0x9B, 0xA9, 0xA8, 0xFF, 0x9B, 0xA8, 0xA8, 0xFF,    0x98, 0xA5, 0xA5, 0xFF, 0x98, 0xA6, 0xA4, 0xFF,
    0x9B, 0xA9, 0xA8, 0xFF, 0x9B, 0xA8, 0xA8, 0xFF,    0x97, 0xA5, 0xA5, 0xFF, 0x97, 0xA6, 0xA5, 0xFF,
    0x95, 0xA3, 0xA2, 0xFF, 0x94, 0xA3, 0xA3, 0xFF,    0x92, 0xA1, 0xA0, 0xFF, 0x92, 0xA1, 0xA0, 0xFF,
    0x95, 0xA3, 0xA2, 0xFF, 0x95, 0xA3, 0xA2, 0xFF,    0x92, 0xA1, 0xA0, 0xFF, 0x92, 0xA1, 0xA0, 0xFF,
    0x8F, 0x9E, 0x9E, 0xFF, 0x8F, 0x9E, 0x9D, 0xFF,    0x8E, 0x9D, 0x9C, 0xFF, 0x8E, 0x9D, 0x9C, 0xFF,
    0x8F, 0x9E, 0x9E, 0xFF, 0x90, 0x9E, 0x9D, 0xFF,    0x8E, 0x9D, 0x9C, 0xFF, 0x8E, 0x9D, 0x9C, 0xFF,
    0x95, 0xA3, 0xA2, 0xFF, 0x94, 0xA3, 0xA3, 0xFF,    0x92, 0xA0, 0x9F, 0xFF, 0x92, 0xA1, 0xA0, 0xFF,
    0x95, 0xA3, 0xA2, 0xFF, 0x95, 0xA3, 0xA2, 0xFF,    0x92, 0xA1, 0xA0, 0xFF, 0x92, 0xA1, 0xA0, 0xFF,
    0x8F, 0x9E, 0x9D, 0xFF, 0x8F, 0x9E, 0x9D, 0xFF,    0x8E, 0x9D, 0x9C, 0xFF, 0x8E, 0x9D, 0x9C, 0xFF,
    0x8F, 0x9E, 0x9E, 0xFF, 0x8F, 0x9E, 0x9E, 0xFF,    0x8E, 0x9D, 0x9C, 0xFF, 0x8E, 0x9D, 0x9C, 0xFF,
};

static x_bitmap *s_pToonTexture = NULL;

// I feel like a rat bastard for doing this, but hey.
class x_bitmap_cheat : public x_bitmap
{
public:
    void SetDataPtr( byte *ptr );
    void SetClutPtr( byte *ptr );
};

//=========================================================================

void x_bitmap_cheat::SetDataPtr( byte *ptr )
{
    m_pData = ptr;
}

//=========================================================================

void x_bitmap_cheat::SetClutPtr( byte *ptr )
{
    m_pClut = ptr;
}

//=========================================================================

static s32 s_IsRegistered = 0;

void XBOX_InitToonTexture( void )
{
    if (!s_IsRegistered)
    {
        s_pToonTexture = (x_bitmap *)xbox_toon_texture;
        // cheat the data ptr
        ((x_bitmap_cheat*)s_pToonTexture)->SetDataPtr( (byte*)(xbox_toon_texture + sizeof(x_bitmap)) );
        ((x_bitmap_cheat*)s_pToonTexture)->SetClutPtr( (byte*)(s_pToonTexture->GetDataPtr() + s_pToonTexture->GetDataSize()) );

        VRAM_Register( *s_pToonTexture, 0 );
    }
    ++s_IsRegistered;
}

void XBOX_ActivateToonTexture( void )
{
    if (s_pToonTexture)
        VRAM_XBOX_Activate( *s_pToonTexture, 3 );
}

void XBOX_KillToonTexture( void )
{
    --s_IsRegistered;
    if (!s_IsRegistered)
        VRAM_UnRegister( *s_pToonTexture );
}


void XBOX_InitSepiaToonCheat( void )
{
    s_bSepiaToonCheatActive = TRUE;
}

void XBOX_KillSepiaToonCheat( void )
{
    s_bSepiaToonCheatActive = FALSE;
}

xbool XBOX_IsToonCheatActive( void )
{
//s_bSepiaToonCheatActive = !s_bSepiaToonCheatActive;
    return s_bSepiaToonCheatActive;
}


