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

#ifndef _GTAUTH_H_
#define _GTAUTH_H_

#define GTI_CHALLENGE_LEN           32
#define GTI_RESPONSE_LEN            32
#ifdef __cplusplus
extern "C" {
#endif


GTByte * gtiGetChallenge
(
	GTByte * buffer
);

GTByte * gtiGetResponse
(
	GTByte * buffer,
	GTByte * challenge
);

GTBool gtiCheckResponse
(
	GTByte * response1,
	GTByte * response2
);

#ifdef __cplusplus
}
#endif


#endif