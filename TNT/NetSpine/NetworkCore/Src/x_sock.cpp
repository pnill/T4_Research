/**
* \file     x_sock.c
*
* \brief    platform independent source file
*
* \author   Daniel Melfi 
*
* \version  1.0 
* 
* \date     8/23/2001
*/


//////////////////////////////////////////////////////////////////////////
// INCLUDES
#include "x_files.hpp"
#include "x_sock.h"
#ifdef TARGET_UNIX
#include <ctype.h>
#elif defined(TARGET_XBOX)
#include <xtl.h>
#endif


//////////////////////////////////////////////////////////////////////////
// DEFINES 

//////////////////////////////////////////////////////////////////////////
// TYPES 

//////////////////////////////////////////////////////////////////////////
// GLOBALS 
static u32 g_dwStartupRefCount = 0;
 
#ifdef TARGET_PS2
u32 DoPSX2Startup( void );
u32 DoPSX2Shutdown( void );
void XSOCK_PrivSetLastError( int newErr );
s32 XSOCKInternal_SetBlockingMode( X_SOCKET s, s32 dwNewBlockMode );
s32 XSOCKInternal_ResolveDNS( const XCHAR *pDNS, XCHAR *pIP, s32 sdwLenghtofIP, u32 Retries );
s32 XSOCKInternal_ReverseDNS( XCHAR *name, u32 namelen, struct X_SOCKADDR *addr, u32 Retries );
#endif

//s32 XSOCKInternal_SetBlockingMode( X_SOCKET s, s32 dwNewBlockMode );


#if (defined (TARGET_WIN32) || defined(TARGET_SDC_WINCE) )
u32 DoWindowsStartup( void )
{
WSADATA wsaData;
u32 SocketVersionHigh, SocketVersionLow;

    if ( WSAStartup(MAKEWORD(2,2),&wsaData) == X_SOCKET_ERROR ) 
    { 
	    WSACleanup(); 

        //EXIT_FAILURE
	    return ( 1 ); 
    } 

	SocketVersionHigh = HIBYTE( wsaData.wHighVersion );
	SocketVersionLow = LOBYTE( wsaData.wHighVersion) ;

//EXIT_SUCCESS
return ( 0 );
}
#endif


#if defined (TARGET_XBOX)
//-----------------------------------------------------------------------------
// Name: XBNet_Init()
// Desc: Performs all required initialization for network play
//-----------------------------------------------------------------------------
u32 XBNet_Init(void)
{
    WSADATA WsaData;	
    // Initialize the network stack. For default initialization, call
    // XNetStartup( NULL );
    XNetStartupParams xnsp;
    ZeroMemory( &xnsp, sizeof(xnsp) );
    xnsp.cfgSizeOfStruct = sizeof(xnsp);
    xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY ;
    if( XNetStartup( &xnsp ) != NO_ERROR )
        return 1;

    // Standard WinSock startup. The Xbox allows all versions of Winsock
    // up through 2.2 (i.e. 1.0, 1.1, 2.0, 2.1, and 2.2), although it 
    // technically supports only and exactly what is specified in the 
    // Xbox network documentation, not necessarily the full Winsock 
    // functional specification.
    if( WSAStartup( MAKEWORD(2,2), &WsaData ) != NO_ERROR )
        return 1;

    return 0;
}


//-----------------------------------------------------------------------------
// Name: XBNet_Cleanup()
// Desc: Performs all required cleanup for network play
//-----------------------------------------------------------------------------
u32 XBNet_Cleanup(void)
{
    // Shutdown sockets
    INT iResult = WSACleanup();
    BOOL bSuccess = ( iResult == NO_ERROR );

    // Shutdown network stack
    // Always call XNetCleanup even if WSACleanup failed
    iResult = XNetCleanup();
    bSuccess &= ( iResult == NO_ERROR );

    return( bSuccess ? 0 : 1 );
}


#endif

/*========================================================================
 * Function: XSOCK_Startup()
 * Desc    : Initialize and verify that sockets can be used.
 */
s32 XSOCK_Startup( void )
{
    
    if ( 0 == g_dwStartupRefCount )
	{
//        x_CoreRuntimeStartUp();
        #if (defined (TARGET_WIN32) || defined(TARGET_SDC_WINCE) )
            if( 0!= DoWindowsStartup() )
            {
                //EXIT_FAILURE
                return ( 1 );
            }
		#elif defined(TARGET_XBOX)
			if(0 != XBNet_Init())
			{
               //EXIT_FAILURE
				return 1;
			}
        #elif defined TARGET_PS2
            if( 0!= DoPSX2Startup() )
            {
                //EXIT_FAILURE
                return ( 1 );
            }
        #endif
	}

    g_dwStartupRefCount++;

//EXIT_SUCCESS
return ( 0 );
}


/*========================================================================
 * Function: XSOCK_Shutdown()
 * Desc    : Shuts down socket layer. 
 */
void XSOCK_Shutdown( void )
{ 
    if( g_dwStartupRefCount == 1 )
    {
//        x_CoreRuntimeShutDown();
        // Do the system shutdown code here
        #if (defined (TARGET_WIN32) || defined(TARGET_SDC_WINCE) )
            WSACleanup();
		#elif defined(TARGET_XBOX)
			XBNet_Cleanup();
        #elif defined TARGET_PS2
            DoPSX2Shutdown();
        #endif        
    }

    if( g_dwStartupRefCount > 0 )
        g_dwStartupRefCount--;

//EXIT_SUCCESS
return;
}


/*========================================================================
 * Function: XSOCK_IsReady()
 * Returns current state of socket library.
 */ 
xbool XSOCK_IsReady( void )
{
    return ( g_dwStartupRefCount > 0 );
}



/*========================================================================
 * Function: XSOCK_ResolveDNS()
 * Desc    : looks up a name 
 */
s32 XSOCK_ResolveDNS( const XCHAR *pDNS, XCHAR *pIP, s32 sdwLenghtofIP )
{
#if (!defined(TARGET_XBOX) && !defined(TARGET_DOLPHIN))
    struct X_HOSTENT * pTempHostent;

    if( ! XSOCK_IsReady() )
    {
		ASSERT(!"Sockets not started properly!");
        //EXIT_FAILURE
        return ( -1 );
    }

    if( sdwLenghtofIP < (s32)SIZEOF_SOCKADDR_IN )
    {
        //EXIT_FAILURE
        return ( -1 );
    }

    pTempHostent = x_gethostbyname( pDNS );
    
	if ( !pTempHostent ) 
	{
		return ( -1 );
	}

	if ( pTempHostent->h_addr_list[0]  ==  NULL )
	{
        
		return ( -1 );
	}
	else
	{
		//we are only supporting AF_INET right now.
		//count the null terminator.
		if ( pTempHostent->h_addrtype != AF_INET ||  sdwLenghtofIP < 16  )
		{
            
			return (-1);
		}
		{
			struct X_SOCKADDRIN sAddr;

            // Copy the address information from the pHostEnt to a sockaddr_in
			// structure.
			x_memcpy ( &sAddr.sin_addr.s_addr, pTempHostent->h_addr_list[0], pTempHostent->h_length);

			//Copy the address to the string.
			x_sprintf( pIP, "%s", x_inet_ntoa(sAddr.sin_addr) );
            
		}
	}
#endif
//EXIT_SUCCESS
return ( 0 );    
}



/**
*  \fn s32 XSOCK_ReverseDNS( XCHAR *name, u32 namelen, struct X_SOCKADDR *addr )
*  \brief <TODO :: add brief description>
*  
*  \b Additional-Info If needed add more detailed information here.
*/ 
s32 XSOCK_ReverseDNS( XCHAR *name, u32 namelen, struct X_SOCKADDR *addr )
{
#ifndef TARGET_XBOX
    struct X_HOSTENT * pTempHostent = NULL;

    u32 tmpAddr = ((struct X_SOCKADDRIN*)addr)->sin_addr.s_addr;

    if( NULL == name || NULL == addr )
    {
        //EXIT_FAILURE
        return ( -1 );
    }

    if( ! XSOCK_IsReady() )
    {
        //EXIT_FAILURE
        return ( -1 );
    }

	pTempHostent = x_gethostbyaddr( (const char *)&tmpAddr, sizeof(tmpAddr), AF_INET );
    
	if ( !pTempHostent ) 
	{
        //A_DError("Err=%d\n", X_HERRNO );
		return ( -1 );
	}

	if ( pTempHostent->h_addr_list[0]  ==  NULL )
	{
		return ( -1 );
	}
	else
	{
		//Copy the name
        x_strncpy(name, pTempHostent->h_name, namelen );
	}
#endif
//EXIT_SUCCESS
return ( 0 );
}



/*========================================================================
 * Function: XSOCK_SetBlockingMode()
 * Desc    : Sets a particular socket to block, or not, during recv() 
 */
s32     XSOCK_SetBlockingMode( X_SOCKET s, s32 NewBlockMode )
{
	ASSERT(XSOCK_IsReady());
#if defined(TARGET_WIN32) || defined ( TARGET_SDC_WINCE ) || defined(TARGET_UNIX) || defined(TARGET_XBOX)
    
	return x_ioctl( s, FIONBIO, (u32*)&NewBlockMode ) ;    

#elif defined(TARGET_PS2)
 
	//return x_ioctl( s, FIONBIO, (u32*)&NewBlockMode ) ;    
    return XSOCKInternal_SetBlockingMode( s, NewBlockMode );

#endif

  //EXIT_FAILURE
  return ( 1 );
}

s32 XSOCK_SetTimeout( X_SOCKET s, u32 dwMilliseconds )
{
#if defined TARGET_PS2
	return x_setsockopt( s, SOL_SOCKET, SO_RCVTIMEO, (char*)&dwMilliseconds, sizeof(dwMilliseconds)) ;
	//return sceInsockSetRecvTimeout( s, dwMilliseconds );
#elif defined TARGET_WIN32
	return x_setsockopt( s, SOL_SOCKET, SO_RCVTIMEO, (char*)&dwMilliseconds, sizeof(dwMilliseconds)) ;
#elif defined (TARGET_DOLPHIN)
	return 0;
#else
	ASSERT(!"Not implemented for this platform");
	return -1;
#endif
}

/*========================================================================
 * Function: XSOCK_GetErrText()
 * Desc    : 
 */
const XCHAR * XSOCK_GetErrText(SOCKERRTYPE socketerr)
{
	switch(socketerr)
	{
        case 0 : return             "SocketErr: SUCCESS"; break;

        case EIO: return            "[EIO] I/O error"; break;
        case EBADS: return          "[EBADS] socket descriptor is invalid"; break;
        case EWOULDBLOCK: return    "[EWOULDBLOCK] function would block"; break;
        case EMSGSIZE: return       "[EMSGSIZE] message to big for buffers"; break;
        case ESOCKTNOSUPPORT: return"[ESOCKTNOSUPPORT] Socket type not supported"; break;
        case EPFNOSUPPORT: return   "[EPFNOSUPPORT] Protocol family not supported"; break;
        case EAFNOSUPPORT: return   "[EAFNOSUPPORT] Address family not supported"; break;
        case ECONNABORTED: return   "[ECONNABORTED] User requested hangup"; break;
        case ENOBUFS: return        "[ENOBUFS] No buffers available"; break;
        case EISCONN: return        "[EISCONN] Socket has closed"; break;
        case ENOTCONN: return       "[ENOTCONN] Socket is not connected"; break;
        case ESHUTDOWN: return      "[ESHUTDOWN] Socket is closed"; break;
        case ETOOMANYREFS: return   "[ETOOMANYREFS] Too many sockets open"; break;
        case ETIMEDOUT: return      "[ETIMEDOUT] Connection timed out"; break;
        case ECONNREFUSED: return   "[ECONNREFUSED] Connection refused"; break;
        case EINPROGRESS: return    "[EINPROGRESS] The socket is non-blocking and the connection cannot be completed immediately"; break;
        case EALREADY: return       "[EALREADY] The socket is non-blocking and a previous connection attempt has not yet completed"; break;
        case ENOTSOCK: return       "[ENOTSOCK] Socket operation on a non-socket"; break;
        case EDESTADDRREQ: return   "[EDESTADDRREQ] The destination address is invalid"; break;
        case EPROTOTYPE: return     "[EPROTOTYPE] Protocol wrong type for socket"; break;
        case ENOPROTOOPT: return    "[ENOPROTOOPT] Protocol not available"; break;
        case EPROTONOSUPPORT: return"[EPROTONOSUPPORT]Protocol not supported"; break;
        case EOPNOTSUPP: return     "[EOPNOTSUPP] Operation not supported on socket"; break;
        case EADDRINUSE: return     "[EADDRINUSE] Address is already in use"; break;
        case EADDRNOTAVAIL: return  "[EADDRNOTAVAIL] Address not available"; break;
        case ENETUNREACH: return    "[ENETUNREACH] Network is unreachable"; break;
        case ENETRESET : return     "[ENETRESET] Network dropped connection on reset"; break;
        case ECONNRESET: return     "[ECONNRESET] The connection has been reset by the peer"; break;
        case EINTR: return          "[EINTR] Interrupted system call"; break;
        case EINVAL: return         "[EINVAL] Invalid argument"; break;

#ifdef TARGET_WIN32
        case EACCES: return      "[WSAEACCES]"; break;
        case EFAULT: return      "[WSAEFAULT]"; break;
        case EMFILE: return      "[WSAEMFILE]"; break;
#endif

        default:
            {
                static char Temp[64];
                x_sprintf( Temp, "SocketErr = %d :: ERROR CODE NOT DEFINED", socketerr );
                return Temp; 
            }
            break;
	}
}


/*========================================================================
 * Function: XSOCK_GetHostInfo()
 * Desc    : 
 */
s32 XSOCK_GetHostInfo( XCHAR *pszName, s32 sdwNameSize, XCHAR *pszIP, s32 sdwIPSize )
{
    s32 Err = 0;

#ifdef TARGET_WIN32

    if( NULL != pszName && 0 < sdwNameSize )
        Err += x_gethostname( pszName, sdwNameSize );

    if( NULL != pszIP && 0 < sdwIPSize )
        Err += XSOCK_ResolveDNS( pszName, pszIP, sdwIPSize );

#elif defined TARGET_PS2
    // This was added by DM for the PS2 Quake port
    ASSERT(!"Not handled::XSOCK_GetHostInfo()");    
#endif

    // return result
    return (0!=Err);
}

#ifdef TARGET_XBOX

//See XNetInAddrToString() from XDK
XCHAR * x_inet_ntoa ( struct X_INADDR hostAddress )
{
	static XCHAR pTemp[64]; //not thread safe

	//big endian format
	u32 dw1 = hostAddress.S_un.S_un_b.s_b1;
	u32 dw2 = hostAddress.S_un.S_un_b.s_b2;
	u32 dw3 = hostAddress.S_un.S_un_b.s_b3;
	u32 dw4 = hostAddress.S_un.S_un_b.s_b4;

	x_sprintf(pTemp, "%u.%u.%u.%u",dw1,dw2,dw3,dw4);
	return pTemp;
}
#endif

