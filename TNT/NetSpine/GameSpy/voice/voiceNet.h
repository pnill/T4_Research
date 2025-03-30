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
#ifndef _VOICE_NET
#define _VOICE_NET

#ifdef __cplusplus
extern "C" {
#endif

#include "../nonport.h"
#include "voiceUtil.h"

#define MAXPACKETSIZE		1400
#define NETERR_CLOSESOCKET	-1

unsigned short VoiceNetInit(void);
void VoiceNetClose(void);
void VoiceNetThink(void);
int SendPacketTCP(SOCKET s, Packet p);
int SendPacketUDP(char *address, unsigned short port, Packet p);
Packet ReceivePacketTCP(SOCKET s, int *err);
Packet ReceivePacketUDP(SOCKADDR_IN *from);
void VoiceNetCloseSocket(SOCKET s);

// 2002.Nov.23.JED - For my own instrumentation of network usage
#define INSTRUMENT_VOICE_NET_TRAFFIC   1
#if INSTRUMENT_VOICE_NET_TRAFFIC

typedef enum _VoiceNetEvent
{
	VNE_SEND,
	VNE_RECV
} VoiceNetEvent;

typedef void (*VoiceNetEventSpyFn)(VoiceNetEvent evt, SOCKET s, const char *buff, int len, int flags, void *instance);

void RegisterVoiceNetTrafficSpy(VoiceNetEventSpyFn func, void *instance);

#endif //INSTRUMENT_VOICE_NET_TRAFFIC

#ifdef __cplusplus
}
#endif


#endif /* _VOICE_NET */
