#ifndef Q_MEMORY_UNIT_HPP
#define Q_MEMORY_UNIT_HPP

//=====================================================================================================================================
// Required Includes
//=====================================================================================================================================
#include "x_files.hpp"
#include "Q_MemoryUnitResource.hpp"
// For more includes see end of file in the "Special includes files" section.


//=====================================================================================================================================
// Definitions
//=====================================================================================================================================
#define MEMORY_UNIT_SAVE_GAME_TITLE_LENGTH      32


// QuagMemoryUnitIDs identify unique memory storage units.  Each platform will have a limited number of them
// due to particular system limitations, but also, each QuagMemoryUnit will correspond to a particular console 
// id as well.  See the specific console Quagmire header files to get more information.
enum QuagMemoryUnitIDs
{
    QUAG_MEMORY_UNIT_ID_0,
    QUAG_MEMORY_UNIT_ID_1,
    QUAG_MEMORY_UNIT_ID_2,
    QUAG_MEMORY_UNIT_ID_3,
    QUAG_MEMORY_UNIT_ID_4,
    QUAG_MEMORY_UNIT_ID_5,
    QUAG_MEMORY_UNIT_ID_6,
    QUAG_MEMORY_UNIT_ID_7,
    QUAG_MEMORY_UNIT_ID_8,
};

#ifdef  TARGET_PS2       
    #define QUAG_MEMORY_UNIT_MAX_ID      QUAG_MEMORY_UNIT_ID_0      // Set to QUAG_MEMORY_UNIT_ID_7 if you want to use all PS2 cards.
#elif defined( TARGET_DOLPHIN )
    #define QUAG_MEMORY_UNIT_MAX_ID      QUAG_MEMORY_UNIT_ID_0      // Set to QUAG_MEMORY_UNIT_ID_1 if you wish to support both slot A and B.
                                                                    // Though you will have some fun TRC's to worry about from Nintendo if you do support both slots.
#elif defined( TARGET_XBOX ) || defined( TARGET_PC )
    #define QUAG_MEMORY_UNIT_MAX_ID      QUAG_MEMORY_UNIT_ID_0
#endif


//-------------------------------------------------------------------------------------------------------------------------------------
enum QuagMemoryUnitErrorCodes
{
    QUAG_MU_EVERYTHING_IS_OK,                       // Everything is alright.
    
    QUAG_MU_NO_ACTIVE_CARD_SELECTED,                // No card is currently selected as the active card.
    QUAG_MU_TRYING_TO_ACTIVATE_MORE_THAN_ONE_CARD,  // You are trying to activate another card when one was already selected.
    QUAG_MU_TRYING_TO_ACTIVATE_ILLEGAL_CARD,        // You are requesting for a card that doesn't exist on this platform.
    
    QUAG_MU_SYSTEM_BUSY,                            // System was busy, so the last command couldn't be executed.
    
    QUAG_MU_NO_CARD,                                // The selected Memory Unit is not conected.
    QUAG_MU_CARD_FOUND,                             // A new card was discovered.
    
    QUAG_MU_NOT_FORMATTED,                          // The selected Memory Unit is not formatted.
    QUAG_MU_ALREADY_FORMATTED,                      // The selected Memory Card is already formatted.
    QUAG_MU_INVALID_CARD,                           // The selected Memory Unit is not compatible.
    QUAG_MU_INVALID_FORMAT,                         // The selected Memory Unit may have been formatted for a system with a different country code.
    
    QUAG_MU_FILE_SYSTEM_DAMAGED,                    // The selected Memory Unit internal table is damaged, the selected Memory Unit must be re-formatted.
    QUAG_MU_CARD_PHYSICALLY_DAMAGED,                // The selected Memory Unit is physically damaged. It's esentially useless.
    
    QUAG_MU_TOO_MANY_FILES_ON_CARD,                 // The selected Memory Unit has reached it's max file limit.
    QUAG_MU_NOT_ENOUGH_FREE_SPACE,                  // The selected Memory Unit does not have enough space free to complete task.
    QUAG_MU_INVALID_FILENAME,                       // The name of the requested file was invalid.
    QUAG_MU_FILE_NOT_FOUND,                         // The file requested to could not be found on the selected Memory Unit.
    QUAG_MU_FILE_ALREADY_EXISTS,                    // The file to be created was already present the selected Memory Unit.
    QUAG_MU_FILE_ACCESS_DENIED,                     // Access to the requested file was denied.
    
    QUAG_MU_GENERIC_SAVE_FAILED,                    // If a save operation failed and there is no other explanation for it.
    QUAG_MU_GENERIC_LOAD_FAILED,                    // If a load operation failed and there is no other explanation for it.
    QUAG_MU_GENERIC_DELETE_FAILED,                  // If a delete operation failed and there is no other explanation for it.
    
    QUAG_MU_OVERSIZED_READ_WRITE,                   // The last read or write operation exceeded the size of the file.

    QUAG_MU_FILE_CONTAINS_BAD_DATA,                 // The file requested has detected that it's contents are corrupt.

    QUAG_MU_WRONG_DEVICE,                           // Device inserted into memory unit slot is not a memory card.

    QUAG_MU_UNKNOWN_PROBLEM,                        // Something unexpected is among us.
    QUAG_MU_TOTAL_ERRORS,
};


//-------------------------------------------------------------------------------------------------------------------------------------
enum QuagMemoryUnitEvents
{
    QUAG_MU_EVENT_COMPLETE,
    QUAG_MU_EVENT_FAILED,

    TOTAL_QUAG_MU_EVENTS,
};

//=====================================================================================================================================
// New Data Types.
//=====================================================================================================================================
/*
#define QUAGMIRE_SAVE_TYPE_FILENAME_SIZE    32

struct QuagmireGameCubeIconResourceData
{
    // Name of the file containing the actual Icon BMP Data.
    char    mIconDataFileName[QUAG_MU_ICON_DATA_FILENAME_SIZE];

    s32     mIconDataSize;              // Size of that Icon Data.
    byte*   mIconData;                  // Actual Icon Data.

    // Icon Attributes
    s32     mNumberOfFrames;            // Maximum is 8.
    s32     mAnimationSpeed;            // 
};




*/



//=====================================================================================================================================
// Common Interface structures.
//=====================================================================================================================================
struct SingleSavedGameInfo
{
    xbool   mbValid;
    s32     mSaveGameType;
    s32     mSaveGameTypeIndex;
    char    mSaveGameTitle[MEMORY_UNIT_SAVE_GAME_TITLE_LENGTH];
};

//-------------------------------------------------------------------------------------------------------------------------------------
struct SaveGameTypeAvailableList
{
    void Clear( s32 SaveGameIndex )
    {
        mSavedGames[SaveGameIndex].mbValid            = FALSE;
        mSavedGames[SaveGameIndex].mSaveGameType      = -1;
        mSavedGames[SaveGameIndex].mSaveGameTypeIndex = -1;
        mSavedGames[SaveGameIndex].mSaveGameTitle[0]  = 0;
    }
    SingleSavedGameInfo     mSavedGames[MEMORY_UNIT_MAX_SAVE_COUNT_PER_TYPE];
};


//-------------------------------------------------------------------------------------------------------------------------------------
struct MemoryUnitEvent
{
    s32     mEventType;
    s32     mReturnValue;
    s32     mErrorCode;
};




//=====================================================================================================================================
// MEMORY_UNIT Common Interface Functions.
//=====================================================================================================================================
// xbool MEMORY_UNIT_Create( char* ApplicationName );
// 
// Description: Creates the Memory Unit Interface.  It's not actually functional until it's initialized, but this prepares
//              the system for Memory Card use.
//
// Inputs: char* ApplicationName - String the size of MEMORY_UNIT_SAVE_GAME_TITLE_LENGTH that represents the title
//                                 of a game.  May or may not be used by all platforms.
//
//=====================================================================================================================================
xbool MEMORY_UNIT_Create( char* ApplicationName = NULL );


//=====================================================================================================================================
// xbool MEMORY_UNIT_Initialize( char* MemoryUnitResourceFilename );
// 
// Description: Used to setup the detailed information about the games that can be saved for this system on this console.
//              The resource file is built using the MemCardWiz tool.  Basically, this resource file contains information
//              on how many different game types are saved by this application, as well, as details on the icon displayed
//              in the browser screens, and all other required information for saving games.
//
//              This must be called after the MEMORY_UNIT_Create if called.  Also, no other memory unit functions will work
//              if this step is skipped.//  
// 
//=====================================================================================================================================
xbool MEMORY_UNIT_Initialize( char* MemoryUnitResourceFilename );


//=====================================================================================================================================
// void MEMORY_UNIT_DeInitialize( void );
// 
// Description: This function is used to release all resources used by the memory card system asside from the actual
//              memory unit manager.  This will release any saveed game info, any icon data or any other data that
//              is not required when you know you will not be using the memory card features.
//
//              To restart the memory card system, a call to MEMORY_UNIT_Initialize(...) must be called.
// 
//=====================================================================================================================================
void MEMORY_UNIT_DeInitialize( void );


//=====================================================================================================================================
// void  MEMORY_UNIT_Destroy( void );
// 
// Description: Release all system resources for the Memory Unit Interface.  This function should only be called during the 
//              destruction of the application.
//=====================================================================================================================================
void  MEMORY_UNIT_Destroy( void );


//=====================================================================================================================================
// void  MEMORY_UNIT_Update( void );
//
// Description: The update function must be called each frame during the ENG_PageFlip function in order to allow the particular
//              console the timeslice it needs to update it's own memory card system libraries.
//=====================================================================================================================================
void  MEMORY_UNIT_Update( void );


//=====================================================================================================================================
// void  MEMORY_UNIT_EnableScanning( void );
//
// Description: Basically the final switch in the MEMORY UNIT interface.  When this is called, the system will scan and update
//              all memory unit functionality.  Connection status, and other actions will only take place when enabled.
//=====================================================================================================================================
void  MEMORY_UNIT_EnableScanning( void );


//=====================================================================================================================================
// void  MEMORY_UNIT_DisableScanning( void );
//
// Description: Basically disables the active memory unit scanning.  The system is not terminated, but just sleeping.
//=====================================================================================================================================
void  MEMORY_UNIT_DisableScanning( void );

//=====================================================================================================================================
// xbool MEMORY_UNIT_IsWriteInProgress( void  );
//
// Description: Function to test if any of the memory units are currently performing a write operation.
//
// Inputs:      NONE.
//
// Outputs:     TRUE  - if a memory card write operation is in progress.
//              FALSE - if not.
//=====================================================================================================================================
xbool MEMORY_UNIT_IsWriteInProgress( void );


//=====================================================================================================================================
// xbool MEMORY_UNIT_IsConnected( s32 MemoryUnitID = -1 );         // If you don't provide a paramenter, the Active Memory Unit is tested.
//
// Description: Function to test if a particular memory unit is connected to the console.
//
// Inputs:      s32 MemoryUnitID - A Quagmire memory unit ID ranging from QUAG_MEMORY_UNIT_ID_0 to QUAG_MEMORY_UNIT_MAX_ID
//                                 where QUAG_MEMORY_UNIT_MAX_ID is console dependant.
//
// Outputs:     TRUE - if that memory unit is connected.
//              FALSE - if that memory unit is not connected.
//=====================================================================================================================================
xbool MEMORY_UNIT_IsConnected( s32 MemoryUnitID = -1 );         // If you don't provide a paramenter, the Active Memory Unit is tested.


//=====================================================================================================================================
// s32 MEMORY_UNIT_GetSaveGameUserDataSizeInBytes( s32 SaveGameType );
//
// Description: See how many bytes of user data are expected to be saved.  You can use this function to make sure that
//              the number of bytes to be saved by the game match the number of bytes that are going to be saved.
//
// Inputs:  s32   SaveGameType    - One of the valid types identified within the resource file.
//
// Returns: s32   The number of bytes used by that type of save.
//                -1 is returned if the provided save type is not valid.  
//
//
//=====================================================================================================================================
s32 MEMORY_UNIT_GetSaveGameUserDataSizeInBytes( s32 SaveGameType );


//=====================================================================================================================================
// How to use the memory unit system
//=====================================================================================================================================
//
// Basically, you can effectively only perform memory unit requests on one memory unit at a time.
// The basic outline is as follows.
// 
// 1. Select the memory card that you wish to use.
// 2. Request memory card action.
// 3. Wait for command action to complete.
// 4. Release the card so another may be selected later.
//
// That is the basic setup, but there are a few more details you need know in order to use the MEMORY UNIT system correctly.
//
// - All MEMORY_UNIT functions will return an immediate value to identify an instant error condition, or if the
//   request to continue this command was allowed.  
//
// - If an error occurred, you must handle it immediately.  
//
// - If your request was granted, then you must poll the memory unit system with the MEMORY_UNIT_GetNextEvent function
//   to find determine the current status of the command that you requested.
//   If the function returns TRUE, it means that a Event occurred.  You must then look at the MemoryUnitEvent structure
//   to determine if the command was successfull, or if it failed.  
// 
//   MemoryUnitEvent::mEventType == QUAG_MU_EVENT_COMPLETE the command was successful.
//   MemoryUnitEvent::mEventType == QUAG_MU_EVENT_FAILED   the command failed.
// 
//   The other members of the MemoryUnitEvent structure are filled with the ReturnValue and the Error code provided by
//   the specific MEMORY_UNIT command.  Each command will return a different set of Return Values and Error Codes, so you must
//   handle each one independantly.
//
//
// If you follow these simple guidelines, you should have simple memory unit operation.  Just remember to select the 
// memory unit you wish to work with, use it, then always release it.
//
//=====================================================================================================================================


//=====================================================================================================================================
// s32 MEMORY_UNIT_SelectActiveMemoryUnit( s32 MemoryUnitID, byte* pFileIOBuffer, s32 BufferSize );
//
// Description: Used to activate, or highlight one particular card so all of the other MEMORY_UNIT functions know which
//              card or Hard Disk to use.
//
// Inputs:      s32 MemoryUnitID - A Quagmire memory unit ID ranging from QUAG_MEMORY_UNIT_ID_0 to QUAG_MEMORY_UNIT_MAX_ID
//                                 where QUAG_MEMORY_UNIT_MAX_ID is console dependant.
//                
//              byte* pFileIOBuffer - On the Gamecube it is required that when you insert a card, you inspect
//                                    to make sure that your files are valid.  Because of this you must
//                                    perform some loading of files when the card is mounted.  Therefore
//                                    you must provide a data buffer that is large enough to hold the size
//                                    of the largest savegame for this application.
//                  
//              s32  BufferSize - The size of this user buffer.
//                                    
//
// Immediate Returns:   QUAG_MU_EVERYTHING_IS_OK
//                      QUAG_MU_NOT_FORMATTED
//                      QUAG_MU_TRYING_TO_ACTIVATE_MORE_THAN_ONE_CARD
//                      QUAG_MU_TRYING_TO_ACTIVATE_ILLEGAL_CARD
//              
// Values returned if Memory Unit Event is QUAG_MU_EVENT_COMPLETE
//      mReturnValue - TRUE
//      mErrorCode   - QUAG_MU_EVERYTHING_IS_OK
//
// Values returned if Memory Unit Event is QUAG_MU_EVENT_FAILED
//      mReturnValue - FALSE
//      mErrorCode   - QUAG_MU_NO_CARD
//                   - QUAG_MU_WRONG_DEVICE
//                   - QUAG_MU_NOT_FORMATTED
//                   - QUAG_MU_INVALID_CARD
//                   - QUAG_MU_INVALID_FORMAT
//                   - QUAG_MU_FILE_SYSTEM_DAMAGED
//                   - QUAG_MU_CARD_PHYSICALLY_DAMAGED
//                   - QUAG_MU_UNKNOWN_PROBLEM
//
//=====================================================================================================================================
s32 MEMORY_UNIT_SelectActiveMemoryUnit( s32 MemoryUnitID, byte* pFileIOBuffer, s32 BufferSize );


//=====================================================================================================================================
// s32 MEMORY_UNIT_ReleaseActiveMemoryUnit( void );
//
// Description: Used to release the selected memory card previously highlighted using the MEMORY_UNIT_SelectActiveMemoryUnit 
//              function.
//
// Inputs:      None.
//
// Immediate Returns:   QUAG_MU_EVERYTHING_IS_OK
//                      QUAG_MU_NO_ACTIVE_CARD_SELECTED
//              
// *** NOTE *** This function is immediate, and does not require to be polled to get the final results.
//
//=====================================================================================================================================
s32 MEMORY_UNIT_ReleaseActiveMemoryUnit( void );


//=====================================================================================================================================
// char* MEMORY_UNIT_GetNameOfActiveMemoryUnit( void );
//
// Description: Used to get the name assigned to the selected memory unit.  On the PS2 and Dolphin, this name is to be generated
//              based on it's position in the port or slot. On XBOX, this value is the personalized name provided by 
//              the user.
//
// Inputs:      None.
//
// Immediate Returns:   The name of the memory card, if there is one.
//                      NULL, is returned otherwise.
//              
// *** NOTE *** This function is immediate, and does not require to be polled to get the final results.
//
//=====================================================================================================================================
char* MEMORY_UNIT_GetNameOfActiveMemoryUnit( void );


//=====================================================================================================================================
// s32 MEMORY_UNIT_GetFreeBlocks( void );
//
// Description: Used to determine the number of blocks remaining on the selected memory unit.
//
// Inputs:      None.
//
// Immediate Returns:   s32 - The number of clusters remaining on the highlighted card.
//                      
// *** NOTE *** This function is immediate, and does not require to be polled to get the final results.
//
//=====================================================================================================================================
s32   MEMORY_UNIT_GetFreeBlocks( void );



//=====================================================================================================================================
// s32 MEMORY_UNIT_GetFreeFiles( void );
//
// Description: Used to determine the number of free file entries remaining on the selected memory unit.
//
// Inputs:      None.
//
// Immediate Returns:   s32 - The number of file slots remaining on the highlighted card.
//                      
// *** NOTE *** This function is immediate, and does not require to be polled to get the final results.
//
//=====================================================================================================================================
s32   MEMORY_UNIT_GetFreeFiles( void );


//=====================================================================================================================================
// xbool MEMORY_UNIT_GetRequiredBlocksFreeToSaveGameType( s32 SaveGameType, s32* pRequiredBlocks, xbool bWorstCase = TRUE );
//
// Description: Used to determine how many blocks, or clusters, or sectors (System dependant terminology) is required to save
//              one of the Save Game Types (identified within the Memory Unit resource file).
//
// Inputs:      s32   SaveGameType    - One of the valid types identified within the resource file.
//              s32*  pRequiredBlocks - Location to store the results.
//              xbool bWorstCase      - Since the PS2 can actually require less space than a file save should thanks
//                                      to their cluster system, this value was provided to get the worst case if no
//                                      free file clusters were present.
//
// Immediate Returns:   QUAG_MU_EVERYTHING_IS_OK        if the number of free clusters exceeds the size of the save game type.
//                      QUAG_MU_NOT_ENOUGH_FREE_SPACE   if the SaveGameType size requires more space than the card has free.
//                      QUAG_MU_NO_ACTIVE_CARD_SELECTED if there was no selected memory unit.
//
// *** NOTE *** This function is immediate, and does not require to be polled to get the final results.
//
//=====================================================================================================================================
s32 MEMORY_UNIT_GetRequiredBlocksFreeToSaveGameType( s32 SaveGameType, s32* pRequiredBlocks, xbool bWorstCase = TRUE );




//=====================================================================================================================================
// s32 MEMORY_UNIT_Format( void );
//
// Description: Used to format the selected memory unit.
//              
// Inputs: None.
//
// Immediate Returns:   QUAG_MU_EVERYTHING_IS_OK
//                      QUAG_MU_SYSTEM_BUSY
//                      QUAG_MU_NO_ACTIVE_CARD_SELECTED
//                      
//              
// Values returned if Memory Unit Event is QUAG_MU_EVENT_COMPLETE
//      mReturnValue - TRUE
//      mErrorCode   - QUAG_MU_EVERYTHING_IS_OK
//
// Values returned if Memory Unit Event is QUAG_MU_EVENT_FAILED
//      mReturnValue - FALSE
//      mErrorCode   - QUAG_MU_NO_CARD
//                   - QUAG_MU_NOT_FORMATTED
//                   - QUAG_MU_ALREADY_FORMATTED
//                   - QUAG_MU_UNKNOWN_PROBLEM
//
//=====================================================================================================================================
s32   MEMORY_UNIT_Format( void );


//=====================================================================================================================================
// s32 MEMORY_UNIT_SaveGame( s32 SaveGameType, s32 TypeOffset, void* pUserData, char* SaveGameBrowserInfo, s32 BrowserInfoBreakOffset );
//
// Description: Used to save a game to the memory unit.
//              
// Inputs:              s32 SaveGameType - This is one of the valid save game types identified in the MemoryUnit Resource File.
//                      
//                      s32 TypeOffset   - Index into this save game type.  Each type can have between 
//                                         1 and MEMORY_UNIT_MAX_SAVE_COUNT_PER_TYPE copies of a particular game type.
//                      
//                      void* pUserData  - Pointer to the user data to be saved.
//
//                      char* SaveGameBrowserInfo - Title that is to be displayed on the consoles browser to describe the game
//                                                  that was saved. (Limit to 16 characters per line.)
//                      
//                      s32   BrowserInfoBreakOffset - Index into the Browser info where a Carriage return would occur to split
//                                                     the browser info text across 2 lines.
//
// Immediate Returns:   QUAG_MU_EVERYTHING_IS_OK
//                      QUAG_MU_SYSTEM_BUSY
//                      QUAG_MU_NO_ACTIVE_CARD_SELECTED
//                      
//              
// Values returned if Memory Unit Event is QUAG_MU_EVENT_COMPLETE
//      mReturnValue - TRUE
//      mErrorCode   - QUAG_MU_EVERYTHING_IS_OK
//
// Values returned if Memory Unit Event is QUAG_MU_EVENT_FAILED
//      mReturnValue - FALSE
//      mErrorCode   - QUAG_MU_NO_CARD
//                   - QUAG_MU_FILE_ALREADY_EXISTS
//                   - QUAG_MU_NOT_ENOUGH_FREE_SPACE
//                   - QUAG_MU_FILE_ACCESS_DENIED
//                   - QUAG_MU_GENERIC_SAVE_FAILED
//                   - QUAG_MU_UNKNOWN_PROBLEM
//
//=====================================================================================================================================
s32   MEMORY_UNIT_SaveGame( s32 SaveGameType, s32 TypeOffset, void* pUserData, char* SaveGameBrowserInfo, s32 BrowserInfoBreakOffset );



//=====================================================================================================================================
// s32 MEMORY_UNIT_LoadGame( s32 SaveGameType, s32 TypeOffset, void* pUserData );
//
// Description: Used to load a game to the memory unit.
//              
// Inputs:              s32 SaveGameType - This is one of the valid save game types identified in the MemoryUnit Resource File.
//                      
//                      s32 TypeOffset   - Index into this save game type.  Each type can have between 
//                                         1 and MEMORY_UNIT_MAX_SAVE_COUNT_PER_TYPE copies of a particular game type.
//                      
//                      void* pUserData  - Pointer to a buffer to store the user data.
//
// Immediate Returns:   QUAG_MU_EVERYTHING_IS_OK
//                      QUAG_MU_SYSTEM_BUSY
//                      QUAG_MU_NO_ACTIVE_CARD_SELECTED
//                      
//              
// Values returned if Memory Unit Event is QUAG_MU_EVENT_COMPLETE
//      mReturnValue - TRUE
//      mErrorCode   - QUAG_MU_EVERYTHING_IS_OK
//
// Values returned if Memory Unit Event is QUAG_MU_EVENT_FAILED
//      mReturnValue - FALSE
//      mErrorCode   - QUAG_MU_NO_CARD
//                   - QUAG_MU_FILE_NOT_FOUND
//                   - QUAG_MU_GENERIC_LOAD_FAILED
//                   - QUAG_MU_UNKNOWN_PROBLEM
//
//=====================================================================================================================================
s32   MEMORY_UNIT_LoadGame( s32 SaveGameType, s32 TypeOffset, void* pUserData );


//=====================================================================================================================================
// s32 MEMORY_UNIT_DeleteGame( s32 SaveGameType, s32 TypeOffset );
//
// Description: Used to delete a game from the memory unit.
//              
// Inputs:              s32 SaveGameType - This is one of the valid save game types identified in the MemoryUnit Resource File.
//                      
//                      s32 TypeOffset   - Index into this save game type.  Each type can have between 
//                                         1 and MEMORY_UNIT_MAX_SAVE_COUNT_PER_TYPE copies of a particular game type.
//                      
// Immediate Returns:   QUAG_MU_EVERYTHING_IS_OK
//                      QUAG_MU_SYSTEM_BUSY
//                      QUAG_MU_NO_ACTIVE_CARD_SELECTED
//                      
//              
// Values returned if Memory Unit Event is QUAG_MU_EVENT_COMPLETE
//      mReturnValue - TRUE
//      mErrorCode   - QUAG_MU_EVERYTHING_IS_OK
//
// Values returned if Memory Unit Event is QUAG_MU_EVENT_FAILED
//      mReturnValue - FALSE
//      mErrorCode   - QUAG_MU_NO_CARD
//                   - QUAG_MU_FILE_NOT_FOUND
//                   - QUAG_MU_GENERIC_DELETE_FAILED
//                   - QUAG_MU_UNKNOWN_PROBLEM
//
//=====================================================================================================================================
s32   MEMORY_UNIT_DeleteGame( s32 SaveGameType, s32 TypeOffset );


//=====================================================================================================================================
// s32   MEMORY_UNIT_GetSaveGameList( s32 SaveGameType, s32* pMaxSaveGameTypeCount, SingleSavedGameInfo** pSaveGameListings );
//
// Description: Used to get the list of the names of available saves for a particular save game type.
//              
// Inputs:              s32 SaveGameType - This is one of the valid save game types identified in the MemoryUnit Resource File.
// 
//                      s32* pMaxSaveGameTypeCount - Location to store the maximum number of SaveGames of this type that can
//                                                   be stored by this application.
//
//                      SingleSavedGameInfo** pSaveGameListings - The address of a pointer to store the location of the
//                                                                list of saved games of this type found on the selected
//                                                                memory unit.
//
// Immediate Returns:   s32 - The number of saves on the memory unit that are of the requested SaveGameType.
//
// *** NOTE *** This function is immediate, and does not require to be polled to get the final results.
//
//=====================================================================================================================================
s32   MEMORY_UNIT_GetSaveGameList( s32 SaveGameType, s32* pMaxSaveGameTypeCount, SingleSavedGameInfo** pSaveGameListings );


//=====================================================================================================================================
// xbool MEMORY_UNIT_GetNextEvent( MemoryUnitEvent* pEvent );
//
// Description: Polling function used to determine if a previously called MEMORY_UNIT function has been completed yet.
//
// Inputs:      MemoryUnitEvent* pEvent - Address of a MemoryUnitEvent structure to be filled.
// 
// Outputs:     FALSE - if the previously requested command is not finished yet.
//              TRUE  - If an the command is finished.
// 
// *** NOTE *** when TRUE is returned, the contents of the MemoryUnitEvent are filled to reflect the return values and
//              error codes of the command function.
//=====================================================================================================================================
xbool MEMORY_UNIT_GetNextEvent( MemoryUnitEvent* pEvent );


//=====================================================================================================================================
// xbool MEMORY_UNIT_GetGenericMemoryCardName( s32 QuagMemCardID, char* pBuffer, s32 SizeOfBuffer );
//
// Description: Function that fills the pBuffer with the generic name for a particular memory card.
//              Example. "memory card (PS2) in MEMORY CARD slot 1" for PS2.
//
// Inputs:      s32   QuagMemCardID        - Valid member of QuagMemoryUnitIDs
//              char* pBuffer              - Buffer to store the name of the memory card.
//              s32   SizeOfBuffer         - The Size of the buffer we are storing the name into.
// 
// Outputs:     FALSE - The name could not fit into the provided buffer.
//              TRUE  - It's all good, the name was provided..
// 
//=====================================================================================================================================
xbool MEMORY_UNIT_GetGenericMemoryCardName( s32 QuagMemCardID, char* pBuffer, s32 SizeOfBuffer );


//=====================================================================================================================================


//=====================================================================================================================================
// Special includes files
// These platform specific includes are at the end of the file to remove compliler problems with undeclared references
// to MemoryUnit class for the specific consoles.
//=====================================================================================================================================
#if   defined( TARGET_PS2     )
    #include "PS2_MemoryUnit.hpp"
#elif defined( TARGET_DOLPHIN )
    #include "GC_MemoryUnit.hpp"
#elif defined( TARGET_XBOX    )
    #include "XBOX_MemoryUnit.hpp"
#elif defined( TARGET_PC      )
    #include "PC_MemoryUnit.hpp"
#endif

#endif // Q_MEMORY_UNIT_HPP