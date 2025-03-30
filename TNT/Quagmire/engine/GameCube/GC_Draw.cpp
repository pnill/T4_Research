////////////////////////////////////////////////////////////////////////////
//
//  GC_DRAW.CPP
//
//
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "Q_Draw.hpp"
#include "Q_Engine.hpp"
#include "Q_GC.hpp"
#include "Q_VRAM.hpp"


////////////////////////////////////////////////////////////////////////////
// CONSTANTS
////////////////////////////////////////////////////////////////////////////

#define DRAW_Z_CONV     (-0.9999995f)


////////////////////////////////////////////////////////////////////////////
// STATIC-GLOBAL VARIABLES
////////////////////////////////////////////////////////////////////////////

static u32          s_DrawMode;
static xbool        s_bUsingFixedAlpha;
static s32          s_DXT3AlphaInUse = -1;
static matrix4      s_VertexL2W;
static matrix4      s_NormalL2W;
//static matrix4      s_VertexL2S;
//static matrix4      s_VertexL2C;
//static matrix4      s_VertexC2S;
//static matrix4      s_LightDir;
//static matrix4      s_LightColor;
static x_bitmap*    s_pActiveTexture;

static s32          s_DrawVPL;
static s32          s_DrawVPT;
static s32          s_DrawVPW;
static s32          s_DrawVPH;


////////////////////////////////////////////////////////////////////////////
// DRAW TEXTURE SET-UP
////////////////////////////////////////////////////////////////////////////

inline void SET_DRAW_TEXTURE_ON( void )
{
    s_DXT3AlphaInUse = GC_GetDXT3AlphaMapStage( 0 );

    if( s_DXT3AlphaInUse == -1 )
    {
        GXSetNumChans    ( 1 );
        GXSetNumTexGens  ( 1 );
        GXSetNumTevStages( 1 );
        GXSetTevOrder    ( GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0 );
        GXSetTexCoordGen ( GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY );

        GXSetTevColorOp  ( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );
        GXSetTevAlphaOp  ( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );

        GXSetTevColorIn  ( GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO );
        GXSetTevAlphaIn  ( GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_RASA, GX_CA_TEXA, GX_CA_ZERO );
    }
    else
    {
        GXSetNumChans    ( 1 );
        GXSetNumTexGens  ( 1 );
        GXSetNumTevStages( 2 );
        GXSetTevOrder    ( GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0 );
        GXSetTevOrder    ( GX_TEVSTAGE1, GX_TEXCOORD0, TEXMAP_ID[s_DXT3AlphaInUse], GX_COLOR0A0 );
        GXSetTexCoordGen ( GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY );

        GXSetTevColorOp  ( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );
        GXSetTevAlphaOp  ( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );

        GXSetTevColorOp  ( GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );
        GXSetTevAlphaOp  ( GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );

        // output color from tev0, alpha from tev0 is ignored, then use the alpha from tev1
        GXSetTevColorIn  ( GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO );
        GXSetTevAlphaIn  ( GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO );

        GXSetTevColorIn  ( GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV );
        GXSetTevAlphaIn  ( GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_RASA, GX_CA_TEXA, GX_CA_ZERO );
    }
}

//--------------------------------------------------------------------------

inline void SET_DRAW_TEXTURE_OFF( void )
{
    s_DXT3AlphaInUse = -1;

    GXSetNumChans    ( 1 );
    GXSetNumTexGens  ( 0 );
    GXSetNumTevStages( 1 );
    GXSetTevOrder    ( GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0 );

    GXSetTevColorOp  ( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );
    GXSetTevAlphaOp  ( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );

    GXSetTevColorIn  ( GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC );
    GXSetTevAlphaIn  ( GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA );
}

//--------------------------------------------------------------------------

inline void UPDATE_DRAW_TEXTURE( void )
{
    if ( (s_DrawMode & DRAW_TEXTURE) == DRAW_TEXTURE )
    {
        if( s_DXT3AlphaInUse != GC_GetDXT3AlphaMapStage( 0 ) )
            SET_DRAW_TEXTURE_ON();
    }
}


////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

err DRAW_InitModule( void )
{
    s_DrawMode = 0;
    s_bUsingFixedAlpha = FALSE;
    s_VertexL2W.Identity();
    s_NormalL2W.Identity();

    s_pActiveTexture = NULL;

    return ERR_SUCCESS;
}

//==========================================================================

void DRAW_KillModule( void )
{
    s_pActiveTexture = NULL;
}

//==========================================================================

inline xbool DRAW_IsMode2D( void )
{
    return ((s_DrawMode & DRAW_2D) == DRAW_2D) ? TRUE : FALSE;
}

//==========================================================================

static void DRAW_SetupFixedAlpha( void )
{
    xbool   FixAlphaOn;
    GXColor FixedColor = {255,255,255,255};

    GXTevStageID LastTevStageAlpha;

    GC_GetFixedAlpha( FixAlphaOn, FixedColor.a );

    if( s_DXT3AlphaInUse == -1 )
        LastTevStageAlpha = GX_TEVSTAGE0;
    else
        LastTevStageAlpha = GX_TEVSTAGE1;

    if( FixAlphaOn && !s_bUsingFixedAlpha )
    {
        //--- fixed alpha is ON and DRAW is not currently setup to use it, so set up the fixed alpha
        GXSetTevColor  ( GX_TEVREG0, FixedColor );
        GXSetTevAlphaOp( LastTevStageAlpha, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );
        GXSetTevAlphaIn( LastTevStageAlpha, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A0 );

        s_bUsingFixedAlpha = TRUE;
    }
    else if( !FixAlphaOn && s_bUsingFixedAlpha )
    {
        GXSetTevAlphaOp( LastTevStageAlpha, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );

        //--- fixed alpha is OFF and DRAW is setup to use it, so restore normal tev operations
        if( (s_DrawMode & DRAW_TEXTURE) == DRAW_TEXTURE )
            GXSetTevAlphaIn( LastTevStageAlpha, GX_CA_ZERO, GX_CA_RASA, GX_CA_TEXA, GX_CA_ZERO );
        else
            GXSetTevAlphaIn( LastTevStageAlpha, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA );

        s_bUsingFixedAlpha = FALSE;
    }
}

//==========================================================================

inline void DRAW_Setup2DView( Mtx44& MSavedProj )
{
    s32   XRes, YRes;
    Mtx44 MOrthoProj;

    //--- save original projection matrix
    GC_GetProjMatrix( MSavedProj );

    ENG_GetResolution( XRes, YRes );

    //--- save current viewport and make new full-screen one
    GC_GetViewport( s_DrawVPL, s_DrawVPT, s_DrawVPW, s_DrawVPH );
    GC_SetViewport( 0, 0, XRes, YRes );

    //--- setup ortho projection matrix
    MTXOrtho( MOrthoProj, 0, YRes-1, 0, XRes-1, 0.0f, 1.0f );
    GXSetProjection( MOrthoProj, GX_ORTHOGRAPHIC );

    //--- clear the current local to view matrix so quad isn't affected by it
    Mtx GC_L2V;
    MTXIdentity( GC_L2V );
    GXLoadPosMtxImm( GC_L2V, GX_PNMTX7 );
    GXLoadNrmMtxImm( GC_L2V, GX_PNMTX7 );
    GXSetCurrentMtx( GX_PNMTX7 );
}

//==========================================================================

inline void DRAW_RestoreProj( Mtx44& MSavedProj )
{
    GXSetProjection( MSavedProj, GX_PERSPECTIVE );
    GXSetCurrentMtx( GX_PNMTX6 );
    GC_SetViewport( s_DrawVPL, s_DrawVPT, s_DrawVPW, s_DrawVPH );
}

//==========================================================================

inline f32 DRAW_Get2DZ( f32 Z3D )
{
    //--- convert Z value range(0.0,1.0) to GameCube
    if( Z3D < 0.0f )
        return 0.0f;
    if( Z3D > 1.0f )
        return DRAW_Z_CONV;

    return Z3D * DRAW_Z_CONV;
}

//==========================================================================

void DRAW_SetL2W( matrix4* VertexL2W, matrix4* NormalL2W )
{
    ASSERT( ENG_GetRenderMode() );

    if( VertexL2W == NULL ) s_VertexL2W.Identity();
    else                    s_VertexL2W = *VertexL2W;

    if( NormalL2W == NULL ) s_NormalL2W.Identity();
    else                    s_NormalL2W = *NormalL2W;


    //--- set transform matrix here on the rendering hardware
    matrix4 W2V, L2V;
    Mtx     GC_L2V;
    Mtx     GC_NRM;

    ENG_GetActiveView()->GetW2VMatrix ( W2V );

    //--- create the local to view matrix
    L2V = W2V * s_VertexL2W;

    //--- rotate 180 degrees for gamecube viewing(done by negating rows 0 & 2),
    //    and copy matrix data into gamecube specific matrix
    MTXRowCol( GC_L2V, 0, 0 ) = -L2V.M[0][0];
    MTXRowCol( GC_L2V, 1, 0 ) =  L2V.M[0][1];
    MTXRowCol( GC_L2V, 2, 0 ) = -L2V.M[0][2];
    MTXRowCol( GC_L2V, 0, 1 ) = -L2V.M[1][0];
    MTXRowCol( GC_L2V, 1, 1 ) =  L2V.M[1][1];
    MTXRowCol( GC_L2V, 2, 1 ) = -L2V.M[1][2];
    MTXRowCol( GC_L2V, 0, 2 ) = -L2V.M[2][0];
    MTXRowCol( GC_L2V, 1, 2 ) =  L2V.M[2][1];
    MTXRowCol( GC_L2V, 2, 2 ) = -L2V.M[2][2];
    MTXRowCol( GC_L2V, 0, 3 ) = -L2V.M[3][0];
    MTXRowCol( GC_L2V, 1, 3 ) =  L2V.M[3][1];
    MTXRowCol( GC_L2V, 2, 3 ) = -L2V.M[3][2];

    MTXInvXpose( GC_L2V, GC_NRM );

    //--- G_L2V can also be used for the normal matrix(unless there's non-uniform scaling)
    GXLoadNrmMtxImm( GC_NRM, GX_PNMTX6 );
    GXLoadPosMtxImm( GC_L2V, GX_PNMTX6 );
    GXSetCurrentMtx( GX_PNMTX6 );
}

//==========================================================================

void DRAW_GetL2W( matrix4& VertexL2W )
{
    VertexL2W = s_VertexL2W;
}

//==========================================================================

void DRAW_SetMode( u32 DrawMode )
{
    ASSERT( ENG_GetRenderMode() );

    //
    // Check if all draw modes have been chosen
    //
    if( (DrawMode & 0x01111111) != 0x01111111 )
    {
        if( (DrawMode & (DRAW_2D|DRAW_3D)) == 0 )
        { ASSERTS( FALSE, "DRAW_SetMode : 2D or 3D not specified" ); }

        if( (DrawMode & (DRAW_FILL|DRAW_NO_FILL)) == 0 )
        { ASSERTS( FALSE, "DRAW_SetMode : FILL or NO_FILL not specified" ); }

        if( (DrawMode & (DRAW_TEXTURE|DRAW_NO_TEXTURE)) == 0 )
        { ASSERTS( FALSE, "DRAW_SetMode : TEXTURE or NO_TEXTURE not specified" ); }

        if( (DrawMode & (DRAW_ALPHA|DRAW_NO_ALPHA)) == 0 )
        { ASSERTS( FALSE, "DRAW_SetMode : ALPHA or NO_ALPHA not specified" ); }

        if( (DrawMode & (DRAW_ZBUFFER|DRAW_NO_ZBUFFER)) == 0 )
        { ASSERTS( FALSE, "DRAW_SetMode : ZBUFFER or NO_ZBUFFER not specified" ); }

        if( (DrawMode & (DRAW_CLIP|DRAW_NO_CLIP)) == 0 )
        { ASSERTS( FALSE, "DRAW_SetMode : CLIP or NO_CLIP not specified" ); }

        if( (DrawMode & (DRAW_LIGHT|DRAW_NO_LIGHT)) == 0 )
        { ASSERTS( FALSE, "DRAW_SetMode : LIGHT or NO_LIGHT not specified" ); }
    }

    //
    // Preserve draw mode
    //
    s_DrawMode = DrawMode;


    //--- Z mode and Alpha mode settings
    u32 ZTestFlag;
    u32 AlphaZFill;

    if( (s_DrawMode & DRAW_ZBUFFER) == DRAW_ZBUFFER )
        ZTestFlag = ENG_ZBUFFER_TEST_ON;
    else
        ZTestFlag = ENG_ZBUFFER_TEST_OFF;

    if( (s_DrawMode & DRAW_ALPHA) == DRAW_ALPHA )
        AlphaZFill = ENG_ZBUFFER_FILL_OFF | ENG_ALPHA_TEST_OFF | ENG_ALPHA_BLEND_ON;
    else
        AlphaZFill = ENG_ZBUFFER_FILL_ON | ENG_ALPHA_TEST_OFF | ENG_ALPHA_BLEND_OFF;

    ENG_SetRenderFlags( ZTestFlag | AlphaZFill );
    ENG_SetBlendMode( ENG_BLEND_NORMAL );

    s_bUsingFixedAlpha = FALSE;

    //--- fill mode setting(GAMECUBE NOT SUPPORTED)
    //if ( (s_DrawMode & DRAW_FILL) == DRAW_FILL )

    //--- texturing on/off setting
    if ( (s_DrawMode & DRAW_TEXTURE) == DRAW_TEXTURE )
    {
        SET_DRAW_TEXTURE_ON();
    }
    else
    {
        SET_DRAW_TEXTURE_OFF();
    }

    //--- lighting on/off setting
    if ( (s_DrawMode & DRAW_LIGHT) == DRAW_LIGHT )
    {
        GC_EnableLighting( TRUE, FALSE );
    }
    else
    {
        GC_EnableLighting( FALSE, FALSE );
    }

}

//==========================================================================

void DRAW_SetTexture( x_bitmap* Texture, xbool bActivate )
{
    ASSERT( ENG_GetRenderMode() );

    s_pActiveTexture = Texture;

    if( Texture == NULL )
    {
        //VRAM_Deactivate();
        return;
    }

    if( bActivate )
        VRAM_Activate( *Texture );

    UPDATE_DRAW_TEXTURE();
}


////////////////////////////////////////////////////////////////////////////
// PRIMITIVE RENDERING FUNCTIONS
////////////////////////////////////////////////////////////////////////////

void DRAW_Points( s32 nVerts, vector3* Pos, color* Color )
{
    s32 i;

    ASSERT( Pos != NULL );

    //--- save current projection matrix and set up an orthographic one(for 2D draw)
    Mtx44 MSavedProj;

    if( DRAW_IsMode2D() )
    {
        DRAW_Setup2DView( MSavedProj );
    }

    UPDATE_DRAW_TEXTURE();
    DRAW_SetupFixedAlpha();

    //--- Setup Vertex Format (FMT7 always used by DRAW_*)
    GXClearVtxDesc ();
    GXSetVtxDesc   ( GX_VA_POS, GX_DIRECT );
    GXSetVtxAttrFmt( GX_VTXFMT7, GX_VA_POS, GX_POS_XYZ, GX_F32, 0 );

    if( Color != NULL )
    {
        GXSetVtxDesc   ( GX_VA_CLR0, GX_DIRECT );
        GXSetVtxAttrFmt( GX_VTXFMT7, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0 );
    }

    //--- Draw the points
    GXBegin( GX_POINTS, GX_VTXFMT7, nVerts );
    if( Color != NULL )
    {
        if( DRAW_IsMode2D() )
        {
            for( i = 0; i < nVerts; i++ )
            {
                GXPosition3f32( Pos[i].X, Pos[i].Y, DRAW_Get2DZ(Pos[i].Z) );
                GXColor4u8    ( Color[i].R, Color[i].G, Color[i].B, Color[i].A );
            }
        }
        else
        {   for( i = 0; i < nVerts; i++ )
            {
                GXPosition3f32( Pos[i].X, Pos[i].Y, Pos[i].Z );
                GXColor4u8    ( Color[i].R, Color[i].G, Color[i].B, Color[i].A );
            }
        }
    }
    else
    {   if( DRAW_IsMode2D() )
        {
            for( i = 0; i < nVerts; i++ )
                GXPosition3f32( Pos[i].X, Pos[i].Y, DRAW_Get2DZ(Pos[i].Z) );
        }
        else
        {   for( i = 0; i < nVerts; i++ )
                GXPosition3f32( Pos[i].X, Pos[i].Y, Pos[i].Z );
        }
    }
    GXEnd();

    if( DRAW_IsMode2D() )
        DRAW_RestoreProj( MSavedProj );
}

//==========================================================================

static void GC_DRAW_Lines( GXPrimitive PrimType, s32 nVerts, vector3* Pos, color* Color )
{
    s32 i;

    ASSERT( nVerts >= 2 );
    ASSERT( Pos != NULL );

    //--- save current projection matrix and set up an orthographic one(for 2D draw)
    Mtx44 MSavedProj;

    if( DRAW_IsMode2D() )
    {
        DRAW_Setup2DView( MSavedProj );
    }

    UPDATE_DRAW_TEXTURE();
    DRAW_SetupFixedAlpha();

    //--- Setup Vertex Format (FMT7 always used by DRAW_*)
    GXClearVtxDesc ();
    GXSetVtxDesc   ( GX_VA_POS, GX_DIRECT );
    GXSetVtxAttrFmt( GX_VTXFMT7, GX_VA_POS, GX_POS_XYZ, GX_F32, 0 );

    if( Color != NULL )
    {
        GXSetVtxDesc   ( GX_VA_CLR0, GX_DIRECT );
        GXSetVtxAttrFmt( GX_VTXFMT7, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0 );
    }

    //--- Draw lines
    GXBegin( PrimType, GX_VTXFMT7, nVerts );
    if( Color != NULL )
    {
        if( DRAW_IsMode2D() )
        {
            for( i = 0; i < nVerts; i++ )
            {
                GXPosition3f32( Pos[i].X, Pos[i].Y, DRAW_Get2DZ(Pos[i].Z) );
                GXColor4u8    ( Color[i].R, Color[i].G, Color[i].B, Color[i].A );
            }
        }
        else
        {   for( i = 0; i < nVerts; i++ )
            {
                GXPosition3f32( Pos[i].X, Pos[i].Y, Pos[i].Z );
                GXColor4u8    ( Color[i].R, Color[i].G, Color[i].B, Color[i].A );
            }
        }
    }
    else
    {   if( DRAW_IsMode2D() )
        {
            for( i = 0; i < nVerts; i++ )
                GXPosition3f32( Pos[i].X, Pos[i].Y, DRAW_Get2DZ(Pos[i].Z) );
        }
        else
        {
            for( i = 0; i < nVerts; i++ )
                GXPosition3f32( Pos[i].X, Pos[i].Y, Pos[i].Z );
        }
    }
    GXEnd();

    if( DRAW_IsMode2D() )
        DRAW_RestoreProj( MSavedProj );
}

//==========================================================================

void DRAW_Lines( s32 nVerts, vector3* Pos, color* Color )
{
    GC_DRAW_Lines( GX_LINES, nVerts, Pos, Color );
}

//==========================================================================

void DRAW_LineStrip( s32 nVerts, vector3* Pos, color* Color )
{
    GC_DRAW_Lines( GX_LINESTRIP, nVerts, Pos, Color );
}

//==========================================================================

static void GC_DRAW_Triangles( GXPrimitive PrimType, s32 nVerts, vector3* Pos, color* Color, vector2* UV, vector3* Normal )
{
    s32 i;

    ASSERT( nVerts >= 3 );
    ASSERT( Pos != NULL );

    //--- save current projection matrix and set up an orthographic one(for 2D draw)
    Mtx44 MSavedProj;

    if( DRAW_IsMode2D() )
    {
        DRAW_Setup2DView( MSavedProj );
    }

    UPDATE_DRAW_TEXTURE();
    DRAW_SetupFixedAlpha();

    //--- Setup Vertex Format (FMT7 always used by DRAW_*)
    GXClearVtxDesc ();
    GXSetVtxDesc   ( GX_VA_POS, GX_DIRECT );
    GXSetVtxAttrFmt( GX_VTXFMT7, GX_VA_POS, GX_POS_XYZ, GX_F32, 0 );

    if( Normal != NULL )
    {
        GXSetVtxDesc   ( GX_VA_NRM, GX_DIRECT );
        GXSetVtxAttrFmt( GX_VTXFMT7, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0 );
    }
    if( Color != NULL )
    {
        GXSetVtxDesc   ( GX_VA_CLR0, GX_DIRECT );
        GXSetVtxAttrFmt( GX_VTXFMT7, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0 );
    }
    if( UV != NULL )
    {
        GXSetVtxDesc   ( GX_VA_TEX0, GX_DIRECT );
        GXSetVtxAttrFmt( GX_VTXFMT7, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0 );
    }

    //--- Draw the polys
    GXBegin( PrimType, GX_VTXFMT7, nVerts );
    if( DRAW_IsMode2D() )
    {
        //2D DRAW has special Z value checks
        for( i = 0; i < nVerts; i++ )
        {
            GXPosition3f32( Pos[i].X, Pos[i].Y, DRAW_Get2DZ(Pos[i].Z) );

            if( Normal != NULL ) GXNormal3f32  ( Normal[i].X, Normal[i].Y, Normal[i].Z );
            if( Color  != NULL ) GXColor4u8    ( Color[i].R, Color[i].G, Color[i].B, Color[i].A );
            if( UV     != NULL ) GXTexCoord2f32( UV[i].X, UV[i].Y );
        }
    }
    else
    {
        for( i = 0; i < nVerts; i++ )
        {
            GXPosition3f32( Pos[i].X, Pos[i].Y, Pos[i].Z );

            if( Normal != NULL ) GXNormal3f32  ( Normal[i].X, Normal[i].Y, Normal[i].Z );
            if( Color  != NULL ) GXColor4u8    ( Color[i].R, Color[i].G, Color[i].B, Color[i].A );
            if( UV     != NULL ) GXTexCoord2f32( UV[i].X, UV[i].Y );
        }
    }
    GXEnd();

    if( DRAW_IsMode2D() )
        DRAW_RestoreProj( MSavedProj );
}

//==========================================================================

void DRAW_Triangles( s32 nVerts, vector3* Pos, color* Color, vector2* UV, vector3* Normal )
{
    GC_DRAW_Triangles( GX_TRIANGLES, nVerts, Pos, Color, UV, Normal );
}

//==========================================================================

void DRAW_TriangleStrip( s32 nVerts, vector3* Pos, color* Color, vector2* UV, vector3* Normal )
{
    GC_DRAW_Triangles( GX_TRIANGLESTRIP, nVerts, Pos, Color, UV, Normal );
}

//==========================================================================

void DRAW_TriangleFan( s32 nVerts, vector3* Pos, color* Color, vector2* UV, vector3* Normal )
{
    GC_DRAW_Triangles( GX_TRIANGLEFAN, nVerts, Pos, Color, UV, Normal );
}

//==========================================================================

void DRAW_Triangles2D( s32 nVerts, vector3* Pos, color* Color, vector2* UV )
{
    u32 SavedDrawMode = s_DrawMode; 

    s_DrawMode &= ~DRAW_3D;
    s_DrawMode |= DRAW_2D;

    GC_DRAW_Triangles( GX_TRIANGLES, nVerts, Pos, Color, UV, NULL );

    s_DrawMode = SavedDrawMode;
}

//==========================================================================

void DRAW_Sprite( f32 X, f32 Y, f32 Z,    // Hot spot (2D Left-Top), (3D Center)
                  f32 W, f32 H,           // (2D pixel W&H), (3D World W&H)
                  color  Color )          // color
{
    DRAW_SpriteUV( X, Y, Z, W, H, 0.0f, 0.0f, 1.0f, 1.0f, Color );
}

//==========================================================================

void DRAW_SpriteUV( f32 X,  f32 Y, f32 Z,   // Hot spot (2D Left-Top), (3D Center)
                    f32 W,  f32 H,          // (2D pixel W&H), (3D World W&H)
                    f32 U0, f32 V0,         // Upper Left   UV  [0.0 - 1.0]
                    f32 U1, f32 V1,         // Bottom Right UV  [0.0 - 1.0]
                    color   Color )         // color
{
    //--- save current projection matrix and set up an orthographic one(for 2D draw)
    Mtx44   MSavedProj;
    vector3 vPos[4];

    if( DRAW_IsMode2D() )
    {
        DRAW_Setup2DView( MSavedProj );

        Z = DRAW_Get2DZ( Z );

        vPos[0].Set( X,   Y,   Z );
        vPos[1].Set( X+W, Y,   Z );
        vPos[2].Set( X+W, Y+H, Z );
        vPos[3].Set( X,   Y+H, Z );
    }
    else
    {
        matrix4 V2W;
        ENG_GetActiveView()->GetV2WMatrix( V2W );
        V2W.SetTranslation( vector3( X, Y, Z ) );

        vPos[0].Set(  W*0.5f,  H*0.5f, 0 ); //TL
        vPos[1].Set( -W*0.5f,  H*0.5f, 0 ); //TR
        vPos[2].Set( -W*0.5f, -H*0.5f, 0 ); //BR
        vPos[3].Set(  W*0.5f, -H*0.5f, 0 ); //BL

        vPos[0] = V2W.Transform( vPos[0] );
        vPos[1] = V2W.Transform( vPos[1] );
        vPos[2] = V2W.Transform( vPos[2] );
        vPos[3] = V2W.Transform( vPos[3] );
    }

    UPDATE_DRAW_TEXTURE();
    DRAW_SetupFixedAlpha();

    //--- Setup vertex description and format
    GXClearVtxDesc();
    GXSetVtxDesc  ( GX_VA_POS,  GX_DIRECT );
    GXSetVtxDesc  ( GX_VA_CLR0, GX_DIRECT );
    GXSetVtxDesc  ( GX_VA_TEX0, GX_DIRECT );

    GXSetVtxAttrFmt( GX_VTXFMT7, GX_VA_POS,  GX_POS_XYZ,  GX_F32,   0 );
    GXSetVtxAttrFmt( GX_VTXFMT7, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0 );
    GXSetVtxAttrFmt( GX_VTXFMT7, GX_VA_TEX0, GX_TEX_ST,   GX_F32,   0 );

    //--- Draw the sprite
    GXBegin( GX_QUADS, GX_VTXFMT7, 4 );
    {
        //Top Left
        GXPosition3f32( vPos[0].X, vPos[0].Y, vPos[0].Z );
        GXColor4u8    ( Color.R, Color.G, Color.B, Color.A );
        GXTexCoord2f32( U0, V0 );

        //Top Right
        GXPosition3f32( vPos[1].X, vPos[1].Y, vPos[1].Z );
        GXColor4u8    ( Color.R, Color.G, Color.B, Color.A );
        GXTexCoord2f32( U1, V0 );

        //Bottom Right
        GXPosition3f32( vPos[2].X, vPos[2].Y, vPos[2].Z );
        GXColor4u8    ( Color.R, Color.G, Color.B, Color.A );
        GXTexCoord2f32( U1, V1 );

        //Bottom Left
        GXPosition3f32( vPos[3].X, vPos[3].Y, vPos[3].Z );
        GXColor4u8    ( Color.R, Color.G, Color.B, Color.A );
        GXTexCoord2f32( U0, V1 );
    }
    GXEnd();

    if( DRAW_IsMode2D() )
        DRAW_RestoreProj( MSavedProj );
}

//==========================================================================

void DRAW_Rectangle( f32 X, f32 Y, f32 Z,    // Hot spot (2D Left-Top), (3D Center)
                     f32 W, f32 H,           // (2D pixel W&H), (3D World W&H)
                     color  TLColor,         // Top Left     color
                     color  TRColor,         // Top Right    color
                     color  BLColor,         // Bottom Left  color
                     color  BRColor )        // Bottom Right color
{
    //--- save current projection matrix and set up an orthographic one(for 2D draw)
    Mtx44   MSavedProj;
    vector3 vPos[4];

    if( DRAW_IsMode2D() )
    {
        DRAW_Setup2DView( MSavedProj );

        Z = DRAW_Get2DZ( Z );

        vPos[0].Set( X,   Y,   Z );
        vPos[1].Set( X+W, Y,   Z );
        vPos[2].Set( X+W, Y+H, Z );
        vPos[3].Set( X,   Y+H, Z );
    }
    else
    {
        matrix4 V2W;
        ENG_GetActiveView()->GetV2WMatrix( V2W );
        V2W.SetTranslation( vector3( X, Y, Z ) );

        vPos[0].Set(  W*0.5f,  H*0.5f, 0 ); //TL
        vPos[1].Set( -W*0.5f,  H*0.5f, 0 ); //TR
        vPos[2].Set( -W*0.5f, -H*0.5f, 0 ); //BR
        vPos[3].Set(  W*0.5f, -H*0.5f, 0 ); //BL

        vPos[0] = V2W.Transform( vPos[0] );
        vPos[1] = V2W.Transform( vPos[1] );
        vPos[2] = V2W.Transform( vPos[2] );
        vPos[3] = V2W.Transform( vPos[3] );
    }

    UPDATE_DRAW_TEXTURE();
    DRAW_SetupFixedAlpha();

    //--- Setup vertex description and format
    GXClearVtxDesc();
    GXSetVtxDesc  ( GX_VA_POS,  GX_DIRECT );
    GXSetVtxDesc  ( GX_VA_CLR0, GX_DIRECT );

    GXSetVtxAttrFmt( GX_VTXFMT7, GX_VA_POS,  GX_POS_XYZ,  GX_F32,   0 );
    GXSetVtxAttrFmt( GX_VTXFMT7, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0 );

    //--- Draw the rectangle
    GXBegin( GX_QUADS, GX_VTXFMT7, 4 );
    {
        //Top Left
        GXPosition3f32( vPos[0].X, vPos[0].Y, vPos[0].Z );
        GXColor4u8    ( TLColor.R, TLColor.G, TLColor.B, TLColor.A );

        //Top Right
        GXPosition3f32( vPos[1].X, vPos[1].Y, vPos[1].Z );
        GXColor4u8    ( TRColor.R, TRColor.G, TRColor.B, TRColor.A );

        //Bottom Right
        GXPosition3f32( vPos[2].X, vPos[2].Y, vPos[2].Z );
        GXColor4u8    ( BRColor.R, BRColor.G, BRColor.B, BRColor.A );

        //Bottom Left
        GXPosition3f32( vPos[3].X, vPos[3].Y, vPos[3].Z );
        GXColor4u8    ( BLColor.R, BLColor.G, BLColor.B, BLColor.A );
    }
    GXEnd();

    if( DRAW_IsMode2D() )
        DRAW_RestoreProj( MSavedProj );
}

//==========================================================================

void DRAW_Input( void )
{

}

//==========================================================================

void DRAW_SetStatTrackers( s32* /*pNVerts*/, s32* /*pNTris*/, s32* /*pNBytes*/ )
{

}

//==========================================================================

void DRAW_BeginTriangles( void )
{
    //TODO: Implement this on GameCube???
}

//==========================================================================

void DRAW_Begin2DTriangles( void )
{
    //TODO: Implement this on GameCube???
}

//==========================================================================

void DRAW_EndTriangles( void )
{
    //TODO: Implement this on GameCube???
}

//==========================================================================

void DRAW_BeginSprite( void )
{
    //TODO: Implement this on GameCube???
}

//==========================================================================

void DRAW_EndSprite( void )
{
    //TODO: Implement this on GameCube???
}

//==========================================================================
