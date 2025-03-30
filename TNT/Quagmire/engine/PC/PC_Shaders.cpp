///////////////////////////////////////////////////////////////////////////
//
//  PC_Shaders.cpp
//
///////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////

#include "x_debug.hpp"

#include "Q_PC.hpp"


//////////////////////////////////////////////////////////////////////////
// DEFINES
//////////////////////////////////////////////////////////////////////////

#define MAX_PC_VTX_SHADERS    16
#define MAX_PC_PIX_SHADERS    16


////////////////////////////////////////////////////////////////////////////
// VARIABLES
////////////////////////////////////////////////////////////////////////////

static s32 s_VtxShaderHandles[MAX_PC_VTX_SHADERS] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, };
static s32 s_PixShaderHandles[MAX_PC_PIX_SHADERS] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, };

static s32 s_NVtxShaderHandles = 0;
static s32 s_ActiveVtxShader   = -1;

static s32 s_NPixShaderHandles = 0;
static s32 s_ActivePixShader   = -1;


//////////////////////////////////////////////////////////////////////////
// FUNCTIONS
//////////////////////////////////////////////////////////////////////////

s32 PC_RegisterVertexShader( void* pVSCodeData, void* pVtxFmtData, u32 Flags )
{
    s32 HandleID;

    ASSERT( s_NVtxShaderHandles < MAX_PC_VTX_SHADERS );

    HandleID = s_NVtxShaderHandles;

    s_NVtxShaderHandles++;

    DXWARN( PC_GetDevice()->CreateVertexShader( (DWORD*)pVtxFmtData,
                                                  (DWORD*)pVSCodeData,
                                                  (DWORD*)&(s_VtxShaderHandles[HandleID]),
                                                  (DWORD)Flags ) );

    return HandleID;
}

//==========================================================================

s32 PC_GetActiveVertexShader( void )
{
    return s_ActiveVtxShader;
}

//==========================================================================

void PC_ActivateVertexShader( s32 ShaderID )
{
    ASSERT( ShaderID >= 0 && ShaderID < MAX_PC_VTX_SHADERS );

//    if( ShaderID == s_ActiveVtxShader )
//        return;

    ASSERT( s_VtxShaderHandles[ShaderID] != NULL );

    s_ActiveVtxShader = ShaderID;

    DXWARN( PC_GetDevice()->SetVertexShader( s_VtxShaderHandles[ShaderID] ) );
}

//==========================================================================

s32 PC_RegisterPixelShader( void* pPSCodeData )
{
    s32 HandleID;

    ASSERT( s_NPixShaderHandles < MAX_PC_PIX_SHADERS );

    HandleID = s_NPixShaderHandles;

    s_NPixShaderHandles++;

    DXWARN( PC_GetDevice()->CreatePixelShader( (const DWORD*)pPSCodeData,
                                                 (DWORD*)&(s_PixShaderHandles[HandleID]) ) );

    return HandleID;
}

//==========================================================================

s32 PC_GetActivePixelShader( void )
{
    return s_ActivePixShader;
}

//==========================================================================

void PC_ActivatePixelShader( s32 ShaderID )
{
    ASSERT( ShaderID >= 0 && ShaderID < MAX_PC_PIX_SHADERS );

//    if( ShaderID == s_ActivePixShader )
//        return;

    ASSERT( s_PixShaderHandles[ShaderID] != NULL );

    s_ActivePixShader = ShaderID;

    DXWARN( PC_GetDevice()->SetPixelShader( s_PixShaderHandles[ShaderID] ) );
}

//==========================================================================

void PC_DeActivatePixelShader( s32 ShaderID )
{
    ASSERT( ShaderID >= 0 && ShaderID < MAX_PC_PIX_SHADERS );

    ASSERT( s_ActivePixShader != -1 );

    s_ActivePixShader = -1;

    DXWARN( PC_GetDevice()->SetPixelShader( NULL ) );
}
//==========================================================================
