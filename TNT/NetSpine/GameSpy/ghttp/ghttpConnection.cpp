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

#include "ghttpConnection.h"
#include "ghttpCommon.h"

// Initial size and increment amount for the connections array.
///////////////////////////////////////////////////////////////
#define CONNECTIONS_CHUNK_LEN      4

// An array of pointers to GHIConnection objects.
// A GHTTPRequest is an index into this array.
/////////////////////////////////////////////////
static GHIConnection ** ghiConnections;
static int ghiConnectionsLen;
static int ghiNumConnections;
static int ghiNextUniqueID;

// Finds a gsifree slot in the ghiConnections array.
// If there are no gsifree slots, the array size will be increased.
////////////////////////////////////////////////////////////////
static int ghiFindFreeSlot
(
	void
)
{
	int i;
	GHIConnection ** tempPtr;
	int oldLen;
	int newLen;

	// Look for an open slot.
	/////////////////////////
	for(i = 0 ; i < ghiConnectionsLen ; i++)
	{
		if(!ghiConnections[i]->inUse)
			return i;
	}

	assert(ghiNumConnections == ghiConnectionsLen);

	// Nothing found, resize the array.
	///////////////////////////////////
	oldLen = ghiConnectionsLen;
	newLen = (ghiConnectionsLen + CONNECTIONS_CHUNK_LEN);
	tempPtr = (GHIConnection **)gsirealloc(ghiConnections, sizeof(GHIConnection *) * newLen);
	if(!tempPtr)
		return -1;
	ghiConnections = tempPtr;

	// Create the new connection objects.
	/////////////////////////////////////
	for(i = oldLen ; i < newLen ; i++)
	{
		ghiConnections[i] = (GHIConnection *)gsimalloc(sizeof(GHIConnection));
		if(!ghiConnections[i])
		{
			for(i-- ; i >= oldLen ; i--)
				gsifree(ghiConnections[i]);
			return -1;
		}
		ghiConnections[i]->inUse = GHTTPFalse;
	}

	// Update the length.
	/////////////////////
	ghiConnectionsLen = newLen;

	return oldLen;
}

GHIConnection * ghiNewConnection
(
	void
)
{
	int slot;
	GHIConnection * connection;
	GHTTPBool bResult;

	ghiLock();

	// Find a gsifree slot.
	////////////////////
	slot = ghiFindFreeSlot();
	if(slot == -1)
	{
		ghiUnlock();
		return NULL;
	}

	// Get a pointer to the object.
	///////////////////////////////
	connection = ghiConnections[slot];

	// Init the object.
	///////////////////
	memset(connection, 0, sizeof(GHIConnection));
	connection->inUse = GHTTPTrue;
	connection->request = (GHTTPRequest)slot;
	connection->uniqueID = ghiNextUniqueID++;
	connection->type = GHIGET;
	connection->state = GHTTPHostLookup;
	connection->URL = NULL;
	connection->serverAddress = NULL;
	connection->serverIP = INADDR_ANY;
	connection->serverPort = (unsigned short)0;
	connection->requestPath = NULL;
	connection->sendHeaders = NULL;
	connection->saveFile = NULL;
	connection->blocking = GHTTPFalse;
	connection->result = GHTTPSuccess;
	connection->progressCallback = NULL;
	connection->completedCallback = NULL;
	connection->callbackParam = NULL;
	connection->socket = INVALID_SOCKET;
	connection->socketError = 0;
	connection->userBufferSupplied = GHTTPFalse;
	connection->statusMajorVersion = 0;
	connection->statusMinorVersion = 0;
	connection->statusCode = 0;
	connection->statusStringIndex = 0;
	connection->completed = GHTTPFalse;
	connection->fileBytesReceived = 0;
	connection->totalSize = -1;
	connection->redirectURL = NULL;
	connection->redirectCount = 0;
	connection->chunkedTransfer = GHTTPFalse;
	connection->processing = GHTTPFalse;
	connection->throttle = GHTTPFalse;
	connection->lastThrottleRecv = 0;
	connection->post = NULL;

	bResult = ghiInitBuffer(connection, &connection->sendBuffer, SEND_BUFFER_INITIAL_SIZE, SEND_BUFFER_INCREMENT_SIZE);
	if(bResult)
		bResult = ghiInitBuffer(connection, &connection->recvBuffer, RECV_BUFFER_INITIAL_SIZE, RECV_BUFFER_INCREMENT_SIZE);

	if(!bResult)
	{
		ghiFreeConnection(connection);
		ghiUnlock();
		return NULL;
	}

	// One more connection.
	///////////////////////
	ghiNumConnections++;

	ghiUnlock();

	return connection;
}

GHTTPBool ghiFreeConnection
(
	GHIConnection * connection
)
{
	/*
	assert(connection);
	assert(connection->request >= 0);
	assert(connection->request < ghiConnectionsLen);
	assert(connection->inUse);
	*/

	// Check args.
	//////////////
	if(!connection)
		return GHTTPFalse;
	if(!connection->inUse)
		return GHTTPFalse;
	if(connection->request < 0)
		return GHTTPFalse;
	if(connection->request >= ghiConnectionsLen)
		return GHTTPFalse;

	ghiLock();

	// Free data.
	/////////////
	gsifree(connection->URL);
	gsifree(connection->serverAddress);
	gsifree(connection->requestPath);
	gsifree(connection->sendHeaders);
	gsifree(connection->redirectURL);
#ifndef NOFILE
	if(connection->saveFile)
		fclose(connection->saveFile);
#endif
	if(connection->socket != INVALID_SOCKET)
	{
		shutdown(connection->socket, 2);
		closesocket(connection->socket);
	}
	ghiFreeBuffer(&connection->sendBuffer);
	ghiFreeBuffer(&connection->recvBuffer);
	ghiFreeBuffer(&connection->getFileBuffer);
	if(connection->postingState.states)
		ghiPostCleanupState(connection);

	// Check for an auto-free post.
	///////////////////////////////
	if(connection->post && ghiIsPostAutoFree(connection->post))
	{
		ghiFreePost(connection->post);
		connection->post = NULL;
	}

	// Free the slot.
	/////////////////
	connection->inUse = GHTTPFalse;

	// One less connection.
	///////////////////////
	ghiNumConnections--;

	ghiUnlock();

	return GHTTPTrue;
}

GHIConnection * ghiRequestToConnection
(
	GHTTPRequest request
)
{
	GHIConnection * connection;

	assert(request >= 0);
	assert(request < ghiConnectionsLen);

	ghiLock();

	// Check args.
	//////////////
	if((request < 0) || (request >= ghiConnectionsLen))
	{
		ghiUnlock();
		return NULL;
	}

	connection = ghiConnections[request];

	// Check for not in use.
	////////////////////////
	if(!connection->inUse)
		connection = NULL;

	ghiUnlock();

	return connection;
}

void ghiEnumConnections
(
	GHTTPBool (* callback)(GHIConnection *)
)
{
	int i;

	// Check for no connections.
	////////////////////////////
	if(ghiNumConnections <= 0)
		return;

	ghiLock();
	for(i = 0 ; i < ghiConnectionsLen ; i++)
		if(ghiConnections[i]->inUse)
			callback(ghiConnections[i]);
	ghiUnlock();
}

void ghiRedirectConnection
(
	GHIConnection * connection
)
{
	assert(connection);
	assert(connection->redirectURL);

	// Reset state.
	///////////////
	connection->state = GHTTPHostLookup;

	// New URL.
	///////////
	gsifree(connection->URL);
	connection->URL = connection->redirectURL;
	connection->redirectURL = NULL;

	// Reset stuff parsed from the URL.
	///////////////////////////////////
	gsifree(connection->serverAddress);
	connection->serverAddress = NULL;
	connection->serverIP = 0;
	connection->serverPort = 0;
	gsifree(connection->requestPath);
	connection->requestPath = NULL;

	// Close the socket.
	////////////////////
	shutdown(connection->socket, 2);
	closesocket(connection->socket);
	connection->socket = INVALID_SOCKET;

	// Reset buffers.
	/////////////////
	ghiResetBuffer(&connection->sendBuffer);
	ghiResetBuffer(&connection->recvBuffer);

	// Reset status.
	////////////////
	connection->statusMajorVersion = 0;
	connection->statusMinorVersion = 0;
	connection->statusCode = 0;
	connection->statusStringIndex = 0;

	// The connection isn't closed.
	///////////////////////////////
	connection->connectionClosed = GHTTPFalse;

	// One more redirect.
	/////////////////////
	connection->redirectCount++;
}

void ghiCleanupConnections
(
	void
)
{
	int i;

	if(!ghiConnections)
		return;

	// Cleanup all running connections.
	///////////////////////////////////
	ghiEnumConnections(ghiFreeConnection);

	// Cleanup the connection states.
	/////////////////////////////////
	for(i = 0 ; i < ghiConnectionsLen ; i++)
		gsifree(ghiConnections[i]);
	gsifree(ghiConnections);
	ghiConnections = NULL;
	ghiConnectionsLen = 0;
	ghiNumConnections = 0;
}
