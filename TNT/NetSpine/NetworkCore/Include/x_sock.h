/**
* \file     x_sock.h
*
* \brief    <TODO :: add brief description>
*
* \author   Daniel Melfi 
*
* \version  1.0
*
* \date     8/23/2001
*/
//////////////////////////////////////////////////////////////////////////
#ifndef __X_SOCK_H__
#define __X_SOCK_H__
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
// INCLUDES 
#include "x_sockdefs.h"



//////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// DEFINES 
#define X_SET_SOCKET_BLOCKING      0
#define X_SET_SOCKET_NONBLOCKING   1
#define MAX_IPADDR_W_PORT          23
#define MAX_IPADDR_Wo_PORT         16

/**
*   \fn s32 XSOCK_Startup( void )
*   \brief Initialize and verify that sockets can be used.
*   \return (0) If success, else nonzero erro code.
*   \b Additional-Info Must be called before any socket operations
*/ 
s32     XSOCK_Startup( void );



/**
*   \fn void XSOCK_Shutdown( void );
*   \brief Shuts down socket layer.
*
*   \b Additional-Info This should be called to ensure all resources are freed.
*   No other socket function should be call after XSOCK_Shutdown.
*/ 
void     XSOCK_Shutdown( void ) ;



/**
*   \fn xbool XSOCK_IsReady( void )
*   \brief Returns current state of socket library.
*   \return Returns true(!0) else return false(0)
*   \b Additional-Info If needed add more detailed information here.
*/ 
xbool   XSOCK_IsReady( void ) ;



/**
*   \fn const XCHAR * XSOCK_GetErrText( SOCKERRTYPE socketerr )
*   \brief Returns a text description for socket error codes
*   \param A socket error code
*   \return A null terminated string identifying the last error.
*/ 
const XCHAR * XSOCK_GetErrText( SOCKERRTYPE socketerr );



/**
*   \fn s32 XSOCK_SetBlockingMode( X_SOCKET *pSock, s32 NewBlockMode )
*   \brief Sets a particular socket to block, or not, during recv()
*   \param pSock Socket descriptor to be modified.
*   \param dwNewBlockMode New mode
*   \arg \c X_SET_SOCKET_BLOCKING Will wait until new data is received.
*   \arg \c X_SET_SOCKET_NONBLOCKING Will not wait for new data to be received.
*   \return 0 if success, else non-zero error code
*   \b Additional-Info Might not be supported by all platforms, but is the
*      prefered method when for client applications to put a socket into
*      a particular state. Should be used in place of ioctl() when possible.
*/ 
s32    XSOCK_SetBlockingMode( X_SOCKET s, s32 NewBlockMode );
s32		XSOCK_SetTimeout( X_SOCKET s, u32 dwMilliseconds );


/**
*   \fn s32 XSOCK_ResolveDNS( const XCHAR *pDNS, XCHAR *pIP, s32 sdwLenghtofIP )
*   \brief Does a DNS lookup on the supplied name
*   \param pDNS Null-terminated string of symbolic name to resolve
*   \param pIP Reference to area to return address in to
*   \param sdwLenghtofIP Length of buffer to place address into
*   \return (0) If successful, else non-zero error code 
*   \b Additional-Info Use this function on client applications for simple DNS lookups
*      instead of x_gethost... functions
*/ 
s32     XSOCK_ResolveDNS( const XCHAR *pDNS, XCHAR *pIP, s32 sdwLenghtofIP );



/**
*  \fn s32 XSOCK_ReverseDNS( XCHAR *name, u32 namelen, struct X_SOCKADDR *addr )
*  \brief Does a reverse DNS lookup according to a X_SOCKADDR.
*/ 
s32     XSOCK_ReverseDNS( XCHAR *name, u32 namelen, struct X_SOCKADDR *addr );



/**
*  \fn s32 XSOCK_GetHostInfo( XCHAR *pszName, s32 sdwNameSize, XCHAR *pszIP, s32 sdwIPSize )
*  \brief Retrieves info about the current client machine (useful for LAN games)
*  \return (0) Success; non-zero Failure;
*/ 
s32     XSOCK_GetHostInfo( XCHAR *pszName, s32 sdwNameSize, XCHAR *pszIP, s32 sdwIPSize );



/**
*   \fn X_SOCKET x_socket (s32 af, s32 type, s32 protocol)
*   \brief Creates a new socket descriptor
*   \param  af This must be set to AF_INET.
*   \param  type This specifies the type of socket to be created, must be set
*           to either SOCK_STREAM or SOCK_DGRAM.
*   \param  protocol This is ignored, should be 0.
*   \return If successful a new socket descriptor, else X_SOCKET_ERROR is returned.
*/ 
X_SOCKET    x_socket ( s32 af, s32 type, s32 protocol ) ;



/**
*   \fn s32 x_close ( X_SOCKET s )
*   \brief Closes a socket.
*   \param s Socket descriptor to close.
*   \return (0) If successful, on failure the error status X_SOCKET_ERROR.
*   errno() can be used to obtain addtional info.
*   \arg \c ENOTINITIALIZED The socket API has not been initialized
*   \arg \c EBADF The socket descriptor s is invalid
*   \warning Resources may not necessarily be released immediatly following closing.
*/ 
s32     x_close ( X_SOCKET s ) ;



/**
*   \fn s32 x_shutdown ( X_SOCKET s, s32 sdwFlags )
*   \brief Shuts down all or part of a full-duplex socket connection.
*   \param s Socket descriptor identifying the socket to be shut down.
*   \param sdwFlags Specifies how the shutdown is to be performed:
*       \arg \c SD_RECEIVE no further receives are allowed
*       \arg \c SD_SEND no further sends are allowed
*       \arg \c SD_BOTH no further sends or receives are allowed
*   \return (0)If successful, else an error status X_SOCKET_ERROR is returned. errno() can be used
*           to obtain addtional info.
*       \arg \c ENOTINITIALIZED The socket API has not been initialized
*       \arg \c EBADF The socket descriptor s is invalid
*       \arg \c ENOTCONN The socket is not connected
*/ 
s32     x_shutdown ( X_SOCKET s, s32 sdwFlags );



/**
*   \fn s32 x_bind ( X_SOCKET s, const struct X_SOCKADDR *addr, s32 addrlen )
*   \brief Binds a local address to a socket.
*   \param s Descriptor identifying an unbound socket. 
*   \param addr Points to the local address to which the socket should be bound. If
*          addr is NULL an AFINET address of all zeroes will be used and addrlen
*          will be ignored.
*   \param addrlen The size in bytes of the address contained in *addr.
*          Ignored if addr is NULL. 
*   \return If successful: A value of zero is returned.
*           On failure: The error status X_SOCKET_ERROR is returned.
*           errno() can be used to obtain additional info
*           \arg \c ENOTINITIALIZED The socket API has not been initialized
*           \arg \c EBADF The socket descriptor s is invalid
*           \arg \c EADDRNOTAVAIL The specified address is not available at the local host
*           \arg \c EADDRINUSE The specified address is already being used
*           \arg \c EINVAL One of the following conditions: socket is bound;addrlen is invalid
*           ENOMEM Out of memory
*/ 
s32     x_bind ( X_SOCKET s, const struct X_SOCKADDR *addr, s32 addrlen );



/**
*   \fn s32 x_listen ( X_SOCKET s, s32 backlog )
*   \brief Set socket to listening for new connections.
*   \param s Socket descriptor identifying the socket that should listen for connection requests.
*   \param backlog  Maximum number of connection requests which the socket will be able to queue.
*   \return (0)If successful, else error status X_SOCKET_ERROR is returned. 
*           errno() can be used to obtain addtional info.
*           \arg \c ENOTINITIALIZED The socket API has not been initialized
*           \arg \c EBADF The socket descriptor s is invalid
*           \arg \c EOPNOTSUPP The operation is not supported by this type of socket
*/ 
s32     x_listen ( X_SOCKET s, s32 backlog );



/**
*   \fn X_SOCKET x_accept ( X_SOCKET s, struct X_SOCKADDR *addr, s32 *addrlen )
*   \brief Accepts a connection request
*   \param s Socket descriptor identifying the socket on which to wait;
*            x_socket, x_bind and x_listen must already have been called
*            for this socket.
*   \param addr Input: Points to where the address of the client which is making the
*               connection request will be returned by this function. If this is not required,
*               set this parameter to NULL. Output: *addr contains the address of the client
*               (if the function was successful and addr was not set to NULL).
*   \param addrlen Input: Points to a value specifying the maximum size in bytes of the
*                  storage area pointed to by addr. If addr was set to NULL, also set this
*                  to NULL. Output: *addrlen contains the actual length of the address in
*                  bytes (if the function was successful and addr was not set to NULL).
*   \return If successful: A positive (non-zero) socket descriptor will be returned;
*           On failure: The error status X_SOCKET_ERROR is returned; *addr and
*           errno() can be used to obtain the reason for failure:
*     \arg \c ENOTINITIALIZED The socket API has not been initialized
*     \arg \c EBADF The socket descriptor s is invalid
*     \arg \c EINVAL One of the following conditions:
*             The socket is no longer accepting connections
*             addr is not NULL but addrlen is NULL
*             *addrlen specified a size which is not large
*             enough to hold the address
*     \arg \c ECONNABORTED The connection was aborted
*     \arg \c EOPNOTSUPP s is not of type SOCK_STREAM
*     \arg \c EWOULDBLOCK The socket is in non-blocking mode and no connection request is pending
*     \arg \c ENOMEM Out of memory
*/ 
X_SOCKET    x_accept ( X_SOCKET s, struct X_SOCKADDR *addr, X_SOCKLEN *addrlen );



/**
*   \fn s32 x_connect ( X_SOCKET s, const struct X_SOCKADDR *addr, s32 addrlen ) 
*   \brief Connects a socket to a specific address.
*   \param s Descriptor identifying an unconnected socket.
*   \param addr Points to the address to which the socket should be connected.
*   \param addrlen The size in bytes of the address contained in *addr.
*   \return (0) If successful, else an error status X_SOCKET_ERROR is returned.
*           errno() can be used to obtain additional info.
*           \arg \c ENOTINITIALIZED The socket API has not been initialized
*           \arg \c EBADF The socket descriptor s is invalid
*           \arg \c EADDRNOTAVAIL The specified address is not available at the localhost
*           \arg \c EADDRINUSE The specified address is already being used
*           \arg \c EAFNOSUPPORT The address family specified by *addr cannot be used with this socket.
*           \arg \c EINVAL One of the following conditions:
*           \arg \c The socket is already bound to an address;Parameter addr is NULL;Parameter addrlen is not valid.
*           \arg \c EISCONN The socket is already connected
*           \arg \c ETIMEDOUT Timed out before connection established
*           \arg \c ECONNREFUSED The connection was rejected
*           \arg \c EINPROGRESS The socket is non-blocking and the connection 
*           could not be immediately established
*           \arg \c EALREADY The socket is non-blocking and a previous
*           \arg \c connection attempt is in progress
*           \arg \c EOPNOTSUPP This socket does not support this operation
*           \arg \c ENOMEM Out of memory
*/ 
s32     x_connect ( X_SOCKET s, const struct X_SOCKADDR *addr, s32 addrlen ) ;



/**
*   \fn s32 x_recv ( X_SOCKET s, XCHAR *pBuff, s32 sdwBuffLen, s32 sdwFlags )
*   \brief Receive data from a connected socket.
*   \param s Socket descriptor of receiving socket 
*   \param pBuff Address of memory to receive data.
*   \param len The size of *pBuff in bytes. 
*   \param flags Specifies how the receive process should operate.
*       \arg \c MSG_OOB Read out-of-band data
*       \arg \c MSG_PEEK Read the received data, but do not remove it.
*   \return If successful the number of bytes of data stored in *pBuff is returned,
*            else X_SOCKET_ERROR is returned.
*            errno() can be used to obtain additional info
*           \arg \c ENOTINITIALIZED The socket API has not been initialized
*           \arg \c EBADF The socket descriptor s is invalid
*           \arg \c ENOTCONN The socket is not connected
*           \arg \c ECONNRESET The connection has been reset
*           \arg \c EINVAL One of the following conditions: len<0,pBuff==NULL,
*           \arg \c EWOULDBLOCK The socket is in non-blocking mode and no data is available
*/ 
s32     x_recv ( X_SOCKET s, XCHAR *pBuff, s32 sdwBuffLen, s32 sdwFlags ) ;



/**
*   \fn s32 x_recvfrom ( X_SOCKET s, XCHAR *pBuff, s32 sdwBuffLen, s32 sdwFlags, struct X_SOCKADDR *pFromAddr, X_SOCKLEN *sdwFromAddrlen )
*   \brief Receives data from a socket
*   \params Input: Socket descriptor identifying the socket that data should be received.
*   \param pBuff References buffer where received data will be returned
*   \param sdwBuffLen The size of *pBuff in bytes. 
*   \param sdwFlags Specifies how the receive process should operate
*       \arg \c MSG_OOB Read out-of-band data
*       \arg \c MSG_PEEK Read the received data, but do not remove it from the socket
*   \param pFromAddr A pointer to where this function will return the address of the sender
*          of the received data. Set this to NULL if the address is not required.
*   \param sdwFromAddrlen Points to a value specifying the maximum size in bytes of the storage
*          area pointed to by pFromAddr.
*   \return If successful the number of bytes of data stored in *pBuff is returned; If the 
*           socket has been closed by the sender, 0 is returned. On failure X_SOCKET_ERROR is returned.
*           errno() can be used to obtain additional info.
*           \arg \c ENOTINITIALIZED The socket API has not been initialized
*           \arg \c EBADF The socket descriptor s is invalid
*           \arg \c ENOTCONN The socket is not connected
*           \arg \c ECONNRESET The connection has been reset
*           \arg \c EINVAL One of the following conditions:
*           len<0, error during processing out-of-band data, sdwFromAddrlen too small.
*           \arg \c EWOULDBLOCK The socket is in non-blocking mode and no data is available
*/ 
s32     x_recvfrom ( X_SOCKET s, XCHAR *pBuff, s32 sdwBuffLen, s32 sdwFlags, struct X_SOCKADDR *pFromAddr, X_SOCKLEN *sdwFromAddrlen ) ;



/**
*   \fn s32 x_send ( X_SOCKET s, const XCHAR *pBuff, s32 sdwBuffLen, s32 sdwFlags )
*   \brief Sends data to a connected socket
*   \param s Socket descriptor identifying the socket that data should be sent on.
*   \param pBuff Points to the buffer containing the data to be sent.
*   \param sdwBuffLen The amount of data contained in pBuff in bytes.
*   \param sdwFlags Specifies how the transmit process should operate 
*           \arg \c MSG_OOB Send out-of-band data
*           \arg \c MSG_DONTROUTE Do not use routing
*           \arg \c MSG_DONTWAIT Send message in a non-blocking manner 
*   \return If successful the number of bytes of data that were sent is returned, else
*           an error status X_SOCKET_ERROR is returned. errno() can be used to obtain additional info.
*           \arg \c ENOTINITIALIZED The socket API has not been initialized
*           \arg \c EBADF The socket descriptor s is invalid
*           \arg \c ENOTCONN The socket is not connected
*           \arg \c ECONNRESET The connection has been reset
*           \arg \c EPIPE Cannot send any more out of socket s
*           \arg \c EINVAL One of the following conditions: len<0,pBuff==NULL,len is invalid.
*           \arg \c EWOULDBLOCK The socket is in non-blocking mode and would need to 
*                   block to complete the operation
*           \arg \c EMSGSIZE The message is larger than the maximum allowed
*/ 
s32     x_send ( X_SOCKET s, const XCHAR *pBuff, s32 sdwBuffLen, s32 sdwFlags ) ;



/**
*   \fn s32 x_sendto ( X_SOCKET s, const XCHAR *pBuff, s32 sdwBuffLen, s32 sdwFlags, const struct X_SOCKADDR *pToAddr, s32 sdwAddrLen )
*   \brief Sends data to a socket.
*   \param s Socket descriptor identifying the socket that data should be sent on.
*   \param pBuff Input: Points to the buffer containing the data to send. Output: None.
*   \param sdwBuffLen Input: The amount of data contained in *buf in bytes. Output: None.
*   \param sdwFlags Input: Specifies how the transmit process should operate
*           \arg \c MSG_OOB Send out-of-band data
*           \arg \c MSG_DONTROUTE Do not use routing
*           \arg \c MSG_DONTWAIT Send message in a non-blocking manner
*   \param pToAddr A pointer to the address to which the data should be sent, else NULL. 
*   \param sdwToAddrLen The size in bytes of the address contained in pToAddr, else NULL. 
*   \return If successful the number of bytes sent is returned, else an error status X_SOCKET_ERROR is returned;
*           errno() can be used to obtain additional info.
*           \arg \c ENOTINITIALIZED The socket API has not been initialized
*           \arg \c EBADF The socket descriptor s is invalid
*           \arg \c ENOTCONN The socket is not connected
*           \arg \c ECONNRESET The connection has been reset
*           \arg \c EPIPE Cannot send any more data out of socket s
*           \arg \c EINVAL One of the following conditions:len<0,len is invalid,sdwToAddrLen is invalid address.
*           \arg \c EWOULDBLOCK The socket is in non-blocking mode and would have to block in order to complete the operation
*           \arg \c EMSGSIZE The message is larger than the maximum supported by the protocol
*           \arg \c ENOBUFS Memory is not available to complete the request
*           \arg \c EDESTADDRREQ A destination address is required but is not available
*/ 
s32     x_sendto ( X_SOCKET s, const XCHAR *pBuff, s32 sdwBuffLen, s32 sdwFlags, const struct X_SOCKADDR *pToAddr, s32 sdwToAddrLen ) ;



/**
*   \fn u32 x_inet_addr ( const XCHAR* pszIPAddress )
*   \brief Converts a string to an X_INADDR structure
*   \param pszIPAddress NULL terminated string containing address to convert
*/ 
u32     x_inet_addr ( const XCHAR* pszIPAddress );



/**
*   \fn XCHAR * x_inet_ntoa ( struct X_INADDR hostAddress )
*   \brief Converts a network address to a string.
*   \param hostAddress Address to convert
*   \return String on success, else NULL  
*/ 
XCHAR * x_inet_ntoa ( struct X_INADDR hostAddress );



/**
*   \fn u32 x_htonl ( u32  hostlong )
*   \brief  Convert a long from host to network endian form.
*/ 
u32     x_htonl ( u32 dwHost );



/**
*   \fn u16 x_htons ( u16 hostshort )
*   \brief Convert a short from host to network endian form.
*/ 
u16     x_htons ( u16 wHost );



/**
*   \fn u32 x_ntohl ( u32 netlong )
*   \brief Convert a long from network to host endian form.
*/ 
u32     x_ntohl ( u32 netlong );



/**
*   \fn u16 x_ntohs ( u16 netshort )
*   \brief Convert a short from network to host endian form.
*/ 
u16 x_ntohs ( u16 netshort );



//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@//
// FUNCTIONS ARE NOT BE SUPPORTED BY ALL PLATFORMS;                      //
// FUNCTIONS THAT DO NOT SUPPORT ALL OPTIONS ON PARTICULAR PLATFORMS;    //
// USE CAUTION WHEN USING IN CLIENT APPLICATIONS                         //
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@//



/**
*  \fn s32 x_gethostname( XCHAR *name, s32 namelen )
*  \brief <TODO :: add brief description>
*  
* \b Additional-Info If needed add more detailed information here.
*/ 
s32 x_gethostname( XCHAR *name, s32 namelen );


/**
*   \fn s32 x_setsockopt ( X_SOCKET s, s32 sdwLevel, s32 sdwOptName, const void *pOptVal, s32 sdwOptLen )
*   \brief Sets a particular socket option.
*   \param s Socket descriptor identifying the socket for which the option should be set
*   \param level Input: Indicates the protocol level for which the socket option should be set,
*   only SOL_SOCKET is supported.
*   \param sdwOptName Specifies the option for which is to be set 
*   \param pOptVal Points to where the option information to be setup is contained.
*   \param sdwOptLen The size in bytes of the option information pointed to by optval.
*   \return (0)If successful, else an error status X_SOCKET_ERROR is returned. errno() can be used
*           to obtain additional info.
*       \arg \c ENOTINITIALIZED The socket API has not been initialized
*       \arg \c EBADF The socket descriptor s is invalid
*       \arg \c ENOPROTOOPT The option is not valid for the level specified
*       \arg \c EINVAL One of the following conditions: pOptVal==NULL, optlen too small.
*   \warning Not all options are supported by all platforms. Do not use in client applications.
*/ 
s32 x_setsockopt ( X_SOCKET s, s32 sdwLevel, s32 sdwOptName, const void *pOptVal, s32 sdwOptLen ) ;



/**
*   \fn s32 x_getsockopt ( X_SOCKET s, s32 level, s32 optname, XCHAR *optval, s32 *optlen )
*   \brief Get a particular socket option.
*   \param s Socket descriptor of socket for which option information is required
*   \param level Indicates the protocol level for which socket information is required,
*    only SOL_SOCKET is supported
*   \param optname Specifies the option for which information is to be obtained
*   \param optval Input: Points to where the option information will be returned by this
*          function. Output: *optval contains the option information.
*   \param optlen Input: Points to a value specifying the maximum size in bytes of the storage
*          area pointed to by optval. Output: *optlen contains the actual length of
*          the returned option information in bytes. \b Additional-Info If needed add more detailed information here.
*   \warning Not all options are supported by all platforms. Do not use in client applications.
*/ 
s32     x_getsockopt ( X_SOCKET s, s32 level, s32 optname, XCHAR *optval, s32 *optlen ) ;



/**
*   \fn s32 x_ioctl ( X_SOCKET s, u32 cmd, u32 *argp )
*   \brief Controls the I/O mode of a socket.
*   \param s Descriptor identifying a socket.
*   \param cmd Command to perform on the socket s. 
*   \param argp [in/out] Pointer to a parameter for cmd. *  
*   \return (0)If successful, else an error status X_SOCKET_ERROR is returned. errno() can be used
*           to obtain additional info.
*   \warning Not all options are supported by all platforms. Do not use in client applications.
*/ 
s32 x_ioctl ( X_SOCKET s, u32 cmd, u32 *argp );



/**
*   \fn s32 x_select ( s32 nfds, X_FD_SET *readfds,  X_FD_SET *writefds, X_FD_SET *exceptfds, struct  x_timeval *timeout )
*   \brief Selects sockets ready for receive or send.
*   \param nfds Ignored. The nfds parameter is included only for compatibility with Berkeley sockets. 
*   \param timeout Maximum time for select to wait, provided in the form of a TIMEVAL structure.
*                  Set the timeout parameter to NULL for blocking operation. 
*   \param readfds Optional pointer to a set of sockets to be checked for readability. 
*   \param writefds Optional pointer to a set of sockets to be checked for writability 
*   \param exceptfds Optional pointer to a set of sockets to be checked for errors. 
*   \return If successful, returns the total number of sockets identified in the returned
*           descriptor sets. On failure an error status X_SOCKET_ERROR is returned.
*   \warning Not all options are supported by all platforms. Do not use in client applications.
*/
s32     x_select ( s32 nfds, fd_set  *readfds, fd_set  *writefds, fd_set  *exceptfds, struct x_timeval *timeout );



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#if defined(TARGET_WIN32) || defined ( TARGET_SDC_WINCE) || defined(TARGET_XBOX)  
#define x_socket(s,p,z)      socket(s,p,z)
#define x_close(s)           closesocket(s)
#define x_shutdown(s,f)      shutdown(s,f)
#define x_bind(s,a,l)        bind(s,a,l)
#define x_accept(s,a,l)      accept(s,a,l)
#define x_listen(s,b)        listen(s,b)
#define x_ioctl(s,c,a)       ioctlsocket(s,c,a)
#define x_connect(s,a,l)     connect(s,a,l)
#define x_inet_addr(s)       inet_addr(s)
#define x_send(s,b,l,f)      send(s,b,l,f)
#define x_recv(s,b,l,f)      recv(s,b,l,f)
#define x_recvfrom(s,b,l,f,a,d) recvfrom(s,b,l,f,a,d)
#define x_select(n,r,w,e,t)  select(n,r,w,e,t)
#define x_sendto(s,b,l,f,a,d)   sendto(s,b,l,f,a,d)
#define x_ntohs(n)				ntohs(n)
#define x_ntohl(n)				ntohl(n)
#define x_htons(n)				htons(n)
#define x_htonl(n)				htonl(n)
#define x_getsockopt(s, l, on, ov, ol) getsockopt(s, l, on, ov, ((int*)ol))
#define x_setsockopt(s, l, on, ov, ol) setsockopt(s, l, on, ov, ol)


#if defined (TARGET_WIN32) || defined ( TARGET_SDC_WINCE) //xbox doesn't support these routines
#define x_inet_ntoa(h)   inet_ntoa(h)
#define x_gethostname    gethostname
#define x_getsockname    getsockname
#define x_gethostbyname(n) gethostbyname(n)
#define x_gethostbyaddr(x,y,z) gethostbyaddr(x,y,z)
#endif

#elif defined(TARGET_PS2)
/*
// Old calls for INET - Insock stuff
#define x_socket(s,p,z)          sceInsockSocket(s,p,z)
#define x_close(s)               x_shutdown(s,0)
#define x_bind(s,a,l)            sceInsockBind(s,a,l)
#define x_accept(s,a,l)          sceInsockAccept(s,a,l)
#define x_listen(s,b)            sceInsockListen(s,b)
#define x_ioctl(s,c,a)           sceInsockIoctl(s,c,a)
#define x_connect(s,a,l)         sceInsockConnect(s,a,l)
#define x_send(s,b,l,f)          sceInsockSend(s,b,l,f)
#define x_sendto(s,b,l,f,a,d)    sceInsockSendto(s,b,l,f,a,d)

#define x_ntohs(n)               sceInsockNtohs(n)
#define x_ntohl(n)               sceInsockNtohl(n)
#define x_htons(n)               sceInsockHtons(n)
#define x_htonl(n)               sceInsockHtonl(n)
#define x_inet_addr(s)           sceInsockInetAddr(s)
#define x_inet_ntoa(n)      	 sceInsockInetNtoa(n)
#define x_getsockname(s,t,v)     sceInsockGetsockname(s,t,v)
#define x_gethostbyname(n)       sceInsockGethostbyname(n)
#define x_gethostbyaddr(x,y,z)   sceInsockGethostbyaddr(x,y,z)

//#define x_getsockopt(s, l, on, ov, ol) sceInsockGetsockopt(s, l, on, ov, ((int*)ol))
#define x_setsockopt(s, l, on, ov, l1) sceInsockSetsockopt(s, l, on, ov, l1) 

void no_select_for_ps2_clients( void );
#define x_select ( n, r, w, e, t ) no_select_for_ps2_clients(0)
*/

#define x_socket(s,p,z)          sceEENetSocket(s,p,z)
#define x_close(s)               sceEENetClose(s) //x_shutdown(s,0)
#define x_bind(s,a,l)            sceEENetBind(s,a,l)
#define x_accept(s,a,l)          sceEENetAccept(s,a,l)
#define x_listen(s,b)            sceEENetListen(s,b)
#define x_ioctl(s,c,a)           sceEENetIoctlSocket(s,c,a)
#define x_connect(s,a,l)         sceEENetConnect(s,a,l)
#define x_send(s,b,l,f)          sceEENetSend(s,b,l,f)
#define x_sendto(s,b,l,f,a,d)    sceEENetSendto(s,b,l,f,a,d)

#define x_shutdown(s,f)			 sceEENetShutdown(s,f)
#define x_recv(s,b,l,f)			 sceEENetRecv(s,b,l,f)
#define x_recvfrom(s,b,l,f,a,d)  sceEENetRecvfrom(s,b,l,f,a,d)
#define x_select(n,r,w,e,t)      sceEENetSelect(n,r,w,e,t)

#define x_ntohs(n)               sceEENetNtohs(n)
#define x_ntohl(n)               sceEENetNtohl(n)
#define x_htons(n)               sceEENetHtons(n)
#define x_htonl(n)               sceEENetHtonl(n)
#define x_inet_addr(s)           sceEENetInetAddr(s)
#define x_inet_ntoa(n)      	 sceEENetInetNtoa(n)
#define x_getsockname(s,t,v)     sceEENetGetsockname(s,t,v)
#define x_gethostbyname(n)       sceEENetGethostbyname(n)
#define x_gethostbyaddr(x,y,z)   sceEENetGethostbyaddr(x,y,z)

//#define x_getsockopt(s, l, on, ov, ol) sceInsockGetsockopt(s, l, on, ov, ((int*)ol))
#define x_getsockopt(s, l, on, ov, ol) sceEENetGetsockopt(s, l, on, ov, ((int*)ol))
#define x_setsockopt(s, l, on, ov, l1) sceEENetSetsockopt(s, l, on, ov, l1) 

//void no_select_for_ps2_clients( void );
//#define x_select ( n, r, w, e, t ) no_select_for_ps2_clients(0)


#elif defined(TARGET_DOLPHIN)

// FORWARD DECLARATIONS To be removed
X_SOCKET _x_socket ( s32 , s32 , s32 );
s32 _x_close ( X_SOCKET );
s32 _x_shutdown ( X_SOCKET , s32 );
s32 _x_bind ( X_SOCKET , const struct X_SOCKADDR *, s32 );
X_SOCKET _x_accept ( X_SOCKET , struct X_SOCKADDR *, X_SOCKLEN * );
s32 _x_listen ( X_SOCKET , s32 );
s32 _x_connect ( X_SOCKET , const struct X_SOCKADDR *, s32  ); 
s32 _x_send ( X_SOCKET , const XCHAR *pBuff, s32 , s32 );
s32 _x_recv ( X_SOCKET , XCHAR *, s32 , s32 );
s32 _x_sendto ( X_SOCKET , const XCHAR *, s32 , s32 , const struct X_SOCKADDR *, s32 );
s32 _x_recvfrom ( X_SOCKET , XCHAR *, s32 , s32 , struct X_SOCKADDR *, X_SOCKLEN * );
u16 _x_htons ( u16 );
u32 _x_htonl ( u32 );
u16 _x_ntohs ( u16 );
u32 _x_ntohl ( u32 );
u32 _x_inet_addr ( const XCHAR* );
XCHAR * _x_inet_ntoa ( struct X_INADDR );
s32 _x_ioctl ( X_SOCKET , u32 , u32 * );
s32 _x_gethostname( XCHAR *, s32 );
struct X_HOSTENT * _x_gethostbyaddr(  const XCHAR *, s32 , s32 );
s32 _x_getsockopt ( X_SOCKET , s32 level, s32 optname, XCHAR *, s32 * );
s32 _x_setsockopt ( X_SOCKET , s32 sdwLevel, s32 sdwOptName, const void *, s32 );
s32 _x_select ( s32 , fd_set  *, fd_set  *, fd_set  *, struct x_timeval * );

// defines To be substituted with Nintendo functions
#define x_socket(s,p,z)          _x_socket(s,p,z)        
#define x_close(s)               _x_close(s)

#define x_shutdown(s,f)			_x_shutdown(s,f)
             
#define x_bind(s,a,l)            _x_bind(s,a,l)          
#define x_accept(s,a,l)          _x_accept(s,a,l)        
#define x_listen(s,b)            _x_listen(s,b)          
#define x_ioctl(s,c,a)           _x_ioctl(s,c,a)         
#define x_connect(s,a,l)         _x_connect(s,a,l)       
#define x_send(s,b,l,f)          _x_send(s,b,l,f)        
#define x_recv(s,b,l,f)          _x_recv(s,b,l,f)        
#define x_sendto(s,b,l,f,a,d)    _x_sendto(s,b,l,f,a,d)  
#define x_recvfrom(s,b,l,f,a,d)  _x_recvfrom(s,b,l,f,a,d)

#define x_ntohs(n)               _x_ntohs(n)             
#define x_ntohl(n)               _x_ntohl(n)             
#define x_htons(n)               _x_htons(n)             
#define x_htonl(n)               _x_htonl(n)             
#define x_inet_addr(s)           _x_inet_addr(s)         
#define x_inet_ntoa(n)      	 _x_inet_ntoa(n)      
#define x_getsockname(s,t,v)     _x_getsockname(s,t,v)   
#define x_gethostbyname(n)       _x_gethostbyname(n)     
#define x_gethostbyaddr(x,y,z)   _x_gethostbyaddr(x,y,z) 

#define x_getsockopt(s, l, on, ov, ol) _x_getsockopt(s, l, on, ov, ((s32*)ol))
#define x_setsockopt(s, l, on, ov, l1) _x_setsockopt(s, l, on, ov, l1) 
#define x_select ( n, r, w, e, t )     _x_select(n, r, w, e, t)

#else

#error "Target doesn't support sockets"

#endif
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
    }
#endif
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
#endif //__X_SOCK_H__
//////////////////////////////////////////////////////////////////////////