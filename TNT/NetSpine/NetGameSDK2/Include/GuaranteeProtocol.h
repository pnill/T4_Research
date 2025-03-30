/**
* \file     GuaranteeProtocol.h
*
* \brief    
*
* \author   Zhi Chen
* \version  1.0
* \date		8/20/2002
*/


#ifndef __GUARANTEEPROTOCOL_H__
#define __GUARANTEEPROTOCOL_H__

#include "GuaranteeAlgorithm.h"

/*-----------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------------*/

typedef void (*pfnUDP_CALLBACK)(u32 dwmsg, const void *pData, u16 wLen, const struct X_SOCKADDRIN *pAddr, void *pContext );

/**
*   \fn u32 Init(callbackroutine, localip, localport); 
*   \brief  
*   @param  fn Callback routine to receive status.
*   @param  pLocalIP Network interface that application want to send data through, usually specify "", which allows OS to choose a best route for destination.
*   @param  wLocalPort Local port to use, usually specify 0, which allows OS to choose a valid port.
*   @param  dwFlag (0) disable NetSim or (1) enable NetSim
*   @return u32
*			@arg \c 0 The function was successfull
*   \b No more details
*/
u32 UDP_Init( pfnUDP_CALLBACK fn, char *pLocalIP, u16 wLocalPort, u32 dwFlag);

/**
*   \fn u32 Shutdown ( ); 
*   \brief  
*   @return u32
*			@arg \c 0 The function was successfull
*   \b No more details
*/
u32 UDP_Shutdown ( );


/**
*   \fn u32 UDP_SendMessage( const void *pdata, u16 wlen, struct sockaddr_in *pDest_addr, SENDMSGFLAG flag); 
*   \brief  send a reliable message to destination address.
*   @param  *pdata Send buffer
*   @param  wlen Send buffer length
*   @param  *pDest_addr Destination address
*   @param  flag need reliable send or unreliable
*   @return u32
*			@arg \c 0 The function was successfull
*			@arg \c 1 wlen larger than 504(512 - 8) bytes
*   \b No more details
*/
u32 UDP_SendMessage( const void *pdata, u16 wlen, struct X_SOCKADDRIN *pDest_addr, GP_SENDMSGFLAG flag);

/**
*   \fn void Tick( ); 
*   \brief  To perform underneath work, if reliable cannot be sent, destination addr will
			return to callback routine.
*   @return void
*   \b No more details
*/
void UDP_Tick( void *pContext );





/*-----------------------------------------------------------------*/
#ifdef __cplusplus
	}
#endif
/*-----------------------------------------------------------------*/



#endif //__GUARANTEEPROTOCOL_H__
