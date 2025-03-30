/*
GameSpy GT SDK 
Dan "Mr. Pants" Schoenblum
dan@gamespy.com

Copyright 2000 GameSpy Industries, Inc

18002 Skypark Circle
Irvine, California 92614
Tel: 949.798.4200
Fax: 949.798.4299
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gtListener.h"
#include "gtConnection.h"
#include "gtSend.h"
#include "gtCallback.h"
#include "gtAuth.h"
#include "gtRecv.h"

#ifdef SN_SYSTEMS   //PS2 stack that doesn't support setting large backlog values
#define BACKLOG 5
#else
#define BACKLOG 32
#endif

#ifdef _DEBUG
#define GTI_NEGOTIATING_TIMEOUT       (600 * 1000)
#else
#define GTI_NEGOTIATING_TIMEOUT       (30 * 1000)
#endif
#define FirstListener                 (*(GTIListener *)&_FirstListener)

typedef struct
{
	struct GTIListener * pnext;
} GTIFirstListener;
static GTIFirstListener _FirstListener;

GTListener gtiNewListener
(
	GTBool allocBuffer
)
{
	GTListener listener;

	// Create the listener.
	///////////////////////
	listener = (GTListener)gsimalloc(sizeof(GTIListener));
	if(!listener)
		return NULL;

	// Init stuff.
	//////////////
	memset(listener, 0, sizeof(GTIListener));
	listener->socket = INVALID_SOCKET;
	listener->UDPSocket = INVALID_SOCKET;

	// Allocate the UDP buffer.
	/////////////////////////
	if(allocBuffer)
	{
		listener->UDPBuffer = (GTByte *)gsimalloc(GTI_MAX_UDP_SIZE);
		if(!listener->UDPBuffer)
		{
			gsifree(listener);
			return NULL;
		}
	}

	// Add it to the head of the list.
	//////////////////////////////////
	listener->pnext = FirstListener.pnext;
	FirstListener.pnext = listener;

	return listener;
}

GTBool gtiCanFreeListener
(
	GTListener listener
)
{
	// Are we in a callback?
	////////////////////////
	if(listener->callbackLevel)
		return GTFalse;

	// Do we have any active connections?
	/////////////////////////////////////
	if(listener->numConnections)
		return GTFalse;

	return GTTrue;
}

void gtiFreeListener
(
	GTListener listener
)
{
	GTListener pprev;
	GTBool found = GTFalse;

	assert(listener);
	if(!listener)
		return;

	// Find it in the list.
	///////////////////////
	for(pprev = &FirstListener ; pprev->pnext ; pprev = pprev->pnext)
	{
		if(pprev->pnext == listener)
		{
			found = GTTrue;
			break;
		}
	}

	// Make sure we found it.
	/////////////////////////
	if(!found)
		return;

	// Take it out of the list.
	///////////////////////////
	pprev->pnext = listener->pnext;

	// Make sure the sockets are closed.
	//////////////////////////////////
	if(listener->socket != INVALID_SOCKET)
	{
		shutdown(listener->socket, 2);
		closesocket(listener->socket);
	}
	if(listener->UDPSocket != INVALID_SOCKET)
	{
		closesocket(listener->UDPSocket);
	}

	// gsifree the UDP buffer.
	///////////////////////
	gsifree(listener->UDPBuffer);

	// gsifree it.
	///////////
	gsifree(listener);
}

void gtiStopAllListeners
(
	GTBool callCallbacks
)
{
	while(FirstListener.pnext)
	{
		if(callCallbacks)
			gtiStoppedCallback(FirstListener.pnext, GTStopped);
		gtiFreeListener(FirstListener.pnext);
	}
}

void gtiStopListener
(
	GTListener listener
)
{
	// Check for already stopped.
	/////////////////////////////
	if(listener->stopped)
		return;

	// Close the TCP socket.
	////////////////////////
	if(listener->socket != INVALID_SOCKET)
	{
		shutdown(listener->socket, 2);
		closesocket(listener->socket);
		listener->socket = INVALID_SOCKET;
	}

	// Mark it as stopped.
	//////////////////////
	listener->stopped = GTTrue;
}

static void gtiCloseListenerUDPSocket
(
	GTListener listener
)
{
	// Cleanup UDP stuff.
	/////////////////////
	if(listener->UDPSocket != INVALID_SOCKET)
		closesocket(listener->UDPSocket);
	listener->UDPSocket = INVALID_SOCKET;
	gsifree(listener->UDPBuffer);
	listener->UDPBuffer = NULL;

	// If we have connections, close their UDP too.
	///////////////////////////////////////////////
	if(listener->numConnections)
	{
		GTConnection connection;

		for(connection = gtiGetHeadConnection() ; connection ; connection = connection->pnext)
		{
			if(connection->listener == listener)
				gtiCloseConnectionUDPSocket(connection);
		}
	}
}

GTBool gtiStartListener
(
	GTListener listener,
	unsigned int IP,
	unsigned short port
)
{
	SOCKADDR_IN address;
	int rcode;
	int len;

	// Create the socket.
	/////////////////////
	listener->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(listener->socket == INVALID_SOCKET)
		return GTFalse;

	// Bind the socket.
	///////////////////
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = IP;
	address.sin_port = htons(port);
	rcode = bind(listener->socket, (SOCKADDR *)&address, sizeof(SOCKADDR_IN));
	if(rcode == SOCKET_ERROR)
		return GTFalse;

#ifndef BLOCKING_SOCKETS
	// Set it to non-blocking.
	//////////////////////////
	if(!SetSockBlocking(listener->socket, 0))
		return GTFalse;
#endif

	// Start listening.
	///////////////////
	rcode = listen(listener->socket, BACKLOG);
	if(rcode == SOCKET_ERROR)
		return GTFalse;

	// If we already have a UDP socket,
	// from an old listener, use that.
	///////////////////////////////////
	if(listener->UDPSocket == INVALID_SOCKET)
	{
		// If we got a 0 port, found out what port we actually bound to.
		////////////////////////////////////////////////////////////////
		if(!port)
		{
			len = sizeof(SOCKADDR_IN);
			rcode = getsockname(listener->socket, (SOCKADDR *)&address, (unsigned int*)&len);
			if(rcode == SOCKET_ERROR)
				return GTFalse;
		}

		// Create the UDP socket.
		/////////////////////////
		listener->UDPSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(listener->UDPSocket != INVALID_SOCKET)
		{
			// Bind it.
			///////////
			rcode = bind(listener->UDPSocket, (SOCKADDR *)&address, sizeof(SOCKADDR_IN));
			if(rcode == SOCKET_ERROR)
			{
				// Try to bind to any port.
				///////////////////////////
				address.sin_port = 0;
				rcode = bind(listener->UDPSocket, (SOCKADDR *)&address, sizeof(SOCKADDR_IN));
				if(rcode == SOCKET_ERROR)
					gtiCloseListenerUDPSocket(listener);
			}

#ifndef BLOCKING_SOCKETS
			// Set non-blocking.
			////////////////////
			if(rcode != SOCKET_ERROR)
			{
				if(!SetSockBlocking(listener->UDPSocket, 0))
					gtiCloseListenerUDPSocket(listener);
			}
#endif

			// Get what port we bound to.
			/////////////////////////////
			listener->localUDPPort = ntohs(address.sin_port);
			if(!listener->localUDPPort)
			{
				len = sizeof(SOCKADDR_IN);
				rcode = getsockname(listener->UDPSocket, (SOCKADDR *)&address, (unsigned int*)&len);
				if(rcode == SOCKET_ERROR)
					gtiCloseListenerUDPSocket(listener);
				else
					listener->localUDPPort = ntohs(address.sin_port);
			}
		}
	}

	return GTTrue;
}

static void gtiListenerThink
(
	GTListener listener
)
{
	SOCKET socket;
	int rcode;
	GTConnection connection;
	SOCKADDR_IN address;
	int addressLen;

	// Is it time to gsifree this listener?
	////////////////////////////////////
	if(listener->stopped && gtiCanFreeListener(listener))
	{
		gtiFreeListener(listener);
		return;
	}

	// Receive UDP for our connections.
	///////////////////////////////////
	if(listener->UDPSocket != INVALID_SOCKET)
	{
		GTIRecvResult result;
		GTIControlCode code;
		int len;

		// Get a message.
		/////////////////
		while((result = gtiReceiveUDPMessage(listener->UDPSocket, listener->UDPBuffer, &len, &code, &address)) == GTIRecvMessage)
		{
			// Find out which of our client's this is from.
			///////////////////////////////////////////////
			for(connection = gtiGetHeadConnection() ; connection ; connection = connection->pnext)
			{
				// Check for a match.
				/////////////////////
				if((connection->listener == listener) &&
					(connection->remoteIP == address.sin_addr.s_addr) &&
					(connection->remoteUDPPort == ntohs(address.sin_port)) && 
					(connection->state != GTIClosed))
				{
					// Handle it.
					/////////////
					gtiHandleReceivedUDP(connection, len, code);
					break;
				}
			}
		}

		// Error check.
		///////////////
		if(result == GTIRecvError)
			gtiCloseListenerUDPSocket(listener);
	}

	// If we're stopped, don't accept anymore.
	//////////////////////////////////////////
	if(listener->stopped)
		return;

	assert(listener->socket != INVALID_SOCKET);
	if(listener->socket == INVALID_SOCKET)
		return;

	// Accept connections.
	//////////////////////
	do
	{
#ifdef BLOCKING_SOCKETS
		// If there's no connection waiting, just return now.
		/////////////////////////////////////////////////////
		if(!gtiCanRecvOnSocket(listener->socket))
			return;
#endif

		// Setup the accept parameters.
		///////////////////////////////
		addressLen = sizeof(SOCKADDR_IN);
		memset(&address, 0, addressLen);
		address.sin_family = AF_INET;

		// Do the accept.
		/////////////////
		socket = accept(listener->socket, (SOCKADDR *)&address, (unsigned int*)&addressLen);

		// Check for error.
		///////////////////
		if(socket != SOCKET_ERROR)
		{
			// Check for a bad address.
			// This is because Wingate seems to cause accept() to return 0
			// instead of -1 when nothing's waiting.
			// We want to return right away, since we know nothings waiting.
			////////////////////////////////////////////////////////////////
			if(!address.sin_addr.s_addr || !address.sin_port)
				return;

			// Create a connection object.
			//////////////////////////////
			connection = gtiNewConnection(NULL);

			if(connection)
			{
				char challenge[GTI_CHALLENGE_LEN];
				int len;

				// Set some member vars.
				////////////////////////
				connection->listener = listener;
				connection->state = GTINegotiating;
				connection->initiated = GTFalse;
				connection->TCPSocket = socket;
				connection->timeout = GTI_NEGOTIATING_TIMEOUT;
				connection->userData = listener->userData;
				connection->remoteIP = address.sin_addr.s_addr;
				connection->remotePort = ntohs(address.sin_port);
				connection->UDPSocket = listener->UDPSocket;
				connection->UDPBuffer = listener->UDPBuffer;

				// Disable the Nagle algorithm.
				///////////////////////////////
				DisableNagle(connection->TCPSocket);
				SetSockBlocking(connection->TCPSocket, 0);

				// Generate a challenge.
				////////////////////////
				gtiGetChallenge(challenge);

				// Send the challenge.
				//////////////////////
				len = (11 + GTI_CHALLENGE_LEN + 8);
				gtiSendTCPHeader(connection, len, GTINegotiation);
				gtiSendTCPData(connection, "\\challenge\\", 11);
				gtiSendTCPData(connection, challenge, GTI_CHALLENGE_LEN);
				gtiSendTCPData(connection, "\\final\\", 8);

				// Store the expected response.
				///////////////////////////////
				gtiGetResponse(connection->response, challenge);

				// One more connection for the listener.
				////////////////////////////////////////
				listener->numConnections++;
			}
			else
			{
				shutdown(socket, 2);
				closesocket(socket);
			}
		}
	}
	while(socket != SOCKET_ERROR);

	// Was there an error?
	//////////////////////
	rcode = GOAGetLastError(listener->socket);
	if((rcode != WSAEWOULDBLOCK) && (rcode != WSAEINPROGRESS))
	{
		// There was an error - stop the listener.
		//////////////////////////////////////////
		gtiStoppedCallback(listener, GTListenerError);
		if(!listener->stopped)
			gtiStopListener(listener);
	}
}

void gtiListenersThink
(
	void
)
{
	GTListener listener;
	GTListener check;

	// Loop through the listeners.
	//////////////////////////////
	for(listener = &FirstListener ; listener->pnext ; )
	{
		// We only advance the pointer if the one
		// we're thinking for hasn't been deleted.
		//////////////////////////////////////////
		check = listener->pnext;
		gtiListenerThink(check);
		if(check == listener->pnext)
			listener = listener->pnext;
	}
}

GTBool gtiIsValidListener
(
	GTListener listener
)
{
	GTListener pcurr;
 
	for(pcurr = &FirstListener ; pcurr->pnext ; pcurr = pcurr->pnext)
		if(pcurr->pnext == listener)
			return GTTrue;

	return GTFalse;
}

GTListener gtiFindListenerByUDPPort
(
	unsigned short port
)
{
	GTListener listener;

	for(listener = FirstListener.pnext ; listener ; listener = listener->pnext)
		if(listener->localUDPPort == port)
			return listener;

	return NULL;
}

#ifdef _DEBUG
#include <stdio.h>
// Get debugging info on listeners.
///////////////////////////////////
void gtiListenersDebugInfo
(
	void (* callback)(const char * output)
)
{
	GTListener listener;
	char buffer[128];
	int index = 0;

	callback("Listeners:");
	callback("index  port");
	callback("-----  -----");
	for(listener = FirstListener.pnext ; listener ; listener = listener->pnext)
	{
		sprintf(buffer, "%5d  %5d", index++, gtGetListenerPort(listener));
		callback(buffer);
	}
	callback("End of Listeners");
}
#endif