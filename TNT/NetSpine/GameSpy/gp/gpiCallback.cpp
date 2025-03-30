/*
gpiCallback.c
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
#include "gpi.h"

//FUNCTIONS
///////////
void
gpiCallErrorCallback(
  GPConnection * connection,
  GPResult result,
  GPEnum fatal
)
{
	GPICallback callback;
	GPIConnection * iconnection = (GPIConnection*)*connection;

	assert(iconnection != NULL);
	assert(result != GP_NO_ERROR);
	assert((fatal == GP_FATAL) || (fatal == GP_NON_FATAL));

	if(fatal == GP_FATAL)
		iconnection->fatalError = GPITrue;

	callback = iconnection->callbacks[GPI_ERROR];
	if(callback.callback != NULL)
	{
		GPErrorArg * arg;
		arg = (GPErrorArg *)gsimalloc(sizeof(GPErrorArg));

		if(arg != NULL)
		{
			arg->result = result;
			arg->fatal = fatal;
			arg->errorCode = iconnection->errorCode;
			arg->errorString = iconnection->errorString;
		}

		gpiAddCallback(connection, callback, arg, NULL, GPI_ADD_ERROR);
	}
}

GPResult
gpiAddCallback(
  GPConnection * connection,
  GPICallback callback,
  void * arg,
  GPIOperation * operation,
  int type
)
{
	GPICallbackData * data;
	GPIConnection * iconnection = (GPIConnection*)*connection;

	// Allocate the callback data.
	//////////////////////////////
	data = (GPICallbackData *)gsimalloc(sizeof(GPICallbackData));
	if(data == NULL)
		Error(connection, GP_MEMORY_ERROR, "Out of memory.");
	data->callback = callback;
	data->arg = arg;
	if(operation != NULL)
		data->operationID = operation->id;
	else
		data->operationID = 0;
	data->type = type;
	data->pnext = NULL;

	// Update the list.
	///////////////////
	if(iconnection->callbackList == NULL)
		iconnection->callbackList = data;
	if(iconnection->lastCallback != NULL)
		iconnection->lastCallback->pnext = data;
	iconnection->lastCallback = data;

	return GP_NO_ERROR;
}

static void
gpiCallCallback(
  GPConnection * connection,
  GPICallbackData * data
)
{
	// Call the callback.
	/////////////////////
	assert(data->callback.callback != NULL);
	assert(data->arg != NULL);
	data->callback.callback(connection, data->arg, data->callback.param);
	if(data->type == GPI_ADD_MESSAGE)
	{
		freeclear(((GPRecvBuddyMessageArg *)data->arg)->message);
	}
	else if(data->type == GPI_ADD_NICKS)
	{
		int i;
		GPGetUserNicksResponseArg * arg = (GPGetUserNicksResponseArg *)data->arg;

		for(i = 0 ; i < arg->numNicks ; i++)
			freeclear(arg->nicks[i]);
		freeclear(arg->nicks);
	}
	else if(data->type == GPI_ADD_PMATCH)
	{
		GPFindPlayersResponseArg * arg = (GPFindPlayersResponseArg *)data->arg;

		freeclear(arg->matches);
	}
	else if(data->type == GPI_ADD_TRANSFER_CALLBACK)
	{
		GPTransferCallbackArg * arg = (GPTransferCallbackArg *)data->arg;

		if(arg->message)
			freeclear(arg->message);
	}
	else if(data->type == GPI_ADD_REVERSE_BUDDIES)
	{
		GPGetReverseBuddiesResponseArg * arg = (GPGetReverseBuddiesResponseArg *)data->arg;

		if(arg->profiles)
			freeclear(arg->profiles);
	}
	freeclear(data->arg);
	freeclear(data);
}

GPResult
gpiProcessCallbacks(
  GPConnection * connection,
  int blockingOperationID
)
{
	GPIConnection * iconnection = (GPIConnection*)*connection;
	GPICallbackData * list;
	GPICallbackData * last;
	GPICallbackData * pcurr;
	GPICallbackData * pnext;
	GPICallbackData * pprev;

	if(blockingOperationID != 0)
	{
		list = iconnection->callbackList;
		last = iconnection->lastCallback;
		iconnection->callbackList = NULL;
		iconnection->lastCallback = NULL;

		pprev = NULL;
		for(pcurr = list ; pcurr != NULL ; )
		{
			pnext = pcurr->pnext;

			if((pcurr->operationID == blockingOperationID) || (pcurr->type == GPI_ADD_ERROR))
			{
				// Take this one out of the list.
				/////////////////////////////////
				if(pprev != NULL)
					pprev->pnext = pcurr->pnext;
				else
					list = pcurr->pnext;
				if(last == pcurr)
					last = pprev;
	
				// Call the callback.
				/////////////////////
				gpiCallCallback(connection, pcurr);
			}
			else
			{
				pprev = pcurr;
			}

			pcurr = pnext;
		}

		// Were callbacks added within the callback?
		////////////////////////////////////////////
		if(iconnection->callbackList != NULL)
		{
			iconnection->lastCallback->pnext = list;
			iconnection->lastCallback = last;
		}
		else
		{
			// Reset the list.
			//////////////////
			iconnection->callbackList = list;
			iconnection->lastCallback = last;
		}

		return GP_NO_ERROR;
	}

	while(iconnection->callbackList != NULL)
	{
		list = iconnection->callbackList;
		iconnection->callbackList = NULL;
		iconnection->lastCallback = NULL;

		for(pcurr = list ; pcurr != NULL ; pcurr = pnext)
		{
			pnext = pcurr->pnext;
			
			// Call the callback.
			/////////////////////
			gpiCallCallback(connection, pcurr);
		}
	}

	return GP_NO_ERROR;
}
