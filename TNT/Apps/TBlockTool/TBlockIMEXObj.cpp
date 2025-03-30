////////////////////////////////////////////////////////////////////////////
//
// TBlockIMEXObj.cpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "x_debug.hpp"

#include "StatusOutput.h"
#include "TBlockIMEXObj.h"
#include "TBlockCmdLineTxt.h"
#include "TBlockWriteBMP.h"
#include "XBmpProcess.h"
#include "x_math.hpp"


#pragma warning( disable : 4311 ) // disable warning(4311): 'type cast' : pointer truncation from 'SOME_TYPE *' to 'SOME_OTHER_TYPE'
#pragma warning( disable : 4312 ) // disable warning(4312): 'type cast' : conversion from 'SOME_TYPE' to 'SOME_OTHER_TYPE *' of greater size


////////////////////////////////////////////////////////////////////////////
// CONSTANTS
////////////////////////////////////////////////////////////////////////////

#define TB_VERIFY(exp)         ((void)( (exp) || (DebugVector( XDB_VERIFY,  __FILE__, __LINE__, #exp, NULL )) || DEBUG_HALT ))
#define TB_VERIFYS(exp,str)    ((void)( (exp) || (DebugVector( XDB_VERIFYS, __FILE__, __LINE__, #exp, str  )) || DEBUG_HALT ))


#define TBLOCK_TEMP_EXPORT_FILE     "_TBLOCKTOOL_TEMP_"
#define TBLOCK_TEMP_EXP_FILE_MAX    256

#define TBLOCK_HEADER_FILESTR       "TBLOCKTEX_30_XE"

#define TBLOCK_ENDIAN_UNKNOWN       'X'
#define TBLOCK_ENDIAN_LITTLE        'L'
#define TBLOCK_ENDIAN_BIG           'B'
#define TBLOCK_ENDIAN_INDEX         13


////////////////////////////////////////////////////////////////////////////
// STRUCTURES
////////////////////////////////////////////////////////////////////////////

struct TBlockHeader
{
    char    m_FileStr[16];
    s32     m_NFiles;
    s32     m_TgtPlatform;
    s32     m_TotalBMPSize;
    s32     m_MinBMPSize;
    s32     m_MaxBMPSize;
    s32     m_BMPDataEnd;
};


////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
////////////////////////////////////////////////////////////////////////////

static void EndianSwapTBlockHdr( TBlockHeader& rTBHdr )
{
    rTBHdr.m_NFiles       = ENDIAN_SWAP_32( rTBHdr.m_NFiles );
    rTBHdr.m_TgtPlatform  = ENDIAN_SWAP_32( rTBHdr.m_TgtPlatform );
    rTBHdr.m_TotalBMPSize = ENDIAN_SWAP_32( rTBHdr.m_TotalBMPSize );
    rTBHdr.m_MinBMPSize   = ENDIAN_SWAP_32( rTBHdr.m_MinBMPSize );
    rTBHdr.m_MaxBMPSize   = ENDIAN_SWAP_32( rTBHdr.m_MaxBMPSize );
    rTBHdr.m_BMPDataEnd   = ENDIAN_SWAP_32( rTBHdr.m_BMPDataEnd );
}

//==========================================================================

static X_FILE* CreateTempExportFile( TBlockPath &rTempFName )
{
    s32             i;
    X_FILE*         pXFile;
    HANDLE          hFind;
    WIN32_FIND_DATA FindData;

    // Attempt 32 times
    for( i = 0; i < TBLOCK_TEMP_EXP_FILE_MAX; i++ )
    {
        x_sprintf( rTempFName, "%s%04ld", TBLOCK_TEMP_EXPORT_FILE, i );

        // check if file exists
        hFind = FindFirstFile( rTempFName, &FindData );

        if( INVALID_HANDLE_VALUE == hFind )
        {
            // file doesn't exist, try to open it for write
            pXFile = x_fopen( rTempFName, "w" );

            if( pXFile != NULL )
            {
                // file creation successful
                FindClose( hFind );
                return pXFile;
            }
        }

        FindClose( hFind );
    }

    // couldn't create a temp file, return NULL
    return (X_FILE*)NULL;
}

////////////////////////////////////////////////////////////////////////////
// TBlockIMEXObj IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////

TBlockIMEXObj::TBlockIMEXObj( void )
{
    m_pPaths      = NULL;
    m_pTBS        = NULL;
    m_pNameOffset = NULL;
    m_pXFile      = NULL;
    m_pThread     = NULL;

    m_bDebugTintMips = FALSE;

    KillData();
}

//==========================================================================

TBlockIMEXObj::~TBlockIMEXObj( void )
{
    KillData();
}

//==========================================================================

void TBlockIMEXObj::KillData( void )
{
    // Make sure execution isn't running on another thread
    if( m_pThread != NULL )
    {
        TB_VERIFYS( m_pThread == NULL, "Attempt to destroy object data while thread active." );
        m_bStopProcess = TRUE;
        m_bKillOnFinish = TRUE;
        return;
    }

    // free up memory
    if( m_pPaths != NULL )      delete[] m_pPaths;
    if( m_pTBS != NULL )        delete[] m_pTBS;
    if( m_pNameOffset != NULL ) delete[] m_pNameOffset;
    if( m_pXFile != NULL )      x_fclose( m_pXFile );

    // zero out members
    m_pPaths      = NULL;
    m_pTBS        = NULL;
    m_pNameOffset = NULL;
    m_pXFile      = NULL;

    m_GlobalTBS.InitDefault();

    m_pFinishCallback = NULL;
    m_cbParam         = 0;
    m_NFiles          = 0;
    m_Target          = TBLOCK_TGT_NOTSET;
    m_bTextFile       = FALSE;
    m_bInProgress     = FALSE;
    m_bStopProcess    = FALSE;
    m_bKillOnFinish   = FALSE;
    m_pThread         = NULL;

    m_Filename[0] = '\0';

    m_strTempFilename[0] = '\0';
}

//==========================================================================

s32 TBlockIMEXObj::GetNFiles( void )
{
    return m_NFiles;
}

//==========================================================================

TBLOCK_TARGET TBlockIMEXObj::GetTarget( void )
{
    return m_Target;
}

//==========================================================================

xbool TBlockIMEXObj::IsTgtBigEndian( void )
{
    if( m_Target == TBLOCK_TGT_GAMECUBE )
        return TRUE;

    return FALSE;
}

//==========================================================================

void TBlockIMEXObj::GetTBSetting( s32 Index, TBlockSettings& rTBS )
{
    if( !m_bInProgress )
    {
        if( (Index >= 0) && (Index < m_NFiles) )
        {
            if( m_pTBS != NULL )
            {
                rTBS = m_pTBS[Index];
                return;
            }
        }
    }

    rTBS = m_GlobalTBS;
}

//==========================================================================

void TBlockIMEXObj::GetTBPath( s32 Index, TBlockPath& rTBPath )
{
    if( !m_bInProgress )
    {
        if( (Index >= 0) && (Index < m_NFiles) )
        {
            if( m_pPaths != NULL )
            {
                x_strncpy( rTBPath, m_pPaths[Index], TBLOCK_PATH_LENGTH );
                return;
            }
        }
    }

    x_strncpy( rTBPath, TBLOCK_EMPTY_PATH, TBLOCK_PATH_LENGTH );
}

//==========================================================================

void TBlockIMEXObj::GetGlobalTBS( TBlockSettings& rGlobalTBS )
{
    rGlobalTBS = m_GlobalTBS;
}

//==========================================================================

const char* TBlockIMEXObj::GetFilename( void )
{
    return m_Filename;
}

//==========================================================================

void TBlockIMEXObj::EnableTintMips( xbool bEnable )
{
    if( IsFinished() )
        m_bDebugTintMips = bEnable;
}

//==========================================================================

xbool TBlockIMEXObj::IsTintMipsEnabled( void )
{
    return m_bDebugTintMips;
}

//==========================================================================

xbool TBlockIMEXObj::IsFinished( void )
{
    return m_bInProgress ? FALSE : TRUE;
}

//==========================================================================

void TBlockIMEXObj::StopProcess( void )
{
    m_bStopProcess = TRUE;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// IMPORT IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

s32 TBlockIMEXObj::Import( const char*     pImportFilename,
                           TBlockSettings* pGlobalSettings,
                           TBExportCB      FinishCallback,
                           u32             cbParam )
{
    // Check if import is in progress(can only do one at a time per instance)
    if( m_bInProgress )
        return TB_RCODE_INPROGRESS;

    // Destroy current data
    KillData();

    ASSERT( pImportFilename != NULL );

    // Do some error checking
    if( pImportFilename == NULL )
    {
        STATUS::Print( STATUS::MSG_ERROR, "Corrupt filename data.\n" );
        return TB_RCODE_FAIL_ERROR;
    }

    // Try to open the file for import
    char  ExtStr[X_MAX_EXT];
    char* pExt;

    x_splitpath( pImportFilename, NULL, NULL, NULL, ExtStr );

    if( ExtStr[0] == '.' )
        pExt = &ExtStr[1];
    else
        pExt = &ExtStr[0];

    // Determine if it is a text file or TBlock file
    if( !x_stricmp( pExt, TBLOCK_FILE_EXT_TXT ) )
    {
        m_pXFile = x_fopen( pImportFilename, "r" );
        m_bTextFile = TRUE;
    }
    else if( !x_stricmp( pExt, TBLOCK_FILE_EXT_TEX ) )
    {
        m_pXFile = x_fopen( pImportFilename, "rb" );
        m_bTextFile = FALSE;
    }
    else
    {
        STATUS::Print( STATUS::MSG_ERROR, "Unrecognized file extension for import: %s on %s\n", ExtStr, pImportFilename );
        return TB_RCODE_FILE_ERROR;
    }

    //ASSERT( m_pXFile != NULL );

    if( m_pXFile == NULL )
    {
        STATUS::Print( STATUS::MSG_ERROR, "Couldn't open file for import: %s\n", pImportFilename );
        return TB_RCODE_FILE_ERROR;
    }

    // save filename
    x_strncpy( m_Filename, pImportFilename, TBLOCK_PATH_LENGTH );

    // copy the global settings(if given, in case import of text file has no settings)
    if( pGlobalSettings != NULL )
        x_memcpy( &m_GlobalTBS, pGlobalSettings, sizeof(TBlockSettings) );

    // save callback function and parameter
    m_pFinishCallback = FinishCallback;
    m_cbParam = cbParam;

    // check for synchronous or async execution
    if( FinishCallback == NULL )
    {
        return ExecuteImport();
    }
    else
    {
        CWinThread* pThread;

        pThread = AfxBeginThread( TBlockIMEXObj::Thread_Import,
                                  (LPVOID)this,
                                  THREAD_PRIORITY_ABOVE_NORMAL,
                                  0,
                                  CREATE_SUSPENDED,
                                  NULL );

        m_pThread = (void*)pThread;

        pThread->ResumeThread();

        return TB_RCODE_IMEX_STARTED;
    }
}

//==========================================================================

UINT TBlockIMEXObj::Thread_Import( LPVOID pParam )
{
    TBlockIMEXObj* pTHIS = (TBlockIMEXObj*)pParam;

    s32 RetCode = pTHIS->ExecuteImport();

    pTHIS->m_pThread = NULL;

    if( pTHIS->m_pFinishCallback( pTHIS->m_cbParam, pTHIS, RetCode ) )
    {
        if( pTHIS->m_bKillOnFinish )
            pTHIS->KillData();
    }

    return RetCode;
}

//==========================================================================

s32 TBlockIMEXObj::ExecuteImport( void )
{
    s32 RetCode;

    m_bInProgress = TRUE;

    if( m_bTextFile )
        RetCode = ImportTextFile();
    else
        RetCode = ImportTBlockFile();

    m_bInProgress = FALSE;

    x_fclose( m_pXFile );
    m_pXFile = NULL;

    return RetCode;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// TEXT FILE IMPORT
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

s32 TBlockIMEXObj::ImportTextFile( void )
{
    s32  RetCode;
    char TextLine[ TBLOCK_PATH_LENGTH + TBCMDLINE::MAX_LENGTH ];
    char SaveChar;
    s32  LineCount;
    s32  Length;

    LineCount = 0;

    // Count number of lines in text file to determine approximate path count
    while( !x_feof( m_pXFile ) )
    {
        if( m_bStopProcess )
        {
            // Operation has been cancelled, kill data and return
            STATUS::Print( STATUS::MSG_IMPORTANT, "Import cancelled on %s\n", m_Filename );
            this->m_bKillOnFinish = TRUE;
            return TB_RCODE_TERMINATED;
        }

        char* pFilename = fgets( TextLine, (TBLOCK_PATH_LENGTH+TBCMDLINE::MAX_LENGTH), (FILE*)m_pXFile );
        if( pFilename != TextLine )
        {
            if( x_feof( m_pXFile ) )
                break;
            else if( ferror( (FILE*)m_pXFile ) )
                STATUS::Print( STATUS::MSG_WARNING, "WARNING: Error occurred reading text file, line %ld.\n", LineCount );
            else
                STATUS::Print( STATUS::MSG_WARNING, "WARNING: Unknown Error occurred reading text file, line %ld.\n", LineCount );
        }

        LineCount++;
    }

    if( LineCount == 0 )
    {
        STATUS::Print( STATUS::MSG_WARNING, "WARNING: Text file is empty: %s\n", m_Filename );
        return TB_RCODE_FAIL_ERROR;
    }

    // Seek back to start of text file
    x_fseek( m_pXFile, 0, X_SEEK_SET );

    // Allocate data necessary for import
    m_pNameOffset = new TBNameOffset[LineCount + 8];
    m_pPaths      = new TBlockPath[LineCount + 8];
    m_pTBS        = new TBlockSettings[LineCount + 8];

    if( (m_pNameOffset == NULL) || (m_pPaths == NULL) || (m_pTBS == NULL) )
    {
        ASSERTS( FALSE, "Memory allocation failure" );
        STATUS::Print( STATUS::MSG_ERROR, "ERROR: Memory allocation failure in %s, line %d.\n", __FILE__, __LINE__ );
        KillData();
        return TB_RCODE_FAIL_ERROR;
    }

    // reset counts
    LineCount = 1;
    m_NFiles  = 0;

    // Parse text file
    while( !x_feof( m_pXFile ) )
    {
        if( m_bStopProcess )
        {
            // Operation has been cancelled, kill data and return
            STATUS::Print( STATUS::MSG_IMPORTANT, "Import cancelled on %s\n", m_Filename );
            this->m_bKillOnFinish = TRUE;
            return TB_RCODE_TERMINATED;
        }

        // Get one line of text at a time
        char* pFilename = fgets( TextLine, (TBLOCK_PATH_LENGTH+TBCMDLINE::MAX_LENGTH), (FILE*)m_pXFile );

        // Check for errors
        if( pFilename != TextLine )
        {
            if( x_feof( m_pXFile ) )
                break;
            else if( ferror( (FILE*)m_pXFile ) )
                STATUS::Print( STATUS::MSG_WARNING, "WARNING: Error occurred reading text file, line %ld.\n", LineCount );
            else
                STATUS::Print( STATUS::MSG_WARNING, "WARNING: Unknown Error occurred reading text file, line %ld.\n", LineCount );
        }

        // remove any whitespace on the end of the text line(newlines, etc.)
        Length = x_strlen( TextLine ) - 1;
        while( x_isspace( TextLine[Length] ) )
        {
            TextLine[Length] = '\0';
            Length--;
            if( Length < 0 )
                break;
        }
        if( Length < 0 )
        {
            STATUS::Print( STATUS::MSG_WARNING, "WARNING: Line %ld has whitespace characters only.\n", LineCount );
            continue;
        }

        // Set current TBS to global settings
        m_pTBS[m_NFiles] = m_GlobalTBS;

        // Parse the text line for commands
        RetCode = TBCMDLINE::Parse( TextLine, m_pTBS[m_NFiles], Length );

        if( RetCode != TBCMDLINE::RCODE_OK )
        {
            if( RetCode == TBCMDLINE::RCODE_UNKNOWN_SWITCH )
            {
                STATUS::Print( STATUS::MSG_WARNING, "WARNING: Unknown switch on line %ld.\n", LineCount );
            }
            else if( RetCode == TBCMDLINE::RCODE_SWITCH_PARAM_MISSING )
            {
                STATUS::Print( STATUS::MSG_WARNING, "WARNING: Missing switch parameter on line %ld.\n", LineCount );
            }
            else if( RetCode == TBCMDLINE::RCODE_SWITCH_PARAM_ERROR )
            {
                STATUS::Print( STATUS::MSG_WARNING, "WARNING: Switch parameter error on line %ld.\n", LineCount );
            }
        }

        // "chop" off command line text to get/test pathname
        SaveChar = TextLine[Length];
        TextLine[Length] = '\0';

        // if the pathname references the global one, init the global settings instead
        if( !x_stricmp( TextLine, TBLOCK_GLOBAL_PATHNAME ) )
             m_GlobalTBS = m_pTBS[m_NFiles];
        else
        {
            // copy pathname and increment count
            x_strcpy( m_pPaths[m_NFiles], TextLine );
            m_NFiles++;
        }

        LineCount++;
    }

    STATUS::Print( STATUS::MSG_NORMAL, "Imported %s successfully.\n", m_Filename );

    return TB_RCODE_OK;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// TBLOCK FILE IMPORT
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

//############# TEMP ###################################################

s32 TBlockIMEXObj::ImportOLDTEXFILE( void )
{
    const s32 FULL_PATHNAME_SIZE    = 264;
    const s32 FLAG_SAVE_PATHS       = (1<<31);
    const s32 FLAG_FILE_OFFSETS     = (1<<30);
    const s32 FLAG_PLATFORM_PS2     = (1<<24);
    const s32 FLAG_PLATFORM_GC      = (1<<25);
    const s32 FLAG_PLATFORM_XBOX    = (1<<26);
    const s32 FLAG_PLATFORM_PC      = (1<<27);
    const s32 FLAG_CONVERT_DATA     = (1<<0);
    const s32 FLAG_APPLY_ALPHA      = (1<<1);
    const s32 FLAG_BUILD_MIPS       = (1<<2);

    struct SBinFileHeader
    {
        u8      mHeader[16];
        s32     mNumFiles;
        s32     mFlags;
        u32     mTotalDataSize;
        u32     mMaxFileSize;
        u32     mMinFileSize;
    };

    SBinFileHeader OldHdr;
    char TempPath[FULL_PATHNAME_SIZE];

    STATUS::Print( STATUS::MSG_IMPORTANT, "NOTICE: Importing OLD TEX file format on: %s\n", m_Filename );

    x_fseek( m_pXFile, 0, X_SEEK_SET );

    x_fread( &OldHdr, sizeof(SBinFileHeader), 1, m_pXFile );

    if( OldHdr.mHeader[15] == 0xFF )
    {
        OldHdr.mNumFiles      = ENDIAN_SWAP_32( OldHdr.mNumFiles );
        OldHdr.mFlags         = ENDIAN_SWAP_32( OldHdr.mFlags );
        OldHdr.mTotalDataSize = ENDIAN_SWAP_32( OldHdr.mTotalDataSize );
        OldHdr.mMaxFileSize   = ENDIAN_SWAP_32( OldHdr.mMaxFileSize );
        OldHdr.mMinFileSize   = ENDIAN_SWAP_32( OldHdr.mMinFileSize );
    }

    if( !(OldHdr.mFlags & FLAG_SAVE_PATHS) )
    {
        STATUS::Print( STATUS::MSG_ERROR, "ERROR: No path information stored in %s\n", m_Filename );
        return TB_RCODE_FAIL_ERROR;
    }

    s32 OffsetAmount = OldHdr.mNumFiles * FULL_PATHNAME_SIZE;

    if( OldHdr.mFlags & FLAG_FILE_OFFSETS )
        OffsetAmount += (s32)sizeof(u32) * OldHdr.mNumFiles;

    x_fseek( m_pXFile, -OffsetAmount, X_SEEK_END );

    // allocate memory to import data
    m_pNameOffset = new TBNameOffset[OldHdr.mNumFiles];
    m_pPaths      = new TBlockPath[OldHdr.mNumFiles];
    m_pTBS        = new TBlockSettings[OldHdr.mNumFiles];

    if( (m_pNameOffset == NULL) || (m_pPaths == NULL) || (m_pTBS == NULL) )
    {
        ASSERTS( FALSE, "Memory allocation failure" );
        STATUS::Print( STATUS::MSG_ERROR, "ERROR: Memory allocation failure in %s, line %d.\n", __FILE__, __LINE__ );
        KillData();
        return TB_RCODE_FAIL_ERROR;
    }

    // save header settings from file
    m_NFiles = OldHdr.mNumFiles;

    if( OldHdr.mFlags & FLAG_PLATFORM_GC )
        m_Target = TBLOCK_TGT_GAMECUBE;
    else if( OldHdr.mFlags & FLAG_PLATFORM_XBOX )
        m_Target = TBLOCK_TGT_XBOX;
    else if( OldHdr.mFlags & FLAG_PLATFORM_PC )
        m_Target = TBLOCK_TGT_PCDX8;
    else
        m_Target = TBLOCK_TGT_PS2;


    if( OldHdr.mFlags & FLAG_CONVERT_DATA )
        m_GlobalTBS.SetSwizzle( TRUE );
    else
        m_GlobalTBS.SetSwizzle( FALSE );

    if( OldHdr.mFlags & FLAG_APPLY_ALPHA )
        m_GlobalTBS.SetApplyAlpha( TRUE, TRUE, FALSE, FALSE );
    else
        m_GlobalTBS.SetApplyAlpha( FALSE, TRUE, FALSE, FALSE );

    if( OldHdr.mFlags & FLAG_BUILD_MIPS )
        m_GlobalTBS.SetBuildMips( TRUE, FALSE, TBLOCK_BUILD_MIPS_MAX );
    else
        m_GlobalTBS.SetBuildMips( FALSE, FALSE, TBLOCK_BUILD_MIPS_MAX );

    s32 i;

    for( i = 0; i < OldHdr.mNumFiles; i++ )
    {
        x_fread( TempPath, sizeof(char), FULL_PATHNAME_SIZE, m_pXFile );
        x_strncpy( m_pPaths[i], TempPath, TBLOCK_PATH_LENGTH );

        TBLOCKPATH_GetShortName( m_pPaths[i], m_pNameOffset[i].m_ShortName );

        m_pNameOffset[i].m_Offset = -1;
        m_pTBS[i] = m_GlobalTBS;
    }

    STATUS::Print( STATUS::MSG_NORMAL, "Imported %s successfully.\n", m_Filename );

    return TB_RCODE_OK;
}

//######################################################################

s32 TBlockIMEXObj::ImportTBlockFile( void )
{
    s32 FileEnd;
    s32 DataSize;
    s32 TBSDataSize;
    TBlockHeader TBHdr;

    // Get file size for sanity checks
    x_fseek( m_pXFile, 0, X_SEEK_END );
    FileEnd = x_ftell( m_pXFile );
    x_fseek( m_pXFile, 0, X_SEEK_SET );

    // read header info
    x_fread( &TBHdr, sizeof(TBlockHeader), 1, m_pXFile );

//############# TEMP ################

    if( !x_strcmp( TBHdr.m_FileStr, "ASB_TEXTURE" ) )
    {
        return ImportOLDTEXFILE();
    }

//###################################

    // swap endian-ness if needed
    if( TBHdr.m_FileStr[TBLOCK_ENDIAN_INDEX] == TBLOCK_ENDIAN_BIG )
        EndianSwapTBlockHdr( TBHdr );

    TBHdr.m_FileStr[TBLOCK_ENDIAN_INDEX] = TBLOCK_ENDIAN_UNKNOWN;

    // calc data size and size of data used by TBlock to import(sanity checks)
    DataSize    = sizeof(TBlockHeader) + (sizeof(TBNameOffset) * TBHdr.m_NFiles) + TBHdr.m_TotalBMPSize;
    TBSDataSize = sizeof(TBlockSettings) + ((sizeof(TBlockSettings) + sizeof(TBlockPath)) * TBHdr.m_NFiles);

    // Perform a bunch of sanity checks to ensure this file is ok
    if( x_strcmp( TBHdr.m_FileStr, TBLOCK_HEADER_FILESTR ) )
    {
        STATUS::Print( STATUS::MSG_ERROR, "ERROR: %s is not a TBlock file.\n", m_Filename );
        TB_VERIFYS( FALSE, "File was not created by TBlock" );
        return TB_RCODE_FAIL_ERROR;
    }
    if( (TBHdr.m_NFiles <= 0) ||
        (TBHdr.m_TgtPlatform <= TBLOCK_TGT_NOTSET) ||
        (TBHdr.m_TgtPlatform >= TBLOCK_TGT_COUNT) ||
        (TBHdr.m_TotalBMPSize <= 0) ||
        (TBHdr.m_MaxBMPSize <= 0) ||
        (TBHdr.m_BMPDataEnd <= 0) ||
        (TBHdr.m_BMPDataEnd != DataSize) )
    {
        STATUS::Print( STATUS::MSG_ERROR, "ERROR: %s has corrupt header data.\n", m_Filename );
        TB_VERIFYS( FALSE, "File has corrupt header data" );
        return TB_RCODE_FAIL_ERROR;
    }
    if( (DataSize + TBSDataSize) != FileEnd )
    {
        STATUS::Print( STATUS::MSG_ERROR, "ERROR: %s has corrupt TBlock import data.\n", m_Filename );
        TB_VERIFYS( FALSE, "File has corrupt TBlock import data" );
        return TB_RCODE_FAIL_ERROR;
    }

    // File appears to be ok, allocate memory to import data
    m_pNameOffset = new TBNameOffset[TBHdr.m_NFiles];
    m_pPaths      = new TBlockPath[TBHdr.m_NFiles];
    m_pTBS        = new TBlockSettings[TBHdr.m_NFiles];

    if( (m_pNameOffset == NULL) || (m_pPaths == NULL) || (m_pTBS == NULL) )
    {
        ASSERTS( FALSE, "Memory allocation failure" );
        STATUS::Print( STATUS::MSG_ERROR, "ERROR: Memory allocation failure in %s, line %d.\n", __FILE__, __LINE__ );
        KillData();
        return TB_RCODE_FAIL_ERROR;
    }

    // save header settings from file
    m_NFiles = TBHdr.m_NFiles;
    m_Target = (TBLOCK_TARGET)TBHdr.m_TgtPlatform;

    // read in short names(offsets mean nothing at the moment)
    x_fread( m_pNameOffset, sizeof(TBNameOffset), m_NFiles, m_pXFile );

    // seek to TBlock data near end of file
    x_fseek( m_pXFile, TBHdr.m_BMPDataEnd, X_SEEK_SET );

    // read in settings(and global setting), as well as path info
    x_fread( m_pTBS,       sizeof(TBlockSettings), m_NFiles, m_pXFile );
    x_fread( &m_GlobalTBS, sizeof(TBlockSettings), 1,        m_pXFile );
    x_fread( m_pPaths,     sizeof(TBlockPath),     m_NFiles, m_pXFile );

    STATUS::Print( STATUS::MSG_NORMAL, "Imported %s successfully.\n", m_Filename );

    return TB_RCODE_OK;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// EXPORT IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

s32 TBlockIMEXObj::Export( const char*      pExportFilename,
                           TBLOCK_TARGET    Target,
                           s32              NFiles,
                           TBlockSettings*  pGlobalSettings,
                           TBlockSettings** ppTexSettings,
                           TBlockPath**     ppTexPaths,
                           TBExportCB       FinishCallback,
                           u32              cbParam )
{
    s32 i;

    // Check if export is in progress(can only do one at a time per instance)
    if( m_bInProgress )
        return TB_RCODE_INPROGRESS;

    // Destroy current data
    KillData();

    // Do some error checking
    ASSERT( NFiles > 0 );
    ASSERT( pGlobalSettings != NULL );
    ASSERT( ppTexSettings != NULL );
    ASSERT( ppTexPaths != NULL );

    if( (NFiles <= 0) || (pGlobalSettings == NULL) || (ppTexSettings == NULL) || (ppTexPaths == NULL) )
    {
        STATUS::Print( STATUS::MSG_ERROR, "Corrupt parameters for export.\n" );
        return TB_RCODE_FAIL_ERROR;
    }

    // Allocate own copy of export data(for ease of use and thread safety)
    m_pPaths      = new TBlockPath[NFiles];
    m_pTBS        = new TBlockSettings[NFiles];
    m_pNameOffset = new TBNameOffset[NFiles];

    // Do some error checking
    ASSERT( m_pPaths != NULL );
    ASSERT( m_pTBS != NULL );
    ASSERT( m_pNameOffset != NULL );

    if( (m_pPaths == NULL) || (m_pTBS == NULL) || (m_pNameOffset == NULL) )
    {
        STATUS::Print( STATUS::MSG_ERROR, "Fatal memory allocation error!\n%s, line %d\n", __FILE__, __LINE__ );
        return TB_RCODE_FAIL_ERROR;
    }

    for( i = 0; i < NFiles; i++ )
    {
        if( m_bStopProcess )
        {
            // Operation has been cancelled, kill data and return
            STATUS::Print( STATUS::MSG_IMPORTANT, "Export cancelled on %s\n", m_Filename );
            this->m_bKillOnFinish = TRUE;
            return TB_RCODE_TERMINATED;
        }

        ASSERT( ppTexPaths[i] != NULL );
        ASSERT( ppTexSettings[i] != NULL );

        // Copy settings for this texture
        if( ppTexSettings[i] != NULL )
            x_memcpy( &m_pTBS[i], ppTexSettings[i], sizeof(TBlockSettings) );
        else
            m_pTBS[i].InitDefault();

        // Copy texture path
        if( ppTexPaths[i] != NULL )
            x_memcpy( &m_pPaths[i], ppTexPaths[i], sizeof(TBlockPath) );
        else
            TBLOCKPATH_InitDefault( m_pPaths[i] );

        // Generate short name and init offset
        TBLOCKPATH_GetShortName( m_pPaths[i], m_pNameOffset[i].m_ShortName );
        m_pNameOffset[i].m_Offset = -1;
    }

    // copy the global settings
    x_memcpy( &m_GlobalTBS, pGlobalSettings, sizeof(TBlockSettings) );

    // save number of files
    m_NFiles = NFiles;

    // export
    return ExportCurData( pExportFilename, Target, FinishCallback, cbParam );
}

//==========================================================================

s32 TBlockIMEXObj::ExportCurData( const char*   pExportFilename,
                                  TBLOCK_TARGET Target,
                                  TBExportCB    FinishCallback,
                                  u32           cbParam )
{
    // Check if export is in progress(can only do one at a time per instance)
    if( m_bInProgress )
        return TB_RCODE_INPROGRESS;

    // Make sure there is data to export
    if( (m_NFiles <= 0) || (m_pNameOffset == NULL) || (m_pPaths == NULL) || (m_pTBS == NULL) )
    {
        STATUS::Print( STATUS::MSG_ERROR, "No data to export.\n" );
        return TB_RCODE_FAIL_ERROR;
    }

    // Do some error checking
    TB_VERIFY( Target > TBLOCK_TGT_NOTSET );
    TB_VERIFY( Target < TBLOCK_TGT_COUNT );

    if( pExportFilename == NULL )
    {
        if( x_strlen( m_Filename ) <= 0 )
        {
            STATUS::Print( STATUS::MSG_ERROR, "No filename specified.\n" );
            return TB_RCODE_FAIL_ERROR;
        }

        pExportFilename = m_Filename;
    }

    if( (Target <= TBLOCK_TGT_NOTSET) || (Target >= TBLOCK_TGT_COUNT) )
    {
        STATUS::Print( STATUS::MSG_ERROR, "Invalid target specified.\n" );
        return TB_RCODE_FAIL_ERROR;
    }

    // Try to open the file for export
    char  ExtStr[X_MAX_EXT];
    char* pExt;

    x_splitpath( pExportFilename, NULL, NULL, NULL, ExtStr );

    if( ExtStr[0] == '.' )
        pExt = &ExtStr[1];
    else
        pExt = &ExtStr[0];

    // Determine if it is a text file or TBlock file
    if( !x_stricmp( pExt, TBLOCK_FILE_EXT_TXT ) )
    {
        m_pXFile = CreateTempExportFile( m_strTempFilename );
        m_bTextFile = TRUE;
    }
    else if( !x_stricmp( pExt, TBLOCK_FILE_EXT_TEX ) )
    {
        m_pXFile = CreateTempExportFile( m_strTempFilename );
        m_bTextFile = FALSE;
    }
    else
    {
        STATUS::Print( STATUS::MSG_ERROR, "Unrecognized file extension for export: %s.\n", pExt );
        return TB_RCODE_FILE_ERROR;
    }

    //ASSERT( m_pXFile != NULL );

    if( m_pXFile == NULL )
    {
        STATUS::Print( STATUS::MSG_ERROR, "Couldn't open/create TEMP file for export.\n" );
        return TB_RCODE_FILE_ERROR;
    }

    // save filename(if given, otherwise use current)
    if( pExportFilename != m_Filename )
        x_strncpy( m_Filename, pExportFilename, TBLOCK_PATH_LENGTH );

    // save export target
    m_Target = Target;

    // save callback function and parameter
    m_pFinishCallback = FinishCallback;
    m_cbParam = cbParam;

    // check for synchronous or async execution
    if( FinishCallback == NULL )
    {
        return ExecuteExport();
    }
    else
    {
        CWinThread* pThread;

        pThread = AfxBeginThread( TBlockIMEXObj::Thread_Export,
                                  (LPVOID)this,
                                  THREAD_PRIORITY_HIGHEST,
                                  0,
                                  CREATE_SUSPENDED,
                                  NULL );

        m_pThread = (void*)pThread;

        pThread->ResumeThread();

        return TB_RCODE_IMEX_STARTED;
    }
}

//==========================================================================

UINT TBlockIMEXObj::Thread_Export( LPVOID pParam )
{
    TBlockIMEXObj* pTHIS = (TBlockIMEXObj*)pParam;

    s32 RetCode = pTHIS->ExecuteExport();

    pTHIS->m_pThread = NULL;

    if( pTHIS->m_pFinishCallback( pTHIS->m_cbParam, pTHIS, RetCode ) )
    {
        if( pTHIS->m_bKillOnFinish )
            pTHIS->KillData();
    }

    return RetCode;
}

//==========================================================================

s32 TBlockIMEXObj::ExecuteExport( void )
{
    s32 RetCode;

    m_bInProgress = TRUE;

    if( m_bTextFile )
        RetCode = ExportTextFile();
    else
        RetCode = ExportTBlockFile();

    m_bInProgress = FALSE;

    x_fclose( m_pXFile );
    m_pXFile = NULL;

    if( RetCode != TB_RCODE_TERMINATED )
    {
        if( 0 == CopyFile( m_strTempFilename, m_Filename, FALSE ) )
        {
            //failure
            ASSERTS( FALSE, fs("Couldn't copy TEMP export file to %s", m_Filename) );
            STATUS::Print( STATUS::MSG_ERROR, "Couldn't copy TEMP export file to %s\n", m_Filename );
        }
    }

    if( 0 == DeleteFile( m_strTempFilename ) )
    {
        //couldn't delete temp file
        STATUS::Print( STATUS::MSG_WARNING, "Couldn't delte TEMP export file: %s\n", m_strTempFilename );
    }

    return RetCode;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// TEXT FILE EXPORT
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

s32 TBlockIMEXObj::ExportTextFile( void )
{
    s32 i;
    s32 RetCode;

    char TextLine[ TBLOCK_PATH_LENGTH + TBCMDLINE::MAX_LENGTH ];

    x_strncpy( TextLine, TBLOCK_GLOBAL_PATHNAME, TBLOCK_PATH_LENGTH );
    TBCMDLINE::Generate( TextLine, m_GlobalTBS );
    x_fprintf( m_pXFile, "%s\n", TextLine );

    for( i = 0; i < m_NFiles; i++ )
    {
        if( m_bStopProcess )
        {
            // Operation has been cancelled, kill data and return
            STATUS::Print( STATUS::MSG_IMPORTANT, "Export cancelled on %s\n", m_Filename );
            this->m_bKillOnFinish = TRUE;
            return TB_RCODE_TERMINATED;
        }

        x_strncpy( TextLine, m_pPaths[i], TBLOCK_PATH_LENGTH );

        //RetCode = TBCMDLINE::Generate( TextLine, m_pTBS[i] );
        RetCode = TBCMDLINE::Generate( TextLine, m_pTBS[i], m_GlobalTBS );

        x_fprintf( m_pXFile, "%s\n", TextLine );
    }

    return TB_RCODE_OK;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// TBLOCK FILE EXPORT
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

s32 TBlockIMEXObj::ExportTBlockFile( void )
{
    s32          i;
    s32          BMPSize;
    s32          BMPFilePosStart;
    x_bitmap     xBMP;
    TBlockHeader TBHdr;

    // Start progress
    STATUS::ProgressStart( m_NFiles );

    if( m_bDebugTintMips )
        STATUS::Print( STATUS::MSG_IMPORTANT, "MIP tinting enabled for export.\n" );

    // Setup header file string
    x_strcpy( TBHdr.m_FileStr, TBLOCK_HEADER_FILESTR );

    if( IsTgtBigEndian() )
        TBHdr.m_FileStr[TBLOCK_ENDIAN_INDEX] = TBLOCK_ENDIAN_BIG;
    else
        TBHdr.m_FileStr[TBLOCK_ENDIAN_INDEX] = TBLOCK_ENDIAN_LITTLE;

    // Init header data
    TBHdr.m_NFiles       = m_NFiles;
    TBHdr.m_TgtPlatform  = (s32)m_Target;
    TBHdr.m_TotalBMPSize = 0x00000000;
    TBHdr.m_MinBMPSize   = 0x7FFFFFFF;
    TBHdr.m_MaxBMPSize   = 0x00000000;
    TBHdr.m_BMPDataEnd   = 0x00000000;

    // Write header to file(it will be overwritten later)
    x_fwrite( &TBHdr, sizeof(TBlockHeader), 1, m_pXFile );

	// Ensure that the short names for the textures are generated
	for( i = 0; i < m_NFiles; i++ )
	{
		TBLOCKPATH_GetShortName( m_pPaths[i], m_pNameOffset[i].m_ShortName );
	}

    // Write short names + offsets to file(it will be overwritten later with new offsets)
    x_fwrite( m_pNameOffset, sizeof(TBNameOffset), m_NFiles, m_pXFile );

    BMPFilePosStart = x_ftell( m_pXFile );

    // loop through each texture, process them and write to file
    for( i = 0; i < m_NFiles; i++ )
    {
        if( m_bStopProcess )
        {
            // Operation has been cancelled, kill data and return
            STATUS::Print( STATUS::MSG_IMPORTANT, "Export cancelled on %s\n", m_Filename );
            STATUS::ProgressStart( 0 );
            this->m_bKillOnFinish = TRUE;
            return TB_RCODE_TERMINATED;
        }

        TBlockSettings* pCurTBS;

        // Check if texture uses global or own settings
        if( m_pTBS[i].GetUseGlobal() )
            pCurTBS = &m_GlobalTBS;
        else
            pCurTBS = &m_pTBS[i];

        // Generate texture data
        XBMP_GenerateTBlockBMP( xBMP, m_Target, m_pPaths[i], pCurTBS, m_bDebugTintMips );

        // Update header info for BMP size
        BMPSize = xBMP.GetSerializedSize();

        if( BMPSize < TBHdr.m_MinBMPSize )
            TBHdr.m_MinBMPSize = BMPSize;
        if( BMPSize > TBHdr.m_MaxBMPSize )
            TBHdr.m_MaxBMPSize = BMPSize;

        // get file offset to BMP
        if( IsTgtBigEndian() )
            m_pNameOffset[i].m_Offset = (u32)ENDIAN_SWAP_32( x_ftell( m_pXFile ) );
        else
            m_pNameOffset[i].m_Offset = (u32)x_ftell( m_pXFile );

        if( m_bStopProcess )
        {
            // Operation has been cancelled, kill data and return
            STATUS::Print( STATUS::MSG_IMPORTANT, "Export cancelled on %s\n", m_Filename );
            STATUS::ProgressStart( 0 );
            this->m_bKillOnFinish = TRUE;
            return TB_RCODE_TERMINATED;
        }

        // Save the BMP and then destroy it
        TBlock_WriteBMP( xBMP, m_Target, m_pXFile );
        xBMP.KillBitmap();

        STATUS::ProgressStep( 1 );
    }

    // get end point of BMP data
    TBHdr.m_BMPDataEnd = (u32)x_ftell( m_pXFile );

    TBHdr.m_TotalBMPSize = TBHdr.m_BMPDataEnd - BMPFilePosStart;

    if( IsTgtBigEndian() )
        EndianSwapTBlockHdr( TBHdr );

    // Save BMP settings, global settings, and BMP paths to end of file
    // so it can be imported back later
    x_fwrite( m_pTBS,       sizeof(TBlockSettings), m_NFiles, m_pXFile );
    x_fwrite( &m_GlobalTBS, sizeof(TBlockSettings), 1,        m_pXFile );
    x_fwrite( m_pPaths,     sizeof(TBlockPath),     m_NFiles, m_pXFile );

    // Seek to start of file to overwrite header and shortname-offset data
    x_fseek( m_pXFile, 0, X_SEEK_SET );

    // Overwrite updated header
    x_fwrite( &TBHdr, sizeof(TBlockHeader), 1, m_pXFile );

    // Overwrite shortnames-offsets with new updated offsets
    x_fwrite( m_pNameOffset, sizeof(TBNameOffset), m_NFiles, m_pXFile );

    x_fseek( m_pXFile, 0, X_SEEK_END );

    STATUS::ProgressStart( 0 );

    STATUS::Print( STATUS::MSG_IMPORTANT, "Exported %s\n", m_Filename );

    return TB_RCODE_OK;
}

//==========================================================================

#pragma pack(1)

struct TGAHEADER
{
    u8      IdLength;       // Image ID Field Length 
    u8      CmapType;       // Color Map Type 
    u8      ImageType;      // Image Type 

    u16     CmapIndex;      // First Entry Index 
    u16     CmapLength;     // Color Map Length 
    u8      CmapEntrySize;  // Color Map Entry Size 

    u16     X_Origin;       // X-origin of Image 
    u16     Y_Origin;       // Y-origin of Image 
    u16     ImageWidth;     // Image Width 
    u16     ImageHeight;    // Image Height 
    u8      PixelDepth;     // Pixel Depth 
    u8      ImagDesc;       // Image Descriptor 
};

#pragma pack()

//--------------------------------------------------------------------------
static void WriteTGA32( x_bitmap& xBMP, const char* pFilename, xbool bAppendName = TRUE );

static void WriteTGA32FromPS2Compressed( x_bitmap &xBMP, const char *pFilename )
{
    char    pFullPath[X_MAX_PATH];
    char    pDrive[X_MAX_DRIVE];
    char    pDir[X_MAX_DIR];
    char    pFName[X_MAX_FNAME];
    char    pFNameMip[X_MAX_FNAME];
    char    pExt[X_MAX_EXT];

    x_splitpath( pFilename, pDrive, pDir, pFName, pExt );

    s32 i;
    s32 nCount = xBMP.GetNMips();

    for (i=0; i<=nCount; ++i)
    {
        s32 width = xBMP.GetWidth( i );
        s32 height = xBMP.GetHeight( i );
        s32 size = xBMP.GetDataSize( i );
        s32 n8BitSize = width * height;
        if (size == n8BitSize)
        {
            byte *pDataBuffer = xBMP.GetDataPtr( i );
            byte *pClutBuffer = xBMP.GetClutPtr();
            
            if (pDataBuffer && pClutBuffer )
            {
                x_bitmap CombinedMap;
                CombinedMap.SetupBitmap( x_bitmap::FMT_P8_ABGR_8888, 
                                        xBMP.GetWidth( i ),
                                        xBMP.GetHeight( i ),
                                        xBMP.GetPWidth( i ),
                                        xBMP.GetPHeight( i ),
                                        FALSE,
                                        (xBMP.GetWidth( i ))*(xBMP.GetHeight( i )) * 3,
                                        pDataBuffer,
                                        FALSE,
                                        0,
                                        pClutBuffer,
                                        256,
                                        0,
                                        FALSE );

                x_sprintf( pFNameMip, "%s_mip%ld", pFName, i );
                x_makepath( pFullPath, pDrive, pDir, pFNameMip, pExt );

                WriteTGA32( CombinedMap, pFullPath, FALSE );
            }
            continue;
        }
        byte *pDataBuffer = xBMP.GetDataPtr( i );
        byte *pColorBuffer = pDataBuffer;
        byte *CLUT = xBMP.GetClutPtr();
        s32 ColorBufferSize = ((xBMP.GetWidth( i ) >> 1) * (xBMP.GetHeight( i ) >> 1));
        s32 IntensitySetupSize = (sizeof( u64 ) * 2) + (sizeof( u64 ) * 12);
        byte *pIntensityBuffer = pColorBuffer + ColorBufferSize + IntensitySetupSize;

        byte *pCombinedBuffer = (byte*)new byte[ (xBMP.GetWidth( i ) * xBMP.GetHeight( i ) * 3) ];
        byte *pCombinedStep = pCombinedBuffer;

        s32 n4BitHeight = xBMP.GetHeight( i );
        s32 n4BitWidth = xBMP.GetWidth( i );

        width = xBMP.GetWidth( i ) >> 1;
        s32 wstep;
        s32 hstep;
        for (hstep=0; hstep<n4BitHeight; ++hstep)
        {
            for (wstep=0; wstep<n4BitWidth; ++wstep)
            {
                // load a color as float
                s32 iIndex = (*(pColorBuffer + (wstep>>1) + ((hstep>>1) * width)));
                s32 palR = CLUT[ (iIndex * 4) + 0 ];
                s32 palG = CLUT[ (iIndex * 4) + 1 ];
                s32 palB = CLUT[ (iIndex * 4) + 2 ];
                f32 fR = (f32)palR;
                f32 fG = (f32)palG;
                f32 fB = (f32)palB;

                // convert to parametric
                fR /= 255.0f;
                fG /= 255.0f;
                fB /= 255.0f;

                // load an intensity
                s32 Lookup;
                byte *pIntense = (pIntensityBuffer + (wstep>>1) + ((hstep * width)));

                if (wstep&0x1)  // lower
                    Lookup = ((*pIntense) & 0x0F);      // grab the lower 4-bits
                else        // upper
                    Lookup = (((*pIntense)>>4)&0x0F);    // grab the upper 4-bits

                s32 IntensityR = CLUT[ (Lookup * 4) + 0 ];
                s32 IntensityG = CLUT[ (Lookup * 4) + 1 ];
                s32 IntensityB = CLUT[ (Lookup * 4) + 2 ];
                IntensityR &= 0x000000FF;
                IntensityG &= 0x000000FF;
                IntensityB &= 0x000000FF;
                // convert the intensity to parametric float
                f32 fIntensityR = ((f32)IntensityR) / 255.0f;
                f32 fIntensityG = ((f32)IntensityG) / 255.0f;
                f32 fIntensityB = ((f32)IntensityB) / 255.0f;

                // combine the two and convert to integer
                s32 R = (s32)((fR * fIntensityR) * 255.0f);
                s32 G = (s32)((fG * fIntensityG) * 255.0f);
                s32 B = (s32)((fB * fIntensityB) * 255.0f);

                // store the new color
                *(pCombinedStep + 0) = R;
                *(pCombinedStep + 1) = G;
                *(pCombinedStep + 2) = B;

                // step the output ptr
                pCombinedStep += 3;
            }
        }
        x_bitmap CombinedMap;
        CombinedMap.SetupBitmap( x_bitmap::FMT_24_RGB_888, 
                                    xBMP.GetWidth( i ),
                                    xBMP.GetHeight( i ),
                                    xBMP.GetPWidth( i ),
                                    xBMP.GetPHeight( i ),
                                    FALSE,
                                    (xBMP.GetWidth( i ))*(xBMP.GetHeight( i )) * 3,
                                    pCombinedBuffer,
                                    FALSE,
                                    0,
                                    NULL,
                                    0,
                                    0,
                                    FALSE );

        x_sprintf( pFNameMip, "%s_mip%ld", pFName, i );
        x_makepath( pFullPath, pDrive, pDir, pFNameMip, pExt );

        WriteTGA32( CombinedMap, pFullPath, FALSE );
        if (pCombinedBuffer)
            delete [] pCombinedBuffer;
        pCombinedBuffer = NULL;
    }
}

static void WriteTGA32( x_bitmap& xBMP, const char* pFilename, xbool bAppendName  )
{
    char    pFullPath[X_MAX_PATH];
    char    pDrive[X_MAX_DRIVE];
    char    pDir[X_MAX_DIR];
    char    pFName[X_MAX_FNAME];
    char    pFNameMip[X_MAX_FNAME];
    char    pExt[X_MAX_EXT];
    X_FILE* pFile;
    s32     NMips;
    s32     i;
    s32     X, Y, W, H;
    color   PixelColor;
    TGAHEADER TgaHdr;

    if (xBMP.IsPS2Compressed())
    {
        WriteTGA32FromPS2Compressed( xBMP, pFilename );
        return;
    }

    x_memset( &TgaHdr, 0, sizeof(TGAHEADER) );

    TgaHdr.ImageType  = 2;      // uncompressed rgb data
    TgaHdr.ImagDesc   = 8;      // image is flipped, or ???
    TgaHdr.PixelDepth = 32;     // 32 bpp, RGBA (actually BGRA)

    x_splitpath( pFilename, pDrive, pDir, pFName, pExt );

    NMips = xBMP.GetNMips();

    // loop through each mip level to write out to a file
    for( i = 0; i <= NMips; i++ )
    {
        W = xBMP.GetWidth( i );
        H = xBMP.GetHeight( i );

        TgaHdr.ImageWidth  = W;
        TgaHdr.ImageHeight = H;

        // make filename, including mip level number
        if (bAppendName)
            x_sprintf( pFNameMip, "%s_mip%ld", pFName, i );
        else
            x_sprintf( pFNameMip, "%s", pFName );
        x_makepath( pFullPath, pDrive, pDir, pFNameMip, pExt );

        STATUS::Print( STATUS::MSG_NORMAL, "Writing out %s.\n", pFullPath );

        pFile = x_fopen( pFullPath, "wb" );

        // write tga header to file
        x_fwrite( &TgaHdr, sizeof(TGAHEADER), 1, pFile );

        x_bitmap::SetPixelFormat( xBMP, 0, i );

        // get each pixel color, and write it to file
        // image is stored bottom-up, so last line is stored first
        for( Y = H-1; Y >= 0; Y-- )
        {
            for( X = 0; X < W; X++ )
            {
                PixelColor = x_bitmap::GetPixelColor( X, Y, i );

                // need to reverse the R and B components
                PixelColor.Set( PixelColor.B, PixelColor.G, PixelColor.R, PixelColor.A );

                x_fwrite( &PixelColor.R, sizeof(u8), 4, pFile );
            }
        }

        x_fclose( pFile );
    }
}

//==========================================================================

s32 TBlockIMEXObj::TestExport( const char*      pExportFilename,
                               TBLOCK_TARGET    Target,
                               TBlockSettings*  pTexSettings,
                               TBlockPath&      rTexPath )
{
    x_bitmap       xBMP;
    TBlockSettings xBMPSettings;

    if( (pExportFilename == NULL) || (pTexSettings == NULL) )
        return TB_RCODE_FAIL_ERROR;

    STATUS::Print( STATUS::MSG_NORMAL, "Generating texture for %s.\n", rTexPath );

    // turn off swizzle option so bitmap data can be read for file output
    x_memcpy( &xBMPSettings, pTexSettings, sizeof(TBlockSettings) );

    xBMPSettings.SetSwizzle( FALSE );

    // create texture
    XBMP_GenerateTBlockBMP( xBMP, Target, rTexPath, &xBMPSettings, FALSE );

    // write to file
    WriteTGA32( xBMP, pExportFilename );

    STATUS::Print( STATUS::MSG_IMPORTANT, "Export complete.\n", rTexPath );

    xBMP.KillBitmap();

    return TB_RCODE_OK;
}

//==========================================================================
