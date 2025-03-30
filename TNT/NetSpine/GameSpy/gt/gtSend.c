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
#include "gtSend.h"

#define GTI_MIN_BUFFER_INCREMENT       (2 * 1024)

#ifdef BLOCKING_SOCKETS
static GTBool gtiCanSendOnSocket
(
	SOCKET socket
)
{
	fd_set fdWrite;
	struct timeval timeout;
	int rcode;

	// Setup the fd stuff.
	//////////////////////
	FD_ZERO(&fdWrite);
	FD_SET(socket, &fdWrite);

	// Setup the timeout.
	/////////////////////
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	// Do the select.
	/////////////////
	rcode = select(FD_SETSIZE, NULL, &fdWrite, NULL, &timeout);
	if(rcode == SOCKET_ERROR)
		return GTFalse;

	// Check if no result.
	//////////////////////
	if(rcode == 0)
		return GTFalse;

	// Check for success.
	/////////////////////
	if(FD_ISSET(socket, &fdWrite))
		return GTTrue;

	// Can't write.
	///////////////
	return GTFalse;
}
#endif

static void gtiSendUDP
(
	GTConnection connection,
	const GTByte * message,
	int len
)
{
	SOCKADDR_IN address;

	// Check for no socket.
	///////////////////////
	if(connection->UDPSocket == INVALID_SOCKET)
		return;

	// Setup the address.
	/////////////////////
	memset(&address, 0, sizeof(SOCKADDR_IN));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = connection->remoteIP;
	address.sin_port = htons(connection->remoteUDPPort);

	// Do the send.
	///////////////
	sendto(connection->UDPSocket, (char *)message, len, 0, (SOCKADDR *)&address, sizeof(SOCKADDR_IN));

	// Last send time.
	//////////////////
	connection->lastUDPSend = current_time();

#ifdef STATS
	// Update stats.
	////////////////
	connection->outgoingBytesUDP += len;
#endif
}

static int gtiSendTCPSegment
(
	GTConnection connection,
	const GTByte * message,
	int len
)
{
	int rcode;

#ifdef BLOCKING_SOCKETS
	// Can we send data?
	if(!gtiCanSendOnSocket(connection->TCPSocket))
		return 0;
#endif

	// Do the send.
	///////////////
	rcode = send(connection->TCPSocket, message, len, 0);

	// Did we send data?
	////////////////////
	if(rcode > 0)
		return rcode;

	return 0;
}

// Return number of bytes read, or 0.
// No errors generated - those will be found in recv.
/////////////////////////////////////////////////////
static int gtiSendTCP
(
	GTConnection connection,
	const GTByte * message,
	int len
)
{
	int total;
	int rcode;
	int sendLen;

	// Check for no socket.
	///////////////////////
	assert(connection->TCPSocket != INVALID_SOCKET);
	if(connection->TCPSocket == INVALID_SOCKET)
		return 0;

	// Check for nothing to send.
	/////////////////////////////
	if(!message || !len)
		return 0;

	// Send in 1400 byte segments.
	// Over that causes fragmenting.
	////////////////////////////////
	total = 0;
	do
	{
		sendLen = min(len, 1400);
		rcode = gtiSendTCPSegment(connection, message, sendLen);
		message += rcode;
		len -= rcode;
		total += rcode;
	}
	while(rcode && len);

#ifdef STATS
	// Update stats.
	////////////////
	connection->outgoingBytesTCP += total;
#endif

	// Last send time.
	//////////////////
	connection->lastTCPSend = current_time();

	return total;
}

void gtiSendBufferedData
(
	GTConnection connection
)
{
	int nSent;
	GTByte * buffer;
	int size;
	int len;
	int start;
	
	assert(connection->outgoingBuffer.len);
	if(!connection->outgoingBuffer.len)
		return;
	
	buffer = connection->outgoingBuffer.buffer;
	size = connection->outgoingBuffer.size;
	len = connection->outgoingBuffer.len;
	start = connection->outgoingBuffer.start;
	
	// Do a send.
	/////////////
	while(len && (nSent = gtiSendTCP(connection, buffer + start, min(len, size - start))))
	{
		// Adjust according to sent amount.
		///////////////////////////////////
		len -= nSent;
		start += nSent;
		if(start == size)
			start = 0;
	}
	
	// Check for everything sent.
	/////////////////////////////
	if(!len)
	{
		connection->outgoingBuffer.len = 0;
		connection->outgoingBuffer.start = 0;
		return;
	}
		  
	// Check for nothing sent.
	//////////////////////////
	if(len == connection->outgoingBuffer.len)
	  return;

	// Done.
	////////
	connection->outgoingBuffer.len = len;
	connection->outgoingBuffer.start = start;
}

static GTBool gtiBufferSend
(
	GTConnection connection,
	const GTByte * data,
	int dataLen
)
{
	GTByte * buffer;
	int size;
	int len;
	int start;
	int copyStart;
	int freeSpace;

	// Check for nothing to send.
	/////////////////////////////
	if(!data || !dataLen)
		return GTTrue;

	buffer = connection->outgoingBuffer.buffer;
	size = connection->outgoingBuffer.size;
	len = connection->outgoingBuffer.len;
	start = connection->outgoingBuffer.start;

	// Is there enough space in the buffer?
	///////////////////////////////////////
	if(dataLen > (size - len))
	{
		int oldSize = size;
		
		// Figure out the new size to allocate.
		///////////////////////////////////////
		size = max(size + GTI_MIN_BUFFER_INCREMENT, size + dataLen);

		// Reallocate the buffer.
		/////////////////////////
		buffer = (GTByte *)gsirealloc(buffer, size);
		if(!buffer)
			return GTFalse;

		// If the buffer is wrapped, move the part that was at
		// the end of the buffer to the new end of the buffer.
		//////////////////////////////////////////////////////
		if((start + len) > oldSize)
		{
			int newStart = (start + (size - oldSize));
			memmove(buffer + newStart, buffer + start, oldSize - start);
			start = newStart;
		}

		// This is the only place we resize, so set these here.
		///////////////////////////////////////////////////////
		connection->outgoingBuffer.buffer = buffer;
		connection->outgoingBuffer.size = size;
		connection->outgoingBuffer.start = start;
	}

	// Find where the copying starts.
	/////////////////////////////////
	copyStart = ((start + len) % size);

	// How much gsifree space before the end of the buffer?
	////////////////////////////////////////////////////
	freeSpace = (size - copyStart);

	// Check if we can copy in without wrapping.
	////////////////////////////////////////////
	if(dataLen <= freeSpace)
	{
		// Copy it all at once.
		///////////////////////
		memcpy(buffer + copyStart, data, dataLen);
	}
	else
	{
		// Do the first copy.
		/////////////////////
		memcpy(buffer + copyStart, data, freeSpace);

		// Do the second copy.
		//////////////////////
		memcpy(buffer, data + freeSpace, dataLen - freeSpace);
	}

	// Set the new length.
	//////////////////////
	connection->outgoingBuffer.len += dataLen;

	return GTTrue;
}

void gtiSendTCPData
(
	GTConnection connection,
	const GTByte * data,
	int len
)
{
	int nBytesSent;

	// Check if there's outgoing data buffered.
	///////////////////////////////////////////
	if(connection->outgoingBuffer.len)
	{
		// Send buffered data.
		//////////////////////
		gtiSendBufferedData(connection);

		// Is there still data buffered?
		////////////////////////////////
		if(connection->outgoingBuffer.len)
		{
			// Buffer it.
			/////////////
			gtiBufferSend(connection, data, len);
			return;
		}
	}

	// Send it.
	///////////
	nBytesSent = gtiSendTCP(connection, data, len);

	// Buffer the unsent part.
	//////////////////////////
	if(nBytesSent != len)
		gtiBufferSend(connection, data + nBytesSent, len - nBytesSent);
}

// Returns the header length.
/////////////////////////////
static int gtiMakeTCPHeader
(
	int len,
	GTIControlCode code,
	GTByte header[3]
)
{
	unsigned short netOrderLen;

	// Construct the header.
	////////////////////////
	netOrderLen = htons((unsigned short)len);
	memcpy(header, &netOrderLen, 2);
	if(code == GTIAppMessage)
		return 2;
	
	header[0] |= 0x80;
	header[2] = code;
	return 3;
}

void gtiSendTCPHeader
(
	GTConnection connection,
	int len,
	GTIControlCode code
)
{
	GTByte header[3];
	int nHeaderLen;

	// Construct it.
	////////////////
	nHeaderLen = gtiMakeTCPHeader(len, code, header);

	// Send it.
	///////////
	gtiSendTCPData(connection, header, nHeaderLen);
}

void gtiSendMessage
(
	GTConnection connection,
	const GTByte * message,
	int len,
	GTBool reliable,
	GTIControlCode code
)
{
	GTByte header[3];
	int nHeaderLen;
	int nTotalLen;

	if(!message || !len)
	{
		message = NULL;
		len = 0;
	}
	else if(len == -1)
		len = (strlen(message) + 1);

	// Check for too long.
	//////////////////////
	if(len > 0x7FFF)
		return;

	// Check for a UDP send.
	////////////////////////
	if(!reliable)
	{
		// Send over UDP if we can, or if its an exploratory ping.
		//////////////////////////////////////////////////////////
		if((connection->UDPSocket != INVALID_SOCKET) && (connection->canSendUDP || (code == GTIUDPPing)))
		{
			// If app message, just send.
			/////////////////////////////
			if(code == GTIAppMessage)
			{
				gtiSendUDP(connection, message, len);
			}
			else
			{
				char controlMessage[GTI_UDP_MAGIC_STRING_LEN + 1];

				memcpy(controlMessage, GTI_UDP_MAGIC_STRING, GTI_UDP_MAGIC_STRING_LEN);
				controlMessage[GTI_UDP_MAGIC_STRING_LEN] = code;

				gtiSendUDP(connection, controlMessage, GTI_UDP_MAGIC_STRING_LEN + 1);
			}

			return;
		}

		// If this is a UDP control message, we can't send it.
		//////////////////////////////////////////////////////
		if(code != GTIAppMessage)
			return;

		// If there's data buffered, we drop unreliable.
		////////////////////////////////////////////////
		if(connection->outgoingBuffer.len)
			return;

		// This is now unreliable over TCP.
		///////////////////////////////////
		code = GTITCPUnreliable;
	}

	// Construct the header.
	////////////////////////
	nHeaderLen = gtiMakeTCPHeader(len, code, header);

	// Get the total len.
	/////////////////////
	nTotalLen = (nHeaderLen + len);

	// Use the UDP buffer if we can.
	////////////////////////////////
	if(nTotalLen <= GTI_MAX_UDP_SIZE)
	{
		// Copy into the buffer.
		////////////////////////
		memcpy(connection->UDPBuffer, header, nHeaderLen);
		memcpy(connection->UDPBuffer + nHeaderLen, message, len);

		// Do the send.
		///////////////
		gtiSendTCPData(connection, connection->UDPBuffer, nTotalLen);
	}
	else
	{
		// Just do it in 2 sends.
		/////////////////////////
		gtiSendTCPHeader(connection, len, code);
		gtiSendTCPData(connection, message, len);
	}
}