/******
nonport.c
GameSpy Common Code
  
Copyright 1999-2002 GameSpy Industries, Inc

18002 Skypark Circle
Irvine, California 92614
949.798.4200 (Tel)
949.798.4299 (Fax)
devsupport@gamespy.com
******/

#include "nonport.h"

#if defined( X_DEBUG )
	#define DBG_PRINT( exp )	exp
#else
	#define DBG_PRINT( exp )
#endif


#if defined(_WIN32) && !defined(UNDER_CE)
#pragma comment(lib, "wsock32")
#pragma comment(lib, "advapi32")
#endif

#ifdef UNDER_CE
#pragma comment(lib, "platutil")
#pragma comment(lib, "winsock")
#endif

// Disable compiler warnings for issues that are unavoidable.
/////////////////////////////////////////////////////////////
#if defined(_MSC_VER) // DevStudio
// Level4, "conditional expression is constant". 
// Occurs with use of the MS provided macro FD_SET
#pragma warning ( disable: 4127 )
#endif // _MSC_VER
	
#ifdef UNDER_CE
time_t time(time_t *timer)
{
	static time_t ret;
	SYSTEMTIME systime;
	FILETIME ftime;
	LONGLONG holder;

	GetLocalTime(&systime);
	SystemTimeToFileTime(&systime, &ftime);
	holder = (ftime.dwHighDateTime << 32) + ftime.dwLowDateTime;
	if (timer == NULL)
		timer = &ret;
	*timer = (time_t)((holder - 116444736000000000) / 10000000);
	return *timer; 
}

int isdigit( int c )
{
	return (c >= '0' && c <= '9');
}

int isxdigit( int c )
{
	return ((c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f') || (c >= '0' && c <= '9'));
}

int isalnum( int c )
{
	return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'));
}

int isspace(int c)
{
	return ((c >= 0x9 && c <= 0xD) || (c == 0x20)); 
}
#endif

#ifdef __KATANA__
/*
631152000 -- diff b/t 1950 & 1970 GMT
631126800 -- diff b/t 1950 GMT & 1970 PDT
*/
time_t time(time_t *timer)
{
	static time_t ret;
	SYS_RTC_DATE date;
	Uint32 count;
	syRtcGetDate(&date);
	syRtcDateToCount(&date, &count);
	if (timer == NULL)
		timer = & ret;
	*timer = (time_t)(count - 631152000);	
}

void *fixed_realloc(void *ptr, int newsize)
{
	if (ptr && newsize == 0)
	{
		gsifree(ptr);
		return NULL;
	}
	else if (ptr)
		return realloc(ptr, newsize);
	else //ptr is null
		return gsimalloc(newsize);
}
#endif

#ifdef __mips64
typedef unsigned char u_char;
typedef struct {
        u_char stat;            /* status */
        u_char second;          /* second */
        u_char minute;          /* minute */
        u_char hour;            /* hour   */

        u_char pad;             /* pad    */
        u_char day;             /* day    */
        u_char month;           /* month  */
        u_char year;            /* year   */
} sceCdCLOCK;

extern "C"
{
	extern int sceCdReadClock( sceCdCLOCK* pClock );
}

unsigned long GetTicks()
{
	unsigned long ticks;
	asm volatile (" mfc0 %0, $9 " : "=r" (ticks));
    return ticks;
}

#define DEC(x) (10*(x/16)+(x%16))
#define _BASE_YEAR 			 70L
#define _MAX_YEAR 			138L
#define _LEAP_YEAR_ADJUST 	 17L
int _days[] = {-1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364};

time_t _gmtotime_t (
        int yr,     /* 0 based */
        int mo,     /* 1 based */
        int dy,     /* 1 based */
        int hr,
        int mn,
        int sc
        )
{
        int tmpdays;
        long tmptim;
        struct tm tb;

        if ( ((long)(yr -= 1900) < _BASE_YEAR) || ((long)yr > _MAX_YEAR) )
                return (time_t)(-1);

        tmpdays = dy + _days[mo - 1];
        if ( !(yr & 3) && (mo > 2) )
                tmpdays++;

        tmptim = (long)yr - _BASE_YEAR;

        tmptim = ( ( ( ( tmptim ) * 365L
                 + ((long)(yr - 1) >> 2) - (long)_LEAP_YEAR_ADJUST
                 + (long)tmpdays )
                 * 24L + (long)hr )
                 * 60L + (long)mn )
                 * 60L + (long)sc;

        tb.tm_yday = tmpdays;
        tb.tm_year = yr;
        tb.tm_mon = mo - 1;
        tb.tm_hour = hr;
        
        return (tmptim >= 0) ? (time_t)tmptim : (time_t)(-1);
}

time_t time(time_t *timer)
{
	time_t tim;
	sceCdCLOCK clocktime; /* defined in libcdvd.h */

	sceCdReadClock(&clocktime); /* libcdvd.a */

	tim =   _gmtotime_t ( DEC(clocktime.year)+2000,
							DEC(clocktime.month),
							DEC(clocktime.day),
							DEC(clocktime.hour),
							DEC(clocktime.minute),
							DEC(clocktime.second));

	if(timer)
		*timer = tim;
		
	return tim;
}

#endif /* __mips64 */

unsigned long current_time()  //returns current time in msec
{ 
#ifdef _WIN32
	return (GetTickCount()); 
#endif

#ifdef _MACOS
	return (TickCount() * 50) / 3;
#endif

#ifdef __KATANA__
	return (syTmrCountToMicro(syTmrGetCount()))/1000;
#endif

#ifdef __mips64
	unsigned int ticks;
	static unsigned int msec = 0;
	static unsigned int lastticks = 0;
	sceCdCLOCK lasttimecalled; /* defined in libcdvd.h */

	if(!msec)
	{
		sceCdReadClock(&lasttimecalled); /* libcdvd.a */
		msec =  (DEC(lasttimecalled.day) * 86400000) +
				(DEC(lasttimecalled.hour) * 3600000) +
				(DEC(lasttimecalled.minute) * 60000) +
				(DEC(lasttimecalled.second) * 1000) + 
				(msec % 1000);
	}

	ticks = GetTicks();
	if(lastticks > ticks)
		msec += ((sizeof(unsigned int) - lastticks) + ticks) / 300000;
	else
		msec += (unsigned int)(ticks-lastticks) / 300000;
	lastticks = ticks;

	return msec;
#endif

#ifdef UNDER_UNIX
	struct timeval time;
	
	gettimeofday(&time, NULL);
	return (time.tv_sec * 1000 + time.tv_usec / 1000);
#endif
}

#ifdef __KATANA__
extern void DrawScreen(void);
#endif

void msleep(unsigned long msec)
{
#ifdef _WIN32
	Sleep(msec);
#endif

#ifdef __KATANA__
	//wait for the approx msec based on vertical refreshes
	unsigned long stoptime = current_time() + msec;
	do
	{
		//vsWaitVSync(1);
		njWaitVSync();
		DrawScreen();
	} while (current_time() < stoptime);
#endif

#ifdef _MACOS
//	EventRecord rec;
	WaitNextEvent(everyEvent,/*&rec*/NULL, (msec*1000)/60, NULL);
#endif

#ifdef _PS2
	#ifdef SN_SYSTEMS
		sn_delay(msec);
	#endif
	#ifdef CISCO_NFT
		;
	#endif
	#ifdef EENET
		if(msec >= 1000)
		{
			sleep(msec / 1000);
			msec -= (msec / 1000);
		}
		if(msec)
			usleep(msec * 1000);
	#endif
#endif

#ifdef UNDER_UNIX
	usleep(msec * 1000);
#endif
}

void SocketStartUp()
{
#if defined(_WIN32) || defined(_MACOS)
	WSADATA data;
	WSAStartup(0x0101, &data);
#endif
}

void SocketShutDown()
{
#if defined(_WIN32) || defined(_MACOS)
	WSACleanup();
#endif
}

char * goastrdup(const char *src)
{
	char *res;
	if(src == NULL)      //PANTS|02.11.00|check for NULL before strlen
		return NULL;
	res = (char *)gsimalloc(strlen(src) + 1);
	if(res != NULL)      //PANTS|02.02.00|check for NULL before strcpy
		strcpy(res, src);
	return res;
}

#if !defined(_WIN32) || defined(UNDER_CE)
#include <ctype.h>

char *_strlwr(char *string)
{
	char *hold = string;
	while (*string)
	{
		*string = tolower(*string);
		string++;
	}

	return hold;
}

char *_strupr(char *string)
{
	char *hold = string;
	while (*string)
	{
		*string = toupper(*string);
		string++;
	}

	return hold;
}
#endif

int SetSockBlocking(SOCKET sock, int isblocking)
{
	int rcode;
#ifdef __KATANA__
	#ifdef KGTRN_ACCESS
		short   argp;
	#endif

	#ifdef KGTRN_PLANET_WEB
		long argp;
	#endif

	#ifdef KGTRN_NEXGEN
		int argp;
	#endif
#endif

#ifdef EENET
	socklen_t argp;
#else
	unsigned long argp;
#endif
	
	if(isblocking)
		argp = 0;
	else
		argp = 1;

#ifdef __KATANA__
	#ifdef KGTRN_ACCESS
		rcode = setsockopt(sock, SOL_SOCKET, SO_NOBLK,(char *)&argp, sizeof(argp));
	#endif

	#ifdef KGTRN_PLANET_WEB
		if ( (argp = net_fcntl( sock, F_GETFL, 0)) < 0 )
			return -1;
		if (isblocking)
			argp &= ~O_NONBLOCK;	
		else
			argp |= O_NONBLOCK;	
		rcode = net_fcntl( sock, F_SETFL, argp );
	#endif

	#ifdef KGTRN_NEXGEN
		if ( (argp = fcntlsocket( sock, F_GETFL, 0)) < 0 )
			return -1;
		if (isblocking)
			argp &= ~O_NONBLOCK;	
		else
			argp |= O_NONBLOCK;	
		rcode = fcntlsocket( sock, F_SETFL, argp );		
	#endif
#endif

#ifdef _PS2
	#ifdef SN_SYSTEMS
		rcode = setsockopt(sock, SOL_SOCKET, (isblocking) ? SO_BIO : SO_NBIO, &argp, sizeof(argp));
	#endif

	#ifdef CISCO_NFT
		// cisco doesn't support non-blocking
	#endif

	#ifdef EENET
		rcode = setsockopt(sock, SOL_SOCKET, SO_NBIO, &argp, sizeof(argp));
	#endif

#else
	rcode = ioctlsocket(sock, FIONBIO, &argp);
#endif

	if(rcode == 0)
		return 1;

	return 0;
}

int DisableNagle(SOCKET sock)
{
#if defined(_WIN32) || defined(KGTRN_PLANET_WEB) || defined(KGTRN_NEXGEN) || defined(UNDER_UNIX) || defined(CISCO_NFT)
	int rcode;
	int noDelay = 1;

	rcode = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&noDelay, sizeof(int));

	return (rcode != SOCKET_ERROR);	
#endif

	// not supported
	return 0;
}

int SetReceiveBufferSize(SOCKET sock, int size)
{
	int rcode;

	rcode = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (const char *)&size, sizeof(int));

	return (rcode != SOCKET_ERROR);
}

int SetSendBufferSize(SOCKET sock, int size)
{
	int rcode;

	rcode = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (const char *)&size, sizeof(int));

	return (rcode != SOCKET_ERROR);
}

int GetReceiveBufferSize(SOCKET sock)
{
	int rcode;
	int size;
	int len;

	len = sizeof(size);
	rcode = sceEENetGetsockopt(sock, SOL_SOCKET, SO_RCVBUF, (void *)&size, (unsigned int*)&len);
	if(rcode == SOCKET_ERROR)
		return -1;

	return size;
}

int GetSendBufferSize(SOCKET sock)
{
	int rcode;
	int size;
	int len;

	len = sizeof(size);
	rcode = sceEENetGetsockopt(sock, SOL_SOCKET, SO_SNDBUF, (void *)&size, (unsigned int*)&len);
	if(rcode == SOCKET_ERROR)
		return -1;

	return size;
}

int CanReceiveOnSocket(SOCKET sock)
{
	fd_set fd;
	struct timeval timeout;
	int rcode;
#ifdef SN_SYSTEMS
	int count = 0;
#endif

	// setup the fd set
	FD_ZERO(&fd);
	FD_SET(sock, &fd);

	// setup the timeout
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

#ifdef SN_SYSTEMS
repeat:
#endif

	// do the actual select
	rcode = select(FD_SETSIZE, &fd, NULL, NULL, &timeout);
	if((rcode == SOCKET_ERROR) || (rcode == 0))
		return 0;

#ifdef SN_SYSTEMS
	// check for an error, but don't get stuck forever
	if(GOAGetLastError(sock))
	{
		if(++count == 10)
			return 0;

		goto repeat;
	}
#endif

	// it was set
	return 1;
}

int CanSendOnSocket(SOCKET sock)
{
	fd_set fd;
	struct timeval timeout;
	int rcode;
#ifdef SN_SYSTEMS
	int count = 0;

repeat:
#endif

	// setup the fd set
	FD_ZERO(&fd);
	FD_SET(sock, &fd);

	// setup the timeout
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	// do the actual select
	rcode = select(FD_SETSIZE, NULL, &fd, NULL, &timeout);
	if((rcode == SOCKET_ERROR) || (rcode == 0))
		return 0;

#ifdef SN_SYSTEMS
	// check for an error, but don't get stuck forever
	if(GOAGetLastError(sock))
	{
		if(++count == 10)
			return 0;
		goto repeat;
	}
#endif

	// it was set
	return 1;
}

HOSTENT * getlocalhost(void)
{
#ifdef EENET
	#define MAX_IPS  5

	static HOSTENT localhost;
	static char * aliases = NULL;
	static char * ipPtrs[MAX_IPS + 1];
	static unsigned int ips[MAX_IPS];

	struct sceEENetIfname * interfaces;
	struct sceEENetIfname * interface;
	int num;
	int i;
	int count;
	int len;
	u_short flags;
	IN_ADDR address;

	// initialize the host
	localhost.h_name = "localhost";
	localhost.h_aliases = &aliases;
	localhost.h_addrtype = AF_INET;
	localhost.h_length = 0;
	localhost.h_addr_list = ipPtrs;

	// get the local interfaces
	sceEENetGetIfnames(NULL, &num);
	interfaces = (struct sceEENetIfname *)gsimalloc(num * sizeof(struct sceEENetIfname));
	if(!interfaces)
		return NULL;
	sceEENetGetIfnames(interfaces, &num);

	// loop through the interfaces
	count = 0;
	for(i = 0 ; i < num ; i++)
	{
		// the next interface
		interface = &interfaces[i];
		//DBG_PRINT(printf("eenet%d: %s\n", i, interface->ifn_name));

		// get the flags
		len = sizeof(flags);
		if(sceEENetGetIfinfo(interface->ifn_name, sceEENET_IFINFO_IFFLAGS, &flags, &len) != 0)
			continue;
		//DBG_PRINT(printf("eenet%d flags: 0x%X\n", i, flags));

		// check for up, running, and non-loopback
		if(!(flags & (IFF_UP|IFF_RUNNING)) || (flags & IFF_LOOPBACK))
			continue;
		//DBG_PRINT(printf("eenet%d: up and running, non-loopback\n", i));

		// get the address
		len = sizeof(address);
		if(sceEENetGetIfinfo(interface->ifn_name, sceEENET_IFINFO_ADDR, &address, &len) != 0)
			continue;
		//DBG_PRINT(printf("eenet%d: %s\n", i, inet_ntoa(address)));

		// add this address
		ips[count] = address.s_addr;
		ipPtrs[count] = (char *)&ips[count];
		count++;
	}

	// free the interfaces
	gsifree(interfaces);

	// check that we got at least one IP
	if(!count)
		return NULL;

	// finish filling in the host struct
	localhost.h_length = (4 * count);
	ipPtrs[count] = NULL;

	return &localhost;
#else
	char hostname[256] = "";

	// get the local host's name
	gethostname(hostname, sizeof(hostname));

	// return the host for that name
	return gethostbyname(hostname);
#endif
}

int IsPrivateIP(IN_ADDR * addr)
{
	int b1;
	int b2;
	unsigned int ip;

	// get the first 2 bytes
	ip = ntohl(addr->s_addr);
	b1 = ((ip >> 24) & 0xFF);
	b2 = ((ip >> 16) & 0xFF);

	// 10.X.X.X
	if(b1 == 10)
		return 1;

	// 172.16-31.X.X
	if((b1 == 172) && ((b2 >= 16) && (b2 <= 31)))
		return 1;

	// 192.168.X.X
	if((b1 == 192) && (b2 == 168))
		return 1;

	return 0;
}

#if defined(_MACOS) || defined(UNDER_CE) || defined(__KATANA__)
int strcasecmp(const char *string1, const char *string2)
{
	while (tolower(*string1) == tolower(*string2) && *string1 != 0 && *string2 != 0)
	{
		*string1++; *string2++;
	}
	return tolower(*string1) - tolower(*string2);
}

int strncasecmp(const char *string1, const char *string2, size_t count)
{
	while (--count > 0 && tolower(*string1) == tolower(*string2) && *string1 != 0 && *string2 != 0)
	{
		*string1++; *string2++;
	}
	return tolower(*string1) - tolower(*string2);
}
#endif

#ifdef __KATANA__

#ifndef __GNUC__
void abort(void)
{
	//called by assert on dreamcast but not present
	//could do a hardware break in here if needed	
}
#endif

unsigned long fixed_inet_addr (const char * cp)
{
	int idata[4];
	unsigned char data[4];
	int pc = 0;
	char *pos;
	
	//validate the IP
	for (pos = (char *)cp ; *pos != 0; pos++)
	{
		if( (*pos < '0' || *pos > '9') && *pos != '.')
			return INADDR_NONE;
		if (*pos == '.')
			pc++; //increment the period counter
	}
	if (pc != 3)
		return INADDR_NONE; //not enough periods
		
	sscanf(cp, "%d.%d.%d.%d", idata, idata+1,idata+2,idata+3);
	data[0] = (unsigned char)idata[0]; data[1] = (unsigned char)idata[1]; data[2] = (unsigned char)idata[2]; data[3] = (unsigned char)idata[3];
	return *(unsigned long *)data;
}

#ifdef KGTRN_PLANET_WEB
#define MAX_HOST_ADDRS 16
struct hostent *pwgethostbyname(const char *name)
{
	static char *aliases = NULL;
	static struct in_addr *paddrs[MAX_HOST_ADDRS];
	static struct hostent hent;
	int i;
	int err;
	static struct addrinfo *ai = NULL;
	struct addrinfo *curai = NULL;
	
	if (ai != NULL)
		freeaddrinfo(ai);
	ai = NULL;
	err = getaddrinfo(name, NULL, NULL, &ai);
	if (err != 0 || ai == NULL)
	{
		return NULL;
	}
	hent.h_name = ai->ai_canonname;
	hent.h_aliases = &aliases;
	hent.h_addrtype = 2;
	hent.h_length = 4;
	
	for (i = 0, curai = ai ; i < MAX_HOST_ADDRS - 1, curai != NULL ; i++, curai = curai->ai_next)
		paddrs[i] = &((struct sockaddr_in *)curai->ai_addr)->sin_addr;
	paddrs[i] = NULL;
	hent.h_addr_list = (char **)paddrs;
	 
	return &hent;
}
#endif //PWEB

#endif //KATANA

#ifdef SN_SYSTEMS
int GOAGetLastError(SOCKET s)
{
	int val;
	int soval = sizeof(val);
	getsockopt(s,SOL_SOCKET,SO_ERROR,&val,&soval);
	return val;
}
#endif

#ifdef UNDER_CE
const char * GOAGetUniqueID_Internal(void)
{
	static char keyval[17];
	unsigned char buff[8];
	DWORD size;

	size = 0;
	for (size = 0 ; size < sizeof(buff) ; size++)
		buff[size] = 0;
	size = sizeof(buff);
	FirmwareGetSerialNumber(buff, &size);
	for (size = 0 ; size < sizeof(buff) ; size++)
	{
		sprintf(keyval + (size * 2),"%02x",buff[size]);
	}
	return keyval;
}
#endif //UNDER_CE


#ifdef __mips64
#ifdef UNIQUEID

#if defined(EENET)

#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/if_ether.h>

const char * GetMAC(void)
{
	static struct sceEENetEtherAddr linkAddress;
	struct sceEENetIfname * interfaces;
	struct sceEENetIfname * interface;
	int num;
	int type;
	int len;
	int i;
	const unsigned char * MAC = NULL;

	// get the local interfaces
	sceEENetGetIfnames(NULL, &num);
	interfaces = (struct sceEENetIfname *)gsimalloc(num * sizeof(struct sceEENetIfname));
	if(!interfaces)
		return NULL;
	sceEENetGetIfnames(interfaces, &num);

	// loop through the interfaces
	for(i = 0 ; i < num ; i++)
	{
		// the next interface
		interface = &interfaces[i];
		//DBG_PRINT(printf("eenet%d: %s\n", i, interface->ifn_name));

		// get the type
		len = sizeof(type);
		if(sceEENetGetIfinfo(interface->ifn_name, sceEENET_IFINFO_IFTYPE, &type, &len) != 0)
			continue;
		//DBG_PRINT(printf("eenet%d type: %d\n", i, type));

		// check for ethernet
		if(type != sceEENET_IFTYPE_ETHER)
			continue;
		//DBG_PRINT(printf("eenet%d: ethernet\n", i));

		// get the address
		len = sizeof(linkAddress);
		if(sceEENetGetIfinfo(interface->ifn_name, sceEENET_IFINFO_MACADDR, &linkAddress, &len) != 0)
			continue;
		MAC = linkAddress.ether_addr_octet;
		//DBG_PRINT(printf("eenet%d: MAC: %02X-%02X-%02X-%02X-%02X-%02X\n", i, MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]));

		break;
	}

	// free the interfaces
	gsifree(interfaces);

	return (const char*)MAC;
}

#elif defined(SN_SYSTEMS)

const char * GetMAC(void)
{
	static char MAC[6];
	int len = sizeof(MAC);
	int rcode;

	// get the MAC
	rcode = sndev_get_status(0, SN_DEV_STAT_MAC, MAC, &len);
	if((rcode != 0) || (len != 6))
		return NULL;

	return MAC;
}

#elif defined(CISCO_NFT)

#include <nft/libnft.h>
#include <libmrpc.h>

#undef ntohs
#undef ntohl
#include <libnet.h>

#define SCEROOT           "host0:/usr/local/sce/"
#define MODROOT           SCEROOT "iop/modules/"
#define APPROOT           SCEROOT "conf/neticon/english/"

#define MOD_INET          MODROOT "inet.irx"
#define	MOD_NETCNF        MODROOT "netcnf.irx"
#define	MOD_DEV9          MODROOT "dev9.irx"
#define	MOD_MSIFRPC       MODROOT "msifrpc.irx"
#define	MOD_SMAP          MODROOT "smap.irx"
#define	MOD_INETCTL       MODROOT "inetctl.irx"
#define	MOD_LIBNET        MODROOT "libnet.irx"

#define	INETCTL_ARG       "-no_auto" "\0" "-no_decode"
#define	NETCNF_ICON       APPROOT "SYS_NET.ICO"
#define	NETCNF_ICONSYS    APPROOT "icon.sys"
#define	NETCNF_ARG        "icon=" NETCNF_ICON "\0" "iconsys=" NETCNF_ICONSYS

static int LoadModule(const char * module, int args, const char * argp)
{
	int rcode;
	
	rcode = sceSifLoadModule(module, args, argp);
	if(rcode < 0)
		DBG_PRINT(printf("Failed to load module %s (%d)\n", module, rcode));

	return rcode;
}

const char * GetMAC(void)
{
	sceSifMClientData cd;
	static unsigned int netBuffer[512] __attribute__((aligned(64)));
	int interfaceID;
	static u_char hardwareAddress[16];
	int rcode;

	// init rpc
	sceSifInitRpc(0);

	// init the IOP heap
    sceSifInitIopHeap();

	// load the needed modules
	LoadModule(MOD_INET, 0, NULL);
	LoadModule(MOD_NETCNF, sizeof(NETCNF_ARG), NETCNF_ARG);
	LoadModule(MOD_DEV9, 0, NULL);
	LoadModule(MOD_SMAP, 0, NULL);
	LoadModule(MOD_INETCTL, sizeof(INETCTL_ARG), INETCTL_ARG);
	LoadModule(MOD_MSIFRPC, 0, NULL);
	LoadModule(MOD_LIBNET, 0, NULL);

	// init mrpc
	sceSifMInitRpc(0);

	// init libnet
    rcode = sceLibnetInitialize(&cd, sceLIBNET_BUFFERSIZE, sceLIBNET_STACKSIZE, sceLIBNET_PRIORITY);
	if(rcode < 0)
		return NULL;
	
	// get the list of interfaces
	rcode = sceInetGetInterfaceList(&cd, netBuffer, &interfaceID, 1);
	if(rcode <= 0)
		return NULL;

	// get info on the interface
	rcode = sceInetInterfaceControl(&cd, netBuffer, interfaceID, (int)sceInetCC_GetHWaddr, hardwareAddress, (int)sizeof(hardwareAddress));
	if(rcode < 0)
		return NULL;

	// free libnet
	sceLibnetTerminate(&cd);

	// exit mrpc
	sceSifMExitRpc();

	// reboot
	while (!sceSifRebootIop(MODROOT IOP_IMAGE_file));
	while (!sceSifSyncIop());

	// reset
	sceFsReset();
	sceSifLoadFileReset();

	return (char *)hardwareAddress;
}

#endif

const char * GOAGetUniqueID_Internal(void)
{
	static char keyval[17];
	const char * MAC;

	// check if we already have the Unique ID
	if(keyval[0])
		return keyval;

	// get the MAC
	MAC = GetMAC();
	if(!MAC)
	{
		// error getting the MAC
		static char errorMAC[6] = { 1, 2, 3, 4, 5, 6 };
		MAC = errorMAC;
	}

	// format it
	sprintf(keyval, "%02X%02X%02X%02X%02X%02X0000",
		MAC[0] & 0xFF,
		MAC[1] & 0xFF,
		MAC[2] & 0xFF,
		MAC[3] & 0xFF,
		MAC[4] & 0xFF,
		MAC[5] & 0xFF);

	return keyval;
}

#endif // UNIQUEID
#endif //__mips64

#ifdef _MACOS
const char * GOAGetUniqueID_Internal(void)
{
	static char keyval[17];
	keyval[0] = '\0';
	return keyval;
}
#endif // _MACOS

#if (defined(_WIN32) || defined(UNDER_UNIX)) && !defined(UNDER_CE)

#define RANa 16807         /* multiplier */
#define LONGRAND_MAX 2147483647L   /* 2**31 - 1 */

static long randomnum = 1;

static long nextlongrand(long seed)
{
	unsigned

	long lo, hi;
	lo = RANa *(long)(seed & 0xFFFF);
	hi = RANa *(long)((unsigned long)seed >> 16);
	lo += (hi & 0x7FFF) << 16;

	if (lo > LONGRAND_MAX)
	{
		lo &= LONGRAND_MAX;
		++lo;
	}
	lo += hi >> 15;

	if (lo > LONGRAND_MAX)
	{
		lo &= LONGRAND_MAX;
		++lo;
	}

	return(long)lo;
}

static long longrand(void)/* return next random long */
{
	randomnum = nextlongrand(randomnum);
	return randomnum;
}

static void Util_RandSeed(unsigned long seed)/* to seed it */
{
	randomnum = seed ? (seed & LONGRAND_MAX) : 1;
	/* nonzero seed */
}

static int Util_RandInt(int low, int high)
{
	int range = high-low;
	int num = longrand() % range;

	return(num + low);
}

static void GenerateID(char *keyval)
{
	int i;
	const char crypttab[63] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
#ifdef _WIN32
	LARGE_INTEGER l1;
	UINT seed;
	if (QueryPerformanceCounter(&l1))
		seed = (l1.LowPart ^ l1.HighPart);
	else
		seed = 0;
	Util_RandSeed(seed ^ GetTickCount() ^ time(NULL) ^ clock());
#else
	Util_RandSeed(time(NULL) ^ clock());
#endif
	for (i = 0; i < 19; i++)
		if (i == 4 || i == 9 || i == 14)
			keyval[i] = '-';
	else
		keyval[i] = crypttab[Util_RandInt(0, 62)];
	keyval[19] = 0;
}

#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif

#ifdef _WIN32
#define REG_KEY	  "Software\\GameSpy\\GameSpy 3D\\Registration"
#endif

const char * GOAGetUniqueID_Internal(void)
{
	static char keyval[PATH_MAX] = "";
	unsigned int ret;

	int docreate;

#ifdef _WIN32
	HKEY thekey;
	DWORD thetype = REG_SZ;
	DWORD len = MAX_PATH;
	DWORD disp;

	if (RegOpenKeyExA(HKEY_CURRENT_USER, REG_KEY, 0, KEY_ALL_ACCESS, &thekey) != ERROR_SUCCESS)
		docreate = 1;
	else
		docreate = 0;
	ret = RegQueryValueExA(thekey, (LPCSTR)"Crypt", 0, &thetype, (LPBYTE)keyval, &len);
#else
	FILE *f;
	f = fopen("id.bin","r");
	if (!f)
		ret = 0;
	else
	{
		ret = fread(keyval,1,19,f);
		keyval[ret] = 0;
		fclose(f);
	}
#endif

	if (ret != 0 || strlen(keyval) != 19)//need to generate a new key
	{
		GenerateID(keyval);
#ifdef _WIN32
		if (docreate)
		{
			ret = RegCreateKeyExA(HKEY_CURRENT_USER, REG_KEY, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &thekey, &disp);
		}
		RegSetValueExA(thekey, (LPCSTR)"Crypt", 0, REG_SZ, (const LPBYTE)keyval, strlen(keyval)+1);
#else
		f = fopen("id.bin","w");
		if (f)
		{
			fwrite(keyval,1,19,f);
			fclose(f);
		} else
			keyval[0] = 0; //don't generate one each time!!
#endif
	}

#ifdef _WIN32
	RegCloseKey(thekey);
#endif

	// Strip out the -'s.
	/////////////////////
	memmove(keyval + 4, keyval + 5, 4);
	memmove(keyval + 8, keyval + 10, 4);
	memmove(keyval + 12, keyval + 15, 4);
	keyval[16] = '\0';
	
	return keyval;
}

#endif

#if !defined(__mips64) || defined(UNIQUEID)
GetUniqueIDFunction GOAGetUniqueID = GOAGetUniqueID_Internal;
#endif

// Re-enable previously disabled compiler warnings
///////////////////////////////////////////////////
#if defined(_MSC_VER)
#pragma warning ( default: 4127 )
#endif // _MSC_VER
