//===========================================================================
// File: framebuf.h
// Date: 10 May 2000
// Author: Mark Breugelmans (Sony Computer Entertainment Europe)
// Description: Simple functions to setup display/draw environments
//              and to draw filtered sprites
//===========================================================================

#ifndef FRAMEBUF_HPP
#define FRAMEBUF_HPP



#include <libgraph.h>
#include <libdma.h>
#include "x_files.hpp"
#include "x_types.hpp"
#include "Q_Engine.hpp"

void SwapGSBuffers(s32 oddOrEven);

typedef struct {
    tGS_PMODE       pmode;
    tGS_SMODE2      smode2;
    tGS_DISPFB2     dispfb;
    tGS_DISPLAY2    display;
    tGS_BGCOLOR     bgcolor;


    tGS_DISPFB1     dispfb1;
    tGS_DISPLAY1    display1;
    tGS_DISPLAY1    pad;
} DispEnvTwoCircuits;

typedef struct 
{
    DispEnvTwoCircuits  disp0;        // GS Priviledged registers to set up disp buffer
    DispEnvTwoCircuits  disp1;        // GS Priviledged registers to set up disp buffer

    sceGifTag     giftagDrawLarge; 
    sceGsDrawEnv1 drawLarge;
    sceGsClear    clearLarge;

    sceGifTag     giftagDrawSmall; 
    sceGsDrawEnv1 drawSmall;   
    sceGsClear    clearSmall;

    s16 drawW;
    s16 drawH;
    s16 drawPSM;
    s16 drawFBP;

    s16 dispW;
    s16 dispH;
    s16 dispPSM;
    s16 dispFBP;

    s16 zPSM;
    s16 zFBP;

} fsAABuff __attribute__((aligned(64)));

typedef struct
{
    u64         GSData[ 8 ];
} gsDrawDecalSpriteData;// __attribute__((aligned(16)));

typedef struct {
    sceGifTag               GIFtag;
    u64                     GSCmds[ 18 ];

    // if we fill the screen with 63x31 sprites, we use 140 different sprites
    gsDrawDecalSpriteData   SpriteData[ 11 ];
} gsDrawDecalSprite;// __attribute__((aligned(16)));

static const s32 kGuardbandX = (2048-320)<<4;
static const s32 kGuardbandY = (2048-224)<<4;
static const s32 kClutStart = 0x3480;

static const s32 kSrcStripWidth = 8;
static const s32 kDstStripWidth = 32;
static const s32 kStripHeight = kDisplayHeight;
static const s32 kSrcStripStart = 0;
static const s32 kDstStripStart = 0;
static const s32 kNumStrips = (kDisplayWidth/kSrcStripWidth)/2;
static const s32 kNumFogStrips = (kDisplayWidth/kDstStripWidth);

typedef struct
{
    u64             UV1;
    u64             UV1addr;
    u64             XYZ1;
    u64             XYZ1addr;
    u64             UV2;
    u64             UV2addr;
    u64             XYZ2;
    u64             XYZ2addr;
}gsRG2BASprite;

typedef struct 
{
    sceGifTag       giftag0;        // Includes prim for decal sprite

    u64             GSCmds[ 24 ];

    gsRG2BASprite   strips[kNumStrips];

    u64             scissor2;
    u64             scissor2addr;

}gsZBufferSprite;

extern gsZBufferSprite ZTargetData;
extern gsZBufferSprite ZOnScreenData;
extern gsZBufferSprite ZSkyMaskData;
extern gsZBufferSprite DisplayData;

typedef struct
{
    u64             XYZ1;
    u64             XYZ1addr;
    u64             XYZ2;
    u64             XYZ2addr;
}gsRG2BANoTexSprite;// __attribute__((aligned(64)));

typedef struct 
{
    sceGifTag       giftag0;        // Includes prim for decal sprite

    u64             test1;
    u64             test1addr;

    u64             alpha;
    u64             alphaaddr;

    u64             frame1;
    u64             frame1addr; 

    u64             zbuf;
    u64             zbufaddr;

    u64             prim;
    u64             primaddr;

    u64             rgba;
    u64             rgbaaddr;

    gsRG2BANoTexSprite   strips[kNumStrips];

}gsZBufferNoTexSprite __attribute__((aligned(64)));

extern gsZBufferNoTexSprite ZNoTexData;

typedef struct 
{
	sceGifTag	GIFClutTransfer;
	u64			BitBltBuf;
	u64			BitBltBufAddr;
	u64			TrxPos;
	u64			TrxPosAddr;
	u64			TrxReg;
	u64			TrxRegAddr;
	u64			TrxDir;
	u64			TrxDirAddr;
	sceGifTag	GIFClutImage;
	sceDmaTag	ClutRefDMA;
}gsFogClutSend;

extern gsFogClutSend FogClut;

typedef struct sprite_tag {

	s32 center;
	s32 r,g,b,a;
	s32 x0,y0;
	s32 x1,y1;
	s32 u0,v0; 
	s32 u1,v1; 
	s32 w;
	s32 h;
	s32 x;
	s32 y; 

}SPRITE;



void SetDispBuffer(fsAABuff *buff, s16 w, s16 h, s16 psm, s16 fbp, xbool bIsProgressive ); 
void SetDrawBufferLarge(fsAABuff *buff, s16 w, s16 h, s16 psm, s16 fbp, 
                        s16 zbp, s16 ztest, s16 zpsm, s16 clear); 
void SetDrawBufferSmall(fsAABuff *buff, s16 w, s16 h, s16 psm, s16 fbp); // No Z Test

void SetDispBuffer_Cheat( fsAABuff *buff, xbool bIsProgressive ); 
void SetDrawBufferLarge_Cheat( fsAABuff *buff, s16 ztest, s16 clear );
void SetDrawBufferSmall_Cheat( fsAABuff *buff, s16 ztest, s16 clear );

void PutDrawBufferSmall(fsAABuff *buff);
void PutDrawBufferLarge(fsAABuff *buff);
void PutDispBuffer(fsAABuff *buff, s32 bufferNum);


// Note half offset only to be used when copying to small buffer (if at all)
void SetHalfOffset(void);
void Setup_FSAA_buffer(fsAABuff *buff, s16 dispW, s16 dispH, s16 dispPSM,
                       s16 drawW, s16 drawH, s16 drawPSM, 
                       s16 ztest, s16 zPSM,
                       s16 clear, xbool bIsProgressive = FALSE);
void Setup_Cheat_buffer( fsAABuff *buff, s16 buff1W, s16 buff1H, s16 buff1PSM,
                         s16 buff2W, s16 buff2H, s16 buff2PSM,
                         s16 ztest, s16 zPSM, s16 zClear, xbool bIsProgressive = FALSE );


void DownSampleToDispBufferTiled(fsAABuff *buff, s32 oddFrame);

void SetDownSampleSpriteToDraw(fsAABuff *buff, gsDrawDecalSprite *spriteStruct, s32 filter);

void SetDownSampleSpriteToDisplay( fsAABuff *buff, gsDrawDecalSprite *spriteStruct, s32 filter );

void PutDownSampleSprite(sceGifTag *spriteTag );

void ResetFogPackets( void );

void BuildGSZBufferTextureOffScreen( u64 Src, u64 Dest );
void BuildGSZBufferTextureOnScreen( u64 Src, u64 Dest );
void ActivateGSZBufferTextureOffScreen( void );
void ActivateGSZBufferTextureOnScreen( void );
void CreateGSZBufferTexture( u64 Src, u64 Dest, gsZBufferSprite *pBuffer );

void BuildGSZBufferSkyMask( u64 Src, u64 Dest, u64 ZBuf );
void ActivateGSZBufferSkyMask( void );
void BuildFogClut( void );
void SendFogClut( void );
void BuildGSFogSprite( u64 Dest, u64 ZBuf );
void DrawGSFogSprite( void );
void BuildGSFogSpriteToDisplayData( u64 Src, u64 Dest, u64 ZBuf );
void CopyGSFogSpriteToDisplay( void );

void CalcFrameBufferPositions(fsAABuff *buff, s16 dispW, s16 dispH, s16 dispPSM,
                              s16 drawW, s16 drawH, s16 drawPSM);





#endif//FRAMEBUF_HPP
