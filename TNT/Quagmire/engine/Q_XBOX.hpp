////////////////////////////////////////////////////////////////////////////
//
//  Q_XBOX.HPP
//
////////////////////////////////////////////////////////////////////////////

#ifndef Q_XBOX_HPP
#define Q_XBOX_HPP

////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "xtl.h"
#include "x_types.hpp"
#include "x_color.hpp"


////////////////////////////////////////////////////////////////////////////
// HELPER MACROS
////////////////////////////////////////////////////////////////////////////

#define SAFE_RELEASE( p )       { if( p ) { (p)->Release(); (p)=NULL; } }

#define DXCHECK( hr )           { if( hr < 0 ) { XBOX_Warning( hr, __FILE__, __LINE__ ); return ERR_FAILURE; } }
#define DXWARN( hr )            { if( hr < 0 ) { XBOX_Warning( hr, __FILE__, __LINE__ ); } }


////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////

// Values passed to XBOX_SetStencilMode()
#define ENG_STENCIL_ON          0x01
#define ENG_STENCIL_REV         0x02
#define ENG_STENCIL_OFF         0x04
#define ENG_STENCIL_CLEAR       0x08

// Values passed to XBOX_SetShadowBiasMode()
#define ENG_SHADOW_BIAS_ON      0x01
#define ENG_SHADOW_BIAS_OFF     0x02
#define ENG_SHADOW_BIAS_RESET   0x04


////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
////////////////////////////////////////////////////////////////////////////

LPDIRECT3DDEVICE8 XBOX_GetDevice    ( void );
LPDIRECT3D8       XBOX_GetD3D       ( void );
HRESULT           XBOX_GetLastError ( void );
void              XBOX_Warning      ( HRESULT hr, char* pFile, u32 line );

D3DXMATRIX  XBOX_GetProjectionMatrix( void );

void XBOX_InitStencilOverlay        ( void );
void XBOX_KillStencilOverlay        ( void );
void XBOX_SetStencilMode            ( s32 StencilMode, u8 StencilRef = 255, xbool bFindRef = TRUE );
void XBOX_DrawStencilOverlay        ( color StencilColor );
void XBOX_DrawStencilOverlayAllRefs ( void );

void XBOX_NextShadowBiasLevel       ( void );
void XBOX_SetShadowBiasMode         ( s32 Mode, u8 ShadowBias = 255 );


void XBOX_InitToonTexture           ( void );
void XBOX_ActivateToonTexture       ( void );
void XBOX_KillToonTexture           ( void );
void XBOX_InitSepiaToonCheat        ( void );
void XBOX_KillSepiaToonCheat        ( void );
extern xbool XBOX_IsToonCheatActive ( void );

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////
//
//  XBOX_RegisterVertexShader()
//
//      Registers a custom vertex shader and returns a vertex shader handle.
//      User can set a callback function that is called when the shader is
//      activated.
//
//
//  XBOX_ActivateVertexShader()
//
//      Activates a fixed/custom vertex shader
//
//
//  XBOX_RunVertexStateShader()
//      
//      Executes a state shader with option data (4 f32s) that are placed 
//      into the V0 shader register
//
//
//  XBOX_GetActiveVertexShader()
//
//      Gets the active vertex shader
//
//  
//  XBOX_SetVertexShaderConst()
//
//      Sets the vertex shader const.  Size is # of registers ( 4 f32 per reg )
//
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

typedef void (*XBOX_VSH_CALLBACK)( s32 ShaderID );

s32  XBOX_RegisterVertexShader ( void* pVSCodeData, void* pVtxFmtData, u32 Flags = 0, XBOX_VSH_CALLBACK funcCallback = NULL );
void XBOX_ActivateVertexShader ( s32 ShaderID );
void XBOX_RunVertexStateShader ( s32 ShaderID, const f32* pData );
s32  XBOX_GetActiveVertexShader( void );
void XBOX_SetVertexShaderConst ( s32 ConstReg, const void* const pConstData, s32 Size );




////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//  PIXEL SHADERS
////////////////////////////////////////////////////////////////////////////
//
//  XBOX_RegisterPixelShader()
//
//  XBOX_ActivatePixelShader()
//
//  XBOX_DeActivatePixelShader()
//
//  XBOX_GetActivePixelShader()
//
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

s32  XBOX_RegisterPixelShader ( void* pPSCodeData );
void XBOX_ActivatePixelShader ( s32 ShaderID );
void XBOX_DeActivatePixelShader ( s32 ShaderID );
s32  XBOX_GetActivePixelShader( void );



/*
s32  XBOX_RegisterVertexShader      ( void* pVSCodeData, void* pVtxFmtData, u32 Flags = 0 );
void XBOX_ActivateVertexShader      ( s32 ShaderID );
s32  XBOX_GetActiveVertexShader     ( void );

s32  XBOX_RegisterPixelShader       ( void* pPSCodeData );
void XBOX_ActivatePixelShader       ( s32 ShaderID );
void XBOX_DeActivatePixelShader     ( s32 ShaderID );
s32  XBOX_GetActivePixelShader      ( void );
*/

////////////////////////////////////////////////////////////////////////////

#endif
