#ifndef PC_MEMORY_UNIT_RESOURCE_HPP
#define PC_MEMORY_UNIT_RESOURCE_HPP

//=====================================================================================================================================
// Required Includes.
//=====================================================================================================================================
#include "Q_MemoryUnitResource.hpp"

//=====================================================================================================================================
// Definitions.
//=====================================================================================================================================
#define PC_USER_DATA_FILENAME_LENGTH      32  // Don't konw what this is yet  so 1 will break things to wake me up.

//=====================================================================================================================================
// PC Resource structures
//=====================================================================================================================================
struct PCMUResourceHeader
{
    s32 mResourceType;                  // Resource Type, see QuagMemoryUnitResourceTypes

    s32 mNumberOfSaveTypes;             // Number of unique types of saves for this game.
    s32 mOffsetToSaveTypes;             // Offset from the start of the file to the save type definitions.

    s32 mSizeOfIconData;
    s32 mOffsetToIconData;              // Offset from the start of the file to the actual icon data.

    s32 mSizeOfEntireResourceFile;      // Size of entire resource file to use to load entire resource if you wish.
};


//-------------------------------------------------------------------------------------------------------------------------------------
struct PCMUUserDataFile
{
    char    mUserDataSaveFileName[PC_USER_DATA_FILENAME_LENGTH];  // The names of each user data file that is saved when using this save type.
    s32     mSizeOfUserDataFile;                                    // The exact number of bytes of user data stored within this file.
};


//-------------------------------------------------------------------------------------------------------------------------------------
struct PCMUResourceSaveGameType
{
    // How many saves of this type are going to be used. Max is MEMORY_UNIT_MAX_SAVE_COUNT_PER_TYPE
    // There is one PS2MemCardBaseDirectory for each save of this type.
    s32                 mMaxNumberOfSaves;
    
    // Information on the user file that is special to this particular save type.
    PCMUUserDataFile  mUserDataFileInfo;
};


//=====================================================================================================================================
// PC Resource File Layout.
//=====================================================================================================================================
// 
//-------------------------------------------------------------------------------------------------------------------------------------
//
// PCMUResourceHeader
//
//-------------------------------------------------------------------------------------------------------------------------------------
//
// PCMUResourceSaveGameType 0
// PCMUResourceSaveGameType 1
// PCMUResourceSaveGameType 2
// ..
// ..
// ..
// PCMUResourceSaveGameType X
//
//-------------------------------------------------------------------------------------------------------------------------------------
//
// PC Icon Data
//
//=====================================================================================================================================

#endif // PC_MEMORY_UNIT_RESOURCE_HPP