/**
* \file     GuaranteeProtocol.cpp
* \brief    
* \author   Zhi Chen
* \version  1.0
* \date		8/20/2002
*/


#include "x_files.hpp"
#include "MemoryStream.h"
#include "CXSocket.h"
#include "GuaranteeProtocol.h"


GP_SOCKETCONTEXT  g_SocketContext; 

pfnUDP_CALLBACK AppCallback = NULL;

void Socket_Context_Callback(u32 dwmsg, const void *pData, u16 wLen, GP_SOCKETCONTEXT hSocket, const struct X_SOCKADDRIN *pAddr, void *pContext )
{
	AppCallback(dwmsg, pData, wLen, pAddr, pContext);
}

u32 UDP_Init( pfnUDP_CALLBACK fn, char *pLocalIP, u16 wLocalPort, u32 dwFlag)
{
	//start up socket system
	XSOCK_Startup();

	CXSocket csocket;
	csocket.Create(SOCK_DGRAM, pLocalIP, wLocalPort );
	csocket.SetToNoBlocking();

	g_SocketContext = GP_CreateSocketContext(csocket.Detach(), dwFlag);

	if(g_SocketContext == NULL)
		return (1);

	AppCallback = fn;
	GP_SetSocketContextCallBack(g_SocketContext, Socket_Context_Callback);	
	return (0);
}

u32 UDP_Shutdown ( )
{
	GP_DestroySocketContext(g_SocketContext);

	//shutdown the socket system.
	XSOCK_Shutdown();
	return (0);
}

u32 UDP_SendMessage( const void *pdata, u16 wlen, struct X_SOCKADDRIN  *pDest_addr, GP_SENDMSGFLAG flag)
{
	return GP_SocketSendTo(g_SocketContext, pdata, wlen, pDest_addr, flag);
}

void UDP_Tick( void *pContext )
{
	GP_TickSocketContext(g_SocketContext, pContext);
}
