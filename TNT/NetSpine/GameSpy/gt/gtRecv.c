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
#include <string.h>
#include "gtRecv.h"
#include "gtConnection.h"

// Receive a message on the stack if its
// under this size. If over this, gsimalloc it.
////////////////////////////////////////////
#ifndef GTI_STACK_RECV_MAX
#define GTI_STACK_RECV_MAX      4096
#endif

#ifdef BLOCKING_SOCKETS
GTBool gtiCanRecvOnSocket
(
	SOCKET socket
)
{
	fd_set fdRead;
	struct timeval timeout;
	int rcode;

	// Setup the fd stuff.
	//////////////////////
	FD_ZERO(&fdRead);
	FD_SET(socket, &fdRead);

	// Setup the timeout.
	/////////////////////
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	// Do the select.
	/////////////////
	rcode = select(FD_SETSIZE, &fdRead, NULL, NULL, &timeout);
	if(rcode == SOCKET_ERROR)
	{
		GOAGetLastError(socket);
		return GTFalse;
	}

	// Check if no result.
	//////////////////////
	if(rcode == 0)
		return GTFalse;

	// Check for success.
	/////////////////////
	if(FD_ISSET(socket, &fdRead))
		return GTTrue;

	// Can't read.
	//////////////
	return GTFalse;
} 
#endif

/********
*********
** TCP **
*********
********/

// Returns the length of the message.
// If -1, couldn't parse the header.
/////////////////////////////////////
static int gtiParseTCPHeader
(
	GTConnection connection,
	GTIControlCode * code
)
{
	GTByte curByte;
	GTBool control;
	unsigned short len;
	GTByte * bufBuffer;
	int bufStart;
	int bufLen;
	int bufSize;
	GTICircularBuffer * buffer;
	int nHeaderLen;

	buffer = &connection->incomingBuffer;
	assert(buffer);
	if(!buffer)
		return -1;

	bufBuffer = buffer->buffer;
	bufStart = buffer->start;
	bufLen = buffer->len;
	bufSize = buffer->size;

	// Needs at least 2 bytes for the length.
	/////////////////////////////////////////
	assert(bufLen >= 0);
	if(bufLen < 2)
		return -1;

	// Get the first byte.
	//////////////////////
	curByte = bufBuffer[bufStart];

	// Check the control bit.
	/////////////////////////
	control = ((curByte & 0x80) != 0);

	// Get the message length.
	//////////////////////////
	len = (((curByte & 0x7F) << 8) & 0xFF00);
	curByte = bufBuffer[(bufStart + 1) % bufSize];
	len |= (curByte & 0x00FF);

	// Get the header length.
	/////////////////////////
	nHeaderLen = (2 + (control?1:0));

	// Check the length.
	////////////////////
	if(bufLen < (nHeaderLen + len))
		return -1;

	// Get the code.
	////////////////
	if(code)
	{
		if(control)
			*code = (GTIControlCode)bufBuffer[(bufStart + 2) % bufSize];
		else
			*code = (GTIControlCode)GTIAppMessage;
	}

	// Adjust the buffer.
	/////////////////////
	buffer->len -= nHeaderLen;
	buffer->start += nHeaderLen;
	buffer->start %= bufSize;

	// Store the size.
	//////////////////
	connection->messageLen = len;

	// Return the message len.
	//////////////////////////
	return len;
}

GTIRecvResult gtiRecvTCPMessageHeader
(
	GTConnection connection,
	int * len,
	GTIControlCode * code
)
{
	int rcode;
	GTByte * buffer;
	int size;
	int bufLen;
	int start;
	int recvStart;
	int recvLen;
	GTBool bReceived = GTFalse;
	GTBool bError = GTFalse;
	int nMessageLen;

	assert(connection->TCPSocket != INVALID_SOCKET);
	if(connection->TCPSocket == INVALID_SOCKET)
		return GTIRecvError;

	buffer = connection->incomingBuffer.buffer;
	size = connection->incomingBuffer.size;
	bufLen = connection->incomingBuffer.len;
	start = connection->incomingBuffer.start;

	// Check for space to recv to.
	// Also, if the connection was already closed, don't recv.
	//////////////////////////////////////////////////////////
	if(!connection->TCPRecvClose && (bufLen != size))
	{
		do
		{
#ifdef BLOCKING_SOCKETS
			// Check for something to read.
			///////////////////////////////
			if(!gtiCanRecvOnSocket(connection->TCPSocket))
			{
				rcode = SOCKET_ERROR;
				GOAGetLastError(connection->TCPSocket);
				break;
			}
#endif
			// Figure out where the recv starts and how much space there is.
			////////////////////////////////////////////////////////////////
			recvStart = ((start + bufLen) % size);
			recvLen = min(size - bufLen, size - recvStart);

			// Do the recv.
			///////////////
			rcode = recv(connection->TCPSocket, buffer + recvStart, recvLen, 0);

			// If we received, update the length in the buffer.
			///////////////////////////////////////////////////
			if(rcode > 0)
			{
				bReceived = GTTrue;
				bufLen += rcode;
#ifdef STATS
				// Update stats.
				////////////////
				connection->incomingBytesTCP += rcode;
#endif
			}
		}
		while((rcode > 0) && (bufLen < size));

		// Update the buffer's length.
		//////////////////////////////
		connection->incomingBuffer.len = bufLen;

		// Check the result.
		////////////////////
		if(rcode == SOCKET_ERROR)
		{
			// Check for an error.
			//////////////////////
			int errorCode = GOAGetLastError(connection->TCPSocket);
			if(errorCode == WSAECONNRESET)
				connection->TCPRecvClose = GTTrue;
			else if((errorCode != 0) && (errorCode != WSAEWOULDBLOCK) && (errorCode != WSAEINPROGRESS))
				bError = GTTrue;
		}
		else if(rcode == 0)
		{
			// Flag the close.
			//////////////////
			connection->TCPRecvClose = GTTrue;
		}
	}

	// Check for a message.
	///////////////////////
	nMessageLen = gtiParseTCPHeader(connection, code);
	if(nMessageLen > -1)
	{
		// We've got a message.
		///////////////////////
		if(len)
			*len = nMessageLen;
		return GTIRecvMessage;
	}

	// Was there an error?
	//////////////////////
	if(bError)
		return GTIRecvError;

	// Was the connection closed?
	/////////////////////////////
	if(connection->TCPRecvClose)
		return GTIRecvClosed;

	// No message.
	//////////////
	return GTIRecvNone;
}

static void gtiBufferToMemory
(
	GTByte * memory,
	GTICircularBuffer * buffer,
	int len
)
{
	GTByte * src;
	int size;
	int start;
	int firstLen;

	src = buffer->buffer;
	size = buffer->size;
	start = buffer->start;
	firstLen = (size - start);

	memcpy(memory, src + start, firstLen);
	memcpy(memory + firstLen, src, len - firstLen);
}

static GTBool gtiStackTCPMessageHandler
(
	GTConnection connection,
	GTITCPMessageHandler handler,
	void * userData,
	GTBool * result
)
{
	GTBool handlerResult;

	// The buffer.
	//////////////
	GTByte stackBuffer[GTI_STACK_RECV_MAX];

	// Put it in memory.
	////////////////////
	gtiBufferToMemory(stackBuffer, &connection->incomingBuffer, connection->messageLen);

	// Call the handler.
	////////////////////
	handlerResult = handler(connection, stackBuffer, connection->messageLen, userData);
	if(result)
		*result = handlerResult;

	return handlerResult;
}

GTBool gtiRecvTCPMessage
(
	GTConnection connection,
	GTITCPMessageHandler handler,
	GTBool * result,
	void * userData
)
{
	GTBool handlerResult;
	int messageLen = connection->messageLen;
	int start = connection->incomingBuffer.start;

	assert(messageLen <= connection->incomingBuffer.len);
	if(messageLen > connection->incomingBuffer.len)
		return GTFalse;

	// Is the message not wrapped?
	//////////////////////////////
	if((start + messageLen) <= connection->incomingBuffer.size)
	{
		// Call the handler.
		////////////////////
		handlerResult = handler(connection, connection->incomingBuffer.buffer + start, messageLen, userData);
		if(result)
			*result = handlerResult;
	}
	// Is this message within the stack limit?
	//////////////////////////////////////////
	else if(messageLen <= GTI_STACK_RECV_MAX)
	{
		handlerResult = gtiStackTCPMessageHandler(connection, handler, userData, result);
	}
	else
	{
		GTByte * message;

		// Allocate the message.
		////////////////////////
		message = (GTByte *)gsimalloc(messageLen);
		if(!message)
			return GTFalse;

		// Copy the message to the heap.
		////////////////////////////////
		gtiBufferToMemory(message, &connection->incomingBuffer, messageLen);

		// Call the handler.
		////////////////////
		handlerResult = handler(connection, message, messageLen, userData);
		if(result)
			*result = handlerResult;

		// gsifree the memory.
		///////////////////
		gsifree(message);
	}

	// Update the buffer.
	/////////////////////
	if(handlerResult)
	{
		if(messageLen == connection->incomingBuffer.len)
		{
			connection->incomingBuffer.len = 0;
			connection->incomingBuffer.start = 0;
		}
		else
		{
			connection->incomingBuffer.len -= messageLen;
			connection->incomingBuffer.start += messageLen;
			connection->incomingBuffer.start %= connection->incomingBuffer.size;
		}
	}

	return GTTrue;
}

/********
*********
** UDP **
*********
********/

GTIRecvResult gtiReceiveUDPMessage
(
	SOCKET socket,
	GTByte * buffer,
	int * len,
	GTIControlCode * code,
	SOCKADDR_IN * address
)
{
	int rcode;
	int addressLen;
	int * pAddressLen;

	assert(code);
	assert(len);

#ifdef BLOCKING_SOCKETS
	// Check for something to read.
	///////////////////////////////
	if(!gtiCanRecvOnSocket(socket))
		return GTIRecvNone;
#endif

	// Do the actual receive.
	/////////////////////////
	if(address)
	{
		addressLen = sizeof(SOCKADDR_IN);
		pAddressLen = &addressLen;
	}
	else
	{
		pAddressLen = NULL;
	}
	rcode = recvfrom(socket, buffer, GTI_MAX_UDP_SIZE, 0, (SOCKADDR *)address, (unsigned int*)pAddressLen);

	// Check for no message.
	////////////////////////
	if(rcode == SOCKET_ERROR)
	{
		// If it was a "bad" error, close the UDP socket.
		/////////////////////////////////////////////////
		rcode = GOAGetLastError(socket);
		if((rcode != WSAEWOULDBLOCK) && (rcode != WSAEINPROGRESS) && (rcode != WSAECONNRESET))
			return GTIRecvError;
		return GTIRecvNone;
	}

	// Check for control messages.
	//////////////////////////////
	if((rcode > GTI_UDP_MAGIC_STRING_LEN) && (memcmp(buffer, GTI_UDP_MAGIC_STRING, GTI_UDP_MAGIC_STRING_LEN) == 0))
		*code = (GTIControlCode)buffer[GTI_UDP_MAGIC_STRING_LEN];
	else
		*code = GTIAppMessage;

	// Store the length.
	////////////////////
	*len = rcode;

	return GTIRecvMessage;
}