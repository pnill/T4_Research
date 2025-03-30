////////////////////////////////////////////////////////////////////////////
//
// PS2_Dev.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_stdio.hpp"
#include "x_plus.hpp"
#include "x_debug.hpp"

#include "PS2_Dev.hpp"

#include <sifdev.h>
#include <stdio.h>
#include "errno.h"
#include "sceerr.h"

//#define MAD_PS2CD_DEBUG

#if defined( MAD_PS2CD_DEBUG ) && defined( X_DEBUG )
    #define MAD_DBG_EXP( exp )  exp
#else
    #define MAD_DBG_EXP( exp )
#endif


#if !defined( TARGET_PS2_DVD )

////////////////////////////////////////////////////////////////////////////
// STORAGE
////////////////////////////////////////////////////////////////////////////

static QSyncReadCallback s_SyncReadCallback   = NULL;


#define MAX_PS2_FILES           8
#define MAX_ASYNC_READ_TASKS    12
#define INVALID_TASK_ID         (-99)
#define INVALID_SECTOR          (-1)

struct PS2_FILE
{
    s32         m_Handle;               // file pointer
    xbool       m_bOpen;            // is the file open
    s32         m_Filepos;          // current position of file "pointer"(synchronous reads only)
    s32         m_Filesize;         // size of file
    xbool       m_ReadMode;         // FALSE = SYNC , TRUE = ASYNC

#ifdef X_DEBUG
    char        m_Filename[256];    // filename used to open this file
#endif

    void Reset( void );
};

//--------------------------------------------------------------------------

struct PS2_AsyncTask
{
    PS2_FILE*       m_pFile;            // file handle that task will read from
    s32             m_TaskID;           // unique task ID
    byte*           m_pUserBuffer;      // pointer to buffer that data will be read into
    s32             m_FileOffset;       // offset into file where read will start from
    s32             m_TotalBytesRead;   // total amount of data read from file
    s32             m_Status;           // current status of task
    s32             m_Priority;         // priority level of task
    PS2_AsyncTask*  m_pNext;            // link to next task in list

    xbool IsValid( void );
    void  Reset  ( void );
};

//--------------------------------------------------------------------------

class PS2_AsyncTaskList
{
  public:
    void           Reset  ( void );
    xbool          Add    ( const PS2_AsyncTask& NewNode );
    xbool          Remove ( s32 TaskID );
    PS2_AsyncTask* Find   ( s32 TaskID );
    PS2_AsyncTask* GetHead( void );

  protected:
    PS2_AsyncTask* m_pListStart;
    PS2_AsyncTask  m_pNodes[ MAX_ASYNC_READ_TASKS ];
};

static s32     PS2DEV_MakeUniqueTaskID ( void );
static void    PS2DEV_InsertReadTask   ( PS2_AsyncTask& NewTask );
static void    PS2DEV_StartNextReadTask( void );
static void    PS2DEV_CompleteReadTask ( PS2_AsyncTask* pReadTask, s32 ReadTaskStatus );
static xbool   PS2DEV_ReadAsync        ( PS2_AsyncTask* pReadTask );
void PS2DEV_UpdateAsyncReads( xbool bContinueTasks = TRUE );

////////////////////////////////////////////////////////////////////////////
// LOCAL STATIC VARIABLES
////////////////////////////////////////////////////////////////////////////

// File IO variables
static PS2_FILE          s_PS2Files[ MAX_PS2_FILES ];
static PS2_AsyncTaskList s_PS2ReadTasksList;
static PS2_AsyncTaskList s_PS2ReadStatusList;

//static xbool             s_bInSyncRead        = FALSE;

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// LOCAL STRUCTURES FUNCTION IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void PS2_FILE::Reset( void )
{
    m_bOpen    = FALSE;
    m_Filepos  = 0;
    m_Filesize = 0;

    #ifdef X_DEBUG
        m_Filename[0] = '\0';
    #endif
}

//==========================================================================

xbool PS2_AsyncTask::IsValid( void )
{
    return m_TaskID != INVALID_TASK_ID;
}

//--------------------------------------------------------------------------

void PS2_AsyncTask::Reset( void )
{
    m_pFile          = NULL;
    m_TaskID         = INVALID_TASK_ID;
    m_pUserBuffer    = NULL;
    m_FileOffset     = 0;
    m_TotalBytesRead = 0;
    m_Status         = X_STATUS_NOTFOUND;
    m_Priority       = X_PRIORITY_LOW;
    m_pNext          = NULL;
}

//==========================================================================

void PS2_AsyncTaskList::Reset( void )
{
    s32 i;

    m_pListStart = NULL;

    for( i = 0; i < MAX_ASYNC_READ_TASKS; i++ )
        m_pNodes[i].Reset();
}

//--------------------------------------------------------------------------

PS2_AsyncTask* PS2_AsyncTaskList::GetHead( void )
{
    return m_pListStart;
}

//--------------------------------------------------------------------------

xbool PS2_AsyncTaskList::Add( const PS2_AsyncTask& NewNode )
{
    s32            Index;
    PS2_AsyncTask* pNewbie = NULL;

    // Find an unused task structure
    for( Index = 0; Index < MAX_ASYNC_READ_TASKS; Index++ )
    {
        if( !m_pNodes[Index].IsValid() )
        {
            pNewbie = &m_pNodes[Index];

            m_pNodes[Index] = NewNode;
            m_pNodes[Index].m_pNext = NULL;
            break;
        }
    }

    // Couldn't find an empty task
    if( pNewbie == NULL )
        return FALSE;

    // If this is the first one on the list, stick to the head.
    if( m_pListStart == NULL )
    {
        m_pListStart = pNewbie;
        m_pListStart->m_pNext = NULL;
    }
    else
    {
        if( (pNewbie->m_Priority > m_pListStart->m_Priority) &&
            (m_pListStart->m_Status == X_STATUS_PENDING) )
        {
            // new task has highest priority in list, add it there
            pNewbie->m_pNext = m_pListStart;
            m_pListStart = pNewbie;
        }
        else
        {
            PS2_AsyncTask* pPrev = m_pListStart;
            PS2_AsyncTask* pTemp = m_pListStart->m_pNext;

            while( pTemp != NULL )
            {
                if( (pNewbie->m_Priority > pTemp->m_Priority) &&
                    (pTemp->m_Status == X_STATUS_PENDING) )
                {
                    // new task's priority has higher value, insert it here
                    pNewbie->m_pNext = pTemp;
                    pPrev->m_pNext   = pNewbie;
                    break;
                }

                pPrev = pTemp;
                pTemp = pTemp->m_pNext;
            }

            // if reached end of list, new task has lowest priority and is added to end of list
            if( pTemp == NULL )
                pPrev->m_pNext = pNewbie;
        }
    }

    return TRUE;
}

//--------------------------------------------------------------------------

xbool PS2_AsyncTaskList::Remove( s32 TaskID )
{
    PS2_AsyncTask* pPrev;
    PS2_AsyncTask* pTemp;

    pPrev = pTemp = m_pListStart;

    // Run through list, try to find task with specified ID
    while( pTemp )
    {
        if( pTemp->m_TaskID == TaskID )
        {
            // if task is at start of list, set head to next task
            if( pTemp == m_pListStart )
                m_pListStart = pTemp->m_pNext;

            // remove task from list and reset it
            pPrev->m_pNext = pTemp->m_pNext;
            pTemp->m_pNext = NULL;
            pTemp->Reset();

            return TRUE;
        }
        else
        {
            pPrev = pTemp;
            pTemp = pTemp->m_pNext;
        }
    }

    return FALSE;
}

//--------------------------------------------------------------------------

PS2_AsyncTask* PS2_AsyncTaskList::Find( s32 TaskID )
{
    PS2_AsyncTask* pTemp;

    pTemp = m_pListStart;

    while( pTemp != NULL )
    {
        if( pTemp->m_TaskID == TaskID )
            return pTemp;

        pTemp = pTemp->m_pNext;
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// ASYNC LOADING HELPER FUNCTIONS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

static s32 PS2DEV_MakeUniqueTaskID( void )
{
    static s32 s_NewID = 0;

    s_NewID++;

    // Make sure the TASK ID is never 0 or less.
    if( s_NewID <= 0 )
        s_NewID = 1;

    return s_NewID;
}

//==========================================================================

static void PS2DEV_InsertReadTask( PS2_AsyncTask& NewTask )
{
    xbool bFull;

    bFull = s_PS2ReadTasksList.Add( NewTask );
    ASSERT( bFull );

    MAD_DBG_EXP( x_printf( "...PS2DEV InsertReadTask: TaskID:%ld  NBytes:%ld  Offset:%ld  File: %s\n", NewTask.m_TaskID, NewTask.m_TotalBytesRead, NewTask.m_FileOffset, NewTask.m_pFile->m_Filename ) );
}

//==========================================================================

static void PS2DEV_StartNextReadTask( void )
{
    PS2_AsyncTask* pLeadTask;

    pLeadTask = s_PS2ReadTasksList.GetHead();

    // Continue to start the most important read task until either you run out of
    // tasks due to errors, or you have a successful read.
    while( pLeadTask != NULL )
    {
        MAD_DBG_EXP( x_printf( "...PS2DEV StartNextReadTask: TaskID:%ld  File: %s\n", pLeadTask->m_TaskID, pLeadTask->m_pFile->m_Filename ) );

		// Seek to where we want the reads to start
		if( pLeadTask->m_FileOffset )
		{
			x_fseek( (X_FILE*)pLeadTask->m_pFile, pLeadTask->m_FileOffset, X_SEEK_SET );
		}

        // Start the read for this task, if successful break out of loop
        if( PS2DEV_ReadAsync( pLeadTask ) )
            break;

        pLeadTask = s_PS2ReadTasksList.GetHead();
    }
}

//==========================================================================

static void PS2DEV_CompleteReadTask( PS2_AsyncTask* pReadTask, s32 ReadTaskStatus )
{
    xbool bAdded;

    MAD_DBG_EXP( x_printf( "...PS2DEV CompleteReadTask: TaskID:%ld, Status:%s  File: %s\n", pReadTask->m_TaskID, ReadTaskStatus == X_STATUS_COMPLETE ? "Success" : "Failure", pReadTask->m_pFile->m_Filename ) );

    // Set status on read task
    pReadTask->m_Status = ReadTaskStatus;

    // Make sure the task is in the read list
    pReadTask = s_PS2ReadTasksList.Find( pReadTask->m_TaskID );
    ASSERT( pReadTask );

    // Copy it to the status list
    bAdded = s_PS2ReadStatusList.Add( *pReadTask );
    ASSERT( bAdded );

    // Remove it from the read task list
    s_PS2ReadTasksList.Remove( pReadTask->m_TaskID );
}

//==========================================================================

void PS2DEV_UpdateAsyncReads( xbool bContinueTasks )
{
    s32            Result;
    PS2_AsyncTask* pTask;

    // This prevents any synchonous read callback from causing havok by
    // checking the "lock" state of a sync-read
//    if( s_bInSyncRead )
//        return;

    pTask = s_PS2ReadTasksList.GetHead();

    // Check if there are any async reads to update
    if( pTask == NULL )
        return;

    // Check if read operation has completed
    sceIoctl(pTask->m_pFile->m_Handle, SCE_FS_EXECUTING, &Result);

    if( Result == TRUE )  // is it still reading?
        return;

    // Remove the active task from the read task list, and start next read task.
    PS2DEV_CompleteReadTask( pTask, X_STATUS_COMPLETE );

    if( bContinueTasks )
    {
        // Keep the read task chain going
        PS2DEV_StartNextReadTask();
    }
}

//==========================================================================

static xbool PS2DEV_ReadAsync( PS2_AsyncTask* pReadTask )
{
    MAD_DBG_EXP( x_printf( "...PS2DEV ReadAsync: TaskID:%ld  NBytes:%ld  File: %s\n", pReadTask->m_TaskID, pReadTask->m_TotalBytesRead, pReadTask->m_pFile->m_Filename ) );

    // Try the async read
    sceRead( pReadTask->m_pFile->m_Handle, pReadTask->m_pUserBuffer, pReadTask->m_TotalBytesRead );
    pReadTask->m_pFile->m_Filepos += pReadTask->m_TotalBytesRead;

    // Mark task as in progress
    pReadTask->m_Status = X_STATUS_INPROGRESS;

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////
// PS2 DEVKIT IO HANDLERS
////////////////////////////////////////////////////////////////////////////

X_FILE* PS2DEV_Open( const char* pFilename, const char* pMode )
{
    PS2_FILE* pFile  = NULL;
    char  Buf[X_MAX_PATH];
    xbool Read    = FALSE;
    xbool Write   = FALSE;
    xbool Append  = FALSE;
    xbool Async   = FALSE;
    s32   OpenArg = 0;
    s32   i;

    // Find an empty file slot
    for( i = 0; i < MAX_PS2_FILES; i++ )
    {
        if( s_PS2Files[i].m_bOpen == FALSE )
        {
            pFile = &s_PS2Files[i];
            break;
        }
    }

    // If this fires, you have too many open files
    ASSERTS( pFile != NULL, "PS2DEV: Too many open files" );
    if( pFile == NULL )
        return NULL;

    // Reset file structure
    pFile->Reset();

    // We need to prefix the filename with "host:".
    Buf[0] = 'h';
    Buf[1] = 'o';
    Buf[2] = 's';
    Buf[3] = 't';
    Buf[4] = ':';
    x_strcpy( &Buf[5], pFilename );

    // Pick through the Mode characters.
    while( *pMode )
    {
        if( (*pMode == 'r') || (*pMode == 'R') )  Read   = TRUE;
        if( (*pMode == 'w') || (*pMode == 'W') )  Write  = TRUE;
        if( (*pMode == 'a') || (*pMode == 'A') )  Append = TRUE;
        if( (*pMode == 'q') || (*pMode == 'Q') )  Async  = TRUE;
        ++pMode;
    }

    ASSERT( Read || Write );

    // Build parameter to sceOpen based on values found in Mode.
    if( Read  )
        OpenArg |= SCE_RDONLY;

    if( Write )
    {
        OpenArg |= SCE_WRONLY;

        if( Append )
        {
            OpenArg |= SCE_TRUNC;
        }
        else
        {
            OpenArg |= SCE_CREAT;
        }
    }

    if( Async )
        OpenArg |= SCE_NOWAIT;

    // For reference, here are the defines for the bit argument
    // to the sceOpen function:
    //
    //  #define SCE_RDONLY      0x0001
    //  #define SCE_WRONLY      0x0002
    //  #define SCE_RDWR        0x0003
    //  #define SCE_NBLOCK      0x0010  // Non-Blocking I/O
    //  #define SCE_APPEND      0x0100  // append (writes guaranteed at the end)
    //  #define SCE_CREAT       0x0200  // open with file create
    //  #define SCE_TRUNC       0x0400  // open with truncation
    //  #define SCE_NOBUF       0x4000  // no device buffer and console interrupt
    //  #define SCE_NOWAIT      0x8000  // asyncronous i/o

    pFile->m_Handle = sceOpen( Buf, OpenArg );

    // Make sure that the handle returned is valid, if it wasn't, just clear the file out, and leave.
    if( pFile->m_Handle >= 0 || pFile->m_Handle == (-SCE_EEXIST) )
    {
        pFile->m_ReadMode = Async;
        // all file access needs to check the execution state when in NOWAIT mode
        if( pFile->m_ReadMode )
        {
            s32 stillExecuting;
            do
            {
                sceIoctl(pFile->m_Handle, SCE_FS_EXECUTING, &stillExecuting);
            } while (stillExecuting == 1);
        }

        pFile->m_bOpen    = TRUE;
    }
    else
    {
        pFile->Reset( );
        pFile = NULL;
    }

    return (X_FILE*)pFile;
}

//==========================================================================

void PS2DEV_Close( X_FILE* pFile )
{
    PS2_FILE *pPS2File = (PS2_FILE*)pFile;
    sceClose( pPS2File->m_Handle );
    pPS2File->m_bOpen = FALSE;
}

//==========================================================================

s32 PS2DEV_Read( X_FILE* pFile, byte* pBuffer, s32 Bytes )
{
	s32 RetVal;
    PS2_FILE *pPS2File = (PS2_FILE*)pFile;

    ASSERT( !pPS2File->m_ReadMode );  // make sure it's not opened async
    
	RetVal = sceRead( pPS2File->m_Handle, pBuffer, Bytes );
    pPS2File->m_Filepos += Bytes;

    // run user-defined callback if provided
    if( s_SyncReadCallback != NULL )
        s_SyncReadCallback();

	return RetVal;
}

//==========================================================================

static s32 PS2DEV_ReadA( void* pBuffer, s32 Bytes, X_FILE* pFile, s32 FileOffset, s32 Priority, s32& TaskID )
{
//    return PS2DEV_Read( pFile, (byte*)pBuffer, Bytes );
    
    ASSERT( (Priority >= X_PRIORITY_LOW) && (Priority <= X_PRIORITY_TIMECRITICAL) );

    ASSERT( pBuffer != NULL );
    ASSERT( pFile != NULL );
    ASSERT( FileOffset >= 0 );
//    ASSERT( FileOffset < ((PS2_FILE*)pFile)->m_Filesize );
    ASSERT( Bytes > 0 );
    ASSERT( ((PS2_FILE*)pFile)->m_ReadMode );  // make sure it's not opened async

    // If any parameter is invalid, then jump out early- garauntees that
    // there won't be reads of zero length and beyond end of file
    if( (Bytes <= 0) || (FileOffset < 0) || (pFile == NULL) ||
        (pBuffer == NULL) )//|| (FileOffset >= ((PS2_FILE*)pFile)->m_Filesize) )
    {
        TaskID = INVALID_TASK_ID;
        return FALSE;
    }

    // Make sure read doesn't go beyond the end of the file
//    if( (FileOffset + Bytes) > (s32)((PS2_FILE*)pFile)->m_Filesize )
//        Bytes = ((PS2_FILE*)pFile)->m_Filesize - FileOffset;

    // Shut down interrupt handlers here(if there are any)
    //

    // Fill in the new task info structure
    PS2_AsyncTask  NewTask;
    PS2_AsyncTask* pLeadTask;

    TaskID = PS2DEV_MakeUniqueTaskID();

    NewTask.Reset();
    NewTask.m_pFile        = (PS2_FILE*)pFile;
    NewTask.m_pUserBuffer  = (byte*)pBuffer;
    NewTask.m_Status       = X_STATUS_PENDING;
    NewTask.m_Priority     = Priority;
    NewTask.m_TaskID       = TaskID;
    NewTask.m_FileOffset   = FileOffset;
    NewTask.m_TotalBytesRead = Bytes; 

    // Add new task to read list
    PS2DEV_InsertReadTask( NewTask );

    pLeadTask = s_PS2ReadTasksList.GetHead();

    // If the new task is the first task in the list, it needs to
    // be started immediately, because it is the active read task.
    if( pLeadTask->m_TaskID == TaskID )
    {
        PS2DEV_StartNextReadTask();
    }

    // Restart interrupt handlers(if there are any)
    //

    return TRUE;
}

//==========================================================================

static s32 PS2DEV_ReadAStatus( s32 TaskID, s32& BytesRead )
{
//    return X_STATUS_COMPLETE;
    
    PS2_AsyncTask* pTask;
    s32 RValue = X_STATUS_NOTFOUND;

    // Initialize BytesRead to zero
    BytesRead = 0;

    // Shut down interrupt handlers(if there are any)
    //

    if( TaskID == -1 )
    {
        // TaskID of -1 is a signal by the calling function to return the
        // status of the currently active read task.

        pTask = s_PS2ReadTasksList.GetHead();

        if( pTask != NULL )
        {
            if( pTask->IsValid() )
            {
                BytesRead = pTask->m_TotalBytesRead;
                RValue    = pTask->m_Status;
            }
        }

        // Restart interrupt handlers(if there are any)
        //

        // Keep the read task chain going
        PS2DEV_UpdateAsyncReads();

        return RValue;
    }

    // Search for the requested task in the Read Task List.
    pTask = s_PS2ReadTasksList.Find( TaskID );

    if( pTask != NULL )
    {
        BytesRead = pTask->m_TotalBytesRead;
        RValue    = pTask->m_Status;

        // Restart interrupt handlers(if there are any)
        //

        // Keep the read task chain going
        PS2DEV_UpdateAsyncReads();

        return RValue;
    }

    // If unable to find the requested task, search for the task in the
    // Status Task List. If it is found in the Status Task List, reset
    // it's slot for use by future read tasks.
    pTask = s_PS2ReadStatusList.Find( TaskID );

    if( pTask != NULL )
    {
        BytesRead = pTask->m_TotalBytesRead;
        RValue    = pTask->m_Status;

        s_PS2ReadStatusList.Remove( TaskID );
    }

    // Restart interrupt handlers(if there are any)
    //

    // Keep the read task chain going
    PS2DEV_UpdateAsyncReads();

    return RValue;
}

//==========================================================================

static s32 PS2DEV_ReadACancel( s32 TaskID )
{
	return X_STATUS_COMPLETE;
}

//==========================================================================

s32 PS2DEV_Write( X_FILE* pFile, byte* pBuffer, s32 Bytes )
{
    s32 RetValue = sceWrite( ((PS2_FILE*)pFile)->m_Handle, pBuffer, Bytes );

    return RetValue;
}

//==========================================================================

s32 PS2DEV_Seek( X_FILE* pFile, s32 Offset, s32 Origin )
{
    s32 Result;
    s32 Where = SCE_SEEK_SET;

    switch( Origin )
    {
    case X_SEEK_SET: 
        Where = SCE_SEEK_SET;
        ((PS2_FILE*)pFile)->m_Filepos = Offset;
        break;
    case X_SEEK_CUR:
        Where = SCE_SEEK_CUR;
        ((PS2_FILE*)pFile)->m_Filepos += Offset;
        break;
    case X_SEEK_END:
        Where = SCE_SEEK_END;
        ((PS2_FILE*)pFile)->m_Filepos = ((PS2_FILE*)pFile)->m_Filesize + Offset;
        break;
    }

    Result = sceLseek( ((PS2_FILE*)pFile)->m_Handle, Offset, Where );

    if( Result == -1 )  return -1;
    else                return  0;
}

//==========================================================================

s32 PS2DEV_Tell( X_FILE* pFile )
{
    ASSERT( !((PS2_FILE*)pFile)->m_ReadMode );
    return sceLseek( ((PS2_FILE*)pFile)->m_Handle, 0, SCE_SEEK_CUR );
}

//==========================================================================

xbool PS2DEV_EOF( X_FILE* pFile )
{
    // We have to do this the hard way.
    //  (1)  Save the current position.
    //  (2)  Seek to the end of the file.
    //  (3)  Save the 'end' position.
    //  (4)  Seek back to the saved position.
    //  (5)  Return comparison between original position and end position.
    //
    // NOTE:  Since this implementation is clearly inefficient, loops
    //        such as "while( !x_feof(f) )" are discouraged.

    s32 Pos;
    s32 End;

    Pos = x_ftell( pFile );     x_fseek( pFile,   0, X_SEEK_END );
    End = x_ftell( pFile );     x_fseek( pFile, Pos, X_SEEK_SET );

    return (Pos == End) ? TRUE : FALSE;
}

//==========================================================================

void PS2DEV_Interface( void )
{
    x_SetIOHandlers( PS2DEV_Open,
                     PS2DEV_Close,
                     PS2DEV_Read,
                     PS2DEV_ReadA,
                     PS2DEV_ReadAStatus,
                     PS2DEV_ReadACancel,
                     PS2DEV_Write,
                     PS2DEV_Seek,
                     PS2DEV_Tell,
                     PS2DEV_EOF );

    s_PS2ReadTasksList.Reset();
    s_PS2ReadStatusList.Reset();
}

//==========================================================================

void PS2DEV_SetReadCallback( QSyncReadCallback CallbackFunc )
{
    s_SyncReadCallback = CallbackFunc;
}

//==========================================================================
#endif // #if !defined( TARGET_PS2_DVD )