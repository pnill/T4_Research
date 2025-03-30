/**
* \file     ps2NS_DeviceManager.h
*
* \brief
*
* \author   Nicholas Gonzalez
* \version  1.0
* \date		7/24/2002
*/


#ifndef __PS2NS_DEVICEMANAGER_H__
#define __PS2NS_DEVICEMANAGER_H__


/*-----------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------------*/

#define MAX_IFC_COUNT  4
#define MAX_DEV_COUNT  4
#define MAX_COMB_COUNT 6

void  PS2NSDM_PreInitialize( const XCHAR *pszIrxDir,
                             const XCHAR *pszConfDir ) ;

const XCHAR * PS2NSDM_GetDeviceDatabase( void );

u32   PS2NSDM_SetDeviceDatabase( const XCHAR *pszNewDeviceDatabase );

u32   PS2NSDM_GetExtendedConfigurationInfo( void *pPS2SpecificInfoStruct, const XCHAR *pszDisplayName );

u32   PS2NSDM_GetDEV_Count( ) ;

u32   PS2NSDM_GetIFC_Count( ) ;

u32   PS2NSDM_UnloadNetcnfIRX( ) ; // free's 14K on IOP for game

/*-----------------------------------------------------------------*/
#ifdef __cplusplus
	}
#endif
/*-----------------------------------------------------------------*/


//=====================================================================================================================================
// PS2 Power off thread required once you start up the Net Device.
//=====================================================================================================================================
extern xbool bPS2PowerOffThreadCreated;
void PreparePS2PowerOff(void);


#endif //__PS2NS_DEVICEMANAGER_H__