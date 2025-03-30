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

#ifndef _GTSEND_H_
#define _GTSEND_H_

#include "gtMain.h"

// Send a message on the connection.
////////////////////////////////////
void gtiSendMessage
(
	GTConnection connection,
	const GTByte * message,
	int len,
	GTBool reliable,
	GTIControlCode code
);

// Lower-level than gtiSendMessage.
// First tries to send any buffered data.
// If data is still buffered, buffers this data.
// If nothing buffered, sends on TCP.
// Anything unsent then gets buffered,
////////////////////////////////////////////////
void gtiSendTCPData
(
	GTConnection connection,
	const GTByte * data,
	int len
);

// Used to send a TCP header.
/////////////////////////////
void gtiSendTCPHeader
(
	GTConnection connection,
	int len,  // Length of the message following the header.
	GTIControlCode code
);

// Sends any data that's been buffered.
///////////////////////////////////////
void gtiSendBufferedData
(
	GTConnection connection
);

#endif