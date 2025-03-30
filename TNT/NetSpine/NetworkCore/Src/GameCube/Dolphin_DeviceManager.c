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

//extern "C" s32 GC_Init(u32 dev);
extern s32 GC_Init(u32 dev);
extern volatile u32 g_connectionEstablished;

static const XCHAR* dummy_data[2] = {"BBA", "Basic"};



u32 NSDM_Initialize( )
{ 
	return 0;
}


u32 NSDM_DeInitialize( )
{
	return 0;
}



u32 NSDM_GetTotalConfigurations( )
{
	return 1;
}


NSDM_ITERATOR       NSDM_GetFirstConfiguration( )
{
	return (void*) 1;
}


NSDM_ITERATOR NSDM_GetNextConfiguration( NSDM_ITERATOR it )
{
	return NULL;
}


NSDM_HCONFIGURATION NSDM_GetConfiguration( NSDM_ITERATOR it )
{
	return NULL;
}


NSDM_HCONFIGURATION NSDM_GetDefaultConfiguration( )
{
	return NULL;
}

xbool NSDM_IsConnected()
{
	ASSERTS(false, "Not implemented yet for GameCube."); 
	return true; 
}


const XCHAR* NSDM_GetConfigurationName(NSDM_HCONFIGURATION hConfig)
{ 
    //if( NULL == hConfig ) return ( NULL ); //EXIT_FAILURE
	return dummy_data[0];
}



const XCHAR* NSDM_GetDeviceName(NSDM_HCONFIGURATION hConfig) 
{
    //if( NULL == hConfig ) return ( NULL ); //EXIT_FAILURE
	return dummy_data[1];
}



NSDM_DEVICE_TYPE    NSDM_GetDeviceType( NSDM_HCONFIGURATION hConfig )
{
	return NSDM_TYPE_ERROR;
}


u32 NSDM_StartDevice( NSDM_HCONFIGURATION hConfig ) 
{
	GC_Init(0);
	return 0;
}


u32	NSDM_StopDevice( )
{
	return 0;
}


NSDM_STATUS NSDM_GetDeviceStatus( )
{
	return (g_connectionEstablished ? NSDM_STATUS_STARTED : NSDM_STATUS_DETACHED);
}


const XCHAR* NSDM_GetStatusString( NSDM_STATUS status )
{
	return NULL;
}


u32 NSDM_SetDefaultConfiguration( NSDM_HCONFIGURATION hConfig )
{
	return 0;
}


NSDM_HCONFIGURATION NSDM_GetConfigurationByName( const XCHAR *pszConfigurationName )
{
	return NULL;
}


    