/*****************************************************************
ps2_lg_capture.c
GameSpy Voice SDK
Richard "Huckster" Rice

Copyright 2002 GameSpy Industries, Inc.

18002 Skypark Circle
Irvine, California 92614
949.798.4200 (Tel)
949.798.4299 (Fax)
devsupport@gamespy.com

********************************************************************
Please see the GameSpy Voice SDK documentation for more information
*******************************************************************/
#include <liblgaud.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sifdev.h>
#include <libgraph.h>
#define GSI_MEM_ONLY
#include "../../nonport.h"
#include "ps2_lg_capture.h"

///////////////////////////////////////////////////////////////////////////
//
//  IRX DEFINES
//
//  The location of the IRX files depend on the target.  
//
///////////////////////////////////////////////////////////////////////////


#ifdef TARGET_PS2_DVD
#define HOST_PREFIX "cdrom0:"
#define SCE_PATH ""
#define IRX_PATH "\\SYSTEM"
#else // !TARGET_PS2_DVD

#ifdef TARGET_PS2_USB
extern char HostPrefix[];
#define HOST_PREFIX HostPrefix
#else  // !TARGET_PS2_USB

#define HOST_PREFIX "host0:"
#endif  // TARGET_PS2_USB

#define SCE_PATH "\\tnt\\3rdparty\\ps2\\sce"
#define IRX_PATH "\\iop\\modules"
#endif  // TARGET_PS2_DVD


// Can't use GSI_UNUSED because of GSI_MEM_ONLY
#ifdef __MWERKS__
#define GSI_UNUSED(a) (a)
#else
#define GSI_UNUSED(a)
#endif

#define INPUTLENGTH 			(2160<<1) // Bytes in each wave buffer
#define NUM_IN_BUFFERS			4
#define IN_BUFFER_SIZE 			(INPUTLENGTH*NUM_IN_BUFFERS) 


#define MAX_IN_DEVICES			10

#define IN_SAMPLING_FREQUENCY 	8000
#define IN_SAMPLING_BITS 		16
#define IN_SAMPLING_CHANNELS 	1

#define OUT_SAMPLING_FREQUENCY 	8000
#define OUT_SAMPLING_BITS		16
#define OUT_SAMPLING_CHANNELS	1
#define OUT_SPOOLERBUF 			10 * OUT_SAMPLING_FREQUENCY * OUT_SAMPLING_CHANNELS * (OUT_SAMPLING_BITS/8)

#define RECORDLENGTH 			50     // milliseconds  
#define READCHUNK 				9000   // enough to hold more than a few frames of audio

#define PLAYBACKLENGTH			50
#define PLAYCHUNK				9000

int g_usb_device = LGAUD_INVALID_DEVICE;
lgAudDeviceDesc g_usb_device_desc;

static CaptureCallback callback = NULL;
//#define INPUTLENGTH (2160*2) // Bytes in each wave buffer
//#define NUMINPUTBUFFERS 10     // Number of buffers we queue

static voiceBool g_isRecording = voiceFalse;
static int g_record_volume = 85;
voiceBool g_isPlaying = voiceFalse;
int g_playback_volume = 80;

static short *inbuffer;
static unsigned int inbuffer_len;

#if defined( X_DEBUG )
	#define DBG_PRINT( exp )	exp
#else
	#define DBG_PRINT( exp )
#endif

//////////////////////////////////////////////////////////////////////
// SifLoadModule
//
// Loads an IRX module
//////////////////////////////////////////////////////////////////////
//static void SifLoadModule(char *module, char *arg, int arglen)
//{
//    char hostmod[256];
//    int delay = 0;
//
//	sprintf( hostmod,	"%s%s%s\\%s",	HOST_PREFIX, ScePath, IRX_PATH, module );
//
//    while(1)    
//    {
//        DBG_PRINT(printf("trying to load >%s<\n", hostmod));
//        if(sceSifLoadModule(hostmod, arglen, arg) >= 0)
//        {
//            return;
//        }
//    
//        DBG_PRINT(printf("Failed to load >%s<\n", module));
//    
//        // this is a really stupid delay loop...
//        // hopefully not stupid enough to be optimized out...
//        for(delay = 0; delay < 1000*1000; delay++)
//        {
//            delay = delay;
//        }
//    }
//}

//////////////////////////////////////////////////////////////////////
// LoadModules
//
// Loads the irx modules
//////////////////////////////////////////////////////////////////////
//static int LoadModules()
//{
//    char usbd_argv[] = "conf=2048";
//
//    //sceSifInitRpc(0);
//    SifLoadModule("USBD.IRX", usbd_argv, sizeof(usbd_argv));
//    SifLoadModule(LGAUD_IRXNAME, 0, 0);
//
//    return 1;
//}

//////////////////////////////////////////////////////////////////////
// ShowDeviceSupport
//
// Displays the capabilities of the selected device 
//////////////////////////////////////////////////////////////////////
static void ShowDeviceSupport()
{
    int i;

    if (g_usb_device == LGAUD_INVALID_DEVICE)
        DBG_PRINT(printf("\n\n"));
    else
    {
        DBG_PRINT(printf("\n"));
        DBG_PRINT(printf("Supported Recording Formats"));
        DBG_PRINT(printf("---------------------------"));
        DBG_PRINT(printf("\n"));
        
        for (i = 0; i < g_usb_device_desc.RecordingFormatsCount; i++)
        {
            DBG_PRINT(printf("%dhz - %dhz %d bit %d channels\n",
                g_usb_device_desc.RecordingFormats[i].LowerSamplingRate,
                g_usb_device_desc.RecordingFormats[i].HigherSamplingRate,
                g_usb_device_desc.RecordingFormats[i].BitResolution,
                g_usb_device_desc.RecordingFormats[i].Channels));
        }           

        DBG_PRINT(printf("\n"));
        DBG_PRINT(printf("Supported Playback Formats"));
        DBG_PRINT(printf("---------------------------"));
        DBG_PRINT(printf("\n"));
        
        for (i = 0; i < g_usb_device_desc.PlaybackFormatsCount; i++)
        {
            DBG_PRINT(printf("%dhz - %dhz %d bit %d channels\n",
                g_usb_device_desc.PlaybackFormats[i].LowerSamplingRate,
                g_usb_device_desc.PlaybackFormats[i].HigherSamplingRate,
                g_usb_device_desc.PlaybackFormats[i].BitResolution,
                g_usb_device_desc.PlaybackFormats[i].Channels));
        }
    }
}

//////////////////////////////////////////////////////////////////////
// FindAndOpenHeadsetDevice
//
// Returns the first attached headset device
//////////////////////////////////////////////////////////////////////
static int FindAndOpenHeadsetDevice()
{
    int ret = LGAUD_SUCCESS;
    int enumret;

    g_usb_device = LGAUD_INVALID_DEVICE;

	enumret = lgAudEnumerate(0, &g_usb_device_desc);
    if(LGAUD_SUCCESS == enumret)
    {
        lgAudOpenParam openParam;

     	memset(&openParam, 0, sizeof(lgAudOpenParam));
     	   
        openParam.Mode = LGAUD_MODE_PLAYBACK | LGAUD_MODE_RECORDING;

		openParam.PlaybackFormat.Channels = OUT_SAMPLING_CHANNELS;
		openParam.PlaybackFormat.BitResolution = OUT_SAMPLING_BITS;
		openParam.PlaybackFormat.SamplingRate = OUT_SAMPLING_FREQUENCY;
		openParam.PlaybackFormat.BufferMilliseconds = PLAYBACKLENGTH;
        
        openParam.RecordingFormat.Channels = IN_SAMPLING_CHANNELS;
        openParam.RecordingFormat.BitResolution = IN_SAMPLING_BITS;
        openParam.RecordingFormat.SamplingRate = IN_SAMPLING_FREQUENCY;
        openParam.RecordingFormat.BufferMilliseconds = RECORDLENGTH;
        
        if ((ret = lgAudOpen(0, &openParam, &g_usb_device)) == LGAUD_SUCCESS)
        {
            
            DBG_PRINT(printf("Opened USB Audio Device\n"));
            ShowDeviceSupport();
            DBG_PRINT(printf("\n\n"));
            
			DBG_PRINT(printf("Playback: opened device %d with format %dHz, %dbit, %dchannel, %dms buffer\n",
				g_usb_device,
				openParam.PlaybackFormat.SamplingRate,
				openParam.PlaybackFormat.BitResolution,
				openParam.PlaybackFormat.Channels,
				openParam.PlaybackFormat.BufferMilliseconds));
            
            DBG_PRINT(printf("Record: opened device %d with format %dHz, %dbit, %dchannel, %dms buffer\n",
                g_usb_device,
                openParam.RecordingFormat.SamplingRate,
                openParam.RecordingFormat.BitResolution,
                openParam.RecordingFormat.Channels,
                openParam.RecordingFormat.BufferMilliseconds));
            DBG_PRINT(printf("\n\n"));
       
            sceGsSyncV(0);
            lgAudSetRecordingVolume(g_usb_device, LGAUD_CH_BOTH, g_record_volume);
            lgAudSetPlaybackVolume(g_usb_device, LGAUD_CH_BOTH, g_playback_volume);
            return g_usb_device;
        }
        else
            DBG_PRINT(printf("ERROR: lgAudOpen failed (0x%x)\n", ret));
    }
    
    return LGAUD_INVALID_DEVICE;
}
//////////////////////////////////////////////////////////////////////
// StartRecording
//
// Initiates a recording session
//////////////////////////////////////////////////////////////////////
static int StartRecording()
{
    if(g_usb_device == LGAUD_INVALID_DEVICE)
        return LGAUD_ERROR;

    if (g_isRecording == 1)
        return LGAUD_SUCCESS;

    if (lgAudStartRecording(g_usb_device) == LGAUD_SUCCESS)
    {        

        DBG_PRINT(printf("Start Record\n"));

        g_isRecording = (voiceBool)1;

        return LGAUD_SUCCESS;
    }    

    return LGAUD_ERROR;
}


//////////////////////////////////////////////////////////////////////
// StopRecording
//
// Leaves a recording session
//////////////////////////////////////////////////////////////////////
static int StopRecording()
{
    if (g_isRecording == 0)
        return LGAUD_SUCCESS;
    
    if(g_usb_device == LGAUD_INVALID_DEVICE)
        return LGAUD_ERROR;
    
    lgAudStopRecording(g_usb_device);
        
    g_isRecording = (voiceBool)0;

    return LGAUD_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// Capture Support Functions
//////////////////////////////////////////////////////////////////////
static void* lgAudMalloc( unsigned int size )
{
	return x_malloc( size );
}

static void lgAudFree( void* pData )
{
	x_free( pData );
}


static voiceBool CaptureOpen(CaptureCallback capturecallback)
{
	int count = 0;
	DBG_PRINT(printf("CaptureOpen()\n"));

/*  These are pre-loaded in PS2_Engine.cpp now.
    if(!LoadModules())
    {
    	DBG_PRINT(printf("Modules not loaded\n"));
    	return voiceFalse;
    }
*/    
	lgAudInit(lgAudMalloc, lgAudFree);

	// need to find out why we can't detect the device the first time. 
	// it takes an average of 30 tries before the attached headset is detected.
	
    while (count < 150 && (g_usb_device = FindAndOpenHeadsetDevice()) == LGAUD_INVALID_DEVICE)
    {
    	count++;
    	//DBG_PRINT(printf("count = %d\n", count));
        sceGsSyncV(0);
	}
	if(g_usb_device == LGAUD_INVALID_DEVICE)
	{
		DBG_PRINT(printf("Unable to find headset device\n"));
		return voiceFalse;
	}
	callback = capturecallback;
	inbuffer_len = 0;
	inbuffer = (short *)gsimalloc(IN_BUFFER_SIZE);
	
	return voiceTrue;
}

static void CaptureClose(void)
{
    lgAudClose(g_usb_device);
   
    g_usb_device = LGAUD_INVALID_DEVICE;
    gsifree(inbuffer);
    inbuffer = NULL;
}

static void CaptureProcess(void)
{
	int i;
	if(inbuffer_len >= INPUTLENGTH && callback)
	{
		AudioSample sample;
		sample.data = inbuffer;
		sample.length = INPUTLENGTH>>1;
		inbuffer_len -= INPUTLENGTH;
		callback(&sample);
		for(i = 0; i < (INPUTLENGTH>>1); i++)
			inbuffer[i] = inbuffer[i+(INPUTLENGTH>>1)];		
	}
	if(g_isRecording)
	{
		int readlen = IN_BUFFER_SIZE-inbuffer_len;
		
		if(lgAudRead(g_usb_device, LGAUD_BLOCKMODE_NOT_BLOCKING, (u_char*)&inbuffer[inbuffer_len>>1], &readlen) == LGAUD_SUCCESS)
		{
			//DBG_PRINT(printf("lgAudRead: read %d bytes and put it in the %d byte stack\n", readlen, inbuffer_len));
			inbuffer_len += readlen;
		}
	}
}

static unsigned int CaptureGetNumDevices(void)
{
	return 1;
}

static const char * CaptureGetDeviceName(unsigned int device)
{
	if(device == 0)
		return "Logitech Headset";

	return NULL;
}

static void CaptureSetDevice(unsigned int device)
{
	GSI_UNUSED(device);
}

static unsigned int CaptureGetDevice(void)
{
	return 0;
}

static void CaptureRecordStart(void)
{
	StartRecording();
}

static void CaptureRecordStop(void)
{
	StopRecording();
}

VoiceCapture *Ps2LgCapture()
{
	static VoiceCapture desc;

	desc.open = CaptureOpen;
	desc.close = CaptureClose;
	desc.process = CaptureProcess;
	desc.get_num_devices = CaptureGetNumDevices;
	desc.get_device_name = CaptureGetDeviceName;
	desc.set_device = CaptureSetDevice;
	desc.get_device = CaptureGetDevice;
	desc.record_start = CaptureRecordStart;
	desc.record_stop = CaptureRecordStop;

	return &desc;
}
