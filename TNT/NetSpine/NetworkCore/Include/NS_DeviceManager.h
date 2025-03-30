/**
* \file     NS_DeviceManager.h
*
* \brief
*
* \author   Nicholas Gonzalez
* \version  1.0
* \date		7/24/2002
*/


#ifndef __NS_DEVICEMANAGER_H__
#define __NS_DEVICEMANAGER_H__

/*-----------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------------*/


typedef void*   NSDM_ITERATOR ;
typedef void*   NSDM_HCONFIGURATION ;

typedef enum    _tagNSDM_STATUS
{
    NSDM_STATUS_DETACHED = 0,
    NSDM_STATUS_STARTING,
    NSDM_STATUS_RETRYING,
    NSDM_STATUS_STARTED,
    NSDM_STATUS_STOPPING,
    NSDM_STATUS_STOPPED,
    NSDM_STATUS_ERROR,
    NSDM_STATUS_LAST
} NSDM_STATUS ;


typedef enum _tagNSDM_DEVICE_TYPE
{
    NSDM_TYPE_INVALID = 0,
    NSDM_TYPE_PPP,           // modem PPP
    NSDM_TYPE_PPPOE,         // PPP over Ethernet
    NSDM_TYPE_ARP,           // Ethernet (static IP and DHCP)
    NSDM_TYPE_INVALID_SOURCE,// The memory card has wrong magic number (ps2 only)
	NSDM_TYPE_ERROR,
    NSDM_TYPE_LAST
} NSDM_DEVICE_TYPE ;

typedef enum _tagNSDM_ERROR_CODES
{
	NSDM_ERROR_LOAD_MODULE = 1,
	NSDM_ERROR_UNLOAD_MODULE,
	NSDM_ERROR_EENET,
	NSDM_ERROR_EZNET,
	NSDM_ERROR_LAST
} NSDM_ERROR_CODE;


/** Initialize/DeInitialize */
u32                 NSDM_Initialize( ) ;
u32                 NSDM_DeInitialize( ) ;

xbool				NSDM_IsInitialized( ) ;

/** Device/Configuration selection */
u32                 NSDM_GetTotalConfigurations( ) ;
NSDM_ITERATOR       NSDM_GetFirstConfiguration( ) ;
NSDM_ITERATOR       NSDM_GetNextConfiguration( NSDM_ITERATOR it ) ;
NSDM_HCONFIGURATION NSDM_GetConfiguration( NSDM_ITERATOR it ) ;
NSDM_HCONFIGURATION NSDM_GetConfigurationByName( const XCHAR *pszConfigurationName ) ;

/** Call this function for quick and easy startup */
NSDM_HCONFIGURATION NSDM_GetDefaultConfiguration( ) ;

/** Call this function in order to set the new default device on next startup */
u32                 NSDM_SetDefaultConfiguration( NSDM_HCONFIGURATION hConfig ) ;


/** Configuration info */
const XCHAR *       NSDM_GetConfigurationName( NSDM_HCONFIGURATION hConfig ) ;
const XCHAR *       NSDM_GetDeviceName( NSDM_HCONFIGURATION hConfig ) ;

/** Device/Configuration usage */
u32                 NSDM_StartDevice( NSDM_HCONFIGURATION hConfig ) ;
u32                 NSDM_StopDevice( ) ;
NSDM_STATUS         NSDM_GetDeviceStatus( ) ;

NSDM_DEVICE_TYPE	NSDM_GetDeviceType( NSDM_HCONFIGURATION hConfig );
xbool				NSDM_IsConnected(void);


/** Helper functions */
const XCHAR*        NSDM_GetStatusString( NSDM_STATUS status ) ;


/*-----------------------------------------------------------------*/
#ifdef __cplusplus
	}
#endif
/*-----------------------------------------------------------------*/



#endif //__NS_DEVICEMANAGER_H__