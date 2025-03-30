/*****************************************************************
voiceChannel.h
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
#ifndef _VOICECHANNEL_H
#define _VOICECHANNEL_H

#include "voiceHardware.h"
#include "voiceUtil.h"
#include "voiceCodec.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
*	VOICE CHANNEL / MEMBER REFERENCE
*************************************************************************/
typedef struct _Channel* Channel;  // Channel Reference Pointer
typedef unsigned short MemberId;   // Channel Member Reference

/*************************************************************************
*	VOICE CHANNEL DEFINES
*************************************************************************/

#define DEFAULT_VOICE_PORT   33000 // Default port used by the Voice SDK
#define INVALID_CHANNEL      (Channel)NULL
#define INVALID_MEMBERID     (MemberId)-1

/*************************************************************************
*	VOICE CHANNEL FLAGS
*************************************************************************/

/*------------------------------------------------------------------------
These flags are used by VoiceChannelCreate() and can be obtained later
by calling "VoiceGetChannelInt(<channel*>,"flags");"
--------------------------------------------------------------------------
CFLAG_HOSTTOPEER    | All voice communication with this channel will 
	                | go through the host, then out to all the channel
	                | members 
CFLAG_PEERTOPEER    | All voice communication with this channel will 
	                | go directly out from the local member to other 
	                | members of the channel
CFLAG_PEERRELAY     | All voice communication with the channel will 
	                | be relayed from one member to one or more 
	                | members until every member of the channel 
	                | received the voice communication
CFLAG_LOCALECHO     | All local member will receive back their own 
	                | voice communication (not recommended unless you 
	                | are using it to setup/test the capture hardware)
------------------------------------------------------------------------*/
#define CFLAG_HOSTTOPEER      0x00000001
#define CFLAG_PEERTOPEER      0x00000002
#define CFLAG_PEERRELAY       0x00000004
#define CFLAG_LOCALECHO       0x00000008

/*************************************************************************
*	VOICE CALLBACKS
*************************************************************************/

/*------------------------------------------------------------------------
CallbackVoiceData
--------------------------------------------------------------------------
If registered, the CallbackVoiceData callback is received when new 
voice data arrives from other channel members and is decoded. The 
provided voice data can be used to allow the application to manipulate
or even play back the audio using it's own playback system. 
For example, in a game the voice data can be used for positional audio
or for passing the voice data through a filter to add effects.
--------------------------------------------------------------------------
channel  | Channel reference pointer
memberid | Channel member reference number
sample   | Structure which contains the voice audio 
context  | Pointer to the registered callback context
------------------------------------------------------------------------*/
typedef void (*CallbackVoiceData)(
	Channel channel, 
	MemberId memberid,
	AudioSample * sample,
	void * context
);

/*------------------------------------------------------------------------
CallbackMiscData
--------------------------------------------------------------------------
If registered, the CallbackMiscData callback is received when 
miscellaneous data arrives from other channel members. Miscellaneous
data is sent via the VoiceChannelMiscData() function and travels
along the same connection as voice data.
--------------------------------------------------------------------------
channel    | Channel reference pointer
memberid   | Channel member reference number
miscdata   | Buffer containing the received data
datalength | Length of the data buffer 'miscdata'
context    | Pointer to the registered callback context
------------------------------------------------------------------------*/
typedef void (*CallbackMiscData)(
	Channel channel,
	MemberId memberid,
	char *miscdata,
	int datalength,
	void *context
);

/*------------------------------------------------------------------------
CallbackMemberJoined
--------------------------------------------------------------------------
If registered, the CallbackMemberJoined callback is received when
a new member has joined a channel.
--------------------------------------------------------------------------
channel  | Channel reference pointer
memberid | Channel member reference number
context  | Pointer to the registered callback context
------------------------------------------------------------------------*/
typedef void (*CallbackMemberJoined)(
	Channel channel,
	MemberId memberid,
	void *context
);

/*------------------------------------------------------------------------
CallbackMemberDescChanged
--------------------------------------------------------------------------
If registered, the CallbackMemberDescChanned callback is received
whenever a member's KeyVal descrition has changed.
A KeyVal description is a string of key/value pairs that represent
abitrary data that is associated with a member. Whenever that data
is changed, all members of the channel will get this notification
callback.
--------------------------------------------------------------------------
channel       | Channel reference pointer
memberid      | Channel member reference number
context       | Pointer to the registered callback context
------------------------------------------------------------------------*/
typedef void (*CallbackMemberDescChanged)(
	Channel channel,
	MemberId memberid,
	void *context
);

/*------------------------------------------------------------------------
CallbackMemberDisconnected
--------------------------------------------------------------------------
If registered, the CallbackMemberDisconnected callback is received
when a member has disconnected from the channel.
Check the reason parameter to determine how the member left.
--------------------------------------------------------------------------
channel   | Channel reference pointer
memberid  | Channel member reference number
reason    | Provides the reason the member was disconnected
context   | Pointer to the registered callback context
--------------------------------------------------------------------------
memberdisconnectReason
--------------------------------------------------------------------------
memberdisconnectLeft     | The member left the channel
memberdisconnectKicked   | The member was kicked from the channel
	                     | by the host
memberdisconnectBanned   | The member was banned from the channel
	                     | by the host
------------------------------------------------------------------------*/
typedef enum
{
	memberdisconnectLeft,
	memberdisconnectKicked,
	memberdisconnectBanned
} memberdisconnectReason;

typedef void (*CallbackMemberDisconnected)(
	Channel channel,
	MemberId memberid,
	memberdisconnectReason reason,
	void *context
);

/*------------------------------------------------------------------------
CallbackChannelDescChanged
--------------------------------------------------------------------------
If registered, the CallbackChannelDescChanned callback is received
whenever the channel's KeyVal descrition has changed.
A KeyVal description is a string of key/value pairs that represent
abitrary data that is associated with a channel. Whenever that data
is changed, all members of the channel will get this notification
callback.
--------------------------------------------------------------------------
channel  | Channel reference pointer
context  | Pointer to the registered callback context
------------------------------------------------------------------------*/
typedef void (*CallbackChannelDescChanged)(
	Channel channel,
	void *context
);

/*------------------------------------------------------------------------
CallbackTalkingStarted
--------------------------------------------------------------------------
If registered, the CallbackTalkingStarted callback is received
when a member of the channel begins broadcasting audio.
--------------------------------------------------------------------------
channel  | Channel reference pointer
memberid | Channel member reference number
context  | Pointer to the registered callback context
------------------------------------------------------------------------*/
typedef void (*CallbackTalkingStarted)(
	Channel channel,
	MemberId memberid,
	void *context
);

/*------------------------------------------------------------------------
CallbackTalkingStopped
--------------------------------------------------------------------------
If registered, the CallbackTalkingStopped callback is received
when a member of the channel stops broadcasting audio
--------------------------------------------------------------------------
channel  | Channel reference pointer
memberid | Channel member reference number
context  | Pointer to the registered callback context
------------------------------------------------------------------------*/
typedef void (*CallbackTalkingStopped)(
	Channel channel,
	MemberId memberid,
	void *context
);

/*------------------------------------------------------------------------
CallbackRehost
--------------------------------------------------------------------------
If registered, the CallbackRehost callback is received when
the channel host leaves the channel and a new host has been
picked.
The newhostid parameter will provide the new host's member id.
--------------------------------------------------------------------------
channel   | Channel reference pointer
newhostid | Channel member reference number of the new channel host
context   | Pointer to the registered callback context
------------------------------------------------------------------------*/
typedef void (*CallbackRehost)(
	Channel channel,
	MemberId newhostid,
	void *context
);

/*------------------------------------------------------------------------
CallbackConnectSuccess
--------------------------------------------------------------------------
If registered, the CallbackConnectSuccess callback is received
upon a successful connection to the channel.
This callback is only returned after a sucessful call to either 
VoiceChannelCreate or VoiceChannelJoin.
--------------------------------------------------------------------------
channel | Channel reference pointer
context | Pointer to the registered callback context
------------------------------------------------------------------------*/
typedef void (*CallbackConnectSuccess)(
	Channel channel,
	void *context
);

/*------------------------------------------------------------------------
CallbackConnectFailed
--------------------------------------------------------------------------
If registered, the CallbackConnectFailed callback is received
upon a failed connection to the channel.
This callback is only returned after a failed call to either
VoiceChannelOprn or VoiceChannelJoin
The reason parameter will provide the reason for the failed
connection.
--------------------------------------------------------------------------
channel      | Channel reference pointer.  If INVALID_CHANNEL, the channel
             | was never allocated & the callback occured from within
             | VoiceChannelJoin or VoiceChannelCreate.
channel_name | NULL terminated string containing the channel name
reason       | Provides the reason for the failed connection
context      | Pointer to the registered callback context
--------------------------------------------------------------------------
connectfailedReason				
--------------------------------------------------------------------------
connectfailedInvalidPassword   | Password for channel is incorrect
connectfailedMissingCodec      | The codec is not registered on the
                               | local system
connectfailedBanned            | The local system is banned from
                               | joining the channel
connectfailedNoSuchChannel     | In the attempt to join a channel,
                               | the channel doesn't exist on the
                               | host machine.
connectfailedChannelExists     | There is already a channel on the
                               | local system of the same name
connectfailedChannelMaxReached | The maximum number of channels
                               | that can be ran on the local system
                               | has been reached
connectfailedNetwork           | A network error has occured
connectfailedBadAddress        | The address provided in the call
                               | to VoiceChannelJoin was an invalid
                               | TCPIP address
connectfailedRedirect          | The member we attemted to connect
                               | to was not the channel host
connectfailedBadVersion        | The Voice SDK version on system
                               | we attempted to connect to was
                               | not compatible with the local 
                               | version
------------------------------------------------------------------------*/
typedef enum 
{
	connectfailedInvalidPassword,
	connectfailedMissingCodec,
	connectfailedBanned,
	connectfailedNoSuchChannel,
	connectfailedChannelExists,
	connectfailedChannelMaxReached,
	connectfailedNetwork,
	connectfailedBadAddress,
	connectfailedRedirect,
	connectfailedBadVersion

} connectfailedReason;

typedef void (*CallbackConnectFailed)(
	Channel channel,
	const char *channel_name,
	connectfailedReason reason,
	void *context
);

/*------------------------------------------------------------------------
CallbackConnectionLost
--------------------------------------------------------------------------
If registered, the CallbackConnectionLost callback is received
whenever a connection to the channel is lost.
The reason parameter will provide the reason for the lost
connection.
--------------------------------------------------------------------------
channel   | Channel reference pointer
reason    | Provides the reason for the lost connection
context   | Pointer to the registered callback context
--------------------------------------------------------------------------
connectionlostReason
--------------------------------------------------------------------------
connectionlostDisconnected  | The remote host disconnected
connectionlostKicked        | The local user was kicked from the 
	                        | channel
connectionlostBanned        | The local user was banned from the 
	                        | channel
connectionlostRehostFailed  | Migrating to a new channel host failed
connectionlostNetwork       | A network failure caused the lost
	                        | connection with the channel
------------------------------------------------------------------------*/
typedef enum
{
	connectionlostDisconnected,
	connectionlostKicked,
	connectionlostBanned,
	connectionlostRehostFailed,
	connectionlostNetwork
} connectionlostReason;

typedef void (*CallbackConnectionLost)(
	Channel channel,
	connectionlostReason reason,
	void *context
);

/*************************************************************************
*	VOICE CALLBACK STRUCTURE
*************************************************************************/
typedef struct
{
	CallbackVoiceData           voicedata_cbf;
	CallbackMiscData            miscdata_cbf;
	CallbackMemberJoined        memberjoined_cbf;
	CallbackMemberDisconnected  memberdisconnect_cbf;
	CallbackMemberDescChanged   memberdescchanged_cbf;
	CallbackConnectSuccess      connectsuccess_cbf;
	CallbackConnectFailed       connectfailed_cbf;
	CallbackTalkingStarted      talkingstarted_cbf;
	CallbackConnectionLost      connectionlost_cbf;
	CallbackTalkingStopped      talkingstopped_cbf;
	CallbackRehost              rehost_cbf;
	void*                       context;

} VoiceCallbacks;

/*************************************************************************
*	VOICE INTERFACE FUNCTIONS
*************************************************************************/

/*------------------------------------------------------------------------
VoiceChannelCreate
--------------------------------------------------------------------------
VoiceChannelCreate is used to create a new channel. If successful,
the local system will become the channel host and all potential
members must connect to the channel host at the host's address and
the port returned by a successful call to VoiceInitialize.
After a call to VoiceChannelCreate there will be either a call to the
CallbackConnectSuccess or CallbackConnectFailed callback functions
depending on whether the channel was successfully created or not.
--------------------------------------------------------------------------
channel_name   | NULL terminated string containing the channel name
channel_pass   | NULL terminated string containing the channel password
codec          | Pointer to the codec to be used by this new channel
flags          | Channel flags which specify special channel behavior.
               | see the CFLAG_* defines.
member_context | NULL terminated sting that will be associated with
               | this member
callbacks      | Structure containing pointers to all the callbacks
               | which this channel will use
return         | a Channel refrence pointer
------------------------------------------------------------------------*/
Channel VoiceChannelCreate(
	const char *channel_name,
	const char *channel_pass,
	VoiceCodec *codec,
	int flags,
	const char *member_context,
	VoiceCallbacks *callbacks
);

/*------------------------------------------------------------------------
VoiceChannelJoin
--------------------------------------------------------------------------
VoiceChannelJoin is used to join an existing channel. If successful.
a connection to the host will be made as a new channel member. Before
a call to VoiceChannelJoin, the channel host's address is port must
be obtained and supplied to the appropriate parameters.
After a call to VoiceChannelJoin there will be either a call to the
CallbackConnectSuccess or CallbackConnectFailed callback functions
depending on whether there was a successful connection to the channel
or not.
--------------------------------------------------------------------------
channel_address | NULL terminated string which contains the channel's
                | network address. The address can be in domain
                | name form or in IP address form.
channel_name    | NULL terminated string which contains the channel's
                | name
port            | Contains the channel's connection port
channel_pass    | NULL terminated string which contains the channel's
                | password
member_context  | NULL terminated string that will be associated
                | with this member
callbacks       | Structure containing pointers to all the callbacks
                | which this channel will use
return          | a Channel refrence pointer
------------------------------------------------------------------------*/
Channel VoiceChannelJoin(
	const char *channel_address,
	const char *channel_name,
	unsigned short port,
	const char *channel_pass,
	const char *member_context,
	VoiceCallbacks *callbacks
);

/*------------------------------------------------------------------------
VoiceChannelDisconnect
--------------------------------------------------------------------------
VoiceChannelDisconnect should be used to disconnect from one or all
of the active channels.
If the channel parameter is a vaild channel reference pointer, then
the local member will be disconnected from only that channel.
If the channel parameter is NULL, then all active channels will be 
disconnected.
--------------------------------------------------------------------------
channel  | Channel reference pointer
------------------------------------------------------------------------*/
void VoiceChannelDisconnect(Channel channel);

/*------------------------------------------------------------------------
VoiceGetChannelCount
--------------------------------------------------------------------------
--------------------------------------------------------------------------
return   | Number of channels maintained
------------------------------------------------------------------------*/
unsigned int VoiceGetChannelCount();

/*------------------------------------------------------------------------
VoiceGetChannelByIndex
--------------------------------------------------------------------------
--------------------------------------------------------------------------
index    | index of the channel desired
return   | Channel reference pointer
------------------------------------------------------------------------*/
Channel VoiceGetChannelByIndex( unsigned int index );

/*------------------------------------------------------------------------
VoiceThink
--------------------------------------------------------------------------
VoiceThink is resposible to progressing the state of the Voice SDK and
should be called at least every 20 milliseconds after the call to
VoiceInitialize.
All callbacks can be expected to be called during the call to
VoiceThink.
------------------------------------------------------------------------*/
void VoiceThink(void);

/*------------------------------------------------------------------------
VoiceActivateLevel
--------------------------------------------------------------------------
VoiceActivateLevel sets the automatic voice level of the capture
device. The level ranges from 1 to 32767. For example, if a level
of 1500 is set, then any audio coming from the registered
capture device that breaks the amplitude level of 1500 will
automatically be broadcast on all active channels.
Typically for manual activation through a talk button, you will
want to set the activation level to 0 which means that a call from
VoiceRecordStart is required before any audio will be broadcast to
active channels.
For manual operation, either never call VoiceActivateLevel or call it
with 0 for the level.
--------------------------------------------------------------------------
level   | Automatic voice capture activation level. 0 - 32767
------------------------------------------------------------------------*/
void VoiceActivateLevel(unsigned short level);

/*------------------------------------------------------------------------
VoiceGetActivateLevel
--------------------------------------------------------------------------
VoiceGetActivateLevel returns the automatic voice level of the capture
device. The level ranges from 1 to 32767. For example, if a level
of 1500 is set, then any audio coming from the registered
capture device that breaks the amplitude level of 1500 will
automatically be broadcast on all active channels.
When set for manual activation, the return value is 0.
--------------------------------------------------------------------------
return   | Automatic voice capture activation level. 0 or 1 - 32767
------------------------------------------------------------------------*/
unsigned short VoiceGetActivateLevel();

/*------------------------------------------------------------------------
VoiceGetInputLevel
--------------------------------------------------------------------------
VoiceGetInputLevel gets the level of the voice input being captured.
The range is 0 - 32767.
--------------------------------------------------------------------------
return   | Current voice input level. 0 - 32767
------------------------------------------------------------------------*/
unsigned short VoiceGetInputLevel();

/*------------------------------------------------------------------------
VoiceRecordStart
--------------------------------------------------------------------------
The VoiceRecordStart function is used to broadcast the captured
audio to the specified channel.
If the channel reference pointer is set to NULL, then all active
channels will receive the captured audio.
A call to VoiceRecordStart will automatically set the automatic voice
activation level to 0, effectively turning off automatic voice
activation if it is set.
--------------------------------------------------------------------------
channel  | Channel reference pointer
------------------------------------------------------------------------*/
void VoiceRecordStart(Channel channel);

/*------------------------------------------------------------------------
VoiceRecordStop
--------------------------------------------------------------------------
VoiceRecordStop is used to stop the broadcast of captured audio
to the specified channel.
If the channel reference pointer is set to NULL, then all active
channels will stop receiving the captured audio.
A call to VoiceRecordStop will automatically set the automatic voice
activation level to 0, effectively turning off automatic voice
activation if it is set.
--------------------------------------------------------------------------
channel  | Channel reference pointer
------------------------------------------------------------------------*/
void VoiceRecordStop(Channel channel);

/*------------------------------------------------------------------------
VoiceGetChannelName
--------------------------------------------------------------------------
VoiceGetChannelName will return the name of the specfied channel.
--------------------------------------------------------------------------
channel  | Channel reference pointer
return   | The name of the channel
------------------------------------------------------------------------*/
const char * VoiceGetChannelName(Channel channel);

/*------------------------------------------------------------------------
VoiceGetMemberCount
--------------------------------------------------------------------------
VoiceGetMemberCount will return the number of members on the specfied
channel.
--------------------------------------------------------------------------
channel  | Channel reference pointer
return   | The number of members on the specified channel
------------------------------------------------------------------------*/
unsigned int VoiceGetMemberCount(Channel channel);

/*------------------------------------------------------------------------
VoiceChannelLocalMember
--------------------------------------------------------------------------
VoiceChannelLocalMember will return the member reference number
of the local member of the specified channel.
--------------------------------------------------------------------------
channel  | Channel reference pointer
return   | Channel member reference number, or an INVALID_MEMBERID if 
         | the channel is invalid
------------------------------------------------------------------------*/
MemberId VoiceChannelLocalMember(Channel channel);

/*------------------------------------------------------------------------
VoiceChannelHostMember
--------------------------------------------------------------------------
VoiceChannelHostMember will return the member reference number
of the host member of the specified cahnnel.
--------------------------------------------------------------------------
channel  | Channel reference pointer
return   | Channel member reference number, or an INVALID_MEMBERID if 
         | the channel is invalid
------------------------------------------------------------------------*/
MemberId VoiceChannelHostMember(Channel channel);

/*------------------------------------------------------------------------
VoiceChannelMemberByIndex
--------------------------------------------------------------------------
VoiceChannelMemberByIndex will return the member reference number
of the member at the specified index location.

Member indexes range from 0 to (VoiceGetMemberCount)-1
--------------------------------------------------------------------------
channel  | Channel reference pointer
index    | Index number of the channel member
return   | Channel member reference for the member at index position, 
         | or an INVALID_MEMBERID if the channel is invalid
------------------------------------------------------------------------*/
MemberId VoiceChannelMemberByIndex(Channel channel, unsigned int index);

/*------------------------------------------------------------------------
VoiceChannelMemberReachable
--------------------------------------------------------------------------
VoiceChannelMemberReachable determines with the specified member
is reachable directly or if they must be reached through the channel
host.

No special action should be taken if a member is not directly 
reachable
--------------------------------------------------------------------------
channel  | Channel reference pointer
memberid | Channel member reference number
--------------------------------------------------------------------------
return   | If the member is directly reachable, then voiceTrue is 
	     | returned.
	     | If the member is not directly reachable, then voiceFalse is 
	     | returned
------------------------------------------------------------------------*/
voiceBool VoiceChannelMemberReachable(Channel channel, MemberId memberid);

/*------------------------------------------------------------------------
VoiceChannelMemberIsTalking
--------------------------------------------------------------------------
VoiceChannelMemberIsTalking returned the talking state of the
specified member.
--------------------------------------------------------------------------
channel  | Channel reference pointer
memberid | Channel member reference number
return   | If the member is actively broadcasting audio, then
	     | voiceTrue is returned
	     | If the member is not actively broadcasting audio, then
	     | voiceFalse is returned
------------------------------------------------------------------------*/
voiceBool VoiceChannelMemberIsTalking(Channel channel, MemberId memberid);

/*------------------------------------------------------------------------
VoiceChannelSendListAdd
--------------------------------------------------------------------------
VoiceChannelSetListAdd adds members to the channels broadcast list.
Any members on the list will receive any broadcasted audio. 
The maximum number of recipiants on a send list is 64 (or MAX_SENDLIST).
The local member cannot be added to the send list.
--------------------------------------------------------------------------
channel  | Channel reference pointer
memberid | Channel member reference number
return   | if voiceTrue, the MemberId was added
	     | if voiceFalse, the MemberId was unable to be added
------------------------------------------------------------------------*/
voiceBool VoiceChannelSendListAdd(Channel channel, MemberId memberid);

/*------------------------------------------------------------------------
VoiceChannelSendListReset
--------------------------------------------------------------------------
VoiceChannelSendListReset clears the active member list. The broadcast
member list is a list of all member who should receive any audio
broadcasted on the channel. When the send list is reset, all members
of the channel will receive any broadcasted audio.
--------------------------------------------------------------------------
channel  | Channel reference pointer
------------------------------------------------------------------------*/
void VoiceChannelSendListReset(Channel channel);

/*------------------------------------------------------------------------
VoiceChannelSendListCount
--------------------------------------------------------------------------
VoiceChannelSendListCount the number of MemberIds on the channel's 
current send list. 
--------------------------------------------------------------------------
channel  | Channel reference pointer
return   | Number of MemberIds on the current send list.
	     | If zero, there is no current send list and all members
	     | of the channel will receive any broadcasted audio.
------------------------------------------------------------------------*/
unsigned short VoiceChannelSendListCount(Channel channel);

/*------------------------------------------------------------------------
VoiceChannelIsMemberOnSendList
--------------------------------------------------------------------------
VoiceChannelIsMemberOnSendList checks if a member is on the send list for
this channel.  The local member is always considered to be on the list.
--------------------------------------------------------------------------
channel  | Channel reference pointer
memberid | Channel member reference number
return   | if voiceTrue, the MemberId is on the list
	     | if voiceFalse, the MemberId is not on the list
------------------------------------------------------------------------*/
voiceBool VoiceChannelIsMemberOnSendList(Channel channel, MemberId memberid);

/*------------------------------------------------------------------------
VoiceChannelSendListGetMember
--------------------------------------------------------------------------
Complimentary to VoiceChannelSendListCount, allows access to individual 
memberid's on the current send list.
--------------------------------------------------------------------------
channel  | Channel reference pointer
memberid | Channel member reference number
return   | MemberId for the index, or INVALID_MEMBERID for error.
------------------------------------------------------------------------*/
MemberId VoiceChannelSendListGetMember( Channel channel, unsigned short index );

/*------------------------------------------------------------------------
VoiceChannelKickMember
--------------------------------------------------------------------------
When VoiceChannelKickMember is called by the channel host, the specified
member is removed from the channel.

The kicked member is allowed to rejoin the channel.
--------------------------------------------------------------------------
channel  | Channel reference pointer
memberid | Channel member reference number
------------------------------------------------------------------------*/
void VoiceChannelKickMember(Channel channel, MemberId memberid);

/*------------------------------------------------------------------------
VoiceChannelBanMember
--------------------------------------------------------------------------
When VoiceChannelBanMember is called by the channel host, the specified
member is removed from the channel.

The removed member is not allowed to rejoin the channel as long as the
channel exists.
--------------------------------------------------------------------------
channel  | Channel reference pointer
memberid | Channel member reference number
------------------------------------------------------------------------*/
void VoiceChannelBanMember(Channel channel, MemberId memberid);

/*------------------------------------------------------------------------
VoiceChannelClearBans
--------------------------------------------------------------------------
Clears all bans placed on a channel with VoiceChannelBanMember.  All
banned members will be allowed to rejoin.
--------------------------------------------------------------------------
channel  | Channel reference pointer
------------------------------------------------------------------------*/
void VoiceChannelClearBans(Channel channel);

/*------------------------------------------------------------------------
VoiceSendMiscData
--------------------------------------------------------------------------
VoiceSendMiscData allows a member to send abitrary data to other
members of the channel. If there is an active send list created by
VoiceChannelSendListAdd then only those members in the list will
receive the provided data.
--------------------------------------------------------------------------
channel    | Channel reference pointer
data       | Buffer containing the miscellaneous data
datalength | Number containing the length of data
------------------------------------------------------------------------*/
void VoiceSendMiscData(
	Channel channel, 
	void *data,
	unsigned short datalength
);

/*------------------------------------------------------------------------
VoiceChannelHost
--------------------------------------------------------------------------
VoiceChannelHost determines if the specified member of the channel is
the channel host.
--------------------------------------------------------------------------
channel  | Channel reference pointer
memberid | Channel member reference number
return   | If the local member is the channel host, then voiceTrue
	     | is returned.
	     | If the local member is not the channel host, then voiceFalse
	     | is returned.
------------------------------------------------------------------------*/
voiceBool VoiceChannelHost(Channel channel, MemberId memberid);

/*************************************************************************
*	KEY / VALUE FUNCTIONS
*************************************************************************/

/*------------------------------------------------------------------------
VoiceGetMemberString
--------------------------------------------------------------------------
VoiceGetMemberString receives a NULL termiated string that has
been associated with the key for the channel member
--------------------------------------------------------------------------
channel  | Channel reference pointer
memberid | Channel member reference number
key      | NULL terminated string containing the key to look up
def      | default value to return if the key was not found
return   | NULL terminated string containing the value associated
         | with the supplied key
------------------------------------------------------------------------*/
const char *VoiceGetMemberString(Channel channel, MemberId memberid, const char *key, const char *def);

/*------------------------------------------------------------------------
VoiceGetMemberInt
--------------------------------------------------------------------------
VoiceGetMemberInt receives an 'int' value that has been associated
with the key for the channel member
--------------------------------------------------------------------------
channel  | Channel reference pointer
memberid | Channel member reference number
key      | NULL terminated string containing the key to look up
def      | default value to return if the key was not found
return   | 'int' containing the value associated with the supplied
         | key
------------------------------------------------------------------------*/
int VoiceGetMemberInt(Channel channel, MemberId memberid, const char *key, int def);

/*------------------------------------------------------------------------
VoiceSetMemberString
--------------------------------------------------------------------------
VoiceSetMemberString associates a NULL terminated string with the 
specified key for the channel member
Changes to member key/values are transmitted the remote memberid, if 
hosting the channel, changes are broadcast to all members of the channel.
This will generate a memberdescchanged callback.
--------------------------------------------------------------------------
channel  | Channel reference pointer
memberid | Channel member reference number
key      | NULL terminated string containing the key to look up
value    | NULL terminated string value to set
return   | voiceTrue if successful, voiceFalse if a failure occured
------------------------------------------------------------------------*/
voiceBool VoiceSetMemberString(Channel channel, MemberId memberid, const char *key, const char *value);

/*------------------------------------------------------------------------
VoiceSetMemberInt
--------------------------------------------------------------------------
VoiceSetMemberInt associates an 'int' value with the specified key
for the channel member
Changes to member key/values are transmitted the remote memberid, if 
hosting the channel, changes are broadcast to all members of the channel.
This will generate a memberdescchanged callback.
--------------------------------------------------------------------------
channel  | Channel reference pointer
memberid | Channel member reference number
key      | NULL terminated string containing the key to look up
value    | 'int' value to set
return   | voiceTrue if successful, voiceFalse if a failure occured
------------------------------------------------------------------------*/
voiceBool VoiceSetMemberInt(Channel channel, MemberId memberid, const char *key, int value);

/*------------------------------------------------------------------------
VoiceEnumMemberKeys
--------------------------------------------------------------------------
Enumerates the keys/values for a given MemberId in a Channel by calling 
VoiceKeyEnumFn with each key/value. The user-defined instance data will 
be passed to the VoiceKeyFn callback
--------------------------------------------------------------------------
channel  | Channel reference pointer
memberid | Channel member reference number
KeyFn    | Address of the callback function that receives the individual 
         | Key/Value pairs as strings.
instance | user defined instance data to pass to the callback
------------------------------------------------------------------------*/
void VoiceEnumMemberKeys(Channel channel, MemberId memberid, VoiceKeyEnumFn KeyFn, void* instance);

/*------------------------------------------------------------------------
VoiceMuteMemberPlayback
--------------------------------------------------------------------------
VoiceMuteMemberPlayback allows the muting of the specified MemberId
--------------------------------------------------------------------------
channel  | Channel reference pointer
memberid | Channel member reference number
mute     | voiceTrue to silence all incoming audio and audio events from 
         | the member. voiceFalse to allow audio to play (voice sdk default)
return   | voiceTrue if successful, voiceFalse if a failure occured
------------------------------------------------------------------------*/
voiceBool VoiceMuteMemberPlayback(Channel channel, MemberId memberid, voiceBool mute);

/*------------------------------------------------------------------------
VoiceIsMemberPlaybackMuted
--------------------------------------------------------------------------
VoiceIsMemberPlaybackMuted determines if the member is muted
--------------------------------------------------------------------------
channel  | Channel reference pointer
memberid | Channel member reference number
return   | voiceTrue if muted, voiceFalse not muted
------------------------------------------------------------------------*/
voiceBool VoiceIsMemberPlaybackMuted(Channel channel, MemberId memberid);

/*------------------------------------------------------------------------
VoiceGetChannelString
--------------------------------------------------------------------------
VoiceGetChannelString receives a NULL termiated string that has
been associated with the key for the channel
--------------------------------------------------------------------------
channel  | Channel reference pointer
key      | NULL terminated string containing the key to look up
def      | default value to return if the key was not found
return   | NULL terminated string containing the value associated
         | with the supplied key
------------------------------------------------------------------------*/
const char *VoiceGetChannelString(Channel channel, const char *key, const char *def);

/*------------------------------------------------------------------------
VoiceGetChannelInt
--------------------------------------------------------------------------
VoiceGetChannelInt receives an 'int' value that has been associated
with the key for the channel
--------------------------------------------------------------------------
channel  | Channel reference pointer
key      | NULL terminated string containing the key to look up
def      | default value to return if the key was not found
return   | 'int' containing the value associated with the supplied
	     | key
------------------------------------------------------------------------*/
int VoiceGetChannelInt(Channel channel, const char *key, int def);

/*------------------------------------------------------------------------
VoiceGetChannelBool
--------------------------------------------------------------------------
VoiceGetChannelBool receives a boolean value that has been associated
with the key for the channel
--------------------------------------------------------------------------
channel  | Channel reference pointer
key      | NULL terminated string containing the key to look up
def      | default value to return if the key was not found
return   | voiceBool containing the value associated with the supplied
         | key
------------------------------------------------------------------------*/
voiceBool VoiceGetChannelBool(Channel channel, const char *key, voiceBool def);

/*------------------------------------------------------------------------
VoiceEnumChannelKeys
--------------------------------------------------------------------------
Enumerates the keys/values for a given Channel by calling VoiceKeyEnumFn 
with each key/value. The user-defined instance data will be passed to the 
VoiceKeyFn callback
--------------------------------------------------------------------------
channel  | Channel reference pointer
KeyFn    | Address of the callback function that receives the individual 
         | Key/Value pairs as strings.
instance | user defined instance data to pass to the callback
------------------------------------------------------------------------*/
void VoiceEnumChannelKeys(Channel channel, VoiceKeyEnumFn KeyFn, void* instance);

/*------------------------------------------------------------------------
VoiceSetChannelString
--------------------------------------------------------------------------
VoiceSetChannelString associates a NULL terminated string with
the specified key for the channel
--------------------------------------------------------------------------
channel  | Channel reference pointer
key      | NULL terminated string containing the key to look up
value    | NULL terminated string containing the value to set
return   | voiceTrue if successful, voiceFalse if a failure occured
------------------------------------------------------------------------*/
voiceBool VoiceSetChannelString(Channel channel, const char *key, const char *value);

/*------------------------------------------------------------------------
VoiceSetChannelInt
--------------------------------------------------------------------------
VoiceSetChannelInt associates an 'int' value with the specified
key for the channel
--------------------------------------------------------------------------
channel  | Channel reference pointer
key      | NULL terminated string containing the key to look up
value    | 'int' containing the value to set
return   | voiceTrue if successful, voiceFalse if a failure occured
------------------------------------------------------------------------*/
voiceBool VoiceSetChannelInt(Channel channel, const char *key, int value);

/*------------------------------------------------------------------------
VoiceSetChannelBool
--------------------------------------------------------------------------
VoiceSetChannelBool associates a boolean value with the specified
key for the channel
--------------------------------------------------------------------------
channel  | Channel reference pointer
key      | NULL terminated string containing the key to look up
value    | voiceBool containing the value to set
return   | voiceTrue if successful, voiceFalse if a failure occured
------------------------------------------------------------------------*/
voiceBool VoiceSetChannelBool(Channel channel, const char *key, voiceBool value);

/*------------------------------------------------------------------------
VoiceGetChannelUserData
--------------------------------------------------------------------------
VoiceGetChannelUserData receives previously set user data from the channel
--------------------------------------------------------------------------
channel  | Channel reference pointer
return   | previously set user data
------------------------------------------------------------------------*/
unsigned long VoiceGetChannelUserData(Channel channel);

/*------------------------------------------------------------------------
VoiceSetChannelUserData
--------------------------------------------------------------------------
VoiceSetChannelUserData associates user data with the channel
--------------------------------------------------------------------------
channel  | Channel reference pointer
userdata | private data to associate with the Channel reference pointer
return   | voiceTrue if successful, voiceFalse if a failure occured
------------------------------------------------------------------------*/
voiceBool VoiceSetChannelUserData(Channel channel, unsigned long userdata);

/*------------------------------------------------------------------------
VoiceMuteChannelPlayback
--------------------------------------------------------------------------
VoiceMuteChannelPlayback allows the muting of the specified channel
--------------------------------------------------------------------------
channel  | Channel reference pointer
mute     | voiceTrue to silence all incoming audio and audio events from 
         | the channel. voiceFalse to allow audio to play (voice sdk default)
return   | voiceTrue if successful, voiceFalse if a failure occured
------------------------------------------------------------------------*/
voiceBool VoiceMuteChannelPlayback(Channel channel, voiceBool mute);

/*------------------------------------------------------------------------
VoiceIsChannelPlaybackMuted
--------------------------------------------------------------------------
VoiceIsChannelPlaybackMuted determines if the channel is muted
--------------------------------------------------------------------------
channel  | Channel reference pointer
return   | voiceTrue if muted, voiceFalse not muted
------------------------------------------------------------------------*/
voiceBool VoiceIsChannelPlaybackMuted(Channel channel);

/*------------------------------------------------------------------------
Closes the audio capture device without shutting down everything
------------------------------------------------------------------------*/
void VoiceCaptureClose();

/*------------------------------------------------------------------------
re-opens the capture device
--------------------------------------------------------------------------
return   | voiceTrue on success, voiceFalse on failure
------------------------------------------------------------------------*/
voiceBool VoiceCaptureOpen();

/*------------------------------------------------------------------------
determines if the capture device is open
--------------------------------------------------------------------------
return   | voiceTrue if the capture device is open
------------------------------------------------------------------------*/
voiceBool VoiceCaptureIsOpen();

/*------------------------------------------------------------------------
Closes the audio playback device without shutting down everthing
------------------------------------------------------------------------*/
void VoicePlaybackClose();

/*------------------------------------------------------------------------
re-opens the audio playback device
--------------------------------------------------------------------------
return   | voiceTrue on success, voiceFalse on failure
------------------------------------------------------------------------*/
voiceBool VoicePlaybackOpen();

/*------------------------------------------------------------------------
determines if the playback device is open
--------------------------------------------------------------------------
return   | voiceTrue if the playback device is open
------------------------------------------------------------------------*/
voiceBool VoicePlaybackIsOpen();

/*------------------------------------------------------------------------
sets the capture interface to use
--------------------------------------------------------------------------
return   | the previous capture interface
------------------------------------------------------------------------*/
VoiceCapture* VoiceSetCaptureInterface(VoiceCapture* voicecapture);

/*------------------------------------------------------------------------
sets the playback interface to use
--------------------------------------------------------------------------
return   | the previous playback interface
------------------------------------------------------------------------*/
VoicePlayback* VoiceSetPlaybackInterface(VoicePlayback* voiceplayback);

#ifdef __cplusplus
}
#endif

#endif /* _VOICECHANNEL_H */
