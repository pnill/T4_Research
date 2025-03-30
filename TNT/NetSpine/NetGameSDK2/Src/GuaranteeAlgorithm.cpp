/**
* \file     GuaranteeAlgorithm.cpp
*
* \brief    
*
* \author   Zhi Chen
* \version  1.0
* \date		8/22/2002
*/

#include "x_files.hpp"
#include "MemoryStream.h"
#include "x_sock.h"
#include "GuaranteeAlgorithm.h"
#include "NS_DeviceManager.h"

#include "NetSimProtocol.h"

//Define to print out debug info
//#define GP_SHOW_DEBUG_INFO

//show guarantee message been sent and received.
//#define GP_SHOW_DEBUG_GUARANTEE


/**
* UDP message flags
*/
#define SENDFLAG_NONE			0x0000
#define SENDFLAG_RELIABLE		0x0001
//#define SENDFLAG_DATA			0x0002
#define SENDFLAG_ACK			0x0004
#define SENDFLAG_UNRELIABLE		0x0008
#define SENDFLAG_FIRST			0x0010

/**
* Garantee behavior parameters
*/
#define DEFAULT_RTT				(0xffffffff)     //ms - invalid
#define RESEND_BASE				(250)	 // ms
#define MAX_RESEND				(5)	 //count
#define IDLETIME_REMOVE			(300000) //ms


/**
* max size of the UDP message
*/
#define SENDMSG_MAX			(512)
#define SENDMSG_HEADERSIZE	(sizeof(u16)+sizeof(u32)+sizeof(u32)+sizeof(u16)+sizeof(u16))
#define SENDMSG_APPDATASIZE (SENDMSG_MAX-SENDMSG_HEADERSIZE)
#define	SENDMSG_VDP_HEADERSIZE (SENDMSG_HEADERSIZE - sizeof(u16))

extern xbool gbSendAcks;
extern xbool NSDM_IsConnected();

static s32 gCallCounter = 0;

/**
* reliable/unreliable message structure
*/
class UDP_MSG
{
public:
	u16 m_cbGameData;// used by Xbox VDP protocol.. holds the size of the game data which will be encrypted.  Zero if voice data.
	u32 dwSeqNumb;
	u32 dwTimeStamp;//used for rtt calculation
	u16 wFlag;
	u16 wDataLen; //limit to SENDMSG_APPDATASIZE
	u8 pData[SENDMSG_APPDATASIZE];

	UDP_MSG();
	const UDP_MSG& operator=(const UDP_MSG& o);
};

UDP_MSG::UDP_MSG()
{
	m_cbGameData = 0;
	dwSeqNumb = 0;
	dwTimeStamp = 0;
	wFlag = SENDFLAG_NONE;
	wDataLen = 0;
}

const UDP_MSG& UDP_MSG::operator=(const UDP_MSG& o)
{
	m_cbGameData = o.m_cbGameData;
	dwSeqNumb = o.dwSeqNumb;
	dwTimeStamp = o.dwTimeStamp;
	wFlag = o.wFlag;
	wDataLen = o.wDataLen;
	x_memcpy(pData, o.pData, SENDMSG_APPDATASIZE);
	return *this;
}

/**
*Context stat info
*/
class  CStat_Info
{
public:
	CStat_Info() :
	              dwTotalTxBytes(0),
	              dwTotalRxBytes(0),
	              dwTotalTimeElapsed(0),
	              dwAcculatedTxBytes(0),
	              dwAcculatedRxBytes(0),
	              dwLastTick(0)
	{
		UpdateTime(x_GetSystemUpTimeMs());
	}

	u32 GetTotalTxBytes() const {return dwTotalTxBytes;}
	u32 GetTotalRxBytes() const {return dwTotalRxBytes;}
	u32 GetTotalTxRxBytesLastSec()const {return GetPerSec(dwLastTxBytesPerSec+dwLastRxBytesPerSec, dwTotalTimeElapsed);}
	u32 GetTotalTxBytesLastSec() const {return GetPerSec(dwLastTxBytesPerSec, dwTotalTimeElapsed);}
	u32 GetTotalRxBytesLastSec() const {return GetPerSec(dwLastRxBytesPerSec, dwTotalTimeElapsed);}
	
	void AccumulateTx(u32 dwValue) {dwTotalTxBytes += dwValue; dwAcculatedTxBytes += dwValue;}
	void AccumulateRx(u32 dwValue) {dwTotalRxBytes += dwValue; dwAcculatedRxBytes += dwValue;}
	//must be called before calling any of the member funciton in order to obtain accurate info
	void UpdateTime(u32 dwCurrentTick)
	{
		//if time counter overlapped
		if(dwCurrentTick < dwLastTick)
			dwLastTick = dwCurrentTick;

		u32 dwDelta = dwCurrentTick - dwLastTick;
		if(dwDelta >= 1000)//1 second
		{
			//update values
			dwLastTxBytesPerSec = dwAcculatedTxBytes;
			dwLastRxBytesPerSec = dwAcculatedRxBytes;
			dwTotalTimeElapsed = dwDelta;
			
			//reset acculated values
			dwAcculatedTxBytes = 0;
			dwAcculatedRxBytes = 0;
			dwLastTick = dwCurrentTick;
		}
		//if not time is not elapsed yet, just ignore this call
	}
private:
	static u32 GetPerSec(u32 dwValue, u32 dwTotalTime) 
	{
		if(dwTotalTime == 0)
			return 0; 
		return (u32)(dwValue/((float)dwTotalTime/1000));
	}
	u32 dwTotalTxBytes;
	u32 dwTotalRxBytes;
	u32 dwLastTxBytesPerSec;
	u32 dwLastRxBytesPerSec;
	u32 dwTotalTimeElapsed; //in ms

	//need to reset these values when time is up.
	u32 dwAcculatedTxBytes;
	u32 dwAcculatedRxBytes;
	u32 dwLastTick;
};

/**
* Session to maintianing and deliverying reliable messages
*/
class CGUARANTEE_UDP_SESSION
{
public:
	CGUARANTEE_UDP_SESSION();
	virtual ~CGUARANTEE_UDP_SESSION();

	//destination address - key of this session
	u32 dwLastReliableSendTime;	//last reliable send time
	u32 dwLastReliableRecvTime; //last reliable message recv time
	u32	dwLastRTT; //last round trip time

	u32 dwSessionIdleSince;
	u32 dwReliableSeqNumbSent; //reliable sequence send
	u32 dwReliableSeqNumbAck; //reliable sequence ack - should be (dwReliableSeqNumbSent - 1) before acknowledgement

	u32 dwLastRecvReliableSeqNumb; //keep last received reliable sequence # for duplication process
	u32 dwWindowSize; // size of sliding window for acks

	u32 dwReliableReSendCount;  //base value is (1)
	u32 dwNextRetransmitTime; //dwReliableReSendCount * RESEND_BASE + dwLastReliableSendTime

	UDP_MSG *GetFirstMessage();
	UDP_MSG *GetFirstUnsentMessage();
	bool PopMessage(SLNKLSTPTR& list);
	bool PopFirstMessage();
	bool PopFirstUnsentMessage();
	bool PushMessage(UDP_MSG &rmessage);
	bool PushUnsentMessage(UDP_MSG &rmessage);
	bool IsEmpty();
	bool HasNoUnsentMsgs();
	SLNKLSTPTR UnackList;  // unack message container. Data in node will be UDP_MSG
	SLNKLSTPTR UnsentList; // like unack container, but contains messages that have yet to be sent (flow control)
};


CGUARANTEE_UDP_SESSION::CGUARANTEE_UDP_SESSION()
{
	dwLastReliableSendTime = 0;
	dwLastReliableRecvTime = 0;
	dwLastRTT = DEFAULT_RTT; 
	
	dwSessionIdleSince = x_GetSystemUpTimeMs(); //we need this time stamp, otherwise this session will be removed immediately
	dwReliableSeqNumbSent = 0;
	dwReliableSeqNumbAck = 0;	

	dwLastRecvReliableSeqNumb	= 1;
	dwWindowSize = 5; // start with a sliding window of 5 messages

	dwReliableReSendCount = 1;
	dwNextRetransmitTime = 0;
	//init list
	SLNKLSTPTR_Init(&UnackList);
	SLNKLSTPTR_Init(&UnsentList);
}
CGUARANTEE_UDP_SESSION::~CGUARANTEE_UDP_SESSION()
{
	//release all allocated 
	while(PopFirstMessage())
	{
		; //continue popping
	}		
	while(PopFirstUnsentMessage())
	{
		; // and keep popping here too
	}
	//deinit list
	SLNKLSTPTR_DeInit(&UnackList);
	SLNKLSTPTR_DeInit(&UnsentList);
}

bool CGUARANTEE_UDP_SESSION::IsEmpty()
{
	if(0 != SLNKLSTPTR_IsEmpty(&UnackList))//nonzero if empty;
		return true;
	return false;
}

bool CGUARANTEE_UDP_SESSION::HasNoUnsentMsgs()
{
	if(0 != SLNKLSTPTR_IsEmpty(&UnsentList))//nonzero if empty;
		return true;
	return false;
}

UDP_MSG *CGUARANTEE_UDP_SESSION::GetFirstMessage()
{
	SLSTNODEPTR* pNodePtr = SLNKLSTPTR_GetHeadNode  (&UnackList);
	if(NULL == pNodePtr)
		return NULL;
	return (UDP_MSG*)SLNKLSTPTR_GetData(&UnackList, pNodePtr );
}

UDP_MSG *CGUARANTEE_UDP_SESSION::GetFirstUnsentMessage()
{
	SLSTNODEPTR* pNodePtr = SLNKLSTPTR_GetHeadNode  (&UnsentList);
	if(NULL == pNodePtr)
		return NULL;
	return (UDP_MSG*)SLNKLSTPTR_GetData(&UnsentList, pNodePtr );
}

bool CGUARANTEE_UDP_SESSION::PopMessage(SLNKLSTPTR& list)
{
	SLSTNODEPTR* pNodePtr = SLNKLSTPTR_GetHeadNode(&list);
	if(NULL == pNodePtr)
		return false;

	//delete data and remove node
	delete (UDP_MSG*)SLNKLSTPTR_GetData(&list, pNodePtr );
	SLNKLSTPTR_RemoveNode(&list, pNodePtr);

	return true;

}
bool CGUARANTEE_UDP_SESSION::PopFirstUnsentMessage()
{
	return PopMessage(UnsentList);
}


bool CGUARANTEE_UDP_SESSION::PopFirstMessage()
{
	return PopMessage(UnackList);
}

bool CGUARANTEE_UDP_SESSION::PushMessage(UDP_MSG &rmessage)
{
	UDP_MSG *pMessage = new UDP_MSG;
	if(NULL == pMessage)
		return false;

	(*pMessage) = rmessage;
	ASSERT(pMessage->dwSeqNumb != 0);	
	SLNKLSTPTR_InsertAtTail_Slow( &UnackList , pMessage);

	return true;
}

bool CGUARANTEE_UDP_SESSION::PushUnsentMessage(UDP_MSG &rmessage)
{
	UDP_MSG *pMessage = new UDP_MSG;
	if(NULL == pMessage)
		return false;

	(*pMessage) = rmessage;
//	ASSERT(pMessage->dwSeqNumb != 0);	
	
	SLNKLSTPTR_InsertAtTail_Slow( &UnsentList , pMessage);

	return true;
}


class CNetSim : public CNetSimProtocol
{
public:
	CNetSim(X_SOCKET socket) : m_hSocket(socket){}

	//will update the time stam as well
	const CStat_Info &GetStatInfo()
	{
		//update the time
		m_StatInfo.UpdateTime(x_GetSystemUpTimeMs());
		return m_StatInfo;
	}
protected:
	//methods override
	//send data directly to the socket.
	//call TxBuffer() to forward data to NetSim for latency and packet drop simulation.
	virtual SOCKERRTYPE ITxTo(void *pData, u16 wLen, const struct X_SOCKADDRIN  &rDest_addr)
	{
		m_StatInfo.AccumulateTx(wLen);
		return x_sendto(m_hSocket, (char*)pData, wLen , 0/*flag*/, (struct X_SOCKADDR*)&rDest_addr, sizeof(struct X_SOCKADDR));
	}
	virtual SOCKERRTYPE IRxFrom(void *pData, u16 wLen, struct X_SOCKADDRIN  &rSource_addr)
	{

		X_SOCKLEN addrlen = SIZEOF_SOCKADDR_IN;
		SOCKERRTYPE sockettype = x_recvfrom (m_hSocket, (char*)pData, wLen, 0/*flag*/, (struct X_SOCKADDR*)&rSource_addr, &addrlen) ;
		if(sockettype != X_SOCKET_ERROR)
			m_StatInfo.AccumulateRx(sockettype);

		return  sockettype;
	}



	X_SOCKET m_hSocket;
private:
	CStat_Info m_StatInfo;
};

class CSocketContext
{
public:
	//with flag set to 1 means plugin NetSim protocol
	CSocketContext(X_SOCKET s, u32 dwFlag = 1);
	virtual ~CSocketContext();
	//send data through member socket.
	SOCKERRTYPE SendData(void *pData, u16 wLen, const struct X_SOCKADDRIN  &rDest_addr);
	SOCKERRTYPE ReceiveData(void *pData, u16 wLen, struct X_SOCKADDRIN  &rDest_addr);	
    void        SetCustomData( void *pCustomData ) { m_pCustomData = pCustomData ; }
    void*       GetCustomData( ) { return m_pCustomData ; }

	//will update the time stam as well
	//if Netsim is enable return the NetSim Stat otherwise return this stat
	const CStat_Info &GetStatInfo(); 

	//Buffer for I/O operation (read/write) - *********SHARE BUFFERS*******
	static u8 m_pInBuffer [SENDMSG_MAX]; //input buffer
	static u8 m_pOutBuffer[SENDMSG_MAX]; //output buffer
	
	//typedef std::map<struct X_SOCKADDRIN , CGUARANTEE_UDP_SESSION, ADDR_LESS> SESSIONMAP;

	X_SOCKET			m_hSocket;
	pfnGP_CONTEXT_CALLBACK	m_pFN;	

	typedef struct 
	{
		X_SOCKADDRIN		   m_SockAddrin;
		CGUARANTEE_UDP_SESSION m_Session;
	}SESSIONLIST_DATA;

	SESSIONLIST_DATA * FindUDPSession(X_SOCKADDRIN &rSockAddrin);
	//return newly created session
	SESSIONLIST_DATA * CreateUDPSession(X_SOCKADDRIN &rSockAddrin);
	//return true is session is found and deleted, otherwise false - not found
	bool DeleteUDPSession(X_SOCKADDRIN &rSockAddrin);
	void DeleteAllSession(void);

	SLNKLSTPTR m_SessionList; //data in the node will be SESSIONLIST_DATA
	CNetSim*		m_pNetSim;
protected:
    void *m_pCustomData ;
	CStat_Info m_StatInfo;
};

//initialize the static member buffer.
u8 CSocketContext::m_pInBuffer [SENDMSG_MAX] = {0};
u8 CSocketContext::m_pOutBuffer[SENDMSG_MAX] = {0};

//if equal, return true, otherwise false
static bool IsAddrEqual(X_SOCKADDRIN &x, X_SOCKADDRIN &y)
{
	return ((x.sin_port == y.sin_port) && (x.sin_addr.s_addr == y.sin_addr.s_addr));    
}

CSocketContext::CSocketContext(X_SOCKET s, u32 dwFlag):
m_hSocket(s), 
m_pFN(NULL),
m_pNetSim(NULL),
m_pCustomData( NULL )
{
	//init list
	SLNKLSTPTR_Init(&m_SessionList);

	if(dwFlag == 1)
	{
		m_pNetSim = new CNetSim(m_hSocket);
		if(m_pNetSim)
			m_pNetSim->StartNetSim();
		//use default attributes
	}
}

CSocketContext::~CSocketContext()
{
	//release all allocated resources
	DeleteAllSession();	

	//deinit list
	SLNKLSTPTR_DeInit(&m_SessionList);

	if(m_pNetSim)
		m_pNetSim->ShutdownNetSim();
	m_pNetSim = NULL;
}


CSocketContext::SESSIONLIST_DATA * CSocketContext::FindUDPSession(X_SOCKADDRIN &rSockAddrin)
{
	SLSTNODEPTR *pNodePtr = SLNKLSTPTR_GetHeadNode(&m_SessionList);
	if(NULL == pNodePtr)
		return NULL;

	SESSIONLIST_DATA *pNetSimData = NULL;
	do
	{
		pNetSimData = (CSocketContext::SESSIONLIST_DATA*)SLNKLSTPTR_GetData(&m_SessionList, pNodePtr );
		if(IsAddrEqual(pNetSimData->m_SockAddrin, rSockAddrin))
			return pNetSimData;

	}while(NULL != (pNodePtr = SLNKLSTPTR_GetNextNode(&m_SessionList, pNodePtr)));

	return NULL; //should be NULL at this point
}

CSocketContext::SESSIONLIST_DATA * CSocketContext::CreateUDPSession(X_SOCKADDRIN &rSockAddrin)
{
	SESSIONLIST_DATA *pSessionData = FindUDPSession(rSockAddrin);
	if(NULL == pSessionData)
	{
		//doesn't exist, create a new object.
		pSessionData = new SESSIONLIST_DATA;
		pSessionData->m_SockAddrin = rSockAddrin;

		//just insert at the head - faster
		SLNKLSTPTR_InsertAtHead(&m_SessionList, pSessionData);
	}

#if defined(GP_SHOW_DEBUG_INFO)
	x_printf("Creating UDP Session, %u\n", m_SessionList.dwNodeCount);
#endif

	return pSessionData;
}
bool CSocketContext::DeleteUDPSession(X_SOCKADDRIN &rSockAddrin)
{
#if defined(GP_SHOW_DEBUG_INFO)
	x_printf("Deleting UDP Session, %u\n", m_SessionList.dwNodeCount);
#endif

	SLSTNODEPTR *pNodePtr = SLNKLSTPTR_GetHeadNode(&m_SessionList);
	if(pNodePtr == NULL)
		return false;

	CSocketContext::SESSIONLIST_DATA *pNetSimData = NULL;
	do
	{
		pNetSimData = (CSocketContext::SESSIONLIST_DATA*)SLNKLSTPTR_GetData(&m_SessionList, pNodePtr );
        if( pNetSimData )
        {
		    if(IsAddrEqual(pNetSimData->m_SockAddrin, rSockAddrin))
		    {
			    delete pNetSimData;
			    pNetSimData = NULL;
			    SLNKLSTPTR_RemoveNode(&m_SessionList, pNodePtr);
			    return true;
		    }
        }
    } while(NULL != (pNodePtr = SLNKLSTPTR_GetNextNode(&m_SessionList, pNodePtr)));
	
	return false;
}

void CSocketContext::DeleteAllSession(void)
{
	SLSTNODEPTR *pNodePtr = SLNKLSTPTR_GetHeadNode(&m_SessionList);
	if(pNodePtr == NULL)
		return ;

	CSocketContext::SESSIONLIST_DATA *pNetSimData = NULL;
	do
	{
		pNetSimData = (CSocketContext::SESSIONLIST_DATA*)SLNKLSTPTR_GetData(&m_SessionList, pNodePtr );
        if( pNetSimData )
        {
		    delete pNetSimData;
		    pNetSimData = NULL;
        }
	}while(NULL != (pNodePtr = SLNKLSTPTR_GetNextNode(&m_SessionList, pNodePtr)));

    SLNKLSTPTR_RemoveAllNodes( &m_SessionList ) ;
}

SOCKERRTYPE CSocketContext::SendData(void *pData, u16 wLen, const struct X_SOCKADDRIN  &rDest_addr)
{
	if(m_pNetSim)
		return m_pNetSim->TxBuffer(pData, wLen, rDest_addr);
	else
	{
		m_StatInfo.AccumulateTx(wLen);
		return x_sendto(m_hSocket, (char*)pData, wLen , 0/*flag*/, (struct X_SOCKADDR*)&rDest_addr, sizeof(struct X_SOCKADDR));
	}
}
SOCKERRTYPE CSocketContext::ReceiveData(void *pData, u16 wLen, struct X_SOCKADDRIN  &rDest_addr)
{
	if(m_pNetSim)
		return m_pNetSim->RxBuffer(pData, wLen, rDest_addr);
	else
	{
		X_SOCKLEN addrlen = SIZEOF_SOCKADDR_IN;
		SOCKERRTYPE sockettype = x_recvfrom (m_hSocket, (char*)pData, wLen, XSOCK_RECV_PEEK, (struct X_SOCKADDR*)&rDest_addr, &addrlen) ;
		if(sockettype != X_SOCKET_ERROR)
			m_StatInfo.AccumulateRx(sockettype);
		return sockettype;
	}
}
const CStat_Info &CSocketContext::GetStatInfo()
{
	if(m_pNetSim)
		return m_pNetSim->GetStatInfo();

	//update the time
	m_StatInfo.UpdateTime(x_GetSystemUpTimeMs());
	return m_StatInfo;
}

GP_SOCKETCONTEXT GP_CreateSocketContext(X_SOCKET s, u32 dwFlag)
{
	return new CSocketContext(s, dwFlag);	
}
X_SOCKET GP_DestroySocketContext(GP_SOCKETCONTEXT hContext)
{
	CSocketContext *pObject = (CSocketContext*)hContext;
	X_SOCKET hSocket = pObject->m_hSocket;
	delete pObject;
	hContext = NULL;
	return hSocket;
}

X_SOCKET GP_GetSocketFromContext(GP_SOCKETCONTEXT hContext )
{
    CSocketContext *pObject = (CSocketContext*)hContext ;
    return pObject->m_hSocket ;
}

void  GP_SetSocketCustomData( GP_SOCKETCONTEXT hContext, void *pCustomData )
{
    CSocketContext *pObject = (CSocketContext*)hContext ;
    pObject->SetCustomData( pCustomData ) ;
}

void*  GP_GetSocketCustomData( GP_SOCKETCONTEXT hContext )
{
    CSocketContext *pObject = (CSocketContext*)hContext ;
    return pObject->GetCustomData( ) ;
}

void HandleSocketError(SOCKERRTYPE error, CSocketContext *sockPtr)
{
#if defined(TARGET_XBOX)
	if( (XNetGetEthernetLinkStatus() & XNET_ETHERNET_LINK_ACTIVE) == 0 ||
		X_ERRNO == WSAEHOSTUNREACH )
	{		
		gCallCounter++;
        if( sockPtr->m_pFN && gCallCounter <= 1 )
		{			
			sockPtr->m_pFN(GP_MESSAGE_UNREACHABLE, NULL, 0, sockPtr, NULL, NULL);
		}
		gCallCounter--;
	}
#elif defined( TARGET_PS2 )
	if( !NSDM_IsConnected() || X_ERRNO == EHOSTUNREACH )
	{
		gCallCounter++;
        if( sockPtr->m_pFN && gCallCounter <= 1 )
		{			
			sockPtr->m_pFN(GP_MESSAGE_UNREACHABLE, NULL, 0, sockPtr, NULL, NULL);
		}
		gCallCounter--;
	}
#endif
}


static /*inline */
SOCKERRTYPE SendMessage(CSocketContext &rSocketContext, const UDP_MSG &rmsg, const struct X_SOCKADDRIN &rdestaddr)
{
	x_mem_stream memstream;
	XSTREAM_Init( &memstream, CSocketContext::m_pOutBuffer, sizeof(CSocketContext::m_pOutBuffer), 0 );
	XSTREAM_Append_u16_noendian(&memstream, rmsg.m_cbGameData);
	XSTREAM_Append_u32(&memstream, rmsg.dwSeqNumb);
	XSTREAM_Append_u32(&memstream, rmsg.dwTimeStamp);
	XSTREAM_Append_u16(&memstream, rmsg.wFlag);
	XSTREAM_Append_u16(&memstream, rmsg.wDataLen);
	XSTREAM_Append_mem(&memstream, (void*)rmsg.pData, rmsg.wDataLen);

	char *pData = (char*)XSTREAM_GetStreamPtr(&memstream);
	u16  wlen   = (u16)XSTREAM_GetStreamDataSize(&memstream);

	return rSocketContext.SendData(pData, wlen, rdestaddr);
}

static /*inline */
SOCKERRTYPE SendMessage(CSocketContext &rSocketContext, CGUARANTEE_UDP_SESSION &rSession, UDP_MSG &rMessage, const struct X_SOCKADDRIN &rAddr)
{
	//update lastsendtime and nextsendtime

	//increment the sequenceNumb
	rSession.dwReliableSeqNumbSent ++;
	rMessage.dwSeqNumb = rSession.dwReliableSeqNumbSent;
	rMessage.dwTimeStamp = rSession.dwLastReliableSendTime;
	ASSERT(rMessage.dwSeqNumb != 0);
 
	SOCKERRTYPE rtn = SendMessage(rSocketContext, rMessage, rAddr);
	if( rtn != X_SOCKET_ERROR )
	{
		// TODO: need to handle time overlap  
		rSession.dwLastReliableSendTime = x_GetSystemUpTimeMs();

		rSession.dwSessionIdleSince = rSession.dwLastReliableSendTime;
		rSession.dwNextRetransmitTime = rSession.dwLastReliableSendTime + RESEND_BASE*rSession.dwReliableReSendCount;
	}
	else
	{
		rSession.dwReliableSeqNumbSent--;
	}

	return rtn;

}

/*
Return number of bytes received, otherwise return X_SOCKET_ERROR
*/
static inline
SOCKERRTYPE ReceiveMessage(CSocketContext &rSocketContext, UDP_MSG &rmsg, struct X_SOCKADDRIN &rsrcaddr)
{
	//try to recv a UDP message
	SOCKERRTYPE  ret = rSocketContext.ReceiveData((char*)CSocketContext::m_pInBuffer, sizeof(CSocketContext::m_pInBuffer), rsrcaddr) ;
	if( ret != X_SOCKET_ERROR )
	{
		//check header here
		if(ret < (s32)SENDMSG_HEADERSIZE)
		{
			return 0;//unrecognized message - message too short.
		}
		x_mem_stream memstream;
		XSTREAM_Init( &memstream, CSocketContext::m_pInBuffer, SENDMSG_MAX, ret );
		XSTREAM_Extract_u16(&memstream, &rmsg.m_cbGameData);
		XSTREAM_Extract_u32(&memstream, &rmsg.dwSeqNumb);
		XSTREAM_Extract_u32(&memstream, &rmsg.dwTimeStamp);
		XSTREAM_Extract_u16(&memstream, &rmsg.wFlag);
		XSTREAM_Extract_u16(&memstream, &rmsg.wDataLen);
		XSTREAM_Extract_mem(&memstream, rmsg.pData, rmsg.wDataLen);
	}
	return ret;
}


static inline 
SOCKERRTYPE SendAck(CSocketContext &rSocketContext, struct X_SOCKADDRIN &rDestaddr, const UDP_MSG &rmsg)
{
	UDP_MSG msg;
	msg.dwSeqNumb = rmsg.dwSeqNumb;
	msg.dwTimeStamp = rmsg.dwTimeStamp;
	msg.wFlag = SENDFLAG_ACK;
	msg.wDataLen = 0;
	msg.m_cbGameData = SENDMSG_VDP_HEADERSIZE;
#if defined (GP_SHOW_DEBUG_INFO)
	x_printf("Ack sent for sequence number %d\n", msg.dwSeqNumb);
#endif
	return SendMessage(rSocketContext, msg, rDestaddr);
}

s32 GP_SocketSendTo(GP_SOCKETCONTEXT hContext, const void *pData, u16 wLen, struct X_SOCKADDRIN  *pDest_addr, GP_SENDMSGFLAG eFlag)
{
	//parameters checking
	if(wLen > SENDMSG_APPDATASIZE)
	{
		return -1;
	}
	CSocketContext *pObject = (CSocketContext*)hContext;
	//msg object to send
	UDP_MSG msg;

	//if application just want unreliable send, just send it through.
	if (eFlag == eUnreliable) //sending unreliable message
	{	
		msg.dwSeqNumb = 0; //don't need seq #
		msg.wFlag = SENDFLAG_UNRELIABLE;
		msg.wDataLen = wLen;
		x_memcpy(msg.pData, pData, wLen);
		msg.m_cbGameData = SENDMSG_VDP_HEADERSIZE+wLen;

		//start writing out to udp layer
		if(X_SOCKET_ERROR != SendMessage(*pObject, msg, *pDest_addr))
			return wLen;
	}
	else //sending reliable message
	{
		if (eFlag == eVoice)
		{
			msg.m_cbGameData = 0;
			msg.dwSeqNumb = 0; //don't need seq #
			msg.wFlag = SENDFLAG_UNRELIABLE;
			msg.wDataLen = wLen;
			x_memcpy(msg.pData, pData, wLen);

			//start writing out to udp layer
			if(X_SOCKET_ERROR != SendMessage(*pObject, msg, *pDest_addr))
				return wLen;
		}
		else
		{
			//check for session existance
			//create session if neccessory
			CSocketContext::SESSIONLIST_DATA *pSessionData = pObject->FindUDPSession(*pDest_addr);
			if(NULL == pSessionData)
			{
				pSessionData = pObject->CreateUDPSession(*pDest_addr);
				msg.wFlag = SENDFLAG_FIRST;
			}
			msg.wFlag    |= SENDFLAG_RELIABLE;
			msg.wDataLen = wLen;
			x_memcpy(msg.pData, pData, wLen);
			msg.m_cbGameData = SENDMSG_VDP_HEADERSIZE+wLen;
			
			xbool bError = FALSE;
			CGUARANTEE_UDP_SESSION &rSession = pSessionData->m_Session;
			//prepare reliable message only when message is ready to go, otherwise, just place in the session unack list
			//if not waiting for acknowledgement, so send it
			xbool bSendingMessage = rSession.dwReliableSeqNumbSent <= rSession.dwReliableSeqNumbAck + rSession.dwWindowSize;
			if(bSendingMessage)
			{	
				//start writing out to udp layer and update session
				if(X_SOCKET_ERROR == SendMessage(*pObject, rSession, msg, *pDest_addr))
					bError = TRUE;
			}
			if(FALSE == bError)
			{
	#if defined (GP_SHOW_DEBUG_GUARANTEE)
				x_printf("Guarantee message been sent\n");
	#endif
				//place this message into the unack list - the end and waiting for acknowledgement.
				if(bSendingMessage) {
					rSession.PushMessage(msg);
	#if defined (GP_SHOW_DEBUG_INFO)
				x_printf("GP_SendMessage pushed unack message number %d, %d in queue\n", msg.dwSeqNumb, rSession.UnackList.dwNodeCount);
	#endif
				} else {
					rSession.PushUnsentMessage(msg);
				}
				return wLen;
			}
		}
	}
	// should only get here if send failed, all other returns are early exit
	// need to refactor this function so it's clear...
	HandleSocketError(X_SOCKET_ERROR, pObject);
	return -1;
}

void GP_SetSocketContextCallBack(GP_SOCKETCONTEXT hContext, pfnGP_CONTEXT_CALLBACK pfn)
{
	CSocketContext *pObject = (CSocketContext*)hContext;
	pObject->m_pFN = pfn;
}

pfnGP_CONTEXT_CALLBACK GP_GetSocketContextCallback( GP_SOCKETCONTEXT hContext )
{
    return ((CSocketContext*)hContext)->m_pFN ;
}

void GP_TickSocketContext(GP_SOCKETCONTEXT hContext, void *pContext )
{
	CSocketContext *pObject = (CSocketContext*)hContext;

	//do some netsim housekeeping work, this tick is independant of the latency or packet drop.
	if(pObject->m_pNetSim)
		pObject->m_pNetSim->Tick();

	//we need to pull up all messages and process them
	for(;;)
	{
		UDP_MSG msg;
		X_SOCKADDRIN srcaddr;
		SOCKERRTYPE  ret = ReceiveMessage(*pObject, msg, srcaddr);
		if(ret != X_SOCKET_ERROR)
		{
			//if ret is (0), either message is too short or g_pUDPsocket is close.
			if(ret == 0)
				break;

			//if unreliable data, just forward it to application
			if(msg.wFlag & SENDFLAG_UNRELIABLE )
			{
				if(pObject->m_pFN)
					pObject->m_pFN(GP_MESSAGE_DATA, msg.pData, msg.wDataLen, hContext, &srcaddr, pContext );
			}
			//if this is a reliable message, we should send an acknowledgement to sender,
			//this message maybe duplicated
			else if(msg.wFlag & SENDFLAG_RELIABLE )
			{
				xbool bWrong = FALSE;
				//so this is a reliable message, if the sequence number is not what we expected, drop it.

				CSocketContext::SESSIONLIST_DATA *pSessionData = pObject->FindUDPSession(srcaddr);
				if((NULL == pSessionData) && !(msg.wFlag & SENDFLAG_FIRST))


				{
					//something is wrong here, don't echo this message
					bWrong = TRUE;
				}
				else
				{
					//just send it out once, if for some reason that message got dropped on the wayback, the send will resend.
					if (msg.dwSeqNumb == 0)
						ASSERT(0);
					
					if( gbSendAcks )
					{
						HandleSocketError(SendAck(*pObject, srcaddr, msg), pObject);
					}
				}
				if(FALSE == bWrong)
				{
					if((NULL == pSessionData) || (msg.wFlag & SENDFLAG_FIRST))
					{
						//if session exist but first message, means that session has restarted.
						if(msg.wFlag & SENDFLAG_FIRST)
							pObject->DeleteUDPSession(srcaddr);

						pSessionData = pObject->CreateUDPSession(srcaddr);
					}

					CGUARANTEE_UDP_SESSION &rSession = pSessionData->m_Session;	


					if((msg.dwSeqNumb == rSession.dwLastRecvReliableSeqNumb) && pObject->m_pFN)
					{
						//increment this number
						rSession.dwLastRecvReliableSeqNumb ++;
						pObject->m_pFN(GP_MESSAGE_DATA, msg.pData, msg.wDataLen, hContext, &srcaddr, pContext );
					}
					else//sequence doesn't match. possibly duplicated or sender resend the message.
					{
#if defined(GP_SHOW_DEBUG_INFO)
						x_printf("Expected Seq. %u, arrived Seq. %u\n", rSession.dwLastRecvReliableSeqNumb, msg.dwSeqNumb);
#endif
					}
				}
				//else - the sequence number don't match. ignore this message, possibily a duplicated message.
			}
			//if ack message, we need to update seq#, let app know about it.
			else if(msg.wFlag & SENDFLAG_ACK) 
			{
				//only process this message if we can find it in our session map
				CSocketContext::SESSIONLIST_DATA *pSessionData = pObject->FindUDPSession(srcaddr);
				if(NULL != pSessionData)
				{
					CGUARANTEE_UDP_SESSION &rSession = pSessionData->m_Session;
					//we only update the session in the following condition
					//msg.dwSeqNumb and rSession.dwReliableSeqNumbSent matches &&
					//!rSession.UnackList.empty() &&
					//rSession.UnackList.begin()->dwSeqNumb and msg.dwSeqNumb matches
					//otherwise, ignore this message
					if(	( msg.dwSeqNumb <= rSession.dwReliableSeqNumbSent ) && 
						( !rSession.IsEmpty() ) &&
						( rSession.GetFirstMessage()->dwSeqNumb == msg.dwSeqNumb) &&
						( rSession.dwReliableSeqNumbAck < msg.dwSeqNumb ) )
					{
#if defined (GP_SHOW_DEBUG_GUARANTEE)
						x_printf("Guarantee message been acked\n");
#endif
						rSession.dwReliableSeqNumbAck = msg.dwSeqNumb;
						rSession.dwReliableReSendCount = 1;

                        rSession.dwLastReliableRecvTime = x_GetSystemUpTimeMs();

						if(msg.dwTimeStamp == rSession.dwLastReliableSendTime)//only update RTT when the "right" packet sequence
							rSession.dwLastRTT = rSession.dwLastReliableRecvTime - rSession.dwLastReliableSendTime;

						//we need to remove the message from unacklist - the first one and let the app know

#if defined(GP_SHOW_DEBUG_INFO)
						x_printf("Before the pop: %d  size: %d\n",rSession.GetFirstMessage()->dwSeqNumb,rSession.UnackList.dwNodeCount);
						x_printf("Popping unack message, sequence number %d\n", msg.dwSeqNumb);
#endif
						rSession.PopFirstMessage();
#if defined(GP_SHOW_DEBUG_INFO)
						if (!rSession.IsEmpty())
						{
							x_printf("After the pop: %d  size: %d\n",rSession.GetFirstMessage()->dwSeqNumb,rSession.UnackList.dwNodeCount);
						}
						else {
							x_printf("After the pop: NOTHING IN UNACK LIST!\n");
						}

#endif

						if(!rSession.HasNoUnsentMsgs())
						{
							UDP_MSG rmsg =	*rSession.GetFirstUnsentMessage();
							rSession.PopFirstUnsentMessage();

							SLSTNODEPTR *pNodePtr = SLNKLSTPTR_GetHeadNode(&pObject->m_SessionList);
							CSocketContext::SESSIONLIST_DATA* nodeVal;
							nodeVal = (CSocketContext::SESSIONLIST_DATA*)SLNKLSTPTR_GetData(&pObject->m_SessionList, pNodePtr);

							while((&(nodeVal->m_Session) != &rSession) && (pNodePtr != SLNKLSTPTR_GetTailNode_Slow(&pObject->m_SessionList)))
							{
								pNodePtr = SLNKLSTPTR_GetNextNode(&pObject->m_SessionList, pNodePtr);
								nodeVal = (CSocketContext::SESSIONLIST_DATA*)SLNKLSTPTR_GetData(&pObject->m_SessionList, pNodePtr);
							}
							ASSERT(&(nodeVal->m_Session) == &rSession);
							HandleSocketError(SendMessage(*pObject, rSession, rmsg, nodeVal->m_SockAddrin), pObject);
							rSession.PushMessage(rmsg);
#if defined (GP_SHOW_DEBUG_INFO)
							x_printf("Received ack, clearing queue space.  Pushing %d to unack queue\n", rmsg.dwSeqNumb);
#endif
						}

						if(pObject->m_pFN)
							pObject->m_pFN(GP_MESSAGE_ACK, NULL, 0, hContext, &srcaddr, pContext );
					}
					else {
#if defined(GP_SHOW_DEBUG_INFO)
						x_printf("Discarded ack for sequence number %d\n", msg.dwSeqNumb);
						if( msg.dwSeqNumb > rSession.dwReliableSeqNumbSent) {
							x_printf("  Reason: sequence number %d exceeds last sequence number sent: %d\n",
								msg.dwSeqNumb, rSession.dwReliableSeqNumbSent);
						} else if(rSession.IsEmpty()) {
							x_printf("  Reason: no unacked messages found\n");
						} else if(rSession.GetFirstMessage()->dwSeqNumb != msg.dwSeqNumb) {
							x_printf("  Reason: expected ack for %d, received for %d instead\n", 
								rSession.GetFirstMessage()->dwSeqNumb, msg.dwSeqNumb);
						} else if(rSession.dwReliableSeqNumbAck >= msg.dwSeqNumb) {
							x_printf("  Reason: expected minimum sequence number was %d, received %d\n",
								rSession.dwReliableSeqNumbAck, msg.dwSeqNumb);
						} else {
							ASSERT(!"Escaped from if by mistake.  BROKEN!");
						}
#endif
					}
				}
			}
			//ignore any other unknown messages
		}
		else if (X_ERRNO == EWOULDBLOCK)
		{
			break;
		}
		else if( ret == X_SOCKET_ERROR )
		{
			char errstr[80];
			x_sprintf(errstr, "recv() returned error %d\n", X_ERRNO);
			ASSERTS( false, errstr );
			break;
		}
	}
	//go through each session in the list,
	//if no session the list, just return,
	SLSTNODEPTR *pNodePtr = SLNKLSTPTR_GetHeadNode(&pObject->m_SessionList);
	if(NULL == pNodePtr)
		return ;

	for(;NULL != pNodePtr;)
	{
		CSocketContext::SESSIONLIST_DATA *pData = (CSocketContext::SESSIONLIST_DATA*)SLNKLSTPTR_GetData(&pObject->m_SessionList, pNodePtr );
		const struct X_SOCKADDRIN &rAddr    = pData->m_SockAddrin;
		CGUARANTEE_UDP_SESSION	 &rSession = pData->m_Session;

		//check to see if it's ready to send next reliable message (if there is one)
		if(rSession.dwReliableSeqNumbSent == rSession.dwReliableSeqNumbAck)
		{
			//check to see if any other message waiting to be send.
			if(!rSession.HasNoUnsentMsgs())
			{
				UDP_MSG rmsg =	*rSession.GetFirstUnsentMessage();
				rSession.PopFirstUnsentMessage();
				HandleSocketError(SendMessage(*pObject, rSession, rmsg, rAddr), pObject);
				rSession.PushMessage(rmsg);
#if defined (GP_SHOW_DEBUG_INFO)
				x_printf("Pushed message, sequence #%d, onto unack list from unsent list\n", rmsg.dwSeqNumb);
				ASSERT(rSession.UnackList.dwNodeCount == 1);
#endif
				rSession.dwSessionIdleSince = rSession.dwLastReliableSendTime; //new idle time stamp

				pNodePtr = SLNKLSTPTR_GetNextNode(&pObject->m_SessionList, pNodePtr );
			}
			//if this session idle for IDLETIME_REMOVE, then remove this session.
            else if(x_GetSystemUpTimeMs() - rSession.dwSessionIdleSince >= IDLETIME_REMOVE)
			{
				CSocketContext::SESSIONLIST_DATA *p = (CSocketContext::SESSIONLIST_DATA*)SLNKLSTPTR_GetData(&pObject->m_SessionList, pNodePtr );
				delete p;

				SLSTNODEPTR *pTemp = SLNKLSTPTR_GetNextNode(&pObject->m_SessionList, pNodePtr);
				SLNKLSTPTR_RemoveNode(&pObject->m_SessionList, pNodePtr);
				pNodePtr = pTemp;
			}
			else
				pNodePtr = SLNKLSTPTR_GetNextNode(&pObject->m_SessionList, pNodePtr);
		}
		else //sequence # don't match (message waiting for acknowledgement)
		{
            u32 dwCurrentTick = x_GetSystemUpTimeMs();

			if(dwCurrentTick >= rSession.dwNextRetransmitTime)
			{
				//check max sent
				//also check for hard disconnect; if so, hack cough and die here rather than trying to retransmit and failing.
				if(rSession.dwReliableReSendCount >= MAX_RESEND || !NSDM_IsConnected())
				{
					//having a problem sending message to destination, destroy session and let the application know.
					if(pObject->m_pFN)
						pObject->m_pFN(GP_MESSAGE_UNREACHABLE, NULL, 0, hContext, &rAddr, pContext );
					
					CSocketContext::SESSIONLIST_DATA *p = (CSocketContext::SESSIONLIST_DATA*)SLNKLSTPTR_GetData(&pObject->m_SessionList, pNodePtr );
					delete p;
					SLSTNODEPTR *pTemp = SLNKLSTPTR_GetNextNode(&pObject->m_SessionList, pNodePtr);
					SLNKLSTPTR_RemoveNode(&pObject->m_SessionList, pNodePtr);
					pNodePtr = pTemp;
					continue; //process next session.
				}
				if(!rSession.IsEmpty())
				{
					SLSTNODEPTR* pNodePtr = SLNKLSTPTR_GetHeadNode (&rSession.UnackList);
					UDP_MSG rmsg = *((UDP_MSG*)SLNKLSTPTR_GetData(&rSession.UnackList, pNodePtr));
				
					const u32 initialSeq = rSession.GetFirstMessage()->dwSeqNumb;
					const u32 initialNodeCount = rSession.UnackList.dwNodeCount;

					//time to retransmit the entire window worth of packets
					for (u32 i=0; ( (i <rSession.UnackList.dwNodeCount) && (i < rSession.dwWindowSize) ); i++)
					{
						ASSERT(rmsg.dwSeqNumb != 0);
						SendMessage(*pObject, rmsg, rAddr);
	#if defined(GP_SHOW_DEBUG_INFO)
						x_printf("Retransmit Seq %u to [%s:%u]\n", rmsg.dwSeqNumb, x_inet_ntoa(rAddr.sin_addr), x_ntohs(rAddr.sin_port));
	#endif
						pNodePtr = SLNKLSTPTR_GetNextNode(&rSession.UnackList, pNodePtr);
						if(pNodePtr) {
							rmsg = *((UDP_MSG*)SLNKLSTPTR_GetData(&rSession.UnackList, pNodePtr)); 
						}
						//update session information 
						rSession.dwLastReliableSendTime = x_GetSystemUpTimeMs();				

						rSession.dwNextRetransmitTime = rSession.dwLastReliableSendTime + RESEND_BASE*rSession.dwReliableReSendCount;
					}

					ASSERT (initialSeq == rSession.GetFirstMessage()->dwSeqNumb);
					ASSERT (initialNodeCount == rSession.UnackList.dwNodeCount);

					rSession.dwReliableReSendCount ++;
				}
			}
			pNodePtr = SLNKLSTPTR_GetNextNode(&pObject->m_SessionList, pNodePtr);
		}

	}
}

void GP_GetSocketContextStatInfo(GP_SOCKETCONTEXT hContext, GP_SOCKETCONTEXT_STAT_INFO *pStatInfo)
{
	CSocketContext *pObject = (CSocketContext*)hContext;

	const CStat_Info &statInfo = pObject->GetStatInfo();

	pStatInfo->dwTotalTxBytes = statInfo.GetTotalTxBytes();
	pStatInfo->dwTotalRxBytes = statInfo.GetTotalRxBytes();
	pStatInfo->dwTotalTxRxBytesPerSec = statInfo.GetTotalTxRxBytesLastSec();
	pStatInfo->dwTotalTxBytesPerSec = statInfo.GetTotalTxBytesLastSec();
	pStatInfo->dwTotalRxBytesPerSec = statInfo.GetTotalRxBytesLastSec();
	return ;
}

void GP_SetLatency( GP_SOCKETCONTEXT hContext, u32 dwLatency )
{
    CSocketContext *pSock = (CSocketContext*)hContext ;
    if( pSock->m_pNetSim )
        pSock->m_pNetSim->SetLatency( dwLatency ) ;
}

void GP_SetDropRate( GP_SOCKETCONTEXT hContext, u32 dwDropRate )
{
    CSocketContext *pSock = (CSocketContext*)hContext ;
    if( pSock->m_pNetSim )
        pSock->m_pNetSim->SetDropRate( dwDropRate ) ;
}

