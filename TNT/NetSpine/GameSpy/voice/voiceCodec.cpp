/*****************************************************************
voiceCodec.c
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
#include "voiceCodec.h"
#include "../nonport.h"

#define MAX_CODECS  255

static VoiceCodec *g_CodecList[MAX_CODECS];
static int length = 0;

void VoiceCodecRegister(VoiceCodec *codec)
{
	int i;
	// add this codec to out list of available codecs
	if(!length)
		memset(g_CodecList, 0, sizeof(g_CodecList));

	// see if this codec is already registered
	for(i=0;i<MAX_CODECS;i++)
	{
		if(g_CodecList[i] && strcmp(g_CodecList[i]->id, codec->id) == 0)
			return; // this codec is already registered
	}
	// now that we know the codec isn't registered, let's find
	// a spot for it in the list
	for(i=0;i<MAX_CODECS;i++)
	{
		if(!g_CodecList[i])
		{
			g_CodecList[i] = codec;
			length++;
			return;
		}
	}
}

void VoiceCodecUnRegister(VoiceCodec *codec)
{
	int i;

	if(!length || !codec)
		return;

	for(i=0;i<MAX_CODECS;i++)
	{
		if(g_CodecList[i] && strcmp(g_CodecList[i]->id, codec->id) == 0)
		{ 
			codec->cleanup();
			length--;
			g_CodecList[i] = g_CodecList[length];
			g_CodecList[length] = NULL;
			return;
		}
	}

}

VoiceCodec *VoiceCodecLookup(const char *id)
{
	int i;

	for(i = 0; i < MAX_CODECS; i++)
	{
		if(g_CodecList[i] && strcmp(id, g_CodecList[i]->id)==0)
			return g_CodecList[i];
	}
	return NULL;
}


int VoiceCodecCount()
{
	return length;
}

VoiceCodec *VoiceCodecLookupIndex(int index)
{
	return g_CodecList[index];
}
