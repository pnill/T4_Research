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

#include <stdio.h>
#include <stdlib.h>
#include "gt.h"

#define GTI_STACK_HOSTLEN_MAX       256

const char * gtAddressToString
(
	unsigned int ip,
	unsigned short port,
	char string[22]
)
{
	static char strAddressArray[2][22];
	static int nIndex;
	char * strAddress;

	if(string)
		strAddress = string;
	else
	{
		nIndex ^= 1;
		strAddress = strAddressArray[nIndex];
	}

	if(ip)
	{
		IN_ADDR inAddr;
		
		inAddr.s_addr = ip;

		if(port)
			sprintf(strAddress, "%s:%d", inet_ntoa(inAddr), port);
		else
			sprintf(strAddress, "%s", inet_ntoa(inAddr));
	}
	else if(port)
		sprintf(strAddress, ":%d", port);
	else
		strAddress[0] = '\0';

	return strAddress;
}

GTBool gtStringToAddress
(
	const char * string,
	unsigned int * ip,
	unsigned short * port
)
{
	unsigned int srcIP;
	unsigned short srcPort;

	if(!string || !string[0])
	{
		srcIP = 0;
		srcPort = 0;
	}
	else
	{
		char stackHost[GTI_STACK_HOSTLEN_MAX + 1];
		const char * colon;
		const char * host;

		// Is there a port?
		///////////////////
		colon = strchr(string, ':');
		if(!colon)
		{
			// The string is the host.
			//////////////////////////
			host = string;

			// No port.
			///////////
			srcPort = 0;
		}
		else
		{
			int len;
			const char * check;
			int temp;

			// Is it just a port?
			/////////////////////
			if(colon == string)
			{
				host = NULL;
				srcIP = 0;
			}
			else
			{
				// Copy the host portion into the array on the stack.
				/////////////////////////////////////////////////////
				len = (colon - string);
				assert(len < GTI_STACK_HOSTLEN_MAX);
				memcpy(stackHost, string, len);
				stackHost[len] = '\0';
				host = stackHost;
			}

			// Check the port.
			//////////////////
			for(check = (colon + 1) ; *check ; check++)
				if(!isdigit(*check))
					return GTFalse;

			// Get the port.
			////////////////
			temp = atoi(colon + 1);
			if((temp < 0) || (temp > 0xFFFF))
				return GTFalse;
			srcPort = (unsigned short)temp;
		}

		// Is there a host?
		///////////////////
		if(host)
		{
			// Try dotted IP.
			/////////////////
			srcIP = inet_addr(host);
			if(srcIP == INADDR_NONE)
			{
				HOSTENT * hostent;

				hostent = gethostbyname(host);
				if(hostent == NULL)
					return GTFalse;

				srcIP = *(unsigned int *)hostent->h_addr_list[0];
			}
		}
	}

	if(ip)
		*ip = srcIP;
	if(port)
		*port = srcPort;

	return GTTrue;
}

const char * gtIPToHostname
(
	unsigned int ip
)
{
//	HOSTENT * hostent;

#ifdef __mips64
	/* gethostbyaddr is not supported on the ps2 via snsystems stack*/
	return NULL;
#else

	hostent = gethostbyaddr((const char *)&ip, 4, AF_INET);
	if(!hostent)
		return NULL;

	return hostent->h_name;
#endif // __mips64
}

unsigned int gtNetworkToHostInt
(
	unsigned int i
)
{
	return (unsigned int)ntohl(i);
}

unsigned int gtHostToNetworkInt
(
	unsigned int i
)
{
	return (unsigned int)htonl(i);
}

unsigned short gtNetworkToHostShort
(
	unsigned short s
)
{
	return ntohs(s);
}

unsigned short gtHostToNetworkShort
(
	unsigned short s
)
{
	return htons(s);
}