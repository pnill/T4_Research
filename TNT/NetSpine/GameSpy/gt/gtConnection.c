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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gtConnection.h"
#include "gtCallback.h"
#include "gtSend.h"
#include "gtRecv.h"
#include "gtAuth.h"

#define GTI_INCOMING_BUFFER_SIZE    (GTI_MAX_MESSAGE_SIZE + GTI_MAX_HEADER_SIZE)
#define GTI_UDP_PING_TIME           (5 * 1000)   // 5 seconds
#define GTI_UDP_PING_ATTEMPTS       24           // 2 minutes total
#define GTI_MAX_FLUSH_TIME          (120 * 1000)
#define FirstConnection             (*(GTIConnection *)&_FirstConnection)

typedef enum
{
	GTIConnectWaiting,
	GTIConnectSuccess,
	GTIConnectFailure
} GTIConnectState;

typedef struct
{
	struct GTIConnection * pnext;
} GTIFirstConnection;
static GTIFirstConnection _FirstConnection;

GTConnection gtiNewConnection
(
	GTConnectResult * result
)
{
	GTConnection connection;

	// Create the connection.
	/////////////////////////
	connection = (GTConnection)gsimalloc(sizeof(GTIConnection));
	if(!connection)
	{
		if(result)
			*result = GTMemoryError;
		return NULL;
	}

	// Init stuff.
	//////////////
	memset(connection, 0, sizeof(GTIConnection));
	connection->TCPSocket = INVALID_SOCKET;
	connection->UDPSocket = INVALID_SOCKET;
	connection->startConnectTime = current_time();
	connection->incomingBuffer.size = GTI_INCOMING_BUFFER_SIZE;
	connection->incomingBuffer.buffer = (GTByte *)gsimalloc(connection->incomingBuffer.size);
	if(!connection->incomingBuffer.buffer)
	{
		if(result)
			*result = GTMemoryError;
		gsifree(connection);
		return NULL;
	}

	// Add it to the head of the list.
	//////////////////////////////////
	connection->pnext = FirstConnection.pnext;
	FirstConnection.pnext = connection;

	return connection;
}

GTBool gtiFreeConnection
(
	GTConnection connection
)
{
	GTConnection pprev;
	GTBool found;

	assert(connection);
	if(!connection)
		return GTFalse;

	// Find it in the list.
	///////////////////////
	found = GTFalse;
	for(pprev = &FirstConnection ; pprev->pnext ; pprev = pprev->pnext)
	{
		if(pprev->pnext == connection)
		{
			found = GTTrue;
			break;
		}
	}

	// Make sure we found it.
	/////////////////////////
	if(!found)
		return GTFalse;

	// If we have a listener, one less connection.
	//////////////////////////////////////////////
	if(connection->listener)
		connection->listener->numConnections--;

	// Take it out of the list.
	///////////////////////////
	pprev->pnext = connection->pnext;

	// Make sure the sockets are closed.
	////////////////////////////////////
	if(connection->TCPSocket != INVALID_SOCKET)
	{
		shutdown(connection->TCPSocket, 2);
		closesocket(connection->TCPSocket);
	}
	if(!connection->listener && (connection->UDPSocket != INVALID_SOCKET))
	{
		shutdown(connection->UDPSocket, 2);
		closesocket(connection->UDPSocket);
	}

	// gsifree the buffers.
	////////////////////
	gsifree(connection->outgoingBuffer.buffer);
	gsifree(connection->incomingBuffer.buffer);
	if(!connection->listener)
		gsifree(connection->UDPBuffer);

	// gsifree the iniial data.
	////////////////////////
	gsifree(connection->initialData);

	// gsifree the send filters list.
	//////////////////////////////
	if(connection->sendFilters)
		ArrayFree(connection->sendFilters);

	// gsifree the connection.
	///////////////////////
	gsifree(connection);

	return GTFalse;
}

static GTBool gtiCanFreeConnection
(
	GTConnection connection
)
{
	// Are we in a callback?
	////////////////////////
	if(connection->callbackLevel)
		return GTFalse;

	// Do we need to wait for an accept/reject?
	///////////////////////////////////////////
	if(connection->freeAtAcceptReject)
		return GTFalse;

	// Is there data buffered?
	//////////////////////////
	if(connection->outgoingBuffer.len)
	{
		// Check if we're within the max flush time.
		////////////////////////////////////////////
		if((current_time() - connection->startCloseTime) < GTI_MAX_FLUSH_TIME)
			return GTFalse;
	}

	return GTTrue;
}

GTBool gtiCloseConnection
(
	GTConnection connection
)
{
	// If we're waiting for an accept/reject we can't gsifree it
	// right away.  There's no closed callback yet, so the app
	// won't know it was freed, and when the accept/reject
	// happens, things could get ugly.
	//////////////////////////////////////////////////////////
	if(connection->state == GTIAwaitingAcceptReject)
		connection->freeAtAcceptReject = GTTrue;

	// Just mark it as closed.
	// It will be freed later.
	//////////////////////////
	connection->state = GTIClosed;

	// Remember the time.
	/////////////////////
	connection->startCloseTime = current_time();

	return GTFalse;
}

void gtiCloseAllConnections
(
	GTBool callCallbacks
)
{
	while(FirstConnection.pnext)
	{
		if(callCallbacks)
			gtiClosedCallback(FirstConnection.pnext, GTLocalClose);
		gtiFreeConnection(FirstConnection.pnext);
	}
}

GTBool gtiStartConnecting
(
	GTConnection connection,
	const char * localAddress,
	const char * remoteAddress,
	const GTByte * message,
	int len,
	int timeout,
	GTBool blocking,
	GTConnectionCallbacks * callbacks,
	GTConnectResult * result
)
{
	unsigned int IP;
	unsigned short port;
	SOCKADDR_IN address;
	int rcode;

	// Get the local IP & port.
	///////////////////////////
	if(!gtStringToAddress(localAddress, &IP, &port))
	{
		if(result)
			*result = GTLocalAddressError;
		return GTFalse;
	}

	// Set data.
	////////////
	connection->initiated = GTTrue;
	connection->timeout = timeout;
	connection->blockingConnect = blocking;
	if(strlen(remoteAddress) >= GTI_MAX_ADDRESS)
	{
		if(result)
			*result = GTRemoteAddressError;
		return GTFalse;
	}
	strcpy(connection->remoteAddress, remoteAddress);
	if(callbacks)
		connection->callbacks = *callbacks;

	// Copy initial data.
	/////////////////////
	if(!message || !len)
	{
		connection->initialData = NULL;
		connection->initialDataLen = 0;
	}
	else
	{
		if(len == -1)
			len = (strlen(message) + 1);
		connection->initialData = (GTByte *)gsimalloc(len);
		if(!connection->initialData)
		{
			if(result)
				*result = GTMemoryError;
			return GTFalse;
		}
		memcpy(connection->initialData, message, len);
		connection->initialDataLen = len;
	}

	// Allocate the UDP buffer.
	///////////////////////////
	connection->UDPBuffer = (GTByte *)gsimalloc(GTI_MAX_UDP_SIZE);
	if(!connection->UDPBuffer)
	{
		if(result)
			*result = GTMemoryError;
		return GTFalse;
	}

	// Create the TCP socket.
	/////////////////////////
	connection->TCPSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(connection->TCPSocket == INVALID_SOCKET)
	{
		if(result)
			*result = GTLocalNetworkError;
		return GTFalse;
	}

#ifndef BLOCKING_SOCKETS
	// Set non-blocking.
	////////////////////
	if(!SetSockBlocking(connection->TCPSocket, 0))
	{
		if(result)
			*result = GTLocalNetworkError;
		return GTFalse;
	}
#endif

	// Disable the Nagle algorithm.
	///////////////////////////////
	DisableNagle(connection->TCPSocket);

	// Bind the socket.
	///////////////////
	memset(&address, 0, sizeof(SOCKADDR_IN));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = IP;
	address.sin_port = htons(port);
	rcode = bind(connection->TCPSocket, (SOCKADDR *)&address, sizeof(SOCKADDR_IN));
	if(rcode == SOCKET_ERROR)
	{
		if(result)
			*result = GTLocalNetworkError;
		return GTFalse;
	}

	// Create the UDP socket.
	/////////////////////////
	connection->UDPSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(connection->UDPSocket != INVALID_SOCKET)
	{
#ifndef BLOCKING_SOCKETS
		// Set it to non-blocking.
		//////////////////////////
		if(!SetSockBlocking(connection->UDPSocket, 0))
			gtiCloseConnectionUDPSocket(connection);
#endif

		// If no port was specified, get what we bound to.
		//////////////////////////////////////////////////
		if(!port)
		{
			int len = sizeof(SOCKADDR_IN);
			rcode = getsockname(connection->TCPSocket, (SOCKADDR *)&address, (unsigned int*)&len);
			if(rcode == SOCKET_ERROR)
				gtiCloseConnectionUDPSocket(connection);
		}

		if(connection->UDPSocket != INVALID_SOCKET)
		{
			// Bind the UDP socket.
			///////////////////////
			if(!gtiBindUDPSocket(connection, &address))
			{
				// Try any port.
				////////////////
				address.sin_port = 0;
				if(!gtiBindUDPSocket(connection, &address))
					gtiCloseConnectionUDPSocket(connection);
			}
		}
	}

	return GTTrue;
}

// Calls the callback and frees the connection.
// This _always_ returns GTFalse to signal the connection closed.
/////////////////////////////////////////////////////////////////
static GTBool gtiConnectFailed
(
	GTConnection connection,
	GTConnectResult result
)
{
	gtiCloseConnection(connection);
	gtiConnectedCallback(connection, result, NULL, 0);

	return GTFalse;
}

// Same as above, but also takes a reason.
//////////////////////////////////////////
static GTBool gtiConnectRejected
(
	GTConnection connection,
	GTByte * message,
	int len
)
{
	gtiCloseConnection(connection);
	gtiConnectedCallback(connection, GTRejected, message, len);

	return GTFalse;
}

GTBool gtiConnectionClosed
(
	GTConnection connection,
	GTCloseReason reason
)
{
	gtiCloseConnection(connection);
	gtiClosedCallback(connection, reason);

	return GTFalse;
}

static GTBool gtiConnectTCPSocket
(
	GTConnection connection
)
{
	SOCKADDR_IN address;
	int rcode;

	assert(connection->TCPSocket != INVALID_SOCKET);

	// Setup the remote address.
	////////////////////////////
	memset(&address, 0, sizeof(SOCKADDR_IN));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = connection->remoteIP;
	address.sin_port = htons(connection->remotePort);

	// Start the TCP connect.
	/////////////////////////
	rcode = connect(connection->TCPSocket, (SOCKADDR *)&address, sizeof(SOCKADDR_IN));
	if(rcode == SOCKET_ERROR)
	{
		rcode = GOAGetLastError(connection->TCPSocket);
		if((rcode != WSAEWOULDBLOCK) && (rcode != WSAEINPROGRESS))
			return gtiConnectFailed(connection, GTLocalNetworkError);
	}

	// One more connect().
	//////////////////////
	connection->connectAttempts++;

	// Set the state to connecting.
	///////////////////////////////
	connection->state = GTIConnecting;

	return GTTrue;
}

GTBool gtiBindUDPSocket
(
	GTConnection connection,
	SOCKADDR_IN * address
)
{
	int rcode;

	assert(connection);

	// Check if there's no UDP socket.
	//////////////////////////////////
	if(connection->UDPSocket == INVALID_SOCKET)
		return GTTrue;

	// Do the bind.
	///////////////
	rcode = bind(connection->UDPSocket, (SOCKADDR *)address, sizeof(SOCKADDR_IN));

	return (rcode != SOCKET_ERROR);
}

void gtiCloseConnectionUDPSocket
(
	GTConnection connection
)
{
	if(!connection->listener && (connection->UDPSocket != INVALID_SOCKET))
		closesocket(connection->UDPSocket);
	connection->UDPSocket = INVALID_SOCKET;
	connection->canSendUDP = GTFalse;
	connection->remoteUDPPort = 0;
}

static GTBool gtiDoHostLookup
(
	GTConnection connection
)
{
	assert(connection->initiated);

	// Resolve the address.
	///////////////////////
	if(!gtStringToAddress(connection->remoteAddress, &connection->remoteIP, &connection->remotePort) ||
		!connection->remoteIP || !connection->remotePort)
		return gtiConnectFailed(connection, GTRemoteAddressError);

	// Do the actual connect.
	/////////////////////////
	return gtiConnectTCPSocket(connection);
}

static GTIConnectState gtiCheckConnect
(
	GTConnection connection
)
{
	fd_set fdWrite;
	fd_set fdExcept;
	struct timeval timeout;
	int rcode;

	// Setup the fd stuff.
	//////////////////////
	FD_ZERO(&fdWrite);
	FD_SET(connection->TCPSocket, &fdWrite);
	FD_ZERO(&fdExcept);
	FD_SET(connection->TCPSocket, &fdExcept);

	// Setup the timeout.
	/////////////////////
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	// Do the select.
	/////////////////
	rcode = select(FD_SETSIZE, NULL, &fdWrite, &fdExcept, &timeout);
	if(rcode == SOCKET_ERROR)
		return GTIConnectFailure;

	// Check if no result yet.
	//////////////////////////
	if(rcode == 0)
		return GTIConnectWaiting;

	// Check for success.
	/////////////////////
	if(FD_ISSET(connection->TCPSocket, &fdWrite))
		return GTIConnectSuccess;

	// Connect failed.
	//////////////////
	return GTIConnectFailure;
}

// Returns GTFalse if the connection timed-out.
// If bCheckOneAttempt is GTTrue, then this will
// return GTTrue if timeout is 0 and this is
// the first connect attempt for this connection.
/////////////////////////////////////////////////
static GTBool gtiCheckTimeout
(
	GTConnection connection,
	GTBool bCheckOneAttempt
)
{
	int timeout = connection->timeout;

	// Check for no timeout.
	////////////////////////
	if(timeout < 0)
		return GTTrue;

	// Check if we want to try once without timeout.
	////////////////////////////////////////////////
	if(bCheckOneAttempt && (timeout == 0) && (connection->connectAttempts == 1))
		return GTTrue;

	// Check the timeout.
	/////////////////////
	if((current_time() - connection->startConnectTime) > (unsigned long)connection->timeout)
		return GTFalse;

	return GTTrue;
}

static GTBool gtiDoConnecting
(
	GTConnection connection
)
{
	GTIConnectState connectState;

	assert(connection->initiated);

	// Check the connect.
	/////////////////////
	connectState = gtiCheckConnect(connection);

	// No response?
	///////////////
	if(connectState == GTIConnectWaiting)
	{
		// Timed-out?
		/////////////
		if(!gtiCheckTimeout(connection, GTTrue))
			return gtiConnectFailed(connection, GTTimedOut);

		// Wait.
		////////
		return GTTrue;
	}
	// Connection refused?
	//////////////////////
	else if(connectState == GTIConnectFailure)
	{
		// Timed-out?
		/////////////
		if(!gtiCheckTimeout(connection, GTFalse))
			return gtiConnectFailed(connection, GTTimedOut);

		// Do a new connect.
		////////////////////
		if(!gtiConnectTCPSocket(connection))
			return gtiConnectFailed(connection, GTLocalNetworkError);

		// Wait.
		////////
		return GTTrue;
	}

	// Connect succeeded, we're now negotiating.
	////////////////////////////////////////////
	connection->state = GTINegotiating;

	return GTTrue;
}

static char * gtiFindString
(
	const GTByte * buffer,
	int len,
	const char * string
)
{
	int nStrLen = strlen(string);
	len -= nStrLen;
	while(len-- >= 0)
	{
		if(memcmp(buffer, string, nStrLen) == 0)
			return (char *)buffer;
		buffer++;
	}

	return NULL;
}

static GTBool gtiReceiveServerChallenge
(
	GTConnection connection,
	GTByte * message,
	int len,
	void * userData
)
{
	char * final;
	char * strChallenge;
	int nMessageLen;
	char response[GTI_RESPONSE_LEN];
	char challenge[GTI_CHALLENGE_LEN];
	char strPort[6];
	int nPortLen;
	SOCKADDR_IN address;
	int rcode;
	unsigned short port;

	// Find the final string.
	/////////////////////////
	final = gtiFindString(message, len, "\\final\\");
	if(!final)
		return gtiConnectFailed(connection, GTNegotiationError);
	final[0] = '\0';

	// Find the challenge string.
	/////////////////////////////
	strChallenge = gtiFindString(message, len, "\\challenge\\");
	if(!strChallenge)
		return gtiConnectFailed(connection, GTNegotiationError);
	strChallenge += 11;
	if(strlen(strChallenge) < 4)
		return gtiConnectFailed(connection, GTNegotiationError);

	// Get the response.
	////////////////////
	if(!gtiGetResponse(response, strChallenge))
		return gtiConnectFailed(connection, GTNegotiationError);

	// Generate a challenge and response for the server.
	////////////////////////////////////////////////////
	gtiGetChallenge(challenge);
	gtiGetResponse(connection->response, challenge);

	// Convert the UDP port to a string.
	////////////////////////////////////
	if(connection->UDPSocket != INVALID_SOCKET)
	{
		rcode = sizeof(SOCKADDR_IN);
		rcode = getsockname(connection->UDPSocket, (SOCKADDR *)&address, (unsigned int*)&rcode);
		if(rcode == SOCKET_ERROR)
		{
			gtiCloseConnectionUDPSocket(connection);
			port = 0;
		}
		else
		{
			port = ntohs(address.sin_port);
		}
	}
	else
	{
		port = 0;
	}

	// The UDP port.
	////////////////
	sprintf(strPort, "%d", port);
	nPortLen = strlen(strPort);

	// Send the message.
	////////////////////
	nMessageLen = (10 + GTI_RESPONSE_LEN + 11 + GTI_CHALLENGE_LEN + 6 + nPortLen + 6 + connection->initialDataLen);
	gtiSendTCPHeader(connection, nMessageLen, GTINegotiation);
	gtiSendTCPData(connection, "\\response\\", 10);
	gtiSendTCPData(connection, response, GTI_RESPONSE_LEN);
	gtiSendTCPData(connection, "\\challenge\\", 11);
	gtiSendTCPData(connection, challenge, GTI_CHALLENGE_LEN);
	gtiSendTCPData(connection, "\\port\\", 6);
	gtiSendTCPData(connection, strPort, nPortLen);
	gtiSendTCPData(connection, "\\data\\", 6);
	gtiSendTCPData(connection, connection->initialData, connection->initialDataLen);

	// gsifree the iniial data.
	////////////////////////
	gsifree(connection->initialData);
	connection->initialData = NULL;

	// We're waiting for the challenge now.
	///////////////////////////////////////
	connection->state = GTIReceivedChallenge;

	return GTTrue;
}

static GTBool gtiReceiveClientResponse
(
	GTConnection connection,
	GTByte * message,
	int len,
	void * userData
)
{
	char * data;
	int dataLen;
	char * response;
	char * challenge;

	// Find the data.
	/////////////////
	data = gtiFindString(message, len, "\\data\\");
	if(data)
	{
		data[0] = '\0';
		data += 6;
		dataLen = (len - (data - message));
		if(!dataLen)
			data = NULL;
	}
	else
	{
		dataLen = 0;
	}

	// Find the response.
	/////////////////////
	response = gtiFindString(message, len, "\\response\\");
	if(!response)
		return gtiCloseConnection(connection);
	response += 10;
	if(strlen(response) < GTI_RESPONSE_LEN)
		return gtiCloseConnection(connection);

	// Check the response.
	//////////////////////
	if(!gtiCheckResponse(response, connection->response))
		return gtiCloseConnection(connection);

	// Find the challenge string.
	/////////////////////////////
	challenge = gtiFindString(message, len, "\\challenge\\");
	if(!challenge)
		return gtiCloseConnection(connection);
	challenge += 11;
	if(strlen(challenge) < GTI_CHALLENGE_LEN)
		return gtiCloseConnection(connection);

	// Generate a response to the client's challenge.
	/////////////////////////////////////////////////
	if(!gtiGetResponse(connection->response, challenge))
		return gtiCloseConnection(connection);

	// Check if we have a UDP socket.
	/////////////////////////////////
	if(connection->UDPSocket != INVALID_SOCKET)
	{
		char * strPort;

		// Find the port.
		/////////////////
		strPort = gtiFindString(message, len, "\\port\\");
		if(!strPort)
			gtiCloseConnectionUDPSocket(connection);
		else
		{
			// Move up to the port itself.
			//////////////////////////////
			strPort += 6;

			// Check the port.
			//////////////////
			connection->remoteUDPPort = (unsigned short)atoi(strPort);
			if(!connection->remoteUDPPort)
				gtiCloseConnectionUDPSocket(connection);
		}
	}

	// New state.
	/////////////
	connection->state = GTIAwaitingAcceptReject;

	// Call the connect-attempt callback.
	/////////////////////////////////////
	return gtiConnectAttemptCallback(connection->listener,
		connection,
		connection->remoteIP,
		ntohs(connection->remotePort),
		data,
		dataLen);
}

static GTBool gtiDoNegotiating
(
	GTConnection connection
)
{
	GTIRecvResult recvResult;
	GTBool handlerResult;
	int len;
	GTIControlCode code;

	if(connection->initiated)
	{
		// Receive the challenge.
		/////////////////////////
		recvResult = gtiRecvTCPMessageHeader(connection, &len, &code);
		if(recvResult == GTIRecvError)
			return gtiConnectFailed(connection, GTLocalNetworkError);
		if(recvResult == GTIRecvClosed)
			return gtiConnectFailed(connection, GTNegotiationError);
		if(recvResult == GTIRecvNone)
		{
			// Check for timeout.
			/////////////////////
			if(!gtiCheckTimeout(connection, GTTrue))
				return gtiConnectFailed(connection, GTTimedOut);

			// Wait.
			////////
			return GTTrue;
		}

		// Check the code.
		//////////////////
		if(code != GTINegotiation)
			return gtiConnectFailed(connection, GTNegotiationError);

		// Handle the actual message.
		/////////////////////////////
		if(!gtiRecvTCPMessage(connection, gtiReceiveServerChallenge, &handlerResult, NULL))
			return gtiConnectFailed(connection, GTMemoryError);

		return handlerResult;
	}

	// Receive the response/challenge/initial data.
	///////////////////////////////////////////////
	recvResult = gtiRecvTCPMessageHeader(connection, &len, &code);
	if((recvResult == GTIRecvError) || (recvResult == GTIRecvClosed))
		return gtiCloseConnection(connection);
	if(recvResult == GTIRecvNone)
	{
		// Check for timeout.
		/////////////////////
		if(!gtiCheckTimeout(connection, GTFalse))
			return gtiCloseConnection(connection);

		// Wait.
		////////
		return GTTrue;
	}

	// Check the code.
	//////////////////
	if(code != GTINegotiation)
		return gtiCloseConnection(connection);

	// Handle the actual message.
	/////////////////////////////
	if(!gtiRecvTCPMessage(connection, gtiReceiveClientResponse, &handlerResult, NULL))
		return gtiCloseConnection(connection);

	return handlerResult;
}

static GTBool gtiReceiveServerResponse
(
	GTConnection connection,
	GTByte * message,
	int len,
	void * userData
)
{
	char * response;
	char * accept;
	char * reason;

	// First check for a reason.
	////////////////////////////
	reason = gtiFindString(message, len, "\\reason\\");
	if(reason)
	{
		reason[0] = '\0';
		reason += 8;
	}

	// Check for accept.
	////////////////////
	accept = gtiFindString(message, len, "\\accept\\");
	if(!accept)
		return gtiConnectFailed(connection, GTNegotiationError);
	accept += 8;
	if(!atoi(accept))
	{
		int reasonLen;

		// Figure out the reason length.
		////////////////////////////////
		if(reason)
		{
			reasonLen = (len - (reason - message));
			if(!reasonLen)
				reason = NULL;
		}
		else
		{
			reasonLen = 0;
		}

		// It was rejected.
		///////////////////
		return gtiConnectRejected(connection, reason, reasonLen);
	}

	// Check if we have a UDP socket.
	/////////////////////////////////
	if(connection->UDPSocket != INVALID_SOCKET)
	{
		char * strPort;

		// Find the port.
		/////////////////
		strPort = gtiFindString(message, len, "\\port\\");
		if(!strPort)
			gtiCloseConnectionUDPSocket(connection);
		else
		{
			// Move up to the port itself.
			//////////////////////////////
			strPort += 6;

			// Check the port.
			//////////////////
			connection->remoteUDPPort = (unsigned short)atoi(strPort);
			if(!connection->remoteUDPPort)
				gtiCloseConnectionUDPSocket(connection);
		}
	}

	// Find the response.
	/////////////////////
	response = gtiFindString(message, len, "\\response\\");
	if(!response)
		return gtiConnectFailed(connection, GTNegotiationError);
	response += 10;
	if(strlen(response) < GTI_RESPONSE_LEN)
		return gtiConnectFailed(connection, GTNegotiationError);

	// Check the response.
	//////////////////////
	if(!gtiCheckResponse(response, connection->response) != 0)
		return gtiConnectFailed(connection, GTNegotiationError);

	// Set the state.
	/////////////////
	connection->state = GTIConnected;

	// Call the callback.
	/////////////////////
	return gtiConnectedCallback(connection, GTAccepted, NULL, 0);
}

static GTBool gtiDoReceivedChallenge
(
	GTConnection connection
)
{
	GTIRecvResult recvResult;
	GTBool handlerResult;
	int len;
	GTIControlCode code;

	// Receive the response.
	////////////////////////
	recvResult = gtiRecvTCPMessageHeader(connection, &len, &code);
	if(recvResult == GTIRecvError)
		return gtiConnectFailed(connection, GTLocalNetworkError);
	if(recvResult == GTIRecvClosed)
		return gtiConnectFailed(connection, GTNegotiationError);
	if(recvResult == GTIRecvNone)
	{
		// Check for timeout.
		/////////////////////
		if(!gtiCheckTimeout(connection, GTTrue))
			return gtiConnectFailed(connection, GTTimedOut);

		// Wait.
		////////
		return GTTrue;
	}

	// Check the code.
	//////////////////
	if(code != GTINegotiation)
		return gtiConnectFailed(connection, GTNegotiationError);

	// Handle the actual message.
	/////////////////////////////
	if(!gtiRecvTCPMessage(connection, gtiReceiveServerResponse, &handlerResult, NULL))
		return gtiConnectFailed(connection, GTMemoryError);

	return handlerResult; 
}

static GTBool gtiReceiveTCPMessageHandler
(
	GTConnection connection,
	GTByte * message,
	int len,
	void * userData
)
{
	GTBool reliable = *(GTBool *)userData;

	// Check if we have any receive filters.
	////////////////////////////////////////
	if(connection->receiveFilters)
	{
		// Filter the receive.
		//////////////////////
		return gtiReceiveFilterCallback(connection, 0, message, len, reliable);
	}

	// Call the callback.
	/////////////////////
	return gtiReceivedCallback(connection, message, len, reliable);
}

static GTBool gtiDoReceiveTCPMessages
(
	GTConnection connection
)
{
	GTIRecvResult recvResult;
	GTBool handlerResult;
	int len;
	GTIControlCode code;

	// Receive the response.
	////////////////////////
	while((recvResult = gtiRecvTCPMessageHeader(connection, &len, &code)) == GTIRecvMessage)
	{
		// Check for a UDP Ping Ack.
		////////////////////////////
		if(code == GTIUDPPingAck)
		{
			// We can send UDP.
			///////////////////
			if(connection->UDPSocket != INVALID_SOCKET)
				connection->canSendUDP = GTTrue;

			// Store the ping time.
			///////////////////////
			connection->UDPPingTime = (current_time() - connection->lastUDPPing);
		}
		// Check for a Keep-alive.
		//////////////////////////
		else if(code == GTITCPKeepalive)
		{
			// Do nothing.
			//////////////
		}
		// Check for a message.
		///////////////////////
		else if((code == GTITCPUnreliable) || (code == GTIAppMessage))
		{
			GTBool reliable;

			// Store if its reliable.
			/////////////////////////
			reliable = (code == GTIAppMessage);

			// Deliver it.
			//////////////
			if(!gtiRecvTCPMessage(connection, gtiReceiveTCPMessageHandler, &handlerResult, (void *)&reliable))
				return gtiConnectionClosed(connection, GTNotEnoughMemory);

			// If the handler failed, pass it on.
			/////////////////////////////////////
			if(!handlerResult)
				return GTTrue;
		}
	}

	if(recvResult == GTIRecvError)
		return gtiConnectionClosed(connection, GTConnectionError);
	if(recvResult == GTIRecvClosed)
		return gtiConnectionClosed(connection, GTRemoteClose);

	return GTTrue;
}

GTBool gtiHandleReceivedUDP
(
	GTConnection connection,
	int len,
	GTIControlCode code
)
{
#ifdef STATS
	// Update stats.
	////////////////
	connection->incomingBytesUDP += len;
#endif

	// Handle based on code.
	////////////////////////
	if(code == GTIUDPPing)
		gtiSendMessage(connection, NULL, 0, GTTrue, GTIUDPPingAck);
	else if(code == GTIAppMessage)
	{
		// Check if we have any receive filters.
		////////////////////////////////////////
		if(connection->receiveFilters)
		{
			// Filter the receive.
			//////////////////////
			if(!gtiReceiveFilterCallback(connection, 0, connection->UDPBuffer, len, GTFalse))
				return GTFalse;
		}
		else
		{
			// Call the callback.
			/////////////////////
			if(!gtiReceivedCallback(connection, connection->UDPBuffer, len, GTFalse))
				return GTFalse;
		}
	}

	return GTTrue;
}

static GTBool gtiDoReceiveUDPMessages
(
	GTConnection connection
)
{
	int len;
	GTIControlCode code;
	GTIRecvResult result;

	// If this connection has a listener, that'll do the receiving.
	///////////////////////////////////////////////////////////////
	if(connection->listener)
		return GTTrue;

	// Check for no socket.
	///////////////////////
	if(connection->UDPSocket == INVALID_SOCKET)
		return GTTrue;

	// Receive messages.
	////////////////////
	while((result = gtiReceiveUDPMessage(connection->UDPSocket, connection->UDPBuffer, &len, &code, NULL)) == GTIRecvMessage)
	{
		// Handle message.
		//////////////////
		if(!gtiHandleReceivedUDP(connection, len, code))
			return GTFalse;
	}

	// Check for error.
	///////////////////
	if(result == GTIRecvError)
		gtiCloseConnectionUDPSocket(connection);

	return GTTrue;
}

static GTBool gtiDoUDPPing
(
	GTConnection connection
)
{
	unsigned long now;

	// Can send UDP?
	////////////////
	if(connection->canSendUDP)
		return GTTrue;

	// Check for no socket.
	///////////////////////
	if(connection->UDPSocket == INVALID_SOCKET)
		return GTTrue;

	// Check if its time for a new one.
	///////////////////////////////////
	now = current_time();
	if((now - connection->lastUDPPing) > GTI_UDP_PING_TIME)
	{
		// Did we hit the limit?
		////////////////////////
		if(connection->UDPPingCount == GTI_UDP_PING_ATTEMPTS)
		{
			gtiCloseConnectionUDPSocket(connection);
			return GTTrue;
		}

		// Update states.
		/////////////////
		connection->lastUDPPing = now;
		connection->UDPPingCount++;

		// Send the ping.
		/////////////////
		gtiSendMessage(connection, NULL, 0, GTFalse, GTIUDPPing);
	}

	return GTTrue;
}

static GTBool gtiDoKeepAlives
(
	GTConnection connection
)
{
	unsigned long now;

	// Get the current time.
	////////////////////////
	now = current_time();

	// Check if we can send UDP.
	////////////////////////////
	if((connection->UDPSocket != INVALID_SOCKET) && connection->canSendUDP)
	{
		// Check if it's time for a UDP keep-alive.
		///////////////////////////////////////////
		if((now - connection->lastUDPSend) > GTI_UDP_KEEPALIVE_TIME)
		{
			// Send the keep-alive.
			///////////////////////
			gtiSendMessage(connection, NULL, 0, GTFalse, GTIUDPKeepalive);
		}
	}

	// Check if it's time for a TCP keep-alive.
	///////////////////////////////////////////
	if((now - connection->lastTCPSend) > GTI_TCP_KEEPALIVE_TIME)
		gtiSendMessage(connection, NULL, 0, GTTrue, GTITCPKeepalive);

	return GTTrue;
}

// Return False if the connection is closed.
////////////////////////////////////////////
static GTBool gtiConnectionThink
(
	GTConnection connection
)
{
	// If this connection is marked as closed,
	// just do whatever cleanup we need to do.
	//////////////////////////////////////////
	if(connection->state == GTIClosed)
	{
		// Flush the outgoing buffer.
		/////////////////////////////
		if(connection->outgoingBuffer.len)
			gtiSendBufferedData(connection);

		return GTFalse;
	}
		
	// Check for looking up the remote system.
	//////////////////////////////////////////
	if(connection->state == GTIHostLookup)
		return gtiDoHostLookup(connection);

	// Check for connecting.
	////////////////////////
	if(connection->state == GTIConnecting)
		return gtiDoConnecting(connection);

	// Send any buffered outgoing data.
	///////////////////////////////////
	if(connection->outgoingBuffer.len)
		gtiSendBufferedData(connection);

	// Check if we're negotiating.
	//////////////////////////////
	if(connection->state == GTINegotiating)
		return gtiDoNegotiating(connection);

	// Did we receive a challenge?
	//////////////////////////////
	if(connection->state == GTIReceivedChallenge)
		return gtiDoReceivedChallenge(connection);

	// Are we waiting for an accept/reject?
	///////////////////////////////////////
	if(connection->state == GTIAwaitingAcceptReject)
		return GTTrue;

	// Handle incoming TCP messages.
	////////////////////////////////
	if(!gtiDoReceiveTCPMessages(connection))
		return GTFalse;

	// Handle incoming UDP messages.
	////////////////////////////////
	if(!gtiDoReceiveUDPMessages(connection))
		return GTFalse;

	// Do a ping if needed.
	///////////////////////
	if(!gtiDoUDPPing(connection))
		return GTFalse;

	// Do keep-alives if needed.
	////////////////////////////
	return gtiDoKeepAlives(connection);
}

GTBool gtiConnectionsThink
(
	GTConnection checkConnection
)
{
	GTConnection connection;
	GTConnection pcurr;
	GTBool result = GTTrue;

	// Loop through the connections.
	////////////////////////////////
	for(pcurr = &FirstConnection ; pcurr->pnext ; )
	{
		connection = pcurr->pnext;

#ifdef STATS
#if 0
		if(pcurr == &FirstConnection)
		{
			char buffer[128];
			unsigned long diff;
			diff = (current_time() - connection->startConnectTime);
			if(diff)
			{
				sprintf(buffer, "I_TCP: %dB (%dBps)\n",
					connection->incomingBytesTCP,
					connection->incomingBytesTCP * 1000 / diff);
				OutputDebugString(buffer);
				sprintf(buffer, "O_TCP: %dB (%dBps)\n",
					connection->outgoingBytesTCP,
					connection->outgoingBytesTCP * 1000 / diff);
				OutputDebugString(buffer);
				sprintf(buffer, "I_UDP: %dB (%dBps)\n",
					connection->incomingBytesUDP,
					connection->incomingBytesUDP * 1000 / diff);
				OutputDebugString(buffer);
				sprintf(buffer, "O_UDP: %dB (%dBps)\n",
					connection->outgoingBytesUDP,
					connection->outgoingBytesUDP * 1000 / diff);
				OutputDebugString(buffer);
			}
		}
#endif
#endif

		// Think.
		/////////
		gtiConnectionThink(connection);

		// Check if we need to return a closed connection.
		//////////////////////////////////////////////////
		if((connection == checkConnection) && (connection->state == GTIClosed))
			result = GTFalse;

		// gsifree a closed connection if we can.
		//////////////////////////////////////
		if((connection->state == GTIClosed) && gtiCanFreeConnection(connection))
			gtiFreeConnection(connection);
		else
			pcurr = connection;
	}

	return result;
}

GTBool gtiIsValidConnection
(
	GTConnection connection
)
{
	GTConnection pcurr;

	for(pcurr = &FirstConnection ; pcurr->pnext ; pcurr = pcurr->pnext)
		if(pcurr->pnext == connection)
			return GTTrue;

	return GTFalse;
}

GTConnection gtiGetHeadConnection
(
	void
)
{
	return FirstConnection.pnext;
}

#ifdef _DEBUG
#include <stdio.h>
void gtiConnectionsDebugInfo
(
	void (* callback)(const char * output)
)
{
	GTConnection connection;
	char buffer[128];
	int index = 0;

	callback("Connections:");
	callback("index  state");
	callback("-----  -----");
	for(connection = FirstConnection.pnext ; connection ; connection = connection->pnext)
	{
		sprintf(buffer, "%5d  %5d", index++, connection->state);
		callback(buffer);
	}
	callback("End of Connections");
}
#endif