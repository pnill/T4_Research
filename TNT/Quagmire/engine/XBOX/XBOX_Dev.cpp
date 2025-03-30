//=====================================================================================================================================
//
// XBOX_Dev.cpp
//
//=====================================================================================================================================


//=====================================================================================================================================
// INCLUDES
//=====================================================================================================================================
#include "x_stdio.hpp"
#include "x_plus.hpp"
#include "x_debug.hpp"
#include "x_memory.hpp"
#include "x_time.hpp"

#include "XBOX_Dev.hpp"
#include "XBOX_Font.hpp"
#include "Q_Engine.hpp"
#include <xtl.h>

//=====================================================================================================================================
// DEFINES
//=====================================================================================================================================
#define MAX_XBOX_FILES          8
#define XBOX_READ_CACHE_SIZE    (1024 * 64)
#define XBOX_SYSREAD_CACHE_SIZE (1024 * 500)    // XBOX system default is 64KB
#define MAX_ASYNC_READ_TASKS    24
#define INVALID_TASK_ID         -99

//=====================================================================================================================================
// Debug Selections
//=====================================================================================================================================
//#define MAD_XBOX_IO_DEBUG_TEXT
//#define USE_ASYNC_READS_ONLY
//#define LOG_FILE_ACCESS

//=====================================================================================================================================
// ASYNC STATUS FLAGS
//=====================================================================================================================================
#define XBOX_ASYNC_NONE         0
#define XBOX_ASYNC_READING      1
#define XBOX_ASYNC_FINISHED     2
#define XBOX_ASYNC_FAILED       3

//=====================================================================================================================================
// FILE READ MODES  (Async read mode is set by passing q/Q as a open flag)
//=====================================================================================================================================
#define XBOX_RMODE_SYNC         0
#define XBOX_RMODE_ASYNC        1

//=====================================================================================================================================
// LOCAL DATA
//=====================================================================================================================================
static QSyncReadCallback   gReadCallback    = NULL;

#if defined( X_DEBUG ) && defined( LOG_FILE_ACCESS )

    // Access log data
    #define MAX_LOG_SIZE    1000

    typedef struct
    {
        char  Name[80];
        f64   ReadTime;
        f64   SeekTime;
        f64   OpenTime;
        xbool OpenStatus;
    } ACCESS_LOG;

    static ACCESS_LOG s_AccessLog[MAX_LOG_SIZE];
    static s32        s_HighestLogID = -1;

#endif

//=====================================================================================================================================
// Local Functions.
//=====================================================================================================================================


//=====================================================================================================================================
// STRUCTURES
//=====================================================================================================================================
// XBOX_FILE
//=====================================================================================================================================
struct XBOX_FILE
{
    // XBOX Specifics
    HANDLE      mhFile;             // Handle to the file on XBOX.
    xbool       mbOpen;             // Is the file open.

    // Information on how the file was created, or opened.
    u32         mOpenMode;          // What file access modes were used to open the file.
    u32         mShareMode;         // How is this file shared.
    u32         mReadMode;          // How is the file to be read (SYNC or ASYNC
    u32         mFileCreationMode;  // How should a the file be opened. Etc. Always create new, truncate, open existing, etc...
    u32         mFlags;             // Other file flags.

    // File Attributes.
#ifdef X_DEBUG
    char        mFilename[255];     // Debug info.
#endif
    s32         mFilesize;          // Size of the file.

    // Information about the drive the file resides on.
    s32         mDriveSectorSize;   // Size of a sector for the drive that the data is being read from.

    // File traversing.
    s32         mFilePos;           // Offset into the file.
    s32         mNBytesToRead;      // The number of bytes that are to be read.                 (During a synchronous read operation)
    s32         mBytesRead;         // How much data has already been copied to the user buffer.(During a synchronous read operation)
    void*       mpUserBuffer;       // Users data buffer to store read data.                    (During a synchronous read operation)

    OVERLAPPED  mXBOXAsyncInfo;     // Contains information used in asynchronous input and output (I/O)

    s32         mAccessLogID;       // Access log ID tag

    // Tool functions.
    void Reset( void );
    s32  GetSectorAddress( void );
};

//-------------------------------------------------------------------------------------------------------------------------------------
void XBOX_FILE::Reset(void)
{ 
    mhFile = NULL;
    mbOpen = FALSE;

    mOpenMode = 0; 
    mShareMode = 0;
    mFileCreationMode = 0;
    mFlags = 0;

    // File Attributes.
#ifdef X_DEBUG
    mFilename[0] = NULL;
#endif

    mFilesize = 0;
    mDriveSectorSize = 0;
    mFilePos = 0;

    mNBytesToRead = 0;
    mpUserBuffer = NULL;
    mBytesRead = 0;
}

//-------------------------------------------------------------------------------------------------------------------------------------
s32 XBOX_FILE::GetSectorAddress( void )
{
    s32 SectorAddress;

    // Set the sector address to the sector boundary either equal to, or below the current file position.
    SectorAddress = (mFilePos / mDriveSectorSize) * mDriveSectorSize;

    return SectorAddress;
}


//=====================================================================================================================================
// XBOX_AsyncTask
//=====================================================================================================================================
struct XBOX_AsyncTask
{
    XBOX_AsyncTask( void );
    xbool IsValid( void );
    void Reset( void );

    XBOX_FILE*      mpFile;
    s32             mTaskID;
    void*           mpUserBuffer;
    s32             mNBytesToRead;
    s32             mBytesRead;
    s32             mReadOffset;
    s32             mStatus;
    XBOX_AsyncTask* mpNext;
};

//-------------------------------------------------------------------------------------------------------------------------------------
XBOX_AsyncTask::XBOX_AsyncTask( void )
{ 
    Reset();
}

//-------------------------------------------------------------------------------------------------------------------------------------
xbool XBOX_AsyncTask::IsValid( void )
{
    return mTaskID != INVALID_TASK_ID;
}


//-------------------------------------------------------------------------------------------------------------------------------------
void XBOX_AsyncTask::Reset( void )
{
    mpFile        = NULL;
    mpUserBuffer  = 0;
    mNBytesToRead = 0;
    mBytesRead    = 0;
    mTaskID       = INVALID_TASK_ID;
    mReadOffset   = 0;
    mStatus       = X_STATUS_NOTFOUND;
    mpNext        = NULL;
}


//=====================================================================================================================================
// XBOXAsyncTaskList
//=====================================================================================================================================
class XBOXAsyncTaskList
{
    public:
        XBOXAsyncTaskList( void );

        xbool           Add( const XBOX_AsyncTask& NewNode );
        xbool           Remove( s32 TaskID );

        XBOX_AsyncTask* GetHead( void );
        XBOX_AsyncTask* GetNext( XBOX_AsyncTask* Current );
        XBOX_AsyncTask* Find( s32 TaskID );

    protected:
        XBOX_AsyncTask*  mpHead;
        XBOX_AsyncTask   mpNodes[MAX_ASYNC_READ_TASKS];
};


//-------------------------------------------------------------------------------------------------------------------------------------
XBOXAsyncTaskList::XBOXAsyncTaskList( void )
{
    mpHead = NULL;
}

//-------------------------------------------------------------------------------------------------------------------------------------
xbool XBOXAsyncTaskList::Add( const XBOX_AsyncTask& NewNode )
{
    s32             Index;
    XBOX_AsyncTask* pNewbie;

    // Find an opening.
    for( Index = 0; Index < MAX_ASYNC_READ_TASKS; Index++ )
    {
        if( mpNodes[Index].IsValid( ) == FALSE )
        {
            pNewbie = &mpNodes[Index];
            *pNewbie = NewNode;
            break;
        }
    }

    if( Index < MAX_ASYNC_READ_TASKS )
    {
        // If this is the first one on the list, stick to the head.
        if( mpHead == NULL )
        {
            mpHead = pNewbie;
            mpHead->mpNext = NULL;
        }
        else
        {
            // Walk the list until you add it to the end.
            XBOX_AsyncTask* pTemp;
            pTemp = mpHead;

            while( pTemp->mpNext )
                pTemp = pTemp->mpNext;

            pTemp->mpNext = pNewbie;
            pNewbie->mpNext = NULL;
        }
    }
    else
    {
        ASSERT( 0 );
        return FALSE;
    }

    return TRUE;
}

//-------------------------------------------------------------------------------------------------------------------------------------
xbool XBOXAsyncTaskList::Remove( s32 TaskID )
{
    XBOX_AsyncTask* pPrev;
    XBOX_AsyncTask* pTemp;

    pPrev = pTemp = mpHead;

    while( pTemp )
    {
        if( pTemp->mTaskID == TaskID )
        {
            if( pTemp == mpHead )
                mpHead = pTemp->mpNext;

            pPrev->mpNext = pTemp->mpNext;
            pTemp->mpNext = NULL;
            pTemp->Reset( );

            return TRUE;
        }
        else
        {
            pPrev = pTemp;
            pTemp = pTemp->mpNext;
        }
    }

    ASSERT( 0 );
    return FALSE;
}


//-------------------------------------------------------------------------------------------------------------------------------------
XBOX_AsyncTask* XBOXAsyncTaskList::GetHead( void )
{
    return mpHead;
}

//-------------------------------------------------------------------------------------------------------------------------------------
XBOX_AsyncTask* XBOXAsyncTaskList::GetNext( XBOX_AsyncTask* Current )
{
    return Current->mpNext;
}

//-------------------------------------------------------------------------------------------------------------------------------------
XBOX_AsyncTask* XBOXAsyncTaskList::Find( s32 TaskID )
{
    // Walk the list until you add it to the end.
    XBOX_AsyncTask* pTemp;
    pTemp = mpHead;

    if( pTemp )
    {
        while( pTemp->mTaskID != TaskID )
        {
            if( pTemp->mpNext )
                pTemp = pTemp->mpNext;
            else
                return NULL;
        }
        return pTemp;
    }
    else
    {
        return NULL;
    }
}


//=====================================================================================================================================
//XBOXReadCache
//=====================================================================================================================================
struct XBOXReadCache
{
    XBOX_FILE*  mCacheFile;
    byte        mBuffer[XBOX_READ_CACHE_SIZE];      // Cache used for the Asynchronous reads.
    s32         mMaxSectorsInCache;                 // Max number of sectors that the cache holds.

    s32         mStartAddr;                         // First byte in the file that is copied to the cache.
    s32         mEndAddr;                           // Last byte of the file that is copied into the cache.

    s32         mUserReadPos;                       // Location in the Cache block where the user data begins.
    s32         mExpectedBytesRead;                 // The number of bytes expected to be read in the next read.

    void Reset( void )
    { 
        mCacheFile = NULL;
        mStartAddr = mEndAddr = 0;
        mMaxSectorsInCache = 0;
        mUserReadPos = 0;
        mExpectedBytesRead = 0;
    }
};


//=====================================================================================================================================
// Local Functions
//=====================================================================================================================================
xbool          XBOX_ReadAsynchronous( void );
void  CALLBACK XBOX_ReadAsyncCallback( DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped );

s32            XBOX_MakeUniqueTaskID( void );
void           XBOX_InsertReadTask( XBOX_AsyncTask& NewTask );
void           XBOX_MoveToStatusList( s32 TaskID );
void           XBOX_StartNextReadTask( void );
void           XBOX_CompleteReadTask( XBOX_AsyncTask* pReadTask, s32 ReadTaskStatus, s32 GlobalAsyncStatus );
void           XBOX_PrintLastReadError( XBOX_FILE* pXBoxFile );


//=====================================================================================================================================
// STATIC-GLOBAL VARIABLES
//=====================================================================================================================================
static XBOX_FILE    s_XboxFiles[MAX_XBOX_FILES];        // Total number of files available.
static s32          s_XboxFilesOpen;                    // How many files are open.

XBOXReadCache   s_XBOXCache;                                 // Cache used for Asynchronous reads.
//XBOXReadCache   s_XBOXSyncCache;                             // Cache used for Synchronous reads.
//XBOX_AsyncTask  s_XBOXAReadTaskList[MAX_ASYNC_READ_TASKS];   // List of Asynchronous read tasks.
//XBOX_AsyncTask  s_XBOXAReadStatusList[MAX_ASYNC_READ_TASKS]; // List of Asynchronous Read Tasks that have failed.
XBOXAsyncTaskList sReadTasksList;
XBOXAsyncTaskList sReadStatusList;


//=====================================================================================================================================
// Asynchronous Task Handling functions.
//=====================================================================================================================================
s32 XBOX_MakeUniqueTaskID( void )
{
    static s32 sNewID = 0;
    sNewID++;

    // Make sure the TASK ID is never 0 or less.
    if( sNewID <= 0 )
        sNewID = 1;

    return sNewID;
}


//-------------------------------------------------------------------------------------------------------------------------------------
void XBOX_InsertReadTask( XBOX_AsyncTask& NewTask )
{
    xbool bFull;

    bFull = sReadTasksList.Add( NewTask );
    ASSERT( bFull );

#ifdef MAD_XBOX_IO_DEBUG_TEXT
    x_printf( "..... XBOX_READ: TaskID:%ld. XBOX_InsertReadTask: File:%s\n", NewTask.mTaskID, NewTask.mpFile->mFilename );
#endif
}

//-------------------------------------------------------------------------------------------------------------------------------------
void XBOX_MoveToStatusList( s32 TaskID )
{
    XBOX_AsyncTask* pFinishedTask;
    xbool           bAdded;

    // Find the task that is finished.
    pFinishedTask = sReadTasksList.Find( TaskID );
    ASSERT( pFinishedTask );

#ifdef MAD_XBOX_IO_DEBUG_TEXT
    x_printf( "..... XBOX_READ: TaskID:%ld. XBOX_MoveToStatusList: File:%s\n", pFinishedTask->mTaskID, pFinishedTask->mpFile->mFilename );
#endif

    // Move it to the status list.
    bAdded = sReadStatusList.Add( *pFinishedTask );
    ASSERT( bAdded );


    // Now that this info is in the status list, remove it from the ReadTaskList.
    sReadTasksList.Remove( pFinishedTask->mTaskID );

}


//-------------------------------------------------------------------------------------------------------------------------------------
void XBOX_StartNextReadTask( void )
{
    XBOX_AsyncTask*  pLeadTask;
    
    // Get the first read task.
    while( pLeadTask = sReadTasksList.GetHead() )
    {
#ifdef MAD_XBOX_IO_DEBUG_TEXT
x_printf( "..... XBOX_READ: TaskID:%ld. XBOX_StartNextReadTask: File:%s\n", pLeadTask->mTaskID, pLeadTask->mpFile->mFilename );
#endif
        // Signal this read task as In-Progress
        pLeadTask->mStatus = X_STATUS_INPROGRESS;

        // Seek to the desired file offset location.
        if( pLeadTask->mReadOffset )
            XBOX_Seek( (X_FILE*)pLeadTask->mpFile, pLeadTask->mReadOffset, X_SEEK_SET );

        // Begin the read.  If the read completes itself, just get out.
        if( XBOX_ReadAsynchronous(  ) )
            return;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
void XBOX_CompleteReadTask( XBOX_AsyncTask* pReadTask, s32 ReadTaskStatus, s32 GlobalAsyncStatus )
{
#ifdef MAD_XBOX_IO_DEBUG_TEXT
    x_printf( "..... XBOX_READ: TaskID:%ld. XBOX_CompleteReadTask: %s File:%s\n", pReadTask->mTaskID, ReadTaskStatus == X_STATUS_COMPLETE ? "Success" : "Failure", pReadTask->mpFile->mFilename );
#endif

    // If an error occurred, then release the information on the read error.
    if( ReadTaskStatus == X_STATUS_ERROR )
        XBOX_PrintLastReadError( pReadTask->mpFile );

    // Remove the active task from the read task list, and start next read task.
    pReadTask->mStatus = ReadTaskStatus;
    XBOX_MoveToStatusList( pReadTask->mTaskID );

    // Now start the next read task since this one is done.
    XBOX_StartNextReadTask( );
}


//=====================================================================================================================================
// XBOX STDIO HANDLERS
//=====================================================================================================================================
X_FILE* XBOX_Open( const char* pFilename, const char* pMode )
{
    XBOX_FILE*  pXBoxFile;
    char        UppercaseFilename[300];
    char        DriveLetter[4];    
    xbool       Read       = FALSE;
    xbool       Write      = FALSE;
    xbool       Append     = FALSE;
    xbool       Async      = FALSE;
    u32         FileIndex;
    s32         i;
#if defined( X_DEBUG ) && defined( LOG_FILE_ACCESS )
    f64         InTime = x_GetTime();
#endif

    // First determine if there are any XBOX file handlers free.
    //-------------------------------------------------------------------------------------------------------------------------------------
    pXBoxFile = NULL;
    for( FileIndex = 0; FileIndex < MAX_XBOX_FILES; FileIndex++ )
    {
        if( s_XboxFiles[FileIndex].mbOpen == FALSE )
        {
            pXBoxFile = &s_XboxFiles[FileIndex];
            break;
        }
    }

    // Was there a free file found?
    //-------------------------------------------------------------------------------------------------------------------------------------
    if( pXBoxFile == NULL )
    {
        return NULL;
    }

    // Pick through the Mode characters.
    //-------------------------------------------------------------------------------------------------------------------------------------
    while( *pMode )
    {
        if( (*pMode == 'r') || (*pMode == 'R') )  Read   = TRUE;
        if( (*pMode == 'w') || (*pMode == 'W') )  Write  = TRUE;
        if( (*pMode == 'a') || (*pMode == 'A') )  Append = TRUE;
        if( (*pMode == 'q') || (*pMode == 'Q') )  Async   = TRUE;
        ++pMode;
    }

#if defined( USE_ASYNC_READS_ONLY )
    Async = TRUE;
#endif

    // Allow either read or write, but not both.
    ASSERTS( ((Read && !Write) || (!Read && Write)), "XBOX doesn't allow READs and WRITEs at the same time." );

    // Build parameter to sceOpen based on values found in Mode.
    //-------------------------------------------------------------------------------------------------------------------------------------
    pXBoxFile->mOpenMode         = 0;
    pXBoxFile->mShareMode        = 0;
    pXBoxFile->mFileCreationMode = 0;
    pXBoxFile->mFlags            = 0;

    if( Read  )
    {
        pXBoxFile->mOpenMode        |= GENERIC_READ;
        pXBoxFile->mShareMode       |= FILE_SHARE_READ;
        pXBoxFile->mFileCreationMode = OPEN_EXISTING;
        if( Async )
        {
            pXBoxFile->mFlags            = FILE_ATTRIBUTE_READONLY | FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING;
            pXBoxFile->mReadMode         = XBOX_RMODE_ASYNC;
        }
        else
        {
            pXBoxFile->mFlags            = 0;//FILE_ATTRIBUTE_READONLY | FILE_FLAG_RANDOM_ACCESS;
            pXBoxFile->mReadMode         = XBOX_RMODE_SYNC;
        }
    }
    if( Write )
    {
        pXBoxFile->mOpenMode        |= GENERIC_WRITE;
        pXBoxFile->mShareMode       |= FILE_SHARE_WRITE;
        pXBoxFile->mFileCreationMode = CREATE_ALWAYS;
    }
    if( Append )
    {
        pXBoxFile->mOpenMode        |= GENERIC_WRITE;
        pXBoxFile->mShareMode       |= FILE_SHARE_WRITE;
        pXBoxFile->mFileCreationMode = OPEN_ALWAYS;
    }

    // Make filename all uppercase with BACK slashes
    //-------------------------------------------------------------------------------------------------------------------------------------
    x_strcpy( UppercaseFilename, pFilename );
    x_strtoupper( UppercaseFilename );

    for( i = 0; UppercaseFilename[i] != '\0'; i++ )
    {
        if( UppercaseFilename[i] == '/' )
            UppercaseFilename[i] = '\\';
    }

    // Strip out the drive letter from the filename.
    //-------------------------------------------------------------------------------------------------------------------------------------
    x_strncpy( DriveLetter, UppercaseFilename, 3 );
    DriveLetter[3] = NULL;

#ifdef X_DEBUG
    x_strncpy( pXBoxFile->mFilename, UppercaseFilename, 255 );
#endif

    // Now that you have everytying, open/create the file.
        //-------------------------------------------------------------------------------------------------------------------------------------
    pXBoxFile->mhFile = CreateFile( UppercaseFilename, pXBoxFile->mOpenMode, pXBoxFile->mShareMode, NULL, pXBoxFile->mFileCreationMode, pXBoxFile->mFlags, NULL );
    if( pXBoxFile->mhFile == INVALID_HANDLE_VALUE )
    {
        XBOX_PrintLastReadError( pXBoxFile );
        return NULL;
    }

    if( Async )
    {
        // Now that the file has been opened/created, it's time to get some statistics on it.
        //-------------------------------------------------------------------------------------------------------------------------------------
        // Determine the size of a single sector on the disk where this file exists.
        pXBoxFile->mDriveSectorSize = XGetDiskSectorSize( DriveLetter );
        ASSERT( pXBoxFile->mDriveSectorSize );
    }
    
    // Determine the size of the opened file.
    pXBoxFile->mFilesize = GetFileSize( pXBoxFile->mhFile, NULL );

    // Set the file position to the beginning of the file.
    pXBoxFile->mFilePos = 0;


    // We need to manually seek to the end of the file for append
    if( Append )
    {
        XBOX_Seek( (X_FILE*)pXBoxFile, 0, X_SEEK_END );
    }

    // Increment the number of open files.
    s_XboxFilesOpen++;

    // Identify that the file is open.
    pXBoxFile->mbOpen = TRUE;

#if defined( X_DEBUG ) && defined( LOG_FILE_ACCESS )
    s32 Found = FALSE;
    f64 OutTime = x_GetTime();
    for( i = 0; i < s_HighestLogID; i++ )
    {
        if( x_strcmp( pFilename, s_AccessLog[i].Name ) == 0 )
        {
            Found = i;
            break;
        }
    }
    if( !Found )
    {
        if( s_HighestLogID + 1 < MAX_LOG_SIZE )
        {
            s_HighestLogID++;
            Found = s_HighestLogID;
            s_AccessLog[Found].OpenTime = 0.0f;
        }
        else
        {
            Found = -1;
            x_printf("XBOX_DEV: Could not log access to file %s.\n", pFilename );
        }
    }
    if( Found >= 0 )
    {
        x_strcpy( s_AccessLog[Found].Name, pFilename );
        s_AccessLog[Found].OpenStatus = TRUE;
        s_AccessLog[Found].ReadTime = 0.0f;
        s_AccessLog[Found].SeekTime = 0.0f;
        s_AccessLog[Found].OpenTime += x_GetTimeDiff( InTime, OutTime );
        pXBoxFile->mAccessLogID = Found;
    }
#endif
    // Send back the file pointer.
    return( (X_FILE*)pXBoxFile );
}


//==========================================================================
void XBOX_Close( X_FILE* pFile )
{
    XBOX_FILE* pXBoxFile = (XBOX_FILE*)pFile;

#if defined( X_DEBUG ) && defined( LOG_FILE_ACCESS )
    f64         InTime = x_GetTime();
#endif

    // Make sure that the file is open.
    if( pXBoxFile->mbOpen )
    {
        // Test to see if this file controls the cache, if it does reset the cache.
        if( s_XBOXCache.mCacheFile == pXBoxFile )
        {
            s_XBOXCache.Reset( );
        }

        // Close the file, and reset the files status structure.
        CloseHandle( pXBoxFile->mhFile );

        // Reset the file handle.
        pXBoxFile->Reset();

        // Reduce the number of open files.
        s_XboxFilesOpen--;
    }
    else
    {
#ifdef MAD_XBOX_IO_DEBUG_TEXT
        // Trying to close a file that isn't open.
        x_printf( "-------------------------------------------------------------------------------------------------------\n" );
        x_printf( "*** File I/O Error *** Trying to close a file that was already closed....\n" );
        x_printf( "-------------------------------------------------------------------------------------------------------\n" );
#endif
    }

#if defined( X_DEBUG ) && defined( LOG_FILE_ACCESS )
    if( pXBoxFile->mAccessLogID >= 0 )
    {
        s_AccessLog[pXBoxFile->mAccessLogID].OpenTime += x_GetTimeDiff( InTime, x_GetTime() );
        s_AccessLog[pXBoxFile->mAccessLogID].OpenStatus = FALSE;
    }
#endif
}


//=====================================================================================================================================

static void XBOX_DVDERR_PrintMsg( const char* pMsgStr )
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

    if( pMsgStr == NULL )
        return;

    // set string to zeroes, next-line "finder" depends on an additional
    // null character after the normal string terminator
    x_memset( MsgStr, 0, sizeof(MsgStr) );

    x_strcpy( MsgStr, pMsgStr );

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

//=====================================================================================================================================

static void XBOX_DisplayDiscErrorMsg( void )
{
//	char ErrStr[] = "There's a problem with the disc you're using.\nIt may be dirty or damaged.\nPress A to continue.";
	char ErrStr[] = "There's a problem with the disc you're using.\nIt may be dirty or damaged.";

	xbool bInRenderMode = FALSE;

	if( ENG_GetRenderMode() )
	{
		bInRenderMode = TRUE;
		ENG_EndRenderMode();
	}

    while( TRUE )
    {
		ENG_BeginRenderMode();
		ENG_FillScreenArea( ENG_AREA_SCREEN, color(0,0,0,255) );

		XBOX_DVDERR_PrintMsg( ErrStr );

		ENG_EndRenderMode();
        ENG_PageFlip();
    }

	if( bInRenderMode )
		ENG_BeginRenderMode();
}

//=====================================================================================================================================
s32 XBOX_Read( X_FILE* pFile, byte* pBuffer, s32 Bytes )
{
    XBOX_FILE* pXBoxFile = (XBOX_FILE*)pFile;
#if defined( X_DEBUG ) && defined( LOG_FILE_ACCESS )
    f64         InTime = x_GetTime();
#endif

    ASSERT( pFile );
    ASSERT( pBuffer );
    ASSERT( Bytes );

    if( pFile == NULL )
    {
        XBOX_DisplayDiscErrorMsg();
        return 0;
    }

    if( (pBuffer == NULL) || (Bytes <= 0) )
    {
        return 0;
    }

    if( pXBoxFile->mReadMode == XBOX_RMODE_SYNC )
    {
#ifdef MAD_XBOX_IO_DEBUG_TEXT
        x_printf( "+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+\n" );
        x_printf( "XBOX_READ: New Synchronous Read: %d from %s\n", Bytes, pXBoxFile->mFilename );
#endif

        pXBoxFile->mpUserBuffer = pBuffer;
        pXBoxFile->mNBytesToRead = Bytes;


        if( ReadFile( pXBoxFile->mhFile, pXBoxFile->mpUserBuffer, pXBoxFile->mNBytesToRead, (LPDWORD)&pXBoxFile->mBytesRead, NULL ) )
        {
#ifdef MAD_XBOX_IO_DEBUG_TEXT
            x_printf( "XBOX_READ: Completed SYNCHRONOUS read of %d from %s\n", pXBoxFile->mBytesRead, pXBoxFile->mFilename );
            x_printf( "+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+\n" );
#endif
            // Advance the local File position by the number of bytes read since it's 
            // actually being done to the file behind the scenes.
            pXBoxFile->mFilePos += pXBoxFile->mBytesRead;
#if defined( X_DEBUG ) && defined( LOG_FILE_ACCESS )
            if( pXBoxFile->mAccessLogID >= 0 )
            {
                s_AccessLog[pXBoxFile->mAccessLogID].ReadTime += x_GetTimeDiff( InTime, x_GetTime() );
            }
#endif
            if( gReadCallback ) gReadCallback();

            return pXBoxFile->mBytesRead;
        }
        else
        {
#ifdef MAD_XBOX_IO_DEBUG_TEXT
            x_printf( "XBOX_READ: Error Reading %s\n\n\n\n\n", pXBoxFile->mFilename );
            x_printf( "+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+\n" );
#endif
            // Microsoft Standards require us to display a message when the disc is screwed, so if we get a bad read,
            // we will just show a bad disc error in a hard loop
            XBOX_DisplayDiscErrorMsg();

            // Advance the local File position by the number of bytes read since it's 
            // actually being done to the file behind the scenes.
            pXBoxFile->mFilePos += pXBoxFile->mBytesRead;
#if defined( X_DEBUG ) && defined( LOG_FILE_ACCESS )
            if( pXBoxFile->mAccessLogID >= 0 )
            {
                s_AccessLog[pXBoxFile->mAccessLogID].ReadTime += x_GetTimeDiff( InTime, x_GetTime() );
            }
#endif
            if( gReadCallback ) gReadCallback();

            return 0;
        }
    }
    else  // read async
    {
        s32 SyncTaskID;
        s32 SyncBytesRead = 0;
        s32 SyncReadStatus;
        xbool ReadDone = FALSE;

        // Start the Async read.
        if( XBOX_ReadA( pBuffer, Bytes, pFile, 0, X_PRIORITY_TIMECRITICAL, SyncTaskID ) )
        {
            while( ReadDone == FALSE )
            {
                SyncReadStatus = x_freadastatus( SyncTaskID, SyncBytesRead );

                switch( SyncReadStatus )
                {
                    case X_STATUS_COMPLETE:
#ifdef MAD_XBOX_IO_DEBUG_TEXT
                        x_printf( "XBOX_READ: Completed Synchronous read. TaskID:%ld.  BytesRead:%ld, %s\n", SyncTaskID, SyncBytesRead, pXBoxFile->mFilename );
                        x_printf( "+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+\n" );
#endif
                        ReadDone = TRUE;
                        break;

                    case X_STATUS_ERROR:
#ifdef MAD_XBOX_IO_DEBUG_TEXT
                        x_printf( "XBOX_READ: Error Reading. TaskID%ld, %s\n\n\n\n\n", SyncTaskID, pXBoxFile->mFilename );
                        x_printf( "+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+\n" );
#endif
                        // Microsoft Standards require us to display a message when the disc is screwed, so if we get a bad read,
                        // we will just show a bad disc error in a hard loop
                        XBOX_DisplayDiscErrorMsg();

                        ReadDone = TRUE;
                        break;

                    case X_STATUS_NOTFOUND:
#ifdef MAD_XBOX_IO_DEBUG_TEXT
                        x_printf( "XBOX_READ: TaskID:%ld, %s was not found\n\n\n\n\n", SyncTaskID, pXBoxFile->mFilename );
                        x_printf( "+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+\n" );
#endif
                        ReadDone = TRUE;
                        break;
                }
            }
        }

#if defined( X_DEBUG ) && defined( LOG_FILE_ACCESS )
            if( pXBoxFile->mAccessLogID >= 0 )
            {
                s_AccessLog[pXBoxFile->mAccessLogID].ReadTime += x_GetTimeDiff( InTime, x_GetTime() );
            }
#endif
        if( gReadCallback ) gReadCallback();

        return SyncBytesRead;
    }
}

//==========================================================================
s32 XBOX_ReadA( void*   pBuffer,    // User buffer to store final data.
                s32     ReqBytes,   // Number of Bytes the user wishes to read.
                X_FILE* pFile,      // File to read from.
                s32     FileOffset, // Offset into the file to read from.
                s32     Priority,   // Async priority. (NOT USED AT THIS TIME)
                s32&    TaskID )
{
    ASSERT( (Priority >= X_PRIORITY_LOW) && (Priority <= X_PRIORITY_TIMECRITICAL) );
    s32 AllowedBytes;

    // Make sure that there are even bytes to be read.
    if( ReqBytes <= 0 )
    {
#ifdef MAD_XBOX_IO_DEBUG_TEXT
        x_printf( "********** XBOX_ReadA, trying to read %ld bytes from %s **********\n", ReqBytes, ((XBOX_FILE*)(pFile))->mFilename);
#endif
        ASSERT( 0 );
        TaskID = INVALID_TASK_ID;
        return 0;
    }
    else
    {
        XBOX_FILE*       pXBoxFile;
        XBOX_AsyncTask   NewAsyncTask;

        pXBoxFile = (XBOX_FILE*)pFile;

        ASSERT( pXBoxFile->mReadMode == XBOX_RMODE_ASYNC );

        // Make sure that you don't read past the end of the file.
        if( (FileOffset + ReqBytes) > pXBoxFile->mFilesize )
        {
            AllowedBytes = pXBoxFile->mFilesize - FileOffset;
#ifdef MAD_XBOX_IO_DEBUG_TEXT
            x_printf( "*** File I/O Asynchronous Read Error, reading past the end of the file %s ***\n", pXBoxFile->mFilename );
            x_printf( "*** Size of File:     %ld\n", pXBoxFile->mFilesize);
            x_printf( "*** Offset into File: %ld\n", FileOffset );
            x_printf( "*** Requested Bytes:  %ld\n", ReqBytes );
            x_printf( "*** Allowable Bytes:  %ld\n", AllowedBytes );
#endif
        }
        else
        {
            AllowedBytes = ReqBytes;
        }

        // Initialize the new Async task.
        //-------------------------------------------------------------------------------------------------------------------------------------
        TaskID = XBOX_MakeUniqueTaskID( );

        NewAsyncTask.mpFile        = pXBoxFile;
        NewAsyncTask.mpUserBuffer  = pBuffer;
        NewAsyncTask.mNBytesToRead = AllowedBytes;
        NewAsyncTask.mBytesRead    = 0;
        NewAsyncTask.mTaskID       = TaskID;
        NewAsyncTask.mReadOffset   = FileOffset;
        NewAsyncTask.mStatus       = X_STATUS_PENDING;

#ifdef MAD_XBOX_IO_DEBUG_TEXT
        x_printf( "================================================================================\n" );
        x_printf( "----> XBOX_READ: TaskID:%ld. New ASYNCHRONOUS Read, Reading %d Bytes from %s\n", TaskID, AllowedBytes, pXBoxFile->mFilename );
#endif

        // Insert the new read request into the system.
        XBOX_InsertReadTask( NewAsyncTask );

        // If the new task is the first task in the list, it needs to be started immediately,
        // because it is the active read task.
        //-------------------------------------------------------------------------------------------------------------------------------------
        if( sReadTasksList.GetHead()->mTaskID == NewAsyncTask.mTaskID )
        {

#ifdef MAD_XBOX_IO_DEBUG_TEXT
        x_printf( "..... XBOX_READ: TaskID:%ld. New read is only read in queue, starting immediately.\n", NewAsyncTask.mTaskID );
        x_printf( "================================================================================\n" );
#endif
            // This is the first read task, so go and read it.
            XBOX_StartNextReadTask( );
        }
        else
        {
#ifdef MAD_XBOX_IO_DEBUG_TEXT
            x_printf( "..... XBOX_READ: TaskID:%ld. New read is now in queue, waiting...\n", NewAsyncTask.mTaskID );
            x_printf( "================================================================================\n" );
#endif
        }

    }

    return 1;
}




//==========================================================================
s32 XBOX_ReadAStatus( s32 TaskID, s32& BytesRead )
{
    XBOX_AsyncTask*  pTask;
    s32 RValue;

    // Initialize this value to 0 each time the user requests the status.
    BytesRead = 0;

    // Have to put the thread to sleep inorder to see if the async read is complete.
    // Letting it sleep for 0 milliseconds should just return right away.
    SleepEx( 1, TRUE );

    // If the TASK ID -1 it's the special signal to indicate to get the status of the current active read.
    if( TaskID == -1 )
    {
        pTask = sReadTasksList.GetHead( );

        if( pTask )
        {
            // If there is an active task, then report the current values.
            if( pTask->IsValid( ) )
            {
                BytesRead = pTask->mBytesRead;
                return pTask->mStatus;
            }
        }
    }
    // If you are now looking for the default, try to match the task you want with the TASK id.
    else
    {
        pTask = sReadTasksList.Find( TaskID );

        // If it was found, update the bytes read, and return it's status.
        if( pTask )
        {
#ifdef MAD_XBOX_IO_DEBUG_TEXT
    x_printf( "!!!!! XBOX_READ: TaskID:%ld. XBOX_ReadAStatus: Not finished. File:%s\n", pTask->mTaskID, pTask->mpFile->mFilename );
#endif
            BytesRead = pTask->mBytesRead;
            return      pTask->mStatus; 
        }
        // If the task wasn't in the current reads, it should have been completed, try to find it in the current status list.
        else
        {
            pTask = sReadStatusList.Find( TaskID );

            if( pTask )
            {
                BytesRead = pTask->mBytesRead;
                RValue    = pTask->mStatus;

#ifdef MAD_XBOX_IO_DEBUG_TEXT
                x_printf( "!!!!! XBOX_READ: TaskID:%ld. XBOX_ReadAStatus: Read completed.  File:%s\n", pTask->mTaskID, pTask->mpFile->mFilename );
                x_printf( "================================================================================\n" );
#endif

#ifdef X_DEBUG
                // Just a spot to put a breakpoint.
                if( RValue == X_STATUS_ERROR )
                {
                    int Breakpoint = 0;
                    Breakpoint = 1;

#ifdef MAD_XBOX_IO_DEBUG_TEXT
                x_printf( "!!!!! XBOX_READ: TaskID:%ld. XBOX_ReadAStatus: STATUS ERROR.  File:%s\n", pTask->mTaskID, pTask->mpFile->mFilename );
                    x_printf( "-------------------------------------------------------------------------------------------------------\n" );
#endif
                }
                else if( RValue == X_STATUS_NOTFOUND ) 
                {
                    int Breakpoint = 0;
                    Breakpoint = 1;

#ifdef MAD_XBOX_IO_DEBUG_TEXT
                x_printf( "!!!!! XBOX_READ: TaskID:%ld. XBOX_ReadAStatus: STATUS NOT FOUND.  File:%s\n", pTask->mTaskID, pTask->mpFile->mFilename );
                    x_printf( "-------------------------------------------------------------------------------------------------------\n" );
#endif
                }
#endif
                sReadStatusList.Remove( TaskID );
                return RValue;
            }
        }
    }

    return X_STATUS_NOTFOUND;
}

//==========================================================================
s32 XBOX_ReadACancel( s32 TaskID )
{
	return X_STATUS_COMPLETE;
}

//==========================================================================
s32 XBOX_Write( X_FILE* pFile, byte* pBuffer, s32 Bytes )
{
    u32 bytesWritten = 0;
    XBOX_FILE* pXBoxFile = (XBOX_FILE*)pFile;

    WriteFile( pXBoxFile->mhFile, pBuffer, Bytes, &bytesWritten, NULL );
    return bytesWritten;
}

//==========================================================================
s32 XBOX_Seek( X_FILE* pFile, s32 Offset, s32 Origin )
{
#if defined( X_DEBUG ) && defined( LOG_FILE_ACCESS )
    f64         InTime = x_GetTime();
#endif
    XBOX_FILE* pXBoxFile = (XBOX_FILE*)pFile;
    s32     Result;
    s32     Where = 0;

    // Identify the offset method.
    switch( Origin )
    {
        case X_SEEK_SET:
            Where = FILE_BEGIN;
            pXBoxFile->mFilePos = Offset;
            break;

        case X_SEEK_CUR: 
            Where = FILE_CURRENT;
            pXBoxFile->mFilePos += Offset;
            break;

        case X_SEEK_END:
            Where = FILE_END;
            pXBoxFile->mFilePos = pXBoxFile->mFilesize + Offset;
            break;

        default:
            ASSERTS( 0, "Invalid Origin used for XBOX_Seek" );
            return -1;
            break;
    }

    // Make sure that the file pos doesn't go off the end of the file in either extreme.
    if( pXBoxFile->mFilePos < 0 )
        pXBoxFile->mFilePos = 0;
    else if( pXBoxFile->mFilePos > pXBoxFile->mFilesize )
        pXBoxFile->mFilePos = pXBoxFile->mFilesize;

    // Has this file been open for reading?
    if( pXBoxFile->mOpenMode & GENERIC_READ )
    {
        if( pXBoxFile->mReadMode == XBOX_RMODE_ASYNC )
        {
            s32     SystemSeekPosition;

            SystemSeekPosition = pXBoxFile->GetSectorAddress( );
            ASSERT( !(SystemSeekPosition % pXBoxFile->mDriveSectorSize) );

            Result = SetFilePointer( pXBoxFile->mhFile, SystemSeekPosition, 0, FILE_BEGIN );
        }
        else
            Result = SetFilePointer( pXBoxFile->mhFile, Offset, 0, Where );
    }
    // Must be for writing then.
    else
    {
        // When writing to a file, let the user do what ever they want.
        Result = SetFilePointer( pXBoxFile->mhFile, Offset, 0, Where );
    }

    // Test for an error.
    if( Result == INVALID_SET_FILE_POINTER )
    {
        // Display the error message.
        XBOX_PrintLastReadError( pXBoxFile );
        ASSERTS( 0, "XBOX_Seek failed" );
#if defined( X_DEBUG ) && defined( LOG_FILE_ACCESS )
        if( pXBoxFile->mAccessLogID >= 0 )
        {
            s_AccessLog[pXBoxFile->mAccessLogID].SeekTime += x_GetTimeDiff( InTime, x_GetTime() );
        }
#endif
        return(  -1 );
    }
    // No problems, return 0.
    else
    {
#if defined( X_DEBUG ) && defined( LOG_FILE_ACCESS )
        if( pXBoxFile->mAccessLogID >= 0 )
        {
            s_AccessLog[pXBoxFile->mAccessLogID].SeekTime += x_GetTimeDiff( InTime, x_GetTime() );
        }
#endif
        return( 0 );
    }
}

//==========================================================================
s32 XBOX_Tell( X_FILE* pFile )
{
    XBOX_FILE* pXBoxFile = (XBOX_FILE*)pFile;
    return(pXBoxFile->mFilePos);
}

//==========================================================================
xbool XBOX_EOF( X_FILE* pFile )
{
    XBOX_FILE* pXBoxFile = (XBOX_FILE*)pFile;
    return ( pXBoxFile->mFilePos == pXBoxFile->mFilesize );
}


//=====================================================================================================================================
xbool XBOX_ReadAsynchronous( void )
{
    s32             ReturnValue;
    s32             NumberOfSectorsToRead;  // Number of sectors of the file to read.
    s32             RemainingBytesToRead;   // Number of user bytes required to read in order to complete the read.
    XBOX_AsyncTask* pReadTask;

    // Always use the highest priority task.
    pReadTask = sReadTasksList.GetHead( );

    // Determine how many more bytes of data must be read in order to complete the user's read request.
    RemainingBytesToRead = pReadTask->mNBytesToRead - pReadTask->mBytesRead;
    ASSERT( RemainingBytesToRead > 0 );

    // If this isn't the cache file, reset the cache, and make it this file.
    if( s_XBOXCache.mCacheFile != pReadTask->mpFile )
    {
#ifdef MAD_XBOX_IO_DEBUG_TEXT
//        x_printf( "***** XBOX READ: TaskID:%ld. Resetting File Cache. %s\n", pReadTask->mTaskID, pXBoxFile->mFilename );
#endif
        s_XBOXCache.Reset( );
        s_XBOXCache.mCacheFile = pReadTask->mpFile;
        s_XBOXCache.mMaxSectorsInCache = XBOX_READ_CACHE_SIZE / s_XBOXCache.mCacheFile->mDriveSectorSize;
    }
    // this file was the cache file already.
    else
    {
        s32 AddressOfFirstUserByteToRead;
        s32 AddressOfLastUserByteToRead;
        s32 AddressOfFirstUserDataInCache;
        s32 AddressOfLastUserDataInCache;

        // Determine the boundaries of the read.
        AddressOfFirstUserByteToRead = pReadTask->mpFile->mFilePos;
        AddressOfLastUserByteToRead  = pReadTask->mpFile->mFilePos + pReadTask->mNBytesToRead;

        // Get the extents of the user data within the cache.
        AddressOfFirstUserDataInCache = s_XBOXCache.mStartAddr + s_XBOXCache.mUserReadPos;
        AddressOfLastUserDataInCache  = s_XBOXCache.mEndAddr;

        // Is the requested user data already read and within the cache?
        if( AddressOfFirstUserByteToRead >= AddressOfFirstUserDataInCache &&
            AddressOfLastUserByteToRead  <= AddressOfLastUserDataInCache )  
        {
            byte* pUserBufferAddress;
            byte* pCacheReadAddress;

            // Update the UserReadPosition in the Cache.
            s_XBOXCache.mUserReadPos = pReadTask->mpFile->mFilePos - s_XBOXCache.mStartAddr;

            // Determine the address into the user buffer where the user data should be copied to.
            // Also find the address of the user data from within the cache.
            pUserBufferAddress = &((byte*)pReadTask->mpUserBuffer)[pReadTask->mBytesRead];
            pCacheReadAddress  = &s_XBOXCache.mBuffer[s_XBOXCache.mUserReadPos];

            // Copy the user data out of the cache.
            x_memcpy( pUserBufferAddress, pCacheReadAddress, pReadTask->mNBytesToRead );

            // Update the user buffers read count.
            pReadTask->mBytesRead += pReadTask->mNBytesToRead;

            // Update the cache to identify what new data has been read from it.
            s_XBOXCache.mUserReadPos += pReadTask->mNBytesToRead;

            // Update the file position now as well.
            pReadTask->mpFile->mFilePos += pReadTask->mNBytesToRead;

            // Make sure that the cache is filled by who you think it should be.
            ASSERT( s_XBOXCache.mCacheFile == pReadTask->mpFile );

#ifdef MAD_XBOX_IO_DEBUG_TEXT
//            x_printf( "..... XBOX_READ: TaskID:%ld. XBOX_ReadAsynchronous Read from Cache (Cache filled by:%s). %ld bytes. %s\n", pReadTask->mTaskID, s_XBOXCache.mCacheFile->mFilename, RemainingBytesToRead, pXBoxFile->mFilename );
#endif

            // Is the read completed? It damn well better be.
            if( pReadTask->mBytesRead == pReadTask->mNBytesToRead )
            {
                // Update the status of this particular read task.
                XBOX_CompleteReadTask( pReadTask, X_STATUS_COMPLETE, XBOX_ASYNC_FINISHED );
                return FALSE;
            }
            else
            {
                // If this asserts, there's something royally messy going on.
                ASSERT( 0 );
            }
        }
    }

    // Make sure that the cache is filled by who you think it should be.
    ASSERT( s_XBOXCache.mCacheFile == pReadTask->mpFile );

    // Determine how many bytes must be read in order to appease the XBOX requirements. (Sector aligned)
    s_XBOXCache.mStartAddr         = pReadTask->mpFile->GetSectorAddress( );
    s_XBOXCache.mUserReadPos       = pReadTask->mpFile->mFilePos - s_XBOXCache.mStartAddr;
    NumberOfSectorsToRead          = MIN( s_XBOXCache.mMaxSectorsInCache, (1 + MAX( ((RemainingBytesToRead + s_XBOXCache.mUserReadPos) / pReadTask->mpFile->mDriveSectorSize), (pReadTask->mpFile->mFilesize / pReadTask->mpFile->mDriveSectorSize))) );
    s_XBOXCache.mExpectedBytesRead = NumberOfSectorsToRead * pReadTask->mpFile->mDriveSectorSize;
    s_XBOXCache.mEndAddr           = s_XBOXCache.mStartAddr + s_XBOXCache.mExpectedBytesRead;

    // Well, we know how much needs to be read, so go and read it.
    pReadTask->mpFile->mXBOXAsyncInfo.Offset     = s_XBOXCache.mStartAddr;
    pReadTask->mpFile->mXBOXAsyncInfo.OffsetHigh = 0;

    ReturnValue = ReadFileEx( s_XBOXCache.mCacheFile->mhFile,
                              s_XBOXCache.mBuffer,
                              s_XBOXCache.mExpectedBytesRead,
                              &pReadTask->mpFile->mXBOXAsyncInfo,
                              XBOX_ReadAsyncCallback );
    if( ReturnValue )
    {
#ifdef MAD_XBOX_IO_DEBUG_TEXT
//    x_printf( "..... XBOX_READ: TaskID:%ld. Filling Cache (Cache file is:%s). %ld bytes, %s, %d Bytes Remaining\n", pReadTask->mTaskID, s_XBOXCache.mCacheFile->mFilename, s_XBOXCache.mExpectedBytesRead, pXBoxFile->mFilename, RemainingBytesToRead );
#endif
        return TRUE;
    }
    else
    {
        XBOX_CompleteReadTask( pReadTask, X_STATUS_ERROR, XBOX_ASYNC_FAILED );
        return FALSE;
    }
}

//=====================================================================================================================================
void CALLBACK XBOX_ReadAsyncCallback( DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped )
{
    XBOX_AsyncTask* pReadTask;
    void*       pUserBufferAddress;
    void*       pCacheReadAddress;
    s32         UserBytesRead;
    s32         BytesRequiredToCompleteTheRead;
    xbool       mbEndOfFileReached;

    pReadTask = sReadTasksList.GetHead( );

    // Was there an error during the read?
    if( dwErrorCode != 0 )
    {
        XBOX_CompleteReadTask( pReadTask, X_STATUS_ERROR, XBOX_ASYNC_FAILED );
        return;
    }

    BytesRequiredToCompleteTheRead = pReadTask->mNBytesToRead - pReadTask->mBytesRead;

    // Determine if the end of the file was reached.
    // If it was, then fudge the Expected bytes read to the size of the file.
    mbEndOfFileReached = (pReadTask->mpFile->mFilePos + (s32)dwNumberOfBytesTransfered) >= pReadTask->mpFile->mFilesize;
    if( mbEndOfFileReached )
    {
        s_XBOXCache.mExpectedBytesRead = dwNumberOfBytesTransfered;
    }

    // Update the end of the usable data in the cache.
    s_XBOXCache.mEndAddr = s_XBOXCache.mStartAddr + dwNumberOfBytesTransfered;

    // First, test to see the correct number of bytes were read into the cache.
    if( s_XBOXCache.mExpectedBytesRead == dwNumberOfBytesTransfered )
    {
        // Make sure that the cache is filled by who you think it should be.
        ASSERT( s_XBOXCache.mCacheFile == pReadTask->mpFile );

        // Determine the number of bytes stored in the cache that are meant for the user.
        UserBytesRead = MIN( BytesRequiredToCompleteTheRead, (s32)dwNumberOfBytesTransfered - s_XBOXCache.mUserReadPos );

        // Update the FilePosition values;
        pReadTask->mpFile->mFilePos += UserBytesRead;
        if( pReadTask->mpFile->mFilePos > pReadTask->mpFile->mFilesize )
        {
            pReadTask->mpFile->mFilePos = pReadTask->mpFile->mFilesize;
            ASSERT( 0 );
        }

        // Determine where to store the user data.
        pUserBufferAddress = &((byte*)pReadTask->mpUserBuffer)[pReadTask->mBytesRead];

        // Locate where that user data begins in the cache.
        pCacheReadAddress = &s_XBOXCache.mBuffer[s_XBOXCache.mUserReadPos];

        // Copy the user data out of the cache.
        x_memcpy( pUserBufferAddress, pCacheReadAddress, UserBytesRead );

        // Now update the user required data.
        pReadTask->mBytesRead += UserBytesRead;

        // Update the user read position in the cache to identify the start of valid user data.
        s_XBOXCache.mUserReadPos += UserBytesRead;

#ifdef MAD_XBOX_IO_DEBUG_TEXT
//        x_printf( "..... XBOX_READ: TaskID:%ld. XBOX_ReadAsyncCallback Read from Cache (Cache file is:%s). %ld bytes. %s\n", pReadTask->mTaskID, s_XBOXCache.mCacheFile->mFilename, UserBytesRead, pReadTask->mpFile->mFilename );
#endif

        // Is the read completed?
        if( pReadTask->mBytesRead == pReadTask->mNBytesToRead )
        {
            XBOX_CompleteReadTask( pReadTask, X_STATUS_COMPLETE, XBOX_ASYNC_FINISHED );
        }
        else
        {
            // If there are still bytes to be read, and you haven't reached the end of the file,
            // continue to read from the file until it's finished.
            if( mbEndOfFileReached == FALSE )
            {
                XBOX_ReadAsynchronous( );
            }
            // If the end of the file was reached, and you are not done, then it's broken.
            else
            {
                XBOX_CompleteReadTask( pReadTask, X_STATUS_ERROR, XBOX_ASYNC_FAILED );
            }
        }
    }
    else
    {
        XBOX_CompleteReadTask( pReadTask, X_STATUS_ERROR, XBOX_ASYNC_FAILED );
    }
}




//=====================================================================================================================================
void XBOX_PrintLastReadError( XBOX_FILE* pXBoxFile )
{
#ifdef MAD_XBOX_IO_DEBUG_TEXT

    s32   LastError           = GetLastError( );
    char* UnknownError        = "Unknown";
    char* ErrorDescriptions[] = {
                                  "Success",
                                  "Invalid Function",
                                  "File not found",
                                  "File Path not found", 
                                  "Too many files open", 
                                  "Access Denied", 
                                  "Invalid File Handle", 
                                };

    char* CurrentErrorDescriptions;
    
    // Test to see if we can describe the error.
    if( LastError >= ERROR_SUCCESS &&  LastError <= ERROR_INVALID_HANDLE )
    {
        CurrentErrorDescriptions = ErrorDescriptions[LastError];
    }
    else
    {
        CurrentErrorDescriptions = UnknownError;
    }

    x_printf( "*** File I/O Error *** Error:%ld - \"%s\", working with file %s.\n", LastError, CurrentErrorDescriptions, pXBoxFile->mFilename );
#endif
}


//==========================================================================
void XBOX_DVD_InitModule( void )
{
    s32 fileIndex;

    x_SetIOHandlers( XBOX_Open,
                     XBOX_Close,
                     XBOX_Read,
                     XBOX_ReadA,
                     XBOX_ReadAStatus,
                     XBOX_ReadACancel,
                     XBOX_Write,
                     XBOX_Seek,
                     XBOX_Tell,
                     XBOX_EOF );

    // Initialize the file handlers
    for( fileIndex = 0; fileIndex < MAX_XBOX_FILES; fileIndex++ )
    {
        s_XboxFiles[fileIndex].Reset();
    }

    // Initialize the Async variables.
    s_XboxFilesOpen     = 0;                    // No files are open.

    // Reset the read cache.
    s_XBOXCache.Reset( );

    XSetFileCacheSize( XBOX_SYSREAD_CACHE_SIZE ); // change the default XBOX cache size (64KB)
}

//=====================================================================================================================================
void XBOX_DVD_KillModule( void )
{
    x_SetIOHandlers( NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL );
}

//==========================================================================

void XBOX_DVD_DisplayAccessLog( xbool ResetTimes )
{
#if defined( X_DEBUG ) && defined( LOG_FILE_ACCESS )
    s32 i;
    f64 TotalOpenTime = 0.0f;
    f64 TotalSeekTime = 0.0f;
    f64 TotalReadTime = 0.0f;
    f64 TotalTotalTime = 0.0f;
    x_printf( "\n--------------------------------[FILE ACCESS LOG]------------------------------------\n" );
    x_printf( "| Open/Clos | Seek Time | Read Time | Total     | Open Stat | File Name\n" );
    x_printf( "-------------------------------------------------------------------------------------\n" );
    for( i = 0; i < s_HighestLogID; i++ )
    {
        f64 TotalTime = s_AccessLog[i].OpenTime + s_AccessLog[i].SeekTime + s_AccessLog[i].ReadTime;
        x_printf(   "| %9.4f | %9.4f | %9.4f | %9.4f | %d | %s\n",
                    s_AccessLog[i].OpenTime,
                    s_AccessLog[i].SeekTime,
                    s_AccessLog[i].ReadTime,
                    TotalTime,
                    s_AccessLog[i].OpenStatus,
                    s_AccessLog[i].Name );
        TotalOpenTime += s_AccessLog[i].OpenTime;
        TotalSeekTime += s_AccessLog[i].SeekTime;
        TotalReadTime += s_AccessLog[i].ReadTime;
        TotalTotalTime += TotalTime;
        s_AccessLog[i].OpenTime = 0.0f;
        s_AccessLog[i].ReadTime = 0.0f;
        s_AccessLog[i].SeekTime = 0.0f;
    }
    x_printf( "\n---------------------------------[    TOTALS    ]------------------------------------\n" );
    x_printf( "| %9.4f | %9.4f | %9.4f | %9.4f | %d\n", TotalOpenTime, TotalSeekTime, TotalReadTime, TotalTotalTime, i );
    x_printf( "-------------------------------------------------------------------------------------\n" );
#else
    x_printf( "XBOX_DEV: Access Log is not enabled!\n" );
#endif
}

//==========================================================================

void XBOX_DVD_SetReadCallback( QSyncReadCallback pFunction )
{
    gReadCallback = pFunction;
}

//==========================================================================
