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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "gtMain.h"
#include "gtListener.h"
#include "gtConnection.h"
#include "gtSend.h"
#include "gtCallback.h"

// Set to true when SocketStartUp has been called.
//////////////////////////////////////////////////
static GTBool Started;

void gtStartup
(
	void
)
{
	SocketStartUp();
	Started = GTTrue;
}

// If connection is not NULL, GTFalse will
// be returned if the connection is closed.
///////////////////////////////////////////
static GTBool gtiThink
(
	GTConnection connection
)
{
	gtiListenersThink();

	return gtiConnectionsThink(connection);
}

void gtThink
(
	void
)
{
	gtiThink(NULL);
}

GTConnection gtConnect
(
	const char * localAddress,
	const char * remoteAddress,
	const GTByte * message,
	int len,
	int timeout,
	GTBool blocking,
	GTConnectionCallbacks * callbacks
)
{
	GTConnection connection;
	GTBool notClosed;
	GTConnectResult result;

	// Make sure sockets are initialized.
	/////////////////////////////////////
	if(!Started)
		gtStartup();

	// Create the object.
	/////////////////////
	connection = gtiNewConnection(&result);
	if(!connection)
	{
		// Call the connected callback.
		// Call it directly since there's no connection object.
		///////////////////////////////////////////////////////
		if(callbacks && callbacks->connected)
			callbacks->connected(NULL, result, NULL, 0);
		return NULL;
	}

	// Start the connection.
	////////////////////////
	if(!gtiStartConnecting(connection, localAddress, remoteAddress, message, len, timeout, blocking, callbacks, &result))
	{
		// Call the connected callback.
		// Call it directly since the connection object isn't very valid.
		/////////////////////////////////////////////////////////////////
		if(callbacks && callbacks->connected)
			callbacks->connected(NULL, result, NULL, 0);
		gtiFreeConnection(connection);
		return NULL;
	}

	// Done if not blocking.
	////////////////////////
	if(!blocking)
		return connection;

	// Block until the connection is closed or connected.
	/////////////////////////////////////////////////////
	while((notClosed = gtiThink(connection)) && (connection->state != GTIConnected))
		msleep(1);

	// If not closed, we're connected.
	//////////////////////////////////
	if(notClosed)
		return connection;

	// The connection was closed.
	/////////////////////////////
	return NULL;
}

void gtCloseConnection
(
	GTConnection connection
)
{
	assert(connection);
	if(!connection)
		return;

	// If this connection is already closed, nothing to do.
	///////////////////////////////////////////////////////
	if(connection->state == GTIClosed)
		return;

	// Closes the connection then calls the callback.
	/////////////////////////////////////////////////
	gtiConnectionClosed(connection, GTLocalClose);
}

void gtSend
(
	GTConnection connection,
	const GTByte * message,
	int len,
	GTBool reliable
)
{
	assert(connection);
	if(!connection)
		return;

	// We can only send if we're connected.
	///////////////////////////////////////
	if(connection->state != GTIConnected)
		return;

	// Check if we have any send filters.
	/////////////////////////////////////
	if(connection->sendFilters)
	{
		// Fix up the parameters.
		/////////////////////////
		if(!message || !len)
		{
			message = NULL;
			len = 0;
		}
		else if(len == -1)
			len = (strlen(message) + 1);

		// Filter the send.
		///////////////////
		gtiSendFilterCallback(connection, 0, message, len, reliable);
	}
	else
	{
		// Do the actual send.
		//////////////////////
		gtiSendMessage(connection, message, len, reliable, GTIAppMessage);
	}
}

GTListener gtListen
(
	const char * localAddress,
	GTListenerCallbacks * callbacks
)
{
	GTListener listener;
	GTListener oldListener;
	unsigned int IP;
	unsigned short port;

	// Make sure sockets are initialized.
	/////////////////////////////////////
	if(!Started)
		gtStartup();

	// Get the IP and port.
	///////////////////////
	if(!gtStringToAddress(localAddress, &IP, &port))
		return NULL;

	// Is there a listener already listening on this address?
	/////////////////////////////////////////////////////////
	oldListener = gtiFindListenerByUDPPort(port);
	if(oldListener)
	{
		// If its not stopped, we'll let nature take its course.
		////////////////////////////////////////////////////////
		if(!oldListener->stopped)
			oldListener = NULL;
	}

	// Create the object.
	/////////////////////
	listener = gtiNewListener(oldListener == NULL);
	if(!listener)
		return NULL;

	// Copy the callbacks.
	//////////////////////
	if(callbacks)
		listener->callbacks = *callbacks;

	// Are we taking over?
	//////////////////////
	if(oldListener)
	{
		// Copy stuff.
		//////////////
		listener->UDPSocket = oldListener->UDPSocket;
		listener->UDPBuffer = oldListener->UDPBuffer;
		listener->numConnections = oldListener->numConnections;
		listener->localUDPPort = oldListener->localUDPPort;
	}

	// Start the listener.
	//////////////////////
	if(!gtiStartListener(listener, IP, port))
	{
		gtiFreeListener(listener);
		return NULL;
	}

	// Kill the old listener.
	/////////////////////////
	if(oldListener)
	{
		GTConnection connection;

		// Make sure the old listener's connections point at the new one.
		/////////////////////////////////////////////////////////////////
		for(connection = gtiGetHeadConnection() ; connection ; connection = connection->pnext)
			if(connection->listener == oldListener)
				connection->listener = listener;

		// Cleanup the old listener.
		////////////////////////////
		oldListener->UDPSocket = INVALID_SOCKET;
		oldListener->UDPBuffer = NULL;
		oldListener->numConnections = 0;
	}

	return listener;
}

void gtStopListener
(
	GTListener listener
)
{
	assert(listener);
	if(!listener)
		return;

#if 1
	// Stop the listener before calling the callback.
	////////////////////////////////////////////////////
	shutdown(listener->socket, 2);
	closesocket(listener->socket);
#endif

	// Call the stopped callback.
	/////////////////////////////
	gtiStoppedCallback(listener, GTStopped);

	// Stop the listener.
	/////////////////////
	if(!listener->stopped)
		gtiStopListener(listener);
}

GTBool gtAccept
(
	GTConnection connection,
	GTConnectionCallbacks * callbacks
)
{
	int len;
	char strPort[6];
	int nPortLen;

	assert(connection);
	if(!connection)
		return GTFalse;

	// This shouldn't ever happen.
	//////////////////////////////
	assert(gtiIsValidConnection(connection));
	if(!gtiIsValidConnection(connection))
		return GTFalse;

	// Check if this has already been closed.
	/////////////////////////////////////////
	if(connection->freeAtAcceptReject)
	{
		// Set it to false so i can be freed.
		/////////////////////////////////////
		connection->freeAtAcceptReject = GTFalse;

		return GTFalse;
	}

	// Check the state.
	///////////////////
	if(connection->state != GTIAwaitingAcceptReject)
		return GTFalse;

	// Convert the port to a string.
	////////////////////////////////
	sprintf(strPort, "%d", connection->listener->localUDPPort);
	nPortLen = strlen(strPort);

	// Send the accept.
	///////////////////
	len = (19 + GTI_RESPONSE_LEN + 6 + nPortLen + 8);
	gtiSendTCPHeader(connection, len, GTINegotiation);
	gtiSendTCPData(connection, "\\accept\\1\\response\\", 19);
	gtiSendTCPData(connection, connection->response, GTI_RESPONSE_LEN);
	gtiSendTCPData(connection, "\\port\\", 6);
	gtiSendTCPData(connection, strPort, nPortLen);
	gtiSendTCPData(connection, "\\final\\", 8);

	// Set the connected state.
	///////////////////////////
	connection->state = GTIConnected;

	// Set the callbacks.
	/////////////////////
	connection->callbacks = *callbacks;

	return GTTrue;
}

void gtReject
(
	GTConnection connection,
	const GTByte * message,
	int len
)
{
	static const char * strReject = "\\accept\\0\\reason\\";
	static int nStrLen;

	assert(connection);
	if(!connection)
		return;

	// This shouldn't ever happen.
	//////////////////////////////
	assert(gtiIsValidConnection(connection));
	if(!gtiIsValidConnection(connection))
		return;

	// Check if this has already been closed.
	/////////////////////////////////////////
	if(connection->freeAtAcceptReject)
	{
		// Set it to false so i can be freed.
		/////////////////////////////////////
		connection->freeAtAcceptReject = GTFalse;

		return;
	}

	// Check the state.
	///////////////////
	if(connection->state != GTIAwaitingAcceptReject)
		return;

	// Check parameters.
	////////////////////
	if(!message || !len)
	{
		message = NULL;
		len = 0;
	}
	if(len == -1)
		len = (strlen(message) + 1);

	// Get the length of the reject string if we don't have it.
	///////////////////////////////////////////////////////////
	if(!nStrLen)
		nStrLen = strlen(strReject);

	// First send the header.
	/////////////////////////
	gtiSendTCPHeader(connection, nStrLen + len, GTINegotiation);

	// Send the reject.
	///////////////////
	gtiSendTCPData(connection, strReject, nStrLen);

	// Send the reason.
	///////////////////
	if(len)
		gtiSendTCPData(connection, message, len);

	// We need to do this here, because CloseConnection checks
	// for AwaitingAcceptReject to see if it shouldd delay the gsifree
	// until the accept/reject.
	///////////////////////////////////////////////////////////////
	connection->state = GTIClosed;

	// Close the connection.
	////////////////////////
	gtiCloseConnection(connection);
}

void gtCleanup
(
	GTBool callCallbacks
)
{
	// Close all connections.
	/////////////////////////
	gtiCloseAllConnections(callCallbacks);

	// Close all listeners.
	///////////////////////
	gtiStopAllListeners(callCallbacks);
}

/*********************
** FILTER FUNCTIONS **
*********************/
static int gtiSendFiltersCompare
(
	const void * elem1,
	const void * elem2
)
{
	gtSendFilterCallback * callback1 = (gtSendFilterCallback *)elem1;
	gtSendFilterCallback * callback2 = (gtSendFilterCallback *)elem2;

	if(*callback1 == *callback2)
		return 0;
	return 1;
}

GTBool gtAddSendFilter
(
	GTConnection connection,
	gtSendFilterCallback callback
)
{
	assert(connection);
	if(!connection)
		return GTFalse;

	// Check if we have a send filters list.
	////////////////////////////////////////
	if(!connection->sendFilters)
	{
		// Make a list.
		///////////////
		connection->sendFilters = ArrayNew(
			sizeof(gtSendFilterCallback),
			2,
			NULL);

		// Check for not enough mem.
		////////////////////////////
		if(!connection->sendFilters)
			return GTFalse;
	}

	// Add this callback to the list.
	/////////////////////////////////
	ArrayAppend(connection->sendFilters, &callback);

	// Return GTTrue if it was added.
	/////////////////////////////////
	return (ArraySearch(connection->sendFilters, &callback, gtiSendFiltersCompare, 0, 0) != NOT_FOUND);
}

void gtRemoveSendFilter
(
	GTConnection connection,
	gtSendFilterCallback callback
)
{
	int index;

	assert(connection);
	if(!connection)
		return;

	// Check for no filters.
	////////////////////////
	if(!connection->sendFilters)
		return;

	// Find it.
	///////////
	index = ArraySearch(connection->sendFilters, &callback, gtiSendFiltersCompare, 0, 0);
	if(index == NOT_FOUND)
		return;

	// Remove it.
	/////////////
	ArrayRemoveAt(connection->sendFilters, index);

	// If there are no filters, remove the list.
	////////////////////////////////////////////
	if(!ArrayLength(connection->sendFilters))
	{
		ArrayFree(connection->sendFilters);
		connection->sendFilters = NULL;
	}
}

void gtRemoveAllSendFilters
(
	GTConnection connection
)
{
	assert(connection);
	if(!connection)
		return;

	// Check for no list.
	/////////////////////
	if(!connection->sendFilters)
		return;

	// Remove the list.
	///////////////////
	ArrayFree(connection->sendFilters);
	connection->sendFilters = NULL;
}

void gtFilteredSend
(
	GTConnection connection,
	int filterID,
	const GTByte * message,
	int len,
	GTBool reliable
)
{
	int num;

	assert(connection);
	if(!connection)
		return;

	// Make sure we're connected.
	/////////////////////////////
	if(connection->state != GTIConnected)
		return;

	// Check for no filters.
	////////////////////////
	if(!connection->sendFilters)
		return;

	// Get the number of filters.
	/////////////////////////////
	num = ArrayLength(connection->sendFilters);

	// Check if its a valid ID.
	///////////////////////////
	if(filterID < 0)
		return;
	if(filterID >= num)
		return;

	// Is it the last one?
	//////////////////////
	if(filterID == (num - 1))
	{
		// Do the actual send.
		//////////////////////
		gtiSendMessage(connection, message, len, reliable, GTIAppMessage);
	}
	else
	{
		// Fix up the parameters.
		/////////////////////////
		if(!message || !len)
		{
			message = NULL;
			len = 0;
		}
		else if(len == -1)
			len = (strlen(message) + 1);

		// Filter it.
		/////////////
		gtiSendFilterCallback(connection, ++filterID, message, len, reliable);
	}
}

static int gtiReceiveFiltersCompare
(
	const void * elem1,
	const void * elem2
)
{
	gtReceiveFilterCallback * callback1 = (gtReceiveFilterCallback *)elem1;
	gtReceiveFilterCallback * callback2 = (gtReceiveFilterCallback *)elem2;

	if(*callback1 == *callback2)
		return 0;
	return 1;
}

GTBool gtAddReceiveFilter
(
	GTConnection connection,
	gtReceiveFilterCallback callback
)
{
	assert(connection);
	if(!connection)
		return GTFalse;

	// Check if we have a receive filters list.
	///////////////////////////////////////////
	if(!connection->receiveFilters)
	{
		// Make a list.
		///////////////
		connection->receiveFilters = ArrayNew(
			sizeof(gtReceiveFilterCallback),
			2,
			NULL);

		// Check for not enough mem.
		////////////////////////////
		if(!connection->receiveFilters)
			return GTFalse;
	}

	// Add this callback to the list.
	/////////////////////////////////
	ArrayAppend(connection->receiveFilters, &callback);

	// Return GTTrue if it was added.
	/////////////////////////////////
	return (ArraySearch(connection->receiveFilters, &callback, gtiReceiveFiltersCompare, 0, 0) != NOT_FOUND);
}

void gtRemoveReceiveFilter
(
	GTConnection connection,
	gtReceiveFilterCallback callback
)
{
	int index;

	assert(connection);
	if(!connection)
		return;

	// Check for no filters.
	////////////////////////
	if(!connection->receiveFilters)
		return;

	// Find it.
	///////////
	index = ArraySearch(connection->receiveFilters, &callback, gtiReceiveFiltersCompare, 0, 0);
	if(index == NOT_FOUND)
		return;

	// Remove it.
	/////////////
	ArrayRemoveAt(connection->receiveFilters, index);

	// If there are no filters, remove the list.
	////////////////////////////////////////////
	if(!ArrayLength(connection->receiveFilters))
	{
		ArrayFree(connection->receiveFilters);
		connection->receiveFilters = NULL;
	}
}

void gtRemoveAllReceiveFilters
(
	GTConnection connection
)
{
	assert(connection);
	if(!connection)
		return;

	// Check for no list.
	/////////////////////
	if(!connection->receiveFilters)
		return;

	// Remove the list.
	///////////////////
	ArrayFree(connection->receiveFilters);
	connection->receiveFilters = NULL;
}

void gtFilteredReceive
(
	GTConnection connection,
	int filterID,
	GTByte * message,
	int len,
	GTBool reliable
)
{
	int num;

	assert(connection);
	if(!connection)
		return;

	// Make sure we're connected.
	/////////////////////////////
	if(connection->state != GTIConnected)
		return;

	// Check for no filters.
	////////////////////////
	if(!connection->receiveFilters)
		return;

	// Get the number of filters.
	/////////////////////////////
	num = ArrayLength(connection->receiveFilters);

	// Check if its a valid ID.
	///////////////////////////
	if(filterID < 0)
		return;
	if(filterID >= num)
		return;

	// Is it the last one?
	//////////////////////
	if(filterID == (num - 1))
	{
		// Call the callback.
		/////////////////////
		gtiReceivedCallback(connection, message, len, reliable);
	}
	else
	{
		// Filter it.
		/////////////
		gtiReceiveFilterCallback(connection, ++filterID, message, len, reliable);
	}
}

/********************
** OTHER FUNCTIONS **
********************/

GTBool gtIsReliableChannelEstablished
(
	GTConnection connection
)
{
	assert(connection);
	if(!connection)
		return GTFalse;

	return (connection->state == GTIConnected);
}

GTBool gtIsUnreliableChannelEstablished
(
	GTConnection connection
)
{
	assert(connection);
	if(!connection)
		return GTFalse;

	// Make sure we're connected.
	/////////////////////////////
	if(connection->state != GTIConnected)
		return GTFalse;

	return connection->canSendUDP;
}

GTBool gtIsValidConnection
(
	GTConnection connection
)
{
	if(!connection)
		return GTFalse;

	return (gtiIsValidConnection(connection) && (connection->state != GTIClosed));
}

GTBool gtIsValidListener
(
	GTListener listener
)
{
	if(!listener)
		return GTFalse;

	return (gtiIsValidListener(listener) && !listener->stopped);
}

int gtGetBufferedDataSize
(
	GTConnection connection
)
{
	assert(connection);
	if(!connection)
		return 0;

	return connection->outgoingBuffer.len;
}

unsigned int gtGetRemoteIP
(
	GTConnection connection
)
{
	assert(connection);
	if(!connection)
		return 0;

	if(connection->state != GTIConnected)
		return 0;

	return connection->remoteIP;
}

unsigned short gtGetListenerPort
(
	GTListener listener
)
{
	SOCKADDR_IN address;
	int len;

	assert(listener);
	if(!listener)
		return 0;

	len = sizeof(SOCKADDR_IN);
	if(getsockname(listener->socket, (SOCKADDR *)&address, (unsigned int*)&len) == SOCKET_ERROR)
		return 0;

	return ntohs(address.sin_port);
}

int gtGetConnectionPing
(
	GTConnection connection
)
{
	assert(connection);
	if(!connection)
		return -1;

	if(!connection->canSendUDP)
		return -1;

	return connection->UDPPingTime;
}

void * gtGetConnectionData
(
	GTConnection connection
)
{
	assert(connection);
	if(!connection)
		return NULL;

	return connection->userData;
}

void gtSetConnectionData
(
	GTConnection connection,
	void * userData
)
{
	assert(connection);
	if(!connection)
		return;

	connection->userData = userData;
}

void * gtGetListenerData
(
	GTListener listener
)
{
	assert(listener);
	if(!listener)
		return NULL;

	return listener->userData;
}

void gtSetListenerData
(
	GTListener listener,
	void * userData
)
{
	assert(listener);
	if(!listener)
		return;

	listener->userData = userData;
}

#ifdef _DEBUG
void gtDebugInfo
(
	void (* callback)(const char * output)
)
{
	gtiConnectionsDebugInfo(callback);
	callback("");
	gtiListenersDebugInfo(callback);
}
#endif