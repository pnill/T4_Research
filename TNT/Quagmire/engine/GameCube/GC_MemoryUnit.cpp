//=====================================================================================================================================
// Includes
//=====================================================================================================================================
#include "GC_MemoryUnit.hpp"
#include "AUX_Bitmap.hpp"


//=====================================================================================================================================
// Forward Declarations
//=====================================================================================================================================


//=====================================================================================================================================
// Definitions
//=====================================================================================================================================
#ifdef X_DEBUG
    #define SHOW_GCMU_DIAGNOSTICS
#endif

#ifdef X_DEBUG
    #define USE_SYNCHRONOUS_FUNCTIONS
#endif


#define GCMU_FILE_INFO_BROWSER_COMMENT_OFFSET   (0)
#define GCMU_FILE_INFO_ICON_DATA_OFFSET         (64)
#define GCMU_MAX_FREE_FILES_VALUE               (127)
#define GCMC_SYSTEM_SECTORS_WASTED_FOR_OVERHEAD (5)

#define GCMU_MIN_SAVED_GAME_TIME                2.0f
#define GCMU_MIN_DELETE_TIME                    2.0f
#define GCMU_MIN_LOAD_GAME_TIME                 1.5f


#define NUMBER_OF_BYTES_IN_A_MEGA_BIT_NUMBER    (1024 * 1024 / 8)

//-------------------------------------------------------------------------------------------------------------------------------------
enum GameCube_LocalMemoryCardCommands
{
    GCMU_CMD_FIRST = 0,
    GCMU_CMD_NONE = GCMU_CMD_FIRST,
    GCMU_CMD_MOUNT,
    GCMU_CMD_FORMAT,
    GCMU_CMD_SAVE,
    GCMU_CMD_LOAD,
    GCMU_CMD_DELETE,
    GCMU_CMD_LAST = GCMU_CMD_DELETE,
};

//-------------------------------------------------------------------------------------------------------------------------------------
enum GameCubeFileTransferStatus
{
    FILE_TRANSFER_STATUS_READ,
    FILE_TRANSFER_STATUS_WAIT_FOR_READ_TO_COMPLETE,
    FILE_TRANSFER_STATUS_WRITE,
    FILE_TRANSFER_STATUS_WAIT_FOR_WRITE_TO_COMPLETE,
    TOTAL_FILE_TRANSFER_STATES,
};

//=====================================================================================================================================
// Globals
//=====================================================================================================================================
GCMemoryUnitManager* gpGCMemoryUnitManager;


//=====================================================================================================================================
// Local Functions
//=====================================================================================================================================
#ifdef SHOW_GCMU_DIAGNOSTICS
    void MC_PRINT( char* FormatString, ... )
    {
        static char TempBuffer[300];

        x_va_list   Args;
        x_va_start( Args, FormatString );

        x_vsprintf( TempBuffer, FormatString, Args );

        x_printf( "[GCMU] --> " );
        x_printf( TempBuffer );
        x_printf( "\n" );
    }
#else
    void MC_PRINT( char* FormatString, ... )
    {
    }
#endif


//=====================================================================================================================================
// MEMORY_UNIT Interface functions.
//=====================================================================================================================================
xbool MEMORY_UNIT_Create( char* ApplicationName )
{
    // Create the manager.
    gpGCMemoryUnitManager = new GCMemoryUnitManager( ApplicationName );

    if( gpGCMemoryUnitManager )
    {
        return TRUE;
    }
    else
    {
        ASSERT( 0 );
        return FALSE;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool MEMORY_UNIT_Initialize( char* MemoryUnitResourceFilename )
{
    // Initialize all of the date for the memory unit manager.
    if( gpGCMemoryUnitManager->Initialize( MemoryUnitResourceFilename ) )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
void MEMORY_UNIT_DeInitialize( void )
{
    if( gpGCMemoryUnitManager )
    {
        gpGCMemoryUnitManager->DeInitialize( );
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
void MEMORY_UNIT_Destroy( void )
{
    if( gpGCMemoryUnitManager )
    {
        delete gpGCMemoryUnitManager;
        gpGCMemoryUnitManager = NULL;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
void MEMORY_UNIT_Update( void )
{
    // As long as there is a memory unit created, and scanning is enabled update the stats of the memory units.
    if( gpGCMemoryUnitManager && gpGCMemoryUnitManager->IsScanningEnabled( ) )
    {
        gpGCMemoryUnitManager->Update( );
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
void MEMORY_UNIT_EnableScanning( void )
{
    gpGCMemoryUnitManager->EnableScanning( );
}


//-------------------------------------------------------------------------------------------------------------------------------------
void MEMORY_UNIT_DisableScanning( void )
{
    gpGCMemoryUnitManager->DisableScanning( );
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool MEMORY_UNIT_IsWriteInProgress( void )
{
    return gpGCMemoryUnitManager->IsWriteInProgress( );
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool MEMORY_UNIT_IsConnected( s32 MemoryUnitID )
{
    if( MemoryUnitID == -1 )
        return gpGCMemoryUnitManager->IsActiveMemoryUnitStillConnected( );
    else
        return gpGCMemoryUnitManager->IsMemoryUnitConnected( MemoryUnitID );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_GetSaveGameUserDataSizeInBytes( s32 SaveGameType )
{
    return gpGCMemoryUnitManager->GetUserDataByteSize( SaveGameType );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_SelectActiveMemoryUnit( s32 MemoryUnitID, byte* pFileIOBuffer, s32 BufferSize )
{
    return gpGCMemoryUnitManager->SelectActiveMemoryUnit( MemoryUnitID, pFileIOBuffer, BufferSize );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_ReleaseActiveMemoryUnit( void )
{    
    return gpGCMemoryUnitManager->ReleaseActiveMemoryUnit( );
}


//-------------------------------------------------------------------------------------------------------------------------------------
char* MEMORY_UNIT_GetNameOfActiveMemoryUnit( void )
{
    return gpGCMemoryUnitManager->GetNameOfActiveMemoryUnit( );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_GetFreeBlocks( void )
{
    return gpGCMemoryUnitManager->GetFreeSpaceRemainingOnActiveUnit( );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_GetFreeFiles( void )
{
    return gpGCMemoryUnitManager->GetFreeFilesRemainingOnActiveUnit( );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_GetRequiredBlocksFreeToSaveGameType( s32 SaveGameType, s32* pRequiredBlocks, xbool bWorstCase )
{
    ASSERT( pRequiredBlocks );
    return gpGCMemoryUnitManager->CalculateRequiredSizeForSaveGameType( SaveGameType, pRequiredBlocks, bWorstCase );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_Format( void )
{
    return gpGCMemoryUnitManager->FormatCard( );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_SaveGame( s32 SaveGameType, s32 TypeOffset, void* pUserData, char* SaveGameBrowserInfo, s32 BrowserInfoBreakOffset )
{
    // BrowserInfoBreakOffset is not used on the gamecube.
    return gpGCMemoryUnitManager->SaveGame( SaveGameType, TypeOffset, pUserData, SaveGameBrowserInfo );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_LoadGame( s32 SaveGameType, s32 TypeOffset, void* pUserData )
{   
    return gpGCMemoryUnitManager->LoadGame( SaveGameType, TypeOffset, pUserData );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_DeleteGame( s32 SaveGameType, s32 TypeOffset )
{   
    return gpGCMemoryUnitManager->DeleteGame( SaveGameType, TypeOffset );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_GetSaveGameList( s32 SaveGameType, s32* pMaxSaveGameTypeCount, SingleSavedGameInfo** pSaveGameListings )
{   
    return gpGCMemoryUnitManager->GetSaveGameList( SaveGameType, pMaxSaveGameTypeCount, pSaveGameListings );
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool MEMORY_UNIT_GetNextEvent( MemoryUnitEvent* pEvent )
{   
    ASSERT( pEvent );

    if( gpGCMemoryUnitManager->IsCommandComplete( ) )
    {
        // Do a cheap wrap here.  If the card isn't connected, always return the NO card error.
        if( MEMORY_UNIT_IsConnected( ) )
        {
            gpGCMemoryUnitManager->GetLastCommandValues( &pEvent->mReturnValue, &pEvent->mErrorCode );

            if( pEvent->mErrorCode == QUAG_MU_EVERYTHING_IS_OK )
                pEvent->mEventType = QUAG_MU_EVENT_COMPLETE;
            else
                pEvent->mEventType = QUAG_MU_EVENT_FAILED;
        }
        else
        {
            gpGCMemoryUnitManager->GetLastCommandValues( NULL, NULL );
            pEvent->mEventType   = QUAG_MU_EVENT_FAILED;
            pEvent->mReturnValue = FALSE;
            pEvent->mErrorCode   = QUAG_MU_NO_CARD;
        }

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
xbool MEMORY_UNIT_GetGenericMemoryCardName( s32 QuagMemCardID, char* pBuffer, s32 SizeOfBuffer )
{
    ASSERT( QuagMemCardID <= QUAG_MEMORY_UNIT_MAX_ID );

    return gpGCMemoryUnitManager->GenerateCardName( QuagMemCardID, pBuffer, SizeOfBuffer );
}

//=====================================================================================================================================
// GCConsoleIconData
//=====================================================================================================================================
GCConsoleIconData::GCConsoleIconData( void )
{
    mpIconData          = NULL;
    mIconDataSize       = 0;
    mIconAnimSpeed      = GC_MEMCARD_ANIMATE_FRAME_NORMAL;
    mNumberOfIconFrames = 0;
}


//-------------------------------------------------------------------------------------------------------------------------------------
GCConsoleIconData::~GCConsoleIconData( void )
{
    ReleaseIconData( );
}


//-------------------------------------------------------------------------------------------------------------------------------------
void GCConsoleIconData::ReleaseIconData( void )
{
    if( mpIconData )
    {
        delete[] mpIconData;
        mpIconData = NULL;

        mIconDataSize = 0;
    }
}



//=====================================================================================================================================
// GCMemoryUnit
//=====================================================================================================================================
GCMemoryUnit::GCMemoryUnit( void )
{
    mUnitID = -1;
    Reset( );
}


//-------------------------------------------------------------------------------------------------------------------------------------
void GCMemoryUnit::Initialize( s32 UnitID )
{
    mUnitID = UnitID;

    // Create the name for this unit.
    x_sprintf( mName, "Memory Card in Slot %c", UnitID == QUAG_MEMORY_UNIT_ID_0 ? 'A' : 'B' );
}


//-------------------------------------------------------------------------------------------------------------------------------------
void GCMemoryUnit::Reset( void )
{
    mbConnected = FALSE;
    mbFormatted = FALSE;
    mbMounted   = FALSE;

    mTotalSize   = 0;
    mSectorSize  = 0;
    mFreeBytes   = 0;
    mFreeBlocks  = 0;
    mFreeFiles   = 0;
}


//=====================================================================================================================================
// GCMU_FileInfo
//=====================================================================================================================================
GCMU_FileInfo::GCMU_FileInfo( void )
{
    // Allocate the space required for data buffering on loads and saves.
    mpSectorDataBuffer = (byte*)x_malloc( sizeof( byte ) * GCMU_SUPPORTED_SECTOR_SIZE );    ASSERT( mpSectorDataBuffer );
}


//-------------------------------------------------------------------------------------------------------------------------------------
GCMU_FileInfo::~GCMU_FileInfo( void )
{
    if( mpSectorDataBuffer ) { x_free( mpSectorDataBuffer ); mpSectorDataBuffer = NULL; }
}

    
//-------------------------------------------------------------------------------------------------------------------------------------
void GCMU_FileInfo::Reset( xbool LeaveUserBufferAlone )
{
    mFileName[0]     = 0;

    mbOpen           = FALSE;
    mbWriteAccess    = FALSE;
    
    mTotalFileSize   = 0;
    mFileDataSize    = 0;

    if( LeaveUserBufferAlone == FALSE )
    {
        mpUserDataBuffer = NULL;
        mUserBufferSize  = 0;
    }
}


//=====================================================================================================================================
// GameCube Memory Unit System Memory card friendly functions.
//=====================================================================================================================================
void GCMUCallBack_CardDisconnected( s32 Slot, s32 Result )
{
    ASSERT( Slot <= QUAG_MEMORY_UNIT_MAX_ID );
    GCMemoryUnit* pMemoryUnit;

    pMemoryUnit = &gpGCMemoryUnitManager->mGCMemoryUnits[gpGCMemoryUnitManager->GetMemoryUnitIDFromSlot( Slot )];

    // Let the system know that the card has disconnected.
    gpGCMemoryUnitManager->ProcessCardDisconnected( pMemoryUnit );
}


//=====================================================================================================================================
// GCMemoryUnitManager
//=====================================================================================================================================
GCMemoryUnitManager::GCMemoryUnitManager( char* ApplicationName )
{
    MC_PRINT( "Constructing" );

    // Initlize the GameCube OS memory card system.
    CARDInit();

    // Initialize some known data about the GameCube Memory Units.
    mGCMemoryUnits[QUAG_MEMORY_UNIT_ID_0].Initialize( QUAG_MEMORY_UNIT_ID_0 );

    // Test to see if we are supporting both slot A and B before initializing a variable that may
    // not be there.
    if( QUAG_MEMORY_UNIT_MAX_ID > QUAG_MEMORY_UNIT_ID_0 )
        mGCMemoryUnits[QUAG_MEMORY_UNIT_ID_1].Initialize( QUAG_MEMORY_UNIT_ID_1 );

    // Reset the pointer to the mounting workspace area, since we don't allocate it until the memory
    // card system is initialized.
    mpMountingWorkArea = NULL;

    ResetCommandHandlingVariables( );

    // Initialize the save game info to defaults until the Memory unit resource file is loaded.
    mNumberOfSaveGameTypes = 0;
    mpSaveGameTypes        = NULL;

    mpAvailableSavedGames  = NULL;
    mCurrentGameType       = 0;
    mCurrentGameTypeIndex  = 0;
    
    // Initialize the title name.
    x_strncpy( mBrowserTitleText, ApplicationName, MEMORY_UNIT_SAVE_GAME_TITLE_LENGTH );

    // Initialize the rest of the data.
    SetBrowserText( "Browser Comment" ); 

    // By default turn off scanning.
    DisableScanning( );

	// Initialize that no saves are in progress.
	mbSavingInProgress = FALSE;

    // Technically the memory unit manager is built, but not initialized yet to actually do anything.
    mInitialized = FALSE;
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool GCMemoryUnitManager::Initialize( char* MemoryUnitResourceFilename )
{
    xbool               bSuccess;
    GCMUResourceHeader  Header;
    X_FILE*             pFile;
    byte*               pTempBuffer;
    s32                 ReadCount;

    GCMUResourceIconInfo*       pResourceIconInfo;
    GCMUResourceSaveGameType*   pResourceSaveGameTypes;
    byte*                       pRawIconData;

    pTempBuffer            = NULL;
    pResourceIconInfo      = NULL;
    pResourceSaveGameTypes = NULL;
    pRawIconData           = NULL;
    bSuccess               = FALSE;

    if( !mInitialized )
    {
        //-------------------------------------------------------------------------------------------------------------------------------------
        MC_PRINT( "Initializing" );

        ResetCommandHandlingVariables( );

        // Allocate the workspace for the mounting of a GameCube memory card.
        if( mpMountingWorkArea == NULL )
        {
            mpMountingWorkArea = x_malloc( sizeof( char ) * CARD_WORKAREA_SIZE );
            ASSERT( mpMountingWorkArea );
        }
        else
        {
            // The mounting work space area should not be allocated already, something is wrong.
            ASSERT( 0 );
        }

        // Get all of the information required to save a game on this system by loading in the instructional resource
        // file.  This file will contain all information about how saved games must be built on this platform, as well
        // the data for an icon, and any information regarding how to draw that icon is also provided.
        pFile = x_fopen( MemoryUnitResourceFilename, "rb" );

        if( pFile )
        {
            ReadCount = x_fread( &Header, sizeof( GCMUResourceHeader ), 1, pFile );

            if( ReadCount == 1 )
            {
                // Test to make sure that it's a PS2 resource file.
                if( Header.mResourceType == QUAG_MEMORY_UNIT_RESOURCE_TYPE_GAMECUBE )
                {
                    // Determine how big the rest of the file is and load the entire thing.
                    pTempBuffer = new byte[Header.mSizeOfEntireResourceFile];
                    ASSERT( pTempBuffer );

                    // Seek back to the beginning of the file and read the entire thing.
                    x_fseek( pFile, 0, X_SEEK_SET );
                    ReadCount = x_fread( pTempBuffer, Header.mSizeOfEntireResourceFile, 1, pFile );

                    // If the entire file is read, then we have it all stored locally now.
                    if( ReadCount == 1 )
                    {
                        // Retrieve the save game information from the resource file.
                        //-------------------------------------------------------------------------------------------------------------------------------------
                        mNumberOfSaveGameTypes = Header.mNumberOfSaveTypes;
                    
                        mpSaveGameTypes = new GCMUResourceSaveGameType[mNumberOfSaveGameTypes];
                        ASSERT( mpSaveGameTypes );

                        // Read in data for the save game types.
                        pResourceSaveGameTypes = (GCMUResourceSaveGameType*)(pTempBuffer + Header.mOffsetToSaveTypes);
                        x_memcpy( mpSaveGameTypes, pResourceSaveGameTypes, sizeof( GCMUResourceSaveGameType ) * mNumberOfSaveGameTypes );

                        // Also initialize the Available saved games list
                        mpAvailableSavedGames = new SaveGameTypeAvailableList[mNumberOfSaveGameTypes];
                        ASSERT( mpAvailableSavedGames );
                        ClearAvailableGamesList( );


                        // Retrieve the icon information for this game from the resource file.
                        //-------------------------------------------------------------------------------------------------------------------------------------
                        pResourceIconInfo = (GCMUResourceIconInfo*)(pTempBuffer + Header.mOffsetToIconInfo);
                        pRawIconData      = pTempBuffer + Header.mOffsetToIconData;
                        BuildIconFromResourceData( pResourceIconInfo, pRawIconData );

                        bSuccess = TRUE;
                    }
                    else
                    {
                        MC_PRINT( "Couldn't read entire resource file" );
                    }

                }
                else
                {
                    MC_PRINT( "Resource File is not for GameCube" );
                }
            }
            else
            {
                MC_PRINT( "Couldn't read resource file header" );
            }
        }
        else
        {
            MC_PRINT( "Couldn't open the resource file %s", MemoryUnitResourceFilename );
        }

        // Release any dynamic memory.
        if( pFile )
            x_fclose( pFile );

        if( pTempBuffer )
        {
            delete[] pTempBuffer;
            pTempBuffer = NULL;
        }

        // The Memory Unit manager is ready to go.
        if( bSuccess )
        {
            MC_PRINT( "Initialization Successful" );
            mInitialized = TRUE;
        }
        else
        {
            MC_PRINT( "Initialization Failed" );
        }
    }
    else
    {
        //-------------------------------------------------------------------------------------------------------------------------------------
        MC_PRINT( "Already Initialized, Ignoring Initialize request." );
        bSuccess = TRUE;
    }

    return bSuccess;
}

//-------------------------------------------------------------------------------------------------------------------------------------
void GCMemoryUnitManager::DeInitialize( void )
{
    if( mInitialized )
    {
        // When we deinitialize, there had better not be a memory unit command in progress.
        ASSERT( !IsCommandInProgress( ) );

        MC_PRINT( "De-Initializing" );

        // Delete the save games types list.
        if( mpSaveGameTypes )
        {
            delete[] mpSaveGameTypes;
            mpSaveGameTypes = NULL;

            mNumberOfSaveGameTypes = 0;
        };

        // Delete the menu of saved games.
        if( mpAvailableSavedGames )
        {
            delete[] mpAvailableSavedGames;
            mpAvailableSavedGames = NULL;
        }

        // Free up the mounted card area.
        if( mpMountingWorkArea )
        { 
            x_free( mpMountingWorkArea );
            mpMountingWorkArea = NULL;
        }

        mIconData.ReleaseIconData( );

        // Identify that the memory manager is not ready for use anymore.
        mInitialized = FALSE;
    }
    else
    {
        MC_PRINT( "System wasn't intialized, de-intialize request ignored." );
    }

}

//-------------------------------------------------------------------------------------------------------------------------------------
GCMemoryUnitManager::~GCMemoryUnitManager( void )
{
    DeInitialize( );
    MC_PRINT( "Destroying" );
}


//-------------------------------------------------------------------------------------------------------------------------------------
// Update Functions.
//-------------------------------------------------------------------------------------------------------------------------------------
void GCMemoryUnitManager::Update( void )
{
    s32           RetVal;
    u32           MemoryUnitOffset;
    GCMemoryUnit* pCurrentMemUnit;
    s32           Slot;

    if( mInitialized )
    {
        if( IsCommandInProgress( ) == FALSE )
        {
            // If there is no command in progress, just do a snap shot of the cards current status.
            for( MemoryUnitOffset = QUAG_MEMORY_UNIT_ID_0; MemoryUnitOffset <= QUAG_MEMORY_UNIT_MAX_ID; MemoryUnitOffset++ )
            {
                pCurrentMemUnit = &mGCMemoryUnits[MemoryUnitOffset];

                Slot = GetSlotFromMemoryUnitID( pCurrentMemUnit->mUnitID );
                // Probe the card to the latest info on it.
                RetVal = CARDProbeEx( Slot, (s32*)&pCurrentMemUnit->mTotalSize, (s32*)&pCurrentMemUnit->mSectorSize );

                // Convert the total size from megabits to bytes.
                pCurrentMemUnit->mTotalSize  = pCurrentMemUnit->mTotalSize * NUMBER_OF_BYTES_IN_A_MEGA_BIT_NUMBER;

                // See if this memory unit is connected.
                if( RetVal == CARD_RESULT_READY ||
                    RetVal == CARD_RESULT_WRONGDEVICE)
                {
                    if( pCurrentMemUnit->mbConnected == FALSE )
                    {
                        ProcessCardConnected( pCurrentMemUnit );

                        //8/9/2002 12:21:12 AM - jimc
                        //wrong devices need to get through to the mounthandler
                        if (RetVal == CARD_RESULT_WRONGDEVICE)
                            pCurrentMemUnit->mbCompatible = TRUE;
                    }
                }
                else
                {
                    if( pCurrentMemUnit->mbConnected )
                    {
                        ProcessCardDisconnected( pCurrentMemUnit );
                    }
                }
            }
        }
        else
        {
            // Prematurely complete any command in progress if there is no active card.
            if( mpActiveMemoryUnit == FALSE )
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
            }
            else
            {
                // If there is a current command handler, use it.
                if( mpCurrentCommandHandler )
                {
                    // Call the current command handler.
                    (this->*mpCurrentCommandHandler)();
                }
                else
                {
                    ASSERT( 0 );
                    // WE Should never get to this point.
                }
            }
        }
    }
    else
    {
        // The system hasn't initialized the memory card system, it's created, but not completely ready.
        // so do nothing.
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
// Interface Functions.
//-------------------------------------------------------------------------------------------------------------------------------------
void GCMemoryUnitManager::SetBrowserText( char* pBrowserInfo )
{
    x_memset(  mBrowserCommentText, 0, GCMU_BROWSER_LINE_LENGTH );
    x_strncpy( mBrowserCommentText, pBrowserInfo, GCMU_BROWSER_LINE_LENGTH );

    // Make sure that the browser text strings are not too large.
    ASSERT( x_strlen( mBrowserCommentText ) < GCMU_BROWSER_LINE_LENGTH );
}


//-------------------------------------------------------------------------------------------------------------------------------------
void GCMemoryUnitManager::GetBrowserText( char* Buffer, s32 Size )
{
    x_strncpy( Buffer, mBrowserCommentText, Size );
}


//-------------------------------------------------------------------------------------------------------------------------------------
// Actual Memory Card Related Functions.
//-------------------------------------------------------------------------------------------------------------------------------------
xbool GCMemoryUnitManager::IsMemoryUnitConnected( s32 MemoryCardID )
{
    ASSERT( MemoryCardID >= QUAG_MEMORY_UNIT_ID_0 && MemoryCardID <= QUAG_MEMORY_UNIT_MAX_ID );

    return mGCMemoryUnits[MemoryCardID].mbConnected;
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool GCMemoryUnitManager::IsActiveMemoryUnitStillConnected( void )
{
    ASSERT( mInitialized );
    if( mpActiveMemoryUnit )
        return mpActiveMemoryUnit->mbConnected;
    else
        return FALSE;
}


//-------------------------------------------------------------------------------------------------------------------------------------
char* GCMemoryUnitManager::GetNameOfActiveMemoryUnit( void )
{
    ASSERT( mInitialized );
    if( mpActiveMemoryUnit )
        return mpActiveMemoryUnit->mName;
    else
        return NULL;
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 GCMemoryUnitManager::GetFreeSpaceRemainingOnActiveUnit( void )
{
    ASSERT( mInitialized );
    if( mpActiveMemoryUnit )
    {
        return mpActiveMemoryUnit->mFreeBlocks;
    }
    else
    {
        return NULL;
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
s32 GCMemoryUnitManager::GetFreeFilesRemainingOnActiveUnit( void )
{
    ASSERT( mInitialized );
    if( mpActiveMemoryUnit )
    {
        return mpActiveMemoryUnit->mFreeFiles;
    }
    else
    {
        return NULL;
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
s32 GCMemoryUnitManager::SelectActiveMemoryUnit( s32 MemoryUnitID, byte* pFileIOBuffer, s32 BufferSize )
{
    ASSERT( mInitialized );
    // Make sure that the card they wish to activate is even there.
    if( mGCMemoryUnits[MemoryUnitID].mbConnected )
    {
        // First Make sure that no command is active.
        if( IsCommandInProgress( ) == FALSE )
        {
            // Make sure that the memory unit ID is valid
            if( (MemoryUnitID >= QUAG_MEMORY_UNIT_ID_0) && (MemoryUnitID <= QUAG_MEMORY_UNIT_MAX_ID) )
            {
                // If there is no active unit, make the requested one the active unit.
                if( mpActiveMemoryUnit == NULL )
                {
                    s32 CommandRequestReturnValue;

                    mpActiveMemoryUnit          = &mGCMemoryUnits[MemoryUnitID];
                    mActiveMemoryUnitSlot       = GetSlotFromMemoryUnitID( MemoryUnitID );
                    mFileInfo.mpUserDataBuffer  = pFileIOBuffer;
                    mFileInfo.mUserBufferSize   = BufferSize;

                    // Request for the next command.
                    CommandRequestReturnValue = RequestNewCommand( GCMU_CMD_MOUNT );

                    // If the command to mount the card failed, then make sure to releaes the active unit.
                    if( CommandRequestReturnValue != QUAG_MU_EVERYTHING_IS_OK )
                    {
                        // You don't want to release the mounted card if all that's wrong is that it's not formatted
                        // This card should still be valid, but the only command allowed should be to format it.
                        if( CommandRequestReturnValue != QUAG_MU_NOT_FORMATTED )
                        {
                            ReleaseActiveMemoryUnit( );
                        }
                    }

                    return CommandRequestReturnValue;
                }
                // If there is an active, make sure it's already the one requested, otherwise FAILURE.
                else if( mpActiveMemoryUnit != &mGCMemoryUnits[MemoryUnitID] )
                {
                    return QUAG_MU_TRYING_TO_ACTIVATE_MORE_THAN_ONE_CARD;
                }
                // if this is the active unit, and it's not formatted, then return that it's not formatted.
                else if( mpActiveMemoryUnit->mbFormatted == FALSE )
                {
                    return QUAG_MU_NOT_FORMATTED;
                }

                // It's all good.
                return QUAG_MU_EVERYTHING_IS_OK;
            }
            // Invalid memory unit requested.
            else
            {
                ASSERT( 0 );
                return QUAG_MU_TRYING_TO_ACTIVATE_ILLEGAL_CARD;
            }
        }
        else
        {
            return QUAG_MU_SYSTEM_BUSY;
        }
    }
    else
    {
        return QUAG_MU_NO_CARD;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 GCMemoryUnitManager::ReleaseActiveMemoryUnit( void )
{
    ASSERT( mInitialized );
    if( mpActiveMemoryUnit != NULL )
    {
        // Force the unmount
        do
        {
            mLastCARDCommandReturnValue = CARDUnmount( mActiveMemoryUnitSlot ); 
        }while( mLastCARDCommandReturnValue == CARD_RESULT_BUSY );

        mpActiveMemoryUnit->mbMounted = FALSE;

        mpActiveMemoryUnit    = NULL;
        mActiveMemoryUnitSlot = -1;

        // Clear the available game list.
        ClearAvailableGamesList( );

        // Prematurely complete any command in progress.
        if( IsCommandInProgress( ) )
            CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );

        // Release was good.
        return QUAG_MU_EVERYTHING_IS_OK;
    }
    else
    {
        // For debugging, just test to see if there is a messed up selected memory unit.
#ifdef X_DEBUG
        s32     i;
        xbool   bFoundMemoryUnit = FALSE;

        for( i = 0; i <= QUAG_MEMORY_UNIT_MAX_ID; i++ )
        {
            // See if you found a valid match.
            if( mpActiveMemoryUnit == mGCMemoryUnits )
            {
                bFoundMemoryUnit = TRUE;
                break;
            }
        }

        if( bFoundMemoryUnit == FALSE )
        {
            MC_PRINT( "Trying to release an active card when there isn't one." );
        }
#endif
        return QUAG_MU_NO_ACTIVE_CARD_SELECTED;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
// Ignore the bWorstCase, since on Gamecube, it's always worst case.
s32 GCMemoryUnitManager::CalculateRequiredSizeForSaveGameType( s32 SaveGameType, s32* pRequiredBlocks, xbool bWorstCase)
{
    ASSERT( mInitialized );

    s32 SizeOfSaveOverhead;
    s32 SaveGameDataSize;
    s32 TotalSize;
    s32 SectorSize;

    // Right off the bat make sure that there is an active card selected, if there's not, there are certain
    // pieces of information that will not be available. so bail right away.
    if( bWorstCase == FALSE )
    {
        if( mpActiveMemoryUnit == NULL )
        {
            *pRequiredBlocks = 0;
            return QUAG_MU_NO_ACTIVE_CARD_SELECTED;
        }
        else
        {
            SectorSize = mpActiveMemoryUnit->mSectorSize;
        }
    }
    else
    {
        SectorSize = GCMU_SUPPORTED_SECTOR_SIZE;
    }

    //=====================================================================================================================================
    // Calculating the size of a GameCube save.
    //=====================================================================================================================================
    //
    // First - Determine the amount of overhead used by the Icons, and browser text, etc...
    SizeOfSaveOverhead =  (mIconData.mNumberOfIconFrames * GC_MEMCARD_ICON_FRAME_SIZE );
    SizeOfSaveOverhead += GC_MEMCARD_CLUT_SIZE;
    SizeOfSaveOverhead += GCMU_BROWSER_LINE_LENGTH + GCMU_BROWSER_LINE_LENGTH;
        
    // Second, using the passed game type, get the size of the save game data for that type.
    SaveGameDataSize = mpSaveGameTypes[SaveGameType].mUserDataFileInfo.mSizeOfUserDataFile;

    // Third, add it up.
    TotalSize = SizeOfSaveOverhead + SaveGameDataSize;

    // Fourth, round it up to the nearest sector.
    TotalSize += SectorSize - (TotalSize % SectorSize);
    ASSERT( !(TotalSize % SectorSize) );

    // Record the size.
    *pRequiredBlocks = TotalSize / SectorSize;

    // To end it, determine if the card currently has enough space to accomodate a file of this type.
    if( bWorstCase == FALSE )
    {
        if( *pRequiredBlocks <= mpActiveMemoryUnit->mFreeBlocks )
        {
            return QUAG_MU_EVERYTHING_IS_OK;
        }
    }

    return QUAG_MU_NOT_ENOUGH_FREE_SPACE;
}

//-------------------------------------------------------------------------------------------------------------------------------------
s32 GCMemoryUnitManager::GetUserDataByteSize( s32 SaveGameType )
{
    ASSERT( mInitialized );
    ASSERT( SaveGameType >= 0 && SaveGameType < mNumberOfSaveGameTypes );
    return mpSaveGameTypes[SaveGameType].mUserDataFileInfo.mSizeOfUserDataFile;
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 GCMemoryUnitManager::FormatCard( void )
{
    ASSERT( mInitialized );

    s32 CommandRequestReturnValue;

    CommandRequestReturnValue = RequestNewCommand( GCMU_CMD_FORMAT );

    if( CommandRequestReturnValue == QUAG_MU_EVERYTHING_IS_OK )
    {
        // Well it's time that we start.
        return QUAG_MU_EVERYTHING_IS_OK;
    }
    else
    {
        return CommandRequestReturnValue;
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
s32 GCMemoryUnitManager::SaveGame( s32 SaveGameType, s32 TypeOffset, void* pUserData, char* SaveGameBrowserInfo )
{
    ASSERT( mInitialized );

    s32 CommandRequestReturnValue;

    CommandRequestReturnValue = RequestNewCommand( GCMU_CMD_SAVE );

    if( CommandRequestReturnValue == QUAG_MU_EVERYTHING_IS_OK )
    {
        // Determine if the save game type is valid.
        ASSERT( SaveGameType >= 0 && SaveGameType < mNumberOfSaveGameTypes );
        mCurrentGameType = SaveGameType;

        // Determine if the offset into this Save Game Type is valid or not.
        ASSERT( TypeOffset >= 0 && TypeOffset <= mpSaveGameTypes[SaveGameType].mMaxNumberOfSaves );
        mCurrentGameTypeIndex = TypeOffset;

        // Grab hold of the user data, and identify how large the save will work out to be.
        ASSERT( pUserData );
        mFileInfo.mpUserDataBuffer = (byte*)pUserData;
        mFileInfo.mUserBufferSize  = mpSaveGameTypes[SaveGameType].mUserDataFileInfo.mSizeOfUserDataFile;
        mFileInfo.mFileDataSize    = mpSaveGameTypes[SaveGameType].mUserDataFileInfo.mSizeOfUserDataFile;

        // Set up the browser text.
        SetBrowserText( SaveGameBrowserInfo );

        // Start your engines.
        return QUAG_MU_EVERYTHING_IS_OK;
    }
    else
    {
        return CommandRequestReturnValue;
    }

    return QUAG_MU_SYSTEM_BUSY;
}

//-------------------------------------------------------------------------------------------------------------------------------------
s32 GCMemoryUnitManager::LoadGame( s32 SaveGameType, s32 TypeOffset, void* pUserData )
{
    ASSERT( mInitialized );

    s32 CommandRequestReturnValue;

    CommandRequestReturnValue = RequestNewCommand( GCMU_CMD_LOAD );

    if( CommandRequestReturnValue == QUAG_MU_EVERYTHING_IS_OK )
    {
        // Determine if the save game type is valid.
        ASSERT( SaveGameType >= 0 && SaveGameType < mNumberOfSaveGameTypes );
        mCurrentGameType = SaveGameType;

        // Determine if the offset into this Save Game Type is valid or not.
        ASSERT( TypeOffset >= 0 && TypeOffset <= mpSaveGameTypes[SaveGameType].mMaxNumberOfSaves );
        mCurrentGameTypeIndex = TypeOffset;

        // Grab hold of the user data.
        ASSERT( pUserData );
        mFileInfo.mpUserDataBuffer = (byte*)pUserData;
        mFileInfo.mUserBufferSize  = mpSaveGameTypes[SaveGameType].mUserDataFileInfo.mSizeOfUserDataFile;
        mFileInfo.mFileDataSize    = mpSaveGameTypes[SaveGameType].mUserDataFileInfo.mSizeOfUserDataFile;

        // Well it's time that we start.
        return QUAG_MU_EVERYTHING_IS_OK;
    }
    else
    {
        return CommandRequestReturnValue;
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
s32 GCMemoryUnitManager::DeleteGame( s32 SaveGameType, s32 TypeOffset )
{
    ASSERT( mInitialized );

    s32 CommandRequestReturnValue;

    CommandRequestReturnValue = RequestNewCommand( GCMU_CMD_DELETE );

    if( CommandRequestReturnValue == QUAG_MU_EVERYTHING_IS_OK )
    {
        // Determine if the save game type is valid.
        ASSERT( SaveGameType >= 0 && SaveGameType < mNumberOfSaveGameTypes );
        mCurrentGameType = SaveGameType;

        // Determine if the offset into this Save Game Type is valid or not.
        ASSERT( TypeOffset >= 0 && TypeOffset <= mpSaveGameTypes[SaveGameType].mMaxNumberOfSaves );
        mCurrentGameTypeIndex = TypeOffset;

        // Get the name of the file.
        GetUserDataFileName( mCurrentGameType, mCurrentGameTypeIndex, mFileInfo.mFileName );

        // Well it's time that we start.
        return QUAG_MU_EVERYTHING_IS_OK;
    }
    else
    {
        return CommandRequestReturnValue;
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
s32 GCMemoryUnitManager::GetSaveGameList( s32 SaveGameType, s32* pMaxSaveGameTypeCount, SingleSavedGameInfo** pSaveGameListings )
{
    ASSERT( mInitialized );

    s32 ValidCount;
    s32 index;

    ASSERT( pSaveGameListings );

    ValidCount = 0;

    // Verify that the requested save type is valid.
    ASSERT( SaveGameType >= 0 && SaveGameType <= mNumberOfSaveGameTypes );

    // Determine the maximum number of saves for this SaveGameType.
    if( pMaxSaveGameTypeCount )
        *pMaxSaveGameTypeCount = mpSaveGameTypes[SaveGameType].mMaxNumberOfSaves;


    // First, just determine how many saved games there are that are valid.
    for( index = 0; index < MEMORY_UNIT_MAX_SAVE_COUNT_PER_TYPE; index++ )
    {
        if( mpAvailableSavedGames[SaveGameType].mSavedGames[index].mbValid )
            ValidCount++;
    }

    // Return the address of the game list for this save game type if there are any valid saves.
    if( ValidCount )
    {
        *pSaveGameListings = mpAvailableSavedGames[SaveGameType].mSavedGames;
    }
    else
    {
        *pSaveGameListings = NULL;
    }
    
    
    return ValidCount;
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool GCMemoryUnitManager::IsCommandComplete( void )
{
    return ( mbCommandCompleted &&                              // If the command is complete 
             mbWaitingForCommandCompleteAcknowledged == TRUE && // and you are waiting to get acknowledgment from the caller
             (mCurrentCommand != GCMU_CMD_NONE)                 // and there is actually a command set.
           );
}

//-------------------------------------------------------------------------------------------------------------------------------------
xbool GCMemoryUnitManager::GetLastCommandValues( s32* pReturnValue, s32* pErrorCode )
{
    ASSERT( mInitialized );

    // Make sure that there is a command waiting to be acknowledged.
    if( mbWaitingForCommandCompleteAcknowledged )
    {
        if( pReturnValue )
            *pReturnValue = mCommandReturnValue;

        if( pErrorCode )
            *pErrorCode = mCommandError;

        mbWaitingForCommandCompleteAcknowledged = FALSE;
        mbCommandCompleted                      = TRUE;
        mpCurrentCommandHandler                 = NULL;
        mCurrentCommand                         = GCMU_CMD_NONE;

        // The command was completed as complete by the user call.
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
// Return TRUE for both a write or a delete, since you could be doing an overwrite, which is in a sense 
// a write operation.
xbool GCMemoryUnitManager::IsWriteInProgress( void )
{
	// This flag mbSavingInProgress will be set to true when a game has begun to save a file.
	// It will be cleared when it has completed the save.
    return mbSavingInProgress;
}

//-------------------------------------------------------------------------------------------------------------------------------------
xbool GCMemoryUnitManager::GenerateCardName( s32 QuagMemCardID, char* pBuffer, s32 SizeOfBuffer )
{
    char NameBuffer[MEMORY_UNIT_PERSONALIZED_NAME_SIZE];

    x_sprintf( NameBuffer, "Memory Card in Slot %c", QuagMemCardID == QUAG_MEMORY_UNIT_ID_0 ? 'A' : 'B' );

    if( x_strlen( NameBuffer ) < SizeOfBuffer - 1 )
    {
        x_strcpy( pBuffer, NameBuffer );
        return TRUE;
    }
    else
    {
        pBuffer[0] = 0;
        return FALSE;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
void GCMemoryUnitManager::CompleteCommandAndWaitForUser( s32 ReturnValue, s32 ErrorCode )
{
    ASSERT( mInitialized );

    // Do a special back up catch all here left over files open here.
    if( mFileInfo.mbOpen )
    {
        // We should never be completing a command if a file is open when its a success.
        // If it's a failure, it's ok, since it could be a failure from having a card disconnect.
        if( ReturnValue == TRUE )
            ASSERT( 0 );
    }

    mbCommandCompleted                      = TRUE;
    mbWaitingForCommandCompleteAcknowledged = TRUE;
    mCommandReturnValue                     = ReturnValue;
    mCommandError                           = ErrorCode;
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 GCMemoryUnitManager::RequestNewCommand( s32 NewCommand )
{
    ASSERT( mInitialized );

    // Make sure the command is valid.
    ASSERT( NewCommand >= GCMU_CMD_FIRST && NewCommand <= GCMU_CMD_LAST );

    // Make sure that an active card is already selected.
    if( mpActiveMemoryUnit )
    {
        // Is there a command in progress?
        if( IsCommandInProgress( ) == FALSE )
        {
            mCurrentCommand                         = NewCommand;
            mCommandState                           = 0;
            mCommandSubState                        = 0;
            mbCommandCompleted                      = FALSE;
            mbWaitingForCommandCompleteAcknowledged = FALSE;

            SetRequestedCommandHandler( NewCommand );

            MC_PRINT( "Requests %s. Request GRANTED", GetCommandNameAsString( NewCommand ) );
            return QUAG_MU_EVERYTHING_IS_OK;
        }
        else
        {
            MC_PRINT( "Requests %s. Request FAILED", GetCommandNameAsString( NewCommand ) );
            return QUAG_MU_SYSTEM_BUSY;
        }
    }
    else
    {
        MC_PRINT( "Requests %s, but there was no active memory card selected.  Request FAILED", GetCommandNameAsString( NewCommand ) );
        return QUAG_MU_NO_ACTIVE_CARD_SELECTED;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool GCMemoryUnitManager::IsCommandInProgress( void )
{
    return mCurrentCommand != GCMU_CMD_NONE;
}


//-------------------------------------------------------------------------------------------------------------------------------------
void GCMemoryUnitManager::SetRequestedCommandHandler( s32 NewCommand )
{
    ASSERT( mInitialized );

    // reset this timer for each command.
    mCommandStartTime = x_GetTime( );

    switch( NewCommand )
    {
        case GCMU_CMD_MOUNT:   mpCurrentCommandHandler = &GCMemoryUnitManager::MountHandler;      break;
        case GCMU_CMD_FORMAT:  mpCurrentCommandHandler = &GCMemoryUnitManager::FormatHandler;     break;
        case GCMU_CMD_SAVE:    mpCurrentCommandHandler = &GCMemoryUnitManager::SaveGameHandler;   break;
        case GCMU_CMD_LOAD:    mpCurrentCommandHandler = &GCMemoryUnitManager::LoadGameHandler;   break;
        case GCMU_CMD_DELETE:  mpCurrentCommandHandler = &GCMemoryUnitManager::DeleteGameHandler; break;

        default:
            ASSERT( 0 );
            mpCurrentCommandHandler = NULL;
            break;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
void GCMemoryUnitManager::ResetCommandHandlingVariables( void )
{
    // Initialize the command handling variables.
    mpCurrentCommandHandler                 = NULL;
    mCurrentCommand                         = GCMU_CMD_NONE;
    mbCommandCompleted                      = TRUE; 
    mbWaitingForCommandCompleteAcknowledged = FALSE;

    // Initialize system to scan the memory card slots.
    mbAllowBackgroundCardChecking      = FALSE;
    mbBackgrounCardCheckInProgress     = FALSE;
    mCurrentBackgroundCardBeingChecked = -1;

    // Initialize the current memory unit to none.
    mpActiveMemoryUnit    = NULL;
    mActiveMemoryUnitSlot = -1;

    // Initialize the file system
    mFileInfo.Reset( FALSE );
}

//-------------------------------------------------------------------------------------------------------------------------------------
// Command handling functions.
//-------------------------------------------------------------------------------------------------------------------------------------
void GCMemoryUnitManager::UpdateFreeBlocksAndFiles( void )
{
    ASSERT( mInitialized );

    s32 TotalBlocksOnTheCard;
    s32 OriginalFreeFiles;

    // Do a quick fast ass test to first see if the active card is still there.
    if( mpActiveMemoryUnit )
    {
        // Get information on the space free on the card.
        if( mpActiveMemoryUnit->mbMounted && mpActiveMemoryUnit->mbFormatted )
        {
            // Once the card has been checked and validated, make sure to update the free blocks.
            if( CARDFreeBlocks( mActiveMemoryUnitSlot,
                                (s32*)&mpActiveMemoryUnit->mFreeBytes,
                                (s32*)&mpActiveMemoryUnit->mFreeFiles ) != CARD_RESULT_READY )
            {
                mpActiveMemoryUnit->mFreeBytes  = 0;
                mpActiveMemoryUnit->mFreeBlocks = 0;
                mpActiveMemoryUnit->mFreeFiles  = 0;
            }

            // One Final test here, if the number of files free is greater than the number of free blocks,
            // you must limit the free files count.  This is required since you can never have more free files than the number
            // of free blocks.
            if( mpActiveMemoryUnit->mSectorSize != 0 )
            {
                mpActiveMemoryUnit->mFreeBlocks = mpActiveMemoryUnit->mFreeBytes / mpActiveMemoryUnit->mSectorSize;
            }
            else
            {
                mpActiveMemoryUnit->mFreeBlocks = 0;
            }

            // Determine the number of free files with some dumb ass check from Nintendo.
            TotalBlocksOnTheCard = (mpActiveMemoryUnit->mTotalSize / mpActiveMemoryUnit->mSectorSize) - GCMC_SYSTEM_SECTORS_WASTED_FOR_OVERHEAD;
            OriginalFreeFiles = mpActiveMemoryUnit->mFreeFiles;

            if( TotalBlocksOnTheCard > GCMU_MAX_FREE_FILES_VALUE )
                TotalBlocksOnTheCard = GCMU_MAX_FREE_FILES_VALUE;

            mpActiveMemoryUnit->mFreeFiles = TotalBlocksOnTheCard - ( GCMU_MAX_FREE_FILES_VALUE - OriginalFreeFiles );
        }
    }
    else
    {
        MC_PRINT( "Trying to get Free blocks and files for when there is no active memory card." );
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
void GCMemoryUnitManager::MountHandler( void )
{
    ASSERT( mInitialized );

    enum MountStates
    {
        MOUNT_STATE_WEED_OUT_BAD_CARD,
        MOUNT_STATE_REQUEST_FOR_MOUNT,
        MOUNT_STATE_WAIT_FOR_MOUNT_TO_COMPLETE,
        MOUNT_STATE_WAIT_FOR_CHECK_TO_COMPLETE,
        MOUNT_STATE_GET_CARD_SAVED_GAMES,
        MOUNT_STATE_COMPLETE,
    };
                
    switch( mCommandState )
    {
        // Weed out any bad cards that are not compatible.
        //-------------------------------------------------------------------------------------------------------------------------------------
        case MOUNT_STATE_WEED_OUT_BAD_CARD:
        {
            // Make sure that the card is still connected.
            if( IsActiveMemoryUnitStillConnected( ) )
            {
                // Is this card compatible with the system.
                if( mpActiveMemoryUnit->mbCompatible )
                {
                    // Well, all looks good, go for the mount.
                    if( mpActiveMemoryUnit->mbMounted == FALSE )
                    {
                        mCommandState = MOUNT_STATE_REQUEST_FOR_MOUNT;                        
                    }
                    else
                    {
                        // This should never happen.
                        ASSERT( 0 );
                        break;
                    }
                }
                else
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_INVALID_CARD );
                }
            }
            else
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
            }
            
            break;
        }

        // Request to the game cube that you wish to mount this card.
        //-------------------------------------------------------------------------------------------------------------------------------------
        case MOUNT_STATE_REQUEST_FOR_MOUNT:
        {
#ifdef USE_SYNCHRONOUS_FUNCTIONS
            mLastCARDCommandReturnValue = CARDMount( mActiveMemoryUnitSlot, mpMountingWorkArea, GCMUCallBack_CardDisconnected );
#else
            CARDMountAsync( mActiveMemoryUnitSlot, mpMountingWorkArea, GCMUCallBack_CardDisconnected, NULL );
#endif
            mCommandState = MOUNT_STATE_WAIT_FOR_MOUNT_TO_COMPLETE;
            break;
        }


        // You have to wait for the mount to complete.
        //-------------------------------------------------------------------------------------------------------------------------------------
        case MOUNT_STATE_WAIT_FOR_MOUNT_TO_COMPLETE:
        {
            xbool bPerformCheck = FALSE;
            s32   ErrorCode = QUAG_MU_EVERYTHING_IS_OK;


#ifndef USE_SYNCHRONOUS_FUNCTIONS
            mLastCARDCommandReturnValue = CARDGetResultCode( mActiveMemoryUnitSlot );
            if( mLastCARDCommandReturnValue == CARD_RESULT_BUSY )
                return;
#endif
            switch( mLastCARDCommandReturnValue )
            {
                // If it was one of these return values, the card was mounted.
                case CARD_RESULT_READY:
                case CARD_RESULT_ENCODING:
                case CARD_RESULT_BROKEN:
                {
                    mpActiveMemoryUnit->mbMounted = TRUE;
                    bPerformCheck = TRUE;
                    break;
                }

                // If it was one of these return values, the card was NOT mounted.
                case CARD_RESULT_WRONGDEVICE:
                case CARD_RESULT_NOCARD:
                case CARD_RESULT_BUSY:
                case CARD_RESULT_FATAL_ERROR:
                case CARD_RESULT_IOERROR:
                    ErrorCode = ConvertCARDSysResultToQuagResult( mLastCARDCommandReturnValue );
                    break;

                default:
                    ErrorCode = QUAG_MU_UNKNOWN_PROBLEM;
                    ASSERT( 0 );
                    break;
            }

            // If the card can be used, or formatted, or something, basically it's not wasted, or has been removed,
            // then you must check the card to make sure that it's 100%.
            if( bPerformCheck )
            {
                MC_PRINT( "%s has been mounted, now checking...\n", mpActiveMemoryUnit->mName );

#ifdef USE_SYNCHRONOUS_FUNCTIONS
                mLastCARDCommandReturnValue = CARDCheck( mActiveMemoryUnitSlot );
#else
                CARDCheckAsync( mActiveMemoryUnitSlot, NULL );
#endif
                mCommandState = MOUNT_STATE_WAIT_FOR_CHECK_TO_COMPLETE;
                mCommandSubState = 0;
            }
            // If the card was not worthy, then back out and fail.
            else
            {
                CompleteCommandAndWaitForUser( FALSE, ErrorCode );
            }

            break;
        }

        // Wait for the check up to be completed.
        //-------------------------------------------------------------------------------------------------------------------------------------
        case MOUNT_STATE_WAIT_FOR_CHECK_TO_COMPLETE:
        {
            s32   ErrorCode = QUAG_MU_EVERYTHING_IS_OK;
            xbool CardHasValue = FALSE;

#ifndef USE_SYNCHRONOUS_FUNCTIONS
            mLastCARDCommandReturnValue = CARDGetResultCode( mActiveMemoryUnitSlot );

            if( mLastCARDCommandReturnValue == CARD_RESULT_BUSY )
                return;
#endif

            switch( mLastCARDCommandReturnValue )
            {
                // The card was mounted.
                case CARD_RESULT_READY:
                    CardHasValue = TRUE;
                    mpActiveMemoryUnit->mbFormatted = TRUE;
                    ErrorCode = QUAG_MU_EVERYTHING_IS_OK;
                    break;

                // The card format does not match the country code of the system that the card is connected to.
                case CARD_RESULT_ENCODING:
                    CardHasValue = TRUE;
                    ErrorCode = QUAG_MU_INVALID_FORMAT;
                    break;

                // The cards file system is broken.
                case CARD_RESULT_BROKEN:
                    CardHasValue = TRUE;
                    ErrorCode = QUAG_MU_FILE_SYSTEM_DAMAGED;
                    break;

                // If it was one of these return values, the card is not usable.
                case CARD_RESULT_WRONGDEVICE:
                case CARD_RESULT_NOCARD:
                case CARD_RESULT_BUSY:
                case CARD_RESULT_FATAL_ERROR:
                case CARD_RESULT_IOERROR:
                    ErrorCode = ConvertCARDSysResultToQuagResult( mLastCARDCommandReturnValue );
                    break;

                default:
                    ErrorCode = QUAG_MU_UNKNOWN_PROBLEM;
                    ASSERT( 0 );
                    break;
            }

            // If the card has no value, it means that it's either messed up beyond repair, or it has
            // been removed.  So end the mount with the correct error code.
            if( CardHasValue == FALSE )
            {
                CompleteCommandAndWaitForUser( FALSE, ErrorCode );
            }
            // If the check succeeded, either the card is valid, or it's salvagable.
            else
            {
                // If the cards free space, free files etc can be looked evaluated, do it.
                // Otherwise exit now.
                if( ErrorCode == QUAG_MU_EVERYTHING_IS_OK )
                {
                    mCommandState = MOUNT_STATE_GET_CARD_SAVED_GAMES;
                    mCommandSubState = 0;
                }
                else
                {
                    CompleteCommandAndWaitForUser( FALSE, ErrorCode );
                }
            }

            break;
        }

        case MOUNT_STATE_GET_CARD_SAVED_GAMES:
        {
            if( MountHandlerGetSavedGames( ) )
            {
                mCommandState = MOUNT_STATE_COMPLETE;
                mCommandSubState = 0;
            }
            break;
        }

        case MOUNT_STATE_COMPLETE:
        {
            UpdateFreeBlocksAndFiles( );
            CompleteCommandAndWaitForUser( TRUE, QUAG_MU_EVERYTHING_IS_OK );
            break;
        }

        default:
            ASSERT( 0 );
            break;
    }

}

//-------------------------------------------------------------------------------------------------------------------------------------
xbool GCMemoryUnitManager::MountHandlerGetSavedGames( void )
{
    ASSERT( mInitialized );

    SingleSavedGameInfo* pSavedGame;

    enum ScanningForSavedGames
    {
        GET_SAVED_GAME_FILES_START,
        GET_SAVED_GAME_LOOK_FOR_FILE_ENTRY,
        GET_SAVED_GAME_OPEN_FILE,
        GET_SAVED_GAME_READ_SAVED_GAME_TITLE_START,
        GET_SAVED_GAME_READ_SAVED_GAME_TITLE_WAIT,
        GET_SAVED_GAME_TEST_SAVED_GAME_VALID_START,
        GET_SAVED_GAME_TEST_SAVED_GAME_VALID_WAIT,

        GET_SAVED_GAME_GO_TO_NEXT_SAVE_GAME,
        GET_SAVED_GAME_COMPLETE,
    };

    // Go one at a time through the base directories that we are using to see if they exist.
    switch( mCommandSubState )
    {
        // Start this investigation off.
        // Initialize the system to the first game type, and the first save of that type.
        case GET_SAVED_GAME_FILES_START:
        {
            mCurrentGameType = 0;
            mCurrentGameTypeIndex = 0;
            mCommandSubState = GET_SAVED_GAME_LOOK_FOR_FILE_ENTRY;

            ClearAvailableGamesList( );
            break;
        }

        // Test to see if all save game types, and all instances of these types have been tested.
        case GET_SAVED_GAME_LOOK_FOR_FILE_ENTRY:
        {
            // See if you have tested for all of the possible files of this save game type.
            if( mCurrentGameTypeIndex == mpSaveGameTypes[mCurrentGameType].mMaxNumberOfSaves )
            {
                mCurrentGameType++;
        
                // If all of the save game types have been processed, then there we are finally done.
                if( mCurrentGameType == mNumberOfSaveGameTypes )
                {
                    mCommandSubState = GET_SAVED_GAME_COMPLETE;
                    break;
                }
                else
                {
                    mCurrentGameTypeIndex = 0;
                }
            }

            mCommandSubState = GET_SAVED_GAME_OPEN_FILE;
            break;
        }

        // Test to see if the save game exists, if it does open it.
        case GET_SAVED_GAME_OPEN_FILE:
        {
            s32 ErrorCode;

            // Set all the data required about how to open the file.
            ErrorCode = OpenFile( FALSE );

            switch( ErrorCode )
            {
                // File was found and opened, continue to read the browser title from it.
                case QUAG_MU_EVERYTHING_IS_OK:
                {
                    mCommandSubState = GET_SAVED_GAME_READ_SAVED_GAME_TITLE_START;
                    break;
                }
                
                // File wasn't found, so just jump to the next one.
                case QUAG_MU_FILE_NOT_FOUND:
                case QUAG_MU_FILE_ACCESS_DENIED:
                {
                    mCommandSubState = GET_SAVED_GAME_GO_TO_NEXT_SAVE_GAME;
                    break;
                }

                // Card is in trouble, deal with it.
                default:
                {
                    CompleteCommandAndWaitForUser( FALSE, ErrorCode );
                    break;
                }

            }
            break;
        }

        // Try to initiate a read to read only the 2 lines of browser text.
        case GET_SAVED_GAME_READ_SAVED_GAME_TITLE_START:
        {
            s32 ErrorCode = PrepareForFileTransfer( FALSE, TRUE );

            if( ErrorCode == QUAG_MU_EVERYTHING_IS_OK )
            {
                mCommandSubState = GET_SAVED_GAME_READ_SAVED_GAME_TITLE_WAIT;
            }
            else
            {
                CompleteCommandAndWaitForUser( FALSE, ErrorCode );
            }

            break;
        }

        // Wait for the read to complete.
        case GET_SAVED_GAME_READ_SAVED_GAME_TITLE_WAIT:
        {
            s32 ErrorCode;

            if( ReadFile( ErrorCode ) )
            {
                // First thing to do is close the file.
                if( ErrorCode == QUAG_MU_EVERYTHING_IS_OK )
                {
                    // If the read was successful, the browser title and comment text is inside of 
                    // the data buffer.  So skip the title, and grab the comment.
                    x_strncpy( mBrowserCommentText, (char*)&mFileInfo.mpUserDataBuffer[GCMU_BROWSER_LINE_LENGTH], GCMU_BROWSER_LINE_LENGTH );

                    // Get the saved game and make it valid, as well, assign the browser text to it.
                    pSavedGame = &mpAvailableSavedGames[mCurrentGameType].mSavedGames[mCurrentGameTypeIndex];
                    pSavedGame->mbValid            = TRUE;
                    pSavedGame->mSaveGameType      = mCurrentGameType;
                    pSavedGame->mSaveGameTypeIndex = mCurrentGameTypeIndex;
                    x_memcpy( pSavedGame->mSaveGameTitle, mBrowserCommentText, MEMORY_UNIT_SAVE_GAME_TITLE_LENGTH - 1 );

                    // Force the string to NULL terminate.
                    pSavedGame->mSaveGameTitle[MEMORY_UNIT_SAVE_GAME_TITLE_LENGTH - 1] = 0;

                    mCommandSubState = GET_SAVED_GAME_GO_TO_NEXT_SAVE_GAME;
                }
                else
                {
                    CompleteCommandAndWaitForUser( FALSE, ErrorCode );
                }

                // Just close the file, who cares what the return is, you are closing the file.
                // If the card is removed, you will nab it somewhere else.
                CARDClose( &mFileInfo.mGameCubeCARDFileInfo );
                mFileInfo.Reset( );
            }
            break;
        }

        case GET_SAVED_GAME_GO_TO_NEXT_SAVE_GAME:
        {
            // Move onto the next saved game of this type.
            mCurrentGameTypeIndex++;

            // Get the next file entry.
            mCommandSubState = GET_SAVED_GAME_LOOK_FOR_FILE_ENTRY;
            break;
        }

        case GET_SAVED_GAME_COMPLETE:
        {
            return TRUE;
            break;
        }


        default:
            ASSERT( 0 );
            break;

    } // switch( mCommandSubState )

    return FALSE;
}
 

//-------------------------------------------------------------------------------------------------------------------------------------
void GCMemoryUnitManager::FormatHandler( void )
{
    ASSERT( mInitialized );

    enum FormatStates
    {
        FORMAT_STATE_START,
        FORMAT_STATE_WAIT,
    };

    switch( mCommandState )
    {
        case FORMAT_STATE_START:
        {
#ifdef USE_SYNCHRONOUS_FUNCTIONS
            mLastCARDCommandReturnValue = CARDFormat( mActiveMemoryUnitSlot );
#else
            mLastCARDCommandReturnValue  = CARDFormatAsync( mActiveMemoryUnitSlot, NULL );
#endif
            mCommandState = FORMAT_STATE_WAIT;
            break;
        }

        case FORMAT_STATE_WAIT:
        {
#ifndef USE_SYNCHRONOUS_FUNCTIONS
            // When going asynchronous, you have to wait until the format is done.
            mLastCARDCommandReturnValue = CARDGetResultCode( mActiveMemoryUnitSlot );
            if( mLastCARDCommandReturnValue == CARD_RESULT_BUSY )
            {
                return;
            }
#endif

            switch( mLastCARDCommandReturnValue )
            {
                case CARD_RESULT_READY:
                    mpActiveMemoryUnit->mbFormatted = TRUE;
                    CompleteCommandAndWaitForUser( TRUE, QUAG_MU_EVERYTHING_IS_OK );
                    break;

                case CARD_RESULT_NOCARD:
                case CARD_RESULT_BUSY:
                case CARD_RESULT_FATAL_ERROR:
                case CARD_RESULT_ENCODING:
                case CARD_RESULT_BROKEN:
                case CARD_RESULT_IOERROR:
                    CompleteCommandAndWaitForUser( FALSE, ConvertCARDSysResultToQuagResult( mLastCARDCommandReturnValue ) );
                    break;

                default:
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_UNKNOWN_PROBLEM );
                    ASSERT( 0 );
                    break;
            }

            break;
        }
        
        default:
            ASSERT( 0 );
            break;
    }

}

//-------------------------------------------------------------------------------------------------------------------------------------
void GCMemoryUnitManager::UnFormatHandler( void )
{
}

//-------------------------------------------------------------------------------------------------------------------------------------
void GCMemoryUnitManager::SaveGameHandler( void )
{
    ASSERT( mInitialized );

    enum SAVE_GAME_STATES
    {
        SAVE_GAME_STATE_INIT,
        SAVE_GAME_STATE_WAIT_FOR_WRITE,
        SAVE_GAME_STATE_DONE,
    };

    switch( mCommandState )
    {
        case SAVE_GAME_STATE_INIT:
        {
            if( mpAvailableSavedGames[mCurrentGameType].mSavedGames[mCurrentGameTypeIndex].mbValid )
            {
                CompleteCommandAndWaitForUser(FALSE, QUAG_MU_FILE_ALREADY_EXISTS );
            }
            // Start by initializing the command sub state.
            else
            {
                s32 ErrorCode = OpenFile( TRUE );

                switch( ErrorCode )
                {
                    // The file was created, so it's time to prepare for the file transfer.
                    case QUAG_MU_EVERYTHING_IS_OK:
                    {
                        // Prepare for the write.
                        ErrorCode = PrepareForFileTransfer( TRUE, FALSE );

                        if( ErrorCode == QUAG_MU_EVERYTHING_IS_OK )
                        {
                            mCommandState = SAVE_GAME_STATE_WAIT_FOR_WRITE;
                        }
                        else
                        {
                            CompleteCommandAndWaitForUser( FALSE, ErrorCode );
                        }
                        
                        break;
                    }
                
                    // Card is in trouble, deal with it.
                    default:
                    {
                        CompleteCommandAndWaitForUser( FALSE, ErrorCode );
                        break;
                    }
                }
            }
            break;
        }

        case SAVE_GAME_STATE_WAIT_FOR_WRITE:
        {
            s32 ErrorCode;

            if( WriteFile( ErrorCode ) )
            {
                // First thing to do is close the file.
                if( ErrorCode == QUAG_MU_EVERYTHING_IS_OK )
                {
                    SingleSavedGameInfo* pSavedGameInfo;

                    // Complete the job by updating the available save game info with this save.
                    pSavedGameInfo = &mpAvailableSavedGames[mCurrentGameType].mSavedGames[mCurrentGameTypeIndex];
                    pSavedGameInfo->mbValid            = TRUE;
                    pSavedGameInfo->mSaveGameType      = mCurrentGameType;
                    pSavedGameInfo->mSaveGameTypeIndex = mCurrentGameTypeIndex;
                    GetBrowserText( pSavedGameInfo->mSaveGameTitle, MEMORY_UNIT_SAVE_GAME_TITLE_LENGTH );
                    
                    // Update the free blocks on the card.
                    UpdateFreeBlocksAndFiles( );
                    mCommandState = SAVE_GAME_STATE_DONE;
                }
                else
                {
                    CompleteCommandAndWaitForUser( FALSE, ErrorCode );
                }

                // Before you close the file, set the files CARD Data.
                CARDSetStatus(mActiveMemoryUnitSlot, mFileInfo.mGameCubeCARDFileInfo.fileNo, &mFileInfo.mGameCubeCARDStat );


                // Just close the file, who cares what the return is, you are closing the file.
                // If the card is removed, you will nab it somewhere else.
                CARDClose( &mFileInfo.mGameCubeCARDFileInfo );
                mFileInfo.Reset( );
            }
            break;
        }
        
        case SAVE_GAME_STATE_DONE:
        {
            f64 ElapsedTime;
            ElapsedTime = x_GetElapsedTime( mCommandStartTime );

            if( MEMORY_UNIT_IsConnected( ) )
            {
                if( ElapsedTime > GCMU_MIN_SAVED_GAME_TIME )
                {
                    CompleteCommandAndWaitForUser( TRUE, QUAG_MU_EVERYTHING_IS_OK );
                }
            }
            else
            {
                CompleteCommandAndWaitForUser( TRUE, QUAG_MU_NO_CARD );
            }


            break;
        }

        default:
            ASSERT( 0 );
            break;
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
void GCMemoryUnitManager::LoadGameHandler( void )
{
    ASSERT( mInitialized );

    enum LOAD_GAME_STATES
    {
        LOAD_GAME_STATE_INIT,
        LOAD_GAME_STATE_WAIT_FOR_READ,
        LOAD_GAME_STATE_DONE,
    };

    switch( mCommandState )
    {
        case LOAD_GAME_STATE_INIT:
        {
            s32 ErrorCode = OpenFile( FALSE );

            switch( ErrorCode )
            {
                // The file was created, so it's time to prepare for the file transfer.
                case QUAG_MU_EVERYTHING_IS_OK:
                {
                    // Prepare for the write.
                    ErrorCode = PrepareForFileTransfer( FALSE, FALSE );

                    if( ErrorCode == QUAG_MU_EVERYTHING_IS_OK )
                    {
                        mCommandState = LOAD_GAME_STATE_WAIT_FOR_READ;
                    }
                    else
                    {
                        CompleteCommandAndWaitForUser( FALSE, ErrorCode );
                    }
                    
                    break;
                }
            
                // Card is in trouble, deal with it.
                default:
                {
                    CompleteCommandAndWaitForUser( FALSE, ErrorCode );
                    break;
                }
            }
            break;
        }

        case LOAD_GAME_STATE_WAIT_FOR_READ:
        {
            s32 ErrorCode;

            if( ReadFile( ErrorCode ) )
            {
                // First thing to do is close the file.
                if( ErrorCode == QUAG_MU_EVERYTHING_IS_OK )
                {
                    UpdateFreeBlocksAndFiles( );
                    mCommandState = LOAD_GAME_STATE_DONE;
                }
                else
                {
                    CompleteCommandAndWaitForUser( FALSE, ErrorCode );
                }

                // Just close the file, who cares what the return is, you are closing the file.
                // If the card is removed, you will nab it somewhere else.
                CARDClose( &mFileInfo.mGameCubeCARDFileInfo );
                mFileInfo.Reset( );
            }
            break;
        }
        
        case LOAD_GAME_STATE_DONE:
        {
            f64 ElapsedTime;
            ElapsedTime = x_GetElapsedTime( mCommandStartTime );

            if( MEMORY_UNIT_IsConnected( ) )
            {
                if( ElapsedTime > GCMU_MIN_LOAD_GAME_TIME )
                {
                    CompleteCommandAndWaitForUser( TRUE, QUAG_MU_EVERYTHING_IS_OK );
                }
            }
            else
            {
                CompleteCommandAndWaitForUser( TRUE, QUAG_MU_NO_CARD );
            }
            break;
        }

        default:
            ASSERT( 0 );
            break;
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
void GCMemoryUnitManager::DeleteGameHandler( void )
{
    ASSERT( mInitialized );

    enum DeleteStates
    {
        DELETE_GAME_START,
        DELETE_GAME_WAIT,
        DELETE_GAME_TIMEOUT,
    };

    switch( mCommandState )
    {
        case DELETE_GAME_START:
        {
#ifdef USE_SYNCHRONOUS_FUNCTIONS
            mLastCARDCommandReturnValue = CARDDelete( mActiveMemoryUnitSlot, mFileInfo.mFileName );
#else
            mLastCARDCommandReturnValue = CARDDeleteAsync( mActiveMemoryUnitSlot, mFileInfo.mFileName, NULL );
#endif
            mCommandState = DELETE_GAME_WAIT;
            break;
        }

        case DELETE_GAME_WAIT:
        {
            s32 ErrorCode;

#ifndef USE_SYNCHRONOUS_FUNCTIONS
            mLastCARDCommandReturnValue = CARDGetResultCode( mActiveMemoryUnitSlot );
            if( mLastCARDCommandReturnValue == CARD_RESULT_BUSY )
                return;
#endif
            switch( mLastCARDCommandReturnValue )
            {
                case CARD_RESULT_READY:
                    mCommandState = DELETE_GAME_TIMEOUT;
                    UpdateFreeBlocksAndFiles( );
                    break;

                case CARD_RESULT_NOCARD:
                case CARD_RESULT_BUSY:
                case CARD_RESULT_NOFILE:
                case CARD_RESULT_NOPERM:
                case CARD_RESULT_FATAL_ERROR:
                case CARD_RESULT_ENCODING:
                case CARD_RESULT_BROKEN:
                case CARD_RESULT_IOERROR:
                    ErrorCode = ConvertCARDSysResultToQuagResult( mLastCARDCommandReturnValue );
                    CompleteCommandAndWaitForUser( FALSE, ErrorCode );
                    break;
                
                default:
                    ASSERT( 0 );
                    break;
            }
            break;
        }

        case DELETE_GAME_TIMEOUT:
        {
            f64 ElapsedTime;
            ElapsedTime = x_GetElapsedTime( mCommandStartTime );

            if( MEMORY_UNIT_IsConnected( ) )
            {
                if( ElapsedTime > GCMU_MIN_DELETE_TIME )
                {
                    // Complete the command by removing this game from the available game list.
                    // and updating the status of the cards sizes.
                    mpAvailableSavedGames[mCurrentGameType].Clear( mCurrentGameTypeIndex );

                    // Return your success.
                    CompleteCommandAndWaitForUser( TRUE, QUAG_MU_EVERYTHING_IS_OK );
                }
            }
            else
            {
                CompleteCommandAndWaitForUser( TRUE, QUAG_MU_NO_CARD );
            }

            break;
        }

        default:
            ASSERT( 0 );
            break;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
// Utility Functions
//-------------------------------------------------------------------------------------------------------------------------------------
void GCMemoryUnitManager::ProcessCardConnected( GCMemoryUnit* pMemoryUnit )
{
    ASSERT( mInitialized );

    // Identify that it's connected
    pMemoryUnit->mbConnected = TRUE;
    pMemoryUnit->mbFormatted = FALSE;
    pMemoryUnit->mbMounted   = FALSE;
    pMemoryUnit->mFreeFiles  = 0;
    pMemoryUnit->mFreeBytes  = 0;
    pMemoryUnit->mFreeBlocks = 0;

    // Does the card have an 8K Block size?
    pMemoryUnit->mbCompatible = pMemoryUnit->mSectorSize == GCMU_SUPPORTED_SECTOR_SIZE;

#ifdef SHOW_GCMU_DIAGNOSTICS
    if( pMemoryUnit->mbCompatible )
    {
        MC_PRINT( "%s was Inserted", pMemoryUnit->mName );
    }
    else
    {
        MC_PRINT( "%s was Detected, but has invalid Block Size of %dK.", pMemoryUnit->mName, pMemoryUnit->mSectorSize / 1024 );
    }
#endif 

}


//-------------------------------------------------------------------------------------------------------------------------------------
void GCMemoryUnitManager::ProcessCardDisconnected( GCMemoryUnit* pMemoryUnit )
{
    ASSERT( mInitialized );

    // If the memory unit that is disconnected is the currently selected memory unit make sure that it's released.
    if( pMemoryUnit == mpActiveMemoryUnit )
    {
        // Now free the active memory unit.
        ReleaseActiveMemoryUnit( );
    }

    MC_PRINT( "%s was Removed", pMemoryUnit->mName );
    pMemoryUnit->Reset( );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 GCMemoryUnitManager::GetSlotFromMemoryUnitID( s32 MemUnitID )
{
    s32 HardwareSlot = -1;

    switch( MemUnitID )
    {
        case QUAG_MEMORY_UNIT_ID_0:     HardwareSlot = GAMECUBE_MEMCARD_SLOTA;      break;
        case QUAG_MEMORY_UNIT_ID_1:     HardwareSlot = GAMECUBE_MEMCARD_SLOTB;      break;

        default:
            ASSERT( 0 );
            break;
    }

    return HardwareSlot;
}

//-------------------------------------------------------------------------------------------------------------------------------------
s32 GCMemoryUnitManager::GetMemoryUnitIDFromSlot( s32 CurrentSlot )
{
    s32 MemoryUnitID = -1;

    switch( CurrentSlot )
    {
        case GAMECUBE_MEMCARD_SLOTA:    MemoryUnitID = QUAG_MEMORY_UNIT_ID_0;      break;
        case GAMECUBE_MEMCARD_SLOTB:    MemoryUnitID = QUAG_MEMORY_UNIT_ID_1;      break;

        default:
            ASSERT( 0 );
            break;
    }

    return MemoryUnitID;
}


//-------------------------------------------------------------------------------------------------------------------------------------
void GCMemoryUnitManager::ClearAvailableGamesList( void )
{
    s32 SaveGameType;
    s32 index;

    // Clear out all local information about what games are saved on which card.
    for( SaveGameType = 0; SaveGameType < mNumberOfSaveGameTypes; SaveGameType++ )
    {
        for( index = 0; index < MEMORY_UNIT_MAX_SAVE_COUNT_PER_TYPE; index++ )
        {
            mpAvailableSavedGames[SaveGameType].Clear( index );
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
void GCMemoryUnitManager::GetUserDataFileName( s32 SaveGameType, s32 TypeIndex, char* Buffer )
{
    ASSERT( mInitialized );
    ASSERT( SaveGameType < mNumberOfSaveGameTypes );
    ASSERT( TypeIndex < mpSaveGameTypes[SaveGameType].mMaxNumberOfSaves );

    x_sprintf( Buffer, "%s%ld", mpSaveGameTypes[SaveGameType].mUserDataFileInfo.mUserDataSaveFileName, TypeIndex );

    ASSERT( x_strlen( Buffer ) < GCMU_USER_DATA_FILENAME_LENGTH );
}




//-------------------------------------------------------------------------------------------------------------------------------------
s32 GCMemoryUnitManager::ConvertCARDSysResultToQuagResult( s32 CARDResult )
{
    s32 QuagResult;

    switch( CARDResult )
    {
        case CARD_RESULT_READY:                 QuagResult = QUAG_MU_EVERYTHING_IS_OK;          break;
        case CARD_RESULT_BUSY:                  QuagResult = QUAG_MU_SYSTEM_BUSY;               break;
        case CARD_RESULT_WRONGDEVICE:           QuagResult = QUAG_MU_WRONG_DEVICE;              break;
        case CARD_RESULT_NOCARD:                QuagResult = QUAG_MU_NO_CARD;                   break;
        case CARD_RESULT_NOFILE:                QuagResult = QUAG_MU_FILE_NOT_FOUND;            break;
        case CARD_RESULT_IOERROR:               QuagResult = QUAG_MU_CARD_PHYSICALLY_DAMAGED;   break;
        case CARD_RESULT_BROKEN:                QuagResult = QUAG_MU_FILE_SYSTEM_DAMAGED;       break;
        case CARD_RESULT_EXIST:                 QuagResult = QUAG_MU_FILE_ALREADY_EXISTS;       break;
        case CARD_RESULT_NOENT:                 QuagResult = QUAG_MU_TOO_MANY_FILES_ON_CARD;    break;
        case CARD_RESULT_INSSPACE:              QuagResult = QUAG_MU_NOT_ENOUGH_FREE_SPACE;     break;
        case CARD_RESULT_NOPERM:                QuagResult = QUAG_MU_FILE_ACCESS_DENIED;        break;
        case CARD_RESULT_LIMIT:                 QuagResult = QUAG_MU_OVERSIZED_READ_WRITE;      break;
        case CARD_RESULT_NAMETOOLONG:           QuagResult = QUAG_MU_INVALID_FILENAME;          break;
        case CARD_RESULT_ENCODING:              QuagResult = QUAG_MU_INVALID_FORMAT;            break;
        case CARD_RESULT_CANCELED:              QuagResult = QUAG_MU_UNKNOWN_PROBLEM;           break;
        case CARD_RESULT_FATAL_ERROR:           QuagResult = QUAG_MU_UNKNOWN_PROBLEM;           break;

        default:
            QuagResult = QUAG_MU_UNKNOWN_PROBLEM;
            ASSERT( 0 );
            break;

    }

    return QuagResult;
}


//-------------------------------------------------------------------------------------------------------------------------------------
char* GCMemoryUnitManager::GetCommandNameAsString( s32 CommandID )
{
    static char* CommandNames[] =
    {
        "GCMU_CMD_NONE",
        "GCMU_CMD_MOUNT",
        "GCMU_CMD_FORMAT",
        "GCMU_CMD_SAVE",
        "GCMU_CMD_LOAD",
        "GCMU_CMD_DELETE",
        "UNKNOWN_CMD",
    };

    if( CommandID >= GCMU_CMD_FIRST && CommandID <= GCMU_CMD_LAST )
    {
        return CommandNames[CommandID];
    }
    else
    {
        return CommandNames[GCMU_CMD_LAST + 1];
    }
}



//-------------------------------------------------------------------------------------------------------------------------------------
// File transfer functions.
//-------------------------------------------------------------------------------------------------------------------------------------
s32 GCMemoryUnitManager::PrepareForFileTransfer( xbool bWrite, xbool bReadingOnlyBrowseInfo )
{
    ASSERT( mInitialized );

    CARDFileInfo* pCARDFileInfo = &mFileInfo.mGameCubeCARDFileInfo;
    CARDStat*     pCARDStat     = &mFileInfo.mGameCubeCARDStat;
    s32           IconFrameCount;
    s32           RetVal;
    s32           BytesTransferred;

    mFileInfo.mSectorTransferCount   = 0; // Reset the sector count to 0.
    mFileInfo.mSectorBufferOffset    = 0; // Reset the Current sector offset.
    mFileInfo.mUserBufferOffset      = 0; // Reset the user offset pointer as well.
    mFileInfo.mMemCardOffset         = 0; // Reset the memory card read/write offset.
    mFileInfo.mSectorBytesTransferred = 0; // No bytes have been transferred.

    mFileInfo.mbOnlyReadingTheBrowserText = bReadingOnlyBrowseInfo;

    // If this file is for writting, setup the file header. (icon, browser comments etc...)
    if( bWrite )
    {
        // Get the current status on the opened file.
        RetVal = CARDGetStatus( mActiveMemoryUnitSlot, pCARDFileInfo->fileNo, pCARDStat);
        if( RetVal != CARD_RESULT_READY )
        {
            return ProcessReadWriteComplete( RetVal );
        }

        // Identify the offset into the data block where the start of the Browser Comments will be.
    	CARDSetCommentAddress( pCARDStat, GCMU_FILE_INFO_BROWSER_COMMENT_OFFSET );

        // Identify the offset into the data block where the start of the Icon data will be.
    	CARDSetIconAddress( pCARDStat, GCMU_FILE_INFO_ICON_DATA_OFFSET );

        // Setup the banner type.
        CARDSetBannerFormat( pCARDStat, CARD_STAT_BANNER_NONE );

        // Identify how to animate the icon.
        CARDSetIconAnim( pCARDStat, CARD_STAT_ANIM_LOOP );

        // Setup the icon informations.
        for( IconFrameCount = 0; IconFrameCount < mIconData.mNumberOfIconFrames; IconFrameCount++ )
        {
            CARDSetIconFormat(pCARDStat, IconFrameCount, CARD_STAT_ICON_C8);
            CARDSetIconSpeed (pCARDStat, IconFrameCount, mIconData.mIconAnimSpeed );
        }

        // Fill the sector with the browser comments, and the icon data.
        // You shouldn't need to get the return value from the PackDataIntoSector Function at this point,
        // since the header must fall within the first few Kbytes.
        PackDataIntoSector( (byte*)mBrowserTitleText,   GCMU_BROWSER_LINE_LENGTH, &BytesTransferred );
        PackDataIntoSector( (byte*)mBrowserCommentText, GCMU_BROWSER_LINE_LENGTH, &BytesTransferred );
        PackDataIntoSector( mIconData.mpIconData, mIconData.mIconDataSize, &BytesTransferred );

        // Identify that there are X number of user bytes still required to be written to the file.
        mFileInfo.mUserBytesToGo = mFileInfo.mFileDataSize; // mFileDataSize is filled on the open.
        mFileInfo.mbSectorLimit  = FALSE;

        mFileInfo.mFileTransferStatus = FILE_TRANSFER_STATUS_WRITE;
    }
    else
    {
        // Get the current status on the opened file.
        RetVal = CARDGetStatus( mActiveMemoryUnitSlot, pCARDFileInfo->fileNo, pCARDStat );
        if( RetVal != CARD_RESULT_READY )
        {
            return ProcessReadWriteComplete( RetVal );
        }

        // Do you only want to read the browser text?
        if( bReadingOnlyBrowseInfo )
        {
            mFileInfo.mSectorBufferOffset = 0;
            mFileInfo.mUserBytesToGo = GCMU_BROWSER_LINE_LENGTH * 2;
        }
        else
        {
            mFileInfo.mSectorBufferOffset = pCARDStat->offsetData;
            mFileInfo.mUserBytesToGo = mFileInfo.mFileDataSize; // mFileDataSize is filled on the open.
        }

        // On a read the sector limit is true since the sector is empty.
        mFileInfo.mbSectorLimit = TRUE;
        mFileInfo.mFileTransferStatus = FILE_TRANSFER_STATUS_READ;
    }

    return QUAG_MU_EVERYTHING_IS_OK;
}

//-------------------------------------------------------------------------------------------------------------------------------------
xbool GCMemoryUnitManager::PackDataIntoSector(  byte* pDataPointer, s32 DesiredBytesToAdd, s32* pActualBytesAdded )
{
    ASSERT( mInitialized );

    s32 SectorSpaceRemaining;
    s32 BytesCopied;
    
    ASSERT( pDataPointer );

    // Determine the free space in the sector
    SectorSpaceRemaining = GCMU_SUPPORTED_SECTOR_SIZE - mFileInfo.mSectorBufferOffset;

    // Determine how many bytes requested can be added to the sector.  The most that can be added to the sector
    // is the minimum of either the number of bytes to Add, or the sectors free space.  Save a copy of this number to
    // give to the calling function.
    BytesCopied = MIN( SectorSpaceRemaining, DesiredBytesToAdd );
    *pActualBytesAdded = BytesCopied;

    // Add the maximum amount of data that you can to the sector buffer.
    x_memcpy( &mFileInfo.mpSectorDataBuffer[mFileInfo.mSectorBufferOffset],  // Destination.
              pDataPointer,                                                                  // Source
              BytesCopied );                                                                 // Byte Count

    mFileInfo.mSectorBufferOffset += BytesCopied;
    SectorSpaceRemaining -= BytesCopied;

    // Identify if the sector is now full.
    if( SectorSpaceRemaining == 0 )
        return TRUE;
    else
        return FALSE;
}

//-------------------------------------------------------------------------------------------------------------------------------------
xbool GCMemoryUnitManager::StripDataFromSector( byte* pDataPointer, s32 DesiredBytesToGet, s32* pActualBytesGiven )
{
    ASSERT( mInitialized );

    s32   SectorSpaceLeftToRead;
    s32   BytesCopied;
    
    ASSERT( pDataPointer );

    // Determine the free space in the sector
    SectorSpaceLeftToRead = GCMU_SUPPORTED_SECTOR_SIZE - mFileInfo.mSectorBufferOffset;

    // Determine the number of bytes that can be read back from this sector.  
    // The most that can be read back is the minimum of either the number of bytes to get, or the SectorSpaceLeftToRead.
    // Save a copy of this number to give to the calling function.
    BytesCopied = MIN( SectorSpaceLeftToRead, DesiredBytesToGet );
    *pActualBytesGiven = BytesCopied;

    // Pull out the correct amount of data from the sector.
    x_memcpy( pDataPointer,                                                          // Destination.
              &mFileInfo.mpSectorDataBuffer[mFileInfo.mSectorBufferOffset],  // Source
              BytesCopied );                                                         // Byte Count

    mFileInfo.mSectorBufferOffset += BytesCopied;
    SectorSpaceLeftToRead -= BytesCopied;

    // Identify if the sector has been completely depleted.
    if( SectorSpaceLeftToRead == 0 )
        return TRUE;
    else
        return FALSE;
}                                              

//-------------------------------------------------------------------------------------------------------------------------------------
s32 GCMemoryUnitManager::OpenFile( xbool Write )
{
    ASSERT( mInitialized );

    // Set all the data required about how to open the file.
    GetUserDataFileName( mCurrentGameType, mCurrentGameTypeIndex, mFileInfo.mFileName );
    mFileInfo.mFileDataSize = mpSaveGameTypes[mCurrentGameType].mUserDataFileInfo.mSizeOfUserDataFile;

    mFileInfo.mbWriteAccess = Write;

    if( Write )
    {
        if( CalculateRequiredSizeForSaveGameType( mCurrentGameType, &mFileInfo.mTotalFileSize, FALSE ) == QUAG_MU_EVERYTHING_IS_OK )
        {
            // Convert the block value returned from the CalculateRequiredSizeForSaveGameType back into bytes.
            mFileInfo.mTotalFileSize *= GCMU_SUPPORTED_SECTOR_SIZE;

            mLastCARDCommandReturnValue = CARDCreate( mActiveMemoryUnitSlot, mFileInfo.mFileName, mFileInfo.mTotalFileSize, &mFileInfo.mGameCubeCARDFileInfo );
            return ProcessOpenCreateComplete( mLastCARDCommandReturnValue );
        }
        else
        {
            return QUAG_MU_NOT_ENOUGH_FREE_SPACE;
        }
    }
    else
    {
        mLastCARDCommandReturnValue = CARDOpen( mActiveMemoryUnitSlot, mFileInfo.mFileName, &mFileInfo.mGameCubeCARDFileInfo );
        return ProcessOpenCreateComplete( mLastCARDCommandReturnValue );
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 GCMemoryUnitManager::ProcessOpenCreateComplete( s32 CARDReturn )
{
    ASSERT( mInitialized );

    s32 ErrorCode;

    switch( CARDReturn )
    {
        case CARD_RESULT_READY:
            mFileInfo.mbOpen = TRUE;
            ErrorCode = QUAG_MU_EVERYTHING_IS_OK;
            break;

        case CARD_RESULT_NOCARD:
        case CARD_RESULT_BUSY:
        case CARD_RESULT_NOFILE:
        case CARD_RESULT_NOPERM:
        case CARD_RESULT_EXIST:
        case CARD_RESULT_NOENT:
        case CARD_RESULT_INSSPACE:
        case CARD_RESULT_NAMETOOLONG:
        case CARD_RESULT_FATAL_ERROR:
        case CARD_RESULT_ENCODING:
        case CARD_RESULT_BROKEN:
        case CARD_RESULT_IOERROR:
            ErrorCode = ConvertCARDSysResultToQuagResult( CARDReturn );
            break;

        default:
            ErrorCode = QUAG_MU_UNKNOWN_PROBLEM;
            ASSERT( 0 );
            break;
    }

    return ErrorCode;
}

//-------------------------------------------------------------------------------------------------------------------------------------
xbool GCMemoryUnitManager::ReadFile( s32& ErrorCode )
{
    ASSERT( mInitialized );

    switch( mFileInfo.mFileTransferStatus )
    {
        case FILE_TRANSFER_STATUS_READ:
        {
            // Read out one sector from the memory card file.
#ifdef USE_SYNCHRONOUS_FUNCTIONS
            mLastCARDCommandReturnValue = CARDRead(&mFileInfo.mGameCubeCARDFileInfo,
                                                    mFileInfo.mpSectorDataBuffer,
                                                    GCMU_SUPPORTED_SECTOR_SIZE,
                                                    mFileInfo.mMemCardOffset );
#else
            mLastCARDCommandReturnValue = CARDReadAsync(&mFileInfo.mGameCubeCARDFileInfo,
                                                         mFileInfo.mpSectorDataBuffer,
                                                         GCMU_SUPPORTED_SECTOR_SIZE,
                                                         mFileInfo.mMemCardOffset,
                                                         NULL); // No callback required, doing it all here.
#endif
            // So the read request has been made, if it's and error condition then get out.
            mFileInfo.mFileTransferStatus = FILE_TRANSFER_STATUS_WAIT_FOR_READ_TO_COMPLETE;
            break;
        }

        case FILE_TRANSFER_STATUS_WAIT_FOR_READ_TO_COMPLETE:
        {
#ifndef USE_SYNCHRONOUS_FUNCTIONS
            // When going asynchronous, you have to wait until the read is done.
            mLastCARDCommandReturnValue = CARDGetResultCode( mActiveMemoryUnitSlot );
            if( mLastCARDCommandReturnValue == CARD_RESULT_BUSY )
            {
                return FALSE;
            }
#endif
            // If the read was a success, then increment the number of sectors that have been transferred.
            if( mLastCARDCommandReturnValue == CARD_RESULT_READY )
            {
                s32 BytesReadFromSector;

                // Increment the number of sectors that have been transferred, then update where the 
                // next read should grab data from.
                mFileInfo.mSectorTransferCount++;
                mFileInfo.mMemCardOffset = mFileInfo.mSectorTransferCount * GCMU_SUPPORTED_SECTOR_SIZE;

                // Now copy the data from the sector to the user buffer.
                mFileInfo.mbSectorLimit = StripDataFromSector( &mFileInfo.mpUserDataBuffer[mFileInfo.mUserBufferOffset],
                                                                mFileInfo.mUserBytesToGo,
                                                               &BytesReadFromSector );
                                 
                // Update the byte counts.
                mFileInfo.mUserBytesToGo    -= BytesReadFromSector;
                mFileInfo.mUserBufferOffset += BytesReadFromSector;

                // Reset the buffer offset, since the sector has been read from.
                mFileInfo.mSectorBufferOffset = 0; // Reset the sector buffer offset.

                // Test to see if the read is complete.
                if( mFileInfo.mUserBytesToGo == 0 )
                {
                    ErrorCode = QUAG_MU_EVERYTHING_IS_OK;
                    return TRUE;
                }
                // Continue the read.
                else if( mFileInfo.mUserBytesToGo > 0 )
                {
                    mFileInfo.mFileTransferStatus = FILE_TRANSFER_STATUS_READ;
                }
                // This should never happen.
                else
                {
                    ASSERT( 0 );
                    break;
                }
            }
            // If there was an error, handle it.
            else
            {
                ErrorCode = ProcessReadWriteComplete( mLastCARDCommandReturnValue );
                return TRUE;
            }
            
            break;
        }

        default:
        {
            ASSERT( 0 );
            break;
        }
    }

    // The read wasn't done yet.
    return FALSE;
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool GCMemoryUnitManager::WriteFile( s32& ErrorCode )
{
    ASSERT( mInitialized );

    switch( mFileInfo.mFileTransferStatus )
    {
        case FILE_TRANSFER_STATUS_WRITE:
        {
            // If there is still user data to be written to, then keep at it.
            if( mFileInfo.mUserBytesToGo )
            {
                // Pack some of the user data into the sector buffer.
                mFileInfo.mbSectorLimit = PackDataIntoSector( &mFileInfo.mpUserDataBuffer[mFileInfo.mUserBufferOffset],
                                                               mFileInfo.mUserBytesToGo,
                                                              &mFileInfo.mSectorBytesTransferred );

                // If either the sector was packed full, or all of the user data that was left to write made
                // it into the sector, then execute the write request.
                if( mFileInfo.mbSectorLimit || mFileInfo.mUserBytesToGo == mFileInfo.mSectorBytesTransferred )
                {
                    mFileInfo.mMemCardOffset = mFileInfo.mSectorTransferCount * GCMU_SUPPORTED_SECTOR_SIZE;

                    
#ifdef USE_SYNCHRONOUS_FUNCTIONS
                    mLastCARDCommandReturnValue = CARDWrite(&mFileInfo.mGameCubeCARDFileInfo,
                                                            mFileInfo.mpSectorDataBuffer,
                                                            GCMU_SUPPORTED_SECTOR_SIZE,
                                                            mFileInfo.mMemCardOffset );
#else
                    mLastCARDCommandReturnValue = CARDWriteAsync(&mFileInfo.mGameCubeCARDFileInfo,
                                                                  mFileInfo.mpSectorDataBuffer,
                                                                  GCMU_SUPPORTED_SECTOR_SIZE,
                                                                  mFileInfo.mMemCardOffset,
                                                                  NULL); // No callback required, doing it all here.
#endif
                    // So the read request has been made, if it's and error condition then get out.
                    mFileInfo.mFileTransferStatus = FILE_TRANSFER_STATUS_WAIT_FOR_WRITE_TO_COMPLETE;
                    break;
                }
                else
                {
                    // Either the sector should be filled, or the data is always met, otherwise, somtin is up.
                    ASSERT( 0 );
                    break;
                }
            }
        }

        case FILE_TRANSFER_STATUS_WAIT_FOR_WRITE_TO_COMPLETE:
        {
#ifndef USE_SYNCHRONOUS_FUNCTIONS
            // When going asynchronous, you have to wait until the read is done.
            mLastCARDCommandReturnValue = CARDGetResultCode( mActiveMemoryUnitSlot );
            if( mLastCARDCommandReturnValue == CARD_RESULT_BUSY )
            {
                return FALSE;
            }
#endif
            // If the read was a success, then increment the number of sectors that have been transferred.
            if( mLastCARDCommandReturnValue == CARD_RESULT_READY )
            {
                mFileInfo.mSectorTransferCount++;  // Increment the number of sectors that have been transferred.
                                 
                // Update the byte counts.
                mFileInfo.mUserBytesToGo    -= mFileInfo.mSectorBytesTransferred;
                mFileInfo.mUserBufferOffset += mFileInfo.mSectorBytesTransferred;

                // Reset the buffer offset, since the sector has been read from.
                mFileInfo.mSectorBufferOffset = 0; // Reset the sector buffer offset.

                // Is the write complete.
                if( mFileInfo.mUserBytesToGo == 0 )
                {
                    ErrorCode = QUAG_MU_EVERYTHING_IS_OK;
                    return TRUE;
                }
                // Continue the read.
                else if( mFileInfo.mUserBytesToGo > 0 )
                {
                    mFileInfo.mFileTransferStatus = FILE_TRANSFER_STATUS_WRITE;
                }
                // This should never happen.
                else
                {
                    ASSERT( 0 );
                    break;
                }
            }
            // If there was an error, handle it.
            else
            {
                ErrorCode = ProcessReadWriteComplete( mLastCARDCommandReturnValue );
                return TRUE;
            }
            
            break;
        }

        default:
        {
            ASSERT( 0 );
            break;
        }
    }

    // The read wasn't done yet.
    return FALSE;
}

//-------------------------------------------------------------------------------------------------------------------------------------
s32 GCMemoryUnitManager::ProcessReadWriteComplete( s32 CARDReturn )
{
    ASSERT( mInitialized );

    s32 ErrorCode;

    switch( CARDReturn )
    {
        case CARD_RESULT_READY:
            ErrorCode = QUAG_MU_EVERYTHING_IS_OK;
            break;

        case CARD_RESULT_NOCARD:
        case CARD_RESULT_BUSY:
        case CARD_RESULT_NOFILE:
        case CARD_RESULT_NOPERM:
        case CARD_RESULT_LIMIT:
        case CARD_RESULT_CANCELED:
        case CARD_RESULT_FATAL_ERROR:
        case CARD_RESULT_ENCODING:
        case CARD_RESULT_BROKEN:
        case CARD_RESULT_IOERROR:
            ErrorCode = ConvertCARDSysResultToQuagResult( CARDReturn );
            break;

        default:
            ASSERT( 0 );
            ErrorCode = QUAG_MU_UNKNOWN_PROBLEM;
            break;
    }

    return ErrorCode;    
}





//-------------------------------------------------------------------------------------------------------------------------------------
// Icon Functions
//-------------------------------------------------------------------------------------------------------------------------------------
void GCMemoryUnitManager::BuildIconFromResourceData( GCMUResourceIconInfo* pResourceData, byte* RawIconData )
{
    mIconData.ReleaseIconData( );

    mIconData.mIconDataSize       = pResourceData->mIconDataSize;
    mIconData.mIconAnimSpeed      = pResourceData->mIconAnimSpeed;
    mIconData.mNumberOfIconFrames = pResourceData->mNumberOfIconFrames;
    
    mIconData.mpIconData = new byte[mIconData.mIconDataSize];
    ASSERT( mIconData.mpIconData );

    // Well at this point, it's time to copy the data from the resource buffer to the local one.
    x_memcpy( mIconData.mpIconData, RawIconData, mIconData.mIconDataSize );
}
