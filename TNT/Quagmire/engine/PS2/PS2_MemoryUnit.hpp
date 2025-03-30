#ifndef PS2_MEMORY_UNIT_HPP
#define PS2_MEMORY_UNIT_HPP

//=====================================================================================================================================
// Required Includes
//=====================================================================================================================================
#include "x_files.hpp"
#include <libmc.h>
#include "Q_MemoryUnit.hpp"


//=====================================================================================================================================
// Forward Declarations
//=====================================================================================================================================


//=====================================================================================================================================
// Data types.
//=====================================================================================================================================


//=====================================================================================================================================
// Definitions
//=====================================================================================================================================
#define MEMORY_UNIT_PERSONALIZED_NAME_SIZE      45

// Card Types 
//-------------------------------------------------------------------------------------------------------------------------------------
#define PS2MU_NOCARD            0
#define PS2MU_PS1CARD           1
#define PS2MU_8MBCARD           2
#define PS2MU_POCKETSTATION     3

// Port and Slot Information
//-------------------------------------------------------------------------------------------------------------------------------------
#define PS2MU_MIN_PORT          0
#define PS2MU_MAX_PORT          1
#define PS2MU_MIN_SLOT          0
#define PS2MU_MAX_SLOT          3

/*
    QUAG_MEMORY_UNIT_ID_0 --> Port0 - Slot0
    QUAG_MEMORY_UNIT_ID_1 --> Port0 - Slot1
    QUAG_MEMORY_UNIT_ID_2 --> Port0 - Slot2
    QUAG_MEMORY_UNIT_ID_3 --> Port0 - Slot3
    QUAG_MEMORY_UNIT_ID_4 --> Port1 - Slot0
    QUAG_MEMORY_UNIT_ID_5 --> Port1 - Slot1
    QUAG_MEMORY_UNIT_ID_6 --> Port1 - Slot2
    QUAG_MEMORY_UNIT_ID_7 --> Port1 - Slot3
*/

// Card Dynamics
//-------------------------------------------------------------------------------------------------------------------------------------
#define PS2MU_SIZE_OF_BLOCK           1024


// Icon / Browser Specific
//-------------------------------------------------------------------------------------------------------------------------------------
#define PS2MU_ICON_DATA_FILENAME_SIZE           32
#define PS2MU_BROWSER_STRING_LENGTH             33

#define PS2MU_SPECIAL_ASCII_SJIS_TABLE_SIZE     33
#define PS2MU_REGULAR_ASCII_SJIS_TABLE_SIZE     3

#define PS2MU_UTILITY_FILENAME_SIZE             255


//=====================================================================================================================================
// PS2ConsoleIconData
//
// Structure which contains all the data to make the Icon.sys and Icon.ico files for a 
// PS2 save file.
//=====================================================================================================================================
struct PS2ConsoleIconData
{
    PS2ConsoleIconData( void );
    ~PS2ConsoleIconData( void );
    void ReleaseData( void );

    byte*           mpIconData;
    s32             mIconDataSize;

    sceMcIconSys    mSonyMcIconSys;  // SONY Required information latter saved as Icon.sys.  This
                                     // This structure contains the instructions of how to setup the browser screen when 
                                     // a user selects a saved game icon for this title.
};


//=====================================================================================================================================
// PS2MemoryUnit
//=====================================================================================================================================
struct PS2MemoryUnit
{
    //-------------------------------------------------------------------------------------------------------------------------------------
    PS2MemoryUnit( void );
    void    Initialize( s32 UnitID );
    void    Reset( void );
    void    SetName( char* NewPersonalizedName );

    //-------------------------------------------------------------------------------------------------------------------------------------
    s32     mUnitID;                                    // Location of the memory unit in the system.

    xbool   mbConnected;                                // Is the unit detected as inserted to the console.
    xbool   mbFormatted;                                // Is the unit ?

    s32     mType;                                      // This is a platform specific value, that may or not be used.
    u32     mFreeClusters;                              // Number of free Clusters on the Memory Unit.
    char    mName[MEMORY_UNIT_PERSONALIZED_NAME_SIZE];  // Name for this unit based on location in the system.
};


//=====================================================================================================================================
// PS2MemoryUnitManager
//=====================================================================================================================================
class PS2MemoryUnitManager
{
    public:
        //-------------------------------------------------------------------------------------------------------------------------------------
        // Construction / Initialization
        //-------------------------------------------------------------------------------------------------------------------------------------
        PS2MemoryUnitManager( void );
        xbool Initialize( char* MemoryUnitResourceFilename );
        void  DeInitialize( void );


        //-------------------------------------------------------------------------------------------------------------------------------------
        // Destruction / Kill
        //-------------------------------------------------------------------------------------------------------------------------------------
        ~PS2MemoryUnitManager( void );


        //-------------------------------------------------------------------------------------------------------------------------------------
        // Memory Unit System Scanning control
        //-------------------------------------------------------------------------------------------------------------------------------------
        void    EnableScanning( void )      { ASSERT( mInitialized ); mbScanningEnabled = TRUE;  }
        void    DisableScanning( void )     { mbScanningEnabled = FALSE; }
        xbool   IsScanningEnabled( void )   { return mbScanningEnabled;  }


        //-------------------------------------------------------------------------------------------------------------------------------------
        // Update Functions.
        //-------------------------------------------------------------------------------------------------------------------------------------
        void    Update( void );

        
        //-------------------------------------------------------------------------------------------------------------------------------------
        // Interface Functions.
        //-------------------------------------------------------------------------------------------------------------------------------------
        void    SetBrowserText( char* pBrowserInfo, s32 BreakCharacter );


        //-------------------------------------------------------------------------------------------------------------------------------------
        // Actual Memory Card Related Functions.
        //-------------------------------------------------------------------------------------------------------------------------------------
        xbool   IsMemoryUnitConnected( s32 MemoryCardID );
		xbool	IsMemoryUnitFormatted( s32 MemoryCardID );
        xbool   IsActiveMemoryUnitStillConnected( void );
        char*   GetNameOfActiveMemoryUnit( void );
        s32     GetFreeSpaceRemainingOnActiveUnit( void );

        s32     SelectActiveMemoryUnit( s32 MemoryUnitID );
        s32     ReleaseActiveMemoryUnit( void );

        s32     CalculateRequiredSizeForSaveGameType( s32 SaveGameType, s32* pRequiredBlocks, xbool bWorstCase );
        s32     GetUserDataByteSize( s32 SaveGameType );

        s32     FormatCard( void );

        s32     SaveGame( s32 SaveGameType, s32 TypeOffset, void* pUserData, char* SaveGameBrowserInfo, s32 BrowserInfoBreakCharacter );
        s32     LoadGame( s32 SaveGameType, s32 TypeOffset, void* pUserData );
        s32     DeleteGame( s32 SaveGameType, s32 TypeOffset );

        s32     GetSaveGameList( s32 SaveGameType, s32* pMaxSaveGameTypeCount, SingleSavedGameInfo** pSaveGameListings );

        xbool   IsCommandComplete( void );
        xbool   GetLastCommandValues( s32* pReturnValue, s32* pErrorCode );
        xbool   IsWriteInProgress( void );

        xbool   GenerateCardName( s32 QuagMemCardID, char* pBuffer, s32 SizeOfBuffer );

protected:
        void    CompleteCommandAndWaitForUser( s32 ReturnValue, s32 ErrorCode );

        xbool   StartBackgroundCardCheck( s32 MemoryUnitID );
        void    ContinueBackgroundCardCheck( void );
        void    CompleteBackgroundCardCheck( void );

        s32     RequestNewCommand( s32 NewCommand );
        xbool   IsCommandInProgress( void );
        void    SetRequestedCommandHandler( s32 NewCommand );

        // Command handling functions.
        //-------------------------------------------------------------------------------------------------------------------------------------
        xbool   ReEvaluateCardStatus( void );

        void    MountHandler( void );
        xbool   MountHandlerGetSavedGames( void );

        //-------------------------------------------------------------------------------------------------------------------------------------
        void    FormatHandler( void );
        void    UnFormantHandler( void );

        //-------------------------------------------------------------------------------------------------------------------------------------
        void    SaveGameHandler( void );
        xbool   SaveGameHandlerCreateBaseDir( void );
        xbool   SaveGameHandlerSaveSingleFile( s32 FileType );

        //-------------------------------------------------------------------------------------------------------------------------------------
        void    LoadGameHandler( void );

        //-------------------------------------------------------------------------------------------------------------------------------------
        void    DeleteGameHandler( void );
        xbool   DeleteGameHandlerDeleteBaseDir( void );
        xbool   DeleteGameHandlerDeleteSingleFile( s32 FileType );
       

        //-------------------------------------------------------------------------------------------------------------------------------------
        // Utility Functions
        //-------------------------------------------------------------------------------------------------------------------------------------
        void    GeneratePersonalizedNameBasedOnPortAndSlot( PS2MemoryUnit* pMemoryUnit );
            
        void    ProcessCardConnected( PS2MemoryUnit* pMemoryUnit );
        void    ProcessCardDisconnected( PS2MemoryUnit* pMemoryUnit );

        void    GetPS2PortAndSlotFromMemoryUnitID( s32 MemUnitID, s32* CurrentPort, s32* CurrentSlot );
        s32     GetMemoryUnitIDFromPS2PortAndSlot( s32 CurrentPort, s32 CurrentSlot );

        void    ClearAvailableGamesList( void );

        xbool   SetCurrentFileHandle( s32 NewFileHandle );
        xbool   IsFileHandleInUse( void );
        void    ClearCurrentFileHandle( void );

        void    ResetCommandHandlingVariables( void );

        //-------------------------------------------------------------------------------------------------------------------------------------
        // Icon Functions
        //-------------------------------------------------------------------------------------------------------------------------------------
        void    InitializeAllDefaultIconData( void );
        void    BuildIconFromResourceData( PS2MemCardResourceIconInfo* pResourceData, byte* RawIconData );

        char* GetCommandNameAsString( s32 CommandID );


//-------------------------------------------------------------------------------------------------------------------------------------
// Data for PS2MemoryUnitManager 
//-------------------------------------------------------------------------------------------------------------------------------------
protected:
        xbool                   mInitialized;

        //-------------------------------------------------------------------------------------------------------------------------------------
        // Command Related Data.
        //-------------------------------------------------------------------------------------------------------------------------------------
        void (PS2MemoryUnitManager::*mpCurrentCommandHandler)(void);
        s32                     mCurrentCommand;
        s32                     mCommandState;
        s32                     mCommandSubState;
        f64                     mCommandStartTime;

        xbool                   mbCommandCompleted;
        xbool                   mbWaitingForCommandCompleteAcknowledged;

        s32                     mCommandReturnValue;
        s32                     mCommandError;
   

        
        //-------------------------------------------------------------------------------------------------------------------------------------
        // System Scanning Data
        //-------------------------------------------------------------------------------------------------------------------------------------
        xbool                   mbScanningEnabled;
        xbool                   mbAllowBackgroundCardChecking;
        xbool                   mbBackgrounCardCheckInProgress;
        s32                     mCurrentBackgroundCardBeingChecked;

        s32                     mMaxPortToScan;
        s32                     mMaxSlotToScan;

        
        //-------------------------------------------------------------------------------------------------------------------------------------
        // Low Level PS2 related.
        //-------------------------------------------------------------------------------------------------------------------------------------
        enum PS2MU_FileTypes
        {
            PS2MU_FILE_TYPE_BASE_DIR_NAME_FILE,
            PS2MU_FILE_TYPE_ICON_INFO,
            PS2MU_FILE_TYPE_ICON_DATA,
            PS2MU_FILE_TYPE_BROWSE_TEXT,
            PS2MU_FILE_TYPE_USER_DATA,
            TOTAL_PS2MU_FILE_TYPES,
        };

        s32                     mOpenFileHandle;
        char                    mCurrentOpenFileName[PS2MU_UTILITY_FILENAME_SIZE];
        void*                   mpOpenGameFileData;
        s32                     mOpenGameFileSize;


        static s32              msBaseDirNameFileData;
        static s32              msBaseDirNameFileSize;

        //-------------------------------------------------------------------------------------------------------------------------------------
        // Icon / Browser Data
        //-------------------------------------------------------------------------------------------------------------------------------------
        PS2ConsoleIconData      mIconData;
        char                    mBrowserTitleText[PS2MU_BROWSER_STRING_LENGTH];
        s32                     mBrowserTitleBreak;     


        //-------------------------------------------------------------------------------------------------------------------------------------
        // SaveGame structure
        //-------------------------------------------------------------------------------------------------------------------------------------
        s32                             mNumberOfSaveGameTypes; // Number of different save game types for this app.
        PS2MemCardResourceSaveGameType* mpSaveGameTypes;        // Info on each one of those types.
       
        SaveGameTypeAvailableList*      mpAvailableSavedGames;  // A running list of which saved games are available on the active card.

        s32                             mCurrentGameType;
        s32                             mCurrentGameTypeIndex;
        void*                           mpUserBuffer;

        //-------------------------------------------------------------------------------------------------------------------------------------
        // Actual Data for the memory cards.
        //-------------------------------------------------------------------------------------------------------------------------------------
        PS2MemoryUnit                   mPS2MemoryUnits[QUAG_MEMORY_UNIT_MAX_ID + 1];
        PS2MemoryUnit*                  mActiveMemoryUnit;
        s32                             mActiveMemoryUnitPort;
        s32                             mActiveMemoryUnitSlot;
        s32                             mActiveMemoryUnitReusableEnvironmentSpace;

};


//=====================================================================================================================================
// Globals
//=====================================================================================================================================
extern PS2MemoryUnitManager* gpPS2MemoryUnitManager;


#endif // PS2_MEMORY_UNIT_HPP