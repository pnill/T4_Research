#if !defined(_DOL_SOCKIMP_H_)
#define _DOL_SOCKIMP_H_

#include <dolphin/types.h>
#include "avetcp.h"

#ifdef __cplusplus
extern "C" {
#endif


/* device ids */

#define DEV_UNKNOWN		0
#define DEV_MODEM		1
#define DEV_BBA			2
#define DEV_LOOPBACK	3

#define ERROR 1

/* ip settings */
#define IP_SETTING_PPP		1
#define IP_SETTING_PPPOE	2
#define IP_SETTING_DHCP		3
#define IP_SETTING_MANUAL	4



#define MAX_SOCKET 5
#define RECV_BUFFER_SIZE 100
#define NUM_SOCKET_BUFFERS 50
#define TCP_RECV_BUFFER_SIZE 1024

#define SCK_OPEN_FLAG		(2^0)
#define SCK_READ_FLAG		(2^1)
#define SCK_WRITE_FLAG		(2^2)
#define SCK_PENDING_FLAG	(2^3)
#define SCK_COMPLETE_FLAG	(2^4)

#define SCK_MASK (SCK_COMPLETE_FLAG - 1)


extern volatile u32 g_connectionEstablished;


typedef struct NIN_SOCKET_BUFFER {
	struct NIN_SOCKET_BUFFER *next;
	AT_IP_ADDR hisAddress;
	s16 hisPort;
	u16 bufferSize;
	u8 data[RECV_BUFFER_SIZE];
}NIN_SOCKET_BUFFER_T;


typedef struct NIN_SOCKET_QUEUE_PTR {
	NIN_SOCKET_BUFFER_T	*front;
	NIN_SOCKET_BUFFER_T	*rear;
}NIN_SOCKET_QUEUE_PTR_T;


typedef struct NIN_SOCKET {

	/* Address information */
	AT_IP_ADDR socketname;
	AT_IP_ADDR peername;
	u32 flags;
	u32 bufferUseCount;
	NIN_SOCKET_QUEUE_PTR_T pending;
	NIN_SOCKET_QUEUE_PTR_T completed;


	u16 type;
	s16 localPort;
	s16 destPort;

	AT_SINT16 networkHandle;

} NIN_SOCKET_T;



extern NIN_SOCKET_T	g_socketDescriptor[];
extern u32 g_freeSocket[];

extern char TCPRecvBuffer[];

extern AT_IF_INFO ifinfo;

s32 GC_Init(u32 dev);

NIN_SOCKET_BUFFER_T *AssignSocketBuffer();
void ReleaseSocketBuffer(NIN_SOCKET_BUFFER_T* buffer);
void AppendSocketBufferToQueue(NIN_SOCKET_QUEUE_PTR_T* queue, NIN_SOCKET_BUFFER_T* buffer);
NIN_SOCKET_BUFFER_T* RemoveSocketBufferFromQueue(NIN_SOCKET_QUEUE_PTR_T* queue);
NIN_SOCKET_BUFFER_T* RemoveSocketBufferFromQueue_AT(NIN_SOCKET_QUEUE_PTR_T* queue, NIN_SOCKET_BUFFER_T* socketBuffer);



AT_SINT16 udp_receiver(AT_SINT16 func, AT_UDP_PRM *param);
void Tcp_recv_callback(AT_SINT16 result, AT_SINT16 nh);
void udp_asr(AT_SINT16 nh, AT_SINT16 event);
void tcp_asr(AT_SINT16 nh, AT_SINT16 event);
void Tcp_send_callback(AT_SINT16 result, AT_SINT16 nh, AT_UBYTE *buffer);


// message queue to block on
extern OSMessageQueue ASRMessageQueue;


#ifdef __cplusplus
    }
#endif


#endif	/*_DOL_SOCKIMP_H_*/