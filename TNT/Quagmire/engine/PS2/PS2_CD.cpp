////////////////////////////////////////////////////////////////////////////
//
// PS2_CD.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include <libcdvd.h>
#include <sifrpc.h>
#include <sifdev.h>
#include <eekernel.h>

#include "x_memory.hpp"
#include "x_plus.hpp"
#include "x_debug.hpp"

#include "PS2_CD.hpp"


////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////
#if defined( TARGET_PS2_DVD )

#if defined( X_DEBUG )
    // Debug compile-time switches
    #define PS2CD_DEBUG
    #define MAD_PS2CD_DEBUG
#endif


#define SYSTEM_IMAGE_FILE       "cdrom0:\\SYSTEM\\DNAS280.IMG;1"

// size of PS2 CD sector, in bytes
#define PS2CD_SECTOR_SIZE       2048

// synchronous and async read buffer sizes, in number of sectors
#define READ_CACHE_SIZE         24
#define ASYNC_BUF_SIZE          16

// sceCdSync parameter values
#define PS2CD_BLOCK             0
#define PS2CD_NOBLOCK           1

// sceCdSync return values
#define PS2CD_CDSYNC_COMPLETE   0
#define PS2CD_CDSYNC_BUSY       1

//
#define MAX_PS2_FILES           8
#define MAX_ASYNC_READ_TASKS    12
#define INVALID_TASK_ID         (-99)
#define INVALID_SECTOR          (-1)


// return values for PS2CD_GetMediaType
#define PS2CD_MEDIA_ILLEGAL     SCECdIllgalMedia
#define PS2CD_MEDIA_PS2_DVD     SCECdPS2DVD
#define PS2CD_MEDIA_PS2_CD      SCECdPS2CD
#define PS2CD_MEDIA_PS2_CDDA    SCECdPS2CDDA
#define PS2CD_MEDIA_PS_CD       SCECdPSCD
#define PS2CD_MEDIA_PS_CDDA     SCECdPSCDDA
#define PS2CD_MEDIA_DVD         SCECdDVDV
#define PS2CD_MEDIA_CDDA        SCECdCDDA
#define PS2CD_MEDIA_NONE        SCECdNODISC


//--------------------------------------------------------------------------

#if defined( PS2CD_DEBUG ) && defined( X_DEBUG )
    #define DBG_EXP( exp )    exp
#else
    #define DBG_EXP( exp )
#endif

#if defined( MAD_PS2CD_DEBUG ) && defined( X_DEBUG )
    #define MAD_DBG_EXP( exp )  exp
#else
    #define MAD_DBG_EXP( exp )
#endif


////////////////////////////////////////////////////////////////////////////
// STRUCTURES
////////////////////////////////////////////////////////////////////////////

struct PS2_FILE
{
    sceCdlFILE  m_FileInfo;         // PS2 file info structure
    xbool       m_bOpen;            // is the file open
    s32         m_Filepos;          // current position of file "pointer"(synchronous reads only)
    s32         m_Filesize;         // size of file

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
    s32             m_NLeadingBytes;    // number of leading bytes requested data is from aligned sector data
    s32             m_NTrailingBytes;   // number of trailing bytes at end of req. data from last sector
    s32             m_BytesToRead;      // number of bytes remaining to read for this task
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

//--------------------------------------------------------------------------

struct PS2_ReadCache
{
    s32         m_Start;            // sector where cached data begins
    s32         m_End;              // sector where cached data ends
    byte*       m_CacheBuffer;      // pointer to cache buffer

    void Reset( void );
};

//--------------------------------------------------------------------------

struct PS2_ReadInfo
{
    s32       m_LSN;
    s32       m_FileOffset;
    s32       m_FileSize;
    byte*     m_pDestBuffer;
    s32       m_BytesToRead;
    s32       m_TotalBytesRead;
    s32       m_FirstSector;
    s32       m_LastSector;
    s32       m_NSectors;
    s32       m_NLeadingBytes;
    s32       m_NTrailingBytes;

    void CalcReadData    ( s32 CacheSize );
    void UpdateReadAmount( s32 CopyLength );
    void ReadCachedData  ( PS2_ReadCache& ReadCache );
};


////////////////////////////////////////////////////////////////////////////
// LOCAL STATIC VARIABLES
////////////////////////////////////////////////////////////////////////////

// File IO variables
static PS2_FILE          s_PS2Files[ MAX_PS2_FILES ];
static PS2_AsyncTaskList s_PS2ReadTasksList;
static PS2_AsyncTaskList s_PS2ReadStatusList;
static PS2_ReadCache     s_PS2CDReadCache[2];
static PS2_ReadCache     s_PS2CDAsyncCache;

static s32               s_LastKnownMediaType = PS2CD_MEDIA_ILLEGAL;
static char*             s_pSystemImageFile   = SYSTEM_IMAGE_FILE;
static xbool             s_bInSyncRead        = FALSE;
static QSyncReadCallback s_SyncReadCallback   = NULL;

// Cache buffers for synchronous and asynchronous reads
static byte              s_PS2CDReadBufferA[ READ_CACHE_SIZE * PS2CD_SECTOR_SIZE ] __attribute__ ( (aligned(64)) );
static byte              s_PS2CDReadBufferB[ READ_CACHE_SIZE * PS2CD_SECTOR_SIZE ] __attribute__ ( (aligned(64)) );
static byte              s_PS2CDAsyncBuffer[ ASYNC_BUF_SIZE  * PS2CD_SECTOR_SIZE ] __attribute__ ( (aligned(64)) );


////////////////////////////////////////////////////////////////////////////
// LOCAL PROTOTYPES
////////////////////////////////////////////////////////////////////////////

static s32     PS2CD_MakeUniqueTaskID ( void );
static void    PS2CD_InsertReadTask   ( PS2_AsyncTask& NewTask );
static void    PS2CD_StartNextReadTask( void );
static void    PS2CD_CompleteReadTask ( PS2_AsyncTask* pReadTask, s32 ReadTaskStatus );
static xbool   PS2CD_ReadAsync        ( PS2_AsyncTask* pReadTask );

static xbool   PS2CD_DoesFileExist    ( const char* pFilename, sceCdlFILE* pFileInfo );

static X_FILE* PS2CD_Open       ( const char* pFilename, const char* pMode );
static void    PS2CD_Close      ( X_FILE* pXFile );
static s32     PS2CD_Read       ( X_FILE* pXFile, byte* pBuffer, s32 Bytes );
static s32     PS2CD_ReadA      ( void* pBuffer, s32 Bytes, X_FILE* pFile, s32 FileOffset, s32 Priority, s32& TaskID );
static s32     PS2CD_ReadAStatus( s32 TaskID, s32& BytesRead );
static s32     PS2CD_ReadACancel( s32 TaskID );
static s32     PS2CD_Write      ( X_FILE* pXFile, byte* pBuffer, s32 Bytes );
static s32     PS2CD_Seek       ( X_FILE* pXFile, s32 Offset, s32 Origin );
static s32     PS2CD_Tell       ( X_FILE* pXFile );
static xbool   PS2CD_EOF        ( X_FILE* pXFile );

#ifdef PS2CD_DEBUG
    static void PS2CD_ExplainError( s32 Err );
#else
    #define PS2CD_ExplainError( Err )
#endif

void  PS2CD_PrepImageFile ( char* pSystemImageFile );
s32   PS2CD_GetMediaType  ( void );
xbool PS2CD_IsMediaUseable( void );
void  PS2CD_Eject         ( void );
void  PS2CD_Standby       ( void );
void  PS2CD_Stop          ( void );


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// LOCAL STRUCTURES FUNCTION IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void PS2_FILE::Reset( void )
{
    x_memset( &m_FileInfo, 0, sizeof(sceCdlFILE) );

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
    m_NLeadingBytes  = 0;
    m_NTrailingBytes = 0;
    m_BytesToRead    = 0;
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

//==========================================================================

void PS2_ReadCache::Reset( void )
{
    m_Start = INVALID_SECTOR;
    m_End   = INVALID_SECTOR;

    // cache buffer is not reset - don't want to lose pointer to it, since
    // this function is only used to "clear" the cache of file data
}

//==========================================================================

inline void PS2_ReadInfo::CalcReadData( s32 CacheSize )
{
    // Since reads are done in terms of sectors(2048 bytes), determine the first sector
    // the file pointer is in, and the last sector that the data to be read in is at.
    // The (BytesToRead-1) is there so that an extra sector is not read in(i.e. BytesToRead
    // is 2048 and the file ptr is at zero- ((0 + 2048 - 1) / 2048) = 0 ) when calculating
    // the number of sectors.
    m_FirstSector = m_LSN + (m_FileOffset / PS2CD_SECTOR_SIZE);
    m_LastSector  = m_LSN + ((m_FileOffset + (m_BytesToRead-1)) / PS2CD_SECTOR_SIZE);
    m_NSectors    = m_LastSector - m_FirstSector + 1;

    // Also find the number of extra bytes read in before the file pointer, since
    // the reads must be aligned to sectors
    m_NLeadingBytes  = m_FileOffset % PS2CD_SECTOR_SIZE;
    m_NTrailingBytes = (m_FileOffset + m_BytesToRead) % PS2CD_SECTOR_SIZE;

    // Adjust the number of sectors to read so it fits in the cache
    m_NSectors = MIN( CacheSize, m_NSectors );
    m_NSectors = MAX( CacheSize, m_NSectors );
}

//--------------------------------------------------------------------------

inline void PS2_ReadInfo::UpdateReadAmount( s32 CopyLength )
{
    // Advance pointers and update bytes read
    m_BytesToRead    -= CopyLength;
    m_TotalBytesRead += CopyLength;
    m_pDestBuffer    += CopyLength;
    m_FileOffset     += CopyLength;
}

//--------------------------------------------------------------------------

void PS2_ReadInfo::ReadCachedData( PS2_ReadCache& ReadCache )
{
    byte* pCacheBuffer;
    s32   CopyLength;
    s32   CachePosStart;
    s32   CachePosEnd;
    s32   FilePosSector;

    if( m_BytesToRead <= 0 )
        return;

    // Calc the disk sector the current file pointer is in
    FilePosSector = m_LSN + (m_FileOffset / PS2CD_SECTOR_SIZE);

    // These checks are to ensure that the sector the current file position is in lies within
    // the cached range.  Even if some part of the data requested is in the cache it will be
    // ignored to keep things simple(i.e. only do forward reads).  These two checks also prevent
    // the following calculations from overflow, since the start of the cache might be less than
    // the start of the file on a DVD- the difference would be less than zero so we need to use
    // s32's.  Since the DVD size is > 4GB, and s32 could not hold the difference between two
    // absolute byte locations that are > ~2GB apart.
    if( ReadCache.m_End < FilePosSector )
        return;
    if( ReadCache.m_Start > FilePosSector )
        return;

    // The cache start and end are in terms of sectors, convert them to relative file position values
    CachePosStart = (ReadCache.m_Start - m_LSN) * PS2CD_SECTOR_SIZE;
    CachePosEnd   = CachePosStart + ((ReadCache.m_End - ReadCache.m_Start) * PS2CD_SECTOR_SIZE);

    // Ensure that the file position really is within the cached range
    if( (m_FileOffset < CachePosStart) || (m_FileOffset >= CachePosEnd) )
        return;

    // If requested bytes to read go beyond the end of the cache,
    // shorten cache copy length to only what's in the cache
    if( (m_FileOffset + m_BytesToRead) > CachePosEnd )
        CopyLength = CachePosEnd - m_FileOffset;
    else
        CopyLength = m_BytesToRead;

    DBG_EXP( x_printf( "...PS2CD ReadCachedData: FPtr:%ld, %ld bytes\n", m_FileOffset, CopyLength ) );

    // Data we need is in cache, simply copy data to user buffer
    pCacheBuffer = &ReadCache.m_CacheBuffer[ m_FileOffset - CachePosStart ];
    x_memcpy( m_pDestBuffer, pCacheBuffer, CopyLength );

    // update current file position and pointers
    UpdateReadAmount( CopyLength );

    // This should never happen, but just to be sure...
    if( m_FileOffset > m_FileSize )
        m_FileOffset = m_FileSize;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// PS2 STDIO HANDLERS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

static xbool PS2CD_DoesFileExist( const char* pFilename, sceCdlFILE* pFileInfo )
{
    extern xbool gPS2CDUtilReady;
    extern s32   PS2CD_SearchFile( sceCdlFILE* pFileInfo, const char* pFilename );

    s32 RetVal;
    s32 SearchTries = 5;

    if( !gPS2CDUtilReady )
        sceCdDiskReady( PS2CD_BLOCK );

    while( SearchTries )
    {
        // Use the better search function if possible
        if( gPS2CDUtilReady )
            RetVal = PS2CD_SearchFile( pFileInfo, pFilename );
        else
            RetVal = sceCdSearchFile( pFileInfo, pFilename );

        if( RetVal )
            return TRUE;

        SearchTries--;
    }

    return FALSE;
}

//==========================================================================

static X_FILE* PS2CD_Open( const char* pFilename, const char* pMode )
{
    PS2_FILE* pFile  = NULL;
    xbool     Read   = FALSE;
    xbool     Write  = FALSE;
    xbool     Append = FALSE;
    s32       i;
    char      FinalFilename[300];

    ASSERT( pFilename != NULL );
    ASSERT( pMode != NULL );

    DBG_EXP( x_printf( "...PS2CD Open: %s\n", pFilename ) );

    // Pick through the Mode characters.
    while( *pMode != '\0' )
    {
        if( (*pMode == 'r') || (*pMode == 'R') )  Read   = TRUE;
        if( (*pMode == 'w') || (*pMode == 'W') )  Write  = TRUE;
        if( (*pMode == 'a') || (*pMode == 'A') )  Append = TRUE;
        ++pMode;
    }

    // PS2 DVD cannot write
    //ASSERT( (Write == FALSE) && (Append == FALSE) );
    if( Write || Append )
    {
        DBG_EXP( x_printf( "...PS2CD WARNING: CD doesn't support writing, file: %s\n", pFilename ) );

        return (X_FILE*)(NULL);
    }

    if( !Read )
    {
        DBG_EXP( x_printf( "...PS2CD WARNING: DVD can only read files, file: %s\n", pFilename ) );

        return (X_FILE*)(NULL);
    }

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
    ASSERTS( pFile != NULL, "PS2CD: Too many open files" );
    if( pFile == NULL )
        return NULL;

    // Reset file structure
    pFile->Reset();

    // Make filename all uppercase with BACKSLASHES, not FORWARD slashes
    // Prepend backslash to name, and end it with ;1
    // Also remove any leading forward or backward slashes
    while( *pFilename != '\0' )
    {
        if( (*pFilename != '/') && (*pFilename != '\\') )
            break;

        pFilename++;
    }

    FinalFilename[0] = '\\';
    x_strcpy( &FinalFilename[1], pFilename );
    x_strtoupper( &FinalFilename[1] );

    for( i = 0; FinalFilename[i] != '\0'; i++ )
    {
        if( FinalFilename[i] == '/' )
            FinalFilename[i] = '\\';
    }

    x_strcpy( &FinalFilename[ x_strlen(FinalFilename) ], ";1" );


    // Make sure the file exists on disk
    if( !PS2CD_DoesFileExist( FinalFilename, &pFile->m_FileInfo ) )
    {
        DBG_EXP( x_printf( "...PS2CD WARNING: Couldn't find file %s\n", FinalFilename ) );

        return (X_FILE*)NULL;
    }

    // File exists
    pFile->m_bOpen    = TRUE;
    pFile->m_Filesize = pFile->m_FileInfo.size;

    #ifdef X_DEBUG
        x_strncpy( pFile->m_Filename, FinalFilename, 256 );
    #endif

    return (X_FILE*)pFile;
}

//==========================================================================

static void PS2CD_Close( X_FILE* pXFile )
{
    //ASSERT( pXFile != NULL );
    if( pXFile == NULL )
        return;

    PS2_FILE *pFile = (PS2_FILE*)pXFile;

    // Make sure file is open
    ASSERT( pFile->m_bOpen );

    // Make sure file is not in use by an async read task
    #ifdef X_DEBUG
        PS2_AsyncTask* pTask = s_PS2ReadTasksList.GetHead();
        while( pTask != NULL )
        {
            ASSERTS( pTask->m_pFile != pFile, "Tried to close file used by async task" );
            pTask = pTask->m_pNext;
        }
    #endif

    // reset file structure
    pFile->Reset();
}

//==========================================================================

static s32 PS2CD_Read( X_FILE* pXFile, byte* pUserBuffer, s32 BytesRequested )
{
    s32 RBuf;
    s32 WBuf;
    s32 CurSector;
    s32 CopyLength;
    s32 ReadStatus;
    s32 SCEStatus;

    PS2_ReadInfo RI;
    PS2_FILE*    pPS2File;
    sceCdRMode   PS2RMode;

    // Do some sanity checks
    ASSERT( pUserBuffer != NULL );
    ASSERT( pXFile != NULL );

    ASSERT( s_bInSyncRead == FALSE );

    // Check to make sure another read isn't called from sync-read callback
    if( s_bInSyncRead )
        return 0;

    if( BytesRequested <= 0 )
        return 0;
    if( (pXFile == NULL) || (pUserBuffer == NULL ) )
        return 0;

    pPS2File = (PS2_FILE*)pXFile;

    ASSERT( pPS2File->m_bOpen );

    // If file pointer is at the end of the file, nothing to read
    if( pPS2File->m_Filepos >= pPS2File->m_Filesize )
        return 0;

    // Setup variables for use
    RI.m_LSN            = pPS2File->m_FileInfo.lsn;
    RI.m_FileOffset     = pPS2File->m_Filepos;
    RI.m_FileSize       = pPS2File->m_Filesize;
    RI.m_pDestBuffer    = pUserBuffer;
    RI.m_BytesToRead    = BytesRequested;
    RI.m_TotalBytesRead = 0;

    // If number of bytes requested to read goes beyond the end of the file,
    // shorten length to only reach the end
    if( (pPS2File->m_Filepos + RI.m_BytesToRead) > pPS2File->m_Filesize )
        RI.m_BytesToRead = pPS2File->m_Filesize - pPS2File->m_Filepos;

    // Read any cached data first- this is done twice on the first buffer,
    // since the order of read sectors for the two buffers isn't known
    RI.ReadCachedData( s_PS2CDReadCache[0] );
    RI.ReadCachedData( s_PS2CDReadCache[1] );
    RI.ReadCachedData( s_PS2CDReadCache[0] );

    // Save updated file position
    pPS2File->m_Filepos = RI.m_FileOffset;

    // if all data requested was in cache, read is done and return total bytes read
    if( RI.m_BytesToRead <= 0 )
    {
        // run user-defined callback if provided
        if( s_SyncReadCallback != NULL )
            s_SyncReadCallback();

        return RI.m_TotalBytesRead;
    }

    // Update any async loads, but don't start another until this sync read is done
    if( s_PS2ReadTasksList.GetHead() )
        PS2CD_UpdateAsyncReads( FALSE );

    // "Lock" the sync-read so a user callback can't do another read
    s_bInSyncRead = TRUE;


    // Setup the PS2 structure for read mode
    PS2RMode.trycount    = 5;                       // Number of retries
    PS2RMode.spindlctrl  = SCECdSpinNom;            // Max rotational velocity, if errors then its reduced
    PS2RMode.datapattern = SCECdSecS2048;           // 2048 byte sectors

    // Calculate the initial read information
    RI.CalcReadData( READ_CACHE_SIZE );

    RBuf = 1;
    WBuf = 0;
    CurSector = RI.m_FirstSector;

    // Issue the first read command
    sceCdSync( PS2CD_BLOCK );
    ReadStatus = sceCdRead( CurSector, READ_CACHE_SIZE, s_PS2CDReadCache[WBuf].m_CacheBuffer, &PS2RMode );
    SCEStatus  = sceCdGetError();

    while( !ReadStatus )
    {
        DBG_EXP( x_printf("...PS2CD Read: Error(1)- sceCdRead returned %ld, error: ", ReadStatus); PS2CD_ExplainError(SCEStatus); x_printf("\n"); );

        //if the read fails, continue trying until it's successful...
        ReadStatus = sceCdRead( CurSector, READ_CACHE_SIZE, s_PS2CDReadCache[WBuf].m_CacheBuffer, &PS2RMode );
        SCEStatus  = sceCdGetError();
    }

    if( !ReadStatus || (SCEStatus != SCECdErNO) )
    {
        // Read failed
        DBG_EXP( x_printf("...PS2CD Read: Error(2)- sceCdRead returned %ld, error: ", ReadStatus); PS2CD_ExplainError(SCEStatus); x_printf("\n"); );
        ASSERTS( FALSE, "Error on sceCdRead()" );
    }
    else
    {
        // Save current cache sector range
        s_PS2CDReadCache[WBuf].m_Start = CurSector;
        s_PS2CDReadCache[WBuf].m_End   = CurSector + READ_CACHE_SIZE;

        // execute loop at least once, to fill up as much cache as possible
        do
        {
            // Swap read-write buffers
            RBuf = 1 - RBuf;
            WBuf = 1 - WBuf;

            // Update current read sector and wait until previous read finishes, then
            // send another read while buffered data is copied
            CurSector += READ_CACHE_SIZE;
            sceCdSync( PS2CD_BLOCK );
            ReadStatus = sceCdRead( CurSector, READ_CACHE_SIZE, s_PS2CDReadCache[WBuf].m_CacheBuffer, &PS2RMode );
            SCEStatus  = sceCdGetError();

            if( !ReadStatus || (SCEStatus != SCECdErNO) )
            {
                // Read failed
                DBG_EXP( x_printf("...PS2CD Read: Error(3)- sceCdRead returned %ld, error: ", ReadStatus); PS2CD_ExplainError(SCEStatus); x_printf("\n"); );
                ASSERTS( FALSE, "Error on sceCdRead()" );
                break;
            }

            // Save current cache sector range
            s_PS2CDReadCache[WBuf].m_Start = CurSector;
            s_PS2CDReadCache[WBuf].m_End   = CurSector + READ_CACHE_SIZE;

            // Calculate amount of data for transfer, and then copy while other read command executes
            CopyLength = MIN( ((READ_CACHE_SIZE * PS2CD_SECTOR_SIZE) - RI.m_NLeadingBytes), RI.m_BytesToRead );
            x_memcpy( RI.m_pDestBuffer, &s_PS2CDReadCache[RBuf].m_CacheBuffer[ RI.m_NLeadingBytes ], CopyLength );

            DBG_EXP( x_printf( "...PS2CD Read: DataCopy- FPtr:%ld, %ld bytes\n", RI.m_FileOffset, CopyLength ) );

            // Update file position, data pointers and amount of data read
            RI.m_NLeadingBytes = 0;
            RI.UpdateReadAmount( CopyLength );

            // run user-defined callback if provided
            if( s_SyncReadCallback != NULL )
                s_SyncReadCallback();

        } while( RI.m_BytesToRead > 0 );

        // Make sure to wait on the last read
        sceCdSync( PS2CD_BLOCK );

        // Update actual file's read position
        pPS2File->m_Filepos = RI.m_FileOffset;
    }

    // "Unlock" the sync read
    s_bInSyncRead = FALSE;

    // Continue any async loads that might have been interupted by this read
    PS2CD_StartNextReadTask();

    return RI.m_TotalBytesRead;
}

//==========================================================================

static s32 PS2CD_ReadA( void* pBuffer, s32 Bytes, X_FILE* pFile, s32 FileOffset, s32 Priority, s32& TaskID )
{
    ASSERT( (Priority >= X_PRIORITY_LOW) && (Priority <= X_PRIORITY_TIMECRITICAL) );

    ASSERT( pBuffer != NULL );
    ASSERT( pFile != NULL );
    ASSERT( FileOffset >= 0 );
    ASSERT( FileOffset < ((PS2_FILE*)pFile)->m_Filesize );
    ASSERT( Bytes > 0 );

    // If any parameter is invalid, then jump out early- garauntees that
    // there won't be reads of zero length and beyond end of file
    if( (Bytes <= 0) || (FileOffset < 0) || (pFile == NULL) ||
        (pBuffer == NULL) || (FileOffset >= ((PS2_FILE*)pFile)->m_Filesize) )
    {
        TaskID = INVALID_TASK_ID;
        return FALSE;
    }

    // Make sure read doesn't go beyond the end of the file
    if( (FileOffset + Bytes) > (s32)((PS2_FILE*)pFile)->m_Filesize )
        Bytes = ((PS2_FILE*)pFile)->m_Filesize - FileOffset;

    // Shut down interrupt handlers here(if there are any)
    //

    // Fill in the new task info structure
    PS2_AsyncTask  NewTask;
    PS2_AsyncTask* pLeadTask;

    TaskID = PS2CD_MakeUniqueTaskID();

    NewTask.Reset();
    NewTask.m_pFile        = (PS2_FILE*)pFile;
    NewTask.m_pUserBuffer  = (byte*)pBuffer;
    NewTask.m_Status       = X_STATUS_PENDING;
    NewTask.m_Priority     = Priority;
    NewTask.m_TaskID       = TaskID;
    NewTask.m_FileOffset   = FileOffset;
    NewTask.m_BytesToRead  = Bytes;

    // Add new task to read list
    PS2CD_InsertReadTask( NewTask );

    pLeadTask = s_PS2ReadTasksList.GetHead();

    // If the new task is the first task in the list, it needs to
    // be started immediately, because it is the active read task.
    if( pLeadTask->m_TaskID == TaskID )
    {
        PS2CD_StartNextReadTask();
    }

    // Restart interrupt handlers(if there are any)
    //

    return TRUE;
}

//==========================================================================

static s32 PS2CD_ReadAStatus( s32 TaskID, s32& BytesRead )
{
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
        PS2CD_UpdateAsyncReads( TRUE );

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
        PS2CD_UpdateAsyncReads( TRUE );

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
    PS2CD_UpdateAsyncReads( TRUE );

    return RValue;
}

//==========================================================================

static s32 PS2CD_ReadACancel( s32 TaskID )
{
    PS2_AsyncTask* pTask;
    s32 RValue;
    s32 Result;

    // Shut down interrupt handlers(if there are any)
    //


    pTask = s_PS2ReadTasksList.GetHead();

    if( pTask != NULL )
    {
        if( (pTask->m_Status == X_STATUS_INPROGRESS) &&
            ((TaskID == -1) || (TaskID == pTask->m_TaskID)) )
        {
            // The currently in-progress task is the one we're trying to cancel, OR
            // all tasks are being canceled, so wait until the read completes to remove
            // the task from the list.

            Result = sceCdSync( PS2CD_NOBLOCK );

            while( Result == PS2CD_CDSYNC_BUSY )
            {
                MAD_DBG_EXP( x_printf( "...PS2CD CDSYNC_BUSY: Waiting on task ID %ld to cancel\n", pTask->m_TaskID ) );

                Result = sceCdSync( PS2CD_NOBLOCK );
            }
        }
    }


    if( TaskID == -1 )
    {
#if defined( PS2CD_DEBUG )
		pTask = s_PS2ReadTasksList.GetHead();
		while( pTask != NULL )
		{
			#if defined( X_DEBUG )
				x_printf( "...PS2CD ReadACancel: (All, Read list)  TaskID:%ld  Status:%ld  File: %s\n", pTask->m_TaskID, pTask->m_Status, pTask->m_pFile->m_Filename );
			#else
				x_printf( "...PS2CD ReadACancel: (All, Read list)  TaskID:%ld  Status:%ld\n", pTask->m_TaskID, pTask->m_Status );
			#endif
			pTask = pTask->m_pNext;
		}

		pTask = s_PS2ReadStatusList.GetHead();
		while( pTask != NULL )
		{
			#if defined( X_DEBUG )
				x_printf( "...PS2CD ReadACancel: (All, Status list)  TaskID:%ld  Status:%ld  File: %s\n", pTask->m_TaskID, pTask->m_Status, pTask->m_pFile->m_Filename );
			#else
				x_printf( "...PS2CD ReadACancel: (All, Status list)  TaskID:%ld  Status:%ld\n", pTask->m_TaskID, pTask->m_Status );
			#endif
			pTask = pTask->m_pNext;
		}
#endif

        // TaskID of -1 is used to cancel all async read tasks
        s_PS2ReadTasksList.Reset();
        s_PS2ReadStatusList.Reset();

        // Restart interrupt handlers(if there are any)
        //

        return X_STATUS_COMPLETE;
    }

    // Search for the requested task in the Read Task List.  If found,
    // return the current status of the task, and remove it from the list.
    pTask = s_PS2ReadTasksList.Find( TaskID );

    if( pTask != NULL )
    {
        RValue = pTask->m_Status;

        s_PS2ReadTasksList.Remove( TaskID );

        // Restart interrupt handlers(if there are any)
        //

        return RValue;
    }

    // If task not found in Read list, find the task in the Status list.
    pTask = s_PS2ReadStatusList.Find( TaskID );

    if( pTask != NULL )
    {
        RValue = pTask->m_Status;

        s_PS2ReadStatusList.Remove( TaskID );
    }
    else
    {
        RValue = X_STATUS_NOTFOUND;
    }

    // Restart interrupt handlers(if there are any)
    //

    return RValue;
}

//==========================================================================

static s32 PS2CD_Write( X_FILE* pXFile, byte* pBuffer, s32 Bytes )
{
    //ASSERTS( FALSE, "PS2 CD DOES NOT SUPPORT WRITING!" );
    return 0;
}

//==========================================================================

static s32 PS2CD_Seek( X_FILE* pXFile, s32 Offset, s32 Origin )
{
    PS2_FILE* pFile = (PS2_FILE*)pXFile;

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

static s32 PS2CD_Tell( X_FILE* pXFile )
{
    PS2_FILE* pFile = (PS2_FILE*)pXFile;

    ASSERT( pFile != NULL );
    ASSERT( pFile->m_bOpen );

    return pFile->m_Filepos;
}

//==========================================================================

static xbool PS2CD_EOF( X_FILE* pXFile )
{
    PS2_FILE* pFile = (PS2_FILE*)pXFile;

    ASSERT( pFile != NULL );

    return (pFile->m_Filepos >= pFile->m_Filesize) ? TRUE : FALSE;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// ASYNC LOADING HELPER FUNCTIONS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

static s32 PS2CD_MakeUniqueTaskID( void )
{
    static s32 s_NewID = 0;

    s_NewID++;

    // Make sure the TASK ID is never 0 or less.
    if( s_NewID <= 0 )
        s_NewID = 1;

    return s_NewID;
}

//==========================================================================

static void PS2CD_InsertReadTask( PS2_AsyncTask& NewTask )
{
    xbool bAdded;

    bAdded = s_PS2ReadTasksList.Add( NewTask );
    ASSERT( bAdded );

    MAD_DBG_EXP( x_printf( "...PS2CD InsertReadTask: TaskID:%ld  NBytes:%ld  Offset:%ld  File: %s\n", NewTask.m_TaskID, NewTask.m_BytesToRead, NewTask.m_FileOffset, NewTask.m_pFile->m_Filename ) );
    DBG_EXP( bAdded ? (void)0 : (void)x_printf( "...PS2CD InsertReadTask: FAILED TO ADD TASK TO LIST!!! TaskID:%ld\n", NewTask.m_TaskID ) );
}

//==========================================================================

static void PS2CD_StartNextReadTask( void )
{
    PS2_AsyncTask* pLeadTask;

    pLeadTask = s_PS2ReadTasksList.GetHead();

    // Continue to start the most important read task until either you run out of
    // tasks due to errors, or you have a successful read.
    while( pLeadTask != NULL )
    {
        MAD_DBG_EXP( x_printf( "...PS2CD StartNextReadTask: TaskID:%ld  File: %s\n", pLeadTask->m_TaskID, pLeadTask->m_pFile->m_Filename ) );

        // Start the read for this task, if successful break out of loop
        if( PS2CD_ReadAsync( pLeadTask ) )
            break;

        pLeadTask = s_PS2ReadTasksList.GetHead();
    }
}

//==========================================================================

static void PS2CD_CompleteReadTask( PS2_AsyncTask* pReadTask, s32 ReadTaskStatus )
{
    xbool bAdded;

    MAD_DBG_EXP( x_printf( "...PS2CD CompleteReadTask: TaskID:%ld, Status:%s  File: %s\n", pReadTask->m_TaskID, ReadTaskStatus == X_STATUS_COMPLETE ? "Success" : "Failure", pReadTask->m_pFile->m_Filename ) );

    // Set status on read task
    pReadTask->m_Status = ReadTaskStatus;

    // Make sure the task is in the read list
    pReadTask = s_PS2ReadTasksList.Find( pReadTask->m_TaskID );
    ASSERT( pReadTask );

    // Copy it to the status list
    bAdded = s_PS2ReadStatusList.Add( *pReadTask );
    ASSERT( bAdded );
    DBG_EXP( bAdded ? (void)0 : (void)x_printf( "...PS2CD CompleteReadTask: FAILED TO ADD TASK TO STATUS LIST!!! TaskID:%ld, Status:%ld\n", pReadTask->m_TaskID, ReadTaskStatus ) );

    // Remove it from the read task list
    s_PS2ReadTasksList.Remove( pReadTask->m_TaskID );
}

//==========================================================================

void PS2CD_UpdateAsyncReads( xbool bContinueTasks )
{
    s32            Result;
    PS2_AsyncTask* pTask;

    // This prevents any synchonous read callback from causing havok by
    // checking the "lock" state of a sync-read
    if( s_bInSyncRead )
        return;

    pTask = s_PS2ReadTasksList.GetHead();

    // Check if there are any async reads to update
    if( pTask == NULL )
        return;

    if( bContinueTasks )
    {
        // Check if read operation has completed
        Result = sceCdSync( PS2CD_NOBLOCK );

        if( Result == PS2CD_CDSYNC_BUSY )
        {
            //MAD_DBG_EXP( x_printf( "...PS2CD CDSYNC_BUSY: Status = %ld\n", (s32)sceCdStatus() ) );
            MAD_DBG_EXP( x_printf( "...PS2CD CDSYNC_BUSY: Waiting on task ID %ld\n", pTask->m_TaskID ) );

            // TJF - 02/08/04 - Commented this out, don't know why Skinner put it in, his comment on the
            // file change was "Possible CD Fixes for network play."  It would cause the game to 'hitch'
            // whenever it was checking for the status of an async read.
            //DelayThread( 1 );
            return;
        }
    }
    else
    {
        // wait and make sure read has completed(only done from PS2CD_Read)
        sceCdSync( PS2CD_BLOCK );
    }

    // If the lead task is not currently in progress, the async read had never started for it, so
    // don't copy data from async read buffer into user data buffer.  Instead try to start
    // the task and jump out.
    if( pTask->m_Status != X_STATUS_INPROGRESS )
    {
        if( bContinueTasks )
        {
            // Keep the read task chain going
            PS2CD_StartNextReadTask();
        }

        return;
    }


    // Calc number of bytes to copy into user buffer
    s32 NBytes = MIN( ((ASYNC_BUF_SIZE * PS2CD_SECTOR_SIZE) - pTask->m_NLeadingBytes), pTask->m_BytesToRead );

    MAD_DBG_EXP( x_printf( "...PS2CD UpdateAsyncReads: TaskID:%ld  BytesRead:%ld  File: %s\n", pTask->m_TaskID, NBytes, pTask->m_pFile->m_Filename ) );

    x_memcpy( pTask->m_pUserBuffer, &s_PS2CDAsyncCache.m_CacheBuffer[ pTask->m_NLeadingBytes ], NBytes );

    // Update file position, dest buffer pointer and number of bytes to read
    pTask->m_BytesToRead    -= NBytes;
    pTask->m_FileOffset     += NBytes;
    pTask->m_pUserBuffer    += NBytes;
    pTask->m_TotalBytesRead += NBytes;

    // Nothing left to read, report as done
    if( pTask->m_BytesToRead <= 0 )
    {
        // Remove the active task from the read task list, and start next read task.
        PS2CD_CompleteReadTask( pTask, X_STATUS_COMPLETE );
    }

    if( bContinueTasks )
    {
        // Keep the read task chain going
        PS2CD_StartNextReadTask();
    }
}

//==========================================================================

static xbool PS2CD_ReadAsync( PS2_AsyncTask* pReadTask )
{
    s32   ReadStatus;
    s32   SCEStatus;

    PS2_ReadInfo RI;
    sceCdRMode   PS2RMode;

    ASSERT( pReadTask != NULL );

    RI.m_LSN            = pReadTask->m_pFile->m_FileInfo.lsn;
    RI.m_FileOffset     = pReadTask->m_FileOffset;
    RI.m_FileSize       = pReadTask->m_pFile->m_Filesize;
    RI.m_pDestBuffer    = pReadTask->m_pUserBuffer;
    RI.m_BytesToRead    = pReadTask->m_BytesToRead;
    RI.m_TotalBytesRead = pReadTask->m_TotalBytesRead;

    RI.ReadCachedData( s_PS2CDAsyncCache );

    // update file position and bytes read
    pReadTask->m_BytesToRead    = RI.m_BytesToRead;
    pReadTask->m_FileOffset     = RI.m_FileOffset;
    pReadTask->m_pUserBuffer    = RI.m_pDestBuffer;
    pReadTask->m_TotalBytesRead = RI.m_TotalBytesRead;

    if( pReadTask->m_BytesToRead <= 0 )
    {
        PS2CD_CompleteReadTask( pReadTask, X_STATUS_COMPLETE );

        // Must return FALSE to keep task chain going- a normal async read would keep
        // chain alive through the callback function, but since this was a simple
        // memcpy PS2CD_StartNextReadTask() needs to keep the loop running
        return FALSE;
    }

    // Setup the PS2 structure for read mode
    PS2RMode.trycount    = 10;                      // Number of retries
    PS2RMode.spindlctrl  = SCECdSpinNom;            // Max rotational velocity, if errors then its reduced
    PS2RMode.datapattern = SCECdSecS2048;           // 2048 byte sectors

    RI.CalcReadData( ASYNC_BUF_SIZE );

    pReadTask->m_NLeadingBytes  = RI.m_NLeadingBytes;
    pReadTask->m_NTrailingBytes = RI.m_NTrailingBytes;

    MAD_DBG_EXP( x_printf( "...PS2CD ReadAsync: TaskID:%ld  NBytes:%ld  File: %s\n", pReadTask->m_TaskID, (RI.m_NSectors * PS2CD_SECTOR_SIZE), pReadTask->m_pFile->m_Filename ) );

    // Try the async read
    sceCdSync( PS2CD_BLOCK );
    ReadStatus = sceCdRead( RI.m_FirstSector, RI.m_NSectors, s_PS2CDAsyncCache.m_CacheBuffer, &PS2RMode );
    SCEStatus  = sceCdGetError();

    if( !ReadStatus )
    {
        DBG_EXP( x_printf("...PS2CD ReadAsync: Error(4)- sceCdRead returned %ld, error: ", ReadStatus); PS2CD_ExplainError(SCEStatus); x_printf("\n"); );
        return FALSE;
    }
    else if( /*!ReadStatus ||*/ (SCEStatus != SCECdErNO) )
    {
        // Error occurred
        DBG_EXP( x_printf("...PS2CD ReadAsync: Error(5)- sceCdRead returned %ld, error: ", ReadStatus); PS2CD_ExplainError(SCEStatus); x_printf("\n"); );

        PS2CD_CompleteReadTask( pReadTask, X_STATUS_ERROR );
        return FALSE;
    }

    // Mark task as in progress
    pReadTask->m_Status = X_STATUS_INPROGRESS;

    // Store used cache information
    s_PS2CDAsyncCache.m_Start = RI.m_FirstSector;
    s_PS2CDAsyncCache.m_End   = RI.m_FirstSector + RI.m_NSectors;

    return TRUE;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// OTHER FILE IO IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#ifdef PS2CD_DEBUG

static void PS2CD_ExplainError( s32 Err )
{
    switch( Err )
    {
     case SCECdErFAIL:   x_printf( "sceCdGetError failure" );             break;
     case SCECdErNO:     x_printf( "No Error" );                          break;
     case SCECdErEOM:    x_printf( "Outermost track reached" );           break;
     case SCECdErTRMOPN: x_printf( "Cover opened" );                      break;
     case SCECdErREAD:   x_printf( "Problem occurred during read" );      break;
     case SCECdErCUD:    x_printf( "Not appropriate for disc in drive" ); break;
     case SCECdErNORDY:  x_printf( "Processing command" );                break;
     case SCECdErABRT:   x_printf( "Abort command recieved" );            break;
     case SCECdErREADCF: x_printf( "Read command issue failed" );         break;

     //case SCECdErPRM:    x_printf( "SCECdErPRM" );     break;
     //case SCECdErILI:    x_printf( "SCECdErILI" );     break;
     //case SCECdErIPI:    x_printf( "SCECdErIPI" );     break;
     //case SCECdErNODISC: x_printf( "SCECdErNODISC" );  break;
     //case SCECdErOPENS:  x_printf( "SCECdErOPENS" );   break;
     //case SCECdErCMD:    x_printf( "SCECdErCMD" );     break;

     default:
        x_printf( "Unknown Error %ld [%08x]", Err );
        break;
    }
}

#endif //PS2CD_DEBUG

//==========================================================================

void PS2CD_PrepImageFile( char* pSystemImageFile )
{
    s_pSystemImageFile = pSystemImageFile;
}

//==========================================================================

void PS2CD_SetReadCallback( QSyncReadCallback CallbackFunc )
{
    s_SyncReadCallback = CallbackFunc;
}

//==========================================================================

s32 PS2CD_GetMediaType( void )
{
    s_LastKnownMediaType = sceCdGetDiskType();

    #ifdef PS2CD_DEBUG
        switch( s_LastKnownMediaType )
        {
            case SCECdIllgalMedia:  x_printf( "PS2CD: Disk Type= Illgal Media\n" );     break;
            case SCECdPS2DVD:       x_printf( "PS2CD: Disk Type= PS2 DVD\n" );          break;
            case SCECdPS2CD:        x_printf( "PS2CD: Disk Type= PS2 CD\n" );           break;
            case SCECdPS2CDDA:      x_printf( "PS2CD: Disk Type= PS2 CD with CDDA\n" ); break;
            case SCECdPSCD:         x_printf( "PS2CD: Disk Type= PS1 CD\n" );           break;
            case SCECdPSCDDA:       x_printf( "PS2CD: Disk Type= PS1 CD with CDDA\n" ); break;
            case SCECdDVDV:         x_printf( "PS2CD: Disk Type= DVD video\n" );        break;
            case SCECdCDDA:         x_printf( "PS2CD: Disk Type= CD-DA\n" );            break;
            case SCECdDETCT:        x_printf( "PS2CD: Disk Type= Detecting...\n" );     break;
            case SCECdNODISC:       x_printf( "PS2CD: Disk Type= No Disc\n" );          break;
            default:
                x_printf("PS2CD :Disk Type= OTHER DISK\n");
                break;
        }
    #endif

    return s_LastKnownMediaType;
}

//==========================================================================

xbool PS2CD_IsMediaUseable( void )
{
    if( (s_LastKnownMediaType == PS2CD_MEDIA_NONE) ||
        (s_LastKnownMediaType == PS2CD_MEDIA_ILLEGAL) )
    {
        return FALSE;
    }

    return TRUE;
}

//==========================================================================

void PS2CD_Eject( void )
{

}

//==========================================================================

void PS2CD_Standby( void )
{
    sceCdDiskReady( PS2CD_BLOCK );
    sceCdSync( PS2CD_BLOCK );
    sceCdStandby();
}

//==========================================================================

void PS2CD_Stop( void )
{
    sceCdDiskReady( PS2CD_BLOCK );
    sceCdSync( PS2CD_BLOCK );
    sceCdStop();
}

//==========================================================================

void PS2CD_Boot( void )
{
    // NO printing should be performed before reboot completion

    // INIT RPC
    sceSifInitRpc( 0 );

    // INIT CD
    sceCdInit( SCECdINIT );

    // SET MEDIA MODE
    #ifdef TARGET_PS2_DEMO
        sceCdMmode( SCECdCD );
    #else
        sceCdMmode( SCECdDVD );
    #endif

    // Reboot the IOP and replace the ROM modules with ones on the CD
    while( !sceSifRebootIop( s_pSystemImageFile ) );
    while( !sceSifSyncIop() );

    // Reinitialize the system again
    sceSifInitRpc( 0 );
    sceSifInitIopHeap();        // new - from the newsgroups
    sceSifLoadFileReset();      // new - from the newgroups
    sceCdInit( SCECdINIT );
    sceCdDiskReady(0);          // new - from the newsgroups

#ifdef TARGET_PS2_DEMO
    sceCdMmode( SCECdCD );
#else
    sceCdMmode( SCECdDVD );
#endif

    sceFsReset();
}

//==========================================================================

void PS2CD_Interface( void )
{
    // setup the x_files stdio handlers to point to our functions
    x_SetIOHandlers( PS2CD_Open,
                     PS2CD_Close,
                     PS2CD_Read,
                     PS2CD_ReadA,
                     PS2CD_ReadAStatus,
                     PS2CD_ReadACancel,
                     PS2CD_Write,
                     PS2CD_Seek,
                     PS2CD_Tell,
                     PS2CD_EOF );
}

//==========================================================================

void PS2CD_Init( void )
{
    s32 i;

    for( i = 0; i < MAX_PS2_FILES; i++ )
        s_PS2Files[i].Reset();

    s_PS2ReadTasksList.Reset();
    s_PS2ReadStatusList.Reset();

    s_PS2CDReadCache[0].Reset();
    s_PS2CDReadCache[0].m_CacheBuffer = s_PS2CDReadBufferA;

    s_PS2CDReadCache[1].Reset();
    s_PS2CDReadCache[1].m_CacheBuffer = s_PS2CDReadBufferB;

    s_PS2CDAsyncCache.Reset();
    s_PS2CDAsyncCache.m_CacheBuffer = s_PS2CDAsyncBuffer;

    PS2CD_GetMediaType();
}

//==========================================================================

void PS2CD_Kill( void )
{
    x_SetIOHandlers( NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL );
}

//==========================================================================

#endif //#if defined( TARGET_PS2_DVD )