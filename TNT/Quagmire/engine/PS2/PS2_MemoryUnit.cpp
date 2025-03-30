//=====================================================================================================================================
// Includes
//=====================================================================================================================================
#include <libmtap.h>
#include <libmc.h>
#include "PS2_MemoryUnit.hpp"


//=====================================================================================================================================
// Forward Declarations
//=====================================================================================================================================


//=====================================================================================================================================
// Definitions
//=====================================================================================================================================
#ifdef insert_username_here
#define SHOW_PS2_DIAGNOSTICS
#endif

#define PS2MU_SYSTEM_REQUEST_GRANTED(SystemValue)   ( (SystemValue == 0)  ? (TRUE) : (FALSE) )
#define PS2MU_SYSTEM_REQUEST_DENIED(SystemValue)    ( (SystemValue <= -1) ? (TRUE) : (FALSE) )

#define PS2MU_SYSTEM_RETURNED_NO_CARD(SystemValue)  ( SystemValue <= -10 )

#define PS2MU_FUNC_ASYNC_MODE               1
#define PS2MU_FUNC_SYNC_MODE                0
#define PS2MU_CLEAR_FILE_HANDLE_VALUE      -1

#define PS2MU_MIN_SAVED_GAME_TIME   2.0f
#define PS2MU_MIN_DELETE_TIME       2.0f
#define PS2MU_MIN_LOAD_GAME_TIME    1.5f


#define ADJUST_PS2_FREE_SPACE( ActualFree )       (MAX( ActualFree - 2, 0 ))

//-------------------------------------------------------------------------------------------------------------------------------------
enum PS2_LocalMemoryCardCommands
{
    PS2MU_CMD_FIRST = 0,
    PS2MU_CMD_NONE = PS2MU_CMD_FIRST,
    PS2MU_CMD_MOUNT,
    PS2MU_CMD_FORMAT,
    PS2MU_CMD_SAVE,
    PS2MU_CMD_LOAD,
    PS2MU_CMD_DELETE,
    PS2MU_CMD_LAST = PS2MU_CMD_DELETE,
};


//=====================================================================================================================================
// Globals
//=====================================================================================================================================
PS2MemoryUnitManager* gpPS2MemoryUnitManager;


//=====================================================================================================================================
// Local Tool Functions
//=====================================================================================================================================
#define MEMORY_CARD_DEBUG_LEVEL     2

#ifdef SHOW_PS2_DIAGNOSTICS
    void MC_PRINT( u8 Level, char* FormatString, ... )
    {
        static char TempBuffer[300];

        if( Level <= MEMORY_CARD_DEBUG_LEVEL )
        {
            x_va_list   Args;
            x_va_start( Args, FormatString );

            x_vsprintf( TempBuffer, FormatString, Args );

            x_printf( "[PS2MU] --> " );
            x_printf( TempBuffer );
            x_printf( "\n" );
        }
    }
#else
    void MC_PRINT( u8 Level, char* FormatString, ... )
    {
    }
#endif


//-------------------------------------------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The title of the game that is saved with the icon file must be in a Japanese format Shift JIS, or SJIS.
// The following tables, and functions to translate ascii to SJIS were provided by
// Sony DevNet by David Coombes
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------------------------------------------------------------
u16 gsSpecialAsciiToSJISCharactersTable[PS2MU_SPECIAL_ASCII_SJIS_TABLE_SIZE][2] =
{
	{0x8140, 32},		/*   */
	{0x8149, 33},		/* ! */
	{0x8168, 34},		/* " */
	{0x8194, 35},		/* # */
	{0x8190, 36},		/* $ */
	{0x8193, 37},		/* % */
	{0x8195, 38},		/* & */
	{0x8166, 39},		/* ' */
	{0x8169, 40},		/* ( */
	{0x816a, 41},		/* ) */
	{0x8196, 42},		/* * */
	{0x817b, 43},		/* + */
	{0x8143, 44},		/* , */
	{0x817c, 45},		/* - */
	{0x8144, 46},		/* . */
	{0x815e, 47},		/* / */
	{0x8146, 58},		/* : */
	{0x8147, 59},		/* ; */
	{0x8171, 60},		/* < */
	{0x8181, 61},		/* = */
	{0x8172, 62},		/* > */
	{0x8148, 63},		/* ? */
	{0x8197, 64},		/* @ */
	{0x816d, 91},		/* [ */
	{0x818f, 92},		/* \ */
	{0x816e, 93},		/* ] */
	{0x814f, 94},		/* ^ */
	{0x8151, 95},		/* _ */
	{0x8165, 96},		/* ` */
	{0x816f, 123},		/* { */
	{0x8162, 124},		/* | */
	{0x8170, 125},		/* } */
	{0x8150, 126},		/* ~ */
};


//-------------------------------------------------------------------------------------------------------------------------------------
u16 gsRegularAsciiToSJISCharactersTable[PS2MU_REGULAR_ASCII_SJIS_TABLE_SIZE][2] =
{
	{0x824f, 0x30},	/* 0-9  */
	{0x8260, 0x41},	/* A-Z  */
	{0x8281, 0x61},	/* a-z  */
};

//-------------------------------------------------------------------------------------------------------------------------------------
u16 AsciiCharToSJISChar( u8 AsciiChar )
{
	u16 sjis_code = 0;
	u8  stmp      = 0;
	u8  stmp2     = 0;

	if((AsciiChar >= 0x20) && (AsciiChar <= 0x2f))
		stmp2 = 1;
	
	else if((AsciiChar >= 0x30) && (AsciiChar <= 0x39))
		stmp = 0;
	
	else if((AsciiChar >= 0x3a) && (AsciiChar <= 0x40))
		stmp2 = 11;
	
	else if((AsciiChar >= 0x41) && (AsciiChar <= 0x5a))
		stmp = 1;
	
	else if((AsciiChar >= 0x5b) && (AsciiChar <= 0x60))
		stmp2 = 37;
	
	else if((AsciiChar >= 0x61) && (AsciiChar <= 0x7a))
		stmp = 2;
	
	else if((AsciiChar >= 0x7b) && (AsciiChar <= 0x7e))
		stmp2 = 63;
	
	else 
	{
		x_printf("bad ASCII code 0x%x\n", AsciiChar);
		return 0;
	}

	if (stmp2)
	   	sjis_code = gsSpecialAsciiToSJISCharactersTable[AsciiChar - 0x20 - (stmp2 - 1)][0];
	else
		sjis_code = gsRegularAsciiToSJISCharactersTable[stmp][0] + AsciiChar - gsRegularAsciiToSJISCharactersTable[stmp][1];

	return sjis_code;
}

//-------------------------------------------------------------------------------------------------------------------------------------
void AsciiStringToSJISString( const char* input, u16* output)
{
	int i=0;
	int len;
	u16 sjis;
	u8  temp1, temp2;

	len = x_strlen(input);
	
	for(i = 0; i < len; i++)
	{
		sjis = AsciiCharToSJISChar((u8)(input[i]));
		temp1 = (sjis & 0xFF);  
		temp2 = (sjis & 0xFF00) >> 8;
		output[i] = temp2 | (temp1 << 8);
   	}
	
	output[i] = 0x0000;
}


//=====================================================================================================================================
// MEMORY_UNIT Interface functions.
//=====================================================================================================================================
xbool MEMORY_UNIT_Create( char* /*ApplicationName*/ )
{
    // Application Name is ignored.

    // Create the manager.
    gpPS2MemoryUnitManager = new PS2MemoryUnitManager;

    if( gpPS2MemoryUnitManager )
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
    if( gpPS2MemoryUnitManager->Initialize( MemoryUnitResourceFilename ) )
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
    if( gpPS2MemoryUnitManager )
    {
        gpPS2MemoryUnitManager->DeInitialize( );
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
void MEMORY_UNIT_Destroy( void )
{
    if( gpPS2MemoryUnitManager )
    {
        delete gpPS2MemoryUnitManager;
        gpPS2MemoryUnitManager = NULL;
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
void MEMORY_UNIT_Update( void )
{
    // As long as there is a memory unit created, and scanning is enabled update the stats of the 
    // memory units.
    if( gpPS2MemoryUnitManager && gpPS2MemoryUnitManager->IsScanningEnabled( ) )
    {
        gpPS2MemoryUnitManager->Update( );
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
void MEMORY_UNIT_EnableScanning( void )
{
    gpPS2MemoryUnitManager->EnableScanning( );
}


//-------------------------------------------------------------------------------------------------------------------------------------
void MEMORY_UNIT_DisableScanning( void )
{
    gpPS2MemoryUnitManager->DisableScanning( );
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool MEMORY_UNIT_IsWriteInProgress( void )
{
    return gpPS2MemoryUnitManager->IsWriteInProgress( );
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool MEMORY_UNIT_IsConnected( s32 MemoryUnitID )
{
    if( MemoryUnitID == -1 )
        return gpPS2MemoryUnitManager->IsActiveMemoryUnitStillConnected( );
    else
        return gpPS2MemoryUnitManager->IsMemoryUnitConnected( MemoryUnitID );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_GetSaveGameUserDataSizeInBytes( s32 SaveGameType )
{
    return gpPS2MemoryUnitManager->GetUserDataByteSize( SaveGameType );
}

//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_SelectActiveMemoryUnit( s32 MemoryUnitID, byte* /*pFileIOBuffer*/, s32 /*BufferSize*/ )
{
    return gpPS2MemoryUnitManager->SelectActiveMemoryUnit( MemoryUnitID );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_ReleaseActiveMemoryUnit( void )
{    
    return gpPS2MemoryUnitManager->ReleaseActiveMemoryUnit( );
}


//-------------------------------------------------------------------------------------------------------------------------------------
char* MEMORY_UNIT_GetNameOfActiveMemoryUnit( void )
{
    return gpPS2MemoryUnitManager->GetNameOfActiveMemoryUnit( );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_GetFreeBlocks( void )
{
    s32 AdjustedFreeBlocks;
    AdjustedFreeBlocks = ADJUST_PS2_FREE_SPACE( gpPS2MemoryUnitManager->GetFreeSpaceRemainingOnActiveUnit( ) );

    return AdjustedFreeBlocks;
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_GetFreeFiles( void )
{
    // PS2 is not concerned with this in our implementation.  We will never have enough files within a directory to even care.
    return 1;
}

//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_GetRequiredBlocksFreeToSaveGameType( s32 SaveGameType, s32* pRequiredBlocks, xbool bWorstCase )
{
    ASSERT( pRequiredBlocks );
    return gpPS2MemoryUnitManager->CalculateRequiredSizeForSaveGameType( SaveGameType, pRequiredBlocks, bWorstCase );
}

//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_Format( void )
{
    return gpPS2MemoryUnitManager->FormatCard( );
}

//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_SaveGame( s32 SaveGameType, s32 TypeOffset, void* pUserData, char* SaveGameBrowserInfo, s32 BrowserInfoBreakOffset )
{
    return gpPS2MemoryUnitManager->SaveGame( SaveGameType, TypeOffset, pUserData, SaveGameBrowserInfo, BrowserInfoBreakOffset );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_LoadGame( s32 SaveGameType, s32 TypeOffset, void* pUserData )
{   
    return gpPS2MemoryUnitManager->LoadGame( SaveGameType, TypeOffset, pUserData );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_DeleteGame( s32 SaveGameType, s32 TypeOffset )
{   
    return gpPS2MemoryUnitManager->DeleteGame( SaveGameType, TypeOffset );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 MEMORY_UNIT_GetSaveGameList( s32 SaveGameType, s32* pMaxSaveGameTypeCount, SingleSavedGameInfo** pSaveGameListings )
{   
    return gpPS2MemoryUnitManager->GetSaveGameList( SaveGameType, pMaxSaveGameTypeCount, pSaveGameListings );
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool MEMORY_UNIT_GetNextEvent( MemoryUnitEvent* pEvent )
{   
    ASSERT( pEvent );

    if( gpPS2MemoryUnitManager->IsCommandComplete( ) )
    {
        // Do a cheap wrap here.  If the card isn't connected, always return the NO card error.
        if( MEMORY_UNIT_IsConnected( ) )
        {
            gpPS2MemoryUnitManager->GetLastCommandValues( &pEvent->mReturnValue, &pEvent->mErrorCode );

            if( pEvent->mErrorCode == QUAG_MU_EVERYTHING_IS_OK )
                pEvent->mEventType = QUAG_MU_EVENT_COMPLETE;
            else
                pEvent->mEventType = QUAG_MU_EVENT_FAILED;
        }
        else
        {
            gpPS2MemoryUnitManager->GetLastCommandValues( NULL, NULL );
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

    return gpPS2MemoryUnitManager->GenerateCardName( QuagMemCardID, pBuffer, SizeOfBuffer );
}

//=====================================================================================================================================
// PS2ConsoleIconData
//=====================================================================================================================================
PS2ConsoleIconData::PS2ConsoleIconData( void )
{
    mpIconData      = NULL;
    mIconDataSize   = 0;
}


//-------------------------------------------------------------------------------------------------------------------------------------
PS2ConsoleIconData::~PS2ConsoleIconData( void )
{
    ReleaseData( );
}


//-------------------------------------------------------------------------------------------------------------------------------------
void PS2ConsoleIconData::ReleaseData( void )
{
    if( mpIconData ) 
    { 
        delete[] mpIconData; 
        mpIconData = NULL; 
        mIconDataSize = 0;
    }
}


//=====================================================================================================================================
// PS2MemoryUnit
//=====================================================================================================================================
PS2MemoryUnit::PS2MemoryUnit( void )
{
    mUnitID         = -1;

    mbConnected     = FALSE;
    mbFormatted     = FALSE;
    mType           = PS2MU_NOCARD;
    mFreeClusters   = 0;
}


//-------------------------------------------------------------------------------------------------------------------------------------
void PS2MemoryUnit::Initialize( s32 UnitID )
{
    mUnitID = UnitID;
    Reset(  );
}


//-------------------------------------------------------------------------------------------------------------------------------------
void PS2MemoryUnit::Reset( void )
{
    mbConnected     = FALSE;
    mbFormatted     = FALSE;
    mType           = PS2MU_NOCARD;
    mFreeClusters   = 0;

    mName[0]        = 0;
}


//-------------------------------------------------------------------------------------------------------------------------------------
void PS2MemoryUnit::SetName( char* NewPersonalizedName )
{
    x_strncpy( mName, NewPersonalizedName, MEMORY_UNIT_PERSONALIZED_NAME_SIZE );
}


//=====================================================================================================================================
// PS2MemoryUnitManager
//=====================================================================================================================================
s32 PS2MemoryUnitManager::msBaseDirNameFileData = 0xAABBCCDD;
s32 PS2MemoryUnitManager::msBaseDirNameFileSize = 4;

//-------------------------------------------------------------------------------------------------------------------------------------
// Construction / Initialization
//-------------------------------------------------------------------------------------------------------------------------------------
PS2MemoryUnitManager::PS2MemoryUnitManager( void )
{
    u32             MemoryUnitOffset;
    PS2MemoryUnit*  pCurrentMemUnit;

    MC_PRINT( 1, "Constructing" );

    // Initialize the PS2 Memory card containers.
    for( MemoryUnitOffset = QUAG_MEMORY_UNIT_ID_0; MemoryUnitOffset <= QUAG_MEMORY_UNIT_MAX_ID; MemoryUnitOffset++ )
    {
        pCurrentMemUnit = &mPS2MemoryUnits[MemoryUnitOffset];
        pCurrentMemUnit->Initialize( MemoryUnitOffset );
    }
    pCurrentMemUnit = NULL;

    //  Identify that this game is to use the multitaps for memory cards as well.
    //  Passing 2 to sceMtapPortOpen represents that there should be memory card access provided on Multitap in port 0
    //  Passing 3 to sceMtapPortOpen represents that there should be memory card access provided on Multitap in port 1
    //
    if(1 != sceMtapPortOpen(2))
    {
        MC_PRINT( 1, "Error opening port 0 for multi-tap access" );
        ASSERT( 0 );
    }
    if(1 != sceMtapPortOpen(3))
    {
        MC_PRINT( 1, "Error opening port 1 for multi-tap access" );
        ASSERT( 0 );
    }

    // Identify that this memory manager does not know how to save games yet.
    mNumberOfSaveGameTypes = 0;
    mpSaveGameTypes = NULL;
    mpAvailableSavedGames = NULL;

    // Initialize the icon data block.
    InitializeAllDefaultIconData( );

    // Have scanning off by default.
    DisableScanning( );

    // Technically the memory unit manager is built, but not initialized yet to actually do anything.
    mInitialized = FALSE;
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool PS2MemoryUnitManager::Initialize( char* MemoryUnitResourceFilename )
{
    xbool                           bSuccess;
    PS2MemCardResourceHeader        Header;
    X_FILE*                         pFile;
    byte*                           pTempBuffer;
    s32                             ReadCount;

    PS2MemCardResourceIconInfo*     pResourceIconBuffer;
    PS2MemCardResourceSaveGameType* pResourceSaveGameTypeBuffer;
    byte*                           pRawIconData;

    pTempBuffer                 = NULL;
    pResourceIconBuffer         = NULL;
    pResourceSaveGameTypeBuffer = NULL;
    pRawIconData                = NULL;
    bSuccess = FALSE;

    if( !mInitialized )
    {
        //-------------------------------------------------------------------------------------------------------------------------------------
        MC_PRINT( 1, "Initializing" );

        ResetCommandHandlingVariables( );

        // Get all of the information required to save a game on this system by loading in the instructional resource
        // file.  This file will contain all information about how saved games must be built on this platform, as well
        // the data for an icon, and any information regarding how to draw that icon is also provided.
        pFile = x_fopen( MemoryUnitResourceFilename, "rb" );

        if( pFile )
        {
            ReadCount = x_fread( &Header, sizeof( Header ), 1, pFile );

            if( ReadCount == 1 )
            {
                // Test to make sure that it's a PS2 resource file.
                if( Header.mResourceType == QUAG_MEMORY_UNIT_RESOURCE_TYPE_PS2 )
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
                    
                        mpSaveGameTypes = new PS2MemCardResourceSaveGameType[mNumberOfSaveGameTypes];
                        ASSERT( mpSaveGameTypes );

                        // Read in data for the save game types.
                        pResourceSaveGameTypeBuffer = (PS2MemCardResourceSaveGameType*)(pTempBuffer + Header.mOffsetToSaveTypes);
                        x_memcpy( mpSaveGameTypes, pResourceSaveGameTypeBuffer, sizeof( PS2MemCardResourceSaveGameType ) * mNumberOfSaveGameTypes );

                        // Also initialize the Available saved games list
                        mpAvailableSavedGames = new SaveGameTypeAvailableList[mNumberOfSaveGameTypes];
                        ASSERT( mpAvailableSavedGames );
                        ClearAvailableGamesList( );


                        // Retrieve the icon information for this game from the resource file.
                        //-------------------------------------------------------------------------------------------------------------------------------------
                        pResourceIconBuffer = (PS2MemCardResourceIconInfo*)(pTempBuffer + Header.mOffsetToIconInfo);
                        pRawIconData        = pTempBuffer + Header.mOffsetToIconData;
                        BuildIconFromResourceData( pResourceIconBuffer, pRawIconData );

                        bSuccess = TRUE;
                    }
                    else
                    {
                        MC_PRINT( 1, "Couldn't read entire resource file" );
                    }

                }
                else
                {
                    MC_PRINT( 1, "Resource File is not for PS2" );
                }
            }
            else
            {
                MC_PRINT( 1, "Couldn't read resource file header" );
            }
        }
        else
        {
            MC_PRINT( 1, "Couldn't open the resource file %s", MemoryUnitResourceFilename );
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
            MC_PRINT( 1, "Initialization Successful" );
            mInitialized = TRUE;
        }
        else
        {
            MC_PRINT( 1, "Initialization Failed" );
        }
    }
    else
    {
        //-------------------------------------------------------------------------------------------------------------------------------------
        MC_PRINT( 1, "Already Initialized, Ignoring Initialize request." );
        bSuccess = TRUE;
    }

    return bSuccess;
}

//-------------------------------------------------------------------------------------------------------------------------------------
void PS2MemoryUnitManager::DeInitialize( void )
{
    if( mInitialized )
    {
        // When we deinitialize, there had better not be a memory unit command in progress.
        ASSERT( !IsCommandInProgress( ) );

        MC_PRINT( 1, "De-Initializing" );

        // Make sure that a background card check was not in progress.
        while( mbBackgrounCardCheckInProgress ) 
        {
            ContinueBackgroundCardCheck( );
        }

        // Turn scanning off by default.
        DisableScanning( );

        // Delete the save games types list.
        if( mpSaveGameTypes )
        {
            delete[] mpSaveGameTypes;
            mpSaveGameTypes = NULL;

            mNumberOfSaveGameTypes = 0;
        }

        // Delete the menu of saved games.
        if( mpAvailableSavedGames )
        {
            delete[] mpAvailableSavedGames;
            mpAvailableSavedGames = NULL;
        }

        // Release the icon data that was loaded when the PS2 Memory manager was initialized.
        mIconData.ReleaseData( );

        // Identify that the memory manager is not ready for use anymore.
        mInitialized = FALSE;
    }
    else
    {
        MC_PRINT( 1, "System wasn't intialized, de-intialize request ignored." );
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
PS2MemoryUnitManager::~PS2MemoryUnitManager( void )
{
    DeInitialize( );

    MC_PRINT( 1, "Destroying" );
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 PS2MemoryUnitManager::SelectActiveMemoryUnit( s32 MemoryUnitID )
{
    ASSERT( mInitialized );

    // Make sure that the card they wish to activate is even there.
    if( mPS2MemoryUnits[MemoryUnitID].mbConnected )
    {
        // First Make sure that no command is active.
        if( IsCommandInProgress( ) == FALSE )
        {
            // Make sure that the memory unit ID is valid
            if( (MemoryUnitID >= QUAG_MEMORY_UNIT_ID_0) && (MemoryUnitID <= QUAG_MEMORY_UNIT_MAX_ID) )
            {
                // If there is no active unit, make the requested one the active unit.
                if( mActiveMemoryUnit == NULL )
                {
                    s32 CommandRequestReturnValue;

                    mActiveMemoryUnit = &mPS2MemoryUnits[MemoryUnitID];
                    GetPS2PortAndSlotFromMemoryUnitID( MemoryUnitID, &mActiveMemoryUnitPort, &mActiveMemoryUnitSlot );

                    // Request for the next command.
                    CommandRequestReturnValue = RequestNewCommand( PS2MU_CMD_MOUNT );

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
                else if( mActiveMemoryUnit != &mPS2MemoryUnits[MemoryUnitID] )
                {
                    return QUAG_MU_TRYING_TO_ACTIVATE_MORE_THAN_ONE_CARD;
                }
                // if this is the active unit, and it's not formatted, then return that it's not formatted.
                else if( mActiveMemoryUnit->mbFormatted == FALSE )
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
s32 PS2MemoryUnitManager::ReleaseActiveMemoryUnit( void )
{
    ASSERT( mInitialized );

    if( mActiveMemoryUnit != NULL )
    {
        mActiveMemoryUnit = NULL;
        mActiveMemoryUnitPort = -1;
        mActiveMemoryUnitSlot = -1;
        mActiveMemoryUnitReusableEnvironmentSpace = 0;

        // Clear the available game list.
        ClearAvailableGamesList( );

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
            if( mActiveMemoryUnit == mPS2MemoryUnits )
            {
                bFoundMemoryUnit = TRUE;
                break;
            }
        }

        if( bFoundMemoryUnit == FALSE )
        {
            MC_PRINT( 1, "Trying to release an active card when there isn't one." );
        }
#endif
        return QUAG_MU_NO_ACTIVE_CARD_SELECTED;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 PS2MemoryUnitManager::CalculateRequiredSizeForSaveGameType( s32 SaveGameType, s32* pRequiredBlocks, xbool bWorstCase )
{
    ASSERT( mInitialized );

    //=====================================================================================================================================
    // How to calculate the size of a saved file on the PS2.
    //
    // Taken from PlayStation®2 EE Library Overview Release 2.3.4T
    //=====================================================================================================================================
    //
    // The space needed to create a file can be calculated as follows.
    //
    // A. The number of clusters used by individual files
    //-------------------------------------------------------------------------------------------------------------------------------------
    // The following formula is used to determine the cluster count by rounding the size of the file to be
    // created to the nearest 1024 bytes (Nearest Block Size).
    //
    // number of clusters = ( number of bytes + 1023) / 1024 (remainder is truncated)
    //
    // B - Number of clusters used in file entry
    //-------------------------------------------------------------------------------------------------------------------------------------
    // The following formula is used to determine the number of clusters used in a file entry.
    //
    // number of clusters = ( number of files to create + 1 ) / 2 (remainder is truncated)
    //
    // C - Number of clusters used in directory entry
    //-------------------------------------------------------------------------------------------------------------------------------------
    // 2 clusters are used for the directory entry when a new directory is created.
    //
    //-------------------------------------------------------------------------------------------------------------------------------------
    // 
    // The space needed to create files is determined by 
    // 1. Calculate (A) for each file and add them together.
    // 2. Add B - Clusters used by file entries.
    // 3. Add C - Clusters used by directory entries.
    //
    // The file size displayed in the PlayStation 2 memory card browser screen is determined by totaling the sizes of 
    // the files themselves and then rounding up to the highest cluster. Thus, the value will be smaller than the 
    // required number of clusters.
    //
    // Depending on how the memory card is used, a previously deleted file entry area can be reused. In such cases, 
    // the required number of clusters calculated above will not all be used. The amount of reusable space
    // can be determined using sceMcGetEntSpace().
    //
    // The file sizes displayed on the Japanese PlayStation 2 browser window are simply the total of the sizes of
    // the files themselves, so the sizes will be smaller than the number of clusters actually being used. The
    // overseas PlayStation 2 browser screens, however, display the actual cluster counts.

    s32 SizeOfIconInfoFile;
    s32 SizeOfIconDataFile;
    s32 SizeOfBrowseTextFile;
    s32 SizeOfBaseDirNameFile;
    s32 SizeOfUserDataFile;

    s32 ClustersUsedByIndividualFiles;
    s32 NumberOfFilesInASave;
    s32 ClustersUsedByFileEntries;
    s32 ReusableEnvironmentSpace;
    s32 ClustersUsedByDirectoryEntries;
    s32 TotalClusters;

    ASSERT( SaveGameType >= 0 && SaveGameType < mNumberOfSaveGameTypes );

    // Right off the bat make sure that there is an active card selected, if there's not, there are certain
    // pieces of information that will not be available. so bail right away.
    if( bWorstCase == FALSE )
    {
        if( mActiveMemoryUnit == NULL )
        {
            *pRequiredBlocks = 0;
            return QUAG_MU_NO_ACTIVE_CARD_SELECTED;
        }
    }


    // (STEP A) --- Determine the sizes of each file that is to be saved to the card.
    //-------------------------------------------------------------------------------------------------------------------------------------
    SizeOfIconInfoFile      = sizeof( mIconData.mSonyMcIconSys );
    SizeOfIconDataFile      = mIconData.mIconDataSize;
    SizeOfBrowseTextFile    = MEMORY_UNIT_SAVE_GAME_TITLE_LENGTH;
    SizeOfBaseDirNameFile   = PS2MemoryUnitManager::msBaseDirNameFileSize;   
    SizeOfUserDataFile      = mpSaveGameTypes[SaveGameType].mUserDataFileInfo.mSizeOfUserDataFile;

    // Now round all of these block sizes up to the next block or cluster.
    SizeOfIconInfoFile      = ((SizeOfIconInfoFile    + (PS2MU_SIZE_OF_BLOCK - 1)) / PS2MU_SIZE_OF_BLOCK);
    SizeOfIconDataFile      = ((SizeOfIconDataFile    + (PS2MU_SIZE_OF_BLOCK - 1)) / PS2MU_SIZE_OF_BLOCK);
    SizeOfBrowseTextFile    = ((SizeOfBrowseTextFile  + (PS2MU_SIZE_OF_BLOCK - 1)) / PS2MU_SIZE_OF_BLOCK);
    SizeOfBaseDirNameFile   = ((SizeOfBaseDirNameFile + (PS2MU_SIZE_OF_BLOCK - 1)) / PS2MU_SIZE_OF_BLOCK);
    SizeOfUserDataFile      = ((SizeOfUserDataFile    + (PS2MU_SIZE_OF_BLOCK - 1)) / PS2MU_SIZE_OF_BLOCK);


    ClustersUsedByIndividualFiles = SizeOfIconInfoFile    +
                                    SizeOfIconDataFile    +
                                    SizeOfBrowseTextFile  +
                                    SizeOfBaseDirNameFile +
                                    SizeOfUserDataFile;

    // (STEP B) --- Determine the memory card space wasted by overhead for the file entries.
    //-------------------------------------------------------------------------------------------------------------------------------------
    // There are 5 new files that are going to be created when making the save game.
    NumberOfFilesInASave = 5;

    // First, Find how many file entries can be created free due to the fact that it takes 2 files to make up one
    // block of memory card space, and there could be some partial blocks open.
    if( bWorstCase == TRUE )
        ReusableEnvironmentSpace = 0;
    else
        ReusableEnvironmentSpace = mActiveMemoryUnitReusableEnvironmentSpace;

    // Using these 2 values, we can determine how many clusters will actually be used by file entries for this save.
    // Here is the formula to determine the true file entry cluster use.
    // (FilesToBeCreated - FreeFile Entries) / 2 rounded up to the nearest integer.
    //
    // Using the limitations of integer math, and truncating the fraction, 
    // ((FilesToBeCreated - FreeFile Entries) + 1) / 2 will yield the same results.
    //
    // *** NOTE *** There are times when you don't want to rely on these free cluster spaces, and you wish to 
    //              find out the worst case.  If that's what  you want, don't factor in the ReusableEnvironmentSpace.
    if( bWorstCase == TRUE )
        ClustersUsedByFileEntries = (NumberOfFilesInASave + 1) / 2;
    else
        ClustersUsedByFileEntries = ((NumberOfFilesInASave - ReusableEnvironmentSpace) + 1) / 2;

    // Well, it now appears that it could be possible to have a negative number of clusters used by file entries
    // after you factor in the ReusableEnvironmentSpace value.  So to be safe, make sure that ClustersUsedByFileEntries
    // is always 0 or higher.
    if( ClustersUsedByFileEntries < 0 )
        ClustersUsedByFileEntries = 0;


    // (STEP C) --- Determine the memory card space used by a directory entry.
    //-------------------------------------------------------------------------------------------------------------------------------------
    // We have only 1 directory per save
    ClustersUsedByDirectoryEntries = 2 * 1;  // 2 blocks per directory * Num of Directories.


    // The total required.
    //-------------------------------------------------------------------------------------------------------------------------------------
    TotalClusters = ClustersUsedByIndividualFiles + ClustersUsedByFileEntries + ClustersUsedByDirectoryEntries;


    // Provide this information to calling function.
    *pRequiredBlocks = TotalClusters;

    // Now return if this is enough or not.
    if( mActiveMemoryUnit != NULL )
    {
        if( TotalClusters <= (s32)mActiveMemoryUnit->mFreeClusters )
        {
            return QUAG_MU_EVERYTHING_IS_OK;
        }
    }

    return QUAG_MU_NOT_ENOUGH_FREE_SPACE;
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 PS2MemoryUnitManager::GetUserDataByteSize( s32 SaveGameType )
{
    ASSERT( mInitialized );
    ASSERT( SaveGameType >= 0 && SaveGameType < mNumberOfSaveGameTypes );
    
    return mpSaveGameTypes[SaveGameType].mUserDataFileInfo.mSizeOfUserDataFile;
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 PS2MemoryUnitManager::FormatCard( void )
{
    ASSERT( mInitialized );
    s32 CommandRequestReturnValue;

    CommandRequestReturnValue = RequestNewCommand( PS2MU_CMD_FORMAT );

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
s32 PS2MemoryUnitManager::SaveGame( s32 SaveGameType, s32 TypeOffset, void* pUserData, char* SaveGameBrowserInfo, s32 BrowserInfoBreakOffset )
{
    ASSERT( mInitialized );

    s32 CommandRequestReturnValue;

    CommandRequestReturnValue = RequestNewCommand( PS2MU_CMD_SAVE );

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
        mpUserBuffer = pUserData;

        // Set up the browser text.
        SetBrowserText( SaveGameBrowserInfo, BrowserInfoBreakOffset );
        
        // Well it's time that we start.
        return QUAG_MU_EVERYTHING_IS_OK;
    }
    else
    {
        return CommandRequestReturnValue;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 PS2MemoryUnitManager::LoadGame( s32 SaveGameType, s32 TypeOffset, void* pUserData )
{
    ASSERT( mInitialized );

    s32 CommandRequestReturnValue;

    CommandRequestReturnValue = RequestNewCommand( PS2MU_CMD_LOAD );

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
        mpUserBuffer = pUserData;

        // Well it's time that we start.
        return QUAG_MU_EVERYTHING_IS_OK;
    }
    else
    {
        return CommandRequestReturnValue;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 PS2MemoryUnitManager::DeleteGame( s32 SaveGameType, s32 TypeOffset )
{
    ASSERT( mInitialized );

    s32 CommandRequestReturnValue;

    CommandRequestReturnValue = RequestNewCommand( PS2MU_CMD_DELETE );

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
s32 PS2MemoryUnitManager::GetSaveGameList( s32 SaveGameType, s32* pMaxSaveGameTypeCount, SingleSavedGameInfo** pSaveGameListings )
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
xbool PS2MemoryUnitManager::IsCommandComplete( void )
{
    ASSERT( mInitialized );

    return ( mbCommandCompleted &&                               // If the command is complete 
             mbWaitingForCommandCompleteAcknowledged == TRUE &&  // and the
             (mCurrentCommand != PS2MU_CMD_NONE)
           );
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool PS2MemoryUnitManager::GetLastCommandValues( s32* pReturnValue, s32* pErrorCode )
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
        mCurrentCommand                         = PS2MU_CMD_NONE;

        // The command was completed as complete by the user call.
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool PS2MemoryUnitManager::IsWriteInProgress( void )
{
    return mCurrentCommand == PS2MU_CMD_SAVE;
}


//-------------------------------------------------------------------------------------------------------------------------------------
void PS2MemoryUnitManager::SetBrowserText( char* pBrowserInfo, s32 BreakCharacter )
{
    ASSERT( mInitialized );
    ASSERT( x_strlen( pBrowserInfo ) < PS2MU_BROWSER_STRING_LENGTH );

    x_strncpy( mBrowserTitleText, pBrowserInfo, PS2MU_BROWSER_STRING_LENGTH );
    mBrowserTitleBreak = BreakCharacter * 2;    // The reason that it's times 2 is that when the game 
                                                // is finally saved, each character is in SJIS, which are 2 byte characters.
}

//-------------------------------------------------------------------------------------------------------------------------------------
xbool PS2MemoryUnitManager::IsMemoryUnitConnected( s32 MemoryCardID )
{
    ASSERT( mInitialized );
    ASSERT( MemoryCardID >= QUAG_MEMORY_UNIT_ID_0 && MemoryCardID <= QUAG_MEMORY_UNIT_MAX_ID );

    return mPS2MemoryUnits[MemoryCardID].mbConnected;
}

//-------------------------------------------------------------------------------------------------------------------------------------
xbool PS2MemoryUnitManager::IsMemoryUnitFormatted( s32 MemoryCardID )
{
    ASSERT( mInitialized );
    ASSERT( MemoryCardID >= QUAG_MEMORY_UNIT_ID_0 && MemoryCardID <= QUAG_MEMORY_UNIT_MAX_ID );

	return mPS2MemoryUnits[MemoryCardID].mbFormatted;
}

//-------------------------------------------------------------------------------------------------------------------------------------
xbool PS2MemoryUnitManager::IsActiveMemoryUnitStillConnected( void )
{
    ASSERT( mInitialized );

    if( mActiveMemoryUnit )
        return mActiveMemoryUnit->mbConnected;
    else
        return FALSE;
}

//-------------------------------------------------------------------------------------------------------------------------------------
char* PS2MemoryUnitManager::GetNameOfActiveMemoryUnit( void )
{
    ASSERT( mInitialized );
    if( mActiveMemoryUnit )
    {
        return mActiveMemoryUnit->mName;
    }
    else
    {
        return NULL;
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
s32 PS2MemoryUnitManager::GetFreeSpaceRemainingOnActiveUnit( void )
{
    ASSERT( mInitialized );
    if( mActiveMemoryUnit )
    {
        return mActiveMemoryUnit->mFreeClusters;
    }
    else
    {
        return NULL;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
void PS2MemoryUnitManager::Update( void )
{
    ASSERT( mInitialized );
    static s32      sMemoryUnitOffset = QUAG_MEMORY_UNIT_ID_0;
    s32             Port, Slot;
    s32             CardIndex;
    s32             MaxCardIndex;
    static xbool    bMultitapPortConnectionLastFrame[2] = { FALSE, FALSE };
    xbool           bIsMultiTapConnected[2];
    xbool           bIgnore;
    PS2MemoryUnit* pCurrentMemUnit;

    // If backgroun port checking is allowed, then continue to scan the ports.
    if( mbAllowBackgroundCardChecking )
    {
        // If there isn't a backgroun port check in progress, start one.
        if( mbBackgrounCardCheckInProgress == FALSE )
        {
            // Test to see if there is a multitap plugged in if the slot is greater than the first one in each port.
            bIsMultiTapConnected[0] = sceMtapGetConnection( 0 ) == 1 ? TRUE : FALSE;
            bIsMultiTapConnected[1] = sceMtapGetConnection( 1 ) == 1 ? TRUE : FALSE;

            // Test to see if the status of the multitap in the first port has changed.
            // If if has then reset the condition of all the cards in that port.
            if( bMultitapPortConnectionLastFrame[0] != bIsMultiTapConnected[0] )
            {
                MC_PRINT( 1, "Multitap Port 0 was %s", (bIsMultiTapConnected[0] == TRUE) ? "Connected" : "Removed" );

                MaxCardIndex = MIN( QUAG_MEMORY_UNIT_MAX_ID, QUAG_MEMORY_UNIT_ID_3 );
                for( CardIndex = QUAG_MEMORY_UNIT_ID_0; CardIndex <= MaxCardIndex; CardIndex++ )
                {
                    if( mPS2MemoryUnits[CardIndex].mbConnected )
                    {
                        ProcessCardDisconnected( &mPS2MemoryUnits[CardIndex] );
                    }
                }
            }

            // Repeat for port 2.
            if( bMultitapPortConnectionLastFrame[1] != bIsMultiTapConnected[1] )
            {
                MC_PRINT( 1, "Multitap Port 1 was %s", (bIsMultiTapConnected[1] == TRUE) ? "Connected" : "Removed" );

                MaxCardIndex = MIN( QUAG_MEMORY_UNIT_MAX_ID, QUAG_MEMORY_UNIT_ID_7 );
                for( CardIndex = QUAG_MEMORY_UNIT_ID_4; CardIndex <= MaxCardIndex; CardIndex++ )
                {
                    if( mPS2MemoryUnits[CardIndex].mbConnected )
                    {
                        ProcessCardDisconnected( &mPS2MemoryUnits[CardIndex] );
                    }
                }
            }

            // Record the current status of the multitap connection.
            bMultitapPortConnectionLastFrame[0] = bIsMultiTapConnected[0];
            bMultitapPortConnectionLastFrame[1] = bIsMultiTapConnected[1];

            // Go through each port and slot to get the status.
            while( sMemoryUnitOffset <= QUAG_MEMORY_UNIT_MAX_ID )
            {
                bIgnore = FALSE;

                // Get access to the memory unit.
                pCurrentMemUnit = &mPS2MemoryUnits[sMemoryUnitOffset];

                // Get the corresponding Quag memory card id from the port and slot.
                GetPS2PortAndSlotFromMemoryUnitID( sMemoryUnitOffset, &Port, &Slot );

                // If the next port to check is greater than the first, then we are in multi tap country,
                // make sure that it's connected.
                if( Slot > 0 )
                {
                    // If the port that you are looking to does not have the multitap connected, then there is a problem.
                    if( bIsMultiTapConnected[Port] == FALSE )
                    {
                        // ignore this port and slot if the multitap isn't there.
                        bIgnore = TRUE;

                        // If this card thought that it was connected, and now it's not, then this should be updated.
                        if( pCurrentMemUnit->mbConnected )
                        {
                            ProcessCardDisconnected( pCurrentMemUnit );
                        }
                    }
                }

                if( bIgnore == FALSE )
                {
                    // Start a check of this card.
                    if( StartBackgroundCardCheck( sMemoryUnitOffset ) )
                    {
                        // Don't remove this, if this isn't incremented here as well as in the else only 
                        // the first memory card will ever be scanned.
                        sMemoryUnitOffset++;
                        break;
                    }
                }

                sMemoryUnitOffset++;
            }

            // Reset this offset so the next time this function is called it starts over again.
            if( sMemoryUnitOffset > QUAG_MEMORY_UNIT_MAX_ID )
                sMemoryUnitOffset = QUAG_MEMORY_UNIT_ID_0;
        }
        else
        {
            ContinueBackgroundCardCheck( );
        }
    }
    else
    {
        // If there are no commands in progress, then just allow the system to keep scanning ports.
        if( IsCommandInProgress( ) == FALSE )
        {
            mbAllowBackgroundCardChecking = TRUE;
        }
        else
        {
            // Is the command completed, and just waiting for the user to reply?
            if( mbWaitingForCommandCompleteAcknowledged == FALSE )
            {
                if( mpCurrentCommandHandler )
                {
                    (this->*mpCurrentCommandHandler)();
                }
                else
                {
                    MC_PRINT( 1, "No command handler set, but system is performing a command." );
                    ASSERT( 0 );
                }
            }
        }
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
void PS2MemoryUnitManager::CompleteCommandAndWaitForUser( s32 ReturnValue, s32 ErrorCode )
{
    ASSERT( mInitialized );

    // Do a special back up catch all here left over files open here.
    if( IsFileHandleInUse( ) )
    {
        // Put the assert here to catch this while debugging.
        // We should never return a true value and leave a file handle open.
        if( ReturnValue == TRUE )
            ASSERT( 0 );

        // Clear the file handle in release mode.
        ClearCurrentFileHandle( );
    }

    MC_PRINT( 1, "%s Complete, ReturnValue:%d, ErrorCode:%d", GetCommandNameAsString( mCurrentCommand ), ReturnValue, ErrorCode );

    mbCommandCompleted                      = TRUE;
    mbWaitingForCommandCompleteAcknowledged = TRUE;
    mCommandReturnValue                     = ReturnValue;
    mCommandError                           = ErrorCode;
}

//-------------------------------------------------------------------------------------------------------------------------------------
xbool PS2MemoryUnitManager::StartBackgroundCardCheck( s32 QuagMemoryID )
{
    ASSERT( mInitialized );

    s32 SystemReturnValue;
    s32 Port;
    s32 Slot;
    PS2MemoryUnit* pBackgroundCard;

    mCurrentBackgroundCardBeingChecked = QuagMemoryID;
    pBackgroundCard = &mPS2MemoryUnits[mCurrentBackgroundCardBeingChecked];

    // Get the slot and port for this memory card.
    GetPS2PortAndSlotFromMemoryUnitID( pBackgroundCard->mUnitID, &Port, &Slot );

    SystemReturnValue = sceMcGetInfo( Port, Slot, &pBackgroundCard->mType, (s32*)&pBackgroundCard->mFreeClusters, &pBackgroundCard->mbFormatted );

    if( PS2MU_SYSTEM_REQUEST_GRANTED( SystemReturnValue ) )
    {
        // If the card is valid, then get more info out of it.
        mbBackgrounCardCheckInProgress = TRUE;
        return TRUE;
    }
    else if( PS2MU_SYSTEM_REQUEST_DENIED( SystemReturnValue )  )
    {
        MC_PRINT( 1, "Error Obtaining Info for card %s", pBackgroundCard->mName );
        CompleteBackgroundCardCheck( );
        return FALSE;
    }
    else
    {
        // Just shouldn't be here.
        ASSERT( 0 );
        return FALSE;
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
void PS2MemoryUnitManager::ContinueBackgroundCardCheck( void )
{
    ASSERT( mInitialized );

    s32 SystemReturnValue;
    s32 FunctionResult;

    PS2MemoryUnit* pBackgroundCard;
    pBackgroundCard = &mPS2MemoryUnits[mCurrentBackgroundCardBeingChecked];
    
    SystemReturnValue = sceMcSync( PS2MU_FUNC_ASYNC_MODE, NULL, &FunctionResult );
    ASSERT( SystemReturnValue != sceMcExecIdle );

    // The function is complete.
    if( SystemReturnValue == sceMcExecFinish )
    {
        // It's the same card as last time this function was called.  All is good.
        if( FunctionResult == sceMcResSucceed )
        {
            // Test to see if the card thinks that it was disconnected.  If it did, it was most likely due to the fact
            // that the ProcessCardDisconnected was called when a multitap was inserted.  When a multitap is inserted
            // the memory cards are forced to be disconnected, on the software side because when they are 
            // re-connected, thier names are properly re-created.  If the sceMcGetInfo function returns a success, it means
            // that the physical card hasn't changed since the last time it was called for this card.  Though we may have
            // thought that it did.  This little test makes sure all is correct.
            if( pBackgroundCard->mbConnected == FALSE )
            {
                ProcessCardConnected( pBackgroundCard );
            }

            CompleteBackgroundCardCheck( );
        }
        else if( FunctionResult == sceMcResChangedCard || 
                 FunctionResult == sceMcResNoFormat
               )
        {
            ProcessCardConnected( pBackgroundCard );
            CompleteBackgroundCardCheck( );
        }
        else
        {
            // If it thought that it was connected, and now it isn't, then rip it out.
            if( pBackgroundCard->mbConnected )
                ProcessCardDisconnected( pBackgroundCard );

            CompleteBackgroundCardCheck( );
        }
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
void PS2MemoryUnitManager::CompleteBackgroundCardCheck( void )
{
    ASSERT( mInitialized );

    mbBackgrounCardCheckInProgress = FALSE;
    if( IsCommandInProgress( ) )
    {
        mbAllowBackgroundCardChecking = FALSE;
    }
}





//-------------------------------------------------------------------------------------------------------------------------------------
s32 PS2MemoryUnitManager::RequestNewCommand( s32 NewCommand )
{
    ASSERT( mInitialized );
    // Make sure the command is valid.
    ASSERT( NewCommand >= PS2MU_CMD_FIRST && NewCommand <= PS2MU_CMD_LAST );

    // Make sure that an active card is already selected.
    if( mActiveMemoryUnit )
    {
        // Is there a command in progress?
        if( IsCommandInProgress( ) == FALSE )
        {
            mCurrentCommand                         = NewCommand;
            mCommandState                           = 0;
            mbCommandCompleted                      = FALSE;
            mbWaitingForCommandCompleteAcknowledged = FALSE;
            SetRequestedCommandHandler( NewCommand );

            MC_PRINT( 1, "Requests %s. Request GRANTED", GetCommandNameAsString( NewCommand ) );
            return QUAG_MU_EVERYTHING_IS_OK;
        }
        else
        {
            MC_PRINT( 1, "Requests %s. Request FAILED", GetCommandNameAsString( NewCommand ) );
            return QUAG_MU_SYSTEM_BUSY;
        }
    }
    else
    {
        MC_PRINT( 1, "Requests %s, but there was no active memory card selected.  Request FAILED", GetCommandNameAsString( NewCommand ) );
        return QUAG_MU_NO_ACTIVE_CARD_SELECTED;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool PS2MemoryUnitManager::IsCommandInProgress( void )
{
    return mCurrentCommand != PS2MU_CMD_NONE;
}


//-------------------------------------------------------------------------------------------------------------------------------------
void PS2MemoryUnitManager::SetRequestedCommandHandler( s32 NewCommand )
{
    ASSERT( mInitialized );

    // reset this timer for each command.
    mCommandStartTime = x_GetTime( );

    switch( NewCommand )
    {
        case PS2MU_CMD_MOUNT:   mpCurrentCommandHandler = &PS2MemoryUnitManager::MountHandler;      break;
        case PS2MU_CMD_FORMAT:  mpCurrentCommandHandler = &PS2MemoryUnitManager::FormatHandler;     break;
        case PS2MU_CMD_SAVE:    mpCurrentCommandHandler = &PS2MemoryUnitManager::SaveGameHandler;   break;
        case PS2MU_CMD_LOAD:    mpCurrentCommandHandler = &PS2MemoryUnitManager::LoadGameHandler;   break;
        case PS2MU_CMD_DELETE:  mpCurrentCommandHandler = &PS2MemoryUnitManager::DeleteGameHandler; break;

        default:
            ASSERT( 0 );
            mpCurrentCommandHandler = NULL;
            break;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool PS2MemoryUnitManager::ReEvaluateCardStatus( void )
{
    ASSERT( mInitialized );

    s32 SystemReturnValue;
    s32 FunctionResult;

    enum ReEvaluateStates
    {
        RE_EVAL_STATE_INIT,
        RE_EVAL_STATE_WAIT_FOR_GET_INFO_TO_COMPLETE,
        RE_EVAL_STATE_TEST_FOR_REUSABLE_CLUSTERS,
        RE_EVAL_STATE_WAIT_FOR_REUSABLE_CLUSTERS_UPDATE,
        RE_EVAL_STATE_COMPLETED,
    };

    switch( mCommandSubState )
    {
        case RE_EVAL_STATE_INIT:
        {
            SystemReturnValue = sceMcGetInfo( mActiveMemoryUnitPort, mActiveMemoryUnitSlot, &mActiveMemoryUnit->mType, (s32*)&mActiveMemoryUnit->mFreeClusters, &mActiveMemoryUnit->mbFormatted );

            if( PS2MU_SYSTEM_REQUEST_GRANTED( SystemReturnValue ) )
            {
                mCommandSubState = RE_EVAL_STATE_WAIT_FOR_GET_INFO_TO_COMPLETE;
            }        
            else if( PS2MU_SYSTEM_REQUEST_DENIED( SystemReturnValue ) )
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_UNKNOWN_PROBLEM );
            }

            break;
        }

        case RE_EVAL_STATE_WAIT_FOR_GET_INFO_TO_COMPLETE:
        {
            SystemReturnValue = sceMcSync( PS2MU_FUNC_ASYNC_MODE, NULL, &FunctionResult );
            ASSERT( SystemReturnValue != sceMcExecIdle );

            // The function is complete.
            if( SystemReturnValue == sceMcExecFinish )
            {
                // It's the same card as last time this function was called.  All is good.
                if( FunctionResult == sceMcResSucceed )
                {
                    mCommandSubState = RE_EVAL_STATE_TEST_FOR_REUSABLE_CLUSTERS;
                }
                else if( PS2MU_SYSTEM_RETURNED_NO_CARD( FunctionResult ) )
                {
                    ProcessCardDisconnected( mActiveMemoryUnit );
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
                }
            }
            
            break;
        }

        case RE_EVAL_STATE_TEST_FOR_REUSABLE_CLUSTERS:
        {
            // Ask the system to find it.
            SystemReturnValue = sceMcGetEntSpace( mActiveMemoryUnitPort, mActiveMemoryUnitSlot, "/" );

            // If the Request to change to this base directory succeeded, then continue to wait for the result.
            if( PS2MU_SYSTEM_REQUEST_GRANTED( SystemReturnValue ) )
            {
                mCommandSubState = RE_EVAL_STATE_WAIT_FOR_REUSABLE_CLUSTERS_UPDATE;
            }
            // It failed, find out why.
            else
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_UNKNOWN_PROBLEM );
            }

            break;
        }

        case RE_EVAL_STATE_WAIT_FOR_REUSABLE_CLUSTERS_UPDATE:
        {
            SystemReturnValue = sceMcSync( PS2MU_FUNC_ASYNC_MODE, NULL, &FunctionResult );
            ASSERT( SystemReturnValue != sceMcExecIdle );

            // The function is complete.
            if( SystemReturnValue == sceMcExecFinish )
            {
                // It's the same card as last time this function was called.  All is good.
                if( FunctionResult >= 0 )
                {
                    mActiveMemoryUnitReusableEnvironmentSpace = FunctionResult;
                    mCommandSubState = RE_EVAL_STATE_COMPLETED;
                }
                else if( PS2MU_SYSTEM_RETURNED_NO_CARD( FunctionResult ) )
                {
                    ProcessCardDisconnected( mActiveMemoryUnit );
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
                }
                else
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_UNKNOWN_PROBLEM );
                }
            }
            
            break;
        }

        case RE_EVAL_STATE_COMPLETED:
        {
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
void PS2MemoryUnitManager::MountHandler( void )
{
    ASSERT( mInitialized );

    enum MountStates
    {
        MOUNT_STATE_TEST_SIMPLE_CARD_INFO,
        MOUNT_STATE_GET_CARD_SAVED_GAMES,
        MOUNT_STATE_RE_EVALUATE_CARD,
        MOUNT_STATE_COMPLETE,
    };
    // On the PS2, as soon as the card is connected, you know the following information about it.
    // 1 - Type of card
    // 2 - Free Clusters (Or Blocks in our case)
    // 3 - If it's formatted.
    //
    // Using this information, we can start the mount procedure.
    //
    // A mount procedure will do the following.
    // 1 - Verify the card is valid and readable.
    // 2 - The base directories for each allowable save game of each save game 'type' are scanned for.
    //     
    //      If the base directory is found, the system then proceeds to open the PS2_STORED_BROWSE_TEXT_FILE file.
    //      {
    //          If the file is present, then the system tries to read out the name of the saved game.
    //          {
    //          }
    //      }
    //
    //      If the read worked, or not, move onto the next save game of this type.
    switch( mCommandState )
    {
        case MOUNT_STATE_TEST_SIMPLE_CARD_INFO:
        {
            // Clear out the Current Available Games
            ClearAvailableGamesList( );

            // Now start testing the cards limits.
            if( IsActiveMemoryUnitStillConnected( ) )
            {
                // If the card is a valid 8MB SONY PS2 card, then all is well.
                if( mActiveMemoryUnit->mType == PS2MU_8MBCARD )
                {
                    if( mActiveMemoryUnit->mbFormatted )
                    {
                        mCommandState    = MOUNT_STATE_GET_CARD_SAVED_GAMES;
                        mCommandSubState = 0;
                        mCurrentGameType = 0;
                        mCurrentGameTypeIndex = 0;
                    }
                    else
                    {
                        CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NOT_FORMATTED );
                    }
                }
                // If this card is a pocket station, pretend that it doesn't exist.
                else if( mActiveMemoryUnit->mType == PS2MU_POCKETSTATION )
                {
                    // If this card is a pocket station, make sure to ignore it.
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
                }
                // Otherwise, there is an invalid card here.
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

        case MOUNT_STATE_GET_CARD_SAVED_GAMES:
        {
            if( MountHandlerGetSavedGames( ) )
            {
                mCommandState    = MOUNT_STATE_RE_EVALUATE_CARD;
                mCommandSubState = 0;
            }
            break;
        }

        case MOUNT_STATE_RE_EVALUATE_CARD:
        {
            if( ReEvaluateCardStatus( ) )
            {
                mCommandState = MOUNT_STATE_COMPLETE;
            }
            
            break;
        }
        
        case MOUNT_STATE_COMPLETE:
        {
            CompleteCommandAndWaitForUser( TRUE, QUAG_MU_EVERYTHING_IS_OK );
            break;
        }

        default:
            ASSERT( 0 );
            break;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool PS2MemoryUnitManager::MountHandlerGetSavedGames( void )
{
    ASSERT( mInitialized );

    s32     SystemReturnValue;
    s32     FunctionResult;
    char*   BasePathString;
    SingleSavedGameInfo* pSavedGame;

    enum ScanningForSavedGames
    {
        GET_SAVED_GAME_LOOK_FOR_BASE_DIRECTORY,
        GET_SAVED_GAME_WAIT_FOR_BASE_DIRECTORY_CHECK_TO_COMPLETE,
        GET_SAVED_GAME_GO_TO_NEXT_SAVE_GAME,

        GET_SAVED_GAME_FOUND_BASE_DIR_NOW_FIND_BROWSE_INFO_FILE,
        GET_SAVED_GAME_WAITING_FOR_BROWSE_INFO_FILE_TO_OPEN,
        GET_SAVED_GAME_WAITING_FOR_BROWSE_INFO_FILE_TO_BE_READ,
        GET_SAVED_GAME_WAITING_FOR_BROWSE_INFO_FILE_TO_CLOSE,

        GET_SAVED_GAME_COMPLETE,
    };

    // Go one at a time through the base directories that we are using to see if they exist.
    switch( mCommandSubState )
    {
        case GET_SAVED_GAME_LOOK_FOR_BASE_DIRECTORY:
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

            // Get the name of the base directory to look for.
            BasePathString = mpSaveGameTypes[mCurrentGameType].mBaseDirectories[mCurrentGameTypeIndex].mActualBaseDirName;

            // Ask the system to find it.
            SystemReturnValue = sceMcChdir( mActiveMemoryUnitPort, mActiveMemoryUnitSlot, BasePathString, 0 );

            // If the Request to change to this base directory succeeded, then continue to wait for the result.
            if( PS2MU_SYSTEM_REQUEST_GRANTED( SystemReturnValue ) )
            {
                mCommandSubState = GET_SAVED_GAME_WAIT_FOR_BASE_DIRECTORY_CHECK_TO_COMPLETE;
            }
            // It failed, find out why.
            else
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_UNKNOWN_PROBLEM );
            }

            break;
        }

        // Waiting to see if the base directory exists.
        case GET_SAVED_GAME_WAIT_FOR_BASE_DIRECTORY_CHECK_TO_COMPLETE:
        {
            SystemReturnValue = sceMcSync( PS2MU_FUNC_ASYNC_MODE, NULL, &FunctionResult );

            // If for whatever reason there is no function to sync up with, it means that the card was pulled.
            if( SystemReturnValue == sceMcExecIdle )
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
            }
            // The function is complete.
            else if( SystemReturnValue == sceMcExecFinish )
            {
                // The base directory was found, time to move onward and find the rest of the required info.
                if( FunctionResult == sceMcResSucceed )
                {
                    // Well, if the base directory exists, it means that a save was started
                    // and as far as we are concerned, the save at this has been occupied. 
                    // Normally after the directory is created the rest of the save data will be stored, 
                    // followed by the Browse Info File.  So we can assume of the browse info file is present
                    // that the save was completed.
                    //
                    // Here's the main problem, once the directory is saved, until it's deleted, you can't 
                    // save to this directory again, so...if you the save is interupted, not all of the files
                    // required to complete the save will be present, and the save space will be wasted.
                    // So to prevent the problem where a user cannot save over a corrupted file, we are going
                    // to mark the slot as a valid but corrupt slot, once the directory has been discovered.
                    //
                    // It's corrupt status will be updated if the browse info file truly is found, and the 
                    // correct file name is restored.
                    pSavedGame = &mpAvailableSavedGames[mCurrentGameType].mSavedGames[mCurrentGameTypeIndex];
                    ASSERT( pSavedGame );

                    pSavedGame->mSaveGameType      = mCurrentGameType;
                    pSavedGame->mSaveGameTypeIndex = mCurrentGameTypeIndex;
                    pSavedGame->mbValid = TRUE;
                    x_sprintf( pSavedGame->mSaveGameTitle, "Corrupted Data" );

                    mCommandSubState = GET_SAVED_GAME_FOUND_BASE_DIR_NOW_FIND_BROWSE_INFO_FILE;
                }
                // The base directory didn't exist, so just move onto the next file.
                else if( FunctionResult == sceMcResNoEntry )
                {
                    mCommandSubState = GET_SAVED_GAME_GO_TO_NEXT_SAVE_GAME;
                }
                // Stop right now, there was no card inserted.
                else if( PS2MU_SYSTEM_RETURNED_NO_CARD( FunctionResult ) )
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
                }
                // Something strange happened.
                else
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_UNKNOWN_PROBLEM );
                }
            }

            break;
        }

        case GET_SAVED_GAME_GO_TO_NEXT_SAVE_GAME:
        {
            // Move onto the next saved game of this type.
            mCurrentGameTypeIndex++;

            // Get the next directory.
            mCommandSubState = GET_SAVED_GAME_LOOK_FOR_BASE_DIRECTORY;

            break;
        }

        case GET_SAVED_GAME_FOUND_BASE_DIR_NOW_FIND_BROWSE_INFO_FILE:
        {
            ASSERT( !IsFileHandleInUse( ) );

            // Try to open the PS2_STORED_BROWSE_TEXT_FILE file.
            SystemReturnValue = sceMcOpen( mActiveMemoryUnitPort, mActiveMemoryUnitSlot, PS2_STORED_BROWSE_TEXT_FILE, SCE_RDONLY );
        
            // Command accepted, now wait.
            if( PS2MU_SYSTEM_REQUEST_GRANTED( SystemReturnValue ) )
            {
                mCommandSubState = GET_SAVED_GAME_WAITING_FOR_BROWSE_INFO_FILE_TO_OPEN;
            }
            // Couldn't open the file, just move onto the next save game file.
            else
            {
                mCommandSubState = GET_SAVED_GAME_GO_TO_NEXT_SAVE_GAME;
            }

            break;
        }

        case GET_SAVED_GAME_WAITING_FOR_BROWSE_INFO_FILE_TO_OPEN:
        {
            SystemReturnValue = sceMcSync( PS2MU_FUNC_ASYNC_MODE, NULL, &FunctionResult );

            // If for whatever reason there is no function to sync up with, it means that the card was pulled.
            if( SystemReturnValue == sceMcExecIdle )
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
            }
            // If the open is complete, find out what's up.
            else if( SystemReturnValue == sceMcExecFinish )
            {
                // If there was a 0 or positive number returned, this is the FileHandle value.  It was a success.
                if( FunctionResult >= 0 )
                {
                    // Get access to the iformation structure for this single save game.  We need access to it's
                    // save game title information.
                    pSavedGame = &mpAvailableSavedGames[mCurrentGameType].mSavedGames[mCurrentGameTypeIndex];
                    ASSERT( pSavedGame );
                    
                    SetCurrentFileHandle( FunctionResult );
                    SystemReturnValue = sceMcRead( mOpenFileHandle, pSavedGame->mSaveGameTitle, MEMORY_UNIT_SAVE_GAME_TITLE_LENGTH );

                    // Command accepted, now wait.
                    if( PS2MU_SYSTEM_REQUEST_GRANTED( SystemReturnValue ) )
                    {
                        mCommandSubState = GET_SAVED_GAME_WAITING_FOR_BROWSE_INFO_FILE_TO_BE_READ;
                    }
                    // The Read could not be processed.  Clear the local file handle and go to the next save game.
                    else
                    {
                        ClearCurrentFileHandle( );
                        mCommandSubState = GET_SAVED_GAME_GO_TO_NEXT_SAVE_GAME;
                    }
                }
                // The file could not be opened.
                else
                {
                    // If the file wasn't found, just move on to the next one.
                    if( FunctionResult == sceMcResNoEntry )
                    {
                        mCommandSubState = GET_SAVED_GAME_GO_TO_NEXT_SAVE_GAME;
                    }
                    // Stop right now, there was no card inserted.
                    else if( PS2MU_SYSTEM_RETURNED_NO_CARD( FunctionResult ) )
                    {
                        CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
                    }
                    // Something strange happened.
                    else
                    {
                        CompleteCommandAndWaitForUser( FALSE, QUAG_MU_UNKNOWN_PROBLEM );
                    }
                }
            }

            break;
        }

        case GET_SAVED_GAME_WAITING_FOR_BROWSE_INFO_FILE_TO_BE_READ:
        {
            // Test to see if the read is complete.
            SystemReturnValue = sceMcSync( PS2MU_FUNC_ASYNC_MODE, NULL, &FunctionResult );

            // If for whatever reason there is no function to sync up with, it means that the card was pulled.
            if( SystemReturnValue == sceMcExecIdle )
            {
                ClearCurrentFileHandle( );
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
            }
            // If the read is complete, find out what's up.
            else if( SystemReturnValue == sceMcExecFinish )
            {
                if( FunctionResult >= 0 )
                {
                    // Verify that the number of bytes read was the number that was expected.
                    if( FunctionResult == MEMORY_UNIT_SAVE_GAME_TITLE_LENGTH )
                    {
                        // Validate the rest of the Single Saved game structure since we did find it's base directory,
                        // then found the "BrowseText" file and read it's contents correctly.
                        pSavedGame = &mpAvailableSavedGames[mCurrentGameType].mSavedGames[mCurrentGameTypeIndex];
                        pSavedGame->mSaveGameType      = mCurrentGameType;
                        pSavedGame->mSaveGameTypeIndex = mCurrentGameTypeIndex;
                        pSavedGame->mbValid = TRUE;

                        // Close up the file.
                        SystemReturnValue = sceMcClose( mOpenFileHandle );

                        // Command accepted, now wait.
                        if( PS2MU_SYSTEM_REQUEST_GRANTED( SystemReturnValue ) )
                        {
                            mCommandSubState = GET_SAVED_GAME_WAITING_FOR_BROWSE_INFO_FILE_TO_CLOSE;
                        }
                        // The close was not allowed.  Clear the local file handle and go to the next save game.
                        else
                        {
                            ClearCurrentFileHandle( );
                            mCommandSubState = GET_SAVED_GAME_GO_TO_NEXT_SAVE_GAME;
                        }
                    }
                    // The correct number of bytes were not read from the file.  Try to close the file and move on.
                    else
                    {
                        // Close up the file.
                        SystemReturnValue = sceMcClose( mOpenFileHandle );

                        // Command accepted, now wait.
                        if( PS2MU_SYSTEM_REQUEST_GRANTED( SystemReturnValue ) )
                        {
                            mCommandSubState = GET_SAVED_GAME_WAITING_FOR_BROWSE_INFO_FILE_TO_CLOSE;
                        }
                        // The close was not allowed.  Clear the local file handle and go to the next save game.
                        else
                        {
                            ClearCurrentFileHandle( );
                            mCommandSubState = GET_SAVED_GAME_GO_TO_NEXT_SAVE_GAME;
                        }
                    }
                }
                else
                {
                    // Regardless of the error, releae the file handle, since the file is automatically closed by 
                    // the PS2 on a failure.
                    ClearCurrentFileHandle( );

                    // Stop right now, there was no card inserted.
                    if( PS2MU_SYSTEM_RETURNED_NO_CARD( FunctionResult ) )
                    {
                        CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
                    }
                    // Something strange happened.
                    else
                    {
                        CompleteCommandAndWaitForUser( FALSE, QUAG_MU_UNKNOWN_PROBLEM );
                    }
                }
            }
            break;
        }

        case GET_SAVED_GAME_WAITING_FOR_BROWSE_INFO_FILE_TO_CLOSE:
        {
            // Test to see if the closing of the file is complete.
            SystemReturnValue = sceMcSync( PS2MU_FUNC_ASYNC_MODE, NULL, &FunctionResult );

            // If for whatever reason there is no function to sync up with, it means that the card was pulled.
            if( SystemReturnValue == sceMcExecIdle )
            {
                ClearCurrentFileHandle( );
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
            }
            else if( SystemReturnValue == sceMcExecFinish )
            {
                // If the close is complete, make sure it wasn't due to a card being removed.
                if( PS2MU_SYSTEM_RETURNED_NO_CARD( FunctionResult ) )
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
                }
                // If all is well, the close was successful, and we can go onto the next save game.
                else
                {
                    ClearCurrentFileHandle( );
                    mCommandSubState = GET_SAVED_GAME_GO_TO_NEXT_SAVE_GAME;
                }
            }

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
void PS2MemoryUnitManager::FormatHandler( void )
{
    ASSERT( mInitialized );

    s32     SystemReturnValue;
    s32     FunctionResult;

    enum FormatStates
    {
        FORMAT_STATE_INIT,
        FORMAT_STATE_REQUEST_FORMAT,
        FORMAT_STATE_WAIT_FOR_FORMAT_TO_COMPLETE,
#ifdef X_DEBUG
        FORMAT_STATE_REQUEST_UN_FORMAT,
#endif
        FORMAT_STATE_RE_EVALUATE_CARD,
        FORMAT_STATE_COMPLETE,
    };

    switch( mCommandState )
    {
        case FORMAT_STATE_INIT:
        {
            // First make sure that the card is actually present before you begin.
            if( IsActiveMemoryUnitStillConnected( ) )
            {
                // Make sure that this card isn't already formatted since you can not re-format a card on the PS2 that's already 
                // formatted.
                if( mActiveMemoryUnit->mbFormatted )
#ifdef X_DEBUG
                // In debug mode, force the card to be unformatted first.
                {
                    MC_PRINT( 1, "Unformatting card %s", mActiveMemoryUnit->mName );
                    mCommandState = FORMAT_STATE_REQUEST_UN_FORMAT;
    
                    // Clear the available games list.
                    ClearAvailableGamesList( );
                }
#else
                // In release mode, identify that the card cannot be formatted again if it's already formatted.
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_ALREADY_FORMATTED );
                }
#endif
                else
                {
                    mCommandState = FORMAT_STATE_REQUEST_FORMAT;
                }
            }
            else
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
            }

            break;
        }

        case FORMAT_STATE_REQUEST_FORMAT:
        {
            // Request for the format of this card.
            SystemReturnValue = sceMcFormat( mActiveMemoryUnitPort, mActiveMemoryUnitSlot );

            // Command accepted, now wait.
            if( PS2MU_SYSTEM_REQUEST_GRANTED( SystemReturnValue ) )
            {
                MC_PRINT( 1, "Formatting card %s", mActiveMemoryUnit->mName );
                mCommandState = FORMAT_STATE_WAIT_FOR_FORMAT_TO_COMPLETE;
            }
            // The format request was not granted, so bail now.
            else
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NOT_FORMATTED );
            }

            break;
        }

        case FORMAT_STATE_WAIT_FOR_FORMAT_TO_COMPLETE:
        {
            SystemReturnValue = sceMcSync( PS2MU_FUNC_ASYNC_MODE, NULL, &FunctionResult );

            // If for whatever reason there is no function to sync up with, it means that the card was pulled.
            if( SystemReturnValue == sceMcExecIdle )
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
            }
            // The function is complete.
            else if( SystemReturnValue == sceMcExecFinish )
            {
                if( FunctionResult == sceMcResSucceed )
                {
                    mCommandState    = FORMAT_STATE_RE_EVALUATE_CARD;
                    mCommandSubState = 0;
                }
                else if( PS2MU_SYSTEM_RETURNED_NO_CARD( FunctionResult ) )
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
                }
                else
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_UNKNOWN_PROBLEM );
                }
            }

            break;
        }

#ifdef X_DEBUG
        case FORMAT_STATE_REQUEST_UN_FORMAT:
        {
            // Request for the format of this card.
            SystemReturnValue = sceMcUnformat( mActiveMemoryUnitPort, mActiveMemoryUnitSlot );

            // Command accepted, now wait.
            if( PS2MU_SYSTEM_REQUEST_GRANTED( SystemReturnValue ) == FALSE )
            {
                ASSERT( 0 );
            }

            // Wait for the unformat to happen, then request a format.
            SystemReturnValue = sceMcSync( PS2MU_FUNC_SYNC_MODE, NULL, &FunctionResult );

            // If for whatever reason there is no function to sync up with, it means that the card was pulled.
            if( SystemReturnValue == sceMcExecFinish )
            {   
                if( FunctionResult == sceMcResSucceed )
                {
                    mActiveMemoryUnit->mbFormatted = FALSE;
                }
                else
                {
                    ASSERT( 0 );
                }
            }

            // Now let the system process that this card has changed.
            SystemReturnValue = sceMcGetInfo( mActiveMemoryUnitPort, mActiveMemoryUnitSlot, &mActiveMemoryUnit->mType, (s32*)&mActiveMemoryUnit->mFreeClusters, &mActiveMemoryUnit->mbFormatted );
                
            SystemReturnValue = sceMcSync( PS2MU_FUNC_SYNC_MODE, NULL, &FunctionResult );
            ASSERT( SystemReturnValue != sceMcExecIdle );

            // The function is complete.
            if( SystemReturnValue == sceMcExecFinish )
            {
                // It's the same card as last time this function was called.  All is good.
                if( FunctionResult == sceMcResSucceed || 
                    FunctionResult == sceMcResChangedCard || 
                    FunctionResult == sceMcResNoFormat
                  )
                {
                    mCommandState = FORMAT_STATE_REQUEST_FORMAT;
                }
                else
                {
                    ASSERT( 0 );
                }
            }

            break;
        }
#endif
        case FORMAT_STATE_RE_EVALUATE_CARD:
        {
            if( ReEvaluateCardStatus( ) )
                mCommandState = FORMAT_STATE_COMPLETE;
            
            break;
        }

        case FORMAT_STATE_COMPLETE:
        {
            mActiveMemoryUnit->mbFormatted = TRUE;
            CompleteCommandAndWaitForUser( TRUE, QUAG_MU_EVERYTHING_IS_OK );
            break;
        }

        default:
            ASSERT( 0 );
            break;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
void PS2MemoryUnitManager::SaveGameHandler( void )
{
    ASSERT( mInitialized );

    // Here's how a save game is going to be performed.
    // 
    // 1 - Create the base directory for this save game type.  The name of the base directory resides in 
    //     mpSaveGameTypes[mCurrentGameType].mBaseDirectories[mCurrentGameTypeIndex].mActualBaseDirName
    //
    // 2 - Fill out the structure required for the PS2_STORED_ICON_INFO file.
    //     It's data is stored within the mIconData.mSonyMcIconSys member.
    //
    // 3 - Save the PS2_STORED_ICON_INFO file.
    //
    // 4 - Save the actual PS2 Icon file, called PS2_STORED_ICON_DATA_FILE.  This file consists entirely of
    //     the raw icon data that is stored in mIconData.mpIconData.
    //  
    // 5 - Create the User Data file provided using the name provided in 
    //     mpSaveGameTypes[mCurrentGameType].mUserDataFileInfo.mUserDataSaveFileName
    //
    // 6 - Prepare the PS2_STORED_BROWSE_TEXT_FILE file, fill it with the browser text, and write it out.
    //
    enum SAVE_GAME_STATES
    {
        SAVE_GAME_STATE_INIT,

        SAVE_GAME_STATE_CREATE_BASE_DIR,                // Create the base directory to store the rest of this saved game.
        SAVE_GAME_STATE_CREATE_BASE_DIR_NAME_FILE,      // PS2 requires a user data file that is the exact name of the base directory.
        SAVE_GAME_STATE_CREATE_ICON_INFO_FILE,          // Create the Icon.sys file which tells the system how to render the icon in the browser.
        SAVE_GAME_STATE_CREATE_ICON_DATA_FILE,          // Create the actual Icon.ico data file.  This is the true icon file.
        SAVE_GAME_STATE_CREATE_USER_DATA_FILE,          // The user required data file for this save game type.
        SAVE_GAME_STATE_CREATE_BROWSE_TEXT_FILE,        // Create a separate user file called "BrowseText", this file contains a copy of the text displayed
                                                        // on the browser screen.

        SAVE_GAME_RE_EVALUATE_CARD_STATUS,

        SAVE_GAME_STATE_DONE,
    };

    switch( mCommandState )
    {
        // Test to see if this game slot is occupied already, if it is, then return that it already
        // exists.  This means that they must delete the save before they can save it again.
        case SAVE_GAME_STATE_INIT:
        {
            if( mpAvailableSavedGames[mCurrentGameType].mSavedGames[mCurrentGameTypeIndex].mbValid )
            {
                CompleteCommandAndWaitForUser(FALSE, QUAG_MU_FILE_ALREADY_EXISTS );
            }
            // Start by initializing the command sub state.
            else
            {
                mCommandSubState = 0;
                mCommandState = SAVE_GAME_STATE_CREATE_BASE_DIR;
            }
            break;
        }
        
        // Process the creation of the games base dir.
        case SAVE_GAME_STATE_CREATE_BASE_DIR:
        {
            if( SaveGameHandlerCreateBaseDir( ) )
            {
                mCommandState = SAVE_GAME_STATE_CREATE_BASE_DIR_NAME_FILE;
                mCommandSubState = 0;
            }
            break;
        }

        case SAVE_GAME_STATE_CREATE_BASE_DIR_NAME_FILE:
        {
            if( SaveGameHandlerSaveSingleFile( PS2MU_FILE_TYPE_BASE_DIR_NAME_FILE ) )
            {
                mCommandState = SAVE_GAME_STATE_CREATE_ICON_INFO_FILE;
                mCommandSubState = 0;
            }
            break;
        }

        case SAVE_GAME_STATE_CREATE_ICON_INFO_FILE:
        {
            if( SaveGameHandlerSaveSingleFile( PS2MU_FILE_TYPE_ICON_INFO ) )
            {
                mCommandState = SAVE_GAME_STATE_CREATE_ICON_DATA_FILE;
                mCommandSubState = 0;
            }
            break;
        }

        case SAVE_GAME_STATE_CREATE_ICON_DATA_FILE:
        {
            if( SaveGameHandlerSaveSingleFile( PS2MU_FILE_TYPE_ICON_DATA ) )
            {
                mCommandState = SAVE_GAME_STATE_CREATE_USER_DATA_FILE;
                mCommandSubState = 0;
            }
            break;
        }

        case SAVE_GAME_STATE_CREATE_USER_DATA_FILE:
        {
            if( SaveGameHandlerSaveSingleFile( PS2MU_FILE_TYPE_USER_DATA ) )
            {
                mCommandState    = SAVE_GAME_STATE_CREATE_BROWSE_TEXT_FILE;
                mCommandSubState = 0;
            }
            break;
        }

        case SAVE_GAME_STATE_CREATE_BROWSE_TEXT_FILE:
        {
            if( SaveGameHandlerSaveSingleFile( PS2MU_FILE_TYPE_BROWSE_TEXT ) )
            {
                SingleSavedGameInfo* pSavedGameInfo;

                // Complete the job by updating the available save game info with this save.
                pSavedGameInfo = &mpAvailableSavedGames[mCurrentGameType].mSavedGames[mCurrentGameTypeIndex];
                pSavedGameInfo->mbValid            = TRUE;
                pSavedGameInfo->mSaveGameType      = mCurrentGameType;
                pSavedGameInfo->mSaveGameTypeIndex = mCurrentGameTypeIndex;
                x_strncpy( pSavedGameInfo->mSaveGameTitle, mBrowserTitleText, MEMORY_UNIT_SAVE_GAME_TITLE_LENGTH );


                // ReEvaluate the status of the active card.
                mCommandState = SAVE_GAME_RE_EVALUATE_CARD_STATUS;
                mCommandSubState = 0;
            }
            break;
        }

        case SAVE_GAME_RE_EVALUATE_CARD_STATUS:
        {
            if( ReEvaluateCardStatus( ) )
                mCommandState = SAVE_GAME_STATE_DONE;
            
            break;
        }

        
        case SAVE_GAME_STATE_DONE:
        {
            f64 ElapsedTime;
            ElapsedTime = x_GetElapsedTime( mCommandStartTime );

            if( ElapsedTime > PS2MU_MIN_SAVED_GAME_TIME )
            {
                // Now complete the command.
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
xbool PS2MemoryUnitManager::SaveGameHandlerCreateBaseDir( void )
{
    ASSERT( mInitialized );

    s32 SystemReturnValue;
    s32 FunctionResult;

    enum SaveGameCreateBaseDirStates
    {
        SAVE_GAME_CREATE_BASE_DIR_INIT,
        SAVE_GAME_CREATE_BASE_DIR_WAIT_FOR_COMPLETE,
        SAVE_GAME_CREATE_BASE_DIR_CHANGE_DIR_TO_BASE_DIRECTORY,
        SAVE_GAME_CREATE_BASE_DIR_WAIT_FOR_CHANGE_DIR_TO_COMPLETE,
    };

    switch( mCommandSubState )
    {
        // Try to initiate the request to make the base directory.
        case SAVE_GAME_CREATE_BASE_DIR_INIT:
        {
            char* BaseDirString;
            BaseDirString = mpSaveGameTypes[mCurrentGameType].mBaseDirectories[mCurrentGameTypeIndex].mActualBaseDirName;

            SystemReturnValue = sceMcMkdir( mActiveMemoryUnitPort, mActiveMemoryUnitSlot, BaseDirString );

            // Command accepted, now wait.
            if( PS2MU_SYSTEM_REQUEST_GRANTED( SystemReturnValue ) )
            {
                mCommandSubState = SAVE_GAME_CREATE_BASE_DIR_WAIT_FOR_COMPLETE;
            }
            // The request to create the base dir failed, just bail now.
            else
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_SAVE_FAILED );
            }
            break;
        }

        // If we are here, we are waiting for the base directory to be created.
        case SAVE_GAME_CREATE_BASE_DIR_WAIT_FOR_COMPLETE:
        {
            SystemReturnValue = sceMcSync( PS2MU_FUNC_ASYNC_MODE, NULL, &FunctionResult );

            // If for whatever reason there is no function to sync up with, it means that the card was pulled.
            if( SystemReturnValue == sceMcExecIdle )
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
            }
            // The function is complete.
            else if( SystemReturnValue == sceMcExecFinish )
            {
                // Did the base directory get created?
                if( FunctionResult == sceMcResSucceed )
                {
                    mCommandSubState = SAVE_GAME_CREATE_BASE_DIR_CHANGE_DIR_TO_BASE_DIRECTORY;
                }
                // Did the memory card have enough space to create a new base directory?
                else if( FunctionResult == sceMcResFullDevice )
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NOT_ENOUGH_FREE_SPACE );
                }
                else if( PS2MU_SYSTEM_RETURNED_NO_CARD( FunctionResult ) )
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
                }
                else
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_UNKNOWN_PROBLEM );
                }
            }

            break;
        }

        case SAVE_GAME_CREATE_BASE_DIR_CHANGE_DIR_TO_BASE_DIRECTORY:
        {
            char* BaseDirString;
            BaseDirString = mpSaveGameTypes[mCurrentGameType].mBaseDirectories[mCurrentGameTypeIndex].mActualBaseDirName;
            SystemReturnValue = sceMcChdir( mActiveMemoryUnitPort, mActiveMemoryUnitSlot, BaseDirString, 0 );

            // If the Request to change to this base directory succeeded, then continue to wait for the result.
            if( PS2MU_SYSTEM_REQUEST_GRANTED( SystemReturnValue ) )
            {
                mCommandSubState = SAVE_GAME_CREATE_BASE_DIR_WAIT_FOR_CHANGE_DIR_TO_COMPLETE;
            }
            // The request to create the base dir failed, just bail now.
            else
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_SAVE_FAILED );
            }
            break;
        }

        case SAVE_GAME_CREATE_BASE_DIR_WAIT_FOR_CHANGE_DIR_TO_COMPLETE:
        {
            SystemReturnValue = sceMcSync( PS2MU_FUNC_ASYNC_MODE, NULL, &FunctionResult );

            // If for whatever reason there is no function to sync up with, it means that the card was pulled.
            if( SystemReturnValue == sceMcExecIdle )
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
            }
            // The function is complete.
            else if( SystemReturnValue == sceMcExecFinish )
            {
                // Did the base directory get created?
                if( FunctionResult == sceMcResSucceed )
                {
                    return TRUE;
                }
                // Was the card removed?
                else if( PS2MU_SYSTEM_RETURNED_NO_CARD( FunctionResult ) )
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
                }
                // Was there another problem?
                else
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_SAVE_FAILED );
                }
            }

            break;
        }

        default:
            ASSERT( 0 );
            break;
    }


    return FALSE;
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool PS2MemoryUnitManager::SaveGameHandlerSaveSingleFile( s32 FileType )
{
    ASSERT( mInitialized );

    s32 SystemReturnValue;
    s32 FunctionResult;

    enum SaveGameSaveSingleFileStates
    {
        SAVE_GAME_SINGLE_FILE_INIT,
        SAVE_GAME_SINGLE_FILE_WAIT_FOR_FILE_TO_BE_CREATED,
        SAVE_GAME_SINGLE_FILE_WAIT_FOR_FILE_TO_BE_WRITTEN,
        SAVE_GAME_SINGLE_FILE_WAIT_FOR_FILE_TO_BE_CLOSED,
        SAVE_GAME_SINGLE_FILE_WAIT_FOR_FILE_TO_BE_CLOSED_AFTER_WRITE_FAILURE,
    };

    switch( mCommandSubState )
    {
        case SAVE_GAME_SINGLE_FILE_INIT:
        {
            switch( FileType )
            {
                case PS2MU_FILE_TYPE_BASE_DIR_NAME_FILE:
                {
                    // Record the name of the file that is to be opened and written to.
                    x_strncpy( mCurrentOpenFileName, 
                               &mpSaveGameTypes[mCurrentGameType].mBaseDirectories[mCurrentGameTypeIndex].mActualBaseDirName[1],    // The first character is '/' and we must remove it.
                               PS2MU_UTILITY_FILENAME_SIZE );

                    // Record where the data to be written is from.
                    mpOpenGameFileData = &PS2MemoryUnitManager::msBaseDirNameFileData;

                    // Record how much data there will be written.
                    mOpenGameFileSize = PS2MemoryUnitManager::msBaseDirNameFileSize;
                    break;
                }

                case PS2MU_FILE_TYPE_ICON_INFO:
                {
                    // Convert the browser text to SJIS format for the icon info file.
                    AsciiStringToSJISString( mBrowserTitleText, (u16*)mIconData.mSonyMcIconSys.TitleName );
                    mIconData.mSonyMcIconSys.OffsLF = mBrowserTitleBreak;

                    // Record the name of the file that is to be opened and written to.
                    x_strncpy( mCurrentOpenFileName, PS2_STORED_ICON_INFO_FILE, PS2MU_UTILITY_FILENAME_SIZE );

                    // Record where the data to be written is from.
                    mpOpenGameFileData = &mIconData.mSonyMcIconSys;

                    // Record how much data there will be written.
                    mOpenGameFileSize = sizeof( mIconData.mSonyMcIconSys );
                    break;
                }

                case PS2MU_FILE_TYPE_ICON_DATA:
                {
                    // Record the name of the file that is to be opened and written to.
                    x_strncpy( mCurrentOpenFileName, PS2_STORED_ICON_DATA_FILE, PS2MU_UTILITY_FILENAME_SIZE );

                    // Record where the data to be written is from.
                    mpOpenGameFileData = mIconData.mpIconData;

                    // Record how much data there will be written.
                    mOpenGameFileSize = mIconData.mIconDataSize;
                    break;
                }
    
                case PS2MU_FILE_TYPE_BROWSE_TEXT:
                {
                    // Record the name of the file that is to be opened and written to.
                    x_strncpy( mCurrentOpenFileName, PS2_STORED_BROWSE_TEXT_FILE, PS2MU_UTILITY_FILENAME_SIZE );

                    // Record where the data to be written is from.
                    mpOpenGameFileData = mBrowserTitleText;

                    // Record how much data there will be written.
                    mOpenGameFileSize = PS2_STORED_DIRECTORY_NAME_SIZE;
                    break;
                }

                case PS2MU_FILE_TYPE_USER_DATA:
                {
                    // Record the name of the file that is to be opened and written to.
                    x_strncpy( mCurrentOpenFileName, 
                               mpSaveGameTypes[mCurrentGameType].mUserDataFileInfo.mUserDataSaveFileName,
                               PS2MU_UTILITY_FILENAME_SIZE );

                    // Record where the data to be written is from.
                    mpOpenGameFileData = mpUserBuffer;

                    // Record how much data there will be written.
                    mOpenGameFileSize = mpSaveGameTypes[mCurrentGameType].mUserDataFileInfo.mSizeOfUserDataFile;
                    break;
                }

                default:
                    ASSERT( 0 );
                    break;
            }



            // Fail if there is already a file open.
            ASSERT( !IsFileHandleInUse( ) );

            // Request that the file be created.
            SystemReturnValue = sceMcOpen( mActiveMemoryUnitPort, mActiveMemoryUnitSlot, mCurrentOpenFileName, SCE_CREAT );

            // Command accepted, now wait.
            if( PS2MU_SYSTEM_REQUEST_GRANTED( SystemReturnValue ) )
            {
                mCommandSubState = SAVE_GAME_SINGLE_FILE_WAIT_FOR_FILE_TO_BE_CREATED;
            }
            // The request to create the file failed, so bail.
            else
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_SAVE_FAILED );
            }
            break;
        }

        case SAVE_GAME_SINGLE_FILE_WAIT_FOR_FILE_TO_BE_CREATED:
        {
            SystemReturnValue = sceMcSync( PS2MU_FUNC_ASYNC_MODE, NULL, &FunctionResult );
            
             // If for whatever reason there is no function to sync up with, it means that the card was pulled.
            if( SystemReturnValue == sceMcExecIdle )
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
            }
            // The function is complete.
            else if( SystemReturnValue == sceMcExecFinish )
            {
                // If there was a 0 or positive number returned, this is the FileHandle value.  It was a success.
                if( FunctionResult >= 0 )
                {
                    // Set the current file handle
                    SetCurrentFileHandle( FunctionResult );
                    SystemReturnValue = sceMcWrite( mOpenFileHandle, mpOpenGameFileData, mOpenGameFileSize );

                    // Command accepted, now wait.
                    if( PS2MU_SYSTEM_REQUEST_GRANTED( SystemReturnValue ) )
                    {
                        mCommandSubState = SAVE_GAME_SINGLE_FILE_WAIT_FOR_FILE_TO_BE_WRITTEN;
                    }
                    // The write could not be processed.  Clear the local file handle and quit.
                    else
                    {
                        ClearCurrentFileHandle(  );
                        CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_SAVE_FAILED );
                    }
                }
                // Did the memory card have enough space to create the new file?.
                else if( FunctionResult == sceMcResFullDevice )
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NOT_ENOUGH_FREE_SPACE );
                }
                else if( FunctionResult == sceMcResDeniedPermit )
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_FILE_ACCESS_DENIED );
                }
                else if( PS2MU_SYSTEM_RETURNED_NO_CARD( FunctionResult ) )
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
                }
                else
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_SAVE_FAILED );
                }
            }

            break;
        }

        case SAVE_GAME_SINGLE_FILE_WAIT_FOR_FILE_TO_BE_WRITTEN:
        {
            SystemReturnValue = sceMcSync( PS2MU_FUNC_ASYNC_MODE, NULL, &FunctionResult );

            // If for whatever reason there is no function to sync up with, it means that the card was pulled.
            if( SystemReturnValue == sceMcExecIdle )
            {
                ClearCurrentFileHandle( );
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
            }
            // The function is complete.
            else if( SystemReturnValue == sceMcExecFinish )
            {
                // If the number returned is 0 or more, that identifies the number of bytes written.
                if( FunctionResult >= 0 )
                {
                    // The file request was granted, time to close the file.
                    SystemReturnValue = sceMcClose( mOpenFileHandle  );

                    // If the close file command was accepted, decide whether to close the file normally,
                    // or create a write error once the close was complete.
                    if( PS2MU_SYSTEM_REQUEST_GRANTED( SystemReturnValue ) )
                    {
                        // Make sure that the correct number of bytes were written to the file.
                        // If there were the correct number of bytes written, then close the file normally.
                        if( FunctionResult == mOpenGameFileSize )
                        {
                            mCommandSubState = SAVE_GAME_SINGLE_FILE_WAIT_FOR_FILE_TO_BE_CLOSED;
                        }
                        // If the number of bytes written to the file were not correct, then
                        // make sure to close the file, but end on an error condition.
                        else
                        {
                            mCommandSubState = SAVE_GAME_SINGLE_FILE_WAIT_FOR_FILE_TO_BE_CLOSED_AFTER_WRITE_FAILURE;
                        }
                    }
                    // The write could not be processed.  Clear the local file handle and quit.
                    else
                    {
                        ClearCurrentFileHandle(  );
                        CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_SAVE_FAILED );
                    }
                }
                // Did the memory card have enough space to create the new file?.
                else if( FunctionResult == sceMcResFullDevice )
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NOT_ENOUGH_FREE_SPACE );
                }
                else if( FunctionResult == sceMcResDeniedPermit )
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_FILE_ACCESS_DENIED );
                }
                else if( PS2MU_SYSTEM_RETURNED_NO_CARD( FunctionResult ) )
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
                }
                else
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_SAVE_FAILED );
                }
            }
            break;
        }

        case SAVE_GAME_SINGLE_FILE_WAIT_FOR_FILE_TO_BE_CLOSED:
        case SAVE_GAME_SINGLE_FILE_WAIT_FOR_FILE_TO_BE_CLOSED_AFTER_WRITE_FAILURE:
        {
            SystemReturnValue = sceMcSync( PS2MU_FUNC_ASYNC_MODE, NULL, &FunctionResult );

            // If for whatever reason there is no function to sync up with, it means that the card was pulled.
            if( SystemReturnValue == sceMcExecIdle )
            {
                ClearCurrentFileHandle( );
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
            }
            // The function is complete.
            else if( SystemReturnValue == sceMcExecFinish )
            {
                ClearCurrentFileHandle( );

                // If the number returned is 0 or more, that identifies the number of bytes written.
                if( FunctionResult == sceMcResSucceed )
                {
                    if( mCommandSubState == SAVE_GAME_SINGLE_FILE_WAIT_FOR_FILE_TO_BE_CLOSED )
                    {
                        return TRUE;
                    }
                    else if( mCommandSubState == SAVE_GAME_SINGLE_FILE_WAIT_FOR_FILE_TO_BE_CLOSED_AFTER_WRITE_FAILURE )
                    {
                        CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_SAVE_FAILED );
                    }
                    else
                    {
                        ASSERT( 0 );
                    }
                }
                else if( PS2MU_SYSTEM_RETURNED_NO_CARD( FunctionResult ) )
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
                }
                else
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_SAVE_FAILED );
                }
            }
            break;
        }
    }

    return FALSE;
}

//-------------------------------------------------------------------------------------------------------------------------------------
void PS2MemoryUnitManager::LoadGameHandler( void )
{
    ASSERT( mInitialized );

    char*   BasePathString;
    s32     SystemReturnValue;
    s32     FunctionResult;

    enum LOAD_GAME_STATES
    {
        LOAD_GAME_STATE_INIT,
        LOAD_GAME_WAIT_FOR_BASE_DIR_TO_BE_SET,
        LOAD_GAME_STATE_OPEN_USER_DATA_FILE,
        LOAD_GAME_STATE_WAIT_FOR_USER_DATA_FILE_TO_OPEN,
        LOAD_GAME_STATE_WAIT_FOR_DATA_FILE_TO_BE_LOADED,
        LOAD_GAME_STATE_WAIT_FOR_DATA_FILE_TO_BE_CLOSED,
        LOAD_GAME_STATE_WAIT_FOR_DATA_FILE_TO_BE_CLOSED_WITH_LOAD_FAILURE,

        LOAD_GAME_STATE_RE_EVALUATE_CARD,

        LOAD_GAME_STATE_DONE,
    };

    switch( mCommandState )
    {
        // Set the current base directory.
        case LOAD_GAME_STATE_INIT:
        {
            // Change to the base directory that stores this saved game.
            BasePathString = mpSaveGameTypes[mCurrentGameType].mBaseDirectories[mCurrentGameTypeIndex].mActualBaseDirName;

            SystemReturnValue = sceMcChdir( mActiveMemoryUnitPort, mActiveMemoryUnitSlot, BasePathString, 0 );

            // If the Request to change to this base directory succeeded, then continue to wait for the result.
            if( PS2MU_SYSTEM_REQUEST_GRANTED( SystemReturnValue ) )
            {
                mCommandState = LOAD_GAME_WAIT_FOR_BASE_DIR_TO_BE_SET;
            }
            // The request to change to the base directory failed, just bail now.
            else
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_LOAD_FAILED );
            }
            break;
        }

        case LOAD_GAME_WAIT_FOR_BASE_DIR_TO_BE_SET:
        {
            SystemReturnValue = sceMcSync( PS2MU_FUNC_ASYNC_MODE, NULL, &FunctionResult );

            // If for whatever reason there is no function to sync up with, it means that the card was pulled.
            if( SystemReturnValue == sceMcExecIdle )
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
            }
            // The function is complete.
            else if( SystemReturnValue == sceMcExecFinish )
            {
                // Are we now in the base directory path?
                if( FunctionResult == sceMcResSucceed )
                {
                    mCommandState = LOAD_GAME_STATE_OPEN_USER_DATA_FILE;
                }
                // Was the card removed?
                else if( PS2MU_SYSTEM_RETURNED_NO_CARD( FunctionResult ) )
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
                }
                // Was there another problem?
                else
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_LOAD_FAILED );
                }
            }

            break;
        }

        case LOAD_GAME_STATE_OPEN_USER_DATA_FILE:
        {
            ASSERT( !IsFileHandleInUse( ) );

            // Record the name of the file that is to be opened and read from.
            x_strncpy( mCurrentOpenFileName, 
                       mpSaveGameTypes[mCurrentGameType].mUserDataFileInfo.mUserDataSaveFileName,
                       PS2MU_UTILITY_FILENAME_SIZE );

            // Record where the data to be written is from.
            mpOpenGameFileData = mpUserBuffer;

            // Record how much data there will be written.
            mOpenGameFileSize = mpSaveGameTypes[mCurrentGameType].mUserDataFileInfo.mSizeOfUserDataFile;

            // Try to open the User Data File.
            SystemReturnValue = sceMcOpen( mActiveMemoryUnitPort, mActiveMemoryUnitSlot, mCurrentOpenFileName, SCE_RDONLY );
        
            // Command accepted, now wait.
            if( PS2MU_SYSTEM_REQUEST_GRANTED( SystemReturnValue ) )
            {
                mCommandState = LOAD_GAME_STATE_WAIT_FOR_USER_DATA_FILE_TO_OPEN;
            }
            // Couldn't open the file, bail now
            else
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_LOAD_FAILED );
            }

            break;
        }

        case LOAD_GAME_STATE_WAIT_FOR_USER_DATA_FILE_TO_OPEN:
        {
            SystemReturnValue = sceMcSync( PS2MU_FUNC_ASYNC_MODE, NULL, &FunctionResult );

            // If for whatever reason there is no function to sync up with, it means that the card was pulled.
            if( SystemReturnValue == sceMcExecIdle )
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
            }
            // If the open is complete, find out what's up.
            else if( SystemReturnValue == sceMcExecFinish )
            {
                // If there was a 0 or positive number returned, this is the FileHandle value.  It was a success.
                if( FunctionResult >= 0 )
                {
                    // Set the current File Handle
                    SetCurrentFileHandle( FunctionResult );

                    // Try to read out the user data.
                    SystemReturnValue = sceMcRead( mOpenFileHandle, mpOpenGameFileData, mOpenGameFileSize );

                    // Command accepted, now wait.
                    if( PS2MU_SYSTEM_REQUEST_GRANTED( SystemReturnValue ) )
                    {
                        mCommandState = LOAD_GAME_STATE_WAIT_FOR_DATA_FILE_TO_BE_LOADED;
                    }
                    // The Read could not be processed.  Time to bail.
                    else
                    {
                        ClearCurrentFileHandle( );
                        CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_LOAD_FAILED );
                    }
                }
                // The file could not be opened.
                else
                {
                    // If the file wasn't found.  This is bad.
                    if( FunctionResult == sceMcResNoEntry )
                    {
                        CompleteCommandAndWaitForUser( FALSE, QUAG_MU_FILE_NOT_FOUND );
                    }
                    // Stop right now, there was no card inserted.
                    else if( PS2MU_SYSTEM_RETURNED_NO_CARD( FunctionResult ) )
                    {
                        CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
                    }
                    // Something strange happened.
                    else
                    {
                        CompleteCommandAndWaitForUser( FALSE, QUAG_MU_UNKNOWN_PROBLEM );
                    }
                }
            }

            break;
        }

        case LOAD_GAME_STATE_WAIT_FOR_DATA_FILE_TO_BE_LOADED:
        {
            // Test to see if the read is complete.
            SystemReturnValue = sceMcSync( PS2MU_FUNC_ASYNC_MODE, NULL, &FunctionResult );

            // If for whatever reason there is no function to sync up with, it means that the card was pulled.
            if( SystemReturnValue == sceMcExecIdle )
            {
                ClearCurrentFileHandle( );
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
            }
            // If the read is complete, find out what's up.
            else if( SystemReturnValue == sceMcExecFinish )
            {
                if( FunctionResult >= 0 )
                {
                    xbool bCorrectNumberOfBytesRead;

                    // Test to see if the number of bytes read was the number that was expected.
                    bCorrectNumberOfBytesRead = FunctionResult == mOpenGameFileSize;

                    // Request to close the opened file.
                    SystemReturnValue = sceMcClose( mOpenFileHandle );

                    // Command accepted, now wait.
                    if( PS2MU_SYSTEM_REQUEST_GRANTED( SystemReturnValue ) )
                    {
                        // If the number of bytes read was correct, assume that the load is complete.
                        // If the number of bytes read was incorrect, then identify that the load failed once the file is closed.
                        if( bCorrectNumberOfBytesRead )
                            mCommandState = LOAD_GAME_STATE_WAIT_FOR_DATA_FILE_TO_BE_CLOSED;
                        else
                            mCommandState = LOAD_GAME_STATE_WAIT_FOR_DATA_FILE_TO_BE_CLOSED_WITH_LOAD_FAILURE;
                    }
                    // The close was not allowed.  Clear the local file handle and just end the load.
                    else
                    {
                        ClearCurrentFileHandle( );

                        // If the number of bytes read was correct, assume that the load is complete.
                        // If the number of bytes read was incorrect, then report that the load failed immediately.
                        if( bCorrectNumberOfBytesRead )
                            mCommandState = LOAD_GAME_STATE_DONE;
                        else
                            CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_LOAD_FAILED );
                    }
                }
                else
                {
                    // Regardless of the error, releae the file handle, since the file is automatically closed by 
                    // the PS2 on a failure.
                    ClearCurrentFileHandle( );

                    // Stop right now, there was no card inserted.
                    if( PS2MU_SYSTEM_RETURNED_NO_CARD( FunctionResult ) )
                    {
                        CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
                    }
                    // Something strange happened.
                    else
                    {
                        CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_LOAD_FAILED );
                    }
                }
            }
            break;
        }

        case LOAD_GAME_STATE_WAIT_FOR_DATA_FILE_TO_BE_CLOSED:
        case LOAD_GAME_STATE_WAIT_FOR_DATA_FILE_TO_BE_CLOSED_WITH_LOAD_FAILURE:
        {
            // Test to see if the closing of the file is complete.
            SystemReturnValue = sceMcSync( PS2MU_FUNC_ASYNC_MODE, NULL, &FunctionResult );

            // If for whatever reason there is no function to sync up with, it means that the card was pulled.
            if( SystemReturnValue == sceMcExecIdle )
            {
                ClearCurrentFileHandle( );
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
            }
            else if( SystemReturnValue == sceMcExecFinish )
            {
                // If the close is complete, make sure it wasn't due to a card being removed.
                if( PS2MU_SYSTEM_RETURNED_NO_CARD( FunctionResult ) )
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
                }
                // If all is well, the close was successful
                else
                {
                    ClearCurrentFileHandle( );

                    // if the current command was to close a succesfully loaded file.  Then re-evaluate the
                    // card's status, and move onward.
                    if( mCommandState == LOAD_GAME_STATE_WAIT_FOR_DATA_FILE_TO_BE_CLOSED )
                    {
                        mCommandState    = LOAD_GAME_STATE_RE_EVALUATE_CARD;
                        mCommandSubState = 0;
                    }
                    // If the current command state was to identify a load error once the file was closed, 
                    // then report that error now.
                    else if( mCommandState == LOAD_GAME_STATE_WAIT_FOR_DATA_FILE_TO_BE_CLOSED_WITH_LOAD_FAILURE )
                    {
                        CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_LOAD_FAILED );
                    }
                    else
                    {
                        ASSERT( 0 );
                    }
                }
            }

            break;
        }

        case LOAD_GAME_STATE_RE_EVALUATE_CARD:
        {
            if( ReEvaluateCardStatus( ) )
                mCommandState = LOAD_GAME_STATE_DONE;
            
            break;
        }

        case LOAD_GAME_STATE_DONE:
        {
            f64 ElapsedTime;
            ElapsedTime = x_GetElapsedTime( mCommandStartTime );

            if( ElapsedTime > PS2MU_MIN_LOAD_GAME_TIME )
                CompleteCommandAndWaitForUser( TRUE, QUAG_MU_EVERYTHING_IS_OK );

            break;
        }

        default:
            ASSERT( 0 );
            break;
    }            

}

//-------------------------------------------------------------------------------------------------------------------------------------
void PS2MemoryUnitManager::DeleteGameHandler( void )
{
    ASSERT( mInitialized );

    char*   BasePathString;
    s32     SystemReturnValue;
    s32     FunctionResult;

    // Here is how a delete works.
    enum DeleteGameStates
    {
        DELETE_GAME_STATE_INIT,
        DELETE_GAME_STATE_WAIT_FOR_BASE_DIR_TO_BE_SET,

        DELETE_GAME_STATE_DELETE_BROWSE_TEXT_FILE,
        DELETE_GAME_STATE_DELETE_USER_DATA_FILE,
        DELETE_GAME_STATE_DELETE_ICON_DATA_FILE,
        DELETE_GAME_STATE_DELETE_ICON_INFO_FILE,
        DELETE_GAME_STATE_DELETE_BASE_DIR_NAME_FILE,
        DELETE_GAME_STATE_DELETE_BASE_DIR,
        
        DELETE_GAME_STATE_RE_EVALUATE_CARD,

        DELETE_GAME_STATE_DONE,

    };

    switch( mCommandState )
    {
        // Test first to see if the game that is to be deleted is actually on the card.
        case DELETE_GAME_STATE_INIT:
        {
            if( mpAvailableSavedGames[mCurrentGameType].mSavedGames[mCurrentGameTypeIndex].mbValid == FALSE )
            {
                CompleteCommandAndWaitForUser(FALSE, QUAG_MU_FILE_NOT_FOUND );
            }
            // If there is a file to be deleted, then try to change directories to the base dir.
            else
            {
                // Change to the base directory that stores this saved game.
                BasePathString = mpSaveGameTypes[mCurrentGameType].mBaseDirectories[mCurrentGameTypeIndex].mActualBaseDirName;

                SystemReturnValue = sceMcChdir( mActiveMemoryUnitPort, mActiveMemoryUnitSlot, BasePathString, 0 );

                // If the Request to change to this base directory succeeded, then continue to wait for the result.
                if( PS2MU_SYSTEM_REQUEST_GRANTED( SystemReturnValue ) )
                {
                    mCommandState = DELETE_GAME_STATE_WAIT_FOR_BASE_DIR_TO_BE_SET;
                }
                // The request to change to the base directory failed, just bail now.
                else
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_DELETE_FAILED );
                }

            }
            break;
        }


        case DELETE_GAME_STATE_WAIT_FOR_BASE_DIR_TO_BE_SET:
        {
            SystemReturnValue = sceMcSync( PS2MU_FUNC_ASYNC_MODE, NULL, &FunctionResult );

            // If for whatever reason there is no function to sync up with, it means that the card was pulled.
            if( SystemReturnValue == sceMcExecIdle )
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
            }
            // The function is complete.
            else if( SystemReturnValue == sceMcExecFinish )
            {
                // Are we now in the base directory path?
                if( FunctionResult == sceMcResSucceed )
                {
                    mCommandSubState = 0;
                    mCommandState = DELETE_GAME_STATE_DELETE_BROWSE_TEXT_FILE;
                }
                // Was the card removed?
                else if( PS2MU_SYSTEM_RETURNED_NO_CARD( FunctionResult ) )
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
                }
                // Was there another problem?
                else
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_DELETE_FAILED );
                }
            }

            break;
        }


        case DELETE_GAME_STATE_DELETE_BROWSE_TEXT_FILE:
        {
            if( DeleteGameHandlerDeleteSingleFile( PS2MU_FILE_TYPE_BROWSE_TEXT ) )
            {
                mCommandState = DELETE_GAME_STATE_DELETE_USER_DATA_FILE;
                mCommandSubState = 0;
            }
            break;
        }

        case DELETE_GAME_STATE_DELETE_USER_DATA_FILE:
        {
            if( DeleteGameHandlerDeleteSingleFile( PS2MU_FILE_TYPE_USER_DATA ) )
            {
                mCommandState = DELETE_GAME_STATE_DELETE_ICON_DATA_FILE;
                mCommandSubState = 0;
            }
            break;
        }

        case DELETE_GAME_STATE_DELETE_ICON_DATA_FILE:
        {
            if( DeleteGameHandlerDeleteSingleFile( PS2MU_FILE_TYPE_ICON_DATA ) )
            {
                mCommandState = DELETE_GAME_STATE_DELETE_ICON_INFO_FILE;
                mCommandSubState = 0;
            }
            break;
        }

        case DELETE_GAME_STATE_DELETE_ICON_INFO_FILE:
        {
            if( DeleteGameHandlerDeleteSingleFile( PS2MU_FILE_TYPE_ICON_INFO ) )
            {
                mCommandState = DELETE_GAME_STATE_DELETE_BASE_DIR_NAME_FILE;
                mCommandSubState = 0;
            }
            break;
        }

        case DELETE_GAME_STATE_DELETE_BASE_DIR_NAME_FILE:
        {
            if( DeleteGameHandlerDeleteSingleFile( PS2MU_FILE_TYPE_BASE_DIR_NAME_FILE ) )
            {
                mCommandState = DELETE_GAME_STATE_DELETE_BASE_DIR;
                mCommandSubState = 0;
            }
            break;
        }

        case DELETE_GAME_STATE_DELETE_BASE_DIR:
        {
            if( DeleteGameHandlerDeleteBaseDir( ) )
            {
                // Complete the command by removing this game from the available game list.
                mpAvailableSavedGames[mCurrentGameType].Clear( mCurrentGameTypeIndex );

                // Re-Evaluate the status of the active card.
                mCommandState = DELETE_GAME_STATE_RE_EVALUATE_CARD;
                mCommandSubState = 0;
            }
            break;
        }

        case DELETE_GAME_STATE_RE_EVALUATE_CARD:
        {
            if( ReEvaluateCardStatus( ) )
                mCommandState = DELETE_GAME_STATE_DONE;
            
            break;
        }

        case DELETE_GAME_STATE_DONE:
        {
            // Complete the command.
            CompleteCommandAndWaitForUser( TRUE, QUAG_MU_EVERYTHING_IS_OK );
            break;
        }

        default:
            ASSERT( 0 );
            break;
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
xbool PS2MemoryUnitManager::DeleteGameHandlerDeleteBaseDir( void )
{
    ASSERT( mInitialized );

    s32 SystemReturnValue;
    s32 FunctionResult;

    enum DeleteGameDeleteBaseDirStates
    {
        DELETE_GAME_DELETE_BASE_DIR_INIT,
        DELETE_GAME_DELETE_BASE_DIR_WAIT_FOR_COMPLETE,
        DELETE_GAME_DELETE_BASE_DIR_COMPLETE,
    };

    switch( mCommandSubState )
    {
        // Try to initiate the request to destroy the base directory.
        case DELETE_GAME_DELETE_BASE_DIR_INIT:
        {
            char* BaseDirString;
            BaseDirString = mpSaveGameTypes[mCurrentGameType].mBaseDirectories[mCurrentGameTypeIndex].mActualBaseDirName;

            SystemReturnValue = sceMcDelete( mActiveMemoryUnitPort, mActiveMemoryUnitSlot, BaseDirString );

            // Command accepted, now wait.
            if( PS2MU_SYSTEM_REQUEST_GRANTED( SystemReturnValue ) )
            {
                mCommandSubState = DELETE_GAME_DELETE_BASE_DIR_WAIT_FOR_COMPLETE;
            }
            // The request to create the base dir failed, just bail now.
            else
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_DELETE_FAILED );
            }
            break;
        }

        // If we are here, we are waiting for the base directory to be created.
        case DELETE_GAME_DELETE_BASE_DIR_WAIT_FOR_COMPLETE:
        {
            SystemReturnValue = sceMcSync( PS2MU_FUNC_ASYNC_MODE, NULL, &FunctionResult );

            // If for whatever reason there is no function to sync up with, it means that the card was pulled.
            if( SystemReturnValue == sceMcExecIdle )
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
            }
            // The function is complete.
            else if( SystemReturnValue == sceMcExecFinish )
            {
                // Did the base directory get destroyed?
                if( FunctionResult == sceMcResSucceed )
                {
                    // Return success.
                    mCommandSubState = DELETE_GAME_DELETE_BASE_DIR_COMPLETE;
                }
                else if( PS2MU_SYSTEM_RETURNED_NO_CARD( FunctionResult ) )
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
                }
                else
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_UNKNOWN_PROBLEM );
                }
            }

            break;
        }

        case DELETE_GAME_DELETE_BASE_DIR_COMPLETE:
        {
            f64 ElapsedTime;
            ElapsedTime = x_GetElapsedTime( mCommandStartTime );

            if( ElapsedTime > PS2MU_MIN_DELETE_TIME )
            {
                return TRUE;
            }

            break;
        }

        default:
            ASSERT( 0 );
            break;
    }


    return FALSE;
}

//-------------------------------------------------------------------------------------------------------------------------------------
xbool PS2MemoryUnitManager::DeleteGameHandlerDeleteSingleFile( s32 FileType )
{
    ASSERT( mInitialized );

    s32 SystemReturnValue;
    s32 FunctionResult;

    enum DeleteGameSingleFileStates
    {
        DELETE_GAME_SINGLE_FILE_INIT,
        DELETE_GAME_SINGLE_FILE_WAIT_FOR_FILE_TO_BE_DELETED,
    };

    switch( mCommandSubState )
    {
        case DELETE_GAME_SINGLE_FILE_INIT:
        {
            switch( FileType )
            {
                case PS2MU_FILE_TYPE_BASE_DIR_NAME_FILE:
                {
                    // Killing the base directory name file.
                    x_strncpy( mCurrentOpenFileName, 
                               &mpSaveGameTypes[mCurrentGameType].mBaseDirectories[mCurrentGameTypeIndex].mActualBaseDirName[1],    // The first character is '/' and we must remove it.
                               PS2MU_UTILITY_FILENAME_SIZE );

                    break;
                }

                case PS2MU_FILE_TYPE_ICON_INFO:
                {
                    // Killing the icon info file.
                    x_strncpy( mCurrentOpenFileName, PS2_STORED_ICON_INFO_FILE, PS2MU_UTILITY_FILENAME_SIZE );
                    break;
                }

                case PS2MU_FILE_TYPE_ICON_DATA:
                {
                    // Killing the icon data file.
                    x_strncpy( mCurrentOpenFileName, PS2_STORED_ICON_DATA_FILE, PS2MU_UTILITY_FILENAME_SIZE );
                    break;
                }
    
                case PS2MU_FILE_TYPE_BROWSE_TEXT:
                {
                    // Kill the browse info file.
                    x_strncpy( mCurrentOpenFileName, PS2_STORED_BROWSE_TEXT_FILE, PS2MU_UTILITY_FILENAME_SIZE );
                    break;
                }

                case PS2MU_FILE_TYPE_USER_DATA:
                {
                    // Kill the user data file.
                    x_strncpy( mCurrentOpenFileName, 
                               mpSaveGameTypes[mCurrentGameType].mUserDataFileInfo.mUserDataSaveFileName,
                               PS2MU_UTILITY_FILENAME_SIZE );
                    break;
                }

                default:
                    ASSERT( 0 );
                    break;
            }

            // Fail if there is already a file open.
            ASSERT( !IsFileHandleInUse( ) );

            // Request that the file be deleted
            SystemReturnValue = sceMcDelete( mActiveMemoryUnitPort, mActiveMemoryUnitSlot, mCurrentOpenFileName );

            // Command accepted, now wait.
            if( PS2MU_SYSTEM_REQUEST_GRANTED( SystemReturnValue ) )
            {
                mCommandSubState = DELETE_GAME_SINGLE_FILE_WAIT_FOR_FILE_TO_BE_DELETED;
            }
            // The request to create the file failed, so bail.
            else
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_DELETE_FAILED );
            }
            break;
        }

        case DELETE_GAME_SINGLE_FILE_WAIT_FOR_FILE_TO_BE_DELETED:
        {
            SystemReturnValue = sceMcSync( PS2MU_FUNC_ASYNC_MODE, NULL, &FunctionResult );

            // If for whatever reason there is no function to sync up with, it means that the card was pulled.
            if( SystemReturnValue == sceMcExecIdle )
            {
                CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
            }
            // The function is complete.
            else if( SystemReturnValue == sceMcExecFinish )
            {
                // Was the delete a success?, or was the file not even there?
                if( FunctionResult == sceMcResSucceed ||
                    FunctionResult == sceMcResNoEntry )
                {
                    ClearCurrentFileHandle( );
                    return TRUE;
                }
                else if( PS2MU_SYSTEM_RETURNED_NO_CARD( FunctionResult ) )
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_NO_CARD );
                }
                else
                {
                    CompleteCommandAndWaitForUser( FALSE, QUAG_MU_GENERIC_DELETE_FAILED );
                }
            }

            break;
        }

        default:
            ASSERT( 0 );
            break;
    }

    return FALSE;
}


//-------------------------------------------------------------------------------------------------------------------------------------
void PS2MemoryUnitManager::GetPS2PortAndSlotFromMemoryUnitID( s32 MemUnitID, s32* CurrentPort, s32* CurrentSlot )
{
    if( (MemUnitID < QUAG_MEMORY_UNIT_ID_4) )
    {
        *CurrentPort = 0;
        *CurrentSlot = MemUnitID;
    }
    else if( MemUnitID <= QUAG_MEMORY_UNIT_ID_7 )
    {
        *CurrentPort = 1;
        *CurrentSlot = MemUnitID - QUAG_MEMORY_UNIT_ID_4;
    }
    else
    {
        ASSERT( 0 );
        *CurrentPort = 0;
        *CurrentSlot = 0;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
s32 PS2MemoryUnitManager::GetMemoryUnitIDFromPS2PortAndSlot( s32 CurrentPort, s32 CurrentSlot )
{
    ASSERT( (CurrentPort >= PS2MU_MIN_PORT) && (CurrentPort <= PS2MU_MAX_PORT) );
    ASSERT( (CurrentSlot >= PS2MU_MIN_SLOT) && (CurrentSlot <= PS2MU_MAX_SLOT) );

    s32 MemUnitID = QUAG_MEMORY_UNIT_ID_0 + (CurrentPort * 4) + (CurrentSlot);

    return MemUnitID;
}

//-------------------------------------------------------------------------------------------------------------------------------------
void PS2MemoryUnitManager::ClearAvailableGamesList( void )
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
xbool PS2MemoryUnitManager::SetCurrentFileHandle( s32 NewFileHandle )
{
    ASSERT( mInitialized );

    if( mOpenFileHandle == PS2MU_CLEAR_FILE_HANDLE_VALUE )
    {
        MC_PRINT( 1, "File handle has been set" );
        mOpenFileHandle = NewFileHandle;
        return TRUE;
    }
    else 
    {
        ASSERT( 0 );
        return FALSE;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool PS2MemoryUnitManager::IsFileHandleInUse( void )
{
    return mOpenFileHandle != PS2MU_CLEAR_FILE_HANDLE_VALUE;
}

//-------------------------------------------------------------------------------------------------------------------------------------
void PS2MemoryUnitManager::ClearCurrentFileHandle( void )
{
    MC_PRINT( 1, "File handle has been cleared" );
    mOpenFileHandle = PS2MU_CLEAR_FILE_HANDLE_VALUE;
}


//-------------------------------------------------------------------------------------------------------------------------------------
void PS2MemoryUnitManager::ResetCommandHandlingVariables( void )
{
    // Initialize all other values.
    mpCurrentCommandHandler                 = NULL;
    mCurrentCommand                         = PS2MU_CMD_NONE;
    mbCommandCompleted                      = TRUE;
    mbWaitingForCommandCompleteAcknowledged = FALSE;

    // Initialize system to scan all ports.
    mbAllowBackgroundCardChecking           = FALSE;
    mbBackgrounCardCheckInProgress          = FALSE;
    mCurrentBackgroundCardBeingChecked      = -1;

    ClearCurrentFileHandle( );

    // Identify that no particular Memory Unit has been selected to be worked with.
    mActiveMemoryUnit = NULL;
    mActiveMemoryUnitPort = -1;
    mActiveMemoryUnitSlot = -1;
    mActiveMemoryUnitReusableEnvironmentSpace = 0;
}


//-------------------------------------------------------------------------------------------------------------------------------------
void PS2MemoryUnitManager::GeneratePersonalizedNameBasedOnPortAndSlot( PS2MemoryUnit* pMemoryUnit )
{
    char NewPersonalizedName[MEMORY_UNIT_PERSONALIZED_NAME_SIZE];

    GenerateCardName( pMemoryUnit->mUnitID, NewPersonalizedName, MEMORY_UNIT_PERSONALIZED_NAME_SIZE );
    pMemoryUnit->SetName( NewPersonalizedName );
}


//-------------------------------------------------------------------------------------------------------------------------------------
xbool PS2MemoryUnitManager::GenerateCardName( s32 QuagMemCardID, char* pBuffer, s32 SizeOfBuffer )
{
    s32   Slot;
    s32   Port;
    char  CardName[MEMORY_UNIT_PERSONALIZED_NAME_SIZE];
    char  MultitapExtension[5];
    s32   StringLength;

    // Get the slot and port for this memory card.
    GetPS2PortAndSlotFromMemoryUnitID( QuagMemCardID, &Port, &Slot );

    // Start by putting in the basic name.
                       // 123456789 123456789 123456789 123456789 12345
    x_sprintf( CardName, "memory card (PS2) in MEMORY CARD slot %d", Port == 0 ? 1 : 2 );

    // If the memory card is inserted into a multitap or that's the only place you will see it
    // you must add the extension to the name.
    if( (sceMtapGetConnection( Port ) == 1) || (Slot > 0) )
    {
        x_sprintf( MultitapExtension, "-%c", 'A' + Slot );
        x_strcat( CardName, MultitapExtension );
    }

    // Make sure that the provided buffer is large enough to hold the entire name.
    StringLength = x_strlen( CardName );
    if( StringLength < (SizeOfBuffer - 1) )
    {
        x_strcpy( pBuffer, CardName );                
        return TRUE;
    }
    else
    {
        pBuffer[0] = 0;
        return FALSE;
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------
void PS2MemoryUnitManager::ProcessCardConnected( PS2MemoryUnit* pMemoryUnit )
{
    ASSERT( mInitialized );

    // Perform a test to see if this memory card is a pocket station or not.  If it is, terminate now
    // we are not supporting them, and therefore must ignore them.
    if( pMemoryUnit->mType == PS2MU_POCKETSTATION )
    {
        // If this is the first time this card has gone through the connected loop when it's first plugged in, 
        // give it a name, and show some debug text.  Otherwise, it's already been through this loop one time,
        // so don't flood the debug window while it's still plugged in.
        if( pMemoryUnit->mName[0] == NULL )
        {
            x_sprintf( pMemoryUnit->mName, "PocketStation%d", pMemoryUnit->mUnitID );
            MC_PRINT( 1, "%s Connected, but will be ignored.", pMemoryUnit->mName );
        }
    }
    else
    {
        // Identify that it's connected
        pMemoryUnit->mbConnected = TRUE;

        // Generate the correct name for this memory unit based on it's connection status.
        GeneratePersonalizedNameBasedOnPortAndSlot( pMemoryUnit );

#ifdef SHOW_PS2_DIAGNOSTICS
        char strFormatted[10];

        if( pMemoryUnit->mbFormatted )
            x_sprintf( strFormatted, "IS" );
        else
            x_sprintf( strFormatted, "IS NOT" );

        MC_PRINT( 1, "Unit %d - %s was Inserted and %s Formatted.", pMemoryUnit->mUnitID, pMemoryUnit->mName, strFormatted );
#endif 
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
void PS2MemoryUnitManager::ProcessCardDisconnected( PS2MemoryUnit* pMemoryUnit )
{
    ASSERT( mInitialized );

    // If the memory unit that is disconnected is the currently selected memory unit make sure that it's released.
    if( pMemoryUnit == mActiveMemoryUnit )
    {
        // Now free the active memory unit.
        ReleaseActiveMemoryUnit( );
    }

    MC_PRINT( 1, "Unit %d - %s was Removed", pMemoryUnit->mUnitID, pMemoryUnit->mName );
    pMemoryUnit->Reset( );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void PS2MemoryUnitManager::InitializeAllDefaultIconData( void )
{
    // Clear out the SONY required information to clear all of the SONY resevered data all at once.
    x_memset( (void*)&mIconData.mSonyMcIconSys, 0, sizeof( mIconData.mSonyMcIconSys ) );

    // Fill the Header.
    x_strcpy( (char*)mIconData.mSonyMcIconSys.Head, "PS2D" );
}


//-------------------------------------------------------------------------------------------------------------------------------------
void PS2MemoryUnitManager::BuildIconFromResourceData( PS2MemCardResourceIconInfo* pResourceData, byte* pRawIconData )
{
    mIconData.mIconDataSize = pResourceData->mIconDataSize;

    // Prepare a data buffer that is large enough to store the icon data.
    mIconData.ReleaseData( );
    mIconData.mpIconData = new byte[mIconData.mIconDataSize];
    ASSERT( mIconData.mpIconData );

    x_memcpy( mIconData.mpIconData, pRawIconData, mIconData.mIconDataSize );

    // Rebuild the SONY required info to make the Icon.sys file.    
    x_strcpy( (char*)mIconData.mSonyMcIconSys.FnameView, PS2_STORED_ICON_DATA_FILE);   // Assign the name of the icon file to be used when listing the games in the browser.
    x_strcpy( (char*)mIconData.mSonyMcIconSys.FnameCopy, PS2_STORED_ICON_DATA_FILE);   // Assign the name of the icon file to be used when copying the game in the browser.
    x_strcpy( (char*)mIconData.mSonyMcIconSys.FnameDel,  PS2_STORED_ICON_DATA_FILE);   // Assign the name of the icon file to be used when deleting the game in the browser.

    // Grab all information regarding how the background of the browser screen on the PS2 is to be 
    // rendered when this games icon is highlighted.
    mIconData.mSonyMcIconSys.TransRate     = pResourceData->mBackgroundTransparency;
    mIconData.mSonyMcIconSys.BgColor[0][0] = pResourceData->mBGUpperLeftColor.R;
    mIconData.mSonyMcIconSys.BgColor[0][1] = pResourceData->mBGUpperLeftColor.G;
    mIconData.mSonyMcIconSys.BgColor[0][2] = pResourceData->mBGUpperLeftColor.B;
    mIconData.mSonyMcIconSys.BgColor[0][3] = 0;

    mIconData.mSonyMcIconSys.BgColor[1][0] = pResourceData->mBGUpperRightColor.R;
    mIconData.mSonyMcIconSys.BgColor[1][1] = pResourceData->mBGUpperRightColor.G;
    mIconData.mSonyMcIconSys.BgColor[1][2] = pResourceData->mBGUpperRightColor.B;
    mIconData.mSonyMcIconSys.BgColor[1][3] = 0;

    mIconData.mSonyMcIconSys.BgColor[2][0] = pResourceData->mBGLowerLeftColor.R;
    mIconData.mSonyMcIconSys.BgColor[2][1] = pResourceData->mBGLowerLeftColor.G;
    mIconData.mSonyMcIconSys.BgColor[2][2] = pResourceData->mBGLowerLeftColor.B;
    mIconData.mSonyMcIconSys.BgColor[2][3] = 0;
    
    mIconData.mSonyMcIconSys.BgColor[3][0] = pResourceData->mBGLowerRightColor.R;
    mIconData.mSonyMcIconSys.BgColor[3][1] = pResourceData->mBGLowerRightColor.G;
    mIconData.mSonyMcIconSys.BgColor[3][2] = pResourceData->mBGLowerRightColor.B;
    mIconData.mSonyMcIconSys.BgColor[3][3] = 0;

    // Grab all information regarding how to 'light' the icon in the browser screen on the PS2 
    // when this games icon is highlighted.
    //
    // As of asb 2003, here is how the axis worked for the ps2 icon in the browser screen.
    //
    //    Y
    //  /
    // +---X
    // |
    // Z
    //
    mIconData.mSonyMcIconSys.LightDir[0][0]   = pResourceData->mLight1_Dir.X;
    mIconData.mSonyMcIconSys.LightDir[0][1]   = pResourceData->mLight1_Dir.Y;
    mIconData.mSonyMcIconSys.LightDir[0][2]   = pResourceData->mLight1_Dir.Z;
    mIconData.mSonyMcIconSys.LightDir[0][3]   = 0.0f;

    mIconData.mSonyMcIconSys.LightColor[0][0] = (f32)pResourceData->mLight1_Color.R / 255.0f;
    mIconData.mSonyMcIconSys.LightColor[0][1] = (f32)pResourceData->mLight1_Color.G / 255.0f;
    mIconData.mSonyMcIconSys.LightColor[0][2] = (f32)pResourceData->mLight1_Color.B / 255.0f;
    mIconData.mSonyMcIconSys.LightColor[0][3] = 0.0f;

    mIconData.mSonyMcIconSys.LightDir[1][0]   = pResourceData->mLight2_Dir.X;
    mIconData.mSonyMcIconSys.LightDir[1][1]   = pResourceData->mLight2_Dir.Y;
    mIconData.mSonyMcIconSys.LightDir[1][2]   = pResourceData->mLight2_Dir.Z;
    mIconData.mSonyMcIconSys.LightDir[1][3]   = 0.0f;

    mIconData.mSonyMcIconSys.LightColor[1][0] = (f32)pResourceData->mLight2_Color.R / 255.0f;
    mIconData.mSonyMcIconSys.LightColor[1][1] = (f32)pResourceData->mLight2_Color.G / 255.0f;
    mIconData.mSonyMcIconSys.LightColor[1][2] = (f32)pResourceData->mLight2_Color.B / 255.0f;
    mIconData.mSonyMcIconSys.LightColor[1][3] = 0.0f;

    mIconData.mSonyMcIconSys.LightDir[2][0]   = pResourceData->mLight3_Dir.X;
    mIconData.mSonyMcIconSys.LightDir[2][1]   = pResourceData->mLight3_Dir.Y;
    mIconData.mSonyMcIconSys.LightDir[2][2]   = pResourceData->mLight3_Dir.Z;
    mIconData.mSonyMcIconSys.LightDir[2][3]   = 0.0f;

    mIconData.mSonyMcIconSys.LightColor[2][0] = (f32)pResourceData->mLight3_Color.R / 255.0f;
    mIconData.mSonyMcIconSys.LightColor[2][1] = (f32)pResourceData->mLight3_Color.G / 255.0f;
    mIconData.mSonyMcIconSys.LightColor[2][2] = (f32)pResourceData->mLight3_Color.B / 255.0f;
    mIconData.mSonyMcIconSys.LightColor[2][3] = 0.0f;

    mIconData.mSonyMcIconSys.Ambient[0]       = (f32)pResourceData->mAmbient_Color.R / 255.0f;
    mIconData.mSonyMcIconSys.Ambient[1]       = (f32)pResourceData->mAmbient_Color.G / 255.0f;
    mIconData.mSonyMcIconSys.Ambient[2]       = (f32)pResourceData->mAmbient_Color.B / 255.0f;
    mIconData.mSonyMcIconSys.Ambient[3]       = 0.0f;
}




//-------------------------------------------------------------------------------------------------------------------------------------
char* PS2MemoryUnitManager::GetCommandNameAsString( s32 CommandID )
{
    static char* CommandNames[] =
    {
        "PS2MU_CMD_NONE",
        "PS2MU_CMD_MOUNT",
        "PS2MU_CMD_FORMAT",
        "PS2MU_CMD_SAVE",
        "PS2MU_CMD_LOAD",
        "PS2MU_CMD_DELETE",
        "UNKNOWN_CMD",
    };

    if( CommandID >= PS2MU_CMD_FIRST && CommandID <= PS2MU_CMD_LAST )
    {
        return CommandNames[CommandID];
    }
    else
    {
        return CommandNames[PS2MU_CMD_LAST + 1];
    }
}
