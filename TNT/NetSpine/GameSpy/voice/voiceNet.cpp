/*****************************************************************
voiceNet.c
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../nonport.h"
#include "../darray.h"
#include "voiceNet.h"
#include "voiceChannel.h"

#define MAX_PORT_RANGE		1024
#define PACKETLEN(buf)		ntohs(*((unsigned short *)buf))
#define ELEMS_TO_ALLOCATE	16

typedef struct
{
	SOCKET socket;				// socket reference
	unsigned short length;		// length of the cache data in the buffer
	char *buffer;				// contains the cache data
	unsigned short alloc_blocks;// number of allocated blocks of MAXPACKETLEN

} net_element;

extern SOCKET gTCPSocket;
extern SOCKET gUDPSocket;
extern unsigned short gPort;
static Packet g_NetPacket = NULL;

static char recvbuffer[MAXPACKETSIZE];
static DArray sendarray = NULL;
static DArray recvarray = NULL;

#if INSTRUMENT_VOICE_NET_TRAFFIC
static VoiceNetEventSpyFn NetSpyFunc = NULL;
static void * NetSpyParam = NULL;
#endif //INSTRUMENT_VOICE_NET_TRAFFIC

//////////////////////////////////////////////////////////////////////
// darray callbacks
//////////////////////////////////////////////////////////////////////

static void NetArrayFreeFn(void * elem)
{
	net_element *n_elem = (net_element *) elem;

	assert(elem);

	if(n_elem->buffer)
		gsifree(n_elem->buffer);
}

static int NetArrayCompareFn(const void *elem1, const void *elem2)
{
	net_element * n_elem1 = (net_element *) elem1;
	net_element * n_elem2 = (net_element *) elem2;

	assert(n_elem1);
	assert(n_elem2);

	return (n_elem1->socket - n_elem2->socket);
}

/////////////////////////////////////////////////////////////////////
// VoiceNet Functions
/////////////////////////////////////////////////////////////////////
unsigned short VoiceNetInit(void)
{
	SOCKADDR_IN address;
	int bFlag;
	unsigned short voiceport = DEFAULT_VOICE_PORT;
	int reuse_addr = 1;

	gTCPSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(gTCPSocket == INVALID_SOCKET)
		return 0;

	memset(&address, 0, sizeof(address));
	address.sin_family		= AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port		= htons(voiceport);

	// Try to speed up PS2 handling of hung sockets
	setsockopt(gTCPSocket, SOL_SOCKET, SO_REUSEADDR,(const char*)&reuse_addr,sizeof(reuse_addr));

	while(bind(gTCPSocket, (SOCKADDR *)&address,
				sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		if((GOAGetLastError(gTCPSocket)==WSAEADDRINUSE) && ((voiceport-DEFAULT_VOICE_PORT) < MAX_PORT_RANGE))
		{
			address.sin_port = htons(++voiceport);
			continue;
		}
		else
		{
			closesocket(gTCPSocket);
			return 0;
		}
	}

	gPort = voiceport;

	if(!SetSockBlocking(gTCPSocket, 0))
	{
		closesocket(gTCPSocket);
		return 0;
	}

	if(listen(gTCPSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		closesocket(gTCPSocket);
		return 0;
	}

	// open up the listening port
	gUDPSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(gUDPSocket == INVALID_SOCKET)
		return 0;

	bFlag = 1;
	if(setsockopt(gUDPSocket, SOL_SOCKET, SO_REUSEADDR, 
		(const char*)&bFlag, sizeof(bFlag) ) == SOCKET_ERROR)
	{
		closesocket(gUDPSocket);
		return 0;
	}

	if(bind(gUDPSocket, (SOCKADDR *)&address, 
			sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		closesocket(gTCPSocket);
		closesocket(gUDPSocket);
		return 0;
	}

	if(!SetSockBlocking(gUDPSocket, 0))
	{
		closesocket(gTCPSocket);
		closesocket(gUDPSocket);
		return 0;
	}

	sendarray = ArrayNew(sizeof(net_element), ELEMS_TO_ALLOCATE, NetArrayFreeFn);
	recvarray = ArrayNew(sizeof(net_element), ELEMS_TO_ALLOCATE, NetArrayFreeFn);

	g_NetPacket = PacketNew(PACKET_DEFAULT);

	return voiceport;
}

static int SendSocketCache(SOCKET s, int idx)
{
	net_element key;
	int ret = 0;
	int i;

	assert(s != INVALID_SOCKET);
	assert(sendarray);

	if(idx == NOT_FOUND && ArrayLength(sendarray))
	{
		// search the send array for cached data that needs to 
		// be sent on this socket
		key.socket = s;
		idx = ArraySearch(sendarray, &key, NetArrayCompareFn, 0, 0);
	}

	if(idx != NOT_FOUND) // found
	{
		net_element *n_elem;

		n_elem = (net_element *)ArrayNth(sendarray, idx);
		assert(n_elem->socket == s);

		if(n_elem->length)
		{
#if INSTRUMENT_VOICE_NET_TRAFFIC
			if( NetSpyFunc )
				NetSpyFunc(VNE_SEND, s, n_elem->buffer, n_elem->length, 0, NetSpyParam);
#endif //INSTRUMENT_VOICE_NET_TRAFFIC

			// there is cached data that needs to be sent on this socket
			ret = send(s, n_elem->buffer, n_elem->length, 0);
			if(ret > 0)
			{
				if(ret < n_elem->length)
				{
					// move remaining data to the start of the buffer
					for(i = 0; i < ret; i++)
						n_elem->buffer[i] = n_elem->buffer[ret+i];
					n_elem->length = (unsigned short)ret;
				}
				else
				{
					n_elem->length = 0;
				}
			}
		}
	}

	return ret;
}

int SendPacketTCP(SOCKET s, Packet p)
{
	int idx = NOT_FOUND;
	net_element key;
	int ret;
	char *buffer;
	int length;
	assert(s != INVALID_SOCKET);
	assert(p);
	assert(sendarray);
	
	buffer = PacketBuffer(p);
	length = PacketLength(p);

	if(ArrayLength(sendarray))
	{
		key.socket = s;
		idx = ArraySearch(sendarray, &key, NetArrayCompareFn, 0, 0);
	}

	if(idx == NOT_FOUND)
	{
		net_element new_elem;

#if 1 // set to 0 to make it easier to test cache accumulation

#if INSTRUMENT_VOICE_NET_TRAFFIC
		if( NetSpyFunc )
			NetSpyFunc(VNE_SEND, s, buffer, length, 0, NetSpyParam);
#endif //INSTRUMENT_VOICE_NET_TRAFFIC

		// first let's attempt to send the data now so
		// we don't have to allocate memory until we have to
		ret = send(s, buffer, length, 0);
		if(ret > 0)
		{
			// data was sent, now we test if it was a full or parial send

			if(ret == length)
			{
				// actually, all the data was sent so we just need to return
				return ret;
			}

			// only a piece of the data was sent, so we 
			// need to cache the unsent data

			// point the buffer the the begining of the unsent data
			// and adjust the length to the size of the remaining data
			buffer += ret;
			length -= ret;
			assert(length);
		}
#endif

		new_elem.socket = s;
		new_elem.length = (unsigned short)length;
		new_elem.alloc_blocks = (new_elem.length / MAXPACKETSIZE) + 1;

		// space is allocated only on the first send on the open socket
		new_elem.buffer = (char *)gsimalloc(MAXPACKETSIZE * new_elem.alloc_blocks);
		assert(new_elem.buffer);

		// copy the new data onto the newly created cache buffer
		memcpy(new_elem.buffer, buffer, length);
		
		// add the new element to the send array
		assert(sendarray);
		if(sendarray)
			ArrayAppend(sendarray, &new_elem);
	}
	else // found element for this socket in sendarray
	{
		net_element *n_elem;
		// append to the current cache buffer for this socket
		n_elem = (net_element *)ArrayNth(sendarray, idx);

		assert(n_elem->socket == s);

		if((n_elem->alloc_blocks * MAXPACKETSIZE) < (length + n_elem->length))
		{
			// the current cache buffer is too small to hold the new packet
			// so we need to expand the memory allocation to hold all the data.
			// this should rarely happen, but it is here in case it is needed
			n_elem->alloc_blocks = ((length + n_elem->length) / MAXPACKETSIZE) + 1;
			n_elem->buffer = (char *)gsirealloc(n_elem->buffer, MAXPACKETSIZE * n_elem->alloc_blocks);
			assert(n_elem->buffer);
		}

		// append the new packet onto the cached buffer
		memcpy(&n_elem->buffer[n_elem->length], buffer, length);
		n_elem->length += length;
	}

	// attempt to send any remaining data in the socket's cache
	ret = SendSocketCache(s, idx);

	return ret;
}

int SendPacketUDP(char *address, unsigned short port, Packet p)
{
	SOCKADDR_IN to;

	to.sin_family = AF_INET;
	to.sin_addr.s_addr = inet_addr(address);
	to.sin_port = htons(port);

#if INSTRUMENT_VOICE_NET_TRAFFIC
		if( NetSpyFunc )
			NetSpyFunc(VNE_SEND, gUDPSocket, PacketBuffer(p), PacketLength(p), 0, NetSpyParam);
#endif //INSTRUMENT_VOICE_NET_TRAFFIC

	return sendto(gUDPSocket, PacketBuffer(p), PacketLength(p), 0, (const struct sockaddr *)&to, sizeof(SOCKADDR_IN));
}

static int ReceiveSocketCache(SOCKET s, int idx, int *err)
{
	int ret;
	net_element key;
	net_element *n_elem;
	int recv_count = 0;

	assert(s != INVALID_SOCKET);
	assert(recvarray);

	*err = 0;

	if(idx == NOT_FOUND && ArrayLength(recvarray))
	{
		memset(&key, 0, sizeof(key));
		key.socket = s;
		idx = ArraySearch(recvarray, &key, NetArrayCompareFn, 0, 0);
	}

	if(idx == NOT_FOUND)
	{
		net_element new_elem;

		new_elem.socket = s;
		new_elem.alloc_blocks = 2;
		new_elem.buffer = (char *)gsimalloc(MAXPACKETSIZE * new_elem.alloc_blocks);
		assert(new_elem.buffer);
		new_elem.length = 0;

		ArrayAppend(recvarray, &new_elem);
		idx = ArraySearch(recvarray, &new_elem, NetArrayCompareFn, 0, 0);
	}

	assert(idx != NOT_FOUND);
	n_elem = (net_element *)ArrayNth(recvarray, idx);

	while((ret = recv(s, &n_elem->buffer[n_elem->length], 
		(MAXPACKETSIZE * n_elem->alloc_blocks) - n_elem->length, 0)) > 0)
	{
#if INSTRUMENT_VOICE_NET_TRAFFIC
		if( NetSpyFunc )
			NetSpyFunc(VNE_RECV, s, &n_elem->buffer[n_elem->length], ret, 0, NetSpyParam);
#endif //INSTRUMENT_VOICE_NET_TRAFFIC

		recv_count += ret;
		if(ret == (MAXPACKETSIZE * n_elem->alloc_blocks) - n_elem->length)
		{
			// we maxed out our receive buffer on this socket
			// so we need to expand it. this shouldn't happen very often
			n_elem->alloc_blocks++;
			n_elem->buffer = (char*)gsirealloc(n_elem->buffer, MAXPACKETSIZE * n_elem->alloc_blocks);
			assert(n_elem->buffer);
		}
		n_elem->length += ret;
	}

	if(ret == SOCKET_ERROR && !recv_count)
		*err = GOAGetLastError(s);
	
	return ret;
}

Packet ReceivePacketTCP(SOCKET s, int *err)
{
	int ret, idx, i;
	net_element key, *n_elem;
	unsigned short packet_len;
	Packet p;

	assert(s != INVALID_SOCKET);

	ret = ReceiveSocketCache(s, NOT_FOUND, err);

	if(ret == 0)
		*err = NETERR_CLOSESOCKET;

	memset(&key, 0, sizeof(key));
	key.socket = s;
	idx = ArraySearch(recvarray, &key, NetArrayCompareFn, 0, 0);

	assert(idx != NOT_FOUND);
	if(idx == NOT_FOUND) 
		return NULL;

	n_elem = (net_element *)ArrayNth(recvarray, idx);

	if(n_elem->length < sizeof(unsigned short))
		return NULL; // not big enough to get a packet length from

	packet_len = PACKETLEN(n_elem->buffer);

	if(n_elem->length < packet_len)
		return NULL; // there is not a complete packet in the cache

	p = g_NetPacket;
	PacketClear(p);
	assert(p);
	if(!p)
		return NULL;

	if(!PacketLoad(p, n_elem->buffer))
	{
		return NULL;
	}

	// remove packet data from cache and move remaining data to the front
	for(i = 0; i < packet_len; i++)
		n_elem->buffer[i] = n_elem->buffer[i+packet_len];
	n_elem->length -= packet_len;
	
	return p;
}

Packet ReceivePacketUDP(SOCKADDR_IN *from)
{
	int fromlen = sizeof(SOCKADDR_IN);
	int result;
	Packet p = NULL;

	memset(recvbuffer, 0, sizeof(recvbuffer));
	result = recvfrom(gUDPSocket, (char *)&recvbuffer, 
		sizeof(recvbuffer), 0, (struct sockaddr *)from, (unsigned int*)&fromlen);

	if(result < (int)sizeof(unsigned short))
		return NULL;

	p = g_NetPacket;
	PacketClear(p);

	assert(p);
	if(!p)
		return NULL;

	if(!PacketLoad(p, recvbuffer))
	{
		return NULL;
	}

	if(result != PacketLength(p))
		return NULL;
	
	return p;
}

void VoiceNetThink(void)
{
	int idx, length;
	net_element *elem;

	length = ArrayLength(sendarray);
	for(idx = 0; idx < length; idx++)
	{
		elem = (net_element *)ArrayNth(sendarray, idx);
		SendSocketCache(elem->socket, idx);
	}
}

void VoiceNetCloseSocket(SOCKET s)
{
	int idx;
	net_element key;
	net_element *elem;

	memset(&key, 0, sizeof(key));
	key.socket = s;	

	if(sendarray)
	{
		idx = ArraySearch(sendarray, &key, NetArrayCompareFn, 0, 0);
		if(idx != NOT_FOUND) // found
		{
			elem = (net_element *)ArrayNth(sendarray, idx);
			if(elem->buffer)
				gsifree(elem->buffer);
			ArrayRemoveAt(sendarray, idx);
		}
	}

	if(recvarray)
	{
		idx = ArraySearch(recvarray, &key, NetArrayCompareFn, 0, 0);
		if(idx != NOT_FOUND) // found
		{
			elem = (net_element *)ArrayNth(recvarray, idx);
			if(elem->buffer)
				gsifree(elem->buffer);
			ArrayRemoveAt(recvarray, idx);
		}
	}

	if(s != INVALID_SOCKET)
	{
		closesocket(s);
		s = INVALID_SOCKET;
	}
}

void VoiceNetClose(void)
{
	int i;
	int length;
	net_element *elem;

	VoiceNetCloseSocket(gUDPSocket);
	VoiceNetCloseSocket(gTCPSocket);

	if(sendarray)
	{
		length = ArrayLength(sendarray);
		for(i = 0; i < length; i++)
		{
			elem = (net_element *)ArrayNth(sendarray, i);

			SendSocketCache(elem->socket, i);
		}
		ArrayFree(sendarray);
		sendarray = NULL;
	}

	if(recvarray)
	{
		ArrayFree(recvarray);
		recvarray = NULL;
	}

	if(g_NetPacket)
	{
		PacketDelete(g_NetPacket);
		g_NetPacket = NULL;
	}
}


#if INSTRUMENT_VOICE_NET_TRAFFIC

void RegisterVoiceNetTrafficSpy(VoiceNetEventSpyFn func, void *instance)
{
	NetSpyFunc = func;
	NetSpyParam = instance;
}

#endif //INSTRUMENT_VOICE_NET_TRAFFIC
