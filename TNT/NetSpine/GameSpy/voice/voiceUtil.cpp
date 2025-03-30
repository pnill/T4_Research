/*****************************************************************
voiceUtil.c
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
*******************************************************************/
#include "voiceUtil.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../nonport.h"

#define KVSTARTLENGTH 256
#define MAXSTRINGSIZE 256
#define NEWPACKETSIZE 2048

#ifdef LINUX
	#define _gcvt gcvt
#endif

typedef struct _KeyVal
{
	int len;
	char * string;
} _KeyVal;

///////////////
// KEY-VALUE //
///////////////

KeyVal VoiceInitKeyVal(const char * kvstring, int len)
{
	_KeyVal * kv;

	assert(len >= 0);

	// allocate the structure
	kv = (_KeyVal*)gsimalloc(sizeof(_KeyVal));
	if(!kv)
		return NULL;

	// check for no string
	if(!kvstring || !len)
	{
		kv->string = NULL;
		kv->len = 0;
	}
	else
	{
		// allocate memory for the string
		kv->string = (char *)gsimalloc(len);
		if(!kv->string)
		{
			gsifree(kv);
			return NULL;
		}

		// copy the string
		memcpy(kv->string, kvstring, len);
		kv->len = len;
	}

	// return the struct
	return kv;
}

void VoiceDeleteKeyVal(KeyVal kv)
{
	assert(kv);

	gsifree(kv->string);
	gsifree(kv);
}

int VoiceGetKeyValLength(KeyVal kv)
{
	assert(kv);

	return kv->len;
}

const char * VoiceGetKeyValBuffer(KeyVal kv)
{
	assert(kv);

	return kv->string;
}

static int VoiceFindNextStringOffset(const char * start, int len)
{
	int i;

	// loop through the rest of the string
	for(i = 0 ; i < len ; i++)
	{
		// check for a NUL
		if(!start[i])
		{
			// check for the end of the string
			if(i == (len - 1))
				return -1;

			// return the position of the next string
			return (i + 1);
		}
	}

	// if we get here, then the kvstring doesn't end with NUL
	// it should always end with NUL
	assert(0);
	return -1;
}

static voiceBool VoiceFindKeyPos(KeyVal kv, const char * key, int * keyPos, int * valuePos)
{
	int pos = 0;
	int offset;

	// loop while there are values to check
	while(pos < kv->len)
	{
		// find where the value starts
		offset = VoiceFindNextStringOffset(kv->string + pos, kv->len - pos);
		if(offset == -1)
			return voiceFalse;

		// is this our key?
		if(strcasecmp(kv->string + pos, key) == 0)
		{
			// set the positions
			if(keyPos)
				*keyPos = pos;
			if(valuePos)
				*valuePos = (pos + offset);

			// found it
			return voiceTrue;
		}

		// skip the key
		pos += offset;

		// find the next key
		offset = VoiceFindNextStringOffset(kv->string + pos, kv->len - pos);
		if(offset == -1)
			return voiceFalse;
		pos += offset;
	}

	return voiceFalse;
}

static const char * VoiceFindKeyValue(KeyVal kv, const char * key)
{
	int valuePos;

	if(!VoiceFindKeyPos(kv, key, NULL, &valuePos))
		return NULL;

	return (kv->string + valuePos);
}

const char * VoiceGetString(KeyVal kv, const char * key, const char * def )
{
	const char * value;

	assert(kv);
	assert(key);

	value = VoiceFindKeyValue(kv, key);
	if(!value)
		value = def;

	return value;
}

int VoiceGetInt(KeyVal kv, const char * key, int def)
{
	const char * value;

	assert(kv);
	assert(key);

	value = VoiceFindKeyValue(kv, key);
	if(!value)
		return def;

	return atoi(value);
}

double VoiceGetFloat(KeyVal kv, const char * key, double def)
{
	const char * value;

	assert(kv);
	assert(key);

	value = VoiceFindKeyValue(kv, key);
	if(!value)
		return def;

	return atof(value);
}

voiceBool VoiceGetBool(KeyVal kv, const char * key, voiceBool def)
{
	const char * value;

	assert(kv);
	assert(key);

	value = VoiceFindKeyValue(kv, key);
	if(!value)
		return def;

	// check the first char for known "false" values
	if( '0' == *value || 'F' == *value || 'f' == *value || 'N' == *value || 'n' == *value )
		return voiceFalse;

	// presume that all other non-zero values are "true"
	return voiceTrue;
}

voiceBool VoiceSetString(KeyVal kv, const char * key, const char *value)
{
	int keyPos;
	int valuePos;
	int keyLen;
	int valueLen;
	int oldValueLen;
	int oldValueEnd;
	int newLen;
	char * newString;

	assert(kv);
	assert(key);

	// can't have a NULL value
	if(!value)
		value = "";

	// get the lengths
	keyLen = (strlen(key) + 1);
	valueLen = (strlen(value) + 1);

	// search for this key in the string
	if(VoiceFindKeyPos(kv, key, &keyPos, &valuePos))
	{
		// get the old value length
		oldValueLen = (strlen(kv->string + valuePos) + 1);

		// store the offset where the old value ends
		oldValueEnd = (valuePos + oldValueLen);

		// move everything past the old value over the key/value
		memmove(kv->string + keyPos, kv->string + oldValueEnd, kv->len - oldValueEnd);

		// adjust the length
		kv->len -= (oldValueEnd - keyPos);
	}

	// figure out the new length
	newLen = (kv->len + keyLen + valueLen);

	// make sure we have enough memory
	newString = (char *)gsirealloc(kv->string, newLen);
	if(!newString)
		return voiceFalse;

	// store the new string
	kv->string = newString;

	// add the key
	memcpy(kv->string + kv->len, key, keyLen);
	kv->len += keyLen;

	// add the value
	memcpy(kv->string + kv->len, value, valueLen);
	kv->len += valueLen;

	return voiceTrue;
}

voiceBool VoiceSetInt(KeyVal kv, const char * key, int value)
{
	char stringValue[16];
	sprintf(stringValue, "%d", value);

	return VoiceSetString(kv, key, stringValue);
}

voiceBool VoiceSetFloat(KeyVal kv, const char * key, double value)
{
	char stringValue[16];
	sprintf(stringValue, "%f", value);

	return VoiceSetString(kv, key, stringValue);
}

voiceBool VoiceSetBool(KeyVal kv, const char * key, voiceBool value)
{
	return VoiceSetString(kv,key,value?"1":"0");
}

void VoiceEnumKeys(KeyVal kv, VoiceKeyEnumFn KeyFn, void* instance)
{
	int pos = 0;
	int offset;
	char* pKey;
	char* pVal;

	assert(kv);
	assert(KeyFn);

	// loop while there are values to check
	while(pos < kv->len && kv )
	{
		// find where the value starts
		offset = VoiceFindNextStringOffset(kv->string + pos, kv->len - pos);
		if(offset == -1)
			return;

		pKey = kv->string + pos;
		pVal = kv->string + pos + offset;

		if( KeyFn )
			KeyFn(pKey,pVal,instance);

		// skip the key
		pos += offset;	

		// find the next key
		offset = VoiceFindNextStringOffset(kv->string + pos, kv->len - pos);
		if(offset == -1)
			return;
		pos += offset;
	}
}

////////////
// PACKET //
////////////

typedef struct __Packet
{
	unsigned short length;
	unsigned short index;
	char *buffer;
	unsigned short allocated;
} * _Packet;

void PacketClear(Packet p)
{
	_Packet _p = (_Packet) p;
	unsigned short *s;

	_p->index = 2;
	_p->length = 2;
	s = (unsigned short *)_p->buffer;
	*s = htons(_p->length);
}

Packet PacketNew(int start_size)
{
	_Packet p = (_Packet)gsimalloc(sizeof(struct __Packet));

	assert(p);
	memset(p, 0, sizeof(struct __Packet));

	if(start_size > 0)
	{
		p->buffer = (char*)gsimalloc(start_size);
		assert(p->buffer);
		p->allocated = start_size;
	}
	else
	{
		p->buffer = (char*)gsimalloc(NEWPACKETSIZE);
		assert(p->buffer);
		p->allocated = NEWPACKETSIZE;
	}

	PacketClear((Packet)p);

	return (Packet)p;
}

voiceBool PacketLoad(Packet p, char *buffer)
{
	_Packet _p = (_Packet)p;
	unsigned short *s;

	if(!p && !buffer)
		return voiceFalse;

	PacketClear(p);

	s = (unsigned short *)buffer;
	if(_p->allocated < ntohs(*s))
	{	
		_p->allocated = ntohs(*s);
		_p->buffer = (char*)gsirealloc(_p->buffer, ntohs(*s));
	}
	assert(_p->buffer);
	_p->length = ntohs(*s);
	memcpy(_p->buffer, buffer, _p->length);

	return voiceTrue;
}

void PacketDelete(Packet p)
{
	_Packet _p = (_Packet) p;

	assert(p);
	
	gsifree(_p->buffer);
	gsifree(_p);
	p = NULL;
}

void PacketReset(Packet p)
{
	_Packet _p = (_Packet) p;

	_p->index = 2;
}

static void PacketExpand(_Packet p, unsigned short add_length)
{
	if((p->length + add_length) > p->allocated)
	{
		p->buffer = (char *)gsirealloc(p->buffer, p->length + add_length);
		assert(p->buffer);
		p->allocated = p->length + add_length;
	}

}

char *PacketBuffer(Packet p)
{
	_Packet _p = (_Packet) p;
	return _p->buffer;
}

unsigned short PacketLength(Packet p)
{
	_Packet _p = (_Packet) p;
	return _p->length;
}

void PacketAddByte(Packet p, unsigned char b)
{
	_Packet _p = (_Packet) p;
	unsigned short *ps;

	PacketExpand(p, sizeof(unsigned char));

	_p->buffer[_p->length] = b;
	_p->length += sizeof(unsigned char);
	ps = (unsigned short *)_p->buffer;
	*ps = htons(_p->length); 
}

void PacketAddShort(Packet p, unsigned short s)
{
	_Packet _p = (_Packet) p;
	unsigned short *ps;

	s = htons(s);

	PacketExpand(p, sizeof(unsigned short));

	memcpy(&_p->buffer[_p->length], &s, sizeof(unsigned short));
	_p->length += sizeof(unsigned short);
	ps = (unsigned short *)_p->buffer;
	*ps = htons(_p->length); 
}

void PacketAddInt(Packet p, unsigned int i)
{
	_Packet _p = (_Packet) p;
	unsigned short *ps;

	i = htonl(i);

	PacketExpand(p, sizeof(unsigned int));

	memcpy(&_p->buffer[_p->length], &i, sizeof(unsigned int));
	_p->length += sizeof(unsigned int);
	ps = (unsigned short *)_p->buffer;
	*ps = htons(_p->length); 
}

void PacketAddString(Packet p, const char *s)
{
	_Packet _p = (_Packet) p;
	unsigned short slen;
	unsigned short *ps;

	assert(s);
	if(!s)
		s = "";

	slen = strlen(s);
	slen++;

	PacketExpand(_p, slen);

	strcpy(_p->buffer+_p->length, s);
	_p->length += slen;
	ps = (unsigned short *)_p->buffer;
	*ps = htons(_p->length); 
}

void PacketAddBlob(Packet p, const char *b, unsigned short l)
{
	_Packet _p = (_Packet) p;
	unsigned short *ps;

	assert(b);

	PacketAddShort(p, l);

	PacketExpand(p, l);

	memcpy(_p->buffer+_p->length, b, l);
	_p->length += l;
	ps = (unsigned short *)_p->buffer;
	*ps = htons(_p->length); 
}

unsigned char PacketGetByte(Packet p)
{
	_Packet _p = (_Packet) p;
	unsigned char c = _p->buffer[_p->index];
	_p->index += sizeof(unsigned char);
	return c;
}

unsigned short PacketGetShort(Packet p)
{
	_Packet _p = (_Packet) p;
	unsigned short s;

	memcpy(&s, &_p->buffer[_p->index], sizeof(unsigned short));
	_p->index += sizeof(unsigned short);
	return ntohs(s);
}

unsigned int PacketGetInt(Packet p)
{
	_Packet _p = (_Packet) p;
	unsigned int i;

	memcpy(&i, &_p->buffer[_p->index], sizeof(unsigned int));
	_p->index += sizeof(unsigned int);
	return ntohl(i);
}

char *PacketGetString(Packet p)
{
	_Packet _p = (_Packet) p;
	char *pstr;

	pstr = (char *)&_p->buffer[_p->index];
	_p->index += strlen(pstr)+1;
	return pstr;
}

char *PacketGetBlob(Packet p, unsigned short *l)
{
	_Packet _p = (_Packet) p;
	char *blob;

	*l = PacketGetShort(p);

	blob = (char *)&_p->buffer[_p->index];
	_p->index += *l;
	return blob;
}
