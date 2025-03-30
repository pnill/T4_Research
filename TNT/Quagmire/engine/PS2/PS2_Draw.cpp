//=========================================================================
//
//  PS2_DRAW.CPP
//
// Old TnT
//=========================================================================
#include <stdlib.h>
#include <eekernel.h>
#include <eeregs.h>
#include <math.h>
#include <libdma.h>
#include <libgraph.h>
#include <sifdev.h>
#include <libvu0.h>
#include <stdio.h>

#include "Q_Draw.hpp"
#include "Q_Engine.hpp"
#include "Q_PS2.hpp"
#include "Q_VRAM.hpp"
#include "Q_SMem.hpp"
#include "x_plus.hpp"
#include "x_debug.hpp"
#include "x_stdio.hpp"

#include "dmahelp.hpp"
#include "vifhelp.hpp"
#include "dvtrace.hpp"

//==========================================================================
// DEFINES
//==========================================================================

#define DRAW_USES_VU0

#define CLIPFLAG_XMIN       (1<<0)
#define CLIPFLAG_XMAX       (1<<1)
#define CLIPFLAG_YMIN       (1<<2)
#define CLIPFLAG_YMAX       (1<<3)
#define CLIPFLAG_ZMIN       (1<<4)
#define CLIPFLAG_ZMAX       (1<<5)

//=========================================================================
// TYPES
//=========================================================================

typedef struct
{
    f32 X,Y,Z,W;
} fvec4;

typedef struct
{
    s32 X,Y,Z,W;
} ivec4;

//=========================================================================
// VARIABLES
//=========================================================================

static u32          s_DrawMode;
static matrix4      s_VertexL2W __attribute__ ((aligned(16)));
static matrix4      s_NormalL2W __attribute__ ((aligned(16)));
static matrix4      s_VertexL2S __attribute__ ((aligned(16)));
static matrix4      s_VertexL2C __attribute__ ((aligned(16)));
static matrix4      s_VertexC2S __attribute__ ((aligned(16)));
static matrix4      s_LightDir;
static matrix4      s_LightColor;
static x_bitmap*    s_pActiveTexture;
static s32*         s_pStatNVerts = NULL;
static s32*         s_pStatNTris = NULL;
static s32*         s_pStatNBytes = NULL;

//=========================================================================
//=========================================================================
//=========================================================================
// INCLUDE TRANSFORM AND RENDER FUNCTIONS
//=========================================================================
//=========================================================================
//=========================================================================
#include "PS2_DrawTrans.cpp"
#include "PS2_DrawPoints.cpp"
#include "PS2_DrawLines.cpp"
#include "PS2_DrawTris.cpp"
#include "PS2_DrawTrisClip.cpp"
#include "PS2_DrawSprites.cpp"
#include "PS2_DrawRects.cpp"

void ENG_GetLightDirAndColor( matrix4& LDir, matrix4& LColor, s32 iSet = 0 );

//==========================================================================
//
//==========================================================================
//==========================================================================
// ASSEMBLY VERSION OF MATRIX MULTIPLY
//==========================================================================

#if defined(DRAW_USES_VU0)
inline void AsmM4_Build_L2W_L2C( matrix4& rDestL2S, matrix4& rDestL2C, 
                                 matrix4& rSrcW2S,  matrix4& rSrcW2C, matrix4& rSrcL2W )
{

    // VU0 Assembly version:
    asm __volatile__
    ("
        LQC2    vf10, 0x00(%2)  # load the SrcW2S matrix vectors into vf10..vf13
        LQC2    vf11, 0x10(%2)
        LQC2    vf12, 0x20(%2)
        LQC2    vf13, 0x30(%2)

        LQC2    vf14, 0x00(%3)  # load the SrcW2C matrix vectors into vf14..vf17
        LQC2    vf15, 0x10(%3)
        LQC2    vf16, 0x20(%3)
        LQC2    vf17, 0x30(%3)

        LQC2    vf18, 0x00(%4)  # load the SrcL2W matrix vectors into vf18..vf21
        LQC2    vf19, 0x10(%4)
        LQC2    vf20, 0x20(%4)
        LQC2    vf21, 0x30(%4)


        VMULAx.xyzw     ACC,    vf10,   vf18x       # do the first column
        VMADDAy.xyzw    ACC,    vf11,   vf18y
        VMADDAz.xyzw    ACC,    vf12,   vf18z
        VMADDw.xyzw    vf02,    vf13,   vf18w

        VMULAx.xyzw     ACC,    vf10,   vf19x       # do the second column
        VMADDAy.xyzw    ACC,    vf11,   vf19y
        VMADDAz.xyzw    ACC,    vf12,   vf19z
        VMADDw.xyzw    vf03,    vf13,   vf19w

        VMULAx.xyzw     ACC,    vf10,   vf20x       # do the third column
        VMADDAy.xyzw    ACC,    vf11,   vf20y
        VMADDAz.xyzw    ACC,    vf12,   vf20z
        VMADDw.xyzw    vf04,    vf13,   vf20w

        VMULAx.xyzw     ACC,    vf10,   vf21x       # do the fourth column
        VMADDAy.xyzw    ACC,    vf11,   vf21y
        VMADDAz.xyzw    ACC,    vf12,   vf21z
        VMADDw.xyzw    vf05,    vf13,   vf21w


        VMULAx.xyzw     ACC,    vf14,   vf18x       # do the first column
        VMADDAy.xyzw    ACC,    vf15,   vf18y
        VMADDAz.xyzw    ACC,    vf16,   vf18z
        VMADDw.xyzw    vf06,    vf17,   vf18w

        VMULAx.xyzw     ACC,    vf14,   vf19x       # do the second column
        VMADDAy.xyzw    ACC,    vf15,   vf19y
        VMADDAz.xyzw    ACC,    vf16,   vf19z
        VMADDw.xyzw    vf07,    vf17,   vf19w

        VMULAx.xyzw     ACC,    vf14,   vf20x       # do the third column
        VMADDAy.xyzw    ACC,    vf15,   vf20y
        VMADDAz.xyzw    ACC,    vf16,   vf20z
        VMADDw.xyzw    vf08,    vf17,   vf20w

        VMULAx.xyzw     ACC,    vf14,   vf21x       # do the fourth column
        VMADDAy.xyzw    ACC,    vf15,   vf21y
        VMADDAz.xyzw    ACC,    vf16,   vf21z
        VMADDw.xyzw    vf09,    vf17,   vf21w

        SQC2    vf02, 0x00(%0)        # store the result L2S
        SQC2    vf03, 0x10(%0)
        SQC2    vf04, 0x20(%0)
        SQC2    vf05, 0x30(%0)

        SQC2    vf06, 0x00(%1)        # store the result L2C
        SQC2    vf07, 0x10(%1)
        SQC2    vf08, 0x20(%1)
        SQC2    vf09, 0x30(%1)

    ": "+r" (&rDestL2S) , "+r" (&rDestL2C) : "r" (&rSrcW2S) , "r" (&rSrcW2C) , "r" (&rSrcL2W));
}
#endif

//=========================================================================
// FUNCTIONS
//=========================================================================

void DRAW_SetStatTrackers( s32* pNVerts, s32* pNTris, s32* pNBytes )
{
    s_pStatNVerts = pNVerts;
    s_pStatNTris = pNTris;
    s_pStatNBytes = pNBytes;
}

//==========================================================================

void DRAW_Init( void )
{
    InitTris();
    InitPoints();
    InitLines();
    InitSprites();
    InitRects();
}

//=========================================================================

void DRAW_SetL2W( matrix4* VertexL2W, matrix4* NormalL2W )
{
//    matrix4 W2S __attribute__ ((aligned(16)));
//    matrix4 W2C __attribute__ ((aligned(16)));
//    matrix4 LD;
//    matrix4 LC;

    ASSERT( ENG_GetRenderMode() );

    ////////////////////////////////////////////////////////////////////////
    //  Set L2W and build/get L2S, L2C, C2S
    ////////////////////////////////////////////////////////////////////////
    if( VertexL2W == NULL )
    {
        s_VertexL2W.Identity();
        ENG_GetW2S( s_VertexL2S );
        ENG_GetW2C( s_VertexL2C );
    }
    else
    {
        s_VertexL2W = *VertexL2W;
        ENG_GetW2S( s_VertexL2S );
        ENG_GetW2C( s_VertexL2C );

        ////////////////////////////////////////////////////////////////////
        //  Transform L2S L2C by L2W
        ////////////////////////////////////////////////////////////////////
        #if defined(DRAW_USES_VU0)
            AsmM4_Build_L2W_L2C( s_VertexL2S, s_VertexL2C, s_VertexL2S, s_VertexL2C, s_VertexL2W );
        #else
            s_VertexL2S = s_VertexL2S * s_VertexL2W;
            s_VertexL2C = s_VertexL2C * s_VertexL2W;
        #endif
    }

    ENG_GetC2S( s_VertexC2S );

    ////////////////////////////////////////////////////////////////////////
    //  Get Normal L2W and build light matrices
    ////////////////////////////////////////////////////////////////////////
    ENG_GetLightDirAndColor( s_LightDir, s_LightColor );
    s_LightColor.M[0][0] *= 0.5f;
    s_LightColor.M[0][1] *= 0.5f;
    s_LightColor.M[0][2] *= 0.5f;
    s_LightColor.M[0][3] *= 0.5f;
    s_LightColor.M[1][0] *= 0.5f;
    s_LightColor.M[1][1] *= 0.5f;
    s_LightColor.M[1][2] *= 0.5f;
    s_LightColor.M[1][3] *= 0.5f;
    s_LightColor.M[2][0] *= 0.5f;
    s_LightColor.M[2][1] *= 0.5f;
    s_LightColor.M[2][2] *= 0.5f;
    s_LightColor.M[2][3] *= 0.5f;
    s_LightColor.M[3][0] *= 0.5f;
    s_LightColor.M[3][1] *= 0.5f;
    s_LightColor.M[3][2] *= 0.5f;
    s_LightColor.M[3][3] *= 0.5f;

    if( NormalL2W == NULL ) 
    {
        s_NormalL2W.Identity();
    }
    else
    {
        s_NormalL2W = *NormalL2W;
        s_LightDir   = s_LightDir * s_NormalL2W;
    }
}

//=========================================================================

typedef struct
{
    sceDmaTag   DMA;
    s32         VIF[4];
    sceGifTag   GIF;
    u64         AD[4];
} alpha_header;
/*
static
void SetAlphaBlending( void )
{
    alpha_header*   pHeader;
    
    pHeader = (alpha_header*)pDList;
    pDList += sizeof( alpha_header );

    // Build the dma command
    DMAHELP_BuildTagCont( &pHeader->DMA, sizeof(alpha_header)-sizeof(sceDmaTag) );

    // Build the VIF command
    pHeader->VIF[0] = 0;
    pHeader->VIF[1] = 0;
    pHeader->VIF[2] = 0;
    pHeader->VIF[3] = SCE_VIF1_SET_DIRECT( 2, 0 );

    // Set the appropriate GIF tag
    VIFHELP_BuildGifTag1( &pHeader->GIF,
                          VIFHELP_GIFMODE_PACKED,
                          1,
                          1,
                          FALSE,
                          0,
                          0,
                          TRUE );
    VIFHELP_BuildGifTag2( &pHeader->GIF,
                          VIFHELP_GIFREG_AD, 0, 0, 0 );

    // Set the alpha
    pHeader->AD[0] = SCE_GS_SET_ALPHA(0,1,0,1,1);
    pHeader->AD[1] = SCE_GS_ALPHA_1;
    pHeader->AD[2] = 0;
    pHeader->AD[3] = 0;
}
*/
//=========================================================================

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

    ENG_SetBlendMode(ENG_BLEND_NORMAL);
    ENG_SetRenderFlags( (( (s_DrawMode & DRAW_ZBUFFER) == DRAW_ZBUFFER )
                           ? ENG_ZBUFFER_TEST_ON : ENG_ZBUFFER_TEST_OFF ) |
                        (( (s_DrawMode & DRAW_ALPHA)   == DRAW_ALPHA   ) 
                           ? (ENG_ZBUFFER_FILL_OFF | ENG_ALPHA_TEST_OFF | ENG_ALPHA_BLEND_ON) : (ENG_ZBUFFER_FILL_ON | ENG_ALPHA_TEST_OFF | ENG_ALPHA_BLEND_OFF))
                      );
}

//=========================================================================

void DRAW_SetTexture( x_bitmap* Texture, xbool bActivate )
{
    ASSERT( ENG_GetRenderMode() );

    s_pActiveTexture = Texture;
    
    if( Texture==NULL )
        return;

    if (bActivate)
        VRAM_Activate( *Texture );
}

