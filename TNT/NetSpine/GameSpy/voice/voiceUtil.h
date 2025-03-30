/*****************************************************************
voiceUtil.h
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
#ifndef _VOICEUTIL_H
#define _VOICEUTIL_H

#ifdef __cplusplus
extern "C" {
#endif
	
#define PACKET_DEFAULT	(-1)

typedef enum { voiceFalse, voiceTrue } voiceBool;

typedef struct _KeyVal* KeyVal;
typedef struct __Packet* Packet;

///////////////
// KEY-VALUE //
///////////////

// VoiceInitKeyVal initializes a KeyVal structure
KeyVal VoiceInitKeyVal(const char * kvstring, int len);

// VoiceDeleteKeyVal clears and deallocates and resources
// associated with the supplied KeyVal reference
void VoiceDeleteKeyVal(KeyVal kv);

// VoiceGetKeyValLength returns the length of the KeyVal buffer
int VoiceGetKeyValLength(KeyVal kv);

// VoiceGetKeyValBuffer retrns a pointer to the KeyVal buffer
const char * VoiceGetKeyValBuffer(KeyVal kv);

// VoiceGetString returns a NULL terminated string that is associated
// with the supplied key
const char * VoiceGetString(KeyVal kv, const char * key, const char * def);

// VoiceGetInt returns an 'int' value that is associated with the 
// supplied key
int VoiceGetInt(KeyVal kv, const char * key, int def);

// VoiceGetFloat returns a 'double' value that is associated with the 
// supplied key
double VoiceGetFloat(KeyVal kv, const char * key, double def);

// VoiceGetBool returns a 'voiceBool' value that is associated with
// the supplied key
voiceBool VoiceGetBool(KeyVal kv, const char * key, voiceBool def);

// VoiceSetString sets a NULL terminated string that is associated with
// the supplied key
voiceBool VoiceSetString(KeyVal kv, const char * key, const char *value);

// VoiceSetInt sets an 'int' value that is associated with the supplied
// key
voiceBool VoiceSetInt(KeyVal kv, const char * key, int value);

// VoiceSetFloat sets a 'double' value that is associated with the
// supplied key
voiceBool VoiceSetFloat(KeyVal kv, const char * key, double value);

// VoiceSetBool sets a 'voiceBool' value that is associated with the 
// supplied key
voiceBool VoiceSetBool(KeyVal kv, const char * key, voiceBool value);

// VoiceDeleteKey removes the key and its associated value from the
// provided KeyVal
voiceBool VoiceDeleteKey(KeyVal kv, const char * key);

// Callback function used for enumerating the keys/values in a KeyVal
typedef void (*VoiceKeyEnumFn)(char *key, char *value, void *instance);

// VoiceEnumKeys
// Enumerates the keys/values for a given KeyVal by calling VoiceKeyEnumFn with each
// key/value. The user-defined instance data will be passed to the KeyFn callback
void VoiceEnumKeys(KeyVal kv, VoiceKeyEnumFn KeyFn, void* instance);

////////////
// PACKET //
////////////

// PacketNew initializes a new packet
Packet PacketNew(int start_size);

// PacketClear re-initializes an existing packet and removes any
// allocaed resources associated with the supplied packet
void PacketClear(Packet p);

// PacketLoad creates a packet from a buffer. This allows a packet
// to be transfered over a network as a standard bufffer, then it can be 
// recreated as a packet on the other side using this function
voiceBool PacketLoad(Packet p, char *buffer);

// PacketDelete reclaims any allocated resources associated with the
// packet
void PacketDelete(Packet p);

// PacketReset is used on a packet to reset its read position to the
// top element in the packet
void PacketReset(Packet p);

// PacketBuffer returns a pointer to the packet data so it can be 
// transfered over a network. Once the buffer is transfered, it can
// be reassembled by using the PacketLoad function
char *PacketBuffer(Packet p);

// PacketLength returns the length of the packet's data buffer
unsigned short PacketLength(Packet p);

// PacketAddByte adds one byte(unsigned char) of data to the packet's 
// list of elements
void PacketAddByte(Packet p, unsigned char b);

// PacketAddShort adds one 'unsigned short' data element to the packet
void PacketAddShort(Packet p, unsigned short s);

// PacketAddInt adds one 'unsigned int' data element to the packet
void PacketAddInt(Packet p, unsigned int i);

// PacketAddString adds a NULL terminiated string element to the packet
void PacketAddString(Packet p, const char *s);

// PacketAddBlob adds an abitrary length of data as an element to the
// packet
void PacketAddBlob(Packet p, const char *b, unsigned short l);

// PacketGetByte returns one byte(unsigned char) of data from the 
// packet and changes the packet's current element pointer to point to
// the next data element.
unsigned char PacketGetByte(Packet p);

// PacketGetShort returns an 'unsigned short' data element from the
// packet and changes the packet's current element pointer to point to
// the next data element.
unsigned short PacketGetShort(Packet p);

// PacketGetInt returns an 'unsigned int' data element from the packet
// and changes the packet's current element pointer to point to the next
// data element.
unsigned int PacketGetInt(Packet p);

// PacketGetString returns a NULL terminated string element from the 
// packet and changes the packet's current element pointer to point to
// the next data element.
char *PacketGetString(Packet p);

// PacketGetBlob returns a pointer to an arbitrary buffer element of the 
// size returned by the 'l' parameter and changes the packet's current
// element pointer to point to the next data element.
char *PacketGetBlob(Packet p, unsigned short *l);

#ifdef __cplusplus
}
#endif


#endif /* _VOICEUTIL_H */
