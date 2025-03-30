/*****************************************************************
voiceHardware.h
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
#ifndef _VOICE_HARDWARE
#define _VOICE_HARDWARE

#include "voiceUtil.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
* VOICEHARDWARE STRUCT DEFINITIONS
*************************************************************************/
typedef struct
{
	short * data;
	unsigned int length;
} AudioSample;

/*************************************************************************
* VOICEHARDWARE CALLBACK DEFINITIONS
*************************************************************************/

typedef void (*CaptureCallback)(AudioSample *sample);
typedef voiceBool (*CallbackCaptureOpen)(CaptureCallback capturecallback);
typedef void (*CallbackCaptureClose)(void);
typedef void (*CallbackCaptureProcess)(void);
typedef unsigned int (*CallbackCaptureGetNumDevices)(void);
typedef const char * (*CallbackCaptureGetDeviceName)(unsigned int device);
typedef void (*CallbackCaptureRecordStart)(void);
typedef void (*CallbackCaptureRecordStop)(void);
typedef void (*CallbackCaptureSetDevice)(unsigned int device);
typedef unsigned int (*CallbackCaptureGetDevice)(void);

typedef void (*PlaybackCallback)(void);
typedef voiceBool (*CallbackPlaybackOpen)(PlaybackCallback playbackcallback);
typedef void (*CallbackPlaybackClose)(void);
typedef void (*CallbackPlaybackPlay)(AudioSample *sample);
typedef void (*CallbackPlaybackProcess)(void);
typedef unsigned int (*CallbackPlaybackGetNumDevices)(void);
typedef const char * (*CallbackPlaybackGetDeviceName)(unsigned int device);
typedef void (*CallbackPlaybackSetDevice)(unsigned int device);
typedef unsigned int (*CallbackPlaybackGetDevice)(void);
typedef unsigned char (*CallbackPlaybackGetVolume)(void);
typedef void (*CallbackPlaybackSetVolume)(unsigned char volume);


/*************************************************************************
* VOICEHARDWARE CALLBACK STRUCTURE
*************************************************************************/
typedef struct
{
	CallbackCaptureOpen           open;
	CallbackCaptureClose          close;
	CallbackCaptureProcess        process;
	CallbackCaptureGetNumDevices  get_num_devices;
	CallbackCaptureGetDeviceName  get_device_name;
	CallbackCaptureSetDevice      set_device;
	CallbackCaptureGetDevice      get_device;
	CallbackCaptureRecordStart    record_start;
	CallbackCaptureRecordStop     record_stop;

} VoiceCapture;

typedef struct
{
	CallbackPlaybackOpen          open;
	CallbackPlaybackClose         close;
	CallbackPlaybackPlay          play;
	CallbackPlaybackProcess       process;
	CallbackPlaybackGetNumDevices get_num_devices;
	CallbackPlaybackGetDeviceName get_device_name;
	CallbackPlaybackSetDevice     set_device;
	CallbackPlaybackGetDevice     get_device;
	CallbackPlaybackGetVolume     get_volume;
	CallbackPlaybackSetVolume     set_volume;

	unsigned int                  max_buffer_length;

} VoicePlayback;

#ifdef __cplusplus
}
#endif

#endif /* _VOICE_HARDWARE */
