/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
*		Filename:	CXSocket.cpp
*		Date:		7/24/2001
*		Desc:		<todo>
*		Revisions History:
\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

#include "CXSocket.h"
#include "x_files.hpp"



CXSocket::CXSocket()
{
	m_hSocket = X_INVALID_SOCKET;
}
CXSocket::~CXSocket()
{
	Close();//close socket if not close
	m_hSocket = X_INVALID_SOCKET;
}
bool CXSocket::Create(s32 nSocketType)
{
#ifdef TARGET_XBOX
	m_hSocket = x_socket(AF_INET, nSocketType, IPPROTO_VDP);
#endif
#ifdef TARGET_PS2
	m_hSocket = x_socket(AF_INET, nSocketType, 0);
#endif
	
	if(X_INVALID_SOCKET == m_hSocket)
		return false;
	return true;
}
bool CXSocket::Create(s32 nSocketType, char *pszSocketAddress, unsigned short nSocketPort)
{
	X_SOCKADDRIN sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = x_htons(nSocketPort);
	if(pszSocketAddress == NULL)
		sockaddr.sin_addr.s_addr = x_inet_addr("");
	else
		sockaddr.sin_addr.s_addr = x_inet_addr(pszSocketAddress);

	return Create(nSocketType, &sockaddr);
}
bool CXSocket::Create(s32 nSocketType, X_SOCKADDRIN *psockaddr)
{
	if(Create(nSocketType))
		return Bind(psockaddr, sizeof(X_SOCKADDRIN));
	return false;	
}

void CXSocket::Attach( X_SOCKET hSocket)
{
	//may be we should check if it's already a valid socket
	m_hSocket = hSocket;
}
X_SOCKET CXSocket::Detach(void)
{
	X_SOCKET temp = m_hSocket;
	m_hSocket = X_INVALID_SOCKET;
	return temp;
}
s32 CXSocket::GetLastError(void)
{
	return X_ERRNO;
}
/*
s32 CXSocket::GetPeerName( char *pszPeerAddress, unsigned short *pPeerPort )
{
	X_SOCKETADDR sockaddr;
	s32 addrlen = sizeof(sockaddr);
	s32 ret;
	ret =x_getpeername(m_hSocket, (struct sockaddr*)&sockaddr, &addrlen);
	if(0 == ret)
	{
		*pPeerPort = x_htons(sockaddr.sin_port);
		x_strcpy(pszPeerAddress, x_inet_ntoa(sockaddr.sin_addr));
	}
	return ret;
}
s32 CXSocket::GetPeerName( X_SOCKETADDR* pSockAddr, s32* pSockAddrLen )
{
	if(x_getpeername(m_hSocket, (struct sockaddr*)pSockAddr, pSockAddrLen))
		return SetLastError(WSAGetLastError());
	return (0);
}
*/
/*
s32 CXSocket::GetSockName( char *pszSocketAddress, unsigned short *pSocketPort )
{
	X_SOCKADDRIN sockaddr;
	int addrlen = sizeof(sockaddr);

	s32 ret;
	ret = GetSockName(&sockaddr, &addrlen);
	if(0 == ret)
	{
		*pSocketPort = x_htons(sockaddr.sin_port);
		x_strcpy(pszSocketAddress, x_inet_ntoa(sockaddr.sin_addr));
	}
	return ret;	
}
s32 CXSocket::GetSockName( X_SOCKADDRIN* pSockAddr, int* pSockAddrLen )
{
	return x_getsockname(m_hSocket, (struct sockaddr*)pSockAddr, pSockAddrLen);
}
*/
//s32 CXSocket::GetSockOpt( s32 nOptionName, void *pOptionValue, s32 * pOptionLen, s32 nLevel)
//{
//	return x_getsockopt(m_hSocket, nLevel, nOptionName, (char*)pOptionValue, pOptionLen );
//}
s32 CXSocket::SetSockOpt( s32 nOptionName, const void *pOptionValue, s32 nOptionLen, s32 nLevel)
{
	return x_setsockopt(m_hSocket, nLevel, nOptionName, (char*)pOptionValue, nOptionLen);
}
s32 CXSocket::SetToNoBlocking(void)
{
	return XSOCK_SetBlockingMode(m_hSocket, X_SET_SOCKET_NONBLOCKING);
}
s32 CXSocket::SetToBlocking(void)
{
	return XSOCK_SetBlockingMode(m_hSocket, X_SET_SOCKET_BLOCKING);
}

s32 CXSocket::SetNoLinger(void)
{
#ifdef TARGET_WIN32
	struct linger	temp;
	temp.l_linger = 0;
	temp.l_onoff = 1;
	return SetSockOpt(SO_LINGER, &temp, sizeof(struct linger) ,SOL_SOCKET);
#else
    return X_SOCKET_ERROR;
#endif
}

bool CXSocket::Accept( CXSocket &rNewSocket, X_SOCKADDRIN *pSockAddr, X_SOCKLEN* pSockAddrLen )
{
	X_SOCKET temp;
	temp = x_accept(m_hSocket, (X_SOCKADDR*)pSockAddr, pSockAddrLen);
	if(X_INVALID_SOCKET == temp)
		return false;

	rNewSocket.Attach(temp);
	return true;
}

bool CXSocket::Bind(const char *pszIP,  u16 nSocketPort)
{
	X_SOCKADDRIN sockaddr;	
	sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = x_htons(nSocketPort);    
	if(pszIP == NULL)
		sockaddr.sin_addr.s_addr = x_inet_addr("");
	else
		sockaddr.sin_addr.s_addr = x_inet_addr(pszIP);

	return Bind(&sockaddr, sizeof(sockaddr));
}
bool CXSocket::Bind( const X_SOCKADDRIN* pSockAddr, s32 nSockAddrLen )
{
	if(x_bind(m_hSocket, (X_SOCKADDR*)pSockAddr, nSockAddrLen))
		return false;
	return true;
}

bool CXSocket::Close( )
{	
	if(m_hSocket == X_INVALID_SOCKET)
		return true;
	
	x_close(m_hSocket);		
	m_hSocket = X_INVALID_SOCKET;
	return true;
}
bool CXSocket::ShutDown( s32 nHow)
{
	if(x_shutdown(m_hSocket, nHow))
		return false;
	return true;
}


bool CXSocket::Connect(const char *pszIP, unsigned short nHostPort )
{
	X_SOCKADDRIN sockaddr;
	sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = x_htons(nHostPort);
	if(pszIP == NULL)
		sockaddr.sin_addr.s_addr = x_inet_addr("");
	else
		sockaddr.sin_addr.s_addr = x_inet_addr(pszIP);

	return Connect(&sockaddr, sizeof(sockaddr));
}
bool CXSocket::Connect( const X_SOCKADDRIN* pSockAddr, s32 nSockAddrLen )
{
	if(x_connect(m_hSocket, (X_SOCKADDR*)pSockAddr, nSockAddrLen))
		return false;
	return true;
}
bool CXSocket::Listen( s32 nCxnBacklog)
{
	if(x_listen( m_hSocket, nCxnBacklog))
		return false;
	return true;
}

s32 CXSocket::IOCtl( s32 cmd, u32* pArgument )
{
#ifndef TARGET_PS2
	return x_ioctl(m_hSocket, cmd, pArgument);
#else
    ASSERT(!"Not supported");
    //EXIT_FAILURE
    return ( -1 );
#endif
}

s32 CXSocket::Receive( void *pBuf, s32 nBufLen, s32 nFlags)
{
	return x_recv(m_hSocket, (char*)pBuf,  nBufLen, nFlags);
}

s32 CXSocket::ReceiveFrom( void *pBuf, s32 nBufLen, char *pszIP, u16 *pSocketPort, s32 nFlags)
{
	X_SOCKADDRIN sockaddr;
	X_SOCKLEN addrlen = SIZEOF_SOCKADDR_IN;
	s32 ret;
	ret = x_recvfrom(m_hSocket, (char*)pBuf, nBufLen, nFlags, (struct X_SOCKADDR*)&sockaddr,  &addrlen);
	if(X_SOCKET_ERROR == ret)
		return ret;

	//copy source address (IP & port)
	*pSocketPort = x_htons(sockaddr.sin_port);
	x_strcpy(pszIP, x_inet_ntoa(sockaddr.sin_addr));

	return ret;
}
s32 CXSocket::ReceiveFrom( void *pBuf, s32 nBufLen, const X_SOCKADDRIN *pSockAddr, s32 *pSockAddrLen, s32 nFlags)
{
	return x_recvfrom(m_hSocket, (char*)pBuf, nBufLen, nFlags, (X_SOCKADDR*)pSockAddr,  (X_SOCKLEN*)pSockAddrLen);
}

s32 CXSocket::Send( const void *pBuf, s32 nBufLen, s32 nFlags )
{
	return x_send(m_hSocket, (char*)pBuf, nBufLen, nFlags);	
}

s32 CXSocket::SendTo( const void *pBuf, s32 nBufLen, const char *pszIP, unsigned short nHostPort, s32 nFlags)
{
	X_SOCKADDRIN sockaddr;
	sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = x_htons(nHostPort);    
    sockaddr.sin_addr.s_addr = x_inet_addr(pszIP);

	return x_sendto(m_hSocket, (char*)pBuf,nBufLen, nFlags, (struct X_SOCKADDR*)&sockaddr, sizeof(sockaddr));	
}

s32 CXSocket::SendTo( const void *pBuf, s32 nBufLen, const X_SOCKADDRIN *pSockAddr, s32 nSockAddrLen, s32 nFlags )
{
	return x_sendto(m_hSocket, (char*)pBuf,nBufLen, nFlags,  (X_SOCKADDR*)pSockAddr, nSockAddrLen);	
}
