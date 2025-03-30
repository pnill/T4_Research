/**
* \file     ng2.h
*
* \brief    
*
* \author   Nicholas Gonzalez
* \version  1.0
* \date		8/19/2002
*/


#ifndef __NG2_H__
#define __NG2_H__
/*-----------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------------*/

///////////////////////////////////////////////////////////////////////////////
// SETTINGS
///////////////////////////////////////////////////////////////////////////////
//-- IF NG USE NETSIM IS SET TO 1 THEN THE LIBRARY IS BUILT WITH THE NETSIM
//-- FEATURES.  WHICH MEANS YOU CAN USE ANY OF THE NGSIM_ family of functions.
//-- OTHERWISE- The NGSIM_ family of functions will not do anything 
//-- (but they will still link)
#define NG_USE_NETSIM (0)

///////////////////////////////////////////////////////////////////////////////
// ENUMERATIONS & CONSTANTS
///////////////////////////////////////////////////////////////////////////////
enum NG_MSG_IDS
{
    NG_MSG_NULL = 0,
    NG_MSGTYPE_TIME_REQUEST,
    NG_MSGTYPE_TIME_RESPONSE,
    NG_MSGTYPE_SYNCH_EVENT_REQUEST,
    NG_MSGTYPE_SYNCH_EVENT_RESPONSE,
    NG_MSGTYPE_RANDNUM_REQUEST,
    NG_MSGTYPE_RANDNUM_RESPONSE,
    NG_MSGTYPE_TIME_RTT,
    NG_MSGTYPE_DSS_UPDATE,
    NG_MSG_LAST
} ;

enum NG_THROUGHPUT_STATS
{
    NG_STATS_INVALID,
    NG_STATS_BPS,
    NG_STATS_BPS_SENT,
    NG_STATS_BPS_RECEIVED,
    NG_STATS_TOTAL_BYTES,
    NG_STATS_BYTES_RECEIVED,
    NG_STATS_BYTES_SENT,
    NG_STATS_LAST
} ;

enum NGES_SYNCH_OPTIONS
{
    NET_SYNCH_OPT_SYNCHED = 0, // same time on both machines
    NET_SYNCH_OPT_RIGHT_AWAY, // as soon as possible on both machines (won't be synched- ready right away )
    NET_SYNCH_OPT_TWO_WAY,    // waits for a response from the receiving machine before progressing
    NET_SYNCH_OPT_LAST
} ;

enum ENG_STATUS
{
    NG_STATUS_OK,
    NG_STATUS_GENERIC_SEND_ERROR,
    NG_STATUS_GENERIC_RECEIVE_ERROR,
    NG_STATUS_GENERIC_NETWORK_ERROR,
    NG_STATUS_SEND_BUFFER_OVERFLOW,
    NG_STATUS_RECEIVE_BUFFER_OVERFLOW,
    NG_STATUS_GENERIC_MEMORY_ERROR,
    NG_STATUS_LAST
} ;
///////////////////////////////////////////////////////////////////////////////
// TYPE DEFINES & STRUCTURE DECLARARTIONS
///////////////////////////////////////////////////////////////////////////////
typedef X_SOCKADDR NG_DESTINATION ;
typedef void* NG_SOCKET_HANDLE ;
typedef void (*pfnNG_TICK_CALLBACK)(u32 dwmsg, const void *pData, u16 wLen, 
                                    NG_SOCKET_HANDLE s, const struct X_SOCKADDRIN *pAddr, void *pContext);

#define NG_MSG_HEADER \
    u32 m_dwMsgType ;

typedef u32         (*pfnNG_GETTIME)(void) ; //-- callback to get local time in milliseconds

struct NG_NETWORK_STATS ;
struct NG_NET_THROUGHPUT_STATS;
struct NG_MSG_TIME_SYNCH_REQ ;
struct NG_MSG_TIME_RTT ;
struct NG_MSG_SYNCH_EVENT ;


///////////////////////////////////////////////////////////////////////////////
// CORE COMMUNICATIONS FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
void				NG_Initialize( void );

void                NG_SetDestination( NG_DESTINATION *pDest, const XCHAR *pszAddress, s16 nPort ) ;

/** creates a socket and binds it to the nPort on the local host */
NG_SOCKET_HANDLE    NG_CreateSocketHandle( s16 nPort, pfnNG_TICK_CALLBACK pfnCallback, xbool bBroadcast, X_SOCKET = NULL ) ;

void                NG_DestroySocketHandle( NG_SOCKET_HANDLE hSock ) ;

pfnNG_TICK_CALLBACK NG_SetTickCallback( NG_SOCKET_HANDLE hSock, pfnNG_TICK_CALLBACK pfnCallback ) ;

/** Returns the size of the data received or SOCKET_ERROR */
s32                 NG_SendTo( NG_SOCKET_HANDLE hSock, u8 *pData, s32 sdwSize, NG_DESTINATION *pDest ) ;
/* Used for Xbox voice sends */
s32                 NG_SendToVoice( NG_SOCKET_HANDLE hSock, u8 *pData, s32 sdwSize, NG_DESTINATION *pDest ) ;

/** Returns the size of the data received or SOCKET_ERROR */
s32                 NG_GuaranteedSendTo( NG_SOCKET_HANDLE hSock, u8 *pData, s32 sdwSize, NG_DESTINATION *pDest ) ;

u32                 NG_GetThroughputStats( NG_SOCKET_HANDLE hSock, u32 dwStatToRetrieve ) ;

void                NG_Tick( NG_SOCKET_HANDLE hSock, void *pContext ) ;

ENG_STATUS          NG_GetStatus( ) ;

///////////////////////////////////////////////////////////////////////////////
// NETWORK SIMULATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
void NGSIM_SetLatency  (NG_SOCKET_HANDLE hSock, u32 dwLatency );
void NGSIM_SetDropRate (NG_SOCKET_HANDLE hSock, u32 dwDropRate );
void NGSIM_SetNextDropCount(NG_SOCKET_HANDLE hSock, u32 dwNextDropCount );


///////////////////////////////////////////////////////////////////////////////
// CLOCK SYNCHRONIZATION & STATS FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

void                NGTIME_InitTimeServer( pfnNG_GETTIME pfnGetTime ) ;
void                NGTIME_InitTimeClient( pfnNG_GETTIME pfnGetTime, NG_DESTINATION *pTimeServer ) ;


void                NGTIME_SetTimeServer( NG_DESTINATION *pDest ) ;

s32                 NGTIME_SendSynchRequest( NG_SOCKET_HANDLE sock, NG_DESTINATION *pDest ) ;

/** Only valid on clients */
void                NGTIME_SetUpdateFrequency( u32 dwMs ) ;
u32                 NGTIME_GetUpdateFrequency( ) ;

//** should be called when a synch request is received */
s32                 NGTIME_ProcessSynchRequest( NG_MSG_TIME_SYNCH_REQ *pMsg, NG_SOCKET_HANDLE s, NG_DESTINATION *pDest ) ;

/** should be called when a synch response is received */
void                NGTIME_ProcessSynchResponse( NG_MSG_TIME_SYNCH_REQ *pMsg, NG_SOCKET_HANDLE s, NG_DESTINATION *pDest ) ;

/** shoudl be called when a rtt msg is received (only on the host ) */
void                NGTIME_ProcessRTTMsg( NG_MSG_TIME_RTT *pMsg ) ;

/** Returns the current time on the time server.  Returns right away (doesn't actually go to the network)*/
u32                 NGTIME_GetGlobalTime( ) ;
NG_NETWORK_STATS*   NGTIME_GetNetworkStats() ;

void				NGTIME_GetNetThroughputStats(NG_SOCKET_HANDLE sock, NG_NET_THROUGHPUT_STATS *pStats);

///////////////////////////////////////////////////////////////////////////////
// EVENT SYNCHRONIZATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
void    NGES_Init( ) ;
s32     NGES_SynchEvent( NG_SOCKET_HANDLE s, NG_DESTINATION *pDest, u32 dwSynchEvent, u32 dwOptions = 0  ) ;
u32     NGES_IsSynchedEventReady( u32 dwSynchEvent ) ;

s32     NGES_SynchDataEvent( NG_SOCKET_HANDLE s, NG_DESTINATION *pDest, u32 dwSynchEvent, void *pMem, s32 sdwSize, u32 dwOptions = 0 ) ;
u32     NGES_IsSynchedDataEventReady( u32 dwSynchEvent, void **ppData, s32 *psdwSize ) ;

void    NGES_ClearPendingEvents( u32 dwEventType ) ;
void    NGES_ClearAllPendingEvents( ) ;


// NGES_GetQueuedEvent:
//		Pass NULL for hEventHandle to get first event in queue, or current valid handle to get
//		the next queued event.  dwSynchEvent, ppData, and psdwSize are filled with event info.
//		Returns valid event handle value, NULL if no queued events exist.
u32     NGES_GetNextQueuedEvent( u32 hEventHandle, u32* dwSynchEvent, void** ppData, s32* psdwSize );

// NGES_RemoveQueuedEvent:
//		Behaves the same as NGES_GetNextQueuedEvent(), except the event that hEventHandle points to
//		will be removed from the queue.  If hEventHandle is NULL, nothing is removed and next valid
//		event handle is returned.
u32     NGES_RemoveQueuedEvent( u32 hEventHandle, u32* dwSynchEvent, void** ppData, s32* psdwSize );


// NGES_DBG_DumpQueue:
//      Prints contents of event queue to the TTY
void    NGES_DBG_DumpQueue( u32 Unused );


///////////////////////////////////////////////////////////////////////////////
// DATA SYNCHRONIZATION SYSTEM FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
typedef void (*pfnNGDDS_UPDATE_NOTIFICATION)( xbool bIncoming, u32 dwGroupID, void *pContext) ;
typedef u32  (*pfnPOINTER_CONVERTER)(xbool bReturnPointer, u32 dwData ) ;

struct  NGDSS_GROUP_OPTIONS ;
typedef void* NGDSS_GROUP_HANDLE ;

#define NGDSS_GROUP_OPT_NONE        (0)
#define NGDSS_GROUP_OPT_FREQUENCY   (0x00000001)
#define NGDSS_GROUP_OPT_CALLBACK    (0x00000002)
#define NGDSS_GROUP_OPT_LASTUPDATE  (0x00000004)
#define NGDSS_GROUP_OPT_ALL         (0x00000007)

enum NGDSS_TYPE
{  
    NGDSS_INVALID = 0, 
    NGDSS_U8, NGDSS_S8, NGDSS_U16, NGDSS_S16, NGDSS_BOOL, 
    NGDSS_U32, NGDSS_S32, NGDSS_F32, NGDSS_F64, NGDSS_U64, NGDSS_S64, 
    NGDSS_BLOB, NGDSS_POINTER, NGDSS_GROUP_TYPE, 
    NGDSS_TOTAL_TYPES 
} ;

/** Core functions */
void                    NGDSS_Init( ) ;
void                    NGDSS_DeInit( ) ;
void                    NGDSS_AddRemoteAddress( NG_DESTINATION *pDest ) ;
void                    NGDSS_RemoveRemoteAddress( NG_DESTINATION *pDest ) ;
void                    NGDSS_ResetAddressList( NG_DESTINATION *pDest ) ;

/** Used to traverse the tree, if the group doesn't exist it will be created */
NGDSS_GROUP_HANDLE      NGDSS_CreateGroup( u32 dwGroupID ) ;
void                    NGDSS_DestroyGroup( NGDSS_GROUP_HANDLE hGroup ) ;
NGDSS_GROUP_HANDLE      NGDSS_GetGroup( u32 dwGroupID ) ;



/** Variable registration functions */
void                    NGDSS_RegisterVariable( NGDSS_GROUP_HANDLE hGroup, NGDSS_TYPE eDataType, void *pSrc ) ;
void                    NGDSS_RegisterBLOB( NGDSS_GROUP_HANDLE hGroup, void *pSrc, s32 sdwSize ) ;
void                    NGDSS_RegisterPointer( NGDSS_GROUP_HANDLE hGroup, void **pPointer, pfnPOINTER_CONVERTER pfnCallbaCk ) ;

/** Group options */
void                    NGDSS_SetGroupOptions( NGDSS_GROUP_HANDLE hGroup, NGDSS_GROUP_OPTIONS *pOptions, u32 dwValidFields ) ;
void                    NGDSS_GetGroupOptions( NGDSS_GROUP_HANDLE hGroup, void *pOptValue, u32 dwField ) ;
/** If you own a group you will send out updates, if not you will be expecting updates */
void                    NGDSS_SetGroupOwnership( NGDSS_GROUP_HANDLE hGroup, xbool bOwn ) ;
xbool                   NGDSS_GetGroupOwnership( NGDSS_GROUP_HANDLE hGroup ) ;

/** Update functions */
/** Forces an update of the current group that we are in */
void                    NGDSS_ForceUpdateRemoteClients( NGDSS_GROUP_HANDLE hGroup, NG_SOCKET_HANDLE hSock ) ;
void                    NGDSS_Tick( NG_SOCKET_HANDLE hSock ) ;




///////////////////////////////////////////////////////////////////////////////
// STRUCTURE DEFINITIONS
///////////////////////////////////////////////////////////////////////////////
struct NGDSS_GROUP_OPTIONS
{
    u32                             m_dwUpdateFrequency ;
    pfnNGDDS_UPDATE_NOTIFICATION    m_pfnNotify ;
    void*                           m_pContext ;
    u32                             m_dwLastUpdate ;
} ;

struct NG_NETWORK_STATS
{
    f32     m_fAverageRTT ; //-- the average round trip time
    u32     m_dwTotalRTT ;  //-- the total rtt
    u32     m_dwRTTCount ;  //-- the number of samples used to compute the total and the average
    u32     m_dwMinRTT ;    //-- the minimum rtt
    u32     m_dwMaxRTT ;    //-- the maximum rtt
    f32     m_fTimeDiff ;   //-- the time difference to that machine
} ;

struct NG_NET_THROUGHPUT_STATS
{
	u32 m_dwTotalTxBytes;
	u32 m_dwTotalRxBytes;
	u32 m_dwTotalTxRxBytesPerSec;
	u32 m_dwTotalTxBytesPerSec;
	u32 m_dwTotalRxBytesPerSec;
};


struct NG_MSG_TIME_SYNCH_REQ
{
    NG_MSG_HEADER
    u32     m_dwClientTime ;
    u32     m_dwServerTime ;
} ;

struct NG_MSG_TIME_RTT //send RTT info to the host so he has it too
{
    NG_MSG_HEADER
    u32     m_dwRTT ;
} ;

struct NG_MSG_SYNCH_EVENT
{
    NG_MSG_HEADER
    u32     m_dwEventID ;
    s32     m_sdwHandshake ; //-- how many times to acknowledge this message
    u32     m_dwTime ; //-- if time is 0, it will attempt to synch it asap
    s32     m_sdwMemSize ; //-- if this is non zero m_sdwMemSize memory follows
} ;

struct NG_MSG_SYNCH_EVENT_RESPONSE
{
    NG_MSG_HEADER
    u32     m_dwEventID ;
} ;


/*-----------------------------------------------------------------*/
#ifdef __cplusplus
	}
#endif
/*-----------------------------------------------------------------*/


#endif //__NG2_H__