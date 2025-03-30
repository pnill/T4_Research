/*
gpiUtility.c
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
#include <stdio.h>
#include <stdarg.h>
#include "gpi.h"

//DEFINES
/////////
#define OUTPUT_MAX_COL     100

// Disable compiler warnings for issues that are unavoidable.
/////////////////////////////////////////////////////////////
#if defined(_MSC_VER) // DevStudio
// Level4, "conditional expression is constant". 
// Occurs with use of the MS provided macro FD_SET
#pragma warning ( disable: 4127 )
#endif // _MSC_VER

//FUNCTIONS
///////////
void
strzcpy(
  char * dest,
  const char * src,
  size_t len
)
{
	assert(dest != NULL);
	assert(src != NULL);
	assert(len >= 0);

	strncpy(dest, src, len);
	dest[len - 1] = '\0';
}

void
gpiDebug(
  GPConnection * connection,
  const char * format,
  ...
)
{
#ifdef GPDEBUG
	GPIConnection * iconnection = *connection;
	va_list argList;
	char * debugBuffer;
	int len;
	char * str;
	int i;
	char c;
#ifdef UNICODE
	wchar_t widebuff[1024];
#endif
#ifdef DEBUG_CRAZY
	FILE * fp;
#endif

	va_start(argList, format);

	//
	// These allocations were thrashing the heap so that we could rarely
	// compact to regain memory. The result was that GPInit() grew our
	// heap by 65MB.
	//
	// Here I allocate a single global buffer that we reuse. Does this
	// have any reentrancy issues? Note that I never freeclear this...I assume
	// the local heap will be freed on exit. (28jan00/bgw)
	//
	//buffer = gsimalloc(200000);

	//
	// If the buffer has not been alloc'd yet, do it now.
	//
	debugBuffer = iconnection->debugBuffer;
	if(debugBuffer == NULL)
		return;
		
	vsprintf(debugBuffer, format, argList);

#ifdef DEBUG_CRAZY
	// Append to the log.
	/////////////////////
	fp = fopen("GP.log", "at");
#endif

	// This is very hacky line splitting and file output code.
	//////////////////////////////////////////////////////////
	str = debugBuffer;
	len = strlen(debugBuffer);
	c = debugBuffer[OUTPUT_MAX_COL + 5];
	debugBuffer[OUTPUT_MAX_COL + 5] = '\0';
#ifdef _WIN32
	#ifdef UNICODE
		MultiByteToWideChar(CP_ACP,0,debugBuffer,-1,widebuff,1024);
		OutputDebugString(widebuff);
	#else
		OutputDebugString(debugBuffer);
	#endif
#else
	fprintf(stderr,"%s",debugBuffer);
#endif

#ifdef DEBUG_CRAZY
	if(fp != NULL)
		fwrite(debugBuffer, 1, min(OUTPUT_MAX_COL + 5, len), fp);
#endif
	debugBuffer[OUTPUT_MAX_COL + 5] = c;
	for(i = OUTPUT_MAX_COL + 5 ; i < len ; i += OUTPUT_MAX_COL)
	{
#ifdef _WIN32
	OutputDebugString(TEXT("\n     "));
#else
	fprintf(stderr,"\n     ");

#endif
		
#ifdef DEBUG_CRAZY
		if(fp != NULL)
			fwrite("\n     ", 1, 6, fp);
#endif
		c = debugBuffer[i + OUTPUT_MAX_COL];
		debugBuffer[i + OUTPUT_MAX_COL] = '\0';
#ifdef _WIN32
	#ifdef UNICODE
		MultiByteToWideChar(CP_ACP,0,&debugBuffer[i],-1,widebuff,1024);
		OutputDebugString(widebuff);
	#else
		OutputDebugString(&debugBuffer[i]);
	#endif
#else
	fprintf(stderr,"%s",&debugBuffer[i]);

#endif
		
#ifdef DEBUG_CRAZY
		if(fp != NULL)
			fwrite(&debugBuffer[i], 1, min(OUTPUT_MAX_COL, len - i), fp);
#endif
		debugBuffer[i + OUTPUT_MAX_COL] = c;
	}

#ifdef DEBUG_CRAZY
	if(fp != NULL)
		fclose(fp);
#endif

//
// We don't freeclear debugBuffer on purpose, we're going to reuse it
// next time through. (28jan00/bgw)
//
//	freeclear(buffer);

	va_end(argList);
#endif

}

GPIBool
gpiCheckForError(
  GPConnection * connection,
  const char * input,
  GPIBool callErrorCallback
)
{
	char buffer[16];
	GPIConnection * iconnection = (GPIConnection*)*connection;
	
	if(strncmp(input, "\\error\\", 7) == 0)
	{
		// Get the err code.
		////////////////////
		if(gpiValueForKey(input, "\\err\\", buffer, sizeof(buffer)))
			iconnection->errorCode = (GPErrorCode)atoi(buffer);
		
		// Get the error string.
		////////////////////////
		if(!gpiValueForKey(input, "\\errmsg\\", iconnection->errorString, sizeof(iconnection->errorString)))
			iconnection->errorString[0] = '\0';

		// Call the error callback?
		///////////////////////////
		if(callErrorCallback)
		{
			GPIBool fatal = (GPIBool)(strstr(input, "\\fatal\\") != NULL);
			gpiCallErrorCallback(connection, GP_SERVER_ERROR, fatal ? GP_FATAL : GP_NON_FATAL);
		}
		
		return GPITrue;
	}

	return GPIFalse;
}

GPIBool
gpiValueForKey(
  const char * command,
  const char * key,
  char * value,
  int len
)
{
	char delimiter;
	char * start;
	int i;
	char c;

	// Check for NULL.
	//////////////////
	assert(command != NULL);
	assert(key != NULL);
	assert(value != NULL);
	assert(len > 0);

	// Find which char is the delimiter.
	////////////////////////////////////
	delimiter = key[0];

	// Find the key.
	////////////////
	start = strstr(command, key);
	if(start == NULL)
		return GPIFalse;

	// Get to the start of the value.
	/////////////////////////////////
	start += strlen(key);

	// Copy in the value.
	/////////////////////
	len--;
	for(i = 0 ; (i < len) && ((c = start[i]) != '\0') && (c != delimiter) ; i++)
	{
		value[i] = c;
	}
	value[i] = '\0';

	return GPITrue;
}

char *
gpiValueForKeyAlloc(
  const char * command,
  const char * key
)
{
	char delimiter;
	char * start;
	char c;
	char * value;
	int len;

	// Check for NULL.
	//////////////////
	assert(command != NULL);
	assert(key != NULL);

	// Find which char is the delimiter.
	////////////////////////////////////
	delimiter = key[0];

	// Find the key.
	////////////////
	start = strstr(command, key);
	if(start == NULL)
		return NULL;

	// Get to the start of the value.
	/////////////////////////////////
	start += strlen(key);

	// Find the key length.
	///////////////////////
	for(len = 0 ; ((c = start[len]) != '\0') && (c != delimiter) ; len++)  { };

	// Allocate the value.
	//////////////////////
	value = (char *)gsimalloc(len + 1);
	if(!value)
		return NULL;

	// Copy in the value.
	/////////////////////
	memcpy(value, start, len);
	value[len] = '\0';

	return value;
}

GPResult
gpiCheckSocketConnect(
  GPConnection * connection,
  SOCKET sock,
  int * state
)
{
	fd_set writeSet;
	fd_set exceptSet;
	struct timeval timeout;
	int rcode;

	// Check if the connect is completed.
	/////////////////////////////////////
	FD_ZERO(&writeSet);
	FD_SET(sock, &writeSet);
	FD_ZERO(&exceptSet);
	FD_SET(sock, &exceptSet);
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	rcode = select(FD_SETSIZE, NULL,	&writeSet, &exceptSet, &timeout);
	if(rcode == SOCKET_ERROR)
	{
		gpiDebug(connection, "Error connecting\n");
		CallbackFatalError(connection, GP_NETWORK_ERROR, GP_NETWORK, "There was an error checking for a completed connection.");
	}

	if (rcode > 0)
	{
		// Check for a failed attempt.
		//////////////////////////////
		if(FD_ISSET(sock, &exceptSet))
		{
			gpiDebug(connection, "Connection rejected\n");
			*state = GPI_DISCONNECTED;
			return GP_NO_ERROR;
		}

		// Check for a successful attempt.
		//////////////////////////////////
		if(FD_ISSET(sock, &writeSet))
		{
			gpiDebug(connection, "Connection accepted\n");
			*state = GPI_CONNECTED;
			return GP_NO_ERROR;
		}
	}

	// Not connected yet.
	/////////////////////
	*state = GPI_NOT_CONNECTED;
	return GP_NO_ERROR;
}

GPResult
gpiReadKeyAndValue(
  GPConnection * connection,
  const char * buffer,
  int * index,
  char key[512],
  char value[512]
)
{
	int c;
	int i;
	char * start;

	assert(buffer != NULL);
	assert(key != NULL);
	assert(value != NULL);

	buffer += *index;
	start = (char *)buffer;

	if(*buffer++ != '\\')
		CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Parse Error.");

	for(i = 0 ; (c = *buffer++) != '\\' ; i++)
	{
		if(c == '\0')
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Parse Error.");
		if(i == 511)
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Parse Error.");
		*key++ = (char)c;
	}
	*key = '\0';

	for(i = 0 ; ((c = *buffer++) != '\\') && (c != '\0') ; i++)
	{
		if(i == 511)
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Parse Error.");
		*value++ = (char)c;
	}
	*value = '\0';

	*index += (buffer - start - 1);

	return GP_NO_ERROR;
}

void
gpiSetError(
  GPConnection * connection,
  GPErrorCode errorCode,
  const char * errorString
)
{
	GPIConnection * iconnection = (GPIConnection*)*connection;
	
	// Copy the string.
	///////////////////
	strzcpy(iconnection->errorString, errorString, GP_ERROR_STRING_LEN);

	// Set the code.
	////////////////
	iconnection->errorCode = errorCode;
}

void
gpiSetErrorString(
  GPConnection * connection,
  const char * errorString
)
{
	GPIConnection * iconnection = (GPIConnection*)*connection;
	
	// Copy the string.
	///////////////////
	strzcpy(iconnection->errorString, errorString, GP_ERROR_STRING_LEN);
}

// Re-enable previously disabled compiler warnings
///////////////////////////////////////////////////
#if defined(_MSC_VER)
#pragma warning ( default: 4127 )
#endif // _MSC_VER

