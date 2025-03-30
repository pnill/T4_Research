/**
* \file     ps2_ThreadSystem.cpp
*
* \brief    
*
* \author   Nicholas Gonzalez
* \version  1.0
* \date		1/9/02
*/

#define MY_PS2
#include "cpc.h"
#include "ThreadSystem.h"


#include <eekernel.h>
#include <sifrpc.h>
#include <sifdev.h>
#include <eeregs.h>
#include <libvu0.h>
#include <libdma.h>
#include <stdio.h>


static void ts_delay_wakeup (int id, u_short timeout, void *arg)
{
    iWakeupThread((int)arg);
}

void TS_Sleep( u32 dwTimeMs )
{
    int tid;
    tid = GetThreadId();
    SetAlarm((int)dwTimeMs, ts_delay_wakeup, (void *)tid);
    SleepThread();
}

#if 1
#include "GenLnkLst.h"

static  u32 g_dwThreadRefCount = 0;
#define PS2_MAX_THREADS (256)   // use this value to account for all possible thread ids (might need a map)
#define STACK_SIZE      (4096)  // WE CAN ONLY MAKE ?? THREADS ON THE PS2
#define PS2_THREADATTRIB    0x02000000
struct PS2_InternalThread
{
    int     nThreadID ;
    xbool   bJoinable ;
    TS_HANDLE hEndThreadEvent ; //only used if this thread is joinable
    u32     dwReturnValue ; // only used by joinable threads
    void   *pStack;
    
} ;

static SLNKLSTPTR g_ITS ;

/**
* Used for debugging
*/
void PrintSemaInfo( int id ) ; 


/////////////////////////////////////////////////////////////////////////////////////////
// INTERNAL FUNCTIONS FOR MAINTAINING THE INTERNAL THREAD LIST (RETURN VALUES, AND EVENTS)
/////////////////////////////////////////////////////////////////////////////////////////

//ITL = internal thread list...
static void* ITL_AddThread( int nID, xbool bJoinable, void *pStack )
{
    PS2_InternalThread *pThread ;
    pThread = (PS2_InternalThread*)x_malloc( sizeof( PS2_InternalThread ) ) ;
    pThread->nThreadID = nID ;
    pThread->hEndThreadEvent = TS_CreateEvent( ) ;
    pThread->bJoinable = bJoinable ;
    pThread->dwReturnValue = 0 ;
    pThread->pStack = pStack;
    SLNKLSTPTR_InsertAtHead( &g_ITS, pThread ) ;
    return pThread ;
}


static PS2_InternalThread* ITL_FindThread( int nID )
{
	SLSTNODEPTR *pNode = NULL ; 
	PS2_InternalThread *pThread = NULL;
    for( pNode = SLNKLSTPTR_GetHeadNode(&g_ITS) ; pNode ; pNode = SLNKLSTPTR_GetNextNode(&g_ITS, pNode) )
    {            
		pThread = (PS2_InternalThread*)SLNKLSTPTR_GetData(&g_ITS, pNode);
        if( pThread->nThreadID == nID)
        {
			return pThread;         
		}
	}
	return NULL;
}


static void ITL_RemoveThread( int nID )
{
	PS2_InternalThread *pThread = ITL_FindThread(nID);
	if( pThread )
	{
		if( pThread->bJoinable )
		{
			TS_DeleteEvent( pThread->hEndThreadEvent ) ;
		}

        //Cant remove node until system is shut down
        //when that occurs we'll remove all nodes and clean up the stacks
        //and nodes that are associated with those threads.
        //DM
//		SLNKLSTPTR_RemoveNodeOfData(&g_ITS, pThread);
//		x_free( pThread ) ;
//      pThread = NULL ;        
	}
}


/////////////////////////////////////////////////////////////////////////////////////////
// API IMPLEMENTATION FOR PLAYSTATION 2
/////////////////////////////////////////////////////////////////////////////////////////

void TS_Init( )
{
	g_dwThreadRefCount++;

	if ( 1 == g_dwThreadRefCount )
	{
	    SLNKLSTPTR_Init(&g_ITS);
        // make sure all of the threads are the same priority to start off with
        // or else we'll have starvation.
        ChangeThreadPriority( GetThreadId( ), 2) ;
    }
}

void TS_DeInit( )
{
	if ( 1 == g_dwThreadRefCount )
	{
        //See remove thread notes as to why we're doing this here
	    SLSTNODEPTR *pNode = NULL ; 
	    PS2_InternalThread *pThread = NULL;
        for( pNode = SLNKLSTPTR_GetHeadNode(&g_ITS) ; pNode ; pNode = SLNKLSTPTR_GetNextNode(&g_ITS, pNode) )
        {            
		    pThread = (PS2_InternalThread*)SLNKLSTPTR_GetData(&g_ITS, pNode);
            if( pThread )
            {
                x_free( pThread->pStack ) ;
                x_free( pThread ) ;
                pThread = NULL ;
		    }
	    }

        SLNKLSTPTR_RemoveAllNodes(&g_ITS);

        SLNKLSTPTR_DeInit(&g_ITS);
    }

	g_dwThreadRefCount--;
	
    return ;
}


TS_HANDLE TS_BeginThread( X_THREAD_FUNC pFunc, void *pParam, xbool bJoinable )
{
    struct ThreadParam tp ;
    int thread_id ;
    PS2_InternalThread *pThread = NULL;

	void *pUnaligned = (void*)x_malloc(STACK_SIZE + 16);
		
	tp.stack = (void*)ALIGN_16(pUnaligned);
/*
	if ( (  = ( char* ) ) == NULL )
    {
		x_printf( "malloc failed.\n" );
		return NULL;
	}
*/
	tp.entry = pFunc ;
    tp.stackSize = STACK_SIZE ;
    tp.gpReg = &_gp ;
    tp.initPriority = 32 ;
	tp.attr = PS2_THREADATTRIB;

    thread_id = ::CreateThread( &tp ) ;
    if( thread_id != -1 )
    {        
        //Once the thread is created initialize the state structure
        //initializes and allocates the structure
        pThread = (PS2_InternalThread*)ITL_AddThread( thread_id, bJoinable, pUnaligned ) ; 
        
        int thread_id2 = ::StartThread( thread_id, pParam ) ;

        if( pThread->nThreadID != thread_id2 )
        {
            ITL_RemoveThread( thread_id) ;
            return NULL ;
        }
    }

    return pThread ;
    // todo: handle x platform return values ;
}



u32  TS_GetThreadID( )
{
    return GetThreadId( ) ;
}



void TS_EndThread( u32 dwRetVal ) 
{
    PS2_InternalThread *pThread = ITL_FindThread( TS_GetThreadID( ) ) ;

	if(!pThread) return;

    //-- if the thread is not joinable we can clean up the resources it is using
    if( !pThread->bJoinable )
    {
        ITL_RemoveThread( TS_GetThreadID( ) ) ;
    }
    else
    {
        //if it is joinable we have to signal the finished event and set the return value
        pThread->dwReturnValue = dwRetVal ;
        TS_SignalEvent( pThread->hEndThreadEvent ) ;
    }
    ExitDeleteThread( ) ;
    

}

u32 TS_WaitForThread( TS_HANDLE hThread )
{
//    ASSERT( hThread ) ;
    PS2_InternalThread *pThread = (PS2_InternalThread*)hThread ;
    // check the list for the thread
    pThread = ITL_FindThread( pThread->nThreadID ) ;
    if( pThread && pThread->bJoinable )
    {
        TS_WaitForEvent( pThread->hEndThreadEvent ) ;
        u32 dwRetCode = pThread->dwReturnValue ;
        return dwRetCode ;
    }
    
    return 0; //todo: find a better return value to signify an error
}


TS_HANDLE TS_CreateSemaphore( u32 dwInitialCount, u32 dwMaxCount )
{
    struct SemaParam sema ;
    sema.currentCount = 0; //ignored, use initCount instead
    sema.maxCount = dwMaxCount ; // not correctly processed ;
    sema.initCount = dwInitialCount ;
    sema.numWaitThreads = 0 ;
    sema.attr = 0 ; 
    sema.option = 0;

    //todo: translate error codes
    return (TS_HANDLE)CreateSema( &sema ) ; 
}


u32 TS_DeleteSemaphore( TS_HANDLE hSema )
{

    DeleteSema( (int)hSema ) ;
    return 0 ;
}

u32  TS_DecSemaphore( TS_HANDLE hSema ) 
{
    WaitSema( (int)hSema ) ;
    return 0 ;
}

u32 TS_IncSemaphore( TS_HANDLE hSema ) 
{
    SignalSema( (int)hSema ) ;
    return 0 ;
}

TS_HANDLE TS_CreateCriticalSection( )
{
    struct SemaParam sema ;
    sema.currentCount = 0; //ignored, use initCount instead
    sema.maxCount = 0 ; // not correctly processed ;
    sema.initCount = 1 ;
    sema.numWaitThreads = 0 ;
    sema.attr = 0 ; 
    sema.option = 0;

    //todo: translate error codes
    return (TS_HANDLE)CreateSema( &sema ) ;    
}


void TS_DeleteCriticalSection( TS_HANDLE hCS )
{
    DeleteSema( (int)hCS ) ;
}

void TS_EnterCriticalSection( TS_HANDLE hCS )
{
    WaitSema( (int)hCS ) ;
}

void TS_LeaveCriticalSection( TS_HANDLE hCS ) 
{

    SignalSema( (int)hCS ) ;
}


TS_HANDLE TS_CreateEvent( )  //add auto reset parameter?
{   

    TS_HANDLE e = (TS_HANDLE)TS_CreateSemaphore( 0, 1 );
    //printf( "create event\n" ) ;
    //PrintSemaInfo( e ) ;
    return e ;

}

void TS_DeleteEvent( TS_HANDLE e)
{
    TS_DeleteSemaphore( (TS_HANDLE)e ) ;
}

void TS_SignalEvent( TS_HANDLE e )
{
    TS_IncSemaphore( (TS_HANDLE)e ) ;
}

void TS_WaitForEvent( TS_HANDLE e)
{
    //printf( "pre wait for event\n" );
    //PrintSemaInfo( e ) ;
    TS_DecSemaphore( (TS_HANDLE)e ) ;
    //printf( "post wait for event\n" );
    //PrintSemaInfo( e ) ;

}
//- should events be auto reset or manual reset? (should they wake up a single thread or many threads?)




/////////////////////////////////////////////////////////////////////////////////////////
// EXTRA FUNCTIONS USED FOR DEBUGGING
/////////////////////////////////////////////////////////////////////////////////////////

#ifdef PS2_TS_DEBUG_FUNCTIONS
void PrintSemaInfo( int id )
{
    struct SemaParam sema ;
    if( id == ReferSemaStatus( id, &sema ) )
    {
        printf( "Semaphore Info: id [%d], currentCount [%d], maxCount[%d]\ninitCount[%d], numWaitThreads[%d], attr[%d], option[%d]\n", 
            id, sema.currentCount, sema.maxCount, sema.initCount, sema.numWaitThreads, sema.attr, sema.option );
    }
    else
    {
        printf( "Refer sema status failed\n" ) ;
    }
}

void Ps2SemaphoreTest( )
{
    int sema_id ;
    struct SemaParam sema ;
    sema.currentCount = 1; 
    sema.maxCount = 1 ; // not correctly processed ;
    sema.initCount = 1 ;
    sema.numWaitThreads = 0 ;
    sema.attr = 0 ;

    sema.option = 0;

    sema_id = CreateSema( &sema ) ;

    printf( "Semaphore created\n", sema_id ) ;
    PrintSemaInfo( sema_id ) ;
    

    WaitSema( sema_id ) ;
    printf( "\nWait sema\n" ) ;
    PrintSemaInfo( sema_id ) ;
    

    
    SignalSema( sema_id ) ;
    printf( "\nSignal sema\n" ) ;
    PrintSemaInfo( sema_id ) ;
    


    SignalSema( sema_id ) ;
    printf( "\nSignal sema\n" ) ;
    PrintSemaInfo( sema_id ) ;
    

    SignalSema( sema_id ) ;
    printf( "\nSignal sema\n" ) ;
    PrintSemaInfo( sema_id ) ;
    

}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// NOTES AND TODO LIST
///////////////////////////////////////////////////////////////////////////////////////////////////
/*
TODO: for ps2:
    implement sleep function with timeout
    make Critical Sections reentrant
*/
#else

#define TS_SUCCESS (0)
#define TS_ERROR (1)

void TS_Init()
{
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


#endif