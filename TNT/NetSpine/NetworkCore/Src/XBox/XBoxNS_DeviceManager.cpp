/**
* \file     XBoxNS_DeviceManager.cpp
*
* \brief    <TODO :: add brief description>
*
* \author   Daniel Melfi 
*
* \version  1.0
*
* \date     8/30/2002
*/

#include "x_files.hpp"
#include "x_sock.h" // using XSOCK_Startup to initialize the XBox device			
#include "NS_DeviceManager.h"

u32 NSDM_Initialize( void )
{
    //x_CoreRuntimeStartUp();

    return XSOCK_Startup();
}

u32 NSDM_DeInitialize( void ) 
{
	XSOCK_Shutdown();

	//x_CoreRuntimeShutDown();

	return 0;
}

xbool NSDM_IsInitialized( ) { return XSOCK_IsReady(); }

u32 NSDM_GetTotalConfigurations( )
{
	//Only one on XBox
	return 1;
}

NSDM_ITERATOR NSDM_GetFirstConfiguration( void )
{
	//TODO
	return NULL;
}

NSDM_ITERATOR NSDM_GetNextConfiguration( NSDM_ITERATOR it )
{
	//ALWAYS NULL
	return NULL;
}


u32 NSDM_StartDevice( NSDM_HCONFIGURATION hConfig )
{
	return 0;
}

u32 NSDM_StopDevice( void )
{
	return 0;
}

NSDM_STATUS NSDM_GetDeviceStatus( void )
{
	//TODO
	return NSDM_STATUS_STARTED;
}

const XCHAR * NSDM_GetStatusString( NSDM_STATUS status )
{
	//TODO
	return NULL;
}

NSDM_HCONFIGURATION NSDM_GetDefaultConfiguration( void )
{
	//TODO
	return NULL;
}

xbool NSDM_IsConnected()
{
	return (XNetGetEthernetLinkStatus() & XNET_ETHERNET_LINK_ACTIVE) != 0;
}


NSDM_HCONFIGURATION NSDM_GetConfiguration( NSDM_ITERATOR it ) { return NULL; }

const XCHAR*        NSDM_GetConfigurationName( NSDM_HCONFIGURATION hConfig ) { return NULL; }

NSDM_DEVICE_TYPE    NSDM_GetDeviceType( NSDM_HCONFIGURATION hConfig ) { return NSDM_TYPE_ERROR; }

u32 NSDM_SetDefaultConfiguration( NSDM_HCONFIGURATION hConfig ) { return 0; }

NSDM_HCONFIGURATION NSDM_GetConfigurationByName( const XCHAR *pszConfigurationName ) { return NULL; }


    