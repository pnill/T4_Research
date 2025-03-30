/*
gpiSearch.c
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
#include <stdlib.h>
#include <string.h>
#include "gpi.h"

//DEFINES
/////////
// Search Manager Address.
//////////////////////////
#define GPI_SEARCH_MANAGER_NAME        "gpsp.gamespy.com"
#define GPI_SEARCH_MANAGER_PORT        29901


//GLOBALS
/////////
char GPSearchManagerHostname[64] = GPI_SEARCH_MANAGER_NAME;

//FUNCTIONS
///////////
static GPResult
gpiStartProfileSearch(
  GPConnection * connection,
  GPIOperation * operation
)
{
	GPISearchData * data = (GPISearchData*)operation->data;
	int rcode;
	struct sockaddr_in address;
	struct hostent * host;

	// Initialize the buffer.
	/////////////////////////
	data->inputBuffer.size = 4096;
	data->inputBuffer.buffer = (char *)gsimalloc(data->inputBuffer.size + 1);
	if(data->inputBuffer.buffer == NULL)
		Error(connection, GP_MEMORY_ERROR, "Out of memory.");

	// Create the socket.
	/////////////////////
	data->sock = socket(AF_INET, SOCK_STREAM, 0);
	if(data->sock == INVALID_SOCKET)
		CallbackFatalError(connection, GP_NETWORK_ERROR, GP_NETWORK, "There was an error creating a socket.");

	// Make it non-blocking.
	////////////////////////
	rcode = SetSockBlocking(data->sock,0);
	if(rcode == 0)
		CallbackFatalError(connection, GP_NETWORK_ERROR, GP_NETWORK, "There was an error making a socket non-blocking.");

	// Bind the socket.
	///////////////////
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	rcode = bind(data->sock, (struct sockaddr *)&address, sizeof(struct sockaddr_in));
	if(rcode == SOCKET_ERROR)
		CallbackFatalError(connection, GP_NETWORK_ERROR, GP_NETWORK, "There was an error binding a socket.");
	
	// Get the server host.
	///////////////////////
	host = gethostbyname(GPSearchManagerHostname);
	if(host == NULL)
		CallbackFatalError(connection, GP_NETWORK_ERROR, GP_NETWORK, "Could not resolve search mananger host name.");

	// Connect the socket.
	//////////////////////
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = *(unsigned int *)host->h_addr_list[0];
	assert(address.sin_addr.s_addr != 0);
	address.sin_port = htons(GPI_SEARCH_MANAGER_PORT);
	rcode = connect(data->sock, (struct sockaddr *)&address, sizeof(struct sockaddr_in));
	if(rcode == SOCKET_ERROR)
	{
		int error = GOAGetLastError(data->sock);
		if((error != WSAEWOULDBLOCK) && (error != WSAEINPROGRESS))
		{
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_NETWORK, "There was an error connecting a socket.");
		}
	}

	// We're waiting for the connect to complete.
	/////////////////////////////////////////////
	operation->state = GPI_CONNECTING;

	return GP_NO_ERROR;
}

static GPResult
gpiInitSearchData(
  GPConnection * connection,
  GPISearchData ** searchData,
  int type
)
{
	GPISearchData * data;

	// Init the data.
	/////////////////
	data = (GPISearchData *)gsimalloc(sizeof(GPISearchData));
	if(data == NULL)
		Error(connection, GP_MEMORY_ERROR, "Out of memory.");
	data->type = type;
	data->sock = INVALID_SOCKET;
	data->inputBuffer.buffer = NULL;
	data->inputBuffer.len = 0;
	data->inputBuffer.pos = 0;
	data->inputBuffer.size = 0;
	data->outputBuffer.len = 0;
	data->outputBuffer.pos = 0;
	data->outputBuffer.size = 4096;
	data->outputBuffer.buffer = (char *)gsimalloc(data->outputBuffer.size + 1);
	if(data->outputBuffer.buffer == NULL)
		Error(connection, GP_MEMORY_ERROR, "Out of memory.");
	data->processing = GPIFalse;
	data->remove = GPIFalse;

	*searchData = data;

	return GP_NO_ERROR;
}

static GPResult
gpiStartSearch(
  GPConnection * connection,
  GPISearchData * data,
  GPEnum blocking,
  GPCallback callback,
  void * param
)
{
	GPIOperation * operation;
	GPIConnection * iconnection = (GPIConnection*)*connection;

	// One more search.
	///////////////////
	iconnection->numSearches++;

	// Create a new operation.
	//////////////////////////
	CHECK_RESULT(gpiAddOperation(connection, GPI_PROFILE_SEARCH, data, &operation, blocking, callback, param));

	// Start the search.
	////////////////////
	CHECK_RESULT(gpiStartProfileSearch(connection, operation));

	// Process it if blocking.
	//////////////////////////
	if(operation->blocking)
		CHECK_RESULT(gpiProcess(connection, operation->id));

	return GP_NO_ERROR;
}

GPResult
gpiProfileSearch(
  GPConnection * connection,
  const char nick[GP_NICK_LEN],
  const char email[GP_EMAIL_LEN],
  const char firstname[GP_FIRSTNAME_LEN],
  const char lastname[GP_LASTNAME_LEN],
  int icquin,
  int skip,
  GPEnum blocking,
  GPCallback callback,
  void * param
)
{
	GPISearchData * data;

	// Error check.
	///////////////
	if((nick == NULL) || (*nick == '\0'))
		if((email == NULL) || (*email == '\0'))
			if((firstname == NULL) || (*firstname == '\0'))
				if((lastname == NULL) || (*lastname == '\0'))
					if(icquin == 0)
						Error(connection, GP_PARAMETER_ERROR, "No search criteria.");

	// Init the data.
	/////////////////
	CHECK_RESULT(gpiInitSearchData(connection, &data, GPI_SEARCH_PROFILE));

	// Fill in the data.
	////////////////////
	if(nick == NULL)
		data->nick[0] = '\0';
	else
		strzcpy(data->nick, nick, GP_NICK_LEN);
	if(email == NULL)
		data->email[0] = '\0';
	else
		strzcpy(data->email, email, GP_EMAIL_LEN);
	_strlwr(data->email);
	if(firstname == NULL)
		data->firstname[0] = '\0';
	else
		strzcpy(data->firstname, firstname, GP_FIRSTNAME_LEN);
	if(lastname == NULL)
		data->lastname[0] = '\0';
	else
		strzcpy(data->lastname, lastname, GP_LASTNAME_LEN);
	data->icquin = icquin;
	if(skip < 0)
		skip = 0;
	data->skip = skip;

	// Start the search.
	////////////////////
	CHECK_RESULT(gpiStartSearch(connection, data, blocking, callback, param));

	return GP_NO_ERROR;
}

GPResult
gpiIsValidEmail(
  GPConnection * connection,
  const char email[GP_EMAIL_LEN],
  GPEnum blocking,
  GPCallback callback,
  void * param
)
{
	GPISearchData * data;

	// Error check.
	///////////////
	if((email == NULL) || (*email == '\0') || (strlen(email) >= GP_EMAIL_LEN))
		Error(connection, GP_PARAMETER_ERROR, "Invalid e-mail.");

	// Init the data.
	/////////////////
	CHECK_RESULT(gpiInitSearchData(connection, &data, GPI_SEARCH_IS_VALID));

	// Fill in the data.
	////////////////////
	strzcpy(data->email, email, GP_EMAIL_LEN);
	_strlwr(data->email);

	// Start the search.
	////////////////////
	CHECK_RESULT(gpiStartSearch(connection, data, blocking, callback, param));

	return GP_NO_ERROR;
}

GPResult
gpiGetUserNicks(
  GPConnection * connection,
  const char email[GP_EMAIL_LEN],
  const char password[GP_PASSWORD_LEN],
  GPEnum blocking,
  GPCallback callback,
  void * param
)
{
	GPISearchData * data;

	// Error check.
	///////////////
	if((email == NULL) || (*email == '\0') || (strlen(email) >= GP_EMAIL_LEN))
		Error(connection, GP_PARAMETER_ERROR, "Invalid e-mail.");
	if((password == NULL) || (strlen(password) >= GP_PASSWORD_LEN))
		Error(connection, GP_PARAMETER_ERROR, "Invalid password.");

	// Init the data.
	/////////////////
	CHECK_RESULT(gpiInitSearchData(connection, &data, GPI_SEARCH_NICKS));

	// Fill in the data.
	////////////////////
	strzcpy(data->email, email, GP_EMAIL_LEN);
	_strlwr(data->email);
	strzcpy(data->password, password, GP_PASSWORD_LEN);

	// Start the search.
	////////////////////
	CHECK_RESULT(gpiStartSearch(connection, data, blocking, callback, param));

	return GP_NO_ERROR;
}

GPResult
gpiFindPlayers(
  GPConnection * connection,
  int productID,
  GPEnum blocking,
  GPCallback callback,
  void * param
)
{
	GPISearchData * data;

	// Init the data.
	/////////////////
	CHECK_RESULT(gpiInitSearchData(connection, &data, GPI_SEARCH_PLAYERS));

	// Fill in the data.
	////////////////////
	data->productID = productID;

	// Start the search.
	////////////////////
	CHECK_RESULT(gpiStartSearch(connection, data, blocking, callback, param));

	return GP_NO_ERROR;
}

GPResult gpiCheckUser(
  GPConnection * connection,
  const char nick[GP_NICK_LEN],
  const char email[GP_EMAIL_LEN],
  const char password[GP_PASSWORD_LEN],
  GPEnum blocking,
  GPCallback callback,
  void * param
)
{
	GPISearchData * data;

	// Init the data.
	/////////////////
	CHECK_RESULT(gpiInitSearchData(connection, &data, GPI_SEARCH_CHECK));

	// Fill in the data.
	////////////////////
	strzcpy(data->email, email, GP_EMAIL_LEN);
	_strlwr(data->email);
	strzcpy(data->nick, nick, GP_NICK_LEN);
	if(password)
		strzcpy(data->password, password, GP_PASSWORD_LEN);

	// Start the search.
	////////////////////
	CHECK_RESULT(gpiStartSearch(connection, data, blocking, callback, param));

	return GP_NO_ERROR;
}

GPResult gpiNewUser(
  GPConnection * connection,
  const char nick[GP_NICK_LEN],
  const char email[GP_EMAIL_LEN],
  const char password[GP_PASSWORD_LEN],
  GPEnum blocking,
  GPCallback callback,
  void * param
)
{
	GPISearchData * data;

	// Init the data.
	/////////////////
	CHECK_RESULT(gpiInitSearchData(connection, &data, GPI_SEARCH_NEWUSER));

	// Fill in the data.
	////////////////////
	strzcpy(data->email, email, GP_EMAIL_LEN);
	strzcpy(data->nick, nick, GP_NICK_LEN);
	strzcpy(data->password, password, GP_PASSWORD_LEN);

	// Start the search.
	////////////////////
	CHECK_RESULT(gpiStartSearch(connection, data, blocking, callback, param));

	return GP_NO_ERROR;
}

GPResult gpiOthersBuddy(
  GPConnection * connection,
  GPEnum blocking,
  GPCallback callback,
  void * param
)
{
	GPISearchData * data;

	// Init the data.
	/////////////////
	CHECK_RESULT(gpiInitSearchData(connection, &data, GPI_SEARCH_OTHERS_BUDDY));

	// Start the search.
	////////////////////
	CHECK_RESULT(gpiStartSearch(connection, data, blocking, callback, param));

	return GP_NO_ERROR;
}

static GPResult
gpiProcessSearch(
  GPConnection * connection,
  GPIOperation * operation
)
{
	int state;
	GPISearchData * data;
	char key[512];
	char value[512];
	GPIBool done;
	int index;
	int oldIndex;
	GPIBool loop;
	GPIBool more;
	GPICallback callback;
	GPIConnection * iconnection = (GPIConnection*)*connection;
	int len;
	GPIBool connClosed;
	GPResult result;
	void * tempPtr;
	GPIBool doneParsingMatch;
	int rcode;
	int pid;
	GPProfileSearchMatch * match;
	
	// Get a pointer to the data.
	/////////////////////////////
	data = (GPISearchData*)operation->data;

	// Loop if blocking.
	////////////////////
	if(operation->blocking)
		loop = GPITrue;
	else
		loop = GPIFalse;

	do
	{
		// Send anything that needs to be sent.
		///////////////////////////////////////
		CHECK_RESULT(gpiSendFromBuffer(connection, data->sock, &data->outputBuffer, &connClosed, GPITrue, "SM"));

		// Is it connecting?
		////////////////////
		if(operation->state == GPI_CONNECTING)
		{
			// Check the connect state.
			///////////////////////////
			CHECK_RESULT(gpiCheckSocketConnect(connection, data->sock, &state));
			
			// Check for a failed attempt.
			//////////////////////////////
			if(state == GPI_DISCONNECTED)
				CallbackError(connection, GP_SERVER_ERROR, GP_SEARCH_CONNECTION_FAILED, "Could not connect to the search manager.");

			// Check if finished connecting.
			////////////////////////////////
			if(state == GPI_CONNECTED)
			{
				// Send a request based on type.
				////////////////////////////////
				if(data->type == GPI_SEARCH_PROFILE)
				{
					gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\search\\");
					gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\sesskey\\");
					gpiAppendIntToBuffer(connection, &data->outputBuffer, iconnection->sessKey);
					gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\profileid\\");
					gpiAppendIntToBuffer(connection, &data->outputBuffer, iconnection->profileid);
					if(data->nick[0] != '\0')
					{
						gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\nick\\");
						gpiAppendStringToBuffer(connection, &data->outputBuffer, data->nick);
					}
					if(data->email[0] != '\0')
					{
						gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\email\\");
						gpiAppendStringToBuffer(connection, &data->outputBuffer, data->email);
					}
					if(data->firstname[0] != '\0')
					{
						gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\firstname\\");
						gpiAppendStringToBuffer(connection, &data->outputBuffer, data->firstname);
					}
					if(data->lastname[0] != '\0')
					{
						gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\lastname\\");
						gpiAppendStringToBuffer(connection, &data->outputBuffer, data->lastname);
					}
					if(data->icquin != 0)
					{
						gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\icquin\\");
						gpiAppendIntToBuffer(connection, &data->outputBuffer, data->icquin);
					}
					if(data->skip > 0)
					{
						gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\skip\\");
						gpiAppendIntToBuffer(connection, &data->outputBuffer, data->skip);
					}
				}
				else if(data->type == GPI_SEARCH_IS_VALID)
				{
					gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\valid\\");
					gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\email\\");
					gpiAppendStringToBuffer(connection, &data->outputBuffer, data->email);
				}
				else if(data->type == GPI_SEARCH_NICKS)
				{
					gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\nicks\\");
					gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\email\\");
					gpiAppendStringToBuffer(connection, &data->outputBuffer, data->email);
					gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\pass\\");
					gpiAppendStringToBuffer(connection, &data->outputBuffer, data->password);
				}
				else if(data->type == GPI_SEARCH_PLAYERS)
				{
					gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\pmatch\\");
					gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\sesskey\\");
					gpiAppendIntToBuffer(connection, &data->outputBuffer, iconnection->sessKey);
					gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\profileid\\");
					gpiAppendIntToBuffer(connection, &data->outputBuffer, iconnection->profileid);
					gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\productid\\");
					gpiAppendIntToBuffer(connection, &data->outputBuffer, data->productID);
				}
				else if(data->type == GPI_SEARCH_CHECK)
				{
					gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\check\\");
					gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\nick\\");
					gpiAppendStringToBuffer(connection, &data->outputBuffer, data->nick);
					gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\email\\");
					gpiAppendStringToBuffer(connection, &data->outputBuffer, data->email);
					gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\pass\\");
					gpiAppendStringToBuffer(connection, &data->outputBuffer, data->password);
				}
				else if(data->type == GPI_SEARCH_NEWUSER)
				{
					gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\newuser\\");
					gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\nick\\");
					gpiAppendStringToBuffer(connection, &data->outputBuffer, data->nick);
					gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\email\\");
					gpiAppendStringToBuffer(connection, &data->outputBuffer, data->email);
					gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\pass\\");
					gpiAppendStringToBuffer(connection, &data->outputBuffer, data->password);
					gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\productID\\");
					gpiAppendIntToBuffer(connection, &data->outputBuffer, iconnection->productID);
				}
				else if(data->type == GPI_SEARCH_OTHERS_BUDDY)
				{
					gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\others\\");
					gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\sesskey\\");
					gpiAppendIntToBuffer(connection, &data->outputBuffer, iconnection->sessKey);
					gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\profileid\\");
					gpiAppendIntToBuffer(connection, &data->outputBuffer, iconnection->profileid);
				}
				else
				{
					assert(0);
				}

				gpiAppendStringToBuffer(connection, &data->outputBuffer, "\\final\\");

				// Update the state.
				////////////////////
				operation->state = GPI_WAITING;
			}
		}
		// Is it waiting?
		/////////////////
		else if(operation->state == GPI_WAITING)
		{
			// Read from the socket.
			////////////////////////
			result = gpiRecvToBuffer(connection, data->sock, &data->inputBuffer, &len, &connClosed, "SM");
			if(result != GP_NO_ERROR)
			{
				if(result == GP_NETWORK_ERROR)
					CallbackError(connection, GP_NETWORK_ERROR, GP_SEARCH_CONNECTION_FAILED, "There was an error reading from the server.");
				return result;
			}

			// Is this the end of the response?
			///////////////////////////////////
			if(strstr(data->inputBuffer.buffer, "\\final\\") != NULL)
			{
				// Reset the index.
				///////////////////
				index = 0;

				// This operation is finishing up.
				//////////////////////////////////
				operation->state = GPI_FINISHING;

				// Check for an error.
				//////////////////////
				if(gpiCheckForError(connection, data->inputBuffer.buffer, GPITrue))
				{
					data->remove = GPITrue;
					return GP_SERVER_ERROR;
				}

				// Process it based on type.
				////////////////////////////
				if(data->type == GPI_SEARCH_PROFILE)
				{
					GPProfileSearchResponseArg arg;
					
					// Start setting up the arg.
					////////////////////////////
					arg.result = GP_NO_ERROR;
					arg.numMatches = 0;
					arg.matches = NULL;
					arg.more = GP_DONE;

					// Parse the message.
					/////////////////////
					done = GPIFalse;
					do
					{
						// Read the next key and value.
						///////////////////////////////
						CHECK_RESULT(gpiReadKeyAndValue(connection, data->inputBuffer.buffer, &index, key, value));

						// Is the list done?
						////////////////////
						if(strcmp(key, "bsrdone") == 0)
						{
							// Check for more.
							//////////////////
							CHECK_RESULT(gpiReadKeyAndValue(connection, data->inputBuffer.buffer, &index, key, value));
							if(strcmp(key, "more") == 0)
							{
								// Make sure there are actually more.
								/////////////////////////////////////
								if(strcmp(value, "0") != 0)
									arg.more = GP_MORE;
							}

							// Done.
							////////
							done = GPITrue;
						}
						else if(strcmp(key, "bsr") == 0)
						{
							// Create a new match.
							//////////////////////
							arg.numMatches++;
							arg.matches = (GPProfileSearchMatch *)gsirealloc(arg.matches, sizeof(GPProfileSearchMatch) * arg.numMatches);
							if(arg.matches == NULL)
								Error(connection, GP_MEMORY_ERROR, "Out of memory.");
							match = &arg.matches[arg.numMatches - 1];
							memset(match, 0, sizeof(GPProfileSearchMatch));

							// Get the profile id.
							//////////////////////
							match->profile = atoi(value);

							// PANTS|05.16.00
							// Changed to be order independent, and ignore unrecognized keys.
							/////////////////////////////////////////////////////////////////

							// Read key/value pairs.
							////////////////////////
							doneParsingMatch = GPIFalse;
							do
							{
								// Read the next key/value.
								///////////////////////////
								oldIndex = index;
								CHECK_RESULT(gpiReadKeyAndValue(connection, data->inputBuffer.buffer, &index, key, value));

								// Set the field based on the key.
								//////////////////////////////////
								if(strcmp(key, "nick") == 0)
									strzcpy(match->nick, value, GP_NICK_LEN);
								else if(strcmp(key, "firstname") == 0)
									strzcpy(match->firstname, value, GP_FIRSTNAME_LEN);
								else if(strcmp(key, "lastname") == 0)
									strzcpy(match->lastname, value, GP_LASTNAME_LEN);
								else if(strcmp(key, "email") == 0)
									strzcpy(match->email, value, GP_EMAIL_LEN);
								else if((strcmp(key, "bsr") == 0) || (strcmp(key, "bsrdone") == 0))
								{
									doneParsingMatch = GPITrue;
									index = oldIndex;
								}
							}
							while(!doneParsingMatch);
						}
						else
						{
							CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Error reading from the search server.");
						}
					} while(!done);

					// Save the more state.
					///////////////////////
					more = (GPIBool)arg.more;

					// Get the callback.
					////////////////////
					callback = operation->callback;

					// Call the callback.
					/////////////////////
					if(callback.callback != NULL)
						callback.callback(connection, &arg, callback.param);

					// Start a new operation if they want more matches.
					///////////////////////////////////////////////////
					if((more == (GPIBool)GP_MORE) && (arg.more == GP_MORE))
						CHECK_RESULT(gpiProfileSearch(connection, data->nick, data->email, data->firstname, data->lastname, data->icquin, arg.numMatches + data->skip, (GPEnum)operation->blocking, operation->callback.callback, operation->callback.param));

					// We're done.
					//////////////
					freeclear(arg.matches);
				}
				else if(data->type == GPI_SEARCH_IS_VALID)
				{
					callback = operation->callback;
					if(callback.callback != NULL)
					{
						GPIsValidEmailResponseArg * arg;

						CHECK_RESULT(gpiReadKeyAndValue(connection, data->inputBuffer.buffer, &index, key, value));
						if(strcmp(key, "vr") != 0)
							CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Error reading from the search server.");

						// Setup the arg.
						/////////////////
						arg = (GPIsValidEmailResponseArg *)gsimalloc(sizeof(GPIsValidEmailResponseArg));
						if(arg == NULL)
							Error(connection, GP_MEMORY_ERROR, "Out of memory.");
						arg->result = GP_NO_ERROR;
						strcpy(arg->email, data->email);
						if(value[0] == '0')
							arg->isValid = GP_INVALID;
						else
							arg->isValid = GP_VALID;

						// Add the callback.
						////////////////////
						CHECK_RESULT(gpiAddCallback(connection, callback, arg, operation, 0));
					}
				}
				else if(data->type == GPI_SEARCH_NICKS)
				{
					callback = operation->callback;
					if(callback.callback != NULL)
					{
						GPGetUserNicksResponseArg * arg;

						// Setup the arg.
						/////////////////
						arg = (GPGetUserNicksResponseArg *)gsimalloc(sizeof(GPGetUserNicksResponseArg));
						if(arg == NULL)
							Error(connection, GP_MEMORY_ERROR, "Out of memory.");
						arg->result = GP_NO_ERROR;
						strcpy(arg->email, data->email);
						arg->numNicks = 0;
						arg->nicks = NULL;

						CHECK_RESULT(gpiReadKeyAndValue(connection, data->inputBuffer.buffer, &index, key, value));
						if(strcmp(key, "nr") != 0)
							CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Error reading from the search server.");

						// Get the nicks.
						/////////////////
						done = GPIFalse;
						do
						{
							CHECK_RESULT(gpiReadKeyAndValue(connection, data->inputBuffer.buffer, &index, key, value));
							if(strcmp(key, "nick") == 0)
							{
								// Add it.
								//////////
								tempPtr = gsirealloc(arg->nicks, sizeof(char *) * (arg->numNicks + 1));
								if(tempPtr == NULL)
									Error(connection, GP_MEMORY_ERROR, "Out of memory.");
								arg->nicks = (char **)tempPtr;
								tempPtr = gsimalloc(GP_NICK_LEN);
								if(tempPtr == NULL)
									Error(connection, GP_MEMORY_ERROR, "Out of memory.");
								arg->nicks[arg->numNicks] = (char*)tempPtr;
								strzcpy(arg->nicks[arg->numNicks], value, GP_NICK_LEN);
								arg->numNicks++;
							}
							else if(strcmp(key, "ndone") == 0)
							{
								// Done.
								////////
								done = GPITrue;
							}
							else
							{
								CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Error reading from the search server.");
							}
						}
						while(!done);

						// Do it.
						/////////
						CHECK_RESULT(gpiAddCallback(connection, callback, arg, operation, GPI_ADD_NICKS));
					}
				}
				else if(data->type == GPI_SEARCH_PLAYERS)
				{
					callback = operation->callback;
					if(callback.callback != NULL)
					{
						GPFindPlayersResponseArg * arg;
						GPFindPlayerMatch * match;
						
						// Start setting up the arg.
						////////////////////////////
						arg = (GPFindPlayersResponseArg *)gsimalloc(sizeof(GPFindPlayersResponseArg));
						if(arg == NULL)
							Error(connection, GP_MEMORY_ERROR, "Out of memory.");
						arg->productID = data->productID;
						arg->result = GP_NO_ERROR;
						arg->numMatches = 0;
						arg->matches = NULL;

						// Parse the message.
						/////////////////////
						done = GPIFalse;
						do
						{
							// Read the next key and value.
							///////////////////////////////
							CHECK_RESULT(gpiReadKeyAndValue(connection, data->inputBuffer.buffer, &index, key, value));

							// Is the list done?
							////////////////////
							if(strcmp(key, "psrdone") == 0)
							{
								// Done.
								////////
								done = GPITrue;
							}
							else if(strcmp(key, "psr") == 0)
							{
								// Create a new match.
								//////////////////////
								arg->numMatches++;
								arg->matches = (GPFindPlayerMatch *)gsirealloc(arg->matches, sizeof(GPFindPlayerMatch) * arg->numMatches);
								if(arg->matches == NULL)
									Error(connection, GP_MEMORY_ERROR, "Out of memory.");
								match = &arg->matches[arg->numMatches - 1];
								memset(match, 0, sizeof(GPFindPlayerMatch));
								match->status = GP_ONLINE;

								// Get the profile id.
								//////////////////////
								match->profile = atoi(value);

								// PANTS|05.16.00
								// Changed to be order independent, and ignore unrecognized keys.
								/////////////////////////////////////////////////////////////////

								// Read key/value pairs.
								////////////////////////
								doneParsingMatch = GPIFalse;
								do
								{
									// Read the next key/value.
									///////////////////////////
									oldIndex = index;
									CHECK_RESULT(gpiReadKeyAndValue(connection, data->inputBuffer.buffer, &index, key, value));

									// Set the field based on the key.
									//////////////////////////////////
									if(strcmp(key, "status") == 0)
										strzcpy(match->statusString, value, GP_STATUS_STRING_LEN);
									else if(strcmp(key, "nick") == 0)
										strzcpy(match->nick, value, GP_NICK_LEN);
									if(strcmp(key, "statuscode") == 0)
										match->status = (GPEnum)atoi(value);
									else if((strcmp(key, "psr") == 0) || (strcmp(key, "psrdone") == 0))
									{
										doneParsingMatch = GPITrue;
										index = oldIndex;
									}
								}
								while(!doneParsingMatch);
							}
							else
							{
								CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Error reading from the search server.");
							}
						} while(!done);

						// Do it.
						/////////
						CHECK_RESULT(gpiAddCallback(connection, callback, arg, operation, GPI_ADD_PMATCH));
					}
				}
				else if(data->type == GPI_SEARCH_CHECK)
				{
					callback = operation->callback;
					if(callback.callback != NULL)
					{
						GPCheckResponseArg * arg;

						CHECK_RESULT(gpiReadKeyAndValue(connection, data->inputBuffer.buffer, &index, key, value));
						if(strcmp(key, "cur") != 0)
							CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Error reading from the search server.");

						rcode = atoi(value);
						if(rcode)
						{
							iconnection->errorCode = (GPErrorCode)rcode;
							pid = 0;
						}
						else
						{
							if(!gpiValueForKey(data->inputBuffer.buffer, "\\pid\\", value, sizeof(value)))
								CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Error reading from the search server.");
							pid = atoi(value);
						}

						// Setup the arg.
						/////////////////
						arg = (GPCheckResponseArg *)gsimalloc(sizeof(GPCheckResponseArg));
						if(arg == NULL)
							Error(connection, GP_MEMORY_ERROR, "Out of memory.");
						arg->result = (GPResult)rcode;
						arg->profile = pid;

						// Add the callback.
						////////////////////
						CHECK_RESULT(gpiAddCallback(connection, callback, arg, operation, 0));
					}
				}
				else if(data->type == GPI_SEARCH_NEWUSER)
				{
					callback = operation->callback;
					if(callback.callback != NULL)
					{
						GPNewUserResponseArg * arg;

						CHECK_RESULT(gpiReadKeyAndValue(connection, data->inputBuffer.buffer, &index, key, value));
						if(strcmp(key, "nur") != 0)
							CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Error reading from the search server.");

						rcode = atoi(value);
						if(rcode)
							iconnection->errorCode = (GPErrorCode)rcode;
						if(!gpiValueForKey(data->inputBuffer.buffer, "\\pid\\", value, sizeof(value)))
						{
							if(rcode == 0)
								CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Error reading from the search server.");
							pid = 0;
						}
						else
							pid = atoi(value);

						// Setup the arg.
						/////////////////
						arg = (GPNewUserResponseArg *)gsimalloc(sizeof(GPNewUserResponseArg));
						if(arg == NULL)
							Error(connection, GP_MEMORY_ERROR, "Out of memory.");
						arg->result = GP_NO_ERROR;
						arg->profile = pid;

						// Add the callback.
						////////////////////
						CHECK_RESULT(gpiAddCallback(connection, callback, arg, operation, 0));
					}
				}
				else if(data->type == GPI_SEARCH_OTHERS_BUDDY)
				{
					callback = operation->callback;
					if(callback.callback != NULL)
					{
						GPGetReverseBuddiesResponseArg * arg;

						// Setup the arg.
						/////////////////
						arg = (GPGetReverseBuddiesResponseArg *)gsimalloc(sizeof(GPGetReverseBuddiesResponseArg));
						if(arg == NULL)
							Error(connection, GP_MEMORY_ERROR, "Out of memory.");
						arg->result = GP_NO_ERROR;
						arg->numProfiles = 0;
						arg->profiles = NULL;

						CHECK_RESULT(gpiReadKeyAndValue(connection, data->inputBuffer.buffer, &index, key, value));
						if(strcmp(key, "others") != 0)
							CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Error reading from the search server.");

						// Get the profiles.
						/////////////////
						done = GPIFalse;
						do
						{
							// Read the next key and value.
							///////////////////////////////
							CHECK_RESULT(gpiReadKeyAndValue(connection, data->inputBuffer.buffer, &index, key, value));

							// Is the list done?
							////////////////////
							if(strcmp(key, "odone") == 0)
							{
								// Done.
								////////
								done = GPITrue;
							}
							else if(strcmp(key, "o") == 0)
							{
								// Add it.
								//////////
								tempPtr = gsirealloc(arg->profiles, sizeof(GPProfileSearchMatch) * (arg->numProfiles + 1));
								if(tempPtr == NULL)
									Error(connection, GP_MEMORY_ERROR, "Out of memory.");
								arg->profiles = (GPProfileSearchMatch *)tempPtr;
								match = &arg->profiles[arg->numProfiles];
								memset(match, 0, sizeof(GPProfileSearchMatch));
								arg->numProfiles++;

								// Get the profile id.
								//////////////////////
								match->profile = atoi(value);

								// Read key/value pairs.
								////////////////////////
								doneParsingMatch = GPIFalse;
								do
								{
									// Read the next key/value.
									///////////////////////////
									oldIndex = index;
									CHECK_RESULT(gpiReadKeyAndValue(connection, data->inputBuffer.buffer, &index, key, value));

									// Set the field based on the key.
									//////////////////////////////////
									if(strcmp(key, "nick") == 0)
										strzcpy(match->nick, value, GP_NICK_LEN);
									else if(strcmp(key, "first") == 0)
										strzcpy(match->firstname, value, GP_FIRSTNAME_LEN);
									else if(strcmp(key, "last") == 0)
										strzcpy(match->lastname, value, GP_LASTNAME_LEN);
									else if(strcmp(key, "email") == 0)
										strzcpy(match->email, value, GP_EMAIL_LEN);
									else if((strcmp(key, "o") == 0) || (strcmp(key, "odone") == 0))
									{
										doneParsingMatch = GPITrue;
										index = oldIndex;
									}
								}
								while(!doneParsingMatch);
							}
							else
							{
								CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Error reading from the search server.");
							}
						}
						while(!done);

						// Do it.
						/////////
						CHECK_RESULT(gpiAddCallback(connection, callback, arg, operation, GPI_ADD_REVERSE_BUDDIES));
					}
				}
				else
				{
					assert(0);
				}

				// Flag the operation for removal.
				//////////////////////////////////
				data->remove = GPITrue;

				// If we're looping, stop.
				//////////////////////////
				loop = GPIFalse;
			}
		}
		//PANTS|05.23.00 - removed sleep
		//crt - added it back 6/13/00
		//PANTS|07.10.00 - only sleep if looping
		if(loop)
			msleep(10);
	} while(loop);

	return GP_NO_ERROR;
}

GPResult
gpiProcessSearches(
  GPConnection * connection
)
{
	GPIConnection * iconnection = (GPIConnection*)*connection;
	GPIOperation ** searchList;
	GPIOperation * operation;
	GPISearchData * data;
	GPResult result;
	int num = 0;
	int i;

	// Are there any searches?
	//////////////////////////
	if(iconnection->numSearches > 0)
	{
		// Alloc mem for a search list.
		///////////////////////////////
		searchList = (GPIOperation **)gsimalloc(sizeof(GPIOperation *) * iconnection->numSearches);
		if(searchList == NULL)
			Error(connection, GP_MEMORY_ERROR, "Out of memory.");

		// Create the search list.
		//////////////////////////
		for(operation = &iconnection->operationList[0] ; operation != NULL ; operation = operation->pnext)
		{
			// Is this a search?
			////////////////////
			if((operation->type == GPI_PROFILE_SEARCH) && (operation->state != GPI_FINISHING))
			{
				// Is this search being processed already?
				//////////////////////////////////////////
				if(!((GPISearchData *)operation->data)->processing)
				{
					assert(num < iconnection->numSearches);
					searchList[num++] = operation;
					((GPISearchData *)operation->data)->processing = GPITrue;
				}
			}
		}

		// Process the searches.
		////////////////////////
		for(i = 0 ; i < num ; i++)
		{
			result = gpiProcessSearch(connection, searchList[i]);
			if(result != GP_NO_ERROR)
				searchList[i]->result = result;
		}

		// Clear the processing flags, and remove searches that are done.
		/////////////////////////////////////////////////////////////////
		for(i = 0 ; i < num ; i++)
		{
			data = ((GPISearchData *)searchList[i]->data);
			data->processing = GPIFalse;
			if(data->remove)
				gpiRemoveOperation(connection, searchList[i]);
		}

		freeclear(searchList);
	}

	return GP_NO_ERROR;
}
