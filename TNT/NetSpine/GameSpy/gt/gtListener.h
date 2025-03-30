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

#ifndef _GTLISTENER_H_
#define _GTLISTENER_H_

#include "gtMain.h"

// Create a new listener.
/////////////////////////
GTListener gtiNewListener
(
	GTBool allocBuffer
);

// Free a listener.
///////////////////
void gtiFreeListener
(
	GTListener listener
);

// Stop all listeners.
//////////////////////
void gtiStopAllListeners
(
	GTBool callCallbacks
);

// Stops the listener.
//////////////////////
void gtiStopListener
(
	GTListener listener
);

// Start the listener listening.
////////////////////////////////
GTBool gtiStartListener
(
	GTListener listener,
	unsigned int IP,
	unsigned short port
);

// Think for all listeners.
///////////////////////////
void gtiListenersThink
(
	void
);

// Checks if a listener is in the linked list.
//////////////////////////////////////////////
GTBool gtiIsValidListener
(
	GTListener listener
);

// Checks for a listener on this UDP port.
//////////////////////////////////////////
GTListener gtiFindListenerByUDPPort
(
	unsigned short port
);

#ifdef _DEBUG
// Get debugging info on listeners.
///////////////////////////////////
void gtiListenersDebugInfo
(
	void (* callback)(const char * output)
);
#endif

#endif