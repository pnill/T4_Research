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

#include <stdlib.h>
#include "gt.h"
#include "gtAuth.h"

#define CALCULATEODDMODE(buffer, i, oddmode) ((buffer[i-1] & 1) ^ (i & 1) ^ oddmode ^ (buffer[0] & 1) ^ ((buffer[0] < 79) ? 1 : 0) ^ ((buffer[i-1] < buffer[0]) ? 1 : 0));

char challengeKey[33] = "3b8dd8995f7c40a9a5c5b7dd5b481341";

static int gtiVerifyChallenge(GTByte *buffer)
{
	int oddmode = 0;
	int i;
	for (i = 1; i < GTI_CHALLENGE_LEN ; i++)
	{
		oddmode = CALCULATEODDMODE(buffer,i, oddmode);
		if ((oddmode && (buffer[i] & 1) == 0) || (!oddmode && ((buffer[i] & 1) == 1)))
			return 0; //failed!!
	}
	return 1;
}

GTByte * gtiGetChallenge
(
	GTByte * buffer
)
{
	int i;
	int oddmode;
	assert(buffer);

	srand((unsigned int)current_time());
	buffer[0] = 33 + rand() % 93; //use chars in the range 33 - 125
	oddmode = 0;
	for (i = 1; i < GTI_CHALLENGE_LEN ; i++)
	{
		oddmode = CALCULATEODDMODE(buffer,i, oddmode);
		buffer[i] = 33 + rand() % 93; //use chars in the range 33 - 125
		//if oddmode make sure the char is odd, otherwise make sure it's even
		if ((oddmode && (buffer[i] & 1) == 0) || (!oddmode && ((buffer[i] & 1) == 1)))
			buffer[i]++;

	}
	return buffer;
}

GTByte * gtiGetResponse
(
	GTByte * buffer,
	GTByte * challenge
)
{
	int i;
	int valid;
	char cchar;
	int keylen = strlen(challengeKey);
	int chalrand;
	valid = gtiVerifyChallenge(challenge); //it's an invalid challenge, give them a bogus response
	assert(GTI_RESPONSE_LEN <= GTI_CHALLENGE_LEN);
	for (i = 0 ; i < GTI_RESPONSE_LEN ; i++)
	{
		//use random vals for spots 0 and 13
		if (!valid || i == 0 || i == 13)
			buffer[i] = 33 + rand() % 93; //use chars in the range 33 - 125
		else
		{ //set the character to look back at, never use the random ones!
			if (i == 1 || i == 14)
				cchar = challenge[i];
			else
				cchar = challenge[i-1];
			chalrand = abs((challenge[((i * challenge[i]) + challengeKey[(i + challenge[i]) % keylen]) % GTI_CHALLENGE_LEN] ^ challengeKey[(i * 17991 * cchar) % keylen]));
			buffer[i] = 33 + chalrand % 93;
		}
	}
	return buffer;
}


GTBool gtiCheckResponse
(
	GTByte * response1,
	GTByte * response2
)
{
	int i; //when comparing ignore the ones that are random
	for (i = 0 ; i < GTI_RESPONSE_LEN ; i++)
	{
		if (i != 0 && i != 13 && response1[i] != response2[i])
			return GTFalse;
	}
	return GTTrue;
}
