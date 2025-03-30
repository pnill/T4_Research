 /*
GameSpy GHTTP SDK 
Dan "Mr. Pants" Schoenblum
dan@gamespy.com

Copyright 1999-2001 GameSpy Industries, Inc

18002 Skypark Circle
Irvine, California 92614
949.798.4200 (Tel)
949.798.4299 (Fax)
devsupport@gamespy.com
*/

#include "ghttpMain.h"
#include "ghttpConnection.h"
#include "ghttpCallbacks.h"
#include "ghttpProcess.h"
#include "ghttpPost.h"
#include "ghttpCommon.h"

// Reference count.
///////////////////
static int ghiReferenceCount;

// Called right before callback is called.
// Sets result based on response status code.
/////////////////////////////////////////////
static void ghiHandleStatus
(
	GHIConnection * connection
)
{
	// Check the status code.
	/////////////////////////
	switch(connection->statusCode / 100)
	{
	case 1:  // Informational.
		return;
	case 2:  // Successful.
		return;
	case 3:  // Redirection.
		return;
	case 4:  // Client Error.
		switch(connection->statusCode)
		{
		case 401:
			connection->result = GHTTPUnauthorized;
			break;
		case 403:
			connection->result = GHTTPForbidden;
			break;
		case 404:
		case 410:
			connection->result = GHTTPFileNotFound;
			break;
		default:
			connection->result = GHTTPRequestRejected;
			break;
		}
		return;
	case 5:  // Internal Server Error.
		connection->result = GHTTPServerError;
		return;
	}
}

// Processes a single connection based on its state.
// Returns true if the connection is finished.
////////////////////////////////////////////////////
static GHTTPBool ghiProcessConnection
(
	GHIConnection * connection
)
{
	GHTTPBool completed;

	assert(connection);
	assert(ghiRequestToConnection(connection->request) == connection);

	// Don't process if already processing this connection.
	// Happens if, for example, ghttpThink is called from a callback.
	/////////////////////////////////////////////////////////////////
	if(connection->processing)
		return GHTTPFalse;

	// We're now processing.
	////////////////////////
	connection->processing = GHTTPTrue;

	// Process based on state.
	// else-if is not used so that if one ghiDo*()
	// finishes the one after it can start.
	//////////////////////////////////////////////
	if(connection->state == GHTTPHostLookup)
		ghiDoHostLookup(connection);
	if(connection->state == GHTTPConnecting)
		ghiDoConnecting(connection);
	if(connection->state == GHTTPSendingRequest)
		ghiDoSendingRequest(connection);
	if(connection->state == GHTTPPosting)
		ghiDoPosting(connection);
	if(connection->state == GHTTPWaiting)
		ghiDoWaiting(connection);
	if(connection->state == GHTTPReceivingStatus)
		ghiDoReceivingStatus(connection);
	if(connection->state == GHTTPReceivingHeaders)
		ghiDoReceivingHeaders(connection);
	if(connection->state == GHTTPReceivingFile)
		ghiDoReceivingFile(connection);

	// Check for a redirect.
	////////////////////////
	if(connection->redirectURL)
		ghiRedirectConnection(connection);

	// Grab completed before we possibly free it.
	/////////////////////////////////////////////
	completed = connection->completed;

	// Is it finished?
	//////////////////
	if(connection->completed)
	{
		// Set result based on status code.
		///////////////////////////////////
		ghiHandleStatus(connection);

		// If we're saving to file, close it before the callback.
		/////////////////////////////////////////////////////////
#ifndef NOFILE
		if(connection->saveFile)
		{
			fclose(connection->saveFile);
			connection->saveFile = NULL;
		}
#endif
		// Call the callback.
		/////////////////////
		ghiCallCompletedCallback(connection);

		// Free it.
		///////////
		ghiFreeConnection(connection);
	}
	else
	{
		// Done processing. This is in the else,
		// because we don't want to set it if the
		// connection has already been freed.
		/////////////////////////////////////////
		connection->processing = GHTTPFalse;
	}

	return completed;
}

void ghttpStartup
(
	void
)
{
	// This will just return if we haven't created the lock yet.
	////////////////////////////////////////////////////////////
	ghiLock();

	// One more startup.
	////////////////////
	ghiReferenceCount++;

	// Check if we are the first.
	/////////////////////////////
	if(ghiReferenceCount == 1)
	{
		// Create the lock.
		///////////////////
		ghiCreateLock();

		// Set some defaults.
		/////////////////////
		ghiThrottleBufferSize = GHI_DEFAULT_THROTTLE_BUFFER_SIZE;
		ghiThrottleTimeDelay = GHI_DEFAULT_THROTTLE_TIME_DELAY;
	}
	else
	{
		// Unlock the lock.
		///////////////////
		ghiUnlock();
	}
}

void ghttpCleanup
(
	void
)
{
	// Lockdown for cleanup.
	////////////////////////
	ghiLock();

	// One less.
	////////////
	ghiReferenceCount--;

	// Should we cleanup?
	/////////////////////
	if(!ghiReferenceCount)
	{
		// Cleanup the connections.
		///////////////////////////
		ghiCleanupConnections();

		// Cleanup proxy.
		/////////////////
		if(ghiProxyAddress)
		{
			gsifree(ghiProxyAddress);
			ghiProxyAddress = NULL;
		}

		// Unlock the lock before freeing it.
		/////////////////////////////////////
		ghiUnlock();

		// Free the lock.
		/////////////////
		ghiFreeLock();
	}
	else
	{
		// Unlock our lock.
		///////////////////
		ghiUnlock();
	}
}

GHTTPRequest ghttpGet
(
	const char * URL,
	GHTTPBool blocking,
	ghttpCompletedCallback completedCallback,
	void * param
)
{
	return ghttpGetEx(URL, NULL, NULL, 0, NULL, GHTTPFalse, blocking, NULL, completedCallback, param);
}

GHTTPRequest ghttpGetEx
(
	const char * URL,
	const char * headers,
	char * buffer,
	int bufferSize,
	GHTTPPost post,
	GHTTPBool throttle,
	GHTTPBool blocking,
	ghttpProgressCallback progressCallback,
	ghttpCompletedCallback completedCallback,
	void * param
)
{
	GHTTPBool bResult;
	GHIConnection * connection;

	assert(URL && URL[0]);
	assert(bufferSize >= 0);
	assert(!buffer || bufferSize);

	// Check args.
	//////////////
	if(!URL || !URL[0])
		return -1;
	if(bufferSize < 0)
		return -1;
	if(buffer && !bufferSize)
		return -1;

	// Startup if it hasn't been done.
	//////////////////////////////////
	if(!ghiReferenceCount)
		ghttpStartup();

	// Get a new connection object.
	///////////////////////////////
	connection = ghiNewConnection();
	if(!connection)
		return -1;

	// Fill in the necessary info.
	//////////////////////////////
	connection->type = GHIGET;
	connection->URL = strdup(URL);
	if(!connection->URL)
	{
		ghiFreeConnection(connection);
		return -1;
	}
	if(headers && *headers)
	{
		connection->sendHeaders = goastrdup(headers);
		if(!connection->sendHeaders)
		{
			ghiFreeConnection(connection);
			return -1;
		}
	}
	connection->post = post;
	connection->blocking = blocking;
	connection->progressCallback = progressCallback;
	connection->completedCallback = completedCallback;
	connection->callbackParam = param;
	connection->throttle = throttle;
	connection->userBufferSupplied = (GHTTPBool)(buffer != NULL);
	if(connection->userBufferSupplied)
		bResult = ghiInitFixedBuffer(connection, &connection->getFileBuffer, buffer, bufferSize);
	else
		bResult = ghiInitBuffer(connection, &connection->getFileBuffer, GET_FILE_BUFFER_INITIAL_SIZE, GET_FILE_BUFFER_INCREMENT_SIZE);
	if(!bResult)
	{
		ghiFreeConnection(connection);
		return -1;
	}

	// Setup the post state if needed.
	//////////////////////////////////
	if(post && !ghiPostInitState(connection))
	{
		ghiFreeConnection(connection);
		return -1;
	}

	// Check blocking.
	//////////////////
	if(blocking)
	{
		// Loop until completed.
		////////////////////////
		while(!ghiProcessConnection(connection))
			msleep(10);

		// Done.
		////////
		return 0;
	}

	return connection->request;
}

GHTTPRequest ghttpSave
(
	const char * URL,
	const char * filename,
	GHTTPBool blocking,
	ghttpCompletedCallback completedCallback,
	void * param
)
{
	return ghttpSaveEx(URL, filename, NULL, NULL, GHTTPFalse, blocking, NULL, completedCallback, param);
}

GHTTPRequest ghttpSaveEx
(
	const char * URL,
	const char * filename,
	const char * headers,
	GHTTPPost post,
	GHTTPBool throttle,
	GHTTPBool blocking,
	ghttpProgressCallback progressCallback,
	ghttpCompletedCallback completedCallback,
	void * param
)
{
	GHIConnection * connection;

	assert(URL && URL[0]);
	assert(filename && filename[0]);

	// Check args.
	//////////////
	if(!URL || !URL[0])
		return -1;
	if(!filename || !filename[0])
		return -1;

	// Startup if it hasn't been done.
	//////////////////////////////////
	if(!ghiReferenceCount)
		ghttpStartup();

	// Get a new connection object.
	///////////////////////////////
	connection = ghiNewConnection();
	if(!connection)
		return -1;

	// Fill in the necessary info.
	//////////////////////////////
	connection->type = GHISAVE;
	connection->URL = strdup(URL);
	if(!connection->URL)
	{
		ghiFreeConnection(connection);
		return -1;
	}
	if(headers && *headers)
	{
		connection->sendHeaders = goastrdup(headers);
		if(!connection->sendHeaders)
		{
			ghiFreeConnection(connection);
			return -1;
		}
	}
	connection->post = post;
	connection->blocking = blocking;
	connection->progressCallback = progressCallback;
	connection->completedCallback = completedCallback;
	connection->callbackParam = param;
	connection->throttle = throttle;

	// Setup the post state if needed.
	//////////////////////////////////
	if(post && !ghiPostInitState(connection))
	{
		ghiFreeConnection(connection);
		return -1;
	}

	// Open the file we're saving to.
	/////////////////////////////////
#ifdef NOFILE
	connection->saveFile = NULL;
#else
	connection->saveFile = fopen(filename, "wb");
#endif
	if(!connection->saveFile)
	{
		ghiFreeConnection(connection);
		return -1;
	}

	// Check blocking.
	//////////////////
	if(blocking)
	{
		// Loop until completed.
		////////////////////////
		while(!ghiProcessConnection(connection))
			msleep(10);

		// Done.
		////////
		return 0;
	}

	return connection->request;
}

GHTTPRequest ghttpStream
(
	const char * URL,
	GHTTPBool blocking,
	ghttpProgressCallback progressCallback,
	ghttpCompletedCallback completedCallback,
	void * param
)
{
	return ghttpStreamEx(URL, NULL, NULL, GHTTPFalse, blocking, progressCallback, completedCallback, param);
}

GHTTPRequest ghttpStreamEx
(
	const char * URL,
	const char * headers,
	GHTTPPost post,
	GHTTPBool throttle,
	GHTTPBool blocking,
	ghttpProgressCallback progressCallback,
	ghttpCompletedCallback completedCallback,
	void * param
)
{
	GHIConnection * connection;

	assert(URL && URL[0]);

	// Check args.
	//////////////
	if(!URL || !URL[0])
		return -1;

	// Startup if it hasn't been done.
	//////////////////////////////////
	if(!ghiReferenceCount)
		ghttpStartup();

	// Get a new connection object.
	///////////////////////////////
	connection = ghiNewConnection();
	if(!connection)
		return -1;

	// Fill in the necessary info.
	//////////////////////////////
	connection->type = GHISTREAM;
	connection->URL = strdup(URL);
	if(!connection->URL)
	{
		ghiFreeConnection(connection);
		return -1;
	}
	if(headers && *headers)
	{
		connection->sendHeaders = goastrdup(headers);
		if(!connection->sendHeaders)
		{
			ghiFreeConnection(connection);
			return -1;
		}
	}
	connection->post = post;
	connection->blocking = blocking;
	connection->progressCallback = progressCallback;
	connection->completedCallback = completedCallback;
	connection->callbackParam = param;
	connection->throttle = throttle;

	// Setup the post state if needed.
	//////////////////////////////////
	if(post && !ghiPostInitState(connection))
	{
		ghiFreeConnection(connection);
		return -1;
	}

	// Check blocking.
	//////////////////
	if(blocking)
	{
		// Loop until completed.
		////////////////////////
		while(!ghiProcessConnection(connection))
			msleep(10);

		// Done.
		////////
		return 0;
	}

	return connection->request;
}

GHTTPRequest ghttpHead
(
	const char * URL,
	GHTTPBool blocking,
	ghttpCompletedCallback completedCallback,
	void * param
)
{
	return ghttpHeadEx(URL, NULL, GHTTPFalse, blocking, NULL, completedCallback, param);
}

GHTTPRequest ghttpHeadEx
(
	const char * URL,
	const char * headers,
	GHTTPBool throttle,
	GHTTPBool blocking,
	ghttpProgressCallback progressCallback,
	ghttpCompletedCallback completedCallback,
	void * param
)
{
	GHIConnection * connection;

	assert(URL && URL[0]);

	// Check args.
	//////////////
	if(!URL || !URL[0])
		return -1;

	// Startup if it hasn't been done.
	//////////////////////////////////
	if(!ghiReferenceCount)
		ghttpStartup();

	// Get a new connection object.
	///////////////////////////////
	connection = ghiNewConnection();
	if(!connection)
		return -1;

	// Fill in the necessary info.
	//////////////////////////////
	connection->type = GHIHEAD;
	connection->URL = strdup(URL);
	if(!connection->URL)
	{
		ghiFreeConnection(connection);
		return -1;
	}
	if(headers && *headers)
	{
		connection->sendHeaders = goastrdup(headers);
		if(!connection->sendHeaders)
		{
			ghiFreeConnection(connection);
			return -1;
		}
	}
	connection->blocking = blocking;
	connection->progressCallback = progressCallback;
	connection->completedCallback = completedCallback;
	connection->callbackParam = param;
	connection->throttle = throttle;

	// Check blocking.
	//////////////////
	if(blocking)
	{
		// Loop until completed.
		////////////////////////
		while(!ghiProcessConnection(connection))
			msleep(10);

		// Done.
		////////
		return 0;
	}

	return connection->request;
}

GHTTPRequest ghttpPost
(
	const char * URL,
	GHTTPPost post,
	GHTTPBool blocking,
	ghttpCompletedCallback completedCallback,
	void * param
)
{
	return ghttpPostEx(URL, NULL, post, GHTTPFalse, blocking, NULL, completedCallback, param);
}

GHTTPRequest ghttpPostEx
(
	const char * URL,
	const char * headers,
	GHTTPPost post,
	GHTTPBool throttle,
	GHTTPBool blocking,
	ghttpProgressCallback progressCallback,
	ghttpCompletedCallback completedCallback,
	void * param
)
{
	GHIConnection * connection;

	assert(URL && URL[0]);
	assert(post);

	// Check args.
	//////////////
	if(!URL || !URL[0])
		return -1;
	if(!post)
		return -1;

	// Startup if it hasn't been done.
	//////////////////////////////////
	if(!ghiReferenceCount)
		ghttpStartup();

	// Get a new connection object.
	///////////////////////////////
	connection = ghiNewConnection();
	if(!connection)
		return -1;

	// Fill in the necessary info.
	//////////////////////////////
	connection->type = GHIPOST;
	connection->URL = strdup(URL);
	if(!connection->URL)
	{
		ghiFreeConnection(connection);
		return -1;
	}
	if(headers && *headers)
	{
		connection->sendHeaders = goastrdup(headers);
		if(!connection->sendHeaders)
		{
			ghiFreeConnection(connection);
			return -1;
		}
	}
	connection->post = post;
	connection->blocking = blocking;
	connection->progressCallback = progressCallback;
	connection->completedCallback = completedCallback;
	connection->callbackParam = param;
	connection->throttle = throttle;

	// Setup the post state if needed.
	//////////////////////////////////
	if(post && !ghiPostInitState(connection))
	{
		ghiFreeConnection(connection);
		return -1;
	}

	// Check blocking.
	//////////////////
	if(blocking)
	{
		// Loop until completed.
		////////////////////////
		while(!ghiProcessConnection(connection))
			msleep(10);

		// Done.
		////////
		return 0;
	}

	return connection->request;
}

void ghttpThink
(
	void
)
{
	// Process all the connections.
	///////////////////////////////
	ghiEnumConnections(ghiProcessConnection);
}

GHTTPBool ghttpRequestThink
(
	GHTTPRequest request
)
{
	GHIConnection * connection;

	// Get the connection object for this request.
	//////////////////////////////////////////////
	connection = ghiRequestToConnection(request);
	if(!connection)
		return GHTTPFalse;

	// Think.
	/////////
	ghiProcessConnection(connection);
	return GHTTPTrue;
}

void ghttpCancelRequest
(
	GHTTPRequest request
)
{
	GHIConnection * connection;

	// Get the connection object for this request.
	//////////////////////////////////////////////
	connection = ghiRequestToConnection(request);
	if(!connection)
		return;

	// Free it.
	///////////
	ghiFreeConnection(connection);
}

GHTTPState ghttpGetState
(
	GHTTPRequest request
)
{
	GHIConnection * connection;

	// Get the connection object for this request.
	//////////////////////////////////////////////
	connection = ghiRequestToConnection(request);
	if(!connection)
		return (GHTTPState)0;

	return connection->state;
}

const char * ghttpGetResponseStatus
(
	GHTTPRequest request,
	int * statusCode
)
{
	GHIConnection * connection;

	// Get the connection object for this request.
	//////////////////////////////////////////////
	connection = ghiRequestToConnection(request);
	if(!connection)
		return NULL;

	// Check if we don't have the status yet.
	/////////////////////////////////////////
	if(connection->state <= GHTTPReceivingStatus)
		return NULL;

	// Set the status code.
	///////////////////////
	if(statusCode)
		*statusCode = connection->statusCode;

	return (connection->recvBuffer.data + connection->statusStringIndex);
}

const char * ghttpGetHeaders
(
	GHTTPRequest request
)
{
	GHIConnection * connection;

	// Get the connection object for this request.
	//////////////////////////////////////////////
	connection = ghiRequestToConnection(request);
	if(!connection)
		return NULL;

	// Check if we don't have the headers yet.
	//////////////////////////////////////////
	if(connection->state < GHTTPReceivingHeaders)
		return NULL;

	// Verify we have headers.
	//////////////////////////
	if(connection->recvBuffer.pos > connection->recvBuffer.len)
		return NULL;

	return (connection->recvBuffer.data + connection->recvBuffer.pos);
}

const char * ghttpGetURL
(
	GHTTPRequest request
)
{
	GHIConnection * connection;

	// Get the connection object for this request.
	//////////////////////////////////////////////
	connection = ghiRequestToConnection(request);
	if(!connection)
		return NULL;

	return connection->URL;
}

GHTTPBool ghttpSetProxy
(
	const char * server
)
{
	return ghiSetProxy(server);
}

void ghttpSetThrottle
(
	GHTTPRequest request,
	GHTTPBool throttle
)
{
	GHIConnection * connection;

	// Get the connection object for this request.
	//////////////////////////////////////////////
	connection = ghiRequestToConnection(request);
	if(!connection)
		return;

	connection->throttle = throttle;

	// Set the buffer size based on the throttle setting.
	/////////////////////////////////////////////////////
	if(connection->socket != INVALID_SOCKET)
		SetReceiveBufferSize(connection->socket, throttle?ghiThrottleBufferSize:(8 * 1024));
}

void ghttpThrottleSettings
(
	int bufferSize,
	int timeDelay
)
{
	ghiThrottleSettings(bufferSize, timeDelay);
}

GHTTPPost ghttpNewPost
(
	void
)
{
	return ghiNewPost();
}

void ghttpPostSetAutoFree
(
	GHTTPPost post,
	GHTTPBool autoFree
)
{
	assert(post);
	if(!post)
		return;
		
	//GSI_UNUSED(autoFree);
	(void)(autoFree);
}

void ghttpFreePost
(
	GHTTPPost post
)
{
	assert(post);
	if(!post)
		return;

	ghiFreePost(post);
}

GHTTPBool ghttpPostAddString
(
	GHTTPPost post,
	const char * name,
	const char * string
)
{
	assert(post);
	assert(name && name[0]);

	if(!post)
		return GHTTPFalse;
	if(!name || !name[0])
		return GHTTPFalse;
	if(!string)
		string = "";

	return ghiPostAddString(post, name, string);
}

GHTTPBool ghttpPostAddFileFromDisk
(
	GHTTPPost post,
	const char * name,
	const char * filename,
	const char * reportFilename,
	const char * contentType
)
{
	assert(post);
	assert(name && name[0]);
	assert(filename && filename[0]);

	if(!post)
		return GHTTPFalse;
	if(!name || !name[0])
		return GHTTPFalse;
	if(!filename || !filename[0])
		return GHTTPFalse;
	if(!reportFilename || !reportFilename[0])
		reportFilename = filename;
	if(!contentType)
		contentType = "application/octet-stream";

	return ghiPostAddFileFromDisk(post, name, filename, reportFilename, contentType);
}

GHTTPBool ghttpPostAddFileFromMemory
(
	GHTTPPost post,
	const char * name,
	const char * buffer,
	int bufferLen,
	const char * reportFilename,
	const char * contentType
)
{
	assert(post);
	assert(name && name[0]);
	assert(bufferLen >= 0);
#ifdef _DEBUG
	if(bufferLen > 0)
		assert(buffer);
#endif
	assert(reportFilename && reportFilename[0]);

	if(!post)
		return GHTTPFalse;
	if(!name || !name[0])
		return GHTTPFalse;
	if(bufferLen < 0)
		return GHTTPFalse;
	if(!bufferLen && !buffer)
		return GHTTPFalse;
	if(!contentType)
		contentType = "application/octet-stream";

	return ghiPostAddFileFromMemory(post, name, buffer, bufferLen, reportFilename, contentType);
}

void ghttpPostSetCallback
(
	GHTTPPost post,
	ghttpPostCallback callback,
	void * param
)
{
	assert(post);

	if(!post)
		return;

	ghiPostSetCallback(post, callback, param);
}
