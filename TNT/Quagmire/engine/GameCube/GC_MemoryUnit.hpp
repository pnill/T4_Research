#ifndef GC_MEMORY_UNIT_HPP
#define GC_MEMORY_UNIT_HPP

//=====================================================================================================================================
//
// INFORMATION ON THE GAMECUBE MEMORY CARD.
// 
//=====================================================================================================================================
//
// Acclaim Introduction to the GameCube Memory Card:
// =================================================
// 
//  - Default memory card are 512KBytes (4 Megabit)
//  - Cards are divided in blocks or Blocks of 8K.  
//  - There are a total of 64 Blocks per 4 Megabit card.
//  - 5 Blocks per card are reserved for system use, leaving a total of 59 Blocks, or 472Kbytes.
//  - Files saved to the memory card must all be multiples of the 8K Block size.
//  - In the future, there will be larger capacity memory cards that may contain different Block sizes.
//
//=====================================================================================================================================
//
// GameCube memory card implementation notes:
// ==========================================
//
// The Gamecube memory card system is very similar to the PS2 with the exception that the cards must go through a 
// process of mounting and un-mounting each time they are connected and disconnect.  This mounting and un-mounting
// requires the use of around 40K (32Byte aligned) of space for the memory card to use as a scratch memory buffer.  
// The Gamecube also allows you to process all of the memory card operations using Synchronous or Asynchronous commands.
// I am going to use the Asynchronous method since it's the same as our current implementation of the PS2.  
//
//
//=====================================================================================================================================
//
// Notes from nintendo from the GameCube Reference Manual (May 2001)
// =================================================================
// 
// Appendix: Memory card size and its Block size
// ----------------------------------------------
// Although the CARD API supports 4, 8, 16, 32, 64, and 128 Mb memory cards, and 8, 16, 32, 64, 128, and 256 KB 
// Block sizes, only the 4Mb (8KB Block) memory card will be available at system launch. 
// Any game program must support memory cards whose Block size is 8KB. 
// Note: Nintendo officially discourages the support of non-8KB memory cards for game programs currently 
// under development. [Effective 5/30/2001] 
//
//=====================================================================================================================================
//
//
// CARD Banners and Icons: (Taken directly from the GameCube Reference Manual)
// ===========================================================================
//
// Introduction:
// ------------
// A memory file can have different icon, banner, and comment configurations. A memory card must have at least 
// one 32x32 icon to display in the IPL Memory Card screen, and two 32-byte long (non-null-terminated) character 
// strings. The 96x32 banner is optional. Banners and icons can be in two formats, RGB5A3 or C8  
// (icons formats can be intermixed if desired). The minimum size requirement for system information is a 32x32 C8 
// icon and 64 bytes of comment strings, or 1600 bytes. 
//
// By default the default icon will only be one frame consiting of a red and white exclamation point.
// Also, Banner support has not been provided in this version of the game cube memory card support.
//
//=====================================================================================================================================
//
//-------------------------------------------------------------------------------------------------------------------------------------
// The structure of a simple game cube save game, using a paletized texture for the icon, and a 16 bit color CLUT.
//-------------------------------------------------------------------------------------------------------------------------------------
//
//  ----------------------
//  | Comments           |  (64 bytes)  1st 32 are for the Games Title
//  | Comments           |              2nd 32 are for the comments.
//  ----------------------
//  | IconTexels Frame 0 | (1024bytes)  Frame 1 of the animated icon
//  | IconTexels Frame 1 | (1024bytes)  Frame 2 of the animated icon
//  | IconTexels Frame 2 | (1024bytes)  Frame 3 of the animated icon
//  |...                 | (1024bytes * number of filler frames)
//  | IconTexels Frame X | (1024bytes)  Frame X (up to 8 Max) of the animated icon.
//  ----------------------
//  | Icon Clut          | (512 Bytes)  
//  ----------------------
//  | GameData           |  Actual save game data.
//  | GameData           |
//  | GameData           |
//  | GameData           |
//  | GameData           |
//  | GameData           |
//  ----------------------
//
//=====================================================================================================================================
// Required Includes
//=====================================================================================================================================
#include "x_files.hpp"
#include <dolphin/card.h>
#include "Q_MemoryUnit.hpp"
#include "GC_MemoryUnitResource.hpp"


//=====================================================================================================================================
// Forward Declarations
//=====================================================================================================================================


//=====================================================================================================================================
// Definitions
//=====================================================================================================================================
// Card Dynamics
//-------------------------------------------------------------------------------------------------------------------------------------
#define MEMORY_UNIT_PERSONALIZED_NAME_SIZE      45


#define GAMECUBE_MEMCARD_SLOTA          0       // EXI Slot on the left of the GameCube     (Bottom on Devkit)
#define GAMECUBE_MEMCARD_SLOTB          1       // EXI Slot on the right of the GameCube    (Top on Devkit)
/*
    QUAG_MEMORY_UNIT_ID_0 --> GAMECUBE_MEMCARD_SLOTA
    QUAG_MEMORY_UNIT_ID_1 --> GAMECUBE_MEMCARD_SLOTB
*/

#define GCMU_SUPPORTED_SECTOR_SIZE     (1024 * 8)   // Supported cluster values are 8K.
#define GCMU_BROWSER_LINE_LENGTH       32




// Saved game related.
//-------------------------------------------------------------------------------------------------------------------------------------


// Icon / Browser Related.
//-------------------------------------------------------------------------------------------------------------------------------------
#define GC_MEMCARD_ICON_FRAME_SIZE      (1024)
#define GC_MEMCARD_CLUT_SIZE            (512)   // 256 entries at 16 bits each using RGB5A4 format.
#define GC_MEMCARD_FILENAME_SIZE        (CARD_FILENAME_MAX)

#define GC_MEMCARD_ANIMATE_FRAME_FAST   (CARD_STAT_SPEED_FAST)
#define GC_MEMCARD_ANIMATE_FRAME_NORMAL (CARD_STAT_SPEED_MIDDLE)
#define GC_MEMCARD_ANIMATE_FRAME_SLOW   (CARD_STAT_SPEED_SLOW)


//=====================================================================================================================================
// GCConsoleIconData
//
// Structure which contains all the data to make the Animated Icon for GameCube Saves.
//=====================================================================================================================================
struct GCConsoleIconData
{
    GCConsoleIconData( void );
    ~GCConsoleIconData( void );
    void    ReleaseIconData( void );

    byte*   mpIconData;                     // The actual icon frame data.
    s32     mIconDataSize;                  // Size in bytes of the icon frames, and the clut.
    s32     mIconAnimSpeed;                 // Speed of the animated icon used by this
    s32     mNumberOfIconFrames;            // The number of frames in the icon.
};


//=====================================================================================================================================
// GCMemoryUnit
//=====================================================================================================================================
struct GCMemoryUnit
{
    //-------------------------------------------------------------------------------------------------------------------------------------
    GCMemoryUnit( void );
    void    Initialize( s32 UnitID );
    void    Reset( void );

    //-------------------------------------------------------------------------------------------------------------------------------------
    s32     mUnitID;                                    // Location of the memory unit in the system.
    char    mName[MEMORY_UNIT_PERSONALIZED_NAME_SIZE];  // Name for this unit based on location in the system.

    xbool   mbConnected;    // Is the unit detected as inserted to the console.
    xbool   mbCompatible;   // Does this card have a supported block size.
    xbool   mbFormatted;    // Is the unit formatted?
    xbool   mbMounted;      // Is the unit mounted.

    s32     mTotalSize;     // Total capacity of the memory card in Bytes.
    s32     mSectorSize;    // Size of a single sector on the card, in bytes.
    s32     mFreeBytes;     // Space remaining on the card in bytes.
    s32     mFreeBlocks;    // Space remaining on the card in blocks/sectors..
    s32     mFreeFiles;     // Number of remaining files available on the card.
};


//=====================================================================================================================================
// GCMU_FileInfo
//=====================================================================================================================================
struct GCMU_FileInfo // GameCube Memory Card File Info.
{
    GCMU_FileInfo( void );
   ~GCMU_FileInfo( void );
    void Reset( xbool LeaveUserBufferAlone = TRUE );

    //-------------------------------------------------------------------------------------------------------------------------------------
    // GameCube System Required.
    //-------------------------------------------------------------------------------------------------------------------------------------
    byte*           mpSectorDataBuffer;             // Buffer to store 1 Block worth of data.
    CARDFileInfo    mGameCubeCARDFileInfo;          // Required by the GameCube during file operations.
    CARDStat        mGameCubeCARDStat;              // Statistics on the currently open or created file.
    s32             mSectorBufferOffset;
    s32             mSectorTransferCount;

    s32             mUserBytesToGo;                 // Number of user bytes still in the file to be read.
    s32             mMemCardOffset;                 // Read or write offset into the memory card (sector alligned).
    s32             mSectorBytesTransferred;        // The number of bytes read from or written to the sector buffer.
    xbool           mbSectorLimit;                  // On Reads, the sector is empty, on writes, the sector is full.
    xbool           mbOnlyReadingTheBrowserText;    // Flag to indicate on a read that we are only interested in the
                                                    // 64 bytes of browser text.
    s32             mFileTransferStatus;
    
    //-------------------------------------------------------------------------------------------------------------------------------------
    // File Specifics
    //-------------------------------------------------------------------------------------------------------------------------------------
    char            mFileName[GCMU_USER_DATA_FILENAME_LENGTH];  // Name of the file to be saved onto the memory card.  This is the system
                                                                // name, not the name seen by the user in the browser.
    xbool           mbOpen;                                     // The File is open.
    xbool           mbWriteAccess;                              // Write Access or read access.

    s32             mTotalFileSize;                             // Total Size of the file with the icon, comments and data.
    s32             mFileDataSize;                              // Size of non-system data.  Basically, this is the amount of bytes
                                                                // in the file that are game specific.

    //-------------------------------------------------------------------------------------------------------------------------------------
    // Read/Write info and data supplied by user.
    //-------------------------------------------------------------------------------------------------------------------------------------
    byte*           mpUserDataBuffer;               // Pointer to read from on a read, or on a write to on a write.
    s32             mUserBufferSize;                // Size of the buffer provided by the user.
    s32             mUserBufferOffset;              // Offset into the user buffer when reading or writing.
    xbool           mbReadingOnlyBrowseInfo;        // Special case setting to allow you to read data including the browser comments
                                                    // and icon data.
    s32             mUserCommandValue1;             // Extra user value used for some commands.
    s32             mUserCommandValue2;             // Extra user value used for some commands.
};




//=====================================================================================================================================
// GCMemoryUnitManager
//=====================================================================================================================================
class GCMemoryUnitManager
{
    friend void GCMUCallBack_CardDisconnected( s32 Slot, s32 Result );

    public:
        //-------------------------------------------------------------------------------------------------------------------------------------
        // Construction / Initialization
        //-------------------------------------------------------------------------------------------------------------------------------------
        GCMemoryUnitManager( char* ApplicationName );
        xbool   Initialize( char* MemoryUnitResourceFilename );
        void    DeInitialize( void );

        //-------------------------------------------------------------------------------------------------------------------------------------
        // Destruction / Kill
        //-------------------------------------------------------------------------------------------------------------------------------------
        ~GCMemoryUnitManager( void );

        //-------------------------------------------------------------------------------------------------------------------------------------
        // Memory Unit System Scanning control
        //-------------------------------------------------------------------------------------------------------------------------------------
        void    EnableScanning( void )      { ASSERT( mInitialized ); mbScanningEnabled = TRUE;  }
        void    DisableScanning( void )     { mbScanningEnabled = FALSE; }
        xbool   IsScanningEnabled( void )   { return mbScanningEnabled;  }

        //-------------------------------------------------------------------------------------------------------------------------------------
        // LAST MINUTE ADDITION FOR SAVEGAME SAFETY
        //-------------------------------------------------------------------------------------------------------------------------------------
		void    SetWritingSaveGame( xbool SaveGameStatus ) { mbSavingInProgress = SaveGameStatus; }

        //-------------------------------------------------------------------------------------------------------------------------------------
        // Update Functions.
        //-------------------------------------------------------------------------------------------------------------------------------------
        void    Update( void );

        //-------------------------------------------------------------------------------------------------------------------------------------
        // Interface Functions.
        //-------------------------------------------------------------------------------------------------------------------------------------
        void    SetBrowserText( char* pBrowserInfo );
        void    GetBrowserText( char* Buffer, s32 Size );

        //-------------------------------------------------------------------------------------------------------------------------------------
        // Actual Memory Card Related Functions.
        //-------------------------------------------------------------------------------------------------------------------------------------
        xbool   IsMemoryUnitConnected( s32 MemoryCardID );
        xbool   IsActiveMemoryUnitStillConnected( void );
        char*   GetNameOfActiveMemoryUnit( void );
        s32     GetFreeSpaceRemainingOnActiveUnit( void );
        s32     GetFreeFilesRemainingOnActiveUnit( void );

        s32     SelectActiveMemoryUnit( s32 MemoryUnitID, byte* pFileIOBuffer, s32 BufferSize );
        s32     ReleaseActiveMemoryUnit( void );

        s32     CalculateRequiredSizeForSaveGameType( s32 SaveGameType, s32* pRequiredBlocks, xbool bWorstCase );
        s32     GetUserDataByteSize( s32 SaveGameType );
        s32     FormatCard( void );

        s32     SaveGame( s32 SaveGameType, s32 TypeOffset, void* pUserData, char* SaveGameBrowserInfo );
        s32     LoadGame( s32 SaveGameType, s32 TypeOffset, void* pUserData );
        s32     DeleteGame( s32 SaveGameType, s32 TypeOffset );

        s32     GetSaveGameList( s32 SaveGameType, s32* pMaxSaveGameTypeCount, SingleSavedGameInfo** pSaveGameListings );

        xbool   IsCommandComplete( void );
        xbool   GetLastCommandValues( s32* pReturnValue, s32* pErrorCode );
        xbool   IsWriteInProgress( void );

        xbool   GenerateCardName( s32 QuagMemCardID, char* pBuffer, s32 SizeOfBuffer );

protected:
        void    CompleteCommandAndWaitForUser( s32 ReturnValue, s32 ErrorCode );

        s32     RequestNewCommand( s32 NewCommand );
        xbool   IsCommandInProgress( void );
        void    SetRequestedCommandHandler( s32 NewCommand );

        void    ResetCommandHandlingVariables( void );

        //-------------------------------------------------------------------------------------------------------------------------------------
        // Command handling functions.
        //-------------------------------------------------------------------------------------------------------------------------------------
        void    UpdateFreeBlocksAndFiles( void );

        void    MountHandler( void );
        xbool   MountHandlerGetSavedGames( void );

        //-------------------------------------------------------------------------------------------------------------------------------------
        void    FormatHandler( void );
        void    UnFormatHandler( void );

        //-------------------------------------------------------------------------------------------------------------------------------------
        void    SaveGameHandler( void );

        //-------------------------------------------------------------------------------------------------------------------------------------
        void    LoadGameHandler( void );

        //-------------------------------------------------------------------------------------------------------------------------------------
        void    DeleteGameHandler( void );

        //-------------------------------------------------------------------------------------------------------------------------------------
        // Utility Functions
        //-------------------------------------------------------------------------------------------------------------------------------------
        void    ProcessCardConnected( GCMemoryUnit* pMemoryUnit );
        void    ProcessCardDisconnected( GCMemoryUnit* pMemoryUnit );

        s32     GetSlotFromMemoryUnitID( s32 MemUnitID );
        s32     GetMemoryUnitIDFromSlot( s32 CurrentSlot );

        void    ClearAvailableGamesList( void );
        void    GetUserDataFileName( s32 SaveGameType, s32 TypeIndex, char* Buffer );

        s32     ConvertCARDSysResultToQuagResult( s32 CARDResult );

        char*   GetCommandNameAsString( s32 CommandID );

        //-------------------------------------------------------------------------------------------------------------------------------------
        // File transfer functions.
        //-------------------------------------------------------------------------------------------------------------------------------------
        s32     PrepareForFileTransfer( xbool bWrite, xbool bReadingOnlyBrowseInfo );
        xbool   PackDataIntoSector(  byte* pDataPointer, s32 DesiredBytesToAdd, s32* pActualBytesAdded );
        xbool   StripDataFromSector( byte* pDataPointer, s32 DesiredBytesToGet, s32* pActualBytesGiven );

        s32     OpenFile( xbool Write );
        s32     ProcessOpenCreateComplete( s32 CARDReturn  );

        xbool   ReadFile( s32& ErrorCode );
        xbool   WriteFile( s32& ErrorCode );
        s32     ProcessReadWriteComplete( s32 CARDReturn );

        //-------------------------------------------------------------------------------------------------------------------------------------
        // Icon Functions
        //-------------------------------------------------------------------------------------------------------------------------------------
        void    BuildIconFromResourceData( GCMUResourceIconInfo* pResourceData, byte* RawIconData );

//-------------------------------------------------------------------------------------------------------------------------------------
// Data for GCMemoryUnitManager 
//-------------------------------------------------------------------------------------------------------------------------------------
protected:
        xbool   mInitialized;

        //-------------------------------------------------------------------------------------------------------------------------------------
        // Command Related Data.
        //-------------------------------------------------------------------------------------------------------------------------------------
        void (GCMemoryUnitManager::*mpCurrentCommandHandler)(void);
        s32     mCurrentCommand;
        s32     mCommandState;
        s32     mCommandSubState;

        xbool   mbCommandCompleted;
        xbool   mbWaitingForCommandCompleteAcknowledged;

        s32     mCommandReturnValue;
        s32     mCommandError;
        
        f64     mCommandStartTime;   

        
        //-------------------------------------------------------------------------------------------------------------------------------------
        // System Scanning Data
        //-------------------------------------------------------------------------------------------------------------------------------------
        xbool   mbScanningEnabled;
        xbool   mbAllowBackgroundCardChecking;
        xbool   mbBackgrounCardCheckInProgress;
        s32     mCurrentBackgroundCardBeingChecked;
		xbool   mbSavingInProgress;

        //-------------------------------------------------------------------------------------------------------------------------------------
        // Low Level GameCube related.
        //-------------------------------------------------------------------------------------------------------------------------------------
        GCMU_FileInfo   mFileInfo;          // System required information when dealing with files.
        void*           mpMountingWorkArea; // This buffer is used to cache gamecube system files from a mounted memory card.
                                            // Each card that is mounted requires one of these buffers to be allocated.
                                            // Each buffer will be CARD_WORKAREA_SIZE which is 40K.
        s32             mLastCARDCommandReturnValue;   // Last value return from the CARD system.

        //-------------------------------------------------------------------------------------------------------------------------------------
        // Icon / Browser Data
        //-------------------------------------------------------------------------------------------------------------------------------------
        GCConsoleIconData           mIconData;
        char                        mBrowserTitleText[GCMU_BROWSER_LINE_LENGTH];
        char                        mBrowserCommentText[GCMU_BROWSER_LINE_LENGTH];


        //-------------------------------------------------------------------------------------------------------------------------------------
        // SaveGame structure
        //-------------------------------------------------------------------------------------------------------------------------------------
        s32                         mNumberOfSaveGameTypes; // Number of different save game types for this app.
        GCMUResourceSaveGameType*   mpSaveGameTypes;        // Info on each one of those types.

        SaveGameTypeAvailableList*  mpAvailableSavedGames;  // A running list of which saved games are available on the active card.
        s32                         mCurrentGameType;
        s32                         mCurrentGameTypeIndex;


        //-------------------------------------------------------------------------------------------------------------------------------------
        // Actual Data for the memory cards.
        //-------------------------------------------------------------------------------------------------------------------------------------
        GCMemoryUnit                mGCMemoryUnits[QUAG_MEMORY_UNIT_MAX_ID + 1];    // Basic info for each card used.
        GCMemoryUnit*               mpActiveMemoryUnit;                              // Pointer to the currently selected card.
        s32                         mActiveMemoryUnitSlot;                          // Slot for currently selected card.
};

//=====================================================================================================================================
// Globals
//=====================================================================================================================================
extern GCMemoryUnitManager* gpGCMemoryUnitManager;


#endif // GC_MEMORY_UNIT_HPP
