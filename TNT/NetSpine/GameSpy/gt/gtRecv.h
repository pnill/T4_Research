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

#ifndef _GTRECV_H_
#define _GTRECV_H_

#include "gtMain.h"

// Possible results for a receive.
//////////////////////////////////
typedef enum
{
	GTIRecvMessage,  // A message was received.
	GTIRecvNone,     // There were no messages
	GTIRecvClosed,   // The connection was closed.
	GTIRecvError     // There was an error.
} GTIRecvResult;

// Receive TCP data on a connection.
////////////////////////////////////
GTIRecvResult gtiRecvTCPMessageHeader
(
	GTConnection connection,
	int * len,
	GTIControlCode * code
);

typedef GTBool (* GTITCPMessageHandler)
(
	GTConnection connection,
	GTByte * message,
	int len,
	void * userData
);

GTBool gtiRecvTCPMessage
(
	GTConnection connection,
	GTITCPMessageHandler handler,
	GTBool * result,
	void * userData
);

// Receive a UDP message.
/////////////////////////
GTIRecvResult gtiReceiveUDPMessage
(
	SOCKET socket,
	GTByte * buffer,
	int * len,
	GTIControlCode * code,
	SOCKADDR_IN * address
);

GTBool gtiCanRecvOnSocket
(
	SOCKET socket
);

#endif