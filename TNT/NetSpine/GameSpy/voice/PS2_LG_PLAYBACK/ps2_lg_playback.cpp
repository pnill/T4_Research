/*****************************************************************
ps2_lg_playback.c
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
#define GSI_MEM_ONLY
#include "../../nonport.h"
#include "ps2_lg_playback.h"

// Can't use GSI_UNUSED because of GSI_MEM_ONLY
#ifdef __MWERKS__
#define GSI_UNUSED(a) (a)
#else
#define GSI_UNUSED(a)
#endif

#define OUTPUTLENGTH 			(2160<<1) // Bytes in each wave buffer
#define NUM_OUT_BUFFERS			4
#define IN_BUFFER_SIZE 			(INPUTLENGTH*NUM_IN_BUFFERS) 

#define MAX_OUT_DEVICES			10

#if defined( X_DEBUG )
	#define DBG_PRINT( exp )	exp
#else
	#define DBG_PRINT( exp )
#endif



//static int buffer_len;
static PlaybackCallback callback = NULL;
//static PlaybackCallback callback_hold = NULL;

// since the Logitech SDK must be initialized in one place, setup is
// done in the ps2_lg_capture.c implementation file with the 
// playback related variables extern'ed here.
extern int g_usb_device;
extern voiceBool g_isPlaying;
extern int g_playback_volume;

#define OUT_BUFFER_SIZE 20480*2
short *outbuffer = NULL;
unsigned int outbuffer_len;

//////////////////////////////////////////////////////////////////////
// StartPlayback
//
// Initiates a playback session
//////////////////////////////////////////////////////////////////////
static int StartPlayback()
{
		DBG_PRINT(printf("Starting StartPlayback()\n"));

	if(g_usb_device == LGAUD_INVALID_DEVICE)
		return LGAUD_ERROR;
	
	if(g_isPlaying == 1)
		return LGAUD_SUCCESS;
	
	if(lgAudStartPlayback(g_usb_device) == LGAUD_SUCCESS)
	{
		int bytes_recorded = 0;
		
		DBG_PRINT(printf("Start playback with %d bytes recorded\n", bytes_recorded));

		g_isPlaying = (voiceBool)1;
		
		return LGAUD_SUCCESS;
	}

	return LGAUD_ERROR;
}
//////////////////////////////////////////////////////////////////////
// StopPlayback
//
// Leaves a playback session
//////////////////////////////////////////////////////////////////////
static int StopPlayback()
{
	DBG_PRINT(printf("Starting StopPlayback()\n"));

	if(g_isPlaying == 0)
		return LGAUD_SUCCESS;
		
	lgAudStopPlayback(g_usb_device);
	
	g_isPlaying = (voiceBool)0;
	
	return LGAUD_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// Playback Support Functions
//////////////////////////////////////////////////////////////////////

static unsigned int PlaybackGetNumDevices(void)
{
	return 1;
}

static const char * PlaybackGetDeviceName(unsigned int device)
{
	if(device == 0)
		return "Logitech Headset";

	return NULL;
}

static void PlaybackPlay(AudioSample *sample)
{
	if(g_isPlaying)
	{
		if(sample->length)
		{
			memcpy(outbuffer, sample->data, sample->length);
			DBG_PRINT(printf("appended %d bytes on the %d byte stack\n", sample->length, outbuffer_len));
			outbuffer_len += sample->length;
		}
	}
}

static voiceBool PlaybackOpen(PlaybackCallback playbackcallback)
{
	callback = playbackcallback;

	outbuffer_len = 0;
	outbuffer = (short *)gsimalloc(OUT_BUFFER_SIZE);
	memset(outbuffer, 0, sizeof(outbuffer));
	StartPlayback();
	return voiceTrue;
}

static void PlaybackClose(void)
{
	StopPlayback();
	gsifree(outbuffer);
	outbuffer = NULL;
}

static void PlaybackProcess(void)
{
	unsigned int i;

	if(outbuffer_len)
	{
		unsigned int writelen = outbuffer_len;
		if(lgAudWrite(g_usb_device, LGAUD_BLOCKMODE_NOT_BLOCKING, (u_char*)outbuffer, (int*)&writelen) == LGAUD_SUCCESS)
		{
			DBG_PRINT(printf("Wrote %d bytes of data\n", writelen));
			outbuffer_len -= writelen;
			for(i = 0; i < outbuffer_len >> 1; i++)
				outbuffer[i] = outbuffer[i+(writelen >> 1)];	
		}
		else
			DBG_PRINT(printf("Lost connection to device\n"));
	}

	if(!outbuffer_len && callback)
		callback();
}

static void PlaybackSetDevice(unsigned int device)
{
	GSI_UNUSED(device);
}

static unsigned int PlaybackGetDevice(void)
{
	return 0;
}

static unsigned char PlaybackGetVolume(void)
{
	return 0;
}

static void PlaybackSetVolume(unsigned char volume)
{
	GSI_UNUSED(volume);
}

VoicePlayback *Ps2LgPlayback()
{
	static VoicePlayback desc;

	desc.open = PlaybackOpen;
	desc.close = PlaybackClose;
	desc.play = PlaybackPlay;
	desc.process = PlaybackProcess;
	desc.get_num_devices = PlaybackGetNumDevices;
	desc.get_device_name = PlaybackGetDeviceName;
	desc.set_device = PlaybackSetDevice;
	desc.get_device = PlaybackGetDevice;
	desc.set_volume = PlaybackSetVolume;
	desc.get_volume = PlaybackGetVolume;
	desc.max_buffer_length = OUTPUTLENGTH;

	return &desc;
}
