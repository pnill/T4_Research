/*
gpiOperation.h
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

#ifndef _GPIOPERATION_H_
#define _GPIOPERATION_H_

//INCLUDES
//////////
#include "gpi.h"

//DEFINES
/////////
// Operation Types.
///////////////////
#define GPI_CONNECT                    0
#define GPI_NEW_PROFILE                1
#define GPI_GET_INFO                   2
#define GPI_PROFILE_SEARCH             3
#define GPI_PEER_MESSAGE               5

// Operation States.
////////////////////
#define GPI_START                      0
//#define GPI_CONNECTING               1
#define GPI_LOGIN                      2
#define GPI_REQUESTING                 3
#define GPI_WAITING                    4
#define GPI_FINISHING                  5

//TYPES
///////
// Operation data.
//////////////////
typedef struct GPIOperation_s
{
  int type;
  void * data;
  GPIBool blocking;
  GPICallback callback;
  int state;
  int id;
  GPResult result;
  struct GPIOperation_s * pnext;
} GPIOperation;

// Connect operation data.
//////////////////////////
typedef struct
{
  char serverChallenge[128];
  char userChallenge[33];
  char passwordHash[33];
  GPIBool newuser;
} GPIConnectData;

//FUNCTIONS
///////////
GPResult
gpiAddOperation(
  GPConnection * connection,
  int type,
  void * data,
  GPIOperation ** op,
  GPEnum blocking,
  GPCallback callback,
  void * param
);

void
gpiRemoveOperation(
  GPConnection * connection,
  GPIOperation * operation
);

void
gpiDestroyOperation(
  GPConnection * connection,
  GPIOperation * operation
);

GPIBool
gpiFindOperationByID(
  GPConnection * connection,
  GPIOperation ** operation,
  int id
);

GPIBool
gpiOperationsAreBlocking(
  GPConnection * connection
);

GPResult
gpiProcessOperation(
  GPConnection * connection,
  GPIOperation * operation,
  const char * input
);

GPResult
gpiFailedOpCallback(
  GPConnection * connection,
  GPIOperation * operation
);

#endif
