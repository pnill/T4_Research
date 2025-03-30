/*
GameSpy GHTTP SDK 
Dan "Mr. Pants" Schoenblum
dan@gamespy.com

Copyright 1999-2001 GameSpy Industries, Inc

18002 Skypark Circle
Irvine, California 92614
949.798.4200 (Tel)
949.798.4299 (Fax)
devsupport@gamespy.com
*/

#include "ghttpProcess.h"
#include "ghttpCallbacks.h"
#include "ghttpPost.h"
#include "ghttpMain.h"
#include "ghttpCommon.h"

// Parse the URL into:
//   server address (and IP)
//   server port
//   request path.
/////////////////////////////
static GHTTPBool ghiParseURL
(
	GHIConnection * connection
)
{
	char * URL;
	int nIndex;
	char tempChar;
	char * str;

	assert(connection);
	if(!connection)
		return GHTTPFalse;

	// 2002.Apr.18.JED - Make sure we have an URL
	/////////////////////////////////////////////
	assert(connection->URL);
	if(!connection->URL)
		return GHTTPFalse;

	URL = connection->URL;

	// Check for "http://".
	//////////////////////
	if(strncmp(URL, "http://", 7) != 0)
		return GHTTPFalse;
	URL += 7;

	// Read the address.
	////////////////////
	nIndex = strcspn(URL, ":/");
	tempChar = URL[nIndex];
	URL[nIndex] = '\0';
	connection->serverAddress = strdup(URL);
	if(!connection->serverAddress)
		return GHTTPFalse;
	URL[nIndex] = tempChar;
	URL += nIndex;

	// Read the port.
	/////////////////
	if(*URL == ':')
	{
		URL++;
		connection->serverPort = (unsigned short)atoi(URL);
		if(!connection->serverPort)
			return GHTTPFalse;
		do
		{
			URL++;
		}while(*URL && (*URL != '/'));
	}
	else
	{
		connection->serverPort = GHI_DEFAULT_PORT;
	}

	// Read the path.
	/////////////////
	if(!*URL)
		URL = "/";
	connection->requestPath = strdup(URL);
	while((str = strchr(connection->requestPath, ' ')) != NULL)
		*str = '+';
	if(!connection->requestPath)
		return GHTTPFalse;

	return GHTTPTrue;
}

/****************
** HOST LOOKUP **
****************/
void ghiDoHostLookup
(
	GHIConnection * connection
)
{
	HOSTENT * host;
	const char * server;

	// Progress.
	////////////
	ghiCallProgressCallback(connection, NULL, 0);

	// Init sockets.
	////////////////
	SocketStartUp();

	// Parse the URL.
	/////////////////
	if(!ghiParseURL(connection))
	{
		connection->completed = GHTTPTrue;
		connection->result = GHTTPParseURLFailed;
		return;
	}

	// Check for using a proxy.
	///////////////////////////
	if(ghiProxyAddress)
		server = ghiProxyAddress;
	else
		server = connection->serverAddress;

	// Try resolving the address as an IP a.b.c.d number.
	/////////////////////////////////////////////////////
	connection->serverIP = inet_addr(server);
	if(connection->serverIP == INADDR_NONE)
	{
		// Try resolving with DNS.
		//////////////////////////
		host = gethostbyname(server);
		if(host == NULL)
		{
			connection->completed = GHTTPTrue;
			connection->result = GHTTPHostLookupFailed;
			return;
		}

		// Get the IP.
		//////////////
		connection->serverIP = *(unsigned int *)host->h_addr_list[0];
	}

	// Progress.
	////////////
	connection->state = GHTTPConnecting;
	ghiCallProgressCallback(connection, NULL, 0);
}

/***************
** CONNECTING **
***************/
void ghiDoConnecting
(
	GHIConnection * connection
)
{
	int rcode;
	SOCKADDR_IN address;
	GHTTPBool writeFlag;
	GHTTPBool exceptFlag;
	GHTTPBool bResult;

	// If we don't have a socket yet, set it up.
	////////////////////////////////////////////
	if(connection->socket == INVALID_SOCKET)
	{
		// Create the socket.
		/////////////////////
		connection->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(connection->socket == INVALID_SOCKET)
		{
			connection->completed = GHTTPTrue;
			connection->result = GHTTPSocketFailed;
			connection->socketError = GOAGetLastError(connection->socket);
			return;
		}

		// Set the socket to non-blocking.
		//////////////////////////////////
		if(!SetSockBlocking(connection->socket, 0))
		{
			connection->completed = GHTTPTrue;
			connection->result = GHTTPSocketFailed;
			connection->socketError = GOAGetLastError(connection->socket);
			return;
		}

		// If throttling, use a small receive buffer.
		/////////////////////////////////////////////
		if(connection->throttle)
			SetReceiveBufferSize(connection->socket, ghiThrottleBufferSize);

		// Setup the server address.
		////////////////////////////
		memset(&address, 0, sizeof(SOCKADDR_IN));
		address.sin_family = AF_INET;
		if(ghiProxyAddress)
			address.sin_port = htons(ghiProxyPort);
		else
			address.sin_port = htons(connection->serverPort);
		address.sin_addr.s_addr = connection->serverIP;

		// Start the connect.
		/////////////////////
		rcode = connect(connection->socket, (SOCKADDR *)&address, sizeof(SOCKADDR_IN));
		if(rcode == SOCKET_ERROR)
		{
			int socketError = GOAGetLastError(connection->socket);
			if((socketError != WSAEWOULDBLOCK) && (socketError != WSAEINPROGRESS))
			{
				connection->completed = GHTTPTrue;
				connection->result = GHTTPConnectFailed;
				connection->socketError = socketError;
				return;
			}
		}
	}

	// Check if the connect has completed.
	//////////////////////////////////////
	bResult = ghiSocketSelect(connection->socket, NULL, &writeFlag, &exceptFlag);
	if(!bResult || exceptFlag)
	{
		connection->completed = GHTTPTrue;
		connection->result = GHTTPConnectFailed;
		if(!bResult)
			connection->socketError = GOAGetLastError(connection->socket);
		return;
	}

	// Check if we're connected.
	////////////////////////////
	if(writeFlag)
	{
		// Progress.
		////////////
		connection->state = GHTTPSendingRequest;
		ghiCallProgressCallback(connection, NULL, 0);
	}
} 

/********************
** SENDING REQUEST **
********************/
void ghiDoSendingRequest
(
	GHIConnection * connection
)
{
	char * requestType;
	int oldPos;

	// If we haven't filled the send buffer yet, do that first.
	///////////////////////////////////////////////////////////
	if(!connection->sendBuffer.len)
	{
		// Fill in the request line.
		////////////////////////////
		if(connection->post)
			requestType = "POST ";
		else if(connection->type == GHIHEAD)
			requestType = "HEAD ";
		else
			requestType = "GET ";
		ghiAppendDataToBuffer(&connection->sendBuffer, requestType, 0);
		if(ghiProxyAddress)
			ghiAppendDataToBuffer(&connection->sendBuffer, connection->URL, 0);
		else
			ghiAppendDataToBuffer(&connection->sendBuffer, connection->requestPath, 0);
		ghiAppendDataToBuffer(&connection->sendBuffer, " HTTP/1.1" CRLF, 0);

		// Add the host header.
		///////////////////////
		if(connection->serverPort == GHI_DEFAULT_PORT)
		{
			ghiAppendHeaderToBuffer(&connection->sendBuffer, "Host", connection->serverAddress);
		}
		else
		{
			ghiAppendDataToBuffer(&connection->sendBuffer, "Host: ", 0);
			ghiAppendDataToBuffer(&connection->sendBuffer, connection->serverAddress, 0);
			ghiAppendCharToBuffer(&connection->sendBuffer, ':');
			ghiAppendIntToBuffer(&connection->sendBuffer, connection->serverPort);
			ghiAppendDataToBuffer(&connection->sendBuffer, CRLF, 2);
		}

		// Add the user-agent header.
		/////////////////////////////
		ghiAppendHeaderToBuffer(&connection->sendBuffer, "User-Agent", "GameSpyHTTP/1.0");

		// We don't do persistant connections.
		//////////////////////////////////////
		ghiAppendHeaderToBuffer(&connection->sendBuffer, "Connection", "close");

		// Post needs extra headers.
		////////////////////////////
		if(connection->post)
		{
			char buf[16];

			// Add the content-length header.
			/////////////////////////////////
			sprintf(buf, "%d", connection->postingState.totalBytes);
			ghiAppendHeaderToBuffer(&connection->sendBuffer, "Content-Length", buf);

			// Add the content-type header.
			///////////////////////////////
			ghiAppendHeaderToBuffer(&connection->sendBuffer, "Content-Type", ghiPostGetContentType(connection));
		}

		// Add user-headers.
		////////////////////
		if(connection->sendHeaders)
			ghiAppendDataToBuffer(&connection->sendBuffer, connection->sendHeaders, 0);

		// Add the blank line to finish it off.
		///////////////////////////////////////
		ghiAppendDataToBuffer(&connection->sendBuffer, CRLF, 2);
	}

	// Store the old position.
	//////////////////////////
	oldPos = connection->sendBuffer.pos;

	// Send what we can.
	////////////////////
	if(!ghiSendBuffer(&connection->sendBuffer, connection))
		return;

	// Log anything we sent.
	////////////////////////
	if(connection->sendBuffer.pos != oldPos)
		ghiLog(connection->sendBuffer.data + oldPos, connection->sendBuffer.pos - oldPos);

	// Check for data still buffered.
	/////////////////////////////////
	if(connection->sendBuffer.pos < connection->sendBuffer.len)
		return;

	// Clear the send buffer.
	/////////////////////////
	ghiResetBuffer(&connection->sendBuffer);

	// Finished sending.
	////////////////////
	if(connection->post)
		connection->state = GHTTPPosting;
	else
		connection->state = GHTTPWaiting;
	ghiCallProgressCallback(connection, NULL, 0);
}

/************
** POSTING **
************/
void ghiDoPosting
(
	GHIConnection * connection
)
{
	GHIPostingResult result;
	int oldBytesPosted;

	// Store the old bytes posted.
	//////////////////////////////
	oldBytesPosted = connection->postingState.bytesPosted;

	// Do some posting.
	///////////////////
	result = ghiPostDoPosting(connection);

	// Check for an error.
	//////////////////////
	if(result == GHIPostingError)
	{
		// Make sure we already set the error stuff.
		////////////////////////////////////////////
		assert(connection->completed && connection->result);

		// Cleanup the posting state.
		/////////////////////////////
		ghiPostCleanupState(connection);

		return;
	}

	// Call the callback if we sent anything.
	/////////////////////////////////////////
	if(oldBytesPosted != connection->postingState.bytesPosted)
		ghiCallPostCallback(connection);

	// Check for done.
	//////////////////
	if(result == GHIPostingDone)
	{
		// Cleanup the posting state.
		/////////////////////////////
		ghiPostCleanupState(connection);

		// Set the new connection state.
		////////////////////////////////
		connection->state = GHTTPWaiting;
		ghiCallProgressCallback(connection, NULL, 0);

		return;
	}
}

/************
** WAITING **
************/
void ghiDoWaiting
(
	GHIConnection * connection
)
{
	GHTTPBool readFlag;

	// We're waiting to receive something.
	//////////////////////////////////////
	if(!ghiSocketSelect(connection->socket, &readFlag, NULL, NULL))
	{
		connection->completed = GHTTPTrue;
		connection->result = GHTTPSocketFailed;
		connection->socketError = GOAGetLastError(connection->socket);
		return;
	}

	// Check for waiting data.
	//////////////////////////
	if(readFlag)
	{
		// Ready to receive.
		////////////////////
		connection->state = GHTTPReceivingStatus;
		ghiCallProgressCallback(connection, NULL, 0);
	}
}

// Parse the status line.
/////////////////////////
static GHTTPBool ghiParseStatus
(
	GHIConnection * connection
)
{
	int majorVersion;
	int minorVersion;
	int statusCode;
	int statusStringIndex;
	int rcode;
	char c;

	assert(connection);
	assert(connection->recvBuffer.len > 0);

	// Parse the string.
	////////////////////
	rcode = sscanf(connection->recvBuffer.data, "HTTP/%d.%d %d%n",
		&majorVersion,
		&minorVersion,
		&statusCode,
		&statusStringIndex);

	// Figure out where the status string starts.
	/////////////////////////////////////////////
	while((c = connection->recvBuffer.data[statusStringIndex]) != '\0' && isspace(c))
		statusStringIndex++;

	// Check what we got.
	/////////////////////
	if((rcode != 3) ||     // Not all fields read.
		//!*statusString ||  // No status string.  PANTS|9.16.02 - apparently some servers don't return a status string
		(majorVersion < 1) ||  // Major version is less than 1.
		(statusCode < 100) ||        // 1xx is lowest status code.
		(statusCode >= 600))         // 5xx is highest status code.
	{
		connection->completed = GHTTPTrue;
		connection->result = GHTTPBadResponse;
		return GHTTPFalse;
	}

	// Set connection members.
	//////////////////////////
	connection->statusMajorVersion = majorVersion;
	connection->statusMinorVersion = minorVersion;
	connection->statusCode = statusCode;
	connection->statusStringIndex = statusStringIndex;

	return GHTTPTrue;
}

/*********************
** RECEIVING STATUS **
*********************/
void ghiDoReceivingStatus
(
	GHIConnection * connection
)
{
	char buffer[1024];
	int bufferLen;
	GHIRecvResult result;
	char * endOfStatus;

	// Get data.
	////////////
	bufferLen = sizeof(buffer);
	result = ghiDoReceive(connection, buffer, &bufferLen);

	// Handle error or no data.
	///////////////////////////
	if(result == GHIError)
		return;
	if((result == GHINoData) && (connection->recvBuffer.pos == connection->recvBuffer.len))
		return;

	// Only append data if we got data.
	///////////////////////////////////
	if(result == GHIRecvData)
	{
		// Add the data to the status buffer.
		/////////////////////////////////////
		if(!ghiAppendDataToBuffer(&connection->recvBuffer, buffer, bufferLen))
			return;
	}

	// Check if the status is finished.
	/////////////////////////////////////
	endOfStatus = strstr(connection->recvBuffer.data, CRLF);
	if(endOfStatus)
	{
		int statusLength;

		// Cap the status.
		//////////////////
		*endOfStatus = '\0';

		// Get the status length.
		/////////////////////////
		statusLength = (endOfStatus - connection->recvBuffer.data);

		// Log it.
		//////////
		ghiLog(connection->recvBuffer.data, statusLength);
		ghiLog("\n", 1);

		// Parse the status line.
		/////////////////////////
		if(!ghiParseStatus(connection))
			return;

		// Set the buffer's position to the start of the headers.
		/////////////////////////////////////////////////////////
		connection->recvBuffer.pos = (statusLength + 2);

		// We're receiving headers now.
		///////////////////////////////
		connection->state = GHTTPReceivingHeaders;
		ghiCallProgressCallback(connection, NULL, 0);
	}
	else if(result == GHIConnClosed)
	{
		// Connection closed.
		/////////////////////
		connection->completed = GHTTPTrue;
		connection->result = GHTTPBadResponse;
		connection->socketError = GOAGetLastError(connection->socket);
		return;
	}
}

// Delivers incoming file data to the appropriate place,
// then calls the progress callback.
// For GetFile, adds to buffer.
// For SaveFile, writes to disk.
// For StreamFile, does nothing.
// Returns false on error.
////////////////////////////////////////////////////////
static GHTTPBool ghiDeliverIncomingFileData
(
	GHIConnection * connection,
	char * data,
	int len
)
{
	char * buffer = NULL;
	int bufferLen = 0;

	// Add this to the total.
	/////////////////////////
	connection->fileBytesReceived += len;

	// Do we have the whole thing?
	//////////////////////////////
	if(connection->fileBytesReceived == connection->totalSize || connection->connectionClosed)
		connection->completed = GHTTPTrue;

	// Handle based on type.
	////////////////////////
	if(connection->type == GHIGET)
	{
		// Put this in the buffer.
		//////////////////////////
		if(!ghiAppendDataToBuffer(&connection->getFileBuffer, data, len))
			return GHTTPFalse;

		// Set the callback parameters.
		///////////////////////////////
		buffer = connection->getFileBuffer.data;
		bufferLen = connection->getFileBuffer.len;
	}
	else if(connection->type == GHISAVE)
	{
		int bytesWritten = 0;
#ifndef NOFILE
		bytesWritten = fwrite(data, 1, len, connection->saveFile);
#endif
		if(bytesWritten != len)
		{
			connection->completed = GHTTPTrue;
			connection->result = GHTTPFileWriteFailed;
			return GHTTPFalse;
		}

		// Set the callback parameters.
		///////////////////////////////
		buffer = data;
		bufferLen = len;
	}
	else if(connection->type == GHISTREAM)
	{
		// Set the callback parameters.
		///////////////////////////////
		buffer = data;
		bufferLen = len;
	}

	// Call the callback.
	/////////////////////
	ghiCallProgressCallback(connection, buffer, bufferLen);

	return GHTTPTrue;
}

// Gets the size of a chunk from a chunk header.
// Returns -1 on error.
////////////////////////////////////////////////
static int ghiParseChunkSize
(
	GHIConnection * connection
)
{
	char * header;
	int len;
	int num;
	int rcode;

	header = connection->chunkHeader;
	len = connection->chunkHeaderLen;

	assert(len);

	rcode = sscanf(header, "%x", &num);
	if(rcode != 1)
		return -1;

	return num;
}

// Appends the data to the chunk header buffer.
///////////////////////////////////////////////
static void ghiAppendToChunkHeaderBuffer
(
	GHIConnection * connection,
	char * data,
	int len
)
{
	assert(connection);
	assert(data);
	assert(len >= 0);

	// This can happen at the end of a header.
	//////////////////////////////////////////
	if(len == 0)
		return;

	// Is there room in the buffer?  If not, just
	// skip, we most likely already have the chunk size.
	////////////////////////////////////////////////////
	if(connection->chunkHeaderLen < CHUNK_HEADER_SIZE)
	{
		int numBytes;

		// How many bytes are we copying?
		/////////////////////////////////
		numBytes = min(CHUNK_HEADER_SIZE - connection->chunkHeaderLen, len);
		
		// Move the (possibly partial) header into the buffer.
		//////////////////////////////////////////////////////
		memcpy(connection->chunkHeader + connection->chunkHeaderLen, data, numBytes);

		// Cap off the buffer.
		//////////////////////
		connection->chunkHeaderLen += numBytes;
		connection->chunkHeader[connection->chunkHeaderLen] = '\0';
	}
}

// Does any neccessary processing to incoming file data
// before it gets delivered.  This includes un-chunking.
// Returns false on error.
////////////////////////////////////////////////////////
static GHTTPBool ghiProcessIncomingFileData
(
	GHIConnection * connection,
	char * data,
	int len
)
{
	assert(connection);
	assert(data);
	assert(len > 0);

	// Is this a chunked transfer?
	//////////////////////////////
	if(connection->chunkedTransfer)
	{
		// Loop while there's stuff to process.
		///////////////////////////////////////
		while(len > 0)
		{
			// Reading a header?
			////////////////////
			if(connection->chunkReadingState == CRHeader)
			{
				char * endOfHeader;

				// Have we hit the LF (as in the CRLF ending the header)?
				/////////////////////////////////////////////////////////
				endOfHeader = strchr(data, 0xA);
				if(endOfHeader)
				{
					// Append what we have to the buffer.
					/////////////////////////////////////
					ghiAppendToChunkHeaderBuffer(connection, data, endOfHeader - data);

					// Adjust data and len.
					///////////////////////
					endOfHeader++;
					len -= (endOfHeader - data);
					data = endOfHeader;

					// Read the chunk size.
					///////////////////////
					connection->chunkBytesLeft = ghiParseChunkSize(connection);
					if(connection->chunkBytesLeft == -1)
					{
						// There was an error reading the chunk size.
						/////////////////////////////////////////////
						connection->completed = GHTTPTrue;
						connection->result = GHTTPBadResponse;
						return GHTTPFalse;
					}

					// Set the chunk reading state.
					///////////////////////////////
					if(connection->chunkBytesLeft == 0)
						connection->chunkReadingState = CRFooter;
					else
						connection->chunkReadingState = CRChunk;
				}
				else
				{
					// Move it all into the buffer.
					///////////////////////////////
					ghiAppendToChunkHeaderBuffer(connection, data, len);

					// Nothing else we can do now.
					//////////////////////////////
					return GHTTPTrue;
				}
			}
			// Reading a chunk?
			///////////////////
			else if(connection->chunkReadingState == CRChunk)
			{
				int numBytes;

				// How many bytes of data are we dealing with?
				//////////////////////////////////////////////
				numBytes = min(connection->chunkBytesLeft, len);

				// Deliver the bytes.
				/////////////////////
				if(!ghiDeliverIncomingFileData(connection, data, numBytes))
					return GHTTPFalse;

				// Adjust data and len.
				///////////////////////
				data += numBytes;
				len -= numBytes;

				// Figure out how many bytes left in chunk.
				///////////////////////////////////////////
				connection->chunkBytesLeft -= numBytes;

				// Did we finish the chunk?
				///////////////////////////
				if(connection->chunkBytesLeft == 0)
					connection->chunkReadingState = CRCRLF;
			}
			// Reading a chunk footer (CRLF)?
			/////////////////////////////////
			else if(connection->chunkReadingState == CRCRLF)
			{
				char * endOfFooter;

				// Did we get an LF?
				////////////////////
				endOfFooter = strchr(data, 0xA);

				// The footer hasn't ended yet.
				///////////////////////////////
				if(!endOfFooter)
					return GHTTPTrue;

				// Adjust data and len.
				///////////////////////
				endOfFooter++;
				len -= (endOfFooter - data);
				data = endOfFooter;

				// Set up for reading the next header.
				//////////////////////////////////////
				connection->chunkHeader[0] = '\0';
				connection->chunkHeaderLen = 0;
				connection->chunkBytesLeft = 0;
				connection->chunkReadingState = CRHeader;
			}
			// Reading the footer?
			//////////////////////
			else if(connection->chunkReadingState == CRFooter)
			{
				// We're done.
				//////////////
				connection->completed = GHTTPTrue;

				return GHTTPTrue;
			}
			// Bad state!
			/////////////
			else
			{
				assert(0);
				return GHTTPFalse;
			}
		}

		return GHTTPTrue;
	}

	// Regular transfer, just deliver it.
	/////////////////////////////////////
	return ghiDeliverIncomingFileData(connection, data, len);
}

/**********************
** RECEIVING HEADERS **
**********************/
void ghiDoReceivingHeaders
(
	GHIConnection * connection
)
{
	char buffer[4096];
	int bufferLen;
	GHIRecvResult result;
	char * headers;
	char * endOfHeaders;

	// Get data.
	////////////
	bufferLen = sizeof(buffer);
	result = ghiDoReceive(connection, buffer, &bufferLen);

	// Handle error, no data, conn closed.
	//////////////////////////////////////
	if(result == GHIError)
		return;
	if((result == GHINoData) && (connection->recvBuffer.pos == connection->recvBuffer.len))
		return;

	// Only append data if we got data.
	///////////////////////////////////
	if(result == GHIRecvData)
	{
		// Add the data to the headers buffer.
		//////////////////////////////////////
		if(!ghiAppendDataToBuffer(&connection->recvBuffer, buffer, bufferLen))
			return;
	}

	// Cache a pointer to the front of the headers.
	///////////////////////////////////////////////
	headers = (connection->recvBuffer.data + connection->recvBuffer.pos);

	// Check if the headers are finished.
	/////////////////////////////////////
	endOfHeaders = strstr(headers, CRLF CRLF);
	if(!endOfHeaders)
		endOfHeaders = strstr(headers, "\xA\xA"); // some servers seem to use LFs only?! Seen in 302 redirect. (28may01/bgw)
	if(endOfHeaders)
	{
		char * fileStart;
		int fileLength;
		int headersLength;
		char * contentLength;

		// Clear off the empty line.
		////////////////////////////
		endOfHeaders += 2;
		*endOfHeaders = '\0';

		// Figure out where the file starts, and how many bytes.
		////////////////////////////////////////////////////////
		headersLength = (endOfHeaders - headers);
		fileStart = (endOfHeaders + 2);
		fileLength = (connection->recvBuffer.len - (fileStart - connection->recvBuffer.data));

		// Set the headers buffer's new length.
		///////////////////////////////////////
		connection->recvBuffer.len = (endOfHeaders - connection->recvBuffer.data);

		// Log it.
		//////////
		ghiLog(headers, headersLength);
		ghiLog("\n", 1);

		// Check for continue.
		//////////////////////
		if((connection->statusCode / 100) == 1)
		{
			if(fileLength)
			{
				// Move any data to the front of the buffer.
				////////////////////////////////////////////
				memmove(connection->recvBuffer.data, fileStart, fileLength + 1);
				connection->recvBuffer.len = fileLength;
				connection->recvBuffer.pos = 0;
			}
			else
			{
				// Reset the buffer.
				/////////////////////////
				ghiResetBuffer(&connection->recvBuffer);
			}

			// We're back to receiving status.
			//////////////////////////////////
			connection->state = GHTTPReceivingStatus;
			ghiCallProgressCallback(connection, NULL, 0);

			return;
		}

		// Check for redirection.
		/////////////////////////
		if((connection->statusCode / 100) == 3)
		{
			char * location;

			// Are we over our redirection count?
			/////////////////////////////////////
			if(connection->redirectCount > 10)
			{
				connection->completed = GHTTPTrue;
				connection->result = GHTTPFileNotFound;
				return;
			}

			// Find the new location.
			/////////////////////////
			location = strstr(headers, "Location:");
			if(location)
			{
				char * end;

				// Find the start of the URL.
				/////////////////////////////
				location += 9;
				while(isspace(*location))
					location++;

				// Find the end.
				////////////////
				for(end = location; *end && !isspace(*end) ; end++)  { };
				*end = '\0';

				// Check if this is not a full URL.
				///////////////////////////////////
				if(*location == '/')
				{
					int len;

					// Recompose the URL ourselves.
					///////////////////////////////
					len = (strlen(connection->serverAddress) + 13 + strlen(location) + 1);
					connection->redirectURL = (char *)gsimalloc(len);
					if(!connection->redirectURL)
					{
						connection->completed = GHTTPTrue;
						connection->result = GHTTPOutOfMemory;
					}
					sprintf(connection->redirectURL, "http://%s:%d%s", connection->serverAddress, connection->serverPort, location);
				}
				else
				{
					// Set the redirect URL.
					////////////////////////
					connection->redirectURL = strdup(location);
					if(!connection->redirectURL)
					{
						connection->completed = GHTTPTrue;
						connection->result = GHTTPOutOfMemory;
					}
				}

				return;
			}
		}

		// If we know the file-length, set it.
		//////////////////////////////////////
		contentLength = strstr(headers, "Content-Length:");
		if(contentLength)
			connection->totalSize = atoi(contentLength + 16);

		// Check the chunky.
		////////////////////
		connection->chunkedTransfer = (GHTTPBool)(strstr(headers, "Transfer-Encoding: chunked") != NULL);
		if(connection->chunkedTransfer)
		{
			connection->chunkHeader[0] = '\0';
			connection->chunkHeaderLen = 0;
			connection->chunkBytesLeft = 0;
			connection->chunkReadingState = CRHeader;
		}

		// If we're just getting headers, or only posting data, we're done.
		///////////////////////////////////////////////////////////////////
		if((connection->type == GHIHEAD) || (connection->type == GHIPOST))
		{
			connection->completed = GHTTPTrue;
			return;
		}

		// We're receiving file data now.
		/////////////////////////////////
		connection->state = GHTTPReceivingFile;

		// Is this an empty file?
		/////////////////////////
		if(contentLength && !connection->totalSize)
		{
			connection->completed = GHTTPTrue;
			return;
		}

		// If any of the body has arrived, handle it.
		/////////////////////////////////////////////
		if(fileLength > 0)
			ghiProcessIncomingFileData(connection, fileStart, fileLength);
	}
	else if(result == GHIConnClosed)
	{
		// The conn was closed, and we didn't finish the headers - bad.
		///////////////////////////////////////////////////////////////
		connection->completed = GHTTPTrue;
		connection->result = GHTTPBadResponse;
		connection->socketError = GOAGetLastError(connection->socket);
	}
}

/*******************
** RECEIVING FILE **
*******************/
void ghiDoReceivingFile
(
	GHIConnection * connection
)
{
	char buffer[8192];
	int bufferLen;
	GHIRecvResult result;

	while(!connection->completed)
	{
		// Get data.
		////////////
		bufferLen = sizeof(buffer);
		result = ghiDoReceive(connection, buffer, &bufferLen);

		// Handle error, no data, conn closed.
		//////////////////////////////////////
		if(result == GHIError)
			return;
		if(result == GHINoData)
			return;
		if(result == GHIConnClosed)
		{
			// The file is done (hopefully).
			////////////////////////////////
			connection->completed = GHTTPTrue;
			return;
		}

		// Process the data.
		////////////////////
		if(!ghiProcessIncomingFileData(connection, buffer, bufferLen))
			return;
	}
}
