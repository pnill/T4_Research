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

#include "gtCallback.h"
#include "gtConnection.h"
#include "gtListener.h"

/*************************
** CONNECTION CALLBACKS **
*************************/

GTBool gtiConnectedCallback
(
	GTConnection connection,
	GTConnectResult result,
	GTByte * message,
	int len
)
{
	GTConnection callbackConnection;

	assert(connection);
	if(!connection)
		return GTFalse;

	if(!connection->callbacks.connected)
		return GTTrue;

	connection->callbackLevel++;

	// If this is blocking, and it failed,
	// don't give them a connection object.
	///////////////////////////////////////
	if(connection->blockingConnect && (result != GTAccepted))
		callbackConnection = NULL;
	else
		callbackConnection = connection;
		
	connection->callbacks.connected(callbackConnection, result, message, len);

	connection->callbackLevel--;

	return (connection->state != GTIClosed);
}

GTBool gtiReceivedCallback
(
	GTConnection connection,
	GTByte * message,
	int len,
	GTBool reliable
)
{
	assert(connection);
	if(!connection)
		return GTFalse;

	if(!connection->callbacks.received)
		return GTTrue;

	// If the length is 0, message should be NULL.
	//////////////////////////////////////////////
	if(!len)
		message = NULL;

	connection->callbackLevel++;

	connection->callbacks.received(connection, message, len, reliable);

	connection->callbackLevel--;

	return (connection->state != GTIClosed);
}

GTBool gtiClosedCallback
(
	GTConnection connection,
	GTCloseReason reason
)
{
	assert(connection);
	if(!connection)
		return GTFalse;

	if(!connection->callbacks.closed)
		return GTTrue;

	connection->callbackLevel++;

	connection->callbacks.closed(connection, reason);

	connection->callbackLevel--;

	return (connection->state != GTIClosed);
}

/***********************
** LISTENER CALLBACKS **
***********************/

GTBool gtiConnectAttemptCallback
(
	GTListener listener,
	GTConnection connection,
	unsigned int ip,
	unsigned short port,
	GTByte * message,
	int len
)
{
	assert(listener);
	if(!listener)
		return GTFalse;

	if(!listener->callbacks.connectAttempt)
		return GTTrue;

	listener->callbackLevel++;

	listener->callbacks.connectAttempt(listener,
		connection,
		ip,
		port,
		message,
		len);

	listener->callbackLevel--;

	return !listener->stopped;
}

GTBool gtiStoppedCallback
(
	GTListener listener,
	GTStopReason reason
)
{
	assert(listener);
	if(!listener)
		return GTFalse;

	if(!listener->callbacks.stopped)
		return GTTrue;

	listener->callbackLevel++;

	listener->callbacks.stopped(listener, reason);

	listener->callbackLevel--;

	return !listener->stopped;
}

/*********************
** FILTER CALLBACKS **
*********************/

GTBool gtiSendFilterCallback
(
	GTConnection connection,
	int filterID,
	const GTByte * message,
	int len,
	GTBool reliable
)
{
	gtSendFilterCallback * callback;

	assert(connection);
	if(!connection)
		return GTFalse;

	callback = (gtSendFilterCallback *)ArrayNth(connection->sendFilters, filterID);
	if(!callback)
		return GTTrue;

	connection->callbackLevel++;

	(*callback)(connection, filterID, message, len, reliable);

	connection->callbackLevel--;

	return (connection->state != GTIClosed);
}

GTBool gtiReceiveFilterCallback
(
	GTConnection connection,
	int filterID,
	GTByte * message,
	int len,
	GTBool reliable
)
{
	gtReceiveFilterCallback * callback;

	assert(connection);
	if(!connection)
		return GTFalse;

	callback = (gtReceiveFilterCallback *)ArrayNth(connection->receiveFilters, filterID);
	if(!callback)
		return GTTrue;

	connection->callbackLevel++;

	(*callback)(connection, filterID, message, len, reliable);

	connection->callbackLevel--;

	return (connection->state != GTIClosed);
}
