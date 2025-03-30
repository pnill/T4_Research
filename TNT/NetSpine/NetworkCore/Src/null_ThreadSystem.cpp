/**
* \file     null_ThreadSystem.cpp
*
* \brief    <TODO :: add brief description>
*
* \author   Daniel Melfi 
*
* \version  1.0
*
* \date     5/16/2002
*/
#include "x_files.hpp"
#include "ThreadSystem.h"

#define TS_SUCCESS (0)
#define TS_ERROR (1)

void TS_Init()
{
    ASSERT(!"Platform doesn't support threads!");
}

void TS_DeInit()
{
}

TS_HANDLE TS_BeginThread( X_THREAD_FUNC pFunc, void *pParam, xbool bJoinable ) 
{
    //EXIT_FAILURE
    return NULL;
}

void TS_EndThread( u32 dwReturnValue ) 
{
}

u32 TS_WaitForThread( TS_HANDLE hThread )
{
    //EXIT_FAILURE
    return TS_ERROR;
}

TS_HANDLE TS_CreateSemaphore( u32 dwInitialCount, u32 dwMaxCount )
{
    //EXIT_FAILURE
    return NULL;
}

u32 TS_DeleteSemaphore( TS_HANDLE hSema )
{
    //EXIT_FAILURE
    return TS_ERROR;
}

u32 TS_IncSemaphore( TS_HANDLE hSema )
{
    //EXIT_FAILURE
    return TS_ERROR;
}

u32 TS_DecSemaphore( TS_HANDLE hSema )
{
    //EXIT_FAILURE
    return TS_ERROR;
}

TS_HANDLE TS_CreateCriticalSection( )
{
    //EXIT_FAILURE
    return NULL;
}

void TS_DeleteCriticalSection( TS_HANDLE hCS )
{    
}

void TS_EnterCriticalSection( TS_HANDLE hCS )
{
    
}
void TS_LeaveCriticalSection( TS_HANDLE hCS ) 
{    
}

TS_HANDLE TS_CreateEvent() 
{
    //EXIT_FAILURE
    return NULL;   
}

void TS_DeleteEvent( TS_HANDLE hEvent ) 
{    
}

void TS_SignalEvent( TS_HANDLE hEvent ) 
{   
}

void TS_WaitForEvent( TS_HANDLE hEvent ) 
{
}