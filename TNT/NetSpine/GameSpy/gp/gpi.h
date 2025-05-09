/*
gpi.h
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

#ifndef _GPI_H_
#define _GPI_H_

//INCLUDES
//////////
#include <stdio.h>
//#include <time.h>
#if defined(applec) || defined(THINK_C) || defined(__MWERKS__) && !defined(__mips64) && !defined(_WIN32)
	#include "::nonport.h"
	#include "::hashtable.h"
	#include "::darray.h"
	#include "::md5.h"
#else
	#include "../nonport.h"
	#include "../hashtable.h"
	#include "../darray.h"
	#include "../md5.h"
#endif
#include "gp.h"


//DEFINES
/////////
//#define GPDEBUG //undef to remove debug output
#define DEBUG_CRAZY

//TYPES
///////
// Boolean.
///////////
typedef enum _GPIBool
{
	GPIFalse,
	GPITrue
} GPIBool;

#include "gpiUtility.h"
#include "gpiCallback.h"
#include "gpiOperation.h"
#include "gpiConnect.h"
#include "gpiBuffer.h"
#include "gpiInfo.h"
#include "gpiProfile.h"
#include "gpiPeer.h"
#include "gpiSearch.h"
#include "gpiBuddy.h"
#include "gpiTransfer.h"

// Connection data.
///////////////////
typedef struct
{
  char errorString[GP_ERROR_STRING_LEN];
  GPIBool infoCaching;
  GPIBool simulation;
  GPIBool firewall;
  char nick[GP_NICK_LEN];
  char email[GP_EMAIL_LEN];
  char password[GP_PASSWORD_LEN];
  int sessKey;
  int userid;
  int profileid;
  GPIBool retryConnect;
  GPICallback callbacks[GPI_NUM_CALLBACKS];
  SOCKET cmSocket;
  int connectState;
  GPIBuffer socketBuffer;
  char * inputBuffer;
  int inputBufferSize;
  GPIBuffer outputBuffer;
  SOCKET peerSocket;
  int peerPort;
  int nextOperationID;
  int numSearches;
  GPEnum lastStatus;
  char lastStatusString[GP_STATUS_STRING_LEN];
  char lastLocationString[GP_LOCATION_STRING_LEN];
  GPErrorCode errorCode;
  GPIBool fatalError;
  FILE * diskCache;
  GPIOperation * operationList;
  GPIProfileList profileList;
  GPIPeer * peerList;
  GPICallbackData * callbackList;
  GPICallbackData * lastCallback;
  GPIBuffer updateproBuffer;
  GPIBuffer updateuiBuffer;
  char * debugBuffer;
  DArray transfers;
  unsigned int nextTransferID;
  int productID;
} GPIConnection;

//FUNCTIONS
///////////
GPResult
gpiInitialize(
  GPConnection * connection,
  int productID
);

void
gpiDestroy(
  GPConnection * connection
);

GPResult
gpiReset(
  GPConnection * connection
);

GPResult
gpiProcessConnectionManager(
  GPConnection * connection
);

GPResult
gpiProcess(
  GPConnection * connection,
  int blockingOperationID
);

GPResult
gpiEnable(
  GPConnection * connection, 
  GPEnum state
);

GPResult
gpiDisable(
  GPConnection * connection, 
  GPEnum state
);

void
gpiReport(
  GPConnection * connection,
  void (* report)(const char * output)
);

#endif
