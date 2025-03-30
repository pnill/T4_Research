/*****************************************************************
voiceCodec.h
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
#ifndef _VOICE_CODEC
#define _VOICE_CODEC

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
* VOICECODEC REFERENCE POINTER
*************************************************************************/

typedef void* CodecState; // Voice Codec State

/*************************************************************************
* VOICECODEC CALLBACK DEFINITIONS
*************************************************************************/

typedef enum
{
	stateEncode,
	stateDecode
} CodecStateType;

typedef CodecState (*CallbackCodecCreateState)(CodecStateType type);
typedef void (*CallbackCodecDestroyState)(CodecState state);
typedef int (*CallbackCodecEncode)(unsigned char *packet, short *buffer, CodecState state);
typedef int (*CallbackCodecDecode)(short *out, unsigned char *in, CodecState state);
typedef void (*CallbackCodecCleanup)(void);

/*************************************************************************
* VOICECODEC CALLBACK STRUCTURE
*************************************************************************/
typedef struct
{
	CallbackCodecCreateState		create_state;
	CallbackCodecDestroyState		destroy_state;
	CallbackCodecEncode 			encode;
	CallbackCodecDecode 			decode;
	CallbackCodecCleanup			cleanup;

	unsigned short					samples_per_frame;
	unsigned short					encoded_frame_size;
	unsigned int					flags;
	char							*id;

} VoiceCodec;

/*************************************************************************
* VOICECODEC FUNCTIONS
*************************************************************************/

void VoiceCodecRegister(VoiceCodec *codec);
void VoiceCodecUnRegister(VoiceCodec *codec);
VoiceCodec *VoiceCodecLookup(const char *id);
int VoiceCodecCount();
VoiceCodec *VoiceCodecLookupIndex(int index);


#ifdef __cplusplus
}
#endif


#endif /* _VOICE_CODEC */
