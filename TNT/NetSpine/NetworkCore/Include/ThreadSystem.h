/**
* \file     ThreadSystem.h
*
* \brief    This is the interface for the cross platformthreading system.  
*           This system makes it easier to use threads across the different platforms
*           and provides a hardware abstraction/emulation layer for threads.
*           Supported platforms:
*               Win32 - debug & release
*               PS2 devkit - debug & release
*
* \author   Nicholas Gonzalez
* \version  1.0
* \date		1/8/02
*/


#ifndef __THREADSYSTEM_H__
#define __THREADSYSTEM_H__
/*-----------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------------*/


#define TS_HANDLE           void*
typedef void (X_STDCALL X_THREAD_FUNC)(void*) ;


/**
*   \fn void TS_Init( ) ; 
*   \brief  Initializes the threading system.  
*   @return void
*   \b Must be called before any of the other thread functions.
*/
void            TS_Init( ) ;

/**
*   \fn void TS_DeInit( ) ; 
*   \brief  DeInitializes the thread system.
*   @return void
* 
*   \b This function frees all resources used by the threading system.
*/
void            TS_DeInit( ) ;

/**
* Thread Functions
*/

/**
*   \fn TS_HANDLE TS_BeginThread( X_THREAD_FUNC pFunc, void *pParam, xbool bJoinable ) ; 
*   \brief  Begin the execution of a specified worker thread function pFunc.
*   @param  pFunc       A pointer to the thread function.  Example: u32 X_STDCALL MyThread( void *pContext )
*   @param  *pParam     A void pointer that will be passed into the thread function as a parameter
*   @param  Flags       Platform specific flags.
*   @return TS_HANDLE   A handle to the newly created and started thread.
*			@arg \c 0 The function was successfull
* 
*   \remark Every thread MUST call TS_EndThread from within the thread to terminate execution of the thread.  Simply
*           returning from the thread WILL NOT terminate the thread.
*
*   \remark On the PS2 platform the Flags  parameter determines whether threads are joinable or not.
*           If it is set to TRUE you MUST call TS_WaitForThread.  If it is set to FALSE, you cannot call TS_WaitForThread.
*/
TS_HANDLE           TS_BeginThread( X_THREAD_FUNC pFunc, void *pParam, s32 Flags ) ;


/**
*   \fn void TS_EndThread( u32 dwRetVal ) ; 
*   \brief  Must be called to terminate a thread.
*   @param  dwRetVal The value returned by the thread.  This value can be obtained
*           by calling TS_WaitForThread, and can only be obtained if the thread was
*           created with the bJoinable flag set to TRUE.  If the thread is not joinable 
*           this parameter is ignored.
*   @return void
* 
*   \b This function cleans up the resources used by this thread.  If this function is
*      not called at the end of every thread there will be memory leaks and subsequent
*      calls to TS_BeginThread may fail.
*/
void                TS_EndThread( u32 dwRetVal ) ;


/**
*   \fn u32 TS_GetThreadID( ) ; 
*   \brief  Returns the id of the calling thread.
*   @return u32
*			@arg \c 0 The ID of the calling thread
*/
u32                 TS_GetThreadID( ) ;

/**
*   \fn u32 TS_WaitForThread( TS_HANDLE hThread ) ; 
*   \brief  Waits for an executing thread to complete.
*   @param  hThread The thread you are waiting for
*   @return u32 The value returned by the thread via TS_EndThread
* 
*   \b Only joinable threads can be waited for.  The behaviour for calling this function
*      with an invalid thread handle or an unjoinable thread is undefined.
*/
u32                 TS_WaitForThread( TS_HANDLE hThread ) ; //The thread must be joinable

/**
*   \fn void TS_Sleep( u32 dwTimeMs ) ; 
*   \brief  Puts the current thread to sleep for dwTimeMs milliseconds
*   @param  dwTimeMs The minimum number of milliseconds to sleep.  If there are other threads working
*           it is possible that this threads sleeps for more than dwTimeMs milliseconds.
*   @return void
* 
*/
void                TS_Sleep( u32 dwTimeMs ) ;


/**
*   \fn void TS_KillThread( TS_HANDLE hThread ) ; 
*   \brief  Terminates the execution of a thread.
*   @param  hThread The thread you wish to terminate.
*   @return void
* 
*/
//void              TS_KillThread( TS_HANDLE hThread ) ;


/**
* Critical Sections
*/
TS_HANDLE           TS_CreateCriticalSection( ) ;
void                TS_DeleteCriticalSection( TS_HANDLE hCS ) ;
void                TS_EnterCriticalSection( TS_HANDLE hCS ) ;
void                TS_LeaveCriticalSection( TS_HANDLE hCS ) ;


/**
* Semaphores
*/
TS_HANDLE           TS_CreateSemaphore( u32 dwInitialCount, u32 dwMaxCount ) ;
u32                 TS_DeleteSemaphore( TS_HANDLE hSema ) ;
u32                 TS_DecSemaphore( TS_HANDLE hSema ) ; //will wait if the value of the sema = 0, otherwise decrements the value of sema
u32                 TS_IncSemaphore( TS_HANDLE hSema ) ;

/**
* Events
* Similar to Win32 auto reset events.  Signalling an event will 
* wake up one thread that is waiting for the event.
*/
TS_HANDLE           TS_CreateEvent( ) ;
void                TS_DeleteEvent( TS_HANDLE hEvent ) ;
void                TS_SignalEvent( TS_HANDLE hEvent ) ;
void                TS_WaitForEvent( TS_HANDLE hEvent ) ;
/*-----------------------------------------------------------------*/
#ifdef __cplusplus
	}
#endif
/*-----------------------------------------------------------------*/



#endif //__THREADSYSTEM_H__