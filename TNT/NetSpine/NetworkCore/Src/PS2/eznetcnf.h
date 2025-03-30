/*
[SCE CONFIDENTIAL]
$PSDocId$
                  Copyright (C) 2002-3 Sony Computer Entertainment America
                                                     All Rights Reserved
//  ------------------------------------------------------------------------

File: eznetcnf.h

//  ------------------------------------------------------------------------
*/

#ifndef _EZNETCNF_H
#define _EZNETCNF_H


#include <netcnfif.h>


#ifdef  __cplusplus
extern "C" {
#endif


/*  --------------------------------------------------------
 *  enum constants and types
 *  --------------------------------------------------------
 */

/*
 *  I/F link encapsulation types
 *  mirrors sceEENET_IFTYPE_* equivalents from <libeenet/net/if.h>
 */
enum {
    eIftypeUnknown = 0,
    eIftypeLoop,
    eIftypeEther,
    eIftypePPP,
    eIftypePPPoE
};

/*
 *  I/F states
 *  mirrors sceINETCTL_S_* and sceEENETCTL_S_* equivalents
 */
enum {
    eStateDetached = 0,
    eStateStarting,
    eStateRetrying,
    eStateStarted,
    eStateStopping,
    eStateStopped,
    eStateDetaching
};

/*
 *  terminal I/F events
 *  mirrors sceEENETCTL_IEV_* equivalents
 */
enum {
    eIEV_Attach = 1,
    eIEV_UpCompleted = 10,
    eIEV_DownCompleted = 11,
    eIEV_DetachCompleted = 12
};

/*
 *  option flags for ezNetCnfGetCombinationList()
 */
#define ezNetCnfCheckProvider       0x1

/*
 *  option flags for ezNetCtlExit()
 */
#define ezNetCtlKeepEventHandler    0x1
#define ezNetCtlKeepModule          0x2


/*  --------------------------------------------------------
 *  ezNetCnf data types
 *  --------------------------------------------------------
 */

#define kMaxCombinations  10  // maximum number of Combinations
#define kStrBufferSize    256 // buffer  size for any text field
#define kStrDisplaySize   64  // display size for any text field

typedef struct ezNetCnfCombination {
    int     status;
    int     iftype;
    char    combinationName[kStrDisplaySize];
    char    ifcName[kStrDisplaySize];
    char    devName[kStrDisplaySize];
} ezNetCnfCombination_t;

typedef struct ezNetCnfCombinationList {
    int     listLength;
    unsigned int defaultNumber;
    unsigned int netdbOrder[kMaxCombinations];
    ezNetCnfCombination_t list[kMaxCombinations];
} ezNetCnfCombinationList_t __attribute__((aligned(64)));


/*  --------------------------------------------------------
 *  ezNetCtl data types
 *  --------------------------------------------------------
 */

#define kIfnameSize       16

typedef struct ezNetCtlStatus {
    int     ifid;       // Inet only
    int     ifid2;      // Inet only
    char    ifname [kIfnameSize];
    char    ifname2[kIfnameSize];
    int     iftype;
    int     error;      // do not display to end user
    int     state;
    int     link;
} ezNetCtlStatus_t __attribute__((aligned(64)));

typedef struct {
    int lo;
    int hi;
    const char *msg;
} ezNetCtlMsg_t;


/*  --------------------------------------------------------
 *  ezNetCnf library interfaces
 *  --------------------------------------------------------
 */

int     ezNetCnfInit(void);
int     ezNetCnfExit(void);

        // RPC functions
int     ezNetCnfGetCount(const char *);
int     ezNetCnfGetCombinationList(const char *, int, ezNetCnfCombinationList_t *);
ezNetCnfCombination_t *ezNetCnfGetCombination(ezNetCnfCombinationList_t *, int);
ezNetCnfCombination_t *ezNetCnfGetDefault(ezNetCnfCombinationList_t *);
int     ezNetCnfGetProperties(const char *, const char *, sceNetcnfifData_t *);


/*  --------------------------------------------------------
 *  ezNetCtl library interfaces
 *  --------------------------------------------------------
 */

int     ezNetCtlInit(void);
int     ezNetCtlExit(int);

        // RPC functions under Inet and EENet
int     ezNetCtlSetProperties(sceNetcnfifData_t *);
int     ezNetCtlSetCombination(const char *, const char *);
int     ezNetCtlPreloadModemCnf(const char *);
int     ezNetCtlUnloadModemCnf(void);

        // RPC functions under Inet
        // non-RPC functions under EENet
int     ezNetCtlUpInterface(void);
int     ezNetCtlDownInterface(void);
int     ezNetCtlGetStatus(ezNetCtlStatus_t *, char *, unsigned int);
int     ezNetCtlWaitEventTO(int);

        // non-RPC localization functions
void    ezNetCtlSwapMessageTables(ezNetCtlMsg_t *, ezNetCtlMsg_t *, ezNetCtlMsg_t *);
void    ezNetCtlSetLanguage(unsigned int);
void    ezNetCtlSetScfLanguage(void);


#ifdef  __cplusplus
}
#endif


#endif	// _EZNETCNF_H
