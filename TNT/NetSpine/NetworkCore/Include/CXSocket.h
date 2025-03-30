/**
* \file     CXSocket.h
* \brief    
* \author   NetSpine Author
* \version  1.0
* \date		8/8/2001
*/


#ifndef __CXSOCKET_H__
#define __CXSOCKET_H__


#include "x_types.hpp"
#include "x_sock.h"

/**
*   \class CXSocket
*   \brief A light weight Socket class wrapper
*   \author Zhi Chen
*   \version 1.0
*   \date 8/8/2001
*/
class CXSocket
{
public:
		//construction
	CXSocket();
	virtual ~CXSocket();	
	/**
	*create a socket with specified Socket type. Must call Bind() explicitly
	*@param nSocketType Either SOCK_STREAM or SOCK_DGRAM
	*@return bool
	*		@arg \c true  for success 
	*		@arg \c false for failed 
	* @see GetLastError()
	*/
	bool Create(s32 nSocketType);

	//@{
	/**
	* Create() method overload also call Bind() internally
	* @see Bind()
	* @see GetLastError()
	*/
	bool Create(s32 nSocketType, char *pszSocketAddress, u16 nSocketPort);
	bool Create(s32 nSocketType, X_SOCKADDRIN *psockaddr);
	//@}
public:
	/**Attach a X_SOCKET to CXSocket object, application has to ensure CXSocket object is empty - didn't call Create()*/
	void Attach( X_SOCKET hSocket);
	/**Detach a X_SOCKET from this CXSocket object. This object will become invalid*/
	X_SOCKET Detach(void);
	/**Returns a copy of this object's internal socket*/
	X_SOCKET GetSocket(void){return m_hSocket;}
	/**Return the last error*/
	s32 GetLastError(void);


	
//	s32 GetPeerName( char *pszPeerAddress, u16 *pPeerPort );
//	s32 GetPeerName( X_SOCKETADDR* lpSockAddr, s32* lpSockAddrLen );
	//zero for success, otherwise call GetLastError() for error
//	s32 GetSockName( char *pszSocketAddress, u16 *pSocketPort );
//	s32 GetSockName( X_SOCKADDRIN* pSockAddr, int* pSockAddrLen );

	
	//@{
	/**
	* @return s32
	*		@arg \c (0) if successful
	*		@arg \c Otherwise failed
	* @see GetLastError()
	*/
	s32 IOCtl( s32 cmd, u32* pArgument );
//	s32 GetSockOpt( s32 nOptionName, void *pOptionValue, s32* pOptionLen, s32 nLevel = SOL_SOCKET );
	s32 SetSockOpt( s32 nOptionName, const void *pOptionValue, s32 nOptionLen, s32 nLevel = SOL_SOCKET );
	s32 SetToNoBlocking(void);
	s32 SetToBlocking(void);
	s32 SetNoLinger(void);
	//@}
public:
	/**
	*@param &rConnectedSocket a "uninitialized CXSocket" object to receive new connection
	*@param *pSockAddr A X_SOCKETADDR object to receive remote address.
	*@param *pSockAddrLen length of the pSockAddr object.
	*@return bool
	*		@arg \c true  A new socket if assigned to rConnectedSocket object
	*		@arg \c false for failed 	
	*@see GetLastError()
	*/
	bool Accept( CXSocket &rNewSocket, X_SOCKADDRIN *pSockAddr = NULL, X_SOCKLEN* pSockAddrLen = NULL );

	//@{
	/**
	*@return bool
	*		@arg \c true  for success 
	*		@arg \c false for failed 
	*@see GetLastError()
	*/
	bool Bind(const char *pszIP = NULL,  u16 nSocketPort = 0);
	bool Bind( const X_SOCKADDRIN* pSockAddr, s32 nSockAddrLen );
	bool Connect(const char *pszIP, u16 nHostPort );
	bool Connect( const X_SOCKADDRIN* pSockAddr, s32 nSockAddrLen );
	bool Listen( s32 nCxnBacklog = 5 );
	bool Close( void );
	bool ShutDown( s32 nHow );
	//@}

	//@{
	/**
	*@return s32
	*		@arg \c length of pBuf been received
	*		@arg \c X_SOCKET_ERROR if error occur
	*@see GetLastError()
	*/
	s32 Receive( void *pBuf, s32 nBufLen, s32 nFlags = 0 );
	s32 ReceiveFrom( void *pBuf, s32 nBufLen, const X_SOCKADDRIN *pSockAddr, s32 *pSockAddrLen, s32 nFlags = 0 );
	s32 ReceiveFrom( void *pBuf, s32 nBufLen, char *pszIP, u16 *pSocketPort, s32 nFlags = 0 );
	//@}

	//@{
	/**
	*@return s32
	*		@arg \c length of pBuf been sent
	*		@arg \c X_SOCKET_ERROR if error occur
	*@see GetLastError()
	*/
	s32 Send( const void *pBuf, s32 nBufLen, s32 nFlags = 0 );
	s32 SendTo( const void *pBuf, s32 nBufLen, const char *pszIP, u16 nHostPort, s32 nFlags = 0 );
	s32 SendTo( const void *pBuf, s32 nBufLen, const X_SOCKADDRIN *pSockAddr, s32 nSockAddrLen, s32 nFlags = 0 );
	//@}

protected:
	X_SOCKET m_hSocket;
};



#endif //__CXSOCKET_H__
