#ifndef PC_MEMORY_UNIT_HPP
#define PC_MEMORY_UNIT_HPP

//=====================================================================================================================================
// Required Includes
//=====================================================================================================================================
#include "x_files.hpp"
#include "Q_MemoryUnit.hpp"
#include "Q_PC.hpp"


//=====================================================================================================================================
// Forward Declarations
//=====================================================================================================================================


//=====================================================================================================================================
// Definitions
//=====================================================================================================================================
#define MEMORY_UNIT_PERSONALIZED_NAME_SIZE        32
#define PCMU_MAX_PATH_LENGTH                      (260) // Found in MAX_PATH in the Windef.h file.
#define PC_HARD_DISK_NAME                         "PC hard disk"

/*
    QUAG_MEMORY_UNIT_ID_0 --> PC_USER_HARD_DISK_UNIT
*/

#define MEMORY_UNIT_PERSONALIZED_NAME_SIZE    32


//=====================================================================================================================================
// PCMemoryUnit
//=====================================================================================================================================
struct PCMemoryUnit
{
    //-------------------------------------------------------------------------------------------------------------------------------------
    PCMemoryUnit( void );
    void    Initialize( s32 UnitID );
    void    Reset( void );
    void    SetName( char* NewPersonalizedName );
    void    SetDriveLetter( char c );
    xbool   UpdateSizes( void );

    //-------------------------------------------------------------------------------------------------------------------------------------
    s32     mUnitID;                                    // Location of the memory unit in the system.
    char    mName[MEMORY_UNIT_PERSONALIZED_NAME_SIZE];  // Name for this unit based on location in the system.
    char    mDriveLetter[4];                            // Drive letter assigned to this memory unit.
    

    xbool   mbConnected;    // Is the unit detected as inserted to the console.
    xbool   mbFormatted;    // Is the unit formatted?
    xbool   mbMounted;      // Is the unit mounted.

    u64     mTotalBytes;    // Total capacity of the memory unit in bytes.
    u64     mAvailableBytes;// The number of available bytes on the memory unit.
    u64     mUsedBytes;     // The number of bytes used up.
    u32     mClusterSize;   // A single unit of space on the memory unit.  Sections of the unit are allocated
                            // in multiples of this size.
    u32     mSectorSize;    // Size of one sector on the memory unit.
    u32     mTotalBlocks;
    u32     mFreeBlocks;
    u32     mUsedBlocks;
};


//=====================================================================================================================================
// PCFileInfo
//=====================================================================================================================================
struct PCFileInfo
{
    void Reset( void )
    {
        mCurrentFile = NULL;
        mCurrentFilename[0] = NULL;
        mBytesToTransfer = 0;
        mBytesTransferred = 0;
    }

    X_FILE* mCurrentFile;
    char    mCurrentFilename[MAX_PATH];
    byte*   mpUserBuffer;

    s32     mBytesToTransfer;
    s32     mBytesTransferred;


};


//=====================================================================================================================================
// PCMemoryUnitManager
//
// Do to lack of time, PCMemoryUnitManager is now only going to allow you to save to the Hard Disk.
//=====================================================================================================================================
class PCMemoryUnitManager
{
    public:
        //-------------------------------------------------------------------------------------------------------------------------------------
        // Construction / Initialization
        //-------------------------------------------------------------------------------------------------------------------------------------
        PCMemoryUnitManager( void );
        xbool   Initialize( char* MemoryUnitResourceFilename );

        //-------------------------------------------------------------------------------------------------------------------------------------
        // Destruction / Kill
        //-------------------------------------------------------------------------------------------------------------------------------------
        ~PCMemoryUnitManager( void );

        //-------------------------------------------------------------------------------------------------------------------------------------
        // Update Functions.
        //-------------------------------------------------------------------------------------------------------------------------------------
        void    Update( void );
        xbool   WasAMemoryCardConnectedLastFrame( void ) { return mbPCMemoryCardInsertionDetectedLastFrame; }

        //-------------------------------------------------------------------------------------------------------------------------------------
        // Interface Functions.
        //-------------------------------------------------------------------------------------------------------------------------------------
        void    SetBrowserText( char* pBrowserInfo, s32 BreakCharacter );
        void    GetBrowserText( char* Buffer );

        //-------------------------------------------------------------------------------------------------------------------------------------
        // Actual Memory Card Related Functions.
        //-------------------------------------------------------------------------------------------------------------------------------------
        char*   GetNameOfHardDisk( void );
        s32     GetFreeBlocksRemainingOnHardDisk( void );
        s32     GetFreeFilesRemainingOnHardDisk( void );

        s32     SelectHardDisk( byte* pFileIOBuffer, s32 BufferSize );
        s32     ReleaseHardDisk( void );

        s32     CalculateRequiredSizeForSaveGameType( s32 SaveGameType, s32* pRequiredBlocks, xbool bWorstCase );
        s32     GetUserDataByteSize( s32 SaveGameType );

        s32     Format( void );
        s32     SaveGame( s32 SaveGameType, s32 TypeOffset, void* pUserData, char* SaveGameBrowserInfo, s32 BrowserInfoBreakCharacter );
        s32     LoadGame( s32 SaveGameType, s32 TypeOffset, void* pUserData );
        s32     DeleteGame( s32 SaveGameType, s32 TypeOffset );

        s32     GetSaveGameList( s32 SaveGameType, s32* pMaxSaveGameTypeCount, SingleSavedGameInfo** pSaveGameListings );

        xbool   IsCommandComplete( void );
        xbool   GetLastCommandValues( s32* pReturnValue, s32* pErrorCode );
        xbool   IsWriteInProgress( void );

protected:
        void    CompleteCommandAndWaitForUser( s32 ReturnValue, s32 ErrorCode );

        s32     RequestNewCommand( s32 NewCommand );
        xbool   IsCommandInProgress( void );
        void    SetRequestedCommandHandler( s32 NewCommand );


        //-------------------------------------------------------------------------------------------------------------------------------------
        // Command handling functions.
        //-------------------------------------------------------------------------------------------------------------------------------------
        void    UpdateFreeBlocksAndFiles( void );
//        void    SetAndFillCurrentPCSaveGameData( s32 SaveGameType, s32 SaveGameIndex, XGAME_FIND_DATA* pPCFindGameData );
//        XGAME_FIND_DATA* GetCurrentPCSaveGameData( s32 SaveGameType, s32 SaveGameIndex );


        void    MountHandler( void );
        xbool   MountHandlerGetSavedGames( void );

        //-------------------------------------------------------------------------------------------------------------------------------------
        void    FormatHandler( void );

        //-------------------------------------------------------------------------------------------------------------------------------------
        void    SaveGameHandler( void );
        xbool   SaveGameHandlerUserDataFile( void );

        //-------------------------------------------------------------------------------------------------------------------------------------
        void    LoadGameHandler( void );
        xbool   LoadGameHandlerUserDataFile( void );

        //-------------------------------------------------------------------------------------------------------------------------------------
        void    DeleteGameHandler( void );

        //-------------------------------------------------------------------------------------------------------------------------------------
        // Utility Functions
        //-------------------------------------------------------------------------------------------------------------------------------------
        void    ClearAvailableGamesList( void );
        void    GetUserDataFileName( s32 SaveGameType, s32 TypeIndex, char* Buffer );

        char*   GetCommandNameAsString( s32 CommandID );

//        xbool   MatchUpPCSaveToExpectedSaveGameType( XGAME_FIND_DATA* pPCSave ); 


        //-------------------------------------------------------------------------------------------------------------------------------------
        // Icon Functions
        //-------------------------------------------------------------------------------------------------------------------------------------
        void    BuildIconFromResourceData( byte* RawIconData, s32 Size );


//-------------------------------------------------------------------------------------------------------------------------------------
// Data for GCMemoryUnitManager 
//-------------------------------------------------------------------------------------------------------------------------------------
protected:
        xbool   mInitialized;
        xbool   mbPCMemoryCardInsertionDetectedLastFrame;

        //-------------------------------------------------------------------------------------------------------------------------------------
        // Command Related Data.
        //-------------------------------------------------------------------------------------------------------------------------------------
        void    (PCMemoryUnitManager::*mpCurrentCommandHandler)(void);
        s32     mCurrentCommand;
        s32     mCommandState;
        s32     mCommandSubState;

        xbool   mbCommandCompleted;
        xbool   mbWaitingForCommandCompleteAcknowledged;

        s32     mCommandReturnValue;
        s32     mCommandError;

        f64     mCommandStartTime;

        //-------------------------------------------------------------------------------------------------------------------------------------
        // Icon / Browser Data
        //-------------------------------------------------------------------------------------------------------------------------------------
        byte*   mIconData;
        s32     mIconDataSize;
        char    mBrowserTitleText[MEMORY_UNIT_SAVE_GAME_TITLE_LENGTH];


        //-------------------------------------------------------------------------------------------------------------------------------------
        // SaveGame structure
        //-------------------------------------------------------------------------------------------------------------------------------------
        s32                         mNumberOfSaveGameTypes; // Number of different save game types for this app.
        PCMUResourceSaveGameType* mpSaveGameTypes;        // Info on each one of those types.

        SaveGameTypeAvailableList*  mpAvailableSavedGames;  // A running list of which saved games are available on the active card.
//        XGAME_FIND_DATA*            mpPCSaveGameData;     // PC specific save game info.
//        XGAME_FIND_DATA*            mpCurrentPCSaveGameData;
        s32                         mMaxPCSavesForThisApp;    // The maximum number of valid saves that this app is expecting.
        s32                         mNumExpectedGamesFound;     // The number of save games found that this game is expecting.
        s32                         mNumSaveGamesFound;         // The number of saves that were found in this apps directory, whether they should have been or not.

        char                        mWideNameBuffer[MEMORY_UNIT_SAVE_GAME_TITLE_LENGTH];
        PCFileInfo                mFileInfo;

        s32                         mCurrentGameType;
        s32                         mCurrentGameTypeIndex;


        //-------------------------------------------------------------------------------------------------------------------------------------
        // Actual Data for the memory cards.
        //-------------------------------------------------------------------------------------------------------------------------------------
        PCMemoryUnit              mHardDisk;
        xbool                       mbHardDiskIsSelected;
        char                        mActiveBaseDirectory[PCMU_MAX_PATH_LENGTH];
};

//=====================================================================================================================================
// Globals
//=====================================================================================================================================
extern PCMemoryUnitManager* gpPCMemoryUnitManager;

#endif // PC_MEMORY_UNIT_HPP
