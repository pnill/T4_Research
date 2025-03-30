/**
* \file     GuaranteeAlgorithm.h
*
* \brief    
*
* \author   Zhi Chen
* \version  1.0
* \date		8/22/2002
*/


#ifndef __GUARANTEEALGORITHM_H__
#define __GUARANTEEALGORITHM_H__

/*-----------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------------*/
typedef void * GP_SOCKETCONTEXT;

//TODO - update this documentation

//algorithm for sending unreliable UDP message
/*
1. set the header flag to unreliable message
2. send the message to socket layer.
3. return to application
4. receive side could receive duplicated messages, or none if message is dropped on the way.
*/

//algorithm for sending reliable UDP message
/*
//SEND
1. use destination address to look for existing sessionID from the sessionID container.
2. if sessionID is not found then create one.
3. else use an existing sessionID to package UDP message.
4. place message into the unack message container.
5. send message to socket layer.
6. update session info
7. wait for THAT message to come back, also have resend, and timeout feature
8. Session also has a idle timeout feature, if the session idle for timeout, the session will be remove.

//RECEIVE
1. send back an ack message to sender and forward the message to application level
2. WILL not maintain state of the incoming reliable messages 
3. application can receive duplicated reliable and unreliable messages.
*/

//Message values for dwmsg parameter in pfnGP_CONTEXT_CALLBACK callback routine
/*previous send guarentee messge cannot reach destination*/
#define GP_MESSAGE_UNREACHABLE	(0x00000001) //previoud send need guarentee failed
#define GP_MESSAGE_ACK			(0x00000002) //data send successfully, pData is round trip time (RTT)
#define GP_MESSAGE_DATA			(0x00000004) //data from remove source, maybe duplicated

//callback routine typedef
typedef void (*pfnGP_CONTEXT_CALLBACK)(u32 dwmsg, const void *pData, u16 wLen, GP_SOCKETCONTEXT hSocket, const struct X_SOCKADDRIN *pAddr, void *pContext );

//socket context stat info
typedef struct _StatInfo
{
	u32 dwTotalTxBytes;
	u32 dwTotalRxBytes;
	u32 dwTotalTxRxBytesPerSec;
	u32 dwTotalTxBytesPerSec;
	u32 dwTotalRxBytesPerSec;
}GP_SOCKETCONTEXT_STAT_INFO;

/**
* send message flag - reliable or unreliable message
*/
typedef enum
{
	eUnreliable,
	eReliable,
	eVoice
}GP_SENDMSGFLAG;


//once the socket is used to create a SocketContext, the socket should not be used for any other i/o operation
//return NULL if failed.
//dwFlag (1) to enable netsim, or (0) to disable netsim.
GP_SOCKETCONTEXT   GP_CreateSocketContext ( X_SOCKET s, u32 dwFlag ) ;
//the created socketcontext resource will be released and the Socket handle will return.
X_SOCKET		GP_DestroySocketContext( GP_SOCKETCONTEXT hContext ) ;

X_SOCKET        GP_GetSocketFromContext( GP_SOCKETCONTEXT hContext ) ;

void            GP_SetSocketCustomData ( GP_SOCKETCONTEXT hContext, void *pCustomData ) ;
void*           GP_GetSocketCustomData ( GP_SOCKETCONTEXT hContext ) ;
//returns number of bytes been sent
//otherwise return (-1) indicating error
s32				GP_SocketSendTo		   ( GP_SOCKETCONTEXT hContext, const void *pData, u16 wLen, struct X_SOCKADDRIN  *pDest_addr, GP_SENDMSGFLAG eFlag);

pfnGP_CONTEXT_CALLBACK 
				GP_GetSocketContextCallback( GP_SOCKETCONTEXT hContext ) ;
void			GP_SetSocketContextCallBack( GP_SOCKETCONTEXT hContext, pfnGP_CONTEXT_CALLBACK pfn);

//Do house keeping job of this SocketContext
void			GP_TickSocketContext       ( GP_SOCKETCONTEXT hContext, void *pContext );

void			GP_GetSocketContextStatInfo( GP_SOCKETCONTEXT hContext, GP_SOCKETCONTEXT_STAT_INFO *pStatInfo);

//Netsim functions
void            GP_SetLatency( GP_SOCKETCONTEXT hContext, u32 dwLatency ) ;
void            GP_SetDropRate( GP_SOCKETCONTEXT hContext, u32 dwDropRate ) ;

/*-----------------------------------------------------------------*/
#ifdef __cplusplus
	}
#endif
/*-----------------------------------------------------------------*/



#endif //__GUARANTEEALGORITHM_H__
