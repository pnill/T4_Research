/**
* \file     NetSimProtocol.h
*
* \brief    Multithread version - use a seperate to handle simulation
*
* \author   Zhi Chen
* \version  1.0
* \date		8/23/2002
*/


#ifndef __NETSIMPROTOCOL_H__
#define __NETSIMPROTOCOL_H__

//#pragma warning(disable:4786)
//#include <map>
//#include <list>
//#include <vector>
#include "GenLnkLst.h"
#include "GuaranteeAlgorithm.h"



#define DEFAULT_LATENCY   (0)
#define DEFAULT_DROPRATE  (0)
#define DEFAULT_NEXTDROPCOUNTCOUNT	(0)
#define DEFAULT_DUPLICATEPACKET		(0)
//ignored
#define DEFAULT_BANDWIDTH (0)

#define DEFAULT_NETSIM_PORT (8888)

//ID for configuration parameters
#define DROPRATE_PARAM_ID      (1)
#define LATENCY_PARAM_ID       (2)
#define BANDWIDTH_PARAM_ID     (3)
#define NEXTDROPCOUNT_PARAM_ID (4)
#define DUPLICATEPACKET_PARAM_ID (5)

class CNetSimSession
{
public:
	CNetSimSession();
	virtual ~CNetSimSession();

	#define BUFFER_MAX 512
	typedef struct{u32 dwQueuedTime; u16 wlen;u8 pData[BUFFER_MAX];} MESSAGE; //copy of the sent message.

	//walk through the m_dwElements flags, and find the state whether this packet should be dropped.
	//MUST call SetAttribute() function first.
	bool DropThisPacket(void);
	void SetDropRate (u32 dwdroprate);
	void SetNextDropCount(u32 dwNextdropCount);

	
	bool DuplicateThisPacket(void);
	void SetDuplicatePacket(u32 dwDuplicate);

	u32 GetMessageCount(void);
	//return a reference to the first message
	//Must call GetMessageCount() first and MUST not return (0)
	MESSAGE &GetFirstMessage(void);
	//false if pop failed, otherwise true
	bool PopFirstMessage(void);
	void PushMessage(u8 *pData, u16 wLen, u32 dwLatency);

protected:
//	void InitDropRandomness(void);
//	void InitDropFlags(void);
	static void InitRandomness(bool *aFlags, u32 dwCount, u32 *pdwIndex, u32 dwPercent);
	static void InitFlags(bool *aFlags, u32 dwCount, bool bValue);

	//local copy
	u32			m_dwDropRate;  //unit in %, i.e. 5 means drop 5 packets / 100 packets - randomly.
	u32			m_dwNextDropCount;
	SLNKLSTPTR	m_ToSendList;//queue the messages up and wait to send.
	enum{ m_dwElements = 100};
	bool		m_bDropFlags[m_dwElements];
	u32			m_dwDropIndex; //current index of the flag array.

	//packet duplication simulation
	bool		m_bDuplicateFlags[m_dwElements];
	u32			m_dwDuplicateRate;
	u32			m_dwDuplicateIndex;
};


class CNetSimProtocol
{
public:
	CNetSimProtocol();
	virtual ~CNetSimProtocol();

	void Tick(void);//do the housekeep
	
	//app call this function to send
	SOCKERRTYPE TxBuffer(void *pData, u16 wLen, const struct X_SOCKADDRIN  &rDest_addr);
	SOCKERRTYPE RxBuffer(void *pData, u16 wLen, struct X_SOCKADDRIN  &rSource_addr);
	void SetLatency  (u32 dwlatency);
	void SetDropRate (u32 dwdroprate);
	void SetBandWidth(u32 dwbandwidth); //no use yet
	void SetNextDropCount(u32 dwNextdropCount);
	void SetDuplicatePacket(u32 dwDuplicate);
	
	bool StartNetSim();
	void ShutdownNetSim();

	typedef struct 
	{
		X_SOCKADDRIN	m_SockAddrin;
		CNetSimSession	m_NetSimSession;
	}NETSIMSESSIONLIST_DATA;
protected:
	//will be called by the inernal to send/receive data.
	virtual SOCKERRTYPE ITxTo(void *pData, u16 wLen, const struct X_SOCKADDRIN  &rDest_addr) = 0;
	virtual SOCKERRTYPE IRxFrom(void *pData, u16 wLen, struct X_SOCKADDRIN  &rSource_addr) = 0;

protected:
	//return 0 for success, otherwise non-zero
	int DoTick();

	SLNKLSTPTR m_NetSimSessionList; //data in the node will be NETSIMSESSIONLIST_DATA

	u32 m_dwLatency;   //unit in ms
	u32 m_dwBandWidth; //unit in kbits/sec, where k = 1000
	u32 m_dwDropRate;  //unit in %, i.e. 5 means packets / 100 packets - randomly.

	u32 m_dwDuplicate; //unit in % of packet didnot get dropped - randomly

//	u32 m_dwDropBurst; //unit in packet
//	u32 m_dwDropBurstInterval; //unit in ms.
	
	//for remote configuration
	//pData should be |ParamID|ParamValue|ParamID|ParamValue|ParamID|ParamValue|
	//all are u32
	static void ConfigCxnCallback(u32 dwmsg, const void *pData, u16 wLen, GP_SOCKETCONTEXT hContext, const struct X_SOCKADDRIN *pAddr, void *pContext );
	GP_SOCKETCONTEXT m_ConfigCxn;
};




#endif //__NETSIMPROTOCOL_H__
