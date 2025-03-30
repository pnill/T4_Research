////////////////////////////////////////////////////////////////////////////
//
// GC_VIDEO.CPP
//
//
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_types.hpp"
#include "x_memory.hpp"
#include "x_plus.hpp"
#include "x_stdio.hpp"

#include "Q_Engine.hpp"
#include "Q_GC.hpp"

#include "GC_Video.hpp"


////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////

#define FIFO_SIZE (256 * 1024)

#if defined( X_DEBUG )
//#define ENABLE_GAMECUBE_SCREENSHOT
#endif

////////////////////////////////////////////////////////////////////////////
// STATIC-GLOBAL VARIABLES
////////////////////////////////////////////////////////////////////////////

static void*            s_pGXData              = NULL;
static void*            s_pEngineFrameBuffer1  = NULL;
static void*            s_pEngineFrameBuffer2  = NULL;
static void*            s_pEngineCurrentBuffer = NULL;
static void*            s_pDefaultFifo         = NULL;
static GXFifoObj*       s_pDefaultFifoObj      = NULL;
static GXRenderModeObj* s_pEngineRenderMode    = NULL;
static GXRenderModeObj  s_EngineRenderMode;
static xbool            s_bClearEFB            = GX_TRUE;


////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

err VIDEO_InitModule( void )
{
    VIInit();

    //--- Setup desired frame buffer type
#if defined( PAL_VERSION )
    s_EngineRenderMode  = GXPal528IntDf;
    s_EngineRenderMode.efbHeight = 480;
#else
    s_EngineRenderMode  = GXNtsc480IntDf;
	s_EngineRenderMode.viXOrigin = (VI_MAX_WIDTH_NTSC - 672)/2;
	s_EngineRenderMode.viWidth   = 672;
#endif
    //s_EngineRenderMode  = GXNtsc480IntAa;
    s_pEngineRenderMode = &s_EngineRenderMode;

    VIConfigure( s_pEngineRenderMode );

    //--- Calculate propper size of external frame buffer
    u32 fbSize = VIPadFrameBufferWidth( s_pEngineRenderMode->fbWidth ) * s_pEngineRenderMode->xfbHeight * (u32)VI_DISPLAY_PIX_SZ;
    fbSize = ALIGN_32( fbSize );

    //--- allocate raw chunk for XFB and FIFO
    s_pGXData = (void*)x_malloc( (fbSize * 2) + FIFO_SIZE );
    ASSERT( s_pGXData != NULL );

    //--- set pointers to data
    s_pEngineFrameBuffer1 = s_pGXData;
    s_pEngineFrameBuffer2 = (void*)((byte*)s_pGXData + fbSize);

    s_pEngineCurrentBuffer = s_pEngineFrameBuffer2;

    //--- Init graphics fifo(will be aligned to 32B since XFB size is multiple of 32)
    s_pDefaultFifo    = (void*)((byte*)s_pGXData + (fbSize * 2));
    s_pDefaultFifoObj = GXInit( s_pDefaultFifo, FIFO_SIZE );

    //=== Setup VI

    // These are all necessary codes that can reflect descriptions
    // of a rendering mode object into the GX configuration.
    GXSetViewport      ( 0.0f, 0.0f, (f32)s_pEngineRenderMode->fbWidth, (f32)s_pEngineRenderMode->xfbHeight, 0.0f, 1.0f );
    GXSetScissor       ( 0, 0, (u32)s_pEngineRenderMode->fbWidth, (u32)s_pEngineRenderMode->efbHeight );
    GXSetDispCopySrc   ( 0, 0, s_pEngineRenderMode->fbWidth, s_pEngineRenderMode->efbHeight );
    GXSetDispCopyDst   ( s_pEngineRenderMode->fbWidth, s_pEngineRenderMode->xfbHeight );
    GXSetDispCopyYScale( (f32)(s_pEngineRenderMode->xfbHeight) / (f32)(s_pEngineRenderMode->efbHeight) );

    VIDEO_GC_EnableTextureCopyFiltering( TRUE );

    //GXSetDither( GX_FALSE );

    GXSetCullMode( GX_CULL_NONE );

    if( s_pEngineRenderMode->aa )
    {
        GXSetPixelFmt( GX_PF_RGB565_Z16, GX_ZC_LINEAR );
    }
    else
    {
       GXSetPixelFmt( GX_PF_RGB8_Z24, GX_ZC_LINEAR );
       //GXSetPixelFmt( GX_PF_RGBA6_Z24, GX_ZC_LINEAR );
    }


    //--- Double buffering initialization
    VISetNextFrameBuffer( s_pEngineFrameBuffer1 );
    s_pEngineCurrentBuffer = s_pEngineFrameBuffer2;

    //--- Tell VI device driver to write the current VI settings so far
    VIFlush();
    VISetBlack( FALSE );
    VIFlush();

    //--- Wait for retrace to start first frame
    VIWaitForRetrace();

    // Because of hardware restriction, we need to wait one more 
    // field to make sure mode is safely changed when we change
    // INT->DS or DS->INT. (VIInit() sets INT mode as a default)
    if( (u32)s_pEngineRenderMode->viTVmode & VI_NON_INTERLACE )
        VIWaitForRetrace();

    //GXSetDispCopyGamma( GX_GM_1_0 );

    //--- Clear out both buffers before we start; since the GXCopyDisp
    //    function clears the EFB AFTER the copy, we call it twice on
    //    XFB 1 to make sure it's cleared to black.
    GXColor Black = { 0, 0, 0, 255 };
    GXSetColorUpdate( GX_TRUE );
    GXSetCopyClear( Black, GX_MAX_Z24 );
    GXCopyDisp( s_pEngineFrameBuffer1, GX_TRUE );
    GXDrawDone();
    GXCopyDisp( s_pEngineFrameBuffer2, GX_TRUE );
    GXDrawDone();
    GXCopyDisp( s_pEngineFrameBuffer1, GX_TRUE );
    GXDrawDone();


    s_bClearEFB = GX_TRUE;

    return ERR_SUCCESS;
}

//==========================================================================

void VIDEO_KillModule( void )
{
    x_free( s_pGXData );

    s_pGXData              = NULL;
    s_pEngineFrameBuffer1  = NULL;
    s_pEngineFrameBuffer2  = NULL;
    s_pEngineCurrentBuffer = NULL;
    s_pDefaultFifo         = NULL;
}

//==========================================================================

xbool VIDEO_IsFieldMode( void )
{
    return (s_pEngineRenderMode->field_rendering == GX_TRUE) ? TRUE : FALSE;
}


//==========================================================================
void VIDEO_GC_EnableTextureCopyFiltering( xbool bEnable )
{
    if( bEnable )
    {
        GXSetCopyFilter( s_pEngineRenderMode->aa, s_pEngineRenderMode->sample_pattern, GX_TRUE, s_pEngineRenderMode->vfilter );
    }
    else
    {
        GXSetCopyFilter( GX_FALSE, NULL, GX_FALSE, NULL );
    }
}


//==========================================================================

void GC_SetEFBClear( xbool bClearEachFrame )
{
    s_bClearEFB = bClearEachFrame ? GX_TRUE : GX_FALSE;
}

//==========================================================================

void VIDEO_PreRender( void )
{
    //--- Set up default viewport
    //GC_SetViewport( 0.0f, 0.0f, (f32)s_pEngineRenderMode->fbWidth, (f32)s_pEngineRenderMode->xfbHeight );

    //--- Invalidate vertex cache in GP
    GXInvalidateVtxCache();

    GXInvalidateTexAll();
}

//==========================================================================

#ifdef ENABLE_GAMECUBE_SCREENSHOT
    static void SCREENSHOTService( void* bufferXFB );
#else
    #define SCREENSHOTService(x)
#endif

//--------------------------------------------------------------------------

void VIDEO_PageFlip( xbool bClearScreen )
{
    //--- Wait until everything is drawn to the EFB.
    GXDrawDone();

    SCREENSHOTService( s_pEngineCurrentBuffer );

    //--- Copy Embedded Frame Buffer(EFB) to one of our external frame buffers(XFB)
    if( bClearScreen )
        GXCopyDisp( s_pEngineCurrentBuffer, (GXBool)s_bClearEFB );

    //--- Wait until everything is drawn and copied into XFB.
    GXDrawDone();

    VISetNextFrameBuffer( s_pEngineCurrentBuffer );

    VIFlush();
    VIWaitForRetrace();

    //--- Swap buffers
    if( s_pEngineCurrentBuffer == s_pEngineFrameBuffer1 )
        s_pEngineCurrentBuffer = s_pEngineFrameBuffer2;
    else
        s_pEngineCurrentBuffer = s_pEngineFrameBuffer1;


    VIDEO_PreRender();
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// GAMECUBE SCREENSHOT CODE
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#ifdef ENABLE_GAMECUBE_SCREENSHOT

//---------------------------------------------------------------------------
//  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.
//
//  These coded instructions, statements, and computer programs contain
//  proprietary information of Nintendo of America Inc. and/or Nintendo
//  Company Ltd., and are protected by Federal copyright law.  They may
//  not be disclosed to third parties or copied or duplicated in any form,
//  in whole or in part, without the prior written consent of Nintendo.
//
//  Created 05-25-01 by Steve Rabin
//
//  Code from screenshot.h and screenshot.c
//
//---------------------------------------------------------------------------

////////////////////////////////
// INCLUDES
////////////////////////////////

#include "x_memory.hpp"
#include "x_plus.hpp"

#undef WIN32
#include "dolphin/hio.h"

////////////////////////////////
// DEFINES, CONSTANTS, TYPEDEFS
////////////////////////////////

#define WIDTH_SCREENSHOT        640
#define HEIGHT_SCREENSHOT       480

#define SIZE_SCREENSHOT_RGB     (WIDTH_SCREENSHOT*HEIGHT_SCREENSHOT*3)
#define SIZE_SCREENSHOT_YUV     (WIDTH_SCREENSHOT*HEIGHT_SCREENSHOT*2)

#define XFB_BASE                1
#define XFB_RANGE               6
#define EFB_BASE                11
#define EFB_RANGE               9
#define PING_BASE               20
#define PING_RANGE              9
#define MINIMIZE_BUFFER_TRUE    35
#define MINIMIZE_BUFFER_FALSE   36


enum GRAB_STATUS
{
    GRAB_NOT_TRANSFERRING,
    GRAB_TRANSFERRING
};


typedef void*(*SCREENSHOTAllocator)  ( u32   size );
typedef void (*SCREENSHOTDeallocator)( void* block );

////////////////////////////////
// STATIC VARIABLES
////////////////////////////////

static xbool g_connected       = FALSE;
static u32   g_received_signal = 0;
static xbool g_mail_waiting    = FALSE;
static xbool g_minimize_buffer = FALSE;
static s32   g_usb_channel     = -1;
static u8*   g_data            = NULL;


////////////////////////////////
// IMPLEMENTATION
////////////////////////////////

static BOOL HostIOEnumCallback( s32 chan )
{
    g_usb_channel = chan;
    return( FALSE );
}

//==========================================================================

static void HostIOCallback( void )
{
    //Mail is waiting for us - don't look at it yet
    g_mail_waiting = TRUE;
}

//==========================================================================

static void CopyoutEFB( u8* image_buffer, u32 width, u32 height )
{
    u16 i, j;
    u32 color;

    for( j = 0; j < height; j++ )
    {
        for( i = 0; i < width; i++ )
        {
            GXPeekARGB( i, j, &color );
            image_buffer[2] = (u8)(color & 0x000000FF);
            image_buffer[1] = (u8)((color & 0x0000FF00) >> 8);
            image_buffer[0] = (u8)((color & 0x00FF0000) >> 16);
            image_buffer += 3;
        }
    }
}

//==========================================================================

static void CopyoutPortionEFB( u8* image_buffer, u32 width, u32 startByte, u32 totalBytes )
{
    u32 color;

    u16 x = (u16)((u32)(startByte/3) % width);
    u16 y = (u16)((u32)(startByte/3) / width);
    u32 component = startByte % 3;

    u32 count = 0;
    while( count < totalBytes )
    {
        GXPeekARGB( x, y, &color );

        if( component == 0 )
        {
            count++;
            image_buffer[0] = (u8)((color & 0x00FF0000) >> 16);
        }
        if( component <= 1 && count < totalBytes )
        {
            count++;
            image_buffer[1 - component] = (u8)((color & 0x0000FF00) >> 8);
        }
        if( component <= 2 && count < totalBytes )
        {
            count++;
            image_buffer[2 - component] = (u8)(color & 0x000000FF);
        }
        image_buffer += 3 - component;
        component = 0;

        x++;
        if( x >= width )
        {
            x = 0;
            y++;
        }
    }
}

//==========================================================================

static void TakeScreenshotEFB( void )
{
    if( g_data == NULL )
    {
        g_data = (u8*)x_malloc( SIZE_SCREENSHOT_RGB );
        ASSERT( g_data != NULL );
    }

    CopyoutEFB( g_data, WIDTH_SCREENSHOT, HEIGHT_SCREENSHOT );
    DCFlushRange( &g_data[0], SIZE_SCREENSHOT_RGB );

    x_printf( "SCREENSHOT: Saved off screenshot to local memory.\n" );
}

//==========================================================================

static void TakeScreenshotXFB( void* bufferXFB )
{
    if( g_data == NULL )
    {
        g_data = (u8*)x_malloc( SIZE_SCREENSHOT_YUV );
        ASSERT( g_data != NULL );
    }

    x_memcpy( g_data, bufferXFB, SIZE_SCREENSHOT_YUV );
    DCFlushRange( &g_data[0], SIZE_SCREENSHOT_YUV );

    x_printf( "SCREENSHOT: Saved off screenshot to local memory.\n" );
}

//==========================================================================

static void WriteScreenshotPortionEFBtoUSB( u32 chunk )
{
    #define SIZE_1K     1024
    #define SIZE_100K   (SIZE_1K * 100)

    u32 i;

    if( g_data == NULL )
    {
        g_data = (u8*)x_malloc( SIZE_1K );
        ASSERT( g_data != NULL );
    }

    for( i = 0; i < 100; i++ )
    {
        CopyoutPortionEFB( g_data, WIDTH_SCREENSHOT, (chunk * SIZE_100K) + (i * SIZE_1K), SIZE_1K );
        DCFlushRange( &g_data[0], SIZE_1K );

        while( !HIOWrite( 0x00000500 + (i*SIZE_1K), g_data, SIZE_1K ) )
        {
            //Spin until communication is successful
        }
    }
}

//==========================================================================

static GRAB_STATUS GrabChunk( u32 chunk, void* bufferXFB )
{
    if( chunk >= XFB_BASE && chunk < XFB_BASE + XFB_RANGE )
    {
        void* buffer = 0;

        if( chunk == XFB_BASE )
        {   //save off XFB screenshot
            if( !g_minimize_buffer )
            {
                TakeScreenshotXFB( bufferXFB );
            }
        }

        //transfer chunk
        if( g_minimize_buffer )
        {
            u8* fb = (u8*)bufferXFB;
            buffer = (void*)( &fb[100*1024*(chunk - XFB_BASE)] );
        }
        else
        {
            buffer = (void*)( &g_data[100*1024*(chunk - XFB_BASE)] );
        }

        while( !HIOWrite( 0x00000500, buffer, 100*1024 ) )
        {
            //Spin until communication is successful
        }

        x_printf( "SCREENSHOT: Wrote chunk #%d.\n", chunk );

        //communicate that the chunk is ready to be read
        while( !HIOWriteMailbox( chunk ) )
        {
            //Spin until communication is successful
        }

        x_printf( "SCREENSHOT: Notify of write chunk #%d put in mailbox.\n", g_received_signal );
    }
    else if( chunk >= EFB_BASE && chunk < EFB_BASE + EFB_RANGE )
    {
        void* buffer;

        if( chunk == EFB_BASE )
        {   //save off EFB screenshot
            if( !g_minimize_buffer )
            {
                TakeScreenshotEFB();
            }
        }

        if( g_minimize_buffer )
        {
            WriteScreenshotPortionEFBtoUSB( chunk - EFB_BASE );
        }
        else
        {   //transfer chunk
            buffer = (void*)( &g_data[100*1024*(chunk - EFB_BASE)] );
            while( !HIOWrite( 0x00000500, buffer, 100*1024 ) )
            {
                //Spin until communication is successful
            }
        }

        x_printf( "SCREENSHOT: Wrote chunk #%d.\n", chunk );

        //communicate that the chunk is ready to be read
        while( !HIOWriteMailbox( chunk ) )
        {
            //Spin until communication is successful
        }
        x_printf( "SCREENSHOT: Notify of write chunk #%d put in mailbox.\n", g_received_signal );
    }

    if( chunk == XFB_BASE + XFB_RANGE - 1 ||
        chunk == EFB_BASE + EFB_RANGE - 1 )
    {   //free screenshot buffer space
        if( g_data != NULL )
        {
            x_free( g_data );
            g_data = NULL;
        }
        return( GRAB_NOT_TRANSFERRING );
    }
    else
    {
        return( GRAB_TRANSFERRING );
    }
}

//==========================================================================

static void CheckMail( void* bufferXFB )
{
    u32         temp        = 0;
    u32         escapeCount = 0;
    GRAB_STATUS status      = GRAB_TRANSFERRING;

    while( status == GRAB_TRANSFERRING )
    {
        escapeCount++;
        if( g_mail_waiting )
        {
            if( HIOReadMailbox( &temp ) )
            {
                g_mail_waiting = FALSE;
                status = GRAB_NOT_TRANSFERRING;

                if( temp >= PING_BASE && temp < PING_BASE + PING_RANGE )
                {   //ping message received
                    while( !HIOWriteMailbox( temp ) )
                    {
                        //Spin until communication is successful
                    }
                    x_printf( "SCREENSHOT: Sent ping back.\n" );
                }
                else if( (temp >= XFB_BASE && temp < XFB_BASE + XFB_RANGE) || 
                         (temp >= EFB_BASE && temp < EFB_BASE + EFB_RANGE) )
                {   //request for screen capture
                    escapeCount = 0;
                    g_received_signal = temp;
                    x_printf( "SCREENSHOT: Grab screenshot request.\n" );
                    status = GrabChunk( g_received_signal, bufferXFB );
                }
                else if( temp == MINIMIZE_BUFFER_TRUE )
                {   //request to use minimal memory (100K), however game pauses
                    g_minimize_buffer = TRUE;
                }
                else if( temp == MINIMIZE_BUFFER_FALSE )
                {   //request to not use minimal memory and keep game running
                    g_minimize_buffer = FALSE;
                }
            }
        }
        if( !g_minimize_buffer )
        {   //Escape loop since were not pausing the game to transfer
            return;
        }
        if( g_minimize_buffer && escapeCount > 150000000 )
        {   //The PC screenshot app has died - escape
            if( g_data != NULL)
            {
                x_free( g_data );
                g_data = NULL;
            }
            return;
        }
    }
}

//==========================================================================

static xbool ConnectToUSB( void )
{
    if( !g_connected )
    {
        if( HIOEnumDevices( HostIOEnumCallback ) )
        {
            if( g_usb_channel >= 0 )
            {
                if( HIOInit( g_usb_channel, HostIOCallback ) )
                {
                    u32 temp = 0;
                    HIOReadMailbox( &temp );
                    g_connected = TRUE;
                    x_printf( "SCREENSHOT: USB connected\n" );
                }
            }
        }
    }

    return( g_connected );
}

//==========================================================================

static void SCREENSHOTService( void* bufferXFB )
{
    if( !g_connected )
    {
        ConnectToUSB();
    }
    else if( g_mail_waiting )
    {
        CheckMail( bufferXFB );
    }
}

//==========================================================================


#endif //ENABLE_GAMECUBE_SCREENSHOT
////////////////////////////////////////////////////////////////////////////
// END GAMECUBE SCREENSHOT CODE
////////////////////////////////////////////////////////////////////////////
