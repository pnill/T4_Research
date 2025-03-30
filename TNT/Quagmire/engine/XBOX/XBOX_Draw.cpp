////////////////////////////////////////////////////////////////////////////
//
// XBOX_Draw.cpp
//
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////
#define USE_NEW_DRAW
#if defined( USE_NEW_DRAW )
 
#include "Q_Engine.hpp"
#include "Q_XBOX.hpp"
#include "Q_Draw.hpp"
#include "Q_VRAM.hpp"
#include "XBOX_Draw.hpp"
  
////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////

#define PUSHBUFFER_SIZE     (1024*512)
#define PUSHBUFFER_KICKSIZE (1024*32)

#define DRAW_MAX_VERTS      4096


////////////////////////////////////////////////////////////////////////////
// VARIABLES
////////////////////////////////////////////////////////////////////////////

static u32              s_DrawMode;
static matrix4          s_VertexL2W;
static matrix4          s_NormalL2W;
static matrix4          s_SaveProjMtx;
static matrix4          s_SaveViewMtx;
static x_bitmap*        s_pActiveTexture;
static xbool            s_bActivateTexture;


static void DRAW_PushPrimative( D3DPRIMITIVETYPE PrimType,
                           s32              nVerts,
                           vector3*         pPos,
                           color*           pColor,
                           vector2*         pUV,
                           vector3*         pNormal,
                           xbool            bForce2D = FALSE );

////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

//==========================================================================
//  InitModule()
//      Initializes the DRAW module
//==========================================================================
err DRAW_InitModule( void )
{
    ASSERT( XBOX_GetDevice() != NULL );

    ////////////////////////////////////////////////////////////////////////
    // Init DRAW's variables
    ////////////////////////////////////////////////////////////////////////
    s_DrawMode = 0;    
    s_VertexL2W.Identity();
    s_NormalL2W.Identity();

    ////////////////////////////////////////////////////////////////////////
    //  Init Active Texture
    ////////////////////////////////////////////////////////////////////////
    s_pActiveTexture = NULL;
    s_bActivateTexture = TRUE;

    return ERR_SUCCESS;
}


//==========================================================================
//  KillModule()
//      Cleans up the draw module
//==========================================================================
void DRAW_KillModule( void )
{

}

void DRAW_SetL2W( matrix4* VertexL2W, matrix4* NormalL2W )
{
    matrix4 W2S;
    matrix4 LD;
    matrix4 LC;

    ASSERT( ENG_GetRenderMode() );

    if( VertexL2W == NULL ) s_VertexL2W.Identity();
    else                    s_VertexL2W = *VertexL2W;

    if( NormalL2W == NULL ) s_NormalL2W.Identity();
    else                    s_NormalL2W = *NormalL2W;

    DXWARN( XBOX_GetDevice()->SetTransform( D3DTS_WORLD, (D3DXMATRIX*)&s_VertexL2W.M ) );
}

//==========================================================================

void DRAW_GetL2W( matrix4& VertexL2W )
{
    VertexL2W = s_VertexL2W;
}

static void DRAW_Build2DProjectionMatrix( void )
{
    s32        XRes, YRes;
    view*      pActiveView;
    f32        ZN, ZF;
    D3DXMATRIX matProj;
    matrix4    LocalToWorld;
    matrix4    IDMatrix;

    IDMatrix.Identity();

    LPDIRECT3DDEVICE8 pD3DDevice = XBOX_GetDevice();

    // Save the current projection and view matrices to restore after rendering
    DXWARN( pD3DDevice->GetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&s_SaveProjMtx.M ));
    DXWARN( pD3DDevice->GetTransform( D3DTS_VIEW,       (D3DMATRIX*)&s_SaveViewMtx.M ));

    ENG_GetResolution( XRes, YRes );
    pActiveView = ENG_GetActiveView();
    pActiveView->GetZLimits( ZN, ZF );

    // Setup an orthographic projection matrix
    D3DXMatrixOrthoOffCenterLH( &matProj, 0.0f, (f32)(XRes - 1), (f32)(YRes - 1), 0.0f, ZN, ZF );
    DXWARN( pD3DDevice->SetTransform( D3DTS_PROJECTION, &matProj ));

    // clear the current L2W and W2V matrices so 2D ops aren't affected by them
    DXWARN( pD3DDevice->SetTransform( D3DTS_VIEW,  (D3DMATRIX*)&IDMatrix.M ) );
    DXWARN( pD3DDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&IDMatrix.M ) );
}

//==========================================================================

static void DRAW_RestoreSavedProjectionMatrix( void )
{
    DXWARN( XBOX_GetDevice()->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&s_SaveProjMtx.M ) );
    DXWARN( XBOX_GetDevice()->SetTransform( D3DTS_VIEW,       (D3DMATRIX*)&s_SaveViewMtx.M ) );
}
/*
//==========================================================================
//  DeactivateMaterial()
//      Deactivates the draw material
//==========================================================================
static void DRAW_DeactivateMaterial( void )
{
    VRAM_XBOX_Deactivate( 0 );
    s_matDraw.Deactivate();
}
*/

static void DRAW_SetXBOXTextureStages( void )
{
    u32   CurRenderFlags;
    u32   CurBlendMode;
    u8    CurFixedAlpha;
    s32   ColorOp;
    s32   AlphaOp;
    xbool UsingTexture;

    LPDIRECT3DDEVICE8 pD3DDevice = XBOX_GetDevice();

    // Determine whether the current draw mode is using a texture or not.
    UsingTexture = (s_DrawMode & DRAW_TEXTURE) == DRAW_TEXTURE;

    // Get the current blend modes and render settings from the engine.
    ENG_GetRenderFlags( CurRenderFlags );
    ENG_GetBlendMode( CurBlendMode, CurFixedAlpha );

    // Set the texture state color blend type.
    if( CurRenderFlags & ENG_ALPHA_BLEND_ON )
    {
        // Test to see if a fixed alpha blending mode is to be used.
        if( CurBlendMode & ENG_BLEND_FIXED_ALPHA )
        {
            // If this draw mode is using a texture, then set the color arg to the texture, otherwise use diffuse color.
            if( UsingTexture )
            {
                ColorOp = D3DTOP_MODULATE;
                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ) );
                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR ) );

                AlphaOp = D3DTOP_MODULATE;
                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ) );
                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR ) );
            }
            else
            {
                ColorOp = D3DTOP_SELECTARG1;
                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE ) );

                AlphaOp = D3DTOP_BLENDFACTORALPHA;
                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE ) );
                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR ) );
            }
        }
        else
        {
            // If this draw mode is using a texture, then set the color arg to the texture, otherwise use diffuse color.
            if( UsingTexture )
            {
                ColorOp = D3DTOP_MODULATE;
                AlphaOp = D3DTOP_MODULATE;
                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ) );
                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE ) );

                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ) );
                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE ) );
            }
            else
            {
                ColorOp = D3DTOP_SELECTARG1;
                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE ) );

                AlphaOp = D3DTOP_SELECTARG1;
                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE ) );
            }
        }
    }
    else
    {
        if( UsingTexture )
        {
            // Disable alpha blending altogether.
            AlphaOp = D3DTOP_DISABLE;
            ColorOp = D3DTOP_MODULATE;

            // Set the colors of the pixel to be a combination of the texture and vertex color. 
            DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ) );
            DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE ) );
        }
        else
        {
            // Disable alpha blending altogether.
            AlphaOp = D3DTOP_DISABLE;
            ColorOp = D3DTOP_SELECTARG1;

            // Set the color compont
            DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE ) );
        }
    }

    // Determine the color and alpha operations to be performed.
    DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, ColorOp ) );
    DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, AlphaOp ) );
    DXWARN( pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE ) );
    DXWARN( pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE ) );
}

//==========================================================================
//  SetMode()
//      Sets the draw mode
//==========================================================================
void DRAW_SetMode( u32 DrawMode )
{
    s32 ZFillMode;
    s32 ZTestMode;
    s32 AlphaBlendingMode;
    s32 PrimitiveFillMode;
    s32 LightingMode;

    ASSERT( ENG_GetRenderMode() );

    // Make sure that all options have been selected.
    if( (DrawMode & 0x01111111) != 0x01111111 )
    {
        // At least one of these was missed, find out the first one that was missed.
        ASSERTS( (DrawMode & (DRAW_2D|DRAW_3D)) != 0,              "DRAW_SetMode : 2D or 3D not specified" );
        ASSERTS( (DrawMode & (DRAW_FILL|DRAW_NO_FILL)) != 0,       "DRAW_SetMode : FILL or NO_FILL not specified" );
        ASSERTS( (DrawMode & (DRAW_TEXTURE|DRAW_NO_TEXTURE))!= 0,  "DRAW_SetMode : TEXTURE or NO_TEXTURE not specified" );
        ASSERTS( (DrawMode & (DRAW_ALPHA|DRAW_NO_ALPHA)) != 0,     "DRAW_SetMode : ALPHA or NO_ALPHA not specified" );
        ASSERTS( (DrawMode & (DRAW_ZBUFFER|DRAW_NO_ZBUFFER)) != 0, "DRAW_SetMode : ZBUFFER or NO_ZBUFFER not specified" );
        ASSERTS( (DrawMode & (DRAW_CLIP|DRAW_NO_CLIP)) != 0,       "DRAW_SetMode : CLIP or NO_CLIP not specified" );
        ASSERTS( (DrawMode & (DRAW_LIGHT|DRAW_NO_LIGHT)) != 0,     "DRAW_SetMode : LIGHT or NO_LIGHT not specified" );
    }

    //if( s_DrawMode != DrawMode )
    {
        s_DrawMode = DrawMode;
        // Determine the draw modes.
        AlphaBlendingMode = (s_DrawMode & DRAW_ALPHA)   == DRAW_ALPHA    ?  ENG_ALPHA_BLEND_ON  : ENG_ALPHA_BLEND_OFF;
        ZFillMode         = (s_DrawMode & DRAW_ZBUFFER) == DRAW_ZBUFFER  ?  ENG_ZBUFFER_FILL_ON : ENG_ZBUFFER_FILL_OFF;
        ZTestMode         = (ZFillMode == ENG_ZBUFFER_FILL_ON)           ?  ENG_ZBUFFER_TEST_ON : ENG_ZBUFFER_TEST_OFF;
        PrimitiveFillMode = (s_DrawMode & DRAW_NO_FILL) == DRAW_NO_FILL  ?  D3DFILL_WIREFRAME   : D3DFILL_SOLID;
        LightingMode      = (s_DrawMode & DRAW_LIGHT) == DRAW_LIGHT      ?  FALSE               : FALSE; // ASB DOES NOT USE LIGHTS

        // Set the engine render modes.
        ENG_SetRenderFlags( ZTestMode | ZFillMode | ENG_ALPHA_TEST_OFF | AlphaBlendingMode );
        ENG_SetBlendMode( ENG_BLEND_NORMAL );

        // Set the fill mode.
        DXWARN( XBOX_GetDevice()->SetRenderState( D3DRS_FILLMODE, PrimitiveFillMode ) );

        // Set the lighting mode.
        DXWARN( XBOX_GetDevice()->SetRenderState( D3DRS_LIGHTING, LightingMode ) );

        DRAW_SetXBOXTextureStages( );
    }
}


//==========================================================================
//  SetTexture()
//      Sets the active draw texture
//==========================================================================
void DRAW_SetTexture( x_bitmap* Texture, xbool bActivate )
{
    ASSERT( ENG_GetRenderMode() );

    s_pActiveTexture = Texture;
    s_bActivateTexture = bActivate;

    ////////////////////////////////////////////////////////////////////////
    // Disable texture use on stages other than zero
    ////////////////////////////////////////////////////////////////////////

    DXWARN( XBOX_GetDevice()->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE) );
}



//==========================================================================
//
//
//==========================================================================
static void DRAW_PushPrimative( D3DPRIMITIVETYPE PrimType,
                           s32              nVerts,
                           vector3*         pPos,
                           color*           pColor,
                           vector2*         pUV,
                           vector3*         pNormal,
                           xbool            bForce2D )
{

    xbool Is2D = ( (s_DrawMode & DRAW_2D) == DRAW_2D ) || bForce2D ;
    ASSERT( XBOX_GetDevice() != NULL );

    if( Is2D )
        DRAW_Build2DProjectionMatrix();

    ////////////////////////////////////////////////////////////////////////
    //  Calc Format and Format Size
    ////////////////////////////////////////////////////////////////////////
    
    ASSERT( pPos );
    u32 Format = ( Is2D ) ? D3DFVF_XYZRHW : D3DFVF_XYZ;
    u32 FormatSize = ( Is2D ) ? sizeof(vector4) : sizeof(vector3);

    if ( pNormal && (s_DrawMode & DRAW_LIGHT) == DRAW_LIGHT )
    {
        Format     |= D3DFVF_NORMAL;
        FormatSize += sizeof(vector3);   
    }

    //if ( pColor )
    {
        Format     |= D3DFVF_DIFFUSE;
        FormatSize += sizeof(color);   
    }
    
    if ( pUV && (s_DrawMode & DRAW_TEXTURE) == DRAW_TEXTURE )
    {
        Format     |= D3DFVF_TEX1;
        FormatSize += sizeof(vector2);      
            
        if ( s_bActivateTexture && s_pActiveTexture )
        {
            VRAM_Activate( *s_pActiveTexture );
        }
        
    }
    else VRAM_XBOX_Deactivate( 0 );

    u32 FormatDWORDs = (FormatSize/4);

    DWORD* pPush; 
    DWORD* pEncodeDWORD;
    DWORD  dwordTotal = FormatDWORDs * nVerts;
    DWORD  dwordCount = 0;

    //XBOX_ActivateVertexShader( Format );
    XBOX_ActivateVertexShader( Format );
    //ActivateStdVertexShader( D3DVertexShaderToStdVertexShader(Format) );


    ////////////////////////////////////////////////////////////////////////
    // The "+5" is to reserve enough overhead for the encoding
    // parameters. NSections adds the overhead needed when the push
	// buffer spans multiple sections.
    ////////////////////////////////////////////////////////////////////////

	u32 NSections = ( ( dwordTotal + 2046 ) / 2047 );
    XBOX_GetDevice()->BeginPush( dwordTotal + 5 + NSections, &pPush );

    pPush[0] = D3DPUSH_ENCODE( D3DPUSH_SET_BEGIN_END, 1 );
    pPush[1] = PrimType;
    pEncodeDWORD = &pPush[2];
    pPush += 3;

    for ( s32 i = 0; i < nVerts; i++)
    {
        pPush[0] = *((DWORD*) &pPos[i].X);
        pPush[1] = *((DWORD*) &pPos[i].Y);
        pPush[2] = *((DWORD*) &pPos[i].Z);
        pPush += 3;

        if ( Is2D ) 
        {
            f32 f(1.0f);
            pPush[0] = *((DWORD*) &f );
            pPush += 1;
        }

        if ( Format & D3DFVF_NORMAL ) 
        {
            pPush[0] = *((DWORD*) &pNormal[i].X);
            pPush[1] = *((DWORD*) &pNormal[i].Y);
            pPush[2] = *((DWORD*) &pNormal[i].Z);
            pPush += 3;     
        }

        if ( pColor ) 
        {
            //TODO: Next year we have color range from 0-127 so we dont need to do this
            //pColor[i].R >>= 1;
            //pColor[i].G >>= 1;
            //pColor[i].B >>= 1;
            u32 C = D3DCOLOR_ARGB( pColor[i].A, pColor[i].R, pColor[i].G, pColor[i].B );
            pPush[0] = *((DWORD*) &C);
            pPush += 1;     
        }
        else
        {
            pPush[0] = 0xffffffff;
            pPush += 1;
        }

        if ( Format & D3DFVF_TEX1 ) 
        {
            pPush[0] = *((DWORD*) &pUV[i].X);
            pPush[1] = *((DWORD*) &pUV[i].Y);
            //pPush[2] = 0;
            pPush += 2;     
        }

        ////////////////////////////////////////////////////////////////////
        // NOTE: A maximum of 2047 DWORDs can be specified to D3DPUSH_ENCODE.
        //   If there is more than 2047 DWORDs of vertex data, simply split
        //   the data into multiple D3DPUSH_ENCODE( D3DPUSH_INLINE_ARRAY )
        //   sections.
        ////////////////////////////////////////////////////////////////////////
        dwordCount += FormatDWORDs;
        if ( (dwordCount + FormatDWORDs + 2) > 2047 )
        {
            *pEncodeDWORD = D3DPUSH_ENCODE( D3DPUSH_NOINCREMENT_FLAG | D3DPUSH_INLINE_ARRAY, dwordCount );

            dwordTotal -= dwordCount;
            dwordCount = 0;

            pEncodeDWORD = &pPush[0];
            pPush += 1;
        }
    }

    *pEncodeDWORD = D3DPUSH_ENCODE( D3DPUSH_NOINCREMENT_FLAG | D3DPUSH_INLINE_ARRAY, dwordCount );

    pPush[0] = D3DPUSH_ENCODE( D3DPUSH_SET_BEGIN_END, 1 );
    pPush[1] = 0;
    pPush += 2;

    XBOX_GetDevice()->EndPush( pPush );

    if( Is2D )
        DRAW_RestoreSavedProjectionMatrix();
}

////////////////////////////////////////////////////////////////////////////
//  DRAW FUNCTIONS
////////////////////////////////////////////////////////////////////////////

//==========================================================================
//
//
//==========================================================================
void DRAW_Points( s32 nVerts, vector3* Pos, color* Color )
{
    DRAW_PushPrimative( D3DPT_POINTLIST, nVerts, Pos, Color, NULL, NULL );
}


//==========================================================================
//
//
//==========================================================================
void DRAW_Lines( s32 nVerts, vector3* Pos, color* Color )
{
    DRAW_PushPrimative( D3DPT_LINELIST, nVerts, Pos, Color, NULL, NULL );
}


//==========================================================================
//
//
//==========================================================================
void DRAW_LineStrip( s32 nVerts, vector3* Pos, color* Color )
{
    DRAW_PushPrimative( D3DPT_LINESTRIP, nVerts, Pos, Color, NULL, NULL );
}


//==========================================================================
//
//
//==========================================================================
void DRAW_Triangles( s32 nVerts, vector3* Pos, color* Color, vector2* UV, vector3* Normal )
{
    DRAW_PushPrimative( D3DPT_TRIANGLELIST, nVerts, Pos, Color, UV, Normal );
}


//==========================================================================
//
//
//==========================================================================
void DRAW_TriangleStrip( s32 nVerts, vector3* Pos, color* Color, vector2* UV, vector3* Normal )
{
    DRAW_PushPrimative( D3DPT_TRIANGLESTRIP, nVerts, Pos, Color, UV, Normal );
}


//==========================================================================
//
//
//==========================================================================
void DRAW_TriangleFan( s32 nVerts, vector3* Pos, color* Color, vector2* UV, vector3* Normal )
{
    DRAW_PushPrimative( D3DPT_TRIANGLEFAN, nVerts, Pos, Color, UV, Normal );
}

//==========================================================================
//
//
//==========================================================================
void DRAW_Triangles2D( s32 nVerts, vector3* Pos, color* Color, vector2* UV )
{
    DRAW_PushPrimative( D3DPT_TRIANGLELIST, nVerts, Pos, Color, UV, NULL, TRUE );
}


//==========================================================================
//
//
//==========================================================================
void DRAW_Sprite( f32 X, f32 Y, f32 Z,      // Hot spot (2D Left-Top), (3D Center)
                  f32 W, f32 H,             // (2D pixel W&H), (3D World W&H)
                  color  Color )            // color
{
    DRAW_SpriteUV( X, Y, Z, W, H, 0.0f, 0.0f, 1.0f, 1.0f, Color );
}


//==========================================================================
//
//
//==========================================================================
void DRAW_SpriteUV( f32 X,  f32 Y, f32 Z,   // Hot spot (2D Left-Top), (3D Center)
                    f32 W,  f32 H,          // (2D pixel W&H), (3D World W&H)
                    f32 U0, f32 V0,         // Upper Left   UV  [0.0 - 1.0]
                    f32 U1, f32 V1,         // Bottom Right UV  [0.0 - 1.0]
                    color   Color )         // color
{
    vector3 Pos[4];
    vector2 UV[4];
    UV[0].X = U0;
    UV[0].Y = V0;
    UV[1].X = U0;
    UV[1].Y = V1;
    UV[2].X = U1;
    UV[2].Y = V0;
    UV[3].X = U1;
    UV[3].Y = V1;
    color C[4] = { Color, Color, Color, Color };

    ////////////////////////////////////////////////////////////////////////
    //Build world space coordinates of billboard verts
    ////////////////////////////////////////////////////////////////////////
    if ( (s_DrawMode & DRAW_2D) == DRAW_2D )
    {
        Pos[0].X = X;
        Pos[0].Y = Y;    
        Pos[0].Z = Z;
        Pos[1].X = X;
        Pos[1].Y = Y+H;    
        Pos[1].Z = Z;
        Pos[2].X = X+W;
        Pos[2].Y = Y;
        Pos[2].Z = Z;
        Pos[3].X = X+W;
        Pos[3].Y = Y+H;
        Pos[3].Z = Z;
        
        DRAW_PushPrimative( D3DPT_TRIANGLESTRIP, 4, Pos, C, UV, NULL );
        return;
    }

    matrix4 W2V;
    ENG_GetActiveView()->GetV2WMatrix( W2V );

    vector3 cross, up, look;
    W2V.GetColumns(cross, up, look);

    f32 halfWidth  = W*0.5f;
    f32 halfHeight = H*0.5f;
    vector3 const origin = vector3(X,Y,Z);        
    Pos[0]  = origin + up*halfHeight + cross*halfWidth;
    Pos[1]  = origin - up*halfHeight + cross*halfWidth;
    Pos[2]  = origin + up*halfHeight - cross*halfWidth;
    Pos[3]  = origin - up*halfHeight - cross*halfWidth;

    DRAW_PushPrimative( D3DPT_TRIANGLESTRIP, 4, Pos, C, UV, NULL );

    /*
    XBOX_GetDevice()->SetRenderState( D3DRS_POINTSPRITEENABLE, TRUE );
    XBOX_GetDevice()->SetRenderState( D3DRS_POINTSCALEENABLE,  TRUE );
    XBOX_GetDevice()->SetRenderState( D3DRS_POINTSIZE,         FtoDW(1.00f) );
    XBOX_GetDevice()->SetRenderState( D3DRS_POINTSCALE_A,      FtoDW(0.00f) );
    XBOX_GetDevice()->SetRenderState( D3DRS_POINTSCALE_B,      FtoDW(0.00f) );
    XBOX_GetDevice()->SetRenderState( D3DRS_POINTSCALE_C,      FtoDW(0.00f) );

    DRAW_PushPrimative( D3DPT_POINTLIST, 4, Pos, C, UV, NULL );
    
    // Reset render states
    XBOX_GetDevice()->SetRenderState( D3DRS_POINTSPRITEENABLE, FALSE );
    XBOX_GetDevice()->SetRenderState( D3DRS_POINTSCALEENABLE,  FALSE );
    */
}


//==========================================================================
//
//
//==========================================================================
void DRAW_Rectangle( f32 X, f32 Y, f32 Z,    // Hot spot (2D Left-Top), (3D Center)
                     f32 W, f32 H,           // (2D pixel W&H), (3D World W&H)
                     color  TLColor,         // Top Left     color
                     color  TRColor,         // Top Right    color
                     color  BLColor,         // Bottom Left  color
                     color  BRColor )        // Bottom Right color
{

    vector3 Pos[4];
    color C[4] = { TLColor, BLColor, TRColor, BRColor };

    ////////////////////////////////////////////////////////////////////////
    //  2D Rect
    ////////////////////////////////////////////////////////////////////////
    if ( (s_DrawMode & DRAW_2D) == DRAW_2D )
    {
        Pos[0].X = X;
        Pos[0].Y = Y;    
        Pos[0].Z = Z;
        Pos[1].X = X;
        Pos[1].Y = Y+H;    
        Pos[1].Z = Z;
        Pos[2].X = X+W;
        Pos[2].Y = Y;
        Pos[2].Z = Z;
        Pos[3].X = X+W;
        Pos[3].Y = Y+H;
        Pos[3].Z = Z;    

        DRAW_PushPrimative( D3DPT_TRIANGLESTRIP, 4, Pos, C, NULL, NULL );
        return;
    }

    ////////////////////////////////////////////////////////////////////////
    //  3D Rect - Build world space coordinates of billboard verts
    ////////////////////////////////////////////////////////////////////////
    matrix4 W2V;
    ENG_GetActiveView()->GetV2WMatrix( W2V );

    vector3 cross, up, look;
    W2V.GetColumns(cross, up, look);

    f32 halfWidth  = W*0.5f;
    f32 halfHeight = H*0.5f;
    vector3 const origin = vector3(X,Y,Z);
    Pos[0]  = origin - up*halfHeight - cross*halfWidth;
    Pos[1]  = origin + up*halfHeight - cross*halfWidth;
    Pos[2]  = origin - up*halfHeight + cross*halfWidth;
    Pos[3]  = origin + up*halfHeight + cross*halfWidth;

    DRAW_PushPrimative( D3DPT_TRIANGLESTRIP, 4, Pos, C, NULL, NULL );


}


////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////
//==========================================================================
//
//
//==========================================================================
void DRAW_BeginTriangles( void )
{

}


//==========================================================================
//
//
//==========================================================================
void DRAW_Begin2DTriangles( void )
{

}


//==========================================================================
//
//
//==========================================================================
void DRAW_EndTriangles( void )
{

}


//==========================================================================
//
//
//==========================================================================
void DRAW_BeginSprite( void )
{

}


//==========================================================================
//
//
//==========================================================================
void DRAW_EndSprite( void )
{

}


//==========================================================================
//
//
//==========================================================================
void DRAW_SetStatTrackers( s32* pNVerts, s32* pNTris, s32* pNBytes )
{

}

//==========================================================================


////////////////////////////////////////////////////////////////////////////
//
// OLD XBOX_Draw.cpp
//
////////////////////////////////////////////////////////////////////////////
#else

////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "Q_Engine.hpp"
#include "Q_XBOX.hpp"
#include "Q_Draw.hpp"
#include "Q_VRAM.hpp"

#include "XBOX_Draw.hpp"


////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////

#define DRAW_MAX_VERTS    4096


// Legend of Custon Vertex Types
//
// XYZ    - 3D Location.
// XYZRHW - 3D Location(homogeneous space, with W component)
// TEXx   - Textured (x represents how many sets, 1 - 4)
// ARGB   - Diffuse color for lit vertex
// NORM   - Vertex normals are provided

#define VERTFORMAT_XYZ_ARGB_NORM_TEX1   ( D3DFVF_XYZ    | D3DFVF_DIFFUSE | D3DFVF_NORMAL | D3DFVF_TEX1 )
#define VERTFORMAT_XYZRHW_ARGB_TEX1     ( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 )


////////////////////////////////////////////////////////////////////////////
// STRUCTURES
////////////////////////////////////////////////////////////////////////////

struct SVtx_XYZ_ARGB_NORM_TEX1
{
    vector3     v;
    vector3     n;
    D3DCOLOR    color;
    vector2     t;
};

struct SVtx_XYZRHW_ARGB_TEX1
{
    D3DXVECTOR4 v;
    D3DCOLOR    color;
    vector2     t;
};

////////////////////////////////////////////////////////////////////////////
//
// NOTE ON VERTEX STRUCTURES AND OPTIMIZATION:
//  To optimize DRAW somewhat, there is a "next vert" postion saved which is the
//  index of the next vert to be used by a DRAW_ function.  The reason for this
//  is that if the vertex buffer is locked with the D3DLOCK_NOOVERWRITE flag, D3D
//  won't have to stall the GPU since we won't be overwriting verts that might be
//  in use by the GPU.  Another flag, D3DLOCK_NOFLUSH, is also used to keep D3D
//  from flushing the vertex cache, since we are only appending to the end of the
//  vertex buffer.  The only restriction with this flag is that the GPU reads in
//  verts 32 bytes at a time, so if we were to write new vert data on the tail
//  end of such a read, the vertex data there would be garbage to the GPU.  To fix
//  this we move the "next vert" position to a point that is aligned to 32 bytes.
//  In the case of the above to structures, their sizes are 36 and 28 bytes, and
//  can be aligned in groups of 8 (36 * 8 and 28 * 8 are multiples of 32).  So in
//  the rendering functions you'll see expressions with ALIGN_8( nVerts ) to
//  accomplish this.
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// VARIABLES
////////////////////////////////////////////////////////////////////////////

static LPDIRECT3DVERTEXBUFFER8 s_pVtxBuf3D = NULL;
static LPDIRECT3DVERTEXBUFFER8 s_pVtxBuf2D = NULL;

static u32          s_DrawMode;
static x_bitmap*    s_pActiveTexture;
static matrix4      s_VertexL2W;
static matrix4      s_NormalL2W;
static matrix4      s_SaveProjMtx;
static matrix4      s_SaveViewMtx;
static s32          s_NextVert3D;
static s32          s_NextVert2D;


////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////





err DRAW_InitModule( void )
{
    ASSERT( XBOX_GetDevice() != NULL );


    // Init DRAW's variables

    s_DrawMode = 0;
    s_VertexL2W.Identity();
    s_NormalL2W.Identity();




    s_pActiveTexture = NULL;


    s_NextVert3D = 0;
    s_NextVert2D = 0;

    // Create vertex buffers for 3D and 2D use
    DXWARN( XBOX_GetDevice()->CreateVertexBuffer( DRAW_MAX_VERTS * sizeof(SVtx_XYZ_ARGB_NORM_TEX1), 0, 0, 0, &s_pVtxBuf3D ) );
    DXWARN( XBOX_GetDevice()->CreateVertexBuffer( DRAW_MAX_VERTS * sizeof(SVtx_XYZRHW_ARGB_TEX1),   0, 0, 0, &s_pVtxBuf2D ) );

    ASSERT( s_pVtxBuf3D != NULL );
    ASSERT( s_pVtxBuf2D != NULL );

    return ERR_SUCCESS;
}

//==========================================================================

void DRAW_KillModule( void )
{
    if( s_pVtxBuf3D != NULL )
        s_pVtxBuf3D->Release();

    if( s_pVtxBuf2D != NULL )
        s_pVtxBuf2D->Release();

    s_pVtxBuf3D = NULL;
    s_pVtxBuf2D = NULL;
}

//==========================================================================

inline xbool DRAW_IsMode2D( void )
{
    return (s_DrawMode & DRAW_2D) == DRAW_2D;
}

//==========================================================================

static void DRAW_SetXBOXTextureStages( void )
{
    u32   CurRenderFlags;
    u32   CurBlendMode;
    u8    CurFixedAlpha;
    s32   ColorOp;
    s32   AlphaOp;
    xbool UsingTexture;

    LPDIRECT3DDEVICE8 pD3DDevice = XBOX_GetDevice();

    // Determine whether the current draw mode is using a texture or not.
    UsingTexture = (s_DrawMode & DRAW_TEXTURE) == DRAW_TEXTURE;

    // Get the current blend modes and render settings from the engine.
    ENG_GetRenderFlags( CurRenderFlags );
    ENG_GetBlendMode( CurBlendMode, CurFixedAlpha );

    // Set the texture state color blend type.
    if( CurRenderFlags & ENG_ALPHA_BLEND_ON )
    {
        // Test to see if a fixed alpha blending mode is to be used.
        if( CurBlendMode & ENG_BLEND_FIXED_ALPHA )
        {
            // If this draw mode is using a texture, then set the color arg to the texture, otherwise use diffuse color.
            if( UsingTexture )
            {
                ColorOp = D3DTOP_MODULATE;
                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ) );
                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR ) );

                AlphaOp = D3DTOP_MODULATE;
                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ) );
                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR ) );
            }
            else
            {
                ColorOp = D3DTOP_SELECTARG1;
                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE ) );

                AlphaOp = D3DTOP_BLENDFACTORALPHA;
                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE ) );
                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR ) );
            }
        }
        else
        {
            // If this draw mode is using a texture, then set the color arg to the texture, otherwise use diffuse color.
            if( UsingTexture )
            {
                ColorOp = D3DTOP_MODULATE;
                AlphaOp = D3DTOP_MODULATE;
                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ) );
                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE ) );

                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ) );
                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE ) );
            }
            else
            {
                ColorOp = D3DTOP_SELECTARG1;
                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE ) );

                AlphaOp = D3DTOP_SELECTARG1;
                DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE ) );
            }
        }
    }
    else
    {
        if( UsingTexture )
        {
            // Disable alpha blending altogether.
            AlphaOp = D3DTOP_DISABLE;
            ColorOp = D3DTOP_MODULATE;

            // Set the colors of the pixel to be a combination of the texture and vertex color. 
            DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ) );
            DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE ) );
        }
        else
        {
            // Disable alpha blending altogether.
            AlphaOp = D3DTOP_DISABLE;
            ColorOp = D3DTOP_SELECTARG1;

            // Set the color compont
            DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE ) );
        }
    }

    // Determine the color and alpha operations to be performed.
    DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, ColorOp ) );
    DXWARN( pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, AlphaOp ) );
    DXWARN( pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE ) );
    DXWARN( pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE ) );
}

//==========================================================================

static void DRAW_Build2DProjectionMatrix( void )
{
    s32        XRes, YRes;
    view*      pActiveView;
    f32        ZN, ZF;
    D3DXMATRIX matProj;
    matrix4    LocalToWorld;
    matrix4    IDMatrix;

    IDMatrix.Identity();

    LPDIRECT3DDEVICE8 pD3DDevice = XBOX_GetDevice();

    // Save the current projection and view matrices to restore after rendering
    DXWARN( pD3DDevice->GetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&s_SaveProjMtx.M ));
    DXWARN( pD3DDevice->GetTransform( D3DTS_VIEW,       (D3DMATRIX*)&s_SaveViewMtx.M ));

    ENG_GetResolution( XRes, YRes );
    pActiveView = ENG_GetActiveView();
    pActiveView->GetZLimits( ZN, ZF );

    // Setup an orthographic projection matrix
    D3DXMatrixOrthoOffCenterLH( &matProj, 0.0f, (f32)(XRes - 1), (f32)(YRes - 1), 0.0f, ZN, ZF );
    DXWARN( pD3DDevice->SetTransform( D3DTS_PROJECTION, &matProj ));

    // clear the current L2W and W2V matrices so 2D ops aren't affected by them
    DXWARN( pD3DDevice->SetTransform( D3DTS_VIEW,  (D3DMATRIX*)&IDMatrix.M ) );
    DXWARN( pD3DDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&IDMatrix.M ) );
}

//==========================================================================

static void DRAW_RestoreSavedProjectionMatrix( void )
{
    DXWARN( XBOX_GetDevice()->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&s_SaveProjMtx.M ) );
    DXWARN( XBOX_GetDevice()->SetTransform( D3DTS_VIEW,       (D3DMATRIX*)&s_SaveViewMtx.M ) );
}

//==========================================================================

void DRAW_SetL2W( matrix4* VertexL2W, matrix4* NormalL2W )
{
    matrix4 W2S;
    matrix4 LD;
    matrix4 LC;

    ASSERT( ENG_GetRenderMode() );

    if( VertexL2W == NULL ) s_VertexL2W.Identity();
    else                    s_VertexL2W = *VertexL2W;

    if( NormalL2W == NULL ) s_NormalL2W.Identity();
    else                    s_NormalL2W = *NormalL2W;

    DXWARN( XBOX_GetDevice()->SetTransform( D3DTS_WORLD, (D3DXMATRIX*)&s_VertexL2W.M ) );
}

//==========================================================================

void DRAW_GetL2W( matrix4& VertexL2W )
{
    VertexL2W = s_VertexL2W;
}


//==========================================================================














void DRAW_SetMode( u32 DrawMode )
{
    s32 ZFillMode;
    s32 ZTestMode;
    s32 AlphaBlendingMode;
    s32 PrimitiveFillMode;
    s32 LightingMode;

    ASSERT( ENG_GetRenderMode() );

    // Make sure that all options have been selected.
    if( (DrawMode & 0x01111111) != 0x01111111 )
    {
        // At least one of these was missed, find out the first one that was missed.
        ASSERTS( (DrawMode & (DRAW_2D|DRAW_3D)) != 0,              "DRAW_SetMode : 2D or 3D not specified" );
        ASSERTS( (DrawMode & (DRAW_FILL|DRAW_NO_FILL)) != 0,       "DRAW_SetMode : FILL or NO_FILL not specified" );
        ASSERTS( (DrawMode & (DRAW_TEXTURE|DRAW_NO_TEXTURE))!= 0,  "DRAW_SetMode : TEXTURE or NO_TEXTURE not specified" );
        ASSERTS( (DrawMode & (DRAW_ALPHA|DRAW_NO_ALPHA)) != 0,     "DRAW_SetMode : ALPHA or NO_ALPHA not specified" );
        ASSERTS( (DrawMode & (DRAW_ZBUFFER|DRAW_NO_ZBUFFER)) != 0, "DRAW_SetMode : ZBUFFER or NO_ZBUFFER not specified" );
        ASSERTS( (DrawMode & (DRAW_CLIP|DRAW_NO_CLIP)) != 0,       "DRAW_SetMode : CLIP or NO_CLIP not specified" );
        ASSERTS( (DrawMode & (DRAW_LIGHT|DRAW_NO_LIGHT)) != 0,     "DRAW_SetMode : LIGHT or NO_LIGHT not specified" );
    }

    //if( s_DrawMode != DrawMode )
    {
        s_DrawMode = DrawMode;
        // Determine the draw modes.
        AlphaBlendingMode = (s_DrawMode & DRAW_ALPHA)   == DRAW_ALPHA    ?  ENG_ALPHA_BLEND_ON  : ENG_ALPHA_BLEND_OFF;
        ZFillMode         = (s_DrawMode & DRAW_ZBUFFER) == DRAW_ZBUFFER  ?  ENG_ZBUFFER_FILL_ON : ENG_ZBUFFER_FILL_OFF;
        ZTestMode         = (ZFillMode == ENG_ZBUFFER_FILL_ON)           ?  ENG_ZBUFFER_TEST_ON : ENG_ZBUFFER_TEST_OFF;
        PrimitiveFillMode = (s_DrawMode & DRAW_NO_FILL) == DRAW_NO_FILL  ?  D3DFILL_WIREFRAME   : D3DFILL_SOLID;
        LightingMode      = (s_DrawMode & DRAW_LIGHT) == DRAW_LIGHT      ?  FALSE               : FALSE; // ASB DOES NOT USE LIGHTS

        // Set the engine render modes.
        ENG_SetRenderFlags( ZTestMode | ZFillMode | ENG_ALPHA_TEST_OFF | AlphaBlendingMode );
        ENG_SetBlendMode( ENG_BLEND_NORMAL );

        // Set the fill mode.
        DXWARN( XBOX_GetDevice()->SetRenderState( D3DRS_FILLMODE, PrimitiveFillMode ) );

        // Set the lighting mode.
        DXWARN( XBOX_GetDevice()->SetRenderState( D3DRS_LIGHTING, LightingMode ) );

        DRAW_SetXBOXTextureStages( );
    }
}

//==========================================================================





void DRAW_SetTexture( x_bitmap* Texture, xbool bActivate )
{
    ASSERT( ENG_GetRenderMode() );

    s_pActiveTexture = Texture;


    if( Texture == NULL )
    {
        VRAM_XBOX_Deactivate( 0 );
        return;
    }

    if( bActivate )
        VRAM_Activate( *Texture );

    // Disable texture use on stages other than zero
    DXWARN( XBOX_GetDevice()->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE) );
}


////////////////////////////////////////////////////////////////////////////
// LINE AND POINT PRIMITIVE RENDERING FUNCTIONS
////////////////////////////////////////////////////////////////////////////

static void DrawLinePointPrim( s32 nVerts, vector3* Pos, color* Color, D3DPRIMITIVETYPE PrimType, s32 nPrims )









{
    s32 i;
    s32 Flags = 0;

    ASSERT( Pos != NULL );
    ASSERT( nVerts < DRAW_MAX_VERTS );

    SVtx_XYZ_ARGB_NORM_TEX1* pVtx;







    // Determine if current pos in vtx buffer will hold all the verts, and either
    // adjust start position or set flags to indicate old verts will not be overwritten
    if( (nVerts + s_NextVert3D) >= DRAW_MAX_VERTS )
    {
        s_NextVert3D = 0;

    }
    else

    {
        Flags |= D3DLOCK_NOOVERWRITE | D3DLOCK_NOFLUSH;













    }























    // Lock the vertex buffer
    DXWARN( s_pVtxBuf3D->Lock( s_NextVert3D * sizeof(SVtx_XYZ_ARGB_NORM_TEX1),
                                nVerts * sizeof(SVtx_XYZ_ARGB_NORM_TEX1), (BYTE**)&pVtx, Flags ) );
    ASSERT( pVtx != NULL );

    // Set vertex values
    for( i = 0; i < nVerts; i++ )
    {
        pVtx[i].v = Pos[i];











        pVtx[i].n.Set( 0.0f, 0.0f, 1.0f );







        if( Color != NULL )
            pVtx[i].color = D3DCOLOR_ARGB( Color[i].A, Color[i].R, Color[i].G, Color[i].B );








        else
            pVtx[i].color = D3DCOLOR_ARGB( 255, 255, 255, 255 );




        pVtx[i].t.Set( 0, 0 );
    }






    DXWARN( s_pVtxBuf3D->Unlock() );










    if( DRAW_IsMode2D() )
        DRAW_Build2DProjectionMatrix();

    // Draw the primitives
    XBOX_ActivateVertexShader( VERTFORMAT_XYZ_ARGB_NORM_TEX1 );
    DXWARN( XBOX_GetDevice()->SetStreamSource( 0, s_pVtxBuf3D, sizeof(SVtx_XYZ_ARGB_NORM_TEX1) ) );
    DXWARN( XBOX_GetDevice()->DrawPrimitive( PrimType, s_NextVert3D, nPrims) );

    // Increment next starting point into vtx buffer
    s_NextVert3D += ALIGN_8( nVerts );

    if( DRAW_IsMode2D() )
        DRAW_RestoreSavedProjectionMatrix();



}

//==========================================================================







void DRAW_Points( s32 nVerts, vector3* Pos, color* Color )
{
    DrawLinePointPrim( nVerts, Pos, Color, D3DPT_POINTLIST, nVerts );
}

//==========================================================================





void DRAW_Lines( s32 nVerts, vector3* Pos, color* Color )
{
    DrawLinePointPrim( nVerts, Pos, Color, D3DPT_LINELIST, (nVerts / 2) );
}

//==========================================================================





void DRAW_LineStrip( s32 nVerts, vector3* Pos, color* Color )
{
    DrawLinePointPrim( nVerts, Pos, Color, D3DPT_LINESTRIP, (nVerts - 1) );
}

////////////////////////////////////////////////////////////////////////////
// TRIANGLE PRIMITIVE RENDERING FUNCTIONS
////////////////////////////////////////////////////////////////////////////

static void DrawTrianglePrim( s32              nVerts,
                              vector3*         Pos,
                              color*           Color,
                              vector2*         UV,
                              vector3*         Normal,
                              D3DPRIMITIVETYPE PrimType,
                              s32              nPrims )
{
    s32 i;
    s32 Flags = 0;

    ASSERT( Pos != NULL );
    ASSERT( nVerts < DRAW_MAX_VERTS );

    SVtx_XYZ_ARGB_NORM_TEX1* pVtx;

    // Determine if current pos in vtx buffer will hold all the verts, and either
    // adjust start position or set flags to indicate old verts will not be overwritten
    if( (nVerts + s_NextVert3D) >= DRAW_MAX_VERTS )
    {
        s_NextVert3D = 0;
    }
    else
    {
        Flags |= D3DLOCK_NOOVERWRITE | D3DLOCK_NOFLUSH;
    }

    // Lock the vertex buffer
    DXWARN( s_pVtxBuf3D->Lock( s_NextVert3D * sizeof(SVtx_XYZ_ARGB_NORM_TEX1),
                                nVerts * sizeof(SVtx_XYZ_ARGB_NORM_TEX1), (BYTE**)&pVtx, Flags ) );
    ASSERT( pVtx != NULL );

    // Set vertex values
    for( i = 0; i < nVerts; i++ )
    {
        pVtx[i].v = Pos[i];

        if( Normal != NULL )
            pVtx[i].n = Normal[i];
        else
            pVtx[i].n.Set( 0.0f, 0.0f, 1.0f );

        if( Color != NULL )
            pVtx[i].color = D3DCOLOR_ARGB( Color[i].A, Color[i].R, Color[i].G, Color[i].B );
        else
            pVtx[i].color = D3DCOLOR_ARGB( 255, 255, 255, 255 );

        if( UV != NULL )
            pVtx[i].t = UV[i];
        else
            pVtx[i].t.Set( 0, 0 );
    }

    DXWARN( s_pVtxBuf3D->Unlock() );

    if( DRAW_IsMode2D() )
        DRAW_Build2DProjectionMatrix();

    // Draw the primitives
    XBOX_ActivateVertexShader( VERTFORMAT_XYZ_ARGB_NORM_TEX1 );
    DXWARN( XBOX_GetDevice()->SetStreamSource( 0, s_pVtxBuf3D, sizeof(SVtx_XYZ_ARGB_NORM_TEX1) ) );
    DXWARN( XBOX_GetDevice()->DrawPrimitive( PrimType, s_NextVert3D, nPrims ) );

    // Increment next starting point into vtx buffer
    s_NextVert3D += ALIGN_8( nVerts );

    if( DRAW_IsMode2D() )
        DRAW_RestoreSavedProjectionMatrix();
}

//==========================================================================

void DRAW_Triangles( s32 nVerts, vector3* Pos, color* Color, vector2* UV, vector3* Normal )
{
    DrawTrianglePrim( nVerts, Pos, Color, UV, Normal, D3DPT_TRIANGLELIST, (nVerts / 3) );
}

//==========================================================================





void DRAW_TriangleStrip( s32 nVerts, vector3* Pos, color* Color, vector2* UV, vector3* Normal )
{
    DrawTrianglePrim( nVerts, Pos, Color, UV, Normal, D3DPT_TRIANGLESTRIP, (nVerts - 2) );
}

//==========================================================================





void DRAW_TriangleFan( s32 nVerts, vector3* Pos, color* Color, vector2* UV, vector3* Normal )
{
    DrawTrianglePrim( nVerts, Pos, Color, UV, Normal, D3DPT_TRIANGLEFAN, (nVerts - 2) );
}

//==========================================================================

void DRAW_Triangles2D( s32 nVerts, vector3* Pos, color* Color, vector2* UV )






{
    s32 i;
    s32 Flags = 0;

    ASSERT( Pos != NULL );
    ASSERT( nVerts < DRAW_MAX_VERTS );

    SVtx_XYZRHW_ARGB_TEX1* pVtx;





















    // Determine if current pos in vtx buffer will hold all the verts, and either
    // adjust start position or set flags to indicate old verts will not be overwritten
    if( (nVerts + s_NextVert2D) >= DRAW_MAX_VERTS )

    {
        s_NextVert2D = 0;
    }
    else
    {
        Flags |= D3DLOCK_NOOVERWRITE | D3DLOCK_NOFLUSH;










    }

    // Lock the vertex buffer
    DXWARN( s_pVtxBuf2D->Lock( s_NextVert2D * sizeof(SVtx_XYZRHW_ARGB_TEX1),
                                nVerts * sizeof(SVtx_XYZRHW_ARGB_TEX1), (BYTE**)&pVtx, Flags ) );
    ASSERT( pVtx != NULL );

    // Set vertex values
    for( i = 0; i < nVerts; i++ )
    {
        pVtx[i].v = D3DXVECTOR4( Pos[i].X, Pos[i].Y, 0.0f, 1.0f );

        if( Color != NULL )
            pVtx[i].color = D3DCOLOR_ARGB( Color[i].A, Color[i].R, Color[i].G, Color[i].B );
        else
            pVtx[i].color = D3DCOLOR_ARGB( 255, 255, 255, 255 );

        if( UV != NULL )
            pVtx[i].t = UV[i];
        else
            pVtx[i].t.Set( 0, 0 );
    }

    DXWARN( s_pVtxBuf2D->Unlock() );


    if( DRAW_IsMode2D() )
        DRAW_Build2DProjectionMatrix();






    // Draw the primitives
    XBOX_ActivateVertexShader( VERTFORMAT_XYZRHW_ARGB_TEX1 );
    DXWARN( XBOX_GetDevice()->SetStreamSource( 0, s_pVtxBuf2D, sizeof(SVtx_XYZRHW_ARGB_TEX1) ) );
    DXWARN( XBOX_GetDevice()->DrawPrimitive( D3DPT_TRIANGLELIST, s_NextVert2D, (nVerts / 3) ) );

    // Increment next starting point into vtx buffer
    s_NextVert2D += ALIGN_8( nVerts );






    if( DRAW_IsMode2D() )
        DRAW_RestoreSavedProjectionMatrix();




}

////////////////////////////////////////////////////////////////////////////
// 2D SPRITE AND RECTANGLE RENDERING FUNCTIONS
////////////////////////////////////////////////////////////////////////////

inline void Make2DVert( SVtx_XYZRHW_ARGB_TEX1& rVtx,
                        f32 X, f32 Y, f32 Z,
                        const color& Color,
                        f32 U, f32 V )
{
    rVtx.v.x = X;
    rVtx.v.y = Y;
    rVtx.v.z = Z;
    rVtx.v.w = 1.0f;

    rVtx.color = D3DCOLOR_ARGB( Color.A, Color.R, Color.G, Color.B );

    rVtx.t.X = U;
    rVtx.t.Y = V;
}

//==========================================================================

static void Draw2DImage( f32 X,  f32 Y, f32 Z,
                         f32 W,  f32 H,
                         f32 U0, f32 V0,
                         f32 U1, f32 V1,
                         const color& TLColor,
                         const color& TRColor,
                         const color& BLColor,
                         const color& BRColor )
{
    s32 Flags = 0;

    SVtx_XYZRHW_ARGB_TEX1* pVtx;


    // Determine if current pos in vtx buffer will hold all the verts, and either
    // adjust start position or set flags to indicate old verts will not be overwritten
    if( (4 + s_NextVert2D) >= DRAW_MAX_VERTS )

    {
        s_NextVert2D = 0;














    }
    else
    {
        Flags |= D3DLOCK_NOOVERWRITE | D3DLOCK_NOFLUSH;
    }

    // Lock the vertex buffer
    DXWARN( s_pVtxBuf2D->Lock( s_NextVert2D * sizeof(SVtx_XYZRHW_ARGB_TEX1),
                                4 * sizeof(SVtx_XYZRHW_ARGB_TEX1), (BYTE**)&pVtx, Flags ) );
    ASSERT( pVtx != NULL );


    // Set vertex values
    Make2DVert( pVtx[1], X,   Y,   Z, TLColor, U0, V0 );
    Make2DVert( pVtx[0], X,   Y+H, Z, BLColor, U0, V1 );
    Make2DVert( pVtx[2], X+W, Y,   Z, TRColor, U1, V0 );
    Make2DVert( pVtx[3], X+W, Y+H, Z, BRColor, U1, V1 );

    DXWARN( s_pVtxBuf2D->Unlock() );







    if( DRAW_IsMode2D() )
        DRAW_Build2DProjectionMatrix();

    // Draw the quad image
    XBOX_ActivateVertexShader( VERTFORMAT_XYZRHW_ARGB_TEX1 );
    DXWARN( XBOX_GetDevice()->SetStreamSource( 0, s_pVtxBuf2D, sizeof(SVtx_XYZRHW_ARGB_TEX1) ) );
    DXWARN( XBOX_GetDevice()->DrawPrimitive( D3DPT_QUADLIST, s_NextVert2D, 1 ) );

    // Increment next starting point into vtx buffer
    s_NextVert2D += ALIGN_8( 4 );








    if( DRAW_IsMode2D() )
        DRAW_RestoreSavedProjectionMatrix();
}





//==========================================================================



void DRAW_Sprite( f32 X, f32 Y, f32 Z,      // Hot spot (2D Left-Top), (3D Center)
                  f32 W, f32 H,             // (2D pixel W&H), (3D World W&H)
                  color  Color )            // color
{
    Draw2DImage( X, Y, Z, W, H, 0.0f, 0.0f, 1.0f, 1.0f, Color, Color, Color, Color );
}





//==========================================================================



void DRAW_SpriteUV( f32 X,  f32 Y, f32 Z,   // Hot spot (2D Left-Top), (3D Center)
                    f32 W,  f32 H,          // (2D pixel W&H), (3D World W&H)
                    f32 U0, f32 V0,         // Upper Left   UV  [0.0 - 1.0]
                    f32 U1, f32 V1,         // Bottom Right UV  [0.0 - 1.0]
                    color   Color )         // color
{
    Draw2DImage( X, Y, Z, W, H, U0, V0, U1, V1, Color, Color, Color, Color );
}





//==========================================================================



void DRAW_Rectangle( f32 X, f32 Y, f32 Z,    // Hot spot (2D Left-Top), (3D Center)
                     f32 W, f32 H,           // (2D pixel W&H), (3D World W&H)
                     color  TLColor,         // Top Left     color
                     color  TRColor,         // Top Right    color
                     color  BLColor,         // Bottom Left  color
                     color  BRColor )        // Bottom Right color
{
    Draw2DImage( X, Y, Z, W, H, 0.0f, 0.0f, 1.0f, 1.0f, TLColor, TRColor, BLColor, BRColor );
}





//==========================================================================

void DRAW_Input( void )
{

}


////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////

void DRAW_BeginTriangles( void )
{

}

//==========================================================================

void DRAW_Begin2DTriangles( void )
{

}

//==========================================================================

void DRAW_EndTriangles( void )
{

}

//==========================================================================

void DRAW_BeginSprite( void )
{

}

//==========================================================================

void DRAW_EndSprite( void )
{

}

//==========================================================================

void DRAW_SetStatTrackers( s32* pNVerts, s32* pNTris, s32* pNBytes )
{

}

//==========================================================================


#endif