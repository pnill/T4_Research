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

#ifndef _GTCONNECTION_H_
#define _GTCONNECTION_H_

#include "gtMain.h"

// Create a new connection.
///////////////////////////
GTConnection gtiNewConnection
(
	GTConnectResult * result  // Only set on failure
);

// Free a connection.
// _Always_ returns GTFalse.
////////////////////////////
GTBool gtiFreeConnection
(
	GTConnection connection
);

// Closes a connection, free it later.
// _Always_ returns GTFalse.
//////////////////////////////////////
GTBool gtiCloseConnection
(
	GTConnection connection
);

// Close all connections.
/////////////////////////
void gtiCloseAllConnections
(
	GTBool callCallbacks
);

// Calls the closed callback then closes the connection.
// This _always_ returns GTFalse to signal the connection closed.
/////////////////////////////////////////////////////////////////
GTBool gtiConnectionClosed
(
	GTConnection connection,
	GTCloseReason reason
);

// Start connecting to a remote system.
///////////////////////////////////////
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
	GTConnectResult * result  // Only set on failure.
);

// Thinks for all connections.
// Returns GTFalse if the given connection is closed.
/////////////////////////////////////////////////////
GTBool gtiConnectionsThink
(
	GTConnection checkConnection
);

// Checks that this connection is in the connection list.
/////////////////////////////////////////////////////////
GTBool gtiIsValidConnection
(
	GTConnection connection
);

// Binds the UDP socket to the address.
///////////////////////////////////////
GTBool gtiBindUDPSocket
(
	GTConnection connection,
	SOCKADDR_IN * address
);

// Close the UDP socket, set it to invalid.
///////////////////////////////////////////
void gtiCloseConnectionUDPSocket
(
	GTConnection connection
);

// Handles incoming UDP for a connection.
// Returns GTFalse if the connection was closed.
////////////////////////////////////////////////
GTBool gtiHandleReceivedUDP
(
	GTConnection connection,
	int len,
	GTIControlCode code
);

// Get a pointer to the first connection in the list.
// Returns NULL if no connections.
//////////////////////////////////
GTConnection gtiGetHeadConnection
(
	void
);

#ifdef _DEBUG
// Get debugging info on connections.
/////////////////////////////////////
void gtiConnectionsDebugInfo
(
	void (* callback)(const char * output)
);
#endif

#endif