/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
*
*		Filename:	x_sock_GC.c
*		Date:		10/11/2002
*
*		Desc:		<todo>
*
*
*		Revisions History:
*
*
\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/


////////////////////////////////////////////////////////////////////////////
//// INCLUDES
#include "cpc.h"

#include <x_sock.h>
#include <avetcp.h>
#include "Dol_SockImp.h"

//////////////////////////////////////////////////////////////////////////
// DEFINES 

//////////////////////////////////////////////////////////////////////////
// TYPES 

//////////////////////////////////////////////////////////////////////////
// GLOBALS 


X_SOCKET _x_socket (s32 family, s32 type, s32 protocol)
{
	X_SOCKET sockfd;
	AT_SINT16 netHandle;

	if(!g_connectionEstablished) return X_SOCKET_ERROR;
	if(family == AF_INET)
	{
		// find a free socket descriptor
		for(sockfd=0; sockfd < MAX_SOCKET; ++sockfd) {
			if(g_freeSocket[sockfd] == 0) break;
		}
		
		if(sockfd >= MAX_SOCKET) return X_SOCKET_ERROR;	// none available
		g_freeSocket[sockfd] = 1;	// mark descriptor used

		if(type == SOCK_DGRAM)
		{
			g_socketDescriptor[sockfd].type = SOCK_DGRAM;
			g_socketDescriptor[sockfd].socketname.type = AT_IPany;
			g_socketDescriptor[sockfd].socketname.v4addr = 0;
			g_socketDescriptor[sockfd].localPort = 0;
			g_socketDescriptor[sockfd].flags = 0;
			return sockfd;
		} else if (type == SOCK_STREAM) {
			netHandle = tcp_create();
			if(netHandle < 0) {
				// return handle
				g_freeSocket[sockfd] = 0;	// mark descriptor free
				return X_SOCKET_ERROR;
			}
			// set asr function
			set_asr(netHandle, tcp_asr);

			g_socketDescriptor[sockfd].networkHandle = netHandle;
			g_socketDescriptor[sockfd].type = SOCK_STREAM;
			g_socketDescriptor[sockfd].socketname.type = AT_IPany;
			g_socketDescriptor[sockfd].socketname.v4addr = 0;
			g_socketDescriptor[sockfd].localPort = 0;
			g_socketDescriptor[sockfd].flags = 0;
			return sockfd;
		}

	} 

	return X_SOCKET_ERROR;
}




s32 _x_bind ( X_SOCKET s, const struct X_SOCKADDR *addr, s32 addrlen )
{
	NIN_SOCKET_T *socket = &g_socketDescriptor[s];
	AT_SINT16 ret = 0;

	if( ((InsockSockaddrIn_t*) addr)->sin_addr.s_addr == 0) {
		socket->socketname.type = AT_IPany;
	} else {
		socket->socketname.type = AT_IPv4;
		socket->socketname.v4addr =((InsockSockaddrIn_t*) addr)->sin_addr.s_addr;
	}
	socket->localPort = ((InsockSockaddrIn_t*) addr)->sin_port;

	if(socket->type == SOCK_STREAM) {
		ret = tcp_bind(socket->networkHandle, &(socket->socketname/*.v4addr*/), socket->localPort);
	}
	return ret;
}




s32 _x_connect(X_SOCKET sockfd, const struct InsockSockaddr *destaddr, s32 addrlen)
{
	NIN_SOCKET_T *socket = &g_socketDescriptor[sockfd];
	AT_SINT16 ret = 0;
	OSMessage msg;
	u32 msgEvent;

	if(((InsockSockaddrIn_t*)destaddr)->sin_addr.s_addr == 0) {
		socket->peername.type = AT_IPany;
	} else {
		socket->peername.type = AT_IPv4;
		socket->peername.v4addr = ((InsockSockaddrIn_t*)destaddr)->sin_addr.s_addr;
	}
	socket->destPort = ((InsockSockaddrIn_t*)destaddr)->sin_port;

	if(socket->type == SOCK_STREAM) {
		ret = tcp_connect(socket->networkHandle, &socket->peername/*.v4addr*/, socket->destPort, AT_NULL);
		OSReceiveMessage(&ASRMessageQueue, &msg, OS_MESSAGE_BLOCK);
		// should check message
		msgEvent = (u32) msg;
		if((s16) msgEvent != AT_ASR_ESTABLISHED) return -1;
	}

	return ret;
}






s32 _x_sendto (X_SOCKET sockfd, const XCHAR* buff, s32 nbytes, s32 sdwFlags, const struct InsockSockaddr *pToAddr, s32 sdwToAddrLen )
{
	AT_SINT16 ret;
	AT_IP_ADDR peer;
	AT_IP_ADDR *peerPtr;
	u16 peerPort;
	AT_SEND_BUFFS sendBuff[3];

	NIN_SOCKET_T *socketPtr = &g_socketDescriptor[sockfd];

	if((g_socketDescriptor[sockfd].flags & SCK_OPEN_FLAG) == 0)
	{
		// open the udp port
		ret = udp_open(
					&g_socketDescriptor[sockfd].socketname,
					g_socketDescriptor[sockfd].localPort,

					&g_socketDescriptor[sockfd].peername,
					g_socketDescriptor[sockfd].destPort,

					udp_asr,
					udp_receiver);

		if(ret < 0) return ret;

		g_socketDescriptor[sockfd].networkHandle = ret;

		// set open flag
		g_socketDescriptor[sockfd].flags |= SCK_OPEN_FLAG;
	}

	// do the send
	if(pToAddr)
	{
		peer.type = AT_IPv4;
		peer.v4addr = ((InsockSockaddrIn_t*) pToAddr)->sin_addr.s_addr;
		peerPtr = &peer;
		peerPort = ((InsockSockaddrIn_t*) pToAddr)->sin_port;
	} else 
	{
		peerPtr = AT_NULL;
		peerPort = g_socketDescriptor[sockfd].destPort;
	}

	sendBuff[0].buff = (AT_UBYTE*) buff;
	sendBuff[0].len = (AT_SINT16) nbytes;

	ret = udp_send(
		g_socketDescriptor[sockfd].networkHandle,		/* network handle */
		AT_NULL,										/* local IP address*/
		peerPtr,										/* remote IP address */
		peerPort,										/* remote port */
		AT_NULL,										/* IP options */
		1,												/* buffer count */
		sendBuff);										/* buffer pointer */
	return ret;
}


s32 _x_send ( X_SOCKET s, const XCHAR *pBuff, s32 bufferSize, s32 flags)
{
	char *sendBuffer;
	NIN_SOCKET_T *socket = &g_socketDescriptor[s];
	AT_SINT16 ret = 0;

	if(socket->type != SOCK_STREAM) return -1;

	sendBuffer = (char*) x_malloc(bufferSize);
	memcpy(sendBuffer, pBuff, bufferSize);
	ret = tcp_send_f(socket->networkHandle, Tcp_send_callback, bufferSize, sendBuffer);
	socket->flags |= (SCK_WRITE_FLAG | SCK_PENDING_FLAG);

	return 0;
}




s32 _x_recvfrom ( X_SOCKET sockfd, XCHAR *buff, s32 sdwBuffLen, s32 sdwFlags, 
				 struct InsockSockaddr *pFromAddr, X_SOCKLEN *sdwFromAddrlen )
{
	NIN_SOCKET_T *socketPtr = &g_socketDescriptor[sockfd];
	NIN_SOCKET_BUFFER_T *socketBuffer;
	AT_SINT16 ret;
	s32 bufferSize = 0;

	if(socketPtr->type != SOCK_DGRAM) return -1;

	if((socketPtr->flags & SCK_OPEN_FLAG) == 0)
	{
		ret = udp_open(
					&g_socketDescriptor[sockfd].socketname,
					g_socketDescriptor[sockfd].localPort,

					&g_socketDescriptor[sockfd].peername,
					g_socketDescriptor[sockfd].destPort,

					udp_asr,
					udp_receiver);

		if(ret < 0) return ret;

		g_socketDescriptor[sockfd].networkHandle = ret;

		// set open flag
		socketPtr->flags |= SCK_OPEN_FLAG;
	}


	if(socketPtr->bufferUseCount == 0) return EWOULDBLOCK;	// blocking

	// find first buffer with data and return it
	socketBuffer = RemoveSocketBufferFromQueue(&socketPtr->completed);

	if(!socketBuffer) return  X_SOCKET_ERROR;	// no buffer found, error

	memcpy(	buff,
			(void*)socketBuffer->data,
			socketBuffer->bufferSize);


	((InsockSockaddrIn_t*)pFromAddr)->sin_addr.s_addr = 
			socketBuffer->hisAddress.v4addr;
	((InsockSockaddrIn_t*)pFromAddr)->sin_port = 
			socketBuffer->hisPort;

	bufferSize = socketBuffer->bufferSize;

	ReleaseSocketBuffer(socketBuffer);
	
	--socketPtr->bufferUseCount;
	return bufferSize;
}




s32 x_recv ( X_SOCKET s, XCHAR *pBuff, s32 sdwBuffLen, s32 sdwFlags )
{
	NIN_SOCKET_T *socket = &g_socketDescriptor[s];
	AT_SINT16 ret = 0;

	if(socket->type == SOCK_STREAM) {
		if(socket->flags & SCK_PENDING_FLAG) {
			return EWOULDBLOCK;
		}

		if(socket->flags & SCK_COMPLETE_FLAG) {
			ret = sdwBuffLen;
		} else {
			ret = tcp_receive(	socket->networkHandle, Tcp_recv_callback, 
								(AT_SINT16) sdwBuffLen, TCPRecvBuffer);
		}
		
		if(ret < 0) {
			if(ret == AT_API_PENDING) {
				socket->flags |= (SCK_READ_FLAG | SCK_PENDING_FLAG);
				ret = EWOULDBLOCK;
			} else if(ret > 0) {
				// data ready
				// copy to user buffer;
				memcpy(TCPRecvBuffer, pBuff, sdwBuffLen);
				socket->flags &= ((SCK_READ_FLAG | SCK_PENDING_FLAG) ^ SCK_MASK);
			}
		}
	}

	return ret;
}




s32 _x_ioctl ( X_SOCKET s, u32 cmd, u32 *argp )
{ 
	return 0;
}




// **********************************************



s32 _x_close ( X_SOCKET s ) { return ( X_SOCKET_ERROR ); }

s32 _x_shutdown ( X_SOCKET s, s32 sdwFlags ) { return ( X_SOCKET_ERROR ); }

X_SOCKET _x_accept ( X_SOCKET s, struct X_SOCKADDR *addr, X_SOCKLEN *addrlen ) { return ( X_SOCKET_ERROR ); }

s32 _x_listen ( X_SOCKET s, s32 backlog ) { return ( X_SOCKET_ERROR ); }


//s32 _x_send ( X_SOCKET s, const XCHAR *pBuff, s32 sdwBuffLen, s32 sdwFlags ) { return ( X_SOCKET_ERROR ); }

//s32 _x_recv ( X_SOCKET s, XCHAR *pBuff, s32 sdwBuffLen, s32 sdwFlags ) { return ( X_SOCKET_ERROR ); }





u16 _x_htons ( u16 wHost ) { return ( wHost ); }

u32 _x_htonl ( u32 dwHost ){ return ( dwHost ); }

u16 _x_ntohs ( u16 netshort ){ return ( netshort ); }

u32 _x_ntohl ( u32 netlong ) { return ( netlong ); }




static u32 makeInt(const char* start, const char* end) 
{
	int len = end - start + 1;
	u32 position;
	u32 ret;

	for(position=1, ret=0;
		len;
		--len, --end, position = position * 10) 
	{
		if((*end < '0') || (*end > '9')) return 0;
		ret += (*end - '0') * position;
	}
	return ret;
}

u32 _x_inet_addr ( const XCHAR* pszIPAddress ) 
{
	u32 i;
	u32 position;
	u32 address = 0;
	u32 done;

	const char *start, *end;
	start = pszIPAddress;

	for(i=0, done=0, position = 0x1000000;
		((i < 4) && !done);
		++i, position >>= 8, ++pszIPAddress)
	{
		while ((*pszIPAddress != '.') && (*pszIPAddress != 0)) ++pszIPAddress;
		if(*pszIPAddress == 0) done = 1;
		end = pszIPAddress - 1;
		address += makeInt(start, end) * position;
		start = pszIPAddress + 1;
	}
	return address;
}



XCHAR * _x_inet_ntoa ( struct X_INADDR hostAddress ) { return ( NULL ); }




s32 _x_gethostname( XCHAR *name, s32 namelen ) { return ( X_SOCKET_ERROR ); }
struct X_HOSTENT * _x_gethostbyaddr(  const XCHAR *addr, s32 len, s32 type ) { return ( NULL ); }

s32 _x_getsockopt ( X_SOCKET s, s32 level, s32 optname, XCHAR *optval, s32 *optlen ) { return ( X_SOCKET_ERROR ); }

s32 _x_setsockopt ( X_SOCKET s, s32 sdwLevel, s32 sdwOptName, const void *pOptVal, s32 sdwOptLen ) { return ( X_SOCKET_ERROR ); }

s32 _x_select ( s32 nfds, fd_set  *readfds, fd_set  *writefds, fd_set  *exceptfds, struct x_timeval *timeout ){ return ( X_SOCKET_ERROR ); } 
