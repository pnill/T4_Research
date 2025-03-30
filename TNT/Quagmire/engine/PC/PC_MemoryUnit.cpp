//=====================================================================================================================================
// Includes
//=====================================================================================================================================
#include "PC_MemoryUnit.hpp"


//=====================================================================================================================================
// Forward Declarations
//=====================================================================================================================================


//=====================================================================================================================================
// Definitions
//=====================================================================================================================================
#define PC_USER_HARDDISK_LETTER               'U'
#define PC_DRIVE_BLOCK_SIZE                   (16*1024)
#define PC_SAVE_GAME_LOGO_IMAGE_FILE_NAME     "saveimage.xbx"
#define PC_READ_WRITE_CHUNK_SIZE              (PC_DRIVE_BLOCK_SIZE * 8)
#define PC_SAVE_GAME_FILENAME                 "ASB2003Data"
#define PC_MAX_SAVED_GAMES                    4096
#define PC_MIN_SAVED_GAME_TIME                3.0
#define PC_MIN_DELETE_TIME                    3.0
#define PC_MIN_LOAD_GAME_TIME                 2.0

#ifdef X_DEBUG
#define SHOW_PC_DIAGNOSTICS
#endif

#ifdef SHOW_PC_DIAGNOSTICS
    void MC_PRINT( char* FormatString, ... )
    {
        static char TempBuffer[300];

        x_va_list   Args;
        x_va_start( Args, FormatString );

        x_vsprintf( TempBuffer, FormatString, Args );

        x_printf( "[PCMU] --> " );
        x_printf( TempBuffer );
        x_printf( "\n" );
    }
#else
    void MC_PRINT( char* FormatString, ... )
    {
    }
#endif


//-------------------------------------------------------------------------------------------------------------------------------------
enum PC_LocalMemoryCardCommands
{
    PCMU_CMD_FIRST = 0,
    PCMU_CMD_NONE = PCMU_CMD_FIRST,
    PCMU_CMD_MOUNT,
    PCMU_CMD_FORMAT,
    PCMU_CMD_SAVE,
    PCMU_CMD_LOAD,
    PCMU_CMD_DELETE,
    PCMU_CMD_LAST = PCMU_CMD_DELETE,
};



//=====================================================================================================================================
// Globals
//=====================================================================================================================================
PCMemoryUnitManager* gpPCMemoryUnitManager = NULL;

//=====================================================================================================================================
// MEMORY_UNIT Interface functions.
//=====================================================================================================================================
xbool MEMORY_UNIT_Create( char* ApplicationName )
{
    // Application Name is ignored.
    
    // Create the manager.
    gpPCMemoryUnitManager = new PCMemoryUnitManager;

    if( gpPCMemoryUnitManager )
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
    if( gpPCMemoryUnitManager->Initialize( MemoryUnitResourceFilename ) )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
void MEMORY_UNIT_Destroy( void )
{
    if( gpPCMemoryUnitManager )
    {
        delete gpPCMemoryUnitManager;
        gpPCMemoryUnitManager = NULL;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
void MEMORY_UNIT_Update( void )
{
    // As long as this thing is created, run with it.
    if( gpPCMemoryUnitManager )
    {
        gpPCMemoryUnitManager->Update( );
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
void MEMORY_UNIT_EnableScanning( void )
{
    return; // PC does not use this option right now.
}


//-------------------------------------------------------------------------------------------------------------------------------------
void MEMORY_UNIT_DisableScanning( void )
{
    return; // PC does not use this option right now.
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool MEMORY_UNIT_IsWriteInProgress( void )
{
    return gpPCMemoryUnitManager->IsWriteInProgress( );
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool MEMORY_UNIT_IsConnected( s32 MemoryUnitID )
{
    // Since the only memory unit is the HardDisk, then this thing had better be there.
    ASSERT( ( MemoryUnitID == QUAG_MEMORY_UNIT_ID_0 ) || (MemoryUnitID == -1 ) );
    return TRUE;
}

//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_GetSaveGameUserDataSizeInBytes( s32 SaveGameType )
{
    return gpPCMemoryUnitManager->GetUserDataByteSize( SaveGameType );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_SelectActiveMemoryUnit( s32 MemoryUnitID, byte* pFileIOBuffer, s32 BufferSize )
{
    ASSERT( ( MemoryUnitID == QUAG_MEMORY_UNIT_ID_0 ) );
    return gpPCMemoryUnitManager->SelectHardDisk( pFileIOBuffer, BufferSize );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_ReleaseActiveMemoryUnit( void )
{    
    return gpPCMemoryUnitManager->ReleaseHardDisk( );
}


//-------------------------------------------------------------------------------------------------------------------------------------
char* MEMORY_UNIT_GetNameOfActiveMemoryUnit( void )
{
    return gpPCMemoryUnitManager->GetNameOfHardDisk( );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_GetFreeBlocks( void )
{
    return gpPCMemoryUnitManager->GetFreeBlocksRemainingOnHardDisk( );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_GetFreeFiles( void )
{
    return gpPCMemoryUnitManager->GetFreeFilesRemainingOnHardDisk( );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_GetRequiredBlocksFreeToSaveGameType( s32 SaveGameType, s32* pRequiredBlocks, xbool bWorstCase )
{
    ASSERT( pRequiredBlocks );
    return gpPCMemoryUnitManager->CalculateRequiredSizeForSaveGameType( SaveGameType, pRequiredBlocks, bWorstCase );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_Format( void )
{
    return gpPCMemoryUnitManager->Format( );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_SaveGame( s32 SaveGameType, s32 TypeOffset, void* pUserData, char* SaveGameBrowserInfo, s32 BrowserInfoBreakOffset )
{
    return gpPCMemoryUnitManager->SaveGame( SaveGameType, TypeOffset, pUserData, SaveGameBrowserInfo, BrowserInfoBreakOffset );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_LoadGame( s32 SaveGameType, s32 TypeOffset, void* pUserData )
{   
    return gpPCMemoryUnitManager->LoadGame( SaveGameType, TypeOffset, pUserData );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_DeleteGame( s32 SaveGameType, s32 TypeOffset )
{   
    return gpPCMemoryUnitManager->DeleteGame( SaveGameType, TypeOffset );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_GetSaveGameList( s32 SaveGameType, s32* pMaxSaveGameTypeCount, SingleSavedGameInfo** pSaveGameListings )
{   
    return gpPCMemoryUnitManager->GetSaveGameList( SaveGameType, pMaxSaveGameTypeCount, pSaveGameListings );
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool MEMORY_UNIT_GetNextEvent( MemoryUnitEvent* pEvent )
{   
    ASSERT( pEvent );

    if( gpPCMemoryUnitManager->IsCommandComplete( ) )
    {
        // Do a cheap wrap here.  If the card isn't connected, always return the NO card error.
        if( MEMORY_UNIT_IsConnected( ) )
        {
            gpPCMemoryUnitManager->GetLastCommandValues( &pEvent->mReturnValue, &pEvent->mErrorCode );

            if( pEvent->mErrorCode == QUAG_MU_EVERYTHING_IS_OK )
                pEvent->mEventType = QUAG_MU_EVENT_COMPLETE;
            else
                pEvent->mEventType = QUAG_MU_EVENT_FAILED;
        }
        else
        {
            gpPCMemoryUnitManager->GetLastCommandValues( NULL, NULL );
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


//=====================================================================================================================================
// PCMemoryUnit
//=====================================================================================================================================
PCMemoryUnit::PCMemoryUnit( void )
{
    mUnitID = -1;
    Reset( );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void PCMemoryUnit::Initialize( s32 UnitID )
{
    // Create the name for this unit.
    mUnitID = UnitID;

    if( mUnitID == QUAG_MEMORY_UNIT_ID_0 )
    {
        SetDriveLetter( 'U' );
        SetName( PC_HARD_DISK_NAME );
    }
    else
    {
        // We are only supporting the hard disk for now.
        ASSERT( 0 );
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
void PCMemoryUnit::Reset( void )
{
    mbConnected = FALSE;
    mbFormatted = FALSE;
    mbMounted   = FALSE;

    mTotalBytes     = 0;
    mAvailableBytes = 0;
    mUsedBytes      = 0;
    mClusterSize    = 0;
    mSectorSize     = 0;
    mTotalBlocks    = 0;
    mFreeBlocks     = 0;
    mUsedBlocks     = 0;
}


//-------------------------------------------------------------------------------------------------------------------------------------
void PCMemoryUnit::SetName( char* NewPersonalizedName )
{
    x_strncpy( mName, NewPersonalizedName, MEMORY_UNIT_PERSONALIZED_NAME_SIZE );
}


//-------------------------------------------------------------------------------------------------------------------------------------
void PCMemoryUnit::SetDriveLetter( char c )
{
    x_sprintf( mDriveLetter, "%c:\\", c );
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool PCMemoryUnit::UpdateSizes( void )
{
    /*
    xbool bSuccess = GetDiskFreeSpaceEx( mDriveLetter,
                                         (PULARGE_INTEGER)&mAvailableBytes,
                                         (PULARGE_INTEGER)&mTotalBytes,
                                         NULL );

    if( bSuccess )
    {
        // You know how many bytes are there, and how many are available, so calculate how many are used up.
        mUsedBytes = mTotalBytes - mAvailableBytes;

        // Now determine the cluster size of the Memory Unit.
        mClusterSize = XGetDiskClusterSize( mDriveLetter );

        // Determine the sector size of the unit.
        mSectorSize = XGetDiskSectorSize( mDriveLetter );

        // Also identify how many blocks, are free. (If what I think is true, a block is the same size as the cluster.)
        mTotalBlocks = (u32)(mTotalBytes / PC_DRIVE_BLOCK_SIZE);
        mUsedBlocks  = (u32)(mUsedBytes  / PC_DRIVE_BLOCK_SIZE);
        mFreeBlocks  = mTotalBlocks - mUsedBlocks;
        return TRUE;
    }
    else
    {
        return FALSE;
        ASSERT( 0 ); // Can't ever really see why this would fail.
    }
    */
    return FALSE;
}


//=====================================================================================================================================
// PCMemoryUnitManager
//=====================================================================================================================================
PCMemoryUnitManager::PCMemoryUnitManager( void )
{
    MC_PRINT( "Constructing" );

    // Initialize some known data about the PC Hardsdisk
    mHardDisk.Initialize( QUAG_MEMORY_UNIT_ID_0 );
    mbHardDiskIsSelected = FALSE;

    // Initialize the command handling variables.
    mpCurrentCommandHandler                 = NULL;
    mCurrentCommand                         = PCMU_CMD_NONE;
    mbCommandCompleted                      = TRUE; 
    mbWaitingForCommandCompleteAcknowledged = FALSE;

    // Initialize the save game info to defaults until the Memory unit resource file is loaded.
    mNumberOfSaveGameTypes = 0;
    mpSaveGameTypes        = NULL;

    mpAvailableSavedGames  = NULL;
    mCurrentGameType       = 0;
    mCurrentGameTypeIndex  = 0;

    mIconData              = 0;
    mIconDataSize          = 0;

    mFileInfo.Reset( );

    // Initialize the rest of the data.
    SetBrowserText( "PC save game default text", 27 ); 
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool PCMemoryUnitManager::Initialize( char* MemoryUnitResourceFilename )
{
    xbool                 bSuccess;
    PCMUResourceHeader  Header;
    X_FILE*               pFile;
    byte*                 pTempBuffer;
    s32                   ReadCount;
    s32                   index;

    PCMUResourceSaveGameType* pResourceSaveGameTypes;
    byte*                       pRawIconData;

    pTempBuffer            = NULL;
    pResourceSaveGameTypes = NULL;
    pRawIconData           = NULL;
    bSuccess               = FALSE;

    //-------------------------------------------------------------------------------------------------------------------------------------
    MC_PRINT( "Initializing" );

    // Get all of the information required to save a game on this system by loading in the instructional resource
    // file.  This file will contain all information about how saved games must be built on this platform, as well
    // the data for an icon, and any information regarding how to draw that icon is also provided.
    pFile = x_fopen( MemoryUnitResourceFilename, "r" );

    if( pFile )
    {
        ReadCount = x_fread( &Header, sizeof( PCMUResourceHeader ), 1, pFile );

        if( ReadCount == 1 )
        {
            // Test to make sure that it's a PS2 resource file.
            if( Header.mResourceType == QUAG_MEMORY_UNIT_RESOURCE_TYPE_PC )
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
                    
                    mpSaveGameTypes = new PCMUResourceSaveGameType[mNumberOfSaveGameTypes];
                    ASSERT( mpSaveGameTypes );

                    // Read in data for the save game types.
                    pResourceSaveGameTypes = (PCMUResourceSaveGameType*)(pTempBuffer + Header.mOffsetToSaveTypes);
                    x_memcpy( mpSaveGameTypes, pResourceSaveGameTypes, sizeof( PCMUResourceSaveGameType ) * mNumberOfSaveGameTypes );

                    // Also initialize the Available saved games list
                    mpAvailableSavedGames = new SaveGameTypeAvailableList[mNumberOfSaveGameTypes];
                    ASSERT( mpAvailableSavedGames );


                    // Retrieve the icon information for this game from the resource file.
                    //-------------------------------------------------------------------------------------------------------------------------------------
                    pRawIconData = pTempBuffer + Header.mOffsetToIconData;
                    BuildIconFromResourceData( pRawIconData, Header.mSizeOfIconData );

                    // Allocate an PC save game list that should acoomodate all possible saves
                    // created by this title.
                    mMaxPCSavesForThisApp = 0;
                    for( index = 0; index < mNumberOfSaveGameTypes; index++ )
                    {
                        mMaxPCSavesForThisApp += mpSaveGameTypes[index].mMaxNumberOfSaves;
                    }

//                    mpPCSaveGameData = new XGAME_FIND_DATA[mMaxPCSavesForThisApp];
//                    ASSERT( mpPCSaveGameData );

                    ClearAvailableGamesList( );

                    bSuccess = TRUE;
                }
                else
                {
                    MC_PRINT( "Couldn't read entire resource file" );
                }

            }
            else
            {
                MC_PRINT( "Resource File is not for PC" );
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

    return bSuccess;
}

//-------------------------------------------------------------------------------------------------------------------------------------
// Destruction / Kill
//-------------------------------------------------------------------------------------------------------------------------------------
PCMemoryUnitManager::~PCMemoryUnitManager( void )
{
    MC_PRINT( "Destroying" );

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

    // Delete the PC save data info.
//    if( mpPCSaveGameData )
//    {
//        delete[] mpPCSaveGameData;
//        mpPCSaveGameData = NULL;
//    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
// Update Functions.
//-------------------------------------------------------------------------------------------------------------------------------------
void PCMemoryUnitManager::Update( void )
{
    u32     MemoryUnitInsertions = 0;
    u32     MemoryUnitRemovals = 0;

    // If there are no other commands in progress, test to see if memory units have been connected.
    // We only need to do this in order to pass a TCR about detecting card connects.  Normally
    // this wouldn't be a problem, but for time restraints, we are only implementing full blown saving and loading
    // of games to the hard disk.
    mbPCMemoryCardInsertionDetectedLastFrame = FALSE;

    if( mpCurrentCommandHandler )
    {
        // Call the current command handler.
        (this->*mpCurrentCommandHandler)();
    }
    else if( IsCommandInProgress( ) == FALSE )
    {
//        if( XGetDeviceChanges( XDEVICE_TYPE_MEMORY_UNIT, &MemoryUnitInsertions, &MemoryUnitRemovals ) )
//        {
            // Test to see if there were some memory card insertions.
//            if( MemoryUnitInsertions != 0 )
//                mbPCMemoryCardInsertionDetectedLastFrame = TRUE;
//        }
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
// Interface Functions.
//-------------------------------------------------------------------------------------------------------------------------------------
void PCMemoryUnitManager::SetBrowserText( char* pBrowserInfo, s32 BreakCharacter )
{
    // The Break Character doesn't mean anything on the PC.
    x_memset(  mBrowserTitleText, 0, MEMORY_UNIT_SAVE_GAME_TITLE_LENGTH);
    x_strncpy( mBrowserTitleText, pBrowserInfo, MEMORY_UNIT_SAVE_GAME_TITLE_LENGTH );

    // Make sure that the browser text strings are not too large.
    ASSERT( x_strlen( mBrowserTitleText) < MEMORY_UNIT_SAVE_GAME_TITLE_LENGTH );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void PCMemoryUnitManager::GetBrowserText( char* Buffer )
{
    x_strncpy( Buffer, mBrowserTitleText, MEMORY_UNIT_SAVE_GAME_TITLE_LENGTH );
}


//-------------------------------------------------------------------------------------------------------------------------------------
// Actual Memory Card Related Functions.
//-------------------------------------------------------------------------------------------------------------------------------------
char* PCMemoryUnitManager::GetNameOfHardDisk( void )
{
    // If the harddisk isn't selected, should they still get the name? Um, no for now, to make it more like the other consoles.
    if( mbHardDiskIsSelected )
    {
        return mHardDisk.mName;
    }
    else
    {
        return NULL;
    }

}

//-------------------------------------------------------------------------------------------------------------------------------------
s32 PCMemoryUnitManager::GetFreeBlocksRemainingOnHardDisk( void )
{
    return (s32)mHardDisk.mFreeBlocks;
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 PCMemoryUnitManager::GetFreeFilesRemainingOnHardDisk( void )
{
    return PC_MAX_SAVED_GAMES - mNumSaveGamesFound;
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 PCMemoryUnitManager::SelectHardDisk( byte* pFileIOBuffer, s32 BufferSize )
{
    // First Make sure that no command is active.
    if( IsCommandInProgress( ) == FALSE )
    {
        if( mbHardDiskIsSelected == FALSE )
        {
            s32 CommandRequestReturnValue;

            mbHardDiskIsSelected = TRUE;

            // Request for the next command.
            CommandRequestReturnValue = RequestNewCommand( PCMU_CMD_MOUNT );

            // If the command to mount the card failed, then make sure to releaes the active unit.
            if( CommandRequestReturnValue != QUAG_MU_EVERYTHING_IS_OK )
            {
                // You don't want to release the mounted card if all that's wrong is that it's not formatted
                // This card should still be valid, but the only command allowed should be to format it.
                if( CommandRequestReturnValue != QUAG_MU_NOT_FORMATTED )
                {
                    // I don't believe that this should ever happen.
                    ASSERT( 0 );
                    ReleaseHardDisk( );
                }
            }

            return CommandRequestReturnValue;
        }
        // If there is an active, make sure it's already the one requested, otherwise FAILURE.
        else
        {
            return QUAG_MU_TRYING_TO_ACTIVATE_MORE_THAN_ONE_CARD;
        }
    }
    else
    {
        return QUAG_MU_SYSTEM_BUSY;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 PCMemoryUnitManager::ReleaseHardDisk( void )
{
    if( mbHardDiskIsSelected )
    {
        mbHardDiskIsSelected = FALSE;

        // Clear the available game list.
        ClearAvailableGamesList( );

        // Release was good.
        return QUAG_MU_EVERYTHING_IS_OK;
    }
    else
    {
        MC_PRINT( "Trying to release an active card when there isn't one." );
        return QUAG_MU_NO_ACTIVE_CARD_SELECTED;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 PCMemoryUnitManager::CalculateRequiredSizeForSaveGameType( s32 SaveGameType, s32* pRequiredBlocks, xbool bWorstCase )
{
    const u32 IMAGE_META_HDR_SIZE  = 2048;          // 2K
    const u32 IMAGE_META_DATA_SIZE = (64 * 64) / 2; // DXT1 is 4 bits per pixel

    u32 UserDataSize;               // User data size, remember to include the signature value in the data sizes.
    u32 MetaDataSizeOverhead;       // Icon and icon text.
    u32 SaveGameSystemOverhead;     // Overhead just to save a game.
    u32 ClusterSize;                // Size of a cluster on the current memory unit.
    u32 TotalBlocksRequiredForSave; // Total number of blocks required to make the save.
    u32 TotalBytes;

    // If the harddisk isn't activated, it's as if they didn't mount a memory card, so bail now.
    if( bWorstCase == FALSE )
    {
        if( mbHardDiskIsSelected == FALSE )
        {
            *pRequiredBlocks = 0;
            return QUAG_MU_NO_ACTIVE_CARD_SELECTED;
        }
    }

    //=====================================================================================================================================
    // How to calculate the size of a saved file on the PC.
    //=====================================================================================================================================
    //
    // The PC only stores save games in terms of Blocks that are 16384 bytes in size.
    //
    // The space needed to create a file can be calculated as follows.
    //
    // A. The number of clusters used by the UserDataFile
    //-------------------------------------------------------------------------------------------------------------------------------------
    // Basically, take the bytes count and round it up to the nearest cluster size.
    // User Data Clusters = (number of bytes + (CluserSize - 1) ) / ClusterSize
    //
    // B - The overhead required for the icon and browser text.
    //-------------------------------------------------------------------------------------------------------------------------------------
    // Again, take the total byte sizes and round it up to the nearest cluster size.
    // MetaData Clusters = (ByteSize of Icon and Text meta data + (Cluster Size - 1)) / ClusterSize
    //
    // C - Number of clusters wasted by the save itself
    //-------------------------------------------------------------------------------------------------------------------------------------
    // 2 clusters are used for the directory entry when a new directory is created.
    //
    //-------------------------------------------------------------------------------------------------------------------------------------
    // 
    // The space needed to create files is determined by 
    // Add A + B and C togetherr, then bring it back to a byte count.  Once you have a byte count, round that value up 
    // to the nearest block count.
    ClusterSize          = mHardDisk.mClusterSize;
    UserDataSize         = (mpSaveGameTypes[SaveGameType].mUserDataFileInfo.mSizeOfUserDataFile + (ClusterSize - 1)) / ClusterSize;
    MetaDataSizeOverhead = (IMAGE_META_HDR_SIZE + IMAGE_META_DATA_SIZE + (ClusterSize - 1)) / ClusterSize;
    SaveGameSystemOverhead = 2;

    TotalBytes = (UserDataSize + MetaDataSizeOverhead + SaveGameSystemOverhead) * ClusterSize;

    TotalBlocksRequiredForSave = (TotalBytes + PC_DRIVE_BLOCK_SIZE - 1) / PC_DRIVE_BLOCK_SIZE;

    // Provide this information to calling function.
    *pRequiredBlocks = TotalBlocksRequiredForSave;

    if( bWorstCase == FALSE )
    {
        // Now return if this is enough or not.
        if( TotalBlocksRequiredForSave <= (s32)mHardDisk.mFreeBlocks )
        {
            return QUAG_MU_EVERYTHING_IS_OK;
        }
    }

    return QUAG_MU_NOT_ENOUGH_FREE_SPACE;
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 PCMemoryUnitManager::GetUserDataByteSize( s32 SaveGameType )
{
    if( mNumberOfSaveGameTypes == 0 ) return 0;
    ASSERT( SaveGameType >= 0 && SaveGameType < mNumberOfSaveGameTypes );
    return mpSaveGameTypes[SaveGameType].mUserDataFileInfo.mSizeOfUserDataFile;
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 PCMemoryUnitManager::Format( void )
{
    // You should never call this on the PC right now since it's the hard drive that you are working with.
    ASSERT( 0 );
    return QUAG_MU_EVERYTHING_IS_OK;
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 PCMemoryUnitManager::SaveGame( s32 SaveGameType, s32 TypeOffset, void* pUserData, char* SaveGameBrowserInfo, s32 BrowserInfoBreakCharacter )
{
    s32 CommandRequestReturnValue;

    CommandRequestReturnValue = RequestNewCommand( PCMU_CMD_SAVE );

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
        mFileInfo.mpUserBuffer = (byte*)pUserData;

        // Set up the browser text.
        SetBrowserText( SaveGameBrowserInfo, BrowserInfoBreakCharacter );

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
s32 PCMemoryUnitManager::LoadGame( s32 SaveGameType, s32 TypeOffset, void* pUserData )
{
    s32 CommandRequestReturnValue;

    CommandRequestReturnValue = RequestNewCommand( PCMU_CMD_LOAD );

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
        mFileInfo.mpUserBuffer = (byte*)pUserData;

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
s32 PCMemoryUnitManager::DeleteGame( s32 SaveGameType, s32 TypeOffset )
{
    s32 CommandRequestReturnValue;

    CommandRequestReturnValue = RequestNewCommand( PCMU_CMD_DELETE );

    if( CommandRequestReturnValue == QUAG_MU_EVERYTHING_IS_OK )
    {
        // Determine if the save game type is valid.
        ASSERT( SaveGameType >= 0 && SaveGameType < mNumberOfSaveGameTypes );
        mCurrentGameType = SaveGameType;

        // Determine if the offset into this Save Game Type is valid or not.
        ASSERT( TypeOffset >= 0 && TypeOffset <= mpSaveGameTypes[SaveGameType].mMaxNumberOfSaves );
        mCurrentGameTypeIndex = TypeOffset;

        // Well it's time that we start.
        return QUAG_MU_EVERYTHING_IS_OK;
    }
    else
    {
        return CommandRequestReturnValue;
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
s32 PCMemoryUnitManager::GetSaveGameList( s32 SaveGameType, s32* pMaxSaveGameTypeCount, SingleSavedGameInfo** pSaveGameListings )
{
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
xbool PCMemoryUnitManager::IsCommandComplete( void )
{
    return ( mbCommandCompleted &&                               // If the command is complete 
             mbWaitingForCommandCompleteAcknowledged == TRUE &&  // and the
             (mCurrentCommand != PCMU_CMD_NONE)
           );
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool PCMemoryUnitManager::GetLastCommandValues( s32* pReturnValue, s32* pErrorCode )
{
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
        mCurrentCommand                         = PCMU_CMD_NONE;

        // The command was completed as complete by the user call.
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool PCMemoryUnitManager::IsWriteInProgress( void )
{
    return mCurrentCommand == PCMU_CMD_SAVE;
}


//-------------------------------------------------------------------------------------------------------------------------------------
void PCMemoryUnitManager::CompleteCommandAndWaitForUser( s32 ReturnValue, s32 ErrorCode )
{
    mbCommandCompleted                      = TRUE;
    mbWaitingForCommandCompleteAcknowledged = TRUE;
    mCommandReturnValue                     = ReturnValue;
    mCommandError                           = ErrorCode;
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 PCMemoryUnitManager::RequestNewCommand( s32 NewCommand )
{
    // Make sure the command is valid.
    ASSERT( NewCommand >= PCMU_CMD_FIRST && NewCommand <= PCMU_CMD_LAST );

    // Make sure that an active card is already selected.
    if( mbHardDiskIsSelected )
    {
        // Is there a command in progress?
        if( IsCommandInProgress( ) == FALSE )
        {
            mCurrentCommand                         = NewCommand;
            mCommandState                           = 0;
            mCommandSubState                        = 0;
            mbCommandCompleted                      = FALSE;
            mbWaitingForCommandCompleteAcknowledged = FALSE;
//            mpCurrentPCSaveGameData               = NULL;

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
        MC_PRINT( "Requests %s, but the Hard Disk wasn't activated.  Request FAILED", GetCommandNameAsString( NewCommand ) );
        return QUAG_MU_NO_ACTIVE_CARD_SELECTED;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool PCMemoryUnitManager::IsCommandInProgress( void )
{
    return ( mbCommandCompleted &&                               // If the command is complete 
             mbWaitingForCommandCompleteAcknowledged == TRUE &&  // and the
             (mCurrentCommand != PCMU_CMD_NONE)
           );
}


//-------------------------------------------------------------------------------------------------------------------------------------
void PCMemoryUnitManager::SetRequestedCommandHandler( s32 NewCommand )
{
    // reset this timer for each command.
    mCommandStartTime = x_GetTime( );

    switch( NewCommand )
    {
        case PCMU_CMD_MOUNT:   mpCurrentCommandHandler = &PCMemoryUnitManager::MountHandler;      break;
        case PCMU_CMD_FORMAT:  mpCurrentCommandHandler = &PCMemoryUnitManager::FormatHandler;     break;
        case PCMU_CMD_SAVE:    mpCurrentCommandHandler = &PCMemoryUnitManager::SaveGameHandler;   break;
        case PCMU_CMD_LOAD:    mpCurrentCommandHandler = &PCMemoryUnitManager::LoadGameHandler;   break;
        case PCMU_CMD_DELETE:  mpCurrentCommandHandler = &PCMemoryUnitManager::DeleteGameHandler; break;

        default:
            ASSERT( 0 );
            mpCurrentCommandHandler = NULL;
            break;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
// Command handling functions.
//-------------------------------------------------------------------------------------------------------------------------------------
void PCMemoryUnitManager::UpdateFreeBlocksAndFiles( void )
{
    if( mbHardDiskIsSelected )
    {
        mHardDisk.UpdateSizes( );
    }
}
/*
//-------------------------------------------------------------------------------------------------------------------------------------
void PCMemoryUnitManager::SetAndFillCurrentPCSaveGameData( s32 SaveGameType, s32 SaveGameIndex, XGAME_FIND_DATA* pPCFindGameData )
{
    s32 index;
    s32 GameType;

    index = 0;

    // Offset by complete save game types.
    for( GameType = 0; GameType < SaveGameType; GameType++ )
        index += mpSaveGameTypes[GameType].mMaxNumberOfSaves;

    // Now offset within this type.
    index += SaveGameIndex;

    // Determine the offset into the mpPCSaveGameData array where this PC save game info should be.
    x_memcpy( &mpPCSaveGameData[index], pPCFindGameData, sizeof( XGAME_FIND_DATA ) );
}


//-------------------------------------------------------------------------------------------------------------------------------------
XGAME_FIND_DATA* PCMemoryUnitManager::GetCurrentPCSaveGameData( s32 SaveGameType, s32 SaveGameIndex )
{
    s32 index;
    s32 GameType;

    index = 0;

    // Offset by complete save game types.
    for( GameType = 0; GameType < SaveGameType; GameType++ )
        index += mpSaveGameTypes[GameType].mMaxNumberOfSaves;

    // Now offset within this type.
    index += SaveGameIndex;

    return &mpPCSaveGameData[index];
}
*/

//-------------------------------------------------------------------------------------------------------------------------------------
void PCMemoryUnitManager::MountHandler( void )
{
    enum MountStates
    {
        MOUNT_STATE_START,
        MOUNT_STATE_GET_CARD_SAVED_GAMES,
        MOUNT_STATE_COMPLETE,
    };

    switch( mCommandState )
    {
        case MOUNT_STATE_START:
        {
            // The hard disk is activated, time to get some shit going. 
            // Determine the sizes of the drive.
            UpdateFreeBlocksAndFiles( );

            mCommandState = MOUNT_STATE_GET_CARD_SAVED_GAMES;
            mCommandSubState = 0;
            break;
        }

        case MOUNT_STATE_GET_CARD_SAVED_GAMES:
        {
            if( MountHandlerGetSavedGames(  ) )
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
xbool PCMemoryUnitManager::MountHandlerGetSavedGames( void )
{
    enum ScanningForSavedGames
    {
        GET_SAVED_GAME_FILES_START,
        GET_SAVED_GAME_LIST_FROM_PC,
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
            mCommandSubState = GET_SAVED_GAME_LIST_FROM_PC;

//-------------------------------------------------------------------------------------------------------------------------------------
/*
            s32                 SaveGameType;
            s32                 SaveGameIndex;
            s32                 ErrorCode;
            char                ThinName[MEMORY_UNIT_SAVE_GAME_TITLE_LENGTH];
            XGAME_FIND_DATA     PCGameFindData;

            for( SaveGameType = 0; SaveGameType < mNumberOfSaveGameTypes; SaveGameType++ )
            {
                for( SaveGameIndex = 0; SaveGameIndex < mpSaveGameTypes[SaveGameType].mMaxNumberOfSaves; SaveGameIndex++ )
                {
                    GetUserDataFileName( SaveGameType, SaveGameIndex, ThinName );
                    wsprintfW( mWideNameBuffer, L"%hs", ThinName );
            
                    // Test
                    ErrorCode = XCreateSaveGame( mHardDisk.mDriveLetter, // Root Drive,
                                                 mWideNameBuffer, 
                                                 CREATE_NEW,
                                                 0,
                                                 PCGameFindData.szSaveGameDirectory,
                                                 MAX_PATH );
                }
            }
*/
//-------------------------------------------------------------------------------------------------------------------------------------
            break;
        }

        case GET_SAVED_GAME_LIST_FROM_PC:
        { 
/*
            HANDLE               hSaveGame;
            XGAME_FIND_DATA      PCGameFindData;
            xbool                bLookForSaves;
            xbool                bFirst;
//            s32                  SaveGameCount;
            s32                  LastError;

            // Clear the available list that are in the quagmire format.
            ClearAvailableGamesList( );

            // Now drudge through the PC save game method to find any saved games for this app.
            bLookForSaves = TRUE;
            bFirst = TRUE;

            while( bLookForSaves )
            {
                if( bFirst )
                {
                    bFirst = FALSE;
                    hSaveGame = XFindFirstSaveGame( mHardDisk.mDriveLetter, &PCGameFindData );
                    bLookForSaves = hSaveGame != INVALID_HANDLE_VALUE;
                }
                else
                {
                    bLookForSaves = XFindNextSaveGame( hSaveGame, &PCGameFindData );
                }

                // If there was a save game for this application found, then process it.
                if( bLookForSaves )
                {
                    // Increment the number of save games found on the card.
                    mNumSaveGamesFound++;

                    // Test to see if all the expected saves have been found, if they have been, just ignore any other
                    // data that still in the user folder.
                    if( MatchUpPCSaveToExpectedSaveGameType( &PCGameFindData ) )
                    {
                        ASSERT( mNumExpectedGamesFound < mMaxPCSavesForThisApp );
                        mNumExpectedGamesFound++;
                    }
                }
                else
                {
                    LastError = GetLastError( );
                    ASSERT( LastError == ERROR_NO_MORE_FILES );
                }
            }

            // Make sure to clear the find list handler when done.
            if( hSaveGame != INVALID_HANDLE_VALUE )
                XFindClose( hSaveGame );

            // The mount is complete.
            //-------------------------------------------------------------------------------------------------------------------------------------
            return TRUE;
*/
            break;
        }

        default:
            ASSERT( 0 );
            break;

    } // switch( mCommandSubState )

    return FALSE;
}

//-------------------------------------------------------------------------------------------------------------------------------------
void PCMemoryUnitManager::FormatHandler( void )
{
    // This should never be called unless the actual memory units are implemented, you can't format the harddisk.
    ASSERT( 0 );
}


//-------------------------------------------------------------------------------------------------------------------------------------
void PCMemoryUnitManager::SaveGameHandler( void )
{
    enum SaveGameStates
    {
        SAVE_GAME_STATE_START,
        SAVE_GAME_STATE_CREATE_ICON_DATA_METAFILE,
        SAVE_GAME_STATE_CREATE_USER_DATA_FILE,
        SAVE_GAME_STATE_DONE_SUCCESS,
    };

    switch( mCommandState )
    {
        case SAVE_GAME_STATE_START:
        {
/*
            s32 ErrorCode;
            char UserDataFilename[PC_USER_DATA_FILENAME_LENGTH];

            // Get the PC save game data structure for it's associated game type and index.
//            mpCurrentPCSaveGameData = GetCurrentPCSaveGameData( mCurrentGameType, mCurrentGameTypeIndex );

            // Get the name of the save for this SaveGameType and Index.
            GetUserDataFileName( mCurrentGameType, mCurrentGameTypeIndex, UserDataFilename );

            // Extend the save name into the wide character form.
//            x_sprintf( mWideNameBuffer, L"%hs", UserDataFilename );

            ErrorCode = XCreateSaveGame( mHardDisk.mDriveLetter, // Root Drive,
                                         mWideNameBuffer, 
                                         CREATE_NEW,
                                         0,
                                         mpCurrentPCSaveGameData->szSaveGameDirectory,
                                         MAX_PATH );

            if( ErrorCode != ERROR_SUCCESS )
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_TOO_MANY_FILES_ON_CARD );
                return;
            }
            // The creation of the save game was a success.
            else
            {
                mCommandState = SAVE_GAME_STATE_CREATE_ICON_DATA_METAFILE;
            }
*/
  
            break;
        }

        case SAVE_GAME_STATE_CREATE_ICON_DATA_METAFILE:
        {
/*
            x_strcpy( mFileInfo.mCurrentFilename, mpCurrentPCSaveGameData->szSaveGameDirectory );
            x_strcat( mFileInfo.mCurrentFilename, PC_SAVE_GAME_LOGO_IMAGE_FILE_NAME );

            mFileInfo.mCurrentFile = x_fopen( mFileInfo.mCurrentFilename, "w" );
            if( mFileInfo.mCurrentFile )
            {
                if( x_fwrite( mIconData, mIconDataSize, 1, mFileInfo.mCurrentFile ) != 1 )
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_SAVE_FAILED );
                }
                x_fclose( mFileInfo.mCurrentFile );            

                mCommandState = SAVE_GAME_STATE_CREATE_USER_DATA_FILE;
                mCommandSubState = 0;
            }
            else
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_SAVE_FAILED );
            }


            mFileInfo.Reset( );
*/
            break;
        }

        case SAVE_GAME_STATE_CREATE_USER_DATA_FILE:
        {
            if( SaveGameHandlerUserDataFile( ) )
            {
                mCommandState = SAVE_GAME_STATE_DONE_SUCCESS;
            }

            break;
        }

        case SAVE_GAME_STATE_DONE_SUCCESS:
        {
            f64 ElapsedTime;
            ElapsedTime = x_GetElapsedTime( mCommandStartTime );

            // On the PC, a save must take at least 3 seconds.  So if the time has not elapsed, don't end the process.
            if( ElapsedTime > PC_MIN_SAVED_GAME_TIME )
            {
                CompleteCommandAndWaitForUser( TRUE, QUAG_MU_EVERYTHING_IS_OK );
            }
            break;
        }


        default:
        {
            ASSERT( 0 );
            break;
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
xbool PCMemoryUnitManager::SaveGameHandlerUserDataFile( void )
{
    enum SaveUserDataFileSubStates
    {
        USER_DATA_SAVE_STATE_START,
        USER_DATA_SAVE_STATE_WRITE_CHUNK,
        USER_DATA_SAVE_STATE_COMPLETE,
    };


    switch( mCommandSubState )
    {
        case USER_DATA_SAVE_STATE_START:
        {
/*
            x_strcpy( mFileInfo.mCurrentFilename, mpCurrentPCSaveGameData->szSaveGameDirectory );
            x_strcat( mFileInfo.mCurrentFilename, PC_SAVE_GAME_FILENAME );

            mFileInfo.mCurrentFile = x_fopen( mFileInfo.mCurrentFilename, "w" );
            if( mFileInfo.mCurrentFile )
            {
                mFileInfo.mBytesTransferred   = 0;
                mFileInfo.mBytesToTransfer = mpSaveGameTypes[mCurrentGameType].mUserDataFileInfo.mSizeOfUserDataFile;

                // Well, the file is open and ready for business.
                mCommandSubState = USER_DATA_SAVE_STATE_WRITE_CHUNK;
            }
            else
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_SAVE_FAILED );
            }
*/
            break;
        }

        case USER_DATA_SAVE_STATE_WRITE_CHUNK:
        {
            s32 WriteSize;

            // Write one chunk or the remainder of the file, which ever is smaller.
            WriteSize = MIN( mFileInfo.mBytesToTransfer, PC_READ_WRITE_CHUNK_SIZE );

            if( x_fwrite( mFileInfo.mpUserBuffer + mFileInfo.mBytesTransferred, WriteSize, 1, mFileInfo.mCurrentFile ) == 1 )
            {
                mFileInfo.mBytesTransferred   += WriteSize;
                mFileInfo.mBytesToTransfer    -= WriteSize;

                if( mFileInfo.mBytesToTransfer == 0 )
                {
                    mCommandSubState = USER_DATA_SAVE_STATE_COMPLETE;
                }
            }
            else
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_SAVE_FAILED );
                x_fclose( mFileInfo.mCurrentFile );
            }

            break;
        }

        case USER_DATA_SAVE_STATE_COMPLETE:
        {
            if( mFileInfo.mCurrentFile )
            {
                x_fclose( mFileInfo.mCurrentFile );
            }
            return TRUE;
            break;
        }

        default:
            ASSERT( 0 );
            break;
    }

    return FALSE;
}


//-------------------------------------------------------------------------------------------------------------------------------------
void PCMemoryUnitManager::LoadGameHandler( void )
{
    enum LoadStates
    {
        LOAD_GAME_STATE_START,
        LOAD_GAME_STATE_LOAD_USER_DATA_FILE,
        LOAD_GAME_STATE_COMPLETE,
    };


    switch( mCommandState )
    {
        case LOAD_GAME_STATE_START:
        {
            // Just make sure that the game that they wish to delete is valid.
            ASSERT( mpAvailableSavedGames[mCurrentGameType].mSavedGames[mCurrentGameTypeIndex].mbValid );

            // Grab the equivalent PC save game data on for this SaveGameType and Index.
//            mpCurrentPCSaveGameData = GetCurrentPCSaveGameData( mCurrentGameType, mCurrentGameTypeIndex );

            // Now ready to read that file.
            mCommandSubState = 0;
            mCommandState = LOAD_GAME_STATE_LOAD_USER_DATA_FILE;

            break;
        }

        case LOAD_GAME_STATE_LOAD_USER_DATA_FILE:
        {
            if( LoadGameHandlerUserDataFile() )
            {
                mCommandState = LOAD_GAME_STATE_COMPLETE;
            }
            
            break;
        }

        case LOAD_GAME_STATE_COMPLETE:
        {
            f64 ElapsedTime;
            ElapsedTime = x_GetElapsedTime( mCommandStartTime );

            if( ElapsedTime > PC_MIN_LOAD_GAME_TIME )
            {
                CompleteCommandAndWaitForUser( TRUE, QUAG_MU_EVERYTHING_IS_OK );
            }
            break;

            break;
        }

        default:
            ASSERT( 0 );
            break;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool PCMemoryUnitManager::LoadGameHandlerUserDataFile( void )
{
    enum LoadUserDataFileSubStates
    {
        USER_DATA_LOAD_STATE_START,
        USER_DATA_LOAD_STATE_READ_CHUNK,
        USER_DATA_LOAD_STATE_COMPLETE,
    };


    switch( mCommandSubState )
    {
        case USER_DATA_LOAD_STATE_START:
        {
/*
            x_strcpy( mFileInfo.mCurrentFilename, mpCurrentPCSaveGameData->szSaveGameDirectory );
            x_strcat( mFileInfo.mCurrentFilename, PC_SAVE_GAME_FILENAME );

            mFileInfo.mCurrentFile = x_fopen( mFileInfo.mCurrentFilename, "r" );
            if( mFileInfo.mCurrentFile )
            {
                mFileInfo.mBytesTransferred   = 0;
                mFileInfo.mBytesToTransfer = mpSaveGameTypes[mCurrentGameType].mUserDataFileInfo.mSizeOfUserDataFile;

                // Test to make sure that the length of the file is not larger or smaller than expected.
                // This is a new TRC validation test for Microsoft. Who knew...
                s32 Filelength;

                x_fseek( mFileInfo.mCurrentFile, 0, X_SEEK_END );
                Filelength = x_ftell( mFileInfo.mCurrentFile );
                x_fseek( mFileInfo.mCurrentFile, 0, X_SEEK_SET );
                Filelength -= x_ftell( mFileInfo.mCurrentFile );

                if( Filelength != mFileInfo.mBytesToTransfer )
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_LOAD_FAILED );
                    x_fclose( mFileInfo.mCurrentFile );
                }
                else
                {
                    // Well, the file is open and ready for business.
                    mCommandSubState = USER_DATA_LOAD_STATE_READ_CHUNK;
                }
            }
            else
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_LOAD_FAILED );
            }
*/
            break;
        }

        case USER_DATA_LOAD_STATE_READ_CHUNK:
        {
            s32 ReadSize;

            // Write one chunk or the remainder of the file, which ever is smaller.
            ReadSize = MIN( mFileInfo.mBytesToTransfer, PC_READ_WRITE_CHUNK_SIZE );

            if( x_fread( mFileInfo.mpUserBuffer + mFileInfo.mBytesTransferred, ReadSize, 1, mFileInfo.mCurrentFile ) == 1 )
            {
                mFileInfo.mBytesTransferred += ReadSize;
                mFileInfo.mBytesToTransfer -= ReadSize;

                if( mFileInfo.mBytesToTransfer == 0 )
                {
                    mCommandSubState = USER_DATA_LOAD_STATE_COMPLETE;
                }
            }
            else
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_LOAD_FAILED );
                x_fclose( mFileInfo.mCurrentFile );
            }

            break;
        }

        case USER_DATA_LOAD_STATE_COMPLETE:
        {
            if( mFileInfo.mCurrentFile )
            {
                x_fclose( mFileInfo.mCurrentFile );
            }
            return TRUE;
            break;
        }

        default:
            ASSERT( 0 );
            break;
    }

    return FALSE;
}


//-------------------------------------------------------------------------------------------------------------------------------------
void PCMemoryUnitManager::DeleteGameHandler( void )
{
    enum DeleteStates
    {
        DELETE_GAME_STATE_START,
        DELETE_GAME_STATE_REMOVED_SAVED_GAME,
        DELETE_GAME_STATE_COMPLETE,
    };

    switch( mCommandState )
    {
        case DELETE_GAME_STATE_START:
        {
            // Just make sure that the game that they wish to delete is valid.
            ASSERT( mpAvailableSavedGames[mCurrentGameType].mSavedGames[mCurrentGameTypeIndex].mbValid );

            // Grab the equivalent PC save game data on for this SaveGameType and Index.
//            mpCurrentPCSaveGameData = GetCurrentPCSaveGameData( mCurrentGameType, mCurrentGameTypeIndex );
            mCommandState = DELETE_GAME_STATE_REMOVED_SAVED_GAME;
            break;
        }

        case DELETE_GAME_STATE_REMOVED_SAVED_GAME:
        {
/*
            s32 ErrorCode;

            ErrorCode = XDeleteSaveGame( mHardDisk.mDriveLetter, mpCurrentPCSaveGameData->szSaveGameName );
            if( ErrorCode == ERROR_SUCCESS )
            {
                // Clear out the PC save game info for this save, now that it's gone.
                x_memset( mpCurrentPCSaveGameData, 0, sizeof( XGAME_FIND_DATA ) );

                // Clear out the game from the available list as well.
                mpAvailableSavedGames[mCurrentGameType].Clear( mCurrentGameTypeIndex );
                mCommandState = DELETE_GAME_STATE_COMPLETE;
            }
            else
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_DELETE_FAILED );
            }
*/
            break;
        }

        case DELETE_GAME_STATE_COMPLETE:
        {
            f64 ElapsedTime;
            ElapsedTime = x_GetElapsedTime( mCommandStartTime );

            if( ElapsedTime > PC_MIN_DELETE_TIME )
            {
                UpdateFreeBlocksAndFiles( );
                CompleteCommandAndWaitForUser( TRUE, QUAG_MU_EVERYTHING_IS_OK );
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
void PCMemoryUnitManager::ClearAvailableGamesList( void )
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

    // Clear out the PC stored data as well.
//    x_memset( mpPCSaveGameData, 0, mMaxPCSavesForThisApp * sizeof( XGAME_FIND_DATA ) );


    // Clear the save games found count.
    mNumExpectedGamesFound = 0;
    mNumSaveGamesFound     = 0;
}


//-------------------------------------------------------------------------------------------------------------------------------------
void PCMemoryUnitManager::GetUserDataFileName( s32 SaveGameType, s32 TypeIndex, char* Buffer )
{
    ASSERT( SaveGameType < mNumberOfSaveGameTypes );
    ASSERT( TypeIndex < mpSaveGameTypes[SaveGameType].mMaxNumberOfSaves );
    char Extension[4];


    // Fill the buffer up with the name of the saved game.
    x_sprintf( Buffer, "%s", mpSaveGameTypes[SaveGameType].mUserDataFileInfo.mUserDataSaveFileName );

    // If there are multiple files available of this type, then append the index on the end of it.
    if( mpSaveGameTypes[SaveGameType].mMaxNumberOfSaves > 1 )
    {
        x_sprintf( Extension, " %ld", TypeIndex + 1);
        x_strcat( Buffer, Extension );
    }

    ASSERT( x_strlen( Buffer ) < PC_USER_DATA_FILENAME_LENGTH );
}


//-------------------------------------------------------------------------------------------------------------------------------------
char* PCMemoryUnitManager::GetCommandNameAsString( s32 CommandID )
{
    static char* CommandNames[] =
    {
        "PCMU_CMD_NONE",
        "PCMU_CMD_MOUNT",
        "PCMU_CMD_FORMAT",
        "PCMU_CMD_SAVE",
        "PCMU_CMD_LOAD",
        "PCMU_CMD_DELETE",
        "UNKNOWN_CMD",
    };

    if( CommandID >= PCMU_CMD_FIRST && CommandID <= PCMU_CMD_LAST )
    {
        return CommandNames[CommandID];
    }
    else
    {
        return CommandNames[PCMU_CMD_LAST + 1];
    }
}

/*
//-------------------------------------------------------------------------------------------------------------------------------------
xbool PCMemoryUnitManager::MatchUpPCSaveToExpectedSaveGameType( XGAME_FIND_DATA* pPCSave )
{
    xbool                bNamesMatch;
    char                 PCSaveGameName[MAX_GAMENAME];
    char                 ExpectedSaveGameName[MEMORY_UNIT_SAVE_GAME_TITLE_LENGTH];
    s32                  GameType;
    s32                  GameIndex;
    SingleSavedGameInfo* pSavedGame;

    // Get the name of the save from the PC save type.
    wsprintf( PCSaveGameName, "%ls", pPCSave->szSaveGameName );

    // Go through each of the possible saved games to see if this file matches up to one of them.
    for( GameType = 0; GameType < mNumberOfSaveGameTypes; GameType++ )
    {
        for( GameIndex = 0; GameIndex < mpSaveGameTypes[GameType].mMaxNumberOfSaves; GameIndex++ )
        {
            pSavedGame = &mpAvailableSavedGames[GameType].mSavedGames[GameIndex];

            if( pSavedGame->mbValid == FALSE )
            {
                // Get the expected name for this type of save game.
                GetUserDataFileName( GameType, GameIndex, ExpectedSaveGameName );

                bNamesMatch = x_strcmp( ExpectedSaveGameName, PCSaveGameName ) == 0;

                if( bNamesMatch )
                {
                    pSavedGame->mbValid = TRUE;
                    pSavedGame->mSaveGameType = GameType;
                    pSavedGame->mSaveGameTypeIndex = GameIndex;
                    x_strncpy( pSavedGame->mSaveGameTitle, ExpectedSaveGameName, MEMORY_UNIT_SAVE_GAME_TITLE_LENGTH );

                    SetAndFillCurrentPCSaveGameData( GameType, GameIndex, pPCSave );
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}
*/
//-------------------------------------------------------------------------------------------------------------------------------------
// Icon Functions
//-------------------------------------------------------------------------------------------------------------------------------------
void PCMemoryUnitManager::BuildIconFromResourceData( byte* RawIconData, s32 Size )
{
    if( mIconData )
    {
        delete[] mIconData;
        mIconData = NULL;

        mIconDataSize = 0;
    }

    mIconData = new byte[Size];
    ASSERT( mIconData );
    mIconDataSize = Size;

    // Well at this point, it's time to copy the data from the resource buffer to the local one.
    x_memcpy( mIconData, RawIconData, Size );
}
