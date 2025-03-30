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

#ifndef _GTCALLBACK_H_
#define _GTCALLBACK_H_

#include "gtMain.h"

// Callbacks return GTFalse if the connection/listener
// was closed/stopped during the callback, else GTTrue.
///////////////////////////////////////////////////////

GTBool gtiConnectedCallback
(
	GTConnection connection,
	GTConnectResult result,
	GTByte * message,
	int len
);

GTBool gtiReceivedCallback
(
	GTConnection connection,
	GTByte * message,
	int len,
	GTBool reliable
);

GTBool gtiClosedCallback
(
	GTConnection connection,
	GTCloseReason reason
);

GTBool gtiConnectAttemptCallback
(
	GTListener listener,
	GTConnection connection,
	unsigned int ip,
	unsigned short port,
	GTByte * message,
	int len
);

GTBool gtiStoppedCallback
(
	GTListener listener,
	GTStopReason reason
);

GTBool gtiSendFilterCallback
(
	GTConnection connection,
	int filterID,
	const GTByte * message,
	int len,
	GTBool reliable
);

GTBool gtiReceiveFilterCallback
(
	GTConnection connection,
	int filterID,
	GTByte * message,
	int len,
	GTBool reliable
);

#endif