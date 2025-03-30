#include <dolphin.h>


#include "Dol_SockImp.h"

//#include <x_sockdefs.h>
//#include "xSockGC.h"

//#define CPC_USING_X_FILES
//#include "x_memory.hpp"
//#include "c_memory.h"




#define MY_IPADDR	0xC0A80001
#define MY_NETMASK	0xFFFFFF00
#define MY_GATEWAY	0
#define MY_DNS1		0xC0A80131
#define MY_DNS2		0xC0A8011A

/* DHCP */
#define DHCP_HOSTNAME "gamecube"




#define UDP_ECHO_PORT 5000




static OSThread NET_Thread;
static u8 NET_ThreadStack[4096];


#define TIME_DIFF(a,b) ((s32) (a) - (s32) (b))





typedef void*(*stateFuncType)(void*);

static void* failed_state(void* ptr);

/* Global avriables */

AT_IF_INFO ifinfo;

static AT_NINT32 gateway = 0;
static AT_IP_ADDR dns1;
static AT_IP_ADDR dns2;

static AT_SINT16 if_num;
static AT_SINT16 if_num_ppp;
static AT_SINT16 if_num_dix;

static u32 g_deviceType = 0;
static stateFuncType g_statePtr = failed_state;
static u32 g_waitCount;
static AT_PPP_START_ARG gPsarg;
static AT_PPP_MODEM_PROPERTY gPmprop;	/* modem properties */
static u32 g_ipSetting = IP_SETTING_DHCP;



u32 g_freeSocket[MAX_SOCKET];
NIN_SOCKET_T	g_socketDescriptor[MAX_SOCKET];
char TCPRecvBuffer[TCP_RECV_BUFFER_SIZE];

static NIN_SOCKET_BUFFER_T g_socketBuffer[NUM_SOCKET_BUFFERS];
static NIN_SOCKET_BUFFER_T *g_socketBufferFreeList;



// message queue for asr wait
#define ASR_MESG_QUEUE_SIZE 16

OSMessageQueue ASRMessageQueue;
static OSMessage ASRMessageArray[ASR_MESG_QUEUE_SIZE];

volatile u32 g_connectionEstablished = 0;



/* PPP */
#define LOGIN_NAME "acclaim101@mindspring.com"
#define LOGIN_PASSWD "aklm.com"
#define TELEPHONE_NUM "4330780"




/* need this to link ??? */
char *__ctype_map;


static void *WaitFunc(void* ptr);
static void *dataLinkStart_state(void*);
static void *dataLinkStop_state(void*);
static void *dataLinkStopWait_state(void*);
static void *if_config_state(void*);
//static void* failed_state(void* ptr);
static void* monitor_state(void* ptr);
static void* dhcpWait_state(void* ptr);





static OSContext commException;


void longjmp(OSContext* buf, u32 val)
{
	buf->gpr[3] = val;
	OSLoadContext(buf);
}


 

extern void* (*x_malloc) (s32);
extern void (*x_free) (void*);


s32 GC_Init(u32 dev)
{
	u32 type;
	AT_INIT_PARAM param;
	AT_SINT32 size;
	AT_UBYTE* addr;
	AT_SINT16 ret;
	u32 i;
	s32 errCode;

	//set long jump

	errCode = OSSaveContext(&commException);

	switch (errCode) {
	case 4:
		dns_term();
		ppp_term();
	case 3:
		dns_term();
	case 2:
		avetcp_term();
	case 1:
		OSReport("error #%d abort starting nintendo net libs.\n", errCode);
		return -errCode;
	}

	// setup socket descriptors 
	memset((void*) g_freeSocket, 0, sizeof(g_freeSocket[MAX_SOCKET]));
	memset((void*) g_socketDescriptor, 0, sizeof(g_socketDescriptor[MAX_SOCKET]));
#if 0
	for(i=0; i < MAX_SOCKET; ++i) {
		g_socketDescriptor[i].rear = &(g_socketDescriptor[i].empty);
		g_socketDescriptor[i].empty.next = &(g_socketDescriptor[i].empty);
		g_socketDescriptor[i].front = &(g_socketDescriptor[i].empty.next);
	}
#endif

	// setup socket buffer pool
	memset((void*) g_socketBuffer, 0, sizeof(g_socketBuffer[NUM_SOCKET_BUFFERS]));

	for(i=0; i < NUM_SOCKET_BUFFERS - 1; ++i) {
		g_socketBuffer[i].next = &g_socketBuffer[i+1];
	}
	g_socketBuffer[i].next = 0;
	g_socketBufferFreeList = g_socketBuffer;

	if(dev) {
		g_deviceType = dev;
	} else {
		EXIGetType(0, 2, &type);
		if(0x04020200 == type) {
			g_deviceType = DEV_BBA;
			OSReport("Found BBA device\n");

		} else if(EXI_MODEM == type) {
			g_deviceType = DEV_MODEM;
			OSReport("Found Modem device\n");

		} else {
			g_deviceType = DEV_UNKNOWN;
			//("No device found.\n");
			longjmp(&commException, 1);
		}
	}

	// remove 
	#if defined(AT_GC_DEBUG)
		AT_SetDebugFlag(AT_DPLEVEL_PPP);
	#endif

	//AT_SetTaskPriority(16);		/* default */
	size = AT_GetInitParam(&param);
	param.tcp_port_num = 4;
	size = AT_SetInitParam(&param);

	if(size < 0)
		return ERROR;
	OSReport("Allocate memory for nin net libs.\n");
	//addr = (AT_UBYTE*) OSAlloc((u32) size);
	//addr = (void* (*) (s32)) (x_malloc)((u32) size);
	addr = (AT_UBYTE*) x_malloc((u32) size);

	if(addr) {
		ret = AT_SetWorkArea(addr, size);
		if(ret < 0)
			return ERROR;
	}

	ret = avetcp_init();
	if(ret < 0)
		return ERROR;

	
	ret = dns_init();
	if(ret < 0)
	{
		longjmp(&commException, 2);
	}

	// data link init
	switch(g_deviceType) {
	case DEV_BBA:
		ret = ppp_init();
		if(ret < 0) {
			longjmp(&commException, 3);
		}
		if_num_ppp = ret;

		ret = arp_init();
		if(ret < 0) {
			longjmp(&commException, 4);
		}

		ret = dix_init();
		if(ret < 0) {
			longjmp(&commException, 4);
		}

		if_num_dix = ret;

		if(g_ipSetting == IP_SETTING_PPPOE)
			g_statePtr = dataLinkStart_state;
		else if(g_ipSetting == IP_SETTING_DHCP) {


			if_num = if_num_dix;

			ret = (AT_SINT16)DHCP_init(if_num);
			if (ret < 0) {
				g_statePtr = failed_state;
				break;
			}

			ret = (AT_SINT16)DHCP_hostname((AT_UBYTE*)DHCP_HOSTNAME);
			if (ret < 0) {
				DHCP_terminate();
				g_statePtr = failed_state;
				break;
			}
			ifinfo.type = AT_IPv4;
			ret = (AT_SINT16)DHCP_request_nb(0, 0, 0, AT_NULL,
							&ifinfo.v4address, &ifinfo.v4netmask, &ifinfo.v4brc, 5);

			if (ret < 0) {
				DHCP_terminate();
				g_statePtr = failed_state;
				break;
			}
			g_statePtr = dhcpWait_state;

		} else if(g_ipSetting == IP_SETTING_MANUAL) {
			if_num = if_num_dix;

			ifinfo.type = AT_IPv4;
			ifinfo.v4address = AT_HTONL(MY_IPADDR);
			ifinfo.v4netmask = AT_HTONL(MY_NETMASK);
			ifinfo.v4brc = AT_HTONL(0);
			gateway = AT_HTONL(MY_GATEWAY);
			dns1.type = AT_IPv4;
			dns1.v4addr = AT_HTONL(MY_DNS1);
			dns2.type = AT_IPv4;
			dns2.v4addr = AT_HTONL(MY_DNS2);
			g_statePtr = if_config_state;
		}
		break;

	case DEV_MODEM:
		ret = ppp_init();
		if(ret < 0)
		{
			longjmp(&commException, 3);
		}

		if_num_ppp = ret;	/*network handle */
		g_statePtr = dataLinkStart_state;
		break;

	case DEV_LOOPBACK:
		if_num = loopback_init();

		ifinfo.type = AT_IPv4;
		ifinfo.v4address = AT_HTONL(MY_IPADDR);
		ifinfo.v4netmask = AT_HTONL(MY_NETMASK);
		ifinfo.v4brc = AT_HTONL(0);
		gateway = AT_HTONL(MY_GATEWAY);
		dns1.type = AT_IPv4;
		dns1.v4addr = AT_HTONL(MY_DNS1);
		dns2.type = AT_IPv4;
		dns2.v4addr = AT_HTONL(MY_DNS2);

		g_statePtr = if_config_state;

	}

	// initilize block message queue for tcp_asr
	OSInitMessageQueue(&ASRMessageQueue, ASRMessageArray, ASR_MESG_QUEUE_SIZE);



	OSCreateThread(
		&NET_Thread,                            // pointer to the thread to initialize
		WaitFunc,                           // pointer to the start routine
		0,                                  // parameter passed to the start routine
		NET_ThreadStack + sizeof NET_ThreadStack,   // initial stack address
		sizeof NET_ThreadStack,                 // stack size
		16,                                 // scheduling priority - lowest
        OS_THREAD_ATTR_DETACH);             // detached by default
    // Starts the thread
	OSReport("Scheduling network backgroung thread\n");
    OSResumeThread(&NET_Thread);

	OSReport("Network thread scheduled and running\n");

	return 0;
}



NIN_SOCKET_BUFFER_T *AssignSocketBuffer()
{
	NIN_SOCKET_BUFFER_T *buffer = g_socketBufferFreeList;
	if(buffer) {
		g_socketBufferFreeList = g_socketBufferFreeList->next;
		buffer->next=0;
	}
	return buffer;
}




void ReleaseSocketBuffer(NIN_SOCKET_BUFFER_T* buffer)
{
	buffer->next = g_socketBufferFreeList;
	g_socketBufferFreeList = buffer;
}




void AppendSocketBufferToQueue(NIN_SOCKET_QUEUE_PTR_T* queue, NIN_SOCKET_BUFFER_T* buffer)
{
	if (queue->front == NULL) {
		queue->front = buffer;
	} else {
		queue->rear->next = buffer;
	}
	buffer->next = 0;
	queue->rear = buffer;
}



NIN_SOCKET_BUFFER_T* RemoveSocketBufferFromQueue(NIN_SOCKET_QUEUE_PTR_T* queue)
{
	NIN_SOCKET_BUFFER_T *temp;

	if(queue->front == 0) return 0;

	temp = queue->front;
	queue->front = temp->next;

	return temp;
}

NIN_SOCKET_BUFFER_T* RemoveSocketBufferFromQueue_AT(NIN_SOCKET_QUEUE_PTR_T* queue, NIN_SOCKET_BUFFER_T* socketBuffer)
{
	NIN_SOCKET_BUFFER_T *current;
	NIN_SOCKET_BUFFER_T *previous;

	if(queue->front == 0) return 0;

	current = queue->front;
	previous = NULL;

	while((current) && (current->data != socketBuffer->data)) {
		previous = current;
		current = current->next;
	}

	if(previous) {
		if(current == queue->rear) {
			queue->rear = previous;
		}
		previous->next = current->next;
	} else if(current) {
		queue->front = current->next;
	}
	return current;
}




static void set_pmprop(AT_PPP_MODEM_PROPERTY* pmprop)
{

	pmprop->needToInit			= 1;
	pmprop->needToDial			= 1; 
	pmprop->needToCd			= 1;
	pmprop->needToDisc			= 0;
	pmprop->countryCode			= AT_NULL;
	pmprop->connectMode			= AT_GC_MDM_CN_DEFAULT;
	pmprop->errorCorrectMode	= AT_GC_MDM_EC_DEFAULT;
	pmprop->compressMode		= AT_GC_MDM_CM_DEFAULT;
	pmprop->atcommand_count		= 0;
	pmprop->atcommand			= AT_NULL;
}


static void set_psarg(AT_PPP_START_ARG* psarg, s32 pppoe)
{

	psarg->mode					= PP_MODE_DIALOUT;
	if (pppoe)
		psarg->mode				|= PP_MODE_PPPOE;
	psarg->iptype				= AT_IPv4;
	psarg->wait					= 0;
	psarg->login				= (unsigned char *)LOGIN_NAME;
	psarg->password				= (unsigned char *)LOGIN_PASSWD;
	psarg->tele_number			= (unsigned char *)TELEPHONE_NUM;
	psarg->notify				= AT_NULL; /*ppp_connect_notify; */
	psarg->tele.dialtype		= 0;
	psarg->tele.outside_line	= 0;
	psarg->tele.outside_number	= AT_NULL;
	psarg->tele.timeout			= 180;
	psarg->conn.recognize		= 4;
	psarg->conn.mru				= 1500;
	psarg->conn.acfcomp			= 0;
	psarg->conn.protocomp		= 0;
	psarg->conn.vjcomp			= 0;
	psarg->ipcp.local_ip		= 0;
	psarg->ipcp.remote_ip		= 0;
	psarg->ipcp.local_dns1		= 0;
	psarg->ipcp.local_dns2		= 0;
	psarg->ipcp.remote_dns1		= 0;
	psarg->ipcp.remote_dns2		= 0;
}



// Modem states

static void* dataLinkWait_state(void* ptr)
{
	AT_PPP_STAT_ARG gPstat;
	static u32 wait;
	if(ptr) wait = g_waitCount + 1;

	if (TIME_DIFF(g_waitCount, wait) < 0) {
		ppp_stat(&gPstat);
		if (gPstat.state == AT_PPP_STAT_FAIL) {
			OSReport("ppp state failed, abort\n");
			return (void*) failed_state;	// change state

		} else if (gPstat.state == AT_PPP_STAT_ESTABLISHED) {
			ifinfo.type = AT_IPv4;
			ifinfo.v4address = gPstat.ipcp.local_ip;
			ifinfo.v4netmask = AT_HTONL(0);
			ifinfo.v4brc = AT_HTONL(0);
			gateway = 0;
			dns1.type = AT_IPv4;
			dns1.v4addr = gPstat.ipcp.local_dns1;
			dns2.type = AT_IPv4;
			dns2.v4addr = gPstat.ipcp.local_dns2;
			return (void*) if_config_state; // change state
		}
		OSReport("entered data link wait state\n");
		wait = g_waitCount - 10;
	}
	return (void*) 0;
}





static void* if_config_state(void* ptr)
{
	AT_SINT16 ret;
	OSReport("ip addr: %08x\n", ifinfo.v4address);
	OSReport("gateway: %08x\n", gateway);
	OSReport("dns 1  : %08x\n", dns1.v4addr);
	OSReport("dns 2  : %08x\n", dns2.v4addr);
	ret = if_config(if_num, &ifinfo);
	if (ret < 0) {
		return failed_state;
	}
	if (gateway) {
		ret = route4_add(0, 0, gateway);
	}
	dns_clear_server();
	ret = dns_set_server(&dns1);
	ret = dns_set_server(&dns2);
	g_connectionEstablished = 1;

	return monitor_state;

}



static void* failed_state(void* ptr)
{
	if(ptr) OSReport("Failed\n");
	return (void*) 0;
}


static void* monitor_state(void* ptr)
{
	return (void*) 0;
}


static void* dataLinkStop_state(void* ptr)
{
	AT_PPP_STOP_ARG stop;
	AT_SINT16 ret;

	stop.wait = 0;
	ret = ppp_stop(&stop);


	return (void*) dataLinkStopWait_state;
}




static void* dataLinkStopWait_state(void* ptr)
{
	return (void*) 0;
}




static void* dataLinkStart_state(void* ptr)
{
	AT_SINT16 ret;
	s16 pppoe;

	if_num = if_num_ppp;

	set_pmprop(&gPmprop);

	pppoe = ((g_deviceType == DEV_BBA) ? 1 : 0);
	ppp_set_modem_property(&gPmprop);
	set_psarg(&gPsarg, pppoe);
	ret = ppp_start(&gPsarg);

	if(ret < 0)
		return dataLinkStop_state;

	return (void*) dataLinkWait_state;
}



// BBA States



static void* dhcpWait_state(void* ptr)
{
	AT_SINT16 ret;
	static u32 wait;

	if(ptr) wait = g_waitCount + 1;

	if (TIME_DIFF(g_waitCount, wait) < 0) {
		ret = (AT_SINT16)DHCP_timer();
		if(ret == DHCP_STATE_BOUND) {
			ret = (AT_SINT16)DHCP_get_gateway(AT_NULL, &gateway);
			dns1.type = AT_IPv4;
			dns2.type = AT_IPv4;
			ret = (AT_SINT16) DHCP_get_dns(AT_NULL, AT_NULL, &dns1.v4addr, &dns2.v4addr);
			if(ret < 0) return (void*) failed_state;
			return (void*) if_config_state;
		} else if(ret == DHCP_STATE_TIMEOUT || ret == DHCP_STATE_ERROR) {
			DHCP_terminate();
			return (void*) failed_state;	// change state
		}
		OSReport("dhcp wait state\n");
		wait = g_waitCount - 6;
	}
	return (void*) 0;
}





static void* WaitFunc(void* ptr)
{

	stateFuncType funcPtr;
	static u32 firstFlag = 10;

	while(1) {
		--g_waitCount;
		funcPtr = (stateFuncType) g_statePtr((void*)firstFlag);
		if(funcPtr) {
			g_statePtr = funcPtr;
			firstFlag = 10;
			OSReport("Network background thread changed\n");
		} else {
			firstFlag = 0;
		}
		VIWaitForRetrace();
	}

	return 0;
}




void udp_asr(AT_SINT16 nh, AT_SINT16 event)
{

	OSReport("usp asr nethandle %d, event %d\n", nh, event);

}



void tcp_asr(AT_SINT16 nh, AT_SINT16 event)
{
	u32 msgEvent = event;

	OSReport("usp asr nethandle %d, event %d\n", nh, event);
	OSSendMessage(&ASRMessageQueue, (void*) msgEvent, OS_MESSAGE_NOBLOCK);

}




static s32 lookupHandle(u32 handle)
{
	s32 i;
	for(i=0; i < MAX_SOCKET; ++i) {
		if(g_socketDescriptor[i].networkHandle == handle) break;
	}
	return ((i < MAX_SOCKET) ? i : -1);
}




// callback function for nintendo library

void Tcp_recv_callback(AT_SINT16 result, AT_SINT16 nh)
{
	s32 socketIdx;

	if(result >=0) {
		// find socket handle from network handle
		socketIdx = lookupHandle(nh);
		if(socketIdx  < 0) return;		// handle not found
		g_socketDescriptor[socketIdx].flags &= (SCK_PENDING_FLAG ^ SCK_MASK);
		g_socketDescriptor[socketIdx].flags |= SCK_COMPLETE_FLAG;
	}
}


void Tcp_send_callback(AT_SINT16 result, AT_SINT16 nh, AT_UBYTE *buffer)
{
	s32 socketIdx;

	if(result == 0) {
		// find socket handle from network handle
		socketIdx = lookupHandle(nh);
		if(socketIdx  < 0) return;		// handle not found
		g_socketDescriptor[socketIdx].flags &= ((SCK_PENDING_FLAG | SCK_WRITE_FLAG) ^ SCK_MASK);
		x_free(buffer);
	}		

}



AT_SINT16 udp_receiver(AT_SINT16 func, AT_UDP_PRM *param)
{
	s32 idx = lookupHandle(param->recvp.nh);
	NIN_SOCKET_T *socket = &g_socketDescriptor[idx];
	NIN_SOCKET_BUFFER_T *socketBuffer;


	if(idx < 0) return (func ? 0 : 1);	// handle not found

	if(func == 0)
	{
		// discare data
		if(param->recvp.len > RECV_BUFFER_SIZE)
		{
			OSReport("message to long for buffer\n");
			return 1;
		}

		// get a free buffer
		socketBuffer = AssignSocketBuffer();
		if(socketBuffer == 0)
		{
			OSReport("buffer overrun\n");
			return 1;
		}

		// append to end of buffer list
		AppendSocketBufferToQueue(&socket->pending, socketBuffer);

		socketBuffer->hisAddress = param->his_addr;
		socketBuffer->hisPort = param->his_port;
		socketBuffer->bufferSize = param->recvp.len;
		param->recvp.recvbuf = (unsigned char*) socketBuffer->data;

	} else {
		// set flag for receive buffer
		socketBuffer = socket->pending.front;
		while(socketBuffer) {
			if(socketBuffer->data == param->recvp.recvbuf) break;
			socketBuffer = socketBuffer->next;
		}

		if(socketBuffer) {
			RemoveSocketBufferFromQueue_AT(&socket->pending, socketBuffer);
			AppendSocketBufferToQueue(&socket->completed, socketBuffer);
			++socket->bufferUseCount;
		}
	}

	return 0;
}



