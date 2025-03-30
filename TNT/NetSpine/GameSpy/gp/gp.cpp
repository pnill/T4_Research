/*
gp.c
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

//FUNCTIONS
///////////
GPResult gpInitialize(
  GPConnection * connection,
  int productID
)
{
	// Error check.
	///////////////
	if(connection == NULL)
		return GP_PARAMETER_ERROR;

	return gpiInitialize(connection, productID);
}

void gpDestroy(
  GPConnection * connection
)
{
	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return;

	gpiDestroy(connection);
}

GPResult gpEnable(
  GPConnection * connection, 
  GPEnum state
)
{
	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	return gpiEnable(connection, state);
}

GPResult gpDisable(
  GPConnection * connection, 
  GPEnum state
)
{
	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	return gpiDisable(connection, state);
}

GPResult gpProcess(
  GPConnection * connection
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;

	// Check for simulation mode.
	/////////////////////////////
	if(iconnection->simulation)
		return GP_NO_ERROR;
	
	return gpiProcess(connection, 0);
}

GPResult gpSetCallback(
  GPConnection * connection,
  GPEnum func,
  GPCallback callback,
  void * param
)
{
	GPIConnection * iconnection;
	int index;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Find which callback.
	///////////////////////
	index = func;
	if((index < 0) || (index >= GPI_NUM_CALLBACKS))
		Error(connection, GP_PARAMETER_ERROR, "Invalid func.");

	// Set the info.
	////////////////
	iconnection->callbacks[index].callback = callback;
	iconnection->callbacks[index].param = param;

	return GP_NO_ERROR;
}

GPResult gpConnect(
  GPConnection * connection,
  const char nick[GP_NICK_LEN],
  const char email[GP_EMAIL_LEN],
  const char password[GP_PASSWORD_LEN],
  GPEnum firewall,
  GPEnum blocking,
  GPCallback callback,
  void * param
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Check for no callback.
	/////////////////////////
	if(callback == NULL)
		Error(connection, GP_PARAMETER_ERROR, "No callback.");

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
	{
		GPConnectResponseArg arg;
		memset(&arg, 0, sizeof(arg));
		callback(connection, &arg, param);
		return GP_NO_ERROR;
	}

	// Do it.
	/////////
	return gpiConnect(connection, nick, email, password, firewall, GPIFalse, blocking, callback, param);
}

GPResult gpConnectNewUser(
  GPConnection * connection,
  const char nick[GP_NICK_LEN],
  const char email[GP_EMAIL_LEN],
  const char password[GP_PASSWORD_LEN],
  GPEnum firewall,
  GPEnum blocking,
  GPCallback callback,
  void * param
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Check for no callback.
	/////////////////////////
	if(callback == NULL)
		Error(connection, GP_PARAMETER_ERROR, "No callback.");

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
	{
		GPConnectResponseArg arg;
		memset(&arg, 0, sizeof(arg));
		callback(connection, &arg, param);
		return GP_NO_ERROR;
	}

	// Do it.
	/////////
	return gpiConnect(connection, nick, email, password, firewall, GPITrue, blocking, callback, param);
}

void gpDisconnect(
  GPConnection * connection
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
		return;

	gpiDisconnect(connection, GPITrue);
}

GPResult gpIsConnected
(
  GPConnection * connection,
  GPEnum * connected
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Set the flag.
	////////////////
	if(iconnection->connectState == GPI_CONNECTED)
		*connected = GP_CONNECTED;
	else
		*connected = GP_NOT_CONNECTED;

	return GP_NO_ERROR;
}

GPResult gpCheckUser(
  GPConnection * connection,
  const char nick[GP_NICK_LEN],
  const char email[GP_EMAIL_LEN],
  const char password[GP_PASSWORD_LEN],
  GPEnum blocking,
  GPCallback callback,
  void * param
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Check for no callback.
	/////////////////////////
	if(callback == NULL)
		Error(connection, GP_PARAMETER_ERROR, "No callback.");

	// Check the length of the nick.
	////////////////////////////////
	if(strlen(nick) >= GP_NICK_LEN)
		Error(connection, GP_PARAMETER_ERROR, "Nick too long.");

	// Check the length of the email.
	/////////////////////////////////
	if(strlen(email) >= GP_EMAIL_LEN)
		Error(connection, GP_PARAMETER_ERROR, "Email too long.");

	// Check the length of the password.
	////////////////////////////////////
	if(password && (strlen(password) >= GP_PASSWORD_LEN))
		Error(connection, GP_PARAMETER_ERROR, "Password too long.");

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
	{
		GPCheckResponseArg arg;
		memset(&arg, 0, sizeof(arg));
		callback(connection, &arg, param);
		return GP_NO_ERROR;
	}

	// Do the check.
	////////////////
	return gpiCheckUser(connection, nick, email, password, blocking, callback, param);
}

GPResult gpNewUser(
  GPConnection * connection,
  const char nick[GP_NICK_LEN],
  const char email[GP_EMAIL_LEN],
  const char password[GP_PASSWORD_LEN],
  GPEnum blocking,
  GPCallback callback,
  void * param
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Check for no callback.
	/////////////////////////
	if(callback == NULL)
		Error(connection, GP_PARAMETER_ERROR, "No callback.");

	// Check the length of the nick.
	////////////////////////////////
	if(strlen(nick) >= GP_NICK_LEN)
		Error(connection, GP_PARAMETER_ERROR, "Nick too long.");

	// Check the length of the email.
	/////////////////////////////////
	if(strlen(email) >= GP_EMAIL_LEN)
		Error(connection, GP_PARAMETER_ERROR, "Email too long.");

	// Check the length of the password.
	////////////////////////////////////
	if(strlen(password) >= GP_PASSWORD_LEN)
		Error(connection, GP_PARAMETER_ERROR, "Password too long.");

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
	{
		GPNewUserResponseArg arg;
		memset(&arg, 0, sizeof(arg));
		callback(connection, &arg, param);
		return GP_NO_ERROR;
	}

	// Creat the new user.
	//////////////////////
	return gpiNewUser(connection, nick, email, password, blocking, callback, param);
}

GPResult gpGetErrorCode(
  GPConnection * connection,
  GPErrorCode * errorCode
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Error check.
	///////////////
	if(errorCode == NULL)
		return GP_PARAMETER_ERROR;

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Check for simulation mode.
	/////////////////////////////
	if(iconnection->simulation)
	{
		*errorCode = (GPErrorCode)0;
		return GP_NO_ERROR;
	}

	// Set the code.
	////////////////
	*errorCode = iconnection->errorCode;

	return GP_NO_ERROR;
}

GPResult gpGetErrorString(
  GPConnection * connection,
  char errorString[GP_ERROR_STRING_LEN]
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Error check.
	///////////////
	if(errorString == NULL)
		return GP_PARAMETER_ERROR;

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Check for simulation mode.
	/////////////////////////////
	if(iconnection->simulation)
	{
		errorString[0] = '\0';
		return GP_NO_ERROR;
	}

	// Copy the error string.
	/////////////////////////
	strzcpy(errorString, iconnection->errorString, GP_ERROR_STRING_LEN);

	return GP_NO_ERROR;
}

GPResult gpNewProfile(
  GPConnection * connection,
  const char nick[GP_NICK_LEN],
  GPEnum replace,
  GPEnum blocking,
  GPCallback callback,
  void * param
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Check for no callback.
	/////////////////////////
	if(callback == NULL)
		Error(connection, GP_PARAMETER_ERROR, "No callback.");

	// Check for no nick.
	// PANTS|05.18.00
	/////////////////////
	if((nick == NULL) || (nick[0] == '\0'))
		Error(connection, GP_PARAMETER_ERROR, "Invalid nick.");

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
	{
		GPNewProfileResponseArg arg;
		memset(&arg, 0, sizeof(arg));
		callback(connection, &arg, param);
		return GP_NO_ERROR;
	}

	// Check for disconnected.
	//////////////////////////
	if(iconnection->connectState == GPI_DISCONNECTED)
		Error(connection, GP_PARAMETER_ERROR, "The connection has already been disconnected.");

	return gpiNewProfile(connection, nick, replace, blocking, callback, param);
}

GPResult gpDeleteProfile(
  GPConnection * connection
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
		return GP_NO_ERROR;

	// Check for disconnected.
	//////////////////////////
	if(iconnection->connectState == GPI_DISCONNECTED)
		Error(connection, GP_PARAMETER_ERROR, "The connection has already been disconnected.");

	return gpiDeleteProfile(connection);
}

GPResult gpProfileFromID(
  GPConnection * connection, 
  GPProfile * profile, 
  int id
)
{
	// Set the profile.
	// This function is depreciated & may be removed from future versions.
	//////////////////////////////////////////////////////////////////////
	*profile = id;

	return GP_NO_ERROR;
}

// gpIDFromProfile
//////////////////
GPResult gpIDFromProfile(
  GPConnection * connection,
  GPProfile profile,
  int * id
)
{
	
	// ID is the same as GPProfile
	// This function is depreciated & may be removed from future versions.
	//////////////////////////////////////////////////////////////////////
	*id = profile;

	return GP_NO_ERROR;
}

// gpUserIDFromProfile
//////////////////
GPResult gpUserIDFromProfile(
  GPConnection * connection,
  GPProfile profile,
  int * userid
)
{
	GPIConnection * iconnection;
	GPIProfile * pProfile;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
	{
		*userid = 0;
		return GP_NO_ERROR;
	}

	// Get the profile object.
	//////////////////////////
	if(!gpiGetProfile(connection, profile, &pProfile))
		Error(connection, GP_PARAMETER_ERROR, "Invalid profile.");

	// Set the id.
	//////////////
	*userid = pProfile->userId;

	return GP_NO_ERROR;
}


GPResult gpProfileSearch(
  GPConnection * connection,
  const char nick[GP_NICK_LEN],
  const char email[GP_EMAIL_LEN],
  const char firstname[GP_FIRSTNAME_LEN],
  const char lastname[GP_LASTNAME_LEN],
  int icquin,
  GPEnum blocking,
  GPCallback callback,
  void * param
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Check for no callback.
	/////////////////////////
	if(callback == NULL)
		Error(connection, GP_PARAMETER_ERROR, "No callback.");

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
	{
		GPProfileSearchResponseArg arg;
		memset(&arg, 0, sizeof(arg));
		arg.more = GP_DONE;
		callback(connection, &arg, param);
		return GP_NO_ERROR;
	}

	// Do the search.
	/////////////////
	return gpiProfileSearch(connection, nick, email, firstname, lastname, icquin, 0, blocking, callback, param);
}

GPResult gpGetInfo(
  GPConnection * connection,
  GPProfile profile, 
  GPEnum checkCache,
  GPEnum blocking,
  GPCallback callback,
  void * param
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL) || (profile == 0))
		return GP_PARAMETER_ERROR;

	// Check for no callback.
	/////////////////////////
	if(callback == NULL)
		Error(connection, GP_PARAMETER_ERROR, "No callback.");

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
	{
		GPGetInfoResponseArg arg;
		memset(&arg, 0, sizeof(arg));
		callback(connection, &arg, param);
		return GP_NO_ERROR;
	}

	// Check for disconnected.
	//////////////////////////
	if(iconnection->connectState == GPI_DISCONNECTED)
		Error(connection, GP_PARAMETER_ERROR, "The connection has already been disconnected.");

	return gpiGetInfo(connection, profile, checkCache, blocking, callback, param);
}

GPResult gpSetInfoi(
  GPConnection * connection, 
  GPEnum info, 
  int value
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
		return GP_NO_ERROR;

	// Check for disconnected.
	//////////////////////////
	if(iconnection->connectState == GPI_DISCONNECTED)
		Error(connection, GP_PARAMETER_ERROR, "The connection has already been disconnected.");

	return gpiSetInfoi(connection, info, value);
}

GPResult gpSetInfos(
  GPConnection * connection, 
  GPEnum info, 
  const char * value
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
		return GP_NO_ERROR;

	// Check for disconnected.
	//////////////////////////
	if(iconnection->connectState == GPI_DISCONNECTED)
		Error(connection, GP_PARAMETER_ERROR, "The connection has already been disconnected.");

	return gpiSetInfos(connection, info, value);
}

GPResult gpSetInfod(
  GPConnection * connection,
  GPEnum info,
  int day,
  int month,
  int year
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
		return GP_NO_ERROR;

	// Check for disconnected.
	//////////////////////////
	if(iconnection->connectState == GPI_DISCONNECTED)
		Error(connection, GP_PARAMETER_ERROR, "The connection has already been disconnected.");

	return gpiSetInfod(connection, info, day, month, year);
}

GPResult gpSetInfoMask(
  GPConnection * connection,
  GPEnum mask
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
		return GP_NO_ERROR;

	// Check for disconnected.
	//////////////////////////
	if(iconnection->connectState == GPI_DISCONNECTED)
		Error(connection, GP_PARAMETER_ERROR, "The connection has already been disconnected.");

	return gpiSetInfoMask(connection, mask);
}

GPResult gpSendBuddyRequest(
  GPConnection * connection,
  GPProfile profile,
  const char reason[GP_REASON_LEN]
)
{
	GPIConnection * iconnection;
	char reasonFixed[GP_REASON_LEN];
	int i;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Check for simulation mode.
	/////////////////////////////
	if(iconnection->simulation)
		return GP_NO_ERROR;

	// Check for disconnected.
	//////////////////////////
	if(iconnection->connectState == GPI_DISCONNECTED)
		Error(connection, GP_PARAMETER_ERROR, "The connection has already been disconnected.");

	// Error check.
	///////////////
	if(reason == NULL)
		Error(connection, GP_PARAMETER_ERROR, "Invalid reason.");

	// Replace backslashes in reason.
	/////////////////////////////////
	strzcpy(reasonFixed, reason, GP_REASON_LEN);
	for(i = 0 ; reasonFixed[i] ; i++)
		if(reasonFixed[i] == '\\')
			reasonFixed[i] = '/';

	// Send the request.
	////////////////////
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\addbuddy\\");
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\sesskey\\");
	gpiAppendIntToBuffer(connection, &iconnection->outputBuffer, iconnection->sessKey);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\newprofileid\\");
	gpiAppendIntToBuffer(connection, &iconnection->outputBuffer, profile);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\reason\\");
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, reasonFixed);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\final\\");

	return GP_NO_ERROR;
}

GPResult gpAuthBuddyRequest(
  GPConnection * connection,
  GPProfile profile
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
		return GP_NO_ERROR;

	// Check for disconnected.
	//////////////////////////
	if(iconnection->connectState == GPI_DISCONNECTED)
		Error(connection, GP_PARAMETER_ERROR, "The connection has already been disconnected.");

	return gpiAuthBuddyRequest(connection, profile);
}

GPResult gpDenyBuddyRequest(
  GPConnection * connection,
  GPProfile profile
)
{
	GPIConnection * iconnection;
	GPIProfile * pProfile;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
		return GP_NO_ERROR;

	// Check for disconnected.
	//////////////////////////
	if(iconnection->connectState == GPI_DISCONNECTED)
		Error(connection, GP_PARAMETER_ERROR, "The connection has already been disconnected.");

	// Get the profile.
	///////////////////
	if(!gpiGetProfile(connection, profile, &pProfile))
		return GP_NO_ERROR;

	// freeclear the sig if no more requests.
	////////////////////////////////////
	pProfile->requestCount--;
	if(!iconnection->infoCaching && (pProfile->requestCount <= 0))
	{
		freeclear(pProfile->authSig);
		if(gpiCanFreeProfile(pProfile))
			gpiRemoveProfile(connection, pProfile);
	}

	return GP_NO_ERROR;
}

GPResult gpGetNumBuddies(
  GPConnection * connection,
  int * numBuddies
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
	{
		*numBuddies = 0;
		return GP_NO_ERROR;
	}

	// Set the number of buddies.
	/////////////////////////////
	*numBuddies = iconnection->profileList.numBuddies;

	return GP_NO_ERROR;
}

GPResult gpGetBuddyStatus(
  GPConnection * connection,
  int index, 
  GPBuddyStatus * status
)
{
	GPIConnection * iconnection;
	int num;
	GPIProfile * profile;
	GPIBuddyStatus * buddyStatus;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
	{
		memset(status, 0, sizeof(GPBuddyStatus));
		return GP_NO_ERROR;
	}

	// Check for a NULL status.
	///////////////////////////
	if(status == NULL)
		Error(connection, GP_PARAMETER_ERROR, "Invalid status.");

	// Check the buddy index.
	/////////////////////////
	num = iconnection->profileList.numBuddies;
	if((index < 0) || (index >= num))
		Error(connection, GP_PARAMETER_ERROR, "Invalid index.");

	// Find the buddy with this index.
	//////////////////////////////////
	profile = gpiFindBuddy(connection, index);
	if(!profile)
		Error(connection, GP_PARAMETER_ERROR, "Invalid index.");

	buddyStatus = profile->buddyStatus;
	assert(buddyStatus);
	status->profile = (GPProfile)profile->profileId;
	status->status = buddyStatus->status;
	if(buddyStatus->statusString)
		strzcpy(status->statusString, buddyStatus->statusString, GP_STATUS_STRING_LEN);
	else
		buddyStatus->statusString[0] = '\0';
	if(buddyStatus->locationString)
		strzcpy(status->locationString, buddyStatus->locationString, GP_LOCATION_STRING_LEN);
	else
		buddyStatus->locationString[0] = '\0';
	status->ip = buddyStatus->ip;
	status->port = buddyStatus->port;

	return GP_NO_ERROR;
}

GPResult gpGetBuddyIndex(
  GPConnection * connection, 
  GPProfile profile, 
  int * index
)
{
	GPIProfile * pProfile;
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
	{
		*index = 0;
		return GP_NO_ERROR;
	}

	// Get the index.
	/////////////////
	if(gpiGetProfile(connection, profile, &pProfile) && pProfile->buddyStatus)
		*index = pProfile->buddyStatus->buddyIndex;
	else
		*index = -1;

	return GP_NO_ERROR;
}

int gpIsBuddy(
  GPConnection * connection,
  GPProfile profile
)
{
	GPIProfile * pProfile;
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return 0;

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
		return 0;

	// Get the index.
	/////////////////
	if(gpiGetProfile(connection, profile, &pProfile) && pProfile->buddyStatus)
		return 1;

	return 0;
}

GPResult gpDeleteBuddy(
  GPConnection * connection,
  GPProfile profile
)
{
	GPIConnection * iconnection;
	
	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
		return GP_NO_ERROR;

	// Check for disconnected.
	//////////////////////////
	if(iconnection->connectState == GPI_DISCONNECTED)
		Error(connection, GP_PARAMETER_ERROR, "The connection has already been disconnected.");

	// Delete the buddy.
	////////////////////
	CHECK_RESULT(gpiDeleteBuddy(connection, profile));

	return GP_NO_ERROR;
}

GPResult gpSetStatus(
  GPConnection * connection,
  GPEnum status,
  const char statusString[GP_STATUS_STRING_LEN],
  const char locationString[GP_LOCATION_STRING_LEN]
)
{
	char statusStringFixed[GP_STATUS_STRING_LEN];
	char locationStringFixed[GP_LOCATION_STRING_LEN];
	GPIConnection * iconnection;
	int i;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
		return GP_NO_ERROR;

	// Check for disconnected.
	//////////////////////////
	if(iconnection->connectState == GPI_DISCONNECTED)
		Error(connection, GP_PARAMETER_ERROR, "The connection has already been disconnected.");

	// Error check.
	///////////////
	if(statusString == NULL)
		Error(connection, GP_PARAMETER_ERROR, "Invalid statusString.");
	if(locationString == NULL)
		Error(connection, GP_PARAMETER_ERROR, "Invalid locationString.");

	// Replace backslashes with slashes.
	////////////////////////////////////
	strcpy(statusStringFixed, statusString);
	for(i = 0 ; statusStringFixed[i] ; i++)
		if(statusStringFixed[i] == '\\')
			statusStringFixed[i] = '/';
	strcpy(locationStringFixed, locationString);
	for(i = 0 ; locationStringFixed[i] ; i++)
		if(locationStringFixed[i] == '\\')
			locationStringFixed[i] = '/';

	// Don't send it if its the same as the previous.
	/////////////////////////////////////////////////
	if((status == iconnection->lastStatus) &&
	   (strcmp(statusStringFixed, iconnection->lastStatusString) == 0) &&
	   (strcmp(locationStringFixed, iconnection->lastLocationString) == 0))
	{
		return GP_NO_ERROR;
	}

	// Copy off the new status.
	///////////////////////////
	iconnection->lastStatus = status;
	strzcpy(iconnection->lastStatusString, statusStringFixed, GP_STATUS_STRING_LEN);
	strzcpy(iconnection->lastLocationString, locationStringFixed, GP_LOCATION_STRING_LEN);

	// Send the new status.
	///////////////////////
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\status\\");
	gpiAppendIntToBuffer(connection, &iconnection->outputBuffer, status);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\sesskey\\");
	gpiAppendIntToBuffer(connection, &iconnection->outputBuffer, iconnection->sessKey);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\statstring\\");
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, statusStringFixed);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\locstring\\");
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, locationStringFixed);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\final\\");

	return GP_NO_ERROR;
}

GPResult gpSendBuddyMessage(
  GPConnection * connection,
  GPProfile profile,
  const char * message
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
		return GP_NO_ERROR;

	// Check for disconnected.
	//////////////////////////
	if(iconnection->connectState == GPI_DISCONNECTED)
		Error(connection, GP_PARAMETER_ERROR, "The connection has already been disconnected.");

	// Error check.
	///////////////
	if(message == NULL)
		Error(connection, GP_PARAMETER_ERROR, "Invalid message.");

	return gpiSendBuddyMessage(connection, profile, 1, message);
}

GPResult gpIsValidEmail(
  GPConnection * connection,
  const char email[GP_EMAIL_LEN],
  GPEnum blocking,
  GPCallback callback,
  void * param
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Check for no callback.
	/////////////////////////
	if(callback == NULL)
		Error(connection, GP_PARAMETER_ERROR, "No callback.");

	// Check the length of the email.
	/////////////////////////////////
	if(strlen(email) >= GP_EMAIL_LEN)
		Error(connection, GP_PARAMETER_ERROR, "Email too long.");

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
	{
		GPIsValidEmailResponseArg arg;
		memset(&arg, 0, sizeof(arg));
		strzcpy(arg.email, email, GP_EMAIL_LEN);
		arg.isValid = GP_INVALID;
		callback(connection, &arg, param);
		return GP_NO_ERROR;
	}

	// Do the validation.
	/////////////////////
	return gpiIsValidEmail(connection, email, blocking, callback, param);
}

GPResult gpGetUserNicks(
  GPConnection * connection,
  const char email[GP_EMAIL_LEN],
  const char password[GP_PASSWORD_LEN],
  GPEnum blocking,
  GPCallback callback,
  void * param
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Check for no callback.
	/////////////////////////
	if(callback == NULL)
		Error(connection, GP_PARAMETER_ERROR, "No callback.");

	// Check the length of the email.
	/////////////////////////////////
	if(strlen(email) >= GP_EMAIL_LEN)
		Error(connection, GP_PARAMETER_ERROR, "Email too long.");

	// Check the length of the password.
	////////////////////////////////////
	if(strlen(password) >= GP_PASSWORD_LEN)
		Error(connection, GP_PARAMETER_ERROR, "Password too long.");

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
	{
		GPGetUserNicksResponseArg arg;
		memset(&arg, 0, sizeof(arg));
		strzcpy(arg.email, email, GP_EMAIL_LEN);
		callback(connection, &arg, param);
		return GP_NO_ERROR;
	}

	// Do the validation.
	/////////////////////
	return gpiGetUserNicks(connection, email, password, blocking, callback, param);
}

GPResult gpSetInvitableGames(
  GPConnection * connection,
  int numProductIDs,
  int * productIDs
)
{
	GPIConnection * iconnection;
	int i;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
		return GP_NO_ERROR;

	// Check for disconnected.
	//////////////////////////
	if(iconnection->connectState == GPI_DISCONNECTED)
		Error(connection, GP_PARAMETER_ERROR, "The connection has already been disconnected.");

	// Error check.
	///////////////
	if(numProductIDs < 0)
		Error(connection, GP_PARAMETER_ERROR, "Invalid numProductIDs.");
	if((numProductIDs > 0) && (productIDs == NULL))
		Error(connection, GP_PARAMETER_ERROR, "Invalid productIDs.");

	// Send the list.
	/////////////////
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\inviteto\\");
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\sesskey\\");
	gpiAppendIntToBuffer(connection, &iconnection->outputBuffer, iconnection->sessKey);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\products\\");
	for(i = 0 ; i < numProductIDs ; i++)
	{
		gpiAppendIntToBuffer(connection, &iconnection->outputBuffer, productIDs[i]);
		if(i < (numProductIDs - 1))
			gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, ",");
	}
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\final\\");

	return GP_NO_ERROR;
}

GPResult gpFindPlayers(
  GPConnection * connection,
  int productID,
  GPEnum blocking,
  GPCallback callback,
  void * param
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Check for no callback.
	/////////////////////////
	if(callback == NULL)
		Error(connection, GP_PARAMETER_ERROR, "No callback.");

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Check for disconnected.
	//////////////////////////
	if(iconnection->connectState == GPI_DISCONNECTED)
		Error(connection, GP_PARAMETER_ERROR, "The connection has already been disconnected.");

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
	{
		GPFindPlayersResponseArg arg;
		memset(&arg, 0, sizeof(arg));
		arg.productID = productID;
		arg.numMatches = 0;
		arg.matches = NULL;
		callback(connection, &arg, param);
		return GP_NO_ERROR;
	}

	// Start the find.
	//////////////////
	return gpiFindPlayers(connection, productID, blocking, callback, param);
}

GPResult gpInvitePlayer(
  GPConnection * connection,
  GPProfile profile,
  int productID
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
		return GP_NO_ERROR;

	// Check for disconnected.
	//////////////////////////
	if(iconnection->connectState == GPI_DISCONNECTED)
		Error(connection, GP_PARAMETER_ERROR, "The connection has already been disconnected.");

	// Send the invite.
	///////////////////
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\pinvite\\");
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\sesskey\\");
	gpiAppendIntToBuffer(connection, &iconnection->outputBuffer, iconnection->sessKey);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\profileid\\");
	gpiAppendIntToBuffer(connection, &iconnection->outputBuffer, profile);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\productid\\");
	gpiAppendIntToBuffer(connection, &iconnection->outputBuffer, productID);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\final\\");

	return GP_NO_ERROR;
}

GPResult gpGetReverseBuddies(
  GPConnection * connection,
  GPEnum blocking,
  GPCallback callback,
  void * param
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Check for no callback.
	/////////////////////////
	if(callback == NULL)
		Error(connection, GP_PARAMETER_ERROR, "No callback.");

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Check for disconnected.
	//////////////////////////
	if(iconnection->connectState == GPI_DISCONNECTED)
		Error(connection, GP_PARAMETER_ERROR, "The connection has already been disconnected.");

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
	{
		GPGetReverseBuddiesResponseArg arg;
		memset(&arg, 0, sizeof(arg));
		callback(connection, &arg, param);
		return GP_NO_ERROR;
	}

	// Start the search.
	////////////////////
	return gpiOthersBuddy(connection, blocking, callback, param);
}

GPResult gpRevokeBuddyAuthorization(
  GPConnection * connection,
  GPProfile profile
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
		return GP_NO_ERROR;

	// Check for disconnected.
	//////////////////////////
	if(iconnection->connectState == GPI_DISCONNECTED)
		Error(connection, GP_PARAMETER_ERROR, "The connection has already been disconnected.");

	// Send the invite.
	///////////////////
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\revoke\\");
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\sesskey\\");
	gpiAppendIntToBuffer(connection, &iconnection->outputBuffer, iconnection->sessKey);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\profileid\\");
	gpiAppendIntToBuffer(connection, &iconnection->outputBuffer, profile);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\final\\");

	return GP_NO_ERROR;
}

#ifndef NOFILE
void gpSetInfoCacheFilename(
  const char * filename
)
{
	gpiSetInfoCacheFilename(filename);
}

static GPResult gpiAddSendingFile(
  GPConnection * connection,
  GPITransfer * transfer,
  const char * path,
  const char * name
)
{
	GPIFile * file = NULL;
	int size = 0;
	unsigned long modTime = 0;

	// Check for a bad path or name.
	////////////////////////////////
	if(!path && !name)
		Error(connection, GP_PARAMETER_ERROR, "File missing path and name.");
	if(path && !path[0])
		Error(connection, GP_PARAMETER_ERROR, "Empty path.");
	if(name && !name[0])
		Error(connection, GP_PARAMETER_ERROR, "Empty name.");

	// Check that the file exists and is readable.
	//////////////////////////////////////////////
	if(path)
	{
		FILE * fileVerify;

		fileVerify = fopen(path, "r");
		if(!fileVerify)
			Error(connection, GP_PARAMETER_ERROR, "Can't find file.");

		if(!gpiGetTransferFileInfo(fileVerify, &size, &modTime))
		{
			fclose(fileVerify);
			Error(connection, GP_PARAMETER_ERROR, "Can't get info on file.");
		}

		fclose(fileVerify);
	}

	// Validate the name.
	/////////////////////
	if(name)
	{
		size_t len;

		len = strlen(name);

		if(strstr(name, "//") || strstr(name, "\\\\"))
			Error(connection, GP_PARAMETER_ERROR, "Empty directory in filename.");
		if(strstr(name, "./") || strstr(name, ".\\") || (name[len - 1] == '.'))
			Error(connection, GP_PARAMETER_ERROR, "Directory level in filename.");
		if((name[0] == '/') || (name[0] == '\\'))
			Error(connection, GP_PARAMETER_ERROR, "Filename can't start with a slash.");
		if(strcspn(name, ":*?\"<>|\n") != len)
			Error(connection, GP_PARAMETER_ERROR, "Invalid character in filename.");
	}
	// The name is the path's title.
	////////////////////////////////
	else
	{
		char * str;

		// Find the end of the path.
		////////////////////////////
		name = strrchr(path, '/');
		str = strrchr(path, '\\');
		if(str > name)
			name = str;

		// Point the name at the title.
		///////////////////////////////
		if(name)
			name++;
		else
			name = path;
	}

	// Add this to the list.
	////////////////////////
	file = gpiAddFileToTransfer(transfer, path, name);
	if(!file)
		Error(connection, GP_MEMORY_ERROR, "Out of memory.");

	// Set the size and time.
	/////////////////////////
	file->size = size;
	file->modTime = modTime;

	// Update the total size.
	/////////////////////////
	transfer->totalSize += size;

	return GP_NO_ERROR;
}

GPResult gpSendFiles(
  GPConnection * connection,
  GPTransfer * transfer,
  GPProfile profile,
  const char * message,
  gpSendFilesCallback callback,
  void * param
)
{
	GPIConnection * iconnection;
	GPITransfer * pTransfer;
	GPResult result;
	const char * path;
	const char * name;
	int numFiles;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Check for simulation mode.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;
	if(iconnection->simulation)
		Error(connection, GP_PARAMETER_ERROR, "Cannot send files in simulation mode.");

	// Check for disconnected.
	//////////////////////////
	if(iconnection->connectState == GPI_DISCONNECTED)
		Error(connection, GP_PARAMETER_ERROR, "The connection has already been disconnected.");

	// Check other stuff.
	/////////////////////
	if(!callback)
		Error(connection, GP_PARAMETER_ERROR, "No callback.");
	if(!iconnection->callbacks[GPI_TRANSFER_CALLBACK].callback)
		Error(connection, GP_PARAMETER_ERROR, "No callback.");

	// No message is an empty message.
	//////////////////////////////////
	if(!message)
		message = "";

	// Create the transfer object.
	//////////////////////////////
	CHECK_RESULT(gpiNewSenderTransfer(connection, &pTransfer, profile));

	// Fill in the message.
	///////////////////////
	pTransfer->message = goastrdup(message);
	if(!pTransfer->message)
	{
		gpiFreeTransfer(connection, pTransfer);
		Error(connection, GP_MEMORY_ERROR, "Out of memory.");
	}

	// Add all the files.
	/////////////////////
	numFiles = 0;
	do
	{
		path = NULL;
		name = NULL;
		callback(connection, numFiles++, &path, &name, param);
		if(path && !path[0])
			path = NULL;
		if(name && !name[0])
			name = NULL;

		if(name || path)
		{
			result = gpiAddSendingFile(connection, pTransfer, path, name);
			if(result != GP_NO_ERROR)
			{
				gpiFreeTransfer(connection, pTransfer);
				return result;
			}
		}
	}
	while(name || path);

	// Check that we got at least 1 file.
	/////////////////////////////////////
	if(!ArrayLength(pTransfer->files))
	{
		gpiFreeTransfer(connection, pTransfer);
		Error(connection, GP_PARAMETER_ERROR, "No files to send.");
	}

	// Ping the receiver.
	/////////////////////
	result = gpiSendBuddyMessage(connection, profile, GPI_BM_PING, "1");
	if(result != GP_NO_ERROR)
	{
		gpiFreeTransfer(connection, pTransfer);
		return result;
	}

	// Successful so far.
	/////////////////////
	if(transfer)
		*transfer = pTransfer->localID;

	return GP_NO_ERROR;
}

GPResult gpAcceptTransfer(
  GPConnection * connection,
  GPTransfer transfer,
  const char * message
)
{
	GPITransfer * pTransfer;
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;

	// Get the transfer.
	////////////////////
	pTransfer = gpiFindTransferByLocalID(connection, transfer);
	if(!pTransfer)
		Error(connection, GP_PARAMETER_ERROR, "Invalid transfer.");

	// Check that we have a directory set.
	//////////////////////////////////////
	if(!pTransfer->baseDirectory)
		Error(connection, GP_PARAMETER_ERROR, "No transfer directory set.");

	// Check if this transfer has been cancelled.
	/////////////////////////////////////////////
	if(pTransfer->state & GPITransferCancelled)
		Error(connection, GP_PARAMETER_ERROR, "Transfer already cancelled.");

	// Send a reply.
	////////////////
	CHECK_RESULT(gpiSendTransferReply(connection, &pTransfer->transferID, pTransfer->peer, GPI_ACCEPTED, message));

	// We're now transferring.
	//////////////////////////
	pTransfer->state = GPITransferTransferring;

	// Set the current file index to the first file.
	////////////////////////////////////////////////
	pTransfer->currentFile = 0;

	return GP_NO_ERROR;
}

GPResult gpRejectTransfer(
  GPConnection * connection,
  GPTransfer transfer,
  const char * message
)
{
	GPITransfer * pTransfer;

	// Get the transfer.
	////////////////////
	pTransfer = gpiFindTransferByLocalID(connection, transfer);
	if(!pTransfer)
		return GP_NO_ERROR;

	// Check if this transfer has been cancelled.
	/////////////////////////////////////////////
	if(pTransfer->state & GPITransferCancelled)
		return GP_NO_ERROR;

	// Send the reply.
	//////////////////
	gpiSendTransferReply(connection, &pTransfer->transferID, pTransfer->peer, GPI_REJECTED, message);

	// Free the transfer.
	/////////////////////
	gpiFreeTransfer(connection, pTransfer);

	return GP_NO_ERROR;
}

GPResult gpFreeTransfer(
  GPConnection * connection,
  GPTransfer transfer
)
{
	GPITransfer * pTransfer;

	// Get the transfer.
	////////////////////
	pTransfer = gpiFindTransferByLocalID(connection, transfer);
	if(!pTransfer)
		return GP_NO_ERROR;

	// Check if this should be a reject.
	////////////////////////////////////
	if(!pTransfer->sender && (pTransfer->state == GPITransferWaiting))
		return gpRejectTransfer(connection, transfer, NULL);

	// Check for cancelling.
	////////////////////////
	if(pTransfer->state < GPITransferComplete)
		gpiCancelTransfer(connection, pTransfer);

	// Free the transfer.
	/////////////////////
	gpiFreeTransfer(connection, pTransfer);

	return GP_NO_ERROR;
}

GPResult gpSetTransferData(
  GPConnection * connection,
  GPTransfer transfer,
  void * userData
)
{
	GPITransfer * pTransfer;

	// Get the transfer.
	////////////////////
	pTransfer = gpiFindTransferByLocalID(connection, transfer);
	if(!pTransfer)
		Error(connection, GP_PARAMETER_ERROR, "Invalid transfer.");

	// Set the data.
	////////////////
	pTransfer->userData = userData;

	return GP_NO_ERROR;
}

void * gpGetTransferData(
  GPConnection * connection,
  GPTransfer transfer
)
{
	GPITransfer * pTransfer;

	// Get the transfer.
	////////////////////
	pTransfer = gpiFindTransferByLocalID(connection, transfer);
	if(!pTransfer)
		return NULL;

	// Return the data.
	///////////////////
	return pTransfer->userData;
}

GPResult gpSetTransferDirectory(
  GPConnection * connection,
  GPTransfer transfer,
  const char * directory
)
{
	GPITransfer * pTransfer;
	char lastChar;

	if(!directory || !directory[0])
		Error(connection, GP_PARAMETER_ERROR, "Invalid directory.");
	lastChar = directory[strlen(directory) - 1];
	if((lastChar != '\\') && (lastChar != '/'))
		Error(connection, GP_PARAMETER_ERROR, "Invalid directory.");

	// Get the transfer.
	////////////////////
	pTransfer = gpiFindTransferByLocalID(connection, transfer);
	if(!pTransfer)
		Error(connection, GP_PARAMETER_ERROR, "Invalid transfer.");

	// This has to be set before transferring.
	//////////////////////////////////////////
	if(pTransfer->sender)
		Error(connection, GP_PARAMETER_ERROR, "Sender has no transfer directory.");
	if(pTransfer->state != GPITransferWaiting)
		Error(connection, GP_PARAMETER_ERROR, "Can only set transfer directory before transferring.");

	// Free any existing directory.
	///////////////////////////////
	if(pTransfer->baseDirectory)
		gsifree(pTransfer->baseDirectory);
	pTransfer->baseDirectory = NULL;

	// Set the directory.
	/////////////////////
	pTransfer->baseDirectory = goastrdup(directory);
	if(!pTransfer->baseDirectory)
		Error(connection, GP_MEMORY_ERROR, "Out of memory.");

	return GP_NO_ERROR;
}

GPResult gpSetTransferThrottle(
  GPConnection * connection,
  GPTransfer transfer,
  int throttle
)
{
	GPITransfer * pTransfer;
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;

	// Negative means no throttle.
	//////////////////////////////
	if(throttle < 0)
		throttle = -1;

	// Get the transfer.
	////////////////////
	pTransfer = gpiFindTransferByLocalID(connection, transfer);
	if(!pTransfer)
		Error(connection, GP_PARAMETER_ERROR, "Invalid transfer.");

	// Store the throttle setting.
	//////////////////////////////
	pTransfer->throttle = throttle;

	// Send the rate.
	/////////////////
	CHECK_RESULT(gpiPeerStartTransferMessage(connection, pTransfer->peer, GPI_BM_FILE_TRANSFER_THROTTLE, (GPITransferID_st)&pTransfer->transferID));
	gpiSendOrBufferString(connection, pTransfer->peer, "\\rate\\");
	gpiSendOrBufferInt(connection, pTransfer->peer, throttle);
	gpiPeerFinishTransferMessage(connection, pTransfer->peer, NULL, 0);

	// If we're the sender, call the callback.
	//////////////////////////////////////////
	if(pTransfer->sender)
	{
		GPTransferCallbackArg * arg;

		// Call the callback.
		/////////////////////
		arg = (GPTransferCallbackArg *)gsimalloc(sizeof(GPTransferCallbackArg));
		if(arg)
		{
			memset(arg, 0, sizeof(GPTransferCallbackArg));
			arg->transfer = pTransfer->localID;
			arg->type = GP_TRANSFER_THROTTLE;
			arg->num = throttle;
			gpiAddCallback(connection, iconnection->callbacks[GPI_TRANSFER_CALLBACK], arg, NULL, GPI_ADD_TRANSFER_CALLBACK);
		}
	}

	return GP_NO_ERROR;
}

GPResult gpGetTransferThrottle(
  GPConnection * connection,
  GPTransfer transfer,
  int * throttle
)
{
	GPITransfer * pTransfer;

	// Get the transfer.
	////////////////////
	pTransfer = gpiFindTransferByLocalID(connection, transfer);
	if(!pTransfer)
		Error(connection, GP_PARAMETER_ERROR, "Invalid transfer.");

	// Get the throttle.
	////////////////////
	*throttle = pTransfer->throttle;

	return GP_NO_ERROR;
}

GPResult gpGetTransferProfile(
  GPConnection * connection,
  GPTransfer transfer,
  GPProfile * profile
)
{
	GPITransfer * pTransfer;

	// Get the transfer.
	////////////////////
	pTransfer = gpiFindTransferByLocalID(connection, transfer);
	if(!pTransfer)
		Error(connection, GP_PARAMETER_ERROR, "Invalid transfer.");

	// Get the profile.
	///////////////////
	*profile = pTransfer->profile;

	return GP_NO_ERROR;
}

GPResult gpGetTransferSide(
  GPConnection * connection,
  GPTransfer transfer,
  GPEnum * side
)
{
	GPITransfer * pTransfer;

	// Get the transfer.
	////////////////////
	pTransfer = gpiFindTransferByLocalID(connection, transfer);
	if(!pTransfer)
		Error(connection, GP_PARAMETER_ERROR, "Invalid transfer.");

	// Get the side.
	////////////////
	if(pTransfer->sender)
		*side = GP_TRANSFER_SENDER;
	else
		*side = GP_TRANSFER_RECEIVER;

	return GP_NO_ERROR;
}

GPResult gpGetTransferSize(
  GPConnection * connection,
  GPTransfer transfer,
  int * size
)
{
	GPITransfer * pTransfer;

	// Get the transfer.
	////////////////////
	pTransfer = gpiFindTransferByLocalID(connection, transfer);
	if(!pTransfer)
		Error(connection, GP_PARAMETER_ERROR, "Invalid transfer.");

	// Get the size.
	////////////////
	*size = pTransfer->totalSize;

	return GP_NO_ERROR;
}

GPResult gpGetTransferProgress(
  GPConnection * connection,
  GPTransfer transfer,
  int * progress
)
{
	GPITransfer * pTransfer;

	// Get the transfer.
	////////////////////
	pTransfer = gpiFindTransferByLocalID(connection, transfer);
	if(!pTransfer)
		Error(connection, GP_PARAMETER_ERROR, "Invalid transfer.");

	// Get the progress.
	////////////////////
	*progress = pTransfer->progress;

	return GP_NO_ERROR;
}

GPResult gpGetNumFiles(
  GPConnection * connection,
  GPTransfer transfer,
  int * num
)
{
	GPITransfer * pTransfer;

	// Get the transfer.
	////////////////////
	pTransfer = gpiFindTransferByLocalID(connection, transfer);
	if(!pTransfer)
		Error(connection, GP_PARAMETER_ERROR, "Invalid transfer.");

	// Get the progress.
	////////////////////
	*num = ArrayLength(pTransfer->files);

	return GP_NO_ERROR;
}

GPResult gpGetCurrentFile(
  GPConnection * connection,
  GPTransfer transfer,
  int * index
)
{
	GPITransfer * pTransfer;

	// Get the transfer.
	////////////////////
	pTransfer = gpiFindTransferByLocalID(connection, transfer);
	if(!pTransfer)
		Error(connection, GP_PARAMETER_ERROR, "Invalid transfer.");

	// Get the current file.
	////////////////////////
	*index = pTransfer->currentFile;

	return GP_NO_ERROR;
}

GPResult gpSkipFile(
  GPConnection * connection,
  GPTransfer transfer,
  int index
)
{
	GPIFile * file;
	GPITransfer * pTransfer;
	GPTransferCallbackArg * arg;
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection*)*connection;

	// Get the transfer.
	////////////////////
	pTransfer = gpiFindTransferByLocalID(connection, transfer);
	if(!pTransfer)
		Error(connection, GP_PARAMETER_ERROR, "Invalid transfer.");

	// Get the file.
	////////////////
	if((index < 0) || (index >= ArrayLength(pTransfer->files)))
		Error(connection, GP_PARAMETER_ERROR, "Invalid index.");
	file = (GPIFile *)ArrayNth(pTransfer->files, index);

	// Are we already past this file?
	/////////////////////////////////
	if(index < pTransfer->currentFile)
		return GP_NO_ERROR;

	// Did we not get to this file yet?
	///////////////////////////////////
	if(pTransfer->currentFile != index)
	{
		// Mark it.
		///////////
		file->flags |= GPI_FILE_SKIP;

		// If we're receiving, let the sender know we want to skip it.
		//////////////////////////////////////////////////////////////
		if(!pTransfer->sender)
			gpiSkipFile(connection, pTransfer, index, GPI_SKIP_USER_SKIP);

		return GP_NO_ERROR;
	}

	// If we're receiving, delete our temp file.
	////////////////////////////////////////////
	if(!pTransfer->sender && (index == pTransfer->currentFile) && file->file)
	{
		fclose(file->file);
		file->file = NULL;
		remove(file->path);
	}

	// Skip the current file.
	/////////////////////////
	gpiSkipCurrentFile(connection, pTransfer, GPI_SKIP_USER_SKIP);

	// Call the callback.
	/////////////////////
	arg = (GPTransferCallbackArg *)gsimalloc(sizeof(GPTransferCallbackArg));
	if(arg)
	{
		memset(arg, 0, sizeof(GPTransferCallbackArg));
		arg->transfer = pTransfer->localID;
		arg->index = index;
		arg->type = GP_FILE_SKIP;
		gpiAddCallback(connection, iconnection->callbacks[GPI_TRANSFER_CALLBACK], arg, NULL, GPI_ADD_TRANSFER_CALLBACK);
	}

	return GP_NO_ERROR;
}

GPResult gpGetFileName(
  GPConnection * connection,
  GPTransfer transfer,
  int index,
  char ** name
)
{
	GPIFile * file;
	GPITransfer * pTransfer;

	// Get the transfer.
	////////////////////
	pTransfer = gpiFindTransferByLocalID(connection, transfer);
	if(!pTransfer)
		Error(connection, GP_PARAMETER_ERROR, "Invalid transfer.");

	// Get the file.
	////////////////
	if((index < 0) || (index >= ArrayLength(pTransfer->files)))
		Error(connection, GP_PARAMETER_ERROR, "Invalid index.");
	file = (GPIFile *)ArrayNth(pTransfer->files, index);

	// Get the name.
	////////////////
	*name = file->name;

	return GP_NO_ERROR;
}

GPResult gpGetFilePath(
  GPConnection * connection,
  GPTransfer transfer,
  int index,
  char ** path
)
{
	GPIFile * file;
	GPITransfer * pTransfer;

	// Get the transfer.
	////////////////////
	pTransfer = gpiFindTransferByLocalID(connection, transfer);
	if(!pTransfer)
		Error(connection, GP_PARAMETER_ERROR, "Invalid transfer.");

	// Get the file.
	////////////////
	if((index < 0) || (index >= ArrayLength(pTransfer->files)))
		Error(connection, GP_PARAMETER_ERROR, "Invalid index.");
	file = (GPIFile *)ArrayNth(pTransfer->files, index);

	// Get the path.
	////////////////
	*path = file->path;

	return GP_NO_ERROR;
}

GPResult gpGetFileSize(
  GPConnection * connection,
  GPTransfer transfer,
  int index,
  int * size
)
{
	GPIFile * file;
	GPITransfer * pTransfer;

	// Get the transfer.
	////////////////////
	pTransfer = gpiFindTransferByLocalID(connection, transfer);
	if(!pTransfer)
		Error(connection, GP_PARAMETER_ERROR, "Invalid transfer.");

	// Get the file.
	////////////////
	if((index < 0) || (index >= ArrayLength(pTransfer->files)))
		Error(connection, GP_PARAMETER_ERROR, "Invalid index.");
	file = (GPIFile *)ArrayNth(pTransfer->files, index);

	// Get the size.
	////////////////
	*size = file->size;

	return GP_NO_ERROR;
}

GPResult gpGetFileProgress(
  GPConnection * connection,
  GPTransfer transfer,
  int index,
  int * progress
)
{
	GPIFile * file;
	GPITransfer * pTransfer;

	// Get the transfer.
	////////////////////
	pTransfer = gpiFindTransferByLocalID(connection, transfer);
	if(!pTransfer)
		Error(connection, GP_PARAMETER_ERROR, "Invalid transfer.");

	// Get the file.
	////////////////
	if((index < 0) || (index >= ArrayLength(pTransfer->files)))
		Error(connection, GP_PARAMETER_ERROR, "Invalid index.");
	file = (GPIFile *)ArrayNth(pTransfer->files, index);

	// Get the progress.
	////////////////////
	*progress = file->progress;

	return GP_NO_ERROR;
}

GPResult gpGetFileModificationTime(
  GPConnection * connection,
  GPTransfer transfer,
  int index,
  unsigned long * modTime
)
{
	GPIFile * file;
	GPITransfer * pTransfer;

	// Get the transfer.
	////////////////////
	pTransfer = gpiFindTransferByLocalID(connection, transfer);
	if(!pTransfer)
		Error(connection, GP_PARAMETER_ERROR, "Invalid transfer.");

	// Get the file.
	////////////////
	if((index < 0) || (index >= ArrayLength(pTransfer->files)))
		Error(connection, GP_PARAMETER_ERROR, "Invalid index.");
	file = (GPIFile *)ArrayNth(pTransfer->files, index);

	// Get the modTime.
	///////////////////
	*modTime = file->modTime;

	return GP_NO_ERROR;
}

GPResult gpGetNumTransfers(
  GPConnection * connection,
  int * num
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Check for NULL.
	//////////////////
	if(num == NULL)
		Error(connection, GP_PARAMETER_ERROR, "NULL pointer.");

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Set num.
	///////////
	*num = ArrayLength(iconnection->transfers);

	return GP_NO_ERROR;
}

GPResult gpGetTransfer(
  GPConnection * connection,
  int index,
  GPTransfer * transfer
)
{
	GPIConnection * iconnection;
	int localID;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return GP_PARAMETER_ERROR;

	// Check for NULL.
	//////////////////
	if(transfer == NULL)
		Error(connection, GP_PARAMETER_ERROR, "NULL pointer.");

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	// Get the local ID.
	////////////////////
	localID = gpiGetTransferLocalIDByIndex(connection, index);

	// Check if it was a bad index.
	///////////////////////////////
	if(localID == -1)
		Error(connection, GP_PARAMETER_ERROR, "Index out of range.");

	// Set the transfer they want.
	//////////////////////////////
	*transfer = localID;

	return GP_NO_ERROR;
}
#endif

#ifdef _DEBUG
void gpProfilesReport(
  GPConnection * connection,
  void (* report)(const char * output)
)
{
	GPIConnection * iconnection;

	// Error check.
	///////////////
	if((connection == NULL) || (*connection == NULL))
		return;

	// Get the connection object.
	/////////////////////////////
	iconnection = (GPIConnection *)*connection;

	gpiReport(connection, report);
}
#endif
