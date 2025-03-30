//===========================================================================
// File: framebuf.c
// Date: 10 May 2000
// Author: Mark Breugelmans (Sony Computer Entertainment Europe)
// Description: Simple functions to setup display/draw environments
//              and to draw filtered sprites
//===========================================================================

#include <libgraph.h>
#include <libpkt.h>
#include "framebuf.hpp"
#include "x_color.hpp"
#include "vifhelp.hpp"

#include "Q_Engine.hpp"

#define SPR_MEM (0x70000000)
#define UNCACHED_MEM (0x20000000)
#define GIFTAG_EOP (1)
#define GIFTAG_PRE (1)
#define GIFTAG_MAX_NLOOP (32767)
#define GIFTAG_NLOOP (CUBE_VERT)
#define DISPBUF_BASE (0)

gsZBufferSprite ZTargetData __attribute__((aligned(64)));
gsZBufferSprite ZOnScreenData __attribute__((aligned(64)));
gsZBufferSprite ZSkyMaskData __attribute__((aligned(64)));
gsZBufferSprite DisplayData __attribute__((aligned(64)));

gsZBufferNoTexSprite ZNoTexData;
gsFogClutSend FogClut;

static s32 s_bBuildPackets = TRUE;

void ResetFogPackets( void )
{
    s_bBuildPackets = TRUE;
}


static color s_FogClut[256] __attribute__((aligned(16)));

void SetSpriteCoords( float u0, float v0, float u1, float v1, int x, int y, int w, int h, short center );

// Note: coord system will always be 2048,2048 centre (use this for scissor)
// Note: don't forget to set the half offset when swapping buffers

/*

Need to set the following registers (USE struct sceGsDrawEnv1):

FRAME_1     - Frame buffer (drawing buffer)
ZBUF_1      - Z buffer address
XY_OFFSET   - Half pixel offset
SCISSOR     - Scissor region
PRMODECONT  - Use / don't use prmode register
COLCLAMP    - Clamp or mask 8bit colour values
DTHE        - Dither on / off
DIMX        - Dither matrix
TEST_1      - Z test / Alpha test modes


Need to clear buffers( USE struct sceGSClear)

TEST        - Set draw mode
PRIM        - Set prim type(sprite)
RGBAQ       - Set clear colour
XYZ2        - Top left pos
XYZ2        - Bottom right pos
TEST        - Reset draw mode

Also need to set the following priviliged registers( USE struct sceGsDispEnv)

PMODE       - Choose read circuits
SMODE2      - Interlace, field/frame mode
DISPFB1     - Sets display buffer settings (format, addr, x, y)
DISPLAY1    - Sets display buffer position on TV (x, y, width, height)
BGCOLOR     - Sets background colour of PCRTC

And need to reset/sync the GS (USE sceGsResetGraph and sceGsSyncV functions)
CSR         - Resets the GS / syncV


*/


//=========================================================================
void Setup_Cheat_buffer( fsAABuff *buff, s16 buff1W, s16 buff1H, s16 buff1PSM,
                         s16 buff2W, s16 buff2H, s16 buff2PSM,
                         s16 ztest, s16 zPSM, s16 zClear, xbool bIsProgressive )
{
    buff->dispW = buff1W;
    buff->dispH = buff1H;
    buff->dispPSM = buff1PSM;

    buff->drawW = buff2W;
    buff->drawH = buff2H;
    buff->drawPSM = buff2PSM;

    buff->zPSM = zPSM;

    // Set the FrameBuffer positions
    buff->dispFBP   = 0;
    buff->drawFBP   = ((buff->dispW * buff->dispH)>>11);   
    if (buff1PSM == SCE_GS_PSMCT16 || buff1PSM == SCE_GS_PSMCT16S)
    {
        buff->drawFBP >>= 1;
        if (buff->dispH == 224 || buff->dispH == 480)
            buff->drawFBP += 5; // offset for half-height buffer (comes out to a half-height tpage)
    }

    buff->zFBP = buff->drawFBP + ((buff2W * buff2H)>>11);
    if (buff2PSM == SCE_GS_PSMCT16 || buff2PSM == SCE_GS_PSMCT16S)
        buff->zFBP = buff->drawFBP + ((buff2W * buff2H)>>12);

    SetDispBuffer_Cheat( buff, bIsProgressive );
    SetDrawBufferLarge_Cheat( buff, ztest, zClear );
    SetDrawBufferSmall_Cheat( buff, ztest, zClear );
}

//=========================================================================
void Setup_FSAA_buffer(fsAABuff *buff, s16 dispW, s16 dispH, s16 dispPSM,
                       s16 drawW, s16 drawH, s16 drawPSM, 
                       s16 ztest, s16 zPSM,
                       s16 clear, xbool bIsProgressive )
{
    buff->dispW = dispW;
    buff->dispH = dispH;
    buff->dispPSM = dispPSM;

    buff->drawW = drawW;
    buff->drawH = drawH;
    buff->drawPSM = drawPSM;

    buff->zPSM = zPSM;

    // Set the FrameBuffer positions
    buff->dispFBP   = 0;
    buff->drawFBP   = ((buff->dispW * buff->dispH)>>11);   
    if (dispPSM == SCE_GS_PSMCT16 || dispPSM == SCE_GS_PSMCT16S)
    {
        buff->drawFBP >>= 1;
        if (buff->dispH == 224 || buff->dispH == 480)
            buff->drawFBP += 5; // offset for half-height buffer (comes out to a half-height tpage)
    }

    buff->zFBP = buff->drawFBP + ((drawW * drawH)>>11);
    if (drawPSM == SCE_GS_PSMCT16 || drawPSM == SCE_GS_PSMCT16S)
        buff->zFBP = buff->drawFBP + ((drawW * drawH)>>12);

    SetDispBuffer( buff, buff->dispW, buff->dispH, buff->dispPSM, buff->dispFBP, bIsProgressive  );

    SetDrawBufferLarge( buff, buff->drawW, buff->drawH, buff->drawPSM, buff->drawFBP,
                        buff->zFBP, ztest, zPSM, clear);

    SetDrawBufferSmall( buff, buff->dispW, buff->dispH, buff->dispPSM, buff->dispFBP );
}

//=========================================================================

void SetDispBuffer( fsAABuff *buff, s16 w, s16 h, s16 psm, s16 fbp, xbool bIsProgressive )
{
    // No giftags needed (all memory mapped registers to set)
    sceGsSetDefDispEnv( (sceGsDispEnv*)&buff->disp0, psm&0xF, w, h, 0, 0 );
    buff->disp0.dispfb.FBP  = fbp; 

    // Buffer 2 ==================================================
    sceGsSetDefDispEnv( (sceGsDispEnv*)&buff->disp1, psm&0xF, w, h, 0, 0 );
    buff->disp1.dispfb.FBP  = fbp; 

    // 2nd circuit
    // Buffer 1
    *(u_long*)&buff->disp0.dispfb1  = *(u_long*)&buff->disp0.dispfb;
    *(u_long*)&buff->disp0.display1 = *(u_long*)&buff->disp0.display;  
    buff->disp0.dispfb.DBY          = 1;                                // dispfb2 has DBY field of 1
    buff->disp0.display.DH          = (buff->disp0.display1.DH) - 1;
    buff->disp0.display.DX          = buff->disp0.display.DX + 4;       // display2 has magh/2
    buff->disp0.pmode.MMOD          = 1;
    buff->disp0.pmode.AMOD          = 0;
    buff->disp0.pmode.EN1           = 1;
    buff->disp0.pmode.EN2           = 1;

    // Buffer 2 (second verse, same as the first, sort of )
    *(u_long*)&buff->disp1.dispfb1  = *(u_long*)&buff->disp1.dispfb;
    *(u_long*)&buff->disp1.display1 = *(u_long*)&buff->disp1.display;
    buff->disp1.dispfb.DBY          = 1;                                // dispfb2 has DBY field of 1
    buff->disp1.display.DH          = (buff->disp1.display1.DH) - 1;
    buff->disp1.display.DX          = buff->disp1.display.DX + 4;       // display2 has magh/2  
    buff->disp1.pmode.MMOD          = 1;
    buff->disp1.pmode.AMOD          = 0;
    buff->disp1.pmode.EN1           = 1;
    buff->disp1.pmode.EN2           = 1;


    if (!bIsProgressive)
    {
        // 2nd circuit
        buff->disp0.pmode.ALP           = 0x55;
        buff->disp1.pmode.ALP           = 0x55;
    }
    else
    {
        buff->disp0.pmode.ALP           = 0xFF;
        buff->disp1.pmode.ALP           = 0xFF;
    }
}

//=========================================================================

void SetDrawBufferLarge(fsAABuff *buff, s16 w, s16 h, s16 psm, s16 fbp, 
                        s16 zbp, s16 ztest, s16 zpsm, s16 clear)
{
    sceGsSetDefDrawEnv(&buff->drawLarge, psm&0xF, w, h, ztest, zpsm&0xF);

    // Fix the fbp values for the draw-buffer and the z-buffer
    buff->drawLarge.frame1.FBP = fbp; 
    if (ztest == 0)
        *(u64 *)&buff->drawLarge.zbuf1 = SCE_GS_SET_ZBUF(zbp, zpsm&0xf, 1); 
    else
        *(u64 *)&buff->drawLarge.zbuf1 = SCE_GS_SET_ZBUF(zbp, zpsm&0xf, 0); 

    // Setup the giftag
    SCE_GIF_CLEAR_TAG(&buff->giftagDrawLarge);
    buff->giftagDrawLarge.NLOOP = (clear?14:8);  // (we need to clear the large draw buffer)
    buff->giftagDrawLarge.EOP   = 1;
    buff->giftagDrawLarge.NREG  = 1;
    buff->giftagDrawLarge.REGS0 = 0xe; // A_plus_D


    if (clear) 
    {
        // NOTE: last 5 numbers are: R, G, B, A, Z
        // NOTE: ztest is needed to re-enable the ztest mode after the clear
        sceGsSetDefClear(&buff->clearLarge, ztest, 2048-(w>>1), 2048-(h>>1), 
            w, h, 0, 0, 0, 0, 0); 
    }
}

//=========================================================================

void SetDrawBufferSmall(fsAABuff *buff, s16 w, s16 h, s16 psm, s16 fbp) // No Z Test
{
    sceGsSetDefDrawEnv(&buff->drawSmall, psm&0xF, w, h, SCE_GS_ZNOUSE, 0); // Last 2 param = ztest, zpsm

    // Fix fbp value for the draw buffer
    buff->drawSmall.frame1.FBP = fbp;

    *(u64 *)&buff->drawSmall.zbuf1 = SCE_GS_SET_ZBUF(0, 0&0xf, 1); 

    // Setup the giftag
    SCE_GIF_CLEAR_TAG(&buff->giftagDrawSmall);
    buff->giftagDrawSmall.NLOOP = 8; // Add 6 registers for clearing
    buff->giftagDrawSmall.EOP   = 1;
    buff->giftagDrawSmall.NREG  = 1;
    buff->giftagDrawSmall.REGS0 = 0xe; // A_plus_D
}

//=========================================================================

void SetDispBuffer_Cheat( fsAABuff *buff, xbool bIsProgressive )
{
    // No giftags needed (all memory mapped registers to set)
    sceGsSetDefDispEnv( (sceGsDispEnv*)&buff->disp0, buff->dispPSM&0xF, buff->dispW, buff->dispH, 0, 0 );
    buff->disp0.dispfb.FBP = buff->dispFBP; 

    // Buffer 2 ==================================================
    sceGsSetDefDispEnv( (sceGsDispEnv*)&buff->disp1, buff->drawPSM&0xF, buff->drawW, buff->drawH, 0, 0 );
    buff->disp1.dispfb.FBP = buff->drawFBP; 

    // 2nd circuit
    // Buffer 1
    *(u_long*)&buff->disp0.dispfb1  = *(u_long*)&buff->disp0.dispfb;
    *(u_long*)&buff->disp0.display1 = *(u_long*)&buff->disp0.display;  
    buff->disp0.dispfb.DBY          = 1;                                // dispfb2 has DBY field of 1
    buff->disp0.display.DH          = (buff->disp0.display1.DH) - 1;
    buff->disp0.display.DX          = buff->disp0.display.DX + 4;       // display2 has magh/2
    buff->disp0.pmode.MMOD          = 1;
    buff->disp0.pmode.AMOD          = 0;
    buff->disp0.pmode.EN1           = 1;
    buff->disp0.pmode.EN2           = 1;

    // Buffer 2 (second verse, same as the first, sort of )
    *(u_long*)&buff->disp1.dispfb1  = *(u_long*)&buff->disp1.dispfb;
    *(u_long*)&buff->disp1.display1 = *(u_long*)&buff->disp1.display;
    buff->disp1.dispfb.DBY          = 1;                                // dispfb2 has DBY field of 1
    buff->disp1.display.DH          = (buff->disp1.display1.DH) - 1;
    buff->disp1.display.DX          = buff->disp1.display.DX + 4;       // display2 has magh/2  
    buff->disp1.pmode.MMOD          = 1;
    buff->disp1.pmode.AMOD          = 0;
    buff->disp1.pmode.EN1           = 1;
    buff->disp1.pmode.EN2           = 1;


    if (!bIsProgressive)
    {
        // 2nd circuit
        buff->disp0.pmode.ALP           = 0x55;
        buff->disp1.pmode.ALP           = 0x55;
    }
    else
    {
        buff->disp0.pmode.ALP           = 0xFF;
        buff->disp1.pmode.ALP           = 0xFF;
    }
}

//=========================================================================

void SetDrawBufferLarge_Cheat( fsAABuff *buff, s16 ztest, s16 clear )
{
    sceGsSetDefDrawEnv(&buff->drawLarge, buff->drawPSM&0xF, buff->drawW, buff->drawH, ztest, buff->zPSM&0xF);

    // Fix the fbp values for the draw-buffer and the z-buffer
    buff->drawLarge.frame1.FBP = buff->drawFBP;
    if (ztest == 0)
        *(u64 *)&buff->drawLarge.zbuf1 = SCE_GS_SET_ZBUF( buff->zFBP, buff->zPSM&0xf, 1); 
    else
        *(u64 *)&buff->drawLarge.zbuf1 = SCE_GS_SET_ZBUF( buff->zFBP, buff->zPSM&0xf, 0); 

    // Setup the giftag
    SCE_GIF_CLEAR_TAG(&buff->giftagDrawLarge);
    buff->giftagDrawLarge.NLOOP = (clear?14:8);  // (we need to clear the large draw buffer)
    buff->giftagDrawLarge.EOP   = 1;
    buff->giftagDrawLarge.NREG  = 1;
    buff->giftagDrawLarge.REGS0 = 0xe; // A_plus_D


    if (clear) 
    {
        // NOTE: last 5 numbers are: R, G, B, A, Z
        // NOTE: ztest is needed to re-enable the ztest mode after the clear
        sceGsSetDefClear(&buff->clearLarge, ztest, 2048-(buff->drawW>>1), 2048-(buff->drawH>>1), 
            buff->drawW, buff->drawH, 0, 0, 0, 0, 0); 
    }
}

//=========================================================================

void SetDrawBufferSmall_Cheat( fsAABuff *buff, s16 ztest, s16 clear )
{
    sceGsSetDefDrawEnv(&buff->drawSmall, buff->dispPSM&0xF, buff->dispW, buff->dispH, ztest, buff->zPSM&0xF);

    // Fix the fbp values for the draw-buffer and the z-buffer
    buff->drawSmall.frame1.FBP = buff->dispFBP;
    if (ztest == 0)
        *(u64 *)&buff->drawSmall.zbuf1 = SCE_GS_SET_ZBUF( buff->zFBP, buff->zPSM&0xf, 1); 
    else
        *(u64 *)&buff->drawSmall.zbuf1 = SCE_GS_SET_ZBUF( buff->zFBP, buff->zPSM&0xf, 0); 

    // Setup the giftag
    SCE_GIF_CLEAR_TAG(&buff->giftagDrawSmall);
    buff->giftagDrawSmall.NLOOP = (clear?14:8);  // (we need to clear the large draw buffer)
    buff->giftagDrawSmall.EOP   = 1;
    buff->giftagDrawSmall.NREG  = 1;
    buff->giftagDrawSmall.REGS0 = 0xe; // A_plus_D

    if (clear) 
    {
        // NOTE: last 5 numbers are: R, G, B, A, Z
        // NOTE: ztest is needed to re-enable the ztest mode after the clear
        sceGsSetDefClear(&buff->clearSmall, ztest, 2048-(buff->dispW>>1), 2048-(buff->dispH>>1), 
            buff->dispW, buff->dispH, 0, 0, 0, 0, 0); 
    }
}

//=========================================================================

void PutDispBuffer(fsAABuff *buff, s32 bufferNum)
{
    if (bufferNum==0)
    {
        buff->disp0.pmode.EN1 = 1;
        buff->disp0.pmode.EN2 = 1;

        DPUT_GS_PMODE(*(u_long *)&buff->disp0.pmode);   //PMODE
        DPUT_GS_SMODE2(*(u_long *)&buff->disp0.smode2);  //SMODE2
        DPUT_GS_DISPFB2(*(u_long *)&buff->disp0.dispfb);  //DISPFB2
        DPUT_GS_DISPLAY2(*(u_long *)&buff->disp0.display); //DISPLAY2
        DPUT_GS_BGCOLOR(*(u_long *)&buff->disp0.bgcolor); //BGCOLOR

        DPUT_GS_DISPLAY1( *(u_long *)&buff->disp0.display1); //DISPLAY1
        DPUT_GS_DISPFB1(*(u_long *)&buff->disp0.dispfb1);  //DISPFB1
    }
    else
    {
        buff->disp1.pmode.EN1 = 1;
        buff->disp1.pmode.EN2 = 1;

        DPUT_GS_PMODE(*(u_long *)&buff->disp1.pmode);   //PMODE
        DPUT_GS_SMODE2(*(u_long *)&buff->disp1.smode2);  //SMODE2
        DPUT_GS_DISPFB2(*(u_long *)&buff->disp1.dispfb);  //DISPFB2
        DPUT_GS_DISPLAY2(*(u_long *)&buff->disp1.display); //DISPLAY2
        DPUT_GS_BGCOLOR(*(u_long *)&buff->disp1.bgcolor); //BGCOLOR

        DPUT_GS_DISPLAY1( *(u_long *)&buff->disp1.display1); //DISPLAY1
        DPUT_GS_DISPFB1(*(u_long *)&buff->disp1.dispfb1);  //DISPFB1
    }
}

//=========================================================================

void PutDrawBufferLarge(fsAABuff *buff)
{
    sceGsPutDrawEnv(&buff->giftagDrawLarge);
}

//=========================================================================

void PutDrawBufferSmall(fsAABuff *buff)
{
    sceGsPutDrawEnv(&buff->giftagDrawSmall);
}

//=========================================================================

static s32 mask = 0;
static
void BuildDownSampleSpriteUsingTarget( gsDrawDecalSprite *spriteStruct, 
                                       s16 SrcWidth, s16 SrcHeight, s16 SrcPSM, s16 SrcFBP,
                                       s16 DestWidth, s16 DestHeight, s16 DestPSM, s16 DestFBP,
                                       s32 filter, xbool bOffset )
{   
    s32 nWidth = SrcWidth>>6;
    f32 fXStep = ((f32)DestWidth / (f32)nWidth);

    x_memset( spriteStruct, 0, sizeof( gsDrawDecalSprite ) );
    VIFHELP_BuildGifTag1( &spriteStruct->GIFtag, VIFHELP_GIFMODE_PACKED, 1, (9 + (4 * nWidth)), FALSE, 0, 0, TRUE );
    VIFHELP_BuildGifTag2( &spriteStruct->GIFtag, VIFHELP_GIFREG_AD, 0, 0, 0 );

    static xbool bFlip = TRUE;
    if (bFlip)
        mask = 0x00F80000;
    else
        mask = 0x000000F8;
    mask = 0;
    spriteStruct->GSCmds[ 0 ] = SCE_GS_SET_FRAME_1( DestFBP<<5, (DestWidth>>6)&0x3F, DestPSM, mask );
    spriteStruct->GSCmds[ 1 ] = SCE_GS_FRAME_1;
    spriteStruct->GSCmds[ 2 ] = SCE_GS_SET_TEX1_1( 1, 0, 1, 1, 1, 0, 1 );
    spriteStruct->GSCmds[ 3 ] = SCE_GS_TEX1_1;
    s32 h = 9;
    if (SrcHeight > 512)
        h = 10;
    spriteStruct->GSCmds[ 4 ] = SCE_GS_SET_TEX0_1( SrcFBP<<5, (SrcWidth>>6)&0x3f, SrcPSM, 10, h, 1, 1, 0, 0, 0, 0, 0 ); 
    spriteStruct->GSCmds[ 5 ] = SCE_GS_TEX0_1;
    spriteStruct->GSCmds[ 6 ] = 0;
    spriteStruct->GSCmds[ 7 ] = SCE_GS_TEXFLUSH;
    spriteStruct->GSCmds[ 8 ] = SCE_GS_SET_PRIM( 6, 0, 1, 0, 0, 0, 1, 0, 0 );
    spriteStruct->GSCmds[ 9 ] = SCE_GS_PRIM;
    spriteStruct->GSCmds[ 10 ] = SCE_GS_SET_RGBAQ( 0x7F, 0x7F, 0x7F, 0x7F, 0x3F800000 );
    spriteStruct->GSCmds[ 11 ] = SCE_GS_RGBAQ;
    if (DestPSM == SCE_GS_PSMCT16 || DestPSM == SCE_GS_PSMCT16S)
        spriteStruct->GSCmds[ 12 ] = SCE_GS_SET_DTHE( 1 );
    else
        spriteStruct->GSCmds[ 12 ] = SCE_GS_SET_DTHE( 0 );
    spriteStruct->GSCmds[ 13 ] = SCE_GS_DTHE;
    if (bFlip)
        spriteStruct->GSCmds[ 14 ] = SCE_GS_SET_DIMX( 0, 1, 2, 3,
                                                      2, 3, 0, 1,
                                                      1, 0, 3, 2, 
                                                      3, 2, 1, 0 );
    else
        spriteStruct->GSCmds[ 14 ] = SCE_GS_SET_DIMX( 1, 0, 3, 2,
                                                      3, 2, 1, 01,
                                                      0, 1, 2, 3, 
                                                      2, 3, 0, 1 );
    bFlip = !bFlip;
    spriteStruct->GSCmds[ 15 ] = SCE_GS_DIMX;
    spriteStruct->GSCmds[ 16 ] = SCE_GS_SET_SCISSOR_1( 0, DestWidth-1, 0, DestHeight-1 );
    spriteStruct->GSCmds[ 17 ] = SCE_GS_SCISSOR_1;

    s32 UStep = 64<<4;
    s32 U0 = 0;
    s32 V0 = 0;
    s32 U1 = U0 + UStep - 1;
    s32 V1 = ((SrcHeight-1)<<4);
    if (bOffset)
    {
        U0 = 8;
        V0 = 8;
        U1 -= 8;
    }

    s32 XStep = (s32)(fXStep * 16.0f);
    s32 X0 = (2048-(DestWidth>>1))<<4;
    s32 X1 = X0 + (XStep-1);

    s32 Y0 = (2048-(DestHeight>>1))<<4;
    s32 Y1 = (2048+(DestHeight>>1))<<4;

    s32 x;
    for (x=0; x<nWidth; ++x)
    {
        spriteStruct->SpriteData[ x ].GSData[ 0 ] = SCE_GS_SET_UV( U0, V0 );
        spriteStruct->SpriteData[ x ].GSData[ 1 ] = SCE_GS_UV;
        spriteStruct->SpriteData[ x ].GSData[ 2 ] = SCE_GS_SET_XYZ2( X0, Y0, 10000 );
        spriteStruct->SpriteData[ x ].GSData[ 3 ] = SCE_GS_XYZ2;
        spriteStruct->SpriteData[ x ].GSData[ 4 ] = SCE_GS_SET_UV( U1, V1 );
        spriteStruct->SpriteData[ x ].GSData[ 5 ] = SCE_GS_UV;
        spriteStruct->SpriteData[ x ].GSData[ 6 ] = SCE_GS_SET_XYZ2( X1, Y1, 10000 );
        spriteStruct->SpriteData[ x ].GSData[ 7 ] = SCE_GS_XYZ2;
        X0 = X1+1;
        X1 += XStep;
        U0 = U1+1;
        U1 += UStep;
    }
}

void SetDownSampleSpriteToDraw( fsAABuff *buff, gsDrawDecalSprite *spriteStruct, s32 filter )
{
    BuildDownSampleSpriteUsingTarget( spriteStruct, 
                                      buff->drawW, buff->drawH, buff->drawPSM, buff->drawFBP,
                                      buff->drawW, buff->drawH, buff->drawPSM, buff->drawFBP,
                                      filter, FALSE );
    FlushCache(0);
}

//=========================================================================

void SetDownSampleSpriteToDisplay( fsAABuff *buff, gsDrawDecalSprite *spriteStruct, s32 filter )
{
    BuildDownSampleSpriteUsingTarget( spriteStruct, 
                                      buff->drawW, buff->drawH, buff->drawPSM, buff->drawFBP,
                                      buff->dispW, buff->dispH, buff->dispPSM, buff->dispFBP,
                                      filter, TRUE );
    FlushCache(0);
}

//=========================================================================

void PutDownSampleSprite(sceGifTag *spriteTag )
{
    u32 vcnt = 0;

    FlushCache(0);

    while(DGET_D2_CHCR() & 0x0100)
    {
        if((vcnt++)>0x1000000)
        {
            x_printf("SPRITE:Error starting DMA: DMA Ch.2 does not terminate\r\n");
        }
    }

    DPUT_D2_QWC((spriteTag->NLOOP+1));
    if(((u32)spriteTag & 0x70000000) == 0x70000000)
        DPUT_D2_MADR((((u32)spriteTag & 0x0fffffff) | 0x80000000));
    else
        DPUT_D2_MADR(((u32)spriteTag & 0x0fffffff));
    *D2_CHCR = (1 << 8) | 1;
}

//=========================================================================

void DownSampleToDispBufferTiled(fsAABuff *buff, s32 oddFrame)
{
    // If buffer 0 work down in page units
    // If buffer 1 work up   in page units

    // Examples of tile transfer

    // 1280x448->640x224: 64x32->32x16 (32bit src -> 32bit dispBuf) 
    //                    64x64->32x32 (16bit src -> 32bit dispBuf)

    // 1024x448->640x224: 64x32->40x16 (32bit src -> 32bit dispBuf)
    //                    64x64->40*32 (16bit src -> 32bit dispBuf)

    //  640x448->640x224: 64x32->64x16 (32bit src -> 32bit dispBuf)
    //                    64x64->64x32 (16bit src -> 32bit dispBuf)

    // 1024x448->512x224: 64x32->32x16 (32bit src -> 32bit dispBuf) 
    //                    64x64->32x32 (16bit src -> 32bit dispBuf)

    //  512x448->512x224: 64x32->64x16 (32bit src -> 32bit dispBuf)
    //                    64x64->64x32 (16bit src -> 32bit dispBuf)

    s16 w, h;

    s16 drawPagesWide = buff->drawW / 64;
    s16 drawPagesHigh = buff->drawH / 64;
    s16 pageAddr;

    //  Set tex0 = tile width, height
    for (h=0; h<drawPagesHigh; h++)
    {
        for (w=0; w<drawPagesWide; w++)
        {
            pageAddr = (h * drawPagesWide) + w;
        }
    }
}
//=========================================================================

void BuildGSZBufferTextureOffScreen( u64 Src, u64 Dest )
{
    CreateGSZBufferTexture( Src, Dest, &ZTargetData );
}

//=========================================================================

void BuildGSZBufferTextureOnScreen( u64 Src, u64 Dest )
{
    CreateGSZBufferTexture( Src, Dest, &ZOnScreenData );
}

//=========================================================================

void ActivateGSZBufferTextureOffScreen( void )
{
    FlushCache(0);

    PutDownSampleSprite( &(ZTargetData.giftag0) );
}

//=========================================================================

void ActivateGSZBufferTextureOnScreen( void )
{
    FlushCache(0);

    PutDownSampleSprite( &(ZOnScreenData.giftag0) );
}

//=========================================================================

void CreateGSZBufferTexture( u64 Src, u64 Dest, gsZBufferSprite *pBuffer )
{
    // take the Z-Buffer Src and BLT it into Dst as an intensity texture
	static const s32 kStripHeight = 896;
	static const s32 kSrcStripStart = 0;
	static const s32 kDstStripStart = 8;
	static const s32 kSrcStripWidth = 8;
	static const s32 kDstStripWidth = 8;
	static const s32 kNumStrips = ((640/kSrcStripWidth) / 2) + 1;

    SCE_GIF_CLEAR_TAG( &pBuffer->giftag0 );
    VIFHELP_BuildGifTag1( &pBuffer->giftag0, VIFHELP_GIFMODE_PACKED, 1, 12 + (4 * kNumStrips), FALSE, 0, 0, TRUE );
    VIFHELP_BuildGifTag2( &pBuffer->giftag0, VIFHELP_GIFREG_AD, 0, 0, 0 );

    pBuffer->GSCmds[ 0 ]    = SCE_GS_SET_TEST_1( 0, 0, 0, 0, 0, 0, 1, SCE_GS_ZALWAYS );
    pBuffer->GSCmds[ 1 ]    = SCE_GS_TEST_1;
    pBuffer->GSCmds[ 2 ]    = SCE_GS_SET_ALPHA_1( 0, 1, 0, 1, 0 );
    pBuffer->GSCmds[ 3 ]    = SCE_GS_ALPHA_1;
    pBuffer->GSCmds[ 4 ]    = SCE_GS_SET_SCISSOR_1( 0, ENG_GetScreenWidth()-1, 0, (ENG_GetScreenHeight()*2)-1 );
    pBuffer->GSCmds[ 5 ]    = SCE_GS_SCISSOR_1;
    pBuffer->GSCmds[ 6 ]    = SCE_GS_SET_FRAME_1( Dest, (640>>6)&0x3F, SCE_GS_PSMCT16, 0x00003FFF );
    pBuffer->GSCmds[ 7 ]    = SCE_GS_FRAME_1;
    pBuffer->GSCmds[ 8 ]    = SCE_GS_SET_ZBUF_1( Src, SCE_GS_PSMZ32, 1 );
    pBuffer->GSCmds[ 9 ]    = SCE_GS_ZBUF_1;
    pBuffer->GSCmds[ 10 ]   = 0;//SCE_GS_SET_XYOFFSET((2048-(ENG_GetScreenWidth()>>1))<<4, (2048-(ENG_GetScreenHeight()>>1))<<4);
    pBuffer->GSCmds[ 11 ]   = 0;//SCE_GS_XYOFFSET_1;
    pBuffer->GSCmds[ 12 ]   = SCE_GS_SET_TEXA( 0x7f, 0, 0x7f );
    pBuffer->GSCmds[ 13 ]   = SCE_GS_TEXA;
    pBuffer->GSCmds[ 14 ]   = SCE_GS_SET_TEX1_1( 0, 0, 0, 0, 0, 0, 0 );
    pBuffer->GSCmds[ 15 ]   = SCE_GS_TEX1_1;
    pBuffer->GSCmds[ 16 ]   = SCE_GS_SET_TEX0_1( Src*32, 10, SCE_GS_PSMZ16, 10, 10, 1, 1, 0, 0, 0, 0, 0 );
    pBuffer->GSCmds[ 17 ]   = SCE_GS_TEX0_1;
    pBuffer->GSCmds[ 18 ]   = 0;
    pBuffer->GSCmds[ 19 ]   = SCE_GS_TEXFLUSH;
    pBuffer->GSCmds[ 20 ]   = SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 1, 0, 0, 0, 1, 0, 0 );
    pBuffer->GSCmds[ 21 ]   = SCE_GS_PRIM;
    pBuffer->GSCmds[ 22 ]   = SCE_GS_SET_RGBAQ( 0x00, 0x00, 0x00, 0x7F, 0x3f800000 );
    pBuffer->GSCmds[ 23 ]   = SCE_GS_RGBAQ;

    pBuffer->scissor2        = SCE_GS_SET_SCISSOR_1( 0, ENG_GetScreenWidth()-1, 0, ENG_GetScreenHeight()-1 );


    s32 i;
    s32 SrcXOffset = kSrcStripStart;
    s32 DstXOffset = kDstStripStart;

    s32 u0, u1;
    s32 x0, x1;
    s32 v0, v1;
    s32 y0, y1;


    for ( i = 0; i < kNumStrips; ++i )
    {
        u0 = (SrcXOffset<<4) + 8;
        u1 = ((SrcXOffset+kSrcStripWidth)<<4) + 8;
        v0 = (0<<4) + 8;
        v1 = (kStripHeight<<4) + 8;

        x0 = kGuardbandX + (DstXOffset<<4);
        x1 = kGuardbandX + ((DstXOffset+kDstStripWidth)<<4);
        y0 = kGuardbandY;
        y1 = kGuardbandY + (kStripHeight<<4);


        pBuffer->strips[i].UV1       = SCE_GS_SET_UV( u0, v0 );
        pBuffer->strips[i].UV1addr   = SCE_GS_UV;
        pBuffer->strips[i].XYZ1      = SCE_GS_SET_XYZ( x0, y0, 0 );
        pBuffer->strips[i].XYZ1addr  = SCE_GS_XYZ2;
        pBuffer->strips[i].UV2       = SCE_GS_SET_UV( u1, v1 );
        pBuffer->strips[i].UV2addr   = SCE_GS_UV;
        pBuffer->strips[i].XYZ2      = SCE_GS_SET_XYZ( x1, y1, 0 );
        pBuffer->strips[i].XYZ2addr  = SCE_GS_XYZ2;

        SrcXOffset += kSrcStripWidth * 2;
        DstXOffset += kDstStripWidth * 2;
    }
}

//=========================================================================

void BuildGSZBufferSkyMask( u64 Src, u64 Dest, u64 ZBuf )
{
	static const s32 kStripHeight = kDisplayHeight;
	static const s32 kSrcStripStart = 0;
	static const s32 kDstStripStart = 0;
	static const s32 kSrcStripWidth = ENG_GetScreenWidth();
	static const s32 kDstStripWidth = ENG_GetScreenWidth();
	static const s32 kNumStrips = ((640/kSrcStripWidth) / 2) + 1;

    SCE_GIF_CLEAR_TAG( &ZSkyMaskData.giftag0 );

    ZSkyMaskData.giftag0.NLOOP   = 12 + (4 * kNumStrips);
    ZSkyMaskData.giftag0.EOP     = 1;
    ZSkyMaskData.giftag0.PRIM    = 0;
    ZSkyMaskData.giftag0.PRE     = 0;
    ZSkyMaskData.giftag0.NREG    = 1;
    ZSkyMaskData.giftag0.FLG     = 0;
    ZSkyMaskData.giftag0.REGS0   = 0xE;  // A_D

    ZSkyMaskData.GSCmds[ 0 ]    = SCE_GS_SET_TEST_1( 1, 4, 0, 0, 0, 0, 1, SCE_GS_ZALWAYS );
    ZSkyMaskData.GSCmds[ 1 ]    = SCE_GS_TEST_1;
    ZSkyMaskData.GSCmds[ 2 ]    = SCE_GS_SET_ALPHA_1( 0, 1, 0, 0, 0 );
    ZSkyMaskData.GSCmds[ 3 ]    = SCE_GS_ALPHA_1;
    ZSkyMaskData.GSCmds[ 4 ]    = SCE_GS_SET_SCISSOR_1( 0, ENG_GetScreenWidth()-1, 0, ENG_GetScreenHeight()-1 );
    ZSkyMaskData.GSCmds[ 5 ]    = SCE_GS_SCISSOR_1;
    ZSkyMaskData.GSCmds[ 6 ]    = SCE_GS_SET_FRAME_1( Dest, (640>>6)&0x3F, SCE_GS_PSMCT32, 0);
    ZSkyMaskData.GSCmds[ 7 ]    = SCE_GS_FRAME_1;
    ZSkyMaskData.GSCmds[ 8 ]    = SCE_GS_SET_ZBUF_1( ZBuf, SCE_GS_PSMZ32, 1 );
    ZSkyMaskData.GSCmds[ 9 ]    = SCE_GS_ZBUF_1;
    ZSkyMaskData.GSCmds[ 10 ]   = 0;//SCE_GS_SET_XYOFFSET((2048-(ENG_GetScreenWidth()>>1))<<4, (2048-(ENG_GetScreenHeight()>>1))<<4);
    ZSkyMaskData.GSCmds[ 11 ]   = 0;//SCE_GS_XYOFFSET_1;
    ZSkyMaskData.GSCmds[ 12 ]   = SCE_GS_SET_TEXA( 0x7f, 0, 0x7f );
    ZSkyMaskData.GSCmds[ 13 ]   = SCE_GS_TEXA;
    ZSkyMaskData.GSCmds[ 14 ]   = SCE_GS_SET_TEX1_1( 0, 0, 0, 0, 0, 0, 0 );
    ZSkyMaskData.GSCmds[ 15 ]   = SCE_GS_TEX1_1;
    ZSkyMaskData.GSCmds[ 16 ]   = SCE_GS_SET_TEX0_1( Src*32, 10, SCE_GS_PSMCT32, 10, 10, 1, 1, 0, 0, 0, 0, 0 );
    ZSkyMaskData.GSCmds[ 17 ]   = SCE_GS_TEX0_1;
    ZSkyMaskData.GSCmds[ 18 ]   = 0;
    ZSkyMaskData.GSCmds[ 19 ]   = SCE_GS_TEXFLUSH;
    ZSkyMaskData.GSCmds[ 20 ]   = SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0 );
    ZSkyMaskData.GSCmds[ 21 ]   = SCE_GS_PRIM;
    ZSkyMaskData.GSCmds[ 22 ]   = SCE_GS_SET_RGBAQ( 0x7f, 0x7f, 0x7f, 0x7f, 0x3f800000 );
    ZSkyMaskData.GSCmds[ 23 ]   = SCE_GS_RGBAQ;

    ZSkyMaskData.scissor2       = SCE_GS_SET_SCISSOR_1( 0, ENG_GetScreenWidth()-1, 0, ENG_GetScreenHeight()-1 );
    ZSkyMaskData.scissor2addr   = SCE_GS_SCISSOR_1;


    s32 i;
    s32 SrcXOffset = kSrcStripStart;
    s32 DstXOffset = kDstStripStart;

    s32 u0, u1;
    s32 x0, x1;
    s32 v0, v1;
    s32 y0, y1;


    for ( i = 0; i < kNumStrips; ++i )
    {
        u0 = (SrcXOffset<<4) + 8;
        u1 = ((SrcXOffset+kSrcStripWidth)<<4) + 8;
        v0 = (0<<4) + 8;
        v1 = (kStripHeight<<4) + 8;

        x0 = kGuardbandX + (DstXOffset<<4);
        x1 = kGuardbandX + ((DstXOffset+kDstStripWidth)<<4);
        y0 = kGuardbandY;
        y1 = kGuardbandY + (kStripHeight<<4);


        ZSkyMaskData.strips[i].UV1       = SCE_GS_SET_UV( u0, v0 );
        ZSkyMaskData.strips[i].UV1addr   = SCE_GS_UV;
        ZSkyMaskData.strips[i].XYZ1      = SCE_GS_SET_XYZ( x0, y0, 0 );
        ZSkyMaskData.strips[i].XYZ1addr  = SCE_GS_XYZ2;
        ZSkyMaskData.strips[i].UV2       = SCE_GS_SET_UV( u1, v1 );
        ZSkyMaskData.strips[i].UV2addr   = SCE_GS_UV;
        ZSkyMaskData.strips[i].XYZ2      = SCE_GS_SET_XYZ( x1, y1, 0 );
        ZSkyMaskData.strips[i].XYZ2addr  = SCE_GS_XYZ2;

        SrcXOffset += kSrcStripWidth * 2;
        DstXOffset += kDstStripWidth * 2;
    }
}

//=========================================================================


void ActivateGSZBufferSkyMask( void )
{
    // take the Z-Buffer Src and BLT it into Dst as an intensity texture
    FlushCache(0);

    PutDownSampleSprite( &(ZSkyMaskData.giftag0) );
}

//=========================================================================

void BuildFogClut( void )
{
	// our fog formula is:
	// fog = MIN( max_thicknexx, ((distance-fog_start)/(end-start) );

	// now build a clut that will map the screen into fog space
	// the formula for screen z is:
	// F = far plane
	// N = near plane
	// S = z-buffer scale
	// A = (F+N)/(F-N)
	// B = -2FN/(F-N)
	// z' = -S/2 * [A+B/z] + S/2
	// Solve for z in terms of z' and we have (used Mathematica to solve):
	// z = F*N*S / (NS + Fz' - Nz')
	
	// grab the variables needed for this equation. The z-buffer bits we get are dependant on
	// the engine's z-scale. The microcode will shift it up four bits as well (fixed-point)

	view* pView = ENG_GetActiveView();
	f32 nearz, farz;
	pView->GetZLimits(nearz, farz);
	s32 zrange = (1<<19)-1;	//#### This must match PS2_ENGINE, ENG_SetActiveView!!!!!!!
	zrange <<= 4;	//#### Can't figure out why I should have to do this. Granted, the microcode does a ftoi4, but the packed mode uses shifted bits as well. ARGHH!!!!
	f32 scale = (f32)(zrange);
	f32 numer = nearz*farz*scale;

	// now build a clut that will map the screen into fog space
	// the clut comes from bits of the z-buffer.
	s32 i;
	for ( i = 0; i < 256; ++i )
	{
		f32 zprime = (f32)(i<<8);
		f32 denom = nearz*scale + farz*zprime - nearz*zprime;
		ASSERT( denom >= 0.001f );
		if ( denom < 0.0001f )
		{
			// This should never get hit...its just here for safety
			denom = 1.0f;
		}
		f32 z = numer / denom;
		f32 fog = (z - g_EngineFog.fStart) * g_EngineFog.fDensity;
		if ( fog > g_EngineFog.fMaxFog )
			fog = g_EngineFog.fMaxFog;
		if ( fog < 0.0f )
			fog = 0.0f;
		//if ( fog > 1.0f )
		//	fog = 1.0f;

		//#### HACK HACK HACK HACK - Artists were relying on vertex colored fog, so their sky-boxes were
		// getting full fog (ouch!) Instead, we'll just cap it to 0.5f.
//		if ( fog > 0.5f )
//			fog = 0.5f;

		// clut is swizzled--fancy trick to find swizzled index is to swap
		// bits 0x08 and 0x10
		s32 clut_index = 0;
		clut_index |= (i&0xe7);
		clut_index |= (i&0x08)<<1;
		clut_index |= (i&0x10)>>1;
		s_FogClut[clut_index].R = eng_fog_color.R;
		s_FogClut[clut_index].G = eng_fog_color.G;
		s_FogClut[clut_index].B = eng_fog_color.B;
		s_FogClut[clut_index].A = (s32)(fog * 128.0f);
	}
}

//=========================================================================

void SendFogClut( void )
{
	// send the fog clut
	gsFogClutSend* pClutSend = &FogClut;

    if (s_bBuildPackets)
    {
        SCE_GIF_CLEAR_TAG( &pClutSend->GIFClutTransfer );
        pClutSend->GIFClutTransfer.NLOOP    = 4;
        pClutSend->GIFClutTransfer.EOP      = 1;
        pClutSend->GIFClutTransfer.PRIM     = 0;
        pClutSend->GIFClutTransfer.PRE      = 0;
        pClutSend->GIFClutTransfer.NREG     = 1;
        pClutSend->GIFClutTransfer.FLG      = 0;
        pClutSend->GIFClutTransfer.REGS0    = 0xE;

	    pClutSend->BitBltBufAddr	= (u64)SCE_GS_BITBLTBUF;
	    pClutSend->TrxPosAddr		= (u64)SCE_GS_TRXPOS;
	    pClutSend->TrxRegAddr		= (u64)SCE_GS_TRXREG;
	    pClutSend->TrxDirAddr		= (u64)SCE_GS_TRXDIR;
	    pClutSend->BitBltBuf		= (u64)SCE_GS_SET_BITBLTBUF( 0, 0, 0, kClutStart, 1, SCE_GS_PSMCT32 );
	    pClutSend->TrxPos			= (u64)SCE_GS_SET_TRXPOS( 0, 0, 0, 0, 0x00 );
	    pClutSend->TrxReg			= (u64)SCE_GS_SET_TRXREG( 16, 16 );
	    pClutSend->TrxDir			= (u64)SCE_GS_SET_TRXDIR( 0x00 );

        SCE_GIF_CLEAR_TAG( &pClutSend->GIFClutImage );
        pClutSend->GIFClutImage.NLOOP   = 64;
        pClutSend->GIFClutImage.EOP     = 1;
        pClutSend->GIFClutImage.PRIM    = 0;
        pClutSend->GIFClutImage.PRE     = 0;
        pClutSend->GIFClutImage.NREG    = 0;
        pClutSend->GIFClutImage.FLG     = 0;
        pClutSend->GIFClutImage.REGS0   = 0;
	    pClutSend->ClutRefDMA.qwc = 64;
	    pClutSend->ClutRefDMA.id	= 0x30;
	    pClutSend->ClutRefDMA.next	= (sceDmaTag*)s_FogClut;
	    pClutSend->ClutRefDMA.mark	= 0;
	    pClutSend->ClutRefDMA.p[0]	= 0;
	    pClutSend->ClutRefDMA.p[1] = 0;
    }

    PutDownSampleSprite( &pClutSend->GIFClutTransfer );
}

//=========================================================================

void BuildGSFogSprite( u64 Dest, u64 ZBuf )
{
    if (s_bBuildPackets)
    {
        gsZBufferNoTexSprite *pBuffer = &ZNoTexData;

        SCE_GIF_CLEAR_TAG( &pBuffer->giftag0 );

        pBuffer->giftag0.NLOOP     = 6 + (2 * kNumFogStrips);
        pBuffer->giftag0.EOP       = 1;
        pBuffer->giftag0.PRIM      = 0;
        pBuffer->giftag0.PRE       = 0;
        pBuffer->giftag0.NREG      = 1;
        pBuffer->giftag0.FLG       = 0;
        pBuffer->giftag0.REGS0     = 0xE;  // A_D

        pBuffer->test1addr         = (u64)SCE_GS_TEST_1;
        pBuffer->alphaaddr         = (u64)SCE_GS_ALPHA_1;
        pBuffer->frame1addr        = (u64)SCE_GS_FRAME_1;
        pBuffer->zbufaddr          = (u64)SCE_GS_ZBUF_1;
        pBuffer->primaddr          = (u64)SCE_GS_PRIM;
        pBuffer->rgbaaddr          = (u64)SCE_GS_RGBAQ;

        pBuffer->test1             = SCE_GS_SET_TEST_1( 0, 0, 0, 0, 0, 0, 1, 2 );
        pBuffer->alpha             = SCE_GS_SET_ALPHA_1( 1, 0, 1, 0, 0 );
        pBuffer->frame1            = SCE_GS_SET_FRAME_1( Dest, (640>>6)&0x3F, SCE_GS_PSMCT32, 0 );
        pBuffer->zbuf              = SCE_GS_SET_ZBUF_1( ZBuf, SCE_GS_PSMZ32, 1 );
        pBuffer->prim              = SCE_GS_SET_PRIM( 0x6, 0, 0, 0, 1, 0, 0, 0, 0 );
        pBuffer->rgba              = SCE_GS_SET_RGBAQ( eng_fog_color.R, eng_fog_color.G, eng_fog_color.B, 0x7f, 0x3f800000 );

	    static const s32 kStripWidth = 32;
	    static const s32 kStripHeight = ENG_GetScreenHeight();
	    static const s32 kNumFogStrips = ENG_GetScreenWidth()/kStripWidth;
        s32 i;
        s32 XOffset = 0;
        for ( i = 0; i < kNumFogStrips; ++i )
        {
            s32 x0, x1;
            s32 y0, y1;
            s32 u0, u1;
            s32 v0, v1;

            x0 = kGuardbandX + (XOffset<<4);
            x1 = kGuardbandX + ((XOffset+kStripWidth)<<4);
            y0 = kGuardbandY;
            y1 = kGuardbandY + (kStripHeight<<4);

            u0 = (XOffset<<4) + 8;
            u1 = ((XOffset+kStripWidth)<<4) + 8;
            v0 = (0<<4) + 8;
            v1 = (kStripHeight<<4) + 8;

            pBuffer->strips[i].XYZ1        = SCE_GS_SET_XYZ( x0, y0, 0x7FFF );
            pBuffer->strips[i].XYZ1addr    = SCE_GS_XYZ2;
            pBuffer->strips[i].XYZ2        = SCE_GS_SET_XYZ( x1, y1, 0x7FFF );
            pBuffer->strips[i].XYZ2addr    = SCE_GS_XYZ2;

            XOffset += kStripWidth;
        }
    }
}

//=========================================================================

void DrawGSFogSprite( void )
{
    FlushCache(0);

    PutDownSampleSprite( &(ZNoTexData.giftag0) );
}

//=========================================================================

void BuildGSFogSpriteToDisplayData( u64 Src, u64 Dest, u64 ZBuf )
{
    if (s_bBuildPackets)
    {
	    s32 StripWidth = ENG_GetScreenWidth();
	    s32 StripHeight = ENG_GetScreenHeight();
	    s32 NumFogStrips = StripWidth/StripWidth;

        gsZBufferSprite *pBuffer = &DisplayData;

        VIFHELP_BuildGifTag1( &pBuffer->giftag0, VIFHELP_GIFMODE_PACKED, 1, 12 + (4 * kNumFogStrips), FALSE, 0, 0, TRUE );
        VIFHELP_BuildGifTag2( &pBuffer->giftag0, VIFHELP_GIFREG_AD, 0, 0, 0 );

        f32 fAlpha = g_EngineFog.fDensity;
	    if ( fAlpha > g_EngineFog.fMaxFog )
		    fAlpha = g_EngineFog.fMaxFog;
	    if ( fAlpha < 0.0f )
		    fAlpha = 0.0f;
        fAlpha *= 127.0f;

        pBuffer->GSCmds[ 0 ]    = SCE_GS_SET_TEST_1( 0, 0, 0, 0, 0, 0, 1, 2 );
        pBuffer->GSCmds[ 1 ]    = SCE_GS_TEST_1;
        pBuffer->GSCmds[ 2 ]    = SCE_GS_SET_ALPHA_1( 0, 1, 2, 1, (s32)fAlpha );
        pBuffer->GSCmds[ 3 ]    = SCE_GS_ALPHA_1;
        pBuffer->GSCmds[ 4 ]    = SCE_GS_SET_SCISSOR_1( 0, StripWidth-1, 0, StripHeight-1 );
        pBuffer->GSCmds[ 5 ]    = SCE_GS_SCISSOR_1;
        pBuffer->GSCmds[ 6 ]    = SCE_GS_SET_FRAME_1( Dest, (StripWidth>>6)&0x3F, SCE_GS_PSMCT32, 0 );
        pBuffer->GSCmds[ 7 ]    = SCE_GS_FRAME_1;
        pBuffer->GSCmds[ 8 ]    = SCE_GS_SET_ZBUF_1( ZBuf, SCE_GS_PSMZ32, 1 );
        pBuffer->GSCmds[ 9 ]    = SCE_GS_ZBUF_1;
        pBuffer->GSCmds[ 10 ]   = 0;//SCE_GS_SET_XYOFFSET((2048-(StripWidth>>1))<<4, (2048-(StripHeight>>1))<<4);
        pBuffer->GSCmds[ 11 ]   = 0;//SCE_GS_XYOFFSET_1;
        pBuffer->GSCmds[ 12 ]   = SCE_GS_SET_TEXA( fAlpha, 0, fAlpha );
        pBuffer->GSCmds[ 13 ]   = SCE_GS_TEXA;
        pBuffer->GSCmds[ 14 ]   = SCE_GS_SET_TEX1_1( 1, 0, 1, 1, 1, 0, 1 );//( 0, 0, 0, 0, 0, 0, 0 );
        pBuffer->GSCmds[ 15 ]   = SCE_GS_TEX1_1;
        pBuffer->GSCmds[ 16 ]   = SCE_GS_SET_TEX0_1( Src*32, 10, SCE_GS_PSMCT32, 10, 10, 1, 1, 0, 0, 0, 0, 0 );
        pBuffer->GSCmds[ 17 ]   = SCE_GS_TEX0_1;
        pBuffer->GSCmds[ 18 ]   = 0;
        pBuffer->GSCmds[ 19 ]   = SCE_GS_TEXFLUSH;
        pBuffer->GSCmds[ 20 ]   = SCE_GS_SET_PRIM( 0x6, 0, 1, 0, 1, 0, 1, 0, 0 );
        pBuffer->GSCmds[ 21 ]   = SCE_GS_PRIM;
        pBuffer->GSCmds[ 22 ]   = SCE_GS_SET_RGBAQ( 0x40, 0x40, 0x40, 0x40, 0x3f800000 );
        pBuffer->GSCmds[ 23 ]   = SCE_GS_RGBAQ;

        pBuffer->scissor2       = SCE_GS_SET_SCISSOR_1( 0, StripWidth-1, 0, kStripHeight-1 );
        pBuffer->scissor2addr   = SCE_GS_SCISSOR_1;

        s32 i;
        s32 XOffset = 0;
        for ( i = 0; i < NumFogStrips; ++i )
        {
            s32 x0, x1;
            s32 y0, y1;
            s32 u0, u1;
            s32 v0, v1;

            x0 = kGuardbandX + (XOffset<<4);
            x1 = kGuardbandX + ((XOffset+StripWidth)<<4);
            y0 = kGuardbandY;
            y1 = kGuardbandY + (StripHeight<<4);

            u0 = (XOffset<<4) + 8;
            u1 = ((XOffset+StripWidth)<<4) + 8;
            v0 = (0<<4) + 8;
            v1 = (StripHeight<<4) + 8;

            pBuffer->strips[i].UV1         = SCE_GS_SET_UV(u0, v0);
            pBuffer->strips[i].UV1addr     = SCE_GS_UV;
            pBuffer->strips[i].XYZ1        = SCE_GS_SET_XYZ( x0, y0, 0x7fff );
            pBuffer->strips[i].XYZ1addr    = SCE_GS_XYZ2;
            pBuffer->strips[i].UV2         = SCE_GS_SET_UV(u1, v1);
            pBuffer->strips[i].UV2addr     = SCE_GS_UV;
            pBuffer->strips[i].XYZ2        = SCE_GS_SET_XYZ( x1, y1, 0x7fff );
            pBuffer->strips[i].XYZ2addr    = SCE_GS_XYZ2;

            XOffset += StripWidth;
        }
    }
}

//=========================================================================

void CopyGSFogSpriteToDisplay( void )
{
    FlushCache(0);
    PutDownSampleSprite( &(DisplayData.giftag0) );
}

//=========================================================================
//=========================================================================


