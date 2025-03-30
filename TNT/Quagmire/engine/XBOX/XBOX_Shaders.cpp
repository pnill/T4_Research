///////////////////////////////////////////////////////////////////////////
//
//  XBOX_Shaders.cpp
//
///////////////////////////////////////////////////////////////////////////

//#define __USE_OLD_CODE__


#ifndef __USE_OLD_CODE__


////////////////////////////////////////////////////////////////////////////
//
//
//
//
////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////

#include "x_types.hpp"
#include "x_debug.hpp"
#include "Q_XBOX.hpp"


//////////////////////////////////////////////////////////////////////////
// DEFINES
//////////////////////////////////////////////////////////////////////////

#define MAX_XBOX_VTX_SHADERS    64
#define MAX_XBOX_CACHED_VTX_SHADERS 32
#define MAX_XBOX_PIX_SHADERS    16

struct SVertexShader
{
    SVertexShader( void ) : Handle(0), Callback(0) {};

    s32                 Handle;
    XBOX_VSH_CALLBACK   Callback;
};

struct SCachedVertexShader
{
    SCachedVertexShader( void ) : ID(0), Addr(0) {};

    s32 ID;
    s32 Addr;
};

////////////////////////////////////////////////////////////////////////////
// VARIABLES
////////////////////////////////////////////////////////////////////////////

static s32 s_PixShaderHandles[MAX_XBOX_PIX_SHADERS] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, };
static s32 s_NPixShaderHandles = 0;
static s32 s_ActivePixShader   = -1;

static SVertexShader        s_VtxShader[MAX_XBOX_VTX_SHADERS];
static s32                  s_NVtxShaderHandles                 = 0;
static s32                  s_ActiveVtxShader                   = 0x7fffffff;
static s32                  s_ActiveVtxShaderAddr               = 0;
static XBOX_VSH_CALLBACK    s_ActiveVtxShaderCallback           = NULL;
static xbool                s_bFixedFunctionShader              = TRUE;
static s32                  s_CACHE_NVtxShaders                 = 0;
static s32                  s_CACHE_VtxShadersSize              = 0;
static SCachedVertexShader  s_CACHE_VtxShader[MAX_XBOX_CACHED_VTX_SHADERS];


////////////////////////////////////////////////////////////////////////////
//  Inline Cache Functions
////////////////////////////////////////////////////////////////////////////

inline void XBOX_VSCACHE_FlushVertexShaders( void )
{
    s_CACHE_NVtxShaders = 0;
    s_CACHE_VtxShadersSize = 0;
}


//==========================================================================
//  XBOX_VSCACHE_LoadVertexShader()
//      
//==========================================================================
inline void XBOX_VSCACHE_LoadVertexShader( s32 ShaderID )
{
    ASSERT( ShaderID >= 0 && ShaderID < MAX_XBOX_VTX_SHADERS );

    //#define VSCACHE_ALWAYSFLUSH
    #if defined(VSCACHE_ALWAYSFLUSH)
    XBOX_VSCACHE_FlushVertexShaders();
    #endif

    ////////////////////////////////////////////////////////////////////////
    //  Search to see if shader is already loaded
    ////////////////////////////////////////////////////////////////////////  
    s32 s;
    for ( s = 0; s < s_CACHE_NVtxShaders; s++ )
    {
        if ( s_CACHE_VtxShader[s].ID == ShaderID ) break;
    }

    ////////////////////////////////////////////////////////////////////////
    //  If shader is not loaded, load and set active
    ////////////////////////////////////////////////////////////////////////  
    if ( s >= s_CACHE_NVtxShaders )
    {
        u32 ShaderSize;
        s32 ShaderHandle = s_VtxShader[ShaderID].Handle;

        ////////////////////////////////////////////////////////////////////
        //  See if we have room for new loaded shader
        ////////////////////////////////////////////////////////////////////
        DXWARN( XBOX_GetDevice()->GetVertexShaderSize( ShaderHandle, (UINT*)&ShaderSize ) );
        if ( ShaderSize + s_CACHE_VtxShadersSize >= 136 || s >= MAX_XBOX_CACHED_VTX_SHADERS ) 
        {
            XBOX_VSCACHE_FlushVertexShaders();
        }

        ////////////////////////////////////////////////////////////////////
        //  Load the shader into the cache and select it
        ////////////////////////////////////////////////////////////////////
        s_CACHE_VtxShader[s_CACHE_NVtxShaders].ID = ShaderID;
        s_CACHE_VtxShader[s_CACHE_NVtxShaders].Addr = s_CACHE_VtxShadersSize;
        DXWARN( XBOX_GetDevice()->LoadVertexShader( ShaderHandle, s_CACHE_VtxShadersSize ) );
        DXWARN( XBOX_GetDevice()->SelectVertexShader( ShaderHandle, s_CACHE_VtxShader[s_CACHE_NVtxShaders].Addr ) );

        ////////////////////////////////////////////////////////////////////
        //  Set active shader addr
        ////////////////////////////////////////////////////////////////////
        s_ActiveVtxShaderAddr = s_CACHE_VtxShader[s_CACHE_NVtxShaders].Addr;

        s_CACHE_NVtxShaders++;
        s_CACHE_VtxShadersSize += ShaderSize;

    }
    ////////////////////////////////////////////////////////////////////////
    //  Shader is loaded, so set active
    ////////////////////////////////////////////////////////////////////////
    else
    {
        DXWARN( XBOX_GetDevice()->SelectVertexShader( s_VtxShader[ShaderID].Handle, s_CACHE_VtxShader[s].Addr ) );
        s_ActiveVtxShaderAddr = s_CACHE_VtxShader[s].Addr;
    }
}


//////////////////////////////////////////////////////////////////////////
// FUNCTIONS
//////////////////////////////////////////////////////////////////////////

//==========================================================================
//  XBOX_RegisterVertexShader()
//  Registers a custom vertex shader to be used by the engine
//==========================================================================
s32 XBOX_RegisterVertexShader( void* pVSCodeData, void* pVtxFmtData, u32 uFlags, XBOX_VSH_CALLBACK funcCallback )
{

    ASSERT( s_NVtxShaderHandles < MAX_XBOX_VTX_SHADERS );

    ////////////////////////////////////////////////////////////////////////
    //  Create a new vertex shader and set callback func
    ////////////////////////////////////////////////////////////////////////
    DXWARN( XBOX_GetDevice()->CreateVertexShader( (DWORD*)pVtxFmtData,
                                                  (DWORD*)pVSCodeData,
                                                  (DWORD*)&(s_VtxShader[s_NVtxShaderHandles].Handle),
                                                  (DWORD)uFlags ) );

    s_VtxShader[s_NVtxShaderHandles].Callback = funcCallback;

    s_NVtxShaderHandles++;

    return ( 0x80000000 | ( s_NVtxShaderHandles - 1 ) );
}


//==========================================================================
//  XBOX_GetActiveVertexShader()
//      Gets the active vertex shader
//==========================================================================
s32 XBOX_GetActiveVertexShader( void )
{
    return s_ActiveVtxShader;
}


//==========================================================================
//  XBOX_ActivateVertexShader()
//      Activates a vertex shader
//==========================================================================
void XBOX_ActivateVertexShader( s32 ShaderID )
{

    ////////////////////////////////////////////////////////////////////////
    //  Valid Engine Awareness of which shader is active
    ////////////////////////////////////////////////////////////////////////
    //#define VALIDATE_VSHADERS
    #if defined ( VALIDATE_VSHADERS ) && defined( X_DEBUG )
    if ( s_ActiveVtxShader != 0x7fffffff )
    {  
        DWORD Handle;
        XBOX_GetDevice()->GetVertexShader( &Handle );
        if ( s_ActiveVtxShader & 0x80000000 )  { ASSERT( static_cast<s32>(Handle) == s_VtxShader[s_ActiveVtxShader&0x7fffffff].Handle ); }
        else                                   { ASSERT( static_cast<s32>(Handle) == s_ActiveVtxShader );        }
    }
    #endif

    ////////////////////////////////////////////////////////////////////////
    //  If Vertex Shader already active... bail
    ////////////////////////////////////////////////////////////////////////
    if( ShaderID != s_ActiveVtxShader )
    {
        s_ActiveVtxShader = ShaderID;

        ////////////////////////////////////////////////////////////////////////
        //  Activate Custom Vertex Shader
        ////////////////////////////////////////////////////////////////////////
        if ( ShaderID & 0x80000000 ) 
        {
            s32 MaskedShaderID = (ShaderID & 0x7FFFFFFF);
            SVertexShader* pVtxShader = &(s_VtxShader[MaskedShaderID]);

            ASSERT( pVtxShader->Handle != NULL ); 
            ASSERT( MaskedShaderID >= 0 && MaskedShaderID < MAX_XBOX_VTX_SHADERS );

            ////////////////////////////////////////////////////////////////
            //  Ready constant mode if needed
            ////////////////////////////////////////////////////////////////
            if ( s_bFixedFunctionShader )
            {
                //DXWARN( XBOX_GetDevice()->SetShaderConstantMode(D3DSCM_192CONSTANTS) );
                s_bFixedFunctionShader = FALSE;
                //XBOX_InvalidateTransform( XBOXTRANSFORM_ALL );
            }

            ////////////////////////////////////////////////////////////////////
            //  Load Shader
            ////////////////////////////////////////////////////////////////////
            XBOX_VSCACHE_LoadVertexShader( MaskedShaderID );

            ////////////////////////////////////////////////////////////////////
            //  Call activate callback if no callback then last callback 
            //  is still the active callback
            ////////////////////////////////////////////////////////////////////
            if ( pVtxShader->Callback && pVtxShader->Callback != s_ActiveVtxShaderCallback )
            {
                pVtxShader->Callback(ShaderID);
                s_ActiveVtxShaderCallback = pVtxShader->Callback;
            }

        }
        ////////////////////////////////////////////////////////////////////////
        //  Activate Fixed Vertex Shader
        ////////////////////////////////////////////////////////////////////////
        else
        {
            ////////////////////////////////////////////////////////////////
            //  Ready constant mode if needed
            ////////////////////////////////////////////////////////////////
            if ( !s_bFixedFunctionShader )
            {
                //XBOX_InvalidateTransform( XBOXTRANSFORM_ALL );
                //DXWARN( XBOX_GetDevice()->SetShaderConstantMode(D3DSCM_96CONSTANTS) );
                s_bFixedFunctionShader = TRUE;
            }

            ////////////////////////////////////////////////////////////////
            //  Flush shader cache and activate
            ////////////////////////////////////////////////////////////////
            XBOX_VSCACHE_FlushVertexShaders();
            DXWARN( XBOX_GetDevice()->SetVertexShader( ShaderID ) );
            s_ActiveVtxShaderCallback = NULL;
            s_ActiveVtxShaderAddr = 0;

        }
    }

    //XBOX_SendTransforms();
}


//==========================================================================
//
//
//==========================================================================
void XBOX_RunVertexStateShader ( s32 ShaderID, const f32* pData )
{
    XBOX_ActivateVertexShader( ShaderID );

    ////////////////////////////////////////////////////////////////////////
    //  Run Vertex State Shader
    ////////////////////////////////////////////////////////////////////////

    DXWARN( XBOX_GetDevice()->RunVertexStateShader( s_ActiveVtxShaderAddr, pData ) );    
}


//==========================================================================
//
//
//=========================================================================
void XBOX_SetVertexShaderConst ( s32 ConstReg, const void* const pConstData, s32 Size )
{
    ////////////////////////////////////////////////////////////////
    //  Ready constant mode if needed
    ////////////////////////////////////////////////////////////////
    if ( s_bFixedFunctionShader )
    {
        //DXWARN( XBOX_GetDevice()->SetShaderConstantMode(D3DSCM_192CONSTANTS) );
        s_bFixedFunctionShader = FALSE;
        //XBOX_InvalidateTransform( XBOXTRANSFORM_ALL );
    }

    DXWARN( XBOX_GetDevice()->SetVertexShaderConstantFast( ConstReg, pConstData, Size ));
}


//==========================================================================

s32 XBOX_RegisterPixelShader( void* pPSCodeData )
{
    s32 HandleID;

    ASSERT( s_NPixShaderHandles < MAX_XBOX_PIX_SHADERS );

    HandleID = s_NPixShaderHandles;

    s_NPixShaderHandles++;

    DXWARN( XBOX_GetDevice()->CreatePixelShader( (D3DPIXELSHADERDEF*)pPSCodeData,
                                                 (DWORD*)&(s_PixShaderHandles[HandleID]) ) );

    return HandleID;
}

//==========================================================================

s32 XBOX_GetActivePixelShader( void )
{
    return s_ActivePixShader;
}

//==========================================================================

void XBOX_ActivatePixelShader( s32 ShaderID )
{
    ASSERT( ShaderID >= 0 && ShaderID < MAX_XBOX_PIX_SHADERS );

//    if( ShaderID == s_ActivePixShader )
//        return;

    ASSERT( s_PixShaderHandles[ShaderID] != NULL );

    s_ActivePixShader = ShaderID;

    DXWARN( XBOX_GetDevice()->SetPixelShader( s_PixShaderHandles[ShaderID] ) );
}

//==========================================================================

void XBOX_DeActivatePixelShader( s32 ShaderID )
{
    if ( s_ActivePixShader != -1 )
    {
        s_ActivePixShader = -1;
        DXWARN( XBOX_GetDevice()->SetPixelShader( NULL ) );
    }
}
//==========================================================================


#else
//////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////

#include "x_debug.hpp"

#include "Q_XBOX.hpp"


//////////////////////////////////////////////////////////////////////////
// DEFINES
//////////////////////////////////////////////////////////////////////////

#define MAX_XBOX_VTX_SHADERS    16
#define MAX_XBOX_PIX_SHADERS    16


////////////////////////////////////////////////////////////////////////////
// VARIABLES
////////////////////////////////////////////////////////////////////////////

static s32 s_VtxShaderHandles[MAX_XBOX_VTX_SHADERS] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, };
static s32 s_PixShaderHandles[MAX_XBOX_PIX_SHADERS] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, };

static s32 s_NVtxShaderHandles = 0;
static s32 s_ActiveVtxShader   = -1;

static s32 s_NPixShaderHandles = 0;
static s32 s_ActivePixShader   = -1;


//////////////////////////////////////////////////////////////////////////
// FUNCTIONS
//////////////////////////////////////////////////////////////////////////

s32 XBOX_RegisterVertexShader( void* pVSCodeData, void* pVtxFmtData, u32 Flags )
{
    s32 HandleID;

    ASSERT( s_NVtxShaderHandles < MAX_XBOX_VTX_SHADERS );

    HandleID = s_NVtxShaderHandles;

    s_NVtxShaderHandles++;

    DXWARN( XBOX_GetDevice()->CreateVertexShader( (DWORD*)pVtxFmtData,
                                                  (DWORD*)pVSCodeData,
                                                  (DWORD*)&(s_VtxShaderHandles[HandleID]),
                                                  (DWORD)Flags ) );

    return HandleID;
}

//==========================================================================

s32 XBOX_GetActiveVertexShader( void )
{
    return s_ActiveVtxShader;
}

//==========================================================================

void XBOX_ActivateVertexShader( s32 ShaderID )
{
    ASSERT( ShaderID >= 0 && ShaderID < MAX_XBOX_VTX_SHADERS );

//    if( ShaderID == s_ActiveVtxShader )
//        return;

    ASSERT( s_VtxShaderHandles[ShaderID] != NULL );

    s_ActiveVtxShader = ShaderID;

    DXWARN( XBOX_GetDevice()->SetVertexShader( s_VtxShaderHandles[ShaderID] ) );
}

//==========================================================================

s32 XBOX_RegisterPixelShader( void* pPSCodeData )
{
    s32 HandleID;

    ASSERT( s_NPixShaderHandles < MAX_XBOX_PIX_SHADERS );

    HandleID = s_NPixShaderHandles;

    s_NPixShaderHandles++;

    DXWARN( XBOX_GetDevice()->CreatePixelShader( (D3DPIXELSHADERDEF*)pPSCodeData,
                                                 (DWORD*)&(s_PixShaderHandles[HandleID]) ) );

    return HandleID;
}

//==========================================================================

s32 XBOX_GetActivePixelShader( void )
{
    return s_ActivePixShader;
}

//==========================================================================

void XBOX_ActivatePixelShader( s32 ShaderID )
{
    ASSERT( ShaderID >= 0 && ShaderID < MAX_XBOX_PIX_SHADERS );

    if( ShaderID == s_ActivePixShader )
        return;

    ASSERT( s_PixShaderHandles[ShaderID] != NULL );

    s_ActivePixShader = ShaderID;

    DXWARN( XBOX_GetDevice()->SetPixelShader( s_PixShaderHandles[ShaderID] ) );
}

//==========================================================================

void XBOX_DeActivatePixelShader( s32 ShaderID )
{
    ASSERT( ShaderID >= 0 && ShaderID < MAX_XBOX_PIX_SHADERS );

    ASSERT( s_ActivePixShader != -1 );

    s_ActivePixShader = -1;

    DXWARN( XBOX_GetDevice()->SetPixelShader( NULL ) );
}
//==========================================================================
#endif