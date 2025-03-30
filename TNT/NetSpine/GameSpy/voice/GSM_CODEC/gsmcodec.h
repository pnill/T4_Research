/*****************************************************************
gsmcodec.h
GameSpy Voice SDK
Richard "Huckster" Rice

Copyright 2002 GameSpy Industries, Inc.

18002 Skypark Circle
Irvine, California 92614
949.798.4200 (Tel)
949.798.4299 (Fax)
devsupport@gamespy.com

********************************************************************
Please see the GameSpy Voice SDK documentation for more information
*******************************************************************/#ifndef _GSMCODEC_H
#define _GSMCODEC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../voiceCodec.h"

VoiceCodec *GSMCodecInit(unsigned int flags);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* _GSMCODEC_H */
