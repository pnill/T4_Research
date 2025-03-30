/**
* \file     w32_ThreadSystem.cpp
*
* \brief    
*
* \author   Nicholas Gonzalez
* \version  1.0
* \date		1/9/02
*/



#define MY_WIN32

#define TS_SUCCESS (0)
#define TS_ERROR (1)

#include <windows.h>
#include <process.h>
#include <stdio.h>

#include "cpc.h"
#include "ThreadSystem.h"


void TS_Init( )
{    
}

void TS_DeInit( )
{
}

/**
* all threads in windows are joinable
*/
TS_HANDLE TS_BeginThread( X_THREAD_FUNC pFunc, void *pParam, s32 Flags ) 
{
    unsigned int dwThreadID ;
    return (HANDLE)_beginthreadex(NULL, 0, (unsigned (__stdcall *)(void*))pFunc, pParam, Flags, &dwThreadID ) ;
}

void TS_EndThread( u32 dwReturnValue ) 
{
    _endthreadex((UINT)dwReturnValue); 
}

//-- todo: do we really need a timeout here?
u32 TS_WaitForThread( TS_HANDLE hThread )
{
    DWORD dwRetVal = WaitForSingleObject( hThread, INFINITE ) ;
    if( dwRetVal == WAIT_OBJECT_0 )
    {
        DWORD dwExitCode  = 0 ;
        GetExitCodeThread( hThread, &dwExitCode ) ;
        return dwExitCode ;
    }
    return 0 ;    
}

TS_HANDLE TS_CreateSemaphore( u32 dwInitialCount, u32 dwMaxCount )
{
    return CreateSemaphore( NULL, dwInitialCount, dwMaxCount, NULL ) ;
}

u32 TS_DeleteSemaphore( TS_HANDLE hSema )
{
    CloseHandle( hSema ) ;
    return TS_SUCCESS ;
}

u32 TS_IncSemaphore( TS_HANDLE hSema )
{
    //inc the semaphore by one
    LONG sdwPrevCount = 0 ;
    if( ReleaseSemaphore( hSema, 1, &sdwPrevCount ) )
    {
        return TS_SUCCESS ;
    }
    return TS_ERROR ;
}

u32 TS_DecSemaphore( TS_HANDLE hSema )
{
    // decrement the semaphore by one
    if( WAIT_OBJECT_0 == WaitForSingleObject( hSema, INFINITE ) )
        return TS_SUCCESS ;
    return TS_ERROR ;
}

/*========================================================================
 * Function : TS_CreateCriticalSection()
 * Date     : 9/11/2002
 */
TS_HANDLE TS_CreateCriticalSection( )
{
    CRITICAL_SECTION *hCS = (CRITICAL_SECTION*)x_malloc( sizeof(CRITICAL_SECTION) );

    InitializeCriticalSection( hCS ) ;

    return hCS ;
}

/*========================================================================
 * Function : TS_DeleteCriticalSection()
 * Date     : 9/11/2002
 */
void TS_DeleteCriticalSection( TS_HANDLE hCS )
{
    DeleteCriticalSection( (CRITICAL_SECTION*)hCS ) ;    
    x_free((CRITICAL_SECTION*)hCS);
}

/*========================================================================
 * Function : TS_EnterCriticalSection()
 * Date     : 9/11/2002
 */
void TS_EnterCriticalSection( TS_HANDLE hCS )
{
    EnterCriticalSection( (CRITICAL_SECTION*)hCS ) ;
}

/*========================================================================
 * Function : TS_LeaveCriticalSection()
 * Date     : 9/11/2002
 */
void TS_LeaveCriticalSection( TS_HANDLE hCS ) 
{
    LeaveCriticalSection( (CRITICAL_SECTION*)hCS ) ;
}

TS_HANDLE TS_CreateEvent( ) 
{
    // create an automatic reset event that will wake up ONE waiting thread
    HANDLE hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    return hEvent ;
}

void TS_DeleteEvent( TS_HANDLE hEvent ) 
{
    CloseHandle( (HANDLE)hEvent ) ;
}

void TS_SignalEvent( TS_HANDLE hEvent ) 
{
   SetEvent( (HANDLE)hEvent ) ; 
}

void TS_WaitForEvent( TS_HANDLE hEvent ) 
{
    WaitForSingleObject( (HANDLE)hEvent, INFINITE ) ;
}

void TS_Sleep( u32 dwTimeMs )
{
    Sleep(dwTimeMs);
}