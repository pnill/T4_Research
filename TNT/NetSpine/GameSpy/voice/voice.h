/*****************************************************************
voice.h
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
#ifndef _VOICE_H
#define _VOICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "voiceCodec.h"
#include "voiceHardware.h"
#include "voiceChannel.h"

/*************************************************************************
* VOICE CHANNEL INITIALIZE / CLEANUP FUNCTIONS
*************************************************************************/

/*------------------------------------------------------------------------
VoiceInitialize
--------------------------------------------------------------------------
VoiceInitialize initializes variables and memory that the voice
system will use. VoiceInitialize must be called before any other
voice API functions are called from voiceChannel.h.
--------------------------------------------------------------------------
voicecapture  | voicecapture must be a valid pointer obtained
	          | by a call to the capture device's initializer.
	          | For example, a valid voicecapture pointer
	          | can be obtained through a call to 
	          | WindowsWavCapture() which returns a pointer
	          | that references the Windows wav capture hardware.
	          | The Voice SDK uses voicecapture to interface
	          | with the system's capture hardware.
voiceplayback | voiceplayback must be a valid pointer obtained
	          | by a call to the playback device's initializer
	          | A valid voiceplayback pointer is obtained in the
	          | same way as voicecapture, but with a call to 
	          | the playback device initializer.
return        | If successful, VoiceInitialize returns the listening
	          | port used by this session of the Voice SDK. Any
	          | channels created with the Voice SDK and any channel
	          | members will use this port to listen for incoming
	          | connections using this port.
	          | If initilization fails, then VoiceInitialize will
	          | return 0.
------------------------------------------------------------------------*/
unsigned short VoiceInitialize(VoiceCapture *voicecapture, 
                               VoicePlayback *voiceplayback);
/*------------------------------------------------------------------------
VoiceCleanup
--------------------------------------------------------------------------
VoiceCleanup shuts down all remaining connections and frees any 
allocated memory that was used by the Voice SDK.
------------------------------------------------------------------------*/
void VoiceCleanup(void);

/*------------------------------------------------------------------------
VoiceIsInitialized
--------------------------------------------------------------------------
VoiceIsInitialized is used to determine if the Voice SDK is initialized.
--------------------------------------------------------------------------
return        | VoiceIsInitialized returns voiceTrue if the SDK has been
              | initialized, voiceFalse if it has not.
------------------------------------------------------------------------*/
voiceBool VoiceIsInitialized();

/*------------------------------------------------------------------------
VoiceGetPort
--------------------------------------------------------------------------
VoiceGetPort is used to get the port the SDK is using.
--------------------------------------------------------------------------
return        | VoiceGetPort returns the port being used by the SDK.
------------------------------------------------------------------------*/
unsigned short VoiceGetPort();

#ifdef __cplusplus
}
#endif

#endif /* _VOICE_H */

