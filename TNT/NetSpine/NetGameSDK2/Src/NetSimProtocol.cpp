/**
* \file     NetSimProtocol.cpp
*
* \brief    
*
* \author   Zhi Chen
* \version  1.0
* \date		8/23/2002
*/

#include "x_files.hpp"
#include "CXSocket.h"
#include "GenLnkLst.h"
#include "NetSimProtocol.h"


CNetSimSession::CNetSimSession() : 
m_dwDropRate(DEFAULT_DROPRATE),
m_dwNextDropCount(DEFAULT_NEXTDROPCOUNTCOUNT),
m_dwDuplicateRate(DEFAULT_DUPLICATEPACKET)
{
	//init list
	SLNKLSTPTR_Init(&m_ToSendList);
	SetDropRate(m_dwDropRate);
	SetDuplicatePacket(m_dwDuplicateRate);
}
CNetSimSession::~CNetSimSession()
{
	//deinit list
	SLNKLSTPTR_DeInit(&m_ToSendList);
}



//void CNetSimSession::InitDropFlags(void)
//{
//	for(u32 i=0; i<m_dwElements; i++)
//		m_bDropFlags[i] = false;
//}

void CNetSimSession::SetDropRate(u32 dwdroprate)
{
	if(dwdroprate > 100)
		dwdroprate = 100;

	m_dwDropRate = dwdroprate;

	//reinitialize internal data
	InitRandomness(m_bDropFlags, m_dwElements, &m_dwDropIndex, m_dwDropRate);
}

//void CNetSimSession::InitDropRandomness(void)
//{
//	u32 dwCount = 0;
//	u32 dwRandomIndex = 0;
//
//	//reset all flags to false
//	InitDropFlags();
//
//	m_dwDropIndex = 0;//reset to 0 index
//	//randomly select the index to drop.
//	for(u32 i = 0; i < m_dwDropRate;)
//	{
//		dwRandomIndex = ((u32)x_rand())%m_dwElements;
//		if(false == m_bDropFlags[dwRandomIndex])
//		{
//			m_bDropFlags[dwRandomIndex] = true;
//			i++;
//		}
//	}
//}

void CNetSimSession::InitRandomness(bool *aFlags, u32 dwCount, u32 *pdwIndex, u32 dwPercent)
{
	u32 dwRandomIndex = 0;

	//reset all flags to false
	InitFlags(aFlags, dwCount, false);

	*pdwIndex = 0;//reset to 0 index
	//randomly select the index to drop.
	for(u32 i = 0; i < dwPercent;)
	{
		dwRandomIndex = ((u32)x_rand())%dwCount;
		if(false == aFlags[dwRandomIndex])
		{
			aFlags[dwRandomIndex] = true;
			i++;
		}
	}
}
void CNetSimSession::InitFlags(bool *aFlags, u32 dwCount, bool bValue)
{
	for(u32 i=0; i<dwCount; i++)
		aFlags[i] = bValue;
}


bool CNetSimSession::DropThisPacket(void)
{
	//if reaching the m_dwElements count, reset to 0 index
	if(!(m_dwDropIndex < m_dwElements))
		InitRandomness(m_bDropFlags, m_dwElements, &m_dwDropIndex, m_dwDropRate);

	if(m_dwNextDropCount == 0)
		return m_bDropFlags[m_dwDropIndex++];

	//user request specific drop count
	m_dwNextDropCount--;
	return true;
}

void CNetSimSession::SetDuplicatePacket(u32 dwDuplicate)
{
	if(dwDuplicate > 100)
		dwDuplicate = 100;

	m_dwDuplicateRate = dwDuplicate;

	//reinitialize internal data
	InitRandomness(m_bDuplicateFlags, m_dwElements, &m_dwDuplicateIndex, m_dwDuplicateRate);
}

bool CNetSimSession::DuplicateThisPacket(void)
{
	//if reaching the m_dwElements count, reset to 0 index
	if(!(m_dwDuplicateIndex < m_dwElements))
		InitRandomness(m_bDuplicateFlags, m_dwElements, &m_dwDuplicateIndex, m_dwDuplicateRate);

	return m_bDuplicateFlags[m_dwDuplicateIndex++];
}

void CNetSimSession::SetNextDropCount(u32 dwNextdropCount)
{
	m_dwNextDropCount = dwNextdropCount;
}

u32 CNetSimSession::GetMessageCount()
{
	u32 dwCount;
	SLNKLSTPTR_GetCount(&m_ToSendList, &dwCount); 
	return dwCount;
}

CNetSimSession::MESSAGE &CNetSimSession::GetFirstMessage(void)
{
	SLSTNODEPTR *pNodePtr = SLNKLSTPTR_GetHeadNode(&m_ToSendList);
	MESSAGE *pMessage = (MESSAGE*)SLNKLSTPTR_GetData(&m_ToSendList, pNodePtr );
	return *pMessage;
	//???may be we should return a pointer as oppose to a reference.
}

bool CNetSimSession::PopFirstMessage(void)
{
	SLSTNODEPTR *pNodePtr = SLNKLSTPTR_GetHeadNode(&m_ToSendList);
	if(NULL == pNodePtr)
		return false;

	//about to remove this node, delete data
	delete (MESSAGE*)SLNKLSTPTR_GetData(&m_ToSendList, pNodePtr );
	SLNKLSTPTR_RemoveNode(&m_ToSendList, pNodePtr);

	return true;
}

void CNetSimSession::PushMessage(u8 *pData, u16 wLen, u32 dwLatency)
{
	//copy the send buffer to the session for later process.
	MESSAGE *pMessage = new MESSAGE;

	x_memcpy(pMessage->pData, pData, wLen);
	pMessage->wlen = wLen;
	pMessage->dwQueuedTime = x_GetSystemUpTimeMs();

	//MUST push it to the end. this is a FIFO
	SLNKLSTPTR_InsertAtTail_Slow(&m_ToSendList, pMessage);
}

void CNetSimProtocol::ConfigCxnCallback(u32 dwmsg, const void *pData, u16 wLen, GP_SOCKETCONTEXT hContext, const struct X_SOCKADDRIN *pAddr, void *pContext)
{
	if(dwmsg == GP_MESSAGE_DATA) 
	{
		CNetSimProtocol *pthis = (CNetSimProtocol*)pContext;
		if(wLen%(sizeof(u32)*2)) //must be Mod of 8 bytes
			return ;

		u32 dwParamCount = wLen/(sizeof(u32)*2);
		u32 *pdwData = (u32 *)pData;
		for(u32 i=0; i<dwParamCount ; i++, pdwData+= 2)
		{
			u32 dwParamID = pdwData[0];

			if(dwParamID == DROPRATE_PARAM_ID)
				pthis->SetDropRate(pdwData[1]);
			else if(dwParamID == LATENCY_PARAM_ID)
				pthis->SetLatency(pdwData[1]);
			else if(dwParamID == BANDWIDTH_PARAM_ID)
				pthis->SetBandWidth(pdwData[1]);
			else if(dwParamID == NEXTDROPCOUNT_PARAM_ID)
				pthis->SetNextDropCount(pdwData[1]);
			else if(dwParamID == DUPLICATEPACKET_PARAM_ID)
				pthis->SetDuplicatePacket(pdwData[1]);			
		}
	}
}

CNetSimProtocol::CNetSimProtocol(): 
m_dwLatency(DEFAULT_LATENCY), 
m_dwBandWidth(DEFAULT_BANDWIDTH),
m_dwDropRate(DEFAULT_DROPRATE)
{	
	//init NetSim session list
	SLNKLSTPTR_Init(&m_NetSimSessionList);
	m_ConfigCxn = NULL;
}

CNetSimProtocol::~CNetSimProtocol()
{
	//deinit NetSim Session list
	SLNKLSTPTR_DeInit(&m_NetSimSessionList);
	ShutdownNetSim();
}

void CNetSimProtocol::SetDropRate(u32 dwdroprate)
{	
	m_dwDropRate = dwdroprate;
	SLSTNODEPTR *pNodePtr = SLNKLSTPTR_GetHeadNode(&m_NetSimSessionList);
	if(pNodePtr == NULL)
		return;
	do
	{
		NETSIMSESSIONLIST_DATA *pNetSimData = (NETSIMSESSIONLIST_DATA*)SLNKLSTPTR_GetData(&m_NetSimSessionList, pNodePtr);
		//update all the droprate of items in the map
		pNetSimData->m_NetSimSession.SetDropRate(m_dwDropRate);	
	}while(NULL != (pNodePtr = SLNKLSTPTR_GetNextNode(&m_NetSimSessionList, pNodePtr)));
}

void CNetSimProtocol::SetDuplicatePacket(u32 dwDuplicate)
{
	m_dwDuplicate = dwDuplicate;
	SLSTNODEPTR *pNodePtr = SLNKLSTPTR_GetHeadNode(&m_NetSimSessionList);
	if(pNodePtr == NULL)
		return;
	do
	{
		NETSIMSESSIONLIST_DATA *pNetSimData = (NETSIMSESSIONLIST_DATA*)SLNKLSTPTR_GetData(&m_NetSimSessionList, pNodePtr);
		//update all the droprate of items in the map
		pNetSimData->m_NetSimSession.SetDuplicatePacket(m_dwDuplicate);	
	}while(NULL != (pNodePtr = SLNKLSTPTR_GetNextNode(&m_NetSimSessionList, pNodePtr)));
}


void CNetSimProtocol::SetLatency  (u32 dwlatency)
{
	m_dwLatency = dwlatency;
}
void CNetSimProtocol::SetBandWidth(u32 dwbandwidth)
{
	m_dwBandWidth = dwbandwidth;
}
void CNetSimProtocol::SetNextDropCount(u32 dwNextdropCount)
{
	SLSTNODEPTR *pNodePtr = SLNKLSTPTR_GetHeadNode(&m_NetSimSessionList);
	if(pNodePtr == NULL)
		return;
	do
	{
		NETSIMSESSIONLIST_DATA *pNetSimData = (NETSIMSESSIONLIST_DATA*)SLNKLSTPTR_GetData(&m_NetSimSessionList, pNodePtr);
		//update all the droprate of items in the map
		pNetSimData->m_NetSimSession.SetNextDropCount(dwNextdropCount);	
	}while(NULL != (pNodePtr = SLNKLSTPTR_GetNextNode(&m_NetSimSessionList, pNodePtr)));
}


bool CNetSimProtocol::StartNetSim()
{
#if defined(TARGET_XBOX)
		OutputDebugString("Start Netspine NetSim...\n");
#endif

	//start up the configuration object;
	CXSocket csocket;
	if(false == csocket.Create(SOCK_DGRAM, "", DEFAULT_NETSIM_PORT ))
	{
#if defined(TARGET_XBOX)
		OutputDebugString("CNetSimProtocol::StartNetSim() failed to start up\n");
#endif
		return false;
	}
	csocket.SetToNoBlocking();
	m_ConfigCxn = GP_CreateSocketContext(csocket.Detach(), 0);
	if(m_ConfigCxn == NULL)
		return false;
	GP_SetSocketContextCallBack(m_ConfigCxn, ConfigCxnCallback);

	return true;
}

void CNetSimProtocol::ShutdownNetSim()
{
#if defined(TARGET_XBOX)
		OutputDebugString("Shutdown Netspine NetSim...\n");
#endif

	if(m_ConfigCxn)
	{
		//destroy the context and sockt associated with it
		X_SOCKET socket = GP_DestroySocketContext(m_ConfigCxn);
		CXSocket csocket;
		csocket.Attach(socket);
		m_ConfigCxn = NULL;
	}
}

//if equal, return true, otherwise false
static bool IsAddrEqual(const struct X_SOCKADDRIN &x, const struct X_SOCKADDRIN &y)
{
	return ((x.sin_port == y.sin_port) && (x.sin_addr.s_addr == y.sin_addr.s_addr));    
}
//if found return pointer, otherwise NULL
static CNetSimProtocol::NETSIMSESSIONLIST_DATA *FindNetSimSession(SLNKLSTPTR &rNetSimSessionList, const struct X_SOCKADDRIN &rSockAddrin)
{
	SLSTNODEPTR *pNodePtr = SLNKLSTPTR_GetHeadNode(&rNetSimSessionList);
	if(pNodePtr == NULL)
		return NULL;
	CNetSimProtocol::NETSIMSESSIONLIST_DATA *pNetSimData = NULL;
	do
	{
		pNetSimData = (CNetSimProtocol::NETSIMSESSIONLIST_DATA*)SLNKLSTPTR_GetData(&rNetSimSessionList, pNodePtr );
		if(IsAddrEqual(pNetSimData->m_SockAddrin, rSockAddrin))
			return pNetSimData;
	}while(NULL != (pNodePtr = SLNKLSTPTR_GetNextNode(&rNetSimSessionList, pNodePtr)));
	
	return pNetSimData; //should be NULL at this point
}

SOCKERRTYPE CNetSimProtocol::TxBuffer(void *pData, u16 wLen, const struct X_SOCKADDRIN  &rDest_addr)
{	
	NETSIMSESSIONLIST_DATA *pNetSimData = FindNetSimSession(m_NetSimSessionList, rDest_addr);
	if(NULL == pNetSimData)
	{
		//doesn't exist, create a new object.
		pNetSimData = new NETSIMSESSIONLIST_DATA;
		pNetSimData->m_SockAddrin = rDest_addr;
		pNetSimData->m_NetSimSession.SetDropRate(m_dwDropRate);

		//just insert at the head - faster
		SLNKLSTPTR_InsertAtHead(&m_NetSimSessionList, pNetSimData);
	}
	pNetSimData->m_NetSimSession.PushMessage((u8*)pData, wLen, m_dwLatency);

	return (wLen);//success
}

SOCKERRTYPE CNetSimProtocol::RxBuffer(void *pData, u16 wLen, struct X_SOCKADDRIN  &rSource_addr)
{
	return IRxFrom(pData, wLen, rSource_addr);
}

void CNetSimProtocol::Tick(void)
{
	DoTick();
	//tick the configuration cxn as well
	GP_TickSocketContext(m_ConfigCxn, this);
}

int CNetSimProtocol::DoTick()
{
//	if((dwCurrentTick - m_dwLastTimeTick) >= m_dwLatency)//if time to send the next packet, iterate through the NetSim list
	{

//#if defined(TARGET_XBOX)
//		{
//			static int i = 0;
//			if(i++>10)
//			{
//				i = 0;
//				char pText[64] = {0};
//				wsprintf(pText, "Latency: [%u]\n", m_dwLatency);
//				OutputDebugString(pText);
//			}
//		}
//
//#endif

		SLSTNODEPTR *pNodePtr = SLNKLSTPTR_GetHeadNode(&m_NetSimSessionList);
		if(NULL == pNodePtr)
			return (0);
		NETSIMSESSIONLIST_DATA *pData = NULL;
		do
		{
			pData = (NETSIMSESSIONLIST_DATA*)SLNKLSTPTR_GetData(&m_NetSimSessionList, pNodePtr );

			CNetSimSession &rNetSimSession = pData->m_NetSimSession;
			const X_SOCKADDRIN   &rDestAddr = pData->m_SockAddrin;
			const u32 nMessageCount = rNetSimSession.GetMessageCount();
			for(u32 i=0; i<nMessageCount; i++)
			{
				//get a reference to the first message
				CNetSimSession::MESSAGE &rMessage = rNetSimSession.GetFirstMessage();
				if(m_dwLatency == 0)//if no latency just continue pumping all messages
				{
					//send out this message, remove it from the list
					if(false == rNetSimSession.DropThisPacket())
					{
						ITxTo(rMessage.pData, rMessage.wlen, rDestAddr);
						//duplicate this packet??
						if(true == rNetSimSession.DuplicateThisPacket())
						ITxTo(rMessage.pData, rMessage.wlen, rDestAddr);
					}
					rNetSimSession.PopFirstMessage();

					continue;
				}
				if(x_GetSystemUpTimeMs() >= (rMessage.dwQueuedTime + m_dwLatency))
				{
					//send out this message, remove it from the list
					if(false == rNetSimSession.DropThisPacket())
					{
						ITxTo(rMessage.pData, rMessage.wlen, rDestAddr);
						//duplicate this packet??
						if(true == rNetSimSession.DuplicateThisPacket())
						ITxTo(rMessage.pData, rMessage.wlen, rDestAddr);
					}
					rNetSimSession.PopFirstMessage();

					continue;
				}
				else//latency is applied and not time to send it yet.
					break;

			}
		}while(NULL != (pNodePtr = SLNKLSTPTR_GetNextNode(&m_NetSimSessionList, pNodePtr)));
	}
	return (0); //don't really want to return any errors.
}
