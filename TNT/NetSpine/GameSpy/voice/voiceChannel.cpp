/*****************************************************************
voiceChannel.c
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

//INCLUDES
//////////
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../nonport.h"
#include "voice.h"
#include "voiceNet.h"

//DEFINES
/////////
#define NETWORKVERSION		(unsigned char)0x04
#define MAX_CHANNELS		24
#define MAX_INCOMING		64
#define MAX_SENDLIST		64
#define MAX_BANLIST         32
#define MAXMIXERSIZE		2048 // max size of the mixed samples
#define JITTER_SIZE			2000
#define ACTIVATION_LEVEL_CUTOFF_TIME     1500
#define MAX_INCOMING_BUFFER_TIME         5000

//MEMBER FLAGS
#define MFLAG_UNREACHABLE	0x00000001

//ENUMS
///////

//COMMANDS
enum
{
	// login
	NET_LOGIN,
	NET_REDIRECT,
	NET_REHOST,

	// errors
	NET_BADVERSION,
	NET_NOSUCHCHANNEL,
	NET_BANNED,
	NET_BADPASSWORD,
	NET_CHANNELFULL,
	NET_REHOSTFAILED,

	// member management commands
	NET_ADDMEMBER,
	NET_REMOVEMEMBER,
	NET_MEMBERFLAG_CHANGED,
	NET_MEMBERDESC_CHANGED,

	// transmission commands
	NET_RELAYDATA,
	NET_RELAYDATA_LIST,
	NET_RELAYPARTIAL,

	// functions
	NET_VOICEDATA,
	NET_VOICEDATA_LIST,
	NET_VOICEBROADCAST,
	NET_MISCDATA,

	// ping/pong
	NET_PING,
	NET_PONG
};

//CHANNEL STATES
typedef enum _ChannelState
{
	STATE_INITIALIZED,
	STATE_LOGIN,
	STATE_CONNECTING,
	STATE_ACTIVE,
	STATE_REHOSTING,
	STATE_REMOVE
} ChannelState;

// MESSAGE RELIABILITY
enum
{
	MSG_UNRELIABLE,
	MSG_RELIABLE
};

// REMOVE MEMBER REASON
enum
{
	NET_RM_DISCONNECTED,
	NET_RM_KICKED,
	NET_RM_BANNED
};

//STRUCTS
/////////
typedef struct
{
	unsigned short encoded_length;
	char *encoded_data;

	unsigned int expand; // amount of samples to expand to remove jitter

	unsigned int processed_length;
	short *processed_data;
} AudioData;

typedef struct
{
	unsigned int flags;
	char *address;
	unsigned short port;
	unsigned int channel_ref;
	KeyVal kv;
	SOCKET TCPSocket;
	MemberId memberid;
	unsigned short relay;
	voiceBool IsTalking;
	
	unsigned long timestamp;
	int expand_history;
	AudioData audiodata;
	CodecState decode_state;

	voiceBool mute;
} _Member;

typedef struct _Channel
{
	unsigned int flags;
	SOCKET TCPHostSocket;
	VoiceCodec *codec;
	CodecState encode_state;
	VoiceCallbacks Callbacks;
	ChannelState state;
	voiceBool IsHost;
	KeyVal kv;
	char *address;
	unsigned short port;
	_Member *local_member;
	_Member *host_member;
	_Member **members;
	unsigned int num_members;
	unsigned int allocated_members;
	unsigned int channel_ref;
	MemberId memberid;
	voiceBool justStoppedTalking;

	unsigned short sendlist_len;
	MemberId sendlist[MAX_SENDLIST];

	unsigned short banlist_len;
	char *banlist[MAX_BANLIST];

	short * excessAudio;
	int excessAudioLen;
	unsigned long userdata;
	voiceBool mute;
} _Channel;

//GLOBALS
/////////
static voiceBool gInitialized = voiceFalse;
static _Channel *g_Channel[MAX_CHANNELS];
static VoiceCapture *g_VoiceCapture = NULL;
static VoicePlayback *g_VoicePlayback = NULL; 
SOCKET gTCPSocket = INVALID_SOCKET;
SOCKET gUDPSocket = INVALID_SOCKET;
unsigned short gPort = DEFAULT_VOICE_PORT;
static SOCKET g_InSockets[MAX_INCOMING];
static unsigned int channel_ref = 0;
static Packet g_Packet = NULL;
static unsigned long gLastActivatedTime;
static voiceBool gGotFirstCapture;
static int gLastInputLevel;
static int gActivateLevel;

//FUNCTION PROTOTYPES
/////////////////////
static void RemoveChannelProcess(_Channel *c);
static _Member* VoiceChannelMemberById(_Channel *c, MemberId memberid);

//FUNCTIONS
///////////

voiceBool static isValidChannel(_Channel *c)
{
	int i = 0;

	if(!c)
		return voiceFalse;

	for(i = 0; i < MAX_CHANNELS; i++)
	{
		if(g_Channel[i] == c)
			return voiceTrue;
	}

	return voiceFalse;
}

static void BroadcastPacket(_Channel *c, Packet p)
{
	unsigned int i;
	_Member *m;

	if(!isValidChannel(c))
	{
		assert(0);
		return;
	}

	assert(c->IsHost);
	if(!c->IsHost)
		return;
	
	for(i = 0; i < c->num_members; i++)
		if((m = c->members[i]) && (m != c->local_member) && (m->TCPSocket != INVALID_SOCKET))
			SendPacketTCP(m->TCPSocket, p);
}

static void SortMembers(_Channel *c)
{
	unsigned int i, j;
	_Member *m1, *m2;

	if(!isValidChannel(c))
	{
		assert(0);
		return;
	}

	assert(c->members);
	if(!c->members)
		return;

	// first sort the members by their member ids
	for( i = 0; i < c->num_members; i++ )
	{
		m1 = c->members[i];
		for( j = i + 1; j < c->num_members; j++ )
		{
			m2 = c->members[j];
			
			if( m2 && ( !m1 || m2->memberid < m1->memberid ) )
			{
				// swap
				c->members[i] = m2;
				c->members[j] = m1;
				m1 = m2;
			}
		}
	}
}

static _Member *AllocMember(void)
{
	_Member *m = (_Member *)gsimalloc(sizeof(_Member));

	assert(m);
	if(!m)
		return NULL;

	memset(m, 0, sizeof(_Member));

	return m;
}

static void AddMember(_Channel *c, _Member *m)
{
	if(!isValidChannel(c))
	{
		assert(0);
		return;
	}

	// Reallocate memory for the members list on the  1st member
	// then every 16th member addition.
	if(!c->num_members || (c->allocated_members / sizeof(int)) <= c->num_members)
	{
		c->allocated_members += 16 * sizeof(int);
		c->members = (_Member**)gsirealloc(c->members, c->allocated_members);
		assert(c->members);
	}

	m->decode_state = c->codec->create_state(stateDecode);
	c->members[c->num_members++] = m;
	SortMembers(c);
}

static voiceBool RemoveMember(_Channel *c, _Member *m)
{
	unsigned int i;

	if(!c || !m)
		return voiceFalse;

	for( i = 0; i < c->num_members; i++ )
	{
		if(c->members[i] == m)
		{
			if(m->decode_state)
				c->codec->destroy_state(m->decode_state);
			if(m->audiodata.encoded_length)
				gsifree(m->audiodata.encoded_data);
			if(m->audiodata.processed_length)
				gsifree(m->audiodata.processed_data);
			if(m->address)
				gsifree(m->address);
			if(m->kv)
				VoiceDeleteKeyVal(m->kv);
			VoiceNetCloseSocket(m->TCPSocket);
			gsifree(m);
			c->members[i] = NULL;
			SortMembers(c);
			// this must be called after SortMembers otherwise 
			// SortMembers may not catch the last member -rich
			c->num_members--;
			return voiceTrue;
		}
	}
	return voiceFalse;
}

static void BroadcastRemoveMember(_Channel *c, _Member *m, int reason)
{
	if(!c || !m)
		return;

	if(!isValidChannel(c))
	{
		assert(0);
		return;
	}
	
	PacketClear(g_Packet);
	PacketAddByte(g_Packet, NET_REMOVEMEMBER);
	PacketAddByte(g_Packet, (unsigned char)reason);
	PacketAddString(g_Packet, m->address);
	PacketAddInt(g_Packet, m->port);
	BroadcastPacket(c, g_Packet);
}

static _Member *MemberById(_Channel *channel, MemberId id)
{
	unsigned int i;
	
	if(!channel)
		return NULL;

	for(i = 0; i < channel->num_members; i++)
		if(channel->members[i]->memberid == id)
			return channel->members[i];

	return NULL;
}

static _Channel * AllocChannel()
{
	int i;
	_Channel *_channel = NULL;

	for(i = 0; i < MAX_CHANNELS; i++ )
	{
		if(!g_Channel[i])
		{
			_channel = g_Channel[i] = (_Channel *)gsimalloc(sizeof(_Channel));
			if(!_channel)
				return NULL;

			memset(_channel, 0, sizeof(_Channel));
			_channel->kv = NULL;
			_channel->num_members = 0;
			_channel->channel_ref = channel_ref++;
			_channel->local_member = NULL;
			_channel->kv = VoiceInitKeyVal(NULL, 0);
			if(!_channel->kv)
			{
				gsifree(_channel);
				g_Channel[i] = NULL;
				return NULL;
			}

			return _channel;
		}
	}

	return NULL;
}

static void SendVoiceData(unsigned int channel_ref, MemberId memberid,
				   _Member *to, char *out, unsigned short len, int reliable)
{
	Packet p = g_Packet;
	PacketClear(p);
	PacketAddByte(p, NET_VOICEDATA);
	PacketAddInt(p, channel_ref);
	PacketAddShort(p, memberid);
	PacketAddBlob(p, out, len);

	if(reliable == MSG_RELIABLE)
		SendPacketTCP(to->TCPSocket, p);
	else
		SendPacketUDP(to->address, to->port, p);
}

static void SendVoiceBroadcast(_Channel *c, char *out, unsigned short len,
						int reliable)
{
	Packet p = g_Packet;
	PacketClear(p);
	PacketAddByte(p, NET_VOICEBROADCAST);
	PacketAddInt(p, c->host_member->channel_ref);
	PacketAddShort(p, c->local_member->memberid);
	PacketAddBlob(p, out, len);

	if(reliable == MSG_RELIABLE)
		SendPacketTCP(c->TCPHostSocket, p);
}

static void RelayData(_Member *m, MemberId memberid, char *out, unsigned short len, int reliable)
{
	Packet p = g_Packet;
	PacketClear(p);
	PacketAddByte(p, NET_RELAYDATA);
	PacketAddInt(p, m->channel_ref);
	PacketAddShort(p, memberid);
	PacketAddBlob(p, out, len);

	if(reliable == MSG_UNRELIABLE)
		SendPacketUDP(m->address, m->port, p);
}

static void VoicePublish(_Channel *c, char *out, unsigned short len)
{
	unsigned int i;
	_Member *m;

	if(!isValidChannel(c))
	{
		assert(0);
		return;
	}

	if(c->sendlist_len)
	{
		if(c->flags & CFLAG_PEERTOPEER)
		{
			for(i = 0; i < c->sendlist_len; i++)
			{
				m = VoiceChannelMemberById(c, c->sendlist[i]);
				if(m)
					SendVoiceData(c->channel_ref, c->local_member->memberid, m, out, len, MSG_UNRELIABLE);
			}
		}
		else // CFLAG_PEERRELAY / CFLAG_HOSTTOPEER
		{
			Packet p = g_Packet;

			PacketClear(p);

			PacketAddByte(p, NET_VOICEDATA_LIST);
			PacketAddInt(p, c->host_member->channel_ref);
			PacketAddShort(p, c->local_member->memberid);
			PacketAddShort(p, c->sendlist_len);
			for(i = 0; i < c->sendlist_len; i++)
				PacketAddShort(p, c->sendlist[i]);
			PacketAddBlob(p, out, len);
			SendPacketUDP(c->host_member->address, c->host_member->port, p);
		}

		return;
	}
	
	// if the local echo flags is set then we must send our own captured audio back to ourseleves for playback
	if(c->flags & CFLAG_LOCALECHO)
		SendVoiceData(c->channel_ref, c->local_member->memberid, c->local_member, out, len, MSG_UNRELIABLE);

	// send this audio out to all unreachable members
	for(i = 0; c->IsHost && i < c->num_members; i++)
		if((m = c->members[i]) && (m->flags & MFLAG_UNREACHABLE))
			SendVoiceData(c->channel_ref, c->local_member->memberid, m, out, len, MSG_RELIABLE);

	if(!c->IsHost && c->local_member->flags & MFLAG_UNREACHABLE)
		SendVoiceBroadcast(c, out, len, MSG_RELIABLE);
	else if(c->flags & CFLAG_PEERTOPEER)
	{
		for(i = 0; i < c->num_members; i++)
			if((m = c->members[i]) && (m != c->local_member))
				SendVoiceData(m->channel_ref, c->local_member->memberid, m, out, len, MSG_UNRELIABLE);
	}
	else if(c->flags & CFLAG_HOSTTOPEER)
	{
		if(c->IsHost)
		{
			for(i = 0; i < c->num_members; i++)
			{
				if((m = c->members[i]) && (m != c->local_member))
					SendVoiceData(m->channel_ref, c->local_member->memberid, m, out, len, MSG_UNRELIABLE);
			}
		}
		else
		{
			SendVoiceBroadcast(c, out, len, MSG_RELIABLE);
		}
	}
	else // CFLAG_PEERRELAY (default)
	{
		unsigned int member_index = 0;
		for(i = 0; i < c->local_member->relay; i++, member_index++)
		{
			// we can't relay to ourselves
			if((m = c->members[member_index]) && (m == c->local_member))
				member_index++;

			if(member_index >= c->num_members)
				break;

			m = c->members[member_index];
			if(m)
				RelayData(m, c->local_member->memberid, out, len, MSG_UNRELIABLE);
		}
	}
}

static void CaptureCBF(AudioSample *sample)
{
	unsigned int i, j;
	int inlevel = 0;
	unsigned long currenttime;
	unsigned short len = 0;
	unsigned char out[MAXPACKETSIZE];
	_Channel * channel = NULL;
	int excess;
	int filler;
	unsigned int sampleLength;
	short * sampleData;

	// get the current time
	currenttime = current_time();
	if(!gGotFirstCapture)
	{
		gGotFirstCapture = voiceTrue;
		gLastActivatedTime = (currenttime - ACTIVATION_LEVEL_CUTOFF_TIME - 1000);
	}

	// get the input level
	for( i = 0; i < sample->length; i++ )
	{
		if(abs(sample->data[i]) > inlevel)
			inlevel = abs(sample->data[i]);
	}

	// store it for VoiceGetInputLevel
	gLastInputLevel = inlevel;

	// check if it's voice activated, and if we're not talking
	if((inlevel < gActivateLevel) && ((currenttime - gLastActivatedTime) > ACTIVATION_LEVEL_CUTOFF_TIME))
	{
		// check where we're talking and stop it
		for(i = 0; i < MAX_CHANNELS; i++)
		{
			channel = g_Channel[i];
			if(channel && channel->state == STATE_ACTIVE && channel->local_member)
			{
				if(channel->local_member->IsTalking)
				{
					channel->local_member->IsTalking = voiceFalse;
					if(channel->Callbacks.talkingstopped_cbf)
						channel->Callbacks.talkingstopped_cbf(channel, channel->local_member->memberid, channel->Callbacks.context);
				}
			}
		}

		// not activated, so stop
		return;
	}

	// check if we're in activation mode, and above the activation level
	if(gActivateLevel && (inlevel >= gActivateLevel))
		gLastActivatedTime = currenttime;

	// loop through all open channels, run through codec and publish
	for( j = 0; j < MAX_CHANNELS; j++ )
	{
		channel = g_Channel[j];

		// check the channel
		if(!channel || channel->state != STATE_ACTIVE || !channel->local_member)
			continue;

		// check if we've just activated this channel
		if((gActivateLevel > 0) && !channel->local_member->IsTalking)
		{
			channel->local_member->IsTalking = voiceTrue;
			if(channel->Callbacks.talkingstarted_cbf)
				channel->Callbacks.talkingstarted_cbf(channel, channel->local_member->memberid, channel->Callbacks.context);
		}

		// check if we're not talking in this channel
		if(!channel->local_member->IsTalking && !channel->justStoppedTalking)
			continue;

		// local echo may be turned off, forward the captured wave data to the app.
		// It's a nice touch to display a waveform in-game, as it immediatly informs the user that their microphone is working.
		if( (channel->flags&CFLAG_LOCALECHO)!=CFLAG_LOCALECHO )
		{
			if(channel->Callbacks.voicedata_cbf && channel->local_member->IsTalking)
				channel->Callbacks.voicedata_cbf(channel, channel->local_member->memberid, sample, channel->Callbacks.context);
		}

		// clear out the encoded audio length
		len = 0;

		// check if we have excess
		if(channel->excessAudioLen)
		{
			// calculate how much filler we need
			filler = (channel->codec->samples_per_frame - channel->excessAudioLen);

			// check for not enough filler
			if(filler > (int)sample->length)
			{
				// copy over what we can and exit, we can't make a full frame of audio
				memcpy(channel->excessAudio + channel->excessAudioLen, sample->data, sample->length * 2);
				channel->excessAudioLen += sample->length;
				continue;
			}

			// fill out the excess buffer
			memcpy(channel->excessAudio + channel->excessAudioLen, sample->data, filler * 2);
			channel->excessAudioLen += filler;

			// readjust the sample
			sampleData = (sample->data + filler);
			sampleLength = (sample->length - filler);

			// use the excess for the first encoding
			len += channel->codec->encode(&out[len], channel->excessAudio, channel->encode_state);
			channel->excessAudioLen = 0;
		}
		else
		{
			sampleData = sample->data;
			sampleLength = sample->length;
		}

		// do the encoding for the channel
		for( i = 0 ; (i < (sampleLength / channel->codec->samples_per_frame)) && (len < sizeof(out)); i++ ) 
			len += channel->codec->encode(&out[len], &sampleData[i * channel->codec->samples_per_frame], channel->encode_state);

		// check for excess audio
		excess = (sampleLength - (i * channel->codec->samples_per_frame));
		if(excess > 0)
		{
			memcpy(channel->excessAudio, &sampleData[i * channel->codec->samples_per_frame], excess * 2);
			channel->excessAudioLen = excess;
		}

		// send out the voice data
		VoicePublish(channel, (char*)out, len);

		// check if we've just stopped talking
		if(channel->justStoppedTalking)
		{
			// clear any excess audio
			channel->excessAudioLen = 0;

			// we've no longer just stopped
			channel->justStoppedTalking = voiceFalse;
		}
	}
}

static void PlaybackCBF(void)
{
	unsigned int min_buffer_size = (unsigned int)(-1);
	unsigned int i, j, k;
	_Channel *c;
	_Member *m;
	static int mixer[MAXMIXERSIZE];
	static short mixed_buffer[MAXMIXERSIZE];
	int mixed_count = 0;
	short *p;

	for(i = 0; i < MAX_CHANNELS; i++)
	{
		if((c = g_Channel[i]) && c->num_members)
		{	
			for(j = 0; j < c->num_members; j++)
			{
				if((m = c->members[j])) 
				{
					if(m->audiodata.processed_length > 0)
					{
						min_buffer_size = min(min(min(min_buffer_size, 
										m->audiodata.processed_length), 
										g_VoicePlayback->max_buffer_length/sizeof(short)),
										MAXMIXERSIZE);
					}
					else if((m != c->local_member) && (m->IsTalking))
					{
						m->IsTalking = voiceFalse;
						if(c->Callbacks.talkingstopped_cbf)
							c->Callbacks.talkingstopped_cbf(c, m->memberid, c->Callbacks.context);
					}
				}
			}
		}
	}

	if(min_buffer_size == (unsigned int)(-1))
		return;

	for(i = 0; i < MAX_CHANNELS; i++)
	{
		if((c = g_Channel[i]))
		{
			for(j = 0; j < c->num_members; j++)
			{
				if((m = c->members[j]) && m->audiodata.processed_length)
				{
					assert(min_buffer_size <= m->audiodata.processed_length);

					for(k = 0; k < min_buffer_size; k++)
					{
						if(mixed_count == 0)
							mixer[k] = (int) m->audiodata.processed_data[k];
						else
							mixer[k] += (int) m->audiodata.processed_data[k];
					}

					assert(m->audiodata.processed_length >= min_buffer_size);
					
					p = NULL;

					if((m->audiodata.processed_length - min_buffer_size) > 0)
						p = (short *)gsimalloc(sizeof(short)*(m->audiodata.processed_length - min_buffer_size));

					if(p)
					{
						memcpy(p, &m->audiodata.processed_data[min_buffer_size], 
							sizeof(short)*(m->audiodata.processed_length - min_buffer_size));

						gsifree(m->audiodata.processed_data);

						m->audiodata.processed_data = p;
						m->audiodata.processed_length = m->audiodata.processed_length - min_buffer_size;
					}
					else
					{
						gsifree(m->audiodata.processed_data);
						m->audiodata.processed_data = NULL;
						m->audiodata.processed_length = 0;
					}
					mixed_count++;
				}
			}
		}
	}
	if(mixed_count)
	{
		memset(mixed_buffer, 0, sizeof(mixed_buffer));
		for(i = 0; i < min_buffer_size; i++)
		{
			mixed_buffer[i] = (short)(mixer[i] / mixed_count);
		}
		if(g_VoicePlayback)
		{
			AudioSample sample;
			sample.data = mixed_buffer;
			sample.length = min_buffer_size * sizeof(short);
			if(g_VoicePlayback)
				g_VoicePlayback->play(&sample);
		}		
	}
}

static voiceBool AudioProcessDecodedData(_Channel * c, _Member * m, short * data, int len)
{
	int expandedLen;
	short *p;
	AudioSample sample;
	float f;
	int i;
	voiceBool starting;
	short * start;

	assert(m);
	if(!m)
		return voiceFalse;

	if(!isValidChannel(c))
	{
		assert(0);
		return voiceFalse;
	}

	if( c->state != STATE_ACTIVE )
		return voiceFalse;
	
	// check if we're just starting to receive audio
	starting = (m->audiodata.processed_length == 0)?voiceTrue:voiceFalse;

	// get the expanded len
	expandedLen = (len + m->audiodata.expand);

	// reallocate the array to hold the new data
	p = (short*)gsirealloc(m->audiodata.processed_data, sizeof(short) * (m->audiodata.processed_length + expandedLen));
	if(!p)
		return voiceFalse;

	// store the new pointer
	m->audiodata.processed_data = p;

	// expand the audio
	start = &m->audiodata.processed_data[m->audiodata.processed_length];
	for(i = 0; i < expandedLen ; i++)
	{
		f = (((float)i / expandedLen) * len);
		start[i] = data[(int)f];
	}

	// populate sample for the callback
	sample.data = (m->audiodata.processed_data + m->audiodata.processed_length);
	sample.length = expandedLen;

	// store the new processed length
	m->audiodata.processed_length += expandedLen;

	// check if we've just started talking
	if(starting && !m->IsTalking)
	{
		m->IsTalking = voiceTrue;
		if(c->Callbacks.talkingstarted_cbf)
			c->Callbacks.talkingstarted_cbf(c, m->memberid, c->Callbacks.context);
	}

	// call the callback with the voice data
	if(c->Callbacks.voicedata_cbf)
		c->Callbacks.voicedata_cbf(c, m->memberid, &sample, c->Callbacks.context);

	return voiceTrue;
}

static void AudioProcess(void)
{
	_Channel *c;
	_Member *m;
	int i, j, k;
	static short out[8000];
	int len;
	int numFrames;

	// loop through the channels
	for(i = 0; i < MAX_CHANNELS; i++)
	{
		// check if the channel is active
		if((c = g_Channel[i]) && c->state == STATE_ACTIVE && c->codec)
		{
			// loop through the members
			for(j = 0; j < (int)c->num_members; j++)
			{
				// check if the member has pending encoded audio
				if((m = c->members[j]) && m->audiodata.encoded_length)
				{
					// clear the decoded len
					len = 0;

					// get the number of frames
					numFrames = (m->audiodata.encoded_length / c->codec->encoded_frame_size);

					// loop through the number of encoded frames
					for(k = 0 ; k < numFrames ; k++)
					{
						// decode one frame
						len += c->codec->decode(&out[len], (unsigned char*)&m->audiodata.encoded_data[k * c->codec->encoded_frame_size], m->decode_state);

						// if we're maxed out then process this and keep going
						if(((len * 2) + (c->codec->samples_per_frame * 2)) > (int)sizeof(out))
						{
							AudioProcessDecodedData(c, m, out, len);
							len = 0;
						}
					}

					// process the remaining decoded data
					if(len)
						AudioProcessDecodedData(c, m, out, len);

					// clear the encoded audio state
					gsifree(m->audiodata.encoded_data);
					m->audiodata.encoded_data = NULL;
					m->audiodata.encoded_length = 0;
					m->audiodata.expand = 0;
				}
			}
		}
	}
}

static _Channel* VoiceChannelByRef(unsigned int channel_ref)
{
	_Channel *c = NULL;
	unsigned int i;

	for(i = 0; i < MAX_CHANNELS; i++)
		if((c = g_Channel[i]) && (c->channel_ref == channel_ref))
			break;
	
	return c;
}

static _Member* VoiceChannelMemberById(_Channel *c, MemberId memberid)
{
	unsigned int i;
	_Member *m = NULL, *m2 = NULL;

	if(!isValidChannel(c))
	{
		assert(0);
		return m;
	}

	for(i = 0; i < c->num_members; i++)
	{
		if((m2 = c->members[i]) && (m2->memberid == memberid))
		{
			m = m2;
			break;
		}
	}

	return m;
}

voiceBool VoiceChannelMemberReachable(Channel channel, MemberId memberid)
{
	_Member *m;

	m = VoiceChannelMemberById((_Channel*)channel, memberid);

	if(m)
		return (voiceBool)!(m->flags & MFLAG_UNREACHABLE);

	return voiceFalse;
}

voiceBool VoiceChannelMemberIsTalking(Channel channel, MemberId memberid)
{
	_Member *m;

	m = VoiceChannelMemberById((_Channel*)channel, memberid);

	assert(m); // bad member id

	if(m)
		return m->IsTalking;

	return voiceFalse;
}

static void AcceptVoiceData(unsigned int channel_ref, MemberId memberid, char *blob, unsigned short blob_len)
{
	_Channel *c = NULL;
	_Member *m = NULL;
	unsigned long time_stamp;
	int expand;
	unsigned int frames;
	unsigned int samples;
	voiceBool newstream = voiceFalse;

	// find which channel this message was meant for
	c = VoiceChannelByRef(channel_ref);
	if(!isValidChannel(c))
	{
		//assert(0);
		return;
	}

	// Check for channel muting
	if(c->mute)
		return;

	// find this member in the channel
	m = VoiceChannelMemberById(c, memberid);
	if(!m)
		return;

	// Check for member muting
	if(m->mute)
		return;

	// limit the amount of buffered incoming data
	if(c->codec)
	{
		// (milliseconds * (bytes/frame) * (samples/millisecond) * (frames/sample)) = bytes
		if(m->audiodata.encoded_length > (MAX_INCOMING_BUFFER_TIME * c->codec->encoded_frame_size * 8 / c->codec->samples_per_frame))
			return;
	}

	// if the member is not talking, start them talking
	if(!m->IsTalking && (m != c->local_member))
	{
		m->IsTalking = voiceTrue;
		if(c->Callbacks.talkingstarted_cbf)
			c->Callbacks.talkingstarted_cbf(c, m->memberid, c->Callbacks.context);
	}

	// check for a timestamp
	if(!m->timestamp)
	{
		time_stamp = m->timestamp = current_time();
		newstream = voiceTrue;
	}
	else
	{
		time_stamp = m->timestamp;
		m->timestamp = current_time();
	}

	// find the number of encoded frames
	frames = (blob_len / c->codec->encoded_frame_size);
	samples = (c->codec->samples_per_frame * frames);
	expand = (((m->timestamp - time_stamp) * 8) - samples);
	if(expand < 0)
		expand = 0;

	// see if this really is the begining of a new stream
	if(!newstream)
	{
		if((unsigned)expand > samples) 
			newstream = voiceTrue;
		else
			newstream = voiceFalse;
	}

	// set expand based upon newstream
	if(newstream)
	{
		expand = m->expand_history = 0;
	}
	else
	{
		if((unsigned)expand > (samples / 8)) 
			expand = (samples / 8);
		m->expand_history = expand = ((m->expand_history * 9) + expand) / 10;
	}

#if 0
	{
		static FILE *f = NULL;
		if(!f)
			f = fopen("debug.txt", "w");
		if(f)
		{
			fprintf(f, "member %d, samples = %d, secs of audio = %.4f, lapsed time = %.4f, expand = %d (%d)\n", 
				m->memberid, samples, (float)samples / 8000, (float)(m->timestamp - time_stamp)/1000, expand, blob_len);
		}
	}
#endif

	if(newstream && JITTER_SIZE)
	{
		newstream = voiceFalse;

		if(!m->audiodata.processed_length)
		{
			m->audiodata.processed_data = (short*)gsimalloc(JITTER_SIZE * sizeof(short));
			memset(m->audiodata.processed_data, 0, JITTER_SIZE * sizeof(short));
			m->audiodata.processed_length = JITTER_SIZE;
		}
		else
		{
			short *p;
			p = (short*)gsimalloc((m->audiodata.processed_length+JITTER_SIZE) * sizeof(short));
			memcpy(p, m->audiodata.processed_data, m->audiodata.processed_length * sizeof(short));
			memset(&p[m->audiodata.processed_length], 0, JITTER_SIZE * sizeof(short));
			gsifree(m->audiodata.processed_data);
			m->audiodata.processed_data = p;
			m->audiodata.processed_length += JITTER_SIZE;
		}
	}

	if(c->codec)
	{
		if(m->audiodata.encoded_length)
		{
			m->audiodata.encoded_data = (char*)gsirealloc(m->audiodata.encoded_data, m->audiodata.encoded_length+blob_len);
			memcpy(&m->audiodata.encoded_data[m->audiodata.encoded_length], blob, blob_len);
			m->audiodata.encoded_length += blob_len;
			m->audiodata.expand += expand;
		}
		else
		{
			m->audiodata.encoded_data = (char *)gsimalloc(blob_len);
			memcpy(m->audiodata.encoded_data, blob, blob_len);
			m->audiodata.encoded_length = blob_len;
			m->audiodata.expand = expand;
		}
	}
}

static void AcceptRelayData(unsigned int channel_ref, MemberId memberid, 
					  char *blob, unsigned short blob_len)
{
	unsigned int i = 0, j = 0;
	_Channel *c = NULL;
	_Member * m = NULL;
	unsigned int   member_index     = 0;
	unsigned int   relay_count      = 0;
	unsigned short originator_relay = 0;
	unsigned int   originator_index = 0;

	c = VoiceChannelByRef(channel_ref);

	if(!isValidChannel(c))
	{
		//assert(0);
		return;
	}

	// relaying voodoo follows
	// basically we...
	//   1. get a NET_RELAY message which has voice data embedded
	//   2. send the voice message back to ourselves as NET_VOICEDATA, decode, and play the audio
	//   3. pass along the NET_RELAY message to as many members below us that we are set to relay to
	//   4. members we relay to will pick up the ball and do what we just did until
	//      there are no more members to send to. tada!


	for(i = 0; (unsigned int)i < c->num_members; i++)
	{
		if((m = c->members[i]) && (m->memberid == memberid))
		{
			originator_relay = m->relay;
			originator_index = i;
			break;
		}
	}

	relay_count = originator_relay;

	for(i = 0; (unsigned int)i < c->num_members; i++)
	{
		if((m = c->members[i]))
		{
			if(c->local_member == m)
			{
				if(originator_index <= relay_count)
					relay_count++;
				// sending this to ourselves so we can decode and play it
				SendVoiceData(m->channel_ref, memberid, m, blob, blob_len, MSG_UNRELIABLE);
				break;
			}
			if(m->memberid != memberid)
				relay_count += m->relay;
		}
	}

	member_index = relay_count;
	for(j = 0; (j < c->local_member->relay) && (member_index < c->num_members); j++, member_index++)
	{
		// don't relay to the originating member or an unreachable member
		while(
				( (m = c->members[member_index]) ) && 
				( (memberid == m->memberid) || (m->flags & MFLAG_UNREACHABLE) ) 
			)
		{
			member_index++;
			if(member_index >= c->num_members)
				break;
		}

		if((member_index < c->num_members) && (m = c->members[member_index]))
		{
			// we should never have this happen, but this will
			// stop us from sending a NET_RELAY message
			// to ourselves which starts a nasty loop
			if(m->memberid == c->local_member->memberid)
				break;

			RelayData(m, memberid, blob, blob_len, MSG_UNRELIABLE);
		}
	}
}

static void AcceptPong(unsigned int channel_ref, MemberId memberid)
{

	_Channel *pc;
	_Member *pm;
	Packet op;

	pc = VoiceChannelByRef(channel_ref);

	if(!pc)
		return;

	if(!pc->IsHost)
		return;


	pm = VoiceChannelMemberById(pc, memberid);

	if(!pm)
		return;

	op = g_Packet;

	PacketClear(op);

	pm->flags &= ~MFLAG_UNREACHABLE;
	PacketAddByte(op, NET_MEMBERFLAG_CHANGED);
	PacketAddShort(op, pm->memberid);
	PacketAddInt(op, pm->flags);

	BroadcastPacket(pc, op);
}

static void AcceptData(void)
{
	SOCKADDR_IN from;
	unsigned int channel_ref; 
	MemberId memberid;
	unsigned short blob_len = 0;
	char *blob;
	Packet p;
	unsigned char command = (unsigned char)(-1);

	while( ( p = ReceivePacketUDP(&from) ) != NULL )
	{
		command = PacketGetByte(p);

		switch(command)
		{
			
		case NET_VOICEDATA:
		{
			channel_ref = PacketGetInt(p);
			memberid = PacketGetShort(p);
			blob = PacketGetBlob(p, &blob_len);
	
			AcceptVoiceData(channel_ref, memberid, blob, blob_len);
		}
		break;

		case NET_VOICEDATA_LIST:
		{
			_Member *m;
			_Channel *c;
			unsigned int i;
			unsigned short num_members;
			MemberId memberlist[MAX_SENDLIST];
			
			// get the channel reference
			channel_ref = PacketGetInt(p);
			
			// get a pointer to the channel
			c = VoiceChannelByRef(channel_ref);
			if(!isValidChannel(c))
			{
				//assert(0);
				break;
			}
			
			// get the sender id and number of members in the list
			memberid = PacketGetShort(p);
			num_members = PacketGetShort(p);

			// check for no members
			if(!num_members)
				break;

			// check if it's above our limit
			if(num_members > MAX_SENDLIST)
				num_members = MAX_SENDLIST;
			
			// get the member id's
			for(i = 0; i < num_members; i++)
				memberlist[i] = PacketGetShort(p);
			
			// get the blob
			blob = PacketGetBlob(p, &blob_len);
			
			// send the data to the members
			for(i = 0; i < num_members; i++)
			{
				m = VoiceChannelMemberById(c, memberlist[i]);
				if(m)
					SendVoiceData(m->channel_ref, memberid, m, blob, blob_len, MSG_RELIABLE);
			}
		}
		break;

		case NET_RELAYDATA:
		{

			channel_ref = PacketGetInt(p);
			memberid = PacketGetShort(p);
			blob = PacketGetBlob(p, &blob_len);

			AcceptRelayData(channel_ref, memberid, blob, blob_len);
		}
		break;

		case NET_PING:
		{
			Packet op = g_Packet;

			PacketClear(op);

			PacketAddByte(op, NET_PONG);
			PacketAddInt(op, PacketGetInt(p));
			PacketAddInt(op, PacketGetInt(p));

			sendto(gUDPSocket, PacketBuffer(op), PacketLength(op), 0, (const struct sockaddr *)&from, sizeof(SOCKADDR_IN));
		}
		break;

		case NET_PONG:
		{
			channel_ref = PacketGetInt(p);
			memberid = PacketGetShort(p);

			AcceptPong(channel_ref, memberid);
		}
		break;

		}
	}
}

void VoiceActivateLevel(unsigned short level)
{
	// if we're setting this to 0, that's equivalent to a stop
	if(gActivateLevel &&  !level)
	{
		VoiceRecordStop(NULL);
		return;
	}

	// store the level
	gActivateLevel = level;
}

unsigned short VoiceGetActivateLevel()
{
	return (unsigned short)gActivateLevel;
}

unsigned short VoiceGetInputLevel()
{
	// if we haven't gotten our first capture return 0
	if(!gGotFirstCapture)
		return 0;

	// return the last recorded input level
	return (unsigned short)gLastInputLevel;
}

VoiceCapture* VoiceSetCaptureInterface(VoiceCapture* voicecapture)
{
	// Backup the old
	VoiceCapture* pOld = g_VoiceCapture;

	// Sanity
	assert(gInitialized);
	if(!gInitialized)
		return NULL;

	// Assign & open the new
	g_VoiceCapture = voicecapture;
	if( g_VoiceCapture )
	{
		g_VoiceCapture->open(CaptureCBF);
	}

	return pOld;
}

VoicePlayback* VoiceSetPlaybackInterface(VoicePlayback* voiceplayback)
{
	// Backup the old
	VoicePlayback* pOld = g_VoicePlayback;

	// Sanity
	assert(gInitialized);
	if(!gInitialized)
		return NULL;

	// Assign & open the new
	g_VoicePlayback = voiceplayback;
	if( g_VoicePlayback )
	{
		g_VoicePlayback->open(PlaybackCBF);
	}

	return pOld;
}

unsigned short VoiceInitialize(VoiceCapture *voicecapture, VoicePlayback *voiceplayback)
{
	unsigned int i;
	unsigned short port;

	assert(!gInitialized);
	if(gInitialized)
		VoiceCleanup();

	SocketStartUp();

	// The capture device must always be opened first. On some
	// devices such as the Logitech headset SDK for the 
	// PS2, there is only one initialization call
	// for both the capture and playback devices, so
	// initilization for such systems will be done in the
	// capture module, then the playback module
	// will start off in a pre-initialized state.
	
	g_VoiceCapture = voicecapture;
	if(g_VoiceCapture)
	{
		if(!g_VoiceCapture->open(CaptureCBF))
			return 0;
	}

	g_VoicePlayback = voiceplayback;
	if(g_VoicePlayback)
	{
		if(!g_VoicePlayback->open(PlaybackCBF))
		{
			if(g_VoiceCapture)
				g_VoiceCapture->close();
			return 0;
		}
	}

	memset(g_Channel, 0, MAX_CHANNELS);
	for(i = 0; i < MAX_INCOMING; i++)
		g_InSockets[i] = INVALID_SOCKET;

	g_Packet = PacketNew(PACKET_DEFAULT);

	port = VoiceNetInit();
	if(!port)
	{
		if(g_VoiceCapture)
			g_VoiceCapture->close();
		if(g_VoicePlayback)
			g_VoicePlayback->close();
		return 0;
	}

	gInitialized = voiceTrue;

	return port;
}

void VoiceCaptureClose()
{
	assert(g_VoiceCapture);

	if( g_VoiceCapture )
		g_VoiceCapture->close();
}

voiceBool VoiceCaptureOpen()
{
	assert(g_VoiceCapture);

	if( g_VoiceCapture )
	{
		return g_VoiceCapture->open(CaptureCBF);
	}
	return voiceFalse;
}

voiceBool VoiceCaptureIsOpen()
{
	assert(g_VoiceCapture);

	if( g_VoiceCapture )
	{
		return (voiceBool)(g_VoicePlayback->get_device() != (unsigned int)-1);
	}

	return voiceFalse;
}

void VoicePlaybackClose()
{
	assert( g_VoicePlayback );

	if( g_VoicePlayback )
		g_VoicePlayback->close();
}

voiceBool VoicePlaybackOpen()
{
	assert( g_VoicePlayback );

	if( g_VoicePlayback )
	{
		return g_VoicePlayback->open(PlaybackCBF);
	}
	return voiceFalse;
}

voiceBool VoicePlaybackIsOpen()
{
	assert( g_VoicePlayback );

	if( g_VoicePlayback )
	{
		return(voiceBool)( g_VoicePlayback->get_device() != (unsigned int)-1 );
	}
	return voiceFalse;
}

void VoiceCleanup(void)
{
	assert(gInitialized);

	if(g_VoiceCapture)
		g_VoiceCapture->close();

	if(g_VoicePlayback)
		g_VoicePlayback->close();

	// make sure any remaining channels are closed
	VoiceChannelDisconnect(NULL);

	VoiceNetClose();

	SocketShutDown();

	if(g_Packet)
		PacketDelete(g_Packet);

	gUDPSocket = INVALID_SOCKET;
	gTCPSocket = INVALID_SOCKET;

	gInitialized = voiceFalse;
}

voiceBool VoiceIsInitialized()
{
	return gInitialized;
}

unsigned short VoiceGetPort()
{
	return gPort;
}

static voiceBool DoJoin(_Channel * channel,
                        const char *channel_address,
                        const char *channel_name,
                        unsigned short port,
                        const char *channel_pass,
                        const char *member_context,
                        VoiceCallbacks *callbacks)
{
	SOCKADDR_IN host;
	SOCKET connectsock;
	struct hostent *hent;
	int rcode;

	assert(callbacks);
	if(!callbacks)
		return voiceFalse;

	// create the socket to connect with
	connectsock = socket(AF_INET, SOCK_STREAM, 0);
	if(connectsock == INVALID_SOCKET)
	{
		if(callbacks->connectfailed_cbf)
			callbacks->connectfailed_cbf(channel, channel_name, connectfailedNetwork, callbacks->context);
		return voiceFalse;
	}

	// setup the host address
	memset(&host, 0, sizeof(host));
	host.sin_family = AF_INET;
	host.sin_addr.s_addr = inet_addr(channel_address);
	host.sin_port = htons(port);
	if(host.sin_addr.s_addr == INADDR_NONE)
	{
		hent = gethostbyname(channel_address);
		if(!hent)
		{
			closesocket(connectsock);
			if(callbacks->connectfailed_cbf)
				callbacks->connectfailed_cbf(channel, channel_name, connectfailedBadAddress, callbacks->context);
			return voiceFalse;
		}
		host.sin_addr.s_addr = *(unsigned long *)hent->h_addr_list[0];
	}

	// set the socket to non-blocking
	if(!SetSockBlocking(connectsock, 0))
	{
		closesocket(connectsock);
		if(callbacks->connectfailed_cbf)
			callbacks->connectfailed_cbf(channel, channel_name, connectfailedNetwork, callbacks->context);
		return voiceFalse;
	}

	// start the connect
	rcode = connect(connectsock, (const struct sockaddr *)&host, sizeof(host));
	if(rcode == SOCKET_ERROR)
	{
		// Note: Some network stacks may return 0, meaning the socket is valid 
		// even though connect return SOCKET_ERROR (PS2)
		int error = GOAGetLastError(connectsock);
		if((error != 0) && (error != WSAEWOULDBLOCK) && (error != WSAEINPROGRESS))
		{
			closesocket(connectsock);
			if(callbacks->connectfailed_cbf)
				callbacks->connectfailed_cbf(channel, channel_name, connectfailedNetwork, callbacks->context);
			return voiceFalse;
		}
	}
 
	// set channel properties
	channel->state = STATE_LOGIN;
	channel->address = goastrdup(inet_ntoa(host.sin_addr));
	channel->port = port;
	memcpy(&channel->Callbacks, callbacks, sizeof(VoiceCallbacks));
	channel->TCPHostSocket = connectsock;
	VoiceSetChannelString(channel, "channelname", channel_name);
	VoiceSetChannelString(channel, "password", channel_pass);
	if(member_context)
		VoiceSetChannelString(channel, "member_context", member_context);

	return voiceTrue;
}

Channel VoiceChannelCreate(const char *channel_name,
                           const char *channel_pass,
                           VoiceCodec *codec,
                           int flags,
                           const char *member_context,
                           VoiceCallbacks *callbacks)
{
	_Channel *c;
	unsigned int i;
	voiceBool success;

	// Sanity checks
	assert(callbacks);
	if(!callbacks)
		return INVALID_CHANNEL;

	assert(codec);
	if(!codec)
		return INVALID_CHANNEL;

	assert(channel_name);
	if(!channel_name)
		return INVALID_CHANNEL;

	// check for a duplicate channel name
	for(i = 0; i < MAX_CHANNELS; i++)
	{
		if((c = g_Channel[i]))
		{
			if(strcasecmp(channel_name, VoiceGetChannelString(c, "channelname", ""))==0)
			{
				if(callbacks && callbacks->connectfailed_cbf)
					callbacks->connectfailed_cbf(NULL, channel_name, connectfailedChannelExists, callbacks->context);
				return INVALID_CHANNEL;
			}
		}
	}

	// allocate a channel
	c = AllocChannel();
	if(!c)
	{
		if(callbacks && callbacks->connectfailed_cbf)
			callbacks->connectfailed_cbf(NULL, channel_name, connectfailedChannelMaxReached, callbacks->context);
		return INVALID_CHANNEL;
	}

	// init the channel's states
	VoiceSetChannelInt(c, "port", gPort);
	VoiceSetChannelInt(c, "flags", flags);
	VoiceSetChannelString(c, "codec", codec->id);
	c->flags = flags;
	c->codec = codec;
	c->IsHost = voiceTrue;
	c->TCPHostSocket = INVALID_SOCKET;
	c->encode_state = codec->create_state(stateEncode);

	// do the join
	success = DoJoin(c, "127.0.0.1", channel_name, gPort, channel_pass, member_context, callbacks);
	if(!success)
	{
		RemoveChannelProcess(c);
		c = INVALID_CHANNEL;
	}
	return c;
}

Channel VoiceChannelJoin(const char *channel_address,
                         const char *channel_name,
                         unsigned short port,
                         const char *channel_pass,
                         const char *member_context,
                         VoiceCallbacks *callbacks)
{
	unsigned int i;
	_Channel *c;
	voiceBool success;

	assert(callbacks);
	if(!callbacks)
		return INVALID_CHANNEL;

	// check for a duplicate channel name
	for(i = 0; i < MAX_CHANNELS; i++)
	{
		if((c = g_Channel[i]))
		{
			if(strcasecmp(channel_name, VoiceGetChannelString(c, "channelname", ""))==0)
			{
				if(callbacks && callbacks->connectfailed_cbf)
					callbacks->connectfailed_cbf(c, channel_name, connectfailedChannelExists, callbacks->context);
				return INVALID_CHANNEL;
			}
		}
	}

	// alloocate a channel to join with
	c = AllocChannel();
	if(!c)
	{
		if(callbacks->connectfailed_cbf)
			callbacks->connectfailed_cbf(c, channel_name, connectfailedChannelMaxReached, callbacks->context);
		return INVALID_CHANNEL;
	}

	// do the join
	success = DoJoin(c, channel_address, channel_name, port, channel_pass, member_context, callbacks);
	if(!success)
	{
		RemoveChannelProcess(c);
		c = INVALID_CHANNEL;
	}
	return c;
}

void VoiceChannelDisconnect(Channel channel)
{
	_Channel *_channel = (_Channel *)channel;
	unsigned int i;

	if(channel)
	{
		RemoveChannelProcess(_channel);
	}
	else // if the channel parameter is NULL then we close all channels
	{
		for(i = 0; i < MAX_CHANNELS; i++)
		{
			if(g_Channel[i])
			{
				VoiceChannelDisconnect(g_Channel[i]);
				g_Channel[i] = NULL;
			}
		}
	}
}

unsigned int VoiceGetChannelCount()
{
	int i;
	unsigned int count = 0;

	if(!gInitialized)
		return count;

	for( i = 0; i < MAX_CHANNELS; i++ )
	{
		if( INVALID_CHANNEL != g_Channel[i] )
		{
			count++;
		}
	}

	return count;
}

Channel VoiceGetChannelByIndex( unsigned int index )
{
	unsigned int i;
	unsigned int nPlace;

	assert(gInitialized);
	if(!gInitialized)
		return INVALID_CHANNEL;

	assert( index >= 0 );
	if( index < 0 )
		return INVALID_CHANNEL;

	assert( index < MAX_CHANNELS );
	if( index >= MAX_CHANNELS )
		return INVALID_CHANNEL;

	for( i = 0, nPlace = 0; i < MAX_CHANNELS; i++ )
	{
		if( INVALID_CHANNEL != g_Channel[i] )
		{
			if( index == nPlace )
				return g_Channel[i];

			nPlace++;
		}
	}
	
	assert(0); // Should be unreachable - right?
	return INVALID_CHANNEL;
}

MemberId VoiceChannelLocalMember(Channel channel)
{
	_Channel *_channel = (_Channel *)channel;

	if(!channel || !_channel->local_member)
		return INVALID_MEMBERID;

	return _channel->local_member->memberid;
}

MemberId VoiceChannelHostMember(Channel channel)
{
	_Channel *_channel = (_Channel *)channel;

	if(!channel || !_channel->host_member)
		return INVALID_MEMBERID;

	return _channel->host_member->memberid;
}

MemberId VoiceChannelMemberByIndex(Channel channel, unsigned int index)
{
	_Channel *_channel = (_Channel *)channel;

	if(!channel || _channel->num_members < index)
		return INVALID_MEMBERID;

	return _channel->members[index]->memberid;
}

voiceBool VoiceChannelSendListAdd(Channel channel, MemberId memberid)
{
	_Channel *_channel = (_Channel *)channel;
	unsigned int i;

	// Parameter checks
	if(!_channel)
	{
		assert(0);
		return voiceFalse;
	}

	// Check if the member is already in the list
	for(i = 0; i < _channel->sendlist_len; i++)
	{
		if(_channel->sendlist[i] == memberid)
			return voiceTrue;
	}

	// Make sure the buffer is not full
	if(_channel->sendlist_len >= MAX_SENDLIST)
		return voiceFalse;

	// Add the member
	_channel->sendlist[_channel->sendlist_len] = memberid;
	_channel->sendlist_len++;

	return voiceTrue;
}

void VoiceChannelSendListReset(Channel channel)
{
	_Channel *_channel = (_Channel *)channel;
	
	assert(channel);
	if(!channel)
		return;

	// reset the count
	_channel->sendlist_len = 0;
}

unsigned short VoiceChannelSendListCount(Channel channel)
{
	_Channel *_channel = (_Channel *)channel;
	assert(_channel);
	if(!_channel)
		return 0;

	return _channel->sendlist_len;
}

voiceBool VoiceChannelIsMemberOnSendList(Channel channel, MemberId memberid)
{
	_Channel *_channel = (_Channel *)channel;
	unsigned int i;

	// Parameter checks
	if(!_channel)
	{
		assert(0);
		return voiceFalse;
	}

	// check for no list
	if(!_channel->sendlist_len)
		return voiceFalse;

	// Check if the member is already in the list
	for(i = 0; i < _channel->sendlist_len; i++)
	{
		if(_channel->sendlist[i] == memberid)
			return voiceTrue;
	}

	// not on the send list
	return voiceFalse;
}

MemberId VoiceChannelSendListGetMember( Channel channel, unsigned short index )
{
	_Channel *c = channel;
	
	// Channel Check
	if(!isValidChannel(c))
	{
		assert(0);
		return INVALID_MEMBERID;
	}

	// Range Check
	assert(index < c->sendlist_len );
	if(index >= c->sendlist_len )
		return INVALID_MEMBERID;

	// Get the MemberId
	return c->sendlist[index];
}

void VoiceChannelKickMember(Channel channel, MemberId memberid)
{
	_Channel *c = channel;
	_Member *m = MemberById(c, memberid);

	if(!isValidChannel(c))
	{
		assert(0);
		return;
	}

	// we can only do this if we're the host
	if(!c->IsHost)
		return;

	// we can't kick ourselves
	if(c->local_member == m)
		return;

	// let everyone know he was kicked
	BroadcastRemoveMember(c, m, NET_RM_KICKED);

	// remove the member
	RemoveMember(c,m);
}

void VoiceChannelBanMember(Channel channel, MemberId memberid)
{
	_Channel *c = channel;
	_Member *m = MemberById(c, memberid);

	if(!isValidChannel(c))
	{
		assert(0);
		return;
	}

	// we can only do this if we're the host
	if(!c->IsHost)
		return;

	// we can't ban ourselves
	if(c->local_member == m)
		return;

	// check for too many bans
	if(c->banlist_len == MAX_BANLIST)
		return;

	// add the address to the ban list
	c->banlist[c->banlist_len++] = goastrdup(m->address);

	// let everyone know he was banned
	BroadcastRemoveMember(c, m, NET_RM_BANNED);

	// remove the member
	RemoveMember(c,m);
}

void VoiceChannelClearBans(Channel channel)
{
	_Channel *c = channel;
	int i;

	if(!isValidChannel(c))
	{
		assert(0);
		return;
	}

	// we can only do this if we're the host
	if(!c->IsHost)
		return;

	// free all existing bans
	for(i = 0 ; i < c->banlist_len ; i++)
		gsifree(c->banlist[i]);

	// zero the list len
	c->banlist_len = 0;
}

void VoiceSendMiscData(Channel channel,
                       void *data,
                       unsigned short datalength)
{
	_Channel *c = channel;
	Packet p = g_Packet;
	
	if(!isValidChannel(c))
	{
		assert(0);
		return;
	}

	// We shouldn't send user data when we're not connected
	assert(c->state == STATE_ACTIVE);
	if( c->state != STATE_ACTIVE )
		return;

	PacketClear(p);
	PacketAddByte(p, NET_MISCDATA);
	PacketAddShort(p, c->local_member->memberid);
	PacketAddBlob(p, (char*)data, datalength);

	SendPacketTCP(c->TCPHostSocket, p);
}

static void AcceptLogin(char *name, char *pass, unsigned short port, 
				 unsigned int cref, char *member_context,
				 SOCKADDR_IN inaddr, SOCKET insocket)
{
	_Channel *c = NULL;
	unsigned short relay = 2;
	unsigned int i, j;
	voiceBool match = voiceFalse;
	voiceBool namematch = voiceFalse;
	voiceBool passmatch = voiceFalse;
	Packet op = g_Packet;

	PacketClear(op);

	// search thru all active channels we are the host of for a match
	for(i = 0; i < MAX_CHANNELS; i++)
	{
		c = g_Channel[i];

		if(!c)
			continue;

		namematch = (voiceBool)(strcasecmp(VoiceGetChannelString(c, "channelname",""), name) == 0);
		passmatch = (voiceBool)(strcmp(VoiceGetChannelString(c, "password",""), pass) == 0);

		for(j = 0; j < c->banlist_len; j++)
		{
			if(strcmp(c->banlist[j], inet_ntoa(inaddr.sin_addr))==0)
			{
				// banned
				PacketAddByte(op, NET_BANNED);
				SendPacketTCP(insocket, op);
				return;
			}
		}

		// check if the channel exists but the password doesn't match
		if(namematch && !passmatch)
			break;

		if(namematch && passmatch)
		{
			// the password matches

			match = voiceTrue;
			if(c->IsHost)
			{
				Packet p2 = NULL;
				_Member *m = NULL, *newm = NULL;
				KeyVal kv = VoiceInitKeyVal(NULL, 0);

				VoiceSetString(kv, "address", inet_ntoa(inaddr.sin_addr));
				VoiceSetInt(kv, "port", port);
				VoiceSetInt(kv, "id", c->memberid+1);
				VoiceSetString(kv, "member_context", member_context);

				// here we notify all the channel members of this new member
				p2 = PacketNew(c->num_members * (sizeof(_Member) + MAXPACKETSIZE));
				if( p2 )
				{
					PacketAddByte(p2, NET_ADDMEMBER);
					PacketAddString(p2, inet_ntoa(inaddr.sin_addr));
					PacketAddInt(p2, port);
					PacketAddInt(p2, cref);
					PacketAddShort(p2, ++c->memberid);
					PacketAddShort(p2, relay);
					PacketAddInt(p2, MFLAG_UNREACHABLE);
					PacketAddBlob(p2, VoiceGetKeyValBuffer(kv), (unsigned short)VoiceGetKeyValLength(kv));

					BroadcastPacket(c, p2);
					
					PacketDelete(p2);
				}
				newm = AllocMember();

				newm->kv = kv;
				newm->TCPSocket = insocket;
				newm->address = goastrdup(inet_ntoa(inaddr.sin_addr));
				newm->port = port;
				newm->channel_ref = cref;
				newm->memberid = c->memberid;
				newm->relay = relay;
				newm->flags = MFLAG_UNREACHABLE;

				AddMember(c, newm);

				if(c->num_members == 1)
					c->local_member = c->host_member = newm;
				
				if(c->Callbacks.memberjoined_cbf)
					c->Callbacks.memberjoined_cbf(c, newm->memberid, c->Callbacks.context);

				VoiceSetChannelInt(c, "flags", c->flags);

				PacketAddByte(op, NET_LOGIN);
				PacketAddBlob(op, VoiceGetKeyValBuffer(c->kv), (unsigned short)VoiceGetKeyValLength(c->kv));
				PacketAddShort(op, c->host_member->memberid);
				PacketAddShort(op, newm->memberid);
				PacketAddInt(op, c->num_members);
				for( j = 0; j < c->num_members; j++ )
				{
					m = c->members[j]; 
					if(m)
					{
						PacketAddString(op, m->address);
						PacketAddInt(op, m->port);
						PacketAddBlob(op, VoiceGetKeyValBuffer(m->kv), (unsigned short)VoiceGetKeyValLength(m->kv));
						PacketAddInt(op, m->channel_ref);
						PacketAddShort(op, m->memberid);
						PacketAddShort(op, m->relay);
						PacketAddInt(op, m->flags);
					}
				}
				SendPacketTCP(insocket, op);

				// send a ping off to the new member to check if he is reachable
				// via UDP
				{
					Packet ping = PacketNew(PACKET_DEFAULT);
					PacketAddByte(ping, NET_PING);
					PacketAddInt(ping, c->channel_ref);
					PacketAddShort(ping, newm->memberid);
					SendPacketUDP(newm->address, newm->port, ping);
					PacketDelete(ping);
				}

			}
			else // we aren't the host, but we can tell them where the real host is
			{
				PacketAddByte(op, NET_REDIRECT);
				PacketAddString(op, c->address);
				PacketAddInt(op, c->port);
				SendPacketTCP(insocket, op);
			}

			return;
		}
	}
	if(!match)
	{
		if(namematch)
			PacketAddByte(op, NET_BADPASSWORD);
		else
			PacketAddByte(op, NET_NOSUCHCHANNEL);
		SendPacketTCP(insocket, op);
	}
}

static void AcceptRehost(char *name, char *pass, MemberId memberid, SOCKET insocket)
{
	_Channel *c = NULL;
	unsigned int i;
	_Member *m = NULL;
	Packet p = g_Packet;

	PacketClear(p);

	for( i = 0; i < MAX_CHANNELS; i++ )
	{
		c = g_Channel[i];
		
		if(c && 
			(strcasecmp(VoiceGetChannelString(c, "channelname",""), name) == 0) &&
			(strcmp(VoiceGetChannelString(c, "password",""), pass) == 0) )
		{
			if( (m = VoiceChannelMemberById(c, memberid) ) )
			{
				m->TCPSocket = insocket;
			}
			break;
		}
	}


	if(m)
	{
		PacketAddByte(p, NET_REHOST);
		SendPacketTCP(insocket, p);
	}
	
	if(!m)
	{
		PacketAddByte(p, NET_REHOSTFAILED);
		SendPacketTCP(insocket, p);
	}
	else
	{
		// send a ping off to the rehosting member to check if he is reachable
		// via UDP
		Packet ping = PacketNew(PACKET_DEFAULT);
		PacketAddByte(ping, NET_PING);
		PacketAddInt(ping, c->channel_ref);
		PacketAddShort(ping, m->memberid);
		SendPacketUDP(m->address, m->port, ping);
		PacketDelete(ping);
	}

}

static void AcceptConnections(void)
{
	SOCKADDR_IN inaddr;
	int inaddrlen;
	SOCKET insocket;
	int err;
	int namelen;
	SOCKADDR_IN address;
	int k;
	voiceBool gotSocket = voiceFalse;
	unsigned char command = (unsigned char)(-1);
	unsigned char networkversion = (unsigned char)(-1);

	// check for new connections
	inaddrlen = sizeof(SOCKADDR_IN);
	insocket = accept(gTCPSocket, (struct sockaddr *)&inaddr, (unsigned int*)&inaddrlen);

	if(insocket != INVALID_SOCKET)
	{
		// get the address
		namelen = sizeof(SOCKADDR_IN);
		getsockname(insocket, (struct sockaddr *)&address, (unsigned int*)&namelen);

		// add the new incoming socket to our list if there is one
		for( k = 0; k < MAX_INCOMING; k++ )
		{
			if(g_InSockets[k] == INVALID_SOCKET)
			{
				g_InSockets[k] = insocket;
				gotSocket = voiceTrue;
				break;
			}
		}

		// make sure we got a socket
		if(!gotSocket)
		{
			closesocket(insocket);
			return;
		}
	}

	// check the active incoming sockets for connection packets 
	for( k = 0; k < MAX_INCOMING; k++ )
	{
		Packet p = NULL;
		insocket = g_InSockets[k]; 

		if(insocket == INVALID_SOCKET)
			continue;


		// we have an incoming connection we need to handle
		g_InSockets[k] = INVALID_SOCKET;
		inaddrlen = sizeof(SOCKADDR_IN);
		getpeername(insocket, (struct sockaddr *)&inaddr, (unsigned int*)&inaddrlen);

		// check for a connection message

		if( ( p = ReceivePacketTCP(insocket, &err) ) != NULL )
		{
			networkversion = PacketGetByte(p);
			command = PacketGetByte(p);

			if(networkversion != NETWORKVERSION)
			{
				Packet op = g_Packet;

				PacketClear(op);
				PacketAddByte(op, NET_BADVERSION);
				SendPacketTCP(insocket, op);
				continue;
			}

			switch(command)
			{
			case NET_LOGIN:
			{
				char *name = PacketGetString(p);
				char *pass = PacketGetString(p);
				unsigned short port = PacketGetShort(p);
				unsigned int cref = PacketGetInt(p);
				char *member_context = PacketGetString(p);

				AcceptLogin(name, pass, port, cref, member_context, inaddr, insocket);

			}
			break;

 			case NET_REHOST:
			{
				char *name = PacketGetString(p);
				char *pass = PacketGetString(p);
				MemberId memberid = PacketGetShort(p);

				AcceptRehost(name, pass, memberid, insocket);
			}
			break;

			default:
				VoiceNetCloseSocket(insocket);
				break;
			}
		}
		else
		{
			if(err == WSAEWOULDBLOCK)
			{
				// keep this socket in the list and check it next time
				g_InSockets[k] = insocket;
			}
			else
			{
				VoiceNetCloseSocket(insocket);
			}
		}
	}
}

static void LoginChannelProcess(_Channel *c)
{
	Packet packet;
	struct timeval timeout = {0,0};
	fd_set w_set, e_set;
	int res;

	FD_ZERO(&e_set);
	FD_ZERO(&w_set);
	FD_SET( c->TCPHostSocket, &w_set);
	FD_SET( c->TCPHostSocket, &e_set);

	if(!isValidChannel(c))
	{
		assert(0);
		return;
	}

	res = select(FD_SETSIZE, NULL, &w_set, &e_set, &timeout);

	if(res == SOCKET_ERROR || FD_ISSET(c->TCPHostSocket, &e_set))
	{
		VoiceNetCloseSocket(c->TCPHostSocket);
		if(c->Callbacks.connectfailed_cbf)
			c->Callbacks.connectfailed_cbf(c, VoiceGetChannelString(c, "channelname",""), connectfailedNetwork, c->Callbacks.context);			
		c->state = STATE_REMOVE;
		return;
	}

	if(!FD_ISSET(c->TCPHostSocket, &w_set))
		return;

	packet = g_Packet;

	PacketClear(packet);

	PacketAddByte(packet, NETWORKVERSION);
	PacketAddByte(packet, NET_LOGIN);
	PacketAddString(packet, VoiceGetChannelString(c, "channelname", ""));
	PacketAddString(packet, VoiceGetChannelString(c, "password", ""));
	PacketAddShort(packet, gPort);
	PacketAddInt(packet, c->channel_ref);
	PacketAddString(packet, VoiceGetChannelString(c, "member_context", ""));

	SendPacketTCP(c->TCPHostSocket, packet);
	c->state = STATE_CONNECTING;
}


static voiceBool ConnectingLogin(_Channel *c, Packet p)
{
	int namelen;
	SOCKADDR_IN address;
	MemberId memberid, hostid;
	int i;
	KeyVal kv;
	unsigned short blobLen;
	const char * blob;
	_Member *m;
	int num_members;

	if(!isValidChannel(c))
	{
		assert(0);
		return voiceFalse;
	}

	// get info on the socket
	namelen = sizeof(SOCKADDR_IN);
	getsockname(c->TCPHostSocket, (struct sockaddr *)&address, (unsigned int*)&namelen);

	// get info from the packet
	blob = PacketGetBlob(p, &blobLen);
	kv = VoiceInitKeyVal(blob, blobLen);
	hostid = PacketGetShort(p);
	memberid = PacketGetShort(p);

	// the channel is active
	c->state = STATE_ACTIVE;

	// use the keys we got in the packet
	if(c->kv)
		VoiceDeleteKeyVal(c->kv);
	c->kv = kv;

	// we don't need to do this if we're the host
	if(!c->IsHost)
	{
		// set info from the key-values
		c->codec = VoiceCodecLookup(VoiceGetChannelString(c, "codec", ""));
		c->flags = VoiceGetChannelInt(c, "flags", 0);

		// we need to keep the latest memberid so we can start
		// asigning member ids in case we ever become host
		c->memberid = memberid;

		// codec setup
		if(!c->codec)
		{
			if(c->Callbacks.connectfailed_cbf)
				c->Callbacks.connectfailed_cbf(c, VoiceGetChannelString(c, "channelname", ""), connectfailedMissingCodec, c->Callbacks.context);
			c->state = STATE_REMOVE;
			return voiceFalse;
		}
		c->encode_state = c->codec->create_state(stateEncode);

		// get the number of members
		num_members = PacketGetInt(p);

		// loop through the members
		for(i = 0; i < num_members; i++)
		{
			// allocate a new member
			m = AllocMember();
			assert(m);

			// setup the member
			m->TCPSocket = INVALID_SOCKET;
			m->address = goastrdup( PacketGetString( p ) );
			m->port = (unsigned short)PacketGetInt( p );
			blob = PacketGetBlob(p, &blobLen);
			m->kv = VoiceInitKeyVal(blob, blobLen);
			m->channel_ref = PacketGetInt(p);
			m->memberid = PacketGetShort(p);
			m->relay = PacketGetShort(p);
			m->flags = PacketGetInt(p);

			// check if this is us
			if(m->memberid == memberid)
				c->local_member = m;

			// check if this is the host
			if(m->memberid == hostid)
				c->host_member = m;
			
			// if the host sends us a member with 127.0.0.1 as the address
			// then we know it should be the our known address for the host
			if(strcmp( m->address, "127.0.0.1" ) == 0)
			{
				gsifree( m->address );
				m->address = goastrdup( c->address );
				VoiceSetString(m->kv, "address", m->address);
			}

			// add the member
			AddMember(c, m);
		}
	}

	// allocate the space for excess audio
	c->excessAudio = (short *)gsimalloc(c->codec->samples_per_frame * 2);

	// call the callback
	if(c->Callbacks.connectsuccess_cbf)
		c->Callbacks.connectsuccess_cbf(c, c->Callbacks.context);

	return voiceTrue;
}

static void ConnectingChannelProcess(_Channel *c)
{
	//int fromlen = sizeof(SOCKADDR_IN);
	int err;
	Packet p;
	unsigned char command = (unsigned char)(-1);

	if(!isValidChannel(c))
	{
		assert(0);
		return;
	}

	// check for data coming from the host
	while((p = ReceivePacketTCP(c->TCPHostSocket, &err)))
	{
		command = PacketGetByte( p );

		switch(command)
		{
		case NET_LOGIN:
			ConnectingLogin(c, p);
			break;

		case NET_BADVERSION:
			c->state = STATE_REMOVE;
			if(c->Callbacks.connectfailed_cbf)
				c->Callbacks.connectfailed_cbf(c, VoiceGetChannelString(c, "channelname", ""), connectfailedBadVersion, c->Callbacks.context);
			break;
		
		case NET_ADDMEMBER:
			// added because the host sends this to himself so we need to trap it for now
			break;

		case NET_BADPASSWORD:
			c->state = STATE_REMOVE;
			if(c->Callbacks.connectfailed_cbf)
				c->Callbacks.connectfailed_cbf(c, VoiceGetChannelString(c, "channelname", ""), connectfailedInvalidPassword, c->Callbacks.context);
			break;

		case NET_NOSUCHCHANNEL:
			c->state = STATE_REMOVE;
			if(c->Callbacks.connectfailed_cbf)
				c->Callbacks.connectfailed_cbf(c, VoiceGetChannelString(c, "channelname", ""), connectfailedNoSuchChannel, c->Callbacks.context);
			break;

		case NET_BANNED:
			c->state = STATE_REMOVE;
			if(c->Callbacks.connectfailed_cbf)
				c->Callbacks.connectfailed_cbf(c, VoiceGetChannelString(c, "channelname", ""), connectfailedBanned, c->Callbacks.context);
			break;

		case NET_REDIRECT:
			// need to change this to STATE_LOGIN and reset everything to point to the real host
			c->state = STATE_REMOVE;
			if(c->Callbacks.connectfailed_cbf)
				c->Callbacks.connectfailed_cbf(c, VoiceGetChannelString(c, "channelname", ""), connectfailedRedirect, c->Callbacks.context);
			break;
		}
		assert(isValidChannel(c));
	}
	if((c->TCPHostSocket != INVALID_SOCKET) && (err != 0) && (err != WSAEWOULDBLOCK))
	{
		if(c->Callbacks.connectfailed_cbf)
			c->Callbacks.connectfailed_cbf(c, VoiceGetChannelString(c, "channelname", ""), connectfailedNetwork, c->Callbacks.context);
		c->state = STATE_REMOVE;
	}
}

static void RehostingChannelProcess(_Channel *c)
{
	unsigned int i;
	_Member *newhost = NULL;

	if(!isValidChannel(c))
	{
		assert(0);
		return;
	}

	for( i = 0; i < c->num_members; i++ )
	{
		if(c->members[i] && !(c->members[i]->flags & MFLAG_UNREACHABLE))
		{
			newhost = c->members[i];
			break;
		}
	}

	if(newhost)
	{
		SOCKADDR_IN host;
		int res;
		Packet p = NULL;

		// we now need to set all the members(except the new host) of the channel to unreachable until
		// the new host determines their connection type
		for(i = 0; i < c->num_members; i++)
			if(!(c->members[i]->flags & MFLAG_UNREACHABLE) && (c->members[i] != c->host_member))
				c->members[i]->flags |= MFLAG_UNREACHABLE;

		VoiceNetCloseSocket(c->TCPHostSocket);
		gsifree(c->address);
		c->address = goastrdup(newhost->address);
		c->port = newhost->port;
		c->host_member = newhost;
		VoiceSetChannelInt(c, "port", c->port);
		
		if(c->local_member == newhost)
		{
			c->IsHost = voiceTrue;
		}

		c->TCPHostSocket = socket(AF_INET, SOCK_STREAM, 0);
		if(c->TCPHostSocket == INVALID_SOCKET)
		{
			c->state = STATE_REMOVE;
			if(c->Callbacks.connectionlost_cbf)
				c->Callbacks.connectionlost_cbf(c, connectionlostNetwork, c->Callbacks.context);
			return;
		}

		memset(&host, 0, sizeof(host));
		host.sin_family = AF_INET;
		host.sin_addr.s_addr = inet_addr(c->address);
		host.sin_port = htons(c->port);

		if( connect(c->TCPHostSocket, (const struct sockaddr *)&host, sizeof(host)) == SOCKET_ERROR )
		{
			VoiceNetCloseSocket(c->TCPHostSocket);
			c->state = STATE_REMOVE;
			if(c->Callbacks.connectionlost_cbf)
				c->Callbacks.connectionlost_cbf(c, connectionlostNetwork, c->Callbacks.context);
			return; // need to return error code here
		}

		p = g_Packet;

		PacketClear(p);

		PacketAddByte(p, NETWORKVERSION);
		PacketAddByte(p, NET_REHOST);
		PacketAddString(p, VoiceGetChannelString(c, "channelname", ""));
		PacketAddString(p, VoiceGetChannelString(c, "password", ""));
		PacketAddShort(p, c->local_member->memberid);

		res = SendPacketTCP(c->TCPHostSocket, p);
		if(res == PacketLength(p))
		{
			if(!SetSockBlocking(c->TCPHostSocket, 0))
			{
				VoiceNetCloseSocket(c->TCPHostSocket);
				c->state = STATE_REMOVE;
				if(c->Callbacks.connectionlost_cbf)
					c->Callbacks.connectionlost_cbf(c, connectionlostNetwork, c->Callbacks.context);
				return;
			}
			c->state = STATE_ACTIVE;

			if(c->Callbacks.rehost_cbf)
				c->Callbacks.rehost_cbf(c, c->host_member->memberid, c->Callbacks.context);
			
		}
		else
			c->state = STATE_REMOVE;
	}
	else // !newhost
	{
		c->state = STATE_REMOVE;
		if(c->Callbacks.connectionlost_cbf)
			c->Callbacks.connectionlost_cbf(c, connectionlostNetwork, c->Callbacks.context);		
	}
}


static void ActiveChannelProcessHost(_Channel *c)
{
	unsigned int i;
	_Member *m;
	Packet p;
	int err;
	const char * blob;
	unsigned short blobLen;
	unsigned char command = (unsigned char)(-1);

	if(!isValidChannel(c))
	{
		assert(0);
		return;
	}

	for( i = 0; i < c->num_members; i++ )
	{
		m = c->members[i];
 		
		if(m && m->TCPSocket != INVALID_SOCKET)
		{
			while((p = ReceivePacketTCP(m->TCPSocket, &err)))
			{
				command = PacketGetByte( p );

				assert(isValidChannel(c));
				
				switch(command)
				{
				case NET_MISCDATA:
				{
					char *data;
					unsigned short datalength;
					MemberId mid;
					
					BroadcastPacket(c, p);

					mid = PacketGetShort(p);
					data = PacketGetBlob(p, &datalength);
					if(c->Callbacks.miscdata_cbf)
						c->Callbacks.miscdata_cbf(c, mid, data, datalength, c->Callbacks.context);
				}
				break;

				case NET_MEMBERDESC_CHANGED:
				{
					MemberId id;
					char *kvstr;
					_Member *tm;

					id = PacketGetShort(p);
					blob = PacketGetBlob(p, &blobLen);
					kvstr = PacketGetString(p);
					tm = MemberById(c, id);

					if(tm)
					{
						VoiceDeleteKeyVal(tm->kv);
						tm->kv = VoiceInitKeyVal(blob, blobLen);
						if(c->Callbacks.memberdescchanged_cbf)
							c->Callbacks.memberdescchanged_cbf(c, id, c->Callbacks.context);
					}

					BroadcastPacket(c, p);
				}
				break;
				
				case NET_VOICEBROADCAST:
				{
					unsigned int j;
					unsigned int channel_ref; 
					MemberId memberid;
					unsigned short len = 0;
					char *blob;
					_Member *bm;

					channel_ref = PacketGetInt(p);
					memberid = PacketGetShort(p);
					blob = PacketGetBlob(p, &len);

					for(j = 0; j < c->num_members; j++)
					{
						if((bm = c->members[j]) && (bm->memberid != memberid))
						{
							Packet op = g_Packet;

							PacketClear(op);
							PacketAddByte(op, NET_VOICEDATA);
							PacketAddInt(op, bm->channel_ref);
							PacketAddShort(op, memberid);
							PacketAddBlob(op, blob, len);

							if(bm->flags & MFLAG_UNREACHABLE)
								SendPacketTCP(bm->TCPSocket, op);
							else
								SendPacketUDP(bm->address, bm->port, op);
						}
					}
				}
				break;

				case NET_VOICEDATA:
				{
					unsigned int channel_ref; 
					MemberId memberid;

					assert(isValidChannel(c));

					channel_ref = PacketGetInt(p);
					memberid = PacketGetShort(p);
				}
				break;
				}

				assert(isValidChannel(c));
			}

			if((err != 0) && (err != WSAEWOULDBLOCK))
			{
				MemberId mID;

				if(!isValidChannel(c))
				{
					assert(0);
					break;
				}

				// now we need to notify all other members of the removal
				BroadcastRemoveMember(c, m, NET_RM_DISCONNECTED);

				assert(isValidChannel(c));

				// remove the member
				mID = m->memberid;
				RemoveMember(c, m);

				assert(isValidChannel(c));

				// call the disconnected callback
				if(c->Callbacks.memberdisconnect_cbf)
					c->Callbacks.memberdisconnect_cbf(c, mID, memberdisconnectLeft, c->Callbacks.context);

				assert(isValidChannel(c));

				// we must break here or bad things will happen with the the 'for' loop  -rich
				break;
			}
		}
	}

	if(!isValidChannel(c))
	{
		assert(0);
	}
}

static void ActiveChannelProcessMember(_Channel *c)
{
	unsigned int i;
	_Member *m;
	Packet p;
	int err;
	const char * blob;
	unsigned short blobLen;
	unsigned char command = (unsigned char)(-1);

	if(!isValidChannel(c))
	{
		assert(0);
		return;
	}

	while((p = ReceivePacketTCP(c->TCPHostSocket, &err)))
	{
		command = PacketGetByte( p );

		switch(command)
		{
		case NET_MISCDATA:
		{
			char *data;
			unsigned short datalength;
			MemberId mid;
			
			mid = PacketGetShort(p);
			data = PacketGetBlob(p, &datalength);
			
			if(c->Callbacks.miscdata_cbf)
				c->Callbacks.miscdata_cbf(c, mid, data, datalength, c->Callbacks.context);
		}
		break;

		case NET_REMOVEMEMBER:
		{
			int reason = PacketGetByte(p);
			char *address = PacketGetString(p);
			int port = PacketGetInt(p);
			unsigned int member_index;
			MemberId mID;

			if(!c->IsHost)
			{
				for(member_index = 0; member_index < c->num_members; member_index++)
				{
					m = c->members[member_index];
					if(m && strcmp(m->address, address) == 0 && port == m->port)
					{
						// is this us?
						if(m == c->local_member)
						{
							c->state = STATE_REMOVE;
							if(c->Callbacks.connectionlost_cbf)
							{
								connectionlostReason clReason;
								if(reason == NET_RM_KICKED)
									clReason = connectionlostKicked;
								else if(reason == NET_RM_BANNED)
									clReason = connectionlostBanned;
								else
									clReason = connectionlostDisconnected;
								c->Callbacks.connectionlost_cbf(c, clReason, c->Callbacks.context);
							}

							return;
						}

						mID = m->memberid;
						RemoveMember(c, m);

						if(c->Callbacks.memberdisconnect_cbf)
						{
							memberdisconnectReason mdReason;
							if(reason == NET_RM_KICKED)
								mdReason = memberdisconnectKicked;
							else if(reason == NET_RM_BANNED)
								mdReason = memberdisconnectBanned;
							else
								mdReason = memberdisconnectLeft;
							c->Callbacks.memberdisconnect_cbf(c, mID, mdReason, c->Callbacks.context);
						}

						break;
					}
				}
			}
		}
		break;

		case NET_ADDMEMBER:
		{
			if(!c->IsHost)
			{
				m = AllocMember();
				m->TCPSocket = INVALID_SOCKET;
				m->address = goastrdup(PacketGetString(p));
				m->port = (unsigned short)PacketGetInt(p);
				m->channel_ref = PacketGetInt(p);
				m->memberid = PacketGetShort(p);
				m->relay = PacketGetShort(p);
				m->flags = PacketGetInt(p);
				blob = PacketGetBlob(p, &blobLen);
				m->kv = VoiceInitKeyVal(blob, blobLen);

				AddMember(c, m);

				// we need to keep the latest memberid so we can start
				// asigning member ids in case we ever become host
				c->memberid = m->memberid;

				if(c->Callbacks.memberjoined_cbf)
					c->Callbacks.memberjoined_cbf(c, m->memberid, c->Callbacks.context);
			}
		}
		break;

		case NET_MEMBERFLAG_CHANGED:
		{
			MemberId memberid = PacketGetShort(p);
			unsigned int j;

			for(j = 0; j < c->num_members; j++)
			{
				if(c->members[j] && c->members[j]->memberid == memberid)
				{
					c->members[j]->flags = PacketGetInt(p);
					break;
				}
			}
		}
		break;

		case NET_MEMBERDESC_CHANGED:
		{
			MemberId id;
			_Member *tm;

			id = PacketGetShort(p);
			blob = PacketGetBlob(p, &blobLen);
			tm = MemberById(c, id);

			if(tm)
			{
				VoiceDeleteKeyVal(tm->kv);
				tm->kv = VoiceInitKeyVal(blob, blobLen);
				if(c->Callbacks.memberdescchanged_cbf)
					c->Callbacks.memberdescchanged_cbf(c, id, c->Callbacks.context);

			}
		}

		case NET_VOICEDATA:
		{
			SOCKADDR_IN to;;

			to.sin_addr.s_addr = inet_addr("127.0.0.1");
			to.sin_port = htons(c->local_member->port);
			to.sin_family = AF_INET;
			
			sendto(gUDPSocket, PacketBuffer(p), PacketLength(p), 0, (const struct sockaddr *)&to, sizeof(SOCKADDR_IN));
		}
		break;

		case NET_REHOST:
		{
		}
		break;

		case NET_REHOSTFAILED:
		{
			c->state = STATE_REMOVE;
			if(c->Callbacks.connectionlost_cbf)
				c->Callbacks.connectionlost_cbf(c, connectionlostRehostFailed, c->Callbacks.context);
		}
		break;
		}
		assert(isValidChannel(c));
	}

	if((err != 0) && (err != WSAEWOULDBLOCK))
	{
		// We lost connection to the host
		_Member *cm = NULL;
		MemberId mID;

		if(!isValidChannel(c))
		{
			assert(0);
			return;
		}

		mID = c->host_member->memberid;
		RemoveMember(c, c->host_member);

		for(i = 0; i < c->num_members; i++)
			if((cm = c->members[i]) && !(cm->flags & MFLAG_UNREACHABLE)) 
				break;

		if(cm)
		{
			c->host_member = cm;
			if(c->host_member == c->local_member)
				c->IsHost = voiceTrue;
			c->state = STATE_REHOSTING;

			if(c->Callbacks.memberdisconnect_cbf)
				c->Callbacks.memberdisconnect_cbf(c, mID, memberdisconnectLeft, c->Callbacks.context);
		}
		else
		{
			c->state = STATE_REMOVE;
			if(c->Callbacks.connectionlost_cbf)
				c->Callbacks.connectionlost_cbf(c, connectionlostDisconnected, c->Callbacks.context);
		}
	}

}

static void ActiveChannelProcess(_Channel *c)
{
	if(!isValidChannel(c))
	{
		assert(0);
		return;
	}

	if(c->IsHost)
		ActiveChannelProcessHost(c);

	if(c->TCPHostSocket != INVALID_SOCKET)
		ActiveChannelProcessMember(c);
}

static void RemoveChannelProcess(_Channel *c)
{
	unsigned int i;
	_Member *m;
	voiceBool hadmembers = voiceFalse;

	if(!isValidChannel(c))
	{
		assert(0);
		return;
	}
	
	if(c->TCPHostSocket != INVALID_SOCKET)
		VoiceNetCloseSocket(c->TCPHostSocket);

	if(c->codec)
	{ 
		if(c->encode_state)
			c->codec->destroy_state(c->encode_state);
	}

	if(c->num_members)
		hadmembers = voiceTrue;
	while((c->num_members) && (m = c->members[0]))
		RemoveMember(c, m);
	if(hadmembers)
		gsifree(c->members);
	if(c->kv)
	{
		VoiceDeleteKeyVal(c->kv);
	}
	if(c->address)
		gsifree(c->address);

	VoiceChannelSendListReset(c);

	for(i = 0 ; i < c->banlist_len ; i++)
		gsifree(c->banlist[i]);

	for(i = 0; i < MAX_CHANNELS; i++)
	{
		if(g_Channel[i] == c)
			g_Channel[i] = NULL;
	}

	gsifree(c->excessAudio);

	gsifree(c);
	c = NULL;
}

void VoiceThink( void )
{
	_Channel *c;
	unsigned int i;
	static voiceBool processing = voiceFalse;

	// make sure we are not already processing for thread safety
	if(processing || gUDPSocket==INVALID_SOCKET || gTCPSocket==INVALID_SOCKET)
		return;
	processing = voiceTrue;

	// process any cached network activity
	VoiceNetThink();
	
	// check for incoming connections
	AcceptConnections();

	for( i = 0; i < MAX_CHANNELS; i++ )
	{

		if((c = g_Channel[i]))
		{
			if(c->state == STATE_LOGIN)
				LoginChannelProcess(c);

			if(c->state == STATE_CONNECTING)
				ConnectingChannelProcess(c);

			if(c->state == STATE_REHOSTING)
				RehostingChannelProcess(c);

			if(c->state == STATE_ACTIVE)
				ActiveChannelProcess(c);

			if(c->state == STATE_REMOVE)
			{
				RemoveChannelProcess(c);
				// mark the channel as NULL so we know
				// we can recycle the channel slot
				g_Channel[i] = NULL;
			}
		}
	}

	if(g_VoiceCapture)
		g_VoiceCapture->process();

	if(g_VoicePlayback)
		g_VoicePlayback->process();

	// check for incoming voice data
	AcceptData();

	// decode, mix, and play collected audio
	AudioProcess();

	// reopen processing
	processing = voiceFalse;
}

void VoiceRecordStart(Channel channel)
{
	_Channel *c = (_Channel *)channel;
	int i;

	if(!isValidChannel(c))
	{
		assert(!c); // If it's invalid, it should be null - otherwise someone passed in an "old" or invalid channel value
	}

	if(isValidChannel(c))
	{
		// We shouldn't send voice when we're not connected
		assert(c->state == STATE_ACTIVE);
		if( c->state != STATE_ACTIVE )
			return;

		if(c->local_member && !c->local_member->IsTalking)
		{
			c->local_member->IsTalking = voiceTrue;
			if(c->Callbacks.talkingstarted_cbf)
				c->Callbacks.talkingstarted_cbf(c, c->local_member->memberid, c->Callbacks.context);
		}
	}
	else
	{
		for(i = 0; i < MAX_CHANNELS; i++)
		{
			if((c = g_Channel[i]) && c->local_member && !c->local_member->IsTalking && c->state == STATE_ACTIVE)
			{
				c->local_member->IsTalking = voiceTrue;
				if(c->Callbacks.talkingstarted_cbf)
					c->Callbacks.talkingstarted_cbf(c, c->local_member->memberid, c->Callbacks.context);
			}
		}
	}

	gActivateLevel = 0;
	if(g_VoiceCapture && g_VoiceCapture->record_start)
		g_VoiceCapture->record_start();
}

void VoiceRecordStop(Channel channel)
{
	_Channel *c = (_Channel *)channel;
	int i;	

	if(!isValidChannel(c))
	{
		assert(!c); // If it's invalid, it should be null - otherwise someone passed in an "old" channel
	}

	if( isValidChannel(c) )
	{
		if(c->local_member && c->local_member->IsTalking)
		{
			c->local_member->IsTalking = voiceFalse;
			c->justStoppedTalking = voiceTrue;
			if(c->Callbacks.talkingstopped_cbf)
				c->Callbacks.talkingstopped_cbf(c, c->local_member->memberid, c->Callbacks.context);
		}
	}
	else
	{
		for(i = 0; i < MAX_CHANNELS; i++)
		{
			if((c = g_Channel[i]) && c->local_member && c->local_member->IsTalking)
			{
				c->local_member->IsTalking = voiceFalse;
				c->justStoppedTalking = voiceTrue;
				if(c->Callbacks.talkingstopped_cbf)
					c->Callbacks.talkingstopped_cbf(c, c->local_member->memberid, c->Callbacks.context);
			}
		}
	}

	gActivateLevel = 0;
	if(g_VoiceCapture && g_VoiceCapture->record_stop)
		g_VoiceCapture->record_stop();
}

voiceBool VoiceChannelHost(Channel channel, MemberId memberid)
{
	_Channel *c = (_Channel *)channel;

	if(!isValidChannel(c))
	{
		assert(0);
		return voiceFalse;
	}

	assert(c->host_member);
	if(!c->host_member)
		return voiceFalse;

	return (voiceBool)(c->host_member->memberid == memberid);
}

const char * VoiceGetChannelName(Channel channel)
{
	return VoiceGetChannelString(channel, "channelname", "");
}

unsigned int VoiceGetMemberCount(Channel channel)
{
	_Channel *c = (_Channel *)channel;
 
	if(!isValidChannel(c))
	{
		assert(0);
		return 0;
	}

	return c->num_members;
}

// Channel and Member wrappers for the Key / Value functions

// get member

const char *VoiceGetMemberString(Channel channel, MemberId memberid, const char *key, const char *def)
{
	_Channel *c = (_Channel *)channel;
	_Member *m = NULL;
	
	if(!isValidChannel(c))
	{
		assert(0);
		return def;
	}

	m = MemberById(c, memberid);
	assert(m);
	if(!m)
		return def;

	return VoiceGetString(m->kv, key, def);
}

void VoiceEnumMemberKeys(Channel channel, MemberId memberid, VoiceKeyEnumFn KeyFn, void* instance)
{
	_Channel *c = (_Channel *)channel;
	_Member *m = NULL;
	
	if(!isValidChannel(c))
	{
		assert(0);
		return;
	}

	assert(KeyFn);
	if(!KeyFn)
		return;

	m = MemberById(c, memberid);
	assert(m);
	if(!m)
		return;

	VoiceEnumKeys(m->kv, KeyFn, instance);
}

voiceBool VoiceMuteMemberPlayback(Channel channel, MemberId memberid, voiceBool mute)
{
	_Channel *c = (_Channel *)channel;
	_Member *m = NULL;
	
	if(!isValidChannel(c))
	{
		assert(0);
		return voiceFalse;
	}

	m = MemberById(c, memberid);
	assert(m);
	if(!m)
		return voiceFalse;

	m->mute = mute;

	return voiceTrue;
}

voiceBool VoiceIsMemberPlaybackMuted(Channel channel, MemberId memberid)
{
	_Channel *c = (_Channel *)channel;
	_Member *m = NULL;
	
	if(!isValidChannel(c))
	{
		assert(0);
		return voiceFalse;
	}

	m = MemberById(c, memberid);
	assert(m);
	if(!m)
		return voiceFalse;

	return m->mute;
}

int VoiceGetMemberInt(Channel channel, MemberId memberid, const char *key, int def)
{
	_Channel *c = (_Channel *)channel;
	_Member *m = NULL;
	
	if(!isValidChannel(c))
	{
		assert(0);
		return def;
	}

	m = MemberById(c, memberid);
	assert(m);
	if(!m)
		return def;

	return VoiceGetInt(m->kv, key, def);
}

// set member
static void VoiceMemberDescBroadcast(_Channel *c, _Member *m)
{
	Packet p = g_Packet;

	if(!isValidChannel(c))
	{
		assert(0);
		return;
	}

	assert(m);
	if(!m)
		return;

	// We shouldn't send user data when we're not connected
	assert(c->state == STATE_ACTIVE);
	if( c->state != STATE_ACTIVE )
		return;
	
	PacketClear(p);

	PacketAddByte(p, NET_MEMBERDESC_CHANGED);
	PacketAddShort(p, m->memberid);
	PacketAddBlob(p, VoiceGetKeyValBuffer(m->kv), (unsigned short)VoiceGetKeyValLength(m->kv));

	if(c->IsHost)
	{
		BroadcastPacket(c, p);
		if(c->Callbacks.memberdescchanged_cbf)
			c->Callbacks.memberdescchanged_cbf(c, m->memberid, c->Callbacks.context);
	}
	else
		SendPacketTCP(c->TCPHostSocket, p);

}

voiceBool VoiceSetMemberString(Channel channel, MemberId memberid, const char *key, const char *value)
{
	_Channel *c = (_Channel *)channel;
	_Member *m = MemberById(c, memberid);

	if(!isValidChannel(c))
	{
		assert(0);
		return voiceFalse;
	}

	assert(m);
	if(!m)
		return voiceFalse;

	if(m->memberid != c->local_member->memberid && !c->IsHost)
		return voiceFalse;

	if(m && VoiceSetString(m->kv, key, value))
	{
		VoiceMemberDescBroadcast(c,m);
		
		return voiceTrue;
	}

	return voiceFalse;
}

voiceBool VoiceSetMemberInt(Channel channel, MemberId memberid, const char *key, int value)
{
	_Channel * c = (_Channel *)channel;
	_Member *m = MemberById(c, memberid);
	voiceBool ret;

	if(!isValidChannel(c))
	{
		assert(0);
		return voiceFalse;
	}

	assert(m);
	if(!m)
		return voiceFalse;

	if(m->memberid != c->local_member->memberid && !c->IsHost)
		return voiceFalse;

	ret = VoiceSetInt(m->kv, key, value);

	VoiceMemberDescBroadcast(c,m);

	return ret;

}

// get channel

const char *VoiceGetChannelString(Channel channel, const char *key, const char *def)
{
	_Channel * _channel = (_Channel *) channel;
	if(!channel)
		return def;
	return VoiceGetString(_channel->kv, key, def);
}

int VoiceGetChannelInt(Channel channel, const char *key, int def)
{
	_Channel * _channel = (_Channel *) channel;
	if(!channel)
		return def;
	return VoiceGetInt(_channel->kv, key, def);
}

voiceBool VoiceGetChannelBool(Channel channel, const char *key, voiceBool def)
{
	_Channel * _channel = (_Channel *) channel;
	if(!channel)
		return def;
	return VoiceGetBool(_channel->kv, key, def);
}

void VoiceEnumChannelKeys(Channel channel, VoiceKeyEnumFn KeyFn, void* instance)
{
	_Channel * _channel = (_Channel *) channel;
	if(!channel)
		return;
	VoiceEnumKeys(_channel->kv, KeyFn, instance);
}

// set channel

voiceBool VoiceSetChannelString(Channel channel, const char *key, const char *value)
{
	_Channel * _channel = (_Channel *) channel;
	if(!channel)
		return voiceFalse;
	return VoiceSetString(_channel->kv, key, value);
}

voiceBool VoiceSetChannelInt(Channel channel, const char *key, int value)
{
	_Channel * _channel = (_Channel *) channel;
	if(!channel)
		return voiceFalse;
	return VoiceSetInt(_channel->kv, key, value);
}

voiceBool VoiceSetChannelBool(Channel channel, const char *key, voiceBool value)
{
	_Channel * _channel = (_Channel *) channel;
	if(!channel)
		return voiceFalse;
	return VoiceSetBool(_channel->kv, key, value);
}

unsigned long VoiceGetChannelUserData(Channel channel)
{
	_Channel * c = (_Channel *)channel;

	if(!isValidChannel(c))
	{
		assert(0);
		return 0;
	}

	return c->userdata;
}

voiceBool VoiceSetChannelUserData(Channel channel, unsigned long userdata)
{
	_Channel * c = (_Channel *)channel;

	if(!isValidChannel(c))
	{
		assert(0);
		return voiceFalse;
	}

	c->userdata = userdata;

	return voiceTrue;
}

voiceBool VoiceMuteChannelPlayback(Channel channel, voiceBool mute )
{
	_Channel * c = (_Channel *)channel;

	if(!isValidChannel(c))
	{
		assert(0);
		return voiceFalse;
	}

	c->mute = mute;

	return voiceTrue;
}

voiceBool VoiceIsChannelPlaybackMuted(Channel channel)
{
	_Channel * c = (_Channel *)channel;

	if(!isValidChannel(c))
	{
		assert(0);
		return voiceFalse;
	}

	return c->mute;
}
