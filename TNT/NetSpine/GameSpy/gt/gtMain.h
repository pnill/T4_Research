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

#ifndef _GTMAIN_H_
#define _GTMAIN_H_

#include "gt.h"
#if defined(applec) || defined(THINK_C) || defined(__MWERKS__) && !defined(__KATANA__)  && !defined(__mips64)
	#include "::nonport.h"
	#include "::darray.h"
#else
	#include "../nonport.h"
	#include "../darray.h"
#endif

/*
GameSpy Transport Protocol

*** Overview of the Negotation Process for the TCP (Reliable) Connection ***
C = Client (connecting)
S = Server (accepting)
C->S  Connect
S->C  ServerChallenge + UDP port
C->S  ServerChallengeResponse + ClientChallenge + UDP port + InitialData
If Accept:
S->C  Accept + ClientChallengeResponse
Else:
S->C  Reject

*** Details of TCP Negotiation ***
C->S  Connect
S->C  \challenge\<random challenge string>\final\
C->S  \response\<challenge response>\challenge\<random challenge string>\port\<UDP port>\data\<initial data>
If Accept:
S->C  \accept\1\response\<challenge response>\port\<UDP port>\final\
Else:
S->C  \accept\0\reason\<reason data>

*** Other Messages ***
TCP:
Keepalive
UDP Ping Ack
Unreliable Message

UDP:
Keepalive
Ping

*** TCP Message Header ***
2 bytes:
[15]    = Control Message Flag
[14-0]  = Message Size

During negotiations, all messages must be control messages.
For all control messages, the 1st byte following the
header signals what type of control message follows.
After negotiations, application messages (control
message flag is 0) are allowed.
0 = Negotiation Message (message follows)
1 = Keepalive (no data follows)
2 = UDP Ping Ack (no data follows)
3 = Unreliable Message (message follows)

*** UDP Messages ***
UDP control messages start with a magic number.  If a UDP message
does not start with the magic number, it is an application message.
The 1st byte following the magic number signals what type of control
message follows.
0 = Keepalive (no data follows)
1 = Ping (no data follows)
*/

// If not commented out, will keep track of stats.
//////////////////////////////////////////////////
//#define STATS
#include "gtAuth.h"
#define GTI_MAX_ADDRESS             128
#define GTI_UDP_KEEPALIVE_TIME      (120 * 1000)
#define GTI_TCP_KEEPALIVE_TIME      (120 * 1000)

// UDP control messages start with this magic string.
/////////////////////////////////////////////////////
#define GTI_UDP_MAGIC_STRING        "\xC2\x6D\xE5\x43\x46\x96\x8B\x2F\x2B\x33\x17\xA7"
#define GTI_UDP_MAGIC_STRING_LEN    12

typedef enum
{
	// User/Application messages.
	/////////////////////////////
	GTIAppMessage    = -1,

	// TCP control messages.
	////////////////////////
	GTINegotiation   = 0,
	GTITCPKeepalive  = 1,
	GTIUDPPingAck    = 2,
	GTITCPUnreliable = 3,

	// UDP control messages.
	////////////////////////
	GTIUDPKeepalive  = 0,
	GTIUDPPing       = 1
} GTIControlCode;

typedef enum
{
	GTIHostLookup,            // Looking up the remote system.
	GTIConnecting,            // Connecting to the remote system.
	GTINegotiating,           // Negotiating the connection.
	GTIReceivedChallenge,     // The challenge has been received.
	GTIAwaitingAcceptReject,  // Awaiting an accept or reject from the app.
	GTIConnected,             // Connected.
	GTIClosed                 // This connection should be freed when possible.
} GTIConnectionState;

typedef struct GTICircularBuffer
{
	GTByte * buffer;          // The buffer's bytes.
	int size;                 // Number of bytes in buffer.
	int len;                  // Length of actual data in buffer.
	int start;                // The start of the data in the buffer.
} GTICircularBuffer;

typedef struct GTIConnection
{
	struct GTIConnection * pnext;       // The next connection object.z

	GTListener listener;                // The listener that spawned this connection, or NULL.

	GTIConnectionState state;           // The current state.
	GTBool initiated;                   // True if this connection was initiated locally.
	GTBool freeAtAcceptReject;          // If true, the connection can't be freed until accept/reject.

	SOCKET UDPSocket;                   // The UDP (unreliable) socket.
	SOCKET TCPSocket;                   // The TCP (reliable) socket.
	GTBool TCPRecvClose;                // Set to true when remotely closed.

	char remoteAddress[GTI_MAX_ADDRESS];// The remote address to connect to.
	unsigned int remoteIP;              // The IP of the remote system (after resolved).
	unsigned short remotePort;          // The port of the remote system (after resolved).
	unsigned short remoteUDPPort;       // The UDP port on the remote system.
	
	GTConnectionCallbacks callbacks;    // The application callbacks.
	int callbackLevel;                  // 0 means not in a callback, >1 means that many callbacks deep.

	void * userData;                    // User-data that can be get/set by the app.

	unsigned long lastUDPSend;          // The last time UDP data was sent.  Used for keepalives.
	unsigned long lastTCPSend;          // The last time TCP data was sent.  Used for keepalives.

	unsigned long startConnectTime;     // When the connect attempt was started.
	int timeout;                        // How long to wait for the connect to finish.
	GTBool blockingConnect;             // Is the connect blocking?
	int connectAttempts;                // Number of times connect() was tried.

	GTBool canSendUDP;                  // True	once a UDP ping has been acknowleged.
	unsigned long lastUDPPing;          // The last time a UDP ping was sent.
	int UDPPingCount;                   // The number of UDP pings that were sent.
	int UDPPingTime;                    // The round-trip time for the last UDP ping.

	char response[GTI_RESPONSE_LEN];    // The response we're sending/expecting during negotiation.
	                                    // If (initiated && state==GTIReceivedChallenge) then
	                                    // this is the expected resonse.
	                                    // If (!initiated && state==GTINegotiating) then this
	                                    // is the expected response.
	                                    // If (!intiated && state==GTIAwaitingAcceptReject) then
	                                    // this is the response we send on gtAccept or gtReject.

	GTICircularBuffer outgoingBuffer;   // The buffer for outgoing TCP data.
	GTICircularBuffer incomingBuffer;   // The buffer for incoming TCP data.
	int messageLen;                     // The length of the message from the last TCP header read.

	GTByte * UDPBuffer;                 // Incoming UDP is received into this buffer.

	GTByte * initialData;               // The initial data to send, can be NULL.
	int initialDataLen;                 // The length of the inital data, can be 0.

	unsigned long startCloseTime;       // The time we marked the connection as closed.

	DArray sendFilters;                 // The list of send filters.
	DArray receiveFilters;              // The list of receive filters.

#ifdef STATS
	int incomingBytesTCP;               // Total bytes sent on TCP.
	int outgoingBytesTCP;               // Total bytes received on TCP.
	int incomingBytesUDP;               // Total bytes sent on UDP.
	int outgoingBytesUDP;               // Total bytes received on UDP.
#endif
} GTIConnection;

typedef struct GTIListener
{
	struct GTIListener * pnext;         // The next listener.

	GTBool stopped;                     // True if the listener was stopped.

	int numConnections;                 // The number of active connections spawned from this listener.

	SOCKET socket;                      // The listener socket.

	SOCKET UDPSocket;                   // The listener's UDP socket.
	unsigned short localUDPPort;        // The UDP socket's port.

	GTByte * UDPBuffer;                 // Incoming UDP is received into this buffer.

	GTListenerCallbacks callbacks;      // The application callbacks.
	int callbackLevel;                  // 0 means not in a callback, >1 means that many callbacks deep.

	void * userData;                    // User-data that can be get/set by the app.
} GTIListener;

// For visual assist, so it doesn't bug out.
////////////////////////////////////////////
#if 0
void (* callback)(const char * output);
const char * output;
#endif

#endif