/**
* \file     Win32NS_DeviceManager.cpp
*
* \brief    <TODO :: add brief description>
*
* \author   Daniel Melfi 
*
* \version  1.0
*
* \date     8/30/2002
*/


#include "cpc.h"
#include "NS_DeviceManager.h"

u32                 NSDM_Initialize( ) { return 0; }
u32                 NSDM_DeInitialize( ) { return 0; }

u32                 NSDM_GetTotalConfigurations( ) { return 0; }

NSDM_ITERATOR       NSDM_GetFirstConfiguration( ) { return NULL; }
NSDM_ITERATOR       NSDM_GetNextConfiguration( NSDM_ITERATOR it ) { return NULL; }

NSDM_HCONFIGURATION NSDM_GetConfiguration( NSDM_ITERATOR it ) { return NULL; }

NSDM_HCONFIGURATION NSDM_GetDefaultConfiguration( ) { return NULL; }

const XCHAR*        NSDM_GetConfigurationName( NSDM_HCONFIGURATION hConfig ) { return NULL; }

NSDM_DEVICE_TYPE    NSDM_GetDeviceType( NSDM_HCONFIGURATION hConfig ) { return NSDM_TYPE_ERROR; }

u32                 NSDM_StartDevice( NSDM_HCONFIGURATION hConfig ) { return 0; }
u32                 NSDM_StopDevice( ) { return 0; }

NSDM_STATUS         NSDM_GetDeviceStatus( ) { return NSDM_STATUS_STARTED; }

xbool NSDM_IsConnected()
{
	ASSERTS(false, "Not implemented yet for Win32."); 
	return true; 
}

const XCHAR*        NSDM_GetStatusString( NSDM_STATUS status ) { return NULL; }

u32 NSDM_SetDefaultConfiguration( NSDM_HCONFIGURATION hConfig ) { return 0; }

NSDM_HCONFIGURATION NSDM_GetConfigurationByName( const XCHAR *pszConfigurationName ) { return NULL; }


    