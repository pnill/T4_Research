/**
* \file     ng2.cpp
*
* \brief    
*
* \author   Nicholas Gonzalez
* \version  1.0
* \date		8/19/2002
*/


#include "x_files.hpp"
#include "x_sock.h"
#include "ng2.h"
#include "GuaranteeAlgorithm.h"
#include "GenLnkLst.h"

#include "MemoryStream.h"
#include "EndianHelper.h"


#ifdef X_DEBUG
    #define NG2_DBG
    #define NG2_DBGOUT( s ) x_printf( (s) ) 
#else
    #define NG2_DBGOUT( s ) 
#endif

///////////////////////////////////////////////////////////////////////////////
// STATIC VARIABLES & CONSTS
///////////////////////////////////////////////////////////////////////////////
const s32 NG_SEND_BUFF_SIZE = 2048 ; // 2K SEND BUFFER
const s32 NG_RECV_BUFF_SIZE = 2048 ; // 2K RECEIVE BUFFER
#define NG_ERROR_MAX_FILE_LENGTH (128)

//static u8 s_aRecvBuff[ NG_RECV_BUFF_SIZE ] ;
static u8 s_aSendBuff[ NG_SEND_BUFF_SIZE ] ;

xbool gbSendAcks;

struct SNG_ERROR
{
    ENG_STATUS m_eStatus ;
    char       m_szFile[NG_ERROR_MAX_FILE_LENGTH] ;
    s32        m_nLine ;

} ;

static SNG_ERROR s_ngError ;

///////////////////////////////////////////////////////////////////////////////
// FORWARD FUNCTION DECLARATIONS
///////////////////////////////////////////////////////////////////////////////

static void NG_MessageReceived( u32 dwmsg, const void *pData, u16 wLen, GP_SOCKETCONTEXT hSocket,
                                  const struct X_SOCKADDRIN *pAddr, void *pContext ) ;

static void NGTIME_PreTick( NG_SOCKET_HANDLE hSock ) ;


///////////////////////////////////////////////////////////////////////////////
// NETGAME CORE COMMUNICATIONS FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
static void NG_SetAddress( X_SOCKADDRIN *pAddr, const XCHAR *pszAddress, s16 nPort )
{
    pAddr->sin_family = AF_INET ;
    pAddr->sin_addr.s_addr = x_inet_addr( pszAddress) ;
    pAddr->sin_port = x_htons(nPort) ;
}

static void NG_SetStatus( ENG_STATUS eStatus, const char *pszFile, int nLine )
{
    // make sure we always keep the first error, because that's what counts
    if( s_ngError.m_eStatus == NG_STATUS_OK )
    {
        s_ngError.m_eStatus = eStatus;
        if( pszFile )
            x_strcpy( s_ngError.m_szFile, pszFile );
        s_ngError.m_nLine = nLine;
    }
}

// macros to get line and file number in debug mode
#ifdef X_DEBUG
#define NG_SET_STATUS( eStatus ) (NG_SetStatus( (eStatus), __FILE__, __LINE__ ) )
#else
#define NG_SET_STATUS( eStatus ) (NG_SetStatus( (eStatus), NULL, 0 ) )
#endif

ENG_STATUS NG_GetStatus()
{
    return s_ngError.m_eStatus;
}

void NG_Initialize( )
{
   s_ngError.m_eStatus = NG_STATUS_OK ;
}

void NG_SetDestination( NG_DESTINATION *pDest, const XCHAR *pszAddress, s16 nPort )
{
   NG_SetAddress( (X_SOCKADDRIN*)pDest, pszAddress, nPort ) ;
}

/** creates a socket and binds it to the nPort on the local host */
NG_SOCKET_HANDLE NG_CreateSocketHandle( s16 nPort, pfnNG_TICK_CALLBACK pfnCallback, xbool bBroadcast, X_SOCKET sock )
{
    NG_SOCKET_HANDLE hSock ;

	if( !sock )
	{
		X_SOCKADDRIN addr ;//, addrClient ;

#ifdef TARGET_XBOX
		sock = x_socket( AF_INET, SOCK_DGRAM, IPPROTO_VDP ) ;
#endif
#ifdef TARGET_PS2
		sock = x_socket( AF_INET, SOCK_DGRAM, 0 ) ;
#endif
		if( sock == X_SOCKET_ERROR )
			return NULL ;
		
		x_memset( &addr, 0, sizeof( addr ) ) ;
		addr.sin_family = AF_INET ;
		addr.sin_addr.s_addr = x_htonl( INADDR_ANY ) ;
		addr.sin_port = x_htons( nPort ) ;
		if( X_SOCKET_ERROR == x_bind( sock, (X_SOCKADDR*)&addr, sizeof( addr ) ) )
		{
			return NULL ; 
		}

		XSOCK_SetBlockingMode( sock, X_SET_SOCKET_NONBLOCKING ) ;

	/*  if( bBroadcast == TRUE )
		{
			if( 0 != x_setsockopt( sock, SOL_SOCKET, SOL_SOCKET, (const char*)&bBroadcast, sizeof(int) ) )
			{
				return NULL ;
			}
		} */
	}

    hSock = (NG_SOCKET_HANDLE*)GP_CreateSocketContext( sock, NG_USE_NETSIM ) ;
//  TODO: Set the default settings for NETSIM
    if( NG_USE_NETSIM )
    {
        GP_SetLatency( hSock, 150 ) ;
    }
    if( hSock )
    {
		gbSendAcks = TRUE;
        GP_SetSocketContextCallBack( hSock, (pfnGP_CONTEXT_CALLBACK)NG_MessageReceived ) ;
        NG_SetTickCallback( hSock, pfnCallback ) ;
    }
    return hSock ;
}

void NG_DestroySocketHandle( NG_SOCKET_HANDLE hSock )
{
    X_SOCKET sock = GP_DestroySocketContext( (GP_SOCKETCONTEXT)hSock ) ;
    x_close( sock ) ;
}

pfnNG_TICK_CALLBACK NG_SetTickCallback( NG_SOCKET_HANDLE hSock, pfnNG_TICK_CALLBACK pfnCallback )
{
    pfnNG_TICK_CALLBACK pfnOld = (pfnNG_TICK_CALLBACK)GP_GetSocketCustomData( (GP_SOCKETCONTEXT)hSock ) ;
    GP_SetSocketCustomData( (GP_SOCKETCONTEXT)hSock, (void*)pfnCallback ) ;
    return pfnOld ;
}

u32 NG_GetThroughputStats( NG_SOCKET_HANDLE hSock, u32 dwStatToRetrieve )
{
    GP_SOCKETCONTEXT_STAT_INFO stats ;
    GP_GetSocketContextStatInfo( (GP_SOCKETCONTEXT)hSock, &stats );

    switch( dwStatToRetrieve )
    {
    case NG_STATS_BPS:
        return stats.dwTotalTxRxBytesPerSec ;
        break ;
    case NG_STATS_BPS_SENT:
        return stats.dwTotalTxBytesPerSec ;
        break ;
    case NG_STATS_BPS_RECEIVED:
        return stats.dwTotalRxBytesPerSec ;
        break ;
    case NG_STATS_TOTAL_BYTES:
        return stats.dwTotalTxBytes + stats.dwTotalRxBytes ;
        break ;
    case NG_STATS_BYTES_RECEIVED:
        return stats.dwTotalRxBytes ;
        break ;
    case NG_STATS_BYTES_SENT:
        return stats.dwTotalTxBytes ;
        break ;
    default:
        return 0 ;
        break ;
    }
}

s32 NG_SendToVoice( NG_SOCKET_HANDLE hSock, u8 *pData, s32 sdwSize, NG_DESTINATION *pDest )
{
    //-- check for error
    s32 sdwError = GP_SocketSendTo( (GP_SOCKETCONTEXT)hSock, pData, (u16)sdwSize, (X_SOCKADDRIN*)pDest, eVoice );
    if( sdwError == -1 )
        NG_SET_STATUS( NG_STATUS_GENERIC_SEND_ERROR );
    return sdwError;    
}

// HACK!  Lie to the caller so we detect disconnect on heartbeat sends.
s32 NG_SendTo( NG_SOCKET_HANDLE hSock, u8 *pData, s32 sdwSize, NG_DESTINATION *pDest )
{
    //-- check for error
    s32 sdwError = GP_SocketSendTo( (GP_SOCKETCONTEXT)hSock, pData, (u16)sdwSize, (X_SOCKADDRIN*)pDest, eUnreliable );
    if( sdwError == -1 )
        NG_SET_STATUS( NG_STATUS_GENERIC_SEND_ERROR );
    return sdwError;    
}

s32 NG_GuaranteedSendTo( NG_SOCKET_HANDLE hSock, u8 *pData, s32 sdwSize, NG_DESTINATION *pDest ) 
{
    s32 sdwError = GP_SocketSendTo( (GP_SOCKETCONTEXT)hSock, pData, (u16)sdwSize, (X_SOCKADDRIN*)pDest, eReliable );
    if( sdwError == -1 )
        NG_SET_STATUS( NG_STATUS_GENERIC_SEND_ERROR );
    return sdwError;
}


void NG_Tick( NG_SOCKET_HANDLE hSock, void *pContext )
{
	NGTIME_PreTick( hSock ) ;
	GP_TickSocketContext( (GP_SOCKETCONTEXT)hSock, pContext ) ;
}

///////////////////////////////////////////////////////////////////////////////
// TIME SYNCHRONIZATION & STATS FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
void NGSIM_SetLatency( NG_SOCKET_HANDLE hSock, u32 dwLatency )
{
    if( NG_USE_NETSIM )
    {

    }
}
void NGSIM_SetDropRate(NG_SOCKET_HANDLE hSock, u32 dwDropRate )
{
    if( NG_USE_NETSIM )
    {

    }
}

void NGSIM_SetNextDropCount(NG_SOCKET_HANDLE hSock, u32 dwNextDropCount)
{
    if( NG_USE_NETSIM )
    {

    }
}

///////////////////////////////////////////////////////////////////////////////
// TIME SYNCHRONIZATION & STATS FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
static NG_NETWORK_STATS s_netStats ;
static pfnNG_GETTIME    NGTIME_GetTimeCB ;
static xbool            s_bTimeServer ;
static u32              s_dwTimeUpdateFrequency ;
static u32              s_dwTimeLastUpdate ;
static NG_DESTINATION   s_addrTimeServer ;

static void NGTIME_Init( pfnNG_GETTIME pfnGetTime, xbool bTimeServer )
{
    s_netStats.m_fAverageRTT = 0.0f ;
    s_netStats.m_fTimeDiff = -1.0f ;
    s_netStats.m_dwMaxRTT = 0 ;
    s_netStats.m_dwMinRTT = 0xffffffff ;
    s_netStats.m_dwRTTCount = 0 ;
    s_netStats.m_dwTotalRTT = 0 ;

	s_bTimeServer = bTimeServer ;
    s_dwTimeUpdateFrequency = 0 ;
    s_dwTimeLastUpdate = 0 ;
    NGTIME_GetTimeCB = pfnGetTime ;
}

void NGTIME_InitTimeServer( pfnNG_GETTIME pfnGetTime )
{
    NGTIME_Init( pfnGetTime, TRUE ) ;
}

void NGTIME_InitTimeClient( pfnNG_GETTIME pfnGetTime, NG_DESTINATION *pDest )
{
    NGTIME_Init( pfnGetTime, FALSE ) ;
    x_memcpy( &s_addrTimeServer, pDest, sizeof( NG_DESTINATION ) ) ;

}

void NGTIME_SetTimeServer( NG_DESTINATION *pDest )
{
    ASSERT( pDest ) ;
    x_memcpy( &s_addrTimeServer, pDest, sizeof( NG_DESTINATION ) ) ;
}

void NGTIME_SetUpdateFrequency( u32 dwMs )
{
    s_dwTimeUpdateFrequency = dwMs ;
}

u32 NGTIME_GetUpdateFrequency( )
{
    return s_dwTimeUpdateFrequency ;
}
s32 NGTIME_SendSynchRequest( NG_SOCKET_HANDLE sock, NG_DESTINATION *pDest )
{
    NG_MSG_TIME_SYNCH_REQ msg;
    msg.m_dwMsgType = NG_MSGTYPE_TIME_REQUEST ;
    msg.m_dwClientTime = NGTIME_GetTimeCB( ) ;
    if( sizeof( msg ) != 
        NG_GuaranteedSendTo( sock, (u8*)&msg, sizeof( msg ), pDest ) )
    {
        return - 1;
    }
    return 0 ;
}

s32 NGTIME_ProcessSynchRequest( NG_MSG_TIME_SYNCH_REQ *pMsg, NG_SOCKET_HANDLE s, NG_DESTINATION *pDest )
{   
    ASSERT( pMsg ) ;
    if( !pMsg )
        return -1 ;

    pMsg->m_dwServerTime = NGTIME_GetTimeCB( ) ;
    pMsg->m_dwMsgType = NG_MSGTYPE_TIME_RESPONSE ;
    if( sizeof( NG_MSG_TIME_SYNCH_REQ ) != 
        NG_GuaranteedSendTo( s, (u8*)pMsg, sizeof( NG_MSG_TIME_SYNCH_REQ ), pDest ) )
    {
        return -1 ;
    }
    return 0 ;
}

// pass in a new rtt and time diff samples and this function will update the netstats
static void NGTIME_UpdateNetStats( u32 dwRTT, f32 fTimeDiff ) 
{
    if( dwRTT <= s_netStats.m_dwMinRTT )
    {
        s_netStats.m_fTimeDiff = fTimeDiff ;
        s_netStats.m_dwMinRTT = dwRTT ;
    }
    
    if( dwRTT > s_netStats.m_dwMaxRTT )
    {
        s_netStats.m_dwMaxRTT = dwRTT ;
    }

	//--Zhi
	//use the last 5 RTT values for average RTT
	static u32 adwRTT[5] = {0,0,0,0,0};
	static u32 dwRTTCount = 0;
	if(!(dwRTTCount < sizeof(adwRTT)/sizeof(u32)))
		dwRTTCount = 0;
	//use the array as a circular buffer
	adwRTT[dwRTTCount] = dwRTT;
	dwRTTCount++;
    //--Zhi end

    s_netStats.m_dwTotalRTT += dwRTT ;
    s_netStats.m_dwRTTCount++ ;

	f32 fRTT = 0.0;
	for(u32 i=0; i < (sizeof(adwRTT)/sizeof(u32)); i++ )
		fRTT += adwRTT[i];

    s_netStats.m_fAverageRTT = fRTT/(f32)(sizeof(adwRTT)/sizeof(u32));
		
		//(f32)(s_netStats.m_dwTotalRTT / (f32)s_netStats.m_dwRTTCount) ;
		//fRTT/ ((f32)(sizeof(adwRTT)/sizeof(u32))) ;//(f32)(s_netStats.m_dwTotalRTT / (f32)s_netStats.m_dwRTTCount) ;

}

void NGTIME_ProcessSynchResponse( NG_MSG_TIME_SYNCH_REQ *pMsg, NG_SOCKET_HANDLE s, NG_DESTINATION *pDest )
{
    u32 dwRTT ;
    f32 fTimeDiff ;

    ASSERT( pMsg ) ;
    if( !pMsg )
        return ;

    //-- get the time difference
    dwRTT = NGTIME_GetTimeCB( ) - pMsg->m_dwClientTime ;
    fTimeDiff = pMsg->m_dwServerTime - (float)(dwRTT / 2.0f) - pMsg->m_dwClientTime ;

    //-- should we use this one?
    NGTIME_UpdateNetStats( dwRTT, fTimeDiff ) ;

    //-- send the other machine (time host) our stats
    NG_MSG_TIME_RTT msgRTT ;
    msgRTT.m_dwMsgType = NG_MSGTYPE_TIME_RTT ;
    msgRTT.m_dwRTT = dwRTT ;

    NG_GuaranteedSendTo( s, (u8*)&msgRTT, sizeof(msgRTT), pDest ) ;
}

void NGTIME_ProcessRTTMsg( NG_MSG_TIME_RTT *pMsg )
{
    //-- should only be received by host
    NGTIME_UpdateNetStats( pMsg->m_dwRTT, 0.0f ) ;    
}

//==========================================================================

u32 NGTIME_GetGlobalTime()
{
	if( !NGTIME_GetTimeCB )
		return 0;

    if( s_bTimeServer )
        return NGTIME_GetTimeCB();
    else
        return NGTIME_GetTimeCB() + (u32)s_netStats.m_fTimeDiff;
}

//==========================================================================

NG_NETWORK_STATS* NGTIME_GetNetworkStats()
{
    return &s_netStats ;
}


void	NGTIME_GetNetThroughputStats(NG_SOCKET_HANDLE sock, NG_NET_THROUGHPUT_STATS *pStats)
{
    if( !pStats )
        return ;

	GP_SOCKETCONTEXT_STAT_INFO statinfo;
	GP_GetSocketContextStatInfo((GP_SOCKETCONTEXT)sock, &statinfo);
	
	pStats->m_dwTotalTxBytes = statinfo.dwTotalTxBytes;
	pStats->m_dwTotalRxBytes = statinfo.dwTotalRxBytes;
	pStats->m_dwTotalTxRxBytesPerSec = statinfo.dwTotalTxRxBytesPerSec;
	pStats->m_dwTotalTxBytesPerSec = statinfo.dwTotalTxBytesPerSec;
	pStats->m_dwTotalRxBytesPerSec = statinfo.dwTotalRxBytesPerSec;
}

void NGTIME_PreTick( NG_SOCKET_HANDLE hSock )
{
    if( (s_dwTimeUpdateFrequency > 0) && 
        ((NGTIME_GetTimeCB( ) - s_dwTimeLastUpdate) > s_dwTimeUpdateFrequency ) )
    {
        NGTIME_SendSynchRequest( hSock, &s_addrTimeServer ) ;
    }
}



///////////////////////////////////////////////////////////////////////////////
// EVENT SYNCHRONIZATION 
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// DATATYPES & CONSTS
///////////////////////////////////////////////////////////////////////////////

const s32 NGES_MAX_SYNCH_DATA_SIZE = 512;
const s32 NGES_MAX_SYNCHED_EVENTS  = 8;
const s32 NGES_SYNCH_INVALID       = 0;

struct SNGES_SYNCH_EVENT_DATA
{
	u32     m_dwEventType;
	u32     m_dwTimeToStart;
	s32     m_sdwDataSize;
	s32     m_sdwHandshake;
	u8      m_aData[ NGES_MAX_SYNCH_DATA_SIZE ];
} ;

///////////////////////////////////////////////////////////////////////////////
// STATIC VARIABLES
///////////////////////////////////////////////////////////////////////////////
//future will support multiple simultaneous requests
static SNGES_SYNCH_EVENT_DATA s_synchEvent[NGES_MAX_SYNCHED_EVENTS] ; 

///////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
///////////////////////////////////////////////////////////////////////////////

void NGES_FreeSynchedEvent( SNGES_SYNCH_EVENT_DATA *pData ) ;


///////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
void NGES_Init( )
{
    for( s32 i = 0 ; i < NGES_MAX_SYNCHED_EVENTS ; i++ )
    {
        NGES_FreeSynchedEvent( &s_synchEvent[i] ) ;
    }
}

//==========================================================================

void NGES_DBG_DumpQueue( u32 Unused )
{
#ifdef X_DEBUG

	s32 i;

    x_printf( "\n <<<<< ONLINE NG2 >>>>>: Network event queue content dump\n" );

    for( i = 0 ; i < NGES_MAX_SYNCHED_EVENTS ; i++ )
    {
        if( s_synchEvent[i].m_dwEventType != (u32)NGES_SYNCH_INVALID )
        {
            x_printf( "   Event[%d]  Time:%d  DataSize: %d  Handshake:%d\n", s_synchEvent[i].m_dwEventType, s_synchEvent[i].m_dwTimeToStart, s_synchEvent[i].m_sdwDataSize, s_synchEvent[i].m_sdwHandshake );
        }
    }

    x_printf( "\n <<<<< ONLINE NG2 >>>>>: End queue content dump\n" );

#endif
}

//==========================================================================

//-- if there is more then one event in the queue it will return the one with 
//-- the earliest synch time
SNGES_SYNCH_EVENT_DATA* NGES_GetSynchedEvent( u32 dwSynchEventType )
{
    s32 nEvent = -1 ;
    for( s32 i = 0 ; i < NGES_MAX_SYNCHED_EVENTS ; i++ )
    {
        if( s_synchEvent[i].m_dwEventType == dwSynchEventType )
        {
            if( nEvent == -1 )
                nEvent = i ;
            else
            {
                nEvent = (s_synchEvent[nEvent].m_dwTimeToStart <= s_synchEvent[i].m_dwTimeToStart ) ? nEvent : i ;
            }
        }
    }
    if( nEvent == -1 )
        return NULL ;
    else
        return &s_synchEvent[nEvent] ;
}

SNGES_SYNCH_EVENT_DATA* NGES_NewSynchedEvent( )
{
    for( s32 i = 0 ; i < NGES_MAX_SYNCHED_EVENTS ; i++ )
    {
        if( s_synchEvent[i].m_dwEventType == (u32)NGES_SYNCH_INVALID )
            return &s_synchEvent[i] ;
    }
    //-- we are out of memory
    NG_SET_STATUS( NG_STATUS_GENERIC_MEMORY_ERROR ) ;
    return NULL ;
}

void NGES_FreeSynchedEvent( SNGES_SYNCH_EVENT_DATA *pData )
{
    ASSERT( pData ) ;
    if( !pData )
        return ;
    pData->m_dwEventType = NGES_SYNCH_INVALID ;
    pData->m_dwTimeToStart = 0xffffffff ;
    pData->m_sdwDataSize = 0 ;
    pData->m_sdwHandshake = 0 ;
}

void NGES_PrepSynchEventRequest( NG_MSG_SYNCH_EVENT* pMsg, u32 dwSynchEvent, u32 dwOptions )
{
    ASSERT( pMsg );
    if( !pMsg )
        return;

    //NG_NETWORK_STATS* pNetStats = NGTIME_GetNetworkStats( );

    pMsg->m_dwMsgType    = NG_MSGTYPE_SYNCH_EVENT_REQUEST ;
    pMsg->m_dwEventID    = dwSynchEvent ;
    pMsg->m_sdwHandshake = 0 ;
    pMsg->m_dwTime       = NGTIME_GetGlobalTime( ) + 150 ;//((u32)(pNetStats->m_fAverageRTT)) ;
    pMsg->m_sdwMemSize   = 0 ;

    if( dwOptions == NET_SYNCH_OPT_RIGHT_AWAY )
    {
        pMsg->m_dwTime = 0 ;
    }
    
    if( dwOptions == NET_SYNCH_OPT_TWO_WAY ) // does a two way handshake (ack)
    {
        pMsg->m_sdwHandshake = 1 ;
    }
}

s32 NGES_SynchEvent( NG_SOCKET_HANDLE sock, NG_DESTINATION *pDest, u32 dwSynchEvent, u32 dwOption )
{
    NG_MSG_SYNCH_EVENT msgSynchEvent;

    NGES_PrepSynchEventRequest( &msgSynchEvent, dwSynchEvent, dwOption );


    //-- this should be guaranteed
    s32 retval = NG_GuaranteedSendTo( sock, (u8*)&msgSynchEvent, sizeof( msgSynchEvent ), pDest );
    
	#ifdef X_DEBUG
		// HACK! HACK! HACK! HACK! HACK! HACK! HACK! HACK! HACK! HACK! HACK! 
		// DEBUG ONLY FOR ASB
		if( retval != -1 )
		{
			void ASBDebugPrintNetMsg( u32 NetSynchEventID, void* pEventData, xbool bReceive );
			ASBDebugPrintNetMsg( dwSynchEvent, NULL, FALSE );
		}
		else
		{
			x_printf( "   <<< NG2 >>>  NGES_SynchEvent:: call to NG_GuaranteedSendTo FAILED\n" );
		}
	#endif
    
    return retval;
}

//==========================================================================

static xbool NGES_InternalIsReady( SNGES_SYNCH_EVENT_DATA *pSynchEvent )
{
    if( (pSynchEvent) &&
        (pSynchEvent->m_dwTimeToStart != 0xfffffff) && 
        (pSynchEvent->m_dwEventType != (u32)NGES_SYNCH_INVALID) &&
        //(NGTIME_GetGlobalTime() >= pSynchEvent->m_dwTimeToStart) &&
        (pSynchEvent->m_sdwHandshake <= 0) )
	{
		return TRUE;
	}

	return FALSE;
}

//==========================================================================

u32 NGES_IsSynchedEventReady( u32 dwSynchEvent )
{
    //-- get the synched event
    SNGES_SYNCH_EVENT_DATA *pSynchEvent = NGES_GetSynchedEvent( dwSynchEvent ) ;

    if( NGES_InternalIsReady( pSynchEvent ) )
    {
        NGES_FreeSynchedEvent( pSynchEvent ) ;
        return (u32)TRUE ;
    }
    return (u32)FALSE ;
}


s32 NGES_SynchDataEvent( NG_SOCKET_HANDLE sock, NG_DESTINATION *pDest, u32 dwSynchEvent, void *pMem, s32 sdwSize, u32 dwOption )
{
    ASSERT( sdwSize <= NGES_MAX_SYNCH_DATA_SIZE ) ;
    //-- handle relese mode error conditions
    if( sdwSize > NGES_MAX_SYNCH_DATA_SIZE )
        return ENOMEM ;

    NG_MSG_SYNCH_EVENT msgSynchEvent ;
    s32 sdwPacketSize ;
    

    NGES_PrepSynchEventRequest( &msgSynchEvent, dwSynchEvent, dwOption ) ;
        
    
    //-- add the information about the data we are sending with this event
    msgSynchEvent.m_sdwMemSize = sdwSize ;

    x_memcpy( s_aSendBuff, &msgSynchEvent, sizeof( msgSynchEvent ) ) ;
    x_memcpy( (void*)((u8*)s_aSendBuff + sizeof( msgSynchEvent)), pMem, sdwSize )  ;

    sdwPacketSize = sdwSize + sizeof( msgSynchEvent ) ;    
    
    s32 retval = NG_GuaranteedSendTo( sock, s_aSendBuff, sdwPacketSize, pDest );

	#ifdef X_DEBUG
		// HACK! HACK! HACK! HACK! HACK! HACK! HACK! HACK! HACK! HACK! HACK! 
		// DEBUG ONLY FOR ASB
		if( retval != -1 )
		{
			void ASBDebugPrintNetMsg( u32 NetSynchEventID, void* pEventData, xbool bReceive );
			ASBDebugPrintNetMsg( dwSynchEvent, pMem, FALSE );
		}
		else
		{
			x_printf( "   <<< NG2 >>>  NGES_SynchDataEvent:: call to NG_GuaranteedSendTo FAILED\n" );
		}
	#endif
    
    return retval;
}



u32 NGES_IsSynchedDataEventReady( u32 dwSynchEvent, void **ppData, s32 *psdwSize )
{
    SNGES_SYNCH_EVENT_DATA *pSynchEvent = NGES_GetSynchedEvent( dwSynchEvent ) ;
    
    if( NGES_InternalIsReady( pSynchEvent ) )
    {
        if( pSynchEvent->m_sdwDataSize > 0)
        {
            *psdwSize = pSynchEvent->m_sdwDataSize ;
            *ppData = (void*)pSynchEvent->m_aData ;
        }
        else
        {
            *psdwSize = 0 ;
            *ppData = NULL ;
        }

        NGES_FreeSynchedEvent( pSynchEvent ) ;
        return (u32)TRUE ;
    }
    return (u32)FALSE ;
    
    
    
}

void NGES_ClearAllPendingEvents( )
{
    s32 i = 0 ;
#ifdef X_DEBUG
    s32 sdwTotalEvents = 0 ;
    //-- if we are in debug mode- let's see what's in here first
    x_printf( "NG: Begin network event report\n" ) ;
    x_printf( "NG: Clearing all pending events\n" ) ;
    for( i = 0 ; i < NGES_MAX_SYNCHED_EVENTS ; i++ )
    {
        if( s_synchEvent[i].m_dwEventType != (u32)NGES_SYNCH_INVALID )
        {
            x_printf( "NG: Event found [%d]\n", s_synchEvent[i].m_dwEventType ) ;
            sdwTotalEvents++ ;
        }
    }
    x_printf( "NG: Total events found [%d]\n", sdwTotalEvents ) ;
    x_printf( "NG: End network event report\n" ) ;
#endif
    for( i = 0 ; i < NGES_MAX_SYNCHED_EVENTS ; i++ )
    {
        NGES_FreeSynchedEvent( &(s_synchEvent[i]) ) ;
    }
}

void NGES_ClearPendingEvents( u32 dwEventType )
{
    for( s32 i = 0 ; i < NGES_MAX_SYNCHED_EVENTS ; i++ )
    {
        if( s_synchEvent[i].m_dwEventType == dwEventType )
        {
            NGES_FreeSynchedEvent( &(s_synchEvent[i]) ) ;
        }
    }
}

//==========================================================================

u32 NGES_GetNextQueuedEvent( u32 hEventHandle, u32* dwSynchEvent, void** ppData, s32* psdwSize )
{
	SNGES_SYNCH_EVENT_DATA* pEvent = (SNGES_SYNCH_EVENT_DATA*)hEventHandle;
	s32 i, j;
	u32 MaxEventTime = 0xFFFFFFF;
	s32 NextEventID = -1;

	// loop through event array, checking if any match the handle passed in
	for( i = 0; i < NGES_MAX_SYNCHED_EVENTS; i++ )
	{
		if( pEvent == &s_synchEvent[i] )
		{
			// hEventHandle is valid, find next queued event
			for( j = 0; j < NGES_MAX_SYNCHED_EVENTS; j++ )
			{
				// check if event is ready to be returned, and that it's timestamp is
				// later than the "current" one defined by hEventHandle
				if( (pEvent != &s_synchEvent[j]) &&
					(NGES_InternalIsReady( &s_synchEvent[j] )) &&
					(s_synchEvent[j].m_dwTimeToStart > pEvent->m_dwTimeToStart) )
				{
					// make sure this event is the earliest one possible
					if( s_synchEvent[j].m_dwTimeToStart < MaxEventTime )
					{
						MaxEventTime = s_synchEvent[j].m_dwTimeToStart;
						NextEventID  = j;
					}
				}
			}

			// if there is no next event found, return NULL
			if( NextEventID == -1 )
				return NULL;
		}
	}


	if( NextEventID == -1 )
	{
		// handle passed in was not valid, so find the first one that is
		for( i = 0; i < NGES_MAX_SYNCHED_EVENTS; i++ )
		{
			// check if event is ready to be returned
			if( NGES_InternalIsReady( &s_synchEvent[i] ) )
			{
				// make sure this event is the earliest one possible
				if( s_synchEvent[i].m_dwTimeToStart < MaxEventTime )
				{
					MaxEventTime = s_synchEvent[i].m_dwTimeToStart;
					NextEventID  = i;
				}
			}
		}
	}

	// if event found, fill in parameter values and return handle
	if( NextEventID != -1 )
	{
		if( dwSynchEvent )
			*dwSynchEvent = s_synchEvent[NextEventID].m_dwEventType;

		if( s_synchEvent[NextEventID].m_sdwDataSize > 0 )
		{
			if( ppData )	*ppData   = (void*)s_synchEvent[NextEventID].m_aData;
			if( psdwSize )	*psdwSize = s_synchEvent[NextEventID].m_sdwDataSize;
		}
		else
		{
			if( ppData )	*ppData   = NULL;
			if( psdwSize )	*psdwSize = NULL;
		}

		return (u32)(&s_synchEvent[NextEventID]);
	}

	return NULL;
}

//==========================================================================

u32 NGES_RemoveQueuedEvent( u32 hEventHandle, u32* dwSynchEvent, void** ppData, s32* psdwSize )
{
	SNGES_SYNCH_EVENT_DATA* pEvent = (SNGES_SYNCH_EVENT_DATA*)hEventHandle;
	s32 i;

	// find event that handle represents
	for( i = 0; i < NGES_MAX_SYNCHED_EVENTS; i++ )
	{
		if( pEvent == &s_synchEvent[i] )
		{
			// hEventHandle is valid, find next queued event
			hEventHandle = NGES_GetNextQueuedEvent( hEventHandle, dwSynchEvent, ppData, psdwSize );
			
			NGES_FreeSynchedEvent( pEvent );
			
			return hEventHandle;
		}
	}

	// hEventHandle was invalid, just return the first valid event
	return NGES_GetNextQueuedEvent( NULL, dwSynchEvent, ppData, psdwSize );
}



///////////////////////////////////////////////////////////////////////////////
// DATA SYNCHRONIZATION SYSTEM FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
#define NGDSS_MAX_ADDRESSES         (32)
#define NGDSS_MAX_VARS              (128)
#define NGDSS_MAX_GROUPS            (128)



struct NGDSS_VARPTR_CONTAINER
{
    NGDSS_TYPE m_eType ;
    void       *m_pData ;
    s32        m_sdwSize ; // in bytes
    pfnPOINTER_CONVERTER PointerConverter ; // if type == pointer
} ;


struct NGDSS_GROUP
{
    u32                     m_dwGroupID ;
    xbool                   m_bOwnership ;
    NGDSS_GROUP_OPTIONS     m_options ;
    //NGDSS_GROUP_NODE*       m_pChildren[NGDSS_MAX_GROUPS] ; //-- list of groups
    NGDSS_VARPTR_CONTAINER* m_pVars[NGDSS_MAX_VARS] ;     //-- list of vars
    s32                     m_nTotalVars ; //-- total vars in the var array
    s32                     m_nTotalGroups ;
} ;


struct  NGDSS_INTERNAL_DATA
{
    // groups
    SLNKLSTPTR          m_lstGroups ;
	xbool				m_validList;
    
    // addresses
    s32            m_nValidAddresses ;
    NG_DESTINATION m_addr[NGDSS_MAX_ADDRESSES] ;
} ;


static NGDSS_INTERNAL_DATA s_ngdss ;


///////////////////////////////////////////////////////////////////////////////
// NGDSS MESSAGES
///////////////////////////////////////////////////////////////////////////////
struct NGDSS_UPDATE_MESSAGE
{
    NG_MSG_HEADER
    u32 m_dwGroupID ;
    s32 m_sdwSize ;
} ;

///////////////////////////////////////////////////////////////////////////////
// HELPER FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
NGDSS_GROUP* NGDSS_GroupFromHandle( NGDSS_GROUP_HANDLE hGroup )
{
    return (NGDSS_GROUP*)hGroup ;
}

s32   NGDSS_GetDataSize( NGDSS_TYPE eType )
{
    switch( eType )
    {
    case NGDSS_U8:
    case NGDSS_S8:
        return 1 ;
    case NGDSS_U16:
    case NGDSS_S16:
        return 2 ;
    case NGDSS_BOOL:
    case NGDSS_POINTER:
    case NGDSS_U32:
    case NGDSS_S32:
    case NGDSS_F32:
        return 4 ;
    case NGDSS_S64:
    case NGDSS_U64:
    case NGDSS_F64:
        return 8 ;
    default:
        return 0 ;
    } ;
    return 0 ;
    
}

///////////////////////////////////////////////////////////////////////////////
// NGDSS FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

void NGDSS_Init( )
{
    s_ngdss.m_nValidAddresses = 0 ;
    SLNKLSTPTR_Init( &s_ngdss.m_lstGroups ) ;
	s_ngdss.m_validList = TRUE;
}

void NGDSS_DeInit( )
{
	if(s_ngdss.m_validList)
	{
		SLNKLSTPTR_DeInit( &s_ngdss.m_lstGroups ) ;
		s_ngdss.m_validList = FALSE;
	}
}

void   NGDSS_AddRemoteAddress( NG_DESTINATION *pDest )
{
    ASSERT( s_ngdss.m_nValidAddresses < NGDSS_MAX_ADDRESSES ) ;
    x_memcpy( &s_ngdss.m_addr[s_ngdss.m_nValidAddresses], (void*)pDest, sizeof( NG_DESTINATION ) ) ;
    s_ngdss.m_nValidAddresses++ ;
}

void NGDSS_RemoveRemoteAddress( NG_DESTINATION *pDest )
{
    ASSERT( 0 ) ;
}

void NGDSS_ResetAddressList( NG_DESTINATION *pDest )
{
    s_ngdss.m_nValidAddresses = 0 ;
}

///////////////////////////////////////////////////////////////////////////////
// NGDSS GROUP FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

NGDSS_GROUP_HANDLE NGDSS_GetGroup( u32 dwGroupID )
{
    SLSTNODEPTR *pNode = NULL ;
    pNode = SLNKLSTPTR_GetHeadNode( &s_ngdss.m_lstGroups ) ;
    while( pNode )
    {
        if( ((NGDSS_GROUP*)(pNode->pNodeData))->m_dwGroupID == dwGroupID )
            return (NGDSS_GROUP_HANDLE)pNode->pNodeData ;
        pNode = SLNKLSTPTR_GetNextNode( &s_ngdss.m_lstGroups, pNode ) ;
    }
    return NULL ;
}

xbool NGDSS_DoesGroupExist( u32 dwGroupID )
{
    if( NGDSS_GetGroup( dwGroupID ) )
        return TRUE ;
    return FALSE ;
}


NGDSS_GROUP_HANDLE NGDSS_CreateGroup( u32 dwGroupID ) 
{
    NGDSS_GROUP *pGroup ;
    if( NGDSS_DoesGroupExist(dwGroupID) )
        return NULL ;
    pGroup = (NGDSS_GROUP*)x_malloc( sizeof(NGDSS_GROUP) ) ;
    if( !pGroup )
        return NULL ;

    pGroup->m_dwGroupID = dwGroupID ;
    pGroup->m_nTotalGroups = 0 ;
    pGroup->m_nTotalVars = 0 ;
    pGroup->m_options.m_dwLastUpdate = 0 ;
    pGroup->m_options.m_dwUpdateFrequency = 0 ;
    pGroup->m_options.m_pContext = NULL ;
    pGroup->m_options.m_pfnNotify = NULL ;
    pGroup->m_bOwnership = FALSE ;

    SLNKLSTPTR_InsertAtHead( &s_ngdss.m_lstGroups, pGroup ) ;

    return (NGDSS_GROUP_HANDLE)pGroup ;
}

void NGDSS_DestroyGroup( NGDSS_GROUP_HANDLE hGroup )
{
    NGDSS_GROUP *pGroup = (NGDSS_GROUP*)NGDSS_GroupFromHandle( hGroup ) ;
    if( pGroup )
    {
        
        for( s32 i = 0 ; i < pGroup->m_nTotalVars ; i++ ) 
        {
            x_free( pGroup->m_pVars[i] ) ;
        }
        SLNKLSTPTR_RemoveNodeOfData( &s_ngdss.m_lstGroups, pGroup ) ;
        x_free( pGroup );
    }
}

static NGDSS_VARPTR_CONTAINER* NGDSS_InternalRegisterVar( NGDSS_GROUP *pGroup, 
                                                         NGDSS_TYPE eDataType, void *pSrc )
{
    NGDSS_VARPTR_CONTAINER *pVar ;
    ASSERT( pGroup ) ;
    if( !pGroup )
        return NULL ;
    pVar = (NGDSS_VARPTR_CONTAINER*)x_malloc( sizeof( NGDSS_VARPTR_CONTAINER ) ) ;
    if( !pVar )
        return NULL ;

    pVar->m_eType = eDataType ;
    pVar->m_pData = pSrc ;
    pVar->m_sdwSize = NGDSS_GetDataSize( eDataType ) ;
    pVar->PointerConverter = NULL ;
    pGroup->m_pVars[pGroup->m_nTotalVars] = pVar ;
    pGroup->m_nTotalVars++ ;

    return pVar ;
}
void NGDSS_RegisterVariable( NGDSS_GROUP_HANDLE hGroup, NGDSS_TYPE eDataType, void *pSrc )
{
    NGDSS_GROUP *pGroup = (NGDSS_GROUP*)NGDSS_GroupFromHandle(hGroup);
    ASSERT( pGroup ) ;
    if( !pGroup )
        return ;

    NGDSS_InternalRegisterVar( pGroup, eDataType, pSrc ) ;
    
    
}

void NGDSS_RegisterBLOB( void *pSrc, s32 sdwSize )
{
    ASSERT( 0 ) ;
}

void NGDSS_RegisterPointer( NGDSS_GROUP_HANDLE hGroup, void **pPointer, pfnPOINTER_CONVERTER pfnCallback ) 
{
    NGDSS_GROUP *pGroup = NGDSS_GroupFromHandle( hGroup ) ;
    NGDSS_VARPTR_CONTAINER *pVar ;

    ASSERT( pGroup ) ;
    if( !pGroup )
        return ;

    pVar = NGDSS_InternalRegisterVar( pGroup, NGDSS_POINTER, (void*)pPointer ) ;
    pVar->PointerConverter = pfnCallback ;
}

void NGDSS_SetGroupOptions( NGDSS_GROUP_HANDLE hGroup, NGDSS_GROUP_OPTIONS *pOptions, u32 dwValidFields )
{
    NGDSS_GROUP *pGroup = (NGDSS_GROUP*)NGDSS_GroupFromHandle(hGroup) ;
    ASSERT( pGroup ) ;
    if( !pGroup )
        return ;
    
    if( dwValidFields & NGDSS_GROUP_OPT_FREQUENCY )
        pGroup->m_options.m_dwUpdateFrequency = pOptions->m_dwUpdateFrequency ;
    
    if( dwValidFields & NGDSS_GROUP_OPT_CALLBACK )
    {
        pGroup->m_options.m_pfnNotify = pOptions->m_pfnNotify ;
        pGroup->m_options.m_pContext = pOptions->m_pContext ;
    }

    if( dwValidFields & NGDSS_GROUP_OPT_LASTUPDATE )
        pGroup->m_options.m_dwLastUpdate = pOptions->m_dwLastUpdate ;
}

void NGDSS_GetGroupOptions( NGDSS_GROUP_HANDLE hGroup, void *pOptValue, u32 dwField )
{
    NGDSS_GROUP *pGroup = (NGDSS_GROUP*)NGDSS_GroupFromHandle(hGroup) ;
    ASSERT( pGroup ) ;
    if( !pGroup )
        return ;

    if( dwField & NGDSS_GROUP_OPT_FREQUENCY )
        *((u32*)pOptValue) = pGroup->m_options.m_dwUpdateFrequency ;
    
    if( dwField & NGDSS_GROUP_OPT_CALLBACK )
        *((pfnNGDDS_UPDATE_NOTIFICATION*)pOptValue) = pGroup->m_options.m_pfnNotify ;

    if( dwField & NGDSS_GROUP_OPT_LASTUPDATE )
        *((u32*)pOptValue) = pGroup->m_options.m_dwLastUpdate ;
}

void NGDSS_SetGroupOwnership( NGDSS_GROUP_HANDLE hGroup, xbool bOwn )
{
    NGDSS_GROUP *pGroup = (NGDSS_GROUP*)NGDSS_GroupFromHandle(hGroup) ;
    ASSERT( pGroup ) ;
    if( !pGroup ) 
        return ;

    pGroup->m_bOwnership = bOwn ;
}

xbool NGDSS_GetGroupOwnership( NGDSS_GROUP_HANDLE hGroup ) 
{
    NGDSS_GROUP *pGroup = NGDSS_GroupFromHandle(hGroup) ;
    ASSERT( pGroup ) ;
    if( !pGroup )
        return FALSE ;

    return pGroup->m_bOwnership ;
}

static void NGDSS_InternalSendUpdate( NGDSS_GROUP *pGroup, NG_SOCKET_HANDLE hSock, u32 dwTime )
{
    u8 *pBuff = s_aSendBuff ;
    s32 i = 0 ;
    u32 dwBools = 0 ; // a u32 for the bools
    u32 dwTotalBools = 0 ; // the number of bools in dwBools 
    u32 dwTemp = 0 ;
    
    ASSERT( pGroup ) ;
    if( !pGroup )
        return ;

    if( pGroup->m_options.m_pfnNotify )
        pGroup->m_options.m_pfnNotify( FALSE, pGroup->m_dwGroupID, pGroup->m_options.m_pContext ) ;

    NGDSS_UPDATE_MESSAGE msg ;
    msg.m_dwMsgType = NG_MSGTYPE_DSS_UPDATE ;
    msg.m_dwGroupID = pGroup->m_dwGroupID ;
    
    //-- copy the msg header
    if( pGroup->m_nTotalVars > 0 )
    {
        x_memcpy( pBuff, &msg, sizeof( msg ) ) ;
        pBuff += sizeof( msg ) ;
    }

    //-- now copy all of the variables
    for( i = 0 ; i < pGroup->m_nTotalVars ; i++ )
    {
        //-- bools are a special case because we can pack 32 of them into a u32
        //-- to save bandwidth
//        if( pGroup->m_pVars[i]->m_eType == NGDSS_BOOL )
//        {
//            
//            if( *((xbool*)(pGroup->m_pVars[i]->m_pData)) )
//            {
//                dwTemp = 1 ;
//                dwTemp = dwTemp << dwTotalBools ;
//                dwBools |= dwTemp ;
//            }
//
//            dwTotalBools++ ;
//            if( dwTotalBools == 32 || 
//                ( ((i+1) < pGroup->m_nTotalVars) && pGroup->m_pVars[i+1]->m_eType != NGDSS_BOOL ) )
//            {
//                x_memcpy( pBuff, &dwBools, sizeof( u32 ) ) ;
//                pBuff += sizeof(u32);
//                dwTotalBools = 0 ;
//                dwBools = 0 ;
//            }
//        }
//        else 
            if( pGroup->m_pVars[i]->m_eType == NGDSS_POINTER )
        {
            dwTotalBools = 0 ;
            dwBools = 0 ;
            dwTemp = 0 ;
            if( pGroup->m_pVars[i]->PointerConverter )
                dwTemp = pGroup->m_pVars[i]->PointerConverter( FALSE, (u32)(pGroup->m_pVars[i]->m_pData) ) ;
            x_memcpy( pBuff, &dwTemp, sizeof(u32) ) ;
            pBuff += sizeof( u32 ) ;

        }
        else
        {
            dwTotalBools = 0 ;
            dwBools = 0 ;

            x_memcpy( pBuff, pGroup->m_pVars[i]->m_pData, pGroup->m_pVars[i]->m_sdwSize ) ;
            pBuff += pGroup->m_pVars[i]->m_sdwSize ;
        }
    }

      
    //-- now send the update to all registered addresses
    for( i = 0 ; i < s_ngdss.m_nValidAddresses ; i++ )
    {
        NG_SendTo( hSock, s_aSendBuff, (pBuff - s_aSendBuff), &s_ngdss.m_addr[i] ) ;
    }

    //-- record the last update time
    pGroup->m_options.m_dwLastUpdate = dwTime ;
}

void NGDSS_ForceUpdateRemoteClients( NGDSS_GROUP_HANDLE hGroup, NG_SOCKET_HANDLE hSock )
{
    NGDSS_GROUP *pGroup = NGDSS_GroupFromHandle( hGroup ) ;
    ASSERT( pGroup ) ;
    if( !pGroup )
        return ;
    NGDSS_InternalSendUpdate( pGroup, hSock, x_GetSystemUpTimeMs( ) ) ;
}

void NGDSS_Tick( NG_SOCKET_HANDLE hSock )
{    
    SLSTNODEPTR *pNode ;
    NGDSS_GROUP *pGroup = NULL ;
    u32 dwTime = x_GetSystemUpTimeMs( ) ;
    

    pNode = SLNKLSTPTR_GetHeadNode( &s_ngdss.m_lstGroups ) ;
    
    while( pNode )
    {
        pGroup = (NGDSS_GROUP*)pNode->pNodeData ;
        //-- update the group
        if( (pGroup->m_bOwnership) && 
            ((dwTime - pGroup->m_options.m_dwLastUpdate) > pGroup->m_options.m_dwUpdateFrequency ) )
        {
            NGDSS_InternalSendUpdate( pGroup, hSock, dwTime ) ;
        }        
        pNode = SLNKLSTPTR_GetNextNode( &s_ngdss.m_lstGroups, pNode ) ;        
    }
}

static void NGDSS_ProcessUpdateMsg( NGDSS_UPDATE_MESSAGE *pMsg)
{
    u8 *pBuff = NULL ;
    s32 i = 0 ;

    u32 dwTotalBools = 0 ;
    u32 dwTemp = 0 ;
    u32 dwBools = 0 ;

    NGDSS_GROUP *pGroup = (NGDSS_GROUP*)NGDSS_GetGroup( pMsg->m_dwGroupID ) ;
    if( !pGroup )
    {
#ifdef NG2_DBG
        x_printf( "NGDSS: Update received but group not found GroupID [%d]\n", pMsg->m_dwGroupID ) ;
#endif
        return ;
    }
    
    // set a pointer to the start of the data
    pBuff = (u8*)pMsg ;
    pBuff += sizeof( *pMsg ) ;

    for( i = 0 ; i < pGroup->m_nTotalVars ; i++ )
    {
//        if( pGroup->m_pVars[i]->m_eType == NGDSS_BOOL )
//        {
//            if( dwTotalBools == 0 )
//            {
//                x_memcpy( &dwBools, pBuff, sizeof( u32 ) ) ;
//                pBuff += sizeof( u32 ) ;
//            }
//            dwTotalBools++ ;
//            dwTemp = 1 ;
//            dwTemp = dwTemp << dwTotalBools ;
//            if( dwBools & dwTemp )
//            {
//                *((xbool*)(pGroup->m_pVars[i]->m_pData)) = TRUE ;
//            }
//            else
//            {
//                *((xbool*)(pGroup->m_pVars[i]->m_pData)) = FALSE ;
//            }
//
//            if( dwTotalBools == 32 )
//            {
//                dwTotalBools = 0 ;
//                dwBools = 0 ;
//            }   
//            
//        }
//        else 

        //-- check the pointers we need to access
        //-- if any of these are invalid- it's a pretty serious problem
        ASSERT( pGroup->m_pVars[i] ) ;
        if( !pGroup->m_pVars[i]  )
            return ;

        ASSERT( pGroup->m_pVars[i]->m_pData ) ;
        if( !pGroup->m_pVars[i]->m_pData )
            return ;
        //-- end check the pointers

        if( pGroup->m_pVars[i]->m_eType == NGDSS_POINTER )
        {
            dwTotalBools = 0 ;
            dwBools = 0 ;
            dwTemp = 0 ;

            x_memcpy( &dwTemp, pBuff, sizeof(u32) ) ;
            pBuff += sizeof(u32 );

            if( pGroup->m_pVars[i]->PointerConverter ) 
                dwTemp = pGroup->m_pVars[i]->PointerConverter( TRUE, dwTemp ) ;

            *((u32*)(pGroup->m_pVars[i]->m_pData)) = dwTemp ;
        }
        else
        {
            dwTotalBools = 0 ;
            dwBools = 0 ;


            x_memcpy( pGroup->m_pVars[i]->m_pData, pBuff, pGroup->m_pVars[i]->m_sdwSize ) ;
            pBuff += pGroup->m_pVars[i]->m_sdwSize ;
        }
    }
    pGroup->m_options.m_dwLastUpdate = x_GetSystemUpTimeMs( ) ;
    if( pGroup->m_options.m_pfnNotify )
        pGroup->m_options.m_pfnNotify( TRUE, pGroup->m_dwGroupID, pGroup->m_options.m_pContext  ) ;
}   


///////////////////////////////////////////////////////////////////////////////
// NETGAME SDK MESSAGE HANDLER
///////////////////////////////////////////////////////////////////////////////
static void NG_MessageReceived( u32 dwMsg, const void *pData, u16 wLen, GP_SOCKETCONTEXT hSocket,
                                  const struct X_SOCKADDRIN *pAddr, void *pContext )
{
    NG_MSG_SYNCH_EVENT *pMsgSynchEvent ;
    NG_DESTINATION *pDest = (NG_DESTINATION*)pAddr ;
//    u32             *pdwRetVal = (u32*)pContext ;
    NG_SOCKET_HANDLE hSock = (NG_SOCKET_HANDLE)hSocket ;
    
    //-- only deal with application data here.  In the future we may be intereseted in acks as well
	// make sure that unreachable gets here to get handled; then handle the SOB
	if(dwMsg == GP_MESSAGE_UNREACHABLE)
	{
		// let the application know that connection fell down, went boom.
		pfnNG_TICK_CALLBACK pfn = (pfnNG_TICK_CALLBACK)GP_GetSocketCustomData( hSocket ) ;
		if( pfn )
		{
//#if !defined(X_DEBUG)
			pfn( dwMsg, pData, wLen, hSocket, pAddr, pContext );
//#endif // !defined(X_DEBUG)
		}
		return;
	}

    if( dwMsg != GP_MESSAGE_DATA )
        return ;
    
     // call the client callback.  TODO: return a value to skip default behavior
    pfnNG_TICK_CALLBACK pfn = (pfnNG_TICK_CALLBACK)GP_GetSocketCustomData( hSocket ) ;
    if( pfn )
        pfn( dwMsg, pData, wLen, hSocket, pAddr, pContext ) ;

    //-- make sure the data pointer is valid and teh length is as big as at least big enough to hold the header
    if( !pData )
        return ;
    if( wLen < sizeof(u32) )
        return ;

    //-- get the msg id
	u32 dwDataMsg =  *(u32*)pData;

    //-- process the specific messages
    switch(dwDataMsg)
    {
    case NG_MSGTYPE_TIME_RESPONSE:
        NGTIME_ProcessSynchResponse( (NG_MSG_TIME_SYNCH_REQ*)pData, hSock, pDest) ;
        //   ASBNET_DebugPrintStats( ) ;
        break ;
    case NG_MSGTYPE_TIME_REQUEST:
        NGTIME_ProcessSynchRequest( (NG_MSG_TIME_SYNCH_REQ*)pData, hSock, pDest);
        //x_printf( "Synch request received\n" ) ;
        break ;
    case NG_MSGTYPE_TIME_RTT:
        NGTIME_ProcessRTTMsg( (NG_MSG_TIME_RTT*)pData ) ;
        break ;
    case NG_MSGTYPE_SYNCH_EVENT_REQUEST:
        {
            SNGES_SYNCH_EVENT_DATA *pSynchEvent = NGES_NewSynchedEvent( ) ;
            ASSERT( pSynchEvent ) ;
            //-- don't want to access a null pointer in release mode...
            if( !pSynchEvent )
                break ;
            
            pMsgSynchEvent               = (NG_MSG_SYNCH_EVENT*)pData ;
            pSynchEvent->m_dwEventType   = pMsgSynchEvent->m_dwEventID ;
            pSynchEvent->m_dwTimeToStart = pMsgSynchEvent->m_dwTime ;
            pSynchEvent->m_sdwHandshake  = pMsgSynchEvent->m_sdwHandshake - 1;
            
            if( pMsgSynchEvent->m_sdwMemSize > 0 )
            {
                pSynchEvent->m_sdwDataSize = pMsgSynchEvent->m_sdwMemSize ;
                x_memcpy( (void*)&pSynchEvent->m_aData[0], 
                          (void*)((u8*)pData + sizeof( NG_MSG_SYNCH_EVENT )), 
                          pMsgSynchEvent->m_sdwMemSize ) ;
            }


			#ifdef X_DEBUG
				// HACK! HACK! HACK! HACK! HACK! HACK! HACK! HACK! HACK! HACK! HACK! 
				// DEBUG ONLY FOR ASB
				{
					void ASBDebugPrintNetMsg( u32 NetSynchEventID, void* pEventData, xbool bReceive );
					ASBDebugPrintNetMsg( pSynchEvent->m_dwEventType, pSynchEvent->m_aData, TRUE );
				}
			#endif


            //-- check if we should send back a response
            if( pSynchEvent->m_sdwHandshake >= 0 )
            {
                NG_MSG_SYNCH_EVENT_RESPONSE msgResp ;
                msgResp.m_dwMsgType = NG_MSGTYPE_SYNCH_EVENT_RESPONSE ;
                msgResp.m_dwEventID = pSynchEvent->m_dwEventType ;
                
                NG_GuaranteedSendTo( hSock, (u8*)&msgResp, sizeof( msgResp ), pDest ) ;
            }
            
        }
        break ;
    case NG_MSGTYPE_SYNCH_EVENT_RESPONSE:
        {
            NG_MSG_SYNCH_EVENT_RESPONSE *pMsgResp = (NG_MSG_SYNCH_EVENT_RESPONSE*)pData ;
            ASSERT( pMsgResp ) ; // for now assert, in the future just ignore the message
            if( !pMsgResp )
                break ;
            
            SNGES_SYNCH_EVENT_DATA *pSynchEvent =  NGES_GetSynchedEvent( pMsgResp->m_dwEventID ) ;
            
            
            pSynchEvent->m_sdwHandshake-- ;
            
        }
        
        break ;
    case NG_MSGTYPE_DSS_UPDATE:
        NGDSS_ProcessUpdateMsg( (NGDSS_UPDATE_MESSAGE*)pData );
        break ;
        
    default:
        break ;
    }
    return ;


}
