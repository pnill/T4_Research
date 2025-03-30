/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
*
*		Filename:	x_sock_PS2.c
*		Date:		9/14/2001
*
*		Desc:		<todo>
*
*
*		Revisions History:
*
*
\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/


////////////////////////////////////////////////////////////////////////////
//// INCLUDES
#include "x_files.hpp"
#include "x_sock.h"
#define _IN_H
#include "libeenet.h"


////////////////////////////////////////////////////////////////////////////
//// DEFINES
#define EENET_TPL		32
#define EENET_APP_PRIO	48



////////////////////////////////////////////////////////////////////////////
//// TYPES

////////////////////////////////////////////////////////////////////////////
//// GLOBALS
//static u_int* g_pNetBuff = NULL;
static u_int* g_pNetBuff_UA = NULL;
  

void no_select_for_ps2_clients( void ) { ASSERT(!"select not implemented"); }
/*========================================================================
 * Function: x_gethostname()
 * Desc    :
 */
s32 x_gethostname( XCHAR *name, s32 namelen )
{
    ASSERT(!"x_gethostname() Not implemented");
    
    //EXIT_SUCCESS
    return ( 0 );
}

void* internal_malloc( size_t size )
{
    return x_malloc( (s32)size ) ;
}

void internal_free( void *p )
{
    x_free( p ) ;
}

/*========================================================================
 * Function: DoPSX2Startup()
 * Desc    :
 */
u32 DoPSX2Startup( void )
{
//	s32 ret;

	ASSERT(NULL==g_pNetBuff_UA);

	// NOTE: All of the PS2 Stack creation and setup is done 
	// in the PS2NS_DeviceManager.cpp NSDM_Initialize() function.

//EXIT_SUCCESS
return ( 0 );
}

/*========================================================================
 * Function: DoPSX2Shutdown()
 * Desc    :
 */
u32 DoPSX2Shutdown( void )
{   

	// NOTE:  All of the PS2 Stack deinit memory cleanup is done
	// in the PS2NS_DeviceManager.cpp NSDM_DeInitialize() function.

	return 0;
}

/*========================================================================
 * Function: XSOCKInternal_SetBlockingMode()
 * Desc    :
 */
s32 XSOCKInternal_SetBlockingMode( X_SOCKET s, s32 dwNewBlockMode )
{
	ASSERT(XSOCK_IsReady());

	x_setsockopt(s, SOL_SOCKET, SO_NBIO, (char*) &dwNewBlockMode, sizeof(s32));

	// Successful return
	return 0;
}

