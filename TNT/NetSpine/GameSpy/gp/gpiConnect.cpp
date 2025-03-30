/*
gpiConnect.c
GameSpy Presence SDK 
Dan "Mr. Pants" Schoenblum

Copyright 1999-2001 GameSpy Industries, Inc

18002 Skypark Circle
Irvine, California 92614
949.798.4200 (Tel)
949.798.4299 (Fax)
devsupport@gamespy.com

***********************************************************************
Please see the GameSpy Presence SDK documentation for more information
**********************************************************************/

//INCLUDES
//////////
#include <stdio.h>
#include <stdlib.h>
#include "gpi.h"
#include <string.h>


//DEFINES
/////////
// Connection Manager Address.
//////////////////////////////
#define GPI_CONNECTION_MANAGER_NAME    "gpcm.gamespy.com"
#define GPI_CONNECTION_MANAGER_PORT    29900

// Random String stuff.
///////////////////////
#define RANDSTRING      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
//this is off by one
//#define RANDOMCHAR()    (RANDSTRING[(rand() * sizeof(RANDSTRING)) / (RAND_MAX + 1)])
#define RANDOMCHAR()    (RANDSTRING[rand() % (sizeof(RANDSTRING) - 1)])

//GLOBALS
/////////
char GPConnectionManagerHostname[64] = GPI_CONNECTION_MANAGER_NAME;

//FUNCTIONS
///////////
static void randomString(
  char * buffer,
  int numChars
)
{
	int i;

	for(i = 0 ; i < numChars ; i++)
		buffer[i] = RANDOMCHAR();
	buffer[i] = '\0';
}

static GPResult
gpiStartConnect(
  GPConnection * connection,
  GPIOperation * operation
)
{
	struct sockaddr_in address;
	int rcode;
	int len;
	GPIConnection * iconnection = (GPIConnection*)*connection;
	struct hostent * host;

	if(!iconnection->firewall)
	{
		// Create the peer listening socket.
		////////////////////////////////////
		iconnection->peerSocket = socket(AF_INET, SOCK_STREAM, 0);
		if(iconnection->peerSocket == INVALID_SOCKET)
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_NETWORK, "There was an error creating a socket.");

		// Make it non-blocking.
		////////////////////////
		rcode = SetSockBlocking(iconnection->peerSocket,0);
		if (rcode == 0)
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_NETWORK, "There was an error making a socket non-blocking.");
		// Bind the socket.
		///////////////////
		memset(&address, 0, sizeof(address));
		address.sin_family = AF_INET;
		rcode = bind(iconnection->peerSocket, (struct sockaddr *)&address, sizeof(struct sockaddr_in));
		if(rcode == SOCKET_ERROR)
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_NETWORK, "There was an error binding a socket.");

		// Start listening on the socket.
		/////////////////////////////////
		rcode = listen(iconnection->peerSocket, SOMAXCONN);
		if(rcode == SOCKET_ERROR)
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_NETWORK, "There was an error listening on a socket.");

		// Get the socket's port.
		/////////////////////////
		len = sizeof(struct sockaddr_in);
		rcode = getsockname(iconnection->peerSocket, (struct sockaddr *)&address, (unsigned int*)&len);
		if(rcode == SOCKET_ERROR)
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_NETWORK, "There was an error getting a socket's addres.");
		iconnection->peerPort = address.sin_port;
	}
	else
	{
		// No local port.
		/////////////////
		iconnection->peerSocket = INVALID_SOCKET;
		iconnection->peerPort = 0;
	}

	// Create the cm socket.
	////////////////////////
	iconnection->cmSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(iconnection->cmSocket == INVALID_SOCKET)
		CallbackFatalError(connection, GP_NETWORK_ERROR, GP_NETWORK, "There was an error creating a socket.");

	// Make it non-blocking.
	////////////////////////
	rcode = SetSockBlocking(iconnection->cmSocket,0);
	if(rcode == 0)
		CallbackFatalError(connection, GP_NETWORK_ERROR, GP_NETWORK, "There was an error making a socket non-blocking.");
/*
	// Bind the socket.
	///////////////////
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	rcode = bind(iconnection->cmSocket, (struct sockaddr *)&address, sizeof(struct sockaddr_in));
	if(rcode == SOCKET_ERROR)
		CallbackFatalError(connection, GP_NETWORK_ERROR, GP_NETWORK, "There was an error binding a socket.");
*/
	// Get the server host.
	///////////////////////
	// reducing the timeout here so we don't fail cert...this is exceptionally brittle though.
	// -- bw 2-22-04
#if defined(TARGET_PS2)
	host = sceEENetGethostbynameTO(GPConnectionManagerHostname, 1, 3);
#else
	host = gethostbyname(GPConnectionManagerHostname);
#endif
	if(host == NULL)
		CallbackFatalError(connection, GP_NETWORK_ERROR, GP_NETWORK, "Could not resolve connection mananger host name.");

	// Connect the socket.
	//////////////////////
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = *(unsigned int *)host->h_addr_list[0];
	assert(address.sin_addr.s_addr != 0);
	address.sin_port = htons(GPI_CONNECTION_MANAGER_PORT);
	rcode = connect(iconnection->cmSocket, (struct sockaddr *)&address, sizeof(struct sockaddr_in));
	if(rcode == SOCKET_ERROR)
	{
		int error = GOAGetLastError(iconnection->cmSocket);
		if((error != WSAEWOULDBLOCK) && (error != WSAEINPROGRESS))
		{
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_NETWORK, "There was an error connecting a socket.");
		}
	}

	// We're waiting for the connect to complete.
	/////////////////////////////////////////////
	operation->state = GPI_CONNECTING;
	iconnection->connectState = GPI_CONNECTING;

	return GP_NO_ERROR;
}

GPResult
gpiConnect(
  GPConnection * connection,
  const char nick[GP_NICK_LEN],
  const char email[GP_EMAIL_LEN],
  const char password[GP_PASSWORD_LEN],
  GPEnum firewall,
  GPIBool newuser,
  GPEnum blocking,
  GPCallback callback,
  void * param
)
{
	GPIConnectData * data;
	GPIOperation * operation;
	GPIConnection * iconnection = (GPIConnection*)*connection;
	GPResult result;

	// Reset if this connection was already used.
	/////////////////////////////////////////////
	if(iconnection->connectState == GPI_DISCONNECTED)
		CHECK_RESULT(gpiReset(connection));

	// Error check.
	///////////////
	if(iconnection->connectState != GPI_NOT_CONNECTED)
		Error(connection, GP_PARAMETER_ERROR, "Invalid connection.");
	if((nick == NULL) || (nick[0] == '\0'))
		Error(connection, GP_PARAMETER_ERROR, "Invalid nick.");
	if((email == NULL) || (email[0] == '\0'))
		Error(connection, GP_PARAMETER_ERROR, "Invalid email.");
	if((password == NULL) || (password[0] == '\0'))
		Error(connection, GP_PARAMETER_ERROR, "Invalid password.");

	// Get the firewall setting.
	////////////////////////////
	switch(firewall)
	{
	case GP_FIREWALL:
		iconnection->firewall = GPITrue;
		break;
	case GP_NO_FIREWALL:
		iconnection->firewall = GPIFalse;
		break;
	default:
		Error(connection, GP_PARAMETER_ERROR, "Invalid firewall.");
	}

	// Get the nick, email, and password.
	/////////////////////////////////////
	strzcpy(iconnection->nick, nick, GP_NICK_LEN);
	strzcpy(iconnection->email, email, GP_EMAIL_LEN);
	strzcpy(iconnection->password, password, GP_PASSWORD_LEN);

	// Lowercase the email.
	///////////////////////
	_strlwr(iconnection->email);

	// Create a connect operation data struct.
	//////////////////////////////////////////
	data = (GPIConnectData *)gsimalloc(sizeof(GPIConnectData));
	if(data == NULL)
		Error(connection, GP_MEMORY_ERROR, "Out of memory.");

	// Check for new user.
	//////////////////////
	data->newuser = newuser;

	// Add the operation to the list.
	/////////////////////////////////
	CHECK_RESULT(gpiAddOperation(connection, GPI_CONNECT, data, &operation, blocking, callback, param));

	// Start it.
	////////////
	result = gpiStartConnect(connection, operation);
	if(result != GP_NO_ERROR)
	{
		operation->result = result;
		gpiFailedOpCallback(connection, operation);
		gpiDisconnect(connection, GPIFalse);
		return result;
	}

	// Process it if blocking.
	//////////////////////////
	if(operation->blocking)
		CHECK_RESULT(gpiProcess(connection, operation->id));

	return GP_NO_ERROR;
}

static GPResult
gpiSendLogin(
  GPConnection * connection,
  GPIConnectData * data
)
{
	char buffer[512];
	char response[33];
	GPIConnection * iconnection = (GPIConnection*)*connection;
	GPIProfile * profile;

	// Construct the user challenge.
	////////////////////////////////
	randomString(data->userChallenge, sizeof(data->userChallenge) - 1);

	// Hash the password.
	/////////////////////
	MD5Digest((unsigned char*)iconnection->password, strlen(iconnection->password), data->passwordHash);

	// Construct the response.
	//////////////////////////
	sprintf(buffer, "%s%s%s@%s%s%s%s",
		data->passwordHash,
		"                                                ",
		iconnection->nick,
		iconnection->email,
		data->userChallenge,
		data->serverChallenge,
		data->passwordHash);
	MD5Digest((unsigned char *)buffer, strlen(buffer), response);

	// Check for an existing profile.
	/////////////////////////////////
	if(iconnection->infoCaching)
	{
		gpiFindProfileByUser(connection, iconnection->nick, iconnection->email, &profile);
		if(profile != NULL)
		{
			// Get the userid and profileid.
			////////////////////////////////
			iconnection->userid = profile->userId;
			iconnection->profileid = profile->profileId;
		}
	}

	// Construct the outgoing message.
	//////////////////////////////////
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\login\\");
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\challenge\\");
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, data->userChallenge);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\user\\");
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, iconnection->nick);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "@");
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, iconnection->email);
	if(iconnection->userid != 0)
	{
		gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\userid\\");
		gpiAppendIntToBuffer(connection, &iconnection->outputBuffer, iconnection->userid);
	}
	if(iconnection->profileid != 0)
	{
		gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\profileid\\");
		gpiAppendIntToBuffer(connection, &iconnection->outputBuffer, iconnection->profileid);
	}
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\response\\");
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, response);
	if(iconnection->firewall == (GPIBool)GP_FIREWALL)
		gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\firewall\\1");
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\port\\");
	gpiAppendIntToBuffer(connection, &iconnection->outputBuffer, (short)ntohs((unsigned short )iconnection->peerPort));
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\productid\\");
	gpiAppendIntToBuffer(connection, &iconnection->outputBuffer, iconnection->productID);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\id\\1");
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\final\\");

	return GP_NO_ERROR;
}

static GPResult
gpiSendNewuser(
  GPConnection * connection
)
{
	GPIConnection * iconnection = (GPIConnection*)*connection;

	// Construct the outgoing message.
	//////////////////////////////////
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\newuser\\");
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\email\\");
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, iconnection->email);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\nick\\");
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, iconnection->nick);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\password\\");
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, iconnection->password);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\productid\\");
	gpiAppendIntToBuffer(connection, &iconnection->outputBuffer, iconnection->productID);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\id\\1");
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\final\\");

	return GP_NO_ERROR;
}

GPResult
gpiProcessConnect(
  GPConnection * connection,
  GPIOperation * operation,
  const char * input
)
{
	char buffer[512];
	char check[33];
	GPIConnectData * data;
	GPIConnection * iconnection = (GPIConnection*)*connection;
	GPICallback callback;
	GPIProfile * profile;

	// Check for an error.
	//////////////////////
	if(gpiCheckForError(connection, input, GPIFalse))
	{
		// Is this a deleted profile?
		/////////////////////////////
		if((iconnection->errorCode == GP_LOGIN_PROFILE_DELETED) && iconnection->profileid)
		{
			// Remove this profile object.
			//////////////////////////////
			gpiRemoveProfileByID(connection, iconnection->profileid);

			// Clear the error.
			///////////////////
			iconnection->errorCode = GP_GENERAL;
			iconnection->errorString[0] = '\0';
			iconnection->fatalError = GPIFalse;

			// If we have the profileid/userid cached, lose them.
			/////////////////////////////////////////////////////
			iconnection->userid = 0;
			iconnection->profileid = 0;

			// Retry the connect.
			/////////////////////
			iconnection->retryConnect = GPITrue;

			return GP_NO_ERROR;
		}
		else
		{
			// Check for creating an existing profile.
			//////////////////////////////////////////
			if(iconnection->errorCode == GP_NEWUSER_BAD_NICK)
			{
				// Store the pid.
				/////////////////
				if(gpiValueForKey(input, "\\pid\\", buffer, sizeof(buffer)))
					iconnection->profileid = atoi(buffer);
			}
			
			// Call the callbacks.
			//////////////////////
			if(strstr(input, "\\fatal\\") != NULL)
			{
				CallbackFatalError(connection, GP_SERVER_ERROR, iconnection->errorCode, iconnection->errorString);
			}
			else
			{
				CallbackError(connection, GP_SERVER_ERROR, iconnection->errorCode, iconnection->errorString);
			}
		}
		return GP_SERVER_ERROR;
	}

	// Get a pointer to the data.
	/////////////////////////////
	data = (GPIConnectData*)operation->data;

	switch(operation->state)
	{
	case GPI_CONNECTING:
		// This should be \lc\1.
		////////////////////////
		if(strncmp(input, "\\lc\\1", 5) != 0)
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Unexpected data was received from the server.");

		// Get the server challenge.
		////////////////////////////
		if(!gpiValueForKey(input, "\\challenge\\", data->serverChallenge, sizeof(data->serverChallenge)))
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Unexpected data was received from the server.");

		// Check if this is a new user.
		///////////////////////////////
		if(data->newuser)
		{
			// Send a new user message.
			///////////////////////////
			CHECK_RESULT(gpiSendNewuser(connection));

			// Update the operation's state.
			////////////////////////////////
			operation->state = GPI_REQUESTING;
		}
		else
		{
			// Send a login message.
			////////////////////////
			CHECK_RESULT(gpiSendLogin(connection, data));

			// Update the operation's state.
			////////////////////////////////
			operation->state = GPI_LOGIN;
		}

		break;

	case GPI_REQUESTING:
		// This should be \nur\.
		////////////////////////
		if(strncmp(input, "\\nur\\", 5) != 0)
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Unexpected data was received from the server.");

		// Get the userid.
		//////////////////
		if(!gpiValueForKey(input, "\\userid\\", buffer, sizeof(buffer)))
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Unexepected data was received from the server.");
		iconnection->userid = atoi(buffer);

		// Get the profileid.
		/////////////////////
		if(!gpiValueForKey(input, "\\profileid\\", buffer, sizeof(buffer)))
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Unexepected data was received from the server.");
		iconnection->profileid = atoi(buffer);

		// Send a login request.
		////////////////////////
		CHECK_RESULT(gpiSendLogin(connection, data));

		// Update the operation's state.
		////////////////////////////////
		operation->state = GPI_LOGIN;

		break;
		
	case GPI_LOGIN:
		// This should be \lc\2.
		////////////////////////
		if(strncmp(input, "\\lc\\2", 5) != 0)
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Unexpected data was received from the server.");

		// Get the sesskey.
		///////////////////
		if(!gpiValueForKey(input, "\\sesskey\\", buffer, sizeof(buffer)))
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Unexepected data was received from the server.");
		iconnection->sessKey = atoi(buffer);

		// Get the userid.
		//////////////////
		if(!gpiValueForKey(input, "\\userid\\", buffer, sizeof(buffer)))
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Unexepected data was received from the server.");
		iconnection->userid = atoi(buffer);

		// Get the profileid.
		/////////////////////
		if(!gpiValueForKey(input, "\\profileid\\", buffer, sizeof(buffer)))
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Unexepected data was received from the server.");
		iconnection->profileid = atoi(buffer);

		// Construct the check.
		///////////////////////
		sprintf(buffer, "%s%s%s@%s%s%s%s",
			data->passwordHash,
			"                                                ",
			iconnection->nick,
			iconnection->email,
			data->serverChallenge,
			data->userChallenge,
			data->passwordHash);
		MD5Digest((unsigned char *)buffer, strlen(buffer), check);

		// Get the proof.
		/////////////////
		if(!gpiValueForKey(input, "\\proof\\", buffer, sizeof(buffer)))
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Unexepected data was received from the server.");

		// Check the server authentication.
		///////////////////////////////////
		if(memcmp(check, buffer, 32) != 0)
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_LOGIN_SERVER_AUTH_FAILED, "Could not authenticate server.");

		// Add the local profile to the list.
		/////////////////////////////////////
		if(iconnection->infoCaching)
		{
			profile = gpiProfileListAdd(connection, iconnection->profileid);
			profile->profileId = iconnection->profileid;
			profile->userId = iconnection->userid;
		}

		// Set the connect state.
		/////////////////////////
		iconnection->connectState = GPI_CONNECTED;

		// Call the connect-response callback.
		//////////////////////////////////////
		callback = operation->callback;
		if(callback.callback != NULL)
		{
			GPConnectResponseArg * arg;
			arg = (GPConnectResponseArg *)gsimalloc(sizeof(GPConnectResponseArg));
			if(arg == NULL)
				Error(connection, GP_MEMORY_ERROR, "Out of memory.");

			arg->profile = (GPProfile)iconnection->profileid;
			arg->result = GP_NO_ERROR;
			
			CHECK_RESULT(gpiAddCallback(connection, callback, arg, operation, 0));
		}

		// This operation is complete.
		//////////////////////////////
		gpiRemoveOperation(connection, operation);

		// Get the local profile's info.
		////////////////////////////////
#if 0
		gpiAddOperation(connection, GPI_GET_INFO, NULL, &operation, GP_NON_BLOCKING, NULL, NULL);
		gpiSendGetInfo(connection, iconnection->profileid, operation->id);
#endif

		break;
		
	default:
		break;
	}
	
	return GP_NO_ERROR;
}

GPResult
gpiCheckConnect(
  GPConnection * connection
)
{
	GPIConnection * iconnection = (GPIConnection*)*connection;
	int state;
	
	// Check if the connection is completed.
	////////////////////////////////////////
	CHECK_RESULT(gpiCheckSocketConnect(connection, iconnection->cmSocket, &state));
	
	// Check for a failed attempt.
	//////////////////////////////
	if(state == GPI_DISCONNECTED)
		CallbackFatalError(connection, GP_SERVER_ERROR, GP_LOGIN_CONNECTION_FAILED, "The server has refused the connection.");

	// Check if not finished connecting.
	////////////////////////////////////
	if(state == GPI_NOT_CONNECTED)
		return GP_NO_ERROR;
	
	// We're now negotiating the connection.
	////////////////////////////////////////
	assert(state == GPI_CONNECTED);
	iconnection->connectState = GPI_NEGOTIATING;

	return GP_NO_ERROR;
}

static GPIBool
gpiDisconnectCleanupProfile(
  GPConnection * connection,
  GPIProfile * profile,
  void * data
)
{
	
	if(profile->buddyStatus)
	{
		freeclear(profile->buddyStatus->statusString);
		freeclear(profile->buddyStatus->locationString);
		freeclear(profile->buddyStatus);
	}
	freeclear(profile->authSig);
	freeclear(profile->peerSig);
	profile->requestCount = 0;

	if(gpiCanFreeProfile(profile))
	{
		gpiRemoveProfile(connection, profile);
		return GPIFalse;
	}

	return GPITrue;
}

void
gpiDisconnect(
  GPConnection * connection,
  GPIBool tellServer
)
{
	GPIConnection * iconnection = (GPIConnection*)*connection;
	GPIPeer * peer;
	GPIPeer * delPeer;
	GPIBool connClosed;

	// Check if we're already disconnected.
	// PANTS|05.15.00
	///////////////////////////////////////
	if(iconnection->connectState == GPI_DISCONNECTED)
		return;

	// Skip most of this stuff if we never actually connected.
	// PANTS|05.16.00
	//////////////////////////////////////////////////////////
	if(iconnection->connectState != GPI_NOT_CONNECTED)
	{
		// Are we connected?
		////////////////////
		if(tellServer && (iconnection->connectState == GPI_CONNECTED))
		{
			// Send the disconnect.
			///////////////////////
			gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\logout\\\\sesskey\\");
			gpiAppendIntToBuffer(connection, &iconnection->outputBuffer, iconnection->sessKey);
			gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\final\\");
		}

		// Always flush remaining messages.
		// PANTS|05.16.00
		///////////////////////////////////
		gpiSendFromBuffer(connection, iconnection->cmSocket, &iconnection->outputBuffer, &connClosed, GPITrue, "CM");

		// Cleanup the connection.
		//////////////////////////
		if(iconnection->cmSocket != INVALID_SOCKET)
		{
			shutdown(iconnection->cmSocket, 2);
			closesocket(iconnection->cmSocket);
			iconnection->cmSocket = INVALID_SOCKET;
		}
		if(iconnection->peerSocket != INVALID_SOCKET)
		{
			shutdown(iconnection->peerSocket, 2);
			closesocket(iconnection->peerSocket);
			iconnection->peerSocket = INVALID_SOCKET;
		}

		// We're disconnected.
		//////////////////////
		iconnection->connectState = GPI_DISCONNECTED;

		// Don't keep the userid/profileid.
		///////////////////////////////////
		iconnection->userid = 0;
		iconnection->profileid = 0;
	}
	
	// freeclear all the memory.
	///////////////////////
	freeclear(iconnection->socketBuffer.buffer);
	freeclear(iconnection->inputBuffer);
	freeclear(iconnection->outputBuffer.buffer);
	freeclear(iconnection->updateproBuffer.buffer);
	freeclear(iconnection->updateuiBuffer.buffer);
	while(iconnection->operationList != NULL)
		gpiRemoveOperation(connection, iconnection->operationList);
	iconnection->operationList = NULL;
	for(peer = iconnection->peerList ; peer != NULL ; )
	{
		delPeer = peer;
		peer = peer->pnext;
		gpiDestroyPeer(connection, delPeer);
	}
	iconnection->peerList = NULL;

	// Cleanup buddies.
	// This is not optimal - because we can't continue the mapping
	// after freeing a profile, we need to start it all over again.
	///////////////////////////////////////////////////////////////
	while(!gpiProfileMap(connection, gpiDisconnectCleanupProfile, NULL))  { };
}
