/*
gpiConnect.h
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

#ifndef _GPICONNECT_H_
#define _GPICONNECT_H_

//INCLUDES
//////////
#include "gpi.h"

//DEFINES
/////////
// Connect States.
//////////////////
#define GPI_NOT_CONNECTED              0
#define GPI_CONNECTING                 1
#define GPI_NEGOTIATING                2
#define GPI_CONNECTED                  3
#define GPI_DISCONNECTED               4

//FUNCTIONS
///////////
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
);

void
gpiDisconnect(
  GPConnection * connection,
  GPIBool tellServer
);

GPResult
gpiProcessConnect(
  GPConnection * connection,
  GPIOperation * operation,
  const char * input
);

GPResult
gpiCheckConnect(
  GPConnection * connection
);

#endif
