/**
* \file     x_sockdefs.h
*
* \brief    Contains platform spedific include files and definitions
*
* \author   Daniel Melfi
*
* \version  1.0
*
* \date     8/23/2001
*/
//////////////////////////////////////////////////////////////////////////
#ifndef __X_SOCKDEF_H__
#define __X_SOCKDEF_H__
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif
//////////////////////////////////////////////////////////////////////////


// -----------------------------------------------------------------------
//
// BEGIN SUPPORTED PLATFORMS INCLUDE FILES
//
#if defined (TARGET_WIN32) 

    // some windows platforms define errno; some don't; lets make sure there not
    #ifndef _INC_ERRNO
    #define _INC_ERRNO
    #endif

    #include <winsock2.h>

#elif defined (TARGET_XBOX)
	#include <XTL.h>

#elif defined(TARGET_DOLPHIN)

	typedef struct InsockInAddr {
		u32 s_addr;
	} InsockInAddr_t;

	#define sa_family_t u16

	typedef struct InsockSockaddr {
		u8			sa_len;			/* total length */
		sa_family_t	sa_family;		/* address family */
		char		sa_data[14];	/* actually longer; address value */
	} InsockSockaddr_t;

	typedef struct InsockSockaddrIn {
		u8				sin_len;
		sa_family_t		sin_family;
		u16				sin_port;
		InsockInAddr_t	sin_addr;
		char			sin_zero[8];
	} InsockSockaddrIn_t;

	typedef struct InsockHostent {
		char	*h_name;		/* official name of host */
		char	**h_aliases;	/* alias list */
		s32		h_addrtype;		/* host address type */
		s32		h_length;		/* length of address */
		char	**h_addr_list;	/* list of addresses from name server */
	#define	h_addr	h_addr_list[0]	/* address, for backward compatibility */
	} InsockHostent_t;



    typedef	int                     X_SOCKET;
    #define X_SOCKADDR              InsockSockaddr
	#define X_SOCKADDRIN            InsockSockaddrIn
    #define X_INADDR                InsockInAddr
    #define X_HOSTENT               InsockHostent
    #define X_SOCKLEN               s32
	typedef s32						fd_set;
    #define SIZEOF_SOCKADDR_IN	    sizeof(struct InsockSockaddrIn)

    typedef s32                     SOCKERRTYPE;
    #define XSOCK_RECV_PEEK         0x00000000

	struct  x_timeval { int dummy;};

	#define	X_SOCKET	int

    #define X_HERRNO                0 // needs to resolve to global

    #define X_INVALID_SOCKET        -1
	#define X_SOCKET_ERROR			-1
	#define X_ERRNO					1
	#define X_INADDR_NONE			0xffffffffl

    #define AF_INET                 0
    #define SOCK_STREAM             1
	#define SOL_SOCKET				0
	#define SOCK_DGRAM				3

	#define INADDR_ANY				0



    #define EBADS                   -2
    #define EBADF                   EBADS - 1
    #define EWOULDBLOCK             EBADS - 2
    #define ENOMEM                  EBADS - 3
    #define EINPROGRESS             EBADS - 4
    #define EALREADY                EBADS - 5
    #define ENOTSOCK                EBADS - 6
    #define EDESTADDRREQ            EBADS - 7
    #define EMSGSIZE                EBADS - 8
    #define EPROTOTYPE              EBADS - 9
    #define ENOPROTOOPT				EBADS - 10
    #define EPROTONOSUPPORT         EBADS - 11
    #define ESOCKTNOSUPPORT         EBADS - 12
    #define EOPNOTSUPP              EBADS - 13
    #define EPFNOSUPPORT            EBADS - 14
    #define EAFNOSUPPORT            EBADS - 15
    #define EADDRINUSE              EBADS - 16
    #define EADDRNOTAVAIL           EBADS - 17
    #define ENETDOWN                EBADS - 18
    #define ENETUNREACH             EBADS - 19
    #define ENETRESET               EBADS - 20
    #define ECONNABORTED            EBADS - 21
    #define ECONNRESET              EBADS - 22
    #define ENOBUFS                 EBADS - 23
    #define EISCONN                 EBADS - 24
    #define ENOTCONN                EBADS - 25
    #define ESHUTDOWN               EBADS - 26
    #define ETOOMANYREFS            EBADS - 27
    #define ETIMEDOUT               EBADS - 28
    #define ECONNREFUSED            EBADS - 29
    #define ELOOP                   EBADS - 30
    #define ENAMETOOLONG            EBADS - 31
    #define EHOSTDOWN               EBADS - 32
    #define EHOSTUNREACH            EBADS - 33
    #define EPROCLIM                EBADS - 34
    #define EUSERS                  EBADS - 35
    #define EDQUOT                  EBADS - 36
    #define ESTALE                  EBADS - 37
    #define EREMOTE                 EBADS - 38
    #define EINTR                   EBADS - 39
    #define EIO                     EBADS - 40
    #define EINVAL                  EBADS - 41
    #define ENOTEMPTY               EBADS - 42
    #define EACCES                  EBADS - 43
    #define EFAULT                  EBADS - 44
    #define EMFILE                  EBADS - 45



#elif defined(TARGET_UNIX)

	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <netdb.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <errno.h>
	#include <sys/ioctl.h>
	#include <sys/time.h>

#elif defined (TARGET_PS2)

	#include <sys/types.h>

// The SN_SYSTEMS PSX2 stack is no longer being supported by ATG-NET
#ifndef SONY_PSX2_STACK
#define SONY_PSX2_STACK
#endif

    #if defined(SONY_PSX2_STACK)

		/*
        #ifndef _WINSOCK_H
        #define _WINSOCK_H
        #endif
		*/

        #define MAXRECVTHREADS      (5)


        /*-------------------------------------------------------------------
        * "sys/errno.h" re-defines NULL so we must do it again ~ D.M.
        *--------------------------------------------------------------------*/
        #ifndef UCHAR
            #define UCHAR
            typedef	unsigned char	u_char;
        #endif
        #ifndef USHORT
            #define UCHAR
            typedef	unsigned short	u_short;
        #endif
        #ifndef UINT
            #define UINT
            typedef	unsigned int	u_int;
        #endif
        #ifndef ULONG
            #define ULONG
            typedef	unsigned long	u_long;
        #endif

/*
		// old inet stack
        #include "libinsck.h"
        #include "libinsck/netinet/in.h"
        #include "libinsck/sys/socket.h"
        #include "libinsck/arpa/inet.h"
*/

		// include LibEEnet headers
		#include "libeenet.h"
		#include "libeenet/netinet/in.h"
		#include "libeenet/sys/socket.h"
		#include "libeenet/arpa/inet.h"

        #include "sys/errno.h"
        /*-------------------------------------------------------------------
        * "sys/errno.h" re-defines NULL so we must do it again ~ D.M.
        *--------------------------------------------------------------------*/
        #ifdef NULL
        #undef NULL
        #endif

        #ifndef NULL
          #ifdef __cplusplus
            #define NULL            0
          #else
            #define NULL            ((void*)0)
          #endif
        #endif

        #ifndef sceInsockDisableSocketSymbolAliases
        #define sceInsockDisableSocketSymbolAliases
        #endif

        #ifndef sceNetGlueDisableSocketSymbolAliases
        #define sceNetGlueDisableSocketSymbolAliases
        #endif

    #elif defined (SN_SYSTEMS_PSX2_STACK)

        #error "-------------------------------------------------------------"
        #error "SN_SYSTEMS PSX2 stack is no longer being supported by ATG-NET"
        #error "-------------------------------------------------------------"

    	/* SN-Systems specific files - currently no longer supported */
        #include <string.h>
	    #include <sntypes.h>    /* SN Systems types                     */
	    #include <sneeutil.h>   /* SN Systems PS2 EE Utilites (General) */
	    #include <snsocket.h>   /* SN Systems socket API header file    */
	    #include <sntcutil.h>   /* SN Systems PS2 EE Utilites (TCP/IP)  */

    #endif

#endif
//
// END SUPPORTED PLATFORMS INCLUDE FILES
//
// -----------------------------------------------------------------------



// -----------------------------------------------------------------------
//
// BEGINS SUPPORTED PLATFORMS DEFINTIONS SECTION
//

/* ERRORS COMMON TO ALL PLATFORMS BUT NOT DEFINED BY BSD STANDARD */
#define ENOTINITIALIZED             (9999)
#define ENOTIMPLEMENTED             (ENOTINITIALIZED + 1)

#if (defined (TARGET_WIN32) || defined(TARGET_XBOX))

    typedef	SOCKET                  X_SOCKET;
    #define X_SOCKADDR              sockaddr
    #define X_SOCKADDRIN            sockaddr_in	
    #define X_HOSTENT               hostent
    typedef s32                     SOCKERRTYPE;
    #define X_INVALID_SOCKET        INVALID_SOCKET
    #define X_SOCKET_ERROR          SOCKET_ERROR
    #define SIZEOF_SOCKADDR_IN	    sizeof(struct X_SOCKADDRIN)
	#define X_ERRNO                 WSAGetLastError()
    #define X_HERRNO                WSAGetLastError()
	#define X_INADDR_ANY			INADDR_ANY
	#define X_INADDR_NONE			0xffffffffl
    #define X_INADDR_BCAST			INADDR_BROADCAST
    #define X_INADDR                in_addr
    #define X_SOCKLEN               int    

//    #define X_PORT_AUTO             0
//    #define X_PORT_ANY              0


	#define X_FD_CLR				FD_CLR
	#define X_FD_ISSET				FD_ISSET
	#define X_FD_SET				FD_SET
	#define X_FD_ZERO				FD_ZERO

    
    #define EBADS                   WSAEBADF
    #define EBADF                   WSAEBADF
    #define EWOULDBLOCK             WSAEWOULDBLOCK
    #define ENOMEM                  WSA_NOT_ENOUGH_MEMORY 
    #define EINPROGRESS             WSAEINPROGRESS
    #define EALREADY                WSAEALREADY
    #define ENOTSOCK                WSAENOTSOCK
    #define EDESTADDRREQ            WSAEDESTADDRREQ
    #define EMSGSIZE                WSAEMSGSIZE
    #define EPROTOTYPE              WSAEPROTOTYPE
    #define ENOPROTOOPT             WSAENOPROTOOPT
    #define EPROTONOSUPPORT         WSAEPROTONOSUPPORT
    #define ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT
    #define EOPNOTSUPP              WSAEOPNOTSUPP
    #define EPFNOSUPPORT            WSAEPFNOSUPPORT
    #define EAFNOSUPPORT            WSAEAFNOSUPPORT
    #define EADDRINUSE              WSAEADDRINUSE
    #define EADDRNOTAVAIL           WSAEADDRNOTAVAIL
    #define ENETDOWN                WSAENETDOWN
    #define ENETUNREACH             WSAENETUNREACH
    #define ENETRESET               WSAENETRESET
    #define ECONNABORTED            WSAECONNABORTED
    #define ECONNRESET              WSAECONNRESET
    #define ENOBUFS                 WSAENOBUFS
    #define EISCONN                 WSAEISCONN
    #define ENOTCONN                WSAENOTCONN
    #define ESHUTDOWN               WSAESHUTDOWN
    #define ETOOMANYREFS            WSAETOOMANYREFS
    #define ETIMEDOUT               WSAETIMEDOUT
    #define ECONNREFUSED            WSAECONNREFUSED
    #define ELOOP                   WSAELOOP
    #define ENAMETOOLONG            WSAENAMETOOLONG
    #define EHOSTDOWN               WSAEHOSTDOWN
    #define EHOSTUNREACH            WSAEHOSTUNREACH
    #define EPROCLIM                WSAEPROCLIM
    #define EUSERS                  WSAEUSERS
    #define EDQUOT                  WSAEDQUOT
    #define ESTALE                  WSAESTALE
    #define EREMOTE                 WSAEREMOTE
    #define EINTR                   WSAEINTR
    #define EIO                     WSAENETDOWN
    #define EINVAL                  WSAEINVAL
    #define ENOTEMPTY               WSAENOTEMPTY
    #define EACCES                  WSAEACCES
    #define EFAULT                  WSAEFAULT
    #define EMFILE                  WSAEMFILE

    #define x_timeval               timeval

    #define XSOCK_RECV_PEEK         0x00000000

#elif defined(TARGET_UNIX)

    typedef	int                     X_SOCKET;
    #define X_SOCKADDR              sockaddr
    #define X_SOCKADDRIN            sockaddr_in
    #define X_HOSTENT               hostent
    typedef s32                     SOCKERRTYPE;
    #define SIZEOF_SOCKADDR_IN	    sizeof(struct X_SOCKADDR)

	#define X_INADDR_ANY			IPADR_ANY
	#define X_INADDR_NONE			IPADR_NONE

	#define x_fd_set				fd_set

	#define X_FD_CLR				FD_CLR
	#define X_FD_ISSET				FD_ISSET
	#define X_FD_SET				FD_SET
	#define X_FD_ZERO				FD_ZERO

    #define SIZEOF_SOCKADDR_IN      sizeof(struct sockaddr_in)

    #define X_SOCKET_ERROR          (-1)
    #define X_INVALID_SOCKET        (-1)

    #define XSOCK_RECV_PEEK         0x00000000

#elif defined( TARGET_PS2 )

//-------------------------------------------------------------------------------------------

//    #define SOCK_DGRAM              sceINETT_DGRAM
//    #define SOCK_STREAM             sceINETT_CONNECT
//    #define AF_INET                 2
//    #define MIN_STARTUP_SOCKETS     10
//    #define SOMAXCONN               MIN_STARTUP_SOCKETS
	typedef	int                     X_SOCKET;
    typedef s32                     SOCKERRTYPE;


	#define X_INADDR_ANY			INADDR_ANY
	#define X_INADDR_NONE			INADDR_NONE
    #define X_INADDR_BCAST			INADDR_BROADCAST
    #define X_SOCKET_ERROR          (-1)
    #define X_INVALID_SOCKET        (-1)
    #define X_HOSTENT               sceEENetHostent

//    #define X_PORT_AUTO             sceINETP_AUTO
//    #define X_PORT_ANY              sceINETP_ANY

    #define X_FD_CLR				FD_CLR
    #define X_FD_ISSET				FD_ISSET
    #define X_FD_SET				FD_SET
    #define X_FD_ZERO				FD_ZERO

    #define X_ERRNO                 sceEENetErrno
    #define X_HERRNO                sceEENetHErrno

    #define SIZEOF_SOCKADDR_IN      sizeof(struct X_SOCKADDRIN)

    #define EBADS                   EBADFD

    #define X_SOCKADDR              sceEENetSockaddr
    #define X_SOCKADDRIN            sceEENetSockaddrIn

    #define X_SOCKLEN               socklen_t
    #define X_INADDR                sceEENetInAddr

    struct x_timeval {
                        u32 tv_sec;
                        u32 tv_usec;
                     };

    /* Definitions for type of shutdown() */
    #define SD_RECEIVE  0
    #define SD_SEND     1
    #define SD_BOTH     2

    /* some other non-supported flags */
/*
	#define FIONBIO      0
    #define SOL_SOCKET   0
    #define MSG_PEEK     0
    #define SO_BROADCAST 0xf1
    #define SOMAXCONN    10
*/
    #define XSOCK_RECV_PEEK          0x00001000

#endif

// END SUPPORTED PLATFORMS DEFINTIONS SECTION
//
// -----------------------------------------------------------------------




//////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
    }
#endif
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
#endif //__X_SOCKDEF_H__
//////////////////////////////////////////////////////////////////////////
