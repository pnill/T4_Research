//=====================================================================================================================================
// INCLUDE
//=====================================================================================================================================

#include "eetypes.h"

#include "x_files.hpp"
#include "x_sock.h"                 // for LibEEnet functions
#include "NS_DeviceManager.h"
#include "PS2NS_DeviceManager.h"
#include "eenetctl.h"
#include "nonport.h"
#include <netcnf.h>

#define _IN_H
#include <libeenet.h>
#include <libmc.h>
#include <libcdvd.h>
#include <netcnfif.h>
#include "eznetcnf.h"
#include <sifrpc.h>
#include <libdev.h>

#include <eekernel.h>
#include <sifdev.h>
#include <libcdvd.h>
#include <libeenet/net/if.h>
 
//=====================================================================================================================================
// DEFINE
//=====================================================================================================================================
#define	A_DError		x_printf
#define	A_DWarning		x_printf
#define A_DStatus		x_printf
#define A_DDetail		x_printf
#define	A_Dbg           x_printf

#define X_SAFE_FREE(p)  if((p) != NULL){ x_free((p));(p) = NULL; }
#define	EZNETCTL_ARG    "-no_auto" "\0" "-no_decode"

#define EENET_MEMSIZE   (300 * 1024) // 300 KB
#define EENET_TPL       32
#define EENET_APP_PRIO  48
#define EENET_IFNAME    "smap0"

#define Ev_Attach           0x01
#define Ev_UpCompleted      0x02
#define Ev_DownCompleted    0x04
#define Ev_DetachCompleted  0x08

#define EZ_NET_Wait_Time    (30 * 1000)

//=====================================================================================================================================
// STRUCTS and TYPEDEFS
//=====================================================================================================================================
typedef enum __EPS2CONNECTION_TYPE
{
    eConnection_Invalid = 0,
    eConnection_PPP,           // modem PPP
    eConnection_PPPoE,         // PPP over Ethernet
    eConnection_ARP,           // Ethernet (static IP and DHCP)
    eConnection_Error,
    eConnection_Last

} EPS2CONNECTION_TYPE;


//-------------------------------------------------------------------------------------------------------------------------------------
/**
 *  network state constants
 *  equivalent to sceINETCTL_S_* macros in <iop/include/inetctl.h>
 */
typedef enum __EPS2DEVICE_STATUS
{
    eState_Detached = 0,
    eState_Starting,
    eState_Retrying,
    eState_Started,
    eState_Stopping,
    eState_Stopped,
    eState_Error,
    eState_Last

} EPS2DEVICE_STATUS;


//-------------------------------------------------------------------------------------------------------------------------------------
struct PS2DM_DATA
{
    u32     m_dwTotalDevices ;

    ezNetCnfCombinationList_t *pCombinationList_UA;
    ezNetCnfCombinationList_t *pCombinationList;

    ezNetCnfCombination_t     *pCurrentCombination;

    sceNetcnfifData_t         *pNetCnfifData_UA;
    sceNetcnfifData_t         *pNetCnfifData;

    ezNetCtlStatus_t          *pNetCtlStatus_UA;
    ezNetCtlStatus_t          *pNetCtlStatus;

    XCHAR   m_szIrxDir[256];
    XCHAR   m_szConfDir[256];
    XCHAR   m_szDeviceDatabaseLocation[256]; // If not set will default to mem-card 0
    
    xbool   m_bIrxLoaded ;                   // used to prevent systems calls that will crash if IRX not loaded
} ;


//=====================================================================================================================================
// ENUMS
//=====================================================================================================================================
enum
{
    PSDM_NETCNF_IRX = 0,    // NETCNF.irx

    PSDM_ENT_DEVM_IRX,      // ENT_DEVM.irx,
    PSDM_ENT_SMAP_IRX,      // ENT_SMAP.irx

    PSDM_EENETCTL_IRX,      // EENETCTL.irx
    PSDM_EZNETCNF_IRX,      // EZNETCNF.irx
    PSDM_EZENTCTL_IRX,      // EZENTCTL.irx

    PSDM_DEV9_IRX,          // DEV9.irx
//    PSDM_PPP_IRX,           // PPP.IRX
//    PSDM_PPPOE_IRX,         // PPPOE.IRX

    PSDM_STR_TOTAL
} ;

//=====================================================================================================================================
// STATIC Vars
//=====================================================================================================================================
static PS2DM_DATA g_DevMgr;
static xbool      g_bInitialized = FALSE;
static u32        g_LoadedIRXs = 0;
static xbool      g_bSifRpcInit = FALSE;
static xbool	  g_bLoadEENetStack = FALSE;
static xbool	  g_bLoadEENetCtl = FALSE;
static xbool	  g_bInterfaceUp = FALSE;

//-------------------------------------------------------------------------------------------------------------------------------------
//-- NG- STATIC STRINGS FOR THIS SCREEN AND ASSOCIATED DIALOG BOXES
static u16 g_PSDMIrxMasks[PSDM_STR_TOTAL] =
{ 
	0x0001,  // NETCNF_MASK

	0x0002,  // ENT_DEVM_MASK
	0x0004,  // ENT_SMAP_MASK

    0x0008,  // eenetctl_MASK
	0x0010,  // EZNETCNF_MASK
	0x0020,  // EZENTCTL_MASK

    0x0040,  // DEV9_MASK
//    0x0080,  // PPP_MASK
//    0x0100,  // PPPOE_MASK
} ;

//-------------------------------------------------------------------------------------------------------------------------------------
//-- NG- STATIC STRINGS FOR THIS SCREEN AND ASSOCIATED DIALOG BOXES
static char * g_strPSDMStrings[PSDM_STR_TOTAL] =
{    
    "NETCNF.IRX",
    "ENT_DEVM.IRX",
    "ENT_SMAP.IRX",
    "EENETCTL.IRX",
	"EZNETCNF.IRX",
	"EZENTCTL.IRX",
	"DEV9.IRX",
//    "PPP.IRX",
//    "PPPO3.IRX"
} ;

static void * eenet_pool;
static void * eenet_pool_UA;

static int sSema_ID = -1;
static int sEvent_Flag = 0;

//=====================================================================================================================================
// Static functions.
//=====================================================================================================================================
static void event_handler(const char *ifname, int af, int type)
{
	A_DStatus("event_handler: event happened. af = %d, type = %d\n", af, type);

	switch(type)
	{
	case sceEENETCTL_IEV_Attach:
		if(x_strcmp(ifname, EENET_IFNAME))
			break;
		sEvent_Flag |= Ev_Attach;
		SignalSema(sSema_ID);
		break;
	case sceEENETCTL_IEV_UpCompleted:
		sEvent_Flag |= Ev_UpCompleted;
		SignalSema(sSema_ID);
		break;
	case sceEENETCTL_IEV_DownCompleted:
		sEvent_Flag |= Ev_DownCompleted;
		SignalSema(sSema_ID);
		break;
	case sceEENETCTL_IEV_DetachCompleted:
		if(x_strcmp(ifname, EENET_IFNAME))
			break;
		sEvent_Flag |= Ev_DetachCompleted;
		SignalSema(sSema_ID);
		break;
	}

	return;
}


//-------------------------------------------------------------------------------------------------------------------------------------
static void WaitEvent( s32 EventID )
{
    s32 ReturnValue;

    // Wait forever
	while(1)
    {
        ReturnValue = WaitSema(sSema_ID);

        if( ReturnValue == sSema_ID )
        {
            if(sEvent_Flag & (EventID))
            {
                break;
            }
	    }
        else if( ReturnValue == -1 )
        {
            // This should not happen.
            ASSERT( 0 );
        }
    }
}



//-------------------------------------------------------------------------------------------------------------------------------------
static int create_sema(int init_count, int max_count){
	struct SemaParam sema_param;
	int NewSema_ID;

	x_memset(&sema_param, 0, sizeof(struct SemaParam));
	sema_param.initCount = init_count;
	sema_param.maxCount = max_count;
	NewSema_ID = CreateSema(&sema_param);

	return NewSema_ID;
}


/*========================================================================
 * Function : IsModuleLoaded()
 * Date     : 1/6/2003
 * Added by : Daniel Melfi
 */
static xbool IsModuleLoaded( u16 wIrxID ) 
{
	return ( g_PSDMIrxMasks[wIrxID] & g_LoadedIRXs ) ? TRUE : FALSE;
}



/*========================================================================
 * Function : LoadPS2DevMngrIRX()
 * Date     : 12/4/2002
 * Added by : Daniel Melfi
 */
static u32 LoadPS2DevMngrIRX( xbool bIsOnCD, u16 wIrxID, int args, char *argv )
{
	char szTempBuff[512];
	int result=0;

	if( IsModuleLoaded(wIrxID) )
	{
		A_DDetail(">>Module %s already loaded\n", g_strPSDMStrings[wIrxID] );
		//EXIT_SUCCESS
		return ( 0 );
	}

	A_DDetail(">>BEFORE module %s is loaded sceSifQueryMaxFreeMemSize()=%d\n", g_strPSDMStrings[wIrxID], sceSifQueryMaxFreeMemSize());

	x_sprintf( szTempBuff, "%s%s%s", g_DevMgr.m_szIrxDir, g_strPSDMStrings[wIrxID], (bIsOnCD) ? ";1" : "" ) ;
	while ( (result = sceSifLoadModule( szTempBuff, args, argv) ) < 0 )
	{
		A_DError("Can't load module %s\n",szTempBuff);
		switch(result)
		{
			case SCE_EBINDMISS :    A_DError("\nBinding to the IOP module failed\n"); break;
			case SCE_EVERSIONMISS : A_DError("The IOP module version does not match\n"); break;
			case SCE_ECALLMISS:     A_DError("Loading IRX failed : SCE_ECALLMISS\n"); break;
			case SCE_ENXIO:         A_DError("Loading IRX failed : SCE_ENXIO\n"); break;
			case SCE_EBADF:         A_DError("Loading IRX failed : SCE_EBADF\n"); break;
			case SCE_ENODEV:        A_DError("Loading IRX failed : SCE_ENODEV\n"); break;
			case SCE_EINVAL:        A_DError("Loading IRX failed : SCE_EINVAL\n"); break;
			case SCE_EMFILE:        A_DError("Loading IRX failed : SCE_EMFILE\n"); break;
			case SCE_ETYPEMISS:     A_DError("Loading IRX failed : SCE_ETYPEMISS\n"); break;
			case SCE_ELOADMISS:     A_DError("Loading IRX failed : SCE_ELOADMISS\n"); break;
			default :               A_DError("Unhandled error code %x\n",result); break;
		}
		
        //EXIT_FAILURE
        ASSERT( 0 );
		return ( NSDM_ERROR_LOAD_MODULE );

	}

	A_DDetail(">>AFTER module %s was loaded sceSifQueryMaxFreeMemSize()=%d\n", g_strPSDMStrings[wIrxID], sceSifQueryMaxFreeMemSize());

	g_LoadedIRXs |= g_PSDMIrxMasks[wIrxID] ;

	//EXIT_SUCCESS
	return ( 0 );
}

/*========================================================================
 * Function : UnloadPS2DevMngrIRX()
 */
static u32 UnloadPS2DevMngrIRX( u16 wIrxID )
{
	if (!IsModuleLoaded(wIrxID))
		return NSDM_ERROR_UNLOAD_MODULE;

	// clear bitmask for module
	g_LoadedIRXs &= ~g_PSDMIrxMasks[wIrxID];

    char ModuleName[16];
	x_memset( ModuleName, 0, sizeof(ModuleName) );

	// chop off the "irx" to get the core module name
	x_strncpy( ModuleName, g_strPSDMStrings[wIrxID], x_strlen(g_strPSDMStrings[wIrxID]) - 4 );
	x_strtolower( ModuleName );

	// find IOP module by name
	s32 ModuleID = sceSifSearchModuleByName( ModuleName );
	if( ModuleID < 0 )
	{
		A_DError( "UnloadPS2DevMngrIRX: Couldn't find module %s\n", ModuleName );
        // Return the error.
		return NSDM_ERROR_UNLOAD_MODULE;
	}

    // stop IOP module
	s32 rcode;
	if( sceSifStopModule( ModuleID, 0, NULL, &rcode) < 0 )
	{
		A_DError( "UnloadPS2DevMngrIRX: Error stopping module %s. Error code: %d\n", ModuleName, rcode );
        ASSERT( 0 );
        return NSDM_ERROR_UNLOAD_MODULE;
	}
	
	// unload IOP module
	A_DDetail(">>BEFORE module %s is unloaded sceSifQueryMaxFreeMemSize()=%d\n", g_strPSDMStrings[wIrxID], sceSifQueryMaxFreeMemSize());
	if( sceSifUnloadModule( ModuleID ) < 0 )
	{
		A_DError( "UnloadPS2DevMngrIRX: Error unloading module %s. Error code: %d\n", ModuleName, rcode );
        ASSERT( 0 );
        return NSDM_ERROR_UNLOAD_MODULE;
	}
	A_DDetail(">>AFTER module %s was unloaded sceSifQueryMaxFreeMemSize()=%d\n", g_strPSDMStrings[wIrxID], sceSifQueryMaxFreeMemSize());

	return 0;
}

/*========================================================================
 * Function : NSDM_IsInitialized()
 * Date     : 1/2/2003
 * Added by : Daniel Melfi
 */
xbool NSDM_IsInitialized( ) { return g_bInitialized; }


/*========================================================================
 * Function : PS2NSDM_PreInitialize()
 */
void  PS2NSDM_PreInitialize( const XCHAR *pszIrxDir,
                             const XCHAR *pszConfDir )
{
    if( TRUE==g_bInitialized || !pszIrxDir || !pszConfDir )
        //EXIT_FAILURE
        return ;


    // -- set defaults for the structure
    x_memset( &g_DevMgr, 0, sizeof(PS2DM_DATA) );

    x_strcpy( g_DevMgr.m_szIrxDir, pszIrxDir ) ;
    x_strcpy( g_DevMgr.m_szConfDir, pszConfDir ) ;
    x_strcpy( g_DevMgr.m_szDeviceDatabaseLocation, "mc0:" );
    g_bInitialized = TRUE;
}


/*========================================================================
 * Function : LoadPS2CombinationLists()
 * Date     : 7/26/2002
 */
static u32 LoadPS2CombinationLists( void )
{
	A_DDetail( "LoadPS2CombinationLists\n" );

    // save this data for future use
    if( !g_DevMgr.pCombinationList_UA ) g_DevMgr.pCombinationList_UA = (ezNetCnfCombinationList_t*) x_malloc ( sizeof(ezNetCnfCombinationList_t) + 64 ) ;
    if( NULL == g_DevMgr.pCombinationList_UA )
    {
        ASSERT( 0 );
        //EXIT_FAILURE
        return ( NSDM_TYPE_ERROR );
    }
	x_memset( g_DevMgr.pCombinationList_UA, 0, sizeof(ezNetCnfCombinationList_t) + 64 );
	
	A_DDetail( "LoadPS2CombinationLists: After 1st allocation\n" );

    //ALIGN THIS SUCKA!!!
    g_DevMgr.pCombinationList = (ezNetCnfCombinationList_t*)ALIGN_64(g_DevMgr.pCombinationList_UA);

    if( !g_DevMgr.pNetCtlStatus_UA ) g_DevMgr.pNetCtlStatus_UA = (ezNetCtlStatus_t*) x_malloc( sizeof(ezNetCtlStatus_t) + 64 ) ;
	
    if( NULL == g_DevMgr.pNetCtlStatus_UA )
    {
        X_SAFE_FREE( g_DevMgr.pCombinationList_UA ) ;
        g_DevMgr.pCombinationList = NULL;
        //EXIT_FAILURE
        ASSERT( 0 );
        return ( NSDM_TYPE_ERROR );
    }
	x_memset( g_DevMgr.pNetCtlStatus_UA, 0, sizeof(ezNetCtlStatus_t) + 64 );

	A_DDetail( "LoadPS2CombinationLists: After 2nd allocation\n" );

    //ALIGN THIS SUCKA!!!
    g_DevMgr.pNetCtlStatus = (ezNetCtlStatus_t*)ALIGN_64(g_DevMgr.pNetCtlStatus_UA);

    if( !g_DevMgr.pNetCnfifData_UA ) 
    {
        g_DevMgr.pNetCnfifData_UA = (sceNetcnfifData_t*) x_malloc( sizeof(sceNetcnfifData_t) + 64 );
    }

    if( NULL == g_DevMgr.pNetCnfifData_UA )
    {
        X_SAFE_FREE( g_DevMgr.pNetCtlStatus_UA );
        X_SAFE_FREE( g_DevMgr.pCombinationList_UA );

        g_DevMgr.pNetCtlStatus = NULL;
        g_DevMgr.pCombinationList = NULL;

        //EXIT_FAILURE
        ASSERT( 0 );
        return ( NSDM_TYPE_ERROR );
    }
	x_memset( g_DevMgr.pNetCnfifData_UA, 0, sizeof(sceNetcnfifData_t) + 64 );

	A_DDetail( "LoadPS2CombinationLists: After deallocations\n" );

    //ALIGN THIS SUCKA!!!
    g_DevMgr.pNetCnfifData = (sceNetcnfifData_t*)ALIGN_64(g_DevMgr.pNetCnfifData_UA);

    if( g_DevMgr.pNetCtlStatus ) g_DevMgr.pNetCtlStatus->state = eState_Detached ;

	FlushCache(0);
	int ret = ezNetCnfGetCombinationList( g_DevMgr.m_szDeviceDatabaseLocation, 0, g_DevMgr.pCombinationList );

	if (ret < 0 )
    {
		A_DDetail( "LoadPS2CombinationLists: ezNetCnfGetCombinationList returned %d\n", ret );
        if( ret == sceNETCNFIF_MAGIC_ERROR )
        {
            return NSDM_TYPE_INVALID_SOURCE;
        }
        else
        {
            ASSERT( 0 );
            return NSDM_TYPE_ERROR;
        }
    }
	
	A_DDetail( "LoadPS2CombinationLists: ezNetCnfGetCombinationList\n" );

	// get config count
	if( g_DevMgr.pCombinationList )
	{		
		s32 i;
		for( i=0; i<g_DevMgr.pCombinationList->listLength && i<kMaxCombinations; i++ )
		{
			if( g_DevMgr.pCombinationList->netdbOrder[i] > 0 )
				g_DevMgr.m_dwTotalDevices++;
		}
	}

	A_DDetail( "LoadPS2CombinationLists: returning successfully\n" );

    return 0 ;   
	
}

/*========================================================================
 * Function : LoadEENetStack()
 * Date     : 12/4/2002
 * Added by : Daniel Melfi
 */
static u32 LoadEENetStack()
{
	s32 rcode;

	// create a semaphore
    A_DDetail( "LoadEENetStack()--> Creating new semaphore.\n" );
	sSema_ID = create_sema(0, 1);
	if(sSema_ID < 0)
	{
    	A_DError("create_sema() failed.\n");
        ASSERT( 0 );
		return NSDM_ERROR_EENET;
	}

	// allocate memory for EENet it must be 64 byte aligned
    A_DDetail( "LoadEENetStack()-->Allocating memory for LibEENet.\n" );
	eenet_pool_UA = x_malloc(EENET_MEMSIZE + 64);
    ASSERT( eenet_pool_UA );

	eenet_pool = (void *)ALIGN_64(eenet_pool_UA);

	if(eenet_pool == NULL)
    {
        return NSDM_ERROR_EENET;
    }

    A_DDetail( "LoadEENetStack()-->Memory allocation PASSED!.\n" );

	// initialize eenet
    A_DDetail( "LoadEENetStack()-->Starting sceEENetInit:\n" );
	rcode = sceEENetInit( eenet_pool, EENET_MEMSIZE, EENET_TPL, 8192, EENET_APP_PRIO );
	if(rcode < 0)
	{
        A_DDetail( "LoadEENetStack()-->Initializing LibEENet: Failed!!!:(%d)\n", rcode );
		return NSDM_ERROR_EENET;
	}

    A_DDetail( "LoadEENetStack()-->Initializing LibEENet: PASSED!!\n" );

	return 0;
}

//-------------------------------------------------------------------------------------------------------------------------------------
static u32 KillEENetStack( void )
{
    s32 rcode;

	rcode = sceEENetTerm();
	if(rcode < 0)
	{
		A_DError("sceEENetTerm failed (%d)\n", rcode);
        ASSERT( 0 );
        return ( NSDM_TYPE_ERROR );
	}

    // free the EENet pool
	if(eenet_pool_UA != NULL)
    {
        x_free(eenet_pool_UA);
        eenet_pool_UA = NULL;
        eenet_pool    = NULL;
    }

    return 0;
}

/*========================================================================
 * Function : LoadEENetCtl()
 * Date     : 12/4/2002
 * Added by : Daniel Melfi
 */
static u32 LoadEENetCtl( void )
{
	s32 rcode;

    // init eenetctl.a
	rcode = sceEENetCtlInit(8192, EENET_APP_PRIO, 8192, EENET_APP_PRIO, 8192, EENET_APP_PRIO, 1, 0);
	if(rcode < 0)
	{
		A_DError("sceEENetCtlInit failed (%d)\n", rcode);
		return NSDM_ERROR_EENET;
	}

    // add event handler
	rcode = sceEENetCtlRegisterEventHandler(event_handler);
	if(rcode < 0)
	{
		A_DError("sceEENetCtlRegisterEventHandler failed (%d)\n", rcode);
		return NSDM_ERROR_EENET;
	}
	
	// Register the network device with EENet.
	rcode = sceEENetDeviceSMAPReg(8192, 8192);
	if(rcode < 0)
	{
		A_DError("sceEENetDeviceSMAPReg failed (%d)\n", rcode);
		return NSDM_ERROR_EENET;
	}
	else
	{
        // wait until target interface is attached
		WaitEvent(Ev_Attach);
	}


	// wait for interface initialization to complete
	//WaitEvent(Ev_UpCompleted);
	return rcode;
}

/*========================================================================
 * Function : NSDM_Initialize()
 * Date     : 7/26/2002
 */
#define DOERROR(x) { error=x; goto ERROR_LABEL; }
u32 NSDM_Initialize( )
{
	u32 error = 0;

	A_DDetail( "NSDM_Initialize()--> Begin NSDM_Initialize() sceSifQueryMaxFreeMemSize()=%d\n",sceSifQueryMaxFreeMemSize());

    if( !g_bInitialized )
    {
        A_DError( "Network not initialized...\n" );
        //EXIT_FAILURE
        ASSERT( 0 );
        return ( NSDM_TYPE_ERROR );
    }

	xbool bIsOnCD = (0==x_strncmp(g_DevMgr.m_szIrxDir,"cdrom",5));

#ifdef X_DEBUG
//    const char  inetArgs[]      = "debug=ff";
//    const char  inetctlArgs[]   = "-no_auto\0-verbose";
#else
//    const char  inetArgs[]      = "debug=00";
//    const char  inetctlArgs[]   = "-no_auto";
#endif

    char netcnfArgs[256]="";

    x_memset( netcnfArgs, 0, sizeof(netcnfArgs) );

    x_strcat( netcnfArgs, "icon=" );
    x_strcat( netcnfArgs, g_DevMgr.m_szConfDir );
	x_strcat( netcnfArgs, "SYS_NET.ICO" );
	if( bIsOnCD ) x_strcat( netcnfArgs, ";1" );

    u32 dwNetcnfArgsCount = x_strlen( netcnfArgs );
    char *p = netcnfArgs + dwNetcnfArgsCount + 1;

    x_strcat( p, "iconsys=" );
	x_strcat( p, g_DevMgr.m_szConfDir );
	x_strcat( p, "ICON.SYS" );
	if( bIsOnCD ) x_strcat( p, ";1" );

    dwNetcnfArgsCount += x_strlen(p) + 2 ;
	
    //=====================================================================================================================================
    // - Force the SIF to reset.
    // - Then load all neccessary IRX files to the IOP.
    //=====================================================================================================================================
	if( !g_bSifRpcInit )
    {
        A_DDetail( "NSDM_Initialize()--> Initializing SIF\n" );

        sceSifInitRpc(0);
	    g_bSifRpcInit = TRUE;
    }

    A_DDetail( "NSDM_Initialize()--> Loading IRX files\n" );

	// Load the device manager IRX file.
    //-------------------------------------------------------------------------------------------------------------------------------------
    if( LoadPS2DevMngrIRX( bIsOnCD, PSDM_ENT_DEVM_IRX, 0, NULL ) != 0 )
        DOERROR( NSDM_ERROR_LOAD_MODULE );

    // Load the network configuration library.
    //-------------------------------------------------------------------------------------------------------------------------------------
	if( LoadPS2DevMngrIRX( bIsOnCD, PSDM_NETCNF_IRX, dwNetcnfArgsCount, netcnfArgs) != 0 )
        DOERROR( NSDM_ERROR_LOAD_MODULE );

    // Load the common network library IOP module.
    //-------------------------------------------------------------------------------------------------------------------------------------
	if( LoadPS2DevMngrIRX( bIsOnCD, PSDM_EENETCTL_IRX, 0, NULL ) !=0 )
        DOERROR( NSDM_ERROR_LOAD_MODULE );

    // Load the HDD \ Ethernet Adapter support drivers. (This must be loaded before PSDM_ENT_SMAP_IRX).
    // This basically just enables the bus that connects from the PS2 to the add on peripherals.
    //-------------------------------------------------------------------------------------------------------------------------------------
    if( LoadPS2DevMngrIRX( bIsOnCD, PSDM_DEV9_IRX, 0, NULL ) != 0 )
        DOERROR( NSDM_ERROR_LOAD_MODULE );

    // Load the "SCE Ethernet Adapter" Drivers.
    //-------------------------------------------------------------------------------------------------------------------------------------
	if( LoadPS2DevMngrIRX( bIsOnCD, PSDM_ENT_SMAP_IRX, 0, NULL ) != 0 )
    {
        DOERROR( NSDM_ERROR_LOAD_MODULE );
    }
    else
    {
        // Prepare the power off handler which is required once you activate the PS2 Network adapter.
        PreparePS2PowerOff( );
    }

	// Setup EENet Stack
    A_DDetail( "NSDM_Initialize()--> Setup EENet Stack\n" );
	ASSERT( !g_bLoadEENetStack );
	if( LoadEENetStack() )
	{
        A_DError( "Error in LoadEENetStack()\n" );
        //EXIT_FAILURE
        ASSERT( 0 );
        DOERROR ( NSDM_TYPE_ERROR );
	}
	g_bLoadEENetStack = TRUE;

	// Setup eenetctl lib
    A_DDetail( "NSDM_Initialize()--> Setup eenetctl lib\n" );
	ASSERT( !g_bLoadEENetCtl );
	if( LoadEENetCtl( ) )
	{
        A_DError( "Error in LoadEENetCtl()\n" );
        //EXIT_FAILURE
        ASSERT( 0 );
        DOERROR ( NSDM_TYPE_ERROR );
	}
	g_bLoadEENetCtl = TRUE;
	
    // Load up the EZ_NET_CNF and EZ_ENET_CTL IRX files up to the IOP.
    //-------------------------------------------------------------------------------------------------------------------------------------
    if( LoadPS2DevMngrIRX( bIsOnCD, PSDM_EZNETCNF_IRX, 0, NULL ) != 0 )
        DOERROR( NSDM_ERROR_LOAD_MODULE );

	if( LoadPS2DevMngrIRX( bIsOnCD, PSDM_EZENTCTL_IRX, 0, NULL ) != 0 )
        DOERROR( NSDM_ERROR_LOAD_MODULE );

    //-------------------------------------------------------------------------------------------------------------------------------------
    A_DDetail( "NSDM_Initialize()--> Initializing EzNetCnf and EzNetCTL\n" );
    if( ezNetCnfInit() )
    {
        ASSERT( 0 );
        DOERROR ( NSDM_ERROR_EZNET );
    }

	if( ezNetCtlInit() )
    {
        ASSERT( 0 );
        DOERROR ( NSDM_ERROR_EZNET );
    }

    // Identify that all IRX files are loaded.
    g_DevMgr.m_bIrxLoaded = TRUE ;

    // Get the available devices names.
    A_DDetail( "NSDM_Initialize()--> Gettin available network configuration names.\n" );
	return LoadPS2CombinationLists();

ERROR_LABEL:
	A_DDetail( "NSDM_Initialize()--> Error\n" );
	NSDM_DeInitialize();

	return error;
}




/*========================================================================
 * Function : PS2NSDM_GetDeviceDatabase()
 * Date     : 7/29/2002
 * Added by : Daniel Melfi
 */
const XCHAR * PS2NSDM_GetDeviceDatabase( void )
{
    return g_DevMgr.m_szDeviceDatabaseLocation;
}


/*========================================================================
 * Function : PS2NSDM_SetDeviceDatabase()
 * Date     : 7/29/2002
 * Added by : Daniel Melfi
 */
u32 PS2NSDM_SetDeviceDatabase( const XCHAR *pszNewDeviceDatabase )
{
    // make sure user has passed in valid parameters
    if( NULL == pszNewDeviceDatabase || 0 == x_strlen(pszNewDeviceDatabase) || (s32)sizeof(g_DevMgr.m_szDeviceDatabaseLocation) < x_strlen(pszNewDeviceDatabase) )
    {
        //EXIT_FAILURE { an invalid parameter has been passed }
        ASSERT( 0 );
        return ( NSDM_TYPE_ERROR );
    }

    // make sure there isnt a connected/active device
    if( TRUE == g_DevMgr.m_bIrxLoaded )
    {
		/*
        if( g_DevMgr.pNetCtlStatus && 0 >= ezNetCtlGetStatus( g_DevMgr.pNetCtlStatus, 0, 0 )
           && ( eStateStarted  == g_DevMgr.pNetCtlStatus->state
           || eStateRetrying == g_DevMgr.pNetCtlStatus->state
           || eStateStarting == g_DevMgr.pNetCtlStatus->state ) )
        {
            //EXIT_FAILURE { devices must be inactive }
            ASSERT( 0 );
            return ( NSDM_TYPE_ERROR );
        }
		*/
        // copy the new device database location
        x_strncpy( g_DevMgr.m_szDeviceDatabaseLocation, pszNewDeviceDatabase , sizeof(g_DevMgr.m_szDeviceDatabaseLocation) );

        // reload device list and return the results to caller
        return 0;//LoadPS2CombinationLists( );
    }
    else
    {
        // copy the new device database location
        x_strncpy( g_DevMgr.m_szDeviceDatabaseLocation, pszNewDeviceDatabase , sizeof(g_DevMgr.m_szDeviceDatabaseLocation) );

        //EXIT_SUCCESS; nothing else todo until app calls NSDM_Initialize()
        return ( 0 );
    }
}



/*========================================================================
 * Function : PS2_ConfigExists()
 * Date     : 7/26/2002
 * Desc     : returns true if the configuration exists, false otherwise
 */
static xbool PS2_ConfigExists( int nDeviceIndex )
{
    // Check for an invalid configuration index;
    if( nDeviceIndex < 0 || nDeviceIndex >= g_DevMgr.pCombinationList->listLength )
        return FALSE ;

	ezNetCnfCombination_t *pCombo = NULL;
	if( g_DevMgr.pCombinationList->netdbOrder[nDeviceIndex-1] > 0 )
		pCombo = ezNetCnfGetCombination( g_DevMgr.pCombinationList, g_DevMgr.pCombinationList->netdbOrder[nDeviceIndex-1] );

	if( pCombo )//&& pCombo->status >= 0 )
    {
        return TRUE ;
    }
    return FALSE ;
}



/*========================================================================
 * Function : NSDM_GetFirstConfiguration()
 * Date     : 7/26/2002
 */
NSDM_ITERATOR NSDM_GetFirstConfiguration( )
{
    if( g_DevMgr.m_dwTotalDevices <= 0 )
        return (NSDM_ITERATOR)0 ;

    if( PS2_ConfigExists( 1 ) )
        return (NSDM_ITERATOR)1 ;
    else
        return (NSDM_ITERATOR)0 ;
}



/*========================================================================
 * Function : NSDM_GetNextConfiguration()
 * Date     : 7/26/2002
 */
NSDM_ITERATOR NSDM_GetNextConfiguration( NSDM_ITERATOR it )
{
	int nDeviceIndex = 0;
	for( nDeviceIndex = (int)it ; nDeviceIndex < (int)g_DevMgr.pCombinationList->listLength; nDeviceIndex++ )
	{
	    if( PS2_ConfigExists( nDeviceIndex+1 ) )
		{
			//EXIT_SUCCESS
		    return (NSDM_ITERATOR)(nDeviceIndex+1) ;
		}
	}

	//EXIT_FAILURE
	return (NSDM_ITERATOR)0 ;
}



/*========================================================================
 * Function : NSDM_GetTotalConfigurations()
 * Date     : 7/26/2002
 */
u32 NSDM_GetTotalConfigurations( )
{
	s32 NumDevices = 0;

	NSDM_ITERATOR itDevice;
    for( itDevice = NSDM_GetFirstConfiguration( ) ; itDevice != NULL ; itDevice = NSDM_GetNextConfiguration( itDevice ) ) 
    {
		s32 DeviceType = NSDM_GetDeviceType( NSDM_GetConfiguration( itDevice ) );
		if( DeviceType == NSDM_TYPE_PPP || DeviceType == NSDM_TYPE_ERROR )
			continue;

		NumDevices++;
	}

	g_DevMgr.m_dwTotalDevices = NumDevices;
    return g_DevMgr.m_dwTotalDevices ;
}



/*========================================================================
 * Function : NSDM_GetConfiguration()
 * Date     : 7/26/2002
 */
NSDM_HCONFIGURATION NSDM_GetConfiguration( NSDM_ITERATOR it )
{
    int nDeviceIndex = (int) it ;

    ASSERT( 0 < nDeviceIndex );
    ASSERT( nDeviceIndex < g_DevMgr.pCombinationList->listLength );

    ezNetCnfCombination_t *pCombo = ezNetCnfGetCombination( g_DevMgr.pCombinationList, g_DevMgr.pCombinationList->netdbOrder[nDeviceIndex-1] );

	if( pCombo )//&& pCombo->status >= 0 )
        return pCombo ;

    return NULL ;
}



/*========================================================================
 * Function : NSDM_GetDefaultConfiguration()
 * Date     : 7/26/2002
 */
NSDM_HCONFIGURATION NSDM_GetDefaultConfiguration( )
{
    ezNetCnfCombination_t *pCombo = ezNetCnfGetDefault( g_DevMgr.pCombinationList ) ;

    if( NULL == pCombo || FALSE == pCombo->status >= 0 )
        //EXIT_FAILURE
        return (NSDM_HCONFIGURATION)(0);

    //EXIT_SUCCESS
    return (NSDM_HCONFIGURATION)pCombo;
}


/*========================================================================
 * Function : GetCombinationIndex()
 * Date     : 7/26/2002
 * Added by : Daniel Melfi
 */
static s32 GetCombinationIndex( const XCHAR *pszDisplayName )
{
    for( s32 dwTmpIndex = 0; dwTmpIndex < g_DevMgr.pCombinationList->listLength; ++dwTmpIndex )
    {
        if( 0 == x_strncmp( g_DevMgr.pCombinationList->list[dwTmpIndex].ifcName, pszDisplayName, 32 ) )
        {
            //EXIT_SUCCESS
            return dwTmpIndex+1;
        }
    }

    //EXIT_FAILURE
    return ( -1 );
}



/*========================================================================
 * Function : NSDM_GetConfigurationByName()
 * Date     : 7/26/2002
 * Added by : Daniel Melfi
 */
NSDM_HCONFIGURATION NSDM_GetConfigurationByName( const XCHAR *pszConfigurationName )
{
    if( NULL == pszConfigurationName )
    {
        //EXIT_FAILURE
        return ( NULL );
    }

    s32 dwTmpIndex = GetCombinationIndex( pszConfigurationName );
    if( 0 <= dwTmpIndex )
    {
        //EXIT_SUCCESS
        return ezNetCnfGetCombination( g_DevMgr.pCombinationList, dwTmpIndex );
    }

    //EXIT_FAILURE
    return ( NULL );
}



/*========================================================================
 * Function : NSDM_SetDefaultConfiguration()
 * Date     : 7/26/2002
 */
u32 NSDM_SetDefaultConfiguration( NSDM_HCONFIGURATION hConfig )
{
    if( NULL == hConfig )
    {
        //EXIT_FAILURE
        ASSERT( 0 );
        return ( NSDM_TYPE_ERROR );
    }

    // is the input configuration the same as the current active configuration
    if( reinterpret_cast<ezNetCnfCombination_t*>(hConfig) == ezNetCnfGetDefault( g_DevMgr.pCombinationList ) )
    {
        //EXIT_SUCCESS
        return ( 0 );
    }


    // set the new index
    /*
	if( 0 == ezNetCnfSetDefault( g_DevMgr.m_szDeviceDatabaseLocation, reinterpret_cast<ezNetCnfCombination_t*>(hConfig)->combinationName ) )
    {
        // We need to reload the lists in order for future configuration iterations to be correct
        return LoadPS2CombinationLists( );
    }
	*/

    //Unable to reload the list, i guess we should continue with the old one
    //EXIT_FAILURE
    ASSERT( 0 );
    return ( NSDM_TYPE_ERROR );
}



/*========================================================================
 * Function : NSDM_GetConfigurationName()
 * Date     : 7/26/2002
 */
const XCHAR* NSDM_GetConfigurationName( NSDM_HCONFIGURATION hConfig )
{
    if( NULL == hConfig )
    {
        //EXIT_FAILURE
        return ( NULL );
    }

    ezNetCnfCombination_t *pCombo = (ezNetCnfCombination_t*)hConfig ;
    if( pCombo )//&& pCombo->status >= 0 )
        return pCombo->ifcName ;
    else
        return NULL ;
}

/*========================================================================
 * Function : NSDM_GetDeviceType()
 * Date     : 1/8/2003
 * Was here but got removed somehow
 */
NSDM_DEVICE_TYPE NSDM_GetDeviceType( NSDM_HCONFIGURATION hConfig )
{
    if( NULL == hConfig )
    {
        //EXIT_FAILURE
        ASSERT( 0 );
        return ( NSDM_TYPE_ERROR );
    }

    ezNetCnfCombination_t *pCombo = (ezNetCnfCombination_t*)hConfig ;

    ASSERT(pCombo);

	switch(pCombo->iftype)//connectionType)
	{
		case eIftypePPP		: return NSDM_TYPE_PPP;		// modem PPP
		case eIftypePPPoE	: return NSDM_TYPE_PPPOE;	// PPP over Ethernet
		case eIftypeEther	: return NSDM_TYPE_ARP;		// Ethernet (static IP and DHCP): return 
		default: 
        {
            ASSERT( 0 );
            return NSDM_TYPE_ERROR;
        }
	}
}


/*========================================================================
 * Function : NSDM_GetDeviceName()
 * Date     : 7/26/2002
 */
const XCHAR * NSDM_GetDeviceName( NSDM_HCONFIGURATION hConfig )
{
    if( NULL == hConfig )
    {
        //EXIT_FAILURE
        return ( NULL );
    }

    ezNetCnfCombination_t *pCombo = (ezNetCnfCombination_t*)hConfig ;

    if( pCombo )//&& pCombo->status >= 0 )
        return pCombo->devName ;
    else
        return NULL ;
}



/*========================================================================
 * Function : NSDM_GetDeviceStatus()
 * Date     : 7/26/2002
 */
NSDM_STATUS NSDM_GetDeviceStatus( void )
{
    s32 ErrorCode;

    // If the device has not been initialized, just leave now.
	if( !g_bInitialized  )
    {
        A_DDetail( "NSDM_GetDeviceStatus Device Not Initialized.\n" );
        return ( NSDM_STATUS_ERROR );
    }
    // If the device list hasn't been created yet, bail now!
    else if( !g_DevMgr.pNetCtlStatus )
    {
        A_DDetail( "NSDM_GetDeviceStatus Device already detached.\n" );
        return ( NSDM_STATUS_ERROR );
    }
    // Otherwise, get the latest status on the device.
    else 
    {
        ErrorCode = ezNetCtlGetStatus( g_DevMgr.pNetCtlStatus, 0, 0 );
        if( ErrorCode < 0 )
        {
            A_DDetail( "NSDM_GetDeviceStatus Failed: ezNetCtlGetStatus ErrorCode:%d\n", ErrorCode );
            ASSERT( 0 );

            //EXIT_FAILURE
            return ( NSDM_STATUS_ERROR );
        }
    }

    //EXIT_SUCCESS
    return (NSDM_STATUS)g_DevMgr.pNetCtlStatus->state;
}


#define PS2_DEV_IS_TRUE(ch)     ( (ch) && (ch) != 0xff)
#define PS2_DEV_IS_FALSE(ch)    (!(ch) || (ch) == 0xff)
#define PS2_DEV_IS_SCE(vendor)  (x_strncmp((vendor), "SCE", 3) == 0)


/*========================================================================
 * Function: LoadDeviceDependentIRXs()
 * Desc    :
 */
static u32 LoadDeviceDependentIRXs( sceNetcnfifData_t *pCurrentCombinationProperties )
{
    //For we are leaving out the PPP, AND PPPOE Calls until we can verify that EENEt doesn't require the
    //IRX files to be loaded separately.
    // Mike Skinner
    
    //int result;

    //xbool bIsOnCD = (0==x_strncmp(g_DevMgr.m_szIrxDir,"cdrom",5));

    //// --------------------------------------------------------
    ////  load optional protocol drivers
    //// --------------------------------------------------------
    //if (pCurrentCombinationProperties->ifc_type == sceNetcnfifData_type_ppp)
    //{
    //    result = LoadPS2DevMngrIRX( bIsOnCD, PSDM_PPP_IRX, 0, 0 );
    //    if( result < 0 )
    //    {
    //        A_DError( "LoadDeviceDependantIRXs--> Failed to load PSDM_PPP_IRX: %d.\n", result );
    //        return 1;
    //    }

    //    if( PS2_DEV_IS_TRUE(pCurrentCombinationProperties->pppoe) )
    //    {
    //        result = LoadPS2DevMngrIRX( bIsOnCD, PSDM_PPPOE_IRX, 0, NULL );
    //        if( result < 0 )
    //        {
    //            A_DError( "LoadDeviceDependantIRXs--> Failed to load PSDM_PPPOE_IRX: %d.\n", result );
    //            return 1;
    //        }
    //    }
    //}

	return ( 0 );
}



/*========================================================================
 * Function : NSDM_StartDevice()
 * Date     : 7/26/2002
 */
u32 NSDM_StartDevice( NSDM_HCONFIGURATION hConfig )
{
    s32 ErrorCode;

    if( NULL == hConfig )
    {
        //EXIT_FAILURE
        ASSERT( 0 );
        return ( NSDM_TYPE_ERROR );
    }

    g_DevMgr.pCurrentCombination = (ezNetCnfCombination_t*)hConfig ;

    A_DDetail( "NSDM_StartDevice--> ezNetCnfGetProperties.\n" );
	ErrorCode = ezNetCnfGetProperties( g_DevMgr.m_szDeviceDatabaseLocation, g_DevMgr.pCurrentCombination->combinationName, g_DevMgr.pNetCnfifData );
    if( ErrorCode < 0 )
    {
        A_DError( "NSDM_StartDevice--> ezNetCnfGetProperties Failed: %d.\n", ErrorCode );

        //EXIT_FAILURE
        ASSERT( 0 );
        return ( NSDM_TYPE_ERROR );
    }

	if( LoadDeviceDependentIRXs(g_DevMgr.pNetCnfifData))
	{
        //EXIT_FAILURE

        return ( NSDM_TYPE_ERROR );
    }

	if(sceNetcnfifData_type_ppp != g_DevMgr.pNetCnfifData->dev_type)
	{
        g_DevMgr.pCurrentCombination->iftype = eIftypeEther;
	}
	else
	{
		g_DevMgr.pCurrentCombination->iftype = (PS2_DEV_IS_FALSE(g_DevMgr.pNetCnfifData->pppoe)) ? eIftypePPP : eIftypePPPoE ;
	}
	
	//g_DevMgr.pNetCtlStatus->id = ezNetCtlSetProperties( g_DevMgr.pNetCnfifData );

	A_DDetail("Starting device %s\n", g_DevMgr.pCurrentCombination->ifcName );
    A_DDetail("NSDM_StartDevice--> ezNetCtlSetProperties\n" );

    ErrorCode = ezNetCtlSetProperties( g_DevMgr.pNetCnfifData );
    if( ErrorCode < 0 )
    {
        A_DError("NSDM_StartDevice--> ezNetCtlSetProperties Failed: %d.\n", ErrorCode );

        ASSERT( 0 );
        return ( NSDM_TYPE_ERROR );
    }

    A_DDetail("NSDM_StartDevice--> ezNetCtlUpInterface\n" );
    ErrorCode = ezNetCtlUpInterface();
    if( ErrorCode < 0 )
    {
        A_DError("NSDM_StartDevice--> ezNetCtlUpInterface Failed: %d\n", ErrorCode );
        //EXIT_FAILURE
        ASSERT( 0 );
        return ( NSDM_TYPE_ERROR );
    }

    g_bInterfaceUp = TRUE;

    // return what this function does
    // 0 = SUCCESS, ELSE non-zero error code
    A_DDetail("NSDM_StartDevice--> ezNetCtlGetStatus\n" );
    ErrorCode = ezNetCtlGetStatus( g_DevMgr.pNetCtlStatus, 0, 0 );
    if( ErrorCode < 0 )
    {
        A_DError("NSDM_StartDevice--> ezNetCtlGetStatus Failed: %d\n", ErrorCode );
        //EXIT_FAILURE
        ASSERT( 0 );
        return ( NSDM_TYPE_ERROR );
    }

    return 0;
}

/*========================================================================
 * Function : PS2NSDM_GetExtendedConfigurationInfo()
 * Date     : 7/26/2002
 * Added by : Daniel Melfi
 */
u32 PS2NSDM_GetExtendedConfigurationInfo( void *pPS2DataOut, const XCHAR *pszName )
{
    if( NULL == pPS2DataOut || NULL == pszName )
    {
        //EXIT_FAILURE
        ASSERT( 0 );
        return ( NSDM_TYPE_ERROR );
    }

    s32 dwTmpIndex = GetCombinationIndex( pszName );

    if( 0 < dwTmpIndex )
    {
        return ezNetCnfGetProperties( g_DevMgr.m_szDeviceDatabaseLocation,
                                      g_DevMgr.pCombinationList->list[dwTmpIndex].combinationName,
                                      reinterpret_cast<sceNetcnfifData_t*>(pPS2DataOut) );
    }

    //EXIT_FAILURE
    ASSERT( 0 );
    return ( NSDM_TYPE_ERROR );
}


/*========================================================================
 * Function : PS2NSDM_GetDEV_Count()
 * Date     : 7/26/2002
 * Added by : Daniel Melfi
 */
u32   PS2NSDM_GetDEV_Count( )
{
    return g_DevMgr.m_dwTotalDevices;//!!!ezNetCnfGetCount( g_DevMgr.m_szDeviceDatabaseLocation, eDeviceFile );
}


/*========================================================================
 * Function : PS2NSDM_GetIFC_Count()
 * Date     : 7/26/2002
 * Added by : Daniel Melfi
 */
u32   PS2NSDM_GetIFC_Count( )
{
	return 1;//!!!ezNetCnfGetCount( g_DevMgr.m_szDeviceDatabaseLocation, eInterfaceFile );
}


/*========================================================================
 * Function : NSDM_StopDevice()
 * Date     : 7/26/2002
 */
u32 NSDM_StopDevice( )
{
    A_DStatus("Stopping communications device\n");

    // Release the network interface.
	s32 rcode = ezNetCtlDownInterface();
	if( rcode < 0 )
	{		
        A_DDetail("NSDM_StopDevice: ezNetCtlDownInterface failed, error code: %d\n", rcode);
        ASSERT( 0 );
        return ( NSDM_TYPE_ERROR );
	}
	else
	{
        A_DDetail( "NSDM_StopDevice( )--> Waiting for Device Down Complete.\n" );
        while( 1 )
        {
            rcode = ezNetCtlWaitEventTO( EZ_NET_Wait_Time );
            if( rcode == sceEENETCTL_IEV_DownCompleted )
            {
                A_DDetail( "NSDM_StopDevice( )--> Device Down Complete.\n" );
                break;
            }
            else if( rcode == -1 )
            {
                A_DDetail( "NSDM_StopDevice( )--> Device Down Failed.\n" );
                ASSERT( 0 );
            }
        }
	}

    return 0;
}


static XCHAR g_statusStrings[][16] =
{
"Detatched", "Starting", "Retrying", "Started", "Stopping", "Stopped", "Error", "Last"
} ;

/*========================================================================
 * Function : NSDM_GetStatusString()
 * Date     : 7/26/2002
 */
const XCHAR* NSDM_GetStatusString( NSDM_STATUS status )
{
    return g_statusStrings[status] ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Function: NSDM_IsConnected()
// Desc: returns true if all non-loopback interfaces are up and have valid media, false otherwise
// Parameters: none
// Author: Bill Williams
// Date: 1-15-04
////////////////////////////////////////////////////////////////////////////////////////////////////

xbool NSDM_IsConnected()
{
	sceEENetIfname* names;
	s32 arraysize;
	sceEENetGetIfnames(NULL, &arraysize);
	names = (sceEENetIfname*)(x_malloc(sizeof(sceEENetIfname) * arraysize));
	sceEENetGetIfnames(names, &arraysize);
	s16 mediaInfo;
	s32 mediaSize = sizeof(mediaInfo);
	bool allIFsup = true;
	for(s32 i = 0; i < arraysize; ++i)
	{
		// loopback is funky and we don't care.
		if(x_strncmp(names[i].ifn_name, "lo", 2) == 0) continue;
		s32 errCode = sceEENetGetIfinfo(names[i].ifn_name, sceEENET_IFINFO_MEDIA, &mediaInfo, &mediaSize);
		ASSERT(errCode == (s32)(0));
		allIFsup = allIFsup && ((mediaInfo & sceEENET_MEDIA_AVALID) && (mediaInfo & sceEENET_MEDIA_ACTIVE));
	}
	x_free(names);
	return allIFsup;
}



/*========================================================================
 * Function : NSDM_DeInitialize()
 * Date     : 7/26/2002
 */
u32 NSDM_DeInitialize( )
{
	s32 ErrorCode;
    s32 DeviceStatus;

    A_DDetail( "NSDM_DeInitialize( )--> Shutting down.\n" );

	if( FALSE == g_bInitialized )
    {
        //EXIT_FAILURE
        ASSERT( 0 );
        return ( NSDM_TYPE_ERROR );
    }

    g_bInitialized = FALSE;

    sceEENetFreeThreadinfo(GetThreadId());

    // Stop the Network adapter, and disconnect it from the network.
    //-------------------------------------------------------------------------------------------------------------------------------------
    DeviceStatus = NSDM_GetDeviceStatus( );
    if( DeviceStatus == NSDM_STATUS_STARTING || DeviceStatus == NSDM_STATUS_RETRYING || DeviceStatus == NSDM_STATUS_STARTED )
    {
        A_DDetail( "NSDM_DeInitialize( )--> Stopping the Network adapter.\n" );
        NSDM_StopDevice( );
    }

	// Unload the network adapter device driver.
    //-------------------------------------------------------------------------------------------------------------------------------------
	if (g_bLoadEENetCtl)
	{
		A_DDetail( "NSDM_DeInitialize( )--> Unload the network adapter device driver.\n" );
		ErrorCode = sceEENetDeviceSMAPUnreg();
		if(ErrorCode < 0)
		{
			ASSERT( 0 );
			A_DError("NSDM_DeInitialize( )--> sceEENetDeviceSMAPUnreg failed (%d)\n", ErrorCode);
			return ( NSDM_TYPE_ERROR );
		}
		else
		{
			// wait for the detach to complete
			WaitEvent( Ev_DetachCompleted );
		}

		// UnRegister the Event Handler.
		//-------------------------------------------------------------------------------------------------------------------------------------
		A_DDetail( "NSDM_DeInitialize( )--> Un Registering the event_handler callback function.\n" );
		ErrorCode = sceEENetCtlUnregisterEventHandler(event_handler);
		if(ErrorCode < 0)
		{
			A_DError("NSDM_DeInitialize( )--> sceEENetCtlUnRegisterEventHandler failed (%d)\n", ErrorCode);
			ASSERT( 0 );
			return ( NSDM_TYPE_ERROR );
		}
		// Unload ezNetCtl and ezNetCnf
		A_DDetail( "NSDM_DeInitialize( )--> Shutting down and ezNetCtrl.\n" );
		if( (ErrorCode = ezNetCtlExit( 0 )) < 0 )
		{
			A_DError( "NSDM_DeInitialize( )--> Error returned from ezNetCtlExit. Error code: %d\n", ErrorCode );
			ASSERT( 0 );
			return (NSDM_TYPE_ERROR);            
		}	

		A_DDetail( "NSDM_DeInitialize( )--> Shutting down and ezNetCfg.\n" );
		if( (ErrorCode = ezNetCnfExit()) < 0 )
		{
			A_DError( "NSDM_DeInitialize( )--> Error returned from ezNetCnfExit. Error code: %d\n", ErrorCode );
			ASSERT( 0 );
			return (NSDM_TYPE_ERROR);            
		}
	}

    // IOP module "PSDM_EZNETCNF_IRX" is removed from the IOP when the call to the EE function ezNetCnfExit is
    // executed. Make sure to identify that this IOP module is no longer loaded.
    UnloadPS2DevMngrIRX( PSDM_EZNETCNF_IRX );

    // IOP module "PSDM_EZENTCTL_IRX" is removed from the IOP when the call to the EE function ezNetCtlExit is
    // executed. Make sure to identify that this IOP module is no longer loaded.
    UnloadPS2DevMngrIRX( PSDM_EZENTCTL_IRX );


	// Perform termination processing for eenetctl.a
    //-------------------------------------------------------------------------------------------------------------------------------------
    if (g_bLoadEENetCtl)
	{
		A_DDetail( "NSDM_DeInitialize( )--> termination processing for eenetctl.a.\n" );
		ErrorCode = sceEENetCtlTerm();
		if( ErrorCode < 0 )
		{
			A_DError("NSDM_DeInitialize( )--> sceEENetCtlTerm failed (%d)\n", ErrorCode);
			ASSERT( 0 );
			return (NSDM_TYPE_ERROR);            
		}
	}

	// Shutdown the EENet Libs
    //-------------------------------------------------------------------------------------------------------------------------------------
	if (g_bLoadEENetStack)
	{
		A_DDetail( "NSDM_DeInitialize( )--> Shutdown the EENet Libs\n" );
		KillEENetStack( );
	}

    // Unload the IOP modules that are no longer in use.
    //-------------------------------------------------------------------------------------------------------------------------------------
    A_DDetail( "NSDM_DeInitialize( )--> Shutting down and unloading IOP modules.\n" );
	UnloadPS2DevMngrIRX( PSDM_ENT_SMAP_IRX );
	UnloadPS2DevMngrIRX( PSDM_EENETCTL_IRX );
    UnloadPS2DevMngrIRX( PSDM_ENT_DEVM_IRX );

	UnloadPS2DevMngrIRX( PSDM_NETCNF_IRX );  // The SONY docs to not specify whether this IRX file should be removed from the IOP.

    // DEV9.irx cannot be unloaded once it's activated.
    // UnloadPS2DevMngrIRX( PSDM_DEV9_IRX );
    
    // Mike Skinner
    // Please put these back if EENET requires us to load PPP and PPPOE irx's manually.
    // UnloadPS2DevMngrIRX( PSDM_PPP_IRX );
    // UnloadPS2DevMngrIRX( PSDM_PPPOE_IRX );

	// Delete the semaphore
	if (sSema_ID >= 0)
	{
		if( DeleteSema( sSema_ID ) == -1 )
		{
			ASSERT( 0 );
			return ( NSDM_TYPE_ERROR );
		}
		sSema_ID = -1;
	}

	// Free netcnf related memory
    //-------------------------------------------------------------------------------------------------------------------------------------
    A_DDetail( "NSDM_DeInitialize( )--> Releasing memory from network configurations.\n" );
	X_SAFE_FREE( g_DevMgr.pNetCnfifData_UA );
    X_SAFE_FREE( g_DevMgr.pNetCtlStatus_UA );
    X_SAFE_FREE( g_DevMgr.pCombinationList_UA );

    g_DevMgr.pCombinationList	= NULL;
    g_DevMgr.pNetCtlStatus		= NULL;
    g_DevMgr.pNetCnfifData		= NULL;

	g_bLoadEENetStack = FALSE;
	g_bLoadEENetCtl = FALSE;
    ASSERT( !x_MemSanityCheck() );
	return 0 ;
}

/*========================================================================
 * Function : PS2NSDM_UnloadNetcnfIRX()
 * Date     : 12/4/2002
 * Added by : Daniel Melfi
 * Comments : free's 14K on IOP for game
 */
u32 PS2NSDM_UnloadNetcnfIRX( )
{
    ASSERT(!"Not tested");

	ezNetCnfExit();

    x_free( g_DevMgr.pNetCnfifData_UA ) ;    
    x_free( g_DevMgr.pCombinationList_UA ) ;

    g_DevMgr.pCombinationList_UA = NULL;
    g_DevMgr.pCombinationList = NULL;

    g_DevMgr.pNetCnfifData_UA = NULL;
    g_DevMgr.pNetCnfifData = NULL;

	int tmp = -1, result = 0;
	int modID = sceSifSearchModuleByName("netcnf");
	if(modID>=0)tmp = sceSifStopModule(modID, 0, 0, &result);
	if(tmp>=0) sceSifUnloadModule(modID);		

    return 0;
}

//=====================================================================================================================================
// PS2 Power off thread required once you start up the Net Device.
//=====================================================================================================================================
xbool   bPS2PowerOffThreadCreated = FALSE;
#define PS2_PowerOffThreadStackSize (512 * 16)

static byte sPS2PowerOffStack[PS2_PowerOffThreadStackSize] __attribute__ ((aligned(16)));

//-------------------------------------------------------------------------------------------------------------------------------------
void PowerOffThread(void *arg)
{
	s32 sid = (s32)arg;
	s32 stat;
	while(1)
    {
		WaitSema(sid);
		x_printf("Power Off Requested.\n");
		// dev9 power off, need to power off PS2
		while(sceDevctl("dev9x:", DDIOC_OFF, NULL, 0, NULL, 0) < 0);
		// PS2 power off
		while(!sceCdPowerOff(&stat) || stat);
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
void PowerOffHandler(void *arg)
{
    s32 sid = (s32)arg;
    iSignalSema(sid);
}

//-------------------------------------------------------------------------------------------------------------------------------------
void PreparePS2PowerOff( void )
{
    struct ThreadParam PS2_PowerOff_tparam;
    struct SemaParam   PS2_PowerOff_sparam;
    int tid;
    int sid;

    // Allow the power off thread to be created only one time.
    if( !bPS2PowerOffThreadCreated )
    {
        // Don't let it happen more than one time.
        bPS2PowerOffThreadCreated = TRUE;

        PS2_PowerOff_sparam.initCount = 0;
        PS2_PowerOff_sparam.maxCount  = 1;
        PS2_PowerOff_sparam.option    = 0;
        sid = CreateSema(&PS2_PowerOff_sparam);

        // Change the main game thread to 2, so the power off thread can be the most important below.
        ChangeThreadPriority(GetThreadId(), 2);
        PS2_PowerOff_tparam.stackSize = PS2_PowerOffThreadStackSize;
        PS2_PowerOff_tparam.gpReg = &_gp;
        PS2_PowerOff_tparam.entry = PowerOffThread;
        PS2_PowerOff_tparam.stack = (void *)sPS2PowerOffStack;
        PS2_PowerOff_tparam.initPriority = 1;
        tid = CreateThread(&PS2_PowerOff_tparam);
        StartThread(tid, (void *)sid);

        sceCdPOffCallback(PowerOffHandler, (void *)sid);
    }
}