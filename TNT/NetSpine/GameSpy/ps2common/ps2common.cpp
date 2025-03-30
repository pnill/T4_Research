#include <eekernel.h>
#include <stdio.h>
#include <string.h>
#include <sifrpc.h>
#include <sifdev.h>
#include <libdev.h>
#include <libgraph.h>
#include "../nonport.h"

// One of the following network devices must be defined
#if 0
#define T10K_ETHERNET
#endif
#if 0
#define USB_ETHERNET
#endif
#if 0
#define USB_ETHERNET_WITH_PPPOE
#endif
#if 1
#define HDD_ETHERNET
#endif
#if 0
#define HDD_ETHERNET_WITH_PPPOE
#endif
#if 0
#define	MODEM
#endif

#ifdef EENET

//#include "/usr/local/sce/ee/sample/libeenet/ent_cnf/ent_cnf.h"

#if defined(USB_ETHERNET)
#define USR_CONF_NAME     "Combination4"
#elif defined(USB_ETHERNET_WITH_PPPOE)
#define USR_CONF_NAME     "Combination5"
#elif defined(HDD_ETHERNET)
#define USR_CONF_NAME     "Combination6"
#elif defined(HDD_ETHERNET_WITH_PPPOE)
#define USR_CONF_NAME     "Combination7"
#elif defined(MODEM)
#define USR_CONF_NAME     ""
#endif

#define SCEROOT           "host0:"
#define MODROOT           SCEROOT "system/netirx/"
#define APPROOT           SCEROOT "cnf/"

#define	MOD_NETCNF        MODROOT "netcnf.irx"
#define MOD_EENETCTL      MODROOT "eenetctl.irx"
#define MOD_DEV9          MODROOT "dev9.irx"
#define MOD_USBD          MODROOT "usbd.irx"
#define MOD_ENT_DEVM      MODROOT "ent_devm.irx"
#define MOD_ENT_SMAP      MODROOT "ent_smap.irx"
#define MOD_ENT_ETH       MODROOT "ent_eth.irx"
#define MOD_ENT_PPP       MODROOT "ent_ppp.irx"
//#define	MOD_ENT_CNF       SCEROOT "modules/ent_cnf.irx"
#define MOD_MODEMDRV      ""

#define	INETCTL_ARG       "-no_auto" "\0" "-no_decode"
#define	NETCNF_ICON       APPROOT "SYS_NET.ICO"
#define	NETCNF_ICONSYS    APPROOT "icon.sys"
#define	NETCNF_ARG        "icon=" NETCNF_ICON "\0" "iconsys=" NETCNF_ICONSYS
#define MODEMDRV_ARG      ""

#define NET_DB            SCEROOT "conf/net/net.db"

#define EENET_MEMSIZE     (512 * 1024)
#define EENET_TPL         32
#define EENET_APP_PRIO    48

static int eenetctl_mid;
//static int ent_cnf_mid;
static int ent_devm_mid;
#if defined(HDD_ETHERNET) || defined(HDD_ETHERNET_WITH_PPPOE)
#define EENET_IFNAME "smap0"
static int ent_smap_mid;
#endif
#if defined(USB_ETHERNET) || defined(USB_ETHERNET_WITH_PPPOE)
#define EENET_IFNAME "eth0"
static int ent_eth_mid;
#endif
#if defined(MODEM)
#define EENET_IFNAME "ppp0"
static int ent_ppp_mid;
static int modem_mid;
#endif

#ifndef EENET_260
static void * eenet_pool;
#endif

static int sema_id;
static int event_flag = 0;
#define Ev_Attach          0x01
#define Ev_UpCompleted     0x02
#define Ev_DownCompleted   0x04
#define Ev_DetachCompleted 0x08

#define WaitEvent(event) \
	while(1){ \
		WaitSema(sema_id); \
		if(event_flag & (event)) \
			break; \
	}

#if defined( X_DEBUG )
	#define DBG_PRINT( exp )	exp
#else
	#define DBG_PRINT( exp )
#endif


static void event_handler(const char *ifname, int af, int type)
{
	DBG_PRINT(printf("event_handler: event happened. af = %d, type = %d\n", af, type));

	switch(type)
	{
	case sceEENETCTL_IEV_Attach:
		if(strcmp(ifname, EENET_IFNAME))
			break;
		event_flag |= Ev_Attach;
		SignalSema(sema_id);
		break;
	case sceEENETCTL_IEV_UpCompleted:
		event_flag |= Ev_UpCompleted;
		SignalSema(sema_id);
		break;
	case sceEENETCTL_IEV_DownCompleted:
		event_flag |= Ev_DownCompleted;
		SignalSema(sema_id);
		break;
	case sceEENETCTL_IEV_DetachCompleted:
		if(strcmp(ifname, EENET_IFNAME))
			break;
		event_flag |= Ev_DetachCompleted;
		SignalSema(sema_id);
		break;
	}

	return;
}

static int create_sema(int init_count, int max_count){
	struct SemaParam sema_param;
	int sema_id;

	memset(&sema_param, 0, sizeof(struct SemaParam));
	sema_param.initCount = init_count;
	sema_param.maxCount = max_count;
	sema_id = CreateSema(&sema_param);

	return sema_id;
}

int load_network_modules()
{
	while((ent_devm_mid = sceSifLoadModule(MOD_ENT_DEVM, 0, NULL)) < 0)
		scePrintf("ent_devm.irx cannot be loaded.\n");
	while(sceSifLoadModule(MOD_NETCNF, sizeof(NETCNF_ARG), NETCNF_ARG) < 0)
		scePrintf("netcnf.irx cannot be loaded.\n");
	//while((ent_cnf_mid = sceSifLoadModule(MOD_ENT_CNF, 0, NULL)) < 0)
	//	scePrintf("ent_cnf.irx cannot be loaded.\n");
#if defined( HDD_ETHERNET ) || defined( HDD_ETHERNET_WITH_PPPOE )
	while(sceSifLoadModule(MOD_DEV9, 0, NULL) < 0)
		scePrintf("dev9.irx cannot be loaded.\n");
	while((ent_smap_mid = sceSifLoadModule(MOD_ENT_SMAP, 0, NULL)) < 0)
		scePrintf("ent_smap.irx cannot be loaded.\n");
	//PreparePowerOff();
#endif
#if defined( USB_ETHERNET ) || defined( USB_ETHERNET_WITH_PPPOE )
	while(sceSifLoadModule(MOD_USBD, 0, NULL) < 0)
		scePrintf("usbd.irx cannot be loaded.\n");
	while((ent_eth_mid = sceSifLoadModule(MOD_ENT_ETH, 0, NULL)) < 0)
		scePrintf("ent_eth.irx cannot be loaded.\n");
#endif
#if defined( MODEM )
	while((ent_ppp_mid = sceSifLoadModule(MOD_ENT_PPP, 0, NULL)) < 0)
		scePrintf("ent_ppp.irx cannot be loaded.\n");
	while((modem_mid = sceSifLoadModule(MOD_MODEMDRV, sizeof(MODEMDRV_ARG), MODEMDRV_ARG)) < 0)
		scePrintf("%s cannot be loaded.\n", MOD_MODEMDRV);
#endif

	return 0;
}

int do_initialisation(void)
{
	int rcode;

	// create a semaphore
	sema_id = create_sema(0, 1);
	if(sema_id < 0)
	{
		DBG_PRINT(printf("create_sema() failed.\n"));
		return -1;
	}

	// load network modules	
	rcode = load_network_modules();
	if(rcode < 0)
		return rcode;	

#ifndef EENET_260
	// allocate memory for EENet
	eenet_pool = memalign(64, EENET_MEMSIZE);
	if(eenet_pool == NULL)
		return -1;

	// initialize eenet
	rcode = sceEENetInit(eenet_pool, EENET_MEMSIZE, EENET_TPL, 8192, EENET_APP_PRIO);
#else
	rcode = sceEENetInit(EENET_MEMSIZE, EENET_TPL, 0, 8192, EENET_APP_PRIO);
#endif
	if(rcode < 0)
	{
		DBG_PRINT(printf("sceEENetInit failed (%d)\n", rcode));
		return -1;
	}

#ifdef _DEBUG
	sceEENetSetLogLevel(EENET_LOG_DEBUG);
#endif

	// init eenetctl.a
	rcode = sceEENetCtlInit(8192, 32, 8192, 32, 8192, 32, 1, 0);
	if(rcode < 0)
	{
		DBG_PRINT(printf("sceEENetCtlInit failed (%d)\n", rcode));
		return -1;
	}

	while((eenetctl_mid = sceSifLoadModule(MOD_EENETCTL, 0, NULL)) < 0)
		scePrintf("eenetctl.irx cannot be loaded.\n");

	// add event handler
	rcode = sceEENetCtlRegisterEventHandler(event_handler);
	if(rcode < 0)
	{
		DBG_PRINT(printf("sceEENetCtlRegisterEventHandler failed (%d)\n", rcode));
		return -1;
	}

	// register network interface driver
#if defined( HDD_ETHERNET ) || defined( HDD_ETHERNET_WITH_PPPOE )
	rcode = sceEENetDeviceSMAPReg(8192, 8192);
	if(rcode < 0)
	{
		DBG_PRINT(printf("sceEENetDeviceSMAPReg failed (%d)\n", rcode));
		return -1;
	}
#endif
#if defined( USB_ETHERNET ) || defined( USB_ETHERNET_WITH_PPPOE )
	rcode = sceEENetDeviceETHReg(8192, 8192);
	if(rcode < 0)
	{
		DBG_PRINT(printf("sceEENetDeviceETHReg failed (%d)\n", rcode));
		return -1;
	}
#endif
#if defined( MODEM )
	rcode = sceEENetDevicePPPReg(8192, 8192);
	if(rcode < 0)
	{
		DBG_PRINT(printf("sceEENetDevicePPPReg failed (%d)\n", rcode));
		return -1;
	}
#endif

	// wait until target interface is attached
	WaitEvent(Ev_Attach);

	// wait for interface initialization to complete
	//WaitEvent(Ev_UpCompleted);

	return 0;
}

void do_shutdown(void)
{
	int rcode;
	
	// bring down the interface
	rcode = sceEENetCtlDownInterface(EENET_IFNAME);
	if(rcode < 0)
	{
		DBG_PRINT(printf("sceEENetCtlDownInterface failed (%d)\n", rcode));
		return;
	}

	// wait for the termination to complete
	WaitEvent(Ev_DownCompleted);

	// unload the driver module
#if defined( HDD_ETHERNET ) || defined( HDD_ETHERNET_WITH_PPPOE )
	rcode = sceEENetDeviceSMAPUnreg();
	if(rcode < 0)
	{
		DBG_PRINT(printf("sceEENetDeviceSMAPUnreg failed (%d)\n", rcode));
		return;
	}
#endif
#if defined( USB_ETHERNET ) || defined( USB_ETHERNET_WITH_PPPOE )
	rcode = sceEENetDeviceETHUnreg();
	if(rcode < 0)
	{
		DBG_PRINT(printf("sceEENetDeviceETHUnreg failed (%d)\n", rcode));
		return;
	}
#endif
#if defined( MODEM )
	rcode = sceEENetDevicePPPUnreg();
	if(rcode < 0)
	{
		DBG_PRINT(printf("sceEENetDevicePPPUnreg failed (%d)\n", rcode));
		return;
	}
#endif

	// wait for the detach to complete
	WaitEvent(Ev_DetachCompleted);

	// cancel the event handler
	rcode = sceEENetCtlUnregisterEventHandler(event_handler);
	if(rcode < 0)
	{
		DBG_PRINT(printf("sceEENetCtlUnRegisterEventHandler failed (%d)\n", rcode));
		return;
	}

	// eenetctl.a termination processing
	rcode = sceEENetCtlTerm();
	if(rcode < 0)
	{
		DBG_PRINT(printf("sceEENetCtlTerm failed (%d)\n", rcode));
		return;
	}

	// terminate eenet
	rcode = sceEENetTerm();
	if(rcode < 0)
	{
		DBG_PRINT(printf("sceEENetTerm failed (%d)\n", rcode));
		return;
	}

	// stop and unload modules
#if defined( HDD_ETHERNET ) || defined( HDD_ETHERNET_WITH_PPPOE )
	sceSifStopModule(ent_smap_mid, 0, NULL, &rcode);
	sceSifUnloadModule(ent_smap_mid);
#endif
#if defined( USB_ETHERNET ) || defined( USB_ETHERNET_WITH_PPPOE )
	sceSifStopModule(ent_eth_mid, 0, NULL, &rcode);
	sceSifUnloadModule(ent_eth_mid);
#endif
#if defined( MODEM )
	sceSifStopModule(modem_mid, 0, NULL, &rcode);
	sceSifUnloadModule(modem_mid);

	sceSifStopModule(ent_ppp_mid, 0, NULL, &rcode);
	sceSifUnloadModule(ent_ppp_mid);
#endif

	sceSifStopModule(eenetctl_mid, 0, NULL, &rcode);
	sceSifUnloadModule(eenetctl_mid);

	sceSifStopModule(ent_devm_mid, 0, NULL, &rcode);
	sceSifUnloadModule(ent_devm_mid);

#ifndef EENET_260
	// free the EENet pool
	if(eenet_pool != NULL)
		free(eenet_pool);
#endif
}

#endif

//extern int test_main(int argc, char ** argp);
/*
int main(int argc, char ** argp)
{
    int result;

    DBG_PRINT(printf("\nGameSpy Test App Initializing\n") 
	       "----------------------------------\n");

#if defined(UNIQUEID) && defined(CISCO_NFT)
	// this should be called before all other PS2 initialization
	// because it resets the PS2 the first time it is called
	GOAGetUniqueID();
#endif

	// init RPC
    sceSifInitRpc(0);

    // initialize the stack
	result = (int)do_initialisation();
    if(result)
    {
        DBG_PRINT(printf("Initialization failed\n"));
        return result;
    }

	// start the actual program
    DBG_PRINT(printf("\nGameSpy Test App Starting\n") 
	       "----------------------------------\n");
	//test_main(argc, argp);

	// do any needed cleanup
	do_shutdown();
    DBG_PRINT(printf("\nGameSpy Test App Exiting\n") 
	       "----------------------------------\n");

    return 0;
}
*/