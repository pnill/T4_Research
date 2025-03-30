/******
nonport.h
GameSpy Common Code
  
Copyright 1999-2002 GameSpy Industries, Inc

18002 Skypark Circle
Irvine, California 92614
949.798.4200 (Tel)
949.798.4299 (Fax)
devsupport@gamespy.com
******/

#ifndef _NONPORT_H_
#define _NONPORT_H_


#ifdef __cplusplus
extern "C++" {
#include "x_files.hpp"
}
#endif

#ifdef X_DEBUG
	#define _DEBUG
#endif

#define EENET
#define UNIQUEID

#ifndef GSI_MEM_ONLY
#define GSI_UNUSED(x) x

#if defined(applec) || defined(THINK_C) || defined(__MWERKS__) && !defined(__KATANA__) && !defined(__mips64)
	#define _MACOS
#endif

#ifdef __mips64
	#if !defined(CISCO_NFT) && !defined(SN_SYSTEMS) && !defined(EENET)
		#define CISCO_NFT
	#endif
	#define _PS2
#endif

#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <winsock.h>
#else
#ifdef _MACOS
	#include <events.h>
	#include "mwinsock.h"
	#define GS_BIG_ENDIAN
#else
#ifdef __KATANA__
	#ifdef KGTRN_PLANET_WEB
		#ifdef __GNUC__
			#include <errno.h>
		#endif
		#include <pwebinc.h>
		#include <dbdebug.h>
		#include <netinet/tcp.h>
	#endif
	#ifdef KGTRN_ACCESS
		#include <types.h>
		#include <socket.h>
		#include <sock_errno.h>
		#include <aveppp.h>
		#include <vsyncpro.h>
		#include <dns.h>
	#endif
	#ifdef KGTRN_NEXGEN
		#include <ngos.h>
		#include <ngdsock.h>
		#include <ngappp.h>
		#include <ngadns.h>
		#include <ngnet.h>
		#include <ngsocket.h>
		#include <ngtcp.h>
	#endif
	#include <stdio.h>
	#include <stddef.h>
	#include <shinobi.h>
	#include <sg_sytmr.h>
#else
#ifdef _PS2
	#define GS_BIG_ENDIAN
	#include <sys/types.h>
	#include <eekernel.h>
	#include <stdio.h>
	#include <sifdev.h>
	#include <sifrpc.h>
	#include <sifcmd.h>
	#include <ilink.h>
	#include <ilsock.h>
	#include <ilsocksf.h>
	#ifdef SN_SYSTEMS	
		#include "sntypes.h"
		#include "snsocket.h"
		#include "sneeutil.h"
		#include "sntcutil.h"
	#endif
	#ifdef CISCO_NFT
		#include <nft/libnft.h>
		#include <nft/unistd.h>
		#include <nft/sys/types.h>
		#include <nft/sys/socket.h>
		#include <nft/sys/types.h>
		#include <nft/sys/socket.h>
		#include <nft/sys/ioctl.h>
		#include <nft/netinet/in.h>
		#include <nft/netdb.h>
		#include <nft/arpa/inet.h>
		#include <nft/errno.h>
		#include <nft/sys/time.h>
	#endif
	#ifdef EENET
		#include <libeenet.h>
		#include <eenetctl.h>
		#include <ifaddrs.h>
		#include <sys/socket.h>
		#include <sys/errno.h>
		#include <netinet/in.h>
		#include <arpa/inet.h>
		#include <net/if.h>
		#ifndef EENET_260
		#include <sys/select.h>
		#include <malloc.h>
		#endif
	#endif
#else //UNIX
	#define UNDER_UNIX
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <stdio.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/ioctl.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <arpa/inet.h>
	#include <ctype.h>
	#include <errno.h>
	#include <sys/time.h>
	#include <limits.h>
	//#include <sys/syslimits.h>
	#include <netinet/tcp.h>
#endif
#endif
#endif
#endif 

#undef NULL
#define NULL 0

#ifdef UNDER_CE
#include <platutil.h>
#endif

#if !defined(UNDER_CE) && !defined(__KATANA__)
#include <assert.h>
#else
#define assert(a)
#endif

#endif

#define gsimalloc x_malloc
#define gsifree x_free

#ifdef __KATANA__
void *fixed_realloc(void *ptr, int newsize);
#define gsirealloc fixed_realloc
#else
#define gsirealloc x_realloc
#endif

#ifndef GSI_MEM_ONLY

typedef int goa_int32;
typedef unsigned int goa_uint32;

#ifdef __cplusplus
extern "C" {
#endif

unsigned long current_time();
void msleep(unsigned long msec);

void SocketStartUp();
void SocketShutDown();

#ifndef SOCKET_ERROR 
	#define SOCKET_ERROR (-1)
#endif

#ifndef INADDR_NONE
   #define INADDR_NONE 0xffffffff
#endif

#ifndef INVALID_SOCKET 
	#define INVALID_SOCKET (-1)
#endif

#ifdef KGTRN_NEXGEN
	#define FD_SETSIZE NG_FD_MAX
	#define select selectsocket
#endif

#if defined(_WIN32) && !defined(UNDER_CE)
	#define strcasecmp _stricmp
	#define strncasecmp _strnicmp
#else	
	char *_strlwr(char *string);
	char *_strupr(char *string);
#endif

#undef strdup
#define strdup goastrdup
#undef _strdup
#define _strdup goastrdup



char * goastrdup(const char *src);

#if defined(_MACOS) || defined(UNDER_CE)
	int strcasecmp(const char *string1, const char *string2);
	int strncasecmp(const char *string1, const char *string2, size_t count);
#endif

#ifdef SN_SYSTEMS
	#define IPPROTO_TCP PF_INET
	#define IPPROTO_UDP PF_INET
	#define FD_SETSIZE SN_MAX_SOCKETS
#endif

#if !defined(_MACOS) && !defined(_WIN32)
	#define SOCKET int

	#ifdef KGTRN_NEXGEN
		#define GOAGetLastError(s) ngGlobalErrno
	#endif
	
	#ifdef KGTRN_ACCESS
		#define GOAGetLastError(s) sock_errno
	#endif
	
	#if defined (KGTRN_PLANET_WEB)
		#if defined(__GNUC__) && defined(__KATANA__)
			#define GOAGetLastError(s) (*__errno())	
			extern int *__errno _PARAMS ((void));
		#else
			#define GOAGetLastError(s) errno
		#endif
	#endif
	
	#ifdef SN_SYSTEMS
		int GOAGetLastError(SOCKET s);
	#endif

	#ifdef CISCO_NFT
		#define GOAGetLastError(s) errno
	#endif

	#ifdef EENET
		#define GOAGetLastError(s) sceEENetErrno
	#endif
	
	#ifdef UNDER_UNIX
		#define GOAGetLastError(s) errno
	#endif

	#ifdef KGTRN_ACCESS
		#define closesocket sock_close
	#endif
	
	#ifdef KGTRN_PLANET_WEB
		#define closesocket net_close
	#endif

	#ifdef CISCO_NFT
		#define closesocket nftclose
	#endif

	#ifdef EENET
		#define closesocket sceEENetClose
	#endif
	
	#ifdef UNDER_UNIX
		#define closesocket close //on unix
	#endif

	#ifdef CISCO_NFT
		#define ioctlsocket nftioctl
		#define FIONBIO UNTESTED_FIONBIO
	#else
		#define ioctlsocket ioctl
	#endif

	#ifdef KGTRN_NEXGEN
		#define WSAEWOULDBLOCK NG_EWOULDBLOCK
		#define WSAEINPROGRESS NG_EINPROGRESS
		#define WSAEALREADY NG_EALREADY
		#define WSAENOTSOCK NG_ENOTSOCK
		#define WSAEDESTADDRREQ NG_EDESTADDRREQ
		#define WSAEMSGSIZE NG_EMSGSIZE
		#define WSAEPROTOTYPE NG_EPROTOTYPE
		#define WSAENOPROTOOPT NG_ENOPROTOOPT
		#define WSAEPROTONOSUPPORT NG_EPROTONOSUPPORT
		#define WSAESOCKTNOSUPPORT NG_ESOCKTNOSUPPORT
		#define WSAEOPNOTSUPP NG_EOPNOTSUPP
		#define WSAEPFNOSUPPORT NG_EPFNOSUPPORT
		#define WSAEAFNOSUPPORT NG_EAFNOSUPPORT
		#define WSAEADDRINUSE NG_EADDRINUSE
		#define WSAEADDRNOTAVAIL NG_EADDRNOTAVAIL
		#define WSAENETDOWN NG_ENETDOWN
		#define WSAENETUNREACH NG_ENETUNREACH
		#define WSAENETRESET NG_ENETRESET
		#define WSAECONNABORTED NG_ECONNABORTED
		#define WSAECONNRESET NG_ECONNRESET
		#define WSAENOBUFS NG_ENOBUFS
		#define WSAEISCONN NG_EISCONN
		#define WSAENOTCONN NG_ENOTCONN
		#define WSAESHUTDOWN NG_ESHUTDOWN
		#define WSAETOOMANYREFS NG_ETOOMANYREFS
		#define WSAETIMEDOUT NG_ETIMEDOUT
		#define WSAECONNREFUSED NG_ECONNREFUSED
		#define WSAELOOP NG_ELOOP
		#define WSAENAMETOOLONG NG_ENAMETOOLONG
		#define WSAEHOSTDOWN NG_EHOSTDOWN
		#define WSAEHOSTUNREACH NG_EHOSTUNREACH
		#define WSAENOTEMPTY NG_ENOTEMPTY
		#define WSAEPROCLIM NG_EPROCLIM
		#define WSAEUSERS NG_EUSERS
		#define WSAEDQUOT NG_EDQUOT
		#define WSAESTALE NG_ESTALE
		#define WSAEREMOTE NG_EREMOTE
	#else
		#define WSAEWOULDBLOCK EWOULDBLOCK             
		#define WSAEINPROGRESS EINPROGRESS             
		#define WSAEALREADY EALREADY                
		#define WSAENOTSOCK ENOTSOCK                
		#define WSAEDESTADDRREQ EDESTADDRREQ            
		#define WSAEMSGSIZE EMSGSIZE                
		#define WSAEPROTOTYPE EPROTOTYPE              
		#define WSAENOPROTOOPT ENOPROTOOPT             
		#define WSAEPROTONOSUPPORT EPROTONOSUPPORT         
		#define WSAESOCKTNOSUPPORT ESOCKTNOSUPPORT         
		#define WSAEOPNOTSUPP EOPNOTSUPP              
		#define WSAEPFNOSUPPORT EPFNOSUPPORT            
		#define WSAEAFNOSUPPORT EAFNOSUPPORT            
		#define WSAEADDRINUSE EADDRINUSE              
		#define WSAEADDRNOTAVAIL EADDRNOTAVAIL           
		#define WSAENETDOWN ENETDOWN                
		#define WSAENETUNREACH ENETUNREACH             
		#define WSAENETRESET ENETRESET               
		#define WSAECONNABORTED ECONNABORTED            
		#define WSAECONNRESET ECONNRESET              
		#define WSAENOBUFS ENOBUFS                 
		#define WSAEISCONN EISCONN                 
		#define WSAENOTCONN ENOTCONN                
		#define WSAESHUTDOWN ESHUTDOWN               
		#define WSAETOOMANYREFS ETOOMANYREFS            
		#define WSAETIMEDOUT ETIMEDOUT               
		#define WSAECONNREFUSED ECONNREFUSED            
		#define WSAELOOP ELOOP                   
		#define WSAENAMETOOLONG ENAMETOOLONG            
		#define WSAEHOSTDOWN EHOSTDOWN               
		#define WSAEHOSTUNREACH EHOSTUNREACH            
		#define WSAENOTEMPTY ENOTEMPTY               
		#define WSAEPROCLIM EPROCLIM                
		#define WSAEUSERS EUSERS                  
		#define WSAEDQUOT EDQUOT                  
		#define WSAESTALE ESTALE                  
		#define WSAEREMOTE EREMOTE
		#define WSAEINVAL EINVAL
 	#endif               
#else
	#define GOAGetLastError(s) WSAGetLastError()
#endif


#ifdef __cplusplus
}
#endif


#ifndef _WIN32
	typedef struct sockaddr SOCKADDR;
	typedef struct sockaddr_in SOCKADDR_IN;
	typedef struct in_addr IN_ADDR;
	typedef struct hostent HOSTENT;
	typedef struct timeval TIMEVAL;
#endif

#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#ifdef _WIN32
	#define PATHCHAR '\\'
#else
#ifdef MACOS
	#define PATHCHAR ':'
#else
	#define PATHCHAR '/'
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

int SetSockBlocking(SOCKET sock, int isblocking);
int DisableNagle(SOCKET sock);
int SetReceiveBufferSize(SOCKET sock, int size);
int SetSendBufferSize(SOCKET sock, int size);
int GetReceiveBufferSize(SOCKET sock);
int GetSendBufferSize(SOCKET sock);
int CanReceiveOnSocket(SOCKET sock);
int CanSendOnSocket(SOCKET sock);

HOSTENT * getlocalhost(void);

int IsPrivateIP(IN_ADDR * addr);

#ifdef __cplusplus
}
#endif

#if defined(UNDER_CE) || defined(KGTRN_PLANET_WEB) || defined(KGTRN_NEXGEN)
//CE does not have the stdlib time() call
	#if defined(KGTRN_PLANET_WEB) || defined(KGTRN_NEXGEN)
		typedef long time_t;
	#endif
	time_t time(time_t *timer);
#else
	#include <time.h>
#endif

#if defined(__KATANA__) && defined(KGTRN_ACCESS)
	unsigned long fixed_inet_addr (const char * cp);
	#define inet_addr fixed_inet_addr
#endif

#if defined(__KATANA__) && defined(KGTRN_PLANET_WEB)
	#define gethostbyname pwgethostbyname
	struct hostent *pwgethostbyname(const char *name);
	#undef shutdown
	#define shutdown(s,h)  //shutdown locks up!
#endif

// SN Systems doesn't support gethostbyaddr
#if defined(SN_SYSTEMS)
	#define gethostbyaddr(a,b,c)   NULL
#endif


#ifndef UNDER_CE
	#include <ctype.h>
#else
	int isdigit(int c);
	int isxdigit(int c);
	int isalnum(int c);
	int isspace(int c);
#endif

#if defined(UNDER_CE) || defined(__KATANA__) || defined(_PS2)
	#define NOFILE
#endif

#ifndef SOMAXCONN
	#define SOMAXCONN 5
#endif

typedef const char * (* GetUniqueIDFunction)();

extern GetUniqueIDFunction GOAGetUniqueID;

#endif

#endif 
