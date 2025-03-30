////////////////////////////////////////////////////////////////////////////
//
// GC_DVD.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "x_memory.hpp"
#include "x_plus.hpp"

#include "Q_GC.hpp"
#include "Q_Engine.hpp"
#include "Q_Input.hpp"

#include "GC_DVD.hpp"
#include "GC_Font.hpp"
#include "GC_Video.hpp"
#include "GC_Input.hpp"
#include "E_Text.hpp"


////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////

// Debug compile-time switches
//#define GCDVD_DEBUG
//#define MAD_GCDVD_DEBUG


#define READ_CACHE_SIZE         ALIGN_32( 1024 * 64 )
#define ASYNC_BUF_SIZE          ALIGN_32( 1024 * 64 )

#define READ_PRIORITY_SYNC      2
#define READ_PRIORITY_ASYNC     3

#define MAX_GC_FILES            8
#define MAX_ASYNC_READ_TASKS    12
#define INVALID_TASK_ID         (-99)


#if defined( GCDVD_DEBUG ) && defined( X_DEBUG )
    #define DBG_EXP( exp )      exp
#else
    #define DBG_EXP( exp )
#endif

#if defined( MAD_GCDVD_DEBUG ) && defined( X_DEBUG )
    #define MAD_DBG_EXP( exp )  exp
#else
    #define MAD_DBG_EXP( exp )
#endif


////////////////////////////////////////////////////////////////////////////
// STRUCTURES
////////////////////////////////////////////////////////////////////////////

struct GC_FILE
{
    DVDFileInfo m_FileInfo;         // GameCube DVD file structure
    xbool       m_bOpen;            // is the file open
    u32         m_Filepos;          // current position of file "pointer"(synchronous reads only)
    u32         m_Filesize;         // size of file

#if (defined( GCDVD_DEBUG ) || defined( MAD_GCDVD_DEBUG )) && defined( X_DEBUG )
    char        m_Filename[256];    // filename used to open this file
#endif

    void Reset( void );
};

//--------------------------------------------------------------------------

struct GC_AsyncTask
{
    GC_FILE*        m_pFile;            // file handle that task will read from
    s32             m_TaskID;           // unique task ID
    byte*           m_pUserBuffer;      // pointer to buffer that data will be read into
    u32             m_FileOffset;       // offset into file where read will start from
    u32             m_SlackAmount;      // number of bytes requested data is off from aligned data read in(0-3 bytes)
    u32             m_BytesToRead;      // number of bytes remaining to read for this task
    u32             m_TotalBytesRead;   // total amount of data read from file
    s32             m_Status;           // current status of task
    s32             m_Priority;         // priority level of task
    GC_AsyncTask*   m_pNext;            // link to next task in list

    xbool IsValid( void );
    void  Reset  ( void );
};

//--------------------------------------------------------------------------

class GC_AsyncTaskList
{
  public:
    void          Reset  ( void );
    xbool         Add    ( const GC_AsyncTask& NewNode );
    xbool         Remove ( s32 TaskID );
    GC_AsyncTask* Find   ( s32 TaskID );
    GC_AsyncTask* GetHead( void );

  protected:
    GC_AsyncTask*  m_pListStart;
    GC_AsyncTask   m_pNodes[ MAX_ASYNC_READ_TASKS ];
};

//--------------------------------------------------------------------------

struct GC_ReadCache
{
    GC_FILE*    m_pFile;            // pointer to file that has data in the cache
    u32         m_Start;            // offset into file where cached data begins
    u32         m_End;              // offset into file where cached data ends
    byte*       m_CacheBuffer;      // pointer to cache buffer

    void Reset( void );
};


////////////////////////////////////////////////////////////////////////////
// LOCAL STATIC VARIABLES
////////////////////////////////////////////////////////////////////////////

// possibly allow the application to set this, but for now this is good.
static char* s_GameName = "All-Star Baseball 2004";

// DVD message format strings for each error that needs to be handled
static char* s_DVDErrorMsgs[GC_DVD_MSG_COUNT] =
{
    "Please insert the %s Game Disc.",
    "The Disc Cover is open.\nIf you want to continue the game,\nplease close the Disc Cover.",
    "Please insert the %s Game Disc.",
    "The Game Disc could not be read.\nPlease read the Nintendo GameCube\nInstruction Booklet for more information.",
    "An error has occurred.\nTurn the power off and refer to the\nNintendo GameCube Instruction Booklet\nfor further instructions.",
    " ",
};


// Default DVD error message handler info
static s32              s_DVDMsgDisplayed = -1;
static GC_DVD_MsgFunc   s_pMsgFunc        = NULL;

static xbool             s_bInSyncRead      = FALSE;
static QSyncReadCallback s_SyncReadCallback = NULL;

// File IO variables
static GC_FILE          s_Files[ MAX_GC_FILES ];
static GC_AsyncTaskList s_ReadTasksList;
static GC_AsyncTaskList s_ReadStatusList;
static GC_ReadCache     s_DVDReadCache;
static GC_ReadCache     s_DVDAsyncCache;

// Cache buffers for synchronous and asynchronous reads
static byte             s_DVDReadBuffer [ READ_CACHE_SIZE ] ATTRIBUTE_ALIGN(32);
static byte             s_DVDAsyncBuffer[ ASYNC_BUF_SIZE  ] ATTRIBUTE_ALIGN(32);


////////////////////////////////////////////////////////////////////////////
// LOCAL PROTOTYPES
////////////////////////////////////////////////////////////////////////////

static s32     GC_MakeUniqueTaskID ( void );
static void    GC_InsertReadTask   ( GC_AsyncTask& NewTask );
static void    GC_StartNextReadTask( void );
static void    GC_CompleteReadTask ( GC_AsyncTask* pReadTask, s32 ReadTaskStatus );
static void    GC_AsyncCallback    ( s32 Result, DVDFileInfo* pFileInfo );
static xbool   GC_CDReadAsync      ( GC_AsyncTask* pReadTask );

static X_FILE* GAMECUBE_Open       ( const char* pFilename, const char* pMode );
static void    GAMECUBE_Close      ( X_FILE* pXFile );
static s32     GAMECUBE_Read       ( X_FILE* pXFile, byte* pBuffer, s32 Bytes );
static s32     GAMECUBE_ReadA      ( void* pBuffer, s32 Bytes, X_FILE* pFile, s32 FileOffset, s32 Priority, s32& TaskID );
static s32     GAMECUBE_ReadAStatus( s32 TaskID, s32& BytesRead );
static s32     GAMECUBE_ReadACancel( s32 TaskID );
static s32     GAMECUBE_Write      ( X_FILE* pXFile, byte* pBuffer, s32 Bytes );
static s32     GAMECUBE_Seek       ( X_FILE* pXFile, s32 Offset, s32 Origin );
static s32     GAMECUBE_Tell       ( X_FILE* pXFile );
static xbool   GAMECUBE_EOF        ( X_FILE* pXFile );



////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// LOCAL STRUCTURES FUNCTION IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void GC_FILE::Reset( void )
{
    x_memset( &m_FileInfo, 0, sizeof(DVDFileInfo) );
    DVDSetUserData( &m_FileInfo.cb, this );

    m_bOpen    = FALSE;
    m_Filepos  = 0;
    m_Filesize = 0;

    #if (defined( GCDVD_DEBUG ) || defined( MAD_GCDVD_DEBUG )) && defined( X_DEBUG )
        m_Filename[0] = '\0';
    #endif
}

//==========================================================================

xbool GC_AsyncTask::IsValid( void )
{
    return m_TaskID != INVALID_TASK_ID;
}

//--------------------------------------------------------------------------

void GC_AsyncTask::Reset( void )
{
    m_pFile          = NULL;
    m_TaskID         = INVALID_TASK_ID;
    m_pUserBuffer    = NULL;
    m_FileOffset     = 0;
    m_SlackAmount    = 0;
    m_BytesToRead    = 0;
    m_TotalBytesRead = 0;
    m_Status         = X_STATUS_NOTFOUND;
    m_Priority       = X_PRIORITY_LOW;
    m_pNext          = NULL;
}

//==========================================================================

void GC_AsyncTaskList::Reset( void )
{
    s32 i;

    m_pListStart = NULL;

    for( i = 0; i < MAX_ASYNC_READ_TASKS; i++ )
        m_pNodes[i].Reset();
}

//--------------------------------------------------------------------------

GC_AsyncTask* GC_AsyncTaskList::GetHead( void )
{
    return m_pListStart;
}

//--------------------------------------------------------------------------

xbool GC_AsyncTaskList::Add( const GC_AsyncTask& NewNode )
{
    s32           Index;
    GC_AsyncTask* pNewbie = NULL;

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
            GC_AsyncTask* pPrev = m_pListStart;
            GC_AsyncTask* pTemp = m_pListStart->m_pNext;

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

xbool GC_AsyncTaskList::Remove( s32 TaskID )
{
    GC_AsyncTask* pPrev;
    GC_AsyncTask* pTemp;

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

GC_AsyncTask* GC_AsyncTaskList::Find( s32 TaskID )
{
    GC_AsyncTask* pTemp;

    pTemp = m_pListStart;

    while( pTemp != NULL )
    {
        if( pTemp->m_TaskID == TaskID )
            return pTemp;

        pTemp = pTemp->m_pNext;
    }

    return NULL;
}

//==========================================================================

void GC_ReadCache::Reset( void )
{
    m_pFile = NULL;
    m_Start = 0;
    m_End   = 0;

    // cache buffer is not reset - don't want to lose pointer to it, since
    // this function is only used to "clear" the cache of file data
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// GAMECUBE STDIO HANDLERS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

static X_FILE* GAMECUBE_Open( const char* pFilename, const char* pMode )
{
    GC_FILE* pFile  = NULL;
    xbool    Read   = FALSE;
    xbool    Write  = FALSE;
    xbool    Append = FALSE;
    s32      EntryNum;
    s32      i;
    char     UppercaseFilename[300];

    ASSERT( pFilename != NULL );
    ASSERT( pMode != NULL );

    DBG_EXP( x_printf( "DVD open: %s\n", pFilename ) );

    // Pick through the Mode characters.
    while( *pMode )
    {
        if( (*pMode == 'r') || (*pMode == 'R') )  Read   = TRUE;
        if( (*pMode == 'w') || (*pMode == 'W') )  Write  = TRUE;
        if( (*pMode == 'a') || (*pMode == 'A') )  Append = TRUE;
        ++pMode;
    }

    // GameCube DVD cannot write
    //ASSERT( Write == FALSE );
    if( Write || Append )
    {
        DBG_EXP( x_printf( "GAMECUBE WARNING: DVD doesn't support writing, file:\"%s\"\n", pFilename ) );

        return (X_FILE*)(NULL);
    }

    if( !Read )
    {
        DBG_EXP( x_printf( "GAMECUBE WARNING: DVD can only read files, file:\"%s\"\n", pFilename ) );

        return (X_FILE*)(NULL);
    }

    // Find an empty file slot
    for( i = 0; i < MAX_GC_FILES; i++ )
    {
        if( s_Files[i].m_bOpen == FALSE )
        {
            pFile = &s_Files[i];
            break;
        }
    }

    // If this fires, you have too many open files
    ASSERTS( pFile != NULL, "GCDVD: Too many open files" );
    if( pFile == NULL )
        return NULL;

    // Reset file structure
    pFile->Reset();

    // Make filename all uppercase with FORWARD slashes, not BACKSLASHES
    x_strcpy( UppercaseFilename, pFilename );
    x_strtoupper( UppercaseFilename );

    for( i = 0; UppercaseFilename[i] != '\0'; i++ )
    {
        if( UppercaseFilename[i] == '\\' )
            UppercaseFilename[i] = '/';
    }

    // Make sure the file exists on disk
    EntryNum = DVDConvertPathToEntrynum( (char*)UppercaseFilename );

    if( EntryNum == -1 )
    {
        DBG_EXP( x_printf( "GAMECUBE WARNING: Couldn't find file \"%s\"\n", UppercaseFilename ) );

        return (X_FILE*)NULL;
    }

    // File exists, try to open it
    if( DVDFastOpen( EntryNum, &pFile->m_FileInfo ) )
    {
        // File opened successfully
        pFile->m_bOpen    = TRUE;
        pFile->m_Filesize = DVDGetLength( &pFile->m_FileInfo );

        #if (defined( GCDVD_DEBUG ) || defined( MAD_GCDVD_DEBUG )) && defined( X_DEBUG )
            x_strncpy( pFile->m_Filename, UppercaseFilename, 256 );
        #endif

        return (X_FILE*)pFile;
    }

    // Open failed
    return (X_FILE*)NULL;
}

//==========================================================================

static void GAMECUBE_Close( X_FILE* pXFile )
{
    //ASSERT( pXFile != NULL );
    if( pXFile == NULL )
        return;

    GC_FILE *pFile = (GC_FILE*)pXFile;

    // Make sure file is open
    ASSERT( pFile->m_bOpen );

    // Make sure file is not in use by an async read task
    #if defined( X_DEBUG )
        GC_AsyncTask* pTask = s_ReadTasksList.GetHead();
        while( pTask != NULL )
        {
            ASSERTS( pTask->m_pFile != pFile, "Tried to close file used by async task" );
            pTask = pTask->m_pNext;
        }
    #endif

    // Call OS function to close file handle
    VERIFY( DVDClose( &pFile->m_FileInfo ) );

//    MAD_DBG_EXP( x_printf( "GC_DVD: Closed file %s\n", pFile->m_Filename ) );

    // reset file structure
    pFile->Reset();

    // if this file was the last to have data read into the cache(s), reset them
    if( s_DVDReadCache.m_pFile == pFile )
        s_DVDReadCache.Reset();

    if( s_DVDAsyncCache.m_pFile == pFile )
        s_DVDAsyncCache.Reset();
}

//==========================================================================

static s32 GAMECUBE_Read( X_FILE* pXFile, byte* pDestBuffer, s32 BytesToRead )
{
    u32   TotalBytesRead = 0;
    byte* pCacheBuffer   = NULL;
    u32   DiscReadLength = 0;
    u32   CopyLength     = 0;
    u32   DiscReadPos    = 0;
    u32   SlackAmount    = 0;
    s32   ReadStatus;

    ASSERT( s_bInSyncRead == FALSE );

    // Check to make sure another read isn't called from sync-read callback
    if( s_bInSyncRead )
        return 0;

    if( BytesToRead <= 0 )
        return 0;

    GC_FILE* pFile = (GC_FILE*)pXFile;

    ASSERT( pDestBuffer != NULL );
    ASSERT( pFile != NULL );
    ASSERT( pFile->m_bOpen );

    // If file pointer is at the end of the file, nothing to read
    if( pFile->m_Filepos >= pFile->m_Filesize )
        return 0;

    // If number of bytes requested to read goes beyond the end of the file,
    // shorten length to only reach the end
    if( (pFile->m_Filepos + BytesToRead) > pFile->m_Filesize )
        BytesToRead = pFile->m_Filesize - pFile->m_Filepos;

    // NOTE:
    // GC DVD read positions must be multiples of 4
    // GC DVD read lengths must be multiples of 32

    // Check to see if data we need is already in cache
    if( pFile == s_DVDReadCache.m_pFile )
    {
        // Is the filepos within the cached range?
        if( (pFile->m_Filepos >= s_DVDReadCache.m_Start) &&
            (pFile->m_Filepos <  s_DVDReadCache.m_End) )
        {
            // If requested bytes to read go beyond the end of the cache,
            // shorten cache copy length to only what's in the cache
            if( (pFile->m_Filepos + (u32)BytesToRead) > s_DVDReadCache.m_End )
                CopyLength = s_DVDReadCache.m_End - pFile->m_Filepos;
            else
                CopyLength = BytesToRead;

            // Data we need is in cache, simply copy data to user buffer
            pCacheBuffer = &s_DVDReadCache.m_CacheBuffer[ pFile->m_Filepos - s_DVDReadCache.m_Start ];
            x_memcpy( pDestBuffer, pCacheBuffer, CopyLength );

            // Flush CPU data cache to ensure data is written to main memory
            DCFlushRange( pDestBuffer, CopyLength );

            MAD_DBG_EXP( x_printf( "GC_DVD: Read %ld bytes(cache hit) from %s\n", (s32)CopyLength, pFile->m_Filename ) );

            // update current file position and pointers
            BytesToRead      -= CopyLength;
            pDestBuffer      += CopyLength;
            TotalBytesRead   += CopyLength;
            pFile->m_Filepos += CopyLength;

            // This should never happen, but just to be sure...
            if( pFile->m_Filepos > pFile->m_Filesize )
                pFile->m_Filepos = pFile->m_Filesize;

            // if all data requested was in cache, read is done and return N bytes read
            if( BytesToRead <= 0 )
            {
                // run user-defined callback if provided
                if( s_SyncReadCallback != NULL )
                    s_SyncReadCallback();

                return TotalBytesRead;
            }
        }
    }

    // "Lock" the sync-read so a user callback can't do another read
    s_bInSyncRead = TRUE;

    // Read data into cache buffer, if read size is larger than cahce, read the
    // data in chunks and copy to user destination buffer
    while( BytesToRead > 0 )
    {
        // Get amount of read slack that is needed for alignment
        SlackAmount = pFile->m_Filepos & 0x03;

        // Calculate where to start reading on the DVD, aligned to 4-byte boundary(round-down)
        DiscReadPos = (pFile->m_Filepos - SlackAmount);
        ASSERT( (DiscReadPos & 0x03) == 0 );

        // Calculate how much data to read- try to get entire file into cache if possible
        DiscReadLength = MIN( READ_CACHE_SIZE, ALIGN_32(pFile->m_Filesize - DiscReadPos) );

        // Read the data into the cache
        ReadStatus = DVDReadAsyncPrio( &pFile->m_FileInfo, s_DVDReadCache.m_CacheBuffer, DiscReadLength, DiscReadPos, NULL, READ_PRIORITY_SYNC );

        if( !ReadStatus )
        {
            // Read failed
            ASSERTS( FALSE, "Error on DVDRead()" );
            return 0;
        }

        // Block progress until read completes, checking for disk errors while waiting
        while( DVDGetFileInfoStatus( &pFile->m_FileInfo ) != DVD_STATE_END )
        {
            GC_CheckForReset();

            switch( DVDGetDriveStatus() )
            {
             case DVD_STATE_FATAL_ERROR: s_pMsgFunc( GC_DVD_MSG_FATALERROR );   break;
             case DVD_STATE_NO_DISK:     s_pMsgFunc( GC_DVD_MSG_NODISK     );   break;
             case DVD_STATE_COVER_OPEN:  s_pMsgFunc( GC_DVD_MSG_COVEROPEN  );   break;
             case DVD_STATE_WRONG_DISK:  s_pMsgFunc( GC_DVD_MSG_WRONGDISK  );   break;
             case DVD_STATE_RETRY:       s_pMsgFunc( GC_DVD_MSG_RETRYERROR );   break;

             default:
                s_pMsgFunc( GC_DVD_MSG_DONE );

                // run user-defined callback if provided
                if( s_SyncReadCallback != NULL )
                    s_SyncReadCallback();

                break;
            }
        }

        // Store used cache information for quick reads later
        s_DVDReadCache.m_pFile = pFile;
        s_DVDReadCache.m_Start = DiscReadPos;
        s_DVDReadCache.m_End   = DiscReadPos + DiscReadLength;

        // Copy cached data to dest buffer
        CopyLength = MIN( (READ_CACHE_SIZE - (s32)SlackAmount), BytesToRead );

        x_memcpy( pDestBuffer, &s_DVDReadCache.m_CacheBuffer[SlackAmount], CopyLength );

        // Flush CPU data cache to ensure data is written to main memory
        DCFlushRange( pDestBuffer, CopyLength );

        MAD_DBG_EXP( x_printf( "GC_DVD: Read %ld bytes from %s\n", (s32)CopyLength, pFile->m_Filename ) );

        // Advance pointers and update bytes read
        pFile->m_Filepos += CopyLength;
        pDestBuffer      += CopyLength;
        BytesToRead      -= CopyLength;
        TotalBytesRead   += CopyLength;
    }

    // "Unlock" the sync read
    s_bInSyncRead = FALSE;

    return TotalBytesRead;
}

//==========================================================================

static s32 GAMECUBE_ReadA( void* pBuffer, s32 Bytes, X_FILE* pFile, s32 FileOffset, s32 Priority, s32& TaskID )
{
    ASSERT( (Priority >= X_PRIORITY_LOW) && (Priority <= X_PRIORITY_TIMECRITICAL) );

    ASSERT( pBuffer != NULL );
    ASSERT( pFile != NULL );
    ASSERT( FileOffset >= 0 );
    ASSERT( (u32)FileOffset < ((GC_FILE*)pFile)->m_Filesize );
    ASSERT( Bytes > 0 );

    // If any parameter is invalid, then jump out early- garauntees that
    // there won't be reads of zero length and beyond end of file
    if( (Bytes <= 0) || (FileOffset < 0) || (pFile == NULL) ||
        (pBuffer == NULL) || ((u32)FileOffset >= ((GC_FILE*)pFile)->m_Filesize) )
    {
        TaskID = INVALID_TASK_ID;
        return FALSE;
    }

    // Make sure read doesn't go beyond the end of the file
    if( (FileOffset + Bytes) > (s32)((GC_FILE*)pFile)->m_Filesize )
        Bytes = ((GC_FILE*)pFile)->m_Filesize - FileOffset;

    // Shut down interrupt handlers
    OSDisableInterrupts();

    // Fill in the new task info structure
    GC_AsyncTask  NewTask;
    GC_AsyncTask* pLeadTask;

    TaskID = GC_MakeUniqueTaskID();

    NewTask.Reset();
    NewTask.m_pFile        = (GC_FILE*)pFile;
    NewTask.m_pUserBuffer  = (byte*)pBuffer;
    NewTask.m_Status       = X_STATUS_PENDING;
    NewTask.m_Priority     = Priority;
    NewTask.m_TaskID       = TaskID;
    NewTask.m_FileOffset   = FileOffset;
    NewTask.m_BytesToRead  = Bytes;

    // Add new task to read list
    GC_InsertReadTask( NewTask );

    pLeadTask = s_ReadTasksList.GetHead();

    // If the new task is the first task in the list, it needs to
    // be started immediately, because it is the active read task.
    if( pLeadTask->m_TaskID == TaskID )
    {
        GC_StartNextReadTask();
    }

    // Restart interrupt handlers
    OSEnableInterrupts();

    // This is here to check for standard DVD errors and report them to the user
    GC_CheckDriveStatus();

    return TRUE;
}

//==========================================================================

static s32 GAMECUBE_ReadAStatus( s32 TaskID, s32& BytesRead )
{
    GC_AsyncTask* pTask;
    s32 RValue = X_STATUS_NOTFOUND;

    // Initialize BytesRead to zero
    BytesRead = 0;

    // This is here to check for standard DVD errors and report them to the user
    GC_CheckDriveStatus();

    // Shut down interrupt handlers
    OSDisableInterrupts();

    if( TaskID == -1 )
    {
        // TaskID of -1 is a signal by the calling function to return the
        // status of the currently active read task.

        pTask = s_ReadTasksList.GetHead();

        if( pTask != NULL )
        {
            if( pTask->IsValid() )
            {
                BytesRead = pTask->m_TotalBytesRead;
                RValue    = pTask->m_Status;
            }
        }

        // Restart interrupt handlers
        OSEnableInterrupts();

        return RValue;
    }

    // Search for the requested task in the Read Task List.
    pTask = s_ReadTasksList.Find( TaskID );

    if( pTask != NULL )
    {
        BytesRead = pTask->m_TotalBytesRead;

        // Restart interrupt handlers
        OSEnableInterrupts();

        return pTask->m_Status;
    }

    // If unable to find the requested task, search for the task in the
    // Status Task List. If it is found in the Status Task List, reset
    // it's slot for use by future read tasks.
    pTask = s_ReadStatusList.Find( TaskID );

    if( pTask != NULL )
    {
        BytesRead = pTask->m_TotalBytesRead;
        RValue    = pTask->m_Status;

        s_ReadStatusList.Remove( TaskID );
    }

    // Restart interrupt handlers
    OSEnableInterrupts();

    return RValue;
}

//==========================================================================

static s32 GAMECUBE_ReadACancel( s32 TaskID )
{
    return X_STATUS_COMPLETE;
}

//==========================================================================

static s32 GAMECUBE_Write( X_FILE* pXFile, byte* pBuffer, s32 Bytes )
{
    //ASSERTS( FALSE, "GAMECUBE DOES NOT SUPPORT WRITING!" );
    return 0;
}

//==========================================================================

static s32 GAMECUBE_Seek( X_FILE* pXFile, s32 Offset, s32 Origin )
{
    GC_FILE* pFile = (GC_FILE*)pXFile;

    ASSERT( pFile != NULL );
    ASSERT( pFile->m_bOpen );

    switch( Origin )
    {
        case X_SEEK_SET:  pFile->m_Filepos  = Offset;                      break;
        case X_SEEK_CUR:  pFile->m_Filepos += Offset;                      break;
        case X_SEEK_END:  pFile->m_Filepos  = pFile->m_Filesize + Offset;  break;
    }

    // if seek goes out of range of file, clamp it accordingly
    if( pFile->m_Filepos < 0 )
        pFile->m_Filepos = 0;

    if( pFile->m_Filepos > pFile->m_Filesize )
        pFile->m_Filepos = pFile->m_Filesize;

    return 0;
}

//==========================================================================

static s32 GAMECUBE_Tell( X_FILE* pXFile )
{
    GC_FILE* pFile = (GC_FILE*)pXFile;

    ASSERT( pFile != NULL );
    ASSERT( pFile->m_bOpen );

    return pFile->m_Filepos;
}

//==========================================================================

static xbool GAMECUBE_EOF( X_FILE* pXFile )
{
    GC_FILE* pFile = (GC_FILE*)pXFile;

    ASSERT( pFile != NULL );

    return (pFile->m_Filepos >= pFile->m_Filesize) ? TRUE : FALSE;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// ASYNC LOADING HELPER FUNCTIONS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

static s32 GC_MakeUniqueTaskID( void )
{
    static s32 s_NewID = 0;

    s_NewID++;

    // Make sure the TASK ID is never 0 or less.
    if( s_NewID <= 0 )
        s_NewID = 1;

    return s_NewID;
}

//==========================================================================

static void GC_InsertReadTask( GC_AsyncTask& NewTask )
{
    xbool bFull;

    bFull = s_ReadTasksList.Add( NewTask );
    ASSERT( bFull );

    MAD_DBG_EXP( x_printf( "..... GC_READ: TaskID:%ld. GC_InsertReadTask: File:%s\n", NewTask.m_TaskID, NewTask.m_pFile->m_Filename ) );
    MAD_DBG_EXP( x_printf( "               NBytes to Read: %ld  FileOffset: %ld\n", NewTask.m_BytesToRead, NewTask.m_FileOffset ) );
}

//==========================================================================

static void GC_StartNextReadTask( void )
{
    GC_AsyncTask* pLeadTask;

    pLeadTask = s_ReadTasksList.GetHead();

    // Continue to start the most important read task until either you run out of
    // tasks due to errors, or you have a successful read.
    while( pLeadTask != NULL )
    {
        MAD_DBG_EXP( x_printf( "..... GC_READ: TaskID:%ld. GC_StartNextReadTask: File:%s\n", pLeadTask->m_TaskID, pLeadTask->m_pFile->m_Filename ) );

        // Start the read for this task, if successful break out of loop
        if( GC_CDReadAsync( pLeadTask ) )
            break;

        pLeadTask = s_ReadTasksList.GetHead();
    }
}

//==========================================================================

static void GC_CompleteReadTask( GC_AsyncTask* pReadTask, s32 ReadTaskStatus )
{
    xbool bAdded;

    MAD_DBG_EXP( x_printf( "..... GC_READ: TaskID:%ld. GC_CompleteReadTask: %s File:%s\n", pReadTask->m_TaskID, ReadTaskStatus == X_STATUS_COMPLETE ? "Success" : "Failure", pReadTask->m_pFile->m_Filename ) );

    // Set status on read task
    pReadTask->m_Status = ReadTaskStatus;

    // Make sure the task is in the read list
    pReadTask = s_ReadTasksList.Find( pReadTask->m_TaskID );
    ASSERT( pReadTask );

    // Copy it to the status list
    bAdded = s_ReadStatusList.Add( *pReadTask );
    ASSERT( bAdded );

    // Remove it from the read task list
    s_ReadTasksList.Remove( pReadTask->m_TaskID );
}

//==========================================================================

static void GC_AsyncCallback( s32 Result, DVDFileInfo* pFileInfo )
{
    GC_AsyncTask* pTask;

    pTask = s_ReadTasksList.GetHead();
    ASSERT( pTask != NULL );

    // make sure current lead task is what the callback thinks it is
    ASSERT( pTask->m_pFile == (GC_FILE*)DVDGetUserData( &pFileInfo->cb ) );

    // Check for bad result
    if( Result == -1 )
    {
        // Remove the active task from the read task list, and start next read task.
        GC_CompleteReadTask( pTask, X_STATUS_ERROR );

        GC_StartNextReadTask();
        return;
    }

    // Calc number of bytes to copy into user buffer
    s32 NBytes = MIN( (ASYNC_BUF_SIZE - pTask->m_SlackAmount), pTask->m_BytesToRead );

    x_memcpy( pTask->m_pUserBuffer, &s_DVDAsyncCache.m_CacheBuffer[pTask->m_SlackAmount], NBytes );
    DCFlushRange( pTask->m_pUserBuffer, NBytes );

    MAD_DBG_EXP( x_printf( "GC_DVD: Async Read %ld bytes from %s\n", NBytes, pTask->m_pFile->m_Filename ) );

    // Update file position, dest buffer pointer and number of bytes to read
    pTask->m_BytesToRead    -= NBytes;
    pTask->m_FileOffset     += NBytes;
    pTask->m_pUserBuffer    += NBytes;
    pTask->m_TotalBytesRead += NBytes;

    // Nothing left to read, report as done
    if( pTask->m_BytesToRead <= 0 )
    {
        // Remove the active task from the read task list, and start next read task.
        GC_CompleteReadTask( pTask, X_STATUS_COMPLETE );
    }

    // Keep the read task chain going
    GC_StartNextReadTask();
}

//==========================================================================

static xbool GC_CDReadAsync( GC_AsyncTask* pReadTask )
{
    u32   DiscReadLength;
    u32   DiscReadPos;
    byte* pCacheBuffer;

    ASSERT( pReadTask != NULL );

    // Check if there is data to get from cache
    if( pReadTask->m_pFile == s_DVDAsyncCache.m_pFile )
    {
        // Is the filepos and requested bytes within the cached range?
        if( (pReadTask->m_FileOffset >= s_DVDAsyncCache.m_Start) &&
            (pReadTask->m_FileOffset <  s_DVDAsyncCache.m_End) )
        {
            // only grab what is available in the cache, if not all of the data
            // is there, then it will be picked up by the next async read
            if( (pReadTask->m_FileOffset + pReadTask->m_BytesToRead) > s_DVDAsyncCache.m_End )
                DiscReadLength = s_DVDAsyncCache.m_End - pReadTask->m_FileOffset;
            else
                DiscReadLength = pReadTask->m_BytesToRead;

            MAD_DBG_EXP( x_printf( "..... GC_READ: TaskID:%ld. GC_CDReadAsync: Data found in cache, File:%s\n", pReadTask->m_TaskID, pReadTask->m_pFile->m_Filename ) );

            // Data we need is in cache, simply copy data to user buffer
            pCacheBuffer = &s_DVDAsyncCache.m_CacheBuffer[ pReadTask->m_FileOffset - s_DVDAsyncCache.m_Start ];
            x_memcpy( pReadTask->m_pUserBuffer, pCacheBuffer, DiscReadLength );

            DCFlushRange( pReadTask->m_pUserBuffer, DiscReadLength );

            // update file position and bytes read
            pReadTask->m_BytesToRead    -= DiscReadLength;
            pReadTask->m_FileOffset     += DiscReadLength;
            pReadTask->m_pUserBuffer    += DiscReadLength;
            pReadTask->m_TotalBytesRead += DiscReadLength;

            if( pReadTask->m_BytesToRead <= 0 )
            {
                GC_CompleteReadTask( pReadTask, X_STATUS_COMPLETE );

                // Must return FALSE to keep task chain going- a normal async read would keep
                // chain alive through the callback function, but since this was a simple
                // memcpy GC_StartNextReadTask() needs to keep the loop running
                return FALSE;
            }
        }
    }


    // Get amount of read slack is needed
    pReadTask->m_SlackAmount = pReadTask->m_FileOffset & 0x03;

    // Calculate where to start reading on the DVD, aligned to 4-byte boundary(round-down)
    DiscReadPos = (pReadTask->m_FileOffset - pReadTask->m_SlackAmount);
    ASSERT( (DiscReadPos & 0x03) == 0 );

    // Calc the read amount to take care of alignment
    DiscReadLength = MIN( ASYNC_BUF_SIZE, ALIGN_32(pReadTask->m_pFile->m_Filesize - DiscReadPos) );

    MAD_DBG_EXP( x_printf( "..... GC_READ: TaskID:%ld. GC_CDReadAsync: Reading data, File:%s\n", pReadTask->m_TaskID, pReadTask->m_pFile->m_Filename ) );

    // Try the async read
    if( !DVDReadAsyncPrio( &pReadTask->m_pFile->m_FileInfo, s_DVDAsyncCache.m_CacheBuffer, DiscReadLength, DiscReadPos, GC_AsyncCallback, READ_PRIORITY_ASYNC ) )
    {
        // Error occurred
        GC_CompleteReadTask( pReadTask, X_STATUS_ERROR );
        return FALSE;
    }

    // Mark task as in progress
    pReadTask->m_Status = X_STATUS_INPROGRESS;

    // Store used cache information
    s_DVDAsyncCache.m_pFile = pReadTask->m_pFile;
    s_DVDAsyncCache.m_Start = DiscReadPos;
    s_DVDAsyncCache.m_End   = DiscReadPos + DiscReadLength;

    return TRUE;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// OTHER FILE IO IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

static void DVDERR_VideoFlip( xbool bClear )
{
    // This function is used for the default error message display,
    // the reason ENG_PageFlip() isn't used is because there are unpredictable
    // complications from all the operations it does.  The message display
    // only needs text to be rendered, so this is all that is needed.

    // set background clear color
    GXColor BKColor = { 0, 0, 0, 255 };
    GXSetCopyClear( BKColor, GX_MAX_Z24 );

    // check for reset before v-sync
    GC_CheckForReset();

    // swap frame buffers
    VIDEO_PageFlip( bClear );

    // check for reset after v-sync
    GC_CheckForReset();

    // poll the input to update rumble motor status
    INPUT_CheckDevices();
}

//==========================================================================

static void DVDERR_PrintMsg( s32 Msg )
{
    s32   i;
    s32   SY;
    s32   XRes;
    s32   YRes;
    s32   NLines;
    s32   LineHeight;
    char* pCurLine;
    char* pNextLine;
    char  MsgStr[300];
    color FontColor( 245, 245, 245, 255 );

    // jump out if message can't be handled here
    if( (Msg < 0) || (Msg >= GC_DVD_MSG_DONE) )
        return;

    // set string to zeroes, next-line "finder" depends on an additional
    // null character after the normal string terminator
    x_memset( MsgStr, 0, sizeof(MsgStr) );

    // format the message string with the game name
    x_sprintf( MsgStr, s_DVDErrorMsgs[Msg], s_GameName, s_GameName, s_GameName );

    // get the screen resolution to center message
    ENG_GetResolution( XRes, YRes );

    // count number of lines needed and get height of font(to advance lines)
    NLines     = FONT_GetStringLineCount( MsgStr );
    LineHeight = FONT_GetLineHeight() + 4;

    // calculate starting Y position of message string
    SY = (YRes / 2) - ((NLines * LineHeight) / 2);

    // start out the next line at the start
    pNextLine = &MsgStr[0];

    FONT_BeginRender();

    for( i = 0; i < NLines; i++ )
    {
        // save current line to use as rendered string
        pCurLine = pNextLine;

        // find newline char, terminate that section of the string with null,
        // then advance one more char to start next line of message
        while( *pNextLine != '\0' )
        {
            if( *pNextLine == '\n' )
            {
                *pNextLine = '\0';
                pNextLine++;
                break;
            }

            pNextLine++;
        }

        // render the current line of text
        FONT_Render( pCurLine, ((XRes/2) - (FONT_GetStringWidth(pCurLine)/2)), SY, FontColor );

        SY += LineHeight;
    }

    FONT_EndRender();
}

//==========================================================================

static void GC_DefaultMessageFunc( s32 Msg )
{
    s32   i;
    color ScreenColor( 0, 0, 0, 255 );

    // Make sure message value is valid
    if( (Msg < 0) || (Msg >= GC_DVD_MSG_COUNT) )
        return;

    // If currently displayed message is the same, do nothing
    if( s_DVDMsgDisplayed == Msg )
        return;

    // If no message is displayed, and new message is DONE, nothing is displayed
    if( (s_DVDMsgDisplayed == -1) && (Msg == GC_DVD_MSG_DONE) )
        return;

    // TRC requires that all rumble motors be stopped during error message
    for( i = 0; i < PAD_MAX_CONTROLLERS; i++ )
        INPUT_RumbleStop( i );

    // Fill screen with solid color
    ENG_FillScreenArea( ENG_AREA_SCREEN, ScreenColor );

    // Save current message
    s_DVDMsgDisplayed = Msg;

    // Print out message text
    if( Msg == GC_DVD_MSG_DONE )
    {
        // Error status no longer exists, flip screen to previous display and reset displayed message
        DVDERR_VideoFlip( FALSE );
        s_DVDMsgDisplayed = -1;
    }
    else
    {
        DVDERR_PrintMsg( Msg );

        // Do two page flips to display message text, one that doesn't clear the frame buffer, and another that does
        DVDERR_VideoFlip( FALSE );
        DVDERR_VideoFlip( TRUE );
    }

}

//=========================================================================

void GC_SetMessageFunction( GC_DVD_MsgFunc MsgFunc )
{
    if( MsgFunc == NULL )
        s_pMsgFunc = GC_DefaultMessageFunc;
    else
        s_pMsgFunc = MsgFunc;
}

//==========================================================================

void GC_CheckDriveStatus( void )
{
    s32 DriveStatus = DVDGetDriveStatus();

    // Check if current status is one that needs to be handled
    if( DriveStatus == DVD_STATE_FATAL_ERROR ||
        DriveStatus == DVD_STATE_NO_DISK     ||
        DriveStatus == DVD_STATE_COVER_OPEN  ||
        DriveStatus == DVD_STATE_WRONG_DISK  ||
        DriveStatus == DVD_STATE_RETRY )
    {
        while( TRUE )
        {
            // Check for reset button press
            GC_CheckForReset();

            // display message based on current drive status
            switch( DriveStatus )
            {
             case DVD_STATE_FATAL_ERROR: s_pMsgFunc( GC_DVD_MSG_FATALERROR );   break;
             case DVD_STATE_NO_DISK:     s_pMsgFunc( GC_DVD_MSG_NODISK     );   break;
             case DVD_STATE_COVER_OPEN:  s_pMsgFunc( GC_DVD_MSG_COVEROPEN  );   break;
             case DVD_STATE_WRONG_DISK:  s_pMsgFunc( GC_DVD_MSG_WRONGDISK  );   break;
             case DVD_STATE_RETRY:       s_pMsgFunc( GC_DVD_MSG_RETRYERROR );   break;

             default:
                // error status has stopped, return to normal behavior
                s_pMsgFunc( GC_DVD_MSG_DONE );
                return;
            }

            DriveStatus = DVDGetDriveStatus();
        }
    }
}

//==========================================================================

void GCDVD_SetReadCallback( QSyncReadCallback CallbackFunc )
{
    s_SyncReadCallback = CallbackFunc;
}

//==========================================================================

err DVD_InitModule( void )
{
    DVD_KillModule();

    // setup the xfiles handlers to point to our functions
    x_SetIOHandlers( GAMECUBE_Open,
                     GAMECUBE_Close,
                     GAMECUBE_Read,
                     GAMECUBE_ReadA,
                     GAMECUBE_ReadAStatus,
                     GAMECUBE_ReadACancel,
                     GAMECUBE_Write,
                     GAMECUBE_Seek,
                     GAMECUBE_Tell,
                     GAMECUBE_EOF );

    return ERR_SUCCESS;
}

//==========================================================================

void DVD_KillModule( void )
{
    s32 i;

    x_SetIOHandlers( NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL );

    for( i = 0; i < MAX_GC_FILES; i++ )
        s_Files[i].Reset();

    s_DVDMsgDisplayed = -1;

    s_ReadTasksList.Reset();
    s_ReadStatusList.Reset();

    s_DVDReadCache.Reset();
    s_DVDReadCache.m_CacheBuffer = s_DVDReadBuffer;

    s_DVDAsyncCache.Reset();
    s_DVDAsyncCache.m_CacheBuffer = s_DVDAsyncBuffer;

    GC_SetMessageFunction( NULL );
}

//==========================================================================
